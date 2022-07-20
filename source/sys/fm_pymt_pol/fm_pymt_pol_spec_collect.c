/*
 *
 *      Copyright (c) 1999 - 2007 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 *
 */

#ifndef lint
static  char    Sccs_id[] = "@(#)%Portal Version: fm_pymt_pol_spec_collect.c:RWSmod7.3.1Int:1:2007-Jun-28 05:32:58 %";
#endif

/*******************************************************************
 * Contains the PCM_OP_PYMT_POL_SPEC_COLLECT operation. 
 *******************************************************************/

#include <stdio.h> 
#include <string.h> 
 
#include "pcm.h"
#include "ops/pymt.h"
#include "pin_pymt.h"
#include "pin_bill.h"
#include "pin_cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_pymt_pol_spec_collect(
        cm_nap_connection_t	*connp,
	int32			opcode,
        int32			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp);

static void
fm_pymt_pol_spec_collect_commit_cust_cc(
	pcm_context_t	*ctxp,
	int32		flags,
	pin_flist_t	*i_flistp,
	pin_flist_t	*o_flistp,
	pin_errbuf_t	*ebufp);

static void
fm_pymt_pol_spec_collect_commit_cust_dd(
	pcm_context_t	*ctxp,
	int32		flags,
	pin_flist_t	*i_flistp,
	pin_flist_t	*o_flistp,
	pin_errbuf_t	*ebufp);

static void
fm_pymt_pol_spec_collect_get_items(
        pcm_context_t   *ctxp,
        int32           flags,
        pin_flist_t     *i_flistp,
        pin_flist_t     *o_flistp,
        pin_errbuf_t    *ebufp);

static void
fm_pymt_pol_spec_collect_get_ach(
        pcm_context_t   *ctxp,
        int32           flags,
        pin_flist_t     *i_flistp,
        pin_flist_t     *o_flistp,
        pin_errbuf_t    *ebufp);

/*******************************************************************
 * Routines needed from elsewhere.
 *******************************************************************/

/*******************************************************************
 * Main routine for the PCM_OP_PYMT_POL_SPEC_COLLECT operation.
 *******************************************************************/
