/*******************************************************************
 *
 * Copyright (c) 2004, 2008, Oracle and/or its affiliates.All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its subsidiaries or licensors and may be used, reproduced, stored
 *      or transmitted only in accordance with a valid Oracle license or
 *      sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_rate_pol_pre_rating.c:IDCmod7.3.1Int:3:2008-Jan-10 12:17:32 %";
#endif

/*******************************************************************
 * Contains the PCM_OP_RATE_POL_PRE_RATING operation. 
 *******************************************************************/

#include <stdio.h> 
#include <string.h> 

#include "pcm.h"
#include "ops/rate.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "pin_rate.h"
#include "pin_subscription.h"
#include "mso_ops_flds.h"
#include "ops/bill.h"
#include "mso_cust.h"

#define FILE_SOURCE_ID  "fm_rate_pol_pre_rating.c(7)"


#define MAX_CALLLIST_SIZE 1024 
#define MAX_IMP_CATEGORY_SIZE 25 
#define CALLLIST_SEPERATOR  ";"
#define IMPACT_CATEGORY_SEPERATOR ";"

#define DROPPED_CALL_RUM_NAME "Dropped_Call_Quantity"
#define CALL_TYPE_NORMAL_CALL 0
#define CALL_TYPE_CONTINUATION_CALL 2

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_rate_pol_pre_rating(
	cm_nap_connection_t	*connp,
	u_int			opcode,
	u_int			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_rate_pol_pre_rating(
	cm_nap_connection_t     *connp,
	pcm_context_t		*ctxp,
	cm_op_info_t		*opstackp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_rate_pol_handle_dropped_calls(
	pin_flist_t	*i_flistp,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp);

static void
fm_rate_pol_process_mostcalled_event(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp);

static void
fm_rate_pol_search_mostcalled_profile(
        pcm_context_t   *ctxp,
        poid_t	*off_pdp,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp);

static void
fm_rate_pol_search_mostcalled_events(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_flist_t     *p_flistp,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp);

static void
fm_rate_pol_aggregate_mostcalled_events(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_flist_t     *p_flistp,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp);

static void
fm_rate_pol_find_mostcalled_nums (
        pin_flist_t     *i_flistp,
        int32           *mc_count,
        char            *mc_criterion,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp);


static void
fm_rate_pol_get_member_services(
        pcm_context_t   *ctxp,
        poid_t          *svc_pdp,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp);

static int32
fm_rate_pol_svc_validate(
        poid_t          *svc_pdp,
        pin_flist_t     *svc_flistp,
        pin_errbuf_t    *ebufp);

static void
fm_rate_pol_update_flag_for_hold (
        pcm_context_t   *ctxp,
	cm_op_info_t	*opstackp,
        pin_flist_t     *i_flistp,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp);

extern int32
fm_mso_get_product_priority(
        pcm_context_t           *ctxp,
        poid_t                  *prod_pdp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

/*******************************************************************
 * Routines needed from elsewhere.
 *******************************************************************/

/*******************************************************************
 * Main routine for the PCM_OP_RATE_POL_PRE_RATING operation.
 *******************************************************************/
void
op_rate_pol_pre_rating(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp			= connp->dm_ctx;
	pin_flist_t		*r_flistp 		= NULL;
	pin_flist_t		*e_flistp 		= NULL;
	poid_t			*e_pdp 			= NULL;
	int32			rerate_flags		= 0;
	void			*vp			= NULL;
	cm_op_info_t		*opstackp = connp->coip;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_RATE_POL_PRE_RATING) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_rate_pol_pre_rating opcode error", ebufp);
		return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_rate_pol_pre_rating input flist", i_flistp);

	/*********************************************************
	 * For Billing time discount events, ERAs are fetched
         * by get_candidate_discounts. However, by doing it here
	 * we are caching the results. If this policy does anything
	 * other than getting the ERAs, then Billing Time Discount
	 * events need to treated seperately.
	 *********************************************************/

	fm_rate_pol_pre_rating(connp, ctxp, opstackp, i_flistp, &r_flistp, ebufp);

	/***********************************************************
	 * This is a sample implementation to find the most called
	 * numbers. The implementation is specific to call events with
	 * RUM as duration. Other type of events may need extra
	 * customization.
	 ***********************************************************/

	/*
	 * Compare the event type and take action 
	 */
	e_flistp = PIN_FLIST_SUBSTR_GET(i_flistp,PIN_FLD_EVENT,1, ebufp);

	if(e_flistp)
	   e_pdp = PIN_FLIST_FLD_GET(e_flistp, PIN_FLD_POID, 1, ebufp);

	if((e_pdp) && (fm_utils_is_subtype(e_pdp, 
			PIN_OBJ_TYPE_EVENT_CYCLE_DISC_MOSTCALLED))) {

		/** Process the most called event **/

		fm_rate_pol_process_mostcalled_event(ctxp, i_flistp,
							&r_flistp,ebufp);


	}

	/*
	 * If the event is the continuation call, set the /event.CALL_TYPE to
	 * continuation call (1). Incase of re-rating CALL_TYPE
	 * will be populated by this policy opcode, and incase of real time
	 * TCF opcodes will take care of setting this field in event before
	 * calling rating.
	 * PIN_FLD_FLAGS set to PIN_RATE_FLG_RERATE will be used to identify
	 * the re-rated event.
	 */
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp);
	if (vp != NULL) {
		rerate_flags = *(int32*)vp;
	}

	/* Call fm_rate_pol_handle_dropped_calls only for the re-rated events.
	 * For re-rated event PIN_FLD_FLAGS is set to PIN_RATE_FLG_RERATE.
	 */
	if ((PIN_RATE_FLG_RERATE & rerate_flags) &&
		e_pdp && fm_utils_is_subtype(e_pdp, "/event/session/telco")) {

		fm_rate_pol_handle_dropped_calls(i_flistp, &r_flistp, ebufp);
	}

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_rate_pol_pre_rating error", ebufp);
		PIN_FLIST_DESTROY(r_flistp, NULL);
		*o_flistpp = NULL;

	} else {
		*o_flistpp = r_flistp;
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_rate_pol_pre_rating return flist", *o_flistpp);
	}

	return;
}

