/*******************************************************************
 * File:	fm_mso_cust_get_channel_and_price.c
 * Opcode:	MSO_OP_CUST_GET_CHANNEL_AND_PRICE 
 * Owner:	Gautam Adak
 * Created:	17-SEPT-2013
 * Description: 
 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 * Input Flist
 *
	0 PIN_FLD_USERID         POID [0] 0.0.0.1 /account 88888
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 88888
	0 PIN_FLD_PLAN_OBJ       POID [0] 0.0.0.1 /plan 33778 6

	0 PIN_FLD_USERID         POID [0] 0.0.0.1 /account 88888
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 88888
	0 PIN_FLD_OFFERING_OBJ   POID [0] 0.0.0.1 /purchased_product 62902 6

********************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_get_channel_and_price.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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

#define HDW_PROD_PRI_START   10
#define HDW_PROD_PRI_END     60
#define SUBS_PROD_PRI_START  90
#define SUBS_PROD_PRI_END   170
#define OTC_PROD_PRI_START  200
#define OTC_PROD_PRI_END    210

#define DEPOSIT_PRODUCT      20

#define CATV_MAIN  0
#define CATV_ADDL  1
#define CATV_ALL   2


/**************************************
* External Functions start
***************************************/

/**************************************
* External Functions end
***************************************/



