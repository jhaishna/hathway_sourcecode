/*
 * @(#)%Portal Version: fm_device_pol_set_state.c:ServerIDCVelocityInt:4:2006-Sep-14 11:35:18 %
 *
 * Copyright (c) 2001 - 2006 Oracle. All rights reserved.
 *
 * This material is the confidential property of Oracle Corporation
 * or its licensors and may be used, reproduced, stored or transmitted
 * only in accordance with a valid Oracle license or sublicense agreement.
 *
 */

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_device_pol_set_state.c:ServerIDCVelocityInt:4:2006-Sep-14 11:35:18 %";
#endif

/*******************************************************************
 * Contains the PCM_OP_DEVICE_POL_SET_STATE operation.
 *******************************************************************/

#include <stdio.h>
#include <string.h>

#include <pcm.h>
#include <pinlog.h>

#define FILE_LOGNAME "fm_device_pol_set_state.c(1)"

#include "ops/device.h"
#include "pin_device.h"
#include "pin_apn.h"
#include "pin_ip.h"
#include "ops/bel.h"
#include "ops/apn.h"
#include "ops/ip.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_channel.h"
#include "fm_utils.h"
#include "mso_device.h"
#include "mso_ops_flds.h"

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_device_pol_set_state(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_device_pol_set_state(
	cm_nap_connection_t	*connp,
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_device_pol_lifecycle_create(
	cm_nap_connection_t	*connp,
	pcm_context_t        *ctxp,
	pin_flist_t        *i_flistp,
	pin_errbuf_t        *ebufp);

/*******************************************************************
 * Routines needed from elsewhere.
 *******************************************************************/

/*******************************************************************
 * Main routine for the PCM_OP_DEVICE_POL_SET_STATE operation.
 *******************************************************************/
void
op_device_pol_set_state(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t	*r_flistp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Null out results
	 ***********************************************************/
	*r_flistpp = NULL;

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_DEVICE_POL_SET_STATE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_device_pol_set_state opcode error", ebufp);

		return;
	}
	
	/***********************************************************
	 * Debut what we got.
         ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_device_pol_set_state input flist", i_flistp);

	/***********************************************************
	 * Main rountine for this opcode
	 ***********************************************************/
	fm_device_pol_set_state(connp, ctxp, flags, i_flistp, r_flistpp, ebufp);
	
	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_FLIST_DESTROY_EX(r_flistpp, ebufp);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_device_pol_set_state error", ebufp);
	} else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_device_pol_set_state output flist", *r_flistpp);
	}

	return;
}

/*******************************************************************
 * fm_device_pol_set_state:
 *
 *******************************************************************/
static void
fm_device_pol_set_state(
	cm_nap_connection_t	*connp,
	pcm_context_t           *ctxp,
	int32                   flags,
	pin_flist_t             *i_flistp,
	pin_flist_t             **r_flistpp,
	pin_errbuf_t            *ebufp)
{

	poid_t          *dev_poidp = NULL;
	char            *poid_type = NULL;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}

	PIN_ERR_CLEAR_ERR(ebufp);
	
	/*
	 * Get the old device object poid
	 */
	dev_poidp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	
	/*
	 * Get the device poid type
	 * this poid will be used in identifiing different Devices like sim,num..
	 */
	poid_type = (char *) PIN_POID_GET_TYPE(dev_poidp);

	if (strcmp(poid_type, PIN_OBJ_TYPE_DEVICE_IP) == 0) {
		/*
		 * call IP policy to do the work we want
		 */
		 PCM_OP(ctxp, PCM_OP_IP_POL_DEVICE_SET_STATE, flags, i_flistp,
			r_flistpp, ebufp);
	} else if (strcmp(poid_type, PIN_OBJ_TYPE_DEVICE_APN) == 0) {
		/*
		 * call APN policy to do the work we want
		 */
		 PCM_OP(ctxp, PCM_OP_APN_POL_DEVICE_SET_STATE, flags, i_flistp,
			r_flistpp, ebufp);
	}
	else {
		/***********************************************************
		 * Prepare return flist
		 ***********************************************************/
		*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
	}
	fm_device_pol_lifecycle_create(connp,ctxp,i_flistp, ebufp);

	return;
}

