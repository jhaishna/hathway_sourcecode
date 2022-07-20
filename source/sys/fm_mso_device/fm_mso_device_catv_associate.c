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
* 0.1    |15/02/2013 |Satya Prakash     | CATV Smart card & STB association and disassociation implementation

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
op_mso_device_catv_associate(
    cm_nap_connection_t    *connp,
    int32            opcode,
    int32            flags,
    pin_flist_t        *i_flistp,
    pin_flist_t        **o_flistpp,
    pin_errbuf_t        *ebufp);

static void
fm_mso_device_catv_associate(
    pcm_context_t        *ctxp,
    pin_flist_t        *i_flistp,
    pin_flist_t        *rfo_flistp,
    pin_errbuf_t        *ebufp);

static void
fm_mso_device_catv_disassociate(
    pcm_context_t        *ctxp,
    pin_flist_t        *i_flistp,
    pin_flist_t        *rfo_flistp,
    pin_errbuf_t        *ebufp);

static int32
fm_mso_device_check_dup_service(
    poid_t            *i_poidp,
    pin_flist_t        *i_flistp,
    pin_errbuf_t        *ebufp);

static void
fm_mso_device_alias_to_services(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    char                    *alias,
    char                    *ne,
    int32                   is_assct,
    pin_errbuf_t            *ebufp);

static void
fm_mso_device_check_service_status(
    pcm_context_t        *ctxp,
    poid_t            *i_poidp,
    pin_errbuf_t         *ebufp);

static void
fm_mso_device_set_warranty(
    pcm_context_t       *ctxp,
    pin_flist_t         *i_flistp,
    int32               *warranty_offset,
    char                *ne,
    pin_flist_t         *paired_flistp,
    pin_errbuf_t         *ebufp);

static void
    fm_mso_device_set_pairing(
    pcm_context_t       *ctxp,
    pin_flist_t         *i_flistp,
    pin_flist_t         **r_flistpp,
    pin_errbuf_t        *ebufp);

static void
    fm_mso_device_reset_pairing(
    pcm_context_t       *ctxp,
    pin_flist_t         *i_flistp,
    pin_errbuf_t         *ebufp);

extern void
fm_mso_device_set_category(
    pcm_context_t       *ctxp,
    pin_flist_t         *i_flistp,
	char				*prog_name,
    pin_errbuf_t        *ebufp);


/*******************************************************************************

 ******************************************************************************/
void
op_mso_device_catv_associate(
    cm_nap_connection_t    *connp,
    int32            opcode,
    int32            flags,
    pin_flist_t        *i_flistp,
    pin_flist_t        **o_flistpp,
    pin_errbuf_t        *ebufp)
{
    pcm_context_t        *ctxp = connp->dm_ctx;
    pin_flist_t        *rfi_flistp = NULL;
    pin_flist_t        *rfo_flistp = NULL;
    pin_flist_t        *sub_flistp = NULL;
    poid_t            *i_poidp = NULL;
    int32            *assoc_flag = NULL;
    char             *poid_type = NULL;


    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR(ebufp);

    /*
     * Insanity check.
     */
    if (opcode != MSO_OP_DEVICE_CATV_ASSOCIATE) {

        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
            PIN_ERR_BAD_OPCODE, 0, 0, opcode);

        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "bad opcode in op_num_pol_device_associate", ebufp);

        *o_flistpp = NULL;
        return;
    }


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "op_mso_device_catv_associate input flist", i_flistp);

    /*
     * Validate the input flist has the correct poid type.
     */
    i_poidp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
    poid_type = (poid_t *)PIN_POID_GET_TYPE (i_poidp);

    if ( strcmp(poid_type, MSO_OBJ_TYPE_DEVICE_STB)    != 0 &&
             strcmp(poid_type, MSO_OBJ_TYPE_DEVICE_VC )    != 0 )
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
            PIN_ERR_BAD_POID_TYPE, PIN_FLD_POID, 0, 0);

        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_mso_device_catv_associate error", ebufp);

        *o_flistpp = NULL;
        return;
    }

    /*
     * Validate the input flist has at least one service element
     */
    if (PIN_FLIST_ELEM_COUNT(i_flistp, PIN_FLD_SERVICES, ebufp) < 1) {

        pin_set_err(ebufp, PIN_ERRLOC_FLIST, PIN_ERRCLASS_APPLICATION,
            PIN_ERR_MISSING_ARG, PIN_FLD_SERVICES, 0, 0);

        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_mso_device_catv_associate error", ebufp);

        *o_flistpp = NULL;
        return;
    }

    /*
     * Read the existing services array for this device along with its
     * current state, values from the database.
     */
    rfi_flistp = PIN_FLIST_CREATE(ebufp);

    PIN_FLIST_FLD_SET(rfi_flistp, PIN_FLD_POID, (void *)i_poidp, ebufp);

    PIN_FLIST_FLD_SET(rfi_flistp, PIN_FLD_STATE_ID, NULL, ebufp);
    PIN_FLIST_FLD_SET(rfi_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);

    /*
     *  need the provisionable attribute for the service alias list which is
     *  in the device_id
     */
    PIN_FLIST_FLD_SET(rfi_flistp, PIN_FLD_DEVICE_ID, NULL, ebufp);

     // for warranty setting
    PIN_FLIST_FLD_SET(rfi_flistp, MSO_FLD_WARRANTY_END_OFFSET, NULL, ebufp);

    // For getting the network node
    PIN_FLIST_FLD_SET(rfi_flistp, PIN_FLD_MANUFACTURER, NULL, ebufp);

    PIN_FLIST_ELEM_SET(rfi_flistp, NULL, PIN_FLD_SERVICES,
        PIN_ELEMID_ANY, ebufp);

    PCM_OP(ctxp, PCM_OP_READ_FLDS, PCM_OPFLG_CACHEABLE, rfi_flistp,
        &rfo_flistp, ebufp);

    /*
     * Perform the association or disassociation based on the input flag
     */
    assoc_flag = (int32 *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS,
        0, ebufp);

    if ((assoc_flag != NULL) &&
        ((*assoc_flag & PIN_DEVICE_FLAG_ASSOCIATE) ==
        PIN_DEVICE_FLAG_ASSOCIATE))
    {
        fm_mso_device_catv_associate(ctxp, i_flistp, rfo_flistp, ebufp);
    }
    else
    {
        fm_mso_device_catv_disassociate(ctxp, i_flistp,
            rfo_flistp, ebufp);
    }

    if (PIN_ERRBUF_IS_ERR(ebufp)) {

        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_mso_device_catv_associate error", ebufp);
        *o_flistpp = NULL;

    }
    else
    {

        /*
         * Return a copy of the input flist
         */
        *o_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "op_stb_pol_device_assoicate return flist",
            *o_flistpp);
    }

    PIN_FLIST_DESTROY_EX(&rfi_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&rfo_flistp, NULL);

    return;
}


