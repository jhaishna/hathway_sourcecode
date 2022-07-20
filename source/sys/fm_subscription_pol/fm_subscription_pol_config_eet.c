/*******************************************************************
 *
* Copyright (c) 2004, 2017, Oracle and/or its affiliates. All rights reserved.
 *      
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/


#ifndef lint
static  char      Sccs_id[] = "@(#)$Id: fm_subscription_pol_config_eet.c /cgbubrm_mainbrm.portalbase/1 2017/11/24 02:01:45 anirbisw Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "pcm.h"
#include "ops/bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "pin_os.h"
#include "pin_os_string.h"

#ifndef PCM_OP_SUBSCRIPTION_POL_CONFIG_EET
#define PCM_OP_SUBSCRIPTION_POL_CONFIG_EET 426
#endif
#define FILE_SOURCE_ID  "fm_subscription_pol_config_eet.c(9)"

#define CHUNKSIZE		1024
#define DELIMITER		"\t"
#define EOL_DELIMITER		"\n"

#define FILE_FLD_FORMAT_9                       (char *)"9"
#define FILE_FLD_FORMAT_X                       (char *)"X"
#define FILE_FLD_FORMAT_Z                       (char *)"Z"
#define FILE_FLD_FORMAT_H                       (char *)"H"
#define FILE_FLD_NOT_USED                       (char *)"NOT_USED"


#define FIELD_DURATION_UOM                      (char *)"SEC"
#define FIELD_VOLUME_SENT_UOM                   (char *)"BYT"
#define FIELD_VOLUME_RECEIVED_UOM               (char *)"BYT"
#define FIELD_NUMBER_OF_UNITS_UOM               (char *)"CLK"

#define RECORD_TYPE_GSM                         (char *)"020"
#define RECORD_TYPE_GPRS                        (char *)"020"
#define RECORD_TYPE_WAP                         (char *)"070"
#define RECORD_TYPE_IMT                         (char *)"020"
#define RECORD_TYPE_PDC                         (char *)"020"
#define RECORD_TYPE_HEADER                      (char *)"010"
#define RECORD_TYPE_TRAILER                     (char *)"090"
#define RECORD_TYPE_GSM_WIRELINE_EXT            (char *)"520"
#define RECORD_TYPE_GPRS_EXT                    (char *)"540"
#define RECORD_TYPE_WAP_EXT                     (char *)"570"
#define RECORD_TYPE_IMT_EXT                     (char *)"520"
#define RECORD_TYPE_PDC_EXT                     (char *)"520"
#define RECORD_TYPE_ASSOC_INFRANET              (char *)"900"
#define RECORD_TYPE_SUP_BAL                     (char *)"600"
#define RECORD_TYPE_SUP_SVC                     (char *)"620"
#define RECORD_TYPE_ASSOC_CHARGE                (char *)"980"
#define RECORD_TYPE_DISCOUNT                    (char *)"690"
#define RECORD_TYPE_SUBBALANCE                  (char *)"609"
#define RECORD_TYPE_SUB_BAL_IMPACTS             (char *)"605"
#define RECORD_TYPE_SUBBAL                      (char *)"607"
#define RECORD_TYPE_SUP_MONITOR_BAL             (char *)"800"
#define RECORD_TYPE_MONITOR_SUB_BAL_IMPACTS     (char *)"805"
#define RECORD_TYPE_MONITOR_SUBBAL              (char *)"807"
#define RERATE_IMPACT_TYPE                      288
#define RECORD_PLACE_HOLDER                     (char *)"****#****"

EXPORT_OP void
op_subscription_pol_config_eet(
	cm_nap_connection_t	*connp,
	u_int			opcode,
	u_int			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_subscription_pol_config_eet(
	pcm_context_t		*ctxp,
	u_int			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistpp,
        pin_errbuf_t		*ebufp);
 
static void
dropEvtDetailBalImpacts(
	pin_flist_t	*evt_flistp,
	pin_errbuf_t	*ebufp);

static void
dropEvtDetailMonitorImpacts(
	pin_flist_t *evt_flistp,
	pin_errbuf_t    *ebufp);

static void 
dropEvtDetailSubBalImpacts( 
	pin_flist_t	*evt_flistp, 
	pin_errbuf_t	*ebufp);

static void 
dropEvtDetailMonitorSubBalImpacts( 
	pin_flist_t	*evt_flistp, 
	pin_errbuf_t	*ebufp);

static void
getReRateObj(
	pcm_context_t           *ctxp,
	pin_flist_t **flistpp,
        poid_t      *rerate_pdp,
        pin_flist_t *prev_bal_flistp,
	pin_errbuf_t	*ebufp);

static void
analyzeBalImpacts(pin_flist_t	**flistpp,
        pin_flist_t *prev_impact_flistp,
	pin_errbuf_t	*ebufp);

static void
analyzeMonitorImpacts(pin_flist_t   **flistpp,
	pin_flist_t *prev_impact_flistp,
	pin_errbuf_t    *ebufp);

static void 
analyzeSubBalImpacts(pin_flist_t **flistpp, 
        pin_flist_t *prev_impact_flistp, 
        pin_fld_num_t fld_type, 
	pin_errbuf_t	*ebufp);

static void
addBalImpactsPoid(
	pin_flist_t *curr_impact_flistp,
	pin_flist_t *evt_flistp,
	pin_errbuf_t	*ebufp);

static void
addMonitorImpactsPoid(
	pin_flist_t *curr_impact_flistp,
	pin_flist_t *evt_flistp,
	pin_errbuf_t    *ebufp);

static void 
addSubBalImpactsPoid( 
	pin_flist_t *curr_impact_flistp, 
	pin_flist_t *evt_flistp, 
	pin_errbuf_t	*ebufp); 

static void 
addMonitorSubBalImpactsPoid( 
	pin_flist_t *curr_impact_flistp, 
	pin_flist_t *evt_flistp, 
	pin_errbuf_t	*ebufp); 

static void
addStr(
        pin_buf_t  		*bufp,
        char                   *string,
        pin_errbuf_t           *ebufp );

static void 
formatFieldsForFile(
	pin_buf_t *edr,
	char *format, 
        char *data, 
        long length,
	pin_errbuf_t	*ebufp);

static void
processSessionBasicDetailImt(
        pin_buf_t       *edr,
        pin_flist_t *imt_flistp,
        int32 number_assoc_recs,
        const char *record_type,
        pin_errbuf_t    *ebufp);

static void
processSessionBasicDetailPdc(
        pin_buf_t       *edr,
        pin_flist_t *pdc_flistp,
        int32 number_assoc_recs,
        const char *record_type,
        pin_errbuf_t    *ebufp);

static void
processSessionBasicDetailGsm(
	pin_buf_t	*edr,
	pin_flist_t *gsm_flistp,
        int32 number_assoc_recs,
        const char *record_type,
	pin_errbuf_t	*ebufp);

static void
processSessionBasicDetailGprs(
	pin_buf_t	*edr,
	pin_flist_t *gprs_flistp,
        int32   number_assoc_recs,
        const char *record_type,
	pin_errbuf_t	*ebufp);

static void
processSessionBasicDetailTelcoGprs(
	pin_buf_t	*edr,
	pin_flist_t *gprs_flistp,
        int32   number_assoc_recs,
        const char *record_type,
	pin_errbuf_t	*ebufp);

static void
processSessionBasicDetailWap(
	pin_buf_t   *edr,
	pin_flist_t   *wap_flistp,
        int32   number_assoc_recs,
        const char      *record_type,
	pin_errbuf_t	*ebufp);

static void
processSessionSupBalance(
	pin_buf_t	*edr,
	pin_flist_t	*flistp,
	pin_errbuf_t	*ebufp);

static void
processSessionSupService(
	pin_buf_t	*edr,
	pin_flist_t	*flistp,
	pin_errbuf_t	*ebufp);

static void
processSessionSupMonitorBalance(
	pin_buf_t	*edr,
	pin_flist_t	*flistp,
	pin_errbuf_t	*ebufp);

static void
processSessionAssocInfranet(
	pin_buf_t	*edr,
	pin_flist_t	*flistp,
	pin_errbuf_t	*ebufp);

static void
processSessionAssocCharge(
	pin_buf_t	*edr,
	pin_flist_t	*flistp,
	pin_errbuf_t	*ebufp);


static void
processSessionSubBalancePackets(
	pin_buf_t	*edr,
	pin_flist_t	*flistp,
	char		*sub_balimp_type,
	char		*sub_balances_type,
	pin_errbuf_t	*ebufp);

static poid_t *
findAccountPoidForSubBalance(
        pin_flist_t *bi_flp,
        pin_flist_t *sbi_flp,
        pin_errbuf_t *ebufp);

static void
processSessionSubBalanceImpacts(
	pin_buf_t	*edr,
	pin_flist_t	*flistp,
	pin_errbuf_t	*ebufp);

static void
processSessionSubBalances(
	pin_buf_t	*edr,
	pin_flist_t	*flistp,
	char		*sub_balances_type,
	pin_errbuf_t	*ebufp);

static void
processSessionMonitorSubBalanceImpacts(
	pin_buf_t	*edr,
	pin_flist_t	*flistp,
	pin_errbuf_t	*ebufp);

static void
processSessionAssocImt(
        pin_buf_t       *edr,
        pin_flist_t *imt_flistp,
        pin_errbuf_t    *ebufp);

static void
processSessionAssocPdc(
        pin_buf_t       *edr,
        pin_flist_t *pdc_flistp,
        pin_errbuf_t    *ebufp);

static void
processSessionAssocGsm(
	pin_buf_t	*edr,
	pin_flist_t *gsm_flistp,
	pin_errbuf_t	*ebufp);

static void
processSessionAssocGprs(
	pin_buf_t	*edr,
	pin_flist_t	*gprs_flistp,
	pin_errbuf_t	*ebufp);

static void
processSessionAssocTelcoGprs(
	pin_buf_t	*edr,
	pin_flist_t	*gprs_flistp,
	pin_errbuf_t	*ebufp);

static void
processSessionAssocWap(
	pin_buf_t	*edr,
	pin_flist_t	*wap_flistp,
	pin_errbuf_t	*ebufp);
/*******************************************************************
 * Main routine for the PCM_OP_SUBSCRIPTION_POL_CONFIG_EET  command
 *******************************************************************/
void
op_subscription_pol_config_eet(
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
	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_SUBSCRIPTION_POL_CONFIG_EET) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_subscription_pol_config_eet", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What did we get?
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_subscription_pol_config_eet input flist", in_flistp);

	/***********************************************************
	 * Call main function to do it
	 ***********************************************************/
	fm_subscription_pol_config_eet(ctxp, flags, in_flistp, &r_flistp, ebufp);

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		*ret_flistpp = (pin_flist_t *)NULL;
		PIN_FLIST_DESTROY(r_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_subscription_pol_config_eet error", ebufp);
	} else {
		*ret_flistpp = r_flistp;
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_subscription_pol_config_eet return flist", r_flistp);
	}

	return;
}

/*******************************************************************
 * fm_subscription_pol_config_eet()
 *
 *	Read the event object and prepare the record for event extraction tool.
 *
 *******************************************************************/
static void
fm_subscription_pol_config_eet(
	pcm_context_t		*ctxp,
	u_int			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistpp,
        pin_errbuf_t		*ebufp)
{
	pin_flist_t	*flistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	pin_flist_t	*evt_flistp = NULL;
	poid_t		*rerate_pdp = NULL;
	poid_t		*event_pdp = NULL;
	void            *vp = NULL;
	char		*event_category = NULL;
	pin_buf_t	*pin_bufp = NULL;

        if (PIN_ERR_IS_ERR(ebufp)) {
                return;
                /*****/
        }

	/*****************************************************************
	 * Set up pin_bufp structure
	 *****************************************************************/
	pin_bufp = (pin_buf_t *) pin_malloc(sizeof(pin_buf_t));
	if (pin_bufp == NULL) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_NO_MEM, 0, 0, 0);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"Memory for pin_bufp couldn't be allocated", ebufp);
			goto ErrOut;
	}

	pin_bufp->flag = 0x0;
	pin_bufp->offset = 0;
	pin_bufp->xbuf_file = NULL;
	pin_bufp->size = 0;
	pin_bufp->data = NULL;

	*out_flistpp = PIN_FLIST_CREATE(ebufp);
	/***********************************************************
	 * Read the event for process
	 ***********************************************************/
	event_pdp = (poid_t *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 
			0, ebufp);
        if(!event_pdp) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_POID, 0, 0);
		goto ErrOut;
	}
	PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_POID, (void *)event_pdp, 
			ebufp);
	event_category = (char *)PIN_FLIST_FLD_GET(in_flistp, 
			PIN_FLD_CATEGORY, 0, ebufp);
        if(!event_category) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_CATEGORY, 0, 0);
                goto ErrOut;
        }


	flistp = PIN_FLIST_CREATE(ebufp);

	PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, (void *)event_pdp, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_RERATE_OBJ, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_DESCR, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_END_T, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_START_T, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_CREATED_T, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_TIMEZONE_ID, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_SERVICE_OBJ, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACCOUNT_OBJ, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_ITEM_OBJ, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_TAX_LOCALES, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_TAX_SUPPLIER, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_INVOICE_DATA, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_PROVIDER_ID, (void *)NULL, ebufp);

	/**
	 * RA Changes - Start
	**/

	PIN_FLIST_FLD_SET(flistp, PIN_FLD_EVENT_NO, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_BATCH_ID, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_ORIGINAL_BATCH_ID, (void *)NULL, ebufp);

	/**
	 * RA Changes - End
	**/

	PIN_FLIST_ELEM_SET(flistp, NULL, PIN_FLD_BAL_IMPACTS, 
			PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_ELEM_SET(flistp, NULL, PIN_FLD_SUB_BAL_IMPACTS, 
			PIN_ELEMID_ANY, ebufp);

    /**
     * Add for Monitored Impacts
    **/
	PIN_FLIST_ELEM_SET(flistp, NULL, PIN_FLD_MONITOR_IMPACTS,
			PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_ELEM_SET(flistp, NULL, PIN_FLD_MONITOR_SUB_BAL_IMPACTS,
			PIN_ELEMID_ANY, ebufp);
		  
        if (!strcasecmp(event_category, "imt")) {

                PIN_FLIST_SUBSTR_SET(flistp, NULL, PIN_FLD_TELCO_INFO, ebufp);
                PIN_FLIST_ELEM_SET(flistp, NULL, PIN_FLD_SERVICE_CODES,
                        PIN_ELEMID_ANY, ebufp);
        } else if (!strcasecmp(event_category, "pdc")) {

                PIN_FLIST_SUBSTR_SET(flistp, NULL, PIN_FLD_TELCO_INFO, ebufp);
                PIN_FLIST_ELEM_SET(flistp, NULL, PIN_FLD_SERVICE_CODES,
                        PIN_ELEMID_ANY, ebufp);

	} else if (!strcasecmp(event_category, "gsm")) {

		PIN_FLIST_SUBSTR_SET(flistp, NULL, PIN_FLD_GSM_INFO, ebufp);
		PIN_FLIST_SUBSTR_SET(flistp, NULL, PIN_FLD_TELCO_INFO, ebufp);
		PIN_FLIST_ELEM_SET(flistp, NULL, PIN_FLD_SERVICE_CODES, 
			PIN_ELEMID_ANY, ebufp);
	} else if (!strcasecmp(event_category, "gprs")) {
		if (!strncmp("/event/delayed/session/telco/gprs",PIN_POID_GET_TYPE(event_pdp), 33)){
			PIN_FLIST_SUBSTR_SET(flistp, NULL, PIN_FLD_TELCO_INFO, ebufp);
		}
		PIN_FLIST_SUBSTR_SET(flistp, NULL, PIN_FLD_GPRS_INFO, ebufp);
	} else if (!strcasecmp(event_category, "wap")) {

		PIN_FLIST_SUBSTR_SET(flistp, NULL, PIN_FLD_WAP_INFO, ebufp);
	} else if (!strcasecmp(event_category, "roaming")) {

		PIN_FLIST_SUBSTR_SET(flistp, NULL, PIN_FLD_TELCO_INFO, ebufp);			
		if (!strncmp("/event/delayed/session/telco/gsm/roaming",PIN_POID_GET_TYPE(event_pdp), 40)){
			PIN_FLIST_ELEM_SET(flistp, NULL, PIN_FLD_SERVICE_CODES,
				PIN_ELEMID_ANY, ebufp);
			PIN_FLIST_SUBSTR_SET(flistp, NULL, PIN_FLD_GSM_INFO, ebufp);
		}
		if (!strncmp("/event/delayed/session/telco/gprs/roaming",PIN_POID_GET_TYPE(event_pdp), 41)){
			PIN_FLIST_SUBSTR_SET(flistp, NULL, PIN_FLD_GPRS_INFO, ebufp);
		}
	}else {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_ARG, 0, 0, 0);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"Unrecognized event category name", ebufp);
		goto ErrOut;
		/******************/
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
	    (char *)"fm_subscription_pol_config_eet input flist for "
	    "single event search", flistp);

	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, flistp, &r_flistp, ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {

		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		    (char *)"*** SINGLE EVENT SELECT FAILED ***", 
		    ebufp);

		goto ErrOut;
		/******************/
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
	    (char *)"fm_subscription_pol_config_eet return flist for "
	    "single event search", r_flistp);

	rerate_pdp = (poid_t *)PIN_FLIST_FLD_GET(r_flistp,
	    PIN_FLD_RERATE_OBJ, 0, ebufp);

	if(!PIN_POID_IS_NULL(rerate_pdp)) {

		evt_flistp = PIN_FLIST_COPY(r_flistp, ebufp);
		dropEvtDetailBalImpacts(evt_flistp, ebufp);
		dropEvtDetailSubBalImpacts( evt_flistp, ebufp ) ; 
		dropEvtDetailMonitorImpacts(evt_flistp, ebufp);
		dropEvtDetailMonitorSubBalImpacts( evt_flistp, ebufp ) ; 
		getReRateObj(ctxp, &r_flistp,rerate_pdp,r_flistp, ebufp);

		/*
		 * Add bal impact details and monitor impact details back
		 * plus orig evt poid so r_flistp
		 * contains all info needed for records
		 */
		addBalImpactsPoid(r_flistp, evt_flistp, ebufp);
		addMonitorImpactsPoid(r_flistp, evt_flistp, ebufp);
                
		/*
		* Add All Calculated Sub bal and monitor sub bal 
		* impact details back to evt_flistp flist.
		*/
		addSubBalImpactsPoid(r_flistp, evt_flistp, ebufp); 
		addMonitorSubBalImpactsPoid(r_flistp, evt_flistp, ebufp); 
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		r_flistp = PIN_FLIST_COPY(evt_flistp, ebufp);
	}

	/*
	 * Process the event
	 */
	if (!strcasecmp(event_category, "imt")) {
		processSessionBasicDetailImt(pin_bufp, r_flistp, 2,
			RECORD_TYPE_IMT, ebufp);
                processSessionAssocImt(pin_bufp, r_flistp, ebufp);
                processSessionAssocInfranet(pin_bufp, r_flistp, ebufp);
                processSessionAssocCharge(pin_bufp, r_flistp, ebufp);
        } else if (!strcasecmp(event_category, "pdc")) {

                processSessionBasicDetailPdc(pin_bufp, r_flistp, 2,
                    RECORD_TYPE_PDC, ebufp);
                processSessionAssocPdc(pin_bufp, r_flistp, ebufp);
                processSessionAssocInfranet(pin_bufp, r_flistp, ebufp);
                processSessionAssocCharge(pin_bufp, r_flistp, ebufp);
	} else if (!strcasecmp(event_category, "gsm")) {

		processSessionBasicDetailGsm(pin_bufp, r_flistp, 2, 
		    RECORD_TYPE_GSM, ebufp);
		processSessionAssocGsm(pin_bufp, r_flistp, ebufp);
		processSessionAssocInfranet(pin_bufp, r_flistp, ebufp);
		processSessionAssocCharge(pin_bufp, r_flistp, ebufp);
	} else if (!strcasecmp(event_category, "gprs")) {

		if (!strncmp("/event/delayed/session/telco/gprs",PIN_POID_GET_TYPE(event_pdp), 33)){
			processSessionBasicDetailTelcoGprs(pin_bufp, r_flistp, 2, 
				RECORD_TYPE_GPRS, ebufp);
			processSessionAssocTelcoGprs(pin_bufp, r_flistp, ebufp);
		}else {
			processSessionBasicDetailGprs(pin_bufp, r_flistp, 2, 
				RECORD_TYPE_GPRS, ebufp);
			processSessionAssocGprs(pin_bufp, r_flistp, ebufp);
		}
		processSessionAssocInfranet(pin_bufp, r_flistp, ebufp);
		processSessionAssocCharge(pin_bufp, r_flistp, ebufp);
	} else if (!strcasecmp(event_category, "wap")) {

		processSessionBasicDetailWap(pin_bufp, r_flistp, 2, 
		    RECORD_TYPE_WAP, ebufp);
		processSessionAssocWap(pin_bufp, r_flistp, ebufp);
		processSessionAssocInfranet(pin_bufp, r_flistp, ebufp);
		processSessionAssocCharge(pin_bufp, r_flistp, ebufp);
	}else if (!strcasecmp(event_category, "roaming")) {
		if (!strncmp("/event/delayed/session/telco/gsm/roaming",PIN_POID_GET_TYPE(event_pdp), 40)){
			processSessionBasicDetailGsm(pin_bufp, r_flistp, 2, 
				RECORD_TYPE_GSM, ebufp);
			processSessionAssocGsm(pin_bufp, r_flistp, ebufp);
		}
		if (!strncmp("/event/delayed/session/telco/gprs/roaming",PIN_POID_GET_TYPE(event_pdp), 41)){
			processSessionBasicDetailTelcoGprs(pin_bufp, r_flistp, 2, 
				RECORD_TYPE_GPRS, ebufp);
			processSessionAssocTelcoGprs(pin_bufp, r_flistp, ebufp);
		}
		/*  following two are common for both */ 
		processSessionAssocInfranet(pin_bufp, r_flistp, ebufp);
		processSessionAssocCharge(pin_bufp, r_flistp, ebufp);
	}else {
		/* Unknown event category */
	}


	
ErrOut:
	if (pin_bufp) {
		if (pin_bufp->data != NULL) {
			pin_bufp->size = pin_bufp->offset+1;
			pin_bufp->offset = 0;

			/* Resize the buffer to release unused spaces */
			pin_bufp->data = (char *)pin_realloc( 
				pin_bufp->data, pin_bufp->size);

			PIN_FLIST_FLD_PUT(*out_flistpp, PIN_FLD_BUFFER, 
				(void *)pin_bufp, ebufp);
		} else {
			pin_free(pin_bufp);
		}
	}

	if (r_flistp) PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	if (flistp) PIN_FLIST_DESTROY_EX(&flistp, NULL);
	if (evt_flistp) PIN_FLIST_DESTROY_EX(&evt_flistp, NULL);

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_subscription_pol_config_eet error", ebufp);
	}

	return;
}


static void
dropEvtDetailBalImpacts(
	pin_flist_t	*evt_flistp,
	pin_errbuf_t	*ebufp)
{
        int32           record_id = 0;
        pin_cookie_t	cookie_id = NULL;

        if (PIN_ERR_IS_ERR(ebufp)) {
                return;
                /*****/
        }

        while ((PIN_FLIST_ELEM_GET_NEXT(
            evt_flistp, PIN_FLD_BAL_IMPACTS, &record_id, 1, 
            &cookie_id, ebufp)) != (pin_flist_t *)NULL) {

                PIN_FLIST_ELEM_DROP(evt_flistp,
                    PIN_FLD_BAL_IMPACTS, record_id, ebufp);
        	cookie_id = NULL;
        }
}

static void
dropEvtDetailMonitorImpacts(
    pin_flist_t *evt_flistp,
    pin_errbuf_t    *ebufp)
{
        int32           record_id = 0;
        pin_cookie_t    cookie_id = NULL;

        if (PIN_ERR_IS_ERR(ebufp)) {
                return;
                /*****/
        }

        while ((PIN_FLIST_ELEM_GET_NEXT(
            evt_flistp, PIN_FLD_MONITOR_IMPACTS, &record_id, 1,
            &cookie_id, ebufp)) != (pin_flist_t *)NULL) {

                PIN_FLIST_ELEM_DROP(evt_flistp,
                    PIN_FLD_MONITOR_IMPACTS, record_id, ebufp);
            cookie_id = NULL;
        }
}

static void 
dropEvtDetailSubBalImpacts( 
        pin_flist_t     *evt_flistp, 
        pin_errbuf_t    *ebufp)
{ 
        int32           record_id = 0; 
        pin_cookie_t    cookie_id = NULL; 
 
        if (PIN_ERR_IS_ERR(ebufp)) {
                return;
                /*****/
        }

        while ((PIN_FLIST_ELEM_GET_NEXT( 
            evt_flistp, PIN_FLD_SUB_BAL_IMPACTS, &record_id, 1, 
            &cookie_id, ebufp)) != (pin_flist_t *)NULL) { 
 
                PIN_FLIST_ELEM_DROP(evt_flistp, 
                    PIN_FLD_SUB_BAL_IMPACTS, record_id, ebufp); 
                cookie_id = NULL; 
        } 
}

static void
dropEvtDetailMonitorSubBalImpacts(
        pin_flist_t     *evt_flistp,
        pin_errbuf_t    *ebufp)
{
        int32           record_id = 0;
        pin_cookie_t    cookie_id = NULL;

        if (PIN_ERR_IS_ERR(ebufp)) {
                return;
                /*****/
        }

        while ((PIN_FLIST_ELEM_GET_NEXT(
            evt_flistp, PIN_FLD_MONITOR_SUB_BAL_IMPACTS, &record_id, 1,
            &cookie_id, ebufp)) != (pin_flist_t *)NULL) {

                PIN_FLIST_ELEM_DROP(evt_flistp,
                    PIN_FLD_MONITOR_SUB_BAL_IMPACTS, record_id, ebufp);
                cookie_id = NULL;
        }
}
 
/****************************************************************************
 * Function     : getReRateObj
 * Description  : Return the flist containing lastest bal impacts
 *--------------+------------------------------------------------------------
 * In/Out       : pin_flist_t	**flistpp
 * In           : poid_t      *rerate_pdp
 * In           : pin_flist_t *prev_bal_flistp
 *--------------+------------------------------------------------------------
 * Returns      : void
 ****************************************************************************/
