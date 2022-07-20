/*
 *
 * Copyright (c) 2003, 2009, Oracle and/or its affiliates. 
 * All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 *
 * Version 1.0: 08-10-2013: Vilva Sabarikanth: Added the MSO Customization of
 * Deposit Payments using MSO_OP_PYMT_CREATE_DEPOSIT opcode and deposit object creation
 * for deposit refunds based on refund rule.
 *
 */

#ifndef lint
static char Sccs_id[] = "@(#)$Id: fm_mso_pymt_deposit.c /cgbubrm_7.5.0. 2013/09/26 04:39:34 vsabarik Exp $";
#endif

/*******************************************************************
 * Contains the MSO_OP_PYMT_CREATE_DEPOSIT operation. 
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
#include "mso_ops_flds.h" 
#include "mso_lifecycle_id.h"

#define DEP_PRIORITY "20.0"
#define SEQ_TYPE_DEPOSIT "MSO_SEQUENCE_TYPE_DEPOSIT"

#define ITEM_STATUS_AVAIL 0
#define ITEM_STATUS_PENDING 1
#define ITEM_STATUS_OPEN 2
#define DEP_STATUS_OPEN 0
#define BROADBAND_SERVICE_TYPE "/service/telco/broadband"
#define PYMT_DEPOSIT "PYMT_DEPOSIT"

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
mso_pymt_create_deposit(
	cm_nap_connection_t	*connp,
	int			opcode,
	int			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);


EXPORT_OP void mso_item_status_update(
	pcm_context_t		*ctxp,
	pin_flist_t		*item_flistp,
	pin_flist_t		**s_flistp,
	pin_errbuf_t		*ebufp);

PIN_IMPORT void fm_mso_utils_sequence_no(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

extern
void fm_mso_create_lifecycle_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	int32			action,
	pin_errbuf_t		*ebufp);

void
mso_pymt_create_bb_deposit(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

void
mso_pymt_create_catv_deposit(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

//static void
//mso_op_deposit_gen_lc_event(
//	pcm_context_t           *ctxp,
//	pin_flist_t             *i_flistp,
//	pin_flist_t             *dep_flistp,
//	pin_decimal_t		*dep_total,
//	char			*dep_no,
//	char                    *disp_name,
//	pin_errbuf_t            *ebufp);

PIN_IMPORT
void fm_mso_utils_gen_event(
	pcm_context_t   *ctxp,
	poid_t          *acct_pdp,
	poid_t          *serv_pdp,
	char            *program,
	char            *descr,
	char            *event_type,
	pin_flist_t     *in_flistp,
	pin_errbuf_t    *ebufp);

PIN_IMPORT
void fm_mso_utils_read_any_object(
	pcm_context_t           *ctxp,
	poid_t                  *objp,
	pin_flist_t             **out_flistpp,
	pin_errbuf_t            *ebufp);

/*******************************************************************
 * Main routine for the MSO_OP_PYMT_CREATE_DEPOSIT operation.
 *******************************************************************/
void
mso_pymt_create_deposit(
	cm_nap_connection_t	*connp,
	int			opcode,
	int			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*r_flistp = NULL;
	poid_t			*serv_pdp = NULL;
	char			*serv_type = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != MSO_OP_PYMT_CREATE_DEPOSIT) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"mso_op_pymt_create_deposit opcode error", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"mso_op_pymt_create_deposit input flist", i_flistp);

	serv_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
	serv_type = (char *)PIN_POID_GET_TYPE(serv_pdp);
	if( serv_type && strcmp(serv_type, BROADBAND_SERVICE_TYPE) == 0)
	{
		mso_pymt_create_bb_deposit(ctxp, i_flistp, &r_flistp, ebufp);
	}
	else
	{
		mso_pymt_create_catv_deposit(ctxp, i_flistp, &r_flistp, ebufp);
	}

	*o_flistpp = PIN_FLIST_COPY(r_flistp,ebufp);
		
	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "mso_op_pymt_create_deposit error ");
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"mso_op_pymt_create_deposit error flist", *o_flistpp);
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"mso_op_pymt_create_deposit return flist", *o_flistpp);
	}

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&r_flistp, ebufp);

	return;
}

