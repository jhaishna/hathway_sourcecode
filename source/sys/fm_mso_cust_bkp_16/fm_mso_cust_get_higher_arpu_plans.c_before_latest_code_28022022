/*******************************************************************
 * File:	fm_mso_cust_get_higher_arpu_plans.c
 * Opcode:	MSO_OP_CUST_GET_HIGHER_ARPU_PLANS 
 * Owner:	Vilva
 * Created:	15-JUN-2016
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
 * MSO_OP_CUST_GET_HIGHER_ARPU_PLANS 
 * 
 *******************************************************************/

EXPORT_OP void
op_mso_cust_get_higher_arpu_plans(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_get_higher_arpu_plans(
	pcm_context_t		*ctxp,
	int32			    flags,
	pin_flist_t		    *i_flistp,
	pin_flist_t		    **r_flistpp,
	pin_errbuf_t		*ebufp);

void
get_source_plan_charge(
    pcm_context_t		*ctxp,
    pin_flist_t			*i_flistp,
	pin_flist_t			**r_flistp,
    pin_errbuf_t		*ebufp);

void
extract_search_plan_charge(
    pcm_context_t   *ctxp,
	pin_flist_t	    *plan_info_flistp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistp,
    pin_errbuf_t    *ebufp);

void
mso_get_dl_speed(
	pcm_context_t   *ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistp,
	pin_errbuf_t    *ebufp);
	
void
mso_get_dl_speed(
	pcm_context_t   *ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistp,
	pin_errbuf_t    *ebufp);

char*
get_acc_package_detail(
	pcm_context_t		*ctxp,
	poid_t			*poid_id,
	char			*acc_poid,
	pin_errbuf_t		*ebufp);

char*	
get_on_request_detail(
	pcm_context_t		*ctxp,
	poid_t			*poid_id,
	char			*acc_poid,
	pin_errbuf_t		*ebufp);
	
int32
get_prod_expiration_detail(
	pcm_context_t		*ctxp,
	poid_t			*poid_id,
	char			*acc_poid,
	char			*pln_name,
	pin_errbuf_t		*ebufp);
	
void
get_hfiber_plans(
	pcm_context_t   *ctxp,
	pin_flist_t		*plan_flistp,
	pin_flist_t		**out_flistp,
    pin_errbuf_t    *ebufp);
	
void
get_promo_adhoc_plans(
	pcm_context_t   *ctxp,
	pin_flist_t		*in_flistp,
	int32			is_expired,
	char			*aac_package_val,
	pin_flist_t		**r_adhoc_flistpp,
    pin_errbuf_t    *ebufp);
	
/*******************************************************************
 * MSO_OP_CUST_GET_HIGHER_ARPU_PLANS  
 *******************************************************************/
void 
op_mso_cust_get_higher_arpu_plans(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t    *ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	*r_flistpp		= NULL;
	poid_t			*inp_pdp = NULL;
    pin_flist_t     *err_flistp = NULL;
	pin_flist_t     *hfiber_return_flist = NULL;
    int             error_status = 1;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_GET_HIGHER_ARPU_PLANS) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_get_higher_arpu_plans error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_get_higher_arpu_plans input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	fm_mso_cust_get_higher_arpu_plans(ctxp, flags, i_flistp, r_flistpp, ebufp);
 	/***********************************************************
	 * Results.
	 ***********************************************************/

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_get_higher_arpu_plans error", ebufp);
        err_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, err_flistp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "511022", ebufp);
        PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Search plan Doesn't suit the Payterm Passed", ebufp);
        PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &error_status, ebufp);

        *r_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_get_higher_arpu_plans output flist", *r_flistpp);
        
        PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
	}
	else
	{
        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, 0 ,ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_get_higher_arpu_plans output flist", *r_flistpp);
	}
	return;
}


/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_get_higher_arpu_plans(
	pcm_context_t   *ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t    *ebufp)
{

	poid_t			*ppdp = NULL;
	pin_flist_t		*out_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*srch_plan_flistp = NULL;
	pin_flist_t		*plan_flistp = NULL;
	pin_flist_t		*srch_plan_out_flistp = NULL;
	pin_flist_t		*add_plan_flistp = NULL;
    pin_flist_t     *sort_template_flistp = NULL;
    pin_flist_t     *sort_inflistp = NULL;
    pin_flist_t     *sort_outflistp = NULL;
    pin_flist_t     *s_plan_flistp = NULL;
    pin_flist_t     *err_flistp = NULL;
	pin_flist_t		*new_plan_out_flistp = NULL;
	pin_flist_t		*existing_out_flistp = NULL;
	pin_flist_t		*in_flistp = NULL;
	int64			db = 1;
	char		    err_str[50]={'\0'};
//	char			all_plan_name[3] = "%";
	char 			all_plan_name[20] = "higher_arpu";
	char			*city = NULL;
	int32			plan_srch_key = 0;
	int32			plan_srch_type = 3;
	int32			svc_type = 2;
	int32			plan_type = 2;
	pin_decimal_t	*prod_priority = NULL;
	double			prod_priority_double = 0.0;
	char			*plan_city = NULL;
	char			*plan_name = NULL;
	pin_decimal_t	*srch_plan_charge = NULL;
	pin_decimal_t	*plan_charge = NULL;
	int32			elem_id_1 = 0;
    int32           elem_id = 0;
	int32			index = 0;
    int32           index1 = 0;
	pin_cookie_t	cookie_1 = NULL;
    pin_cookie_t    cookie = NULL;
	char			*srch_plan_name = NULL;
	int             error_status = 1;
	int				success_status = 0;
	int32			status = 2;
	int32			ex_status = 2;
	int32			payterm = 0;
	pin_decimal_t	*plan_dl_speed = NULL;
	pin_decimal_t	*srch_plan_dl_speed = NULL;
	void			*vp = NULL;
	int32 			res_total = 0;
	char			*aac_package_val = NULL;
	char			*adhoc_type = NULL;
	//pin_flist_t		*res_flistp = NULL;
	FILE			*filep = NULL;


	filep = fopen("/tmp/higherarpu","a+");


	/*
	Algorithm:
	1. Get the input parameters and search the plan and get the plan charge applicable to city/payterm and plan name.
	2. Search other plans matching the criteria of the current plan using MSO_OP_CUST_GET_PLAN_DETAILS opcode.
	3. Check the charge for each of the plans based on the city and payterm passed.
	4. If the charge is higher than the current plan charge, add it to return flist.
	*/
	PIN_ERRBUF_CLEAR(ebufp);

	plan_name = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_PLAN_NAME, 0, ebufp);
	plan_city = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CITY, 0, ebufp);
	payterm  = *(int32 *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);
