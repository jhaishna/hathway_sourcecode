/*
 * (#)$Id: fm_mso_lifecycle_device.c st_cgbubrm_gsivakum_bug-8453030/1 2009/05/05 23:37:48 gsivakum Exp $
 *
 * Copyright (c) 2001, 2009, Oracle and/or its affiliates. All rights reserved.
 *
 * This material is the confidential property of Oracle Corporation or its
 * licensors and may be used, reproduced, stored or transmitted only in
 * accordance with a valid Oracle license or sublicense agreement.
 *
 */
/***************************************************************************************************************
*VERSION |   DATE    |    Author        |               DESCRIPTION                                            *
*--------------------------------------------------------------------------------------------------------------*
* 0.1    |12/12/2013 |Satya Prakash     | CATV Smart Card Blacklisting Implementation
* 0.2    |20/11/2014 |Someshwar         | Updated code for BB devices
*--------------------------------------------------------------------------------------------------------------*
****************************************************************************************************************/

#ifndef lint
static const char Sccs_id[] = "(#)$Id: fm_mso_lifecycle_device.c st_cgbubrm_gsivakum_bug-8453030/1 2009/05/05 23:37:48 gsivakum Exp $";
#endif

#include <stdio.h>
#include <string.h>
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
#include "mso_errs.h"

#define FILE_LOGNAME "fm_mso_lifecycle_device.c"


/*******************************************************************************
 * Functions Defined outside this source file
 ******************************************************************************/
extern int32
fm_mso_trans_open(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	int32		flag,
	pin_errbuf_t	*ebufp);

extern void 
fm_mso_trans_commit(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	pin_errbuf_t	*ebufp);

extern void 
fm_mso_trans_abort(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	pin_errbuf_t	*ebufp);


/*******************************************************************************
 * Functions contained within this source file
 ******************************************************************************/
EXPORT_OP void
op_mso_lifecycle_device(
    cm_nap_connection_t    *connp,
    int32            opcode,
    int32            flags,
    pin_flist_t        *i_flistp,
    pin_flist_t        **o_flistpp,
    pin_errbuf_t        *ebufp);

static void
fm_mso_lifecycle_device(
    pcm_context_t        *ctxp,
    pin_flist_t        *i_flistp,
    int              *error_flag,
    pin_flist_t        **ret_flistpp,
    pin_errbuf_t       *ebufp);

static void
set_error_descr(
    pin_flist_t             **ret_flistpp,
    int                     error_code,
    pin_errbuf_t            *ebufp);

static void
fm_mso_lifecycle_device_create(
	pcm_context_t		*ctxp, 
	pin_flist_t		*i_flistp, 
	pin_flist_t		**cobj_iflistp, 
	int			*error_codep, 
	pin_errbuf_t		*ebufp);

static void
fm_mso_lifecycle_device_state(
	pcm_context_t		*ctxp, 
	pin_flist_t		*i_flistp, 
	pin_flist_t		**cobj_iflistp, 
	int			*error_codep, 
	pin_errbuf_t		*ebufp);

static void
fm_mso_lifecycle_device_associate(
	pcm_context_t		*ctxp, 
	pin_flist_t		*i_flistp, 
	pin_flist_t		**cobj_iflistp, 
	pin_flist_t		**device_read_obj,
	int			*error_codep, 
	pin_errbuf_t		*ebufp);

static void
fm_mso_lifecycle_device_disassociate(
	pcm_context_t		*ctxp, 
	pin_flist_t		*i_flistp, 
	pin_flist_t		**cobj_iflistp, 
	pin_flist_t		**device_read_obj,
	int			*error_codep, 
	pin_errbuf_t		*ebufp);

static void
fm_mso_lifecycle_device_set_att(
	pcm_context_t		*ctxp, 
	pin_flist_t		*i_flistp, 
	pin_flist_t		**cobj_iflistp, 
	int			*error_codep, 
	pin_errbuf_t		*ebufp);

static void
fm_mso_lifecycle_device_movement(
	pcm_context_t		*ctxp, 
	pin_flist_t		*i_flistp, 
	pin_flist_t		**cobj_iflistp, 
	int			*error_codep, 
	pin_errbuf_t		*ebufp);

static void
fm_mso_lifecycle_device_removal(
	pcm_context_t		*ctxp, 
	pin_flist_t		*i_flistp, 
	pin_flist_t		**cobj_iflistp, 
	int			*error_codep, 
	pin_errbuf_t		*ebufp);

static void
fm_mso_get_device_detail(
	pcm_context_t		*ctxp, 
	poid_t			*dev_pdp, 
	pin_flist_t		**dev_flistp, 
	pin_errbuf_t		*ebufp);

static void
fm_mso_get_agg_no(
	pcm_context_t		*ctxp, 
	poid_t			*srv_pdp, 
	char			**agg_no, 
	pin_errbuf_t		*ebufp);

static void
fm_mso_get_accno(
	pcm_context_t		*ctxp, 
	poid_t			*srv_pdp, 
	char			**acc_no, 
	pin_errbuf_t		*ebufp);

/*******************************************************************************

 ******************************************************************************/
