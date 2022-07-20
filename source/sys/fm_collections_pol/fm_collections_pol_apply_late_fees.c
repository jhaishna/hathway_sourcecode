/*
 *      Copyright (c) 2001-2007 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 */

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_collections_pol_apply_late_fees.c:CollectionsR1Int:3:2007-Oct-01 12:07:15 %";
#endif

#include <stdio.h>
#include <string.h>

#include <pcm.h>
#include <pinlog.h>

#include "cm_fm.h"
#include "cm_cache.h"
#include "ops/collections.h"
#include "ops/bill.h"
#include "ops/bal.h"
#include "ops/act.h"
#include "fm_utils.h"
#include "fm_collections.h"
#include "pin_rate.h"
#include "mso_ops_flds.h"

#ifdef MSDOS
#include <WINDOWS.h>
#endif


/*******************************************************************
 * Routines defined here.
 *******************************************************************/

EXPORT_OP void
op_collections_pol_apply_late_fees(
	cm_nap_connection_t	*connp, 
	int32			opcode, 
	int32			flags, 
	pin_flist_t		*in_flistp, 
	pin_flist_t		**ret_flistpp, 
	pin_errbuf_t		*ebufp);

extern void fm_mso_create_lifecycle_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	int32			action,
	pin_errbuf_t		*ebufp);

static void
fm_collections_pol_apply_late_fees(
	pcm_context_t		*ctxp, 
	int32			flags, 
	pin_flist_t		*in_flistp, 
	pin_flist_t		**ret_flistpp, 
	pin_errbuf_t		*ebufp);

int32
fm_collections_pol_get_currency(
	pcm_context_t		*ctxp, 
	poid_t			*a_pdp, 
	pin_errbuf_t		*ebufp);

void
fm_collections_pol_calc_late_fees(
	pcm_context_t		*ctxp, 
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);
/*******************************************************************
 * Variables/functions defined elsewhere
 *******************************************************************/
/*******************************************************************
 * Main routine for the PCM_OP_COLLECTIONS_POL_APPLY_LATE_FEES 
 *******************************************************************/
void
op_collections_pol_apply_late_fees(
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
	if (opcode != PCM_OP_COLLECTIONS_POL_APPLY_LATE_FEES) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_collections_apply_late_fees opcode error", 
			ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_collections_apply_late_fees input flist", in_flistp);

	/***********************************************************
	 * Do the actual op in a sub. Copy it since we may need to
	 * replace it later.
	 ***********************************************************/

	fm_collections_pol_apply_late_fees(ctxp, flags, in_flistp, 
			ret_flistpp, ebufp);

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_collections_apply_late_fees error", ebufp);
	} else {
		/***************************************************
		 * Debug: What we're sending back.
		 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_collections_apply_late_fees return flist", 
			*ret_flistpp);
	}
	return;
}

/*******************************************************************
 * fm_collections_pol_apply_late_fees()
 *
 * This function applies late fees for the billinfo.
 * The late fees can be a fixed amount or a percentage of the current
 * overdue amount or the combination of the two.
 *******************************************************************/
static void
fm_collections_pol_apply_late_fees(
	pcm_context_t		*ctxp, 
	int32			flags, 
	pin_flist_t		*in_flistp, 
	pin_flist_t		**ret_flistpp, 
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*i_flistp = NULL;
	pin_flist_t		*lr_flistp = NULL;
	pin_flist_t		*e_flistp = NULL;
	pin_flist_t		*e1_flistp = NULL;
	pin_flist_t		*bi_flistp = NULL;

	poid_t			*action_pdp = NULL;
	poid_t			*a_pdp = NULL;
	poid_t			*b_pdp = NULL;
	void			*vp = NULL;
	char			*prog_name = "pin_collections_process";
	time_t			now_t = pin_virtual_time((time_t *)NULL);
	int32			impact_type = PIN_IMPACT_TYPE_PRERATED;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_collections_pol_apply_late_fees input flist",
		in_flistp);

	action_pdp = (poid_t *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 
			0, ebufp);

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


	fm_collections_pol_calc_late_fees(ctxp,in_flistp ,&lr_flistp,ebufp);

	vp = PIN_FLIST_FLD_GET(lr_flistp,PIN_FLD_ACTION_NAME,0,ebufp);

	PIN_FLIST_FLD_SET(e_flistp, PIN_FLD_DESCR , (void *)vp,
			ebufp);
	


	/* Fill BAL_IMPACTS Array */
		
	bi_flistp = PIN_FLIST_ELEM_ADD(e_flistp, PIN_FLD_BAL_IMPACTS, 
					0, ebufp);

	PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_IMPACT_TYPE , (void *)&impact_type,
			ebufp);


	vp = PIN_FLIST_FLD_GET(lr_flistp,PIN_FLD_CURRENCY,0,ebufp);
	PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_RESOURCE_ID , (void *)vp,
			ebufp);

	vp = PIN_FLIST_FLD_GET(lr_flistp,PIN_FLD_AMOUNT,0,ebufp);
	PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_AMOUNT , (void *)vp,
			ebufp);


	*ret_flistpp = e1_flistp;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_collections_pol_apply_late_fees return flist",
		*ret_flistpp);

	PIN_FLIST_DESTROY_EX(&lr_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
}