static void
getReRateObj(
	pcm_context_t		*ctxp,
	pin_flist_t **flistpp,
        poid_t      *rerate_pdp,
        pin_flist_t *prev_bal_flistp,
	pin_errbuf_t	*ebufp)
{
        pin_flist_t	*flistp = NULL;
        pin_flist_t	*r_flistp = NULL;
        pin_flist_t	*tmp_flistp = NULL;
                	
        poid_t          *next_rerate_pdp = NULL;
        int32		element_id = 0;
        pin_cookie_t	cookie = NULL;

        if (PIN_ERR_IS_ERR(ebufp)) {
		return;
                /*****/
	}

        
	/* 
	 * Allocate a search flist. 
	 */
	flistp = PIN_FLIST_CREATE(ebufp);

	PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, (poid_t *)rerate_pdp, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_RERATE_OBJ, (void *)NULL, ebufp);
	PIN_FLIST_ELEM_SET(flistp, NULL, PIN_FLD_BAL_IMPACTS, 
			PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_ELEM_SET(flistp, NULL, PIN_FLD_SUB_BAL_IMPACTS,  
			PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_ELEM_SET(flistp, NULL, PIN_FLD_MONITOR_IMPACTS, 
			PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_ELEM_SET(flistp, NULL, PIN_FLD_MONITOR_SUB_BAL_IMPACTS,  
			PIN_ELEMID_ANY, ebufp);
       
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
	    (char *)"Input to re-rate search.", flistp);

	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, flistp, &r_flistp, ebufp);
       
	if (PIN_ERR_IS_ERR(ebufp)) {

		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		    (char *)"*** RERATE SELECT FAILED ***", ebufp);

		PIN_FLIST_DESTROY_EX(&flistp, NULL);
		return;
		/*****/
	}

        /*
         * Calculate and Update current flist i.e r_flistp with
         * previous Sub balance Impact and monitor sub impact info.
         */
        analyzeSubBalImpacts(&r_flistp, prev_bal_flistp, PIN_FLD_SUB_BAL_IMPACTS, ebufp ) ;
        analyzeSubBalImpacts(&r_flistp, prev_bal_flistp, PIN_FLD_MONITOR_SUB_BAL_IMPACTS, ebufp ) ;

        /*
         * Calculate and Update current flist i.e r_flistp with
         * previous balance Impact and monitor impact info.
         */

        analyzeBalImpacts(&r_flistp,prev_bal_flistp,ebufp);
        analyzeMonitorImpacts(&r_flistp,prev_bal_flistp,ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	    (char *)"Result of re-rate search. r_flistp\n", r_flistp);

        next_rerate_pdp = (poid_t *)PIN_FLIST_FLD_GET(r_flistp,
	    PIN_FLD_RERATE_OBJ, 0, ebufp);

        /*
         * Pursue flist with latest bal impacts info
         */
        if (!PIN_POID_IS_NULL((poid_t *)next_rerate_pdp)) {

                /*
                 * Recursively call until 
                 * we get the desired flist
                 */
                getReRateObj(ctxp, flistpp, next_rerate_pdp, r_flistp, ebufp);
                PIN_FLIST_DESTROY_EX(&flistp, NULL);
                return;
                /*****/
        }     

        *flistpp = r_flistp;
        PIN_FLIST_DESTROY_EX(&flistp, NULL);
        return;
        /*****/
}


/****************************************************************************
 * Function     : analyzeBalImpacts
 * Description  : Check for bal impacts that sum to zero
 *              : 
 *--------------+------------------------------------------------------------
 * In/Out       : pin_flist_t	**flistpp
 * In           : pin_flist_t   *prev_impact_flistp
 *--------------+------------------------------------------------------------
 * Returns      : void
 ****************************************************************************/
static void
analyzeBalImpacts(pin_flist_t	**flistpp,
        pin_flist_t *prev_impact_flistp,
	pin_errbuf_t	*ebufp)
{

        pin_flist_t  *cur_bal_impact_flistp = NULL;
        pin_flist_t  *prev_bal_impact_flistp = NULL;

        int32          *cur_gl_id_p = NULL;
        int32          cur_gl_id = 0;
        pin_decimal_t  *cur_amt_p = NULL;
        double         cur_amt = 0;
        char           *cur_info_str = NULL;
        int32          *prev_gl_id_p = NULL;
        int32          prev_gl_id = 0;
        pin_decimal_t  *prev_amt_p = NULL;
        double         prev_amt = 0;
        char           *prev_info_str = NULL;
        int32           cur_resource_id = 0;
        int32           prev_resource_id = 0;
        int32          *resourcep = NULL;

        int32           record_id = 0;
        pin_cookie_t	cookie_id = NULL;
        int32           record_id2 = 0;
        pin_cookie_t	cookie_id2 = NULL;

        if (PIN_ERR_IS_ERR(ebufp)) {
                return;
                /*****/
        }

        while ((cur_bal_impact_flistp = PIN_FLIST_ELEM_GET_NEXT(*flistpp,
	    PIN_FLD_BAL_IMPACTS, &record_id, 1, &cookie_id,
            ebufp)) != (pin_flist_t *)NULL) {

                cur_gl_id_p = (int32 *)PIN_FLIST_FLD_GET(cur_bal_impact_flistp,
                    PIN_FLD_GL_ID, 0, ebufp);
                if(!cur_gl_id_p) {
                        pin_set_err(ebufp, PIN_ERRLOC_APP,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_NULL_PTR, PIN_FLD_GL_ID, 0, 0);
			goto ErrNullPointer; 
                }
                cur_gl_id = *cur_gl_id_p;

                resourcep = (int32 *)PIN_FLIST_FLD_GET(cur_bal_impact_flistp,
                    PIN_FLD_RESOURCE_ID, 0, ebufp);
                if(!resourcep) {
                        pin_set_err(ebufp, PIN_ERRLOC_APP,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_NULL_PTR, PIN_FLD_RESOURCE_ID, 0, 0);
			goto ErrNullPointer;
                }
                cur_resource_id = *resourcep;

                cur_amt_p = (pin_decimal_t *)PIN_FLIST_FLD_GET(
                    cur_bal_impact_flistp, PIN_FLD_AMOUNT, 0, ebufp);
                cur_amt = pin_decimal_to_double(cur_amt_p, ebufp);

                cur_info_str = (char *)PIN_FLIST_FLD_GET(cur_bal_impact_flistp,
                    PIN_FLD_DISCOUNT_INFO, 0, ebufp);
		if ( !cur_info_str ) {
			pin_set_err(ebufp, PIN_ERRLOC_APP,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_NULL_PTR, PIN_FLD_DISCOUNT_INFO, 0, 0);
			goto ErrNullPointer;
		}

                record_id2 = 0;
                cookie_id2 = NULL;

                /*
                 * Now walk all prev impacts looking for a match
                 */
                while ((prev_bal_impact_flistp = PIN_FLIST_ELEM_GET_NEXT(
                    prev_impact_flistp, PIN_FLD_BAL_IMPACTS, &record_id2, 1, 
                    &cookie_id2, ebufp)) != (pin_flist_t *)NULL) {

                        prev_gl_id_p = (int32 *)PIN_FLIST_FLD_GET(
                            prev_bal_impact_flistp, PIN_FLD_GL_ID, 0, ebufp);
                	if ( !prev_gl_id_p ) {
                       		pin_set_err(ebufp, PIN_ERRLOC_APP,
	                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
       	                 	PIN_ERR_NULL_PTR, PIN_FLD_GL_ID, 0, 0);
       		                goto ErrNullPointer;
                	} 
                        prev_gl_id = *prev_gl_id_p;

                	resourcep = (int32 *)PIN_FLIST_FLD_GET(
				prev_bal_impact_flistp, 
				PIN_FLD_RESOURCE_ID, 0, ebufp);
	                if ( !resourcep ) {
       		                pin_set_err(ebufp, PIN_ERRLOC_APP,
                        	PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        	PIN_ERR_NULL_PTR, PIN_FLD_RESOURCE_ID, 0, 0);
                        	goto ErrNullPointer;
                	}
       		        prev_resource_id = *resourcep;

                        prev_amt_p = (pin_decimal_t *)PIN_FLIST_FLD_GET(
                            prev_bal_impact_flistp, PIN_FLD_AMOUNT, 0, 
                            ebufp);
                        prev_amt = pin_decimal_to_double(prev_amt_p, ebufp);

                        prev_info_str = (char *)PIN_FLIST_FLD_GET(
                            prev_bal_impact_flistp, PIN_FLD_DISCOUNT_INFO, 0, 
                            ebufp);
                        if ( !prev_info_str ) {
                                pin_set_err(ebufp, PIN_ERRLOC_APP,
                                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                                PIN_ERR_NULL_PTR, PIN_FLD_DISCOUNT_INFO, 0, 0);
                                goto ErrNullPointer;
                        }

                        if( (cur_gl_id == prev_gl_id) &&
                            (cur_resource_id == prev_resource_id) &&
                            ((cur_amt + prev_amt) == 0) &&
                            (cur_info_str != NULL) &&
                            (prev_info_str != NULL) &&
                            !strcmp(cur_info_str, prev_info_str) ) {

                		cur_amt_p = pbo_decimal_from_str("0", ebufp);
                		PIN_FLIST_FLD_PUT(cur_bal_impact_flistp,
                            		PIN_FLD_AMOUNT, cur_amt_p, ebufp);

                		PIN_FLIST_ELEM_DROP(prev_impact_flistp,
                            		PIN_FLD_BAL_IMPACTS, record_id2, ebufp);
                		break;
                        }
                }
        }

ErrNullPointer:
        if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "analyzeBalImpacts error", ebufp);
                return;
                /*****/
        }
        
}

/****************************************************************************
 * Function     : analyzeMonitorImpacts
 * Description  : Drop any monitor impacts from the latest event object that
 *              : are duplicates
 *--------------+------------------------------------------------------------
 * In/Out       : pin_flist_t   **flistpp
 * In           : pin_flist_t   *prev_impact_flistp
 *--------------+------------------------------------------------------------
 * Returns      : void
 ****************************************************************************/
static void
analyzeMonitorImpacts(pin_flist_t   **flistpp,
        pin_flist_t *prev_impact_flistp,
    pin_errbuf_t    *ebufp)
{

        pin_flist_t  *cur_mon_impact_flistp = NULL;
        pin_flist_t  *prev_mon_impact_flistp = NULL;

        pin_decimal_t  *cur_amt_p = NULL;
        double         cur_amt = 0;
        pin_decimal_t  *prev_amt_p = NULL;
        double         prev_amt = 0;
        int32           cur_resource_id = 0;
        int32           prev_resource_id = 0;
        int32          *resourcep = NULL;

        int32           record_id = 0;
        pin_cookie_t    cookie_id = NULL;
        int32           record_id2 = 0;
        pin_cookie_t    cookie_id2 = NULL;
        pin_cookie_t    cookie_tmp = NULL;

        if (PIN_ERR_IS_ERR(ebufp)) {
                return;
                /*****/
        }

        while ((cur_mon_impact_flistp = PIN_FLIST_ELEM_GET_NEXT(*flistpp,
        PIN_FLD_MONITOR_IMPACTS, &record_id, 1, &cookie_id,
            ebufp)) != (pin_flist_t *)NULL) {

                resourcep = (int32 *)PIN_FLIST_FLD_GET(cur_mon_impact_flistp,
                    PIN_FLD_RESOURCE_ID, 0, ebufp);
                if ( !resourcep ) {
                        pin_set_err(ebufp, PIN_ERRLOC_APP,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_NULL_PTR, PIN_FLD_RESOURCE_ID, 0, 0);
                        goto ErrNullPointer;
                }
                cur_resource_id = *resourcep;
               
                cur_amt_p = (pin_decimal_t *)PIN_FLIST_FLD_GET(
                    cur_mon_impact_flistp, PIN_FLD_AMOUNT, 0, ebufp);
                cur_amt = pin_decimal_to_double(cur_amt_p, ebufp);

                record_id2 = 0;
                cookie_id2 = NULL;

                /*
                 * Now walk all prev impacts looking for a match
                 */
                while ((prev_mon_impact_flistp = PIN_FLIST_ELEM_GET_NEXT(
                    prev_impact_flistp, PIN_FLD_MONITOR_IMPACTS, &record_id2, 1,
                    &cookie_id2, ebufp)) != (pin_flist_t *)NULL) {

                    resourcep = (int32 *)PIN_FLIST_FLD_GET(
	                prev_mon_impact_flistp,
       		        PIN_FLD_RESOURCE_ID, 0, ebufp);
                	if ( !resourcep ) {
                        	pin_set_err(ebufp, PIN_ERRLOC_APP,
                        	PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        	PIN_ERR_NULL_PTR, PIN_FLD_RESOURCE_ID, 0, 0);
                        	goto ErrNullPointer;
                	}
                        prev_resource_id = *resourcep;
                    

                        prev_amt_p = (pin_decimal_t *)PIN_FLIST_FLD_GET(
                            prev_mon_impact_flistp, PIN_FLD_AMOUNT, 0,
                            ebufp);
                        prev_amt = pin_decimal_to_double(prev_amt_p, ebufp);

                        if ((cur_resource_id == prev_resource_id) &&
                            ((cur_amt + prev_amt) == 0)) {

                                PIN_FLIST_ELEM_DROP(*flistpp,
                                    PIN_FLD_MONITOR_IMPACTS, record_id, ebufp);
                        cookie_id = cookie_tmp;

                                break;
                                /****/
                        }
                }

        cookie_tmp = cookie_id;
        }
ErrNullPointer:
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "analyzeMonitorImpacts error", ebufp);
                return;
                /*****/
        }
}


/****************************************************************************
 * Function     : analyzeSubBalImpacts
 * Description  : Update any Sub bal impacts for the latest event object.
 *--------------+------------------------------------------------------------
 * In/Out       : pin_flist_t   **flistpp
 * In           : pin_flist_t   *prev_impact_flistp
 *--------------+------------------------------------------------------------
 * Returns      : void
 ****************************************************************************/

static void 
analyzeSubBalImpacts(pin_flist_t **flistpp, 
                     pin_flist_t *prev_impact_flistp, 
                     pin_fld_num_t fld_type, 
                     pin_errbuf_t	*ebufp) 
{

        pin_flist_t     *cur_sub_bal_impact_flistp = NULL; 
        pin_flist_t     *prev_sub_bal_impact_flistp = NULL; 
	pin_flist_t     *prev_sub_bal_flistp = NULL ; 
	pin_flist_t     *cur_sub_bal_flistp = NULL ; 
        pin_flist_t     *temp_flist = NULL ;
	pin_decimal_t   *cur_amt_p = NULL; 
        pin_decimal_t   *prev_amt_p = NULL;
        pin_decimal_t   *total_amt_p = NULL ;
        poid_t          *cur_bal_group_poid = NULL; 
        poid_t          *prev_bal_group_poid = NULL; 
        int32           cur_resource_id = 0; 
        int32           prev_resource_id = 0; 
        int32           record_id = 0; 
        int32           record_sub_id = 0; 
        int32           recordFound = 0 ;
        int32           temp_no_element = 0 ;
        int32           record_id2 = 0; 
        int32           *resourcep = NULL;
        pin_cookie_t	cookie_id = NULL; 
        pin_cookie_t	cookie_id2 = NULL; 
        pin_cookie_t	cookie_tmp = NULL; 
	pin_cookie_t    cookie_sub_cur = NULL ; 
	pin_cookie_t    cookie_sub_prev = NULL ; 

        if (PIN_ERR_IS_ERR(ebufp)) {
                return;
                /*****/
        }

        while(( prev_sub_bal_impact_flistp = PIN_FLIST_ELEM_GET_NEXT( prev_impact_flistp, 
            fld_type, &record_id, 1, &cookie_id,ebufp ) ) != (pin_flist_t *)NULL) { 
                recordFound = 0 ; 
                resourcep = (int32 *) PIN_FLIST_FLD_GET( prev_sub_bal_impact_flistp, 
                    PIN_FLD_RESOURCE_ID, 0,ebufp)  ;
                if ( !resourcep ) {
                        pin_set_err(ebufp, PIN_ERRLOC_APP,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_NULL_PTR, PIN_FLD_RESOURCE_ID, 0, 0);
                        goto ErrNullPointer;
                }
                prev_resource_id  = *resourcep ;
            

                prev_bal_group_poid = (poid_t *) PIN_FLIST_FLD_GET( prev_sub_bal_impact_flistp, 
                    PIN_FLD_BAL_GRP_OBJ, 0, ebufp);
                if ( !prev_bal_group_poid ) {
                        pin_set_err(ebufp, PIN_ERRLOC_APP,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_NULL_PTR, PIN_FLD_BAL_GRP_OBJ, 0, 0);
                        goto ErrNullPointer;
                }
                record_id2 = 0;
                cookie_id2 = NULL;
 
                /* 
                 * Now walk all current impacts looking for a match 
                 */ 
                while (( cur_sub_bal_impact_flistp = PIN_FLIST_ELEM_GET_NEXT( 
                    *flistpp, PIN_FLD_SUB_BAL_IMPACTS, &record_id2, 1,
                    &cookie_id2, ebufp)) != (pin_flist_t *)NULL) { 
                        cookie_sub_cur = 0 ; 
                        cookie_sub_prev = 0; 
                        record_sub_id = 0; 
			cur_amt_p = NULL ;
			prev_amt_p = NULL ;
			total_amt_p = NULL ;

                        resourcep = (int32 *)PIN_FLIST_FLD_GET(	cur_sub_bal_impact_flistp , 
                            PIN_FLD_RESOURCE_ID, 0, ebufp) ; 
                	if ( !resourcep ) {
                        	pin_set_err(ebufp, PIN_ERRLOC_APP,
                        	PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        	PIN_ERR_NULL_PTR, PIN_FLD_RESOURCE_ID, 0, 0);
                        	goto ErrNullPointer;
                	}
                        cur_resource_id  = *resourcep; 
                       
 
                        cur_bal_group_poid  = (poid_t *) PIN_FLIST_FLD_GET( 
                            cur_sub_bal_impact_flistp , PIN_FLD_BAL_GRP_OBJ, 0,  
                            ebufp); 
	                if ( !cur_bal_group_poid ) {
                        	pin_set_err(ebufp, PIN_ERRLOC_APP,
                        	PIN_ERRCLASS_SYSTEM_DETERMINATE,
                       	 	PIN_ERR_NULL_PTR, PIN_FLD_BAL_GRP_OBJ, 0, 0);
                        	goto ErrNullPointer;
                	}

                        if( ( !PIN_POID_COMPARE(cur_bal_group_poid,prev_bal_group_poid,0,ebufp) ) &&
                            ( cur_resource_id == prev_resource_id ) ) {
                                while((cur_sub_bal_flistp = PIN_FLIST_ELEM_GET_NEXT(cur_sub_bal_impact_flistp,
                                     PIN_FLD_SUB_BALANCES, &record_sub_id, 1, &cookie_sub_cur, ebufp)) != NULL) {
                                	total_amt_p = pbo_decimal_from_double(0.0, ebufp);
                                        cur_amt_p = (pin_decimal_t *)PIN_FLIST_FLD_GET( cur_sub_bal_flistp,
                              					PIN_FLD_AMOUNT, 0,ebufp ) ;
                                	if( cur_amt_p ) {
                                     		pbo_decimal_add_assign(total_amt_p, cur_amt_p, ebufp);
					}
					if((prev_sub_bal_flistp = PIN_FLIST_ELEM_GET(prev_sub_bal_impact_flistp,
						PIN_FLD_SUB_BALANCES, record_sub_id, 1, ebufp)) != NULL) {
							prev_amt_p = (pin_decimal_t *)PIN_FLIST_FLD_GET( prev_sub_bal_flistp,
									PIN_FLD_AMOUNT, 0,ebufp);
                                			if( prev_amt_p ) {
			           				 pbo_decimal_add_assign(total_amt_p, prev_amt_p, ebufp);
							}
					}

                                	PIN_FLIST_FLD_PUT( cur_sub_bal_flistp,PIN_FLD_AMOUNT,total_amt_p,ebufp ) ;													
                                }


                                recordFound = 1 ; 
                        }
                  }
                  if( !recordFound ) {
                      temp_flist = PIN_FLIST_COPY( prev_sub_bal_impact_flistp,ebufp ) ;
                      temp_no_element = PIN_FLIST_ELEM_COUNT( *flistpp, PIN_FLD_SUB_BAL_IMPACTS, ebufp);
                      PIN_FLIST_ELEM_ADD(*flistpp, fld_type, temp_no_element,ebufp ) ; 
                      PIN_FLIST_ELEM_PUT(*flistpp,temp_flist,fld_type,temp_no_element,ebufp ) ;
                 }
        }


ErrNullPointer:
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "analyzeSubBalImpacts error", ebufp);
                return;
                /*****/
        }
}

/**************************************************************************** 
 * Function     : addSubBalImpactsPoid 
 * Description  : Add Sub bal impacts back from the latest event object 
 *              : plus the original evt poid (from the latest event) 
 *--------------+------------------------------------------------------------ 
 * In           : pin_flist_t   *curr_impact_flistp 
 * In           : pin_flist_t   *evt_flistp 
 * In           : pin_errbuf_t  *ebufp 
 *--------------+------------------------------------------------------------ 
 * Returns      : void 
 ****************************************************************************/ 
static void 
addSubBalImpactsPoid( 
	pin_flist_t *curr_impact_flistp, 
	pin_flist_t *evt_flistp, 
	pin_errbuf_t	*ebufp)
{ 
 
        pin_flist_t  *cur_bal_impact_flistp = NULL; 
        poid_t       *orig_evt_poidp = NULL; 
         
        int32           record_id = 0; 
        pin_cookie_t	cookie_id = NULL; 

        if (PIN_ERR_IS_ERR(ebufp)) {
                return;
                /*****/
        }

        if (PIN_ERR_IS_ERR(ebufp)) {
                return;
                /*****/
        }

        orig_evt_poidp = (char *)PIN_FLIST_FLD_GET(curr_impact_flistp, 
            PIN_FLD_POID, 0, ebufp); 
        PIN_FLIST_FLD_SET(evt_flistp, PIN_FLD_POID,  
	    (poid_t *)orig_evt_poidp, ebufp); 
                 
        while ((cur_bal_impact_flistp = PIN_FLIST_ELEM_GET_NEXT( 
            curr_impact_flistp, PIN_FLD_SUB_BAL_IMPACTS, &record_id, 1,  
            &cookie_id, ebufp)) != (pin_flist_t *)NULL) { 
 
                PIN_FLIST_ELEM_SET(evt_flistp, cur_bal_impact_flistp, 
                    PIN_FLD_SUB_BAL_IMPACTS , record_id, ebufp); 
        }         
}          

/****************************************************************************
 * Function     : addMonitorSubBalImpactsPoid
 * Description  : Add Monitor Sub bal impacts back from the latest event object
 *              : plus the original evt poid (from the latest event)
 *--------------+------------------------------------------------------------
 * In           : pin_flist_t   *curr_impact_flistp
 * In           : pin_flist_t   *evt_flistp
 * In           : pin_errbuf_t  *ebufp
 *--------------+------------------------------------------------------------
 * Returns      : void
 ****************************************************************************/
static void
addMonitorSubBalImpactsPoid(
    pin_flist_t *curr_impact_flistp,
    pin_flist_t *evt_flistp,
    pin_errbuf_t    *ebufp)
{
        pin_flist_t  *cur_bal_impact_flistp = NULL;
        poid_t       *orig_evt_poidp = NULL;

        int32           record_id = 0;
        pin_cookie_t    cookie_id = NULL;

        if (PIN_ERR_IS_ERR(ebufp)) {
                return;
                /*****/
        }

        orig_evt_poidp = (char *)PIN_FLIST_FLD_GET(curr_impact_flistp,
            PIN_FLD_POID, 0, ebufp);
        PIN_FLIST_FLD_SET(evt_flistp, PIN_FLD_POID,
        (poid_t *)orig_evt_poidp, ebufp);

        while ((cur_bal_impact_flistp = PIN_FLIST_ELEM_GET_NEXT(
            curr_impact_flistp, PIN_FLD_MONITOR_SUB_BAL_IMPACTS, &record_id, 1,
            &cookie_id, ebufp)) != (pin_flist_t *)NULL) {

                PIN_FLIST_ELEM_SET(evt_flistp, cur_bal_impact_flistp,
                    PIN_FLD_MONITOR_SUB_BAL_IMPACTS , record_id, ebufp);
        }
}

/****************************************************************************
 * Function     : addBalImpactsPoid
 * Description  : Add bal impacts back from the latest event object
 *              : plus the original evt poid (from the latest event)
 *--------------+------------------------------------------------------------
 * In           : pin_flist_t   *curr_impact_flistp
 * In           : pin_flist_t   *evt_flistp
 * In           : pin_errbuf_t  *ebufp
 *--------------+------------------------------------------------------------
 * Returns      : void
 ****************************************************************************/
static void
addBalImpactsPoid(
	pin_flist_t *curr_impact_flistp,
	pin_flist_t *evt_flistp,
	pin_errbuf_t	*ebufp)
{
        pin_flist_t  *cur_bal_impact_flistp = NULL;
        poid_t       *orig_evt_poidp = NULL;
        
        int32           record_id = 0;
        pin_cookie_t	cookie_id = NULL;

        if (PIN_ERR_IS_ERR(ebufp)) {
                return;
                /*****/
        }

        orig_evt_poidp = (char *)PIN_FLIST_FLD_GET(curr_impact_flistp,
           PIN_FLD_POID, 0, ebufp);
        PIN_FLIST_FLD_SET(evt_flistp, PIN_FLD_POID, 
			(poid_t *)orig_evt_poidp, ebufp);
                
        while ((cur_bal_impact_flistp = PIN_FLIST_ELEM_GET_NEXT(
            curr_impact_flistp, PIN_FLD_BAL_IMPACTS, &record_id, 1, 
            &cookie_id, ebufp)) != (pin_flist_t *)NULL) {

                PIN_FLIST_ELEM_SET(evt_flistp, cur_bal_impact_flistp,
                    PIN_FLD_BAL_IMPACTS, record_id, ebufp);
        }        
}
               
/****************************************************************************
 * Function     : addMonitorImpactsPoid
 * Description  : Add Monitor impacts back from the latest event object
 *              : plus the original evt poid (from the latest event)
 *--------------+------------------------------------------------------------
 * In           : pin_flist_t   *curr_impact_flistp
 * In           : pin_flist_t   *evt_flistp
 * In           : pin_errbuf_t  *ebufp
 *--------------+------------------------------------------------------------
 * Returns      : void
 ****************************************************************************/
static void
addMonitorImpactsPoid(
    pin_flist_t *curr_impact_flistp,
    pin_flist_t *evt_flistp,
    pin_errbuf_t    *ebufp)
{
        pin_flist_t  *cur_mon_impact_flistp = NULL;
        poid_t       *orig_evt_poidp = NULL;

        int32           record_id = 0;
        pin_cookie_t    cookie_id = NULL;

        if (PIN_ERR_IS_ERR(ebufp)) {
                return;
                /*****/
        }

        orig_evt_poidp = (char *)PIN_FLIST_FLD_GET(curr_impact_flistp,
           PIN_FLD_POID, 0, ebufp);
        PIN_FLIST_FLD_SET(evt_flistp, PIN_FLD_POID,
            (poid_t *)orig_evt_poidp, ebufp);

        while ((cur_mon_impact_flistp = PIN_FLIST_ELEM_GET_NEXT(
            curr_impact_flistp, PIN_FLD_MONITOR_IMPACTS, &record_id, 1,
            &cookie_id, ebufp)) != (pin_flist_t *)NULL) {

                PIN_FLIST_ELEM_SET(evt_flistp, cur_mon_impact_flistp,
                    PIN_FLD_MONITOR_IMPACTS, record_id, ebufp);
        }
}
/****************************************************************************
 * Function     : processSessionBasicDetailImt
 * Description  : Builds the report record for a basic detail record
 *--------------+------------------------------------------------------------
 * In           : pin_buf_t	*edr
 * In           : pin_flist_t	*imt_flistp
 * In           : int32 number_assoc_recs
 * In           : const char      *record_type
 * In			: pin_errbuf_t	*ebufp
 *--------------+------------------------------------------------------------
 * Returns      : void
 ****************************************************************************/