void
op_mso_lifecycle_device(
	cm_nap_connection_t		*connp,
	int32				opcode,
	int32				flags,
	pin_flist_t			*i_flistp,
	pin_flist_t			**o_flistpp,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*r_flistp = NULL;
	pcm_context_t			*ctxp = connp->dm_ctx;
	int32				t_status;
	poid_t				*pdp = NULL;
	int				error_code = 0;


	/***********************************************************
	 * Null	out	results	until we have some.
	 ***********************************************************/
	*o_flistpp = NULL;
	PIN_ERRBUF_CLEAR(ebufp);

	/***********************************************************
	 * Insanity	check.
	 ***********************************************************/
	if (opcode != MSO_OP_LIFECYCLE_DEVICE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_mso_lifecycle_device", ebufp);
		return;
	}

	/***********************************************************
	 * Log input flist
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_mso_lifecycle_device input flist", i_flistp);

	/***********************************************************
	 * Call main function to do it
	 ***********************************************************/
	pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	t_status = fm_mso_trans_open(ctxp, pdp, 1, ebufp);

	/***********************************************************
	* Call main function to do it
	***********************************************************/
	fm_mso_lifecycle_device(ctxp, i_flistp, &error_code, &r_flistp, ebufp);

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		*o_flistpp = (pin_flist_t *)NULL;
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_lifecycle_device error", ebufp);
		//PIN_ERRBUF_RESET(ebufp);
		if (t_status == 0)
		{
			fm_mso_trans_abort(ctxp, pdp, ebufp);
		}
		return;
//		*o_flistpp = PIN_FLIST_CREATE(ebufp);
//		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *o_flistpp, PIN_FLD_POID, ebufp);
//		PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
//		if (error_code)
//		{
//		    set_error_descr(o_flistpp, error_code, ebufp);
//		}
//		else
//		{
//			*o_flistpp = (pin_flist_t *)NULL;
//		}
	} 
	else 
	{
		//PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &oap_success, ebufp);
		*o_flistpp = r_flistp;

		PIN_ERRBUF_CLEAR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_mso_lifecycle_device return flist", *o_flistpp);
		if (t_status == 0)
		{
			fm_mso_trans_commit(ctxp, pdp, ebufp);
		}
	}
	return;

}


/*******************************************************************************
 * fm_mso_lifecycle_device()
 *
 * Inputs: flist with device poid and services array, each element including
 *         a service poid and an account poid; flist with device attributes
 * Outputs: void; ebuf set if errors encountered
 *
 * Description:
 * This function validates the input services against the given device, and
 * ensures the state of the device is Assigned if successful.
 ******************************************************************************/
static void
fm_mso_lifecycle_device(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	int			*error_codep,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*cobj_iflistp = NULL;
	pin_flist_t		*cobj_oflistp = NULL;
	pin_flist_t		*device_read_out = NULL;

	int32			*flag = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_lifecycle_device: Input Flist", i_flistp);
	flag = (int32 *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_lifecycle_device: Flags missing in input", ebufp);
		*error_codep = MSO_DEV_FLAG_ERR;
		return;
	}
	switch(*flag)
	{
		case 1: 
			//For Device Create
			fm_mso_lifecycle_device_create(ctxp, i_flistp, &cobj_iflistp, error_codep, ebufp);
			break;
		case 2:
			//For Device Set State
			fm_mso_lifecycle_device_state(ctxp, i_flistp, &cobj_iflistp, error_codep, ebufp);
			break;
		case 3:
			//For Device Associate
			fm_mso_lifecycle_device_associate(ctxp, i_flistp, &cobj_iflistp, &device_read_out, error_codep, ebufp);
			break;
		case 4:
			//For Device Disassociate
			fm_mso_lifecycle_device_disassociate(ctxp, i_flistp, &cobj_iflistp, &device_read_out, error_codep, ebufp);
			break;
		case 5:
			//For Device Set Attribute
			fm_mso_lifecycle_device_set_att(ctxp, i_flistp, &cobj_iflistp, error_codep, ebufp);
			break;
		case 6:
			//For Device Movement
			fm_mso_lifecycle_device_movement(ctxp, i_flistp, &cobj_iflistp, error_codep, ebufp);
			break;
		case 7:
			//For Device Removal
			fm_mso_lifecycle_device_removal(ctxp, i_flistp, &cobj_iflistp, error_codep, ebufp);
			break;
		default:
			*error_codep = MSO_DEV_FLAG_ERR;
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
			goto cleanup;
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_lifecycle_device: Error", ebufp);
		goto cleanup;
	}
	//Create the Lifecycle Object
	if(cobj_iflistp)
	{
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, cobj_iflistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACTION, cobj_iflistp, PIN_FLD_ACTION, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, cobj_iflistp, PIN_FLD_PROGRAM_NAME, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, cobj_iflistp, PIN_FLD_SERVICE_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, cobj_iflistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
			"fm_mso_lifecycle_device: Create Obj Input flist", cobj_iflistp);
		//Call the PCM_OP_CREATE_OBJ opcode
		PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, cobj_iflistp, &cobj_oflistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_lifecycle_device: Error", ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
			"fm_mso_lifecycle_device: Create Obj Output Flist", cobj_oflistp);
	}
	else
	{
		*error_codep = MSO_DEV_INTERNAL_ERR;
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
		goto cleanup;
	}
	
	if (device_read_out)
	{
		*ret_flistpp = PIN_FLIST_COPY(device_read_out, ebufp);
		PIN_FLIST_DESTROY_EX(&device_read_out, ebufp);
	}
	else
	{
		*ret_flistpp = PIN_FLIST_COPY(cobj_oflistp, ebufp);
	}
