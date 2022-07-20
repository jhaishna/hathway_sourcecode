/*******************************************************************
 * File:        fm_mso_cust_catv_discount_config.c
 * Opcode:      MSO_OP_CUST_DISCOUNT_COFIG
 * Owner:       Sasikumar
 * Created:     25-APR-2017
 * Description: This opcode is for configuring/Updating/Fetching discount details for customer/Plans

 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_catv_discount_config.c.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif


#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "pin_cust.h"
#include "ops/act.h"
#include "ops/cust.h"
#include "ops/bill.h"
#include "ops/bal.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "pin_currency.h"
#include "mso_ops_flds.h"
#include "mso_cust.h"
#include "mso_lifecycle_id.h"


EXPORT_OP void
op_mso_cust_catv_discount_config(
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

void fm_mso_cust_catv_service_disc(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**ret_flistp,
	pin_errbuf_t	*ebufp);

void fm_mso_cust_catv_plan_disc(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_flist_t     **ret_flistp,
        pin_errbuf_t    *ebufp);

void fm_mso_validate_acct_device(
        pcm_context_t    *ctxp,
        pin_flist_t     *i_flistp,
        pin_errbuf_t    *ebufp);

void calc_base_disc(
	pcm_context_t    *ctxp,
        pin_decimal_t   *discount,
	pin_decimal_t	*tax_rate,
        pin_decimal_t   **base_discount,
	pin_errbuf_t    *ebufp);

void
op_mso_cust_catv_discount_config(
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
	pcm_context_t	*ctxp = connp->dm_ctx;
	int32		disc_type = -1;
	int32		failure = 1;
	int32		status = -1;

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                return;
        }
        PIN_ERRBUF_CLEAR(ebufp);

        /*******************************************************************
         * Insanity Check
         *******************************************************************/
        if (opcode != MSO_OP_CUST_CATV_DISC_CONFIG) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_mso_cust_catv_discount_config error",
                        ebufp);
                return;
        }
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "op_mso_cust_catv_discount_config input flist", i_flistp);
	disc_type = *(int32 *) PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SERVICE_TYPE, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERRBUF_RESET(ebufp);
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "55001", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Discount type missing", ebufp);
                return;
        }
	if(disc_type == 0)
	{
        	fm_mso_cust_catv_service_disc(ctxp, i_flistp, r_flistpp, ebufp);
	}
	else
	{
		fm_mso_cust_catv_plan_disc(ctxp, i_flistp,  r_flistpp, ebufp);
	}
	status = *(int32 *) PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_STATUS, 0, ebufp);
	if (status == failure ||PIN_ERRBUF_IS_ERR(ebufp) )
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_YET_DONE, 0, 0, 0);
		PIN_ERR_LOG_MSG(1, "ERROR DURING DISCOUNT CONFIGURATION");
	
	}

		
	PIN_ERR_LOG_FLIST(3, "op_mso_cust_catv_discount_config return flist", *r_flistpp);
        return;
}


