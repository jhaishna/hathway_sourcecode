/*
* Copyright (c) 2001, 2016, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 */

#ifndef lint
static char Sccs_id[] = "@(#)$Id: fm_collections_pol_prep_dunning_data.c /cgbubrm_2.0.0.collections/1 2016/07/08 10:15:49 bmaturi Exp $";
#endif

#include <stdio.h>
#include <string.h>

#include <pcm.h>
#include <pinlog.h>

#include "cm_fm.h"
#include "cm_cache.h"
#include "ops/collections.h"
#include "fm_utils.h"
#include "fm_bill_utils.h"
#include "fm_collections.h"

#ifdef MSDOS
#include <WINDOWS.h>
#endif


/*******************************************************************
 * Routines defined here.
 *******************************************************************/

EXPORT_OP void
op_collections_pol_prep_dunning_data(
	cm_nap_connection_t	*connp, 
	int32			opcode, 
	int32			flags, 
	pin_flist_t		*in_flistp, 
	pin_flist_t		**ret_flistpp, 
	pin_errbuf_t		*ebufp);

static void
fm_collections_pol_prep_dunning_data(
	pcm_context_t		*ctxp, 
	int32			flags, 
	pin_flist_t		*in_flistp, 
	pin_flist_t		**ret_flistpp, 
	pin_errbuf_t		*ebufp);

static void
fm_collections_pol_get_account_info(
        pcm_context_t           *ctxp,
        poid_t                  *a_pdp,
	poid_t                  *b_pdp,
        pin_flist_t             **ret_flistpp, 
        pin_errbuf_t            *ebufp);

void
fm_collections_pol_read_billinfo_payinfo_address(
        pcm_context_t           *ctxp,
        poid_t 			*pay_pdp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);


/*******************************************************************
 * Variables/functions defined elsewhere
 *******************************************************************/
/*******************************************************************
 * Main routine for the PCM_OP_COLLECTIONS_POL_PREP_DUNNING_DATA 
 *******************************************************************/
void
op_collections_pol_prep_dunning_data(
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
	if (opcode != PCM_OP_COLLECTIONS_POL_PREP_DUNNING_DATA) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_collections_pol_prep_dunning_data opcode error", 
			ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_collections_pol_prep_dunning_data input flist", in_flistp);

	/***********************************************************
	 * Do the actual op in a sub. Copy it since we may need to
	 * replace it later.
	 ***********************************************************/

	fm_collections_pol_prep_dunning_data(ctxp, flags, in_flistp, 
			ret_flistpp, ebufp);

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_collections_pol_prep_dunning_data error", ebufp);
	} else {
		/***************************************************
		 * Debug: What we're sending back.
		 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_collections_pol_prep_dunning_data return flist", 
			*ret_flistpp);
	}
	return;
}

/*******************************************************************
 * fm_collections_pol_prep_dunning_data()
 *
 * This function reads account and creates dunning flist.
 *
 *******************************************************************/
static void
fm_collections_pol_prep_dunning_data(
	pcm_context_t		*ctxp, 
	int32			flags, 
	pin_flist_t		*in_flistp, 
	pin_flist_t		**ret_flistpp, 
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*tmp_fm_utils_init_cfg_beid = NULL;
	poid_t			*a_pdp = NULL;
	poid_t			*b_pdp = NULL;
	void			*vp = NULL;
	pin_flist_t             *t_flistp = NULL;
	int32			currency = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/* Prepare the return flist */
	*ret_flistpp = PIN_FLIST_COPY(in_flistp, ebufp);

	a_pdp = (poid_t *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ,
			0, ebufp);

	b_pdp = (poid_t *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_BILLINFO_OBJ,
			0, ebufp);
	fm_collections_pol_get_account_info(ctxp, a_pdp,b_pdp, &r_flistp, ebufp);

	if ( PIN_ERR_IS_ERR( ebufp )) { 
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_collections_pol_prep_dunning_data error", ebufp); 
		goto cleanup; 
		/***********/
	}

	PIN_FLIST_ELEM_PUT(*ret_flistpp, r_flistp, PIN_FLD_ACCTINFO, 0, 
		ebufp);
	r_flistp = NULL;

	/* Add currency elem to the return flist */
	vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_CURRENCY, 1, ebufp);
	if (vp) {
		currency = *(int32 *)vp;
	} 
	if (!currency) {
		t_flistp = PIN_FLIST_ELEM_GET(*ret_flistpp, 
			PIN_FLD_ACCTINFO, 0, 0, ebufp);
		vp = PIN_FLIST_FLD_GET(t_flistp, PIN_FLD_CURRENCY, 0, 
				ebufp);
		if (vp) {
			currency = *(int32 *)vp;
		}
	}

	tmp_fm_utils_init_cfg_beid = fm_bill_utils_cfg_get_beid(ctxp, ebufp);
	t_flistp = PIN_FLIST_ELEM_GET(tmp_fm_utils_init_cfg_beid, 
			PIN_FLD_BALANCES, currency, 0, ebufp);

	if ( PIN_ERR_IS_ERR( ebufp )) { 
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_collections_pol_prep_dunning_data cannot find "
			"currency info", ebufp); 
		goto cleanup; 
		/***********/
	}
	PIN_FLIST_ELEM_SET(*ret_flistpp, t_flistp, 
			PIN_FLD_CURRENCIES, currency, ebufp);
