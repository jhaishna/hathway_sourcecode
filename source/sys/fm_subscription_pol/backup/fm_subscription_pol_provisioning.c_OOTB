/*******************************************************************
 * Copyright (c) 1998, 2009, Oracle and/or its affiliates.All rights reserved. 
 *      
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static	char	Sccs_id[] = "@(#)$Id: fm_subscription_pol_provisioning.c /cgbubrm_7.3.2.rwsmod/2 2009/03/24 07:02:25 amamidi Exp $";
#endif

#include <stdio.h>	/* for FILE * in pcm.h */
#include "ops/cust.h"
#include "ops/bill.h"
#include "ops/subscription.h"
#include "pin_flds.h"
#include <string.h>
#include "pcm.h"
#include "cm_fm.h"
#include "pin_bill.h"
#include "pin_channel.h"
#include "fm_utils.h"
#include "fm_bill_utils.h"
#include "pinlog.h"
#include "pin_teracomm.h"
#include "pin_msexchange.h"
#include "pin_gsm.h"


/********************************************************************
 * contains:
 *	PCM_OP_BILL_POL_PURCHASE_PROD_PROVISIONING
 *	PCM_OP_BILL_POL_CANCEL_PROD_PROVISIONING
 *	PCM_OP_BILL_POL_GET_PROD_PROVISIONING_TAGS
 *	the config table for the above.
 *	the actual customizing functions.
 *
 * CONFIGURATION
 *
 * to add new service type:
 * 1. add the declaration of the function(s) below.
 * 2. add a new entry in the service_info table.
 * 3. add the actual function(s) down in the section "PROVISIONING FUNCTIONS".
 *
 * do NOT change anything else.
 *
 *******************************************************************/


/* Handy wireless constants--you shouldn't have to change these */
#define WAPCONFIG	        "/config/wap"
#define WAPSERVICE	        "/service/wap/interactive"
#define WAPSERVICEPROFILE	"/service/wap"
#define GPRSCONFIG	        "/config/gprs"
#define GPRSSERVICE	        "/service/ip/gprs"
#define GPRSSERVICEPROFILE      "/service/ip/gprs"

/* Handy content constants--you shouldn't have to change these */
#define CONTENTCONFIG	        "/config/content"
#define CONTENTSERVICE	        "/service/content"
#define CONTENTSERVICEPROFILE   "/service/content"

/* Handy telco constants--you shouldn't have to change these */
#define TELCO_CONFIG 		"/config/telco"
#define TELCO_GSM_CONFIG 	"/config/telco/gsm"
#define TELCO_GSM_DATA_CONFIG	"/config/telco/gsm/data"
#define TELCO_GSM_FAX_CONFIG	"/config/telco/gsm/fax"
#define TELCO_GSM_SMS_CONFIG	"/config/telco/gsm/sms"
#define TELCO_GSM_TELEPHONY_CONFIG	"/config/telco/gsm/telephony"
#define TELCO_SERVICE		"/service/telco"
#define TELCO_GSM_SERVICE	"/service/telco/gsm"
#define TELCO_GSM_DATA_SERVICE	"/service/telco/gsm/data"
#define TELCO_GSM_FAX_SERVICE	"/service/telco/gsm/fax"
#define TELCO_GSM_SMS_SERVICE	"/service/telco/gsm/sms"
#define TELCO_GSM_TELEPHONY_SERVICE	"/service/telco/gsm/telephony"
#define TELCO_SERVICE_PROFILE 	"/service/telco"
#define TELCO_GSM_SERVICE_PROFILE 	"/service/telco/gsm"
#define TELCO_GSM_DATA_SERVICE_PROFILE	"/service/telco/gsm/data"
#define TELCO_GSM_FAX_SERVICE_PROFILE	"/service/telco/gsm/fax"
#define TELCO_GSM_SMS_SERVICE_PROFILE	"/service/telco/gsm/sms"
#define TELCO_GSM_TELEPHONY_SERVICE_PROFILE	"/service/telco/gsm/telephony"

/* Handy ssg constants--you shouldn't have to change these */
#define SSGCONFIG	        "/config/ssg/tags"
#define SSGSERVICE	        "/service/ssg"
#define SSGSERVICEPROFILE   	"/service/ssg"
#define VPDNSERVICE	        "/service/vpdn"
#define IPSERVICE	        "/service/ip"

/* Handy pdc constants--you shouldn't have to change these */  
#define TELCO_PDC_CONFIG                "/config/telco/pdc"
#define TELCO_PDC_SERVICE               "/service/telco/pdc"
#define TELCO_PDC_SERVICE_PROFILE       "/service/telco/pdc"

/* Handy imt constants--you shouldn't have to change these */
#define TELCO_IMT_CONFIG                "/config/telco/imt"
#define TELCO_IMT_SERVICE               "/service/telco/imt"
#define TELCO_IMT_SERVICE_PROFILE       "/service/telco/imt"

/* Handy telco gprs constants--you shouldn't have to change these */
#define TELCO_GPRS_CONFIG                "/config/telco/gprs"
#define TELCO_GPRS_SERVICE               "/service/telco/gprs"
#define TELCO_GPRS_SERVICE_PROFILE       "/service/telco/gprs"

#define MAX_SIZE                256
/********************************************************************
 * forward declare the actual provisioning functions here, so they
 * may be listed in the table below.
 * Their prototype should be:
 * void function(
 *      pcm_context_t	*ctxp,		- context for reads/writes to database
 *      poid_t		*svc_obj_p,	- (read only) POID of service obj
 *      int32		buy,		- TRUE if purchase, else cancel
 *      char            *tag,		- provisioning tag
 *      pin_errbuf_t	*ebufp)		- for return of errors
 *******************************************************************/
static void plp_example(pcm_context_t *ctxp, poid_t *svc_obj_p, int32 buy,
	char *tag, pin_errbuf_t *ebp);
static void plp_mail_regular(pcm_context_t *ctxp, poid_t *svc_obj_p, int32 buy,
	char *tag, pin_errbuf_t *ebp);
static void plp_mail_jumbo(pcm_context_t *ctxp, poid_t *svc_obj_p, int32 buy,
	char *tag, pin_errbuf_t *ebp);
static void plp_ssg(pcm_context_t *ctxp, poid_t *svc_obj_p, int32 buy,
	char *tag, pin_errbuf_t *ebp);
static void plp_ip(pcm_context_t *ctxp, poid_t *svc_obj_p, int32 buy,
	char *tag, pin_errbuf_t *ebp);
static void plp_email(pcm_context_t *ctxp, poid_t *svc_obj_p, int32 buy,
	char *tag, pin_errbuf_t *ebp);
static void plp_teracomm(pcm_context_t *ctxp, poid_t *svc_obj_p, int32 buy,
	char *tag, pin_errbuf_t *ebp);
static void plp_wireless_wap(pcm_context_t *ctxp, poid_t *svc_obj_p, int32 buy,
	char *tag, pin_errbuf_t *ebp);
static void plp_wireless_gprs(pcm_context_t *ctxp, poid_t *svc_obj_p, int32 buy,
	char *tag, pin_errbuf_t *ebp);
static void plp_msexchange(pcm_context_t *ctxp, poid_t *svc_obj_p, int32 buy,
	char *tag, pin_errbuf_t *ebp);
static void plp_telephony(pcm_context_t *ctxp, poid_t *svc_obj_p, int32 buy,
        char *tag, pin_errbuf_t *ebp);
static void plp_dsl(pcm_context_t *ctxp, poid_t *svc_obj_p, int32 buy,
        char *tag, pin_errbuf_t *ebp);
static void plp_gsm_telephony(pcm_context_t *ctxp, poid_t *svc_obj_p, int32 buy,
	char *tag, pin_errbuf_t *ebp);
static void plp_gsm_sms(pcm_context_t *ctxp, poid_t *svc_obj_p, int32 buy,
	char *tag, pin_errbuf_t *ebp);
static void plp_gsm_data(pcm_context_t *ctxp, poid_t *svc_obj_p, int32 buy,
	char *tag, pin_errbuf_t *ebp);
static void plp_gsm_fax(pcm_context_t *ctxp, poid_t *svc_obj_p, int32 buy,
	char *tag, pin_errbuf_t *ebp);


static void plp_tcf(pcm_context_t *ctxp, poid_t *svc_obj_p, int32 buy,
	char *tag, pin_errbuf_t *ebp);
static void plp_gsm_tcf(pcm_context_t *ctxp, poid_t *svc_obj_p, int32 buy,
	char *tag, pin_errbuf_t *ebp);
static void plp_wireless_content(
	pcm_context_t *ctxp,
	poid_t *svc_obj_p,
	int32 buy,
	char *tag,
	pin_errbuf_t *ebp);
static void plp_bel(pcm_context_t *ctxp, poid_t *svc_obj_p, int32 buy,
	char *tag, pin_errbuf_t *ebp);
static void tag_fn_get_tags(pcm_context_t *ctxp, poid_t *svc_poidp, 
	pin_flist_t *r_flp, int32 *eidp, pin_errbuf_t *ebufp, 
	char *configtype, char *servicetype);
static void plp_pdc_tcf(pcm_context_t *ctxp, poid_t *svc_obj_p, 
	int32 buy, char *tag, pin_errbuf_t *ebp);
static void plp_imt_tcf(pcm_context_t *ctxp, poid_t *svc_obj_p, 
	int32 buy, char *tag, pin_errbuf_t *ebp);
static void plp_gprs_tcf(pcm_context_t *ctxp, poid_t *svc_obj_p,
        int32 buy, char *tag, pin_errbuf_t *ebp);




static void fm_subscription_pol_get_conf_provisioning_tags(
	pcm_context_t	*ctxp,
	poid_t		*in_pdp,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp);

/********************************************************************
 * forward declare the provisioning tag collection functions here,
 * so they may be listed in the table below.
 * Their prototype should be:
 * void function(
 *      pcm_context_t *ctxp, - context for reads/writes to database
 *      poid_t	      *svc_obj_p, - (read only) POID of service obj
 *      pin_flist_t   *r_flistp, - the output flist for opcode
 *                          PCM_OP_BILL_POL_GET_PROD_PROVISIONING_TAGS
 *      int32         *eidp, - current element ID in flist array
 *                              PIN_FLD_PROD_PROVISIONING_TAGS---this
 *                              function must increment this with the
 *                              number of elements added to the array
 *      pin_errbuf_t  *ebufp); - for return of errors
 *******************************************************************/
static void tag_fn_ip(pcm_context_t *ctxp, poid_t *svc_obj_p,
	pin_flist_t *r_flistp, int32 *eidp, pin_errbuf_t *ebufp);
static void tag_fn_vpdn(pcm_context_t *ctxp, poid_t *svc_obj_p,
	pin_flist_t *r_flistp, int32 *eidp, pin_errbuf_t *ebufp);
static void tag_fn_ssg(pcm_context_t *ctxp, poid_t *svc_obj_p,
	pin_flist_t *r_flistp, int32 *eidp, pin_errbuf_t *ebufp);
static void tag_fn_email(pcm_context_t *ctxp, poid_t *svc_obj_p,
	pin_flist_t *r_flistp, int32 *eidp, pin_errbuf_t *ebufp);
static void tag_fn_wap(pcm_context_t *ctxp, poid_t *svc_obj_p,
	pin_flist_t *r_flistp, int32 *eidp, pin_errbuf_t *ebufp);
static void tag_fn_gprs(pcm_context_t *ctxp, poid_t *svc_obj_p,
	pin_flist_t *r_flistp, int32 *eidp, pin_errbuf_t *ebufp);
static void tag_fn_teracomm(pcm_context_t *ctxp, poid_t *svc_obj_p, 
	pin_flist_t *r_flistp, int32 *eidp, pin_errbuf_t *ebufp);
static void tag_fn_wireless(pcm_context_t *ctxp, poid_t *svc_obj_p,
	pin_flist_t *r_flistp, int32 *eidp, pin_errbuf_t *ebufp, 
	char *configtype, char *svctype);
static void tag_fn_telco(pcm_context_t *ctxp, poid_t *svc_obj_p,
	pin_flist_t *r_flistp, int32 *eidp, pin_errbuf_t *ebufp, 
	char *configtype, char *svctype);
static void tag_fn_msexchange_user(pcm_context_t *ctxp, poid_t *svc_obj_p, 
	pin_flist_t *r_flistp, int32 *eidp, pin_errbuf_t *ebufp);
static void tag_fn_msexchange_firstadmin(pcm_context_t *ctxp, poid_t *svc_obj_p, 
	pin_flist_t *r_flistp, int32 *eidp, pin_errbuf_t *ebufp);
static void tag_fn_telephony(pcm_context_t *ctxp, poid_t *svc_obj_p,
        pin_flist_t *r_flistp, int32 *eidp, pin_errbuf_t *ebufp);
static void tag_fn_dsl(pcm_context_t *ctxp, poid_t *svc_obj_p,
        pin_flist_t *r_flistp, int32 *eidp, pin_errbuf_t *ebufp);
static void tag_fn_gsm_telephony(pcm_context_t *ctxp, poid_t *svc_obj_p,
	pin_flist_t *r_flistp, int32 *eidp, pin_errbuf_t *ebufp);
static void tag_fn_gsm_sms(pcm_context_t *ctxp, poid_t *svc_obj_p,
	pin_flist_t *r_flistp, int32 *eidp, pin_errbuf_t *ebufp);
static void tag_fn_gsm_fax(pcm_context_t *ctxp, poid_t *svc_obj_p,
	pin_flist_t *r_flistp, int32 *eidp, pin_errbuf_t *ebufp);
static void tag_fn_gsm_data(pcm_context_t *ctxp, poid_t *svc_obj_p,
	pin_flist_t *r_flistp, int32 *eidp, pin_errbuf_t *ebufp);
static void tag_fn_gsm( pcm_context_t *ctxp, poid_t *svc_obj_p,
	pin_flist_t *r_flistp, int32 *eidp, char    *service_recd,
	char    *config_recd, pin_errbuf_t *ebufp);


static void tag_fn_tcf(pcm_context_t *ctxp, poid_t *svc_obj_p,
	pin_flist_t *r_flistp, int32 *eidp, pin_errbuf_t *ebufp);
static void tag_fn_gsm_tcf(pcm_context_t *ctxp, poid_t *svc_obj_p,
	pin_flist_t *r_flistp, int32 *eidp, pin_errbuf_t *ebufp);
static void tag_fn_gsm_data_tcf(pcm_context_t *ctxp, poid_t *svc_obj_p,
	pin_flist_t *r_flistp, int32 *eidp, pin_errbuf_t *ebufp);
static void tag_fn_gsm_fax_tcf(pcm_context_t *ctxp, poid_t *svc_obj_p,
	pin_flist_t *r_flistp, int32 *eidp, pin_errbuf_t *ebufp);
static void tag_fn_gsm_sms_tcf(pcm_context_t *ctxp, poid_t *svc_obj_p,
	pin_flist_t *r_flistp, int32 *eidp, pin_errbuf_t *ebufp);
static void tag_fn_gsm_telephony_tcf(pcm_context_t *ctxp, poid_t *svc_obj_p,
	pin_flist_t *r_flistp, int32 *eidp, pin_errbuf_t *ebufp);
static void tag_fn_content(pcm_context_t *ctxp, poid_t *svc_obj_p,
	pin_flist_t *r_flistp, int32 *eidp, pin_errbuf_t *ebufp);
static void tag_fn_bel(pcm_context_t *ctxp, poid_t *svc_obj_p, 
	pin_flist_t *r_flistp, int32 *eidp, pin_errbuf_t *ebufp);
static void tag_fn_pdc_tcf(pcm_context_t *ctxp, poid_t *svc_obj_p, 
	pin_flist_t *r_flistp, int32 *eidp, pin_errbuf_t *ebufp);
static void tag_fn_imt_tcf(pcm_context_t *ctxp, poid_t *svc_obj_p,
	pin_flist_t *r_flistp, int32 *eidp, pin_errbuf_t *ebufp);
static void tag_fn_gprs_tcf(pcm_context_t *ctxp, poid_t *svc_obj_p,
        pin_flist_t *r_flistp, int32 *eidp, pin_errbuf_t *ebufp);

/********************************************************************
 * forward declare the provisioning tag validation functions here,
 * so they may be listed in the table below.
 * Their prototype should be:
 * int32 function(
 *      pcm_context_t *ctxp, - context for reads/writes to database
 *      poid_t	      *svc_obj_p, - (read only) POID of service obj
 *	char          *tag); - tag to be checked for validity
 * These functions return PIN_BOOLEAN_FALSE if the tag is invalid,
 * PIN_BOOLEAN_TRUE if the tag is valid
 *******************************************************************/
static int32 valid_tag_ip(pcm_context_t *ctxp, poid_t *svc_obj_p,
	char *tag);
static int32 valid_tag_vpdn(pcm_context_t *ctxp, poid_t *svc_obj_p,
	char *tag);
static int32 valid_tag_ssg(pcm_context_t *ctxp, poid_t *svc_obj_p,
	char *tag);
static int32 valid_tag_email(pcm_context_t *ctxp, poid_t *svc_obj_p,
	char *tag);
static int32 valid_tag_teracomm(pcm_context_t *ctxp, poid_t *svc_obj_p,
	char *tag);
static int32 valid_tag_wap(pcm_context_t *ctxp, poid_t *svc_obj_p,
	char *tag);
static int32 valid_tag_gprs(pcm_context_t *ctxp, poid_t *svc_obj_p,
	char *tag);
static int32 valid_tag_wireless(pcm_context_t *ctxp, poid_t *svc_obj_p,
	char *tag, char *type);
static int32 valid_tag_msexchange(pcm_context_t *ctxp, poid_t *svc_obj_p,
	char *tag);
static int32 valid_tag_telephony(pcm_context_t *ctxp, poid_t *svc_obj_p,
        char *tag);
static int32 valid_tag_dsl(pcm_context_t *ctxp, poid_t *svc_obj_p,
        char *tag);
static int32 valid_tag_content(pcm_context_t *ctxp, poid_t *svc_obj_p,
        char *tag);
static int32 valid_tag_bel(pcm_context_t *ctxp, poid_t *svc_obj_p,
        char *tag);
static int32 valid_tag_gsm_telephony(pcm_context_t *ctxp, poid_t *svc_obj_p,
	char *tag);
static int32 valid_tag_gsm_sms(pcm_context_t *ctxp, poid_t *svc_obj_p,
	char *tag);
static int32 valid_tag_gsm_fax(pcm_context_t *ctxp, poid_t *svc_obj_p,
	char *tag);
static int32 valid_tag_gsm_data(pcm_context_t *ctxp, poid_t *svc_obj_p,
	char *tag);
static int32 valid_tag_gsm( pcm_context_t *ctxp, poid_t *svc_obj_p,
	char *tag, char *config_recd);

