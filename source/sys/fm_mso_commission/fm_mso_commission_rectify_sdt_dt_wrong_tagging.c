
/*******************************************************************
 * File:        fm_mso_commission_rectify_sdt_dt_wrong_tagging.c
 * Opcode:      MSO_OP_COMMISSION_RECTIFY_SDT_DT_WRONG_TAGGING
 * Description:
 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_commission_rectify_sdt_dt_wrong_tagging.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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
#include "mso_errs.h"



/*******************************************
External Subroutines - Start
********************************************/
extern int32
fm_mso_utils_get_account_type(
        pcm_context_t           *ctxp,
        poid_t                  *acnt_pdp,
        pin_errbuf_t            *ebufp);

extern void
fm_mso_utils_prepare_status_flist(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        int                     flag,
        char                    *status_descr,
        pin_errbuf_t            *ebufp);

extern int
fm_mso_commission_get_vald_lco_acct_comm_mdl (
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

extern void 
fm_mso_utils_get_bal_grp
(
    pcm_context_t       *ctxp,
    poid_t              *act_pdp,
    char		*flags,
    poid_t             	**ret_bal_grp_poidp,
    poid_t             	**ret_serv_poidp,
    pin_errbuf_t       	*ebufp);

extern void
fm_mso_utils_prepare_error_flist(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        char                    *error_code,
        char                    *error_descr,
        pin_errbuf_t            *ebufp);

extern void
fm_mso_commission_record_failure (
        pcm_context_t           *ctxp,
        char                    *event_type,
        pin_flist_t             *i_flistp,
        pin_flist_t             *err_flistp,
        pin_errbuf_t            *ebufp);

extern int
fm_mso_commission_acct_has_agreement(
        pcm_context_t           *ctxp,
        poid_t                  *lco_acct_objp,
        pin_errbuf_t            *ebufp) ;

extern int
fm_mso_get_bu_commission_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		**out_flist,
	int32			flags,
	pin_errbuf_t		*ebufp);

/*******************************************
External Subroutines - End
********************************************/

EXPORT_OP void
op_mso_commission_rectify_sdt_dt_wrong_tagging (
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);


static void
fm_mso_commission_rectify_sdt_dt_wrong_tagging (
        pcm_context_t            *ctxp,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_mso_commission_get_sdt_dt_commission_charges(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **old_lco_comm_flistp,
        pin_flist_t             **err_flistp,
	int32			child_acct_type,
	int32			parent_acct_type,
        pin_errbuf_t            *ebufp);

static void
bu_change_revert_sdt_commission(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **err_flistp,
        pin_errbuf_t            *ebufp);

static void
bu_change_revert_dtr_commission(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **err_flistp,
        pin_errbuf_t            *ebufp);


static void
bu_change_apply_commission_for_new_sdt(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             *old_lco_comm_flistp,
        pin_flist_t             **err_flistp,
        pin_errbuf_t            *ebufp);

static void
bu_change_apply_commission_for_new_dtr(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             *old_lco_comm_flistp,
        pin_flist_t             **err_flistp,
        pin_errbuf_t            *ebufp);

void
op_mso_commission_rectify_sdt_dt_wrong_tagging (
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
        if (opcode != MSO_OP_COMMISSION_RECTIFY_SDT_DT_WRONG_TAGGING) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_mso_commission_rectify_sdt_dt_wrong_tagging error",ebufp);
                return;
        }
        /*******************************************************************

        /*******************************************************************
         * Debug: Input flist
         *******************************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_mso_commission_rectify_sdt_dt_wrong_tagging input flist ", i_flistp);

        fm_mso_commission_rectify_sdt_dt_wrong_tagging(ctxp, flags, i_flistp, r_flistpp, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_mso_commission_rectify_sdt_dt_wrong_tagging output flist", *r_flistpp);

        return;
}

static void
fm_mso_commission_rectify_sdt_dt_wrong_tagging(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t	*all_comm_flistp = NULL;
//	pin_flist_t	*old_lco_comm_flistp = NULL;
	pin_flist_t	*old_sdt_comm_flistp = NULL;
	pin_flist_t	*old_dtr_comm_flistp = NULL;
//	pin_flist_t	*lco_results = NULL;
	pin_flist_t	*sdt_results = NULL;
	pin_flist_t	*dtr_results = NULL;
	pin_flist_t	*err_flistp = NULL;
	pin_flist_t	*old_ws_info    = NULL;
	pin_flist_t	*new_ws_info    = NULL;
	pin_flist_t	*old_data_arr = NULL;
	pin_flist_t	*new_data_arr = NULL;

	poid_t		*acct_pdp = NULL;
	int32		acct_type = 0;
	int32		parent_acct_type = 0 ;

	void		*vp = NULL;

	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_rectify_sdt_dt_wrong_tagging input flist", i_flistp);

	/*populate old hierarchy details if not passed*/

	new_ws_info = PIN_FLIST_SUBSTR_GET(i_flistp, MSO_FLD_WHOLESALE_INFO, 1, ebufp );
	if (new_ws_info)
	{
		vp = (int32*)PIN_FLIST_FLD_GET(new_ws_info, PIN_FLD_CUSTOMER_TYPE , 1, ebufp);
		if (vp )
		{
			parent_acct_type = *(int32*)vp;
		}
	}

	acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp );
	if (acct_pdp)
	{
		acct_type = fm_mso_utils_get_account_type(ctxp, acct_pdp, ebufp);
		if (acct_type != ACCT_TYPE_LCO && acct_type != ACCT_TYPE_SUB_DTR )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Child account type not LCO/SDT");
			return;
		}
		if (parent_acct_type != ACCT_TYPE_DTR && parent_acct_type != ACCT_TYPE_SUB_DTR )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Parent account type not SDT/DT");
			return;
		}
		
		if (acct_type == ACCT_TYPE_LCO )
		{
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, i_flistp, MSO_FLD_LCO_OBJ, ebufp);
		}
		else if (acct_type == ACCT_TYPE_SUB_DTR )
		{
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, i_flistp, MSO_FLD_SDT_OBJ, ebufp);
		}

		fm_mso_get_bu_commission_info (ctxp, i_flistp, &old_ws_info, acct_type, ebufp);
	}


	new_data_arr = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_DATA_ARRAY, 1, ebufp);
	PIN_FLIST_SUBSTR_SET(new_data_arr, new_ws_info, MSO_FLD_WHOLESALE_INFO, ebufp);
	
	old_data_arr = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_DATA_ARRAY, 0, ebufp);
	PIN_FLIST_SUBSTR_PUT(old_data_arr, old_ws_info, MSO_FLD_WHOLESALE_INFO, ebufp);



	/*: Get all the commission belongs to the old lco account START*/
	fm_mso_commission_get_sdt_dt_commission_charges(ctxp, i_flistp, &all_comm_flistp, &err_flistp, acct_type, parent_acct_type, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_commission_revert_lco_commission: Write fields error",ebufp);
		return;
	}
	/*: Get all the commission belongs to the old lco account END*/

	if (all_comm_flistp)
	{
		old_sdt_comm_flistp = PIN_FLIST_ELEM_GET(all_comm_flistp, MSO_FLD_SDTINFO, 0, 1, ebufp);
		old_dtr_comm_flistp = PIN_FLIST_ELEM_GET(all_comm_flistp, MSO_FLD_DTRINFO, 0, 1, ebufp);
	}

	if (old_sdt_comm_flistp)
	{
		sdt_results = PIN_FLIST_ELEM_GET(old_sdt_comm_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp );
	}
	if (old_dtr_comm_flistp)
	{
		dtr_results = PIN_FLIST_ELEM_GET(old_dtr_comm_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp );
	}

	/*********************
	SDT
	*********************/
	/*: Revert all the commission belongs to the old lco account START*/
	if(sdt_results) 
	{
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, old_sdt_comm_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
		bu_change_revert_sdt_commission(ctxp, old_sdt_comm_flistp, &err_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "bu_change_revert_sdt_commission: Write fields error",ebufp);
			PIN_ERRBUF_RESET(ebufp);
			goto CLEANUP;
		}
	}
	/*: Revert all the commission belongs to the old lco account END*/

	
	/*: Apply all applicable commission to the new LCO account START*/
	if(sdt_results) 
	{
		bu_change_apply_commission_for_new_sdt(ctxp, i_flistp, old_sdt_comm_flistp, &err_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "bu_change_apply_commission_for_new_sdt: Write fields error",ebufp);
			PIN_ERRBUF_RESET(ebufp);
			goto CLEANUP;
		}
	}
	/*: Apply all applicable commission to the new LCO account END*/

	/*********************
	DT
	*********************/
	/*: Revert all the commission belongs to the old lco account START*/
	if(dtr_results) 
	{
		PIN_FLIST_FLD_COPY(i_flistp,  PIN_FLD_PROGRAM_NAME, old_dtr_comm_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
		bu_change_revert_dtr_commission(ctxp, old_dtr_comm_flistp, &err_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "bu_change_revert_dtr_commission: Write fields error",ebufp);
			PIN_ERRBUF_RESET(ebufp);
			goto CLEANUP;
		}
	}
	/*: Revert all the commission belongs to the old lco account END*/

	
	/*: Apply all applicable commission to the new LCO account START*/
	if(dtr_results) 
	{
		bu_change_apply_commission_for_new_dtr(ctxp, i_flistp, old_dtr_comm_flistp, &err_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "bu_change_apply_commission_for_new_dtr: Write fields error",ebufp);
			PIN_ERRBUF_RESET(ebufp);
			goto CLEANUP;
		}
	}
	/*: Apply all applicable commission to the new LCO account END*/


