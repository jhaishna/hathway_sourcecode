 /*******************************************************************
 *
 * Copyright (c) 1999, 2009, Oracle	and/or its affiliates. 
 * All rights reserved.	
 *
 *		This material is the confidential property of Oracle Corporation
 *		or its licensors and may be	used, reproduced, stored or	transmitted
 *		only in	accordance with	a valid	Oracle license or sublicense agreement.
 *
 *******************************************************************/

 /***************************************************************************************************************
*VERSION |	 DATE	 |	  Author		|				DESCRIPTION											   *
*--------------------------------------------------------------------------------------------------------------*
* 0.1	 |29/07/2014	 |Someshwar			|  BB Welcome Note Implementation
*
*****************************************************************************************************************/

#ifndef	lint
static	char Sccs_Id[] = "@(#)$Id: fm_mso_ntf_create_welcome_note.c	/cgbubrm_7.3.2.rwsmod/1	2009/08/04 23:37:19	pjegan Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/timeb.h>
#include "pin_sys.h"
#include "pin_os.h"
#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "mso_ntf.h"
#include "mso_prov.h"
#include "cm_cache.h"

#define HFIBER_BUSINESS_TYPE 97681200

/*******************************************************************
 * external variable for bb provisioning config pointer
 *******************************************************************/
extern cm_cache_t *mso_prov_bb_config_ptr;

extern void	
fm_mso_utils_sequence_no(
	pcm_context_t			*ctxp,
	pin_flist_t			*i_flistp,
	pin_flist_t			**r_flistpp,
	pin_errbuf_t			*ebufp);

EXPORT_OP void
op_mso_ntf_create_welcome_note(
	cm_nap_connection_t		*connp,
	int32				opcode,
	int32				flags,
	pin_flist_t			*in_flistp,
	pin_flist_t			**ret_flistpp,
	pin_errbuf_t			*ebufp);

static void
fm_mso_ntf_create_email_welcome_note(
	pcm_context_t			*ctxp,
	pin_flist_t			*in_flistp,
	int32				flags,
	pin_flist_t			**out_flistpp,
	pin_errbuf_t			*ebufp);

static void
fm_mso_ntf_create_sms_welcome_note(
	pcm_context_t			*ctxp,
	pin_flist_t			*in_flistp,
	int32				flags,
	pin_flist_t			**out_flistpp,
	pin_errbuf_t			*ebufp);

static void
prepare_email_welcome_note_notification_buf(
	pcm_context_t			*ctxp,
	pin_flist_t			*i_flistp,
	pin_flist_t			**ret_flistpp,
	int				**technology,
	pin_errbuf_t			*ebufp);

static void
get_bb_email_template(
	pcm_context_t			*ctxp, 
	pin_flist_t			*i_flistp, 
	char				**email_template,
	int				*flag,
	pin_errbuf_t			*ebufp);

static void
mso_get_prov_tag(
	pcm_context_t			*ctxp, 
	poid_t				*srv_pdp, 
	char				**prov_name,
	char				**pkg_name,
	poid_t				**prod_pdp,
	pin_errbuf_t			*ebufp);

static void
mso_get_plan_amt(
	pcm_context_t			*ctxp, 
	char				*pkg_name,
	poid_t				*srv_pdp,
	char				*mso_city,  
	int				pay_term,
	pin_decimal_t			**amount, 
	pin_decimal_t			**speed, 
	pin_decimal_t			**fup_speed,
        pin_decimal_t                   **quota,
	pin_errbuf_t			*ebufp);

static void
mso_get_hw_plan(
	pcm_context_t			*ctxp, 
	poid_t				*srv_pdp, 
	char				**tech_s, 
	pin_errbuf_t			*ebufp);

static void
prepare_sms_welcome_note_notification_buf(
	pcm_context_t			*ctxp,
	pin_flist_t			*i_flistp,
	pin_flist_t			**ret_flistpp,
	pin_errbuf_t			*ebufp);

static void
get_bb_sms_template(
	pcm_context_t			*ctxp, 
	pin_flist_t			*i_flistp, 
	char				**email_template,
	int				*flag,
	pin_errbuf_t			*ebufp);

PIN_EXPORT void
get_helpdesk_details(
        pcm_context_t                   *ctxp,
        pin_flist_t                     *i_flistp,
	char                     	*city,
	pin_flist_t                     **ret_flistpp,
        pin_errbuf_t                    *ebufp);

static void
mso_get_plan_details(
	pcm_context_t			*ctxp, 
	char				*prov_name, 
	int				*tech, 
	pin_decimal_t			**quota, 
	pin_decimal_t			**speed, 
	pin_decimal_t			**fup_speed, 
	pin_errbuf_t			*ebufp);

static void
mso_get_addidional_address(
        pcm_context_t                   *ctxp,
        pin_flist_t                     *i_flistp,
        char                            *address,
        char                            *install_address,
        pin_errbuf_t                    *ebufp);

static void
fm_mso_get_purchase_prod_details(
        pcm_context_t    *ctxp,
        poid_t           *act_pdp,
        pin_flist_t      **rt_flistp,
        pin_errbuf_t     *ebufp);


/*******************************************************************
 * Main	routine	for	the	MSO_OP_NTF_CREATE_WELCOME_NOTE command
 *******************************************************************/

void
op_mso_ntf_create_welcome_note(
	cm_nap_connection_t		*connp,
	int32				opcode,
	int32				flags,
	pin_flist_t			*in_flistp,
	pin_flist_t			**ret_flistpp,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*r_flistp = NULL;
	pin_flist_t			*r_sms_flistp =	NULL;
	pcm_context_t			*ctxp =	connp->dm_ctx;


	/***********************************************************
	 * Null	out	results	until we have some.
	 ***********************************************************/
	*ret_flistpp = NULL;
	PIN_ERRBUF_CLEAR(ebufp);

	/***********************************************************
	 * Insanity	check.
	 ***********************************************************/
	if (opcode != MSO_OP_NTF_CREATE_WELCOME_NOTE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_BAD_OPCODE,	0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode	in op_mso_ntf_create_welcome_note",	ebufp);
		return;
	}

	/***********************************************************
	 * Log input flist
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_mso_ntf_create_welcome_note	input flist", in_flistp);

	/***********************************************************
	 * Call	main function to do	it
	 ***********************************************************/
	fm_mso_ntf_create_email_welcome_note(ctxp, in_flistp, flags, &r_flistp,	ebufp);
	fm_mso_ntf_create_sms_welcome_note(ctxp, in_flistp,	flags, &r_sms_flistp, ebufp);

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*ret_flistpp = (pin_flist_t	*)NULL;
		PIN_FLIST_DESTROY_EX(&r_flistp,	NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_ntf_create_welcome_note	error",	ebufp);
		PIN_ERRBUF_CLEAR(ebufp);
	} 
	else {
		*ret_flistpp = r_flistp;
		PIN_ERRBUF_CLEAR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_mso_ntf_create_welcome_note	return flist", r_flistp);
	}

	return;
}

/*******************************************************************
 * fm_mso_ntf_create_email_welcome_note()
 *
 * Perfomrs	following action:
 * 1. Get the email	template.
 * 2. Populate the date
 * 3. get notification sequence	
 * 4. create notification order	/mso_ntf_welcome
 *******************************************************************/
static void
fm_mso_ntf_create_email_welcome_note(
	pcm_context_t			*ctxp,
	pin_flist_t			*in_flistp,
	int32				flags,
	pin_flist_t			**out_flistpp,
	pin_errbuf_t			*ebufp)
{
	int32				*action_flag = NULL;
	int64				db_no;
	poid_t				*service_poidp = NULL;
	poid_t				*acc_poidp = NULL;
	poid_t				*objp =	NULL;
	pin_flist_t			*payload_flistp	= NULL;
	pin_flist_t			*order_i_flistp		= NULL;
	pin_flist_t			*tmp_flistp= NULL;
	pin_flist_t			*order_seq_i_flistp	= NULL;
	pin_flist_t			*order_seq_o_flistp	= NULL;
	pin_buf_t			*nota_buf	  =	NULL;

	char				*flistbuf =	NULL;
	int				flistlen=0;
	int				status = NTF_STATE_NEW;
	char				*act = NULL;
	char				*email_addr	= NULL;
	char				*email_template	= NULL;
	int				*tech =	NULL;
	int32				priority;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);

	/***********************************************************
	 * Log input flist
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_ntf_create_welcome_note	input flist", in_flistp);

	/***********************************************************
	 * Check mandatory fields are available
	 ***********************************************************/
	email_addr = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_EMAIL_ADDR, 0, ebufp);
	//action_flag =	PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_FLAGS,	0, ebufp);


	acc_poidp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_ntf_create_email_notification error: required field	missing	in input flist", ebufp);
		return;
	}

	/***********************************************************
	 * Enrich With EMAIL Text
	 ***********************************************************/
	payload_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_EMAIL_ADDR, email_addr, ebufp);
	PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_FROM_NAME, "alert@hathway.net", ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
	prepare_email_welcome_note_notification_buf(ctxp, in_flistp, &payload_flistp, &tech, ebufp);
	priority = 1;
	/*
	 * Return if validation	and	enrichment returns error
	 */
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_FLIST_DESTROY_EX(&payload_flistp, NULL);
		return;
	}

	/*
	 * Get Order Id	and	set	it in payload
	 */
	order_seq_i_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(order_seq_i_flistp, PIN_FLD_POID,	acc_poidp, ebufp);
	PIN_FLIST_FLD_SET(order_seq_i_flistp, PIN_FLD_NAME,	"MSO_NTF_SEQUENCE",	ebufp);
	//call the function	to read	the	provisioning order sequence
	fm_mso_utils_sequence_no(ctxp, order_seq_i_flistp, &order_seq_o_flistp,	ebufp);


	/*
	 * Convert flist to	String Payload.	it will	be used	by Adaptor process to send EMAIL
	 */
	PIN_FLIST_TO_XML( payload_flistp, PIN_XML_BY_SHORT_NAME, 0,
			&flistbuf, &flistlen, "Email", ebufp );
