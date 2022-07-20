/*
 *      Copyright (c) 1999-2006 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 */

#ifndef lint
static  char    Sccs_id[] = "@(#)%Portal Version: fm_cust_pol_read_plan.c:BillingVelocityInt:6:2006-Sep-05 04:32:12 %";
#endif

#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>

#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pinlog.h"

/*******************************************************************
 * Exported functions
 *******************************************************************/
EXPORT_OP void
op_cust_pol_read_plan(
        cm_nap_connection_t	*connp,
	int32			opcode,
        int32			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp);

/*******************************************************************
 * Local functions
 *******************************************************************/
static void
fm_cust_pol_read_plan(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**o_flistpp,
        pin_errbuf_t	*ebufp);
static void
fm_cust_pol_read_plan_deal(pcm_context_t *ctxp,
                           pin_flist_t *s_flistp,
                           int32 is_account_deal,
			   int32 deal_array,
                           pin_errbuf_t *ebufp);

PIN_EXPORT void
fm_cust_convert_prod_disc_date_details_into_offset_unit(
	pcm_context_t   *ctxp,
	int32		infld,
	pin_flist_t     **d_flistpp,
	pin_errbuf_t    *ebufp);

PIN_EXPORT void
fm_cust_validity_decode_details(
	pcm_context_t   *ctxp,
	int32           details,
	int32           *unit_p,
	int32           *offset_p,
	pin_errbuf_t    *ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_READ_PLAN operation. 
 *******************************************************************/
void
op_cust_pol_read_plan(
        cm_nap_connection_t	*connp,
	int32			opcode,
        int32			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*r_flistp = NULL;

	/***********************************************************
	 * Null out results until we have some.
	 ***********************************************************/
	*ret_flistpp = NULL;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_CUST_POL_READ_PLAN) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_cust_pol_read_plan", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What did we get?
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_cust_pol_read_plan input flist", i_flistp);

	/***********************************************************
	 * Call main function to do it
	 ***********************************************************/
	fm_cust_pol_read_plan(ctxp, i_flistp, &r_flistp, ebufp);

	/***********************************************************
	 * Set the results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		*ret_flistpp = NULL;
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_cust_pol_read_plan error", ebufp);
	} else {
		*ret_flistpp = r_flistp;
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_read_plan return flist", *ret_flistpp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_read_plan_deal()
 *
 *	For the specified deal object, forms a tree.
 *      Implemented as a subroutine since it is called once for each
 *      service, and then once for any account level deals
 *
 *******************************************************************/        
static void
fm_cust_pol_read_plan_deal(pcm_context_t *ctxp,
                           pin_flist_t *s_flistp,
                           int32 is_account_deal,
			   int32 deal_array,
                           pin_errbuf_t *ebufp)
{
	pin_flist_t	*flistp = NULL;		/* temp flist */
	pin_flist_t	*d_flistp = NULL;	/* dummy flist */
	pin_flist_t	*rd_flistp = NULL;	/* deal flist (read) */
	pin_flist_t	*p_flistp = NULL;	/* product flist */
	int		elem_id;
        void            *vp = NULL;
	poid_t		*d_pdp = NULL;

	pin_cookie_t	p_cookie = NULL;

        /***************************************************
         * No point doing any work if there isn't a deal
         ***************************************************/
        vp = PIN_FLIST_FLD_GET(s_flistp, PIN_FLD_DEAL_OBJ, 1, ebufp);
        if (PIN_POID_IS_NULL((poid_t *)vp)) {
                /*
                 * Don't drop the DEAL_OBJ is this is an account deal,
                 * because even though it is NULL, it breaks admin for good
                 * measure!
                 */
                if (!is_account_deal) {
                        PIN_FLIST_FLD_DROP(s_flistp, PIN_FLD_DEAL_OBJ, ebufp);
                }
                return;
        }
        
        /***************************************************
         * Read the deal object and add it to the plan tree.
         ***************************************************/
        d_flistp = PIN_FLIST_CREATE(ebufp);

        d_pdp = PIN_POID_COPY((poid_t *)vp, ebufp);
        PIN_FLIST_FLD_SET(d_flistp, PIN_FLD_POID, vp, ebufp);

        PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, d_flistp, &rd_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&d_flistp, NULL);

        PIN_FLIST_FLD_DROP(rd_flistp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_PUT(rd_flistp, PIN_FLD_DEAL_OBJ, (poid_t *)d_pdp, ebufp);

        /***************************************************
         * Walk thru the products array and read the fields
         * needed from the respective product obj.
         ***************************************************/
        while ((flistp = PIN_FLIST_ELEM_GET_NEXT(rd_flistp, 
                                                 PIN_FLD_PRODUCTS, &elem_id, 1,
                                                 &p_cookie, ebufp)) != NULL) {
                d_flistp = PIN_FLIST_CREATE(ebufp);

                vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);

                PIN_FLIST_FLD_SET(d_flistp, PIN_FLD_POID, vp, ebufp);

                PIN_FLIST_FLD_SET(d_flistp, PIN_FLD_NAME, NULL, ebufp);
                PIN_FLIST_FLD_SET(d_flistp, PIN_FLD_DESCR, NULL, ebufp);
                PIN_FLIST_FLD_SET(d_flistp, PIN_FLD_OWN_MAX, NULL, ebufp);
                PIN_FLIST_FLD_SET(d_flistp, PIN_FLD_OWN_MIN, NULL, ebufp);

                PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, d_flistp, &p_flistp, ebufp);
                PIN_FLIST_FLD_DROP(p_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_CONCAT(flistp, p_flistp, ebufp);

                PIN_FLIST_DESTROY_EX(&d_flistp, NULL);
                PIN_FLIST_DESTROY_EX(&p_flistp, NULL);
        }

		p_cookie = NULL;
		
        while ((flistp = PIN_FLIST_ELEM_GET_NEXT(rd_flistp, 
                                                 PIN_FLD_DISCOUNTS, &elem_id, 1,
                                                 &p_cookie, ebufp)) != NULL) {
                d_flistp = PIN_FLIST_CREATE(ebufp);

                vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_DISCOUNT_OBJ, 0, ebufp);

                PIN_FLIST_FLD_SET(d_flistp, PIN_FLD_POID, vp, ebufp);

                PIN_FLIST_FLD_SET(d_flistp, PIN_FLD_NAME, NULL, ebufp);
                PIN_FLIST_FLD_SET(d_flistp, PIN_FLD_DESCR, NULL, ebufp);
                PIN_FLIST_FLD_SET(d_flistp, PIN_FLD_OWN_MAX, NULL, ebufp);
                PIN_FLIST_FLD_SET(d_flistp, PIN_FLD_OWN_MIN, NULL, ebufp);

                PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, d_flistp, &p_flistp, ebufp);
                PIN_FLIST_FLD_DROP(p_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_CONCAT(flistp, p_flistp, ebufp);

                PIN_FLIST_DESTROY_EX(&d_flistp, NULL);
                PIN_FLIST_DESTROY_EX(&p_flistp, NULL);
        }

	/* Decode the encoded dates into unit and offset */
	fm_cust_convert_prod_disc_date_details_into_offset_unit(ctxp,
		PIN_FLD_PRODUCTS, &rd_flistp, ebufp);

	fm_cust_convert_prod_disc_date_details_into_offset_unit(ctxp,
		 PIN_FLD_DISCOUNTS, &rd_flistp, ebufp);

	if (deal_array) {
		PIN_FLIST_FLD_DROP(rd_flistp, PIN_FLD_DEAL_OBJ, ebufp);
                PIN_FLIST_CONCAT(s_flistp, rd_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&rd_flistp, NULL);
	} 
	else {
        	PIN_FLIST_ELEM_PUT(s_flistp, rd_flistp, PIN_FLD_DEALS, 0, ebufp);
	}

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_read_plan_deal error", ebufp);
	} 
}                           
        
