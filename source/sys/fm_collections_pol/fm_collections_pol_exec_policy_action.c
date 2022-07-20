/* Continuus file information --- %full_filespec: fm_collections_pol_exec_policy_action.c~5:csrc:1 % */
/*
 * @(#) %full_filespec: fm_collections_pol_exec_policy_action.c~5:csrc:1 %
 *
 *      Copyright (c) 2001-2008 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 */

#ifndef lint
static const char Sccs_id[] = "@(#) %full_filespec: fm_collections_pol_exec_policy_action.c~5:csrc:1 %";
#endif

#include <stdio.h>
#include <string.h>

#include <pcm.h>
#include <pinlog.h>

#include "cm_fm.h"
#include "cm_cache.h"
#include "ops/collections.h"
#include "fm_utils.h"
#include "fm_collections.h"
#include "mso_ops_flds.h"

#ifdef MSDOS
#include <WINDOWS.h>
#endif

#define FILE_LOGNAME "fm_collections_pol_exec_policy_action.c(1)"

/*******************************************************************
 * Routines defined here.
 *******************************************************************/

EXPORT_OP void
op_collections_pol_exec_policy_action(
	cm_nap_connection_t	*connp, 
	int32			opcode, 
	int32			flags, 
	pin_flist_t		*in_flistp, 
	pin_flist_t		**ret_flistpp, 
	pin_errbuf_t		*ebufp);

static void
fm_collections_pol_exec_policy_action(
	pcm_context_t		*ctxp, 
	int32			flags, 
	pin_flist_t		*in_flistp, 
	pin_flist_t		**ret_flistpp, 
	pin_errbuf_t		*ebufp);

// MSO Customization Starts

#define GRACE_DAY_REMINDER "GRACE_DAY_REMINDER"
#define DOWNGRADE_PLAN "DOWNGRADE_SERVICE"
#define SERVICE_TERMINATION "SERVICE_TERMINATION"

#define NOTIFICATION 13

#define FTA_PLAN "Ala-Carte DD National"

static void
mso_collections_pol_send_reminder(
	pcm_context_t		*ctxp, 
	pin_flist_t		*in_flistp, 
	pin_flist_t		**ret_flistpp, 
	pin_errbuf_t		*ebufp);

static void
mso_collections_pol_downgrade_plan(
	pcm_context_t		*ctxp, 
	pin_flist_t		*in_flistp, 
	pin_flist_t		**ret_flistpp, 
	pin_errbuf_t		*ebufp);

static void
mso_collections_pol_terminate_service(
	pcm_context_t		*ctxp, 
	pin_flist_t		*in_flistp, 
	pin_flist_t		**ret_flistpp, 
	pin_errbuf_t		*ebufp);