static void
processSessionBasicDetailImt(
	pin_buf_t	*edr,
	pin_flist_t *imt_flistp,
        int32 number_assoc_recs,
        const char *record_type,
	pin_errbuf_t	*ebufp)
{
        pin_flist_t	*telco_info_flistp = NULL;
        pin_flist_t	*bal_impact_flistp = NULL;
                
        int32           record_id = 0;
        pin_cookie_t	cookie_id = NULL;
                
        char            generic_conv[256];
        int32           *generic_int = NULL;
        char            *generic_char = NULL;
        time_t          *generic_time = NULL;
        time_t          *generic_time2 = NULL;
        pin_decimal_t   *generic_decimal= NULL;

        int32           num_service_codes = 0;
        int32           num_impact_packets = 0;
        int32           num_mon_impact_packets = 0;
        char            integrate_not_used[2] = "0";
        
        
        if (PIN_ERR_IS_ERR(ebufp)) {
		return;
                /*****/
	}

        telco_info_flistp = PIN_FLIST_SUBSTR_GET(imt_flistp, 
			PIN_FLD_TELCO_INFO, 1, ebufp);

	if (!telco_info_flistp) {

              goto PROCESS_FILE_ERROR;
              /**********************/
        }

        record_id = 0;
        cookie_id = NULL;

        bal_impact_flistp = PIN_FLIST_ELEM_GET_NEXT(imt_flistp,
            PIN_FLD_BAL_IMPACTS, &record_id, 1, &cookie_id, ebufp);


	if (bal_impact_flistp == (pin_flist_t *)NULL) {

              goto PROCESS_FILE_ERROR;
              /**********************/
        }

        /*
         * Construct the flist in the correct order
         * Spec: sol42 CDR/EDR/IDR Format 6.3
         */
        
        /*
         * RECORD TYPE 
         */
        formatFieldsForFile(edr, 
		(char *)FILE_FLD_FORMAT_9,(char *)record_type,3, ebufp);
        
        /*
         * RECORD NUMBER 
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)RECORD_PLACE_HOLDER,9,ebufp);
        
        /**
         * RA Changes - Start
        **/

        generic_char = (char *)PIN_FLIST_FLD_GET(imt_flistp,
           PIN_FLD_EVENT_NO, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr, 
		(char *)FILE_FLD_FORMAT_9,(char *)generic_char,60, ebufp);

        addStr(edr, DELIMITER, ebufp);

        generic_char = (char *)PIN_FLIST_FLD_GET(imt_flistp,
           PIN_FLD_ORIGINAL_BATCH_ID, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr, 
		(char *)FILE_FLD_FORMAT_9,(char *)generic_char,80, ebufp);

        /**
         * RA Changes - End
        **/

        /*
         * DISCARDING - DEFAULT OF 0
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,1,ebufp);
        
        /*
         * CHAIN REFERENCE 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_NETWORK_SESSION_ID, 0, ebufp);
	if ( !generic_int ) {
        	pin_set_err(ebufp, PIN_ERRLOC_APP,
        	PIN_ERRCLASS_SYSTEM_DETERMINATE,
        	PIN_ERR_NULL_PTR, PIN_FLD_NETWORK_SESSION_ID, 0, 0);
        	goto ErrNullPointer;
        }
        sprintf(generic_conv, "%ld", *generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,generic_conv,10,ebufp);

        /*
         * SOURCE NETWORK TYPE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * SOURCE NETWORK 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_ORIGIN_NETWORK, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr, 
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,14,ebufp);

        /*
         * DESTINATION NETWORK TYPE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        
        /*
         * DESTINATION NETWORK 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_DESTINATION_NETWORK, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,14,ebufp);

        /*
         * TYPE OF A-IDENTIFICATION - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * A-MODIFICATION INDICATOR - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * A-TYPE OF NUMBER - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)integrate_not_used,1,ebufp);

        /*
         * A-NUMBERING PLAN - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * A-NUMBER 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_PRIMARY_MSID, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,40,ebufp);

        /*
         * B-MODIFICATION INDICATOR 
         */

		  addStr(edr, DELIMITER, ebufp);

        /*
         * B-TYPE OF NUMBER - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)integrate_not_used,1,ebufp);

        /*
         * B-NUMBERING PLAN - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * B-NUMBER 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_CALLED_TO, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,40,ebufp);

        /*
         * DESCRIPTION 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(imt_flistp,
           PIN_FLD_DESCR, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,50,ebufp);

        /*
         * C-MODIFICATION INDICATOR - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * C-TYPE OF NUMBER - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)integrate_not_used,1,ebufp);

        /*
         * C-NUMBERING PLAN - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * C-NUMBER - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * USAGE DIRECTION 
         */
		  addStr(edr, DELIMITER, ebufp);


        /*
         * CONNECT TYPE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * CONNECT SUB TYPE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * BASIC SERVICE 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_SVC_TYPE, 0, ebufp);
	if ( !generic_char ) {
        	pin_set_err(ebufp, PIN_ERRLOC_APP,
        	PIN_ERRCLASS_SYSTEM_DETERMINATE,
        	PIN_ERR_NULL_PTR, PIN_FLD_SVC_TYPE, 0, 0);
        	goto ErrNullPointer;
        }
        strcpy(generic_conv, generic_char);
        generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_SVC_CODE, 0, ebufp);
        if ( !generic_char ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_SVC_CODE, 0, 0);
                goto ErrNullPointer;
        }
        strcat(generic_conv, generic_char);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,3,ebufp);
        
        /*
         * QUALITY OF SERVICE REQUESTED 
         */
		  addStr(edr, DELIMITER, ebufp);

        /*
         * QUALITY OF SERVICE USED 
         */
		 addStr(edr, DELIMITER, ebufp);

        /*
         * CALL COMPLETION INDICATOR 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_TERMINATE_CAUSE, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_TERMINATE_CAUSE, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv, "%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr, 
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,3, ebufp);

        /*
         * LONG DURATION INDICATOR 
         */
		 addStr(edr, DELIMITER, ebufp);

        /*
         *  CHARGING START TIMESTAMP
         */
        generic_time = (time_t *)PIN_FLIST_FLD_GET(imt_flistp,
           PIN_FLD_START_T, 0, ebufp);
        if ( !generic_time ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_START_T, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv, "%ld",*generic_time);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)generic_conv,
            strlen(generic_conv),ebufp);

        /*
         *  CHARGING END TIMESTAMP
         */
        generic_time = (time_t *)PIN_FLIST_FLD_GET(imt_flistp,
           PIN_FLD_END_T, 0, ebufp);
        if ( !generic_time ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_END_T, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv, "%ld",*generic_time);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)generic_conv,
            strlen(generic_conv),ebufp);

        /*
         *  CREATED TIMESTAMP
         */
        generic_time = (time_t *)PIN_FLIST_FLD_GET(imt_flistp,
           PIN_FLD_CREATED_T, 0, ebufp);
        if ( !generic_time ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_CREATED_T, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv, "%ld",*generic_time);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)generic_conv,
            strlen(generic_conv),ebufp);

        /*
         * UTC TIME OFFSET 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(imt_flistp,
           PIN_FLD_TIMEZONE_ID, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,5,ebufp);

        /*
         *  DURATION
         */
        addStr(edr, DELIMITER, ebufp);
        generic_time  = (time_t *)PIN_FLIST_FLD_GET(imt_flistp, 
                                                    PIN_FLD_START_T, 0, ebufp);
        if ( !generic_time ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_START_T, 0, 0);
                goto ErrNullPointer;
        }
        generic_time2 = (time_t *)PIN_FLIST_FLD_GET(imt_flistp, 
                                                    PIN_FLD_END_T, 0, ebufp);
        if ( !generic_time2 ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_END_T, 0, 0);
                goto ErrNullPointer;
        }

        /* If end_t is greater or equal to start_t, 
         * then we have a valid scenario for calculating duration
         */
        if ((generic_time2 != NULL) && (generic_time != NULL) && 
            (*generic_time2 >= *generic_time))
        {
          int32 duration= *generic_time2 - *generic_time;
          sprintf(generic_conv,"%ld",duration);

        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,15,ebufp);
        }
        else
        {
          formatFieldsForFile(edr, (char *)FILE_FLD_FORMAT_9,
                             (char *)integrate_not_used,15,ebufp);
        }

        /*
         *  DURATION UoM
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)FIELD_DURATION_UOM,3, ebufp);

        /*
         * VOLUME SENT 
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,15,ebufp);

        /*
         *  VOLUME SENT UOM
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)FIELD_VOLUME_SENT_UOM,3,ebufp);

        /*
         * VOLUME RECEIVED 
         */

	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,15,ebufp);

        /*
         *  VOLUME RECEIVED UOM
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)FIELD_VOLUME_RECEIVED_UOM,
		3,ebufp);

        /*
         * NUMBER OF UNITS 
         */

	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,15,ebufp);

        /*
         *  NUMBER OF UNITS UoM
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)FIELD_NUMBER_OF_UNITS_UOM,
		3,ebufp);

        /*
         * RETAIL IMPACT CATEGORY - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * RETAIL CHARGED AMOUNT VALUE - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,11,ebufp);

        /*
         * RETAIL CHARGED AMOUNT CURRENCY - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * RETAIL CHARGED TAX TREATMENT - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * RETAIL CHARGED TAX RATE - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,4,ebufp);

        /*
         * WHOLESALE IMPACT CATEGORY - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * WHOLESALE CHARGED AMOUNT VALUE - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,11,ebufp);

        /*
         * WHOLESALE CHARGED AMOUNT CURRENCY - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * WHOLESALE CHARGED TAX TREATMENT - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * WHOLESALE CHARGED TAX RATE - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,4,ebufp);

        /*
         * ZONE DESCRIPTION 
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * IC DESCRIPTION 
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * ZONE ENTRY NAME
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * TARIFF CLASS - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * TARIFF SUB CLASS - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * USAGE CLASS 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_USAGE_CLASS, 0, ebufp);
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,5,ebufp);

        /*
         * USAGE TYPE 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_USAGE_TYPE, 0, ebufp);
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,5,ebufp);

        /*
         * BILLCYCLE PERIOD - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * PREPAID INDICATOR - DEFAULT
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,2,ebufp);

        /*
         * NUMBER ASSOCIATED RECORDS
         */
        num_service_codes = PIN_FLIST_ELEM_COUNT(imt_flistp, 
            PIN_FLD_SERVICE_CODES, ebufp);
        number_assoc_recs += num_service_codes;
        num_impact_packets = PIN_FLIST_ELEM_COUNT(imt_flistp, 
            PIN_FLD_BAL_IMPACTS, ebufp);
        number_assoc_recs += num_impact_packets;
        num_mon_impact_packets = PIN_FLIST_ELEM_COUNT(imt_flistp, 
            PIN_FLD_MONITOR_IMPACTS, ebufp);
        number_assoc_recs += num_mon_impact_packets;
        sprintf(generic_conv, "%ld",number_assoc_recs);
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,2,ebufp);

        /*
         * NETWORK ELEMENT START DATE/TIME STAMP - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * NETWORK ELEMENT END DATE/TIME STAMP - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * NETWORK ELEMENT START DATE/UTC TIME OFFSET - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * NETWORK ELEMENT END DATE/UTC TIME OFFSET - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);


        /*
         * UTC END TIME OFFSET - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * INCOMING ROUTE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);


        /*
         * ROUTING CATEGORY - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * INTERN PROCESS STATUS - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * DROPPED CALL QUANTITY - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)integrate_not_used,1,ebufp);

        /*
         * DROPPED CALL STATUS - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)integrate_not_used,1,ebufp);

        /*
         * End of record (line) indicator
         */
	addStr(edr, EOL_DELIMITER, ebufp);

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, (char *)edr->data);

        return;
        /*****/

PROCESS_FILE_ERROR:

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, 
            (char *)"*** MISSING IMT INFO FIELDS - "
            "UNABLE TO PROCESS ***", 
            imt_flistp);

        return;
        /*****/

ErrNullPointer:
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "processSessionBasicDetailImt error", ebufp);
                return;
                /*****/
        }
}

/****************************************************************************
 * Function     : processSessionBasicDetailPdc
 * Description  : Builds the report record for a basic detail record
 *--------------+------------------------------------------------------------
 * In           : pin_buf_t	*edr
 * In           : pin_flist_t	*pdc_flistp
 * In           : int32 number_assoc_recs
 * In           : const char      *record_type
 * In		    : pin_errbuf_t	*ebufp
 *--------------+------------------------------------------------------------
 * Returns      : void
 ****************************************************************************/
static void
processSessionBasicDetailPdc(
	pin_buf_t	*edr,
	pin_flist_t *pdc_flistp,
        int32 number_assoc_recs,
        const char *record_type,
	pin_errbuf_t	*ebufp)
{
        pin_flist_t	*telco_info_flistp = NULL;
        pin_flist_t	*bal_impact_flistp = NULL;
                
        int32           record_id = 0;
        pin_cookie_t	cookie_id = NULL;
                
        char            generic_conv[256];
        int32           *generic_int = NULL;
        char            *generic_char = NULL;
        time_t          *generic_time = NULL;
        time_t          *generic_time2 = NULL;
        pin_decimal_t   *generic_decimal= NULL;

        int32           num_service_codes = 0;
        int32           num_impact_packets = 0;
        int32           num_mon_impact_packets = 0;
        char            integrate_not_used[2] = "0";
        
        
        if (PIN_ERR_IS_ERR(ebufp)) {
		return;
                /*****/
	}

        telco_info_flistp = PIN_FLIST_SUBSTR_GET(pdc_flistp, 
			PIN_FLD_TELCO_INFO, 1, ebufp);

	if (!telco_info_flistp) {

              goto PROCESS_FILE_ERROR;
              /**********************/
        }

        record_id = 0;
        cookie_id = NULL;

        bal_impact_flistp = PIN_FLIST_ELEM_GET_NEXT(pdc_flistp,
            PIN_FLD_BAL_IMPACTS, &record_id, 1, &cookie_id, ebufp);


	if (bal_impact_flistp == (pin_flist_t *)NULL) {

              goto PROCESS_FILE_ERROR;
              /**********************/
        }

        /*
         * Construct the flist in the correct order
         * Spec: sol42 CDR/EDR/IDR Format 6.3
         */
        
        /*
         * RECORD TYPE 
         */
        formatFieldsForFile(edr, 
		(char *)FILE_FLD_FORMAT_9,(char *)record_type,3, ebufp);
        
        /*
         * RECORD NUMBER 
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)RECORD_PLACE_HOLDER,9,ebufp);
        
        /**
         * RA Changes - Start
        **/

        generic_char = (char *)PIN_FLIST_FLD_GET(pdc_flistp,
           PIN_FLD_EVENT_NO, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr, 
		(char *)FILE_FLD_FORMAT_9,(char *)generic_char,60, ebufp);

        addStr(edr, DELIMITER, ebufp);

        generic_char = (char *)PIN_FLIST_FLD_GET(pdc_flistp,
           PIN_FLD_ORIGINAL_BATCH_ID, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr, 
		(char *)FILE_FLD_FORMAT_9,(char *)generic_char,80, ebufp);

        /**
         * RA Changes - End
        **/

        /*
         * DISCARDING - DEFAULT OF 0
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,1,ebufp);
        
        /*
         * CHAIN REFERENCE 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_NETWORK_SESSION_ID, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_NETWORK_SESSION_ID, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv, "%ld", *generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,generic_conv,10,ebufp);

        /*
         * SOURCE NETWORK TYPE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * SOURCE NETWORK 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_ORIGIN_NETWORK, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr, 
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,14,ebufp);

        /*
         * DESTINATION NETWORK TYPE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        
        /*
         * DESTINATION NETWORK 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_DESTINATION_NETWORK, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,14,ebufp);

        /*
         * TYPE OF A-IDENTIFICATION - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * A-MODIFICATION INDICATOR - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * A-TYPE OF NUMBER - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)integrate_not_used,1,ebufp);

        /*
         * A-NUMBERING PLAN - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * A-NUMBER 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_PRIMARY_MSID, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,40,ebufp);

        /*
         * B-MODIFICATION INDICATOR 
         */

		  addStr(edr, DELIMITER, ebufp);

        /*
         * B-TYPE OF NUMBER - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)integrate_not_used,1,ebufp);

        /*
         * B-NUMBERING PLAN - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * B-NUMBER 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_CALLED_TO, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,40,ebufp);

        /*
         * DESCRIPTION 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(pdc_flistp,
           PIN_FLD_DESCR, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,50,ebufp);

        /*
         * C-MODIFICATION INDICATOR - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * C-TYPE OF NUMBER - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)integrate_not_used,1,ebufp);

        /*
         * C-NUMBERING PLAN - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * C-NUMBER - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * USAGE DIRECTION 
         */
		  addStr(edr, DELIMITER, ebufp);


        /*
         * CONNECT TYPE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * CONNECT SUB TYPE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * BASIC SERVICE 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_SVC_TYPE, 0, ebufp);
        if ( !generic_char ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_SVC_TYPE, 0, 0);
                goto ErrNullPointer;
        }
        strcpy(generic_conv, generic_char);
        generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_SVC_CODE, 0, ebufp);
        if ( !generic_char ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_SVC_CODE, 0, 0);
                goto ErrNullPointer;
        }
        strcat(generic_conv, generic_char);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,3,ebufp);
        
        /*
         * QUALITY OF SERVICE REQUESTED 
         */
		  addStr(edr, DELIMITER, ebufp);

        /*
         * QUALITY OF SERVICE USED 
         */
		 addStr(edr, DELIMITER, ebufp);

        /*
         * CALL COMPLETION INDICATOR 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_TERMINATE_CAUSE, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_TERMINATE_CAUSE, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv, "%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr, 
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,3, ebufp);

        /*
         * LONG DURATION INDICATOR 
         */
		 addStr(edr, DELIMITER, ebufp);

        /*
         *  CHARGING START TIMESTAMP
         */
        generic_time = (time_t *)PIN_FLIST_FLD_GET(pdc_flistp,
           PIN_FLD_START_T, 0, ebufp);
        if ( !generic_time ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_START_T, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv, "%ld",*generic_time);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)generic_conv,
            strlen(generic_conv),ebufp);

        /*
         *  CHARGING END TIMESTAMP
         */
        generic_time = (time_t *)PIN_FLIST_FLD_GET(pdc_flistp,
           PIN_FLD_END_T, 0, ebufp);
        if ( !generic_time ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_END_T, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv, "%ld",*generic_time);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)generic_conv,
            strlen(generic_conv),ebufp);

        /*
         *  CREATED TIMESTAMP
         */
        generic_time = (time_t *)PIN_FLIST_FLD_GET(pdc_flistp,
           PIN_FLD_CREATED_T, 0, ebufp);
        if ( !generic_time ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_CREATED_T, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv, "%ld",*generic_time);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)generic_conv,
            strlen(generic_conv),ebufp);

        /*
         * UTC TIME OFFSET 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(pdc_flistp,
           PIN_FLD_TIMEZONE_ID, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,5,ebufp);

        /*
         *  DURATION
         */
        addStr(edr, DELIMITER, ebufp);
        generic_time  = (time_t *)PIN_FLIST_FLD_GET(pdc_flistp, 
                                                    PIN_FLD_START_T, 0, ebufp);
        if ( !generic_time ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_START_T, 0, 0);
                goto ErrNullPointer;
        }
        generic_time2 = (time_t *)PIN_FLIST_FLD_GET(pdc_flistp, 
                                                    PIN_FLD_END_T, 0, ebufp);
        if ( !generic_time2 ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_END_T, 0, 0);
                goto ErrNullPointer;
        }

        /* If end_t is greater or equal to start_t, 
         * then we have a valid scenario for calculating duration
         */
        if ((generic_time2 != NULL) && (generic_time != NULL) && 
            (*generic_time2 >= *generic_time))
        {
          int32 duration= *generic_time2 - *generic_time;
          sprintf(generic_conv,"%ld",duration);

        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,15,ebufp);
        }
        else
        {
          formatFieldsForFile(edr, (char *)FILE_FLD_FORMAT_9,
                             (char *)integrate_not_used,15,ebufp);
        }

        /*
         *  DURATION UoM
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)FIELD_DURATION_UOM,3, ebufp);

        /*
         * VOLUME SENT 
         */

	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,15,ebufp);

        /*
         *  VOLUME SENT UOM
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)FIELD_VOLUME_SENT_UOM,3,ebufp);

        /*
         * VOLUME RECEIVED 
         */

	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,15,ebufp);

        /*
         *  VOLUME RECEIVED UOM
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)FIELD_VOLUME_RECEIVED_UOM,
		3,ebufp);

        /*
         * NUMBER OF UNITS 
         */

	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,15,ebufp);

        /*
         *  NUMBER OF UNITS UoM
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)FIELD_NUMBER_OF_UNITS_UOM,
		3,ebufp);

        /*
         * RETAIL IMPACT CATEGORY - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * RETAIL CHARGED AMOUNT VALUE - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,11,ebufp);

        /*
         * RETAIL CHARGED AMOUNT CURRENCY - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * RETAIL CHARGED TAX TREATMENT - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * RETAIL CHARGED TAX RATE - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,4,ebufp);

        /*
         * WHOLESALE IMPACT CATEGORY - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * WHOLESALE CHARGED AMOUNT VALUE - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,11,ebufp);

        /*
         * WHOLESALE CHARGED AMOUNT CURRENCY - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * WHOLESALE CHARGED TAX TREATMENT - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * WHOLESALE CHARGED TAX RATE - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,4,ebufp);

        /*
         * ZONE DESCRIPTION 
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * IC DESCRIPTION 
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * ZONE ENTRY NAME
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * TARIFF CLASS - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * TARIFF SUB CLASS - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * USAGE CLASS 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_USAGE_CLASS, 0, ebufp);
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,5,ebufp);

        /*
         * USAGE TYPE 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_USAGE_TYPE, 0, ebufp);
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,5,ebufp);

        /*
         * BILLCYCLE PERIOD - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * PREPAID INDICATOR - DEFAULT
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,2,ebufp);

        /*
         * NUMBER ASSOCIATED RECORDS
         */
        num_service_codes = PIN_FLIST_ELEM_COUNT(pdc_flistp, 
            PIN_FLD_SERVICE_CODES, ebufp);
        number_assoc_recs += num_service_codes;
        num_impact_packets = PIN_FLIST_ELEM_COUNT(pdc_flistp, 
            PIN_FLD_BAL_IMPACTS, ebufp);
        number_assoc_recs += num_impact_packets;
        num_mon_impact_packets = PIN_FLIST_ELEM_COUNT(pdc_flistp, 
            PIN_FLD_MONITOR_IMPACTS, ebufp);
        number_assoc_recs += num_mon_impact_packets;
        sprintf(generic_conv, "%ld",number_assoc_recs);
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,2,ebufp);

        /*
         * NETWORK ELEMENT START DATE/TIME STAMP - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * NETWORK ELEMENT END DATE/TIME STAMP - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * NETWORK ELEMENT START DATE/UTC TIME OFFSET - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * NETWORK ELEMENT END DATE/UTC TIME OFFSET - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);


        /*
         * UTC END TIME OFFSET - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * INCOMING ROUTE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);


        /*
         * ROUTING CATEGORY - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * INTERN PROCESS STATUS - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * DROPPED CALL QUANTITY - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)integrate_not_used,1,ebufp);

        /*
         * DROPPED CALL STATUS - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)integrate_not_used,1,ebufp);

        /*
         * End of record (line) indicator
         */
	addStr(edr, EOL_DELIMITER, ebufp);

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, (char *)edr->data);

        return;
        /*****/

PROCESS_FILE_ERROR:

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, 
            (char *)"*** MISSING PDC INFO FIELDS - "
            "UNABLE TO PROCESS ***", 
            pdc_flistp);

        return;
        /*****/

ErrNullPointer:
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "processSessionBasicDetailPdc error", ebufp);
                return;
                /*****/
        }
}

/****************************************************************************
 * Function     : processSessionBasicDetailGsm
 * Description  : Builds the report record for a basic detail record
 *--------------+------------------------------------------------------------
 * In           : pin_buf_t	*edr
 * In           : pin_flist_t	*gsm_flistp
 * In           : int32 number_assoc_recs
 * In           : const char      *record_type
 * In		: pin_errbuf_t	*ebufp
 *--------------+------------------------------------------------------------
 * Returns      : void
 ****************************************************************************/