//	existing_out_flistp = PIN_FLIST_CREATE(ebufp);
	get_source_plan_charge(ctxp, i_flistp, &existing_out_flistp, ebufp);
	vp = PIN_FLIST_FLD_GET(existing_out_flistp, PIN_FLD_STATUS, 1, ebufp);
	if (vp && vp != NULL)//akshayrc
		status = *(int32 *)vp;
	if (existing_out_flistp && (status == 0))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Existing Plan Details Retrieved Successfully.");

		plan_charge = PIN_FLIST_FLD_GET(existing_out_flistp, PIN_FLD_AMOUNT, 1, ebufp);
		plan_dl_speed = PIN_FLIST_FLD_GET(existing_out_flistp, MSO_FLD_DL_SPEED, 1, ebufp);

		if (!pbo_decimal_is_null(plan_charge, ebufp) && !pbo_decimal_is_null(plan_dl_speed, ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "average monthly charge for the current plan is ");
	    	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, pbo_decimal_to_str(plan_charge, ebufp));
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "average Download Speed for the current plan is ");
    		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, pbo_decimal_to_str(plan_dl_speed, ebufp));
	    }
		else
		{
			err_flistp = PIN_FLIST_CREATE(ebufp);
	        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, err_flistp, PIN_FLD_POID, ebufp);
		    PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "511022", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Search plan Doesn't suit the Payterm Passed", ebufp);
	        PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &error_status, ebufp);

		    *r_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
        
			PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
	        goto CLEANUP;
		}
	

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

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "get plan details input flist", srch_plan_flistp);  
		PCM_OP(ctxp, MSO_OP_CUST_GET_PLAN_DETAILS, 0, srch_plan_flistp, &srch_plan_out_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "get plan details output flist ", srch_plan_out_flistp);
		PIN_FLIST_DESTROY_EX(&srch_plan_flistp, NULL);

		sort_outflistp = PIN_FLIST_CREATE(ebufp);
		sort_inflistp = PIN_FLIST_CREATE(ebufp);
		
		
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, sort_inflistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, sort_outflistp, PIN_FLD_POID, ebufp);

		elem_id_1 = 0;
		cookie_1 = NULL;
		index = 0;
		
		
		while ((plan_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_plan_out_flistp, PIN_FLD_PLAN,
				&elem_id_1, 1, &cookie_1, ebufp)) != (pin_flist_t *)NULL)
		{
			pin_flist_t		*hfiber_return_flist = NULL;
			// get the charge for each of the plans.
			srch_plan_name = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_NAME, 0, ebufp);
			if (srch_plan_name && !strstr(srch_plan_name,"Retention"))
			{
				in_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, in_flistp, PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_SET(in_flistp, MSO_FLD_PLAN_NAME, srch_plan_name, ebufp);
				PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_CITY, plan_city, ebufp);
				PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_BILL_WHEN, &payterm, ebufp);

				extract_search_plan_charge(ctxp, plan_flistp, in_flistp, &new_plan_out_flistp, ebufp);
				vp = NULL;
				vp = PIN_FLIST_FLD_GET(new_plan_out_flistp, PIN_FLD_STATUS, 1, ebufp);
				if (vp && vp != NULL)//akshayrc
					ex_status = *(int32 *)vp;
				if (new_plan_out_flistp && (ex_status == 0))
				{
					srch_plan_charge = PIN_FLIST_FLD_GET(new_plan_out_flistp, PIN_FLD_AMOUNT, 0, ebufp);
					srch_plan_dl_speed = PIN_FLIST_FLD_GET(new_plan_out_flistp, MSO_FLD_DL_SPEED, 0, ebufp);

				
					if (!pbo_decimal_is_null(srch_plan_charge, ebufp) && !pbo_decimal_is_null(srch_plan_dl_speed, ebufp))
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "average monthly charge for the current plan is ");
					    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, pbo_decimal_to_str(plan_charge, ebufp));
		    			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "average monthly charge for the search result plan is ");
				    	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, pbo_decimal_to_str(srch_plan_charge, ebufp));
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Download Speed for the current plan is ");
					    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, pbo_decimal_to_str(plan_dl_speed, ebufp));
	    				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Download Speed for the search result plan is ");
			    		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, pbo_decimal_to_str(srch_plan_dl_speed, ebufp));
					}
            
				    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Extraction FLIST", plan_flistp);
					
					/*******************************************************************
					 * Excluding H-fiber Plans - Biju.J
					*******************************************************************/
					
					get_hfiber_plans(ctxp,plan_flistp, r_flistpp,ebufp);
						
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "get hfiber plans output flist");
						if (*r_flistpp != (pin_flist_t	*)NULL) {
							hfiber_return_flist = PIN_FLIST_ELEM_GET(*r_flistpp, PIN_FLD_RESULTS, 0, 1, ebufp);
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"hfiber_return_flist ", hfiber_return_flist);
						}
						
						if (hfiber_return_flist == (pin_flist_t	*)NULL)
						{
					// if the charge is > than plan_charge, add it to ret flistp.
							if((!pbo_decimal_is_null(srch_plan_charge, ebufp) && !pbo_decimal_is_null(plan_charge, ebufp)) 
								&& (pbo_decimal_compare(srch_plan_charge, plan_charge, ebufp) > 0 )
								&& (!pbo_decimal_is_null(srch_plan_dl_speed, ebufp) && !pbo_decimal_is_null(plan_dl_speed, ebufp)) 
								&& (pbo_decimal_compare(srch_plan_dl_speed, plan_dl_speed, ebufp) >= 0 )) 
							{								
								add_plan_flistp = PIN_FLIST_ELEM_ADD(sort_inflistp, PIN_FLD_PLAN, index, ebufp);
								PIN_FLIST_FLD_COPY(plan_flistp, PIN_FLD_POID, add_plan_flistp, PIN_FLD_PLAN_OBJ, ebufp);
								PIN_FLIST_FLD_COPY(plan_flistp, PIN_FLD_NAME, add_plan_flistp, PIN_FLD_NAME, ebufp);
								PIN_FLIST_FLD_SET(add_plan_flistp, PIN_FLD_AMOUNT, srch_plan_charge, ebufp);
								PIN_FLIST_FLD_SET(add_plan_flistp, MSO_FLD_DL_SPEED, srch_plan_dl_speed, ebufp);
								PIN_FLIST_FLD_SET(add_plan_flistp, MSO_FLD_DISPLAY_FLAG, "0", ebufp);
								index++;
							}
						}
				}
				else
				{
					err_flistp = PIN_FLIST_CREATE(ebufp);
			        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, err_flistp, PIN_FLD_POID, ebufp);
				    PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "511022", ebufp);
					PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Existing Plan Details Retrieval Error", ebufp);
			        PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &error_status, ebufp);

				    *r_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
        
					PIN_FLIST_DESTROY_EX(&err_flistp, NULL);

				}
			}		
		}

	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Return Preparation flist", sort_inflistp);

		sort_template_flistp = PIN_FLIST_CREATE(ebufp);
		s_plan_flistp = PIN_FLIST_ELEM_ADD(sort_template_flistp, PIN_FLD_PLAN, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(s_plan_flistp, PIN_FLD_AMOUNT, 0,ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Sorting template flist", sort_template_flistp);

		if (index > 5)
	    {
		    res_total = 4;
		    // Have to show maximum 5 plans only in the HIGHER ARPU plans
			//PIN_FLIST_SORT_REVERSE(sort_inflistp, sort_template_flistp, 0, ebufp);
		    // As per Business Requirement, sorting the most closer ARPU products
			PIN_FLIST_SORT(sort_inflistp, sort_template_flistp, 0, ebufp);
	        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Sorted plan_list_active", sort_inflistp);

		    while ((plan_flistp = PIN_FLIST_ELEM_GET_NEXT(sort_inflistp, PIN_FLD_PLAN, &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
    		{
				ret_flistp = PIN_FLIST_ELEM_ADD(sort_outflistp, PIN_FLD_PLAN, index1, ebufp);
	            PIN_FLIST_FLD_COPY(plan_flistp, PIN_FLD_PLAN_OBJ, ret_flistp, PIN_FLD_PLAN_OBJ, ebufp);
		        PIN_FLIST_FLD_COPY(plan_flistp, PIN_FLD_NAME, ret_flistp, PIN_FLD_NAME, ebufp);
			    PIN_FLIST_FLD_COPY(plan_flistp, PIN_FLD_AMOUNT, ret_flistp, PIN_FLD_AMOUNT, ebufp);
				PIN_FLIST_FLD_COPY(plan_flistp, MSO_FLD_DL_SPEED, ret_flistp, MSO_FLD_DL_SPEED, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, MSO_FLD_DISPLAY_FLAG, "0", ebufp);
				index1++;
	            if (index1 >=5)
		        {
			        break;
				}
	        }
	    // As per Business Requirement, the display should be from higher arpu to the lower ones in the closest list. Hence sorting in reverse.
		PIN_FLIST_SORT_REVERSE(sort_outflistp, sort_template_flistp, 0, ebufp);
	        *r_flistpp = PIN_FLIST_COPY(sort_outflistp, ebufp);
		PIN_ERRBUF_RESET(ebufp);
		}
		else if (index < 1)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No Plans Available for this Features - Hfiber");
			err_flistp = PIN_FLIST_CREATE(ebufp);
                	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, err_flistp, PIN_FLD_POID, ebufp);
                    	PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "511022", ebufp);
                        PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Search plan is not having any higher ARPU plans", ebufp);
                	PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &error_status, ebufp);

                    	*r_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
			
			goto CLEANUP;
		}
	    	else
		{   
			res_total = index -1;
			PIN_FLIST_SORT_REVERSE(sort_inflistp, sort_template_flistp, 0, ebufp);
	        	*r_flistpp = PIN_FLIST_COPY(sort_inflistp, ebufp);
			PIN_ERRBUF_RESET(ebufp);
		}

		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			err_flistp = PIN_FLIST_CREATE(ebufp);
	        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, err_flistp, PIN_FLD_POID, ebufp);
		    PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "511022", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Search plan Doesn't suit the Payterm Passed", ebufp);
	        PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &error_status, ebufp);
	
		    *r_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
        
			PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
	        goto CLEANUP;
		}

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Sorted Final Plans", *r_flistpp);
		
