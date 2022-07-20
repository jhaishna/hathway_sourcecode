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


#define FILE_LOGNAME "fm_mso_preactvn_modify_srvc.c"

#define LOCAL_TRANS_OPEN_SUCCESS 0
#define LOCAL_TRANS_OPEN_FAIL 1

#define READONLY 0
#define READWRITE 1
#define LOCK_OBJ 2



/*******************************************************************************
 * Functions contained within this source file
 ******************************************************************************/
EXPORT_OP void
op_mso_preactvn_modify_srvc(
    cm_nap_connection_t    *connp,
    int32            opcode,
    int32            flags,
    pin_flist_t        *i_flistp,
    pin_flist_t        **o_flistpp,
    pin_errbuf_t        *ebufp);

static void
fm_mso_preactvn_modify_srvc(
    pcm_context_t        *ctxp,
    pin_flist_t        *i_flistp,
    pin_flist_t        **ret_flistpp,
    pin_errbuf_t        *ebufp);

static void
fm_mso_preactv_change_plan(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp);


static void
fm_mso_preactv_suspend_svc(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp);

static void
fm_mso_preactv_reactivate_svc(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp);

static void
fm_mso_preactv_svc_param_change(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **r_flistpp,
	char					*param_name,
    pin_errbuf_t            *ebufp);

static void
fm_mso_preactv_swap(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **r_flistpp,
	char					*device_type,
    pin_errbuf_t            *ebufp);

static void
    fm_mso_preactv_device_reset_pairing(
    pcm_context_t       *ctxp,
    pin_flist_t         *i_flistp,
    pin_flist_t         **r_flistpp,
    pin_errbuf_t         *ebufp);


 static void
	mso_get_device_details(
		pcm_context_t       *ctxp,
		pin_flist_t		*i_flistp,
		pin_flist_t		**r_flistpp,
		pin_errbuf_t         *ebufp);

static void
    fm_mso_preatv_device_set_pairing(
    pcm_context_t       *ctxp,
    pin_flist_t         *i_flistp,
    pin_flist_t         **r_flistpp,
	time_t				*warranty_endp,
    pin_errbuf_t         *ebufp);



/**************************************
* External Functions start
***************************************/
extern int32
fm_mso_trans_open(
        pcm_context_t   *ctxp,
        poid_t          *pdp,
        int32           flag,
        pin_errbuf_t    *ebufp);

extern void
fm_mso_trans_commit(
        pcm_context_t   *ctxp,
        poid_t          *pdp,
        pin_errbuf_t    *ebufp);

extern void
fm_mso_trans_abort(
        pcm_context_t   *ctxp,
        poid_t          *pdp,
        pin_errbuf_t    *ebufp);

extern int
fm_mso_preactvn_validate_changeplan_inp(
    pcm_context_t        *ctxp,
    pin_flist_t        *i_flistp,
    pin_flist_t        **ret_flistpp,
    pin_errbuf_t        *ebufp);

extern int
fm_mso_preactvn_validate_suspndsvc_inp(
    pcm_context_t        *ctxp,
    pin_flist_t        *i_flistp,
    pin_flist_t        **ret_flistpp,
    pin_errbuf_t        *ebufp);


/*******************************************************************************
 * Sachid: 
 * Opcode Name: MSO_OP_PREACTVN_MODIFY_SRVC
 * Description:
 * Input Flist: 
 * Level     Name		Type	M/O	Sample Value			Description
 *------    -------     	-----   ----    -------------------------       -------------
 *  0	 PIN_FLD_POID 		POID	M	0.0.0.1 /account 41345 11	Account Poid present in /mso_catv_preactivated_svc
 *  0	 PIN_FLD_USERID		POID	M	0.0.0.1 /account 44029 10	CSR Account poid doing this action
 *  0	 PIN_FLD_ACTION_MODE	ENUM	M	1				1:  CHANGE_PLAN
 *										2:  SUSPEND_SERVICE
 *										3:  REACTIVATE_SUBSCRIBER
 *										4:  SWAP_STB
 *										5:  SWAP_VC
 *										6:  LOCATION_CHANGE
 *										7:  CHANGE_BOUQUET ID
 *										8:  RESEND_ZIP_CODE
 *										9:  RESET_PIN
 *										10: BMAIL
 *										11: OSD
 *										12: RETRACK
 *  0	PIN_FLD_SERVICE_OBJ	POID	M	0.0.0.1 /mso_catv_preactivated_svc 824758	preactivated service poid
 *  0	PIN_FLD_PROGRAM_NAME	STR	M	OAP|suresh001	<Application>|<username>
 *  0	PIN_FLD_PLAN_LISTS	ARRAY	M	 	Array to contain plan
 *  1	PIN_FLD_FLAGS	INT	M	0	0:Delete
 *  						1:Add
 *  1	MSO_FLD_CA_ID	STR	M	250	CA_ID of the plan
 *  0	PIN_FLD_PLAN_LISTS	ARRAY	M	 	 
 *  1	PIN_FLD_FLAGS	STRING	M	1	0:Delete
 *  						1: Add
 *  1	PIN_FLD_PLAN_OBJ	STRING	M	0.0.0.1 /plan 41089 8	Plan Poid passed by OAP
 *  1   MSO_FLD_NETWORK_NODE    STRING  O       Network node when FLAG=1
 *  ----------------------
 *  Return flist
 *
 ******************************************************************************/
void
op_mso_preactvn_modify_srvc(
    cm_nap_connection_t	*connp,
    int32            	opcode,
    int32            	flags,
    pin_flist_t        	*i_flistp,
    pin_flist_t        	**o_flistpp,
    pin_errbuf_t	*ebufp)
{
    pcm_context_t        *ctxp = connp->dm_ctx;

    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR(ebufp);

    /*
     * Sanity check.
     */
    if (opcode != MSO_OP_PREACTVN_MODIFY_SRVC) 
    {

        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
            PIN_ERR_BAD_OPCODE, 0, 0, opcode);

        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "bad opcode in op_mso_preactvn_modify_srvc", ebufp);

        *o_flistpp = NULL;
        return;
    }


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "op_mso_preactvn_modify_srvc input flist", i_flistp);

    /*******************************************************************
     *   # number of field entries allocated 20, used 9
     *   0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 987643 1
     *   0 PIN_FLD_USERID         POID [0] 0.0.0.1 /account 80076 1
     *   0 PIN_FLD_ACTION_MODE    ENUM [0] 1
     *   0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /mso_catv_preactivated_svc 628964 1
     *   0 PIN_FLD_PROGRAM_NAME    STR [0] "OAP|suresh001"
     *   0 PIN_FLD_PLAN_LISTS    ARRAY [0] allocated 1, used 1
     *   1    PIN_FLD_FLAGS        INT [0] 0
     *   1    MSO_FLD_CA_ID        STR [0] "253"
     *   0 PIN_FLD_PLAN_LISTS    ARRAY [1] allocated 1, used 1
     *   1    PIN_FLD_FLAGS        STR [0] 1
     *   1    PIN_FLD_PLAN_OBJ     STR [0] 0.0.0.1 /plan 41089 8
     *   1    MSO_FLD_NETWORK_NODE STR [0] "NDS"
     *********************************************************************/

    fm_mso_preactvn_modify_srvc(ctxp, i_flistp, o_flistpp, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp)) 
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_mso_preactvn_modify_srvc error", ebufp);
        *o_flistpp = NULL;
    }
    else
    {
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "op_mso_preactvn_modify_srvc return flist",
            *o_flistpp);
    }

    return;
}


/*******************************************************************************
 * Sachid : fm_mso_preactvn_modify_srvc()
 *
 * Description:
 * Action	Description
 * ------ 	---------------------
 *   101	Change Plan of the preactivated service
 *   102	Suspend the preactivated service
 *   103	Reactivate the suspended preactivated service
 *   104	Swap STB of the preactivated service
 *   105	Swap VC of the preactivated service
 *   106	Change location of the preactivated service
 ******************************************************************************/
static void
fm_mso_preactvn_modify_srvc(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **ret_flistpp,
    pin_errbuf_t            *ebufp)
{

    int32		*action_p = NULL;
    int32		action;
    int			local = LOCAL_TRANS_OPEN_SUCCESS;
    int32		status_uncommitted =1;
	
	char		*error = NULL;
    pin_flist_t	*r_flistp = NULL;

    poid_t		*inp_pdp = NULL;

    if (PIN_ERRBUF_IS_ERR(ebufp)){
        return;
    }
    PIN_ERRBUF_CLEAR(ebufp);

    /*
     * Validate the input flist if required fields are present
     */
    /*
      TODO : Validate mandatory fields in the input
	   : Also validate that PIN_FLD_PLAN_LISTS ARRAY [1] should contain PIN_FLD_FLAGS - 1
	   : and PIN_FLD_PLAN_LISTS ARRAY [0] should contain PIN_FLD_FLAGS - 0
     */
    
    /* Open transaction */
    inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
    local = fm_mso_trans_open(ctxp, inp_pdp, 3,ebufp);
     if(PIN_ERRBUF_IS_ERR(ebufp))
     {
                     PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Unable to open Local Transaction: Error", ebufp);
                     PIN_ERRBUF_CLEAR(ebufp);
                     *ret_flistpp = PIN_FLIST_CREATE(ebufp);
                     PIN_FLIST_FLD_SET(*ret_flistpp , PIN_FLD_POID, inp_pdp, ebufp );
                     PIN_FLIST_FLD_SET(*ret_flistpp , PIN_FLD_STATUS, &status_uncommitted, ebufp);
                     PIN_FLIST_FLD_SET(*ret_flistpp , PIN_FLD_ERROR_CODE, "51413", ebufp);
                     PIN_FLIST_FLD_SET(*ret_flistpp , PIN_FLD_ERROR_DESCR, "Unable to open Local Transaction: Error", ebufp);
                     return;
     }
     if ( local == LOCAL_TRANS_OPEN_FAIL )
     {
                     PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Unable to open Local Transaction: Already Open", ebufp);
                     PIN_ERRBUF_RESET(ebufp);
                     *ret_flistpp = PIN_FLIST_CREATE(ebufp);
                     PIN_FLIST_FLD_SET(*ret_flistpp , PIN_FLD_POID, inp_pdp, ebufp );
                     PIN_FLIST_FLD_SET(*ret_flistpp , PIN_FLD_STATUS, &status_uncommitted, ebufp);
                     PIN_FLIST_FLD_SET(*ret_flistpp , PIN_FLD_ERROR_CODE, "51414", ebufp);
                     PIN_FLIST_FLD_SET(*ret_flistpp , PIN_FLD_ERROR_DESCR, "Unable to open Local Transaction: Already Open", ebufp);
                     return;
     }


    action_p = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTION_MODE,0,ebufp);
    action = *action_p;

    switch(action){
	case PREACTV_CHANGE_PLAN:
		/* Validate change plan flist */
/*
	        fm_mso_validation_rule_1(ctxp, in_flist, MSO_OP_CUST_ADD_PLAN, &r_flistp, ebufp);
        	if (r_flistp)
        	{	
                	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "00000");
                	*ret_flistp = r_flistp;
                	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ret_flistp",*ret_flistp );
                	return 0;
        	}
*/
		fm_mso_preactv_change_plan(ctxp,i_flistp,ret_flistpp,ebufp);

		break;
	case PREACTV_SUSPEND_SERVICE:
		fm_mso_preactv_suspend_svc(ctxp,i_flistp,ret_flistpp,ebufp);
		break;
	case PREACTV_REACTIVATE_SUBSCRIBER:
		fm_mso_preactv_reactivate_svc(ctxp,i_flistp,ret_flistpp,ebufp);
		break;
	case PREACTV_SWAP_STB:
		//fm_mso_preactv_swap_stb(ctxp,i_flistp,ret_flistpp,ebufp);
		fm_mso_preactv_swap(ctxp,i_flistp,ret_flistpp,"/device/stb",ebufp);
		break;
	case PREACTV_SWAP_VC:
		fm_mso_preactv_swap(ctxp,i_flistp,ret_flistpp,"/device/vc", ebufp);
		break;
	case PREACTV_LOCATION_CHANGE:
		fm_mso_preactv_svc_param_change(ctxp,i_flistp,ret_flistpp, "REGION_KEY", ebufp);
		break;
	case PREACTV_CHANGE_BOUQUET_ID:
		fm_mso_preactv_svc_param_change(ctxp,i_flistp,ret_flistpp, "BOUQUET_ID", ebufp);
		break;
	case PREACTV_RESEND_ZIP_CODE:
		break;
	case PREACTV_RESET_PIN:
		break;
	case PREACTV_BMAIL:
		break;
	case PREACTV_OSD:
		break;
	case PREACTV_RETRACK:
		break;
    }

    /***********************************************************
     * Results.
     ***********************************************************/
	error = (char*)PIN_FLIST_FLD_GET(*ret_flistpp, PIN_FLD_ERROR_CODE, 1, ebufp);
    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_mso_preactvn_modify_srvc error", ebufp);
			PIN_ERRBUF_RESET(ebufp);
            *ret_flistpp = PIN_FLIST_CREATE(ebufp);
            PIN_FLIST_FLD_SET(*ret_flistpp , PIN_FLD_POID, inp_pdp, ebufp );
            PIN_FLIST_FLD_SET(*ret_flistpp , PIN_FLD_STATUS, &status_uncommitted, ebufp);
            PIN_FLIST_FLD_SET(*ret_flistpp , PIN_FLD_ERROR_CODE, "51413", ebufp);
            PIN_FLIST_FLD_SET(*ret_flistpp , PIN_FLD_ERROR_DESCR, "Error in fm_mso_preactvn_modify_srvc ", ebufp);
            //return;

            if(local == LOCAL_TRANS_OPEN_SUCCESS )
            {
                    fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
            }
    }
    else if ((error!=NULL) && strcmp(error,""))
    {
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_mso_preactvn_modify_srvc error", ebufp);
            if(local == LOCAL_TRANS_OPEN_SUCCESS )
            {
                    fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
            }
    }
    else
    {
            if(local == LOCAL_TRANS_OPEN_SUCCESS)
            {
                    fm_mso_trans_commit(ctxp, inp_pdp, ebufp);
            }
            else
            {
                            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Transaction NOT Comitted");
                            PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, &status_uncommitted, ebufp);
            }
    }


    CLEANUP:
        return;
}

