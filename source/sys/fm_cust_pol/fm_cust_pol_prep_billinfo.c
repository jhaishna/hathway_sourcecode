/*******************************************************************
 *
* Copyright (c) 1999, 2019, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static char Sccs_id[] = "@(#)$Id: fm_cust_pol_prep_billinfo.c /cgbubrm_7.5.0.portalbase/2 2016/03/10 02:26:43 anirbisw Exp $";
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pin_bill.h"
#include "pin_pymt.h"
#include "pin_currency.h"
#include "psiu_currency.h"
#include "pinlog.h"
#include "pin_os.h"
#include "fm_utils.h"
#include "fm_bill_utils.h"
#include "psiu_business_params.h"
#include "fm_utils_bparams_cache.h"

/********************************************************************
 * Global Variable for cache and structure for dom allocation using 
 * process time by weighted average method
 ********************************************************************/

typedef struct dom_map_entry{
	int32		dom;
	int32		bs;
	int32		status;
	int32		num_acc;
	int32		max_acc;
	int32		num_srv;
	int32		max_srv;
	int32		process_t;
} dom_map_t;

/*******************************************************************/

EXPORT_OP void
op_cust_pol_prep_billinfo(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_cust_pol_prep_billinfo(
	pcm_context_t	*ctxp,
	int32		flags,
	pin_flist_t	*in_flistp,
	pin_flist_t	**out_flistpp,
	pin_errbuf_t	*ebufp);
 
static void
fm_cust_pol_prep_billinfo_bill_when(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp, 
	pin_flist_t	*bi_flistp,
	pin_flist_t	**bis_flistpp, 
	pin_errbuf_t	*ebufp);
 
static void
fm_cust_pol_prep_billinfo_dom(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp, 
	pin_flist_t	*bi_flistp, 
	pin_flist_t	**bis_flistpp, 
	int32		is_creation,
	pin_errbuf_t	*ebufp);

static void
fm_cust_pol_prep_billinfo_actg_type(
	pin_flist_t	*bi_flistp, 
	pcm_context_t   *ctxp,
	pin_errbuf_t	*ebufp);

static void
fm_cust_pol_prep_billinfo_next_future(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp, 
	pin_flist_t	*bi_flistp, 
	int32		is_creation,
	pin_errbuf_t	*ebufp);

static void
fm_cust_pol_prep_billinfo_cycle_length(
	pcm_context_t    *ctxp,
	pin_flist_t	*bi_flistp, 
	pin_errbuf_t	*ebufp);

static void 
fm_cust_pol_prep_billinfo_read(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	*bi_flistp,
	pin_flist_t	**bis_flistpp,
	pin_errbuf_t	*ebufp);

static int32
fm_cust_pol_prep_billinfo_get_next_dom_from_cache(
	dom_map_t	*dom_map_cache,
	int32		today,
	int32		num_elem,
	pin_errbuf_t	*ebufp);

int32
fm_cust_pol_prep_billinfo_get_dom_from_process_t_from_cache(
	dom_map_t	*dom_map_cache,
	int32		today,
	int32		num_elem,
	pin_errbuf_t	*ebufp);

int32
fm_cust_pol_prep_billinfo_get_rand_no(
	int32		num);

PIN_IMPORT int fm_utils_cycle_get_dom(
	time_t		a_time,
	pin_errbuf_t	*ebufp);

/*******************************************************************
 * Routine from elsewhere
 *******************************************************************/
extern pin_flist_t *
fm_cust_utils_get_config_dom_map_cache(
	pcm_context_t   *ctxp,
	int64		database,
	pin_errbuf_t    *ebufp);

extern int32
fm_bill_utils_get_31day_billing_opt(
	pin_errbuf_t	*ebufp);
/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_PREP_BILLINFO  command
 *******************************************************************/
void
op_cust_pol_prep_billinfo(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t	*ctxp = connp->dm_ctx;
	pin_flist_t	*r_flistp = NULL;
	void		*vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	*ret_flistpp = NULL;

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_CUST_POL_PREP_BILLINFO) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_cust_pol_prep_billinfo", ebufp);
		return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_cust_pol_prep_billinfo input flist", i_flistp);

	/***********************************************************
	 * Call main function to do it
	 ***********************************************************/
	fm_cust_pol_prep_billinfo(ctxp, flags, i_flistp, &r_flistp, ebufp);

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_cust_pol_prep_billinfo error", ebufp);
	} else {
		*ret_flistpp = PIN_FLIST_CREATE(ebufp);
		vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_POID, vp, ebufp);
		PIN_FLIST_ELEM_PUT(*ret_flistpp, r_flistp, PIN_FLD_BILLINFO, 0,
									ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_prep_billinfo return flist", *ret_flistpp);
	}
	return;
}

/*******************************************************************
 * fm_cust_pol_prep_billinfo()
 *******************************************************************/