/**************************************************
	 * Promotion Validity Extension  - Biju.J
***************************************************/
		
		char			*city_promo = NULL;
		char			*pln_name = NULL;
		char			*aac_package_val = NULL;
		char			*on_request = NULL;
		char			*on_request_val = NULL;
		int32			is_expired = -1;
		poid_t			*acc_obj_id = NULL;
		poid_t			*poid_id = NULL;
		poid_t			*default_account = NULL;
		char	allow_offer[36] = "P%";
		char template_promo[200] = {'\0'}; 
		char		 msg_v[1000];

		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"new flist for the promo ", i_flistp);
		
		if (i_flistp != (pin_flist_t	*)NULL) {
		
			acc_obj_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
			poid_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
			pln_name = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_PLAN_NAME, 0, ebufp);
			aac_package_val = get_acc_package_detail(ctxp,poid_id,acc_obj_id,ebufp);
			
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "aac_package_val is ");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, aac_package_val);
	
			city_promo = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CITY, 0, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "city_promo_val is ");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, city_promo);
			
			is_expired = get_prod_expiration_detail(ctxp,poid_id,acc_obj_id,pln_name,ebufp);
			
			on_request_val = get_on_request_detail(ctxp,poid_id,acc_obj_id,ebufp);
			
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "on_request_val is ");
			sprintf(msg_v,"on_request_val is 1 : %ld",*on_request_val);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg_v);
			//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, *on_request_val);
			
		}
		
		//old_old
		
		/*if (strcmp(aac_package_val, "Y") == 0) 
		{
			//sprintf(template_promo, "select X from /higher_arpu_promotion_plan 
				//					where PIN_FLD_CITY = 'BANGALORE' 
			//					and activity = 'change' and (allow_offer like 'P*' OR adhoc_type like V4) ");
			sprintf(template_promo, "select X from /higher_arpu_promotion_plan where F1 = V1 and F2 = V2 and (F3 like V3 OR F4 like V4) ");
		}
		else 
		{
			//sprintf(template_promo, "select X from /higher_arpu_promotion_plan 
														//where PIN_FLD_CITY = 'BANGALORE' 
			                                            //and activity = 'change' and allow_offer is null 
														//and adhoc_type like V4 ");
			sprintf(template_promo, "select X from /higher_arpu_promotion_plan where F1 = V1 and F2 = V2 and F3 is null and F4 like V4 ");
			
		}

		if (is_expired == 0)  //Expiry>30 days
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "plan expiration value is zero");
			adhoc_type = "R";
		}
		else  //Expiry<30 days
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "plan expiration value is one");
			adhoc_type = "P";
		}*/
		
		//old_old
		
		//old logic
		/*if (strcmp(aac_package_val, "Y") == 0) 
		{
			sprintf(template_promo, "select X from /higher_arpu_promotion_plan where F1 = V1 and F2 = V2 and (F3 like V3 OR F4 like V4) and F5 = V5");
		}
		else 
		{
			sprintf(template_promo, "select X from /higher_arpu_promotion_plan where F1 = V1 and F2 = V2 and F3 is null and F4 like V4 and F5 = V5");
			//sprintf(template_promo, "select X from /higher_arpu_promotion_plan where F1 = V1 and F2 = V2 and F4 like V4 ");
			
		}

		
		if (is_expired == 0 )  //Expiry>30 days
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "plan expiration value is zero");
			adhoc_type = "R";
			on_request = "ALL";
			sprintf(template_promo, "select X from /higher_arpu_promotion_plan where F1 = V1 and F2 = V2 and (F3 like V3 OR F4 like V4) and F5 = V5");
		}
		//else if (is_expired == 1 && strcmp(on_request_val, "Y") == 0)  //Expiry<30 days
		else 				  //Expiry<30 days
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "plan expiration value is one");
			adhoc_type = "P";
			on_request = "Y"
		} */
		//old logic ends
		
		
		if (is_expired == 0 )  //Expiry>1000 days
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "plan expiration value is zero");
			//adhoc_type = "R";
			on_request = "ALL";
			sprintf(template_promo, "select X from /higher_arpu_promotion_plan where F1 = V1 and F2 like V2 and (F3 like V3 or F4 = V4) ");
			//sprintf(template_promo, "select X from /higher_arpu_promotion_plan 
									//where PIN_FLD_CITY = 'BANGALORE' 
								//and activity = 'change' and (allow_offer like 'P*' OR adhoc_type like 'R') ");
		}
		else if (is_expired != 0 )	  //Expiry<1000 days
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "plan expiration value is one");
			//adhoc_type = "P";
			on_request = "Y";
			
			if (strcmp(on_request_val, "YES") == 0)
			{
				if (strcmp(aac_package_val, "Y") == 0) 
				{
					sprintf(template_promo, "select X from /higher_arpu_promotion_plan where F1 = V1 and F2 like V2 and F3 like V3 and F4 = V4 and F5 = V5");
				} else 
				{
					sprintf(template_promo, "select X from /higher_arpu_promotion_plan where F1 = V1 and F2 like V2 and (F3 like V3 or F3 is null) and F4 = V4 and F5 = V5");
				}
				
			}
			else 
			{
			
				//default_account = (poid_t*)1;
				default_account = PIN_POID_CREATE(db, "/account", 1, ebufp);
				if (strcmp(aac_package_val, "Y") == 0) 
				{
					sprintf(template_promo, "select X from /higher_arpu_promotion_plan where F1 = V1 and F2 like V2 and F3 like V3 and F4 <> V4 and higher_arpu_promotion_plan_t.account_obj_id0 = 1 ");
					//sprintf(template1, "select X from /config_higher_arpu_hfiber where config_higher_arpu_hfiber_t.plan_obj_id = %lld ",pln_obj);
				} else 
				{
					sprintf(template_promo, "select X from /higher_arpu_promotion_plan where F1 = V1 and F2 like V2 and (F3 like V3 or F3 is null) and F4 <> V4 and higher_arpu_promotion_plan_t.account_obj_id0 = 1");
				}
			}		
		} 
		
		
			//Adding Promotional Plans with Top five Plans -- Biju.J
			int32			srch_flag = 768;
			poid_t			*srch_pdp = NULL;
			pin_flist_t		*promo_in_flistp = NULL;
			pin_flist_t		*promo_out_flistp = NULL;
			pin_flist_t		*pr_flistp = NULL;
			pin_flist_t		*pr_r_flistp = NULL;
			pin_flist_t		*arg_flist = NULL;
			pin_flist_t		*add_promo_plan_flist = NULL;
			pin_flist_t		*pln_array = NULL;
			pin_flist_t		*result_flist = NULL;
			pin_flist_t		*res_flistp = NULL;
			char			activity[36] = "%";
			elem_id = 0;
			cookie = NULL;
			 		
			PIN_ERRBUF_CLEAR(ebufp);
			promo_out_flistp = PIN_FLIST_CREATE(ebufp);
			
			srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
			promo_in_flistp = PIN_FLIST_CREATE(ebufp);
				
			PIN_FLIST_FLD_PUT(promo_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
			PIN_FLIST_FLD_SET(promo_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
			PIN_FLIST_FLD_SET(promo_in_flistp, PIN_FLD_TEMPLATE, template_promo, ebufp);
			arg_flist = PIN_FLIST_ELEM_ADD(promo_in_flistp, PIN_FLD_ARGS, 1, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_CITY, city_promo, ebufp);
			arg_flist = PIN_FLIST_ELEM_ADD(promo_in_flistp, PIN_FLD_ARGS, 2, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_CLASS_NAME, activity, ebufp);
			arg_flist = PIN_FLIST_ELEM_ADD(promo_in_flistp, PIN_FLD_ARGS, 3, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_CLASSMARK, allow_offer, ebufp);
			//arg_flist = PIN_FLIST_ELEM_ADD(promo_in_flistp, PIN_FLD_ARGS, 4, ebufp );
			//PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_CHECK_NO, adhoc_type, ebufp);
			arg_flist = PIN_FLIST_ELEM_ADD(promo_in_flistp, PIN_FLD_ARGS, 4, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, MSO_FLD_CUSTOMER_BIT, on_request, ebufp);
			arg_flist = PIN_FLIST_ELEM_ADD(promo_in_flistp, PIN_FLD_ARGS, 5, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, poid_id, ebufp);
			
			result_flist = PIN_FLIST_ELEM_ADD(promo_in_flistp, PIN_FLD_RESULTS, 0, ebufp );
		
			PIN_FLIST_FLD_SET(promo_out_flistp, PIN_FLD_PLAN_OBJ, NULL, ebufp); 
			PIN_FLIST_FLD_SET(promo_out_flistp, PIN_FLD_NAME, NULL, ebufp);
			PIN_FLIST_FLD_SET(promo_out_flistp, PIN_FLD_AMOUNT, NULL, ebufp);
			PIN_FLIST_FLD_SET(promo_out_flistp, MSO_FLD_DL_SPEED, NULL, ebufp);
			PIN_FLIST_FLD_SET(promo_out_flistp, MSO_FLD_DISPLAY_FLAG, NULL, ebufp);

		
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Promo search input flist", promo_in_flistp);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, promo_in_flistp, &promo_out_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Promo search output flist", promo_out_flistp);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Promo search output result_flist", result_flist);
			
			PIN_ERRBUF_CLEAR(ebufp);
			pr_flistp = PIN_FLIST_CREATE(ebufp);
			
			pr_r_flistp = PIN_FLIST_CREATE(ebufp);
			
			while((pln_array = PIN_FLIST_ELEM_GET_NEXT(promo_out_flistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp))!= (pin_flist_t *)NULL) 
			{
				//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Promo search output flist inside while loop promooutput", promo_out_flistp);
				//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Promo search output flist inside while loop existing", *r_flistpp);
				res_total = res_total + 1 ;	
				
				res_flistp = PIN_FLIST_ELEM_ADD(pr_flistp, PIN_FLD_PLAN, res_total, ebufp);
				PIN_FLIST_FLD_COPY(pln_array, PIN_FLD_PLAN_OBJ, res_flistp, PIN_FLD_PLAN_OBJ, ebufp);
		        PIN_FLIST_FLD_COPY(pln_array, PIN_FLD_NAME, res_flistp, PIN_FLD_NAME, ebufp);
			    PIN_FLIST_FLD_COPY(pln_array, PIN_FLD_AMOUNT, res_flistp, PIN_FLD_AMOUNT, ebufp);
				PIN_FLIST_FLD_COPY(pln_array, MSO_FLD_DL_SPEED, res_flistp, MSO_FLD_DL_SPEED, ebufp);
				PIN_FLIST_FLD_COPY(pln_array, PIN_FLD_CLASSMARK, res_flistp, PIN_FLD_CLASSMARK, ebufp);
				PIN_FLIST_FLD_SET(res_flistp, MSO_FLD_DISPLAY_FLAG, "1", ebufp);
				
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "TEST Flist -1", pr_flistp);
				//PIN_FLIST_CONCAT(pr_r_flistp, pr_flistp, ebufp);	
				//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "TEST Flist -2", pr_r_flistp);
				
			}
			
			//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Promo search output flist outside loop", pr_r_flistp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Promo search output flist outside loop -1 ", *r_flistpp);
			
			//Get Adhoc Plans --starts
			/*get_promo_adhoc_plans(ctxp, in_flistp, is_expired, aac_package_val, r_adhoc_flistpp,ebufp);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Adhoc search output flist -1 ", *r_adhoc_flistpp);
			
			if (r_adhoc_flistpp != (pin_flist_t	*)NULL) {
			{
				PIN_FLIST_CONCAT(pr_flistp, r_adhoc_flistpp, ebufp);
			}*/
			//Get Adhoc Plans -- ends
			
			//PIN_FLIST_CONCAT(*r_flistpp, pr_r_flistp, ebufp);
			PIN_FLIST_CONCAT(*r_flistpp, pr_flistp, ebufp);
			
			/*char plan_flag[36] = "promo1";
			
			if (strcmp(plan_flag, "all") == 0)
			{
				PIN_FLIST_CONCAT(*r_flistpp, pr_r_flistp, ebufp);
			}
			else if (strcmp(plan_flag, "promo") == 0)
			{
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, pr_r_flistp, PIN_FLD_POID, ebufp);
				*r_flistpp = PIN_FLIST_COPY(pr_r_flistp, ebufp);				
			}
			else
			{
				*r_flistpp = PIN_FLIST_COPY(*r_flistpp, ebufp);
			}
			*/
			
			PIN_FLIST_DESTROY_EX(&res_flistp,NULL);
			PIN_FLIST_DESTROY_EX(&pln_array,NULL);
			PIN_FLIST_DESTROY_EX(&promo_out_flistp,NULL);
			PIN_FLIST_DESTROY_EX(&promo_in_flistp,NULL);
	}
	else
	{
		err_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, err_flistp, PIN_FLD_POID, ebufp);
	    PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "511022", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Existing Plan Details Retrieval Error", ebufp);
        PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &error_status, ebufp);

	    *r_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
        
		PIN_FLIST_DESTROY_EX(&err_flistp, NULL);

	}

	CLEANUP:
	
    PIN_FLIST_DESTROY_EX(&srch_plan_out_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&sort_outflistp, NULL);
    PIN_FLIST_DESTROY_EX(&sort_inflistp, NULL);
    PIN_FLIST_DESTROY_EX(&sort_template_flistp,NULL);
	PIN_FLIST_DESTROY_EX(&existing_out_flistp,NULL);
	PIN_FLIST_DESTROY_EX(&new_plan_out_flistp,NULL);
	PIN_FLIST_DESTROY_EX(&in_flistp,NULL);

	return;

}