CLEANUP:
	if (err_flistp) 
	{
		*r_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
	}
	else 
	{
		fm_mso_utils_prepare_status_flist(ctxp, i_flistp, &err_flistp, PIN_FLD_POID, "COMMISSION - Wrong Tagging Rectified Succesfully", ebufp);
		*r_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
	}
	PIN_FLIST_DESTROY_EX(&err_flistp,ebufp);
	PIN_FLIST_DESTROY_EX(&all_comm_flistp,ebufp);
	return;
}


/************************************************
function to adjust the wrong commssions 
This function will call the OOB PCM_OP_AR_EVENT_ADJUSTMENT for wrong commssions 
It will also create a new commssion objects with new adjustment event 
***********************************************/
static void
bu_change_revert_sdt_commission(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
	pin_flist_t		**err_flistp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t		*adj_evt_i_flistp = NULL;
	pin_flist_t 		*adj_evt_o_flistp = NULL;
	pin_flist_t  		*adj_evts_arry_flp = NULL;
	pin_flist_t 		*adj_info_arry_flp = NULL;
	pin_flist_t 		*src_res_flistp = NULL;
	pin_flist_t		*wrtflds_iflistp = NULL;
	pin_flist_t		*wrtflds_oflistp = NULL;
//	pin_flist_t		*prs_comm_iflistp = NULL;
//	pin_flist_t		*prs_comm_oflistp = NULL;
//	pin_flist_t		*tmp_flistp = NULL;
//	pin_flist_t		*tmp_balimp_flp = NULL;
	pin_flist_t		*rev_comm_flistp = NULL;
	pin_flist_t		*r_rev_comm_flistp = NULL;

	pin_cookie_t 		cookie = NULL;
//	pin_cookie_t 		tmp_cookie = NULL;

	int32			status = 999;
	int64			db = 1;
	int			elem_id = 0;
//	int			tmp_elem_id = 0;
	int 			resource_id = CURRENCY;
	int 			adj_flag = ADJ_NO_TAX;
	pin_decimal_t           *rev_comm_charge = NULL;

	char			*poid_type = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "bu_change_revert_sdt_commission input flist", i_flistp);

	while ((src_res_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp )) != NULL) 
	{
		adj_evt_i_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(src_res_flistp,MSO_FLD_SDT_OBJ,adj_evt_i_flistp,PIN_FLD_POID,ebufp);
		adj_evts_arry_flp = PIN_FLIST_ELEM_ADD(adj_evt_i_flistp,PIN_FLD_EVENTS,0,ebufp);
		PIN_FLIST_FLD_COPY(src_res_flistp,PIN_FLD_EVENT_OBJ,adj_evts_arry_flp,PIN_FLD_POID,ebufp);
		adj_info_arry_flp = PIN_FLIST_ELEM_ADD(adj_evt_i_flistp,PIN_FLD_ADJUSTMENT_INFO,0,ebufp);
		PIN_FLIST_FLD_SET(adj_info_arry_flp,PIN_FLD_RESOURCE_ID,(void * ) &resource_id,ebufp);
		PIN_FLIST_FLD_SET(adj_info_arry_flp,PIN_FLD_FLAGS,(void * ) &adj_flag,ebufp);
		//PIN_FLIST_FLD_SET(adj_evt_i_flistp,PIN_FLD_PROGRAM_NAME,"SDT Commission Correction",ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, adj_evt_i_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
		PIN_FLIST_FLD_SET(adj_evt_i_flistp,PIN_FLD_DESCR,"SDT Commission Correction",ebufp);
		PIN_FLIST_FLD_SET(adj_evt_i_flistp,PIN_FLD_SYS_DESCR,"SDT Commission Correction",ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "bu_change_revert_sdt_commission  OOB adjustment input flist", adj_evt_i_flistp);
		PCM_OP(ctxp, PCM_OP_AR_EVENT_ADJUSTMENT, 0, adj_evt_i_flistp, &adj_evt_o_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_commission_rectify_sdt_wrong_tagging Event Adjustment error",ebufp);
			return;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "bu_change_revert_sdt_commission  OOB adjustment output flist", adj_evt_o_flistp);

		//To Update the status of commission report to 999(Unique status)
		wrtflds_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(src_res_flistp,PIN_FLD_POID,wrtflds_iflistp,PIN_FLD_POID,ebufp);
		PIN_FLIST_FLD_SET(wrtflds_iflistp, PIN_FLD_STATUS, &status, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "bu_change_revert_sdt_commission: Write fields input flist", wrtflds_iflistp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, wrtflds_iflistp, &wrtflds_oflistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "bu_change_revert_sdt_commission: Write fields error",ebufp);
			PIN_FLIST_DESTROY_EX(&wrtflds_iflistp,ebufp);
                        return;
                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_revert_lco_commission: Write fields output flist", wrtflds_oflistp);
		PIN_FLIST_DESTROY_EX(&wrtflds_iflistp,ebufp);
		PIN_FLIST_DESTROY_EX(&wrtflds_oflistp,ebufp);

		//Creating commission report to nullify the old charge 
		poid_type = (char*)PIN_POID_GET_TYPE( PIN_FLIST_FLD_GET(src_res_flistp, PIN_FLD_POID, 0, ebufp));
		rev_comm_flistp = PIN_FLIST_COPY(src_res_flistp, ebufp);
		PIN_FLIST_FLD_DROP (rev_comm_flistp, PIN_FLD_CREATED_T, ebufp);
		PIN_FLIST_FLD_DROP (rev_comm_flistp, PIN_FLD_MOD_T, ebufp);
		PIN_FLIST_FLD_DROP (rev_comm_flistp, PIN_FLD_READ_ACCESS, ebufp);
		PIN_FLIST_FLD_DROP (rev_comm_flistp, PIN_FLD_WRITE_ACCESS, ebufp);
		PIN_FLIST_FLD_PUT (rev_comm_flistp, PIN_FLD_POID, 
			PIN_POID_CREATE(db, poid_type, -1, ebufp), ebufp);
		rev_comm_charge = pbo_decimal_negate(PIN_FLIST_FLD_GET(rev_comm_flistp, MSO_FLD_COMMISSION_CHARGE, 0, ebufp), ebufp);
		PIN_FLIST_FLD_DROP (rev_comm_flistp, MSO_FLD_COMMISSION_CHARGE, ebufp);
		PIN_FLIST_FLD_PUT(rev_comm_flistp, MSO_FLD_COMMISSION_CHARGE, rev_comm_charge, ebufp);
		PIN_FLIST_FLD_SET(rev_comm_flistp, PIN_FLD_STATUS_MSG, "Commission Reversed Due to SDT Wrong Tagging", ebufp);

                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "bu_change_revert_sdt_commission: Create input flist", rev_comm_flistp);
                PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, rev_comm_flistp, &r_rev_comm_flistp, ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp)) {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "bu_change_revert_sdt_commission: Create error",ebufp);
                        return;
                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "bu_change_revert_sdt_commission: Create output flist", r_rev_comm_flistp);
		PIN_FLIST_DESTROY_EX(&rev_comm_flistp,ebufp);
		PIN_FLIST_DESTROY_EX(&r_rev_comm_flistp,ebufp);
		
		/* creating commision object after adjustment */ 
		PIN_FLIST_DESTROY_EX(&adj_evt_o_flistp,ebufp);
		PIN_FLIST_DESTROY_EX(&adj_evt_i_flistp,ebufp);
		PIN_FLIST_DESTROY_EX(&wrtflds_iflistp,ebufp);
		PIN_FLIST_DESTROY_EX(&wrtflds_oflistp,ebufp);
	}
}

