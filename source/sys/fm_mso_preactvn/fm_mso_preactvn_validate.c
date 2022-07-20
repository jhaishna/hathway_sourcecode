/*
 * (#)$Id: fm_mso_preactvn_modify_srvc.c Sachidanand Joshi 2014/07/14 16:37:48 sacjoshi $
 *
 * Copyright (c) 2001-2014, Oracle and/or its affiliates. All rights reserved.
 *
 * This material is the confidential property of Oracle Corporation or its
 * licensors and may be used, reproduced, stored or transmitted only in
 * accordance with a valid Oracle license or sublicense agreement.
 *
 */
/***************************************************************************************************************
*VERSION |   DATE    |    Author        |               DESCRIPTION                                            *
*--------------------------------------------------------------------------------------------------------------*
* 0.1    |14/07/2014 |Sachidanand Joshi | Modifications to preactivated services
*
*--------------------------------------------------------------------------------------------------------------*
****************************************************************************************************************/


#ifndef lint
static const char Sccs_id[] = "(#)$Id: fm_mso_preactvn_modify_srvc.c Sachidanand Joshi 2014/07/14 16:37:48 sacjoshi $ ";
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <pcm.h>
#include <pinlog.h>

#include "ops/device.h"
#include "pin_device.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "fm_utils.h"
#include "pin_cust.h"
#include "mso_device.h"
#include "mso_prov.h"
#include "mso_ops_flds.h"


/*******************************************************************************
 * Functions contained within this source file
 ******************************************************************************/
int
fm_mso_preactvn_validate_changeplan_inp(
    pcm_context_t        *ctxp,
    pin_flist_t        *i_flistp,
    pin_flist_t        **ret_flistpp,
    pin_errbuf_t        *ebufp);

int
fm_mso_preactvn_validate_suspndsvc_inp(
    pcm_context_t        *ctxp,
    pin_flist_t        *i_flistp,
    pin_flist_t        **ret_flistpp,
    pin_errbuf_t        *ebufp);


/*******************************************************************************
 * Sachid: fm_mso_preactvn_validate_changeplan_inp
 *
 * Description:
 * Validate the input of change plan operation
 * Return 0 if validation fails, else 1
 * Todo:
 *
 ******************************************************************************/
int
fm_mso_preactvn_validate_changeplan_inp(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp){


	pin_flist_t	*srch_iflp = NULL;
	pin_flist_t	*srch_oflp = NULL;
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*res_flp = NULL;
	pin_flist_t	*planlist_flp = NULL;
	pin_flist_t	*tmp_flp = NULL;
	pin_flist_t	*wrt_flp = NULL;
	pin_flist_t	*wrt_oflp = NULL;
	pin_flist_t	*robj_oflp = NULL;
	pin_flist_t	*provaction_inflistp = NULL;
	pin_flist_t	*taskelem_flp = NULL;
	pin_flist_t	*delpkg_flp = NULL;
	pin_flist_t	*provaction_outflistp = NULL;
	pin_flist_t	*validate_flp = NULL;
	
	void		*vp = NULL;
	int		change_plan_failure = 1;
	char		msng_fldp[1024];
	char		func_name[] = "fm_mso_preactvn_validate_changeplan_inp";
	char		msg[1024];


   	if (PIN_ERRBUF_IS_ERR(ebufp)){
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_preactvn_validate_changeplan_inp: Error before starting the function",ebufp);
        	return 0;
	}
	PIN_ERRBUF_CLEAR(ebufp);
		
	sprintf(msg,"%s: Input flist", func_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg, i_flistp);
	/*******************************************************************
     	 *   # number of field entries allocated 20, used 9
     	 *   0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 987643 1
     	 *   0 PIN_FLD_USERID         POID [0] 0.0.0.1 /account 80076 1
     	 *   0 PIN_FLD_ACTION_MODE    ENUM [0] 101
     	 *   0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /mso_catv_preactivated_svc 628964 1
     	 *   0 PIN_FLD_PROGRAM_NAME    STR [0] "OAP|suresh001"
     	 *   0 MSO_FLD_NETWORK_NODE STR [0] "NDS"
     	 *   0 PIN_FLD_PLAN_LISTS    ARRAY [0] allocated 1, used 1
     	 *   1    PIN_FLD_FLAGS        INT [0] 0
     	 *   1    MSO_FLD_CA_ID        STR [0] "253"
     	 *   0 PIN_FLD_PLAN_LISTS    ARRAY [1] allocated 1, used 1
     	 *   1    PIN_FLD_FLAGS        STR [0] 1
     	 *   1    PIN_FLD_PLAN_OBJ     STR [0] 0.0.0.1 /plan 41089 8
     	 *********************************************************************/

	/* Validate mandatory fields */
	sprintf(msg,"%s: Validating mandatory fields", func_name);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , msg);
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTION_MODE, 0, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp)){
		sprintf(msng_fldp,"PIN_FLD_ACTION_MODE"); goto ERROR;
	}
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp)){
		sprintf(msng_fldp , "PIN_FLD_SERVICE_OBJ"); goto ERROR;
	}
	vp = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp)){
		sprintf(msng_fldp , "MSO_FLD_NETWORK_NODE"); goto ERROR;
	}
	validate_flp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PLAN_LISTS,0, 0, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp)){
		sprintf(msng_fldp , "PIN_FLD_PLAN_LISTS,0"); goto ERROR;
	}
	vp = PIN_FLIST_FLD_GET(validate_flp, PIN_FLD_FLAGS, 0, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp)){
		sprintf(msng_fldp , "PIN_FLD_FLAGS"); goto ERROR;
	}
	vp = PIN_FLIST_FLD_GET(validate_flp, MSO_FLD_CA_ID, 0, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp)){
		sprintf(msng_fldp , "MSO_FLD_CA_ID"); goto ERROR;
	}
	validate_flp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PLAN_LISTS,1, 0, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp)){
		sprintf(msng_fldp , "PIN_FLD_PLAN_LISTS,1"); goto ERROR;
	}
	vp = PIN_FLIST_FLD_GET(validate_flp, PIN_FLD_FLAGS, 0, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp)){
		sprintf(msng_fldp , "PIN_FLD_FLAGS"); goto ERROR;
	}
	vp = PIN_FLIST_FLD_GET(validate_flp, PIN_FLD_PLAN_OBJ, 0, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp)){
		sprintf(msng_fldp , "PIN_FLD_PLAN_OBJ"); goto ERROR;
	}
	ERROR:
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
		sprintf(msg,"%s: Error: Mandatory field missing for Change Plan",func_name);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, msg, ebufp);
                PIN_ERRBUF_RESET(ebufp);

                *r_flistpp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51416", ebufp);
		sprintf(msg,"Error: Missing mandatory field: %s",msng_fldp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, msg, ebufp);

                return 0;
        }
        return 1;

}
 
			