static void
fm_rate_pol_pre_rating(
	cm_nap_connection_t     *connp,
	pcm_context_t	*ctxp,
	cm_op_info_t	*opstackp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**r_flistp,
	pin_errbuf_t	*ebufp)
{

	pin_flist_t	*evt_flistp = NULL;
	pin_flist_t	*cycle_flistp = NULL;
	
	poid_t		*evt_pdp = NULL;
	poid_t		*srvc_pdp = NULL;

	pin_decimal_t	*scale = NULL;
	pin_decimal_t	*new_scale = NULL;
	
	int32		*cycle_flags = NULL;
	int32		*indicator = NULL;
	int32		cycle_flg = 0;
	int32		susp_flags = 20;
	
	char		msg[200];
	int             prty = 0;
	int32           mod_prty=0;
	char            *strp = NULL;
	int32		fup_top = 0;
	pin_decimal_t	*priority = NULL;
	pin_decimal_t	*zero_decimalp = pbo_decimal_from_str("0", ebufp);
	poid_t          *prod_pdp = NULL;
        pin_flist_t     *prod_array = NULL;
	pin_flist_t	*ret_flistp = NULL;
	pin_flist_t	*event_array = NULL;
	char            *prog_name = NULL;

	/* call opcode to process ERAs and fill PIN_FLD_USAGE_TYPE
	 * field in the event flist
	 */

	evt_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_EVENT, 1, ebufp);
	evt_pdp = PIN_FLIST_FLD_GET(evt_flistp, PIN_FLD_POID, 0, ebufp);
	srvc_pdp = PIN_FLIST_FLD_GET(evt_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	prog_name = PIN_FLIST_SUBSTR_GET(evt_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	cycle_flistp = PIN_FLIST_SUBSTR_GET(evt_flistp, PIN_FLD_CYCLE_INFO, 1, ebufp);
        if(cycle_flistp){
                scale = PIN_FLIST_FLD_GET(cycle_flistp, PIN_FLD_SCALE, 1 ,ebufp);
                cycle_flags = PIN_FLIST_FLD_GET(cycle_flistp, PIN_FLD_FLAGS, 1, ebufp);
                sprintf(msg, "cycle_flags = %d", *cycle_flags);
                PIN_ERR_LOG_MSG(3, msg);
        }	
	if(srvc_pdp && !strcmp(PIN_POID_GET_TYPE(srvc_pdp),"/service/catv") && strstr(PIN_POID_GET_TYPE(evt_pdp),"fdp")!=NULL 
		&& strstr(PIN_POID_GET_TYPE(evt_pdp),"lifecycle")==NULL)
	{
		PIN_ERR_LOG_FLIST(3, "Event flistp for pre-rating fdp catv", evt_flistp);
		/*cycle_flistp = PIN_FLIST_SUBSTR_GET(evt_flistp, PIN_FLD_CYCLE_INFO, 0, ebufp);
		scale = PIN_FLIST_FLD_GET(cycle_flistp, PIN_FLD_SCALE, 1 ,ebufp);
		cycle_flags = PIN_FLIST_FLD_GET(cycle_flistp, PIN_FLD_FLAGS, 1, ebufp);
		sprintf(msg, "cycle_flags = %d", *cycle_flags);
		PIN_ERR_LOG_MSG(3, msg);*/
		cycle_flg = *cycle_flags & PIN_RATE_FLG_INVERTED;
		sprintf(msg, "And operation output : %d", cycle_flg);
		PIN_ERR_LOG_MSG(3, msg);
		//cycle_flg = *cycle_flags ;
		if(*cycle_flags && !(*cycle_flags & PIN_RATE_FLG_INVERTED))
		{
			PIN_ERR_LOG_MSG(3, "scale reset entered");
			new_scale = pbo_decimal_from_str("1.0",ebufp);
			PIN_FLIST_FLD_SET(cycle_flistp, PIN_FLD_SCALE, new_scale, ebufp);
			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_SCALE, new_scale, ebufp);
			pbo_decimal_destroy(&new_scale);
		}
		PIN_ERR_LOG_FLIST(3, "New flist after setting scale", i_flistp);
	}
	//FOR ISP, in cancellation scenario for prepaid plans, we don''t need to provide refund as we have alaredy providing
	// before cancellation with PREPAY_DISCONNECTION_CREDIT program
	else if(srvc_pdp && !strcmp(PIN_POID_GET_TYPE(srvc_pdp),"/service/telco/broadband") &&
                strstr(PIN_POID_GET_TYPE(evt_pdp),"/event/billing/product/fee/cycle/cycle_forward") &&
                        (((cycle_flags && (*cycle_flags & PIN_RATE_FLG_INVERTED)) && ((prog_name && strcmp(prog_name, "PREPAY_DISCONNECTION_CREDIT") != 0)))                                            || fm_utils_op_is_ancestor(connp->coip,11208)) )
	{
		PIN_ERR_LOG_MSG(3,"In DISCONECTION CREDIT HANDLING");
		event_array = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_EVENT, 1, ebufp);
		prod_array = PIN_FLIST_SUBSTR_GET(event_array, PIN_FLD_PRODUCT, 1, ebufp);
		if(prod_array!=NULL)
		{
			
			prod_pdp = PIN_FLIST_FLD_GET(prod_array, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
			fup_top = fm_mso_get_product_priority(ctxp, prod_pdp, &ret_flistp, ebufp);
			if (PIN_ERR_IS_ERR(ebufp))
			{
				 PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error after returning from function fm_mso_get_product_priority", ebufp);
				return;
			}
			PIN_ERR_LOG_FLIST(3, "GET PRODUCT PRIORITY RETFLIST", ret_flistp);
			//PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
			priority = PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_PRIORITY, 1, ebufp);	
			strp = pbo_decimal_to_str(priority, ebufp);
			prty = atoi(strp);
		
			// For PREPAID products, since we are giving refund before cancellation, suspend the refund rating.
			if (prty%1000 < 500){
				PIN_ERR_LOG_MSG(3, "reset scale to zero for prepaid cancellation cases");
				new_scale = pbo_decimal_from_str("0.0",ebufp);
				//PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_QUANTITY, new_scale, ebufp);
				PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_SCALE, new_scale, ebufp);
				PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_FLAGS, &susp_flags, ebufp);
				PIN_FLIST_FLD_SET(cycle_flistp, PIN_FLD_FLAGS, &susp_flags, ebufp);
				PIN_FLIST_FLD_SET(evt_flistp, PIN_FLD_FLAGS, &susp_flags, ebufp);
				PIN_FLIST_FLD_PUT(cycle_flistp, PIN_FLD_SCALE, new_scale, ebufp);
				PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_PERCENT, zero_decimalp, ebufp);
				PIN_FLIST_FLD_PUT(i_flistp, PIN_FLD_DISCOUNT, zero_decimalp, ebufp);
			}
	       }
	}
	else if(srvc_pdp && !strcmp(PIN_POID_GET_TYPE(srvc_pdp),"/service/telco/broadband") && 
		strstr(PIN_POID_GET_TYPE(evt_pdp),"mso_hw_amc") && 
			((!(cycle_flags && (*cycle_flags & PIN_RATE_FLG_INVERTED)) && fm_utils_op_is_ancestor(connp->coip, MSO_OP_CUST_BB_HW_AMC))))
	{
		PIN_ERR_LOG_MSG(3, "For TOPUP cases, set AMC SCALE to 1 to remove duplicate charges");
		new_scale = pbo_decimal_from_str("1",ebufp);
		//PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_QUANTITY, new_scale, ebufp);
		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_SCALE, new_scale, ebufp);
		PIN_FLIST_FLD_PUT(cycle_flistp, PIN_FLD_SCALE, new_scale, ebufp);
	}
	PCM_OP(ctxp, PCM_OP_RATE_POL_PROCESS_ERAS, 0, i_flistp,
					r_flistp, ebufp);
	
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		    "fm_rate_pol_pre_rating: Error in processing ERAS", ebufp);
		return;
	}

		fm_rate_pol_update_flag_for_hold(ctxp, opstackp, i_flistp, r_flistp, ebufp);


}

/**************************************************************************************
 *
 * fm_rate_pol_handle_dropped_calls()
 *
 * Routine to handle dropped call specific logic in re-rating flow.
 *
 * 1. Populates the /event.DROPPED_CALL_QUANTITY with the value taken from 
 *    CANDIDATE_RUMS.QUANTITY matching the Dropped call RUM.
 *
 * 2. Populates the /event.CALL_TYPE to Continuation call if the Dropped calls 
 *    candidate rums is present, else populated as Normal call.
 *
 * Return values: returns the modified event substruct in the output flist (r_flistpp).
 *
 *************************************************************************************/
static void
fm_rate_pol_handle_dropped_calls(
	pin_flist_t	*i_flistp,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*event_flistp 		= NULL;
	pin_flist_t	*candidate_rums_flistp 	= NULL;
	pin_cookie_t	cookie			= NULL;
	char		*rum_name		= NULL;
	int32		elem_id			= 0;
	int32		call_type		= CALL_TYPE_NORMAL_CALL;
	pin_decimal_t	*qtyp			= NULL;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_rate_pol_handle_dropped_calls input flist", i_flistp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_rate_pol_handle_dropped_calls initial return flist", *r_flistpp);

	event_flistp = PIN_FLIST_SUBSTR_GET(*r_flistpp, PIN_FLD_EVENT,
				0, ebufp);

	while ((candidate_rums_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp,
		PIN_FLD_CANDIDATE_RUMS, &elem_id, 1, &cookie, ebufp)) != NULL) {

		rum_name = (char*)PIN_FLIST_FLD_GET(candidate_rums_flistp,
				PIN_FLD_RUM_NAME, 0, ebufp);

		if (rum_name && (strcmp(rum_name, DROPPED_CALL_RUM_NAME) == 0)) {

			qtyp = (pin_decimal_t*)PIN_FLIST_FLD_GET(candidate_rums_flistp,
				PIN_FLD_QUANTITY, 0, ebufp);

			/* Set the call type to continuation call.
			 * NOTE: We are not checking the value of the Candidate rums
			 *       quantity here. Presence of Dropped call RUM is key
			 *       to tag the event as Continuation call.
			 */
			call_type = CALL_TYPE_CONTINUATION_CALL;

			/* Dropped call quantity is set only if we have the quantity
			 * inside candidate rums array, else not required.
			 */
			PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_DROPPED_CALL_QUANTITY,
					qtyp, ebufp);
			break;
		}

	}

	/* Set the call type (Continuous call or Normal call) */
	PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_CALL_TYPE, &call_type, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_rate_pol_handle_dropped_calls return flist", *r_flistpp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_rate_pol_handle_dropped_calls error", ebufp);
	}

	return;
}

