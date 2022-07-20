/*******************************************************************
 * File:	fm_mso_cust_device_register.c
 * Opcode:	MSO_OP_CUST_DEVICE_REGISTER 
 * Owner:	Suresh Nadar
 * Created:	05-OCT-2018
 * Description: opcode to Set Is Registered Flag in profile_cust_offer

r << xx 1
0 PIN_FLD_POID         POID [0] 0.0.0.1 /service -1 0
0 MSO_FLD_MAC_ID       STR [0] "CDAF123G"
0 MSO_FLD_RMN          STR [0] "9090909087"
0 MSO_FLD_RMAIL          STR [0] "hath@ymail.com"
0 MSO_FLD_SERIAL_NO       STR [0] "OTT1243R5"
0 PIN_FLD_ACCOUNT_NO      STR [0] "1123465734"
xx
xop MSO_OP_CUST_DEVICE_REGISTER 0 1


	# number of field entries allocated 20, used 3
	0 PIN_FLD_POID         POID [0] 0.0.0.1 /service -1 0
	0 MSO_FLD_MAC_ID       STR [0] ""
	0 MSO_FLD_RMN          STR [0] ""
	0 PIN_FLD_DESCR    STR [0] "SUCCESS"
	0 PIN_FLD_STATUS  	   INT [0] 1

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_device_register.c:ServerIDCVelocityInt:4:2018-Oct-07 11:34:45 %";
#endif

/*******************************************************************
 * Contains the MSO_OP_CUST_DEVICE_REGISTER operation.
 *******************************************************************/

#include <stdio.h>
#include <string.h>
#include <pinlog.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "pin_cust.h"
#include "ops/act.h"
#include "ops/device.h"
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
op_mso_cust_device_register(
    cm_nap_connection_t    *connp,
    int32            opcode,
    int32            flags,
    pin_flist_t        *i_flistp,
    pin_flist_t        **r_flistpp,
    pin_errbuf_t        *ebufp);

static void
fm_mso_cust_device_register(
    pcm_context_t        *ctxp,
    int32            flags,
    pin_flist_t        *i_flistp,
    pin_flist_t        **r_flistpp,
    pin_errbuf_t        *ebufp);

void
fm_mso_get_service_reg_info(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flistp,
	int32			flag,
	pin_errbuf_t		*ebufp);

static int
fm_mso_call_modify_cust(
	pcm_context_t		*ctxp,
	char			*act_no,
	pin_flist_t		*in_flistp,
	pin_flist_t		**r_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_get_device_det_reg(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flist,
	pin_flist_t		**ret_flist,
	pin_errbuf_t		*ebufp);

void
fm_mso_get_device_sun_coupon(
	pcm_context_t		*ctxp,
	pin_flist_t		**ret_flist,
	pin_errbuf_t		*ebufp);

void
fm_mso_get_device_reg_act(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flist,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * Main routine for the MSO_OP_CUST_DEVICE_REGISTER operation.
 *******************************************************************/
void
op_mso_cust_device_register(
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
    if (opcode != MSO_OP_CUST_DEVICE_REGISTER)
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
            PIN_ERR_BAD_OPCODE, 0, 0, opcode);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_mso_cust_device_register opcode error", ebufp);

        return;
    }
    
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "op_mso_cust_device_register input flist", i_flistp);

    /***********************************************************
     * Main routine for this opcode
     ***********************************************************/
    fm_mso_cust_device_register(ctxp, flags, i_flistp, r_flistpp, ebufp);
    
    /***********************************************************
     * Error?
     ***********************************************************/
    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        PIN_FLIST_DESTROY_EX(r_flistpp, ebufp);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_mso_cust_device_register error", ebufp);
    } else {
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "op_mso_cust_device_register output flist", *r_flistpp);
    }

    return;
}

/*******************************************************************
 * fm_mso_cust_device_register:
 *
 *******************************************************************/
