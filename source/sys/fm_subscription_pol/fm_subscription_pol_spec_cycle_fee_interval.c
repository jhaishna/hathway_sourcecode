/**********************************************************************
 *
 * Copyright (c) 1999, 2009, Oracle and/or its affiliates.All rights reserved. 
 *      
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 **********************************************************************/

#ifndef lint
static char Sccs_id[] = "@(#)$Id: fm_subscription_pol_spec_cycle_fee_interval.c /cgbubrm_7.3.2.rwsmod/1 2009/03/24 07:02:26 amamidi Exp $";
#endif

/*******************************************************************
 * Contains the PCM_OP_SUBSCRIPTION_POL_SPEC_CYCLE_FEE_INTERVAL operation. 
 *******************************************************************/

#include <stdio.h> 
#include <string.h> 
 
#include "pcm.h"
#include "ops/bill.h"
#include "ops/subscription.h"
#include "ops/bal.h"
#include "pin_bill.h"
#include "pin_rate.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "mso_ops_flds.h"
#include "mso_cust.h"
#include "mso_prov.h"
#include "fm_bal.h"

#define POSTPAID 0
#define ADVANCE  1
#define PREPAID	 2
#define CATV_PREPAID 1


int postpaid_grants_priorities[13] = {510,540,570,600,630,660,690,720,750,780,810,840};

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_subscription_pol_spec_cycle_fee_interval(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_subscription_pol_spec_cycle_fee_interval(
	cm_nap_connection_t     *connp,
	pcm_context_t		*ctxp,
	cm_op_info_t		*opstackp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void 
mso_fm_cycle_fee_evt_type(
	pcm_context_t		*ctxp,
	char			*event_type, 
	time_t			*charged_from_t,
	time_t			*charged_to_t,
	int32			is_charged_t_bdom,
	pin_errbuf_t		*ebufp);

static int32 
fm_mso_modify_for_mcf(
	pcm_context_t		*ctxp,
	pin_flist_t		**i_flistp,
	pin_flist_t		**r_flistpp,
	cm_op_info_t		*opstackp,
	pin_errbuf_t		*ebufp);

void read_prod_details(pcm_context_t		*ctxp,
		pin_flist_t		*i_flistp,
		pin_flist_t		**prod_flistp,
		pin_errbuf_t		*ebufp);

void fm_mso_update_service_fup_status(
		cm_nap_connection_t     *connp,
		pcm_context_t		*ctxp,
		pin_flist_t		*i_flistp,
		pin_flist_t		*prod_details_flistp,
		char			*event_type,
		cm_op_info_t		*opstackp,
		pin_flist_t		*r_flistp,
		pin_errbuf_t		*ebufp);

void create_lifecycle_event(
		pcm_context_t		*ctxp,
		pin_flist_t		*i_flistp,
		pin_flist_t		*prod_details_flistp,
		pin_errbuf_t		*ebufp);

int32 get_bill_when_from_service(
		pcm_context_t		*ctxp,
		poid_t			*svc_pdp,
		pin_errbuf_t		*ebufp);


void fm_mso_set_charged_to_t(
		pcm_context_t		*ctxp,
		pin_flist_t		*i_flistp,
		pin_flist_t		**r_flistpp,
		cm_op_info_t		*opstackp,
		pin_errbuf_t		*ebufp);


static void mso_cust_renew_quota(
		pcm_context_t		*ctxp,
		poid_t		*acct_pdp,
		char		*action,
		poid_t		*svc_pdp,
		poid_t		*p_pdp,
		poid_t		*purch_pdp,
		int32             flag,
		pin_flist_t	*r_flistp,
		pin_errbuf_t		*ebufp);

int is_autorenew_flow(
		cm_nap_connection_t     *connp,
		cm_op_info_t *opstackp,
		pin_errbuf_t     *ebufp);

static void 
update_for_topup(	
		pcm_context_t	*ctxp, 
		cm_op_info_t	*opstackp,
		char		*event_type,
		pin_flist_t	*i_flistp, 
		pin_flist_t	**r_flistpp, 
		pin_errbuf_t	*ebufp);

static void
fm_mso_cust_bb_hw_amc_get_cycle_details(
        pcm_context_t   *ctxp,
        pin_flist_t      *i_flistp,
        pin_errbuf_t    *ebufp);

static void
get_last_plan_valid_to_fup(
	pcm_context_t           *ctxp,
	poid_t                  *acc_obj,
	poid_t                  *svc_obj,
	time_t			*fup_valid_to,
	pin_errbuf_t            *ebufp);
	
extern int32
fm_mso_get_product_priority(
        pcm_context_t           *ctxp,
        poid_t                  *prod_pdp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

extern void
fm_mso_prov_update_product_cycle_fee_charge_dates(
        pcm_context_t   *ctxp,
        poid_t          *acct_pdp,
        poid_t          *service_obj,
        poid_t          *offering_obj,
        pin_flist_t     **ret_flistpp,
        pin_errbuf_t    *ebufp);

PIN_IMPORT int32
fm_mso_catv_pt_pkg_type(
        pcm_context_t           *ctxp,
        poid_t                  *prd_pdp,
        pin_errbuf_t            *ebufp);

extern void
fm_mso_get_poid_details(
        pcm_context_t           *ctxp,
        poid_t                  *poid_pdp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

void
get_last_mso_purchased_plan_modified(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flist,
        poid_t                  *plan_pdp,
	int32			flag,
        pin_flist_t             **bb_ret_flistp,
        pin_errbuf_t            *ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_SUBSCRIPTION_POL_SPEC_CYCLE_FEE_INTERVAL operation.
 *******************************************************************/
void
op_subscription_pol_spec_cycle_fee_interval(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	void			*vp = NULL;

	pcm_context_t		*ctxp = connp->dm_ctx;
	cm_op_info_t		*opstackp = connp->coip;

	*r_flistpp = NULL;
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*
	 * Insanity check.
	 */
	if (opcode != PCM_OP_SUBSCRIPTION_POL_SPEC_CYCLE_FEE_INTERVAL) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_subscription_pol_spec_cycle_fee_interval bad opcode error", 
			ebufp);
		return;
	}

	/*
	 * Debug: What we got.
	 */
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_subscription_pol_spec_cycle_fee_interval input flist", i_flistp);

	/*
	 * Prepare return flist.
	 */
	*r_flistpp = PIN_FLIST_CREATE(ebufp);
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, vp, ebufp);

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CHARGED_FROM_T,
		0, ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CHARGED_FROM_T, 
		vp, ebufp);
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CHARGED_TO_T,
		0, ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CHARGED_TO_T, 
		vp, ebufp);
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SCALE, 1, ebufp);
	if (vp) {
        	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_SCALE,
                				vp, ebufp);
	}
        /*
	 * Error?
	 */
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"op_subscription_pol_spec_cycle_fee_interval error", ebufp);
		return;
	}

	/* calling facility function*/

	//fm_subscription_pol_spec_cycle_fee_interval(ctxp, opstackp, flags, i_flistp, r_flistpp, ebufp); 
	fm_subscription_pol_spec_cycle_fee_interval(connp, ctxp, opstackp, flags, i_flistp, r_flistpp, ebufp);


	/* Set the FUP_STATUS to BEFORE_FUP for FUP products here */

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"op_subscription_pol_spec_cycle_fee_interval error", ebufp);
	} else {

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_subscription_pol_spec_cycle_fee_interval output flist", *r_flistpp);
	}
	return;
}

/*******************************************************************
 * This is the custom implementation for this policy
 *******************************************************************/