static int32 valid_tag_tcf(pcm_context_t *ctxp, poid_t *svc_obj_p,
	char *tag);
static int32 valid_tag_gsm_tcf(pcm_context_t *ctxp, poid_t *svc_obj_p,
	char *tag);
static int32 valid_tag_gsm_data_tcf(pcm_context_t *ctxp, poid_t *svc_obj_p,
	char *tag);
static int32 valid_tag_gsm_fax_tcf(pcm_context_t *ctxp, poid_t *svc_obj_p,
	char *tag);
static int32 valid_tag_gsm_sms_tcf(pcm_context_t *ctxp, poid_t *svc_obj_p,
	char *tag);
static int32 valid_tag_gsm_telephony_tcf(pcm_context_t *ctxp, poid_t *svc_obj_p,
	char *tag);
static int32 valid_tag_tcf_telco( pcm_context_t *ctxp, poid_t *svc_obj_p,
	char *tag, char *config_recd);
static int32 valid_tag_pdc_tcf(pcm_context_t *ctxp, poid_t *svc_obj_p,
	 char *tag);
static int32 valid_tag_imt_tcf(pcm_context_t *ctxp, poid_t *svc_obj_p, 
	char *tag);
static int32 valid_tag_gprs_tcf(pcm_context_t *ctxp, poid_t *svc_obj_p,
         char *tag);

static int32 isValidSSGTag(char	*provTag);
static struct service_info {
	char  *service;
	void  (*plp_func)(pcm_context_t *, poid_t *, int32, char *,
		pin_errbuf_t *);
	void  (*tag_func)(pcm_context_t *, poid_t *, pin_flist_t *, 
		int32 *, pin_errbuf_t *);
	int32 (*valid_func)(pcm_context_t *, poid_t *, char *);
} service_info[] = {
	{ "/service/ip",    plp_ip,    tag_fn_ip,     valid_tag_ip },
	{ "/service/vpdn",  plp_ssg,   tag_fn_vpdn,   valid_tag_vpdn },
	{ "/service/ssg",   plp_ssg,   tag_fn_ssg,    valid_tag_ssg },
	{ "/service/email", plp_email, tag_fn_email,  valid_tag_email },
	{ "/service/ip/cable/teracomm", 
		plp_teracomm, tag_fn_teracomm,  valid_tag_teracomm },
	{ CONTENTSERVICE, plp_wireless_content, tag_fn_content,  valid_tag_content },
	{ WAPSERVICE, plp_wireless_wap, tag_fn_wap,  valid_tag_wap },
	{ GPRSSERVICE, plp_wireless_gprs, tag_fn_gprs,  valid_tag_gprs },
	{ PIN_MSEXCHANGE_SERVICE_TYPE_USER, 
		plp_msexchange, tag_fn_msexchange_user,  valid_tag_msexchange },
	{ PIN_MSEXCHANGE_SERVICE_TYPE_FIRSTADMIN, 
		plp_msexchange, tag_fn_msexchange_firstadmin,  valid_tag_msexchange },
	 { "/service/telephony",
                plp_telephony, tag_fn_telephony, valid_tag_telephony },
        { "/service/dsl",
                plp_dsl, tag_fn_dsl, valid_tag_dsl },
	{ "/service/video",
                plp_bel, tag_fn_bel, valid_tag_bel },
	{ "/service/gsm/telephony",
		plp_gsm_telephony, tag_fn_gsm_telephony, valid_tag_gsm_telephony },
	{ "/service/gsm/sms",
		plp_gsm_sms, tag_fn_gsm_sms, valid_tag_gsm_sms },
	{ "/service/gsm/data",
		plp_gsm_data, tag_fn_gsm_data, valid_tag_gsm_data },
	{ "/service/gsm/fax",
		plp_gsm_fax, tag_fn_gsm_fax, valid_tag_gsm_fax },
    { TELCO_SERVICE, plp_tcf, tag_fn_tcf, valid_tag_tcf },
    { TELCO_GSM_SERVICE, plp_gsm_tcf, tag_fn_gsm_tcf, valid_tag_gsm_tcf },
    { TELCO_GSM_DATA_SERVICE, plp_gsm_tcf, tag_fn_gsm_data_tcf, valid_tag_gsm_data_tcf },
    { TELCO_GSM_FAX_SERVICE, plp_gsm_tcf, tag_fn_gsm_fax_tcf, valid_tag_gsm_fax_tcf },
    { TELCO_GSM_SMS_SERVICE, plp_gsm_tcf, tag_fn_gsm_sms_tcf, valid_tag_gsm_sms_tcf },
    { TELCO_GSM_TELEPHONY_SERVICE, plp_gsm_tcf, tag_fn_gsm_telephony_tcf, valid_tag_gsm_telephony_tcf },
    { TELCO_PDC_SERVICE, plp_pdc_tcf, tag_fn_pdc_tcf, valid_tag_pdc_tcf },
    { TELCO_IMT_SERVICE, plp_imt_tcf, tag_fn_imt_tcf, valid_tag_imt_tcf },
    {TELCO_GPRS_SERVICE, plp_gprs_tcf, tag_fn_gprs_tcf, valid_tag_gprs_tcf },
    { NULL, NULL, NULL, NULL }    /* MUST BE LAST! */
};
static char *tags_ip[] = {
	"example",
	NULL    /* MUST BE LAST! */
};

static char *tags_vpdn[] = {
	NULL    /* MUST BE LAST! */
};

static char *tags_email[] = {
	"regular",
	"jumbo",
	NULL    /* MUST BE LAST! */
};

extern void plp_link_to_ssg(pcm_context_t *ctxp, poid_t *svc_obj_p,
	int32 buy, char *ssg_svc_name, pin_errbuf_t *ebp);
extern void plp_link_to_service_profile(pcm_context_t *ctxp, poid_t *svc_obj_p,
	int32 buy, char *svc_profile_type, char *svc_profile_login, 
	pin_errbuf_t *ebp);
extern void plp_link_to_content_profile(pcm_context_t *ctxp, poid_t *svc_obj_p,
	int32 buy, char *content_tag, pin_errbuf_t *ebp);

/********************************************************************
 * END CONFIGURATION
 *******************************************************************/

/********************************************************************
 * UTILITIES FOR OPCODE FUNCTIONS - leave these alone
 *******************************************************************/
static struct service_info *
_get_service_info(
	char *service
	)
{
	struct service_info *sp;
	char   epb[256];

	for (sp = service_info; sp->service; sp++) {
		if (strcmp(service, sp->service) == 0) {
			return(sp);
		}
	}
	/* couldn't find it */
	sprintf(epb, "can't locate service info for service type %s",
		service);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_WARNING, epb);
	return(NULL);
}

static void
_subscription_pol_provisioning_op_check(
	pcm_context_t		*ctxp,
	int32			opcode,
	int32			my_opcode,
	char			*opname,
	pin_flist_t		*i_flistp,
	struct service_info     **svcpp,
	poid_t			**sppp,		/* service poid ptr ptr */
	pin_errbuf_t		*ebufp)
{
	poid_t			*pp;
	pin_flist_t		*flistp = (pin_flist_t *)NULL;
	pin_flist_t		*r_flistp = (pin_flist_t *)NULL;
	char			epb[256];
	char			*ptstr;
	struct service_info     *svcp;
	char			*tag;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
		/******/
	}

	PIN_ERR_CLEAR_ERR(ebufp);

	if (opcode != my_opcode) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		sprintf(epb, "%s: opcode error", opname);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, epb, ebufp);
		return;
		/*****/
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	sprintf(epb, "%s: input flist", opname);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, epb, i_flistp);

	if (opcode == PCM_OP_BILL_POL_GET_PROD_PROVISIONING_TAGS) {
		return;
		/******/
	}

	/***********************************************************
	 * for purchase or cancel, check iff poid is of type "service"
	 ***********************************************************/
	pp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
		sprintf(epb, "%s: POID get error", opname);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, epb, ebufp);
		return;
		/*****/
	}
	if (strncmp(PIN_POID_GET_TYPE(pp), "/service", strlen("/service"))) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_TYPE, 0, 0, opcode);
		sprintf(epb, "%s: poid type not \"/service...\"", opname);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, epb, ebufp);
		return;
		/*****/
	}

	/***********************************************************
	 * see if we have a tag.
	 ***********************************************************/
	tag = (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROVISIONING_TAG,
	    0, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
		sprintf(epb, "%s: PROVISIONING_TAG get error", opname);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, epb, ebufp);
		return;
		/*****/
	}

	/***********************************************************
	 * do robj of the service obj, which:
	 * (a) verifies that it exists, and
	 * (b) allows the verification of the type.
	 ***********************************************************/
	flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, pp, ebufp);

	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, flistp, &r_flistp, ebufp);

	/*
	 * NOTE "TAKE" of this POID, so we can return it.
	 */
	pp = (poid_t *)PIN_FLIST_FLD_TAKE(r_flistp, PIN_FLD_POID, 0, ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		sprintf(epb, "%s: bad rflds for poid type", opname);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, epb, ebufp);
		goto cleanup;
		/***********/
	}

	ptstr = (char *)PIN_POID_GET_TYPE(pp);
	if (ptstr == (char *)NULL) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_NULL_PTR, PIN_FLD_POID, 0, opcode);
		sprintf(epb, "%s: got NULL poid type ptr", opname);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, epb, ebufp);
		goto cleanup;
		/***********/
	}

	/***********************************************************
	 * lookup service/tag.
	 ***********************************************************/
	if ((svcp = _get_service_info(ptstr)) != NULL
			&& (*(svcp->valid_func))(ctxp, pp, tag) 
					!= PIN_BOOLEAN_FALSE) {
		/* all is well */
		PIN_ERR_CLEAR_ERR(ebufp);
		*svcpp = svcp;
		*sppp = pp;
		goto cleanup;
		/************/
	}
	/*
	 * if not found, set PIN_ERR_NOT_FOUND
	 */
	pin_set_err(ebufp, PIN_ERRLOC_FM,
		PIN_ERRCLASS_SYSTEM_DETERMINATE,
		PIN_ERR_NOT_FOUND, PIN_FLD_PROVISIONING_TAG, 0, opcode);
	*svcpp = (struct service_info *)NULL;
	*sppp = (poid_t *)NULL;
	PIN_POID_DESTROY(pp, NULL);

cleanup:
	PIN_FLIST_DESTROY(flistp, NULL);
	PIN_FLIST_DESTROY(r_flistp, NULL);

	return;
}

/*
 * Notify the system channel object that a service object
 * was modified.  Ignore errors from it.
 */
void
_subscription_pol_provisioning_channel_push(
	pcm_context_t		*ctxp,
	int32			flags,
	poid_t			*svc_obj_p)
{
	pin_flist_t	*flistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	poid_t		*ch_pdp = NULL;
	poid_t		*sup_pdp = NULL;
	int64		db_id;
	pin_errbuf_t	ebf;

	PIN_ERR_CLEAR_ERR(&ebf);
	db_id = PIN_POID_GET_DB(svc_obj_p);
	ch_pdp = PIN_POID_CREATE(db_id, "/channel", PIN_CHANNEL_SERVICE_MODIFY,
		&ebf);
	/*
	 * use the event for product purchase/cancel as supplier obj.
	 */
	sup_pdp = PIN_POID_CREATE(db_id, PIN_OBJ_TYPE_EVENT_PRODUCT, -1, &ebf);

	flistp = PIN_FLIST_CREATE(&ebf);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)ch_pdp, &ebf);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_SUPPLIER_OBJ, (void *)sup_pdp, &ebf);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_SOURCE_OBJ, (void *) svc_obj_p, &ebf);

	fm_utils_channel_push(ctxp, flags, flistp, &r_flistp, &ebf);

	PIN_FLIST_DESTROY(flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

	if(PIN_ERR_IS_ERR(&ebf)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		    "_subscription_pol_provisioning_channel_push error", &ebf);
	}
	return;
}

/********************************************************************
 * OPCODE FUNCTIONS - leave these alone
 *******************************************************************/
/*
 * Purchase
 */
EXPORT_OP void
op_subscription_pol_purchase_prod_provisioning(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,		/*ARGSUSED*/
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	struct service_info	*svcp;
	poid_t			*svc_obj_p;
	pin_flist_t		*r_flistp = (pin_flist_t *)NULL;
	int32			err;
	char			*tag;
	char			epb[256];

	_subscription_pol_provisioning_op_check(ctxp, opcode,
	    PCM_OP_BILL_POL_PURCHASE_PROD_PROVISIONING,
	    "op_subscription_pol_purchase_prod_provisioning", i_flistp,
	    &svcp, &svc_obj_p, ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
		/******/
	}

	tag = (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROVISIONING_TAG,
	    0, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
		sprintf(epb, "%s: PROVISIONING_TAG get error", 
			"op_subscription_pol_purchase_prod_provisioning");
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, epb, ebufp);
		return;
		/*****/
	}

	/***********************************************************
	 * invoke function with "buy" set
	 ***********************************************************/
	(*(svcp->plp_func))(ctxp, svc_obj_p, PIN_BOOLEAN_TRUE, tag, ebufp);

	if (! PIN_ERR_IS_ERR(ebufp)) {
		_subscription_pol_provisioning_channel_push(ctxp, flags, svc_obj_p);
	}
	err = ebufp->pin_err;
	r_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(r_flistp, PIN_FLD_POID, svc_obj_p, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_RESULT, &err, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	    "op_subscription_pol_purchase_prod_provisioning output", r_flistp);
	*o_flistpp = r_flistp;
	return;
}

/*
 * Cancel
 */
EXPORT_OP void
op_subscription_pol_cancel_prod_provisioning(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,		/*ARGSUSED*/
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	struct service_info	*svcp;
	poid_t			*svc_obj_p;
	pin_flist_t		*r_flistp = (pin_flist_t *)NULL;
	int32			err;
	char			*tag;
	char			epb[256];

	_subscription_pol_provisioning_op_check(ctxp, opcode,
	    PCM_OP_BILL_POL_CANCEL_PROD_PROVISIONING,
	    "op_subscription_pol_cancel_prod_provisioning", i_flistp,
	    &svcp, &svc_obj_p, ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
		/******/
	}

	tag = (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROVISIONING_TAG,
	    0, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
		sprintf(epb, "%s: PROVISIONING_TAG get error", 
			"op_subscription_pol_cancel_prod_provisioning");
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, epb, ebufp);
		return;
		/*****/
	}

	/***********************************************************
	 * invoke function with "buy" clear
	 ***********************************************************/
	(*(svcp->plp_func))(ctxp, svc_obj_p, PIN_BOOLEAN_FALSE, tag, ebufp);

	if (! PIN_ERR_IS_ERR(ebufp)) {
		_subscription_pol_provisioning_channel_push(ctxp, flags, svc_obj_p);
	}
	err = ebufp->pin_err;
	r_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(r_flistp, PIN_FLD_POID, svc_obj_p, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_RESULT, &err, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	    "op_subscription_pol_cancel_prod_provisioning output", r_flistp);
	*o_flistpp = r_flistp;
	return;
}

/*
 * Get list on known service/tags pairs
 */
EXPORT_OP void
op_subscription_pol_get_prod_provisioning_tags(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,		/*ARGSUSED*/
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	poid_t			*pp;
	pin_flist_t		*r_flistp = (pin_flist_t *)NULL;
	pin_flist_t		*pt_flistp = (pin_flist_t *)NULL;
	pin_flist_t		*res_flistp = (pin_flist_t *)NULL;
	pin_flist_t		*per_flistp = (pin_flist_t *)NULL;
	pin_flist_t		*tmp_flistp = (pin_flist_t *)NULL;

	char			*name = NULL;
	char			*permitted = NULL;
	int32			eid = 0;
	int32			resId = 0;
	int32			perId = 0;
	pin_cookie_t		res_cookie = NULL;
	pin_cookie_t		per_cookie = NULL;
	struct service_info	*svcp;

	/***********************************************************
	 * check opcode, errs
	 ***********************************************************/
	_subscription_pol_provisioning_op_check(ctxp, opcode,
	    PCM_OP_BILL_POL_GET_PROD_PROVISIONING_TAGS,
	    "op_subscription_pol_get_prod_provisioning_tags", i_flistp,
	    (struct service_info **)NULL, (poid_t **)NULL, ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
		/******/
	}

	/***********************************************************
	 * create output flist
	 ***********************************************************/
	r_flistp = PIN_FLIST_CREATE(ebufp);
	pp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, pp, ebufp);

	/*
	 * call each service type's tag collection function to build
	 * a list of provisioning tags
	 */
	eid = 0;
	for (svcp = service_info; svcp->service; svcp++) {
		/* add this service type's tags to the output flist */
		/* eid is incremented in the tag_func */
		(*(svcp->tag_func))(ctxp, pp, r_flistp, &eid, ebufp);
	}

	fm_subscription_pol_get_conf_provisioning_tags(ctxp, 
		pp, &pt_flistp, ebufp);	

	while ((res_flistp = PIN_FLIST_ELEM_GET_NEXT(pt_flistp,
		PIN_FLD_RESULTS, &resId, 1, &res_cookie, ebufp)) 
		!= (pin_flist_t *)NULL) {

		name = (char *)PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_NAME, 0,
			ebufp);
		if (name && strcmp(name, "__DEFAULT__") == 0) {
			continue;
		}

		perId = 0;
		tmp_flistp = NULL;
		per_cookie = NULL;
		while ((per_flistp = PIN_FLIST_ELEM_GET_NEXT(res_flistp,
			PIN_FLD_PERMITTEDS, &perId, 1, &per_cookie, ebufp))
			!= (pin_flist_t *)NULL) {

			permitted = (char *)PIN_FLIST_FLD_GET(per_flistp,
				PIN_FLD_PERMITTED, 0, ebufp);

			tmp_flistp = PIN_FLIST_ELEM_ADD(r_flistp,
				PIN_FLD_PROD_PROVISIONING_TAGS, 
				PIN_FLIST_ELEM_COUNT(r_flistp,
				PIN_FLD_PROD_PROVISIONING_TAGS, ebufp),
				ebufp);
			PIN_FLIST_FLD_SET(tmp_flistp,
				PIN_FLD_PROVISIONING_TAG, name, ebufp);
			PIN_FLIST_FLD_SET(tmp_flistp,
				PIN_FLD_TYPE_STR, permitted, ebufp);
		}
	}

	PIN_FLIST_DESTROY_EX(&pt_flistp, NULL);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	    "op_subscription_pol_get_prod_provisioning_tags output", r_flistp);
	*o_flistpp = r_flistp;
	return;
}

/*
 * Generic tag checking function - see if tag exists in array of strings
 */
static int32
_valid_tag(char *i_tag, char **tag_array)
{
	char **tagp;
	char *tag;

	if (i_tag == NULL) {
		return(PIN_BOOLEAN_FALSE);
	}

	for (tagp = tag_array; *tagp != NULL; tagp++) {
		tag = *tagp;
		if (strcmp(tag, i_tag) == 0) {
			/* found it */
			return(PIN_BOOLEAN_TRUE);
		}
	}
	/* couldn't find it */
	return(PIN_BOOLEAN_FALSE);
}