cleanup:
	PIN_FLIST_DESTROY_EX(&cobj_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&cobj_iflistp, ebufp);
	return;
}

static void
set_error_descr(
	pin_flist_t             **ret_flistpp,
	int                     error_code,
	pin_errbuf_t            *ebufp)
{
	char            code[100] = {'\0'};
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"set_error_descr input flist", *ret_flistpp);
	sprintf(code,"%d",error_code);
	PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, code, ebufp);

	if (error_code == MSO_DEV_FLAG_ERR)
	{
	    strcpy(code,MSO_DEV_FLAG_ERR_DESCR);
	}
	else if (error_code == MSO_DEV_INTERNAL_ERR)
	{
	    strcpy(code,MSO_DEV_INTERNAL_ERR_DESCR);
	}
	else if (error_code == MSO_DEV_MANDATORY_ERR)
	{
	    strcpy(code,MSO_DEV_MANDATORY_ERR_DESCR);
	}
	PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, (void *)code, ebufp);
	return;
}

static void
fm_mso_lifecycle_device_create(
	pcm_context_t		*ctxp, 
	pin_flist_t		*i_flistp, 
	pin_flist_t		**cobj_iflistp, 
	int			*error_codep, 
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*temp_flistp = NULL;
	poid_t			*dev_pdp = NULL;
	char			*descr = NULL;
	char			*dev_id = NULL;
	char			*dev_type = NULL;
        time_t                  *shipment_date= NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_lifecycle_device_create: Input Flist", i_flistp);
	dev_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEVICE_OBJ, 0, ebufp);
	dev_id = (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEVICE_ID, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		*error_codep = MSO_DEV_MANDATORY_ERR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_lifecycle_device_create: Error", ebufp);
		return;
	}
	dev_type = (char *)PIN_POID_GET_TYPE(dev_pdp);
	//Create the Description
	descr = (char *)malloc((strlen("DEVICE TYPE  with DEVICE  CREATED") + strlen(dev_type) + strlen(dev_id)) + 1);
	sprintf(descr, "DEVICE TYPE %s with DEVICE %s CREATED", dev_type, dev_id);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Description");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, descr);

	//Prepare the flist
	*cobj_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*cobj_iflistp, PIN_FLD_DESCR, (void *)descr, ebufp);
	temp_flistp = PIN_FLIST_ELEM_ADD(*cobj_iflistp, PIN_FLD_DEVICES, 0, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_STATE_ID, temp_flistp, PIN_FLD_NEW_STATE, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DEVICE_ID, temp_flistp, PIN_FLD_DEVICE_ID, ebufp);
        /*Inventory Change*/
        shipment_date = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_CR_ADJ_DATE, 0, ebufp);
        if(shipment_date != (time_t *)NULL)
        {
        PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE,temp_flistp,PIN_FLD_START_T, ebufp);
        }
        else
        {
        PIN_ERRBUF_CLEAR(ebufp);
        }
       /*Changes End Here*/
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		*error_codep = MSO_DEV_MANDATORY_ERR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_lifecycle_device_create: Error", ebufp);
	}
	return;
}