/************************************************************************
	 * Promotion Validity Extension - To get Aac_package value - Biju.J
*************************************************************************/
char*
get_acc_package_detail(
	pcm_context_t		*ctxp,
	poid_t			*poid_id,
	char			*acc_obj_id,
	pin_errbuf_t		*ebufp)
	
	{
		int64			db = -1;
		int32			srch_flag = 768;
		char			err_str[50]={'\0'};
		char			*aac_package_val = NULL;
		char			*ret_val = NULL;
		poid_t			*srch_pdp = NULL;
		pin_flist_t		*result_flistp = NULL;
		pin_flist_t		*args_flistp = NULL;
		pin_flist_t		*aac_in_flistp = NULL;
		pin_flist_t		**aac_out_flistpp = NULL;
		pin_flist_t		*aac_out_flist = NULL;
		char template[200] = "select X from /account where F1 = V1";

		
		if (acc_obj_id && acc_obj_id != NULL)
		{
			
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Acc obj is defined");
			PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Acc obj is : ", acc_obj_id);
			
			PIN_ERRBUF_CLEAR(ebufp);
			aac_out_flist = PIN_FLIST_CREATE(ebufp);
			
			db = PIN_POID_GET_DB(poid_id);
			srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
			aac_in_flistp = PIN_FLIST_CREATE(ebufp);
				
			PIN_FLIST_FLD_PUT(aac_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
			PIN_FLIST_FLD_SET(aac_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
			PIN_FLIST_FLD_SET(aac_in_flistp, PIN_FLD_TEMPLATE, template, ebufp);
			
			args_flistp = PIN_FLIST_ELEM_ADD(aac_in_flistp, PIN_FLD_ARGS, 1, ebufp);
			PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_POID, acc_obj_id, ebufp);
			
			result_flistp = PIN_FLIST_ELEM_ADD(aac_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
		
			PIN_FLIST_FLD_SET(result_flistp,PIN_FLD_AAC_PACKAGE, NULL, ebufp);
		
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Acc_package input flist", aac_in_flistp);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, aac_in_flistp, &aac_out_flist, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Acc_package search output flist", aac_out_flist);
		
			result_flistp = PIN_FLIST_ELEM_GET(aac_out_flist, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Acc_package search output flist", result_flistp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account is not active");
				PIN_ERRBUF_CLEAR(ebufp);
				*aac_out_flistpp = NULL;
				goto CLEANUP;
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Found Account Details");
				aac_package_val = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_AAC_PACKAGE, 0, ebufp);
			}
				
			
			if (!aac_package_val)
			{
				pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MATCH, PIN_FLD_CODE, 0, 0);
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "error fetching aac_package_val", ebufp);
				goto CLEANUP;
			}
			else
			{
				ret_val = (char *) malloc(sizeof(aac_package_val));
				strcpy(ret_val, aac_package_val);
			}
	}
	
	CLEANUP:
		if(aac_in_flistp)
		PIN_FLIST_DESTROY_EX(&aac_in_flistp, NULL);
		if(aac_out_flist)
		PIN_FLIST_DESTROY_EX(&aac_out_flist, NULL);
		
		return ret_val;
} 

//To Get on request value from Account_t class(PIN_FLD_ACCOUNT_TAG field) -- Biju.J