static void
fm_rate_pol_process_mostcalled_event(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp)
{
        pin_flist_t     *pr_flistp = NULL;
        pin_flist_t     *er_flistp = NULL;
        pin_flist_t     *r_flistp = NULL;
        pin_flist_t     *e_flistp = NULL;
        pin_flist_t     *dd_flistp = NULL;
        pin_flist_t     *ii_flistp = NULL;
        pin_flist_t     *temp_flistp = NULL;
        pin_flist_t     *d_flistp = NULL;
        poid_t      	*off_pdp = NULL;
        void		*vp = NULL;

	pin_decimal_t   *t_cost = NULL;
        pin_decimal_t   *t_dur = NULL;
        int32		count = 0; 
	char		*mc_list = "";

        t_cost = pin_decimal ("0.0", ebufp);
        t_dur = pin_decimal ("0.0", ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
                return;
        }
        PIN_ERR_CLEAR_ERR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_rate_pol_process_mostcalled_event input flist", i_flistp);

	/***********************************************
	* 1. Get the offering_obj from the i_flistp.
	* 2. Search for /profile/mostcalled for the given
	*    offering_obj.
	* 3. Get the count, impact_cat, criterion etc. from
   	*    the profile.
      	* 4. Create a search template using the above values
	* 5. Execute PCM_OP_SEARCH.
	* 6. Loop thru the results and aggregate cost, occurences
	*    and duration.
	* 7. Enrich the event flist with the data.
	* 8. Return the event flist.
	************************************************/

	/************************************************
	 * For a cycle discount event, there is only one
	 * discount in the DISCOUNT_LIST array. We'll get
	 * the offering_obj from that discount.
	 ************************************************/
	e_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_EVENT,0, ebufp);

	dd_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_DISCOUNT_DATA,
								1, ebufp);
	temp_flistp = PIN_FLIST_ELEM_GET(dd_flistp, PIN_FLD_DISCOUNTS,
						PIN_ELEMID_ANY, 1, ebufp);
	d_flistp = PIN_FLIST_ELEM_GET(temp_flistp, PIN_FLD_DISCOUNT_LIST,
						PIN_ELEMID_ANY, 1, ebufp);
	off_pdp = PIN_FLIST_FLD_GET(d_flistp, PIN_FLD_OFFERING_OBJ, 0, ebufp);

	/** Search for the /profile/mostcalled **/
	fm_rate_pol_search_mostcalled_profile(ctxp,
				off_pdp, &pr_flistp, ebufp);

	if(pr_flistp) {
		ii_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_INHERITED_INFO, 0, ebufp);
		vp = PIN_FLIST_FLD_GET(ii_flistp, PIN_FLD_LAST_BILL_T, 0, ebufp);
		PIN_FLIST_FLD_SET(e_flistp, PIN_FLD_LAST_BILL_T, vp, ebufp);
		vp = PIN_FLIST_FLD_GET(ii_flistp, PIN_FLD_NEXT_BILL_T, 0, ebufp);
		PIN_FLIST_FLD_SET(e_flistp, PIN_FLD_NEXT_BILL_T, vp, ebufp);

		/** Search for all the events for this cycle **/
		fm_rate_pol_search_mostcalled_events(ctxp,
					e_flistp, pr_flistp, &er_flistp, ebufp);

	}


	if(er_flistp && (PIN_FLIST_ELEM_COUNT(er_flistp,
                                PIN_FLD_RESULTS, ebufp) > 0)) {
		fm_rate_pol_aggregate_mostcalled_events(ctxp,
					er_flistp,pr_flistp,&r_flistp,ebufp);
	}

	if(r_flistp) {
	        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_rate_pol_process_mostcalled_event return flist", r_flistp);

		temp_flistp = PIN_FLIST_SUBSTR_GET(*r_flistpp, PIN_FLD_EVENT,
					0, ebufp);
		PIN_FLIST_SUBSTR_SET(temp_flistp, r_flistp,
			PIN_FLD_MOST_CALLED_INFO, ebufp);
	}
	else {
		/* We still populate zeros in the return flist*/

		temp_flistp = PIN_FLIST_SUBSTR_GET(*r_flistpp, PIN_FLD_EVENT,
					0, ebufp);
		d_flistp = PIN_FLIST_SUBSTR_ADD(temp_flistp, 
					PIN_FLD_MOST_CALLED_INFO, ebufp);

		PIN_FLIST_FLD_SET(d_flistp, PIN_FLD_MOST_CALLED_LIST, mc_list, ebufp);
        	PIN_FLIST_FLD_SET(d_flistp, PIN_FLD_AMOUNT, t_cost, ebufp);
        	PIN_FLIST_FLD_SET(d_flistp, PIN_FLD_QUANTITY, t_dur, ebufp);
        	PIN_FLIST_FLD_SET(d_flistp, PIN_FLD_COUNT, &count, ebufp);		
	}

	/***********************************************************
         * Cleanup.
         ***********************************************************/
        PIN_FLIST_DESTROY_EX(&pr_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&er_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	pin_decimal_destroy(t_cost);
        pin_decimal_destroy(t_dur);
	/***********************************************************
         * Errors? Log.
         ***********************************************************/
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_rate_pol_process_mostcalled_event error", ebufp);
        }

        return;	
}

/***************************************************
* This function searches for the /profile/mostcalled
* for the given offering_obj.
****************************************************/
static void
fm_rate_pol_search_mostcalled_profile(
        pcm_context_t   *ctxp,
        poid_t      *off_pdp,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp)
{

	pin_flist_t     *s_flistp = NULL;
	pin_flist_t     *sub_flistp = NULL;
	pin_flist_t     *p_flistp = NULL;
	pin_flist_t     *flistp = NULL;
	poid_t	*s_pdp = NULL;
	void		*vp = NULL;

	int64           database = 0;
	int32           sflags = SRCH_DISTINCT;

	if (PIN_ERR_IS_ERR(ebufp))
                return;
	PIN_ERR_CLEAR_ERR(ebufp);


	/**********************************************************
	* Check the input. Get the DB no.
	***********************************************************/

	/**Check if the off_pdp is NULL or if its a null poid **/
	if(PIN_POID_IS_NULL(off_pdp)){

        	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_WARNING, "Bad Offering_obj passed " \
			"to fm_rate_pol_search_mostcalled_profile");
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			(char *)PCM_OP_RATE_POL_PRE_RATING, ebufp);

		return;

	}

	database = PIN_POID_GET_DB(off_pdp);

	/***********************************************************
         * Allocate an flist for the search.
         ***********************************************************/
        s_flistp = PIN_FLIST_CREATE(ebufp);
        s_pdp = PIN_POID_CREATE(database, "/search", -1, ebufp);
        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&sflags, ebufp);

        /*************************************************************
         * Set the search template
         ************************************************************/
        vp = (void *)"select X from /profile/mostcalled where F1 = V1 ";
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, vp, ebufp);

        /*************************************************************
         * 1st arg. Matching  offering_obj
         ************************************************************/
        flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
        sub_flistp = PIN_FLIST_SUBSTR_ADD(flistp,
                        PIN_FLD_MOST_CALLED_INFO,ebufp);
        PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_OFFERING_OBJ,off_pdp, ebufp);

        /*************************************************************
         * Add the result fields we want to read
         ************************************************************/
        PIN_FLIST_ELEM_SET(s_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp);

        /*************************************************************
         * Debug. Search flist.
         ************************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search input flist", s_flistp);

        /*************************************************************
         * Do the search.
         ************************************************************/
        PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE,
                s_flistp, &p_flistp, ebufp);

        /*************************************************************
         * Debug. Return flist.
         ************************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Search result flist.", p_flistp);

	/************************************************************
	 * Copy the profile to the return flist
	 ************************************************************/
	if(p_flistp) {
		*r_flistpp = PIN_FLIST_ELEM_TAKE(p_flistp, 
					PIN_FLD_RESULTS,0,1, ebufp);
	}
	else {
		*r_flistpp =NULL;	
	}
        /***********************************************************
         * Cleanup.
         ***********************************************************/
        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&p_flistp, NULL);

        /***********************************************************
         * Errors? Log.
         ***********************************************************/
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_rate_pol_search_mostcalled_profile error", ebufp);
        }

        return;


}