static void
fm_mso_cust_device_register(
    pcm_context_t           *ctxp,
    int32                   flags,
    pin_flist_t             *i_flistp,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp)
{
	char 			*msg;
	int32 			status = 1;	
       pin_flist_t          *err_flistp = NULL;
       pin_flist_t          *srvc_info = NULL;
	poid_t			*acnt_pdp = NULL;
	char 			*act_no = "";
    	pin_flist_t          *s_flistp = NULL;
    	pin_flist_t          *ar_flistp = NULL;
    	pin_flist_t          *al_flistp = NULL;
    	pin_flist_t          *device_flistp = NULL;
	pin_flist_t 		*device_out_flistp =NULL;
       pin_flist_t          *sun_coupon_flistp = NULL;
	int			elem_id=0;
	pin_cookie_t 		cookie= NULL;
	int			telem_id=0; 
	pin_cookie_t 		tcookie= NULL;
	int			flag;
	int			asc_flag;
	int32		    *coup_flag = NULL;
    	char		    *manufacturer = NULL;
	int32		    err = 0;
	
    if (PIN_ERRBUF_IS_ERR(ebufp)) 
    {
        return;
    }
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_device_register input Flist is : ", i_flistp);

		if(PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SERIAL_NO, 1, ebufp) ==NULL && PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_MAC_ID, 1, ebufp) ==NULL)
		{
			status = 2;
			goto CLEANUP_ERROR;
		}
		else if(PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp) == NULL)
		{
			status = 2;
			goto CLEANUP_ERROR;
		}
		//CALLING MODIFY AFTER ASSOCIATION TO AVOID ISSUES OF PARTIAL TRANSACTION POSSIBILITIES
		act_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp);
		if (PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_ISREGISTERED, 1, ebufp) != NULL && *(int *)PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_ISREGISTERED, 1, ebufp) == 2)
		{
		  sun_coupon_flistp = PIN_FLIST_COPY(i_flistp,ebufp);
		  status = 1;
		  PIN_FLIST_FLD_SET(sun_coupon_flistp, MSO_FLD_ISREGISTERED, &status, ebufp);
		  status = fm_mso_call_modify_cust(ctxp,act_no,sun_coupon_flistp,&s_flistp,ebufp);
		}
		else
		{
		   status = fm_mso_call_modify_cust(ctxp,act_no,i_flistp,&s_flistp,ebufp);
		}	
		if (status !=0)
		{
			goto CLEANUP_ERROR;
		}
		else
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_call_modify_cust output Flist is : ", s_flistp);
			flag = GET_UPASS_ALL;
			acnt_pdp = PIN_FLIST_FLD_GET(s_flistp, PIN_FLD_POID, 1, ebufp);
			fm_mso_get_service_reg_info(ctxp, acnt_pdp, &srvc_info,flag, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_service_reg_info output Flist is : ", srvc_info);
			if (PIN_ERRBUF_IS_ERR(ebufp) || srvc_info == (pin_flist_t *)NULL)
			{
				PIN_ERRBUF_RESET(ebufp);
				status = 7;
				goto CLEANUP_ERROR;
			}
			if (srvc_info && srvc_info !=NULL && PIN_FLIST_ELEM_GET(srvc_info, PIN_FLD_SERVICES,PIN_ELEMID_ANY,1,ebufp) !=NULL)
			{
				ar_flistp = PIN_FLIST_ELEM_GET(srvc_info, PIN_FLD_SERVICES,PIN_ELEMID_ANY,1,ebufp);
			}
			else
			{
				status = 6;
				goto CLEANUP_ERROR;
			}
		}

		//CALLING ASSOCIATION DEASSOCIATION BASED ON FALG
		if (PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_ISREGISTERED, 1, ebufp) == NULL || *(int *)PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_ISREGISTERED, 1, ebufp) == 1)	
		{
			//NORMAL ASSOCIATION CALL
			s_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(s_flistp, MSO_FLD_MAC_ID, PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_MAC_ID, 1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(s_flistp, MSO_FLD_SERIAL_NO, PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SERIAL_NO, 1, ebufp), ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_device_det_reg input Flist is : ", s_flistp);
			fm_mso_get_device_det_reg(ctxp, s_flistp, &al_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_device_det_reg output Flist is : ", al_flistp);
			if (PIN_ERRBUF_IS_ERR(ebufp) || al_flistp == (pin_flist_t *)NULL)
			{
				PIN_ERRBUF_RESET(ebufp);
				status = 8;
				goto CLEANUP_ERROR;
			}
			else if (PIN_FLIST_ELEM_COUNT(al_flistp, PIN_FLD_RESULTS, ebufp) == 0)
			{
				status = 8;
				goto CLEANUP_ERROR;
			}
			srvc_info = PIN_FLIST_ELEM_GET(al_flistp, PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
			asc_flag = MSO_DEVICE_ACTIVATION_NEW;
			manufacturer = PIN_FLIST_FLD_GET(srvc_info, PIN_FLD_MANUFACTURER, 1, ebufp);
			
			if ((*(int *)PIN_FLIST_FLD_GET(srvc_info,PIN_FLD_STATE_ID,1,ebufp) == 1||*(int *)PIN_FLIST_FLD_GET(srvc_info,PIN_FLD_STATE_ID,1,ebufp) == 9) && strstr(manufacturer,"VM")== NULL )
			{
			//CHECK FOR NOT ALLOWING MULTIPLE OTT IN 1 ACCOUNT
			fm_mso_get_device_reg_act(ctxp, acnt_pdp, &sun_coupon_flistp, ebufp);
			PIN_ERRBUF_RESET(ebufp);
			
			if (sun_coupon_flistp !=NULL && PIN_FLIST_ELEM_COUNT(sun_coupon_flistp, PIN_FLD_RESULTS, ebufp) > 0)
			{
				status = 12;
				goto CLEANUP_ERROR;
			}

				device_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(device_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(srvc_info, PIN_FLD_POID, 1, ebufp), ebufp);
				msg = "OTT_ASSOCIATION";
				PIN_FLIST_FLD_SET(device_flistp, PIN_FLD_PROGRAM_NAME, msg, ebufp);
				PIN_FLIST_FLD_SET(device_flistp, PIN_FLD_FLAGS, &asc_flag, ebufp);
				al_flistp = PIN_FLIST_ELEM_ADD(device_flistp, PIN_FLD_SERVICES, 0,ebufp);
				PIN_FLIST_FLD_SET(al_flistp, PIN_FLD_ACCOUNT_OBJ, (void *) PIN_FLIST_FLD_GET(ar_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp), ebufp);
				PIN_FLIST_FLD_SET(al_flistp, PIN_FLD_SERVICE_OBJ, (void *) PIN_FLIST_FLD_GET(ar_flistp, PIN_FLD_POID, 1, ebufp), ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Device Association input Flist is : ", device_flistp);
				PCM_OP(ctxp, PCM_OP_DEVICE_ASSOCIATE, 0, device_flistp, &device_out_flistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Device Association input Flist is : ", device_out_flistp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
				     PIN_ERRBUF_RESET(ebufp);
			     	     status = 9;
			     	     goto CLEANUP_ERROR;
				}
				PIN_FLIST_DESTROY_EX(&s_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&device_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&device_out_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&sun_coupon_flistp, ebufp);
			}
			else if (strstr(manufacturer,"VM"))
			{
				if(*(int *)PIN_FLIST_FLD_GET(srvc_info,PIN_FLD_STATE_ID,1,ebufp) == 1)
				{
					status = 15;
					goto CLEANUP_ERROR;
				}
				device_flistp = PIN_FLIST_ELEM_GET(srvc_info, PIN_FLD_SERVICES,PIN_ELEMID_ANY,1,ebufp);
				if(device_flistp && device_flistp !=NULL)
				{
					if(PIN_POID_GET_ID(acnt_pdp)==PIN_POID_GET_ID(PIN_FLIST_FLD_GET(device_flistp,PIN_FLD_ACCOUNT_OBJ,1,ebufp)))
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Account matched for VM Boxes");
					}
					else
					{
						status = 16;
						goto CLEANUP_ERROR;
					}	
				}
				else
				{
					status = 17;
					goto CLEANUP_ERROR;
				}	
			}
			else
			{
				     status = 13;
			     	     goto CLEANUP_ERROR;
			}
		}
		//CALLING DEASSOCIATION BASED ON FALG
		else if (*(int *)PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_ISREGISTERED, 1, ebufp) == 0)	
		{
			s_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(s_flistp, MSO_FLD_MAC_ID, PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_MAC_ID, 1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(s_flistp, MSO_FLD_SERIAL_NO, PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SERIAL_NO, 1, ebufp), ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_device_det_reg input Flist is : ", s_flistp);
			fm_mso_get_device_det_reg(ctxp, s_flistp, &al_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_device_det_reg input Flist is : ", al_flistp);
			if (PIN_ERRBUF_IS_ERR(ebufp) || al_flistp == (pin_flist_t *)NULL)
			{
				PIN_ERRBUF_RESET(ebufp);
				status = 8;
				goto CLEANUP_ERROR;
			}
			else if (PIN_FLIST_ELEM_COUNT(al_flistp, PIN_FLD_RESULTS, ebufp) == 0)
			{
				status = 8;
				goto CLEANUP_ERROR;
			}
			srvc_info = PIN_FLIST_ELEM_GET(al_flistp, PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
			asc_flag = MSO_DEVICE_DEACTIVATION_REASON_SERVICE_CLOSURE;
			manufacturer = PIN_FLIST_FLD_GET(srvc_info, PIN_FLD_MANUFACTURER, 1, ebufp);

			if (*(int *)PIN_FLIST_FLD_GET(srvc_info,PIN_FLD_STATE_ID,1,ebufp) == 2 && strstr(manufacturer,"VM")==NULL)
			{
				device_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(device_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(srvc_info, PIN_FLD_POID, 1, ebufp), ebufp);
				msg = "OTT_DISASSOCIATION";
				PIN_FLIST_FLD_SET(device_flistp, PIN_FLD_PROGRAM_NAME, msg, ebufp);
				PIN_FLIST_FLD_SET(device_flistp, PIN_FLD_FLAGS, &asc_flag, ebufp);
				al_flistp = PIN_FLIST_ELEM_ADD(device_flistp, PIN_FLD_SERVICES, 0,ebufp);
				PIN_FLIST_FLD_SET(al_flistp, PIN_FLD_ACCOUNT_OBJ, (void *) PIN_FLIST_FLD_GET(ar_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp), ebufp);
				PIN_FLIST_FLD_SET(al_flistp, PIN_FLD_SERVICE_OBJ, (void *) PIN_FLIST_FLD_GET(ar_flistp, PIN_FLD_POID, 1, ebufp), ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Device DisAssociation input Flist is : ", device_flistp);
				PCM_OP(ctxp, PCM_OP_DEVICE_ASSOCIATE, 0, device_flistp, &device_out_flistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Device DisAssociation input Flist is : ", device_out_flistp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
				     PIN_ERRBUF_RESET(ebufp);
			     	     status = 9;
			     	     goto CLEANUP_ERROR;
				}
				PIN_FLIST_DESTROY_EX(&s_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&device_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&device_out_flistp, ebufp);
			}
		}
		//CALLING SUN COUPON ASSOCIATION BASED ON FALG
		else if(*(int *)PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_ISREGISTERED, 1, ebufp) == 2)
		{
			pin_conf("fm_mso_cust", "enable_coupon", PIN_FLDT_INT, (caddr_t*)&coup_flag, &err );
			if (coup_flag && *coup_flag == 1)
			{
				//COUPON ASSOCIATION
				fm_mso_get_device_sun_coupon(ctxp, &sun_coupon_flistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Sun Coupon Search output flist : ", sun_coupon_flistp);
				if (PIN_ERRBUF_IS_ERR(ebufp) || PIN_FLIST_ELEM_COUNT(sun_coupon_flistp, PIN_FLD_RESULTS, ebufp) == 0)
				{
					PIN_ERRBUF_RESET(ebufp);
					status = 10;
					goto CLEANUP_ERROR;
				}

				device_flistp = PIN_FLIST_CREATE(ebufp);
				al_flistp = PIN_FLIST_ELEM_GET(sun_coupon_flistp, PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
				PIN_FLIST_FLD_SET(device_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(al_flistp, PIN_FLD_POID, 1, ebufp), ebufp);
				msg = "COUPON_ASSOCIATION";
				PIN_FLIST_FLD_SET(device_flistp, PIN_FLD_PROGRAM_NAME, msg, ebufp);
				asc_flag = MSO_DEVICE_ACTIVATION_NEW;
				PIN_FLIST_FLD_SET(device_flistp, PIN_FLD_FLAGS, &asc_flag, ebufp);
				al_flistp = PIN_FLIST_ELEM_ADD(device_flistp, PIN_FLD_SERVICES, 0,ebufp);
				PIN_FLIST_FLD_SET(al_flistp, PIN_FLD_ACCOUNT_OBJ, (void *) PIN_FLIST_FLD_GET(ar_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp), ebufp);
				PIN_FLIST_FLD_SET(al_flistp, PIN_FLD_SERVICE_OBJ, (void *) PIN_FLIST_FLD_GET(ar_flistp, PIN_FLD_POID, 1, ebufp), ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Device Association input Flist is : ", device_flistp);
				PCM_OP(ctxp, PCM_OP_DEVICE_ASSOCIATE, 0, device_flistp, &device_out_flistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Device Association input Flist is : ", device_out_flistp);

				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERRBUF_RESET(ebufp);
						status = 11;
						goto CLEANUP_ERROR;
				}
			}
			else
			{
					PIN_ERRBUF_RESET(ebufp);
					status = 14;
					goto CLEANUP_ERROR;				
			}
		}	

		// PREPARING RETURN FLIST
		PIN_ERRBUF_RESET(ebufp);
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp), ebufp );
		PIN_FLIST_FLD_SET(*r_flistpp, MSO_FLD_MAC_ID, PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_MAC_ID, 1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, MSO_FLD_RMN, PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_RMN, 1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, MSO_FLD_RMAIL, PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_RMAIL, 1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ACCOUNT_NO, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, MSO_FLD_SERIAL_NO, PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SERIAL_NO, 1, ebufp), ebufp);
		status = 1;
		msg = "SUCCESS";
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_DESCR, msg, ebufp);		

CLEANUP_ERROR : if(status >1)
		  {
			PIN_ERRBUF_RESET(ebufp);
			err_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID,PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp),ebufp);
			PIN_FLIST_FLD_SET(err_flistp, MSO_FLD_MAC_ID, PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_MAC_ID, 1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(err_flistp, MSO_FLD_SERIAL_NO, PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SERIAL_NO, 1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(err_flistp, MSO_FLD_RMN, PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_RMN, 1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(err_flistp, MSO_FLD_RMAIL, PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_RMAIL, 1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ACCOUNT_NO, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp), ebufp);
			if(status == 2)
			{
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE,"54993",ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,"Could not process Request Mandatory Field Missing.", ebufp);
			}
			if(status == 3)
			{
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE,"54994",ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,"Error while Searching Account No.", ebufp);
			}
			if(status == 4)
			{
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE,"54995",ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,"Account Doesnot exist in BRM.", ebufp);
			}
			if(status == 5)
			{
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE,"54996",ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,"Profile Creation Error in Modify Customer.", ebufp);
			}
			if(status == 6)
			{
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE,"54997",ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,"No Valid Service Associated with the Account.", ebufp);
			}
			if(status == 7)
			{
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE,"54998",ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,"Error in Searching Service for Account.", ebufp);
			}
			if(status == 8)
			{
				PIN_FLIST_DESTROY_EX(&s_flistp, ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE,"54999",ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,"Error in Searching Device, Device does not Exists.", ebufp);
			}
			if(status == 9)
			{
				PIN_FLIST_DESTROY_EX(&s_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&device_flistp, ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE,"55000",ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,"Error in Associating Device with service.", ebufp);
			}
			if(status == 10)
			{
				PIN_FLIST_DESTROY_EX(&s_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&device_flistp, ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE,"55001",ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,"Error in Searching Coupon / No Free Sun Coupon exists in System.", ebufp);
			}
			if(status == 11)
			{
				PIN_FLIST_DESTROY_EX(&s_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&device_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&sun_coupon_flistp, ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE,"55002",ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,"Error in Associating Sun Coupon with service,try with another coupon", ebufp);
			}
			if(status == 12)
			{
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE,"55003",ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,"Sorry Multiple OTT Box on same Account is not allowed.", ebufp);
			}
			if(status == 13)
			{
				PIN_FLIST_DESTROY_EX(&s_flistp, ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE,"55004",ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,"Device State is Not Ready to use.", ebufp);
			}
			if(status == 14)
			{
				PIN_FLIST_DESTROY_EX(&s_flistp, ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE,"55005",ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,"Configuration to set SUN COUPON Associstion is OFF.", ebufp);
			}
			if(status == 15)
			{
				PIN_FLIST_DESTROY_EX(&s_flistp, ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE,"55006",ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,"Hybrid box is free.. first Activate it an then move for Registraion", ebufp);
			}
			if(status == 16)
			{
				PIN_FLIST_DESTROY_EX(&s_flistp, ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE,"55007",ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,"Hybrid box is not associated with the given account.Check Account No.", ebufp);
			}
			if(status == 17)
			{
				PIN_FLIST_DESTROY_EX(&s_flistp, ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE,"55008",ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,"Hybrid box is not associated with any account", ebufp);
			}
			status = 0;
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
			msg = "FAILURE";
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_DESCR, msg, ebufp);
			*r_flistpp = PIN_FLIST_COPY(err_flistp,ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "OUTPUT FLIST AFTER DEVICE REGISTER", *r_flistpp);
			PIN_FLIST_DESTROY_EX(&err_flistp, ebufp);
		  }
	return;
}

