/*******************************************************************
 * File:	fm_mso_cust_get_plan_details_list.c
 * Opcode:	MSO_OP_CUST_GET_PLAN_DETAILS_LIST 
 * Description: This opcode is for fetching a plan details 
 *******************************************************************/
/*******************************************************************

r << xx 1
0 PIN_FLD_POID           POID [0] 0.0.0.1 /plan 6431189943 0
xx
xop MSO_OP_CUST_GET_PLAN_DETAILS_LIST 0 1


#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_get_plan_details_list.c:ServerIDCVelocityInt:4:2006-Sep-14 11:34:45 %";
#endif
SELECT a.name,c.AMOUNT,c.CITY,c.PAYTERM,c.UL_SPEED,c.DL_SPEED FROM PLAN_T a, MSO_CFG_CREDIT_LIMIT_T b, MSO_CFG_CREDIT_LIMIT_CITIES_T c 
where a.POID_ID0=b.PLAN_OBJ_ID0 and b.POID_ID0=c.OBJ_ID0
and a.POID_ID0=6431189943;
/*******************************************************************
 * Contains the MSO_OP_CUST_GET_PLAN_DETAILS_LIST operation.
 *******************************************************************/

#include <stdio.h>
#include <string.h>

#include <pcm.h>
#include <pinlog.h>

#define FILE_LOGNAME "fm_mso_cust_get_plan_details_list.c(1)"

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
#include "pinlog.h"
#include "pcm_ops.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "mso_cust.h"
#include "mso_prov.h"
#include "mso_lifecycle_id.h"
#include "mso_device.h"
#include "mso_utils.h"
#include "mso_errs.h"


/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_mso_cust_get_plan_details_list(
    cm_nap_connection_t    *connp,
    int32            opcode,
    int32            flags,
    pin_flist_t        *i_flistp,
    pin_flist_t        **r_flistpp,
    pin_errbuf_t        *ebufp);

static void
fm_mso_cust_get_plan_details_list(
    pcm_context_t           *ctxp,
		int32            flags,
        pin_flist_t		*i_flistp,
		pin_flist_t		**r_flistpp,
        pin_errbuf_t            *ebufp);

void
fm_mso_get_prod_details_from_plan(
    pcm_context_t           *ctxp,
    pin_flist_t             *in_flist,
    pin_flist_t             **ret_flistp,
    pin_errbuf_t            *ebufp);

void
fm_mso_get_priority_details(
    pcm_context_t           *ctxp,
    pin_flist_t             *in_flist,
    pin_flist_t             **ret_flistp,
    pin_errbuf_t            *ebufp);

void
fm_mso_plan_credit_limit_details(
    pcm_context_t           *ctxp,
    pin_flist_t             *in_flist,
    pin_flist_t             **ret_flistp,
    pin_errbuf_t            *ebufp);

/*******************************************************************
 * Main routine for the MSO_OP_CUST_GET_PLAN_DETAILS operation.
 *******************************************************************/
void
op_mso_cust_get_plan_details_list(
    cm_nap_connection_t    *connp,
    int32            opcode,
    int32            flags,
    pin_flist_t        *i_flistp,
    pin_flist_t        **r_flistpp,
    pin_errbuf_t        *ebufp)
{
    pcm_context_t        *ctxp = connp->dm_ctx;
    pin_flist_t    *r_flistp = NULL;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;
    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Null out results
     ***********************************************************/
    *r_flistpp = NULL;

    /***********************************************************
     * Insanity check.
     ***********************************************************/
    if (opcode != MSO_OP_CUST_GET_PLAN_DETAILS_LIST) {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
            PIN_ERR_BAD_OPCODE, 0, 0, opcode);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_mso_cust_get_plan_details_list opcode error", ebufp);

        return;
    }
    
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "op_mso_cust_get_plan_details_list input flist", i_flistp);

    /***********************************************************
     * Main routine for this opcode
     ***********************************************************/
    fm_mso_cust_get_plan_details_list(ctxp, flags, i_flistp, r_flistpp, ebufp);
    
    /***********************************************************
     * Error?
     ***********************************************************/
    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        PIN_FLIST_DESTROY_EX(r_flistpp, ebufp);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_mso_cust_get_plan_details_list error", ebufp);
    } else {
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "op_mso_cust_get_plan_details_list output flist", *r_flistpp);
    }

    return;
}


/*******************************************************************************
 * fm_mso_cust_get_plan_details_list()
 * Gets Plan details using Plan poid 
 ******************************************************************************/