/***************************************************
* This function creates a search template for event
* search and executes PCM_OP_SEARCH.
****************************************************/
static void
fm_rate_pol_search_mostcalled_events(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_flist_t     *p_flistp,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp)
{

	pin_flist_t	*s_flistp = NULL;
        pin_flist_t	*sub_flistp = NULL;
        pin_flist_t	*flistp = NULL;
        poid_t		*s_pdp = NULL;
        poid_t		*acct_pdp = NULL;
        poid_t		*event_pdp = NULL;
        char		*event_type = NULL;
	char		*evt_type = "/event/delayed/session/telco";	
        void		*vp = NULL;
	pin_flist_t     *svc_flistp = NULL;
	pin_flist_t     *t_flistp = NULL;
        pin_flist_t     *result_flistp = NULL;
	poid_t		*svc_pdp = NULL;
	char		template[1024];

        int64		database = 0;
        int32		sflags = SRCH_DISTINCT;
	int32		zero_val = 0;
	int32		argCnt = 0;
	int32		elem_id = 0;
    	pin_cookie_t	cookie = NULL;
	int32		svc_valid = 0;

        if (PIN_ERR_IS_ERR(ebufp))
                return;
        PIN_ERR_CLEAR_ERR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_rate_pol_search_mostcalled_events input", i_flistp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_rate_pol_search_mostcalled_events profile flist", p_flistp);
	/**************************************************************
	 * NOTE: The current implementation assumes only events of type
	 * telco/gsm and looks for fields under PIN_FLD_TELCO_INFO substruct. 
	 * For other types of events, this section of code
  	 * may need changes to fetch specific fields from the event.
	 **************************************************************/

        /**********************************************************
        * Check the input. Get the DB no.
        ***********************************************************/
	acct_pdp = (poid_t *) PIN_FLIST_FLD_GET(i_flistp, 
				PIN_FLD_ACCOUNT_OBJ,0, ebufp);
        database = PIN_POID_GET_DB(acct_pdp);

        /***********************************************************
         * Allocate an flist for the search.
         ***********************************************************/
        s_flistp = PIN_FLIST_CREATE(ebufp);
        s_pdp = PIN_POID_CREATE(database, "/search", -1, ebufp);
        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&sflags, ebufp);

        /*************************************************************
         * Set the search template
         ************************************************************/
	strcpy(template, "select X from /event/delayed/session/telco where ");

        /*************************************************************
         * 1st arg. Matching account obj.
         ************************************************************/
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, ++argCnt, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACCOUNT_OBJ,acct_pdp, ebufp);
	sprintf(template, "%sF%d = V%d ", template, argCnt, argCnt);

        /*************************************************************
         * 2nd arg. Matching service obj(s)
        ************************************************************/
	svc_pdp = PIN_FLIST_FLD_GET(p_flistp, PIN_FLD_SERVICE_OBJ,1, ebufp);
        if(!PIN_POID_IS_NULL(svc_pdp)){
	   /*********************************************************
	    * Check if the service is a subscription service and fetch
	    * its member services. If member services are found, add
            * them as search arguments in the events search.
	    *********************************************************/
	   fm_rate_pol_get_member_services(ctxp,svc_pdp, &svc_flistp, ebufp);
           if((svc_flistp) && (PIN_FLIST_ELEM_COUNT(svc_flistp,
				PIN_FLD_RESULTS, ebufp) > 0)){
		/**********************************************************
		 * If there are too many member services, the search 
		 * template becomes too long. Hence we will check for the
		 * member count. If more than 25, we add a IN statement
	   	 * in the search template string.
		 **********************************************************/
		if(PIN_FLIST_ELEM_COUNT(svc_flistp,
                                PIN_FLD_RESULTS, ebufp) > 25) {
		   	svc_valid = 1;
		} else {
           	   	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, ++argCnt, ebufp);
           	   	PIN_FLIST_FLD_SET(flistp, PIN_FLD_SERVICE_OBJ,svc_pdp, ebufp);
	   	   	sprintf(template, "%sAND ( F%d = V%d ", template, argCnt, argCnt);

		   	while(t_flistp = PIN_FLIST_ELEM_GET_NEXT(svc_flistp, 
					PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)){
				svc_pdp = PIN_FLIST_FLD_GET(t_flistp, PIN_FLD_POID,
							0, ebufp);
           			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 	
							++argCnt, ebufp);
           			PIN_FLIST_FLD_SET(flistp, PIN_FLD_SERVICE_OBJ,
							svc_pdp, ebufp);
	   			sprintf(template, "%sOR F%d = V%d ", template, 
							argCnt, argCnt);

		   	}
		   	sprintf(template, "%s) ", template);
		}
	    }
	    else {
                flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, ++argCnt, 
									ebufp);
                PIN_FLIST_FLD_SET(flistp, PIN_FLD_SERVICE_OBJ,svc_pdp, ebufp);
		sprintf(template, "%sAND F%d = V%d ", template, argCnt, 
									argCnt);
	    }
	}

	/*************************************************************
         * 3rd arg. event.end_t >= cycle_start_t
         ************************************************************/
        flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, ++argCnt, ebufp);
        vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_LAST_BILL_T,0, ebufp);

        PIN_FLIST_FLD_SET(flistp, PIN_FLD_END_T, vp, ebufp);
	sprintf(template, "%sAND F%d >= V%d ", template, argCnt, argCnt);

	/*************************************************************
         * 4th arg. event.end_t < cycle_end_t
         ************************************************************/
        flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, ++argCnt, ebufp);
        vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_NEXT_BILL_T,0, ebufp);

        PIN_FLIST_FLD_SET(flistp, PIN_FLD_END_T, vp, ebufp);
	sprintf(template, "%sAND F%d < V%d ", template, argCnt, argCnt);

	/*************************************************************
         * 5th arg. Matching event_type 
         ************************************************************/
        sub_flistp = PIN_FLIST_SUBSTR_GET(p_flistp,
                        PIN_FLD_MOST_CALLED_INFO, 0, ebufp);
	event_type =(char *) PIN_FLIST_FLD_GET(sub_flistp, 
                                PIN_FLD_EVENT_TYPE,0, ebufp);

	/**Add Event Type only if it is populated in the profile **/
	if(event_type && strcmp(event_type, "") != 0){

		if(!strncmp(event_type, evt_type, strlen(evt_type))){

					
			flistp = PIN_FLIST_ELEM_ADD(s_flistp, 
					PIN_FLD_ARGS, ++argCnt, ebufp);
			event_pdp = PIN_POID_CREATE(database, 
					event_type, -1, ebufp);
       			PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID,
					 event_pdp, ebufp);
			sprintf(template, "%sAND F%d LIKE V%d ", template, 
						argCnt, argCnt);
		}
		else {
			/********************************************
			 * We handle only /e/d/s/telco and its 
			 * subtypes. If anything else is passed, we
			 * bail out.
			 *******************************************/
			 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_WARNING, 
				"Event Type not a subclass of " \
				"/event/delayed/session/telco event");
			 *r_flistpp = NULL;

			 return;	


		}
	}

	/**Add the template String ***/
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, template, ebufp);

	/************************************************************
         * Add the result fields we want to read.
	 * NOTE: This section of code may need changes if more fields
	 * need to be compared. Enhance the results flist to fetch
         * more event fields.
         ************************************************************/
        flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);
	sub_flistp = PIN_FLIST_SUBSTR_ADD(flistp,PIN_FLD_TELCO_INFO,
								ebufp);
        PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_CALLED_TO, NULL, ebufp);
	sub_flistp = PIN_FLIST_ELEM_ADD(flistp,PIN_FLD_BAL_IMPACTS,
						PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_AMOUNT, NULL, ebufp);
        PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_QUANTITY, NULL, ebufp);
        PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_IMPACT_TYPE, &zero_val, ebufp);
        PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_IMPACT_CATEGORY, NULL, ebufp);
        PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_RESOURCE_ID, &zero_val, ebufp);

        /*************************************************************
         * Debug. Search flist.
         ************************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search usage input", s_flistp);

        /*************************************************************
         * Do the search.
         ************************************************************/
        PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE,
		s_flistp, &result_flistp, ebufp);

        /*************************************************************
         * Debug. Return flist.
         ************************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Search result flist.", result_flistp);

	if(svc_valid){
		elem_id = 0;
		cookie = NULL;
		while(t_flistp = PIN_FLIST_ELEM_GET_NEXT(result_flistp,
                                PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)){

			svc_pdp = PIN_FLIST_FLD_GET(t_flistp,
					 PIN_FLD_SERVICE_OBJ, 0, ebufp);

			if(fm_rate_pol_svc_validate(svc_pdp, svc_flistp, 
								ebufp))	{
				/*Add this event to the outgoing flist */
				if(*r_flistpp == NULL) {
					*r_flistpp = PIN_FLIST_CREATE(ebufp);
				} 
				
				PIN_FLIST_ELEM_SET(*r_flistpp, t_flistp,
					PIN_FLD_RESULTS, elem_id, ebufp);
				
			}
		}

	} else {
		*r_flistpp = PIN_FLIST_COPY(result_flistp, ebufp);
	}

        /***********************************************************
         * Cleanup.
         ***********************************************************/
        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&svc_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&result_flistp, NULL);

        /***********************************************************
         * Errors? Log.
         ***********************************************************/
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_rate_pol_search_mostcalled_events error", ebufp);
        }

        return;

}