static void
fm_cust_pol_prep_billinfo(
	pcm_context_t	*ctxp,
	int32		flags,
	pin_flist_t	*i_flistp,
	pin_flist_t	**out_flistpp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*bi_flistp = NULL;
	pin_flist_t	*tmp_flistp = NULL;
	pin_flist_t	*bis_flistp = NULL;
	int32		is_creation = PIN_BOOLEAN_FALSE;
	int32		future_dom = 0;
	int32		i_future_dom = 0;
	int32		pay_type = PIN_PAY_TYPE_UNDEFINED;
	pin_cookie_t	cookie = NULL;
	int32		elemid = 0;
	int32		dom_advanced = PIN_BILL_FLG_DOM_NOT_INCREMENTED;
	poid_t		*bi_pdp = NULL;
	void		*vp = NULL;
	time_t		end_t = (time_t)0;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);       

	tmp_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_BILLINFO,
						&elemid, 0, &cookie, ebufp);
	bi_flistp = PIN_FLIST_COPY(tmp_flistp, ebufp);

	bi_pdp = (poid_t *)PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_POID, 1, ebufp);
	if (!bi_pdp) {
		PIN_FLIST_DESTROY_EX(&bi_flistp, NULL);
		return;
	}
	if (PIN_POID_GET_ID(bi_pdp) <= 0) {
		is_creation = PIN_BOOLEAN_TRUE;
	}

	/***********************************************************
	 * Calculate ACTG_TYPE
	 ***********************************************************/
	if (is_creation) {
		fm_cust_pol_prep_billinfo_actg_type(bi_flistp,ctxp, ebufp);
	}

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 1, ebufp);
	if (!vp) {
		end_t = pin_virtual_time((time_t *)NULL);
		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_END_T, (void *)&end_t, ebufp);
	}

	/***********************************************************
	 * Save initial FUTURE_DOM
	 ***********************************************************/
	vp = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_ACTG_FUTURE_DOM, 1, ebufp);
	if (vp) {
		i_future_dom = *(int32 *)vp;
	}

	/***********************************************************
	 * Calculate ACTG_CYCLE_DOM
	 ***********************************************************/
	fm_cust_pol_prep_billinfo_dom(ctxp, i_flistp, bi_flistp, &bis_flistp, 
							is_creation, ebufp);

	/***********************************************************
	 * Calculate ACTG_LAST_T and LAST_BILL_T
	 ***********************************************************/
	if (is_creation) {
		vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 0, ebufp);      
		if (vp) {
			end_t = fm_utils_time_round_to_midnight(*((time_t *)vp));
			PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_ACTG_LAST_T, 
				(void *)&end_t, ebufp);
			PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_LAST_BILL_T, 
				(void *)&end_t, ebufp);
		}
	}

	/***********************************************************
	 * Calculate ACTG_NEXT_T and ACTG_FUTURE_T
	 ***********************************************************/
	if (is_creation || i_future_dom) {
		fm_cust_pol_prep_billinfo_next_future(ctxp, i_flistp, bi_flistp,
							is_creation, ebufp);
	}

	/***********************************************************
	 * Increase future_dom in the case of advance billing
	 * The operation is not required if DOM is already 
	 * incremented. 
	 ***********************************************************/
	if (pin_conf_advance_bill_cycle && (is_creation || i_future_dom)) {
		vp = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_FLAGS, 1, ebufp);
		if (vp) {
			dom_advanced = *(int32 *)vp;
			PIN_FLIST_FLD_DROP(bi_flistp, PIN_FLD_FLAGS, ebufp);
		}

		vp = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_ACTG_FUTURE_DOM, 0, 
									ebufp);
		if (vp && 
			(dom_advanced == PIN_BILL_FLG_DOM_NOT_INCREMENTED)) {
			future_dom = *(int32 *)vp;
			future_dom++;
			PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_ACTG_FUTURE_DOM,
						(void *)&future_dom, ebufp);
		}
	}

	/***********************************************************
	 * Calculate BILL_WHEN
	 ***********************************************************/
	if (is_creation) {
		fm_cust_pol_prep_billinfo_bill_when(ctxp, i_flistp, bi_flistp, 
							&bis_flistp, ebufp);
	}

	/********************************************************************
	 * if PAYINFO_OBJ is not passed, then set it to 0
	 * for PAY_TYPE = UNDEFINED, SUBORD, or VOUCHER
	 ********************************************************************/
	vp = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_PAY_TYPE, 1, ebufp);
	if (vp) {
		pay_type = *(int32 *)vp;
		vp = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_PAYINFO_OBJ, 1,ebufp);
		if (!vp &&
			((pay_type == PIN_PAY_TYPE_UNDEFINED) ||
			/**** Pavan Bellala 14-08-2015 **********
			Bug id :1062 
			Commented below line to allow payinfo object for
			subordinate account
			*************************************************
			(pay_type == PIN_PAY_TYPE_SUBORD) || **/
			(pay_type == PIN_PAY_TYPE_VOUCHER))) {
			vp = (void *)PIN_POID_CREATE(0, "", 0, ebufp);
			PIN_FLIST_FLD_PUT(bi_flistp, PIN_FLD_PAYINFO_OBJ, vp,
									ebufp);
		}
	}

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_prep_billinfo error", ebufp);
	}

	PIN_FLIST_DESTROY_EX(&bis_flistp, NULL);
	*out_flistpp = bi_flistp;
	return;
}

