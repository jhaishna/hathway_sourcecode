/*******************************************************************
 *
 * Copyright (c) 1999, 2009, Oracle and/or its affiliates. 
 * All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

/***************************************************************************************************************
*VERSION |   DATE    |    Author        |               DESCRIPTION                                            *
*--------------------------------------------------------------------------------------------------------------*
* 0.1    |20/12/2013 |Satya Prakash     |  CATV SMS Notification implementation 
*--------------------------------------------------------------------------------------------------------------*
* 0.2    |13/04/2014 | Vilva Sabarikanth|  CATV SMS Notification based on flags enablement from CM's pin.conf  *
*--------------------------------------------------------------------------------------------------------------*
* 0.3    |31/07/2014 | Someshwar        | Added BB notifications                                               *
*--------------------------------------------------------------------------------------------------------------*
*--------------------------------------------------------------------------------------------------------------*
* 0.4    |09/03/2017 | Tony             |  Added Usage type field to /mso_ntf_sms and functionality to send day's
*                                       |  first copy of each type to configured business users
*--------------------------------------------------------------------------------------------------------------*
* 0.5    |13/04/2020 | Jyothirmayi K    |  Added functionality to send SMS for Additional GB Topup
*--------------------------------------------------------------------------------------------------------------*
* 0.6    |20/04/2020 | Jyothirmayi K    |  Added functionality to include the Service Provider ID for 
					   Hathway or Den in the mso_ntf_sms_t for Service Provider 
					   identification
*--------------------------------------------------------------------------------------------------------------*
****************************************************************************************************************/

#ifndef lint
static  char Sccs_Id[] = "@(#)$Id: fm_mso_ntf_create_sms_notification.c /cgbubrm_7.3.2.rwsmod/1 2009/08/04 23:37:19 pjegan Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/timeb.h>
#include "pin_sys.h"
#include "pin_os.h"
#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "mso_ntf.h"
#include "mso_prov.h"

#include "fm_mso_ntf_common_routines.c"

#define CHANNEL "SMS"
#define HFIBER_BUSINESS_TYPE 97681200
#define IPTV_BUSINESS_TYPE 9948

extern void 
fm_mso_utils_sequence_no(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);


/*******************************************************************
 * Fuctions defined in this code 
 *******************************************************************/

EXPORT_OP void
op_mso_ntf_create_sms_notification(
	cm_nap_connection_t		*connp,
	int32				opcode,
	int32				flags,
	pin_flist_t			*in_flistp,
	pin_flist_t			**ret_flistpp,
	pin_errbuf_t			*ebufp);

static void
fm_mso_ntf_create_sms_notification(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	int32			flags,
	pin_flist_t		**out_flistpp,
	pin_errbuf_t		*ebufp);

static void
get_sms_template(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	char			**sms_template,
	char			*ne,
	pin_errbuf_t		*ebufp);

static void
get_bb_sms_template(
	pcm_context_t		*ctxp, 
	pin_flist_t		*i_flistp, 
	char			**sms_template, 
	pin_errbuf_t		*ebufp);

extern void
get_osd_template(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	char			**osd_template,
	char				*ne,
	pin_errbuf_t		*ebufp);

extern void
get_bmail_template(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	char			**bmail_template,
	char			*ne,
	pin_errbuf_t		*ebufp);

extern void
get_primary_connection_details(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

extern void
fm_mso_send_osd_request(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	char			*str,
	char			*ne,
	pin_errbuf_t		*ebufp);

extern void
fm_mso_send_bmail_request(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	char			*str,
	char			*ne,
	pin_errbuf_t		*ebufp);

static void
prepare_service_activation_notification_buf(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

static void
prepare_service_suspension_notification_buf(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

static void
prepare_service_reactivation_notification_buf(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

static void
prepare_service_termination_notification_buf(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

static void
prepare_service_swap_notification_buf(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **ret_flistpp,
	pin_errbuf_t		*ebufp);

static void
prepare_add_plan_notification_buf(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

static void
prepare_change_plan_notification_buf(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

static void
prepare_cancel_plan_notification_buf(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

static void
prepare_billing_notification_buf(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

static void
prepare_due_reminder_notification_buf(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

static void
prepare_payment_notification_buf(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

static void
prepare_payment_reversal_notification_buf(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

static void
prepare_cheque_reversal_notification_buf(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

static void
prepare_deposit_refund_notification_buf(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

static void
mso_prov_get_provisioning_tag_details(
	char			*prov_tag,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

static void
prepare_change_passwd_notification_buf(
	pcm_context_t			*ctxp, 
	pin_flist_t			*i_flistp, 
	pin_flist_t			**payload_flistp, 
	pin_errbuf_t			*ebufp);

static void
prepare_before_fup_notification_buf(
	pcm_context_t			*ctxp, 
	pin_flist_t			*i_flistp, 
	pin_flist_t			**payload_flistp, 
	pin_errbuf_t			*ebufp);

static void
prepare_after_fup_notification_buf(
	pcm_context_t			*ctxp, 
	pin_flist_t			*i_flistp, 
	pin_flist_t			**payload_flistp, 
	pin_errbuf_t			*ebufp);

static void
prepare_before_expiry_notification_buf(
	pcm_context_t			*ctxp, 
	pin_flist_t			*i_flistp, 
	pin_flist_t			**payload_flistp, 
	pin_errbuf_t			*ebufp);

static void
prepare_prepaid_lim_usage_notification_buf(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **ret_flistpp,
    pin_errbuf_t            *ebufp);

static void
prepare_upgrade_plan_notification_buf(
        pcm_context_t                   *ctxp,
        pin_flist_t                     *i_flistp,
        pin_flist_t                     **payload_flistp,
        pin_errbuf_t                    *ebufp);

static void
add_package_detail(
	pcm_context_t		*ctxp,
	pin_flist_t		**i_flistpp,
	int32			flag,
	pin_errbuf_t		*ebufp);

PIN_IMPORT void
get_helpdesk_details(
        pcm_context_t              	*ctxp,
        pin_flist_t                     *i_flistp,
        char                            *city,
        pin_flist_t                     **ret_flistpp,
        pin_errbuf_t                    *ebufp);

void
fm_mso_get_dl_speed(
        pcm_context_t   *ctxp,
        pin_flist_t     *in_flistp,
        pin_flist_t     **out_flistp,
        pin_errbuf_t    *ebufp);
		
/*****************************************************
 * Added this function to faciliate sending SMS 
 * Notification for Additonal GB purchased via Topup 
 * from Quick Pay
 *****************************************************/
static void
prepare_extra_gb_topup_notification_buf(
	pcm_context_t   *ctxp,
	pin_flist_t     *i_flistp,
	pin_flist_t     **ret_flistpp,
	pin_errbuf_t    *ebufp);

PIN_IMPORT int32	sms_config[20];

/*******************************************************************
 * Main routine for the MSO_OP_CREATE_CATV_ACTION command
 *******************************************************************/
void
op_mso_ntf_create_sms_notification(
	cm_nap_connection_t		*connp,
	int32				opcode,
	int32				flags,
	pin_flist_t			*in_flistp,
	pin_flist_t			**ret_flistpp,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*r_flistp = NULL;
	pcm_context_t			*ctxp = connp->dm_ctx;


	/***********************************************************
	* Null out results until we have some.
	***********************************************************/
	*ret_flistpp = NULL;
	PIN_ERRBUF_CLEAR(ebufp);

	/***********************************************************
	* Insanity check.
	***********************************************************/
	if (opcode != MSO_OP_NTF_CREATE_SMS_NOTIFICATION) 
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_mso_ntf_create_sms_notification", ebufp);
		return;
	}

	/***********************************************************
	* Log input flist
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_mso_ntf_create_sms_notification input flist", in_flistp);

	/***********************************************************
	* Call main function to do it
	***********************************************************/
	fm_mso_ntf_create_sms_notification(ctxp, in_flistp, flags, &r_flistp, ebufp);

	/***********************************************************
	* Results.
	***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		*ret_flistpp = (pin_flist_t *)NULL;
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_ntf_create_sms_notification error", ebufp);
		PIN_ERRBUF_CLEAR(ebufp);
	} 
	else
	{
		if(r_flistp)
		{
			*ret_flistpp = r_flistp;
			PIN_ERRBUF_CLEAR(ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"op_mso_ntf_create_sms_notification return flist", r_flistp);
		}
		else
		{
			*ret_flistpp = PIN_FLIST_COPY(in_flistp, ebufp);
			PIN_ERRBUF_CLEAR(ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"op_mso_ntf_create_sms_notification return flist", *ret_flistpp);
		}
	}

	return;
}

/*******************************************************************
 * fm_mso_ntf_create_sms_notification()
 *
 * Perfomrs following action:
 * 1. call action specific function to enrich the input from service
 *        and package detail
 * 2. get notification sequence 
 * 3. create notificaiton order /mso_ntf_sms
 *******************************************************************/
static void
fm_mso_ntf_create_sms_notification(
    pcm_context_t       *ctxp,
    pin_flist_t         *in_flistp,
    int32               flags,
    pin_flist_t         **out_flistpp,
    pin_errbuf_t        *ebufp)
{
    int32               *action_flag = NULL;
    int64               db_no;
    poid_t              *service_poidp = NULL;
    poid_t              *acc_poidp = NULL;
    poid_t              *objp = NULL;
    pin_flist_t         *payload_flistp = NULL;
    pin_flist_t         *order_i_flistp     = NULL;
    pin_flist_t         *tmp_flistp= NULL;
    pin_flist_t         *order_seq_i_flistp = NULL;
    pin_flist_t         *order_seq_o_flistp = NULL;;
    pin_flist_t         *bus_users_flistp = NULL;
    pin_flist_t         *bus_users_res_flistp = NULL;
    pin_flist_t         *so_flistp = NULL;
    pin_flist_t         *si_flistp = NULL;
    pin_buf_t           *nota_buf     = NULL;
    char                *flistbuf = NULL,
                        *usage_type = NULL,
                        *channel = NULL;
    int                 flistlen=0;
    int                 status = NTF_STATE_NEW;
    char                *act = NULL;
    char                *phone = NULL;
    char                *sms_template = NULL;
    char		*acc_nop = NULL;
    char		*et_zonep = NULL;
    int32               priority;
    int32		notif_disabled = 0;
    int32		op_status = 0; //SUCCESS
    int32               send_to_users = 0;
    int32               elemid = 0;
    int32               business_type = 0;
	int32					*carrier_id = 0;

    pin_cookie_t        cookie = NULL;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_ntf_create_sms_notification input flist", in_flistp);

    /***********************************************************
     * Check mandatory fields are available
     ***********************************************************/
    phone = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PHONE, 0, ebufp);
    action_flag = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_FLAGS, 0, ebufp);
	
    
    if (action_flag && 
        !(*action_flag == NTF_DUE_REMINDER  || 
           *action_flag == NTF_REMINDER_POST_DUE_DATE || 
           *action_flag == NTF_BILLING ) )
    {
       service_poidp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
    }

    acc_poidp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);

   /* Changes to stop notification for H-Fiber customers. No SMS to be send for H-Fiber customers */ 
    si_flistp = PIN_FLIST_CREATE(ebufp);

    PIN_FLIST_FLD_SET(si_flistp, PIN_FLD_POID, acc_poidp, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "Read account type from /acccount input flist", si_flistp);
    PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "Read account type from /acccount output flist", so_flistp);

    if(so_flistp && so_flistp != NULL)
	{
	business_type = *(int *) PIN_FLIST_FLD_GET(so_flistp,PIN_FLD_BUSINESS_TYPE,0,ebufp);
        PIN_FLIST_FLD_COPY(so_flistp, PIN_FLD_BUSINESS_TYPE, in_flistp, PIN_FLD_BUSINESS_TYPE, ebufp);
		if ( business_type == HFIBER_BUSINESS_TYPE )
		return;

	}

	business_type = business_type/10000;
	acc_nop = PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
	et_zonep = PIN_FLIST_FLD_GET(so_flistp, MSO_FLD_ET_ZONE, 0, ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_ACCOUNT_NO, acc_nop, ebufp);
/*  End of change for H-fiber filtering*/	

    if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "fm_mso_ntf_create_sms_notification error: required field missing in input flist", ebufp);
        return;
    }

    /***********************************************************
     * Enrich With SMS Text
     ***********************************************************/
    if ( *action_flag ==  NTF_SERVICE_ACTIVATION )
    {
		//Not required for BB
		service_poidp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		//if(PIN_POID_IS_NULL(service_poidp) || !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
		if(service_poidp &&  !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
		{
			if ((sms_config[6] == 1 || sms_config[6] == 3) && et_zonep && strstr(et_zonep, "_PP_"))
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
				prepare_service_activation_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
				priority = 1;
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Disabled");
				notif_disabled = 1;
				goto Cleanup;
			}
		}
    }
    else if ( *action_flag ==  NTF_SERVICE_SUSPENSION)
    {
		//Not required for BB
		service_poidp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		//if(PIN_POID_IS_NULL(service_poidp) || !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
		if(service_poidp &&  !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
		{
			if (sms_config[7] == 1 || sms_config[7] == 3)
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
				prepare_service_suspension_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
				priority = 1;
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Disabled");
				notif_disabled = 1;
				goto Cleanup;
			}
		}
    }
    else if ( *action_flag ==  NTF_SERVICE_REACTIVATION )
    {
		//Not required for BB
		service_poidp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		//if(PIN_POID_IS_NULL(service_poidp) || !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
		if(service_poidp &&  !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
		{
			//Sent if CATV and BOTH option selected
			if (sms_config[8] == 1 || sms_config[8] == 3)
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
				prepare_service_reactivation_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
				priority = 1;
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Disabled");
				notif_disabled = 1;
				goto Cleanup;
			}
		}
    }
    else if ( *action_flag ==  NTF_SERVICE_TERMINATON )
    {
		service_poidp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		//For CATV
		if (sms_config[9] == 1 && business_type == IPTV_BUSINESS_TYPE)
		{
			//if(PIN_POID_IS_NULL(service_poidp) || !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
			if(service_poidp &&  !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
				prepare_service_termination_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
				priority = 1;
			}
		}
		//For BB
		else if(sms_config[9] == 2)
		{
			if(service_poidp && !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/telco/broadband"))
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
				prepare_service_termination_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
				priority = 1;
			}
		}
		//For both CATV and BB
		else if(sms_config[9] == 3 && business_type == IPTV_BUSINESS_TYPE)
		{
			payload_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
			prepare_service_termination_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
			priority = 1;
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Disabled");
				notif_disabled = 1;
			goto Cleanup;
		}
    }
    else if (*action_flag ==  NTF_SERVICE_SWAP)
    {
		service_poidp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		//For CATV
		if (business_type == IPTV_BUSINESS_TYPE && (sms_config[15] == 1 || sms_config[15] == 3))
		{
			if(service_poidp &&  !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
				prepare_service_swap_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
				priority = 1;
			}
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Disabled");
				notif_disabled = 1;
			goto Cleanup;
		}
    }        
    else if ( *action_flag ==  NTF_ADD_PLAN )
    {
		//Not required for BB
		service_poidp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		//if(PIN_POID_IS_NULL(service_poidp) || !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
		if(service_poidp &&  !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
		{
			if (sms_config[10] == 1 || sms_config[10] == 3)
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
				prepare_add_plan_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
				priority = 1;
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Disabled");
				notif_disabled = 1;
				goto Cleanup;
			}
		}
    }
    else if ( *action_flag ==  NTF_CHANGE_PLAN )
    {
		//For CATV only
		service_poidp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		if (sms_config[11] == 1)
		{
			//if(PIN_POID_IS_NULL(service_poidp) || !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
			if(service_poidp &&  !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
				prepare_change_plan_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
				priority = 1;
			}
		}
		//For BB only
		else if (sms_config[11] == 2)
		{
			if(service_poidp && !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/telco/broadband"))
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
				prepare_change_plan_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
				priority = 1;
			}
		}
		//For both CATV and BB
		else if (sms_config[11] == 3)
		{
			payload_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
			prepare_change_plan_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
			priority = 1;
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Disabled");
				notif_disabled = 1;
			goto Cleanup;
		}
	}
	else if ( *action_flag ==  NTF_CANCEL_PLAN )
	{
		//Not required for BB
		service_poidp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		//if(PIN_POID_IS_NULL(service_poidp) || !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
		if(service_poidp &&  !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
		{
			//Flag enabled for CATV or Both
			if (sms_config[12] == 1 || sms_config[12] == 3)
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
				prepare_cancel_plan_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
				priority = 1;
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Disabled");
				notif_disabled = 1;
				goto Cleanup;
			}
		}
	}
	else if (*action_flag ==  NTF_REMINDER_POST_DUE_DATE )
	{
		service_poidp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		//For CATV only
		if (sms_config[13] == 1)
		{
			//if(PIN_POID_IS_NULL(service_poidp) || !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
			if(service_poidp &&  !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
				prepare_due_reminder_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
				priority = 1;
			}
		}
		//For BB only
		else if (sms_config[13] == 2)
		{
			if(service_poidp && !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/telco/broadband"))
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
				prepare_due_reminder_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
				priority = 1;
			}
		}
		//For both CATV and BB
		else if (sms_config[13] == 3)
		{
			payload_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
			prepare_due_reminder_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
			priority = 1;
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Disabled");
				notif_disabled = 1;
			goto Cleanup;
		}
	}
	else if ( *action_flag ==  NTF_BILLING )
	{
		service_poidp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		// Notification for CATV only
		if (sms_config[1] == 1)
		{
			//if(PIN_POID_IS_NULL(service_poidp) || !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
			if(service_poidp &&  !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
				prepare_billing_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
				priority = 1;
			}
		}
		// Notification for BB only
		else if (sms_config[1] == 2)
		{
			if(service_poidp && !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/telco/broadband"))
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
				prepare_billing_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
				priority = 1;
			}
		}
		//Both CATV and BB customers
		else if (sms_config[1] == 3)
		{
			payload_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
			prepare_billing_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
			priority = 1;
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Disabled");
			notif_disabled = 1;
			goto Cleanup;
		}
	}
	else if ( *action_flag ==  NTF_DUE_REMINDER )
	{
		service_poidp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		// Notification for CATV only
		if (sms_config[2] == 1)
		{
			//if(PIN_POID_IS_NULL(service_poidp) || !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
			if(service_poidp &&  !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
				prepare_due_reminder_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
				priority = 1;
			}
		}
		//For BB only
		else if (sms_config[2] == 2)
		{
			if(service_poidp && !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/telco/broadband"))
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
				prepare_due_reminder_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
				priority = 1;
			}
		}
		//For Both CATV and BB
		else if (sms_config[2] == 3)
		{
			payload_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
			prepare_due_reminder_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
			priority = 1;
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Disabled");
				notif_disabled = 1;
			goto Cleanup;
		}
	}
	else if ( *action_flag ==  NTF_PAYMENT || *action_flag ==  NTF_BB_PAYMENT_CASH || *action_flag ==  NTF_BB_PAYMENT_CHEQUE ||*action_flag == NTF_BB_PAYMENT_ONLINE )
	{
		carrier_id = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_CARRIER_ID, 0, ebufp); // Added for DEN SMS --carrier_id 1 for DEN and 0 for HW

		if (( *carrier_id == 1 &&  sms_config[15] != 2 && sms_config[15] != 3 )  || (*carrier_id == 0 &&  sms_config[15] != 1 && sms_config[15] != 3))
			// 0 for NO SMS ... 1 for hathway, 2 for Den and 3 for both
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Disabled For Den/Hathway");
				notif_disabled = 1;
			goto Cleanup;
		}

		// The action flags are checked for all the three payment templates. but the config flag is checked only for payment as it is catv specific and 
		// we have enough complications as it is already. Will handle it inside the function only for Broadband.
		service_poidp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
		//For CATV
		if (sms_config[3] == 1)
		{
			//if(PIN_POID_IS_NULL(service_poidp) || !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
			if(service_poidp &&  !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
				prepare_payment_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
				priority = 1;
			}
		}
		//For BB only
		else if (sms_config[3] == 2)
		{
			if(service_poidp && !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/telco/broadband"))
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
				prepare_payment_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
				priority = 1;
			}
		}
		//For both CATV and BB
		else if (sms_config[3] == 3 )
		{
			payload_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
			prepare_payment_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
			priority = 1;
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Disabled");
				notif_disabled = 1;
			goto Cleanup;
		}
	}
	else if ( *action_flag ==  NTF_PAYMENT_REVERSAL )
	{
		//For CATV only
		if (sms_config[4] == 1)
		{
			//if(PIN_POID_IS_NULL(service_poidp) || !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
			if(service_poidp &&  !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
				prepare_payment_reversal_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
				priority = 1;
			}
		}
		//For BB only
		else if (sms_config[4] == 2)
		{
			if(service_poidp && !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/telco/broadband"))
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
				prepare_payment_reversal_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
				priority = 1;
			}
		}
		//For both CATV and BB
		else if (sms_config[4] == 3)
		{
			payload_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
			prepare_payment_reversal_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
			priority = 1;
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Disabled");
				notif_disabled = 1;
			goto Cleanup;
		}
	}
	else if ( *action_flag ==  NTF_DEPOSIT_REFUND )
	{
		//Not required for BB
		service_poidp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		//if(PIN_POID_IS_NULL(service_poidp) || !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
		if(service_poidp &&  !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
		{
			if (sms_config[5] == 1 || sms_config[5] == 3)
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
				prepare_deposit_refund_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
				priority = 1;
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Disabled");
				notif_disabled = 1;
				goto Cleanup;
			}
		}
	}
	else if ( *action_flag ==  NTF_CHEQUE_REVERSAL )
	{
		if (sms_config[4] == 1 && service_poidp &&  !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv") )
		{
			payload_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
			prepare_cheque_reversal_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
			priority = 1;
		}
		//For BB only
		else if (sms_config[4] == 2 && service_poidp && !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/telco/broadband"))
		{
			payload_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
			prepare_cheque_reversal_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
			priority = 1;
		}
		else if (sms_config[4] == 3)
		{
			payload_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
			prepare_cheque_reversal_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
			priority = 1;
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Disabled");
				notif_disabled = 1;
			goto Cleanup;
		}
	}
	else if ( *action_flag ==  NTF_BB_CHANGE_PASSWORD )
	{
		payload_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
		prepare_change_passwd_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
		priority = 1;
	}
	else if ( *action_flag == NTF_BEFORE_FUP)
	{
		payload_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
		prepare_before_fup_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
		priority = 1;
	}
	else if ( *action_flag == NTF_AFTER_FUP)
	{
		payload_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
		prepare_after_fup_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
		priority = 1;
	}
	else if ( *action_flag == NTF_BEFORE_EXPIRY)
	{
		payload_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
		prepare_before_expiry_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
		priority = 1;
	} 
	else if (*action_flag == NTF_BB_ON_EXPIRY)
	{
		if (sms_config[7] == 2 || sms_config[7] == 3)
		{
			if(service_poidp && !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/telco/broadband"))
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
				prepare_service_suspension_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
				priority = 1;
			}
		}
	} 
	else if (*action_flag == NTF_BB_PREPAID_LIM_USAGE)
	{
		payload_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
		prepare_prepaid_lim_usage_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
		priority = 1;
	}
	else if (*action_flag == NTF_BB_UPGRADE_PLAN)
	{
		payload_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
		prepare_upgrade_plan_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
		priority = 1;
	}
	// SMS Notification for Provision of Additional GB on Quickpay
	else if (*action_flag == NTF_EXTRA_GB_TOPUP)
	{
		payload_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled for EXTRA GB TOPUP");
		prepare_extra_gb_topup_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
	}
	else
	{
		//set error
		pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"Invalid Action code", ebufp);
		return;
    }

    /*
     * Return if validation and enrichment returns error
     */
    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        PIN_FLIST_DESTROY_EX(&payload_flistp, NULL);
        return;
    }

    /*
     * Get Order Id and set it in payload
     */
    if(payload_flistp)
	{
	    order_seq_i_flistp = PIN_FLIST_CREATE(ebufp);
	    PIN_FLIST_FLD_SET(order_seq_i_flistp, PIN_FLD_POID, acc_poidp, ebufp);
	    PIN_FLIST_FLD_SET(order_seq_i_flistp, PIN_FLD_NAME, "MSO_NTF_SEQUENCE", ebufp);
	    //call the function to read the provisioning order sequence
	    fm_mso_utils_sequence_no(ctxp, order_seq_i_flistp, &order_seq_o_flistp, ebufp);

	    /*
	     * Convert flist to String Payload. it will be used by Adaptor process to send SMS
	     */
	     PIN_FLIST_TO_XML( payload_flistp, PIN_XML_BY_SHORT_NAME, 0,
		    &flistbuf, &flistlen, "SMS", ebufp );
	//    PIN_FLIST_TO_STR(payload_flistp,&flistbuf,&flistlen,ebufp );

	    /*
	     * Create flist for order creation
	     */
	    order_i_flistp = PIN_FLIST_CREATE(ebufp);
	    db_no = PIN_POID_GET_DB(acc_poidp); 
	    objp = PIN_POID_CREATE(db_no, "/mso_ntf_sms", -1, ebufp);
	    PIN_FLIST_FLD_SET(order_i_flistp, PIN_FLD_POID, objp, ebufp);
	    PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, order_i_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	    PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SERVICE_OBJ, order_i_flistp, PIN_FLD_SERVICE_OBJ, ebufp);


	    PIN_FLIST_FLD_COPY(order_seq_o_flistp, PIN_FLD_STRING, order_i_flistp, MSO_FLD_NTF_ID, ebufp);
	    PIN_FLIST_FLD_SET(order_i_flistp, PIN_FLD_STATUS, &status, ebufp); //NEW
	    PIN_FLIST_FLD_SET(order_i_flistp, PIN_FLD_RANK, &priority, ebufp); 
		
		// Service Provider Identification - Hathway or DEN 
		if ( *action_flag ==  NTF_PAYMENT || *action_flag ==  NTF_BB_PAYMENT_CASH || *action_flag ==  NTF_BB_PAYMENT_CHEQUE ||*action_flag == NTF_BB_PAYMENT_ONLINE )
		{
			PIN_FLIST_FLD_COPY(payload_flistp, PIN_FLD_CARRIER_ID, order_i_flistp, PIN_FLD_CARRIER_ID, ebufp);
		}

	    nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
	    if ( nota_buf == NULL ) 
	    {
		     pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
		     PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp ); 
		     return;
	    }   

	    nota_buf->flag   = 0;
	    nota_buf->size   = flistlen - 1;
	    nota_buf->offset = 0;
	    nota_buf->data   = ( caddr_t)flistbuf;
	    nota_buf->xbuf_file = ( char *) NULL;

	    PIN_FLIST_FLD_PUT( order_i_flistp, PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );

	    //Adding usage type description to the object
		fm_mso_ntf_common_get_usage_type_descr(ctxp, in_flistp, action_flag, &usage_type, ebufp);
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in function fm_mso_ntf_common_get_usage_type_descr", ebufp);
			return;
		}
		if(usage_type == NULL)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Unable to retrieve usage type description");
			return;
		}

		PIN_FLIST_FLD_PUT(order_i_flistp, PIN_FLD_USAGE_TYPE, usage_type, ebufp);

	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_ntf_create_sms_notification create input flist", order_i_flistp);
	    PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, order_i_flistp, out_flistpp, ebufp);
	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_ntf_create_sms_notification order create output flist", *out_flistpp);
	    PIN_FLIST_FLD_COPY(order_i_flistp, MSO_FLD_NTF_ID, *out_flistpp, MSO_FLD_NTF_ID, ebufp);

	    //Adding logic for sending copy to business users
	    channel = (char *) malloc (sizeof(char) * 60);
	    memset(channel, '\0', sizeof(channel));
	    strcpy(channel, CHANNEL);
	    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Channel type initialized");
	    send_to_users = fm_mso_ntf_common_check_delivery_status(ctxp, in_flistp, usage_type, channel, ebufp);
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Unable to check for delivery status to business users");
		}
	    free(channel);
		
		if(send_to_users)
		{
			fm_mso_ntf_common_get_business_users(ctxp, in_flistp, &bus_users_flistp, ebufp);
			if(PIN_ERRBUF_IS_ERR(ebufp) || bus_users_flistp == NULL)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "fm_mso_ntf_create_sms_notification: Error in retrieving business users list");
			}
			else
			{
                while((bus_users_res_flistp =
                        (pin_flist_t *) PIN_FLIST_ELEM_GET_NEXT(bus_users_flistp, PIN_FLD_RESULTS, &elemid, 1, &cookie, ebufp)) != NULL)
				{
					phone = NULL;
                    phone = PIN_FLIST_FLD_GET(bus_users_res_flistp, PIN_FLD_PHONE, 1, ebufp);

					if(phone)
					{
						//Update phone no in payload
						PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_MSISDN, phone, ebufp);
						//call the function to read the provisioning order sequence
						fm_mso_utils_sequence_no(ctxp, order_seq_i_flistp, &order_seq_o_flistp, ebufp);

						/*
						* Convert flist to String Payload. it will be used by Adaptor process to send SMS
						*/
						flistlen = 0;
						flistbuf = NULL;
                        PIN_FLIST_TO_XML( payload_flistp, PIN_XML_BY_SHORT_NAME, 0, &flistbuf, &flistlen, "SMS", ebufp );

						PIN_FLIST_FLD_COPY(order_seq_o_flistp, PIN_FLD_STRING, order_i_flistp, MSO_FLD_NTF_ID, ebufp);

						nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
						if ( nota_buf == NULL )
						{
							 pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
							 PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
							 return;
						}

						nota_buf->flag   = 0;
						nota_buf->size   = flistlen - 1;
						nota_buf->offset = 0;
						nota_buf->data   = ( caddr_t)flistbuf;
						nota_buf->xbuf_file = ( char *) NULL;

						PIN_FLIST_FLD_PUT( order_i_flistp, PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );

						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                    "fm_mso_ntf_create_sms_notification create busines user input flist", order_i_flistp);
						PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, order_i_flistp, out_flistpp, ebufp);
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
										"fm_mso_ntf_create_sms_notification order create business user output flist", *out_flistpp);
						PIN_FLIST_FLD_COPY(order_i_flistp, MSO_FLD_NTF_ID, *out_flistpp, MSO_FLD_NTF_ID, ebufp);
					}
					else
					{
						continue;
					}
				}
			}
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No need to send to business users");
		}
	}

