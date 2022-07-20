/*
 *      Copyright (c) 2001-2006 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 */

#ifndef lint
static const char Sccs_id[] = "@(#) %full_filespec: fm_collections_pol_select_profile.c~2:csrc:1 %";
#endif

#include <stdio.h>
#include <string.h>

#include <pcm.h>
#include <pinlog.h>

#include "cm_fm.h"
#include "cm_cache.h"
#include "ops/collections.h"
#include "fm_utils.h"
#include "pin_cust.h"
#include "fm_collections.h"
#include "mso_ops_flds.h"

#ifdef MSDOS
#include <WINDOWS.h>
#endif

#define BB_BILLINFO "BB"
#define CATV_BILLINFO "CATV"
#define CATV_COLL_PROFILE "MSO_CATV"
#define BB_COLL_PROFILE "MSO_BB"


/*******************************************************************
 * Routines defined here.
 *******************************************************************/

EXPORT_OP void
op_collections_pol_select_profile(
	cm_nap_connection_t	*connp, 
	int32			opcode, 
	int32			flags, 
	pin_flist_t		*in_flistp, 
	pin_flist_t		**ret_flistpp, 
	pin_errbuf_t		*ebufp);

static void
fm_collections_pol_select_profile(
	pcm_context_t		*ctxp, 
	int32			flags, 
	pin_flist_t		*in_flistp, 
	pin_flist_t		**ret_flistpp, 
	pin_errbuf_t		*ebufp);

PIN_IMPORT void
fm_mso_retrieve_service(
        pcm_context_t           *ctxp,
        poid_t                  *binfo_pdp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

/*******************************************************************
 * Variables/functions defined elsewhere
 *******************************************************************/

/*******************************************************************
 * Main routine for the PCM_OP_COLLECTIONS_POL_SELECT_PROFILE 
 *******************************************************************/
void
op_collections_pol_select_profile(
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
	if (opcode != PCM_OP_COLLECTIONS_POL_SELECT_PROFILE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_collections_pol_select_profile opcode error", 
			ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_collections_pol_select_profile input flist", in_flistp);

	/***********************************************************
	 * Do the actual op in a sub. Copy it since we may need to
	 * replace it later.
	 ***********************************************************/

	fm_collections_pol_select_profile(ctxp, flags, in_flistp, 
			ret_flistpp, ebufp);

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_collections_pol_select_profile error", ebufp);
	} else {
		/***************************************************
		 * Debug: What we're sending back.
		 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_collections_pol_select_profile return flist", 
			*ret_flistpp);
	}
	return;
}

/*******************************************************************
 * fm_collections_pol_select_profile()
 *
 * This function calls corresponding action opcode and updates status.
 *
 *******************************************************************/
static void
fm_collections_pol_select_profile(
	pcm_context_t		*ctxp, 
	int32			flags, 
	pin_flist_t		*in_flistp, 
	pin_flist_t		**ret_flistpp, 
	pin_errbuf_t		*ebufp)
{
	poid_t			*a_pdp = NULL;
	poid_t			*b_pdp = NULL;
	poid_t			*p_pdp = NULL;

// MSO Customization Starts
	poid_t			*s_pdp = NULL;
	poid_t			*pdp = NULL;
	pin_flist_t		*s_flistp = NULL;
        pin_flist_t             *o_flistp = NULL;
	pin_flist_t		*sub_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	pin_flist_t		*billinfo_inread = NULL;
	pin_flist_t		*billinfo_outread = NULL;
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*rs_flistp = NULL;
	u_int64         id = (u_int64)-1;
	int32           s_flags = 512;
	int64           db = 0;
	char		*billinfo_id = NULL;
	char			*serv_type = NULL;
// MSO Customization Ends

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/* Customers should change this policy to return a config profile */
	/* pointer for a given account.  One should define several profiles */
	/* from the configuration center and map the given account to */
	/* one of the profiles based on account related information. */
	/* By default, this policy returns the default profile. */
	*ret_flistpp = PIN_FLIST_CREATE(ebufp);
	billinfo_inread = PIN_FLIST_CREATE(ebufp);

	a_pdp = (poid_t *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	b_pdp = (poid_t *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_BILLINFO_OBJ, 0, ebufp);

	/* Logic to check if the account bill-unit belongs to Broadband Service */
//	fm_mso_retrieve_service(ctxp, b_pdp, &ret_flistp, ebufp);
//	rs_flistp = PIN_FLIST_ELEM_GET(ret_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
//	s_pdp = PIN_FLIST_FLD_GET(rs_flistp, PIN_FLD_POID, 0, ebufp);
//	serv_type = (char *)PIN_POID_GET_TYPE(s_pdp);
// Read billinfo id for the billnfo

	PIN_FLIST_FLD_SET(*ret_flistpp, 
				PIN_FLD_POID, a_pdp, ebufp);

	PIN_FLIST_FLD_SET(*ret_flistpp, 
				PIN_FLD_BILLINFO_OBJ, b_pdp, ebufp);

	
	PIN_FLIST_FLD_SET(billinfo_inread, PIN_FLD_POID, b_pdp, ebufp);
	PIN_FLIST_FLD_SET(billinfo_inread, PIN_FLD_BILLINFO_ID, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read  billinfo in flist", billinfo_inread);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, billinfo_inread, &billinfo_outread, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read  billinfo out flist", billinfo_outread);
	if(billinfo_outread)
	{
		billinfo_id = PIN_FLIST_FLD_GET(billinfo_outread, PIN_FLD_BILLINFO_ID,0,ebufp);	
	}
	
	// MSO Customization Starts
	s_flistp = PIN_FLIST_CREATE(ebufp);
	db = PIN_POID_GET_DB(a_pdp);
        pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /config/collections/profile where F1 = V1", ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	sub_flistp = PIN_FLIST_SUBSTR_ADD(flistp, PIN_FLD_PROFILE_INFO, ebufp);
	/* MSO Logic to attach the service based collection profile */
	if (billinfo_id && strcmp(billinfo_id, BB_BILLINFO) == 0)
	{
		PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_PROFILE_NAME, BB_COLL_PROFILE, ebufp);
	}
	else if (billinfo_id && strcmp(billinfo_id, CATV_BILLINFO) == 0)
	{

		PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_PROFILE_NAME, CATV_COLL_PROFILE, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_PROFILE_NAME, "NONE", ebufp);//set nne to handle search
	}
	
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 1, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Collections Profile search input flist", s_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &o_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Collections Profile search output flist", o_flistp);	
	flistp = PIN_FLIST_ELEM_GET(o_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	p_pdp = (poid_t *)PIN_FLIST_FLD_TAKE(flistp, PIN_FLD_POID, 1, ebufp);
	
        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);	
        PIN_FLIST_DESTROY_EX(&o_flistp, NULL); 
	PIN_FLIST_DESTROY_EX(&billinfo_inread, NULL);	
        PIN_FLIST_DESTROY_EX(&billinfo_outread, NULL); 
/*	p_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(a_pdp), 
			"/config/collections/profile", 
			369049, ebufp);
*/
		// MSO Customization Ends
	PIN_FLIST_FLD_PUT(*ret_flistpp, 
				PIN_FLD_CONFIG_PROFILE_OBJ, p_pdp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Collection final flist", *ret_flistpp);
}