/*******************************************************************************
 * fm_mso_device_catv_associate()
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
fm_mso_device_catv_associate(
    pcm_context_t        *ctxp,
    pin_flist_t        *i_flistp,
    pin_flist_t        *rfo_flistp,
    pin_errbuf_t        *ebufp)
{
    pin_cookie_t        cookie = NULL;
    pin_flist_t         *i_serv_flistp = NULL;
    pin_flist_t         *db_serv_flistp = NULL;
    pin_flist_t         *dssi_flistp = NULL;
    pin_flist_t         *dsso_flistp = NULL;
    pin_flist_t         *r_flistp = NULL;
    poid_t              *i_serv_acct_poidp = NULL;
    poid_t              *i_serv_serv_poidp = NULL;
    poid_t              *db_serv_acct_poidp = NULL;
    poid_t              *db_dev_brand_poidp = NULL;
    int32               elem_id = 0;
    int32               next_state = 0;
    int32               *db_state_id = NULL;
    int32               brand_checked = PIN_BOOLEAN_FALSE;
    void                *vp = NULL;
    char                *prov_attr = NULL;
    poid_t              *i_poidp = NULL;
    int32               *warranty_offset = NULL;
    char                *ne = NULL;

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        return;
    }
    PIN_ERRBUF_CLEAR(ebufp);

    i_poidp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

    /*
     * Attempt to get an account_obj value from the existing services
     * of the device.  It is possible that the device has no services
     * at this time.
     */

    if ((db_serv_flistp = (pin_flist_t *)PIN_FLIST_ELEM_GET(rfo_flistp,
         PIN_FLD_SERVICES, PIN_ELEMID_ANY, 1, ebufp))
         != NULL)
    {

        db_serv_acct_poidp = (poid_t *)PIN_FLIST_FLD_GET(
            db_serv_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
    }

    /*
     * For each element of the input services array, validate the
     * account poid provided is the same as the account poid for the
     * existing services.
     */
    while(    (i_serv_flistp = (pin_flist_t *)
        PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_SERVICES,
        &elem_id, 1, &cookie, ebufp)) != NULL)
    {

        /*
         * Get the account poid and service poid from this input
         * service element
         */
        i_serv_acct_poidp = (poid_t *)PIN_FLIST_FLD_GET(
            i_serv_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);

        i_serv_serv_poidp = (poid_t *)PIN_FLIST_FLD_GET(
            i_serv_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);

        /*
         * If the service poid is not null, (i.e) service level association
         * Ensure their are no duplicate service types amongst the
         * input services.  The function should find one match which
         * which we acknowledge as the input poid from that flist.
         */


        if (! PIN_POID_IS_NULL(i_serv_serv_poidp) )
        {

            if (fm_mso_device_check_dup_service(i_serv_serv_poidp, i_flistp,
                ebufp) != 1)
            {

                pin_set_err(ebufp, PIN_ERRLOC_FM,
                    PIN_ERRCLASS_APPLICATION,
                    PIN_ERR_DUPLICATE,
                    PIN_FLD_SERVICE_OBJ, elem_id, 0);

                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                    "fm_mso_device_catv_associate duplicate "
                    "service type found in input flist", ebufp);

                break;
            }

            /*
             * Ensure their are no duplicate service types amongst the
             * existing services for this device.  The function should
             * should not find any matching service types.
             */
            if (fm_mso_device_check_dup_service(i_serv_serv_poidp, rfo_flistp,
                ebufp) != 0)
            {

                pin_set_err(ebufp, PIN_ERRLOC_FM,
                    PIN_ERRCLASS_APPLICATION,
                    PIN_ERR_DUPLICATE,
                    PIN_FLD_SERVICE_OBJ, elem_id, 0);

                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                    "fm_mso_device_catv_associate duplicate "
                    "service type found in existing services",
                    ebufp);

                break;
            }
        }
        /*
         * It is assumed that all current services in the database
         * for this device have the same account_obj value.
         * Compare the account_obj of the current input service element
         * with an arbitrary account_obj from the device's existing
         * services.
         */
        if (db_serv_acct_poidp == NULL)
        {
            db_serv_acct_poidp = i_serv_acct_poidp;
        }

        if (PIN_POID_COMPARE(i_serv_acct_poidp, db_serv_acct_poidp,
            0, ebufp) != 0)
        {

            pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_APPLICATION,
                PIN_COMPARE_NOT_EQUAL, PIN_FLD_ACCOUNT_OBJ,
                elem_id, 0);

            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                "fm_mso_device_catv_associate device has "
                "different account poid than provided as input",
                ebufp);

            break;
        }

        /*
         * If the service poid is not null, (i.e) service level association
         */

        if (! PIN_POID_IS_NULL(i_serv_serv_poidp) )
        {
            /*
             * If the status of the service is closed throw error message
             *
             */
            fm_mso_device_check_service_status(ctxp, i_serv_serv_poidp,
                ebufp);
            if (PIN_ERRBUF_IS_ERR(ebufp))
            {
                PIN_ERR_LOG_POID(PIN_ERR_LEVEL_ERROR,
                                "fm_mso_device_check_service_status Service poid status is closed",
                    i_serv_serv_poidp);

                return;
            }
        }
    }

    /* get the prov_attr */
    prov_attr = PIN_FLIST_FLD_GET(rfo_flistp, PIN_FLD_DEVICE_ID,
                   0, ebufp);

    warranty_offset = PIN_FLIST_FLD_GET(rfo_flistp, MSO_FLD_WARRANTY_END_OFFSET,
                   0, ebufp);

    ne = PIN_FLIST_FLD_GET(rfo_flistp, PIN_FLD_MANUFACTURER, 0, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        return;
    }


    /*
     * Set the warranty offset as 0 if not specified
     */
    if(!warranty_offset)
    {
        *warranty_offset = 0;
    }


    /*
     * Add the prov_attr to the services alias list in elem_id 1
     * PIN_BOOLEAN_TRUE for is_assct indicates this is
     * for association
     */
    fm_mso_device_alias_to_services(ctxp, i_flistp, prov_attr, ne,
                        PIN_BOOLEAN_TRUE, ebufp);

    /*
     * Pair STB with VC and viceversa 
     */
	/* Sachid: Call this function only for NDS as Cisco don't have VC */
    if(strstr(ne,"NDS") || strstr(ne, "GOSPELL")){
       fm_mso_device_set_pairing(ctxp, i_flistp, &r_flistp, ebufp);
    }

    /*
     * Set the warranty end date of the device 
     */
    /* Sachid: Pass manufacturer as new parameter */
    //fm_mso_device_set_warranty(ctxp, i_flistp, warranty_offset, r_flistp, ebufp);
    fm_mso_device_set_warranty(ctxp, i_flistp, warranty_offset, ne, r_flistp, ebufp);

    /*
     * Update the category attribute of device 
     */
    fm_mso_device_set_category(ctxp, i_flistp,FILE_LOGNAME, ebufp);
	
    /*
     * If the state of the device is New
     * set the state to Assigned.
     *
     * Note that additional constraints could be placed here to
     * restrict a quarantined number from becoming assigned.
     */

    db_state_id = (int32 *)PIN_FLIST_FLD_GET(rfo_flistp, PIN_FLD_STATE_ID,
        0, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        return;
    }

    if ( strcmp(PIN_POID_GET_TYPE(i_poidp), MSO_OBJ_TYPE_DEVICE_STB) == 0 )
    {
        switch (*db_state_id)
        {

        case MSO_STB_STATE_GOOD:
        case MSO_STB_STATE_PREACTIVE:
            dssi_flistp = PIN_FLIST_CREATE(ebufp);

            vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
            PIN_FLIST_FLD_SET(dssi_flistp, PIN_FLD_POID, vp, ebufp);

            next_state = MSO_STB_STATE_ALLOCATED;

            PIN_FLIST_FLD_SET(dssi_flistp, PIN_FLD_NEW_STATE,
                (void *)&next_state, ebufp);

            PCM_OP(ctxp, PCM_OP_DEVICE_SET_STATE, 0,
                dssi_flistp, &dsso_flistp, ebufp);

            PIN_FLIST_DESTROY_EX(&dssi_flistp, NULL);
            PIN_FLIST_DESTROY_EX(&dsso_flistp, NULL);
            break;
		 /*Repaired device Change*/
	case MSO_STB_STATE_REPAIRED:
            dssi_flistp = PIN_FLIST_CREATE(ebufp);

            vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
            PIN_FLIST_FLD_SET(dssi_flistp, PIN_FLD_POID, vp, ebufp);

            next_state = MSO_STB_STATE_ALLOCATED;

            PIN_FLIST_FLD_SET(dssi_flistp, PIN_FLD_NEW_STATE,
                (void *)&next_state, ebufp);

            PCM_OP(ctxp, PCM_OP_DEVICE_SET_STATE, 0,
                dssi_flistp, &dsso_flistp, ebufp);

            PIN_FLIST_DESTROY_EX(&dssi_flistp, NULL);
            PIN_FLIST_DESTROY_EX(&dsso_flistp, NULL);
            break;
        case MSO_STB_STATE_ALLOCATED:
        /*
         * The device is already in Assigned state; do nothing
         */
            break;
        default:
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_BAD_VALUE,
                PIN_FLD_STATE_ID, 0, *db_state_id);

            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                "fm_mso_device_catv_associate unknown state_id "
                "read for this device",    ebufp);
        }
    }
    else if ( strcmp(PIN_POID_GET_TYPE(i_poidp), MSO_OBJ_TYPE_DEVICE_VC) == 0  )
    {
        switch (*db_state_id) {

        case MSO_VC_STATE_GOOD:
        case MSO_VC_STATE_PREACTIVE:
            dssi_flistp = PIN_FLIST_CREATE(ebufp);

            vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
            PIN_FLIST_FLD_SET(dssi_flistp, PIN_FLD_POID, vp, ebufp);

            next_state = MSO_VC_STATE_ALLOCATED;

            PIN_FLIST_FLD_SET(dssi_flistp, PIN_FLD_NEW_STATE,
                (void *)&next_state, ebufp);

            PCM_OP(ctxp, PCM_OP_DEVICE_SET_STATE, 0,
                dssi_flistp, &dsso_flistp, ebufp);

            PIN_FLIST_DESTROY_EX(&dssi_flistp, NULL);
            PIN_FLIST_DESTROY_EX(&dsso_flistp, NULL);
            break;
			
		/*Repaired device Change*/
	case MSO_VC_STATE_REPAIRED:
            dssi_flistp = PIN_FLIST_CREATE(ebufp);

            vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
            PIN_FLIST_FLD_SET(dssi_flistp, PIN_FLD_POID, vp, ebufp);

            next_state = MSO_VC_STATE_ALLOCATED;

            PIN_FLIST_FLD_SET(dssi_flistp, PIN_FLD_NEW_STATE,
                (void *)&next_state, ebufp);

            PCM_OP(ctxp, PCM_OP_DEVICE_SET_STATE, 0,
                dssi_flistp, &dsso_flistp, ebufp);

            PIN_FLIST_DESTROY_EX(&dssi_flistp, NULL);
            PIN_FLIST_DESTROY_EX(&dsso_flistp, NULL);
            break;
        case MSO_VC_STATE_ALLOCATED:
        /*
         * The device is already in Assigned state; do nothing
         */
            break;
        default:
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_BAD_VALUE,
                PIN_FLD_STATE_ID, 0, *db_state_id);

            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                "fm_mso_device_catv_associate unknown state_id "
                "read for this device",    ebufp);
        }
    }
    return;
}


