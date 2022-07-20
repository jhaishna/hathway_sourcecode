/*******************************************************************
 *
* Copyright (c) 2000, 2016, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char    Sccs_id[] = "(#)$Id: fm_inv_pol_format_invoice_html.c /cgbubrm_7.5.0.portalbase/2 2016/04/19 13:26:50 vivilin Exp $";
#endif


#include <stdio.h>
#include <string.h>
#include "pcm.h"
#include "ops/inv.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "fm_bill_utils.h"
#include "cm_cache.h"

#include "pin_inv.h"

#include "fm_inv_pol_common.h"


/*******************************************************************
 * Global cache pointers (from fm_inv_pol_init)
 *******************************************************************/

/*******************************************************************
 * Locally used structure.
 *******************************************************************/
typedef struct {
	char token[20];
	char *value;
} fm_inv_pol_html_token_t;
	
#define CORPADDR1 	"Oracle Corporation"
#define CORPADDR2	"500 Oracle Parkway"
#define CORPADDR3	"Redwood Shores, CA 94065"
#define CORPADDR4 	"USA"
#define CORPADDR5	""
#define TAXAMT		"Tax Amount"

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_inv_pol_format_invoice_html();

static void
fm_inv_pol_format_invoice_html(
	pcm_context_t           *ctxp,
	int32                     flags,
	pin_flist_t             *in_flistp,
	pin_flist_t             **ret_flistpp,
	pin_errbuf_t            *ebufp);

static void
fm_inv_pol_format_invoice_html_prep_fill_template(
	pcm_context_t           *ctxp,
	pin_flist_t             *in_flistp,
	fm_inv_pol_str_buf_t	*dbufp,
	pin_errbuf_t            *ebufp);

static void
fm_inv_pol_format_invoice_html_get_tax_body(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        fm_inv_pol_str_buf_t    *taxbufp,
        pin_errbuf_t            *ebufp);

static void
fm_inv_pol_format_invoice_html_get_detail_body(
	pin_flist_t		*in_flistp,
	int32			fld_no,
	int32			currency,
	char			*locale,
	fm_inv_pol_str_buf_t	*itembufp,
	fm_inv_pol_str_buf_t	*eventbufp,
        pin_errbuf_t            *ebufp);

static void
fm_inv_pol_format_invoice_html_get_items_events_body(
	pin_flist_t		*in_flistp,
	int32			fld_no,
	int32			currency,
	fm_inv_pol_str_buf_t	*itembufp,
	fm_inv_pol_str_buf_t	*eventbufp,
        pin_errbuf_t            *ebufp);