/*******************************************************
* This function loops thru the events and aggregates the
* cost,duration and # of events for the most called numbers.
*******************************************************/
static void
fm_rate_pol_aggregate_mostcalled_events(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_flist_t     *p_flistp,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp)
{

	pin_flist_t     *events_flistp = NULL;
	pin_flist_t     *e_flistp = NULL;
	pin_flist_t     *r_flistp = NULL;
	pin_flist_t     *bi_flistp = NULL;
	pin_flist_t     *t_flistp = NULL;
	pin_flist_t     *temp_flistp = NULL;
	pin_flist_t     *flistp = NULL;
	pin_decimal_t	*amountp = NULL;
	pin_decimal_t	*total_cost = NULL;
	pin_decimal_t	*durationp = NULL;
	pin_decimal_t	*total_dur = NULL;
	pin_decimal_t	*zerop = pin_decimal("0.0",ebufp);

	int32           elem_id = 0;
        pin_cookie_t    cookie = NULL;

	int32           elem_id1 = 0;
        pin_cookie_t    cookie1 = NULL;

	char		*imp_cat = NULL;
	char            *categories[MAX_IMP_CATEGORY_SIZE], *last;
	char		*impact_cat = NULL;
	char		*call_no = NULL;
	char		*criterion = NULL;
	int32		*mc_count = NULL;
	int32		*imp_type = NULL;
	int32           *res_id = NULL;
	int32           total = 0;
	int32           ic_flag= PIN_BOOLEAN_TRUE;
	int32           index = 0;

	int32           elem_id2 = 0;
	void		*vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp)) {
                return;
        }
        PIN_ERR_CLEAR_ERR(ebufp);


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_rate_pol_aggregate_mostcalled_events input", i_flistp);

	/** Create a hash flist to store the aggregate values **/
	events_flistp = PIN_FLIST_CREATE(ebufp);
	r_flistp = PIN_FLIST_CREATE(ebufp);

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(events_flistp, PIN_FLD_POID, vp, ebufp);

	/** Get the required params from the profile **/
	flistp = PIN_FLIST_SUBSTR_GET(p_flistp,
                        PIN_FLD_MOST_CALLED_INFO, 0, ebufp);
        mc_count =(int *)PIN_FLIST_FLD_GET(flistp,
                                PIN_FLD_COUNT,0, ebufp);
        criterion =(char *)PIN_FLIST_FLD_GET(flistp,
                                PIN_FLD_CRITERION,0, ebufp);
        imp_cat =(char *)PIN_FLIST_FLD_GET(flistp,
                                PIN_FLD_IMPACT_CATEGORY,0, ebufp);
	/**************************************************
	 * The IMPACT_CATEGORY field in the profile may
 	 * semi-colon(;) seperated values. In this case,
	 * we need to tokenize it. For example, if the value
	 * is "Europe;Domestic;LongDistance", then the tokens
	 * would look like:
	 *       categories[0] = "Europe" 
	 *	 categories[1] = "Domestic"
	 *	 categories[2] = "LongDistance"
	 **************************************************/
	if(imp_cat && strcmp(imp_cat, "") != 0) {
		categories[0] = (char *)strtok_r(imp_cat, IMPACT_CATEGORY_SEPERATOR, &last);
         	while (categories[++total] = 
				(char *)strtok_r(NULL, IMPACT_CATEGORY_SEPERATOR, &last));
	}	

	/** Loop thru the events in the input **/
	while (e_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp,
				PIN_FLD_RESULTS, &elem_id, 1 , 
						&cookie, ebufp)){


		/** Initialize the counters **/
		total_cost = pin_decimal ("0.0", ebufp);
		total_dur = pin_decimal ("0.0", ebufp);

		/* For each event, loop thru the balance_impacts array*/
		elem_id1 = 0;
		cookie1 = NULL;
		while (bi_flistp = PIN_FLIST_ELEM_GET_NEXT(e_flistp,
                                	PIN_FLD_BAL_IMPACTS, &elem_id1, 1, 
                                                &cookie1, ebufp)){
		
			imp_type = PIN_FLIST_FLD_GET(bi_flistp, 
					PIN_FLD_IMPACT_TYPE, 0, ebufp);
			/****************************************************** 
			 * Consider only rated balance impacts(Gross). Ignore
			 * discount and tax balance impacts. If NET value is 
			 * desired, add other impact_types. Refer to pin_rate.h
			 * for all possible impact_types.
			 ******************************************************/
			if(imp_type && (*imp_type !=  PIN_IMPACT_TYPE_INTEGRATE_PRERATED) &&
			   (*imp_type !=  PIN_IMPACT_TYPE_INTEGRATE_RERATED)){
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
				"Impact Type Mismatch. " \
				"Not cosidered for aggregation.");
				continue;

			}


			/** Lets look at the currency buckets only **/
			res_id = (int32 *)PIN_FLIST_FLD_GET(bi_flistp,
                                        PIN_FLD_RESOURCE_ID, 0, ebufp);

			if((res_id != NULL) &&
                                !PIN_BEID_IS_CURRENCY(*res_id)){
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
				"BEID ID  Mismatch. "\
				"Not cosidered for aggregation.");
                                continue;

                        }

			/*********************************************** 
			 * Compare the impact category with that from 
			 * the profile. If they are not same, skip this 
			 * bal_impact. 
			 ***********************************************/

			impact_cat = PIN_FLIST_FLD_GET(bi_flistp, 
					PIN_FLD_IMPACT_CATEGORY, 0, ebufp);

			/*******************************************
			 * If the profile has no value in the IMPACT
			 * _CATEGORY field, then treat it as ALL. Else
			 * compare with each impact category passed.
			 ********************************************/
			if(imp_cat && strcmp(imp_cat, "") != 0){
				ic_flag = PIN_BOOLEAN_FALSE;
				for (index = total - 1; index >= 0 && 
					ic_flag == PIN_BOOLEAN_FALSE; index--) {

				   /********************************************
				    * Compare the impact categories if its not
				    * empty in the profile object.
				    *******************************************/
				   if(impact_cat && (strcmp(impact_cat, (char *)categories[index]) == 0)){

					ic_flag = PIN_BOOLEAN_TRUE;

				   }

				}
			}

			if(!ic_flag){
			
				continue;
			}

			/*
			 * Get the amount and add it to the aggregator
			 */
			amountp = PIN_FLIST_FLD_GET(bi_flistp, 
						PIN_FLD_AMOUNT, 0, ebufp);
			pin_decimal_add_assign(total_cost, amountp, ebufp);	

			/*
			 * Get the duration and add it to the aggregator
			 *
			 */
			durationp = PIN_FLIST_FLD_GET(bi_flistp, 
						PIN_FLD_QUANTITY, 0, ebufp);
			pin_decimal_add_assign(total_dur, durationp, ebufp);


		} /*inner while*/

		/***************************************************
		 * Get the Called_to number. Add the aggregated values
		 * for this number to the hashmap flist
		 ***************************************************/

		t_flistp = PIN_FLIST_SUBSTR_GET(e_flistp, PIN_FLD_TELCO_INFO, 1, ebufp);

		if(t_flistp) {
			if((pin_decimal_compare(total_cost,zerop,ebufp) != 0) || (pin_decimal_compare(total_dur,zerop,ebufp) != 0))
			{

				call_no = (char *) PIN_FLIST_FLD_GET(t_flistp,
							 PIN_FLD_CALLED_TO, 0, ebufp);

				temp_flistp = PIN_FLIST_ELEM_ADD(events_flistp, 
							PIN_FLD_AGGREGATE_AMOUNTS,
							elem_id2, ebufp);

				PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_AMOUNT,
								 total_cost, ebufp);
        			PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_QUANTITY,
								 total_dur, ebufp);
        			PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_CALLED_TO, 
								call_no, ebufp);

				elem_id2++;
			}

		}	

	} /*outer while*/

	/**********************************************
	 * At this point the hash flist is populated with
	 * the event-level aggregated value. Now we need 
	 * to find the most called numbers.
	 **********************************************/

	fm_rate_pol_find_mostcalled_nums(events_flistp, mc_count, 
					criterion, &r_flistp, ebufp);

	/** Copy to the outgoing Flist **/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_rate_pol_aggregate_mostcalled_events return", r_flistp);

        *r_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);

	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&events_flistp, NULL);
        pin_decimal_destroy(total_cost);
        pin_decimal_destroy(total_dur);
        pin_decimal_destroy(zerop);
        /***********************************************************
         * Errors? Log.
         ***********************************************************/
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_rate_pol_aggregate_mostcalled_events error", ebufp);
        }

        return;
}