static void 
fm_subscription_pol_spec_cycle_fee_interval(
	cm_nap_connection_t     *connp,
	pcm_context_t		*ctxp,
	cm_op_info_t		*opstackp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*readproduct_inflistp = NULL;
	pin_flist_t		*readproduct_outflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*prod_details_flistp = NULL;
	pin_flist_t		*ppo_iflistp = NULL;
	pin_flist_t		*ppo_oflistp = NULL;
	pin_flist_t             *cfee_flistp = NULL;
	pin_flist_t     	*profile_flistp = NULL;
	cm_op_info_t 		*tmp_opstackp = connp->coip;
	poid_t			*product_poid = NULL;
	poid_t			*svc_pdp = NULL;
	poid_t                  *acc_pdp = NULL;
	poid_t          	*pdt_pdp = NULL;
	
	char			*event_type = NULL;
	char			*svc_type = NULL;
	char            	*prod_namep = NULL;
	
	int			csr_access = 0;
	int			acct_update = 0;
	int32			modify = 0;
	int32			cancel = 0;
	int32           	*customer_typep = NULL;

	time_t                  *actual_charge_to = NULL;
	time_t                  act_charge_to = 0;
	time_t			*charged_from_t = NULL;
	time_t			*charged_to_t = NULL;
	time_t			*ppo_cet = NULL;
	time_t			*actg_next_t = NULL;
	int32			charged_day = 0;
	int32			actg_cycle_dom = 0;
	int32			is_charged_t_bdom = 0;
	int32			is_base_pack = -1;
	char			buf[60];	
	time_t			current_time = 0;
	char			*action = NULL;
	time_t			chrg_from = 0;	
	struct tm       	*timeeff;
 	time_t			*end_t = NULL;
	time_t                  *ppo_cst = NULL;
	time_t                  *actg_nxt_t = NULL;
	time_t                  *actual_chg_from_t = NULL;
	time_t                  ref_chg_from_t ;
	time_t                  new_charged_to_t;

	struct tm               ta;
	struct tm               tt;
	struct tm               *ts = NULL ;

	int32			*fup_status = NULL;
	int32			*indic = NULL;
	//Future dated renewal changes
	time_t                  *cf_end_t = NULL;
        time_t                  *pp_charged_to = NULL;
        pin_flist_t             *cfee_oflistp = NULL;
        int32                   future_flag = 0;

	//NTO changes
        time_t                  cur_t = pin_virtual_time ((time_t *)NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_subscription_pol_spec_cycle_fee_interval input flist", i_flistp);	
	
	product_poid = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
	acc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	if (!product_poid)
	{	
		return;
	}
  	actual_charge_to = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CHARGED_TO_T, 1, ebufp);
        act_charge_to = *actual_charge_to;	
	
	/*******************************************************************
	* Read product for the event type
	*******************************************************************/

	readproduct_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(readproduct_inflistp, PIN_FLD_POID, product_poid, ebufp );
	PIN_FLIST_FLD_SET(readproduct_inflistp, PIN_FLD_NAME, NULL, ebufp );
	args_flistp = PIN_FLIST_ELEM_ADD(readproduct_inflistp, PIN_FLD_USAGE_MAP, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_EVENT_TYPE, NULL, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_subscription_pol_spec_cycle_fee_interval read product input list", readproduct_inflistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, readproduct_inflistp, &readproduct_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&readproduct_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&readproduct_outflistp, NULL);	
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_subscription_pol_spec_cycle_fee_interval read product output flist", readproduct_outflistp);
	prod_namep = PIN_FLIST_FLD_GET(readproduct_outflistp, PIN_FLD_NAME, 0, ebufp);	
	
	args_flistp = PIN_FLIST_ELEM_GET(readproduct_outflistp, PIN_FLD_USAGE_MAP, 0, 1, ebufp );
	if (args_flistp)
	{
		event_type = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_EVENT_TYPE, 1, ebufp );
	}
	
 	/*******************************************************************
	* Modify Dates for Advanced Charging
	*******************************************************************/
	if(strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_grant") != 0) {
	    PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_EVENT_TYPE, event_type, ebufp);		
	    modify = fm_mso_modify_for_mcf(ctxp, &i_flistp, r_flistpp,opstackp,  ebufp);
	} else {
		fm_mso_set_charged_to_t(ctxp, i_flistp, r_flistpp, opstackp, ebufp);
	} 
	//    modify = fm_mso_modify_for_mcf(ctxp, &i_flistp, opstackp,  ebufp);
	if (modify)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_for_mcf return 1");
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_CYCLE_FEE_END_T, *r_flistpp, PIN_FLD_CYCLE_FEE_END_T, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_CHARGED_TO_T,    *r_flistpp, PIN_FLD_CHARGED_TO_T, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACTG_NEXT_T,     *r_flistpp, PIN_FLD_ACTG_NEXT_T, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACTG_FUTURE_T,   *r_flistpp, PIN_FLD_ACTG_FUTURE_T, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_CYCLE_FEE_START_T,  *r_flistpp, PIN_FLD_CYCLE_FEE_START_T, ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_subscription_pol_spec_cycle_fee_interval modified input flist", i_flistp);
	
	indic = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_INDICATOR, 1, ebufp);
	action = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTION, 1, ebufp);
	
	if((event_type && strstr(event_type, "mso_hw_rental/ip")) && (action && (strcmp(action, "CANCEL") == 0)))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Tal subscription cancel flow");	
		return;
	}
	if((event_type && strstr(event_type, "mso_hw_rental/ip")) && (indic && *indic == ADVANCE))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Tal subscription  flow returning if it is postpaid");
                return;
        }
	if ((event_type) && ((strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_annual/mso_sb_adn_fdp") == 0)
		|| (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_sb_adn_fdp") == 0)
		|| (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_bimonthly/mso_sb_adn_fdp") == 0)
		|| (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quarterly/mso_sb_adn_fdp") == 0)
		|| (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_semiannual/mso_sb_adn_fdp") == 0) 

		|| (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_sb_alc_fdp") == 0)
		|| (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_sb_pkg_fdp") == 0)
		|| (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_bimonthly/mso_sb_alc_fdp") == 0)
		|| (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_bimonthly/mso_sb_pkg_fdp") == 0)
		|| (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quarterly/mso_sb_alc_fdp") == 0)
		|| (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quarterly/mso_sb_pkg_fdp") == 0)
		|| (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_semiannual/mso_sb_alc_fdp") == 0)
		|| (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_semiannual/mso_sb_pkg_fdp") == 0)
		|| (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_annual/mso_sb_alc_fdp") == 0)
		|| (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_annual/mso_sb_pkg_fdp") == 0)

		|| ((strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_hw_rental",	
			strlen("/event/billing/product/fee/cycle/cycle_forward_monthly/mso_hw_rental")) == 0) &&
			/* below has been added to not to allow the catv flow to enter through this condition */
			(strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_hw_rental") != 0))
		
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quarterly/mso_hw_rental",	
			strlen("/event/billing/product/fee/cycle/cycle_forward_quarterly/mso_hw_rental")) == 0)

		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_semiannual/mso_hw_rental",	
			strlen("/event/billing/product/fee/cycle/cycle_forward_semiannual/mso_hw_rental")) == 0)

		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_annual/mso_hw_rental",	
			strlen("/event/billing/product/fee/cycle/cycle_forward_annual/mso_hw_rental")) == 0)

		/*|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_hw_amc",	
			strlen("/event/billing/product/fee/cycle/cycle_forward_monthly/mso_hw_amc")) == 0)
		
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quarterly/mso_hw_amc",	
			strlen("/event/billing/product/fee/cycle/cycle_forward_quarterly/mso_hw_amc")) == 0)
		
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_semiannual/mso_hw_amc",	
			strlen("/event/billing/product/fee/cycle/cycle_forward_semiannual/mso_hw_amc")) == 0)
		
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_annual/mso_hw_amc",	
			strlen("/event/billing/product/fee/cycle/cycle_forward_annual/mso_hw_amc")) == 0) */


		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly_fdp",	
			strlen("/event/billing/product/fee/cycle/cycle_forward_monthly_fdp")) == 0)

		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quarterly_fdp",	
			strlen("/event/billing/product/fee/cycle/cycle_forward_quarterly_fdp")) == 0)

		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_semiannual_fdp",	
			strlen("/event/billing/product/fee/cycle/cycle_forward_semiannual_fdp")) == 0)

		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_annual_fdp",	
			strlen("/event/billing/product/fee/cycle/cycle_forward_annual_fdp")) == 0)

		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_bimonthly_fdp",	
			strlen("/event/billing/product/fee/cycle/cycle_forward_bimonthly_fdp")) == 0)

		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quadmonthly_fdp",	
			strlen("/event/billing/product/fee/cycle/cycle_forward_quadmonthly_fdp")) == 0)
                /* AMC EVENTS CM AND DCM START*/
                || (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_hw_amc/cm/pre",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_monthly/mso_hw_amc/cm/pre")) == 0)
                || (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_semiannual/mso_hw_amc/cm/pre",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_semiannual/mso_hw_amc/cm/pre")) == 0)
                || (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_annual/mso_hw_amc/cm/pre",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_annual/mso_hw_amc/cm/pre")) == 0)
                || (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_bimonthly/mso_hw_amc/cm/pre",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_bimonthly/mso_hw_amc/cm/pre")) == 0)
                || (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quadmonthly/mso_hw_amc/cm/pre",
                      strlen("/event/billing/product/fee/cycle/cycle_forward_quadmonthly/mso_hw_amc/cm/pre")) == 0)
                || (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quarterly/mso_hw_amc/cm/pre",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_quarterly/mso_hw_amc/cm/pre")) == 0)
                || (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_hw_amc/dcm/pre",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_monthly/mso_hw_amc/dcm/pre")) == 0)
                || (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_semiannual/mso_hw_amc/dcm/pre",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_semiannual/mso_hw_amc/dcm/pre")) == 0)
                || (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_annual/mso_hw_amc/dcm/pre",
                         strlen("/event/billing/product/fee/cycle/cycle_forward_annual/mso_hw_amc/dcm/pre")) == 0)
                || (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_bimonthly/mso_hw_amc/dcm/pre",
                         strlen("/event/billing/product/fee/cycle/cycle_forward_bimonthly/mso_hw_amc/dcm/pre")) == 0)
                || (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quadmonthly/mso_hw_amc/dcm/pre",
                      strlen("/event/billing/product/fee/cycle/cycle_forward_quadmonthly/mso_hw_amc/dcm/pre")) == 0)
                || (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quarterly/mso_hw_amc/dcm/pre",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_quarterly/mso_hw_amc/dcm/pre")) == 0)
		/***added new 9 months AMC event***/
                || (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_nine_months/mso_hw_amc/dcm/pre",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_nine_months/mso_hw_amc/dcm/pre")) == 0)
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_nine_months/mso_hw_amc/cm/pre",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_nine_months/mso_hw_amc/cm/pre")) == 0)	
                /* AMC EVENTS CM AND DCM END */
		/**** Pavan Bellala 21-09-2015 *******
		Added Additional IP Plan events 
		**************************************/
                || (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_hw_rental/ip",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_monthly/mso_hw_rental/ip")) == 0)
		/**** Pavan Bellala 14-10-2015 ****************
		Added Additional IP Plan events for all cycles
		***********************************************/
                || (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quadmonthly/mso_hw_rental/ip",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_quadmonthly/mso_hw_rental/ip")) == 0)
                || (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_bimonthly/mso_hw_rental/ip",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_bimonthly/mso_hw_rental/ip")) == 0)
                || (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quarterly/mso_hw_rental/ip",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_quarterly/mso_hw_rental/ip")) == 0)
                || (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_semiannual/mso_hw_rental/ip",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_semiannual/mso_hw_rental/ip")) == 0)
                || (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_annual/mso_hw_rental/ip",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_annual/mso_hw_rental/ip")) == 0)
		/***added new 9 months TAL ip event***/
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_nine_months/mso_hw_rental/ip",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_nine_months/mso_hw_rental/ip")) == 0)
		/***added new 9 months subscription event***/
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_nine_months_fdp",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_nine_months_fdp")) == 0)
				/***added new 11M & 4M &5M &7M &8M &1D subscription event***/
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quadmonthly",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_quadmonthly")) == 0)
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quadmonthly/mso_sb_adn_fdp",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_quadmonthly/mso_sb_adn_fdp")) == 0)
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quadmonthly/mso_sb_pkg_fdp",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_quadmonthly/mso_sb_pkg_fdp")) == 0
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quadmonthly/mso_sb_alc_fdp",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_quadmonthly/mso_sb_alc_fdp")) == 0)
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_elevenmonthly",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_elevenmonthly")) == 0)
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_elevenmonthly/mso_sb_adn_fdp",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_elevenmonthly/mso_sb_adn_fdp")) == 0)
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_elevenmonthly/mso_sb_pkg_fdp",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_elevenmonthly/mso_sb_pkg_fdp")) == 0)
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_elevenmonthly/mso_sb_alc_fdp",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_elevenmonthly/mso_sb_alc_fdp")) == 0)
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_fivemonthly",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_fivemonthly")) == 0)
		|| strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_fivemonthly")
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_fivemonthly/mso_sb_adn_fdp",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_fivemonthly/mso_sb_adn_fdp")) == 0)
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_fivemonthly/mso_sb_pkg_fdp",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_fivemonthly/mso_sb_pkg_fdp")) == 0)
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_fivemonthly/mso_sb_alc_fdp",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_fivemonthly/mso_sb_alc_fdp")) == 0)
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_sevenmonthly",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_sevenmonthly")) == 0)
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_sevenmonthly/mso_sb_adn_fdp",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_sevenmonthly/mso_sb_adn_fdp")) == 0)
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_sevenmonthly/mso_sb_pkg_fdp",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_sevenmonthly/mso_sb_pkg_fdp")) == 0)
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_sevenmonthly/mso_sb_alc_fdp",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_sevenmonthly/mso_sb_alc_fdp")) == 0)
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_eightmonthly",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_eightmonthly")) == 0)
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_eightmonthly/mso_sb_adn_fdp",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_eightmonthly/mso_sb_adn_fdp")) == 0)
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_eightmonthly/mso_sb_pkg_fdp",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_eightmonthly/mso_sb_pkg_fdp")) == 0)
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_eightmonthly/mso_sb_alc_fdp",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_eightmonthly/mso_sb_alc_fdp")) == 0)		
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_daily",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_daily")) == 0)
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_daily/mso_sb_adn_fdp",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_daily/mso_sb_adn_fdp")) == 0)
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_daily/mso_sb_pkg_fdp",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_daily/mso_sb_pkg_fdp")) == 0)
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_daily/mso_sb_alc_fdp",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_daily/mso_sb_alc_fdp")) == 0)
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_ninemonthly",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_ninemonthly")) == 0)
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_ninemonthly/mso_sb_adn_fdp",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_ninemonthly/mso_sb_adn_fdp")) == 0)
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_ninemonthly/mso_sb_pkg_fdp",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_ninemonthly/mso_sb_pkg_fdp")) == 0)
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_ninemonthly/mso_sb_alc_fdp",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_ninemonthly/mso_sb_alc_fdp")) == 0)
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_tenmonthly",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_tenmonthly")) == 0)
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_tenmonthly/mso_sb_adn_fdp",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_tenmonthly/mso_sb_adn_fdp")) == 0)
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_tenmonthly/mso_sb_pkg_fdp",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_tenmonthly/mso_sb_pkg_fdp")) == 0)
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_tenmonthly/mso_sb_alc_fdp",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_tenmonthly/mso_sb_alc_fdp")) == 0)					
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_ten_day",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_ten_day")) == 0)
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_ten_day/mso_sb_adn_fdp",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_ten_day/mso_sb_adn_fdp")) == 0)
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_ten_day/mso_sb_pkg_fdp",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_ten_day/mso_sb_pkg_fdp")) == 0)
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_ten_day/mso_sb_alc_fdp",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_ten_day/mso_sb_alc_fdp")) == 0)					
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_bimonthly/mso_sb_alc_fdp",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_bimonthly/mso_sb_alc_fdp")) == 0)
		|| (strncmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_bimonthly/mso_sb_pkg_fdp",
                        strlen("/event/billing/product/fee/cycle/cycle_forward_bimonthly/mso_sb_pkg_fdp")) == 0)
		||      strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_thirty_days")
		||      strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_sixty_days")
		||      strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_ninety_days")
		||	strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_onetwenty_days")
		||      strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_oneeighty_days")
	   	// Jyothirmayi Kachiraju 19th Nov 2019 === New Event Creation
		|| strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_onefifty_days")
		|| strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_twoten_days")
		|| strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_twoforty_days")
		|| strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_twoseventy_days")
		|| strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_threehundred_days")
		|| strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_threethirty_days")
		|| strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_threesixty_days")
		|| strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_three_days")
		|| strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_fourteen_days")
		|| strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_weekly")
		|| strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_fifteen_days")
	   ))) 
	{
	        ppo_iflistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_OFFERING_OBJ, ppo_iflistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(ppo_iflistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);
                PIN_FLIST_FLD_SET(ppo_iflistp, PIN_FLD_CYCLE_END_T, NULL, ebufp);
		PIN_FLIST_FLD_SET(ppo_iflistp, PIN_FLD_USAGE_START_T, NULL, ebufp);
                cfee_flistp = PIN_FLIST_ELEM_ADD(ppo_iflistp, PIN_FLD_CYCLE_FEES, 1, ebufp);
                PIN_FLIST_FLD_SET(cfee_flistp, PIN_FLD_CHARGED_FROM_T, NULL, ebufp);
                PIN_FLIST_FLD_SET(cfee_flistp, PIN_FLD_CHARGED_TO_T, NULL, ebufp);
		// Added for future date renewal
  	        PIN_FLIST_FLD_SET(cfee_flistp, PIN_FLD_CYCLE_FEE_END_T, NULL, ebufp);
                PIN_FLIST_FLD_SET(cfee_flistp, PIN_FLD_CYCLE_FEE_START_T, NULL, ebufp);

                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchase product read flds input flist", ppo_iflistp);
                PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, ppo_iflistp, &ppo_oflistp, ebufp);
                PIN_FLIST_DESTROY_EX(&ppo_iflistp, NULL);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchase product read flds output flist", ppo_oflistp);
                        
		ppo_cet = PIN_FLIST_FLD_GET(ppo_oflistp, PIN_FLD_CYCLE_END_T, 1, ebufp);
		ppo_cst = PIN_FLIST_FLD_GET(ppo_oflistp, PIN_FLD_USAGE_START_T, 1, ebufp);
                svc_pdp = PIN_FLIST_FLD_GET(ppo_oflistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
                svc_type = (char *)PIN_POID_GET_TYPE(svc_pdp);	
		//Added for future dated renewal
		cfee_oflistp = PIN_FLIST_ELEM_GET(ppo_oflistp, PIN_FLD_CYCLE_FEES, 1, 1, ebufp);
                if(cfee_oflistp){
                        cf_end_t = PIN_FLIST_FLD_GET(cfee_oflistp, PIN_FLD_CYCLE_FEE_END_T, 1, ebufp);
                        pp_charged_to = PIN_FLIST_FLD_GET(cfee_oflistp, PIN_FLD_CHARGED_TO_T, 1, ebufp);
                }

		if(strcmp(svc_type,"/service/catv") == 0)
        	{
                	while(tmp_opstackp != NULL)
                 	{	
                        	sprintf(buf, "OPCODE=%d",  tmp_opstackp->opcode);
                        	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, buf);
                        	if(tmp_opstackp->opcode == PCM_OP_SUBSCRIPTION_CANCEL_DEAL){
                                	cancel = 1;
                               		 break;
                        }
                        	tmp_opstackp = tmp_opstackp->next;
                	}
       		 }	
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, event_type);
		charged_from_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CHARGED_FROM_T, 1, ebufp);
		charged_to_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CHARGED_TO_T, 1, ebufp);

		if (charged_from_t && charged_to_t)
		{
			charged_day = (localtime(charged_from_t))->tm_mday;
			actg_next_t = PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_ACTG_NEXT_T,0, ebufp);
			actg_cycle_dom = (localtime(actg_next_t))->tm_mday;
			if(charged_day == actg_cycle_dom) {
			    is_charged_t_bdom = 1;
			}
			
			if (!fm_utils_op_is_ancestor(connp->coip, MSO_OP_CUST_SUSPEND_SERVICE) && !fm_utils_op_is_ancestor(connp->coip, MSO_OP_CUST_HOLD_SERVICE))
			{
			   /*** Added PIN_FLD_ACTION in input for UNHOLD cases to reset FUP***/
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"not calling for suspend service or hold service");
				action = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTION, 1, ebufp);
				if(action != NULL)
				{
					if (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly_fdp/mso_grant") == 0 ||
						strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_thirty_days/mso_grant") == 0)
					{
						charged_from_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CHARGED_FROM_T, 1, ebufp);
						current_time = pin_virtual_time(NULL);
						if(charged_from_t && *charged_from_t <= current_time )
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "inside change dates");
							mso_fm_cycle_fee_evt_type(ctxp, event_type, charged_from_t, charged_to_t, is_charged_t_bdom, ebufp);
						}
						else
						{
							timeeff = localtime(charged_from_t);
							timeeff->tm_mon = timeeff->tm_mon - 1;
							chrg_from = mktime (timeeff);
							PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_CHARGED_FROM_T, i_flistp, PIN_FLD_CHARGED_TO_T, ebufp);
							charged_to_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CHARGED_TO_T, 1, ebufp);
							PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CHARGED_FROM_T, &chrg_from, ebufp);
							PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CHARGED_FROM_T, &chrg_from, ebufp);
							PIN_FLIST_FLD_DROP(i_flistp, PIN_FLD_ACTION, ebufp);
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "outside change dates");
						}
					}
				}
				else
				{	// Added for future dated renewal
					//if(!((strcmp(svc_type,"/service/catv") == 0) && (cancel == 1))){
					  if(!((strcmp(svc_type,"/service/catv") == 0) &&
                                                ((cancel == 1)|| fm_utils_op_is_ancestor(connp->coip, MSO_OP_CUST_TERMINATE_SERVICE))))
                                        {
                                                PIN_ERR_LOG_MSG(3, "Not Entering for CATV Cancellations & terminations");
                                                if(cf_end_t && pp_charged_to && (*cf_end_t != *pp_charged_to))
                                                {
                                                        PIN_ERR_LOG_MSG(3, "Futured dated flag set");
                                                        *charged_from_t = *cf_end_t;
                                                        future_flag =1;
                                                }

                                                PIN_ERR_LOG_MSG(3, "NOt Entering for CATV Cancellations");
                                                mso_fm_cycle_fee_evt_type(ctxp, event_type, charged_from_t, charged_to_t, is_charged_t_bdom, ebufp);
                                        }
                                        else
                                        {
                                                PIN_ERR_LOG_MSG(3, "Entering for CATV Cancellations");
                                                sprintf(buf, "charge_to_t = %d ",*charged_to_t);
                                                PIN_ERR_LOG_MSG(3, buf);
                                                sprintf(buf, "actual_charge_to_t = %d ",act_charge_to);
                                                PIN_ERR_LOG_MSG(3, buf);
                                                *charged_to_t = act_charge_to;
                                                sprintf(buf, "charge_to_t = %d, actual_charge_to_t = %d",*charged_to_t , act_charge_to);
                                                PIN_ERR_LOG_MSG(3, buf);

                                        }
				}
			}
		// The below block moved above	
		/*	ppo_iflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_OFFERING_OBJ, ppo_iflistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(ppo_iflistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);
			PIN_FLIST_FLD_SET(ppo_iflistp, PIN_FLD_CYCLE_END_T, NULL, ebufp);
			cfee_flistp = PIN_FLIST_ELEM_ADD(ppo_iflistp, PIN_FLD_CYCLE_FEES, 1, ebufp);
			PIN_FLIST_FLD_SET(cfee_flistp, PIN_FLD_CHARGED_FROM_T, NULL, ebufp);
			PIN_FLIST_FLD_SET(cfee_flistp, PIN_FLD_CHARGED_TO_T, NULL, ebufp);


			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchase product read flds input flist", ppo_iflistp);
    			PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, ppo_iflistp, &ppo_oflistp, ebufp);
			PIN_FLIST_DESTROY_EX(&ppo_iflistp, NULL);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchase product read flds output flist", ppo_oflistp);

			ppo_cet = PIN_FLIST_FLD_GET(ppo_oflistp, PIN_FLD_CYCLE_END_T, 1, ebufp);
			svc_pdp = PIN_FLIST_FLD_GET(ppo_oflistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
			svc_type = (char *)PIN_POID_GET_TYPE(svc_pdp);*/
		// Block moved up
			
		//	if((ppo_cet != NULL) && (*ppo_cet != 0) && (strcmp(svc_type,"/service/telco/broadband") == 0)) {
			if((ppo_cet != NULL) && (*ppo_cet != 0) ) 
			{
		 		if(*ppo_cet < *charged_to_t)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Resetting charged_to_t to cycle_end_t");
                                        sprintf(buf, "ppo_cet=%d; charged_to=%d", *ppo_cet,*charged_to_t);
				    	*charged_to_t = *ppo_cet;
				}
				if(strcmp(svc_type,"/service/catv") == 0)
				 {
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CATV setting charged_to_t to cycle_end_t");
                                        sprintf(buf, "ppo_cet=%d; charged_to=%d", *ppo_cet,*charged_to_t);
                                        *charged_to_t = *ppo_cet;
                                }

			}

			/*CHECK AMC EVENT AND SET CHARGED_TO FOR PREPAID SCENARIOS*/
			if((ppo_cet != NULL) && (*ppo_cet != 0) && strstr(event_type, AMC_EVENT) && (!strstr(event_type, "post")))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " AMC CHANGES Resetting charged_to_t to cycle_end_t");
				*charged_to_t = *ppo_cet;
		*charged_from_t = *ppo_cst;
			}
			/***Added changed to check weather the charged_to_t is greater than then Purchase_end_t of the extended date***/
			if(fm_utils_op_is_ancestor(connp->coip,11208))
			{
				charged_from_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CHARGED_FROM_T, 1, ebufp);
				charged_to_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CHARGED_TO_T, 1, ebufp);
				fup_status = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_FUP_STATUS, 1, ebufp);	
				if((fup_status && (*fup_status == 1 || *fup_status == 2)) && 
					(strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly_fdp/mso_grant") == 0 ||
					strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_thirty_days/mso_grant") == 0))
				{
					
                                        charged_to_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CHARGED_FROM_T, 1, ebufp);
                                        PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CYCLE_FEE_START_T, charged_to_t, ebufp );
                                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CYCLE_FEE_START_T, charged_to_t, ebufp );

				}
				else
				{
                                        end_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 1, ebufp);
                                        if(charged_to_t && end_t && (*end_t < *charged_to_t))
                                        {
                                                 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Resetting charged_to_t to cycle_end_t in validity extension flow");
                                                 *charged_to_t = *end_t;
                                        }
				}
			}

			//Added for preventing CATV FDP products getting renewed 	
			if(!((strcmp(svc_type,"/service/catv") == 0) && fm_utils_op_is_ancestor(connp->coip, MSO_OP_CUST_REACTIVATE_SERVICE)))
			{
				sprintf(buf,"service_type %s opcode : %d",svc_type, fm_utils_op_is_ancestor(connp->coip, MSO_OP_CUST_REACTIVATE_SERVICE));
				PIN_ERR_LOG_MSG(3,buf);
				// Added for future dated renewal				
				if(future_flag ==1)
                                {
                                        PIN_ERR_LOG_MSG(3, "future charging flag is set");
                                        PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CYCLE_FEE_START_T, cf_end_t, ebufp );
                                        PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CHARGED_FROM_T, cf_end_t, ebufp );
                                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CHARGED_FROM_T, cf_end_t, ebufp );
                                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CYCLE_FEE_START_T, cf_end_t, ebufp );
                                }
				PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CYCLE_FEE_END_T, charged_to_t, ebufp );
				PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CHARGED_TO_T, charged_to_t, ebufp );
				PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CHARGED_FROM_T, charged_from_t, ebufp );
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_subscription_pol_spec_cycle_fee_interval entering block", i_flistp);
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CHARGED_TO_T, charged_to_t, ebufp );
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CHARGED_FROM_T, charged_from_t, ebufp );
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CYCLE_FEE_END_T, charged_to_t, ebufp );
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CYCLE_FEE_START_T, charged_from_t, ebufp );
			}
			// Added for preventing refund/charging for FDP products in catv
			sprintf(buf,"service_type %s opcode : %d",svc_type, fm_utils_op_is_ancestor(connp->coip, MSO_OP_CUST_REACTIVATE_SERVICE));
			PIN_ERR_LOG_MSG(3,buf);		
			if( (strcmp(svc_type,"/service/catv") == 0) && 
			(fm_utils_op_is_ancestor(connp->coip, MSO_OP_CUST_REACTIVATE_SERVICE) || fm_utils_op_is_ancestor(connp->coip, MSO_OP_CUST_SUSPEND_SERVICE)))
			{

				PIN_ERR_LOG_MSG(3,"Entering suspension/Reactivation case");
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_subscription_pol_spec_cycle_fee_interval entering react/susp block", *r_flistpp);
				charged_to_t = PIN_FLIST_FLD_GET(*r_flistpp,PIN_FLD_CHARGED_TO_T,1,ebufp);
				PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CYCLE_FEE_START_T, charged_to_t, ebufp );
				PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CHARGED_FROM_T, charged_to_t, ebufp );
		       	        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_subscription_pol_spec_cycle_fee_interval entering block", i_flistp);
		                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CHARGED_FROM_T, charged_to_t, ebufp );
        	    		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CYCLE_FEE_START_T, charged_to_t, ebufp );
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_subscription_pol_spec_cycle_fee_interval entering  react/susp block iflist", i_flistp);
			}
			
			// Added for preventing refund/charging for FDP products in catv from Cancellation
			sprintf(buf,"service_type %s opcode : %d",svc_type, fm_utils_op_is_ancestor(connp->coip, MSO_OP_CUST_CANCEL_PLAN));
			PIN_ERR_LOG_MSG(3,buf);	
			if ((strcmp(svc_type,"/service/catv") == 0) && cancel == 1)
		        {
              			fm_mso_cust_get_pp_type(ctxp, acc_pdp, &profile_flistp, ebufp);
                		customer_typep = PIN_FLIST_FLD_GET(profile_flistp, PIN_FLD_CUSTOMER_TYPE, 0, ebufp);
            		}
	
			//NTO modified
		/*
			if( (strcmp(svc_type,"/service/catv") == 0) && cancel == 1 && 
			(fm_utils_op_is_ancestor(connp->coip, MSO_OP_CUST_CANCEL_PLAN) ||
                                fm_utils_op_is_ancestor(connp->coip, MSO_OP_CUST_CHANGE_PLAN) ||
				fm_utils_op_is_ancestor(connp->coip, MSO_OP_CUST_TERMINATE_SERVICE)))
			{
				
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Entering Cancel Plan case");
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_subscription_pol_spec_cycle_fee_interval entering cancel block", *r_flistpp);
				//Incase of CANCEL or TERMINATE, don't refund anything */
                                /**************************************************************
                                 * NT: Allow refund if customer is not used service even one day
                                 * after renew.
                                 **************************************************************/
                             /*   cur_t = fm_utils_time_round_to_midnight(cur_t);
                                if (cur_t >= fm_utils_time_round_to_midnight(*charged_from_t)) 
				 {

				/*	cfee_flistp = PIN_FLIST_ELEM_GET(ppo_oflistp, PIN_FLD_CYCLE_FEES, PIN_ELEMID_ANY,1,ebufp);
					actual_chg_from_t = PIN_FLIST_FLD_GET(cfee_flistp,PIN_FLD_CHARGED_FROM_T,1,ebufp);
					charged_to_t = PIN_FLIST_FLD_GET(cfee_flistp, PIN_FLD_CHARGED_TO_T, 1, ebufp); */
				/*	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CYCLE_FEE_START_T, charged_to_t, ebufp );
					PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CHARGED_FROM_T, charged_to_t, ebufp );
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "first set update", i_flistp);
					PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CHARGED_FROM_T, charged_to_t, ebufp );
					PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CYCLE_FEE_START_T, charged_to_t, ebufp );
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "after cancel block update: iflist", i_flistp); 
				}
				//Please keep below commented code for a while as requirement frequenty changing.. and can be removed after a few days

				/*pdt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
				if (pdt_pdp)
				{
					is_base_pack = fm_mso_catv_pt_pkg_type(ctxp, pdt_pdp, ebufp);

					/*************************** FDP - Refund Phase-2 - 26-08-2016**************************************
					* This is added to refund the amount for Other Addon and Alacarte from subsequent months
					* that is from next cycle. Ex: if prod is cancelled in the mid of cycle, it would be
					* refunded from next cycle (not BDOM). By default, we don't give refund for Monthly CF.
					* We would set the CHARGED_FROM_T value to next cycle. For Base and Special addons, prorated refund!
					* Also, if activation and cancellation happens on same day, refund would be skipped for that cycle.
					****************************************************************************************************
					if(is_base_pack == 1 || is_base_pack == 2)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Non-Base Pack Cancellation" );
						if (strstr(event_type, "fdp"))
						{
							if (!strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly"))
							{
								cfee_flistp = PIN_FLIST_ELEM_GET(ppo_oflistp, PIN_FLD_CYCLE_FEES, PIN_ELEMID_ANY,1,ebufp);
								actual_chg_from_t = PIN_FLIST_FLD_GET(cfee_flistp,PIN_FLD_CHARGED_FROM_T,1,ebufp);
								charged_to_t = PIN_FLIST_FLD_GET(cfee_flistp, PIN_FLD_CHARGED_TO_T, 1, ebufp);
								ref_chg_from_t = pin_virtual_time(NULL);
								ts = localtime(&ref_chg_from_t);
								localtime_r(actual_chg_from_t, &ta);
								localtime_r(charged_to_t, &tt);
								// Skip fist cycle refund
								if ( ts->tm_mday >= ta.tm_mday || (fm_utils_time_round_to_midnight(*actual_chg_from_t) ==                                                                                                                        fm_utils_time_round_to_midnight(ref_chg_from_t)) )
									ts->tm_mon = ts->tm_mon+1;
								/**********************************************************************************
								* If current month is having special days 29, 30 and 31st, then we need to set
								* CHARGED_FROM_T to end of the last month if subsequent month is not having same day
								* else set to actual charge day (only day). For ex: If cancel is on Aug 31st, and
								* Sep doesn't have 31, so we change it to Sep 30 by adding date logic
								**********************************************************************************
								if(ta.tm_mday == 31 || ta.tm_mday == 30 || ta.tm_mday == 29){
									if( ta.tm_mday == 31 && (ts->tm_mon == 3 || ts->tm_mon == 5 || ts->tm_mon == 8                                                                                                                                   || ts->tm_mon == 10 )){
										ts->tm_mday = ta.tm_mday-1;
									}//Below is for setting cycle for end dates as 31
									/*else if (ta.tm_mday == 30 && (ts->tm_mon == 0 || ts->tm_mon == 2 || ts->tm_mon == 4                                                                                              || ts->tm_mon == 6 || ts->tm_mon == 7 || ts->tm_mon == 9 || ts->tm_mon == 11)){
									if ( ta.tm_mon == 3 || ta.tm_mon == 5 || ta.tm_mon == 8  || ta.tm_mon == 10)
									{
										ts->tm_mday = ta.tm_mday+1;
									}
									}*
									else if (ts->tm_mon == 1){
										if(((ts->tm_year%4 == 0) && (ts->tm_year%100 != 0))||(ts->tm_year%400 == 0)){
											ts->tm_mday = 29;
										}
										else
											ts->tm_mday = 28;
										
										if (tt.tm_mon == 2 && (tt.tm_mday <= 3 )){
											tt.tm_mday = 1;
										}
									}
									else{ // if no special cases, set to the actual charge day of the date
										ts->tm_mday = ta.tm_mday;
									}
									// Setting CHARGED_FROM_T and CHARGED_TO_T to same day to refund zero
									ref_chg_from_t = mktime(ts);
									new_charged_to_t = mktime(&tt);
									if ( (ts->tm_mday == 30 || ts->tm_mday == 28 || ts->tm_mday == 29) && tt.tm_mday == 1 &&                                                                                        ((*charged_to_t - fm_utils_time_round_to_midnight(ref_chg_from_t ) <= 86400 )                                                                                           || (new_charged_to_t - fm_utils_time_round_to_midnight(ref_chg_from_t ) <= 86400 )))
									{
										tt.tm_mday = tt.tm_mday-1;
									}
								}
								else
									ts->tm_mday = ta.tm_mday;
								
								ref_chg_from_t = mktime(ts);
								new_charged_to_t = mktime(&tt);
								PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CYCLE_FEE_START_T, &ref_chg_from_t, ebufp );
								PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CHARGED_FROM_T, &ref_chg_from_t, ebufp );
								PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CHARGED_FROM_T, &ref_chg_from_t, ebufp );
								PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CYCLE_FEE_START_T, &ref_chg_from_t, ebufp );
								PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CYCLE_FEE_END_T, &new_charged_to_t, ebufp );
								PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CHARGED_TO_T, &new_charged_to_t, ebufp );
								PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CHARGED_TO_T, &new_charged_to_t, ebufp );
								PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CYCLE_FEE_END_T, &new_charged_to_t, ebufp );
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Changed CHARGED dates" );
								PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "after cancel block update: iflist", i_flistp);
							}
							else{
								charged_to_t = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_CHARGED_TO_T, 1, ebufp);
								PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CYCLE_FEE_START_T, charged_to_t, ebufp );
								PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CHARGED_FROM_T, charged_to_t, ebufp );
								PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "first set update", i_flistp);
								PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CHARGED_FROM_T, charged_to_t, ebufp );
								PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CYCLE_FEE_START_T, charged_to_t, ebufp );
								PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "after cancel block update: iflist", i_flistp);
							}
								
						}
					}
				    else
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Base Pack Cancellation" );
				}*/
                
		//	}
                        /***************************************************************
                         * NTO: Commenting out refund logic as its same as cancel*/

			//Below added to provide refund from next day in case of CATV Change Plan 
			if( (strcmp(svc_type,"/service/catv") == 0) && cancel == 1 && (fm_utils_op_is_ancestor(connp->coip, MSO_OP_CUST_CHANGE_PLAN)
				 || fm_utils_op_is_ancestor(connp->coip, MSO_OP_CUST_CANCEL_PLAN) || fm_utils_op_is_ancestor(connp->coip, MSO_OP_CUST_TERMINATE_SERVICE)))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Change Plan CANCEL case - New change " );
				cfee_flistp = PIN_FLIST_ELEM_GET(ppo_oflistp, PIN_FLD_CYCLE_FEES, PIN_ELEMID_ANY,1,ebufp);
				if(cfee_flistp && ((customer_typep && *customer_typep == 48) ||
                   			 fm_utils_op_is_ancestor(connp->coip, MSO_OP_CUST_CHANGE_PLAN)))
				{
					actual_chg_from_t = PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_CHARGED_FROM_T,1,ebufp);
					charged_to_t = PIN_FLIST_FLD_GET(cfee_flistp, PIN_FLD_CHARGED_TO_T, 1, ebufp);
					if(fm_utils_time_round_to_midnight(*actual_chg_from_t) != *charged_to_t) //If both are on same day, then don't set
					{ 
						if (prod_namep && strstr(prod_namep, "NCF") && *customer_typep == 48 &&
		        	                   fm_utils_op_is_ancestor(connp->coip, MSO_OP_CUST_CHANGE_PLAN))
						{
							ref_chg_from_t = fm_utils_time_round_to_midnight(*actual_chg_from_t);
						}
						else
						{
							ref_chg_from_t = fm_utils_time_round_to_midnight(*actual_chg_from_t)+86400; //Add +1 day
						}
						PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CYCLE_FEE_START_T, &ref_chg_from_t, ebufp );
						PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CHARGED_FROM_T, &ref_chg_from_t, ebufp );
						PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CHARGED_FROM_T, &ref_chg_from_t, ebufp );
						PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CYCLE_FEE_START_T, &ref_chg_from_t, ebufp );
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Change Plan Changed CHARGED dates" );
					}
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "after cancel block update: iflist", i_flistp);
				}
				else
		                {
                		    PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CYCLE_FEE_START_T, charged_to_t, ebufp );
                  		    PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CHARGED_FROM_T, charged_to_t, ebufp );
                   		    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "first set update", i_flistp);
                    		    PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CHARGED_FROM_T, charged_to_t, ebufp );
                    		    PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CYCLE_FEE_START_T, charged_to_t, ebufp );
                    		    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "after cancel block update: iflist", i_flistp);
               			 }

					
			}
                       /* **********************************************************************************/

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_subscription_pol_spec_cycle_fee_interval after changing input flist", i_flistp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_subscription_pol_spec_cycle_fee_interval r_flistpp", *r_flistpp);
		}
	}

	if(event_type && !strstr(event_type, AMC_EVENT))
	{
		update_for_topup(ctxp, opstackp, event_type, i_flistp, r_flistpp, ebufp);
	}

	/* Set the FUP_STATUS to BEFORE_FUP for FUP products here */
	read_prod_details(ctxp, i_flistp, &prod_details_flistp, ebufp);
	//fm_mso_update_service_fup_status(ctxp, i_flistp, prod_details_flistp, event_type, opstackp, ebufp);
	if(!fm_utils_op_is_ancestor(connp->coip,11208))
		fm_mso_update_service_fup_status(connp, ctxp, i_flistp, prod_details_flistp, event_type, opstackp, *r_flistpp,  ebufp);
	//create_lifecycle_event(ctxp, i_flistp, prod_details_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&readproduct_outflistp, NULL);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Returning from fm_subscription_pol_spec_cycle_fee_interval after changing input flist");
	return;
}