/*******************************************************************************
 * Sachid: fm_mso_preactv_change_plan
 *
 * Description:
 * Change plan of the preactivate service
* Todo:
 * 1. Find CA_ID using the plan poid in add plan list
 * 2. Update /mso_catv_preactivated_svc with the new CA_ID
 * 3. Call create_action to create authorize and unauthorize tasks
 *
 ******************************************************************************/
static void
fm_mso_preactv_change_plan(
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
	
	poid_t		*srch_pdp = NULL;
	poid_t		*plan_pdp = NULL;
	void		*vp = NULL;
	int64		database = -1;
	int		change_plan_failure = 1;
	int		srch_flag = SRCH_EXACT;
	int		args_cnt = 1;
	int		preactv_flag = CATV_PREACTV_MOD_SVC;
	int		year;
	int		month;
	int		day;

	char		template [] = "select X from /mso_cfg_catv_pt 1 , /product 2, /deal 3, /plan 4 where 1.F1 = 2.F2 and 1.F3 = V3 and 2.F4 = 3.F5 and 3.F6 = 4.F7 and 4.F8 = V8 ";
	char		func_name[] = "fm_mso_preactv_change_plan";
	char		msg[1024];
	char		*ca_id = NULL;
	char		validity[10];
	char		msng_fldp[1024];

	time_t		rawtime;
	struct tm	*current_time = NULL;

   	if (PIN_ERRBUF_IS_ERR(ebufp)){
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_preactv_change_plan: Error before starting the function",ebufp);
        	return;
	}
	PIN_ERRBUF_CLEAR(ebufp);
		
	sprintf(msg,"%s: Input flist", func_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg, i_flistp);

	/* Validate mandatory fields */
	if(fm_mso_preactvn_validate_changeplan_inp(ctxp,i_flistp,r_flistpp,ebufp) == 0)
		return; //Validation failed
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

	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_preactv_change_plan: Prepare search flist to search CA_ID using /plan poid");

	srch_iflp = PIN_FLIST_CREATE(ebufp);
	
	// Get the plan to add from PIN_FLD_PLAN_LISTS[1]
	planlist_flp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PLAN_LISTS, 1, 0, ebufp);
	plan_pdp = PIN_FLIST_FLD_GET(planlist_flp, PIN_FLD_PLAN_OBJ, 0 , ebufp);	

	// Get the DB
	database = PIN_POID_GET_DB(plan_pdp);

	// Create /search/pin poid
	srch_pdp = (poid_t*)PIN_POID_CREATE(database, "/search/pin",-1, ebufp);

	PIN_FLIST_FLD_PUT(srch_iflp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_iflp, PIN_FLD_TEMPLATE, template, ebufp);
	PIN_FLIST_FLD_SET(srch_iflp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	
	// Add ARGS array
	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflp, PIN_FLD_ARGS, args_cnt++, ebufp);
 	PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_PROVISIONING_TAG,"",ebufp); 

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflp, PIN_FLD_ARGS, args_cnt++, ebufp);
 	PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_PROVISIONING_TAG,"",ebufp); 

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflp, PIN_FLD_ARGS, args_cnt++, ebufp);
 	tmp_flp = PIN_FLIST_ELEM_ADD(args_flistp,MSO_FLD_CATV_PT_DATA,PIN_ELEMID_ANY,ebufp); 
	PIN_FLIST_FLD_COPY(i_flistp,MSO_FLD_NETWORK_NODE, tmp_flp, MSO_FLD_NETWORK_NODE, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflp, PIN_FLD_ARGS, args_cnt++, ebufp);
 	PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_POID,NULL,ebufp); 

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflp, PIN_FLD_ARGS, args_cnt++, ebufp);
 	tmp_flp = PIN_FLIST_ELEM_ADD(args_flistp,PIN_FLD_PRODUCTS,PIN_ELEMID_ANY,ebufp); 
	PIN_FLIST_FLD_SET(tmp_flp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflp, PIN_FLD_ARGS, args_cnt++, ebufp);
 	PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_POID,NULL,ebufp); 

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflp, PIN_FLD_ARGS, args_cnt++, ebufp);
 	tmp_flp = PIN_FLIST_ELEM_ADD(args_flistp,PIN_FLD_SERVICES,PIN_ELEMID_ANY,ebufp); 
	PIN_FLIST_FLD_SET(tmp_flp, PIN_FLD_DEAL_OBJ, NULL, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflp, PIN_FLD_ARGS, args_cnt++, ebufp);
 	PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_POID,plan_pdp,ebufp); 

	//Add result
	PIN_FLIST_ELEM_ADD(srch_iflp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	
	sprintf(msg, "%s: PCM_OP_SEARCH input flist", func_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, srch_iflp);

	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflp, &srch_oflp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_iflp, NULL);
	if(PIN_ERRBUF_IS_ERR(ebufp)){
		sprintf(msg, "%s: PCM_OP_SEARCH error", func_name);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,msg, ebufp);
		return;
	}

	sprintf(msg, "%s: PCM_OP_SEARCH return flist", func_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, srch_oflp);

	res_flp = PIN_FLIST_ELEM_GET(srch_oflp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
	tmp_flp = PIN_FLIST_ELEM_GET(res_flp, MSO_FLD_CATV_PT_DATA, PIN_ELEMID_ANY,0,  ebufp);
	ca_id = PIN_FLIST_FLD_GET(tmp_flp, MSO_FLD_CA_ID, 0, ebufp);
	

	/* Now update /mso_catv_preactivated_svc object with the CA_ID */
	wrt_flp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, wrt_flp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(wrt_flp, MSO_FLD_CA_ID, ca_id, ebufp);
	sprintf(msg, "%s: PCM_OP_WRITE_FLDS input flist", func_name);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, wrt_flp);
	PCM_OP(ctxp,PCM_OP_WRITE_FLDS, 0, wrt_flp, &wrt_oflp, ebufp);
	PIN_FLIST_DESTROY_EX(&wrt_flp, NULL);
	sprintf(msg, "%s: PCM_OP_WRITE_FLDS return flist", func_name);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, wrt_oflp);


	// TODO : Create task
	provaction_inflistp = PIN_FLIST_COPY(i_flistp,ebufp);
	//PIN_FLIST_FLD_COPY(i_flistp,MSO_FLD_NETWORK_NODE, provaction_inflistp, MSO_FLD_NETWORK_NODE, ebufp);
	PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS,&preactv_flag, ebufp);

	/* Read MSO_FLD_CAS_SUBSCRIBER_ID from /mso_catv_preactivated_svc */
	sprintf(msg, "%s: PCM_OP_READ_OBJ input flist", func_name);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, wrt_oflp);
	PCM_OP(ctxp,PCM_OP_READ_OBJ, 0, wrt_oflp, &robj_oflp, ebufp);
	sprintf(msg, "%s: PCM_OP_READ_OBJ return flist", func_name);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, robj_oflp);

	PIN_FLIST_FLD_COPY(robj_oflp, MSO_FLD_CAS_SUBSCRIBER_ID, provaction_inflistp, MSO_FLD_CAS_SUBSCRIBER_ID, ebufp);
	
	/* now create task node to add plan - AUTHORIZE_SERVICES */
	taskelem_flp = PIN_FLIST_ELEM_ADD(provaction_inflistp, MSO_FLD_TASK, 1, ebufp);
	PIN_FLIST_FLD_SET(taskelem_flp, MSO_FLD_TASK_NAME, "AUTHORIZE_SERVICES", ebufp);
	delpkg_flp = PIN_FLIST_ELEM_ADD(taskelem_flp, MSO_FLD_ADD_PACKAGE_INFO, 1, ebufp);
	PIN_FLIST_FLD_SET(delpkg_flp, MSO_FLD_CA_ID, ca_id, ebufp);
	PIN_FLIST_FLD_SET(delpkg_flp, MSO_FLD_TAPING_AUTHORIZATION, "N" , ebufp);
	PIN_FLIST_FLD_SET(delpkg_flp, MSO_FLD_EXPIRATION_DATE, "0" , ebufp);
	
	/* now create task node to delete plan - UNAUTHORIZE_SERVICES */
	taskelem_flp = PIN_FLIST_ELEM_ADD(provaction_inflistp, MSO_FLD_TASK, 2, ebufp);
	PIN_FLIST_FLD_SET(taskelem_flp, MSO_FLD_TASK_NAME, "UNAUTHORIZE_SERVICES", ebufp);
	delpkg_flp = PIN_FLIST_ELEM_ADD(taskelem_flp, MSO_FLD_DEL_PACKAGE_INFO, 1, ebufp);
	// Get CA_ID of the delete plan from input_flist
	planlist_flp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PLAN_LISTS, 0, 0, ebufp);
	PIN_FLIST_FLD_COPY(planlist_flp, MSO_FLD_CA_ID, delpkg_flp, MSO_FLD_CA_ID, ebufp);
	PIN_FLIST_FLD_SET(delpkg_flp, MSO_FLD_TAPING_AUTHORIZATION, "N" , ebufp);

	PIN_FLIST_ELEM_DROP(provaction_inflistp,PIN_FLD_PLAN_LISTS, 0, ebufp);
	PIN_FLIST_ELEM_DROP(provaction_inflistp,PIN_FLD_PLAN_LISTS, 1, ebufp);

	// Set the current date as expiration date
	rawtime = time(NULL);
	current_time = localtime(&rawtime);
	year = current_time->tm_year + 1900;
	month = current_time->tm_mon + 1;
	day = current_time->tm_mday;
	memset(validity,10,'\0');
	sprintf(validity, "%04d%02d%02d",year, month, day);
	PIN_FLIST_FLD_SET(delpkg_flp, MSO_FLD_EXPIRATION_DATE, validity, ebufp);

	/* Now call MSO_OP_PROV_CREATE_ACTION */
	sprintf(msg,"%s: MSO_OP_PROV_CREATE_ACTION input flist",func_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg,provaction_inflistp);
	PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
        //PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_CREATE_ACTION ", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);

                *r_flistpp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51416", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Preactivated service - Change Plan - MSO_OP_PROV_CREATE_ACTION error", ebufp);

                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "provaction_outflistp return flist", provaction_outflistp);
	*r_flistpp = provaction_outflistp;
}