/*
 * Generic tag collection function
 * - iterate array of strings and add to flist
 */
static void
_tag_fn(char *type_str, char **tag_array, pin_flist_t *r_flistp,
	int32 *eidp, pin_errbuf_t *ebufp)
{
	pin_flist_t *flistp;
	int32       eid = *eidp;
	char        **tagp;
	char        *tag;

	for (tagp = tag_array; *tagp != NULL; tagp++) {
		tag = *tagp;
		flistp = PIN_FLIST_ELEM_ADD(r_flistp,
		PIN_FLD_PROD_PROVISIONING_TAGS, eid, ebufp);
		eid++;
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_TYPE_STR,
			(void *)type_str, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_PROVISIONING_TAG,
			(void *)tag, ebufp);
	}
	*eidp = eid;
}

/*
 * function to return an flist containing a /config/XXX
 * object, where XXX is the name of the object of interest
 * - set up a search using PCM_OP_SEARCH
 * - "take" the result flist and return it
 */
static pin_flist_t *
_get_config_object(
	pcm_context_t *ctxp, 
	poid_t        *pp, 
	char          *config_name,    /* "/config/XXX" */
	pin_errbuf_t  *ebufp)
{
	int64           db_no;
	pin_flist_t 	*r_flistp = NULL;
	pin_flist_t 	*s_flistp = NULL;
	pin_flist_t 	*flistp = NULL;
	poid_t 		*pdp; 
	pin_cookie_t	cookie = NULL;
	int32		elem_id = 0;
	char		ebp[1024];

	/* get db number from the service poid */
	db_no = PIN_POID_GET_DB(pp);
	
	/* set up the search flist */
	s_flistp = PIN_FLIST_CREATE(ebufp);
	pdp = PIN_POID_CREATE(db_no, "/search", (int64)500, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);

	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	pdp = PIN_POID_CREATE(db_no, config_name, (int64)-1, ebufp);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);
	PIN_FLIST_ELEM_SET(s_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp);

	/* do the search */
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_flistp, ebufp);

	/* "take" the result to return */
	flistp = PIN_FLIST_ELEM_TAKE_NEXT(r_flistp, PIN_FLD_RESULTS,
		&elem_id, 1, &cookie, ebufp);

	/* clean up */
	PIN_FLIST_DESTROY(s_flistp, NULL);
	PIN_FLIST_DESTROY(r_flistp, NULL);

	/* what to return */
	if (PIN_ERR_IS_ERR(ebufp)) {
		sprintf(ebp, "error retrieving %s object", config_name);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, ebp, ebufp);
		PIN_FLIST_DESTROY(flistp, NULL);
		return((pin_flist_t *)NULL);
	}

	return(flistp);
}

/********************************************************************
 * PROVISIONING FUNCTIONS
 * Here is where customization is done.
 * The rules:
 * (1) do NOT overwrite the svc_obj_p POID.
 *
 *******************************************************************/

/*
 * group of functions for /service/ip
 */
static void
tag_fn_ip(pcm_context_t *ctxp, poid_t *svc_obj_p, 
	pin_flist_t *r_flistp, int32 *eidp, pin_errbuf_t *ebufp)
{
	_tag_fn(IPSERVICE, tags_ip, r_flistp, eidp, ebufp);
	tag_fn_get_tags(ctxp, svc_obj_p, r_flistp, eidp, ebufp,
				SSGCONFIG, IPSERVICE);
}

static int32
valid_tag_ip(pcm_context_t *ctxp, poid_t *svc_obj_p, char *tag)
{
	if(isValidSSGTag(tag)) {
		return valid_tag_wireless(ctxp, svc_obj_p, tag, SSGCONFIG);
	} else {
		return(_valid_tag(tag, tags_ip));
	}
}

static void
plp_ip(pcm_context_t *ctxp, poid_t *svc_obj_p, int32 buy,
	char *tag, pin_errbuf_t *ebufp)
{
	if (strcmp(tag, "example") == 0) {
		plp_example(ctxp, svc_obj_p, buy, tag, ebufp);
	}
	else {
		plp_ssg(ctxp, svc_obj_p, buy, tag, ebufp);
	}
}
/*
 * an ip example - on buy, sets the (unused) field PROTOCOL to 1
 * on cancel, clears PROTOCOL
 */
static void
plp_example(pcm_context_t *ctxp, poid_t *svc_obj_p, int32 buy,
	char *tag, pin_errbuf_t *ebufp)
{
	pin_flist_t	*flistp;
	pin_flist_t	*sub_flistp;
	int32		protocol;
	pin_flist_t	*r_flistp = (pin_flist_t *)NULL;

	flistp = PIN_FLIST_CREATE(ebufp);

	/* 
	 * build flist from POID and any fields desired to set/clear
	 */
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, svc_obj_p, ebufp);
	sub_flistp = PIN_FLIST_SUBSTR_ADD(flistp, PIN_FLD_SERVICE_IP, ebufp);
	if (buy) {
		protocol = 1;
	} else {
		protocol = 0;
	}
	PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_PROTOCOL,
	    (void *)&protocol, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	    "DDD plp_example WFLDS flist", flistp);

	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, flistp, &r_flistp, ebufp);

	PIN_FLIST_DESTROY(flistp, NULL);
	PIN_FLIST_DESTROY(r_flistp, NULL);

	return;
}

static void
tag_fn_vpdn(pcm_context_t *ctxp, poid_t *svc_obj_p, 
	pin_flist_t *r_flistp, int32 *eidp, pin_errbuf_t *ebufp)
{
	_tag_fn(VPDNSERVICE, tags_vpdn, r_flistp, eidp, ebufp);
	tag_fn_get_tags(ctxp, svc_obj_p, r_flistp, eidp, ebufp, 
		SSGCONFIG, VPDNSERVICE);
}

static int32
valid_tag_vpdn(pcm_context_t *ctxp, poid_t *svc_obj_p, char *tag)
{
	if(isValidSSGTag(tag)) {
		return valid_tag_wireless(ctxp, svc_obj_p, tag, SSGCONFIG);
	} else {
		return(_valid_tag(tag, tags_vpdn));
	}
}

static void
tag_fn_ssg(pcm_context_t *ctxp, poid_t *svc_obj_p, 
	pin_flist_t *r_flistp, int32 *eidp, pin_errbuf_t *ebufp)
{
	tag_fn_get_tags(ctxp, svc_obj_p, r_flistp, eidp, ebufp, 
			SSGCONFIG, SSGSERVICE);
}

static int32
valid_tag_ssg(pcm_context_t *ctxp, poid_t *svc_obj_p, char *tag)
{
	return valid_tag_wireless(ctxp, svc_obj_p, tag, SSGCONFIG);
}

/*
 * SSG example:
 * This routine looks at the provisioning tag and performs the
 * following services:
 *  1. "Change Name" - Change PIN_FLD_NAME of this service to a specific
 *      value.  Only for /service/ssg type services.
 *  2. "Link to SSG" - Add this service to the PIN_FLD_SERVICES array of
 *      the /service/ssg that has the same /account POID and whose
 *      PIN_FLD_NAME matches the specified value.  Only for /service/ip or
 *      /service/vpdn types of services.
 *  3. "Link to Profile Service" - Change PIN_FLD_PROFILE_OBJ of this service
 *      to point to the service of the specified type and login.
 *
 * To accomplish this, the provisioning tag will be a series of tokens with
 * each token being separated by a period.  The provisioning tag has the
 * following formats:
 *    ssg=<ssg name>
 *       or
 *    ssg=<ssg name>;profile=<service type>.<service login>
 *       or
 *    profile=<service type>.<service login>
 *
 * The provisioning tag for /service/ssg services must be of the form:
 *
 *      ssg=<ssg name>
 *
 * The provisioning tag for all other services must be one of the
 * following forms:
 *
 *      ssg=<ssg name>
 *         or
 *      ssg=<ssg name>;profile=<service type>.<service login>
 *         or
 *      profile=<service type>.<service login>
 *
 * On buy:
 *   When called for a /service/ssg object, this code sets PIN_FLD_NAME
 *   of that object to be <ssg name> from the provisioning tag.
 *   When called for all other services, this code adds this service to
 *   the user's /service/ssg service that has the name of <ssg name>.
 *   An entry is added to the /service/ssg's PIN_FLD_SERVICES array.
 *   It then sets PIN_FLD_PROFILE_OBJ of this service to point to the
 *   service of the specified <service type> that has the login of
 *   <service.login>.
 *
 * On cancel:
 *   When called for a /service/ssg object, this code sets PIN_FLD_NAME
 *   of that object to be "".
 *   When called for all other services, this code removes this service
 *   from the user's /service/ssg service that has the name of <ssg name>.
 *   It then sets PIN_FLD_PROFILE_OBJ to point to be a NULL POID.
 */
static void
plp_ssg(pcm_context_t *ctxp, poid_t *svc_obj_p, int32 buy,
	char *tag, pin_errbuf_t *ebufp)
{
	pin_flist_t		*d_rflds_flistp = NULL;
	pin_flist_t		*d_ret_rflds_flistp = NULL;
	pin_flist_t		*d_wflds_flistp = NULL;
	pin_flist_t		*d_ret_wflds_flistp = NULL;
	pin_flist_t		*a_flistp = NULL;
	pin_flist_t		*temp_flistp = NULL;
	char			*ssg_name = NULL;
	char			*service_type = NULL;
	char			*service_login = NULL;
	char			*tag_type = NULL;
	char			*temp_ptr = NULL;
	char			my_tag_str[256];
	pin_const_poid_type_t	srvc_type = NULL;
	int32			cnt = 0;
	int32			mflags = PCM_OPFLG_ADD_ENTRY;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
		/******/
	}

	PIN_ERR_CLEAR_ERR(ebufp);
	/*
	 * Parse the "provisioning tag".
	 * Make a local copy, parse it, then change the ';' and '=' to NULLs
	 * to terminate the tokens.
	 */
	pin_strlcpy(my_tag_str, tag, sizeof(my_tag_str));
	tag_type = &my_tag_str[0];
	temp_ptr = &my_tag_str[0];
	while (tag_type != NULL) {
		temp_ptr = strchr(temp_ptr, '=');
		if (temp_ptr == NULL) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
				"plp_ssg Invalid tag, missing =");
			return;
			/******/
		}
		*temp_ptr = '\x00';
		temp_ptr += 1;

		/* 
		 * Now see what tag_type this is. 
		 */

		if (!strcmp(tag_type, "ssg")) {
			/*
			 * parse ssg=<ssg name>
			 */
			ssg_name = temp_ptr;
		}
		else if (!strcmp(tag_type, "profile")) {
			/*
			 * parse profile=<type>.<login> 
			 */
			service_type = temp_ptr;
			temp_ptr = strchr(temp_ptr, '.');
			if (temp_ptr == NULL) {
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
					"plp_ssg Invalid profile tag, "
					"missing .");
				return;
				/******/
			}
			*temp_ptr = '\x00';
			service_login = temp_ptr + 1;
		}
		else {
			/*
			 * Invalid tag type 
			 */
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
				"plp_ssg Invalid tag type.");
			return;
			/******/
		}
		/*
		 * Check for more tag types
		 */
		temp_ptr = strchr(temp_ptr, ';');
		if (temp_ptr == NULL) {
			/* done */
			tag_type = NULL;
		}
		else {
			/* we have another tag_type. */
			*temp_ptr = '\x00';
			temp_ptr += 1;
			tag_type = temp_ptr;
		}
	} /* end of while */

	/*
	 * What functions are to be performed?
	 *
	 * If ssg_name has been specified and this is a /service/ssg 
	 * service, then this is a "Change Name" function.
	 *
	 * If ssg_name has been specified and this is not a /service/ssg
	 * service, then this is a "Link to SSG" function.
	 *
	 * If service_type has been specified then this is a 
	 * "Link to Service Profile" function.
	 */

	if (ssg_name != NULL) {

		srvc_type = PIN_POID_GET_TYPE(svc_obj_p);

		if (srvc_type && !strcmp(srvc_type, SSGSERVICE)) {
			/* Processing a /service/ssg service.
			 * This is the "Change Name" function.
			 * All I  modify is PIN_FLD_NAME. I set it to 
			 * <ssg_name> if this is a "buy". If this is a
			 * "cancel", then I set it to "".
			 */
			pin_flist_t		*d_mod_flistp = NULL;
			pin_flist_t		*d_ret_mod_flistp = NULL;
			pin_flist_t		*t_flistp = NULL;
			int32			mflags = 0;

			if (!buy) {
				ssg_name = "";
			}
			d_mod_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(d_mod_flistp, PIN_FLD_POID,
				svc_obj_p, ebufp);
			PIN_FLIST_FLD_SET(d_mod_flistp, PIN_FLD_NAME,
				ssg_name, ebufp);
			/*
			 * Debug: What we're about to modify
			 */
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"plp_ssg MODIFY PIN_FLD_NAME "
				"input flist", d_mod_flistp);

			PCM_OP(ctxp, PCM_OP_WRITE_FLDS, mflags, d_mod_flistp,
			       &d_ret_mod_flistp, ebufp);
			/*
			 * Debug: What we're got back from the modify
			 */
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"plp_ssg MODIFY PIN_FLD_NAME "
				"return flist",	d_ret_mod_flistp);

			PIN_FLIST_DESTROY(d_mod_flistp, NULL);
			PIN_FLIST_DESTROY(d_ret_mod_flistp, NULL);
		}
		else {
			/*
			 * NOT Processing a /service/ssg service.
			 */		
			plp_link_to_ssg(ctxp, svc_obj_p, buy, ssg_name,
				ebufp);
		}
	}

	if (service_type != NULL) {		
		plp_link_to_service_profile(ctxp, svc_obj_p, buy,
			service_type, service_login, ebufp);
	}

}

/*
 * group of functions for /service/content
 */
static void
tag_fn_content(pcm_context_t *ctxp, poid_t *svc_obj_p, 
	pin_flist_t *r_flp, int32 *eidp, pin_errbuf_t *ebufp)
{
	tag_fn_wireless(ctxp, svc_obj_p, r_flp, eidp, ebufp, CONTENTCONFIG, 
		CONTENTSERVICEPROFILE);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"tag_fn_content return flist",	r_flp);
}

/*
 * group of functions for /service/ip/gprs and /service/wap
 */

static void
tag_fn_wap(pcm_context_t *ctxp, poid_t *svc_poidp, 
	pin_flist_t *r_flp, int32 *eidp, pin_errbuf_t *ebufp)
{
	/* Wrapper to call real function with appropriate type */

	tag_fn_wireless(ctxp, svc_poidp, r_flp, eidp, ebufp, WAPCONFIG, 
		WAPSERVICEPROFILE);
}
	
static void
tag_fn_gprs(pcm_context_t *ctxp, poid_t *svc_poidp, 
	pin_flist_t *r_flp, int32 *eidp, pin_errbuf_t *ebufp)
{
	/* Wrapper to call real function with appropriate type */

	tag_fn_wireless(ctxp, svc_poidp, r_flp, eidp, ebufp, 
		GPRSCONFIG, GPRSSERVICEPROFILE);
	tag_fn_get_tags(ctxp, svc_poidp, r_flp, eidp, ebufp, 
		SSGCONFIG, GPRSSERVICEPROFILE);
}


static void
tag_fn_get_tags(pcm_context_t *ctxp, poid_t *svc_poidp, 
	pin_flist_t *r_flp, int32 *eidp, pin_errbuf_t *ebufp, 
	char *configtype, char *servicetype)
{
	/*
	 * Searches the appropriate config object to get list 
	 * of valid profile names.
	 */

	pin_cookie_t    cookie = NULL;
	int32		elem_id = 0;
	char		*tag_name = NULL;
	char		srvcType[PCM_MAX_POID_TYPE];
	pin_flist_t	*flp = NULL;
	pin_flist_t	*entry_flp = NULL;
	pin_flist_t	*config_flp = NULL;
	poid_t		*srvcPoidp = NULL;
	char		*srvcPoidType = NULL;	
	int32		eid = *eidp;

	/*
	 * Read config object from DB
	 */

	/* Read the config object with the appropriate type */
	config_flp = _get_config_object(ctxp, svc_poidp, configtype, ebufp);

	/* Did something go wrong while reading? */
	if (config_flp == (pin_flist_t *) NULL) {
		
		if (PIN_ERR_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"tag_fn_get_tags function can't load config object!",
				ebufp);
		}
		/* Else, config object not found, but no error. */

		return;
	}

	/*
	 * Walk array of provisioning tags found in config object
	 */

	while ((entry_flp = PIN_FLIST_ELEM_GET_NEXT(config_flp,
		PIN_FLD_PROD_PROVISIONING_TAGS, &elem_id, 1,
		&cookie, ebufp)) != (pin_flist_t *)NULL) {

		/* Grab tag from current entry */
		tag_name = (char *) PIN_FLIST_FLD_GET(entry_flp,
			PIN_FLD_PROVISIONING_TAG, 0, ebufp);

		srvcPoidp = PIN_FLIST_FLD_GET(entry_flp, 
			PIN_FLD_SERVICE_OBJ, 0, ebufp);

		/* Is everything groovy? */
		if (tag_name == NULL || PIN_ERR_IS_ERR(ebufp)){
			/* No: clean up and exit */
			PIN_FLIST_DESTROY_EX(&config_flp, NULL);

                	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        	"tag_fn_get_tags function can't get "
				"PIN_FLD_PROVISIONING_TAG",
			 	ebufp);
			return;
		}
		
		srvcPoidType = (char *)PIN_POID_GET_TYPE(srvcPoidp);
		if (strcmp(servicetype, srvcPoidType) == 0) {
			/* Add current tag to output */
			flp = PIN_FLIST_ELEM_ADD(r_flp,
				PIN_FLD_PROD_PROVISIONING_TAGS, eid, ebufp);
			eid++;
			PIN_FLIST_FLD_SET(flp, PIN_FLD_TYPE_STR,
				servicetype, ebufp);
			PIN_FLIST_FLD_SET(flp, PIN_FLD_PROVISIONING_TAG,
				(void *)tag_name, ebufp);
		}	
	} /* while */

	/* Return current element ID */
	*eidp = eid;

	/* clean up */
	PIN_FLIST_DESTROY_EX(&config_flp, NULL);
}


