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
* 0.1    |16/04/2014 |Phani Jandhyala   |  This opcode will give the detailed information of all failed jobs 
*--------------------------------------------------------------------------------------------------------------*
*
****************************************************************************************************************/

#ifndef lint
static  char Sccs_Id[] = "@(#)$Id: fm_mso_integ_get_failed_job_details.c /cgbubrm_7.3.2.rwsmod/1 2009/08/04 23:37:19 pjegan Exp $";
#endif

#define _GNU_SOURCE 1

#define STB_DEVICE_LOAD "device/stb"
#define VC_DEVICE_LOAD "device/vc"
#define MODEM_DEVICE_LOAD "device/modem"
#define CABLE_ROUTER_DEVICE_LOAD "device/router/cable"
#define WIFI_ROUTER_DEVICE_LOAD "device/router/wifi"
#define NAT_DEVICE_LOAD "device/nat"
#define STATIC_IP_DEVICE_LOAD "device/ip/static"
#define FRAMED_IP_DEVICE_LOAD "device/ip/framed"

#define STB_LOCATION_UPDATE "location_update/stb"
#define VC_LOCATION_UPDATE "location_update/vc"
#define MODEM_LOCATION_UPDATE "location_update/modem"
#define CABLE_ROUTER_LOCATION_UPDATE "location_update/router/cable"
#define WIFI_ROUTER_LOCATION_UPDATE "location_update/router/wifi"
#define NAT_LOCATION_UPDATE "location_update/nat"
#define STATIC_IP_LOCATION_UPDATE "location_update/ip/static"
#define FRAMED_IP_LOCATION_UPDATE "location_update/ip/framed"

#define STB_STATE_CHANGE "state_change/stb"
#define VC_STATE_CHANGE "state_change/vc"
#define MODEM_STATE_CHANGE "state_change/modem"
#define CABLE_ROUTER_STATE_CHANGE "state_change/router/cable"
#define WIFI_ROUTER_STATE_CHANGE "state_change/router/wifi"
#define NAT_STATE_CHANGE "state_change/nat"
#define STATIC_IP_STATE_CHANGE "state_change/ip/static"
#define FRAMED_IP_STATE_CHANGE "state_change/ip/framed"

#define PREACTIVATION "catv_preactivation"
#define ADD_PLAN	"add_plan"
#define CHANGE_PLAN "change_plan"
#define CANCEL_PLAN "remove_plan"
#define SUSPENSION "suspension"
#define RECONNECTION "reconnection"
#define TERMINATION "termination"
#define NDS_RETRACK "nds_retrack"
#define PK_RETRACK "cisco_retrack"
#define PK1_RETRACK "cisco1_retrack"
#define	VM_RETRACK "vm_retrack" // VERIMATRIX
#define BILL_ADJ "bill_adj"
#define ACCT_ADJ "act_adj"
#define OSD "osd"
#define BMAIL "email"
#define BOUQUET_ID "bouquet"
#define PERSONAL_BIT "personnel_bit"


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
#include "mso_prov.h"
#include "mso_bulk_load.h"

/*******************************************************************
 * Fuctions defined in this code 
 *******************************************************************/
extern void 
replace_char (
	char	*sptr, 
	char	find, 
	char	replace);
 
 
