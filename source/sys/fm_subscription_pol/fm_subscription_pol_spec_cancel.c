/*
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. 
All rights reserved. 
 *      
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *
 */

#ifndef lint
static  char    Sccs_id[] = "@(#)$Id: fm_subscription_pol_spec_cancel.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 06:17:43 nishahan Exp $";
#endif

/*******************************************************************
 * Contains the PCM_OP_SUBSCRIPTION_POL_SPEC_CANCEL operation. 
 *
 * Also included are subroutines specific to the operation.
 *
 * These routines are generic and work for all account types.
 *******************************************************************/

#include <stdio.h> 
#include <string.h> 
 
#include "pcm.h"
#include "ops/bill.h"
#include "ops/subscription.h"
#include "pin_act.h"
#include "pin_bill.h"
#include "pin_cust.h"
#include "pin_currency.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "fm_bill_utils.h"
#include "pin_rate.h"
#include "mso_rate.h"
#include "mso_ntf.h"
#include "mso_cust.h"
#include "mso_errs.h"
#include "mso_ops_flds.h"
#include "mso_commission.h"
#include "mso_lifecycle_id.h"
#include "mso_prov.h"
#include "mso_glid.h"

#define FILE_SOURCE_ID  "fm_subscription_pol_spec_cancel.c(1.3)"


/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_subscription_pol_spec_cancel(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp);

void fm_subs_pol_get_cancel_specs();

void
mso_get_mso_purchased_plan_from_product(
        //pcm_context_t   *ctxp,
	cm_nap_connection_t     *connp,
        pin_flist_t      *i_flistp,
        pin_errbuf_t    *ebufp);

void
mso_cancel_mso_purchased_plan_and_service(
        pcm_context_t   *ctxp,
        pin_flist_t      *i_flistp,
        pin_errbuf_t    *ebufp);

void
mso_cancel_deal(
        pcm_context_t   *ctxp,
        pin_flist_t      *i_flistp,
	poid_t		*offering_pdp,
        pin_errbuf_t    *ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_SUBSCRIPTION_POL_SPEC_CANCEL operation.
 *******************************************************************/
void
op_subscription_pol_spec_cancel(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	pin_cookie_t		cookie = NULL;

	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*flistp = NULL;

	poid_t			*a_pdp = NULL;

	char			*action;
	char			*cp;

	u_int			rec_id;
	u_int			done = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_SUBSCRIPTION_POL_SPEC_CANCEL) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_subscription_pol_spec_cancel opcode error", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_subscription_pol_spec_cancel input flist", in_flistp);

	/***********************************************************
	 * Get the policy return flist.
	 ***********************************************************/
	fm_subs_pol_get_cancel_specs(ctxp, in_flistp, &r_flistp, ebufp);



	mso_get_mso_purchased_plan_from_product(connp,in_flistp,ebufp);

	/***********************************************************
	 * Errors?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {

		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_subscription_pol_spec_cancel error", ebufp);
		PIN_FLIST_DESTROY(r_flistp, NULL);
		*ret_flistpp = NULL;

	} else {

		/***************************************************
		 * Point the real return flist to the right thing.
		 ***************************************************/
		PIN_ERR_CLEAR_ERR(ebufp);
		*ret_flistpp = r_flistp;
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"op_subscription_pol_spec_cancel return flist", 
				*ret_flistpp);
	}

	return;
}


/*******************************************************************
 * fm_subs_pol_get_cancel_specs():
 *
 *	Valid ACTION (Strings) are ( Only one of the following )
 *		1)  PIN_BILL_CANCEL_PRODUCT_ACTION_CANCEL_ONLY
 *		    only if the product needs to be cancelled but
 *		      not deleted from the d/b.
 *		    (if you expect some event batch to come in
 *			after the product is cancelled. )
 *		2)  PIN_BILL_CANCEL_PRODUCT_ACTION_CANCEL_DELETE 
 *		    if the product is to be cancelled and deleted.
 *		    This is what is passed as default to this policy.
 * 		3)  PIN_BILL_CANCEL_PRODUCT_ACTION_DONOT_CANCEL, if 
 *		    the product is not to be cancelled and not to be
 *		    deleted.
 *******************************************************************/