void
fm_mso_get_prod_amnt(
	pcm_context_t		*ctxp,
	poid_t			*prod_pdp,
	pin_flist_t		**prod_amnt_flist,
	int32			conn_type,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * MSO_OP_CUST_GET_CHANNEL_AND_PRICE 
 *
 * 
 *******************************************************************/

EXPORT_OP void
op_mso_cust_get_device_details(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_get_channel_and_price(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * MSO_OP_CUST_GET_CHANNEL_AND_PRICE  
 *******************************************************************/
void 
op_mso_cust_get_channel_and_price(
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
	if (opcode != MSO_OP_CUST_GET_CHANNEL_AND_PRICE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_get_channel_and_price error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_get_channel_and_price input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	fm_mso_cust_get_channel_and_price(ctxp, flags, i_flistp, r_flistpp, ebufp);
 	/***********************************************************
	 * Results.
	 ***********************************************************/

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_get_channel_and_price error", ebufp);
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_get_channel_and_price output flist", *r_flistpp);
	}
	return;
}



/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_get_channel_and_price(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*services = NULL;
	pin_flist_t		*products = NULL;
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_channels = NULL;
	pin_flist_t		*channels_oflistp = NULL;
	pin_flist_t		*prod_amnt_flist = NULL;
	pin_flist_t		*channels_array = NULL;

	poid_t			*user_id = NULL;
	poid_t			*acnt_pdp = NULL;
	poid_t			*plan_obj = NULL;
	poid_t			*purchased_prod = NULL;
	poid_t			*srch_pdp = NULL;
	poid_t			*product_obj = NULL;


	char			*template_1 = "select X from /product 1, /deal 2, /plan 3 where 3.F1 = V1  and 3.F2 = 2.F3 and 2.F4 = 1.F5 " ;
	char			*template_2 = "select X from /product 1, /purchased_product 2 where 2.F1 = V1  and 2.F2 = 1.F3 " ;
	char			*template_3 = "select X from /mso_cfg_catv_channel_master 1, /mso_cfg_catv_pt_channel_map 2 where 1.F1 = 2.F2 and  2.F3 = V3 " ;
	char			*prov_tag;

	int64			db = -1;

	int32			elem_id =0;
	int32			srch_flag_without_poid = 1024;
	int32			srch_flag = 768;
	int32			failure = 1;
	int32			success = 0;

	pin_cookie_t		cookie = NULL;

	pin_decimal_t		*priority = NULL;
	double			priority_double = 0.0;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_channel_and_price input flist", i_flistp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_cust_get_channel_and_price", ebufp);
		goto CLEANUP;
	}

	user_id =  PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID , 1, ebufp );
	acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp ); 
	plan_obj = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PLAN_OBJ, 1,  ebufp);
	purchased_prod = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_OFFERING_OBJ, 1, ebufp);

	db = PIN_POID_GET_DB(user_id);

	if (!plan_obj && !purchased_prod)     // At least one should be passed
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"/plan or /purhased_product poid Missing", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51000", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "/plan or /purhased_product poid Missing", ebufp);
		goto CLEANUP;
	}

	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp);
	
	if (plan_obj)
	{
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template_1 , ebufp);
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, plan_obj, ebufp );

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp);
		services = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_SERVICES, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(services, PIN_FLD_DEAL_OBJ, NULL, ebufp );

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp) ;
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL, ebufp );

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp) ;
		products = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(products, PIN_FLD_PRODUCT_OBJ, NULL, ebufp );

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 5, ebufp) ;
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL, ebufp );
	}
	else if (purchased_prod)
	{
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template_2 , ebufp);
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, purchased_prod, ebufp );

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_PRODUCT_OBJ, NULL, ebufp );
		
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL, ebufp );
	}
  
	result_flist = 	PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_PRIORITY, NULL, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_PROVISIONING_TAG, NULL, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search prov_tag and priority input flist ", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search prov_tag and priority output flist", ret_flistp);

	/************************************************************************
	* Get Channel Information only for subscripion product
	************************************************************************/
	while((result_flist = PIN_FLIST_ELEM_GET_NEXT(ret_flistp, PIN_FLD_RESULTS,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		product_obj = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_POID, 0, ebufp); 
		priority = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_PRIORITY, 0, ebufp);
		prov_tag = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_PROVISIONING_TAG, 0, ebufp);
		priority_double = pbo_decimal_to_double(priority, ebufp);
		if ( priority_double  >= SUBS_PROD_PRI_START  &&
		     priority_double  <= SUBS_PROD_PRI_END
		   )
		{
			/*************************************************
			* search Channels
			*************************************************/
			srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
			srch_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
			PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag_without_poid, ebufp);
			PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template_3 , ebufp);
			
			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp) ;
			PIN_FLIST_FLD_SET(arg_flist, MSO_FLD_CHANNEL_ID, "", ebufp);

			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp) ;
			channels_array = PIN_FLIST_ELEM_ADD(arg_flist, MSO_FLD_CATV_CHANNELS, PIN_ELEMID_ANY, ebufp) ;
			PIN_FLIST_FLD_SET(channels_array, MSO_FLD_CHANNEL_ID, "", ebufp);

			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp) ;
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_PROVISIONING_TAG, prov_tag, ebufp);

			result_channels = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp);
			PIN_FLIST_FLD_SET(result_channels, MSO_FLD_CHANNEL_GENRE, NULL, ebufp );
			PIN_FLIST_FLD_SET(result_channels, MSO_FLD_CHANNEL_ID, NULL, ebufp );
			PIN_FLIST_FLD_SET(result_channels, MSO_FLD_CHANNEL_NAME, NULL, ebufp );

			//PIN_FLIST_ELEM_SET(result_channels, NULL, MSO_FLD_CATV_CHANNELS, PIN_ELEMID_ANY, ebufp);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Channels input list", srch_flistp);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &channels_oflistp, ebufp);
			PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
				goto CLEANUP;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Channels output flist", channels_oflistp);

			result_channels = PIN_FLIST_ELEM_GET(channels_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
			if (result_channels)
			{
				PIN_FLIST_FLD_SET(channels_oflistp, PIN_FLD_PRODUCT_OBJ, product_obj, ebufp);
				PIN_FLIST_FLD_RENAME(channels_oflistp,PIN_FLD_RESULTS, MSO_FLD_CATV_CHANNELS, ebufp);
				PIN_FLIST_CONCAT(result_flist, channels_oflistp, ebufp);
				PIN_FLIST_FLD_DROP(result_flist, PIN_FLD_POID, ebufp);
			}
			PIN_FLIST_DESTROY_EX(&channels_oflistp, NULL);
 		}

		fm_mso_get_prod_amnt(ctxp, product_obj, &prod_amnt_flist, CATV_ALL, ebufp);
		if (prod_amnt_flist)
		{
			PIN_FLIST_CONCAT(result_flist, prod_amnt_flist, ebufp);
			PIN_FLIST_DESTROY_EX(&prod_amnt_flist, NULL);
		}
		PIN_FLIST_FLD_DROP(result_flist, PIN_FLD_POID, ebufp);
	}

	CLEANUP:
	*r_flistpp = ret_flistp;
	return;
}


