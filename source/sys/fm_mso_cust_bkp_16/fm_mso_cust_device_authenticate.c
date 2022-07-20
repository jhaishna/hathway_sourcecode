/*******************************************************************
 * File:	fm_mso_cust_device_authenticate.c
 * Opcode:	MSO_OP_CUST_DEVICE_AUTHENTICATE 
 * Owner:	Sushmitha
 * Created:	08-AUG-2018
 * Description: Dummy Opcode

	r << xx 1
	0 PIN_FLD_POID         POID [0] 0.0.0.1 /service -1 0
	0 MSO_FLD_MAC_ID       STR [0] ""
	0 MSO_FLD_SERIAL_NO    STR [0] ""
	xx
	xop MSO_OP_CUST_DEVICE_AUTHENTICATE 0 1


	# number of field entries allocated 20, used 3
	0 PIN_FLD_POID         POID [0] 0.0.0.1 /service -1 0
	0 MSO_FLD_MAC_ID       STR [0] ""
	0 MSO_FLD_SERIAL_NO    STR [0] ""
	0 PIN_FLD_STATUS  	   INT [0] 1
	0 PIN_FLD_EXPIRATION_T TSTAMP [0] 1577836799

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_device_authenticate.c:ServerIDCVelocityInt:4:2006-Sep-14 11:34:45 %";
#endif

/*******************************************************************
 * Contains the MSO_OP_CUST_DEVICE_AUTHENTICATE operation.
 *******************************************************************/

#include <stdio.h>
#include <string.h>

#include <pcm.h>
#include <pinlog.h>

#define FILE_LOGNAME "fm_mso_cust_device_authenticate.c(1)"

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
op_mso_cust_device_authenticate(
    cm_nap_connection_t    *connp,
    int32            opcode,
    int32            flags,
    pin_flist_t        *i_flistp,
    pin_flist_t        **r_flistpp,
    pin_errbuf_t        *ebufp);

static void
fm_mso_cust_device_authenticate(
    pcm_context_t        *ctxp,
    int32            flags,
    pin_flist_t        *i_flistp,
    pin_flist_t        **r_flistpp,
    pin_errbuf_t        *ebufp);

void
fm_mso_get_eprofile_offer(
	pcm_context_t		*ctxp,
	poid_t			*ser_pdp,
	char			*ofr_desc,
	pin_flist_t		**ret_flist,
	pin_errbuf_t		*ebufp);

void
fm_mso_get_device_det_reg(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flist,
	pin_flist_t		**ret_flist,
	pin_errbuf_t		*ebufp);

void 
fm_mso_get_base_prod_active(
        pcm_context_t           *ctxp,
        poid_t                  *act_pdp,
	 poid_t                  *serv_pdp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);
		
void
fm_mso_get_acct_det(
	pcm_context_t		*ctxp,
	char			*act_no,
	pin_flist_t		**r_flistp,
	pin_errbuf_t		*ebufp);

int
fm_mso_chk_comm_lco(
	pcm_context_t		*ctxp,
	char			*lco_code,
	pin_errbuf_t		*ebufp);

int32
fm_mso_chk_device_excl(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flist,
	pin_errbuf_t		*ebufp);

extern void
fm_mso_get_subscr_purchased_products(
        pcm_context_t           *ctxp,
        poid_t                  *act_pdp,
        poid_t                  *srvc_pdp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);


/*******************************************************************
 * Main routine for the MSO_OP_CUST_CREATE_PROMO_OBJ operation.
 *******************************************************************/
void
op_mso_cust_device_authenticate(
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
    if (opcode != MSO_OP_CUST_DEVICE_AUTHENTICATE) {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
            PIN_ERR_BAD_OPCODE, 0, 0, opcode);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_mso_cust_device_authenticate opcode error", ebufp);

        return;
    }
    
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "op_mso_cust_device_authenticate input flist", i_flistp);

    /***********************************************************
     * Main routine for this opcode
     ***********************************************************/
    fm_mso_cust_device_authenticate(ctxp, flags, i_flistp, r_flistpp, ebufp);
    
    /***********************************************************
     * Error?
     ***********************************************************/
    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        PIN_FLIST_DESTROY_EX(r_flistpp, ebufp);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_mso_cust_device_authenticate error", ebufp);
    } else {
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "op_mso_cust_device_authenticate output flist", *r_flistpp);
    }

    return;
}

