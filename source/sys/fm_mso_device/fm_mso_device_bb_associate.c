/*
 * (#)$Id: fm_mso_device_bb_associate.c st_cgbubrm_gsivakum_bug-8453030/1 2009/05/05 23:37:48 gsivakum Exp $
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
static const char Sccs_id[] = "(#)$Id: fm_mso_device_bb_associate.c st_cgbubrm_gsivakum_bug-8453030/1 2009/05/05 23:37:48 gsivakum Exp $";
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

#define FILE_LOGNAME "fm_mso_device_bb_associate.c"

/*******************************************************************************
 * Functions contained within this source file
 ******************************************************************************/
EXPORT_OP void
op_mso_device_bb_associate(
    cm_nap_connection_t    *connp,
    int32            opcode,
    int32            flags,
    pin_flist_t        *i_flistp,
    pin_flist_t        **o_flistpp,
    pin_errbuf_t        *ebufp);

static void
fm_mso_device_bb_associate(
    pcm_context_t      *ctxp,
    cm_op_info_t       *opstackp,
    pin_flist_t        *i_flistp,
    pin_flist_t        *rfo_flistp,
    pin_errbuf_t        *ebufp);

static void
fm_mso_device_bb_disassociate(
    pcm_context_t        *ctxp,
	cm_op_info_t       *opstackp,
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
    cm_op_info_t       *opstackp,
    pin_flist_t         *i_flistp,
    int32               *warranty_offset,
    pin_flist_t         *paired_flistp,
    pin_errbuf_t         *ebufp);

void
fm_mso_device_set_category(
    pcm_context_t       *ctxp,
    pin_flist_t         *i_flistp,
	char				*prog_name,
    pin_errbuf_t        *ebufp);


/*******************************************************************************

 ******************************************************************************/
