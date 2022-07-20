/*******************************************************************
 *
* Copyright (c) 1999, 2016, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char    Sccs_id[] = "@(#)$Id: fm_inv_pol_format_invoice_doc1.c /cgbubrm_7.5.0.portalbase/2 2016/07/07 10:53:20 bmaturi Exp $";
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pcm.h"
#include "ops/inv.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "fm_bill_utils.h"

#include "pin_inv.h"

#include "fm_inv_pol_common.h"

#define	SEPARATOR_DOC1	"\t"
#define TAGSIZ		10

/*******************************************************************
 * Routines contained within.
 *******************************************************************/

EXPORT_OP void
op_inv_pol_format_invoice_doc1();

static void
fm_inv_pol_format_invoice_doc1(
	pcm_context_t           *ctxp,
	int32                     flags,
	pin_flist_t             *in_flistp,
	pin_flist_t             **ret_flistpp,
	pin_errbuf_t            *ebufp);

static void
fm_inv_pol_format_invoice_doc1_add_single(
	pcm_context_t		*ctxp,
	poid_t			*cur_bill_obj, 
	fm_inv_pol_str_buf_t	*dbufp, 
	pin_errbuf_t		*ebufp);

static void
fm_inv_pol_format_invoice_doc1_single(
	pin_flist_t		*in_flistp,
	char			*prefix,
	fm_inv_pol_str_buf_t	*dbufp,
	pin_errbuf_t		*ebufp);

static void
fm_inv_pol_format_invoice_doc1_repeats_service(
	pcm_context_t           *ctxp,
	pin_flist_t		*in_flistp,
	int32			fld_no,
	poid_t			*last_bill_obj,
	fm_inv_pol_str_buf_t	*dbufp,
	pin_errbuf_t		*ebufp);

static void
fm_inv_pol_format_invoice_doc1_repeats(
	pcm_context_t           *ctxp,
	pin_flist_t		*in_flistp,
	int32			fld_no,
	pin_flist_t		*service_flistp,
	poid_t			*last_bill_obj,
	fm_inv_pol_str_buf_t	*dbufp,
	pin_errbuf_t		*ebufp);

static void
fm_inv_pol_format_invoice_doc1_repeats_item(
	pin_flist_t		*iitem_flistp,
	pin_flist_t		*in_flistp,
	fm_inv_pol_str_buf_t	*dbufp,
	char			*r_doc1tag,	
	pin_errbuf_t		*ebufp);

static void
fm_inv_pol_format_invoice_doc1_repeats_event(
	pin_flist_t		*ievent_flistp,
	pin_flist_t		*in_flistp,
	fm_inv_pol_str_buf_t	*dbufp,
	char			*doc1tag,	
	pin_errbuf_t		*ebufp);

static void
fm_inv_pol_format_invoice_doc1_addfield(
	char			*doc1tag,
	char			*fieldname, 
	int32			type,
	fm_inv_pol_str_buf_t	*dbufp,
	void			*vp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_INV_POL_FORMAT_INVOICE_DOC1 operation.
 *
 * NOTE:
 * This opcode will become OBSOLETE in future Portal versions.
 *******************************************************************/
void
op_inv_pol_format_invoice_doc1(connp, opcode, flags, in_flistp, 
			       ret_flistpp, ebufp)
        cm_nap_connection_t	*connp;
	int32			opcode;
        int32			flags;
        pin_flist_t		*in_flistp;
        pin_flist_t		**ret_flistpp;
        pin_errbuf_t		*ebufp;
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_INV_POL_FORMAT_INVOICE_DOC1) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_inv_pol_format_invoice_doc1 opcode error", 
			ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_inv_pol_format_invoice_doc1 input flist", in_flistp);

	/***********************************************************
	 * Do the actual op in a sub. Copy it since we may need to
	 * replace it later.
	 ***********************************************************/

	fm_inv_pol_format_invoice_doc1(ctxp, flags, in_flistp, 
				       ret_flistpp, ebufp);

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_inv_pol_format_invoice_doc1 error", ebufp);
	} else {
		/***************************************************
		 * Debug: What we're sending back.
		 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_inv_pol_format_invoice_doc1 return flist", 
				  *ret_flistpp);
	}
	return;
}

/*******************************************************************
 * fm_inv_pol_format_invoice_doc1():
 *
 * This function converts the input flist into the default Portal
 * invoicing output for the DOC1 format.
 *
 *
 * NOTE:
 * This function could be called from one of the following policy
 * opcode:
 *	       PCM_OP_INV_POL_FORMAT_INVOICE
 *	       PCM_OP_INV_POL_VIEW_INVOICE
 *
 * Description:
 * This function takes the input flist and converts its contents into 
 * Name:Value pairs that conform to the default Portal required DOC1
 * input (item groups). 
 *
 * The ordering of the groups and within the groups is of importants.
 *
 * The the groups are converted into NAME:VALUE pairs of the
 * following format:
 *
 *      <DOC1 TAG><TAB><PIN FIELD NAME><"><VALUE><">
 *
 *      where the DOC1 TAG is fixed width (10 bytes)
 *
 * e.g. PARPAY   PIN_FLD_NAME"Steve"
 *      PARPAY   PIN_FLD_ADDRESS"Steven's Creek"
 *      ...
 *      PARTOT   PIN_FLD_BILL_NO"B-2"
 *      ...
 *
 * The NAME:VALUE pairs are stored in a character buffer that is
 * return to the calling function.
 *
 * We pass in:
 *      i_flistp	Input flist with all the invoice data. 
 *	bufpp		A buffer that will contain the invoice data
 *			in the correct format for the DOC1 software, 
 *			as required for the default Portal DOC1 
 *	lenp		The length of the buffer
 *
 *******************************************************************/
static void
fm_inv_pol_format_invoice_doc1(
	pcm_context_t		*ctxp,	
	int32			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp)
{
  	fm_inv_pol_str_buf_t	dbuf;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Reset ret_flistpp.
	 ***********************************************************/
	*ret_flistpp = NULL;

	/***********************************************************
	 * Debug what we have got.
	 ***********************************************************/

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	 	 "fm_inv_pol_format_invoice_doc1: input flist",
	 	 in_flistp);

	/*******************************************************
	 * Initialize dbuf - will hold the doc1 formatted buffer
	 *******************************************************/

	dbuf.strp = NULL;
	dbuf.size = 0;
	dbuf.strsize = 0;
	dbuf.chunksize = 4096;

	/***********************************************************
	 * Call function to add a specif string/token to the final
	 * output.
	 ***********************************************************/

	fm_inv_pol_add_str_to_buf(&dbuf, (char *)"BEGINDOC  \n", ebufp);

	/***********************************************************
	 * Call functions to convert the PARPAY, PARNAM, PARTOT
	 * item groups.
	 ***********************************************************/

	fm_inv_pol_format_invoice_doc1_single(in_flistp, "PAR", &dbuf, 
		ebufp);


	/***********************************************************
	 * Call functions to conver the items and events into DOC1
	 * item groups.
	 * xxx_repeats_service function is called first, in case the
	 * service centric invoicing is enabled.
	 ***********************************************************/

	fm_inv_pol_format_invoice_doc1_repeats_service(ctxp, in_flistp, 
		PIN_FLD_AR_ITEMS, NULL, &dbuf, ebufp);
	fm_inv_pol_format_invoice_doc1_repeats_service(ctxp, in_flistp, 
		PIN_FLD_SUB_ITEMS, NULL, &dbuf, ebufp);
	fm_inv_pol_format_invoice_doc1_repeats_service(ctxp, in_flistp, 
		PIN_FLD_OTHER_ITEMS, NULL, &dbuf, ebufp);


	/***********************************************************
	 * Call function to add a specif string/token to the final
	 * output.
	 ***********************************************************/

	fm_inv_pol_add_str_to_buf( &dbuf, (char *)"ENDDOC    \n", ebufp);

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
	 	 	 "fm_inv_pol_format_invoice_doc1: error" \
	 	 	 " converting in_flistp into DOC1 format", ebufp);
		goto ErrOut;
	}

	/*******************************************************
	 * The buffer that was just filled, may have unused memory
	 * in it.  Let's free up the unused memory with realloc
	 * before exiting. this trims off the unused stuff at end
 	 *******************************************************/
	dbuf.strp = (char *)pin_realloc( dbuf.strp, dbuf.strsize + 1 );

	/***********************************************************
	 * Now create the return flist and put the buffer onto it.
	 ***********************************************************/
	fm_inv_pol_format_invoice_finalize(&dbuf, 
		FM_INV_FORMAT_TYPE_DOC1 ,ret_flistpp, ebufp);