/*******************************************************************
 * fm_mso_cust_device_authenticate:
 *
 *******************************************************************/
static void
fm_mso_cust_device_authenticate(
    pcm_context_t           *ctxp,
    int32                   flags,
    pin_flist_t             *i_flistp,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp)
{
	pin_flist_t 	*rr_flistpp = NULL;
	pin_flist_t 	*in_flistpp = NULL;
	pin_flist_t 	*err_flistp = NULL;
	pin_flist_t 	*dev_flistp = NULL;
	pin_flist_t 	*res_flistp = NULL;
	pin_flist_t 	*sub_bal_flistp = NULL;
	poid_t 	*acnt_pdp = NULL;
	poid_t 	*srvc_pdp = NULL;
	poid_t 	*isp_srvc_pdp = NULL;
	poid_t 	*isp_act_pdp = NULL;
	char 	*aac_pkg;
	char 	*lco_code;
	poid_t 	*bal_grp_pdp = NULL;
	char 		*msg = "OTT";
        char                    msg1[1024];

	int32 		status = 1;
	int64         db = 0;
	int			flag;
	int                     lco_comm_flg;
    	int32           business_type = 0;
        void            *bus_type = NULL;
    	int32           arr_business_type[8];
        int32           comm_flag = -1;
	time_t		current_t = pin_virtual_time(NULL);
	int32		postpaid_end = 1999999999;
	int32		end_t = 1999999999;
	char		bb_service_type[]="/service/telco/broadband";
	pin_decimal_t     *zero_p = pbo_decimal_from_str("0.0",ebufp);
	pin_decimal_t     *base_amt;
	pin_decimal_t     *curr_bal;
	
    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }
		PIN_ERRBUF_CLEAR(ebufp);
	       PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Function Called ....");
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " input Flist is : ", i_flistp);

		//OUTPUT PREPARATION
		PIN_ERRBUF_RESET(ebufp);
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp), ebufp );
		PIN_FLIST_FLD_SET(*r_flistpp, MSO_FLD_MAC_ID, PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_MAC_ID, 1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, MSO_FLD_SERIAL_NO, PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SERIAL_NO, 1, ebufp), ebufp);
		//SEARCH DEVICE PRESENCE
		fm_mso_get_device_det_reg(ctxp, i_flistp, &dev_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp) || dev_flistp == (pin_flist_t *)NULL)
		{
			status=2;
			goto CLEANUP_ERROR;
		}
		else if(dev_flistp != (pin_flist_t *)NULL && PIN_FLIST_ELEM_GET(dev_flistp, PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp) ==NULL) 
		{
			status=3;
			goto CLEANUP_ERROR;
		}
		else
		{
			res_flistp = PIN_FLIST_ELEM_GET(dev_flistp, PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
			if (PIN_FLIST_ELEM_COUNT(res_flistp, PIN_FLD_SERVICES, ebufp) == 0)
			{
			  status=4;
			  goto CLEANUP_ERROR;
			}
			lco_code = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_SOURCE,1, ebufp);
			err_flistp =  PIN_FLIST_ELEM_GET(res_flistp, PIN_FLD_SERVICES,PIN_ELEMID_ANY,1,ebufp);
			acnt_pdp = PIN_FLIST_FLD_GET(err_flistp, PIN_FLD_ACCOUNT_OBJ,1, ebufp);
			srvc_pdp = PIN_FLIST_FLD_GET(err_flistp, PIN_FLD_SERVICE_OBJ,1, ebufp);
			//Reading Account Details to Know AAC_PACKAGE Status
			err_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID,acnt_pdp, ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_AAC_PACKAGE,NULL, ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_BAL_GRP_OBJ,NULL, ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_BUSINESS_TYPE,NULL, ebufp);
			PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, err_flistp, &dev_flistp, ebufp);	
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "READ FLD FOR ACCOUNT OUTPUT FLIST", dev_flistp);
			aac_pkg = PIN_FLIST_FLD_GET(dev_flistp, PIN_FLD_AAC_PACKAGE,1, ebufp);
			bal_grp_pdp = PIN_FLIST_FLD_GET(dev_flistp, PIN_FLD_BAL_GRP_OBJ,1, ebufp);
			bus_type = PIN_FLIST_FLD_GET(dev_flistp, PIN_FLD_BUSINESS_TYPE,1, ebufp);
                                        business_type = *(int32 *)bus_type;
                                        num_to_array(business_type, arr_business_type);
                                        comm_flag = arr_business_type[7];
                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "flag 1");
                                        sprintf(msg1,"%d: comm_flag", comm_flag);
                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg1);

			if(strlen(aac_pkg) >0)
			{
				fm_mso_get_acct_det(ctxp, aac_pkg, &dev_flistp, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp) || PIN_FLIST_ELEM_COUNT(dev_flistp, PIN_FLD_RESULTS, ebufp) == 0)
				{
					PIN_ERRBUF_RESET(ebufp);
					flag = 99;
				}
				else
				{	
					sub_bal_flistp = PIN_FLIST_ELEM_GET(dev_flistp, PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
					isp_act_pdp = PIN_FLIST_FLD_GET(sub_bal_flistp, PIN_FLD_POID, 1, ebufp);
					flag = GET_UPASS_ALL;
					fm_mso_get_service_reg_info(ctxp, isp_act_pdp, &dev_flistp,flag, ebufp);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_service_reg_info output Flist is : ", dev_flistp);
					if (PIN_ERRBUF_IS_ERR(ebufp) || PIN_FLIST_ELEM_GET(dev_flistp, PIN_FLD_SERVICES,PIN_ELEMID_ANY,1,ebufp) ==NULL)
					{
						PIN_ERRBUF_RESET(ebufp);
						flag = 99;
					}
					else
					{
						res_flistp =PIN_FLIST_ELEM_GET(dev_flistp, PIN_FLD_SERVICES,PIN_ELEMID_ANY,1,ebufp);
						isp_srvc_pdp =PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_POID, 1, ebufp);
						if(strncmp(PIN_POID_GET_TYPE(isp_srvc_pdp),bb_service_type,strlen(bb_service_type))!=0)
						{
							flag = 99;
						}
					}
				}	
			}
			else
			{
				flag = 99;
			}
			PIN_FLIST_DESTROY_EX(&err_flistp, ebufp);
			//Reading Service Details to Know Service Status
			err_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID,srvc_pdp, ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS,NULL, ebufp);
			PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, err_flistp, &dev_flistp, ebufp);	
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "READ FLD OUTPUT FLIST", dev_flistp);
			//Setting Final Status Based on Status of service
			if (*(int *)PIN_FLIST_FLD_GET(dev_flistp, PIN_FLD_STATUS,1, ebufp) == PIN_STATUS_ACTIVE)
			{
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status, ebufp);
			}
			else
			{
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &db, ebufp);
			}
		}
		// SEARCH ISREGISTERED FLAG
		fm_mso_get_eprofile_offer(ctxp, acnt_pdp, msg, &dev_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_eprofile_offer output flist", dev_flistp);
		if (PIN_ERRBUF_IS_ERR(ebufp) || dev_flistp == (pin_flist_t *)NULL || PIN_FLIST_ELEM_COUNT(dev_flistp, PIN_FLD_RESULTS, ebufp) == 0)
		{		
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error after calling fm_mso_get_eprofile_offer", ebufp);
				PIN_ERRBUF_RESET(ebufp);
			}
			PIN_FLIST_FLD_SET(*r_flistpp, MSO_FLD_ISREGISTERED, &db, ebufp);
		}
		else
		{
			res_flistp = PIN_FLIST_ELEM_GET(dev_flistp, PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, MSO_FLD_ISREGISTERED, PIN_FLIST_FLD_GET(res_flistp, MSO_FLD_ISREGISTERED, 1, ebufp), ebufp);
			db = *(int *)PIN_FLIST_FLD_GET(res_flistp, MSO_FLD_ISREGISTERED, 1, ebufp);
		}
		dev_flistp = (pin_flist_t *)NULL;
		res_flistp = (pin_flist_t *)NULL;
		if (strncmp(PIN_POID_GET_TYPE(srvc_pdp),bb_service_type,strlen(bb_service_type))==0)
		{
		    fm_mso_get_subscr_purchased_products(ctxp, acnt_pdp, srvc_pdp, &dev_flistp, ebufp);
		}
		else if (strncmp(PIN_POID_GET_TYPE(srvc_pdp),bb_service_type,strlen(bb_service_type))!=0 && flag !=99)
		{
			fm_mso_get_subscr_purchased_products(ctxp, isp_act_pdp, isp_srvc_pdp, &dev_flistp, ebufp);
		}
		else
		{
		    fm_mso_get_base_prod_active(ctxp, acnt_pdp, srvc_pdp, &dev_flistp, ebufp);
		}
		if (PIN_ERRBUF_IS_ERR(ebufp) || dev_flistp == (pin_flist_t *)NULL || PIN_FLIST_ELEM_COUNT(dev_flistp, PIN_FLD_RESULTS, ebufp) == 0)
		{		
			status=5;
			goto CLEANUP_ERROR;
		}
		else
       	{
			res_flistp = PIN_FLIST_ELEM_GET(dev_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
			if (strncmp(PIN_POID_GET_TYPE(srvc_pdp),bb_service_type,strlen(bb_service_type))!=0 && flag ==99)
			{	
				//CALLING PLAN PRICE CHECK FUNCTION
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling price Calc for plan");
				fm_mso_get_plan_price(ctxp, PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp),NULL, &rr_flistpp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "OUTPUT FLIST fm_mso_get_plan_price", rr_flistpp);
				if (PIN_ERRBUF_IS_ERR(ebufp) || rr_flistpp == NULL)
				{
					PIN_ERRBUF_RESET(ebufp);
				}
				else
				{
					base_amt=PIN_FLIST_FLD_GET(rr_flistpp, PIN_FLD_AMOUNT, 1, ebufp);
					in_flistpp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(in_flistpp, PIN_FLD_BAL_GRP_OBJ,bal_grp_pdp, ebufp);
					PIN_FLIST_FLD_SET(in_flistpp, PIN_FLD_POID,acnt_pdp, ebufp);
					sub_bal_flistp = PIN_FLIST_ELEM_ADD(in_flistpp,PIN_FLD_BALANCES,PIN_ELEMID_ANY,ebufp);
					PIN_FLIST_FLD_SET(sub_bal_flistp,PIN_FLD_CURRENT_BAL,(void *)zero_p,ebufp);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "CR balances input flist",in_flistpp);
					PCM_OP(ctxp,PCM_OP_BAL_GET_BALANCES,0,in_flistpp,&rr_flistpp,ebufp);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "CR balances output flist",rr_flistpp);
					dev_flistp = PIN_FLIST_ELEM_GET(rr_flistpp, PIN_FLD_BALANCES, 356, 1, ebufp);
					curr_bal = PIN_FLIST_FLD_GET(dev_flistp, PIN_FLD_CURRENT_BAL, 1, ebufp);
					if (pbo_decimal_compare(pbo_decimal_abs(curr_bal,ebufp),pbo_decimal_add(base_amt,base_amt,ebufp),ebufp) > 0 && pbo_decimal_compare(curr_bal,zero_p,ebufp) <0)
					{
						flag=*(int *)PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_CYCLE_END_T, 1, ebufp);
						flag=flag+2246400;
						PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_CYCLE_END_T, &flag, ebufp);
					}	
					pbo_decimal_destroy(&zero_p);
					PIN_FLIST_DESTROY_EX(&in_flistpp, ebufp);				
				}	
			}

			//Check for device auth exclusion list f
			end_t  = fm_mso_chk_device_excl(ctxp,i_flistp,ebufp);
			current_t = fm_utils_time_round_to_midnight(current_t);
			if(end_t && end_t >= current_t && *(time_t *)PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_CYCLE_END_T, 1, ebufp) >= current_t ) 
			{
				db=1;
				end_t = end_t + 19800;
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_EXPIRATION_T, &end_t, ebufp);
				PIN_FLIST_FLD_SET(*r_flistpp, MSO_FLD_ISREGISTERED, &db, ebufp);
			}
			else
			{
			//CHECK LCO CODE FOR COMMERCIAL LIST
			lco_comm_flg = fm_mso_chk_comm_lco(ctxp,lco_code,ebufp);
			current_t = fm_utils_time_round_to_midnight(current_t);
			if (*(time_t *)PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_CYCLE_END_T, 1, ebufp) >= current_t)
			{
				flag = *(int *)PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_CYCLE_END_T, 1, ebufp);
				if(lco_comm_flg){
					flag = flag + 1209600 + 19800; //2246400; 15 days + 5:30 for IST
					PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_EXPIRATION_T, &flag, ebufp);
				}
				else {
                                	flag = flag + 19800;
					PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_EXPIRATION_T, &flag, ebufp);
				}
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "flag 1");
					sprintf(msg1,"%d: comm_flag", comm_flag);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg1);
					if (comm_flag == 1 && (flag - current_t <= 864000))
					{
						current_t = current_t + 864000;
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "flag 2");
						PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_EXPIRATION_T, &current_t, ebufp);
					}
			}
			else if(*(int *)PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_CYCLE_END_T, 1, ebufp) == 0)
			{
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_EXPIRATION_T,&postpaid_end,ebufp);
			}
			else 
			{
				status=5;
				goto CLEANUP_ERROR;
			}
			}
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "OUTPUT FLIST AFTER SAUTHENTICATE DEVICE", *r_flistpp);