static void
fm_inv_pol_format_invoice_html_fill_template(
	fm_inv_pol_str_buf_t	*bufp,
        char                    *template,
	int32			template_size,
        fm_inv_pol_html_token_t	*tokenp,
	int			tokens,
        pin_errbuf_t            *ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_INV_POL_FORMAT_INVOICE_HTML operation.
 *
 * NOTE:
 * This opcode will become OBSOLETE in future Portal versions.
 *******************************************************************/
void
op_inv_pol_format_invoice_html(connp, opcode, flags, in_flistp, 
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
	if (opcode != PCM_OP_INV_POL_FORMAT_INVOICE_HTML) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_inv_pol_format_invoice_html opcode error", 
			ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_inv_pol_format_invoice_html input flist", in_flistp);

	/***********************************************************
	 * Do the actual op in a sub. Copy it since we may need to
	 * replace it later.
	 ***********************************************************/

	fm_inv_pol_format_invoice_html(ctxp, flags, in_flistp, 
				       ret_flistpp, ebufp);

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_inv_pol_format_invoice_html error", ebufp);
	} else {
		/***************************************************
		 * Debug: What we're sending back.
		 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_inv_pol_format_invoice_html return flist", 
				  *ret_flistpp);
	}
	return;
}

/*******************************************************************
 * fm_inv_pol_format_invoice_html():
 *
 * This function converts the input flist into the default Portal
 * invoicing output for the HTML format.
 *
 * NOTE:
 * By default this function is called from either of the following
 * policy opcode:
 *             PCM_OP_INV_POL_FORMAT_INVOICE
 *             PCM_OP_INV_POL_VIEW_INVOICE
 *
 * Description:
 *******************************************************************/
static void
fm_inv_pol_format_invoice_html(
	pcm_context_t           *ctxp,
	int32                     flags,
	pin_flist_t             *in_flistp,
	pin_flist_t             **ret_flistpp,
	pin_errbuf_t            *ebufp)
{
	fm_inv_pol_str_buf_t	dbuf;


	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	if( in_flistp == NULL ) {
		return;
	}

	/***********************************************************
	 * Reset ret_flistpp.
	 ***********************************************************/
	*ret_flistpp = NULL;

        /*******************************************************
         * Initialize dbuf - will hold the html formatted buffer
         *******************************************************/

        dbuf.strp = NULL;
        dbuf.size = 0;
        dbuf.strsize = 0;
        dbuf.chunksize = 4096;




	/***********************************************************
	 * Dump the template
	 ***********************************************************/
	fm_inv_pol_format_invoice_html_prep_fill_template(ctxp, in_flistp, 
		&dbuf, ebufp);

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
		FM_INV_FORMAT_TYPE_HTML, ret_flistpp, ebufp);

	if( dbuf.strp ) {
		pin_free( dbuf.strp );
	}
	return;
}

static void
fm_inv_pol_format_invoice_html_prep_fill_template(
	pcm_context_t           *ctxp,
	pin_flist_t             *in_flistp,
	fm_inv_pol_str_buf_t	*dbufp,
	pin_errbuf_t            *ebufp)
{
	fm_inv_pol_html_token_t	token_arr[10];
	int			tokens = 0;
	int			i = 0;

	pin_flist_t		*template_flistp = NULL;
	poid_t			*a_pdp = NULL;
	pin_buf_t		*templatep = NULL;

	char			*billno = NULL;
	char			billdate_start[50] = {""};
	char			billdate_end[50] = {""};
	fm_inv_pol_str_buf_t	itembuf;
	fm_inv_pol_str_buf_t	eventbuf;
	pin_flist_t		*tmp_billinfo_flistp = NULL;
	pin_flist_t		*tmp_acctinfo_flistp = NULL;
	pin_flist_t		*tmp_nameinfo_flistp = NULL;
	pin_flist_t		*tmp_currency_flistp = NULL;
	pin_flist_t		*tmp_amount_flistp = NULL;
	void			*vp = NULL;
	char			*corporatelogo = "./image.gif";
	char			tmp_str[4096] = {""};
	char			*acctno = NULL;
	char			duedate[50] = {""};
	int32			currency = 0;
	int32			currency2 = 0;
	pin_decimal_t		*pdp = NULL;
	pin_decimal_t		*pdp_rounded = NULL;
	char			symbol[32] = {""}; 
	char			symbol2[32] = {""};
	char			*rate_str = NULL;
	int			currency_operator = 0;
	char			exchange_rate[128] = {""};
	char			*prev_total = NULL;
	char			*subord_total = NULL;
	char			*curr_total = NULL;
	char			*total_due = NULL;
	char			*prev_total2 = NULL;
	char			*subord_total2 = NULL;
	char			*curr_total2 = NULL;
	char			*total_due2 = NULL;
	char			table_head_col2[100] = {""};
	char			table_head_col3[100] = {""};
	char			prev_total_col2[80] = {""};
	char			prev_total_col3[80] = {""};
	char			subord_total_col2[80] = {""};
	char			subord_total_col3[80] = {""};
	char			curr_total_col2[80] = {""};
	char			curr_total_col3[80] = {""};
	char			total_due_col2[80] = {""};
	char			total_due_col3[80] = {""};
	char			table_heading_row[400] = {""};
	char			prev_total_row[256] = {""};
	char			subord_total_row[256] = {""};
	char			curr_total_row[256] = {""};
	char			total_due_row[256] = {""};
	char			hline[80] = {""};
	char			portal_locale[BUFSIZ] = {""};
	char			*locale = NULL;

	char			item_header[BUFSIZ] = {""};
	char			item_footer[128] = {""};
	char			event_header[BUFSIZ] = {""};
	char			event_footer[256] = {""};
        char                    *addr = NULL;
        char                    *addrline = NULL;
	char                    tax_header[BUFSIZ] = {""};
	fm_inv_pol_str_buf_t    taxbuf;
	char                    tax_footer[256] = {""};
	
        
	

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	

        /***********************************************************
	 * Get the array handlers.
         ***********************************************************/
        tmp_billinfo_flistp = PIN_FLIST_ELEM_GET(in_flistp, 
		PIN_FLD_BILLINFO, 0, 0, ebufp);
        tmp_acctinfo_flistp = PIN_FLIST_ELEM_GET(in_flistp, 
		PIN_FLD_ACCTINFO, 0, 0, ebufp);
        tmp_nameinfo_flistp = PIN_FLIST_ELEM_GET(in_flistp,
		 PIN_FLD_NAMEINFO, PIN_ELEMID_ANY , 0, ebufp);

	a_pdp = (poid_t *)PIN_FLIST_FLD_GET(tmp_acctinfo_flistp, 
		PIN_FLD_ACCOUNT_OBJ, 0, ebufp);

        /***********************************************************
	 * Set the locale.
         ***********************************************************/
	vp = PIN_FLIST_FLD_GET(tmp_acctinfo_flistp, PIN_FLD_LOCALE, 
		1, ebufp);
	if (vp == NULL) { /* Set default locale */
		strcpy(portal_locale, "en_US");
	} else {
		strcpy(portal_locale, (char *)vp);
	}
	locale = PIN_MAP_INFRANET_TO_PLATFORM_LOCALE(portal_locale, 
		ebufp);
        if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"Bad locale", ebufp);
		goto ErrOut;
	}
	
        /***********************************************************
	 * Now, get the template.
         ***********************************************************/
	fm_inv_pol_format_invoice_get_template(ctxp, a_pdp, portal_locale,
		"HTML", &template_flistp, ebufp);

	if (template_flistp == NULL) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
			"No template flist found!");
		goto ErrOut;
	}

	templatep = (pin_buf_t *) PIN_FLIST_FLD_GET(template_flistp, 
		PIN_FLD_BUFFER, 0, ebufp);
	if (templatep == NULL) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
			"No template buffer found!");
		goto ErrOut;
	}

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"Error loading template from /config/invoice_template",
			ebufp);
		goto ErrOut;
	}

        /***********************************************************
         * Get the currency information. There should always be a
         * primary currency.
	 * The currency will determine the precision of the amount.
         ***********************************************************/
        vp = PIN_FLIST_FLD_GET(tmp_billinfo_flistp, PIN_FLD_CURRENCY, 0, ebufp);

        if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"Primary currency not set correctly", ebufp);
		goto ErrOut;
	}

        if (vp) {
		currency = *(int32 *)vp;
	} else {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Currency not found!");
		goto ErrOut;
	}

	tmp_currency_flistp = PIN_FLIST_ELEM_GET(in_flistp,
		PIN_FLD_CURRENCIES, currency, 0, ebufp);

	vp = PIN_FLIST_FLD_GET(tmp_currency_flistp, 
		PIN_FLD_SYMBOL, 0, ebufp);
	if (vp) {
		strcpy(symbol, (char *)vp);
	}
	pdp = (pin_decimal_t *)PIN_FLIST_FLD_GET(tmp_billinfo_flistp, 
		PIN_FLD_PREVIOUS_TOTAL, 0, ebufp);
	/*************************************************************
	 * Round the amount to a proper precision 
	 * NOTE: The function takes a decimal pointer in and get the
	 * result out in the same pointer, but the old decimal was
	 * not released. That's why we can directly use it for the
	 * decimal pointer GOT from the flist.
	 *************************************************************/
	pdp_rounded = fm_utils_round_balance(pdp, currency);
	prev_total = pbo_decimal_to_str(pdp_rounded, ebufp);
	pin_decimal_destroy( pdp_rounded );

	pdp = (pin_decimal_t *)PIN_FLIST_FLD_GET(tmp_billinfo_flistp,
		PIN_FLD_SUBORDS_TOTAL, 0, ebufp);
	pdp_rounded = fm_utils_round_balance(pdp, currency);
	subord_total = pbo_decimal_to_str(pdp_rounded, ebufp);
	pin_decimal_destroy( pdp_rounded );

	pdp = (pin_decimal_t *)PIN_FLIST_FLD_GET(tmp_billinfo_flistp,
		PIN_FLD_CURRENT_TOTAL, 0, ebufp);
	pdp_rounded = fm_utils_round_balance(pdp, currency);
	curr_total = pbo_decimal_to_str(pdp_rounded, ebufp);
	pin_decimal_destroy( pdp_rounded );

	pdp = (pin_decimal_t *)PIN_FLIST_FLD_GET(tmp_billinfo_flistp,
		PIN_FLD_TOTAL_DUE, 0, ebufp);
	pdp_rounded = fm_utils_round_balance(pdp, currency);
	total_due = pbo_decimal_to_str(pdp_rounded, ebufp);
	pin_decimal_destroy( pdp_rounded );

        /***********************************************************
         * If we have a secondary currency, then add the required
         * informatin.
         ***********************************************************/
        vp = PIN_FLIST_FLD_GET(tmp_billinfo_flistp, 
		PIN_FLD_CURRENCY_SECONDARY, 1, ebufp);

        if (vp) {
		currency2 = *(int32 *)vp;
	} else {
		currency2 = 0;
	}

        if (currency2 != 0) {

                tmp_currency_flistp = PIN_FLIST_ELEM_GET(in_flistp,
			 PIN_FLD_CURRENCIES, currency2, 0, ebufp);

                /***************************************************
                 * Add the secondary currency symbol
                 ***************************************************/
                vp = PIN_FLIST_FLD_GET(tmp_currency_flistp, 
			PIN_FLD_SYMBOL, 0, ebufp);
		if (vp) {
			strcpy(symbol2, (char *)vp);
		}
                /***************************************************
                 * Add the currency rate and operator.
                 ***************************************************/
                pdp = (pin_decimal_t *)PIN_FLIST_FLD_GET(
			tmp_currency_flistp,
                        PIN_FLD_CURRENCY_RATE, 
			0, ebufp);
		rate_str = pbo_decimal_to_str(pdp, ebufp);

                vp = PIN_FLIST_FLD_GET(tmp_currency_flistp,
                        PIN_FLD_CURRENCY_OPERATOR, 0, ebufp);
		if (vp) {
			currency_operator = *(int32 *)vp;
		}
		if (currency_operator == 0) {
			pin_snprintf(exchange_rate, sizeof(exchange_rate), "1 %s = %s.%s", 
				symbol, rate_str, symbol2);
		} else {
			pin_snprintf(exchange_rate, sizeof(exchange_rate),  "1 %s = %s.%s", 
				symbol2, rate_str, symbol);
		}	
		pin_free(rate_str);

                /***************************************************
                 * Add the converted currency amounts.
                 * NOTE: Element 1 contains the converted amounts.
                 ***************************************************/
                tmp_amount_flistp = PIN_FLIST_ELEM_GET(in_flistp,
			PIN_FLD_BILLINFO, 1, 0, ebufp);

                pdp = (pin_decimal_t *)PIN_FLIST_FLD_GET(
			tmp_amount_flistp,
                        PIN_FLD_PREVIOUS_TOTAL,
			0, ebufp);
		pdp_rounded = fm_utils_round_balance(pdp, currency2);
		prev_total2 = pbo_decimal_to_str(pdp_rounded, ebufp);
		pin_decimal_destroy( pdp_rounded );

                pdp = (pin_decimal_t *)PIN_FLIST_FLD_GET(
			tmp_amount_flistp,
                        PIN_FLD_SUBORDS_TOTAL, 
			0, ebufp);
		pdp_rounded = fm_utils_round_balance(pdp, currency2);
		subord_total2 = pbo_decimal_to_str(pdp_rounded, ebufp);
		pin_decimal_destroy( pdp_rounded );

                pdp = (pin_decimal_t *)PIN_FLIST_FLD_GET(
			tmp_amount_flistp,
                        PIN_FLD_CURRENT_TOTAL, 
			0, ebufp);
		pdp_rounded = fm_utils_round_balance(pdp, currency2);
		curr_total2 = pbo_decimal_to_str(pdp_rounded, ebufp);
		pin_decimal_destroy( pdp_rounded );

                pdp = (pin_decimal_t *)PIN_FLIST_FLD_GET(
			tmp_amount_flistp,
                        PIN_FLD_TOTAL_DUE, 
			0, ebufp);
		pdp_rounded = fm_utils_round_balance(pdp, currency2);
		total_due2 = pbo_decimal_to_str(pdp_rounded, ebufp);
		pin_decimal_destroy( pdp_rounded );

        }


        /***********************************************************
	 * Replace each occurance of the Name with the Value in the 
	 * template file
	 *
	 * _INVOICENUM_
	 * _CORPIMAG_
	 * _BILLDATE_
	 *  _INVOICENUMBER_
	 * _ACCTNUM_
	 * _DUEDATE_
	 * _AMOUNTDUE_
	 * _CORPADDR_
	 * _CUSTADDR_
	 * _ACCOUNTSUMMARY_
	 * _ITEMS_
	 * _EVENTS_
         ***********************************************************/

        /***********************************************************
	 * _INVOICENUM_
         ***********************************************************/
        vp = PIN_FLIST_FLD_GET(tmp_billinfo_flistp, PIN_FLD_BILL_NO, 
		0, ebufp);

	billno = strdup((char *)vp);
	strcpy(token_arr[tokens].token, "_INVOICENUM_");
	token_arr[tokens].value = billno;
	tokens ++;

        /***********************************************************
	 * _CORPIMAG_
         ***********************************************************/
	sprintf(tmp_str, "<img src=\"%s\">", corporatelogo);
	strcpy(token_arr[tokens].token, "_CORPIMAG_");
	token_arr[tokens].value = strdup(tmp_str);
	tokens ++;
	
        /***********************************************************
	 * _BILLDATE_
         ***********************************************************/
        vp = PIN_FLIST_FLD_GET(tmp_billinfo_flistp, PIN_FLD_START_T, 
		0, ebufp);
	if (vp) {
		pin_strftimet(billdate_start, sizeof(billdate_start), 
			"%b %d %Y", *(time_t *)vp);
	}
	strcpy(token_arr[tokens].token, "_BILLDATE_");
	token_arr[tokens].value = strdup(billdate_start);
	tokens ++;

        /***********************************************************
	 * _INVOICENUMBER_
         ***********************************************************/
        /***********************************************************
	 * _ACCTNUM_
         ***********************************************************/
        /***********************************************************
	 * _DUEDATE_
         ***********************************************************/
        /***********************************************************
	 * _CORPADDR_
         ***********************************************************/
	sprintf(tmp_str, "%s<br>\n%s<br>\n%s<br>\n%s<br>\n%s<br>\n",
		CORPADDR1, CORPADDR2, CORPADDR3, CORPADDR4, CORPADDR5);

	token_arr[tokens].value = strdup(tmp_str);

	strcpy(token_arr[tokens].token, "_CORPADDR_");
	tokens ++;
	
        /***********************************************************
	 * _CUSTADDR_
         ***********************************************************/

        vp = PIN_FLIST_FLD_GET(tmp_nameinfo_flistp, PIN_FLD_SALUTATION, 
		0, ebufp);
	if (vp != NULL) {
		sprintf(tmp_str, "%s ", (char *)vp);
	}
        vp = PIN_FLIST_FLD_GET(tmp_nameinfo_flistp, PIN_FLD_NAME, 1, ebufp);
	if (vp) { /* use the name field from /payinfo/invoice */
		sprintf(tmp_str, "%s%s<br>\n", tmp_str, (char *)vp);
	} else { /* construct the name from /account NAMEINFO array */
		vp = PIN_FLIST_FLD_GET(tmp_nameinfo_flistp, 
			PIN_FLD_FIRST_NAME, 0, ebufp);
		if (vp != NULL) {
			sprintf(tmp_str, "%s%s", tmp_str, (char *)vp);
		}
		vp = PIN_FLIST_FLD_GET(tmp_nameinfo_flistp, 
			PIN_FLD_MIDDLE_NAME, 0, ebufp);
		if (vp != NULL) {
			sprintf(tmp_str, "%s %s", tmp_str, (char *)vp);
		}
		vp = PIN_FLIST_FLD_GET(tmp_nameinfo_flistp, 
			PIN_FLD_LAST_NAME, 0, ebufp);
		if (vp != NULL) {
			sprintf(tmp_str, "%s %s<br>\n", tmp_str, (char *)vp);
		}
	}
	addr =(char *)PIN_FLIST_FLD_GET(tmp_nameinfo_flistp, PIN_FLD_ADDRESS,
			1, ebufp);
	/* Since the address can have multiple lines, we need to add <br> */
	/* at the end of each line */
	if (addr != NULL) {
		addrline = strtok(addr, "\n");
		if (addrline != NULL) {
			sprintf(tmp_str, "%s%s<br>\n", tmp_str, addrline);
		}	
		while ((addrline = strtok(NULL, "\n")) != NULL) {
			sprintf(tmp_str, "%s%s<br>\n", tmp_str, addrline);
		}
	}
        vp = PIN_FLIST_FLD_GET(tmp_nameinfo_flistp, PIN_FLD_CITY, 1,
		 ebufp);
	if (vp != NULL) {
		sprintf(tmp_str, "%s%s", tmp_str, (char *)vp);
	}	
        vp = PIN_FLIST_FLD_GET(tmp_nameinfo_flistp, PIN_FLD_STATE, 0,
		 ebufp);
	if (vp != NULL) {
		sprintf(tmp_str, "%s, %s", tmp_str, (char *)vp);
	}
        vp = PIN_FLIST_FLD_GET(tmp_nameinfo_flistp, PIN_FLD_ZIP, 0,
		 ebufp);
	if (vp != NULL) {
		sprintf(tmp_str, "%s %s<br>\n", tmp_str, (char *)vp);
	}
        vp = PIN_FLIST_FLD_GET(tmp_nameinfo_flistp, PIN_FLD_COUNTRY, 0,
		 ebufp);
	if (vp != NULL) {
		sprintf(tmp_str, "%s%s<br>\n", tmp_str, (char *)vp);
	}	

	token_arr[tokens].value = strdup(tmp_str);

	strcpy(token_arr[tokens].token, "_CUSTADDR_");
	tokens ++;

        /***********************************************************
	 * _ACCOUNTSUMMARY_
         ***********************************************************/
	table_head_col2[0] = '\0';
	prev_total_col2[0] = '\0';
	subord_total_col2[0] = '\0';
	curr_total_col2[0] = '\0';
	total_due_col2[0] = '\0';
	if (currency2 != 0) { /* secondary currency */
		sprintf(table_head_col2, 
			"<th width=\"12%%\" align=\"right\" "
			"bgcolor=\"#cfcfcf\">%s Total &nbsp;</th>\n",
			symbol2);
		sprintf(prev_total_col2,
			"<td align=\"right\">%s&nbsp;</td>\n",
			prev_total2);
		sprintf(subord_total_col2,
			"<td align=\"right\">%s&nbsp;</td>\n",
			subord_total2);
		sprintf(curr_total_col2,
			"<td align=\"right\">%s&nbsp;</td>\n",
			curr_total2);
		sprintf(total_due_col2,
			"<td align=\"right\"><b>%s&nbsp;</b></td>\n",
			total_due2);
	}
	sprintf(table_head_col3, 
		"<th width=\"12%%\" align=\"right\" bgcolor=\"#cfcfcf\">"
		"%s Total &nbsp;</th>\n", symbol);
	sprintf(prev_total_col3,
		"<td align=\"right\">%s&nbsp;</td>\n", prev_total);
	sprintf(subord_total_col3,
		"<td align=\"right\">%s&nbsp;</td>\n", subord_total);
	sprintf(curr_total_col3,
		"<td align=\"right\">%s&nbsp;</td>\n", curr_total);
	sprintf(total_due_col3,
		"<td align=\"right\"><b>%s&nbsp;</b></td>\n", total_due);

	sprintf(table_heading_row, 
		"<tr valign=\"top\"  align=\"left\" bgcolor=\"#cfcfcf\">\n"
		"<th align=\"left\" bgcolor=\"#cfcfcf\">Balances</th>\n"
		"%s%s</tr>\n", table_head_col2, table_head_col3);
	sprintf(prev_total_row,
		"<tr valign=\"top\"><td align=\"left\">Previous Balance:"
		"</td>\n%s%s</tr>\n", prev_total_col2, prev_total_col3);
	sprintf(subord_total_row,
		"<tr valign=\"top\"><td align=\"left\">"
		"Subordinate Accounts:</td>\n%s%s</tr>\n",
		subord_total_col2, subord_total_col3);
	sprintf(curr_total_row,
		"<tr valign=\"top\"><td align=\"left\">Current Balance:"
		"</td>\n%s%s</tr>\n", curr_total_col2, curr_total_col3);
	if (total_due_col2[0] != '\0') {
		sprintf(hline, 
			"<td><hr size=\"1\"></td><td><hr size=\"1\"></td>");
	} else {
		sprintf(hline, "<td><hr size=\"1\"></td>");
	}
	sprintf(total_due_row,
		"<tr valign=\"top\" ><td></td>%s</tr>\n"
		"<tr valign=\"top\" ><td align=\"left\"><b>"
		"Total Balance Due:</b></td>\n%s%s</tr>\n",
		hline, total_due_col2, total_due_col3);
	sprintf(tmp_str, "<table width=\"100%%\" border=\"0\">\n"
		"%s%s%s%s%s</table>\n", table_heading_row,
		prev_total_row, subord_total_row, 
		curr_total_row, total_due_row);

	token_arr[tokens].value = strdup(tmp_str);
	strcpy(token_arr[tokens].token, "_ACCOUNTSUMMARY_");
	tokens ++;

	/***********************************************************
         * _TAXSUMMARY_
         ***********************************************************/
	sprintf(tax_header,
                "<table width=\"100%%\" border=\"0\">\n"
                "<tr valign=\"top\" align=\"center\" bgcolor=\"#cfcfcf\">\n"
                "<th width=\"30%%\" align=\"center\" bgcolor=\"#cfcfcf\">"
                "Tax Supplier ID.</th>\n"
		"<th width=\"30%%\" align=\"center\" bgcolor=\"#cfcfcf\">Tax Code</th>\n"
                "<th align=\"center\" bgcolor=\"#cfcfcf\">%s</th>\n", TAXAMT);
	
	taxbuf.strp = NULL;
        taxbuf.size = 0;
        taxbuf.strsize = 0;
        taxbuf.chunksize = 4096;
	
	fm_inv_pol_add_str_to_buf(&taxbuf, tax_header, ebufp);
	fm_inv_pol_format_invoice_html_get_tax_body(ctxp, in_flistp, &taxbuf, ebufp);
	sprintf(tax_footer, "<tr valign=\"top\" ><td></td><td></td><td></td>"
                "<td></td><td><hr size=\"1\"></td></tr></table>\n");
	fm_inv_pol_add_str_to_buf(&taxbuf, tax_footer, ebufp);
	token_arr[tokens].value = taxbuf.strp;
        strcpy(token_arr[tokens].token, "_TAXSUMMARY_");
        tokens ++;	
		

        /***********************************************************
	 * _ITEMS_
	 * _EVENTS_
         ***********************************************************/
	sprintf(item_header, 
		"<table width=\"100%%\" border=\"0\">\n"
		"<tr valign=\"top\" align=\"left\" bgcolor=\"#cfcfcf\">\n"
		"<th width=\"15%%\" align=\"left\" bgcolor=\"#cfcfcf\">"
		"Item No.</th>\n"
		"<th align=\"left\" bgcolor=\"#cfcfcf\">Description</th>\n"
		"<th width=\"12%%\" align=\"right\" bgcolor=\"#cfcfcf\">"
		"%sTotal &nbsp;</th></tr>\n", symbol);

	sprintf(event_header, 
		"<table width=\"100%%\" border=\"0\"><tr valign=\"top\" " 
		"align=\"left\" bgcolor=\"#cfcfcf\">\n"
		"<th width=\"15%%\" align=\"left\" bgcolor=\"#cfcfcf\">"
		"Date</th>\n<th align=\"left\" bgcolor=\"#cfcfcf\">"
		"Description</th>\n<th width=\"30%%\" align=\"left\" "
		"bgcolor=\"#cfcfcf\">Rate Description &nbsp;</th>\n"
		"<th width=\"12%%\" align=\"right\" bgcolor=\"#cfcfcf\">"
		"%s Total &nbsp;</th>\n</tr>\n", symbol);

        /***********************************************************
         * Loop through the items array and identify if we have a
         * parent item or a subordinate item.
         ***********************************************************/
	itembuf.strp = eventbuf.strp =NULL;
	itembuf.size = eventbuf.size = 0;
	itembuf.strsize = eventbuf.strsize = 0;
	itembuf.chunksize = eventbuf.chunksize = 4096;

	fm_inv_pol_add_str_to_buf(&itembuf, item_header, ebufp);
	fm_inv_pol_add_str_to_buf(&eventbuf, event_header, ebufp);

	fm_inv_pol_format_invoice_html_get_detail_body(in_flistp, 
		PIN_FLD_AR_ITEMS, currency, locale, &itembuf, 
		&eventbuf, ebufp);

	fm_inv_pol_format_invoice_html_get_detail_body(in_flistp, 
		PIN_FLD_SUB_ITEMS, currency, locale, &itembuf, 
		&eventbuf, ebufp);

	fm_inv_pol_format_invoice_html_get_detail_body(in_flistp, 
		PIN_FLD_OTHER_ITEMS, currency, locale, &itembuf, 
		&eventbuf, ebufp);

	sprintf(item_footer, "</table>\n");
	sprintf(event_footer, "<tr valign=\"top\" ><td></td><td></td><td></td>"
		"<td></td><td><hr size=\"1\"></td></tr></table>\n");

	fm_inv_pol_add_str_to_buf(&itembuf, item_footer, ebufp);
	fm_inv_pol_add_str_to_buf(&eventbuf, event_footer, ebufp);

	token_arr[tokens].value = itembuf.strp;
	strcpy(token_arr[tokens].token, "_ITEMS_");
	tokens ++;

	token_arr[tokens].value = eventbuf.strp;
	strcpy(token_arr[tokens].token, "_EVENTS_");
	tokens ++;

        /***********************************************************
	 * _AMOUNTDUE_
         ***********************************************************/

	vp = PIN_FLIST_FLD_GET(tmp_billinfo_flistp, PIN_FLD_DUE_T, 1, ebufp);

	if (vp == NULL) {
		duedate[0] = '\0';
	} else {
		pin_strftimet(duedate, sizeof(duedate), "%b %d %Y", 
			*(time_t *)vp); 
	}

        vp = PIN_FLIST_FLD_GET(tmp_billinfo_flistp, PIN_FLD_END_T, 
		0, ebufp);
	if (vp) {
		pin_strftimet(billdate_end, sizeof(billdate_end), 
			"%b %d %Y", *(time_t *)vp);
	}
	acctno = (char *)PIN_FLIST_FLD_GET(tmp_acctinfo_flistp, 
		PIN_FLD_ACCOUNT_NO, 0, ebufp);
	if (currency2 != 0) {
		sprintf(tmp_str, 
			"<table width=\"100%%\" border=\"0\" "
			"cellpadding=\"0\"><tr valign=\"top\" >"
			"<td align=\"left\">\n"
			"<table width=\"100%%\" border=\"1\" "
			"cellspacing=\"0\"><tr valign=\"top\"  "
			"align=\"center\">\n"
			"<td bgcolor=\"#cfcfcf\" width=\"25%%\">"
			"<font size=\"2\">&nbsp;Bill Date&nbsp;"
			"</font></td>"
			"<td bgcolor=\"#cfcfcf\" width=\"25%%\">"
			"<font size=\"2\">&nbsp;Invoice Number"
			"&nbsp;</font></td>"
			"<td bgcolor=\"#cfcfcf\" width=\"25%%\">"
			"<font size=\"2\">&nbsp;Account Number"
			"&nbsp;</font></td>"
			"<td bgcolor=\"#cfcfcf\"width=\"25%%\">"
			"<font size=\"2\">&nbsp;Payment Due"
			"&nbsp;</font></td>"
			"<tr valign=\"top\"  align=\"center\">"
			"</font></td><td align=\"center\">"
			"<font size=\"2\">%s"
			"</font></td><td align=\"center\">"
			"<font size=\"2\">%s"
			"</font></td><td align=\"center\">"
			"<font size=\"2\">%s"
			"</font></td><td align=\"center\">"
			"<font size=\"2\">%s"
			"<br>&nbsp;\n"
			"<tr><td colspan=\"4\"><br></td></tr>\n"
			"<tr valign=\"top\"  align=\"center\">\n"
			"<td bgcolor=\"#cfcfcf\" width=\"25%%\">"
			"<font size=\"2\">&nbsp;Exchange Rate&nbsp;"
			"</font></td>\n"
			"<td bgcolor=\"#cfcfcf\"width=\"25%%\">"
			"<font size=\"2\">&nbsp;Amount Due&nbsp;"
			"</font></td>\n"
			"<td bgcolor=\"#cfcfcf\"width=\"25%%\">"
			"<font size=\"2\">&nbsp;Amount Due&nbsp;"
			"</font></td>\n"
			"<td bgcolor=\"#cfcfcf\"width=\"25%%\">"
			"<font size=\"2\">&nbsp;Amount Enclosed&nbsp;"
			"</font></td></tr>\n"
			"<tr valign=\"top\" ><td align=\"center\">"
			"<font size=\"2\">%s"
			"</font></td><td align=\"center\">"
			"<font size=\"2\">%s %s"
			"</font></td><td align=\"center\">"
			"<font size=\"2\">%s %s" 
			"</font></td><td align=\"left\">"
			"<font size=\"2\">[&nbsp;&nbsp;&nbsp;]&nbsp;"
			"%s<br>[&nbsp;&nbsp;&nbsp;]&nbsp;"
			"%s</font></td></tr>\n</table>\n"
			"</td></tr></table>",
			billdate_end, billno, acctno, 
			duedate, exchange_rate,
			symbol2, total_due2,
			symbol, total_due, symbol, symbol2);
	} else {
		sprintf(tmp_str, 
			"<table width=\"100%%\" border=\"0\" "
			"cellpadding=\"0\"><tr valign=\"top\">"
			"<td align=\"left\">\n"
			"<table width=\"100%%\" border=\"1\" "
			"cellspacing=\"0\"><tr valign=\"top\" "
			"align=\"center\">\n"
			"<td bgcolor=\"#cfcfcf\" width=\"16%%\">"
			"<font size=\"2\">&nbsp;Bill Date&nbsp;"
			"</font></td>\n"
			"<td bgcolor=\"#cfcfcf\"width=\"16%%\">"
			"<font size=\"2\">&nbsp;Invoice Number&nbsp;"
			"</font></td>\n"
			"<td bgcolor=\"#cfcfcf\"width=\"16%%\">"
			"<font size=\"2\">&nbsp;Account Number&nbsp;"
			"</font></td>\n"
			"<td bgcolor=\"#cfcfcf\"width=\"16%%\">"
			"<font size=\"2\">&nbsp;Payment Due&nbsp;"
			"</font></td>\n"
			"<td bgcolor=\"#cfcfcf\"width=\"16%%\">"
			"<font size=\"2\">&nbsp;Amount Due&nbsp;"
			"</font></td>\n"
			"<td bgcolor=\"#cfcfcf\"><font size=\"2\">"
			"&nbsp;Amount Enclosed&nbsp;</font></td></tr>"
			"<tr valign=\"top\" ><td align=\"center\">"
			"<font size=\"2\">%s"
			"</font></td><td align=\"center\">"
			"<font size=\"2\">%s"
			"</font></td><td align=\"center\">"
			"<font size=\"2\">%s"
			"</font></td><td align=\"center\">"
			"<font size=\"2\">%s" 
			"</font></td><td align=\"center\">"
			"<font size=\"2\">%s" 
			"&nbsp;</font></td><td align=\"left\">"
			"<font size=\"2\">%s</font>\n"
			"</td></tr></table>\n</td></tr></table>\n",
			billdate_end, billno, acctno, duedate,
			total_due, symbol);
	}
	token_arr[tokens].value = strdup(tmp_str);
	strcpy(token_arr[tokens].token, "_AMOUNTDUE_");
	tokens ++;

        /***********************************************************
         * Now ready to fill in the template
         ***********************************************************/

	fm_inv_pol_format_invoice_html_fill_template(dbufp, templatep->data, 
		templatep->size, token_arr, tokens, ebufp);

        /***********************************************************
         * Error?
         ***********************************************************/
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                         "fm_inv_pol_format_invoice_html_prep_fill_template: error",
                         ebufp);
		goto ErrOut;
        }
	