ErrOut:
	if( dbuf.strp ) pin_free( dbuf.strp );
	return;
}

/*******************************************************************
 * fm_inv_pol_format_invoice_doc1_add_single():
 *
 * This function is to prepare the required fields/array to call 
 * fm_inv_pol_format_invoice_doc1_single() so as to add SUBPAY, SUBNAM
 * and SUBTOT groups on the output.
 *
 * We pass in:
 *  ctxp, cur_bill_obj	used to retrieve information from /bill and 
 *		/account to generate in_flistp to call 
 *		fm_inv_pol_format_invoice_doc1_single().
 *  dbufp	formatted buffer
 *******************************************************************/
static void
fm_inv_pol_format_invoice_doc1_add_single(
	pcm_context_t		*ctxp,
	poid_t			*cur_bill_obj, 
	fm_inv_pol_str_buf_t	*dbufp, 
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*in_flistp = NULL;
	pin_flist_t		*i_flistp = NULL;
	pin_flist_t		*acct_flistp = NULL;
	pin_flist_t		*bill_flistp = NULL;
	pin_flist_t		*billinfo_flistp = NULL;
	pin_flist_t		*payinfo_flistp = NULL;
	pin_flist_t		*nameinfo_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*bal_flistp = NULL;
	pin_flist_t		*tmp_fm_utils_init_cfg_beid = NULL;
	void			*vp = NULL;

	int32			elemid = 0;
	pin_cookie_t		cookie = NULL;
	int32			currency = 0;

	if (PIN_ERR_IS_ERR(ebufp))
	 	 return;
	PIN_ERR_CLEAR_ERR(ebufp);
	
	in_flistp = PIN_FLIST_CREATE(ebufp);

	/***********************************************************
	 * Specify the result set for /bill.
	 ***********************************************************/
	i_flistp = PIN_FLIST_CREATE(ebufp);

	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_POID, (void *)cur_bill_obj, 
		ebufp);
	vp = (void *)NULL;
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_ACCOUNT_OBJ, vp, ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_BILL_NO, vp, ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_START_T, vp, ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_END_T, vp, ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CURRENT_TOTAL, vp, ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_PREVIOUS_TOTAL, vp, ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_SUBORDS_TOTAL, vp, ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_TOTAL_DUE, vp, ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CURRENCY, vp, ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CURRENCY_SECONDARY, vp,
		ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_BILLINFO_OBJ, vp, ebufp);

	/***********************************************************
	 * Read the fields from the /bill object.
	 ***********************************************************/

	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, i_flistp, &bill_flistp, ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_inv_pol_format_invoice_doc1_add_single: "
			"bad read flds for poid bill", ebufp);
		goto cleanup;
		/***********/
	}

	/***********************************************************
	 * Specify the result set for /billinfo.
	 ***********************************************************/
	PIN_FLIST_DESTROY_EX(&i_flistp, NULL);

	i_flistp = PIN_FLIST_CREATE(ebufp);
	vp = PIN_FLIST_FLD_GET(bill_flistp, PIN_FLD_BILLINFO_OBJ, 0, ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_POID, vp, ebufp);

	vp = NULL;
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_PAYINFO_OBJ, vp, ebufp);

	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, i_flistp, &billinfo_flistp, ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_inv_pol_format_invoice_doc1_add_single: "
			"bad read flds for poid billinfo", ebufp);
		goto cleanup;
		/***********/
	}

	/***********************************************************
	 * Specify the result set for /account.
	 ***********************************************************/
	PIN_FLIST_DESTROY_EX(&i_flistp, NULL);

	i_flistp = PIN_FLIST_CREATE(ebufp);
	vp = PIN_FLIST_FLD_GET(bill_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_POID, vp, ebufp);


	vp = NULL;
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_ACCOUNT_NO, vp, ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_PAYINFO_OBJ, vp, ebufp);

	/***********************************************************
	 * Read all elements of the PIN_FLD_NAMEINFO array.
	 * NOTE: We only want certain fields from PIN_FLD_NAMEINFO.
	 ***********************************************************/
	tmp_flistp = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_NAMEINFO,
		PIN_ELEMID_ANY, ebufp);

	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_SALUTATION, vp, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_LAST_NAME, vp, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_FIRST_NAME, vp, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_MIDDLE_NAME, vp, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_TITLE, vp, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_COMPANY, vp, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ADDRESS, vp, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_CITY, vp, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STATE, vp, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ZIP, vp, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_COUNTRY, vp, ebufp);

	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, i_flistp, &acct_flistp, ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_inv_pol_format_invoice_doc1_add_single: "
			"bad read flds for poid account", ebufp);
		goto cleanup;
		/***********/
	}

	/***********************************************************
	 * Specify the result set for /payinfo.
	 ***********************************************************/
	PIN_FLIST_DESTROY_EX(&i_flistp, NULL);

	i_flistp = PIN_FLIST_CREATE(ebufp);
	vp = PIN_FLIST_FLD_GET(billinfo_flistp, PIN_FLD_PAYINFO_OBJ, 0, ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_POID, vp, ebufp);

	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, i_flistp, &payinfo_flistp, ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_inv_pol_format_invoice_doc1_add_single: "
			"bad read objects for poid payinfo", ebufp);
		goto cleanup;
		/***********/
	}

	/***********************************************************
	 * Move ACCOUNT, BILL and PAYINFO arrays to in_flistp
	 ***********************************************************/
	PIN_FLIST_ELEM_PUT(in_flistp, bill_flistp, PIN_FLD_BILLINFO, 0, ebufp);
	bill_flistp = NULL;

	nameinfo_flistp = PIN_FLIST_ELEM_TAKE(acct_flistp,
		PIN_FLD_NAMEINFO, PIN_ELEMID_ANY, 0, ebufp);

	vp = PIN_FLIST_FLD_TAKE(payinfo_flistp, PIN_FLD_NAME, 0, ebufp);
	PIN_FLIST_FLD_PUT(nameinfo_flistp, PIN_FLD_NAME, vp, ebufp);

	PIN_FLIST_ELEM_PUT(in_flistp, nameinfo_flistp, 
		PIN_FLD_NAMEINFO, 0, ebufp);

	PIN_FLIST_ELEM_PUT(in_flistp, acct_flistp, PIN_FLD_ACCTINFO, 0, 
		ebufp);
	acct_flistp = NULL;

	/***********************************************************
	 * Prepare the CURRENCIES array
	 ***********************************************************/
	tmp_flistp = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_BILLINFO,
		PIN_ELEMID_ANY, 0, ebufp);

	vp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CURRENCY, 0, ebufp);

	if (vp) currency = *(int32 *)vp;
	else currency = 0;
	
	tmp_fm_utils_init_cfg_beid = fm_bill_utils_cfg_get_beid(ctxp, ebufp);
	if (currency) {
		bal_flistp = PIN_FLIST_ELEM_GET(tmp_fm_utils_init_cfg_beid,
			PIN_FLD_BALANCES, currency, 0, ebufp);

		PIN_FLIST_ELEM_SET(in_flistp, bal_flistp,
			PIN_FLD_CURRENCIES, currency, ebufp);
	}

	vp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CURRENCY_SECONDARY, 
		0, ebufp);

	if (vp) currency = *(int32 *)vp;
	else currency = 0;
	
	if (currency) {
		bal_flistp = PIN_FLIST_ELEM_GET(tmp_fm_utils_init_cfg_beid,
			PIN_FLD_BALANCES, currency, 0, ebufp);

		PIN_FLIST_ELEM_SET(in_flistp, bal_flistp,
			PIN_FLD_CURRENCIES, currency, ebufp);
	}

	/***********************************************************
	 *  Now ready to call fm_inv_pol_format_invoice_doc1_single()
	 ***********************************************************/
	fm_inv_pol_format_invoice_doc1_single(in_flistp, "SUB", 
		dbufp, ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_inv_pol_format_invoice_doc1_add_single: "
			"Error!", ebufp);
		goto cleanup;
		/***********/
	}