static void
processSessionBasicDetailGsm(
	pin_buf_t	*edr,
	pin_flist_t *gsm_flistp,
        int32 number_assoc_recs,
        const char *record_type,
	pin_errbuf_t	*ebufp)
{
        pin_flist_t	*gsm_info_flistp = NULL;
        pin_flist_t	*telco_info_flistp = NULL;
        pin_flist_t	*bal_impact_flistp = NULL;
                
        int32           record_id = 0;
        pin_cookie_t	cookie_id = NULL;
                
        char            generic_conv[256];
        int32           *generic_int = NULL;
        char            *generic_char = NULL;
        time_t          *generic_time = NULL;
        time_t          *generic_time2 = NULL;
        pin_decimal_t   *generic_decimal= NULL;

        int32           num_service_codes = 0;
        int32           num_impact_packets = 0;
        int32           num_mon_impact_packets = 0;
        char            integrate_not_used[2] = "0";
        
        
        if (PIN_ERR_IS_ERR(ebufp)) {
		return;
                /*****/
	}

        gsm_info_flistp = PIN_FLIST_SUBSTR_GET(gsm_flistp, PIN_FLD_GSM_INFO, 
			1, ebufp);
        telco_info_flistp = PIN_FLIST_SUBSTR_GET(gsm_flistp, 
			PIN_FLD_TELCO_INFO, 1, ebufp);

	if (!gsm_info_flistp || !telco_info_flistp) {

              goto PROCESS_FILE_ERROR;
              /**********************/
        }

        record_id = 0;
        cookie_id = NULL;

        bal_impact_flistp = PIN_FLIST_ELEM_GET_NEXT(gsm_flistp,
            PIN_FLD_BAL_IMPACTS, &record_id, 1, &cookie_id, ebufp);


	if (bal_impact_flistp == (pin_flist_t *)NULL) {

              goto PROCESS_FILE_ERROR;
              /**********************/
        }

        /*
         * Construct the flist in the correct order
         * Spec: sol42 CDR/EDR/IDR Format 6.3
         */
        
        /*
         * RECORD TYPE 
         */
        formatFieldsForFile(edr, 
		(char *)FILE_FLD_FORMAT_9,(char *)record_type,3, ebufp);
        
        /*
         * RECORD NUMBER 
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)RECORD_PLACE_HOLDER,9,ebufp);
        
        /**
         * RA Changes - Start
        **/

        generic_char = (char *)PIN_FLIST_FLD_GET(gsm_flistp,
           PIN_FLD_EVENT_NO, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr, 
		(char *)FILE_FLD_FORMAT_9,(char *)generic_char,60, ebufp);

        addStr(edr, DELIMITER, ebufp);

        generic_char = (char *)PIN_FLIST_FLD_GET(gsm_flistp,
           PIN_FLD_ORIGINAL_BATCH_ID, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr, 
		(char *)FILE_FLD_FORMAT_9,(char *)generic_char,80, ebufp);

        /**
         * RA Changes - End
        **/

        /*
         * DISCARDING - DEFAULT OF 0
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,1,ebufp);
        
        /*
         * CHAIN REFERENCE 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_NETWORK_SESSION_ID, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_NETWORK_SESSION_ID, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv, "%ld", *generic_int);
      
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,generic_conv,10,ebufp);

        /*
         * SOURCE NETWORK TYPE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * SOURCE NETWORK 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_ORIGIN_NETWORK, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr, 
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,14,ebufp);

        /*
         * DESTINATION NETWORK TYPE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        
        /*
         * DESTINATION NETWORK 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_DESTINATION_NETWORK, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,14,ebufp);

        /*
         * TYPE OF A-IDENTIFICATION - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * A-MODIFICATION INDICATOR - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * A-TYPE OF NUMBER - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)integrate_not_used,1,ebufp);

        /*
         * A-NUMBERING PLAN - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * A-NUMBER 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_PRIMARY_MSID, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,40,ebufp);

        /*
         * B-MODIFICATION INDICATOR 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gsm_info_flistp,
           PIN_FLD_CALLED_NUM_MODIF_MARK, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_CALLED_NUM_MODIF_MARK, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv, "%ld",*generic_int);
        
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_H,(char *)generic_conv,2,ebufp);

        /*
         * B-TYPE OF NUMBER - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)integrate_not_used,1,ebufp);

        /*
         * B-NUMBERING PLAN - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * B-NUMBER 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_CALLED_TO, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,40,ebufp);

        /*
         * DESCRIPTION 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gsm_flistp,
           PIN_FLD_DESCR, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,50,ebufp);

        /*
         * C-MODIFICATION INDICATOR - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * C-TYPE OF NUMBER - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)integrate_not_used,1,ebufp);

        /*
         * C-NUMBERING PLAN - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * C-NUMBER - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * USAGE DIRECTION 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gsm_info_flistp,
           PIN_FLD_DIRECTION, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_DIRECTION, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv, "%ld",*generic_int);
        
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,1,ebufp);

        /*
         * CONNECT TYPE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * CONNECT SUB TYPE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * BASIC SERVICE 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_SVC_TYPE, 0, ebufp);
        if ( !generic_char ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_SVC_TYPE, 0, 0);
                goto ErrNullPointer;
        }
        strcpy(generic_conv, generic_char);
        generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_SVC_CODE, 0, ebufp);
        if ( !generic_char ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_SVC_CODE, 0, 0);
                goto ErrNullPointer;
        }
        strcat(generic_conv, generic_char);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,3,ebufp);
        
        /*
         * QUALITY OF SERVICE REQUESTED 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gsm_info_flistp,
           PIN_FLD_QOS_REQUESTED, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_QOS_REQUESTED, 0, 0);
                goto ErrNullPointer;
        }
        
        sprintf(generic_conv, "%ld",*generic_int);
        
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr, 
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,5, ebufp);

        /*
         * QUALITY OF SERVICE USED 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gsm_info_flistp,
           PIN_FLD_QOS_NEGOTIATED, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_QOS_NEGOTIATED, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv, "%ld",*generic_int);
        
       addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr, 
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,5, ebufp);
        
        /*
         * CALL COMPLETION INDICATOR 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_TERMINATE_CAUSE, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_TERMINATE_CAUSE, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv, "%ld",*generic_int);
     
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr, 
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,3, ebufp);

        /*
         * LONG DURATION INDICATOR 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gsm_info_flistp,
           PIN_FLD_SUB_TRANS_ID, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,1,ebufp);

        /*
         *  CHARGING START TIMESTAMP
         */
        generic_time = (time_t *)PIN_FLIST_FLD_GET(gsm_flistp,
           PIN_FLD_START_T, 0, ebufp);
        if ( !generic_time ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_START_T, 0, 0);
                goto ErrNullPointer;
        }
       
        sprintf(generic_conv, "%ld",*generic_time);
      
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)generic_conv,
            strlen(generic_conv),ebufp);

        /*
         *  CHARGING END TIMESTAMP
         */
        generic_time = (time_t *)PIN_FLIST_FLD_GET(gsm_flistp,
           PIN_FLD_END_T, 0, ebufp);
        if ( !generic_time ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_END_T, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv, "%ld",*generic_time);

        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)generic_conv,
            strlen(generic_conv),ebufp);

        /*
         *  CREATED TIMESTAMP
         */
        generic_time = (time_t *)PIN_FLIST_FLD_GET(gsm_flistp,
           PIN_FLD_CREATED_T, 0, ebufp);
        if ( !generic_time ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_CREATED_T, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv, "%ld",*generic_time);

        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)generic_conv,
            strlen(generic_conv),ebufp);

        /*
         * UTC TIME OFFSET 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gsm_flistp,
           PIN_FLD_TIMEZONE_ID, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,5,ebufp);

        /*
         *  DURATION
         */
        addStr(edr, DELIMITER, ebufp);
        generic_time  = (time_t *)PIN_FLIST_FLD_GET(gsm_flistp, 
                                                    PIN_FLD_START_T, 0, ebufp);
        if ( !generic_time ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_START_T, 0, 0);
                goto ErrNullPointer;
        }
        generic_time2 = (time_t *)PIN_FLIST_FLD_GET(gsm_flistp, 
                                                    PIN_FLD_END_T, 0, ebufp);
        if ( !generic_time2 ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_END_T, 0, 0);
                goto ErrNullPointer;
        }

        /* If end_t is greater or equal to start_t, 
         * then we have a valid scenario for calculating duration
         */
        if ((generic_time2 != NULL) && (generic_time != NULL) && 
            (*generic_time2 >= *generic_time))
        {
          int32 duration= *generic_time2 - *generic_time;
          sprintf(generic_conv,"%ld",duration);

        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,15,ebufp);
        }
        else
        {
          formatFieldsForFile(edr, (char *)FILE_FLD_FORMAT_9,
                             (char *)integrate_not_used,15,ebufp);
        }

        /*
         *  DURATION UoM
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)FIELD_DURATION_UOM,3, ebufp);

        /*
         * VOLUME SENT 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gsm_info_flistp,
           PIN_FLD_BYTES_OUT, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_BYTES_OUT, 0, 0);
                goto ErrNullPointer;
        }
  
        sprintf(generic_conv, "%ld",*generic_int);
    
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,15,ebufp);

        /*
         *  VOLUME SENT UOM
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)FIELD_VOLUME_SENT_UOM,3,ebufp);

        /*
         * VOLUME RECEIVED 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gsm_info_flistp,
           PIN_FLD_BYTES_IN, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_BYTES_IN, 0, 0);
                goto ErrNullPointer;
        }

        sprintf(generic_conv, "%ld",*generic_int);
   
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,15,ebufp);

        /*
         *  VOLUME RECEIVED UOM
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)FIELD_VOLUME_RECEIVED_UOM,
		3,ebufp);

        /*
         * NUMBER OF UNITS 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gsm_info_flistp,
           PIN_FLD_NUMBER_OF_UNITS, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_NUMBER_OF_UNITS, 0, 0);
                goto ErrNullPointer;
        }
 
        sprintf(generic_conv, "%ld",*generic_int);

	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,15,ebufp);

        /*
         *  NUMBER OF UNITS UoM
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)FIELD_NUMBER_OF_UNITS_UOM,
		3,ebufp);

        /*
         * RETAIL IMPACT CATEGORY - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * RETAIL CHARGED AMOUNT VALUE - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,11,ebufp);

        /*
         * RETAIL CHARGED AMOUNT CURRENCY - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * RETAIL CHARGED TAX TREATMENT - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * RETAIL CHARGED TAX RATE - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,4,ebufp);

        /*
         * WHOLESALE IMPACT CATEGORY - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * WHOLESALE CHARGED AMOUNT VALUE - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,11,ebufp);

        /*
         * WHOLESALE CHARGED AMOUNT CURRENCY - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * WHOLESALE CHARGED TAX TREATMENT - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * WHOLESALE CHARGED TAX RATE - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,4,ebufp);

        /*
         * ZONE DESCRIPTION 
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * IC DESCRIPTION 
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * ZONE ENTRY NAME
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * TARIFF CLASS - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * TARIFF SUB CLASS - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * USAGE CLASS 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_USAGE_CLASS, 0, ebufp);
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,5,ebufp);

        /*
         * USAGE TYPE 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_USAGE_TYPE, 0, ebufp);
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,5,ebufp);

        /*
         * BILLCYCLE PERIOD - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * PREPAID INDICATOR - DEFAULT
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,2,ebufp);

        /*
         * NUMBER ASSOCIATED RECORDS
         */
        num_service_codes = PIN_FLIST_ELEM_COUNT(gsm_flistp, 
            PIN_FLD_SERVICE_CODES, ebufp);
        number_assoc_recs += num_service_codes;
        num_impact_packets = PIN_FLIST_ELEM_COUNT(gsm_flistp, 
            PIN_FLD_BAL_IMPACTS, ebufp);
        number_assoc_recs += num_impact_packets;
        num_mon_impact_packets = PIN_FLIST_ELEM_COUNT(gsm_flistp, 
            PIN_FLD_MONITOR_IMPACTS, ebufp);
        number_assoc_recs += num_mon_impact_packets;
        sprintf(generic_conv, "%ld",number_assoc_recs);
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,2,ebufp);

        /*
         * NETWORK ELEMENT START DATE/TIME STAMP - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * NETWORK ELEMENT END DATE/TIME STAMP - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * NETWORK ELEMENT START DATE/UTC TIME OFFSET - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * NETWORK ELEMENT END DATE/UTC TIME OFFSET - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);


        /*
         * UTC END TIME OFFSET - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * INCOMING ROUTE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);


        /*
         * ROUTING CATEGORY - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * INTERN PROCESS STATUS - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * DROPPED CALL QUANTITY - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)integrate_not_used,1,ebufp);

        /*
         * DROPPED CALL STATUS - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)integrate_not_used,1,ebufp); 

        /*
         * End of record (line) indicator
         */
	addStr(edr, EOL_DELIMITER, ebufp);

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, (char *)edr->data);

        return;
        /*****/

PROCESS_FILE_ERROR:

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, 
            (char *)"*** MISSING GSM INFO FIELDS - "
            "UNABLE TO PROCESS ***", 
            gsm_flistp);

        return;
        /*****/
ErrNullPointer:
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "processSessionBasicDetailGsm error", ebufp);
                return;
                /*****/
        }
}


/****************************************************************************
 * Function     : processSessionBasicDetailGprs
 * Description  : Builds the report record for a basic detail record
 *--------------+------------------------------------------------------------
 * In           : pin_buf_t	*edr
 * In           : pin_flist_t	*gprs_flistp
 * In           : int32 number_assoc_recs
 * In           : const char *record_type
 * In		: pin_errbuf_t	*ebufp
 *--------------+------------------------------------------------------------
 * Returns      : void
 ****************************************************************************/
static void
processSessionBasicDetailGprs(
	pin_buf_t	*edr,
	pin_flist_t *gprs_flistp,
        int32   number_assoc_recs,
        const char *record_type,
	pin_errbuf_t	*ebufp)
{
        pin_flist_t	*gprs_info_flistp = NULL;
        pin_flist_t	*bal_impact_flistp = NULL;
                
        int32           record_id = 0;
        pin_cookie_t	cookie_id = NULL;
                
        char          generic_conv[256];
        int32           *generic_int = NULL;
        char            *generic_char = NULL;
        time_t          *generic_time = NULL;
        time_t          *generic_time2 = NULL;
        pin_decimal_t   *generic_decimal= NULL;

        int32           num_impact_packets = 0;
        int32           num_mon_impact_packets = 0;
        char            integrate_not_used[2] = "0";
        
        if (PIN_ERR_IS_ERR(ebufp)) {
		return;
                /*****/
	}

        gprs_info_flistp = PIN_FLIST_ELEM_GET_NEXT(gprs_flistp,
            PIN_FLD_GPRS_INFO, &record_id, 1, &cookie_id, ebufp);

	if (gprs_info_flistp == (pin_flist_t *)NULL) {

              goto PROCESS_FILE_ERROR;
              /**********************/
        }

        record_id = 0;
        cookie_id = NULL;

        bal_impact_flistp = PIN_FLIST_ELEM_GET_NEXT(gprs_flistp,
            PIN_FLD_BAL_IMPACTS, &record_id, 1, &cookie_id, ebufp);

	if (bal_impact_flistp == (pin_flist_t *)NULL) {

              goto PROCESS_FILE_ERROR;
              /**********************/
        }
	
        /*
         * Construct the string in the correct order
         * Spec: sol42 CDR/EDR/IDR Format 6.3
         */
        
        /*
         * RECORD TYPE 
         */
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)record_type,3,ebufp);

        /*
         * RECORD NUMBER 
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)RECORD_PLACE_HOLDER,9,ebufp);
        
        /**
         * RA Changes - Start
        **/

        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_flistp,
           PIN_FLD_EVENT_NO, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr, 
		(char *)FILE_FLD_FORMAT_9,(char *)generic_char,60, ebufp);

        addStr(edr, DELIMITER, ebufp);

        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_flistp,
           PIN_FLD_ORIGINAL_BATCH_ID, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr, 
		(char *)FILE_FLD_FORMAT_9,(char *)generic_char,80, ebufp);

        /**
         * RA Changes - End
        **/

        /*
         * DISCARDING - DEFAULT OF 0
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,1,ebufp);
        
        /*
         * CHAIN REFERENCE - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * SOURCE NETWORK TYPE - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * SOURCE NETWORK - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * DESTINATION NETWORK TYPE - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * DESTINATION NETWORK - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * TYPE OF A-IDENTIFICATION - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * A-MODIFICATION INDICATOR - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * A-TYPE OF NUMBER - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)integrate_not_used,1,ebufp);

        /*
         * A-NUMBERING PLAN - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * A-NUMBER 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_MSISDN, 0, ebufp);
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,40,ebufp);

        /*
         * B-MODIFICATION INDICATOR - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * B-TYPE OF NUMBER - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)integrate_not_used,1,ebufp);

        /*
         * B-NUMBERING PLAN - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * B-NUMBER - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * DESCRIPTION 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_flistp,
           PIN_FLD_DESCR, 0, ebufp);
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,50,ebufp);

        /*
         * C-MODIFICATION INDICATOR - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * C-TYPE OF NUMBER - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)integrate_not_used,1,ebufp);

        /*
         * C-NUMBERING PLAN - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * C-NUMBER - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * USAGE DIRECTION - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * CONNECT TYPE 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_ANONYMOUS_LOGIN, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_ANONYMOUS_LOGIN, 0, 0);
                goto ErrNullPointer;
        }
 
        sprintf(generic_conv,"%ld",*generic_int);
 
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,2,ebufp);

        /*
         * CONNECT SUB TYPE - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * BASIC SERVICE 
         */
	addStr(edr, DELIMITER, ebufp);
	formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)"GPR",3,ebufp);

        
        /*
         * QUALITY OF SERVICE REQUESTED - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * QUALITY OF SERVICE USED - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * CALL COMPLETION INDICATOR 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_CLOSE_CAUSE, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_CLOSE_CAUSE, 0, 0);
                goto ErrNullPointer;
        }

        sprintf(generic_conv, "%ld",*generic_int);

	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,3,ebufp);

        /*
         * LONG DURATION INDICATOR - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         *  CHARGING START TIMESTAMP
         */
        generic_time = (time_t *)PIN_FLIST_FLD_GET(gprs_flistp,
           PIN_FLD_START_T, 0, ebufp);
        if ( !generic_time ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_START_T, 0, 0);
                goto ErrNullPointer;
        }
  
        sprintf(generic_conv,"%ld",*generic_time);
 
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)generic_conv,
            strlen(generic_conv),ebufp);
        
        /*
         *  TRACK THE EFFECT ON THE TRAILER RECORD
         */
        /*analyzeChargingStart(*generic_time);
	*/

        /*
         *  CHARGING END TIMESTAMP
         */
        generic_time = (time_t *)PIN_FLIST_FLD_GET(gprs_flistp,
           PIN_FLD_END_T, 0, ebufp);
        if ( !generic_time ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_END_T, 0, 0);
                goto ErrNullPointer;
        }
 
        sprintf(generic_conv,"%ld",*generic_time);
 
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)generic_conv,
            strlen(generic_conv),ebufp);

        /*
         *  CREATED TIMESTAMP
         */
        generic_time = (time_t *)PIN_FLIST_FLD_GET(gprs_flistp,
           PIN_FLD_CREATED_T, 0, ebufp);
        if ( !generic_time ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_CREATED_T, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_time);

	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)generic_conv,
            strlen(generic_conv),ebufp);

        /*
         * UTC TIME OFFSET 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_flistp,
           PIN_FLD_TIMEZONE_ID, 0, ebufp);
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,5,ebufp);

        /*
         *  DURATION
         */
        addStr(edr, DELIMITER, ebufp);
        generic_time  = (time_t *)PIN_FLIST_FLD_GET(gprs_flistp, 
                                                    PIN_FLD_START_T, 0, ebufp);
        if ( !generic_time ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_START_T, 0, 0);
                goto ErrNullPointer;
        }
        generic_time2 = (time_t *)PIN_FLIST_FLD_GET(gprs_flistp, 
                                                    PIN_FLD_END_T, 0, ebufp);
        if ( !generic_time2 ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_END_T, 0, 0);
                goto ErrNullPointer;
        }

        /* If end_t is greater or equal to start_t, 
         * then we have a valid scenario for calculating duration
         */
        if ((generic_time2 != NULL) && (generic_time != NULL) && 
				(*generic_time2 >= *generic_time))
        {
          int32 duration= *generic_time2 - *generic_time;
          sprintf(generic_conv,"%ld",duration);

        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,15,ebufp);
        }
        else
        {
          formatFieldsForFile(edr, (char *)FILE_FLD_FORMAT_9,
                             (char *)integrate_not_used,15,ebufp);
        }

        /*
         *  DURATION UoM
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)FIELD_DURATION_UOM,3,ebufp);
        
        /*
         * VOLUME SENT 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_BYTES_OUT, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_BYTES_OUT, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
 
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,15,ebufp);

        /*
         *  VOLUME SENT UOM
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)FIELD_VOLUME_SENT_UOM,3,ebufp);

        /*
         * VOLUME RECEIVED 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_BYTES_IN, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_BYTES_IN, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,15,ebufp);

        /*
         *  VOLUME RECEIVED UOM
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)FIELD_VOLUME_RECEIVED_UOM,3,
		ebufp);

        /*
         * NUMBER OF UNITS 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_NUMBER_OF_UNITS, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_NUMBER_OF_UNITS, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,15,ebufp);

        /*
         *  NUMBER OF UNITS UoM
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)FIELD_NUMBER_OF_UNITS_UOM,
		3,ebufp);

        /*
         * RETAIL IMPACT CATEGORY - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * RETAIL CHARGED AMOUNT VALUE - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,11,ebufp);

        /*
         * RETAIL CHARGED AMOUNT CURRENCY - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * RETAIL CHARGED TAX TREATMENT - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * RETAIL CHARGED TAX RATE - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,4,ebufp);

        /*
         * WHOLESALE IMPACT CATEGORY - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * WHOLESALE CHARGED AMOUNT VALUE - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,11,ebufp);

        /*
         * WHOLESALE CHARGED AMOUNT CURRENCY - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * WHOLESALE CHARGED TAX TREATMENT - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * WHOLESALE CHARGED TAX RATE - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,4,ebufp);

        /*
         * ZONE DESCRIPTION 
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * IC DESCRIPTION 
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * ZONE ENTRY NAME
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * TARIFF CLASS - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * TARIFF SUB CLASS - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * USAGE CLASS 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_USAGE_CLASS, 0, ebufp);
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,5,ebufp);

        /*
         * USAGE TYPE 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_USAGE_TYPE, 0, ebufp);
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,5,ebufp);

        /*
         * BILLCYCLE PERIOD - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * PREPAID INDICATOR - DEFAULT
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,2,ebufp);

        /*
         * NUMBER ASSOCIATED RECORDS
         */
        num_impact_packets = PIN_FLIST_ELEM_COUNT(gprs_flistp, 
            PIN_FLD_BAL_IMPACTS, ebufp);
        number_assoc_recs += num_impact_packets;
        num_mon_impact_packets = PIN_FLIST_ELEM_COUNT(gprs_flistp, 
            PIN_FLD_MONITOR_IMPACTS, ebufp);
        number_assoc_recs += num_mon_impact_packets;
        sprintf(generic_conv, "%ld",number_assoc_recs);
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,2,ebufp);

        /*
         * NETWORK ELEMENT START DATE/TIME STAMP - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * NETWORK ELEMENT END DATE/TIME STAMP - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * NETWORK ELEMENT START DATE/UTC TIME OFFSET - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * NETWORK ELEMENT END DATE/UTC TIME OFFSET - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);


        /*
         * UTC END TIME OFFSET - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * INCOMING ROUTE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);


        /*
         * ROUTING CATEGORY - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * INTERN PROCESS STATUS - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * DROPPED CALL QUANTITY - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)integrate_not_used,1,ebufp);

        /*
         * DROPPED CALL STATUS - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)integrate_not_used,1,ebufp); 

        /*
         * End of record (line) indicator
         */
	addStr(edr, EOL_DELIMITER, ebufp);

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, (char *)edr->data);
	        
        /*
         * Clean up
         */

        return;
        /*****/

PROCESS_FILE_ERROR:

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, 
            (char *)"*** MISSING GPRS INFO FIELDS - "
            "UNABLE TO PROCESS ***", 
            gprs_flistp);

        /*
         * Clean up
         */
ErrNullPointer:
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "processSessionBasicDetailGprs error", ebufp);
                return;
                /*****/
        }
}

/****************************************************************************
 * Function     : processSessionBasicDetailWap
 * Description  : Builds the report record for a basic detail record
 *--------------+------------------------------------------------------------
 * In/Out       : pin_buf_t	*edr
 * In           : pin_flist_t	*wap_flistp
 * In           : int32 number_assoc_recs
 * In           : const char *record_type
 * In           : pin_errbuf_t *ebufp
 *--------------+------------------------------------------------------------
 * Returns      : void
 ****************************************************************************/
static void
processSessionBasicDetailWap(
	pin_buf_t   *edr,
	pin_flist_t   *wap_flistp,
        int32   number_assoc_recs,
        const char      *record_type,
	pin_errbuf_t	*ebufp)
{
        pin_flist_t	*wap_info_flistp = NULL;
        pin_flist_t	*bal_impact_flistp = NULL;
                
        int32           record_id = 0;
        pin_cookie_t	cookie_id = NULL;
        
        char          generic_conv[256];
        int32           *generic_int = NULL;
        char            *generic_char = NULL;
        time_t          *generic_time = NULL;
        pin_decimal_t   *generic_decimal= NULL;

        int32           num_impact_packets = 0;
        int32           num_mon_impact_packets = 0;
        char            integrate_not_used[2] = "0";
        
        
        if (PIN_ERR_IS_ERR(ebufp)) {
		return;
                /*****/
	}

        wap_info_flistp = PIN_FLIST_ELEM_GET_NEXT(wap_flistp,
            PIN_FLD_WAP_INFO, &record_id, 1, &cookie_id, ebufp);

	if (wap_info_flistp == (pin_flist_t *)NULL) {

              goto PROCESS_FILE_ERROR;
              /**********************/
        }

        record_id = 0;
        cookie_id = NULL;

        bal_impact_flistp = PIN_FLIST_ELEM_GET_NEXT(wap_flistp,
            PIN_FLD_BAL_IMPACTS, &record_id, 1, &cookie_id, ebufp);

	if (bal_impact_flistp == (pin_flist_t *)NULL) {

              goto PROCESS_FILE_ERROR;
              /**********************/
        }

        /*
         * Construct the flist in the correct order
         * Spec: sol42 CDR/EDR/IDR Format 6.3
         */
        
        /*
         * RECORD TYPE 
         */
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)record_type,3,ebufp);

        /*
         * RECORD NUMBER 
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)RECORD_PLACE_HOLDER,9,ebufp);

        /**
         * RA Changes - Start
        **/

        generic_char = (char *)PIN_FLIST_FLD_GET(wap_flistp,
           PIN_FLD_EVENT_NO, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr, 
		(char *)FILE_FLD_FORMAT_9,(char *)generic_char,60, ebufp);

        addStr(edr, DELIMITER, ebufp);

        generic_char = (char *)PIN_FLIST_FLD_GET(wap_flistp,
           PIN_FLD_ORIGINAL_BATCH_ID, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr, 
		(char *)FILE_FLD_FORMAT_9,(char *)generic_char,80, ebufp);

        /**
         * RA Changes - End
        **/

        /*
         * DISCARDING - DEFAULT OF 0
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,1,ebufp);
        
        /*
         * CHAIN REFERENCE - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * SOURCE NETWORK TYPE - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * SOURCE NETWORK - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * DESTINATION NETWORK TYPE - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * DESTINATION NETWORK - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * TYPE OF A-IDENTIFICATION - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * A-MODIFICATION INDICATOR - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * A-TYPE OF NUMBER - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)integrate_not_used,1,ebufp);

        /*
         * A-NUMBERING PLAN - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * A-NUMBER 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(wap_info_flistp,
           PIN_FLD_MSISDN, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,40,ebufp);

        /*
         * B-MODIFICATION INDICATOR - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * B-TYPE OF NUMBER - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)integrate_not_used,1,ebufp);

        /*
         * B-NUMBERING PLAN - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * B-NUMBER - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * DESCRIPTION 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(wap_flistp,
           PIN_FLD_DESCR, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,50,ebufp);

        /*
         * C-MODIFICATION INDICATOR - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * C-TYPE OF NUMBER - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)integrate_not_used,1,ebufp);

        /*
         * C-NUMBERING PLAN - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * C-NUMBER - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * USAGE DIRECTION - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * CONNECT TYPE 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(wap_info_flistp,
           PIN_FLD_CONN_TYPE, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_CONN_TYPE, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,2,ebufp);

        /*
         * CONNECT SUB TYPE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * BASIC SERVICE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)"WAP",3,ebufp);

        
        /*
         * QUALITY OF SERVICE REQUESTED - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * QUALITY OF SERVICE USED - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        
        /*
         * CALL COMPLETION INDICATOR - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * LONG DURATION INDICATOR - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         *  CHARGING START TIMESTAMP
         */
        generic_time = (time_t *)PIN_FLIST_FLD_GET(wap_flistp,
           PIN_FLD_START_T, 0, ebufp);
        if ( !generic_time ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_START_T , 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_time);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)generic_conv,
            strlen(generic_conv),ebufp);

        /*
         *  CHARGING END TIMESTAMP
         */
        generic_time = (time_t *)PIN_FLIST_FLD_GET(wap_flistp,
           PIN_FLD_END_T, 0, ebufp);
        if ( !generic_time ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_END_T, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_time);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)generic_conv,
            strlen(generic_conv),ebufp);

        /*
         *  CREATED TIMESTAMP
         */
        generic_time = (time_t *)PIN_FLIST_FLD_GET(wap_flistp,
           PIN_FLD_CREATED_T, 0, ebufp);
        if ( !generic_time ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_CREATED_T, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_time);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)generic_conv,
            strlen(generic_conv),ebufp);

        /*
         * UTC TIME OFFSET 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(wap_flistp,
           PIN_FLD_TIMEZONE_ID, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,5,ebufp);

        /*
         *  DURATION - NOT USED 
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,15,ebufp);

        /*
         *  DURATION UoM
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)FIELD_DURATION_UOM,3,ebufp);

        /*
         * VOLUME SENT - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         *  VOLUME SENT UOM - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * VOLUME RECEIVED 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(wap_info_flistp,
           PIN_FLD_BYTES_IN, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_BYTES_IN, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,15,ebufp);

        /*
         *  VOLUME RECEIVED UOM
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)FIELD_VOLUME_RECEIVED_UOM,
		3,ebufp);

        /*
         * NUMBER OF UNITS 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(wap_info_flistp,
           PIN_FLD_NUMBER_OF_UNITS, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_NUMBER_OF_UNITS, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,15,ebufp);

        /*
         *  NUMBER OF UNITS UoM
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)FIELD_NUMBER_OF_UNITS_UOM,
		3,ebufp);

        /*
         * RETAIL IMPACT CATEGORY - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * RETAIL CHARGED AMOUNT VALUE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,11,ebufp);
        
        /*
         * RETAIL CHARGED AMOUNT CURRENCY - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * RETAIL CHARGED TAX TREATMENT - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * RETAIL CHARGED TAX RATE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,4,ebufp);

        /*
         * WHOLESALE IMPACT CATEGORY - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * WHOLESALE CHARGED AMOUNT VALUE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,11,ebufp);

        /*
         * WHOLESALE CHARGED AMOUNT CURRENCY - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * WHOLESALE CHARGED TAX TREATMENT - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * WHOLESALE CHARGED TAX RATE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,4,ebufp);

        /*
         * ZONE DESCRIPTION 
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * IC DESCRIPTION 
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * ZONE ENTRY NAME
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * TARIFF CLASS - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * TARIFF SUB CLASS - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * USAGE CLASS 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(wap_info_flistp,
           PIN_FLD_USAGE_CLASS, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,5,ebufp);

        /*
         * USAGE TYPE 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(wap_info_flistp,
           PIN_FLD_USAGE_TYPE, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,5,ebufp);

        /*
         * BILLCYCLE PERIOD - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * PREPAID INDICATOR 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(wap_info_flistp,
           PIN_FLD_PREPAID_INDICATOR, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_char,2,ebufp);

        /*
         * NUMBER ASSOCIATED RECORDS
         */
        num_impact_packets = PIN_FLIST_ELEM_COUNT(wap_flistp, 
            PIN_FLD_BAL_IMPACTS, ebufp);
        number_assoc_recs += num_impact_packets;
        num_mon_impact_packets = PIN_FLIST_ELEM_COUNT(wap_flistp, 
            PIN_FLD_MONITOR_IMPACTS, ebufp);
        number_assoc_recs += num_mon_impact_packets;
        sprintf(generic_conv,"%ld",number_assoc_recs);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,2,ebufp);

        /*
         * NETWORK ELEMENT START DATE/TIME STAMP - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * NETWORK ELEMENT END DATE/TIME STAMP - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * NETWORK ELEMENT START DATE/UTC TIME OFFSET - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * NETWORK ELEMENT END DATE/UTC TIME OFFSET - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);


        /*
         * UTC END TIME OFFSET - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * INCOMING ROUTE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);


        /*
         * ROUTING CATEGORY - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        
        /*
         * INTERN PROCESS STATUS - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * DROPPED CALL QUANTITY - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)integrate_not_used,1,ebufp);
        
        /*
         * DROPPED CALL STATUS - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)integrate_not_used,1,ebufp); 

        /*
         * End of record (line) indicator
         */
        addStr(edr, EOL_DELIMITER, ebufp);

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, (char *)edr->data);

        return;
        /*****/