static void 
update_for_topup(
	pcm_context_t	*ctxp, 
	cm_op_info_t	*opstackp,
	char		*event_type,
	pin_flist_t	*i_flistp, 
	pin_flist_t	**r_flistpp, 
	pin_errbuf_t	*ebufp)
{
	int32		stack_opcode = 0;
	int32		initial_opcode = 0;
	char		debug_msg[250];
	char		*program_name = NULL;
	char            *action = NULL;
	pin_flist_t	*initial_flistp = NULL;
	pin_flist_t	*pp_read_iflistp = NULL;
	pin_flist_t	*pp_read_oflistp = NULL;
	int32		is_topup = 0;
	int32		bill_when = 0;
	int32		fup_plan = 0;
	struct tm       *timeeff;
	time_t		*purchase_end_t = 0;
	time_t          *usage_start_t = 0;
	time_t		charged_from_t = 0;
	time_t		charged_to_t = 0;
	poid_t		*service_pdp = NULL;
	poid_t		*prod_pdp = NULL;
	poid_t		*acc_pdp = NULL;
	pin_decimal_t	*prod_priority = NULL;
	double		prod_priority_double = 0;
	pin_flist_t	*readproduct_inflistp = NULL;
	pin_flist_t	*readproduct_outflistp = NULL;

	int32		is_add_mb = 0;
	int32		subs_topup = 0;
	poid_t		*plan_pdp = NULL;
	poid_t		*purc_pdp = NULL;
	pin_flist_t	*ret_flistpp = NULL;
	time_t		*purch_start_t = NULL;
	int32           day_in_month;
	int             month;
	time_t		*chrg_to_t = NULL;
	time_t		chrg_to = 0;
	int		o_charged_day;
	int		o_charged_month;
	int		n_day_f;
	int		n_day_t;
	int		n_year_t;
	time_t		*chrg_from_t = NULL;
	int32		flags = 1;
	int32           flag = 0;
	int32		leapyear = 0;
	char            tmp[200]="";
	int32		leap_year = 0;
	struct tm       *gm_time;
        char            time_buf[30] = {'\0'};
	int32           month_change = 0;
	pin_flist_t	*res_flistpp = NULL;
	pin_flist_t	*bb_info_flistp = NULL;
	poid_t		*serv_pdp = NULL;

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Entered update_for_topup ");
    while(opstackp != NULL)
    {
		stack_opcode = opstackp->opcode;
		sprintf(debug_msg, "OPCODE=%d", stack_opcode);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"BEFORE PCM_OP_ACT_USAGE FLIST", opstackp->in_flistp);
		initial_flistp = opstackp->in_flistp;
		initial_opcode = opstackp->opcode;
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Intial opcode input FLIST", initial_flistp);
		if ((initial_opcode == MSO_OP_PROV_BB_PROCESS_RESPONSE))
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"MSO_OP_PROV_BB_PROCESS_RESPONSE input FLIST",initial_flistp);
			action = PIN_FLIST_FLD_GET(initial_flistp, PIN_FLD_ACTION, 0, ebufp);
			program_name = PIN_FLIST_FLD_GET(initial_flistp, PIN_FLD_PROGRAM_NAME, 0, ebufp);
			if((action) && (strstr(action, "TOPUP_WO_PLAN") ||
				strstr(action, "ACTIVATION") ||
				strstr(action, "CHANGE PLAN") ||
				strstr(action, "ADD PLAN")))
			{
				is_topup = 1;
				subs_topup = 1;
				break;
			}
			//Added else condition for Add MB/GB
			else if((action) && strstr(action, "ADD_DATA_DOCSIS"))
			{
				is_add_mb = 1;
				break;
			}

		}
		opstackp = opstackp->next;
    }

	if(is_topup || is_add_mb) {
		pp_read_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_OFFERING_OBJ, pp_read_iflistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(pp_read_iflistp, PIN_FLD_PURCHASE_END_T, NULL, ebufp);
		PIN_FLIST_FLD_SET(pp_read_iflistp, PIN_FLD_USAGE_START_T, NULL, ebufp);
		PIN_FLIST_FLD_SET(pp_read_iflistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);
		PIN_FLIST_FLD_SET(pp_read_iflistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
		PIN_FLIST_FLD_SET(pp_read_iflistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchase product read purchase_end_t input flist", pp_read_iflistp);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, pp_read_iflistp, &pp_read_oflistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchase product read purchase_end_t output flist", pp_read_oflistp);

		purchase_end_t = PIN_FLIST_FLD_GET(pp_read_oflistp, PIN_FLD_PURCHASE_END_T, 0, ebufp);
		usage_start_t = PIN_FLIST_FLD_GET(pp_read_oflistp, PIN_FLD_USAGE_START_T, 0, ebufp);
		service_pdp = PIN_FLIST_FLD_GET(pp_read_oflistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
		acc_pdp = PIN_FLIST_FLD_GET(pp_read_oflistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
		prod_pdp = PIN_FLIST_FLD_GET(pp_read_oflistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
		bill_when = get_bill_when_from_service(ctxp, service_pdp, ebufp);

		// Read product priority
		readproduct_inflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(readproduct_inflistp, PIN_FLD_POID, prod_pdp, ebufp );
		PIN_FLIST_FLD_SET(readproduct_inflistp, PIN_FLD_PRIORITY, 0, ebufp );

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "update_for_topup read product input list", readproduct_inflistp);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, readproduct_inflistp, &readproduct_outflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&readproduct_inflistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&readproduct_outflistp, NULL);	
			return;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "update_for_topup read product output flist", readproduct_outflistp);
		prod_priority = PIN_FLIST_FLD_GET(readproduct_outflistp, PIN_FLD_PRIORITY, 0, ebufp );
		prod_priority_double = pbo_decimal_to_double(prod_priority, ebufp);
		prod_priority_double = ((int)prod_priority_double) % 1000;
		PIN_FLIST_DESTROY_EX(&readproduct_outflistp, NULL);

		if(prod_priority_double >= BB_UNLIMITED_FUP_RANGE_START 
			&& prod_priority_double <= BB_UNLIMITED_FUP_RANGE_END)
		{
			fup_plan=1;
		}
		if(	!fup_plan ||
			((fup_plan && strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_grant") != 0) &&
			(fup_plan && strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_thirty_days/mso_grant") != 0) &&
			(fup_plan && strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly_fdp/mso_grant") != 0))
		    )
		{
			if(!is_add_mb) {
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Updating dates for topup.");
				//timeeff = localtime(purchase_end_t);
				//timeeff->tm_mon = timeeff->tm_mon - bill_when;
				//charged_from_t = mktime (timeeff);

				//PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CHARGED_FROM_T, &charged_from_t, ebufp );
				//PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CYCLE_FEE_START_T, &charged_from_t, ebufp );
				PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CHARGED_TO_T, purchase_end_t, ebufp );
				PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CYCLE_FEE_END_T, purchase_end_t, ebufp );
				PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CHARGED_FROM_T, usage_start_t, ebufp );
                                PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CYCLE_FEE_START_T, usage_start_t, ebufp );


				//PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CHARGED_FROM_T, &charged_from_t, ebufp );
				//PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CYCLE_FEE_START_T, &charged_from_t, ebufp );
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CHARGED_TO_T, purchase_end_t, ebufp );
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CYCLE_FEE_END_T, purchase_end_t, ebufp );
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CHARGED_FROM_T, usage_start_t, ebufp );
                                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CYCLE_FEE_START_T, usage_start_t, ebufp );
			} else {
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Updating dates for Add MB.");
				PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CHARGED_TO_T, purchase_end_t, ebufp );
				PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CYCLE_FEE_END_T, purchase_end_t, ebufp );
				//align cycle fee start to charged from
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_CHARGED_FROM_T, i_flistp, PIN_FLD_CYCLE_FEE_START_T, ebufp );

				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CHARGED_TO_T, purchase_end_t, ebufp );
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CYCLE_FEE_END_T, purchase_end_t, ebufp );
				PIN_FLIST_FLD_COPY(*r_flistpp, PIN_FLD_CHARGED_FROM_T, *r_flistpp, PIN_FLD_CYCLE_FEE_START_T, ebufp );
			}

		}
		else if(fup_plan && strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_grant") == 0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Updating dates for Grant Product.");
			// Charged_to date must be set to 1 month from the expiry date of current plan.
			// New purcahse end date - bill when = expiry date of current plan
			// So we subtract bill_when and add 1 month to it.
			//timeeff = localtime(purchase_end_t);
			//timeeff->tm_mon = timeeff->tm_mon - bill_when + 1;
			//charged_to_t = mktime (timeeff);

			get_last_plan_valid_to_fup(ctxp, acc_pdp, service_pdp, &charged_to_t, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling get_last_plan_valid_to_fup", ebufp);
				PIN_FLIST_DESTROY_EX(&readproduct_outflistp, NULL);	
				return;
			}
			//PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CHARGED_FROM_T, &charged_from_t, ebufp );
			//PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CYCLE_FEE_START_T, &charged_from_t, ebufp );
			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CHARGED_TO_T, &charged_to_t, ebufp );
			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CYCLE_FEE_END_T, &charged_to_t, ebufp );

			//PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CHARGED_FROM_T, &charged_from_t, ebufp );
			//PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CYCLE_FEE_START_T, &charged_from_t, ebufp );
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CHARGED_TO_T, &charged_to_t, ebufp );
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CYCLE_FEE_END_T, &charged_to_t, ebufp );
		}
		if(fup_plan && subs_topup ==1 && (fup_plan && 
			(strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly_fdp/mso_grant") == 0 ||
			strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_thirty_days/mso_grant") == 0)))
		{
			purc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_OFFERING_OBJ, 0, ebufp);
			fm_mso_get_poid_details(ctxp, purc_pdp, &res_flistpp, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error after calling fm_mso_get_poid_details", ebufp);
				goto NOMATCH;
				//return;
			}
			serv_pdp = PIN_FLIST_FLD_GET(res_flistpp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
			plan_pdp = PIN_FLIST_FLD_GET(res_flistpp, PIN_FLD_PLAN_OBJ, 0, ebufp);
			fm_mso_get_poid_details(ctxp, serv_pdp, &ret_flistpp, ebufp);
                        if (PIN_ERRBUF_IS_ERR(ebufp))
                        {
                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error after calling fm_mso_get_poid_details serv_poid", ebufp);
				goto NOMATCH;
                                //return;
                        }
			bb_info_flistp = PIN_FLIST_SUBSTR_GET(ret_flistpp, MSO_FLD_BB_INFO, 0, ebufp );
			purch_start_t = PIN_FLIST_FLD_GET(bb_info_flistp, MSO_FLD_RENEW_T, 0, ebufp);
			if(purch_start_t && *purch_start_t == 0)	
			{
				PIN_FLIST_DESTROY_EX(&ret_flistpp, NULL);
				ret_flistpp = NULL;
				get_last_mso_purchased_plan_modified(ctxp, i_flistp, plan_pdp, flag,&ret_flistpp, ebufp);
				purch_start_t = PIN_FLIST_FLD_GET(ret_flistpp, PIN_FLD_PURCHASE_START_T, 0, ebufp);
			}
			chrg_from_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CHARGED_FROM_T, 0, ebufp);
                        chrg_to_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CHARGED_TO_T, 0, ebufp);
			
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error after calling get_last_mso_purchased_plan_modified", ebufp);
				goto NOMATCH;
				//return;
			}
			
			o_charged_day = (localtime(purch_start_t))->tm_mday;
			n_day_f = (localtime(chrg_from_t))->tm_mday;
                        n_day_t = (localtime(chrg_to_t))->tm_mday;
			
			if(o_charged_day <= n_day_f)
                        {
				timeeff = localtime(chrg_to_t);
                        	month = timeeff->tm_mon;
				
				if(n_day_f == 30 || n_day_f == 31 || n_day_f == 29 || n_day_f == 28)
                                {
                                        if(n_day_t != 30 && n_day_t != 31 && n_day_t != 29 && n_day_t != 28)
                                        {
						month_change = 1;
						
                                                month = month -1;
						timeeff->tm_mon = month;
                                                chrg_to = mktime (timeeff);
						if(*chrg_from_t > chrg_to)
						{
							chrg_to = 0;
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "in case of trail plan");
							month = month + 1;
						}
                                        }
                                }

				gm_time = localtime(chrg_to_t);
					
                        }
			else
			{	
				gm_time = localtime(chrg_from_t);

				/**if reset date is less than the charged_from_t, it will go into this loop*/
				/**ex: reset = 27 and charged_from_t = 26 where 27 > 26*/
				timeeff = localtime(chrg_from_t);
                                month = timeeff->tm_mon;
			}

			strftime(time_buf, sizeof(time_buf), "%Y", gm_time);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Expiry Time :");
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, time_buf);
                        n_year_t = atoi(time_buf);

			 /**check for leap year**/
                        if (((n_year_t % 4 == 0) && (n_year_t % 100!= 0)) || (n_year_t%400 == 0))
                        {
                                leap_year = 1;
                                sprintf(tmp,"Leap Year : %d",n_year_t);
                        }
                        else
                        {
                                sprintf(tmp,"Not a Leap Year : %d",n_year_t);
                        }
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, tmp );

			if (month == 0 || month == 2 || month == 4 || month == 6 || month == 7 || month == 9 || month == 11)
			{
				day_in_month = 31;
			}
			else if (month == 1)
			{
				if(leap_year == 0)
				{
					day_in_month = 28;
				}
				else
				{
					day_in_month = 29;
				}
			}
			else
			{
				day_in_month = 30;
			}
	
			if(month_change)
                        {
                                        
                                        timeeff->tm_mon = month;
                        }	
			/** if o_charged_day = 31 and day_in_month = 28 then 31>28 we have to make the day as 28*/
			if(o_charged_day >= day_in_month)
			{
                                timeeff->tm_mday = day_in_month;
			}
			else
			{
				timeeff->tm_mday = o_charged_day;
			}

			chrg_to = mktime (timeeff) ;
			
			if(chrg_to > *purchase_end_t || chrg_to< *chrg_from_t)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Dont make any changes");
				goto NOMATCH;
			}
			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CHARGED_TO_T, &chrg_to, ebufp);
			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CYCLE_FEE_END_T, &chrg_to, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CHARGED_TO_T, &chrg_to, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CYCLE_FEE_END_T, &chrg_to, ebufp);
		}

	} else {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," Not a top-up flow. Doing nothing");
	}
NOMATCH:
	PIN_FLIST_DESTROY_EX(&res_flistpp,NULL);
	PIN_FLIST_DESTROY_EX(&ret_flistpp, NULL);
	PIN_FLIST_DESTROY_EX(&pp_read_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&pp_read_oflistp, NULL);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Leaving update_for_topup ");
    return;

}


/*********************************************************************************************************
*  calculating the correct end time for the fixed duration package
*  Any event added in the below finction, the same event should be added above at if condition
*********************************************************************************************************/