cleanup:	
	PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&bill_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&billinfo_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&acct_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&payinfo_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
}

/*******************************************************************
 * fm_inv_pol_format_invoice_doc1_single():
 *
 * This function gets all the required fields for the PARPAY, PARNAM,
 * PARTOT group off the input flist and translates them into
 * Name:Value pairs. 
 *
 * We pass in:
 *  in_flistp   Flist will all the invoice data
 *  dbufp	formatted buffer
 *
 *******************************************************************/
static void
fm_inv_pol_format_invoice_doc1_single(
	pin_flist_t		*in_flistp,
	char			*prefix,
	fm_inv_pol_str_buf_t	*dbufp,
	pin_errbuf_t		*ebufp)
{
	void			*vp = NULL;
	int32			currency = 0;

	pin_flist_t		*tmp_nameinfo_flistp = NULL;
	pin_flist_t		*tmp_acctinfo_flistp = NULL;
	pin_flist_t		*tmp_billinfo_flistp = NULL;
	pin_flist_t		*tmp_currency_flistp = NULL;
	pin_flist_t		*tmp_amount_flistp   = NULL;

	char			doc1tag[TAGSIZ+1];

	if (PIN_ERR_IS_ERR(ebufp))
	 	 return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Get the pointers to the required arrays on in_flistp.
	 * NOTE: We get hold of array element 0 of PIN_FLD_NAME_INFO.
	 ***********************************************************/

	tmp_nameinfo_flistp = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_NAMEINFO,
						PIN_ELEMID_ANY, 0, ebufp);

	pin_snprintf(doc1tag, sizeof(doc1tag), "%sPAY", prefix);
	/***********************************************************
	 * Get the fields of the in_flistp and convert them into 
	 * Name:Value pairs. We handle the data for the PARPAY 
	 * item group here.
	 ***********************************************************/

	vp = PIN_FLIST_FLD_GET(tmp_nameinfo_flistp, PIN_FLD_NAME, 0, ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag,
			 (char *)"PIN_FLD_NAME", PIN_FLDT_STR, 
			 dbufp, (void *)vp, ebufp);

	vp = PIN_FLIST_FLD_GET(tmp_nameinfo_flistp, PIN_FLD_ADDRESS, 0, ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag, 
			 (char *)"PIN_FLD_ADDRESS", PIN_FLDT_STR, 
			 dbufp, (void *)vp, ebufp);

	vp = PIN_FLIST_FLD_GET(tmp_nameinfo_flistp, PIN_FLD_CITY, 0, ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag, 
			(char *)"PIN_FLD_CITY", PIN_FLDT_STR, 
			dbufp, (void *)vp, ebufp);

	vp = PIN_FLIST_FLD_GET(tmp_nameinfo_flistp, PIN_FLD_STATE, 0, ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag,
			(char *)"PIN_FLD_STATE", PIN_FLDT_STR, 
			dbufp, (void *)vp, ebufp);

	vp = PIN_FLIST_FLD_GET(tmp_nameinfo_flistp, PIN_FLD_ZIP, 0, ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag,
	 (char *)"PIN_FLD_ZIP", PIN_FLDT_STR, dbufp, (void *)vp, ebufp);

	vp = PIN_FLIST_FLD_GET(tmp_nameinfo_flistp, PIN_FLD_COUNTRY, 0, ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag,
	 (char *)"PIN_FLD_COUNTRY", PIN_FLDT_STR, dbufp, (void *)vp, ebufp); 

	/***********************************************************
	 * Get the fields of the in_flistp and convert them into 
	 * Name:Value pairs. We handle the data for the PARNAM 
	 * item group here.
	 ***********************************************************/

	tmp_acctinfo_flistp = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_ACCTINFO,
						PIN_ELEMID_ANY, 0, ebufp);

	pin_snprintf(doc1tag, sizeof(doc1tag), "%sNAM", prefix);
	vp = PIN_FLIST_FLD_GET(tmp_acctinfo_flistp, PIN_FLD_ACCOUNT_NO, 0,
				 ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag,
	 (char *)"PIN_FLD_ACCOUNT_NO", PIN_FLDT_STR, dbufp, (void *)vp, ebufp);

	vp = PIN_FLIST_FLD_GET(tmp_nameinfo_flistp, PIN_FLD_SALUTATION, 0,
				 ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag,
	 (char *)"PIN_FLD_SALUTATION", PIN_FLDT_STR, dbufp, (void *)vp, ebufp);

	vp = PIN_FLIST_FLD_GET(tmp_nameinfo_flistp, PIN_FLD_LAST_NAME, 0,
				 ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag,
	 (char *)"PIN_FLD_LAST_NAME", PIN_FLDT_STR, dbufp, (void *)vp,
	 ebufp);

	vp = PIN_FLIST_FLD_GET(tmp_nameinfo_flistp, PIN_FLD_FIRST_NAME, 0,
				 ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag,
	 (char *)"PIN_FLD_FIRST_NAME", PIN_FLDT_STR, dbufp, (void *)vp, ebufp);

	vp = PIN_FLIST_FLD_GET(tmp_nameinfo_flistp, PIN_FLD_MIDDLE_NAME, 0,
				 ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag,
	 (char *)"PIN_FLD_MIDDLE_NAME", PIN_FLDT_STR, dbufp, (void *)vp,
	 ebufp);

	vp = PIN_FLIST_FLD_GET(tmp_nameinfo_flistp, PIN_FLD_TITLE, 0, ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag,
	 (char *)"PIN_FLD_TITLE", PIN_FLDT_STR, dbufp, (void *)vp, ebufp);

	vp = PIN_FLIST_FLD_GET(tmp_nameinfo_flistp, PIN_FLD_COMPANY, 0, ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag,
	 (char *)"PIN_FLD_COMPANY", PIN_FLDT_STR, dbufp, (void *)vp,
	 ebufp);

	vp = PIN_FLIST_FLD_GET(tmp_nameinfo_flistp, PIN_FLD_ADDRESS, 0, ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag,
	 (char *)"PIN_FLD_ADDRESS", PIN_FLDT_STR, dbufp, (void *)vp,
	 ebufp);

	vp = PIN_FLIST_FLD_GET(tmp_nameinfo_flistp, PIN_FLD_CITY, 0, ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag,
	 (char *)"PIN_FLD_CITY", PIN_FLDT_STR, dbufp, (void *)vp,
	 ebufp);

	vp = PIN_FLIST_FLD_GET(tmp_nameinfo_flistp, PIN_FLD_STATE, 0, ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag,
	 (char *)"PIN_FLD_STATE", PIN_FLDT_STR, dbufp, (void *)vp,
	 ebufp);

	vp = PIN_FLIST_FLD_GET(tmp_nameinfo_flistp, PIN_FLD_ZIP, 0, ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag,
	 (char *)"PIN_FLD_ZIP", PIN_FLDT_STR, dbufp, (void *)vp,
	 ebufp);

	vp = PIN_FLIST_FLD_GET(tmp_nameinfo_flistp, PIN_FLD_COUNTRY, 0, ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag,
	 (char *)"PIN_FLD_COUNTRY", PIN_FLDT_STR, dbufp, (void *)vp,
	 ebufp);

	/***********************************************************
	 * Get the fields of the in_flistp and convert them into 
	 * Name:Value pairs. We handle the data for the PARTOT 
	 * item group here.
	 ***********************************************************/

	tmp_billinfo_flistp = PIN_FLIST_ELEM_GET(in_flistp, 
						 PIN_FLD_BILLINFO, 0,
	 	 	 	 	 	 0, ebufp);
	pin_snprintf(doc1tag, sizeof(doc1tag), "%sTOT", prefix);

	vp = PIN_FLIST_FLD_GET(tmp_billinfo_flistp, PIN_FLD_BILL_NO, 0, ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag,
	 (char *)"PIN_FLD_BILL_NO", PIN_FLDT_STR, dbufp, (void *)vp, ebufp);

	vp = PIN_FLIST_FLD_GET(tmp_billinfo_flistp, PIN_FLD_START_T, 0, ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag,
	 (char *)"PIN_FLD_START_T", PIN_FLDT_TSTAMP, dbufp, (void *)vp, ebufp);

	vp = PIN_FLIST_FLD_GET(tmp_billinfo_flistp, PIN_FLD_END_T, 0, ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag,
	 (char *)"PIN_FLD_END_T", PIN_FLDT_TSTAMP, dbufp, (void *)vp, ebufp);

	vp = PIN_FLIST_FLD_GET(tmp_billinfo_flistp, PIN_FLD_PREVIOUS_TOTAL, 
			       0, ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag,
	 (char *)"PIN_FLD_PREVIOUS_TOTAL", PIN_FLDT_DECIMAL, dbufp, 
	 (void *)vp, ebufp);

	vp = PIN_FLIST_FLD_GET(tmp_billinfo_flistp, PIN_FLD_SUBORDS_TOTAL, 
			       0, ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag,
	 (char *)"PIN_FLD_SUBORDS_TOTAL", PIN_FLDT_DECIMAL, 
	  dbufp, (void *)vp, ebufp);

	vp = PIN_FLIST_FLD_GET(tmp_billinfo_flistp, PIN_FLD_CURRENT_TOTAL, 
			       0, ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag,
	 (char *)"PIN_FLD_CURRENT_TOTAL", PIN_FLDT_DECIMAL, dbufp, 
	 (void *)vp, ebufp);

	vp = PIN_FLIST_FLD_GET(tmp_billinfo_flistp, PIN_FLD_TOTAL_DUE, 0,
		 ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag,
	 (char *)"PIN_FLD_TOTAL_DUE", PIN_FLDT_DECIMAL, dbufp, 
	 (void *)vp, ebufp);

	/***********************************************************
	 * Add the currency information. There should always be a
	 * primary currency.
	 ***********************************************************/
	vp = PIN_FLIST_FLD_GET(tmp_billinfo_flistp, PIN_FLD_CURRENCY, 
			       0, ebufp);

	if (vp) currency = *(int32 *)vp;
	else currency = 0;

	tmp_currency_flistp = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_CURRENCIES,
				 currency, 0, ebufp);

	vp = PIN_FLIST_FLD_GET(tmp_currency_flistp, PIN_FLD_NAME, 0, ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag,
		 (char *)"PIN_FLD_CUR_NAME", PIN_FLDT_STR, dbufp,
		 (void *)vp, ebufp);

	vp = PIN_FLIST_FLD_GET(tmp_currency_flistp, PIN_FLD_SYMBOL, 0, ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag,
		 (char *)"PIN_FLD_CUR_SYMBOL", PIN_FLDT_STR, dbufp,
		 (void *)vp, ebufp);

	vp = PIN_FLIST_FLD_GET(tmp_currency_flistp, PIN_FLD_BEID_STR_CODE, 0,
		 ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag,
		 (char *)"PIN_FLD_CUR_ISO_CODE", PIN_FLDT_STR, dbufp,
		 (void *)vp, ebufp);


	/***********************************************************
	 * If we have a secondary currency, then add the required
	 * informatin. 
	 ***********************************************************/
	vp = PIN_FLIST_FLD_GET(tmp_billinfo_flistp, PIN_FLD_CURRENCY_SECONDARY,
				 0, ebufp);

	if (vp) currency = *(int32 *)vp;
	else currency = 0;

	if (currency != 0) {
					
		tmp_currency_flistp = PIN_FLIST_ELEM_GET(in_flistp,
				 PIN_FLD_CURRENCIES, currency, 0, ebufp);

		/***************************************************
		 * Add the currency rate and operator.
	 	 ***************************************************/
		vp = PIN_FLIST_FLD_GET(tmp_currency_flistp, 
			PIN_FLD_CURRENCY_RATE, 0, ebufp);
		fm_inv_pol_format_invoice_doc1_addfield(doc1tag,
		 (char *)"PIN_FLD_CUR_RATE", PIN_FLDT_DECIMAL, dbufp,
		 (void *)vp, ebufp);

		vp = PIN_FLIST_FLD_GET(tmp_currency_flistp, 
			PIN_FLD_CURRENCY_OPERATOR, 0, ebufp);
		fm_inv_pol_format_invoice_doc1_addfield(doc1tag,
		 (char *)"PIN_FLD_CUR_OPERATOR", PIN_FLDT_ENUM, dbufp,
		 (void *)vp, ebufp);

		
		/***************************************************
		 * Add the converted currency amounts. 
		 * NOTE: Element 1 contains the converted amounts.
	 	 ***************************************************/
		tmp_amount_flistp = PIN_FLIST_ELEM_GET(in_flistp,
					PIN_FLD_BILLINFO, 1, 1, ebufp);

		if (tmp_amount_flistp != NULL) {
			vp = PIN_FLIST_FLD_GET(tmp_amount_flistp, 
				PIN_FLD_PREVIOUS_TOTAL, 0, ebufp);
			fm_inv_pol_format_invoice_doc1_addfield(doc1tag,
				"PIN_FLD_PREVIOUS_TOTAL_SEC", 
				PIN_FLDT_DECIMAL, 
				dbufp, (void *)vp, ebufp);

			vp = PIN_FLIST_FLD_GET(tmp_amount_flistp, 
				PIN_FLD_SUBORDS_TOTAL, 0, ebufp);
			fm_inv_pol_format_invoice_doc1_addfield(doc1tag,
				"PIN_FLD_SUBORDS_TOTAL_SEC", 
				PIN_FLDT_DECIMAL, 
				dbufp, (void *)vp, ebufp);

			vp = PIN_FLIST_FLD_GET(tmp_amount_flistp, 
				PIN_FLD_CURRENT_TOTAL, 0, ebufp);
			fm_inv_pol_format_invoice_doc1_addfield(doc1tag,
				"PIN_FLD_CURRENT_TOTAL_SEC", 
				PIN_FLDT_DECIMAL, 
				dbufp, (void *)vp, ebufp);

			vp = PIN_FLIST_FLD_GET(tmp_amount_flistp, 
				PIN_FLD_TOTAL_DUE, 0, ebufp);
			fm_inv_pol_format_invoice_doc1_addfield(doc1tag, 
				"PIN_FLD_TOTAL_DUE_SEC", 
				PIN_FLDT_DECIMAL, 
				dbufp, (void *)vp, ebufp);
		}
		
		/***************************************************
		 * Add the secondary currency name, symbol, iso code. 
	 	 ***************************************************/

		vp = PIN_FLIST_FLD_GET(tmp_currency_flistp, PIN_FLD_NAME, 0,
				ebufp);
		fm_inv_pol_format_invoice_doc1_addfield(doc1tag,
		 (char *)"PIN_FLD_CUR_NAME", PIN_FLDT_STR, dbufp,
		 (void *)vp, ebufp);

		vp = PIN_FLIST_FLD_GET(tmp_currency_flistp, PIN_FLD_SYMBOL, 0,
				ebufp);
		fm_inv_pol_format_invoice_doc1_addfield(doc1tag,
		 (char *)"PIN_FLD_CUR_SYMBOL", PIN_FLDT_STR, dbufp,
		 (void *)vp, ebufp);

		vp = PIN_FLIST_FLD_GET(tmp_currency_flistp,
			 PIN_FLD_BEID_STR_CODE, 0, ebufp);
		fm_inv_pol_format_invoice_doc1_addfield(doc1tag,
		 (char *)"PIN_FLD_CUR_ISO_CODE", PIN_FLDT_STR, dbufp,
		 (void *)vp, ebufp);
	}

	/***********************************************************
	 * Error?
	 ***********************************************************/

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
	 	 	 "fm_inv_pol_format_invoice_default_DOC1_PAR: error" \
	 	 	 " creating FLISTS", ebufp);
		goto ErrOut;
	}

	return;