static void
fm_mso_cust_get_plan_details_list(
        pcm_context_t           *ctxp,
	int32            flags,
        pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t		*details_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;
        pin_flist_t             *p_flistp = NULL;
        pin_flist_t             *ss_flistp = NULL;
        pin_flist_t             *l_flistp = NULL;
        pin_flist_t             *x_flistp = NULL;
        pin_flist_t             *prod_flistp = NULL;
        pin_flist_t             *priority_flistp = NULL;
        pin_flist_t             *return_flistp = NULL;
        pin_flist_t             *prod_details_flistp = NULL;
        pin_flist_t             *output_flistp = NULL;
        pin_flist_t             *pcd_flistp = NULL;
        pin_flist_t             *sub_flistp = NULL;

        int32                   status_failure = 1;
        int32                   status_success = 0;

	poid_t                  *plan_poid = NULL;
	poid_t			*srch_pdp = NULL;
        int32                   s_flags = 256;
	char                    *plan_type = NULL;
        pin_decimal_t           *priority = NULL;
        int                      priority_int = -1;
        char                    *tmp_str = NULL;
        int             elemid = 0;
        pin_cookie_t    cookie = NULL;
		
	plan_poid = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

	if (plan_poid == NULL || (PIN_POID_GET_ID(plan_poid) == 0)){
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Plan poid is null") ;
	        ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID,ret_flistp , PIN_FLD_POID, ebufp);
	        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
        	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51100", ebufp);
	        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Plan poid is mandatory", ebufp);
                *r_flistpp = PIN_FLIST_COPY(ret_flistp,ebufp);
        	goto CLEANUP;
	}
		

	fm_mso_get_prod_details_from_plan(ctxp,i_flistp, &prod_details_flistp,ebufp);

	if((prod_details_flistp == NULL)|| (PIN_FLIST_ELEM_COUNT(prod_details_flistp, PIN_FLD_RESULTS, ebufp) == 0)){
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No products and deals for the plan") ;
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51101", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Input poid is not a valid plan poid ", ebufp);
	        *r_flistpp = PIN_FLIST_COPY(ret_flistp,ebufp);
                goto CLEANUP;
	}  		

        output_flistp= PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, output_flistp, PIN_FLD_POID, ebufp);
	
	while ((prod_flistp = PIN_FLIST_ELEM_TAKE_NEXT(prod_details_flistp, PIN_FLD_RESULTS, &elemid, 1, &cookie, ebufp))!= (pin_flist_t *)NULL) {
		
	        fm_mso_get_priority_details(ctxp,prod_flistp, &return_flistp,ebufp);
	        if((return_flistp == NULL)|| (PIN_FLIST_ELEM_COUNT(return_flistp, PIN_FLD_RESULTS, ebufp) == 0)){
        	        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Product priority details not available in plan_priority table") ;
                	ret_flistp = PIN_FLIST_CREATE(ebufp);
	                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp);
        	        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
                	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51102", ebufp);
	                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Plan priority details are not available in brm ", ebufp);
        	        *r_flistpp = PIN_FLIST_COPY(ret_flistp,ebufp);
                	goto CLEANUP;
	        }

		priority_flistp = PIN_FLIST_ELEM_GET(return_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	        priority = PIN_FLIST_FLD_GET(priority_flistp, PIN_FLD_PRIORITY, 0, ebufp);
                plan_type = PIN_FLIST_FLD_GET(priority_flistp, PIN_FLD_PIN2, 0, ebufp);

	        tmp_str = pbo_decimal_to_str(priority, ebufp);
        	priority_int = atoi(tmp_str);
	
		/*if (priority > 1000){
		
			if (plan_type == "BASE"){
			
                		PIN_FLIST_FLD_COPY(priority_flistp, PIN_FLD_PDP_TYPE,output_flistp, PIN_FLD_PDP_TYPE, ebufp);
                		PIN_FLIST_FLD_COPY(priority_flistp, PIN_FLD_PIN2, output_flistp, PIN_FLD_PIN2, ebufp);
		                PIN_FLIST_FLD_COPY(priority_flistp, PIN_FLD_PIPELINE_CATEGORY, output_flistp, PIN_FLD_PIPELINE_CATEGORY, ebufp);
                		PIN_FLIST_FLD_COPY(priority_flistp, PIN_FLD_PIPELINE_NAME, output_flistp, PIN_FLD_PIPELINE_NAME, ebufp);
		                PIN_FLIST_FLD_COPY(priority_flistp, PIN_FLD_PRIORITY, output_flistp, PIN_FLD_PRIORITY, ebufp);
				break;
			}
					
		}
		else{*/

                PIN_FLIST_FLD_COPY(priority_flistp, PIN_FLD_PDP_TYPE, output_flistp, PIN_FLD_PDP_TYPE, ebufp);
                PIN_FLIST_FLD_COPY(priority_flistp, PIN_FLD_PIN2, output_flistp, PIN_FLD_PIN2, ebufp);
                PIN_FLIST_FLD_COPY(priority_flistp, PIN_FLD_PIPELINE_CATEGORY, output_flistp, PIN_FLD_PIPELINE_CATEGORY, ebufp);
                PIN_FLIST_FLD_COPY(priority_flistp, PIN_FLD_PIPELINE_NAME, output_flistp, PIN_FLD_PIPELINE_NAME, ebufp);
                PIN_FLIST_FLD_COPY(priority_flistp, PIN_FLD_PRIORITY, output_flistp, PIN_FLD_PRIORITY, ebufp);

		if ( priority_int > 1000){
			if (plan_type && strcmp(plan_type, "BASE") == 0 ){
				break;
			}
		}
		else{
			break;	
		}				
	        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "details_flistp : ", output_flistp);
	}

        fm_mso_plan_credit_limit_details(ctxp,i_flistp, &pcd_flistp,ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "details_flistp : ", pcd_flistp);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Details -- 1");

        sub_flistp = PIN_FLIST_ELEM_GET(pcd_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
        PIN_FLIST_FLD_DROP(sub_flistp,PIN_FLD_POID, ebufp);

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Details -- 2");
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "details_flistp : ", output_flistp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "details_flistp : ", sub_flistp);
	
	PIN_FLIST_CONCAT(output_flistp, sub_flistp, ebufp);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Details -- 3");

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "details_flistp : ", output_flistp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Plan Details Error",*r_flistpp);
		return;
	}
			
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Found Plan Details -- 1");
	*r_flistpp = PIN_FLIST_COPY(output_flistp,ebufp);

        CLEANUP:
        PIN_FLIST_DESTROY_EX(&ret_flistp,NULL);
        PIN_FLIST_DESTROY_EX(&output_flistp,NULL);
	return;

}

