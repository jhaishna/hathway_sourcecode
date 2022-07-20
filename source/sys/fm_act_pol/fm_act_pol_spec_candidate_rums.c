/*
 *      Copyright (c) 1999 - 2006 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 */

#ifndef lint
static  char    Sccs_id[] = "@(#)%Portal Version: fm_act_pol_spec_candidate_rums.c:BillingVelocityInt:6:2006-Sep-08 04:12:20 %";
#endif

#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <pbo_decimal.h>

#include "pcm.h"
#include "ops/act.h"
#include "pin_act.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "pin_term.h"
#include "pin_rate.h"

#define FILE_LOGNAME "fm_act_pol_spec_candidate_rums.c(14)"
#define PIN_ACT_POL_DROPPED_CALL_RUM_NAME "Dropped_Call_Quantity"

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_act_pol_spec_candidate_rums(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**r_flistpp,
        pin_errbuf_t		*ebufp);

void
fm_act_pol_handle_dropped_calls(
	pin_flist_t             *i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t            *ebufp);

int32
fm_act_pol_populate_dropped_call_quantity(
	pin_flist_t	*i_flistp,
	pin_flist_t	*candidate_rum_flistp,
	pin_errbuf_t	*ebufp);

/*******************************************************************
 * Routines needed from elsewhere.
 *******************************************************************/

/*******************************************************************
 * Main routine for the PCM_OP_ACT_POL_SPEC_CANDIDATE_RUMS operation.
 *******************************************************************/
void
op_act_pol_spec_candidate_rums(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**r_flistpp,
        pin_errbuf_t		*ebufp)
{
	poid_t			*pdp = NULL;
	pin_flist_t		*b_flistp = NULL;
	pin_flist_t		*c_flistp = NULL;

	pin_cookie_t		cookie = NULL;
	int32			element_id;	
	char 			*rum_name = NULL;
	pin_decimal_t		*qtyp = NULL;
	poid_t			*e_pdp = NULL;
	pin_flist_t		*e_flistp = NULL;
	pin_flist_t		*d_flistp = NULL;
	pin_term_status_t	*d_statusp = NULL;
	pin_flist_t		*cr_flistp = NULL;
	char			buffer[50];
	pin_rate_unit_t		unit;
	const char		*tmp_poid_type = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/*******************************************************************
	 * Insanity check.
 	 *******************************************************************/
	if (opcode != PCM_OP_ACT_POL_SPEC_CANDIDATE_RUMS) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_act_pol_spec_candidate_rums bad opcode error", 
			ebufp);
		return;
	}
	*r_flistpp = PIN_FLIST_CREATE(ebufp); 

	/*******************************************************************
	 * Debug: What we got.
 	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_act_pol_spec_candidate_rums input flist", i_flistp);


	/*********************************************************************
	 * Copy the PIN_FLD_POID and the PIN_FLD_CANDIDATE_RUMS array to output
	 * flist. 
	 * Change the code below to add your own candidate RUM. If the 
	 * candidate RUM you are adding has the same name with any one in the 
	 * input flist, drop the one from the input flist to ensure that the 
	 * RUM names are unique. 
	 *********************************************************************/
	pdp = (poid_t *) PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);	
	PIN_FLIST_FLD_SET ( *r_flistpp, PIN_FLD_POID, (void*) pdp, ebufp );

	e_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_EVENT,0, ebufp);
	e_pdp = (poid_t *) PIN_FLIST_FLD_GET(e_flistp, PIN_FLD_POID,
							 0, ebufp);	

	while ((b_flistp = PIN_FLIST_ELEM_GET_NEXT( i_flistp, 
		PIN_FLD_CANDIDATE_RUMS, &element_id, 1, &cookie, ebufp)) 
			!= (pin_flist_t *) NULL) {
		
		PIN_FLIST_ELEM_SET( *r_flistpp, b_flistp, 
			PIN_FLD_CANDIDATE_RUMS,element_id, ebufp );
	}
	
	
	/*
	 * Code for supporting Orphan sessions for dialup events. 
	 * When a session is made an orphan it must not be rated. 
	 * This is achieved by adding a RUM with a name 'None' and
	 * passing this to the rating engine. 
	 * The rating engine then does not rate this event/create a 
	 * a balance impact.
	 */

	tmp_poid_type = PIN_POID_GET_TYPE(e_pdp);
	if (tmp_poid_type && !strcmp("/event/session/dialup", tmp_poid_type)) {
		d_flistp = PIN_FLIST_SUBSTR_GET(e_flistp, PIN_FLD_DIALUP,0, ebufp);
		d_statusp = PIN_FLIST_FLD_GET(d_flistp, PIN_FLD_STATUS, 1, ebufp);
		if (d_statusp != (void *)NULL && 
		    *d_statusp == PIN_TERM_STATUS_ORPHAN) {
			/* create a new candidate RUM */
			cr_flistp = PIN_FLIST_ELEM_ADD( *r_flistpp, 
					PIN_FLD_CANDIDATE_RUMS,++element_id, ebufp);
			/* set RUM name to 'None' */
			strcpy (buffer, "None");
			PIN_FLIST_FLD_SET(cr_flistp, PIN_FLD_RUM_NAME, 
						(void*)buffer, ebufp); 
			/* set quantity to 1 */
			qtyp = pbo_decimal_from_str("1", ebufp);
			PIN_FLIST_FLD_SET(cr_flistp, PIN_FLD_QUANTITY, 
						(void *)qtyp, ebufp);
			pbo_decimal_destroy(&qtyp);
			/* set unit to NONE */
			unit = PIN_RATE_UNIT_NONE;
			PIN_FLIST_FLD_SET(cr_flistp, PIN_FLD_UNIT,
						(void*)&unit, ebufp);
		}
	}

	/*
	 * Handle the dropped calls RUM for Telco events.
	 * In dropped call scenario, Continuation call will be rated
	 * along with the Dropped call quantity (rated quantity of the
	 * actual call that is dropped) by leveraging the Multi-RUM
	 * capabilities.
	 * Set the Dropped call quantity in the CANDIDATE_RUMS for
	 * the Dropped call RUM.
	 */
	if (fm_utils_is_subtype(e_pdp, "/event/session/telco")) {

		fm_act_pol_handle_dropped_calls(i_flistp, r_flistpp, ebufp);
	}

	/*********************************************************************
	 * Do not change the below lines. 
	 *********************************************************************/	
	
	/*********************************************************************
	 * Error checking 
	 *********************************************************************/
	if ( PIN_ERR_IS_ERR(ebufp)) {
		PIN_FLIST_DESTROY_EX(r_flistpp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"pcm_op_act_pol_spec_candidate_rums error", ebufp);
	} else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
			"pcm_op_act_pol_spec_candidate_rums return flist", 
			*r_flistpp);
	}
	return; 	
}