/*************************************************
 * This function analyzes the hash flist and finds
 * the most called numbers based on either cost, dur
 * or occurrences.
 *************************************************/
static void
fm_rate_pol_find_mostcalled_nums (
        pin_flist_t     *i_flistp,
        int32     	*mc_count,
        char     	*mc_criterion,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp)
{
	pin_flist_t     *array_flistp = NULL;
        pin_flist_t     *max_flistp = NULL;
        pin_flist_t     *srt_flistp = NULL;
        pin_flist_t     *temp_flistp = NULL;
        pin_flist_t     *r_flistp = NULL;

        pin_decimal_t   *temp_decimal = NULL;
        pin_decimal_t   *t_cost = NULL;
        pin_decimal_t   *t_dur = NULL;
        pin_decimal_t   *t_count = NULL;

	char		*called_to = NULL;
	char		*ncalled_to = NULL;
	char		mc_list[MAX_CALLLIST_SIZE];
        int32           count = 0;
        int32           rec_id = 0;
        int32           iter = 0;

	int32		elem_id = 0;
	pin_cookie_t	cookie = NULL;

	if (PIN_ERR_IS_ERR(ebufp)) {
                return;
        }
        PIN_ERR_CLEAR_ERR(ebufp);


	t_cost = pin_decimal ("0.0", ebufp);
        t_dur = pin_decimal ("0.0", ebufp);
	memset(mc_list,'\0',sizeof(mc_list));

	max_flistp = PIN_FLIST_CREATE(ebufp);
	r_flistp = PIN_FLIST_CREATE(ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_rate_pol_find_mostcalled_nums input", i_flistp);

	/********************************************
	 * Bail out if the input flist is null or
	 * empty.
	 ********************************************/
	
	 if((i_flistp) && (PIN_FLIST_ELEM_COUNT(i_flistp,
				PIN_FLD_AGGREGATE_AMOUNTS, ebufp) > 0)) {
		/****************************************************
	 	* The input contains multiple arrays entries for each
	 	* called_to number. We need to  do a second-level
	 	* aggregation based on the called number. For that, 
		* we need to sort the flist.
	 	******************************************************/

		srt_flistp = PIN_FLIST_CREATE(ebufp);

		temp_flistp = PIN_FLIST_ELEM_ADD(srt_flistp,
                          PIN_FLD_AGGREGATE_AMOUNTS, PIN_ELEMID_ANY, ebufp);

		PIN_FLIST_FLD_SET(temp_flistp, 
				PIN_FLD_CALLED_TO, NULL, ebufp);

		PIN_FLIST_SORT(i_flistp, srt_flistp, 0, ebufp);

		PIN_FLIST_DESTROY_EX(&srt_flistp, NULL);

		/*********************************************
	 	* Loop thru the arrays and aggregate the amounts
	 	* per called_to number
	 	*********************************************/
		count = 1;
		rec_id = 0;
		called_to = "default";
		while(array_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp,
					PIN_FLD_AGGREGATE_AMOUNTS,
						&elem_id, 1, &cookie, ebufp)){

			ncalled_to = (char *)PIN_FLIST_FLD_GET(array_flistp,
                                         PIN_FLD_CALLED_TO, 0, ebufp);

			if(ncalled_to && strcmp(called_to, ncalled_to) == 0){
				/*Aggregate*/
				temp_decimal =(pin_decimal_t *)PIN_FLIST_FLD_GET(array_flistp,
                                         PIN_FLD_AMOUNT, 0, ebufp);
               	         	pin_decimal_add_assign(t_cost, temp_decimal, ebufp);  

               		        temp_decimal =(pin_decimal_t *)PIN_FLIST_FLD_GET(array_flistp,
                                         PIN_FLD_QUANTITY, 0, ebufp);
               		        pin_decimal_add_assign(t_dur, temp_decimal, ebufp);   

               		        count++;

			} 
			else {
				/*******************************************
			 	* This is a new number. Save the aggregated
			 	* values in a temp flist.
			 	*******************************************/
				if(strcmp(called_to, "default") != 0) {
					temp_flistp = PIN_FLIST_ELEM_ADD(max_flistp, 
					PIN_FLD_AGGREGATE_AMOUNTS, 
							rec_id++, ebufp);

					PIN_FLIST_FLD_SET(temp_flistp, 
						PIN_FLD_CALLED_TO, called_to, ebufp);
					PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_AMOUNT,
							t_cost, ebufp);
					PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_QUANTITY,
							t_dur, ebufp);
					PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_COUNT,
							&count, ebufp);

				}

				/** Reset the counters **/

				count = 0;
				t_cost = pin_decimal ("0.0", ebufp);
        			t_dur = pin_decimal ("0.0", ebufp);	
		
				/** Start the aggregation for the new number **/

				called_to = (char *)PIN_FLIST_FLD_GET(array_flistp,
                                         PIN_FLD_CALLED_TO, 0, ebufp);
                        	temp_decimal =(pin_decimal_t *)PIN_FLIST_FLD_GET(array_flistp,
                                         PIN_FLD_AMOUNT, 0, ebufp);
                        	pin_decimal_add_assign(t_cost, temp_decimal, ebufp);

                        	temp_decimal =(pin_decimal_t *)PIN_FLIST_FLD_GET(array_flistp,
                                         PIN_FLD_QUANTITY, 0, ebufp);
                        	pin_decimal_add_assign(t_dur, temp_decimal, ebufp);

                        	count++;				

			}


		} /*While*/

		/*********************************************
		 * Add the last set of aggregated values to the
		 * max_flistp
		 *********************************************/
		if(count > 0) {
			temp_flistp = PIN_FLIST_ELEM_ADD(max_flistp,
                                        PIN_FLD_AGGREGATE_AMOUNTS,
                                                        rec_id++, ebufp);

                        PIN_FLIST_FLD_SET(temp_flistp,
                                    PIN_FLD_CALLED_TO, called_to, ebufp);
                        PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_AMOUNT,
                                        t_cost, ebufp);
                        PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_QUANTITY,
                                         t_dur, ebufp);
                        PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_COUNT,
                                        &count, ebufp);

		}

		/*********************************************
	 	* At this point, we have the final aggregated
	 	* values in the max_flist. We now need to 
	 	* determine the most called numbers based on 
	 	* either cost, duration or occurrences.
	 	*********************************************/
	
		if((PIN_FLIST_ELEM_COUNT(max_flistp,
                                PIN_FLD_AGGREGATE_AMOUNTS, ebufp) > 0)) {

		   /** sort the flist based on cost, dur or count **/

		   srt_flistp = PIN_FLIST_CREATE(ebufp);

        	   temp_flistp = PIN_FLIST_ELEM_ADD(srt_flistp,
                          PIN_FLD_AGGREGATE_AMOUNTS, PIN_ELEMID_ANY, ebufp);

		   if(strcasecmp(mc_criterion, "COST") == 0) {

           		PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_AMOUNT, 
								NULL, ebufp);
		   }
		   else if(strcasecmp(mc_criterion, "DURATION") == 0) {

           		PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_QUANTITY, 
								NULL, ebufp);
		   } else {

           		PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_COUNT, 
								NULL, ebufp);
		   }

	           PIN_FLIST_SORT_REVERSE(max_flistp, srt_flistp, 0, ebufp);

        	   PIN_FLIST_DESTROY_EX(&srt_flistp, NULL);

		   PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
               	    "fm_rate_pol_find_mostcalled_nums MAX  FLIST", max_flistp);

		   /*******************************************
	 	   * We now just pick the n most called numbers
	 	   * and aggregate their totals
	 	   *******************************************/

		   count = 0;
		   t_cost = pin_decimal ("0.0", ebufp);
       		   t_dur = pin_decimal ("0.0", ebufp);
		   elem_id = 0;
        	   cookie = NULL;	
		   for(iter = 0; mc_count && iter < *mc_count; iter++) {

			temp_flistp = PIN_FLIST_ELEM_GET_NEXT(max_flistp,
				PIN_FLD_AGGREGATE_AMOUNTS, &elem_id,
						 1, &cookie,  ebufp);
	
			if(!temp_flistp){
				/***************************************
				 * This means the list of called_to nums
				 * is less that the count specified in the
				 * profile. We'll just return whatever we
				 * found.
				 ****************************************/
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_WARNING, 
                        	"No. of called numbers LESS than the " \
				"configured most called count!");
				break;

			}
	
			temp_decimal = (pin_decimal_t *)PIN_FLIST_FLD_GET(temp_flistp,
                                         PIN_FLD_AMOUNT, 0, ebufp);
               		pin_decimal_add_assign(t_cost, temp_decimal, ebufp);

                	temp_decimal = (pin_decimal_t *)PIN_FLIST_FLD_GET(temp_flistp,
                                         PIN_FLD_QUANTITY, 0, ebufp);
                	pin_decimal_add_assign(t_dur, temp_decimal, ebufp);

                	t_count = (int32 *)PIN_FLIST_FLD_GET(temp_flistp,
                                         PIN_FLD_COUNT, 0, ebufp);

			if (t_count) {
				count = *((int32 *)t_count) + count;
			}

			called_to = (char *)PIN_FLIST_FLD_GET(temp_flistp,
                                         PIN_FLD_CALLED_TO, 0, ebufp);

			if(iter == 0){

				pin_snprintf(mc_list, sizeof(mc_list), "%s", called_to);

			} else {
				strncat(mc_list, CALLLIST_SEPERATOR, (sizeof(mc_list) - strlen(mc_list) - 1));
				strncat(mc_list, called_to, (sizeof(mc_list) - strlen(mc_list) - 1));
			}
		   } /*for*/
	   	} /*If*/
	}/*If*/

	/**************************************************** 
	 * Add everything to the return flist. We still send
	 * the return flist with the fields event if the
	 * aggregation counters are zero.
	 ****************************************************/

	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_MOST_CALLED_LIST, mc_list, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_AMOUNT, t_cost, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_QUANTITY, t_dur, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_COUNT, &count, ebufp);

	/** Copy to the outgoing Flist **/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_rate_pol_find_mostcalled_nums output", r_flistp);

	*r_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);

        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&max_flistp, NULL);
	pin_decimal_destroy(t_cost);
	pin_decimal_destroy(t_dur);

	/***********************************************************
         * Errors? Log.
         ***********************************************************/
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_rate_pol_find_mostcalled_nums error", ebufp);
        }

        return;	

}

