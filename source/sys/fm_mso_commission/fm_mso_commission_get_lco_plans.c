
/*******************************************************************
 * File:	fm_mso_commission_get_lco_plans.c
 * Opcode:	MSO_OP_COMMISSION_GET_LCO_PLANS 
 * Description: 
 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_commission_get_lco_plans.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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
#include "mso_ops_flds.h"

/*******************************************************************
 * MSO_OP_COMMISSION_GET_LCO_PLANS 
 *
 * This policy is used to get the LCO plans and its associated deals
 *******************************************************************/

EXPORT_OP void
op_mso_commission_get_lco_plans(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_commission_get_lco_plans(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * MSO_OP_COMMISSION_GET_LCO_PLANS  
 *******************************************************************/
void 
op_mso_commission_get_lco_plans(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	*r_flistpp		= NULL;
	//poid_t			*inp_pdp = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_COMMISSION_GET_LCO_PLANS) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_commission_get_lco_plans error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_commission_get_lco_plans input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	fm_mso_commission_get_lco_plans(ctxp, flags, i_flistp, r_flistpp, ebufp);
 	/***********************************************************
	 * Results.
	 ***********************************************************/

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_commission_get_lco_plans error", ebufp);
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_commission_get_lco_plans output flist", *r_flistpp);
	}
	return;
}


/*******************************************************************
 * Function: fm_mso_commission_get_lco_plans
 *
 * This function is used to get the LCO plans. Function expects a 
 * Dummy poid and function itself will prepare the search template 
 * to retrieve the LCO plans.
 *
 * Arguments:
 *      1. Context Pointer
 *      2. Flags
 *      3. Input Flist
 *      4. Output Flist [Includes List of Plans->Services->Deals]
 *      5. Error Buffer
 *  
 * Return value: None
 *******************************************************************/

static void 
fm_mso_commission_get_lco_plans(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*services_array = NULL;

	poid_t			*service_pdp = NULL;
	poid_t			*srch_pdp = NULL;
	poid_t			*plan_pdp = NULL;
	//char			*srch_val = NULL;
	char			template[100]="select X from /plan where F1.type = V1 " ;
	int64			db = -1;
	int32			srch_flag = 256;
	//int32			srch_key  = -1;
	//int32			srch_type = -1;
	//int32			*srch_type_ptr = NULL;
	//time_t			*start_t = NULL;
	//time_t			*end_t = NULL;
	int32			status_success =0;
	int32			status_fail =1;
	//int32			*service_type = NULL;
	//int32			*plan_type = NULL;
	//pin_decimal_t		*plan_sub_type = 0;

	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_get_lco_plans input flist", i_flistp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_commission_get_lco_plans", ebufp);
		goto CLEANUP;
	}
	plan_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	db = PIN_POID_GET_DB(plan_pdp);

	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp);
	services_array = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_SERVICES, 0, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template, ebufp);

	service_pdp = PIN_POID_FROM_STR("0.0.0.1 /service/settlement/ws/outcollect -1", NULL, ebufp);
	PIN_FLIST_FLD_PUT(services_array,PIN_FLD_SERVICE_OBJ, service_pdp, ebufp);

	
	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_CODE, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_NAME, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_DESCR, NULL, ebufp);

	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_LIMIT, NULL, ebufp);
	services_array = PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_SERVICES, 0, ebufp);
	PIN_FLIST_FLD_SET(services_array, PIN_FLD_DEAL_OBJ, NULL, ebufp); 
	PIN_FLIST_FLD_SET(services_array, PIN_FLD_SERVICE_OBJ, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_get_lco_plans search flist", srch_flistp);
	
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		return;
	}

        if ((PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp)))
        {
                PIN_FLIST_FLD_SET(srch_out_flistp, PIN_FLD_STATUS, &status_success, ebufp);
        }
        else
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"No Records found, try after changing search criteria", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_FLD_SET(srch_out_flistp, PIN_FLD_STATUS, &status_fail, ebufp);
                PIN_FLIST_FLD_SET(srch_out_flistp, PIN_FLD_ERROR_CODE, "51130", ebufp);
                PIN_FLIST_FLD_SET(srch_out_flistp, PIN_FLD_ERROR_DESCR, "No Records found.", ebufp);
                goto CLEANUP;
        }


CLEANUP:
	*r_flistpp = srch_out_flistp;
	return;
}

