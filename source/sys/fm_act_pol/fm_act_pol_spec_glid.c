/*******************************************************************
 *
* Copyright (c) 1999, 2019, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char    Sccs_id[] = "@(#)$Id: fm_act_pol_spec_glid.c /cgbubrm_mainbrm.portalbase/1 2019/02/14 22:51:15 agangrad Exp $";
#endif

/*******************************************************************
 * Contains the PCM_OP_ACT_POL_SPEC_GLID operation. 
 *******************************************************************/

#include <stdio.h> 
#include <string.h> 
 
#include "pcm.h"
#include "ops/act.h"
#include "pin_act.h"
#include "pin_cust.h"
#include "pin_rate.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "fm_bill_utils.h"
#include "pin_bill.h"
#include "pin_pymt.h"

//MSO Customization Starts
#include "mso_ops_flds.h"
#include "mso_glid.h"

#define MSO_ET_MAIN "MSO_ET_MAIN"
#define MSO_ET_ADDI "MSO_ET_ADDI"
#define MSO_VAT "MSO_VAT"
#define SERVICE_TAX "Service_Tax"
#define MSO_SERVICE_TAX "MSO_Service_Tax"
#define EDUCATION_CESS "Education_Cess"
#define SHE_CESS "Secondary_Higher_Education_Cess"
#define SERVICE_TYPE_BB "/service/telco/broadband"
#define SERVICE_TYPE_CATV "/service/catv"
#define CGST "CGST"
#define SGST "SGST"
#define IGST "IGST"

#define DEB_NOTE_EVENT "/event/billing/mso_cr_dr_note"
#define LATE_FEE_EVENT "/event/billing/late_fee"

//MSO Customization Ends
/*******************************************************************
 * Prototypes.
 *******************************************************************/
static void
fm_act_pol_spec_glid (
        cm_nap_connection_t	*connp,
        pin_flist_t		*i_flistp,
	const char			*event_type,
        pin_flist_t		*s_flistp,
        pin_errbuf_t		*ebufp);

void
fm_act_pol_add_event_bal_impacts (
	pcm_context_t		*ctxp,
        pin_flist_t		*e_flistp,
        pin_errbuf_t		*ebufp);

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_act_pol_spec_glid(
        cm_nap_connection_t	*connp,
	int			opcode,
        int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**r_flistpp,
        pin_errbuf_t		*ebufp);


/*******************************************************************
 * Routines needed from elsewhere.
 *******************************************************************/