/*******************************************************************************
 * Sachid: fm_mso_preactvn_validate_suspndsvc_inp
 *
 * Description:
 * Validate the input of Suspend service operation
 * Return 0 if validation fails, else 1
 * Todo:
 *
 ******************************************************************************/
int
fm_mso_preactvn_validate_suspndsvc_inp(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp){


	pin_flist_t	*srch_iflp = NULL;
	pin_flist_t	*srch_oflp = NULL;
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*res_flp = NULL;
	pin_flist_t	*planlist_flp = NULL;
	pin_flist_t	*tmp_flp = NULL;
	pin_flist_t	*wrt_flp = NULL;
	pin_flist_t	*wrt_oflp = NULL;
	pin_flist_t	*robj_oflp = NULL;
	pin_flist_t	*provaction_inflistp = NULL;
	pin_flist_t	*taskelem_flp = NULL;
	pin_flist_t	*delpkg_flp = NULL;
	pin_flist_t	*provaction_outflistp = NULL;
	pin_flist_t	*validate_flp = NULL;
	
	void		*vp = NULL;
	int		change_plan_failure = 1;
	char		msng_fldp[1024];
	char		func_name[] = "fm_mso_preactvn_validate_suspndsvc_inp";
	char		msg[1024];


   	if (PIN_ERRBUF_IS_ERR(ebufp)){
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_preactvn_validate_suspndsvc_inp: Error before starting the function",ebufp);
        	return 0;
	}
	PIN_ERRBUF_CLEAR(ebufp);
		
	sprintf(msg,"%s: Input flist", func_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg, i_flistp);
	/*******************************************************************
     	 *   # number of field entries allocated 20, used 9
     	 *   0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 987643 1
     	 *   0 PIN_FLD_USERID         POID [0] 0.0.0.1 /account 80076 1
     	 *   0 PIN_FLD_ACTION_MODE    ENUM [0] 101
     	 *   0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /mso_catv_preactivated_svc 628964 1
     	 *   0 PIN_FLD_PROGRAM_NAME    STR [0] "OAP|suresh001"
     	 *   0 MSO_FLD_NETWORK_NODE STR [0] "NDS"
     	 *   0 PIN_FLD_PLAN_LISTS    ARRAY [0] allocated 1, used 1
     	 *   1    MSO_FLD_CA_ID        STR [0] "253"
     	 *********************************************************************/

	/* Validate mandatory fields */
	sprintf(msg,"%s: Validating mandatory fields", func_name);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , msg);
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTION_MODE, 0, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp)){
		sprintf(msng_fldp,"PIN_FLD_ACTION_MODE"); goto ERROR;
	}
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp)){
		sprintf(msng_fldp , "PIN_FLD_SERVICE_OBJ"); goto ERROR;
	}
	vp = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp)){
		sprintf(msng_fldp , "MSO_FLD_NETWORK_NODE"); goto ERROR;
	}

	validate_flp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PLAN_LISTS,0, 0, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp)){
		sprintf(msng_fldp , "PIN_FLD_PLAN_LISTS,0"); goto ERROR;
	}
	vp = PIN_FLIST_FLD_GET(validate_flp, MSO_FLD_CA_ID, 0, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp)){
		sprintf(msng_fldp , "MSO_FLD_CA_ID"); goto ERROR;
	}
/*
	vp = PIN_FLIST_FLD_GET(validate_flp, PIN_FLD_FLAGS, 0, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp)){
		sprintf(msng_fldp , "PIN_FLD_FLAGS"); goto ERROR;
	}
	validate_flp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PLAN_LISTS,1, 0, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp)){
		sprintf(msng_fldp , "PIN_FLD_PLAN_LISTS,1"); goto ERROR;
	}
	vp = PIN_FLIST_FLD_GET(validate_flp, PIN_FLD_FLAGS, 0, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp)){
		sprintf(msng_fldp , "PIN_FLD_FLAGS"); goto ERROR;
	}
	vp = PIN_FLIST_FLD_GET(validate_flp, PIN_FLD_PLAN_OBJ, 0, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp)){
		sprintf(msng_fldp , "PIN_FLD_PLAN_OBJ"); goto ERROR;
	}
*/
	ERROR:
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
		sprintf(msg,"%s: Error: Mandatory field missing for Suspend Preactivated Service",func_name);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, msg, ebufp);
                PIN_ERRBUF_RESET(ebufp);

                *r_flistpp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51416", ebufp);
				sprintf(msg,"Error: Missing mandatory field: %s",msng_fldp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, msg, ebufp);

                return 0;
        }
        return 1;

}

	