CLEANUP_ERROR : if(status >1)
		  {
			PIN_ERRBUF_RESET(ebufp);
			err_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID,PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp),ebufp);
			PIN_FLIST_FLD_SET(err_flistp, MSO_FLD_MAC_ID, PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_MAC_ID, 1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(err_flistp, MSO_FLD_SERIAL_NO, PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SERIAL_NO, 1, ebufp), ebufp);
			if(status == 2)
			{
				PIN_FLIST_DESTROY_EX(&dev_flistp, ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE,"55002",ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,"Device Search Failed.", ebufp);
			}
			if(status == 3)
			{
				PIN_FLIST_DESTROY_EX(&dev_flistp, ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE,"55003",ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,"Invalid Device Id or Serial No Check the Device.", ebufp);
			}
			if(status == 4)
			{
				PIN_FLIST_DESTROY_EX(&dev_flistp, ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE,"55004",ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,"Device is not Associated to a service.", ebufp);
			}
			if(status == 5)
			{
				PIN_FLIST_DESTROY_EX(&dev_flistp, ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE,"55005",ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,"No Hathway Subscription Found for ISP/CATV.", ebufp);
			}	
			status = 0;
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
			PIN_FLIST_FLD_SET(err_flistp, MSO_FLD_ISREGISTERED, &db, ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_EXPIRATION_T, &status, ebufp);
			msg = "FAILURE";
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_DESCR, msg, ebufp);
			*r_flistpp = PIN_FLIST_COPY(err_flistp,ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "OUTPUT FLIST AFTER AUTHENTICATE DEVICE", *r_flistpp);
			PIN_FLIST_DESTROY_EX(&err_flistp, ebufp);
		  }
						
    return;
}

