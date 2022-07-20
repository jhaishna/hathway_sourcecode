/*
 * (#)$Id: fm_mso_device_vc_blacklist.c st_cgbubrm_gsivakum_bug-8453030/1 2009/05/05 23:37:48 gsivakum Exp $
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
*
*--------------------------------------------------------------------------------------------------------------*
****************************************************************************************************************/


#ifndef lint
static const char Sccs_id[] = "(#)$Id: fm_mso_device_vc_blacklist.c st_cgbubrm_gsivakum_bug-8453030/1 2009/05/05 23:37:48 gsivakum Exp $";
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


#define FILE_LOGNAME "fm_mso_device_vc_blacklist.c"


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
op_mso_device_vc_blacklist(
    cm_nap_connection_t    *connp,
    int32            opcode,
    int32            flags,
    pin_flist_t        *i_flistp,
    pin_flist_t        **o_flistpp,
    pin_errbuf_t        *ebufp);

static void
fm_mso_device_vc_blacklist(
    pcm_context_t        *ctxp,
    pin_flist_t        *i_flistp,
    int32              *error_flag,
    pin_flist_t        **ret_flistpp,
    pin_errbuf_t       *ebufp);

static void
fm_mso_get_vc_details(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    char                    *device_id,
	int32					*flag,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp);

/*******************************************************************************

 ******************************************************************************/
void
op_mso_device_vc_blacklist(
    cm_nap_connection_t    *connp,
    int32            opcode,
    int32            flags,
    pin_flist_t        *i_flistp,
    pin_flist_t        **o_flistpp,
    pin_errbuf_t        *ebufp)
{
    pcm_context_t        *ctxp = connp->dm_ctx;
    poid_t               *pdp = NULL;
    int32                t_status;
    int32                error_set = 0;
	int32				 flag=0;

    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR(ebufp);

    /*
     * Insanity check.
     */
    if (opcode != MSO_OP_DEVICE_BLACKLIST_VC) 
    {

        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
            PIN_ERR_BAD_OPCODE, 0, 0, opcode);

        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "bad opcode in op_mso_device_vc_blacklist", ebufp);

        *o_flistpp = NULL;
        return;
    }


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "op_mso_device_vc_blacklist input flist", i_flistp);

    pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
    t_status = fm_mso_trans_open(ctxp, pdp, 1, ebufp);

    fm_mso_device_vc_blacklist(ctxp, i_flistp, &error_set, o_flistpp, ebufp);

    if ((PIN_ERRBUF_IS_ERR(ebufp) ||  error_set == 1 ) && t_status == 0 ) 
    {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"abort transaction");
        fm_mso_trans_abort(ctxp, pdp, ebufp);
    }
    else if ( ! PIN_ERRBUF_IS_ERR(ebufp) &&   error_set == 0 && t_status == 0)
    {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"commit transaction");
        fm_mso_trans_commit(ctxp, pdp, ebufp);
    }

    if (PIN_ERRBUF_IS_ERR(ebufp) &&  error_set == 0 ) 
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_mso_device_vc_blacklist error", ebufp);
        //unhandled error -- Ideally should not 
        error_set = 1;
        PIN_ERRBUF_CLEAR(ebufp);
        *o_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *o_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_STATUS, &error_set, ebufp);
    }
    else if ( ! PIN_ERRBUF_IS_ERR(ebufp) &&   error_set == 0)
    {
        PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_STATUS, &error_set, ebufp);
    }

     PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "op_mso_device_vc_blacklist return flist",
            *o_flistpp);

    return;
}


/*******************************************************************************
 * fm_mso_device_vc_blacklist()
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
fm_mso_device_vc_blacklist(
    pcm_context_t        *ctxp,
    pin_flist_t        *i_flistp,
    int32               *error_flag,
    pin_flist_t        **ret_flistpp,
    pin_errbuf_t        *ebufp)
{
    char                    *vc_id = NULL;
    pin_flist_t             *vc_id_r_flistp = NULL;
    pin_flist_t             *set_state_flistp = NULL;
    pin_flist_t             *set_state_o_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    pin_flist_t             *sub_flistp = NULL;
    poid_t                  *vc_poidp = NULL;
    char                    *program_name = "Blacklist VC";
    int32                   blacklist = MSO_VC_STATE_BLACKLIST;
	int32					flag = 0; // VERIMATRIX

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        return;
    }
    PIN_ERRBUF_CLEAR(ebufp);

    /*
     * Validate the input flist if required fields are present
     */
    vc_id = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_VC_ID, 1, ebufp);
	
	if(vc_id == NULL) // VERIMATRIX - fetching STB_ID in case device details are not sent in VC_ID
	{
	  vc_id = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_STB_ID, 1, ebufp);
	  flag = 1;
	} 

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        PIN_ERRBUF_CLEAR(ebufp);
        *error_flag = 1;
        *ret_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, error_flag, ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "52007", ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "VC id is reqired in input", ebufp);
        goto CLEANUP;
    }


    /*
     * Get VC id poid
     */
    fm_mso_get_vc_details(ctxp, i_flistp, vc_id,&flag,&vc_id_r_flistp, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_GET(vc_id_r_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
    vc_poidp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_POID, 0, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        PIN_ERRBUF_CLEAR(ebufp);
        *error_flag = 1;
        *ret_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, error_flag, ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "52008", ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "Invalid VC id", ebufp);
        goto CLEANUP;
    }

    /*
     * Update VC: 
     *          : LCO, Network_node, paired STB 
     *           : Change state to preactivated
     */
    set_state_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_SET(set_state_flistp, PIN_FLD_POID, (void *) vc_poidp, ebufp);
    PIN_FLIST_FLD_SET(set_state_flistp, PIN_FLD_PROGRAM_NAME, program_name, ebufp);
    PIN_FLIST_FLD_SET(set_state_flistp, PIN_FLD_NEW_STATE, &blacklist, ebufp);


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                    "fm_mso_device_vc_blacklist set state input flist ", set_state_flistp);
    PCM_OP(ctxp, PCM_OP_DEVICE_SET_STATE, 0, set_state_flistp,
        ret_flistpp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                    "fm_mso_device_vc_blacklist set state output flist", *ret_flistpp);

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        PIN_ERRBUF_CLEAR(ebufp);
        *error_flag = 1;
        *ret_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, error_flag, ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "52009", ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "Black listing Failed for VC", ebufp);
        goto CLEANUP;
    }


CLEANUP:
    PIN_FLIST_DESTROY_EX(&set_state_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&vc_id_r_flistp, NULL);

    return;
}

/*******************************************************************************
 * fm_mso_get_vc_details()
 *
 * Description:
 * Search device from /device object
 ******************************************************************************/
static void
fm_mso_get_vc_details(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    char                    *device_id,
	int32					*flag,
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

    if (*flag == 0) // VERIMATRIX - setting poid type based on VC_ID or STB_ID 
	{
     device_pdp = (poid_t *)PIN_POID_CREATE(database, "/device/vc", -1, ebufp);
    }
	else 
	{
		device_pdp = (poid_t *)PIN_POID_CREATE(database, "/device/stb", -1, ebufp);
	}

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
    PIN_FLIST_FLD_PUT(tmp_flistp, PIN_FLD_POID,(void *)device_pdp, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_POID, NULL, ebufp);
    PIN_FLIST_ELEM_SET(tmp_flistp, NULL, PIN_FLD_SERVICES, PCM_RECID_ALL, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_get_vc_details search input flist ", search_i_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_i_flistp,
        r_flistpp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_get_vc_details search output flist", *r_flistpp);

    PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);

    return;
}