Cleanup:
	if(notif_disabled == 1) 
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," Preparing dummy output flist with status as SUCCESS");
		*out_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_STATUS, &op_status, ebufp);
	}
    PIN_FLIST_DESTROY_EX(&payload_flistp, NULL);
    //PIN_FLIST_DESTROY_EX(&order_seq_i_flistp, NULL);
    //PIN_FLIST_DESTROY_EX(&order_seq_o_flistp, NULL);
    //PIN_FLIST_DESTROY_EX(&order_i_flistp, NULL);
    
    return;
}

/*******************************************************************
 * prepare_service_activation_notification_buf()
 *
 * Enrich input filst with service details required
 * optionally performa validation on input fields
 *******************************************************************/
static void
prepare_service_activation_notification_buf(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **ret_flistpp,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *si_flistp = NULL;
    pin_flist_t             *so_flistp = NULL;
    pin_flist_t             *ai_flistp = NULL;
    pin_flist_t             *ao_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    char                    *ne = NULL;
    char                    *plan_code = NULL;
    char                    *sms_template = NULL;
    char                    *account_no = NULL;
    char                    *stb_id = NULL;
    char                    *vc_id = NULL;
    char                    *phone = NULL;
    char                     str[1024];
	char                     msg[200];
	pin_decimal_t			 *divisor = pbo_decimal_from_str("10000", ebufp);

    int32               business_type = 0;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_service_activation_notification_buf input flist", i_flistp);


    /*
     * Validate input received
     */



    /*
     * GET STB id and Smart card no
     */
    si_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);
    tmp_flistp = PIN_FLIST_SUBSTR_ADD(si_flistp, MSO_FLD_CATV_INFO, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_NETWORK_NODE, NULL, ebufp);
    tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, PIN_ELEMID_ANY, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_NAME, NULL, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_service_activation_notification_buf PCM_OP_READ_FLDS input flist", si_flistp);
    PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_service_activation_notification_buf PCM_OP_READ_FLDS output flist", so_flistp);

    tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
    ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 
    plan_code = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CODE, 1, ebufp);
    /*
     * get action and NE specific SMS template
     */


	business_type = *(int *) PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BUSINESS_TYPE, 0, ebufp);
	business_type = business_type/10000;
	sprintf(msg, " business_type = %d", &business_type);
	PIN_ERR_LOG_MSG(3, msg);

	//if ( business_type == IPTV_BUSINESS_TYPE )
//	{
		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_BUSINESS_TYPE, &business_type, ebufp);
	//}



    get_sms_template(ctxp, i_flistp, &sms_template, ne, ebufp);
    if (!sms_template)
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
        goto CLEANUP;
    }

    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, sms_template );

    
    account_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
    phone = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PHONE, 0, ebufp);
    /*
     * Prepare Message
     */
	business_type = business_type/10000;
	if (ne && business_type == IPTV_BUSINESS_TYPE &&
		( strstr(ne,"JVM1") || strstr(ne,"JVM") )    )
    {
		sprintf(str,sms_template, plan_code, account_no);
	}
    else if (ne && strstr(ne,"NDS") )
    {
        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 1, 0, ebufp);
        vc_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        sprintf(str,sms_template, account_no, phone);
    }
    else if (ne && strstr(ne,"CISCO") )
    {
        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 2, 0, ebufp);
        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        sprintf(str,sms_template, account_no, phone);
    }
	/*********** VERIMATRIX CHANGES - BEGIN - added verimatrix network node condition **/
    else if (ne && strstr(ne,"VM") )
    {
        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        sprintf(str,sms_template, account_no, phone);
    }	
	/*********** VERIMATRIX CHANGES - END - added verimatrix network node condition **/
    else
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
        goto CLEANUP;
    }

    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );

    /*
     * Push Message in SMS payload
     */
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_MESSAGE, str, ebufp);


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_service_activation_notification_buf enriched input flist", *ret_flistpp);