void
op_pymt_pol_spec_collect(
        cm_nap_connection_t	*connp,
	int32			opcode,
        int32			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*r_flistp = NULL;
	pin_bill_type_t		*p_type = NULL;
	int32			result = PIN_BOOLEAN_FALSE;

	char			*action = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_PYMT_POL_SPEC_COLLECT) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_pymt_pol_spec_collect opcode error", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_pymt_pol_spec_collect input flist", i_flistp);

	/***********************************************************
	 * Prep the return flist.
	 ***********************************************************/
	r_flistp = PIN_FLIST_CREATE(ebufp);

	/***********************************************************
	 * Get the requested action. The following are supported:
	 *	- "commit customer"
	 *
	 ***********************************************************/
	action = (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTION, 0, ebufp);

	if ((action != (char *)NULL) && (!strcmp(action, "commit customer"))) {

		p_type = (pin_bill_type_t *)PIN_FLIST_FLD_GET(i_flistp,
			PIN_FLD_PAY_TYPE, 0, ebufp);

		/***************************************************
		 * Credit card
		 ***************************************************/
		if ((p_type != NULL) && (*p_type == PIN_BILL_TYPE_CC)) {
			fm_pymt_pol_spec_collect_commit_cust_cc(ctxp,
				flags, i_flistp, r_flistp, ebufp);

		/***************************************************
		 * Direct Debit
		 ***************************************************/
		} else if ((p_type != NULL) && (*p_type == PIN_BILL_TYPE_DD)) {
			fm_pymt_pol_spec_collect_commit_cust_dd(ctxp,
				flags, i_flistp, r_flistp, ebufp);

		/***************************************************
		 * Assume nothing to collect
		 ***************************************************/
		} else {
			result = PIN_BOOLEAN_FALSE;
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_BOOLEAN,
			(void *)&result, ebufp);
        	}

	} else {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_ACTION, 0, 0);
	}

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {

		/***************************************************
		 * Log something and return nothing.
		 ***************************************************/
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_pymt_pol_spec_collect error", ebufp);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		*o_flistpp = NULL;

	} else {

		/***************************************************
		 * Point the real return flist to the right thing.
		 ***************************************************/
		PIN_ERR_CLEAR_ERR(ebufp);
		*o_flistpp = r_flistp;

		/***************************************************
		 * Debug: What we're sending back.
		 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_pymt_pol_spec_collect return flist", r_flistp);

	}

	return;
}


/*******************************************************************
 * fm_pymt_pol_spec_collect_commit_cust_cc():
 *
 *	Fill the the spec flist for the "commit customer" action.
 *	This routine is for Credit Card specific.
 *
 *******************************************************************/
static void
fm_pymt_pol_spec_collect_commit_cust_cc(
	pcm_context_t	*ctxp,
	int32		flags,
	pin_flist_t	*i_flistp,
	pin_flist_t	*o_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*flistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	pin_flist_t	*b_flistp = NULL;

	poid_t		*a_pdp = NULL;
	pin_bill_type_t	*b_type = NULL;
	pin_decimal_t	*amount = NULL;
	int32		*curr = NULL;
	int32		result = 0;

	int32		cc_collect_flag = 0;
	int32		err = 0;
	void		*valp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Get (and add) the account poid.
	 ***********************************************************/
	a_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(o_flistp, PIN_FLD_POID, (void *)a_pdp, ebufp);

        /***********************************************************
         * Set cc_collect flag
         ***********************************************************/
        cc_collect_flag = FM_CC_COLLECT_FLAG_DEFAULT;
        pin_conf(FM_PYMT_POL, FM_CC_COLLECT_TOKEN, PIN_FLDT_INT,
                        (caddr_t *)&valp, &err);
        switch (err) {
        case PIN_ERR_NONE:
                cc_collect_flag = *((int32 *)valp);
		free(valp);
                break;
 
        case PIN_ERR_NOT_FOUND:
                break;
 
        default:
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE, err, 0, 0, 0);
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "Unable to read cc_collect flag from pin.conf",ebufp);
                break;
        }
	/***********************************************************
	 * If the flag says not to collect, then skip collection.
	 ***********************************************************/
	if (!cc_collect_flag) {
		result = PIN_BOOLEAN_FALSE;
		PIN_FLIST_FLD_SET(o_flistp, PIN_FLD_BOOLEAN,
		(void *)&result, ebufp);
		goto Done;
		/********/
	}

	fm_pymt_pol_spec_collect_get_items(ctxp, flags, i_flistp, 
							o_flistp, ebufp);
Done:

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_pymt_pol_spec_collect_commit_cust_cc error", 
			ebufp);
	}

	return;
}


/*******************************************************************
 * fm_pymt_pol_spec_collect_commit_cust_dd():
 *
 *	Fill the the spec flist for the "commit customer" action.
 *	This routine is for Direct Debit specific.
 *
 *******************************************************************/
static void
fm_pymt_pol_spec_collect_commit_cust_dd(
	pcm_context_t	*ctxp,	
	int32		flags,
	pin_flist_t	*i_flistp,
	pin_flist_t	*o_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*b_flistp = NULL;

	poid_t		*a_pdp = NULL;
	pin_decimal_t	*amount = NULL;
	int32		*curr = NULL;
	int32		result = 0;

	int32		dd_collect_flag = 0;
	int32		err = 0;
	void		*valp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Get (and add) the account poid.
	 ***********************************************************/
	a_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(o_flistp, PIN_FLD_POID, (void *)a_pdp, ebufp);

        /***********************************************************
         * Set dd_collect flag
         ***********************************************************/
        dd_collect_flag = FM_DD_COLLECT_FLAG_DEFAULT;
        pin_conf(FM_PYMT_POL, FM_DD_COLLECT_TOKEN, PIN_FLDT_INT,
                        (caddr_t *)&valp, &err);
        switch (err) {
        case PIN_ERR_NONE:
                dd_collect_flag = *((int32 *)valp);
		free(valp);
                break;
 
        case PIN_ERR_NOT_FOUND:
                break;
 
        default:
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE, err, 0, 0, 0);
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "Unable to read dd_collect flag from pin.conf",ebufp);
                break;
        }
	/***********************************************************
	 * If the flag says not to collect, then skip collection.
	 ***********************************************************/
	if (!dd_collect_flag) {
		result = PIN_BOOLEAN_FALSE;
		PIN_FLIST_FLD_SET(o_flistp, PIN_FLD_BOOLEAN,
		(void *)&result, ebufp);
		goto Done;
		/********/
	}

	fm_pymt_pol_spec_collect_get_items(ctxp, flags, i_flistp, 
							o_flistp, ebufp);

