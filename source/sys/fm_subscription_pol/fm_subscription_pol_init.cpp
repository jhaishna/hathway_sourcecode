/*
 *
 * Copyright (c) 2004, 2009, Oracle and/or its affiliates. 
All rights reserved. 
 *      
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 */

#ifndef lint
static  char    Sccs_id[] = "@(#)$Id: fm_subscription_pol_init.cpp /cgbubrm_7.5.0.portalbase/1 2015/11/27 06:17:10 nishahan Exp $";
#endif

#include "pcm.h"
#include "cm_fm.h"
#include "Pin.h"

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
#ifdef __cplusplus
extern "C" {

EXPORT_OP void 
fm_subscription_pol_init(
    int32                   *errp);


}
#endif


/* Global flist to hold the snowball distribution rule */
PIN_EXPORT PinFlistOwner fm_subs_pol_snowball_distrib_sp;

static void fm_subscription_pol_init_snowball_distrib(
	PinContextBase &ctx_sp,
	const PinPoidDb &db_id);

void fm_subscription_pol_init(
	int32 *errp)
{
	PinErrorBuf ebuf;
	ebuf.Clear();

	try {
		PinFlistObserver null_flist_sp = PinFlist::createAsObserved(NULL);
		PinContextOwner ctx_sp = PinContext::create(null_flist_sp);
		PinPoidDb db_id = ctx_sp->getUserId()->getDb();
		
		fm_subscription_pol_init_snowball_distrib(ctx_sp, db_id);

	}
    catch (PinDeterminateExc &exc) {
        PIN_LOG(exc, PIN_ERR_LEVEL_ERROR, "fm_subscription_pol_init error");
        exc.copyInto(&ebuf);
	}
	
	*errp = ebuf.getError();
	return;
}

static void fm_subscription_pol_init_snowball_distrib(
	PinContextBase &ctx_sp,
	const PinPoidDb &db_id)
{
	try {
		PinFlistOwner s_flist_sp = PinFlist::create();
		PinPoidOwner tmp_pd = PinPoid::create(db_id, "/search", 500);
		s_flist_sp->put(tsf_PIN_FLD_POID, tmp_pd);
		s_flist_sp->set(tsf_PIN_FLD_PARAMETERS, "snowball_distribution");
		
		PinFlistObserver flist_sp = s_flist_sp->add(tsf_PIN_FLD_ARGS, 1);
		tmp_pd = PinPoid::create(db_id, "/config/snowball_distribution", -1);
		flist_sp->put(tsf_PIN_FLD_POID, tmp_pd);

		PinFlistObserver null_observer;
		s_flist_sp->set(tsf_PIN_FLD_RESULTS, null_observer, 0);
		
		PinFlistOwner o_flist_sp;
		ctx_sp->op(PCM_OP_SEARCH, 0, s_flist_sp, o_flist_sp);

		fm_subs_pol_snowball_distrib_sp =
			o_flist_sp->take(tsf_PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1);

		if (!fm_subs_pol_snowball_distrib_sp->isNull()) {
			PIN_LOG(fm_subs_pol_snowball_distrib_sp, PIN_ERR_LEVEL_DEBUG,
				"fm_subs_pol_snowball_distrib_sp content");
		}
	}
    catch (PinDeterminateExc &exc) {
        PIN_LOG(exc, PIN_ERR_LEVEL_ERROR, "fm_subscription_pol_init_snowball_distrib error");
		throw exc;
	}
}