/************************************************
function to adjust the wrong commssions 
This function will call the OOB PCM_OP_AR_EVENT_ADJUSTMENT for wrong commssions 
It will also create a new commssion objects with new adjustment event 
***********************************************/
static void
bu_change_revert_dtr_commission(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
	pin_flist_t		**err_flistp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t		*adj_evt_i_flistp = NULL;
	pin_flist_t 		*adj_evt_o_flistp = NULL;
	pin_flist_t  		*adj_evts_arry_flp = NULL;
	pin_flist_t 		*adj_info_arry_flp = NULL;
	pin_flist_t 		*src_res_flistp = NULL;
	pin_flist_t		*wrtflds_iflistp = NULL;
	pin_flist_t		*wrtflds_oflistp = NULL;
//	pin_flist_t		*prs_comm_iflistp = NULL;
//	pin_flist_t		*prs_comm_oflistp = NULL;
//	pin_flist_t		*tmp_flistp = NULL;
//	pin_flist_t		*tmp_balimp_flp = NULL;
	pin_flist_t		*rev_comm_flistp = NULL;
	pin_flist_t		*r_rev_comm_flistp = NULL;

	pin_cookie_t 		cookie = NULL;
//	pin_cookie_t 		tmp_cookie = NULL;

	int64			db = 1;
	int			elem_id = 0;
	int32			status = 999;
//	int			tmp_elem_id = 0;
	int 			resource_id = CURRENCY;
	int 			adj_flag = ADJ_NO_TAX;
	pin_decimal_t           *rev_comm_charge = NULL;

	char			*poid_type = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "bu_change_revert_dtr_commission input flist", i_flistp);

	while ((src_res_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp )) != NULL) {
		adj_evt_i_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(src_res_flistp,MSO_FLD_DT_OBJ,adj_evt_i_flistp,PIN_FLD_POID,ebufp);
		adj_evts_arry_flp = PIN_FLIST_ELEM_ADD(adj_evt_i_flistp,PIN_FLD_EVENTS,0,ebufp);
		PIN_FLIST_FLD_COPY(src_res_flistp,PIN_FLD_EVENT_OBJ,adj_evts_arry_flp,PIN_FLD_POID,ebufp);
		adj_info_arry_flp = PIN_FLIST_ELEM_ADD(adj_evt_i_flistp,PIN_FLD_ADJUSTMENT_INFO,0,ebufp);
		PIN_FLIST_FLD_SET(adj_info_arry_flp,PIN_FLD_RESOURCE_ID,(void * ) &resource_id,ebufp);
		PIN_FLIST_FLD_SET(adj_info_arry_flp,PIN_FLD_FLAGS,(void * ) &adj_flag,ebufp);
		//PIN_FLIST_FLD_SET(adj_evt_i_flistp,PIN_FLD_PROGRAM_NAME,"DTR Commission Correction",ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, adj_evt_i_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
		PIN_FLIST_FLD_SET(adj_evt_i_flistp,PIN_FLD_DESCR,"DTR Commission Correction",ebufp);
		PIN_FLIST_FLD_SET(adj_evt_i_flistp,PIN_FLD_SYS_DESCR,"DTR Commission Correction",ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_revert_dtr_commission  OOB adjustment input flist", adj_evt_i_flistp);
		PCM_OP(ctxp, PCM_OP_AR_EVENT_ADJUSTMENT, 0, adj_evt_i_flistp, &adj_evt_o_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_commission_rectify_dtr_wrong_tagging Event Adjustment error",ebufp);
			return;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "bu_change_revert_dtr_commission  OOB adjustment output flist", adj_evt_o_flistp);

		//To Update the status of commission report to 999(Unique status)
		wrtflds_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(src_res_flistp,PIN_FLD_POID,wrtflds_iflistp,PIN_FLD_POID,ebufp);
		PIN_FLIST_FLD_SET(wrtflds_iflistp, PIN_FLD_STATUS, &status, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "bu_change_revert_dtr_commission: Write fields input flist", wrtflds_iflistp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, wrtflds_iflistp, &wrtflds_oflistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "bu_change_revert_dtr_commission: Write fields error",ebufp);
			PIN_FLIST_DESTROY_EX(&wrtflds_iflistp,ebufp);
                        return;
                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_revert_dtr_commission: Write fields output flist", wrtflds_oflistp);
		PIN_FLIST_DESTROY_EX(&wrtflds_iflistp,ebufp);
		PIN_FLIST_DESTROY_EX(&wrtflds_oflistp,ebufp);

		//Creating commission report to nullify the old charge 
		poid_type = (char*)PIN_POID_GET_TYPE( PIN_FLIST_FLD_GET(src_res_flistp, PIN_FLD_POID, 0, ebufp));
		rev_comm_flistp = PIN_FLIST_COPY(src_res_flistp, ebufp);
		PIN_FLIST_FLD_DROP (rev_comm_flistp, PIN_FLD_CREATED_T, ebufp);
		PIN_FLIST_FLD_DROP (rev_comm_flistp, PIN_FLD_MOD_T, ebufp);
		PIN_FLIST_FLD_DROP (rev_comm_flistp, PIN_FLD_READ_ACCESS, ebufp);
		PIN_FLIST_FLD_DROP (rev_comm_flistp, PIN_FLD_WRITE_ACCESS, ebufp);
		PIN_FLIST_FLD_PUT (rev_comm_flistp, PIN_FLD_POID, 
			PIN_POID_CREATE(db, poid_type, -1, ebufp), ebufp);
		rev_comm_charge = pbo_decimal_negate(PIN_FLIST_FLD_GET(rev_comm_flistp, MSO_FLD_COMMISSION_CHARGE, 0, ebufp), ebufp);
		PIN_FLIST_FLD_DROP (rev_comm_flistp, MSO_FLD_COMMISSION_CHARGE, ebufp);
		PIN_FLIST_FLD_PUT(rev_comm_flistp, MSO_FLD_COMMISSION_CHARGE, rev_comm_charge, ebufp);
		PIN_FLIST_FLD_SET(rev_comm_flistp, PIN_FLD_STATUS_MSG, "Commission Reversed Due to DTR Wrong Tagging", ebufp);

                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "bu_change_revert_dtr_commission: Create input flist", rev_comm_flistp);
                PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, rev_comm_flistp, &r_rev_comm_flistp, ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp)) {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "bu_change_revert_dtr_commission: Create error",ebufp);
                        return;
                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "bu_change_revert_dtr_commission: Create output flist", r_rev_comm_flistp);
		PIN_FLIST_DESTROY_EX(&rev_comm_flistp,ebufp);
		PIN_FLIST_DESTROY_EX(&r_rev_comm_flistp,ebufp);
		
		/* creating commision object after adjustment */ 
		PIN_FLIST_DESTROY_EX(&adj_evt_o_flistp,ebufp);
		PIN_FLIST_DESTROY_EX(&adj_evt_i_flistp,ebufp);
		PIN_FLIST_DESTROY_EX(&wrtflds_iflistp,ebufp);
		PIN_FLIST_DESTROY_EX(&wrtflds_oflistp,ebufp);
	}
}