/*******************************************************************************
 * Sachid: fm_mso_preactv_svc_param_change
 *
 * Description:
 * Change the paramater of the preactivate service
 * Update /mso_catv_preactivated_svc MSO_FLD_REGION_KEY or MS_FLD_BOUQUET_ID as
 * specified in the input flist
 * Create appropriate provisioning action 
 ******************************************************************************/
static void
fm_mso_preactv_svc_param_change(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	char		*param_name,
	pin_errbuf_t	*ebufp){

	pin_flist_t	*wrt_flp = NULL;
	pin_flist_t	*wrt_oflp = NULL;
	pin_flist_t	*taskelem_flp = NULL;
	pin_flist_t	*provaction_inflistp = NULL;
	pin_flist_t	*provaction_outflistp = NULL;
	pin_flist_t	*robj_iflp = NULL;
	pin_flist_t	*robj_oflp = NULL;
	int		preactv_flag = CATV_PREACTV_MOD_SVC;
	int		change_plan_failure = 1;
	char		msg[1024];
	char		func_name[] = "fm_mso_preactv_svc_param_change";
	char		*manufacturer = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp)){
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_preactv_svc_param_change: Error before starting the function",ebufp);
        	return;
	}
	PIN_ERRBUF_CLEAR(ebufp);
	
  	sprintf(msg,"%s: Input flist",func_name);
  	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, i_flistp);
  	sprintf(msg,"%s: Param to be changed : %s",func_name, param_name);
  	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
  	/* Input flist for update service param 
		0 PIN_FLD_POID           POID [0] 0.0.0.1 /account -1 1
		0 PIN_FLD_USERID         POID [0] 0.0.0.1 /account 80076 1
		0 PIN_FLD_ACTION_MODE    ENUM [0] 106
		0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /mso_catv_preactivated_svc 628964 1
		0 PIN_FLD_PROGRAM_NAME    STR [0] "OAP|suresh001"
		0 PIN_FLD_NETWORK_NODE    STR [0] “<NDS>”
		[
		0 MSO_FLD_REGION_KEY      STR [0] "IND600006"    
				or
		0 MSO_FLD_BOUQUET_ID      STR [0] "BOUQUET_ID1"
		]
		0 PIN_FLD_PLAN_LISTS    ARRAY [0] allocated 1, used 1
		1    MSO_FLD_CA_ID        STR [0] "253"
	
  	*/
  
	sprintf(msg, "%s: Update MSO_FLD_%s of /mso_catv_preactivated_svc",func_name, param_name);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
	

	/* Update parameter of /mso_catv_preactivated_svc object */
	wrt_flp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, wrt_flp, PIN_FLD_POID, ebufp);
	if(!strcmp(param_name,"REGION_KEY")){
		PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_REGION_KEY, wrt_flp, MSO_FLD_REGION_KEY, ebufp);
	}
	else if(!strcmp(param_name,"BOUQUET_ID")){
		PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_BOUQUET_ID, wrt_flp, MSO_FLD_BOUQUET_ID, ebufp);
	}
	sprintf(msg, "%s: PCM_OP_WRITE_FLDS input flist", func_name);
    	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, wrt_flp);
	PCM_OP(ctxp,PCM_OP_WRITE_FLDS, 0, wrt_flp, &wrt_oflp, ebufp);
	PIN_FLIST_DESTROY_EX(&wrt_flp, NULL);
	sprintf(msg, "%s: PCM_OP_WRITE_FLDS return flist", func_name);
    	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, wrt_oflp);


	// Create task
	provaction_inflistp = PIN_FLIST_COPY(i_flistp,ebufp);
	PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS,&preactv_flag, ebufp);

	/* Read MSO_FLD_CAS_SUBSCRIBER_ID from /mso_catv_preactivated_svc */
	PIN_FLIST_FLD_SET(wrt_oflp, MSO_FLD_CAS_SUBSCRIBER_ID, NULL, ebufp);
	PIN_FLIST_FLD_SET(wrt_oflp, MSO_FLD_REGION_KEY, NULL, ebufp);
	PIN_FLIST_FLD_SET(wrt_oflp, PIN_FLD_DEVICE_TYPE, NULL, ebufp);
	PIN_FLIST_FLD_SET(wrt_oflp, MSO_FLD_STB_OBJ, NULL, ebufp);
	sprintf(msg, "%s: PCM_OP_READ_FLDS input flist", func_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, wrt_oflp);
	PCM_OP(ctxp,PCM_OP_READ_FLDS, 0, wrt_oflp, &robj_oflp, ebufp);
	sprintf(msg, "%s: PCM_OP_READ_FLDS return flist", func_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, robj_oflp);
	PIN_FLIST_DESTROY_EX(&wrt_oflp, NULL);

	manufacturer = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);

	PIN_FLIST_FLD_COPY(robj_oflp, MSO_FLD_CAS_SUBSCRIBER_ID, provaction_inflistp, MSO_FLD_CAS_SUBSCRIBER_ID, ebufp);
	PIN_FLIST_ELEM_DROP(provaction_inflistp,PIN_FLD_PLAN_LISTS, 0, ebufp);
	taskelem_flp = PIN_FLIST_ELEM_ADD(provaction_inflistp,MSO_FLD_TASK, 1, ebufp);
	if(!strcmp(param_name,"REGION_KEY")){
		PIN_FLIST_FLD_DROP(provaction_inflistp,MSO_FLD_REGION_KEY, ebufp);
		PIN_FLIST_FLD_COPY(robj_oflp,MSO_FLD_REGION_KEY, taskelem_flp, MSO_FLD_REGION_KEY, ebufp);
		PIN_FLIST_FLD_COPY(robj_oflp,PIN_FLD_DEVICE_TYPE, taskelem_flp, PIN_FLD_DEVICE_TYPE, ebufp);

		/* Read MSO_FLD_STB_ID  from /device/stb */
		robj_iflp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(robj_oflp,MSO_FLD_STB_OBJ, robj_iflp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(robj_iflp, PIN_FLD_DEVICE_ID, NULL, ebufp);
		sprintf(msg, "%s: PCM_OP_READ_FLDS input flist", func_name);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, robj_iflp);
		PCM_OP(ctxp,PCM_OP_READ_FLDS, 0, robj_iflp, &robj_oflp, ebufp);
		sprintf(msg, "%s: PCM_OP_READ_FLDS return flist", func_name);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, robj_oflp);

		PIN_FLIST_FLD_COPY(robj_oflp,PIN_FLD_DEVICE_ID, taskelem_flp, MSO_FLD_STB_ID, ebufp);
	}
	else if(!strcmp(param_name,"BOUQUET_ID")){
		PIN_FLIST_FLD_DROP(provaction_inflistp,MSO_FLD_BOUQUET_ID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_BOUQUET_ID, taskelem_flp, MSO_FLD_BOUQUET_ID, ebufp);
	}


	/* Now call MSO_OP_PROV_CREATE_ACTION */
	sprintf(msg,"%s: MSO_OP_PROV_CREATE_ACTION input flist",func_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg,provaction_inflistp);
	PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
        //PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_CREATE_ACTION ", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);

                *r_flistpp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51416", ebufp);
		sprintf(msg, "Preactivated service - Change %s - MSO_OP_PROV_CREATE_ACTION error",param_name);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, msg, ebufp);

                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "provaction_outflistp return flist", provaction_outflistp);
	*r_flistpp = provaction_outflistp;
}



/*******************************************************************************
 * Sachid: fm_mso_preactv_suspend_svc
 *
 * Description:
 * Suspend the preactivate service
 * Update mso_catv_preactivated_svc state id(PIN_FLD_STATE_ID) as suspended(10102)
 * Create appropriate provisioning action 
 ******************************************************************************/
