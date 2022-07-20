/*******************************************************************
 *
 *      Copyright (c) 2001-2006 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char Sccs_Id[] = "@(#)%Portal Version: fm_cust_pol_prep_inherited.c:BillingVelocityInt:4:2006-Sep-05 04:31:17 %";
#endif

#include <stdio.h>
#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "fm_utils.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pinlog.h"
#include "pin_teracomm.h"
#include "pin_msexchange.h"


EXPORT_OP void
op_cust_pol_prep_inherited(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp);

static void
fm_cust_pol_prep_inherited(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistpp,
        pin_errbuf_t		*ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_PREP_INHERITED  command
 *******************************************************************/
void
op_cust_pol_prep_inherited(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
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
	if (opcode != PCM_OP_CUST_POL_PREP_INHERITED) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_cust_pol_prep_inherited", ebufp);
		return;
	}

	/******************************************************************
	 * Debug: What we got.
	 ******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_cust_pol_prep_inherited input flist", in_flistp);

	/***********************************************************
	 * We will not open any transactions with Policy FM
	 * since policies should NEVER modify the database.
	 ***********************************************************/

	/***********************************************************
	 * Call main function to do it
	 ***********************************************************/
	fm_cust_pol_prep_inherited(ctxp, in_flistp, &r_flistp, ebufp);

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		*ret_flistpp = (pin_flist_t *)NULL;
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_cust_pol_prep_inherited error", ebufp);
	} else {
		*ret_flistpp = r_flistp;
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_prep_inherited return flist", r_flistp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_prep_inherited()
 *
 *	Prep the inherited info to be ready for on-line registration.
 *
 *	XXX STUBBED ONLY XXX
 *
 *	We do fill in an empty service_ip substruct if one is needed. 
 *
 *******************************************************************/
static void
fm_cust_pol_prep_inherited(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistpp,
        pin_errbuf_t		*ebufp)
{
	pin_flist_t		*flistp = NULL;
	pin_flist_t		*sub_flistp = NULL;
	poid_t			*s_pdp = NULL;
	int32			intValue = 0;
	int32			*intp = NULL;
	char			*strp = NULL;
	const char		*s_type = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Create outgoing flist
	 ***********************************************************/
	*out_flistpp = PIN_FLIST_COPY(in_flistp, ebufp);

	/***********************************************************
	 * Dummy up the inherited info if none provided 
	 ***********************************************************/
	s_pdp = (poid_t *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);

	/**********************************************************
	 * Get the type information for quick comaparisons
	 **********************************************************/
	s_type = PIN_POID_GET_TYPE(s_pdp);

	/***********************************************************
	 * Add any mandatory "inherited_info" to the input flist. This
	 * effectively gets around the problem of missing substructs
	 * on service creations.
	 ***********************************************************/
        fm_utils_cust_add_missing_service_substructs(ctxp, *out_flistpp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_cust_pol_prep_inherited ouput flist after adding missing substructs...", *out_flistpp);

	/***********************************************************
	 * Next, for any services that require special initialization,
	 * do that right here.
	 ***********************************************************/
	if ( s_type ) {
		
		if (strstr(s_type, "/service/gsm")) {
			flistp = PIN_FLIST_SUBSTR_GET(*out_flistpp,
				PIN_FLD_INHERITED_INFO, 0, ebufp);
			flistp = PIN_FLIST_SUBSTR_GET(flistp,
				PIN_FLD_GSM_INFO, 0, ebufp);

			PIN_FLIST_FLD_SET(flistp, PIN_FLD_BEARER_SERVICE, (void *)" ",
						ebufp);
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_PRIMARY_MSISDN,
						(void *)&intValue, ebufp);
                }
	
		if ((strcmp(s_type, "/service/telco") == 0) ||
			(strncmp(s_type, "/service/telco/", 15) == 0)) {
			flistp = PIN_FLIST_SUBSTR_GET(*out_flistpp,
				PIN_FLD_INHERITED_INFO, 0, ebufp);
			flistp = PIN_FLIST_SUBSTR_GET(flistp,
				PIN_FLD_TELCO_INFO, 0, ebufp);

			intValue = 1;
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_PRIMARY_NUMBER,
				(void *)&intValue, ebufp);
        }

		if (strstr(s_type, "/service/telco/gsm") == s_type) {
			flistp = PIN_FLIST_SUBSTR_GET(*out_flistpp,
				PIN_FLD_INHERITED_INFO, 0, ebufp);
			flistp = PIN_FLIST_SUBSTR_GET(flistp,
				PIN_FLD_TELCO_INFO, 0, ebufp);

			/*
			 * By default Primary Number flag is 0 for GSM 
			 * services, so override the value set in above
			 * generic /service/telco block.
			 */
			intValue = 0;
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_PRIMARY_NUMBER,
				(void *)&intValue, ebufp);

			flistp = PIN_FLIST_SUBSTR_GET(*out_flistpp,
				PIN_FLD_INHERITED_INFO, 0, ebufp);
			flistp = PIN_FLIST_SUBSTR_GET(flistp,
				PIN_FLD_GSM_INFO, 0, ebufp);

			PIN_FLIST_FLD_SET(flistp, PIN_FLD_BEARER_SERVICE,
					(void *)"Bearer Service", ebufp);
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_IMEI,
					(void *)"IMEI", ebufp);
		}

		if ( !strcmp("/service/telephony", s_type ) ) { 

			flistp = PIN_FLIST_SUBSTR_GET(*out_flistpp,
				PIN_FLD_INHERITED_INFO, 0, ebufp);
			flistp = PIN_FLIST_SUBSTR_GET(flistp,
				PIN_FLD_SERVICE_TELEPHONY, 0, ebufp);

			intp = (int32 *)PIN_FLIST_FLD_GET(flistp, 
				PIN_FLD_MAX_ACTIVE_CALLS, 1, ebufp);
			if(intp == (int32 *)NULL) {
				/* Set "non-zero, non-blank" default values */
				intValue = 1;  /* Default Max Active Calls */
				PIN_FLIST_FLD_SET(flistp, PIN_FLD_MAX_ACTIVE_CALLS, 
						(void*)&intValue, ebufp);

			}

		}
	
		if ( !strcmp( "/service/ip/cable/teracomm", s_type ) ) {
			int32		zero = 0;
			int32		init_led_status;
	
			flistp = PIN_FLIST_SUBSTR_GET(*out_flistpp,
				PIN_FLD_INHERITED_INFO, 0, ebufp);
			sub_flistp = PIN_FLIST_SUBSTR_GET(flistp, 
				PIN_FLD_SERVICE_IP_CABLE, 0, ebufp);
			/*
		 	 * Set PIN_FLD_MODEM_MAC_ADDR and the bandwidth limit
		 	 * fields to known initial values, so that dm_teracomm
		 	 * can determine whether provisioning fields are set.
		 	 * The initial value for PIN_FLD_MODEM_MAC_ADDR is not
		 	 * a valid address (valid addresses are 12 hex. chars.).
		 	 */
			strp = (char *)PIN_FLIST_FLD_GET(sub_flistp, 
				PIN_FLD_MODEM_MAC_ADDR, 1, ebufp);
			if(strp == (char *)NULL) {
				PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_MODEM_MAC_ADDR,
					(void *)PIN_TERACOMM_UNSPECIFIED_MODEM_MAC_ADDR,
					ebufp);
			}
			PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_MIN_DOWNSTREAM_BANDWD,
					(void *)&zero, ebufp);
			PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_MAX_DOWNSTREAM_BANDWD,
					(void *)&zero, ebufp);
			PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_MIN_UPSTREAM_BANDWD,
					(void *)&zero, ebufp);
			PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_MAX_UPSTREAM_BANDWD,
					(void *)&zero, ebufp);
	
			/* 
		 	 * Initialize fields in /service/ip/cable/teracomm substruct.
		 	 */
			sub_flistp = PIN_FLIST_SUBSTR_GET(flistp, 
					PIN_FLD_SERVICE_IP_CABLE_TERACOMM, 0, ebufp);
			PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_IP_LIMIT,
					(void *)&zero, ebufp);
			init_led_status = PIN_TERACOMM_LED_STATUS_OFF;
			PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_LED_STATUS,
					(void *)&init_led_status, ebufp);
			strp = (char *)PIN_FLIST_FLD_GET(sub_flistp, 
					PIN_FLD_TERALINK_NAME, 1, ebufp);
			if(strp == (char *)NULL) {
				PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_TERALINK_NAME,
					(void *)"", ebufp);
			}
			strp = (char *)PIN_FLIST_FLD_GET(sub_flistp, 
					PIN_FLD_ASSOC_NETWORK, 1, ebufp);
			if(strp == (char *)NULL) {
				PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_ASSOC_NETWORK,
					(void *)"", ebufp);
			}
		}
	
		if ( !strcmp(PIN_MSEXCHANGE_SERVICE_TYPE_USER, s_type ) ) {
			char 		*int_str = "";
			flistp = PIN_FLIST_SUBSTR_GET(*out_flistpp,
				PIN_FLD_INHERITED_INFO, 0, ebufp);
			sub_flistp = PIN_FLIST_SUBSTR_GET(flistp, 
				PIN_FLD_SERVICE_MSEXCHANGE_USER, 0, ebufp);
			PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_ORG_UNIT_NAME,
					(void *)int_str, ebufp);
		}
		if ( !strcmp( PIN_MSEXCHANGE_SERVICE_TYPE_FIRSTADMIN, s_type ) ){
			int32		int_value = PIN_MSEXCHANGE_INIT_STATE_INT_VALUE;
			char 		*int_str = "";
			flistp = PIN_FLIST_SUBSTR_GET(*out_flistpp,
				PIN_FLD_INHERITED_INFO, 0, ebufp);
			sub_flistp = PIN_FLIST_SUBSTR_GET(flistp, 
				PIN_FLD_SERVICE_MSEXCHANGE_USER, 0, ebufp);
			PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_ORG_UNIT_NAME,
					(void *)int_str, ebufp);
			flistp = PIN_FLIST_SUBSTR_GET(*out_flistpp,
				PIN_FLD_INHERITED_INFO, 0, ebufp);
			sub_flistp = PIN_FLIST_SUBSTR_GET(flistp, 
				PIN_FLD_SERVICE_MSEXCHANGE_ORG, 0, ebufp);
			PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_ORG_DISPLAY_NAME,
					(void *)int_str, ebufp);
			PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_SMTP_DOMAIN_NAME,
					(void *)int_str, ebufp);
			PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_MAX_USER_COUNT,
					(void *)&int_value, ebufp);
			PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_DEFAULT_MAX_MBOX_SIZE,
					(void *)&int_value, ebufp);
		}
	}

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_prep_inherited error", ebufp);
	}

	return;
}