/*******************************************************************
 * fm_cust_pol_read_plan()
 *
 *	For the specified plan object, forms a tree.
 *
 *******************************************************************/
static void
fm_cust_pol_read_plan(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**o_flistpp,
        pin_errbuf_t	*ebufp)
{
        pin_flist_t     *d_flistp = NULL;
	pin_flist_t	*s_flistp = NULL;	/* service flist */
	pin_flist_t	*rp_flistp = NULL;	/* product flist (read) */

	pin_cookie_t	d_cookie = NULL;
	pin_cookie_t	cookie = NULL;

	int32		rec_id = 0;
	int32		d_rec_id = 0;

	void		*vp = NULL;


	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Read the plan object.
	 ***********************************************************/
	d_flistp = PIN_FLIST_CREATE(ebufp);
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(d_flistp, PIN_FLD_POID, vp, ebufp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 
		PCM_OPFLG_ORDER_BY_REC_ID, d_flistp, &rp_flistp, ebufp);

	PIN_FLIST_DESTROY_EX(&d_flistp, NULL);

	while ((s_flistp = PIN_FLIST_ELEM_GET_NEXT(rp_flistp, PIN_FLD_SERVICES,
                                                   &rec_id, 1, &d_cookie,
                                                   ebufp)) != NULL) {
		vp = PIN_FLIST_FLD_GET(s_flistp, PIN_FLD_DEAL_OBJ, 1, ebufp);
		if (vp && PIN_POID_IS_NULL(vp)) {
			cookie = NULL;
			while ((d_flistp = PIN_FLIST_ELEM_GET_NEXT(s_flistp,
				PIN_FLD_DEALS, &d_rec_id, 1, &cookie, ebufp))
				!= NULL) {
                		fm_cust_pol_read_plan_deal(ctxp, d_flistp, 0, 
					 1, ebufp);
			}
		}
		else {
                	fm_cust_pol_read_plan_deal(ctxp, s_flistp, 0, 0, ebufp);
		}	
	}

        /*************************************************************
         * Now deal with account level plans/deals/products (ie those
         * with no services associated)
         *************************************************************/
        fm_cust_pol_read_plan_deal(ctxp, rp_flistp, 1, 0, ebufp);
        
	/***********************************************************
	 * Errors.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_read_plan error", ebufp);
		/**************************************************
		 * Since there is an error return a NULL flist and
		 * delete the one we read
		 **************************************************/
		*o_flistpp = NULL;
		PIN_FLIST_DESTROY_EX( &rp_flistp, NULL );
	} else {
		*o_flistpp = rp_flistp; 
	}
	

	return;
}