//	  PIN_FLIST_TO_STR(payload_flistp,&flistbuf,&flistlen,ebufp	);


	/*
	 * Create flist	for	order creation
	 */
	order_i_flistp = PIN_FLIST_CREATE(ebufp);
	db_no =	PIN_POID_GET_DB(acc_poidp);	
	objp = PIN_POID_CREATE(db_no, "/mso_ntf_welcome", -1, ebufp);
	PIN_FLIST_FLD_SET(order_i_flistp, PIN_FLD_POID,	objp, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID,	order_i_flistp,	PIN_FLD_ACCOUNT_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SERVICE_OBJ, order_i_flistp, PIN_FLD_SERVICE_OBJ,	ebufp);

	PIN_FLIST_FLD_COPY(order_seq_o_flistp, PIN_FLD_STRING, order_i_flistp, MSO_FLD_NTF_ID, ebufp);
	PIN_FLIST_FLD_SET(order_i_flistp, PIN_FLD_STATUS, &status, ebufp); //NEW
	PIN_FLIST_FLD_SET(order_i_flistp, PIN_FLD_RANK, &priority, ebufp); 
	PIN_FLIST_FLD_SET(order_i_flistp, MSO_FLD_TECHNOLOGY, (void *)tech,	ebufp);	

	nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
	if ( nota_buf == NULL )	
	{
		pin_set_err( ebufp,	PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE,	PIN_ERR_NO_MEM,	0, 0 ,0	);
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf",	ebufp ); 
		return;
	}	

	nota_buf->flag = 0;
	nota_buf->size = flistlen - 1;
	nota_buf->offset = 0;
	nota_buf->data = (caddr_t)flistbuf;
	nota_buf->xbuf_file = (char *)NULL;

	PIN_FLIST_FLD_PUT( order_i_flistp, PIN_FLD_INPUT_FLIST,	( void *) nota_buf,	ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_ntf_create_welcome_note create input flist", order_i_flistp);
	PCM_OP(ctxp, PCM_OP_CREATE_OBJ,	0, order_i_flistp, out_flistpp,	ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_ntf_create_welcome_note order create output flist", *out_flistpp);

	PIN_FLIST_FLD_COPY(order_i_flistp, MSO_FLD_NTF_ID, *out_flistpp, MSO_FLD_NTF_ID, ebufp);

Cleanup:
	PIN_FLIST_DESTROY_EX(&payload_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&order_seq_i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&order_seq_o_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&order_i_flistp, NULL);
	
	return;
}

/*******************************************************************
 * fm_mso_ntf_create_sms_welcome_note()
 *
 * Perfomrs	following action:
 * 1. Get the email	template.
 * 2. Populate the date
 * 3. get notification sequence	
 * 4. create notification order	/mso_ntf_sms
 *******************************************************************/
static void
fm_mso_ntf_create_sms_welcome_note(
	pcm_context_t			*ctxp,
	pin_flist_t			*in_flistp,
	int32				flags,
	pin_flist_t			**out_flistpp,
	pin_errbuf_t			*ebufp)
{
	int32				*action_flag = NULL;
	int64				db_no;
	poid_t				*service_poidp = NULL;
	poid_t				*acc_poidp = NULL;
	poid_t				*objp =	NULL;
	pin_flist_t			*payload_flistp	= NULL;
	pin_flist_t			*order_i_flistp		= NULL;
	pin_flist_t			*tmp_flistp= NULL;
	pin_flist_t			*order_seq_i_flistp	= NULL;
	pin_flist_t			*order_seq_o_flistp	= NULL;
	pin_flist_t         *so_flistp = NULL;
    pin_flist_t         *si_flistp = NULL;
	pin_buf_t			*nota_buf	  =	NULL;

	char				*flistbuf =	NULL;
	int				flistlen=0;
	int				status = NTF_STATE_NEW;
	char				*act = NULL;
	char				*phone = NULL;
	char				*sms_template =	NULL;
	int32				priority;
	int32               business_type = 0;
	

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);

	/***********************************************************
	 * Log input flist
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_ntf_create_sms_welcome_note input flist", in_flistp);

	/***********************************************************
	 * Check mandatory fields are available
	 ***********************************************************/
	 phone = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PHONE, 0,	ebufp);
	//action_flag =	PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_FLAGS,	0, ebufp);

	acc_poidp =	PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_ntf_create_sms_notification error: required field missing in input flist", ebufp);
		return;
	}

	/***********************************************************
	 * Enrich With EMAIL Text
	 ***********************************************************/
	payload_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
	prepare_sms_welcome_note_notification_buf(ctxp,	in_flistp, &payload_flistp, ebufp);
	priority = 1;
	/*
	 * Return if validation	and	enrichment returns error
	 */
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_FLIST_DESTROY_EX(&payload_flistp, NULL);
		return;
	}
	
	/* To stop notification SMS to be send for H-Fiber customers - Biju.J */
	si_flistp = PIN_FLIST_CREATE(ebufp);

	PIN_FLIST_FLD_SET(si_flistp, PIN_FLD_POID, acc_poidp, ebufp);	


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "Read account type from /acccount input flist", si_flistp);
    PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "Read account type from /acccount output flist", so_flistp);

    if(so_flistp && so_flistp != NULL)
	{
		business_type = *(int *) PIN_FLIST_FLD_GET(so_flistp,PIN_FLD_BUSINESS_TYPE,0,ebufp);
		if ( business_type == HFIBER_BUSINESS_TYPE )
		return;

	}
/* To stop notification SMS to be send for H-Fiber customers - Biju.J */	
	
	/*
	 * Get Order Id	and	set	it in payload
	 */
	order_seq_i_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(order_seq_i_flistp, PIN_FLD_POID, acc_poidp, ebufp);
	PIN_FLIST_FLD_SET(order_seq_i_flistp, PIN_FLD_NAME, "MSO_NTF_SEQUENCE",	ebufp);
	//call the function	to read	the	provisioning order sequence
	fm_mso_utils_sequence_no(ctxp, order_seq_i_flistp, &order_seq_o_flistp,	ebufp);


	/*
	 * Convert flist to	String Payload.	it will	be used	by Adaptor process to send EMAIL
	 */
	PIN_FLIST_TO_XML( payload_flistp, PIN_XML_BY_SHORT_NAME, 0,
			&flistbuf, &flistlen, "SMS", ebufp );