EXPORT_OP void
op_mso_integ_get_failed_job_details(
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_mso_integ_get_failed_job_details(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **out_flistpp,
        pin_errbuf_t            *ebufp);


/*******************************************************************
 * Main routine for the MSO_OP_PROV_GET_FAILED_ORDERS command
 *******************************************************************/
void
op_mso_integ_get_failed_job_details(
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *r_flistp = NULL;
        pcm_context_t           *ctxp = connp->dm_ctx;
        poid_t                  *pdp = NULL;
        int32                   failure = 1;
		int32					sucess = 0;

    /***********************************************************
     * Null out results until we have some.
     ***********************************************************/
    *ret_flistpp = NULL;
    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Insanity check.
     ***********************************************************/
    if (opcode != MSO_OP_INTEG_GET_FAILED_JOB_DETAILS) {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_OPCODE, 0, 0, opcode);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "bad opcode in op_mso_integ_get_failed_job_details", ebufp);
        return;
    }

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        " op_mso_integ_get_failed_job_details input flist", in_flistp);


    /***********************************************************
     * Call main function to do it
     ***********************************************************/
    fm_mso_integ_get_failed_job_details(ctxp, in_flistp,&r_flistp, ebufp);

    /***********************************************************
     * Results.
     ***********************************************************/
    if (PIN_ERRBUF_IS_ERR(ebufp)) 
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            " op_mso_integ_get_failed_job_details error", ebufp);

        PIN_ERRBUF_CLEAR(ebufp);
        *ret_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, &failure, ebufp);
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
    } 
    else 
    {
        *ret_flistpp = r_flistp;
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, &sucess, ebufp);
        PIN_ERRBUF_CLEAR(ebufp);
    }

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
         " op_mso_integ_get_failed_job_details return flist", *ret_flistpp);

    return;
}

/*******************************************************************
 * fm_mso_integ_get_failed_job_details()
 *
 * Perfomrs following action:
 * 1. 
 *******************************************************************/