cleanup:
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
}

/*******************************************************************
 * fm_collections_pol_get_account_info()
 *
 * This function reads the information from /account 
 * if the pay_type is invoice ,it reads ADDRESS...etc from payinfo/invoice
 *******************************************************************/

static void
fm_collections_pol_get_account_info(
        pcm_context_t           *ctxp,
        poid_t                  *a_pdp,
        poid_t                  *b_pdp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *r_flistp = NULL;
        pin_flist_t             *br_flistp = NULL;
        pin_flist_t             *i_flistp = NULL;
        pin_flist_t             *t_flistp = NULL;
        pin_flist_t             *invinfo_flistp = NULL;
        pin_flist_t             *nameinfo_flistp = NULL;
        void                    *vp = NULL;
        poid_t                  *pay_pdp = NULL;
        char                    *pay_type = NULL;


        if (PIN_ERR_IS_ERR(ebufp))
                return;
        PIN_ERR_CLEAR_ERR(ebufp);

        i_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_POID, (void *)b_pdp, ebufp);
        PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_PAY_TYPE, NULL, ebufp);
        PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CURRENCY, NULL, ebufp);
        PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CURRENCY_SECONDARY, NULL, ebufp);
        PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_PAYINFO_OBJ, NULL, ebufp);


        PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, i_flistp, &br_flistp, ebufp);

        if ( PIN_ERR_IS_ERR( ebufp )) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_collections_pol_get_account_info read billinfo error",
                        ebufp);
                goto cleanup;
                /***********/
        }


        pay_pdp = (poid_t*)PIN_FLIST_FLD_TAKE(br_flistp,PIN_FLD_PAYINFO_OBJ,
			0 ,ebufp);

	if (!PIN_POID_IS_NULL(pay_pdp)) {
        	pay_type = (char*)PIN_POID_GET_TYPE(pay_pdp);
	}


        /* Read the account object */
        i_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_POID, (void *)a_pdp, ebufp);
        PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_ACCOUNT_NO, NULL, ebufp);
        PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_LOCALE, NULL, ebufp);
	t_flistp = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_NAMEINFO, 
			PIN_ELEMID_ANY , ebufp);

        PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_FIRST_NAME, NULL, ebufp);
        PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_MIDDLE_NAME, NULL, ebufp);
        PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_LAST_NAME, NULL, ebufp);

        PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_CONTACT_TYPE, NULL, ebufp);
        PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_SALUTATION, NULL, ebufp);
        PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_TITLE, NULL, ebufp);
        PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_COMPANY, NULL, ebufp);

        if (pay_type && strcasecmp(pay_type, PIN_OBJ_TYPE_PAYINFO_INVOICE) != 0) {
                PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_EMAIL_ADDR, NULL, ebufp);
                PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_ADDRESS, NULL, ebufp);
                PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_CITY, NULL, ebufp);
                PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_STATE, NULL, ebufp);
                PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_ZIP, NULL, ebufp);
                PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_COUNTRY, NULL, ebufp);
        }


        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_collections_pol_get_account_info input flist", i_flistp);

        PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, i_flistp, &r_flistp, ebufp);

        if ( PIN_ERR_IS_ERR( ebufp )) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_collections_pol_get_account_info read error",
                        ebufp);
                goto cleanup;
                /***********/
        }
        vp = PIN_FLIST_FLD_TAKE(r_flistp,PIN_FLD_POID,0 ,ebufp);
        PIN_FLIST_FLD_PUT(r_flistp, PIN_FLD_ACCOUNT_OBJ, vp, ebufp);

        vp = PIN_FLIST_FLD_TAKE(br_flistp,PIN_FLD_PAY_TYPE,0 ,ebufp);
        PIN_FLIST_FLD_PUT(r_flistp, PIN_FLD_PAY_TYPE, vp, ebufp);

        vp = PIN_FLIST_FLD_TAKE(br_flistp,PIN_FLD_POID,0 ,ebufp);
        PIN_FLIST_FLD_PUT(r_flistp, PIN_FLD_POID, vp, ebufp);

        vp = PIN_FLIST_FLD_TAKE(br_flistp,PIN_FLD_CURRENCY,0 ,ebufp);
        PIN_FLIST_FLD_PUT(r_flistp,PIN_FLD_CURRENCY,vp ,ebufp);

        vp = PIN_FLIST_FLD_TAKE(br_flistp,PIN_FLD_CURRENCY_SECONDARY,0,
			ebufp);
        PIN_FLIST_FLD_PUT(r_flistp,PIN_FLD_CURRENCY_SECONDARY,vp ,ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_collections_pol_get_account_info result flist", 
			r_flistp);

        PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&br_flistp, NULL);

        if (pay_type && !strcasecmp(pay_type, PIN_OBJ_TYPE_PAYINFO_INVOICE) ) {

		fm_collections_pol_read_billinfo_payinfo_address(ctxp,pay_pdp,
                        &invinfo_flistp,ebufp);


                nameinfo_flistp = PIN_FLIST_ELEM_GET(r_flistp,PIN_FLD_NAMEINFO , 
					PIN_ELEMID_ANY, 0, ebufp);


                PIN_FLIST_CONCAT(nameinfo_flistp, invinfo_flistp,  ebufp);


                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "After adding invoice address r_flistp  ", r_flistp);
        }


        *ret_flistpp = r_flistp;