static void
fm_mso_lifecycle_device_state(
	pcm_context_t		*ctxp, 
	pin_flist_t		*i_flistp, 
	pin_flist_t		**cobj_iflistp, 
	int			*error_codep, 
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*temp_flistp = NULL;
	pin_flist_t		*robj_iflistp = NULL;
	pin_flist_t		*dev_flistp = NULL;
	poid_t			*dev_pdp = NULL;
	char			*descr = NULL;
	char			*dev_id = NULL;
	char			*dev_type = NULL;
    char            *flow_type = NULL;
	int			*new_state = 0;
	int			*old_state = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_lifecycle_device_state: Input Flist", i_flistp);
	dev_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEVICE_OBJ, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		*error_codep = MSO_DEV_MANDATORY_ERR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_lifecycle_device_state: Error", ebufp);
		return;
	}
	dev_type = (char *)PIN_POID_GET_TYPE(dev_pdp);
	fm_mso_get_device_detail(ctxp, dev_pdp, &dev_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		*error_codep = MSO_DEV_INTERNAL_ERR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_lifecycle_device_state: Error", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_lifecycle_device_state: Device Flist", dev_flistp);
	dev_id = (char *)PIN_FLIST_FLD_GET(dev_flistp, PIN_FLD_DEVICE_ID, 0, ebufp);
	old_state = (int *)PIN_FLIST_FLD_GET(dev_flistp, PIN_FLD_STATE_ID, 0, ebufp);
    flow_type = (char *)PIN_FLIST_FLD_GET(dev_flistp, MSO_FLD_ORDER_TYPE, 1, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		*error_codep = MSO_DEV_INTERNAL_ERR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_lifecycle_device_state: Error", ebufp);
		goto cleanup;
	}
	new_state = (int *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_STATE_ID, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		*error_codep = MSO_DEV_MANDATORY_ERR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_lifecycle_device_state: Error", ebufp);
		return;
	}
	//Create the Description
	descr = (char *)malloc(strlen("DEVICE TYPE  with DEVICE  STATE CHANGED FROM  TO ") 
		+ strlen(dev_type) + strlen(dev_id) + sizeof(old_state) + sizeof(new_state)+ 1);
	sprintf(descr, "DEVICE TYPE %s with DEVICE %s STATE CHANGED FROM %d TO %d", 
		dev_type, dev_id, *old_state, *new_state);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Description");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, descr);

	//Prepare the flist
	*cobj_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*cobj_iflistp, PIN_FLD_DESCR, (void *)descr, ebufp);
	temp_flistp = PIN_FLIST_ELEM_ADD(*cobj_iflistp, PIN_FLD_DEVICES, 0, ebufp);
    if (flow_type)
    {
        PIN_FLIST_FLD_SET(temp_flistp, MSO_FLD_ORDER_TYPE, (void *)flow_type, ebufp);
    }
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_STATE_ID, temp_flistp, PIN_FLD_NEW_STATE, ebufp);
	PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_OLD_STATE, (void *)old_state, ebufp);
	PIN_FLIST_FLD_COPY(dev_flistp, PIN_FLD_DEVICE_ID, temp_flistp, PIN_FLD_DEVICE_ID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DEVICE_OBJ, temp_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		*error_codep = MSO_DEV_MANDATORY_ERR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_lifecycle_device_state: Error", ebufp);
	}
cleanup:
	PIN_FLIST_DESTROY_EX(&dev_flistp, NULL);
	return;
}

static void
fm_mso_lifecycle_device_associate(
	pcm_context_t		*ctxp, 
	pin_flist_t		*i_flistp, 
	pin_flist_t		**cobj_iflistp, 
        pin_flist_t             **device_read_out,
	int			*error_codep, 
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*temp_flistp = NULL;
	pin_flist_t		*robj_iflistp = NULL;
	pin_flist_t		*dev_flistp = NULL;
	poid_t			*dev_pdp = NULL;
	poid_t			*srv_pdp = NULL;
	poid_t			*acc_pdp = NULL;
	char			*descr = NULL;
	char			*dev_id = NULL;
	char			*dev_type = NULL;
	char			*acc_no = NULL;
	char			*agg_no = NULL;
    char            *flow_type = NULL;
	int			*old_state = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_lifecycle_device_associate: Input Flist", i_flistp);
	dev_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEVICE_OBJ, 0, ebufp);
	srv_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
	acc_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		*error_codep = MSO_DEV_MANDATORY_ERR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_lifecycle_device_associate: Error", ebufp);
		return;
	}
	fm_mso_get_accno(ctxp, acc_pdp, &acc_no, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		*error_codep = MSO_DEV_INTERNAL_ERR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_lifecycle_device_associate: Error", ebufp);
		goto cleanup;
	}
	fm_mso_get_agg_no(ctxp, srv_pdp, &agg_no, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		*error_codep = MSO_DEV_INTERNAL_ERR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_lifecycle_device_associate: Error", ebufp);
		goto cleanup;
	}
	dev_type = (char *)PIN_POID_GET_TYPE(dev_pdp);

	fm_mso_get_device_detail(ctxp, dev_pdp, &dev_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		*error_codep = MSO_DEV_INTERNAL_ERR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_lifecycle_device_associate: Error", ebufp);
		goto cleanup;
	}
        if (dev_flistp)
        {
                *device_read_out = PIN_FLIST_COPY(dev_flistp, ebufp);

        }


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_lifecycle_device_associate: Device Flist", dev_flistp);
	dev_id = (char *)PIN_FLIST_FLD_GET(dev_flistp, PIN_FLD_DEVICE_ID, 1, ebufp);
	old_state = (int *)PIN_FLIST_FLD_GET(dev_flistp, PIN_FLD_STATE_ID, 0, ebufp);
    flow_type = (char *)PIN_FLIST_FLD_GET(dev_flistp, MSO_FLD_ORDER_TYPE, 1, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		*error_codep = MSO_DEV_INTERNAL_ERR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_lifecycle_device_associate: Error", ebufp);
		goto cleanup;
	}
	//Create the Description
	if(agg_no && strcmp(agg_no,"") != 0)
	{
		descr = (char *)malloc(strlen("DEVICE TYPE  with DEVICE  ASSOCIATED WITH  ACCOUNT_NO :  AND AGREEMENT_NO ") 
			+ strlen(dev_type) + strlen(dev_id) + strlen(acc_no) + strlen(agg_no)+ 1);
		sprintf(descr, "DEVICE TYPE %s with DEVICE %s ASSOCIATED WITH ACCOUNT_NO : %s AND AGREEMENT_NO %s", 
			dev_type, dev_id, acc_no, agg_no);
	}
	else
	{
		descr = (char *)malloc(strlen("DEVICE TYPE  with DEVICE  ASSOCIATED WITH  ACCOUNT_NO :  ") 
			+ strlen(dev_type) + strlen(dev_id) + strlen(acc_no) + 1);
		sprintf(descr, "DEVICE TYPE %s with DEVICE %s ASSOCIATED WITH ACCOUNT_NO : %s ", 
			dev_type, dev_id, acc_no);
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Description");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, descr);

	//Prepare the flist
	*cobj_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*cobj_iflistp, PIN_FLD_DESCR, (void *)descr, ebufp);
	temp_flistp = PIN_FLIST_ELEM_ADD(*cobj_iflistp, PIN_FLD_DEVICES, 0, ebufp);
    if (flow_type)
    {
        PIN_FLIST_FLD_SET(temp_flistp, MSO_FLD_ORDER_TYPE, (void *)flow_type, ebufp);
    }
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_STATE_ID, temp_flistp, PIN_FLD_NEW_STATE, ebufp);
	PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_OLD_STATE, (void *)old_state, ebufp);
	PIN_FLIST_FLD_COPY(dev_flistp, PIN_FLD_DEVICE_ID, temp_flistp, PIN_FLD_DEVICE_ID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DEVICE_OBJ, temp_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		*error_codep = MSO_DEV_MANDATORY_ERR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_lifecycle_device_associate: Error", ebufp);
	}
