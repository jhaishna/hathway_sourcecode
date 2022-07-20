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
* 0.1    |25/10/2013 |Satya Prakash     |  CATV Notification implementation 
*--------------------------------------------------------------------------------------------------------------*
*
****************************************************************************************************************/


#ifndef lint
static  char Sccs_Id[] = "@(#)$Id: fm_mso_ntf_create_notification.c /cgbubrm_7.3.2.rwsmod/1 2009/08/04 23:37:19 pjegan Exp $";
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

EXPORT_OP void
op_mso_ntf_create_notification(
        cm_nap_connection_t    *connp,
        int32            opcode,
        int32            flags,
        pin_flist_t        *in_flistp,
        pin_flist_t        **ret_flistpp,
        pin_errbuf_t    *ebufp);

static void
fm_mso_ntf_create_notification(
        cm_nap_connection_t    *connp,
        pin_flist_t        *in_flistp,
        int32            flags,
        pin_flist_t        **out_flistpp,
        pin_errbuf_t    *ebufp);

/*******************************************************************
 * Main routine for the MSO_OP_PROV_CREATE_ACTION command
 *******************************************************************/
void
op_mso_ntf_create_notification(
        cm_nap_connection_t    *connp,
        int32            opcode,
        int32            flags,
        pin_flist_t        *in_flistp,
        pin_flist_t        **ret_flistpp,
        pin_errbuf_t    *ebufp)
{
    pin_flist_t        *r_flistp = NULL;

    /***********************************************************
     * Null out results until we have some.
     ***********************************************************/
    *ret_flistpp = NULL;
    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Insanity check.
     ***********************************************************/
    if (opcode != MSO_OP_NTF_CREATE_NOTIFICATION) {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
            PIN_ERR_BAD_OPCODE, 0, 0, opcode);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "bad opcode in op_mso_ntf_create_notification", ebufp);
        return;
    }

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "op_mso_ntf_create_notification input flist", in_flistp);

    /***********************************************************
     * Call main function to do it
     ***********************************************************/
    fm_mso_ntf_create_notification(connp, in_flistp, flags, &r_flistp, ebufp);

    /***********************************************************
     * Results.
     ***********************************************************/
    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        *ret_flistpp = (pin_flist_t *)NULL;
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_mso_ntf_create_notification error", ebufp);
    } else {
        *ret_flistpp = r_flistp;
        PIN_ERRBUF_CLEAR(ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "op_mso_ntf_create_notification return flist", r_flistp);
    }

    return;
}

/*******************************************************************
 * fm_mso_ntf_create_notification()
 *
 *    Prep the login to be ready for on-line registration.
 *
 *******************************************************************/