/*******************************************************************
 * fm_cust_pol_prep_billinfo_actg_type()
 *
 * Calculates the accounting type for the billinfo from input 
 * flist or from the pin.conf file. If it is not found in both
 * these places, the default(PIN_ACTG_TYPE_BALANCE_FORWARD) is used.
 *
 * @param bi_flistp :		billinfo flist.
 * @param ebufp     :		error buffer.
 *
 *******************************************************************/
static void
fm_cust_pol_prep_billinfo_actg_type(
	pin_flist_t	*bi_flistp,
	pcm_context_t	*ctxp,
        pin_errbuf_t    *ebufp)
{
        int32           actg_type = PIN_ACTG_TYPE_BALANCE_FORWARD;
        int32           *actg_typep = NULL;
	int32           err = 0;
	char		actg_type_conf[4] = {0};

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

        /***********************************************************
         * Set the Accounting type for this billinfo. The accounting
         * type should be either PIN_ACTG_TYPE_OPEN_ITEMS or
         * PIN_ACTG_TYPE_BALANCE_FORWARD. First check the input
         * flist, if not specified in the input flist take the
         * one from the pin.conf file. If not specified in both
         * places, use the default value.
         ***********************************************************/
        actg_typep = (int32 *) PIN_FLIST_FLD_GET(bi_flistp, 
			PIN_FLD_ACTG_TYPE, 1, ebufp);
        if (actg_typep && (*actg_typep != 0)) {
                actg_type = *actg_typep;
        } else {
                pin_conf("fm_cust_pol", "actg_type", PIN_FLDT_INT,
                        (caddr_t *)&actg_typep, &(err));

                if (actg_typep != (int32*) NULL) {
                        actg_type = *actg_typep;
			pin_snprintf(actg_type_conf,sizeof(actg_type_conf),"%d",actg_type);
			fm_utils_bparam_cache_update_from_pinconf(PSIU_BPARAMS_BILLING_PARAMS,
				PSIU_BPARAMS_ACTG_TYPE,actg_type_conf,ebufp);
                        free(actg_typep);
                }
		else {
			actg_type = fm_utils_bparams_cache_get_int(ctxp, PSIU_BPARAMS_BILLING_PARAMS,
				PSIU_BPARAMS_ACTG_TYPE, 0, ebufp);
		}
        }

        PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_ACTG_TYPE, (void *)&actg_type, ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_prep_billinfo_actg_type error", ebufp);
	}
	return;
}

/*******************************************************************
 * fm_cust_pol_prep_billinfo_bill_when()
 *******************************************************************/
static void
fm_cust_pol_prep_billinfo_bill_when(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	*bi_flistp,
	pin_flist_t	**bis_flistpp,
	pin_errbuf_t	*ebufp)
{
	void		*vp = NULL;
	int32		*billwhenp = NULL;
	int32		err = 0;
	int32		billwhen = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);
	if(bi_flistp){

	billwhenp = (int32 *)PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_BILL_WHEN, 1,
									ebufp);
	}
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_cust_pol_prep_billinfo_bill_when input flistp", i_flistp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_cust_pol_prep_billinfo_bill_when bi_flistp", bi_flistp);


	
	if (billwhenp) {
		billwhen = *billwhenp;
	} else {
		/***********************************************************
		* Try to get bill_when from other billinfo
		************************************************************/
		if (!(*bis_flistpp)) {
			fm_cust_pol_prep_billinfo_read(ctxp, i_flistp, 
						bi_flistp, bis_flistpp, ebufp);
		}
		billwhenp = (int32 *)PIN_FLIST_FLD_GET(*bis_flistpp, 
						PIN_FLD_BILL_WHEN, 1, ebufp);
		if (billwhenp) {
			billwhen = *billwhenp;
		} else {
			/******************************************************
			* Try to get bill_when from pin_conf
			*******************************************************/
			pin_conf("fm_cust_pol", "bill_when", PIN_FLDT_INT,
				(caddr_t *)&billwhenp, &(err));
			if (billwhenp) {
				billwhen = *billwhenp;
				free(billwhenp);
			}
		}
	}

	/***************************************************************
	 * If not configured, enforce billing every accounting cycle.
	 ***************************************************************/
	if (billwhen == 0) {
		billwhen = 1;
	}
	PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_BILL_WHEN, (void *)&billwhen, 
									ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_prep_billinfo_bill_when error", ebufp);
	}
	return;
}