static void
fm_device_pol_lifecycle_create(
	cm_nap_connection_t	*connp,
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_errbuf_t            *ebufp)
{
	poid_t		*pdp=NULL;
	poid_t		*poid_pdp=NULL;	
	poid_t		*lifecycle_pdp=NULL;
	poid_t		*account_pdp=NULL;
	int64		database=1;
	int64		calling_opcode=2700;
	int32		action_mode=1;
	char		*action = "STATE_CHANGE";
	pin_flist_t	*set_lifecycle_flistp=NULL;
	pin_flist_t	*r_flistp = NULL;
	cm_op_info_t	*opstackp = connp->coip;
	int32		stack_opcode = 0;
	pin_flist_t	*stack_flistp=NULL;
	//int32		*calling_opcode=NULL;
	int32		counter=0;
	int32		flag = SET_STATE;



	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_device_pol_create lifecycle error at begining", ebufp);
		return;
	}
	
	PIN_ERRBUF_CLEAR(ebufp);


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"MSO_OP_LIFECYCLE_DEVICE input flist", i_flistp);

	/*
	while(opstackp != NULL)
	{
		if(counter >2)
			return;
		stack_opcode = opstackp->opcode;
		stack_flistp = opstackp->in_flistp;
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "dev_set_state stack input flist is ", stack_flistp);
		counter++;		
		opstackp = opstackp->next;
	}
	
	if ( stack_opcode != PCM_OP_DEVICE_SET_STATE )
		return;
	*/	

	pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	database = PIN_POID_GET_DB(pdp);  

	/*****************************************************************************
		0 PIN_FLD_POID                POID [0] 0.0.0.1 /mso_lifecycle/device -1 0
		0 PIN_FLD_ACCOUNT_OBJ         POID [0] 0.0.0.1 /account 1 0
		0 PIN_FLD_ACTION               STR [0] "Device loading"
		0 PIN_FLD_DESCR                STR [0] "Device Create: 0.0.0.1 /device/vc 995189 0"
		0 PIN_FLD_PROGRAM_NAME         STR [0] "VC Device loading"
		0 PIN_FLD_SERVICE_OBJ         POID [0] 0.0.0.1 /service/catv 218076
		0 PIN_FLD_DEVICE_ID		STR [0] “1123425565”
		0 PIN_FLD_DEVICE_OBJ		POID [0] 0.0.0.0 /device/modem 11 1
		0 PIN_FLD_STATE_ID		INT [0] 1
	********************************************************************************/
	
	set_lifecycle_flistp = PIN_FLIST_CREATE(ebufp);
	lifecycle_pdp = (poid_t *)PIN_POID_CREATE(database, "/mso_lifecycle/device", -1, ebufp);
	PIN_FLIST_FLD_PUT(set_lifecycle_flistp, PIN_FLD_POID, (void *)lifecycle_pdp, ebufp);
	PIN_FLIST_FLD_SET(set_lifecycle_flistp, PIN_FLD_ACTION,action, ebufp);
	PIN_FLIST_FLD_SET(set_lifecycle_flistp, PIN_FLD_FLAGS, (void *)&flag, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME,set_lifecycle_flistp,PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ,set_lifecycle_flistp,PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ,set_lifecycle_flistp,PIN_FLD_ACCOUNT_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DEVICE_ID,set_lifecycle_flistp,PIN_FLD_DEVICE_ID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID,set_lifecycle_flistp,PIN_FLD_DEVICE_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_NEW_STATE, set_lifecycle_flistp,PIN_FLD_STATE_ID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"MSO_OP_LIFECYCLE_DEVICE input flist",set_lifecycle_flistp);
	
	PCM_OP(ctxp,MSO_OP_LIFECYCLE_DEVICE, flag,set_lifecycle_flistp,&r_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&set_lifecycle_flistp,ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"MSO_OP_LIFECYCLE_DEVICE output flist",r_flistp);	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_FLIST_DESTROY_EX(&r_flistp, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"End of Inside error buffer");
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"op_device_pol_create lifecycle error", ebufp);
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"op_device_pol_create lifecycle output flist", r_flistp);
	}
	
	PIN_FLIST_DESTROY_EX(&r_flistp,ebufp);
	return;
}
