
/*******************************************************************
 * File:        fm_mso_commission_handle_pymt_reversal.c
 * Opcode:      MSO_OP_COMMISSION_HANDLE_PYMT_REVERSAL
 * Description:
 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_commission_handle_pymt_reversal.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "pin_cust.h"
#include "ops/act.h"
#include "ops/cust.h"
#include "ops/bill.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_rate.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "mso_cust.h"
#include "mso_commission.h"

EXPORT_OP void
op_mso_commission_handle_pymt_reversal (
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);


static void
fm_mso_commission_handle_pymt_reversal (
        pcm_context_t            *ctxp,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_mso_commission_get_events_related_to_pymt (
        pcm_context_t            *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

static void 
fm_mso_commission_revert_commission (
        pcm_context_t            *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_mso_commission_create_report (
        pcm_context_t            *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

void
op_mso_commission_handle_pymt_reversal (
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp) {

        pcm_context_t           *ctxp = connp->dm_ctx;
        *r_flistpp              = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

        /*******************************************************************
         * Insanity Check
         *******************************************************************/
        if (opcode != MSO_OP_COMMISSION_HANDLE_PYMT_REVERSAL) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_mso_commission_handle_pymt_reversal error",ebufp);
                return;
        }
        /*******************************************************************

        /*******************************************************************
         * Debug: Input flist
         *******************************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_mso_commission_handle_pymt_reversal input flist", i_flistp);

        fm_mso_commission_handle_pymt_reversal(ctxp, flags, i_flistp, r_flistpp, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_mso_commission_handle_pymt_reversal output flist", *r_flistpp);
        return;
}

static void
fm_mso_commission_handle_pymt_reversal(
        pcm_context_t           *ctxp,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t		*events_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_handle_pymt_reversal input flist", i_flistp);

	//Get all the commission report/events to revert 
	fm_mso_commission_get_events_related_to_pymt(ctxp, i_flistp, &events_flistp, ebufp);

	//Revert all the commission and create the report for the same
	fm_mso_commission_revert_commission(ctxp, events_flistp, &r_flistp, ebufp);

	*r_flistpp = r_flistp; 

	PIN_FLIST_DESTROY_EX(&events_flistp, NULL);
}
	

static void
fm_mso_commission_get_events_related_to_pymt (
        pcm_context_t            *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp) {

        poid_t          *acct_objp = NULL;
        poid_t          *pymt_item_objp = NULL;
        poid_t          *s_pdp = NULL;

        pin_flist_t     *acct_flistp = NULL;
        pin_flist_t     *s_flistp = NULL;
        pin_flist_t     *args_flistp = NULL;
        pin_flist_t     *ch_flistp = NULL;
        pin_flist_t     *tmp_flistp = NULL;
        pin_flist_t     *r_s_flistp = NULL;

        int32           status = 1;
        int32           sflag = 256;
        int32           ret_val = 0;
        int64           db = 1;
        int             elem_id = 0;
        pin_cookie_t cookie = NULL;
	char            *template = "select X from /mso_commission_rpt/chrg_head_based where F1 = V1 and F2 = V2 and F3 = V3 ";


        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_get_events_related_to_pymt input flist", i_flistp);

        /*Get the LCO/DTR account poid and payment item from the input START*/
        acct_objp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
        pymt_item_objp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ITEM_OBJ, 0, ebufp);
        /*Get the LCO/DTR account poid and payment item from the input END*/

        /*Get all the commission belongs to the old lco/DTR account START*/
        s_flistp = PIN_FLIST_CREATE(ebufp);
        s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
        PIN_FLIST_FLD_PUT(s_flistp,PIN_FLD_POID, s_pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_FLAGS, (void*)&sflag, ebufp);
        PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_TEMPLATE, template, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acct_objp, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
        ch_flistp = PIN_FLIST_SUBSTR_ADD(args_flistp, MSO_FLD_CHARGE_HEAD_INFO, ebufp);
        PIN_FLIST_FLD_SET(ch_flistp,PIN_FLD_ITEM_OBJ, pymt_item_objp, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_STATUS, &status, ebufp);
        PIN_FLIST_ELEM_SET(s_flistp, NULL, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_handle_pymt_reversal get old commissions input flist", s_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_s_flistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_commission_handle_pymt_reversal get old commissions search error",ebufp);
                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_handle_pymt_reversal get old commissions output flist", r_s_flistp);
        /*Get all the commission belongs to the old lco/DTR account END*/

        *r_flistpp = PIN_FLIST_COPY(r_s_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_get_events_related_to_pymt output flist", *r_flistpp);

        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&r_s_flistp, NULL);

}

