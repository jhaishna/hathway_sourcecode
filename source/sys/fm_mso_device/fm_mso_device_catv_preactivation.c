/*
 * (#)$Id: fm_mso_device_catv_preactivation.c st_cgbubrm_gsivakum_bug-8453030/1 2009/05/05 23:37:48 gsivakum Exp $
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
* 0.1    |12/12/2013 |Satya Prakash     | Preactivation of STB and VC device implementation
*
*--------------------------------------------------------------------------------------------------------------*
****************************************************************************************************************/


#ifndef lint
static const char Sccs_id[] = "(#)$Id: fm_mso_device_catv_preactivation.c st_cgbubrm_gsivakum_bug-8453030/1 2009/05/05 23:37:48 gsivakum Exp $";
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


#define FILE_LOGNAME "fm_mso_device_catv_preactivation.c"

/*******************************************************************************
 * Functions contained within this source file
 ******************************************************************************/
EXPORT_OP void
op_mso_device_catv_preactivation(
    cm_nap_connection_t    *connp,
    int32            opcode,
    int32            flags,
    pin_flist_t        *i_flistp,
    pin_flist_t        **o_flistpp,
    pin_errbuf_t        *ebufp);

static void
fm_mso_device_catv_preactivation(
    pcm_context_t        *ctxp,
    pin_flist_t        *i_flistp,
    pin_flist_t        **ret_flistpp,
    pin_errbuf_t        *ebufp);

void
fm_mso_get_device_poid(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    char                    *device_id,
    int32                   flag,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp);

static void
fm_mso_update_device_details(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_errbuf_t            *ebufp);

static void
fm_mso_device_create_provisioning_request(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    poid_t                  *preactvated_svc_pdp,
    pin_errbuf_t            *ebufp);

static void
fm_mso_device_create_catv_preactivation_object(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    poid_t                  *stb_poidp,
    poid_t                  *vc_poidp,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp);

static void
validate_package_id(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_errbuf_t            *ebufp);

/*******************************************************************************

 ******************************************************************************/
void
op_mso_device_catv_preactivation(
    cm_nap_connection_t    *connp,
    int32            opcode,
    int32            flags,
    pin_flist_t        *i_flistp,
    pin_flist_t        **o_flistpp,
    pin_errbuf_t        *ebufp)
{
    pcm_context_t        *ctxp = connp->dm_ctx;

    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR(ebufp);

    /*
     * Insanity check.
     */
    if (opcode != MSO_OP_DEVICE_CATV_PREACTIVATION) 
    {

        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
            PIN_ERR_BAD_OPCODE, 0, 0, opcode);

        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "bad opcode in op_mso_device_catv_preactivation", ebufp);

        *o_flistpp = NULL;
        return;
    }


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "op_mso_device_catv_preactivation input flist", i_flistp);

    /*******************************************************************
     *   # number of field entries allocated 20, used 9
     *   0 PIN_FLD_POID           POID [0] 0.0.0.1 /catv_preactivation -1 0
     *   0 PIN_FLD_ACCOUNT_OBJ    POID [0] 0.0.0.1 /account 122833 23
     *   0 MSO_FLD_STB_ID          STR [0] "5940000000000007"
     *   0 MSO_FLD_VC_ID           STR [0] "000033155977"
     *   0 MSO_FLD_NETWORK_NODE    STR [0] "NDS"
     *   0 PIN_FLD_DEVICE_TYPE     STR [0] "PVR"
     *   0 MSO_FLD_CA_ID           STR [0] "276"
     *   0 MSO_FLD_REGION_KEY      STR [0] "IND600006"
     *   0 MSO_FLD_BOUQUET_ID      STR [0] "BOUQUET_ID"
     *********************************************************************/

    fm_mso_device_catv_preactivation(ctxp, i_flistp, o_flistpp, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp)) 
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_mso_device_catv_preactivation error", ebufp);
        *o_flistpp = NULL;
    }
    else
    {
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "op_mso_device_catv_preactivation return flist",
            *o_flistpp);
    }

    return;
}