static void 
mso_fm_cycle_fee_evt_type(
pcm_context_t		*ctxp,
char			*event_type, 
time_t			*charged_from_t,
time_t			*charged_to_t,
int32			is_charged_t_bdom,
pin_errbuf_t		*ebufp)
{
	struct tm		*timeeff;
        char                    log_buf[255];

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_fm_cycle_fee_evt_type :");	

	timeeff = localtime(charged_from_t);

	if (event_type) 
	{
	     if(((strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_annual/mso_sb_adn_fdp") == 0)
	               || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_annual/mso_sb_alc_fdp") == 0)
	               || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_annual/mso_sb_pkg_fdp") == 0) 
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_annual_fdp/mso_sb_norm") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_annual_fdp/mso_sb_norm/trial") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_annual_fdp/mso_sb_dc2") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_annual_fdp/mso_sb_dc3") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_annual_fdp/mso_sb_dc3/trial") == 0) 
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_annual_fdp/mso_sb_fib") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_annual_fdp/mso_sb_tod") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_annual/mso_hw_rental/ip") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_annual_fdp/mso_grant") == 0))
	     || ((is_charged_t_bdom) && (
				   (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_annual/mso_hw_rental/cm") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_annual/mso_hw_rental/cr") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_annual/mso_hw_rental/huwr") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_annual/mso_hw_rental/nd") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_annual/mso_hw_rental/wlnd") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_annual/mso_hw_rental/dcm") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_annual/mso_hw_rental/nwifi") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_annual/mso_hw_rental/crsoho") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_annual/mso_hw_amc/cm") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_annual/mso_hw_amc/cr") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_annual/mso_hw_amc/nd") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_annual/mso_hw_amc/wlnd") == 0)
				
	       )))
	    {
		timeeff->tm_year = timeeff->tm_year + 1; 
	    }
	    else if (((strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_sb_adn_fdp") == 0)
			    || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_sb_alc_fdp") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly_fdp/mso_sb_norm") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly_fdp/mso_sb_norm/trial") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly_fdp/mso_sb_dc2") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly_fdp/mso_sb_dc2/trial") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly_fdp/mso_sb_dc3") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly_fdp/mso_sb_dc3/trial") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly_fdp/mso_sb_fib") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly_fdp/mso_sb_tod") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly_fdp/mso_grant") == 0)
               		           || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_hw_rental/ip")==0)
	                    || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_sb_pkg_fdp") == 0) )
			|| ((is_charged_t_bdom) &&  (
				    (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_hw_rental/cm") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_hw_rental/cr") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_hw_rental/huwr") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_hw_rental/nd") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_hw_rental/wlnd") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_hw_rental/dcm") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_hw_rental/nwifi") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_hw_rental/crsoho") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_hw_amc/cm") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_hw_amc/cr") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_hw_amc/nd") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_hw_amc/wlnd") == 0)
                			/**** Pavan Bellala 21-09-2015 *******
        			        Added Additional IP Plan events
			                **************************************/
					/**** Pavan Bellala 14-10-2015 ****************
					Added Additional IP Plan events for all cycles
					***********************************************/
				
	)))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Setting the charged_to_t to 1 month");
		timeeff->tm_mon = timeeff->tm_mon + 1; 
	}
	else if ((strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_bimonthly/mso_sb_adn_fdp") == 0)
	        || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_bimonthly/mso_sb_alc_fdp") == 0)
	        || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_bimonthly/mso_sb_pkg_fdp") == 0)
		|| (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_bimonthly/mso_hw_rental/ip") == 0)
	        )
	{
		timeeff->tm_mon = timeeff->tm_mon + 2; 
	}
	else if (((strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quarterly/mso_sb_adn_fdp") == 0)
	 	            || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quarterly/mso_sb_alc_fdp") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quarterly_fdp/mso_sb_norm") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quarterly_fdp/mso_sb_dc2") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quarterly_fdp/mso_sb_dc3") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quarterly_fdp/mso_sb_fib") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quarterly_fdp/mso_sb_tod") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quarterly_fdp/mso_grant") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quarterly/mso_hw_rental/ip") == 0)
	                    || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quarterly/mso_sb_pkg_fdp") == 0) )
		 || ((is_charged_t_bdom) && (
 				    (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quarterly/mso_hw_rental/cm") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quarterly/mso_hw_rental/cr") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quarterly/mso_hw_rental/huwr") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quarterly/mso_hw_rental/nd") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quarterly/mso_hw_rental/wlnd") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quarterly/mso_hw_rental/dcm") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quarterly/mso_hw_rental/nwifi") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quarterly/mso_hw_rental/crsoho") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quarterly/mso_hw_amc/cm") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quarterly/mso_hw_amc/cr") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quarterly/mso_hw_amc/nd") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quarterly/mso_hw_amc/wlnd") == 0)
	        )))
	{
		timeeff->tm_mon = timeeff->tm_mon + 3; 
	}
	else if (((strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_semiannual/mso_sb_adn_fdp") == 0)
	 	            || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_semiannual/mso_sb_alc_fdp") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_semiannual_fdp/mso_sb_norm") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_semiannual_fdp/mso_sb_norm/trial") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_semiannual_fdp/mso_sb_dc2") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_semiannual_fdp/mso_sb_dc3") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_semiannual_fdp/mso_sb_fib") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_semiannual_fdp/mso_sb_tod") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_semiannual_fdp/mso_grant") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_semiannual/mso_hw_rental/ip") == 0)
	                    || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_semiannual/mso_sb_pkg_fdp") == 0) )
		|| ((is_charged_t_bdom) && (
				   (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_semiannual/mso_hw_rental/cm") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_semiannual/mso_hw_rental/cr") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_semiannual/mso_hw_rental/huwr") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_semiannual/mso_hw_rental/nd") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_semiannual/mso_hw_rental/wlnd") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_semiannual/mso_hw_rental/dcm") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_semiannual/mso_hw_rental/nwifi") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_semiannual/mso_hw_rental/crsoho") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_semiannual/mso_hw_amc/cm") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_semiannual/mso_hw_amc/cr") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_semiannual/mso_hw_amc/nd") == 0)
				   || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_semiannual/mso_hw_amc/wlnd") == 0)
	        )))
	{
		timeeff->tm_mon = timeeff->tm_mon + 6; 
	} else if (((strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_bimonthly_fdp/mso_sb_norm") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_bimonthly_fdp/mso_sb_dc2") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_bimonthly_fdp/mso_sb_dc3") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_bimonthly_fdp/mso_sb_fib") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_bimonthly_fdp/mso_sb_tod") == 0) 
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_bimonthly_fdp/mso_grant") == 0)) 
		|| ((is_charged_t_bdom) && (
                 (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_bimonthly/mso_hw_rental/cr") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_bimonthly/mso_hw_rental/huwr") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_bimonthly/mso_hw_rental/nd") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_bimonthly/mso_hw_rental/wlnd") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_bimonthly/mso_hw_rental/dcm") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_bimonthly/mso_hw_rental/nwifi") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_bimonthly/mso_hw_rental/crsoho") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_bimonthly/mso_hw_amc/cm") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_bimonthly/mso_hw_amc/cr") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_bimonthly/mso_hw_amc/nd") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_bimonthly/mso_hw_amc/wlnd") == 0)
		)))
	{
		timeeff->tm_mon = timeeff->tm_mon + 2;
	} else if (((strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quadmonthly_fdp/mso_sb_norm") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quadmonthly_fdp/mso_sb_dc2") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quadmonthly_fdp/mso_sb_dc3") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quadmonthly_fdp/mso_sb_fib") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quadmonthly_fdp/mso_sb_tod") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quadmonthly_fdp/mso_grant") == 0)
				 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quadmonthly") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quadmonthly/mso_sb_adn_fdp") == 0)
		         || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quadmonthly/mso_sb_alc_fdp") == 0)
		 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quadmonthly/mso_hw_rental/ip") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quadmonthly/mso_sb_pkg_fdp") == 0)) 
		|| ((is_charged_t_bdom) && (
                 (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quadmonthly/mso_hw_rental/cr") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quadmonthly/mso_hw_rental/huwr") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quadmonthly/mso_hw_rental/nd") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quadmonthly/mso_hw_rental/wlnd") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quadmonthly/mso_hw_rental/dcm") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quadmonthly/mso_hw_rental/nwifi") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quadmonthly/mso_hw_rental/crsoho") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quadmonthly/mso_hw_amc/cm") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quadmonthly/mso_hw_amc/cr") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quadmonthly/mso_hw_amc/nd") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_quadmonthly/mso_hw_amc/wlnd") == 0)
                )))
        {
                timeeff->tm_mon = timeeff->tm_mon + 4;
        }
	/**changes for new 9 months event****/
	else if (((strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_nine_months_fdp/mso_sb_dc3") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_nine_months_fdp/mso_sb_dc2") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_nine_months_fdp/mso_sb_fib") == 0)
		 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_nine_months/mso_hw_rental/ip") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_nine_months_fdp/mso_grant") == 0))
		 || ((is_charged_t_bdom) && (
		 (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_nine_months/mso_hw_amc/cm/pre") == 0)
		 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_nine_months/mso_hw_amc/dcm/pre") == 0)
		 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_nine_months/mso_hw_amc/huwr/pre") == 0)
		 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_nine_months/mso_hw_amc/nd/pre") == 0)
		 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_nine_months/mso_hw_amc/wlnd/pre") == 0)
		 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_nine_months/mso_hw_amc/nwifi/pre") == 0)
		 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_nine_months/mso_hw_amc/crsoho/pre") == 0)
		 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_nine_months/mso_hw_amc/cr/pre") == 0)
		)))
        {
                timeeff->tm_mon = timeeff->tm_mon + 9;
        }
    /**changes for new 5 months event****/
       else if (((strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_fivemonthly/mso_sb_dc3") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_fivemonthly/mso_sb_dc2") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_fivemonthly/mso_sb_fib") == 0)
         || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_fivemonthly/mso_hw_rental/ip") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_fivemonthly/mso_grant") == 0))
         || ((is_charged_t_bdom) && (
         (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_fivemonthly/mso_hw_amc/cm/pre") == 0)
         || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_fivemonthly/mso_hw_amc/dcm/pre") == 0)
         || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_fivemonthly/mso_hw_amc/huwr/pre") == 0)
         || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_fivemonthly/mso_hw_amc/nd/pre") == 0)
         || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_fivemonthly/mso_hw_amc/wlnd/pre") == 0)
         || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_fivemonthly/mso_hw_amc/nwifi/pre") == 0)
         || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_fivemonthly/mso_hw_amc/crsoho/pre") == 0)
         || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_fivemonthly/mso_hw_amc/cr/pre") == 0)
        )))
        {
                timeeff->tm_mon = timeeff->tm_mon + 5;
        }

	/**changes for new 11,8,7,9,10,1d,10d(1d,10d,Not Required) months event ,Added by Suresh Nadar****/
	else if (((strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_elevenmonthly") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_elevenmonthly/mso_sb_adn_fdp") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_elevenmonthly/mso_sb_pkg_fdp") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_elevenmonthly/mso_sb_alc_fdp") == 0))
                )
        {
                timeeff->tm_mon = timeeff->tm_mon + 11;
        }
	else if (((strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_fivemonthly") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_fivemonthly/mso_sb_adn_fdp") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_fivemonthly/mso_sb_pkg_fdp") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_fivemonthly/mso_sb_alc_fdp") == 0))
                )
        {
                timeeff->tm_mon = timeeff->tm_mon + 5;
        }
	else if (((strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_ninemonthly") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_ninemonthly/mso_sb_adn_fdp") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_ninemonthly/mso_sb_pkg_fdp") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_ninemonthly/mso_sb_alc_fdp") == 0))
                )
        {
                timeeff->tm_mon = timeeff->tm_mon + 9;
        }
	else if (((strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_tenmonthly") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_tenmonthly/mso_sb_adn_fdp") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_tenmonthly/mso_sb_pkg_fdp") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_tenmonthly/mso_sb_alc_fdp") == 0))
                )
        {
                timeeff->tm_mon = timeeff->tm_mon + 10;
        }
	else if (((strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_sevenmonthly") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_sevenmonthly/mso_sb_adn_fdp") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_sevenmonthly/mso_sb_pkg_fdp") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_sevenmonthly/mso_sb_alc_fdp") == 0))
                )
        {
                timeeff->tm_mon = timeeff->tm_mon + 7;
        }
	else if (((strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_eightmonthly") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_eightmonthly/mso_sb_adn_fdp") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_eightmonthly/mso_sb_pkg_fdp") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_eightmonthly/mso_sb_alc_fdp") == 0))
                )
        {
                timeeff->tm_mon = timeeff->tm_mon + 8;
        }
	else if (((strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_ten_day") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_ten_day/mso_sb_adn_fdp") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_ten_day/mso_sb_pkg_fdp") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_ten_day/mso_sb_alc_fdp") == 0))
                )
        {
                timeeff->tm_mday = timeeff->tm_mday + 10;
        }
	else if (((strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_daily") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_daily/mso_sb_adn_fdp") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_daily/mso_sb_pkg_fdp") == 0)
                 || (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_daily/mso_sb_alc_fdp") == 0))
                )
        {
                timeeff->tm_mday = timeeff->tm_mday + 1;
        }
        else if (strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_thirty_days"))
        {
                timeeff->tm_mday = timeeff->tm_mday + 30;
        }
        else if (strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_sixty_days"))
        {
                timeeff->tm_mday = timeeff->tm_mday + 60;
        }
        else if (strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_ninety_days"))
        {
                timeeff->tm_mday = timeeff->tm_mday + 90;
        }
	else if (strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_onetwenty_days"))
	{
		timeeff->tm_mday = timeeff->tm_mday + 120;
	}
        else if (strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_oneeighty_days"))
        {
                timeeff->tm_mday = timeeff->tm_mday + 180;
        }
        else if (strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_three_days"))
        {
                timeeff->tm_mday = timeeff->tm_mday + 3;
        }
	else if (strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_fourteen_days"))
        {
                timeeff->tm_mday = timeeff->tm_mday + 14;
        }
        else if (strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_weekly"))
        {
                timeeff->tm_mday = timeeff->tm_mday + 7;
        }
        else if (strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_fifteen_days"))
        {
                timeeff->tm_mday = timeeff->tm_mday + 15;
        }
		// Jyothirmayi Kachiraju 19th Nov 2019 === New Event Creation
		else if (strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_onefifty_days"))
        {
            timeeff->tm_mday = timeeff->tm_mday + 150;
        }
		else if (strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_twoten_days"))
        {
            timeeff->tm_mday = timeeff->tm_mday + 210;
        }
		else if (strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_twoforty_days"))
        {
            timeeff->tm_mday = timeeff->tm_mday + 240;
        }
		else if (strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_twoseventy_days"))
        {
            timeeff->tm_mday = timeeff->tm_mday + 270;
        }
		else if (strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_threehundred_days"))
        {
            timeeff->tm_mday = timeeff->tm_mday + 300;
        }
		else if (strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_threethirty_days"))
        {
            timeeff->tm_mday = timeeff->tm_mday + 330;
        }
		else if (strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_threesixty_days"))
        {
            timeeff->tm_mday = timeeff->tm_mday + 360;
        }
	
	// Set the timeeff to charged_to_t for hardware events with non BDOM charged day. Not AMC
	if(!is_charged_t_bdom && (strstr(event_type, "mso_hw") != NULL) 
			      && (strstr(event_type, AMC_EVENT) == NULL)
			      && (strstr(event_type, "mso_hw_rental/ip") == NULL)) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Setting the charged_to_t to default");
	    timeeff = localtime(charged_to_t);
	} 
	}

	/*Commented as not available in svn on 29JAN19
	 Setting day in charge_to_t to correct day as per  calendar to fix duplicate charging issue. 

        switch (timeeff->tm_mon)
	{
	case 1:
	case 3:
	case 5:
	case 7:
	case 8:
	case 10:
	case 12:

	if(timeeff->tm_mday > 31 )
	 timeeff->tm_mday = 31;
	break;
	case 4:
	case 6:
	case 9:
	case 11:
	if(timeeff->tm_mday > 30 )
         timeeff->tm_mday = 30;
	break;
	case 2:
	if((timeeff->tm_mday > 28 ) && (timeeff->tm_year % 4 != 0 ))
	  timeeff->tm_mday = 28;
	else if ((timeeff->tm_mday > 28 )) timeeff->tm_mday = 29;
	}
        *Commented switch case as not available in svn on 29JAN19*/


	*charged_to_t = mktime (timeeff);
//;
        sprintf(log_buf, "charged_from=%d; charged_to=%d", *charged_from_t,*charged_to_t );
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, log_buf);

	return;
}


/*********************************************************************************************************
*  
*  
*********************************************************************************************************/

static int32 
fm_mso_modify_for_mcf(
	pcm_context_t		*ctxp,
	pin_flist_t		**i_flistp, 
	pin_flist_t		**r_flistpp,
	cm_op_info_t		*opstackp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_billinfo_iflist = NULL;
	pin_flist_t		*srch_billinfo_oflist = NULL;
	pin_flist_t		*read_payinfo_iflist = NULL;
	pin_flist_t		*read_payinfo_oflist = NULL;
	pin_flist_t		*stack_flistp = NULL;
	pin_flist_t		*service_array = NULL;
	pin_flist_t		*bal_info = NULL;
	pin_flist_t		*results = NULL;
	pin_flist_t		*inv_info = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*update_billinfo_iflist = NULL;
	pin_flist_t		*update_billinfo_oflist = NULL;
	pin_flist_t		*catv_info = NULL;
	pin_flist_t		*svc_read_in_flistp = NULL;
	pin_flist_t		*svc_read_out_flistp = NULL;
	pin_flist_t		*bb_info = NULL;

	poid_t			*acnt_pdp = NULL;
	poid_t			*service_pdp = NULL;
	poid_t			*ar_billinfo_obj = NULL;

	int64			db = -1;
	int32			search_flags = 256;
	int32			activate_customer_failure = 1;
	int32			stack_opcode = 0;
	int32			flg_called_by_activation  = 0;
	int32			flg_called_by_billing     = 0;
	int32			flg_called_by_add_plan    = 0;
	int32			flg_called_by_cancel_plan = 0;
	int32			flg_called_by_change_plan = 0;
	int32			flg_called_by_suspend_srvc= 0;
	int32			flg_called_by_terminate_srvc=0;
	int32			flg_called_by_reactivate_srvc=0;
	int32			flg_called_by_bill_now=0;
	int32			flg_called_by_hold_svc=0;
	int32			flg_called_by_unhold_svc=0;
	int32                   flg_called_by_amc_advance = 0;
	pin_flist_t		*amc_iflistp = NULL;
	time_t           	amc_current_t = 0;
	time_t                  *amc_cycle_end_t = NULL;
	time_t			amc_default_time = 0;
	int32			prepaid_indicator = 0;
	int32			srch_flags = 256;
	int32			conn_type = -1;
	int32			partial_bill = 0;
	int32			*subscriber_type = NULL;

	char			log_buf[255];
	char			*prog_name = NULL;
	char			*service_pdp_type = NULL;
	char			billinfo_id[20];
	time_t			*next_bill_t = NULL;
	time_t			*future_bill_t = NULL;
	time_t			*actg_next_t = NULL;

	void			*vp = NULL;
	struct tm		*timeeff;
	int32			actg_cycle_dom = 1;
	int32			charged_day = 0;
	int32			bill_when = 0;
	time_t          *charged_from_t = NULL;
	time_t          charged_to_t = 0;
	int32			is_bb_service = 0;
	char                    *event_type = NULL;
	int32			has_change_plan = 0;
	int32 			status = 10100;
	char			*action = NULL;	
	int32			unhold = 0;	
	pin_flist_t		*ret_flistpp = NULL;
	poid_t			*prod_pdp = NULL;
	int32			*count = NULL;
	int32			*unit = NULL;
	pin_cookie_t    	c_cookie = NULL;
        int32           	c_elem_id = 0;	
	pin_flist_t		*cycle_fees_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	int32			*fup_check = NULL;
	int32			amc_cancel = 0;
	int32			amc_purchase = 0;
	int			*pkg_id_n = NULL;
	int			*pkg_id_o = NULL;
	poid_t			*purc_pdp = NULL;
	poid_t			*product_pdp = NULL;
	time_t			*purch_start = NULL;
	char			*prov_tag = NULL;
	pin_flist_t		*write_in_flistp = NULL;
	pin_flist_t		*write_out_flistp = NULL;

 	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_for_mcf : Error");
		return 0;
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_for_mcf : Start");

	db = PIN_POID_GET_ID(PIN_FLIST_FLD_GET(*i_flistp, PIN_FLD_POID, 0, ebufp));
	charged_from_t = PIN_FLIST_FLD_GET(*i_flistp, PIN_FLD_CHARGED_FROM_T, 1, ebufp);
	event_type = PIN_FLIST_FLD_GET(*i_flistp, PIN_FLD_EVENT_TYPE, 1, ebufp); 
	prod_pdp = PIN_FLIST_FLD_GET(*i_flistp, PIN_FLD_OFFERING_OBJ, 1, ebufp);
	svc_read_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_OFFERING_OBJ,svc_read_in_flistp,PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(svc_read_in_flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(svc_read_in_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, svc_read_in_flistp, &svc_read_out_flistp, ebufp);
	acnt_pdp = PIN_FLIST_FLD_GET(svc_read_out_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	service_pdp = PIN_FLIST_FLD_GET(svc_read_out_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
	service_pdp_type = (char*)PIN_POID_GET_TYPE(service_pdp);
	if(strcmp (service_pdp_type,"/service/telco/broadband")==0) {
	    is_bb_service = 1;
	}

	if (service_pdp_type && is_bb_service) {

		while(opstackp != NULL)
		{
			stack_opcode = opstackp->opcode;
			stack_flistp = opstackp->in_flistp;
			//if(stack_opcode == 10017)
			//{
			//		break;
			//}
			memset(log_buf, '\0', sizeof(log_buf));
			sprintf(log_buf, "Current opcode: %d, ", stack_opcode);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, log_buf);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "cm_op_info_t stack input flist is ", stack_flistp);
			action = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_ACTION, 1, ebufp);
			if(action &&(strstr(action, "UNHOLD_SERVICES")))
			{
				PIN_FLIST_FLD_SET(*i_flistp, PIN_FLD_ACTION, (int *)action, ebufp);	
			}
			prog_name = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
						
			/* AMC POSTPAID CASE START */
                        if(event_type &&
                             (strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_hw_amc/cm/post") == 0
                              ||strcmp(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_hw_amc/dcm/post") == 0))
                        {
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "AMC ADVANCE SCENARIO");
                                flg_called_by_amc_advance = 1;
                                break;
                        }
                        /* AMC POSTPAID CASE END */


			if (stack_opcode == 102)
			{
				/*************************************************************************
				* if called by billing and opcode=102(PCM_OP_BILL_CYCLE_FORWARD)
				*************************************************************************/
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Billing input flist is ", stack_flistp);
				bal_info = PIN_FLIST_ELEM_GET(stack_flistp, PIN_FLD_BAL_INFO, 0, 1, ebufp);
				if (bal_info)
				{
					results = PIN_FLIST_ELEM_GET(bal_info, PIN_FLD_RESULTS, 0, 1, ebufp); 
					if (results)
					{
						//service_array=PIN_FLIST_ELEM_GET(results, PIN_FLD_SERVICES, 0, 1, ebufp);
						//if (service_array)
						//{
							//commented becuase it picks wrong service poid in case of two services
							//service_pdp=PIN_FLIST_FLD_GET(results, PIN_FLD_SERVICE_OBJ, 1, ebufp);
							acnt_pdp = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_POID, 0, ebufp);
							flg_called_by_billing = 1;
							break;
						//}
					}
				}
			}
			else if(stack_opcode == PCM_OP_CUST_SET_PRODUCT_STATUS)
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Set product status flist called . Capture the service and account object to be used if this is activation flow",stack_flistp);
				service_pdp = PIN_FLIST_FLD_GET(stack_flistp,PIN_FLD_SERVICE_OBJ,0, ebufp);
				acnt_pdp = PIN_FLIST_FLD_GET(stack_flistp,PIN_FLD_POID,0,ebufp);
				// Do not set the flag or break. Go through the loop to determine if this is a part of activation call or not.
				// acct and svc poids will be overwritten anyways in other checks.
			}
			else if(stack_opcode == 11208)
			{
					/*****If it is Validity extension flow then handling the charged_from details from current purchased_product_t for FUP grant**/
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Validity extension flist input flist is ", stack_flistp);
					fm_mso_get_poid_details(ctxp, service_pdp, &ret_flistpp, ebufp);
					args_flistp = PIN_FLIST_ELEM_GET(ret_flistpp, MSO_FLD_BB_INFO, PIN_ELEMID_ANY, 0, ebufp );
					fup_check = PIN_FLIST_FLD_GET(args_flistp, MSO_FLD_FUP_STATUS, 1, ebufp );
					PIN_FLIST_FLD_COPY(stack_flistp, PIN_FLD_END_T, *i_flistp, PIN_FLD_END_T, ebufp );
					PIN_FLIST_FLD_SET(*i_flistp, MSO_FLD_FUP_STATUS, fup_check, ebufp);
					if(fup_check &&  (*fup_check == 1 || *fup_check == 2) && event_type && strstr (event_type,"grant"))
					{
						PIN_FLIST_DESTROY_EX(&ret_flistpp, NULL);
						c_cookie = NULL;
						fm_mso_get_poid_details(ctxp, prod_pdp, &ret_flistpp, ebufp);
						while ((cycle_fees_flistp = PIN_FLIST_ELEM_GET_NEXT(ret_flistpp, PIN_FLD_CYCLE_FEES,
						&c_elem_id, 1, &c_cookie, ebufp)) != NULL)
						{
							count = PIN_FLIST_FLD_GET(cycle_fees_flistp, PIN_FLD_COUNT, 1, ebufp);
							unit  = PIN_FLIST_FLD_GET(cycle_fees_flistp, PIN_FLD_UNIT, 1, ebufp);

							if(count && unit && *unit == 0 &&  *count == 1)
							{
								charged_from_t = PIN_FLIST_FLD_GET(cycle_fees_flistp, PIN_FLD_CHARGED_FROM_T, 0, ebufp);
								PIN_FLIST_FLD_SET(*i_flistp, PIN_FLD_CHARGED_FROM_T, charged_from_t, ebufp);
								PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CHARGED_FROM_T, charged_from_t, ebufp);
							}
						}

					}
			}
			else if(stack_opcode == MSO_OP_PROV_BB_PROCESS_RESPONSE ||
				( stack_opcode == MSO_OP_CUST_ACTIVATE_BB_SERVICE && event_type && 
				( strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_hw_rental") ||
				strstr( event_type, "/event/billing/product/fee/cycle/cycle_forward_bimonthly/mso_hw_rental" ) ||
				strstr( event_type, "/event/billing/product/fee/cycle/cycle_forward_quarterly/mso_hw_rental" ) ||
				strstr( event_type, "/event/billing/product/fee/cycle/cycle_forward_quadmonthly/mso_hw_rental" ) ||
				strstr( event_type, "/event/billing/product/fee/cycle/cycle_forward_semiannual/mso_hw_rental" ) ||
				strstr( event_type, "/event/billing/product/fee/cycle/cycle_forward_annual/mso_hw_rental" ))))
								// Will be called only upon activation 
			{
				/*************************************************************************
				* if called by activation and opcode=11205()
				*************************************************************************/
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Activation input flist is ", stack_flistp);
					action = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_ACTION, 1, ebufp);
					if(action && (strstr(action,"RENEW_SUBSCRIBER") || strstr(action,"CHANGE_SERVICES") || (strstr(action, "ACTIVATE_SUBSCRIBER") && !strstr(action, "DEACTIVATE_SUBSCRIBER"))))
					{
						product_pdp = PIN_FLIST_FLD_GET(*i_flistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
						fm_mso_get_poid_details(ctxp, product_pdp, &ret_flistpp, ebufp);
						if (PIN_ERRBUF_IS_ERR(ebufp))
                                        	{
                                                	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error after returning from function fm_mso_get_poid_details", ebufp);
                                                	return;
                                        	}
						prov_tag = PIN_FLIST_FLD_GET(ret_flistpp, PIN_FLD_PROVISIONING_TAG, 0, ebufp);
						if(prov_tag && strlen(prov_tag) > 0)
						{
							/*PIN_FLIST_DESTROY_EX(&ret_flistpp, NULL);
							fm_mso_get_poid_details(ctxp, prod_pdp, &ret_flistpp, ebufp);
							if (PIN_ERRBUF_IS_ERR(ebufp))
                                                	{
                                                        	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error after returning from function fm_mso_get_poid_details1", ebufp);
                                                        	return;
                                                	}
							purch_start = PIN_FLIST_FLD_GET(ret_flistpp, PIN_FLD_PURCHASE_START_T, 0, ebufp);*/
							write_in_flistp = PIN_FLIST_CREATE(ebufp);
							PIN_FLIST_FLD_SET(write_in_flistp, PIN_FLD_POID, service_pdp, ebufp);
							args_flistp = PIN_FLIST_SUBSTR_ADD(write_in_flistp, MSO_FLD_BB_INFO, ebufp );
							PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_RENEW_T, charged_from_t, ebufp);
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Write flds Input Flist", write_in_flistp);
							PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_in_flistp, &write_out_flistp, ebufp);
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Write flds output flist", write_out_flistp);
							if (PIN_ERRBUF_IS_ERR(ebufp))
							{
								PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in updating mso_plans_activation status", ebufp);
								return;
							}
						}
						PIN_FLIST_DESTROY_EX(&ret_flistpp, NULL);
					}
					flg_called_by_activation = 1;
					break;
			}
			else if ( stack_opcode == 11007)
			{
				/*************************************************************************
				* if called by add plan
				*************************************************************************/
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "add plan input flist is ", stack_flistp);
				service_pdp = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp );
				acnt_pdp = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_POID, 0, ebufp);
				subscriber_type = (int32*)PIN_FLIST_FLD_GET(stack_flistp, MSO_FLD_SUBSCRIBER_TYPE, 1, ebufp );
				if (service_pdp)
				{
					flg_called_by_add_plan = 1;
					break;
				}
			}
			else if ( stack_opcode == 11006)
			{
				/*************************************************************************
				* if called by cancel plan
				*************************************************************************/
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "cancel plan input flist is ", stack_flistp);
				service_pdp = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp );
				acnt_pdp = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_POID, 0, ebufp);
				if (service_pdp)
				{
					
					PIN_FLIST_FLD_SET(*i_flistp, PIN_FLD_ACTION, "CANCEL", ebufp);
					if(event_type && !strstr(event_type,"mso_hw_rental/ip"))
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Not a ip rental plan");
						flg_called_by_cancel_plan = 1;
						break;
					}
				}
			}
			else if ( stack_opcode == 11005)
			{
				/*************************************************************************
				* if called by change plan
				*************************************************************************/
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "change plan input flist is ", stack_flistp);
				service_pdp = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp );
				acnt_pdp = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_POID, 0, ebufp);
				if (service_pdp)
				{
					flg_called_by_change_plan = 1;
					//flg_called_by_cancel_plan = 1;
					break;
				}
			}
			else if ( stack_opcode == 11008)
			{
				/*************************************************************************
				* if called by suspend service
				*************************************************************************/
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "suspend service input flist is ", stack_flistp);
				service_pdp = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp );
				acnt_pdp = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_POID, 0, ebufp);
				if (service_pdp)
				{
					flg_called_by_suspend_srvc = 1;
					break;
				}
			}
			else if ( stack_opcode == 11009)
			{
				/*************************************************************************
				* if called by terminate service
				*************************************************************************/
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "terminate service input flist is ", stack_flistp);
				service_pdp = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp );
				acnt_pdp = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_POID, 0, ebufp);
				if (service_pdp)
				{
					flg_called_by_terminate_srvc = 1;
					break;
				}
			}
			else if ( stack_opcode == 11010)
			{
				/*************************************************************************
				* if called by reactivate service
				*************************************************************************/
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "reactivate service input flist is ", stack_flistp);
				service_pdp = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp );
				acnt_pdp = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_POID, 0, ebufp);
				if (service_pdp)
				{
					flg_called_by_reactivate_srvc = 1;
					break;
				}
			}
			else if ( stack_opcode == 13200) // MSO_OP_BILL_MAKE_BILL_NOW
			{
				/*************************************************************************
				* if called by reactivate service
				*************************************************************************/
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_bill_make_now input flist is ", stack_flistp);
				service_pdp = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp );
				acnt_pdp = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
				if (service_pdp)
				{
					flg_called_by_bill_now = 1;
					break;
				}
			}
			else if(stack_opcode == MSO_OP_CUST_HOLD_SERVICE) {
				/*************************************************************************
				* if called by hold service
				*************************************************************************/
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_HOLD_SERVICE flist is ", stack_flistp);
				service_pdp = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp );
				acnt_pdp = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_POID, 0, ebufp);
				if (service_pdp)
				{
					flg_called_by_hold_svc = 1;
					break;
				}
			}
			else if(stack_opcode == MSO_OP_CUST_UNHOLD_SERVICE) {
				/*************************************************************************
				* if called by hold service
				*************************************************************************/
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_UNHOLD_SERVICE flist is ", stack_flistp);
				service_pdp = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp );
				acnt_pdp = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_POID, 0, ebufp);
				if (service_pdp)
				{
					flg_called_by_unhold_svc = 1;
					break;
				}
			}
			opstackp = opstackp->next;
		}
		strcpy(billinfo_id, "BB");
	}else { // CaTV
		while(opstackp != NULL)
		{
			stack_opcode = opstackp->opcode;
			stack_flistp = opstackp->in_flistp;
			//if(stack_opcode == 10017)
			//{
			//		break;
			//}
			memset(log_buf, '\0', sizeof(log_buf));
			sprintf(log_buf, "Current opcode: %d, ", stack_opcode);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, log_buf);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "cm_op_info_t stack input flist is ", stack_flistp);

			prog_name = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_BILLINFO_OBJ, 1, ebufp);

			if (stack_opcode == 102 && (prog_name && strcmp(prog_name, "pin_bill_accts")))
			{
				/*************************************************************************
				* if called by billing and opcode=102(PCM_OP_BILL_CYCLE_FORWARD)
				*************************************************************************/
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Billing input flist is ", stack_flistp);
				bal_info = PIN_FLIST_ELEM_GET(stack_flistp, PIN_FLD_BAL_INFO, 0, 1, ebufp);
				if (bal_info)
				{
					results = PIN_FLIST_ELEM_GET(bal_info, PIN_FLD_RESULTS, 0, 1, ebufp); 
					if (results)
					{
						//service_array=PIN_FLIST_ELEM_GET(results, PIN_FLD_SERVICES, 0, 1, ebufp);
						//if (service_array)
						//{
							service_pdp=PIN_FLIST_FLD_GET(results, PIN_FLD_SERVICE_OBJ, 1, ebufp);
							acnt_pdp = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_POID, 0, ebufp);
							flg_called_by_billing = 1;
							break;
						//}
					}
				}
			}
			else if(stack_opcode == 11002 )
			{
				/*************************************************************************
				* if called by activation and opcode=11002()
				*************************************************************************/
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Activation input flist is ", stack_flistp);
				service_array = PIN_FLIST_ELEM_GET(stack_flistp, PIN_FLD_SERVICES, 0, 1, ebufp );
				if (service_array)
				{
					service_pdp=PIN_FLIST_FLD_GET(service_array, PIN_FLD_SERVICE_OBJ, 0, ebufp);
					acnt_pdp = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp); 
					catv_info = PIN_FLIST_SUBSTR_GET(service_array, MSO_FLD_CATV_INFO, 1, ebufp);
					vp = PIN_FLIST_FLD_GET(catv_info, PIN_FLD_CONN_TYPE, 1, ebufp);
					if (vp && *(int32*)vp == 1)
					{
						conn_type = 1;
					}
					flg_called_by_activation = 1;
					break;
				}
				
			}
			else if ( stack_opcode == 11007)
			{
				/*************************************************************************
				* if called by add plan
				*************************************************************************/
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "add plan input flist is ", stack_flistp);
				service_pdp = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp );
				acnt_pdp = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_POID, 0, ebufp);
				subscriber_type = (int32*)PIN_FLIST_FLD_GET(stack_flistp, MSO_FLD_SUBSCRIBER_TYPE, 1, ebufp );
				if (service_pdp)
				{
					flg_called_by_add_plan = 1;
					break;
				}
			}
			else if ( stack_opcode == 11006)
			{
				/*************************************************************************
				* if called by cancel plan
				*************************************************************************/
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "cancel plan input flist is ", stack_flistp);
				service_pdp = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp );
				acnt_pdp = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_POID, 0, ebufp);
				if (service_pdp)
				{
					flg_called_by_cancel_plan = 1;
					break;
				}
			}
			else if ( stack_opcode == 11005)
			{
				/*************************************************************************
				* if called by change plan
				*************************************************************************/
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "change plan input flist is ", stack_flistp);
				service_pdp = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp );
				acnt_pdp = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_POID, 0, ebufp);
				if (service_pdp)
				{
					flg_called_by_cancel_plan = 1;
					break;
				}
			}
			else if ( stack_opcode == 11008)
			{
				/*************************************************************************
				* if called by suspend service
				*************************************************************************/
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "suspend service input flist is ", stack_flistp);
				service_pdp = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp );
				acnt_pdp = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_POID, 0, ebufp);
				if (service_pdp)
				{
					flg_called_by_suspend_srvc = 1;
					break;
				}
			}
			else if ( stack_opcode == 11009)
			{
				/*************************************************************************
				* if called by terminate service
				*************************************************************************/
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "terminate service input flist is ", stack_flistp);
				service_pdp = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp );
				acnt_pdp = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_POID, 0, ebufp);
				if (service_pdp)
				{
					flg_called_by_terminate_srvc = 1;
					break;
				}
			}
			else if ( stack_opcode == 11010)
			{
				/*************************************************************************
				* if called by reactivate service
				*************************************************************************/
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "reactivate service input flist is ", stack_flistp);
				service_pdp = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp );
				acnt_pdp = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_POID, 0, ebufp);
				if (service_pdp)
				{
					flg_called_by_reactivate_srvc = 1;
					break;
				}
			}			
			opstackp = opstackp->next;
		}
		strcpy(billinfo_id, "CATV");
	}


	
	if (!service_pdp || !acnt_pdp)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Unable to find Account/Service");
		return 0;
	}

	//Search BILLINFO Start
	/*service_pdp_type = (char*)PIN_POID_GET_TYPE(service_pdp);
	if (service_pdp_type && strcmp (service_pdp_type,"/service/catv")==0)
	{
		strcpy(billinfo_id, "CATV");
	}
	else
	{
		strcpy(billinfo_id, "BB");
	}*/


	db = PIN_POID_GET_DB(acnt_pdp);
	srch_billinfo_iflist = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_billinfo_iflist, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_billinfo_iflist, PIN_FLD_FLAGS, &srch_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_billinfo_iflist, PIN_FLD_TEMPLATE, "select X from /billinfo  where F1.id = V1 and F2 = V2 and F3 = V3", ebufp);
	//PIN_FLIST_FLD_SET(srch_billinfo_iflist, PIN_FLD_TEMPLATE, "select X from /billinfo  where F1.id = V1 and F2 = V2 ", ebufp);
	arg_flist = PIN_FLIST_ELEM_ADD(srch_billinfo_iflist, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_billinfo_iflist, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_BILLINFO_ID, billinfo_id, ebufp);
	
	 //Added to pick only active billinfo's
        arg_flist = PIN_FLIST_ELEM_ADD(srch_billinfo_iflist, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STATUS,&status, ebufp);


	result_flist = PIN_FLIST_ELEM_ADD(srch_billinfo_iflist, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_NEXT_BILL_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACTG_CYCLE_DOM, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_FUTURE_BILL_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACTG_NEXT_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_PAYINFO_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_PAY_TYPE, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_BILLING_STATE, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_AR_BILLINFO_OBJ, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_for_mcf search billinfo input list", srch_billinfo_iflist);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_billinfo_iflist, &srch_billinfo_oflist, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_billinfo_iflist, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_FLIST_DESTROY_EX(&srch_billinfo_oflist, NULL);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_for_mcf search billinfo output flist", srch_billinfo_oflist);
	result_flist = PIN_FLIST_ELEM_GET(srch_billinfo_oflist, PIN_FLD_RESULTS, 0, 1, ebufp);

	if (result_flist)
	{	/*********************************************************************************
		* If the thccount is non-paying child then use the /payinfo of  parent
		*********************************************************************************/
		partial_bill = *((int32*)PIN_FLIST_FLD_GET(result_flist, PIN_FLD_BILLING_STATE, 0, ebufp));
		actg_next_t = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_ACTG_NEXT_T, 1, ebufp);
		next_bill_t = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_NEXT_BILL_T, 1, ebufp);
		
		if (*(int32*)PIN_FLIST_FLD_GET(result_flist, PIN_FLD_PAY_TYPE, 0, ebufp) == 10007 && !is_bb_service )
		{
			ar_billinfo_obj = PIN_POID_COPY((poid_t*)PIN_FLIST_FLD_GET(result_flist, PIN_FLD_AR_BILLINFO_OBJ, 0, ebufp) , ebufp);
			PIN_FLIST_DESTROY_EX(&srch_billinfo_oflist, NULL);
			PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "ar_billinfo_obj", ar_billinfo_obj);

			srch_billinfo_iflist = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(srch_billinfo_iflist, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(srch_billinfo_iflist, PIN_FLD_FLAGS, &srch_flags, ebufp);
			PIN_FLIST_FLD_SET(srch_billinfo_iflist, PIN_FLD_TEMPLATE, "select X from /billinfo  where F1.id = V1 ", ebufp);

			arg_flist = PIN_FLIST_ELEM_ADD(srch_billinfo_iflist, PIN_FLD_ARGS, 1, ebufp);
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, ar_billinfo_obj, ebufp);

			result_flist = PIN_FLIST_ELEM_ADD(srch_billinfo_iflist, PIN_FLD_RESULTS, 0, ebufp);
			PIN_FLIST_FLD_SET(result_flist, PIN_FLD_NEXT_BILL_T, NULL, ebufp);
			PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACTG_CYCLE_DOM, NULL, ebufp);
			PIN_FLIST_FLD_SET(result_flist, PIN_FLD_FUTURE_BILL_T, NULL, ebufp);
			PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACTG_NEXT_T, NULL, ebufp);
			PIN_FLIST_FLD_SET(result_flist, PIN_FLD_PAYINFO_OBJ, NULL, ebufp);
			PIN_FLIST_FLD_SET(result_flist, PIN_FLD_PAY_TYPE, NULL, ebufp);
			PIN_FLIST_FLD_SET(result_flist, PIN_FLD_BILLING_STATE, NULL, ebufp);
			PIN_FLIST_FLD_SET(result_flist, PIN_FLD_AR_BILLINFO_OBJ, NULL, ebufp);


			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "parent: fm_mso_modify_for_mcf search billinfo input list", srch_billinfo_iflist);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_billinfo_iflist, &srch_billinfo_oflist, ebufp);
			PIN_FLIST_DESTROY_EX(&srch_billinfo_iflist, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
				PIN_FLIST_DESTROY_EX(&srch_billinfo_oflist, NULL);
				return 0;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "parent:fm_mso_modify_for_mcf search billinfo output flist", srch_billinfo_oflist);
			result_flist = PIN_FLIST_ELEM_GET(srch_billinfo_oflist, PIN_FLD_RESULTS, 0, 1, ebufp);
		}
	}

 	if (result_flist)
	{
		//Read PAYINFO Start
		read_payinfo_iflist = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_PAYINFO_OBJ, read_payinfo_iflist, PIN_FLD_POID, ebufp);
		PIN_FLIST_ELEM_SET(read_payinfo_iflist, NULL, PIN_FLD_INV_INFO, 0, ebufp );

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_for_mcf read payinfo input list", read_payinfo_iflist);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_payinfo_iflist, &read_payinfo_oflist, ebufp);
		PIN_FLIST_DESTROY_EX(&read_payinfo_iflist, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
			PIN_FLIST_DESTROY_EX(&read_payinfo_oflist, NULL);
			return 0;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_for_mcf read payinfo output flist", read_payinfo_oflist);
		if (read_payinfo_oflist)
		{
			inv_info = PIN_FLIST_ELEM_GET(read_payinfo_oflist, PIN_FLD_INV_INFO, 0, 1, ebufp);
			prepaid_indicator = *(int32 *)PIN_FLIST_FLD_GET(inv_info, PIN_FLD_INDICATOR, 0, ebufp);
			PIN_FLIST_FLD_SET(*i_flistp, PIN_FLD_INDICATOR, &prepaid_indicator, ebufp);
			
		}
		//PIN_FLIST_DESTROY_EX(&read_payinfo_oflist, NULL);
		//Read PAYINFO End

		if ((prepaid_indicator == PREPAID) || (!is_bb_service && prepaid_indicator == CATV_PREPAID))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "prepaid_indicator == PREPAID");
			if (flg_called_by_activation && conn_type ==1 )	//Addl connection
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "called_by_activation");
			  	if (partial_bill == 1 && (*actg_next_t == *next_bill_t))
				{

					PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_FUTURE_BILL_T, *i_flistp, PIN_FLD_CHARGED_TO_T , ebufp );
					PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_FUTURE_BILL_T, *i_flistp, PIN_FLD_CYCLE_FEE_END_T , ebufp );

				}
				else
				{

					PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_NEXT_BILL_T, *i_flistp, PIN_FLD_CHARGED_TO_T , ebufp );
					PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_NEXT_BILL_T, *i_flistp, PIN_FLD_CYCLE_FEE_END_T , ebufp );				
				}
			}
			else if (flg_called_by_billing)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "called_by_billing");
				PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_FUTURE_BILL_T,   *i_flistp, PIN_FLD_CHARGED_TO_T , ebufp );
				PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_FUTURE_BILL_T,   *i_flistp, PIN_FLD_CYCLE_FEE_END_T , ebufp );

			}
			else if (flg_called_by_cancel_plan)
			{
				if (partial_bill == 1 && (*actg_next_t == *next_bill_t))
                   {
                           /*PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "called_by_cancel_plan");
                           PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_NEXT_BILL_T,   *i_flistp, PIN_FLD_CHARGED_TO_T , ebufp );
                           //PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_NEXT_BILL_T,   *i_flistp, PIN_FLD_CYCLE_FEE_END_T , ebufp );
                           PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_END_T,   *i_flistp, PIN_FLD_CYCLE_FEE_END_T , ebufp );
                           PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_CHARGED_FROM_T,   *i_flistp, PIN_FLD_CYCLE_FEE_START_T , ebufp );*/

                           PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "called_by_cancel_plan");
                           PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_FUTURE_BILL_T,   *i_flistp, PIN_FLD_CHARGED_TO_T , ebufp );
                           PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_FUTURE_BILL_T,   *i_flistp, PIN_FLD_CYCLE_FEE_END_T , ebufp );
                           PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_CHARGED_FROM_T,   *i_flistp, PIN_FLD_CYCLE_FEE_START_T , ebufp );
                   }
                   else
                   {
                           /*PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "called_by_cancel_plan");
                           PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_FUTURE_BILL_T,   *i_flistp, PIN_FLD_CHARGED_TO_T , ebufp );
                           PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_FUTURE_BILL_T,   *i_flistp, PIN_FLD_CYCLE_FEE_END_T , ebufp );
                           PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_CHARGED_FROM_T,   *i_flistp, PIN_FLD_CYCLE_FEE_START_T , ebufp );*/

                           PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "called_by_cancel_plan");
                           PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_NEXT_BILL_T,   *i_flistp, PIN_FLD_CHARGED_TO_T , ebufp );
                           //PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_NEXT_BILL_T,   *i_flistp, PIN_FLD_CYCLE_FEE_END_T , ebufp );
                           PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_END_T,   *i_flistp, PIN_FLD_CYCLE_FEE_END_T , ebufp );
                           PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_CHARGED_FROM_T,   *i_flistp, PIN_FLD_CYCLE_FEE_START_T , ebufp ); 
                  }

			}
			else if (flg_called_by_add_plan)
			{
				if (partial_bill == 1 && (*actg_next_t == *next_bill_t))
				{
					/*PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "called_by_add_plan");
					PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_NEXT_BILL_T,   *i_flistp, PIN_FLD_CHARGED_TO_T , ebufp );
					PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_NEXT_BILL_T,   *i_flistp, PIN_FLD_CYCLE_FEE_END_T , ebufp );
					PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_CHARGED_FROM_T,   *i_flistp, PIN_FLD_CYCLE_FEE_START_T , ebufp );*/

					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "called_by_add_plan1");
					PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_FUTURE_BILL_T,   *i_flistp, PIN_FLD_CHARGED_TO_T , ebufp );
					PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_FUTURE_BILL_T,   *i_flistp, PIN_FLD_CYCLE_FEE_END_T , ebufp );
					PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_CHARGED_FROM_T,   *i_flistp, PIN_FLD_CYCLE_FEE_START_T , ebufp );
				}
				else
				{
					/*PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "called_by_add_plan1");
					PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_FUTURE_BILL_T,   *i_flistp, PIN_FLD_CHARGED_TO_T , ebufp );
					PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_FUTURE_BILL_T,   *i_flistp, PIN_FLD_CYCLE_FEE_END_T , ebufp );
					PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_CHARGED_FROM_T,   *i_flistp, PIN_FLD_CYCLE_FEE_START_T , ebufp );*/

					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "called_by_add_plan");
					PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_NEXT_BILL_T,   *i_flistp, PIN_FLD_CHARGED_TO_T , ebufp );
					PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_NEXT_BILL_T,   *i_flistp, PIN_FLD_CYCLE_FEE_END_T , ebufp );
					PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_CHARGED_FROM_T,   *i_flistp, PIN_FLD_CYCLE_FEE_START_T , ebufp );
				}
			}
			else if (flg_called_by_change_plan && !is_bb_service)
			{
				if (partial_bill == 1 && (*actg_next_t == *next_bill_t))
				{
					/*PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "called_by_change_plan");
					PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_NEXT_BILL_T,   *i_flistp, PIN_FLD_CHARGED_TO_T , ebufp );
					PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_NEXT_BILL_T,   *i_flistp, PIN_FLD_CYCLE_FEE_END_T , ebufp );
					PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_CHARGED_FROM_T,   *i_flistp, PIN_FLD_CYCLE_FEE_START_T , ebufp );*/

					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "called_by_change_plan");
					PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_FUTURE_BILL_T,   *i_flistp, PIN_FLD_CHARGED_TO_T , ebufp );
					PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_FUTURE_BILL_T,   *i_flistp, PIN_FLD_CYCLE_FEE_END_T , ebufp );
					PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_CHARGED_FROM_T,   *i_flistp, PIN_FLD_CYCLE_FEE_START_T , ebufp );
				}
				else
				{
					/*PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "called_by_change_plan");
					PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_FUTURE_BILL_T,   *i_flistp, PIN_FLD_CHARGED_TO_T , ebufp );
					PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_FUTURE_BILL_T,   *i_flistp, PIN_FLD_CYCLE_FEE_END_T , ebufp );
					PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_CHARGED_FROM_T,   *i_flistp, PIN_FLD_CYCLE_FEE_START_T , ebufp );*/

					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "called_by_change_plan");
					PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_NEXT_BILL_T,   *i_flistp, PIN_FLD_CHARGED_TO_T , ebufp );
					PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_NEXT_BILL_T,   *i_flistp, PIN_FLD_CYCLE_FEE_END_T , ebufp );
					PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_CHARGED_FROM_T,   *i_flistp, PIN_FLD_CYCLE_FEE_START_T , ebufp );
				}
			} 
			else if (flg_called_by_hold_svc)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "called_by_hold_svc. Do nothing.");
			}
			else if (flg_called_by_unhold_svc)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "called_by_unhold_svc. Do nothing");
			}
			else if (flg_called_by_suspend_srvc)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "called_by_suspend_srvc");
				//PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_END_T,   *i_flistp, PIN_FLD_CYCLE_FEE_START_T , ebufp );
				//PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_END_T,   *i_flistp, PIN_FLD_CYCLE_FEE_END_T , ebufp );
				//PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_END_T,   *i_flistp, PIN_FLD_CHARGED_TO_T , ebufp );
			}
			else if (flg_called_by_terminate_srvc)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "called_by_terminate_srvc");
				PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_END_T,   *i_flistp, PIN_FLD_CYCLE_FEE_START_T , ebufp );
				PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_END_T,   *i_flistp, PIN_FLD_CYCLE_FEE_END_T , ebufp );
				               //      PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_END_T,   *i_flistp, PIN_FLD_CHARGED_TO_T , ebufp );
                                if(is_bb_service)
                               {
                                       PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_END_T,   *i_flistp, PIN_FLD_CHARGED_TO_T , ebufp );
                               }

			}
			else if (flg_called_by_reactivate_srvc)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "called_by_reactivate_srvc");
				PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_CHARGED_FROM_T,   *i_flistp, PIN_FLD_CYCLE_FEE_START_T , ebufp );
			}
		}

		//	Broadband changes
		if (prepaid_indicator == ADVANCE && is_bb_service)
		{	
			timeeff = localtime(charged_from_t);
			actg_cycle_dom = *(int32 *)PIN_FLIST_FLD_GET(result_flist,PIN_FLD_ACTG_CYCLE_DOM,0, ebufp);
			charged_day = timeeff->tm_mday;

			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "prepaid_indicator == ADVANCE");
			if (flg_called_by_activation)	//Addl connection
			{						
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "called_by_activation");
				if (partial_bill == 1 && (*actg_next_t == *next_bill_t))
				{
					PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_FUTURE_BILL_T, *i_flistp, PIN_FLD_CHARGED_TO_T , ebufp );
					PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_FUTURE_BILL_T, *i_flistp, PIN_FLD_CYCLE_FEE_END_T , ebufp );
				}
				else
				{				
					// if current date is in middle of month, use next_bill_t. Otherwise, if it is DOM, then determine the 
					// BILL_WHEN and add payterm number of months to charge_from_t.					

					if(charged_day != actg_cycle_dom) {
						PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_NEXT_BILL_T,   *i_flistp, PIN_FLD_CHARGED_TO_T , ebufp );
						PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_NEXT_BILL_T,   *i_flistp, PIN_FLD_CYCLE_FEE_END_T , ebufp );				
					} else {
						bill_when = get_bill_when_from_service(ctxp, service_pdp, ebufp);
						timeeff->tm_mon = timeeff->tm_mon + bill_when;
						charged_to_t = mktime (timeeff);
						PIN_FLIST_FLD_SET(*i_flistp, PIN_FLD_CHARGED_TO_T ,&charged_to_t, ebufp );
						PIN_FLIST_FLD_SET(*i_flistp, PIN_FLD_CYCLE_FEE_END_T ,&charged_to_t, ebufp );
					}
				}
			}
			else if (flg_called_by_billing)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "called_by_billing");
				if(charged_day != actg_cycle_dom) {
				    PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_FUTURE_BILL_T,   *i_flistp, PIN_FLD_CHARGED_TO_T , ebufp );
					PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_FUTURE_BILL_T,   *i_flistp, PIN_FLD_CYCLE_FEE_END_T , ebufp );				
				} else {
					bill_when = get_bill_when_from_service(ctxp, service_pdp, ebufp);
					timeeff->tm_mon = timeeff->tm_mon + bill_when;
					charged_to_t = mktime (timeeff);
					PIN_FLIST_FLD_SET(*i_flistp, PIN_FLD_CHARGED_TO_T ,&charged_to_t, ebufp );
					PIN_FLIST_FLD_SET(*i_flistp, PIN_FLD_CYCLE_FEE_END_T ,&charged_to_t, ebufp );

				}
				
			}
			else if (flg_called_by_cancel_plan)
			{
				if (partial_bill == 1 && (*actg_next_t == *next_bill_t))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "called_by_cancel_plan");
					PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_FUTURE_BILL_T,   *i_flistp, PIN_FLD_CHARGED_TO_T , ebufp );
					PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_FUTURE_BILL_T,   *i_flistp, PIN_FLD_CYCLE_FEE_END_T , ebufp );
					PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_CHARGED_FROM_T,   *i_flistp, PIN_FLD_CYCLE_FEE_START_T , ebufp );
				}
				else
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "called_by_cancel_plan");
					if(charged_day != actg_cycle_dom) {
						PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_NEXT_BILL_T,   *i_flistp, PIN_FLD_CHARGED_TO_T , ebufp );
						PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_END_T,   *i_flistp, PIN_FLD_CYCLE_FEE_END_T , ebufp );
						PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_CHARGED_FROM_T,   *i_flistp, PIN_FLD_CYCLE_FEE_START_T , ebufp );				
					} else {
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Reading Bill When from Service");
                        bill_when = get_bill_when_from_service(ctxp, service_pdp, ebufp);
                        timeeff->tm_mon = timeeff->tm_mon + bill_when;
                        charged_to_t = mktime (timeeff);
                        PIN_FLIST_FLD_SET(*i_flistp, PIN_FLD_CHARGED_TO_T ,&charged_to_t, ebufp );
                        PIN_FLIST_FLD_SET(*i_flistp, PIN_FLD_CYCLE_FEE_END_T ,&charged_to_t, ebufp );
                        PIN_FLIST_FLD_SET(*i_flistp, PIN_FLD_CYCLE_FEE_START_T ,&charged_to_t,  ebufp );
					}
				}
					

				
			}
			else if (flg_called_by_add_plan)
			{
				if (partial_bill == 1 && (*actg_next_t == *next_bill_t))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "called_by_add_plan1");
					PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_FUTURE_BILL_T,   *i_flistp, PIN_FLD_CHARGED_TO_T , ebufp );
					PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_FUTURE_BILL_T,   *i_flistp, PIN_FLD_CYCLE_FEE_END_T , ebufp );
					PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_CHARGED_FROM_T,   *i_flistp, PIN_FLD_CYCLE_FEE_START_T , ebufp );
				}
				else
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "called_by_add_plan");
					if(charged_day != actg_cycle_dom) {
						PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_NEXT_BILL_T,   *i_flistp, PIN_FLD_CHARGED_TO_T , ebufp );
						PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_NEXT_BILL_T,   *i_flistp, PIN_FLD_CYCLE_FEE_END_T , ebufp );
						PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_CHARGED_FROM_T,   *i_flistp, PIN_FLD_CYCLE_FEE_START_T , ebufp );			
					} else {
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Reading Bill When from Service");
                        bill_when = get_bill_when_from_service(ctxp, service_pdp, ebufp);
                        timeeff->tm_mon = timeeff->tm_mon + bill_when;
                        charged_to_t = mktime (timeeff);
                        PIN_FLIST_FLD_SET(*i_flistp, PIN_FLD_CHARGED_TO_T ,&charged_to_t, ebufp );
                        PIN_FLIST_FLD_SET(*i_flistp, PIN_FLD_CYCLE_FEE_END_T ,&charged_to_t, ebufp );
                        PIN_FLIST_FLD_SET(*i_flistp, PIN_FLD_CYCLE_FEE_START_T ,&charged_to_t,  ebufp );					
					}
				}
			}
			else if (flg_called_by_change_plan)
			{
				/*if (partial_bill == 1 && (*actg_next_t == *next_bill_t))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "called_by_change_plan");
					PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_FUTURE_BILL_T,   *i_flistp, PIN_FLD_CHARGED_TO_T , ebufp );
					PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_FUTURE_BILL_T,   *i_flistp, PIN_FLD_CYCLE_FEE_END_T , ebufp );
					PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_CHARGED_FROM_T,   *i_flistp, PIN_FLD_CYCLE_FEE_START_T , ebufp );
				}
				else
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "called_by_change_plan");
					if(charged_day != actg_cycle_dom) { 
						PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_NEXT_BILL_T,   *i_flistp, PIN_FLD_CHARGED_TO_T , ebufp );
						PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_NEXT_BILL_T,   *i_flistp, PIN_FLD_CYCLE_FEE_END_T , ebufp );
						PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_CHARGED_FROM_T,   *i_flistp, PIN_FLD_CYCLE_FEE_START_T , ebufp );
					}else {
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Reading Bill When from Service");
                        bill_when = get_bill_when_from_service(ctxp, service_pdp, ebufp);
                        timeeff->tm_mon = timeeff->tm_mon + bill_when;
                        charged_to_t = mktime (timeeff);
                        PIN_FLIST_FLD_SET(*i_flistp, PIN_FLD_CHARGED_TO_T ,&charged_to_t, ebufp );
                        PIN_FLIST_FLD_SET(*i_flistp, PIN_FLD_CYCLE_FEE_END_T ,&charged_to_t, ebufp );
                        PIN_FLIST_FLD_SET(*i_flistp, PIN_FLD_CYCLE_FEE_START_T ,&charged_to_t,  ebufp );
					}
					
				}*/
			} 
			else if (flg_called_by_suspend_srvc)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "called_by_suspend_srvc");
				//PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_END_T,   *i_flistp, PIN_FLD_CYCLE_FEE_START_T , ebufp );
				//PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_END_T,   *i_flistp, PIN_FLD_CYCLE_FEE_END_T , ebufp );
				//PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_END_T,   *i_flistp, PIN_FLD_CHARGED_TO_T , ebufp );
			}
			else if (flg_called_by_terminate_srvc)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "called_by_terminate_srvc");
				/*
				PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_END_T,   *i_flistp, PIN_FLD_CYCLE_FEE_START_T , ebufp );
				PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_END_T,   *i_flistp, PIN_FLD_CYCLE_FEE_END_T , ebufp );
				PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_END_T,   *i_flistp, PIN_FLD_CHARGED_TO_T , ebufp );
				*/
			}
			else if (flg_called_by_reactivate_srvc)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "called_by_reactivate_srvc");
				//PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_CHARGED_FROM_T,   *i_flistp, PIN_FLD_CYCLE_FEE_START_T , ebufp );
			}
			/* AMC CHANGES START */
                        else if(flg_called_by_amc_advance)
                        {
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "flg_called_by_amc_advance");
				/**read the current purchased_product details**/
				purc_pdp = PIN_FLIST_FLD_GET(*i_flistp, PIN_FLD_OFFERING_OBJ, 1, ebufp);
				fm_mso_get_poid_details(ctxp, purc_pdp, &ret_flistpp, ebufp);
	
				while(opstackp != NULL)
				{
					stack_opcode = opstackp->opcode;
					stack_flistp = opstackp->in_flistp;
					if (stack_opcode == 11005)
					{
						has_change_plan = 1;
					}
					else if(stack_opcode == MSO_OP_CUST_CANCEL_PLAN)
					{
						prog_name = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
						if(prog_name && strstr(prog_name, "top_up_plan"))
						{
							arg_flist = PIN_FLIST_ELEM_GET(stack_flistp, PIN_FLD_PLAN_LISTS, PIN_ELEMID_ANY, 1, ebufp);
							if(arg_flist)
							{
								pkg_id_o = PIN_FLIST_FLD_GET(arg_flist, PIN_FLD_PACKAGE_ID, 1, ebufp);
								pkg_id_n = PIN_FLIST_FLD_GET(ret_flistpp, PIN_FLD_PACKAGE_ID, 1, ebufp);
								if(pkg_id_n && pkg_id_o && *pkg_id_n == *pkg_id_o)
								{
									amc_cancel = 1;
								}
							}
						}
					}
					else if(stack_opcode == PCM_OP_SUBSCRIPTION_PURCHASE_DEAL)
					{
						prog_name = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
						if(prog_name && (strcmp(prog_name,"MSO_OP_CUST_BB_HW_AMC") == 0))
						{
							amc_purchase = 1;
						}
					}
					else if(stack_opcode == MSO_OP_PROV_BB_PROCESS_RESPONSE)
					{
						/***Checking if the reponse is Unhold then for postpaid we have to return without changing the charged details***/ 
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Response input flist is ", stack_flistp);
						action = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_ACTION, 1, ebufp);
						if(action &&(strstr(action, "UNHOLD_SERVICES")))
						{
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "testt ", stack_flistp);
							unhold = 1;
						}
					}
					opstackp = opstackp->next;
				}

				PIN_FLIST_DESTROY_EX(&ret_flistpp, NULL);
				if(amc_cancel == 1 && amc_purchase == 0)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "This is cancel scenario: Roll back");
					return;
				}
				if(stack_opcode == MSO_OP_CUST_HOLD_SERVICE || stack_opcode == MSO_OP_CUST_TERMINATE_SERVICE || (unhold == 1 && prepaid_indicator == ADVANCE && is_bb_service))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "AMC HOLD/TERMINATE");
					return 1;
				}

				amc_iflistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(amc_iflistp, PIN_FLD_POID, acnt_pdp, ebufp);
				/* read subscription product cycle end date */
				fm_mso_cust_bb_hw_amc_get_cycle_details(ctxp, amc_iflistp, ebufp);

			        if (PIN_ERRBUF_IS_ERR(ebufp))
        			{
            				PIN_ERRBUF_CLEAR(ebufp);
					amc_default_time = pin_virtual_time(NULL);
					PIN_FLIST_FLD_SET(amc_iflistp, PIN_FLD_CYCLE_END_T, &amc_default_time, ebufp);

        			}

				amc_cycle_end_t = PIN_FLIST_FLD_GET(amc_iflistp, PIN_FLD_CYCLE_END_T, 0, ebufp);
				amc_current_t = pin_virtual_time(NULL);

                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "flg_called_by_amc_advance setting bill_when");
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Reading Bill When from Service");
                                bill_when = get_bill_when_from_service(ctxp, service_pdp, ebufp);
			
				if(amc_cycle_end_t && amc_current_t >= *amc_cycle_end_t)
				{
					timeeff = localtime(amc_cycle_end_t);
					timeeff->tm_mon = timeeff->tm_mon + bill_when;
					charged_to_t = mktime (timeeff);
				}
				else
				{
					timeeff = localtime(amc_cycle_end_t);
					charged_to_t = mktime (timeeff);
				}
                                PIN_FLIST_FLD_SET(*i_flistp, PIN_FLD_CHARGED_TO_T ,&charged_to_t, ebufp );
                                PIN_FLIST_FLD_SET(*i_flistp, PIN_FLD_CYCLE_FEE_END_T ,&charged_to_t, ebufp );
				PIN_FLIST_DESTROY_EX(&amc_iflistp, NULL);
				if (has_change_plan == 1 )
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "flg_called_by_amc_advance CHANGE PLAN");
                                	if (partial_bill == 1 && (*actg_next_t == *next_bill_t))
                                	{
                                        	PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_FUTURE_BILL_T,   
									*i_flistp, PIN_FLD_CHARGED_TO_T , ebufp );
                                        	PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_FUTURE_BILL_T,   
									*i_flistp, PIN_FLD_CYCLE_FEE_END_T , ebufp );
                                        	PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_CHARGED_FROM_T,   
									*i_flistp, PIN_FLD_CYCLE_FEE_START_T , ebufp );
                                	}
                                	else
                                	{
                                        	if(charged_day != actg_cycle_dom) 
						{
                                                	PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_NEXT_BILL_T,   
									*i_flistp, PIN_FLD_CHARGED_TO_T , ebufp );
                                                	PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_NEXT_BILL_T,   
									*i_flistp, PIN_FLD_CYCLE_FEE_END_T , ebufp );
                                                	PIN_FLIST_FLD_COPY(*i_flistp, PIN_FLD_CHARGED_FROM_T,   
									*i_flistp, PIN_FLD_CYCLE_FEE_START_T , ebufp );
                                        	}
						else 
						{
                                                	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Reading Bill When from Service");
                        				bill_when = get_bill_when_from_service(ctxp, service_pdp, ebufp);
                        				timeeff = localtime(charged_from_t);
							timeeff->tm_mon = timeeff->tm_mon + bill_when;
                        				charged_to_t = mktime (timeeff);
                        				PIN_FLIST_FLD_SET(*i_flistp, PIN_FLD_CHARGED_TO_T ,&charged_to_t, ebufp );
                        				PIN_FLIST_FLD_SET(*i_flistp, PIN_FLD_CYCLE_FEE_END_T ,&charged_to_t, ebufp );
                                        	}
                                	}
				}


                        } /* AMC CHANGES END */

		}
	}
	else
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Search returning NULL", ebufp);
		PIN_FLIST_DESTROY_EX(&srch_billinfo_oflist, NULL);
		return 0;
	}

	PIN_FLIST_DESTROY_EX(&srch_billinfo_oflist, NULL);
	PIN_FLIST_DESTROY_EX(&read_payinfo_oflist, NULL);
	return 1;
}