/*******************************************************************
 * fm_cust_pol_prep_billinfo_dom()
 *
 * Set the accounting cycle day-of-month for the billinfo object
 *
 *******************************************************************/
static void
fm_cust_pol_prep_billinfo_dom(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp, 
	pin_flist_t	*bi_flistp, 
	pin_flist_t	**bis_flistpp,
	int32		is_creation,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*cache_flistp = NULL;
	pin_flist_t	*flistp = NULL;
	pin_cookie_t	cookie = NULL;
	poid_t		*pdp = NULL;
	dom_map_t	*dom_map_cache = NULL;

	int64		database = 0;
	int32		recid = 0;
	int32		count = 0;
	int32		err = 0;
	int32		*domp = NULL;
	int32		*future_domp = NULL;
	int32		*billing_segmentp = NULL;
	int32		cycle_dom = 0;
	int32		future_dom = 0;
	int32		billing_segment = 0;
	int32		bs = 0;
	int32		status = 0;
	int32		today = 0;
	int32		is_adjust = PIN_BOOLEAN_FALSE;
	int32		set_dom = PIN_BOOLEAN_FALSE;
	int32		dom_advanced = PIN_BILL_FLG_DOM_NOT_INCREMENTED;
	int32		move_bill_day_forward = 0;
	time_t		end_t = 0;
	char		*key = NULL;
	void		*vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	pdp = PCM_GET_USERID(ctxp);
        database = PIN_POID_GET_DB(pdp);
        PIN_POID_DESTROY(pdp, NULL);

	/***************************************************************
	 * Calculate future_dom
	 ***************************************************************/
	future_domp = (int32*)PIN_FLIST_FLD_GET(bi_flistp, 
					PIN_FLD_ACTG_FUTURE_DOM, 1, ebufp);
	domp = (int32*)PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_ACTG_CYCLE_DOM, 1,
									ebufp);
	if (domp) {
		cycle_dom =  *domp;
	}
	/***************************************************************
	 * Get the Billing Segment
	 ***************************************************************/
	billing_segmentp = (int32 *) PIN_FLIST_FLD_GET(bi_flistp, 
					PIN_FLD_BILLING_SEGMENT, 1, ebufp);
	if(billing_segmentp) {
		billing_segment = *billing_segmentp;
	}

	/***********************************************
	* Get future_dom from end_t or virtual time
	***********************************************/
	vp =(time_t*)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 1, ebufp);
	if (vp) {
		end_t = *(time_t *)vp;
	}
	else {
		end_t = pin_virtual_time( (time_t *)NULL);
	}
	today = fm_utils_cycle_get_dom(end_t, ebufp);

	if (future_domp) {
		future_dom = *future_domp;
		is_adjust = PIN_BOOLEAN_TRUE;
	} else {
		if (domp) {
			future_dom = *domp;
		} else {
			/***************************************************************
			 * If Billing Segment is '0' then use original method of DOM 
			 * allocation	
			 ***************************************************************/
			cache_flistp = fm_cust_utils_get_config_dom_map_cache(ctxp, database, ebufp);
			
			if(cache_flistp && billing_segment) {

				/***************************************************************
				* Count the number of elements in the cache flist
				***************************************************************/
				count = PIN_FLIST_ELEM_COUNT(cache_flistp, PIN_FLD_MAP, ebufp);

				if(count) {
					/***************************************************************
					* Allocate the memomry dynamically
					***************************************************************/
					dom_map_cache = (dom_map_t *) 
						malloc(count * sizeof(dom_map_t));	
					if(dom_map_cache == NULL) {
						pin_set_err(ebufp, PIN_ERRLOC_FM, 
							 PIN_ERRCLASS_SYSTEM_DETERMINATE,
							 PIN_ERR_NO_MEM, 0, 0, 0);
						PIN_FLIST_DESTROY_EX(&cache_flistp, NULL);
						return;
					}

					count = 0;
				
					/***************************************************************
					* Scan through the cache flist and fill the data structure
					*
					* Fills only records with the status - PIN_ACTG_DOM_STATUS_OPEN
					* and whose billing segment is same as in the input flist
					***************************************************************/
					while((flistp = PIN_FLIST_ELEM_GET_NEXT(cache_flistp, PIN_FLD_MAP, 
							 &recid, 1, &cookie, ebufp)) != (pin_flist_t *) NULL) {  
						vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_BILLING_SEGMENT, 0, ebufp);
						if(vp) bs = *(int32 *)vp;
						vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_STATUS, 0, ebufp);
						if(vp) status = *(int32 *)vp;
			
						if(bs == billing_segment && status != PIN_ACTG_DOM_STATUS_CLOSE) {
							/* dom_map_cache was checked not be NULL earlier.
							   Otherwise it returns before reaching this point */

							vp = PIN_FLIST_FLD_GET(flistp,
									 PIN_FLD_ACTG_CYCLE_DOM, 0, ebufp);
                                                        if (vp) {
                                                                dom_map_cache[count].dom = *(int32 *)vp;
                                                        }
							dom_map_cache[count].bs = bs;
							dom_map_cache[count].status = status;
							vp = PIN_FLIST_FLD_GET(flistp,
									 PIN_FLD_NUM_ACCOUNTS, 0, ebufp); 
                                                        if (vp) {
                                                                dom_map_cache[count].num_acc = *(int32 *)vp;
                                                        }
							vp = PIN_FLIST_FLD_GET(flistp,
									 PIN_FLD_MAX_ACCOUNTS, 0, ebufp); 
                                                        if (vp) {
                                                                dom_map_cache[count].max_acc = *(int32 *)vp;
                                                        }
							vp = PIN_FLIST_FLD_GET(flistp,
									 PIN_FLD_NUM_SERVICES, 0, ebufp); 
                                                        if (vp) {
                                                                dom_map_cache[count].num_srv = *(int32 *)vp;
                                                        }
							vp = PIN_FLIST_FLD_GET(flistp,
									 PIN_FLD_MAX_SERVICES, 0, ebufp); 
                                                        if (vp) {
                                                                dom_map_cache[count].max_srv = *(int32 *)vp;
                                                        }
							vp = PIN_FLIST_FLD_GET(flistp,
									 PIN_FLD_TOTAL_PROCESS_T, 0, ebufp); 
                                                        if (vp) {
                                                                dom_map_cache[count].process_t = *(int32 *)vp;
                                                        }
							count++;
						}
					}

					if(count == 0) {
						set_dom = PIN_BOOLEAN_TRUE;
					}
					else {
						/******************************************************
						* Get the next future_dom starting from current date
						* i.e; today 
						*******************************************************/
						future_dom = fm_cust_pol_prep_billinfo_get_next_dom_from_cache(
									dom_map_cache, today, count, ebufp);
					}				
					free(dom_map_cache);
				} 
				else {
					set_dom = PIN_BOOLEAN_TRUE;
				}
			}
			else {
				set_dom = PIN_BOOLEAN_TRUE;
			}
			if(set_dom) {
				/******************************************************
				* Try to get future_dom from other billinfo
				*******************************************************/
				if (!(*bis_flistpp)) {
					fm_cust_pol_prep_billinfo_read(ctxp, i_flistp, 
							bi_flistp, bis_flistpp, ebufp);
				}
				domp = (int32 *)PIN_FLIST_FLD_GET(*bis_flistpp,
						PIN_FLD_ACTG_CYCLE_DOM, 1, ebufp);
				if (domp) {
					future_dom = *domp;
				} else {
					is_adjust = PIN_BOOLEAN_TRUE;
					/***********************************************
					* Try to get future_dom from pin.conf
					***********************************************/
					pin_conf("fm_cust_pol", "actg_dom", 
						PIN_FLDT_INT, (caddr_t *)&vp, &(err));
					if (vp) {
						future_dom = *(int32 *)vp;
						free(vp);
					}
					if (future_dom == 0) {
						future_dom = today;
					}
				}
			}
		}
	}

	/*******************************************************
	* Adjust future_dom in order not to use 29-31 days.
	* If future_dom is set to 1 (>=28), in case 
	* advance_bill_cycle = 1 and move_bill_day_forward = 0,
	* no date increment should happen in 
	* PCM_OP_BILL_POL_SPEC_FUTURE_CYCLE.  
	*******************************************************/
	move_bill_day_forward =
		fm_bill_utils_get_31day_billing_opt(ebufp);

	if (is_adjust && (future_dom > 28) &&
		((move_bill_day_forward == 0) ||
			pin_conf_advance_bill_cycle)) {
		future_dom = 1;
		if (pin_conf_advance_bill_cycle) {
			dom_advanced = PIN_BILL_FLG_DOM_INCREMENTED;
		}
	} else if (is_adjust && (future_dom == 28) &&
		(move_bill_day_forward == 0) &&
			pin_conf_advance_bill_cycle) {
		future_dom = 1;
		dom_advanced = PIN_BILL_FLG_DOM_INCREMENTED;
	}

	/***************************************************************
	 * Calculate cycle_dom
	 ***************************************************************/

	if (!cycle_dom) {
		if (is_creation && !pin_conf_advance_bill_cycle) {
			/***********************************************
			* Get cycle_dom from end_t or virtual time
			***********************************************/
			vp = (time_t*)PIN_FLIST_FLD_GET(i_flistp, 
						PIN_FLD_END_T, 1, ebufp);
			if (vp) {
				end_t = *(time_t *)vp;
			} else {
				end_t =pin_virtual_time((time_t *)NULL);
			}
			cycle_dom = fm_utils_cycle_get_dom(end_t, ebufp);
		} else {
			cycle_dom =  future_dom;
		}
	}

	PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_ACTG_CYCLE_DOM,  
						(void *)&cycle_dom, ebufp);
	PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_ACTG_FUTURE_DOM,  
						(void *)&future_dom, ebufp);

	if (dom_advanced) {
		PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_FLAGS,
					(void *)&dom_advanced, ebufp);
	}

	/***********************************************************
	 * Calculate ACTG_CYCLE_LEN
	 ***********************************************************/
	fm_cust_pol_prep_billinfo_cycle_length(ctxp, bi_flistp, ebufp);

	PIN_FLIST_DESTROY_EX(&cache_flistp, NULL);
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_prep_billinfo_dom error", ebufp);
	}
	return;
}

