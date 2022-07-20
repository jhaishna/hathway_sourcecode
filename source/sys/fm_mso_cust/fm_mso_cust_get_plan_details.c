/*******************************************************************
 * File:	fm_mso_cust_get_plan_details.c
 * Opcode:	MSO_OP_CUST_REGISTER_CUSTOMER 
 * Owner:	Gautam Adak
 * Created:	17-SEPT-2013
 * Description: 
 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_get_plan_details.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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

#define NAME  0
#define CODE  1
#define DESC  2
#define CREATION_TIME 3
#define MODIFICATION_TIME 4
#define POID 5

#define EQUALS	     0
#define STARTS_WITH  1
#define ENDS_WITH    2
#define CONTAINS     3

#define CATV  1
#define BB 2




/**************************************
* External Functions start
***************************************/
extern void
fm_mso_get_products_from_deal(
	pcm_context_t		*ctxp,
	int32			fliter_products,
	int32			add_info_flag,
	pin_flist_t		*in_flistp,
	pin_errbuf_t		*ebufp);
/**************************************
* External Functions end
***************************************/

/*******************************************************************
 * MSO_OP_CUST_GET_PLAN_DETAILS 
 *
 * 
 *******************************************************************/

EXPORT_OP void
op_mso_cust_get_plan_details(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_get_plan_details(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
generate_higher_arpu_template(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t             **ret_flistp,
	pin_errbuf_t		*ebufp);

int32
fm_mso_cust_get_valid_tech(
        pcm_context_t           *ctxp,
        poid_t                  *pdp,
        char                    *prov,
        int                     *tech,
        pin_errbuf_t            *ebufp);

/*******************************************************************
 * MSO_OP_CUST_GET_PLAN_DETAILS  
 *******************************************************************/
void 
op_mso_cust_get_plan_details(
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
	if (opcode != MSO_OP_CUST_GET_PLAN_DETAILS) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_get_plan_details error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_get_plan_details input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	fm_mso_cust_get_plan_details(ctxp, flags, i_flistp, r_flistpp, ebufp);
 	/***********************************************************
	 * Results.
	 ***********************************************************/

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_get_plan_details error", ebufp);
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_get_plan_details output flist", *r_flistpp);
	}
	return;
}



/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_get_plan_details(
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
	pin_flist_t		*city_array = NULL;

	poid_t			*service_pdp = NULL;
	poid_t			*srch_pdp = NULL;
	poid_t			*plan_pdp = NULL;
	char			*srch_val = NULL;
	char			*curr_city = NULL;
	char			template[100] ;
	pin_flist_t			*h_arpu_temp_flistp = NULL;
	int64			db = -1;
	int32			srch_flag = 256;
	int32			srch_key  = -1;
	int32			srch_type = -1;
	int32			*srch_type_ptr = NULL;
	time_t			*start_t = NULL;
	time_t			*end_t = NULL;
	int32			status_success =0;
	int32			status_fail =1;
	int32			*service_type = NULL;
	int32			*plan_type = NULL;
	pin_decimal_t		*plan_sub_type = 0;
	int32			*plan_category = NULL;
	int32			*payment_type = NULL;
	int32			*device_type = NULL;
	int32			add_info_flag = 1;
	void			*vp = NULL;
	char			*city = NULL;
	pin_flist_t	     *plan_flistp = NULL;	
	pin_flist_t		*deal_flistp = NULL;
	pin_flist_t 		*prod_flistp = NULL;
	char			*prov_tag = NULL;
	int                     elem_id = 0;
	int                     p_elem_id = 0;
	pin_cookie_t            cookie = NULL;
	pin_cookie_t            p_cookie = NULL;
	int			*tech = NULL;	
	int32			ret_val = -1;
	pin_flist_t		*copy_in_flistp= NULL;
	int32			*serv_type = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_plan_details input flist", i_flistp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_cust_get_plan_details", ebufp);
		goto CLEANUP;
	}
	plan_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	db = PIN_POID_GET_DB(plan_pdp);
	srch_key  = *(int32 *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SEARCH_KEY, 0, ebufp);
	srch_type_ptr = (int32 *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SEARCH_TYPE, 1, ebufp);
	if (srch_type_ptr)
	{
		srch_type = *srch_type_ptr;	
	}
	srch_val = (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_VALUE, 1, ebufp);
	start_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_START_T, 1, ebufp);
	end_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 1, ebufp);
	
	service_type = (int32*)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_TYPE_OF_SERVICE, 0, ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST_0001");
	plan_type = (int32*)PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_PLAN_TYPE, 0, ebufp); 
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST_0001");
	plan_sub_type = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_PLAN_SUB_TYPE, 1, ebufp); 
 	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST_0001");

	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_MODE, 1, ebufp);
	if(vp != NULL) {
	    add_info_flag = *(int32 *)vp;
	}

	/*******************************************************************
	* Create different search templates based on input- START 
	*******************************************************************/
	if (srch_key == NAME  ||
	    srch_key == CODE  ||
	    srch_key == DESC
	 )
	{
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		if ( srch_key == NAME )
		{
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_NAME, srch_val, ebufp);
		}
		else if ( srch_key == CODE )
		{
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_CODE, srch_val, ebufp);
		}
		else if ( srch_key == DESC )
		{
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_DESCR, srch_val, ebufp);
		}

		if ( srch_type == EQUALS )
		{
			strcpy(template,"select X from /plan where  F1 = V1 ");
			PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

		}
		if ( srch_type == STARTS_WITH ||
		     srch_type == ENDS_WITH ||
		     srch_type == CONTAINS
		   )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST_0001");
			if ( service_type && *service_type == CATV)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST_0001");
				strcpy(template,"select X from /plan where  upper(F1) like upper(V1) and F2.type = V2 ");
				PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

				arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
				services_array = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_SERVICES, 0, ebufp );
				PIN_FLIST_FLD_PUT(services_array, PIN_FLD_SERVICE_OBJ, PIN_POID_CREATE(db, "/service/catv", -1, ebufp), ebufp);
			}
			else if ( service_type && *service_type == BB)
			{
			
				if (strcmp(srch_val, "higher_arpu") == 0)
				{ 
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "higher_arpu bb 1 ");
					generate_higher_arpu_template(ctxp, i_flistp, &h_arpu_temp_flistp, ebufp);
					
					PIN_FLIST_DESTROY_EX(&srch_flistp,NULL);
					
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "higher_arpu bb 2 ");
					
					srch_flistp = PIN_FLIST_COPY(h_arpu_temp_flistp,ebufp);
					
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "higher_arpu bb 3 ");
					
					PIN_FLIST_DESTROY_EX(&h_arpu_temp_flistp,NULL);
					
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "higher_arpu bb 4 ");
				}
				
				
				else {
				strcpy(template,"select X from /plan where  upper(F1) like upper(V1) and F2.type = V2 ");
				PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

				arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
				services_array = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_SERVICES, 0, ebufp );
				PIN_FLIST_FLD_PUT(services_array, PIN_FLD_SERVICE_OBJ, PIN_POID_CREATE(db, "/service/telco/broadband", -1, ebufp), ebufp);
				
				}
			}
			else
			{
				strcpy(template,"select X from /plan where  upper(F1) like upper(V1) ");
				PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
			}
		}
	}
	if (srch_key == CREATION_TIME )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Searching Plans based on CREATION_TIME ");
		if (service_type && *service_type == CATV)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Searching Plans for CATV based on creation time");
			strcpy(template,"select X from /plan where  F1 >= V1 and  F2 <= V2 and F3.type = V3 ");
			PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_CREATED_T, start_t, ebufp);

			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_CREATED_T, end_t, ebufp);

			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
			services_array = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_SERVICES, 0, ebufp );
			PIN_FLIST_FLD_PUT(services_array, PIN_FLD_SERVICE_OBJ, PIN_POID_CREATE(db, "/service/catv", -1, ebufp), ebufp);
		}
		else if (service_type && *service_type == BB)
		{
			if (strcmp(srch_val, "higher_arpu") == 0)
				{ 
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "higher_arpu bb 2 ");
					generate_higher_arpu_template(ctxp, i_flistp, &h_arpu_temp_flistp, ebufp);
					
					PIN_FLIST_DESTROY_EX(&srch_flistp,NULL);
					
					srch_flistp = PIN_FLIST_COPY(h_arpu_temp_flistp,ebufp);
					
				   PIN_FLIST_DESTROY_EX(&h_arpu_temp_flistp,NULL);
					
				}
				else{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Searching Plans for BB based on creation time");
					strcpy(template,"select X from /plan where  F1 >= V1 and  F2 <= V2 and F3.type = V3 ");
					PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

					arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
					PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_CREATED_T, start_t, ebufp);

					arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
					PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_CREATED_T, end_t, ebufp);

					arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
					services_array = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_SERVICES, 0, ebufp );
					PIN_FLIST_FLD_PUT(services_array, PIN_FLD_SERVICE_OBJ, PIN_POID_CREATE(db, "/service/telco/broadband", -1, ebufp), ebufp);
				}
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Searching Plans for CATV & BB based on creation time");
			strcpy(template,"select X from /plan where  F1 >= V1 and  F2 <= V2 ");
			PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_CREATED_T, start_t, ebufp);

			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_CREATED_T, end_t, ebufp);
		}
	}
	if (service_type && srch_key == MODIFICATION_TIME )
	{
		if (*service_type == CATV)
		{
			strcpy(template,"select X from /plan where  F1 >= V1 and  F2 <= V2 ");
			PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_MOD_T, start_t, ebufp);

			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_MOD_T, end_t, ebufp);

			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
			services_array = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_SERVICES, 0, ebufp );
			PIN_FLIST_FLD_PUT(services_array, PIN_FLD_SERVICE_OBJ, PIN_POID_CREATE(db, "/service/catv", -1, ebufp), ebufp);
		}
		if (*service_type == BB)
		{
			
			if (strcmp(srch_val, "higher_arpu") == 0)
			{ 
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "higher_arpu bb 3 ");
				generate_higher_arpu_template(ctxp, i_flistp, &h_arpu_temp_flistp, ebufp);
				
				PIN_FLIST_DESTROY_EX(&srch_flistp,NULL);
				
				srch_flistp = PIN_FLIST_COPY(h_arpu_temp_flistp,ebufp);
				
			   PIN_FLIST_DESTROY_EX(&h_arpu_temp_flistp,NULL);
				
			}
			else{
				strcpy(template,"select X from /plan where  F1 >= V1 and  F2 <= V2 ");
				PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

				arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
				PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_MOD_T, start_t, ebufp);

				arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
				PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_MOD_T, end_t, ebufp);

				arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
				services_array = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_SERVICES, 0, ebufp );
				PIN_FLIST_FLD_PUT(services_array, PIN_FLD_SERVICE_OBJ, PIN_POID_CREATE(db, "/service/telco/broadband", -1, ebufp), ebufp);
			}
		}
		else
		{
			strcpy(template,"select X from /plan where  F1 >= V1 and  F2 <= V2 ");
			PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_MOD_T, start_t, ebufp);

			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_MOD_T, end_t, ebufp);
		}
	}
	if (srch_key == POID )
	{
		strcpy(template,"select X from /plan where  F1.id = V1 ");
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, plan_pdp, ebufp);
	}