cleanup:
        PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&br_flistp, NULL);
}

/*******************************************************************
 * fm_collections_pol_get_account_info()
 * For the pay_type invoice ,it reads ADDRESS...etc from payinfo/invoice
 *******************************************************************/

void
fm_collections_pol_read_billinfo_payinfo_address(
        pcm_context_t           *ctxp,
        poid_t 			*pay_pdp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *s_flistp = NULL;
        pin_flist_t             *s1_flistp = NULL;
        pin_flist_t             *r_flistp = NULL;
        pin_flist_t             *invinfo_flistp = NULL;


        if (PIN_ERR_IS_ERR(ebufp))
                return;
        PIN_ERR_CLEAR_ERR(ebufp);


        s_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_POID, (void *)pay_pdp, ebufp);
        s1_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_INV_INFO, 
			PIN_ELEMID_ANY, ebufp);

        PIN_FLIST_FLD_SET(s1_flistp, PIN_FLD_ADDRESS, "", ebufp);
        PIN_FLIST_FLD_SET(s1_flistp, PIN_FLD_CITY, "", ebufp);
        PIN_FLIST_FLD_SET(s1_flistp, PIN_FLD_COUNTRY, "", ebufp);
        PIN_FLIST_FLD_SET(s1_flistp, PIN_FLD_STATE, "", ebufp);
        PIN_FLIST_FLD_SET(s1_flistp, PIN_FLD_ZIP, "", ebufp);
	PIN_FLIST_FLD_SET(s1_flistp, PIN_FLD_EMAIL_ADDR, "", ebufp);


        PCM_OP(ctxp, PCM_OP_READ_FLDS, PCM_OPFLG_CACHEABLE, s_flistp,
                &r_flistp, ebufp);

        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);

        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "Error in reading objs from /payinfo ",
                        ebufp);
                goto cleanup;
        }


        invinfo_flistp = PIN_FLIST_ELEM_TAKE(r_flistp,
        		PIN_FLD_INV_INFO, PIN_ELEMID_ANY, 0, ebufp);

        *ret_flistpp = invinfo_flistp;
cleanup:
        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
}