PROCESS_FILE_ERROR:

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, 
            (char *)"*** MISSING WAP INFO FIELDS - "
            "UNABLE TO PROCESS ***", 
            wap_flistp);

ErrNullPointer:
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "processSessionBasicDetailWap error", ebufp);
                return;
                /*****/
        }
}

/****************************************************************************
 * Function     : processSessionSupBalance
 * Description  : Builds the report record for every supplemental
 *              : Balance Impact Packet record
 *--------------+------------------------------------------------------------
 * In/Out       : pin_buf_t	*edr
 * In           : pin_flist_t	*flistp
 * In           : pin_errbuf_t *ebufp
 *--------------+------------------------------------------------------------
 * Returns      : void
 ****************************************************************************/
static void
processSessionSupBalance(
	pin_buf_t	*edr,
	pin_flist_t	*flistp,
	pin_errbuf_t	*ebufp)
{
        pin_flist_t     *event_packet_flistp = NULL;
        
        int32           record_id = 0;
        pin_cookie_t	cookie_id = NULL;
        
        char            generic_conv[256];
        int32           *generic_int = NULL;
        char            *generic_char = NULL;
        time_t          *generic_time = NULL;
        pin_decimal_t   *generic_decimal = NULL;
        poid_t          *generic_poid = NULL;

        int32	        maxpoidlen = 0;
	char	        poid_string[100];
        int32           generic_rerate_int = 0 ;
        char            integrate_not_used[2] = "0";
        
        if (PIN_ERR_IS_ERR(ebufp)) {
		return;
                /*****/
	}

        /*
         * Construct the flist in the correct order
         * Spec: sol42 CDR/EDR/IDR Format 6.3
         */

        while ((event_packet_flistp = PIN_FLIST_ELEM_GET_NEXT(flistp,
	            PIN_FLD_BAL_IMPACTS, &record_id, 1, &cookie_id,
	            ebufp)) != (pin_flist_t *)NULL) {
        
                /*
                 * RECORD TYPE 
                 */
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_9,(char *)RECORD_TYPE_SUP_BAL,
			3,ebufp);

                /*
                 * RECORD NUMBER 
                 */
                addStr(edr, DELIMITER, ebufp);
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_9,(char *)RECORD_PLACE_HOLDER,
			9, ebufp);
                
                /*
                 * PIN ACCOUNT POID 
                 */
                generic_poid = (char *)PIN_FLIST_FLD_GET(
                    event_packet_flistp,PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
                if ( !generic_poid ) {
                        pin_set_err(ebufp, PIN_ERRLOC_APP,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_NULL_PTR, PIN_FLD_ACCOUNT_OBJ, 0, 0);
                        goto ErrNullPointer;
                }
                memset(poid_string,0,sizeof(poid_string));
                generic_char = poid_string;
                maxpoidlen = sizeof(poid_string);
                PIN_POID_TO_STR((poid_t *)generic_poid, &generic_char, 
                    &maxpoidlen, ebufp);
                addStr(edr, DELIMITER, ebufp);
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_X,(char *)generic_char+6,255,ebufp);

                /*
                 * PIN BAL_GRP POID 
                 */
                generic_poid = (char *)PIN_FLIST_FLD_GET(
                    event_packet_flistp,PIN_FLD_BAL_GRP_OBJ, 0, ebufp);
                if ( !generic_poid ) {
                        pin_set_err(ebufp, PIN_ERRLOC_APP,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_NULL_PTR, PIN_FLD_BAL_GRP_OBJ, 0, 0);
                        goto ErrNullPointer;
                }
                memset(poid_string,0,sizeof(poid_string));
                generic_char = poid_string;
                maxpoidlen = sizeof(poid_string);
                PIN_POID_TO_STR((poid_t *)generic_poid, &generic_char, 
                    &maxpoidlen, ebufp);
                addStr(edr, DELIMITER, ebufp);
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_X,(char *)generic_char+6,255,ebufp);

                /*
                 * PIN ITEM POID 
                 */
                generic_poid = (char *)PIN_FLIST_FLD_GET(
                    event_packet_flistp,PIN_FLD_ITEM_OBJ, 0, ebufp);
                if ( !generic_poid ) {
                        pin_set_err(ebufp, PIN_ERRLOC_APP,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_NULL_PTR, PIN_FLD_ITEM_OBJ, 0, 0);
                        goto ErrNullPointer;
                }
                memset(poid_string,0,sizeof(poid_string));
                generic_char = poid_string;
                maxpoidlen = sizeof(poid_string);
                PIN_POID_TO_STR((poid_t *)generic_poid, &generic_char, 
                    &maxpoidlen, ebufp);
                addStr(edr, DELIMITER, ebufp);
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_X,(char *)generic_char+6,255,ebufp);

                /*
                 * PIN RESOURCE ID 
                 */             
                generic_int = (int32 *)PIN_FLIST_FLD_GET(
                    event_packet_flistp,
                    PIN_FLD_RESOURCE_ID, 0, ebufp);
                if ( !generic_int ) {
                        pin_set_err(ebufp, PIN_ERRLOC_APP,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_NULL_PTR, PIN_FLD_RESOURCE_ID, 0, 0);
                        goto ErrNullPointer;
                }
                sprintf(generic_conv,"%ld",*generic_int);
                addStr(edr, DELIMITER, ebufp);
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,9,ebufp);

                /*
                 * PIN RESOURCE ID ORIG
                 */
                generic_int = (int32 *)PIN_FLIST_FLD_GET(
                    event_packet_flistp,
                    PIN_FLD_RESOURCE_ID_ORIG, 0, ebufp);
                if ( !generic_int ) {
                        pin_set_err(ebufp, PIN_ERRLOC_APP,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_NULL_PTR, PIN_FLD_RESOURCE_ID_ORIG, 0, 0);
                        goto ErrNullPointer;
                }
                sprintf(generic_conv,"%ld",*generic_int);
                addStr(edr, DELIMITER, ebufp);
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,9,ebufp);

                                        
                /*
                 * PIN IMPACT CATEGORY 
                 */
                generic_char = (char *)PIN_FLIST_FLD_GET(
                    event_packet_flistp,PIN_FLD_IMPACT_CATEGORY, 0,
                    ebufp);
                addStr(edr, DELIMITER, ebufp);
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_X,(char *)generic_char,255,ebufp);

                /*
                 * PIN IMPACT TYPE 
                 * RERATED event should always have 
                 * Impact type as rerated
                 */
                generic_rerate_int = ( int32 ) RERATE_IMPACT_TYPE ;
                sprintf(generic_conv,"%ld",generic_rerate_int);
                addStr(edr, DELIMITER, ebufp);
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,9,ebufp);

                /*
                 * PIN PRODUCT POID 
                 */
                generic_poid = (char *)PIN_FLIST_FLD_GET(
                    event_packet_flistp,PIN_FLD_PRODUCT_OBJ, 0, ebufp);
                if ( !generic_poid ) {
                        pin_set_err(ebufp, PIN_ERRLOC_APP,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_NULL_PTR, PIN_FLD_PRODUCT_OBJ, 0, 0);
                        goto ErrNullPointer;
                }
                memset(poid_string,0,sizeof(poid_string));
                generic_char = poid_string;
                maxpoidlen = sizeof(poid_string);
                PIN_POID_TO_STR((poid_t *)generic_poid, &generic_char, 
                    &maxpoidlen, ebufp);
                addStr(edr, DELIMITER, ebufp);
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_X,(char *)generic_char+6,
			255,ebufp);

                /*
                 * PIN GL ID 
                 */             
                generic_int = (int32 *)PIN_FLIST_FLD_GET(
                    event_packet_flistp,
                    PIN_FLD_GL_ID, 0, ebufp);
                if ( !generic_int ) {
                        pin_set_err(ebufp, PIN_ERRLOC_APP,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_NULL_PTR, PIN_FLD_GL_ID, 0, 0);
                        goto ErrNullPointer;
                }
                sprintf(generic_conv,"%ld",*generic_int);
                addStr(edr, DELIMITER, ebufp);
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,
			9,ebufp);
		
		/*
                 * PIN RUM_ID
                 */
                generic_int = (int32 *)PIN_FLIST_FLD_GET(
                    event_packet_flistp,
                    PIN_FLD_RUM_ID, 0, ebufp);
                if ( !generic_int ) {
                        pin_set_err(ebufp, PIN_ERRLOC_APP,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_NULL_PTR, PIN_FLD_RUM_ID, 0, 0);
                        goto ErrNullPointer;
                }
                sprintf(generic_conv,"%ld",*generic_int);
                addStr(edr, DELIMITER, ebufp);
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,
                        9,ebufp);

                /*
                 * PIN TAX CODE   
                 */
                generic_char = (char *)PIN_FLIST_FLD_GET(
                    event_packet_flistp,PIN_FLD_TAX_CODE, 0,
                    ebufp);
                addStr(edr, DELIMITER, ebufp);
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_X,(char *)generic_char,
			255,ebufp);

                /*
                 * PIN RATE TAG   
                 */
                generic_char = (char *)PIN_FLIST_FLD_GET(
                    event_packet_flistp,PIN_FLD_RATE_TAG, 0,
                    ebufp);
                addStr(edr, DELIMITER, ebufp);
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_X,(char *)generic_char,
			255,ebufp);

                /*
                 * PIN LINEAGE   
                 */
                generic_char = (char *)PIN_FLIST_FLD_GET(
                   event_packet_flistp,PIN_FLD_LINEAGE, 0,
                   ebufp);
                addStr(edr, DELIMITER, ebufp);
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_X,(char *)generic_char,
			255,ebufp);

                /*
                 * PIN OFFERING OBJ
                 */
                generic_poid = (char *)PIN_FLIST_FLD_GET(
                    event_packet_flistp,PIN_FLD_OFFERING_OBJ, 0, ebufp);
                if ( !generic_poid ) {
                        pin_set_err(ebufp, PIN_ERRLOC_APP,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_NULL_PTR, PIN_FLD_OFFERING_OBJ, 0, 0);
                        goto ErrNullPointer;
                }
                memset(poid_string,0,sizeof(poid_string));
                generic_char = poid_string;
                maxpoidlen = sizeof(poid_string);
                PIN_POID_TO_STR((poid_t *)generic_poid, &generic_char, 
                    &maxpoidlen, ebufp);
                addStr(edr, DELIMITER, ebufp);
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_X,(char *)generic_char+6,255,ebufp);

                /*
                 * PIN QUANTITY
                 */
                generic_decimal = (pin_decimal_t *)PIN_FLIST_FLD_GET(
                    event_packet_flistp,PIN_FLD_QUANTITY, 0, ebufp);
		if(generic_decimal) {
                  generic_char = pbo_decimal_to_str(generic_decimal, ebufp);
                  addStr(edr, DELIMITER, ebufp);
                  formatFieldsForFile(edr,
                      (char *)FILE_FLD_FORMAT_9,(char *)generic_char,15,ebufp);
  
                  if (generic_char) {
                          pin_free(generic_char);
                  }
    		}
        	else
        	{
            	  addStr(edr, DELIMITER, ebufp);
          	  formatFieldsForFile(edr, (char *)FILE_FLD_FORMAT_9,
                               	(char *)integrate_not_used,15,ebufp);
        	}

                /*
                 * PIN AMOUNT
                 */
                generic_decimal = (pin_decimal_t *)PIN_FLIST_FLD_GET(
                    event_packet_flistp,PIN_FLD_AMOUNT, 0, ebufp);
		if(generic_decimal) {
                  generic_char = pbo_decimal_to_str(generic_decimal, ebufp);
                  addStr(edr, DELIMITER, ebufp);
                  formatFieldsForFile(edr,
                      (char *)FILE_FLD_FORMAT_9,(char *)generic_char,11,ebufp);
  
                  if (generic_char) {
                          pin_free(generic_char);
                  }
                }
                else
                {
                  addStr(edr, DELIMITER, ebufp);
                  formatFieldsForFile(edr, (char *)FILE_FLD_FORMAT_9,
                                (char *)integrate_not_used,11,ebufp);
                }

                /*
                 * PIN AMOUNT ORIG
                 */
                generic_decimal = (pin_decimal_t *)PIN_FLIST_FLD_GET(
                    event_packet_flistp,PIN_FLD_AMOUNT_ORIG, 0, ebufp);
                if(generic_decimal) {
                  generic_char = pbo_decimal_to_str(generic_decimal, ebufp);
                  addStr(edr, DELIMITER, ebufp);
                  formatFieldsForFile(edr,
                      (char *)FILE_FLD_FORMAT_9,(char *)generic_char,11,ebufp);

                  if (generic_char) {
                          pin_free(generic_char);
                  }
                }
                else
                {
                  addStr(edr, DELIMITER, ebufp);
                  formatFieldsForFile(edr, (char *)FILE_FLD_FORMAT_9,
                                (char *)integrate_not_used,11,ebufp);
                }
 
                /*
                 * PIN AMOUNT DEFERRED
                 */
                generic_decimal = (pin_decimal_t *)PIN_FLIST_FLD_GET(
                    event_packet_flistp,PIN_FLD_AMOUNT_DEFERRED, 0, ebufp);
		if(generic_decimal) {
                  generic_char = pbo_decimal_to_str(generic_decimal, ebufp);
                  addStr(edr, DELIMITER, ebufp);
                  formatFieldsForFile(edr,
                      (char *)FILE_FLD_FORMAT_9,(char *)generic_char,11,ebufp);
  
                  if (generic_char) {
                          pin_free(generic_char);
                  }
                }
                else
                {
                  addStr(edr, DELIMITER, ebufp);
                  formatFieldsForFile(edr, (char *)FILE_FLD_FORMAT_9,
                                (char *)integrate_not_used,11,ebufp);
                }

                /*
                 * PIN DISCOUNT
                 */
                generic_decimal = (pin_decimal_t *)PIN_FLIST_FLD_GET(
                    event_packet_flistp,PIN_FLD_DISCOUNT, 0, ebufp);
		if(generic_decimal) {
                  generic_char = pbo_decimal_to_str(generic_decimal, ebufp);
                  addStr(edr, DELIMITER, ebufp);
                  formatFieldsForFile(edr,
                      (char *)FILE_FLD_FORMAT_9,(char *)generic_char,11,ebufp);
  
                  if (generic_char) {
                          pin_free(generic_char);
                  }
                }
                else
                {
                  addStr(edr, DELIMITER, ebufp);
                  formatFieldsForFile(edr, (char *)FILE_FLD_FORMAT_9,
                                (char *)integrate_not_used,11,ebufp);
                }
                /*
                 * PIN PERCENT
                 */
                generic_decimal = (pin_decimal_t *)PIN_FLIST_FLD_GET(
                    event_packet_flistp,PIN_FLD_PERCENT, 0, ebufp);
                if(generic_decimal) {
                  generic_char = pbo_decimal_to_str(generic_decimal, ebufp);
                  addStr(edr, DELIMITER, ebufp);
                  formatFieldsForFile(edr,
                      (char *)FILE_FLD_FORMAT_9,(char *)generic_char,11,ebufp);

                  if (generic_char) {
                          pin_free(generic_char);
                  }
                }
                else
                {
                  addStr(edr, DELIMITER, ebufp);
                  formatFieldsForFile(edr, (char *)FILE_FLD_FORMAT_9,
                                (char *)integrate_not_used,11,ebufp);
                }


                /*
                 * PIN DISCOUNT INFO STRING   
                 */
                generic_char = (char *)PIN_FLIST_FLD_GET(
                    event_packet_flistp,PIN_FLD_DISCOUNT_INFO, 0,
                    ebufp);
                addStr(edr, DELIMITER, ebufp);
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_X,(char *)generic_char,2000,ebufp);
                
                /*
                 * End of record (line) indicator
                 */
                addStr(edr, EOL_DELIMITER, ebufp);
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, (char *)edr->data);
        }

ErrNullPointer:
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "processSessionSupBalance error", ebufp);
                return;
                /*****/
        }
}

/****************************************************************************
 * Function     : processSessionSupService
 * Description  : Builds the report record for every supplemental
 *              : Service Event record
 *--------------+------------------------------------------------------------
 * In/Out       : pin_buf_t	*edr
 * In           : pin_flist_t	*flistp
 * In           : pin_errbuf_t *ebufp
 *--------------+------------------------------------------------------------
 * Returns      : void
 ****************************************************************************/
static void
processSessionSupService(
	pin_buf_t	*edr,
	pin_flist_t	*flistp,
	pin_errbuf_t	*ebufp)
{
        pin_flist_t     *svc_packet_flistp = NULL;
        
        int32           record_id = 0;
        pin_cookie_t	cookie_id = NULL;
        
        char	        generic_conv[256];
        int32           *generic_int = NULL;
        char            *generic_char = NULL;
        time_t          *generic_time = NULL;
        pin_decimal_t   *generic_decimal = NULL;
        
        if (PIN_ERR_IS_ERR(ebufp)) {
		return;
                /*****/
	}

        /*
         * Construct the flist in the correct order
         * Spec: sol42 CDR/EDR/IDR Format 6.3
         */

        while ((svc_packet_flistp = PIN_FLIST_ELEM_GET_NEXT(flistp,
	            PIN_FLD_SERVICE_CODES, &record_id, 1, &cookie_id,
	            ebufp)) != (pin_flist_t *)NULL) {
        
                            
                /*
                 * RECORD TYPE 
                 */
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_9,(char *)RECORD_TYPE_SUP_SVC,
			3,ebufp);

                /*
                 * RECORD NUMBER 
                 */
                addStr(edr, DELIMITER, ebufp);
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_9,(char *)RECORD_PLACE_HOLDER,
			9,ebufp);

                /*
                 * ACTION CODE   
                 */
                generic_char = (char *)PIN_FLIST_FLD_GET(
                    svc_packet_flistp,PIN_FLD_SS_ACTION_CODE, 0,
                    ebufp);
                addStr(edr, DELIMITER, ebufp);
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_H,(char *)generic_char,1,ebufp);

                /*
                 * SS CODE  
                 */
                generic_char = (char *)PIN_FLIST_FLD_GET(
                    svc_packet_flistp,PIN_FLD_SS_CODE, 0,
                    ebufp);
                addStr(edr, DELIMITER, ebufp);
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_H,(char *)generic_char,2,ebufp);

                /*
                 * End of record (line) indicator
                 */
                addStr(edr, EOL_DELIMITER, ebufp);

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, (char *)edr->data); 

        }

}

/****************************************************************************
 * Function     : processSessionSupMonitorBalance
 * Description  : Builds the report record for every 
 *              : Monitor Impact Packet (800) record
 *--------------+------------------------------------------------------------
 * In/Out       : pin_buf_t	*edr
 * In           : pin_flist_t	*flistp
 * In           : pin_errbuf_t *ebufp
 *--------------+------------------------------------------------------------
 * Returns      : void
 ****************************************************************************/
static void
processSessionSupMonitorBalance(
	pin_buf_t	*edr,
	pin_flist_t	*flistp,
	pin_errbuf_t	*ebufp)
{
        pin_flist_t     *event_packet_flistp = NULL;
        
        int32           record_id = 0;
        pin_cookie_t	cookie_id = NULL;
        
        char            generic_conv[256];
        int32           *generic_int = NULL;
        char            *generic_char = NULL;
        time_t          *generic_time = NULL;
        pin_decimal_t   *generic_decimal = NULL;
        poid_t          *generic_poid = NULL;

        int32	        maxpoidlen = 0;
	char	        poid_string[100];
        int32           generic_rerate_int = 0 ;
        char            integrate_not_used[2] = "0";
        
        if (PIN_ERR_IS_ERR(ebufp)) {
		return;
                /*****/
	}

        /*
         * Construct the flist in the correct order
         * Spec: sol42 CDR/EDR/IDR Format 6.3
         */

        while ((event_packet_flistp = PIN_FLIST_ELEM_GET_NEXT(flistp,
	            PIN_FLD_MONITOR_IMPACTS, &record_id, 1, &cookie_id,
	            ebufp)) != (pin_flist_t *)NULL) {
        
                /*
                 * RECORD TYPE 
                 */
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_9,(char *)RECORD_TYPE_SUP_MONITOR_BAL,
			3,ebufp);

                /*
                 * RECORD NUMBER 
                 */
                addStr(edr, DELIMITER, ebufp);
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_9,(char *)RECORD_PLACE_HOLDER,
			9, ebufp);
                
                /*
                 * PIN ACCOUNT POID 
                 */
                generic_poid = (char *)PIN_FLIST_FLD_GET(
                    event_packet_flistp,PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
                if ( !generic_poid ) {
                        pin_set_err(ebufp, PIN_ERRLOC_APP,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_NULL_PTR, PIN_FLD_ACCOUNT_OBJ, 0, 0);
                        goto ErrNullPointer;
                }
                memset(poid_string,0,sizeof(poid_string));
                generic_char = poid_string;
                maxpoidlen = sizeof(poid_string);
                PIN_POID_TO_STR((poid_t *)generic_poid, &generic_char, 
                    &maxpoidlen, ebufp);
                addStr(edr, DELIMITER, ebufp);
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_X,(char *)generic_char+6,255,ebufp);

                /*
                 * PIN BAL_GRP POID 
                 */
                generic_poid = (char *)PIN_FLIST_FLD_GET(
                    event_packet_flistp,PIN_FLD_BAL_GRP_OBJ, 0, ebufp);
                if ( !generic_poid ) {
                        pin_set_err(ebufp, PIN_ERRLOC_APP,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_NULL_PTR, PIN_FLD_BAL_GRP_OBJ, 0, 0);
                        goto ErrNullPointer;
                }
                memset(poid_string,0,sizeof(poid_string));
                generic_char = poid_string;
                maxpoidlen = sizeof(poid_string);
                PIN_POID_TO_STR((poid_t *)generic_poid, &generic_char, 
                    &maxpoidlen, ebufp);
                addStr(edr, DELIMITER, ebufp);
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_X,(char *)generic_char+6,255,ebufp);

                /*
                 * PIN RESOURCE ID 
                 */             
                generic_int = (int32 *)PIN_FLIST_FLD_GET(
                    event_packet_flistp,
                    PIN_FLD_RESOURCE_ID, 0, ebufp);
                if ( !generic_int ) {
                        pin_set_err(ebufp, PIN_ERRLOC_APP,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_NULL_PTR, PIN_FLD_RESOURCE_ID, 0, 0);
                        goto ErrNullPointer;
                }
                sprintf(generic_conv,"%ld",*generic_int);
                addStr(edr, DELIMITER, ebufp);
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,9,ebufp);
                                        
                /*
                 * PIN AMOUNT
                 */
                generic_decimal = (pin_decimal_t *)PIN_FLIST_FLD_GET(
                    event_packet_flistp,PIN_FLD_AMOUNT, 0, ebufp);
		if(generic_decimal) {
                  generic_char = pbo_decimal_to_str(generic_decimal, ebufp);
                  addStr(edr, DELIMITER, ebufp);
                  formatFieldsForFile(edr,
                      (char *)FILE_FLD_FORMAT_9,(char *)generic_char,11,ebufp);
  
                  if (generic_char) {
                          pin_free(generic_char);
                  }
                }
                else
                {
                  addStr(edr, DELIMITER, ebufp);
                  formatFieldsForFile(edr, (char *)FILE_FLD_FORMAT_9,
                                (char *)integrate_not_used,11,ebufp);
                } 
                /*
                 * End of record (line) indicator
                 */
                addStr(edr, EOL_DELIMITER, ebufp);
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, (char *)edr->data);
        }

ErrNullPointer:
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "processSessionSupMonitorBalance error", ebufp);
                return;
                /*****/
        }
}

/****************************************************************************
 * Function     : processSessionAssocInfranet
 * Description  : Builds the report record for an associated 
 *              : Portal Billing record
 *--------------+------------------------------------------------------------
 * In/Out       : pin_buf_t	*edr
 * In           : pin_flist_t  *flistp
 * In           : pin_errbuf_t *ebufp
 *--------------+------------------------------------------------------------
 * Returns      : void
 ****************************************************************************/
