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
* 0.1    |20/12/2013 |Satya Prakash     |  CATV EMAIL Notification implementation 
*--------------------------------------------------------------------------------------------------------------*
* 0.2    |13/04/2014 | Vilva Sabarikanth|  CATV EMAIL Notification based on flags enablement from CM's pin.conf *
*--------------------------------------------------------------------------------------------------------------*
* 0.3    |31/07/2014 | Someshwar        | Added changes for Broadband                                          *
*--------------------------------------------------------------------------------------------------------------*
*--------------------------------------------------------------------------------------------------------------*
* 0.4    |09/03/2017 | Tony             |  Added Usage type field to /mso_ntf_email and functionality to send day's
*                                       |  first copy of each type to configured business users
*--------------------------------------------------------------------------------------------------------------*
****************************************************************************************************************/


#ifndef lint
static  char Sccs_Id[] = "@(#)$Id: fm_mso_ntf_create_email_notification.c /cgbubrm_7.3.2.rwsmod/1 2009/08/04 23:37:19 pjegan Exp $";
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

#define CHANNEL "Email"

extern void 
fm_mso_utils_sequence_no(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);


/*******************************************************************
 * Fuctions defined in this code 
 *******************************************************************/

EXPORT_OP void
op_mso_ntf_create_email_notification(
        cm_nap_connection_t    *connp,
        int32               opcode,
        int32               flags,
        pin_flist_t         *in_flistp,
        pin_flist_t         **ret_flistpp,
        pin_errbuf_t        *ebufp);

static void
fm_mso_ntf_create_email_notification(
        pcm_context_t       *ctxp,
        pin_flist_t         *in_flistp,
        int32               flags,
        pin_flist_t         **out_flistpp,
        pin_errbuf_t        *ebufp);

extern void
get_email_template(
        pcm_context_t       *ctxp,
        pin_flist_t         *i_flistp,
        char                **email_template,
        char                *ne,
        pin_errbuf_t        *ebufp);

extern void
get_osd_template(
        pcm_context_t       *ctxp,
        pin_flist_t         *i_flistp,
        char                **osd_template,
        char                *ne,
        pin_errbuf_t        *ebufp);

extern void
get_bmail_template(
        pcm_context_t       *ctxp,
        pin_flist_t         *i_flistp,
        char                **bmail_template,
        char                *ne,
        pin_errbuf_t        *ebufp);

extern void
get_primary_connection_details(
        pcm_context_t       *ctxp,
        pin_flist_t         *i_flistp,
        pin_flist_t         **ret_flistpp,
        pin_errbuf_t        *ebufp);

extern void
fm_mso_send_osd_request(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    char                    *str,
    char                    *ne,
    pin_errbuf_t            *ebufp);

extern void
fm_mso_send_bmail_request(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    char                    *str,
    char                    *ne,
    pin_errbuf_t            *ebufp);

static void
prepare_service_activation_notification_buf(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **ret_flistpp,
    pin_errbuf_t            *ebufp);

static void
prepare_service_suspension_notification_buf(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **ret_flistpp,
    pin_errbuf_t            *ebufp);

static void
prepare_service_reactivation_notification_buf(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **ret_flistpp,
    pin_errbuf_t            *ebufp);

static void
prepare_service_termination_notification_buf(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **ret_flistpp,
    pin_errbuf_t            *ebufp);

static void
prepare_add_plan_notification_buf(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **ret_flistpp,
    pin_errbuf_t            *ebufp);

static void
prepare_change_plan_notification_buf(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **ret_flistpp,
    pin_errbuf_t            *ebufp);

static void
prepare_cancel_plan_notification_buf(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **ret_flistpp,
    pin_errbuf_t            *ebufp);

static void
prepare_billing_notification_buf(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **ret_flistpp,
    pin_errbuf_t            *ebufp);

static void
prepare_due_reminder_notification_buf(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **ret_flistpp,
    pin_errbuf_t            *ebufp);

static void
prepare_payment_notification_buf(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **ret_flistpp,
    pin_errbuf_t            *ebufp);

static void
prepare_payment_reversal_notification_buf(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **ret_flistpp,
    pin_errbuf_t            *ebufp);

static void
prepare_change_passwd_notification_buf(
	pcm_context_t			*ctxp, 
	pin_flist_t				*i_flistp, 
	pin_flist_t				**payload_flistp, 
	pin_errbuf_t			*ebufp);

static void
prepare_deposit_refund_notification_buf(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **ret_flistpp,
    pin_errbuf_t            *ebufp);

static void
prepare_upgrade_plan_notification_buf(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

static void
mso_prov_get_provisioning_tag_details(
        char                *prov_tag,
        pin_flist_t         **ret_flistpp,
        pin_errbuf_t        *ebufp);

static void
add_package_detail(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    int32                   flag,
    pin_errbuf_t            *ebufp);

static void
get_bb_email_template(
	pcm_context_t		*ctxp, 
	pin_flist_t			*i_flistp, 
	char				**email_template, 
	pin_errbuf_t		*ebufp);

PIN_IMPORT int32		email_config[20];

/*******************************************************************
 * Main routine for the MSO_OP_CREATE_CATV_ACTION command
 *******************************************************************/
void
op_mso_ntf_create_email_notification(
        cm_nap_connection_t    *connp,
        int32               opcode,
        int32               flags,
        pin_flist_t         *in_flistp,
        pin_flist_t         **ret_flistpp,
        pin_errbuf_t        *ebufp)
{
    pin_flist_t             *r_flistp = NULL;
    pcm_context_t           *ctxp = connp->dm_ctx;


    /***********************************************************
     * Null out results until we have some.
     ***********************************************************/
    *ret_flistpp = NULL;
    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Insanity check.
     ***********************************************************/
    if (opcode != MSO_OP_NTF_CREATE_EMAIL_NOTIFICATION) {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_OPCODE, 0, 0, opcode);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "bad opcode in op_mso_ntf_create_email_notification", ebufp);
        return;
    }

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "op_mso_ntf_create_email_notification input flist", in_flistp);

    /***********************************************************
     * Call main function to do it
     ***********************************************************/
    fm_mso_ntf_create_email_notification(ctxp, in_flistp, flags, &r_flistp, ebufp);

    /***********************************************************
     * Results.
     ***********************************************************/
    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        *ret_flistpp = (pin_flist_t *)NULL;
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_mso_ntf_create_email_notification error", ebufp);
		PIN_ERRBUF_CLEAR(ebufp);
    } else {
        *ret_flistpp = r_flistp;
        PIN_ERRBUF_CLEAR(ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "op_mso_ntf_create_email_notification return flist", r_flistp);
    }

    return;
}

