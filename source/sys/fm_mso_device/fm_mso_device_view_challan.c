/*
 *
 * Copyright (c) 2003, 2009, Oracle and/or its affiliates. 
 * All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 * 
 * Version 1.0: 04-11-2013: Vilva Sabarikanth: Added the MSO Customization for
 * searching the receipts using MSO_OP_DEVICE_VIEW_CHALLAN with account poid as input
 *
 */

#ifndef lint
static char Sccs_id[] = "@(#)$Id: fm_op_mso_device_view_challan.c  /cgbubrm_7.5.0. 2013/11/04 04:39:34 vsabarik Exp $";
#endif

/*******************************************************************
 * Contains the MSO_OP_DEVICE_VIEW_CHALLAN operation. 
 *******************************************************************/


#include <stdio.h> 
#include <string.h> 
#include <time.h>
 
#include "pcm.h"
#include "ops/pymt.h"
#include "pin_rate.h"
#include "pin_pymt.h"
#include "pin_bill.h"
#include "pin_cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "pin_ar.h"
#include "ops/bill.h"
#include "psiu_business_params.h"
#include "mso_ops_flds.h"
#include "mso_utils.h"


#define GST_PROFILE "/profile/gst_info"

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_mso_device_view_challan(
	cm_nap_connection_t	*connp,
	int					opcode,
	int					flags,
	pin_flist_t			*i_flistp,
	pin_flist_t			**o_flistpp,
	pin_errbuf_t		*ebufp);

static void mso_retrieve_challan_details(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flistp,
	pin_flist_t			**o_flistpp,
	pin_errbuf_t		*ebufp);


/*******************************************************************
 * Main routine for the MSO_OP_DEVICE_VIEW_CHALLAN operation.
 *******************************************************************/