void
fm_mso_get_prod_details_from_plan(
    	pcm_context_t           *ctxp,
	pin_flist_t             *in_flist,
    	pin_flist_t             **ret_flistp,
	pin_errbuf_t            *ebufp)
{
        pin_flist_t     *srch_iflistp=NULL;
        pin_flist_t     *srch_oflistp=NULL;
        pin_flist_t     *arg_flist=NULL;
        pin_flist_t     *sub_flistp=NULL;

        char    template[512]={'\0'};
        char    prov_tag[30]="";
        int64   db = 1;
        int32   srch_flag = 256;
        poid_t      *plan_pdp = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in before searching prov tag", ebufp);
        }

	plan_pdp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 0, ebufp);

        srch_iflistp= PIN_FLIST_CREATE(ebufp);

        PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);

        PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        strcpy(template,"select X from /product 1,/deal 2,/plan 3 where 2.F2 = 1.F1 and 3.F4 = 2.F3 and 3.F5 = V5");

        PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, template, ebufp);

        arg_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp);  //Product poid
        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL, ebufp);

        arg_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 2, ebufp);
        sub_flistp = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_PRODUCTS, 0, ebufp);
        PIN_FLIST_FLD_PUT(sub_flistp,PIN_FLD_PRODUCT_OBJ, NULL, ebufp);

        arg_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 3, ebufp);  //deal poid
        PIN_FLIST_FLD_SET(arg_flist,PIN_FLD_POID, NULL, ebufp);

        arg_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 4, ebufp);
        sub_flistp = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_SERVICES, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(sub_flistp,PIN_FLD_DEAL_OBJ, NULL, ebufp);

        arg_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 5, ebufp); //plan poid
        PIN_FLIST_FLD_SET(arg_flist,PIN_FLD_POID, plan_pdp, ebufp);

        arg_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS,PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(arg_flist,PIN_FLD_POID, NULL, ebufp);
        PIN_FLIST_FLD_SET(arg_flist,PIN_FLD_NAME, NULL, ebufp);
        PIN_FLIST_FLD_SET(arg_flist,PIN_FLD_DESCR, NULL, ebufp);
        PIN_FLIST_FLD_SET(arg_flist,PIN_FLD_PRIORITY, NULL, ebufp);
        PIN_FLIST_FLD_SET(arg_flist,PIN_FLD_PROVISIONING_TAG, NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_prov_tag_from_plan search input flist is : ", srch_iflistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_prov_tag_from_plan search output flist is : ", srch_oflistp);


        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH for provision tag", ebufp);
        }
        *ret_flistp = PIN_FLIST_COPY(srch_oflistp,ebufp);

        CLEANUP:
        PIN_FLIST_DESTROY_EX(&srch_iflistp,NULL);
        PIN_FLIST_DESTROY_EX(&srch_oflistp,NULL);
	return;

}