void
fm_subs_pol_get_cancel_specs(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**out_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t     *r_flistp = NULL;
        pin_flist_t     *s_flistp = NULL;
        pin_flist_t     *p_arrayp = NULL;
        pin_cookie_t    cookie = NULL;
        int             elem_id = 0;
	void		*vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/*
	 * look for existing provisioning tag in product object
	 */
	s_flistp = PIN_FLIST_CREATE(ebufp);

	p_arrayp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_PRODUCTS,
		&elem_id, 0, &cookie, ebufp);

	/*
	 * set product object (from input flist) on search flist
	 */
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_POID,
		PIN_FLIST_FLD_GET(p_arrayp, PIN_FLD_PRODUCT_OBJ, 0, ebufp),
		ebufp);

	/*
	 * set provisioning tag on search flist to retrieve it
	 */
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_PROVISIONING_TAG,
		NULL, ebufp);

	/*
	 * call opcode to read flds from db
	 */
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, s_flistp, &r_flistp, ebufp);

	vp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_PROVISIONING_TAG,
                0, ebufp);

        if (vp && strcmp((char *)vp, "") != 0) {

		/*
		 * Set action not to delete product for all products with a
		 * provisioning tag (status of product needs to be updated by
		 * the provisioning system and so must remain in the table).
		 */

		PIN_FLIST_FLD_SET(p_arrayp, PIN_FLD_ACTION,
			PIN_BILL_CANCEL_PRODUCT_ACTION_CANCEL_ONLY, ebufp);

	} else {

		/*
		 * For all products without a provisioning tag cancel and
		 * delete product from table.
		 */

		if (pin_conf_keep_cancelled_products_or_discounts == 0){
			PIN_FLIST_FLD_SET(p_arrayp, PIN_FLD_ACTION,
			PIN_BILL_CANCEL_PRODUCT_ACTION_CANCEL_DELETE, ebufp);
		} else {
			PIN_FLIST_FLD_SET(p_arrayp, PIN_FLD_ACTION,
                        PIN_BILL_CANCEL_PRODUCT_ACTION_CANCEL_ONLY, ebufp);
		}	

	} /* end else */

	
	/*
	 * Allocate the output flist
	 */
	*out_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
	
	/*
	 * Cleanup
	 */
	PIN_FLIST_DESTROY(s_flistp, NULL);
	PIN_FLIST_DESTROY(r_flistp, NULL);

	/*
	 * Error?
	 */
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_subs_pol_get_cancel_specs error", ebufp);

	}

	return;
}