//	  PIN_FLIST_TO_STR(payload_flistp,&flistbuf,&flistlen,ebufp	);


	/*
	 * Create flist	for	order creation
	 */
	order_i_flistp = PIN_FLIST_CREATE(ebufp);
	db_no =	PIN_POID_GET_DB(acc_poidp);	
	objp = PIN_POID_CREATE(db_no, "/mso_ntf_sms", -1, ebufp);
	PIN_FLIST_FLD_SET(order_i_flistp, PIN_FLD_POID,	objp, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID,	order_i_flistp,	PIN_FLD_ACCOUNT_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SERVICE_OBJ, order_i_flistp, PIN_FLD_SERVICE_OBJ,	ebufp);

	PIN_FLIST_FLD_COPY(order_seq_o_flistp, PIN_FLD_STRING, order_i_flistp, MSO_FLD_NTF_ID, ebufp);
	PIN_FLIST_FLD_SET(order_i_flistp, PIN_FLD_STATUS, &status, ebufp); //NEW
	PIN_FLIST_FLD_SET(order_i_flistp, PIN_FLD_RANK,	&priority, ebufp); 

	nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t	) );
	if ( nota_buf == NULL )	
	{
		pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE,	PIN_ERR_NO_MEM,	0, 0 ,0	);
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf",	ebufp ); 
		return;
	}	

	nota_buf->flag	 = 0;
	nota_buf->size	 = flistlen	- 1;
	nota_buf->offset = 0;
	nota_buf->data	 = ( caddr_t)flistbuf;
	nota_buf->xbuf_file	= (	char *)	NULL;

	PIN_FLIST_FLD_PUT( order_i_flistp, PIN_FLD_INPUT_FLIST,	( void *) nota_buf,	ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_ntf_create_sms_welcome_note	create input flist", order_i_flistp);
	PCM_OP(ctxp, PCM_OP_CREATE_OBJ,	0, order_i_flistp, out_flistpp,	ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_ntf_create_sms_welcome_note	order create output	flist",	*out_flistpp);

	PIN_FLIST_FLD_COPY(order_i_flistp, MSO_FLD_NTF_ID, *out_flistpp, MSO_FLD_NTF_ID, ebufp);

Cleanup:
	PIN_FLIST_DESTROY_EX(&payload_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&order_seq_i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&order_seq_o_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&order_i_flistp, NULL);
	
	return;
}

/*******************************************************************
 * prepare_email_welcome_note_notification_buf()
 *
 * Enrich input	filst with service details required
 * optionally performa validation on input fields
 *******************************************************************/
static void
prepare_email_welcome_note_notification_buf(
	pcm_context_t				*ctxp,
	pin_flist_t				*i_flistp,
	pin_flist_t				**ret_flistpp,
	int					**technology,
	pin_errbuf_t				*ebufp)
{
	pin_flist_t				*si_flistp = NULL;
	pin_flist_t				*so_flistp = NULL;
	pin_flist_t				*rfld_iflistp =	NULL;
	pin_flist_t				*rfld_oflistp =	NULL;
	pin_flist_t				*ai_flistp = NULL;
	pin_flist_t				*ao_flistp = NULL;
	pin_flist_t				*tmp_flistp	= NULL;
	pin_flist_t				*phone_flistp =	NULL;
	pin_flist_t				*alias_list	= NULL;
	pin_flist_t				*tmp2_flistp = NULL;
	pin_flist_t				*tmp3_flistp = NULL;
	poid_t					*srv_pdp = NULL;
	poid_t					*prod_pdp =	NULL;
	char					*ne	= NULL;
	char					*email_template	= NULL;
	char					*account_no	= NULL;
	char					*stb_id	= NULL;
	char					*vc_id = NULL;
	char					*accno = NULL;
	char					*phone = NULL;
	char					*city =	NULL;
	char					*zip = NULL;
	char					*fname = NULL;
	char					*lname = NULL;
	char					*cfname	= NULL;
	char					*clname	= NULL;
	char					*address = NULL;
	char					additional_addr [512]     = {""};
	char					*addnl_addr  = NULL;
	char					*welcome_note =	NULL;
	char					*tmp_str = NULL;
	char					 str[10240]	= {""};
	char					*cmts_name = NULL;
	char					*tech_s	= NULL;
	char					*login = NULL;
	char					*mac = NULL;
	char					*wifi_mac =	NULL;
	char					*passwd	= NULL;
	char					*user_id = NULL;
	char					*email = NULL;
	char					*prov_name = NULL;
	char					*pkg_name =	NULL;
	char					*mso_city =	NULL;
	int					*tech =	NULL;
	struct					tm *current_time;
	time_t					curr_time =	0;
	int					year = 0;
	int					month =	0;
	int					day	= 0;
	int					flag = 0;
	int					*pay_term =	0;
	char					curr_date[50] =	{""};
	int					*ph_type = 0;
	int					rec_id = 0;
	pin_decimal_t				*amount	= NULL;
	pin_decimal_t				*speed = NULL;
	pin_decimal_t				*quota = NULL;
	pin_decimal_t				*fup_speed = NULL;
	double					amount_d = 0;
	double					speed_d = 0;
	double					quota_d = 0;
	double					fup_speed_d = 0;
	pin_cookie_t				cookie = NULL;
        int                                     *conn_type = 0;
        time_t                                  *validity =  NULL;
	poid_t 					*acct_pdp = NULL;
        char                                    validity_date[50] = {""};
	pin_flist_t				*pur_prod_flistp=NULL;

        char                                    inst_addr [600]     = {""};
        char                                    *install_address  = NULL;
        char                                    *in_address = NULL;
        pin_flist_t                             *tmp1_flistp     = NULL;
        char                                    *in_city = NULL;
        char                                    *in_zip = NULL;
        char                            msg[200]="";

        char                            *help_email = NULL;
        char                            *help_phone = NULL;
        pin_flist_t                     *helpdesk_flistp = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);


	/***********************************************************
	 * Log input flist
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_email_welcome_note_notification_buf input flist", i_flistp);


	//Get the Account poid
	rfld_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, rfld_iflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(rfld_iflistp,	PIN_FLD_ACCOUNT_NO,	(void *)NULL, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(rfld_iflistp, PIN_FLD_NAMEINFO,	PIN_ELEMID_ANY,	ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ADDRESS, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_CITY,	(void *)NULL, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ZIP, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_FIRST_NAME, (void	*)NULL,	ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_LAST_NAME, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_EMAIL_ADDR, (void	*)NULL,	ebufp);
	phone_flistp = PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_PHONES, PIN_ELEMID_ANY, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_email_welcome_note_notification_buf PCM_OP_READ_FLDS input	flist",	rfld_iflistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, rfld_iflistp,	&rfld_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_email_welcome_note_notification_buf PCM_OP_READ_FLDS output flist", rfld_oflistp);
	//Get the account no 
	accno =	(char *)PIN_FLIST_FLD_GET(rfld_oflistp,	PIN_FLD_ACCOUNT_NO,	0, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_GET(rfld_oflistp, PIN_FLD_NAMEINFO,	1, 0, ebufp);
	if(tmp_flistp){
		//Get the Address
		address	= (char	*)PIN_FLIST_FLD_GET(tmp_flistp,	PIN_FLD_ADDRESS, 0,	ebufp);
		city = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 0, ebufp);
		zip	= (char	*)PIN_FLIST_FLD_GET(tmp_flistp,	PIN_FLD_ZIP, 0,	ebufp);
		fname =	(char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_FIRST_NAME, 0, ebufp);
		lname =	(char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_LAST_NAME, 0,	ebufp);
		email =	(char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_EMAIL_ADDR, 0, ebufp);
		//Get the Primary Mobile Number
		while((phone_flistp	= PIN_FLIST_ELEM_GET_NEXT (tmp_flistp, 
			PIN_FLD_PHONES,	&rec_id, 1,&cookie,	ebufp))	!= (pin_flist_t	*)NULL)
		{
			ph_type	= (int *)PIN_FLIST_FLD_GET(phone_flistp, PIN_FLD_TYPE, 0, ebufp);
			if(*ph_type	== 5)
				phone =	(char *)PIN_FLIST_FLD_GET(phone_flistp,	PIN_FLD_PHONE, 0, ebufp);
		}
	}
        tmp1_flistp = PIN_FLIST_ELEM_GET(rfld_oflistp, PIN_FLD_NAMEINFO, 2, 1, ebufp);
        if(tmp1_flistp){
                //Get the Address
                in_address = (char *)PIN_FLIST_FLD_GET(tmp1_flistp, PIN_FLD_ADDRESS, 0,ebufp);
                in_city = (char *)PIN_FLIST_FLD_GET(tmp1_flistp, PIN_FLD_CITY, 0, ebufp);
                in_zip     = (char *)PIN_FLIST_FLD_GET(tmp1_flistp, PIN_FLD_ZIP, 0, ebufp);

	}

	//Check	if the company contact info	exists
	tmp2_flistp	= PIN_FLIST_ELEM_GET(rfld_oflistp, PIN_FLD_NAMEINFO, 3,	1, ebufp);
	if(tmp2_flistp)
	{
		cfname = (char *)PIN_FLIST_FLD_GET(tmp2_flistp,	PIN_FLD_FIRST_NAME,	0, ebufp);
		clname = (char *)PIN_FLIST_FLD_GET(tmp2_flistp,	PIN_FLD_LAST_NAME, 0, ebufp);
	}
	//Else set the contact person as the main guy
	else
	{
		cfname = fname;
		clname = lname;
	}
	//Get additional address information
	memset( additional_addr, '\0', 512*sizeof(char));
        memset( inst_addr, '\0', 600*sizeof(char));

	mso_get_addidional_address(ctxp,i_flistp,additional_addr,inst_addr,ebufp);
	addnl_addr = (char *)malloc(strlen(additional_addr)+ strlen(address)+2);
	memset( addnl_addr, '\0', (strlen(additional_addr)+ strlen(address)+2));
	strcpy(addnl_addr, address );
	//strcpy(addnl_addr, additional_addr);
	strcat( addnl_addr, additional_addr );

        install_address = (char *)malloc(strlen(inst_addr)+ strlen(in_address)+50);
        memset( install_address, '\0', (strlen(inst_addr)+ strlen(in_address)+50));
        strcpy(install_address,in_address );
        strcat( install_address, inst_addr );

        if(in_city){
               sprintf(install_address, "%s, %s", install_address,in_city);
        }
        if(in_zip){
               sprintf(install_address, "%s, %s", install_address,in_zip);
        }


	//strcat(address,addnl_addr);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Final address");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, address );
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "install_address" );

	//sprintf(address,additional_addr);
	//Get the Service Level	Details
	si_flistp =	PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);
	//PIN_FLIST_FLD_SET(si_flistp, PIN_FLD_LOGIN,	(char *)NULL, ebufp);
	//PIN_FLIST_FLD_SET(si_flistp, PIN_FLD_PASSWD, (char *)NULL, ebufp);
	//tmp_flistp = PIN_FLIST_SUBSTR_ADD(si_flistp, MSO_FLD_BB_INFO, ebufp);
	//PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT, (char *)NULL, ebufp);
	//PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_TECHNOLOGY, (char	*)NULL,	ebufp);
	//PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_CITY,	(char *)NULL, ebufp);
	//PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_BILL_WHEN, (char *)NULL, ebufp);
	//tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, PIN_ELEMID_ANY, ebufp);
	//PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_NAME,	NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_email_welcome_note_notification_buf PCM_OP_READ_FLDS input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_email_welcome_note_notification_buf PCM_OP_READ_FLDS output flist", so_flistp);

	login = (char *)PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_LOGIN, 0, ebufp);
	tmp3_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	cmts_name = (char *)PIN_FLIST_FLD_GET(tmp3_flistp, MSO_FLD_NETWORK_ELEMENT, 1, ebufp);
	passwd = (char *)PIN_FLIST_FLD_GET(tmp3_flistp, MSO_FLD_PASSWORD, 1, ebufp);
	*technology = (int *)PIN_FLIST_FLD_TAKE(tmp3_flistp, MSO_FLD_TECHNOLOGY, 1, ebufp);
	mso_city = (char *)PIN_FLIST_FLD_GET(tmp3_flistp, PIN_FLD_CITY, 1, ebufp);
	pay_term = (int *)PIN_FLIST_FLD_GET(tmp3_flistp, PIN_FLD_BILL_WHEN, 1, ebufp);
	/*Chandrakala -- Changes done for checking payment type*/
        conn_type = (int *)PIN_FLIST_FLD_GET(tmp3_flistp, PIN_FLD_INDICATOR, 1, ebufp);

	if(*conn_type == 0 || *conn_type ==1){
	//	*validity = 0;		
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," validity_date 1" );

	        strcpy(validity_date," ");
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "validity_date 2 ");


	}
	else if (*conn_type == 2){
		fm_mso_get_purchase_prod_details(ctxp,(PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp)),&pur_prod_flistp,ebufp);	
	        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_purchase_prod_details 3");

        	validity = PIN_FLIST_FLD_GET(pur_prod_flistp, PIN_FLD_PURCHASE_END_T, 0, ebufp);
		*validity = *validity - 1; 
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_purchase_prod_details 4");
	        pin_strftimet(validity_date, sizeof(validity_date), "%d/%m/%Y %r", *validity);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_purchase_prod_details 1");

	        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, validity_date );

	}
	alias_list = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 1, ebufp);
	if(alias_list)
		mac = (char *)PIN_FLIST_FLD_GET(alias_list, PIN_FLD_NAME, 1, ebufp);
