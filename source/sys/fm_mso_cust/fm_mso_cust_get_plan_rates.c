/*******************************************************************
 * File:	fm_mso_cust_get_plan_rates.c
 * Opcode:	MSO_OP_CUST_GET_PLAN_RATES 
 * Owner:	Nagaraj
 * Created:	22-AUG-2014
 * Description: 
 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_get_plan_rates.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "ops/act.h"
#include "mso_cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"


/*******************************************************************
 * MSO_OP_CUST_GET_PLAN_RATES 
 *
 * 
 *******************************************************************/

EXPORT_OP void
op_mso_cust_get_plan_rates(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_get_plan_rates(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);


void prepare_flist_for_search(double prod_priority, 
								pin_flist_t	*srch_plan_flistp, 
								pin_errbuf_t		*ebufp);


pin_decimal_t *get_plan_charge(pcm_context_t		*ctxp,
								char *plan_name, 
								char *city, 
								pin_errbuf_t		*ebufp);

pin_decimal_t *extract_plan_charge(pcm_context_t		*ctxp,
					pin_flist_t	*plan_info_flistp,
					char *city, 
					pin_errbuf_t		*ebufp) ;
/*******************************************************************
 * MSO_OP_CUST_GET_PLAN_RATES  
 *******************************************************************/
void 
op_mso_cust_get_plan_rates(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	*r_flistpp		= NULL;
	poid_t			*inp_pdp = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_GET_PLAN_RATES) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_get_plan_rates error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_get_plan_rates input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	fm_mso_cust_get_plan_rates(ctxp, flags, i_flistp, r_flistpp, ebufp);
 	/***********************************************************
	 * Results.
	 ***********************************************************/

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_get_plan_rates error", ebufp);
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_get_plan_rates output flist", *r_flistpp);
	}
	return;
}


/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_get_plan_rates(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{

	poid_t			*ppdp = NULL;
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*srch_plan_flistp = NULL;
	pin_flist_t		*plan_flistp = NULL;
	pin_flist_t		*srch_plan_out_flistp = NULL;
	pin_flist_t		*add_plan_flistp = NULL;
	int64			db = 1;
	char			all_plan_name[3] = "%";
	char			*city = NULL;
	int32			plan_srch_key = 0;
	int32			plan_srch_type = 3;
	int32			svc_type = 2;
	int32			plan_type = 2;
	pin_decimal_t	*prod_priority = NULL;
	double			prod_priority_double = 0.0;
	char			zero[5] = "0.0";
	pin_decimal_t	*srch_plan_charge = NULL;
	pin_decimal_t	*plan_charge = NULL;
	int32			elem_id_1 = 0;
	int32			index = 0;
	pin_cookie_t	cookie_1 = NULL;
	char			*plan_city = NULL;
	char			*plan_name = NULL;
	char			*srch_plan_name = NULL;
	int32			payterm = 0;

	/*
	Algorithm:
	1. Get the input parameters and search the plan and get the plan charge applicable to city/payterm and plan name.
	2. Search other plans matching the criteria of the current plan using PCM_OP_MSO_CUST_GET_PLAN_DETAILS opcode.
	3. Check the charge for each of the plans based on the city and payterm passed.
	4. If the charge is higher than the current plan charge, add it to return flist.
	*/

	plan_name = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_PLAN_NAME, 0, ebufp);
	plan_city = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CITY, 0, ebufp);
	payterm  = *(int32 *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);

	plan_charge = get_plan_charge(ctxp, plan_name, plan_city, ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "average monthly charge for the current plan is ");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, pbo_decimal_to_str(plan_charge, ebufp));

	ppdp = PIN_POID_CREATE(db, "/plan", -1, ebufp);
	srch_plan_flistp = PIN_FLIST_CREATE(ebufp);
	
	PIN_FLIST_FLD_PUT(srch_plan_flistp, PIN_FLD_POID, ppdp, ebufp);
	PIN_FLIST_FLD_SET(srch_plan_flistp, PIN_FLD_SEARCH_KEY, &plan_srch_key, ebufp);	
	PIN_FLIST_FLD_SET(srch_plan_flistp, PIN_FLD_SEARCH_TYPE, &plan_srch_type, ebufp);
	PIN_FLIST_FLD_SET(srch_plan_flistp, MSO_FLD_PLAN_TYPE, &plan_type, ebufp);
	PIN_FLIST_FLD_SET(srch_plan_flistp, PIN_FLD_TYPE_OF_SERVICE, &svc_type, ebufp);
	PIN_FLIST_FLD_SET(srch_plan_flistp,PIN_FLD_VALUE, all_plan_name, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_CITY,srch_plan_flistp,PIN_FLD_CITY, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_PLAN_CATEGORY, srch_plan_flistp, MSO_FLD_PLAN_CATEGORY, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_DEVICE_TYPE, srch_plan_flistp, MSO_FLD_DEVICE_TYPE, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_PAYMENT_TYPE, srch_plan_flistp, MSO_FLD_PAYMENT_TYPE, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_PLAN_SUB_TYPE, srch_plan_flistp,  MSO_FLD_PLAN_SUB_TYPE, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "get plan details input flist ", srch_plan_flistp);  
	PCM_OP(ctxp, MSO_OP_CUST_GET_PLAN_DETAILS, 0, srch_plan_flistp, &srch_plan_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "get plan details output flist ", srch_plan_out_flistp);
	PIN_FLIST_DESTROY_EX(&srch_plan_flistp, NULL);

	ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp);

	elem_id_1 = 0;
	cookie_1 = NULL;
	index = 0;
	while ((plan_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_plan_out_flistp, PIN_FLD_PLAN,
			&elem_id_1, 1, &cookie_1, ebufp)) != (pin_flist_t *)NULL)
	{
		// get the charge for each of the plans.
		srch_plan_name = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_NAME, 0, ebufp);
		if (strcmp(srch_plan_name, plan_name))
		{
			//srch_plan_charge = get_plan_charge(ctxp, srch_plan_name, plan_city,  ebufp);
			srch_plan_charge = extract_plan_charge(ctxp, plan_flistp, plan_city,  ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "average monthly charge for the search result plan is ");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, pbo_decimal_to_str(srch_plan_charge, ebufp));
			// if the charge is > than plan_charge, add it to ret flistp.
			if((!pbo_decimal_is_null(srch_plan_charge, ebufp) && !pbo_decimal_is_null(plan_charge, ebufp)) 
				&& pbo_decimal_compare(srch_plan_charge, plan_charge, ebufp) > 0) {
				add_plan_flistp = PIN_FLIST_ELEM_ADD(ret_flistp, PIN_FLD_PLAN, index, ebufp);
				PIN_FLIST_FLD_COPY(plan_flistp, PIN_FLD_POID, add_plan_flistp, PIN_FLD_PLAN_OBJ, ebufp);
				PIN_FLIST_FLD_COPY(plan_flistp, PIN_FLD_NAME, add_plan_flistp, PIN_FLD_NAME, ebufp);
				index++;
			}			
		}		
	}

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_plan_out_flistp, NULL);
	if(!pbo_decimal_is_null(srch_plan_charge, ebufp)) {
	    pbo_decimal_destroy(&srch_plan_charge);
	}
	if(!pbo_decimal_is_null(plan_charge, ebufp)) {
	    pbo_decimal_destroy(&plan_charge);
	}
	*r_flistpp = ret_flistp;
	return;

}