char*
get_on_request_detail(
	pcm_context_t		*ctxp,
	poid_t			*poid_id,
	char			*acc_obj_id,
	pin_errbuf_t		*ebufp)
	
	{
		int64			db = -1;
		int32			srch_flag = 768;
		char			err_str[50]={'\0'};
		char			*on_request_val = NULL;
		char			*ret_val = NULL;
		poid_t			*srch_pdp = NULL;
		pin_flist_t		*result_flistp = NULL;
		pin_flist_t		*args_flistp = NULL;
		pin_flist_t		*aac_in_flistp = NULL;
		pin_flist_t		**aac_out_flistpp = NULL;
		pin_flist_t		*aac_out_flist = NULL;
		char template[200] = "select X from /account where F1 = V1";

		
		if (acc_obj_id && acc_obj_id != NULL)
		{
			
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Acc obj is defined");
			PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Acc obj is : ", acc_obj_id);
			
			PIN_ERRBUF_CLEAR(ebufp);
			aac_out_flist = PIN_FLIST_CREATE(ebufp);
			
			db = PIN_POID_GET_DB(poid_id);
			srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
			aac_in_flistp = PIN_FLIST_CREATE(ebufp);
				
			PIN_FLIST_FLD_PUT(aac_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
			PIN_FLIST_FLD_SET(aac_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
			PIN_FLIST_FLD_SET(aac_in_flistp, PIN_FLD_TEMPLATE, template, ebufp);
			
			args_flistp = PIN_FLIST_ELEM_ADD(aac_in_flistp, PIN_FLD_ARGS, 1, ebufp);
			PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_POID, acc_obj_id, ebufp);
			
			result_flistp = PIN_FLIST_ELEM_ADD(aac_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
		
			PIN_FLIST_FLD_SET(result_flistp,PIN_FLD_ACCOUNT_TAG, NULL, ebufp);
		
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "On_request input flist", aac_in_flistp);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, aac_in_flistp, &aac_out_flist, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "On_request search output flist", aac_out_flist);
		
			result_flistp = PIN_FLIST_ELEM_GET(aac_out_flist, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "On_request search output flist", result_flistp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account is not active");
				PIN_ERRBUF_CLEAR(ebufp);
				*aac_out_flistpp = NULL;
				goto CLEANUP;
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Found Account Details");
				on_request_val = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_ACCOUNT_TAG, 0, ebufp);
			}
				
			
			if (!on_request_val)
			{
				pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MATCH, PIN_FLD_CODE, 0, 0);
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "error fetching On_request", ebufp);
				goto CLEANUP;
			}
			else
			{
				ret_val = (char *) malloc(sizeof(on_request_val));
				strcpy(ret_val, on_request_val);
			}
	}
	
	CLEANUP:
		if(aac_in_flistp)
		PIN_FLIST_DESTROY_EX(&aac_in_flistp, NULL);
		if(aac_out_flist)
		PIN_FLIST_DESTROY_EX(&aac_out_flist, NULL);
		
		return ret_val;
} 

// To get active Plan expiration detail (PIN_FLD_CYCLE_END_T) -- Biju.J
int32
get_prod_expiration_detail(
	pcm_context_t		*ctxp,
	poid_t			*poid_id,
	char			*acc_poid,
	char			*pln_name,
	pin_errbuf_t		*ebufp)
	
	{
	
		int64			db = -1;
		int32			srch_flag = 768;
		char			err_str[50]={'\0'};
		int32			pln_expired = -1;
		char			sub_prod[30] = "%SUBSCRIPTION%";
		poid_t			*srch_pdp = NULL;
		pin_flist_t		*result_flistp = NULL;
		pin_flist_t		*args_flistp = NULL;
		pin_flist_t		*aac_in_flistp = NULL;
		pin_flist_t		*aac_out_flist = NULL;
		//time_t          curr_date = 0;
		time_t 			cycle_end = 0;
		//int32 			exp_date = 0;
		int32 			*exp_date = NULL;
		time_t 			rem_date = 0;
		int32 			errp = 0;
		char		 msg_v[1000];
		
		char template[200] = "select X from /purchased_product 1, /plan 2 where 1.F1 = V1 and 2.F3 = 1.F4 and 2.F2 = V2 and 1.F5 like V5 ";
		
		if (acc_poid && acc_poid != NULL)
		{
			
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Acc obj is defined");
			PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Acc obj is : ", acc_poid);
			
			PIN_ERRBUF_CLEAR(ebufp);
			aac_out_flist = PIN_FLIST_CREATE(ebufp);
			
			db = PIN_POID_GET_DB(poid_id);
			srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
			aac_in_flistp = PIN_FLIST_CREATE(ebufp);
				
			PIN_FLIST_FLD_PUT(aac_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
			PIN_FLIST_FLD_SET(aac_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
			PIN_FLIST_FLD_SET(aac_in_flistp, PIN_FLD_TEMPLATE, template, ebufp);
			
			args_flistp = PIN_FLIST_ELEM_ADD(aac_in_flistp, PIN_FLD_ARGS, 1, ebufp);
			PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_ACCOUNT_OBJ, acc_poid, ebufp);
			
			args_flistp = PIN_FLIST_ELEM_ADD(aac_in_flistp, PIN_FLD_ARGS, 2, ebufp);
			PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_NAME, pln_name, ebufp);
			
			args_flistp = PIN_FLIST_ELEM_ADD(aac_in_flistp, PIN_FLD_ARGS, 3, ebufp);
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);

			args_flistp = PIN_FLIST_ELEM_ADD(aac_in_flistp, PIN_FLD_ARGS, 4, ebufp);
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, NULL, ebufp);
			
			args_flistp = PIN_FLIST_ELEM_ADD(aac_in_flistp, PIN_FLD_ARGS, 5, ebufp);
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DESCR, sub_prod, ebufp);
			
			result_flistp = PIN_FLIST_ELEM_ADD(aac_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
		
			PIN_FLIST_FLD_SET(result_flistp,PIN_FLD_CYCLE_END_T, NULL, ebufp);
		
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "prod exp date input flist", aac_in_flistp);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, aac_in_flistp, &aac_out_flist, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "prod exp date output flist", aac_out_flist);
		
			result_flistp = PIN_FLIST_ELEM_GET(aac_out_flist, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "prod exp date output flist", result_flistp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account is not active");
				PIN_ERRBUF_CLEAR(ebufp);
				goto CLEANUP;
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Found exp Details");
				cycle_end = *(time_t *)PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_CYCLE_END_T, 0, ebufp);
			}
				
			
			if (cycle_end == 0)
			{
				pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MATCH, PIN_FLD_CODE, 0, 0);
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "error fetching cycle_end_t", ebufp);
				goto CLEANUP;
			}
			else
			{
				time_t		current_t = pin_virtual_time(NULL);
				//curr_date = pin_virtual_time((time_t *)NULL);
				current_t = fm_utils_time_round_to_midnight(current_t);
				//cycle_end = fm_utils_time_round_to_midnight(cycle_end);
				cycle_end = cycle_end - current_t;
				pin_conf("fm_mso_cust","expiry_days", PIN_FLDT_INT, (caddr_t*) &exp_date, &errp);	
				sprintf(msg_v,"test res 1  current_t : %ld",current_t);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg_v);
				sprintf(msg_v,"test res 2 cycle_end : %ld",cycle_end);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg_v);
				sprintf(msg_v,"test res 3 exp_date : %ld",*exp_date);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg_v);
				if (exp_date)
				{
					rem_date = 86400 * (*exp_date);
				}
				
				//sprintf(msg_v,"test res 3 exp_date : %ld",rem_date);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg_v);
				
				//if (cycle_end > 2592000)     //checking whether the difference between the current date and cycle_end date is more than 30(2592000 Sec) days or nor
				//if (*cycle_end  > *(int32*)exp_date)
				if (cycle_end > rem_date) 
				{
					pln_expired = 0;
				}
				else
				{
					pln_expired = 1;
				}
				
				sprintf(msg_v,"test res 3 exp_date : %ld",pln_expired);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg_v);
			} 
	}
	
	CLEANUP:
		if(aac_in_flistp)
		PIN_FLIST_DESTROY_EX(&aac_in_flistp, NULL);
		if(aac_out_flist)
		PIN_FLIST_DESTROY_EX(&aac_out_flist, NULL);
		
		return pln_expired;
}
/*******************************************************************
	 * Excluding H-fiber Plans - Biju.J
*******************************************************************/
void
get_hfiber_plans(
	pcm_context_t   *ctxp,
	pin_flist_t		*plan_flistp,
	pin_flist_t		**out_flistp,
    pin_errbuf_t    *ebufp)
	
	{
	
	/* Changes for H-Fiber Plans */
		int64			db = 1;
		int32			srch_flag = 768;
		int64          pln_obj = 0;
		char		err_str[50]={'\0'};
		poid_t			*srch_pdp = NULL;
		poid_t			*plan_obj_id = NULL;
		pin_flist_t		*result_flistp = NULL;
		pin_flist_t		*args_flistp = NULL;
		pin_flist_t		*hfiber_out_flistp = NULL;
		pin_flist_t		*hfiber_ex_in_flistp = NULL;
		pin_flist_t		*hfiber_ex_out_flistp = NULL;
		pin_flist_t		*hfiber_ex_out_flist = NULL;
		char			*fiberplan_name = NULL;
		char template1[200] = {'\0'};
		
		plan_obj_id = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_POID, 0, ebufp);
		if (plan_obj_id && plan_obj_id != NULL)
		{
			
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Plan obj is defined");
			PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Plan obj is : ", plan_obj_id);
			pln_obj =   PIN_POID_GET_ID(plan_obj_id);

			sprintf(err_str,"Poid is : %lld", pln_obj);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,err_str); 
			
			sprintf(err_str,"%lld", pln_obj);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,err_str); 
			
			 sprintf(template1, "select X from /config_higher_arpu_hfiber where config_higher_arpu_hfiber_t.plan_obj_id = %lld ",pln_obj);
			
			PIN_ERRBUF_CLEAR(ebufp);
			hfiber_ex_out_flist = PIN_FLIST_CREATE(ebufp);
			
			srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
			hfiber_ex_in_flistp = PIN_FLIST_CREATE(ebufp);
				
			PIN_FLIST_FLD_PUT(hfiber_ex_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
			PIN_FLIST_FLD_SET(hfiber_ex_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
			PIN_FLIST_FLD_SET(hfiber_ex_in_flistp, PIN_FLD_TEMPLATE, template1, ebufp);
			
			args_flistp = PIN_FLIST_ELEM_ADD(hfiber_ex_in_flistp, PIN_FLD_ARGS, 1, ebufp);
			PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_ACCOUNT_NUM, &pln_obj, ebufp);
			
			result_flistp = PIN_FLIST_ELEM_ADD(hfiber_ex_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
		
			PIN_FLIST_FLD_SET(result_flistp,PIN_FLD_ACTION_NAME, NULL, ebufp);
		
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "HFiber Plan search input flist", hfiber_ex_in_flistp);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, hfiber_ex_in_flistp, &hfiber_ex_out_flist, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "HFiber Plan search output flist", hfiber_ex_out_flist);
		
			result_flistp = PIN_FLIST_ELEM_GET(hfiber_ex_out_flist, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Plan is not defined");
				PIN_ERRBUF_CLEAR(ebufp);
				*out_flistp = NULL;
				goto CLEANUP;
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Found HFiber Plan");
				fiberplan_name = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_ACTION_NAME, 0, ebufp);
				*out_flistp = PIN_FLIST_COPY(hfiber_ex_out_flist, ebufp);
			}
				
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Return HFiber plan name");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, fiberplan_name);
			
			
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "HFiber Plan search output flist", *out_flistp);
	CLEANUP:
		if(hfiber_ex_in_flistp)
		PIN_FLIST_DESTROY_EX(&hfiber_ex_in_flistp, NULL);
		if(hfiber_ex_out_flistp)
		PIN_FLIST_DESTROY_EX(&hfiber_ex_out_flistp, NULL);
		return;
}