void fm_mso_cust_catv_service_disc(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_flist_t     **ret_flistp,
        pin_errbuf_t    *ebufp)
{
	pin_flist_t	*srch_iflistp = NULL;
	pin_flist_t	*srch_oflistp = NULL;
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*res_flistp = NULL;
	pin_flist_t	*srch_res_flistp = NULL;
	pin_flist_t	*wrt_iflistp = NULL;
	pin_flist_t	*wrt_oflistp = NULL;
	pin_flist_t	*create_iflistp = NULL;
	pin_flist_t	*create_oflistp = NULL;

	pin_cookie_t	cookie = NULL;
	
	poid_t		*srch_pdp = NULL;
	poid_t		*service_pdp = NULL;
	poid_t		*acct_pdp = NULL;
	poid_t		*service_disc_pdp = NULL;
	
	pin_decimal_t	*discount = NULL;
	pin_decimal_t	*disc_amt = NULL;
	pin_decimal_t	*one = pbo_decimal_from_str("1.0", ebufp);
	pin_decimal_t	*base_disc = NULL;
	
	int32		db = 1;
	int32		srch_flags = 256;
	int32		pkg_type = -1;
	int32		old_pkg_type = -1;
	int32		disc_flag = -1;	
	int32		old_disc_flag = -1;
	int32		failure = 1;
	int32		elem_id = 0;
	int32		success =0;
	
	time_t		*end_t = NULL;
	time_t		*disc_end_t = NULL;
	time_t		current_t = pin_virtual_time((time_t *)NULL);

	char		*acct_no = NULL;
	char		*device_id = NULL;
	char		*srch_tmpl = "select X from /mso_cfg_disc_service where 1.F1 = V1 and 1.F2 = V2 and 1.F3 = V3 and 1.F4 > V4 ";
	
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) 
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(3, "service level discount configuration", i_flistp);
	//Validations on input
	acct_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	device_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEVICE_ID, 1, ebufp);
	pkg_type = *(int32 *) PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_PKG_TYPE, 1, ebufp);
	discount = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DISCOUNT, 1, ebufp);
	disc_flag = *(int32 *) PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DISCOUNT_FLAGS, 1, ebufp);
	end_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 1, ebufp);

	if(acct_no == NULL || device_id == NULL || pkg_type == -1 || disc_flag == -1 || pbo_decimal_is_null(discount, ebufp))
	{
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "55002", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Mandatory fields missing:Accno/device_id/pkg_type/disc_type/discount are mandatory", ebufp);
		goto CLEANUP;
	}
	if(pkg_type < 0 || pkg_type > 10)
	{
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "55002", ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Invalid package_type  ", ebufp);
                goto CLEANUP;
	}	
	if(disc_flag == 0 && pbo_decimal_compare(discount, one, ebufp) >0)
	{
		PIN_ERR_LOG_MSG(3, "Percentage configured more than 100%");
		PIN_ERRBUF_RESET(ebufp);
                *ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "55003", ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "percentage discount amount cannot be more than 1", ebufp);
                goto CLEANUP;	
	}
	/*if(disc_flag ==0 && pbo_decimal_sign(discount, ebufp) <= 0)
        {
                PIN_ERRBUF_RESET(ebufp);
                *ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "55012", ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Discount Amount or percentage cannot be less than or equal to Zero", ebufp);

                goto CLEANUP;
        }*/

	fm_mso_validate_acct_device(ctxp, i_flistp, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{	
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "55004", ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Account number & Device doesnt match", ebufp);
                goto CLEANUP;
	}
	/*if(disc_flag == 1)
	{
		calc_base_disc(ctxp, discount, discount, &base_disc, ebufp);
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_RESET(ebufp);
			*ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "55006", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error during base discount calculation", ebufp);
			goto CLEANUP;
		}
	}
	else
	{
		base_disc = pbo_decimal_copy(discount, ebufp);;
	}*/
	base_disc = pbo_decimal_copy(discount, ebufp);
	acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	service_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(srch_iflistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &srch_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, srch_tmpl, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, service_pdp, ebufp);		

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_PKG_TYPE, &pkg_type, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_END_T, &current_t, ebufp);

	res_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS, 0, ebufp);
		
	PIN_ERR_LOG_FLIST(3, "fm_mso_cust_catv_service_disc : search flist", srch_iflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERRBUF_RESET(ebufp);
                *ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "55005", ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error during service disc configuration", ebufp);
                goto CLEANUP;
        }
	/******************************************************
	 * NTO 2.0: End existing discount
	 ******************************************************/
	while ((srch_res_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_oflistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
	{
		/****
		NTO 2.0
		disc_end_t =  PIN_FLIST_FLD_GET(srch_res_flistp, PIN_FLD_END_T, 0, ebufp);
		disc_amt = PIN_FLIST_FLD_GET(srch_res_flistp, PIN_FLD_DISCOUNT, 0, ebufp);
		old_disc_flag = *(int32 *) PIN_FLIST_FLD_GET(srch_res_flistp, PIN_FLD_DISCOUNT_FLAGS, 0, ebufp);
		
		if((end_t && disc_end_t && (*end_t != *disc_end_t)) || old_disc_flag != disc_flag || pbo_decimal_compare(base_disc, disc_amt,ebufp) !=0)
		{
			write = 1;
		}
		else
		{
			PIN_ERRBUF_RESET(ebufp);
	                *ret_flistp = PIN_FLIST_CREATE(ebufp);
        	        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
                	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
	                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "55006", ebufp);
        	        PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "discount Already exist", ebufp);
                	goto CLEANUP;
		}
		if(write)
		{*/
			wrt_iflistp = PIN_FLIST_CREATE(ebufp);
	                PIN_FLIST_FLD_COPY(srch_res_flistp, PIN_FLD_POID, wrt_iflistp, PIN_FLD_POID, ebufp);
			//if(end_t)
		 	PIN_FLIST_FLD_SET(wrt_iflistp, PIN_FLD_END_T, &current_t, ebufp);
	                //PIN_FLIST_FLD_SET(wrt_iflistp, PIN_FLD_DISCOUNT_FLAGS, &disc_flag, ebufp);
                        //PIN_FLIST_FLD_SET(wrt_iflistp, PIN_FLD_DISCOUNT, base_disc, ebufp);
			PIN_ERR_LOG_FLIST(3, "Discount update input flist", wrt_iflistp);
			PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, wrt_iflistp, &wrt_oflistp, ebufp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERRBUF_RESET(ebufp);
				*ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "55006", ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error during updating service disc configuration", ebufp);
				goto CLEANUP;
			}
			PIN_ERR_LOG_FLIST(3, "discount update output flist", wrt_oflistp);
			/**ret_flistp = PIN_FLIST_COPY(wrt_oflistp, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &success, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "0000", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Discount configuration updated successfully", ebufp);*/
		//}
	}
	//else
	//{
    if ( end_t && (fm_utils_time_round_to_midnight(*end_t) > fm_utils_time_round_to_midnight(current_t)))
    {
		PIN_ERR_LOG_MSG(3, "Create a new discount entry");
		create_iflistp = PIN_FLIST_CREATE(ebufp);
		service_disc_pdp = PIN_POID_CREATE(db, "/mso_cfg_disc_service", -1, ebufp);
		PIN_FLIST_FLD_PUT(create_iflistp, PIN_FLD_POID, service_disc_pdp, ebufp);
		PIN_FLIST_FLD_SET(create_iflistp, PIN_FLD_ACCOUNT_NO, acct_no, ebufp);
		PIN_FLIST_FLD_SET(create_iflistp, PIN_FLD_NAME, device_id, ebufp);
		PIN_FLIST_FLD_SET(create_iflistp, PIN_FLD_SERVICE_OBJ, service_pdp, ebufp);
		PIN_FLIST_FLD_SET(create_iflistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
		PIN_FLIST_FLD_SET(create_iflistp, MSO_FLD_PKG_TYPE, &pkg_type, ebufp);
		PIN_FLIST_FLD_SET(create_iflistp, PIN_FLD_DISCOUNT_FLAGS, &disc_flag, ebufp);
		PIN_FLIST_FLD_SET(create_iflistp, PIN_FLD_END_T, end_t, ebufp);
               	PIN_FLIST_FLD_SET(create_iflistp, PIN_FLD_DISCOUNT, base_disc, ebufp);
		
		PIN_ERR_LOG_FLIST(3, "Discount Creation input flist", create_iflistp);
		PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, create_iflistp, &create_oflistp, ebufp);
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_RESET(ebufp);
			*ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "55006", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error during creating service disc configuration", ebufp);
			goto CLEANUP;
		}
		PIN_ERR_LOG_FLIST(3, "Discount configuration output", create_oflistp);
		*ret_flistp = PIN_FLIST_COPY(create_oflistp, ebufp);
	}
    else if (wrt_oflistp)
    {
		*ret_flistp = PIN_FLIST_COPY(wrt_oflistp, ebufp);
    }
    else
    {
        *ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
    }

    PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &success, ebufp);
    PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "0000", ebufp);
    PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Discount configuration updated successfully", ebufp);

