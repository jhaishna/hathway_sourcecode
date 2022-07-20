/*
 *
 *      Copyright (c) 2006 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 *
 */

#ifndef lint
static  char Sccs_id[] = "@(#)%Portal Version: fm_bill_pol_get_item_tag.cpp:BillingVelocityInt:3:2006-Sep-05 21:51:28 %";
#endif

/*******************************************************************
 * Contains the PCM_OP_BILL_POL_GET_ITEM_TAG operation. 
 *******************************************************************/

#include "pcm.h"
#include "ops/bill.h"
#include "cm_fm.h"
#include "Pin.h"
#include "pin_bill.h"

#define FILE_SOURCE_ID  "fm_bill_pol_get_item_tag.cpp"
#define DEPOSIT_PRIORITY	"20.0"

#define DEPOSIT_TAG "mso_deposit"
#define MIN_COMMIT_TAG "mso_min_commitment"
#define MSO_CR_DR_NOTE_TAG "mso_cr_dr_note"
#define LATE_FEE_TAG "mso_late_fee"
#define CQ_BOUNCE_PENALTY_TAG "mso_penalty"

#define MIN_COMM_TYPE "/event/billing/mso_catv_commitment"
#define LATE_FEE_TYPE "/event/billing/late_fee"
#define MSO_CR_DR_NOTE "/event/billing/mso_cr_dr_note"
#define CFW_ET_TYPE "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_et"
#define CQ_BOUNCE_PENALTY_TYPE "/event/billing/mso_penalty"

#define MSO_ST "MSO_Service_Tax"
#define MSO_VAT "MSO_VAT"
#define MSO_VAT_ST "MSO_VAT_ST"
#define MSO_ET "MSO_ET"
#define EMPTY_TAX ""

#define TAG_Service_Tax "mso_st"
//#define TAG_EDU_CESS "mso_edu_cess"
//#define TAG_SHECESS "mso_shecess"
#define TAG_HW_Service_Tax "mso_hw_st"
//#define TAG_HW_EDU_CESS "mso_hw_edu_cess"
//#define TAG_HW_SHECESS "mso_hw_shecess"
#define	TAG_HW_SWACHH_CESS "mso_hw_swachh_cess"
#define	TAG_SWACHH_CESS	"mso_swachh_cess"
#define TAG_HW_KRISH_CESS "mso_hw_krish_cess"
#define TAG_KRISH_CESS "mso_krish_cess"
#define TAG_USG_SERVICE_TAX "mso_usg_st"
#define	TAG_USG_SWACHH_CESS "mso_usg_swachh_cess"
#define TAG_USG_KRISH_CESS "mso_usg_krish_cess"
#define TAG_GST "mso_gst"
#define TAG_HW_GST "mso_hw_gst"
#define TAG_USG_GST "mso_usg_gst"

#define TAG_VAT "mso_vat"
#define TAG_ARR_ET "mso_et"
#define TAG_CFW_ET "cfw_mso_et"

// Broad band related changes
#define PURCHASE_FEE "/event/billing/product/fee/purchase"
#define ITEM_CYCLE_FORWARD "cycle_forward"
#define BB_SERVICE "/service/telco/broadband"

int tax_impacts = 1; 

const int64 NORMAL_INSTALLATION = 10;
#define NORMAL_INSTALLATION_TAG    "mso_installation_normal"
const int64 WIRELESS_INSTALLATION = 20;
#define WIRELESS_INSTALLATION_TAG  "mso_installation_wireless"
const int64 HW_DEPOSIT_CM = 50;
#define HW_DEPOSIT_CM_TAG          "mso_hw_dep_cm"
const int64 HW_DEPOSIT_CR = 60;
#define HW_DEPOSIT_CR_TAG          "mso_hw_dep_cr"
const int64 HW_DEPOSIT_HUWR = 70;
#define HW_DEPOSIT_HUWR_TAG        "mso_hw_dep_huwr"
const int64 HW_DEPOSIT_ND = 80;
#define HW_DEPOSIT_ND_TAG          "mso_hw_dep_nd"
const int64 HW_DEPOSIT_WLND = 90;
#define HW_DEPOSIT_WLND_TAG        "mso_hw_dep_wlnd"
const int64 HW_DEPOSIT_NWIFI = 100;
#define HW_DEPOSIT_NWIFI_TAG       "mso_hw_dep_nwifi"
const int64 HW_DEPOSIT_CRSOHO = 110;
#define HW_DEPOSIT_CRSOHO_TAG      "mso_hw_dep_crsoho"
const int64 HW_DEPOSIT_DCM = 120;
#define HW_DEPOSIT_DCM_TAG         "mso_hw_dep_dcm"
const int64 HW_DEPOSIT_PO = 130;
#define HW_DEPOSIT_PO_TAG          "mso_hw_dep_po"
const int64 HW_OUTRIGHT_CM = 350;
#define HW_OUTRIGHT_CM_TAG         "mso_hw_outright_cm"
const int64 HW_OUTRIGHT_CR = 360;
#define HW_OUTRIGHT_CR_TAG         "mso_hw_outright_cr"
const int64 HW_OUTRIGHT_HUWR= 370;
#define HW_OUTRIGHT_HUWR_TAG       "mso_hw_outright_huwr"
const int64 HW_OUTRIGHT_CRSOHO = 380;
#define HW_OUTRIGHT_CRSOHO_TAG     "mso_hw_outright_crsoho"
const int64 HW_OUTRIGHT_ND = 390;
#define HW_OUTRIGHT_ND_TAG         "mso_hw_outright_nd"
const int64 HW_OUTRIGHT_NWIFI   = 400;
#define HW_OUTRIGHT_NWIFI_TAG      "mso_hw_outright_nwifi"
const int64 HW_OUTRIGHT_DCM = 410;
#define HW_OUTRIGHT_DCM_TAG        "mso_hw_outright_dcm"
const int64 HW_OUTRIGHT_WLND = 420;
#define HW_OUTRIGHT_WLND_TAG       "mso_hw_outright_wlnd"
const int64 ADDITIONAL_IP = 430;
#define ADDITIONAL_IP_TAG          "mso_ip_addi"
const int64 DOMAIN_HOSTING = 610;
#define DOMAIN_HOSTING_TAG         "mso_domain_host"
const int64 CABLE_SHIFTING = 620;
#define CABLE_SHIFTING_TAG         "mso_cable_shift"
const int64 ADDITIONAL_EMAIL = 630;
#define ADDITIONAL_EMAIL_TAG       "mso_email_addi"
const int64 ADDITIONAL_SPACE = 640;
#define ADDITIONAL_SPACE_TAG       "mso_space_addi"
const int64     FUP_TOPUP = 900;
#define FUP_TOPUP_TAG "mso_sb_fup_topup"
const int64     ADD_MB_GB = 930;
#define ADD_MB_GB_TAG "mso_sb_add_mb_gb"
// Broad band related changes end