static void
tag_fn_wireless(pcm_context_t *ctxp, poid_t *svc_poidp, 
	pin_flist_t *r_flp, int32 *eidp, pin_errbuf_t *ebufp, 
	char *configtype, char *servicetype)
{

	/*
	 * Searches the appropriate config object to get list 
	 * of valid profile names.
	 */

	pin_cookie_t    cookie = NULL;
	int32		elem_id;
	char		*tag_name = NULL;
	pin_flist_t	*flp = NULL;
	pin_flist_t	*entry_flp = NULL;
	pin_flist_t	*config_flp = NULL;

	int32		eid = *eidp;

	/*
	 * Read config object from DB
	 */

	/* Read the config object with the appropriate type */
	config_flp = _get_config_object(ctxp, svc_poidp, configtype, ebufp);

	/* Did something go wrong while reading? */
	if (config_flp == (pin_flist_t *) NULL) {
		/* Uh, yeah, so exit */
		if (PIN_ERR_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"Wireless tag function can't load config object!",
				ebufp);
		}
		/* Else, config object not found, but no error. */

		return;
	}

	/*
	 * Walk array of provisioning tags found in config object
	 */

	while ((entry_flp = PIN_FLIST_ELEM_GET_NEXT(config_flp,
		PIN_FLD_PROD_PROVISIONING_TAGS, &elem_id, 1,
		&cookie, ebufp)) != (pin_flist_t *)NULL) {

		/* Grab tag from current entry */
		tag_name = (char *) PIN_FLIST_FLD_GET(entry_flp,
			PIN_FLD_PROVISIONING_TAG, 0, ebufp);

		/* Is everything groovy? */
		if (tag_name == NULL || PIN_ERR_IS_ERR(ebufp)){
			/* No: clean up and exit */
			PIN_FLIST_DESTROY_EX(&config_flp, NULL);

           		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
              		    	"wirless tag function can't get "
				"PIN_FLD_PROVISIONING_TAG",
				 ebufp);
			return;
		}

		/* Add current tag to output */
		flp = PIN_FLIST_ELEM_ADD(r_flp,
			PIN_FLD_PROD_PROVISIONING_TAGS, eid, ebufp);
		eid++;
		PIN_FLIST_FLD_SET(flp, PIN_FLD_TYPE_STR,
			servicetype, ebufp);
		PIN_FLIST_FLD_SET(flp, PIN_FLD_PROVISIONING_TAG,
			(void *)tag_name, ebufp);
	} /* while */

	/* Return current element ID */
	*eidp = eid;

	/* clean up */
	PIN_FLIST_DESTROY_EX(&config_flp, NULL);
}

static void
tag_fn_telco(pcm_context_t *ctxp, poid_t *svc_poidp, 
	pin_flist_t *r_flp, int32 *eidp, pin_errbuf_t *ebufp, 
	char *configtype, char *servicetype)
{

	/*
	 * Searches the appropriate config object to get list 
	 * of valid profile names.
	 */

	pin_cookie_t    cookie = NULL;
	int32		elem_id;
	char		*tag_name = NULL;
	pin_flist_t	*flp = NULL;
	pin_flist_t	*entry_flp = NULL;
	pin_flist_t	*config_flp = NULL;

	int32		eid = *eidp;

	/*
	 * Read config object from DB
	 */

	/* Read the config object with the appropriate type */
	config_flp = _get_config_object(ctxp, svc_poidp, configtype, ebufp);

	/* Did something go wrong while reading? */
	if (config_flp == (pin_flist_t *) NULL) {
		/* Uh, yeah, so exit */
		if (PIN_ERR_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"Telco tag function can't load config object!",
				ebufp);
		}
		/* Else, config object not found, but no error. */

		return;
	}

	/*
	 * Walk array of provisioning tags found in config object
	 */

	while ((entry_flp = PIN_FLIST_ELEM_GET_NEXT(config_flp,
		PIN_FLD_PROD_PROVISIONING_TAGS, &elem_id, 1,
		&cookie, ebufp)) != (pin_flist_t *)NULL) {

		/* Grab tag from current entry */
		tag_name = (char *) PIN_FLIST_FLD_GET(entry_flp,
			PIN_FLD_PROVISIONING_TAG, 0, ebufp);

		/* Is everything groovy? */
		if (tag_name == NULL || PIN_ERR_IS_ERR(ebufp)){
			/* No: clean up and exit */
			PIN_FLIST_DESTROY_EX(&config_flp, NULL);

           		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
              		    	"telco tag function can't get "
				"PIN_FLD_PROVISIONING_TAG",
				 ebufp);
			return;
		}

		/* Add current tag to output */
		flp = PIN_FLIST_ELEM_ADD(r_flp,
			PIN_FLD_PROD_PROVISIONING_TAGS, eid, ebufp);
		eid++;
		PIN_FLIST_FLD_SET(flp, PIN_FLD_TYPE_STR,
			servicetype, ebufp);
		PIN_FLIST_FLD_SET(flp, PIN_FLD_PROVISIONING_TAG,
			(void *)tag_name, ebufp);
	} /* while */

	/* Return current element ID */
	*eidp = eid;

	/* clean up */
	PIN_FLIST_DESTROY_EX(&config_flp, NULL);
}

static int32
valid_tag_content(pcm_context_t *ctxp, poid_t *svc_poidp, char *tag)
{
	/* Wrapper to call real function with appropriate type */
	return valid_tag_wireless(ctxp, svc_poidp, tag, CONTENTCONFIG); 
}
	

static int32
valid_tag_wap(pcm_context_t *ctxp, poid_t *svc_poidp, char *tag)
{
	/* Wrapper to call real function with appropriate type */

	return valid_tag_wireless(ctxp, svc_poidp, tag, WAPCONFIG);
}
	
static int32
valid_tag_gprs(pcm_context_t *ctxp, poid_t *svc_poidp, char *tag)
{
	/* Wrapper to call real function with appropriate type */
	if(isValidSSGTag(tag)) {
		return valid_tag_wireless(ctxp, svc_poidp, tag, SSGCONFIG);
	} else {
		return valid_tag_wireless(ctxp, svc_poidp, tag, GPRSCONFIG);
	}
	
}

static int32
valid_tag_wireless(pcm_context_t *ctxp, poid_t *svc_poidp, char *tag, char *type)
{

	/* Valid tags for wireless are listed in the config object,
	 * so our valid function trys to find the given 
	 * tag in the appropriate object.
	 */

	pin_flist_t 	*config_flp = NULL;
	pin_flist_t 	*entry_flp = NULL;
	pin_errbuf_t 	ebuf;
	pin_cookie_t 	cookie = NULL;
	int32		elem_id;
	char		*ctag_name = NULL;
	int32		isvalid = PIN_BOOLEAN_FALSE;

	PIN_ERR_CLEAR_ERR(&ebuf);

	/*
	 * Read config object from DB
	 */

	/* Read the config object with the appropriate type */
	config_flp = _get_config_object(ctxp, svc_poidp, type, &ebuf);

	/* Did something go wrong while reading? */
	if (config_flp == (pin_flist_t *) NULL) {
		/* Uh, yeah, so exit */
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"Wireless tag function can't load config object!",
				&ebuf);

			return PIN_BOOLEAN_FALSE;
	}


	/* 
	 * Walk array of valid tags, comparing to our input
	 */

	while ((entry_flp = PIN_FLIST_ELEM_GET_NEXT(config_flp,
		PIN_FLD_PROD_PROVISIONING_TAGS, &elem_id, 1,
		&cookie, &ebuf)) != (pin_flist_t *)NULL) {
		
		/* Grab tag in current entry */
		ctag_name = (char *) PIN_FLIST_FLD_GET(entry_flp,
			PIN_FLD_PROVISIONING_TAG, 0, &ebuf);
		if (ctag_name == NULL) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_WARNING,
				"NULL tag name in wireless config "
				"object");
			continue; /* next element */
		}

		/* Compare tag in current entry to input */
		if (strcmp(ctag_name, tag) == 0) {
			isvalid = PIN_BOOLEAN_TRUE;
			break;
	}
	} /* while */
	
	/* clean up and return  */
	PIN_FLIST_DESTROY(config_flp, NULL);
	return isvalid;
}

static void
plp_wireless_content(pcm_context_t *ctxp, poid_t *svc_obj_p, int32 buy,
	char *tag, pin_errbuf_t *ebufp)
{
	plp_link_to_content_profile(ctxp, svc_obj_p, buy,
		tag, ebufp);
}


static void
plp_wireless_wap(pcm_context_t *ctxp, poid_t *svc_obj_p, int32 buy,
	char *tag, pin_errbuf_t *ebufp)
{

	/* Provisioning for wireless is simply setting the service 
	 * profile, so that's all we do!
	 */

	plp_link_to_service_profile(ctxp, svc_obj_p, buy,
		"wap", tag, ebufp);
}

static void
plp_wireless_gprs(pcm_context_t *ctxp, poid_t *svc_obj_p, int32 buy,
	char *tag, pin_errbuf_t *ebufp)
{
	if(isValidSSGTag(tag)) {
		plp_ssg(ctxp, svc_obj_p, buy, tag, ebufp);
	} else {
		plp_link_to_service_profile(ctxp, svc_obj_p, buy,
			"ip/gprs", tag, ebufp);
	}
}

/* Checks whether tag is a vlid SSG Tag or not */
static int32 
isValidSSGTag(char	*tag)
{
	if (strstr(tag, "ssg=") != NULL) 
		return 1; 
	if (strstr(tag, "profile=") != NULL) 
		return 1;
	return 0;
}

/*
 * group of functions for /service/email
 */

static void
tag_fn_email(pcm_context_t *ctxp, poid_t *svc_obj_p, 
	pin_flist_t *r_flistp, int32 *eidp, pin_errbuf_t *ebufp)
{
	_tag_fn("/service/email", tags_email, r_flistp, eidp, ebufp);
}

static int32
valid_tag_email(pcm_context_t *ctxp, poid_t *svc_obj_p, char *tag)
{
	return(_valid_tag(tag, tags_email));
}

/* function to choose between jumbo and regular */
static void
plp_email(pcm_context_t *ctxp, poid_t *svc_obj_p, int32 buy,
	char *tag, pin_errbuf_t *ebufp)
{
	if (strcmp(tag, "regular") == 0) {
		plp_mail_regular(ctxp, svc_obj_p, buy, tag, ebufp);
	}
	else {
		plp_mail_jumbo(ctxp, svc_obj_p, buy, tag, ebufp);
	}
}

/*
 * example of just setting the MAX_MBOX_SIZE and MAX_MSG_SIZE.
 */
static void
plp_mail_regular(pcm_context_t *ctxp, poid_t *svc_obj_p, int32 buy,
	char *tag, pin_errbuf_t *ebufp)
{
	pin_flist_t	*flistp;
	pin_flist_t	*sub_flistp;
	int32		mbox_size;
	int32		msg_size;
	pin_flist_t	*r_flistp = (pin_flist_t *)NULL;

	flistp = PIN_FLIST_CREATE(ebufp);

	if (buy) {
		/* 
		 * build flist from POID and any fields desired to set
		 */
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, svc_obj_p, ebufp);
		sub_flistp = PIN_FLIST_SUBSTR_ADD(flistp, 
			PIN_FLD_SERVICE_EMAIL, ebufp);
		mbox_size = 2 * 1024 * 1024;
		PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_MAX_MBOX_SIZE,
		    (void *)&mbox_size, ebufp);
		msg_size = 512 * 1024;
		PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_MAX_MSG_SIZE,
		    (void *)&msg_size, ebufp);

		PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, flistp, &r_flistp, ebufp);

	} else {
		/*
		 * build flist from POID and any fields desired to clear
		 */
		/* nothing to do with email, tho' we could read and clear */
		PIN_ERR_CLEAR_ERR(ebufp);
	}

	PIN_FLIST_DESTROY(flistp, NULL);
	PIN_FLIST_DESTROY(r_flistp, NULL);

	return;
}

/*
 * example of setting the MAX_MBOX_SIZE and MAX_MSG_SIZE (larger),
 * and "calculating" the mail server path.
 */
static void
plp_mail_jumbo(pcm_context_t *ctxp, poid_t *svc_obj_p, int32 buy,
	char *tag, pin_errbuf_t *ebufp)
{
	char		*login = NULL;
	char		*spool = NULL;
	char		mbox_path[256];		/* 255 + 1 for NULL */
	int32		i = 0;
	int32		mbox_size = 0;
	int32		msg_size = 0;
	int64		id = 0;
	pin_flist_t	*flistp = (pin_flist_t *)NULL;
	pin_flist_t	*sub_flistp = (pin_flist_t *)NULL;
	pin_flist_t	*r_flistp = (pin_flist_t *)NULL;

	flistp = PIN_FLIST_CREATE(ebufp);

	if (buy) {
		/*
		 * get the user's name
		 */
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, svc_obj_p, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_LOGIN, (char *)NULL, ebufp);

		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, flistp, &r_flistp, ebufp);

		login = (char *)PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_LOGIN,
		    0, ebufp);

		if (PIN_ERR_IS_ERR(ebufp)) {
			goto cleanup;
			/***********/
		}

		/*
		 * is this example, we "know" we have 4 spool areas.
		 * and we assume random POID ids, and just split people
		 * up accross them by modulo arithmetic.
		 */
		id = PIN_POID_GET_ID(svc_obj_p);
		i = (int32) (id % 4);
		switch (i) {
		case 0:
			spool = "/export/disk1/";
			break;
		case 1:
			spool = "/export/disk2/";
			break;
		case 2:
			spool = "/var/email/md001/";
			break;
		case 3:
			spool = "/var/email/md002/";
			break;
		}

		if ((strlen(spool) + strlen(login)) >= sizeof(mbox_path)) {
			pin_set_err(ebufp, PIN_ERRLOC_FM,
			    PIN_ERRCLASS_SYSTEM_DETERMINATE,
			    PIN_ERR_TOO_BIG, PIN_FLD_PATH, 0,
			    (strlen(spool) + strlen(login)));
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			    "mbox path would be too big", ebufp);
			goto cleanup;
			/***********/
		}

		sprintf(mbox_path, "%s/%s", spool, login);

		PIN_FLIST_DESTROY(flistp, NULL);
		PIN_FLIST_DESTROY(r_flistp, NULL);
		flistp = PIN_FLIST_CREATE(ebufp);
		r_flistp = (pin_flist_t *)NULL;

		/* 
		 * build flist from POID and any fields desired to set
		 */
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, svc_obj_p, ebufp);
		sub_flistp = PIN_FLIST_SUBSTR_ADD(flistp, 
			PIN_FLD_SERVICE_EMAIL, ebufp);
		mbox_size = 10 * 1024 * 1024;
		PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_MAX_MBOX_SIZE,
		    (void *)&mbox_size, ebufp);
		msg_size = 1 * 1024 * 1024;
		PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_MAX_MSG_SIZE,
		    (void *)&msg_size, ebufp);
		PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_PATH,
		    (void *)mbox_path, ebufp);

		PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, flistp, &r_flistp, ebufp);

	} else {
		/*
		 * build flist from POID and any fields desired to clear
		 */
		/* nothing to do with email, tho' we could read and clear */
		PIN_ERR_CLEAR_ERR(ebufp);
	}

cleanup:
	PIN_FLIST_DESTROY(flistp, NULL);
	PIN_FLIST_DESTROY(r_flistp, NULL);

	return;
}

/*
 * iterate the PIN_FLD_PROD_PROVISIONING_TAGS array of the
 * /config/teracommtags object comparing each 
 * PIN_FLD_PROVISIONING_TAG with the argument
 * return PIN_BOOLEAN_TRUE if found, PIN_BOOLEAN_FALSE otherwise
 */
static int32 
valid_tag_teracomm(
	pcm_context_t *ctxp, 
	poid_t *svc_obj_p, 
	char *tag
	)
{
	poid_t 		*pp = svc_obj_p;
	pin_flist_t 	*c_flistp = NULL;
	pin_flist_t 	*a_flistp = NULL;
	pin_errbuf_t 	ebuf;
	pin_cookie_t 	cookie = NULL;
	int32		elem_id;
	char		*ctag_name = NULL;
	int32		valid = PIN_BOOLEAN_FALSE;

	PIN_ERR_CLEAR_ERR(&ebuf);

	c_flistp = _get_config_object(ctxp, pp, "/config/teracommtags", 
		&ebuf);
	if (c_flistp == (pin_flist_t *)NULL){
		return(PIN_BOOLEAN_FALSE);
	}

	/* iterate flist comparing tag to PIN_FLD_PROVISIONING_TAG */
	while ((a_flistp = PIN_FLIST_ELEM_GET_NEXT(c_flistp,
		PIN_FLD_PROD_PROVISIONING_TAGS, &elem_id, 1,
		&cookie, &ebuf)) != (pin_flist_t *)NULL) {
		
		ctag_name = (char *) PIN_FLIST_FLD_GET(a_flistp,
			PIN_FLD_PROVISIONING_TAG, 0, &ebuf);
		if (ctag_name == NULL) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_WARNING,
				"NULL tag name in /config/teracommtags "
				"object");
			continue; /* next element */
		}
		if (strcmp(ctag_name, tag) == 0) {
			valid = PIN_BOOLEAN_TRUE;
			break;
		}
	} /* while */

	/* clean up and return  */
	PIN_FLIST_DESTROY(c_flistp, NULL);
	return(valid);
}

/* 
 * retrieve the /config/teracommtags object from the database and
 * collect the provisioning tags
 */
static void 
tag_fn_teracomm(
	pcm_context_t *ctxp,
	poid_t *svc_obj_p,
	pin_flist_t *r_flistp, 
	int32 *eidp,
	pin_errbuf_t *ebufp
	)
{
	poid_t      *pp = svc_obj_p;
	pin_flist_t *flistp = NULL;
	pin_flist_t *c_flistp = NULL;
	pin_flist_t *a_flistp = NULL;
	int32       eid = *eidp;
	char        *tag_name = NULL;
	pin_cookie_t    cookie = NULL;
	int32		elem_id;

	c_flistp = _get_config_object(ctxp, pp, "/config/teracommtags",
		ebufp);
	if (c_flistp == (pin_flist_t *)NULL) {
		if (PIN_ERR_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"Error retrieving teracommtags config object",
				ebufp);
		}
		/*  Else, config object not found, but no error. */
		return;
	}

	while ((a_flistp = PIN_FLIST_ELEM_GET_NEXT(c_flistp,
		PIN_FLD_PROD_PROVISIONING_TAGS, &elem_id, 1,
		&cookie, ebufp)) != (pin_flist_t *)NULL) {
		tag_name = (char *) PIN_FLIST_FLD_GET(a_flistp,
			PIN_FLD_PROVISIONING_TAG, 0, ebufp);
		if (tag_name == NULL || PIN_ERR_IS_ERR(ebufp)){
			PIN_FLIST_DESTROY(c_flistp, NULL);
                	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        	"teracommtags config object: missing "
				"PIN_FLD_PROVISIONING_TAG",
			 	ebufp);
			break;
                        /****/
		}
		flistp = PIN_FLIST_ELEM_ADD(r_flistp,
			PIN_FLD_PROD_PROVISIONING_TAGS, eid, ebufp);
		eid++;
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_TYPE_STR,
			(void *)"/service/ip/cable/teracomm", ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_PROVISIONING_TAG,
			(void *)tag_name, ebufp);
	} /* while */

	*eidp = eid;
	/* clean up */
	PIN_FLIST_DESTROY(c_flistp, NULL);
}