void
mso_pymt_create_bb_deposit(
	pcm_context_t		*ctxp,
	pin_flist_t			*i_flistp,
	pin_flist_t			**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*in_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*rs_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*bal_flistp = NULL;
	pin_flist_t		*dep_flistp = NULL;
	pin_flist_t		*device_flistp = NULL;
	pin_flist_t		*item_flistp = NULL;
	pin_flist_t		*err_flistp = NULL;
	pin_flist_t		*item_arrp = NULL;
	pin_flist_t		*notif_flistp = NULL;
	pin_flist_t		*it_flistp = NULL;
	time_t			now_t = 0;
	poid_t			*binfo_pdp = NULL;
	poid_t			*item_pdp = NULL;
	poid_t			*dep_pdp = NULL;
	poid_t			*device_pdp = NULL;
	poid_t			*p_pdp = NULL;
	poid_t			*a_pdp = NULL;
	pin_decimal_t	*dep_actual_amt = NULL;
	pin_decimal_t	*dep_amt = NULL;
	pin_decimal_t	*dep_total = NULL;
	char			*dep_no = NULL;
	char			*device_id = NULL;
	u_int64			db = 1;
	u_int64			id = (u_int64)-1;
	int32			i_status = ITEM_STATUS_AVAIL;
	int32			d_status = DEP_STATUS_OPEN;
	int32			status = 0;
	time_t			tstamp = 0;
	int				d_flag = 0;
	int32			*impact_type = NULL;
	int32			rec_id1 = 0;
	int32			rec_id2 = 0;
	int32			notif = 5;
	pin_cookie_t	cookie1 = NULL;
	pin_cookie_t	cookie2 = NULL;

	char            search_template[100] = " select X from /device where F1 = V1 ";
	int             search_flags = 768;
	pin_flist_t     *args_flistp = NULL;
	pin_flist_t     *search_inflistp = NULL;
	pin_flist_t     *search_outflistp = NULL;
	pin_flist_t     *results_flistp = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	a_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	dep_flistp = PIN_FLIST_CREATE(ebufp);
	db = PIN_POID_GET_DB(a_pdp);
	dep_pdp = PIN_POID_CREATE(db, "/mso_deposit",id, ebufp);
	PIN_FLIST_FLD_PUT(dep_flistp, PIN_FLD_POID, (void *)dep_pdp, ebufp);

	dep_total = pbo_decimal_from_str("0.0", ebufp);
	/*************************************************************
	 * Looping through the PIN_FLD_RESULTS and then through PIN_FLD_BAL_IMPACTS
	 * to prepare the deposit flist.
	 *************************************************************/
	while((rs_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_RESULTS, 
		&rec_id1, 1, &cookie1, ebufp))!=(pin_flist_t*)NULL)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Results Flist:",rs_flistp);
		rec_id2 = 0;
		cookie2 = NULL;
		while((bal_flistp = PIN_FLIST_ELEM_GET_NEXT(rs_flistp, PIN_FLD_BAL_IMPACTS, 
			&rec_id2, 1, &cookie2, ebufp))!=(pin_flist_t*)NULL)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"bal_flistp:",bal_flistp);
			impact_type = PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_IMPACT_TYPE, 0, ebufp);
			dep_amt = (pin_decimal_t *)PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_AMOUNT, 0, ebufp);
			item_pdp = PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_ITEM_OBJ, 0, ebufp);
			
			/***********************************************************
			 * Update the item object status
			 ***********************************************************/