void read_prod_details(pcm_context_t		*ctxp,
		pin_flist_t		*i_flistp,
		pin_flist_t		**prod_flistp,
		pin_errbuf_t		*ebufp)
{
	pin_flist_t	*read_in_flistp = NULL;
	pin_flist_t	*read_out_flistp = NULL;
	poid_t		*prod_pdp = NULL;

	prod_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);

	read_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PRODUCT_OBJ, read_in_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(read_in_flistp, PIN_FLD_PRIORITY, NULL, ebufp);
	PIN_FLIST_FLD_SET(read_in_flistp, PIN_FLD_PERMITTED, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read_prod_details read product input list", read_in_flistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_in_flistp, &read_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read_prod_details read product output list", read_out_flistp);

	*prod_flistp = PIN_FLIST_COPY(read_out_flistp,ebufp);
	return;
}

void fm_mso_update_service_fup_status(
		cm_nap_connection_t     *connp,
		pcm_context_t		*ctxp,
		pin_flist_t		*i_flistp,
		pin_flist_t		*prod_flistp,
		char			*event_type,
		cm_op_info_t            *opstackp,
		pin_flist_t		*r_flistp,
		pin_errbuf_t		*ebufp) {

	poid_t	*product_pdp = NULL;
	poid_t		*acnt_pdp = NULL;
	char	*permitted=NULL;
	char	*broadband = "/service/telco/broadband";
	poid_t		*service_obj = NULL; 
	int32		flag = 0;
	pin_flist_t	*svc_upd_in_flistp = NULL;
	pin_flist_t	*svc_upd_out_flistp = NULL;
	pin_flist_t	*read_in_flistp = NULL;
	pin_flist_t	*read_out_flistp = NULL;
	pin_flist_t	*bb_info_flistp = NULL;
	char	msg[250];
	double	prod_priority = 0;
	int32	fup_status = BEF_FUP;
	int32 prepaid_fup_start = (BB_PREPAID_START + BB_UNLIMITED_FUP_RANGE_START);
	int32 prepaid_fup_end = (BB_PREPAID_START + BB_UNLIMITED_FUP_RANGE_END);
	int32 postpaid_fup_start = (BB_POSTPAID_START + BB_UNLIMITED_FUP_RANGE_START);
	int32 postpaid_fup_end = (BB_POSTPAID_START + BB_UNLIMITED_FUP_RANGE_END);
	int32 postpaid_lim_start = (BB_POSTPAID_START + BB_LIMITED_RANGE_START);
	int32 postpaid_lim_end = (BB_POSTPAID_START + BB_LIMITED_RANGE_END);
	int32 postpaid_unlim_start = (BB_POSTPAID_START + BB_UNLIMITED_NO_FUP_RANGE_START);
        int32 postpaid_unlim_end = (BB_POSTPAID_START + BB_UNLIMITED_NO_FUP_RANGE_END);
	char		*action = NULL;	
	pin_cookie_t    c_cookie = NULL;	
	int32           c_elem_id = 0;
	pin_flist_t	*cycle_fees_flistp = NULL;	
	time_t		*chrg_from = NULL;
	time_t		chrg_to;
	int32		*count = NULL;
	int32		*indictor = NULL;
	time_t		*chrg_frm = NULL;	
	int		an_local = 0;
	struct tm       *timeeff;	
	poid_t		*purch_pdp = NULL;
	int32		*count_inp = NULL;
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Reading PERMITTED");
	permitted = PIN_FLIST_FLD_GET(prod_flistp, PIN_FLD_PERMITTED, 0, ebufp);

	if(strcmp(event_type,"/event/billing/product/fee/cycle/cycle_forward_monthly/mso_grant") != 0  &&
	   strcmp(event_type,"/event/billing/product/fee/cycle/cycle_forward_thirty_days/mso_grant") != 0 &&	
	   strcmp(event_type,"/event/billing/product/fee/cycle/cycle_forward_monthly_fdp/mso_grant") != 0
	  ) 
	{
	    return;
	}

	if ((permitted != NULL) && strcmp(permitted, broadband) == 0)
	{	

                product_pdp = PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_PRODUCT_OBJ,1,ebufp);
		purch_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_OFFERING_OBJ, 1, ebufp);
		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Reading PRIORITY");
		prod_priority = pbo_decimal_to_double( PIN_FLIST_FLD_GET(prod_flistp, PIN_FLD_PRIORITY, 0, ebufp),ebufp);		
		prod_priority = ((int)prod_priority%1000);

		read_in_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_OFFERING_OBJ, read_in_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(read_in_flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);
		PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_in_flistp, &read_out_flistp, ebufp);
		service_obj = PIN_FLIST_FLD_GET(read_out_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_service_fup_status readobj purchased_product", read_out_flistp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Comparing PRIORITY");
		action = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTION, 1, ebufp);
		if(action != NULL)
		{
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, action);
                        svc_upd_in_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(svc_upd_in_flistp, PIN_FLD_POID, service_obj, ebufp);
                        bb_info_flistp = PIN_FLIST_ELEM_ADD(svc_upd_in_flistp, MSO_FLD_BB_INFO, 0, ebufp);
                        PIN_FLIST_FLD_SET(bb_info_flistp, PIN_FLD_INDICATOR, (int *)NULL, ebufp);
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_service_fup_status readflds service input list", svc_upd_in_flistp);
                        PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, svc_upd_in_flistp, &svc_upd_out_flistp, ebufp);
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_service_fup_status readflds service output list", svc_upd_out_flistp);
                        PIN_FLIST_DESTROY_EX(&svc_upd_in_flistp, NULL);
                        if(PIN_ERRBUF_IS_ERR(ebufp))
                        {
                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error calling service readflds ", ebufp);
                        }
			bb_info_flistp = PIN_FLIST_ELEM_GET(svc_upd_out_flistp, MSO_FLD_BB_INFO, PIN_ELEMID_ANY, 1, ebufp);
                        indictor = PIN_FLIST_FLD_GET(bb_info_flistp, PIN_FLD_INDICATOR, 1, ebufp);
			
                        if(indictor && *indictor == 1)
                        {
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "check111");
				chrg_frm = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CHARGED_FROM_T, 1, ebufp);
				c_cookie = NULL;
			   while ((cycle_fees_flistp = PIN_FLIST_ELEM_GET_NEXT(read_out_flistp, PIN_FLD_CYCLE_FEES,
                                        &c_elem_id, 1, &c_cookie, ebufp)) != NULL)
                       	   {
				
				count = PIN_FLIST_FLD_GET(cycle_fees_flistp, PIN_FLD_COUNT, 1, ebufp);	
				count_inp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_COUNT, 1, ebufp);
				if(count && count_inp && *count == *count_inp)
				{
				   
				   chrg_from = PIN_FLIST_FLD_GET(cycle_fees_flistp, PIN_FLD_CHARGED_FROM_T, 0, ebufp);
				}
				
				if(chrg_from)
				{
					timeeff = localtime(chrg_from);	
					
					timeeff->tm_mon = timeeff->tm_mon + 1;
					
					timeeff->tm_mday = 1;
					
					chrg_to = mktime (timeeff);
					if(chrg_frm && (chrg_to > *chrg_frm))
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "SHOULD NOT RENEW THIS PRODUCT");
						action = NULL;	
					}
				}
			    }
			}
			  PIN_FLIST_DESTROY_EX(&svc_upd_out_flistp, NULL);
		   }
		 	
		if((prod_priority >= prepaid_fup_start && prod_priority <= prepaid_fup_end) ||
			(prod_priority >= postpaid_fup_start && prod_priority <= postpaid_fup_end)) {
			svc_upd_in_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(svc_upd_in_flistp, PIN_FLD_POID, service_obj, ebufp);
			bb_info_flistp = PIN_FLIST_ELEM_ADD(svc_upd_in_flistp, MSO_FLD_BB_INFO, 0, ebufp);
			PIN_FLIST_FLD_SET(bb_info_flistp, MSO_FLD_FUP_STATUS, &fup_status, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_service_fup_status update service input list", svc_upd_in_flistp);
			PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 32, svc_upd_in_flistp, &svc_upd_out_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_service_fup_status update service output list", svc_upd_out_flistp);
			PIN_FLIST_DESTROY_EX(&svc_upd_in_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&svc_upd_out_flistp, NULL);
		}

	    sprintf(msg, "priority is %f", prod_priority);
	    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	    sprintf(msg, "Prepaid FUP range  is [%d-%d]", prepaid_fup_start, prepaid_fup_end);
	    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	    sprintf(msg, "Postpaid FUP range  is [%d-%d]", postpaid_fup_start, postpaid_fup_end);
	    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	    sprintf(msg, "Postpaid limited range  is [%d-%d]", postpaid_lim_start, postpaid_lim_end);
	    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	    sprintf(msg, "Postpaid unlimited range  is [%d-%d]", postpaid_unlim_start, postpaid_unlim_end);
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
		

	    if((prod_priority >= prepaid_fup_start && prod_priority <= prepaid_fup_end) ||
               (prod_priority >= postpaid_fup_start && prod_priority <= postpaid_fup_end) ||
	       (prod_priority >= postpaid_unlim_start && prod_priority <= postpaid_unlim_end)||
		(prod_priority >= postpaid_lim_start && prod_priority <= postpaid_lim_end)) {

	    sprintf(msg, "In the range for renewal quota");
	    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	
		acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	    if((prod_priority >= prepaid_fup_start && prod_priority <= prepaid_fup_end) ||
               (prod_priority >= postpaid_fup_start && prod_priority <= postpaid_fup_end)||
	       (prod_priority >= postpaid_unlim_start && prod_priority <= postpaid_unlim_end))
            {
		flag = 1;
	    }
	    
	  if((prod_priority >= postpaid_lim_start && prod_priority <= postpaid_lim_end))
	  {
	        flag = 0;
	  }
		
		if(is_autorenew_flow(connp, opstackp, ebufp) || (action && strstr(action, "UNHOLD"))) {
	    	    sprintf(msg, "called from pin_cycle_fee or billing.. calling renew quota");
	    	    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
		    mso_cust_renew_quota(ctxp, acnt_pdp, action,service_obj, product_pdp, purch_pdp,flag, r_flistp, ebufp);
        	}
	    }
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Leaving fm_mso_update_service_fup_status");
	return;
}