/*******************************************************************************
 * fm_mso_device_catv_preactivation()
 *
 * Description:
 * for CISCO Box:
 *      Updates the LCO object and changes the state to preactivated.
 *      creates mso_catv_preactivate_svc service object
 *      raises provisioning order /mso_prov_order
 * for NDS Box:
 *      Updates the LCO object, pairs the devices and changes the state to preactivated.
 *      creates mso_catv_preactivate_svc service object
 *      raises provisioning order /mso_prov_order
 ******************************************************************************/
static void
fm_mso_device_catv_preactivation(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **ret_flistpp,
    pin_errbuf_t            *ebufp)
{
    char                    *stb_id = NULL;
    char                    *vc_id = NULL;
    pin_flist_t             *vc_r_flistp = NULL;
    pin_flist_t             *stb_r_flistp = NULL;
    pin_flist_t             *stb_flistp = NULL;
    pin_flist_t             *vc_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    pin_flist_t             *ret_flistp = NULL;
    poid_t                  *stb_poidp = NULL;
    poid_t                  *vc_poidp = NULL;
    poid_t                  *preactivate_svc_poidp = NULL;
    char                    *ne = NULL;
    char                    *ca_id = NULL;
    int32		    failure = 1;

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        return;
    }
    PIN_ERRBUF_CLEAR(ebufp);

    /*
     * Validate the input flist if required fields are present
     */
    ne = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);