void
fm_mso_get_priority_details(
    pcm_context_t           *ctxp,
    pin_flist_t             *in_flist,
    pin_flist_t             **ret_flistp,
    pin_errbuf_t            *ebufp)
{
	char template[200] = "select X from /mso_plan_priority where F1 = V1 ";
	pin_flist_t     *srch_iflistp=NULL;
	pin_flist_t     *srch_oflistp=NULL;
	poid_t 		*pdp = NULL;
	poid_t 		*srch_pdp = NULL;
	pin_flist_t 	*arg_flistp = NULL;
        int32                   s_flags = 256;

    	pdp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 0, ebufp);

	srch_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(pdp), "/search", -1, ebufp);
	srch_iflistp = PIN_FLIST_CREATE(ebufp);
				
	PIN_FLIST_FLD_PUT(srch_iflistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &s_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, template, ebufp);
			
	arg_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PRIORITY, arg_flistp, PIN_FLD_PRIORITY, ebufp);

	PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_priority_details input flist", srch_iflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 	"fm_mso_get_priority_details Output flist",srch_oflistp);	

        *ret_flistp = PIN_FLIST_COPY(srch_oflistp,ebufp);

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_iflistp,NULL);
	PIN_FLIST_DESTROY_EX(&srch_oflistp,NULL);
	return;

}

void
fm_mso_plan_credit_limit_details(
    pcm_context_t           *ctxp,
    pin_flist_t             *in_flist,
    pin_flist_t             **ret_flistp,
    pin_errbuf_t            *ebufp)
{
        pin_flist_t     *srch_iflistp=NULL;
        pin_flist_t     *srch_oflistp=NULL;
        poid_t          *plan_poid = NULL;
        poid_t          *srch_pdp = NULL;
        pin_flist_t     *arg_flistp = NULL;
        pin_flist_t     *cities_flistp = NULL;
        pin_flist_t     *cp_flistp = NULL;
        int32            s_flags = 256;
		char template[200] = "select X from /mso_cfg_credit_limit where F1 = V1 ";

        plan_poid = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 0, ebufp);

        srch_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(plan_poid), "/search", -1, ebufp);
        srch_iflistp = PIN_FLIST_CREATE(ebufp);

        PIN_FLIST_FLD_PUT(srch_iflistp, PIN_FLD_POID, srch_pdp, ebufp);
        PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &s_flags, ebufp);
        PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, template, ebufp);

        arg_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_PLAN_OBJ, plan_poid, ebufp);

        arg_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_PLAN_NAME, NULL, ebufp);
        cities_flistp = PIN_FLIST_ELEM_ADD(arg_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(cities_flistp,PIN_FLD_CITY, NULL, ebufp);
        PIN_FLIST_FLD_SET(cities_flistp,PIN_FLD_BILL_WHEN, NULL, ebufp);
        PIN_FLIST_FLD_SET(cities_flistp,PIN_FLD_CHARGE_AMT, NULL, ebufp);
        PIN_FLIST_FLD_SET(cities_flistp,MSO_FLD_FUP_DL_SPEED, NULL, ebufp);
        PIN_FLIST_FLD_SET(cities_flistp,MSO_FLD_FUP_UL_SPEED, NULL, ebufp);
        PIN_FLIST_FLD_SET(cities_flistp,MSO_FLD_DL_SPEED, NULL, ebufp);
        PIN_FLIST_FLD_SET(cities_flistp,MSO_FLD_UL_SPEED, NULL, ebufp);
        PIN_FLIST_FLD_SET(cities_flistp,MSO_FLD_QUOTA_CODE, NULL, ebufp);
        cp_flistp = PIN_FLIST_ELEM_ADD(cities_flistp, MSO_FLD_CREDIT_PROFILE,PIN_ELEMID_ANY, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_plan_credit_limit_details input flist", srch_iflistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,  "fm_mso_plan_credit_limit_details Output flist",srch_oflistp);

        *ret_flistp = PIN_FLIST_COPY(srch_oflistp,ebufp);

        CLEANUP:
        PIN_FLIST_DESTROY_EX(&srch_iflistp,NULL);
        PIN_FLIST_DESTROY_EX(&srch_oflistp,NULL);
        return;
}
             