static void
fm_mso_preactv_suspend_svc(
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
	pin_flist_t	*dvc_riflp = NULL;
	pin_flist_t	*dvc_roflp = NULL;

	poid_t		*srch_pdp = NULL;
	poid_t		*plan_pdp = NULL;
	
	int64		database = -1;
	int		change_plan_failure = 1;
	int		srch_flag = SRCH_EXACT;
	int		args_cnt = 1;
	int		suspend_flag = MSO_PREACTIVATED_SVC_INACTIVE;
	int		preactv_flag = CATV_PREACTV_MOD_SVC;
	
	int		year;
	int		month;
	int		day;

	char		func_name[] = "fm_mso_preactv_suspend_svc";
	char		msg[1024];
	char		*ca_id = NULL;
	char		validity[10];
	char		*manufacturer = NULL;

	time_t		rawtime;
	struct tm	*current_time = NULL;

   	if (PIN_ERRBUF_IS_ERR(ebufp)){
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_preactv_suspend_svc: Error before starting the function",ebufp);
        	return;
	}
	PIN_ERRBUF_CLEAR(ebufp);
	
  sprintf(msg,"%s: Input flist",func_name);
  PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, i_flistp);
  /* Input flist for suspend preactivated service 
     0 PIN_FLD_POID           POID [0] 0.0.0.1 /account -1 1
     0 PIN_FLD_USERID         POID [0] 0.0.0.1 /account 80076 1
     0 PIN_FLD_ACTION_MODE    ENUM [0] 102
	 0 MSO_FLD_NETWORK_NODE    STR [0] “<NDS|CISCO>”
     0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /mso_catv_preactivated_svc 628964 1
     0 PIN_FLD_PROGRAM_NAME    STR [0] "OAP|Sachid001"
     0 PIN_FLD_PLAN_LISTS    ARRAY [0] allocated 1, used 1
     1    MSO_FLD_CA_ID        STR [0] "253"

  */
  
	sprintf(msg, "%s: Validate inputs",func_name);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);

	if(fm_mso_preactvn_validate_suspndsvc_inp(ctxp,i_flistp,r_flistpp,ebufp) == 0)
		return; //Validation failed

	sprintf(msg, "%s: Update PIN_FLD_STATE_ID or /mso_catv_preactivated_svc as 10102",func_name);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
	

	/* Update /mso_catv_preactivated_svc object with suspend state_id */
	wrt_flp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, wrt_flp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(wrt_flp, PIN_FLD_STATE_ID, &suspend_flag, ebufp);
	sprintf(msg, "%s: PCM_OP_WRITE_FLDS input flist", func_name);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, wrt_flp);
	PCM_OP(ctxp,PCM_OP_WRITE_FLDS, 0, wrt_flp, &wrt_oflp, ebufp);
	PIN_FLIST_DESTROY_EX(&wrt_flp, NULL);
	sprintf(msg, "%s: PCM_OP_WRITE_FLDS return flist", func_name);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, wrt_oflp);


	// TODO : Create task
	provaction_inflistp = PIN_FLIST_COPY(i_flistp,ebufp);
	PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS,&preactv_flag, ebufp);

	/* Read MSO_FLD_CAS_SUBSCRIBER_ID from /mso_catv_preactivated_svc */
	sprintf(msg, "%s: PCM_OP_READ_OBJ input flist", func_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, wrt_oflp);
	PCM_OP(ctxp,PCM_OP_READ_OBJ, 0, wrt_oflp, &robj_oflp, ebufp);
	sprintf(msg, "%s: PCM_OP_READ_OBJ return flist", func_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, robj_oflp);

	manufacturer = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);

	if((manufacturer != NULL) && strstr(manufacturer,"NDS")){
		PIN_FLIST_FLD_COPY(robj_oflp, MSO_FLD_CAS_SUBSCRIBER_ID, provaction_inflistp, MSO_FLD_CAS_SUBSCRIBER_ID, ebufp);
	}
	else if((manufacturer != NULL) && strstr(manufacturer,"CISCO")){

		dvc_riflp = PIN_FLIST_CREATE(ebufp);

		PIN_FLIST_FLD_COPY(robj_oflp, MSO_FLD_STB_OBJ, dvc_riflp , PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(dvc_riflp, PIN_FLD_DEVICE_ID, NULL, ebufp);
		sprinft(msg,"%s: PCM_OP_READ_FLDS input flist",func_name);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, dvc_riflp);
		PCM_OP(ctxp,PCM_OP_READ_FLDS, 0, dvc_riflp, &dvc_roflp, ebufp);
		sprintf(msg, "%s: PCM_OP_READ_FLDS return flist", func_name);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, dvc_roflp);
		PIN_FLIST_FLD_COPY(dvc_roflp, PIN_FLD_DEVICE_ID, provaction_inflistp, MSO_FLD_STB_ID, ebufp);
		PIN_FLIST_DESTROY_EX(&dvc_riflp, NULL);

	}
	//PIN_FLIST_FLD_COPY(robj_oflp, MSO_FLD_NETWORK_NODE, provaction_inflistp, MSO_FLD_NETWORK_NODE, ebufp);
	PIN_FLIST_ELEM_DROP(provaction_inflistp,PIN_FLD_PLAN_LISTS, 0, ebufp);
	
	
	/* Now call MSO_OP_PROV_CREATE_ACTION */
	sprintf(msg,"%s: MSO_OP_PROV_CREATE_ACTION input flist",func_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg,provaction_inflistp);
	PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
        //PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_CREATE_ACTION ", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);

                *r_flistpp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51416", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Preactivated service - Change Plan - MSO_OP_PROV_CREATE_ACTION error", ebufp);

                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "provaction_outflistp return flist", provaction_outflistp);
	*r_flistpp = provaction_outflistp;
}


/*******************************************************************************
 * Sachid: fm_mso_preactv_reactivate_svc
 *
 * Description:
 * Activate the suspended preactivated service
 * Update mso_catv_preactivated_svc state id(PIN_FLD_STATE_ID) as active (10101)
 * if the state was 10102 (suspened)
 * Create appropriate provisioning action 
 ******************************************************************************/
static void
fm_mso_preactv_reactivate_svc(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp){

	pin_flist_t	*dvc_riflp = NULL;
	pin_flist_t	*dvc_roflp = NULL;
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
	
	poid_t		*srch_pdp = NULL;
	poid_t		*plan_pdp = NULL;
	
	int64		database = -1;
	int		change_plan_failure = 1;
	int		srch_flag = SRCH_EXACT;
	int		args_cnt = 1;
	int		suspend_flag = MSO_PREACTIVATED_SVC_INACTIVE;
	int		active_flag = MSO_PREACTIVATED_SVC_ACTIVE;
	int		preactv_flag = CATV_PREACTV_MOD_SVC;
	
	int		year;
	int		month;
	int		day;

	char		func_name[] = "fm_mso_preactv_reactivate_svc";
	char		msg[1024];
	char		*ca_id = NULL;
	char		validity[10];
	char		*manufacturer = NULL;

	time_t		rawtime;
	struct tm	*current_time = NULL;

   	if (PIN_ERRBUF_IS_ERR(ebufp)){
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_preactv_reactivate_svc: Error before starting the function",ebufp);
        	return;
	}
	PIN_ERRBUF_CLEAR(ebufp);
	
  sprintf(msg,"%s: Input flist",func_name);
  PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, i_flistp);
  /* Input flist for reactivating preactivated service 
     0 PIN_FLD_POID           POID [0] 0.0.0.1 /account -1 1
     0 PIN_FLD_USERID         POID [0] 0.0.0.1 /account 80076 1
     0 PIN_FLD_ACTION_MODE    ENUM [0] 103
	 0 MSO_FLD_NETWORK_NODE    STR [0] “<NDS|CISCO>”
	 0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /mso_catv_preactivated_svc 628964 1
     0 PIN_FLD_PROGRAM_NAME    STR [0] "OAP|Sachid001"
     0 PIN_FLD_PLAN_LISTS    ARRAY [0] allocated 1, used 1
     1    MSO_FLD_CA_ID        STR [0] "253"

  */
  	sprintf(msg, "%s: Validate inputs",func_name);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);

	if(fm_mso_preactvn_validate_suspndsvc_inp(ctxp,i_flistp,r_flistpp,ebufp) == 0)
		return; //Validation failed

	/* Read /mso_catv_preactivated_svc */
	wrt_flp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, wrt_flp, PIN_FLD_POID, ebufp);
	sprintf(msg, "%s: PCM_OP_READ_OBJ input flist", func_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, wrt_flp);
	PCM_OP(ctxp,PCM_OP_READ_OBJ, 0, wrt_flp, &robj_oflp, ebufp);
	sprintf(msg, "%s: PCM_OP_READ_OBJ return flist", func_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, robj_oflp);

	if((int)*((int*)PIN_FLIST_FLD_GET(robj_oflp, PIN_FLD_STATE_ID, 0, ebufp)) == MSO_PREACTIVATED_SVC_INACTIVE){
		sprintf(msg, "%s: Preactivated service is currently suspended, Update PIN_FLD_STATE_ID of /mso_catv_preactivated_svc as 10101",func_name);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
		/* Update /mso_catv_preactivated_svc object with suspend state_id */
		PIN_FLIST_FLD_SET(wrt_flp, PIN_FLD_STATE_ID, &active_flag, ebufp);
		sprintf(msg, "%s: PCM_OP_WRITE_FLDS input flist", func_name);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, wrt_flp);
		PCM_OP(ctxp,PCM_OP_WRITE_FLDS, 0, wrt_flp, &wrt_oflp, ebufp);
		PIN_FLIST_DESTROY_EX(&wrt_flp, NULL);
		sprintf(msg, "%s: PCM_OP_WRITE_FLDS return flist", func_name);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, wrt_oflp);

	}
	

	// TODO : Create Provisioning action
	provaction_inflistp = PIN_FLIST_COPY(i_flistp,ebufp);
	PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS,&preactv_flag, ebufp);

	manufacturer = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);

	if((manufacturer != NULL) && strstr(manufacturer,"NDS")){
		PIN_FLIST_FLD_COPY(robj_oflp, MSO_FLD_CAS_SUBSCRIBER_ID, provaction_inflistp, MSO_FLD_CAS_SUBSCRIBER_ID, ebufp);
	}
	else if((manufacturer != NULL) && strstr(manufacturer,"CISCO")){

		dvc_riflp = PIN_FLIST_CREATE(ebufp);

		PIN_FLIST_FLD_COPY(robj_oflp, MSO_FLD_STB_OBJ, dvc_riflp , PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(dvc_riflp, PIN_FLD_DEVICE_ID, NULL,ebufp);
		sprinft(msg,"%s: PCM_OP_READ_FLDS input flist",func_name);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, dvc_riflp);
		PCM_OP(ctxp,PCM_OP_READ_FLDS, 0, dvc_riflp, &dvc_roflp, ebufp);
		sprintf(msg, "%s: PCM_OP_READ_FLDS return flist", func_name);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, dvc_roflp);
		PIN_FLIST_FLD_COPY(dvc_roflp, PIN_FLD_DEVICE_ID, provaction_inflistp, MSO_FLD_STB_ID, ebufp);
		PIN_FLIST_DESTROY_EX(&dvc_riflp, NULL);

	}	PIN_FLIST_ELEM_DROP(provaction_inflistp,PIN_FLD_PLAN_LISTS, 0, ebufp);
	
	
	/* Now call MSO_OP_PROV_CREATE_ACTION */
	sprintf(msg,"%s: MSO_OP_PROV_CREATE_ACTION input flist",func_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg,provaction_inflistp);
	PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
        //PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_CREATE_ACTION ", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);

                *r_flistpp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51416", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Preactivated service - Change Plan - MSO_OP_PROV_CREATE_ACTION error", ebufp);

                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "provaction_outflistp return flist", provaction_outflistp);
	*r_flistpp = provaction_outflistp;
}




/*******************************************************************************
 * Sachid: fm_mso_preactv_swap_stb
 *
 * Description:
 * Swap the old STB with new STB
 * Tasks
 * Update /mso_catv_preactivated_svc with new STB poid
 * Update /device/stb and /device/vc objects
 * Create appropriate provisioning action 
 ******************************************************************************/
 