#ifdef __cplusplus
extern "C" {

EXPORT_OP void
op_bill_pol_get_item_tag(
    cm_nap_connection_t	*connp,
    u_int		opcode,
    u_int		flags,
    pin_flist_t		*in_flistp,
    pin_flist_t		**out_flistpp,
    pin_errbuf_t	*ebufp);
}

#endif

static void fm_bill_pol_get_item_tag(
    PinContextObserver	&ctxp,
    PinOpFlags		flags,
    PinFlistObserver	&inFlist,
    PinFlistOwner	&outFlist,
    pin_errbuf_t	*ebufp);

static void update_item_tag (PinContextObserver  &ctxp,
                        PinOpFlags              flags,
                        PinFlistObserver &eventFlist,
                        PinFlistOwner &outFlist,
                        PinStrObserver  tag);

static void update_crdr_note_tag(PinFlistObserver &evtFlist,
                          PinFlistOwner    &outFlist);
/**
 * Main routine for the PCM_OP_BILL_POL_GET_ITEM_TAG operation. This
 * policy can be used to further customize the flexible item assignment
 * that allows customers to define the item types to use, depending on
 * service type and event type. The PCM_OP_BILL_ITEM_ASSIGN will call this
 * policy to determine the item tag to be used during item assignment. 
 * By default, it will return the same item tag it receives in its input.
 * @param connp The connection pointer.
 * @param opcode This opcode.
 * @param flags The opcode flags.
 * @param in_flistp The input flist from BILL_ITEM_ASSIGN that contains 
 *        the EVENT substruct and the chosen ITEM_TAG.
 * @param out_flistpp The output flist with possibly an updated ITEM_TAG. 
 * @param ebufp The error buffer.
 * @return nothing.
 */
void
op_bill_pol_get_item_tag(
    cm_nap_connection_t	*connp,
    u_int		opcode,
    u_int		flags,
    pin_flist_t		*in_flistp,
    pin_flist_t		**out_flistpp,
    pin_errbuf_t	*ebufp)
{
    try {
	PinContextObserver ctxp = PinContext::createAsObserved(connp->dm_ctx);
	PinOpFlags         opflags = flags;
	PinFlistObserver   inFlist = PinFlist::createAsObserved(in_flistp);
	PinFlistOwner      outFlist;
	PinErrorBuf        ebuf(ebufp);

	if (ebuf.getError()) {
		return;
	}
	ebuf.Clear();

	*out_flistpp = NULL;

	// Sanity check.
	if (opcode != PCM_OP_BILL_POL_GET_ITEM_TAG) {
	    PIN_SET_ERR(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE,
		PIN_ERR_BAD_OPCODE, 0, 0, opcode);
	    throw PinDeterminateExc(ebufp);
	}

	// Debug: What we got.
	PIN_LOG(inFlist, PIN_ERR_LEVEL_DEBUG,
	    "op_bill_pol_get_item_tag input flist");

	// Do the actual operation.
	fm_bill_pol_get_item_tag(ctxp, opflags, inFlist, outFlist, ebufp);

	// Log and return the output flist.
	PIN_LOG(outFlist, PIN_ERR_LEVEL_DEBUG,
	    "op_bill_pol_get_item_tag output flist");
	*out_flistpp = outFlist->release();
	return;

    } catch (PinDeterminateExc &exc) {
	PIN_LOG(exc, PIN_ERR_LEVEL_ERROR, "op_bill_pol_get_item_tag error");
	exc.copyInto(ebufp);
    }
}