cleanup:
	PIN_FLIST_DESTROY_EX(&dev_flistp, NULL);
	if(acc_no)
		free(acc_no);
	if(agg_no)
		free(agg_no);
	return;
}

static void
fm_mso_lifecycle_device_disassociate(
	pcm_context_t		*ctxp, 
	pin_flist_t		*i_flistp, 
	pin_flist_t		**cobj_iflistp, 
	pin_flist_t		**device_read_out,
	int			*error_codep, 
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*temp_flistp = NULL;
	pin_flist_t		*robj_iflistp = NULL;
	pin_flist_t		*dev_flistp = NULL;
	poid_t			*dev_pdp = NULL;
	poid_t			*srv_pdp = NULL;
	poid_t			*acc_pdp = NULL;
	char			*descr = NULL;
	char			*dev_id = NULL;
	char			*dev_type = NULL;
	char			*acc_no = NULL;
	char			*agg_no = NULL;
    char            *flow_type = NULL;
	int			*old_state = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_lifecycle_device_disassociate: Input Flist", i_flistp);
	dev_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEVICE_OBJ, 0, ebufp);
	srv_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	acc_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		*error_codep = MSO_DEV_MANDATORY_ERR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_lifecycle_device_disassociate: Error", ebufp);
		return;
	}
	fm_mso_get_accno(ctxp, acc_pdp, &acc_no, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		*error_codep = MSO_DEV_INTERNAL_ERR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_lifecycle_device_disassociate: Error", ebufp);
		goto cleanup;
	}
	fm_mso_get_agg_no(ctxp, srv_pdp, &agg_no, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		*error_codep = MSO_DEV_INTERNAL_ERR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_lifecycle_device_disassociate: Error", ebufp);
		goto cleanup;
	}
	dev_type = (char *)PIN_POID_GET_TYPE(dev_pdp);

	fm_mso_get_device_detail(ctxp, dev_pdp, &dev_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		*error_codep = MSO_DEV_INTERNAL_ERR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_lifecycle_device_disassociate: Error", ebufp);
		goto cleanup;
	}

	if (dev_flistp)
	{
		*device_read_out = PIN_FLIST_COPY(dev_flistp, ebufp);
	 
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_lifecycle_device_disassociate: Device Flist", dev_flistp);
	dev_id = (char *)PIN_FLIST_FLD_GET(dev_flistp, PIN_FLD_DEVICE_ID, 0, ebufp);
	old_state = (int *)PIN_FLIST_FLD_GET(dev_flistp, PIN_FLD_STATE_ID, 0, ebufp);
    flow_type = (char *)PIN_FLIST_FLD_GET(dev_flistp, MSO_FLD_ORDER_TYPE, 1, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		*error_codep = MSO_DEV_INTERNAL_ERR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_lifecycle_device_disassociate: Error", ebufp);
		goto cleanup;
	}
	//Create the Description
	if(agg_no && strcmp(agg_no,"") != 0)
	{
		descr = (char *)malloc(strlen("DEVICE TYPE  with DEVICE  DISASSOCIATED FROM  ACCOUNT_NO :  AND AGREEMENT_NO ") 
			+ strlen(dev_type) + strlen(dev_id) + strlen(acc_no) + strlen(agg_no)+ 1);
		sprintf(descr, "DEVICE TYPE %s with DEVICE %s DISASSOCIATED FROM ACCOUNT_NO : %s AND AGREEMENT_NO %s", 
			dev_type, dev_id, acc_no, agg_no);
	}
	else
	{
		descr = (char *)malloc(strlen("DEVICE TYPE  with DEVICE  DISASSOCIATED FROM  ACCOUNT_NO :  ") 
			+ strlen(dev_type) + strlen(dev_id) + strlen(acc_no) + 1);
		sprintf(descr, "DEVICE TYPE %s with DEVICE %s DISASSOCIATED FROM ACCOUNT_NO : %s", 
			dev_type, dev_id, acc_no);
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Description");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, descr);

	//Prepare the flist
	*cobj_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*cobj_iflistp, PIN_FLD_DESCR, (void *)descr, ebufp);
	temp_flistp = PIN_FLIST_ELEM_ADD(*cobj_iflistp, PIN_FLD_DEVICES, 0, ebufp);
    if (flow_type)
    {
        PIN_FLIST_FLD_SET(temp_flistp, MSO_FLD_ORDER_TYPE, (void *)flow_type, ebufp);
    }
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_STATE_ID, temp_flistp, PIN_FLD_NEW_STATE, ebufp);
	PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_OLD_STATE, (void *)old_state, ebufp);
	PIN_FLIST_FLD_COPY(dev_flistp, PIN_FLD_DEVICE_ID, temp_flistp, PIN_FLD_DEVICE_ID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DEVICE_OBJ, temp_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		*error_codep = MSO_DEV_MANDATORY_ERR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_lifecycle_device_disassociate: Error", ebufp);
	}