void
op_mso_device_view_challan(
	cm_nap_connection_t	*connp,
	int					opcode,
	int					flags,
	pin_flist_t			*i_flistp,
	pin_flist_t			**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t			*r_flistp = NULL;
	pin_flist_t			*err_flistp = NULL;
	int32				status = 0;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != MSO_OP_DEVICE_VIEW_CHALLAN) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_op_mso_device_view_challan opcode error", ebufp);
		return;
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_mso_device_view_challan input flist", i_flistp);
	r_flistp = PIN_FLIST_CREATE(ebufp);
	//Retrieving the account poid from the i_flistp
	
	mso_retrieve_challan_details(ctxp, i_flistp, &r_flistp, ebufp);

	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		err_flistp = PIN_FLIST_CREATE(ebufp);
		status = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Payment Challans(/mso_repair_challan) Not Found");
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53010", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Error while searching challan for vendor", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_POID,err_flistp,PIN_FLD_POID,ebufp);
		PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_PROGRAM_NAME,err_flistp,PIN_FLD_PROGRAM_NAME,ebufp);
		PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_USERID,err_flistp,PIN_FLD_USERID,ebufp);
		*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
		goto CLEANUP;
	}
	else {

		/***************************************************
		 * Point the real return flist to the right thing.
		 ***************************************************/
		PIN_ERRBUF_CLEAR(ebufp);
		*o_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);
		if(PIN_FLIST_FLD_GET(r_flistp,PIN_FLD_STATUS,1,ebufp)){
			status = 1;
			PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_STATUS, &status, ebufp);
		}else{
			PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_STATUS, &status, ebufp);
		}
		PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_POID,*o_flistpp,PIN_FLD_POID,ebufp);
		PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_PROGRAM_NAME,*o_flistpp,PIN_FLD_PROGRAM_NAME,ebufp);
		PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_USERID,*o_flistpp,PIN_FLD_USERID,ebufp);
		/***************************************************
		 * Debug: What we're sending back.
		 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_mso_device_view_challan return flist", *o_flistpp);
	}

CLEANUP:
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_mso_device_view_challan return flist", *o_flistpp);
	return;
}

static void mso_retrieve_challan_details(
	pcm_context_t		*ctxp,
	pin_flist_t			*i_flistp,
	pin_flist_t			**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	u_int64			id = (u_int64)-1;
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*d_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*robj_iflistp = NULL;
	pin_flist_t		*robj_oflistp = NULL;
	pin_flist_t		*gst_flistp = NULL;
	pin_flist_t		*srch_prof_oflistp = NULL;
	pin_flist_t		*srch_prof_iflistp = NULL;
    	int32			s_flags = 512;
	int64			db = 0;
	poid_t			*pdp = NULL;
	poid_t			*acct_pdp = NULL;
	pin_cookie_t            cookie = NULL;
        int32                   rec_id = 0;
        int32                   status_fail = 1;
	int32           	profile_srch_type = MSO_FLAG_SRCH_BY_SELF_ACCOUNT;
	void			*vp = NULL;


	acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	db = PIN_POID_GET_DB(acct_pdp);

	s_flistp = PIN_FLIST_CREATE(ebufp);
	pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_VENDOR, flistp, PIN_FLD_VENDOR, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_RECEIPT_NO, flistp, PIN_FLD_RECEIPT_NO, ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Searching challan");
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /mso_repair_challan where F1 = V1 and F2 = V2 ", ebufp);
	res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Setting Results");
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_RECEIPT_NO, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_VENDOR, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_CREATED_T, NULL, ebufp);
	d_flistp = PIN_FLIST_ELEM_ADD(res_flistp, PIN_FLD_DEVICES, PIN_ELEMID_ANY, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Challan search input flist", s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Challan search output flist",r_flistp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Challan search Error",r_flistp);
	return;
	}
	flistp = PIN_FLIST_ELEM_GET(r_flistp,PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
	if(flistp){
		while((res_flistp = PIN_FLIST_ELEM_GET_NEXT(flistp, PIN_FLD_DEVICES, &rec_id, 1, &cookie, ebufp))!=(pin_flist_t*)NULL)
        	{
			PIN_FLIST_ELEM_COPY(flistp, PIN_FLD_DEVICES,rec_id,*r_flistpp,PIN_FLD_DEVICES,rec_id,ebufp);
		}
		PIN_FLIST_FLD_COPY(flistp, PIN_FLD_RECEIPT_NO, *r_flistpp, PIN_FLD_RECEIPT_NO, ebufp);
		PIN_FLIST_FLD_COPY(flistp, PIN_FLD_VENDOR, *r_flistpp, PIN_FLD_VENDOR, ebufp);
		PIN_FLIST_FLD_COPY(flistp, PIN_FLD_CREATED_T, *r_flistpp, PIN_FLD_CREATED_T, ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Challan search return flist",*r_flistpp);

	robj_iflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(robj_iflistp, PIN_FLD_POID, acct_pdp, ebufp);
        flistp = PIN_FLIST_ELEM_ADD(robj_iflistp, PIN_FLD_NAMEINFO, 1, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_CITY, NULL, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_STATE, NULL, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_COUNTRY, NULL, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_ZIP, NULL, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_COMPANY, NULL, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_ADDRESS, NULL, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_SALUTATION, NULL, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_FIRST_NAME, NULL, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_LAST_NAME, NULL, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_MIDDLE_NAME, NULL, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "READ FLDS FOR CITY INPUT", robj_iflistp);
        PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, robj_iflistp, &robj_oflistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "READ FLDS FOR CITY OUTPUT", robj_oflistp);
        robj_iflistp = PIN_FLIST_ELEM_GET(robj_oflistp, PIN_FLD_NAMEINFO, PIN_ELEMID_ANY, 1, ebufp);
	if(robj_iflistp){
	PIN_FLIST_FLD_COPY(robj_iflistp, PIN_FLD_CITY, *r_flistpp, PIN_FLD_CITY, ebufp);
	PIN_FLIST_FLD_COPY(robj_iflistp, PIN_FLD_STATE, *r_flistpp, PIN_FLD_STATE, ebufp);
	PIN_FLIST_FLD_COPY(robj_iflistp, PIN_FLD_COUNTRY, *r_flistpp, PIN_FLD_COUNTRY, ebufp);
	PIN_FLIST_FLD_COPY(robj_iflistp, PIN_FLD_ZIP, *r_flistpp, PIN_FLD_ZIP, ebufp);
	PIN_FLIST_FLD_COPY(robj_iflistp, PIN_FLD_COMPANY, *r_flistpp, PIN_FLD_COMPANY, ebufp);
	PIN_FLIST_FLD_COPY(robj_iflistp, PIN_FLD_ADDRESS, *r_flistpp, PIN_FLD_ADDRESS, ebufp);
	PIN_FLIST_FLD_COPY(robj_iflistp, PIN_FLD_SALUTATION, *r_flistpp, PIN_FLD_SALUTATION, ebufp);
	PIN_FLIST_FLD_COPY(robj_iflistp, PIN_FLD_FIRST_NAME, *r_flistpp, PIN_FLD_FIRST_NAME, ebufp);
	PIN_FLIST_FLD_COPY(robj_iflistp, PIN_FLD_LAST_NAME, *r_flistpp, PIN_FLD_LAST_NAME, ebufp);
	vp = PIN_FLIST_FLD_GET(robj_iflistp,PIN_FLD_MIDDLE_NAME,1,ebufp);
	if(vp && strlen(vp) > 0){
		PIN_FLIST_FLD_COPY(robj_iflistp, PIN_FLD_MIDDLE_NAME, *r_flistpp, PIN_FLD_MIDDLE_NAME, ebufp);
	}else{
		PIN_FLIST_FLD_SET(*r_flistpp,PIN_FLD_MIDDLE_NAME," ",ebufp);
	}
	}

	srch_prof_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(srch_prof_iflistp, PIN_FLD_POID, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_prof_iflistp, PIN_FLD_OBJECT, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_prof_iflistp, PIN_FLD_PROFILE_DESCR, GST_PROFILE, ebufp);
	PIN_FLIST_FLD_SET(srch_prof_iflistp, PIN_FLD_SEARCH_TYPE, &profile_srch_type, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , "fm_mso_device_view_challan get gst_profile input", srch_prof_iflistp);
	fm_mso_get_profile_info(ctxp, srch_prof_iflistp, &srch_prof_oflistp, ebufp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , "fm_mso_device_view_challan get gst_profile output", srch_prof_oflistp);
	PIN_FLIST_DESTROY_EX(&srch_prof_iflistp, NULL);
        if (!srch_prof_oflistp)
        {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "GST Profile object not found ");
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_fail , ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE , "51051" , ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "GST Profile object not found " , ebufp);
		goto CLEANUP;
	}

	gst_flistp = PIN_FLIST_SUBSTR_GET(srch_prof_oflistp,MSO_FLD_GST_INFO, 1, ebufp);
	if (gst_flistp)
	{
		PIN_FLIST_FLD_COPY(gst_flistp, PIN_FLD_ACCOUNT_NO, *r_flistpp, MSO_FLD_GSTIN, ebufp);
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"r_flistpp before returning",*r_flistpp);


CLEANUP:
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&robj_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_prof_oflistp, NULL);
	return;
}