void
op_mso_device_bb_associate(
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
    cm_op_info_t            *opstackp = connp->coip;


    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR(ebufp);

    /*
     * Insanity check.
     */
    if (opcode != MSO_OP_DEVICE_BB_ASSOCIATE) {

        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
            PIN_ERR_BAD_OPCODE, 0, 0, opcode);

        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "bad opcode in op_num_pol_device_associate", ebufp);

        *o_flistpp = NULL;
        return;
    }


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "op_mso_device_bb_associate input flist", i_flistp);

    /*
     * Validate the input flist has the correct poid type.
     */
    i_poidp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
    poid_type = (poid_t *)PIN_POID_GET_TYPE (i_poidp);

    if ( strcmp(poid_type, MSO_OBJ_TYPE_DEVICE_MODEM)    != 0 &&
             strcmp(poid_type, MSO_OBJ_TYPE_DEVICE_ROUTER_CABLE )    != 0 && 
             strcmp(poid_type, MSO_OBJ_TYPE_DEVICE_ROUTER_WIFI )    != 0 && 
             strcmp(poid_type, MSO_OBJ_TYPE_DEVICE_NAT )    != 0 && 
             strcmp(poid_type, MSO_OBJ_TYPE_DEVICE_IP_STATIC )    != 0 && 
             strcmp(poid_type, MSO_OBJ_TYPE_DEVICE_IP_FRAMED )    != 0 
             )
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
            PIN_ERR_BAD_POID_TYPE, PIN_FLD_POID, 0, 0);

        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_mso_device_bb_associate error", ebufp);

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
            "op_mso_device_bb_associate error", ebufp);

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

     // for provisionable flag setting
    PIN_FLIST_FLD_SET(rfi_flistp, MSO_FLD_PROVISIONABLE_FLAG, NULL, ebufp);


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
        fm_mso_device_bb_associate(ctxp, opstackp, i_flistp, rfo_flistp, ebufp);
    }
    else
    {
        fm_mso_device_bb_disassociate(ctxp, opstackp, i_flistp,
            rfo_flistp, ebufp);
    }

    if (PIN_ERRBUF_IS_ERR(ebufp)) {

        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_mso_device_bb_associate error", ebufp);
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
 * fm_mso_device_bb_associate()
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
fm_mso_device_bb_associate(
    pcm_context_t      *ctxp,
    cm_op_info_t       *opstackp,
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
    int32               *prov_flag = NULL;
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
                    "fm_mso_device_bb_associate duplicate "
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
                    "fm_mso_device_bb_associate duplicate "
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
                "fm_mso_device_bb_associate device has "
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

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"DEBUG input flist", rfo_flistp);


    /* get the PROVISIONABLE_FLAG */
    prov_flag = (int32 *)PIN_FLIST_FLD_GET(rfo_flistp, MSO_FLD_PROVISIONABLE_FLAG,
                   0, ebufp);

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
     * Set the provisionable flag as 0 if not specified
     */
    if(!prov_flag)
    {
        *prov_flag = 0;
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
    if (*prov_flag == 1) 
    {
    	fm_mso_device_alias_to_services(ctxp, i_flistp, prov_attr, ne,
                        PIN_BOOLEAN_TRUE, ebufp);
    }

    /*
     * Set the warranty end date of the device 
     */
    fm_mso_device_set_warranty(ctxp, opstackp, i_flistp, warranty_offset, r_flistp, ebufp);

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

    if ( strcmp(PIN_POID_GET_TYPE(i_poidp), MSO_OBJ_TYPE_DEVICE_MODEM) == 0 )
    {
        switch (*db_state_id)
        {

        case MSO_MODEM_STATE_GOOD:
            dssi_flistp = PIN_FLIST_CREATE(ebufp);

            vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
            PIN_FLIST_FLD_SET(dssi_flistp, PIN_FLD_POID, vp, ebufp);

            next_state = MSO_MODEM_STATE_ALLOCATED;

            PIN_FLIST_FLD_SET(dssi_flistp, PIN_FLD_NEW_STATE,
                (void *)&next_state, ebufp);

            PCM_OP(ctxp, PCM_OP_DEVICE_SET_STATE, 0,
                dssi_flistp, &dsso_flistp, ebufp);
	    
            PIN_FLIST_DESTROY_EX(&dssi_flistp, NULL);
            PIN_FLIST_DESTROY_EX(&dsso_flistp, NULL);
            break;
			
		/*Repaired device Change*/
			case MSO_MODEM_STATE_REPAIRED:
            dssi_flistp = PIN_FLIST_CREATE(ebufp);

            vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
            PIN_FLIST_FLD_SET(dssi_flistp, PIN_FLD_POID, vp, ebufp);

            next_state = MSO_MODEM_STATE_ALLOCATED;

            PIN_FLIST_FLD_SET(dssi_flistp, PIN_FLD_NEW_STATE,
                (void *)&next_state, ebufp);

            PCM_OP(ctxp, PCM_OP_DEVICE_SET_STATE, 0,
                dssi_flistp, &dsso_flistp, ebufp);

            PIN_FLIST_DESTROY_EX(&dssi_flistp, NULL);
            PIN_FLIST_DESTROY_EX(&dsso_flistp, NULL);
            break;
        case MSO_MODEM_STATE_ALLOCATED:
        /*
         * The device is already in Assigned state; do nothing
         */
            break;
        default:
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_BAD_VALUE,
                PIN_FLD_STATE_ID, 0, *db_state_id);

            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                "fm_mso_device_bb_associate unknown state_id "
                "read for this device",    ebufp);
        }
    }
    else if ( strcmp(PIN_POID_GET_TYPE(i_poidp), MSO_OBJ_TYPE_DEVICE_ROUTER_CABLE) == 0  || 
    	strcmp(PIN_POID_GET_TYPE(i_poidp), MSO_OBJ_TYPE_DEVICE_ROUTER_WIFI) == 0
    	)
    {
        switch (*db_state_id)
        {

        case MSO_ROUTER_STATE_GOOD:
            dssi_flistp = PIN_FLIST_CREATE(ebufp);

            vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
            PIN_FLIST_FLD_SET(dssi_flistp, PIN_FLD_POID, vp, ebufp);

            next_state = MSO_ROUTER_STATE_ALLOCATED;

            PIN_FLIST_FLD_SET(dssi_flistp, PIN_FLD_NEW_STATE,
                (void *)&next_state, ebufp);

            PCM_OP(ctxp, PCM_OP_DEVICE_SET_STATE, 0,
                dssi_flistp, &dsso_flistp, ebufp);

            PIN_FLIST_DESTROY_EX(&dssi_flistp, NULL);
            PIN_FLIST_DESTROY_EX(&dsso_flistp, NULL);
            break;
	/*Repaired device Change*/
	case MSO_ROUTER_STATE_REPAIRED:
            dssi_flistp = PIN_FLIST_CREATE(ebufp);

            vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
            PIN_FLIST_FLD_SET(dssi_flistp, PIN_FLD_POID, vp, ebufp);

            next_state = MSO_ROUTER_STATE_ALLOCATED;

            PIN_FLIST_FLD_SET(dssi_flistp, PIN_FLD_NEW_STATE,
                (void *)&next_state, ebufp);

            PCM_OP(ctxp, PCM_OP_DEVICE_SET_STATE, 0,
                dssi_flistp, &dsso_flistp, ebufp);

            PIN_FLIST_DESTROY_EX(&dssi_flistp, NULL);
            PIN_FLIST_DESTROY_EX(&dsso_flistp, NULL);
            break;
			/*Changes Completed*/
        case MSO_ROUTER_STATE_ALLOCATED:
        /*
         * The device is already in Assigned state; do nothing
         */
            break;
        default:
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_BAD_VALUE,
                PIN_FLD_STATE_ID, 0, *db_state_id);

            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                "fm_mso_device_bb_associate unknown state_id "
                "read for this device",    ebufp);
        }
    }
    else if ( strcmp(PIN_POID_GET_TYPE(i_poidp), MSO_OBJ_TYPE_DEVICE_NAT) == 0 )
    {
        switch (*db_state_id)
        {

        case MSO_NAT_STATE_GOOD:
            dssi_flistp = PIN_FLIST_CREATE(ebufp);

            vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
            PIN_FLIST_FLD_SET(dssi_flistp, PIN_FLD_POID, vp, ebufp);

            next_state = MSO_NAT_STATE_ALLOCATED;

            PIN_FLIST_FLD_SET(dssi_flistp, PIN_FLD_NEW_STATE,
                (void *)&next_state, ebufp);

            PCM_OP(ctxp, PCM_OP_DEVICE_SET_STATE, 0,
                dssi_flistp, &dsso_flistp, ebufp);

            PIN_FLIST_DESTROY_EX(&dssi_flistp, NULL);
            PIN_FLIST_DESTROY_EX(&dsso_flistp, NULL);
            break;
		/*Repaired device Change*/
			case MSO_NAT_STATE_REPAIRED:
            dssi_flistp = PIN_FLIST_CREATE(ebufp);

            vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
            PIN_FLIST_FLD_SET(dssi_flistp, PIN_FLD_POID, vp, ebufp);

            next_state = MSO_NAT_STATE_ALLOCATED;

            PIN_FLIST_FLD_SET(dssi_flistp, PIN_FLD_NEW_STATE,
                (void *)&next_state, ebufp);

            PCM_OP(ctxp, PCM_OP_DEVICE_SET_STATE, 0,
                dssi_flistp, &dsso_flistp, ebufp);

            PIN_FLIST_DESTROY_EX(&dssi_flistp, NULL);
            PIN_FLIST_DESTROY_EX(&dsso_flistp, NULL);
            break;
			/*Changes Completed*/
        case MSO_NAT_STATE_ALLOCATED:
        /*
         * The device is already in Assigned state; do nothing
         */
            break;
        default:
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_BAD_VALUE,
                PIN_FLD_STATE_ID, 0, *db_state_id);

            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                "fm_mso_device_bb_associate unknown state_id "
                "read for this device",    ebufp);
        }
    }
    else if ( strcmp(PIN_POID_GET_TYPE(i_poidp), MSO_OBJ_TYPE_DEVICE_IP_STATIC) == 0 || 
    	strcmp(PIN_POID_GET_TYPE(i_poidp), MSO_OBJ_TYPE_DEVICE_IP_FRAMED) == 0
    	)
    {
        switch (*db_state_id)
        {

        case MSO_IP_STATE_GOOD:
            dssi_flistp = PIN_FLIST_CREATE(ebufp);

            vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
            PIN_FLIST_FLD_SET(dssi_flistp, PIN_FLD_POID, vp, ebufp);

            next_state = MSO_IP_STATE_ALLOCATED;

            PIN_FLIST_FLD_SET(dssi_flistp, PIN_FLD_NEW_STATE,
                (void *)&next_state, ebufp);

            PCM_OP(ctxp, PCM_OP_DEVICE_SET_STATE, 0,
                dssi_flistp, &dsso_flistp, ebufp);

            PIN_FLIST_DESTROY_EX(&dssi_flistp, NULL);
            PIN_FLIST_DESTROY_EX(&dsso_flistp, NULL);
            break;
				/*Repaired device Change*/
			case MSO_IP_STATE_REPAIRED:
            dssi_flistp = PIN_FLIST_CREATE(ebufp);

            vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
            PIN_FLIST_FLD_SET(dssi_flistp, PIN_FLD_POID, vp, ebufp);

            next_state = MSO_IP_STATE_ALLOCATED;

            PIN_FLIST_FLD_SET(dssi_flistp, PIN_FLD_NEW_STATE,
                (void *)&next_state, ebufp);

            PCM_OP(ctxp, PCM_OP_DEVICE_SET_STATE, 0,
                dssi_flistp, &dsso_flistp, ebufp);

            PIN_FLIST_DESTROY_EX(&dssi_flistp, NULL);
            PIN_FLIST_DESTROY_EX(&dsso_flistp, NULL);
            break;
			/*Changes Completed*/	
			
        case MSO_IP_STATE_ALLOCATED:
        /*
         * The device is already in Assigned state; do nothing
         */
            break;
        default:
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_BAD_VALUE,
                PIN_FLD_STATE_ID, 0, *db_state_id);

            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                "fm_mso_device_bb_associate unknown state_id "
                "read for this device",    ebufp);
        }
    }

    return;
}