PIN_IMPORT void fm_mso_utils_get_service_from_balgrp(
	pcm_context_t		*ctxp,
	poid_t			*balgrp_pdp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_ACT_POL_SPEC_GLID operation.
 *******************************************************************/
void
op_act_pol_spec_glid(
        cm_nap_connection_t	*connp,
	int			opcode,
        int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**r_flistpp,
        pin_errbuf_t		*ebufp)
{
	pin_cookie_t		cookie = NULL;
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*e_flistp = NULL;
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	const char			*event_type = NULL;
	poid_t			*e_pdp = NULL;
	poid_t			*i_pdp = NULL;

	int			rec_id = 0;
//	int32		*glid_orig = NULL;
	int			*glid = NULL;
	int			t_glid = 0;
	char		*taxcode = NULL;
	char		*descr = NULL;
	char		*name = NULL;
	char		*item_type = NULL;
	int32		no_tax_flag = 1;

	poid_t		*balgrp_pdp = NULL;
	poid_t		*srvice_pdp = NULL;
	char		*service_type = NULL;
	pin_flist_t	*service_flistp = NULL;
	pin_flist_t	*rslt_flistp = NULL;
	char		*bb_serv_tax = MSO_SERVICE_TAX;

	/*CHIDHU: Variable declaration*/
	int             tax_flags=1;
	/*CHIDHU: Variable declaration*/

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);
	*r_flistpp = (pin_flist_t *)NULL;

	/*******************************************************************
	 * Insanity check.
 	 *******************************************************************/
	if (opcode != PCM_OP_ACT_POL_SPEC_GLID) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_act_pol_spec_glid bad opcode error", ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: What we got.
 	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_act_pol_spec_glid input flist", i_flistp);

	/*******************************************************************
	 * Our action will depend on the type of event
	 * Get the event substruct to get all the needed data
 	 *******************************************************************/
	*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
	e_flistp = PIN_FLIST_SUBSTR_GET(*r_flistpp, PIN_FLD_EVENT, 0, ebufp);
	e_pdp = (poid_t *)PIN_FLIST_FLD_GET(e_flistp, PIN_FLD_POID, 0, ebufp);
	event_type = PIN_POID_GET_TYPE(e_pdp);

		/*Ignore tax for balance transfer-START */
        if (event_type && (strcmp(event_type, "/event/billing/adjustment/account") == 0) &&
                (fm_utils_op_is_parent(connp->coip,
                        PCM_OP_BILL_TRANSFER_BALANCE))) {
            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "op_act_pol_spec_glid parent input flist", i_flistp);
            PIN_FLIST_FLD_DROP(e_flistp,PIN_FLD_TAX_SUPPLIER,ebufp);
            PIN_FLIST_FLD_DROP(e_flistp,PIN_FLD_FLAGS,ebufp);
            PIN_FLIST_FLD_SET(e_flistp,PIN_FLD_FLAGS,&tax_flags,ebufp);
	    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Removing the PIN_FLD_TAX_SUPPLIER and FLAGS from the input to ignore tax");
        }
        /*Ignore tax for balance transfer-END */
	/******************************************************************
	 * Check PIN_FLD_RATES_USED array, add a dummy one if not.
	 ******************************************************************/
	if (PIN_FLIST_ELEM_COUNT(e_flistp, PIN_FLD_BAL_IMPACTS, ebufp) == 0) {

		/**********************************************************
		 * Add the bal impacts array for this pre-rates event.
 	 	 **********************************************************/
		fm_act_pol_add_event_bal_impacts(ctxp, e_flistp, ebufp);
	}

	//MSO Customization Starts
	// Added for populating the GLID for Min. Commitment Adjustment event
	descr = (char *)PIN_FLIST_FLD_GET(e_flistp, PIN_FLD_DESCR, 1, ebufp);

	//MSO Customization Ends
	/******************************************************************
	 * Now walk the PIN_FLD_RATES_USED array and check for G/L ID. 
	 ******************************************************************/
	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(e_flistp, PIN_FLD_BAL_IMPACTS, 
		&rec_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) {

		/**********************************************************
	 	 * G/L id is inside the PIN_FLD_SUBTOTAL array.
 	 	 **********************************************************/
//		glid_orig = (void *)PIN_FLIST_FLD_GET(flistp, PIN_FLD_GL_ID, 1, ebufp);
//		glid = (int )*glid_orig;
		glid = (int *)PIN_FLIST_FLD_GET(flistp, PIN_FLD_GL_ID, 1, ebufp);
		i_pdp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_ITEM_OBJ, 1, ebufp);
		item_type = (char *)PIN_POID_GET_TYPE(i_pdp);

		/* Get the balance group poid to identify type of serice BB/CATV */
		balgrp_pdp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_BAL_GRP_OBJ, 1, ebufp);
		if(balgrp_pdp != NULL)
		{
			fm_mso_utils_get_service_from_balgrp(ctxp, balgrp_pdp, &service_flistp, ebufp);
			if (PIN_ERR_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"Error while getting service details error", ebufp);
				PIN_FLIST_DESTROY_EX(&service_flistp, NULL);
				return;
			}
			rslt_flistp = PIN_FLIST_ELEM_GET(service_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
			if(rslt_flistp != NULL)
			{
				srvice_pdp = PIN_FLIST_FLD_GET(rslt_flistp, PIN_FLD_POID, 0, ebufp);
				service_type = (char *)PIN_POID_GET_TYPE(srvice_pdp);
			}
		}
		

		/**********************************************************
		 * We need to fill in the G/L id now.
 	 	 **********************************************************/
		if (glid == (int *)NULL || *glid == 0) {
			//MSO Customization Starts
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"GLID is NULL!!!");
			taxcode = (char *)PIN_FLIST_FLD_GET(flistp, PIN_FLD_TAX_CODE, 1, ebufp);

			if ( (fm_utils_op_is_ancestor(connp->coip, MSO_OP_PYMT_COLLECT) ||
				fm_utils_op_is_ancestor(connp->coip, MSO_OP_PYMT_REVERSE_PAYMENT)) &&
				fm_utils_op_is_ancestor(connp->coip, PCM_OP_BILL_TRANSFER_BALANCE))
			{
				PIN_FLIST_FLD_SET(flistp, PIN_FLD_TAX_CODE, "", ebufp);
				PIN_FLIST_FLD_SET(e_flistp, PIN_FLD_FLAGS, &no_tax_flag, ebufp);
			}

			if(service_type != NULL && (strcmp(service_type, SERVICE_TYPE_CATV)==0) )
			{
				if (strcmp(taxcode, MSO_ET_MAIN)==0 || strcmp(taxcode, MSO_ET_ADDI)==0)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"ET Taxcode Matched, assigning ET GLID");
					t_glid = GLID_ET;
					PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp);
				}
                                else if (strcmp(taxcode, IGST)==0)
                                {
                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"IGST Taxcode Matched, assigning IGST GLID");
                                        t_glid = GLID_IGST;
                                        PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp);
                                }
                                else if (strcmp(taxcode, CGST)==0)
                                {
                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"CGST Taxcode Matched, assigning CGST GLID");
                                        t_glid = GLID_CGST;
                                        PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp);
                                }
                                else if (strcmp(taxcode, SGST)==0)
                                {
                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"SGST Taxcode Matched, assigning SGST GLID");
                                        t_glid = GLID_SGST;
                                        PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp);
                                }
				else if (strcmp(taxcode, MSO_VAT)==0)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"VAT Taxcode Matched, assigning VAT GLID");
					t_glid = GLID_VAT;
					PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp);
				}
				else if (strcmp(taxcode, SERVICE_TAX)==0)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"ST Taxcode Matched, assigning ST GLID");
					t_glid = GLID_ST;
					PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp);
				}
				else if (strcmp(taxcode, EDUCATION_CESS)==0)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"ECess Taxcode Matched, assigning ECess GLID");
					t_glid = GLID_ECess;
					PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp);
				}
				else if (strcmp(taxcode, SHE_CESS)==0)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"SHECess Taxcode Matched, assigning SHECess GLID");
					t_glid = GLID_SHECess;
					PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp);
				}
				else if (descr && (strcmp(descr,"Min. Bill Amount Adjustment")==0))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Minimum Commitment Adjustment Matched, assigning GLID");
					t_glid = GLID_MIN_COMMIT;
					PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp); 
				}
				else if (descr && (strcmp(descr,"Payment Bounce Fine Amount")==0))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Payment Bounce Adjustment Matched, assigning GLID");
					t_glid = GLID_Cheque_Bounce;
					PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp);
				}
				else if (item_type && (strcmp(item_type,"/item/payment")==0))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Payment Matched, assigning GLID");
					t_glid = GLID_Payment;
					PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp);
				}
				else if (item_type && (strcmp(item_type,"/item/payment/reversal")==0))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Payment Reversal Matched, assigning GLID");
					t_glid = GLID_Payment;
					PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp);
				}
				else if (item_type && (strcmp(item_type,"/item/adjustment")==0))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Adjustment Matched, assigning GLID");
					t_glid = GLID_Adjustment;
					PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp);
				}
				else if (item_type && (strcmp(item_type,"/item/dispute")==0))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Dispute Matched, assigning GLID");
					t_glid = GLID_Dispute;
					PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp);
				}
				else if (item_type && (strcmp(item_type,"/item/settlement")==0))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Settlement Matched, assigning GLID");
					t_glid = GLID_Settlement;
					PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp);
				}
				else 
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"No Taxcodes Matched, assigning default GLID");
					//MSO Customization Ends with 1 more end curly braces
					fm_act_pol_spec_glid(connp, i_flistp, 
						event_type, flistp, ebufp);
				}
			} /* CATV End*/
			else if(service_type !=NULL && (strcmp(service_type, SERVICE_TYPE_BB)==0) )
			{
				if (strcmp(taxcode, MSO_ET_MAIN)==0 || strcmp(taxcode, MSO_ET_ADDI)==0)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"ET Taxcode Matched, assigning ET GLID");
					t_glid = GLID_BB_ET;
					PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp);
				}
                                else if (strcmp(taxcode, IGST)==0)
                                {
                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"IGST Taxcode Matched, assigning IGST GLID");
                                        t_glid = GLID_BB_IGST;
                                        PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp);
                                }
                                else if (strcmp(taxcode, CGST)==0)
                                {
                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"CGST Taxcode Matched, assigning CGST GLID");
                                        t_glid = GLID_BB_CGST;
                                        PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp);
                                }
                                else if (strcmp(taxcode, SGST)==0)
                                {
                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"SGST Taxcode Matched, assigning SGST GLID");
                                        t_glid = GLID_BB_SGST;
                                        PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp);
                                }
				else if (strcmp(taxcode, MSO_VAT)==0)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"VAT Taxcode Matched, assigning VAT GLID");
					t_glid = GLID_BB_VAT;
					PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp);
				}
				else if (strcmp(taxcode, SERVICE_TAX)==0)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"ST Taxcode Matched, assigning ST GLID");
					t_glid = GLID_BB_ST;
					PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp);
				}
				else if (strcmp(taxcode, EDUCATION_CESS)==0)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"ECess Taxcode Matched, assigning ECess GLID");
					t_glid = GLID_BB_ECess;
					PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp);
				}
				else if (strcmp(taxcode, SHE_CESS)==0)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"SHECess Taxcode Matched, assigning SHECess GLID");
					t_glid = GLID_BB_SHECess;
					PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp);
				}
				if (descr && (strcmp(descr,"Payment Bounce Fine Amount")==0))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Payment Bounce Adjustment Matched, assigning GLID");
					t_glid = GLID_BB_Cheque_Bounce;
					PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp);
				}
				else if (item_type && (strcmp(item_type,"/item/payment")==0))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Payment Matched, assigning GLID");
					t_glid = GLID_BB_Payment;
					PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp);
				}
				else if (item_type && (strcmp(item_type,"/item/payment/reversal")==0))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Payment Reversal Matched, assigning GLID");
					t_glid = GLID_BB_Payment;
					PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp);
				}
				else if (item_type && (strcmp(item_type,"/item/adjustment")==0))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Adjustment Matched, assigning GLID");
					t_glid = GLID_BB_Adjustment;
					PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp);
				}
				else if (item_type && (strcmp(item_type,"/item/dispute")==0))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Dispute Matched, assigning GLID");
					t_glid = GLID_BB_Dispute;
					PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp);
				}
				else if (item_type && (strcmp(item_type,"/item/settlement")==0))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Settlement Matched, assigning GLID");
					t_glid = GLID_BB_Settlement;
					PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp);
				}
				else
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Assigning default GLID");
					fm_act_pol_spec_glid(connp, i_flistp, event_type, flistp, ebufp);
				}

			} /* BB End */
			else
			{
				name = PIN_FLIST_FLD_GET(e_flistp, PIN_FLD_NAME, 1, ebufp);
				if (descr && (strcmp(descr,"Min. Bill Amount Adjustment")==0))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Minimum Commitment Adjustment Matched, assigning GLID");
					t_glid = GLID_MIN_COMMIT;
					PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp); 
				}
				else if (name && (strcmp(name, "BB_CQ_BOUNCE_PENALTY")==0))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Check bounce penalty for broadband, assigning GLID");
					t_glid = GLID_BB_Cheque_Bounce;
					PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp); 
				}
				else if (name && (strcmp(name, "CATV_CQ_BOUNCE_PENALTY")==0))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Check bounce penalty for CATV, assigning GLID");
					t_glid = GLID_Cheque_Bounce;
					PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp); 
				}
				else if (name && (strcmp(name, "BB_CR_DR_NOTE")==0))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Debit / Credit Note for broadband, assigning GLID");
					t_glid = GLID_BB_DebitNote;
					PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp); 
				}
				else if (name && (strcmp(name, "CATV_CR_DR_NOTE")==0))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Debit / Credit Note for CATV, assigning GLID");
					t_glid = GLID_DebitNote;
					PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp); 
				}
				else if (name && (strcmp(name, "BB_Custom_Late_Fee")==0))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Collection Late Fee for broadband, assigning GLID");
					t_glid = GLID_BB_Late_Payment;
					PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp); 
				//	PIN_FLIST_FLD_SET(flistp, PIN_FLD_TAX_CODE, bb_serv_tax, ebufp); 
				}
				else if (name && (strcmp(name, "CATV_Custom_Late_Fee")==0))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Collection Late Fee for CATV, assigning GLID");
					t_glid = GLID_Late_Payment;
					PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp); 
				}
				else if (name && ((strcmp(name, "mso_pymt_deposit_refund")==0) 
					|| (strcmp(name, "mso_pymt_reverse_refund")==0)))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
					"Broadband Deposit Refund reversal, assigning GLID");
					t_glid = GLID_BB_REV_REFUND;
					PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp); 
				}
				else if (name && (strcmp(name, "mso_pymt_process_refund")==0))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Broadband Deposit Refund, assigning GLID");
					t_glid = GLID_BB_REFUND;
					PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, &t_glid, ebufp); 
				}
				else
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Assigning default GLID");
					fm_act_pol_spec_glid(connp, i_flistp, event_type, flistp, ebufp);
				}
			}
		}
	}

	/*******************************************************************
	 * Error?
 	 *******************************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_act_pol_spec_glid error", ebufp);
	} else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_act_pol_spec_glid return flist", *r_flistpp);
	}
	PIN_FLIST_DESTROY_EX(&service_flistp, NULL);

	return;
}

/*******************************************************************
 * fm_act_pol_spec_glid
 * Routine that assigns the G/L id for pre/partially rated events.
 *******************************************************************/