Done:
	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_pymt_pol_spec_collect_commit_cust_dd error", ebufp);
	}

	return;
}

/*******************************************************************
 * fm_pymt_pol_spec_collect_get_items()
 * Get all the Open items
 *******************************************************************/

static void
fm_pymt_pol_spec_collect_get_items(
        pcm_context_t   *ctxp,
        int32           flags,
        pin_flist_t     *i_flistp,
        pin_flist_t     *o_flistp,
        pin_errbuf_t    *ebufp)
{
        pin_flist_t     *items_flistp = NULL;
        pin_flist_t     *r_flistp = NULL;
        pin_flist_t     *flistp = NULL;
        poid_t          *s_pdp = NULL;
        poid_t          *bi_pdp = NULL;
        int32           status = 0;
        int64           database = 0;
        int32           sflags = SRCH_DISTINCT;
        int32           result = PIN_BOOLEAN_FALSE;
        void            *vp = NULL;
        pin_decimal_t   *zero_decimal =  pin_decimal("0.0", ebufp);

        if (PIN_ERR_IS_ERR(ebufp)) {
                return;
        }
        PIN_ERR_CLEAR_ERR(ebufp);

        /*********************************************************
         * Get the ACH details.
         *********************************************************/
        fm_pymt_pol_spec_collect_get_ach(ctxp, flags, i_flistp,
                                                o_flistp, ebufp);

        bi_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BILLINFO_OBJ, 0, ebufp);

        /*********************************************************
         * Create the search flist to search in the item objects
         *********************************************************/
        items_flistp = PIN_FLIST_CREATE(ebufp);
        database = PIN_POID_GET_DB(bi_pdp);
        s_pdp = PIN_POID_CREATE(database, "/search", -1, ebufp);
        PIN_FLIST_FLD_PUT(items_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
        PIN_FLIST_FLD_SET(items_flistp, PIN_FLD_TEMPLATE,
         (void *) "select X from /item where F1 = V1 and ( F2 = V2 or F3 = V3 )" \
                " and F4 != V4 order by F5 ", ebufp);
        PIN_FLIST_FLD_SET(items_flistp, PIN_FLD_FLAGS, (void *)&sflags, ebufp);

        /*********************************************************
         * Add the search arguments.
         *********************************************************/
        flistp = PIN_FLIST_ELEM_ADD(items_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_AR_BILLINFO_OBJ, (void *)bi_pdp,
                                                                ebufp);

        status = PIN_ITEM_STATUS_PENDING;
        flistp = PIN_FLIST_ELEM_ADD(items_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_STATUS, (void *)&status, ebufp);

        status = PIN_ITEM_STATUS_OPEN;
        flistp = PIN_FLIST_ELEM_ADD(items_flistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_STATUS, (void *)&status, ebufp);

        flistp = PIN_FLIST_ELEM_ADD(items_flistp, PIN_FLD_ARGS, 4, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_DUE, (void *)zero_decimal, ebufp);

        flistp = PIN_FLIST_ELEM_ADD(items_flistp, PIN_FLD_ARGS, 5, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_EFFECTIVE_T, (void *)NULL, ebufp);

        /*********************************************************
         * Add the results array for the search.
         *********************************************************/

        flistp = PIN_FLIST_ELEM_ADD(items_flistp, PIN_FLD_RESULTS, 0, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID,(void *)NULL, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_BILL_OBJ, (void *)NULL, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_AR_BILL_OBJ, (void *)NULL, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACCOUNT_OBJ, (void *)NULL, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_DUE,(void *) NULL, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_EFFECTIVE_T, (void *)NULL, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_ITEM_TOTAL, (void *)NULL, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_DISPUTED, (void *)NULL, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_RECVD, (void *)NULL, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_ADJUSTED, (void *)NULL, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_TRANSFERED, (void *)NULL, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_CURRENCY, (void *)NULL, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_STATUS, (void *)NULL, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_ITEM_NO, (void *)NULL, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_BILLINFO_OBJ, (void *)NULL, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_AR_BILLINFO_OBJ, (void *)NULL, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_SERVICE_OBJ, (void *)NULL, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_EVENT_POID_LIST, (void *)NULL, ebufp);

        /**********************************************************
         * search ALL Open and Pending items
         **********************************************************/
        PCM_OP(ctxp, PCM_OP_SEARCH,
                PCM_OPFLG_READ_UNCOMMITTED , items_flistp, &r_flistp,ebufp);

        if (PIN_FLIST_FLD_COPY(r_flistp, PIN_FLD_RESULTS, o_flistp,
                PIN_FLD_ITEMS, ebufp) == 1) {
                result = PIN_BOOLEAN_TRUE;
        }

        PIN_FLIST_FLD_SET(o_flistp, PIN_FLD_BOOLEAN, (void *)&result, ebufp);

        /*********************************************************
         * Cleanup...
         *********************************************************/
        PIN_FLIST_DESTROY_EX(&items_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
        PIN_DECIMAL_DESTROY_EX(&zero_decimal);

        /*********************************************************
         * Errors..?
         *********************************************************/
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_pymt_pol_spec_collect_get_items error", ebufp);
        }

        return;
}