CLEANUP:
    if (sms_template)
    {
        free(sms_template);
    }

    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&ai_flistp, NULL);
    return;
}

/*******************************************************************
 * prepare_prepaid_lim_usage_notification_buf()
 *
 * Enrich input filst with service details required
 * optionally performa validation on input fields
 *******************************************************************/
static void
prepare_prepaid_lim_usage_notification_buf(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **ret_flistpp,
    pin_errbuf_t            *ebufp)
{
    char                    *sms_template = NULL;
	char                     str[1024];
	pin_decimal_t			*quota = NULL;
	pin_decimal_t			*mb_consumed = NULL;
	pin_decimal_t           *zero = pin_decimal( "0.0", ebufp );

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_prepaid_lim_usage_notification_buf input flist", i_flistp);

    /*
     * Validate input received
     */

	quota = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_TOTAL_VALUE, 1, ebufp);
	if (pbo_decimal_is_null(quota, ebufp)) 
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"PIN_FLD_TOTAL_VALUE not set in input flist. Exiting without raising notification.");
		return;
	}

	mb_consumed = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USED_QUANTITY, 1, ebufp);
	if (pbo_decimal_is_null(quota, ebufp)) 
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"PIN_FLD_USED_QUANTITY not set in input flist. Exiting without raising notification.");
		return;
	}
	if (pbo_decimal_compare(mb_consumed,zero, ebufp) == 0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"PIN_FLD_USED_QUANTITY not set in input flist. Exiting without raising notification.");
		return;
	}

	get_bb_sms_template(ctxp, i_flistp, &sms_template, ebufp);
	if (!sms_template)
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
		goto CLEANUP;
	}

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, sms_template );
	/*
	 * Prepare Message
	 */
	sprintf(str, sms_template, pbo_decimal_to_str(mb_consumed,ebufp), pbo_decimal_to_str(quota,ebufp));
	PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_MESSAGE, str, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_prepaid_lim_usage_notification_buf enriched input flist", *ret_flistpp);

CLEANUP:
    if (sms_template)
    {
        free(sms_template);
    }
    return;
}

/*******************************************************************
 * prepare_service_suspension_notification_buf()
 *
 * Enrich input filst with service details required
 * optionally performa validation on input fields
 *******************************************************************/
static void
prepare_service_suspension_notification_buf(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **ret_flistpp,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *si_flistp = NULL;
    pin_flist_t             *so_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    char                    *ne = NULL;
    char                    *sms_template = NULL;
    char                    *account_no = NULL;
    char                    *stb_id = NULL;
    char                    *vc_id = NULL;
    char                     str[1024];
    char                     due_date[50] = "";
    poid_t		    *service_pdp = NULL;
    time_t		    *eff_t = NULL;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);


    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_service_suspension_notification_buf input flist", i_flistp);


    /*
     * Validate input received
     */

    account_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);

	service_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp,
		PIN_FLD_SERVICE_OBJ, 1, ebufp);
	if(PIN_POID_IS_NULL(service_pdp) || 
		!(strcmp(PIN_POID_GET_TYPE(service_pdp), "/service/catv")))
	{

    /*
     * GET STB id and Smart card no
     */
    si_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);
    tmp_flistp = PIN_FLIST_SUBSTR_ADD(si_flistp, MSO_FLD_CATV_INFO, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_NETWORK_NODE, NULL, ebufp);
    tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, PIN_ELEMID_ANY, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_NAME, NULL, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_service_suspension_notification_buf PCM_OP_READ_FLDS input flist", si_flistp);
    PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_service_suspension_notification_buf PCM_OP_READ_FLDS output flist", so_flistp);

    tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
    ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 

    /*
     * get action and NE specific SMS template
     */
    get_sms_template(ctxp, i_flistp, &sms_template, ne, ebufp);
    if (!sms_template)
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
        goto CLEANUP;
    }

    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, sms_template );

    account_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);

    /*
     * Prepare Message
     */
    if (ne && strstr(ne,"NDS") )
    {
        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 1, 0, ebufp);
        vc_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        sprintf(str,sms_template, account_no, stb_id, vc_id);
    }
    else if (ne && strstr(ne,"CISCO") )
    {
        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 2, 0, ebufp);
        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        sprintf(str,sms_template, account_no, stb_id);
    }
	/*********** VERIMATRIX CHANGES - BEGIN - added verimatrix network node condition **/
    else if (ne && strstr(ne,"VM") )
    {
        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        sprintf(str,sms_template, account_no, stb_id);
    }	
	/*********** VERIMATRIX CHANGES - END - added verimatrix network node condition **/
    else
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
        goto CLEANUP;
    }

    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );

    /*
     * Push Message in SMS payload
     */
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_MESSAGE, str, ebufp);
	} else {
		get_bb_sms_template(ctxp, i_flistp, &sms_template, ebufp);
		if (!sms_template)
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
			goto CLEANUP;
		}

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, sms_template );
		/*
		 * Prepare Message
		 */
		eff_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_EFFECTIVE_T, 0, ebufp);
		pin_strftimet(due_date, sizeof(due_date), "%d-%b-%y", *eff_t);
		sprintf(str, sms_template, account_no, due_date);
    		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_MESSAGE, str, ebufp);
	}


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_service_suspension_notification_buf enriched input flist", *ret_flistpp);

CLEANUP:
    if (sms_template)
    {
        free(sms_template);
    }

    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
    return;
}

/*******************************************************************
 * prepare_service_reactivation_notification_buf()
 *
 * Enrich input filst with service details required
 * optionally performa validation on input fields
 *******************************************************************/
static void
prepare_service_reactivation_notification_buf(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **ret_flistpp,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *si_flistp = NULL;
    pin_flist_t             *so_flistp = NULL;
    pin_flist_t             *ai_flistp = NULL;
    pin_flist_t             *ao_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    char                    *ne = NULL;
    char                    *sms_template = NULL;
    char                    *account_no = NULL;
    char                    *stb_id = NULL;
    char                    *vc_id = NULL;
    char                     str[1024];

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);


    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_service_reactivation_notification_buf input flist", i_flistp);


    /*
     * Validate input received
     */



    /*
     * GET STB id and Smart card no
     */
    si_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);
    tmp_flistp = PIN_FLIST_SUBSTR_ADD(si_flistp, MSO_FLD_CATV_INFO, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_NETWORK_NODE, NULL, ebufp);
    tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, PIN_ELEMID_ANY, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_NAME, NULL, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_service_reactivation_notification_buf PCM_OP_READ_FLDS input flist", si_flistp);
    PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_service_reactivation_notification_buf PCM_OP_READ_FLDS output flist", so_flistp);

    tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
    ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 

    /*
     * get action and NE specific SMS template
     */
    get_sms_template(ctxp, i_flistp, &sms_template, ne, ebufp);
    if (!sms_template)
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
        goto CLEANUP;
    }

    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, sms_template );

    /*
     * Read account no
     */
    ai_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ai_flistp, PIN_FLD_POID, ebufp);
    PIN_FLIST_FLD_SET(ai_flistp, PIN_FLD_ACCOUNT_NO, NULL, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_service_reactivation_notification_buf PCM_OP_READ_FLDS input flist", ai_flistp);
    PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, ai_flistp, &ao_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_service_reactivation_notification_buf PCM_OP_READ_FLDS output flist", ao_flistp);

    account_no = PIN_FLIST_FLD_GET(ao_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);

    /*
     * Prepare Message
     */
    if (ne && strstr(ne,"NDS") )
    {
        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 1, 0, ebufp);
        vc_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        sprintf(str,sms_template, account_no, stb_id, vc_id);
    }
    else if (ne && strstr(ne,"CISCO") )
    {
        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 2, 0, ebufp);
        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        sprintf(str,sms_template, account_no, stb_id);
    }
	/*********** VERIMATRIX CHANGES - BEGIN - added verimatrix network node condition **/
    else if (ne && strstr(ne,"VM") )
    {
        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        sprintf(str,sms_template, account_no, stb_id);
    }	
	/*********** VERIMATRIX CHANGES - END - added verimatrix network node condition **/
    else
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
        goto CLEANUP;
    }

    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );

    /*
     * Push Message in SMS payload
     */
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_MESSAGE, str, ebufp);


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_service_reactivation_notification_buf enriched input flist", *ret_flistpp);

CLEANUP:
    if (sms_template)
    {
        free(sms_template);
    }

    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&ai_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&ao_flistp, NULL);
    return;
}

/**************************************************************************
 * prepare_service_termination_notification_buf()
 *
 * Enrich input filst with service details required
 * optionally performa validation on input fields
 **************************************************************************/
static void
prepare_service_termination_notification_buf(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **ret_flistpp,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *si_flistp = NULL;
    pin_flist_t             *so_flistp = NULL;
    pin_flist_t             *ai_flistp = NULL;
    pin_flist_t             *ao_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
	poid_t					*service_pdp = NULL;
    struct                  tm *current_time;
    time_t                  current_t = 0;
    char                    *ne = NULL;
    char                    *sms_template = NULL;
    char                    *account_no = NULL;
    char                    *stb_id = NULL;
    char                    *vc_id = NULL;
    char                     str[1024];
    char                    eff_date[12];
    int32                   business_type = 0;
    int                     year;
    int                     month;
    int                     day;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);


    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_service_termination_notification_buf input flist", i_flistp);


    /*
     * Validate input received
     */

    business_type = *(int *) PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_BUSINESS_TYPE,0,ebufp);
    account_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);

	service_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp,
		PIN_FLD_SERVICE_OBJ, 1, ebufp);
	if(PIN_POID_IS_NULL(service_pdp) || 
		!(strcmp(PIN_POID_GET_TYPE(service_pdp), "/service/catv")))
	{
		/*
		 * GET STB id and Smart card no
		 */
		si_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);
		tmp_flistp = PIN_FLIST_SUBSTR_ADD(si_flistp, MSO_FLD_CATV_INFO, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_NETWORK_NODE, NULL, ebufp);
		tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_NAME, NULL, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"prepare_service_termination_notification_buf PCM_OP_READ_FLDS input flist", si_flistp);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"prepare_service_termination_notification_buf PCM_OP_READ_FLDS output flist", so_flistp);

		tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
		ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 

		/*
		 * get action and NE specific SMS template
		 */
		get_sms_template(ctxp, i_flistp, &sms_template, ne, ebufp);
		if (!sms_template)
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
			goto CLEANUP;
		}

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, sms_template );

        /*
		 * Prepare Message
		 */
		if (ne && strstr(ne,"NDS") )
		{
			tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
			stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

			tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 1, 0, ebufp);
			vc_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

			sprintf(str,sms_template, account_no, stb_id, vc_id);
		}
		else if (ne && strstr(ne,"CISCO") )
		{
			tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 2, 0, ebufp);
			stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

			sprintf(str,sms_template, account_no, stb_id);
		}
		/*********** VERIMATRIX CHANGES - BEGIN - added verimatrix network node condition **/
		else if (ne && strstr(ne,"VM") )
		{
			tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
			stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);
    
	        business_type = business_type/10000;
            if (business_type == IPTV_BUSINESS_TYPE)
            {
                current_t = pin_virtual_time(NULL);
                current_time = localtime(&current_t);
		        year = current_time->tm_year + 1900;
		        month = current_time->tm_mon +1;
		        day = current_time->tm_mday;
		        sprintf(eff_date,"%d-%d-%d",day,month,year);
			    sprintf(str,sms_template, account_no, eff_date);
            }
            else
            {
			    sprintf(str,sms_template, account_no, stb_id);
            }
		}		
		/*********** VERIMATRIX CHANGES - END - added verimatrix network node condition **/
		else
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
			goto CLEANUP;
		}
	}
	//For BB Notification
	else
	{
		get_bb_sms_template(ctxp, i_flistp, &sms_template, ebufp);
		if (!sms_template)
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
			goto CLEANUP;
		}

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, sms_template );
		/*
		 * Prepare Message
		 */
		 sprintf(str, sms_template, account_no);
	}
    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );

    /*
     * Push Message in SMS payload
     */
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_MESSAGE, str, ebufp);


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_service_termination_notification_buf enriched input flist", *ret_flistpp);

CLEANUP:
    if (sms_template)
    {
        free(sms_template);
    }

    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&ai_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&ao_flistp, NULL);
    return;
}

/**************************************************************************
 * prepare_service_swap_notification_buf()
 *
 * Enrich input filst with service details required
 * optionally performa validation on input fields
 **************************************************************************/
static void
prepare_service_swap_notification_buf(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **ret_flistpp,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *si_flistp = NULL;
    pin_flist_t             *so_flistp = NULL;
    pin_flist_t             *ai_flistp = NULL;
    pin_flist_t             *ao_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
	poid_t					*service_pdp = NULL;
    struct                  tm *current_time;
    time_t                  current_t = 0;
    char                    *ne = NULL;
    char                    *sms_template = NULL;
    char                    *account_no = NULL;
    char                    *stb_id = NULL;
    char                    *vc_id = NULL;
    char                     str[1024];
    char                    eff_date[12];
    int32                   business_type = 0;
    int                     year;
    int                     month;
    int                     day;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);


    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_service_swap_notification_buf input flist", i_flistp);


    /*
     * Validate input received
     */

    business_type = *(int *) PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_BUSINESS_TYPE,0,ebufp);
    account_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);

	service_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp,
		PIN_FLD_SERVICE_OBJ, 1, ebufp);
	if(PIN_POID_IS_NULL(service_pdp) || 
		!(strcmp(PIN_POID_GET_TYPE(service_pdp), "/service/catv")))
	{
		/*
		 * GET STB id and Smart card no
		 */
		si_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);
		tmp_flistp = PIN_FLIST_SUBSTR_ADD(si_flistp, MSO_FLD_CATV_INFO, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_NETWORK_NODE, NULL, ebufp);
		tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_NAME, NULL, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"prepare_service_termination_notification_buf PCM_OP_READ_FLDS input flist", si_flistp);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"prepare_service_termination_notification_buf PCM_OP_READ_FLDS output flist", so_flistp);

		tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
		ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 

		/*
		 * get action and NE specific SMS template
		 */
		get_sms_template(ctxp, i_flistp, &sms_template, ne, ebufp);
		if (!sms_template)
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
			goto CLEANUP;
		}

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, sms_template );

        /*
		 * Prepare Message
		 */
		if (ne && strstr(ne,"NDS") )
		{
			tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
			stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

			tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 1, 0, ebufp);
			vc_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

			sprintf(str,sms_template, account_no, stb_id, vc_id);
		}
		else if (ne && strstr(ne,"CISCO") )
		{
			tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 2, 0, ebufp);
			stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

			sprintf(str,sms_template, account_no, stb_id);
		}
		/*********** VERIMATRIX CHANGES - BEGIN - added verimatrix network node condition **/
		else if (ne && strstr(ne,"VM") )
		{
			tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
			stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);
    
	        business_type = business_type/10000;
            if (business_type == IPTV_BUSINESS_TYPE)
            {
			    stb_id = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SERIAL_NO, 0, ebufp);
			    sprintf(str,sms_template, account_no, stb_id);
            }
            else
            {
			    sprintf(str,sms_template, account_no, stb_id);
            }
		}		
		/*********** VERIMATRIX CHANGES - END - added verimatrix network node condition **/
		else
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
			goto CLEANUP;
		}
	}
	//For BB Notification
	else
	{
		get_bb_sms_template(ctxp, i_flistp, &sms_template, ebufp);
		if (!sms_template)
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
			goto CLEANUP;
		}

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, sms_template );
		/*
		 * Prepare Message
		 */
		 sprintf(str, sms_template, account_no);
	}
    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );

    /*
     * Push Message in SMS payload
     */
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_MESSAGE, str, ebufp);


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_service_termination_notification_buf enriched input flist", *ret_flistpp);

CLEANUP:
    if (sms_template)
    {
        free(sms_template);
    }

    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&ai_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&ao_flistp, NULL);
    return;
}

/**************************************************************************
 * prepare_add_plan_notification_buf()
 *
 * Enrich input filst with service details required
 * optionally performa validation on input fields
 **************************************************************************/