/*
	alias_list = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 1, 1, ebufp);
	if(alias_list)
		mac = (char *)PIN_FLIST_FLD_GET(alias_list, PIN_FLD_NAME, 1, ebufp);
*/
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "prepare_email_welcome_note_notification_buf: Error", ebufp);
		goto CLEANUP;
	}

	//Get the Provisioning Tag
	srv_pdp	= (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
	mso_get_prov_tag(ctxp, srv_pdp, &prov_name, &pkg_name, &prod_pdp, ebufp);
	if(prov_name)
	{
		PIN_ERR_LOG_MSG(3, "Prov");
		PIN_ERR_LOG_MSG(3, prov_name);
	}
	if (pkg_name == NULL || prov_name == NULL)
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0,	0);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
		"prepare_email_welcome_note_notification_buf: No Package OR Provisioning Error", ebufp);
		goto CLEANUP;
	}
	//Get the Plan Amount
	mso_get_plan_amt(ctxp, pkg_name, srv_pdp, mso_city, *pay_term,  &amount, &speed, &fup_speed, &quota, ebufp);

	//Get the Hardware Plan	Name
	mso_get_hw_plan(ctxp, srv_pdp, &tech_s,	ebufp);

	//Added hathway contact and email 		
        get_helpdesk_details(ctxp, i_flistp, in_city, &helpdesk_flistp, ebufp);
        if(helpdesk_flistp)
        {
                help_phone = PIN_FLIST_FLD_GET(helpdesk_flistp, PIN_FLD_PHONE, 0, ebufp);
                help_email = PIN_FLIST_FLD_GET(helpdesk_flistp, PIN_FLD_EMAIL_ADDR, 0, ebufp);
        }

	//Get Plan Details
	//mso_get_plan_details(ctxp, prov_name, *technology, &quota, &speed, &fup_speed, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "prepare_email_welcome_note_notification_buf: Error", ebufp);
		goto CLEANUP;
	}
	//Get the current date
	curr_time = pin_virtual_time(NULL);
	pin_strftimet(curr_date, sizeof(curr_date), "%d/%m/%Y %r", curr_time);
	
	if(!pbo_decimal_is_null(amount, ebufp))
		amount_d = pbo_decimal_to_double(amount, ebufp);
	if(!pbo_decimal_is_null(speed, ebufp))
		speed_d = pbo_decimal_to_double(speed, ebufp);
	if(!pbo_decimal_is_null(quota, ebufp))
		quota = pbo_decimal_negate(quota, ebufp);
		quota_d = pbo_decimal_to_double(quota, ebufp);
	if(!pbo_decimal_is_null(fup_speed, ebufp))
		fup_speed_d = pbo_decimal_to_double(fup_speed, ebufp);
	
	//Create the Welcome Note email
	flag = 1;
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Getting the EMAIL	Templ");
	get_bb_email_template(ctxp, i_flistp, &email_template, &flag, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "prepare_email_welcome_note_notification_buf: Error", ebufp);
		goto CLEANUP;
	}
	if (!email_template)
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0,	0);
		goto CLEANUP;
	}
                        sprintf(msg, "quote is %.01f", quota_d);
                        PIN_ERR_LOG_MSG(3, msg);


	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Email Template");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, email_template );
	sprintf(str, email_template, curr_date, fname, lname, addnl_addr, install_address,
		city, zip, phone, cfname, clname, accno, cmts_name,
		pkg_name, tech_s, mac, wifi_mac,login, email, passwd, 
		prov_name, *conn_type, validity_date, *pay_term, speed_d, quota_d, fup_speed_d, amount_d, help_phone, help_email);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "welcome str");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );
	welcome_note = (char *)malloc(sizeof(str));
	strcpy(welcome_note, str); 

	//Log the building of the welcome note
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Welcome Note");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, welcome_note);

	/*
	 * Push	Message	in EMAIL payload
	 */
	PIN_FLIST_FLD_SET(*ret_flistpp,	PIN_FLD_MESSAGE, welcome_note, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistpp,	PIN_FLD_SUBJECT, "Welcome to Hathway", ebufp);


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"prepare_welcome_note_notification_buf enriched	input flist", *ret_flistpp);

CLEANUP:
	if (email_template)
	{
		free(email_template);
	}
	if (welcome_note)
	{
		free(welcome_note);
	}
	if (prov_name)
	{
		free(prov_name);
	}
        if (install_address)
        {
                free(install_address);
        }
        if (addnl_addr)
        {
                free(addnl_addr);
        }

	if(!PIN_POID_IS_NULL(prod_pdp)){
		PIN_POID_DESTROY(prod_pdp, ebufp);
	}
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&ai_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&ao_flistp, NULL);
	return;
}

static void
get_bb_email_template(
	pcm_context_t			*ctxp, 
	pin_flist_t			*i_flistp, 
	char				**email_template,
	int				*flag,
	pin_errbuf_t			*ebufp)
{
	//int32				*flag	= NULL;
	poid_t				*pdp = NULL;
	poid_t				*s_pdp = NULL;
	int64				database;
	pin_flist_t			*search_i_flistp = NULL;
	pin_flist_t			*search_o_flistp = NULL;
	pin_flist_t			*tmp_flistp	= NULL;
	int32				s_flags	;
	int32				*string_id = NULL;
	char				*msg_template =	NULL;
	int32				str_ver;
	char				*template = "select X from /strings where F1 = V1 and F2 = V2 ";


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"get_bb_email_template input flist", i_flistp);

	//string_id	= flag;
	pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	database = PIN_POID_GET_DB(pdp);

	search_i_flistp	= PIN_FLIST_CREATE(ebufp);
	s_pdp =	(poid_t	*)PIN_POID_CREATE(database, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(search_i_flistp, PIN_FLD_POID, (void *)s_pdp,	ebufp);

	s_flags	= SRCH_DISTINCT	;
	PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void	*)&s_flags,	ebufp);

	PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);

	tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_DOMAIN, "WELCOME_NOTE_TEMPLATE", ebufp);

	tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STRING_ID, flag, ebufp);

	tmp_flistp = PIN_FLIST_ELEM_ADD	(search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_STRING, NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"get_bb_email_template search input flist ", search_i_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_i_flistp,
		&search_o_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"get_bb_email_template search output flist", search_o_flistp);

	tmp_flistp = PIN_FLIST_ELEM_GET(search_o_flistp, PIN_FLD_RESULTS, 0, 0,	ebufp);
	msg_template = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_STRING, 0,	ebufp);

	if (msg_template && strlen(msg_template) > 0 )
	{
		 *email_template = (char *) malloc(strlen(msg_template)+1);
		 memset(*email_template, 0, sizeof(msg_template)+1);
		 strcpy(*email_template,msg_template);
		 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*email_template );
	}

	PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_o_flistp, NULL);

	return;
}