/*******************************************************************************
 * fm_mso_device_catv_disassociate()
 *
 * Inputs: input flist with device poid and services, flist with device poid
 *         and services from database
 * Outputs: void; ebuf set if errors encountered
 *
 * Description:
 * This function changes the state of the device to Quarantined if it is
 * currently Assigned and all of the current services are being disassociated.
 ******************************************************************************/
static void
fm_mso_device_catv_disassociate(
    pcm_context_t        *ctxp,
    pin_flist_t        *i_flistp,
    pin_flist_t        *rfo_flistp,
    pin_errbuf_t        *ebufp)
{
    pin_flist_t      *dssi_flistp = NULL;
    pin_flist_t      *dsso_flistp = NULL;
    int32            i_count = 0;
    int32            db_count = 0;
    int32            next_state = 0;
    int32            *db_state_id = NULL;
    void             *vp = NULL;
    char             *reason_code = NULL;
    char             *prov_attr = NULL;
    poid_t           *i_poidp = NULL;
    char             *ne = NULL;
    int32            *flags;

    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR(ebufp);

    i_poidp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
    flags = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 0 , ebufp);

    /*
     * remove the prov_attr from the alias list of the
     * disassociated services
     */

    /* get the prov_attr */
    prov_attr = PIN_FLIST_FLD_GET(rfo_flistp, PIN_FLD_DEVICE_ID,
                                   0, ebufp);

    ne = PIN_FLIST_FLD_GET(rfo_flistp, PIN_FLD_MANUFACTURER, 0, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp)) {
         return;
    }

    /* PIN_BOOLEAN_FALSE for is_assct indicates this is for disassociation */
    fm_mso_device_alias_to_services(ctxp, i_flistp, prov_attr, ne, PIN_BOOLEAN_FALSE, ebufp);

    fm_mso_device_reset_pairing(ctxp, i_flistp, ebufp);

    /*
     * Compare the number of services currently associated with the device
     * against the number of services on the input flist.
     */
    i_count = PIN_FLIST_ELEM_COUNT(i_flistp, PIN_FLD_SERVICES, ebufp);
    db_count = PIN_FLIST_ELEM_COUNT(rfo_flistp, PIN_FLD_SERVICES, ebufp);

    if (i_count < db_count) {

        /*
         * Not all of the services are being disassociated; do nothing
         */

    } else if (i_count == db_count) {

        /*
         * If the state of the device is Assigned, set the state
         * to Quarantined.  Else the current state does not support
         * service associations; report an error.
         */

        db_state_id = (int32 *)PIN_FLIST_FLD_GET(rfo_flistp,
            PIN_FLD_STATE_ID, 0, ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
            return;
        }

        if ( strcmp(PIN_POID_GET_TYPE(i_poidp), MSO_OBJ_TYPE_DEVICE_STB) == 0 )
        {
            switch (*db_state_id) {

            case MSO_STB_STATE_ALLOCATED:

                dssi_flistp = PIN_FLIST_CREATE(ebufp);

                vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID,
                    0, ebufp);
                PIN_FLIST_FLD_SET(dssi_flistp, PIN_FLD_POID, vp, ebufp);

                if ( *flags == MSO_DEVICE_DEACTIVATION_REASON_FAULTY)
                {
                    next_state = MSO_STB_STATE_FAULTY;
                }
                // Complete deactivation of services & disassociaton of temporary devices
                else
                {
                    next_state = MSO_STB_STATE_GOOD;
                }

               	// Changes for Repaired Device
				
		if ( *flags == MSO_DEVICE_ACTIVATION_REASON_REPAIRED)
                {
                    next_state = MSO_STB_STATE_REPAIRED;
                }

                PIN_FLIST_FLD_SET(dssi_flistp, PIN_FLD_NEW_STATE,
                    (void *)&next_state, ebufp);

                PCM_OP(ctxp, PCM_OP_DEVICE_SET_STATE, 0,
                    dssi_flistp, &dsso_flistp, ebufp);

                PIN_FLIST_DESTROY_EX(&dssi_flistp, NULL);
                PIN_FLIST_DESTROY_EX(&dsso_flistp, NULL);

                break;

            default:

                pin_set_err(ebufp, PIN_ERRLOC_FM,
                    PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    PIN_ERR_BAD_VALUE, PIN_FLD_STATE_ID,
                    0, *db_state_id);

                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                    "fm_mso_device_catv_disassociate invalid "
                    "state_id for disassociation of services",
                    ebufp);

            }
        }
        else if ( strcmp(PIN_POID_GET_TYPE(i_poidp), MSO_OBJ_TYPE_DEVICE_VC) == 0 )
        {
            switch (*db_state_id) {

            case MSO_VC_STATE_ALLOCATED:

                dssi_flistp = PIN_FLIST_CREATE(ebufp);

                vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID,
                    0, ebufp);
                PIN_FLIST_FLD_SET(dssi_flistp, PIN_FLD_POID, vp, ebufp);

                //Faulty device
                if ( *flags == MSO_DEVICE_DEACTIVATION_REASON_FAULTY)
                {
                    next_state = MSO_VC_STATE_FAULTY;
                }
                // Complete deactivation of services & disassociaton of temporary devices
                else
                {
                    next_state = MSO_VC_STATE_GOOD;
                }
               
                // Changes for Repaired Device

                if ( *flags == MSO_DEVICE_ACTIVATION_REASON_REPAIRED)
                {
                    next_state = MSO_VC_STATE_REPAIRED;
                }

                PIN_FLIST_FLD_SET(dssi_flistp, PIN_FLD_NEW_STATE,
                    (void *)&next_state, ebufp);

                PCM_OP(ctxp, PCM_OP_DEVICE_SET_STATE, 0,
                    dssi_flistp, &dsso_flistp, ebufp);

                PIN_FLIST_DESTROY_EX(&dssi_flistp, NULL);
                PIN_FLIST_DESTROY_EX(&dsso_flistp, NULL);

                break;

            default:

                pin_set_err(ebufp, PIN_ERRLOC_FM,
                    PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    PIN_ERR_BAD_VALUE, PIN_FLD_STATE_ID,
                    0, *db_state_id);

                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                    "fm_mso_device_catv_disassociate invalid "
                    "state_id for disassociation of services",
                    ebufp);

            }
        }
    } else {  /* (i_count > db_count) */

        pin_set_err(ebufp, PIN_ERRLOC_APP, PIN_ERRCLASS_APPLICATION,
            PIN_ERR_BAD_VALUE, PIN_FLD_SERVICES, 0, i_count);

        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "fm_mso_device_catv_disassociate more input services "
            "than device currently has", ebufp);
    }

    return;
}