static void
fm_mso_preactv_swap_stb(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp){

	pin_flist_t	*dvc_riflp = NULL;
	pin_flist_t	*dvc_roflp = NULL;
	pin_flist_t	*srch_iflp = NULL;
	pin_flist_t	*srch_oflp = NULL;

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
	pin_flist_t	*rstcurstb_flp= NULL;
	
	poid_t		*srch_pdp = NULL;
	poid_t		*acct_pdp = NULL;
	poid_t		*device_pdp = NULL;
	
	int64		database = -1;
	int			change_plan_failure = 1;
	int			srch_flag = SRCH_EXACT;
	int			args_cnt = 1;
	int			suspend_flag = MSO_PREACTIVATED_SVC_INACTIVE;
	int			active_flag = MSO_PREACTIVATED_SVC_ACTIVE;
	int			preactv_flag = CATV_PREACTV_MOD_SVC;
	int			res_cnt = 0;
	
	int			year;
	int			month;
	int			day;

	char		func_name[] = "fm_mso_preactv_swap_stb";
	char		msg[1024];
	char		*ca_id = NULL;
	char		validity[10];
	char		*manufacturer = NULL;
	char		*device_type = NULL;
	char		*device_id = NULL;	

	time_t		*warranty_endp;

   	if (PIN_ERRBUF_IS_ERR(ebufp)){
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_preactv_swap_stb: Error before starting the function",ebufp);
        	return;
	}
	PIN_ERRBUF_CLEAR(ebufp);
	
  sprintf(msg,"%s: Input flist",func_name);
  PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, i_flistp);
  /* Input flist for swapping STB of preactivated service 
		0 PIN_FLD_POID           POID [0] 0.0.0.1 /account -1 1
		0 PIN_FLD_USERID         POID [0] 0.0.0.1 /account 80076 1
		0 PIN_FLD_ACTION_MODE    ENUM [0] 104
		0 PIN_FLD_REASON	     ENUM [0] <MSO_DEVICE_DEACTIVATION_REASON_FAULTY| Other>
		0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /mso_catv_preactivated_svc 628964 1
		0 PIN_FLD_PROGRAM_NAME    STR [0] "OAP|suresh001"
		0 PIN_FLD_NETWORK_NODE    STR [0] “<NDS|CISCO>”
		0 PIN_FLD_OLD_VALUE       STR [0] "2000000000000007"
		0 PIN_FLD_NEW_VALUE       STR [0] "2000000000000008"
		0 PIN_FLD_PLAN_LISTS    ARRAY [0] allocated 1, used 1
  */
	device_id = (char*)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_OLD_VALUE, 0, ebufp);
	sprintf(msg,"%s: Prepare to reset pairing for %s",func_name,device_id);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);

    /* Search device poid using device id */
	sprintf(msg,"%s: Search /device/stb poid using PIN_FLD_OLD_VALUE",func_name);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);

	dvc_riflp = PIN_FLIST_CREATE(ebufp);
	database = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_POID,0,ebufp));
	device_pdp = (poid_t*)PIN_POID_CREATE(database, "/device/stb",-1, ebufp);
	PIN_FLIST_FLD_PUT(dvc_riflp, PIN_FLD_POID, device_pdp, ebufp);
	PIN_FLIST_FLD_SET(dvc_riflp, PIN_FLD_DEVICE_ID, device_id, ebufp);

	mso_get_device_details(ctxp,dvc_riflp,&srch_oflp, ebufp);
	PIN_FLIST_DESTROY_EX(&dvc_riflp, NULL);
	if(PIN_ERRBUF_IS_ERR(ebufp)){
	    *r_flistpp = srch_oflp;
	     PIN_ERRBUF_CLEAR(ebufp);
	    return;
	}

	res_flp = PIN_FLIST_ELEM_GET(srch_oflp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
	sprintf(msg,"%s: Got the following details for %s",func_name, device_id);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg,res_flp);

	/* Get the warranty end date of the old device */
	warranty_endp = PIN_FLIST_FLD_GET(res_flp, MSO_FLD_WARRANTY_END, 0, ebufp);
	rstcurstb_flp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(res_flp, PIN_FLD_POID,rstcurstb_flp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(res_flp,PIN_FLD_STATE_ID, rstcurstb_flp, PIN_FLD_STATE_ID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_REASON, rstcurstb_flp, PIN_FLD_REASON, ebufp);



	/* TODO: 
		1. Reset pairing of the old stb with the associated VC
		2. Associate VC with the new STB
		3. Update the next device state for old and new STB
		4. Reset warranty for old device and update to new device
	*/
    fm_mso_preactv_device_reset_pairing(ctxp,rstcurstb_flp,r_flistpp,ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp)){
		PIN_ERRBUF_CLEAR(ebufp);
		return;
	}
	dvc_riflp = PIN_FLIST_CREATE(ebufp);
	database = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_POID,0,ebufp));
	device_pdp = (poid_t*)PIN_POID_CREATE(database, "/device/stb",-1, ebufp);
	PIN_FLIST_FLD_PUT(dvc_riflp, PIN_FLD_POID, device_pdp, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_NEW_VALUE, dvc_riflp, PIN_FLD_DEVICE_ID, ebufp);

	mso_get_device_details(ctxp,dvc_riflp,&srch_oflp, ebufp);
	PIN_FLIST_DESTROY_EX(&dvc_riflp, NULL);
	PIN_FLIST_DESTROY_EX(&rstcurstb_flp, NULL);
	if(PIN_ERRBUF_IS_ERR(ebufp)){
	    *r_flistpp = srch_oflp;
	     PIN_ERRBUF_CLEAR(ebufp);
	    return;
	}

	res_flp = PIN_FLIST_ELEM_GET(srch_oflp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
	sprintf(msg,"%s: Got the following details for %s",func_name, device_id);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg,res_flp);

	rstcurstb_flp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(res_flp, PIN_FLD_POID,rstcurstb_flp, PIN_FLD_POID, ebufp);
	
	/* Update /mso_catv_preactivated_svc with new device */
	wrt_flp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, wrt_flp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(wrt_flp, MSO_FLD_STB_OBJ, PIN_FLIST_FLD_GET(res_flp, PIN_FLD_POID, 0, ebufp), ebufp);
	sprintf(msg, "%s: PCM_OP_WRITE_FLDS input flist", func_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, wrt_flp);
	PCM_OP(ctxp,PCM_OP_WRITE_FLDS, 0, wrt_flp, &robj_oflp, ebufp);
	sprintf(msg, "%s: PCM_OP_WRITE_FLDS return flist", func_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, robj_oflp);

	/* Read /mso_catv_preactivated_svc object */
	sprintf(msg, "%s: PCM_OP_READ_OBJ input flist", func_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, wrt_flp);
	PCM_OP(ctxp,PCM_OP_READ_OBJ, 0, wrt_flp, &robj_oflp, ebufp);
	sprintf(msg, "%s: PCM_OP_READ_OBJ return flist", func_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, robj_oflp);
	PIN_FLIST_FLD_COPY(robj_oflp, MSO_FLD_VC_OBJ,rstcurstb_flp, MSO_FLD_VC_OBJ, ebufp);

	fm_mso_preatv_device_set_pairing(ctxp,rstcurstb_flp,r_flistpp, warranty_endp, ebufp);

	// TODO : Create Provisioning action
	provaction_inflistp = PIN_FLIST_COPY(i_flistp,ebufp);
	PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS,&preactv_flag, ebufp);

	manufacturer = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);

	taskelem_flp = PIN_FLIST_ELEM_ADD(provaction_inflistp, MSO_FLD_TASK, 1, ebufp);
	PIN_FLIST_FLD_COPY(res_flp,PIN_FLD_DEVICE_ID,taskelem_flp, MSO_FLD_STB_ID, ebufp);
	PIN_FLIST_FLD_COPY(res_flp,PIN_FLD_DEVICE_TYPE,taskelem_flp, PIN_FLD_DEVICE_TYPE, ebufp);
	PIN_FLIST_FLD_COPY(robj_oflp,MSO_FLD_REGION_KEY,taskelem_flp, MSO_FLD_REGION_KEY, ebufp);

	if((manufacturer != NULL) && strstr(manufacturer,"NDS")){
		PIN_FLIST_FLD_COPY(robj_oflp, MSO_FLD_CAS_SUBSCRIBER_ID, provaction_inflistp, MSO_FLD_CAS_SUBSCRIBER_ID, ebufp);
	}
	else if((manufacturer != NULL) && strstr(manufacturer,"CISCO")){
		PIN_FLIST_FLD_COPY(res_flp, PIN_FLD_DEVICE_ID, provaction_inflistp, MSO_FLD_STB_ID, ebufp);
	}	
	
	PIN_FLIST_ELEM_DROP(provaction_inflistp,PIN_FLD_PLAN_LISTS, 0, ebufp);
	PIN_FLIST_FLD_DROP(provaction_inflistp,PIN_FLD_OLD_VALUE, ebufp);
	PIN_FLIST_FLD_DROP(provaction_inflistp,PIN_FLD_NEW_VALUE, ebufp);
	//PIN_FLIST_FLD_DROP(provaction_inflistp,PIN_FLD_NEW_VALUE, ebufp);
	
	
	/* Now call MSO_OP_PROV_CREATE_ACTION */
	sprintf(msg,"%s: MSO_OP_PROV_CREATE_ACTION input flist",func_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg,provaction_inflistp);
	PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
        //PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_CREATE_ACTION ", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);

                *r_flistpp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51416", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Preactivated service - SWAP_STB - MSO_OP_PROV_CREATE_ACTION error", ebufp);

                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "provaction_outflistp return flist", provaction_outflistp);
	*r_flistpp = provaction_outflistp;
}


/*******************************************************************************
 * Sachid: fm_mso_preactv_swap
 *
 * Description:
 * Swap the old VC/STB with new VC/STB
 * Tasks
 * Update /mso_catv_preactivated_svc with new VC/STB poid
 * Update /device/stb and /device/vc objects
 * Create appropriate provisioning action 
 ******************************************************************************/