static void 
fm_mso_commission_get_sdt_dt_commission_charges(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **all_comm_flistp,
        pin_flist_t             **err_flistp,
	int32			child_acnt_type,
	int32			parent_acnt_type,
	pin_errbuf_t            *ebufp)
{
	//poid_t		*cust_acct_objp = NULL;
	poid_t		*old_lco_objp  = NULL;
	poid_t		*new_lco_objp   = NULL;
	poid_t		*s_pdp          = NULL;
	poid_t		*new_sdt_objp   = NULL;
	poid_t		*old_sdt_objp   = NULL;
	poid_t		*new_dt_objp   = NULL;
	poid_t		*old_dt_objp   = NULL;

	//pin_flist_t	*acct_flistp    = NULL;
	pin_flist_t	*s_flistp       = NULL;
	pin_flist_t	*args_flistp    = NULL;
	pin_flist_t	*r_s_flistp     = NULL;
	pin_flist_t	*data_array     = NULL;
	pin_flist_t	*profile_info   = NULL;
	pin_flist_t	*lco_comm_flistp = NULL;
	pin_flist_t	*sdt_comm_flistp = NULL;
	pin_flist_t	*dtr_comm_flistp = NULL;
	pin_flist_t	*res_flist = NULL;
//	pin_flist_t 	*src_res_flistp = NULL;
	char		*template = "select X from /mso_commission_rpt where F1 = V1 and F2 = V2 and F3 != V3";

	int32		sflag = 256;
	int32		status = 999;
	int32		old_sdt_is_valid =0;
//	int32		acnt_type_lco = ACCT_TYPE_LCO;
//	int32		acnt_type_sdt = ACCT_TYPE_SUB_DTR;
//	int32		acnt_type_dtr = ACCT_TYPE_DTR;
	int32		sdt_is_different = 0;
	int32		dt_is_different  = 0;
	int32		elem_id = 0;
	int32		*comm_for = NULL;
	int64		db = 1;
	pin_cookie_t	cookie = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_get_sdt_dt_commission_charges input flist", i_flistp);

        /*: Get both LCO accounts START*/
//        acct_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_ACCOUNTS, 0, 0, ebufp);
//        curr_lco_objp = PIN_FLIST_FLD_GET(acct_flistp, PIN_FLD_POID, 0, ebufp);
//        acct_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_ACCOUNTS, 1, 0, ebufp);
//        new_lco_objp = PIN_FLIST_FLD_GET(acct_flistp, PIN_FLD_POID, 0, ebufp);
//
	/*************************************
	NEW SDT/DT
	**************************************/
	data_array = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_DATA_ARRAY, 1, 1, ebufp );
	if (data_array)
	{
		profile_info = PIN_FLIST_SUBSTR_GET(data_array, MSO_FLD_WHOLESALE_INFO, 1, ebufp );

		if (profile_info)
		{
			new_lco_objp = PIN_FLIST_FLD_GET(profile_info, MSO_FLD_LCO_OBJ, 1, ebufp);
			new_sdt_objp = PIN_FLIST_FLD_GET(profile_info, MSO_FLD_SDT_OBJ, 1, ebufp);
			new_dt_objp  = PIN_FLIST_FLD_GET(profile_info, MSO_FLD_DT_OBJ, 1, ebufp);
		}
	}

	/*************************************
	OLD SDT/DT
	**************************************/
	data_array = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_DATA_ARRAY, 0, 1, ebufp );
	if (data_array)
	{
		profile_info = PIN_FLIST_SUBSTR_GET(data_array, MSO_FLD_WHOLESALE_INFO, 1, ebufp );

		if (profile_info)
		{
			old_lco_objp = PIN_FLIST_FLD_GET(profile_info, MSO_FLD_LCO_OBJ, 1, ebufp);
			old_sdt_objp = PIN_FLIST_FLD_GET(profile_info, MSO_FLD_SDT_OBJ, 1, ebufp);
			old_dt_objp  = PIN_FLIST_FLD_GET(profile_info, MSO_FLD_DT_OBJ, 1, ebufp);
		}
	}

	if (new_sdt_objp && old_sdt_objp && PIN_POID_COMPARE(new_sdt_objp, old_sdt_objp, 0, ebufp)!=0 )
	{
		sdt_is_different = 1	;
	}
	if (new_dt_objp && old_dt_objp && PIN_POID_COMPARE(new_dt_objp, old_dt_objp, 0, ebufp)!=0 )
	{
		dt_is_different = 1	;
	}

	if ( (!new_sdt_objp || PIN_POID_GET_ID(new_sdt_objp) <=0) &&   // For valid old SDT and  NULL new SDT
	     (old_sdt_objp && PIN_POID_GET_ID(old_sdt_objp) >0)	       //
	   )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "old_sdt_is_valid");
		old_sdt_is_valid = 1;
	}

        PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Source (old) LCO Account POID", old_lco_objp);
        PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Target (new) LCO Account POID", new_lco_objp);
        PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Source (old) SDT Account POID", old_sdt_objp);
        PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Target (new) SDT Account POID", new_sdt_objp);
        PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Source (old) DT Account POID", old_dt_objp);
        PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Target (new) DT Account POID", new_dt_objp);

	/*: Get both LCO accounts END*/


        /*: Search for old LCO Commission Charges START*/
        s_flistp = PIN_FLIST_CREATE(ebufp);
        s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
        PIN_FLIST_FLD_PUT(s_flistp,PIN_FLD_POID, s_pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_FLAGS, (void*)&sflag, ebufp);
        PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_TEMPLATE, template, ebufp);

	if (parent_acnt_type == ACCT_TYPE_SUB_DTR )
	{
		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(args_flistp,MSO_FLD_SDT_OBJ, old_sdt_objp, ebufp);
	}
	if (parent_acnt_type == ACCT_TYPE_DTR )
	{
		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(args_flistp,MSO_FLD_DT_OBJ, old_dt_objp, ebufp);
	}
	
	if (child_acnt_type == ACCT_TYPE_LCO)
	{
		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_LCO_OBJ, old_lco_objp, ebufp);
	}
	if (child_acnt_type == ACCT_TYPE_SUB_DTR)
	{
		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_SDT_OBJ, old_sdt_objp, ebufp);
	}