/*******************************************************************
 * fm_cust_pol_prep_billinfo_cycle_length():
 *
 * This function calculates the cycle_length attribute, which can have values
 * either LONG or SHORT.
 *
 * The default implementation sets cycle_length to LONG, if the distance
 * between current dom and future dom less than 15 days. 
 * Otherwise, cycle_length is set to SHORT.
 *
 *******************************************************************/
static void
fm_cust_pol_prep_billinfo_cycle_length(
	pcm_context_t    *ctxp,
	pin_flist_t	*bi_flistp, 
	pin_errbuf_t	*ebufp)
{
	void		*vp = NULL;
	int32		cycle_len = 0;
	int32		curr_dom = 0;
	int32		future_dom = 0;
	int32           pay_type = PIN_PAY_TYPE_UNDEFINED;
	time_t	  	tmp_t = (time_t)0;
	int32           short_bill_cycle = 0 ;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/*
	 * Get current dom
	 */
	vp = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_ACTG_NEXT_T, 1, ebufp);
	if (vp) {
		tmp_t = *((time_t *)vp);
		if (tmp_t) {
			curr_dom = fm_utils_cycle_get_dom(tmp_t, ebufp);
		}
	}

	vp = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_PAY_TYPE, 1, ebufp);
	if (vp) {
		pay_type = *(int32 *)vp;
	}

	if ((curr_dom == 0) || (pay_type == PIN_PAY_TYPE_SUBORD)) {
		vp = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_ACTG_CYCLE_DOM, 0, 
			ebufp);
		if (vp) {
			curr_dom = *((int32 *)vp);
		}
	}
	if (curr_dom == 0) {
		curr_dom = fm_utils_cycle_get_dom((time_t)0, ebufp);
	}

	vp = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_ACTG_FUTURE_DOM, 0, ebufp);
	if (vp) {
		future_dom = *((int32 *)vp);
	}

	short_bill_cycle = fm_utils_bparams_cache_get_int(ctxp,
                                                PSIU_BPARAMS_BILLING_PARAMS, PSIU_BPARAMS_SHORT_CYCLE,
                                                0, ebufp);
        if (short_bill_cycle) {
                cycle_len = PIN_ACTG_CYCLE_SHORT;
        }
        else {
                /*
                 * Calculate the desired cycle_length
                 */
                cycle_len = PIN_ACTG_CYCLE_SHORT;

                if ((future_dom > curr_dom) && ((future_dom - curr_dom) < 15)) {
                        //cycle_len = PIN_ACTG_CYCLE_LONG;
			cycle_len = PIN_ACTG_CYCLE_SHORT;
                }
	}
	if ((future_dom < curr_dom) && ((curr_dom - future_dom) > 15)) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "cycle_len=PIN_ACTG_CYCLE_SHORT");
		cycle_len = PIN_ACTG_CYCLE_SHORT;
	//	cycle_len = PIN_ACTG_CYCLE_LONG;
	}

	/*
	 * Set the cycle length to the output
	 */
	PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_ACTG_CYCLE_LEN, 
		(void *)&cycle_len, ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_prep_billinfo_cycle_length error", ebufp);
	}
	return;
}