cleanup:
	PIN_FLIST_DESTROY_EX(&dev_flistp, NULL);
	if(acc_no)
		free(acc_no);
	if(agg_no)
		free(agg_no);
	return;
}

static void
fm_mso_lifecycle_device_set_att(
	pcm_context_t		*ctxp, 
	pin_flist_t		*i_flistp, 
	pin_flist_t		**cobj_iflistp, 
	int			*error_codep, 
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*temp_flistp = NULL;
	pin_flist_t		*robj_iflistp = NULL;
	pin_flist_t		*dev_flistp = NULL;
	poid_t			*dev_pdp = NULL;
	poid_t			*srv_pdp = NULL;
	poid_t			*acc_pdp = NULL;
	char			*descr = NULL;
	char			*dev_id = NULL;
	char			*dev_type = NULL;
	char			*acc_no = NULL;
	char			*agg_no = NULL;
	int			*old_state = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_lifecycle_device_set_att: Input Flist", i_flistp);
	dev_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEVICE_OBJ, 0, ebufp);
	srv_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		*error_codep = MSO_DEV_MANDATORY_ERR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_lifecycle_device_set_att: Error", ebufp);
		return;
	}
	dev_type = (char *)PIN_POID_GET_TYPE(dev_pdp);
	//Create the Description
        fm_mso_get_device_detail(ctxp, dev_pdp, &dev_flistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp)){
                *error_codep = MSO_DEV_INTERNAL_ERR;
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_mso_lifecycle_device_movement: Error", ebufp);
                goto cleanup;
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_mso_lifecycle_device_movement: Device Flist", dev_flistp);
        dev_id = (char *)PIN_FLIST_FLD_GET(dev_flistp, PIN_FLD_DEVICE_ID, 0, ebufp);

	descr = (char *)malloc(strlen("DEVICE TYPE  with DEVICE  ATTRIBUTES MODIFIED") 
		+ strlen(dev_type) + strlen(dev_id)+ 1);
	sprintf(descr, "DEVICE TYPE %s with DEVICE %s ATTRIBUTES MODIFIED", 
		dev_type, dev_id);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Description");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, descr);
	dev_type = (char *)PIN_POID_GET_TYPE(dev_pdp);
	//Prepare the flist
	*cobj_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*cobj_iflistp, PIN_FLD_DESCR, (void *)descr, ebufp);
	temp_flistp = PIN_FLIST_ELEM_ADD(*cobj_iflistp, PIN_FLD_DEVICES, 0, ebufp);
	PIN_FLIST_FLD_COPY(dev_flistp, PIN_FLD_DEVICE_ID, temp_flistp, PIN_FLD_DEVICE_ID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DEVICE_OBJ, temp_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
	
	//Inventory changes - Added new fields
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CHANNEL_NAME, temp_flistp, MSO_FLD_CHANNEL_NAME, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_VALID_TO, temp_flistp, PIN_FLD_VALID_TO, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DELIVERY_STATUS, temp_flistp, PIN_FLD_DELIVERY_STATUS, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_VENDOR_WARRANTY_END, temp_flistp, MSO_FLD_VENDOR_WARRANTY_END,  ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_RECEIPT_NO, temp_flistp, PIN_FLD_RECEIPT_NO,  ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ORDER_ID, temp_flistp, PIN_FLD_ORDER_ID,  ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_ORDER_TYPE, temp_flistp, MSO_FLD_ORDER_TYPE,  ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp)){
		*error_codep = MSO_DEV_MANDATORY_ERR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_lifecycle_device_set_att: Error", ebufp);
	}
cleanup:
	PIN_FLIST_DESTROY_EX(&dev_flistp, NULL);
	return;
}