static void
prepare_add_plan_notification_buf(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **ret_flistpp,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *si_flistp = NULL;
    pin_flist_t             *so_flistp = NULL;
    pin_flist_t             *ai_flistp = NULL;
    pin_flist_t             *ao_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    char                    *ne = NULL;
    char                    *sms_template = NULL;
    char                    *account_no = NULL;
    char                    *stb_id = NULL;
    char                    *vc_id = NULL;
    char                    str[1024];
    char                    package[256];
    int32                   rec_id = 1;
    pin_cookie_t            cookie = NULL;
    char                    *plan_name = NULL;
    int32                   plan_count;
    int32                   count = 0;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);


    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_add_plan_notification_buf input flist", i_flistp);

    /*
     * Validate input received
     */
    tmp_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PLAN, 0, 0, ebufp);
    plan_name = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0 , ebufp);
    plan_count = PIN_FLIST_ELEM_COUNT(i_flistp, PIN_FLD_PLAN, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    strcpy(package, "");

     while ( (tmp_flistp = PIN_FLIST_ELEM_GET_NEXT (i_flistp,
                PIN_FLD_PLAN, &rec_id, 1,&cookie, ebufp))
                                          != (pin_flist_t *)NULL ) 
     {
        plan_name = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0 , ebufp);
        strcat(package, plan_name);
        if (++count != plan_count)
        {
            strcat(package, ", ");
        }
     }

    /*
     * GET STB id and Smart card no
     */
    si_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);
    tmp_flistp = PIN_FLIST_SUBSTR_ADD(si_flistp, MSO_FLD_CATV_INFO, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_NETWORK_NODE, NULL, ebufp);
    tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, PIN_ELEMID_ANY, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_NAME, NULL, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_add_plan_notification_buf PCM_OP_READ_FLDS input flist", si_flistp);
    PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_add_plan_notification_buf PCM_OP_READ_FLDS output flist", so_flistp);

    tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
    ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 

    /*
     * get action and NE specific SMS template
     */
    get_sms_template(ctxp, i_flistp, &sms_template, ne, ebufp);
    if (!sms_template)
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
        goto CLEANUP;
    }

    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, sms_template );

    /*
     * Read account no
     */
    ai_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ai_flistp, PIN_FLD_POID, ebufp);
    PIN_FLIST_FLD_SET(ai_flistp, PIN_FLD_ACCOUNT_NO, NULL, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_add_plan_notification_buf PCM_OP_READ_FLDS input flist", ai_flistp);
    PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, ai_flistp, &ao_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_add_plan_notification_buf PCM_OP_READ_FLDS output flist", ao_flistp);

    account_no = PIN_FLIST_FLD_GET(ao_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);

    /*
     * Prepare Message
     */
    if (ne && strstr(ne,"NDS") )
    {
        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 1, 0, ebufp);
        vc_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        sprintf(str,sms_template,package, account_no, stb_id, vc_id);
    }
    else if (ne && strstr(ne,"CISCO") )
    {
        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 2, 0, ebufp);
        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        sprintf(str,sms_template, package, account_no, stb_id);
    }
	/*********** VERIMATRIX CHANGES - BEGIN - added verimatrix network node condition **/
    else if (ne && strstr(ne,"VM") )
    {
        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        sprintf(str,sms_template, package, account_no, stb_id);
    }	
	/*********** VERIMATRIX CHANGES - END - added verimatrix network node condition **/
    else
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
        goto CLEANUP;
    }

    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );

    /*
     * Push Message in SMS payload
     */
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_MESSAGE, str, ebufp);


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_add_plan_notification_buf enriched input flist", *ret_flistpp);

CLEANUP:
    if (sms_template)
    {
        free(sms_template);
    }

    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&ai_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&ao_flistp, NULL);
    return;
}

/*******************************************************************
 * prepare_change_plan_notification_buf()
 *
 * Enrich input filst with service details required
 * optionally performa validation on input fields
 *******************************************************************/
static void
prepare_change_plan_notification_buf(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **ret_flistpp,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *si_flistp = NULL;
    pin_flist_t             *so_flistp = NULL;
    pin_flist_t             *ai_flistp = NULL;
    pin_flist_t             *ao_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
	poid_t					*service_pdp = NULL;
    char                    *ne = NULL;
    char                    *sms_template = NULL;
    char                    *account_no = NULL;
    char                    *stb_id = NULL;
    char                    *vc_id = NULL;
    char                    str[1024];
    char                    *cancealled_plan_name = NULL;
    char                    *added_plan_name = NULL;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);


    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_change_plan_notification_buf input flist", i_flistp);

    /*
     * Validate input received
     */
    tmp_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PLAN, 0, 0, ebufp);
    cancealled_plan_name = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0 , ebufp);
    tmp_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PLAN, 1, 0, ebufp);
    added_plan_name = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0 , ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;
	
	service_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	if(PIN_POID_IS_NULL(service_pdp) || 
		!(strcmp(PIN_POID_GET_TYPE(service_pdp), "/service/catv")))
	{
		//For CATV
		/*
		 * GET STB id and Smart card no
		 */
		si_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);
		tmp_flistp = PIN_FLIST_SUBSTR_ADD(si_flistp, MSO_FLD_CATV_INFO, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_NETWORK_NODE, NULL, ebufp);
		tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_NAME, NULL, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"prepare_change_plan_notification_buf PCM_OP_READ_FLDS input flist", si_flistp);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"prepare_change_plan_notification_buf PCM_OP_READ_FLDS output flist", so_flistp);

		tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
		ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 

		/*
		 * get action and NE specific SMS template
		 */
		get_sms_template(ctxp, i_flistp, &sms_template, ne, ebufp);
		if (!sms_template)
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
			goto CLEANUP;
		}

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, sms_template );

		/*
		 * Read account no
		 */
		ai_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ai_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(ai_flistp, PIN_FLD_ACCOUNT_NO, NULL, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"prepare_change_plan_notification_buf PCM_OP_READ_FLDS input flist", ai_flistp);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, ai_flistp, &ao_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"prepare_change_plan_notification_buf PCM_OP_READ_FLDS output flist", ao_flistp);

		account_no = PIN_FLIST_FLD_GET(ao_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);

		/*
		 * Prepare Message
		 */
		if (ne && strstr(ne,"NDS") )
		{
			tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
			stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

			tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 1, 0, ebufp);
			vc_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

			sprintf(str,sms_template, cancealled_plan_name, added_plan_name, account_no, stb_id, vc_id);
		}
		else if (ne && strstr(ne,"CISCO") )
		{
			tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 2, 0, ebufp);
			stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

			sprintf(str,sms_template, cancealled_plan_name, added_plan_name, account_no, stb_id);
		}
		/*********** VERIMATRIX CHANGES - BEGIN - added verimatrix network node condition **/
		else if (ne && strstr(ne,"VM") )
		{
			tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
			stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

			sprintf(str,sms_template, cancealled_plan_name, added_plan_name, account_no, stb_id);
		}		
		/*********** VERIMATRIX CHANGES - END - added verimatrix network node condition **/
		else
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
			goto CLEANUP;
		}
	}
	else
	{
		//For BB
		get_bb_sms_template(ctxp, i_flistp, &sms_template, ebufp);
		if (!sms_template)
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
			goto CLEANUP;
		}

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, sms_template );
		/*
		 * Prepare Message
		 */
		sprintf(str, sms_template, cancealled_plan_name, added_plan_name);

	}
    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );

    /*
     * Push Message in SMS payload
     */
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_MESSAGE, str, ebufp);


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_change_plan_notification_buf enriched input flist", *ret_flistpp);

CLEANUP:
    if (sms_template)
    {
        free(sms_template);
    }

    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&ai_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&ao_flistp, NULL);
    return;
}

/*******************************************************************
 * prepare_cancel_plan_notification_buf()
 *
 * Enrich input filst with service details required
 * optionally performa validation on input fields
 *******************************************************************/
static void
prepare_cancel_plan_notification_buf(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **ret_flistpp,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *si_flistp = NULL;
    pin_flist_t             *so_flistp = NULL;
    pin_flist_t             *ai_flistp = NULL;
    pin_flist_t             *ao_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    char                    *ne = NULL;
    char                    *sms_template = NULL;
    char                    *account_no = NULL;
    char                    *stb_id = NULL;
    char                    *vc_id = NULL;
    char                    str[1024];
    char                    package[256];
    int32                   rec_id = 1;
    pin_cookie_t            cookie = NULL;
    char                    *plan_name = NULL;
    int32                   plan_count;
    int32                   count = 0;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);


    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_cancel_plan_notification_buf input flist", i_flistp);

    /*
     * Validate input received
     */
    tmp_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PLAN, 0, 0, ebufp);
    plan_name = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0 , ebufp);
    plan_count = PIN_FLIST_ELEM_COUNT(i_flistp, PIN_FLD_PLAN, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    strcpy(package, "");

     while ( (tmp_flistp = PIN_FLIST_ELEM_GET_NEXT (i_flistp,
                PIN_FLD_PLAN, &rec_id, 1,&cookie, ebufp))
                                          != (pin_flist_t *)NULL ) 
     {
        plan_name = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0 , ebufp);
        strcat(package, plan_name);
        if (++count != plan_count)
        {
            strcat(package, ", ");
        }
     }

    /*
     * GET STB id and Smart card no
     */
    si_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);
    tmp_flistp = PIN_FLIST_SUBSTR_ADD(si_flistp, MSO_FLD_CATV_INFO, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_NETWORK_NODE, NULL, ebufp);
    tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, PIN_ELEMID_ANY, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_NAME, NULL, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_cancel_plan_notification_buf PCM_OP_READ_FLDS input flist", si_flistp);
    PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_cancel_plan_notification_buf PCM_OP_READ_FLDS output flist", so_flistp);

    tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
    ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 

    /*
     * get action and NE specific SMS template
     */
    get_sms_template(ctxp, i_flistp, &sms_template, ne, ebufp);
    if (!sms_template)
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
        goto CLEANUP;
    }

    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, sms_template );

    /*
     * Read account no
     */
    ai_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ai_flistp, PIN_FLD_POID, ebufp);
    PIN_FLIST_FLD_SET(ai_flistp, PIN_FLD_ACCOUNT_NO, NULL, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_cancel_plan_notification_buf PCM_OP_READ_FLDS input flist", ai_flistp);
    PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, ai_flistp, &ao_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_cancel_plan_notification_buf PCM_OP_READ_FLDS output flist", ao_flistp);

    account_no = PIN_FLIST_FLD_GET(ao_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);

    /*
     * Prepare Message
     */
    if (ne && strstr(ne,"NDS") )
    {
        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 1, 0, ebufp);
        vc_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        sprintf(str,sms_template,package, account_no, stb_id, vc_id);
    }
    else if (ne && strstr(ne,"CISCO") )
    {
        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 2, 0, ebufp);
        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        sprintf(str,sms_template, package, account_no, stb_id);
    }
	/*********** VERIMATRIX CHANGES - BEGIN - added verimatrix network node condition **/
    else if (ne && strstr(ne,"VM") )
    {
        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        sprintf(str,sms_template, package, account_no, stb_id);
    }	
	/*********** VERIMATRIX CHANGES - END - added verimatrix network node condition **/
    else
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
        goto CLEANUP;
    }

    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );

    /*
     * Push Message in SMS payload
     */
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_MESSAGE, str, ebufp);


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_cancel_plan_notification_buf enriched input flist", *ret_flistpp);

CLEANUP:
    if (sms_template)
    {
        free(sms_template);
    }

    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&ai_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&ao_flistp, NULL);
    return;
}

/*******************************************************************
 * prepare_billing_notification_buf()
 *
 * Enrich input filst with service details required
 * optionally performa validation on input fields
 *******************************************************************/
static void
prepare_billing_notification_buf(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **ret_flistpp,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *p_svc_flistp = NULL;
    pin_flist_t             *ai_flistp = NULL;
    pin_flist_t             *ao_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    pin_flist_t             *tmp1_flistp = NULL;
	poid_t					*service_pdp = NULL;
    char                    *ne = NULL;
    char                    *sms_template = NULL;
    char                    *osd_template = NULL;
    char                    *bmail_template = NULL;
    char                    *account_no = NULL;
    char                    *stb_id = NULL;
    char                    *vc_id = NULL;
    char                    str[1024];
	char					due_date[50] = {""};
	char					curr_date[50] = {""};
    char                    *bill_no = NULL;
    pin_decimal_t           *amount_d = NULL;
    double                  amount;
    int32                   send_osd = PIN_BOOLEAN_FALSE;
    int32                   send_bmail = PIN_BOOLEAN_FALSE;
    void                    *vp = NULL;
	time_t					*now = 0;
	time_t					*due_t = 0;


    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);


    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_billing_notification_buf input flist", i_flistp);

    /*
     * Validate input received
     */
    bill_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BILL_NO, 0 , ebufp);
    amount_d = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 0 , ebufp);
    amount = pbo_decimal_to_double(amount_d, ebufp);
	/*
	 * Read account no
	 */
	ai_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ai_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(ai_flistp, PIN_FLD_ACCOUNT_NO, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_billing_notification_buf PCM_OP_READ_FLDS input flist", ai_flistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, ai_flistp, &ao_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_billing_notification_buf PCM_OP_READ_FLDS output flist", ao_flistp);
	
	account_no = PIN_FLIST_FLD_GET(ao_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
	due_t = (time_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DUE_T, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"prepare_billing_notification_buf: Error", ebufp);
		goto cleanup;
	}
	//Convert the Due Date to String
	pin_strftimet(due_date, sizeof(due_date), "%d-%b-%y", *due_t);

	// Get the BILL DATE from INPUT FLIST to set in the message
	now = (time_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"prepare_billing_notification_buf: Error", ebufp);
		goto cleanup;
	}
	//Convert the current time to string
//	pin_strftimet(curr_date, sizeof(curr_date), "%d-%b-%y", *now);
    service_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);

	if(PIN_POID_IS_NULL(service_pdp) || 
		!(strcmp(PIN_POID_GET_TYPE(service_pdp), "/service/catv")))
	{
		pin_strftimet(curr_date, sizeof(curr_date), "%d-%b-%y", *now);
		vp = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SEND_OSD, 1, ebufp);
		if (vp)
		{
			send_osd = *(int32 *)vp;
		}

		vp = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SEND_BMAIL, 1, ebufp);
		if (vp)
		{
			send_bmail = *(int32 *)vp;
		}

		if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

		/*
		 *Get Primary CATV connection
		 */
		get_primary_connection_details(ctxp, i_flistp, &p_svc_flistp, ebufp);

		tmp1_flistp = PIN_FLIST_SUBSTR_GET(p_svc_flistp, PIN_FLD_RESULTS, 0, ebufp);
		tmp_flistp = PIN_FLIST_SUBSTR_GET(tmp1_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
		ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);

		if (ne && strstr(ne,"NDS") )
		{
			tmp_flistp = PIN_FLIST_ELEM_GET(tmp1_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
			stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);
			tmp_flistp = PIN_FLIST_ELEM_GET(tmp1_flistp, PIN_FLD_ALIAS_LIST, 1, 0, ebufp);
			vc_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);
		}
		else if (ne && strstr(ne,"CISCO") )
		{
			tmp_flistp = PIN_FLIST_ELEM_GET(tmp1_flistp, PIN_FLD_ALIAS_LIST, 2, 0, ebufp);
			stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);
		}
		/*********** VERIMATRIX CHANGES - BEGIN - added verimatrix network node condition **/
		else if (ne && strstr(ne,"VM") )
		{
			tmp_flistp = PIN_FLIST_ELEM_GET(tmp1_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
			stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);
		}		
		/*********** VERIMATRIX CHANGES - END - added verimatrix network node condition **/
		else
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
			goto cleanup;
		}

		if (PIN_ERRBUF_IS_ERR(ebufp))
			goto cleanup;
               /*
                 * Read account no
                 */
                ai_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ai_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(ai_flistp, PIN_FLD_ACCOUNT_NO, NULL, ebufp);
            
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                    "prepare_billing_notification_buf PCM_OP_READ_FLDS input flist", ai_flistp);
                PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, ai_flistp, &ao_flistp, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                    "prepare_billing_notification_buf PCM_OP_READ_FLDS output flist", ao_flistp);
            
                account_no = PIN_FLIST_FLD_GET(ao_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
		

	   if (send_osd == PIN_BOOLEAN_TRUE)
		{
			get_osd_template(ctxp, i_flistp, &osd_template, ne, ebufp);
			if (!osd_template)
			{
				pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
						PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
				goto cleanup;
			}
			if (ne && strstr(ne,"NDS") )
			{
				sprintf(str,osd_template, bill_no, account_no, stb_id, vc_id, amount);
			}
			else if (ne && strstr(ne,"CISCO") )
			{
				sprintf(str,osd_template, bill_no, account_no, stb_id, amount);
			}
			/*********** VERIMATRIX CHANGES - BEGIN - added verimatrix network node condition **/
			else if (ne && strstr(ne,"VM") )
			{
				sprintf(str,osd_template, bill_no, account_no, stb_id, amount);
			}	
			/*********** VERIMATRIX CHANGES - END - added verimatrix network node condition **/

			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );
			fm_mso_send_osd_request(ctxp, p_svc_flistp, str, ne, ebufp);
		}

		if (send_bmail == PIN_BOOLEAN_TRUE)
		{
			get_bmail_template(ctxp, i_flistp, &bmail_template, ne, ebufp);
			if (!bmail_template)
			{
				pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
						PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
				goto cleanup;
			}
			if (ne && strstr(ne,"NDS") )
			{
				sprintf(str,osd_template, bill_no, account_no, stb_id, vc_id, amount);
			}
			else if (ne && strstr(ne,"CISCO") )
			{
				sprintf(str,osd_template, bill_no, account_no, stb_id, amount);
			}
			/*********** VERIMATRIX CHANGES - BEGIN - added verimatrix network node condition **/
			else if (ne && strstr(ne,"VM") )
			{
				sprintf(str,osd_template, bill_no, account_no, stb_id, amount);
			}		
			/*********** VERIMATRIX CHANGES - END - added verimatrix network node condition **/

			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );
			fm_mso_send_bmail_request(ctxp, p_svc_flistp, str, ne, ebufp);
		}


		/*
		 * get action and NE specific SMS template
		 */
		get_sms_template(ctxp, i_flistp, &sms_template, ne, ebufp);
		if (!sms_template)
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
			goto cleanup;
		}

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, sms_template );

		/*
		 * Prepare Message
		 */
		if (ne && strstr(ne,"NDS") )
		{
			sprintf(str,sms_template, bill_no, account_no, stb_id, vc_id, amount);
		}
		else if (ne && strstr(ne,"CISCO") )
		{
			sprintf(str,sms_template, bill_no, account_no, stb_id, amount);
		}
		// VERIMATRIX CHANGES - added verimatrix network node condition
		else if (ne && strstr(ne,"VM") )
		{
			sprintf(str,sms_template, bill_no, account_no, stb_id, amount);
		}		
	}
	else
	{
		//For BB
		pin_strftimet(curr_date, sizeof(curr_date), "%B-%y", *now);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Test 2");
		get_bb_sms_template(ctxp, i_flistp, &sms_template, ebufp);
		if (!sms_template)
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
			goto cleanup;
		}

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, sms_template );
		/*
		 * Prepare Message
		 */
		//sprintf(str, sms_template, account_no, curr_date, amount, due_date);
		sprintf(str, sms_template, curr_date, amount, due_date);
	}

    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );

    /*
     * Push Message in SMS payload
     */
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_MESSAGE, str, ebufp);


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_billing_notification_buf enriched input flist", *ret_flistpp);

