/*******************************************************************
 *
 *      Copyright (c) 1999-2006 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char Sccs_Id[] = "@(#)%Portal Version: fm_cust_pol_valid_taxinfo.c:BillingVelocityInt:2:2006-Sep-05 04:32:51 %";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pcm.h"
#include "ops/rate.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pin_rate.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "fm_bill_utils.h"

/************************************************************************
 * Forward declarations.
 ************************************************************************/
EXPORT_OP void
op_cust_pol_valid_taxinfo(
        cm_nap_connection_t	*connp,
	int32			opcode,
        int32			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp);

static void 
fm_cust_pol_valid_taxinfo(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistpp,
        pin_errbuf_t		*ebufp);

static void
do_vat_cert_validation(
	pcm_context_t	*ctxp,
	pin_flist_t	*in_flistp,
	pin_flist_t	**s_flistpp,
	char		*vat_certp,
	pin_errbuf_t    *ebufp);

/***********************************************************************
 * Routines needed from elsewhere
 ***********************************************************************/
extern pin_flist_t *
fm_cust_pol_valid_add_fail(
        pin_flist_t     *r_flistp,
        int32           field,
        int32           elemid,
        int32           result,
        char            *descr,
        void            *val,
        pin_errbuf_t    *ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_VALID_TAXINFO  command
 *******************************************************************/
void
op_cust_pol_valid_taxinfo(
        cm_nap_connection_t	*connp,
	int32			opcode,
        int32			flags,
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
	if (opcode != PCM_OP_CUST_POL_VALID_TAXINFO) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_cust_pol_valid_taxinfo", ebufp);
		return;
		/*****/
	}

	/***********************************************************
	 * Debug - What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_cust_pol_valid_taxinfo input flist", in_flistp);

	/***********************************************************
	 * We will not open any transactions with Policy FM.  We 
	 * don't need to since policies should NEVER modify the
	 * database
	 ***********************************************************/

	/***********************************************************
	 * Call main function to do it
	 ***********************************************************/
	fm_cust_pol_valid_taxinfo(ctxp, in_flistp, &r_flistp, ebufp);

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		*ret_flistpp = (pin_flist_t *)NULL;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_valid_taxinfo error", ebufp);
	} else {
		*ret_flistpp = r_flistp;
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_valid_taxinfo return flist", r_flistp);
	}

	return;
}


/*******************************************************************
 * fm_cust_pol_valid_taxinfo()
 *
 *     Function to validate tax related fields like:
 *		PIN_FLD_VAT_CERT
 *		PIN_FLD_EXEMPTIONS (not implemented yet)
 *	from the flist that is ready for on-line registration.
 *
 *******************************************************************/
static void 
fm_cust_pol_valid_taxinfo(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistpp,
        pin_errbuf_t		*ebufp)
{
	pin_flist_t		*r_flistp = NULL;
	int32			result = 0;
	void                    *vp = NULL;
	int32			err = 0;
	int			tax_valid_flag=0;
	char			*vat_certp = NULL;

	/***********************************************************
	 * Set result.
	 ***********************************************************/
	result = PIN_CUST_VERIFY_FAILED;

	/***********************************************************
	 * Null out results until we have some.
	 ***********************************************************/
	*out_flistpp = NULL;

	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Create outgoing flist
	 ***********************************************************/
	r_flistp = PIN_FLIST_CREATE(ebufp);

	/***********************************************************
	 * Copy the incoming PIN_FLD_POID to the outgoing list - 
	 * 	mandatory
	 ***********************************************************/
	vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, vp, ebufp);

	vat_certp = (char *)PIN_FLIST_FLD_GET(in_flistp, 
				PIN_FLD_VAT_CERT, 1, ebufp);
	if( !vat_certp  || strlen(vat_certp) == 0) {
		result = PIN_CUST_VERIFY_PASSED;
		goto Done;
		/********/
	}

	/***********************************************************
	 * Validate vat certificate if switch: tax_valid in pin.conf
	 * is ON
	 **********************************************************/
	pin_conf(FM_CUST_POL, "tax_valid", PIN_FLDT_INT,
			(caddr_t *)&vp, &err);

	switch (err) {
	case PIN_ERR_NONE:
		tax_valid_flag = *((int *)vp);
		free(vp);
		break;

	case PIN_ERR_NOT_FOUND:
		break;

	default:
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE, err, 0, 0, 0);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"Unable to read tax_valid from pin.conf",ebufp);
		break;
	}

	if( !tax_valid_flag ||
		(pin_conf_taxation_switch == PIN_RATE_TAX_SWITCH_NONE) ) {

		goto Done;
		/********/
	}

	/***********************************************************
	 * Pass taxPkg to use in the validation
	 ***********************************************************/
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_TAXPKG_TYPE,
		(void*)&tax_valid_flag, ebufp);

	do_vat_cert_validation(ctxp, in_flistp, &r_flistp, vat_certp, ebufp);

	if (PIN_FLIST_ELEM_COUNT(r_flistp, PIN_FLD_FIELD, ebufp) == 0)
        {
                result = PIN_CUST_VERIFY_PASSED;
        } else {
                result = PIN_CUST_VERIFY_FAILED;
        }