static void
fm_mso_ntf_create_notification(
    cm_nap_connection_t     *connp,
    pin_flist_t             *in_flistp,
    int32                   flags,
    pin_flist_t             **out_flistpp,
    pin_errbuf_t            *ebufp)
{
    pcm_context_t           *ctxp = connp->dm_ctx;
    char                    *poid_type = NULL;
    poid_t                  *acc_poidp = NULL;
    pin_flist_t             *si_flistp = NULL;
    pin_flist_t             *so_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    pin_flist_t             *tmp2_flistp = NULL;
    int32                   *pref = NULL;
    int32                   rec_id = 0;
    pin_cookie_t            cookie = NULL;
    int32                   *type = NULL;
	int						*flag = NULL;
    char                    *contact_no = NULL;
    char                    *email_addr = NULL;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    acc_poidp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
    poid_type = (char *) PIN_POID_GET_TYPE(acc_poidp);

	//Get the Flag value
	flag = (int *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_FLAGS, 0, ebufp);
    
	/***********************************************************
     * call service specific opcode
     ***********************************************************/
	if (!strcmp(poid_type, "/account")) 
    {

        si_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(si_flistp, PIN_FLD_POID, acc_poidp, ebufp);
        PIN_FLIST_FLD_SET(si_flistp, MSO_FLD_CONTACT_PREF, NULL, ebufp);
	PIN_FLIST_FLD_SET(si_flistp, MSO_FLD_RMN, NULL, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_NAMEINFO, 1, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_EMAIL_ADDR, NULL, ebufp);
        /*tmp2_flistp = PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_PHONES, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(tmp2_flistp, PIN_FLD_PHONE, NULL, ebufp);
        PIN_FLIST_FLD_SET(tmp2_flistp, PIN_FLD_TYPE, NULL, ebufp);
	*/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "fm_mso_ntf_create_notification PCM_OP_READ_FLDS input flist", si_flistp);
        PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "fm_mso_ntf_create_notification PCM_OP_READ_FLDS output flist", so_flistp);

    } 

    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "fm_mso_ntf_create_notification error: Invalid subscriber account", ebufp);
        goto CLEANUP;
    }

    pref = PIN_FLIST_FLD_GET(so_flistp, MSO_FLD_CONTACT_PREF, 1, ebufp);
	
	//Check the Action Flag
	if(*flag == NTF_WELCOME_NOTE)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Welcome Note Email notification");
        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_NAMEINFO, 1, 0, ebufp);
        /*while ( (tmp2_flistp = PIN_FLIST_ELEM_GET_NEXT (tmp_flistp,
					PIN_FLD_PHONES, &rec_id, 1,&cookie, ebufp))
											  != (pin_flist_t *)NULL ) 
		{
			type = PIN_FLIST_FLD_GET(tmp2_flistp, PIN_FLD_TYPE, 0, ebufp);

			if ( *type == 5 )
			{
				contact_no = PIN_FLIST_FLD_GET(tmp2_flistp, PIN_FLD_PHONE, 0, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Phone number found");
				break;
			}
		}*/
	email_addr = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_EMAIL_ADDR, 0, ebufp);
	contact_no = PIN_FLIST_FLD_GET(so_flistp, MSO_FLD_RMN, 0, ebufp);
        if (email_addr && strlen(email_addr) > 0 && contact_no && strlen(contact_no) > 0)
        {
            PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_EMAIL_ADDR, email_addr, ebufp);
			PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_PHONE, contact_no, ebufp);
            PCM_OP(ctxp, MSO_OP_NTF_CREATE_WELCOME_NOTE, 0, in_flistp, out_flistpp, ebufp);
        }
        else
        {
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    PIN_ERR_BAD_VALUE, PIN_FLD_EMAIL_ADDR, 0, 0);
            goto CLEANUP;
        }
	}
	else
	{
		if (pref && *pref == 0) //SMS
		{   
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"SMS notification");
			tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_NAMEINFO, 1, 0, ebufp);

			/*while ( (tmp2_flistp = PIN_FLIST_ELEM_GET_NEXT (tmp_flistp,
					PIN_FLD_PHONES, &rec_id, 1,&cookie, ebufp))
											  != (pin_flist_t *)NULL ) 
			{
				type = PIN_FLIST_FLD_GET(tmp2_flistp, PIN_FLD_TYPE, 0, ebufp);

				if ( *type == 5 )
				{
					contact_no = PIN_FLIST_FLD_GET(tmp2_flistp, PIN_FLD_PHONE, 0, ebufp);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Phone number found");
					break;
				}
			}*/
			contact_no = PIN_FLIST_FLD_GET(so_flistp, MSO_FLD_RMN, 0, ebufp);

			if (contact_no && strlen(contact_no) > 0 )
			{
				PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_PHONE, contact_no, ebufp);
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"fm_mso_ntf_create_notification enriched input flist", in_flistp);
				PCM_OP(ctxp, MSO_OP_NTF_CREATE_SMS_NOTIFICATION, 0, in_flistp, out_flistpp, ebufp);
			}
			else
			{
				pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
						PIN_ERR_BAD_VALUE, PIN_FLD_PHONE, 0, 0);
				goto CLEANUP;
			}
		}
		else if (pref && *pref == 1 )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Email notification");
			tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_NAMEINFO, 1, 0, ebufp);
			email_addr = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_EMAIL_ADDR, 0, ebufp);
			/*while ( (tmp2_flistp = PIN_FLIST_ELEM_GET_NEXT (tmp_flistp,
					PIN_FLD_PHONES, &rec_id, 1,&cookie, ebufp))
											  != (pin_flist_t *)NULL ) 
			{
				type = PIN_FLIST_FLD_GET(tmp2_flistp, PIN_FLD_TYPE, 0, ebufp);

				if ( *type == 5 )
				{
					contact_no = PIN_FLIST_FLD_GET(tmp2_flistp, PIN_FLD_PHONE, 0, ebufp);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Phone number found");
					break;
				}
			}*/
			if (email_addr && strlen(email_addr) > 0 )
			{
				if (*flag != NTF_BB_PAYMENT_CASH && *flag != NTF_BB_PAYMENT_CHEQUE 
					&& *flag != NTF_BB_PAYMENT_ONLINE)
				{
					PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_EMAIL_ADDR, email_addr, ebufp);
					PCM_OP(ctxp, MSO_OP_NTF_CREATE_EMAIL_NOTIFICATION, 0, in_flistp, out_flistpp, ebufp);
				}
			}
			else
			{
				pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
						PIN_ERR_BAD_VALUE, PIN_FLD_EMAIL_ADDR, 0, 0);
				goto CLEANUP;
			}
		        contact_no = PIN_FLIST_FLD_GET(so_flistp, MSO_FLD_RMN, 0 , ebufp);

                        if (contact_no && strlen(contact_no) > 0 )
                        {
                                PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_PHONE, contact_no, ebufp);
                                                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                "fm_mso_ntf_create_notification enriched input flist", in_flistp);
                                PCM_OP(ctxp, MSO_OP_NTF_CREATE_SMS_NOTIFICATION, 0, in_flistp, out_flistpp, ebufp);
                        }
                        else
                        {
                                pin_set_err(ebufp, PIN_ERRLOC_FM,
                                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                                                PIN_ERR_BAD_VALUE, PIN_FLD_PHONE, 0, 0);
                                goto CLEANUP;
                        }
	
		}
		else if (pref && *pref == 2) //Both Email and SMS
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"SMS & Email notification");
			tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_NAMEINFO, 1, 0, ebufp);
			email_addr = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_EMAIL_ADDR, 1, ebufp);

			if (email_addr && strlen(email_addr) > 0 && *flag != NTF_BB_PAYMENT_CASH 
				&& *flag != NTF_BB_PAYMENT_CHEQUE && *flag != NTF_BB_PAYMENT_ONLINE)
			{
				PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_EMAIL_ADDR, email_addr, ebufp);
				PCM_OP(ctxp, MSO_OP_NTF_CREATE_EMAIL_NOTIFICATION, 0, in_flistp, out_flistpp, ebufp);
			}
			else if (email_addr == NULL || (email_addr && strlen(email_addr) == 0))
			{
				pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
						PIN_ERR_BAD_VALUE, PIN_FLD_EMAIL_ADDR, 0, 0);
				goto CLEANUP;
			}
			//Send SMS Notification
		/*	while ( (tmp2_flistp = PIN_FLIST_ELEM_GET_NEXT (tmp_flistp,
					PIN_FLD_PHONES, &rec_id, 1,&cookie, ebufp))
											  != (pin_flist_t *)NULL ) 
			{
				type = PIN_FLIST_FLD_GET(tmp2_flistp, PIN_FLD_TYPE, 0, ebufp);

				if ( *type == 5 )
				{
					contact_no = PIN_FLIST_FLD_GET(tmp2_flistp, PIN_FLD_PHONE, 0, ebufp);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Phone number found");
					break;
				}
			}*/
			contact_no = PIN_FLIST_FLD_GET(so_flistp, MSO_FLD_RMN, 0 , ebufp);

			if (contact_no && strlen(contact_no) > 0 )
			{
				PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_PHONE, contact_no, ebufp);
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"fm_mso_ntf_create_notification enriched input flist", in_flistp);
				PCM_OP(ctxp, MSO_OP_NTF_CREATE_SMS_NOTIFICATION, 0, in_flistp, out_flistpp, ebufp);
			}
			else
			{
				pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
						PIN_ERR_BAD_VALUE, PIN_FLD_PHONE, 0, 0);
				goto CLEANUP;
			}

		}
		else
		{
    			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_WARNING,
        			"Invalid MSO_FLD_CONTACT_PREF op_mso_ntf_create_notification", in_flistp);
		}
	}

CLEANUP:
    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&so_flistp, NULL);

        
    return;
}