PIN_IMPORT void
fm_mso_retrieve_service(
	pcm_context_t		*ctxp, 
	poid_t				*binfo_pdp, 
	pin_flist_t			**ret_flistpp, 
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * Variables/functions defined elsewhere
 *******************************************************************/
/*******************************************************************
 * Main routine for the PCM_OP_COLLECTIONS_POL_EXEC_POLICY_ACTION 
 *******************************************************************/
void
op_collections_pol_exec_policy_action(
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
	if (opcode != PCM_OP_COLLECTIONS_POL_EXEC_POLICY_ACTION) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_collections_pol_exec_policy_action opcode error", 
			ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_collections_pol_exec_policy_action input flist", in_flistp);

	/***********************************************************
	 * Do the actual op in a sub. Copy it since we may need to
	 * replace it later.
	 ***********************************************************/

	fm_collections_pol_exec_policy_action(ctxp, flags, in_flistp, 
			ret_flistpp, ebufp);

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_collections_pol_exec_policy_action error", ebufp);
	} else {
		/***************************************************
		 * Debug: What we're sending back.
		 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_collections_pol_exec_policy_action return flist", 
			*ret_flistpp);
	}
	return;
}

/*******************************************************************
 * fm_collections_pol_exec_policy_action()
 * Default behaviour of this function is to set the action status to
 * PENDING and return.
 * This function can be customized to perform custom collections 
 * actions and update the action status to COMPLETED. 
 *******************************************************************/
static void
fm_collections_pol_exec_policy_action(
	pcm_context_t		*ctxp, 
	int32			flags, 
	pin_flist_t		*in_flistp, 
	pin_flist_t		**ret_flistpp, 
	pin_errbuf_t		*ebufp)
{
	pin_action_status_t	action_status = PIN_ACTION_PENDING;
	void			*vp = NULL;
// MSO Customization Starts
	poid_t			*pdp = NULL;
	poid_t			*a_pdp = NULL;
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*rs_flistp = NULL;
	pin_flist_t		*sub_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	u_int64         id = (u_int64)-1;
	int32           s_flags = 512;
	int64           db = 0;
	char			*action_name = NULL;
// MSO Customization Ends

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	// MSO Customization Starts
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "POLICY_ACTION_ENTERED");
		a_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
		db = PIN_POID_GET_DB(a_pdp);
        
		s_flistp = PIN_FLIST_CREATE(ebufp);
		pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /config/collections/action/custom 1, /collections_action/policy_action 2 where 1.F1 = 2.F2 and 2.F3 = V3", ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, NULL, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_CONFIG_ACTION_OBJ, NULL, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, a_pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 1, ebufp);
		sub_flistp = PIN_FLIST_SUBSTR_ADD(flistp, PIN_FLD_CONFIG_ACTION_INFO, ebufp);
		PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_ACTION_NAME, NULL, ebufp);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Collections Actions search input flist", s_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &sub_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Collections Actions search output flist", sub_flistp);	
		rs_flistp = PIN_FLIST_FLD_GET(sub_flistp, PIN_FLD_RESULTS, 1, ebufp);
		flistp = PIN_FLIST_FLD_GET(rs_flistp, PIN_FLD_CONFIG_ACTION_INFO, 1, ebufp);
		action_name = (char *)PIN_FLIST_FLD_GET(flistp, PIN_FLD_ACTION_NAME, 1, ebufp);
	
		if (strcmp(action_name, GRACE_DAY_REMINDER) == 0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Sending Grace Days Reminder");
			mso_collections_pol_send_reminder(ctxp, in_flistp, &r_flistp, ebufp);
			action_status=PIN_ACTION_COMPLETED;
		}
		else if (strcmp(action_name, DOWNGRADE_PLAN) == 0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Downgrading the Service");
			mso_collections_pol_downgrade_plan(ctxp, in_flistp, &r_flistp, ebufp);
			action_status=PIN_ACTION_COMPLETED;
		}
		else if (strcmp(action_name, SERVICE_TERMINATION) == 0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Downgrading the Service");
			mso_collections_pol_terminate_service(ctxp, in_flistp, &r_flistp, ebufp);
			action_status=PIN_ACTION_COMPLETED;
		}
	/* Prepare the return flist */
	*ret_flistpp = PIN_FLIST_CREATE(ebufp);
	vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_POID, vp, ebufp);

	/* Default behavior */
	/* do nothing, simply set the status to pending */
	PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, 
			(void *)&action_status, ebufp);
	PIN_FLIST_DESTROY_EX(&s_flistp, ebufp);
	// MSO Customization Ends

}