/**************************************************
* Function: 	fm_mso_get_prod_amnt()
* Owner:        Gautam Adak
* Decription:   For getting amount of product
*		
* 
* 
***************************************************/
void
fm_mso_get_prod_amnt(
	pcm_context_t		*ctxp,
	poid_t			*prod_pdp,
	pin_flist_t		**prod_amnt_flist,
	int32			conn_type,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*rqt_flistp = NULL;
	pin_flist_t		*rbl_flistp = NULL;
	pin_flist_t		*balimpact_flistp = NULL;
	pin_flist_t		*quantity_flistp = NULL;
	pin_flist_t		*amounts = NULL;
	pin_flist_t		*amounts1 = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*quant_flistp = NULL;
	pin_flist_t		*bal_flistp = NULL;

	poid_t			*srch_pdp = NULL;

	pin_decimal_t		*amnnt = NULL; 
	pin_decimal_t		*b_scl_amt = NULL;
	pin_decimal_t		*b_fx_amt = NULL;
	pin_decimal_t		*amnnt_1 = NULL;

	int32			flags = 768;
	int32			cur = 356;
	int32			*resource_id = NULL;

	int32			elem_id   =0;
	int32			elem_id_1 =0;
	int32			elem_id_2 =0;

	pin_cookie_t		cookie   = NULL;
	pin_cookie_t		cookie_1 = NULL;
	pin_cookie_t		cookie_2 = NULL;

	char *tmpl_catv_addl  = "select X from /rate 1, /rate_plan 2, /product 3 where 1.F1 = 2.F2 and 2.F3 = 3.F4  and 3.F5 = V5 and 1.F6 = V6 and 2.F7 like V7 ";
	char *tmpl_catv_main  = "select X from /rate 1, /rate_plan 2, /product 3 where 1.F1 = 2.F2 and 2.F3 = 3.F4  and 3.F5 = V5 and 1.F6 = V6 and 2.F7 not like V7";
	char *tmpl_catv_all   = "select X from /rate 1, /rate_plan 2, /product 3 where 1.F1 = 2.F2 and 2.F3 = 3.F4  and 3.F5 = V5 and 1.F6 = V6 ";

	int64			db = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Err in fm_mso_get_prod_amnt", ebufp);
		return;
	}


	db = PIN_POID_GET_DB(prod_pdp);
	srch_pdp = PIN_POID_CREATE(db, "/search", (int64)-1, ebufp);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	if (conn_type == CATV_ALL)
	{
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, tmpl_catv_all, ebufp);
	}
	if (conn_type == CATV_MAIN)
	{
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, tmpl_catv_main, ebufp);
	}
	if (conn_type == CATV_ADDL)
	{
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, tmpl_catv_addl, ebufp);
	}

	args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_RATE_PLAN_OBJ, NULL, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 5, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, prod_pdp, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 6, ebufp);
	quant_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_QUANTITY_TIERS, PIN_ELEMID_ANY, ebufp);
	bal_flistp = PIN_FLIST_ELEM_ADD(quant_flistp, PIN_FLD_BAL_IMPACTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_ELEMENT_ID, &cur, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 7, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CODE, "%ADDL", ebufp);

	result_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	rqt_flistp = PIN_FLIST_ELEM_ADD(result_flistp, PIN_FLD_QUANTITY_TIERS, PIN_ELEMID_ANY, ebufp);
	rbl_flistp = PIN_FLIST_ELEM_ADD(rqt_flistp, PIN_FLD_BAL_IMPACTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(rbl_flistp, PIN_FLD_SCALED_AMOUNT, NULL, ebufp);
	PIN_FLIST_FLD_SET(rbl_flistp, PIN_FLD_FIXED_AMOUNT, NULL, ebufp);
	PIN_FLIST_FLD_SET(rbl_flistp, PIN_FLD_ELEMENT_ID, NULL, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "get amount input flistp is", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "get amount output flistp is", ret_flistp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error while searching for product amount", ebufp);
		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
		return;
	}

	while((result_flistp = PIN_FLIST_ELEM_GET_NEXT(ret_flistp, PIN_FLD_RESULTS,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		amnnt = pbo_decimal_from_str("0.0", ebufp);
		elem_id_1 = 0;
		cookie_1 = NULL;
		while((quantity_flistp = PIN_FLIST_ELEM_GET_NEXT(result_flistp, PIN_FLD_QUANTITY_TIERS, 
			&elem_id_1, 1, &cookie_1, ebufp)) != NULL)
		{
			elem_id_2 = 0;
			cookie_2 = NULL;
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "get amount quantity_flistp is", quantity_flistp);
			while((balimpact_flistp = PIN_FLIST_ELEM_GET_NEXT(quantity_flistp, PIN_FLD_BAL_IMPACTS,
				&elem_id_2, 1, &cookie_2, ebufp)) != NULL)
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "get amount balimpact_flistp is", balimpact_flistp);
				resource_id = PIN_FLIST_FLD_GET(balimpact_flistp, PIN_FLD_ELEMENT_ID, 0, ebufp);
				if(*resource_id == cur)
				{
					b_scl_amt = (pin_decimal_t *)PIN_FLIST_FLD_GET(balimpact_flistp, PIN_FLD_SCALED_AMOUNT, 0, ebufp);
					b_fx_amt = (pin_decimal_t*)PIN_FLIST_FLD_GET(balimpact_flistp, PIN_FLD_FIXED_AMOUNT, 0, ebufp);
					pbo_decimal_add_assign(amnnt, b_scl_amt, ebufp);
					pbo_decimal_add_assign(amnnt, b_fx_amt,  ebufp);
				}
			}
		}
		if (!(*prod_amnt_flist))
		{
			*prod_amnt_flist = PIN_FLIST_CREATE(ebufp);
		}
		amounts = PIN_FLIST_ELEM_ADD(*prod_amnt_flist, PIN_FLD_AMOUNTS, elem_id, ebufp );
		PIN_FLIST_FLD_PUT(amounts, PIN_FLD_AMOUNT, amnnt, ebufp);
		PIN_FLIST_FLD_SET(amounts, PIN_FLD_SERVICE_TYPE, "MAIN", ebufp);

		if ( elem_id > 0 ) //Only when rateplan selector is configured
		{
			amounts = PIN_FLIST_ELEM_GET(*prod_amnt_flist, PIN_FLD_AMOUNTS, 0, 0, ebufp );
			amnnt =  PIN_FLIST_FLD_GET(amounts, PIN_FLD_AMOUNT, 0, ebufp );

			amounts1 = PIN_FLIST_ELEM_GET(*prod_amnt_flist, PIN_FLD_AMOUNTS, 1, 0, ebufp );
			amnnt_1 =  PIN_FLIST_FLD_GET(amounts, PIN_FLD_AMOUNT, 0, ebufp );

			if ( pbo_decimal_compare(amnnt, amnnt_1, ebufp ) == 1 )	 // amnnt > amnnt1
			{
				PIN_FLIST_FLD_SET(amounts1, PIN_FLD_SERVICE_TYPE, "ADDITIONAL", ebufp);
			}
			else
			{
				PIN_FLIST_FLD_SET(amounts, PIN_FLD_SERVICE_TYPE,  "ADDITIONAL", ebufp);
				PIN_FLIST_FLD_SET(amounts1, PIN_FLD_SERVICE_TYPE, "MAIN", ebufp);
			}
		}
	}
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
}