cleanup:
    if (sms_template)
    {
        free(sms_template);
    }

    if (osd_template)
    {
        free(osd_template);
    }

    if (bmail_template)
    {
        free(bmail_template);
    }

    PIN_FLIST_DESTROY_EX(&p_svc_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&ai_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&ao_flistp, NULL);
    return;
}

/*******************************************************************
 * prepare_due_reminder_notification_buf()
 *
 * Enrich input filst with service details required
 * optionally performa validation on input fields
 *******************************************************************/
static void
prepare_due_reminder_notification_buf(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **ret_flistpp,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *ai_flistp = NULL;
    pin_flist_t             *ao_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    pin_flist_t             *tmp1_flistp = NULL;
    pin_flist_t             *p_svc_flistp = NULL;
	poid_t					*service_pdp = NULL;
    char                    *ne = NULL;
    char                    *sms_template = NULL;
    char                    *osd_template = NULL;
    char                    *bmail_template = NULL;
    char                    *account_no = NULL;
    char                    str[1024];
    char                    *bill_no = NULL;
    pin_decimal_t           *due_d = NULL;
    double                  due ;
    time_t                  *due_t ;
    struct                  tm *current_time;
    int                     year;
    int                     month;
    int                     day;
    char                    due_date[12];
    int32                   send_osd = PIN_BOOLEAN_FALSE;
    int32                   send_bmail = PIN_BOOLEAN_FALSE;
    void                    *vp = NULL;


    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);


    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_due_reminder_notification_buf input flist", i_flistp);

    /*
     * Validate input received
     */
    bill_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BILL_NO, 0, ebufp);
    due_d = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DUE, 0, ebufp);
    due = pbo_decimal_to_double(due_d, ebufp);
    due_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DUE_T, 0, ebufp);
	//Convert the Due Date to String
	pin_strftimet(due_date, sizeof(due_date), "%d-%b-%y", *due_t);
	/*
     * Read account no
     */
    ai_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ai_flistp, PIN_FLD_POID, ebufp);
    PIN_FLIST_FLD_SET(ai_flistp, PIN_FLD_ACCOUNT_NO, NULL, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_due_reminder_notification_buf PCM_OP_READ_FLDS input flist", ai_flistp);
    PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, ai_flistp, &ao_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_due_reminder_notification_buf PCM_OP_READ_FLDS output flist", ao_flistp);
    account_no = PIN_FLIST_FLD_GET(ao_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);

    service_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);

	if(PIN_POID_IS_NULL(service_pdp) || 
		!(strcmp(PIN_POID_GET_TYPE(service_pdp), "/service/catv")))
	{
		vp = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SEND_OSD, 1, ebufp);
		if (vp)
		{
			send_osd = *(int32 *)vp;
		}

		vp = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SEND_BMAIL, 1, ebufp);
		if (vp)
		{
			send_bmail = *(int32 *)vp;
		}

		if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

		/*
		 *Get Primary CATV connection
		 */
		get_primary_connection_details(ctxp, i_flistp, &p_svc_flistp, ebufp);

		tmp1_flistp = PIN_FLIST_SUBSTR_GET(p_svc_flistp, PIN_FLD_RESULTS, 0, ebufp);
		tmp_flistp = PIN_FLIST_SUBSTR_GET(tmp1_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
		ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

                 /*
                 * Read account no
                 */
                ai_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ai_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(ai_flistp, PIN_FLD_ACCOUNT_NO, NULL, ebufp);
            
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                    "prepare_due_reminder_notification_buf PCM_OP_READ_FLDS input flist", ai_flistp);
                PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, ai_flistp, &ao_flistp, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                    "prepare_due_reminder_notification_buf PCM_OP_READ_FLDS output flist", ao_flistp);
            
                account_no = PIN_FLIST_FLD_GET(ao_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);

		current_time = localtime(due_t);
		year = current_time->tm_year + 1900;
		month = current_time->tm_mon +1;
		day = current_time->tm_mday;
		sprintf(due_date,"%d-%d-%d",day,month,year);

	   if (send_osd == PIN_BOOLEAN_TRUE)
		{
			get_osd_template(ctxp, i_flistp, &osd_template, ne, ebufp);
			if (!osd_template)
			{
				pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
						PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
				goto CLEANUP;
			}
			sprintf(str,osd_template, account_no, due, bill_no, due_date);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );
			fm_mso_send_osd_request(ctxp, p_svc_flistp, str, ne, ebufp);
		}

		if (send_bmail == PIN_BOOLEAN_TRUE)
		{
			get_bmail_template(ctxp, i_flistp, &bmail_template, ne, ebufp);
			if (!bmail_template)
			{
				pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
						PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
				goto CLEANUP;
			}
			sprintf(str,osd_template, account_no, due, bill_no, due_date);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );
			fm_mso_send_bmail_request(ctxp, p_svc_flistp, str, ne, ebufp);
		}

		/*
		 * get action and NE specific SMS template
		 */
		get_sms_template(ctxp, i_flistp, &sms_template, ne, ebufp);
		if (!sms_template)
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
			goto CLEANUP;
		}

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, sms_template );

		/*
		 * Prepare Message
		 */
		if (ne && strstr(ne,"NDS") )
		{
			sprintf(str,sms_template, account_no, due, bill_no, due_date);
		}
		else if (ne && strstr(ne,"CISCO") )
		{
			sprintf(str,sms_template, account_no, due, bill_no, due_date);
		}
		// VERIMATRIX CHANGES - added verimatrix network node condition
		else if (ne && strstr(ne,"VM") )
		{
			sprintf(str,sms_template, account_no, due, bill_no, due_date);
		}		
		else
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
			goto CLEANUP;
		}
	}
	else
	{
		//For BB
		get_bb_sms_template(ctxp, i_flistp, &sms_template, ebufp);
		if (!sms_template)
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
			goto CLEANUP;
		}

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, sms_template );
		/*
		 * Prepare Message
		 */
		sprintf(str, sms_template, account_no, due, due_date);
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );

    /*
     * Push Message in SMS payload
     */
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_MESSAGE, str, ebufp);



    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_due_reminder_notification_buf enriched input flist", *ret_flistpp);

CLEANUP:
    if (sms_template)
    {
        free(sms_template);
    }

    if (osd_template)
    {
        free(osd_template);
    }

    if (bmail_template)
    {
        free(bmail_template);
    }

    PIN_FLIST_DESTROY_EX(&p_svc_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&ai_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&ao_flistp, NULL);
    return;
}

/*******************************************************************
 * prepare_payment_notification_buf()
 *
 * Enrich input filst with service details required
 * optionally performa validation on input fields
 *******************************************************************/
static void
prepare_payment_notification_buf(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **ret_flistpp,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *si_flistp = NULL;
    pin_flist_t             *so_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
	poid_t					*service_pdp = NULL;
    char                    *ne = "NDS";
    char                    *sms_template = NULL;
    char                    *account_no = NULL;
    char                    *stb_id = NULL;
    char                    *vc_id = NULL;
    char                    str[1024];
    char                    *trans_id = NULL;
    pin_decimal_t           *amount_d = NULL;
    double                  amount;
    time_t                  *trans_t ;
    struct                  tm *current_time;
    int                     year;
    int                     month;
    int                     day;
    char                    trans_date[12];
    int32		    *action_flag = NULL;
    char		    *cheque_no = NULL;
	int32					*carrier_id = NULL;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);


    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_payment_notification_buf input flist", i_flistp);

    /*
     * Validate input received
     */
    trans_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_TRANS_ID, 0 , ebufp);
    amount_d = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 0 , ebufp);
    amount = pbo_decimal_to_double(amount_d, ebufp);
    trans_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACK_TIME, 0 , ebufp);
    account_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
	carrier_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CARRIER_ID, 0, ebufp);
	
    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    service_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);

	if(PIN_POID_IS_NULL(service_pdp) || 
		!(strcmp(PIN_POID_GET_TYPE(service_pdp), "/service/catv")))
	{
		/*
		 * GET STB id and Smart card no
		 
		si_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);
		tmp_flistp = PIN_FLIST_SUBSTR_ADD(si_flistp, MSO_FLD_CATV_INFO, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_NETWORK_NODE, NULL, ebufp);
	//    tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, PIN_ELEMID_ANY, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"prepare_payment_notification_buf PCM_OP_READ_FLDS input flist", si_flistp);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"prepare_payment_notification_buf PCM_OP_READ_FLDS output flist", so_flistp);

		tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
		ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); */

		/*
		 * get action and NE specific SMS template
		 */
		get_sms_template(ctxp, i_flistp, &sms_template, ne, ebufp);
		if (!sms_template)
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
			goto CLEANUP;
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, sms_template );

		current_time = localtime(trans_t);
		year = current_time->tm_year + 1900;
		month = current_time->tm_mon +1;
		day = current_time->tm_mday;
		sprintf(trans_date,"%d-%d-%d",day,month,year);


		/*
		 * Prepare Message
		 */
		if (ne && strstr(ne,"NDS") )
		{
	//        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
	//        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

	//        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 1, 0, ebufp);
	//        vc_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

			sprintf(str,sms_template, amount, trans_date, trans_id, account_no);
		}
		else if (ne && strstr(ne,"CISCO") )
		{
	//        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 2, 0, ebufp);
	//        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

			sprintf(str,sms_template, amount, trans_date, trans_id, account_no);
		}
		// VERIMATRIX CHANGES - added verimatrix network node condition
		else if (ne && strstr(ne,"VM") )
		{
			sprintf(str,sms_template, amount, trans_date, trans_id, account_no);
		}		
		else
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
			goto CLEANUP;
		}
	}
	else
	{
		//For BB
		pin_strftimet(trans_date, sizeof(trans_date), "%d-%b-%y", *trans_t);
		get_bb_sms_template(ctxp, i_flistp, &sms_template, ebufp);
		if (!sms_template)
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
			goto CLEANUP;
		}

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, sms_template );
		action_flag = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 0, ebufp);
		if(*action_flag == NTF_BB_PAYMENT_CHEQUE) {
                    cheque_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CHECK_NO, 0 , ebufp);
                   // sprintf(str, sms_template,  amount, trans_date, trans_id, cheque_no);
		      sprintf(str, sms_template, amount, cheque_no, trans_id);
                }
		else if (*action_flag == NTF_BB_PAYMENT_ONLINE) {
			account_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
			sprintf(str, sms_template, amount, account_no, trans_id);
		}
		else if (*action_flag == NTF_BB_PAYMENT_CASH ){
			sprintf(str, sms_template, amount, trans_id);
		}
		else {
                  	sprintf(str, sms_template,  amount, trans_date, trans_id);
			//sprintf(str, sms_template,  amount, trans_id);
                }
		/*
		 * Prepare Message
		 */
		//sprintf(str, sms_template,  amount, trans_date, trans_id);
	}
		   PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );

    /*
     * Push Message in SMS payload
     */
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_MESSAGE, str, ebufp);

	// Setting the Service Provider Identification for the SMS Notification
	PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_CARRIER_ID, carrier_id, ebufp);
	
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_payment_notification_buf enriched input flist", *ret_flistpp);

CLEANUP:
    if (sms_template)
    {
        free(sms_template);
    }

    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
    return;
}

/*******************************************************************
 * prepare_payment_reversal_notification_buf()
 *
 * Enrich input filst with service details required
 * optionally performa validation on input fields
 *******************************************************************/
static void
prepare_payment_reversal_notification_buf(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **ret_flistpp,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *si_flistp = NULL;
    pin_flist_t             *so_flistp = NULL;
    pin_flist_t             *ai_flistp = NULL;
    pin_flist_t             *ao_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    poid_t		    *service_pdp = NULL;
    char                    *ne = NULL;
    char                    *sms_template = NULL;
    char                    *account_no = NULL;
    char                    *stb_id = NULL;
    char                    *vc_id = NULL;
    char                    str[1024];
    char                    *trans_id = NULL;
    pin_decimal_t           *amount_d = NULL;
    double                  amount;
    time_t                  *trans_t ;
    struct                  tm *current_time;
    int                     year;
    int                     month;
    int                     day;
    char                    trans_date[12];

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);


    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_payment_reversal_notification_buf input flist", i_flistp);

    /*
     * Validate input received
     */
    trans_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_TRANS_ID, 0 , ebufp);
    amount_d = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 0 , ebufp);
    amount = pbo_decimal_to_double(amount_d, ebufp);
    trans_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACK_TIME, 0 , ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    service_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);

	if(PIN_POID_IS_NULL(service_pdp) || 
		!(strcmp(PIN_POID_GET_TYPE(service_pdp), "/service/catv")))
	{
		/*
		 * GET STB id and Smart card no
		 */
		si_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);
		tmp_flistp = PIN_FLIST_SUBSTR_ADD(si_flistp, MSO_FLD_CATV_INFO, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_NETWORK_NODE, NULL, ebufp);
	//    tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, PIN_ELEMID_ANY, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"prepare_payment_reversal_notification_buf PCM_OP_READ_FLDS input flist", si_flistp);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"prepare_payment_reversal_notification_buf PCM_OP_READ_FLDS output flist", so_flistp);

		tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
		ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 

		/*
		 * get action and NE specific SMS template
		 */
		get_sms_template(ctxp, i_flistp, &sms_template, ne, ebufp);
		if (!sms_template)
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
			goto CLEANUP;
		}

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, sms_template );

		/*
		 * Read account no
		 */
		ai_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ai_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(ai_flistp, PIN_FLD_ACCOUNT_NO, NULL, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"prepare_payment_reversal_notification_buf PCM_OP_READ_FLDS input flist", ai_flistp);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, ai_flistp, &ao_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"prepare_payment_reversal_notification_buf PCM_OP_READ_FLDS output flist", ao_flistp);

		account_no = PIN_FLIST_FLD_GET(ao_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);

		current_time = localtime(trans_t);
		year = current_time->tm_year + 1900;
		month = current_time->tm_mon +1;
		day = current_time->tm_mday;
		sprintf(trans_date,"%d-%d-%d",day,month,year);

		/*
		 * Prepare Message
		 */
		if (ne && strstr(ne,"NDS") )
		{
	//        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
	//        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

	//        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 1, 0, ebufp);
	//        vc_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

			sprintf(str,sms_template, amount, trans_date, trans_id, account_no);
		}
		else if (ne && strstr(ne,"CISCO") )
		{
	//        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 2, 0, ebufp);
	//        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

			sprintf(str,sms_template, amount, trans_date, trans_id, account_no);
		}
		// VERIMATRIX CHANGES - added verimatrix network node condition
		else if (ne && strstr(ne,"VM") )
		{
			sprintf(str,sms_template, amount, trans_date, trans_id, account_no);
		}		
		else
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
			goto CLEANUP;
		}
	}
	else
	{
		//For BB
		pin_strftimet(trans_date, sizeof(trans_date), "%d-%b-%y", *trans_t);
		get_bb_sms_template(ctxp, i_flistp, &sms_template, ebufp);
		if (!sms_template)
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
			goto CLEANUP;
		}

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, sms_template );
		/*
		 * Prepare Message
		 */
		sprintf(str, sms_template,  amount, trans_date, trans_id);
	}
    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );

    /*
     * Push Message in SMS payload
     */
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_MESSAGE, str, ebufp);


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_payment_reversal_notification_buf enriched input flist", *ret_flistpp);

