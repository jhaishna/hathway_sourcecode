/*
 *
 * Copyright (c) 2003, 2009, Oracle and/or its affiliates. 
 * All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 * 
 * Version 1.0: 01-10-2013: Vilva Sabarikanth: Added the MSO Customization of
 * preparing the Payment Input FLIST for MSO_OP_PYMT_LCO_COLLECT_UPDATE opcode and payment 
 * posting and receipt creation for bulk payments from LCO through OAP
 *
 */

#ifndef lint
static char Sccs_id[] = "@(#)$Id: fm_mso_pymt_lco_collect.c  /cgbubrm_7.5.0. 2013/09/26 04:39:34 vsabarik Exp $";
#endif

/*******************************************************************
 * Contains the MSO_OP_PYMT_LCO_COLLECT_UPDATE operation. 
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

#define LOCAL_TRANS_OPEN_SUCCESS 0
#define LOCAL_TRANS_OPEN_FAIL 1

#define READONLY 0
#define READWRITE 1
#define LOCK_OBJ 2

#define CATV_PROFILE "CATV"
#define BB_PROFILE "BB"

#define PAY_TYPE_CASH 10011
#define PAY_TYPE_CHECK 10012

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
void
mso_pymt_lco_collect_update(
        cm_nap_connection_t	*connp,
	int			opcode,
        int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp);

PIN_IMPORT int32 fm_mso_trans_open(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	int32		flag,
	pin_errbuf_t	*ebufp);

PIN_IMPORT void fm_mso_trans_commit(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	pin_errbuf_t	*ebufp);

PIN_IMPORT void fm_mso_trans_abort(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	pin_errbuf_t	*ebufp);

void mso_retrieve_account_poid(
        pcm_context_t     	*ctxp,
        char	            *acct_no,
		pin_flist_t			**o_flistpp,
        pin_errbuf_t        *ebufp);

int mso_confirm_service_type(
        pcm_context_t     	*ctxp,
        int32	            *srvc_type,
		poid_t				*acct_pdp,
        pin_errbuf_t            *ebufp);

/*******************************************************************
 * Main routine for the MSO_OP_PYMT_LCO_COLLECT_UPDATE operation.
 *******************************************************************/
void
mso_pymt_lco_collect_update(
        cm_nap_connection_t	*connp,
		int				opcode,
        int				flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t	*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	int					*cmdp = NULL;
	pin_flist_t			*r_flistp = NULL;
	int32				p_status=0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != MSO_OP_PYMT_LCO_COLLECT_UPDATE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_pymt_lco_collect_update opcode error", ebufp);
		return;
	}
	
	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"mso_pymt_lco_collect_update input flist", i_flistp);
	//Reading the Input Params

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"LCO Payment Object Update input flist", i_flistp);
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, i_flistp, &r_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"LCO Payment Object Update output flist", r_flistp);


	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp)) {

		/***************************************************
		 * Log something and return nothing.
		 ***************************************************/
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"mso_pymt_lco_collect_update error", ebufp);
		*o_flistpp = NULL;
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *o_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_ERROR_CODE, "53037", ebufp);
		PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_ERROR_DESCR, "LCO Payment Update Failed", ebufp);
		p_status=1;
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
			"mso_pymt_lco_collect_update return flist", *o_flistpp);
		PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_DESCR, "LCO Payment Updated Successfully", ebufp);
	}
	PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_STATUS, &p_status, ebufp);
	return;
}