/*******************************************************************
	 * END Excluding H-fiber Plans - Biju.J
*******************************************************************/

/*void
get_promo_adhoc_plans(
	pcm_context_t   *ctxp,
	pin_flist_t		*i_flistp,
	int32 			is_expired,
	chat			*aac_package_val,
	pin_flist_t		**r_adhoc_flistpp,
    pin_errbuf_t    *ebufp)
	
	{
		
		int64			db = 1;
		int32			srch_flag = 768;
		//int64          	pln_obj = 0;
		char			err_str[50]={'\0'};
		poid_t			*srch_pdp = NULL;
		poid_t			*acc_obj_id = NULL;
		pin_flist_t		*result_flistp = NULL;
		pin_flist_t		*args_flistp = NULL;
		pin_flist_t		*adhoc_in_flistp = NULL;
		pin_flist_t		*adhoc_out_flist = NULL;
		pin_flist_t		*pln_array = NULL;
		pin_flist_t		*ad_flistp = NULL;
		char			*fiberplan_name = NULL;
		char 			*adhoc_type = NULL;
		char template1[200] =  "select X from /higher_arpu_promotion_plan where F1 = V1 and F2 = V2 ";
		
		acc_obj_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
		
		if (acc_obj_id && acc_obj_id != NULL)
		{
			if (is_expired == 0 ) // Expiry > 30 Days
			{
				adhoc_type = "R";
			}
			//else if (is_expired != 0 && strcmp(aac_package_val, "Y") == 0 )
			//{
			//	adhoc_type = "P";
			//}
			else
			{
				adhoc_type = "R";
			}
			
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account obj is defined");
			PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "account obj is : ", acc_obj_id);

			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,err_str); 
			
			PIN_ERRBUF_CLEAR(ebufp);
			adhoc_out_flist = PIN_FLIST_CREATE(ebufp);
			
			srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
			adhoc_in_flistp = PIN_FLIST_CREATE(ebufp);
				
			PIN_FLIST_FLD_PUT(adhoc_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
			PIN_FLIST_FLD_SET(adhoc_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
			PIN_FLIST_FLD_SET(adhoc_in_flistp, PIN_FLD_TEMPLATE, template1, ebufp);
			
			args_flistp = PIN_FLIST_ELEM_ADD(adhoc_in_flistp, PIN_FLD_ARGS, 1, ebufp);
			PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_ACCOUNT_OBJ, acc_obj_id, ebufp);
			args_flistp = PIN_FLIST_ELEM_ADD(adhoc_in_flistp, PIN_FLD_ARGS, 2, ebufp);
			PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_CHECK_NO, adhoc_type, ebufp);
			
			result_flistp = PIN_FLIST_ELEM_ADD(adhoc_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
		
			PIN_FLIST_FLD_SET(adhoc_in_flistp, PIN_FLD_PLAN_OBJ, NULL, ebufp); 
			PIN_FLIST_FLD_SET(adhoc_in_flistp, PIN_FLD_NAME, NULL, ebufp);
			PIN_FLIST_FLD_SET(adhoc_in_flistp, PIN_FLD_AMOUNT, NULL, ebufp);
			PIN_FLIST_FLD_SET(adhoc_in_flistp, MSO_FLD_DL_SPEED, NULL, ebufp);
			PIN_FLIST_FLD_SET(adhoc_in_flistp, MSO_FLD_DISPLAY_FLAG, NULL, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Adhoc Plan search input flist", adhoc_in_flistp);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, adhoc_in_flistp, &adhoc_out_flist, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Adhoc Plan search output flist", adhoc_out_flist);
		
			//result_flistp = PIN_FLIST_ELEM_GET(adhoc_out_flist, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);			
			PIN_ERRBUF_CLEAR(ebufp);
			ad_flistp = PIN_FLIST_CREATE(ebufp);
			
			
			while((pln_array = PIN_FLIST_ELEM_GET_NEXT(adhoc_out_flist, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp))!= (pin_flist_t *)NULL) 
			{
				//res_total = res_total + 1 ;	
				
				res_flistp = PIN_FLIST_ELEM_ADD(ad_flistp, PIN_FLD_PLAN, res_total, ebufp);
				PIN_FLIST_FLD_COPY(pln_array, PIN_FLD_PLAN_OBJ, res_flistp, PIN_FLD_PLAN_OBJ, ebufp);
		        PIN_FLIST_FLD_COPY(pln_array, PIN_FLD_NAME, res_flistp, PIN_FLD_NAME, ebufp);
			    PIN_FLIST_FLD_COPY(pln_array, PIN_FLD_AMOUNT, res_flistp, PIN_FLD_AMOUNT, ebufp);
				PIN_FLIST_FLD_COPY(pln_array, MSO_FLD_DL_SPEED, res_flistp, MSO_FLD_DL_SPEED, ebufp);
				PIN_FLIST_FLD_SET(res_flistp, MSO_FLD_DISPLAY_FLAG, "1", ebufp);
				
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "TEST Flist -1", ad_flistp);
			}

			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Plan is not defined");
				PIN_ERRBUF_CLEAR(ebufp);
				*r_adhoc_flistpp = NULL;
				goto CLEANUP;
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Adhoc Plans fould");
				*r_adhoc_flistpp = PIN_FLIST_COPY(adhoc_out_flist, ebufp);
			}
			
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Adhoc Plan search output flist", *r_adhoc_flistpp);
	CLEANUP:
		if(hfiber_ex_in_flistp)
		PIN_FLIST_DESTROY_EX(&adhoc_in_flistp, NULL);
		return;
}
*/