/*******************************************************************
 * prepare_sms_welcome_note_notification_buf()
 *
 * Enrich input	filst with service details required
 * optionally performa validation on input fields
 *******************************************************************/
static void
prepare_sms_welcome_note_notification_buf(
	pcm_context_t			*ctxp,
	pin_flist_t			*i_flistp,
	pin_flist_t			**ret_flistpp,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*si_flistp = NULL;
	pin_flist_t			*so_flistp = NULL;
	pin_flist_t			*rfld_iflistp =	NULL;
	pin_flist_t			*rfld_oflistp =	NULL;
	pin_flist_t			*ai_flistp = NULL;
	pin_flist_t			*ao_flistp = NULL;
	pin_flist_t			*tmp_flistp	= NULL;
	pin_flist_t			*phone_flistp =	NULL;
	pin_flist_t			*alias_list	= NULL;
	pin_flist_t                     *helpdesk_flistp = NULL;

	char				*ne	= NULL;
	char				*sms_template =	NULL;
	char				*account_no	= NULL;
	char				*stb_id	= NULL;
	char				*vc_id = NULL;
	char				*accno = NULL;
	char				*phone = NULL;
	char				*city =	NULL;
	char				*zip = NULL;
	char				*fname = NULL;
	char				*lname = NULL;
	char				*address = NULL;
	char				*welcome_note =	NULL;
	char				*tmp_str = NULL;
	char				 str[1024];
	char				*cmts_name = NULL;
	char				tech_s[10] = {""};
	char				*login = NULL;
	char				*mac = NULL;
	char				*passwd	= NULL;
	char				*user_id = NULL;
	char				*email = NULL;
	char				*help_email = NULL;
	char				*help_phone = NULL;
	int				*tech =	NULL;
	struct				tm *current_time;
	time_t				curr_time =	0;
	int				year;
	int				month;
	int				day;
	int				flag;
   // char					  curr_date[50];
	int				*ph_type = 0;
	int				rec_id = 0;

	pin_cookie_t			cookie = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);


	/***********************************************************
	 * Log input flist
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_sms_welcome_note_notification_buf input flist", i_flistp);


	//Get the Account poid
	rfld_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, rfld_iflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(rfld_iflistp,	PIN_FLD_ACCOUNT_NO,	(void *)NULL, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(rfld_iflistp, PIN_FLD_NAMEINFO,	1, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_sms_welcome_note_notification_buf PCM_OP_READ_FLDS	input flist", rfld_iflistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, rfld_iflistp,	&rfld_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_sms_welcome_note_notification_buf PCM_OP_READ_FLDS	output flist", rfld_oflistp);
	//Get the account no 
	accno =	(char *)PIN_FLIST_FLD_GET(rfld_oflistp,	PIN_FLD_ACCOUNT_NO,	0, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_GET(rfld_oflistp, PIN_FLD_NAMEINFO,	1, 0, ebufp);
	if(tmp_flistp){
		//Get the Address
		address	= (char	*)PIN_FLIST_FLD_GET(tmp_flistp,	PIN_FLD_ADDRESS, 1,	ebufp);
		city = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 1, ebufp);
		zip	= (char	*)PIN_FLIST_FLD_GET(tmp_flistp,	PIN_FLD_ZIP, 1,	ebufp);
		fname =	(char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_FIRST_NAME, 1, ebufp);
		lname =	(char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_LAST_NAME, 1,	ebufp);
		email =	(char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_EMAIL_ADDR, 1, ebufp);
		//Get the Primary Mobile Number
		while((phone_flistp	= PIN_FLIST_ELEM_GET_NEXT (tmp_flistp, 
			PIN_FLD_PHONES,	&rec_id, 1,&cookie,	ebufp))	!= (pin_flist_t	*)NULL)
		{
			ph_type	= (int *)PIN_FLIST_FLD_GET(phone_flistp, PIN_FLD_TYPE, 0, ebufp);
			if(*ph_type	== 5)
				phone =	(char *)PIN_FLIST_FLD_GET(phone_flistp,	PIN_FLD_PHONE, 0, ebufp);
		}
	}
	//Get the Service Level	Details
	si_flistp =	PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(si_flistp, PIN_FLD_LOGIN,	(char *)NULL, ebufp);
	PIN_FLIST_FLD_SET(si_flistp, PIN_FLD_PASSWD, (char *)NULL, ebufp);
	tmp_flistp = PIN_FLIST_SUBSTR_ADD(si_flistp, MSO_FLD_BB_INFO, ebufp);
	/*Pawan:17-02-2015: Added below 2 lines since MSO_FLD_BB_INFO is returned as null */
	PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT,	NULL, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_PASSWORD,	NULL, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_NAME,	NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_sms_welcome_note_notification_buf PCM_OP_READ_FLDS	input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_sms_welcome_note_notification_buf PCM_OP_READ_FLDS	output flist", so_flistp);

	login =	(char *)PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_LOGIN,	1, ebufp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 1, ebufp);
	if(tmp_flistp)
	{
		cmts_name = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT, 1, ebufp);
		passwd = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_PASSWORD, 1, ebufp);
	}
	alias_list = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 1, ebufp);
	if(alias_list)
		mac	= (char	*)PIN_FLIST_FLD_GET(alias_list,	PIN_FLD_NAME, 1, ebufp);
/*
	alias_list = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 1, 1, ebufp);
	if(alias_list)
		mac	= (char	*)PIN_FLIST_FLD_GET(alias_list,	PIN_FLD_NAME, 1, ebufp);
*/

	//Create the Welcome Note sms
	flag = 1;
	get_bb_sms_template(ctxp, i_flistp, &sms_template, &flag, ebufp);
	get_helpdesk_details(ctxp, i_flistp, city, &helpdesk_flistp, ebufp);
	if(helpdesk_flistp)
	{
		help_phone = PIN_FLIST_FLD_GET(helpdesk_flistp, PIN_FLD_PHONE, 0, ebufp);
		help_email = PIN_FLIST_FLD_GET(helpdesk_flistp, PIN_FLD_EMAIL_ADDR, 0, ebufp);
	}
	if (!sms_template)
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0,	0);
		goto CLEANUP;
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, sms_template );
	//sprintf(str, sms_template, accno, fname, lname,	login, email, passwd, help_phone, help_email);
	sprintf(str, sms_template, accno, phone, help_phone, help_email);
	welcome_note = (char *)malloc(sizeof(str));
	strcpy(welcome_note, str); 

	/*
	 * Prepare Message
	 */
   

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, welcome_note );

	/*
	 * Push	Message	in EMAIL payload
	 */
	PIN_FLIST_FLD_SET(*ret_flistpp,	PIN_FLD_MESSAGE, welcome_note, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"prepare_welcome_note_notification_buf enriched	input flist", *ret_flistpp);

CLEANUP:
	if (sms_template)
	{
		free(sms_template);
	}
	if (welcome_note)
	{
		free(welcome_note);
	}
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&ai_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&ao_flistp, NULL);
	return;
}

static void
get_bb_sms_template(
	pcm_context_t			*ctxp, 
	pin_flist_t			*i_flistp, 
	char				**sms_template,
	int				*flag,
	pin_errbuf_t			*ebufp)
{
	//int32				*flag	= NULL;
	poid_t				*pdp = NULL;
	poid_t				*s_pdp = NULL;
	int64				database;
	pin_flist_t			*search_i_flistp = NULL;
	pin_flist_t			*search_o_flistp = NULL;
	pin_flist_t			*tmp_flistp	= NULL;
	int32				s_flags	;
	int32				*string_id = NULL;
	char				*msg_template =	NULL;
	int32				str_ver;
	char				*template =	"select x from /strings where F1 = V1 and F2 = V2 ";


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"get_bb_sms_template input flist", i_flistp);

	//string_id	= flag;
	pdp	= PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID,	0, ebufp);
	database = PIN_POID_GET_DB(pdp);

	search_i_flistp	= PIN_FLIST_CREATE(ebufp);
	s_pdp =	(poid_t	*)PIN_POID_CREATE(database,	"/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(search_i_flistp, PIN_FLD_POID, (void *)s_pdp,	ebufp);

	s_flags	= SRCH_DISTINCT	;
	PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void	*)&s_flags,	ebufp);

	PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);

	tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_DOMAIN, "WELCOME_NOTE_TEMPLATE_SMS", ebufp);

	tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STRING_ID, flag, ebufp);

	tmp_flistp = PIN_FLIST_ELEM_ADD	(search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_STRING, NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"get_bb_sms_template search input flist ", search_i_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_i_flistp,
		&search_o_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"get_bb_sms_template search output flist", search_o_flistp);

	tmp_flistp = PIN_FLIST_ELEM_GET(search_o_flistp, PIN_FLD_RESULTS, 0, 0,	ebufp);
	msg_template = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_STRING, 0,	ebufp);

	if (msg_template && strlen(msg_template) > 0 )
	{
		 *sms_template = (char *) malloc(strlen(msg_template)+1);
		 memset(*sms_template, 0, sizeof(msg_template)+1);
		 strcpy(*sms_template,msg_template);
		 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*sms_template );
	}

	PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_o_flistp, NULL);

	return;
}