/*
			item_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(item_flistp, PIN_FLD_POID, (void *)item_pdp, ebufp);
			PIN_FLIST_FLD_SET(item_flistp, PIN_FLD_STATUS, (void *)&i_status, ebufp);

			r_flistp = NULL;
			mso_item_status_update(ctxp, item_flistp, &r_flistp, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERRBUF_CLEAR(ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, 
							"Error updating the deposit product item status");
				err_flistp = PIN_FLIST_CREATE(ebufp);
				status = 1;
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, binfo_pdp, ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53042", ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, 
							"Error updating the deposit product item status", ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
				*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&err_flistp, ebufp);
				goto CLEANUP;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"Item Status update output flist", r_flistp);
			PIN_FLIST_DESTROY_EX(&item_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&r_flistp, ebufp);
*/
			/***********************************************************
			 * Creating the Deposit Flist
			 ************************************************************/
			if(impact_type && *impact_type != PIN_IMPACT_TYPE_TAX)
			{
				db = PIN_POID_GET_DB(item_pdp);
				now_t = pin_virtual_time((time_t *)NULL);
						
				in_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, item_pdp, ebufp);
				PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_NAME, SEQ_TYPE_DEPOSIT, ebufp);
				
				r_flistp = NULL;
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Deposit Object Sequence Input Flist", in_flistp);
				fm_mso_utils_sequence_no(ctxp, in_flistp, &r_flistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Deposit Object Sequence Output Flist", r_flistp);
				dep_no = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_STRING, 0, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERRBUF_CLEAR(ebufp);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Cannot find /data/sequence object");
					err_flistp = PIN_FLIST_CREATE(ebufp);
					status = 1;
					PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, binfo_pdp, ebufp);
					PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53042", ebufp);
					PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Cannot find /data/sequence object", ebufp);
					PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
					*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
					PIN_FLIST_DESTROY_EX(&err_flistp, ebufp);
					goto CLEANUP;
				}

				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, dep_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DEVICE_OBJ, dep_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
				PIN_FLIST_FLD_SET(dep_flistp, MSO_FLD_DEPOSIT_DATE, &now_t, ebufp);
				PIN_FLIST_FLD_SET(dep_flistp, MSO_FLD_DEPOSIT_NO, (char *)dep_no, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_CODE, dep_flistp, MSO_FLD_REFUND_RULE, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_NAME, dep_flistp, PIN_FLD_DESCR, ebufp);
				PIN_FLIST_FLD_SET(dep_flistp, PIN_FLD_STATUS, (void *)&d_status, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, dep_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, dep_flistp, PIN_FLD_USERID, ebufp);

				item_arrp = PIN_FLIST_ELEM_ADD(dep_flistp, PIN_FLD_ITEMS, rec_id2, ebufp);
				PIN_FLIST_FLD_SET(item_arrp, PIN_FLD_ITEM_OBJ, item_pdp, ebufp);
				PIN_FLIST_FLD_SET(item_arrp, PIN_FLD_AMOUNT, dep_amt, ebufp);
				dep_actual_amt = pbo_decimal_copy(dep_amt, ebufp);
				pbo_decimal_add_assign(dep_total, dep_amt, ebufp);
			}
			else if (impact_type && *impact_type == PIN_IMPACT_TYPE_TAX)
			{
				item_arrp = PIN_FLIST_ELEM_ADD(dep_flistp, PIN_FLD_ITEMS, rec_id2, ebufp);
				PIN_FLIST_FLD_SET(item_arrp, PIN_FLD_ITEM_OBJ, item_pdp, ebufp);
				PIN_FLIST_FLD_SET(item_arrp, PIN_FLD_AMOUNT, dep_amt, ebufp);
				PIN_FLIST_FLD_COPY(bal_flistp, PIN_FLD_IMPACT_TYPE, item_arrp, 
									PIN_FLD_IMPACT_TYPE, ebufp);
				//pbo_decimal_add_assign(dep_total, dep_amt, ebufp);
			}

			d_flag = 1;
		}
		/* We need to parse only once if PIN_FLD_BAL_IMPACTS array is processed */
		if(d_flag)
			break;
	}

	device_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_DEVICES, PIN_ELEMID_ANY, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		err_flistp = PIN_FLIST_CREATE(ebufp);
		status = 1;
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, a_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53043", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "No Device passed for Deposit", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&err_flistp, ebufp);
		goto CLEANUP;
	}
	if(device_flistp)
	{
		device_id = PIN_FLIST_FLD_GET(device_flistp, PIN_FLD_DEVICE_ID , 1, ebufp);
		
		/*************
		 * search flist to search device poid
		 ************/
		search_inflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
		PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DEVICE_ID, device_id, ebufp);
		results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_device_object_with_plan search device input list", search_inflistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);

		if ((PIN_ERRBUF_IS_ERR(ebufp)) || (PIN_FLIST_ELEM_COUNT(search_outflistp , PIN_FLD_RESULTS , ebufp) == 0))
		{
			PIN_ERRBUF_CLEAR(ebufp);
			err_flistp = PIN_FLIST_CREATE(ebufp);
			status = 1;
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, a_pdp, ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53044", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "No  Device found for the deposit", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
			*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&err_flistp, ebufp);
			goto CLEANUP;
		}
		results_flistp = PIN_FLIST_ELEM_GET(search_outflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		device_pdp = PIN_FLIST_FLD_GET(results_flistp , PIN_FLD_POID , 1, ebufp);
		PIN_FLIST_FLD_SET(dep_flistp, PIN_FLD_DEVICE_OBJ, device_pdp, ebufp);

	}
	else
	{
		if ((PIN_ERRBUF_IS_ERR(ebufp)) || (PIN_FLIST_ELEM_COUNT(search_outflistp , PIN_FLD_RESULTS , ebufp) == 0))
		{
			PIN_ERRBUF_CLEAR(ebufp);
			err_flistp = PIN_FLIST_CREATE(ebufp);
			status = 1;
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, a_pdp, ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53044", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "No Device found for the deposit", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
			*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&err_flistp, ebufp);
			goto CLEANUP;
		}
	}
	

	if(d_flag == 0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Incorrect Input");
		PIN_ERRBUF_CLEAR(ebufp);
		err_flistp = PIN_FLIST_CREATE(ebufp);
		status = 1;
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, p_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53040", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, 
				"No Deposit products are valid for processing", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&err_flistp, ebufp);
		goto CLEANUP;
	}

	PIN_FLIST_FLD_SET(dep_flistp, MSO_FLD_DEPOSIT_AMOUNT, dep_total, ebufp);
	ret_flistp = NULL;
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"Deposit Object Creation input flist", dep_flistp);
	PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, dep_flistp, &ret_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		err_flistp = PIN_FLIST_CREATE(ebufp);
		status = 1;
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, binfo_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53042", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "No Deposit Items qualified for processing", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&err_flistp, ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"Deposit Object Creation output flist", ret_flistp);

	/************************************************************
	* Create the Deposit Lifecycle Event:
	************************************************************/
	PIN_FLIST_FLD_SET(ret_flistp, MSO_FLD_DEPOSIT_AMOUNT, dep_total, ebufp);
	PIN_FLIST_FLD_SET(ret_flistp, MSO_FLD_DEPOSIT_NO, dep_no, ebufp);
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_NAME, PYMT_DEPOSIT, ebufp);
	fm_mso_create_lifecycle_evt(ctxp, i_flistp, ret_flistp, ID_DEPOSIT, ebufp );

	//mso_op_deposit_gen_lc_event(ctxp, i_flistp, ret_flistp, dep_total, dep_no, PYMT_DEPOSIT, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		err_flistp = PIN_FLIST_CREATE(ebufp);
		status = 1;
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, binfo_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53042", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, 
			"Error while creating deposit lifecycle event", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&err_flistp, ebufp);
		goto CLEANUP;
	}

	//Preparing Input and Calling Notification API
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Calling Notification API");
	notif_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, notif_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, notif_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, notif_flistp, PIN_FLD_USERID, ebufp);
	PIN_FLIST_FLD_SET(notif_flistp, PIN_FLD_FLAGS, &notif, ebufp);
	PIN_FLIST_FLD_SET(notif_flistp, PIN_FLD_AMOUNT, dep_total, ebufp);
	PIN_FLIST_FLD_SET(notif_flistp, MSO_FLD_DEPOSIT_NO, dep_no, ebufp);
	PIN_FLIST_FLD_SET(notif_flistp, MSO_FLD_DEPOSIT_DATE, &now_t, ebufp);

	it_flistp = NULL;
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Notification input flist", notif_flistp);
	PCM_OP(ctxp, MSO_OP_NTF_CREATE_NOTIFICATION, 0, notif_flistp, &it_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Notification output flist", it_flistp);
	PIN_FLIST_DESTROY_EX(&it_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		err_flistp = PIN_FLIST_CREATE(ebufp);
		status = 1;
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, binfo_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53042", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Deposit Notification failure", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&err_flistp, ebufp);
		goto CLEANUP;
	}

	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status, ebufp);
	PIN_FLIST_FLD_SET(ret_flistp, MSO_FLD_DEPOSIT_NO, (char *)dep_no, ebufp);
	PIN_FLIST_FLD_SET(ret_flistp, MSO_FLD_DEPOSIT_AMOUNT, dep_actual_amt, ebufp);

	*o_flistpp = PIN_FLIST_COPY(ret_flistp,ebufp);
		
	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "mso_op_pymt_create_deposit error ");
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"mso_op_pymt_create_deposit error flist", *o_flistpp);
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"mso_op_pymt_create_deposit return flist", *o_flistpp);
	}