/***************************************************
* This function searches for the member services
* for a given subscription service.
****************************************************/
static void
fm_rate_pol_get_member_services(
        pcm_context_t   *ctxp,
        poid_t      	*svc_pdp,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp)
{

	pin_flist_t     *s_flistp = NULL;
	pin_flist_t     *flistp = NULL;
	poid_t		*s_pdp = NULL;
	void		*vp = NULL;

	int64           database = 0;
	int32           sflags = SRCH_DISTINCT;

	if (PIN_ERR_IS_ERR(ebufp))
                return;
	PIN_ERR_CLEAR_ERR(ebufp);


	/**********************************************************
	* Check the input. Get the DB no.
	***********************************************************/

	/**Check if the svc_pdp is NULL or if its a null poid **/
	if(PIN_POID_IS_NULL(svc_pdp)){

        	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_WARNING, "Bad Service Obj passed " \
			"to fm_rate_pol_get_member_services");
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			(char *)PCM_OP_RATE_POL_PRE_RATING, ebufp);

		return;

	}

	database = PIN_POID_GET_DB(svc_pdp);

	/***********************************************************
         * Allocate an flist for the search.
         ***********************************************************/
        s_flistp = PIN_FLIST_CREATE(ebufp);
        s_pdp = PIN_POID_CREATE(database, "/search", -1, ebufp);
        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&sflags, ebufp);

        /*************************************************************
         * Set the search template
         ************************************************************/
        vp = (void *)"select X from /service where F1 = V1 ";
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, vp, ebufp);

        /*************************************************************
         * 1st arg. Matching  subscription_obj
         ************************************************************/
        flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_SUBSCRIPTION_OBJ,svc_pdp, ebufp);

        /*************************************************************
         * Add the result fields we want to read
         ************************************************************/
        flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 
						PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID,NULL, ebufp);

        /*************************************************************
         * Debug. Search flist.
         ************************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Service Search input flist", s_flistp);

        /*************************************************************
         * Do the search.
         ************************************************************/
        PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE,
                s_flistp, r_flistpp, ebufp);

        /*************************************************************
         * Debug. Return flist.
         ************************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Service Search result flist.", *r_flistpp);

        /***********************************************************
         * Cleanup.
         ***********************************************************/
        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);

        /***********************************************************
         * Errors? Log.
         ***********************************************************/
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_rate_pol_get_member_services error", ebufp);
        }

        return;

}

static int32
fm_rate_pol_svc_validate(
        poid_t          *svc_pdp,
        pin_flist_t     *svc_flistp,
        pin_errbuf_t    *ebufp) 
{
	pin_flist_t     *t_flistp = NULL;
	int32		result = 0;
	poid_t		*s_pdp = NULL;	
	int32 		elem_id = 0;
	pin_cookie_t	cookie = NULL;

	while(t_flistp = PIN_FLIST_ELEM_GET_NEXT(svc_flistp,
                                PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)){

	        s_pdp = PIN_FLIST_FLD_GET(t_flistp, PIN_FLD_POID, 0, ebufp); 
		
		if (PIN_POID_COMPARE(svc_pdp, s_pdp, 0, ebufp) == 0) {
			result = 1;
			break;
		}

	}

	return result;	
}