CLEANUP:
    if (sms_template)
    {
        free(sms_template);
    }

    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&ai_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&ao_flistp, NULL);
    return;
}

/*****************************************************************
* prepare_change_passwd_notification_buf()
*
* Enrich the input flist with password change details
******************************************************************/
static void
prepare_change_passwd_notification_buf(
	pcm_context_t			*ctxp, 
	pin_flist_t			*i_flistp, 
	pin_flist_t			**payload_flistp, 
	pin_errbuf_t			*ebufp)
{
	char				*passwd = NULL;
	char                    	*sms_template = NULL;
	char                    	str[1024];
	if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);


    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_change_passwd_notification_buf input flist", i_flistp);
	/* Pawan:16-02-2015: Commented below line and added next line to fetch the
		password from MSO_FLD_PASSWORD */
	//passwd = (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PASSWD, 0, ebufp);
	passwd = (char *)PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_PASSWORD, 0, ebufp);
	get_bb_sms_template(ctxp, i_flistp, &sms_template, ebufp);
	if (!sms_template)
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
		goto CLEANUP;
	}

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, sms_template );
	/*
	 * Prepare Message
	 */
	sprintf(str, sms_template, passwd);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );
    /*
     * Push Message in EMAIL payload
     */
    PIN_FLIST_FLD_SET(*payload_flistp, PIN_FLD_MESSAGE, str, ebufp);
    //PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_SUBJECT, "Password Change Successful", ebufp);


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_change_passwd_notification_buf enriched input flist", *payload_flistp);

CLEANUP:
    if (sms_template)
    {
        free(sms_template);
    }
	return;
}

/*****************************************************************
* prepare_before_fup_notification_buf()
*
* Enrich the input flist with FUP threshold breach
******************************************************************/
static void
prepare_before_fup_notification_buf(
	pcm_context_t			*ctxp, 
	pin_flist_t			*i_flistp, 
	pin_flist_t			**payload_flistp, 
	pin_errbuf_t			*ebufp)
{
	pin_flist_t		*ai_flistp = NULL;
	pin_flist_t		*ao_flistp = NULL;
	pin_flist_t		*nameinfo_flistp = NULL;
	pin_flist_t		*helpdesk_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	char			*acc_no = NULL;
	pin_decimal_t		*threshold = NULL;
	double			amount;
	char                    *sms_template = NULL;
	char                    str[1024];
	char			*city = NULL;
	char			*help_phone = NULL;
	if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);


    /***********************************************************
     * Log input flist
     ***********************************************************/
    	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        	"prepare_before_fup_notification_buf input flist", i_flistp);
	threshold = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PERCENT, 0 , ebufp);
    	amount = pbo_decimal_to_double(threshold, ebufp);

	//Get the Account No
	ai_flistp = PIN_FLIST_CREATE(ebufp);
    	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ai_flistp, PIN_FLD_POID, ebufp);
    	PIN_FLIST_FLD_SET(ai_flistp, PIN_FLD_ACCOUNT_NO, NULL, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(ai_flistp, PIN_FLD_NAMEINFO, 2, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_CITY, NULL, ebufp);

    	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        	"prepare_before_fup_notification_buf PCM_OP_READ_FLDS input flist", ai_flistp);
    	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, ai_flistp, &ao_flistp, ebufp);
    	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        	"prepare_before_fup_notification_buf PCM_OP_READ_FLDS output flist", ao_flistp);
    	acc_no = PIN_FLIST_FLD_GET(ao_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
	nameinfo_flistp = PIN_FLIST_ELEM_GET(ao_flistp, PIN_FLD_NAMEINFO, 2, 1, ebufp);
	city = PIN_FLIST_FLD_GET(nameinfo_flistp, PIN_FLD_CITY, 0, ebufp);
	if (city)
	{
		get_helpdesk_details(ctxp, i_flistp, city, &helpdesk_flistp, ebufp);
	        if(helpdesk_flistp)
       		{
               		help_phone = PIN_FLIST_FLD_GET(helpdesk_flistp, PIN_FLD_PHONE, 1, ebufp);
		}
	}

	get_bb_sms_template(ctxp, i_flistp, &sms_template, ebufp);
	if (!sms_template)
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
		goto CLEANUP;
	}

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, sms_template );
	/*
	 * Prepare Message
	 */
	sprintf(str, sms_template, acc_no, amount, help_phone);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );
    /*
     * Push Message in EMAIL payload
     */
    	PIN_FLIST_FLD_SET(*payload_flistp, PIN_FLD_MESSAGE, str, ebufp);


    	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_before_fup_notification_buf enriched input flist", *payload_flistp);

CLEANUP:
    if (sms_template)
    {
        free(sms_template);
    }
	return;
}

/*****************************************************************
* prepare_after_fup_notification_buf()
*
* Enrich the input flist with post FUP
******************************************************************/
static void
prepare_after_fup_notification_buf(
	pcm_context_t		*ctxp, 
	pin_flist_t		*i_flistp, 
	pin_flist_t		**payload_flistp, 
	pin_errbuf_t		*ebufp)
{
        pin_flist_t             *ai_flistp = NULL;
        pin_flist_t             *ao_flistp = NULL;
        pin_flist_t             *nameinfo_flistp = NULL;
	pin_flist_t		*out_flistp = NULL;
        //pin_flist_t           *helpdesk_flistp = NULL;
        pin_flist_t             *tmp_flistp = NULL;
        char                    *acc_no = NULL;
        char                    *sms_template = NULL;
        char                    str[1024];
        char                    *city = NULL;
	//Added for new notification 
	pin_flist_t		*validity_flistp = NULL;
	pin_decimal_t           *one_mb = pbo_decimal_from_str("1024", ebufp);
        pin_decimal_t           *dl_speed = NULL;
        pin_decimal_t           *fup_dl_speed = NULL;
        pin_decimal_t           *speed_mb = NULL;
        pin_decimal_t           *fup_speed_mb = NULL;
        char                    *speed = NULL;
        char                    *fup_speed = NULL;
        poid_t                  *srvc_pdp = NULL;
	time_t			*eff_t	= NULL;
        char                    due_date[12];
	//char                  *help_phone = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);


    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_after_fup_notification_buf input flist", i_flistp);

	srvc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);		
	//Get the Account No
	ai_flistp = PIN_FLIST_CREATE(ebufp);
   	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ai_flistp, PIN_FLD_POID, ebufp);
    	PIN_FLIST_FLD_SET(ai_flistp, PIN_FLD_ACCOUNT_NO, NULL, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(ai_flistp, PIN_FLD_NAMEINFO, 2, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_CITY, NULL, ebufp);


        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_after_fup_notification_buf PCM_OP_READ_FLDS input flist", ai_flistp);
        PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, ai_flistp, &ao_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_after_fup_notification_buf PCM_OP_READ_FLDS output flist", ao_flistp);
        acc_no = PIN_FLIST_FLD_GET(ao_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
        /*nameinfo_flistp = PIN_FLIST_ELEM_GET(ao_flistp, PIN_FLD_NAMEINFO, 2, 1, ebufp);
        city = PIN_FLIST_FLD_GET(nameinfo_flistp, PIN_FLD_CITY, 0, ebufp);
        if (city)
        {
                get_helpdesk_details(ctxp, i_flistp, city, &helpdesk_flistp, ebufp);
                if(helpdesk_flistp)
                {
                        help_phone = PIN_FLIST_FLD_GET(helpdesk_flistp, PIN_FLD_PHONE, 1, ebufp);
                }
        }*/
        get_bb_sms_template(ctxp, i_flistp, &sms_template, ebufp);
        if (!sms_template)
        {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                                PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
                goto CLEANUP;
        }

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, sms_template );
	fm_mso_get_dl_speed(ctxp, i_flistp, &out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(3, "Speed output flist", out_flistp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_MSG(1, "Error during fetching DL speed");
                goto CLEANUP;
        }
        dl_speed = PIN_FLIST_FLD_GET(out_flistp, MSO_FLD_DL_SPEED, 0, ebufp);
        speed_mb = pbo_decimal_divide(dl_speed, one_mb, ebufp);
        pbo_decimal_round_assign(speed_mb, 2, ROUND_HALF_UP, ebufp);
        speed = pbo_decimal_to_str(speed_mb, ebufp);

        fup_dl_speed = PIN_FLIST_FLD_GET(out_flistp, MSO_FLD_FUP_DL_SPEED, 0, ebufp);
        fup_speed_mb = pbo_decimal_divide(fup_dl_speed, one_mb, ebufp);
        pbo_decimal_round_assign(fup_speed_mb, 2, ROUND_HALF_UP, ebufp);
        fup_speed = pbo_decimal_to_str(fup_speed_mb, ebufp);


        PIN_FLIST_FLD_PUT(out_flistp, PIN_FLD_NAME, speed, ebufp);
        PIN_ERR_LOG_FLIST(3, "Speed output flist", out_flistp);
		validity_flistp = PIN_FLIST_CREATE(ebufp);
        fm_mso_rate_aaa_get_charged_dates(ctxp, srvc_pdp, &validity_flistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_MSG(1, "Error during fetching DL speed");
                goto CLEANUP;
        }
        PIN_ERR_LOG_FLIST(3, "validity output flist", validity_flistp);
		eff_t = PIN_FLIST_FLD_GET(validity_flistp, PIN_FLD_CHARGED_TO_T, 0, ebufp);	
		pin_strftimet(due_date, sizeof(due_date), "%d-%b-%y", *eff_t);	
        /*
         * Prepare Message
         */
        //sprintf(str, sms_template, acc_no, help_phone);
		sprintf(str, sms_template, acc_no, fup_speed, speed, due_date);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );
    /*
     * Push Message in EMAIL payload
     */
    PIN_FLIST_FLD_SET(*payload_flistp, PIN_FLD_MESSAGE, str, ebufp);


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_after_fup_notification_buf enriched input flist", *payload_flistp);

CLEANUP:
    if (sms_template)
    {
        free(sms_template);
    }
	PIN_FLIST_DESTROY_EX(&out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&validity_flistp, NULL);
	return;
}

/*****************************************************************
* prepare_before_expiry_notification_buf()
*
* Enrich the input flist with expiry details
******************************************************************/
static void
prepare_before_expiry_notification_buf(
	pcm_context_t			*ctxp, 
	pin_flist_t			*i_flistp, 
	pin_flist_t			**payload_flistp, 
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*ai_flistp = NULL;
	pin_flist_t			*ao_flistp = NULL;
	char				*acc_no = NULL;
	int				*threshold = NULL;
	char                    	*sms_template = NULL;
	time_t				*expiry = NULL;
	char				expr[12] = {""};
	char                    	str[1024] = {""};

	if (PIN_ERRBUF_IS_ERR(ebufp))
        	return;

    	PIN_ERRBUF_CLEAR(ebufp);

    	/***********************************************************
    	 * Log input flist
     	 ***********************************************************/
    	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        	"prepare_before_expiry_notification_buf input flist", i_flistp);
	threshold = (int *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DUE_DAY, 0 , ebufp);
	expiry = (time_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_EXPIRATION_T, 0 , ebufp);
	pin_strftimet(expr, sizeof(expr), "%d-%b-%Y", *expiry);

	//Get the Account No
	ai_flistp = PIN_FLIST_CREATE(ebufp);
    	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ai_flistp, PIN_FLD_POID, ebufp);
    	PIN_FLIST_FLD_SET(ai_flistp, PIN_FLD_ACCOUNT_NO, NULL, ebufp);

    	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        	"prepare_before_expiry_notification_buf PCM_OP_READ_FLDS input flist", ai_flistp);
    	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, ai_flistp, &ao_flistp, ebufp);
    	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        	"prepare_before_expiry_notification_buf PCM_OP_READ_FLDS output flist", ao_flistp);
    	acc_no = PIN_FLIST_FLD_GET(ao_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);

	get_bb_sms_template(ctxp, i_flistp, &sms_template, ebufp);
	if (!sms_template)
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
		goto CLEANUP;
	}

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, sms_template );
	/*
	 * Prepare Message
	 */
	sprintf(str, sms_template, acc_no, *threshold, expr);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );
    	/*
     	 * Push Message in EMAIL payload
     	 */
    	PIN_FLIST_FLD_SET(*payload_flistp, PIN_FLD_MESSAGE, str, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_before_expiry_notification_buf enriched input flist", *payload_flistp);

CLEANUP:
    	if (sms_template)
    	{
        	free(sms_template);
    	}
	return;
}

/*******************************************************************
 * prepare_service_activation_notification_buf()
 *
 * Enrich input filst with service details required
 * optionally performa validation on input fields
 *******************************************************************/
static void
prepare_deposit_refund_notification_buf(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **ret_flistpp,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *si_flistp = NULL;
    pin_flist_t             *so_flistp = NULL;
    pin_flist_t             *ai_flistp = NULL;
    pin_flist_t             *ao_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    char                    *ne = NULL;
    char                    *sms_template = NULL;
    char                    *account_no = NULL;
    char                    *stb_id = NULL;
    char                    *vc_id = NULL;
    char                    str[1024];
    char                    *refund_id = NULL;
    pin_decimal_t           *amount_d = NULL;
    double                  amount;
    time_t                  *trans_t ;
    struct                  tm *current_time;
    int                     year;
    int                     month;
    int                     day;
    char                    trans_date[12];

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);


    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_payment_reversal_notification_buf input flist", i_flistp);

    /*
     * Validate input received
     */
    refund_id = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_REFUND_NO, 0 , ebufp);
    amount_d = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 0 , ebufp);
    amount = pbo_decimal_to_double(amount_d, ebufp);
    trans_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACK_TIME, 0 , ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    /*
     * GET STB id and Smart card no
     */
    si_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);
    tmp_flistp = PIN_FLIST_SUBSTR_ADD(si_flistp, MSO_FLD_CATV_INFO, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_NETWORK_NODE, NULL, ebufp);
//    tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, PIN_ELEMID_ANY, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_deposit_refund_notification_buf PCM_OP_READ_FLDS input flist", si_flistp);
    PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_deposit_refund_notification_buf PCM_OP_READ_FLDS output flist", so_flistp);

    tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
    ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 

    /*
     * get action and NE specific SMS template
     */
    get_sms_template(ctxp, i_flistp, &sms_template, ne, ebufp);
    if (!sms_template)
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
        goto CLEANUP;
    }

    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, sms_template );

    /*
     * Read account no
     */
    ai_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ai_flistp, PIN_FLD_POID, ebufp);
    PIN_FLIST_FLD_SET(ai_flistp, PIN_FLD_ACCOUNT_NO, NULL, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_deposit_refund_notification_buf PCM_OP_READ_FLDS input flist", ai_flistp);
    PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, ai_flistp, &ao_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_deposit_refund_notification_buf PCM_OP_READ_FLDS output flist", ao_flistp);

    account_no = PIN_FLIST_FLD_GET(ao_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);

    current_time = localtime(trans_t);
    year = current_time->tm_year + 1900;
    month = current_time->tm_mon +1;
    day = current_time->tm_mday;
    sprintf(trans_date,"%d-%d-%d",day,month,year);

    /*
     * Prepare Message
     */
    if (ne && strstr(ne,"NDS") )
    {
//        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
//        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

//        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 1, 0, ebufp);
//        vc_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        sprintf(str,sms_template, amount, trans_date, refund_id, account_no);
    }
    else if (ne && strstr(ne,"CISCO") )
    {
//        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 2, 0, ebufp);
//        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        sprintf(str,sms_template, amount, trans_date, refund_id, account_no);
    }
	// VERIMATRIX CHANGES - added verimatrix network node condition
    else if (ne && strstr(ne,"VM") )
    {
        sprintf(str,sms_template, amount, trans_date, refund_id, account_no);
    }	
    else
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
        goto CLEANUP;
    }

    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );

    /*
     * Push Message in SMS payload
     */
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_MESSAGE, str, ebufp);


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_deposit_refund_notification_buf enriched input flist", *ret_flistpp);

CLEANUP:
    if (sms_template)
    {
        free(sms_template);
    }

    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&ai_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&ao_flistp, NULL);
    return;
}

/*******************************************************************
 * prepare_cheque_reversal_notification_buf()
 *
 * Enrich input filst with service details required
 * optionally performa validation on input fields
 *******************************************************************/