/**************************************************
* Function: 	fm_mso_get_device_det_reg()
* Decription:   Returns the device details 
***************************************************/
void
fm_mso_get_device_det_reg(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flist,
	pin_flist_t		**ret_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_flist_t		*result_flist = NULL;

	poid_t			*srch_pdp = NULL;
	int32			srch_flag = 512;
	int64			db = 1;

	char			*template = "select X from /device where F1 = V1" ;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_device_det_reg", ebufp);
		return;
	}
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	if(PIN_FLIST_FLD_GET(i_flist, MSO_FLD_SERIAL_NO, 1, ebufp) !=NULL)
	{
		arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
		PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_SERIAL_NO, PIN_FLIST_FLD_GET(i_flist, MSO_FLD_SERIAL_NO, 1, ebufp), ebufp);
	}
	else
	{
		arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
		PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_DEVICE_ID, PIN_FLIST_FLD_GET(i_flist, MSO_FLD_MAC_ID, 1, ebufp), ebufp);
	}	
	result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_device_det_reg search profile input flist", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_device_det_reg output flist", srch_out_flistp);	

	*ret_flist = PIN_FLIST_COPY(srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);

	return;
}

/**************************************************
* Function  : fm_mso_get_base_prod_active()
* Decription: function to fetch base active purchased_product's
*             for service and account
*
***************************************************/
void 
fm_mso_get_base_prod_active(
        pcm_context_t           *ctxp,
        poid_t                  *act_pdp,
		poid_t                  *serv_pdp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *s_flistp = NULL;
        pin_flist_t             *args_flistp = NULL;
        pin_flist_t             *res_flistp = NULL;
        poid_t                  *s_pdp = NULL;
        int32                   flags = 256;
        int64                   db = 0;
        int64                   id = -1;
        char                    *s_template = "select X from /purchased_product 1,/product 2 where 1.F1 = V1 and 1.F2 = V2 and 1.F3 = V3 and 1.F4 = 2.F5 and 2.F6 > V6 and 2.F7 < V7 ";
        int32                   act_status = 1;
		pin_flist_t		*ret_flistp = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);
	
        db = PIN_POID_GET_DB(act_pdp);
        s_flistp = PIN_FLIST_CREATE(ebufp);
        s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, act_pdp, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &act_status, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, serv_pdp, ebufp);
		
		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
		
		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);
		
		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 6, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRIORITY, pbo_decimal_from_str("100",ebufp), ebufp);
		
		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 7, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRIORITY, pbo_decimal_from_str("130",ebufp), ebufp);

        res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_base_prod_active", s_flistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &ret_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
        {
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_base_prod_active - Error in calling SEARCH", ebufp);
			PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
			return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_base_prod_active search output flist", ret_flistp);
		*ret_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
}