/**************************************************
* Function: 	fm_mso_call_modify_cust()
* Decription:   Call for Modify Customer to register in profile_cust_offer 
***************************************************/
static int
fm_mso_call_modify_cust(
	pcm_context_t		*ctxp,
	char			*act_no,
	pin_flist_t		*in_flistp,
	pin_flist_t		**r_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*modf_flistp = NULL;
	pin_flist_t		*modf_oflistp = NULL;

	poid_t			*srch_pdp = NULL;
	int32			srch_flag = 512;
	int			db = 1;
	int			telem_id=0; 
	pin_cookie_t 		tcookie= NULL;
	char			*prg_name="OTT_REGISTRATION";
	char			*descr="OTT";
	poid_t			*AMS_POID = PIN_POID_FROM_STR("0.0.0.1 /account 6464305206 0", NULL, ebufp);
	int32		    	*coup_flag = NULL;
	int32		    	err = 0;

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
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_call_modify_cust search input flist", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_call_modify_cust output flist", srch_out_flistp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		return 3;
	}

	if (PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp) > 0)
	{
		while((arg_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_out_flistp, PIN_FLD_RESULTS,&telem_id, 1, &tcookie, ebufp)) != (pin_flist_t *)NULL)
		{
			modf_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Fields from Search Account No", arg_flistp);
			PIN_FLIST_FLD_SET(modf_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(arg_flistp, PIN_FLD_POID, 1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(modf_flistp, PIN_FLD_USERID, AMS_POID, ebufp);
			PIN_FLIST_FLD_SET(modf_flistp, PIN_FLD_ACCOUNT_OBJ, PIN_FLIST_FLD_GET(arg_flistp, PIN_FLD_POID, 1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(modf_flistp, PIN_FLD_PROGRAM_NAME, prg_name, ebufp);
			result_flist = PIN_FLIST_ELEM_ADD(modf_flistp, MSO_FLD_REGISTER_CUSTOMER, telem_id, ebufp );
			if(PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_ISREGISTERED, 1, ebufp) == NULL || *(int *)PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_ISREGISTERED, 1, ebufp) == 1)
			{
				PIN_FLIST_FLD_SET(result_flist, MSO_FLD_ISREGISTERED, &db, ebufp);
				PIN_FLIST_FLD_SET(result_flist, MSO_FLD_OFFER_VALUE, &db, ebufp);
			}
			else
			{
				db = 0;
				PIN_FLIST_FLD_SET(result_flist, MSO_FLD_ISREGISTERED, &db, ebufp);
				PIN_FLIST_FLD_SET(result_flist, MSO_FLD_OFFER_VALUE, &db, ebufp);
			}
			PIN_FLIST_FLD_SET(result_flist, MSO_FLD_OFFER_DESCR, descr, ebufp);
			PIN_FLIST_FLD_SET(result_flist, MSO_FLD_RMN, PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_RMN, 1, ebufp), ebufp );
			PIN_FLIST_FLD_SET(result_flist, MSO_FLD_RMAIL, PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_RMAIL, 1, ebufp), ebufp );
			//Changes to Add Netflix profile
			pin_conf("fm_mso_cust", "enable_netflix_tag", PIN_FLDT_INT, (caddr_t*)&coup_flag, &err );
			if (coup_flag && *coup_flag == 1 && (PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_ISREGISTERED, 1, ebufp) == NULL || *(int *)PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_ISREGISTERED, 1, ebufp) == 1))
			{
			db = 1;
			result_flist = PIN_FLIST_ELEM_ADD(modf_flistp, MSO_FLD_REGISTER_CUSTOMER, db, ebufp);
			descr="Netflix";
			PIN_FLIST_FLD_SET(result_flist, MSO_FLD_OFFER_DESCR, descr, ebufp);
			PIN_FLIST_FLD_SET(result_flist, MSO_FLD_RMN, PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_RMN, 1, ebufp), ebufp );
			PIN_FLIST_FLD_SET(result_flist, MSO_FLD_RMAIL, PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_RMAIL, 1, ebufp), ebufp );
			PIN_FLIST_FLD_SET(result_flist, MSO_FLD_OFFER_VALUE,&db, ebufp);
			//Changes to Add Netflix profile
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_MODIFY_CUSTOMER input flist for profile_cust_offer creation", modf_flistp);
			PCM_OP(ctxp, MSO_OP_CUST_MODIFY_CUSTOMER, 0, modf_flistp, &modf_oflistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_MODIFY_CUSTOMER output flist for profile_cust_offer creation", modf_oflistp);
			if (PIN_ERRBUF_IS_ERR(ebufp) || PIN_FLIST_FLD_GET(modf_oflistp, PIN_FLD_STATUS, 1, ebufp) == 0 || PIN_FLIST_FLD_GET(modf_oflistp, PIN_FLD_STATUS, 1, ebufp) == NULL)
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in MSO_OP_CUST_MODIFY_CUSTOMER", ebufp);
				return 5;
			}
		PIN_FLIST_DESTROY_EX(&modf_flistp, NULL);
		}
	}
	else
	{
     		return 4;
	}	
	*r_flistp = PIN_FLIST_COPY(modf_oflistp,ebufp);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&modf_oflistp, NULL);
	if (!PIN_POID_IS_NULL(AMS_POID))
	{
		PIN_POID_DESTROY(AMS_POID,ebufp);
	}	

	return 0;
}