Done:
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_RESULT, (void *)&result, ebufp);

	*out_flistpp = r_flistp;

	return;
}

/*************************************************************************
 * do_vat_cert_validation()
 *
 *************************************************************************/
static void
do_vat_cert_validation(
	pcm_context_t	*ctxp,
	pin_flist_t	*in_flistp,
	pin_flist_t	**r_flistpp,
	char		*vat_certp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*ti_flistp = NULL;
	pin_flist_t	*tr_flistp = NULL;
	void		*vp = NULL;
	int32		command = 0;
	int32		*resultp = NULL;
	int32		r=0;

	/*******************************************************
	 * create flist 
	 ********************************************************/
	ti_flistp = PIN_FLIST_CREATE(ebufp);

	vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(ti_flistp, PIN_FLD_POID, vp, ebufp);

	command = PIN_CUST_TAX_VAL_VAT_CERT;
	PIN_FLIST_FLD_SET(ti_flistp, PIN_FLD_COMMAND, 
					(void *)&command, ebufp);

	PIN_FLIST_FLD_SET(ti_flistp, PIN_FLD_VAT_CERT, 
					(void *)vat_certp, ebufp);

	vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_TAXPKG_TYPE, 0, ebufp);
	PIN_FLIST_FLD_SET(ti_flistp, PIN_FLD_TAXPKG_TYPE, vp, ebufp);

	PCM_OP(ctxp, PCM_OP_RATE_TAX_CALC, 0, ti_flistp, &tr_flistp, ebufp);

	if( tr_flistp == (pin_flist_t *)NULL) {
		goto vat_valid_fail;
		/*******************/
	}

	resultp = (int32 *)PIN_FLIST_FLD_GET( tr_flistp, 
			PIN_FLD_RESULT, 1, ebufp);

	if( resultp == (int32 *)NULL) {
		PIN_FLIST_DESTROY_EX(&tr_flistp, NULL);
		goto vat_valid_fail;
		/*******************/
	}

	r = *resultp;
	PIN_FLIST_DESTROY_EX(&tr_flistp, NULL);

	if( !r ) {
		goto vat_valid_fail;
		/*******************/
	} 

	PIN_FLIST_DESTROY_EX(&ti_flistp, NULL);
	return;
	/*****/
	
vat_valid_fail:

	PIN_FLIST_DESTROY_EX(&ti_flistp, NULL);

	(void)fm_cust_pol_valid_add_fail(*r_flistpp,
		PIN_FLD_VAT_CERT, 0,
		PIN_CUST_VAL_ERR_INVALID,
		PIN_CUST_TAX_VAL_VAT_CERT_ERR_MSG, vat_certp,
		ebufp);
	return;
}