/* forward decls. */
static int valid_teracomm_mm_addr(char *mm_addr);
static int teracomm_svc_provisioned(
	pcm_context_t	*ctxp,
	poid_t		*svc_obj_p,
	pin_errbuf_t	*ebufp);

static void
plp_teracomm(
	pcm_context_t *ctxp, 
	poid_t *svc_obj_p,
	int32 buy,
	char *tag, 
	pin_errbuf_t *ebufp
	)
{
	pin_flist_t 	*flistp = (pin_flist_t *)NULL;
	pin_flist_t 	*sub_flistp = (pin_flist_t *)NULL;
	pin_flist_t 	*r_flistp = (pin_flist_t *)NULL;
	pin_flist_t	*ii_flistp = (pin_flist_t *)NULL;
	int 		min_downstm_bandwd = 0;
	int 		max_downstm_bandwd = 0;
	int 		min_upstm_bandwd = 0;
	int		max_upstm_bandwd = 0;
	int 		ip_limit = 0;
	
	if (buy) {
		pin_flist_t 	*c_flistp = (pin_flist_t *)NULL;
		pin_flist_t 	*a_flistp = (pin_flist_t *)NULL;
		char        	*tag_name = NULL;
		pin_cookie_t    cookie = NULL;
		int32		elem_id;
		int		*int_p = NULL;
		int 		found_it = PIN_BOOLEAN_FALSE;
		char		epb[512];
		char		tag_str[256];

		c_flistp = _get_config_object(ctxp, svc_obj_p, 
			"/config/teracommtags", ebufp);
		if (c_flistp == (pin_flist_t *)NULL){
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"can't retrieve teracommtags config object",
				ebufp);
			return;
		}
		 /* look for tag in config flist */
		while ((a_flistp = PIN_FLIST_ELEM_GET_NEXT(c_flistp,
				PIN_FLD_PROD_PROVISIONING_TAGS, &elem_id, 1,
				&cookie, ebufp)) != (pin_flist_t *)NULL) {
			tag_name = (char *) PIN_FLIST_FLD_GET(a_flistp,
				PIN_FLD_PROVISIONING_TAG, 0, ebufp);
			if (tag_name == NULL || PIN_ERR_IS_ERR(ebufp)) {
				PIN_FLIST_DESTROY(c_flistp, NULL);
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"teracommtags config object: missing "
					"PIN_FLD_PROVISIONING_TAG",
			 		ebufp);
				return;
			}
			if (strcmp(tag, tag_name) == 0) { /* found it */
				found_it = PIN_BOOLEAN_TRUE;
				int_p = (int *) PIN_FLIST_FLD_GET(a_flistp,
					PIN_FLD_MIN_DOWNSTREAM_BANDWD, 0, ebufp);
				if (int_p != NULL) {
					min_downstm_bandwd = *int_p;
				}
				int_p = (int *) PIN_FLIST_FLD_GET(a_flistp,
					PIN_FLD_MAX_DOWNSTREAM_BANDWD, 0, ebufp);
				if (int_p != NULL) {
					max_downstm_bandwd = *int_p;
				}
				int_p = (int *) PIN_FLIST_FLD_GET(a_flistp,
					PIN_FLD_MIN_UPSTREAM_BANDWD, 0, ebufp);
				if (int_p != NULL) {
					min_upstm_bandwd = *int_p;
				}
				int_p = (int *) PIN_FLIST_FLD_GET(a_flistp,
					PIN_FLD_MAX_UPSTREAM_BANDWD, 0, ebufp);
				if (int_p != NULL) {
					max_upstm_bandwd = *int_p;
				}
				int_p = (int *) PIN_FLIST_FLD_GET(a_flistp,
					PIN_FLD_IP_LIMIT, 0, ebufp);
				if (int_p != NULL) {
					ip_limit = *int_p;
				}
				if (PIN_ERR_IS_ERR(ebufp)) {
					PIN_FLIST_DESTROY(c_flistp, NULL);
                			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        			"problem retrieving config info",
			 			ebufp);
					return;
				}
				break; /* exit while loop */
			} /* found it */
		} /* while */

		/* done with c_flistp */
		PIN_FLIST_DESTROY(c_flistp, NULL);

		if (PIN_ERR_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"Error looking up tag in config. object.", ebufp);
			return;
		}

		if (!found_it) {
			/* No error, but tag not found in config object. */
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_NONEXISTANT_ELEMENT, PIN_FLD_PROVISIONING_TAG,
				0, 0);
			pin_strlcpy(tag_str, tag, sizeof(tag_str));
			sprintf(epb, "can't find tag \"%s\" in config object",
				tag_str);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, epb, ebufp);
			return;
		}

		/* Make sure service is not already provisioned. */
		
		if (teracomm_svc_provisioned(ctxp, svc_obj_p, ebufp)) {
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_OP_ALREADY_DONE, 0, 0, 0);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"Teracomm service already provisioned.",
				ebufp);
			return;
		}
		
	} /* if (buy) */
	else {
		/*
		 * Cancelling product:
	 	 * Set bandwidth limits and ip limit to all zeros.
		 * Leave all other values untouched, in case another
		 * product is purchased for the service.
		 */
		min_downstm_bandwd = max_downstm_bandwd = 0;
		min_upstm_bandwd = max_upstm_bandwd = 0;
		ip_limit = 0;
	}
		

	/* 
	 * Update service object with provisioning data.  Use
	 * PCM_OP_CUST_MODIFY_SERVICE, so that
	 * /event/notification/service/modify is generated.
	 */
	flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, svc_obj_p, ebufp);
	ii_flistp = PIN_FLIST_SUBSTR_ADD(flistp,
				PIN_FLD_INHERITED_INFO, ebufp);
	sub_flistp = PIN_FLIST_SUBSTR_ADD(ii_flistp,
				PIN_FLD_SERVICE_IP_CABLE, ebufp);
	PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_MIN_DOWNSTREAM_BANDWD,
				&min_downstm_bandwd, ebufp);
	PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_MAX_DOWNSTREAM_BANDWD,
				&max_downstm_bandwd, ebufp);
	PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_MIN_UPSTREAM_BANDWD,
				&min_upstm_bandwd, ebufp);
	PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_MAX_UPSTREAM_BANDWD,
				&max_upstm_bandwd, ebufp);
	sub_flistp = PIN_FLIST_SUBSTR_ADD(ii_flistp,
				PIN_FLD_SERVICE_IP_CABLE_TERACOMM, ebufp);
	PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_IP_LIMIT, &ip_limit, ebufp);

	PCM_OP(ctxp, PCM_OP_CUST_MODIFY_SERVICE, 0, flistp, &r_flistp, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                	"Error modifying service object with "
			"provisioning data", ebufp);
	}

	/* clean up */
	PIN_FLIST_DESTROY(flistp, NULL);
	PIN_FLIST_DESTROY(r_flistp, NULL);
}

static pin_fld_num_t teracomm_bw_flds[] = {
	PIN_FLD_MIN_DOWNSTREAM_BANDWD,
	PIN_FLD_MAX_DOWNSTREAM_BANDWD,
	PIN_FLD_MIN_UPSTREAM_BANDWD,
	PIN_FLD_MAX_UPSTREAM_BANDWD,
	0
};

/* 
 * teracomm_svc_provisioned: Returns whether a TeraComm service
 * is already provisioned.  The service is provisioned if the
 * service object contains a syntactically correct modem mac. addr.,
 * and the bandwidth limit fields are not all zero.
 */
static int teracomm_svc_provisioned(
	pcm_context_t	*ctxp,
	poid_t		*svc_obj_p,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t 	*flistp = (pin_flist_t *)NULL;
	pin_flist_t 	*sub_flistp = (pin_flist_t *)NULL;
	pin_flist_t 	*r_flistp = (pin_flist_t *)NULL;
	int		i;
	void		*vp = NULL;
	char		*modem_mac_addr = NULL;
	int		*bandwdp = NULL;
	int		is_provisioned = PIN_BOOLEAN_FALSE;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return PIN_BOOLEAN_FALSE;
	}
	/*
	 * Read the fields of the service object, needed for
	 * checking if provisioned.
	 */
	flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, svc_obj_p, ebufp);
	sub_flistp = PIN_FLIST_SUBSTR_ADD(flistp,
			PIN_FLD_SERVICE_IP_CABLE, ebufp);
	PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_MODEM_MAC_ADDR,
				(void *)NULL, ebufp);
	for (i = 0; teracomm_bw_flds[i] != 0; i++) {
		PIN_FLIST_FLD_SET(sub_flistp, teracomm_bw_flds[i],
			(void *)NULL, ebufp);
	}
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, flistp, &r_flistp, ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                	"Error reading teracomm service object", ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"Provisioning fields of teracomm service object:", r_flistp);
	/* 
	 * Check fields of service object which indicate whether
	 * modem is provisioned.
	 */
	sub_flistp = PIN_FLIST_SUBSTR_GET(r_flistp,
		PIN_FLD_SERVICE_IP_CABLE, 0, ebufp);	
	/* 
	 * Check if valid modem mac. addr.
	 */
	vp = PIN_FLIST_FLD_GET(sub_flistp,
			PIN_FLD_MODEM_MAC_ADDR, 0, ebufp);
	modem_mac_addr = (char *)vp;
	if (valid_teracomm_mm_addr(modem_mac_addr)) {
		/* 
		 * Check for non-zero bandwidth fields.
		 */
		for (i = 0;
		teracomm_bw_flds[i] != 0 && !is_provisioned;
		i++) {
			vp = PIN_FLIST_FLD_GET(sub_flistp,
				teracomm_bw_flds[i], 0, ebufp);
			bandwdp = (int *)vp;
			if (bandwdp != NULL && *bandwdp != 0) {
				is_provisioned = PIN_BOOLEAN_TRUE;
			}
		}
	}

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                	"Error checking teracomm service object", ebufp);
	}

cleanup:
	PIN_FLIST_DESTROY(flistp, NULL);
	PIN_FLIST_DESTROY(r_flistp, NULL);

	return is_provisioned;
}

/*
 * valid_teracomm_mm_addr: Returns whether a modem mac. addr.
 * is syntactically valid for teracomm.
 */ 
static int valid_teracomm_mm_addr(char *mm_addr)
{
	char c;

	/* A valid TeraComm modem mac. addr. is 12 hex. digits. */
	if (mm_addr == NULL ||
		(strlen(mm_addr) !=
		PIN_TERACOMM_VALID_MODEM_MAC_ADDR_LEN)) {
		return PIN_BOOLEAN_FALSE;
	}
	while (c = *mm_addr++) {
		if ((c < '0' || c > '9') &&
			(c < 'A' || c > 'F') &&
			(c < 'a' || c > 'f')) {
			return PIN_BOOLEAN_FALSE;
		}
	}
	return PIN_BOOLEAN_TRUE;
}

/*
 * Validates the tag for given msexchange service
 */

static int32 
valid_tag_msexchange(
	pcm_context_t *ctxp, 
	poid_t *svc_obj_p, 
	char *tag
	)
{
	/*
	 * Tag name will be validated against Email Product first
	 * and then in Org. tags
   	 */

	poid_t 		*pp = svc_obj_p;
	pin_flist_t 	*c_flistp = NULL;
	pin_flist_t 	*a_flistp = NULL;
	pin_errbuf_t 	ebuf;
	pin_cookie_t 	cookie = NULL;
	int32		elem_id;
	char		*ctag_name = NULL;
	int32		valid = PIN_BOOLEAN_FALSE;
	char            *obj_typep;
	char 		msg[512];

	PIN_ERR_CLEAR_ERR(&ebuf);

	obj_typep = (char *)PIN_POID_GET_TYPE(pp);
	if (obj_typep == (char *)NULL) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        	"valid_tag_msexchange : Error while getting Service type",
			 	&ebuf);
		return(valid);
	}

	c_flistp = _get_config_object(ctxp, pp, "/config/msexchange", 
		&ebuf);

	if (c_flistp == (pin_flist_t *)NULL){
		return(PIN_BOOLEAN_FALSE);
	}

	/* First validate E_Mail Provision tags first and then Org tags */

	while ((a_flistp = PIN_FLIST_ELEM_GET_NEXT(c_flistp,
		PIN_FLD_PROD_EMAIL_PROVISIONING_TAGS, &elem_id, 1,
		&cookie, &ebuf)) != (pin_flist_t *)NULL) {
		
		ctag_name = (char *) PIN_FLIST_FLD_GET(a_flistp,
			PIN_FLD_PROVISIONING_TAG, 0, &ebuf);
		if (ctag_name == NULL) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_WARNING,
				"NULL tag name in /config/msexchange object for email product ");
			continue; /* next element */
		}
		if (strcmp(ctag_name, tag) == 0) {
			valid = PIN_BOOLEAN_TRUE;
			break;
		}
	} /* while */

	/* If Provision doesn't match in email product, check with org. tags */
	if( ( valid == PIN_BOOLEAN_FALSE ) && 
		(!strcmp(obj_typep, PIN_MSEXCHANGE_SERVICE_TYPE_FIRSTADMIN)) ) {
		while ((a_flistp = PIN_FLIST_ELEM_GET_NEXT(c_flistp,
			PIN_FLD_PROD_ORG_PROVISIONING_TAGS, &elem_id, 1,
			&cookie, &ebuf)) != (pin_flist_t *)NULL) {
			
			ctag_name = (char *) PIN_FLIST_FLD_GET(a_flistp,
				PIN_FLD_PROVISIONING_TAG, 0, &ebuf);
			if (ctag_name == NULL) {
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_WARNING,
					"NULL tag name in /config/msexchange object for org product ");
				continue; /* next element */
			}
			if (strcmp(ctag_name, tag) == 0) {
				valid = PIN_BOOLEAN_TRUE;
				break;
			}
		} /* while */
	}

	/* clean up and return  */
	PIN_FLIST_DESTROY(c_flistp, NULL);
	return(valid);
}

/* 
 * retrieve the /config/msexchange object from the database and
 * collect the provisioning tags for given type of service
 */

static void 
tag_fn_msexchange(
	pcm_context_t *ctxp,
	poid_t *svc_obj_p,
	pin_flist_t *r_flistp, 
	int32 *eidp,
	pin_msexchange_service_type_t   service_received,
	pin_errbuf_t *ebufp
	)
{
	poid_t      	*pp = svc_obj_p;
	pin_flist_t 	*flistp = NULL;
	pin_flist_t 	*c_flistp = NULL;
	pin_flist_t 	*a_flistp = NULL;
	int32       	eid = *eidp;
	char        	*tag_name = NULL;
	pin_cookie_t    cookie = NULL;
	int32		elem_id;
	char            *obj_typep;
	char 		msg[512];


	if((service_received != PIN_MSEXCHANGE_SERVICE_USER) && (service_received != PIN_MSEXCHANGE_SERVICE_FIRSTADMIN)){
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        	"tag_fn_msexchange : Invalid Service Type",
			 	ebufp);
		return;

	}

	c_flistp = _get_config_object(ctxp, pp, "/config/msexchange",
		ebufp);

	if (c_flistp == (pin_flist_t *)NULL) {
		if (PIN_ERR_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"Error retrieving msexchange config object",
				ebufp);
		}
		/*  Else, config object not found, but no error. */
		return;
	}

	while ((a_flistp = PIN_FLIST_ELEM_GET_NEXT(c_flistp,
		PIN_FLD_PROD_EMAIL_PROVISIONING_TAGS, &elem_id, 1,
		&cookie, ebufp)) != (pin_flist_t *)NULL) {
		tag_name = (char *) PIN_FLIST_FLD_GET(a_flistp,
			PIN_FLD_PROVISIONING_TAG, 0, ebufp);
		if (tag_name == NULL || PIN_ERR_IS_ERR(ebufp)){
			PIN_FLIST_DESTROY(c_flistp, NULL);
                	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        	"msexchange config object Email Product tags: missing "
				"PIN_FLD_PROVISIONING_TAG",
			 	ebufp);
			return;
		}
		flistp = PIN_FLIST_ELEM_ADD(r_flistp,
			PIN_FLD_PROD_PROVISIONING_TAGS, eid, ebufp);
		eid++;
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_PROVISIONING_TAG,
			(void *)tag_name, ebufp);
		if( service_received == PIN_MSEXCHANGE_SERVICE_USER){
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_TYPE_STR,
				(void *)PIN_MSEXCHANGE_SERVICE_TYPE_USER, ebufp);
		}
		if ( service_received == PIN_MSEXCHANGE_SERVICE_FIRSTADMIN){
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_TYPE_STR,
				(void *)PIN_MSEXCHANGE_SERVICE_TYPE_FIRSTADMIN, ebufp);
		}
	} /* while */


	/* 
	 * If the service type is firstadmin, add provision tags for firstadmin
	 */
	cookie = NULL;
	if ( service_received == PIN_MSEXCHANGE_SERVICE_FIRSTADMIN){
		while ((a_flistp = PIN_FLIST_ELEM_GET_NEXT(c_flistp,
			PIN_FLD_PROD_ORG_PROVISIONING_TAGS, &elem_id, 1,
			&cookie, ebufp)) != (pin_flist_t *)NULL) {
			tag_name = (char *) PIN_FLIST_FLD_GET(a_flistp,
				PIN_FLD_PROVISIONING_TAG, 0, ebufp);
			if (tag_name == NULL || PIN_ERR_IS_ERR(ebufp)){
				PIN_FLIST_DESTROY(c_flistp, NULL);
	                	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        		"msexchange config object Org Product tags: missing "
					"PIN_FLD_PROVISIONING_TAG",
				 	ebufp);
				return;
			}
			flistp = PIN_FLIST_ELEM_ADD(r_flistp,
				PIN_FLD_PROD_PROVISIONING_TAGS, eid, ebufp);
			eid++;
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_PROVISIONING_TAG,
				(void *)tag_name, ebufp);
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_TYPE_STR,
				(void *)PIN_MSEXCHANGE_SERVICE_TYPE_FIRSTADMIN, ebufp);
		} /* while */
	}

	/* Debug what we sending out */
	 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			 "tag_fn_msexchange output flist", r_flistp);
	*eidp = eid;
	/* clean up */
	PIN_FLIST_DESTROY(c_flistp, NULL);
}

/*
 * retrieve the /config/msexchange object from the database and
 * collect the provisioning tags for E Mail Product
 */

static void 
tag_fn_msexchange_user(
	pcm_context_t *ctxp,
	poid_t *svc_obj_p,
	pin_flist_t *r_flistp, 
	int32 *eidp,
	pin_errbuf_t *ebufp
	)
{
	pin_msexchange_service_type_t user_service= PIN_MSEXCHANGE_SERVICE_USER;
	tag_fn_msexchange(ctxp, svc_obj_p,r_flistp,eidp,user_service,ebufp);
}