//    ca_id = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_CA_ID, 0, ebufp);

    validate_package_id(ctxp, i_flistp, ebufp);
    if(PIN_ERRBUF_IS_ERR(ebufp))
    {
	PIN_ERRBUF_CLEAR(ebufp);
	ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51624", ebufp);
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "No Provisioning tag defined for the given CA_ID", ebufp);
	goto CLEANUP;
    }
    if (ne && strcmp(ne,"CISCO") == 0 )
    {
        stb_id = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_STB_ID, 0, ebufp);

        /*
         * Get STB poid 
         */
        fm_mso_get_device_poid(ctxp, i_flistp, stb_id, 2, &stb_r_flistp, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_GET(stb_r_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		PIN_FLIST_DESTROY_EX(&stb_r_flistp, NULL);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51040", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "No Device found for the given STB ID", ebufp);
		goto CLEANUP;
	}
	if(tmp_flistp != NULL)
	{
		stb_poidp = PIN_FLIST_FLD_TAKE(tmp_flistp, PIN_FLD_POID, 0, ebufp);

		/*
		 * Update STB: 
		 *           : LCO, Network_node
		 *           : Change state to preactivated
		 */
		stb_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(stb_flistp, PIN_FLD_POID, stb_poidp, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_NETWORK_NODE, stb_flistp, PIN_FLD_MANUFACTURER, ebufp);
	    //    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DEVICE_TYPE, stb_flistp, PIN_FLD_DEVICE_TYPE, ebufp);
		tmp_flistp = PIN_FLIST_ELEM_ADD(stb_flistp, MSO_FLD_STB_DATA, 0,ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, tmp_flistp, PIN_FLD_OWNER_OBJ, ebufp);

		fm_mso_update_device_details(ctxp, stb_flistp, ebufp);

		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
			PIN_FLIST_DESTROY_EX(&stb_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&stb_r_flistp, NULL);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51043", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "STB Device state can not be Preactivated from the current state", ebufp);
			goto CLEANUP;
		}
		/*
		 * Create /mso_catv_preactivate_svc object
		 */
		fm_mso_device_create_catv_preactivation_object ( ctxp, i_flistp, stb_poidp, (poid_t *)NULL, ret_flistpp, ebufp);

		preactivate_svc_poidp = PIN_FLIST_FLD_GET(*ret_flistpp, PIN_FLD_POID, 0, ebufp);

		/*
		 * Generate provisioning request
		 */
		fm_mso_device_create_provisioning_request ( ctxp, i_flistp, preactivate_svc_poidp, ebufp);
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
			PIN_FLIST_DESTROY_EX(&stb_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&stb_r_flistp, NULL);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51044", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in creating the Provisioning Request", ebufp);
			goto CLEANUP;
		}

		PIN_FLIST_DESTROY_EX(&stb_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&stb_r_flistp, NULL);
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " STB Device not present");
	}
    }
    else if (ne && strcmp(ne,"NDS") == 0)
    {
        stb_id = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_STB_ID, 0, ebufp);
        vc_id =  PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_VC_ID, 0, ebufp);

        /*
         * Get VC poid 
         */
        fm_mso_get_device_poid(ctxp, i_flistp, vc_id, 1, &vc_r_flistp, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_GET(vc_r_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_DESTROY_EX(&vc_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&stb_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&vc_r_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&stb_r_flistp, NULL);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51040", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "No Device found for the given VC ID", ebufp);
		goto CLEANUP;
	}
	if(tmp_flistp != NULL)
	{
           vc_poidp = PIN_FLIST_FLD_TAKE(tmp_flistp, PIN_FLD_POID, 0, ebufp);
	}

        /*
         * Get STB poid 
         */
        fm_mso_get_device_poid(ctxp, i_flistp, stb_id, 2, &stb_r_flistp, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_GET(stb_r_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		PIN_FLIST_DESTROY_EX(&vc_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&stb_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&vc_r_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&stb_r_flistp, NULL);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51040", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "No Device found for the given STB ID", ebufp);
		goto CLEANUP;
	}
	if(tmp_flistp != NULL)
	{
		stb_poidp = PIN_FLIST_FLD_TAKE(tmp_flistp, PIN_FLD_POID, 0, ebufp);
	}

        /*
         * Update VC: 
         *          : LCO, Network_node, paired STB 
         *           : Change state to preactivated
         */
        vc_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(vc_flistp, PIN_FLD_POID, vc_poidp, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_NETWORK_NODE, vc_flistp, PIN_FLD_MANUFACTURER, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(vc_flistp, MSO_FLD_VC_DATA, 0,ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_STB_OBJ, stb_poidp, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, tmp_flistp, PIN_FLD_OWNER_OBJ, ebufp);

        fm_mso_update_device_details(ctxp, vc_flistp, ebufp);
	
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		PIN_FLIST_DESTROY_EX(&vc_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&stb_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&vc_r_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&stb_r_flistp, NULL);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51043", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Device state can not be Preactivated from the current state", ebufp);
		goto CLEANUP;
	}
        /*
         * Update STB: 
         *           : LCO, Network_node, paired VC 
         *           : Change state to preactivated
         */
        stb_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(stb_flistp, PIN_FLD_POID, stb_poidp, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_NETWORK_NODE, stb_flistp, PIN_FLD_MANUFACTURER, ebufp);
    //    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DEVICE_TYPE, stb_flistp, PIN_FLD_DEVICE_TYPE, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(stb_flistp, MSO_FLD_STB_DATA, 0,ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_VC_OBJ, vc_poidp, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, tmp_flistp, PIN_FLD_OWNER_OBJ, ebufp);

        fm_mso_update_device_details(ctxp, stb_flistp, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		PIN_FLIST_DESTROY_EX(&vc_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&stb_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&vc_r_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&stb_r_flistp, NULL);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51043", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Device state can not be Preactivated from the current state", ebufp);
		goto CLEANUP;
	}

        /*
         * Create /mso_catv_preactivate_svc object
         */
        fm_mso_device_create_catv_preactivation_object ( ctxp, i_flistp, stb_poidp, vc_poidp, ret_flistpp, ebufp);

        preactivate_svc_poidp = PIN_FLIST_FLD_GET(*ret_flistpp, PIN_FLD_POID, 0, ebufp);

        /*
         * Generate provisioning request
         */
        fm_mso_device_create_provisioning_request ( ctxp, i_flistp, preactivate_svc_poidp, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		PIN_FLIST_DESTROY_EX(&vc_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&stb_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&vc_r_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&stb_r_flistp, NULL);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51044", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in creating the Provisioning Request", ebufp);
		goto CLEANUP;
	}

        PIN_FLIST_DESTROY_EX(&vc_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&stb_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&vc_r_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&stb_r_flistp, NULL);
        
    }
    else
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
    }

    CLEANUP:
	if(ret_flistp != NULL)
	{
	        *ret_flistpp = ret_flistp;
	}

        return;
}