static void
fm_act_pol_spec_glid (
        cm_nap_connection_t	*connp,
        pin_flist_t		*i_flistp,
	const char			*event_type,
        pin_flist_t		*s_flistp,
        pin_errbuf_t		*ebufp)
{
	int			glid = 0;
        pcm_context_t           *ctxp = connp->dm_ctx;
	pin_flist_t		*p_flistp = NULL;
        pin_flist_t		*e_flistp = NULL;
	int32			*cmdp = NULL;

	/*******************************************************************
	 * get glid from /config/map_glid based on event_type and
	 * string version and string id of the Debit/Credit account
	 * adjustment reason code, if any.
 	 *******************************************************************/

	e_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_EVENT,
			0, ebufp);

	glid = fm_utils_get_glid(ctxp, e_flistp, ebufp);

	if (!strstr(event_type, "/event/billing/payment") &&
		!strstr(event_type, "/event/billing/refund")) {

		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_GL_ID, 
				(void *)&glid, ebufp);
	} else {
	
		/**********************************************************
		 * Check the command to see if it was PIN_CHARGE_CMD_REFUND
		 **********************************************************/
		if (strstr(event_type, "/event/billing/payment")) {
			p_flistp = PIN_FLIST_SUBSTR_GET(e_flistp, 
				PIN_FLD_PAYMENT, 0, ebufp);
		} else {
			p_flistp = PIN_FLIST_SUBSTR_GET(e_flistp, 
				PIN_FLD_REFUND, 0, ebufp);
		}
		cmdp = (int32 *)PIN_FLIST_FLD_GET(p_flistp,
				PIN_FLD_COMMAND, 0, ebufp);

		if (PIN_ERR_IS_ERR(ebufp)) {
			 PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_act_pol_spec_glid error - no command", 
				ebufp);
			return;
		}

		if (cmdp && *cmdp == PIN_CHARGE_CMD_REFUND) {
			/***********************************************
			 * Important - This is where a GLID for the 
			 * refund should be specified, it is set to a
			 * default of the payment GLID.
			 ***********************************************/
			PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_GL_ID, 
					(void *)&glid, ebufp);
		} else {
			PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_GL_ID,
					(void *)&glid, ebufp);
		}
	}
	return;
}

