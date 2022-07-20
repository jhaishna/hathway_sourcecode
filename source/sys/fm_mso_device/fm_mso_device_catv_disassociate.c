/*
 * (#)$Id: fm_mso_device_catv_associate.c st_cgbubrm_gsivakum_bug-8453030/1 2009/05/05 23:37:48 gsivakum Exp $
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
* 0.1    |15/05/2014 |J R Phani kumar   | CATV Smart card & STB dis association 

*--------------------------------------------------------------------------------------------------------------*
****************************************************************************************************************/


#ifndef lint
static const char Sccs_id[] = "(#)$Id: fm_mso_device_catv_associate.c st_cgbubrm_gsivakum_bug-8453030/1 2009/05/05 23:37:48 gsivakum Exp $";
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
#include "mso_ops_flds.h"

#define FILE_LOGNAME "fm_mso_device_catv_associate.c"

/*******************************************************************************
 * Functions contained within this source file
 ******************************************************************************/
EXPORT_OP void
op_mso_device_catv_dis_associate(
    cm_nap_connection_t    *connp,
    int32            opcode,
    int32            flags,
    pin_flist_t        *i_flistp,
    pin_flist_t        **o_flistpp,
    pin_errbuf_t        *ebufp);

static void
fm_mso_device_catv_dis_associate(
    pcm_context_t        *ctxp,
    pin_flist_t        *i_flistp,
    pin_flist_t        **rfo_flistp,
    pin_errbuf_t        *ebufp);

extern void
fm_mso_search_device_object(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_errbuf_t	*ebufp);


/*******************************************************************************

 ******************************************************************************/
void
op_mso_device_catv_dis_associate(
    cm_nap_connection_t    *connp,
    int32            opcode,
    int32            flags,
    pin_flist_t        *i_flistp,
    pin_flist_t        **o_flistpp,
    pin_errbuf_t        *ebufp)
{
    pcm_context_t        *ctxp = connp->dm_ctx;
    pin_flist_t        *rfi_flistp = NULL;
    pin_flist_t        *r_flistp = NULL;
    pin_flist_t        *sub_flistp = NULL;
    poid_t            *i_poidp = NULL;
    int32            *assoc_flag = NULL;
    char             *poid_type = NULL;
    int32	sucess = 0;
    int32	fail = 1;


    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR(ebufp);

    /*
     * Insanity check.
     */
    if (opcode != MSO_OP_DEVICE_CATV_DISASSOCIATE) {

        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
            PIN_ERR_BAD_OPCODE, 0, 0, opcode);

        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "bad opcode in op_dev_pol_device_disassociate", ebufp);

        *o_flistpp = NULL;
        return;
    }


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "op_mso_device_catv_dis_associate input flist", i_flistp);

    /*
     * Validate the input flist has the correct poid type.
     */
    i_poidp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
    poid_type = (poid_t *)PIN_POID_GET_TYPE (i_poidp);

    /*
     * Validate the input flist has at least one service element
     */
    if (PIN_FLIST_ELEM_COUNT(i_flistp, PIN_FLD_DEVICES, ebufp) < 1) {

        pin_set_err(ebufp, PIN_ERRLOC_FLIST, PIN_ERRCLASS_APPLICATION,
            PIN_ERR_MISSING_ARG, PIN_FLD_SERVICES, 0, 0);

        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_mso_device_catv_disassociate error", ebufp);

        *o_flistpp = NULL;
        return;
    }

    /*
     * Read the existing services array for this device along with its
     * current state, values from the database.
     */
       
     fm_mso_device_catv_dis_associate(ctxp, i_flistp,&r_flistp, ebufp);


    if (PIN_ERRBUF_IS_ERR(ebufp)) {

        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_mso_device_catv_disassociate error", ebufp);
        *o_flistpp = NULL;
	PIN_FLIST_FLD_SET(*o_flistpp , PIN_FLD_STATUS , &fail , ebufp);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
    }
    else
    {

        /*
         * Return a copy of the input flist
         */
        *o_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);
	PIN_FLIST_FLD_SET(*o_flistpp , PIN_FLD_STATUS , &sucess , ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "op_device_diss_assoicate return flist",
            *o_flistpp);
    }

    PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

    return;
}


