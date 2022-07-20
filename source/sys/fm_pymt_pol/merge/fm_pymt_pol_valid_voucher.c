/*
 *
 * Copyright (c) 2004, 2009, Oracle and/or its affiliates. 
 * All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 *
 */

#ifndef lint
static char Sccs_id[] = "@(#)$Id: fm_pymt_pol_valid_voucher.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 05:46:02 nishahan Exp $";
#endif

#include "pcm.h"
#include "ops/pymt.h"
#include "ops/voucher.h"
#include "ops/cust.h"
#include "pinlog.h"
#include "pin_flds.h"
#include "cm_fm.h"
#include "psiu_currency.h"
#include "pbo_decimal.h"



/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_pymt_pol_valid_voucher(
        cm_nap_connection_t	*connp,
	int32			opcode,
        int32			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**r_flistpp,
        pin_errbuf_t		*ebufp);

static void 
fm_pymt_pol_call_voucher_manager(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		**out_flistpp,
	pin_errbuf_t		*ebufp);


static void 
fm_pymt_pol_find_shortest_validity(
	pin_flist_t		*i_flistpp,
	pin_flist_t		**out_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_PYMT_POL_VALID_VOUCHER operation.
 *******************************************************************/
void
op_pymt_pol_valid_voucher(
        cm_nap_connection_t	*connp,
	int32			opcode,
        int32			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**r_flistpp,
        pin_errbuf_t		*ebufp)
{
	pin_flist_t		*flistp = NULL;
	pin_flist_t		*t_flistp = NULL;
	void			*vp	= NULL;
	poid_t			*a_pdp	= NULL;
	poid_t			*d_pdp	= NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);
        
	/*******************************************************************
	 * Insanity check.
 	 *******************************************************************/
	if (opcode != PCM_OP_PYMT_POL_VALID_VOUCHER) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_pymt_pol_valid_voucher bad opcode error", ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: What we got.
 	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_pymt_pol_valid_voucher input flist", i_flistp);

	/* Get the voucher details */
	flistp = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_INHERITED_INFO, 
						0, ebufp);
	flistp = PIN_FLIST_ELEM_GET(flistp, PIN_FLD_VOUCHERS_INFO, 
						PIN_ELEMID_ANY, 0, ebufp);

	/** Create the return flist **/
	*r_flistpp = PIN_FLIST_CREATE(ebufp);
	a_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

        d_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(a_pdp), "/device/voucher",
                                    -1, ebufp);
   
	PIN_FLIST_FLD_PUT(*r_flistpp, PIN_FLD_POID, (void *)d_pdp, ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ACCOUNT_OBJ, a_pdp, ebufp);

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 0, ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_PROGRAM_NAME, vp, ebufp);

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_START_T, 1, ebufp);
	if (vp) PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_START_T, vp, ebufp);

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 1, ebufp);
	if (vp) PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_END_T, vp, ebufp);

	vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_BILLINFO_OBJ, 0, ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_BILLINFO_OBJ, vp, ebufp);

	vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_BAL_GRP_OBJ, 1, ebufp);
	if (vp) PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_BAL_GRP_OBJ, vp, ebufp);

	vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	if (vp) {
		t_flistp = PIN_FLIST_ELEM_ADD(*r_flistpp, PIN_FLD_SERVICES, 0, ebufp);
		PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_SERVICE_OBJ, vp, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_SERVICE_OBJ, vp, ebufp);
	} else {
		/*
		 * Since Services array is mandatory for
		 * PCM_OP_VOUCHER_ASSOCIATE_VOUCHER opcode, passing account obj
		 * if no service obj passed in i/p flist.
		 */
		t_flistp = PIN_FLIST_ELEM_ADD(*r_flistpp, PIN_FLD_SERVICES, 0, ebufp);
		PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_ACCOUNT_OBJ, a_pdp, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ACCOUNT_OBJ, a_pdp, ebufp);
	}

	t_flistp = PIN_FLIST_SUBSTR_ADD(*r_flistpp, PIN_FLD_DEVICE_VOUCHER, ebufp);
	vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_DEVICE_ID, 0, ebufp);
	PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_DEVICE_ID, vp, ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_DEVICE_ID, vp, ebufp);

	vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_VOUCHER_PIN, 0, ebufp);
	PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_VOUCHER_PIN, vp, ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_VOUCHER_PIN, vp, ebufp);

        /******************************************************************
         ** Plug in calls to Voucher Manager here.
         *****************************************************************/
       	fm_pymt_pol_call_voucher_manager(connp->dm_ctx, flags,
                                                        r_flistpp, ebufp); 
	
	/******************************************************************	
	 ** To enable the Simulator uncomment the code below and
	 ** comment out call to the Voucher Manager above.
	 *****************************************************************/ 
	 	
	/*fm_pymt_pol_valid_voucher_simulator(r_flistpp, ebufp);*/



	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_pymt_pol_valid_voucher return flist", *r_flistpp);
	return;
}


/***************************************************************************
 * fm_pYmt_pol_call_voucher_manager()
 * This function interface with Portal voucher manager.
 **************************************************************************/