/*******************************************************************
 * fm_mso_ntf_create_email_notification()
 *
 * Perfomrs following action:
 * 1. call action specific function to enrich the input from service
 *        and package detail
 * 2. get notification sequence 
 * 3. create notification order /mso_ntf_email
 *******************************************************************/
static void
fm_mso_ntf_create_email_notification(
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
	pin_buf_t           *nota_buf     = NULL;
//    char                *action = NULL;
	char                *flistbuf = NULL,
                            *usage_type = NULL,
                            *channel = NULL;
	int                 flistlen=0;
	int                 status = NTF_STATE_NEW;
	char                *act = NULL;
	char                *email_addr = NULL;
	char                *email_template = NULL;
	int32               priority;
	int32		    notif_disabled = 0;
	int32		    op_status = 0; //SUCCESS
        int32               send_to_users = 0;
        int32               elemid = 0;

        pin_cookie_t        cookie = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);

	/***********************************************************
	 * Log input flist
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_ntf_create_email_notification input flist", in_flistp);

	/***********************************************************
	 * Check mandatory fields are available
	 ***********************************************************/
	email_addr = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_EMAIL_ADDR, 0, ebufp);
	action_flag = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_FLAGS, 0, ebufp);

	if (action_flag && 
		!(*action_flag == NTF_DUE_REMINDER  || 
		  *action_flag == NTF_REMINDER_POST_DUE_DATE || 
		  *action_flag == NTF_BILLING ) )
	{
		service_poidp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
	}

	acc_poidp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_ntf_create_email_notification error: required field missing in input flist", ebufp);
		return;
	}

	/***********************************************************
	 * Enrich With EMAIL Text
	 ***********************************************************/
	if ( *action_flag ==  NTF_SERVICE_ACTIVATION )
	{	
		//Not required for BB
		service_poidp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		if(service_poidp && !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
		{
			if (email_config[6] == 1 || email_config[6] == 3)
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_EMAIL_ADDR, email_addr, ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_FROM_NAME, "alert@hathway.net", ebufp);
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
		else{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Disabled");
			notif_disabled = 1;
			goto Cleanup;
		}
	}
	else if ( *action_flag ==  NTF_SERVICE_SUSPENSION )
	{
		//Not required for BB
		service_poidp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		//if(PIN_POID_IS_NULL(service_poidp) || !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
		if(service_poidp && !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
		{
			if (email_config[7] == 1 || email_config[7] == 3)
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_EMAIL_ADDR, email_addr, ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_FROM_NAME, "alert@hathway.net", ebufp);
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
		else{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Disabled");
			notif_disabled = 1;
			goto Cleanup;
		}
	}
	else if ( *action_flag ==  NTF_SERVICE_REACTIVATION )
	{
		//Not required for BB
		service_poidp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		//if(PIN_POID_IS_NULL(service_poidp) || !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
		if(service_poidp && !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
		{
			if (email_config[8] == 1 || email_config[8] == 3)
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_EMAIL_ADDR, email_addr, ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_FROM_NAME, "alert@hathway.net", ebufp);
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
		else{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Disabled");
			notif_disabled = 1;
			goto Cleanup;
		}
	}
	else if ( *action_flag ==  NTF_SERVICE_TERMINATON )
	{
		//Not required for BB
		service_poidp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		//if(PIN_POID_IS_NULL(service_poidp) || !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
		if(service_poidp && !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
		{
			if (email_config[9] == 1 || email_config[9] == 3)
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_EMAIL_ADDR, email_addr, ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_FROM_NAME, "alert@hathway.net", ebufp);
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
		else{
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
		if(service_poidp && !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
		{
			if (email_config[10] == 1 || email_config[10] == 3)
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_EMAIL_ADDR, email_addr, ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_FROM_NAME, "alert@hathway.net", ebufp);
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
		else{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Disabled");
			notif_disabled = 1;
			goto Cleanup;
		}
	}
	else if ( *action_flag ==  NTF_CHANGE_PLAN )
	{
		//For CATV only
		service_poidp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		if (email_config[11] == 1)
		{
			//if(PIN_POID_IS_NULL(service_poidp) || !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
			if(service_poidp && !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_EMAIL_ADDR, email_addr, ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_FROM_NAME, "alert@hathway.net", ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
				prepare_change_plan_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
				priority = 1;
			}
		}
		//For BB only
		else if (email_config[11] == 2)
		{
			if(service_poidp && !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/telco/broadband"))
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_EMAIL_ADDR, email_addr, ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_FROM_NAME, "alert@hathway.net", ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
				prepare_change_plan_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
				priority = 1;
			}
		}
		//For both CATV & BB
		else if (email_config[11] == 3)
		{
			payload_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_EMAIL_ADDR, email_addr, ebufp);
			PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_FROM_NAME, "alert@hathway.net", ebufp);
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
		if(service_poidp && !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
		{
			if (email_config[12] == 1 || email_config[12] == 3)
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_EMAIL_ADDR, email_addr, ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_FROM_NAME, "alert@hathway.net", ebufp);
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
		else{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Disabled");
			notif_disabled = 1;
			goto Cleanup;
		}
	}
	else if ( *action_flag ==  NTF_REMINDER_POST_DUE_DATE )
	{
		//Not required for BB
		service_poidp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		//if(PIN_POID_IS_NULL(service_poidp) || !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
		if(service_poidp && !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
		{
			if (email_config[13] == 1 || email_config[13] == 3)
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_EMAIL_ADDR, email_addr, ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_FROM_NAME, "alert@hathway.net", ebufp);
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
		else{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Disabled");
			notif_disabled = 1;
			goto Cleanup;
		}
	}
	else if ( *action_flag ==  NTF_BILLING )
	{
		service_poidp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		//For CATV
		if (email_config[1] == 1)
		{
			//if(PIN_POID_IS_NULL(service_poidp) || !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
			if(service_poidp && !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_EMAIL_ADDR, email_addr, ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_FROM_NAME, "alert@hathway.net", ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
				prepare_billing_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
				priority = 1;
			}
		}
		//For BB
		else if (email_config[1] == 2)
		{
			if(service_poidp && !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/telco/broadband"))
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_EMAIL_ADDR, email_addr, ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_FROM_NAME, "alert@hathway.net", ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
				prepare_billing_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
				priority = 1;
			}
		}
		//For Both CATV and BB
		else if (email_config[1] == 3)
		{
			payload_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_EMAIL_ADDR, email_addr, ebufp);
			PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_FROM_NAME, "alert@hathway.net", ebufp);
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
		//Not required for BB
		service_poidp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		//if(PIN_POID_IS_NULL(service_poidp) || !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
		if(service_poidp && !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
		{
			if (email_config[2] == 1 || email_config[2] == 3)
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_EMAIL_ADDR, email_addr, ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_FROM_NAME, "alert@hathway.net", ebufp);
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
		else{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Disabled");
			notif_disabled = 1;
			goto Cleanup;
		}
	}
	else if ( *action_flag ==  NTF_PAYMENT   )
	{
		service_poidp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		if (email_config[3] == 1)
		{
			//if(PIN_POID_IS_NULL(service_poidp) || !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
			if(service_poidp && !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_EMAIL_ADDR, email_addr, ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_FROM_NAME, "alert@hathway.net", ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
				prepare_payment_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
				priority = 1;
			}
		}
		else if (email_config[3] == 2)
		{
			if(service_poidp && !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/telco/broadband"))
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_EMAIL_ADDR, email_addr, ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_FROM_NAME, "alert@hathway.net", ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
				prepare_payment_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
				priority = 1;
			}
		}
		else if (email_config[3] == 3)
		{
			payload_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_EMAIL_ADDR, email_addr, ebufp);
			PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_FROM_NAME, "alert@hathway.net", ebufp);
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
		service_poidp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		if (email_config[4] == 1)
		{
			//if(PIN_POID_IS_NULL(service_poidp) || !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
			if(service_poidp && !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_EMAIL_ADDR, email_addr, ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_FROM_NAME, "alert@hathway.net", ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
				prepare_payment_reversal_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
				priority = 1;
			}
		}
		else if (email_config[4] == 2)
		{
			if(service_poidp && !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/telco/broadband"))
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_EMAIL_ADDR, email_addr, ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_FROM_NAME, "alert@hathway.net", ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
				prepare_payment_reversal_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
				priority = 1;
			}
		}
		if (email_config[4] == 3)
		{
			payload_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_EMAIL_ADDR, email_addr, ebufp);
			PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_FROM_NAME, "alert@hathway.net", ebufp);
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
	//	if(PIN_POID_IS_NULL(service_poidp) || !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
		if(service_poidp && !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/catv"))
		{
			if (email_config[5] == 1 || email_config[5] == 3)
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_EMAIL_ADDR, email_addr, ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_FROM_NAME, "alert@hathway.net", ebufp);
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
		else{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Disabled");
			notif_disabled = 1;
			goto Cleanup;
		}
	}
	else if ( *action_flag ==  NTF_BB_CHANGE_PASSWORD )
	{		
		if (email_config[5] == 1 || email_config[5] == 2 || email_config[5] == 3)
		{
			payload_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_EMAIL_ADDR, email_addr, ebufp);
			PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_FROM_NAME, "alert@hathway.net", ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
			prepare_change_passwd_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
			priority = 1;
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Disabled");
				notif_disabled = 1;
			goto Cleanup;
		}	
	}
	else if ( *action_flag ==  NTF_AFTER_FUP )
        {
		notif_disabled = 1;
		PIN_ERRBUF_CLEAR(ebufp);	
		goto Cleanup;
	}
	else if ( *action_flag ==  NTF_BB_UPGRADE_PLAN)
	{		
		service_poidp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		if (email_config[1] == 2)
		{
			if(service_poidp && !strcmp(PIN_POID_GET_TYPE(service_poidp), "/service/telco/broadband"))
			{
				payload_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_EMAIL_ADDR, email_addr, ebufp);
				PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_FROM_NAME, "alert@hathway.net", ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Notification Enabled");
				prepare_upgrade_plan_notification_buf(ctxp, in_flistp, &payload_flistp, ebufp);
				priority = 1;
			}
		}
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
	if(payload_flistp)
	{
		/*
		 * Get Order Id and set it in payload
		 */
		order_seq_i_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(order_seq_i_flistp, PIN_FLD_POID, acc_poidp, ebufp);
		PIN_FLIST_FLD_SET(order_seq_i_flistp, PIN_FLD_NAME, "MSO_NTF_SEQUENCE", ebufp);
		//call the function to read the provisioning order sequence
		fm_mso_utils_sequence_no(ctxp, order_seq_i_flistp, &order_seq_o_flistp, ebufp);


	    /*
	     * Convert flist to String Payload. it will be used by Adaptor process to send EMAIL
	     */
		PIN_FLIST_TO_XML( payload_flistp, PIN_XML_BY_SHORT_NAME, 0,
				&flistbuf, &flistlen, "Email", ebufp );
	//    PIN_FLIST_TO_STR(payload_flistp,&flistbuf,&flistlen,ebufp );


	    /*
	     * Create flist for order creation
	     */
		order_i_flistp = PIN_FLIST_CREATE(ebufp);
		db_no = PIN_POID_GET_DB(acc_poidp); 
		objp = PIN_POID_CREATE(db_no, "/mso_ntf_email", -1, ebufp);
		PIN_FLIST_FLD_SET(order_i_flistp, PIN_FLD_POID, objp, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, order_i_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SERVICE_OBJ, order_i_flistp, PIN_FLD_SERVICE_OBJ, ebufp);

		PIN_FLIST_FLD_COPY(order_seq_o_flistp, PIN_FLD_STRING, order_i_flistp, MSO_FLD_NTF_ID, ebufp);
		PIN_FLIST_FLD_SET(order_i_flistp, PIN_FLD_STATUS, &status, ebufp); //NEW
		PIN_FLIST_FLD_SET(order_i_flistp, PIN_FLD_RANK, &priority, ebufp); 

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
                if(PIN_ERRBUF_IS_ERR(ebufp)){
                    PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in function fm_mso_ntf_common_get_usage_type_descr", ebufp);
                    return;
                }
                if(usage_type == NULL){
                    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Unable to retrieve usage type description");
                    return;
                }

                PIN_FLIST_FLD_PUT(order_i_flistp, PIN_FLD_USAGE_TYPE, usage_type, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_mso_ntf_create_email_notification create input flist", order_i_flistp);
		PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, order_i_flistp, out_flistpp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_mso_ntf_create_email_notification order create output flist", *out_flistpp);

		PIN_FLIST_FLD_COPY(order_i_flistp, MSO_FLD_NTF_ID, *out_flistpp, MSO_FLD_NTF_ID, ebufp);

		//Adding logic for sending copy to business users
                channel = (char *) malloc (sizeof(char) * 60);
                memset(channel, '\0', sizeof(channel));
                strcpy(channel, CHANNEL);
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Channel type initialized");
                send_to_users = fm_mso_ntf_common_check_delivery_status(ctxp, in_flistp, usage_type, channel, ebufp);
                if(PIN_ERRBUF_IS_ERR(ebufp)){
                    PIN_ERRBUF_CLEAR(ebufp);
                    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Unable to check for delivery status to business users");
                }
                free(channel);
                if(send_to_users){

                fm_mso_ntf_common_get_business_users(ctxp, in_flistp, &bus_users_flistp, ebufp);
                if(PIN_ERRBUF_IS_ERR(ebufp) || bus_users_flistp == NULL){

                    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "fm_mso_ntf_create_email_notification: Error in retrieving business users list");
                }
                else{
                    while((bus_users_res_flistp =
                        (pin_flist_t *) PIN_FLIST_ELEM_GET_NEXT(bus_users_flistp, PIN_FLD_RESULTS, &elemid, 1, &cookie, ebufp)) != NULL){

			email_addr = NULL;
                        email_addr = PIN_FLIST_FLD_GET(bus_users_res_flistp, PIN_FLD_EMAIL_ADDR, 1, ebufp);

			if(email_addr){
                            //Update email_id in payload
                            PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_EMAIL_ADDR, email_addr, ebufp);

			    //call the function to read the provisioning order sequence
                            fm_mso_utils_sequence_no(ctxp, order_seq_i_flistp, &order_seq_o_flistp, ebufp);

			    /*
                             * Convert flist to String Payload. it will be used by Adaptor process to send SMS
                             */
                             flistlen = 0;
                             flistbuf = NULL;
                             PIN_FLIST_TO_XML( payload_flistp, PIN_XML_BY_SHORT_NAME, 0,
                                    &flistbuf, &flistlen, "Email", ebufp );

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
                                    "fm_mso_ntf_create_email_notification create busines user input flist", order_i_flistp);
                            PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, order_i_flistp, out_flistpp, ebufp);

                            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                            "fm_mso_ntf_create_email_notification order create business user output flist", *out_flistpp);
                            PIN_FLIST_FLD_COPY(order_i_flistp, MSO_FLD_NTF_ID, *out_flistpp, MSO_FLD_NTF_ID, ebufp);
                        }
                        else{
                                continue;
                        }
                    }
                }
            }
            else{
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No need to send to business users");
            }
	}
Cleanup:
	if(notif_disabled == 1) {
	    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," Preparing dummy output flist with status as SUCCESS");
	    *out_flistpp = PIN_FLIST_CREATE(ebufp);
	    PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
	    PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_STATUS, &op_status, ebufp);
	}
	PIN_FLIST_DESTROY_EX(&payload_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&order_seq_i_flistp, NULL);
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
    char                    *email_template = NULL;
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

    /*
     * get action and NE specific EMAIL template
     */
    get_email_template(ctxp, i_flistp, &email_template, ne, ebufp);
    if (!email_template)
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
        goto CLEANUP;
    }
    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, email_template );

    /*
     * Read account no
     */
    ai_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ai_flistp, PIN_FLD_POID, ebufp);
    PIN_FLIST_FLD_SET(ai_flistp, PIN_FLD_ACCOUNT_NO, NULL, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_service_activation_notification_buf PCM_OP_READ_FLDS input flist", ai_flistp);
    PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, ai_flistp, &ao_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_service_activation_notification_buf PCM_OP_READ_FLDS output flist", ao_flistp);

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

        sprintf(str,email_template, account_no, stb_id, vc_id);
    }
    else if (ne && strstr(ne,"CISCO") )
    {
        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 2, 0, ebufp);
        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        sprintf(str,email_template, account_no, stb_id);
    }
	// VERIMATRIX CHANGES - BEGIN - added verimatrix network node condition
    else if (ne && strstr(ne,"VM") )
    {
        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        sprintf(str,email_template, account_no, stb_id);
    }
	// VERIMATRIX CHANGES - END - added verimatrix network node condition
    else
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
        goto CLEANUP;
    }

    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );

    /*
     * Push Message in EMAIL payload
     */
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_MESSAGE, str, ebufp);
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_SUBJECT, "Welcome to Hathway", ebufp);


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_service_activation_notification_buf enriched input flist", *ret_flistpp);