/*******************************************************************************
 * fm_mso_device_check_dup_service()
 *
 * Inputs: service poid; flist with services array of service_obj values
 * Outputs: number of services with the same type as the input poid;
 *          ebuf set if errors encountered
 *
 * Description:
 * This function checks for duplicate service types in the services array.
 ******************************************************************************/
static int32
fm_mso_device_check_dup_service(
    poid_t            *i_poidp,
    pin_flist_t        *i_flistp,
    pin_errbuf_t        *ebufp)
{
    pin_cookie_t        cookie = NULL;
    pin_flist_t         *i_serv_flistp = NULL;
    poid_t              *i_serv_serv_poidp = NULL;
    int32               elem_id = 0;
    int32               dup_count = 0;


    if (PIN_ERRBUF_IS_ERR(ebufp)) {

        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "fm_mso_device_check_dup_service error", ebufp);

        return(-1);
    }
    PIN_ERRBUF_CLEAR(ebufp);


    while(    (i_serv_flistp = (pin_flist_t *)
        PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_SERVICES,
        &elem_id, 1, &cookie, ebufp)) != NULL) {

        /*
         * get the service poid from this input service element
         */
        i_serv_serv_poidp = (poid_t *)PIN_FLIST_FLD_GET(
            i_serv_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);

        if( ! PIN_POID_IS_NULL(i_serv_serv_poidp) ) {
            if ( strcmp(PIN_POID_GET_TYPE(i_poidp),
                PIN_POID_GET_TYPE(i_serv_serv_poidp)) == 0 ){
                dup_count++;
            }
        }

    }

    return(dup_count);
}






/*******************************************************************
 * Function used to add or delete an Provisionable attribute alias
 * to/from the service alias list that are passed into the i_flist
 * input:
 *  i_flist: an flist, where we get services poid or account poid
 *           from the PIN_FLD_SERVICES array
 *           An assumption is that only the services of one account
 *           are are listed here
 *  is_assct : This flag indicates whether this function is called from
 *          associate or disassociate. (TRUE/FALSE)
 *  alias :  alias to add or delete.
 *
 * output:
 *  ebuf: set in case of error
 *******************************************************************/