/*********************************************************************
* mso_get_prov_tag : 
* 1. Read the /mso_purchased_plan based	on the service poid.
* 2. Get the plan name and provisioning	tag.
**********************************************************************/
static void
mso_get_prov_tag(
	pcm_context_t			*ctxp, 
	poid_t				*srv_pdp, 
	char				**prov_name,
	char				**pkg_name,
	poid_t				**prod_pdp,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*srch_iflistp =	NULL;
	pin_flist_t			*srch_oflistp =	NULL;
	pin_flist_t			*rfld_iflistp =	NULL;
	pin_flist_t			*rfld_oflistp =	NULL;
	pin_flist_t			*temp_flistp = NULL;
	pin_flist_t			*prods_flistp =	NULL;

	poid_t				*srch_pdp =	NULL;
	poid_t				*plan_pdp =	NULL;

	char				*tmpl =	"select x from /mso_purchased_plan where F1 = V1 ";
	char				*prov_tag =	NULL;
	int64				database = 0;
	int32				rec_id = 0;
	int32				rec_id1 = 0;
	int					s_flags	= SRCH_DISTINCT;
	int				flag = 0;
	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie1 = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);
	database = PIN_POID_GET_DB(srv_pdp);
	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	srch_pdp = (poid_t *)PIN_POID_CREATE(database, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(srch_iflistp,	PIN_FLD_POID, (void	*)srch_pdp,	ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp,	PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp,	PIN_FLD_TEMPLATE, (void	*)tmpl,	ebufp);
	//Add the Argument 1
	temp_flistp	= PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1,	ebufp);
	PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_SERVICE_OBJ,	(void *)srv_pdp, ebufp);
	//Add the Result Array
	temp_flistp	= PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS,	PIN_ELEMID_ANY,	ebufp);
	PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_PLAN_OBJ, (void *)NULL, ebufp);
	prods_flistp = PIN_FLIST_ELEM_ADD(temp_flistp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_NAME, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_PROVISIONING_TAG, (void *)NULL, ebufp);

	//Print	the	Flist
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_get_prov_tag: Search Input Flist", srch_iflistp);
	//Call the PCM_OP_SEARCH
	PCM_OP(ctxp, PCM_OP_SEARCH,	0, srch_iflistp, &srch_oflistp,	ebufp);
	//Print	the	Output
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_get_prov_tag: Search Output	Flist",	srch_oflistp);
	//temp_flistp	= PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	while( (temp_flistp = PIN_FLIST_ELEM_GET_NEXT (srch_oflistp, PIN_FLD_RESULTS, &rec_id, 1,&cookie, ebufp))
		!=     (pin_flist_t *)NULL  )
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"temp_flistp: ", temp_flistp);
		plan_pdp = (poid_t *)PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
		 cookie1 = NULL;
		 rec_id1 = 0;
		 while ( (prods_flistp = PIN_FLIST_ELEM_GET_NEXT (temp_flistp, PIN_FLD_PRODUCTS, &rec_id1, 1,&cookie1, ebufp))
			 !=	(pin_flist_t *)NULL	)
		{
			prov_tag = (char *) PIN_FLIST_FLD_GET(prods_flistp, PIN_FLD_PROVISIONING_TAG, 1, ebufp);

			if(prov_tag && strcmp(prov_tag,"") != 0)
				*prov_name = strdup(prov_tag);

			if(*prov_name){
				PIN_ERR_LOG_MSG(3, "Prov");
				PIN_ERR_LOG_MSG(3, *prov_name);
				flag = 1;
				break;
			}
		}
		if(flag)
		  break;
		
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_get_prov_tag: Error calling Search",	ebufp);
		goto cleanup;
	}
	if(!(PIN_POID_IS_NULL(plan_pdp)))
	{
		//Get the Plan Name
		rfld_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(rfld_iflistp,	PIN_FLD_POID, (void	*)plan_pdp,	ebufp);
		PIN_FLIST_FLD_SET(rfld_iflistp,	PIN_FLD_NAME, (void	*)NULL,	ebufp);
		//Print	the	Flist
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_get_prov_tag: ReadFields Input Flist", rfld_iflistp);
		//Call the PCM_OP_SEARCH
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, rfld_iflistp,	&rfld_oflistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_get_prov_tag: Error calling Search",	ebufp);
			goto cleanup;
		}
		//Print	the	Output
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_get_prov_tag: ReadFields Output Flist", rfld_oflistp);
		*pkg_name = (char *)PIN_FLIST_FLD_TAKE(rfld_oflistp, PIN_FLD_NAME, 1, ebufp);
		PIN_ERR_LOG_MSG(3, "pkg_name");
		PIN_ERR_LOG_MSG(3, *pkg_name);
		
	}

cleanup:
	//Free the memory
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_FLIST_DESTROY_EX(&srch_iflistp,	NULL);
	PIN_FLIST_DESTROY_EX(&srch_oflistp,	NULL);
	PIN_FLIST_DESTROY_EX(&rfld_iflistp,	NULL);
	PIN_FLIST_DESTROY_EX(&rfld_oflistp,	NULL);
	return;
}

/*********************************************************************
* mso_get_hw_pan : 
* 1. Get the product object.
* 2. Get the rate object based on the city and pay term.
* 3. From that get the fixed amount
**********************************************************************/
static void
mso_get_plan_amt(
	pcm_context_t			*ctxp, 
	char				*pkg_name,
	poid_t				*srv_pdp,
	char				*mso_city,  
	int				pay_term,
	pin_decimal_t			**amount, 
	pin_decimal_t			**speed, 
	pin_decimal_t			**fup_speed,
        pin_decimal_t                   **quota,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*srch_iflistp =	NULL;
	pin_flist_t			*srch_oflistp =	NULL;
	pin_flist_t			*rfld_iflistp =	NULL;
	pin_flist_t			*rlfd_oflistp =	NULL;
	pin_flist_t			*temp_flistp = NULL;
	pin_flist_t			*rates_flistp =	NULL;
	pin_flist_t			*bal_flistp	= NULL;
	poid_t				*srch_pdp =	NULL;
	poid_t				*rate_pdp =	NULL;
	int64				database = 0;
	int				s_flags	= 768;
	int				*res_id	= 0;
	int32				rec_id = 0;
	int32				plan_payterm = 0;
	//char				*tmpl = "select X from /mso_cfg_credit_limit where F1 = V1 and ( F2 = V2 or F3 = V3 ) ";
        char                            *tmpl = "select X from /mso_cfg_credit_limit where F1 = V1 and ( F2 = V2 or F3 = V3 ) and F4 = V4 ";
	char				*city = NULL;
	pin_cookie_t			cookie = NULL;

        pin_flist_t                     *cl_flistp = NULL;
        char                            msg[200]="";

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_MSG(3, "Test 1");
	database = PIN_POID_GET_DB(srv_pdp);
	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	srch_pdp = (poid_t *)PIN_POID_CREATE(database, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(srch_iflistp,	PIN_FLD_POID, (void	*)srch_pdp,	ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp,	PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp,	PIN_FLD_TEMPLATE, (void	*)tmpl,	ebufp);
	//Add the Argument 1
	temp_flistp	= PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(temp_flistp, MSO_FLD_PLAN_NAME, (void *)pkg_name, ebufp);
	//Add the Argument 2
	temp_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 2, ebufp);
	rates_flistp = PIN_FLIST_ELEM_ADD(temp_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp);
        //rates_flistp = PIN_FLIST_ELEM_ADD(temp_flistp, MSO_FLD_CITIES, 0, ebufp);
	PIN_FLIST_FLD_SET(rates_flistp, PIN_FLD_CITY, (void *)mso_city, ebufp);
	//Add the Argument 3
	temp_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 3, ebufp);
	rates_flistp = PIN_FLIST_ELEM_ADD(temp_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp);
        //rates_flistp = PIN_FLIST_ELEM_ADD(temp_flistp, MSO_FLD_CITIES, 0, ebufp);
        PIN_FLIST_FLD_SET(rates_flistp, PIN_FLD_CITY, (void *)"*", ebufp);


        temp_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 4, ebufp);
        rates_flistp = PIN_FLIST_ELEM_ADD(temp_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(rates_flistp, PIN_FLD_BILL_WHEN, &pay_term, ebufp);

	//Add the Result Array
	temp_flistp	= PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	//Print	the	Flist
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_get_plan_amt: Search Input Flist", srch_iflistp);
	//Call the PCM_OP_SEARCH
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_get_plan_amt: Error calling Search",	ebufp);
		goto cleanup;
	}
	//Print	the	Output
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_get_plan_amt: Search Output Flist",	srch_oflistp);

	//Get the Rate Obj
	temp_flistp	= PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	if(temp_flistp)
	{	
		PIN_ERR_LOG_MSG(3, "Test Temp");
		while ( (bal_flistp = PIN_FLIST_ELEM_GET_NEXT (temp_flistp, MSO_FLD_CITIES, &rec_id, 1,&cookie, ebufp)) 
			!= (pin_flist_t *)NULL)
		{	
			PIN_ERR_LOG_MSG(3, "Test Temp1");
			city = (void *)PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_CITY, 0, ebufp);
			plan_payterm = *(int32 *)PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);
			if((strcmp(city, mso_city) == 0 || strcmp(city, "*") == 0) && (pay_term == plan_payterm))
			{
				PIN_ERR_LOG_MSG(3, "Test Temp2");
				*amount = PIN_FLIST_FLD_TAKE(bal_flistp, PIN_FLD_CHARGE_AMT, 0, ebufp);
				*speed = PIN_FLIST_FLD_TAKE(bal_flistp, MSO_FLD_DL_SPEED, 0, ebufp);
				*fup_speed = PIN_FLIST_FLD_TAKE(bal_flistp, MSO_FLD_FUP_DL_SPEED, 0, ebufp);

				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_get_plan_amt: Error", ebufp);
					goto cleanup;
				}
				//break;
			}
		        cl_flistp = PIN_FLIST_ELEM_GET(bal_flistp, MSO_FLD_CREDIT_PROFILE, 1000104, 1, ebufp);
			if(cl_flistp){
	                        *quota = (pin_decimal_t *)PIN_FLIST_FLD_TAKE(cl_flistp, PIN_FLD_CREDIT_FLOOR, 1, ebufp);
			}else{
                        	cl_flistp = PIN_FLIST_ELEM_GET(bal_flistp, MSO_FLD_CREDIT_PROFILE, 1100010, 1, ebufp);
                        	if(cl_flistp){
                                	*quota = (pin_decimal_t *)PIN_FLIST_FLD_TAKE(cl_flistp, PIN_FLD_CREDIT_FLOOR, 1, ebufp);
                        	}
			}
                        sprintf(msg, "quote is %s", pbo_decimal_to_str(*quota, ebufp));
                        PIN_ERR_LOG_MSG(3, msg);
                        sprintf(msg, "speed is %s", pbo_decimal_to_str(*speed, ebufp));
                        PIN_ERR_LOG_MSG(3, msg);
                        sprintf(msg, "fup_speed is %s", pbo_decimal_to_str(*fup_speed, ebufp));
                        PIN_ERR_LOG_MSG(3, msg);
			break;
		}
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_get_plan_amt: Error ", ebufp);
		goto cleanup;
	}