/*******************************************************************
 * fm_cust_pol_prep_billinfo_next_future()
 *
 * Call the SPEC_FUTURE_CYCLE policy to calculate ACTG_NEXT_T and ACTG_FUTURE_T
 *
 *******************************************************************/
static
void fm_cust_pol_prep_billinfo_next_future(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp, 
	pin_flist_t	*bi_flistp, 
	int32		is_creation,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*s_flistp = NULL; 
	pin_flist_t	*r_flistp = NULL; 
	int32		*pay_type = NULL;
	time_t		*actg_next_t = NULL;
	time_t		*actg_future_t = NULL;
	void		*vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/*******************************************************
	 * Calculate ACTG_NEXT_T and ACTG_FUTURE_T only if 
	 * FUTURE_DOM has been passed/calculated
	 *******************************************************/
	if (!PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_ACTG_FUTURE_DOM, 1, ebufp)) {
		return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_cust_pol_prep_billinfo_next_future input flistp", i_flistp);

	/*
	 * In case of creation of sub-ordinate billinfo, the fields 
	 * PIN_FLD_ACTG_NEXT_T and PIN_FLD_ACTG_FUTURE_T would have been 
	 * already propagated from the parent's billinfo. So, we should 
	 * skip caculating them again, otherwise we might end up in wrong
	 * billing cycle calculation for the subordinate account.
	 */
	pay_type = (int32 *) PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_PAY_TYPE, 1, ebufp);
	actg_next_t = (time_t *) PIN_FLIST_FLD_GET(bi_flistp, 
		PIN_FLD_ACTG_NEXT_T, 1, ebufp);
	actg_future_t = (time_t *) PIN_FLIST_FLD_GET(bi_flistp, 
		PIN_FLD_ACTG_FUTURE_T, 1, ebufp);

	if ( is_creation && pay_type && (*pay_type == PIN_PAY_TYPE_SUBORD) &&
		actg_next_t && actg_future_t ) {
		return;
	}

	s_flistp = PIN_FLIST_CREATE(ebufp);

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_POID, vp, ebufp);

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 0, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_END_T, vp, ebufp);

	if (!is_creation) {
		vp = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_ACTG_NEXT_T, 1,ebufp);
		if (!vp) {
	                PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
			return;
		}
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_ACTG_NEXT_T, vp, ebufp);
	}

	vp = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_ACTG_CYCLE_LEN, 0,ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_ACTG_CYCLE_LEN, vp, ebufp);

	vp = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_ACTG_FUTURE_DOM, 0, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_ACTG_CYCLE_DOM, vp, ebufp);

	vp = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_PAY_TYPE, 0, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_PAY_TYPE, vp, ebufp);

	vp = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_FLAGS, 1, ebufp);
	if (vp) {
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, vp, ebufp);
	}

	PCM_OP(ctxp, PCM_OP_BILL_POL_SPEC_FUTURE_CYCLE, 0, s_flistp,
		&r_flistp, ebufp);

	vp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_ACTG_NEXT_T, 0, ebufp);
	PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_ACTG_NEXT_T, vp, ebufp);

	vp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_ACTG_FUTURE_T, 0, ebufp);
	PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_ACTG_FUTURE_T, vp, ebufp);

	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_prep_billinfo_next_future error", ebufp);
	}
	return;
}