static void
fm_mso_integ_get_failed_job_details(
    pcm_context_t           *ctxp,
    pin_flist_t             *in_flistp,
    pin_flist_t             **out_flistpp,
    pin_errbuf_t            *ebufp)
{

    pin_flist_t     *read_i_flist = NULL;
    pin_flist_t     *read_o_flist = NULL;
    pin_flist_t     *tmp_flistp= NULL;
    pin_flist_t     *r_flistp = NULL;
    pin_flist_t     *sub_flistp = NULL;
	pin_flist_t     *nameinfo_bill=NULL;
	pin_flist_t 	*nameinfo_inst=NULL;
	pin_flist_t     *results_nameinfo_flistp=NULL;
	pin_flist_t		*resubmit_nameinfo_bill_flistp=NULL;
	pin_flist_t     *resubmit_nameinfo_inst_flistp=NULL;
	
	char *inst_addrsss=NULL;
	char *bill_address=NULL;
	
    pin_buf_t	*pin_bufp     = NULL;
    int32	state_change_flag	= BULK_DEVICE_STATE_UPDATE;
    int32	location_updater_flag	= BULK_DEVICE_LOCATION_UPDATER;
    int32	device_loader_flag	= BULK_DEVICE_LOAD;
    int32	preactivation_flag = BULK_DEVICE_CATV_PREACTIVATION;
    int32	add_plan_flag	= BULK_ADD_PLAN;
    int32	change_plan_flag = BULK_CHANGE_PLAN;
    int32	cancel_plan_flag = BULK_CANCEL_PLAN;
    int32	suspension_flag = BULK_SERVICE_SUSPENSION;
    int32	reconnection_flag = BULK_SERVICE_REACTIVATION;
    int32	termination_flag = BULK_SERVICE_TERMINATION;
    int32	adjustment_flag = BULK_ADJUSTMENT;
    int32	retrack_flag = BULK_RETRACK;
    int32	osd_flag = BULK_OSD;
    int32	bmail_flag = BULK_BMAIL;
    int32	bit_flag = BULK_SET_PERSONAL_BIT;
    int32	bouquet_flag = BULK_CHANGE_BOUQUET_ID;
    poid_t	*task_poidp = NULL;
    char	*task_pdp_type = NULL;
    char	*agreement_no	= NULL;
    char	*account_no	= NULL;

    int32	job_flag=0;
    int32	task_no=0;
    int32	invalid_task_no=0;
    pin_flist_t	*data_flistp = NULL;
    pin_flist_t	*plan_flistp = NULL;
    pin_flist_t	*product_flistp = NULL;
    pin_flist_t	*temp_flistp = NULL;
    poid_t	*obj_pdp = NULL;
    int32	elem_id = 0;
    pin_cookie_t	cookie = NULL;
	char 	*res_inst_addrsss=NULL;
	char 	*res_bill_addrsss=NULL;
	char	*res_bill_address=NULL;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_integ_get_failed_job_details input flist", in_flistp);

    /***********************************************************
     * Check mandatory fields are available
     ***********************************************************/


    if (PIN_ERRBUF_IS_ERR(ebufp)) 
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "fm_mso_integ_get_failed_job_details error: required field missing in input flist", ebufp);
        return;
    }

	read_i_flist = PIN_FLIST_CREATE(ebufp);
	*out_flistpp = PIN_FLIST_CREATE(ebufp);

    PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ORDER_OBJ, read_i_flist, PIN_FLD_POID, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_integ_get_failed_job_details search input flist ", read_i_flist);
    PCM_OP(ctxp, PCM_OP_READ_OBJ, PCM_OPFLG_CACHEABLE, read_i_flist,
        &read_o_flist, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_integ_get_failed_job_details search output flist", read_o_flist);

	task_poidp = PIN_FLIST_FLD_GET(read_o_flist, PIN_FLD_ORDER_OBJ, 1, ebufp);
if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Nagaraj: Error here 1");
    }
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, " task_poid is ", task_poidp);
	task_pdp_type = (char *)PIN_POID_GET_TYPE(task_poidp);

	if(strstr(task_pdp_type,STB_DEVICE_LOAD))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " STB device load get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &device_loader_flag , ebufp);
		PIN_FLIST_FLD_SET(*out_flistpp ,PIN_FLD_DEVICE_TYPE, "STB" , ebufp);
	}
	if(strstr(task_pdp_type,VC_DEVICE_LOAD))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " VC device load get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &device_loader_flag , ebufp);
		PIN_FLIST_FLD_SET(*out_flistpp ,PIN_FLD_DEVICE_TYPE, "VC" , ebufp);
	}
	if(strstr(task_pdp_type,MODEM_DEVICE_LOAD))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " MODEM device load get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &device_loader_flag , ebufp);
		PIN_FLIST_FLD_SET(*out_flistpp ,PIN_FLD_DEVICE_TYPE, "MODEM" , ebufp);
	}
	if(strstr(task_pdp_type,NAT_DEVICE_LOAD))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " NAT device load get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &device_loader_flag , ebufp);
		PIN_FLIST_FLD_SET(*out_flistpp ,PIN_FLD_DEVICE_TYPE, "NAT" , ebufp);
	}
	if(strstr(task_pdp_type,CABLE_ROUTER_DEVICE_LOAD))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Cable Router device load get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &device_loader_flag , ebufp);
		PIN_FLIST_FLD_SET(*out_flistpp ,PIN_FLD_DEVICE_TYPE, "ROUTER/CABLE" , ebufp);
	}
	if(strstr(task_pdp_type,WIFI_ROUTER_DEVICE_LOAD))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " WIFI Router device load get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &device_loader_flag , ebufp);
		PIN_FLIST_FLD_SET(*out_flistpp ,PIN_FLD_DEVICE_TYPE, "ROUTER/WIFI" , ebufp);
	}
	if(strstr(task_pdp_type,STATIC_IP_DEVICE_LOAD))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Static IP device load get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &device_loader_flag , ebufp);
		PIN_FLIST_FLD_SET(*out_flistpp ,PIN_FLD_DEVICE_TYPE, "IP/STATIC" , ebufp);
	}
	if(strstr(task_pdp_type,FRAMED_IP_DEVICE_LOAD))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Framed IP device load get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &device_loader_flag , ebufp);
		PIN_FLIST_FLD_SET(*out_flistpp ,PIN_FLD_DEVICE_TYPE, "IP/FRAMED" , ebufp);
	}
	if(strstr(task_pdp_type,STB_LOCATION_UPDATE))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " STB device location update get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &location_updater_flag , ebufp);
		PIN_FLIST_FLD_SET(*out_flistpp ,PIN_FLD_DEVICE_TYPE, "STB" , ebufp);
	}
	if(strstr(task_pdp_type,VC_LOCATION_UPDATE))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " VC device location update get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &location_updater_flag , ebufp);
		PIN_FLIST_FLD_SET(*out_flistpp ,PIN_FLD_DEVICE_TYPE, "VC" , ebufp);
	}
	if(strstr(task_pdp_type,MODEM_LOCATION_UPDATE))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " MODEM device location update get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &location_updater_flag , ebufp);
		PIN_FLIST_FLD_SET(*out_flistpp ,PIN_FLD_DEVICE_TYPE, "MODEM" , ebufp);
	}
	if(strstr(task_pdp_type,CABLE_ROUTER_LOCATION_UPDATE))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Cable Router device location update get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &location_updater_flag , ebufp);
		PIN_FLIST_FLD_SET(*out_flistpp ,PIN_FLD_DEVICE_TYPE, "ROUTER/CABLE" , ebufp);
	}
	if(strstr(task_pdp_type,WIFI_ROUTER_LOCATION_UPDATE))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " WIFI Router device location update get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &location_updater_flag , ebufp);
		PIN_FLIST_FLD_SET(*out_flistpp ,PIN_FLD_DEVICE_TYPE, "ROUTER/WIFI" , ebufp);
	}
	if(strstr(task_pdp_type,NAT_LOCATION_UPDATE))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " NAT device location update get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &location_updater_flag , ebufp);
		PIN_FLIST_FLD_SET(*out_flistpp ,PIN_FLD_DEVICE_TYPE, "NAT" , ebufp);
	}
	if(strstr(task_pdp_type,STATIC_IP_LOCATION_UPDATE))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Static IP device location update get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &location_updater_flag , ebufp);
		PIN_FLIST_FLD_SET(*out_flistpp ,PIN_FLD_DEVICE_TYPE, "IP/STATIC" , ebufp);
	}
	if(strstr(task_pdp_type,FRAMED_IP_LOCATION_UPDATE))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Framed IP device location update get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &location_updater_flag , ebufp);
		PIN_FLIST_FLD_SET(*out_flistpp ,PIN_FLD_DEVICE_TYPE, "IP/FRAMED" , ebufp);
	}
	if(strstr(task_pdp_type,STB_STATE_CHANGE))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " STB state change get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &state_change_flag , ebufp);
		PIN_FLIST_FLD_SET(*out_flistpp ,PIN_FLD_DEVICE_TYPE, "STB" , ebufp);
	}
	if(strstr(task_pdp_type,VC_STATE_CHANGE))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " VC state change get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &state_change_flag , ebufp);
		PIN_FLIST_FLD_SET(*out_flistpp ,PIN_FLD_DEVICE_TYPE, "VC" , ebufp);
	}
	if(strstr(task_pdp_type,MODEM_STATE_CHANGE))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " MODEM state change get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &state_change_flag , ebufp);
		PIN_FLIST_FLD_SET(*out_flistpp ,PIN_FLD_DEVICE_TYPE, "MODEM" , ebufp);
	}
	if(strstr(task_pdp_type,CABLE_ROUTER_STATE_CHANGE))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Cable Router state change get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &state_change_flag , ebufp);
		PIN_FLIST_FLD_SET(*out_flistpp ,PIN_FLD_DEVICE_TYPE, "ROUTER/CABLE" , ebufp);
	}
	if(strstr(task_pdp_type,WIFI_ROUTER_STATE_CHANGE))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " WIFI Router state change get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &state_change_flag , ebufp);
		PIN_FLIST_FLD_SET(*out_flistpp ,PIN_FLD_DEVICE_TYPE, "ROUTER/WIFI" , ebufp);
	}
	if(strstr(task_pdp_type,NAT_STATE_CHANGE))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " NAT state change get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &state_change_flag , ebufp);
		PIN_FLIST_FLD_SET(*out_flistpp ,PIN_FLD_DEVICE_TYPE, "NAT" , ebufp);
	}
	if(strstr(task_pdp_type,STATIC_IP_STATE_CHANGE))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Static IP state change get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &state_change_flag , ebufp);
		PIN_FLIST_FLD_SET(*out_flistpp ,PIN_FLD_DEVICE_TYPE, "IP/STATIC" , ebufp);
	}
	if(strstr(task_pdp_type,FRAMED_IP_STATE_CHANGE))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Framed IP state change get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &state_change_flag , ebufp);
		PIN_FLIST_FLD_SET(*out_flistpp ,PIN_FLD_DEVICE_TYPE, "IP/FRAMED" , ebufp);
	}
	if(strstr(task_pdp_type,PREACTIVATION))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " catv preactivation get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &preactivation_flag , ebufp);
	}
	if(strstr(task_pdp_type,ADD_PLAN))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Add Plan get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &add_plan_flag , ebufp);
	}
	if(strstr(task_pdp_type,CHANGE_PLAN))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " change plan get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &change_plan_flag , ebufp);
	}
	if(strstr(task_pdp_type,CANCEL_PLAN))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " cancel plan get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &cancel_plan_flag , ebufp);
	}
	if(strstr(task_pdp_type,SUSPENSION))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " service suspension get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &suspension_flag , ebufp);
	}
	if(strstr(task_pdp_type,RECONNECTION))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " service reconection get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &reconnection_flag , ebufp);
	}
	if(strstr(task_pdp_type,TERMINATION))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " service termination get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &termination_flag , ebufp);
	}
	if((strstr(task_pdp_type,NDS_RETRACK)) || (strstr(task_pdp_type,PK_RETRACK)) || (strstr(task_pdp_type,PK1_RETRACK)) || (strstr(task_pdp_type,VM_RETRACK))) // VERIMATRIX
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Bulk retrack get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &retrack_flag , ebufp);
	}
	if((strstr(task_pdp_type,BILL_ADJ)) || (strstr(task_pdp_type,ACCT_ADJ)))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Bulk adjustments get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &adjustment_flag , ebufp);
	}
	if(strstr(task_pdp_type,OSD))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Bulk OSD get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &osd_flag , ebufp);
	}
	if(strstr(task_pdp_type,BMAIL))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Bulk Bmail get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &bmail_flag , ebufp);
	}
	/*if(strstr(task_pdp_type,BOUQUET_ID))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Bulk change bouquet id get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &bouquet_flag , ebufp);
	}*/
	if(strstr(task_pdp_type,PERSONAL_BIT))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Bulk set personnel bit get details");
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &bit_flag , ebufp);
	}
        if(strstr(task_pdp_type,TOPUP))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Bulk set Topup get details");
                job_flag = BULK_TOPUP;
        }
        if(strstr(task_pdp_type,RENEW))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Bulk set Renew get details");
		job_flag = BULK_RENEW;
        }
        if(strstr(task_pdp_type,EXTEND_VALIDITY))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Bulk set Extend validity get details");
		job_flag = BULK_EXTEND_VALIDITY;
        }
        if(strstr(task_pdp_type,ADDITION_MB_GB))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Bulk set Addition MB_GB get details");
		job_flag = BULK_ADDITION_MB_GB;
        }
        if(strstr(task_pdp_type,WRITEOFF_CPE))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Bulk set WRITEOFF CPE get details");
		job_flag = BULK_WRITEOFF_CPE;
        }
        if(strstr(task_pdp_type,SWAPPING_CPE))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Bulk set Swapping CPE get details");
		job_flag = BULK_SWAPPING_CPE;
        }
        if(strstr(task_pdp_type,CHECK_BOUNCE))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Bulk set Check Bounce get details");
		job_flag = BULK_CHECK_BOUNCE;
        }
        if(strstr(task_pdp_type,BILL_HOLD))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Bulk set Bill Hold get details");
		job_flag = BULK_BILL_HOLD;
        }
        if(strstr(task_pdp_type,REFUND_LOAD))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Bulk set Refund loading get details");
		job_flag = BULK_REFUND_LOAD;
        }
        if(strstr(task_pdp_type,REFUND_REV_LOAD))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Bulk set Refund Reversal loading get details");
		job_flag = BULK_REFUND_REV_LOAD;
        }
	if (job_flag != 0) {
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_FLAGS , &job_flag , ebufp);
	}