CLEANUP:
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ACCOUNT_NO, acct_no, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_DEVICE_ID, device_id, ebufp);
		PIN_FLIST_DESTROY_EX(&wrt_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&wrt_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&create_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&create_oflistp, NULL);
		if(!pbo_decimal_is_null(one, ebufp))
	        {
        	        pbo_decimal_destroy(&one);
        	}
		if(!pbo_decimal_is_null(base_disc, ebufp))
		{
			pbo_decimal_destroy(&base_disc);
		}
		return;		


}

void calc_base_disc(
	pcm_context_t	*ctxp,
	pin_decimal_t	*discount,
	pin_decimal_t	*tax_rate,
	pin_decimal_t	**base_discount,
	pin_errbuf_t	*ebufp)
{
	pin_decimal_t *hundred = NULL;
	pin_decimal_t *divisor = NULL;
	pin_decimal_t *disc_calc = NULL;
	char		msg[100] ="";

	hundred = pbo_decimal_from_str("100.0", ebufp);
	pbo_decimal_multiply_assign(tax_rate, hundred, ebufp);
	sprintf(msg, "total tax percent = %s", pbo_decimal_to_str(tax_rate, ebufp));
        PIN_ERR_LOG_MSG(3, msg);
	divisor = pbo_decimal_add(hundred, tax_rate, ebufp);
	sprintf(msg, "Divisor  = %s", pbo_decimal_to_str(divisor, ebufp));
        PIN_ERR_LOG_MSG(3, msg);	
	disc_calc = pbo_decimal_multiply(discount, hundred, ebufp);
	sprintf(msg, "discount calculate  = %s", pbo_decimal_to_str(disc_calc, ebufp));
        PIN_ERR_LOG_MSG(3, msg);
	pbo_decimal_divide_assign(disc_calc, divisor, ebufp);
	*base_discount = pbo_decimal_round(disc_calc, 2, ROUND_HALF_UP, ebufp);
	sprintf(msg, "new base discount = %s", pbo_decimal_to_str(*base_discount, ebufp));
	PIN_ERR_LOG_MSG(3, msg);
	if(!pbo_decimal_is_null(hundred, ebufp))
	{
		pbo_decimal_destroy(&hundred);
	}
	if(!pbo_decimal_is_null(divisor, ebufp))
	{
		pbo_decimal_destroy(&divisor);
	}
	if(!pbo_decimal_is_null(disc_calc, ebufp))
	{
		pbo_decimal_destroy(&disc_calc);
	}

	return;
}


