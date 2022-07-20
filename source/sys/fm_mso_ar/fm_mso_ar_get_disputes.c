/*******************************************************************
 * File:	fm_mso_ar_get_disputes.c
 * Opcode:	MSO_OP_AR_GET_DISPUTES
 * Owner:	Vilva Sabarikanth
 * Created:	05/12/2013
 * Description: 
// Document History
//
// DATE         /  CHANGE                                       /  Author
//05/12/2013    / Initial Version                               /Vilva Sabarikanth
//17-Oct-2014   / Added BB Customzations and Error Handling     / Harish Kulkarni
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Podrtal Version: fm_mso_ar_get_disputes.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "ops/act.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "pcm_ops.h"
#include "mso_ops_flds.h"
#include "ops/ar.h"
#include "ops/bill.h"


/**************************************
* External Functions start
***************************************/
/**************************************
* External Functions end
***************************************/

 
/**************************************
* Local Functions start
***************************************/


/*******************************************************************
 * MSO_OP_AR_GET_DISPUTES 
 *******************************************************************/

EXPORT_OP void
mso_op_ar_get_disputes(
	cm_nap_connection_t	*connp,
	int32				opcode,
	int32				flags,
	pin_flist_t			*i_flistp,
	pin_flist_t			**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
mso_ar_get_disputes(
	pcm_context_t		*ctxp,
	int32				flags,
	pin_flist_t			*i_flistp,
	pin_flist_t			**r_flistpp,
	pin_errbuf_t		*ebufp);
/*
static void
mso_retrieve_ar_billinfo(
	pcm_context_t		*ctxp,
	pin_flist_t			*i_flistp,
	pin_flist_t			**r_flistpp,
	pin_errbuf_t		*ebufp);
*/
static void 
fm_mso_srch_dispute_details(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void 
fm_mso_srch_settlement_details(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

PIN_IMPORT
void
fm_mso_utils_read_any_object(
	pcm_context_t           *ctxp,
	poid_t                  *objp,
	pin_flist_t             **out_flistpp,
	pin_errbuf_t            *ebufp);

/*******************************************************************
 * MSO_OP_AR_GET_DISPUTES  
 *******************************************************************/
void 
mso_op_ar_get_disputes(
	cm_nap_connection_t	*connp,
	int32				opcode,
	int32				flags,
	pin_flist_t			*i_flistp,
	pin_flist_t			**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	pin_flist_t		*err_flistp = NULL;
	int				status = 0;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) 
		return;	

	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_AR_GET_DISPUTES) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_ar_get_details error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_ar_get_details input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	mso_ar_get_disputes(ctxp, flags, i_flistp, r_flistpp, ebufp);

	err_flistp = PIN_FLIST_CREATE(ebufp);
 	/***********************************************************
	 * Results.
	 ***********************************************************/
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{	
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in Get Disputes");
		PIN_ERRBUF_CLEAR(ebufp);
		status = 1;
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, err_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53085", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Error in Get Disputes", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		*r_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		goto Cleanup;
	}
	else
	{
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"mso_ar_get_disputes output flist", *r_flistpp);
	}
Cleanup:
	PIN_FLIST_DESTROY_EX(&err_flistp, ebufp);
	return;
}