/**
 * This function is the actual implemenatation of the policy.
 * @param connp The context pointer.
 * @param flags The opcode flags.
 * @param inFlist The input flist.
 * @param outFlistp The output flist. Contains the POID and ITEM_TAG from
 *        the inFlist.
 * @param ebufp The error buffer.
 * @return nothing.
 */
static void fm_bill_pol_get_item_tag(
    PinContextObserver	&ctxp,
    PinOpFlags		flags,
    PinFlistObserver	&inFlist,
    PinFlistOwner	&outFlist,
    pin_errbuf_t	*ebufp)
{
    int32	hw_prod = 0;
    int32	usage = 0;
    char	*hw_prefix = "mso_hw";
    char	*inst_prefix = "mso_installation";
    try {

	// Create the output flist with required POID.
	PinBigDecimalOwner priority;
	outFlist = PinFlist::create();
	PinPoidObserver pd = inFlist->get(tsf_PIN_FLD_POID, 0);
	outFlist->set(tsf_PIN_FLD_POID, pd);

	// Your code here to assign a new item tag ...
	// Default implementation is just to return the same one.
	// Note that ITEM_TAG can be NULL if there was no match for
	// a given (event,service) from the /config/item_tags.
	PinStrObserver tag = inFlist->get(tsf_PIN_FLD_ITEM_TAG, 0);
	//MSO Customization Starts
	PIN_MSG(PIN_ERR_LEVEL_DEBUG, "Custom Implementation for fixing the Item Tags");
	if (tag->isNull())
	{
		PIN_MSG(PIN_ERR_LEVEL_DEBUG, "Evaluating Null Item Tags");
		PinFlistObserver evtFlist = inFlist->get(tsf_PIN_FLD_EVENT, 0);
		PinPoidObserver evt_pd = evtFlist->get(tsf_PIN_FLD_POID, 0);
		PinStr evt = (PinStr )evt_pd->getType();
	
		if (0==strcmp((PinStr )evt, MIN_COMM_TYPE) ||
			0==strcmp((PinStr )evt, LATE_FEE_TYPE) ||
			0==strcmp((PinStr )evt, CQ_BOUNCE_PENALTY_TYPE))
		{
		/*
		 * Checks Whether the event poid matches the CATV Min. Commitment type, Based on that it assigns
		 * the item tag that needs to be created for Min. Commitment Pre-rated Event(mso_catv_commitment)
		 */
	        PIN_MSG(PIN_ERR_LEVEL_DEBUG, "Min. Commitment Event");
//			outFlist->set(tsf_PIN_FLD_ITEM_TAG, MIN_COMMIT_TAG);
			PinFlistObserver elem = outFlist->add(tsf_PIN_FLD_ITEM_TAGS, 0);
			elem->set(tsf_PIN_FLD_ITEM_TAG, MIN_COMMIT_TAG);
			PinElemObservingIterator iter = evtFlist->getElements(tsf_PIN_FLD_BAL_IMPACTS);
			while( iter.hasMore()) 
			{
				PinFlistObserver bi_flist = iter.next();
				PinStrObserver tax_code = bi_flist->get(tsf_PIN_FLD_TAX_CODE);
				if( !strcmp( tax_code->value(), "CGST" ) || !strcmp( tax_code->value(), "IGST" ) || !strcmp( tax_code->value(), "SGST" )){
					elem = evtFlist->add(tsf_PIN_FLD_ITEM_TAGS, iter.getRecId());
					elem->set(tsf_PIN_FLD_ITEM_TAG, TAG_GST);
				}
			}
		}
		else if (0==strcmp((PinStr )evt, MSO_CR_DR_NOTE))
		{
		
		//  Checks Whether the event poid matches the Late Payment event, Based on that it assigns
		//  the item tag that needs to be created for Late Payent Pre-rated Event(CR_DR_NOTE)
		 
	        PIN_MSG(PIN_ERR_LEVEL_DEBUG, "CR DR Note");
			update_crdr_note_tag(evtFlist,outFlist);
		}
		/*else if (0==strcmp((PinStr )evt, CQ_BOUNCE_PENALTY_TYPE))
		{
			PIN_MSG(PIN_ERR_LEVEL_DEBUG, "Check Bounce Event");
			PinFlistObserver elem = outFlist->add(tsf_PIN_FLD_ITEM_TAGS, 0);
			elem->set(tsf_PIN_FLD_ITEM_TAG, CQ_BOUNCE_PENALTY_TAG);
			PinElemObservingIterator iter = evtFlist->getElements(tsf_PIN_FLD_BAL_IMPACTS);
			while( iter.hasMore()) 
			{
				PinFlistObserver bi_flist = iter.next();
				PinStrObserver tax_code = bi_flist->get(tsf_PIN_FLD_TAX_CODE);
				if( !strcmp( tax_code->value(), "CGST" ) || !strcmp( tax_code->value(), "IGST" ) || !strcmp( tax_code->value(), "SGST" )){
					elem = outFlist->add(tsf_PIN_FLD_ITEM_TAGS, iter.getRecId());
					elem->set(tsf_PIN_FLD_ITEM_TAG, TAG_GST);
				}
			}
		}
		else if (0==strcmp((PinStr )evt, LATE_FEE_TYPE))
		{
			PIN_MSG(PIN_ERR_LEVEL_DEBUG, "LATE_FEE_TYPE");
			PinFlistObserver elem = outFlist->add(tsf_PIN_FLD_ITEM_TAGS, 0);
			elem->set(tsf_PIN_FLD_ITEM_TAG, LATE_FEE_TAG);
			PinElemObservingIterator iter = evtFlist->getElements(tsf_PIN_FLD_BAL_IMPACTS);
			while( iter.hasMore()) 
			{
				PinFlistObserver bi_flist = iter.next();
				PinStrObserver tax_code = bi_flist->get(tsf_PIN_FLD_TAX_CODE);
				if( !strcmp( tax_code->value(), "CGST" ) || !strcmp( tax_code->value(), "IGST" ) || !strcmp( tax_code->value(), "SGST" )){
					elem = outFlist->add(tsf_PIN_FLD_ITEM_TAGS, iter.getRecId());
					elem->set(tsf_PIN_FLD_ITEM_TAG, TAG_GST);
				}
			}
		}*/
		else 
		{
			PinFlistObserver balFlist = evtFlist->get(tsf_PIN_FLD_BAL_IMPACTS, 0);
			PinPoidObserver pdt_pd = balFlist->get(tsf_PIN_FLD_PRODUCT_OBJ, 1);
			if(!pdt_pd->isNull()){
				PIN_MSG(PIN_ERR_LEVEL_DEBUG, "PRODUCT POID IS NOT NULL");
				PinFlistOwner read_flist = PinFlist::create();
				PinFlistOwner tmp_flist;
				read_flist->set(tsf_PIN_FLD_POID, pdt_pd);
				read_flist->set(tsf_PIN_FLD_PRIORITY, priority);
				ctxp->op(PCM_OP_READ_FLDS, flags, read_flist, tmp_flist);
				priority = (PinBigDecimalOwner ) tmp_flist->take(tsf_PIN_FLD_PRIORITY, 0);
				PIN_MSG(PIN_ERR_LEVEL_DEBUG, "Retrieved Product Priority");
			}
			
			if(0 == (priority->compare((PinBigDecimal )DEPOSIT_PRIORITY)))
			{
			/*
			 * Checks Whether the Product Priority is as of Deposit(20), then
			 * the item tag that needs to be created for deposit payment(MSO_Deposit)
			 */
				PIN_MSG(PIN_ERR_LEVEL_DEBUG, "Deposit Product");
				outFlist->set(tsf_PIN_FLD_ITEM_TAG, DEPOSIT_TAG);
				return;
			}
			else
			{
				outFlist->set(tsf_PIN_FLD_ITEM_TAG, tag);
			}
		}
	}
	else
	{
		PIN_MSG(PIN_ERR_LEVEL_DEBUG, "Evaluating Non-Null Item Tags");
		PinFlistObserver evtFlist = inFlist->get(tsf_PIN_FLD_EVENT, 0);
		PinFlistObserver balFlist = evtFlist->get(tsf_PIN_FLD_BAL_IMPACTS, 0);
		PinStrObserver tax_tag = balFlist->get(tsf_PIN_FLD_TAX_CODE, 0);
		PinPoidObserver evt_pd = evtFlist->get(tsf_PIN_FLD_POID, 0);
		PinStr evt = (PinStr )evt_pd->getType();
		/*Checking for the Tax Code, so that the individual balance impacts are assigned 
		 * to appropriate items, if tax code matches to :
		 * MSO_Service_Tax, 3 additional item tags (ST, Edu.Cess, SHECess) to be added
		 * MSO_VAT, 1 additional item tag (VAT) to be added
		 * MSO_VAT_ST, 4 additional item tags (VAT, ST, Edu.Cess, SHECess) to be added
		 *
		 * Element Id for PIN_FLD_BAL_IMPACTS will be starting from 5(for ST) / 6(for ST + VAT), as the customization
		 * on addition of Balance Impacts Array is done and dropped in rating policy
		 * fm_rate_pol_map_tax_supplier.c file.
		 */

		 PinInt count = evtFlist->count(tsf_PIN_FLD_BAL_IMPACTS);
//		 PIN_LOG(count, PIN_ERR_LEVEL_DEBUG, "Total Count");

		PinPoidObserver svc_pd = evtFlist->get(tsf_PIN_FLD_SERVICE_OBJ, 1);
		PinStr svc_type = (PinStr )svc_pd->getType();
		if(count<=1)
		{
			tax_impacts = 0;
			// Broadband related changes start
			PIN_MSG(PIN_ERR_LEVEL_DEBUG, "Non-Rating Opertations");
			PinPoidObserver pdt_pd = balFlist->get(tsf_PIN_FLD_PRODUCT_OBJ,1);
			if (0==strcmp((PinStr )svc_type, BB_SERVICE))
			{
				update_item_tag(ctxp, flags, evtFlist, outFlist, tag);
			} else {
				outFlist->set(tsf_PIN_FLD_ITEM_TAG, tag);
			}
			PIN_ERRBUF_CLEAR(ebufp);
			// Broadband related changes end
		}
		else
		{
			PIN_MSG(PIN_ERR_LEVEL_DEBUG, "Rated Operations");
			outFlist->set(tsf_PIN_FLD_ITEM_TAG, tag);
			if (strcmp(tag->value(), MIN_COMMIT_TAG)==0 || 
				strcmp(tag->value(), CQ_BOUNCE_PENALTY_TAG)==0 || 
				strcmp(tag->value(), LATE_FEE_TAG)==0
				)
			{
				PinFlistObserver elem = outFlist->add(tsf_PIN_FLD_ITEM_TAGS, 0);
				elem->set(tsf_PIN_FLD_ITEM_TAG, tag);
				PinElemObservingIterator iter =
				evtFlist->getElements(tsf_PIN_FLD_BAL_IMPACTS);
				while( iter.hasMore()) 
				{
					PinFlistObserver bi_flist = iter.next();
					PinStrObserver tax_code = bi_flist->get(tsf_PIN_FLD_TAX_CODE);
					if( !strcmp( tax_code->value(), "CGST" ) || !strcmp( tax_code->value(), "IGST" ) || !strcmp( tax_code->value(), "SGST" )){
						elem = outFlist->add(tsf_PIN_FLD_ITEM_TAGS, iter.getRecId());
						elem->set(tsf_PIN_FLD_ITEM_TAG, TAG_GST);
					}
				}
			}
			else if (strcmp(tag->value(), MSO_CR_DR_NOTE_TAG)==0)
			{
				update_crdr_note_tag(evtFlist,outFlist);
			}
			/*else if (strcmp(tag->value(), CQ_BOUNCE_PENALTY_TAG)==0)
			{
				PinFlistObserver elem = outFlist->add(tsf_PIN_FLD_ITEM_TAGS, 0);
				elem->set(tsf_PIN_FLD_ITEM_TAG, tag);
				PinElemObservingIterator iter =
				evtFlist->getElements(tsf_PIN_FLD_BAL_IMPACTS);
				while( iter.hasMore()) 
				{
					PinFlistObserver bi_flist = iter.next();
					PinStrObserver tax_code = bi_flist->get(tsf_PIN_FLD_TAX_CODE);
					if( !strcmp( tax_code->value(), "CGST" ) || !strcmp( tax_code->value(), "IGST" ) || !strcmp( tax_code->value(), "SGST" )){
						elem = outFlist->add(tsf_PIN_FLD_ITEM_TAGS, iter.getRecId());
						elem->set(tsf_PIN_FLD_ITEM_TAG, TAG_USG_GST);
					}
				}
			}
			else if (strcmp(tag->value(), LATE_FEE_TAG)==0)
			{
				PinFlistObserver elem = outFlist->add(tsf_PIN_FLD_ITEM_TAGS, 0);
				elem->set(tsf_PIN_FLD_ITEM_TAG, tag);
				PinElemObservingIterator iter =
				evtFlist->getElements(tsf_PIN_FLD_BAL_IMPACTS);
				while( iter.hasMore()) 
				{
					PinFlistObserver bi_flist = iter.next();
					PinStrObserver tax_code = bi_flist->get(tsf_PIN_FLD_TAX_CODE);
					if( !strcmp( tax_code->value(), "CGST" ) || !strcmp( tax_code->value(), "IGST" ) || !strcmp( tax_code->value(), "SGST" )){
						elem = outFlist->add(tsf_PIN_FLD_ITEM_TAGS, iter.getRecId());
						elem->set(tsf_PIN_FLD_ITEM_TAG, TAG_USG_GST);
					}
				}
			}*/
			else if (!strcmp(tag->value(), "mso_usage" )){
				update_item_tag(ctxp, flags, evtFlist, outFlist, tag);
				PinFlistObserver elem = outFlist->add(tsf_PIN_FLD_ITEM_TAGS, 0);
				elem->set(tsf_PIN_FLD_ITEM_TAG, tag);
				PinElemObservingIterator iter =
				evtFlist->getElements(tsf_PIN_FLD_BAL_IMPACTS);
				while( iter.hasMore()) 
				{
					PinFlistObserver bi_flist = iter.next();
					PinStrObserver tax_code = bi_flist->get(tsf_PIN_FLD_TAX_CODE);
					if( !strcmp( tax_code->value(), "CGST" ) || !strcmp( tax_code->value(), "IGST" ) || !strcmp( tax_code->value(), "SGST" )){
						elem = outFlist->add(tsf_PIN_FLD_ITEM_TAGS, iter.getRecId());
						elem->set(tsf_PIN_FLD_ITEM_TAG, TAG_USG_GST);
					}
				}
			}
			/*else if (0==strcmp(tax_tag->value(), MSO_ST))
			{
				if (0==strcmp((PinStr )svc_type, BB_SERVICE))
				{
					update_item_tag(ctxp, flags, evtFlist, outFlist, tag);
					tag = outFlist->get(tsf_PIN_FLD_ITEM_TAG, 0);
					if(strncmp(tag->value(), hw_prefix, strlen(hw_prefix)) == 0 ||
					strncmp(tag->value(), inst_prefix, strlen(inst_prefix)) == 0 )
				    {
					if(!strstr(tag->value(),"hw_amc") && 
						!strstr(tag->value(), "hw_additional_ip"))
					{
						hw_prod = 1;
					}
				    }
				}
				if(hw_prod == 1)
				{
					PinFlistObserver elem = outFlist->add(tsf_PIN_FLD_ITEM_TAGS, 0);
					elem->set(tsf_PIN_FLD_ITEM_TAG, tag);
					PinElemObservingIterator iter = outFlist->getElements(tsf_PIN_FLD_BAL_IMPACTS);
					while( iter.hasMore())
					{
						PinFlistObserver bi_flist = iter.next();
						elem = outFlist->add(tsf_PIN_FLD_ITEM_TAGS, iter.getRecId());
						elem->set(tsf_PIN_FLD_ITEM_TAG, TAG_HW_GST);
					}
				} 
				else 
				{
					PinFlistObserver elem = outFlist->add(tsf_PIN_FLD_ITEM_TAGS, 0);
					elem->set(tsf_PIN_FLD_ITEM_TAG, tag);
					PinElemObservingIterator iter = outFlist->getElements(tsf_PIN_FLD_BAL_IMPACTS);
					while( iter.hasMore())
					{
						PinFlistObserver bi_flist = iter.next();
						elem = outFlist->add(tsf_PIN_FLD_ITEM_TAGS, iter.getRecId());
						elem->set(tsf_PIN_FLD_ITEM_TAG, TAG_GST);
					}
				}
			}
			else if (0==strcmp(tax_tag->value(), MSO_VAT_ST))
			{
				if (0==strcmp((PinStr )svc_type, BB_SERVICE))
				{
					update_item_tag(ctxp, flags, evtFlist, outFlist, tag);
					tag = outFlist->get(tsf_PIN_FLD_ITEM_TAG, 0);
				}
				PinFlistObserver elem = outFlist->add(tsf_PIN_FLD_ITEM_TAGS, 0);
				elem->set(tsf_PIN_FLD_ITEM_TAG, tag);
				elem = outFlist->add(tsf_PIN_FLD_ITEM_TAGS, 6);
				elem->set(tsf_PIN_FLD_ITEM_TAG, TAG_VAT);
				elem = outFlist->add(tsf_PIN_FLD_ITEM_TAGS, 7);
				elem->set(tsf_PIN_FLD_ITEM_TAG, TAG_Service_Tax);
				elem = outFlist->add(tsf_PIN_FLD_ITEM_TAGS, 8);
				elem->set(tsf_PIN_FLD_ITEM_TAG, TAG_SWACHH_CESS);
				elem = outFlist->add(tsf_PIN_FLD_ITEM_TAGS, 9);
				elem->set(tsf_PIN_FLD_ITEM_TAG, TAG_KRISH_CESS);
			}
			else if (0==strcmp(tax_tag->value(), MSO_VAT))
			{
				if (0==strcmp((PinStr )svc_type, BB_SERVICE))
				{
					update_item_tag(ctxp, flags, evtFlist, outFlist, tag);
					tag = outFlist->get(tsf_PIN_FLD_ITEM_TAG, 0);
				}
				PinFlistObserver elem = outFlist->add(tsf_PIN_FLD_ITEM_TAGS, 0);
				elem->set(tsf_PIN_FLD_ITEM_TAG, tag);
				elem = outFlist->add(tsf_PIN_FLD_ITEM_TAGS, 1);
				elem->set(tsf_PIN_FLD_ITEM_TAG, TAG_VAT);
			}*/
			else if (0!=strcmp(tax_tag->value(), EMPTY_TAX))
			{
				if (0==strcmp((PinStr )svc_type, BB_SERVICE))
				{
					update_item_tag(ctxp, flags, evtFlist, outFlist, tag);
				    tag = outFlist->get(tsf_PIN_FLD_ITEM_TAG, 0);
				    if(strncmp(tag->value(), hw_prefix, strlen(hw_prefix)) == 0 ||
					strncmp(tag->value(), inst_prefix, strlen(inst_prefix)) == 0 )
				    {
						if(!strstr(tag->value(),"hw_amc") && 
							!strstr(tag->value(), "hw_additional_ip"))
						{
							hw_prod = 1;
						}
				    }
				}
				if(hw_prod == 1)
				{
					PinFlistObserver elem = outFlist->add(tsf_PIN_FLD_ITEM_TAGS, 0);
					elem->set(tsf_PIN_FLD_ITEM_TAG, tag);
					PinElemObservingIterator iter =
					evtFlist->getElements(tsf_PIN_FLD_BAL_IMPACTS);
					while( iter.hasMore()) 
					{
						PinFlistObserver bi_flist = iter.next();
						PinStrObserver tax_code = bi_flist->get(tsf_PIN_FLD_TAX_CODE);
						if( !strcmp( tax_code->value(), "CGST" ) || !strcmp( tax_code->value(), "IGST" ) || !strcmp( tax_code->value(), "SGST" )){
							elem = outFlist->add(tsf_PIN_FLD_ITEM_TAGS, iter.getRecId());
							elem->set(tsf_PIN_FLD_ITEM_TAG, TAG_HW_GST);
						}
					}
				} 
				else 
				{
					PIN_MSG(PIN_ERR_LEVEL_DEBUG, "Cycle Arreat ET Event");
					PinFlistObserver elem = outFlist->add(tsf_PIN_FLD_ITEM_TAGS, 0);
					elem->set(tsf_PIN_FLD_ITEM_TAG, tag);
					PinElemObservingIterator iter =
					evtFlist->getElements(tsf_PIN_FLD_BAL_IMPACTS);
					while( iter.hasMore()) 
					{
						PinFlistObserver bi_flist = iter.next();
						PinStrObserver tax_code = bi_flist->get(tsf_PIN_FLD_TAX_CODE);
						if( !strcmp( tax_code->value(), "CGST" ) || !strcmp( tax_code->value(), "IGST" ) || !strcmp( tax_code->value(), "SGST" )){
							elem = outFlist->add(tsf_PIN_FLD_ITEM_TAGS, iter.getRecId());
							elem->set(tsf_PIN_FLD_ITEM_TAG, TAG_GST);
						}
					}
				}
			}
			else 
			{
				PIN_MSG(PIN_ERR_LEVEL_DEBUG, "Calling update_item_tag to determine the item tag");
				update_item_tag(ctxp, flags, evtFlist, outFlist, tag);
			}
		}
	}
	return;

    } catch (PinDeterminateExc &exc) {
	PIN_LOG(exc, PIN_ERR_LEVEL_ERROR, "fm_bill_pol_get_item_tag error");
	exc.copyInto(ebufp);
    }
}