static void
mso_collections_pol_send_reminder(
	pcm_context_t		*ctxp, 
	pin_flist_t		*in_flistp, 
	pin_flist_t		**ret_flistpp, 
	pin_errbuf_t		*ebufp)
{
	poid_t			*pdp = NULL;
	poid_t			*a_pdp = NULL;
//	poid_t			*binfo_pdp = NULL;
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*rs_flistp = NULL;
	pin_flist_t		*rs_flistp1 = NULL;
	pin_flist_t		*sub_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	pin_flist_t		*i_flistp = NULL;
	int32			flags = NOTIFICATION;
	int32           s_flags = 512;
	int32			osd = 1;
	int64           db = 0;
	u_int64         id = (u_int64)-1;
	
	a_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	db = PIN_POID_GET_DB(a_pdp);
        
		//Searching for Bill Number
		s_flistp = PIN_FLIST_CREATE(ebufp);
		pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /bill where F1 = V1 and F2 <> V2 order by F3 desc", ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_BILLINFO_OBJ, flistp, PIN_FLD_BILLINFO_OBJ, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_BILL_NO, "(null)", ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_CREATED_T, NULL, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 1, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_BILL_NO, NULL, ebufp);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Bill search input flist", s_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &sub_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Bill search output flist", sub_flistp);	
		rs_flistp = PIN_FLIST_FLD_GET(sub_flistp, PIN_FLD_RESULTS, 1, ebufp);
		
		//Retrieve Service POID
//		binfo_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_BILLINFO_OBJ, 0, ebufp);
//		fm_mso_retrieve_service(ctxp, binfo_pdp, ret_flistpp, ebufp);
//		rs_flistp1 = PIN_FLIST_FLD_GET(*ret_flistpp, PIN_FLD_RESULTS, 1, ebufp);


	//Preparing the Input Flist for Notification Opcode
	i_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_OBJ, i_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_COPY(rs_flistp, PIN_FLD_BILL_NO, i_flistp, PIN_FLD_BILL_NO, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_DUE, i_flistp, PIN_FLD_DUE, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_DUE_T, i_flistp, PIN_FLD_DUE_T, ebufp);

	PIN_FLIST_FLD_COPY(rs_flistp, PIN_FLD_POID, i_flistp, PIN_FLD_BILL_OBJ, ebufp);
	PIN_FLIST_FLD_SET(i_flistp, MSO_FLD_SEND_OSD, &osd, ebufp);

	//Calling Notification Opcode
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Reminder Notification input flist", i_flistp);
    PCM_OP(ctxp, MSO_OP_NTF_CREATE_NOTIFICATION, 0, i_flistp, ret_flistpp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Reminder Notification output flist", *ret_flistpp);	

	PIN_FLIST_DESTROY_EX(&s_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&i_flistp, ebufp);
	return;
}


static void
mso_collections_pol_downgrade_plan(
	pcm_context_t		*ctxp, 
	pin_flist_t		*in_flistp, 
	pin_flist_t		**ret_flistpp, 
	pin_errbuf_t		*ebufp)
{
	poid_t			*binfo_pdp = NULL;
	poid_t			*pdp = NULL;
	pin_flist_t		*i_flistp = NULL;
	pin_flist_t		*rs_flistp = NULL;
	pin_flist_t		*rss_flistp = NULL;
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*p_flistp = NULL;
	pin_flist_t		*d_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	u_int64         db = 0;
	u_int64         id = (u_int64)1;
	u_int64         id1 = (u_int64)0;
	int32			main_conn = 0;
	int32           s_flags = 256;

	binfo_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_BILLINFO_OBJ, 0, ebufp);
	db = PIN_POID_GET_DB(binfo_pdp);
		//Plan Details Retrieval from purchased_products table	
		s_flistp = PIN_FLIST_CREATE(ebufp);
		pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /purchased_product 1, /service/catv 2, /balance_group 3, /billinfo 4 where 1.F1 = 2.F2 and 2.F3 = V3 and 2.F4 = 3.F5 and 3.F6 = 4.F7 and 1.F8.id <> V8 and 4.F9 = V9", ebufp);

        flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, NULL, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
		flistp = PIN_FLIST_SUBSTR_ADD(flistp, MSO_FLD_CATV_INFO, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_CONN_TYPE, &main_conn, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_BAL_GRP_OBJ, NULL, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, NULL, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 6, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_BILLINFO_OBJ, NULL, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 7, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, NULL, ebufp);
		pdp = PIN_POID_CREATE(db, "/plan", id1, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 8, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 9, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, (void *)binfo_pdp, ebufp);
	    flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Plan Details Retrieval input flist", s_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, ret_flistpp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Plan Details Retrieval output flist", *ret_flistpp);
		rs_flistp = PIN_FLIST_FLD_GET(*ret_flistpp, PIN_FLD_RESULTS, 1, ebufp);
		PIN_FLIST_DESTROY_EX(&s_flistp, ebufp);

		//Preparing the Input Flist for MSO_OP_CUST_CHANGE_PLAN
		i_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_OBJ, i_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(rs_flistp, PIN_FLD_SERVICE_OBJ, i_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
		pdp = PIN_POID_CREATE(db, "/account", id, ebufp);
		PIN_FLIST_FLD_PUT(i_flistp, PIN_FLD_USERID, (void *)pdp, ebufp);
		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_PROGRAM_NAME, "Collections Action", ebufp);
		p_flistp = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_PLAN_LISTS, 0, ebufp);
		PIN_FLIST_FLD_SET(p_flistp, PIN_FLD_FLAGS, &id1, ebufp);
		PIN_FLIST_FLD_COPY(rs_flistp, PIN_FLD_PLAN_OBJ, p_flistp, PIN_FLD_PLAN_OBJ, ebufp);
		d_flistp = PIN_FLIST_ELEM_ADD(p_flistp, PIN_FLD_DEALS, 0, ebufp);
		PIN_FLIST_FLD_COPY(rs_flistp, PIN_FLD_DEAL_OBJ, d_flistp, PIN_FLD_DEAL_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(rs_flistp, PIN_FLD_PACKAGE_ID, d_flistp, PIN_FLD_PACKAGE_ID, ebufp);

		//Retrieve the FTA Plan Details for downgrading
		s_flistp = PIN_FLIST_CREATE(ebufp);
		pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /plan where F1 = V1", ebufp);

        flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_NAME, FTA_PLAN, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Plan Details Retrieval input flist", s_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, ret_flistpp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Plan Details Retrieval output flist", *ret_flistpp);
		rs_flistp = PIN_FLIST_FLD_GET(*ret_flistpp, PIN_FLD_RESULTS, 1, ebufp);
		rss_flistp = PIN_FLIST_ELEM_GET(rs_flistp, PIN_FLD_SERVICES, 0, 1, ebufp);


		//Continuation for Input Flist construction of MSO_OP_CUST_CHANGE_PLAN
		p_flistp = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_PLAN_LISTS, 1, ebufp);
		PIN_FLIST_FLD_SET(p_flistp, PIN_FLD_FLAGS, &id, ebufp);
		PIN_FLIST_FLD_COPY(rs_flistp, PIN_FLD_POID, p_flistp, PIN_FLD_PLAN_OBJ, ebufp);
		d_flistp = PIN_FLIST_ELEM_ADD(p_flistp, PIN_FLD_DEALS, 0, ebufp);
		PIN_FLIST_FLD_COPY(rss_flistp, PIN_FLD_DEAL_OBJ, d_flistp, PIN_FLD_DEAL_OBJ, ebufp);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Downgrade Service input flist", i_flistp);
        PCM_OP(ctxp, MSO_OP_CUST_CHANGE_PLAN, 0, i_flistp, ret_flistpp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Downgrade Service output flist", *ret_flistpp);

	PIN_FLIST_DESTROY_EX(&i_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&s_flistp, ebufp);
	return;
}