/**************************************************
* Function: 	fm_mso_get_service_reg_info()
* Owner:        Suresh Nadar
* Decription:   For getting service information
***************************************************/
void
fm_mso_get_service_reg_info(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flistp,
	int32			flag,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_oflistp = NULL;


	poid_t			*srch_pdp = NULL;
	poid_t			*plan_obj = NULL;

	int32			srch_flag = 512;
	int32			srvc_status_active = PIN_STATUS_ACTIVE;
	int32			srvc_status_inactive = PIN_STATUS_INACTIVE;
	int32			srvc_status_cancelled = PIN_STATUS_CLOSED;


	int64			db = -1;
	//Netflix Changes
	char			*template = "select X from /service where F1.id = V1 and (F2.type = V2 or F3.type = V3 or F4.type = V4) and F5 = V5 " ;
	char			*template_1 = "select X from /service where F1.id = V1 and (F2.type = V2 or F3.type = V3 or F4.type = V4) " ;


	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_service_info", ebufp);
		return;
	}
	db = PIN_POID_GET_DB(acnt_pdp) ;
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/service/catv", -1, ebufp), ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
	PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/service/telco/broadband", -1, ebufp), ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp );
	PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/service/settlement/ws/outcollect", -1, ebufp), ebufp);

	if (flag == GET_ALL)
	{
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template_1 , ebufp);
	}
	else if (flag == GET_UPASS_ALL)
	{
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 5, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STATUS, &srvc_status_active, ebufp);
	}
	else if (flag == GET_INACTIVE_SERVICE)
	{
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 5, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STATUS, &srvc_status_inactive, ebufp);
	}
	else if (flag == GET_CANCELLED_SERVICE)
	{
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 5, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STATUS, &srvc_status_cancelled, ebufp);
	}
	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
	/*PIN_FLIST_FLD_SET(result_flist, PIN_FLD_BAL_GRP_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_CREATED_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_MOD_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_CLOSE_WHEN_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_EFFECTIVE_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_LOGIN, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STATUS, NULL, ebufp);
	PIN_FLIST_SUBSTR_SET(result_flist, NULL, MSO_FLD_CATV_INFO, ebufp);
	PIN_FLIST_ELEM_SET(result_flist,   NULL, MSO_FLD_PERSONAL_BIT_INFO, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_ELEM_SET(result_flist,   NULL, PIN_FLD_ALIAS_LIST, PIN_ELEMID_ANY, ebufp);*/

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_service_info read input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_service_info error", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_service_info READ output list", srch_oflistp);
	if (PIN_FLIST_ELEM_COUNT(srch_oflistp, PIN_FLD_RESULTS, ebufp) == 0)
	{
		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
		return;
	}
	PIN_FLIST_FLD_RENAME(srch_oflistp, PIN_FLD_RESULTS, PIN_FLD_SERVICES, ebufp);
	
	*ret_flistp = PIN_FLIST_COPY(srch_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);

	CLEANUP:
	return;
}