static void
fm_mso_device_alias_to_services(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        char                    *alias,
        char                    *ne,
        int32                   is_assct,
        pin_errbuf_t            *ebufp)
{
    pin_flist_t             *s_flistp = NULL;      /* set flist     */
    pin_flist_t             *log_flistp = NULL;    /* login flist   */
    pin_flist_t             *tmp_flistp = NULL;     /* alias flist   */
    pin_flist_t             *r_flistp = NULL;      /* return flist  */
    pin_flist_t             *svc_flistp = NULL;    /* service list flist*/
    pin_flist_t             *alias_flistp = NULL;    /* alias list flist*/
    pin_flist_t             *alias_in_flistp = NULL;    /* alias list flist*/
    poid_t                  *svc_poid = NULL;      /* service poid */
    poid_t                  *acct_poid = NULL;     /* account poid */
    int32                   elem_id = 0;    /* array element        */
    int32                   elem_id1 = -1;    /* array element        */
    pin_cookie_t            cookie = NULL;  /* to loop through array */
    void                    *vp=NULL;
    char                    *al_name = NULL;
    poid_t                  *dev_poidp = NULL;
    char                    *device_type = NULL;

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
            return;
    }

    PIN_ERRBUF_CLEAR(ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                    "fm_mso_device_alias_to_services input flist ", i_flistp);
    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, ne);


    /*
     * Get the device poid
     */
    dev_poidp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

    /*
     * Get the device poid type
     */
    device_type = (char *) PIN_POID_GET_TYPE(dev_poidp);


    /*
     * loop through the array of all services and
     * build SET_LOGIN input flist and call the opcode
     */
    /* if END_T is passed by caller */
    vp=PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_END_T,1,ebufp);

    while((svc_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_SERVICES,
            &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
    {
         svc_poid = PIN_FLIST_FLD_GET(svc_flistp, PIN_FLD_SERVICE_OBJ,
                                         1, ebufp);

         /*
          * No need to call SET_LOGIN opcode if service poid doesnt exist
          */
         if ( PIN_POID_IS_NULL(svc_poid) )
         {
               continue;
         }

         /*
         * create and set the update_services input flist
        */
        s_flistp = PIN_FLIST_CREATE(ebufp);

        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_PROGRAM_NAME, FILE_LOGNAME,
                      ebufp);

        /******************************************************************
         * -> Read ALIAS_LIST from svc_obj.
        *
        * -> If this function is called because of associate then
        *    find the max_elem_id and prepare the input to cust_set_login
         *    to add the new element in the ALIAS_LIST.
         *
         * -> If this function is called because of disassociate then
         *    loop through existing alias list and get the matching elem_id.
         *    Use this elem id to delete the ALIAS_LIST.
         *
         * -> call cust_set_login.
         *
         * -> Above steps need to be done for each service which are associated
         *    with this device.
         *
         * -> In case of same device associated then it is handled in
         *    fm_device_associate(). So no need to handle in this case.
         *******************************************************************/

        log_flistp = PIN_FLIST_CREATE(ebufp);
        alias_in_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(alias_in_flistp, PIN_FLD_POID, (void *)svc_poid, ebufp);
        PIN_FLIST_ELEM_SET(alias_in_flistp, NULL, PIN_FLD_ALIAS_LIST,
            PIN_ELEMID_ANY, ebufp);

        PCM_OP(ctxp, PCM_OP_READ_FLDS, PCM_OPFLG_CACHEABLE, alias_in_flistp,
            &alias_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "Alias List in service object", alias_flistp);
        /*
         * elem_id hardcoded? I will say this is as per design!!
         *
         * Set the alias elem id
         *      For STB it is 0 for NDS
         *      For STB it is 2 for CISCO
         *      For VC it is 1
         */
		/******* VERIMATRIX CHANGES - Enhanced condition to check for verimatrix network node ***/
        if (!strcmp(device_type,MSO_OBJ_TYPE_DEVICE_STB) &&
            ((strstr(ne,"VM")) || (strstr(ne,"NDS"))|| (strstr(ne,"GOSPELL"))
		|| (strstr(ne,"NAGRA"))||(strstr(ne,"NONE"))) )
        {
              elem_id1 = 0;
        }
        else if (!strcmp(device_type,MSO_OBJ_TYPE_DEVICE_STB) &&
            strstr(ne,"CISCO") )
        {
              elem_id1 = 2;
        }
        else if(!strcmp(device_type,MSO_OBJ_TYPE_DEVICE_VC))
        {
              elem_id1 = 1;
        }

        if (is_assct == PIN_BOOLEAN_TRUE)  /* Caller is pol_device_associate  */
        {

            tmp_flistp = PIN_FLIST_ELEM_ADD(log_flistp, PIN_FLD_ALIAS_LIST,
                               elem_id1, ebufp);
            PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_NAME, alias, ebufp);
        }
        else  /* Caller is pol_device_disassociate */
        {
            tmp_flistp = PIN_FLIST_ELEM_GET(alias_flistp, PIN_FLD_ALIAS_LIST, elem_id1, 0, ebufp);
            al_name = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

            if (PIN_ERRBUF_IS_ERR(ebufp))
            {
                    goto cleanup;
            }

            if (strcmp(al_name, alias) == 0)
            {
                /*
                * Remove the alias from the service alias list
                * set the flist to NULL for the elem_id
                */
                PIN_FLIST_ELEM_SET(log_flistp, (void*)NULL,
                          PIN_FLD_ALIAS_LIST, elem_id1, ebufp);
            }
        }

        acct_poid = PIN_FLIST_FLD_GET(svc_flistp,
                                          PIN_FLD_ACCOUNT_OBJ,
                                          0, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_POID,
                          acct_poid, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_SERVICE_OBJ,
                          svc_poid, ebufp);

        PIN_FLIST_ELEM_PUT(s_flistp, log_flistp,
                           PIN_FLD_LOGINS, 0 , ebufp);
        /* if END_T is not NULL set it */
        if(vp)
        {
                PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_END_T,vp,ebufp);
        }
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        	    "PCM_OP_CUST_SET_LOGIN input flist", s_flistp);

        PCM_OP(ctxp, PCM_OP_CUST_SET_LOGIN, 0, s_flistp,
               &r_flistp, ebufp);

cleanup:
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&alias_in_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&alias_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
    }

}

/*******************************************************************
 * Function used to validate the Service status
 * input:
 *  service_poidp: service poid
 * output:
 *  ebuf: set in case of error
 *******************************************************************/
static void
fm_mso_device_check_service_status(
    pcm_context_t        *ctxp,
    poid_t            *service_poidp,
    pin_errbuf_t         *ebufp) {

    pin_flist_t             *service_inp_flistp = NULL;
    pin_flist_t             *service_out_flistp = NULL;
    pin_status_t        *service_statep = NULL;

    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }

    service_inp_flistp = PIN_FLIST_CREATE(ebufp);

    PIN_FLIST_FLD_SET(service_inp_flistp, PIN_FLD_POID, (void *)service_poidp, ebufp);

    PIN_FLIST_FLD_SET(service_inp_flistp, PIN_FLD_STATUS, NULL, ebufp);

    /*
     * Read the Service Poid and Get the service status.
     */

    PCM_OP(ctxp, PCM_OP_READ_FLDS, PCM_OPFLG_CACHEABLE, service_inp_flistp,
        &service_out_flistp, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "fm_mso_device_check_service_status PCM_OP_READ_FLDS error", ebufp);

        goto cleanup;
    }

    service_statep = (pin_status_t *) PIN_FLIST_FLD_GET(service_out_flistp,
            PIN_FLD_STATUS, 0, ebufp);


    if (service_statep == NULL) {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "fm_mso_device_check_service_status service status is null", ebufp);

        goto cleanup;
    }

    if (*service_statep == PIN_STATUS_CLOSED) {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
            PIN_ERR_BAD_VALUE, PIN_FLD_STATUS, 0, 0);

        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "fm_mso_device_check_service_status Service status is closed ", ebufp);

        goto cleanup;
    }

    cleanup:
        PIN_FLIST_DESTROY_EX(&service_inp_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&service_out_flistp, NULL);

    return;

}

/*******************************************************************
 * Function used to set the warranty of the device
 * warranty end = current time + warranty offset in months
 *******************************************************************/