/*******************************************************************
 * fm_cust_pol_prep_billinfo_read():
 *
 * This reads billinfo objects for the given account
 *
 *******************************************************************/
static void 
fm_cust_pol_prep_billinfo_read(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	*bi_flistp,
	pin_flist_t	**bis_flistpp,
	pin_errbuf_t	*ebufp) 
{
	pin_flist_t*	r_flistp = NULL;
	pin_flist_t*	s_flistp = NULL;
	pin_flist_t*	tmp_flistp = NULL;
	poid_t		*a_pdp = NULL;
	poid_t		*bi_pdp = NULL;
	poid_t		*s_pdp = NULL;
	int64		database = 0;
	int32		s_flags = 0;
	int32		el_id = 0;
	pin_cookie_t	cookie = NULL;
	void		*vp = NULL;

	if(PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);
	if (*bis_flistpp) {
		return;
	}
	
	a_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	s_flistp = PIN_FLIST_CREATE(ebufp);
	database = (int64)PIN_POID_GET_DB(a_pdp);
	s_pdp = (poid_t *)PIN_POID_CREATE(database, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

	tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, (void *)a_pdp,ebufp);

	bi_pdp  = (poid_t *)PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_POID, 1,ebufp);
	if (bi_pdp && (PIN_POID_GET_ID(bi_pdp) > 0)) {
		tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2,
									ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_POID, (void *)bi_pdp,
									ebufp);
		vp = (void *)"select X from /billinfo where ( F1 = V1 and F2 != V2 ) ";
	} else {
		vp = (void *)"select X from /billinfo where ( F1 = V1 ) ";
	}
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, vp, ebufp);

	tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_BILL_WHEN, NULL, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACTG_CYCLE_DOM, NULL, ebufp);

	PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_READ_UNCOMMITTED, s_flistp, 
							&r_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);

	*bis_flistpp = PIN_FLIST_ELEM_TAKE_NEXT(r_flistp, PIN_FLD_RESULTS,
						&el_id, 1, &cookie, ebufp);
	if (!(*bis_flistpp)) {
		*bis_flistpp = PIN_FLIST_CREATE(ebufp);
	}
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_prep_billinfo_read error", ebufp);
	}
	return;
}

/*******************************************************************
 * fm_cust_pol_prep_billinfo_get_next_dom_from_cache():
 *
 * This function allocated the FUTURE_DOM from the dom map configuration
 * starting from the current date and then pickig the next available DOM
 *
 *******************************************************************/