CLEANUP:
	PIN_FLIST_DESTROY_EX(&r_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&in_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&item_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&dep_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_inflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_outflistp, ebufp);
	if(!pbo_decimal_is_null(dep_total,ebufp))
		pbo_decimal_destroy(&dep_total);
	if(!pbo_decimal_is_null(dep_actual_amt,ebufp))
		pbo_decimal_destroy(&dep_actual_amt);
	return;
}

void
mso_pymt_create_catv_deposit(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{

	pin_flist_t		*ip_flistp = NULL;
	pin_flist_t		*in_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*pur_flistp = NULL;
	pin_flist_t		*rs_flistp = NULL;
	pin_flist_t		*bal_flistp = NULL;
	pin_flist_t		*dep_flistp = NULL;
	pin_flist_t		*item_flistp = NULL;
	pin_flist_t		*err_flistp = NULL;
	pin_flist_t		*p_flistp = NULL;
	pin_flist_t		*deal_flistp = NULL;
	pin_flist_t		*prod_flistp = NULL;

	time_t			now_t = 0;
	time_t			tstamp = 0;

	poid_t			*binfo_pdp = NULL;
	poid_t			*item_pdp = NULL;
	poid_t			*dep_pdp = NULL;
	poid_t			*p_pdp = NULL;

	pin_decimal_t		*dep_amt = NULL;
	pin_decimal_t		*qty = NULL;
	pin_decimal_t		*dep_total = NULL;

	char			*dep_no = NULL;

	u_int64			db = 0;
	u_int64			id = (u_int64)-1;

	int32			i_status = ITEM_STATUS_AVAIL;
	int32			d_status = ITEM_STATUS_OPEN;
	int32			status = 0;
	int32			p_status = 1;
	int32			rec_id = 0;
	int32			rec_id1 = 0;
	int32			rec_id2 = 0;
	int32			flag = 0;
	int32			*impact_type = NULL;

	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie1 = NULL;
	pin_cookie_t		cookie2 = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	r_flistp = PIN_FLIST_CREATE(ebufp);
	pur_flistp = PIN_FLIST_CREATE(ebufp);
	qty = pbo_decimal_from_str("1.0", ebufp);
	dep_total = pbo_decimal_from_str("0.0", ebufp);

	/*************************************************************
	 * Building the Purchase Deal Input Flist 
	 *************************************************************/
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, pur_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, pur_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_SET(pur_flistp, PIN_FLD_PROGRAM_NAME, "Hardware Deposit Product Purchase", ebufp);
	deal_flistp = PIN_FLIST_SUBSTR_ADD(pur_flistp, PIN_FLD_DEAL_INFO, ebufp);
//	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "1");

	while((p_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_PLAN, &rec_id, 1, &cookie, ebufp))!=(pin_flist_t*)NULL)
	{
		PIN_FLIST_FLD_COPY(p_flistp, PIN_FLD_DEAL_OBJ, deal_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(p_flistp, PIN_FLD_PLAN_OBJ, deal_flistp, PIN_FLD_PLAN_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(p_flistp, PIN_FLD_NAME, deal_flistp, PIN_FLD_DESCR, ebufp);
		PIN_FLIST_FLD_COPY(p_flistp, PIN_FLD_NAME, deal_flistp, PIN_FLD_NAME, ebufp);
//		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "2", pur_flistp);
		PIN_FLIST_FLD_SET(deal_flistp, PIN_FLD_START_T, &tstamp, ebufp);
		PIN_FLIST_FLD_SET(deal_flistp, PIN_FLD_END_T, &tstamp, ebufp);
//		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "3", pur_flistp);
		PIN_FLIST_FLD_SET(deal_flistp, PIN_FLD_FLAGS, &flag, ebufp);
		prod_flistp = PIN_FLIST_ELEM_ADD(deal_flistp, PIN_FLD_PRODUCTS, 0, ebufp);

		PIN_FLIST_FLD_COPY(p_flistp, PIN_FLD_PRODUCT_OBJ, prod_flistp, PIN_FLD_PRODUCT_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(p_flistp, PIN_FLD_NAME, prod_flistp, PIN_FLD_DESCR, ebufp);
//		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "4", pur_flistp);
		PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_PURCHASE_START_T, &tstamp, ebufp);
		PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_PURCHASE_END_T, &tstamp, ebufp);
		PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_QUANTITY, qty, ebufp);
		PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_STATUS, &p_status, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchase Deal Input Flist", pur_flistp);
		PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_PURCHASE_DEAL, 0, pur_flistp, &r_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchase Deal output flist", r_flistp);

		/************************************************
		*    Get total deposit amount start
		************************************************/
		rec_id1 =0;
		cookie1 = NULL;
		while((rs_flistp = PIN_FLIST_ELEM_GET_NEXT(r_flistp, PIN_FLD_RESULTS,
			&rec_id1, 1, &cookie1, ebufp))!=(pin_flist_t*)NULL)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Results Flist:",rs_flistp);
			rec_id2 = 0;
			cookie2 = NULL;
			while((bal_flistp = PIN_FLIST_ELEM_GET_NEXT(rs_flistp, PIN_FLD_BAL_IMPACTS,
				&rec_id2, 1, &cookie2, ebufp))!=(pin_flist_t*)NULL)
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"bal_flistp:",bal_flistp);
				impact_type = PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_IMPACT_TYPE, 0, ebufp);
				dep_amt = (pin_decimal_t *)PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_AMOUNT, 0, ebufp);
				if(impact_type && *impact_type != PIN_IMPACT_TYPE_TAX)
				{
					pbo_decimal_add_assign(dep_total, dep_amt, ebufp);
				}
			}
		}

	
		rs_flistp = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
		bal_flistp = PIN_FLIST_ELEM_GET(rs_flistp, PIN_FLD_BAL_IMPACTS, 0, 0, ebufp);
		if (!PIN_ERRBUF_IS_ERR(ebufp)) {
			
			dep_amt = (pin_decimal_t *)PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_AMOUNT, 0, ebufp);
			item_pdp = PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_ITEM_OBJ, 0, ebufp);
			
			if(!PIN_ERRBUF_IS_ERR(ebufp)) {

				/***********************************************************
				 * Update the item object status
				 ***********************************************************/
				item_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(item_flistp, PIN_FLD_POID, (void *)item_pdp, ebufp);
				PIN_FLIST_FLD_SET(item_flistp, PIN_FLD_STATUS, (void *)&i_status, ebufp);

				r_flistp = NULL;
				mso_item_status_update(ctxp, item_flistp, &r_flistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"Item Status update output flist", r_flistp);
			
				/***********************************************************
				 * Creating the Deposit Object
				 ***********************************************************/
				db = PIN_POID_GET_DB(item_pdp);
				now_t = pin_virtual_time((time_t *)NULL);
				dep_pdp = PIN_POID_CREATE(db, "/mso_deposit",id, ebufp);
						
				in_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, item_pdp, ebufp);
				PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_NAME, SEQ_TYPE_DEPOSIT, ebufp);
				
				r_flistp = NULL;
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Deposit Object Sequence Input Flist", in_flistp);
				fm_mso_utils_sequence_no(ctxp, in_flistp, &r_flistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Deposit Object Sequence Output Flist", r_flistp);
				dep_no = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_STRING, 0, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Cannot find /data/sequence object");
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
						"Data Sequence find Output Flist", r_flistp);
					PIN_ERRBUF_CLEAR(ebufp);
				}
				dep_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_PUT(dep_flistp, PIN_FLD_POID, (void *)dep_pdp, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, dep_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
				PIN_FLIST_FLD_SET(dep_flistp, MSO_FLD_DEPOSIT_AMOUNT, dep_amt, ebufp);
				PIN_FLIST_FLD_SET(dep_flistp, MSO_FLD_DEPOSIT_DATE, &now_t, ebufp);
				PIN_FLIST_FLD_SET(dep_flistp, MSO_FLD_DEPOSIT_NO, (char *)dep_no, ebufp);
				PIN_FLIST_FLD_SET(dep_flistp, PIN_FLD_ITEM_OBJ, (void *)item_pdp, ebufp);
				PIN_FLIST_FLD_COPY(p_flistp, PIN_FLD_CODE, dep_flistp, MSO_FLD_REFUND_RULE, ebufp);
				PIN_FLIST_FLD_COPY(p_flistp, PIN_FLD_NAME, dep_flistp, PIN_FLD_NAME, ebufp);
				PIN_FLIST_FLD_SET(dep_flistp, PIN_FLD_STATUS, (void *)&d_status, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, dep_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, dep_flistp, PIN_FLD_USERID, ebufp);

				r_flistp = NULL;
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"Deposit Object Creation input flist", dep_flistp);
				PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, dep_flistp, &r_flistp, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, MSO_FLD_DEPOSIT_NO, (char *)dep_no, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, MSO_FLD_DEPOSIT_AMOUNT, dep_amt, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ITEM_OBJ, (void *)item_pdp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"Deposit Object Creation output flist", r_flistp);
		
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "No Deposit Items qualified for processing");
				PIN_ERRBUF_CLEAR(ebufp);
				err_flistp = PIN_FLIST_CREATE(ebufp);
				status = 1;
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, binfo_pdp, ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53042", ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "No Deposit Items qualified for processing", ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
				*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&err_flistp, ebufp);
				goto CLEANUP;
			}
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Deposit Product Purchase Failed");
			PIN_ERRBUF_CLEAR(ebufp);
			err_flistp = PIN_FLIST_CREATE(ebufp);
			status = 1;
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, p_pdp, ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53040", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Deposit Product Purchase Failed", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
			*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&err_flistp, ebufp);
			goto CLEANUP;
		}
	}

	/************************************************************
	* Create the Deposit Lifecycle Event:
	************************************************************/
	PIN_FLIST_FLD_SET(r_flistp, MSO_FLD_DEPOSIT_AMOUNT, dep_total, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, MSO_FLD_DEPOSIT_NO, dep_no, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_NAME, PYMT_DEPOSIT, ebufp);
	fm_mso_create_lifecycle_evt(ctxp, i_flistp, r_flistp, ID_DEPOSIT, ebufp );
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		err_flistp = PIN_FLIST_CREATE(ebufp);
		status = 1;
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, p_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53042", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, 
			"Error while creating deposit lifecycle event", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&err_flistp, ebufp);
		goto CLEANUP;
	}

	*o_flistpp = PIN_FLIST_COPY(r_flistp,ebufp);
		
	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "mso_op_pymt_create_deposit error ");
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"mso_op_pymt_create_deposit error flist", *o_flistpp);
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"mso_op_pymt_create_deposit return flist", *o_flistpp);
	}

