/*******************************************************************
 *
 *      Copyright (c) 1999-2006 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char Sccs_Id[] = "@(#)%Portal Version: fm_cust_pol_valid_locale.c:BillingVelocityInt:2:2006-Sep-05 04:32:35 %";
#endif

#include <stdio.h>

#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pinlog.h"

/*******************************************************************
 * Functions contained within.
 *******************************************************************/
EXPORT_OP void
op_cust_pol_valid_locale(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp);

static void
fm_cust_pol_valid_locale(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**out_flistpp,
        pin_errbuf_t		*ebufp);

/*******************************************************************
 * Routines needed from elsewhere.
 *******************************************************************/
extern pin_flist_t *
fm_cust_pol_valid_add_fail(pin_flist_t  *r_flistp,
                           int  field,
                           int  elemid,
                           int  result,
                           char         *descr,
                           void         *val,
                           pin_errbuf_t *ebufp);


 
/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_VALID_LOCALE operation.
 *******************************************************************/
void
op_cust_pol_valid_locale(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*r_flistp = NULL;


	/*
	 * Null out results until we have some.
	 */
	*ret_flistpp = NULL;
	PIN_ERR_CLEAR_ERR(ebufp);

	/*
	 * Insanity check.
	 */
	if (opcode != PCM_OP_CUST_POL_VALID_LOCALE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_cust_pol_valid_locale", ebufp);
		return; 
	}

	/*
	 * Debug: what did we get?
	 */
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_cust_pol_valid_locale input flist", i_flistp);

	/*
	 * Call main function to do it
	 */
	fm_cust_pol_valid_locale(ctxp, i_flistp, &r_flistp, ebufp);

	/*
	 * Results.
	 */
	if (PIN_ERR_IS_ERR(ebufp)) {
		*ret_flistpp = (pin_flist_t *)NULL;
		PIN_FLIST_DESTROY(r_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_cust_pol_valid_locale error", ebufp);
	} else {
		*ret_flistpp = r_flistp;
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_valid_locale return flist", r_flistp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_valid_locale()
 *
 *	Validate the locale for an account.
 *
 *	XXX NOOP - STUBBED ONLY - ALWAYS RETURN PASS XXX
 *
 *******************************************************************/
static void
fm_cust_pol_valid_locale(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**out_flistpp,
        pin_errbuf_t		*ebufp)
{
	int32			result = PIN_CUST_VERIFY_PASSED;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*a_flistp = NULL;
	char			*sourceLocale = NULL;
	int			isValid = 0;
	poid_t			*a_pdp = NULL;
	pin_cookie_t		cookie = NULL;
	int32			dummy = 0;


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_cust_pol_valid_locale input flist", i_flistp);

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/*
	 * Create outgoing flist
	 */
	r_flistp = PIN_FLIST_CREATE(ebufp);
	*out_flistpp = r_flistp;
	a_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, a_pdp, ebufp);

	sourceLocale = (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_LOCALE, 1, ebufp);
	if (!sourceLocale) {
		goto checkout;
		/************/
	}

	isValid = PIN_TEST_INFRANET_LOCALE(sourceLocale, ebufp );

	/*
	 * No need to set error in ebufp for validation errors
	 */
	PIN_ERR_CLEAR_ERR(ebufp);

	if(isValid) {
		goto checkout;
		/************/
	}

	result = PIN_CUST_VERIFY_FAILED;
	fm_cust_pol_valid_add_fail(r_flistp,
                                        PIN_FLD_LOCALE, 0,
                                        PIN_ERR_BAD_VALUE,
                                        PIN_CUST_BAD_VALUE_ERR_MSG,
                                        sourceLocale, ebufp);

checkout:
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_RESULT, (void *)&result, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_cust_pol_valid_locale return flist", r_flistp);
	
	return;
}