//	if (flags  == 0 )  //LCO
//	{
//		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
//		PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_COMMISSION_FOR, &acnt_type_lco, ebufp);
//	}

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &status, ebufp);

        PIN_FLIST_ELEM_SET(s_flistp, NULL, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_get_sdt_dt_commission_charges get old commission search flist", s_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_s_flistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_commission_get_sdt_dt_commission_charges get old commissions search error",ebufp);
                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_get_sdt_dt_commission_charges get old commissions output flist", r_s_flistp);
        /*: Search for old LCO Commission Charges END*/


	/******************************************************************
	Browse through results to seggrigate LCO/SDT/DT  mso_commission_rpt
	******************************************************************/
	*all_comm_flistp = PIN_FLIST_CREATE(ebufp);
	lco_comm_flistp = PIN_FLIST_ELEM_ADD(*all_comm_flistp, MSO_FLD_LCOINFO, 0, ebufp);
	sdt_comm_flistp = PIN_FLIST_ELEM_ADD(*all_comm_flistp, MSO_FLD_SDTINFO, 0, ebufp);
	dtr_comm_flistp = PIN_FLIST_ELEM_ADD(*all_comm_flistp, MSO_FLD_DTRINFO, 0, ebufp);

 	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "all_comm_flistp", *all_comm_flistp);


	if (r_s_flistp)
	{
		while ((res_flist = PIN_FLIST_ELEM_GET_NEXT(r_s_flistp, PIN_FLD_RESULTS, 
			&elem_id, 1, &cookie, ebufp )) != NULL) 
		{
			comm_for = PIN_FLIST_FLD_GET(res_flist, MSO_FLD_COMMISSION_FOR, 1, ebufp);

			if (comm_for && *comm_for == ACCT_TYPE_SUB_DTR && 
			    (sdt_is_different || old_sdt_is_valid )&& 
			    child_acnt_type   !=  ACCT_TYPE_SUB_DTR /*&&
			    parent_acnt_type  == ACCT_TYPE_SUB_DTR*/
			   )
			{

				PIN_FLIST_ELEM_COPY(r_s_flistp, PIN_FLD_RESULTS, elem_id, sdt_comm_flistp, PIN_FLD_RESULTS, elem_id, ebufp);
			}
			else if (comm_for && *comm_for == ACCT_TYPE_DTR && 
			         dt_is_different 
			        )
			{
				PIN_FLIST_ELEM_COPY(r_s_flistp, PIN_FLD_RESULTS, elem_id, dtr_comm_flistp, PIN_FLD_RESULTS, elem_id, ebufp);
			}
		}
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "all_comm_flistp", *all_comm_flistp);



	/*Copy the result flist to return flist*/
	//*all_comm_flistp = PIN_FLIST_COPY(r_s_flistp, ebufp);

	/*: Cleanup Memory START*/
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_s_flistp, NULL);
	/*: Cleanup Memory END*/
}