CLEANUP:
    if (email_template)
    {
        free(email_template);
    }

    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&ai_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&ao_flistp, NULL);
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
    pin_flist_t             *ai_flistp = NULL;
    pin_flist_t             *ao_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    char                    *ne = NULL;
    char                    *email_template = NULL;
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
        "prepare_service_suspension_notification_buf input flist", i_flistp);


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
        "prepare_service_suspension_notification_buf PCM_OP_READ_FLDS input flist", si_flistp);
    PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_service_suspension_notification_buf PCM_OP_READ_FLDS output flist", so_flistp);

    tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
    ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 

    /*
     * get action and NE specific EMAIL template
     */
    get_email_template(ctxp, i_flistp, &email_template, ne, ebufp);
    if (!email_template)
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
        goto CLEANUP;
    }
    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, email_template );

    /*
     * Read account no
     */
    ai_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ai_flistp, PIN_FLD_POID, ebufp);
    PIN_FLIST_FLD_SET(ai_flistp, PIN_FLD_ACCOUNT_NO, NULL, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_service_suspension_notification_buf PCM_OP_READ_FLDS input flist", ai_flistp);
    PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, ai_flistp, &ao_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_service_suspension_notification_buf PCM_OP_READ_FLDS output flist", ao_flistp);

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

        sprintf(str,email_template, account_no, stb_id, vc_id);
    }
    else if (ne && strstr(ne,"CISCO") )
    {
        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 2, 0, ebufp);
        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        sprintf(str,email_template, account_no, stb_id);
    }
	// VERIMATRIX CHANGES - BEGIN - added verimatrix network node condition
    else if (ne && strstr(ne,"VM") )
    {
        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        sprintf(str,email_template, account_no, stb_id);
    }	
	// VERIMATRIX CHANGES - END - added verimatrix network node condition
    else
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
        goto CLEANUP;
    }

    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );

    /*
     * Push Message in EMAIL payload
     */
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_MESSAGE, str, ebufp);
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_SUBJECT, "Service Suspended", ebufp);


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_service_suspension_notification_buf enriched input flist", *ret_flistpp);