int is_autorenew_flow(
		cm_nap_connection_t     *connp,
		cm_op_info_t *opstackp,
		pin_errbuf_t     *ebufp) {


    int32               stack_opcode = 0;
    pin_flist_t		*stack_flistp = NULL;
    char		*prog_name = NULL;

    char		msg[100];

    while(opstackp != NULL) 
    {
        stack_opcode = opstackp->opcode;
	stack_flistp = opstackp->in_flistp;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"is_autorenew_flow:Stack flist here",stack_flistp);
	prog_name = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);

	if(prog_name) PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,prog_name);

	sprintf(msg,"is_autorenew_flow check :Calling previous opcode is %s",pcm_opcode_to_opname(stack_opcode));
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);

        if(((stack_opcode == PCM_OP_SUBSCRIPTION_APPLY_RATE || 
	     stack_opcode == PCM_OP_SUBSCRIPTION_CYCLE_FORWARD || 
	     stack_opcode == PCM_OP_SUBSCRIPTION_CYCLE_ARREARS) 
		/*** Pavan Bellala - 28-08-2015 *********************************************
		Condition changed as the program name is "pin_cycle" and not "pin_cycle_fees
		****************************************************************************/
		//&& ((prog_name) && (strcmp(prog_name,"pin_cycle_fee")==0))) || 
		&& ((prog_name) && (strstr(prog_name,"pin_cycle")!= NULL))) || 
		(stack_opcode == PCM_OP_BILL_MAKE_BILL)) {

		if(fm_utils_op_is_ancestor(connp->coip,MSO_OP_CUST_ACTIVATE_BB_SERVICE)){
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"This is service activation.Hence not renew flow");
			return 0;
		}
		else if(fm_utils_op_is_ancestor(connp->coip,MSO_OP_CUST_HOLD_SERVICE))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"This is Hold action scenario which is done before billing");
                        return 0;
		}
		

    	return 1;
        }

	/**** Pavan Bellala 20-08-2015 Section commented and added for prepaid FUP Renewal *******
        if(	stack_opcode == PCM_OP_SUBSCRIPTION_APPLY_RATE || 
		stack_opcode == PCM_OP_SUBSCRIPTION_CYCLE_FORWARD || 
		stack_opcode == PCM_OP_SUBSCRIPTION_CYCLE_ARREARS || 
		((prog_name) && (strcmp(prog_name,"pin_cycle_fee")==0)) || 
		stack_opcode == PCM_OP_BILL_MAKE_BILL) {

		if(fm_utils_op_is_ancestor(connp->coip,MSO_OP_CUST_ACTIVATE_BB_SERVICE)){
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"This is service activation.Hence not renew flow");
			return 0;
		} 
    	return 1;
        }
	******/
        opstackp = opstackp->next;
    }