// Broadband related changes start
void update_item_tag (PinContextObserver  &ctxp,
                        PinOpFlags              flags,
                        PinFlistObserver &evtFlist,
                        PinFlistOwner    &outFlist,
                        PinStrObserver  tag)
{

    PinPoidObserver evt_pd = evtFlist->get(tsf_PIN_FLD_POID, 0);
    PinStr evt = (PinStr )evt_pd->getType();

    PIN_MSG(PIN_ERR_LEVEL_DEBUG, " Event type is ");
    PIN_MSG(PIN_ERR_LEVEL_DEBUG, (PinStr )evt);
    if (strcmp(tag->value(), MSO_CR_DR_NOTE_TAG)==0) {
	    update_crdr_note_tag(evtFlist, outFlist);
	}
    else if (0!=strcmp((PinStr )evt, PURCHASE_FEE) && 0 != strcmp(tag->value(), ITEM_CYCLE_FORWARD))
    {
        outFlist->set(tsf_PIN_FLD_ITEM_TAG, tag );
    }else {

        PinBigDecimalOwner priority;
        PinFlistObserver balFlist = evtFlist->get(tsf_PIN_FLD_BAL_IMPACTS, 0);
        PinPoidObserver pdt_pd = balFlist->get(tsf_PIN_FLD_PRODUCT_OBJ,0);
        PinFlistOwner read_flist = PinFlist::create();
        PinFlistOwner tmp_flist;
        read_flist->set(tsf_PIN_FLD_POID, pdt_pd);
        read_flist->set(tsf_PIN_FLD_PRIORITY, priority);
        ctxp->op(PCM_OP_READ_FLDS, flags, read_flist, tmp_flist);
	PIN_LOG(tmp_flist, PIN_ERR_LEVEL_DEBUG,
	    "product priority output flist");
        priority = (PinBigDecimalOwner ) tmp_flist->take(tsf_PIN_FLD_PRIORITY, 0);
        PinInt int_prio = priority->getInt64();
        PIN_MSG(PIN_ERR_LEVEL_DEBUG, "Retrieved Product Priority");

	if(int_prio > 1000) {
	    int_prio = int_prio % 1000;
	}


        switch (int_prio) {
        case NORMAL_INSTALLATION:
                outFlist->set(tsf_PIN_FLD_ITEM_TAG, NORMAL_INSTALLATION_TAG );
            break;

        case WIRELESS_INSTALLATION:
                outFlist->set(tsf_PIN_FLD_ITEM_TAG, WIRELESS_INSTALLATION_TAG );
            break;

        case HW_DEPOSIT_CM:
                outFlist->set(tsf_PIN_FLD_ITEM_TAG, HW_DEPOSIT_CM_TAG );
            break;

        case HW_DEPOSIT_CR:
                outFlist->set(tsf_PIN_FLD_ITEM_TAG, HW_DEPOSIT_CR_TAG );
            break;

        case HW_DEPOSIT_HUWR:
                outFlist->set(tsf_PIN_FLD_ITEM_TAG, HW_DEPOSIT_HUWR_TAG );
            break;

        case HW_DEPOSIT_ND:
                outFlist->set(tsf_PIN_FLD_ITEM_TAG, HW_DEPOSIT_ND_TAG );
            break;

        case HW_DEPOSIT_WLND:
                outFlist->set(tsf_PIN_FLD_ITEM_TAG, HW_DEPOSIT_WLND_TAG );
            break;

        case HW_DEPOSIT_NWIFI:
                outFlist->set(tsf_PIN_FLD_ITEM_TAG, HW_DEPOSIT_NWIFI_TAG );
            break;

        case HW_DEPOSIT_CRSOHO:
                outFlist->set(tsf_PIN_FLD_ITEM_TAG, HW_DEPOSIT_CRSOHO_TAG );
            break;

        case HW_DEPOSIT_DCM:
                outFlist->set(tsf_PIN_FLD_ITEM_TAG, HW_DEPOSIT_DCM_TAG );
            break;

        case HW_OUTRIGHT_CM:
                outFlist->set(tsf_PIN_FLD_ITEM_TAG, HW_OUTRIGHT_CM_TAG );
            break;

        case HW_OUTRIGHT_CR:
                outFlist->set(tsf_PIN_FLD_ITEM_TAG, HW_OUTRIGHT_CR_TAG );
            break;

        case HW_OUTRIGHT_HUWR:
                outFlist->set(tsf_PIN_FLD_ITEM_TAG, HW_OUTRIGHT_HUWR_TAG );
            break;

        case HW_OUTRIGHT_CRSOHO:
                outFlist->set(tsf_PIN_FLD_ITEM_TAG, HW_OUTRIGHT_CRSOHO_TAG );
            break;

        case HW_OUTRIGHT_ND:
                outFlist->set(tsf_PIN_FLD_ITEM_TAG, HW_OUTRIGHT_ND_TAG );
            break;
          break;

        case HW_OUTRIGHT_NWIFI:
                outFlist->set(tsf_PIN_FLD_ITEM_TAG, HW_OUTRIGHT_NWIFI_TAG );
            break;

        case HW_OUTRIGHT_DCM:
                outFlist->set(tsf_PIN_FLD_ITEM_TAG, HW_OUTRIGHT_DCM_TAG );
            break;

        case HW_OUTRIGHT_WLND:
                outFlist->set(tsf_PIN_FLD_ITEM_TAG, HW_OUTRIGHT_WLND_TAG );
            break;

        case ADDITIONAL_IP:
                outFlist->set(tsf_PIN_FLD_ITEM_TAG, ADDITIONAL_IP_TAG );
            break;

        case DOMAIN_HOSTING:
                outFlist->set(tsf_PIN_FLD_ITEM_TAG, DOMAIN_HOSTING_TAG );
            break;

        case CABLE_SHIFTING:
                outFlist->set(tsf_PIN_FLD_ITEM_TAG, CABLE_SHIFTING_TAG );
            break;

        case ADDITIONAL_EMAIL:
                outFlist->set(tsf_PIN_FLD_ITEM_TAG, ADDITIONAL_EMAIL_TAG );
            break;

        case ADDITIONAL_SPACE:
                outFlist->set(tsf_PIN_FLD_ITEM_TAG, ADDITIONAL_SPACE_TAG );
            break;

        case FUP_TOPUP:
                outFlist->set(tsf_PIN_FLD_ITEM_TAG, FUP_TOPUP_TAG);
            break;

        case ADD_MB_GB:
                outFlist->set(tsf_PIN_FLD_ITEM_TAG, ADD_MB_GB_TAG);
            break;
        }
    }



}