CLEANUP:
    if (email_template)
    {
        free(email_template);
    }

    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&ai_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&ao_flistp, NULL);
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
    char                    *email_template = NULL;
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
     * get action and NE specific EMAIL template
     */
    get_email_template(ctxp, i_flistp, &email_template, ne, ebufp);
    if (!email_template)
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
        goto CLEANUP;
    }
    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, email_template );

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

        sprintf(str,email_template, account_no, stb_id, vc_id);
    }
    else if (ne && strstr(ne,"CISCO") )
    {
        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 2, 0, ebufp);
        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        sprintf(str,email_template, account_no, stb_id);
    }
	// VERIMATRIX CHANGES - BEGIN - added verimatrix network node condition
    else if (ne && strstr(ne,"VM") )
    {
        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        sprintf(str,email_template, account_no, stb_id);
    }	
	// VERIMATRIX CHANGES - END - added verimatrix network node condition
    else
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
        goto CLEANUP;
    }

    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );

    /*
     * Push Message in EMAIL payload
     */
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_MESSAGE, str, ebufp);
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_SUBJECT, "Service Reactivated", ebufp);


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_service_reactivation_notification_buf enriched input flist", *ret_flistpp);

CLEANUP:
    if (email_template)
    {
        free(email_template);
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
    char                    *ne = NULL;
    char                    *email_template = NULL;
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
        "prepare_service_termination_notification_buf input flist", i_flistp);


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
        "prepare_service_termination_notification_buf PCM_OP_READ_FLDS input flist", si_flistp);
    PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_service_termination_notification_buf PCM_OP_READ_FLDS output flist", so_flistp);

    tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
    ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 

    /*
     * get action and NE specific EMAIL template
     */
    get_email_template(ctxp, i_flistp, &email_template, ne, ebufp);
    if (!email_template)
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
        goto CLEANUP;
    }
    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, email_template );

    /*
     * Read account no
     */
    ai_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ai_flistp, PIN_FLD_POID, ebufp);
    PIN_FLIST_FLD_SET(ai_flistp, PIN_FLD_ACCOUNT_NO, NULL, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_service_termination_notification_buf PCM_OP_READ_FLDS input flist", ai_flistp);
    PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, ai_flistp, &ao_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_service_termination_notification_buf PCM_OP_READ_FLDS output flist", ao_flistp);

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

        sprintf(str,email_template, account_no, stb_id, vc_id);
    }
    else if (ne && strstr(ne,"CISCO") )
    {
        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 2, 0, ebufp);
        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        sprintf(str,email_template, account_no, stb_id);
    }
	// VERIMATRIX CHANGES - BEGIN - added verimatrix network node condition
    else if (ne && strstr(ne,"VM") )
    {
        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        sprintf(str,email_template, account_no, stb_id);
    }
	// VERIMATRIX CHANGES - END - added verimatrix network node condition	
    else
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
        goto CLEANUP;
    }

    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );

    /*
     * Push Message in EMAIL payload
     */
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_MESSAGE, str, ebufp);
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_SUBJECT, "Service Terminated", ebufp);


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_service_termination_notification_buf enriched input flist", *ret_flistpp);

