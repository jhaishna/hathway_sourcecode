/*
 *
 * Copyright (c) 2003, 2009, Oracle and/or its affiliates. 
 * All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 * 
 * Version 1.0: 29-11-2013: Vilva Sabarikanth: Added the MSO Customization for
 * searching the bulk payments using MSO_OP_PYMT_BULK_GET_PAYMENTS with bulk payment poid as input
 *
 */

#ifndef lint
static char Sccs_id[] = "@(#)$Id: fm_mso_pymt_blk_get_payments.c  /cgbubrm_7.5.0. 2013/11/04 04:39:34 vsabarik Exp $";
#endif

/*******************************************************************
 * Contains the MSO_OP_PYMT_BULK_GET_PAYMENTS operation. 
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


/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
mso_pymt_blk_get_payments(
        cm_nap_connection_t	*connp,
	int			opcode,
        int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp);

EXPORT_OP static void mso_retrieve_blk_payments(
        pcm_context_t     	*ctxp,
		pin_flist_t         *i_flistp,
        pin_flist_t         **o_flistpp,
        pin_errbuf_t        *ebufp);


/*******************************************************************
 * Main routine for the MSO_OP_PYMT_BULK_GET_PAYMENTS operation.
 *******************************************************************/
void
mso_pymt_blk_get_payments(
        cm_nap_connection_t	*connp,
	int			opcode,
        int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*err_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	poid_t			*blk_pymt_pdp = NULL;
	int32			status = 0;
		
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != MSO_OP_PYMT_BULK_GET_PAYMENTS) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_pymt_blk_get_payments opcode error", ebufp);
		return;
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"mso_pymt_blk_get_payments input flist", i_flistp);
	r_flistp = PIN_FLIST_CREATE(ebufp);
	blk_pymt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

	mso_retrieve_blk_payments(ctxp, i_flistp, &r_flistp, ebufp);

	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
	flistp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_RESULTS, 0, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{	
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "No Payments Found");
		PIN_ERRBUF_CLEAR(ebufp);
		err_flistp = PIN_FLIST_CREATE(ebufp);
		status = 1;
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, blk_pymt_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53036", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "No Payments Found", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		goto Cleanup;
	}

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp)) {

		/***************************************************
		 * Log something and return nothing.
		 ***************************************************/
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"mso_pymt_blk_get_payments error", ebufp);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		*o_flistpp = NULL;

	} else {

		/***************************************************
		 * Point the real return flist to the right thing.
		 ***************************************************/
		PIN_ERRBUF_CLEAR(ebufp);
		*o_flistpp = r_flistp;

		/***************************************************
		 * Debug: What we're sending back.
		 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"mso_pymt_blk_get_payments return flist", *o_flistpp);
	}
	Cleanup:
//	PIN_FLIST_DESTROY_EX(&r_flistp, ebufp);
//	PIN_FLIST_DESTROY_EX(&err_flistp, ebufp);
	return;
}

EXPORT_OP static void mso_retrieve_blk_payments(
        pcm_context_t     	*ctxp,
        pin_flist_t         *i_flistp,
        pin_flist_t         **r_flistpp,
        pin_errbuf_t        *ebufp)
{
	pin_flist_t     *s_flistp = NULL;
	pin_flist_t	*flistp = NULL;
	poid_t          *pdp = NULL;
	poid_t		*lco_pdp = NULL;
	poid_t		*user_pdp = NULL;
	int64           db = 1;
	int32		*d_status = NULL;
	int32		*flags = NULL;
	int32		*dn_flags = NULL;
	int32            s_flags = 512;
	char		*ref_id = NULL;
	u_int64         id = (u_int64)-1;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"mso_retrieve_blk_payments input flist", i_flistp);

	flags = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp);
	d_status = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_STATUS, 1, ebufp);
	dn_flags = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DN_FLAGS, 1, ebufp); 

	//pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp));
        s_flistp = PIN_FLIST_CREATE(ebufp);
        pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
	// Flag 0 is for Master Object and Flag 1 for Individual Objects
	if (flags && *flags == 0)
	{

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Retreiving payment bulk order");
		if (d_status && *d_status == 1)//LCO accont obj & date range
		{
			lco_pdp = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_LCO_OBJ, 0, ebufp);
			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
			PIN_FLIST_FLD_SET(flistp, MSO_FLD_LCO_OBJ, (void *)lco_pdp, ebufp);

			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_START_T, flistp, PIN_FLD_CREATED_T, ebufp);

			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_END_T, flistp, PIN_FLD_CREATED_T, ebufp);

			PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /mso_lco_blk_payment where F1 = V1 and (F2 >= V2 and F3 <= V3) ", ebufp);

		}
		else if (d_status && *d_status == 2)//User account obj & date range
		{
			user_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 0, ebufp);

			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_USERID, (void *)user_pdp, ebufp);

			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_START_T, flistp, PIN_FLD_CREATED_T, ebufp);

			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_END_T, flistp, PIN_FLD_CREATED_T, ebufp);

			PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /mso_lco_blk_payment where F1 = V1 and (F2 >= V2 and F3 <= V3) ", ebufp);

		}
		else if (d_status && *d_status == 3)//with ref_id & date range 
		{
			ref_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_REFERENCE_ID, 0, ebufp);

			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_REFERENCE_ID, (void *)ref_id, ebufp);

			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_START_T, flistp, PIN_FLD_CREATED_T, ebufp);

			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_END_T, flistp, PIN_FLD_CREATED_T, ebufp);

			PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /mso_lco_blk_payment where F1 = V1 and (F2 >= V2 and F3 <= V3) ", ebufp);

		}
		else//with date range  only
		{
			
			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_START_T, flistp, PIN_FLD_CREATED_T, ebufp);

			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_END_T, flistp, PIN_FLD_CREATED_T, ebufp);

			PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /mso_lco_blk_payment where F1 >= V1 and F2 <= V2 ", ebufp);
		}
		
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, MSO_FLD_LCO_OBJ, NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_CREATED_T, NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, MSO_FLD_AMOUNT_TOTAL, NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, MSO_FLD_PYMT_COLLECTION_T, NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_REFERENCE_ID, NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACCOUNT_NO, NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_PROGRAM_NAME, NULL, ebufp);
	}
	else if ((flags && *flags == 1) && (dn_flags && *dn_flags == 1))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Download Bulk Order");
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /mso_lco_payment where F1 = V1 and F2 = V2 ", ebufp);
		
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
		//PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, flistp, MSO_FLD_LCO_BLK_PYMT_OBJ, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_STATUS, flistp, MSO_FLD_PYMT_STATUS, ebufp);

		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	}

	else 
	{
//		if (*d_status == 3)
//		{
//			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
//			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_START_T, flistp, PIN_FLD_CREATED_T, ebufp);
//			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
//	        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_END_T, flistp, PIN_FLD_CREATED_T, ebufp);
//			PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /mso_lco_payment 1,  where F1 = V1 and (F2 >= V2 and F3 <= V3)", ebufp);
//		}
//		else 
//		{
//			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
//			PIN_FLIST_FLD_SET(flistp, MSO_FLD_PYMT_STATUS, d_status, ebufp);
//			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
//			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_START_T, flistp, PIN_FLD_CREATED_T, ebufp);
//			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
//	        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_END_T, flistp, PIN_FLD_CREATED_T, ebufp);
//			PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /mso_lco_payment where F1 = V1 and F2 = V2 and (F3 >= V3 and F4 <= V4)", ebufp);
//		}
//		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
//	    PIN_FLIST_FLD_SET(flistp, MSO_FLD_LCO_OBJ, (void *)lco_pdp, ebufp);

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Retreiving payment bulk order");
		if (d_status && *d_status == 1)//LCO accont obj & date range
		{
			lco_pdp = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_LCO_OBJ, 0, ebufp);
			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
			PIN_FLIST_FLD_SET(flistp, MSO_FLD_LCO_OBJ, (void *)lco_pdp, ebufp);

			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_START_T, flistp, PIN_FLD_CREATED_T, ebufp);

			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_END_T, flistp, PIN_FLD_CREATED_T, ebufp);

			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID,NULL, ebufp);

			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
			PIN_FLIST_FLD_SET(flistp, MSO_FLD_LCO_BLK_PYMT_OBJ,NULL, ebufp);
	

			PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /mso_lco_payment 1, /mso_lco_blk_payment 2  where 2.F1 = V1 and (2.F2 >= V2 and 2.F3 <= V3) and 2.F4 = 1.F5 ", ebufp);

		}
		else if (d_status && *d_status == 2)//User account obj & date range
		{
			user_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 0, ebufp);

			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_USERID, (void *)user_pdp, ebufp);

			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_START_T, flistp, PIN_FLD_CREATED_T, ebufp);

			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_END_T, flistp, PIN_FLD_CREATED_T, ebufp);

			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID,NULL, ebufp);

			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
			PIN_FLIST_FLD_SET(flistp, MSO_FLD_LCO_BLK_PYMT_OBJ,NULL, ebufp);


			PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /mso_lco_payment 1, /mso_lco_blk_payment 2  where 2.F1 = V1 and (2.F2 >= V2 and 2.F3 <= V3) and 2.F4 = 1.F5 ", ebufp);

		}
		else if (d_status && *d_status == 3)//with ref_id & date range 
		{
			ref_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_REFERENCE_ID, 0, ebufp);

			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_REFERENCE_ID, (void *)ref_id, ebufp);

			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_START_T, flistp, PIN_FLD_CREATED_T, ebufp);

			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_END_T, flistp, PIN_FLD_CREATED_T, ebufp);

			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID,NULL, ebufp);

			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
			PIN_FLIST_FLD_SET(flistp, MSO_FLD_LCO_BLK_PYMT_OBJ,NULL, ebufp);

			PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /mso_lco_payment 1, /mso_lco_blk_payment 2  where 2.F1 = V1 and (2.F2 >= V2 and 2.F3 <= V3) and 2.F4 = 1.F5 ", ebufp);

		}
		else//with date range  only
		{
			
			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_START_T, flistp, PIN_FLD_CREATED_T, ebufp);

			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_END_T, flistp, PIN_FLD_CREATED_T, ebufp);

			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID,NULL, ebufp);

			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
			PIN_FLIST_FLD_SET(flistp, MSO_FLD_LCO_BLK_PYMT_OBJ,NULL, ebufp);

			PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /mso_lco_payment 1, /mso_lco_blk_payment 2  where (2.F1 >= V1 and 2.F2 <= V2) and 2.F3 = 1.F4 ", ebufp);
		}
		

		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
/*		PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, MSO_FLD_LCO_OBJ, NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACCOUNT_NO, NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, MSO_FLD_PYMT_PROCESSING_T, NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, MSO_FLD_SERVICE_TYPE, NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_PAY_TYPE, NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_AMOUNT, NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, MSO_FLD_PYMT_STATUS, NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_ERROR_CODE, NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_ERROR_DESCR, NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_REFERENCE_ID, NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_BANK_CODE, NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_BANK_ACCOUNT_NO, NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_BANK_NAME, NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_BRANCH_NO, NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_CHECK_NO, NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, MSO_FLD_PYMT_COLLECTION_T, NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_START_T, NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_END_T, NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_USERID, NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_DESCR, NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_IP_ADDRESS, NULL, ebufp);
*/
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Bulk Payments search input flist", s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, r_flistpp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Bulk Payments search output flist",*r_flistpp);
	PIN_FLIST_DESTROY_EX(&s_flistp, ebufp);

	return;
}