/*******************************************************************************
 * fm_mso_get_device_poid()
 *
 * Description:
 * Search device from /device object
 ******************************************************************************/
void
fm_mso_get_device_poid(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    char                    *device_id,
    int32                   flag,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *search_i_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    poid_t                  *s_pdp = NULL;
    poid_t                  *pdp = NULL;
    poid_t                  *device_pdp = NULL;
    char                    *template =  NULL;
    int32                   s_flags = 0;
    int64                   database = 0;


    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR(ebufp);

    pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
    database = PIN_POID_GET_DB(pdp); 

    search_i_flistp = PIN_FLIST_CREATE(ebufp);
    s_pdp = (poid_t *)PIN_POID_CREATE(database, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(search_i_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

    s_flags = SRCH_DISTINCT ;
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

    template = "select X from /device where F1 = V1 and F2.type = V2 ";
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_DEVICE_ID, device_id, ebufp);

    if (flag == 1 )
    {
          device_pdp = (poid_t *)PIN_POID_CREATE(database, "/device/vc", -1, ebufp);
    }
    else if( flag == 2 )
    {
          device_pdp = (poid_t *)PIN_POID_CREATE(database, "/device/stb", -1, ebufp);
    }

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
    PIN_FLIST_FLD_PUT(tmp_flistp, PIN_FLD_POID,(void *)device_pdp, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_POID, NULL, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_get_device_poid search input flist ", search_i_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_i_flistp,
        r_flistpp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_get_device_poid search output flist", *r_flistpp);

    PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);

    return;
}


/*******************************************************************
 * fm_mso_update_device_details()
 *
 * Description:
 *  Update device with the input provided in the input flist
 *  change device state to preactivated
 *******************************************************************/
static void
fm_mso_update_device_details(
    pcm_context_t               *ctxp,
    pin_flist_t                 *i_flistp,
    pin_errbuf_t                *ebufp)
{
    pin_flist_t                 *set_attr_o_flistp = NULL;
    pin_flist_t                 *set_state_i_flistp = NULL;
    pin_flist_t                 *set_state_o_flistp = NULL;
    int32                       preactive =  MSO_STB_STATE_PREACTIVE;
    char                        *program = "catv_preactivation";

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
            return;
    }
    PIN_ERRBUF_CLEAR(ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                    "fm_mso_update_device_details input flist ", i_flistp);
    PCM_OP(ctxp, PCM_OP_WRITE_FLDS, PCM_OPFLG_ADD_ENTRY, i_flistp,
        &set_attr_o_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_update_device_details output flist", set_attr_o_flistp);

    set_state_i_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, set_state_i_flistp, PIN_FLD_POID, ebufp);
    PIN_FLIST_FLD_SET (set_state_i_flistp, PIN_FLD_NEW_STATE, &preactive, ebufp);
    PIN_FLIST_FLD_SET(set_state_i_flistp, PIN_FLD_PROGRAM_NAME, program, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                    "fm_mso_update_device_details set state input flist ", set_state_i_flistp);
    PCM_OP(ctxp, PCM_OP_DEVICE_SET_STATE, 0, set_state_i_flistp,
        &set_state_o_flistp, ebufp);
	
    if(PIN_ERRBUF_IS_ERR(ebufp))
    {
	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Device state can not be changed to pre activated ", ebufp);
	PIN_FLIST_DESTROY_EX(&set_attr_o_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&set_state_i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&set_state_o_flistp, NULL);
	return;
    }

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_update_device_details set state output flist", set_state_o_flistp);

    PIN_FLIST_DESTROY_EX(&set_attr_o_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&set_state_i_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&set_state_o_flistp, NULL);

    return;
}