return 0;
}

void create_lifecycle_event(pcm_context_t		*ctxp,
		pin_flist_t		*i_flistp,
		pin_flist_t		*prod_details_flistp,
		pin_errbuf_t		*ebufp)
		
{

	char	*program = "AUTO_RENEWAL";
	char	*msg = "Automatic Renewal";
	char	*event = "/event/activity/mso_lifecycle/auto_renew";
	char	*permitted = NULL;
	double	prod_priority = 0;
	int32	flag = 6;
	int	i = 0;
	poid_t	*acct_pdp = NULL,*service_obj = NULL; 
	pin_flist_t	*read_flds_in_flistp = NULL;
	pin_flist_t	*read_flds_out_flistp = NULL;
	char	*broadband = "/service/telco/broadband";
	char	debug_msg[250];
		
	permitted = PIN_FLIST_FLD_GET(prod_details_flistp, PIN_FLD_PERMITTED, 0, ebufp);
	sprintf(debug_msg, "Permitted is %s",permitted);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
	if (strcmp(permitted, broadband) == 0)
	{		
		prod_priority = pbo_decimal_to_double( PIN_FLIST_FLD_GET(prod_details_flistp, PIN_FLD_PRIORITY, 0, ebufp),ebufp);		
		prod_priority = ((int)prod_priority%1000);

		if (prod_priority >= BB_POSTPAID_START && prod_priority <= BB_POSTPAID_END)
		{
	sprintf(debug_msg, "Comparing priorities for %d",prod_priority);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
			for(i = 0; i < 12; i++) {
				if(prod_priority == postpaid_grants_priorities[i]) {
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Found matching priorities");
					acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
					read_flds_in_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_OFFERING_OBJ, read_flds_in_flistp, PIN_FLD_POID, ebufp);
					PIN_FLIST_FLD_SET(read_flds_in_flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);
					PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flds_in_flistp, &read_flds_out_flistp, ebufp);
					service_obj = PIN_FLIST_FLD_GET(read_flds_out_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
					//fm_mso_cust_gen_event(ctxp, acct_pdp, service_obj, program, msg, event, flag,ebufp);
				}
			}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Done comparing priorities");
		}
	}
	sprintf(debug_msg, "Leaving create_lifecycle_event");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
	return;
}

int32 get_bill_when_from_service(pcm_context_t		*ctxp,
								poid_t				*svc_pdp,
								pin_errbuf_t		*ebufp) 
{
	int32	bill_when = 0;
	pin_flist_t	*read_in_flistp = PIN_FLIST_CREATE(ebufp);
	pin_flist_t	*read_out_flistp = NULL;
	pin_flist_t	*bb_info_flistp = NULL;

	PIN_FLIST_FLD_SET(read_in_flistp, PIN_FLD_POID, svc_pdp, ebufp);
	bb_info_flistp = PIN_FLIST_ELEM_ADD(read_in_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	PIN_FLIST_FLD_SET(bb_info_flistp, PIN_FLD_BILL_WHEN, NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input flist to read bill_when from service", read_in_flistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_in_flistp, &read_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Output flist to read bill_when from service", read_out_flistp);
	bb_info_flistp = PIN_FLIST_ELEM_GET(read_out_flistp, MSO_FLD_BB_INFO, 0 ,0, ebufp);
	bill_when = *(int32 *) PIN_FLIST_FLD_GET(bb_info_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);
	return bill_when;
}



void fm_mso_set_charged_to_t(pcm_context_t		*ctxp,
								pin_flist_t		*i_flistp,
								pin_flist_t		**r_flistpp,
								cm_op_info_t		*opstackp,
								pin_errbuf_t	*ebufp)
{
	pin_flist_t	*read_payinfo_iflist = NULL;
	pin_flist_t	*read_payinfo_oflist = NULL;
	pin_flist_t	*readproduct_inflistp = NULL;
	pin_flist_t	*readproduct_outflistp = NULL;
	pin_flist_t	*srch_in_flistp = NULL;
	pin_flist_t	*srch_out_flistp = NULL;
	pin_flist_t	*srch_billinfo_iflist = NULL;
	pin_flist_t	*srch_billinfo_oflist = NULL;
	pin_flist_t	*srch_ar_billinfo_iflist = NULL;
	pin_flist_t	*srch_ar_billinfo_oflist = NULL;
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*result_flistp = NULL;
	pin_flist_t	*inv_info = NULL;
	pin_flist_t	*bb_info_flistp = NULL;
	char		*template = "select X from /service 1, /purchased_product 2 where 2.F1 = V1 and 1.F2 = 2.F3 ";
	int64		db = 1;
	int32		search_flags = 256;
	poid_t		*srch_pdp = NULL;
	poid_t		*product_poid = NULL;
	poid_t		*payinfo_obj = NULL;
	poid_t		*ar_billinfo_obj = NULL;
	time_t		*charged_from_t = NULL;
	time_t		charged_to_t;
	struct tm		*timeeff;
	int32		payterm = 0;
	int32		prepaid_indicator = 0;
	int32		stack_opcode=0;
	int32		hold_svc_found=0;
	char		*billinfo_id = "BB";
	pin_decimal_t		*prod_priority = NULL;
	double		prod_priority_double = 0;
	char			log_buf[255];
	char		*action = NULL;
	pin_flist_t     *stack_flistp = NULL;
	poid_t		*prod_off_pdp = NULL;	
	int32		fup_top = 0;
	pin_flist_t	*ret_flistp = NULL;	
	poid_t		*account_obj = NULL;
	poid_t		*service_obj = NULL;
	time_t		*end_t = NULL;
	pin_flist_t	*ret_flistpp = NULL;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_set_charged_to_t input flist", i_flistp);
	// Algo
	// Read payinfo to determine if it is a prepaid service or not.
	// if not prepaid, return
	// Get priority of the product to determine the type of plan.
	// if Limited plan,
	//		get the payterm from the service and add that number of months to charged_from_t
	// if Unlimited No FUP, leave it as is.
	// if Unlimited with FUP,
	//		Add 1 month to the charged_from_t.



	// Read payinfo to determine if it is a prepaid service or not.
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

	// Search ar_billinfo for payinfo obj..
	srch_ar_billinfo_iflist = PIN_FLIST_CREATE(ebufp);

	PIN_FLIST_FLD_PUT(srch_ar_billinfo_iflist, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_ar_billinfo_iflist, PIN_FLD_FLAGS, &search_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_ar_billinfo_iflist, PIN_FLD_TEMPLATE, "select X from /billinfo  where F1 = V1 ", ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_ar_billinfo_iflist, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, args_flistp, PIN_FLD_ACCOUNT_OBJ,  ebufp);

	result_flistp = PIN_FLIST_ELEM_ADD(srch_ar_billinfo_iflist, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_AR_BILLINFO_OBJ, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_for_mcf search billinfo input list", srch_ar_billinfo_iflist);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_ar_billinfo_iflist, &srch_ar_billinfo_oflist, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_ar_billinfo_iflist, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_FLIST_DESTROY_EX(&srch_ar_billinfo_oflist, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_for_mcf search billinfo output flist", srch_ar_billinfo_oflist);

	result_flistp = PIN_FLIST_ELEM_GET(srch_ar_billinfo_oflist, PIN_FLD_RESULTS, 0, 0, ebufp);
	ar_billinfo_obj = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_AR_BILLINFO_OBJ, 0, ebufp);


	// Search billinfo for payinfo obj..
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_billinfo_iflist = PIN_FLIST_CREATE(ebufp);

	PIN_FLIST_FLD_PUT(srch_billinfo_iflist, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_billinfo_iflist, PIN_FLD_FLAGS, &search_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_billinfo_iflist, PIN_FLD_TEMPLATE, "select X from /billinfo  where F1.id = V1 and F2 = V2 ", ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_billinfo_iflist, PIN_FLD_ARGS, 1, ebufp);
	//PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, args_flistp, PIN_FLD_ACCOUNT_OBJ,  ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, ar_billinfo_obj, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_billinfo_iflist, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BILLINFO_ID, billinfo_id, ebufp);

	result_flistp = PIN_FLIST_ELEM_ADD(srch_billinfo_iflist, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PAYINFO_OBJ, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_for_mcf search billinfo input list", srch_billinfo_iflist);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_billinfo_iflist, &srch_billinfo_oflist, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_billinfo_iflist, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_FLIST_DESTROY_EX(&srch_billinfo_oflist, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_for_mcf search billinfo output flist", srch_billinfo_oflist);

	result_flistp = PIN_FLIST_ELEM_GET(srch_billinfo_oflist, PIN_FLD_RESULTS, 0, 0, ebufp);
	payinfo_obj = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_PAYINFO_OBJ, 0, ebufp);
	
	// Read payinfo object for prepaid indicator.
	read_payinfo_iflist = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_payinfo_iflist, PIN_FLD_POID, payinfo_obj, ebufp);

	PIN_FLIST_DESTROY_EX(&srch_billinfo_oflist, NULL);
	PIN_FLIST_DESTROY_EX(&srch_ar_billinfo_oflist, NULL);
	
	PIN_FLIST_ELEM_SET(read_payinfo_iflist, NULL, PIN_FLD_INV_INFO, 0, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_for_mcf read payinfo input list", read_payinfo_iflist);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_payinfo_iflist, &read_payinfo_oflist, ebufp);
	PIN_FLIST_DESTROY_EX(&read_payinfo_iflist, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
		PIN_FLIST_DESTROY_EX(&read_payinfo_oflist, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_for_mcf read payinfo output flist", read_payinfo_oflist);

	if (read_payinfo_oflist)
	{
		inv_info = PIN_FLIST_ELEM_GET(read_payinfo_oflist, PIN_FLD_INV_INFO, 0, 1, ebufp);
		prepaid_indicator = *(int32 *)PIN_FLIST_FLD_GET(inv_info, PIN_FLD_INDICATOR, 0, ebufp);
	}

	 // Added below while loop to find if the flow is from HOLD_SERVICE.
        // For Hold service (FUP plan) do not perform this action.	
	while(opstackp != NULL)
        {
                stack_opcode = opstackp->opcode;
                stack_flistp = opstackp->in_flistp;
                sprintf(log_buf, "OPCODE=%d", stack_opcode);
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, log_buf);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "cm_op_info_t stack input flist is ", stack_flistp);
                if (stack_opcode == MSO_OP_CUST_HOLD_SERVICE)
                {
                        hold_svc_found = 1;
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Hold Service Found");
                        break;
                }
                if(stack_opcode == MSO_OP_PROV_BB_PROCESS_RESPONSE)
                {
                        action = PIN_FLIST_FLD_GET(stack_flistp, PIN_FLD_ACTION, 1, ebufp);
                        if(action &&(strstr(action, "UNHOLD_SERVICES")))
                        {
                                PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_ACTION, action, ebufp);
                        }
			//added condition to set the purchase_end_t to charged_t of fup_topup product  
                        else if(action &&(strstr(action, "UPDATE_SUBSCRIBER-FUP")))
                        {
				prod_off_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
				//to check weather product is fup topup product or not
				fup_top = fm_mso_get_product_priority(ctxp, prod_off_pdp, &ret_flistp, ebufp);
                                if (PIN_ERRBUF_IS_ERR(ebufp))
                                {
                                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in after returing from fm_mso_get_product_priority", ebufp);
                                        return;
                                }
                                if(fup_top == 1)
                                {
					prod_off_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_OFFERING_OBJ, 1, ebufp);
					//get the purchase_end_t
                                        fm_mso_prov_update_product_cycle_fee_charge_dates(ctxp, NULL, NULL, prod_off_pdp, &ret_flistpp, ebufp);
					if (PIN_ERRBUF_IS_ERR(ebufp))
                                	{
                                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in after returing from fm_mso_prov_update_product_cycle_fee_charge_dates", ebufp);
                                        	return;
                                	}
					end_t = PIN_FLIST_FLD_GET(ret_flistpp, PIN_FLD_PURCHASE_END_T, 1, ebufp );
                                        PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CYCLE_FEE_END_T, end_t, ebufp );
                                        PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CHARGED_TO_T, end_t, ebufp );
                                        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_CYCLE_FEE_END_T, *r_flistpp, PIN_FLD_CYCLE_FEE_END_T, ebufp);
                                        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_CHARGED_TO_T,    *r_flistpp, PIN_FLD_CHARGED_TO_T, ebufp);	
					PIN_FLIST_DESTROY_EX(&ret_flistpp, NULL);
				}
			}
		}
                opstackp = opstackp->next;
        }

	if(prepaid_indicator != PREPAID) {
		// if not prepaid, return;
		return;
	}
	PIN_FLIST_DESTROY_EX(&read_payinfo_oflist, NULL);


	// Get priority of the product to determine the type of plan.
	product_poid = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
	if (!product_poid)
	{	
		return;
	}	


	readproduct_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(readproduct_inflistp, PIN_FLD_POID, product_poid, ebufp );
	PIN_FLIST_FLD_SET(readproduct_inflistp, PIN_FLD_PRIORITY, 0, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_set_charged_to_t read product input list", readproduct_inflistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, readproduct_inflistp, &readproduct_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&readproduct_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&readproduct_outflistp, NULL);	
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_set_charged_to_t read product output flist", readproduct_outflistp);
	
	prod_priority = PIN_FLIST_FLD_GET(readproduct_outflistp, PIN_FLD_PRIORITY, 0, ebufp );

	prod_priority_double = pbo_decimal_to_double(prod_priority, ebufp);

	prod_priority_double = ((int)prod_priority_double) % 1000;

	PIN_FLIST_DESTROY_EX(&readproduct_outflistp, NULL);



	// Limited
	if (prod_priority_double >= BB_LIMITED_RANGE_START &&
        prod_priority_double <= BB_LIMITED_RANGE_END) {

		// if Limited plan,
		//		get the payterm from the service and add that number of months to charged_from_t
	
		
		srch_in_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
		PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &search_flags, ebufp);
		PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_OFFERING_OBJ, args_flistp, PIN_FLD_POID,ebufp );
		args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);

		result_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, 0, ebufp);
		bb_info_flistp = PIN_FLIST_SUBSTR_ADD(srch_in_flistp, MSO_FLD_BB_INFO, ebufp);
		PIN_FLIST_FLD_SET(bb_info_flistp, PIN_FLD_BILL_WHEN, NULL, ebufp);

		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_set_charged_to_t search input flist", srch_in_flistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_set_charged_to_t search output flist", srch_out_flistp);

		PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);

		result_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "After time set 1");
		bb_info_flistp = PIN_FLIST_SUBSTR_GET(result_flistp, MSO_FLD_BB_INFO, 0, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "After time set 2");
		payterm = *((int32 *)PIN_FLIST_FLD_GET(bb_info_flistp, PIN_FLD_BILL_WHEN, 0, ebufp));
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "After time set 3");

		charged_from_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CHARGED_FROM_T, 1, ebufp);




		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "After time set 4");
		timeeff = localtime(charged_from_t);
		timeeff->tm_mon = timeeff->tm_mon + payterm;
		charged_to_t = mktime (timeeff);

		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_COUNT, &payterm, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Modified input flist", i_flistp);



		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "After time set");

		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CHARGED_TO_T, &charged_to_t, ebufp);
		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CYCLE_FEE_END_T, &charged_to_t, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_CYCLE_FEE_END_T, *r_flistpp, PIN_FLD_CYCLE_FEE_END_T, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_CHARGED_TO_T,    *r_flistpp, PIN_FLD_CHARGED_TO_T, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "After END_T set");
		PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	}
	// Unlimited without FUP
	else if (prod_priority_double >= BB_UNLIMITED_NO_FUP_RANGE_START &&
        prod_priority_double <= BB_UNLIMITED_NO_FUP_RANGE_END) {
		// if Unlimited No FUP, leave it as is.
	
		return;
	}
	// Unlimited with FUP
	else if (prod_priority_double >= BB_UNLIMITED_FUP_RANGE_START &&
        prod_priority_double <= BB_UNLIMITED_FUP_RANGE_END && !hold_svc_found) {
		// if Unlimited with FUP, Add 1 month to the charged_from_t.
		//  For Hold, it should not be executed.
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Setting charged_to_t for unlimited with fup");
		charged_from_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CHARGED_FROM_T, 1, ebufp);
		timeeff = localtime(charged_from_t);
		timeeff->tm_mon = timeeff->tm_mon + 1;
		charged_to_t = mktime (timeeff);
		sprintf(log_buf, "charged_from=%d; charged_to=%d", *charged_from_t,charged_to_t );
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, log_buf);
		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CHARGED_TO_T, &charged_to_t, ebufp);
		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CYCLE_FEE_END_T, &charged_to_t, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_CYCLE_FEE_END_T, *r_flistpp, PIN_FLD_CYCLE_FEE_END_T, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_CHARGED_TO_T,    *r_flistpp, PIN_FLD_CHARGED_TO_T, ebufp);

	}

	return;
}