CLEANUP:
	PIN_FLIST_DESTROY_EX(&r_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&in_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&item_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&dep_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&pur_flistp, ebufp);
	if (!pbo_decimal_is_null(qty, ebufp))
	{
		pbo_decimal_destroy(&qty);
	}
	if (!pbo_decimal_is_null(dep_total, ebufp))
	{
		pbo_decimal_destroy(&dep_total);
	}
	/*if (!pbo_decimal_is_null(dep_amt, ebufp))
	{
		pbo_decimal_destroy(&dep_amt);
	}*/
	return;
}

/* This module retrieves the item poid details based from the Input Flist passed to MSO_OP_PYMT_CREATE_DEPOSIT */
/*void mso_find_item_poid(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
        u_int64         id = (u_int64)-1;
        pin_flist_t     *s_flistp = NULL;
		pin_flist_t		*flistp = NULL;
        int32           s_flags = 256;
		poid_t          *pdp = NULL;
		int64           db= 0;
		poid_t		*binfo_pdp = NULL;
		poid_t		*s_item_pdp = NULL;
		int32		p_items = ITEM_STATUS_PENDING;

		binfo_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
		db = PIN_POID_GET_DB(binfo_pdp);

        s_flistp = PIN_FLIST_CREATE(ebufp);
        pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
		s_item_pdp = PIN_POID_CREATE(db, "/item/payment/mso_deposit", id, ebufp);
        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /item where F1.type = V1 and F2 = V2 and F3 = V3", ebufp);
        flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)s_item_pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_STATUS, (void *)&p_items, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_BILLINFO_OBJ, (void *)binfo_pdp, ebufp);
	    flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, (void *)NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "item poid search input flist", s_flistp);
	//Retrieves the Billinfo POID and accounting type
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, r_flistpp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "item poid search output flist", *r_flistpp);
        if (PIN_ERRBUF_IS_ERR(ebufp)) {
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "No Pending Items found for processing", ebufp);
//                return;
        }

        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
}
*/

