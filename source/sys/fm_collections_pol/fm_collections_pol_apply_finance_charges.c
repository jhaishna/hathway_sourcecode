/*
 *      Copyright (c) 2001-2006 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 */

#ifndef lint
static const char Sccs_id[] = "@(#) %full_filespec: fm_collections_pol_apply_finance_charges.c~2:csrc:1 %";
#endif

#include <stdio.h>
#include <string.h>

#include <pcm.h>
#include <pinlog.h>

#include "cm_fm.h"
#include "cm_cache.h"
#include "ops/collections.h"
#include "ops/bal.h"
#include "fm_utils.h"
#include "fm_collections.h"
#include "pin_rate.h"


#ifdef MSDOS
#include <WINDOWS.h>
#endif


/*******************************************************************
 * Routines defined here.
 *******************************************************************/

EXPORT_OP void
op_collections_pol_apply_finance_charges(
	cm_nap_connection_t	*connp, 
	int32			opcode, 
	int32			flags, 
	pin_flist_t		*in_flistp, 
	pin_flist_t		**ret_flistpp, 
	pin_errbuf_t		*ebufp);

static void
fm_collections_pol_apply_finance_charges(
	pcm_context_t		*ctxp, 
	int32			flags, 
	pin_flist_t		*in_flistp, 
	pin_flist_t		**ret_flistpp, 
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * Variables/functions defined elsewhere
 *******************************************************************/
extern int32 fm_collections_pol_get_currency(
        pcm_context_t           *ctxp,
        poid_t                  *a_pdp,
        pin_errbuf_t            *ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_COLLECTIONS_POL_APPLY_FINANCE_CHARGES 
 *******************************************************************/
void
op_collections_pol_apply_finance_charges(
	cm_nap_connection_t	*connp, 
	int32			opcode, 
	int32			flags, 
	pin_flist_t		*in_flistp, 
	pin_flist_t		**ret_flistpp, 
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_COLLECTIONS_POL_APPLY_FINANCE_CHARGES) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_collections_pol_apply_finance_charges opcode error", 
			ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_collections_pol_apply_finance_charges input flist", in_flistp);

	/***********************************************************
	 * Do the actual op in a sub. Copy it since we may need to
	 * replace it later.
	 ***********************************************************/

	fm_collections_pol_apply_finance_charges(ctxp, flags, in_flistp, 
			ret_flistpp, ebufp);

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_collections_pol_apply_finance_charges error", ebufp);
	} else {
		/***************************************************
		 * Debug: What we're sending back.
		 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_collections_pol_apply_finance_charges return flist", 
			*ret_flistpp);
	}
	return;
}

/*******************************************************************
 * fm_collections_pol_apply_finance_charges()
 *
 * This function calculates the finance charges.
 *
 *******************************************************************/
static void
fm_collections_pol_apply_finance_charges(
	pcm_context_t		*ctxp, 
	int32			flags, 
	pin_flist_t		*in_flistp, 
	pin_flist_t		**ret_flistpp, 
	pin_errbuf_t		*ebufp)
{
	void			*vp = NULL;
	pin_flist_t		*i_flistp = NULL;
	pin_flist_t		*e_flistp = NULL;
	pin_flist_t		*e1_flistp = NULL;
	pin_flist_t		*bi_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*ca_flistp = NULL;
	char			*action_name = NULL;

	poid_t			*a_pdp = NULL;
	poid_t			*action_pdp = NULL;
	poid_t                  *b_pdp = NULL;
	pin_flist_t		*fc_flistp = NULL;
	pin_decimal_t		*charge = NULL;
	pin_decimal_t		*rate_pdp = NULL;
	pin_decimal_t		*denominator = NULL;
	char			*prog_name = "pin_collections_process";
	int32			currency = 0;
	time_t                  now_t = pin_virtual_time((time_t *)NULL);
	int32                   impact_type = PIN_IMPACT_TYPE_PRERATED;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);


	
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_collections_pol_apply_finance_charges input flist",
                in_flistp);


	action_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);


	a_pdp = (poid_t *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ, 
				0, ebufp);

	b_pdp = (poid_t *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_BILLINFO_OBJ,
                        0, ebufp);



	e1_flistp = PIN_FLIST_CREATE(ebufp);

        PIN_FLIST_FLD_SET(e1_flistp, PIN_FLD_POID , (void *)action_pdp,
                        ebufp);

        e_flistp = PIN_FLIST_ELEM_ADD(e1_flistp, PIN_FLD_EVENT,
                                        0, ebufp);


        PIN_FLIST_FLD_SET(e_flistp, PIN_FLD_ACCOUNT_OBJ , (void *)a_pdp,
                        ebufp);
        PIN_FLIST_FLD_SET(e_flistp, PIN_FLD_BILLINFO_OBJ ,(void *)b_pdp,
                        ebufp);



	/* Get the action name to start with */
	i_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_POID, (void *)action_pdp, ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CONFIG_ACTION_OBJ, NULL, ebufp);


	PCM_OP(ctxp, PCM_OP_READ_FLDS, PCM_OPFLG_CACHEABLE, i_flistp, 
				&r_flistp, ebufp);

	PIN_FLIST_DESTROY_EX(&i_flistp, NULL);

	if ( PIN_ERR_IS_ERR( ebufp )) { 
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_collections_pol_apply_finance_charges read error",
			ebufp);
		goto cleanup;
		/***********/
	}


	vp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_CONFIG_ACTION_OBJ, 0, ebufp);
	ca_flistp = fm_collections_cache_get_action(ctxp, (poid_t *)vp, ebufp);

	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);


	vp = PIN_FLIST_FLD_GET(ca_flistp, PIN_FLD_ACTION_NAME,
			0, ebufp);

	PIN_FLIST_FLD_SET(e_flistp, PIN_FLD_DESCR , (void *)vp,
                        ebufp);


	/* Get the finance charge info substructure */
	fc_flistp = PIN_FLIST_SUBSTR_GET(ca_flistp, 
				PIN_FLD_FINANCE_CHARGE_INFO, 0, ebufp);

	rate_pdp = (pin_decimal_t *)PIN_FLIST_FLD_GET(fc_flistp, 
			PIN_FLD_PERCENT, 0, ebufp);

	/* Calculate the finance charges */
	vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_DUE, 0, ebufp);
	charge = pbo_decimal_multiply((pin_decimal_t *)vp, rate_pdp, ebufp);
	denominator = pbo_decimal_from_double(100.0, ebufp);
	pbo_decimal_divide_assign(charge, denominator, ebufp);
	pbo_decimal_destroy(&denominator);



	currency = fm_collections_pol_get_currency(ctxp, b_pdp, ebufp);

	
        /* Fill BAL_IMPACTS Array */

        bi_flistp = PIN_FLIST_ELEM_ADD(e_flistp, PIN_FLD_BAL_IMPACTS,
                                        0, ebufp);

        PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_IMPACT_TYPE , (void *)&impact_type,
                        ebufp);


        PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_RESOURCE_ID , (void *)&currency,
                        ebufp);

        PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_AMOUNT , (void *)charge,
                        ebufp);


        *ret_flistpp = e1_flistp;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_collections_pol_apply_finance_charges return flist", 
			*ret_flistpp);

cleanup:
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
}