static void
bu_change_apply_commission_for_new_sdt(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             *old_sdt_comm_flistp,
        pin_flist_t             **err_flistp,
        pin_errbuf_t            *ebufp)
{
//	pin_flist_t		*acct_flistp = NULL;
	pin_flist_t		*old_comm_flistp = NULL;
	pin_flist_t		*new_comm_flistp = NULL;
	pin_flist_t		*r_new_comm_flistp = NULL;
	pin_flist_t		*sdt_flistp = NULL;
	pin_flist_t		*charge_flistp = NULL;
	pin_flist_t		*data_array = NULL;
	pin_flist_t		*profile_info = NULL;

	poid_t			*new_lco_objp = NULL;
	poid_t			*new_sdt_objp = NULL;
	poid_t			*new_dtr_objp = NULL;
	pin_cookie_t		cookie = NULL;
	int32			elem_id = 0;
//	int32			ret_val = 0;

//	poid_t			*sdt_acct_objp = NULL;
	poid_t			*sdt_comm_grp_objp = NULL;
	poid_t			*sdt_outcollect_serv_objp = NULL;
	int32			*commission_model = NULL;
	int32			*commission_service = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "bu_change_apply_commission_for_new_sdt input1 flist", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "bu_change_apply_commission_for_new_sdt input2 flist", old_sdt_comm_flistp);

	/*: Get the new sdt account poid START*/
//	acct_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_ACCOUNTS, 1, 0, ebufp);
//	new_sdt_objp = PIN_FLIST_FLD_GET(acct_flistp, PIN_FLD_POID, 0, ebufp);
	data_array = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_DATA_ARRAY, 1, 1, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "data_array", data_array);

	if (data_array)
	{
		profile_info = PIN_FLIST_SUBSTR_GET(data_array, PIN_FLD_CUSTOMER_CARE_INFO, 1, ebufp);
		if (!profile_info)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "1111");
			profile_info = PIN_FLIST_SUBSTR_GET(data_array, MSO_FLD_WHOLESALE_INFO, 1, ebufp);
		}

		if (profile_info)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "2222");
			new_lco_objp = PIN_FLIST_FLD_GET(profile_info, MSO_FLD_LCO_OBJ, 1, ebufp);
			new_sdt_objp = PIN_FLIST_FLD_GET(profile_info, MSO_FLD_SDT_OBJ, 1, ebufp);
			new_dtr_objp = PIN_FLIST_FLD_GET(profile_info, MSO_FLD_DT_OBJ, 1, ebufp);

			PIN_FLIST_FLD_COPY(profile_info, MSO_FLD_SDT_OBJ, i_flistp, MSO_FLD_SDT_OBJ, ebufp);
			PIN_FLIST_FLD_COPY(profile_info, MSO_FLD_DT_OBJ, i_flistp, MSO_FLD_DT_OBJ, ebufp);

			if (!new_sdt_objp || PIN_POID_GET_ID(new_sdt_objp) <=0 )
			{
				return;
			}
		}
	}
	/*: Get the new sdt account poid END*/

	/*: Get the commission model of the new sdt account START*/
	if (fm_mso_get_bu_commission_info (ctxp, i_flistp, &sdt_flistp, ACCT_TYPE_SUB_DTR, ebufp) != 1) 
	{
		commission_model = PIN_FLIST_FLD_GET(sdt_flistp, MSO_FLD_COMMISION_MODEL, 0, ebufp);
		commission_service = PIN_FLIST_FLD_GET(sdt_flistp, MSO_FLD_COMMISION_SERVICE, 0, ebufp);
	}
	else 
	{
		PIN_ERRBUF_CLEAR(ebufp);
		*err_flistp = PIN_FLIST_COPY(sdt_flistp, ebufp);
	}
	/*: Get the commission model of the new sdt account END*/

	/*: Verify the new LCO's commission bucket START*/
	fm_mso_utils_get_bal_grp(ctxp, new_sdt_objp, COMMISSION_BUCKET, &sdt_comm_grp_objp, &sdt_outcollect_serv_objp, ebufp);
	if (sdt_comm_grp_objp == NULL) 
	{
		fm_mso_utils_prepare_error_flist(ctxp, sdt_flistp, err_flistp, "61053", "COMMISSION - Commission Bucket not found",ebufp);
		fm_mso_commission_record_failure (ctxp, "/event/billing/settlement/ws/outcollect/charge_head_based", i_flistp, *err_flistp, ebufp);
		pin_set_err(ebufp, PIN_ERRLOC_FM_COMM, PIN_ERRCLASS_APPLICATION, MSO_ERR_LCO_COMM_BUCKET_NOT_FOUND, 0, 0, 0);
		return;
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "SDT Commission Balance Group Buckets Retrieved");
	/*: Verify the new LCO's commission bucket END*/