static void
fm_mso_device_set_warranty(
    pcm_context_t       *ctxp,
    pin_flist_t         *i_flistp,
    int32               *warranty_offset,
    char                *ne,
    pin_flist_t         *paired_flistp,
    pin_errbuf_t        *ebufp)
{
    pin_flist_t         *w_in_flistp = NULL;
    pin_flist_t         *w_o_flistp = NULL;
    pin_flist_t         *read_i_flistp = NULL;
    pin_flist_t         *read_o_flistp = NULL;
    pin_flist_t         *tmp_flistp = NULL;
    pin_flist_t         *sub_flistp = NULL;
    poid_t              *prev_device_pdp = NULL;
    poid_t              *device_pidp = NULL;
    int32               *prev_warranty = NULL;
    char                *device_type = NULL;
    time_t              now;
    time_t              warranty_end;
    struct              tm *current_time;
    int32               *flags = NULL ;
    int32               count = 0;



    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }

    device_pidp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
    device_type = (char *) PIN_POID_GET_TYPE (device_pidp);
    flags = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 0 , ebufp);

    if ( *flags == MSO_DEVICE_ACTIVATION_NEW)
    {
        /*
         * warranty end = current time + warranty offset in months
         */
        now = pin_virtual_time((time_t *)NULL);
        current_time = localtime(&now);
        current_time->tm_year += *warranty_offset / 12;
        current_time->tm_mon += *warranty_offset % 12 ;
        warranty_end = mktime(current_time);
    }
    else
    {
         PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "already paired device information", paired_flistp);

	/* Sachid: Check if manufacturer is NDS */
        if (!strcmp(device_type,MSO_OBJ_TYPE_DEVICE_STB) && (strstr(ne,"NDS") || strstr(ne,"GOSPELL")))
        {
            tmp_flistp = PIN_FLIST_ELEM_GET(paired_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
            count = PIN_FLIST_ELEM_COUNT(tmp_flistp, MSO_FLD_VC_DATA, ebufp);
            if (count == 1)
            {
                //sub_flistp = PIN_FLIST_ELEM_GET(tmp_flistp, MSO_FLD_VC_DATA, 0, 0, ebufp);
                sub_flistp = PIN_FLIST_ELEM_GET(tmp_flistp, MSO_FLD_VC_DATA, PCM_RECID_ALL, 0, ebufp);
                prev_device_pdp = PIN_FLIST_FLD_GET( sub_flistp, MSO_FLD_STB_OBJ, 0, ebufp);
            }
            else
            {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                    PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    PIN_ERR_NOT_FOUND, 0, 0, MSO_FLD_VC_DATA);
                return;
            }
        }
		/* Sachid: Handling the case for non-NDS networks */
        else if(!strcmp(device_type,MSO_OBJ_TYPE_DEVICE_STB) && !strstr(ne,"NDS") && !strstr(ne, "GOSPELL")){
	    //PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG,"Ebuf",ebufp);
	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Swap for non-NDS, input flist:",i_flistp);
	    //PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Swap for non-NDS, input flist:",PIN_FLIST_ELEM_GET(i_flistp,PIN_FLD_SERVICES,0,0, ebufp));
	    sub_flistp = PIN_FLIST_SUBSTR_GET(i_flistp,PIN_FLD_EXTENDED_INFO,0, ebufp);
	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Swap for non-NDS, sub flist:",sub_flistp);
	    //sub1_flistp = PIN_FLIST_ELEM_GET(sub_flistp,MSO_FLD_STB_DATA,0,0, ebufp);
	    //PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Swap for non-NDS, sub1 flist:",sub1_flistp);
	    prev_device_pdp = PIN_FLIST_FLD_GET( sub_flistp,MSO_FLD_VC_OBJ, 0 , ebufp);
	    PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Swap for non-NDS, MSO_FLD_VC_OBJ",prev_device_pdp);
	    if(PIN_POID_IS_NULL(prev_device_pdp)){
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Previous STB device not found");
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                    PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    PIN_ERR_NOT_FOUND, 0, 0, MSO_FLD_VC_DATA);
                return;
	    }
	}
        else if(!strcmp(device_type,MSO_OBJ_TYPE_DEVICE_VC))
        {
            tmp_flistp = PIN_FLIST_ELEM_GET(paired_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
            count = PIN_FLIST_ELEM_COUNT(tmp_flistp, MSO_FLD_STB_DATA, ebufp);
            if (count == 1)
            {
              //sub_flistp = PIN_FLIST_ELEM_GET(tmp_flistp, MSO_FLD_STB_DATA, 0, 0, ebufp);
              sub_flistp = PIN_FLIST_ELEM_GET(tmp_flistp, MSO_FLD_STB_DATA, PCM_RECID_ALL, 0, ebufp);
              prev_device_pdp = PIN_FLIST_FLD_GET( sub_flistp, MSO_FLD_VC_OBJ, 0, ebufp);
            }
            else
            {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                    PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    PIN_ERR_NOT_FOUND, 0, 0, MSO_FLD_STB_DATA);
                return;
            }
        }

        read_i_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET( read_i_flistp, PIN_FLD_POID, (void *) prev_device_pdp, ebufp);
        PIN_FLIST_FLD_SET( read_i_flistp, MSO_FLD_WARRANTY_END, NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "fm_mso_device_set_warranty read prev device input flist", read_i_flistp);

        PCM_OP(ctxp, PCM_OP_READ_FLDS, PCM_OPFLG_CACHEABLE, read_i_flistp,
                &read_o_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "fm_mso_device_set_warranty read prev device output flist", read_o_flistp);


        prev_warranty = PIN_FLIST_FLD_GET(read_o_flistp, MSO_FLD_WARRANTY_END, 0, ebufp);

        warranty_end = *prev_warranty;
    }

    w_in_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, w_in_flistp, PIN_FLD_POID, ebufp);
    PIN_FLIST_FLD_SET(w_in_flistp, MSO_FLD_WARRANTY_END, &warranty_end, ebufp);
    
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_device_set_warranty write_flds input flist", w_in_flistp);
    PCM_OP(ctxp, PCM_OP_WRITE_FLDS, PCM_OPFLG_CACHEABLE, w_in_flistp,
        &w_o_flistp, ebufp);

     PIN_FLIST_DESTROY_EX(&read_i_flistp, NULL);
     PIN_FLIST_DESTROY_EX(&read_o_flistp, NULL);
     PIN_FLIST_DESTROY_EX(&w_in_flistp, NULL);
     PIN_FLIST_DESTROY_EX(&w_o_flistp, NULL);


return;
}

/*******************************************************************
 * Function used to set the pairing of the device
 *******************************************************************/