/*******************************************************************************
 * fm_mso_device_bb_disassociate()
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
fm_mso_device_bb_disassociate(
    pcm_context_t        *ctxp,
	cm_op_info_t       *opstackp,
    pin_flist_t        *i_flistp,
    pin_flist_t        *rfo_flistp,
    pin_errbuf_t        *ebufp)
{
    pin_flist_t      *dssi_flistp = NULL;
    pin_flist_t      *dsso_flistp = NULL;
    pin_flist_t      *device_flistp = NULL;
    pin_flist_t      *device_oflistp = NULL;
    pin_flist_t      *del_flistp = NULL;
    pin_flist_t      *del_out_flistp = NULL;
    pin_flist_t      *plan_flistp = NULL;
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
	int32            *prov_flag = NULL;
	int32			 swap_device = 0;
	int32			 stack_opcode = 0;

    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_device_bb_disassociate input flist ", i_flistp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_device_bb_disassociate rfo_flistp flist ", rfo_flistp);

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

    /* get the PROVISIONABLE_FLAG */
    prov_flag = (int32 *)PIN_FLIST_FLD_GET(rfo_flistp, MSO_FLD_PROVISIONABLE_FLAG,
                   0, ebufp);

	
    if (PIN_ERRBUF_IS_ERR(ebufp)) {
         return;
    }
	/*Pawan:23-02-2015: Added the the below condition to call
		fm_mso_device_alias_to_services function only when prov_flag is 1*/
		
    if (prov_flag && *prov_flag == 1) 
    {
		/* PIN_BOOLEAN_FALSE for is_assct indicates this is for disassociation */
		fm_mso_device_alias_to_services(ctxp, i_flistp, prov_attr, ne, PIN_BOOLEAN_FALSE, ebufp);
	}

    //fm_mso_device_reset_pairing(ctxp, i_flistp, ebufp);

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
		 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Not all of the services are being disassociated; do nothing");

    } else if (i_count == db_count) {

        /*
         * If the state of the device is Assigned, set the state
         * to Quarantined.  Else the current state does not support
         * service associations; report an error.
         */
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Disassociating the device..");
        db_state_id = (int32 *)PIN_FLIST_FLD_GET(rfo_flistp,
            PIN_FLD_STATE_ID, 0, ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
            return;
        }

        if ( strcmp(PIN_POID_GET_TYPE(i_poidp), MSO_OBJ_TYPE_DEVICE_MODEM) == 0 )
        {
            switch (*db_state_id) {

            case MSO_MODEM_STATE_ALLOCATED:

                dssi_flistp = PIN_FLIST_CREATE(ebufp);

                vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID,
                    0, ebufp);
                PIN_FLIST_FLD_SET(dssi_flistp, PIN_FLD_POID, vp, ebufp);

                if ( *flags == MSO_DEVICE_DEACTIVATION_REASON_FAULTY)
                {
                    next_state = MSO_MODEM_STATE_FAULTY;
                }
                // Complete deactivation of services & disassociaton of temporary devices
                else
                {
                    next_state = MSO_MODEM_STATE_GOOD;
                }
				
				// Changes for Repaired Device
				
				if ( *flags == MSO_DEVICE_ACTIVATION_REASON_REPAIRED)
                {
                    next_state = MSO_MODEM_STATE_REPAIRED;
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
                    "fm_mso_device_bb_disassociate invalid "
                    "state_id for disassociation of services",
                    ebufp);

            }
        }
        else if ( strcmp(PIN_POID_GET_TYPE(i_poidp), MSO_OBJ_TYPE_DEVICE_ROUTER_CABLE) == 0 || 
        	strcmp(PIN_POID_GET_TYPE(i_poidp), MSO_OBJ_TYPE_DEVICE_ROUTER_WIFI) == 0 
        	)
        {
            switch (*db_state_id) {

            case MSO_ROUTER_STATE_ALLOCATED:

                dssi_flistp = PIN_FLIST_CREATE(ebufp);

                vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID,
                    0, ebufp);
                PIN_FLIST_FLD_SET(dssi_flistp, PIN_FLD_POID, vp, ebufp);

                if ( *flags == MSO_DEVICE_DEACTIVATION_REASON_FAULTY)
                {
                    next_state = MSO_ROUTER_STATE_FAULTY;
                }
                // Complete deactivation of services & disassociaton of temporary devices
                else
                {
                    next_state = MSO_ROUTER_STATE_GOOD;
                }
				
				// Changes for Repaired Device
				
				if ( *flags == MSO_DEVICE_ACTIVATION_REASON_REPAIRED)
                {
                    next_state = MSO_ROUTER_STATE_REPAIRED;
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
                    "fm_mso_device_bb_disassociate invalid "
                    "state_id for disassociation of services",
                    ebufp);

            }
        }
        else if ( strcmp(PIN_POID_GET_TYPE(i_poidp), MSO_OBJ_TYPE_DEVICE_NAT) == 0 )
        {
            switch (*db_state_id) {

            case MSO_NAT_STATE_ALLOCATED:

                dssi_flistp = PIN_FLIST_CREATE(ebufp);

                vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID,
                    0, ebufp);
                PIN_FLIST_FLD_SET(dssi_flistp, PIN_FLD_POID, vp, ebufp);

                if ( *flags == MSO_DEVICE_DEACTIVATION_REASON_FAULTY)
                {
                    next_state = MSO_NAT_STATE_FAULTY;
                }
                // Complete deactivation of services & disassociaton of temporary devices
                else
                {
                    next_state = MSO_NAT_STATE_GOOD;
                }
				
				// Changes for Repaired Device
				
				if ( *flags == MSO_DEVICE_ACTIVATION_REASON_REPAIRED)
                {
                    next_state = MSO_NAT_STATE_REPAIRED;
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
                    "fm_mso_device_bb_disassociate invalid "
                    "state_id for disassociation of services",
                    ebufp);

            }
        }
        else if ( strcmp(PIN_POID_GET_TYPE(i_poidp), MSO_OBJ_TYPE_DEVICE_IP_STATIC) == 0 || 
        	strcmp(PIN_POID_GET_TYPE(i_poidp), MSO_OBJ_TYPE_DEVICE_IP_FRAMED) == 0 
        	)
        {
            switch (*db_state_id) {

            case MSO_IP_STATE_ALLOCATED:

                dssi_flistp = PIN_FLIST_CREATE(ebufp);

                vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID,
                    0, ebufp);
                PIN_FLIST_FLD_SET(dssi_flistp, PIN_FLD_POID, vp, ebufp);
				
				// Changes for Repaired Device
				
				if ( *flags == MSO_DEVICE_ACTIVATION_REASON_REPAIRED)
                {
                    next_state = MSO_IP_STATE_REPAIRED;
                }
                else
                {
                    next_state = MSO_IP_STATE_GOOD;
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
                    "fm_mso_device_bb_disassociate invalid "
                    "state_id for disassociation of services",
                    ebufp);

            }
        }
		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Searching the ancestor opcodes to determine if the plan needs to be removed from device..");
		while(opstackp != NULL) {
			stack_opcode = opstackp->opcode;
			 if(stack_opcode == MSO_OP_CUST_SWAP_BB_DEVICE) {
				 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," Called from MSO_OP_CUST_SWAP_BB_DEVICE");
				swap_device = 1;
				break;
			 }
			opstackp = opstackp->next; 
		}

		if(!swap_device) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," Not a swap device call.. Going ahead to remove the plan from device");
			device_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, device_flistp, PIN_FLD_POID, ebufp);
			plan_flistp = PIN_FLIST_ELEM_ADD(device_flistp, PIN_FLD_PLAN, 0, ebufp);
			PIN_FLIST_FLD_SET(plan_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, NULL, ebufp);
			PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_TYPE, NULL, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Delete plans input flist ", device_flistp);
			PCM_OP(ctxp, PCM_OP_DELETE_FLDS, 0, device_flistp, &device_oflistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Delete plans output flist", device_oflistp);
			PIN_FLIST_DESTROY_EX(&device_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&device_oflistp, NULL);
			
		}			
    } else {  /* (i_count > db_count) */

        pin_set_err(ebufp, PIN_ERRLOC_APP, PIN_ERRCLASS_APPLICATION,
            PIN_ERR_BAD_VALUE, PIN_FLD_SERVICES, 0, i_count);

        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "fm_mso_device_bb_disassociate more input services "
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
    int32                   elem_id5 = 1;    /* array element        */
    pin_cookie_t            cookie = NULL;  /* to loop through array */
    pin_cookie_t            cookie5 = NULL;  /* to loop through array */
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

        //PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_PROGRAM_NAME,s_flistp, PIN_FLD_PROGRAM_NAME,ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_PROGRAM_NAME, FILE_LOGNAME,
                      ebufp);

        /*PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_PROGRAM_NAME, FILE_LOGNAME,
                      ebufp);
	Fixed done to set correct program name passed from originator
	*/

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
         *      0 for user id
         *      1 for mac Address
         *      2 for static ip
         *      3 for framed ip
         */