/**************************************************
* Function: 	fm_mso_get_device_sun_coupon()
* Decription:   Returns the active device details 
***************************************************/
void
fm_mso_get_device_sun_coupon(
	pcm_context_t		*ctxp,
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

	char			*template = "select X from /device where device_t.poid_type = '/device/router/wifi' and F1 = V1 and F2 = V2 and ROWNUM = 1 " ;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_device_sun_coupon", ebufp);
		return;
	}
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_DEVICE_TYPE, SUN_COUPON, ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 2, ebufp );
	db = MSO_MODEM_STATE_GOOD;
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_STATE_ID, &db, ebufp);
	result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_device_sun_coupon search profile input flist", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_device_sun_coupon output flist", srch_out_flistp);	

	*ret_flist = PIN_FLIST_COPY(srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);

	return;
}

/**************************************************
* Function: 	fm_mso_get_device_reg_act()
* Decription:   Returns the active device details 
***************************************************/
void
fm_mso_get_device_reg_act(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_flist_t		*svc_flistp = NULL;
	pin_flist_t		*result_flist = NULL;

	poid_t			*srch_pdp = NULL;
	int32			srch_flag = 512;
	int64			db = 1;

	char			*template = "select X from /device where device_t.poid_type = '/device/modem' and F1 = V1 and F2 = V2 and F3 = V3 " ;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_device_reg_act", ebufp);
		return;
	}
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_DEVICE_TYPE, OTT_MODEM, ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 2, ebufp );
	db = MSO_STB_STATE_ALLOCATED;
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_STATE_ID, &db, ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 3, ebufp );
	svc_flistp = PIN_FLIST_ELEM_ADD(arg_flistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(svc_flistp, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);
	result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_device_reg_act search profile input flist", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_device_reg_act output flist", srch_out_flistp);	

	*ret_flist = PIN_FLIST_COPY(srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);

	return;
}