/*******************************************************************
 * This function calculates the late fees for the billinfo.
 * The late fees can be a fixed amount or a percentage of the current
 * overdue amount or the combination of the two.
 *******************************************************************/

void
fm_collections_pol_calc_late_fees(
	pcm_context_t		*ctxp, 
	pin_flist_t		*in_flistp ,
	pin_flist_t		**ret_flistpp ,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*i_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*t_flistp = NULL;
	pin_flist_t		*ca_flistp = NULL;
	pin_flist_t		*lf_flistp = NULL;
	void			*vp = NULL;
	poid_t			*ca_pdp = NULL;
	poid_t			*a_pdp = NULL;
	poid_t			*b_pdp = NULL;
	poid_t			*action_pdp = NULL;
	pin_decimal_t           *fixed_amt = NULL;
        pin_decimal_t           *percent = NULL;
        pin_decimal_t           *latefee_fixed_amt = NULL;
        pin_decimal_t           *latefee_percent_amt = NULL;
        pin_decimal_t           *late_fee = NULL;
	int32                   option = 0;
        int32                   currency_src = 0;
	int32			currency_dst = 0;
	char 			*action_name = NULL;
	pin_decimal_t   	*denominator = NULL;
	time_t			now_t = pin_virtual_time((time_t *)NULL);

	// MSO Customization Starts Here
	pin_decimal_t   	*days = NULL;
	pin_decimal_t   	*total_days = NULL;
	
	days = pbo_decimal_from_str("5.0", ebufp);
	total_days = pbo_decimal_from_str("365.0", ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Custom Late Fee Calculation");
	// MSO Customization Ends HEre

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_collections_pol_calc_late_fees input flist",
		in_flistp);

        *ret_flistpp = PIN_FLIST_CREATE(ebufp);

	action_pdp = (poid_t *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 
			0, ebufp);

	a_pdp = (poid_t *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ,
			0, ebufp);

	b_pdp = (poid_t *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_BILLINFO_OBJ,
			0, ebufp);


	/* Get the action config object */
        i_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_POID, (void *)action_pdp, ebufp);
        PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CONFIG_ACTION_OBJ, NULL, ebufp);


        PCM_OP(ctxp, PCM_OP_READ_FLDS, PCM_OPFLG_CACHEABLE, i_flistp,
                        &r_flistp, ebufp);


        ca_pdp = (poid_t *)PIN_FLIST_FLD_GET(r_flistp,
                        PIN_FLD_CONFIG_ACTION_OBJ, 0, ebufp);

        ca_flistp = fm_collections_cache_get_action(ctxp, ca_pdp, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_collections_pol_calc_late_fees read ca_flist", ca_flistp);

        action_name = (char *)PIN_FLIST_FLD_GET(ca_flistp,
                        PIN_FLD_ACTION_NAME, 0, ebufp);

        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ACTION_NAME,(void*)action_name, 
				ebufp);

        /* Get the late fee info substructure */
        lf_flistp = PIN_FLIST_SUBSTR_GET(ca_flistp, PIN_FLD_LATE_FEE_INFO, 0,
                                ebufp);

        fixed_amt = (pin_decimal_t *)PIN_FLIST_FLD_GET(lf_flistp,
                                PIN_FLD_AMOUNT, 0, ebufp);

        percent = (pin_decimal_t *)PIN_FLIST_FLD_GET(lf_flistp,
                                PIN_FLD_PERCENT, 0, ebufp);

        vp = PIN_FLIST_FLD_GET(lf_flistp, PIN_FLD_OPTIONS, 0, ebufp);
        if (vp) {
                option = *(int32 *)vp;
        }

        vp = PIN_FLIST_FLD_GET(lf_flistp, PIN_FLD_CURRENCY, 0, ebufp);
        if (vp) {
                currency_src = *(int32 *)vp;
        }

	PIN_FLIST_DESTROY_EX(&i_flistp, NULL);

        if ( PIN_ERR_IS_ERR( ebufp )) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_collections_pol_apply_late_fee LATE_FEE_INFO "
                        "not found ", ebufp);
                goto cleanup;
        }


        /* Read the currency from billinfo object if not on the input flist*/
        vp  = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_CURRENCY, 1, ebufp);

	if(vp) {
        	currency_dst  = *(int32 *)vp;
	}

        if (!currency_dst ) {
                currency_dst = fm_collections_pol_get_currency(ctxp, b_pdp, ebufp);
        }

        /* Now calculate the late fee */
        if ((pbo_decimal_is_null(fixed_amt, ebufp)
                                || pbo_decimal_is_zero(fixed_amt, ebufp))
                                && (pbo_decimal_is_null(percent, ebufp)
                                || pbo_decimal_is_zero(percent, ebufp))) {
                /* Latefee is not charged */
                goto cleanup;
                /***********/
        }

        /* Get fixed amount */
        if (pbo_decimal_is_null(fixed_amt, ebufp)
                                || pbo_decimal_is_zero(fixed_amt, ebufp)) {
                latefee_fixed_amt = pbo_decimal_from_str("0.0", ebufp);
        } else {
                latefee_fixed_amt = pbo_decimal_copy(fixed_amt, ebufp);
        }

        /* Calculate the percentage charge */
        if (pbo_decimal_is_null(percent, ebufp)
                                || pbo_decimal_is_zero(percent, ebufp)) {
                latefee_percent_amt = pbo_decimal_from_str("0.0", ebufp);
        } else {
                vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_DUE, 0, ebufp);
		if(vp && percent) {
                	latefee_percent_amt = pbo_decimal_multiply((pin_decimal_t *)vp, percent,
                        ebufp);
					//MSO Customization Starts Here
//					days = pbo_decimal_from_double(5.0, ebufp);
//					total_days = pbo_decimal_from_double(365.0, ebufp);
					/*  Late Fee prorated on day basis, considering 5 days from due date 
					 * and total days as 365 in a year.
					 * Hence Late Fee = Calculated Amount * 5 days / 365 days  
					 */					
					pbo_decimal_multiply_assign(latefee_percent_amt, days, ebufp);
					pbo_decimal_divide_assign(latefee_percent_amt, total_days, ebufp);
					//MSO Customization Ends Here
		}
                denominator = pbo_decimal_from_double(100.0, ebufp);
                pbo_decimal_divide_assign(latefee_percent_amt, denominator, ebufp);
                pbo_decimal_destroy(&denominator);
        }

        /* Get the final late fee charge */
        switch (option) {
                case PIN_OPTION_MIN:
                        late_fee = pbo_decimal_copy(
                                pbo_decimal_compare(latefee_fixed_amt, latefee_percent_amt, ebufp)
                                        < 0 ? latefee_fixed_amt : latefee_percent_amt, ebufp);
                        break;
                case PIN_OPTION_MAX:
                        late_fee = pbo_decimal_copy(
                                pbo_decimal_compare(latefee_fixed_amt, latefee_percent_amt, ebufp)
                                        < 0 ? latefee_percent_amt : latefee_fixed_amt, ebufp);
                        break;
                default: /* PIN_OPTION_BOTH/PIN_OPTION_EITHER */
                        late_fee = pbo_decimal_add(latefee_fixed_amt, latefee_percent_amt, ebufp);
        }

	pbo_decimal_destroy(&latefee_fixed_amt);
        pbo_decimal_destroy(&latefee_percent_amt);

        /* For later use */
        PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		
        /* Convert the latefee from source currency to dest currency */
        if (currency_src == currency_dst || !currency_src) {
                /* The latefee is for currency_dst */
        } else { /* Do a conversion */
                PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
                i_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_POID, (void *)a_pdp, ebufp);
                PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CURRENCY_SRC,
                                        (void *)&currency_src, ebufp);
                PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CURRENCY_DST,
                                        (void *)&currency_dst, ebufp);
                PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_END_T, (void *)&now_t,
                                        ebufp);
                t_flistp = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_AMOUNTS,
                                        0, ebufp);
                PIN_FLIST_FLD_PUT(t_flistp, PIN_FLD_AMOUNT_SRC,
                                        (void *)late_fee, ebufp);

                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_collections_pol_apply_late_fee convert currency input "
                        "flist", i_flistp);

                PCM_OP(ctxp, PCM_OP_BILL_CURRENCY_CONVERT_AMOUNTS, 0, i_flistp,
                                &r_flistp, ebufp);

		PIN_FLIST_DESTROY_EX(&i_flistp, NULL);

                if ( PIN_ERR_IS_ERR( ebufp )) {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "fm_collections_pol_apply_late_fee currency "
                                "convert error", ebufp);
                        goto cleanup;
                }

                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_collections_pol_apply_late_fee convert currency result "
                        "flist", r_flistp);
                t_flistp = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_AMOUNTS,
                                PIN_ELEMID_ANY, 0, ebufp);
                late_fee = (pin_decimal_t *)PIN_FLIST_FLD_TAKE(t_flistp,
                                PIN_FLD_AMOUNT_DST, 0, ebufp);
        }
        PIN_FLIST_FLD_PUT(*ret_flistpp, PIN_FLD_AMOUNT,(void*)late_fee, ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_CURRENCY,(void*)&currency_dst, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_collections_pol_calc_late_fees result  flist",
		*ret_flistpp);