CLEANUP:
    if (email_template)
    {
        free(email_template);
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
    char                    *email_template = NULL;
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
     * get action and NE specific EMAIL template
     */
    get_email_template(ctxp, i_flistp, &email_template, ne, ebufp);
    if (!email_template)
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
        goto CLEANUP;
    }
    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, email_template );

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

        sprintf(str,email_template,package, account_no, stb_id, vc_id);
    }
    else if (ne && strstr(ne,"CISCO") )
    {
        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 2, 0, ebufp);
        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        sprintf(str,email_template, package, account_no, stb_id);
    }
	// VERIMATRIX CHANGES - BEGIN - added verimatrix network node condition
    else if (ne && strstr(ne,"VM") )
    {
        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        sprintf(str,email_template, account_no, stb_id);
    }	
	// VERIMATRIX CHANGES - END - added verimatrix network node condition
    else
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
        goto CLEANUP;
    }

    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );

    /*
     * Push Message in EMAIL payload
     */
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_MESSAGE, str, ebufp);
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_SUBJECT, "Package successfully added for your service", ebufp);


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_add_plan_notification_buf enriched input flist", *ret_flistpp);

CLEANUP:
    if (email_template)
    {
        free(email_template);
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
    char                    *email_template = NULL;
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
	
	//Get the service poid
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
		 * get action and NE specific EMAIL template
		 */
		get_email_template(ctxp, i_flistp, &email_template, ne, ebufp);
		if (!email_template)
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
			goto CLEANUP;
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, email_template );

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

			sprintf(str,email_template, cancealled_plan_name, added_plan_name, account_no, stb_id, vc_id);
		}
		else if (ne && strstr(ne,"CISCO") )
		{
			tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 2, 0, ebufp);
			stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

			sprintf(str,email_template, cancealled_plan_name, added_plan_name, account_no, stb_id);
		}
		// VERIMATRIX CHANGES - BEGIN - added verimatrix network node condition
		else if (ne && strstr(ne,"VM") )
		{
			tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
			stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

			sprintf(str,email_template, account_no, stb_id);
		}	
		// VERIMATRIX CHANGES - END - added verimatrix network node condition
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
		get_bb_email_template(ctxp, i_flistp, &email_template, ebufp);
		if (!email_template)
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
			goto CLEANUP;
		}

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, email_template );
		/*
		 * Prepare Message
		 */
		 sprintf(str, email_template, cancealled_plan_name, added_plan_name);
	}
    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );

    /*
     * Push Message in EMAIL payload
     */
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_MESSAGE, str, ebufp);
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_SUBJECT, "Plan change completed for your service", ebufp);


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_change_plan_notification_buf enriched input flist", *ret_flistpp);