/**************************************************
* Function: 	fm_mso_get_acct_det()
***************************************************/
void
fm_mso_get_acct_det(
	pcm_context_t		*ctxp,
	char			*act_no,
	pin_flist_t		**r_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_flist_t		*result_flist = NULL;
	poid_t			*srch_pdp = NULL;
	int32			srch_flag = 512;
	int			db = 1;
	char			*template = "select X from /account  where F1 = V1 ";

	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACCOUNT_NO, act_no, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp );
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_acct_det search input flist", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_acct_det output flist", srch_out_flistp);
	*r_flistp = PIN_FLIST_COPY(srch_out_flistp,ebufp);
}

/**************************************************
* Function: 	fm_mso_chk_comm_lco()
***************************************************/
int
fm_mso_chk_comm_lco(
	pcm_context_t		*ctxp,
	char			*lco_code,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_flist_t		*result_flist = NULL;
	int32			srch_flag = 512;
	int			db = 1;
	char			*template = "select X from /mso_commercial_lco where F1 = V1 ";

	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_SOURCE, lco_code, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp );
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_chk_comm_lco search input flist", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_chk_comm_lco output flist", srch_out_flistp);

	if (PIN_ERRBUF_IS_ERR(ebufp) || PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp) == 0)
	{
		PIN_ERRBUF_CLEAR(ebufp);
		return 0;
	}
	else
	{
		return 1;
	}

	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
}