ErrOut:
	return;
}

static void
fm_inv_pol_format_invoice_doc1_repeats_service(
	pcm_context_t           *ctxp,
	pin_flist_t		*in_flistp,
	int32			fld_no,
	poid_t			*last_bill_obj,
	fm_inv_pol_str_buf_t	*dbufp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*service_flistp = NULL;
	int32			elemid = 0;
	pin_cookie_t		cookie = NULL;
	int32                   service_enabled = 0;
	void			*vp = NULL;

	/***********************************************************
	 * Is the service centric feature enabled?
	 ***********************************************************/
	vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_FLAGS, 1, ebufp);
	if (vp != NULL) {
		service_enabled = *(int32 *)vp;
	}
	
	if (service_enabled) {
		while ((service_flistp = PIN_FLIST_ELEM_GET_NEXT(
				in_flistp, fld_no, &elemid, 1, &cookie, 
				ebufp)) != (pin_flist_t *)NULL) {

			/** Further coding needed here to put service specific
			    line items in the DOC1 output raw file */
			fm_inv_pol_format_invoice_doc1_repeats(
				ctxp, in_flistp, PIN_FLD_ITEMS, service_flistp,
				last_bill_obj, dbufp, ebufp);
		}
	} else { /* Normal invoice */
		fm_inv_pol_format_invoice_doc1_repeats(
			ctxp, in_flistp, fld_no, last_bill_obj, NULL,
			dbufp, ebufp);
	}
}