//	/*: Verify the new LCO's commission bucket END*/
//	ret_val = fm_mso_commission_acct_has_agreement(ctxp,  new_sdt_objp, ebufp);
//	if (ret_val == 0)  // Not having active agreement
//	{ 
//		if (commission_model != NULL && *commission_model != 0) 
//		{
//			fm_mso_utils_prepare_error_flist(ctxp, sdt_flistp, err_flistp, "61054", "COMMISSION - New sdt has no active agreement",ebufp);
//			fm_mso_commission_record_failure(ctxp, "/event/billing/settlement/ws/outcollect/charge_head_based", i_flistp, *err_flistp, ebufp);
//			pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, MSO_ERR_LCO_NO_ACTIVE_AGREEMENT, 0, 0, 0);
//		}
//		else 
//		{
//			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
//				"COMMISSION - No Agreement for this new LCO Account, hence skipping the commission");
//			*err_flistp = NULL;
//		}
//		return;
//	}
//	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "LCO Commission Agreement Retrieved");
//	/*: Verify the new LCO's commission bucket END*/

	
	while ((old_comm_flistp = PIN_FLIST_ELEM_GET_NEXT(old_sdt_comm_flistp, PIN_FLD_RESULTS, 
		&elem_id, 1, &cookie, ebufp )) != NULL) 
	{

		/*Prepare the commission flist START*/
		new_comm_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(old_comm_flistp, PIN_FLD_EVENT_OBJ, new_comm_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(old_comm_flistp, MSO_FLD_COMMISSION_RULE, new_comm_flistp, MSO_FLD_COMMISSION_RULE, ebufp);
		PIN_FLIST_FLD_COPY(old_comm_flistp, PIN_FLD_ACCOUNT_OBJ, new_comm_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(old_comm_flistp, PIN_FLD_EFFECTIVE_T, new_comm_flistp, PIN_FLD_EFFECTIVE_T, ebufp);
		PIN_FLIST_FLD_COPY(old_comm_flistp, PIN_FLD_EVENT_OBJ, new_comm_flistp, PIN_FLD_EVENT_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(old_comm_flistp, PIN_FLD_SESSION_OBJ, new_comm_flistp, PIN_FLD_SESSION_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(old_comm_flistp, PIN_FLD_STATUS, new_comm_flistp, PIN_FLD_STATUS, ebufp);
		PIN_FLIST_FLD_COPY(old_comm_flistp, PIN_FLD_STATUS_MSG, new_comm_flistp, PIN_FLD_STATUS_MSG, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, new_comm_flistp, PIN_FLD_PROGRAM_NAME, ebufp);

		PIN_FLIST_FLD_SET(new_comm_flistp, MSO_FLD_LCO_OBJ, new_lco_objp, ebufp);
		PIN_FLIST_FLD_SET(new_comm_flistp, MSO_FLD_SDT_OBJ, new_sdt_objp, ebufp);
		PIN_FLIST_FLD_SET(new_comm_flistp, MSO_FLD_DT_OBJ, new_dtr_objp, ebufp);

		PIN_FLIST_FLD_SET(new_comm_flistp, MSO_FLD_SDT_COMM_BUCKET_OBJ, sdt_comm_grp_objp, ebufp);
		PIN_FLIST_FLD_SET(new_comm_flistp, MSO_FLD_SDT_OUTCOLLECT_SERV_OBJ, sdt_outcollect_serv_objp, ebufp);
		
		charge_flistp = PIN_FLIST_SUBSTR_GET(old_comm_flistp, MSO_FLD_CHARGE_HEAD_INFO, 1, ebufp);
		PIN_FLIST_SUBSTR_SET(new_comm_flistp, charge_flistp, MSO_FLD_CHARGE_HEAD_INFO, ebufp);
		/*Prepare the commission flist END*/

		PCM_OP(ctxp,MSO_OP_COMMISSION_PROCESS_COMMISSION, SDT_COMMISSION, new_comm_flistp, &r_new_comm_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"bu_change_apply_commission_for_new_sdt: Process Commission opcode error", ebufp);
			return;
		}

		PIN_FLIST_DESTROY_EX(&new_comm_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&r_new_comm_flistp, NULL);
		
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "bu_change_apply_commission_for_new_sdt output flist", i_flistp);

}

static void
bu_change_apply_commission_for_new_dtr(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             *old_dtr_comm_flistp,
        pin_flist_t             **err_flistp,
        pin_errbuf_t            *ebufp)
{
//	pin_flist_t		*acct_flistp = NULL;
	pin_flist_t		*old_comm_flistp = NULL;
	pin_flist_t		*new_comm_flistp = NULL;
	pin_flist_t		*r_new_comm_flistp = NULL;
	pin_flist_t		*dtr_flistp = NULL;
	pin_flist_t		*charge_flistp = NULL;
	pin_flist_t		*data_array = NULL;
	pin_flist_t		*profile_info = NULL;

	poid_t			*new_lco_objp = NULL;
	poid_t			*new_sdt_objp = NULL;
	poid_t			*new_dtr_objp = NULL;
	pin_cookie_t		cookie = NULL;
	int32			elem_id = 0;
//	int32			ret_val = 0;

//	poid_t			*dtr_acct_objp = NULL;
	poid_t			*dtr_comm_grp_objp = NULL;
	poid_t			*dtr_outcollect_serv_objp = NULL;
	int32			*commission_model = NULL;
	int32			*commission_service = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "bu_change_apply_commission_for_new_dtr input1 flist", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "bu_change_apply_commission_for_new_dtr input2 flist", old_dtr_comm_flistp);

	/*: Get the new dtr account poid START*/
//	acct_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_ACCOUNTS, 1, 0, ebufp);
//	new_dtr_objp = PIN_FLIST_FLD_GET(acct_flistp, PIN_FLD_POID, 0, ebufp);
	data_array = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_DATA_ARRAY, 1, 1, ebufp);
	if (data_array)
	{
		profile_info = PIN_FLIST_SUBSTR_GET(data_array, PIN_FLD_CUSTOMER_CARE_INFO, 1, ebufp);
		if (!profile_info)
		{
			profile_info = PIN_FLIST_SUBSTR_GET(data_array, MSO_FLD_WHOLESALE_INFO, 1, ebufp);
		}

		if (profile_info)
		{
			new_lco_objp = PIN_FLIST_FLD_GET(profile_info, MSO_FLD_LCO_OBJ, 1, ebufp);
			new_sdt_objp = PIN_FLIST_FLD_GET(profile_info, MSO_FLD_SDT_OBJ, 1, ebufp);
			new_dtr_objp = PIN_FLIST_FLD_GET(profile_info, MSO_FLD_DT_OBJ, 1, ebufp);

			PIN_FLIST_FLD_COPY(profile_info, MSO_FLD_SDT_OBJ, i_flistp, MSO_FLD_SDT_OBJ, ebufp);
			PIN_FLIST_FLD_COPY(profile_info, MSO_FLD_DT_OBJ, i_flistp, MSO_FLD_DT_OBJ, ebufp);
		}
	}
	/*: Get the new dtr account poid END*/

	/*: Get the commission model of the new dtr account START*/
	if (fm_mso_get_bu_commission_info (ctxp, i_flistp, &dtr_flistp, ACCT_TYPE_DTR, ebufp) != 1) 
	{
		commission_model = PIN_FLIST_FLD_GET(dtr_flistp, MSO_FLD_COMMISION_MODEL, 0, ebufp);
		commission_service = PIN_FLIST_FLD_GET(dtr_flistp, MSO_FLD_COMMISION_SERVICE, 0, ebufp);
	}
	else 
	{
		PIN_ERRBUF_CLEAR(ebufp);
		*err_flistp = PIN_FLIST_COPY(dtr_flistp, ebufp);
	}
	/*: Get the commission model of the new dtr account END*/

	/*: Verify the new LCO's commission bucket START*/
	fm_mso_utils_get_bal_grp(ctxp, new_dtr_objp, COMMISSION_BUCKET, &dtr_comm_grp_objp, &dtr_outcollect_serv_objp, ebufp);
	if (dtr_comm_grp_objp == NULL) 
	{
		fm_mso_utils_prepare_error_flist(ctxp, dtr_flistp, err_flistp, "61053", "COMMISSION - Commission Bucket not found",ebufp);
		fm_mso_commission_record_failure (ctxp, "/event/billing/settlement/ws/outcollect/charge_head_based", i_flistp, *err_flistp, ebufp);
		pin_set_err(ebufp, PIN_ERRLOC_FM_COMM, PIN_ERRCLASS_APPLICATION, MSO_ERR_LCO_COMM_BUCKET_NOT_FOUND, 0, 0, 0);
		return;
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "DTR Commission Balance Group Buckets Retrieved");
	/*: Verify the new LCO's commission bucket END*/

//	/*: Verify the new LCO's commission bucket END*/
//	ret_val = fm_mso_commission_acct_has_agreement(ctxp,  new_dtr_objp, ebufp);
//	if (ret_val == 0)  // Not having active agreement
//	{ 
//		if (commission_model != NULL && *commission_model != 0) 
//		{
//			fm_mso_utils_prepare_error_flist(ctxp, dtr_flistp, err_flistp, "61054", "COMMISSION - New dtr has no active agreement",ebufp);
//			fm_mso_commission_record_failure(ctxp, "/event/billing/settlement/ws/outcollect/charge_head_based", i_flistp, *err_flistp, ebufp);
//			pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, MSO_ERR_LCO_NO_ACTIVE_AGREEMENT, 0, 0, 0);
//		}
//		else 
//		{
//			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
//				"COMMISSION - No Agreement for this new LCO Account, hence skipping the commission");
//			*err_flistp = NULL;
//		}
//		return;
//	}
//	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "LCO Commission Agreement Retrieved");
//	/*: Verify the new LCO's commission bucket END*/

	
	while ((old_comm_flistp = PIN_FLIST_ELEM_GET_NEXT(old_dtr_comm_flistp, PIN_FLD_RESULTS, 
		&elem_id, 1, &cookie, ebufp )) != NULL) 
	{

		/*Prepare the commission flist START*/
		new_comm_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(old_comm_flistp, PIN_FLD_EVENT_OBJ, new_comm_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(old_comm_flistp, MSO_FLD_COMMISSION_RULE, new_comm_flistp, MSO_FLD_COMMISSION_RULE, ebufp);
		PIN_FLIST_FLD_COPY(old_comm_flistp, PIN_FLD_ACCOUNT_OBJ, new_comm_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(old_comm_flistp, PIN_FLD_EFFECTIVE_T, new_comm_flistp, PIN_FLD_EFFECTIVE_T, ebufp);
		PIN_FLIST_FLD_COPY(old_comm_flistp, PIN_FLD_EVENT_OBJ, new_comm_flistp, PIN_FLD_EVENT_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(old_comm_flistp, PIN_FLD_SESSION_OBJ, new_comm_flistp, PIN_FLD_SESSION_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(old_comm_flistp, PIN_FLD_STATUS, new_comm_flistp, PIN_FLD_STATUS, ebufp);
		PIN_FLIST_FLD_COPY(old_comm_flistp, PIN_FLD_STATUS_MSG, new_comm_flistp, PIN_FLD_STATUS_MSG, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, new_comm_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
		
		PIN_FLIST_FLD_SET(new_comm_flistp, MSO_FLD_LCO_OBJ, new_lco_objp, ebufp);
		PIN_FLIST_FLD_SET(new_comm_flistp, MSO_FLD_SDT_OBJ, new_sdt_objp, ebufp);
		PIN_FLIST_FLD_SET(new_comm_flistp, MSO_FLD_DT_OBJ,  new_dtr_objp, ebufp);
		PIN_FLIST_FLD_SET(new_comm_flistp, MSO_FLD_DTR_COMM_BUCKET_OBJ, dtr_comm_grp_objp, ebufp);
		PIN_FLIST_FLD_SET(new_comm_flistp, MSO_FLD_DTR_OUTCOLLECT_SERV_OBJ, dtr_outcollect_serv_objp, ebufp);
		charge_flistp = PIN_FLIST_SUBSTR_GET(old_comm_flistp, MSO_FLD_CHARGE_HEAD_INFO, 1, ebufp);
		PIN_FLIST_SUBSTR_SET(new_comm_flistp, charge_flistp, MSO_FLD_CHARGE_HEAD_INFO, ebufp);
		/*Prepare the commission flist END*/

		PCM_OP(ctxp,MSO_OP_COMMISSION_PROCESS_COMMISSION, DTR_COMMISSION, new_comm_flistp, &r_new_comm_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"bu_change_apply_commission_for_new_dtr: Process Commission opcode error", ebufp);
			return;
		}

		PIN_FLIST_DESTROY_EX(&new_comm_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&r_new_comm_flistp, NULL);
		
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "bu_change_apply_commission_for_new_dtr output flist", i_flistp);

}