ErrOut:
	PIN_FLIST_DESTROY_EX(&template_flistp, NULL);
	pin_free(locale);
	pin_free(prev_total);
	pin_free(curr_total);
	pin_free(subord_total);
	pin_free(total_due);
	if (currency2 != 0) {
		pin_free(prev_total2);
		pin_free(curr_total2);
		pin_free(subord_total2);
		pin_free(total_due2);
	}
	for (i=0; i<tokens; i++)
		pin_free(token_arr[i].value);
}

static void
fm_inv_pol_format_invoice_html_get_detail_body(
	pin_flist_t		*in_flistp,
	int32			fld_no,
	int32			currency,
	char			*locale,
	fm_inv_pol_str_buf_t	*itembufp,
	fm_inv_pol_str_buf_t	*eventbufp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t		*service_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
        int32                     elemid = 0;
        pin_cookie_t            cookie = NULL;

	void			*vp = NULL;
	char			line_itemp[BUFSIZ];
	char			subtitle[80];
	char			service_title[256];
	poid_t			*service_pdp = NULL;
	int32			service_enabled = 0;
	int32			orig_len = 0;
        int32                   dev_elemid = 0;
        pin_cookie_t            dev_cookie = NULL;
        int32                   ser_elemid = 0;
        pin_cookie_t            ser_cookie = NULL;
        int32                   i_elemid = 0;
        pin_cookie_t            i_cookie = NULL;
	pin_flist_t		*device_flistp = NULL;
	pin_flist_t		*ser_flistp = NULL;
	pin_flist_t		*i_flistp = NULL;
	pin_flist_t		*item_flistp = NULL;
	int32                   rec_id = 0;
	int32                   device_format = 1;

        /***********************************************************
	 * Has service centric invoicing been enabled?
	 * The FLAGS is set in FORMAT_VIEW pol opcode
         ***********************************************************/
	vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_FLAGS, 1, ebufp);
	if (vp != NULL) {
		service_enabled = *(int32 *)vp;
	}

        tmp_flistp = PIN_FLIST_ELEM_GET(in_flistp, fld_no,
		PIN_ELEMID_ANY, 1, ebufp);
	if (tmp_flistp != NULL) { /* old format items*/
		device_format = 0;
	}
	if (device_format ==1){
		item_flistp = PIN_FLIST_CREATE(ebufp);
		while ((device_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp,
			PIN_FLD_DEVICES, &dev_elemid, 1, &dev_cookie, ebufp))
			!= (pin_flist_t *)NULL) {
			
			ser_elemid = 0;
			ser_cookie = NULL;
			while ((ser_flistp = PIN_FLIST_ELEM_GET_NEXT(device_flistp,
				PIN_FLD_SERVICES, &ser_elemid, 1, &ser_cookie, ebufp))
				!= (pin_flist_t *)NULL) {

				i_elemid = 0;
				i_cookie = NULL;
				while ((i_flistp = PIN_FLIST_ELEM_GET_NEXT(ser_flistp,
					fld_no, &i_elemid, 1, &i_cookie, ebufp))
					!= (pin_flist_t *)NULL) {
					PIN_FLIST_ELEM_SET(item_flistp, i_flistp,
						fld_no, rec_id, ebufp);
					rec_id++;
				}
			}
		}
		tmp_flistp = PIN_FLIST_ELEM_GET(item_flistp, fld_no,
		PIN_ELEMID_ANY, 1, ebufp);
	}
        /***********************************************************
	 * Do we have the array with fld_no on the input flist? 
         ***********************************************************/
	if (tmp_flistp == NULL) { /* No items at all */
		PIN_FLIST_DESTROY_EX(&item_flistp, NULL);
		return; 
	}

        /***********************************************************
	 * Determine the subtitle
         ***********************************************************/
	switch (fld_no) {
	case PIN_FLD_AR_ITEMS:
		sprintf(subtitle, "Accounts/Receivable Items");
		break;
	case PIN_FLD_SUB_ITEMS:
		sprintf(subtitle, "Subordinate Items");
		break;
	case PIN_FLD_OTHER_ITEMS:
		sprintf(subtitle, "Other Items");
	}

        /***********************************************************
	 * Put the subtitle in the table 
         ***********************************************************/
	sprintf(line_itemp, "<tr valign=\"top\" align=\"left\">"
	"<td colspan=\"3\"><b>%s&nbsp;</b></td>"
	"</tr>\n", subtitle);
	fm_inv_pol_add_str_to_buf(itembufp, line_itemp, ebufp);

        /***********************************************************
	 * Has service centric invoicing been enabled?
	 * The FLAGS is set in FORMAT_VIEW pol opcode
         ***********************************************************/
	if (service_enabled) {
		while ((service_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp, 
				fld_no, &elemid, 1, 
				&cookie, ebufp)) != 
				(pin_flist_t *)NULL) {
			service_pdp = PIN_FLIST_FLD_GET(service_flistp, 
					PIN_FLD_SERVICE_OBJ,
					1,
					ebufp);
			if (service_pdp == NULL) {
				strcpy(service_title, "Service: Misc.");
			} else {
				vp = PIN_FLIST_FLD_GET(service_flistp, 
					PIN_FLD_LOGIN, 0, ebufp);
				sprintf(service_title, "Service: %s"
					" -- Login: %s", 
					PIN_POID_GET_TYPE(service_pdp), 
					(char *)vp);
			}

			/* Put the service subtitle in the table */
			pin_snprintf(line_itemp, sizeof(line_itemp), "<tr valign=\"top\" align=\"left\">"
			"<td colspan=\"3\"><i>%s&nbsp;</i></td>"
			"</tr>\n", service_title);

			fm_inv_pol_add_str_to_buf(itembufp, line_itemp, ebufp);

			fm_inv_pol_format_invoice_html_get_items_events_body(
					service_flistp, 
					PIN_FLD_ITEMS, 
					currency, 
					itembufp, 
					eventbufp, 
					ebufp);
		}
	} else {
		if (device_format == 1){
			fm_inv_pol_format_invoice_html_get_items_events_body(
					item_flistp, 
					fld_no, 
					currency, 
					itembufp, 
					eventbufp, 
					ebufp);
			PIN_FLIST_DESTROY_EX(&item_flistp, NULL);
		}
		else{
			fm_inv_pol_format_invoice_html_get_items_events_body(
					in_flistp,
					fld_no,
					currency,
					itembufp,
					eventbufp,
					ebufp);
		}
	}
}