void
get_source_plan_charge(
    pcm_context_t   *ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**out_flistp,
    pin_errbuf_t    *ebufp)
{

	char			template[200] = "select X from /mso_cfg_credit_limit where F1 = V1 and ( F2 = V2 or F3 = V3 ) and F4 = V4 ";
	int32			srch_flag = 768;
	int64			db = 1;
	poid_t			*srch_pdp = NULL;
	pin_flist_t		*srch_plan_details_in_flistp = NULL;
	pin_flist_t		*srch_plan_details_out_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*cities_flistp = NULL;
	pin_flist_t     *err_flistp = NULL;
	pin_flist_t     *r_flistp = NULL;
	pin_flist_t     *ret_flistp = NULL;
	char			*plan_city = NULL;
	char			all_cities[5] = "*";
	char			*plan_name = NULL;
	char			*city = NULL;
	pin_decimal_t	*plan_charge = NULL;
	pin_decimal_t	*all_cities_plan_charge = NULL;
	pin_decimal_t	*dl_speed= NULL;
	int32			elem_id_1 = 0;
    int32           chk_payterm = -1;
	pin_cookie_t	cookie_1 = NULL;
	double		    plan_payterm = 1;
	int32			payterm = 0;
	int				error_status = 1;
	int				success_status = 0;
	int				status = 0;
	void			*vp = NULL;
	
	PIN_ERRBUF_CLEAR(ebufp);
	*out_flistp = PIN_FLIST_CREATE(ebufp);
	plan_name = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_PLAN_NAME, 0, ebufp);
	city = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CITY, 0, ebufp);
	payterm  = *(int32 *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);

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
	
	args_flistp = PIN_FLIST_ELEM_ADD(srch_plan_details_in_flistp, PIN_FLD_ARGS, 4, ebufp);
	cities_flistp = PIN_FLIST_ELEM_ADD(args_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(cities_flistp,PIN_FLD_BILL_WHEN, &payterm, ebufp);

	result_flistp = PIN_FLIST_ELEM_ADD(srch_plan_details_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	cities_flistp = PIN_FLIST_ELEM_ADD(result_flistp, MSO_FLD_CITIES, PCM_RECID_ALL, ebufp);
	PIN_FLIST_FLD_SET(cities_flistp,PIN_FLD_CITY, NULL, ebufp);
	PIN_FLIST_FLD_SET(cities_flistp,PIN_FLD_BILL_WHEN, NULL, ebufp);
	PIN_FLIST_FLD_SET(cities_flistp,PIN_FLD_CHARGE_AMT, NULL, ebufp);
	PIN_FLIST_FLD_SET(cities_flistp, MSO_FLD_DL_SPEED, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Plan search input flist", srch_plan_details_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_plan_details_in_flistp, &srch_plan_details_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Plan search output flist", srch_plan_details_out_flistp);

//	ret_flistp = PIN_FLIST_CREATE(ebufp);

	result_flistp = PIN_FLIST_ELEM_GET(srch_plan_details_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		err_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, err_flistp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "511022", ebufp);
        PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Search Plan Doesn't suit the Payterm Passed", ebufp);
        PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &error_status, ebufp);

        *out_flistp = PIN_FLIST_COPY(err_flistp, ebufp);
        
        PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
        goto CLEANUP;
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Processing the Individual Plans");
	    while ((cities_flistp = PIN_FLIST_ELEM_GET_NEXT(result_flistp, MSO_FLD_CITIES,
			&elem_id_1, 1, &cookie_1, ebufp)) != (pin_flist_t *)NULL)
	    {
		plan_city = PIN_FLIST_FLD_GET(cities_flistp, PIN_FLD_CITY, 0, ebufp);
        	vp = PIN_FLIST_FLD_GET(cities_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);
		if (vp && vp != NULL)//akshayrc
			chk_payterm = *(int32 *)vp;
		if((strcmp(plan_city, all_cities) == 0) && (payterm == chk_payterm))
        	{
			all_cities_plan_charge = pbo_decimal_copy(PIN_FLIST_FLD_GET(cities_flistp, PIN_FLD_CHARGE_AMT, 0, ebufp), ebufp);
			//plan_payterm = (double)*(int32 *)PIN_FLIST_FLD_GET(cities_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);
			plan_payterm = (double)chk_payterm;
			dl_speed = PIN_FLIST_FLD_GET(cities_flistp, MSO_FLD_DL_SPEED, 0, ebufp);
		} 
        	else if((strcmp(plan_city, city) == 0) && (payterm == chk_payterm))
        	{
			plan_charge = pbo_decimal_copy(PIN_FLIST_FLD_GET(cities_flistp, PIN_FLD_CHARGE_AMT, 0, ebufp), ebufp);
			//plan_payterm = (double)*(int32 *)PIN_FLIST_FLD_GET(cities_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);
			plan_payterm = (double)chk_payterm;
			dl_speed = PIN_FLIST_FLD_GET(cities_flistp, MSO_FLD_DL_SPEED, 0, ebufp);
			break;
		}
	    }
	    if(plan_charge == NULL)
	    {
		plan_charge = all_cities_plan_charge;
	    }

		// Divide the total charge by payterm.
		pbo_decimal_divide_assign(plan_charge, pbo_decimal_from_double(plan_payterm, ebufp), ebufp);

		PIN_FLIST_FLD_SET(*out_flistp, PIN_FLD_AMOUNT, plan_charge, ebufp);
		if(pbo_decimal_is_null(dl_speed, ebufp)){
			err_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, err_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "511022", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Search Plan Download Speed Retrieval Error", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &error_status, ebufp);

			*out_flistp = PIN_FLIST_COPY(err_flistp, ebufp);
			
			PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
			goto CLEANUP;
			
		}
		else{
			PIN_FLIST_FLD_SET(*out_flistp, MSO_FLD_DL_SPEED, dl_speed, ebufp);
			PIN_FLIST_FLD_SET(*out_flistp, PIN_FLD_STATUS, &success_status, ebufp);
		}
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Getting the Download Speed");
//	r_flistp = PIN_FLIST_CREATE(ebufp);
	/*mso_get_dl_speed(ctxp, i_flistp, &r_flistp, ebufp);
	vp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_STATUS, 0, ebufp);
	if (vp)
		status = *(int32 *)vp;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		err_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, err_flistp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "511022", ebufp);
        PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Search Plan Download Speed Retrieval Error", ebufp);
        PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &error_status, ebufp);

        *out_flistp = PIN_FLIST_COPY(err_flistp, ebufp);
        
        PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
        goto CLEANUP;
	}
	else
	{
		if (status == 1)
		{
			err_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, err_flistp, PIN_FLD_POID, ebufp);
	        PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "511022", ebufp);
		    PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Search Plan Download Speed Retrieval Error", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &error_status, ebufp);

	        *out_flistp = PIN_FLIST_COPY(err_flistp, ebufp);
		    
			PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
	        goto CLEANUP;
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Getting the Download Speed - Success");
			PIN_FLIST_FLD_COPY(r_flistp, MSO_FLD_DL_SPEED, *out_flistp, MSO_FLD_DL_SPEED, ebufp);
			PIN_FLIST_FLD_SET(*out_flistp, PIN_FLD_STATUS, &success_status, ebufp);

//			*out_flistp = PIN_FLIST_COPY(ret_flistp, ebufp);
		    
//			PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
		}

	}*/

CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_plan_details_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_plan_details_out_flistp, NULL);
	return;
}

