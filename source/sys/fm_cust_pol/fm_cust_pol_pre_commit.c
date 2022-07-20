/*******************************************************************
 *
 *      Copyright (c) 1999-2007 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char Sccs_Id[] = "@(#)%Portal Version: fm_cust_pol_pre_commit.c:RWSmod7.3.1Int:1:2007-Jun-28 05:36:33 %";
#endif

#include <stdio.h>
#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pinlog.h"

EXPORT_OP void
op_cust_pol_pre_commit(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp);

static void
fm_cust_pol_pre_commit(
	pcm_context_t		*ctxp,
	u_int           flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistpp,
        pin_errbuf_t		*ebufp);

static int32
fm_cust_pol_pre_commit_is_OOD_feature_enabled(
	pcm_context_t       *ctxp,
	pin_flist_t         *i_flistp,
	pin_errbuf_t        *ebufp);

static void
fm_cust_pol_pre_commit_create_profile_OOD_obj(
	pcm_context_t       *ctxp,
	u_int               flags,
	pin_flist_t         *i_flistp,
	pin_errbuf_t        *ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_PRE_COMMIT  command
 *******************************************************************/
void
op_cust_pol_pre_commit(
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
	if (opcode != PCM_OP_CUST_POL_PRE_COMMIT) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_cust_pol_pre_commit", ebufp);
		return;
	}

	/***********************************************************
	 * We will not open any transactions with Policy FM
	 * since policies should NEVER modify the database.
	 ***********************************************************/

	/***********************************************************
	 * Call main function to do it
	 ***********************************************************/
	fm_cust_pol_pre_commit(ctxp, flags, in_flistp, &r_flistp, ebufp);

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		*ret_flistpp = (pin_flist_t *)NULL;
		PIN_FLIST_DESTROY(r_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_cust_pol_pre_commit error", ebufp);
	} else {
		*ret_flistpp = r_flistp;
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_pre_commit return flist", r_flistp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_pre_commit()
 *
 *	Pre-creation legacy system hook.
 *
 *	XXX NOOP - STUBBED ONLY XXX
 *
 *******************************************************************/
static void
fm_cust_pol_pre_commit(
	pcm_context_t		*ctxp,
	u_int           flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistpp,
        pin_errbuf_t		*ebufp)
{
	void			*vp;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Create outgoing flist
	 ***********************************************************/
	*out_flistpp = PIN_FLIST_CREATE(ebufp);

	vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_POID, vp, ebufp);

	/***********************************************************
	 * Check if Out Of Order detection feature is enabled.
	 * If so, create the /profile/event_ordering object.
	 ***********************************************************/
	if (fm_cust_pol_pre_commit_is_OOD_feature_enabled(ctxp, in_flistp, ebufp)) {
		fm_cust_pol_pre_commit_create_profile_OOD_obj(ctxp, flags, in_flistp, ebufp);
	}	

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_pre_commit error", ebufp);
	}

	return;
}

static int32
fm_cust_pol_pre_commit_is_OOD_feature_enabled(
		pcm_context_t       *ctxp,
		pin_flist_t         *i_flistp,
		pin_errbuf_t        *ebufp) 
{
	pin_flist_t     *sch_flistp = NULL;     /* search flist */
	pin_flist_t     *arg_flistp = NULL;     /* arguments array element */
	pin_flist_t     *ret_flistp = NULL;     /* return flist */
	int32           flags = SRCH_DISTINCT;   /* search flag */
	poid_t          *sch_pdp = NULL;        /* search poid */
	int32           res_count = 0;
	void            *vp = NULL;
	char            *template = " select X from /config/event_order_criteria where F1 like V1 ";
	int32           rval = PIN_BOOLEAN_FALSE;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return rval;
	}
	PIN_ERR_CLEAR_ERR(ebufp);

	/*
	* Search for /config/event_order_criteria object.
	*/
	sch_flistp = PIN_FLIST_CREATE(ebufp);

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	sch_pdp = PIN_POID_CREATE(PIN_POID_GET_DB((poid_t*)vp), 
				"/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(sch_flistp, PIN_FLD_POID, (void *)sch_pdp, 
				ebufp);
	PIN_FLIST_FLD_SET(sch_flistp, PIN_FLD_FLAGS, (void *)&flags, 
				ebufp);
	PIN_FLIST_FLD_SET(sch_flistp, PIN_FLD_TEMPLATE, (void *)template, 
				ebufp);
	PIN_FLIST_ELEM_ADD(sch_flistp, PIN_FLD_RESULTS, 
				PIN_ELEMID_ANY, ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(sch_flistp, PIN_FLD_ARGS, 1, 
				ebufp);

	sch_pdp = NULL;
	sch_pdp = PIN_POID_CREATE(PIN_POID_GET_DB((poid_t*)vp), 
				"/config/event_order_criteria", -1, ebufp);
	PIN_FLIST_FLD_PUT(arg_flistp, PIN_FLD_POID, (void *)sch_pdp, ebufp);
	
	/* search the /config/event_order_criteria object  */

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_cust_pol_pre_commit_is_OOD_feature_enabled input flist for search", 
				sch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, sch_flistp, &ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&sch_flistp, NULL);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_pre_commit_is_OOD_feature_enabled error performing /config obj search", 
			ebufp);
		goto ErrOut;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_cust_pol_pre_commit_is_OOD_feature_enabled search result", 
			ret_flistp);

	res_count = PIN_FLIST_ELEM_COUNT(ret_flistp,
		PIN_FLD_RESULTS, ebufp);
	if (res_count)  {
		rval = PIN_BOOLEAN_TRUE;
	}	
	else  {
		rval = PIN_BOOLEAN_FALSE;
	}

ErrOut:
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
	return rval;
}

static void
fm_cust_pol_pre_commit_create_profile_OOD_obj(
	pcm_context_t   *ctxp,
	u_int           flags,
	pin_flist_t     *i_flistp,
	pin_errbuf_t    *ebufp)
{
	pin_flist_t *flistp = NULL;    /* i/p to PCM_OP_CREATE_OBJ */
	pin_flist_t *ret_flistp = NULL;    /* o/p to PCM_OP_CREATE_OBJ */
	poid_t      *p_pdp = NULL;

	void        *vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);


	/***********************************************************
	* Init the flist for creating the profile.
	***********************************************************/
	flistp = PIN_FLIST_CREATE(ebufp);

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	p_pdp = PIN_POID_CREATE(pin_poid_get_db((poid_t*)vp),
			"/profile/event_ordering", -1, ebufp);
	/***********************************************************
	* Create the flist for create_obj.
	***********************************************************/
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)p_pdp, ebufp);

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACCOUNT_OBJ, vp, ebufp);

	PIN_FLIST_FLD_SET(flistp, PIN_FLD_NAME, "cm", ebufp);

	/***********************************************************
	* Create the profile via a create_obj.
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"/profile/event_ordering creation input flist", flistp);

	PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, flistp, &ret_flistp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"/profile/event_ordering creation output flist", ret_flistp);

	/***********************************************************
	* Clean up.
	***********************************************************/
	PIN_FLIST_DESTROY_EX(&flistp, NULL);
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);

	/***********************************************************
	* Errors?
	***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"fm_cust_pol_pre_commit_create_profile_OOD_obj error", ebufp);
	}

	return;
}