/*	if(srch_key == ALL ) {
		if ( service_type && *service_type == CATV)
		{
			strcpy(template,"select X from /plan where F2.type = V2 ");
			PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
			services_array = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_SERVICES, 0, ebufp );
			PIN_FLIST_FLD_PUT(services_array, PIN_FLD_SERVICE_OBJ, PIN_POID_CREATE(db, "/service/catv", -1, ebufp), ebufp);
		} 
		else if ( service_type && *service_type == BB)
		{
			strcpy(template,"select X from /plan where F2.type = V2 ");
			PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
			services_array = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_SERVICES, 0, ebufp );
			PIN_FLIST_FLD_PUT(services_array, PIN_FLD_SERVICE_OBJ, PIN_POID_CREATE(db, "/service/telco/broadband", -1, ebufp), ebufp);
		}		
	}*/

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST_0001");
	
	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_CREATED_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_MOD_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_CODE, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_NAME, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_DESCR, NULL, ebufp);

	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_LIMIT, NULL, ebufp);
	services_array = PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_SERVICES, 0, ebufp);
	PIN_FLIST_FLD_SET(services_array, PIN_FLD_DEAL_OBJ, NULL, ebufp); 
	PIN_FLIST_FLD_SET(services_array, PIN_FLD_SERVICE_OBJ, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_plan_details search flist", srch_flistp);
	/*******************************************************************
	* Create different search templates based on input- END 
	*******************************************************************/
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		return;
	}
	PIN_FLIST_FLD_SET(srch_out_flistp, MSO_FLD_PLAN_TYPE, plan_type, ebufp);
	if (plan_sub_type)
	{
		PIN_FLIST_FLD_SET(srch_out_flistp, MSO_FLD_PLAN_SUB_TYPE, plan_sub_type, ebufp);
	}



	if ((PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp)))
	{
		plan_category = (int32 *)PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_PLAN_CATEGORY, 1, ebufp);
		if(plan_category) {
			PIN_FLIST_FLD_SET(srch_out_flistp, MSO_FLD_PLAN_CATEGORY, plan_category, ebufp);
		}
		payment_type = (int32 *)PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_PAYMENT_TYPE, 1, ebufp);
		if(payment_type) {
			PIN_FLIST_FLD_SET(srch_out_flistp, MSO_FLD_PAYMENT_TYPE, payment_type, ebufp);
		}
		device_type = (int32 *)PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_DEVICE_TYPE, 1, ebufp);
		if(device_type)
		{
			PIN_FLIST_FLD_SET(srch_out_flistp, MSO_FLD_DEVICE_TYPE, device_type, ebufp);
		}
		city = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CITY, 1, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "City in input is ");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, city);
		if(city) {
			PIN_FLIST_FLD_SET(srch_out_flistp, PIN_FLD_CITY, city, ebufp);
		}
		fm_mso_get_products_from_deal(ctxp, 1, add_info_flag, srch_out_flistp, ebufp );
	        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_products_from_deal out flist", srch_out_flistp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_get_products_from_deal()", ebufp);
			PIN_ERRBUF_RESET(ebufp);

			PIN_FLIST_FLD_SET(srch_out_flistp, PIN_FLD_STATUS, &status_fail, ebufp);
			PIN_FLIST_FLD_SET(srch_out_flistp, PIN_FLD_ERROR_CODE, "51131", ebufp);
			PIN_FLIST_FLD_SET(srch_out_flistp, PIN_FLD_ERROR_DESCR, "Error in calling fm_mso_get_products_from_deal()", ebufp);
			goto CLEANUP;
		}
		/***adding changes to check weather plan is docsis or gpon**/
		copy_in_flistp = PIN_FLIST_COPY(srch_out_flistp, ebufp);
		if ( (PIN_FLIST_ELEM_COUNT(copy_in_flistp, PIN_FLD_PLAN, ebufp)) == 0 )
		{
			goto SKIP;
		}
		cookie = NULL;
                while((plan_flistp = PIN_FLIST_ELEM_GET_NEXT(copy_in_flistp, PIN_FLD_PLAN,
                        &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
                {
			serv_type = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_TYPE_OF_SERVICE, 1, ebufp);
			if(serv_type && *serv_type != 2)
                        	break;

			tech = PIN_FLIST_FLD_GET(plan_flistp, MSO_FLD_DEVICE_TYPE, 1, ebufp);
			deal_flistp = PIN_FLIST_ELEM_GET(plan_flistp, PIN_FLD_DEALS, PIN_ELEMID_ANY, 1, ebufp);
			if(!deal_flistp)
			{
				goto SKIP;
			}
			else if((PIN_FLIST_ELEM_COUNT(deal_flistp, PIN_FLD_PRODUCTS, ebufp)) == 0)
			{
				goto SKIP;
			}
			p_cookie = NULL;
			while((prod_flistp = PIN_FLIST_ELEM_GET_NEXT(deal_flistp, PIN_FLD_PRODUCTS,
                        &p_elem_id, 1, &p_cookie, ebufp)) != (pin_flist_t *)NULL)
			{
				prov_tag = PIN_FLIST_FLD_GET(prod_flistp, PIN_FLD_PROVISIONING_TAG, 1, ebufp);
				if ( prov_tag && strcmp(prov_tag,"") != 0 )
				{
					if(tech && *tech == 2)
					{
						*tech = 1;
					}
					else if(tech && *tech == 3)
                                        {
                                                *tech = 2;
                                        }
					else if(tech && *tech == 1)
					{
						*tech = 3;
					}
					ret_val = fm_mso_cust_get_valid_tech(ctxp, plan_pdp, prov_tag, tech, ebufp);
					if(ret_val == 1)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Technology matched");
						break;
					}
					else if(ret_val == 0)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Technology mismatch");
                                                PIN_FLIST_ELEM_DROP( srch_out_flistp, PIN_FLD_PLAN, elem_id, ebufp);
						break;
					}	
				}
			}
		}
		SKIP:
		PIN_FLIST_FLD_SET(srch_out_flistp, PIN_FLD_STATUS, &status_success, ebufp);
		PIN_FLIST_FLD_DROP(srch_out_flistp, MSO_FLD_PLAN_TYPE, ebufp );
	}
	else
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"No Records found, try after changing search criteria", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_FLD_SET(srch_out_flistp, PIN_FLD_STATUS, &status_fail, ebufp);
		PIN_FLIST_FLD_SET(srch_out_flistp, PIN_FLD_ERROR_CODE, "51130", ebufp);
		PIN_FLIST_FLD_SET(srch_out_flistp, PIN_FLD_ERROR_DESCR, "No Records found, try after changing search criteria", ebufp);
		goto CLEANUP;
	}

	CLEANUP:
	//PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	*r_flistpp = srch_out_flistp;
	PIN_FLIST_DESTROY_EX(&copy_in_flistp, NULL);
	return;
}