static void
fm_inv_pol_format_invoice_html_get_tax_body(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        fm_inv_pol_str_buf_t    *taxbufp,
        pin_errbuf_t            *ebufp)
{
	int32                   elemid = 0;
        pin_cookie_t            cookie = NULL;
        int32                   elemid1 = 0;
        pin_cookie_t            cookie1 = NULL;
        pin_flist_t             *tax_flistp = NULL;
        pin_flist_t             *tax_tot_flistp = NULL;
        char                    *tax_amount = NULL;
        char                    *tax_code = NULL;
	pin_decimal_t           *pdp = NULL;
        pin_decimal_t           *pdp_rounded = NULL;
	char                    tax_body[BUFSIZ] = {""};
	int32			ts_id = 0;
	int32			len = 0;
	void			*vp = NULL;
	pin_flist_t		*ts_flistp = NULL;
	char			*ts_name = NULL;

	while ((tax_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp,
                                PIN_FLD_TAXES, &elemid, 1,
                                &cookie, ebufp)) != (pin_flist_t *)NULL) {

                vp = PIN_FLIST_FLD_GET(tax_flistp, 
				PIN_FLD_TAX_SUPPLIER, 0, ebufp);
		if (vp) {
			ts_id = *(int32*)vp;
		} else {
			ts_id = fm_bill_utils_cfg_get_default_ts_elemid(ctxp,ebufp);
		}

		/* Lookup the PIN_FLD_NAME of the tax supplier from
		 * the cache and print that, instead of ts_id.
		 */
		ts_flistp = fm_utils_get_tax_supplier_elem(ctxp, ts_id, ebufp);
		if (ts_flistp) {
			ts_name = PIN_FLIST_FLD_GET(ts_flistp, PIN_FLD_NAME,
				0, ebufp);
		} else {
			ts_name = "<Unknown>";
		}

		elemid1 = 0;
		cookie1 = NULL;
                while ((tax_tot_flistp = PIN_FLIST_ELEM_GET_NEXT(tax_flistp,
                                PIN_FLD_TOTAL, &elemid1, 1,
                                &cookie1, ebufp)) != (pin_flist_t *)NULL) {

                        tax_code = PIN_FLIST_FLD_GET(tax_tot_flistp,
				PIN_FLD_TAX_CODE, 0, ebufp);
                        pdp = (pin_decimal_t *)PIN_FLIST_FLD_GET(tax_tot_flistp,
				PIN_FLD_AMOUNT, 0, ebufp);
			if (!tax_code || !pdp) {
				continue;
				/*******/
			}
                        tax_amount = pbo_decimal_to_str(pdp, ebufp);
			
			sprintf(tax_body,
                        "<tr valign=\"top\" align=\"center\"><td>"
                        "<font size=\"3\">%s&nbsp;</font></td><td>%s&nbsp;"
                        "</td><td align=\"center\">%s&nbsp;</td></tr>\n",
                        ts_name, tax_code, tax_amount);
			pin_free(tax_amount);

			fm_inv_pol_add_str_to_buf(taxbufp, tax_body, ebufp);
		}
	}
	return;
}