CLEANUP:
    if (email_template)
    {
        free(email_template);
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
    char                    *email_template = NULL;
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
     * get action and NE specific EMAIL template
     */
    get_email_template(ctxp, i_flistp, &email_template, ne, ebufp);
    if (!email_template)
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
        goto CLEANUP;
    }
    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, email_template );

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

        sprintf(str,email_template,package, account_no, stb_id, vc_id);
    }
    else if (ne && strstr(ne,"CISCO") )
    {
        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 2, 0, ebufp);
        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        sprintf(str,email_template, package, account_no, stb_id);
    }
	// VERIMATRIX CHANGES - BEGIN - added verimatrix network node condition
    else if (ne && strstr(ne,"VM") )
    {
        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        sprintf(str,email_template, account_no, stb_id);
    }	
	// VERIMATRIX CHANGES - END - added verimatrix network node condition
    else
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
        goto CLEANUP;
    }

    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );

    /*
     * Push Message in EMAIL payload
     */
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_MESSAGE, str, ebufp);
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_SUBJECT, "Package removed from your service", ebufp);


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_cancel_plan_notification_buf enriched input flist", *ret_flistpp);

CLEANUP:
    if (email_template)
    {
        free(email_template);
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
    char                    *ne = NULL;
    char                    *email_template = NULL;
    char                    *osd_template = NULL;
    char                    *bmail_template = NULL;
    char                    *account_no = NULL;
    char                    *stb_id = NULL;
    char                    *vc_id = NULL;
    char                    str[1024];
    char                    *bill_no = NULL;
	char					due_date[50] = {""};
	char					curr_date[50] = {""};
    pin_decimal_t           *amount_d = NULL;
    double                  amount;
    int32                   send_osd = PIN_BOOLEAN_FALSE;
    int32                   send_bmail = PIN_BOOLEAN_FALSE;
    void                    *vp = NULL;
	poid_t					*service_pdp = NULL;
	poid_t					*bill_pdp = NULL;
	time_t					now = 0;
	time_t					*due_t = 0;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);


    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_billing_notification_buf input flist", i_flistp);

    //Get the service poid
	service_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp,
		PIN_FLD_SERVICE_OBJ, 1, ebufp);
	if(PIN_POID_IS_NULL(service_pdp) || 
		!(strcmp(PIN_POID_GET_TYPE(service_pdp), "/service/catv")))
	{
		/*
		 * Validate input received
		 */
		bill_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BILL_NO, 0 , ebufp);
		amount_d = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 0 , ebufp);
		amount = pbo_decimal_to_double(amount_d, ebufp);

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
		// VERIMATRIX CHANGES - BEGIN - added verimatrix network node condition
		else if (ne && strstr(ne,"VM") )
		{
			tmp_flistp = PIN_FLIST_ELEM_GET(tmp1_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
			stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);
		}	
		// VERIMATRIX CHANGES - END - added verimatrix network node condition
		else
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
			goto CLEANUP;
		}

		if (PIN_ERRBUF_IS_ERR(ebufp))
			goto CLEANUP;

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
				goto CLEANUP;
			}

			if (ne && strstr(ne,"NDS") )
			{
				sprintf(str,osd_template, bill_no, account_no, stb_id, vc_id, amount);
			}
			else if (ne && strstr(ne,"CISCO") )
			{
				sprintf(str,osd_template, bill_no, account_no, stb_id, amount);
			}
			// VERIMATRIX CHANGES - added verimatrix network node condition
			else if (ne && strstr(ne,"VM") )
			{
				sprintf(str,osd_template, bill_no, account_no, stb_id, amount);
			}			
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

			if (ne && strstr(ne,"NDS") )
			{
				sprintf(str,osd_template, bill_no, account_no, stb_id, vc_id, amount);
			}
			else if (ne && strstr(ne,"CISCO") )
			{
				sprintf(str,osd_template, bill_no, account_no, stb_id, amount);
			}
			// VERIMATRIX CHANGES - added verimatrix network node condition
			else if (ne && strstr(ne,"VM") )
			{
				sprintf(str,osd_template, bill_no, account_no, stb_id, amount);
			}			
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );
			fm_mso_send_bmail_request(ctxp, p_svc_flistp, str, ne, ebufp);
		}
		/*
		 * get action and NE specific EMAIL template
		 */
		get_email_template(ctxp, i_flistp, &email_template, ne, ebufp);
		if (!email_template)
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
			goto CLEANUP;
		}

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, email_template );
		/*
		 * Prepare Message
		 */
		if (ne && strstr(ne,"NDS") )
		{
			sprintf(str,email_template, bill_no, account_no, stb_id, vc_id, amount);
		}
		else if (ne && strstr(ne,"CISCO") )
		{
			sprintf(str,email_template, bill_no, account_no, stb_id, amount);
		}
		// VERIMATRIX CHANGES - added verimatrix network node condition
		else if (ne && strstr(ne,"VM") )
		{
			sprintf(str,email_template, bill_no, account_no, stb_id, amount);
		}		

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );

		/*
		 * Push Message in EMAIL payload
		 */
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_MESSAGE, str, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_SUBJECT, "Bill Generated", ebufp);


		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"prepare_billing_notification_buf enriched input flist", *ret_flistpp);
	}
	//For BroadBand Service
	else
	{
		bill_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BILL_NO, 0 , ebufp);
		amount_d = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 0 , ebufp);
		amount = pbo_decimal_to_double(amount_d, ebufp);
		due_t = (time_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DUE_T, 0, ebufp);
		//Convert the Due Date to String
		pin_strftimet(due_date, sizeof(due_date), "%d-%b-%y", *due_t);
		//Get the current date
		now = pin_virtual_time(NULL);
		//Convert the current time to string
		pin_strftimet(curr_date, sizeof(curr_date), "%d-%b-%y", now);
		//Get the BB email template
		get_bb_email_template(ctxp, i_flistp, &email_template, ebufp);
		if (!email_template)
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
			goto CLEANUP;
		}

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, email_template );
		/*
		 * Prepare Message
		 */
		 sprintf(str, email_template, bill_no, curr_date, amount, due_date);
		 /*
		 * Push Message in EMAIL payload
		 */
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_MESSAGE, str, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_SUBJECT, "Bill Generated", ebufp);


		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"prepare_billing_notification_buf enriched input flist", *ret_flistpp);
	}