/*
 * retrieve the /config/msexchange object from the database and
 * collect the provisioning tags for Org. Product
 */


static void 
tag_fn_msexchange_firstadmin(
	pcm_context_t *ctxp,
	poid_t *svc_obj_p,
	pin_flist_t *r_flistp, 
	int32 *eidp,
	pin_errbuf_t *ebufp
	)
{
	pin_msexchange_service_type_t org_service= PIN_MSEXCHANGE_SERVICE_FIRSTADMIN;
	tag_fn_msexchange(ctxp, svc_obj_p,r_flistp,eidp, org_service,ebufp);
}

/*
 * This function will set the service attributes depending on the
 * provisioning tag for the proudct
 */

static void
plp_msexchange(
	pcm_context_t *ctxp, 
	poid_t *svc_obj_p,
	int32 buy,
	char *tag, 
	pin_errbuf_t *ebufp
	)
{
	pin_flist_t 	*flistp = (pin_flist_t *)NULL;
	pin_flist_t 	*sub_flistp = (pin_flist_t *)NULL;
	pin_flist_t 	*r_flistp = (pin_flist_t *)NULL;
	pin_flist_t	*ii_flistp = (pin_flist_t *)NULL;
	pin_flist_t     *c_flistp = (pin_flist_t *)NULL;
	pin_flist_t     *a_flistp = (pin_flist_t *)NULL;
	pin_flist_t     *org_flistp = (pin_flist_t *)NULL;
	pin_flist_t     *email_flistp = (pin_flist_t *)NULL;
	int 		product =0;
	int 		found_it = PIN_BOOLEAN_FALSE;
	char        	*tag_name = NULL;
	char            *obj_typep;
	char		epb[512];
	char		tag_str[256];
	poid_t      	*pp = svc_obj_p;
	int		*int_p = NULL;
	int32		mflags = PCM_OPFLG_ADD_ENTRY;
	int 		max_mbox_size,mbox_size_warn_limit,mbox_size_prohibit_limit;
	int 		max_out_msg_size,max_in_msg_size,max_recipient_count;
	int 		pop3_access,imap4_access,http_access;
	int 		max_user_count,default_max_mbox_size;
	int32		elem_id;
	pin_cookie_t    cookie = NULL;
	int		*ip = NULL;
	int 		svc_max_mbox_size=0;
	int 		svc_max_user_count=0;

	c_flistp = _get_config_object(ctxp, svc_obj_p, 
		"/config/msexchange", ebufp);
	if (c_flistp == (pin_flist_t *)NULL){
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"can't retrieve msexchange config object",
			ebufp);
		return;
	}
	obj_typep = (char *)PIN_POID_GET_TYPE(pp);
	if (obj_typep == (char *)NULL) {
		PIN_FLIST_DESTROY(c_flistp, NULL);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        	"plp_msexchange : Error while getting Service type",
			 	ebufp);
		return;
	}

	/* First check the tag name in Email Provisioning tags array */

	while ((a_flistp = PIN_FLIST_ELEM_GET_NEXT(c_flistp,
			PIN_FLD_PROD_EMAIL_PROVISIONING_TAGS, &elem_id, 1,
			&cookie, ebufp)) != (pin_flist_t *)NULL) {
		tag_name = (char *) PIN_FLIST_FLD_GET(a_flistp,
			PIN_FLD_PROVISIONING_TAG, 0, ebufp);
		if (tag_name == NULL || PIN_ERR_IS_ERR(ebufp)) {
			PIN_FLIST_DESTROY(c_flistp, NULL);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        	"msexchange config object Email Product tags: missing "
				"PIN_FLD_PROVISIONING_TAG",
		 		ebufp);
			return;
		}
		if (strcmp(tag, tag_name) == 0) { /* found it */
			found_it = PIN_BOOLEAN_TRUE;
			product = PIN_MSEXCHANGE_PRODUCT_EMAIL;
			break;
		}
	}

	cookie = NULL;
	if (!strcmp(obj_typep, PIN_MSEXCHANGE_SERVICE_TYPE_FIRSTADMIN)
				&& !found_it) {
		while ((a_flistp = PIN_FLIST_ELEM_GET_NEXT(c_flistp,
				PIN_FLD_PROD_ORG_PROVISIONING_TAGS, &elem_id, 1,
				&cookie, ebufp)) != (pin_flist_t *)NULL) {
			tag_name = (char *) PIN_FLIST_FLD_GET(a_flistp,
				PIN_FLD_PROVISIONING_TAG, 0, ebufp);
			if (tag_name == NULL || PIN_ERR_IS_ERR(ebufp)) {
				PIN_FLIST_DESTROY(c_flistp, NULL);
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        		"msexchange config object Org Product tags: missing "
					"PIN_FLD_PROVISIONING_TAG",
			 		ebufp);
				return;
			}
			if (strcmp(tag, tag_name) == 0) { /* found it */
				found_it = PIN_BOOLEAN_TRUE;
				product = PIN_MSEXCHANGE_PRODUCT_ORG;
				break;
			}
		}
	}

	if (!found_it) {
		/* No error, but tag not found in config object. */
		PIN_FLIST_DESTROY(c_flistp, NULL);
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_NONEXISTANT_ELEMENT, PIN_FLD_PROVISIONING_TAG,
			0, 0);
		pin_strlcpy(tag_str, tag, sizeof(tag_str));
		sprintf(epb, "can't find tag \"%s\" in config object",
			tag_str);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, epb, ebufp);
		return;
	}

	if (buy) {

			/*
			 * Read some Service Object fields to check whether provisioning
			 * product already purchased
			 */
			flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, svc_obj_p, ebufp);

			/* Get max. mbox size field from email product array */
			sub_flistp = PIN_FLIST_ELEM_ADD(flistp,
						PIN_FLD_MSEXCHANGE_USER_EMAIL, 0, ebufp);
			PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_MAX_MBOX_SIZE,
						&svc_max_mbox_size, ebufp);

			/* Get max. user count field from org product substruct (IF ANY) */
			if (!strcmp(obj_typep, PIN_MSEXCHANGE_SERVICE_TYPE_FIRSTADMIN))
			{
				sub_flistp = PIN_FLIST_SUBSTR_ADD(flistp,
							PIN_FLD_SERVICE_MSEXCHANGE_ORG, ebufp);
				PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_MAX_USER_COUNT,
							&svc_max_user_count, ebufp);
			}

			sub_flistp = NULL;

			PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, flistp, &r_flistp, ebufp);
			PIN_FLIST_DESTROY(flistp, NULL);

			if (PIN_ERR_IS_ERR(ebufp)) {
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                			"Error reading service object provisioning data", ebufp);
				PIN_FLIST_DESTROY(c_flistp, NULL);
				return;
			}


		 	if( product == PIN_MSEXCHANGE_PRODUCT_EMAIL){
				svc_max_mbox_size = PIN_MSEXCHANGE_INIT_STATE_INT_VALUE;
				email_flistp = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_MSEXCHANGE_USER_EMAIL,
						0, 1, ebufp);
				if (email_flistp != NULL){
					ip = (int *)PIN_FLIST_FLD_GET(email_flistp,
						PIN_FLD_MAX_MBOX_SIZE, 0, ebufp);
					if(ip != NULL){
						svc_max_mbox_size = *ip;
					}
				}

				/* Check whether Email Product already purchased */
				if(( svc_max_mbox_size != PIN_MSEXCHANGE_CANCELLED_STATE_INT_VALUE )  &&
					 	( svc_max_mbox_size != PIN_MSEXCHANGE_INIT_STATE_INT_VALUE )){
					PIN_FLIST_DESTROY(c_flistp, NULL);
					PIN_FLIST_DESTROY(r_flistp, NULL);
					pin_set_err(ebufp, PIN_ERRLOC_FM,
						PIN_ERRCLASS_APPLICATION,
						PIN_ERR_OP_ALREADY_DONE, PIN_FLD_MSEXCHANGE_USER_EMAIL,
						0, 0);
					sprintf(epb,
					"Email Provisioning product has already been purchased");
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, epb, ebufp);
					return;
				}
				int_p = (int *) PIN_FLIST_FLD_GET(a_flistp,
					PIN_FLD_MAX_MBOX_SIZE, 0, ebufp);
				if (int_p != NULL) {
					max_mbox_size = *int_p;
				}
				int_p = (int *) PIN_FLIST_FLD_GET(a_flistp,
					PIN_FLD_MBOX_SIZE_WARN_LIMIT, 0, ebufp);
				if (int_p != NULL) {
					mbox_size_warn_limit = *int_p;
				}
				int_p = (int *) PIN_FLIST_FLD_GET(a_flistp,
					PIN_FLD_MBOX_SIZE_PROHIBIT_LIMIT, 0, ebufp);
				if (int_p != NULL) {
					mbox_size_prohibit_limit = *int_p;
				}
				int_p = (int *) PIN_FLIST_FLD_GET(a_flistp,
					PIN_FLD_MAX_OUT_MSG_SIZE, 0, ebufp);
				if (int_p != NULL) {
					max_out_msg_size = *int_p;
				}
				int_p = (int *) PIN_FLIST_FLD_GET(a_flistp,
					PIN_FLD_MAX_IN_MSG_SIZE, 0, ebufp);
				if (int_p != NULL) {
					max_in_msg_size = *int_p;
				}
				int_p = (int *) PIN_FLIST_FLD_GET(a_flistp,
					PIN_FLD_MAX_RECIPIENT_COUNT, 0, ebufp);
				if (int_p != NULL) {
					max_recipient_count = *int_p;
				}
				int_p = (int *) PIN_FLIST_FLD_GET(a_flistp,
					PIN_FLD_POP3_ACCESS, 0, ebufp);
				if (int_p != NULL) {
					pop3_access = *int_p;
				}
				int_p = (int *) PIN_FLIST_FLD_GET(a_flistp,
					PIN_FLD_IMAP4_ACCESS, 0, ebufp);
				if (int_p != NULL) {
					imap4_access = *int_p;
				}
				int_p = (int *) PIN_FLIST_FLD_GET(a_flistp,
					PIN_FLD_HTTP_ACCESS, 0, ebufp);
				if (int_p != NULL) {
					http_access = *int_p;
				}
				if (PIN_ERR_IS_ERR(ebufp)) {
					PIN_FLIST_DESTROY(c_flistp, NULL);
					PIN_FLIST_DESTROY(r_flistp, NULL);
	                		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
	                        		"Problem retrieving msexchange email product config info",
			 			ebufp);
					return;
				}
			}
		 	if( product == PIN_MSEXCHANGE_PRODUCT_ORG){
				svc_max_user_count = PIN_MSEXCHANGE_INIT_STATE_INT_VALUE;
				org_flistp = PIN_FLIST_SUBSTR_GET(r_flistp, 
					PIN_FLD_SERVICE_MSEXCHANGE_ORG,  0, ebufp);
				if (org_flistp != NULL){
					ip = (int *)PIN_FLIST_FLD_GET(org_flistp,
						PIN_FLD_MAX_USER_COUNT, 1, ebufp);
					if(ip != NULL){
						svc_max_user_count = *ip;
					}
				}
				if(( svc_max_user_count != PIN_MSEXCHANGE_CANCELLED_STATE_INT_VALUE ) && 
					 ( svc_max_user_count != PIN_MSEXCHANGE_INIT_STATE_INT_VALUE )){
					PIN_FLIST_DESTROY(c_flistp, NULL);
					PIN_FLIST_DESTROY(r_flistp, NULL);
					pin_set_err(ebufp, PIN_ERRLOC_FM,
							PIN_ERRCLASS_APPLICATION,
							PIN_ERR_OP_ALREADY_DONE, PIN_FLD_SERVICE_MSEXCHANGE_ORG,
					0, 0);
	                		sprintf(epb,
						"Organization Provisioning product has already been purchased");
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, epb, ebufp);
					return;
				}
				int_p = (int *) PIN_FLIST_FLD_GET(a_flistp,
						PIN_FLD_MAX_USER_COUNT, 0, ebufp);
				if (int_p != NULL) {
					max_user_count = *int_p;
				}
				int_p = (int *) PIN_FLIST_FLD_GET(a_flistp,
						PIN_FLD_DEFAULT_MAX_MBOX_SIZE, 0, ebufp);
				if (int_p != NULL) {
					default_max_mbox_size = *int_p;
				}
				if (PIN_ERR_IS_ERR(ebufp)) {
					PIN_FLIST_DESTROY(c_flistp, NULL);
					PIN_FLIST_DESTROY(r_flistp, NULL);
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
	                        		"Problem retrieving msexchange Organization  product config info",
						ebufp);
					return;
				}
			}
		/* done with r_flistp */
		PIN_FLIST_DESTROY(r_flistp, NULL);

	} /* if (buy) */
	else {
		/*
		 * Cancelling product:
		 */
		 if( product == PIN_MSEXCHANGE_PRODUCT_EMAIL){
			max_mbox_size = PIN_MSEXCHANGE_CANCELLED_STATE_INT_VALUE;
			mbox_size_warn_limit= PIN_MSEXCHANGE_CANCELLED_STATE_INT_VALUE;
			mbox_size_prohibit_limit= PIN_MSEXCHANGE_CANCELLED_STATE_INT_VALUE;
			max_out_msg_size = PIN_MSEXCHANGE_CANCELLED_STATE_INT_VALUE;
			max_in_msg_size = PIN_MSEXCHANGE_CANCELLED_STATE_INT_VALUE;
			max_recipient_count=  PIN_MSEXCHANGE_CANCELLED_STATE_INT_VALUE;
			pop3_access= PIN_MSEXCHANGE_DISABLED_STATE;
			imap4_access= PIN_MSEXCHANGE_DISABLED_STATE;
			http_access= PIN_MSEXCHANGE_DISABLED_STATE;

		 }
		 if( product == PIN_MSEXCHANGE_PRODUCT_ORG){
			max_user_count = PIN_MSEXCHANGE_CANCELLED_STATE_INT_VALUE;
			default_max_mbox_size = PIN_MSEXCHANGE_CANCELLED_STATE_INT_VALUE;
		 }
	}
		
	/* done with c_flistp */
	PIN_FLIST_DESTROY(c_flistp, NULL);

	/* 
	 * Update service object with provisioning data.  Use
	 * PCM_OP_CUST_MODIFY_SERVICE, so that
	 * /event/notification/service/modify is generated.
	 */
	flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, svc_obj_p, ebufp);
	ii_flistp = PIN_FLIST_SUBSTR_ADD(flistp,
				PIN_FLD_INHERITED_INFO, ebufp);
	/* Add Arrary with index 0 */
	elem_id  = 0;
	if( product == PIN_MSEXCHANGE_PRODUCT_EMAIL){
		sub_flistp = PIN_FLIST_ELEM_ADD(ii_flistp,
					PIN_FLD_MSEXCHANGE_USER_EMAIL,elem_id, ebufp);
		PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_MAX_MBOX_SIZE,
					&max_mbox_size, ebufp);
		PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_MBOX_SIZE_WARN_LIMIT,
					&mbox_size_warn_limit, ebufp);
		PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_MBOX_SIZE_PROHIBIT_LIMIT,
					&mbox_size_prohibit_limit, ebufp);
		PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_MAX_OUT_MSG_SIZE,
					&max_out_msg_size, ebufp);
		PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_MAX_IN_MSG_SIZE,
					&max_in_msg_size, ebufp);
		PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_MAX_RECIPIENT_COUNT,
					&max_recipient_count, ebufp);
		PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_POP3_ACCESS,
					&pop3_access, ebufp);
		PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_IMAP4_ACCESS,
					&imap4_access, ebufp);
		PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_HTTP_ACCESS,
					&http_access, ebufp);
	}
	if( product == PIN_MSEXCHANGE_PRODUCT_ORG){
		sub_flistp = PIN_FLIST_SUBSTR_ADD(ii_flistp,
					PIN_FLD_SERVICE_MSEXCHANGE_ORG, ebufp);
		PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_MAX_USER_COUNT,
					&max_user_count, ebufp);
		PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_DEFAULT_MAX_MBOX_SIZE,
					&default_max_mbox_size, ebufp);
	}

	PCM_OP(ctxp, PCM_OP_CUST_MODIFY_SERVICE, mflags, flistp, &r_flistp, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                	"Error modifying service object with "
			"provisioning data", ebufp);
	}

	/* clean up */
	PIN_FLIST_DESTROY(r_flistp, NULL);
	PIN_FLIST_DESTROY(flistp, NULL);
}
/*****************************************************************************
 * Group of functions for /service/telephony
 *****************************************************************************/

/*
 * iterate the PIN_FLD_PROVISIONING_TAG_INFO array of the
 * /config/telephony_provision_tag object comparing values in
 * object with the argument; return PIN_BOOLEAN_TRUE if found,
 * PIN_BOOLEAN_FALSE otherwise
 */

static int32
valid_tag_telephony(
        pcm_context_t   *ctxp,
        poid_t          *svc_obj_p,
        char            *tag) {

        poid_t          *pp = svc_obj_p;
        pin_flist_t     *c_flistp = NULL;
        pin_flist_t     *a_flistp = NULL;
        pin_errbuf_t    ebuf;
        pin_cookie_t    cookie = NULL;
        int32           elem_id;
        int32           valid = PIN_BOOLEAN_FALSE;
        char            *product = NULL;
        char            *feature = NULL;
        char            ctag_name[PCM_MAXIMUM_STR];


        PIN_ERR_CLEAR_ERR(&ebuf);

        /* get provisioning tags from db */
        c_flistp = _get_config_object(ctxp, pp,
                "/config/telephony_provision_tags", &ebuf);

        /* return false if config object was not found */
        if (c_flistp == (pin_flist_t *)NULL) {

                return(PIN_BOOLEAN_FALSE);

        } /* end if */

        /* iterate flist comparing tag to values in config object of db */
        while ((a_flistp = PIN_FLIST_ELEM_GET_NEXT(c_flistp,
                PIN_FLD_PROVISIONING_TAG_INFO, &elem_id, 1,
                &cookie, &ebuf)) != (pin_flist_t *)NULL) {

                product = PIN_FLIST_FLD_GET(a_flistp,
                        PIN_FLD_PROV_PRINCIPLE_PRODUCT, 0, &ebuf);

                feature = PIN_FLIST_FLD_GET(a_flistp,
                        PIN_FLD_FEATURE_NAME, 1, &ebuf);

                if (feature != NULL && strcmp(feature, "")) {

                        sprintf(ctag_name, "%s;%s", product, feature);

                } /* end if */

                else {

                        sprintf(ctag_name, "%s", product);

                } /* end else */

                if (ctag_name == NULL) {

                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_WARNING,
                                "NULL values in /config/telephony_provision_"
                                "tags object");

                        continue; /* next element */

                } /* end if */

		if (strcmp(ctag_name, tag) == 0) {

                        valid = PIN_BOOLEAN_TRUE;

                        /* clean up and return  */
                        PIN_FLIST_DESTROY(c_flistp, NULL);
                        return(valid);

                } /* end if */

        } /* while */

        /*
		 * indicate provisioning tag in invalid because it doesn't match
         * an existing tag in db
		 */
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                "Invalid provisioning tag", &ebuf);

        /* clean up and return  */
        PIN_FLIST_DESTROY(c_flistp, NULL);
        return(valid);

} /* end valid_tag_telephony */