/*******************************************************************
 * fm_act_pol_handle_dropped_calls()
 *
 * Routine used to handle the dropped calls.
 *
 * Populates the dropped call RUM quantity in Candidate RUMS array
 * matching the Dropped call RUM name. Quantity is taken from
 * /event.DROPPED_CALL_QUANTITY incase of real time or 
 * event.RUM_MAP.NET_QUANTITY incase of re-rating flow.
 *******************************************************************/
void
fm_act_pol_handle_dropped_calls(
	pin_flist_t             *i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t            *ebufp)
{

	pin_cookie_t	cookie 				= NULL;
	int32		elem_id				= 0;
	pin_flist_t	*tmp_flistp 			= NULL;
	char 		*rum_name 			= NULL;
	int32		dropped_call_qty_populated	= PIN_BOOLEAN_FALSE;


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_act_pol_handle_dropped_calls input flist", 
		i_flistp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_act_pol_handle_dropped_calls Initial return flist", 
		*r_flistpp);

	while ((tmp_flistp = PIN_FLIST_ELEM_GET_NEXT(*r_flistpp,
			PIN_FLD_CANDIDATE_RUMS, &elem_id, 1,
			&cookie, ebufp)) != NULL) {


		rum_name = (char*) PIN_FLIST_FLD_GET(tmp_flistp,
				PIN_FLD_RUM_NAME, 0, ebufp);

		if (rum_name &&
			(strcmp(rum_name, PIN_ACT_POL_DROPPED_CALL_RUM_NAME) == 0)) {

			/* Populate the candidate rums quantity taken
			 * from /event.DROPPED_CALL_QUANTITY incase of Real time
			 * and event.RUM_MAP.QUANTITY incase of re-rating.
			 */
			dropped_call_qty_populated = fm_act_pol_populate_dropped_call_quantity(
							i_flistp, tmp_flistp, ebufp);

			/* If we didnt populate dropped call quantity then no need to send
			 * Candidate rums for Dropped call RUM name to Rating.
			 */
			if (dropped_call_qty_populated == PIN_BOOLEAN_FALSE) {
				PIN_FLIST_ELEM_DROP(*r_flistpp, PIN_FLD_CANDIDATE_RUMS,
					elem_id, ebufp);
				cookie = NULL;
			}

			break;
		}
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_act_pol_handle_dropped_calls return flist", 
		*r_flistpp);

	/***********************************************************
	* Error?
	***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_act_pol_handle_dropped_calls error", ebufp);
	}
	return;

}

/*******************************************************************
 * fm_act_pol_populate_dropped_call_quantity()
 *
 * Routine used to populate the dropped call quantity.
 *
 * Quantity is taken from /event.DROPPED_CALL_QUANTITY incase of
 * real time or event.RUM_MAP.NET_QUANTITY incase of
 * re-rating flow.
 *******************************************************************/