cleanup:
	//Free the memory
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_FLIST_DESTROY_EX(&srch_iflistp,	NULL);
	PIN_FLIST_DESTROY_EX(&srch_oflistp,	NULL);
	return;
}

/*********************************************************************
* mso_get_hw_pan : 
* 1. Get the /device object	for	the	service.
* 2. Based on that get the /mso_purchased_plan.
* 3. From that,	we can get the plan	name.
**********************************************************************/
static void
mso_get_hw_plan(
	pcm_context_t			*ctxp, 
	poid_t				*srv_pdp, 
	char				**tech_s, 
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*srch_iflistp =	NULL;
	pin_flist_t			*srch_oflistp =	NULL;
	pin_flist_t			*srv_flistp	= NULL;
	pin_flist_t			*temp_flistp = NULL;
	pin_flist_t			*plan_flistp = NULL;
	poid_t				*srch_pdp =	NULL;
	int64				database = 0;
	int				s_flags	= SRCH_DISTINCT;
	char				*tmpl =	"select x from /plan 1, /mso_purchased_plan 2, /device 3 where 3.F1 = V1 and 2.F2 = 3.F3 and 1.F4 = 2.F5 ";

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	
	database = PIN_POID_GET_DB(srv_pdp);
	srch_pdp = (poid_t *)PIN_POID_CREATE(database, "/search", -1, ebufp);
	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_iflistp,	PIN_FLD_POID, (void	*)srch_pdp,	ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp,	PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp,	PIN_FLD_TEMPLATE, (void	*)tmpl,	ebufp);
	//Add the Argument 1
	temp_flistp	= PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1,	ebufp);
	srv_flistp = PIN_FLIST_ELEM_ADD(temp_flistp, PIN_FLD_SERVICES, 1, ebufp);
	PIN_FLIST_FLD_SET(srv_flistp, PIN_FLD_SERVICE_OBJ, (void *)srv_pdp, ebufp);
	//Add the Argument 2
	temp_flistp	= PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 2,	ebufp);
	PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_POID, (void *)NULL, ebufp);
	//Add the Argument 3
	temp_flistp	= PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 3,	ebufp);
	plan_flistp	= PIN_FLIST_ELEM_ADD(temp_flistp, PIN_FLD_PLAN, 0,	ebufp);
	PIN_FLIST_FLD_SET(plan_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, (void *)NULL, ebufp);
	//Add the Argument 4
	temp_flistp	= PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 4,	ebufp);
	PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_POID, (void *)NULL, ebufp);
	//Add the Argument 5
	temp_flistp	= PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 5,	ebufp);
	PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_PLAN_OBJ, (void *)NULL, ebufp);
	//Add the Result Array
	temp_flistp	= PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS,	PIN_ELEMID_ANY,	ebufp);
	PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_NAME, (void *)NULL, ebufp);
	//Print	the	Flist
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_get_hw_plan: Search Input Flist", srch_iflistp);
	//Call the PCM_OP_SEARCH
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_get_hw_plan: Error calling Search",	ebufp);
		goto cleanup;
	}
	//Print	the	Output
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_get_hw_plan: Search Output Flist",	srch_oflistp);
	temp_flistp	= PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS,	PIN_ELEMID_ANY,	1, ebufp);
	if(temp_flistp)
	{
		*tech_s	= (char	*)PIN_FLIST_FLD_TAKE(temp_flistp, PIN_FLD_NAME,	1, ebufp);
	}
cleanup:
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_FLIST_DESTROY_EX(&srch_iflistp,	NULL);
	PIN_FLIST_DESTROY_EX(&srch_oflistp,	NULL);
	return;
}

/***********************************************************************
* mso_get_plan_details: 
* Get the plan details from the /mso_config_bb_pt
*
************************************************************************/
static void
mso_get_plan_details(
	pcm_context_t			*ctxp, 
	char				*prov_name,
	int				*tech,
	pin_decimal_t			**quota, 
	pin_decimal_t			**speed, 
	pin_decimal_t			**fup_speed, 
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*cm_cache_t = NULL;
	pin_flist_t			*cache_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*c_flistp = NULL;
	pin_flist_t			*t_flistp = NULL;
	
	int32				err = PIN_ERR_NONE;
	int32				elemid = 0;
	int32				prov_tech = 0;
	pin_cookie_t			cookie = NULL;
	cm_cache_key_iddbstr_t		kids;
	char				msg[200]="";
	
	/******************************************************
	* Clear the ebuf
	******************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
    
	/******************************************************
	* If the cahce is empty error out.
	******************************************************/
	if ( mso_prov_bb_config_ptr == NULL) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "no config flist for /mso_config_bb_pt cached ");
		pin_set_err(ebufp, PIN_ERRLOC_CM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_FLD_CACHE_NAME, 0, 0);
	}
	//Chech the cache entry
	kids.id = 0;
	kids.db = 0;
	kids.str = "/mso_config_bb_pt";
	cache_flistp = cm_cache_find_entry(mso_prov_bb_config_ptr, 
		&kids, &err);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"cm_cache_find_entry found flist", cache_flistp);
	switch(err) 
	{
		case PIN_ERR_NONE:
			break;
		default:
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
				"mso_get_plan_details from_cache: error "
				"accessing data dictionary cache.");
			pin_set_err(ebufp, PIN_ERRLOC_CM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE, 
				PIN_ERR_NOT_FOUND, PIN_FLD_CACHE_NAME, 0, 0);
			goto cleanup;
	}
	PIN_ERR_LOG_MSG(3, "Before printing prov_name");
	sprintf(msg, "prov_name is %s", prov_name);
	PIN_ERR_LOG_MSG(3, msg);
	sprintf(msg, "technology is %d", *tech);
	PIN_ERR_LOG_MSG(3, msg);
	while ( (tmp_flistp = PIN_FLIST_ELEM_GET_NEXT (cache_flistp,
		PIN_FLD_RESULTS, &elemid, 1,&cookie, ebufp))!= (pin_flist_t *)NULL )
	{
		prov_tech = *(int *)PIN_FLIST_FLD_GET(tmp_flistp,  MSO_FLD_BB_TECHNOLOGY, 0, ebufp);
		if ((prov_name != NULL && strcmp((char *)PIN_FLIST_FLD_GET(tmp_flistp, 
			PIN_FLD_PROVISIONING_TAG, 0, ebufp ), prov_name) == 0) &&
			prov_tech == *tech)
		{
			t_flistp = PIN_FLIST_COPY(tmp_flistp, ebufp); 
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"TEMP FLIST", t_flistp);
			*quota = (pin_decimal_t *)PIN_FLIST_FLD_TAKE(t_flistp,
				MSO_FLD_LIMIT, 1, ebufp);
			sprintf(msg, "quote is %s", pbo_decimal_to_str(*quota, ebufp));
			PIN_ERR_LOG_MSG(3, msg);
			if(!speed || pbo_decimal_is_null(*speed, ebufp)){
				*speed = (pin_decimal_t *)PIN_FLIST_FLD_TAKE(t_flistp,
					MSO_FLD_DL_SPEED, 1, ebufp);
				sprintf(msg, "speed is %s", pbo_decimal_to_str(*speed, ebufp));
				PIN_ERR_LOG_MSG(3, msg);
			}
			else{
				sprintf(msg, "speed is %s", pbo_decimal_to_str(*speed, ebufp));
				PIN_ERR_LOG_MSG(3, msg);
			}
			if(!fup_speed || pbo_decimal_is_null(*fup_speed, ebufp)){
				*fup_speed = (pin_decimal_t *)PIN_FLIST_FLD_TAKE(t_flistp,
					MSO_FLD_FUP_DL_SPEED, 1, ebufp);
				sprintf(msg, "fup_speed is %s", pbo_decimal_to_str(*fup_speed, ebufp));
				PIN_ERR_LOG_MSG(3, msg);
			}else{
				sprintf(msg, "fup_speed is %s", pbo_decimal_to_str(*fup_speed, ebufp));
				PIN_ERR_LOG_MSG(3, msg);
			}
			PIN_FLIST_DESTROY_EX(&t_flistp, NULL);
		}
	}