/* This module retrieves the deposit amount from the product purchased for deposit */
/*void mso_retrieve_deposit_amount(
        pcm_context_t     *ctxp,
        poid_t			  *pdt_pdp,
        pin_flist_t       **ret_flistpp,
        pin_errbuf_t      *ebufp)
{
        u_int64         id = (u_int64)0;
        u_int64         db = 0;
        pin_flist_t     *s_flistp = NULL;
        int32           s_flags = 256;
		int32			status = 0;
        pin_flist_t     *flistp = NULL;
		pin_flist_t		*q_flistp = NULL;
		pin_flist_t		*r_flistp = NULL;
		pin_flist_t		*b_flistp = NULL;
		pin_flist_t		*s_r_flistp = NULL;
        poid_t          *pdp = NULL;
		poid_t			*rp_pdp = NULL;
		poid_t			*p_pdp = NULL;
		pin_decimal_t	*d_amt = NULL;
		pin_decimal_t	*f_amt = NULL;
		pin_decimal_t	*s_amt = NULL;
		pin_decimal_t	*d_pr = NULL;

		d_amt = pbo_decimal_from_str("0.0",ebufp);
		f_amt = pbo_decimal_from_str("0.0",ebufp);
		s_amt = pbo_decimal_from_str("0.0",ebufp);
		d_pr = pbo_decimal_from_str(DEP_PRIORITY,ebufp);

        db=(u_int64)PIN_POID_GET_DB(pdt_pdp);
        s_flistp = PIN_FLIST_CREATE(ebufp);
		rp_pdp = PIN_POID_CREATE(db, "/rate_plan", id, ebufp);
		p_pdp = PIN_POID_CREATE(db, "/product", id, ebufp);
        pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /rate 1, /rate_plan 2, /product 3 where (1.F1 = 2.F2 and 2.F3 = 3.F4 and 3.F5 = V5 and 3.F6 = V6) ", ebufp);
        flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_RATE_PLAN_OBJ, (void *)rp_pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, (void *)rp_pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_PRODUCT_OBJ, (void *)p_pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, (void *)p_pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, (void *)pdt_pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 6, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_PRIORITY, d_pr, ebufp);
        flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "Deposit Amount search input flist", s_flistp);
*/
        /* Get the Deposit Amount from /rate and /rate_plan objects 
		 * Assuming there is only one quantity tier and only one
		 * balance impact configuration for deposits in the fixed amount, 
		 * for better performance.
		 */