/**********************************************************************
 * fm_pymt_pol_spec_collect_get_ach()
 * Get ACH information
 **********************************************************************/
static void
fm_pymt_pol_spec_collect_get_ach(
	pcm_context_t	*ctxp,
	int32		flags,
	pin_flist_t	*i_flistp,
	pin_flist_t	*o_flistp,
	pin_errbuf_t	*ebufp)
{

	void		*vp		= NULL;
	pin_flist_t	*flistp		= NULL;
	pin_flist_t	*p_flistp	= NULL;
	pin_flist_t	*rp_flistp	= NULL;
	pin_flist_t	*a_flistp	= NULL;
	pin_flist_t	*ra_flistp	= NULL;
	int32		rec_id		= 0;
	poid_t		*p_pdp		= NULL;
	poid_t		*a_pdp		= NULL;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}

	PIN_ERR_CLEAR_ERR(ebufp);

	p_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PAYINFO_OBJ, 0, ebufp);
        a_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	
	/** read the payinfo to get the ach pointer. **/
	p_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(p_flistp, PIN_FLD_POID, p_pdp, ebufp);
	PIN_FLIST_FLD_SET(p_flistp, PIN_FLD_ACH, &rec_id, ebufp);
	
	PCM_OP(ctxp, PCM_OP_READ_FLDS, PCM_OPFLG_READ_UNCOMMITTED, 
					p_flistp, &rp_flistp, ebufp);	

	if (rp_flistp != (pin_flist_t *)NULL) {
		vp = PIN_FLIST_FLD_GET(rp_flistp, PIN_FLD_ACH, 0, ebufp);
		
		if (vp) {
			rec_id = *(int32 *)vp;
		}
                a_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_POID, a_pdp, ebufp);
                PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_ACH,
                                                (void *) &rec_id, ebufp);

                PCM_OP(ctxp, PCM_OP_PYMT_GET_ACH_INFO, 0, a_flistp,
                                                &ra_flistp, ebufp);

                flistp = PIN_FLIST_ELEM_GET(ra_flistp, PIN_FLD_RESULTS, 0,
                                                PIN_ELEMID_ANY, ebufp);

                vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_MERCHANT, 0, ebufp);
                PIN_FLIST_FLD_SET(o_flistp, PIN_FLD_MERCHANT, vp, ebufp);

                PIN_FLIST_FLD_SET(o_flistp, PIN_FLD_ACH, (void *) &rec_id, ebufp);

                vp = PIN_FLIST_FLD_TAKE(flistp, PIN_FLD_POID_VAL, 0, ebufp);

                flistp = PIN_FLIST_ELEM_ADD(o_flistp, PIN_FLD_BATCH_INFO, 0,
                                                                ebufp);
                PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID_VAL, vp, ebufp);
	}

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_pymt_pol_spec_collect_get_ach error", ebufp);
	}

	PIN_FLIST_DESTROY_EX(&p_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&rp_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&a_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&ra_flistp, NULL);

	return;
}