void fm_mso_cust_catv_plan_disc(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_flist_t     **ret_flistp,
        pin_errbuf_t    *ebufp)
{
	pin_flist_t	*srch_iflistp = NULL;
	pin_flist_t	*srch_oflistp = NULL;
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*srch_res_flistp = NULL;
	pin_flist_t	*results_flistp = NULL;
	pin_flist_t	*plan_flistp = NULL;
	pin_flist_t	*disc_flistp = NULL;
	pin_flist_t	*wrt_iflistp = NULL;
	pin_flist_t	*wrt_oflistp = NULL;
	pin_flist_t	*create_iflistp = NULL;
	pin_flist_t	*create_oflistp = NULL;

	
	poid_t		*srch_pdp = NULL;
	poid_t		*plan_pdp = NULL;
	poid_t		*plan_disc_pdp = NULL;
	
	pin_decimal_t	*discount = NULL;
	pin_decimal_t	*base_disc = NULL;
	pin_decimal_t	*disc_amt = NULL;
	pin_decimal_t	*one = pbo_decimal_from_str("1.0", ebufp);

	time_t		*end_t = NULL;
	time_t		*disc_end_t = NULL;
	
	char		*city = NULL;
	char		*das_type = NULL;
	char		*plan_name = NULL;
	char		msg[500] = "";
	char		*plan_tmpl = "select X from /plan 1 where F1 = V1 ";
	char		*disc_tmpl = "select X from /mso_cfg_disc_plan where F1 = V1 and F2 = V2 and F3 = V3 and F4 = V4  and F5 = V5";
	
	int32		srch_flags = 256;
	int32		db = 1;
	int32		pp_type = -1;
	int32		conn_type = -1;
	int32		disc_flag = -1;
	int32		old_disc_flag = -1;
	int32		failure = 1;
	int32		success = 0;
	int32		write = 0;
	int32		*temp_p = NULL;	
	
	

	if(PIN_ERRBUF_IS_ERR(ebufp))
                PIN_ERRBUF_CLEAR(ebufp);
	
	plan_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_NAME, 1, ebufp);
	city = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CITY, 1, ebufp);
	das_type = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_DAS_TYPE, 1, ebufp);
	discount = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DISCOUNT, 1, ebufp);
	temp_p = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DISCOUNT_FLAGS, 1, ebufp);
	if(temp_p)
	{
		disc_flag = *temp_p;
	}
	temp_p = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_PP_TYPE, 1, ebufp);
	if(temp_p)
	{
		pp_type = *temp_p;
	}
	temp_p = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CONN_TYPE, 1, ebufp);
	if(temp_p)
        {
                conn_type = *temp_p;
        }
	end_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 1, ebufp);
	if(plan_name == NULL ||  city == NULL || das_type == NULL || pp_type == -1  || disc_flag == -1 || conn_type ==-1 || pbo_decimal_is_null(discount, ebufp))
        {
                *ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "55008", ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Mandatory fields missing:plan_name/city/disc_type/discount/das/pp_type are mandatory", ebufp);
                goto CLEANUP;
        }
	if(disc_flag == 0 && pbo_decimal_compare(discount, one, ebufp) >0)
        {
                PIN_ERR_LOG_MSG(3, "Percentage configured more than 100%");
                PIN_ERRBUF_RESET(ebufp);
                *ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "55003", ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "percentage discount amount cannot be more than 1", ebufp);
                goto CLEANUP;
        }
	if(disc_flag == 0 && pbo_decimal_sign(discount, ebufp) <= 0)
	{
		PIN_ERRBUF_RESET(ebufp);
                *ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "55012", ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Discount Amount or percentage cannot be less than or equal to Zero", ebufp);

                goto CLEANUP;
	}
	//Search plan details
	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(srch_iflistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &srch_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, plan_tmpl, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp,  PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_NAME, plan_name, ebufp);
	results_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS, 0, ebufp);
	
	PIN_ERR_LOG_FLIST(3, "Plan search in flistp:", srch_iflistp);
	
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "55009", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error during plan search", ebufp);
		goto CLEANUP;
	}
	plan_flistp = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	if(plan_flistp)
	{
		plan_pdp = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_POID, 0, ebufp);
	}
	else
	{
		PIN_ERRBUF_RESET(ebufp);
                *ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "55010", ebufp);
		sprintf(msg, "%s plan doesn not exist in the system", plan_name);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, msg, ebufp);
                goto CLEANUP;
	}
	PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	/*if(disc_flag == 1)
	{
		calc_base_disc(ctxp, discount, &base_disc, ebufp);
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_RESET(ebufp);
			*ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "55006", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error during base discount calculation", ebufp);
			goto CLEANUP;
		}

	}
	else
	{
		base_disc = pbo_decimal_copy(discount, ebufp);
	}*/
	base_disc = pbo_decimal_copy(discount, ebufp);
	//Searching for plan discount configuration 

	srch_iflistp = PIN_FLIST_CREATE(ebufp);
        srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
        PIN_FLIST_FLD_PUT(srch_iflistp, PIN_FLD_POID, srch_pdp, ebufp);
        PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &srch_flags, ebufp);
        PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, disc_tmpl, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp,  PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_NAME, plan_name, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp,  PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_CITY, city, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp,  PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_PP_TYPE, &pp_type , ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp,  PIN_FLD_ARGS, 4, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_DAS_TYPE, das_type , ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 5, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CONN_TYPE, &conn_type, ebufp);

        results_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_ERR_LOG_FLIST(3, "discount search in flistp:", srch_iflistp);

        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
        if(PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERRBUF_RESET(ebufp);
                *ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "55011", ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error during plan level disc search", ebufp);
                goto CLEANUP;
        }
	PIN_ERR_LOG_FLIST(3, "Discount configured ", srch_oflistp);
        disc_flistp = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, 0, 1, ebufp);
        if(disc_flistp)
        {
		disc_end_t =  PIN_FLIST_FLD_GET(disc_flistp, PIN_FLD_END_T, 0, ebufp);
                disc_amt = PIN_FLIST_FLD_GET(disc_flistp, PIN_FLD_DISCOUNT, 0, ebufp);
                old_disc_flag = *(int32 *) PIN_FLIST_FLD_GET(disc_flistp, PIN_FLD_DISCOUNT_FLAGS, 0, ebufp);
		sprintf(msg ," old disc flag %d new disc flag %d", old_disc_flag, disc_flag);
		PIN_ERR_LOG_MSG(3,msg);
                if((end_t && disc_end_t && (*end_t != *disc_end_t)) || old_disc_flag != disc_flag || pbo_decimal_compare(base_disc, disc_amt,ebufp) !=0)
                {
                        write = 1;
                }
                else
                {
                        PIN_ERRBUF_RESET(ebufp);
                        *ret_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
                        PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
                        PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "55006", ebufp);
                        PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "discount Already exist", ebufp);
                        goto CLEANUP;
                }
                if(write)
                {
                        wrt_iflistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_COPY(disc_flistp, PIN_FLD_POID, wrt_iflistp, PIN_FLD_POID, ebufp);
                        if(end_t)
                        PIN_FLIST_FLD_SET(wrt_iflistp, PIN_FLD_END_T, end_t, ebufp);
                        PIN_FLIST_FLD_SET(wrt_iflistp, PIN_FLD_DISCOUNT_FLAGS, &disc_flag, ebufp);
                        PIN_FLIST_FLD_SET(wrt_iflistp, PIN_FLD_DISCOUNT, base_disc, ebufp);
                        PIN_ERR_LOG_FLIST(3, "Discount update input flist", wrt_iflistp);
                        PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, wrt_iflistp, &wrt_oflistp, ebufp);
                        if(PIN_ERRBUF_IS_ERR(ebufp))
                        {
                                PIN_ERRBUF_RESET(ebufp);
                                *ret_flistp = PIN_FLIST_CREATE(ebufp);
                                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
                                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
                                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "55006", ebufp);
                                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error during updating service disc configuration", ebufp);
                                goto CLEANUP;
                        }
                        PIN_ERR_LOG_FLIST(3, "discount update output flist", wrt_oflistp);
                        *ret_flistp = PIN_FLIST_COPY(wrt_oflistp, ebufp);
                        PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &success, ebufp);
                        PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "0000", ebufp);
                        PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Discount configuration updated successfully", ebufp);
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(3, "Create a new discount entry");
		create_iflistp = PIN_FLIST_CREATE(ebufp);
		plan_disc_pdp = PIN_POID_CREATE(db, "/mso_cfg_disc_plan", -1, ebufp);
		PIN_FLIST_FLD_PUT(create_iflistp, PIN_FLD_POID, plan_disc_pdp, ebufp);
		PIN_FLIST_FLD_SET(create_iflistp, MSO_FLD_CITY, city, ebufp);
		PIN_FLIST_FLD_SET(create_iflistp, PIN_FLD_NAME, plan_name, ebufp);
		PIN_FLIST_FLD_SET(create_iflistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
		PIN_FLIST_FLD_SET(create_iflistp, PIN_FLD_CONN_TYPE, &conn_type, ebufp);
		PIN_FLIST_FLD_SET(create_iflistp, MSO_FLD_DAS_TYPE, das_type, ebufp);
		PIN_FLIST_FLD_SET(create_iflistp, PIN_FLD_DISCOUNT_FLAGS, &disc_flag, ebufp);
		PIN_FLIST_FLD_SET(create_iflistp, MSO_FLD_PP_TYPE, &pp_type, ebufp);
		PIN_FLIST_FLD_SET(create_iflistp, PIN_FLD_END_T, end_t, ebufp);
		PIN_FLIST_FLD_SET(create_iflistp, PIN_FLD_DISCOUNT, base_disc, ebufp);

		PIN_ERR_LOG_FLIST(3, "Discount Creation input flist", create_iflistp);
		PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, create_iflistp, &create_oflistp, ebufp);
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_RESET(ebufp);
			*ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "55006", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error during creating service disc configuration", ebufp);
			goto CLEANUP;
		}
		PIN_ERR_LOG_FLIST(3, "Discount configuration output", create_oflistp);
                *ret_flistp = PIN_FLIST_COPY(create_oflistp, ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &success, ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "0000", ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Discount configuration updated successfully", ebufp);


        }	


	CLEANUP:
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_NAME, plan_name, ebufp);
		PIN_FLIST_DESTROY_EX(&wrt_iflistp, NULL);
                PIN_FLIST_DESTROY_EX(&wrt_oflistp, NULL);
                PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
                PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
                PIN_FLIST_DESTROY_EX(&create_iflistp, NULL);
                PIN_FLIST_DESTROY_EX(&create_oflistp, NULL);
	        if(!pbo_decimal_is_null(one, ebufp))
                {
                        pbo_decimal_destroy(&one);
                }
		if(!pbo_decimal_is_null(base_disc, ebufp))
		{
			pbo_decimal_destroy(&base_disc);
		}
		return;
}
	
		