/*        PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &s_r_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "Deposit Amount search output flist", s_r_flistp);

		r_flistp = PIN_FLIST_FLD_GET(s_r_flistp, PIN_FLD_RESULTS, 0, ebufp);
		q_flistp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_QUANTITY_TIERS, 0, ebufp);
		b_flistp = PIN_FLIST_FLD_GET(q_flistp, PIN_FLD_BAL_IMPACTS, 0, ebufp);
		f_amt = (pin_decimal_t *)PIN_FLIST_FLD_GET(b_flistp, PIN_FLD_FIXED_AMOUNT, 0, ebufp);
		s_amt = (pin_decimal_t *)PIN_FLIST_FLD_GET(b_flistp, PIN_FLD_SCALED_AMOUNT, 0, ebufp);
		pbo_decimal_add_assign(d_amt, s_amt, ebufp);
		pbo_decimal_add_assign(d_amt, f_amt, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERRBUF_CLEAR(ebufp);
			status = 1;
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_POID, p_pdp, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Deposit Amount Cannot be Retrieved");
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "53041", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "Deposit Amount Cannot be Retrieved", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, &status, ebufp);
            goto CleanUp;
        }
		else
		{
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_POID, (void *)pdt_pdp, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_AMOUNT, (pin_decimal_t *)d_amt, ebufp);
		}
	CleanUp:
		PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
//		PIN_FLIST_DESTROY_EX(&s_r_flistp, NULL);
//		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
//		PIN_FLIST_DESTROY_EX(&q_flistp, NULL);
//		PIN_FLIST_DESTROY_EX(&b_flistp, NULL);
		PIN_POID_DESTROY(rp_pdp, ebufp);
		PIN_POID_DESTROY(p_pdp, ebufp)
}
*/