//Optimizing Higher ARPU API - Filtering Plan list based on City and Technology // - Biju
static void
generate_higher_arpu_template(
	pcm_context_t    *ctxp,
	pin_flist_t      *i_flistp,
	pin_flist_t             **ret_flistp,
	pin_errbuf_t 	 *ebufp
)
{
        int32			*pay_type = NULL;
		int32			*dev_type = NULL;
		int32			tech = 5;
		char			*city_val = NULL;
		
		pin_flist_t		*srch_flistp = NULL;
		
		pin_flist_t		*arg_flist = NULL;
		pin_flist_t		*services_array = NULL;
		pin_flist_t		*city_array = NULL;
		pin_flist_t		*deal_prd_flistp = NULL;
		pin_flist_t		*srv_flistp = NULL;
		
		int64	db = 1;
		int32	flags = 256;
		char    err_str[256] = {'\0'};
		char	hfiber_template[512]={'\0'};
		char	*strtmp = "PrTag";
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "higher_arpu : generate_higher_arpu_template inside");
		dev_type = (int32 *)PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_DEVICE_TYPE, 0, ebufp);
		pay_type = (int32 *)PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_PAYMENT_TYPE, 0, ebufp);
		city_val = (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CITY,0, ebufp);
		sprintf(err_str,"dev_type is  : %ld", *dev_type);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, err_str);
		sprintf(err_str,"pay_type is  : %ld", *pay_type);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, err_str);
		
		if( dev_type && *dev_type == 2)
		{
			tech = 1;
		}
		else if(dev_type && *dev_type == 3)
		{
			tech = 2;
		}
		else if(dev_type && *dev_type == 1)
		{
			tech = 3;
		}
		else if(dev_type && *dev_type == 4)
		{
			tech = 4;
		}		
		//service_type = (int32*)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_TYPE_OF_SERVICE, 0, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST_0001");
		
		//strcpy( hfiber_template,"select X from /plan 1, /mso_cfg_credit_limit 2, /deal 3, /product 4, /mso_config_bb_pt 5  where  F1.type = V1 and (2.F2 = V2 or 2.F2 = '*') and 5.F3 = V3 and 5.F4 = V4 and 1.F7 = 2.F8 and 1.F10 = 3.F9 and 3.F11 = 4.F12 and 4.F5 = 5.F6" );
		
		strcpy( hfiber_template,"select X from /plan 1, /mso_cfg_credit_limit 2, /deal 3, /product 4, /mso_config_bb_pt 5  where  F1.type = V1 and (2.F2 = V2 or 2.F2 = '*') and 5.F3 = V3 and 1.F6 = 2.F7 and 1.F9 = 3.F8 and 3.F10 = 4.F11 and 4.F4 = 5.F5" );
		
		//strcpy( hfiber_template,"select X from /plan 1, /mso_cfg_credit_limit 2, /deal 3, /product 4, /mso_config_bb_pt 5  where  F1.type = V1 and (2.F2 = V2 or 2.F2 = '*') and  1.F5 = 2.F6 and 1.F8 = 3.F7 and 3.F9 = 4.F10 and 4.F3 = 5.F4 ");
		
		srch_flistp= PIN_FLIST_CREATE(ebufp);
		
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_POID,
			PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
				
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &flags, ebufp);
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, hfiber_template, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		services_array = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_SERVICES, 0, ebufp );
		PIN_FLIST_FLD_SET(services_array, PIN_FLD_SERVICE_OBJ, PIN_POID_CREATE(db, "/service/telco/broadband", -1, ebufp), ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		city_array = PIN_FLIST_ELEM_ADD(arg_flist, MSO_FLD_CITIES, 0, ebufp );
		PIN_FLIST_FLD_SET(city_array, PIN_FLD_CITY,city_val, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(arg_flist, MSO_FLD_BB_TECHNOLOGY, &tech, ebufp);

		//arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp);
		//PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_INDICATOR, pay_type, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp);
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_PROVISIONING_TAG, strtmp, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 5, ebufp);
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_PROVISIONING_TAG, strtmp, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 6, ebufp);  //Plan
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 7, ebufp);
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_PLAN_OBJ, NULL, ebufp);
		
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 8, ebufp);  //Deal
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL, ebufp);
		
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 9, ebufp);
		srv_flistp = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_SERVICES, 0, ebufp);
		PIN_FLIST_FLD_SET(srv_flistp, PIN_FLD_DEAL_OBJ, NULL, ebufp);
		
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 10, ebufp);
		deal_prd_flistp = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_PRODUCTS, 0, ebufp);
		PIN_FLIST_FLD_SET(deal_prd_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
		
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 11, ebufp);  //product
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "generate_higher_arpu_template search flist is : ", srch_flistp);
		*ret_flistp = PIN_FLIST_COPY(srch_flistp,ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "generate_higher_arpu_template search flist is : No " );
		PIN_FLIST_DESTROY_EX(&srch_flistp,NULL);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "SEARCH Result search flist is : ", *ret_flistp);
		return;
}