void
extract_search_plan_charge(
    pcm_context_t   *ctxp,
	pin_flist_t	    *plan_info_flistp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistp,
    pin_errbuf_t    *ebufp) 
{
    char            *plan_city = NULL;
    char            all_cities[5] = "*";
	char			*plan_name = NULL;
	poid_t			*planinfo_flistp = NULL;
	pin_flist_t		*cities_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*err_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;
	pin_decimal_t   *all_cities_plan_charge = NULL;
	pin_decimal_t   *plan_charge = NULL;
	double			plan_payterm = 1;
	int32			elem_id_1 = 0;
    int32           chk_payterm = -1;
	pin_cookie_t	cookie_1 = NULL;
	char            *city = NULL;
    int32           payterm = 0;
	int32			status = 0;
	int				error_status = 1;
	int				success_status = 0;
	void		*vp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);
	
	payterm = *(int32 *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);
	city = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_CITY, 0, ebufp);
	plan_name = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PLAN_NAME, 0, ebufp);
	
	*out_flistp = PIN_FLIST_CREATE(ebufp);

	planinfo_flistp = PIN_FLIST_SUBSTR_GET(plan_info_flistp, MSO_FLD_PLAN_INFO, 0, ebufp);
	
	if (planinfo_flistp)
	{
		while ((cities_flistp = PIN_FLIST_ELEM_GET_NEXT(planinfo_flistp, MSO_FLD_CITIES,
                        &elem_id_1, 1, &cookie_1, ebufp)) != (pin_flist_t *)NULL)
		{
			plan_city = PIN_FLIST_FLD_GET(cities_flistp, PIN_FLD_CITY, 0, ebufp);
	        	vp = PIN_FLIST_FLD_GET(cities_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);
			if (vp)
				chk_payterm = *(int32 *)vp;
		    if(plan_city && (strcmp(plan_city, all_cities) == 0) && (payterm == chk_payterm))  
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Matched: All Cities");
	            all_cities_plan_charge = pbo_decimal_copy(PIN_FLIST_FLD_GET(cities_flistp, PIN_FLD_CHARGE_AMT, 0, ebufp), ebufp);
		        //plan_payterm = (double)*(int32 *)PIN_FLIST_FLD_GET(cities_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);
			plan_payterm = (double)chk_payterm;
			} 
	        else if(plan_city && (strcmp(plan_city, city) == 0) && (payterm == chk_payterm))
		    {
			    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Matched: Specific Cities");
				plan_charge = pbo_decimal_copy(PIN_FLIST_FLD_GET(cities_flistp, PIN_FLD_CHARGE_AMT, 0, ebufp), ebufp);
	            	//	plan_payterm = (double)*(int32 *)PIN_FLIST_FLD_GET(cities_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);
				plan_payterm = (double)chk_payterm;
		        break;
			}
		}	
	    if(plan_charge == NULL)
		{
	    plan_charge = all_cities_plan_charge;
		}

	    // Divide the total charge by payterm.
		pbo_decimal_divide_assign(plan_charge, pbo_decimal_from_double(plan_payterm, ebufp), ebufp);

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Getting the Download Speed");
//		r_flistp = PIN_FLIST_CREATE(ebufp);
		fm_mso_get_service_quota_codes(ctxp, plan_name, &payterm, city, &r_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_service_quota_codes", r_flistp);
		/*mso_get_dl_speed(ctxp, in_flistp, &r_flistp, ebufp);
		vp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_STATUS, 1, ebufp);
		if (vp)
			status = *(int32 *)vp;
	*/
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			err_flistp = PIN_FLIST_CREATE(ebufp);
	        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, err_flistp, PIN_FLD_POID, ebufp);
		    PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "511022", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Search Plan Download Speed Retrieval Error", ebufp);
	        PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &error_status, ebufp);

		    *out_flistp = PIN_FLIST_COPY(err_flistp, ebufp);
        
			PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
	        goto CLEANUP;
		}
		else
		{
				PIN_FLIST_FLD_SET(*out_flistp, PIN_FLD_AMOUNT, plan_charge, ebufp);
				PIN_FLIST_FLD_SET(*out_flistp, PIN_FLD_STATUS, &success_status, ebufp);
				if(r_flistp)
					PIN_FLIST_FLD_COPY(r_flistp, MSO_FLD_DL_SPEED, *out_flistp, MSO_FLD_DL_SPEED, ebufp);
				else
					PIN_FLIST_FLD_SET(*out_flistp, MSO_FLD_DL_SPEED, NULL, ebufp);

				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Getting the Download Speed - Success");

			/*if (status == 1)
			{
				err_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, err_flistp, PIN_FLD_POID, ebufp);
		        PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "511022", ebufp);
			    PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Search Plan Download Speed Retrieval Error", ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &error_status, ebufp);

		        *out_flistp = PIN_FLIST_COPY(err_flistp, ebufp);
		    
				PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
			    goto CLEANUP;
			}
			else
			{
//				ret_flistp = PIN_FLIST_CREATE(ebufp);
				
				PIN_FLIST_FLD_SET(*out_flistp, PIN_FLD_AMOUNT, plan_charge, ebufp);
				PIN_FLIST_FLD_COPY(r_flistp, MSO_FLD_DL_SPEED, *out_flistp, MSO_FLD_DL_SPEED, ebufp);
				PIN_FLIST_FLD_SET(*out_flistp, PIN_FLD_STATUS, &success_status, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Getting the Download Speed - Success");

//				*out_flistp = PIN_FLIST_COPY(ret_flistp, ebufp);
		    
//				PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
			}*/

		}

	}
CLEANUP:
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
}

void
mso_get_dl_speed(
	pcm_context_t   *ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistp,
	pin_errbuf_t    *ebufp)
{
	int32			srch_flag = 768;
	int64			db = 1;
	poid_t			*srch_pt_pdp = NULL;
	pin_flist_t		*srch_pt_in_flistp = NULL;
	pin_flist_t		*srch_pt_out_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*pdts_flistp = NULL;
	pin_flist_t		*services_flistp = NULL;
	pin_flist_t		*err_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;
	int				error_status = 1;
	int				success_status = 0;
	char			template1[200] = "select X from /mso_config_bb_pt 1, /product 2, /deal 3, /plan 4 where 1.F1 = 2.F2 and 2.F3 = 3.F4 and 3.F5 = 4.F6 and 4.F7 = V7";
	char			*plan_name = NULL;


	PIN_ERRBUF_RESET(ebufp);
	*r_flistp = PIN_FLIST_CREATE(ebufp);
	plan_name = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_PLAN_NAME, 1, ebufp);

	srch_pt_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_pt_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_pt_in_flistp, PIN_FLD_POID, srch_pt_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_pt_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_pt_in_flistp, PIN_FLD_TEMPLATE, template1, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_pt_in_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PROVISIONING_TAG, plan_name, ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(srch_pt_in_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PROVISIONING_TAG, plan_name, ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(srch_pt_in_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(srch_pt_in_flistp, PIN_FLD_ARGS, 4, ebufp);
	pdts_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(pdts_flistp,PIN_FLD_PRODUCT_OBJ, NULL, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_pt_in_flistp, PIN_FLD_ARGS, 5, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(srch_pt_in_flistp, PIN_FLD_ARGS, 6, ebufp);
	services_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(services_flistp, PIN_FLD_DEAL_OBJ, NULL, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_pt_in_flistp, PIN_FLD_ARGS, 7, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_NAME, plan_name, ebufp);

	res_flistp = PIN_FLIST_ELEM_ADD(srch_pt_in_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, MSO_FLD_DL_SPEED, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Prov_Tag search input flist", srch_pt_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_pt_in_flistp, &srch_pt_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Prov_Tag search output flist", srch_pt_out_flistp);

	result_flistp = PIN_FLIST_ELEM_GET(srch_pt_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		err_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, err_flistp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "511022", ebufp);
        PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Search Plan Doesn't suit the Provisioning Tag Passed", ebufp);
        PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &error_status, ebufp);

        *r_flistp = PIN_FLIST_COPY(err_flistp, ebufp);
        
        PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
        goto CLEANUP;
	}
	else if (!result_flistp || result_flistp == NULL)
	{
		err_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, err_flistp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "511022", ebufp);
        PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Search Plan Doesn't suit the Provisioning Tag Passed", ebufp);
        PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &error_status, ebufp);

        *r_flistp = PIN_FLIST_COPY(err_flistp, ebufp);
        
        PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
        goto CLEANUP;
	}
	else
	{
//		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_STATUS, &success_status, ebufp);
		PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_POID, *r_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(result_flistp, MSO_FLD_DL_SPEED, *r_flistp, MSO_FLD_DL_SPEED, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Download Speed - Assigned");
		
//		*r_flistp = PIN_FLIST_COPY(ret_flistp, ebufp);
        
//		PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);

	}
	

CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_pt_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_pt_out_flistp, NULL);
}