/* This module updates the /item/payment/mso_deposit status to 0, so that it is 
 * untouched by billing process and can be retrieved for all invoices for display 
 * and later on for the deposit refund
 *
 * Input : Item Poid & Item Status(to be updated)
 * 
 */
EXPORT_OP void mso_item_status_update(
	pcm_context_t		*ctxp,
	pin_flist_t			*item_flistp,
	pin_flist_t			**r_flistp,
	pin_errbuf_t		*ebufp)
{
	int32		status = 0;
	poid_t		*item_pdp = NULL;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"Item Status Update input flist", item_flistp);
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, item_flistp, r_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"Item Status Update output flist", *r_flistp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Item Status Updation error", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		status = 1;
		PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_POID, item_pdp, ebufp);
		PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_CODE, "53043", ebufp);
		PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_DESCR, "Deposit Item Status Updation error", ebufp);
		PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_STATUS, &status, ebufp);
		return;
	}
}

//static void
//mso_op_deposit_gen_lc_event(
//	pcm_context_t           *ctxp,
//	pin_flist_t             *i_flistp,
//	pin_flist_t             *dep_flistp,
//	pin_decimal_t		*dep_total,
//	char			*dep_no,
//	char                    *disp_name,
//	pin_errbuf_t            *ebufp)
//{
//	pin_flist_t             *lc_iflistp = NULL;
//	pin_flist_t             *lc_temp_flistp = NULL;
//	pin_flist_t             *ro_flistp = NULL;
//	poid_t                  *user_id = NULL;
//	poid_t                  *acct_pdp = NULL;
//	poid_t                  *serv_pdp = NULL;
//	char                    *event = "/event/activity/mso_lifecycle/deposit";
//
//	if (PIN_ERRBUF_IS_ERR(ebufp))
//			return;
//
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_deposit_gen_lc_event Input FList:", i_flistp);
//
//	acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp );
//	serv_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp );
//	user_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 0, ebufp );
//	lc_iflistp = PIN_FLIST_CREATE(ebufp);
//	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_DEPOSIT, 0, ebufp);
//	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_NAME, disp_name, ebufp);
//	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_USERID, user_id, ebufp);
//	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_DEPOSIT_AMOUNT, dep_total, ebufp);
//	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_DEPOSIT_NO, dep_no, ebufp);
//	PIN_FLIST_FLD_COPY(dep_flistp, PIN_FLD_POID, lc_temp_flistp, PIN_FLD_EVENT_OBJ, ebufp);
//
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_gen_event Event FList:", lc_iflistp);
//
//	fm_mso_utils_gen_event(ctxp, acct_pdp, serv_pdp, "MSO_OP_PYMT_CREATE_DEPOSIT",
//							disp_name, event, lc_iflistp, ebufp);
//
//	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
//	PIN_FLIST_DESTROY_EX(&ro_flistp, NULL);
//	return;
//}