static void
fm_mso_commission_revert_commission (
        pcm_context_t            *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp) {

	pin_flist_t		*adj_flistp = NULL;
	pin_flist_t		*r_adj_flistp = NULL;
	pin_flist_t		*adj_event_flistp = NULL;
	pin_flist_t		*adj_info_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*tmp_flistp1 = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*rr_flistp = NULL;

	int32			elem_id = 0;
	int32			resource_id = 356;
	int32			adj_flag = ADJ_NO_TAX;
	pin_cookie_t		cookie = NULL;
	void			*vp = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_revert_commission input flist", i_flistp);

	adj_flistp = PIN_FLIST_CREATE(ebufp);
	adj_event_flistp = PIN_FLIST_ELEM_ADD(adj_flistp, PIN_FLD_EVENTS, 0, ebufp);
	adj_info_flistp = PIN_FLIST_ELEM_ADD(adj_flistp, PIN_FLD_ADJUSTMENT_INFO, 0, ebufp);
	PIN_FLIST_FLD_SET(adj_info_flistp, PIN_FLD_RESOURCE_ID, (void*) &resource_id, ebufp);
	PIN_FLIST_FLD_SET(adj_info_flistp, PIN_FLD_FLAGS, (void*) &adj_flag, ebufp);
	PIN_FLIST_FLD_SET(adj_flistp, PIN_FLD_PROGRAM_NAME, "LCO/DTR Commission Reversal", ebufp);
	PIN_FLIST_FLD_SET(adj_flistp, PIN_FLD_DESCR, "LCO/DTR Commission Reversal", ebufp);
	
	r_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);

	while ((res_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp )) != NULL) {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "rev", res_flistp);
		vp = NULL;
		vp = PIN_FLIST_FLD_GET(res_flistp, MSO_FLD_LCO_OBJ, 1, ebufp);
		if (PIN_POID_IS_NULL(vp) == 0) {
			PIN_FLIST_FLD_COPY(res_flistp, MSO_FLD_LCO_OBJ, adj_flistp, PIN_FLD_POID, ebufp);
		}
		else {
			PIN_FLIST_FLD_COPY(res_flistp, MSO_FLD_DT_OBJ, adj_flistp, PIN_FLD_POID, ebufp);
		}
		PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_EVENT_OBJ, adj_event_flistp, PIN_FLD_POID, ebufp);
	
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_revert_commission event adjustment input flist", adj_flistp);
		PCM_OP(ctxp, PCM_OP_AR_EVENT_ADJUSTMENT, 0, adj_flistp, &r_adj_flistp, ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp)) {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_commission_revert_commission event adjustment error",ebufp);
                        return;
                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_revert_commission event adjustment output flist", r_adj_flistp);
		fm_mso_commission_create_report(ctxp, res_flistp, &rr_flistp, ebufp);

		tmp_flistp = PIN_FLIST_ELEM_ADD(r_flistp, PIN_FLD_RESULTS, elem_id, ebufp);
		PIN_FLIST_FLD_COPY(r_adj_flistp, PIN_FLD_POID, tmp_flistp, PIN_FLD_POID, ebufp);
		tmp_flistp1 = PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_EVENTS, 0, ebufp);
		PIN_FLIST_FLD_COPY(r_adj_flistp, PIN_FLD_EVENT_OBJ, tmp_flistp1, PIN_FLD_EVENT_OBJ, ebufp);
		tmp_flistp1 = PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_EVENTS, 1, ebufp);
		PIN_FLIST_FLD_COPY(rr_flistp, PIN_FLD_POID, tmp_flistp1, PIN_FLD_EVENT_OBJ, ebufp);
		PIN_FLIST_DESTROY_EX(&r_adj_flistp, NULL);

	}
	*r_flistpp = r_flistp;
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_revert_commission output flist", *r_flistpp);
}

static void
fm_mso_commission_create_report (
        pcm_context_t            *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp) {

	pin_flist_t		*r_flistp = NULL;
	poid_t			*comm_poidp = NULL;
	pin_decimal_t		*rev_comm_charge = NULL;
	int32			db = 1;
	int32			status = 555;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_create_report input flist", i_flistp);

	/*Update the existing commission report object's status to 555 START*/
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_STATUS, &status, ebufp);
        PIN_FLIST_FLD_DROP(i_flistp, PIN_FLD_CREATED_T, ebufp);
        PIN_FLIST_FLD_DROP(i_flistp, PIN_FLD_MOD_T, ebufp);
        PIN_FLIST_FLD_DROP(i_flistp, PIN_FLD_READ_ACCESS, ebufp);
        PIN_FLIST_FLD_DROP(i_flistp, PIN_FLD_WRITE_ACCESS, ebufp);
	PCM_OP (ctxp, PCM_OP_WRITE_FLDS, 0, i_flistp, &r_flistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_commission_create_report updating existing commission report error",ebufp);
                return;
        }
	/*Update the existing commission report object's status to 555 END*/

	/*Create the reversal commission report object START*/
	*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
	PIN_FLIST_FLD_DROP(*r_flistpp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_DROP(*r_flistpp, MSO_FLD_COMMISSION_CHARGE, ebufp);

	comm_poidp = PIN_POID_CREATE(db, "/mso_commission_rpt/chrg_head_based", -1, ebufp);
	PIN_FLIST_FLD_PUT(*r_flistpp, PIN_FLD_POID, comm_poidp, ebufp);
	rev_comm_charge = pbo_decimal_negate(PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_COMMISSION_CHARGE, 0, ebufp), ebufp);
	PIN_FLIST_FLD_PUT(*r_flistpp, MSO_FLD_COMMISSION_CHARGE, rev_comm_charge, ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status, ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS_MSG, "Commission Reversed Due to Payment Reversal", ebufp);

	PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, *r_flistpp, &r_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_commission_revert_commission event adjustment error",ebufp);
		return;
	}
	PIN_FLIST_DESTROY_EX(r_flistpp, NULL);
	*r_flistpp = r_flistp;

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_create_report output flist", *r_flistpp);
	/*Create the reversal commission report object END*/

}