int32
fm_mso_chk_device_excl(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_flist_t		*result_flist = NULL;
	int32			srch_flag = 512;
	int32		end_t = 0;
	int			db = 1;
	char			*template = "select X from /mso_device_auth_excl_list where F1 = V1 ";

	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);


	if(PIN_FLIST_FLD_GET(i_flist, MSO_FLD_SERIAL_NO, 1, ebufp) !=NULL)
	{
		arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
		PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_SERIAL_NO, PIN_FLIST_FLD_GET(i_flist, MSO_FLD_SERIAL_NO, 1, ebufp), ebufp);
	}
	else
	{
		arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
		PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_DEVICE_ID, PIN_FLIST_FLD_GET(i_flist, MSO_FLD_MAC_ID, 1, ebufp), ebufp);
	}


	result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_EXPIRATION_T, NULL, ebufp );
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_chk_device_excl search input flist", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_chk_device_excl search output flist", srch_out_flistp);

	if (PIN_ERRBUF_IS_ERR(ebufp) || PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp) == 0)
	{
		PIN_ERRBUF_CLEAR(ebufp);
		return 0;
	}
	else
	{
		result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
		end_t = *(int32 *)PIN_FLIST_FLD_GET(result_flist, PIN_FLD_EXPIRATION_T, 1, ebufp);
		return end_t;
	}

	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
}


