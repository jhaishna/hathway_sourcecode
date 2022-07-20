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
* 0.1    |25/10/2013 |Satya Prakash     |  CATV Provisioning implementation 
* 0.2	 |04/08/2014 |Sachidanand Joshi | Added provisioning call for /mso_catv_preactivated_svc 
*--------------------------------------------------------------------------------------------------------------*
*
****************************************************************************************************************/


#ifndef lint
static  char Sccs_Id[] = "@(#)$Id: fm_mso_prov_create_action.c /cgbubrm_7.3.2.rwsmod/1 2009/08/04 23:37:19 pjegan Exp $";
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

EXPORT_OP void
op_mso_prov_create_action(
        cm_nap_connection_t	*connp,
		int32			opcode,
        int32			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t	*ebufp);

static void
fm_mso_prov_create_action(
        cm_nap_connection_t	*connp,
		pin_flist_t		*in_flistp,
        int32			flags,
		pin_flist_t		**out_flistpp,
	    pin_errbuf_t	*ebufp);

/*******************************************************************
 * Main routine for the MSO_OP_PROV_CREATE_ACTION command
 *******************************************************************/
void
op_mso_prov_create_action(
        cm_nap_connection_t	*connp,
		int32			opcode,
        int32			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t	*ebufp)
{
	pin_flist_t		*r_flistp = NULL;

	/***********************************************************
	 * Null out results until we have some.
	 ***********************************************************/
	*ret_flistpp = NULL;
	PIN_ERRBUF_CLEAR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != MSO_OP_PROV_CREATE_ACTION) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_mso_prov_create_action", ebufp);
		return;
	}

	/***********************************************************
	 * Log input flist
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_mso_prov_create_action input flist", in_flistp);

	/***********************************************************
	 * Call main function to do it
	 ***********************************************************/
	fm_mso_prov_create_action(connp, in_flistp, flags, &r_flistp, ebufp);

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*ret_flistpp = (pin_flist_t *)NULL;
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_prov_create_action error", ebufp);
	} else {
		*ret_flistpp = r_flistp;
		PIN_ERRBUF_CLEAR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_mso_prov_create_action return flist", r_flistp);
	}

	return;
}

/*******************************************************************
 * fm_mso_prov_create_action()
 *
 *	Prep the login to be ready for on-line registration.
 *
 *******************************************************************/
static void
fm_mso_prov_create_action(
    cm_nap_connection_t	*connp,
	pin_flist_t			*in_flistp,
	int32               flags,
	pin_flist_t			**out_flistpp,
    pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	char				*service_type = NULL;
	poid_t			*serv_poid_p = NULL;
	char			msg[1024];
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);

	serv_poid_p = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
	service_type = (char *) PIN_POID_GET_TYPE(serv_poid_p);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_prov_create_action error: PIN_FLD_TYPE_STR required in input flist", ebufp);
		return;
	}

	

	sprintf(msg, "service_type = %s",service_type);
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG,msg,serv_poid_p);
	/***********************************************************
	 * call service specific opcode
	 ***********************************************************/
	if (!strcmp(service_type, "/service/catv") || !strcmp(service_type,"/mso_catv_preactivated_svc")) 
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Inside catv");
		PCM_OP(ctxp, MSO_OP_PROV_CREATE_CATV_ACTION, 0, in_flistp, out_flistpp, ebufp)
	} 
	else if (!strcmp(service_type, "/service/telco/broadband"))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Inside broadband");
		PCM_OP(ctxp, MSO_OP_PROV_CREATE_BB_ACTION, 0, in_flistp, out_flistpp, ebufp)
	}
	
	return;
}