cleanup:
	PIN_FLIST_DESTROY_EX(&cache_flistp, NULL);
	return;
}
static void
mso_get_addidional_address(
	pcm_context_t			*ctxp, 
	pin_flist_t			*i_flistp, 
	char				*address,
        char                            *install_address,
	pin_errbuf_t			*ebufp)
{
	poid_t				*pdp = NULL;
	poid_t				*s_pdp = NULL;
	int64				database;
	int				s_flags;
	pin_flist_t			*search_i_flistp = NULL;
	pin_flist_t			*search_o_flistp = NULL;
	pin_flist_t			*name_info_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	char				*template = "select X from /mso_customer_credential where F1 = V1 ";
	char				*bldg_name = NULL ;
	char				*landmark = NULL ;
	char				*location = NULL ;
	char				*street_name = NULL ;
	char				*area = NULL ;
	char				*comma = ", " ;

        pin_flist_t                     *install_name_info_flistp = NULL;



	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"mso_get_addidional_address input flist", i_flistp);

	pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	database = PIN_POID_GET_DB(pdp);

	search_i_flistp	= PIN_FLIST_CREATE(ebufp);
	s_pdp =	(poid_t	*)PIN_POID_CREATE(database, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(search_i_flistp, PIN_FLD_POID, (void *)s_pdp,	ebufp);

	s_flags	= SRCH_DISTINCT	;
	PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void	*)&s_flags,	ebufp);

	PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);

	tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, pdp, ebufp);

	tmp_flistp = PIN_FLIST_ELEM_ADD	(search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"mso_get_addidional_address search input flist ", search_i_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_i_flistp,
		&search_o_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"mso_get_addidional_address search output flist", search_o_flistp);

	tmp_flistp = PIN_FLIST_ELEM_GET(search_o_flistp, PIN_FLD_RESULTS, 0, 0,	ebufp);
//	name_info_flistp = PIN_FLIST_ELEM_GET(tmp_flistp, PIN_FLD_NAMEINFO, PIN_ELEMID_ANY, 0, ebufp);
        name_info_flistp = PIN_FLIST_ELEM_GET(tmp_flistp, PIN_FLD_NAMEINFO, 1, 0, ebufp);
        install_name_info_flistp = PIN_FLIST_ELEM_GET(tmp_flistp, PIN_FLD_NAMEINFO, 2, 1, ebufp);

	if (name_info_flistp)
	{
		bldg_name = PIN_FLIST_FLD_GET(name_info_flistp,MSO_FLD_BUILDING_NAME,0,ebufp);
		if(bldg_name){
			sprintf(address, "%s, %s", address,bldg_name);
		}
		//landmark = PIN_FLIST_FLD_GET(name_info_flistp,MSO_FLD_LANDMARK,0,ebufp);
		//if(landmark){
		//	sprintf(address, "%s, %s", address,landmark);
		//}
		location = PIN_FLIST_FLD_GET(name_info_flistp,MSO_FLD_LOCATION_NAME,0,ebufp);
		if(location){
			sprintf(address, "%s, %s", address,location);
		}
		street_name = PIN_FLIST_FLD_GET(name_info_flistp,MSO_FLD_STREET_NAME,0,ebufp);
		if(street_name){
			sprintf(address, "%s, %s", address,street_name);
		}
		area = PIN_FLIST_FLD_GET(name_info_flistp,MSO_FLD_AREA_NAME,0,ebufp);
		if(area){
			sprintf(address, "%s, %s", address,area);
		}
	}
	//sprintf(address, ", %s, %s, %s, %s, %s", bldg_name,landmark, location, street_name, area);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "additional address");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, address );

        if (install_name_info_flistp)
        {
                bldg_name = PIN_FLIST_FLD_GET(install_name_info_flistp,MSO_FLD_BUILDING_NAME,0,ebufp);
                if(bldg_name){
                        sprintf(install_address, "%s, %s", install_address,bldg_name);
                }
                location = PIN_FLIST_FLD_GET(install_name_info_flistp,MSO_FLD_LOCATION_NAME,0,ebufp);
                if(location){
                        sprintf(install_address, "%s, %s", install_address,location);
                }
                street_name = PIN_FLIST_FLD_GET(install_name_info_flistp,MSO_FLD_STREET_NAME,0,ebufp);
                if(street_name){
                        sprintf(install_address, "%s, %s", install_address,street_name);
                }
                area = PIN_FLIST_FLD_GET(install_name_info_flistp,MSO_FLD_AREA_NAME,0,ebufp);
                if(area){
                        sprintf(install_address, "%s, %s", install_address,area);
                }
        }
        //sprintf(address, ", %s, %s, %s, %s, %s", bldg_name,landmark, location, street_name, area);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "install address");
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,install_address );


	PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_o_flistp, NULL);

	return;
}

void
get_helpdesk_details(
        pcm_context_t                   *ctxp,
        pin_flist_t                     *i_flistp,
	char                     	*city,
        pin_flist_t                     **ret_flistpp,
        pin_errbuf_t                    *ebufp)
{
        poid_t                          *pdp = NULL;
        poid_t                          *s_pdp = NULL;
        int64                           database;
        pin_flist_t                     *search_i_flistp = NULL;
        pin_flist_t                     *search_o_flistp = NULL;
        pin_flist_t                     *tmp_flistp     = NULL;
        int32                           s_flags = 256;
        int32                           *string_id = NULL;
        char                            *msg_template = NULL;
        int32                           str_ver;
        char                            *template = "select x from /mso_cfg_helpdesk where F1 = V1 ";


        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;

        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "get_helpdesk_details input flist", i_flistp);

        pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
        database = PIN_POID_GET_DB(pdp);

        search_i_flistp = PIN_FLIST_CREATE(ebufp);
        s_pdp = (poid_t *)PIN_POID_CREATE(database, "/search", -1, ebufp);
        PIN_FLIST_FLD_PUT(search_i_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
        PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void *)&s_flags,     ebufp);
        PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);

        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_CITY, city, ebufp);

        tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
        PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_EMAIL_ADDR, NULL, ebufp);
	PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_PHONE, NULL, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "get_helpdesk_details search input flist ", search_i_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_i_flistp, &search_o_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "get_helpdesk_details search output flist", search_o_flistp);

        tmp_flistp = PIN_FLIST_ELEM_GET(search_o_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
	if (tmp_flistp){
		*ret_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_EMAIL_ADDR, *ret_flistpp, PIN_FLD_EMAIL_ADDR, ebufp);
		PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_PHONE, *ret_flistpp, PIN_FLD_PHONE, ebufp);
	}
        PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&search_o_flistp, NULL);

        return;
}

static void
fm_mso_get_purchase_prod_details(
        pcm_context_t    *ctxp,
        poid_t           *act_pdp,
        pin_flist_t      **rt_flistp,
        pin_errbuf_t     *ebufp)
{
        pin_flist_t     *srch_iflistp=NULL;
        pin_flist_t     *srch_oflistp=NULL;
        pin_flist_t     *args_flistp=NULL;
        pin_flist_t     *sub_flistp=NULL;
        int              product_status = 1;

        char    *template="select X from /purchased_product 1, /mso_purchased_plan 2 where 1.F1 = V1 and 1.F2 = V2 and descr like '%SUBSCRIPTION' and 2.F3 = 1.F1 and 2.F4 = V4 and 1.F5 = 2.F6";
        int64   db = 1;
        int32   srch_flag = 256;

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in before searching prov tag", ebufp);
        }
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_purchase_prod_details ");

        srch_iflistp= PIN_FLIST_CREATE(ebufp);

        PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, template, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, act_pdp, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &product_status, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, act_pdp, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 4, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DESCR, "base plan", ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 5, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, NULL, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 6, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, NULL, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS,PIN_ELEMID_ANY, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_purchase_prod_details input flist is : ", srch_iflistp);

        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_purchase_prod_details output flist is : ", srch_oflistp);

        sub_flistp = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_purchase_prod_details sub flist is : ", sub_flistp);

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_purchase_prod_details 1");

       *rt_flistp = PIN_FLIST_COPY(sub_flistp,ebufp);
//            *ret_flistp = srch_oflistp ;
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_purchase_prod_details 2");
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_purchase_prod_details sub flist is : ", *rt_flistp);

        PIN_FLIST_DESTROY_EX(&srch_iflistp,NULL);
        PIN_FLIST_DESTROY_EX(&srch_oflistp,NULL);
        return;
}