/*************************************************************************
For each product on the deal, if purchase/cycle/usage start/end dates are
greater than 0, then these are encoded relative dates need to decode into
unit and offset. Drop the details field after conversion.
*************************************************************************/
void
fm_cust_convert_prod_disc_date_details_into_offset_unit(
	pcm_context_t   *ctxp,
	int32		infld,
	pin_flist_t     **d_flistpp,
	pin_errbuf_t    *ebufp)
{


	pin_flist_t     *elem_flistp = NULL;
	void            *vp = NULL;
	int32           details = 0;
	int32           unit = 0;
	int32           offset = 0;
	int32           element_id = 0;
	pin_cookie_t    cookie = NULL;

	while ((elem_flistp = PIN_FLIST_ELEM_GET_NEXT(*d_flistpp,
				infld, &element_id, 1, &cookie,
				ebufp)) != (pin_flist_t *)NULL) {

		vp = (int32 *)PIN_FLIST_FLD_GET(elem_flistp,
                                PIN_FLD_PURCHASE_START_DETAILS, 1,ebufp);

		if (vp) {
			details = *(int32 *)vp;
			fm_cust_validity_decode_details(ctxp,details,
						&unit,&offset,ebufp);
			if(unit != 0){
				PIN_FLIST_FLD_SET(elem_flistp,
					PIN_FLD_PURCHASE_START_UNIT ,
					(void * )&unit, ebufp);

				PIN_FLIST_FLD_SET(elem_flistp,
					PIN_FLD_PURCHASE_START_OFFSET ,
					(void * )&offset, ebufp);
			}
		}

		vp = NULL;
		details = unit = offset =0;

		vp = (int32 *)PIN_FLIST_FLD_GET(elem_flistp,
				PIN_FLD_PURCHASE_END_DETAILS, 1,ebufp);

		if (vp) {  
			details = *(int32 *)vp;
			fm_cust_validity_decode_details(ctxp,details,
						&unit,&offset,ebufp);
			if(unit != 0){ 
				PIN_FLIST_FLD_SET(elem_flistp,
					PIN_FLD_PURCHASE_END_UNIT ,
					(void * )&unit, ebufp);

				PIN_FLIST_FLD_SET(elem_flistp,
					PIN_FLD_PURCHASE_END_OFFSET ,
					(void * )&offset, ebufp);
			}
		}

		vp = NULL;
		details = unit = offset =0;

		vp = (int32 *)PIN_FLIST_FLD_GET(elem_flistp,
				PIN_FLD_CYCLE_START_DETAILS, 1,ebufp);
		if (vp) { 
			details = *(int32 *)vp;
			fm_cust_validity_decode_details(ctxp,details,
						&unit,&offset,ebufp);
			if(unit != 0){ 
				PIN_FLIST_FLD_SET(elem_flistp,
					PIN_FLD_CYCLE_START_UNIT ,
					 (void * )&unit, ebufp);

				PIN_FLIST_FLD_SET(elem_flistp,
					PIN_FLD_CYCLE_START_OFFSET ,
					(void * )&offset, ebufp);
			}
		}

		vp = NULL;
		details = unit = offset =0;

		vp = (int32 *)PIN_FLIST_FLD_GET(elem_flistp,
				PIN_FLD_CYCLE_END_DETAILS, 1,ebufp);

		if(vp){ 
			details = *(int32 *)vp;
			fm_cust_validity_decode_details(ctxp,details,
						&unit,&offset,ebufp);
			if(unit != 0){
				PIN_FLIST_FLD_SET(elem_flistp,
					PIN_FLD_CYCLE_END_UNIT ,
					(void * )&unit, ebufp);

				PIN_FLIST_FLD_SET(elem_flistp,
					PIN_FLD_CYCLE_END_OFFSET ,
					(void * )&offset, ebufp);
			}
		}

		vp = NULL;
		details = unit = offset =0;

		vp = (int32 *)PIN_FLIST_FLD_GET(elem_flistp,
				PIN_FLD_USAGE_START_DETAILS, 1,ebufp);

		if (vp) {
			details = *(int32 *)vp;
			fm_cust_validity_decode_details(ctxp,details,
							&unit,&offset,ebufp);

			if(unit != 0){
				PIN_FLIST_FLD_SET(elem_flistp,
					PIN_FLD_USAGE_START_UNIT ,
					(void * )&unit, ebufp);

				PIN_FLIST_FLD_SET(elem_flistp,
					PIN_FLD_USAGE_START_OFFSET ,
					(void * )&offset, ebufp);
			}
		}

		vp = NULL;
		details = unit = offset =0;

		vp = (int32 *)PIN_FLIST_FLD_GET(elem_flistp,
			PIN_FLD_USAGE_END_DETAILS, 1,ebufp);

		if (vp) {                    
			details = *(int32 *)vp;
			fm_cust_validity_decode_details(ctxp,details,
				&unit,&offset,ebufp);
			if(unit != 0){
				PIN_FLIST_FLD_SET(elem_flistp,
					PIN_FLD_USAGE_END_UNIT ,
					(void * )&unit, ebufp);

				PIN_FLIST_FLD_SET(elem_flistp,
					PIN_FLD_USAGE_END_OFFSET ,
					(void * )&offset, ebufp);
			}
		}
	}

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"Error in converting product/discount into offset unit", ebufp);
	} 
}