/*******************************************************************
 * fm_inv_pol_format_invoice_doc1_repeats():
 *
 * This function translates the events and items into Name:Value
 * pairs. 
 *******************************************************************/
static void
fm_inv_pol_format_invoice_doc1_repeats(
	pcm_context_t           *ctxp,
	pin_flist_t		*in_flistp,
	int32			fld_no,
	pin_flist_t		*service_flistp,
	poid_t			*last_bill_obj,
	fm_inv_pol_str_buf_t	*dbufp,
	pin_errbuf_t		*ebufp)
{

	pin_flist_t		*item_flistp = NULL;
	pin_flist_t		*event_flistp = NULL;
	pin_flist_t		*bill_flistp = NULL;

	int32			elemid = 0;
	pin_cookie_t		cookie = NULL;
	int32			inner_elemid = 0;
	pin_cookie_t		inner_cookie = NULL;

	poid_t			*cur_bill_obj = NULL;

	char			r_doc1tag[TAGSIZ+1];

	if (PIN_ERR_IS_ERR(ebufp))
 		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	memset(r_doc1tag, 0, sizeof(r_doc1tag));

	/***********************************************************
 	 * Loop through the items array and identify if we have a
 	 * parent item or a subordinate item.
 	 ***********************************************************/
	if (service_flistp == NULL) {
		service_flistp = in_flistp;
	}

	while ((item_flistp = PIN_FLIST_ELEM_GET_NEXT(service_flistp, fld_no,
		&elemid, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) {

		cur_bill_obj = (poid_t *)PIN_FLIST_FLD_GET(item_flistp,
			PIN_FLD_BILL_OBJ, 0, ebufp);
		if (fld_no != PIN_FLD_AR_ITEMS && PIN_POID_COMPARE(
				cur_bill_obj, last_bill_obj, 
				0, ebufp)) {
			last_bill_obj = cur_bill_obj;
			fm_inv_pol_format_invoice_doc1_add_single(ctxp,
				cur_bill_obj, dbufp, ebufp);
		}
		/***************************************************
 	 	 * Convert the item. 
 	 	 ***************************************************/

		fm_inv_pol_format_invoice_doc1_repeats_item(item_flistp, 
			in_flistp, dbufp, (char *)r_doc1tag, ebufp);

		/***************************************************
 	 	 * Loop through the events for the current item.
	 	 * First set up the event tag (add 'EVEO').
 	 	 ***************************************************/

		inner_elemid = 0;
		inner_cookie = NULL;

		sprintf(r_doc1tag, "%sEVEO", r_doc1tag);

		while ((event_flistp = PIN_FLIST_ELEM_GET_NEXT(item_flistp,
			PIN_FLD_EVENTS, &inner_elemid, 1, &inner_cookie, 
			ebufp)) != (pin_flist_t *)NULL) {

			fm_inv_pol_format_invoice_doc1_repeats_event(
				event_flistp, in_flistp, dbufp, r_doc1tag, 
				ebufp);

		} /* end - event while loop */

	} /* end - item while loop */

	/***************************************************
	 * Error?
	 ***************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		 "fm_inv_pol_format_invoice_doc1_repeats: error" \
		 " creating item/event Name:Value pairs", ebufp);
		goto ErrOut;
	}

	return;
ErrOut:
	return;
}

/*******************************************************************
 * fm_inv_pol_format_invoice_doc1_repeats_item():
 *
 * This function converts the items into Name:Value pairs. It also
 * distinguishes between parent and subordinate accounts.
 *******************************************************************/
static void
fm_inv_pol_format_invoice_doc1_repeats_item(
	pin_flist_t		*iitem_flistp,
	pin_flist_t		*in_flistp,
	fm_inv_pol_str_buf_t	*dbufp,
	char			*r_doc1tag,	
	pin_errbuf_t		*ebufp)
{
	void			*vp = NULL;
	char			*item_type;
	char			doc1tag[TAGSIZ+1];
	char			bill_tag[3+1];

	pin_flist_t		*tmp_acctinfo_flistp = NULL;
	poid_t			*acct_pdp = NULL;

	char			errmsg[100];

	if (PIN_ERR_IS_ERR(ebufp))
 		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	memset( doc1tag, 0, sizeof( doc1tag));

	/***********************************************************
 	 * Work out if we have got a parent or subordinate item.
 	 ***********************************************************/

	tmp_acctinfo_flistp = PIN_FLIST_ELEM_GET(in_flistp, 
				PIN_FLD_ACCTINFO, 0, 0, ebufp);

	acct_pdp = (poid_t *)PIN_FLIST_FLD_GET(tmp_acctinfo_flistp,
			PIN_FLD_ACCOUNT_OBJ, 0, ebufp);

	vp = PIN_FLIST_FLD_GET(iitem_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);

	if ( vp && PIN_POID_COMPARE((poid_t *)acct_pdp, 
				    (poid_t *)vp, 0, ebufp) == 0) {
		strcpy(bill_tag, "PAR");
	}
	else {
		strcpy(bill_tag, "SUB");
	}

	/***************************************************
	 * Work out what type of item it is.
 	 ***************************************************/

	vp = PIN_FLIST_FLD_GET(iitem_flistp, PIN_FLD_NAME, 0, ebufp);

	if (vp) item_type = (char *)vp;
	else item_type = "";

	if (strstr(item_type, "Cycle forward") != (char *)NULL) {
		sprintf(doc1tag, "%s%s", bill_tag, "CFO");
	}
	else if (strstr(item_type, "Usage") != (char *)NULL) {
		sprintf(doc1tag, "%s%s", bill_tag, "USG");
	}
	else if (strstr(item_type, "Payment") != (char *)NULL) {
		sprintf(doc1tag, "%s%s", bill_tag, "PMT");
	}
	else if (strstr(item_type, "Payment Reversal") != (char *)NULL) {
		sprintf(doc1tag, "%s%s", bill_tag, "REV");
	}
	else if (strstr(item_type, "Dispute") != (char *)NULL) {
		sprintf(doc1tag, "%s%s", bill_tag, "DIS");
	}
	else if (strstr(item_type, "Settlement") != (char *)NULL) {
		sprintf(doc1tag, "%s%s", bill_tag, "SET");
	}
	else if (strstr(item_type, "Adjustment") != (char *)NULL) {
		sprintf(doc1tag, "%s%s", bill_tag, "ADJ");
	}
	else if (strstr(item_type, "Write-off") != (char *)NULL) {
		sprintf(doc1tag, "%s%s", bill_tag, "WRO");
	}
	else {
		sprintf(doc1tag, "%s", bill_tag);
	}

	/* NSC - if always excatly correct string (e.g. 'Usage')
		 then use ENUM and lookup up type...
	*/


	/***************************************************
	 * Create the Name:Value pairs for the current item.
 	 ***************************************************/

	vp = PIN_FLIST_FLD_GET(iitem_flistp, PIN_FLD_DUE_T, 0, ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag, (char *)"PIN_FLD_DUE_T",
		PIN_FLDT_TSTAMP, dbufp, (void *)vp, ebufp);

	vp = PIN_FLIST_FLD_GET(iitem_flistp, PIN_FLD_EFFECTIVE_T, 0, ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag, 
	 (char *)"PIN_FLD_EFFECTIVE_T", PIN_FLDT_TSTAMP, dbufp, 
	  (void *)vp, ebufp);

	vp = PIN_FLIST_FLD_GET(iitem_flistp, PIN_FLD_NAME, 0, ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag, (char *)"PIN_FLD_NAME",
		PIN_FLDT_STR, dbufp, (void *)vp, ebufp);

	vp = PIN_FLIST_FLD_GET(iitem_flistp, PIN_FLD_ITEM_NO, 0, ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag, (char *)"PIN_FLD_ITEM_NO",
		PIN_FLDT_STR, dbufp, (void *)vp, ebufp);

	vp = PIN_FLIST_FLD_GET(iitem_flistp, PIN_FLD_ITEM_TOTAL, 0, ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag, (char *)"PIN_FLD_ITEM_TOTAL",
		PIN_FLDT_DECIMAL, dbufp, (void *)vp, ebufp);

	/***********************************************************
	 * Error?
	 ***********************************************************/

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
	 	 	 "fm_inv_pol_format_invoice_doc1_repeats_item: error" \
	 	 	 " creating item Name:Value pair", ebufp);

		goto cleanup;
		/***********/
	}

	/* This could be done more effeciently */
	strcpy(r_doc1tag, doc1tag);
	return;

cleanup:
	r_doc1tag[0] = '\0' ;
	return;

}

/*******************************************************************
 * fm_inv_pol_format_invoice_doc1_repeats_event():
 *
 * This function converts the events into Name:Value pairs.
 *******************************************************************/
static void
fm_inv_pol_format_invoice_doc1_repeats_event(
	pin_flist_t		*ievent_flistp,
	pin_flist_t		*in_flistp,
	fm_inv_pol_str_buf_t	*dbufp,
	char			*doc1tag,	
	pin_errbuf_t		*ebufp)
{
	void			*vp = NULL;
	pin_flist_t		*tmp_acctinfo_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	int32			currency = 0;

	if (PIN_ERR_IS_ERR(ebufp))
 		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Convert the event info into the Name:Value pair. 
	 ***********************************************************/

	vp = PIN_FLIST_FLD_GET(ievent_flistp, PIN_FLD_END_T, 0, ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag, (char *)"PIN_FLD_END_T",
		PIN_FLDT_TSTAMP, dbufp, (void *)vp, ebufp);

	vp = PIN_FLIST_FLD_GET(ievent_flistp, PIN_FLD_SYS_DESCR, 0, ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag, (char *)"PIN_FLD_SYS_DESCR",
		PIN_FLDT_STR, dbufp, (void *)vp, ebufp);

	vp = PIN_FLIST_FLD_GET(ievent_flistp, PIN_FLD_LABEL, 1, ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag, (char *)"PIN_FLD_RATE_TAG",
		PIN_FLDT_STR, dbufp, (void *)vp, ebufp);

	/***********************************************************
	 * Get the amount that corresponds to our PIN_FLD_CURRENCY
	 * value in PIN_FLD_ACCTINFO. 
	 ***********************************************************/

	tmp_acctinfo_flistp = PIN_FLIST_ELEM_GET(in_flistp, 
				PIN_FLD_ACCTINFO, 0, 0, ebufp);
	vp = PIN_FLIST_FLD_GET(tmp_acctinfo_flistp, PIN_FLD_CURRENCY,
				0, ebufp);

	if (vp) currency = *(int32 *)vp;
	else currency = 0;
 
	tmp_flistp = PIN_FLIST_ELEM_GET(ievent_flistp, PIN_FLD_TOTAL, 
				currency, 0, ebufp);
	vp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_AMOUNT, 0, ebufp);
	fm_inv_pol_format_invoice_doc1_addfield(doc1tag, (char *)"PIN_FLD_AMOUNT",
		PIN_FLDT_DECIMAL, dbufp, (void *)vp, ebufp);


	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
	 	 	 "fm_inv_pol_format_invoice_doc1_repeats_event: error" \
	 	 	 " creating events Name:Value pair", ebufp);
		goto ErrOut;
	}

	return;