static void
fm_inv_pol_format_invoice_html_get_items_events_body(
	pin_flist_t		*in_flistp,
	int32			fld_no,
	int32			currency,
	fm_inv_pol_str_buf_t	*itembufp,
	fm_inv_pol_str_buf_t	*eventbufp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t		*item_flistp = NULL;
	pin_flist_t		*event_flistp = NULL;
	pin_flist_t		*tmp_total_flistp = NULL;
        int32                     elemid = 0;
        pin_cookie_t            cookie = NULL;
        int32                     inner_elemid = 0;
        pin_cookie_t            inner_cookie = NULL;

	pin_decimal_t		*pdp;
	pin_decimal_t		*pdp_rounded;
	void			*vp = NULL;
	char			*item_desc = NULL;
	char			*item_no = NULL;
	char			*item_total = NULL;
	char			item_body[BUFSIZ];
	char			event_body[BUFSIZ];
	char                    event_date[40] = {""};
	char			*sys_desc;
	char			rate_tag[80] = {""};
	char			*event_amount;
	int32			inner_array_id = 0;

        while ((item_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp, 
		fld_no, &elemid, 1, &cookie, ebufp)) != 
		(pin_flist_t *)NULL) {

		item_desc = (char *)PIN_FLIST_FLD_GET(item_flistp, 
			PIN_FLD_NAME, 0, ebufp);

		item_no = (char *)PIN_FLIST_FLD_GET(item_flistp, 
			PIN_FLD_ITEM_NO, 0, ebufp);

		pdp = (pin_decimal_t *)PIN_FLIST_FLD_GET(item_flistp, 
			PIN_FLD_ITEM_TOTAL, 0, ebufp);
		pdp_rounded = fm_utils_round_balance(pdp, currency);
		item_total = pbo_decimal_to_str(pdp_rounded, ebufp);
		pin_decimal_destroy( pdp_rounded );

		sprintf(item_body, 
			"<tr valign=\"top\" align=\"left\"><td>"
			"<font size=\"3\">%s&nbsp;</font></td><td>%s&nbsp;"
			"</td><td align=\"right\">%s&nbsp;</td></tr>\n",
			item_no, item_desc, item_total);
		pin_free(item_total);

		fm_inv_pol_add_str_to_buf(itembufp, item_body, ebufp);

                inner_elemid = 0;
                inner_cookie = NULL;
                while ((event_flistp = PIN_FLIST_ELEM_GET_NEXT(item_flistp,
                        PIN_FLD_EVENTS, &inner_elemid, 1, &inner_cookie,
                        ebufp)) != (pin_flist_t *)NULL) {
			
			/* There are two timezone related fields on the
			 * event flist, which can be used to show the
			 * correct time.  
			 * PIN_FLD_RATED_TIMEZONE_ID: The timezone ID (string)
			 * PIN_FLD_TIMEZONE_ADJ_END_T: The adjusted end time
			 *
			 * To use PIN_FLD_TIMEZONE_ADJ_END_T, one may not
			 * use pin_strftimet() to convert it to a time string
			 * because the localtime() is called internally.
			 * The right way to handle this is to call
			 * gmtime_r() to get the tm structure from the time
			 * stamp then call strftime() to convert it to 
			 * a string.
			 * 
			 * Users can feel free to replace the following 
			 * code piece with code to deal with the timezone 
			 * adjusted time, as long as they need to show 
			 * exact time (not just date).
			 */

			vp = PIN_FLIST_FLD_GET(event_flistp, PIN_FLD_END_T, 
				0, ebufp);
			if (vp) {
				pin_strftimet(event_date, sizeof(event_date), 
					"%b %d %Y", *(time_t *)vp);
			}
			sys_desc = (char *)PIN_FLIST_FLD_GET(event_flistp, 
				PIN_FLD_SYS_DESCR, 0, ebufp);

			vp = (char *)PIN_FLIST_FLD_GET(
				event_flistp, PIN_FLD_LABEL, 1, ebufp);
			if (vp == NULL) { /* Set default locale */
				strcpy(rate_tag, "N/A");
			} else {
				strcpy(rate_tag, (char *)vp);
			}

			tmp_total_flistp = PIN_FLIST_ELEM_GET(event_flistp,
				PIN_FLD_TOTAL, currency, 1, ebufp);
			if (tmp_total_flistp!=NULL)
			{
				pdp = (pin_decimal_t *)PIN_FLIST_FLD_GET(
					tmp_total_flistp, PIN_FLD_AMOUNT, 
					0, ebufp);
				pdp_rounded = fm_utils_round_balance(pdp, currency);
				event_amount = pbo_decimal_to_str(pdp_rounded, ebufp);
				pin_decimal_destroy( pdp_rounded );

				sprintf(event_body, 
					"<tr valign=\"top\"  align=\"left\"><td>"
					"<font size=\"3\">%s&nbsp;</font></td><td>"
					"%s&nbsp;</td><td>%s&nbsp;</td>"
					"<td align=\"right\">%s&nbsp;</td></tr>\n",
					event_date, sys_desc, rate_tag, event_amount);
				pin_free(event_amount);

				fm_inv_pol_add_str_to_buf(eventbufp, event_body, ebufp);
			}
		}
	}
}