pin_decimal_t *get_plan_charge(pcm_context_t		*ctxp,
								char *plan_name, 
								char *city, 
								pin_errbuf_t		*ebufp)
{

	char			template[200] = "select X from /mso_cfg_credit_limit where F1 = V1 and ( F2 = V2 or F3 = V3 ) "; //and F4 = V4 ";
	int32			srch_flag = 0;
	int64			db = 1;
	poid_t			*srch_pdp = NULL;
	pin_flist_t		*srch_plan_details_in_flistp = NULL;
	pin_flist_t		*srch_plan_details_out_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*cities_flistp = NULL;
	char			*plan_city = NULL;
	char			all_cities[5] = "*";
	pin_decimal_t	*plan_charge = NULL;
	pin_decimal_t	*all_cities_plan_charge = NULL;
	int32			elem_id_1 = 0;
	pin_cookie_t	cookie_1 = NULL;
	double		payterm = 1;

	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_plan_details_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_plan_details_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_plan_details_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_plan_details_in_flistp, PIN_FLD_TEMPLATE, template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_plan_details_in_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp,MSO_FLD_PLAN_NAME, plan_name, ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(srch_plan_details_in_flistp, PIN_FLD_ARGS, 2, ebufp);
	cities_flistp = PIN_FLIST_ELEM_ADD(args_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(cities_flistp,PIN_FLD_CITY, city, ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(srch_plan_details_in_flistp, PIN_FLD_ARGS, 3, ebufp);
	cities_flistp = PIN_FLIST_ELEM_ADD(args_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(cities_flistp,PIN_FLD_CITY, all_cities, ebufp);
	
/*
	args_flistp = PIN_FLIST_ELEM_ADD(srch_plan_details_in_flistp, PIN_FLD_ARGS, 4, ebufp);
	cities_flistp = PIN_FLIST_ELEM_ADD(args_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(cities_flistp,PIN_FLD_BILL_WHEN, &payterm, ebufp);
*/

	result_flistp = PIN_FLIST_ELEM_ADD(srch_plan_details_in_flistp, PIN_FLD_RESULTS, 0, ebufp);
	cities_flistp = PIN_FLIST_ELEM_ADD(result_flistp, MSO_FLD_CITIES, PCM_RECID_ALL, ebufp);
	PIN_FLIST_FLD_SET(cities_flistp,PIN_FLD_CITY, NULL, ebufp);
	PIN_FLIST_FLD_SET(cities_flistp,PIN_FLD_BILL_WHEN, NULL, ebufp);
	PIN_FLIST_FLD_SET(cities_flistp,PIN_FLD_CHARGE_AMT, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Plan search input flist", srch_plan_details_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_plan_details_in_flistp, &srch_plan_details_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Plan search output flist", srch_plan_details_out_flistp);


	result_flistp = PIN_FLIST_ELEM_GET(srch_plan_details_out_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
	    while ((cities_flistp = PIN_FLIST_ELEM_GET_NEXT(result_flistp, MSO_FLD_CITIES,
			&elem_id_1, 1, &cookie_1, ebufp)) != (pin_flist_t *)NULL)
	    {
		plan_city = PIN_FLIST_FLD_GET(cities_flistp, PIN_FLD_CITY, 0, ebufp);
		if(strcmp(plan_city, all_cities) == 0) {
			all_cities_plan_charge = pbo_decimal_copy(PIN_FLIST_FLD_GET(cities_flistp, PIN_FLD_CHARGE_AMT, 0, ebufp), ebufp);
			payterm = (double)*(int32 *)PIN_FLIST_FLD_GET(cities_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);
		} else if(strcmp(plan_city, city) == 0) {
			plan_charge = pbo_decimal_copy(PIN_FLIST_FLD_GET(cities_flistp, PIN_FLD_CHARGE_AMT, 0, ebufp), ebufp);
			payterm = (double)*(int32 *)PIN_FLIST_FLD_GET(cities_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);
			break;
		}
	    }
	    if(plan_charge == NULL)
	    {
		plan_charge = all_cities_plan_charge;
	    }

	// Divide the total charge by payterm.
	pbo_decimal_divide_assign(plan_charge, pbo_decimal_from_double(payterm, ebufp), ebufp);

	PIN_FLIST_DESTROY_EX(&srch_plan_details_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_plan_details_out_flistp, NULL);
	return plan_charge;
}


pin_decimal_t *extract_plan_charge(pcm_context_t		*ctxp,
					pin_flist_t	*plan_info_flistp,
					char *city, 
					pin_errbuf_t		*ebufp) 
{
        char                    *plan_city = NULL;
        char                    all_cities[5] = "*";
	poid_t			*planinfo_flistp = NULL;
	pin_flist_t		*cities_flistp = NULL;
	pin_decimal_t		*all_cities_plan_charge = NULL;
	pin_decimal_t		*plan_charge = NULL;
	double			payterm = 1;
	int32			elem_id_1 = 0;
	pin_cookie_t	cookie_1 = NULL;

	planinfo_flistp = PIN_FLIST_SUBSTR_GET(plan_info_flistp, MSO_FLD_PLAN_INFO, 0, ebufp);

	while ((cities_flistp = PIN_FLIST_ELEM_GET_NEXT(planinfo_flistp, MSO_FLD_CITIES,
                        &elem_id_1, 1, &cookie_1, ebufp)) != (pin_flist_t *)NULL)
            {
                plan_city = PIN_FLIST_FLD_GET(cities_flistp, PIN_FLD_CITY, 0, ebufp);
                if(strcmp(plan_city, all_cities) == 0) {
                        all_cities_plan_charge = pbo_decimal_copy(PIN_FLIST_FLD_GET(cities_flistp, PIN_FLD_CHARGE_AMT, 0, ebufp), ebufp);
                        payterm = (double)*(int32 *)PIN_FLIST_FLD_GET(cities_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);
                } else if(strcmp(plan_city, city) == 0) {
                        plan_charge = pbo_decimal_copy(PIN_FLIST_FLD_GET(cities_flistp, PIN_FLD_CHARGE_AMT, 0, ebufp), ebufp);
                        payterm = (double)*(int32 *)PIN_FLIST_FLD_GET(cities_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);
                        break;
                }
            }
            if(plan_charge == NULL)
            {
                plan_charge = all_cities_plan_charge;
            }

        // Divide the total charge by payterm.
        pbo_decimal_divide_assign(plan_charge, pbo_decimal_from_double(payterm, ebufp), ebufp);

	return plan_charge;

}
