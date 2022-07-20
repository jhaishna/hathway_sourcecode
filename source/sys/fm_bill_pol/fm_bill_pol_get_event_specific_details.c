/*****************************************************************************
 *
* Copyright (c) 2007, 2010, Oracle and/or its affiliates. All rights reserved. 
 *
 *	This material is the confidential property of Oracle Corporation or
 *	its licensors and may be used, reproduced, stored or transmitted only
 *	in accordance with a valid Oracle license or sublicense agreement.
 *
 *****************************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)$Id: fm_bill_pol_get_event_specific_details.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 05:32:57 nishahan Exp $";
#endif


/*******************************************************************
 * This file contains the PCM_OP_BILL_POL_GET_EVENT_SPECIFIC_DETAILS 
 * search operation.Also included are subroutines specific to the operation.
 *******************************************************************/

#include <stdio.h>

#include "pcm.h"
#include "ops/bill.h"
#include "cm_fm.h"
#include "pin_bill.h"
#include "pin_cust.h"
#include "pin_errs.h"
#include "pinlog.h"

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void op_bill_pol_get_event_specific_details();
void fm_bill_pol_event_specific_search();

/********************************************************************************
 * Main routine for the PCM_OP_BILL_POL_GET_EVENT_SPECIFIC_DETAILS operation.
 *******************************************************************************/
void
op_bill_pol_get_event_specific_details(connp, opcode, flags, i_flistp, r_flistpp,
								ebufp)
        cm_nap_connection_t     *connp;
        int32                   opcode;
        int32                   flags;
        pin_flist_t             *i_flistp;
        pin_flist_t             **r_flistpp;
        pin_errbuf_t            *ebufp;
{
        pcm_context_t           *ctxp = connp->dm_ctx;

        if (PIN_ERR_IS_ERR(ebufp))
                return;
        PIN_ERR_CLEAR_ERR(ebufp);

        /***********************************************************
         * Insanity check.
         ***********************************************************/
         if (opcode != PCM_OP_BILL_POL_GET_EVENT_SPECIFIC_DETAILS) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_bill_pol_get_event_specific_details opcode error", 
						ebufp);
                return;
        }

        /***********************************************************
         * Debug: What we got.
         ***********************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "op_bill_pol_get_event_specific_details input flist", i_flistp);

        /***********************************************************
         * Main Sub-routine to search events
         ***********************************************************/
        fm_bill_pol_event_specific_search(ctxp, flags, i_flistp, r_flistpp, ebufp);

        /***********************************************************
         * Error?
         ***********************************************************/
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_bill_pol_get_event_specific_details error", ebufp);
        } else {
                /***************************************************
                 * Debug: What we're sending back.
                 ***************************************************/
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "op_bill_pol_get_event_specific_details return flist",
						*r_flistpp);
        }

        return;
}
/****************************************************************************
 * This function will fetch the event specific details like called Number etc.
 * based on the event type. This function can be customized further 
 * to retrieve event information for the different event types. 
 *******************************************************************/
void
fm_bill_pol_event_specific_search(ctxp, flags, i_flistp, r_flistpp, ebufp)
	pcm_context_t		*ctxp;
	u_int			flags;
	pin_flist_t		*i_flistp;
	pin_flist_t		**r_flistpp;
	pin_errbuf_t		*ebufp;
{
	void			*vp = NULL;
	pin_flist_t		*read_flistp = NULL;
	pin_flist_t		*rs_flistp = NULL;
	pin_flist_t		*t_flistp = NULL;
	pin_flist_t		*ei_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	const char		*event_poid_type = NULL;
	
	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	event_poid_type = PIN_POID_GET_TYPE((poid_t *) vp);

	r_flistp = PIN_FLIST_CREATE(ebufp); 
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, vp, ebufp);

	if(event_poid_type != NULL) {
		/* If telco event, get called_to */
		if ((strcmp(event_poid_type, "/event/activity/telco") == 0 ) ||
				(strcmp(event_poid_type,
				"/event/delayed/session/telco/gsm") == 0 ) ||
				(strcmp(event_poid_type,
				"/event/delayed/session/telco/gsm/roaming")
				== 0)) {

			read_flistp = PIN_FLIST_CREATE(ebufp);

			vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID,
					0, ebufp);
			PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_POID,
				vp, ebufp);
			t_flistp = PIN_FLIST_ELEM_ADD(read_flistp,
					PIN_FLD_TELCO_INFO,
					PIN_ELEMID_ANY, ebufp);
			PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_CALLED_TO,
				(void *)NULL, ebufp);

			PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flistp,
				&rs_flistp, ebufp);

			t_flistp = PIN_FLIST_ELEM_GET(rs_flistp, 
				PIN_FLD_TELCO_INFO, PIN_ELEMID_ANY, 0, ebufp);

			ei_flistp = PIN_FLIST_SUBSTR_ADD(r_flistp, 
				PIN_FLD_EXTENDED_INFO, ebufp);

			vp = PIN_FLIST_FLD_GET(t_flistp, PIN_FLD_CALLED_TO,
				0, ebufp);

			PIN_FLIST_FLD_SET(ei_flistp, PIN_FLD_CALLED_TO,
				vp, ebufp); 

			PIN_FLIST_DESTROY_EX(&read_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&rs_flistp, NULL);
		}
		/* If gprs event, get bytes_downlink , bytes_uplink */
		else if ((strcmp(event_poid_type,
				"/event/delayed/session/telco/gprs") == 0) ||
			 	(strcmp(event_poid_type,
				"/event/delayed/session/telco/gprs/roaming") == 0)) {

		   	read_flistp = PIN_FLIST_CREATE(ebufp);
			vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID,
					0, ebufp);
			PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_POID,
				vp, ebufp);
			t_flistp = PIN_FLIST_SUBSTR_ADD(read_flistp,
					PIN_FLD_TELCO_INFO, ebufp);
			PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_BYTES_DOWNLINK,
				NULL, ebufp);
			PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_BYTES_UPLINK,
				NULL, ebufp);

			PCM_OP(ctxp, PCM_OP_READ_FLDS,0, read_flistp,
				&rs_flistp, ebufp);

	  		t_flistp = PIN_FLIST_ELEM_GET(rs_flistp,
					PIN_FLD_TELCO_INFO, PIN_ELEMID_ANY,
					0, ebufp);
			ei_flistp = PIN_FLIST_SUBSTR_ADD(r_flistp, 
				PIN_FLD_EXTENDED_INFO, ebufp);

			vp = PIN_FLIST_FLD_GET(t_flistp, PIN_FLD_BYTES_DOWNLINK,
				0, ebufp);
			PIN_FLIST_FLD_SET(ei_flistp, PIN_FLD_BYTES_DOWNLINK, 
				vp, ebufp);
			vp = PIN_FLIST_FLD_GET(t_flistp, PIN_FLD_BYTES_UPLINK, 
					0, ebufp);
			PIN_FLIST_FLD_SET(ei_flistp, PIN_FLD_BYTES_UPLINK, 
				vp, ebufp);

			PIN_FLIST_DESTROY_EX(&read_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&rs_flistp, NULL);
		}
	}

	if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_bill_pol_event_specific_search error", ebufp);
	}

	*r_flistpp = r_flistp;
}