static void
fm_mso_preactv_swap(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **r_flistpp,
    char		    *device_type,
    pin_errbuf_t            *ebufp){

	pin_flist_t	*dvc_riflp = NULL;
	pin_flist_t	*dvc_roflp = NULL;
	pin_flist_t	*srch_iflp = NULL;
	pin_flist_t	*srch_oflp = NULL;

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
	pin_flist_t	*rstcurstb_flp= NULL;
	
	poid_t		*srch_pdp = NULL;
	poid_t		*acct_pdp = NULL;
	poid_t		*device_pdp = NULL;
	
	int64		database = -1;
	int			change_plan_failure = 1;
	int			srch_flag = SRCH_EXACT;
	int			args_cnt = 1;
	int			suspend_flag = MSO_PREACTIVATED_SVC_INACTIVE;
	int			active_flag = MSO_PREACTIVATED_SVC_ACTIVE;
	int			preactv_flag = CATV_PREACTV_MOD_SVC;
	int			res_cnt = 0;
	
	int			year;
	int			month;
	int			day;

	char		func_name[] = "fm_mso_preactv_swap";
	char		msg[1024];
	char		*ca_id = NULL;
	char		validity[10];
	char		*manufacturer = NULL;
	char		*device_id = NULL;	

	time_t		*warranty_endp;

   	if (PIN_ERRBUF_IS_ERR(ebufp)){
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_preactv_swap: Error before starting the function",ebufp);
        	return;
	}
	PIN_ERRBUF_CLEAR(ebufp);
	
  sprintf(msg,"%s: Input flist",func_name);
  PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, i_flistp);
  /* Input flist for swapping STB of preactivated service 
		0 PIN_FLD_POID           POID [0] 0.0.0.1 /account -1 1
		0 PIN_FLD_USERID         POID [0] 0.0.0.1 /account 80076 1
		0 PIN_FLD_ACTION_MODE    ENUM [0] 104
		0 PIN_FLD_REASON	     ENUM [0] <MSO_DEVICE_DEACTIVATION_REASON_FAULTY| Other>
		0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /mso_catv_preactivated_svc 628964 1
		0 PIN_FLD_PROGRAM_NAME    STR [0] "OAP|suresh001"
		0 PIN_FLD_NETWORK_NODE    STR [0] “NDS”
		0 PIN_FLD_OLD_VALUE       STR [0] "000007000013"
		0 PIN_FLD_NEW_VALUE       STR [0] "000033166978"
		0 PIN_FLD_PLAN_LISTS    ARRAY [0] allocated 1, used 1
  */
	device_id = (char*)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_OLD_VALUE, 0, ebufp);
	sprintf(msg,"%s: Prepare to reset pairing for %s",func_name,device_id);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);

    /* Search device poid using device id */
	sprintf(msg,"%s: Search %s poid using PIN_FLD_OLD_VALUE",func_name, device_type);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);

	dvc_riflp = PIN_FLIST_CREATE(ebufp);
	database = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_POID,0,ebufp));
	device_pdp = (poid_t*)PIN_POID_CREATE(database, device_type,-1, ebufp);
	PIN_FLIST_FLD_PUT(dvc_riflp, PIN_FLD_POID, device_pdp, ebufp);
	PIN_FLIST_FLD_SET(dvc_riflp, PIN_FLD_DEVICE_ID, device_id, ebufp);

	mso_get_device_details(ctxp,dvc_riflp,&srch_oflp, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp)){
	     PIN_ERRBUF_CLEAR(ebufp);
	    *r_flistpp = srch_oflp;
	    return;
	}
	PIN_FLIST_DESTROY_EX(&dvc_riflp, NULL);

	res_flp = PIN_FLIST_ELEM_GET(srch_oflp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
	sprintf(msg,"%s: Got the following details for %s",func_name, device_id);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg,res_flp);

	/* Get the warranty end date of the old device */
	warranty_endp = PIN_FLIST_FLD_GET(res_flp, MSO_FLD_WARRANTY_END, 0, ebufp);
	rstcurstb_flp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(res_flp, PIN_FLD_POID,rstcurstb_flp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(res_flp,PIN_FLD_STATE_ID, rstcurstb_flp, PIN_FLD_STATE_ID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_REASON, rstcurstb_flp, PIN_FLD_REASON, ebufp);



	/* TODO: 
		1. Reset pairing of the old stb with the associated VC
		2. Associate VC with the new STB
		3. Update the next device state for old and new STB
		4. Reset warranty for old device and update to new device
	*/
    fm_mso_preactv_device_reset_pairing(ctxp,rstcurstb_flp,r_flistpp,ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp)){
		PIN_ERRBUF_CLEAR(ebufp);
		return;
	}

	dvc_riflp = PIN_FLIST_CREATE(ebufp);
	database = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_POID,0,ebufp));
	device_pdp = (poid_t*)PIN_POID_CREATE(database, device_type,-1, ebufp);
	PIN_FLIST_FLD_PUT(dvc_riflp, PIN_FLD_POID, device_pdp, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_NEW_VALUE, dvc_riflp, PIN_FLD_DEVICE_ID, ebufp);

	mso_get_device_details(ctxp,dvc_riflp,&srch_oflp, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp)){
	     PIN_ERRBUF_CLEAR(ebufp);
	    *r_flistpp = srch_oflp;
	    return;
	}
	PIN_FLIST_DESTROY_EX(&dvc_riflp, NULL);
	PIN_FLIST_DESTROY_EX(&rstcurstb_flp, NULL);

	res_flp = PIN_FLIST_ELEM_GET(srch_oflp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
	sprintf(msg,"%s: Got the following details for %s",func_name, device_id);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg,res_flp);

	rstcurstb_flp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(res_flp, PIN_FLD_POID,rstcurstb_flp, PIN_FLD_POID, ebufp);
	
	/* Update /mso_catv_preactivated_svc with new device */
	wrt_flp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, wrt_flp, PIN_FLD_POID, ebufp);
	if(!strcmp(device_type, MSO_OBJ_TYPE_DEVICE_STB)){
		PIN_FLIST_FLD_SET(wrt_flp, MSO_FLD_STB_OBJ, PIN_FLIST_FLD_GET(res_flp, PIN_FLD_POID, 0, ebufp), ebufp);
	}
	else if(!strcmp(device_type, MSO_OBJ_TYPE_DEVICE_VC)){
		PIN_FLIST_FLD_SET(wrt_flp, MSO_FLD_VC_OBJ, PIN_FLIST_FLD_GET(res_flp, PIN_FLD_POID, 0, ebufp), ebufp);
	}
	sprintf(msg, "%s: PCM_OP_WRITE_FLDS input flist", func_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, wrt_flp);
	PCM_OP(ctxp,PCM_OP_WRITE_FLDS, 0, wrt_flp, &robj_oflp, ebufp);
	sprintf(msg, "%s: PCM_OP_WRITE_FLDS return flist", func_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, robj_oflp);

	/* Read /mso_catv_preactivated_svc object */
	sprintf(msg, "%s: PCM_OP_READ_OBJ input flist", func_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, wrt_flp);
	PCM_OP(ctxp,PCM_OP_READ_OBJ, 0, wrt_flp, &robj_oflp, ebufp);
    if(PIN_ERRBUF_IS_ERR(ebufp))
		return;

	sprintf(msg, "%s: PCM_OP_READ_OBJ return flist", func_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, robj_oflp);

	if(!strcmp(device_type, MSO_OBJ_TYPE_DEVICE_STB)){
		PIN_FLIST_FLD_COPY(robj_oflp, MSO_FLD_VC_OBJ,rstcurstb_flp, MSO_FLD_VC_OBJ, ebufp);
	}
	else if(!strcmp(device_type, MSO_OBJ_TYPE_DEVICE_VC)){
		PIN_FLIST_FLD_COPY(robj_oflp, MSO_FLD_STB_OBJ,rstcurstb_flp, MSO_FLD_STB_OBJ, ebufp);
	}


	fm_mso_preatv_device_set_pairing(ctxp,rstcurstb_flp,r_flistpp, warranty_endp, ebufp);
    	if(PIN_ERRBUF_IS_ERR(ebufp))
		return;

	// TODO : Create Provisioning action
	provaction_inflistp = PIN_FLIST_COPY(i_flistp,ebufp);
	PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS,&preactv_flag, ebufp);

	manufacturer = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);

	taskelem_flp = PIN_FLIST_ELEM_ADD(provaction_inflistp, MSO_FLD_TASK, 1, ebufp);
	PIN_FLIST_FLD_COPY(res_flp,PIN_FLD_DEVICE_ID,taskelem_flp, MSO_FLD_STB_ID, ebufp);
	PIN_FLIST_FLD_COPY(res_flp,PIN_FLD_DEVICE_TYPE,taskelem_flp, PIN_FLD_DEVICE_TYPE, ebufp);
	PIN_FLIST_FLD_COPY(robj_oflp,MSO_FLD_REGION_KEY,taskelem_flp, MSO_FLD_REGION_KEY, ebufp);

	if((manufacturer != NULL) && strstr(manufacturer,"NDS")){
		PIN_FLIST_FLD_COPY(robj_oflp, MSO_FLD_CAS_SUBSCRIBER_ID, provaction_inflistp, MSO_FLD_CAS_SUBSCRIBER_ID, ebufp);
	}
	else if((manufacturer != NULL) && strstr(manufacturer,"CISCO")){
		PIN_FLIST_FLD_COPY(res_flp, PIN_FLD_DEVICE_ID, provaction_inflistp, MSO_FLD_STB_ID, ebufp);
	}	
	
	PIN_FLIST_ELEM_DROP(provaction_inflistp,PIN_FLD_PLAN_LISTS, 0, ebufp);
	PIN_FLIST_FLD_DROP(provaction_inflistp,PIN_FLD_OLD_VALUE, ebufp);
	PIN_FLIST_FLD_DROP(provaction_inflistp,PIN_FLD_NEW_VALUE, ebufp);
	//PIN_FLIST_FLD_DROP(provaction_inflistp,PIN_FLD_NEW_VALUE, ebufp);
	
	
	/* Now call MSO_OP_PROV_CREATE_ACTION */
	sprintf(msg,"%s: MSO_OP_PROV_CREATE_ACTION input flist",func_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg,provaction_inflistp);
	PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
    //PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_CREATE_ACTION ", ebufp);
            PIN_ERRBUF_RESET(ebufp);
            PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);

            *r_flistpp = PIN_FLIST_CREATE(ebufp);
            PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp );
            PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
            PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51416", ebufp);
            PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Preactivated service - SWAP_STB - MSO_OP_PROV_CREATE_ACTION error", ebufp);

            return;
    }
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "provaction_outflistp return flist", provaction_outflistp);
	*r_flistpp = provaction_outflistp;
}



/******************************************************************
 * Function to get device details using its device id
 ******************************************************************/

 static void
	mso_get_device_details(
		pcm_context_t       *ctxp,
		pin_flist_t		*i_flistp,
		pin_flist_t		**r_flistpp,
		pin_errbuf_t         *ebufp)
	{
		pin_flist_t		*srch_iflp = NULL;
		pin_flist_t		*args_flistp = NULL;
		
		poid_t			*device_pdp = NULL;
		poid_t			*srch_pdp = NULL;

		int				srch_flag = SRCH_EXACT;
		int64			database = -1;
		int				args_cnt = 1;
		int				res_cnt = 0;
		int				change_plan_failure = 1;
		char			template [100];
		char			func_name[] = "mso_get_device_details";
		char			msg[1024];
		char			*device_type = NULL;

		if(PIN_ERRBUF_IS_ERR(ebufp)){
			sprintf(msg, "%s: Error before processing start", func_name);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG,msg, ebufp);
			return;
		}
		sprintf(msg, "%s: Input flist", func_name);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, i_flistp);

		srch_iflp = PIN_FLIST_CREATE(ebufp);	
		// Get the plan to add from PIN_FLD_PLAN_LISTS[1]
		device_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0 , ebufp);	
		device_type = (char*)PIN_POID_GET_TYPE(device_pdp);

		// Get the DB
		database = PIN_POID_GET_DB(device_pdp);

		// Create /search/pin poid
		srch_pdp = (poid_t*)PIN_POID_CREATE(database, "/search/pin",-1, ebufp);

		PIN_FLIST_FLD_PUT(srch_iflp, PIN_FLD_POID, srch_pdp, ebufp);
		sprintf(template, " select X from %s where F1 = V1 ",device_type);
		PIN_FLIST_FLD_SET(srch_iflp, PIN_FLD_TEMPLATE, template, ebufp);
		PIN_FLIST_FLD_SET(srch_iflp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	
		// Add ARGS array
		args_flistp = PIN_FLIST_ELEM_ADD(srch_iflp, PIN_FLD_ARGS, args_cnt, ebufp);
 		PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_DEVICE_ID,args_flistp,PIN_FLD_DEVICE_ID,ebufp); 

		//Add result
		PIN_FLIST_ELEM_ADD(srch_iflp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	
		sprintf(msg, "%s: PCM_OP_SEARCH input flist", func_name);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, srch_iflp);

		PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflp, r_flistpp, ebufp);
		PIN_FLIST_DESTROY_EX(&srch_iflp, NULL);
		//if(PIN_ERRBUF_IS_ERR(ebufp)){
			//sprintf(msg, "%s: PCM_OP_SEARCH error", func_name);
			//PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,msg, ebufp);
			//return;
		//}

		sprintf(msg, "%s: PCM_OP_SEARCH return flist", func_name);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, *r_flistpp);

		//sprintf(msg, "%s: PCM_OP_SEARCH before count:");
		//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
		PIN_FLIST_ELEM_GET(*r_flistpp, PIN_FLD_RESULTS,PIN_ELEMID_ANY,0, ebufp);
		//sprintf(msg, "%s: PCM_OP_SEARCH count: %d", res_cnt);
		//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH, PIN_FLD_NEW_VALUE not found ", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&srch_iflp, NULL);

			*r_flistpp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51416", ebufp);
			sprintf(msg,"Preactivated service - Device Swap - Wrong Device ID: %s",PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_DEVICE_ID,0,ebufp));
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR,msg , ebufp);

			pin_set_err(ebufp, PIN_ERRLOC_DM, PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_NOT_FOUND, 0, 0, MSO_OP_PREACTVN_MODIFY_SRVC);
			return;
		}
		//sprintf(msg, "%s: PCM_OP_SEARCH count: %d", res_cnt);
		//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
 }