static void
    fm_mso_device_set_pairing(
    pcm_context_t       *ctxp,
    pin_flist_t         *i_flistp,
    pin_flist_t         **r_flistpp,
    pin_errbuf_t         *ebufp)
{

    pin_flist_t         *search_i_flistp = NULL;
//    pin_flist_t         *search_o_flistp = NULL;
    pin_flist_t         *w_in_flistp = NULL;
    pin_flist_t         *w_o_flistp = NULL;
    pin_flist_t         *tmp_flistp = NULL;
    pin_flist_t         *sub_flistp = NULL;
    pin_flist_t         *rec_id_flistp = NULL;
    pin_status_t        *service_statep = NULL;
    poid_t              *service_pdp = NULL;      /* service poid */
    poid_t              *device_pdp = NULL;
    poid_t              *device_pidp = NULL;
    poid_t              *s_pdp = NULL;
    int32               s_flags;
    time_t              now;
    int64               database;
    char                *template = NULL;
    char                *device_type = NULL;
    int32               count = 0;
    int32   		rec_id = 0;
    pin_cookie_t        rec_id_cookie = NULL;

    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }

      /*
      * Find already connected STB or VC with the service 
      */
    tmp_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY, 0, ebufp);
    device_pidp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
    device_type = (char *) PIN_POID_GET_TYPE (device_pidp);

    service_pdp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
    database = (int64) PIN_POID_GET_DB(service_pdp);

    if (!strcmp(device_type,MSO_OBJ_TYPE_DEVICE_STB) )
    {
          device_pdp = (poid_t *)PIN_POID_CREATE(database, "/device/vc", -1, ebufp);
    }
    else if(!strcmp(device_type,MSO_OBJ_TYPE_DEVICE_VC))
    {
          device_pdp = (poid_t *)PIN_POID_CREATE(database, "/device/stb", -1, ebufp);
    }

    search_i_flistp = PIN_FLIST_CREATE(ebufp);

    s_pdp = (poid_t *)PIN_POID_CREATE(database, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(search_i_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

    s_flags = SRCH_DISTINCT ;
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

    template = "select X from /device where F1 = V1 and F2.type = V2 ";
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
    sub_flistp = PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY, ebufp);
    PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_SERVICE_OBJ, (void *)service_pdp, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
    PIN_FLIST_FLD_PUT(tmp_flistp, PIN_FLD_POID,(void *)device_pdp, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_POID, NULL, ebufp);

        if (!strcmp(device_type,MSO_OBJ_TYPE_DEVICE_STB) )
        {
            sub_flistp = PIN_FLIST_ELEM_ADD (tmp_flistp, MSO_FLD_VC_DATA, PIN_ELEMID_ANY, ebufp);
            PIN_FLIST_FLD_SET (sub_flistp, MSO_FLD_STB_OBJ, NULL, ebufp);
        }
        else if(!strcmp(device_type,MSO_OBJ_TYPE_DEVICE_VC))
        {
            sub_flistp = PIN_FLIST_ELEM_ADD (tmp_flistp, MSO_FLD_STB_DATA, PIN_ELEMID_ANY, ebufp);
            PIN_FLIST_FLD_SET (sub_flistp, MSO_FLD_VC_OBJ, NULL, ebufp);
        }

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_device_set_pairing search input flist ", search_i_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_i_flistp,
        r_flistpp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_device_set_pairing search output flist", *r_flistpp);
    
    count = PIN_FLIST_ELEM_COUNT(*r_flistpp, PIN_FLD_RESULTS, ebufp);

    if (count == 1)
    {
          /*
          * Update the pairing detail for
          * existing device as well as current device
          */
        tmp_flistp = PIN_FLIST_ELEM_GET(*r_flistpp, PIN_FLD_RESULTS, 0, 0, ebufp);
	
	if (!strcmp(device_type,MSO_OBJ_TYPE_DEVICE_STB) )
        {
		rec_id = 0;
		rec_id_cookie = NULL;
	 	while((rec_id_flistp = (pin_flist_t *)PIN_FLIST_ELEM_GET_NEXT(tmp_flistp, MSO_FLD_VC_DATA,
        		&rec_id, 1, &rec_id_cookie, ebufp)) != NULL)
    		{
		// This while loop is to get the appropriate recid 	
		}
        	w_in_flistp = PIN_FLIST_CREATE(ebufp);
        	PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_POID, w_in_flistp, PIN_FLD_POID, ebufp);
		sub_flistp = PIN_FLIST_ELEM_ADD (w_in_flistp, MSO_FLD_VC_DATA, rec_id, ebufp);
            	PIN_FLIST_FLD_SET(sub_flistp, MSO_FLD_STB_OBJ, device_pidp, ebufp);
		

	}
        else if(!strcmp(device_type,MSO_OBJ_TYPE_DEVICE_VC))
        {
		rec_id = 0;
                rec_id_cookie = NULL;
                while((rec_id_flistp = (pin_flist_t *)PIN_FLIST_ELEM_GET_NEXT(tmp_flistp, MSO_FLD_STB_DATA,
                        &rec_id, 1, &rec_id_cookie, ebufp)) != NULL)
                {
		// This while loop is to get the appropriate recid
        	}
        	w_in_flistp = PIN_FLIST_CREATE(ebufp);
        	PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_POID, w_in_flistp, PIN_FLD_POID, ebufp);
            	sub_flistp = PIN_FLIST_ELEM_ADD (w_in_flistp, MSO_FLD_STB_DATA, rec_id, ebufp);
            	PIN_FLIST_FLD_SET(sub_flistp, MSO_FLD_VC_OBJ, device_pidp, ebufp);
   	} 
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "fm_mso_device_set_pairing write_flds for existing device input flist", w_in_flistp);
         /*
          * Update the pairing detail for existing device
          */
        PCM_OP(ctxp, PCM_OP_WRITE_FLDS, PCM_OPFLG_CACHEABLE, w_in_flistp,
            &w_o_flistp, ebufp);

        PIN_FLIST_DESTROY_EX(&w_o_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&w_in_flistp, NULL);
        
        w_in_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(w_in_flistp, PIN_FLD_POID, device_pidp, ebufp);
        if (!strcmp(device_type,MSO_OBJ_TYPE_DEVICE_STB) )
        {
            sub_flistp = PIN_FLIST_ELEM_ADD (w_in_flistp, MSO_FLD_STB_DATA, rec_id, ebufp);
            PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_POID, sub_flistp, MSO_FLD_VC_OBJ, ebufp);
        }
        else if(!strcmp(device_type,MSO_OBJ_TYPE_DEVICE_VC))
        {
            sub_flistp = PIN_FLIST_ELEM_ADD (w_in_flistp, MSO_FLD_VC_DATA, rec_id, ebufp);
            PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_POID, sub_flistp, MSO_FLD_STB_OBJ, ebufp);
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "fm_mso_device_set_pairing write_flds for current device  input flist", w_in_flistp);
         /*
          * Update the pairing detail for current device
          */
        PCM_OP(ctxp, PCM_OP_WRITE_FLDS, PCM_OPFLG_CACHEABLE, w_in_flistp,
            &w_o_flistp, ebufp);

        PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_POID, w_in_flistp, PIN_FLD_POID, ebufp);
    }

     PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);
//     PIN_FLIST_DESTROY_EX(&search_o_flistp, NULL);
     PIN_FLIST_DESTROY_EX(&w_in_flistp, NULL);
     PIN_FLIST_DESTROY_EX(&w_o_flistp, NULL);

return;
}

/*******************************************************************
 * Function used to remove pairing of the device
 *******************************************************************/