CLEANUP:
    if (email_template)
    {
        free(email_template);
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
    char                    *ne = NULL;
    char                    *email_template = NULL;
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
    bill_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BILL_NO, 0 , ebufp);
    due_d = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DUE, 0 , ebufp);
    due = pbo_decimal_to_double(due_d, ebufp);
    due_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DUE_T, 0 , ebufp);

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
    get_email_template(ctxp, i_flistp, &email_template, ne, ebufp);
    if (!email_template)
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
        goto CLEANUP;
    }

    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, email_template );
    /*
     * Prepare Message
     */
    if (ne && strstr(ne,"NDS") )
    {
        sprintf(str,email_template, account_no, due, bill_no, due_date);
    }
    else if (ne && strstr(ne,"CISCO") )
    {
        sprintf(str,email_template, account_no, due, bill_no, due_date);
    }
	// VERIMATRIX CHANGES - added verimatrix network node condition
    else if (ne && strstr(ne,"VM") )
    {
        sprintf(str,email_template, account_no, due, bill_no, due_date);
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
     * Push Message in EMAIL payload
     */
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_MESSAGE, str, ebufp);
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_SUBJECT, "Payment Reminder", ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_due_reminder_notification_buf enriched input flist", *ret_flistpp);
CLEANUP:
    if (email_template)
    {
        free(email_template);
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
    pin_flist_t             *ai_flistp = NULL;
    pin_flist_t             *ao_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
	poid_t					*service_pdp = NULL;
    char                    *ne = NULL;
    char                    *email_template = NULL;
    char                    *account_no = NULL;
    char                    *stb_id = NULL;
    char                    *vc_id = NULL;
    char                    str[1024];
    char                    *trans_id = NULL;
	char					curr_date[50] = {""};
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
        "prepare_payment_notification_buf input flist", i_flistp);

    /*
     * Validate input received
     */
    trans_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_TRANS_ID, 0 , ebufp);
    amount_d = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 0 , ebufp);
    amount = pbo_decimal_to_double(amount_d, ebufp);
    trans_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACK_TIME, 0 , ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

     //Get the service poid
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
	//    tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, PIN_ELEMID_ANY, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"prepare_payment_notification_buf PCM_OP_READ_FLDS input flist", si_flistp);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"prepare_payment_notification_buf PCM_OP_READ_FLDS output flist", so_flistp);

		tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
		ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 

		/*
		 * get action and NE specific EMAIL template
		 */
		get_email_template(ctxp, i_flistp, &email_template, ne, ebufp);
		if (!email_template)
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
			goto CLEANUP;
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, email_template );

		/*
		 * Read account no
		 */
		ai_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ai_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(ai_flistp, PIN_FLD_ACCOUNT_NO, NULL, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"prepare_payment_notification_buf PCM_OP_READ_FLDS input flist", ai_flistp);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, ai_flistp, &ao_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"prepare_payment_notification_buf PCM_OP_READ_FLDS output flist", ao_flistp);

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

			sprintf(str,email_template, amount, trans_date, trans_id, account_no);
		}
		else if (ne && strstr(ne,"CISCO") )
		{
	//        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 2, 0, ebufp);
	//        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

			sprintf(str,email_template, amount, trans_date, trans_id, account_no);
		}
		// VERIMATRIX CHANGES - added verimatrix network node condition
		else if (ne && strstr(ne,"VM") )
		{
			sprintf(str,email_template, amount, trans_date, trans_id, account_no);
		}		
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
		pin_strftimet(curr_date, sizeof(curr_date), "%d-%b-%y", *trans_t);
		get_bb_email_template(ctxp, i_flistp, &email_template, ebufp);
		if (!email_template)
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
			goto CLEANUP;
		}

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, email_template );
		/*
		 * Prepare Message
		 */
		 sprintf(str, email_template, amount, curr_date, trans_id);
	}
    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );

    /*
     * Push Message in EMAIL payload
     */
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_MESSAGE, str, ebufp);
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_SUBJECT, "Payment Successful", ebufp);


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_payment_notification_buf enriched input flist", *ret_flistpp);

CLEANUP:
    if (email_template)
    {
        free(email_template);
    }

    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&ai_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&ao_flistp, NULL);
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
	poid_t					*service_pdp = NULL;
    char                    *ne = NULL;
    char                    *email_template = NULL;
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

     //Get the service poid
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
	//    tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, PIN_ELEMID_ANY, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"prepare_payment_reversal_notification_buf PCM_OP_READ_FLDS input flist", si_flistp);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"prepare_payment_reversal_notification_buf PCM_OP_READ_FLDS output flist", so_flistp);

		tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
		ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 

		/*
		 * get action and NE specific EMAIL template
		 */
		get_email_template(ctxp, i_flistp, &email_template, ne, ebufp);
		if (!email_template)
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
			goto CLEANUP;
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, email_template );

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

			sprintf(str,email_template, amount, trans_date, trans_id, account_no);
		}
		else if (ne && strstr(ne,"CISCO") )
		{
	//        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 2, 0, ebufp);
	//        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

			sprintf(str,email_template, amount, trans_date, trans_id, account_no);
		}
		// VERIMATRIX CHANGES - added verimatrix network node condition
		else if (ne && strstr(ne,"VM") )
		{
            sprintf(str,email_template, amount, trans_date, trans_id, account_no);
		}		
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
		pin_strftimet(trans_date, sizeof(trans_date), "%d-%b-%y", *trans_t);
		get_bb_email_template(ctxp, i_flistp, &email_template, ebufp);
		if (!email_template)
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
			goto CLEANUP;
		}

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, email_template );
		/*
		 * Prepare Message
		 */
		 sprintf(str, email_template, amount, trans_date, trans_id);
	}
    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );

    /*
     * Push Message in EMAIL payload
     */
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_MESSAGE, str, ebufp);
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_SUBJECT, "Payment Reversal Successful", ebufp);


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_payment_reversal_notification_buf enriched input flist", *ret_flistpp);