static void
prepare_cheque_reversal_notification_buf(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **ret_flistpp,
    pin_errbuf_t            *ebufp)
{
	pin_flist_t             *si_flistp = NULL;
	pin_flist_t             *so_flistp = NULL;
	pin_flist_t             *ai_flistp = NULL;
	pin_flist_t             *ao_flistp = NULL;
	pin_flist_t             *tmp_flistp = NULL;
	poid_t			*service_pdp = NULL;
	char                    *ne = NULL;
	char                    *sms_template = NULL;
	char                    *account_no = NULL;
	char                    *stb_id = NULL;
	char                    *vc_id = NULL;
	char                    str[1024];
	char                    *cheque_no = NULL;
	pin_decimal_t           *amount_d = NULL;
	double                  amount;
	time_t                  *trans_t ;
	struct                  tm *current_time;
	int                     year;
	int                     month;
	int                     day;
	char                    trans_date[12];
	char			*bank_name = NULL;

    	if (PIN_ERRBUF_IS_ERR(ebufp))
        	return;

    	PIN_ERRBUF_CLEAR(ebufp);

    	/***********************************************************
     	 * Log input flist
     	 ***********************************************************/
    	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        	"prepare_cheque_reversal_notification_buf input flist", i_flistp);

    	/*
     	 * Validate input received
     	 */
    	cheque_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CHECK_NO, 0 , ebufp);
    	amount_d = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 0 , ebufp);
    	amount = pbo_decimal_to_double(amount_d, ebufp);
    	trans_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_EFFECTIVE_T, 0 , ebufp);

    	if (PIN_ERRBUF_IS_ERR(ebufp))
        	return;

	service_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
	if(PIN_POID_IS_NULL(service_pdp) ||!(strcmp(PIN_POID_GET_TYPE(service_pdp), "/service/catv")))
	{
    		/*
     		 * GET STB id and Smart card no
     		 */
    		si_flistp = PIN_FLIST_CREATE(ebufp);
    		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);
    		tmp_flistp = PIN_FLIST_SUBSTR_ADD(si_flistp, MSO_FLD_CATV_INFO, ebufp);
    		PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_NETWORK_NODE, NULL, ebufp);
//    		tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, PIN_ELEMID_ANY, ebufp);

    		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        		"prepare_cheque_reversal_notification_buf PCM_OP_READ_FLDS input flist", si_flistp);
    		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
    		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        		"prepare_cheque_reversal_notification_buf PCM_OP_READ_FLDS output flist", so_flistp);

    		tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
   		ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 

    		/*
     		 * get action and NE specific SMS template
     		 */
    		get_sms_template(ctxp, i_flistp, &sms_template, ne, ebufp);
    		if (!sms_template)
    		{
        		pin_set_err(ebufp, PIN_ERRLOC_FM,
            		PIN_ERRCLASS_SYSTEM_DETERMINATE,
                		PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
        		goto CLEANUP;
    		}

    		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, sms_template );

    		/*
     		 * Read account no
     		 */
    		ai_flistp = PIN_FLIST_CREATE(ebufp);
    		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ai_flistp, PIN_FLD_POID, ebufp);
    		PIN_FLIST_FLD_SET(ai_flistp, PIN_FLD_ACCOUNT_NO, NULL, ebufp);

    		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        		"prepare_cheque_reversal_notification_buf PCM_OP_READ_FLDS input flist", ai_flistp);
    		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, ai_flistp, &ao_flistp, ebufp);
    		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        		"prepare_cheque_reversal_notification_buf PCM_OP_READ_FLDS output flist", ao_flistp);

    		account_no = PIN_FLIST_FLD_GET(ao_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);

    		current_time = localtime(trans_t);
    		year = current_time->tm_year + 1900;
    		month = current_time->tm_mon +1;
    		day = current_time->tm_mday;
    		sprintf(trans_date,"%d-%d-%d",day,month,year);

    		/*
     		 * Prepare Message
     		 */
    		if (ne && strstr(ne,"NDS") )
    		{
//        		tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
//        		stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

//        		tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 1, 0, ebufp);
//        		vc_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        		sprintf(str,sms_template, cheque_no, trans_date, amount);
    		}
    		else if (ne && strstr(ne,"CISCO") )
    		{
//        		tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 2, 0, ebufp);
//        		stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        		sprintf(str,sms_template, cheque_no, trans_date, amount);
    		}
		// VERIMATRIX CHANGES - added verimatrix network node condition
    		else if (ne && strstr(ne,"VM") )
    		{
        		sprintf(str,sms_template, cheque_no, trans_date, amount);
    		}	
    		else
    		{
        		pin_set_err(ebufp, PIN_ERRLOC_FM,
            				PIN_ERRCLASS_SYSTEM_DETERMINATE,
                			PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
        		goto CLEANUP;
    		}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Payment reversal is for BB");
		pin_strftimet(trans_date, sizeof(trans_date), "%d-%b-%y", *trans_t);
		get_bb_sms_template(ctxp, i_flistp, &sms_template, ebufp);
		bank_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BANK_NAME, 0, ebufp);
		sprintf(str,sms_template, cheque_no, bank_name, trans_date, amount);	
	}

    	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );

    	/*
     	 * Push Message in SMS payload
     	 */
    	PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_MESSAGE, str, ebufp);

    	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_cheque_reversal_notification_buf enriched input flist", *ret_flistpp);

CLEANUP:
    if (sms_template)
    {
        free(sms_template);
    }

    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&ai_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&ao_flistp, NULL);
    return;
}


void
fm_mso_send_osd_request(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    char                    *str,
    char                    *ne,
    pin_errbuf_t            *ebufp)
{

    pin_flist_t             *prov_i_flistp = NULL;
    pin_flist_t             *prov_o_flistp = NULL;
    int32                   mailbox_code = 1;
    int32                   duration = 20;
    poid_t                  *pdp = NULL;
    poid_t                  *a_pdp = NULL;
    poid_t                  *s_pdp = NULL;
    int32                   osd_number = 253;
    int64                   database;
    pin_flist_t             *tmp_flistp = NULL;
    pin_flist_t             *tmp1_flistp = NULL;
    char                    *stb_id = NULL;
    char                    *cas_subs_id = NULL;
    int32                   prov_flag = CATV_SEND_OSD;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);


    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_send_osd_request input flist", i_flistp);

    prov_i_flistp = PIN_FLIST_CREATE(ebufp);
    pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
    tmp1_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_RESULTS, 0, ebufp);
    database = PIN_POID_GET_DB(pdp);

    a_pdp = (poid_t *)PIN_POID_CREATE(database, "/account", 1, ebufp);
    s_pdp = (poid_t *)PIN_POID_CREATE(database, "/service/catv", -1, ebufp);
    PIN_FLIST_FLD_PUT(prov_i_flistp, PIN_FLD_POID, (void *)a_pdp, ebufp);
    PIN_FLIST_FLD_PUT(prov_i_flistp, PIN_FLD_SERVICE_OBJ, (void *)s_pdp, ebufp);
    PIN_FLIST_FLD_SET(prov_i_flistp, PIN_FLD_FLAGS, &prov_flag, ebufp);
    PIN_FLIST_FLD_SET(prov_i_flistp, MSO_FLD_NETWORK_NODE, ne, ebufp);
    
    if (ne && strstr(ne,"NDS") )
    {
        tmp_flistp = PIN_FLIST_ELEM_GET(tmp1_flistp, MSO_FLD_CATV_INFO, 0, 0, ebufp);
        cas_subs_id = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, 0, ebufp);
        PIN_FLIST_FLD_SET(prov_i_flistp, PIN_FLD_ADDRESS, cas_subs_id, ebufp);
        PIN_FLIST_FLD_SET(prov_i_flistp, PIN_FLD_MESSAGE, str, ebufp);
        PIN_FLIST_FLD_SET(prov_i_flistp, PIN_FLD_DURATION_SECONDS, &duration, ebufp);
        PIN_FLIST_FLD_SET(prov_i_flistp, MSO_FLD_OSD_NUMBER, &osd_number, ebufp);
    }
        else if (ne && strstr(ne,"CISCO") )
    {
        tmp_flistp = PIN_FLIST_ELEM_GET(tmp1_flistp, PIN_FLD_ALIAS_LIST, 2, 0, ebufp);
        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(prov_i_flistp, MSO_FLD_STB_ID_LIST, 0, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_STB_ID, stb_id, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(prov_i_flistp, MSO_FLD_MSG_CONTENT_LIST, 0, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_MESSAGE, str, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(prov_i_flistp, MSO_FLD_MSG_TITLE_LIST, 0, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_TITLE, "Payment Due Notification", ebufp);
    }
	// VERIMATRIX CHANGES - BEGIN - added verimatrix network node condition
	else if (ne && strstr(ne,"VM") )
    {
        tmp_flistp = PIN_FLIST_ELEM_GET(tmp1_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(prov_i_flistp, MSO_FLD_STB_ID_LIST, 0, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_STB_ID, stb_id, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(prov_i_flistp, MSO_FLD_MSG_CONTENT_LIST, 0, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_MESSAGE, str, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(prov_i_flistp, MSO_FLD_MSG_TITLE_LIST, 0, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_TITLE, "Payment Due Notification", ebufp);
    }	
	// VERIMATRIX CHANGES - END - added verimatrix network node condition
    else
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
        goto CLEANUP;
    }

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_send_osd_request notification input flist", prov_i_flistp);
    PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION, 0, prov_i_flistp, &prov_o_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_send_osd_request notification output flist", prov_i_flistp);
    
CLEANUP:
    PIN_FLIST_DESTROY_EX(&prov_i_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&prov_o_flistp, NULL);

    return;
}


void
fm_mso_send_bmail_request(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    char                    *str,
    char                    *ne,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *prov_i_flistp = NULL;
    pin_flist_t             *prov_o_flistp = NULL;
    int32                   mailbox_code = 1;
    poid_t                  *pdp = NULL;
    poid_t                  *a_pdp = NULL;
    poid_t                  *s_pdp = NULL;
    int64                   database;
    pin_flist_t             *tmp_flistp = NULL;
    pin_flist_t             *tmp1_flistp = NULL;
    char                    *stb_id = NULL;
    char                    *cas_subs_id = NULL;
    char 		    *subject = BMAIL_SUBJECT;
    int32                   prov_flag = CATV_EMAIL;
    time_t                  current_t;
    time_t                  next_t;
    struct                  tm *current_time;
    int                     year;
    int                     month;
    int                     day;
    char                    validity[10];
    char                    day_s[2];
    char                    month_s[2];


    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);


    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
       "fm_mso_send_bmail_request input flist", i_flistp);

    prov_i_flistp = PIN_FLIST_CREATE(ebufp);
    pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
    tmp1_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_RESULTS, 0, ebufp);
    database = PIN_POID_GET_DB(pdp);
    a_pdp = (poid_t *)PIN_POID_CREATE(database, "/account", 1, ebufp);
    s_pdp = (poid_t *)PIN_POID_CREATE(database, "/service/catv", -1, ebufp);
    PIN_FLIST_FLD_PUT(prov_i_flistp, PIN_FLD_POID, (void *)a_pdp, ebufp);
    PIN_FLIST_FLD_PUT(prov_i_flistp, PIN_FLD_SERVICE_OBJ, (void *)s_pdp, ebufp);
    PIN_FLIST_FLD_SET(prov_i_flistp, PIN_FLD_FLAGS, &prov_flag, ebufp);
    PIN_FLIST_FLD_SET(prov_i_flistp, MSO_FLD_NETWORK_NODE, ne, ebufp);
    
    if (ne && strstr(ne,"NDS") )
    {
        current_t = pin_virtual_time(NULL);
        next_t = current_t + 86400;
        current_time = localtime(&next_t);
        year = current_time->tm_year + 1900;
        month = current_time->tm_mon +1;
        day = current_time->tm_mday;

        if (day < 10)
        {
            if (month < 10)
            {
                sprintf(validity,"%d0%d0%d",year,month,day); 
            }
            else
            {
                sprintf(validity,"%d%d0%d",year,month,day);
            }
        }
        else
        {
            if (month < 10)
            {
                sprintf(validity,"%d0%d%d",year,month,day); 
            }
            else
            {
                sprintf(validity,"%d%d%d",year,month,day); 
            }
        }

        tmp_flistp = PIN_FLIST_ELEM_GET(tmp1_flistp, MSO_FLD_CATV_INFO, 0, 0, ebufp);
        cas_subs_id = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, 0, ebufp);
        PIN_FLIST_FLD_SET(prov_i_flistp, PIN_FLD_ADDRESS, cas_subs_id, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(prov_i_flistp, PIN_FLD_MESSAGES, 0, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_MESSAGE, str, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_SUBJECT, subject, ebufp);
        PIN_FLIST_FLD_SET(prov_i_flistp, MSO_FLD_EMMG_DELETION_DATE, validity, ebufp);
        PIN_FLIST_FLD_SET(prov_i_flistp, MSO_FLD_STB_DELETE_DATE, validity, ebufp);
        PIN_FLIST_FLD_SET(prov_i_flistp, MSO_FLD_CODE_FOR_MAILBOX, &mailbox_code, ebufp);
    }
    else if (ne && strstr(ne,"CISCO") )
    {
        tmp_flistp = PIN_FLIST_ELEM_GET(tmp1_flistp, PIN_FLD_ALIAS_LIST, 2, 0, ebufp);
        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(prov_i_flistp, MSO_FLD_STB_ID_LIST, 0, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_STB_ID, stb_id, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(prov_i_flistp, MSO_FLD_MSG_CONTENT_LIST, 0, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_MESSAGE, str, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(prov_i_flistp, MSO_FLD_MSG_TITLE_LIST, 0, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_TITLE, "Payment Due Notification", ebufp);
    }
	// VERIMATRIX CHANGES - BEGIN - added verimatrix network node condition
    else if (ne && strstr(ne,"VM") )
    {
        tmp_flistp = PIN_FLIST_ELEM_GET(tmp1_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(prov_i_flistp, MSO_FLD_STB_ID_LIST, 0, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_STB_ID, stb_id, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(prov_i_flistp, MSO_FLD_MSG_CONTENT_LIST, 0, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_MESSAGE, str, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(prov_i_flistp, MSO_FLD_MSG_TITLE_LIST, 0, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_TITLE, "Payment Due Notification", ebufp);
    }	
	// VERIMATRIX CHANGES - END - added verimatrix network node condition
    else
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
        goto CLEANUP;
    }

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_send_bmail_request notification input flist", prov_i_flistp);
    PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION, 0, prov_i_flistp, &prov_o_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_send_bmail_request notification output flist", prov_i_flistp);
    
CLEANUP:
    PIN_FLIST_DESTROY_EX(&prov_i_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&prov_o_flistp, NULL);

    return;
}

void
get_primary_connection_details(
        pcm_context_t       *ctxp,
        pin_flist_t         *i_flistp,
        pin_flist_t         **ret_flistpp,
        pin_errbuf_t        *ebufp)
{

    pin_flist_t             *search_i_flistp = NULL;
//    pin_flist_t           *search_o_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    pin_flist_t             *tmp1_flistp = NULL;
    poid_t                  *s_pdp = NULL;
    poid_t                  *pdp = NULL;
    char                    *template =  NULL;
    int32                   s_flags = 0;
    int64                   database = 0;
    int32                   primary = 0;


    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "get_primary_connection_details input flist", i_flistp);

    /*
     * Validate input received
     */

    /*
     * Search Primary Service
     */
    pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
    database = PIN_POID_GET_DB(pdp); 

    search_i_flistp = PIN_FLIST_CREATE(ebufp);
    s_pdp = (poid_t *)PIN_POID_CREATE(database, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(search_i_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

    s_flags = SRCH_DISTINCT ;
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

    template = "select X from /service/catv where F1 = V1 and F2 = V2 ";
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, pdp, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
    tmp1_flistp = PIN_FLIST_SUBSTR_ADD(tmp_flistp, MSO_FLD_CATV_INFO, ebufp); //CISCO - Mac adddress
    PIN_FLIST_FLD_SET(tmp1_flistp, PIN_FLD_CONN_TYPE, &primary, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
    tmp1_flistp = PIN_FLIST_SUBSTR_ADD(tmp_flistp, MSO_FLD_CATV_INFO, ebufp);
    PIN_FLIST_FLD_SET(tmp1_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, NULL, ebufp);
    PIN_FLIST_FLD_SET(tmp1_flistp, MSO_FLD_NETWORK_NODE, NULL, ebufp);
    tmp1_flistp = PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_ALIAS_LIST, PIN_ELEMID_ANY, ebufp);
    PIN_FLIST_FLD_SET(tmp1_flistp, PIN_FLD_NAME, NULL, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "get_primary_connection_details PCM_OP_SEARCH input flist", search_i_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_i_flistp, ret_flistpp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "get_primary_connection_details PCM_OP_SEARCH output flist", *ret_flistpp);

    PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);

    return;
}

void
get_sms_template(
        pcm_context_t       *ctxp,
        pin_flist_t         *i_flistp,
        char                **sms_template,
        char                *ne,
        pin_errbuf_t        *ebufp)
{
    int32                   *flag = NULL;
    poid_t                  *pdp = NULL;
    poid_t                  *s_pdp = NULL;
    int64                   database;
    pin_flist_t             *search_i_flistp = NULL;
    pin_flist_t             *search_o_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    int32                   s_flags ;
    int32                   *string_id = NULL;
    char                    *msg_template = NULL;
    int32                   str_ver;
    int32               *action_flag = NULL;
    char                    *template = "select X from /strings where F1 = V1 and F2 = V2 and F3 = V3 ";
	int32               business_type = 0;
    void                    *vp = NULL;


    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "get_sms_template input flist", i_flistp);

	action_flag = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 0, ebufp);

    string_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 0, ebufp);
    pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
    database = PIN_POID_GET_DB(pdp);

    search_i_flistp = PIN_FLIST_CREATE(ebufp);
    s_pdp = (poid_t *)PIN_POID_CREATE(database, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(search_i_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

    s_flags = SRCH_DISTINCT ;
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_DOMAIN, "SMS_TEMPLATE", ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STRING_ID, string_id, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 3, ebufp);

    vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BUSINESS_TYPE, 1, ebufp);
    if (vp)
    {
       business_type = *(int *) PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_BUSINESS_TYPE,0,ebufp);
    }

	business_type = business_type/10000;
	if (*action_flag ==  NTF_SERVICE_ACTIVATION && business_type == IPTV_BUSINESS_TYPE &&
		 strstr(ne,"JVM1")   )
	{
		str_ver =  6;		
	}
    else if (*action_flag ==  NTF_SERVICE_ACTIVATION && business_type == IPTV_BUSINESS_TYPE &&
                strstr(ne,"JVM")  ) 
        {
                str_ver =  5;
        }
    else if (*action_flag ==  NTF_SERVICE_ACTIVATION &&  (strstr(ne,"NDS1") ||  strstr(ne,"CISCO1") || strstr(ne,"NAGRA") || strstr(ne,"GOSPELL") || strstr(ne,"JVM1") ))
	{
		str_ver =  4;

	}
    else if (strstr(ne,"NDS") )
    {
        str_ver =  1;
    }
    else if (strstr(ne,"CISCO"))
    {
        str_ver =  2;
    }
	// VERIMATRIX CHANGES - added verimatrix network node condition
	else if (strstr(ne,"VM"))
    {
        str_ver =  3;
    }
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STR_VERSION, &str_ver, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_STRING, NULL, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "get_sms_template search input flist ", search_i_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_i_flistp,
        &search_o_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "get_sms_template search output flist", search_o_flistp);

    tmp_flistp = PIN_FLIST_ELEM_GET(search_o_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
    msg_template = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_STRING, 0, ebufp);

    if (msg_template && strlen(msg_template) > 0 )
    {
             *sms_template = (char *) malloc(strlen(msg_template)+1);
             memset(*sms_template, 0, sizeof(msg_template)+1);
             strcpy(*sms_template,msg_template);
             PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*sms_template );
    }

    PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&search_o_flistp, NULL);

    return;
}