if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Nagaraj: Error here:");
    }

	
	PIN_FLIST_FLD_COPY(read_o_flist, PIN_FLD_POID, *out_flistpp , PIN_FLD_POID , ebufp);
	PIN_FLIST_FLD_COPY(read_o_flist, PIN_FLD_ORDER_ID , *out_flistpp ,PIN_FLD_ORDER_ID , ebufp);
	PIN_FLIST_FLD_COPY(read_o_flist, PIN_FLD_ORDER_OBJ , *out_flistpp ,PIN_FLD_ORDER_OBJ , ebufp);	

if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Nagaraj: Error here 2");
    }
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "before reading input flist", read_o_flist);
	pin_bufp = (pin_buf_t *) PIN_FLIST_FLD_GET(read_o_flist, PIN_FLD_INPUT_FLIST, 0, ebufp );
    if (pin_bufp) 
    { 
        //PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, pin_bufp->data);

        /* Convert the buffer to a flist */
        PIN_STR_TO_FLIST((char *)pin_bufp->data, 1, &r_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " r_flistp is " , r_flistp);
		if((strstr(task_pdp_type,ADD_PLAN)) || (strstr(task_pdp_type,CHANGE_PLAN)) || (strstr(task_pdp_type,CANCEL_PLAN)))
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " r_flistp is " , r_flistp);

			account_no = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp);
			agreement_no	 = PIN_FLIST_FLD_GET(r_flistp, MSO_FLD_AGREEMENT_NO , 1 , ebufp);

			PIN_FLIST_FLD_SET(r_flistp ,PIN_FLD_ACCOUNT_NO , account_no , ebufp);
			PIN_FLIST_FLD_SET(r_flistp ,MSO_FLD_AGREEMENT_NO , agreement_no , ebufp);

			if(PIN_ERRBUF_IS_ERR(ebufp))
				PIN_ERRBUF_CLEAR(ebufp);
		}
		if((strstr(task_pdp_type,PK_RETRACK)) || (strstr(task_pdp_type,PK1_RETRACK)) || (strstr(task_pdp_type,VM_RETRACK))) // VERIMATRIX
		{
			PIN_FLIST_FLD_RENAME(r_flistp, MSO_FLD_STB_ID, PIN_FLD_DEVICE_ID , ebufp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
				PIN_ERRBUF_CLEAR(ebufp);
		}
		if(strstr(task_pdp_type,NDS_RETRACK))
		{
			PIN_FLIST_FLD_RENAME(r_flistp, MSO_FLD_VC_ID, PIN_FLD_DEVICE_ID , ebufp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
				PIN_ERRBUF_CLEAR(ebufp);
		}
	if (strstr(task_pdp_type, BULK_TOPUP_TASK_TYPE)) task_no = BULK_TOPUP;
	else if (strstr(task_pdp_type, BULK_RENEW_TASK_TYPE)) task_no = BULK_RENEW;
	else if (strstr(task_pdp_type, BULK_EXTEND_VALIDITY_TASK_TYPE)) task_no = BULK_EXTEND_VALIDITY;
	else if (strstr(task_pdp_type, BULK_ADDITION_MB_GB_TASK_TYPE)) task_no = BULK_ADDITION_MB_GB;
	else if (strstr(task_pdp_type, BULK_WRITEOFF_CPE_TASK_TYPE)) task_no = BULK_WRITEOFF_CPE;
	else if (strstr(task_pdp_type, BULK_SWAPPING_CPE_TASK_TYPE)) task_no = BULK_SWAPPING_CPE;
	else if (strstr(task_pdp_type, BULK_CHECK_BOUNCE_TASK_TYPE)) task_no = BULK_CHECK_BOUNCE;
	else if (strstr(task_pdp_type, BULK_BILL_HOLD_TASK_TYPE)) task_no = BULK_BILL_HOLD;
	else if (strstr(task_pdp_type, BULK_REFUND_LOAD_TASK_TYPE)) task_no = BULK_REFUND_LOAD;
	else if (strstr(task_pdp_type, BULK_REFUND_REV_LOAD_TASK_TYPE)) task_no = BULK_REFUND_REV_LOAD;
	else invalid_task_no = 1;

	if (invalid_task_no != 1 && task_no != BULK_REFUND_LOAD && task_no != BULK_REFUND_REV_LOAD && 
		task_no != BULK_CHECK_BOUNCE) {

		/* Populate account number */
		if (task_no != BULK_BILL_HOLD) {
//			obj_pdp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_POID, 0, ebufp);
//			fm_mso_utils_read_object(ctxp, ACCOUNT_OBJECT, obj_pdp, &data_flistp, ebufp);
//			PIN_FLIST_FLD_DROP(r_flistp, PIN_FLD_POID, ebufp);
//			if (task_no != BULK_WRITEOFF_CPE && task_no != BULK_EXTEND_VALIDITY && task_no != BULK_SWAPPING_CPE)
//				PIN_FLIST_FLD_DROP(r_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
//			PIN_FLIST_FLD_COPY(data_flistp, PIN_FLD_ACCOUNT_NO, r_flistp, PIN_FLD_ACCOUNT_NO, ebufp);
		}
		else {
//			obj_pdp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
//			fm_mso_utils_read_object(ctxp, ACCOUNT_OBJECT, obj_pdp, &data_flistp, ebufp);
//			PIN_FLIST_FLD_DROP(r_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
//			PIN_FLIST_FLD_COPY(data_flistp, PIN_FLD_ACCOUNT_NO, r_flistp, PIN_FLD_ACCOUNT_NO, ebufp);
//
//			/* Dropping the billinfo poid*/
//			temp_flistp = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_BILLINFO, 0, 0, ebufp);
//			PIN_FLIST_FLD_DROP(temp_flistp, PIN_FLD_POID, ebufp);
		}

		if (task_no == BULK_TOPUP || task_no == BULK_RENEW || task_no == BULK_ADDITION_MB_GB 
		    || task_no == BULK_SWAPPING_CPE || task_no == BULK_EXTEND_VALIDITY) {
			/* Populate service agreement number*/
//                        obj_pdp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
//                        fm_mso_utils_read_object(ctxp, BB_SERVICE_OBJECT, obj_pdp, &data_flistp, ebufp);
//                        PIN_FLIST_FLD_DROP(r_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
//                        PIN_FLIST_FLD_COPY(data_flistp, MSO_FLD_AGREEMENT_NO, r_flistp, MSO_FLD_AGREEMENT_NO, ebufp);

			if (task_no != BULK_EXTEND_VALIDITY && task_no != BULK_SWAPPING_CPE) {
                        	/* Populate plan name*/
                        	//plan_flistp = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_PLAN, 0, 0, ebufp);
//                        	obj_pdp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
//                        	fm_mso_utils_read_object(ctxp, PLAN_OBJECT, obj_pdp, &data_flistp, ebufp);
                        	//PIN_FLIST_FLD_DROP(plan_flistp, PIN_FLD_PLAN_OBJ, ebufp);
//                        	PIN_FLIST_FLD_COPY(data_flistp, PIN_FLD_NAME, r_flistp, PIN_FLD_NAME, ebufp);

                        	/* Populate product name*/
//				while (product_flistp = PIN_FLIST_ELEM_GET_NEXT(plan_flistp, PIN_FLD_PRODUCTS, 
//					&elem_id, 1, &cookie, ebufp)) {
//                        		obj_pdp = PIN_FLIST_FLD_GET(product_flistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
//                        		fm_mso_utils_read_object(ctxp, PRODUCT_OBJECT, obj_pdp, &data_flistp, ebufp);
//                        		PIN_FLIST_FLD_DROP(product_flistp, PIN_FLD_PRODUCT_OBJ, ebufp);
//                        		PIN_FLIST_FLD_COPY(data_flistp, PIN_FLD_NAME, product_flistp, PIN_FLD_NAME, ebufp);
//				}
			}

			PIN_FLIST_DESTROY_EX(&data_flistp, NULL);
		}
	}
		
		tmp_flistp = PIN_FLIST_ELEM_ADD(*out_flistpp, MSO_FLD_TASK, 0, ebufp);
        PIN_FLIST_CONCAT(tmp_flistp, r_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_mso_integ_get_failed_job_details return flist", *out_flistpp);
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
    }
 PIN_FLIST_DESTROY_EX(&read_i_flist, NULL);
 PIN_FLIST_DESTROY_EX(&read_o_flist, NULL);
    return;
}