CLEANUP:
    if (email_template)
    {
        free(email_template);
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
	pin_flist_t				*i_flistp, 
	pin_flist_t				**payload_flistp, 
	pin_errbuf_t			*ebufp)
{
	char					*passwd = NULL;
	char                    *email_template = NULL;
	char                    str[1024];
	if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);


    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_change_passwd_notification_buf input flist", i_flistp);
	passwd = (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PASSWD, 0, ebufp);
	get_bb_email_template(ctxp, i_flistp, &email_template, ebufp);
	if (!email_template)
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
		goto CLEANUP;
	}

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, email_template );
	/*
	 * Prepare Message
	 */
	sprintf(str, email_template, passwd);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );
    /*
     * Push Message in EMAIL payload
     */
    PIN_FLIST_FLD_SET(*payload_flistp, PIN_FLD_MESSAGE, str, ebufp);
    PIN_FLIST_FLD_SET(*payload_flistp, PIN_FLD_SUBJECT, "Password Change Successful", ebufp);


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_change_passwd_notification_buf enriched input flist", *payload_flistp);

CLEANUP:
    if (email_template)
    {
        free(email_template);
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
    char                    *email_template = NULL;
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
     * get action and NE specific EMAIL template
     */
    get_email_template(ctxp, i_flistp, &email_template, ne, ebufp);
    if (!email_template)
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
        goto CLEANUP;
    }
    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, email_template );

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

        sprintf(str,email_template, amount, trans_date, refund_id, account_no);
    }
    else if (ne && strstr(ne,"CISCO") )
    {
//        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 2, 0, ebufp);
//        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        sprintf(str,email_template, amount, trans_date, refund_id, account_no);
    }
	// VERIMATRIX CHANGES - added verimatrix network node condition
    else if (ne && strstr(ne,"VM") )
    {
        sprintf(str,email_template, amount, trans_date, refund_id, account_no);
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
     * Push Message in EMAIL payload
     */
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_MESSAGE, str, ebufp);
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_SUBJECT, "Depoist refund successful", ebufp);


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_deposit_refund_notification_buf enriched input flist", *ret_flistpp);

CLEANUP:
    if (email_template)
    {
        free(email_template);
    }

    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&ai_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&ao_flistp, NULL);
    return;
}

void
get_email_template(
        pcm_context_t       *ctxp,
        pin_flist_t         *i_flistp,
        char                **email_template,
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
        "get_email_template input flist", i_flistp);

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
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_DOMAIN, "EMAIL_TEMPLATE", ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STRING_ID, string_id, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 3, ebufp);
    if (strstr(ne,"NDS") )
    {
        str_ver =  1;
    }
    else if (strstr(ne,"CISCO") )
    {
        str_ver =  2;
    }
	// VERIMATRIX CHANGES - added verimatrix network node condition
	else if (strstr(ne,"VM") )
    {
        str_ver =  3;
    }	
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STR_VERSION, &str_ver, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_STRING, NULL, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "get_email_template search input flist ", search_i_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_i_flistp,
        &search_o_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "get_email_template search output flist", search_o_flistp);

    tmp_flistp = PIN_FLIST_ELEM_GET(search_o_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
    msg_template = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_STRING, 0, ebufp);

    if (msg_template && strlen(msg_template) > 0 )
    {
         *email_template = (char *) malloc(strlen(msg_template)+1);
         memset(*email_template, 0, sizeof(msg_template)+1);
         strcpy(*email_template,msg_template);
         PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*email_template );
    }

    PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&search_o_flistp, NULL);

    return;
}

static void
get_bb_email_template(
	pcm_context_t		*ctxp, 
	pin_flist_t			*i_flistp, 
	char				**email_template, 
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
        "get_bb_email_template input flist", i_flistp);

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
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_DOMAIN, "BB_EMAIL_TEMPLATE", ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STRING_ID, string_id, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_STRING, NULL, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "get_bb_email_template search input flist ", search_i_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_i_flistp,
        &search_o_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "get_bb_email_template search output flist", search_o_flistp);

    tmp_flistp = PIN_FLIST_ELEM_GET(search_o_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
    msg_template = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_STRING, 0, ebufp);

    if (msg_template && strlen(msg_template) > 0 )
    {
         *email_template = (char *) malloc(strlen(msg_template)+1);
         memset(*email_template, 0, sizeof(msg_template)+1);
         strcpy(*email_template,msg_template);
         PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*email_template );
    }

    PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&search_o_flistp, NULL);

    return;
}

/*******************************************************************
 * prepare_change_plan_notification_buf()
 *
 * Enrich input filst with service details required
 * optionally performa validation on input fields
 *******************************************************************/
static void
prepare_upgrade_plan_notification_buf(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t             **ret_flistpp,
	pin_errbuf_t            *ebufp)
{

	pin_flist_t		*help_flistp = NULL;	
	pin_flist_t		*result_flistp = NULL;	
	int32			*num_months = NULL;
	pin_decimal_t		*amountp  = NULL;
	char			*email_template = NULL;
	char			*city = NULL;
	char			*phone = NULL;
	char			str[1024];

	double			amount = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	return;

	PIN_ERRBUF_CLEAR(ebufp);

	/***********************************************************
	* Log input flist
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_upgrade_plan_notification_buf input flist", i_flistp);

	/*
	* Validate input received
	*/
	amountp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 0, ebufp);
	amount = pbo_decimal_to_double(amountp, ebufp);
	num_months = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_NUM_UNITS, 0, ebufp);
	city = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CITY, 0, ebufp);
	if(city){
		fm_mso_utils_get_helpdesk_details(ctxp, city, &help_flistp, ebufp);
		if(PIN_ERRBUF_IS_ERR(ebufp)){
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in getting helpdesk details", ebufp);
			goto CLEANUP;
		}
		if(help_flistp && PIN_FLIST_ELEM_COUNT(help_flistp, PIN_FLD_RESULTS, ebufp) > 0 ){
			result_flistp = PIN_FLIST_ELEM_GET(help_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);			
			if(result_flistp){
				phone = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_PHONE, 1, ebufp);
			}
		}
	}
	get_bb_email_template(ctxp, i_flistp, &email_template, ebufp);
	if (!email_template)
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_NOT_FOUND, PIN_FLD_TEMPLATE, 0, 0);
		goto CLEANUP;
	}

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, email_template );
	/*
	 * Prepare Message
	 */
	sprintf(str, email_template, amount, *num_months, phone);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str );

	/*
	* Push Message in EMAIL payload
	*/
	PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_MESSAGE, str, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_SUBJECT, "Plan change completed for your service", ebufp);


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_change_plan_notification_buf enriched input flist", *ret_flistpp);

CLEANUP:
	if (email_template)
	{
		free(email_template);
	}
	PIN_FLIST_DESTROY_EX(&help_flistp, NULL);

    return;
}