void
mso_get_mso_purchased_plan_from_product(
        //pcm_context_t   *ctxp,
	cm_nap_connection_t     *connp,
        pin_flist_t      *i_flistp,
        pin_errbuf_t    *ebufp)
{

	cm_op_info_t            *opstackp = connp->coip;
	pcm_context_t           *ctxp = connp->dm_ctx;

	int32           stack_opcode = 0;
	int32           initial_opcode = 0;
        char            debug_msg[250];
	int		*statusp = NULL;
	int64		db = 1;

	poid_t		*a_pdp = NULL;
	poid_t		*product_pdp = NULL;
	poid_t		*offering_pdp = NULL;

	pin_flist_t	*initial_flistp = NULL;
	pin_flist_t	*products_arrayp = NULL;
	pin_flist_t	*results_flistp = NULL;
	pin_flist_t	*p_flistp = NULL;
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*search_inflistp = NULL;
	pin_flist_t	*search_outflistp = NULL;
	pin_flist_t	*stack_flistp = NULL;

	char		*descrp = NULL;
	char		*prog_namep = NULL;

	int		search_flags = 256;
	char		*search_template = "select X from /mso_purchased_plan where F1 = V1 and F2 = V2 ";
	

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;

        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "mso_get_mso_purchased_plan_from_product input flist", i_flistp);
	
	a_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	db = PIN_POID_GET_DB(a_pdp);

	/*** Run up the stack to fetch the main input flist **********/
	while(opstackp != NULL)
	{
		stack_opcode = opstackp->opcode;
		sprintf(debug_msg, "Calling previous opcode is %s", pcm_opcode_to_opname(stack_opcode));
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);

		initial_flistp = opstackp->in_flistp;
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prev opcode flistp",initial_flistp);
		initial_opcode = opstackp->opcode;

		prog_namep = PIN_FLIST_FLD_GET(initial_flistp,PIN_FLD_PROGRAM_NAME,1,ebufp);

		if((initial_opcode == MSO_OP_CUST_TOP_UP_BB_PLAN) ||( initial_opcode == MSO_OP_CUST_CHANGE_PLAN)||
			(initial_opcode == MSO_OP_CUST_CANCEL_PLAN ) || (initial_opcode == PCM_OP_SUBSCRIPTION_CANCEL_DEAL)
			|| (initial_opcode == MSO_OP_CUST_TERMINATE_SERVICE)
			|| ((prog_namep) && (strstr(prog_namep,"pin_cycle"))))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"No need to handle mso_purchased_plan separately");
			return;
		}
		if(initial_opcode == PCM_OP_SUBSCRIPTION_CANCEL_PRODUCT)
		{
			stack_flistp = PIN_FLIST_COPY(initial_flistp,ebufp);
			PIN_FLIST_FLD_COPY(stack_flistp,PIN_FLD_PROGRAM_NAME,i_flistp,PIN_FLD_PROGRAM_NAME,ebufp);
			PIN_FLIST_FLD_COPY(stack_flistp,PIN_FLD_SERVICE_OBJ,i_flistp,PIN_FLD_SERVICE_OBJ,ebufp);
		}


		opstackp = opstackp->next;
	}
		
	/*** Fetch the required fields from stack flist *****/
	if(stack_flistp != NULL)
	{
		products_arrayp = PIN_FLIST_ELEM_GET(stack_flistp,PIN_FLD_PRODUCTS,PIN_ELEMID_ANY, 1, ebufp);
		if(products_arrayp != NULL)
		{
			product_pdp = PIN_FLIST_FLD_GET(products_arrayp, PIN_FLD_PRODUCT_OBJ, 0,ebufp);
			offering_pdp = PIN_FLIST_FLD_GET(products_arrayp, PIN_FLD_OFFERING_OBJ, 0,ebufp);


			search_inflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
			PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);
			args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, a_pdp, ebufp);
			args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp);
			p_flistp =  PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY,ebufp);
			PIN_FLIST_FLD_SET(p_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, offering_pdp, ebufp);

			results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"mso_get_mso_purchased_plan_from_product: search input list", search_inflistp);

			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				//On Error
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,
						"mso_get_mso_purchased_plan_from_product Error preparation", search_inflistp);
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_get_mso_purchased_plan_from_product Error preparation", ebufp);
				PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
				return;

			}
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp,&search_outflistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"mso_get_mso_purchased_plan_from_product: search out list", search_outflistp);

			PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				//On Error
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,
						"mso_get_mso_purchased_plan_from_product Error search", search_inflistp);
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_get_mso_purchased_plan_from_product Error search", ebufp);
				PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
				PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
				return;
			}
			
			results_flistp = PIN_FLIST_ELEM_GET(search_outflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY,1,ebufp);
			if(results_flistp != NULL)
			{
				statusp = PIN_FLIST_FLD_GET(results_flistp,PIN_FLD_STATUS,0,ebufp);
				descrp = PIN_FLIST_FLD_GET(results_flistp,PIN_FLD_DESCR, 0, ebufp);
				if((statusp && (*statusp == MSO_PROV_ACTIVE)) && (strcmp(descrp,"base plan")==0))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"mso_purchased_plan active. Cancellation required");
					
					/***** cancel plan and deal ********/
					mso_cancel_deal(ctxp,i_flistp,offering_pdp,ebufp);
					if(PIN_ERRBUF_IS_ERR(ebufp))
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
							"mso_cancel_deal returned error",ebufp);
						return;
					}
					mso_cancel_mso_purchased_plan_and_service(ctxp,i_flistp,ebufp);
					if(PIN_ERRBUF_IS_ERR(ebufp))
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
							"mso_cancel_mso_purchased_plan_and_service returned error",ebufp);
						return;
					}

				}

			}

		}

	} else {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"No stack flist found.Could not cancel mso_purchased_plan");
		return;
	}

}