int32
fm_mso_cust_get_valid_tech(
        pcm_context_t           *ctxp,
	poid_t			*pdp,
        char	             	*prov,
	int			*tech,
        pin_errbuf_t            *ebufp)
{
	char                    *template = " select X from /mso_config_bb_pt  where F1 = V1 and F2 = V2 " ;	
	pin_flist_t             *read_in_flistp = NULL;
        pin_flist_t             *read_out_flistp = NULL;
	int64                   db = 0;	
	poid_t			*srch_pdp = NULL;
	int32                   flags = 512;
	pin_flist_t		*arg_flistp = NULL;
	int32			ret_val=-2;
	

	if(PIN_ERRBUF_IS_ERR(ebufp)) {
                return;
        }
	
	
	db = PIN_POID_GET_DB(pdp);
        srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
        read_in_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(read_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
        PIN_FLIST_FLD_SET(read_in_flistp, PIN_FLD_FLAGS, &flags, ebufp);
        PIN_FLIST_FLD_SET(read_in_flistp, PIN_FLD_TEMPLATE, template, ebufp);
        arg_flistp = PIN_FLIST_ELEM_ADD(read_in_flistp, PIN_FLD_ARGS, 1, ebufp );
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_PROVISIONING_TAG, prov, ebufp);
        arg_flistp = PIN_FLIST_ELEM_ADD(read_in_flistp, PIN_FLD_ARGS, 2, ebufp );
        PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_BB_TECHNOLOGY, tech, ebufp);
        arg_flistp = PIN_FLIST_ELEM_ADD(read_in_flistp, PIN_FLD_RESULTS, 0, ebufp );
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Plan validation search input flist", read_in_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, read_in_flistp, &read_out_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&read_in_flistp, NULL);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Plan validation search output flist", read_out_flistp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
		ret_val = -1;
		goto CLEANUP;
	}
	if(!PIN_FLIST_ELEM_COUNT(read_out_flistp, PIN_FLD_RESULTS, ebufp))
	{
		ret_val = 0;
		goto CLEANUP;
	}
	else
	{
		ret_val = 1;
		goto CLEANUP;
	}
	
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&read_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);	
	return ret_val;
}