/*******************************************************************
 * Function used to remove pairing of the device
 *******************************************************************/
static void
    fm_mso_preactv_device_reset_pairing(
    pcm_context_t       *ctxp,
    pin_flist_t         *i_flistp,
    pin_flist_t         **r_flistpp,
    pin_errbuf_t         *ebufp)
{

    pin_flist_t         *read_iflistp = NULL;
    pin_flist_t         *read_oflistp = NULL;
    pin_flist_t         *write_iflisp = NULL;
    pin_flist_t         *write_oflisp = NULL;
    pin_status_t        *service_statep = NULL;
    poid_t              *service_pdp = NULL;      /* service poid */
    poid_t              *device_pdp = NULL;
    poid_t              *device_pidp = NULL;
    poid_t              *s_pdp = NULL;

    int32               s_flags;
    int					*db_state_id = NULL;
    int32               *reason = NULL;
    int32               next_state = 0;
	int64               database;
    char                *template = NULL;
    char                *device_type = NULL;
	char				msg[1024];
	char				func_name[] = "fm_mso_preactv_device_reset_pairing";
    int32               count = 0;
	int					change_plan_failure = 1;
	time_t				no_warranty = 0;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		sprintf(msg,"%s: Error before processing:",func_name);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG, msg, ebufp);
        return;
    }

	PIN_ERRBUF_CLEAR(ebufp);
	
	sprintf(msg,"%s: Input flist:",func_name);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg, i_flistp);
	/* Input flist
		0 PIN_FLD_POID           POID [0] 0.0.0.1 /device/stb 1234234 0
		0 PIN_FLD_REASON	     ENUM [0] <MSO_DEVICE_DEACTIVATION_REASON_FAULTY| Other>
		0 PIN_FLD_STATE_ID	     INT  [0] 2

	*/


    device_pidp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
    device_type = (char *) PIN_POID_GET_TYPE (device_pidp);
	reason = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_REASON, 0, ebufp);
	db_state_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_STATE_ID, 0, ebufp);

	if(!strcmp(device_type,MSO_OBJ_TYPE_DEVICE_STB)){
		/* Drop the MSO_FLD_STB_DATA to reset pairing, the VC pairing will reset 
		   when we will overwrite the new STB in /device/vc */
			write_iflisp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, write_iflisp, PIN_FLD_POID, ebufp);
			PIN_FLIST_ELEM_SET(write_iflisp, NULL, MSO_FLD_STB_DATA, PIN_ELEMID_ANY,ebufp);
			sprintf(msg,"%s:  PCM_OP_DELETE_FLDS input flist:",func_name);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg, write_iflisp);
			PCM_OP(ctxp, PCM_OP_DELETE_FLDS, 0, write_iflisp, &write_oflisp, ebufp);
            //PIN_FLIST_DESTROY_EX(&write_iflisp, NULL);

	        if (PIN_ERRBUF_IS_ERR(ebufp))
		    {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_DELETE_FLDS ", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_DESTROY_EX(&write_oflisp, NULL);

                *r_flistpp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51416", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Preactivated service - Change Plan - MSO_OP_PROV_CREATE_ACTION error", ebufp);

                return;
			}
			sprintf(msg,"%s:  PCM_OP_DELETE_FLDS return flist:",func_name);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg, write_oflisp);
			PIN_FLIST_DESTROY_EX(&write_oflisp, NULL);

			PIN_FLIST_ELEM_DROP(write_iflisp, MSO_FLD_STB_DATA, PIN_ELEMID_ANY,ebufp);
			/* Reset warranty end date for old device */
			PIN_FLIST_FLD_SET(write_iflisp, MSO_FLD_WARRANTY_END, &no_warranty, ebufp);
			sprintf(msg,"%s:  PCM_OP_WRITE_FLDS input flist to reset warranty of old device:",func_name);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg, write_iflisp);
			PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_iflisp, &write_oflisp, ebufp);
			sprintf(msg,"%s:  PCM_OP_WRITE_FLDS return flist",func_name);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg, write_oflisp);
			
			/* Now change device state*/
			switch (*db_state_id) {
				case MSO_STB_STATE_PREACTIVE:
				case MSO_STB_STATE_ALLOCATED:
		                    if ( *reason == MSO_DEVICE_DEACTIVATION_REASON_FAULTY){
					sprintf(msg, "%s: Updating device state from MSO_STB_STATE_ALLOCATED to MSO_STB_STATE_FAULTY", func_name);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
					next_state = MSO_STB_STATE_FAULTY;
				     }
		                    // Complete deactivation of services & disassociaton of temporary devices
				     else{
					sprintf(msg, "%s: Updating device state from MSO_STB_STATE_ALLOCATED to MSO_STB_STATE_GOOD", func_name);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
		                    	next_state = MSO_STB_STATE_GOOD;
				     }

				   PIN_FLIST_FLD_SET(write_iflisp, PIN_FLD_NEW_STATE,(void *)&next_state, ebufp);
		                   PCM_OP(ctxp, PCM_OP_DEVICE_SET_STATE, 0,write_iflisp, &write_oflisp, ebufp);

		                   PIN_FLIST_DESTROY_EX(&write_iflisp, NULL);
				   PIN_FLIST_DESTROY_EX(&write_oflisp, NULL);
					break;
				default:
				sprintf(msg, "%s: Invalid device state", func_name);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, msg);

	            PIN_ERRBUF_CLEAR(ebufp);
                
                *r_flistpp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51416", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Error: Invalid old device state", ebufp);
                PIN_FLIST_DESTROY_EX(&write_iflisp, NULL);
		        PIN_FLIST_DESTROY_EX(&write_oflisp, NULL);
				
				
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                    PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    PIN_ERR_BAD_VALUE, PIN_FLD_STATE_ID,
                    0, *db_state_id);

                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                    "fm_mso_device_catv_disassociate invalid "
                    "state_id for disassociation of services",
                    ebufp);
				
			}

			return;
	}
	else if(!strcmp(device_type,MSO_OBJ_TYPE_DEVICE_VC)){
		/* Drop the MSO_FLD_VC_DATA to reset pairing, the VC pairing will reset 
		   when we will overwrite the new STB in /device/vc */
			write_iflisp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, write_iflisp, PIN_FLD_POID, ebufp);
			PIN_FLIST_ELEM_SET(write_iflisp,NULL, MSO_FLD_VC_DATA, PIN_ELEMID_ANY,ebufp);
			sprintf(msg,"%s:  PCM_OP_DELETE_FLDS input flist:",func_name);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg, write_iflisp);
			PCM_OP(ctxp, PCM_OP_DELETE_FLDS, 0, write_iflisp, &write_oflisp, ebufp);
            //PIN_FLIST_DESTROY_EX(&write_iflisp, NULL);

	        if (PIN_ERRBUF_IS_ERR(ebufp))
		    {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_DELETE_FLDS ", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                //PIN_FLIST_DESTROY_EX(&write_oflisp, NULL);

                *r_flistpp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51416", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Preactivated service - Change Plan - MSO_OP_PROV_CREATE_ACTION error", ebufp);

                return;
			}
			sprintf(msg,"%s:  PCM_OP_DELETE_FLDS return flist:",func_name);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg, write_oflisp);

			PIN_FLIST_ELEM_DROP(write_iflisp, MSO_FLD_VC_DATA, PIN_ELEMID_ANY,ebufp);
			/* Reset warranty end date for old device */
			PIN_FLIST_FLD_SET(write_iflisp, MSO_FLD_WARRANTY_END, &no_warranty, ebufp);
			sprintf(msg,"%s:  PCM_OP_WRITE_FLDS input flist to reset warranty of old device:",func_name);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg, write_iflisp);
			PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_iflisp, &write_oflisp, ebufp);
			sprintf(msg,"%s:  PCM_OP_WRITE_FLDS return flist",func_name);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg, write_oflisp);

			/* Now change device state*/
			switch (*db_state_id) {
				case MSO_VC_STATE_PREACTIVE:
				case MSO_VC_STATE_ALLOCATED:
		                if ( *reason == MSO_DEVICE_DEACTIVATION_REASON_FAULTY){
							sprintf(msg, "%s: Updating device state from MSO_VC_STATE_ALLOCATED to MSO_VC_STATE_FAULTY", func_name);
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
							next_state = MSO_VC_STATE_FAULTY;
						 }
		                // Complete deactivation of services & disassociaton of temporary devices
				        else{
							sprintf(msg, "%s: Updating device state from MSO_VC_STATE_ALLOCATED to MSO_VC_STATE_GOOD", func_name);
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
		                    next_state = MSO_VC_STATE_GOOD;
				        }

						PIN_FLIST_FLD_SET(write_iflisp, PIN_FLD_NEW_STATE,(void *)&next_state, ebufp);
		                PCM_OP(ctxp, PCM_OP_DEVICE_SET_STATE, 0,write_iflisp, &write_oflisp, ebufp);
		                PIN_FLIST_DESTROY_EX(&write_iflisp, NULL);
				        PIN_FLIST_DESTROY_EX(&write_oflisp, NULL);
						break;
				default:
						sprintf(msg, "%s: Invalid device state", func_name);
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, msg);
				        PIN_ERRBUF_CLEAR(ebufp);
				    
						*r_flistpp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp );
				        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
			            PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51416", ebufp);
				        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Error: Invalid old device state", ebufp);
					    PIN_FLIST_DESTROY_EX(&write_iflisp, NULL);
						PIN_FLIST_DESTROY_EX(&write_oflisp, NULL);

					    pin_set_err(ebufp, PIN_ERRLOC_FM,
		                PIN_ERRCLASS_SYSTEM_DETERMINATE,
			            PIN_ERR_BAD_VALUE, PIN_FLD_STATE_ID,
				        0, *db_state_id);

						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			            "fm_mso_device_catv_disassociate invalid "
				        "state_id for disassociation of services",
					    ebufp);
		                PIN_FLIST_DESTROY_EX(&write_iflisp, NULL);
				        PIN_FLIST_DESTROY_EX(&write_oflisp, NULL);

				return;
			}


			return;
	}
		