/*******************************************************************
 * fm_act_pol_add_event_bal_impacts():
 *******************************************************************/
void
fm_act_pol_add_event_bal_impacts(
	pcm_context_t		*ctxp,
	pin_flist_t		*a_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_cookie_t		cookie = NULL;
	pin_flist_t		*bi_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	pin_flist_t		*bia_flistp = NULL;
	poid_t			*r_pdp = NULL;
	void			*vp = NULL;

	int			impact_type = 0;
	int			count = 0;
	int			rec_id = 0;
	pin_decimal_t 		*dummyp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	dummyp = pbo_decimal_from_str("0.0", ebufp);
	/***********************************************************
	 * Set the balance impact type as pre-rated.
	 ***********************************************************/
	impact_type = PIN_IMPACT_TYPE_PRERATED;

	bia_flistp = PIN_FLIST_CREATE(ebufp);

	/***********************************************************
	 * Walk the PIN_FLD_TOTALS array in the original event and
	 * add a corresponding balance impact arary for that.
	 ***********************************************************/
	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(a_flistp, PIN_FLD_TOTAL, 
		&rec_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) {

		/***************************************************
	 	 * Add a new element for this balance impact.
		 ***************************************************/
		count = (int)PIN_FLIST_ELEM_COUNT(bia_flistp, 
				PIN_FLD_BAL_IMPACTS, ebufp);
		bi_flistp = PIN_FLIST_ELEM_ADD(bia_flistp, 
				PIN_FLD_BAL_IMPACTS, count, ebufp);

		/***************************************************
	 	 * Start filling in the standard field values now.
		 ***************************************************/
		PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_IMPACT_TYPE,
				(void *)&impact_type, ebufp);
		PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_RESOURCE_ID,
				(void *)&rec_id, ebufp);
		PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_TAX_CODE, 
				(void *)"", ebufp);
		PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_RATE_TAG, 
				(void *)"", ebufp);

		/***************************************************
		 * Get and Set the account and item object poids.
		 ***************************************************/
		vp = PIN_FLIST_FLD_GET(a_flistp, 
				PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
		PIN_FLIST_FLD_SET(bi_flistp, 
				PIN_FLD_ACCOUNT_OBJ, vp, ebufp);

		vp = PIN_FLIST_FLD_GET(a_flistp, 
				PIN_FLD_ITEM_OBJ, 1, ebufp);
		if (vp) {
			PIN_FLIST_FLD_SET(bi_flistp, 
				PIN_FLD_ITEM_OBJ, vp, ebufp);
		}

		/***************************************************
		 * Fill in the dummy rate object poid.
		 ***************************************************/
		r_pdp = PIN_POID_CREATE(0, "", 0, ebufp);
		PIN_FLIST_FLD_PUT(bi_flistp, PIN_FLD_RATE_OBJ, 
				(void *)r_pdp, ebufp);

		/***************************************************
		 * set the 0 for quantity, discount and percentage
		 ***************************************************/
		PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_DISCOUNT, 
				(void *)dummyp, ebufp);
		PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_PERCENT, 
				(void *)dummyp, ebufp);
		PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_QUANTITY, 
				(void *)dummyp, ebufp);

		/***************************************************
		 * Set the amount deferred to 0.0
		 ***************************************************/
		PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_AMOUNT_DEFERRED, 
				(void *)dummyp, ebufp);

		/***************************************************
		 * Get the amount and set in the bal impact.
		 ***************************************************/
		vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_AMOUNT, 0, ebufp);
		PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_AMOUNT, vp, ebufp);

	}

	PIN_FLIST_CONCAT(a_flistp, bia_flistp, ebufp);

	PIN_FLIST_DESTROY_EX(&bia_flistp, NULL);
	pbo_decimal_destroy(&dummyp);
	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_act_pol_add_event_bal_impacts error", ebufp);
	}

	return;
}