void fm_mso_validate_acct_device(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_errbuf_t 	*ebufp)
{
	pin_flist_t	*srch_iflistp = NULL;
	pin_flist_t	*srch_oflistp = NULL;
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*res_flistp = NULL;
	pin_flist_t	*results_flistp = NULL;
	pin_flist_t	*alias_list = NULL;
	
	poid_t		*srch_pdp = NULL;

	int32		srch_flags = 256;
	int32		closed = 10103;
	int32		db = 1;
	char 		*srch_tmpl = "select X from /service 1, /account 2 where 1.F1 = V1 and 1.F2 != V2 and 1.F3 = 2.F4 and 2.F5 = V5";
	char		*acct_no = NULL;
	char		*device_id = NULL;
	
	
	PIN_ERR_LOG_FLIST(3, "validate account & service ", i_flistp);
	acct_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
	device_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEVICE_ID, 0, ebufp);
	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(srch_iflistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &srch_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, srch_tmpl, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp);
        alias_list = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_ALIAS_LIST,  PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(alias_list, PIN_FLD_NAME, device_id, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &closed, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 4, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 5, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_NO, acct_no, ebufp);
        results_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(3, "Search input flist", srch_iflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_MSG(1, "Error during service search");
                goto CLEANUP;
        }
        res_flistp = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, 0, 0, ebufp);
        PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_POID, i_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
        PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_ACCOUNT_OBJ, i_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
        PIN_ERR_LOG_FLIST(3, "Enriched input flist", i_flistp);

        CLEANUP:

        PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
        PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
	return;
}