void
mso_cancel_mso_purchased_plan_and_service(
        pcm_context_t   *ctxp,
        pin_flist_t      *i_flistp,
        pin_errbuf_t    *ebufp)
{

	int32		validity_expiry = VALIDITY_EXPIRY;
	int32           prov_status = MSO_PROV_DEACTIVE;
	int32           status_success = 0;

	pin_flist_t	*deactivate_flistp = NULL;
	pin_flist_t	*deactivate_out_flistp = NULL;
	int32           status = 1;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;

        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "mso_cancel_mso_purchased_plan_and_service input flist", i_flistp);
	
	deactivate_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_POID,deactivate_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_POID,deactivate_flistp, PIN_FLD_USERID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_SERVICE_OBJ,deactivate_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_SET(deactivate_flistp,PIN_FLD_STATUS,&prov_status,ebufp);
        PIN_FLIST_FLD_SET(deactivate_flistp,PIN_FLD_MODE, &validity_expiry, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, deactivate_flistp, PIN_FLD_PROGRAM_NAME, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_DEACTIVATE_BB_SERVICE input flist", deactivate_flistp);
        PCM_OP(ctxp, MSO_OP_CUST_DEACTIVATE_BB_SERVICE, 0, deactivate_flistp, &deactivate_out_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_DEACTIVATE_BB_SERVICE output flist", deactivate_out_flistp);

        status = *(int32 *)PIN_FLIST_FLD_GET(deactivate_out_flistp,PIN_FLD_STATUS,0, ebufp);

        if(status != status_success){
		pin_set_err(ebufp, PIN_ERRLOC_FM,PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        MSO_ERR_DEACTIVATE_BB_SERVICE_FAILED, 0, 0, MSO_OP_CUST_DEACTIVATE_BB_SERVICE);

		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"mso_cancel_mso_purchased_plan_and_service error",ebufp);
	}

	PIN_FLIST_DESTROY_EX(&deactivate_flistp,NULL);
	PIN_FLIST_DESTROY_EX(&deactivate_out_flistp,NULL);
}


void
mso_cancel_deal(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
	poid_t		*offering_pdp,
        pin_errbuf_t    *ebufp)
{

	pin_flist_t	*read_flistp = NULL;
	pin_flist_t	*read_out_flistp = NULL;
	pin_flist_t	*deactivate_flistp = NULL;
	pin_flist_t	*deactivate_out_flistp = NULL;
	pin_flist_t	*deal_flistp = NULL;

	int		flags = PIN_RATE_FLG_CUT;
	
        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;

        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "mso_cancel_deal input flist", i_flistp);
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG,"mso_cancel_deal poid",offering_pdp);
		
	read_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_flistp,PIN_FLD_POID,offering_pdp,ebufp);
        PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_flistp, &read_out_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read offering poid output flist", read_out_flistp);
	
	deactivate_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_POID,deactivate_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_SERVICE_OBJ,deactivate_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, deactivate_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_SET(deactivate_flistp,PIN_FLD_DESCR,"Cancel product - Validity expired",ebufp);
	flags = PIN_RATE_FLG_CUT;
	PIN_FLIST_FLD_SET(i_flistp,PIN_FLD_FLAGS, &flags,ebufp);
	deal_flistp = PIN_FLIST_ELEM_ADD(deactivate_flistp,PIN_FLD_DEAL_INFO,0,ebufp);
	PIN_FLIST_FLD_COPY(read_out_flistp,PIN_FLD_DEAL_OBJ,deal_flistp,PIN_FLD_DEAL_OBJ,ebufp);
	PIN_FLIST_FLD_COPY(read_out_flistp,PIN_FLD_PACKAGE_ID,deal_flistp,PIN_FLD_PACKAGE_ID,ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_SUBSCRIPTION_CANCEL_DEAL input flist", deactivate_flistp);
        PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_CANCEL_DEAL, 0, deactivate_flistp, &deactivate_out_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_SUBSCRIPTION_CANCEL_DEAL output flist", deactivate_out_flistp);

	PIN_FLIST_DESTROY_EX(&deactivate_flistp,NULL);
	PIN_FLIST_DESTROY_EX(&deactivate_out_flistp,NULL);
	PIN_FLIST_DESTROY_EX(&read_flistp,NULL);
	PIN_FLIST_DESTROY_EX(&read_out_flistp,NULL);



}