static void
    fm_mso_device_reset_pairing(
    pcm_context_t       *ctxp,
    pin_flist_t         *i_flistp,
    pin_errbuf_t         *ebufp)
{

    pin_flist_t         *search_i_flistp = NULL;
    pin_flist_t         *w_in_flistp = NULL;
    pin_flist_t         *read_iflistp = NULL;
    pin_flist_t         *read_oflistp = NULL;
    pin_flist_t         *write_iflisp = NULL;
    pin_flist_t         *write_oflisp = NULL;
    pin_flist_t         *vc_data = NULL;
    pin_flist_t         *w_o_flistp = NULL;
    pin_flist_t         *r_flistp = NULL;
    pin_flist_t         *tmp_flistp = NULL;
    pin_flist_t         *sub_flistp = NULL;
    pin_flist_t         *rec_id_flistp = NULL;
    pin_status_t        *service_statep = NULL;
    poid_t              *service_pdp = NULL;      /* service poid */
    poid_t              *device_pdp = NULL;
    poid_t              *device_pidp = NULL;
    poid_t              *s_pdp = NULL;
    int32               s_flags;
    time_t              now;
    int64               database;
    char                *template = NULL;
    char                *device_type = NULL;
    int32               count = 0;
    int32   		rec_id = 0;
    pin_cookie_t        rec_id_cookie = NULL;

    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }
    
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Reset pairing input flist is ", i_flistp);
    device_pidp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
    device_type = (char *) PIN_POID_GET_TYPE (device_pidp);
    tmp_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY, 0, ebufp);
    device_pidp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
    device_type = (char *) PIN_POID_GET_TYPE (device_pidp);

    service_pdp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
    database = (int64) PIN_POID_GET_DB(service_pdp);

    if (!strcmp(device_type,MSO_OBJ_TYPE_DEVICE_STB) )
    {
          device_pdp = (poid_t *)PIN_POID_CREATE(database, "/device/vc", -1, ebufp);
    }
    else if(!strcmp(device_type,MSO_OBJ_TYPE_DEVICE_VC))
    {
          device_pdp = (poid_t *)PIN_POID_CREATE(database, "/device/stb", -1, ebufp);
    }

    search_i_flistp = PIN_FLIST_CREATE(ebufp);

    s_pdp = (poid_t *)PIN_POID_CREATE(database, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(search_i_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

    s_flags = SRCH_DISTINCT ;
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

    template = "select X from /device where F1 = V1 and F2.type = V2 ";
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
    sub_flistp = PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY, ebufp);
    PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_SERVICE_OBJ, (void *)service_pdp, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
    PIN_FLIST_FLD_PUT(tmp_flistp, PIN_FLD_POID,(void *)device_pdp, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_POID, NULL, ebufp);

        if (!strcmp(device_type,MSO_OBJ_TYPE_DEVICE_STB) )
        {
            sub_flistp = PIN_FLIST_ELEM_ADD (tmp_flistp, MSO_FLD_VC_DATA, PIN_ELEMID_ANY, ebufp);
            PIN_FLIST_FLD_SET (sub_flistp, MSO_FLD_STB_OBJ, NULL, ebufp);
        }
        else if(!strcmp(device_type,MSO_OBJ_TYPE_DEVICE_VC))
        {
            sub_flistp = PIN_FLIST_ELEM_ADD (tmp_flistp, MSO_FLD_STB_DATA, PIN_ELEMID_ANY, ebufp);
            PIN_FLIST_FLD_SET (sub_flistp, MSO_FLD_VC_OBJ, NULL, ebufp);
        }

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_device_reset_pairing search input flist ", search_i_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_i_flistp,&r_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_device_reset_pairing search output flist", r_flistp);
    
    count = PIN_FLIST_ELEM_COUNT(r_flistp, PIN_FLD_RESULTS, ebufp);

    if (count == 1)
    {
          /*
          * Update the pairing detail for
          * existing device as well as current device
          */
        tmp_flistp = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
	
	if (!strcmp(device_type,MSO_OBJ_TYPE_DEVICE_STB) )
        {
		rec_id = 0;
		rec_id_cookie = NULL;
	 	while((rec_id_flistp = (pin_flist_t *)PIN_FLIST_ELEM_GET_NEXT(tmp_flistp, MSO_FLD_VC_DATA,
        		&rec_id, 1, &rec_id_cookie, ebufp)) != NULL)
    		{
		// This while loop is to get the appropriate recid 	
		}
        	w_in_flistp = PIN_FLIST_CREATE(ebufp);
        	PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_POID, w_in_flistp, PIN_FLD_POID, ebufp);
		sub_flistp = PIN_FLIST_ELEM_ADD (w_in_flistp, MSO_FLD_VC_DATA, rec_id, ebufp);
            	PIN_FLIST_FLD_SET(sub_flistp, MSO_FLD_STB_OBJ, device_pidp, ebufp);
		

	}
        else if(!strcmp(device_type,MSO_OBJ_TYPE_DEVICE_VC))
        {
		rec_id = 0;
                rec_id_cookie = NULL;
                while((rec_id_flistp = (pin_flist_t *)PIN_FLIST_ELEM_GET_NEXT(tmp_flistp, MSO_FLD_STB_DATA,
                        &rec_id, 1, &rec_id_cookie, ebufp)) != NULL)
                {
		// This while loop is to get the appropriate recid
        	}
        	w_in_flistp = PIN_FLIST_CREATE(ebufp);
        	PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_POID, w_in_flistp, PIN_FLD_POID, ebufp);
            	sub_flistp = PIN_FLIST_ELEM_ADD (w_in_flistp, MSO_FLD_STB_DATA, rec_id, ebufp);
            	PIN_FLIST_FLD_SET(sub_flistp, MSO_FLD_VC_OBJ, device_pidp, ebufp);
   	}
    
	 /*
	* Reset the pairing detail for current device
	*/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_device_reset_pairing write_flds for current device  input flist", w_in_flistp);
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, PCM_OPFLG_CACHEABLE, w_in_flistp,&w_o_flistp, ebufp);
    }
	
    /***** Read the device poid ******/

	read_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_iflistp , PIN_FLD_POID , device_pidp , ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Read device poid flist is " , read_iflistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ , 0 , read_iflistp , &read_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " read device output flist is " , read_oflistp);
	
	if(!strcmp(device_type,MSO_OBJ_TYPE_DEVICE_VC))
	{
		if(PIN_FLIST_ELEM_COUNT(read_oflistp , MSO_FLD_VC_DATA, ebufp) > 0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " VC_DATA is present ");
			vc_data = PIN_FLIST_ELEM_GET(read_oflistp , MSO_FLD_VC_DATA , PIN_ELEMID_ANY , 1 , ebufp);
			rec_id = 0;
	                rec_id_cookie = NULL;
        	        while((rec_id_flistp = (pin_flist_t *)PIN_FLIST_ELEM_GET_NEXT(read_oflistp, MSO_FLD_VC_DATA,
                	        &rec_id, 1, &rec_id_cookie, ebufp)) != NULL)
                	{
                		// This while loop is to get the appropriate recid
                	}
			write_iflisp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(write_iflisp , PIN_FLD_POID , device_pidp , ebufp);
			PIN_FLIST_ELEM_SET(write_iflisp,vc_data, MSO_FLD_VC_DATA,rec_id, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG ," write flds input is " , write_iflisp);
			PCM_OP(ctxp, PCM_OP_DELETE_FLDS, 0, write_iflisp , &write_oflisp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG ," write flds output is ", write_oflisp);	

		}
	}
	else if(!strcmp(device_type,MSO_OBJ_TYPE_DEVICE_STB))
	{
		if(PIN_FLIST_ELEM_COUNT(read_oflistp , MSO_FLD_STB_DATA, ebufp) > 0)
                {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " STB DATA present");
                        vc_data = PIN_FLIST_ELEM_GET(read_oflistp , MSO_FLD_STB_DATA , PIN_ELEMID_ANY , 1 , ebufp);
			rec_id = 0;
                        rec_id_cookie = NULL;
                        while((rec_id_flistp = (pin_flist_t *)PIN_FLIST_ELEM_GET_NEXT(read_oflistp, MSO_FLD_STB_DATA,
                                &rec_id, 1, &rec_id_cookie, ebufp)) != NULL)
                        {
                                // This while loop is to get the appropriate recid
                        }

                        write_iflisp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(write_iflisp , PIN_FLD_POID , device_pidp , ebufp);
                        PIN_FLIST_ELEM_SET(write_iflisp,vc_data, MSO_FLD_STB_DATA, rec_id, ebufp);
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG ," write flds input is " , write_iflisp);
                        PCM_OP(ctxp, PCM_OP_DELETE_FLDS, 0, write_iflisp , &write_oflisp, ebufp);
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG ," write flds output is ", write_oflisp);
                }

	}

    PIN_FLIST_DESTROY_EX(&w_in_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&w_o_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
    PIN_FLIST_DESTROY_EX(&read_iflistp, NULL);
    PIN_FLIST_DESTROY_EX(&write_iflisp, NULL);
    PIN_FLIST_DESTROY_EX(&write_oflisp, NULL);

return;
}