/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
mso_ar_get_disputes(
	pcm_context_t		*ctxp,
	int32				flags,
	pin_flist_t			*i_flistp,
	pin_flist_t			**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	poid_t			*billinfo_pdp = NULL;
	pin_flist_t		*flistp = NULL;
	pin_flist_t		*b_flistp = NULL;
	pin_flist_t		*err_flistp = NULL;
	pin_flist_t		*get_flistp = NULL;
	pin_flist_t		*get_oflistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	int				status = 0;
	int				children = 0;
	int32			*flag = NULL;

	b_flistp = PIN_FLIST_CREATE(ebufp);
	get_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "get ar billinfo poid read input list", i_flistp);

	// Get the AR Billinfo details
	fm_mso_utils_read_any_object(ctxp, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp), 
				&b_flistp, ebufp);
	//mso_retrieve_ar_billinfo(ctxp, i_flistp, &b_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "get ar billinfo poid read output list", b_flistp);
	billinfo_pdp = PIN_FLIST_FLD_GET(b_flistp, PIN_FLD_AR_BILLINFO_OBJ, 0, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{	
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "No Valid AR Billinfo Object Found");
		PIN_ERRBUF_CLEAR(ebufp);
		err_flistp = PIN_FLIST_CREATE(ebufp);
		status = 1;
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, err_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53086", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "No Valid AR Billinfo Object Found", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		*r_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		goto Cleanup;
	}
	
	flag = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_STATUS, 0, ebufp);
	//PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, get_flistp, PIN_FLD_AR_BILLINFO_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, get_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, get_flistp, PIN_FLD_BILL_OBJ, ebufp);
	PIN_FLIST_FLD_SET(get_flistp, PIN_FLD_AR_BILLINFO_OBJ, billinfo_pdp, ebufp);
	PIN_FLIST_FLD_SET(get_flistp, PIN_FLD_INCLUDE_CHILDREN, &children, ebufp);
	if (flag /*&& *flag!=0*/)
	{
		PIN_FLIST_FLD_SET(get_flistp, PIN_FLD_STATUS, flag, ebufp);
	}
	/*
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Disputes search input flist", get_flistp);
	PCM_OP(ctxp, PCM_OP_AR_GET_DISPUTES, 0, get_flistp, &get_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Disputes search output flist",get_oflistp);
	*/

	fm_mso_srch_dispute_details(ctxp, get_flistp, &get_oflistp, ebufp);


	// Check if there are any results. Return accordingly.
	r_flistp = PIN_FLIST_ELEM_GET(get_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
        if(!r_flistp)
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "No disputes found");
                PIN_ERRBUF_CLEAR(ebufp);
                err_flistp = PIN_FLIST_CREATE(ebufp);
                status = 1;
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, err_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53085", ebufp);
                PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "No disputes found", ebufp);
                PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
                *r_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
                goto Cleanup;
        }
        *r_flistpp = PIN_FLIST_COPY(get_oflistp, ebufp);

Cleanup:
	PIN_FLIST_DESTROY_EX(&b_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&get_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&get_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&err_flistp, ebufp);
	return;
}

/*********************************
* Get the AR Billinfo details
*********************************/
/*
static void 
mso_retrieve_ar_billinfo(
	pcm_context_t		*ctxp,
	pin_flist_t			*i_flistp,
	pin_flist_t			**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	poid_t			*billinfo_pdp = NULL;
	poid_t			*srch_pdp = NULL;
	pin_flist_t		*flistp = NULL;
	pin_flist_t		*srch_flistp = NULL;
	int64			db = 0;
	int32			srch_flag = 256;

	billinfo_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	db = PIN_POID_GET_DB(billinfo_pdp) ;
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, "select X from /billinfo where F1 = V1 and F2 = V2" , ebufp);
	flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, billinfo_pdp,ebufp);
	flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_AR_BILLINFO_OBJ, NULL, ebufp);
	//PIN_FLIST_ELEM_SET(srch_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "get ar billinfo poid search input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, r_flistpp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "get ar billinfo poid search output list", *r_flistpp);

	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	return;
}
*/