static void
processSessionAssocInfranet(
	pin_buf_t	*edr,
	pin_flist_t	*flistp,
	pin_errbuf_t	*ebufp)
{
        pin_flist_t     *event_packet_flistp = NULL;
        
        int32           record_id = 0;
        pin_cookie_t	cookie_id = NULL;
        int32           num_impact_packets = 0;
        int32           num_mon_impact_packets = 0;
        
        char            generic_conv[256];
        int32           *generic_int = NULL;
        char            *generic_char = NULL;
        time_t          *generic_time = NULL;
        pin_decimal_t   *generic_decimal = NULL;
        poid_t          *generic_poid = NULL;

        int32	        maxpoidlen = 0;
	char	        poid_string[100];
	void		*vp = NULL;
        
        if (PIN_ERR_IS_ERR(ebufp)) {
		return;
                /*****/
	}

        /*
         * Construct the flist in the correct order
         * Spec: sol42 CDR/EDR/IDR Format 6.3
         */
                
        /*
         * RECORD TYPE 
         */
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)RECORD_TYPE_ASSOC_INFRANET,
		3,ebufp);

        /*
         * RECORD NUMBER 
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)RECORD_PLACE_HOLDER,9,ebufp);

        /*
         * ACCOUNT POID
         */
        generic_poid = (char *)PIN_FLIST_FLD_GET(flistp,
           PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
        if ( !generic_poid ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_ACCOUNT_OBJ, 0, 0);
                goto ErrNullPointer;
        }
        memset(poid_string,0,sizeof(poid_string));
        generic_char = poid_string;
        maxpoidlen = sizeof(poid_string);
        PIN_POID_TO_STR((poid_t *)generic_poid, &generic_char, 
            &maxpoidlen, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char+6,255,ebufp);

        /*
         * SERVICE POID
         */
        generic_poid = (char *)PIN_FLIST_FLD_GET(flistp,
           PIN_FLD_SERVICE_OBJ, 0, ebufp);
        if ( !generic_poid ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_SERVICE_OBJ, 0, 0);
                goto ErrNullPointer;
        }
        memset(poid_string,0,sizeof(poid_string));
        generic_char = poid_string;
        maxpoidlen = sizeof(poid_string);
        PIN_POID_TO_STR((poid_t *)generic_poid, &generic_char, 
            &maxpoidlen, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char+6,255,ebufp);

        /*
         * ITEM POID
         */
        generic_poid = (char *)PIN_FLIST_FLD_GET(flistp,
           PIN_FLD_ITEM_OBJ, 0, ebufp);
        if ( !generic_poid ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_ITEM_OBJ, 0, 0);
                goto ErrNullPointer;
        }
        memset(poid_string,0,sizeof(poid_string));
        generic_char = poid_string;
        maxpoidlen = sizeof(poid_string);
        PIN_POID_TO_STR((poid_t *)generic_poid, &generic_char, 
            &maxpoidlen, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char+6,255,ebufp);

        /*
         * ORIGINAL EVENT POID
         */
        generic_poid = (char *)PIN_FLIST_FLD_GET(flistp,
           PIN_FLD_POID, 0, ebufp);
        if ( !generic_poid ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_POID, 0, 0);
                goto ErrNullPointer;
        }
        memset(poid_string,0,sizeof(poid_string));
        generic_char = poid_string;
        maxpoidlen = sizeof(poid_string);
        PIN_POID_TO_STR((poid_t *)generic_poid, &generic_char, 
            &maxpoidlen, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char+6,255,ebufp);
        
        /*
         * PIN TAX LOCALES
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(flistp,
           PIN_FLD_TAX_LOCALES, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,255,ebufp);

        /*
         * PIN TAX SUPPLIER
         */
        vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_TAX_SUPPLIER, 0, ebufp);
	if (vp) {
		record_id = *(int32*)vp;
	}
	sprintf(generic_conv, "%ld", record_id);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,2,ebufp);

        /*
         * PIN PROVIDER ID
         */
        generic_poid = (char *)PIN_FLIST_FLD_GET(flistp,
           PIN_FLD_PROVIDER_ID, 0, ebufp);
        if ( !generic_poid ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_PROVIDER_ID, 0, 0);
                goto ErrNullPointer;
        }
        memset(poid_string,0,sizeof(poid_string));
        generic_char = poid_string;
        maxpoidlen = sizeof(poid_string);
        PIN_POID_TO_STR((poid_t *)generic_poid, &generic_char, 
            &maxpoidlen, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char+6,255,ebufp);

        /*
         * NUMBER OF PIN BALANCE IMPACT PACKETS 
         */
        num_impact_packets = PIN_FLIST_ELEM_COUNT(flistp, 
            PIN_FLD_BAL_IMPACTS, ebufp);
        sprintf(generic_conv,"%ld",num_impact_packets);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,2,ebufp);

        /*
         * NUMBER OF PIN MONITOR IMPACT PACKETS
         */
        num_mon_impact_packets = PIN_FLIST_ELEM_COUNT(flistp,
            PIN_FLD_MONITOR_IMPACTS, ebufp);
        sprintf(generic_conv,"%ld",num_mon_impact_packets);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,2,ebufp);

        /*
         * PIN INVOICE DATA
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(flistp,
           PIN_FLD_INVOICE_DATA, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,255,ebufp);

        /*
         * End of record (line) indicator
         */
        addStr(edr,EOL_DELIMITER,ebufp);
        
        /*
         * PROCESS SUP BALANCE RECORDS IF APPLICABLE
         */
        if(num_impact_packets) {

                processSessionSupBalance(edr, flistp,ebufp);
				processSessionSubBalanceImpacts(edr,flistp,ebufp);
        }
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, RECORD_TYPE_SUB_BAL_IMPACTS);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, RECORD_TYPE_SUBBAL);

        /*
         * PROCESS MONITOR SUB BALANCE RECORDS 
         */
        if(num_mon_impact_packets) {

                processSessionSupMonitorBalance(edr, flistp,ebufp);
                processSessionMonitorSubBalanceImpacts(edr,flistp,ebufp);
        }
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, RECORD_TYPE_MONITOR_SUB_BAL_IMPACTS);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, RECORD_TYPE_MONITOR_SUBBAL);

ErrNullPointer:
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "processSessionAssocInfranet error", ebufp);
                return;
                /*****/
        }
}

/****************************************************************************
 * Function     : processSessionAssocCharge
 * Description  : Builds the report record for an associated 
 *              : charge record
 *--------------+------------------------------------------------------------
 * In/Out       : pin_buf_t	*edr
 * In           : pin_flist_t  *flistp
 * In           : pin_errbuf_t *ebufp
 *--------------+------------------------------------------------------------
 * Returns      : void
 ****************************************************************************/
static void
processSessionAssocCharge(
	pin_buf_t	*edr,
	pin_flist_t	*flistp,
	pin_errbuf_t	*ebufp)
{
        pin_flist_t     *event_packet_flistp = NULL;
        
        int32           record_id = 0;
        pin_cookie_t	cookie_id = NULL;
        int32           num_charge_packets = 0;
        
        char            generic_conv[256];
        int32           *generic_int = NULL;
        char            *generic_char = NULL;
        time_t          *generic_time = NULL;
        pin_decimal_t   *generic_decimal = NULL;
        poid_t          *generic_poid = NULL;
        char            integrate_not_used[2] = "0";

        int32	        maxpoidlen = 0;
	char	        poid_string[100];
        
        if (PIN_ERR_IS_ERR(ebufp)) {
		return;
                /*****/
	}

        /*
         * Construct the flist in the correct order
         * Spec: sol42 CDR/EDR/IDR Format 6.5
         */
                
        /*
         * RECORD TYPE 
         */
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)RECORD_TYPE_ASSOC_CHARGE,
		3,ebufp);

        /*
         * RECORD NUMBER 
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)RECORD_PLACE_HOLDER,9,ebufp);

        /*
         * CONTRACT CODE
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * SEGMENT CODE
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * CUSTOMER CODE
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * ACCOUNT CODE
         */
        addStr(edr, DELIMITER, ebufp);
        
        /*
         * SYSTEM BRAND CODE
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * SERVICE CODE
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * CUSTOMER RATEPLAN CODE
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * SERVICE LEVEL AGREEMENT CODE
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * CUSTOMER BILLCYCLE
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * CUSTOMER CURRENCY
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * CUSTOMER TAXGROUP
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * NUMBER OF CHARGE PACKETS
         */
	sprintf(generic_conv,"%ld",num_charge_packets);
	addStr(edr, DELIMITER, ebufp);
	formatFieldsForFile(edr,
	    (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,2,ebufp);


        /*
         * OPENING BALANCE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,11,ebufp);


        /*
         * CLOSING BALANCE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,11,ebufp);


        /*
         * End of record (line) indicator
         */
        addStr(edr,EOL_DELIMITER,ebufp);
        
        /*
         * PROCESS DISCOUNT IF APPLICABLE
         */
	
	/*processSessionDiscountPackets(edr, flistp,ebufp);*/
	    processSessionSubBalancePackets(edr, flistp, 
		RECORD_TYPE_DISCOUNT, RECORD_TYPE_SUBBALANCE, ebufp); 

}

/****************************************************************************
 * Function     : processSessionSubBalancePackets
 * Description  : Builds the report record for subbalance packets
 *--------------+------------------------------------------------------------
 * In/Out       : pin_buf_t	*edr
 * In           : pin_flist_t  *flistp
 * In           : char	  *sub_balimp_type
 * In           : char	  *sub_balances_type
 * In           : pin_errbuf_t *ebufp
 *--------------+------------------------------------------------------------
 * Returns      : void
 ****************************************************************************/
static void
processSessionSubBalancePackets(
	pin_buf_t	*edr,
	pin_flist_t	*flistp,
	char		*sub_balimp_type,
	char		*sub_balances_type,
	pin_errbuf_t	*ebufp)
{
        pin_flist_t     *event_packet_flistp = NULL;
        pin_flist_t     *sbi_flistp = NULL;
        pin_flist_t     *sb_flistp = NULL;
        
        int32           record_id = 0;
        pin_cookie_t	cookie_id = NULL;
        int32           num_charge_packets = 0;
        
        char            generic_conv[256];
        int32           *generic_int = NULL;
        char            *generic_char = NULL;
        time_t          *generic_time = NULL;
        pin_decimal_t   *generic_decimal = NULL;
        poid_t          *generic_poid = NULL;

        int32	        maxpoidlen = 0;
	char	        poid_string[100];
        char            integrate_not_used[2] = "0";

	poid_t          *a_pdp = NULL;
	
        
        if (PIN_ERR_IS_ERR(ebufp)) {
		return;
                /*****/
	}

        while ((sbi_flistp = PIN_FLIST_ELEM_GET_NEXT(flistp, 
			PIN_FLD_SUB_BAL_IMPACTS, &record_id, 1, &cookie_id, 
			ebufp)) != (pin_flist_t *)NULL) {
		
		if(strcmp(sub_balimp_type,RECORD_TYPE_SUB_BAL_IMPACTS)!=0)
		{
		/*
		 * Construct the flist in the correct order
		 * Spec: sol42 CDR/EDR/IDR Format 6.5
		 */
			
		/*
		 * RECORD TYPE 
		 */
		formatFieldsForFile(edr,
		    (char *)FILE_FLD_FORMAT_9,(char *)sub_balimp_type,3,ebufp);

		/*
		 * CREATED
		 */
		addStr(edr, DELIMITER, ebufp);

		/*
		 * OBJECT ID
		 */
		addStr(edr, DELIMITER, ebufp);

		/*
		 * OBJECT TYPE
		 */
		addStr(edr, DELIMITER, ebufp);

		/*
		 * OBJECT ACCOUNT
		 */
		addStr(edr, DELIMITER, ebufp);
		a_pdp = findAccountPoidForSubBalance (flistp, sbi_flistp, ebufp);
		if (a_pdp) {
	                pin_snprintf (poid_string, sizeof(poid_string), "%" I64_PRINTF_PATTERN "d",
        		 		PIN_POID_GET_ID(a_pdp));
	                formatFieldsForFile(edr,(char *)FILE_FLD_FORMAT_9, poid_string,
						strlen(poid_string),ebufp);  
		}
		
		/*
		 * OBJECT OWNER ID
		 */
		addStr(edr, DELIMITER, ebufp);
		formatFieldsForFile(edr, 
			(char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,
			1,ebufp);

		/*
		 * OBJECT OWNER TYPE
		 */
		addStr(edr, DELIMITER, ebufp);

		/*
		 * DISCOUNT MODEL
		 */
		addStr(edr, DELIMITER, ebufp);

		/*
		 * DISCOUNT RULE
		 */
		addStr(edr, DELIMITER, ebufp);

		/*
		 * DISCOUNT STEP ID
		 */
		addStr(edr, DELIMITER, ebufp);
		formatFieldsForFile(edr, 
			(char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,
			1,ebufp);

		/*
		 * DISCOUNT BALIMPACT ID
		 */
		addStr(edr, DELIMITER, ebufp);
		formatFieldsForFile(edr, 
			(char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,
			1,ebufp);

		/*
		 * TAX CODE
		 */
		addStr(edr, DELIMITER, ebufp);

		/*
		 * AMOUNT
		 */
		addStr(edr, DELIMITER, ebufp);
		formatFieldsForFile(edr, 
			(char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,
			1,ebufp);

		/*
		 * PIN_PERCENT
		 */
		addStr(edr, DELIMITER, ebufp);
		formatFieldsForFile(edr,
			(char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,
			1,ebufp);

		/*
		 * QUANTITY
		 */
		addStr(edr, DELIMITER, ebufp);
		formatFieldsForFile(edr, 
			(char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,
			1,ebufp);

		/*
		 * GRANTED AMOUNT
		 */
		addStr(edr, DELIMITER, ebufp);
		formatFieldsForFile(edr, 
			(char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,
			1,ebufp);

		/*
		* GRANTED_AMOUNT_ORIG
		*/
		addStr(edr, DELIMITER, ebufp);
		formatFieldsForFile(edr,
			(char *)FILE_FLD_FORMAT_Z,(char *)integrate_not_used,1,ebufp);

		/*
		 * GRANTED QUANTITY
		 */
		addStr(edr, DELIMITER, ebufp);
		formatFieldsForFile(edr, 
			(char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,
			1,ebufp);

		/*
		 * QUANTITY FROM
		 */
		addStr(edr, DELIMITER, ebufp);
		formatFieldsForFile(edr, 
			(char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,
			1,ebufp);

		/*
		 * QUANTITY TO
		 */
		addStr(edr, DELIMITER, ebufp);
		formatFieldsForFile(edr, 
			(char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,
			1,ebufp);

		/*
		 * VALID FROM
		 */
		addStr(edr, DELIMITER, ebufp);
		formatFieldsForFile(edr, 
			(char *)FILE_FLD_FORMAT_Z,(char *)integrate_not_used,
			1,ebufp);

		/*
		 * VALID TO
		 */
		addStr(edr, DELIMITER, ebufp);
		formatFieldsForFile(edr, 
			(char *)FILE_FLD_FORMAT_Z,(char *)integrate_not_used,
			1,ebufp);

		/*
		 * BALANCE GROUP ID
		 */
                generic_poid = (int32 *)PIN_FLIST_FLD_GET( sbi_flistp, 
			PIN_FLD_BAL_GRP_OBJ, 0, ebufp);
                if ( !generic_poid ) {
                        pin_set_err(ebufp, PIN_ERRLOC_APP,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_NULL_PTR, PIN_FLD_BAL_GRP_OBJ, 0, 0);
                        goto ErrNullPointer;
                }
                sprintf(generic_conv,"%"I64_PRINTF_PATTERN"d", 
				PIN_POID_GET_ID(generic_poid));
		addStr(edr, DELIMITER, ebufp);
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,9,ebufp);

		/*
		 * RESOURCE ID
		 */
                generic_int = (int32 *)PIN_FLIST_FLD_GET( sbi_flistp, 
			PIN_FLD_RESOURCE_ID, 0, ebufp);
                if ( !generic_int ) {
                        pin_set_err(ebufp, PIN_ERRLOC_APP,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_NULL_PTR, PIN_FLD_BAL_GRP_OBJ, 0, 0);
                        goto ErrNullPointer;
                }
                sprintf(generic_conv,"%ld",*generic_int);
                addStr(edr, DELIMITER, ebufp);
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,9,ebufp);

		/*
		* RESOURCE_ID_ORIG
		*/
		addStr(edr, DELIMITER, ebufp);
		formatFieldsForFile(edr,
			(char *)FILE_FLD_FORMAT_Z,(char *)integrate_not_used,1,ebufp);

		/*
		 * ZONEMODEL_CODE
		 */
		addStr(edr, DELIMITER, ebufp);

		/*
		 * IMPACT_CATEGORY
		 */
		addStr(edr, DELIMITER, ebufp);

		/*
		 * SERVICE_CODE
		 */
		addStr(edr, DELIMITER, ebufp);

		/*
		 * TIMEZONE_CODE
		 */
		addStr(edr, DELIMITER, ebufp);

		/*
		 * TIMEMODEL_CODE
		 */
		addStr(edr, DELIMITER, ebufp);

		/*
		 * SERVICE_CLASS
		 */
		addStr(edr, DELIMITER, ebufp);

		/*
		 * PRICEMODEL_CODE
		 */
		addStr(edr, DELIMITER, ebufp);

		/*
		 * RUM
		 */
		addStr(edr, DELIMITER, ebufp);

		/*
		 * RATETAG
		 */
		addStr(edr, DELIMITER, ebufp);

		/*
		 * RATEPLAN
		 */
		addStr(edr, DELIMITER, ebufp);

		/*
		 * GLID
		 */
		addStr(edr, DELIMITER, ebufp);

		/*
		 * End of record (line) indicator
		 */
		addStr(edr,EOL_DELIMITER,ebufp);
        
		/*
		 * PROCESS DISCOUNT IF APPLICABLE
		 */
		}
         processSessionSubBalances(edr, sbi_flistp, sub_balances_type, 
		 ebufp);
	}

ErrNullPointer:
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "processSessionSubBalancePackets error", ebufp);
                return;
                /*****/
        }
}

/****************************************************************************
* Function     : findAccountPoidForSubBalance
* Description  : Finds the account poid for a given
*              : sub balance.
*--------------+------------------------------------------------------------
* In           : pin_flist_t *bi_flp
* In           : pin_flist_t *sbi_flp
* In           : pin_errbuf_t *ebufp
*--------------+------------------------------------------------------------
* Returns      : poid_t *
****************************************************************************/
static poid_t *
findAccountPoidForSubBalance(pin_flist_t *bi_flp,
                                     pin_flist_t *sbi_flp,
                                     pin_errbuf_t *ebufp)
{
	poid_t *bi_bg_pdp = NULL;
	poid_t *sbi_bg_pdp = NULL;
	pin_cookie_t cookie = NULL;
	pin_flist_t *imp_flp = NULL;
	int32 rec = 0;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return NULL;
	}

	/*
	 * Get balance group of sub-balance impact and search for the equivalent
	 * on a balance impact - from here we can get the account.
	 */
	sbi_bg_pdp = PIN_FLIST_FLD_GET(sbi_flp, PIN_FLD_BAL_GRP_OBJ, 0, ebufp);
	while ((imp_flp = PIN_FLIST_ELEM_GET_NEXT(bi_flp,
		PIN_FLD_BAL_IMPACTS, &rec, 1,
		&cookie, ebufp)) != NULL)
	{
		bi_bg_pdp = PIN_FLIST_FLD_GET(imp_flp, PIN_FLD_BAL_GRP_OBJ, 0, ebufp);
		if (PIN_POID_COMPARE(bi_bg_pdp, sbi_bg_pdp, 0, ebufp) == 0) {
			return (poid_t *) PIN_FLIST_FLD_GET(imp_flp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
		}
	}

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"findAccountPoidForSubBalance error", ebufp);
        }

	return NULL;
}
                         
/****************************************************************************
 * Function     : processSessionSubBalances
 * Description  : Builds the report record for SubBalances packets type 607 
 *              : and 807
 *--------------+------------------------------------------------------------
 * In/Out       : pin_buf_t	*edr
 * In           : pin_flist_t  *flistp
 * In           : char	  *sub_balances_type
 * In           : pin_errbuf_t *ebufp
 *--------------+------------------------------------------------------------
 * Returns      : void
 ****************************************************************************/
static void
processSessionSubBalances(
	pin_buf_t	*edr,
	pin_flist_t	*flistp,
	char		*sub_balances_type,
	pin_errbuf_t	*ebufp)
{
        pin_flist_t     *event_packet_flistp = NULL;
        pin_flist_t     *sb_flistp = NULL;
        
        int32           record_id = 0;
        pin_cookie_t	cookie_id = NULL;
        
        char            generic_conv[256];
        int32           *generic_int = NULL;
        char            *generic_char = NULL;
        time_t          *generic_time = NULL;
        pin_decimal_t   *generic_decimal = NULL;
        poid_t          *generic_poid = NULL;

        char            integrate_not_used[2] = "0";
        
        if (PIN_ERR_IS_ERR(ebufp)) {
		return;
                /*****/
	}

        while ((sb_flistp = PIN_FLIST_ELEM_GET_NEXT(flistp, 
			PIN_FLD_SUB_BALANCES, &record_id, 1, &cookie_id, 
			ebufp)) != (pin_flist_t *)NULL) {
		
		/*
		 * Construct the flist in the correct order
		 * Spec: sol42 CDR/EDR/IDR Format 6.5
		 */
			
		/*
		 * RECORD TYPE 
		 */

                formatFieldsForFile(edr,
		    (char *)FILE_FLD_FORMAT_9,(char *)sub_balances_type,
			3,ebufp);

		/*
		 * RECORD NUMBER 
		 */
                sprintf(generic_conv,"%ld", record_id);
                addStr(edr, DELIMITER, ebufp);
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,9,ebufp);

		/*
		 * PIN AMOUNT
		 */
                generic_decimal = (pin_decimal_t *)PIN_FLIST_FLD_GET(
			sb_flistp,PIN_FLD_AMOUNT, 0, ebufp);
		if(generic_decimal) {
                  generic_char = pbo_decimal_to_str(generic_decimal, ebufp);
                  addStr(edr, DELIMITER, ebufp);
                  formatFieldsForFile(edr,
                      (char *)FILE_FLD_FORMAT_9,(char *)generic_char,11,ebufp);
  
                  if (generic_char) {
                          pin_free(generic_char);
                  }
                }
                else
                {
                  addStr(edr, DELIMITER, ebufp);
                  formatFieldsForFile(edr, (char *)FILE_FLD_FORMAT_9,
                                (char *)integrate_not_used,11,ebufp);
                }

		/*
		 * VALID FROM
		 */
		generic_time = (time_t *)PIN_FLIST_FLD_GET(sb_flistp, 
			PIN_FLD_VALID_FROM, 0, ebufp); 
                if ( !generic_time ) {
                        pin_set_err(ebufp, PIN_ERRLOC_APP,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_NULL_PTR, PIN_FLD_VALID_FROM, 0, 0);
                        goto ErrNullPointer;
                }
		sprintf(generic_conv, "%ld",*generic_time); 
		addStr(edr, DELIMITER, ebufp); 
		formatFieldsForFile(edr, (char *)FILE_FLD_FORMAT_Z,
			(char *)generic_conv, strlen(generic_conv),ebufp);

		/*
		 * VALID TO
		 */
		generic_time = (time_t *)PIN_FLIST_FLD_GET(sb_flistp, 
			PIN_FLD_VALID_TO, 0, ebufp); 
                if ( !generic_time ) {
                        pin_set_err(ebufp, PIN_ERRLOC_APP,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_NULL_PTR, PIN_FLD_VALID_TO, 0, 0);
                        goto ErrNullPointer;
                }
		sprintf(generic_conv, "%ld",*generic_time); 
		addStr(edr, DELIMITER, ebufp); 
		formatFieldsForFile(edr, (char *)FILE_FLD_FORMAT_Z,
			(char *)generic_conv, strlen(generic_conv),ebufp);

		/*
		 * CONTRIBUTOR
		 */
		generic_char = (char *)PIN_FLIST_FLD_GET(sb_flistp, 
			PIN_FLD_CONTRIBUTOR_STR, 0, ebufp);
		addStr(edr, DELIMITER, ebufp);
		formatFieldsForFile(edr,
		    (char *)FILE_FLD_FORMAT_X,(char *)generic_char,255,ebufp);


		/*
		 * End of record (line) indicator
		 */
		addStr(edr,EOL_DELIMITER,ebufp);
	}

ErrNullPointer:
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "processSessionSubBalances error", ebufp);
                return;
                /*****/
        }
}

/****************************************************************************
 * Function     : processSessionAssocImt
 * Description  : Builds the report record for an associated 
 *              : IMT extension record
 *--------------+------------------------------------------------------------
 * In/Out       : pin_buf_t	*edr
 * In           : pin_flist_t	*imt_flistp
 * In           : pin_errbuf_t *ebufp
 *--------------+------------------------------------------------------------
 * Returns      : void
 ****************************************************************************/
static void
processSessionAssocImt(
	pin_buf_t	*edr,
	pin_flist_t *imt_flistp,
	pin_errbuf_t	*ebufp)
{
       /* pin_flist_t	*gsm_info_flistp = NULL;*/

        pin_flist_t	*telco_info_flistp = NULL;
        pin_flist_t	*event_packet_flistp = NULL;        
        
        int32           record_id = 0;
        pin_cookie_t	cookie_id = NULL;
        int32           num_service_codes = 0;
        
        char            generic_conv[256];
        int32           *generic_int = NULL;
        char            *generic_char = NULL;
        time_t          *generic_time = NULL;
        pin_decimal_t   *generic_decimal= NULL;
        char            integrate_not_used[2] = "0";

        if (PIN_ERR_IS_ERR(ebufp)) {
		return;
                /*****/
	}

        telco_info_flistp = PIN_FLIST_SUBSTR_GET(imt_flistp,
            PIN_FLD_TELCO_INFO, 1, ebufp);

	if (!telco_info_flistp) {

              goto PROCESS_FILE_ERROR;
              /**********************/
        }

        /*
         * Construct the flist in the correct order
         * Spec: sol42 CDR/EDR/IDR Format 6.3
         */
        
        /*
         * RECORD TYPE 
         */
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)RECORD_TYPE_IMT_EXT,
		3,ebufp);

        /*
         * RECORD NUMBER 
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)RECORD_PLACE_HOLDER,9,ebufp);

        /*
         * PORT NUMBER
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_SECONDARY_MSID, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,24,ebufp);

        /*
         * DEVICE NUMBER
         */

	 addStr(edr, DELIMITER, ebufp);

        /*
         * A-NUMBER USER
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_CALLING_FROM, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,40,ebufp);

        /*
         * DIALED DIGITS
         */
	 addStr(edr, DELIMITER, ebufp);

        /*
         * BASIC DUAL SERVICE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * VAS/PRODUCT CODE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * ORIGINATING SWITCH ID
         */
	 addStr(edr, DELIMITER, ebufp);


        /*
         * TERMINATING SWITCH ID
         */
		  addStr(edr, DELIMITER, ebufp);

        /*
         * TRUNK INPUT - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * TRUNK OUTPUT - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * LOCATION AREA INDICATOR
         */
		  addStr(edr, DELIMITER, ebufp);

        /*
         * CELL ID
         */
	 addStr(edr, DELIMITER, ebufp);


        /*
         * MS CLASS MARK - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,1,ebufp);

        /*
         * TIME BEFORE ANSWER - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,5,ebufp);

        /*
         * BASIC AoC AMOUNT VALUE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,11,ebufp);

        /*
         * BASIC AoC AMOUNT CURRENCY - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * ROAMER AoC AMOUNT VALUE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,11,ebufp);

        /*
         * ROAMER AoC AMOUNT CURRENCY - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * NUMBER OF SUPPLEMENTARY SERVICE PACKETS 
         */
        num_service_codes = PIN_FLIST_ELEM_COUNT(imt_flistp, 
            PIN_FLD_SERVICE_CODES, ebufp);
        sprintf(generic_conv,"%ld",num_service_codes);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,2,ebufp);


        /*
         * B PARTY CELL ID - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * LAST CELL ID - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);


        /*
	 * SMSC_ADDRESS - NOT USED
	 */
	addStr(edr, DELIMITER, ebufp);


	/*
         * End of record (line) indicator
         */
        addStr(edr, EOL_DELIMITER, ebufp);

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, (char *)edr->data);

        /*
         * WRITE SUPPLEMENTARY SERVICE PACKETS IF APPLICABLE
         */
        if(num_service_codes) {

                processSessionSupService(edr, imt_flistp,ebufp);
        }

        return;
        /*****/