/*
 * retrieve the /config/telephony_provision_tags object from the database and
 * collect the provisioning tags
 */

static void
tag_fn_telephony(
        pcm_context_t   *ctxp,
        poid_t          *svc_obj_p,
        pin_flist_t     *r_flistp,
        int32           *eidp,
        pin_errbuf_t    *ebufp) {

        poid_t          *pp = svc_obj_p;
        pin_flist_t     *flistp = NULL;
        pin_flist_t     *c_flistp = NULL;
        pin_flist_t     *a_flistp = NULL;
        pin_cookie_t    cookie = NULL;
        char            *product = NULL;
        char            *feature = NULL;
        char            ctag_name[PCM_MAXIMUM_STR];
        int32           eid = *eidp;
        int32           elem_id;

        /*
		 * get provisioning tags from db
		 */
        c_flistp = _get_config_object(ctxp, pp,
                "/config/telephony_provision_tags", ebufp);

        /*
		 * check to see if tags were returned
		 */
        if (c_flistp == (pin_flist_t *)NULL) {
                return;
        } /* end if */

        /* process all tags returned from db into format for drop-down list
         * in pricing tool GUI: <principle_product>;<feature_name> or
         * <principle_product> if feature name does not exist
		 */

        while ((a_flistp = PIN_FLIST_ELEM_GET_NEXT(c_flistp,
                PIN_FLD_PROVISIONING_TAG_INFO, &elem_id, 1,
                &cookie, ebufp)) != (pin_flist_t *)NULL) {

                product = PIN_FLIST_FLD_GET(a_flistp,
                        PIN_FLD_PROV_PRINCIPLE_PRODUCT, 0, ebufp);

                feature = PIN_FLIST_FLD_GET(a_flistp,
                        PIN_FLD_FEATURE_NAME, 1, ebufp);

                if (feature != NULL && strcmp(feature, "")) {

                        sprintf(ctag_name, "%s;%s", product, feature);

                } /* end if */

                else {

                        sprintf(ctag_name, "%s", product);

                } /* end else */

                /* display error if field is not found */
                if (ctag_name == NULL || PIN_ERR_IS_ERR(ebufp)) {

                        PIN_FLIST_DESTROY(c_flistp, NULL);

                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "Telephony provision tags config object: missing "
                                "PIN_FLD_PROV_PRINCIPLE_PRODUCT",
                                ebufp);

                        return;

                } /* end if */

		/* add service type and provisioning tag back in flist */
                flistp = PIN_FLIST_ELEM_ADD(r_flistp,
                        PIN_FLD_PROD_PROVISIONING_TAGS, eid, ebufp);

                /* increment provisioning tags array index */
                eid++;

                PIN_FLIST_FLD_SET(flistp, PIN_FLD_TYPE_STR,
                        (void *)"/service/telephony", ebufp);

                PIN_FLIST_FLD_SET(flistp, PIN_FLD_PROVISIONING_TAG,
                        (void *)ctag_name, ebufp);

        } /* while */

        *eidp = eid;

        /* clean up */
        PIN_FLIST_DESTROY(c_flistp, NULL);

} /* end tag_fn_telephony */

/* This function has no functionality because the service object is
 * not populated in this opcode.
 */

static void
plp_telephony(
        pcm_context_t   *ctxp,
        poid_t          *svc_obj_p,
        int32           buy,
        char            *tag,
        pin_errbuf_t    *ebufp) {

} /* end plp_telephony */

/*****************************************************************************
 * Group of functions for /service/dsl
 *****************************************************************************/

/*
 * iterate the PIN_FLD_PROVISIONING_TAG_INFO array of the
 * /config/dsl_provision_tag object comparing values in
 * object with the argument; return PIN_BOOLEAN_TRUE if found,
 * PIN_BOOLEAN_FALSE otherwise
 */

static int32
valid_tag_dsl(
        pcm_context_t   *ctxp,
        poid_t          *svc_obj_p,
        char            *tag) {

        poid_t          *pp = svc_obj_p;
        pin_flist_t     *c_flistp = NULL;
        pin_flist_t     *a_flistp = NULL;
        pin_errbuf_t    ebuf;
        pin_cookie_t    cookie = NULL;
        int32           elem_id;
        int32           valid = PIN_BOOLEAN_FALSE;
        char            *tag_name;


        PIN_ERR_CLEAR_ERR(&ebuf);

        /* get provisioning tags from db */
        c_flistp = _get_config_object(ctxp, pp,
                "/config/dsl_provision_tags", &ebuf);

        /* return false if config object was not found */
        if (c_flistp == (pin_flist_t *)NULL) {

                return(PIN_BOOLEAN_FALSE);

        } /* end if */

        /* iterate flist comparing tag to values in config object of db */
        while ((a_flistp = PIN_FLIST_ELEM_GET_NEXT(c_flistp,
                PIN_FLD_PROVISIONING_TAG_INFO, &elem_id, 1,
                &cookie, &ebuf)) != (pin_flist_t *)NULL) {

                tag_name = (char *) PIN_FLIST_FLD_GET(a_flistp,
                        PIN_FLD_PROVISIONING_TAG, 0, &ebuf);

                if (tag_name == NULL) {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_WARNING,
                                "NULL tag name in /config/dsl_provision_tags "
                                "object");

                        continue; /* next element */

                } /* end if */

                if (strcmp(tag_name, tag) == 0) {

                        valid = PIN_BOOLEAN_TRUE;
                        break;

                } /* end if */

        } /* while */

        /* clean up and return  */
        PIN_FLIST_DESTROY(c_flistp, NULL);
        return(valid);

} /* end valid_tag_dsl */

/*
 * retrieve the /config/dsl_provision_tags object from the database and
 * collect the provisioning tags
 */
static void
tag_fn_dsl(
        pcm_context_t   *ctxp,
        poid_t          *svc_obj_p,
        pin_flist_t     *r_flistp,
        int32           *eidp,
        pin_errbuf_t    *ebufp) {

        poid_t          *pp = svc_obj_p;
        pin_flist_t     *flistp = NULL;
        pin_flist_t     *c_flistp = NULL;
        pin_flist_t     *a_flistp = NULL;
        int32           eid = *eidp;
        char            *tag_name = NULL;
        pin_cookie_t    cookie = NULL;
        int32           elem_id;

        c_flistp = _get_config_object(ctxp, pp, "/config/dsl_provision_tags",
                ebufp);

        if (c_flistp == (pin_flist_t *)NULL) {
			return;
        } /* end if */

        while ((a_flistp = PIN_FLIST_ELEM_GET_NEXT(c_flistp,
                PIN_FLD_PROVISIONING_TAG_INFO, &elem_id, 1,
                &cookie, ebufp)) != (pin_flist_t *)NULL) {

                tag_name = (char *) PIN_FLIST_FLD_GET(a_flistp,
                        PIN_FLD_PROVISIONING_TAG, 0, ebufp);

                if (tag_name == NULL || PIN_ERR_IS_ERR(ebufp)){
                        PIN_FLIST_DESTROY(c_flistp, NULL);
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "dsl provision tags config object: missing "
                                "PIN_FLD_PROVISIONING_TAG",
                                ebufp);

                        return;

                } /* end if */

                flistp = PIN_FLIST_ELEM_ADD(r_flistp,
                        PIN_FLD_PROD_PROVISIONING_TAGS, eid, ebufp);

                eid++;

                PIN_FLIST_FLD_SET(flistp, PIN_FLD_TYPE_STR,
                        (void *)"/service/dsl", ebufp);

                PIN_FLIST_FLD_SET(flistp, PIN_FLD_PROVISIONING_TAG,
                        (void *)tag_name, ebufp);

        } /* while */

        *eidp = eid;

        /* clean up */
        PIN_FLIST_DESTROY(c_flistp, NULL);

} /* end tag_fn_dsl */

static void
plp_dsl(
	pcm_context_t   *ctxp,
	poid_t          *svc_obj_p,
	int32           buy,
	char            *tag,
	pin_errbuf_t    *ebufp) {

	return;

} /* end plp_dsl */

/*****************************************************************
 * Start of provisioning tag support functions for /service/video
 *
 *****************************************************************/

/****************************************************************************
* Function     : valid_tag_bel
* Description  : iterate the PIN_FLD_PROD_PROVISIONING_TAGS array of the
*              : /config/bel_provision_tags object comparing each
*              : PIN_FLD_PROVISIONING_TAG with the argument passed in
*--------------+-------------------------------------------------------------
*  In          : ctxp           - context pointer
*  In          : svc_obj_p      - service object poid
*  In          : tag        - passed in tag to validate
*--------------+-------------------------------------------------------------
* Returns      : return PIN_BOOLEAN_TRUE if found, PIN_BOOLEAN_FALSE otherwise
****************************************************************************/
static int32
valid_tag_bel(
	pcm_context_t   *ctxp,
	poid_t          *svc_obj_p,
	char            *tag
	)
{
	poid_t          *pp = svc_obj_p;
	pin_flist_t     *c_flistp = NULL;
	pin_flist_t     *a_flistp = NULL;
	pin_errbuf_t    ebuf;
	pin_cookie_t    cookie = NULL;
	int32           elem_id;
	char            *ctag_name = NULL;
	int32           valid = PIN_BOOLEAN_FALSE;

	PIN_ERR_CLEAR_ERR(&ebuf);

	c_flistp = _get_config_object(ctxp, pp, "/config/bel_provision_tags",
		&ebuf);
	if (c_flistp == (pin_flist_t *)NULL){
		return(PIN_BOOLEAN_FALSE);
	}

	/* iterate flist comparing tag to PIN_FLD_PROVISIONING_TAG */
	while ((a_flistp = PIN_FLIST_ELEM_GET_NEXT(c_flistp,
		PIN_FLD_PROD_PROVISIONING_TAGS, &elem_id, 1,
		&cookie, &ebuf)) != (pin_flist_t *)NULL) {

		ctag_name = (char *) PIN_FLIST_FLD_GET(a_flistp,
			PIN_FLD_PROVISIONING_TAG, 0, &ebuf);
		if (ctag_name == NULL) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_WARNING,
			"NULL tag name in /config/bel_provision_tags object");
			continue; /* next element */
		}
		if (strcmp(ctag_name, tag) == 0) {
			valid = PIN_BOOLEAN_TRUE;
			break;
		}
	} /* while */

	/* clean up and return  */
	PIN_FLIST_DESTROY(c_flistp, NULL);
	return(valid);
}


/****************************************************************************
* Function     : tag_fn_bel
* Description  : retrieve the /config/bel_provision_tags object from the
*              : database and collect the provisioning tags
*--------------+-------------------------------------------------------------
*  In          : ctxp           - context pointer
*  In          : svc_obj_p      - service object poid
*  Out         : r_flistp       - return flist
*  Out         : eidp           - array element count
*  Out         : ebufp          - error buf
*--------------+-------------------------------------------------------------
* Returns      : void
****************************************************************************/
static void
tag_fn_bel(
	pcm_context_t   *ctxp,
	poid_t          *svc_obj_p,
	pin_flist_t     *r_flistp,
	int32           *eidp,
	pin_errbuf_t    *ebufp
	)
{
	poid_t                  *pp = svc_obj_p;
	pin_flist_t             *flistp = NULL;
	pin_flist_t             *c_flistp = NULL;
	pin_flist_t             *a_flistp = NULL;
	int32                   eid = *eidp;
	char                    *tag_name = NULL;
	pin_cookie_t            cookie = NULL;
	int32                   elem_id;

	c_flistp = _get_config_object(ctxp, pp, "/config/bel_provision_tags",
		ebufp);
	if (c_flistp == (pin_flist_t *)NULL) {
			if (PIN_ERR_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"Error retrieving bel_provision_tags config object",
			ebufp);
		}
		/*  Else, config object not found, but no error. */
		return;
	}

	while ((a_flistp = PIN_FLIST_ELEM_GET_NEXT(c_flistp,
		PIN_FLD_PROD_PROVISIONING_TAGS, &elem_id, 1,
		&cookie, ebufp)) != (pin_flist_t *)NULL) {
		tag_name = (char *) PIN_FLIST_FLD_GET(a_flistp,
			PIN_FLD_PROVISIONING_TAG, 0, ebufp);
		if (tag_name == NULL || PIN_ERR_IS_ERR(ebufp)){
			PIN_FLIST_DESTROY(c_flistp, NULL);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"bel_provision_tags config object: missing "
				"PIN_FLD_PROVISIONING_TAG",
				ebufp);
			break;
                        /****/
		}
		flistp = PIN_FLIST_ELEM_ADD(r_flistp,
			PIN_FLD_PROD_PROVISIONING_TAGS, eid, ebufp);
		eid++;
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_TYPE_STR,
			(void *)"/service/video", ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_PROVISIONING_TAG,
			(void *)tag_name, ebufp);
	} /* while */

	*eidp = eid;
	/* clean up */
	PIN_FLIST_DESTROY(c_flistp, NULL);
}

/****************************************************************************
* Function     : plp_bel
* Description  : For BEL this function will do nothing...Needed for whole
*              : provisioning tag scheme to work
*--------------+-------------------------------------------------------------
*  In          : ctxp           - context pointer
*  In          : svc_obj_p      - service object poid
*  In          : buy            - buy/cancel flag
*  In          : tag            - passed in provisioning tag
*  Out         : ebufp          - error buf
*--------------+-------------------------------------------------------------
* Returns      : void
****************************************************************************/
static void
plp_bel(
	pcm_context_t   *ctxp,
	poid_t          *svc_obj_p,
	int32           buy,
	char            *tag,
	pin_errbuf_t    *ebufp
	)
{
	return;
}

/*
 * Validates the tag for given gsm telephony service
 */
static int32
valid_tag_gsm_telephony(
	pcm_context_t *ctxp,
	poid_t *svc_obj_p,
	char *tag
	)
{
	return (int32)valid_tag_gsm(ctxp, svc_obj_p, tag, PIN_OBJ_TYPE_CONFIG_GSMTAGS_TEL);
}

/*
 * Validates the tag for given gsm sms service
 */
static int32
valid_tag_gsm_sms(
	pcm_context_t *ctxp,
	poid_t *svc_obj_p,
	char *tag
	)
{
	return (int32)valid_tag_gsm(ctxp, svc_obj_p, tag, PIN_OBJ_TYPE_CONFIG_GSMTAGS_SMS);
}

/*
 * Validates the tag for given gsm fax service
 */
static int32
valid_tag_gsm_fax(
	pcm_context_t *ctxp,
	poid_t *svc_obj_p,
	char *tag
	)
{
	return (int32)valid_tag_gsm(ctxp, svc_obj_p, tag, PIN_OBJ_TYPE_CONFIG_GSMTAGS_FAX);
}

/*
 * Validates the tag for given gsm data service
 */
static int32
valid_tag_gsm_data(
	pcm_context_t *ctxp,
	poid_t *svc_obj_p,
	char *tag
	)
{
	return (int32)valid_tag_gsm(ctxp, svc_obj_p, tag, PIN_OBJ_TYPE_CONFIG_GSMTAGS_DATA);
}

/*
 * Validates the tag for given gsm service specified in service_recd
 */

static int32
valid_tag_gsm(
	pcm_context_t           *ctxp,
	poid_t                  *svc_obj_p,
	char                    *tag,
	char                    *config_recd
	)
{
	poid_t          *pp = svc_obj_p;
	pin_flist_t     *c_flistp = NULL;
	pin_flist_t     *a_flistp = NULL;
	pin_errbuf_t    ebuf;
	pin_cookie_t    cookie = NULL;
	int32           elem_id;
	char            *ctag_name = NULL;
	int32           valid = PIN_BOOLEAN_FALSE;
	char            *obj_typep = NULL;
	char            obj_type_str[PCM_MAX_POID_TYPE];
	char            msg[512];

	PIN_ERR_CLEAR_ERR(&ebuf);

	obj_typep = (char *)PIN_POID_GET_TYPE(pp);
	memset(msg, '\0', 512);

	if (obj_typep == (char *)NULL) {
		pin_set_err(&ebuf, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_POID_TYPE, PIN_FLD_POID, 0, 0);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"valid_tag_gsm : Error while getting Service type for"
			" config obj : ", &ebuf);
		return(valid);
	}

	c_flistp = _get_config_object(ctxp, pp, config_recd,
		&ebuf);

	if (c_flistp == (pin_flist_t *)NULL){
		pin_strlcpy(obj_type_str, obj_typep, PCM_MAX_POID_TYPE);
		sprintf(msg, "No config object found for object : %s", obj_type_str);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_WARNING, msg);
		return(PIN_BOOLEAN_FALSE);
	}

	/*  Walk array of tags, comparing to our input */

	while ((a_flistp = PIN_FLIST_ELEM_GET_NEXT(c_flistp,
		PIN_FLD_PROD_PROVISIONING_TAGS, &elem_id, 1,
		&cookie, &ebuf)) != (pin_flist_t *)NULL) {

		ctag_name = (char *) PIN_FLIST_FLD_GET(a_flistp,
			PIN_FLD_PROVISIONING_TAG, 0, &ebuf);
		if (ctag_name == NULL) {
			PIN_FLIST_DESTROY(c_flistp, NULL);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"valid_tag_gsm : NULL provisioning_tag in "
				" config obj : ", &ebuf);
			return(PIN_BOOLEAN_FALSE);
		}
		if (strcmp(ctag_name, tag) == 0) {
			valid = PIN_BOOLEAN_TRUE;
			break;
		}
	} /* while */

	/* clean up and return  */
	PIN_FLIST_DESTROY(c_flistp, NULL);
	return(valid);
} /* valid_tag_gsm */



/*
 * retrieve the /config/gsmtags/telephony object from the database and
 * collect the provisioning tags
 */

static void
tag_fn_gsm_telephony(
	pcm_context_t *ctxp,
	poid_t *svc_obj_p,
	pin_flist_t *r_flistp,
	int32 *eidp,
	pin_errbuf_t *ebufp
	)
{
	tag_fn_gsm(ctxp, svc_obj_p,r_flistp,eidp,PIN_OBJ_TYPE_SERVICE_GSM_TEL,
		PIN_OBJ_TYPE_CONFIG_GSMTAGS_TEL,ebufp);
}