/*******************************************************************
 * fm_mso_device_create_provisioning_request()
 *
 * Description:
 *  Call MSO_OP_PROV_CREATE_ACTION to generate provisioning request 
 *  /mso_prov_order object
 *******************************************************************/
static void
fm_mso_device_create_provisioning_request(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    poid_t                  *preactvated_svc_pdp,
    pin_errbuf_t            *ebufp) 
{
    pin_flist_t             *prov_i_flistp = NULL;
    pin_flist_t             *prov_o_flistp = NULL;
    int64                   database = 0;
    poid_t                  *pdp = NULL;
    poid_t                  *svc_pdp = NULL;
    poid_t                  *acc_pdp = NULL;
    int32                   flags = CATV_PREACTIVATION;


    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR(ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                    "fm_mso_device_create_provisioning_request input flist ", i_flistp);

    pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
    database = PIN_POID_GET_DB(pdp); 
    //dummy service poid, we don't have actual servive obj at this time
    svc_pdp = (poid_t *)PIN_POID_CREATE(database, "/service/catv", -1, ebufp); 
    //dummy account poid, we don't have actual account obj at this time
    acc_pdp = (poid_t *)PIN_POID_CREATE(database, "/account", -1, ebufp);

    prov_i_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_SET(prov_i_flistp, PIN_FLD_POID,(void *)preactvated_svc_pdp, ebufp);
    PIN_FLIST_FLD_PUT(prov_i_flistp, PIN_FLD_SERVICE_OBJ, (void *)svc_pdp, ebufp);
    PIN_FLIST_FLD_PUT(prov_i_flistp, PIN_FLD_ACCOUNT_OBJ, (void *)acc_pdp, ebufp);
    PIN_FLIST_FLD_SET(prov_i_flistp, PIN_FLD_FLAGS, &flags, ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_STB_ID, prov_i_flistp, MSO_FLD_STB_ID, ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_VC_ID, prov_i_flistp, MSO_FLD_VC_ID, ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_NETWORK_NODE, prov_i_flistp, MSO_FLD_NETWORK_NODE, ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CA_ID, prov_i_flistp, MSO_FLD_CA_ID, ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_REGION_KEY, prov_i_flistp, MSO_FLD_REGION_KEY, ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_BOUQUET_ID, prov_i_flistp, MSO_FLD_BOUQUET_ID, ebufp);


     PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
           "fm_mso_device_create_provisioning_request create order input flist ", prov_i_flistp);
    PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION, 0, prov_i_flistp, &prov_o_flistp, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp)) {
	    
	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR," Error In creating the Provisioning action", ebufp);
	PIN_FLIST_DESTROY_EX(&prov_i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&prov_o_flistp, NULL);
        return;
    }

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
           "fm_mso_device_create_provisioning_request create order output flist", prov_o_flistp);

    PIN_FLIST_DESTROY_EX(&prov_i_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&prov_o_flistp, NULL);

    return;

}

/*******************************************************************
 * fm_mso_device_create_catv_preactivation_object()
 *
 * Description:
 *  Create /mso_catv_preactivate_svc object
 *******************************************************************/