ErrOut:
	return;
}


/*******************************************************************
 * fm_inv_pol_format_invoice_doc1_addfield():
 *
 * This function converts the field information (doc1tag, name, value)
 * into a NAME:VALUE pair and adds it to our char buffer.   
 *
 * The following format is used:
 *
 *      <DOC1 TAG><TAB><PIN FIELD NAME><"><VALUE><">
 *
 *      where the DOC1 TAG is fixed width (10 bytes)
 *
 * e.g. PARPAY   PIN_FLD_NAME"Steve"
 *      PARPAY   PIN_FLD_ADDRESS"Steven's Creek"
 *      ...
 *      PARTOT   PIN_FLD_BILL_NO"B-2"
 *
 * We pass in:
 *  doc1tag	Character pointer to the DOC1 item group name e.g. 
 *		PARPAY, PARTOT, SUBCFO, etc. 
 *  fieldname 	Character pointer to the name of the PIN field e.g.
 *		"PIN_FLD_NAME", "PIN_FLD_AMOUNT", etc.
 *  type	Type of the PIN field  
 *  bufp	Pointer to char buffer (also our return variable)
 *  lenp	Pointer to length of string
 *  vp		Void pointer to the actual value for a given field
 *
 *******************************************************************/
static void
fm_inv_pol_format_invoice_doc1_addfield(
	char			*doc1tag,
	char			*fieldname, 
	int32			type,
	fm_inv_pol_str_buf_t	*dbufp,
	void			*vp,
	pin_errbuf_t		*ebufp)
{
	char			tbuf[1024];

	char			errmsg[100];

	int32			ival = 0;
	double			dval = 0.0;
	char			*amountp = NULL;
	char			datestr[50];
	char			rate_tag[256] ="";
	time_t                  time_val = 0;

	if (PIN_ERR_IS_ERR(ebufp))
	 	 return;
	PIN_ERR_CLEAR_ERR(ebufp);

	memset(tbuf, 0, sizeof(tbuf));

	/***********************************************************
	 * Now handle the data of this field. 
	 * NOTE: We don't handle arrays and substructures! 
	 ***********************************************************/
	switch (type) {

	case PIN_FLDT_INT:
		if (vp) ival = *((int32 *)vp);
		else ival = 0;
		pin_snprintf(tbuf, sizeof(tbuf), "%-10s%s%.1024s%s\"%d\"\n", doc1tag, SEPARATOR_DOC1,
			 fieldname, SEPARATOR_DOC1, ival);
		fm_inv_pol_add_str_to_buf( dbufp, tbuf, ebufp );
		break;

	case PIN_FLDT_ENUM:
		if (vp) ival = *((int32 *)vp);
		else ival = 0;
		pin_snprintf(tbuf, sizeof(tbuf), "%-10s%s%.1024s%s\"%d\"\n", doc1tag, SEPARATOR_DOC1,
			 fieldname, SEPARATOR_DOC1, ival);
		fm_inv_pol_add_str_to_buf( dbufp, tbuf, ebufp );
		break;

	case PIN_FLDT_TSTAMP:
	        if (vp) time_val = *(time_t *)vp;
                else time_val = 0;
		pin_strftimet(datestr, sizeof(datestr), "%c",
				 time_val);
		pin_snprintf(tbuf, sizeof(tbuf), "%-10s%s%.1024s%s\"(%d) %s\"\n", doc1tag, 
			SEPARATOR_DOC1, fieldname, SEPARATOR_DOC1, 
			time_val,  datestr);
		fm_inv_pol_add_str_to_buf( dbufp, tbuf, ebufp );
		break;

	case PIN_FLDT_NUM:
		if (vp) dval = *((double *)vp);
		else dval = 0.0;
		sprintf(tbuf, "%-10s%s%s%s\"%.2f\"\n", doc1tag, SEPARATOR_DOC1,
			 fieldname, SEPARATOR_DOC1, dval);
		fm_inv_pol_add_str_to_buf( dbufp, tbuf, ebufp );
		break;
	
	case PIN_FLDT_STR:
		if(strcmp(fieldname,"PIN_FLD_RATE_TAG")==0){

			if (vp == NULL) { /* Set default locale */
                                strcpy(rate_tag, "N/A");
                        } else {
                                strcpy(rate_tag, (char *)vp);
                        }
			pin_snprintf(tbuf, sizeof(tbuf), "%-10s%s%.1024s%s\"%s\"\n", doc1tag, SEPARATOR_DOC1,
                         fieldname, SEPARATOR_DOC1, rate_tag);
		} else {
			
	  		if( vp == NULL ) vp = "";
			pin_snprintf(tbuf, sizeof(tbuf), "%-10s%s%.1024s%s\"%s\"\n", doc1tag, SEPARATOR_DOC1,
			 	fieldname, SEPARATOR_DOC1, (char *)vp);
		}
		fm_inv_pol_add_str_to_buf( dbufp, tbuf, ebufp );
		break;

	case PIN_FLDT_DECIMAL:
		amountp = pbo_decimal_to_str((pin_decimal_t *)vp, ebufp);
		pin_snprintf(tbuf, sizeof(tbuf), "%-10s%s%.128s%s\"%s\"\n", doc1tag, SEPARATOR_DOC1,
			 fieldname, SEPARATOR_DOC1, amountp);
		pin_free((void *)amountp);
		fm_inv_pol_add_str_to_buf( dbufp, tbuf, ebufp );
		break;
		
	case PIN_FLDT_ARRAY:
	case PIN_FLDT_SUBSTRUCT:
	case PIN_FLDT_BUF:
	case PIN_FLDT_BINSTR:
	case PIN_FLDT_POID:
	case PIN_FLDT_OBJ:
		/* FALLTHRU */
	default:
		/* Error */
		sprintf(errmsg, "fm_inv_pol_format_invoice_doc1_addfield:"
			" Unknown type (%d)", type);
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_ARG, 0, 0, 0);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, errmsg, ebufp );
		goto ErrOut;
	}	

	return;
ErrOut:
	return;
}