static void
get_bb_sms_template(
	pcm_context_t		*ctxp, 
	pin_flist_t		*i_flistp, 
	char			**sms_template, 
	pin_errbuf_t		*ebufp)
{
    int32                   *flag = NULL;
    poid_t                  *pdp = NULL;
    poid_t                  *s_pdp = NULL;
    int64                   database;
    pin_flist_t             *search_i_flistp = NULL;
    pin_flist_t             *search_o_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    int32                   s_flags ;
    int32                   *string_id = NULL;
    char                    *msg_template = NULL;
    int32                   str_ver;
    char                    *template = "select X from /strings where F1 = V1 and F2 = V2 ";


    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "get_bb_sms_template input flist", i_flistp);

    string_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 0, ebufp);
    pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
    database = PIN_POID_GET_DB(pdp);

    search_i_flistp = PIN_FLIST_CREATE(ebufp);
    s_pdp = (poid_t *)PIN_POID_CREATE(database, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(search_i_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

    s_flags = SRCH_DISTINCT ;
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_DOMAIN, "BB_SMS_TEMPLATE", ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STRING_ID, string_id, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_STRING, NULL, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "get_bb_sms_template search input flist ", search_i_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_i_flistp,
        &search_o_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "get_bb_sms_template search output flist", search_o_flistp);

    tmp_flistp = PIN_FLIST_ELEM_GET(search_o_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
    msg_template = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_STRING, 0, ebufp);

    if (msg_template && strlen(msg_template) > 0 )
    {
         *sms_template = (char *) malloc(strlen(msg_template)+1);
         memset(*sms_template, 0, sizeof(msg_template)+1);
         strcpy(*sms_template,msg_template);
         PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*sms_template );
    }

    PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&search_o_flistp, NULL);

    return;
}

void
get_osd_template(
        pcm_context_t       *ctxp,
        pin_flist_t         *i_flistp,
        char                **osd_template,
        char                *ne,
        pin_errbuf_t        *ebufp)
{
    int32                   *flag = NULL;
    poid_t                  *pdp = NULL;
    poid_t                  *s_pdp = NULL;
    int64                   database;
    pin_flist_t             *search_i_flistp = NULL;
    pin_flist_t             *search_o_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    int32                   s_flags ;
    int32                   *string_id = NULL;
    char                    *msg_template = NULL;
    int32                   str_ver;
    char                    *template = "select X from /strings where F1 = V1 and F2 = V2 and F3 = V3 ";


    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "get_osd_template input flist", i_flistp);

    string_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 0, ebufp);
    pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
    database = PIN_POID_GET_DB(pdp);

    search_i_flistp = PIN_FLIST_CREATE(ebufp);
    s_pdp = (poid_t *)PIN_POID_CREATE(database, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(search_i_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

    s_flags = SRCH_DISTINCT ;
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_DOMAIN, "OSD_TEMPLATE", ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STRING_ID, string_id, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 3, ebufp);
    if (strstr(ne,"NDS") )
    {
        str_ver =  1;
    }
    else if (strstr(ne,"CISCO"))
    {
        str_ver =  2;
    }
	// VERIMATRIX CHANGES - added verimatrix network node condition
    else if (strstr(ne,"VM"))
    {
        str_ver =  3;
    }	
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STR_VERSION, &str_ver, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_STRING, NULL, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "get_osd_template search input flist ", search_i_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_i_flistp,
        &search_o_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "get_osd_template search output flist", search_o_flistp);

    tmp_flistp = PIN_FLIST_ELEM_GET(search_o_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
    msg_template = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_STRING, 0, ebufp);

    if (msg_template && strlen(msg_template) > 0 )
    {
             *osd_template = (char *) malloc(strlen(msg_template)+1);
             memset(*osd_template, 0, sizeof(msg_template)+1);
             strcpy(*osd_template,msg_template);
             PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*osd_template );
    }

    PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&search_o_flistp, NULL);

    return;
}

void
get_bmail_template(
        pcm_context_t       *ctxp,
        pin_flist_t         *i_flistp,
        char                **bmail_template,
        char                *ne,
        pin_errbuf_t        *ebufp)
{
    int32                   *flag = NULL;
    poid_t                  *pdp = NULL;
    poid_t                  *s_pdp = NULL;
    int64                   database;
    pin_flist_t             *search_i_flistp = NULL;
    pin_flist_t             *search_o_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    int32                   s_flags ;
    int32                   *string_id = NULL;
    char                    *msg_template = NULL;
    int32                   str_ver;
    char                    *template = "select X from /strings where F1 = V1 and F2 = V2 and F3 = V3 ";


    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "get_bmail_template input flist", i_flistp);

    string_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 0, ebufp);
    pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
    database = PIN_POID_GET_DB(pdp);

    search_i_flistp = PIN_FLIST_CREATE(ebufp);
    s_pdp = (poid_t *)PIN_POID_CREATE(database, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(search_i_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

    s_flags = SRCH_DISTINCT ;
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_DOMAIN, "BMAIL_TEMPLATE", ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STRING_ID, string_id, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 3, ebufp);
    if (strstr(ne,"NDS") )
    {
        str_ver =  1;
    }
    else if (strstr(ne,"CISCO"))
    {
        str_ver =  2;
    }
	// VERIMATRIX CHANGES - added verimatrix network node condition
    else if (strstr(ne,"VM"))
    {
        str_ver =  3;
    }	
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STR_VERSION, &str_ver, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_STRING, NULL, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "get_bmail_template search input flist ", search_i_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_i_flistp,
        &search_o_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "get_bmail_template search output flist", search_o_flistp);

    tmp_flistp = PIN_FLIST_ELEM_GET(search_o_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
    msg_template = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_STRING, 0, ebufp);

    if (msg_template && strlen(msg_template) > 0 )
    {
             *bmail_template = (char *) malloc(strlen(msg_template)+1);
             memset(*bmail_template, 0, sizeof(msg_template)+1);
             strcpy(*bmail_template,msg_template);
             PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*bmail_template );
    }

    PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&search_o_flistp, NULL);

    return;
}


/*****************************************************************
* prepare_upgrade_plan_notification_buf()
*
* Enrich the input flist with password change details
******************************************************************/
static void
prepare_upgrade_plan_notification_buf(
	pcm_context_t		*ctxp, 
	pin_flist_t		*i_flistp, 
	pin_flist_t		**payload_flistp, 
	pin_errbuf_t		*ebufp)
{
	int32                   *num_months = NULL;
	pin_decimal_t           *amountp  = NULL;
	char			str[1024];
	char                    *sms_template = NULL;

	double                  amount = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

	PIN_ERRBUF_CLEAR(ebufp);


	/***********************************************************
	* Log input flist
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_upgrade_plan_notification_buf input flist", i_flistp);
	/* Pawan:16-02-2015: Commented below line and added next line to fetch the
		password from MSO_FLD_PASSWORD */
	//passwd = (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PASSWD, 0, ebufp);
	 /*
	* Validate input received
	*/
	amountp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 0, ebufp);
	amount = pbo_decimal_to_double(amountp, ebufp);
	num_months = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_NUM_UNITS, 0, ebufp);

	get_bb_sms_template(ctxp, i_flistp, &sms_template, ebufp);
	if (!sms_template)
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
		goto CLEANUP;
	}

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, sms_template );
	/*
	 * Prepare Message
	 */
	sprintf(str, sms_template, amount, *num_months);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );
	/*
	* Push Message in EMAIL payload
	*/
	PIN_FLIST_FLD_SET(*payload_flistp, PIN_FLD_MESSAGE, str, ebufp);
	//PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_SUBJECT, "Password Change Successful", ebufp);


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_upgrade_plan_notification_buf enriched input flist", *payload_flistp);

CLEANUP:
    if (sms_template)
    {
        free(sms_template);
    }
	return;
}

void 
fm_mso_get_dl_speed(
	pcm_context_t	*ctxp,
	pin_flist_t	*in_flistp, 
	pin_flist_t	**out_flistp, 
	pin_errbuf_t	*ebufp)
{	
	poid_t		*act_poid = NULL;
	poid_t		*srvc_poid = NULL;
	poid_t		*plan_poid = NULL;
	char		*plan_name = NULL;
	char		*city 	= NULL;
	int32		*bill_when = NULL;
	int32		 status = 2;
	int32		prod_status = 1;
	pin_flist_t	*mso_pur_flistp = NULL;
	pin_flist_t	*result_flist = NULL;
	pin_flist_t	*read_iflistp = NULL;
	pin_flist_t	*read_oflistp = NULL;
	pin_flist_t	*srvc_oflistp = NULL;
	pin_flist_t	*bb_info = NULL;
	pin_flist_t	*res_flistp = NULL;

	PIN_ERR_LOG_FLIST(3, "fm_mso_get_dl_speed : input flistp:", in_flistp);
	act_poid = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	srvc_poid = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
	fm_mso_utils_get_mso_baseplan_details(ctxp, act_poid, srvc_poid, MSO_PROV_STATE_ACTIVE, prod_status, &mso_pur_flistp, ebufp);
	PIN_ERR_LOG_FLIST(3, "Base plan details flist", mso_pur_flistp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_ERR(1, "Error during getting base_plan details");
		goto CLEANUP;
	}
	res_flistp = PIN_FLIST_ELEM_GET(mso_pur_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	if(res_flistp)
	{
		plan_poid  = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
		read_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(read_iflistp, PIN_FLD_POID, plan_poid,  ebufp);
		PIN_FLIST_FLD_SET(read_iflistp, PIN_FLD_NAME, NULL, ebufp);
		PIN_ERR_LOG_FLIST(3, "Plan name readflds ", read_iflistp);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_iflistp, &read_oflistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_ERR(1, "Error during getting base_plan details");
                	goto CLEANUP;
	        }
		plan_name = PIN_FLIST_FLD_GET(read_oflistp, PIN_FLD_NAME, 0, ebufp);
		PIN_FLIST_DESTROY_EX(&read_iflistp, NULL);
		read_iflistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(read_iflistp, PIN_FLD_POID, srvc_poid,  ebufp);
		bb_info = PIN_FLIST_SUBSTR_ADD(read_iflistp, MSO_FLD_BB_INFO, ebufp);
		PIN_FLIST_FLD_SET(bb_info, PIN_FLD_CITY, NULL, ebufp);
		PIN_FLIST_FLD_SET(bb_info, PIN_FLD_BILL_WHEN, NULL, ebufp);
		PIN_ERR_LOG_FLIST(3, "Plan name readflds ", read_iflistp);
                PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_iflistp, &srvc_oflistp, ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp)) {
                        PIN_ERR_LOG_ERR(1, "Error during getting base_plan details");
                        goto CLEANUP;
                }
		bb_info = PIN_FLIST_SUBSTR_GET(srvc_oflistp, MSO_FLD_BB_INFO, 0, ebufp);
		city = PIN_FLIST_FLD_GET(bb_info, PIN_FLD_CITY, 0, ebufp);
		bill_when = PIN_FLIST_FLD_GET(bb_info, PIN_FLD_BILL_WHEN, 0, ebufp);
		fm_mso_get_service_quota_codes(ctxp, plan_name, bill_when, city,                                                                                                                        &result_flist, ebufp);	
		  if (PIN_ERRBUF_IS_ERR(ebufp)) 
                {
                        PIN_ERR_LOG_ERR(1, "Error during getting base_plan details");
                      	goto CLEANUP; 
                }

		*out_flistp = PIN_FLIST_COPY(result_flist, ebufp);

	}

	CLEANUP:
		PIN_FLIST_DESTROY_EX(&mso_pur_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&result_flist, NULL);
		PIN_FLIST_DESTROY_EX(&read_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&srvc_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
		return;
	
}

/*****************************************************
 * Added this function to faciliate sending SMS 
 * Notification for Additonal GB purchased via Topup 
 * from Quick Pay
 *****************************************************/
static void
prepare_extra_gb_topup_notification_buf(
	pcm_context_t         *ctxp,
        pin_flist_t           *i_flistp,
        pin_flist_t           **ret_flistpp,
        pin_errbuf_t          *ebufp)
{
    	char        *sms_template = NULL;
    	char        *account_no = NULL;
    	char        *additional_gb = NULL;
	char        *phone = NULL;
	char        validity_date[50] = {""};    
    	char        str[1024];	
	time_t      *end_t;
    
    	if (PIN_ERRBUF_IS_ERR(ebufp))
       		return;

    	PIN_ERRBUF_CLEAR(ebufp);

    	/***********************************************************
     	* Log input flist
     	***********************************************************/
    	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "prepare_extra_gb_topup_notification_buf input flist", i_flistp);
	
    	/*******************************************
     	 * Get the SMS Template
     	 *******************************************/
	 get_bb_sms_template(ctxp, i_flistp, &sms_template, ebufp);
    	if (!sms_template)
    	{
        	pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
        	goto CLEANUP;
    	}

    	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, sms_template);

	account_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
    	phone = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PHONE, 0, ebufp);
	additional_gb = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_VALUE, 0, ebufp);
	end_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 0, ebufp);
	
	pin_strftimet(validity_date, sizeof(validity_date), "%d-%h-%Y", *end_t);
    	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, validity_date );
	
    	/***********************************
     	 * Prepare Message
     	 ***********************************/
	if(account_no && strlen(account_no) != 0 &&
	   additional_gb && strlen(additional_gb) != 0 &&
	   validity_date && strlen(validity_date) != 0)
	{
		sprintf(str, sms_template, additional_gb, account_no, validity_date, phone);
	}
    
    	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );

    	/****************************************
     	 * Push Message in SMS payload
     	 ****************************************/
    	PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_MESSAGE, str, ebufp);

    	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_extra_gb_topup_notification_buf enriched input flist", *ret_flistpp);

CLEANUP:
    if (sms_template)
    {
        free(sms_template);
    }
   return;
}