/*        if (!strcmp(device_type,MSO_OBJ_TYPE_DEVICE_MODEM) || 
             !strcmp(device_type,MSO_OBJ_TYPE_DEVICE_ROUTER_CABLE) || 
             !strcmp(device_type,MSO_OBJ_TYPE_DEVICE_ROUTER_WIFI) || 
             !strcmp(device_type,MSO_OBJ_TYPE_DEVICE_NAT) 
            )
        {
              elem_id1 = 1;
        }
        else if (!strcmp(device_type,MSO_OBJ_TYPE_DEVICE_IP_STATIC))
        {
              elem_id1 = 2;
        }
        else if(!strcmp(device_type,MSO_OBJ_TYPE_DEVICE_IP_FRAMED))
        {
              elem_id1 = 3;
        } */

        if (is_assct == PIN_BOOLEAN_TRUE)  /* Caller is pol_device_associate  */
        {
		elem_id1 = 0;
           	elem_id1 = PIN_FLIST_ELEM_COUNT(alias_flistp,PIN_FLD_ALIAS_LIST, ebufp); 
		tmp_flistp = PIN_FLIST_ELEM_ADD(log_flistp, PIN_FLD_ALIAS_LIST,
                               elem_id1, ebufp);
            PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_NAME, alias, ebufp);
        }
        else  /* Caller is pol_device_disassociate */
        {
            tmp_flistp = PIN_FLIST_ELEM_GET(alias_flistp, PIN_FLD_ALIAS_LIST, elem_id1, 0, ebufp);
            
    	    while( (tmp_flistp = (pin_flist_t *)
        	    PIN_FLIST_ELEM_GET_NEXT(alias_flistp, PIN_FLD_ALIAS_LIST,
        	&elem_id5, 1, &cookie5, ebufp)) != NULL)
	    {	
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
				  PIN_FLD_ALIAS_LIST, elem_id5, ebufp);
		    }	
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
    cm_op_info_t       *opstackp,
    pin_flist_t         *i_flistp,
    int32               *warranty_offset,
    pin_flist_t         *paired_flistp,
    pin_errbuf_t        *ebufp)
{
    pin_flist_t         *w_in_flistp = NULL;
    pin_flist_t         *w_o_flistp = NULL;
    pin_flist_t         *read_i_flistp = NULL;
    pin_flist_t         *read_o_flistp = NULL;
    pin_flist_t         *tmp_flistp = NULL;
    pin_flist_t         *sub_flistp = NULL;
    pin_flist_t         *device_flistp = NULL;
    pin_flist_t         *dev_read_flistp = NULL;
    pin_flist_t         *dev_read_oflistp = NULL;
    pin_flist_t         *stack_flistp = NULL;
    pin_flist_t         *args_flistp = NULL;
    pin_flist_t         *result_flistp = NULL;
    poid_t              *prev_device_pdp = NULL;
    poid_t              *device_pidp = NULL;
    poid_t              *srch_pdp = NULL;
    int32               *prev_warranty = NULL;
    char                *device_type = NULL;
    time_t              now;
    time_t              warranty_end;
    struct              tm *current_time;
    int32               *flags = NULL ;
    int32               count = 0;
	int32				stack_opcode = 0;
	int32				swap_device = 0;
	int32				elem_id = 0;
	int32				*flag = NULL;
	pin_cookie_t		cookie = NULL;
	char				*old_device_id = NULL;
	char				*template = "select X from /device where F1 = V1 ";
	int64				db = 1;
	int32				srch_flags = 0;




    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"fm_mso_device_set_warranty input flist", i_flistp);

    device_pidp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
    device_type = (char *) PIN_POID_GET_TYPE (device_pidp);
    flags = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 0 , ebufp);

    if ( *flags == MSO_DEVICE_ACTIVATION_NEW)
    {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Iterating over the opstack for SWAP DEVICE");
		while(opstackp != NULL) {
			stack_opcode = opstackp->opcode;
            stack_flistp = opstackp->in_flistp;

			 if(stack_opcode == MSO_OP_CUST_SWAP_BB_DEVICE) {
				 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," Called from MSO_OP_CUST_SWAP_BB_DEVICE");
				swap_device = 1;
				while ((device_flistp = PIN_FLIST_ELEM_GET_NEXT(stack_flistp, PIN_FLD_DEVICES, &elem_id, 1, &cookie, ebufp )) != NULL)
		        {
					 flag = PIN_FLIST_FLD_GET(device_flistp, PIN_FLD_FLAGS, 1, ebufp );

					if (flag && *(int*)flag == 0) {
						old_device_id = PIN_FLIST_FLD_GET(device_flistp, PIN_FLD_DEVICE_ID, 1, ebufp );
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," Old device is ");
						 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,old_device_id);

						break;
					}					
				}
				break;
			 }
			opstackp = opstackp->next; 
		}
		if(swap_device) {
			if(old_device_id != NULL) {
				 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," Reading warranty for old device to move to the new one");
				 srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
				dev_read_flistp = PIN_FLIST_CREATE(ebufp);

				PIN_FLIST_FLD_SET(dev_read_flistp, PIN_FLD_POID, srch_pdp, ebufp);
				PIN_FLIST_FLD_SET(dev_read_flistp, PIN_FLD_FLAGS, &srch_flags, ebufp);
				PIN_FLIST_FLD_SET(dev_read_flistp, PIN_FLD_TEMPLATE, template, ebufp);
				args_flistp = PIN_FLIST_ELEM_ADD(dev_read_flistp, PIN_FLD_ARGS, 1, ebufp);
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DEVICE_ID, old_device_id, ebufp);
				result_flistp = PIN_FLIST_ELEM_ADD(dev_read_flistp, PIN_FLD_RESULTS, 0, ebufp);
				PIN_FLIST_FLD_SET(result_flistp, MSO_FLD_WARRANTY_END, NULL, ebufp);
	
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"fm_mso_device_set_warranty search prev device input flist", dev_read_flistp);

				PCM_OP(ctxp, PCM_OP_SEARCH, 0, dev_read_flistp,
						&dev_read_oflistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"fm_mso_device_set_warranty search prev device output flist", dev_read_oflistp);

				result_flistp = PIN_FLIST_ELEM_GET(dev_read_oflistp, PIN_FLD_RESULTS, 0, 0, ebufp);
				prev_warranty = PIN_FLIST_FLD_GET(result_flistp, MSO_FLD_WARRANTY_END, 1, ebufp);
				if(prev_warranty != NULL) {
					warranty_end = *prev_warranty;
				}
								 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Warranty derived");
			}

		} else {
			/*
			 * warranty end = current time + warranty offset in months
			 */
			now = pin_virtual_time((time_t *)NULL);
			current_time = localtime(&now);
			current_time->tm_year += *warranty_offset / 12;
			current_time->tm_mon += *warranty_offset % 12 ;
			warranty_end = mktime(current_time);
		}
    }
    else // DISASSOCIATION FLOW
    {        
		// Set the warranty end to now.
        warranty_end = pin_virtual_time((time_t *)NULL); 
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

/****************************************************************
 * function: fm_mso_device_set_category
 * 			This function is used to set the device category
 * 			to USED during device association.
 ****************************************************************/
void
fm_mso_device_set_category(
    pcm_context_t       *ctxp,
    pin_flist_t         *i_flistp,
	char				*prog_name,
    pin_errbuf_t        *ebufp)
{
    pin_flist_t         *w_in_flistp = NULL;
    pin_flist_t         *w_o_flistp = NULL;
	char				used[] = "USED";

    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }

    w_in_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, w_in_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(w_in_flistp, PIN_FLD_PROGRAM_NAME, prog_name, ebufp);
    PIN_FLIST_FLD_SET(w_in_flistp, PIN_FLD_CATEGORY, used, ebufp);
    
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "PCM_OP_DEVICE_SET_ATTR input flist", w_in_flistp);
    PCM_OP(ctxp, PCM_OP_DEVICE_SET_ATTR, 0, w_in_flistp,
        &w_o_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "PCM_OP_DEVICE_SET_ATTR output flist", w_o_flistp);
		
     PIN_FLIST_DESTROY_EX(&w_in_flistp, NULL);
     PIN_FLIST_DESTROY_EX(&w_o_flistp, NULL);

return;
}