static void
fm_rate_pol_update_flag_for_hold(
        pcm_context_t   *ctxp,
	cm_op_info_t	*opstackp,
        pin_flist_t     *i_flistp,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp)
{
	int32		stack_opcode = 0;
	int32		initial_opcode = 0;
	int32		hold_svc_found = 0;
	int32		topup_found = 0;
	int32		cancel_found = 0;
	int32		hold_flags = PIN_RATE_FLG_INVERTED + PIN_RATE_FLG_PRO_FIRST + PIN_RATE_FLG_PRO_LAST;
	int32		topup_flags = PIN_RATE_FLG_INVERTED + PIN_RATE_FLG_PRO_LAST;
	int32		update_req = 0;
	char		debug_msg[255];
	char		*evt_type = NULL;
	char		*svc_type = NULL;
	char		*tmp_str = NULL ;
	pin_flist_t	*initial_flistp = NULL;
	pin_flist_t	*cycle_info = NULL;
	pin_flist_t	*e_flistp = NULL;
	pin_flist_t	*read_in_flistp = NULL;
	pin_flist_t	*read_out_flistp = NULL;
	pin_flist_t	*prod_flistp = NULL;
	time_t		*cycle_fee_start = NULL;
	time_t          current_t = 0;
	poid_t		*e_pdp = NULL;
	poid_t		*svc_pdp = NULL;
	poid_t		*prod_pdp = NULL;
	pin_decimal_t		*thousand = pbo_decimal_from_str("1000.0",ebufp);
	pin_decimal_t		*priority      = NULL;
	int32		stop_change_refund = 0;
	int32		cancel_deal_found = 0;
	int32		change_found = 0;
	int32		prty = 0;
	int32		mod_prty = 0;

	if (PIN_ERR_IS_ERR(ebufp)) {
                return;
        }
        PIN_ERR_CLEAR_ERR(ebufp);


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_rate_pol_update_flag_for_hold input", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "Return flist at begining", *r_flistpp);
	while(opstackp != NULL)
	{
		stack_opcode = opstackp->opcode;
		sprintf(debug_msg, "OPCODE=%d", stack_opcode);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
		initial_flistp = opstackp->in_flistp;
		initial_opcode = opstackp->opcode;
		if ((initial_opcode == MSO_OP_CUST_HOLD_SERVICE))
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"MSO_OP_CUST_HOLD_SERVICE input FLIST",initial_flistp);
			hold_svc_found = 1;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Hold Service Found");
			break;
		}
		if ((initial_opcode == MSO_OP_CUST_CANCEL_PLAN))
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"MSO_OP_CUST_CANCEL_PLAN input FLIST",initial_flistp);
			{
				cancel_found = 1;
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Cancel Found");
				// Do not add break here since we have to find both cancel plan
				// and topup_plan. i.e Cancel API called from Topup.
			}

		}
		if ((initial_opcode == MSO_OP_CUST_TOP_UP_BB_PLAN))
		{

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"MSO_OP_CUST_TOP_UP_BB_PLAN input FLIST",initial_flistp);
			{
				topup_found = 1;
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Topup Found");
				break;
			}

		}
		if ((initial_opcode == PCM_OP_SUBSCRIPTION_CANCEL_DEAL))
		{

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"PCM_OP_SUBSCRIPTION_CANCEL_DEAL input FLIST",initial_flistp);
			{
				cancel_deal_found = 1;
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Cancel deal Found");
				// Do not add break here since we have to find both subscription_cancel_deal
				// and change plan. i.e Cancel Deal API called from Chnage Plan API.
			}

		}
		if ((initial_opcode == MSO_OP_CUST_CHANGE_PLAN))
		{

			
			{
				change_found = 1;
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Change Found");
				break;
			}

		}
		opstackp = opstackp->next;

	}
	if (cancel_deal_found && change_found)
	{
		// If cancel deal is called from change_plan then verify
		//	1. If the service is broadband
		//	2. If the cancelled plan is prepaid
		// If above conditions are met then set stop_change_refund = 1
		e_flistp = PIN_FLIST_SUBSTR_GET(i_flistp,PIN_FLD_EVENT,1, ebufp);
		if(e_flistp)
		   svc_pdp = PIN_FLIST_FLD_GET(e_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		if (svc_pdp)
		{
			svc_type = (char *)PIN_POID_GET_TYPE(svc_pdp);
			if((svc_type) && (strstr(svc_type, MSO_BB)) )
			{
				prod_flistp = PIN_FLIST_SUBSTR_GET(e_flistp,PIN_FLD_PRODUCT,1, ebufp);
				if(prod_flistp)
				{
					prod_pdp = PIN_FLIST_FLD_GET(prod_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
					if (prod_pdp)
					{
						read_in_flistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_SET(read_in_flistp, PIN_FLD_POID, (void *)prod_pdp, ebufp);
						PIN_FLIST_FLD_SET(read_in_flistp, PIN_FLD_PRIORITY, NULL, ebufp);
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read flds input",read_in_flistp);
						PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_in_flistp, &read_out_flistp, ebufp);
						PIN_FLIST_DESTROY_EX(&read_in_flistp, NULL);
						if (PIN_ERR_IS_ERR(ebufp))
						{
							PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
							PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);
							return;
						}
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read flds output",read_out_flistp);
						priority = PIN_FLIST_FLD_GET(read_out_flistp,PIN_FLD_PRIORITY , 0, ebufp);
						if ( priority && pbo_decimal_compare(priority, thousand, ebufp) >= 0)
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "priority>1000 ");
							tmp_str = pbo_decimal_to_str(priority, ebufp);
							prty = atoi(tmp_str);
							mod_prty = prty%1000;
							if(mod_prty >BB_PREPAID_START && mod_prty <=BB_PREPAID_END)
							{
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "stop_change_refund=1 ");
								stop_change_refund = 1;
							}

						}

					}

				}

			}
		}


	}
	if(hold_svc_found || (topup_found && cancel_found) || stop_change_refund)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "This is stop refund scenario.");
		e_flistp = PIN_FLIST_SUBSTR_GET(i_flistp,PIN_FLD_EVENT,1, ebufp);
		if(e_flistp)
		   e_pdp = PIN_FLIST_FLD_GET(e_flistp, PIN_FLD_POID, 1, ebufp);
		if(e_pdp)
		{
			evt_type = (char *)PIN_POID_GET_TYPE(e_pdp);
			if((evt_type) && (strstr(evt_type, "/event/billing/product/fee/cycle/cycle_forward")) )
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "cycle_forward event");
				cycle_info = PIN_FLIST_SUBSTR_GET(e_flistp,PIN_FLD_CYCLE_INFO,1, ebufp);
				if(cycle_info)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Cycle info found.");
					cycle_fee_start = PIN_FLIST_FLD_GET(cycle_info, PIN_FLD_CYCLE_START_T, 0, ebufp);
					current_t = pin_virtual_time(NULL);
					if(cycle_fee_start && *cycle_fee_start > current_t)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Cycle fee start in future.");
						//PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_FLAGS, (void *)&flags, ebufp);
						//PIN_FLIST_FLD_SET(cycle_info, PIN_FLD_FLAGS, (void *)&flags, ebufp);
						update_req = 1;
					}
				}
			}
		}

		if(update_req) 
		{
			e_flistp = PIN_FLIST_SUBSTR_GET(*r_flistpp,PIN_FLD_EVENT,1, ebufp);
			if(e_flistp)
			{
				cycle_info = PIN_FLIST_SUBSTR_GET(e_flistp,PIN_FLD_CYCLE_INFO,1, ebufp);
				if(cycle_info)
				{
					if(hold_svc_found)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Updating flag for hold.");
						PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_FLAGS, (void *)&hold_flags, ebufp);
						PIN_FLIST_FLD_SET(cycle_info, PIN_FLD_FLAGS, (void *)&hold_flags, ebufp);
					}
					else
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Updating flag for topup-cancel.");
						PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_FLAGS, (void *)&topup_flags, ebufp);
						PIN_FLIST_FLD_SET(cycle_info, PIN_FLD_FLAGS, (void *)&topup_flags, ebufp);
					}
				}
			}
		}
	}
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_rate_pol_update_flag_for_hold error", ebufp);
        }
        return;
}
