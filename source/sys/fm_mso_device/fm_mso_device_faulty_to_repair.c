/******************************************************************************
 *
 * Copyright (c) 2019 Hathway. All rights reserved.
 *
 * This material is the confidential property of Hathway 
 * or its licensors and may be used, reproduced, stored or transmitted
 * only in accordance with a valid Hathway license or sublicense agreement.
 *
 *****************************************************************************/
/*******************************************************************************
 * Maintentance Log:
 *
 * Date: 20191211
 * Author: Tata Consultancy Services 
 * Identifier: Faulty Device Management
 * Notes: Moving faulty device/devices to repairing 
 ***********************************************************************/
/*******************************************************************************
 * File History
 *
 * DATE     |  CHANGE                                           |  Author
 ***********|***************************************************|****************
 |01/12/2019|Initial vesion					| Rama Puppala
 |**********|***************************************************|****************
 ********************************************************************************/

#ifndef lint
static const char Sccs_id[] = "(#)$Id: fm_mso_device_faulty_to_repair.c 2019/12/11 20:37:48 rama Exp $";
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
#include "mso_lifecycle_id.h"


#define FILE_LOGNAME "fm_mso_device_faulty_to_repair.c"


/*******************************************************************************
 * Functions Defined outside this source file
 ******************************************************************************/


/*******************************************************************************
 * Functions contained within this source file
 ******************************************************************************/
EXPORT_OP void
op_mso_device_faulty_to_repair(
    cm_nap_connection_t    *connp,
    int32            opcode,
    int32            flags,
    pin_flist_t        *i_flistp,
    pin_flist_t        **o_flistpp,
    pin_errbuf_t        *ebufp);

static void
fm_mso_device_faulty_to_repair(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	int32		flags,
	pin_flist_t	**ret_flistpp,
	pin_errbuf_t	*ebufp);

/*******************************************************************************

 ******************************************************************************/
void
op_mso_device_faulty_to_repair(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
        	return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*************************************************
	 * Insanity check.
	 ************************************************/
	if (opcode != MSO_OP_DEVICE_FAULTY_TO_REPAIR) 
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);

		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_mso_device_faulty_to_repair", ebufp);
        	return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_mso_device_faulty_to_repair input flist", i_flistp);

	fm_mso_device_faulty_to_repair(ctxp, i_flistp, flags, o_flistpp, ebufp);


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_mso_device_faulty_to_repair return flist", *o_flistpp);

    return;
}


/*******************************************************************************
 * fm_mso_device_faulty_to_repair()
 *
 * Inputs: flist with devicep array, each element including
 *         a STB_ID/VC_ID and a connection type; flist with customer contacts 
 * Outputs: void; ebuf set if errors encountered
 *
 * Description:
 * This function is a wrapper to calll MSO_OP_DEVICE_SET_ATTR opcode 
 ******************************************************************************/
static void
fm_mso_device_faulty_to_repair(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	int32		flags,
	pin_flist_t	**ret_flistpp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*attr_flistp = NULL;
	pin_flist_t	*attr_oflistp = NULL;
	pin_flist_t	*tmp_flistp = NULL;
	pin_cookie_t	cookie = NULL;
	poid_t          *pdp = NULL;
	int32		elem_id = 0;
	int32		ret_status = 0;	

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	/*
	 * Validate the input flist if required fields are present
	 */
	pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

	attr_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, attr_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, attr_flistp, PIN_FLD_USERID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, attr_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_NO, attr_flistp, PIN_FLD_ACCOUNT_NO, ebufp);
		
	while((tmp_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_DEVICES,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_DEVICE_ID, attr_flistp, PIN_FLD_DEVICE_ID, ebufp);
		PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_DEVICE_TYPE, attr_flistp, PIN_FLD_DEVICE_TYPE, ebufp);

		PCM_OP(ctxp, MSO_OP_DEVICE_SET_ATTR, 0, attr_flistp, &attr_oflistp, ebufp);

		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in location update", ebufp);
			ret_status = 1;
			PIN_ERRBUF_CLEAR(ebufp);
			break;
		}

		ret_status = *(int32 *)PIN_FLIST_FLD_GET(attr_oflistp, PIN_FLD_STATUS, 0, ebufp);
		if (ret_status != 0)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "Error in location update", attr_oflistp);
			break;
		}
		PIN_FLIST_DESTROY_EX(&attr_oflistp, NULL);
	}

	if (PIN_ERRBUF_IS_ERR(ebufp) || (ret_status != 0)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_device_faulty_to_repair: failed", ebufp);
		PIN_ERR_CLEAR_ERR(ebufp);
		*ret_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, &ret_status, ebufp);
		if (ret_status != 0)
		{
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "72001", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "Failed location to repair vendor !!!", ebufp);
		}
		goto CLEANUP;
	}

	if (!ret_status)
	{
		*ret_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
       		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, &ret_status, ebufp);
       		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_DESCR, "Movement of device to repair vendor is successful", ebufp);
	}

CLEANUP:
	PIN_FLIST_DESTROY_EX(&attr_flistp, NULL);
    	PIN_FLIST_DESTROY_EX(&attr_oflistp, NULL);
	return;
}