static void
fm_pymt_pol_call_voucher_manager (
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		**out_flistpp,
	pin_errbuf_t		*ebufp)
{
        pin_flist_t             *ret_flistp = NULL;
        pin_flist_t             *flistp = NULL;
        pin_flist_t             *tr_flistp = NULL;
        pin_flist_t             *ex_bal_flistp = NULL;
        pin_flist_t             *bal_flistp = NULL;
        pin_flist_t             *s_flistp = NULL;
        pin_flist_t             *r_s_flistp = NULL;
        pin_flist_t             *t_flistp = NULL;
        pin_flist_t             *vv_flistp = NULL;
	pin_flist_t             *sub_bal_flistp = NULL;
	pin_flist_t		*tax_jurisdictions_flistp = NULL;
        pin_cookie_t            cookie = NULL;
        int32                   rec_id = 0;
        u_int32                 *res_id = NULL;
        u_int32                 currency = 0;
        int32                   cnt_curr_type = 0;
        void                    *vp = NULL;
        char                    msg[256] = "";
        pin_decimal_t           *amountp = NULL;
        int32                   dummy_currency = 0;
        pin_decimal_t           *accumulated_amtp = NULL;


        if (PIN_ERR_IS_ERR(ebufp))
                return;
        PIN_ERR_CLEAR_ERR(ebufp);

        accumulated_amtp = pin_decimal("0.0", ebufp);

        /***** Call the Voucher Associate opcode *****/
        PCM_OP(ctxp, PCM_OP_VOUCHER_ASSOCIATE_VOUCHER, flags,
                                        *out_flistpp, &ret_flistp, ebufp);

        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_pymt_pol_call_voucher_manager error", ebufp);
        }

	vv_flistp = PIN_FLIST_SUBSTR_GET(ret_flistp, PIN_FLD_DEVICE_VOUCHER, 0,
					ebufp);
	if (vv_flistp) {
		vp = PIN_FLIST_FLD_GET(vv_flistp, PIN_FLD_EXPIRATION_T, 0,
					ebufp);
		PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_EXPIRATION_T, vp,
					ebufp);
	}

        tr_flistp = PIN_FLIST_SUBSTR_ADD(*out_flistpp,
                        PIN_FLD_TOPUP_RESOURCE_INFO, ebufp);

       /* Get the balance impacts information */
        while ((bal_flistp = PIN_FLIST_ELEM_GET_NEXT(ret_flistp,
                                 PIN_FLD_BAL_IMPACTS, &rec_id, 1, &cookie,
                                     ebufp)) != (pin_flist_t *)NULL) {

		res_id = (u_int32 *)PIN_FLIST_FLD_GET(bal_flistp,
				PIN_FLD_RESOURCE_ID, 0, ebufp);
		if (res_id && (psiu_currency_is_currency(*res_id))) {
			cnt_curr_type++;
			amountp = (pin_decimal_t *)PIN_FLIST_FLD_GET(bal_flistp,
					PIN_FLD_AMOUNT, 0, ebufp);

			pbo_decimal_add_assign (accumulated_amtp, amountp, ebufp);

			/* 
			 * For voucher topups, if the resource is a currency
			 * resource, the credit will be applied to the customer
			 * eventually by making a payment using PCM_OP_PYMT_COLLECT.
			 * So, make sure the amount is positive.
			 */
			if (pbo_decimal_sign(amountp, ebufp) < 0) {
				if(amountp) {
					amountp=pbo_decimal_copy(amountp,ebufp);
					pbo_decimal_negate_assign(amountp, ebufp);
					PIN_FLIST_FLD_PUT(bal_flistp, 
						PIN_FLD_AMOUNT,(void *)amountp, ebufp);
				}
			}
			PIN_FLIST_ELEM_SET(tr_flistp, bal_flistp,
				PIN_FLD_BAL_IMPACTS, rec_id, ebufp);

			currency = *res_id;
		} else { 
			/* the resource is non-currency */
			PIN_FLIST_ELEM_SET(tr_flistp, bal_flistp,
				PIN_FLD_BAL_IMPACTS, rec_id, ebufp);
		}
	}

	/********************************************************
	 * For voucher topup, if the resource is a currency
	 * resource, the credit will be applied to the customer
	 * eventually by making a payment using PCM_OP_PYMT_COLLECT.
	 * So, make sure the amount is positive.
	 ********************************************************/
	if (pbo_decimal_sign(accumulated_amtp, ebufp) < 0) {
		pbo_decimal_negate_assign(accumulated_amtp, ebufp);
	}
	PIN_FLIST_FLD_PUT(*out_flistpp, PIN_FLD_AMOUNT, (void *)accumulated_amtp, ebufp);
 
	/*
	 * Set the PIN_FLD_CURRENCY
	 */
	if (currency) {
		PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_CURRENCY, 
				(void *)&currency, ebufp); 
	}

	rec_id=0;
	cookie = NULL;
	   /************************************************************ 
	   * Get the sub balance impacts information if exists which is passed
	   * by the PCM_OP_VOUCHER_ASSOCIATE_VOUCHER opcode and append to the 
	   * output flist. The sub balance impacts array contains the net and
	   * tax amount
	   ******************************************************************/
        while ((sub_bal_flistp = PIN_FLIST_ELEM_GET_NEXT(ret_flistp,
                                 PIN_FLD_SUB_BAL_IMPACTS, &rec_id, 1, &cookie,
                                     ebufp)) != (pin_flist_t *)NULL) {
					     
		PIN_FLIST_ELEM_SET(tr_flistp, sub_bal_flistp,
                               PIN_FLD_SUB_BAL_IMPACTS, rec_id, ebufp);
	}
	
	rec_id=0;
	cookie = NULL;
	/******************************************************************
	 * Get the tax jurisdictions array if exists which is passed
	 * by the PCM_OP_VOUCHER_ASSOCIATE_VOUCHER opcode and append to the 
	 * output flist.
	 ******************************************************************/
        while ((tax_jurisdictions_flistp = PIN_FLIST_ELEM_GET_NEXT(ret_flistp,
                                 PIN_FLD_TAX_JURISDICTIONS, &rec_id, 1, &cookie,
                                     ebufp)) != (pin_flist_t *)NULL) {
					     
		PIN_FLIST_ELEM_SET(tr_flistp, tax_jurisdictions_flistp,
                               PIN_FLD_TAX_JURISDICTIONS, rec_id, ebufp);
	}
	
        /*
         * If there is no currrency resource in the voucher, store a dummy
         * amount and a dummy currency in the output flist
         */
        if (cnt_curr_type == 0) {
            amountp = pin_decimal("0.0", ebufp);
            PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_AMOUNT, (void *)amountp,
                ebufp);
            PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_CURRENCY,
                (void *)&dummy_currency, ebufp);
            pin_decimal_destroy (amountp);
        }

        /*
         * We have the validity dates for each resource, but not for the voucher
         * itself. So, find the resource which has the shortest validity and use
         * to set the validity dates of the voucher
         */
        fm_pymt_pol_find_shortest_validity(ret_flistp,
                                               out_flistpp, ebufp);

	vp = PIN_FLIST_ELEM_GET(*out_flistpp, PIN_FLD_SERVICES, PIN_ELEMID_ANY,
					1, ebufp);
        if (vp) {
                PIN_FLIST_ELEM_DROP(*out_flistpp, PIN_FLD_SERVICES, PIN_ELEMID_ANY,
                                        ebufp);
        }
        vp = PIN_FLIST_SUBSTR_GET(*out_flistpp, PIN_FLD_DEVICE_VOUCHER, 1,
                                        ebufp);
        if (vp) {
                PIN_FLIST_SUBSTR_DROP(*out_flistpp, PIN_FLD_DEVICE_VOUCHER,
                                        ebufp);
        }

        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_pymt_pol_call_voucher_manager error", ebufp);
        }

        PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
        return;

}	

 
/*****************************************************************************
 * fm_pymt_pol_find_shortest_validity ()
 * This function examines the VALID_FROM and VALID_TO fields in each element
 * of a PIN_FLD_BAL_IMPACTS array and determines the shortest validity period.
 ****************************************************************************/