/*
 * retrieve the /config/gsmtags/sms object from the database and
 * collect the provisioning tags
 */

static void
tag_fn_gsm_sms(
	pcm_context_t *ctxp,
	poid_t *svc_obj_p,
	pin_flist_t *r_flistp,
	int32 *eidp,
	pin_errbuf_t *ebufp
	)
{
	tag_fn_gsm(ctxp, svc_obj_p,r_flistp,eidp,PIN_OBJ_TYPE_SERVICE_GSM_SMS,
		PIN_OBJ_TYPE_CONFIG_GSMTAGS_SMS,ebufp);
}

/*
 * retrieve the /config/gsmtags/fax object from the database and
 * collect the provisioning tags
 */

static void
tag_fn_gsm_fax(
	pcm_context_t *ctxp,
	poid_t *svc_obj_p,
	pin_flist_t *r_flistp,
	int32 *eidp,
	pin_errbuf_t *ebufp
	)
{
	tag_fn_gsm(ctxp, svc_obj_p,r_flistp,eidp,PIN_OBJ_TYPE_SERVICE_GSM_FAX,
		PIN_OBJ_TYPE_CONFIG_GSMTAGS_FAX,ebufp);
}

/*
 * retrieve the /config/gsmtags/data object from the database and
 * collect the provisioning tags
 */

static void
tag_fn_gsm_data(
	pcm_context_t *ctxp,
	poid_t *svc_obj_p,
	pin_flist_t *r_flistp,
	int32 *eidp,
	pin_errbuf_t *ebufp
	)
{
	tag_fn_gsm(ctxp, svc_obj_p,r_flistp,eidp,PIN_OBJ_TYPE_SERVICE_GSM_DATA,
		PIN_OBJ_TYPE_CONFIG_GSMTAGS_DATA,ebufp);
}


/*
 * retrieve the /config/gsmtags/xxx object from the database and
 * collect the provisioning tags for given type of service
 */

static void
tag_fn_gsm(
	pcm_context_t *ctxp,
	poid_t *svc_obj_p,
	pin_flist_t *r_flistp,
	int32 *eidp,
	char    *service_recd,
	char    *config_recd,
	pin_errbuf_t *ebufp
	)
{
	poid_t          *pp = svc_obj_p;
	pin_flist_t     *flistp = NULL;
	pin_flist_t     *c_flistp = NULL;
	pin_flist_t     *a_flistp = NULL;
	int32           eid = *eidp;
	char            *tag_name = NULL;
	pin_cookie_t    cookie = NULL;
	int32           elem_id;
	char            msg[512];

	memset(msg, '\0', 512);
	if(strcmp(service_recd, PIN_OBJ_TYPE_SERVICE_GSM_TEL) &&
		strcmp(service_recd, PIN_OBJ_TYPE_SERVICE_GSM_SMS) &&
		strcmp(service_recd, PIN_OBJ_TYPE_SERVICE_GSM_FAX) &&
		strcmp(service_recd, PIN_OBJ_TYPE_SERVICE_GSM_DATA)) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_NO_MATCH, 0, 0, 0);
		sprintf(msg, "tag_fn_gsm_xxx : Invalid Service Type : %s", service_recd);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, msg, ebufp);
		return;

	}

	c_flistp = _get_config_object(ctxp, pp, config_recd,
		ebufp);

	if (c_flistp == (pin_flist_t *)NULL) {
		if (PIN_ERR_IS_ERR(ebufp)) {
			sprintf(msg, "Error retrieving gsm config object xxx : %s", config_recd);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, msg, ebufp);
		}
		/*  Else, config object not found, but no error. */
		return;
	}

	while ((a_flistp = PIN_FLIST_ELEM_GET_NEXT(c_flistp,
		PIN_FLD_PROD_PROVISIONING_TAGS, &elem_id, 1,
		&cookie, ebufp)) != (pin_flist_t *)NULL) {
		tag_name = (char *) PIN_FLIST_FLD_GET(a_flistp,
			PIN_FLD_PROVISIONING_TAG, 0, ebufp);
		if (tag_name == NULL || PIN_ERR_IS_ERR(ebufp)){
			PIN_FLIST_DESTROY(c_flistp, NULL);
			sprintf(msg, "gsm config object tag: missing PIN_FLD_PROVISIONING_TAG : %s",
				config_recd);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, msg, ebufp);
			return;
		}
		flistp = PIN_FLIST_ELEM_ADD(r_flistp,
			PIN_FLD_PROD_PROVISIONING_TAGS, eid, ebufp);
		eid++;
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_PROVISIONING_TAG,
			(void *)tag_name, ebufp);

		PIN_FLIST_FLD_SET(flistp, PIN_FLD_TYPE_STR,
			service_recd, ebufp);
	} /* while */

	/* Debug what we sending out */
	sprintf(msg, "tag_fn_gsm_xxx output flist : %s", config_recd);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg, r_flistp);
	*eidp = eid;
	/* clean up */
	PIN_FLIST_DESTROY(c_flistp, NULL);
} /* tag_fn_gsm */

static void
plp_gsm_telephony(
	pcm_context_t   *ctxp,
	poid_t          *svc_obj_p,
	int32           buy,
	char            *tag,
	pin_errbuf_t    *ebufp) {

	return;

} /* end plp_gsm_telephony */

static void
plp_gsm_sms(
	pcm_context_t   *ctxp,
	poid_t          *svc_obj_p,
	int32           buy,
	char            *tag,
	pin_errbuf_t    *ebufp) {

	return;

} /* end plp_gsm_sms */

static void
plp_gsm_data(
	pcm_context_t   *ctxp,
	poid_t          *svc_obj_p,
	int32           buy,
	char            *tag,
	pin_errbuf_t    *ebufp) {

	return;

} /* end plp_gsm_data */

static void
plp_gsm_fax(
	pcm_context_t   *ctxp,
	poid_t          *svc_obj_p,
	int32           buy,
	char            *tag,
	pin_errbuf_t    *ebufp) {

	return;

} /* end plp_gsm_fax */

/*
 * Validates the tag for given telco service
 */
static int32
valid_tag_tcf(
	pcm_context_t *ctxp,
	poid_t *svc_obj_p,
	char *tag
	)
{
	return (int32)valid_tag_tcf_telco(ctxp, svc_obj_p, tag, TELCO_CONFIG);
}

static int32
valid_tag_gsm_tcf(
	pcm_context_t *ctxp,
	poid_t *svc_obj_p,
	char *tag
	)
{
	return (int32)valid_tag_tcf_telco(ctxp, svc_obj_p, tag, TELCO_GSM_CONFIG);
}

static int32
valid_tag_gsm_data_tcf(
	pcm_context_t *ctxp,
	poid_t *svc_obj_p,
	char *tag
	)
{
	return (int32)valid_tag_tcf_telco(ctxp, svc_obj_p, tag, TELCO_GSM_DATA_CONFIG);
}

static int32
valid_tag_gsm_fax_tcf(
	pcm_context_t *ctxp,
	poid_t *svc_obj_p,
	char *tag
	)
{
	return (int32)valid_tag_tcf_telco(ctxp, svc_obj_p, tag, TELCO_GSM_FAX_CONFIG);
}

static int32
valid_tag_gsm_sms_tcf(
	pcm_context_t *ctxp,
	poid_t *svc_obj_p,
	char *tag
	)
{
	return (int32)valid_tag_tcf_telco(ctxp, svc_obj_p, tag, TELCO_GSM_SMS_CONFIG);
}

static int32
valid_tag_gsm_telephony_tcf(
	pcm_context_t *ctxp,
	poid_t *svc_obj_p,
	char *tag
	)
{
	return (int32)valid_tag_tcf_telco(ctxp, svc_obj_p, tag, TELCO_GSM_TELEPHONY_CONFIG);
}

static int32
valid_tag_pdc_tcf( pcm_context_t *ctxp, poid_t *svc_obj_p, char *tag)
{
        return (int32)valid_tag_tcf_telco(ctxp, svc_obj_p, tag, TELCO_PDC_CONFIG);
}

static int32
valid_tag_imt_tcf( pcm_context_t *ctxp, poid_t *svc_obj_p, char *tag)
{
        return (int32)valid_tag_tcf_telco(ctxp, svc_obj_p, tag, TELCO_IMT_CONFIG);
}

static int32
valid_tag_gprs_tcf( pcm_context_t *ctxp, poid_t *svc_obj_p, char *tag)
{
        return (int32)valid_tag_tcf_telco(ctxp, svc_obj_p, tag, TELCO_GPRS_CONFIG);
}

/*
 * group of functions for /service/telco
 */
static void
tag_fn_tcf(pcm_context_t *ctxp, poid_t *svc_obj_p, 
	pin_flist_t *r_flp, int32 *eidp, pin_errbuf_t *ebufp)
{
	tag_fn_telco(ctxp, svc_obj_p, r_flp, eidp, ebufp, TELCO_CONFIG, 
		TELCO_SERVICE_PROFILE);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"tag_fn_tcf return flist",	r_flp);
}

static void
tag_fn_gsm_tcf(pcm_context_t *ctxp, poid_t *svc_obj_p, 
	pin_flist_t *r_flp, int32 *eidp, pin_errbuf_t *ebufp)
{
	tag_fn_telco(ctxp, svc_obj_p, r_flp, eidp, ebufp, TELCO_GSM_CONFIG, 
		TELCO_GSM_SERVICE_PROFILE);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"tag_fn_gsm_tcf return flist",	r_flp);
}

static void
tag_fn_gsm_data_tcf(pcm_context_t *ctxp, poid_t *svc_obj_p, 
	pin_flist_t *r_flp, int32 *eidp, pin_errbuf_t *ebufp)
{
	tag_fn_telco(ctxp, svc_obj_p, r_flp, eidp, ebufp, TELCO_GSM_DATA_CONFIG, 
		TELCO_GSM_DATA_SERVICE_PROFILE);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"tag_fn_gsm_data_tcf return flist",	r_flp);
}

static void
tag_fn_gsm_fax_tcf(pcm_context_t *ctxp, poid_t *svc_obj_p, 
	pin_flist_t *r_flp, int32 *eidp, pin_errbuf_t *ebufp)
{
	tag_fn_telco(ctxp, svc_obj_p, r_flp, eidp, ebufp, TELCO_GSM_FAX_CONFIG, 
		TELCO_GSM_FAX_SERVICE_PROFILE);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"tag_fn_gsm_fax_tcf return flist",	r_flp);
}

static void
tag_fn_gsm_sms_tcf(pcm_context_t *ctxp, poid_t *svc_obj_p, 
	pin_flist_t *r_flp, int32 *eidp, pin_errbuf_t *ebufp)
{
	tag_fn_telco(ctxp, svc_obj_p, r_flp, eidp, ebufp, TELCO_GSM_SMS_CONFIG, 
		TELCO_GSM_SMS_SERVICE_PROFILE);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"tag_fn_gsm_sms_tcf return flist",	r_flp);
}

static void
tag_fn_gsm_telephony_tcf(pcm_context_t *ctxp, poid_t *svc_obj_p, 
	pin_flist_t *r_flp, int32 *eidp, pin_errbuf_t *ebufp)
{
	tag_fn_telco(ctxp, svc_obj_p, r_flp, eidp, ebufp, TELCO_GSM_TELEPHONY_CONFIG, 
		TELCO_GSM_TELEPHONY_SERVICE_PROFILE);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"tag_fn_gsm_telephony_tcf return flist",	r_flp);
}

static void
tag_fn_pdc_tcf(pcm_context_t *ctxp, poid_t *svc_obj_p, pin_flist_t *r_flp, 
			int32 *eidp, pin_errbuf_t *ebufp) {
        tag_fn_telco(ctxp, svc_obj_p, r_flp, eidp, ebufp, 
		TELCO_PDC_CONFIG, TELCO_PDC_SERVICE_PROFILE);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tag_fn_gsm_tcf "
				"return flist",  r_flp);
}

static void
tag_fn_imt_tcf(pcm_context_t *ctxp, poid_t *svc_obj_p, pin_flist_t *r_flp, 
		int32 *eidp, pin_errbuf_t *ebufp) {
        tag_fn_telco(ctxp, svc_obj_p, r_flp, eidp, ebufp, TELCO_IMT_CONFIG,
		 TELCO_IMT_SERVICE_PROFILE);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tag_fn_gsm_tcf "
				"return flist",  r_flp);
}

static void
tag_fn_gprs_tcf(pcm_context_t *ctxp, poid_t *svc_obj_p, pin_flist_t *r_flp,
                        int32 *eidp, pin_errbuf_t *ebufp) {
        tag_fn_telco(ctxp, svc_obj_p, r_flp, eidp, ebufp,
                TELCO_GPRS_CONFIG, TELCO_GPRS_SERVICE_PROFILE);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tag_fn_gprs_tcf "
                                "return flist",  r_flp);
}

/*
 * For the generalized telco framework, the provisioning is
 * triggered through service management APIs and the frame
 * work provisioning platform
 */  
static void
plp_tcf(
	pcm_context_t   *ctxp,
	poid_t          *svc_obj_p,
	int32           buy,
	char            *tag,
	pin_errbuf_t    *ebufp) {

	return;

} /* end plp_tcf */

static void
plp_gsm_tcf(
	pcm_context_t   *ctxp,
	poid_t          *svc_obj_p,
	int32           buy,
	char            *tag,
	pin_errbuf_t    *ebufp) {

	return;

} /* end plp_gsm_tcf */

static void plp_pdc_tcf(
        pcm_context_t   *ctxp,
        poid_t          *svc_obj_p,
        int32           buy,
        char            *tag,
        pin_errbuf_t    *ebufp) {

        return;

} /* end plp_pdc_tcf */

static void plp_imt_tcf(
        pcm_context_t   *ctxp,
        poid_t          *svc_obj_p,
        int32           buy,
        char            *tag,
        pin_errbuf_t    *ebufp) {

        return;

} /* end plp_imt_tcf */

static void plp_gprs_tcf(
        pcm_context_t   *ctxp,
        poid_t          *svc_obj_p,
        int32           buy,
        char            *tag,
        pin_errbuf_t    *ebufp) {

        return;

} /* end plp_gprs_tcf */

/*
 * Validates the tag for given telco service specified in service_recd
 */

static int32
valid_tag_tcf_telco(
	pcm_context_t		*ctxp,
	poid_t			*svc_obj_p,
	char			*tag,
	char			*config_recd
	)
{
	poid_t		*pp = svc_obj_p;
	pin_flist_t	*c_flistp = NULL;
	pin_flist_t	*a_flistp = NULL;
	pin_errbuf_t	ebuf;
	pin_cookie_t	cookie = NULL;
	int32		elem_id;
	char		*ctag_name = NULL;
	int32		valid = PIN_BOOLEAN_FALSE;
	char		*obj_typep;
	char		msg[512];

	PIN_ERR_CLEAR_ERR(&ebuf);

	obj_typep = (char *)PIN_POID_GET_TYPE(pp);
	memset(msg, '\0', 512);

	if (obj_typep == (char *)NULL) {
		pin_set_err(&ebuf, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_POID_TYPE, PIN_FLD_POID, 0, 0);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"valid_tag_tcf : Error while getting Service type for"
			" config obj : ", &ebuf);
		return(valid);
	}

	c_flistp = _get_config_object(ctxp, pp, config_recd,
		&ebuf);

	if (c_flistp == (pin_flist_t *)NULL){
		sprintf(msg, "No config object found for object : %s", obj_typep);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_WARNING, msg);
		return(PIN_BOOLEAN_FALSE);
	}

	/*  Walk array of tags, comparing to our input */

	while ((a_flistp = PIN_FLIST_ELEM_GET_NEXT(c_flistp,
		PIN_FLD_PROD_PROVISIONING_TAGS, &elem_id, 1,
		&cookie, &ebuf)) != (pin_flist_t *)NULL) {

		ctag_name = (char *) PIN_FLIST_FLD_GET(a_flistp,
			PIN_FLD_PROVISIONING_TAG, 0, &ebuf);
		if (ctag_name == NULL) {
			PIN_FLIST_DESTROY(c_flistp, NULL);
			pin_set_err(&ebuf, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_NOT_FOUND, PIN_FLD_PROVISIONING_TAG, 
					0, 0);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"valid_tag_tcf : NULL provisioning_tag in "
				" config obj : ", &ebuf);
			return(PIN_BOOLEAN_FALSE);
		}
		if (strcmp(ctag_name, tag) == 0) {
			valid = PIN_BOOLEAN_TRUE;
			break;
		}
	} /* while */

	/* clean up and return  */
	PIN_FLIST_DESTROY(c_flistp, NULL);
	return(valid);
} /* valid_tag_tcf */

/**
 * Retrieve service/tags pairs from /config/provisioning_tag instances.
 * @param ctxp : context pointer
 * @param in_pdp : input poid pointer
 * @param r_flistpp : the resulting flist is returned in this paramater
 * @param ebufp : error buffer
 * @return nothing.
 */
static void
fm_subscription_pol_get_conf_provisioning_tags(
	pcm_context_t	*ctxp,
	poid_t		*in_pdp,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*s_flistp = NULL;
	pin_flist_t	*tmp_flistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	poid_t		*s_pdp = NULL;
	poid_t		*arg_pdp = NULL;

	void		*vp = NULL;

	int32		sflags = SRCH_DISTINCT;
	char		s_template[1024];

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Create the search flist.
	 ***********************************************************/
	s_flistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = PIN_POID_CREATE(pin_poid_get_db(in_pdp), "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&sflags, ebufp);

	/***********************************************************
	 * Start with the search template.
	 ***********************************************************/
	memset(s_template, 0, sizeof(s_template));
	sprintf(s_template, "%s", (char *)"select X from " \
		"/config/provisioning_tag where F1 like V1 ");
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE,
		(void *)s_template, ebufp);

	/***********************************************************
	 * 1st arg. All /config/provisioning_tag instances.
	 ***********************************************************/
	tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	arg_pdp = PIN_POID_CREATE(pin_poid_get_db(in_pdp),
		"/config/provisioning_tag", -1, ebufp);
	PIN_FLIST_FLD_PUT(tmp_flistp, PIN_FLD_POID, (void *)arg_pdp, ebufp);

	/***********************************************************
	 * Define fields we want to get from the search
	 ***********************************************************/
	tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	vp = (void *)NULL;
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_NAME, vp, ebufp);
	PIN_FLIST_ELEM_SET(tmp_flistp, (pin_flist_t *)NULL,
		PIN_FLD_PERMITTEDS, PIN_ELEMID_ANY, ebufp);

	/***********************************************************
	 * Perform the search.
	 ***********************************************************/
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_flistp, ebufp);

	*r_flistpp = r_flistp;

	/***********************************************************
	 * Cleanup.
	 ***********************************************************/
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);

	/***********************************************************
	 * Errors? Log.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"fm_subscription_pol_get_conf_provisioning_tags error", ebufp);
	}

	return;
}