static void
fm_mso_lifecycle_device_movement(
	pcm_context_t		*ctxp, 
	pin_flist_t		*i_flistp, 
	pin_flist_t		**cobj_iflistp, 
	int			*error_codep, 
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*temp_flistp = NULL;
	pin_flist_t		*robj_iflistp = NULL;
	pin_flist_t		*dev_flistp = NULL;
	poid_t			*dev_pdp = NULL;
	poid_t			*old_pdp = NULL;
	poid_t			*acc_pdp = NULL;
	char			*descr = NULL;
	char			*dev_id = NULL;
	char			*dev_type = NULL;
	char			*acc_no = NULL;
	char			*source = NULL;
	char			*agg_no = NULL;
	int			*old_state = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_lifecycle_device_movement: Input Flist", i_flistp);
	dev_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEVICE_OBJ, 0, ebufp);
	acc_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	source = (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SOURCE, 1, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		*error_codep = MSO_DEV_MANDATORY_ERR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_lifecycle_device_movement: Error", ebufp);
		return;
	}

	dev_type = (char *)PIN_POID_GET_TYPE(dev_pdp);

	fm_mso_get_device_detail(ctxp, dev_pdp, &dev_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		*error_codep = MSO_DEV_INTERNAL_ERR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_lifecycle_device_movement: Error", ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_lifecycle_device_movement: Device Flist", dev_flistp);
	dev_id = (char *)PIN_FLIST_FLD_GET(dev_flistp, PIN_FLD_DEVICE_ID, 0, ebufp);
	acc_no = (char *)PIN_FLIST_FLD_GET(dev_flistp, PIN_FLD_SOURCE, 0, ebufp);
	old_pdp = (poid_t *)PIN_FLIST_FLD_GET(dev_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		*error_codep = MSO_DEV_INTERNAL_ERR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_lifecycle_device_movement: Error", ebufp);
		goto cleanup;
	}
	//Create the Description
	descr = (char *)malloc(strlen("DEVICE TYPE  with DEVICE  MOVED FROM ENTITY NO  TO ") 
		+ strlen(dev_type) + strlen(dev_id) + strlen(acc_no) + strlen(source)+ 1);
	sprintf(descr, "DEVICE TYPE %s with DEVICE %s MOVED FROM ENTITY NO %s TO %s", 
		dev_type, dev_id, acc_no, source);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Description");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, descr);

	//Prepare the flist
	*cobj_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*cobj_iflistp, PIN_FLD_DESCR, (void *)descr, ebufp);
	temp_flistp = PIN_FLIST_ELEM_ADD(*cobj_iflistp, PIN_FLD_DEVICES, 0, ebufp);
	PIN_FLIST_FLD_COPY(dev_flistp, PIN_FLD_DEVICE_ID, temp_flistp, PIN_FLD_DEVICE_ID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DEVICE_OBJ, temp_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
	PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_OWNER, (void *)old_pdp, ebufp);
	PIN_FLIST_FLD_SET(temp_flistp, MSO_FLD_NEW_OWNER, (void *)acc_pdp, ebufp);
	/**old account_no **/
	PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_ACCOUNT_NO, (void *)acc_no, ebufp);
	/**new account_no **/
	PIN_FLIST_FLD_SET(temp_flistp, MSO_FLD_ACCOUNT_OWNER, (void *)source, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_RECEIPT_NO, temp_flistp, PIN_FLD_RECEIPT_NO, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ORDER_ID, temp_flistp, PIN_FLD_ORDER_ID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_REFERENCE_ID, temp_flistp, PIN_FLD_REFERENCE_ID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_TRANSACTION_ID, temp_flistp, MSO_FLD_TRANSACTION_ID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_STATE, temp_flistp, PIN_FLD_STATE, ebufp);
	//Inventory changes //
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CHANNEL_NAME, temp_flistp, MSO_FLD_CHANNEL_NAME, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_VALID_TO, temp_flistp, PIN_FLD_VALID_TO, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DELIVERY_STATUS, temp_flistp, PIN_FLD_DELIVERY_STATUS, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_VENDOR_WARRANTY_END, temp_flistp, MSO_FLD_VENDOR_WARRANTY_END,  ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_ORDER_TYPE, temp_flistp, MSO_FLD_ORDER_TYPE,  ebufp);
		
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		*error_codep = MSO_DEV_MANDATORY_ERR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_lifecycle_device_movement: Error", ebufp);
	}
cleanup:
	PIN_FLIST_DESTROY_EX(&dev_flistp, NULL);
	return;
}