static void
fm_pymt_pol_find_shortest_validity (
	pin_flist_t		*i_flistp,
	pin_flist_t		**out_flistpp,
	pin_errbuf_t		*ebufp) {

        pin_flist_t             *flistp = NULL;
        pin_flist_t             *tmp_flistp = NULL;
        pin_cookie_t            cookie = NULL;
        int32                   rec_id = 0;
        time_t                  valid_to = 0;
        time_t                  valid_from = 0;
        time_t                  tmp_time = 0;
        void                    *vp = NULL;
        
        if (PIN_ERR_IS_ERR(ebufp)) {
                return; 
        }
	PIN_ERR_CLEAR_ERR(ebufp);

        flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_BAL_IMPACTS,
                         PIN_ELEMID_ANY, 0, ebufp);
        vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_VALID_FROM, 0, ebufp);
	if (vp) {
		valid_from = *(time_t *)vp;
	}
        vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_VALID_TO, 0, ebufp);
	if (vp) {
		valid_to = *(time_t *)vp;
	}

        while ((flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp,
                                 PIN_FLD_BAL_IMPACTS, &rec_id, 1, &cookie,
                                     ebufp)) != (pin_flist_t *)NULL) {
		vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_VALID_FROM, 0, ebufp);
		if (vp) {
			tmp_time = *(time_t *)vp;
			if (valid_from > tmp_time) {
				valid_from = tmp_time;
			}
		}
		vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_VALID_TO, 0, ebufp);
		if (vp) {
			tmp_time = *(time_t *)vp;
			if (valid_to > tmp_time) {
				valid_to = tmp_time;
			}
		}
        }

        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_VALID_FROM, (void *)&valid_from,
                             ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_VALID_TO, (void *)&valid_to,
                             ebufp);

        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_pymt_pol_find_shortest_validity error", ebufp);
        }
	return;
}