PROCESS_FILE_ERROR:

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, 
            (char *)"*** MISSING ASSOC IMT INFO FIELDS - UNABLE TO PROCESS ***", 
            imt_flistp);

}

/****************************************************************************
 * Function     : processSessionAssocPdc
 * Description  : Builds the report record for an associated 
 *              : PDC extension record
 *--------------+------------------------------------------------------------
 * In/Out       : pin_buf_t	*edr
 * In           : pin_flist_t	*pdc_flistp
 * In           : pin_errbuf_t *ebufp
 *--------------+------------------------------------------------------------
 * Returns      : void
 ****************************************************************************/
static void
processSessionAssocPdc(
	pin_buf_t	*edr,
	pin_flist_t *pdc_flistp,
	pin_errbuf_t	*ebufp)
{
        pin_flist_t	*telco_info_flistp = NULL;
        pin_flist_t	*event_packet_flistp = NULL;        
        
        int32           record_id = 0;
        pin_cookie_t	cookie_id = NULL;
        int32           num_service_codes = 0;
        
        char            generic_conv[256];
        int32           *generic_int = NULL;
        char            *generic_char = NULL;
        time_t          *generic_time = NULL;
        pin_decimal_t   *generic_decimal= NULL;
        char            integrate_not_used[2] = "0";

        if (PIN_ERR_IS_ERR(ebufp)) {
		return;
                /*****/
	}


        telco_info_flistp = PIN_FLIST_SUBSTR_GET(pdc_flistp,
            PIN_FLD_TELCO_INFO, 1, ebufp);

	if (!telco_info_flistp) {

              goto PROCESS_FILE_ERROR;
              /**********************/
        }

        /*
         * Construct the flist in the correct order
         * Spec: sol42 CDR/EDR/IDR Format 6.3
         */
        
        /*
         * RECORD TYPE 
         */
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)RECORD_TYPE_IMT_EXT,
		3,ebufp);

        /*
         * RECORD NUMBER 
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)RECORD_PLACE_HOLDER,9,ebufp);

        /*
         * PORT NUMBER
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_SECONDARY_MSID, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,24,ebufp);

        /*
         * DEVICE NUMBER
         */

	 addStr(edr, DELIMITER, ebufp);


        /*
         * A-NUMBER USER
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_CALLING_FROM, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,40,ebufp);

        /*
         * DIALED DIGITS
         */
	 addStr(edr, DELIMITER, ebufp);

        /*
         * BASIC DUAL SERVICE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * VAS/PRODUCT CODE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * ORIGINATING SWITCH ID
         */
	 addStr(edr, DELIMITER, ebufp);


        /*
         * TERMINATING SWITCH ID
         */
		  addStr(edr, DELIMITER, ebufp);

        /*
         * TRUNK INPUT - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * TRUNK OUTPUT - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * LOCATION AREA INDICATOR
         */
		  addStr(edr, DELIMITER, ebufp);

        /*
         * CELL ID
         */
	 addStr(edr, DELIMITER, ebufp);

        /*
         * MS CLASS MARK - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,1,ebufp);

        /*
         * TIME BEFORE ANSWER - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,5,ebufp);

        /*
         * BASIC AoC AMOUNT VALUE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,11,ebufp);

        /*
         * BASIC AoC AMOUNT CURRENCY - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * ROAMER AoC AMOUNT VALUE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,11,ebufp);

        /*
         * ROAMER AoC AMOUNT CURRENCY - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * NUMBER OF SUPPLEMENTARY SERVICE PACKETS 
         */
        num_service_codes = PIN_FLIST_ELEM_COUNT(pdc_flistp, 
            PIN_FLD_SERVICE_CODES, ebufp);
        sprintf(generic_conv,"%ld",num_service_codes);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,2,ebufp);


        /*
         * B PARTY CELL ID - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * LAST CELL ID - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);


        /*
         * End of record (line) indicator
         */
        addStr(edr, EOL_DELIMITER, ebufp);

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, (char *)edr->data);

        /*
         * WRITE SUPPLEMENTARY SERVICE PACKETS IF APPLICABLE
         */
        if(num_service_codes) {

                processSessionSupService(edr, pdc_flistp,ebufp);
        }

        return;
        /*****/

PROCESS_FILE_ERROR:

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, 
            (char *)"*** MISSING ASSOC PDC INFO FIELDS - UNABLE TO PROCESS ***", 
            pdc_flistp);

}

/****************************************************************************
 * Function     : processSessionAssocGsm
 * Description  : Builds the report record for an associated 
 *              : GSM/Wireline extension record
 *--------------+------------------------------------------------------------
 * In/Out       : pin_buf_t	*edr
 * In           : pin_flist_t	*gsm_flistp
 * In           : pin_errbuf_t *ebufp
 *--------------+------------------------------------------------------------
 * Returns      : void
 ****************************************************************************/
static void
processSessionAssocGsm(
	pin_buf_t	*edr,
	pin_flist_t *gsm_flistp,
	pin_errbuf_t	*ebufp)
{
        pin_flist_t	*gsm_info_flistp = NULL;
        pin_flist_t	*telco_info_flistp = NULL;
        pin_flist_t	*event_packet_flistp = NULL;        
        
        int32           record_id = 0;
        pin_cookie_t	cookie_id = NULL;
        int32           num_service_codes = 0;
        
        char            generic_conv[256];
        int32           *generic_int = NULL;
        char            *generic_char = NULL;
        time_t          *generic_time = NULL;
        pin_decimal_t   *generic_decimal= NULL;
        char            integrate_not_used[2] = "0";

        if (PIN_ERR_IS_ERR(ebufp)) {
		return;
                /*****/
	}

        gsm_info_flistp = PIN_FLIST_SUBSTR_GET(gsm_flistp,
            PIN_FLD_GSM_INFO, 1, ebufp);

        telco_info_flistp = PIN_FLIST_SUBSTR_GET(gsm_flistp,
            PIN_FLD_TELCO_INFO, 1, ebufp);

	if (!gsm_info_flistp || !telco_info_flistp) {

              goto PROCESS_FILE_ERROR;
              /**********************/
        }

        /*
         * Construct the flist in the correct order
         * Spec: sol42 CDR/EDR/IDR Format 6.3
         */
        
        /*
         * RECORD TYPE 
         */
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)RECORD_TYPE_GSM_WIRELINE_EXT,
		3,ebufp);

        /*
         * RECORD NUMBER 
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)RECORD_PLACE_HOLDER,9,ebufp);

        /*
         * PORT NUMBER
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_SECONDARY_MSID, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,24,ebufp);

        /*
         * DEVICE NUMBER
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gsm_info_flistp,
           PIN_FLD_IMEI, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,24,ebufp);

        /*
         * A-NUMBER USER
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_CALLING_FROM, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,40,ebufp);

        /*
         * DIALED DIGITS
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gsm_info_flistp,
           PIN_FLD_DIALED_NUMBER, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,40,ebufp);

        /*
         * BASIC DUAL SERVICE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * VAS/PRODUCT CODE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * ORIGINATING SWITCH ID
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gsm_info_flistp,
           PIN_FLD_ORIGIN_SID, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,15,ebufp);

        /*
         * TERMINATING SWITCH ID
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gsm_info_flistp,
           PIN_FLD_DESTINATION_SID, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,15,ebufp);

        /*
         * TRUNK INPUT - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * TRUNK OUTPUT - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * LOCATION AREA INDICATOR
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gsm_info_flistp,
           PIN_FLD_LOC_AREA_CODE, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,10,ebufp);

        /*
         * CELL ID
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gsm_info_flistp,
           PIN_FLD_CELL_ID, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,10,ebufp);

        /*
         * MS CLASS MARK - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,1,ebufp);

        /*
         * TIME BEFORE ANSWER - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,5,ebufp);

        /*
         * BASIC AoC AMOUNT VALUE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,11,ebufp);

        /*
         * BASIC AoC AMOUNT CURRENCY - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * ROAMER AoC AMOUNT VALUE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,11,ebufp);

        /*
         * ROAMER AoC AMOUNT CURRENCY - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * NUMBER OF SUPPLEMENTARY SERVICE PACKETS 
         */
        num_service_codes = PIN_FLIST_ELEM_COUNT(gsm_flistp, 
            PIN_FLD_SERVICE_CODES, ebufp);
        sprintf(generic_conv,"%ld",num_service_codes);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,2,ebufp);


        /*
         * B PARTY CELL ID - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * LAST CELL ID - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);


        /*
	 * SMSC_ADDRESS - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);


	/*
         * End of record (line) indicator
         */
        addStr(edr, EOL_DELIMITER, ebufp);

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, (char *)edr->data);

        /*
         * WRITE SUPPLEMENTARY SERVICE PACKETS IF APPLICABLE
         */
        if(num_service_codes) {

                processSessionSupService(edr, gsm_flistp,ebufp);
        }

        return;
        /*****/

PROCESS_FILE_ERROR:

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, 
            (char *)"*** MISSING ASSOC GSM INFO FIELDS - UNABLE TO PROCESS ***", 
            gsm_flistp);

}

/****************************************************************************
 * Function     : processSessionAssocGprs
 * Description  : Builds the report record for an associated 
 *              : GPRS extension record
 *--------------+------------------------------------------------------------
 * In/Out       : pin_buf_t	*edr
 * In           : pin_flist_t	*gprs_flistp
 * In           : pin_errbuf_t *ebufp
 *--------------+------------------------------------------------------------
 * Returns      : void
 ****************************************************************************/
static void
processSessionAssocGprs(
	pin_buf_t	*edr,
	pin_flist_t	*gprs_flistp,
	pin_errbuf_t	*ebufp)
{
        pin_flist_t	*gprs_info_flistp = NULL;
        
        int32           record_id = 0;
        pin_cookie_t	cookie_id = NULL;
        int32           num_service_codes = 0;
        
        char            generic_conv[256];
        int32           *generic_int = NULL;
        char            *generic_char = NULL;
        time_t          *generic_time = NULL;
        pin_decimal_t   *generic_decimal= NULL;
        char            integrate_not_used[2] = "0";

        if (PIN_ERR_IS_ERR(ebufp)) {
		return;
                /*****/
	}

        gprs_info_flistp = PIN_FLIST_ELEM_GET_NEXT(gprs_flistp,
            PIN_FLD_GPRS_INFO, &record_id, 1, &cookie_id, ebufp);

	if (gprs_info_flistp == (pin_flist_t *)NULL) {

              goto PROCESS_FILE_ERROR;
              /**********************/
        }

	/*
         * Construct the flist in the correct order
         * Spec: sol42 CDR/EDR/IDR Format 6.3
         */
        
        /*
         * RECORD TYPE 
         */
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)RECORD_TYPE_GPRS_EXT,3,ebufp);

        /*
         * RECORD NUMBER 
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)RECORD_PLACE_HOLDER,9,ebufp);

        /*
         * PORT NUMBER
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_IMSI, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,24,ebufp);

        /*
         * DEVICE NUMBER
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_IMEI, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,24,ebufp);

        /*
         * A-NUMBER USER - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * DIALED DIGITS - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * VAS/PRODUCT CODE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * ORIGINATING SWITCH ID - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * TERMINATING SWITCH ID - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * MS CLASS MARK - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,1,ebufp);

        /*
         * ROUTING AREA
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_ROUTING_AREA, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,10,ebufp);

        /*
         * LOCATION AREA INDICATOR
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_LOC_AREA_CODE, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,10,ebufp);

        /*
         * CHARGING ID 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_SESSION_ID, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_SESSION_ID, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,10,ebufp);

        /*
         * SGSN ADDRESS
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_SGSN_ADDRESS, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,64,ebufp);

        /*
         * GGSN ADDRESS
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_GGSN_ADDRESS, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,64,ebufp);

        /*
         * APN ADDRESS
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_APN, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,64,ebufp);

        /*
         * NODE ID
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_NODE_ID, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,64,ebufp);

        /*
         * TRANS ID
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_TRANS_ID, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_char,10,ebufp);

        /*
         * SUB TRANS ID
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_SUB_TRANS_ID, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_char,10,ebufp);

        /*
         * NETWORK INITIATED PDP 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_NI_PDP, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_NI_PDP, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,1,ebufp);

        /*
         * PDP TYPE
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_PDP_TYPE, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,4,ebufp);

        /*
         * PDP ADDRESS
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_PDP_ADDRESS, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,64,ebufp);

        /*
         * PDP REMOTE ADDRESS
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_PDP_RADDRESS, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,255,ebufp);

        /*
         * PDP DYNAMIC ADDRESS 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_PDP_DYNADDR, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_PDP_DYNADDR, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,1,ebufp);

        /*
         * DIAGNOSTICS
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_DIAGNOSTICS, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,255,ebufp);

        /*
         * CELL ID
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_CELL_ID, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,10,ebufp);

        /*
         * CHANGE CONDITION 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_CHANGE_CONDITION, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_CHANGE_CONDITION, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,1,ebufp);

        /*
         * QoS REQUESTED PRECEDENCE 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_QOS_REQ_PRECEDENCE, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_QOS_REQ_PRECEDENCE, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,1,ebufp);

        /*
         * QoS REQUESTED DELAY 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_QOS_REQ_DELAY, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_QOS_REQ_DELAY, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,1,ebufp);

        /*
         * QoS REQUESTED RELIABILITY 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_QOS_REQ_RELIABILITY, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_QOS_REQ_RELIABILITY, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,1,ebufp);

        /*
         * QoS REQUESTED PEAK THROUGHPUT 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_QOS_REQ_PEAK_THROUGH, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_QOS_REQ_PEAK_THROUGH, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,2,ebufp);

        /*
         * QoS REQUESTED MEAN THROUGHPUT 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_QOS_REQ_MEAN_THROUGH, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_QOS_REQ_MEAN_THROUGH, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,2,ebufp);

        /*
         * QoS USED PRECEDENCE 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_QOS_NEGO_PRECEDENCE, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_QOS_NEGO_PRECEDENCE, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,1,ebufp);
        
        /*
         * QoS USED DELAY 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_QOS_NEGO_DELAY, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_QOS_NEGO_DELAY, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,1,ebufp);

        /*
         * QoS USED RELIABILITY 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_QOS_NEGO_RELIABILITY, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_QOS_NEGO_RELIABILITY, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,1,ebufp);

        /*
         * QoS USED PEAK THROUGHPUT 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_QOS_NEGO_PEAK_THROUGH, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_QOS_NEGO_PEAK_THROUGH, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,2,ebufp);

        /*
         * QoS USED MEAN THROUGHPUT 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_QOS_NEGO_MEAN_THROUGH, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_QOS_NEGO_MEAN_THROUGH, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,2,ebufp);

        /*
         * NETWORK CAPABILITY
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_NETWORK_CAPABILITY, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,10,ebufp);

        /*
         * SGSN CHANGE 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_SGSN_CHANGE, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_SGSN_CHANGE, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,1,ebufp);

        /*
         * SEQUENCE NUMBER OF FIRST DATA EVENT - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * SEQUENCE NUMBER OF LAST DATA EVENT - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);


        /*
         * B PARTY CELL ID - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * LAST CELL ID - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * End of record (line) indicator
         */
        addStr(edr,EOL_DELIMITER,ebufp);

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, (char *)edr->data);
        
        return;
        /*****/

PROCESS_FILE_ERROR:

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, 
            (char *)"*** MISSING ASSOC GPRS INFO FIELDS "
            "- UNABLE TO PROCESS ***", 
            gprs_flistp);

        return;
        /*****/

ErrNullPointer:
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "processSessionAssocGprs error", ebufp);
                return;
                /*****/
        }
}

/****************************************************************************
 * Function     : processSessionAssocWap
 * Description  : Builds the report record for an associated 
 *              : WAP extension record
 *--------------+------------------------------------------------------------
 * In/Out       : pin_buf_t	*edr
 * In           : pin_flist_t *wap_flistp
 * In           : pin_errbuf_t *ebufp
 *--------------+------------------------------------------------------------
 * Returns      : void
 ****************************************************************************/
static void
processSessionAssocWap(
	pin_buf_t	*edr,
	pin_flist_t	*wap_flistp,
	pin_errbuf_t	*ebufp)
{
        pin_flist_t	*wap_info_flistp = NULL;
        
        int32           record_id = 0;
        pin_cookie_t	cookie_id = NULL;
        int32           num_service_codes = 0;
        
        char            generic_conv[256];
        int32           *generic_int = NULL;
        char            *generic_char = NULL;
        time_t          *generic_time = NULL;
        pin_decimal_t   *generic_decimal= NULL;
        
        if (PIN_ERR_IS_ERR(ebufp)) {
		return;
                /*****/
	}

        wap_info_flistp = PIN_FLIST_ELEM_GET_NEXT(wap_flistp,
                            PIN_FLD_WAP_INFO,
		            &record_id, 1, &cookie_id, ebufp);

	if (wap_info_flistp == (pin_flist_t *)NULL) {

              goto PROCESS_FILE_ERROR;
              /**********************/
        }

	/*
         * Construct the flist in the correct order
         * Spec: sol42 CDR/EDR/IDR Format 6.3
         */
        
        /*
         * RECORD TYPE 
         */
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)RECORD_TYPE_WAP_EXT,3,ebufp);

        /*
         * RECORD NUMBER 
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)RECORD_PLACE_HOLDER,9,ebufp);

        /*
         * PORT NUMBER
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(wap_info_flistp,
           PIN_FLD_IMSI, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,24,ebufp);

        /*
         * DEVICE NUMBER
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(wap_info_flistp,
           PIN_FLD_IMEI, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,24,ebufp);

        /*
         * SESSION ID
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(wap_info_flistp,
           PIN_FLD_SESSION_ID, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_SESSION_ID, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,64,ebufp);

        /*
         * RECORDING ENTITY
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(wap_info_flistp,
           PIN_FLD_RECORDING_ENTITY, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,64,ebufp);

        /*
         * TERMINAL CLIENT ID - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * TERMINAL IP ADDRESS
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(wap_info_flistp,
           PIN_FLD_TERMINAL_IP_ADDRESS, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,64,ebufp);

        /*
         * DOMAIN URL
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(wap_info_flistp,
           PIN_FLD_URL, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,255,ebufp);

        /*
         * BEARER SERVICE
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(wap_info_flistp,
           PIN_FLD_BEARER_TYPE, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,3,ebufp);

        /*
         * HTTP STATUS
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(wap_info_flistp,
           PIN_FLD_HTTP_STATUS, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_HTTP_STATUS, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,3,ebufp);

        /*
         * WAP STATUS
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(wap_info_flistp,
           PIN_FLD_WAP_STATUS, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_WAP_STATUS, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,3,ebufp);

        /*
         * ACKNOWLEDGE STATUS
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(wap_info_flistp,
           PIN_FLD_WAP_ACK_STATUS, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_WAP_ACK_STATUS, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,1,ebufp);

        /*
         * ACKNOWLEDGE TIME
         */
        generic_time = (time_t *)PIN_FLIST_FLD_GET(wap_info_flistp,
           PIN_FLD_ACK_TIME, 0, ebufp);
        if ( !generic_time ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_ACK_TIME, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_time);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)generic_conv,
            strlen(generic_conv),ebufp);

        /*
         * EVENT NUMBER - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * GGSN ADDRESS
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(wap_info_flistp,
           PIN_FLD_GGSN_ADDRESS, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,64,ebufp);

        /*
         * SERVER TYPE
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(wap_info_flistp,
           PIN_FLD_SERVER_TYPE, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,64,ebufp);

        /*
         * CHARGING ID
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(wap_info_flistp,
           PIN_FLD_CHARGING_ID, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_char,10,ebufp);

        /*
         * WAP LOGIN
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(wap_info_flistp,
           PIN_FLD_LOGIN, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,24,ebufp);

        /*
         * End of record (line) indicator
         */
        addStr(edr, EOL_DELIMITER, ebufp);

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, (char *)edr->data);

        return;
        /*****/

PROCESS_FILE_ERROR:

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, 
            (char *)"*** MISSING ASSOC WAP INFO FIELDS - UNABLE TO PROCESS ***", 
            wap_flistp);

ErrNullPointer:
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "processSessionAssocWap error", ebufp);
                return;
                /*****/
        }
}
/****************************************************************************
 * Function     : processSessionBasicDetailTelcoGprs
 * Description  : Builds the report record for a basic detail record
 *--------------+------------------------------------------------------------
 * In           : pin_buf_t	*edr
 * In           : pin_flist_t	*gprs_flistp
 * In           : int32 number_assoc_recs
 * In           : const char *record_type
 * In		: pin_errbuf_t	*ebufp
 *--------------+------------------------------------------------------------
 * Returns      : void
 ****************************************************************************/