static void
fm_inv_pol_format_invoice_html_fill_template(
	fm_inv_pol_str_buf_t	*bufp,
        char                    *template,
        int32                 template_size,
        fm_inv_pol_html_token_t	*tokenp,
	int			tokens,
        pin_errbuf_t            *ebufp)
{
	int 			i = 0;
	int			match = 0;
        char                    *cp = NULL;
        char                    *op = NULL;
	int32			cp_size = 0;
	int32			tokensize = 0;

        if( PIN_ERR_IS_ERR( ebufp )) {
                return;
        }

        if( bufp == NULL || template == NULL ) {
                return;
        }

        /* in case they forget to set the chunk size */

        if( bufp->chunksize <= 0 ) {
                /* safeguard */
                bufp->chunksize = 512;
        }

        /*********************************************************
         * If strp is still null, need to allocate first block
         *********************************************************/

        if( bufp->strp == NULL ) {
                bufp->strp = (char *)pin_malloc( bufp->chunksize );
                if (bufp->strp == NULL) {
                        pin_set_err(ebufp, PIN_ERRLOC_PCM,
                                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                                PIN_ERR_NO_MEM, 0, 0, 0);
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "Memory for buffer couldn't be allocated",
                                ebufp);
                        goto ErrOut;
                }

                memset( bufp->strp, 0, bufp->chunksize);
                bufp->size = bufp->chunksize;
                bufp->strsize = 0;
        }

        /***********************************************************
         * Now start copying the characters one at a time from the
         * input string to the bufp->strp buffer.  If we run out
         * of room in the bufp->strp, then realloc some more mem
         * and continue.  Keep track of size and strsize as we go.
         ***********************************************************/

        cp = template;

        /* calc starting point of output buffer */
        op = (char *)(bufp->strp + bufp->strsize );

        /* copy everything over */
        while( cp_size < template_size ) {

                /* if the string size has exceeded limit
                 * (use fudge factor of 50)
                 * Then realloc more memory
                 */
                if( bufp->strsize > bufp->size - 50 ) {

                        bufp->size = bufp->size + bufp->chunksize;
                        bufp->strp = (char *)pin_realloc( bufp->strp,
                                                          bufp->size );
                        if (bufp->strp == NULL) {
                                pin_set_err(ebufp, PIN_ERRLOC_PCM,
                                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                                        PIN_ERR_NO_MEM, 0, 0, 0);
                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "Memory for buffer couldn't be allocated",
                                                 ebufp);
                                goto ErrOut;
                        }
			op = (char *)(bufp->strp + bufp->strsize );
                }

		/***********************************************************
		 * Search for the token in template and replace them
		 * with desired strings.
         	 ***********************************************************/
		match = 0;
		for (i = 0; i < tokens; i++) {
			tokensize = strlen(tokenp[i].token);
			if ((strncmp(cp, tokenp[i].token, 
				tokensize)) == 0) {
				match = 1;
				break;
			}
		}
                /* now copy the character  and also null terminator
                 * increment the local char pointers and also the
                 * strsize
                 */
		if (match) {
			fm_inv_pol_add_str_to_buf(
				bufp, tokenp[i].value, ebufp);	
			op = (char *)(bufp->strp + bufp->strsize );
			cp += tokensize;
			cp_size += tokensize;
		} else {
			*op = *cp;
			cp++;
			cp_size++;
			op++;
			*op = '\0';
			bufp->strsize++;
		}
        }

        /* string should be copie now */
        return;
ErrOut:
        return;
}