static void mso_cust_renew_quota(pcm_context_t		*ctxp,
					poid_t		*acct_pdp,
					char		*action,
					poid_t		*svc_pdp,
					poid_t		*product_pdp, //added new field
					poid_t		*purch_pdp,
					int32             flag,
					pin_flist_t	*r_flistp,
					pin_errbuf_t		*ebufp) 
{
	//DOC_BB_AUTO_RENEW_POST

	pin_flist_t	*mso_pp_in_flistp = NULL;
	pin_flist_t	*mso_pp_out_flistp = NULL;
	pin_flist_t	*prov_in_flistp = NULL;
	pin_flist_t	*prov_out_flistp = NULL;
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*result_flistp = NULL;
	char mso_pp_srch_template[200] = "select X from /mso_purchased_plan where F1 = V1 and ( F2 = V2 or F3 = V3 or F5 = V5 ) and F4 = V4 and F6 = V6";
	int32	active_status = 2;
	char	*base_plan_str = "base plan";
	int32	renew_post_flag = DOC_BB_AUTO_RENEW_POST;
	char	*program_name = "pin_cycle_fee";
	int32	srch_flag = 0;
	poid_t	*srch_pdp = NULL;
	poid_t	*mso_pp_pdp = NULL;
	int64	db = 1;
	int32	susp_status = 5;
	int32   inprog_status = 1;
	char	*err_desc = NULL;	
	pin_flist_t	*prod_flistp = NULL;
	
    	mso_pp_in_flistp = PIN_FLIST_CREATE(ebufp);
    	srch_pdp = PIN_POID_CREATE(db, "/search", 0, ebufp);
    	PIN_FLIST_FLD_SET(mso_pp_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
    	PIN_FLIST_FLD_SET(mso_pp_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
    	PIN_FLIST_FLD_SET(mso_pp_in_flistp, PIN_FLD_TEMPLATE, mso_pp_srch_template, ebufp);
    	args_flistp = PIN_FLIST_ELEM_ADD(mso_pp_in_flistp, PIN_FLD_ARGS, 1, ebufp);
    	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(mso_pp_in_flistp, PIN_FLD_ARGS, 2, ebufp);
    	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &active_status, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(mso_pp_in_flistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &susp_status, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(mso_pp_in_flistp, PIN_FLD_ARGS, 4, ebufp);
    	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DESCR, base_plan_str, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(mso_pp_in_flistp, PIN_FLD_ARGS, 5, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &inprog_status, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(mso_pp_in_flistp, PIN_FLD_ARGS, 6, ebufp);
	prod_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(prod_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, purch_pdp, ebufp);
	result_flistp = PIN_FLIST_ELEM_ADD(mso_pp_in_flistp, PIN_FLD_RESULTS, 0, ebufp);
    	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_DESCR, NULL, ebufp);

    	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_cust_renew_quota search input flist for base plan search", mso_pp_in_flistp);
    	PCM_OP(ctxp, PCM_OP_SEARCH, 0, mso_pp_in_flistp, &mso_pp_out_flistp, ebufp);
    	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_cust_renew_quota search output flist for base plan search", mso_pp_out_flistp);

    	result_flistp = PIN_FLIST_ELEM_GET(mso_pp_out_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);

	if(result_flistp == NULL) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " No base plan exists for this, which is surprising. Bailing out... ");
		goto cleanup;
	}
	mso_pp_pdp = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_POID, 0, ebufp);
	if(flag == 1)
	{
           renew_post_flag = DOC_BB_FUP_REVERSAL;
	}
	prov_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_POID, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_SERVICE_OBJ, svc_pdp, ebufp);
	PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_FLAGS, &renew_post_flag, ebufp);
	PIN_FLIST_FLD_SET(prov_in_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, mso_pp_pdp, ebufp);
	PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_PROGRAM_NAME, program_name, ebufp);
	PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_USERID, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_PRODUCT_OBJ, product_pdp, ebufp);
	if(action)
	{
		PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_ACTION, action, ebufp);
	}
	PIN_FLIST_FLD_COPY(r_flistp, PIN_FLD_CHARGED_FROM_T, prov_in_flistp, PIN_FLD_CHARGED_FROM_T, ebufp);
	PIN_FLIST_FLD_COPY(r_flistp, PIN_FLD_CHARGED_TO_T, prov_in_flistp, PIN_FLD_CHARGED_TO_T, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_PROV_CREATE_ACTION input flist", prov_in_flistp);
    	PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION, 0, prov_in_flistp, &prov_out_flistp, ebufp);
    	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_PROV_CREATE_ACTION output flist", prov_out_flistp);
	
	/***Handiling the provisioning output check weather it is succes or not********/
	if (PIN_ERRBUF_IS_ERR(ebufp) || !prov_out_flistp)
    	{
        	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            		"mso_cust_renew_quota  FUP_RESET error", ebufp);
		goto cleanup;
	}
	else if(prov_out_flistp)
	{
		err_desc = (char *)PIN_FLIST_FLD_GET(prov_out_flistp, PIN_FLD_ERROR_DESCR, 1, ebufp);
		if(err_desc != NULL)
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_VALUE, 0, 0, 0);	
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        	"mso_cust_renew_quota  FUP_RESET failed in provisioning error", ebufp);
			goto cleanup;
		}
	}
	

cleanup:
		PIN_FLIST_DESTROY_EX(&mso_pp_in_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&mso_pp_out_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&prov_in_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&prov_out_flistp, NULL);

}


/* AMC CHANGES */

/**********************************************
 * Search service for the mso_purchased_plan 
 * Entries and get subscription Details  
 *********************************************/

static  void
fm_mso_cust_bb_hw_amc_get_cycle_details(
        pcm_context_t   *ctxp,
        pin_flist_t      *i_flistp,
        pin_errbuf_t    *ebufp)
{


        pin_flist_t     *args_flistp = NULL;
        pin_flist_t     *search_inflistp = NULL;
        pin_flist_t     *search_outflistp = NULL;
        pin_flist_t     *results_flistp = NULL;
        pin_flist_t     *services_flistp = NULL;
	pin_flist_t     *subs_outflistp = NULL;

        char            *device_id = NULL;
        char            search_template[100] = " select X from /mso_purchased_plan where F1 = V1 and F2 = V2 ";
        int             search_flags = 768;
        int64           db = 1;
        int             elem_id = 0;
        pin_cookie_t    pcookie = NULL;
	int             pelem_id = 0;
        pin_cookie_t    cookie = NULL;
        poid_t          *service_pdp = NULL;
	poid_t          *a_pdp = NULL;
        pin_flist_t     *s_flistp = NULL;
	int32		subs_found = 0;
	poid_t		*subs_pdp = NULL;
	char		*prov_tag = NULL;
	pin_flist_t	*products_flistp = NULL;
	int32 		*p_status = NULL;
	time_t		*cycle_end_t = NULL;
	pin_flist_t     *res_flistp = NULL;
	pin_flist_t 	*pr_flistp = NULL;
	pin_flist_t	*cycle_flistp = NULL;
	poid_t		*p_pdp = NULL;
	int		inr = 356;
	char		*plan_type = "base plan";
	
		

        if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
                
	PIN_ERRBUF_CLEAR(ebufp);

    	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_mso_cust_bb_hw_amc_get_cycle_details input flist", i_flistp);

	a_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	

        /*************
         * search flist to search device details based on service poid
         ************/
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                //On Error
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,
                                "Subscription product Error preparation", search_inflistp);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error calling Subscription search", ebufp);
                return;

        }


	db = PIN_POID_GET_DB(a_pdp);

        search_inflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
        PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, a_pdp, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DESCR, plan_type, ebufp);
        results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "search mso_purchased_plan input list", search_inflistp);

        if (PIN_ERRBUF_IS_ERR(ebufp)) 
        {
                //On Error
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,
                                "Subscription product Error preparation", search_inflistp);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error calling Subscription search", ebufp);
                PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
                PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
                return;

        }


        PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp) || search_outflistp == NULL)
        {
            //On Error
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,
				"search input list Subscription search", search_inflistp);
        	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error calling Subscription Search", ebufp);
        	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
		return;
				
	}	

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                "Subscription Search", search_outflistp);
         /*search Through the results and get the product which has Prov Tag */
	while ((results_flistp =   
				PIN_FLIST_ELEM_GET_NEXT(search_outflistp, PIN_FLD_RESULTS,
                               &elem_id, 1, &cookie, ebufp)) != NULL)
        {
		p_status = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_STATUS , 1, ebufp);
		products_flistp = NULL;
		pelem_id = 0;
		pcookie = NULL;
		if(subs_found)
			break;
		if(p_status && (*p_status == MSO_PROV_ACTIVE || *p_status == MSO_PROV_IN_PROGRESS || *p_status == MSO_PROV_SUSPEND))
		{		
			while ((products_flistp =   
				PIN_FLIST_ELEM_GET_NEXT(results_flistp, PIN_FLD_PRODUCTS,
                              		&pelem_id, 1, &pcookie, ebufp)) != NULL)
			{
				prov_tag = (char *)PIN_FLIST_FLD_GET(products_flistp, PIN_FLD_PROVISIONING_TAG, 1, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Inside PROV TAG SEARCH", products_flistp);
				if(prov_tag && strlen(prov_tag)>0 )
		
				{
					subs_found = 1;
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "FOUND SUBS PRODUCT");
					subs_pdp = PIN_FLIST_FLD_GET(products_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, 0, ebufp);
					break;
				}
			}
              

        	}
	}
		
		
	if (PIN_ERRBUF_IS_ERR(ebufp) || subs_found == 0 ) 
        {
            	//On Error
            	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,
				"Subscription product search error", search_inflistp);
            	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error calling Subscription search", ebufp);
            	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
		return;
				
	}	
		
	/*get cycle details of product */
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	search_inflistp = PIN_FLIST_CREATE(ebufp);
		
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_POID, subs_pdp,ebufp);
		
	PCM_OP(ctxp, PCM_OP_READ_OBJ , 0, search_inflistp, &subs_outflistp, ebufp);
		
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                "Read Subs Product Output", subs_outflistp);
								
	if (PIN_ERRBUF_IS_ERR(ebufp) || subs_outflistp == NULL)
        {
            //On Error
            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,
				"Error in reading subs product", search_inflistp);
        	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error read obj of subscription", ebufp);
        	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		PIN_FLIST_DESTROY_EX(&subs_outflistp, NULL);
		PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
		return;
				
	}

	p_pdp = PIN_FLIST_FLD_GET(subs_outflistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp); 
	//IF CYCLE Array Present get charged_to_t 
	if(PIN_FLIST_ELEM_COUNT(subs_outflistp, PIN_FLD_CYCLE_FEES, ebufp))
	{
		cycle_flistp = PIN_FLIST_ELEM_GET(subs_outflistp, PIN_FLD_CYCLE_FEES, PIN_ELEMID_ANY, 1, ebufp);
		cycle_end_t = PIN_FLIST_FLD_GET(cycle_flistp, PIN_FLD_CHARGED_TO_T, 0, ebufp);
	}
	else // set cycle_end_t as charged_to_t of subscription
	{
		cycle_end_t = PIN_FLIST_FLD_GET(subs_outflistp, PIN_FLD_CYCLE_END_T, 0, ebufp);
	}

        if (PIN_ERRBUF_IS_ERR(ebufp)) 
        {
            //On Error
            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,
                                "cannot Read SUBS PRODUCT CYCLE DATES ", search_inflistp);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error read obj of subscription", ebufp);
                PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
                PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
		PIN_FLIST_DESTROY_EX(&subs_outflistp, NULL);
                return;

        }
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CYCLE_END_T, cycle_end_t, ebufp);
	/*pr_flistp = PIN_FLIST_ELEM_ADD(res_flistp, PIN_FLD_BAL_IMPACTS, 0, ebufp);
	PIN_FLIST_FLD_SET(pr_flistp, PIN_FLD_RESOURCE_ID, &inr, ebufp); */

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"Modified Input Flist is ", i_flistp);

		
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	PIN_FLIST_DESTROY_EX(&subs_outflistp, ebufp);
	//PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
        return;
}

static void
get_last_plan_valid_to_fup(
	pcm_context_t           *ctxp,
	poid_t                  *acc_obj,
	poid_t                  *svc_obj,
	time_t			*fup_valid_to,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t             *srch_iflistp = NULL;
	pin_flist_t             *srch_oflistp = NULL;
	pin_flist_t             *r_i_flistp = NULL;
	pin_flist_t             *r_o_flistp = NULL;
	pin_flist_t             *args_flist = NULL;
	pin_flist_t             *res_flistp = NULL;
	pin_flist_t             *bal_flistp = NULL;
	pin_flist_t             *sub_bal_flistp = NULL;
	pin_flist_t             *prod_flistp = NULL;
	int32                   srch_flag = 0;
	char                    *s_template = "select X from /mso_purchased_plan where F1 = V1 and F2 = V2 and F3 = V3 order by mod_t desc ";

	poid_t                  *pdp = NULL;
	poid_t                  *pp_obj = NULL;
	poid_t                  *grantor_obj = NULL;
	poid_t                  *bal_grp_obj = NULL;
	int32			status = 0;
	int32			bal_elem_id = 0;
	int32			sub_bal_id = 0;
	int32			elem_id = 0;
	int32			bal_flags = 0;
	void			*vp = NULL;
	time_t			current_t;

	pin_cookie_t		cookie = NULL;
	pin_cookie_t		bal_cookie = NULL;
	pin_cookie_t		sub_cookie = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling get_last_plan_valid_to_fup", ebufp);
		return;
	}
	*fup_valid_to = 0;
	// 1.Find the latest cancelled plan with given plan_obj since it is topup flow.
	status = MSO_PROV_TERMINATE;
	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	pdp = PIN_POID_CREATE(1, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(srch_iflistp, PIN_FLD_POID, pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, s_template, ebufp);

	args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(args_flist, PIN_FLD_SERVICE_OBJ, svc_obj, ebufp);
	
	args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(args_flist, PIN_FLD_DESCR, "base plan", ebufp);

	args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 3, ebufp );
	PIN_FLIST_FLD_SET(args_flist, PIN_FLD_STATUS, &status, ebufp);

	PIN_FLIST_ELEM_SET(srch_iflistp, NULL, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	//PIN_FLIST_FLD_SET(args_flist, PIN_FLD_POID, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "get_last_plan_valid_to_fup search input", srch_iflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"get_last_plan_valid_to_fup - Error in calling SEARCH", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"get_last_plan_valid_to_fup search output", srch_oflistp);
	// Fetch the first element in search output.
	res_flistp = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, 0, 0, ebufp );

	// Read balance group form service
	r_i_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(r_i_flistp, PIN_FLD_POID, svc_obj, ebufp);
	PIN_FLIST_FLD_SET(r_i_flistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_obj, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read Flds input flist", r_i_flistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS , 0, r_i_flistp, &r_o_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read Flds output flist", r_o_flistp);
	PIN_FLIST_DESTROY_EX(&r_i_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "get_last_plan_valid_to_fup - Error in calling READ_FLDS", ebufp);
		return;
	}
	bal_grp_obj = PIN_FLIST_FLD_TAKE(r_o_flistp, PIN_FLD_BAL_GRP_OBJ, 0, ebufp);
	PIN_FLIST_DESTROY_EX(&r_o_flistp, NULL);

	// 2. Find the FUP sub balance that was granted by cancelled plan (grant product)
	//	The validity of this sub balance will be extended during topup as 
	//	existing + 1month. So the newly granted sub bal must have same validity.
	r_i_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(r_i_flistp, PIN_FLD_POID, acc_obj, ebufp);
	PIN_FLIST_FLD_PUT(r_i_flistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_obj, ebufp);
	bal_flags = PIN_BAL_GET_ALL_BARE_RESULTS;
	current_t=pin_virtual_time ((time_t *) NULL);
	PIN_FLIST_FLD_SET(r_i_flistp, PIN_FLD_FLAGS, &bal_flags, ebufp);
	PIN_FLIST_FLD_SET(r_i_flistp, PIN_FLD_END_T, &current_t, ebufp);
	PIN_FLIST_ELEM_SET(r_i_flistp, NULL, PIN_FLD_BALANCES, PIN_ELEMID_ANY, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read balance group input list", r_i_flistp);
	PCM_OP(ctxp, PCM_OP_BAL_GET_BALANCES , 0, r_i_flistp, &r_o_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read balance group output list", r_o_flistp);
	PIN_FLIST_DESTROY_EX(&r_i_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "get_last_plan_valid_to_fup - Error in calling BAL_GET_BALANCES", ebufp);
		return;
	}

	while ((bal_flistp = PIN_FLIST_ELEM_GET_NEXT(r_o_flistp, PIN_FLD_BALANCES, 
			&bal_elem_id, 1, &bal_cookie, ebufp)) != NULL)
	{
		if (bal_elem_id != MSO_FUP_TRACK )
		{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Non an FUP resource");
				continue;
		}
		// Loop through each sub balance of FUP resource.
		sub_cookie = NULL;
		while ((sub_bal_flistp = PIN_FLIST_ELEM_GET_NEXT(bal_flistp, PIN_FLD_SUB_BALANCES, 
			&sub_bal_id, 1, &sub_cookie, ebufp )) != NULL)
		{
			grantor_obj = PIN_FLIST_FLD_GET(sub_bal_flistp, PIN_FLD_GRANTOR_OBJ, 0, ebufp);
			PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Grantor Obj :",grantor_obj);
			cookie = NULL;
			while ((prod_flistp = PIN_FLIST_ELEM_GET_NEXT(res_flistp, PIN_FLD_PRODUCTS, 
					&elem_id, 1, &cookie, ebufp)) != NULL)
			{
				pp_obj = PIN_FLIST_FLD_GET(prod_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, 0, ebufp );
				PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Pur Prod Obj :",pp_obj);
				if(PIN_POID_COMPARE(grantor_obj, pp_obj, 0, ebufp)==0)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Poid matched...");
					vp = PIN_FLIST_FLD_GET(sub_bal_flistp, PIN_FLD_VALID_TO, 0, ebufp);
					if(vp) 
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Assigning valid to..");
						*fup_valid_to = *(time_t *)vp;
					}
				}
			}
		}
	}
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
}

/* Search all the /purchased_product with following args:
and get the valid purchase_start_t to set the charged_to_t
during subscription topup for grant product*/

void
get_last_mso_purchased_plan_modified(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flist,
        poid_t                  *plan_pdp,
	int32			add_flag,
        pin_flist_t             **bb_ret_flistp,
        pin_errbuf_t            *ebufp)
{
        char                    *search_prt_template1 = "select X from /purchased_product 1,/product 2 where 1.F1 = V1 and 1.F2 = V2 and 1.F5 = 2.F6 and 2.F4 LIKE V4 order by 1.F3 desc ";
        int32                   cancel_status = 3;
        pin_flist_t             *args_flistp = NULL;
        int                     db = -1;
        int                     srch_flag = 512;
        pin_flist_t             *srch_flistp = NULL;
        pin_flist_t             *srch_out_flistp = NULL;
        pin_flist_t             *result_flistp = NULL;
        int32                   status_change_failure = 1;
        poid_t                  *service_obj = NULL;
        pin_flist_t             *results_flistp = NULL;
        poid_t                  *prod_pdp = NULL;
        poid_t                  *acc_pdp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	int32			flag = 0;
	poid_t			*old_prod_pdp = NULL;
	pin_cookie_t        	cookie = NULL;
	int32              	elem_id = 0;
	char			*descr = NULL;
	int32                   active_status = 1;	
	
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"get_last_mso_purchased_plan_modified error before calling function", ebufp);
                return;
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Getting latest closed date for the plan to be renewed", in_flist);

        prod_pdp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
        acc_pdp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp);
	if(!acc_pdp)
	{
		acc_pdp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	}
        db = PIN_POID_GET_DB(plan_pdp);
        srch_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, search_prt_template1, ebufp);
	 args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acc_pdp, ebufp);
	if(add_flag == 1)
	{
		args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
        	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &active_status, ebufp);
	}
	else
	{
		args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
        	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &cancel_status, ebufp);
	}
        args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PURCHASE_END_T, (time_t *)NULL, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp );
        tmp_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_USAGE_MAP, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_EVENT_TYPE, "%fdp/mso_grant%", ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 5, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 6, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);
        result_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Getting latest closed date for the plan to be renewed", srch_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Last updated Effective date for the plan is :", srch_out_flistp);
        results_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp) )
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH or no result flist", ebufp);
		return;
        }
	else
	{
		/*fetch the old product date during topup*/
		tmp_flistp = NULL;
		if(add_flag == 1)
		{
			goto SKIP;
		}
		while((result_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_out_flistp, PIN_FLD_RESULTS,
                	&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Inside loop ", result_flistp);
			old_prod_pdp = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);	
			descr = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_DESCR, 1, ebufp);
			if(PIN_POID_COMPARE(old_prod_pdp, prod_pdp, 0, ebufp)==0)
			{
				
				if(flag == 1)
					break;

				if(tmp_flistp!= NULL && strcmp(descr,"pin_cycle_fees") ==0)
				{
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "pin_cycle_fees deac", tmp_flistp);
					break;
				}

				tmp_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, elem_id, 1, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Latest purchased_product", tmp_flistp);
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," before the skip");
				flag = 1;
			}
		}
	}

	SKIP:
	if(tmp_flistp!=NULL)
	{
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "flist check", tmp_flistp);
        	*bb_ret_flistp = PIN_FLIST_COPY(tmp_flistp, ebufp);
	}
	else if(add_flag == 1)
	{
		*bb_ret_flistp = PIN_FLIST_COPY(results_flistp, ebufp);
	}	
	else
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "Matching product details not found", ebufp);
		pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_VALUE, PIN_FLD_PLAN_OBJ, 0, 0);
	}
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	return;
}