static void
mso_collections_pol_terminate_service(
	pcm_context_t		*ctxp, 
	pin_flist_t		*in_flistp, 
	pin_flist_t		**ret_flistpp, 
	pin_errbuf_t		*ebufp)
{
	poid_t			*binfo_pdp = NULL;
	poid_t			*pdp = NULL;
	int32			terminate_status = 10103;
	pin_flist_t		*i_flistp = NULL;
	pin_flist_t		*rs_flistp = NULL;
	u_int64         db = 0;
	u_int64         id = (u_int64)1;

	
	//Retrieve Service POID
	binfo_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_BILLINFO_OBJ, 0, ebufp);
	db = PIN_POID_GET_DB(binfo_pdp);
	fm_mso_retrieve_service(ctxp, binfo_pdp, ret_flistpp, ebufp);
	rs_flistp = PIN_FLIST_FLD_GET(*ret_flistpp, PIN_FLD_RESULTS, 1, ebufp);
	
	//Preparing Input Flist for service termination
	i_flistp = PIN_FLIST_CREATE(ebufp);
	pdp = PIN_POID_CREATE(db, "/account", id, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_OBJ, i_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_DESCR, "Terminate Service", ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_PROGRAM_NAME, "MSO Collections Action", ebufp);
	PIN_FLIST_FLD_PUT(i_flistp, PIN_FLD_USERID, pdp, ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_STATUS, &terminate_status, ebufp);
	PIN_FLIST_FLD_COPY(rs_flistp, PIN_FLD_POID, i_flistp, PIN_FLD_SERVICE_OBJ, ebufp);

	// Calling Terminate Service Opcode
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Terminate Service input flist", i_flistp);
    PCM_OP(ctxp, MSO_OP_CUST_TERMINATE_SERVICE, 0, i_flistp, ret_flistpp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Terminate Service output flist", *ret_flistpp);

	PIN_FLIST_DESTROY_EX(&i_flistp, ebufp);
	return;
}