static void
fm_mso_lifecycle_device_removal(
	pcm_context_t		*ctxp, 
	pin_flist_t		*i_flistp, 
	pin_flist_t		**cobj_iflistp, 
	int			*error_codep, 
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*temp_flistp = NULL;
	pin_flist_t		*robj_iflistp = NULL;
	pin_flist_t		*dev_flistp = NULL;
	poid_t			*dev_pdp = NULL;
	poid_t			*acc_pdp = NULL;
	char			*descr = NULL;
	char			*dev_id = NULL;
	char			*dev_type = NULL;
	char			*acc_no = NULL;
	char			*agg_no = NULL;
    char            *flow_type = NULL;
	int			*old_state = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_lifecycle_device_removal: Input Flist", i_flistp);
	dev_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEVICE_OBJ, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		*error_codep = MSO_DEV_MANDATORY_ERR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_lifecycle_device_removal: Error", ebufp);
		return;
	}
	dev_type = (char *)PIN_POID_GET_TYPE(dev_pdp);

	fm_mso_get_device_detail(ctxp, dev_pdp, &dev_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		*error_codep = MSO_DEV_INTERNAL_ERR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_lifecycle_device_removal: Error", ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_lifecycle_device_removal: Device Flist", dev_flistp);
	dev_id = (char *)PIN_FLIST_FLD_GET(dev_flistp, PIN_FLD_DEVICE_ID, 0, ebufp);
    flow_type = (char *)PIN_FLIST_FLD_GET(dev_flistp, MSO_FLD_ORDER_TYPE, 1, ebufp);

	//Create the Description
	descr = (char *)malloc(strlen("DEVICE TYPE  with DEVICE  REMOVED") 
		+ strlen(dev_type) + strlen(dev_id)+ 1);
	sprintf(descr, "DEVICE TYPE %s with DEVICE %s REMOVED", 
		dev_type, dev_id);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Description");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, descr);

	//Prepare the flist
	*cobj_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*cobj_iflistp, PIN_FLD_DESCR, (void *)descr, ebufp);
	temp_flistp = PIN_FLIST_ELEM_ADD(*cobj_iflistp, PIN_FLD_DEVICES, 0, ebufp);
    if (flow_type)
    {
        PIN_FLIST_FLD_SET(temp_flistp, MSO_FLD_ORDER_TYPE, (void *)flow_type, ebufp);
    }
	PIN_FLIST_FLD_COPY(dev_flistp, PIN_FLD_DEVICE_ID, temp_flistp, PIN_FLD_DEVICE_ID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DEVICE_OBJ, temp_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		*error_codep = MSO_DEV_MANDATORY_ERR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_lifecycle_device_removal: Error", ebufp);
	}
	cleanup:
		PIN_FLIST_DESTROY_EX(&dev_flistp, NULL);
	return;
}

static void
fm_mso_get_device_detail(
	pcm_context_t		*ctxp, 
	poid_t			*dev_pdp, 
	pin_flist_t		**dev_flistp, 
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*robj_iflistp = NULL;
	pin_flist_t		*robj_oflistp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);
	robj_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(robj_iflistp, PIN_FLD_POID, (void *)dev_pdp, ebufp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, robj_iflistp, &robj_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_get_device_detail: Error", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_get_device_detail: Output Flist", robj_oflistp);
	*dev_flistp = PIN_FLIST_COPY(robj_oflistp, ebufp);
cleanup:
	PIN_FLIST_DESTROY_EX(&robj_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&robj_oflistp, NULL);
	return;
}

static void
fm_mso_get_agg_no(
	pcm_context_t		*ctxp, 
	poid_t			*srv_pdp, 
	char			**agg_no, 
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*robj_iflistp = NULL;
	pin_flist_t		*robj_oflistp = NULL;
	pin_flist_t		*bb_info = NULL;
	char			*serv_type = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);

	serv_type = (char *)PIN_POID_GET_TYPE(srv_pdp);
	
	robj_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(robj_iflistp, PIN_FLD_POID, (void *)srv_pdp, ebufp);
	if(serv_type && strcmp(serv_type, "/service/catv") == 0)
	{
		bb_info = PIN_FLIST_SUBSTR_ADD(robj_iflistp, MSO_FLD_CATV_INFO, ebufp);
	}
	else
	{
		bb_info = PIN_FLIST_SUBSTR_ADD(robj_iflistp, MSO_FLD_BB_INFO, ebufp);
	}
	PIN_FLIST_FLD_SET(bb_info, MSO_FLD_AGREEMENT_NO, (void *)NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_get_agg_no: Input Flist", robj_iflistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, robj_iflistp, &robj_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_get_agg_no: Error", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_get_agg_no: Output Flist", robj_oflistp);
	if(serv_type && strcmp(serv_type, "/service/catv") == 0)
	{	bb_info = PIN_FLIST_SUBSTR_GET(robj_oflistp, MSO_FLD_CATV_INFO, 0, ebufp);}
	else
	{	bb_info = PIN_FLIST_SUBSTR_GET(robj_oflistp, MSO_FLD_BB_INFO, 0, ebufp);}

	*agg_no = PIN_FLIST_FLD_TAKE(bb_info, MSO_FLD_AGREEMENT_NO, 0, ebufp);
cleanup:
	PIN_FLIST_DESTROY_EX(&robj_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&robj_oflistp, NULL);
	return;
}

static void
fm_mso_get_accno(
	pcm_context_t		*ctxp, 
	poid_t			*acc_pdp, 
	char			**acc_no, 
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*robj_iflistp = NULL;
	pin_flist_t		*robj_oflistp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);
	robj_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(robj_iflistp, PIN_FLD_POID, (void *)acc_pdp, ebufp);
	PIN_FLIST_FLD_SET(robj_iflistp, PIN_FLD_ACCOUNT_NO, (void *)NULL, ebufp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, robj_iflistp, &robj_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_get_accno: Error", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_get_accno: Output Flist", robj_oflistp);
	*acc_no = PIN_FLIST_FLD_TAKE(robj_oflistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
cleanup:
	PIN_FLIST_DESTROY_EX(&robj_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&robj_oflistp, NULL);
	return;
}
