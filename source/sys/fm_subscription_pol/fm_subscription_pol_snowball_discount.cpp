/*
 *
 * Copyright (c) 2004, 2009, Oracle and/or its affiliates. 
All rights reserved. 
 *      
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *
 */

#ifndef lint
static  char    Sccs_id[] = "@(#)$Id: fm_subscription_pol_snowball_discount.cpp /cgbubrm_7.5.0.portalbase/1 2015/11/27 06:17:40 nishahan Exp $";
#endif

/*******************************************************************
 * Contains the PCM_OP_SUBSCRIPTION_POL_SNOWBALL_DISCOUNT operation. 
 *
 * Also included are subroutines specific to the operation.
 *
 * These routines are generic and work for all account types.
 *******************************************************************/

#include <stdio.h> 
#include <string.h> 
 
#include "pcm.h"
#include "ops/bal.h"
#include "ops/subscription.h"
#include "fm_bal.h"
#include "pin_act.h"
#include "pin_bill.h"
#include "pin_cust.h"
#include "pin_currency.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "Pin.h"

#define FILE_SOURCE_ID  "fm_subscription_pol_snowball_discount.cpp(1)"


/*******************************************************************
 * Routines contained within.
 *******************************************************************/
#ifdef __cplusplus
extern "C" {

	EXPORT_OP void
	op_subscription_pol_snowball_discount(
        cm_nap_connection_t	*connp,
		u_int			opcode,
        u_int			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp);
}

#endif

/* Global flist to hold the snowball distribution rule */
PIN_IMPORT PinFlistOwner fm_subs_pol_snowball_distrib_sp;

static void fm_subs_pol_snowball_discount(
	pcm_context_t	*ctxp,
	u_int			flags,
	pin_flist_t	*i_flistp,
	pin_flist_t	**out_flistp,
	pin_errbuf_t	*ebufp);

static void fm_subs_pol_find_disc_sharing_group(
	PinContextObserver &ctx,
	u_int flags,
	PinFlistBase &o_flist,
	PinPoidObserver &disc_pd,
	PinFlistOwner &group_flist);

static void fm_subs_pol_distribute_snowball(
	PinContextObserver &ctx,
	u_int flags,
	PinFlistOwner &o_flist,
	PinRecId bi_elem,
	int32 resource_id,
	PinFlistOwner &group_flist);

static PinPoidObserver NULL_POID;
/*******************************************************************
 * Main routine for the PCM_OP_SUBSCRIPTION_POL_SNOWBALL_DISCOUNT operation.
 *******************************************************************/
void
op_subscription_pol_snowball_discount(
	cm_nap_connection_t	*connp,
	u_int			opcode,
	u_int			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	pin_cookie_t		cookie = NULL;

	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*flistp = NULL;

	poid_t			*a_pdp = NULL;

	char			*action;
	char			*cp;

	u_int			rec_id;
	u_int			done = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_SUBSCRIPTION_POL_SNOWBALL_DISCOUNT) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_subscription_pol_snowball_discount opcode error",
			ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_subscription_pol_snowball_discount input flist", 
		in_flistp);

	/***********************************************************
	 * Get the policy return flist.
	 ***********************************************************/
	fm_subs_pol_snowball_discount(ctxp, flags, in_flistp, &r_flistp, ebufp);

	/***********************************************************
	 * Errors?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {

		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_subscription_pol_snowball_discount error", 
			ebufp);
		PIN_FLIST_DESTROY(r_flistp, NULL);
		*ret_flistpp = NULL;

	} else {

		/***************************************************
		 * Point the real return flist to the right thing.
		 ***************************************************/
		PIN_ERR_CLEAR_ERR(ebufp);
		*ret_flistpp = r_flistp;
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_subscription_pol_snowball_discount return flist",
			*ret_flistpp);
	}

	return;
}


/*******************************************************************
 * fm_subs_pol_snowball_discount():
 *
 *	This policy op-code is invoked if the billing time discount
 *	is configured as a "Snowball Discount". The standard FM_
 *	SUBSCRIPTION op-code will trigger the billing time discount
 *	event and after the discount calculation is done, this
 *	policy is invoked to do the SNOWBALL distribution. 
 *******************************************************************/