/****************************************************************************
 * This function decode the details filed into mode, unit and offset
 ***************************************************************************/
void 
fm_cust_validity_decode_details(
	pcm_context_t   *ctxp,
	int32           details,
	int32		*unit_p,
	int32		*offset_p,
	pin_errbuf_t    *ebufp)
{
	pin_validity_modes_t      mode = 0; 
	
	/*********************************************************
	* Check if details is first usage or relative or immediate
	* Mode = PIN_START_FIRST_USAGE.
	* Mode = PIN_START_ABSOLUTE.
	* Mode = PIN_START_IMMEDIATE. 
	*********************************************************/

	mode = PIN_VALIDITY_GET_MODE(details);

	switch(mode){
		case PIN_VALIDITY_FIRST_USAGE:
			*unit_p = -1;
			*offset_p = 0;
			break;

		case PIN_VALIDITY_RELATIVE:
			*(unit_p)   = PIN_VALIDITY_GET_UNIT(details); 
			*(offset_p) = PIN_VALIDITY_GET_OFFSET(details);
			break;

		case PIN_VALIDITY_ABSOLUTE:
		case PIN_VALIDITY_IMMEDIATE:
			*unit_p = 0;
			*offset_p = 0;
			break;

		default:
			*unit_p = 0; 
			*offset_p = 0;
			break;
	}
	
}