/*********************************
* fm_mso_srch_dispute_details()
*********************************/
static void 
fm_mso_srch_dispute_details(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*flistp = NULL;
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*res_flist   = NULL;
	pin_flist_t		*final_out_flist = NULL;
	pin_flist_t		*item_out_flist = NULL;
	pin_flist_t		*trf_info = NULL;
	pin_flist_t		*set_iflist = NULL;

	poid_t			*bill_pdp = NULL;
	poid_t			*srch_pdp = NULL;
	poid_t			*action_obj = NULL;

	int64			db = 0;
	int32			srch_flag = 256;
	int32			count = 0;
	int32			elem_id = 0;
	int32			elem_id1 = 0;
	int32			elem_id2 = 0;
	int32			srch_status = 0;
 	int32			*status = NULL;
	int32			not_settled = 2;
	int32			settled = 4;
	int32			disp_status = 0;

	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie1 = NULL;
	pin_cookie_t		cookie2 = NULL;
	pin_cookie_t		prev_cookie = NULL;

	void			*vp = NULL;



	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_dispute_details", i_flistp);
	bill_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BILL_OBJ, 0, ebufp);
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_STATUS, 0, ebufp);
	if (vp)
	{
		srch_status = *(int32*)vp;
	}

	/********************************************
	Search items of bill
	********************************************/
	db = PIN_POID_GET_DB(bill_pdp) ;
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, "select X from /item where F1 = V1 " , ebufp);
	
	flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_AR_BILL_OBJ, bill_pdp,ebufp);
	
	flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch items linked to bill iflist", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch items linked to bill iflist", srch_oflistp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

	/********************************************
	call PCM_OP_AR_GET_ITEM_DETAIL for each 
	no-ar item
	********************************************/
	while((res_flist = PIN_FLIST_ELEM_GET_NEXT(srch_oflistp, PIN_FLD_RESULTS,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		if (!final_out_flist)
		{
			final_out_flist = PIN_FLIST_CREATE(ebufp);
		}
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_AR_GET_ITEM_DETAIL input flist", res_flist );
		PCM_OP(ctxp, PCM_OP_AR_GET_ITEM_DETAIL, 0, res_flist, &item_out_flist, ebufp);
		//PIN_FLIST_DESTROY_EX(&res_flist, NULL);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_AR_GET_ITEM_DETAIL output flist", item_out_flist );

		elem_id1 = 0;
		cookie1  = NULL;
		while((vp = PIN_FLIST_ELEM_GET_NEXT(item_out_flist, PIN_FLD_RESULTS,
			&elem_id1, 1, &cookie1, ebufp)) != (pin_flist_t *)NULL)
		{
			status = PIN_FLIST_FLD_GET((pin_flist_t*)vp, PIN_FLD_STATUS, 1, ebufp);
			/*if (srch_status ==0 ||
			    status && *status == srch_status
			   )
			{
				PIN_FLIST_ELEM_COPY(item_out_flist, PIN_FLD_RESULTS, elem_id1, final_out_flist, PIN_FLD_RESULTS, count, ebufp);
				count++;
			}*/

			elem_id2 = 0;
			cookie2  = NULL;
			while((trf_info = PIN_FLIST_ELEM_GET_NEXT((pin_flist_t*)vp, PIN_FLD_TRANSFERS_INTO,
				&elem_id2, 1, &cookie2, ebufp)) != (pin_flist_t *)NULL)
			{
				action_obj = PIN_FLIST_FLD_GET(trf_info, PIN_FLD_RELATED_ACTION_ITEM_OBJ, 1, ebufp);
				if (action_obj && PIN_POID_GET_ID(action_obj) >0)
				{
					if (srch_status !=2 )//Searching non settled isputes 
					{
						set_iflist = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_COPY(trf_info, PIN_FLD_RELATED_ACTION_ITEM_OBJ, set_iflist, PIN_FLD_POID, ebufp);
						fm_mso_srch_settlement_details(ctxp, set_iflist, &trf_info, ebufp);
						PIN_FLIST_DESTROY_EX(&set_iflist, NULL);
					}
					PIN_FLIST_FLD_SET(trf_info, PIN_FLD_STATUS, &settled, ebufp);
					disp_status = settled;
				}
				else if (action_obj && PIN_POID_GET_ID(action_obj) == 0 )
				{
					PIN_FLIST_FLD_SET(trf_info, PIN_FLD_STATUS, &not_settled, ebufp);
					disp_status = not_settled;
				}

				/*****************************************************
				Dropping elements which are not matching the 
				input search criteria
				*****************************************************/
				prev_cookie = NULL;
				if ( (srch_status == not_settled && disp_status == settled ) ||
				     (srch_status == settled && disp_status == not_settled )
				   )
				{	
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Dropping elem ", trf_info);
					PIN_FLIST_ELEM_DROP((pin_flist_t*)vp, PIN_FLD_TRANSFERS_INTO, elem_id2, ebufp);
					cookie2 = prev_cookie;
				}
				prev_cookie = cookie2;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "item_out_flist after dropping ", item_out_flist);
			PIN_FLIST_ELEM_COPY(item_out_flist, PIN_FLD_RESULTS, elem_id1, final_out_flist, PIN_FLD_RESULTS, count, ebufp);
			count++;
		}


		PIN_FLIST_FLD_COPY(item_out_flist, PIN_FLD_POID, final_out_flist, PIN_FLD_POID, ebufp);
		PIN_FLIST_DESTROY_EX(&item_out_flist, NULL);
 	}

	/******************************************************************************************/

	*r_flistpp = final_out_flist;
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_dispute_details out flist ", *r_flistpp);

	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	return;
}



/*********************************
* fm_mso_srch_settlement_details()
*********************************/
static void 
fm_mso_srch_settlement_details(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*settlement_info = NULL;
	pin_flist_t		*set_oflistp = NULL;
	pin_flist_t		*res_flist   = NULL;
	pin_flist_t		*trf_out = NULL;
	pin_flist_t		*item_out_flist = NULL;

	void			*vp = NULL;




	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_settlement_details", i_flistp);
	
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
	if (vp )
	{
		PCM_OP(ctxp, PCM_OP_AR_GET_ITEM_DETAIL, 0, i_flistp, &set_oflistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_AR_GET_ITEM_DETAIL output flist", set_oflistp );
	}

	if (set_oflistp)
	{
		res_flist = PIN_FLIST_ELEM_GET(set_oflistp, PIN_FLD_RESULTS, 0, 1, ebufp);
		if (res_flist)
		{
			trf_out = PIN_FLIST_ELEM_GET(res_flist, PIN_FLD_TRANSFERS_OUT, 0, 1, ebufp);
			settlement_info = PIN_FLIST_ELEM_ADD(*r_flistpp, PIN_FLD_SETTLEMENTS, 0, ebufp);

			PIN_FLIST_FLD_COPY(trf_out,   PIN_FLD_POID,       settlement_info, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(trf_out,   PIN_FLD_SYS_DESCR,  settlement_info, PIN_FLD_SYS_DESCR, ebufp);
			PIN_FLIST_FLD_COPY(trf_out,   PIN_FLD_END_T,      settlement_info, PIN_FLD_END_T, ebufp);
			PIN_FLIST_FLD_COPY(res_flist, PIN_FLD_POID,       settlement_info, PIN_FLD_ITEM_OBJ, ebufp);
			PIN_FLIST_FLD_COPY(res_flist, PIN_FLD_ITEM_TOTAL, settlement_info, PIN_FLD_ITEM_TOTAL, ebufp);
			PIN_FLIST_FLD_COPY(res_flist, PIN_FLD_DISPUTED,   settlement_info, PIN_FLD_DISPUTED, ebufp);
			PIN_FLIST_FLD_COPY(res_flist, PIN_FLD_TRANSFERED, settlement_info, PIN_FLD_TRANSFERED, ebufp);
			PIN_FLIST_FLD_COPY(res_flist, PIN_FLD_DESCR,      settlement_info, PIN_FLD_DESCR, ebufp);
			PIN_FLIST_FLD_COPY(res_flist, PIN_FLD_EVENT_OBJ,  settlement_info, PIN_FLD_EVENT_OBJ, ebufp);
			PIN_FLIST_FLD_COPY(res_flist, PIN_FLD_ITEM_NO,    settlement_info, PIN_FLD_ITEM_NO, ebufp);
		}
	}


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_settlement_details mofified output", *r_flistpp);

	PIN_FLIST_DESTROY_EX(&set_oflistp, NULL);
	return;
}