int32
fm_act_pol_populate_dropped_call_quantity(
	pin_flist_t	*i_flistp,
	pin_flist_t	*candidate_rums_flistp,
	pin_errbuf_t	*ebufp)
{

	pin_flist_t	*event_flistp 		= NULL;
	pin_flist_t	*rum_map_flistp 	= NULL;
	pin_cookie_t	cookie			= NULL;
	int32		elem_id			= 0;
	int32		rerate_flags		= 0;
	int32		dropped_call_qty_found	= PIN_BOOLEAN_FALSE;
	void		*vp 			= NULL;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_act_pol_populate_dropped_call_quantity "
		"Input flist", i_flistp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_act_pol_populate_dropped_call_quantity "
		"Initial candidate rums flist", candidate_rums_flistp);

	event_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_EVENT,
					0, ebufp);

	/*
	 * Adjust the candidate rums quantity as per the real time or re-rating flow.
	 * If PIN_FLD_FLAGS contains PIN_RATE_FLG_RERATE
	 *    read the EVENT.RUM_MAP.NET_QUANTITY matching the dropped call RUM
	 *    and popuate the value inside CANDIDATE_RUMS.QUANTITY.
	 * else it is a normal event
	 *    read the EVENT.DROPPED_CALL_QUANTITY and populate as CANDIDATE_RUMS.QUANTITY.
	 */
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp);
	if (vp != NULL) {
		rerate_flags = *(int32*)vp;
	}

	if (rerate_flags & PIN_RATE_FLG_RERATE) {

		/* Parse through the RUM_MAP array to find out the quantity
		 * for the Dropped call RUM.
		 */
		while ((rum_map_flistp = PIN_FLIST_ELEM_GET_NEXT(
				event_flistp, PIN_FLD_RUM_MAP, &elem_id,
				1, &cookie, ebufp)) != NULL) {

			vp = PIN_FLIST_FLD_GET(rum_map_flistp, PIN_FLD_RUM_NAME, 0, ebufp);
			if (vp != NULL &&
				(strcmp((char*)vp, PIN_ACT_POL_DROPPED_CALL_RUM_NAME) == 0)) {

				/* Get the Dropped call quantity. */
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"fm_act_pol_get_dropped_call_quantity() "
					"Found Dropped call quantity from RUM_MAP.NET_QUANTITY",
					rum_map_flistp);

				vp = PIN_FLIST_FLD_GET(rum_map_flistp,
						PIN_FLD_NET_QUANTITY, 0, ebufp);

				PIN_FLIST_FLD_SET(candidate_rums_flistp,
						PIN_FLD_QUANTITY, vp, ebufp);

				dropped_call_qty_found = PIN_BOOLEAN_TRUE;

				break;
			}
		}
	}
	else {

		/* Get the Dropped call quantity in event, and set to candidate rums quantity.
		 */
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_act_pol_get_dropped_call_quantity() "
			"Reading Dropped call quantity in event");

		vp = PIN_FLIST_FLD_GET(event_flistp, PIN_FLD_DROPPED_CALL_QUANTITY,
					1, ebufp);

		if (vp != NULL) {

			PIN_FLIST_FLD_SET(candidate_rums_flistp, PIN_FLD_QUANTITY, vp, ebufp);
			dropped_call_qty_found = PIN_BOOLEAN_TRUE;
		}

	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_act_pol_populate_dropped_call_quantity "
		"return candidate rums flist", candidate_rums_flistp);

	/***********************************************************
	* Error?
	***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_act_pol_populate_dropped_call_quantity error", ebufp);
	}

	return dropped_call_qty_found;
}