cleanup:
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&ca_flistp, NULL);
        pbo_decimal_destroy(&latefee_fixed_amt);
        pbo_decimal_destroy(&latefee_percent_amt);
       
}

/*******************************************************************
 * fm_collections_pol_get_currency()
 * This function retrieves the currency for the billinfo.
 *******************************************************************/

int32
fm_collections_pol_get_currency(
	pcm_context_t		*ctxp, 
	poid_t			*b_pdp, 
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*bi_flistp = NULL;
	pin_flist_t		*br_flistp = NULL;
	void			*vp = NULL;
	int32			currency = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return 0;
	PIN_ERR_CLEAR_ERR(ebufp);
	/* Get the billinfo currency */
	bi_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_POID, (void *)b_pdp, 
			ebufp);
	PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_CURRENCY, NULL, ebufp);


	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, bi_flistp, 
			&br_flistp, ebufp);

	if ( PIN_ERR_IS_ERR( ebufp )) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_collections_pol_get_currency read error", 
			ebufp); 
		goto cleanup; 
	}

	vp = PIN_FLIST_FLD_GET(br_flistp, PIN_FLD_CURRENCY, 0, ebufp);
	if (vp) {
		currency =  *(int32 *)vp;
	} 
cleanup:
	PIN_FLIST_DESTROY_EX(&bi_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&br_flistp, NULL);
	return currency;
}