void update_crdr_note_tag(PinFlistObserver &evtFlist,
                          PinFlistOwner    &outFlist) {

	PinFlistObserver elem = outFlist->add(tsf_PIN_FLD_ITEM_TAGS, 0);
	PinStrObserver cr_dr_item_tag = evtFlist->get(tsf_PIN_FLD_DESCR, 0);

	if (!cr_dr_item_tag->isNull()) {
	    outFlist->set(tsf_PIN_FLD_ITEM_TAG,cr_dr_item_tag);
            elem->set(tsf_PIN_FLD_ITEM_TAG, cr_dr_item_tag);
	} else {
	    outFlist->set(tsf_PIN_FLD_ITEM_TAG,MSO_CR_DR_NOTE_TAG);
                elem->set(tsf_PIN_FLD_ITEM_TAG, MSO_CR_DR_NOTE_TAG);
	}
	if(tax_impacts != 0)
	{
        	elem = outFlist->add(tsf_PIN_FLD_ITEM_TAGS, 1);
	        elem->set(tsf_PIN_FLD_ITEM_TAG, TAG_Service_Tax);
		elem = outFlist->add(tsf_PIN_FLD_ITEM_TAGS, 2);
                elem->set(tsf_PIN_FLD_ITEM_TAG, TAG_SWACHH_CESS);
		elem = outFlist->add(tsf_PIN_FLD_ITEM_TAGS, 3);
                elem->set(tsf_PIN_FLD_ITEM_TAG, TAG_KRISH_CESS);
       		/*elem = outFlist->add(tsf_PIN_FLD_ITEM_TAGS, 2);
       		elem->set(tsf_PIN_FLD_ITEM_TAG, TAG_EDU_CESS);
       		elem = outFlist->add(tsf_PIN_FLD_ITEM_TAGS, 3);
        	elem->set(tsf_PIN_FLD_ITEM_TAG, TAG_SHECESS);*/
	}
}
// Broadband related changes end