static void
processSessionBasicDetailTelcoGprs(
	pin_buf_t	*edr,
	pin_flist_t *gprs_flistp,
        int32   number_assoc_recs,
        const char *record_type,
	pin_errbuf_t	*ebufp)
{
        pin_flist_t	*gprs_info_flistp = NULL;
	pin_flist_t	*telco_info_flistp = NULL;
        pin_flist_t	*bal_impact_flistp = NULL;
                
        int32           record_id = 0;
        pin_cookie_t	cookie_id = NULL;
                
        char          generic_conv[256];
	char*         generic_decimal_str = NULL;
        int32           *generic_int = NULL;
        char            *generic_char = NULL;
        time_t          *generic_time = NULL;
        time_t          *generic_time2 = NULL;
        pin_decimal_t   *generic_decimal= NULL;

        int32           num_impact_packets = 0;
        int32           num_mon_impact_packets = 0;
        char            integrate_not_used[2] = "0";
        
        if (PIN_ERR_IS_ERR(ebufp)) {
		return;
                /*****/
	}

        gprs_info_flistp = PIN_FLIST_ELEM_GET_NEXT(gprs_flistp,
            PIN_FLD_GPRS_INFO, &record_id, 1, &cookie_id, ebufp);
		telco_info_flistp = PIN_FLIST_SUBSTR_GET(gprs_flistp, 
			PIN_FLD_TELCO_INFO, 1, ebufp);

	if (!gprs_info_flistp || !telco_info_flistp) {

              goto PROCESS_FILE_ERROR;
              /**********************/
        }

        record_id = 0;
        cookie_id = NULL;

        bal_impact_flistp = PIN_FLIST_ELEM_GET_NEXT(gprs_flistp,
            PIN_FLD_BAL_IMPACTS, &record_id, 1, &cookie_id, ebufp);

	if (bal_impact_flistp == (pin_flist_t *)NULL) {

              goto PROCESS_FILE_ERROR;
              /**********************/
        }
	
        /*
         * Construct the string in the correct order
         * Spec: sol42 CDR/EDR/IDR Format 6.3
         */
        
        /*
         * RECORD TYPE 
         */
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)record_type,3,ebufp);

        /*
         * RECORD NUMBER 
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)RECORD_PLACE_HOLDER,9,ebufp);
        
        /**
         * RA Changes - Start
        **/

        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_flistp,
           PIN_FLD_EVENT_NO, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr, 
		(char *)FILE_FLD_FORMAT_9,(char *)generic_char,60, ebufp);

        addStr(edr, DELIMITER, ebufp);

        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_flistp,
           PIN_FLD_ORIGINAL_BATCH_ID, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr, 
		(char *)FILE_FLD_FORMAT_9,(char *)generic_char,80, ebufp);

        /**
         * RA Changes - End
        **/

        /*
         * DISCARDING - DEFAULT OF 0
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,1,ebufp);
        
        /*
         * CHAIN REFERENCE - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * SOURCE NETWORK TYPE - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * SOURCE NETWORK - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * DESTINATION NETWORK TYPE - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * DESTINATION NETWORK - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * TYPE OF A-IDENTIFICATION - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * A-MODIFICATION INDICATOR - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * A-TYPE OF NUMBER - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)integrate_not_used,1,ebufp);

        /*
         * A-NUMBERING PLAN - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * A-NUMBER 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_PRIMARY_MSID, 0, ebufp);
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,40,ebufp);

        /*
         * B-MODIFICATION INDICATOR - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * B-TYPE OF NUMBER - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)integrate_not_used,1,ebufp);

        /*
         * B-NUMBERING PLAN - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * B-NUMBER - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * DESCRIPTION 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_flistp,
           PIN_FLD_DESCR, 0, ebufp);
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,50,ebufp);

        /*
         * C-MODIFICATION INDICATOR - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * C-TYPE OF NUMBER - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)integrate_not_used,1,ebufp);

        /*
         * C-NUMBERING PLAN - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * C-NUMBER - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * USAGE DIRECTION - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * CONNECT TYPE 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_ANONYMOUS_LOGIN, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_ANONYMOUS_LOGIN, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,2,ebufp);

        /*
         * CONNECT SUB TYPE - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * BASIC SERVICE 
         */
	generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_SVC_TYPE, 0, ebufp);
        if ( !generic_char ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_SVC_TYPE, 0, 0);
                goto ErrNullPointer;
        }
        strcpy(generic_conv, generic_char);

        generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_SVC_CODE, 0, ebufp);
        if (!generic_char) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_NULL_PTR, PIN_FLD_SVC_CODE, 0, 0);
                goto ErrNullPointer;
        }

        strcat(generic_conv, generic_char);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,3,ebufp);

        
        /*
         * QUALITY OF SERVICE REQUESTED - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * QUALITY OF SERVICE USED - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * CALL COMPLETION INDICATOR 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_TERMINATE_CAUSE, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_TERMINATE_CAUSE, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv, "%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr, 
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,3, ebufp);

        /*
         * LONG DURATION INDICATOR - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         *  CHARGING START TIMESTAMP
         */
        generic_time = (time_t *)PIN_FLIST_FLD_GET(gprs_flistp,
           PIN_FLD_START_T, 0, ebufp);
        if ( !generic_time ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_START_T, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_time);
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)generic_conv,
            strlen(generic_conv),ebufp);
        
        /*
         *  TRACK THE EFFECT ON THE TRAILER RECORD
         */
        /*analyzeChargingStart(*generic_time);
	*/

        /*
         *  CHARGING END TIMESTAMP
         */
        generic_time = (time_t *)PIN_FLIST_FLD_GET(gprs_flistp,
           PIN_FLD_END_T, 0, ebufp);
        if ( !generic_time ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_END_T, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_time);
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)generic_conv,
            strlen(generic_conv),ebufp);

        /*
         *  CREATED TIMESTAMP
         */
        generic_time = (time_t *)PIN_FLIST_FLD_GET(gprs_flistp,
           PIN_FLD_CREATED_T, 0, ebufp);
        if ( !generic_time ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_CREATED_T, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_time);
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)generic_conv,
            strlen(generic_conv),ebufp);

        /*
         * UTC TIME OFFSET 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_flistp,
           PIN_FLD_TIMEZONE_ID, 0, ebufp);
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,5,ebufp);

        /*
         *  DURATION
         */
        addStr(edr, DELIMITER, ebufp);
        generic_time  = (time_t *)PIN_FLIST_FLD_GET(gprs_flistp, 
                                                    PIN_FLD_START_T, 0, ebufp);
        if ( !generic_time ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_START_T, 0, 0);
                goto ErrNullPointer;
        }

        generic_time2 = (time_t *)PIN_FLIST_FLD_GET(gprs_flistp, 
                                                    PIN_FLD_END_T, 0, ebufp);
        if ( !generic_time2 ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_END_T, 0, 0);
                goto ErrNullPointer;
        }
        /* If end_t is greater or equal to start_t, 
         * then we have a valid scenario for calculating duration
         */
        if (*generic_time2 >= *generic_time)
        {
          int32 duration= *generic_time2 - *generic_time;
          sprintf(generic_conv,"%ld",duration);

        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,15,ebufp);
        }
        else
        {
          formatFieldsForFile(edr, (char *)FILE_FLD_FORMAT_9,
                             (char *)integrate_not_used,15,ebufp);
        }

        /*
         *  DURATION UoM
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)FIELD_DURATION_UOM,3,ebufp);
        
        /*
         * VOLUME SENT 
         */
        generic_decimal = (pin_decimal_t *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_BYTES_UPLINK, 0, ebufp);		
	if(generic_decimal) {
	  generic_decimal_str = pbo_decimal_to_str(generic_decimal,ebufp);
  	  addStr(edr, DELIMITER, ebufp);
          formatFieldsForFile(edr,
              (char *)FILE_FLD_FORMAT_9,(char *)generic_decimal_str,15,ebufp);
        }
        else
        {
          addStr(edr, DELIMITER, ebufp);
          formatFieldsForFile(edr, (char *)FILE_FLD_FORMAT_9,
                             (char *)integrate_not_used,15,ebufp);
        }

        /*
         *  VOLUME SENT UOM
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)FIELD_VOLUME_SENT_UOM,3,ebufp);

        /*
         * VOLUME RECEIVED 
         */
        generic_decimal = (pin_decimal_t *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_BYTES_DOWNLINK, 0, ebufp);		
	if(generic_decimal) {
		generic_decimal_str = pbo_decimal_to_str(generic_decimal,ebufp);        
		addStr(edr, DELIMITER, ebufp);
        	formatFieldsForFile(edr,
            		(char *)FILE_FLD_FORMAT_9,(char *)generic_decimal_str,15,ebufp);
        }
        else
        {
          addStr(edr, DELIMITER, ebufp);
          formatFieldsForFile(edr, (char *)FILE_FLD_FORMAT_9,
                             (char *)integrate_not_used,15,ebufp);
        }

        /*
         *  VOLUME RECEIVED UOM
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)FIELD_VOLUME_RECEIVED_UOM,3,
		ebufp);

        /*
         * NUMBER OF UNITS 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_NUMBER_OF_UNITS, 1, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_NUMBER_OF_UNITS, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,15,ebufp);

        /*
         *  NUMBER OF UNITS UoM
         */
	addStr(edr, DELIMITER, ebufp);
	formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)FIELD_NUMBER_OF_UNITS_UOM,
		3,ebufp);

        /*
         * RETAIL IMPACT CATEGORY - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * RETAIL CHARGED AMOUNT VALUE - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,11,ebufp);

        /*
         * RETAIL CHARGED AMOUNT CURRENCY - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * RETAIL CHARGED TAX TREATMENT - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * RETAIL CHARGED TAX RATE - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,4,ebufp);

        /*
         * WHOLESALE IMPACT CATEGORY - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * WHOLESALE CHARGED AMOUNT VALUE - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,11,ebufp);

        /*
         * WHOLESALE CHARGED AMOUNT CURRENCY - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * WHOLESALE CHARGED TAX TREATMENT - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * WHOLESALE CHARGED TAX RATE - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,4,ebufp);

        /*
         * ZONE DESCRIPTION 
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * IC DESCRIPTION 
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * ZONE ENTRY NAME
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * TARIFF CLASS - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);
        
        /*
         * TARIFF SUB CLASS - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * USAGE CLASS 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_USAGE_CLASS, 0, ebufp);
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,5,ebufp);

        /*
         * USAGE TYPE 
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_USAGE_TYPE, 0, ebufp);
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,5,ebufp);

        /*
         * BILLCYCLE PERIOD - NOT USED
         */
	addStr(edr, DELIMITER, ebufp);

        /*
         * PREPAID INDICATOR - DEFAULT
         */
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,2,ebufp);

        /*
         * NUMBER ASSOCIATED RECORDS
         */
        num_impact_packets = PIN_FLIST_ELEM_COUNT(gprs_flistp, 
            PIN_FLD_BAL_IMPACTS, ebufp);
        number_assoc_recs += num_impact_packets;
        num_mon_impact_packets = PIN_FLIST_ELEM_COUNT(gprs_flistp, 
            PIN_FLD_MONITOR_IMPACTS, ebufp);
        number_assoc_recs += num_mon_impact_packets;
        sprintf(generic_conv, "%ld",number_assoc_recs);
	addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,2,ebufp);

        /*
         * NETWORK ELEMENT START DATE/TIME STAMP - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * NETWORK ELEMENT END DATE/TIME STAMP - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * NETWORK ELEMENT START DATE/UTC TIME OFFSET - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * NETWORK ELEMENT END DATE/UTC TIME OFFSET - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);


        /*
         * UTC END TIME OFFSET - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * INCOMING ROUTE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);


        /*
         * ROUTING CATEGORY - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * INTERN PROCESS STATUS - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * DROPPED CALL QUANTITY - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)integrate_not_used,1,ebufp);

        /*
         * DROPPED CALL STATUS - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_Z,(char *)integrate_not_used,1,ebufp); 

        /*
         * End of record (line) indicator
         */
	addStr(edr, EOL_DELIMITER, ebufp);

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, (char *)edr->data);
	        
        /*
         * Clean up
         */

        return;
        /*****/

PROCESS_FILE_ERROR:

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, 
            (char *)"*** MISSING GPRS INFO FIELDS - "
            "UNABLE TO PROCESS ***", 
            gprs_flistp);

        /*
         * Clean up
         */

ErrNullPointer:
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        " processSessionBasicDetailTelcoGprs error", ebufp);
                return;
                /*****/
        }
}
/****************************************************************************
 * Function     : processSessionAssocTelcoGprs
 * Description  : Builds the report record for an associated 
 *              : GPRS extension record
 *--------------+------------------------------------------------------------
 * In/Out       : pin_buf_t	*edr
 * In           : pin_flist_t	*gprs_flistp
 * In           : pin_errbuf_t *ebufp
 *--------------+------------------------------------------------------------
 * Returns      : void
 ****************************************************************************/
static void
processSessionAssocTelcoGprs(
	pin_buf_t	*edr,
	pin_flist_t	*gprs_flistp,
	pin_errbuf_t	*ebufp)
{
        pin_flist_t	*gprs_info_flistp = NULL;
		pin_flist_t	*telco_info_flistp = NULL;
        
        int32           record_id = 0;
        pin_cookie_t	cookie_id = NULL;
        int32           num_service_codes = 0;
        
        char            generic_conv[256];
        int32           *generic_int = NULL;
        char            *generic_char = NULL;
        time_t          *generic_time = NULL;
        pin_decimal_t   *generic_decimal= NULL;
        char            integrate_not_used[2] = "0";

        if (PIN_ERR_IS_ERR(ebufp)) {
		return;
                /*****/
	}

        gprs_info_flistp = PIN_FLIST_SUBSTR_GET(gprs_flistp,
            PIN_FLD_GPRS_INFO, 1, ebufp);

        telco_info_flistp = PIN_FLIST_SUBSTR_GET(gprs_flistp,
            PIN_FLD_TELCO_INFO, 1, ebufp);

	if (!gprs_info_flistp || !telco_info_flistp) {

              goto PROCESS_FILE_ERROR;
              /**********************/
        }	

	/*
         * Construct the flist in the correct order
         * Spec: sol42 CDR/EDR/IDR Format 6.3
         */
        
        /*
         * RECORD TYPE 
         */
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)RECORD_TYPE_GPRS_EXT,3,ebufp);

        /*
         * RECORD NUMBER 
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)RECORD_PLACE_HOLDER,9,ebufp);

        /*
         * PORT NUMBER
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(telco_info_flistp,
           PIN_FLD_SECONDARY_MSID, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,24,ebufp);

        /*
         * DEVICE NUMBER
         */        
		addStr(edr, DELIMITER, ebufp);

        /*
         * A-NUMBER USER - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * DIALED DIGITS - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * VAS/PRODUCT CODE - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * ORIGINATING SWITCH ID - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * TERMINATING SWITCH ID - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * MS CLASS MARK - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)integrate_not_used,1,ebufp);

        /*
         * ROUTING AREA
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_ROUTING_AREA, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,10,ebufp);

        /*
         * LOCATION AREA INDICATOR
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_LOC_AREA_CODE, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,10,ebufp);

        /*
         * CHARGING ID 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_SESSION_ID, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_SESSION_ID, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,10,ebufp);

        /*
         * SGSN ADDRESS
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_SGSN_ADDRESS, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,64,ebufp);

        /*
         * GGSN ADDRESS
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_GGSN_ADDRESS, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,64,ebufp);
	
        /*
         * APN ADDRESS
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_APN, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,64,ebufp);

        /*
         * NODE ID
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_NODE_ID, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,64,ebufp);

        /*
         * TRANS ID
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_TRANS_ID, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_char,10,ebufp);

        /*
         * SUB TRANS ID
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_SUB_TRANS_ID, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_char,10,ebufp);

        /*
         * NETWORK INITIATED PDP 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_NI_PDP, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_NI_PDP, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,1,ebufp);

        /*
         * PDP TYPE
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_PDP_TYPE, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,4,ebufp);

        /*
         * PDP ADDRESS
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_PDP_ADDRESS, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,64,ebufp);

        /*
         * PDP REMOTE ADDRESS
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_PDP_RADDRESS, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,255,ebufp);

        /*
         * PDP DYNAMIC ADDRESS 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_PDP_DYNADDR, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_PDP_DYNADDR, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,1,ebufp);

        /*
         * DIAGNOSTICS
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_DIAGNOSTICS, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,255,ebufp);

        /*
         * CELL ID
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_CELL_ID, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,10,ebufp);

        /*
         * CHANGE CONDITION 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_CHANGE_CONDITION, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_CHANGE_CONDITION, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,1,ebufp);

        /*
         * QoS REQUESTED PRECEDENCE 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_QOS_REQ_PRECEDENCE, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_QOS_REQ_PRECEDENCE, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,1,ebufp);

        /*
         * QoS REQUESTED DELAY 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_QOS_REQ_DELAY, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_QOS_REQ_DELAY, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,1,ebufp);

        /*
         * QoS REQUESTED RELIABILITY 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_QOS_REQ_RELIABILITY, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_QOS_REQ_RELIABILITY, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,1,ebufp);

        /*
         * QoS REQUESTED PEAK THROUGHPUT 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_QOS_REQ_PEAK_THROUGH, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_QOS_REQ_PEAK_THROUGH, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,2,ebufp);

        /*
         * QoS REQUESTED MEAN THROUGHPUT 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_QOS_REQ_MEAN_THROUGH, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_QOS_REQ_MEAN_THROUGH, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,2,ebufp);

        /*
         * QoS USED PRECEDENCE 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_QOS_NEGO_PRECEDENCE, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_QOS_NEGO_PRECEDENCE, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,1,ebufp);
        
        /*
         * QoS USED DELAY 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_QOS_NEGO_DELAY, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_QOS_NEGO_DELAY, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,1,ebufp);

        /*
         * QoS USED RELIABILITY 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_QOS_NEGO_RELIABILITY, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_QOS_NEGO_RELIABILITY, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,1,ebufp);

        /*
         * QoS USED PEAK THROUGHPUT 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_QOS_NEGO_PEAK_THROUGH, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_QOS_NEGO_PEAK_THROUGH, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,2,ebufp);

        /*
         * QoS USED MEAN THROUGHPUT 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_QOS_NEGO_MEAN_THROUGH, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_QOS_NEGO_MEAN_THROUGH, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_conv,2,ebufp);

        /*
         * NETWORK CAPABILITY
         */
        generic_char = (char *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_NETWORK_CAPABILITY, 0, ebufp);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_X,(char *)generic_char,10,ebufp);

        /*
         * SGSN CHANGE 
         */
        generic_int = (int32 *)PIN_FLIST_FLD_GET(gprs_info_flistp,
           PIN_FLD_SGSN_CHANGE, 0, ebufp);
        if ( !generic_int ) {
                pin_set_err(ebufp, PIN_ERRLOC_APP,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NULL_PTR, PIN_FLD_SGSN_CHANGE, 0, 0);
                goto ErrNullPointer;
        }
        sprintf(generic_conv,"%ld",*generic_int);
        addStr(edr, DELIMITER, ebufp);
        formatFieldsForFile(edr,
            (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,1,ebufp);

        /*
         * SEQUENCE NUMBER OF FIRST DATA EVENT - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * SEQUENCE NUMBER OF LAST DATA EVENT - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);


        /*
         * B PARTY CELL ID - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * LAST CELL ID - NOT USED
         */
        addStr(edr, DELIMITER, ebufp);

        /*
         * End of record (line) indicator
         */
        addStr(edr,EOL_DELIMITER,ebufp);

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, (char *)edr->data);
        
        return;
        /*****/

PROCESS_FILE_ERROR:

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, 
            (char *)"*** MISSING ASSOC GPRS INFO FIELDS "
            "- UNABLE TO PROCESS ***", 
            gprs_flistp);

        return;
        /*****/

ErrNullPointer:
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        " processSessionAssocTelcoGprs error", ebufp);
                return;
                /*****/
        }
}

/*******************************************************************
 * addStr():
 *
 * Function to keep adding to buffer with given string as input.
 * The pin_buf_t structure is reused here. 
 * The length of the buffer is tracked with the "size" variable.
 * "offset" is used to find out how much is currenlty used. pin_realloc
 * is used when needed to lengthen the buf, and the "size" will be
 * updated with the new length.
 *
 *******************************************************************/
static void
addStr(
        pin_buf_t  		*bufp,
        char                   *string,
        pin_errbuf_t           *ebufp )
{
        char                    *cp = NULL;
        char                    *op = NULL;

        if( PIN_ERR_IS_ERR( ebufp )) {
                return;
        }

        if( bufp == NULL || string == NULL ) {
                return;
        }

        /*********************************************************
         * If strp is still null, need to allocate first block
         *********************************************************/

        if( bufp->data == NULL ) {
                bufp->data = (char *)pin_malloc( CHUNKSIZE );
                if (bufp->data == NULL) {
                        pin_set_err(ebufp, PIN_ERRLOC_FM,
                                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                                PIN_ERR_NO_MEM, 0, 0, 0);
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "Memory for buffer couldn't be allocated",
                                ebufp);
                        goto ErrOut;
                }

                memset( bufp->data, 0, CHUNKSIZE);
                bufp->size = CHUNKSIZE;
                bufp->offset = 0;
        }

        /***********************************************************
         * Now start copying the characters one at a time from the
         * input string to the bufp->strp buffer.  If we run out
         * of room in the bufp->strp, then realloc some more mem
         * and continue.  Keep track of size and strsize as we go.
         ***********************************************************/

        cp = string;

        /* calc starting point of output buffer */
        op = (char *)(bufp->data + bufp->offset );

        /* copy everything over */
        while( *cp != '\0' ) {

                /* if the string size has exceeded limit
                 * (use fudge factor of 50)
                 * Then realloc more memory
                 */
                if( bufp->offset > bufp->size - 50 ) {

                        bufp->size = bufp->size + CHUNKSIZE;
                        bufp->data = (char *)pin_realloc( bufp->data,
                                                          bufp->size );
			op = (char *)(bufp->data + bufp->offset );
                        if (bufp->data == NULL) {
                                pin_set_err(ebufp, PIN_ERRLOC_FM,
                                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                                        PIN_ERR_NO_MEM, 0, 0, 0);
                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "Memory for buffer couldn't be allocated",
                                                 ebufp);
                                goto ErrOut;
                        }
                }

                /* now copy the character  and also null terminator
                 * increment the local char pointers and also the
                 * offset
                 */

                *op = *cp;
                cp++;
                op++;
                *op = '\0';
                bufp->offset++;
        }

        /* string should be copie now */
        return;
ErrOut:
        return;
}


/****************************************************************************
 * Function     : formatFieldsForFile
 * Description  : Takes a format specifier, some data and the max length 
 *              :     and returns the formatted data.  The data can be either
 *              :     fixed witdh (the lenght specifier) or variable length
 *              :     determined by the member variable m_isFixedWidth.
 *--------------+------------------------------------------------------------
 * In           : pin_buf_t  *edr
 * In           : char*  format
 * In           : char*  data
 * In           : long    length
 * In           : pin_errbuf_t    *ebufp
 *--------------+------------------------------------------------------------
 * Returns      : void
 ****************************************************************************/
static void 
formatFieldsForFile(
	pin_buf_t *edr,
	char *format, 
        char *data, 
        long length,
	pin_errbuf_t	*ebufp)
{

	char 	debugMsg[1024];
	char 	buf[256];
	int32	isFixedWidth = 0;
	int32	counter = 0;

        if(!data) {
                return;
        }
        if (PIN_ERR_IS_ERR(ebufp)) {
                return;
                /*****/
        }
        
	pin_snprintf(debugMsg,sizeof(debugMsg), "\nArg=%s\nFormat=%s\nLength=%ld\n", 
		data, format, length);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, (char *)debugMsg);

	memset(buf, 0, 256);

        if (strcmp(format, FILE_FLD_FORMAT_9) == 0) {
                /*
                 * Numeric, right-justified, filled with
                 * leading zeros to the left
                 */
                if (isFixedWidth) {
			sprintf(buf, "%2$.*1$d", length, atoi(data));
			addStr(edr, buf, ebufp);
                } else {
			addStr(edr, data, ebufp);
                }
        } else if ( strcmp(format, FILE_FLD_FORMAT_X) == 0 ||
                    strcmp(format, FILE_FLD_FORMAT_Z) == 0 ) {
                /*
                 * Alphanumeric, left-justified, filled with
                 * trailing spaces to the right.
                 * OR
                 * Numeric, left-justified, filled with
                 * trailing spaces to the right.
                 */
                addStr(edr, data, ebufp);
                if (isFixedWidth) {
			memset(buf, ' ', length);
			addStr(edr, buf, ebufp);
                }
        } else if (strcmp(format, FILE_FLD_FORMAT_H) == 0) {
                /*
                 * Hexadecimal value (0-9, A-F), right-justified,
                 * filled with leading zeros to the left.
                 */
                long    num = atol(data);
                if (isFixedWidth) {
			sprintf(buf, "%0*lx", length, num);
			addStr(edr, buf, ebufp);
			for (counter = 0; counter < edr->offset; counter++) {
				edr->data[edr->offset] = toupper(
					edr->data[edr->offset]);
			}
                } else {
			sprintf(buf, "%lx", num);
			addStr(edr, buf, ebufp);
                }
        } else {
                /*
                 * Default is left justified filled with
                 * trailing spaces to the right
                 */
		addStr(edr, data, ebufp);
                if (isFixedWidth) {
			memset(buf, ' ', length);
			addStr(edr, buf, ebufp);
                }
        }
}

/****************************************************************************
 * Function     : processSessionSubBalanceImpacts
 * Description  : Builds the report record for SubBalances Impacts (605)packets
 *--------------+------------------------------------------------------------
 * In/Out       : pin_buf_t	*edr
 * In           : pin_flist_t  *flistp
 * In           : pin_errbuf_t *ebufp
 *--------------+------------------------------------------------------------
 * Returns      : void
 ****************************************************************************/

static void
processSessionSubBalanceImpacts(
	pin_buf_t	*edr,
	pin_flist_t	*flistp,
	pin_errbuf_t	*ebufp)
{
        pin_flist_t     *event_packet_flistp = NULL;
        
        int32           record_id = 0;
        pin_cookie_t	cookie_id = NULL;
        
        char            generic_conv[256];
        int32           *generic_int = NULL;
        char            *generic_char = NULL;
        time_t          *generic_time = NULL;
        pin_decimal_t   *generic_decimal = NULL;
        poid_t          *generic_poid = NULL;

        int32	        maxpoidlen = 0;
	    char	        poid_string[100];
        
        if (PIN_ERR_IS_ERR(ebufp)) {
		return;
                /*****/
	}

        /*
         * Construct the flist in the correct order
         * Spec: sol42 CDR/EDR/IDR Format 6.3
         */

           while ((event_packet_flistp = PIN_FLIST_ELEM_GET_NEXT(flistp,
	            PIN_FLD_SUB_BAL_IMPACTS, &record_id, 1, &cookie_id,
	            ebufp)) != (pin_flist_t *)NULL) {
        
                /*
                 * RECORD TYPE 
                 */
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_9,(char *)RECORD_TYPE_SUB_BAL_IMPACTS,
			    3,ebufp);

                /*
                 * RECORD NUMBER 
                 */
                addStr(edr, DELIMITER, ebufp);
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_9,(char *)RECORD_PLACE_HOLDER,
			     9, ebufp);
                
                

                /*
                 * PIN BAL_GRP POID 
                 */
                generic_poid = (char *)PIN_FLIST_FLD_GET(
                    event_packet_flistp,PIN_FLD_BAL_GRP_OBJ, 0, ebufp);
                if ( !generic_poid ) {
                        pin_set_err(ebufp, PIN_ERRLOC_APP,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_NULL_PTR, PIN_FLD_BAL_GRP_OBJ, 0, 0);
                        goto ErrNullPointer;
                }
                memset(poid_string,0,sizeof(poid_string));
                generic_char = poid_string;
                maxpoidlen = sizeof(poid_string);
                PIN_POID_TO_STR((poid_t *)generic_poid, &generic_char, 
                    &maxpoidlen, ebufp);
                addStr(edr, DELIMITER, ebufp);
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_X,(char *)generic_char+6,255,ebufp);

               

                /*
                 * PIN RESOURCE ID 
                 */             
                generic_int = (int32 *)PIN_FLIST_FLD_GET(
                    event_packet_flistp,
                    PIN_FLD_RESOURCE_ID, 0, ebufp);
                if ( !generic_int ) {
                        pin_set_err(ebufp, PIN_ERRLOC_APP,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_NULL_PTR, PIN_FLD_RESOURCE_ID, 0, 0);
                        goto ErrNullPointer;
                }
                sprintf(generic_conv,"%ld",*generic_int);
                addStr(edr, DELIMITER, ebufp);
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,9,ebufp);
                                 
               
                /*
                 * End of record (line) indicator
                 */
                addStr(edr, EOL_DELIMITER, ebufp);

                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, (char *)edr->data);
				
		processSessionSubBalances(edr, event_packet_flistp, RECORD_TYPE_SUBBAL, ebufp);
        }

ErrNullPointer:
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        " processSessionSubBalanceImpacts error", ebufp);
                return;
                /*****/
        }
}

/****************************************************************************
 * Function     : processSessionMonitorSubBalanceImpacts
 * Description  : Builds the report record for SubBalances Impacts (805)packets
 *--------------+------------------------------------------------------------
 * In/Out       : pin_buf_t *edr
 * In           : pin_flist_t  *flistp
 * In           : pin_errbuf_t *ebufp
 *--------------+------------------------------------------------------------
 * Returns      : void
 ****************************************************************************/

static void
processSessionMonitorSubBalanceImpacts(
    pin_buf_t   *edr,
    pin_flist_t *flistp,
    pin_errbuf_t    *ebufp)
{
        pin_flist_t     *event_packet_flistp = NULL;

        int32           record_id = 0;
        pin_cookie_t    cookie_id = NULL;

        char            generic_conv[256];
        int32           *generic_int = NULL;
        char            *generic_char = NULL;
        time_t          *generic_time = NULL;
        pin_decimal_t   *generic_decimal = NULL;
        poid_t          *generic_poid = NULL;

        int32           maxpoidlen = 0;
        char            poid_string[100];

        if (PIN_ERR_IS_ERR(ebufp)) {
        return;
                /*****/
        }

        /*
         * Construct the flist in the correct order
         * Spec: sol42 CDR/EDR/IDR Format 6.3
         */

           while ((event_packet_flistp = PIN_FLIST_ELEM_GET_NEXT(flistp,
                PIN_FLD_MONITOR_SUB_BAL_IMPACTS, &record_id, 1, &cookie_id,
                ebufp)) != (pin_flist_t *)NULL) {

                /*
                 * RECORD TYPE
                 */
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_9,(char *)RECORD_TYPE_MONITOR_SUB_BAL_IMPACTS,
                3,ebufp);

                /*
                 * RECORD NUMBER
                 */
                addStr(edr, DELIMITER, ebufp);
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_9,(char *)RECORD_PLACE_HOLDER,
                 9, ebufp);



                /*
                 * PIN BAL_GRP POID
                 */
                generic_poid = (char *)PIN_FLIST_FLD_GET(
                    event_packet_flistp,PIN_FLD_BAL_GRP_OBJ, 0, ebufp);
                if ( !generic_poid ) {
                        pin_set_err(ebufp, PIN_ERRLOC_APP,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_NULL_PTR, PIN_FLD_BAL_GRP_OBJ, 0, 0);
                        goto ErrNullPointer;
                }
                memset(poid_string,0,sizeof(poid_string));
                generic_char = poid_string;
                maxpoidlen = sizeof(poid_string);
                PIN_POID_TO_STR((poid_t *)generic_poid, &generic_char,
                    &maxpoidlen, ebufp);
                addStr(edr, DELIMITER, ebufp);
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_X,(char *)generic_char+6,255,ebufp);



                /*
                 * PIN RESOURCE ID
                 */
                generic_int = (int32 *)PIN_FLIST_FLD_GET(
                    event_packet_flistp,
                    PIN_FLD_RESOURCE_ID, 0, ebufp);
                if ( !generic_int ) {
                        pin_set_err(ebufp, PIN_ERRLOC_APP,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_NULL_PTR, PIN_FLD_RESOURCE_ID, 0, 0);
                        goto ErrNullPointer;
                }
                sprintf(generic_conv,"%ld",*generic_int);
                addStr(edr, DELIMITER, ebufp);
                formatFieldsForFile(edr,
                    (char *)FILE_FLD_FORMAT_9,(char *)generic_conv,9,ebufp);


                /*
                 * End of record (line) indicator
                 */
                addStr(edr, EOL_DELIMITER, ebufp);
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, (char *)edr->data);
   
        processSessionSubBalances(edr, event_packet_flistp, RECORD_TYPE_MONITOR_SUBBAL, ebufp);
        }

ErrNullPointer:
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        " processSessionMonitorSubBalanceImpacts error", ebufp);
                return;
                /*****/
        }
}