/*******************************************************************************
 * fm_mso_device_catv_dis_associate()
 *
 * Inputs: flist with device poid and services array, each element including
 *         a service poid and an account poid; flist with device attributes
 * Outputs: void; ebuf set if errors encountered
 *
 * Description:
 * This function is a wrapper opcode which will prepare the flist and will call the original disassociation function
 * *****************************************************************************/
static void
fm_mso_device_catv_dis_associate(
    pcm_context_t      *ctxp,
    pin_flist_t        *i_flistp,
    pin_flist_t        **o_flistp,
    pin_errbuf_t       *ebufp)
{
    pin_flist_t      *update_oflistp = NULL;
    pin_flist_t      *update_iflistp = NULL;
    pin_flist_t      *device_flist = NULL;
    pin_flist_t	     *read_iflist = NULL;
    pin_flist_t	     *read_oflist = NULL;
    pin_flist_t	     *services_flist = NULL;
    pin_flist_t	     *service_flist = NULL;

    poid_t           *device_poid = NULL;
    
    int		elem_id = 0;
    pin_cookie_t	cookie = NULL;

    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR(ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_device_catv_dis_associate input flist", i_flistp);

    while ((device_flist = PIN_FLIST_ELEM_GET_NEXT(i_flistp,PIN_FLD_DEVICES,&elem_id, 1, &cookie, ebufp))!= NULL)
    {
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Device flist is " , device_flist);

	fm_mso_search_device_object(ctxp, device_flist , ebufp);

	device_poid = PIN_FLIST_FLD_GET(device_flist , PIN_FLD_DEVICE_OBJ, 1, ebufp);

	read_iflist = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_iflist , PIN_FLD_POID , device_poid , ebufp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_iflist , &read_oflist , ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " read flds output is " , read_oflist);

	if(PIN_FLIST_ELEM_COUNT(read_oflist , PIN_FLD_SERVICES , ebufp) == 1)
	{
	    services_flist = PIN_FLIST_ELEM_GET(read_oflist , PIN_FLD_SERVICES, PIN_ELEMID_ANY, 1, ebufp);
	}
	else
	{
	    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " No service is associated ");
	}

	/***
	* Prepare the flist to dis associate the device 
	*****/

	if(services_flist)
	{
	    update_iflistp = PIN_FLIST_CREATE(ebufp);
	    PIN_FLIST_FLD_SET(update_iflistp , PIN_FLD_POID , device_poid , ebufp);
	    PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_FLAGS, update_iflistp , PIN_FLD_FLAGS , ebufp);
	    PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_END_T , update_iflistp , PIN_FLD_END_T , ebufp);
	    PIN_FLIST_FLD_SET(update_iflistp , PIN_FLD_DESCR , " Disassociation after termination" , ebufp);
	    PIN_FLIST_FLD_SET(update_iflistp , PIN_FLD_PROGRAM_NAME , " Disassociation after termination" , ebufp);
	    service_flist = PIN_FLIST_ELEM_ADD(update_iflistp , PIN_FLD_SERVICES , 0, ebufp);
	    PIN_FLIST_CONCAT(service_flist , services_flist , ebufp);
	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Device Diss association input flist is " , update_iflistp);
	    PCM_OP(ctxp, 2703 , 0,update_iflistp , &update_oflistp , ebufp);
	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Device Diss association output flist is " , update_oflistp);

	    if (PIN_ERRBUF_IS_ERR(ebufp)) 
	    {
		    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Error in device disassociate ");
		    PIN_FLIST_DESTROY_EX(&update_oflistp , NULL);
		    PIN_ERRBUF_CLEAR(ebufp);
	    }
	}
	else
	{
	    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " No services hence no diss association ");
	}

	PIN_FLIST_DESTROY_EX(&update_iflistp , NULL);
	PIN_FLIST_DESTROY_EX(&read_iflist , NULL);
    }

	*o_flistp = PIN_FLIST_COPY(update_oflistp , ebufp);

	PIN_FLIST_DESTROY_EX(&update_oflistp , NULL);
	PIN_FLIST_DESTROY_EX(&read_oflist , NULL);
	return;
}