static int32
fm_cust_pol_prep_billinfo_get_next_dom_from_cache(
	dom_map_t	*dom_map_cache,
	int32		today,
	int32		num_elem,
	pin_errbuf_t	*ebufp)
{
	int32		i_dom = 0; 
	int32		i = 0;
	int32           v_days = 28; /*Number of valid DOMs in a month*/
	int32		move_bill_day_forward = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return(0);
	PIN_ERR_CLEAR_ERR(ebufp);

	/**************************************************************
	 * Return '0' if number of elements is zero '0'
	 **************************************************************/ 
	if(!num_elem) 
		return(0);

        /**************************************************************
         * If 31 day billing feature is used, 31 DOMs are valid.
	 * If not, reset current dom if current date is greater than 28.
         **************************************************************/
	move_bill_day_forward =
		fm_bill_utils_get_31day_billing_opt(ebufp);

	if(move_bill_day_forward != 0) {
	v_days = 31;
	}
	else if (today > 28){
		today = 1;
	}

	/**************************************************************
	 * Set to hold the current DOM. We know from where we started lookup
	 **************************************************************/ 
	i_dom = today;

	/**************************************************************
	 * Scan through the cached DOM map information
	 *
	 * Data Structure (dom_map_cache) is available as a global variable
	 **************************************************************/ 
	while(1) {
		for(i = 0; i < num_elem ; i++) {
			if(dom_map_cache[i].dom == today) {
				if(dom_map_cache[i].status == PIN_ACTG_DOM_STATUS_OPEN) {
					return(dom_map_cache[i].dom);
				}
			}
		}
		today++;

		/************************************************
		 * We have reached from where we started without 
		 * finding the matching dom. Return 0 
		 ************************************************/ 
		if(today == i_dom) {
			return(0);
		}
		
		if(today > v_days) {
			if(i_dom == 1) {
				return (0); /* checked from 1 to number of valid DOMs already */
			} else {
				today = 1;
			}
		}
	}
}

/*******************************************************************
 * fm_cust_pol_prep_billinfo_get_dom_from_process_t_from_cache():
 *
 * This function allocated the FUTURE_DOM from the dom map configuration
 * using the weighted average method of the total process time 
 *
 * This function is provided as a sample implementation for allocating 
 * the future_dom
 *******************************************************************/
int32
fm_cust_pol_prep_billinfo_get_dom_from_process_t_from_cache(
	dom_map_t	*dom_map_cache,
	int32		today,
	int32		num_elem,
	pin_errbuf_t	*ebufp)
{
	int32		*pt[2]; 
	int32		rnum = 0;
	int32		total = 0;
	int32		count = 0;
	int32		i = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return(0);
	PIN_ERR_CLEAR_ERR(ebufp);

	/**************************************************************
	 * Return '0' if number of elements in data structure is zero '0'
	 **************************************************************/ 
	if(!num_elem)
		return(0);

	/**************************************************************
	 * Allocate the two-dimensional array to hold num_elem
	 **************************************************************/
	pt[0] = (int32 *) malloc(num_elem * sizeof(int32));
	pt[1] = (int32 *) malloc(num_elem * sizeof(int32));
	if(pt[0] == NULL || pt[1] == NULL) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_NO_MEM, 0, 0, 0);
	}

	/**************************************************************
	 * Scan through the data structure
	 *
	 * Data Structure dom_map_cache is defined global so I can use it here 
	 *
	 * Fill the array by the integer value from the calulation 
	 * (2*24*60*60)/process time. Here we take 2*24*60*60 (2 Days) 
	 * as a constant. Sum the total resultant integer value from 
	 * the invidual calculation for random number generation
	 **************************************************************/ 
	for(i = 0; i < num_elem; i++)
	{
		if(dom_map_cache[i].status == PIN_ACTG_DOM_STATUS_OPEN) {
			pt[0][count] = dom_map_cache[i].dom;
			pt[1][count] = ((2*24*60*60) / dom_map_cache[i].process_t);
			total += pt[1][count];
			count++;
		}
	}

	/**************************************************************
	 * Get the Random number using total as a seed value
	 **************************************************************/ 
	rnum = fm_cust_pol_prep_billinfo_get_rand_no(total);  

	/**************************************************************
	 * If Random number - first entry of the integer value from the 
	 * calculation ((2*24*60*60)/process time) is <= 0 then return
	 * the DOM else use the resultant value and substract from the 
	 * next entry. Do till the resultant value is <= 0. Return the
	 * DOM where the check is successful i.e; <= 0
	 **************************************************************/
	for(i = 0; i < count; i++)  
	{       
		rnum -= pt[1][i];  
		if(rnum <= 0) { 
			rnum = pt[0][i];   
			free(pt[0]);
			free(pt[1]);
			return(rnum);
		}
	}
	free(pt[0]);
	free(pt[1]);
	return(0);
}

/*******************************************************************
 * fm_cust_pol_prep_billinfo_get_rand_no():
 *
 * This function generates the random number
 *
 *******************************************************************/
int32
fm_cust_pol_prep_billinfo_get_rand_no(
	int32 num)
{
	srand(num);

	return (int32)((rand() * num) / RAND_MAX  );
}