return;
}


/*******************************************************************
 * Function used to set the pairing of the device for preactivated service
 *******************************************************************/
static void
    fm_mso_preatv_device_set_pairing(
    pcm_context_t       *ctxp,
    pin_flist_t         *i_flistp,
    pin_flist_t         **r_flistpp,
    time_t		*warranty_endp,
    pin_errbuf_t         *ebufp)
{

    pin_flist_t         *robj_flp = NULL;
    pin_flist_t         *w_in_flistp = NULL;
    pin_flist_t         *w_o_flistp = NULL;
    pin_flist_t         *tmp_flistp = NULL;
    pin_flist_t         *sub_flistp = NULL;
    pin_flist_t         *dssi_flistp = NULL;
    pin_flist_t         *dsso_flistp = NULL;
    pin_status_t        *service_statep = NULL;
    poid_t              *service_pdp = NULL;      /* service poid */
    poid_t              *device_pdp = NULL;
    poid_t              *device_pidp = NULL;
    poid_t              *s_pdp = NULL;
    int32               s_flags;
    int32               next_state = 0;
    int32		err_code = 51413;
    time_t              now;
    int64               database;
    char                *template = NULL;
    char                *device_type = NULL;
    char		func_name[] = "fm_mso_preatv_device_set_pairing";
    char		msg[1024];
    int32               count = 0;
    int32               rec_id = 0;
    int32		*db_state_id = NULL;
    pin_cookie_t        rec_id_cookie = NULL;

    if (PIN_ERRBUF_IS_ERR(ebufp)) {
		sprintf(msg, "%s: Error before processing", func_name);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG, msg, ebufp);
        return;
    }

		sprintf(msg, "%s: Input flist", func_name);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg, i_flistp);
		/* input flist for STB swap
			0 PIN_FLD_POID           POID [0] 0.0.0.1 /device/stb 130057 6
			0 MSO_FLD_VC_OBJ         POID [0] 0.0.0.1 /device/vc 269820 1
			
			input flist for VC swap
			0 PIN_FLD_POID           POID [0] 0.0.0.1 /device/vc 269820 1
			0 MSO_FLD_VC_OBJ         POID [0] 0.0.0.1 /device/stb 130057 6
		*/

        /*
        * Update the pairing detail for
        * existing device as well as current device
        */

		/* Read /device object details */
		sprintf(msg, "%s: Read /device details using input flist", func_name);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg, i_flistp);
		PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, i_flistp, &robj_flp, ebufp);
		sprintf(msg, "%s: Read /device details return flist", func_name);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg, robj_flp);

		device_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
		device_type = (char*)PIN_POID_GET_TYPE(device_pdp);

		db_state_id = PIN_FLIST_FLD_GET(robj_flp, PIN_FLD_STATE_ID, 0, ebufp);
        if (!strcmp(device_type,MSO_OBJ_TYPE_DEVICE_STB) )
        {
			w_in_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, w_in_flistp, PIN_FLD_POID, ebufp);
			sub_flistp = PIN_FLIST_ELEM_ADD (w_in_flistp, MSO_FLD_STB_DATA, 0, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_VC_OBJ, sub_flistp, MSO_FLD_VC_OBJ,  ebufp);

			/* Update device state */
			switch (*db_state_id)
			{

				case MSO_STB_STATE_GOOD:
						sprintf(msg, "%s: Updating device state from MSO_STB_STATE_GOOD to MSO_STB_STATE_ALLOCATED", func_name);
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
				case MSO_STB_STATE_PREACTIVE:
						sprintf(msg, "%s: Updating device state from MSO_STB_STATE_PREACTIVE to MSO_STB_STATE_ALLOCATED", func_name);
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
						dssi_flistp = PIN_FLIST_CREATE(ebufp);

						PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, dssi_flistp, PIN_FLD_POID, ebufp);

						next_state = MSO_STB_STATE_ALLOCATED;
						PIN_FLIST_FLD_SET(dssi_flistp, PIN_FLD_NEW_STATE,(void *)&next_state, ebufp);
						PCM_OP(ctxp, PCM_OP_DEVICE_SET_STATE, 0,dssi_flistp, &dsso_flistp, ebufp);
						PIN_FLIST_DESTROY_EX(&dssi_flistp, NULL);
						PIN_FLIST_DESTROY_EX(&dsso_flistp, NULL);
						break;
				case MSO_STB_STATE_ALLOCATED:
						sprintf(msg, "%s: Device already in MSO_STB_STATE_ALLOCATED state", func_name);
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
						/*
				         * The device is already in Assigned state; do nothing
						 */
		            break;
				default:
					sprintf(msg, "%s: Invalid device state", func_name);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, msg);
					*r_flistpp = PIN_FLIST_CREATE(ebufp);
                     			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, device_pdp, ebufp );
                     			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &err_code, ebufp);
                     			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "51413", ebufp);
                     			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, 
						"Invalid device state", ebufp);

					pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_BAD_VALUE,
					PIN_FLD_STATE_ID, 0, *db_state_id);

					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"fm_mso_device_catv_associate unknown state_id "
					"read for this device",    ebufp);
					return;
			}

        }
        else if(!strcmp(device_type,MSO_OBJ_TYPE_DEVICE_VC))
        {
			w_in_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, w_in_flistp, PIN_FLD_POID, ebufp);
			sub_flistp = PIN_FLIST_ELEM_ADD (w_in_flistp, MSO_FLD_VC_DATA, 0, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_STB_OBJ, sub_flistp, MSO_FLD_STB_OBJ, ebufp);

			/* Set device state */
			switch (*db_state_id) {

				case MSO_VC_STATE_GOOD:
					sprintf(msg, "%s: Updating device state from MSO_VC_STATE_GOOD to MSO_VC_STATE_ALLOCATED", func_name);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
				case MSO_VC_STATE_PREACTIVE:
					sprintf(msg, "%s: Updating device state from MSO_VC_STATE_PREACTIVE to MSO_VC_STATE_ALLOCATED", func_name);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
					dssi_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, dssi_flistp, PIN_FLD_POID, ebufp);
					next_state = MSO_VC_STATE_ALLOCATED;
					PIN_FLIST_FLD_SET(dssi_flistp, PIN_FLD_NEW_STATE,(void *)&next_state, ebufp);
					PCM_OP(ctxp, PCM_OP_DEVICE_SET_STATE, 0,dssi_flistp, &dsso_flistp, ebufp);
					PIN_FLIST_DESTROY_EX(&dssi_flistp, NULL);
					PIN_FLIST_DESTROY_EX(&dsso_flistp, NULL);
					break;
				case MSO_VC_STATE_ALLOCATED:
					sprintf(msg, "%s: Device already in MSO_VC_STATE_ALLOCATED state", func_name);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
					/*
					* The device is already in Assigned state; do nothing
   					      */
					break;
				default:
					sprintf(msg, "%s: Invalid device state", func_name);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, msg);
					*r_flistpp = PIN_FLIST_CREATE(ebufp);
                     			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, device_pdp, ebufp );
                     			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &err_code, ebufp);
                     			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "51413", ebufp);
                     			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, 
						"Invalid device state", ebufp);
					pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_BAD_VALUE,
					PIN_FLD_STATE_ID, 0, *db_state_id);

					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"fm_mso_device_catv_associate unknown state_id "
					"read for this device",    ebufp);
					return;
			}

        }

	/* Reset warranty end date for old device */
	PIN_FLIST_FLD_SET(w_in_flistp, MSO_FLD_WARRANTY_END, warranty_endp, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "fm_mso_preatv_device_set_pairing PCM_OP_WRITE_FLDS input flist", w_in_flistp);
         /*
          * Update the pairing detail for existing device
          */
        PCM_OP(ctxp, PCM_OP_WRITE_FLDS, PCM_OPFLG_CACHEABLE, w_in_flistp,&w_o_flistp, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "fm_mso_preatv_device_set_pairing PCM_OP_WRITE_FLDS return flist", w_in_flistp);
       PIN_FLIST_DESTROY_EX(&w_o_flistp, NULL);
       PIN_FLIST_DESTROY_EX(&w_in_flistp, NULL);

		/* Now updated the correspoding STB/VC with current device */
        w_in_flistp = PIN_FLIST_CREATE(ebufp);
        if (!strcmp(device_type,MSO_OBJ_TYPE_DEVICE_STB) )
        {
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_VC_OBJ, w_in_flistp, PIN_FLD_POID, ebufp);
			/* Check if MSO_FLD_STB_DATA is already there then replace else add */
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"fm_mso_preatv_device_set_pairing PCM_OP_READ_OBJ input flist", w_in_flistp);
			PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, w_in_flistp, &w_o_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"fm_mso_preatv_device_set_pairing PCM_OP_READ_OBJ return flist", w_o_flistp);
			/*
			sub_flistp = PIN_FLIST_ELEM_GET(w_o_flistp, MSO_FLD_VC_DATA, 0, 1, ebufp);
			if(sub_flistp != NULL){
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"fm_mso_preatv_device_set_pairing found MSO_FLD_VC_DATA ", sub_flistp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, sub_flistp, MSO_FLD_STB_OBJ, ebufp);
			}
			else{
				*/
				sub_flistp = PIN_FLIST_ELEM_ADD (w_in_flistp, MSO_FLD_VC_DATA, 0, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, sub_flistp, MSO_FLD_STB_OBJ, ebufp);
			//}			
        }
        else if(!strcmp(device_type,MSO_OBJ_TYPE_DEVICE_VC))
        {
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_STB_OBJ, w_in_flistp, PIN_FLD_POID, ebufp);
			/* Check if MSO_FLD_VC_DATA is already there then replace else add */
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"fm_mso_preatv_device_set_pairing PCM_OP_READ_OBJ input flist", w_in_flistp);
			PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, w_in_flistp, &w_o_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"fm_mso_preatv_device_set_pairing PCM_OP_READ_OBJ return flist", w_o_flistp);
			/*
			sub_flistp = PIN_FLIST_ELEM_GET(w_o_flistp, MSO_FLD_STB_DATA, 0, 1, ebufp);
			if(sub_flistp != NULL){
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, sub_flistp, MSO_FLD_VC_OBJ, ebufp);
			}
			else{
				*/
				sub_flistp = PIN_FLIST_ELEM_ADD (w_in_flistp, MSO_FLD_STB_DATA, 0, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, sub_flistp, MSO_FLD_VC_OBJ, ebufp);
			//}
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "fm_mso_device_set_pairing write_flds for current device  input flist", w_in_flistp);
         /*
          * Update the pairing detail for current device
          */
        PCM_OP(ctxp, PCM_OP_WRITE_FLDS, PCM_OPFLG_CACHEABLE, w_in_flistp,
            &w_o_flistp, ebufp);

     PIN_FLIST_DESTROY_EX(&w_in_flistp, NULL);
     PIN_FLIST_DESTROY_EX(&w_o_flistp, NULL);

return;
}