void
fm_subs_pol_snowball_discount(
	pcm_context_t	*ctxp,
	u_int			flags,
	pin_flist_t	*i_flistp,
	pin_flist_t	**out_flistpp,
	pin_errbuf_t	*ebufp)
{
	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	try {
		PinContextObserver ctx = PinContext::createAsObserved(ctxp);
        PinFlistObserver in_flist = PinFlist::createAsObserved(i_flistp);
		PinFlistOwner o_flist = in_flist->clone();

		if (!fm_subs_pol_snowball_distrib_sp->isNull() &&
			fm_subs_pol_snowball_distrib_sp->count(tsf_PIN_FLD_DISTRIBUTION) &&
			o_flist->count(tsf_PIN_FLD_BAL_IMPACTS)) {

			PinElemObservingIterator iter =
				in_flist->getElements(tsf_PIN_FLD_BAL_IMPACTS);
			while (iter.hasMore()) {
				PinFlistObserver bi_flist = iter.next();
				PinPoidObserver prod_pd =
					bi_flist->get(tsf_PIN_FLD_PRODUCT_OBJ);
				if (strcmp(prod_pd->getType(), "/discount")) {
					continue;
				}

				bool found = false;
				int32 resource_id = 0;
				
				PinElemObservingIterator sb_iter =
					fm_subs_pol_snowball_distrib_sp->getElements(tsf_PIN_FLD_DISTRIBUTION);
				while (sb_iter.hasMore()) {
					PinFlistObserver sb_flist = sb_iter.next();
					PinPoidObserver disc_pd =
						sb_flist->get(tsf_PIN_FLD_DISCOUNT_OBJ);
					if (disc_pd->isEqual(prod_pd)) {
						found = true;
						resource_id =
							sb_flist->get(tsf_PIN_FLD_RESOURCE_ID)->value();
						break;
					}
				}
				if (!found || resource_id < 0) {
					continue;
				}

				PinFlistOwner group_flist;
				fm_subs_pol_find_disc_sharing_group(ctx, flags,
					o_flist, prod_pd, group_flist);
				
				if (!group_flist->isNull() &&
					group_flist->count(tsf_PIN_FLD_MEMBERS)) {
					// XXX needs to distribute here
					fm_subs_pol_distribute_snowball(ctx, flags,
						o_flist, iter.getRecId(), resource_id, group_flist);
				}
			}
		}
		
        *out_flistpp = o_flist->release();
	}
	catch (PinDeterminateExc &exc) {
        PIN_LOG(exc, PIN_ERR_LEVEL_ERROR,
            "fm_subs_pol_snowball_discount error");
        exc.copyInto(ebufp);
    }

	/***********************************************************
	 * Errors?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_subs_pol_snowball_discount error", ebufp);

	}

	return;
}

static void fm_subs_pol_find_disc_sharing_group(
	PinContextObserver &ctx,
	u_int flags,
	PinFlistBase &out_flist,
	PinPoidObserver &disc_pd,
	PinFlistOwner &group_flist)
{
	try {
		PinFlistOwner search_flist = PinFlist::create();
		PinPoidOwner tmp_pd = PinPoid::create(disc_pd->getDb(), "/search", -1);
		search_flist->put(tsf_PIN_FLD_POID, tmp_pd);
		search_flist->set(tsf_PIN_FLD_FLAGS, SRCH_DISTINCT);
            
		PinFlistObserver args = search_flist->add(tsf_PIN_FLD_ARGS, 1);
		args->set(tsf_PIN_FLD_ACCOUNT_OBJ,
			out_flist->get(tsf_PIN_FLD_ACCOUNT_OBJ));

		args = search_flist->add(tsf_PIN_FLD_ARGS, 2);
		args = args->add(tsf_PIN_FLD_DISCOUNTS, PIN_ELEMID_ANY);
		args->set(tsf_PIN_FLD_DISCOUNT_OBJ, disc_pd);

		args = search_flist->add(tsf_PIN_FLD_RESULTS, 0);
		args->set(tsf_PIN_FLD_POID, NULL_POID);
		args->set(tsf_PIN_FLD_MEMBERS, PinFlistObserver(),
			PIN_ELEMID_ANY);
		
		const char *search_str = "select X from /group/sharing/discounts"
			" where F1 = V1 and F2 = V2 ";
		search_flist->set(tsf_PIN_FLD_TEMPLATE, search_str);
                
		PIN_LOG(search_flist, PIN_ERR_LEVEL_DEBUG,
			"search /group/sharing/discounts input flist");
		PinFlistOwner t_o_flist;
		ctx->opref(PCM_OP_SEARCH, flags, search_flist, t_o_flist);
		PIN_LOG(t_o_flist, PIN_ERR_LEVEL_DEBUG,
			"search /group/sharing/discounts output flist");

		group_flist = t_o_flist->take(tsf_PIN_FLD_RESULTS, 0, 1);
		
    }
    catch (PinDeterminateExc &exc) {
        PIN_LOG(exc, PIN_ERR_LEVEL_ERROR,
            "fm_subs_pol_find_disc_sharing_group error");
		throw exc;
    }
}

static void fm_subs_pol_distribute_snowball(
	PinContextObserver &ctx,
	u_int flags,
	PinFlistOwner &o_flist,
	PinRecId bi_elem,
	int32 resource_id,
	PinFlistOwner &group_flist)
{
	time_t cend_t = (time_t)0;

	try {
		PinFlistObserver ci_flist =
			o_flist->get(tsf_PIN_FLD_CYCLE, PIN_ELEMID_ANY);
		PinTstampObserver tmpT;
        	tmpT = ci_flist->get(tsf_PIN_FLD_CYCLE_END_T, 1);
        	if (!tmpT->isNull()) {
                	cend_t = tmpT->value() - 1;
        	}

		PinFlistObserver bi_flist =
			o_flist->get(tsf_PIN_FLD_BAL_IMPACTS, bi_elem);
		PinPoidObserver bg_pd = bi_flist->get(tsf_PIN_FLD_BAL_GRP_OBJ);
		PinPoidObserver acct_pd = bi_flist->get(
				tsf_PIN_FLD_ACCOUNT_OBJ);
		PinFlistOwner bal_flist;
		if (resource_id > 0) {
			// get the balances out of the balance group
			PinFlistOwner inp_flist = PinFlist::create();
			inp_flist->set(tsf_PIN_FLD_POID, acct_pd);
			inp_flist->set(tsf_PIN_FLD_BAL_GRP_OBJ, bg_pd);
			inp_flist->set(tsf_PIN_FLD_END_T, cend_t);
			inp_flist->set(tsf_PIN_FLD_FLAGS, 
					PIN_BAL_GET_BARE_RESULTS);
			inp_flist->set(tsf_PIN_FLD_BALANCES, PinFlistObserver(),
					resource_id);
			ctx->op(PCM_OP_BAL_GET_BALANCES, flags, 
					inp_flist, bal_flist);
		}

		PinBigDecimal total_amt = *(bi_flist->get(tsf_PIN_FLD_AMOUNT));
		PinBigDecimal total_cnt;
		if (resource_id > 0) {
			PinFlistObserver flist = bal_flist->get(
					tsf_PIN_FLD_BALANCES, resource_id);
			total_cnt = *(flist->get(tsf_PIN_FLD_CURRENT_BAL));
		} else {
			total_cnt = PinBigDecimal(group_flist->count(
					tsf_PIN_FLD_MEMBERS) + 1.0, 1);
		}

		PinBigDecimal distributed_amt;
		
		PinElemObservingIterator iter =
			group_flist->getElements(tsf_PIN_FLD_MEMBERS);
		while (iter.hasMore()) {
			PinFlistObserver flist = iter.next();

			PinFlistOwner inp_flist = PinFlist::create();
			PinPoidObserver the_acct_pd = flist->get(
					tsf_PIN_FLD_ACCOUNT_OBJ);
			inp_flist->set(tsf_PIN_FLD_POID, the_acct_pd);
			PinPoidObserver svc_pd = flist->get(tsf_PIN_FLD_SERVICE_OBJ);
			if (!svc_pd->isNull()) {
				inp_flist->set(tsf_PIN_FLD_SERVICE_OBJ, svc_pd);
			}
			inp_flist->set(tsf_PIN_FLD_FLAGS, PIN_BAL_GET_POID_ONLY);
			PinFlistOwner tmp_flist;
			ctx->op(PCM_OP_BAL_GET_BALANCES, flags, inp_flist, tmp_flist);
			PinPoidObserver the_bg_pd = tmp_flist->get(tsf_PIN_FLD_POID);

			if (the_bg_pd->isEqual(bg_pd)) {
				// same balance group poid, no need distribution for this entry
				continue;
			}
			
			PinBigDecimal this_cnt;
			if (resource_id == 0) {
				this_cnt = PinBigDecimal(1.0, 1);
			} else {
				char contrib_str[BUFSIZ];
				if (!svc_pd->isNull()) {
					svc_pd->toString(contrib_str, 256, 1);
				} else {
					the_acct_pd->toString(contrib_str, 256, 1);
				}
				// get the contributed amount out
				PinFlistObserver flist = bal_flist->get(tsf_PIN_FLD_BALANCES,
					resource_id);
				PinElemObservingIterator sb_iter =
					flist->getElements(tsf_PIN_FLD_SUB_BALANCES, 1);
				while (sb_iter.hasMore()) {
					PinFlistObserver t_flist = sb_iter.next();
					PinStrObserver t_contrib_str =
						t_flist->get(tsf_PIN_FLD_CONTRIBUTOR_STR);
					if (!strcmp(t_contrib_str->value(), contrib_str)) {
						this_cnt += *(t_flist->get(tsf_PIN_FLD_CURRENT_BAL));
					}
				}
			}

			PinBigDecimal this_amt = this_cnt / total_cnt * total_amt;
			if (this_amt.isZero()) {
				continue;
			}
			
			distributed_amt += this_amt;
			PinFlistOwner t_bi_flist = bi_flist->clone();
			t_bi_flist->set(tsf_PIN_FLD_BAL_GRP_OBJ, the_bg_pd);
			t_bi_flist->set(tsf_PIN_FLD_ACCOUNT_OBJ, the_acct_pd);
			t_bi_flist->take(tsf_PIN_FLD_OWNER_OBJ, 1);
			t_bi_flist->take(tsf_PIN_FLD_BILLINFO_OBJ, 1);
			t_bi_flist->take(tsf_PIN_FLD_ITEM_OBJ, 1);
			t_bi_flist->set(tsf_PIN_FLD_AMOUNT, this_amt);
			PinBigDecimalObserver deferred_amt =
				t_bi_flist->get(tsf_PIN_FLD_AMOUNT_DEFERRED, 1);
			if (!deferred_amt->isNull() && !deferred_amt->isZero()) {
				t_bi_flist->set(tsf_PIN_FLD_AMOUNT_DEFERRED, this_amt);
			}

			o_flist->put(tsf_PIN_FLD_BAL_IMPACTS, t_bi_flist,
				o_flist->count(tsf_PIN_FLD_BAL_IMPACTS));
		}

		if (!distributed_amt.isZero()) {
			total_amt -= distributed_amt;
			bi_flist->set(tsf_PIN_FLD_AMOUNT, total_amt);
			PinBigDecimalObserver deferred_amt =
				bi_flist->get(tsf_PIN_FLD_AMOUNT_DEFERRED, 1);
			if (!deferred_amt->isNull() && !deferred_amt->isZero()) {
				bi_flist->set(tsf_PIN_FLD_AMOUNT_DEFERRED, total_amt);
			}
		}
		
		// XXX may need to do rounding for the new amounts
    }
    catch (PinDeterminateExc &exc) {
        PIN_LOG(exc, PIN_ERR_LEVEL_ERROR,
            "fm_subs_pol_distribute_snowball error");
		throw exc;
    }
}