static void
fm_mso_device_create_catv_preactivation_object(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    poid_t                  *stb_poidp,
    poid_t                  *vc_poidp,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp)
{

    pin_flist_t             *create_i_flistp = NULL;
    pin_flist_t             *create_o_flistp = NULL;
    int64                   database = 0;
    poid_t                  *pdp = NULL;
    poid_t                  *poidp = NULL;
    int32                   status = MSO_DEVICE_PREACTIVATION_NEW;
    /* SachidL Added state_id */
    int32                   state_id = MSO_PREACTIVATED_SVC_ACTIVE;


    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR(ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                    "fm_mso_device_create_catv_preactivation_object input flist ", i_flistp);

    pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
    database = PIN_POID_GET_DB(pdp);

    create_i_flistp = PIN_FLIST_CREATE(ebufp);
    poidp = (poid_t *)PIN_POID_CREATE(database, "/mso_catv_preactivated_svc", -1, ebufp);
    PIN_FLIST_FLD_PUT(create_i_flistp, PIN_FLD_POID,(void *)poidp, ebufp);

    PIN_FLIST_FLD_SET(create_i_flistp, PIN_FLD_STATUS, &status, ebufp);

    PIN_FLIST_FLD_SET(create_i_flistp, MSO_FLD_STB_OBJ, (void *)stb_poidp, ebufp);
    PIN_FLIST_FLD_SET(create_i_flistp, MSO_FLD_VC_OBJ, (void *)vc_poidp, ebufp);   
    PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_NETWORK_NODE, create_i_flistp, MSO_FLD_NETWORK_NODE, ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CA_ID, create_i_flistp, MSO_FLD_CA_ID, ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_REGION_KEY, create_i_flistp, MSO_FLD_REGION_KEY, ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_BOUQUET_ID, create_i_flistp, MSO_FLD_BOUQUET_ID, ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DEVICE_TYPE, create_i_flistp, PIN_FLD_DEVICE_TYPE, ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, create_i_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
    /* Sachid: Setting initial PIN_FLD_STATE_ID to MSO_PREACTIVATED_SVC_ACTIVE */
    PIN_FLIST_FLD_SET(create_i_flistp, PIN_FLD_STATE_ID, &state_id, ebufp);


     PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
           "fm_mso_device_create_catv_preactivation_object create oject input flist ", create_i_flistp);
    PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, create_i_flistp, r_flistpp, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
           "fm_mso_device_create_catv_preactivation_object create object output flist", *r_flistpp);

    PIN_FLIST_DESTROY_EX(&create_i_flistp, NULL);

    return;
}

/*******************************************************************
 * validate_package_id()
 *
 * Description:
 *  Create /mso_catv_preactivate_svc object
 *******************************************************************/
static void
validate_package_id(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *search_i_flistp = NULL;
    pin_flist_t             *search_o_flistp = NULL;
    pin_flist_t             *data_flist = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    poid_t                  *s_pdp = NULL;
    poid_t                  *pdp = NULL;
    poid_t                  *device_pdp = NULL;
    char                    *template =  NULL;
    int32                   s_flags = 0;
    int64                   database = 0;
    char                    *ne = NULL;
    char                    *ca_id = NULL;
    int32                   count ;

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        return;
    }
    PIN_ERRBUF_CLEAR(ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                    "validate_package_id input flist ", i_flistp);

    ne = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);
    ca_id = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_CA_ID, 0, ebufp);


    pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
    database = PIN_POID_GET_DB(pdp); 

    search_i_flistp = PIN_FLIST_CREATE(ebufp);
    s_pdp = (poid_t *)PIN_POID_CREATE(database, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(search_i_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

    s_flags = SRCH_DISTINCT ;
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

    template = "select X from /mso_cfg_catv_pt where F1 = V1 ";
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
    if (strcmp(ne,"NDS") == 0)
    {
	data_flist = PIN_FLIST_ELEM_ADD(tmp_flistp,MSO_FLD_CATV_PT_DATA, 0, ebufp);
        PIN_FLIST_FLD_SET(data_flist, MSO_FLD_CA_ID, ca_id, ebufp);
    }
    else
    {
	data_flist = PIN_FLIST_ELEM_ADD(tmp_flistp,MSO_FLD_CATV_PT_DATA, 1, ebufp);
        PIN_FLIST_FLD_SET(data_flist, MSO_FLD_CA_ID, ca_id, ebufp);
    }

    tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_POID, NULL, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "validate_package_id search input flist ", search_i_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_i_flistp,
        &search_o_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "validate_package_id search output flist", search_o_flistp);

    count = PIN_FLIST_ELEM_COUNT(search_o_flistp, PIN_FLD_RESULTS, ebufp);

    if (count <= 0 )
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, MSO_FLD_CA_ID, 0, 0);
    }

    PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&search_o_flistp, NULL);

    return;
}

