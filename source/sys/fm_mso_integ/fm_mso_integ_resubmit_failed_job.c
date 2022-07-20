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
* 0.1    |21/04/2014 |Phani Jandhyala         |  Bulk Resubmit Job
* 0.2    |13/02/2020 |Jyothirmayi Kachiraju   |  Changes to include the Termination reason type, subtype 
*						 and remarks for CATV Service Termination resubmission cases
*--------------------------------------------------------------------------------------------------------------*
*
****************************************************************************************************************/


#ifndef lint
static  char Sccs_Id[] = "@(#)$Id: fm_mso_integ_resubmit_failed_job.c /cgbubrm_7.3.2.rwsmod/1 2009/08/04 23:37:19 pjegan Exp $";
#endif

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
#include "cm_cache.h"
#include "mso_bulk_load.h"
#include "ops/device.h"
#define CALLED_BY_OAP  0
#define CALLED_BY_MTA  1


/*******************************************************************
 * Functions Defined outside this source file
 *******************************************************************/
extern int32
fm_mso_trans_open(
    pcm_context_t       *ctxp,
    poid_t              *pdp,
    int32               flag,
    pin_errbuf_t        *ebufp);

extern void
fm_mso_trans_commit(
    pcm_context_t       *ctxp,
    poid_t              *pdp,
    pin_errbuf_t        *ebufp);

extern void
fm_mso_trans_abort(
    pcm_context_t       *ctxp,
    poid_t              *pdp,
    pin_errbuf_t        *ebufp);

extern void
fm_mso_search_device_object(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_errbuf_t	*ebufp);

extern void
fm_mso_search_lco_account(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_errbuf_t	*ebufp);

extern void 
fm_mso_integ_create_add_plan_job(
	pcm_context_t	*ctxp,
	char			*task_id,
	poid_t			*task_pdp,
	pin_flist_t		*task_flistp,
	pin_errbuf_t	*ebufp);

extern void 
fm_mso_integ_create_cancel_plan_job(
	pcm_context_t	*ctxp,
	char			*task_id,
	poid_t			*task_pdp,
	pin_flist_t		*task_flistp,
	pin_errbuf_t	*ebufp);

extern void 
fm_mso_integ_create_change_plan_job(
	pcm_context_t	*ctxp,
	char			*task_id,
	poid_t			*task_pdp,
	pin_flist_t		*task_flistp,
	pin_errbuf_t	*ebufp);

extern void
fm_mso_integ_create_state_change_job(
	pcm_context_t	*ctxp,
	char			*task_no,
	poid_t			*task_pdp,
	pin_flist_t		*task_flistp,
	pin_errbuf_t	*ebufp);

extern void
fm_mso_integ_create_location_update_job(
	pcm_context_t	*ctxp,
	char			*task_no,
	poid_t			*task_pdp,
	pin_flist_t		*task_flistp,
	pin_errbuf_t	*ebufp);

extern void
fm_mso_integ_create_catv_preactivation_job(
	pcm_context_t	*ctxp,
	char			*task_no,
	poid_t			*task_pdp,
	pin_flist_t		*task_flistp,
	pin_errbuf_t	*ebufp);

extern void
fm_mso_integ_create_service_actions_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

extern void
fm_mso_integ_create_retrack_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

extern void
fm_mso_integ_create_adjustment_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

extern void
fm_mso_integ_create_bmail_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

extern void
fm_mso_integ_create_osd_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

extern void
fm_mso_integ_set_bulk_crf(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

extern void
fm_mso_integ_create_hierarchy_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	int32		caller,
	pin_errbuf_t	*ebufp);

extern void
fm_mso_integ_create_ippool_update_job(
        pcm_context_t   *ctxp,
        char                    *task_no,
        poid_t                  *task_pdp,
        pin_flist_t             *task_flistp,
        pin_errbuf_t    *ebufp);

extern void 
fm_mso_integ_topup_create_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

extern void 
fm_mso_integ_renew_create_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

extern void 
fm_mso_integ_add_mb_gb_create_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

extern void 
fm_mso_integ_extend_validity_create_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

extern void 
fm_mso_integ_bill_hold_create_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

extern void 
fm_mso_integ_swaping_cpe_create_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

extern void 
fm_mso_integ_cpe_writeoff_create_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

extern void 
fm_mso_integ_cheque_bounce_create_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);
extern void 
fm_mso_integ_refund_load_create_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

extern void 
fm_mso_integ_refund_reversal_load_create_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

extern void
fm_mso_integ_create_attribute_update_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

extern void
fm_mso_integ_create_grv_uploader_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

extern void
fm_mso_integ_create_crdr_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

extern int
fm_mso_integ_match_patterns(
	char* pattern_string,
	char* input_string);

/*******************************************************************
 * Fuctions defined in this code
 *******************************************************************/

EXPORT_OP void
op_mso_integ_resubmit_failed_job(
        cm_nap_connection_t    *connp,
        int32               opcode,
        int32               flags,
        pin_flist_t         *in_flistp,
        pin_flist_t         **ret_flistpp,
        pin_errbuf_t        *ebufp);

static void
fm_mso_integ_resubmit_failed_job(
        pcm_context_t       *ctxp,
        pin_flist_t         *in_flistp,
        pin_flist_t         **out_flistpp,
        pin_errbuf_t        *ebufp);

PIN_IMPORT void fm_mso_create_update_gst_info_jobs(
        pcm_context_t   *ctxp,
        char            *task_no,
        poid_t          *task_pdp,
        poid_t          *user_id,
        char            *program_name,
        pin_flist_t     *in_flistp,
        pin_errbuf_t    *ebufp);

PIN_IMPORT void fm_mso_create_modify_gst_info_jobs(
        pcm_context_t   *ctxp,
        char            *task_no,
        poid_t          *task_pdp,
        poid_t          *user_id,
        char            *program_name,
        pin_flist_t     *in_flistp,
        pin_errbuf_t    *ebufp);


/*******************************************************************
 * Main routine for the MSO_OP_PROV_RESUBMIT_FAILED_ORDER command
 *******************************************************************/
void
op_mso_integ_resubmit_failed_job(
        cm_nap_connection_t    *connp,
        int32               opcode,
        int32               flags,
        pin_flist_t         *in_flistp,
        pin_flist_t         **ret_flistpp,
        pin_errbuf_t        *ebufp)
{
    pin_flist_t             *r_flistp = NULL;
    pcm_context_t           *ctxp = connp->dm_ctx;
    poid_t                  *pdp = NULL;
    int32                   t_status;
    int32                   oap_failure = 1;
    int32                   oap_success = 0;

    /***********************************************************
     * Null out results until we have some.
     ***********************************************************/
    *ret_flistpp = NULL;
    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Insanity check.
     ***********************************************************/
    if (opcode != MSO_OP_INTEG_RESUBMIT_FAILED_JOB) {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_OPCODE, 0, 0, opcode);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "bad opcode in op_mso_integ_resubmit_failed_job", ebufp);
        return;
	}

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "op_mso_integ_resubmit_failed_job input flist", in_flistp);

    pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
    t_status = fm_mso_trans_open(ctxp, pdp, 1, ebufp);

    /***********************************************************
     * Call main function to do it
     ***********************************************************/
    fm_mso_integ_resubmit_failed_job(ctxp, in_flistp,&r_flistp, ebufp);

    /***********************************************************
     * Results.
     ***********************************************************/
    *ret_flistpp = PIN_FLIST_CREATE(ebufp);
    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_mso_integ_resubmit_failed_job error", ebufp);

        PIN_ERRBUF_CLEAR(ebufp);

        if (t_status == 0)
        {
            fm_mso_trans_abort(ctxp, pdp, ebufp);
        }

        *ret_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "Fail", ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "Order Resubmission Failed", ebufp);
    }
    else
    {
        if (t_status == 0)
        {
            fm_mso_trans_commit(ctxp, pdp, ebufp);
        }
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, &oap_success, ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "Success", ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "Order Successfully Resubmitted", ebufp);
        PIN_ERRBUF_CLEAR(ebufp);
    }

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "op_mso_integ_resubmit_failed_job return flist", *ret_flistpp);
    PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

    return;
}

/*******************************************************************
 * fm_mso_integ_resubmit_failed_job()
 *
 * Perfomrs following action:
 * 1.
 *******************************************************************/

static void
fm_mso_integ_resubmit_failed_job(
    pcm_context_t       *ctxp,
    pin_flist_t         *in_flistp,
    pin_flist_t         **out_flistpp,
    pin_errbuf_t        *ebufp)
{
    	pin_flist_t         	*plan_flist = NULL;
   	pin_flist_t         	*read_inflist = NULL;
    	pin_flist_t         	*read_oflist = NULL;
    	pin_flist_t         	*write_iflistp = NULL;
    	pin_flist_t         	*write_oflistp = NULL;
    	pin_flist_t         	*job_iflistp = NULL;
    	pin_flist_t         	*job_oflistp = NULL;
    	pin_flist_t        	*task_flistp= NULL;
	pin_flist_t	    	*read_oflistp = NULL;
	pin_flist_t		*plan_flist_old = NULL;
	pin_flist_t		*plan_flist_new = NULL;
	pin_flist_t	        *nameinfo_inst=NULL;
	pin_flist_t 		*nameinfo_bill=NULL;
	
    	poid_t      		*job_pdp = NULL;
	poid_t			*failed_task_obj = NULL;
	poid_t			*failed_job_poid = NULL;
	poid_t			*order_poidp = NULL;
	poid_t			*device_pdp = NULL;
	poid_t			*lco_account_pdp = NULL;
	poid_t			*service_pdp = NULL;
	poid_t			*task_pdp = NULL;
	poid_t			*user_id = NULL;
	poid_t			*user_pd = NULL;
    
	int			flistlen = 0;
	int64       		db;     
	int32			*state_id = NULL;
    	int32			*flags = NULL;
	int32			status = 0;
	int32			status_uncommit = 3;
	int32			order_status_success = 0;
	int32			status_sucess = 1;
	int32			status_fail = 2;
	int32			add_plan_flags = 2;
	int32			service_type = 1;
	int32          		*callerp = NULL;
	int32          		caller = CALLED_BY_OAP;
	
    	pin_buf_t		*nota_buf  = NULL;
	
	char			*flistbuf = NULL;
    	char			*order_id = NULL; 
	char			*task_poid_type = NULL;
    	char			*order_po_no = NULL;
	char			*failed_task_id = NULL;
	char			*device_id	= NULL;
	char			*account_no = NULL;
	char			*plan_name	= NULL;
	char			*old_plan_name = NULL;
	char			*new_plan_name = NULL;
	char			*agreement_no = NULL;
	char			*network_node = NULL;
	char        		*inst_address = NULL;
	char        		*bill_address = NULL;
	char			*task_name = NULL;
	char			msg[256];
	char			*task_no = NULL;
	char			*program_name = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
	  ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_integ_resubmit_failed_job input flist", in_flistp);

    /***********************************************************
     * Check mandatory fields are available
     ***********************************************************/
	
    failed_task_obj = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ORDER_OBJ, 1, ebufp);
		
	failed_job_poid = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1, ebufp);
	db = PIN_POID_GET_DB(failed_job_poid);
	failed_task_id	= PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ORDER_ID, 1, ebufp);
	flags = PIN_FLIST_FLD_GET(in_flistp , PIN_FLD_FLAGS, 1, ebufp);
	network_node = PIN_FLIST_FLD_GET(in_flistp ,MSO_FLD_NETWORK_NODE, 1, ebufp);
	

	/******* 
	 * read the task object to get the order object 
	 ******/
	
	 read_inflist = PIN_FLIST_CREATE(ebufp);
	 
	 PIN_FLIST_FLD_SET(read_inflist, PIN_FLD_POID, failed_task_obj, ebufp);	 
	 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , "fm_mso_integ_resubmit_failed_job  read job poid input flist", read_inflist);
	 PCM_OP(ctxp, PCM_OP_READ_OBJ , 0, read_inflist , &read_oflist , ebufp);
	 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , "fm_mso_integ_resubmit_failed_job  read job poid output flist", read_oflist);
	

	 if(PIN_ERRBUF_IS_ERR(ebufp))
	 {
		 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " error in reading the object");
		 PIN_ERRBUF_CLEAR(ebufp);
		 PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
	 }
	 if(read_oflist)
	{
		 order_poidp = PIN_FLIST_FLD_GET(read_oflist, MSO_FLD_ORDER_OBJ, 1 , ebufp);
		 /***** 
		  * Read the order poid to get the order number 
		  *****/
		  PIN_FLIST_FLD_SET(read_inflist, PIN_FLD_POID, order_poidp, ebufp);
		  PCM_OP(ctxp, PCM_OP_READ_OBJ , 0, read_inflist , &read_oflistp , ebufp);
		  PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , "fm_mso_integ_resubmit_failed_job  read order poid output flist", read_oflistp);

		  if(PIN_ERRBUF_IS_ERR(ebufp))
		  {
			 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " error in reading the order object");
			 PIN_ERRBUF_CLEAR(ebufp);
			 PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
		  }
		  if(read_oflistp)
		  {
			  order_po_no = PIN_FLIST_FLD_GET(read_oflistp, MSO_FLD_PO_NO, 1, ebufp);
			  order_id = PIN_FLIST_FLD_GET(read_oflistp, PIN_FLD_ORDER_ID, 1, ebufp);

		  }
	}

	if(*flags == 1)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Device loading job resubmission " );

		task_poid_type = (char *)PIN_POID_GET_TYPE(failed_task_obj);

		task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);

		char	*flistbuf	= NULL;
		pin_buf_t	*nota_buf	= NULL;
		flistlen = 0;
		int32		opcode = PCM_OP_DEVICE_CREATE;

		PIN_FLIST_FLD_SET(task_flistp,MSO_FLD_ORDER_OBJ ,order_poidp , ebufp);
		
		if(strstr(task_poid_type , "stb"))
		{
			PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_POID , PIN_POID_CREATE(db ,"/device/stb", -1 ,ebufp),ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PROGRAM_NAME , "STB Device loading",ebufp);
		}
		else if(strstr(task_poid_type , "vc"))
		{
			PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_POID , PIN_POID_CREATE(db ,"/device/vc", -1,ebufp),ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PROGRAM_NAME , "VC Device loading",ebufp);
		}
		else if(strstr(task_poid_type , "modem"))
		{
			PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_POID , PIN_POID_CREATE(db ,"/device/modem", -1,ebufp),ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PROGRAM_NAME , "MODEM Device loading",ebufp);

		}
		else if(strstr(task_poid_type , "router/cable"))
		{
			PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_POID , PIN_POID_CREATE(db ,"/device/router/cable", -1,ebufp),ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PROGRAM_NAME , "Cable Router Device loading",ebufp);
		}
		else if(strstr(task_poid_type , "router/wifi"))
		{
			PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_POID , PIN_POID_CREATE(db ,"/device/router/wifi", -1,ebufp),ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PROGRAM_NAME , "WIFI Router Device loading",ebufp);
		}
		else if(strstr(task_poid_type , "nat"))
		{
			PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_POID , PIN_POID_CREATE(db ,"/device/nat", -1,ebufp),ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PROGRAM_NAME , "NAT Device loading",ebufp);
		}
		else if(strstr(task_poid_type , "ip/static"))
		{
			PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_POID , PIN_POID_CREATE(db ,"/device/ip/static", -1,ebufp),ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PROGRAM_NAME , "Static IP Device loading",ebufp);
		}
		else if(strstr(task_poid_type , "ip/framed"))
		{
			PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_POID , PIN_POID_CREATE(db ,"/device/ip/framed", -1,ebufp),ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PROGRAM_NAME , "Framed IP Device loading",ebufp);
		}

		PIN_FLIST_TO_STR( task_flistp, &flistbuf, &flistlen, ebufp );

		nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
		if ( nota_buf == NULL )
		{
			pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
			PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
			 return;
		}
		 device_id = PIN_FLIST_FLD_GET(task_flistp ,PIN_FLD_DEVICE_ID, 1 , ebufp);
		/*
		 * Create flist for job order creation
		 */
	
		job_iflistp	= PIN_FLIST_CREATE(ebufp);
		if(strstr(task_poid_type ,"stb"))
		{
			job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/device/stb", -1 ,ebufp);
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , failed_task_obj , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , failed_task_id , ebufp);
		
			PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
                	PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
		}
		else if(strstr(task_poid_type ,"vc"))
		{
			job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/device/vc", -1 ,ebufp);
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , failed_task_obj , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , failed_task_id , ebufp);
			PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
             		PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
		}
		else if(strstr(task_poid_type ,"modem"))
		{
			job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/device/modem", -1 ,ebufp);
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , failed_task_obj , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , failed_task_id , ebufp);
			
			if(device_id && (fm_mso_integ_match_patterns("^([0-9a-fA-F]{2})(([/\\s:-][0-9a-fA-F]{2}){5})$", device_id) <=0) )
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the task object " );
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS, &status_fail, ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid Device ID: %s, it is NOT a MAC address.", ebufp);

			}
			       
		}
		else if(strstr(task_poid_type ,"router/cable"))
		{
			job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/device/router/cable", -1 ,ebufp);
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , failed_task_obj , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , failed_task_id , ebufp);
		
			
			if(device_id && (fm_mso_integ_match_patterns("^([0-9a-fA-F]{2})(([/\\s:-][0-9a-fA-F]{2}){5})$", device_id) <=0 ))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the task object " );
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS, &status_fail, ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid Device ID: %s, it is NOT a MAC address.", ebufp);
			}
			
		}
		else if(strstr(task_poid_type ,"router/wifi"))
		{
			job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/device/router/wifi", -1 ,ebufp);
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , failed_task_obj , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , failed_task_id , ebufp);
			
			if(device_id && (fm_mso_integ_match_patterns("^([0-9a-fA-F]{2})(([/\\s:-][0-9a-fA-F]{2}){5})$", device_id) <=0) )
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the task object " );
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS, &status_fail, ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid Device ID: %s, it is NOT a MAC address.", ebufp);
			}
			
		}
		else if(strstr(task_poid_type ,"nat"))
		{
			job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/device/nat", -1 ,ebufp);
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , failed_task_obj , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , failed_task_id , ebufp);
			
			if(device_id && (fm_mso_integ_match_patterns("^([0-9a-fA-F]{2})(([/\\s:-][0-9a-fA-F]{2}){5})$", device_id) <=0 ))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the task object " );
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS, &status_fail, ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid Device ID: %s, it is NOT a MAC address.", ebufp);
			}
			
		}
		else if(strstr(task_poid_type ,"ip/static"))
		{
			job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/device/ip/static", -1 ,ebufp);
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , failed_task_obj , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , failed_task_id , ebufp);
		}
		else if(strstr(task_poid_type ,"ip/framed"))
		{
			job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/device/ip/framed", -1 ,ebufp);
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , failed_task_obj , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , failed_task_id , ebufp);
		}
		nota_buf->flag   = 0;
		nota_buf->size   = flistlen - 2;
		nota_buf->offset = 0;
		nota_buf->data   = ( caddr_t)flistbuf;
		nota_buf->xbuf_file = ( char *) NULL;
		PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );

		//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Job create input flist " ,job_iflistp);

		PCM_OP(ctxp,PCM_OP_CREATE_OBJ ,0, job_iflistp , &job_oflistp , ebufp);

		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the mso_mta job object " );
			PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
			PIN_ERRBUF_CLEAR(ebufp);
			/*r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_create_job job object create failed", ebufp);
			*ret_flistp = PIN_FLIST_COPY(r_flistp , ebufp);
			PIN_FLIST_DESTROY_EX(&r_flistp , NULL);
			return;*/
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " job create output flist is " , job_oflistp);
		PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
		if(nota_buf)
		{
			free(nota_buf);
		}

		/******
		 * update the status of the failed job to uncommited 
		 ******/

		write_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
			PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
			PIN_ERRBUF_CLEAR(ebufp);
		}

		PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
		PIN_FLIST_DESTROY_EX(&read_inflist , NULL);
		PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
	}
	if(*flags == 2)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Device location update job resubmission " );

		task_poid_type = (char *)PIN_POID_GET_TYPE(failed_task_obj);

		task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);

		if(strstr(task_poid_type ,"stb"))
		{
			PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_POID , PIN_POID_CREATE(db ,"/device/stb", -1 ,ebufp),ebufp);
			PIN_FLIST_FLD_SET(task_flistp,PIN_FLD_DEVICE_TYPE,"STB",ebufp);
		}
		else if(strstr(task_poid_type ,"vc"))
		{
			PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_POID , PIN_POID_CREATE(db ,"/device/vc", -1 ,ebufp),ebufp);
			PIN_FLIST_FLD_SET(task_flistp,PIN_FLD_DEVICE_TYPE,"VC",ebufp);
		}
		else if(strstr(task_poid_type ,"modem"))
		{
			PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_POID , PIN_POID_CREATE(db ,"/device/modem", -1 ,ebufp),ebufp);
			PIN_FLIST_FLD_SET(task_flistp,PIN_FLD_DEVICE_TYPE,"MODEM",ebufp);
		}
		else if(strstr(task_poid_type ,"router/cable"))
		{
			PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_POID , PIN_POID_CREATE(db ,"/device/router/cable", -1 ,ebufp),ebufp);
			PIN_FLIST_FLD_SET(task_flistp,PIN_FLD_DEVICE_TYPE,"ROUTER/CABLE",ebufp);
		}
		else if(strstr(task_poid_type ,"router/wifi"))
		{
			PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_POID , PIN_POID_CREATE(db ,"/device/router/wifi", -1 ,ebufp),ebufp);
			PIN_FLIST_FLD_SET(task_flistp,PIN_FLD_DEVICE_TYPE,"ROUTER/WIFI",ebufp);
		}
		else if(strstr(task_poid_type ,"nat"))
		{
			PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_POID , PIN_POID_CREATE(db ,"/device/nat", -1 ,ebufp),ebufp);
			PIN_FLIST_FLD_SET(task_flistp,PIN_FLD_DEVICE_TYPE,"NAT",ebufp);
		}
		else if(strstr(task_poid_type ,"ip/static"))
		{
			PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_POID , PIN_POID_CREATE(db ,"/device/ip/static", -1 ,ebufp),ebufp);
			PIN_FLIST_FLD_SET(task_flistp,PIN_FLD_DEVICE_TYPE,"IP/STATIC",ebufp);
		}
		else if(strstr(task_poid_type ,"ip/framed"))
		{
			PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_POID , PIN_POID_CREATE(db ,"/device/ip/framed", -1 ,ebufp),ebufp);
			PIN_FLIST_FLD_SET(task_flistp,PIN_FLD_DEVICE_TYPE,"IP/FRAMED",ebufp);
		}

		PIN_FLIST_FLD_SET(task_flistp,MSO_FLD_ORDER_OBJ ,order_poidp , ebufp);

		PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_USERID , task_flistp ,PIN_FLD_USERID , ebufp);
                PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_PROGRAM_NAME , task_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
		fm_mso_integ_create_location_update_job(ctxp,failed_task_id,failed_task_obj,task_flistp, ebufp);
		
		/******
		 * update the status of the failed job to uncommited 
		 ******/
		write_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
			PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
			PIN_ERRBUF_CLEAR(ebufp);
		}

			PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
			PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
	}
	if(*flags == 3)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Device state change job resubmission " );

		task_poid_type = (char *)PIN_POID_GET_TYPE(failed_task_obj);

		task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);

		if(strstr(task_poid_type ,"stb"))
		{
			PIN_FLIST_FLD_SET(task_flistp,PIN_FLD_DEVICE_TYPE,"STB",ebufp);
		}
		else if(strstr(task_poid_type ,"vc"))
		{
			PIN_FLIST_FLD_SET(task_flistp,PIN_FLD_DEVICE_TYPE,"VC",ebufp);
		}
		else if(strstr(task_poid_type ,"modem"))
		{
			PIN_FLIST_FLD_SET(task_flistp,PIN_FLD_DEVICE_TYPE,"MODEM",ebufp);
		}
		else if(strstr(task_poid_type ,"router/cable"))
		{
			PIN_FLIST_FLD_SET(task_flistp,PIN_FLD_DEVICE_TYPE,"ROUTER/CABLE",ebufp);
		}
		else if(strstr(task_poid_type ,"router/wifi"))
		{
			PIN_FLIST_FLD_SET(task_flistp,PIN_FLD_DEVICE_TYPE,"ROUTER/WIFI",ebufp);
		}
		else if(strstr(task_poid_type ,"nat"))
		{
			PIN_FLIST_FLD_SET(task_flistp,PIN_FLD_DEVICE_TYPE,"NAT",ebufp);
		}
		else if(strstr(task_poid_type ,"ip/static"))
		{
			PIN_FLIST_FLD_SET(task_flistp,PIN_FLD_DEVICE_TYPE,"IP/STATIC",ebufp);
		}
		else if(strstr(task_poid_type ,"ip/framed"))
		{
			PIN_FLIST_FLD_SET(task_flistp,PIN_FLD_DEVICE_TYPE,"IP/FRAMED",ebufp);
		}

		PIN_FLIST_FLD_SET(task_flistp,MSO_FLD_ORDER_OBJ ,order_poidp , ebufp);
		//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PROGRAM_NAME,"Bulk State change" , ebufp);

		PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_USERID , task_flistp ,PIN_FLD_USERID , ebufp);
                PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_PROGRAM_NAME , task_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
		
		fm_mso_integ_create_state_change_job(ctxp,failed_task_id,failed_task_obj,task_flistp, ebufp);

		/******
		 * update the status of the failed job to uncommited 
		 ******/
		write_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
			PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
			PIN_ERRBUF_CLEAR(ebufp);
		}

		PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&read_inflist , NULL);
		PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
	}
	if(*flags == 4)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " catv preactivation job resubmission " );

		//task_poid_type = (char *)PIN_POID_GET_TYPE(failed_task_obj);

		task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);
		//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PROGRAM_NAME,"Bulk catv preactivation" , ebufp);

		PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_USERID , task_flistp ,PIN_FLD_USERID , ebufp);
                PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_PROGRAM_NAME , task_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);

		fm_mso_integ_create_catv_preactivation_job(ctxp,failed_task_id,failed_task_obj,task_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error while creating the catv preactivation job poid");
			PIN_ERRBUF_CLEAR(ebufp);
		}
		/******
		 * update the status of the failed job to uncommited 
		 ******/
		write_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
			PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
			PIN_ERRBUF_CLEAR(ebufp);
		}

		PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
		PIN_FLIST_DESTROY_EX(&read_inflist , NULL);
		PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
	}

	if(*flags == 5)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Add Plan job resubmission " );

		//task_poid_type = (char *)PIN_POID_GET_TYPE(failed_task_obj);

		task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);

		PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_FLAGS, &add_plan_flags , ebufp);
		//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PROGRAM_NAME,"Bulk Add Plan" , ebufp);
		PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_USERID , task_flistp ,PIN_FLD_USERID , ebufp);
                PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_PROGRAM_NAME , task_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);

		fm_mso_integ_create_add_plan_job(ctxp,failed_task_id,failed_task_obj,task_flistp, ebufp);

		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error set");
			PIN_ERRBUF_CLEAR(ebufp);
		}

		/******
		* update the status of the failed job to uncommited 
		******/

		write_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
			PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
			PIN_ERRBUF_CLEAR(ebufp);
		}

		PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&read_inflist , NULL);	
	}
	if(*flags == 6)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " change Plan job resubmission " );

		//task_poid_type = (char *)PIN_POID_GET_TYPE(failed_task_obj);

		task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);

		PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_FLAGS, &add_plan_flags , ebufp);
		//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PROGRAM_NAME,"Bulk Change Plan" , ebufp);
		PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_USERID , task_flistp ,PIN_FLD_USERID , ebufp);
                PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_PROGRAM_NAME , task_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
		fm_mso_integ_create_change_plan_job(ctxp,failed_task_id,failed_task_obj,task_flistp, ebufp);
		
		/******
		 * update the status of the failed job to uncommited 
		 ******/

		write_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
			PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
			PIN_ERRBUF_CLEAR(ebufp);
		}

		PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&read_inflist , NULL);
		PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
	}
	if(*flags == 7)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " cancel Plan job resubmission " );

		task_poid_type = (char *)PIN_POID_GET_TYPE(failed_task_obj);

		task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);

		PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_FLAGS, &add_plan_flags , ebufp);
		//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PROGRAM_NAME,"Bulk cancel plan" , ebufp);

		PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_USERID , task_flistp ,PIN_FLD_USERID , ebufp);
                PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_PROGRAM_NAME , task_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
		fm_mso_integ_create_cancel_plan_job(ctxp,failed_task_id,failed_task_obj,task_flistp, ebufp);
		
		/******
		 * update the status of the failed job to uncommited 
		 ******/

		write_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
			PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
			PIN_ERRBUF_CLEAR(ebufp);
		}

		PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&read_inflist , NULL);
		PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
	}
	if(*flags == 8)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " service suspension job resubmission " );
		int32		suspend_status = 10102;
		int32		opcode	= MSO_OP_CUST_SUSPEND_SERVICE;

		task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);

		PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_OPCODE, &opcode , ebufp);
		PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_STATUS , &suspend_status, ebufp);
		//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PROGRAM_NAME,"Bulk service suspension" , ebufp);

		PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_USERID , task_flistp ,PIN_FLD_USERID , ebufp);
                PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_PROGRAM_NAME , task_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);

		fm_mso_integ_create_service_actions_job(ctxp,failed_task_id,failed_task_obj,task_flistp, ebufp);

		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error set");
			PIN_ERRBUF_CLEAR(ebufp);
		}

		/******
		* update the status of the failed job to uncommited 
		******/

		write_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
			PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
			PIN_ERRBUF_CLEAR(ebufp);
		}

		PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&read_inflist , NULL);
		PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
	}
	if(*flags == 9)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " service reconnection job resubmission " );
		int32		suspend_status = 10100;
		int32		opcode	= MSO_OP_CUST_REACTIVATE_SERVICE;

		task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);

		PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_OPCODE, &opcode , ebufp);
		PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_STATUS , &suspend_status, ebufp);
		//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PROGRAM_NAME,"Bulk service reconnection" , ebufp);
		PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_USERID , task_flistp ,PIN_FLD_USERID , ebufp);
                PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_PROGRAM_NAME , task_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);

		fm_mso_integ_create_service_actions_job(ctxp,failed_task_id,failed_task_obj,task_flistp, ebufp);

		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error set");
			PIN_ERRBUF_CLEAR(ebufp);
		}

		/******
		* update the status of the failed job to uncommited 
		******/

		write_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
			PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
			PIN_ERRBUF_CLEAR(ebufp);
		}

		PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&read_inflist , NULL);
		PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
	}
	if(*flags == 10)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " service termination job resubmission " );
		int32		suspend_status = 10103;
		int32		opcode	= MSO_OP_CUST_TERMINATE_SERVICE;

		task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);

		PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_OPCODE, &opcode , ebufp);
		PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_STATUS , &suspend_status, ebufp);
		//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PROGRAM_NAME,"Bulk service termination" , ebufp);
	
		PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_USERID , task_flistp ,PIN_FLD_USERID, ebufp);
        PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_PROGRAM_NAME , task_flistp, PIN_FLD_PROGRAM_NAME , ebufp);
		// CATV GRV AD === Included the Termination reason for Bulk Termination resubmission cases
		//PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_DESCR, task_flistp, PIN_FLD_DESCR, ebufp);
	
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Printing task_flistp .... " );
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_resubmit_failed_job task_flistp >> ", task_flistp);
		
		fm_mso_integ_create_service_actions_job(ctxp,failed_task_id,failed_task_obj,task_flistp, ebufp);

		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error set");
			PIN_ERRBUF_CLEAR(ebufp);
		}

		/******
		* update the status of the failed job to uncommited 
		******/

		write_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
			PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
			PIN_ERRBUF_CLEAR(ebufp);
		}

		PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&read_inflist , NULL);
		PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
	}
	if(*flags == 11)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Bulk Adjustment resubmission " );
		task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);
		//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PROGRAM_NAME,"Bulk ADJUSTMENT" , ebufp);

		PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_USERID , task_flistp ,PIN_FLD_USERID , ebufp);
                PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_PROGRAM_NAME , task_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
		fm_mso_integ_create_adjustment_job(ctxp,failed_task_id,failed_task_obj,task_flistp, ebufp);

		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error set");
			PIN_ERRBUF_CLEAR(ebufp);
		}

		/******
		* update the status of the failed job to uncommited 
		******/

		write_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
			PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
			PIN_ERRBUF_CLEAR(ebufp);
		}

		PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&read_inflist , NULL);
		PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
	}
	if(*flags == 12)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " retrack job resubmission " );
		task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);

		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_FLAGS , task_flistp , PIN_FLD_FLAGS , ebufp);
		//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PROGRAM_NAME,"Bulk retrack" , ebufp);
		PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_USERID , task_flistp ,PIN_FLD_USERID , ebufp);
                PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_PROGRAM_NAME , task_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
		if(network_node)
		{
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE , network_node , ebufp);
		}

		fm_mso_integ_create_retrack_job(ctxp,failed_task_id,failed_task_obj,task_flistp, ebufp);

		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error set");
			PIN_ERRBUF_CLEAR(ebufp);
		}

		/******
		* update the status of the failed job to uncommited 
		******/

		write_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
			PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
			PIN_ERRBUF_CLEAR(ebufp);
		}

		PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&read_inflist , NULL);
		PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
	}
	if(*flags == 13)
	{
		int32	osd_flags = 23;

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Bulk OSD job resubmission " );
		task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);

		PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_FLAGS ,&osd_flags, ebufp);
		//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PROGRAM_NAME,"Bulk OSD" , ebufp);
		PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_USERID , task_flistp ,PIN_FLD_USERID , ebufp);
                PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_PROGRAM_NAME , task_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
		if(network_node)
		{
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE , network_node , ebufp);
		}

		fm_mso_integ_create_osd_job(ctxp,failed_task_id,failed_task_obj,task_flistp, ebufp);

		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error set");
			PIN_ERRBUF_CLEAR(ebufp);
		}

		/******
		* update the status of the failed job to uncommited 
		******/

		write_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
			PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
			PIN_ERRBUF_CLEAR(ebufp);
		}

		PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&read_inflist , NULL);
		PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
	}
	if(*flags == 14)
	{
		int32	bmail_flags = 11;

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Bulk Bmail job resubmission " );
		task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);

		PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_FLAGS, &bmail_flags, ebufp);
		//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PROGRAM_NAME,"Bulk Bmail" , ebufp);
		PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_USERID , task_flistp ,PIN_FLD_USERID , ebufp);
                PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_PROGRAM_NAME , task_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);

		if(network_node)
		{
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE , network_node , ebufp);
		}
		fm_mso_integ_create_bmail_job(ctxp,failed_task_id,failed_task_obj,task_flistp, ebufp);

		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error set");
			PIN_ERRBUF_CLEAR(ebufp);
		}

		/******
		* update the status of the failed job to uncommited 
		******/

		write_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
			PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
			PIN_ERRBUF_CLEAR(ebufp);
		}

		PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&read_inflist , NULL);
		PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
	}
	/*if(*flags == 15)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Bulk bouquet id change job resubmission " );
		task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);

		PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PROGRAM_NAME,"Bulk Change Bouquet id" , ebufp);

		fm_mso_integ_create_bouquet_id_job(ctxp,failed_task_id,failed_task_obj,task_flistp, ebufp);

		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error set");
			PIN_ERRBUF_CLEAR(ebufp);
		}

		/******
		* update the status of the failed job to uncommited 
		******/

		/*write_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
			PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
			PIN_ERRBUF_CLEAR(ebufp);
		}

		PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&read_inflist , NULL);
		PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
	}*/
	if(*flags == 16)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Bulk set personnel bit job resubmission " );
		task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);
		//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PROGRAM_NAME,"Bulk set personnel bit" , ebufp);
		PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_USERID , task_flistp ,PIN_FLD_USERID , ebufp);
                PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_PROGRAM_NAME , task_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);

		fm_mso_integ_set_personnel_bit_job(ctxp,failed_task_id,failed_task_obj,task_flistp, ebufp);

		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error set");
			PIN_ERRBUF_CLEAR(ebufp);
		}

		/******
		* update the status of the failed job to uncommited 
		******/

		write_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
			PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
			PIN_ERRBUF_CLEAR(ebufp);
		}

		PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&read_inflist , NULL);
		PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
	}	
        if(*flags == 17)
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Bulk CRF change job resubmission " );
                task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);

                PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_PROGRAM_NAME,task_flistp,PIN_FLD_PROGRAM_NAME,ebufp);
                PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_USERID,task_flistp,PIN_FLD_USERID,ebufp);

                fm_mso_integ_set_bulk_crf(ctxp,failed_task_id,failed_task_obj,task_flistp, ebufp);

                if(PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error set");
                        PIN_ERRBUF_CLEAR(ebufp);
                }

                /******
                * update the status of the failed job to uncommited
                ******/

                write_iflistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
                PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
                PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
                        PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
                        PIN_ERRBUF_CLEAR(ebufp);
                }

                PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
                PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
                PIN_FLIST_DESTROY_EX(&read_inflist , NULL);
                PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
        }
    if(*flags == 18)
    {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Bulk hierarchy change job resubmission " );
                task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);

                //PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PROGRAM_NAME,"Bulk hierarchy change" , ebufp);


                callerp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PARENT_FLAGS, 1, ebufp);
                if (callerp && *(int32*)callerp == CALLED_BY_MTA )
                {
                        caller = CALLED_BY_MTA;
                }
                PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_USERID , task_flistp ,PIN_FLD_USERID , ebufp);
                PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_PROGRAM_NAME , task_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
                fm_mso_integ_create_hierarchy_job(ctxp,failed_task_id,failed_task_obj,task_flistp,caller, ebufp);

                if(PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error set");
                        PIN_ERRBUF_CLEAR(ebufp);
                }

                /******
                * update the status of the failed job to uncommited
                ******/

                write_iflistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
                PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
                PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
                    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"@@Hierarchy resubmt ",write_iflistp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
                        PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
                        PIN_ERRBUF_CLEAR(ebufp);
                }
                //fm_mso_integ_create_hierarchy_job(ctxp,failed_task_id,failed_task_obj,task_flistp, ebufp);

                PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
                PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
                PIN_FLIST_DESTROY_EX(&read_inflist , NULL);
                PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
        }

//	if (flags && (*flags > 19 && *flags < 30 )) {
//        	switch (*flags) {
//                	case BULK_TOPUP:
//                    	    task_name = "Bulk TOPUP"; break;
//                	case BULK_RENEW:
//                    	    task_name = "Bulk RENEW"; break;
//                	case BULK_EXTEND_VALIDITY:
//                    	    task_name = "Bulk Extend Validity"; break;
//                	case BULK_ADDITION_MB_GB:
//                    	    task_name = "Bulk Addition MB_GB"; break;
//                	case BULK_WRITEOFF_CPE:
//                    	    task_name = "Bulk Writeoff CPE"; break;
//                	case BULK_SWAPPING_CPE:
//                    	    task_name = "Bulk Swapping CPE"; break;
//                	case BULK_CHECK_BOUNCE:
//                    	    task_name = "Bulk Check Bounce"; break;
//                	case BULK_BILL_HOLD:
//                    	    task_name = "Bulk Bill Hold"; break;
//                	case BULK_REFUND_LOAD:
//                    	    task_name = "Bulk Refund Load"; break;
//                	case BULK_REFUND_REV_LOAD:
//                    	    task_name = "Bulk Refund Reversal Load"; break;
//        	}
//                sprintf(msg , " Bulk set %s job resubmission ", task_name );
//                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
//                task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);
//
//		sprintf(msg, "OAP|%s", task_name);
//                PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PROGRAM_NAME,msg, ebufp);
//        	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, task_name);
//		user_pd = PIN_FLIST_FLD_GET(in_flistp , PIN_FLD_USERID , 1 , ebufp);
//	
//		fm_mso_integ_create_job_generic(ctxp,*flags,failed_task_id,failed_task_obj,task_flistp,user_pd,ebufp);
//
//                if(PIN_ERRBUF_IS_ERR(ebufp)) {
//                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error set");
//                        PIN_ERRBUF_CLEAR(ebufp);
//                }
//		}
        if(*flags == 19)
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Bulk IP Pool Updaer job resubmission " );
                task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);

                PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_PROGRAM_NAME,task_flistp,PIN_FLD_PROGRAM_NAME,ebufp);
                PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_USERID,task_flistp,PIN_FLD_USERID,ebufp);
		PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_POID,task_flistp,MSO_FLD_ORDER_OBJ,ebufp);
		PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_POID,task_flistp,PIN_FLD_POID,ebufp);

                fm_mso_integ_create_ippool_update_job(ctxp,failed_task_id,failed_task_obj,task_flistp, ebufp);

                if(PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error set");
                        PIN_ERRBUF_CLEAR(ebufp);
                }

                /******
                * update the status of the failed job to uncommited
                ******/

                write_iflistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
                PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
                PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
                        PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
                        PIN_ERRBUF_CLEAR(ebufp);
                }

                PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
                PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
                PIN_FLIST_DESTROY_EX(&read_inflist , NULL);
                PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
        }

        if(*flags == 20)
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Bulk topup job resubmission " );
                task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);

                PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_PROGRAM_NAME,task_flistp,PIN_FLD_PROGRAM_NAME,ebufp);
                PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_USERID,task_flistp,PIN_FLD_USERID,ebufp);
		PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_POID,task_flistp,MSO_FLD_ORDER_OBJ,ebufp);
		PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_POID,task_flistp,PIN_FLD_POID,ebufp);

                fm_mso_integ_topup_create_job(ctxp,failed_task_id,failed_task_obj,task_flistp, ebufp);

                if(PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error set");
                        PIN_ERRBUF_CLEAR(ebufp);
                }

                /******
                * update the status of the failed job to uncommited
                ******/

                write_iflistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
                PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
                PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
                        PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
                        PIN_ERRBUF_CLEAR(ebufp);
                }

                PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
                PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
                PIN_FLIST_DESTROY_EX(&read_inflist , NULL);
                PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
        }
        if(*flags == 21)
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Bulk Renew job resubmission " );
                task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);

                PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_PROGRAM_NAME,task_flistp,PIN_FLD_PROGRAM_NAME,ebufp);
                PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_USERID,task_flistp,PIN_FLD_USERID,ebufp);
		PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_POID,task_flistp,MSO_FLD_ORDER_OBJ,ebufp);
		PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_POID,task_flistp,PIN_FLD_POID,ebufp);

                fm_mso_integ_renew_create_job(ctxp,failed_task_id,failed_task_obj,task_flistp, ebufp);

                if(PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error set");
                        PIN_ERRBUF_CLEAR(ebufp);
                }

                /******
                * update the status of the failed job to uncommited
                ******/

                write_iflistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
                PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
                PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
                        PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
                        PIN_ERRBUF_CLEAR(ebufp);
                }

                PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
                PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
                PIN_FLIST_DESTROY_EX(&read_inflist , NULL);
                PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
        }
        if(*flags == 22)
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Bulk Extend validity job resubmission " );
                task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);

                PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_PROGRAM_NAME,task_flistp,PIN_FLD_PROGRAM_NAME,ebufp);
                PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_USERID,task_flistp,PIN_FLD_USERID,ebufp);
		PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_POID,task_flistp,MSO_FLD_ORDER_OBJ,ebufp);
		PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_POID,task_flistp,PIN_FLD_POID,ebufp);

                fm_mso_integ_extend_validity_create_job(ctxp,failed_task_id,failed_task_obj,task_flistp, ebufp);

                if(PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error set");
                        PIN_ERRBUF_CLEAR(ebufp);
                }

                /******
                * update the status of the failed job to uncommited
                ******/

                write_iflistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
                PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
                PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
                        PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
                        PIN_ERRBUF_CLEAR(ebufp);
                }

                PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
                PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
                PIN_FLIST_DESTROY_EX(&read_inflist , NULL);
                PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
        }
        if(*flags == 23)
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Bulk Addition MB & GB job resubmission " );
                task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);

                PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_PROGRAM_NAME,task_flistp,PIN_FLD_PROGRAM_NAME,ebufp);
                PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_USERID,task_flistp,PIN_FLD_USERID,ebufp);
		PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_POID,task_flistp,MSO_FLD_ORDER_OBJ,ebufp);
		PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_POID,task_flistp,PIN_FLD_POID,ebufp);

                fm_mso_integ_create_add_plan_job(ctxp,failed_task_id,failed_task_obj,task_flistp, ebufp);

                if(PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error set");
                        PIN_ERRBUF_CLEAR(ebufp);
                }

                /******
                * update the status of the failed job to uncommited
                ******/

                write_iflistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
                PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
                PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
                        PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
                        PIN_ERRBUF_CLEAR(ebufp);
                }

                PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
                PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
                PIN_FLIST_DESTROY_EX(&read_inflist , NULL);
                PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
        }
        if(*flags == 24)
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Bulk writeoff CPE job resubmission " );
                task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);

                PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_PROGRAM_NAME,task_flistp,PIN_FLD_PROGRAM_NAME,ebufp);
                PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_USERID,task_flistp,PIN_FLD_USERID,ebufp);
		PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_POID,task_flistp,MSO_FLD_ORDER_OBJ,ebufp);
		PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_POID,task_flistp,PIN_FLD_POID,ebufp);

                fm_mso_integ_cpe_writeoff_create_job(ctxp,failed_task_id,failed_task_obj,task_flistp, ebufp);

                if(PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error set");
                        PIN_ERRBUF_CLEAR(ebufp);
                }

                /******
                * update the status of the failed job to uncommited
                ******/

                write_iflistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
                PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
                PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
                        PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
                        PIN_ERRBUF_CLEAR(ebufp);
                }

                PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
                PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
                PIN_FLIST_DESTROY_EX(&read_inflist , NULL);
                PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
        }
        if(*flags == 25)
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Bulk Swaping CPE job resubmission " );
                task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);

                PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_PROGRAM_NAME,task_flistp,PIN_FLD_PROGRAM_NAME,ebufp);
                PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_USERID,task_flistp,PIN_FLD_USERID,ebufp);
		PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_POID,task_flistp,MSO_FLD_ORDER_OBJ,ebufp);
		PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_POID,task_flistp,PIN_FLD_POID,ebufp);

                fm_mso_integ_swaping_cpe_create_job(ctxp,failed_task_id,failed_task_obj,task_flistp, ebufp);

                if(PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error set");
                        PIN_ERRBUF_CLEAR(ebufp);
                }

                /******
                * update the status of the failed job to uncommited
                ******/

                write_iflistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
                PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
                PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
                        PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
                        PIN_ERRBUF_CLEAR(ebufp);
                }

                PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
                PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
                PIN_FLIST_DESTROY_EX(&read_inflist , NULL);
                PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
        }		
        if(*flags == 26)
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Bulk Cheque Bounce job resubmission " );
                task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);

                PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_PROGRAM_NAME,task_flistp,PIN_FLD_PROGRAM_NAME,ebufp);
                PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_USERID,task_flistp,PIN_FLD_USERID,ebufp);
		PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_POID,task_flistp,MSO_FLD_ORDER_OBJ,ebufp);
		PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_POID,task_flistp,PIN_FLD_POID,ebufp);

                fm_mso_integ_cheque_bounce_create_job(ctxp,failed_task_id,failed_task_obj,task_flistp, ebufp);

                if(PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error set");
                        PIN_ERRBUF_CLEAR(ebufp);
                }

                /******
                * update the status of the failed job to uncommited
                ******/

                write_iflistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
                PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
                PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
                        PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
                        PIN_ERRBUF_CLEAR(ebufp);
                }

                PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
                PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
                PIN_FLIST_DESTROY_EX(&read_inflist , NULL);
                PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
        }		
        if(*flags == 27)
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Bulk Bill Hold job resubmission " );
                task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);

                PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_PROGRAM_NAME,task_flistp,PIN_FLD_PROGRAM_NAME,ebufp);
                PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_USERID,task_flistp,PIN_FLD_USERID,ebufp);
		PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_POID,task_flistp,MSO_FLD_ORDER_OBJ,ebufp);
		PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_POID,task_flistp,PIN_FLD_POID,ebufp);;

                fm_mso_integ_bill_hold_create_job(ctxp,failed_task_id,failed_task_obj,task_flistp, ebufp);

                if(PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error set");
                        PIN_ERRBUF_CLEAR(ebufp);
                }

                /******
                * update the status of the failed job to uncommited
                ******/

                write_iflistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
                PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
                PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
                        PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
                        PIN_ERRBUF_CLEAR(ebufp);
                }

                PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
                PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
                PIN_FLIST_DESTROY_EX(&read_inflist , NULL);
                PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
        }		
        if(*flags == 28)
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Bulk Refund Load job resubmission " );
                task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);

                PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_PROGRAM_NAME,task_flistp,PIN_FLD_PROGRAM_NAME,ebufp);
                PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_USERID,task_flistp,PIN_FLD_USERID,ebufp);
		PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_POID,task_flistp,MSO_FLD_ORDER_OBJ,ebufp);
		PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_POID,task_flistp,PIN_FLD_POID,ebufp);

                fm_mso_integ_refund_load_create_job(ctxp,failed_task_id,failed_task_obj,task_flistp, ebufp);

                if(PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error set");
                        PIN_ERRBUF_CLEAR(ebufp);
                }

                /******
                * update the status of the failed job to uncommited
                ******/

                write_iflistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
                PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
                PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
                        PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
                        PIN_ERRBUF_CLEAR(ebufp);
                }

                PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
                PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
                PIN_FLIST_DESTROY_EX(&read_inflist , NULL);
                PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
        }
        if(*flags == 29)
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Bulk Refund reversal job resubmission " );
                task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);

                PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_PROGRAM_NAME,task_flistp,PIN_FLD_PROGRAM_NAME,ebufp);
                PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_USERID,task_flistp,PIN_FLD_USERID,ebufp);
		PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_POID,task_flistp,MSO_FLD_ORDER_OBJ,ebufp);
		PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_POID,task_flistp,PIN_FLD_POID,ebufp);

                fm_mso_integ_refund_reversal_load_create_job(ctxp,failed_task_id,failed_task_obj,task_flistp, ebufp);

                if(PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error set");
                        PIN_ERRBUF_CLEAR(ebufp);
                }

                /******
                * update the status of the failed job to uncommited
                ******/

                write_iflistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
                PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
                PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
                        PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
                        PIN_ERRBUF_CLEAR(ebufp);
                }

                PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
                PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
                PIN_FLIST_DESTROY_EX(&read_inflist , NULL);
                PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
        }

        if(*flags == 31)
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Bulk CR DR Note job resubmission " );
                task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);

                PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_PROGRAM_NAME,task_flistp,PIN_FLD_PROGRAM_NAME,ebufp);
                PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_USERID,task_flistp,PIN_FLD_USERID,ebufp);
		PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_POID,task_flistp,MSO_FLD_ORDER_OBJ,ebufp);
		PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_POID,task_flistp,PIN_FLD_POID,ebufp);

                fm_mso_integ_create_crdr_job(ctxp,failed_task_id,failed_task_obj,task_flistp, ebufp);

                if(PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error set");
                        PIN_ERRBUF_CLEAR(ebufp);
                }

                /******
                * update the status of the failed job to uncommited
                ******/

                write_iflistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
                PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
                PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
                        PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
                        PIN_ERRBUF_CLEAR(ebufp);
                }

                PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
                PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
                PIN_FLIST_DESTROY_EX(&read_inflist , NULL);
                PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
        }

        if(*flags == 32)
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Bulk upload GRV job resubmission " );
                task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);

                PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_PROGRAM_NAME,task_flistp,PIN_FLD_PROGRAM_NAME,ebufp);
                PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_USERID,task_flistp,PIN_FLD_USERID,ebufp);
		PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_POID,task_flistp,MSO_FLD_ORDER_OBJ,ebufp);
		PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_POID,task_flistp,PIN_FLD_POID,ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Bulk upload GRV job resubmission input flist", task_flistp);
                fm_mso_integ_create_grv_uploader_job(ctxp,failed_task_id,failed_task_obj,task_flistp, ebufp);

                if(PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error set");
                        PIN_ERRBUF_CLEAR(ebufp);
                }

                /******
                * update the status of the failed job to uncommited
                ******/

                write_iflistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
                PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
                PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
                        PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
                        PIN_ERRBUF_CLEAR(ebufp);
                }

                PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
                PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
                PIN_FLIST_DESTROY_EX(&read_inflist , NULL);
                PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
        }
        if(*flags == 33)
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Bulk device attribute updater job resubmission " );
                task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);

		task_poid_type = (char *)PIN_POID_GET_TYPE(failed_task_obj);

                task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);


                PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_PROGRAM_NAME,task_flistp,PIN_FLD_PROGRAM_NAME,ebufp);
                PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_USERID,task_flistp,PIN_FLD_USERID,ebufp);
		PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_POID,task_flistp,MSO_FLD_ORDER_OBJ,ebufp);
		PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_POID,task_flistp,PIN_FLD_POID,ebufp);

                fm_mso_integ_create_attribute_update_job(ctxp,failed_task_id,failed_task_obj,task_flistp, ebufp);

                if(PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error set");
                        PIN_ERRBUF_CLEAR(ebufp);
                }

                /******
                * update the status of the failed job to uncommited
                ******/

                write_iflistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
                PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
                PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
                        PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
                        PIN_ERRBUF_CLEAR(ebufp);
                }

                PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
                PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
                PIN_FLIST_DESTROY_EX(&read_inflist , NULL);
                PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
        }
        if(*flags == 34)
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Bulk CMTS Change job resubmission " );	
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Flist Is" , in_flistp);
                task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Task Flist IS", task_flistp);

                PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_PROGRAM_NAME,task_flistp,PIN_FLD_PROGRAM_NAME,ebufp);
                PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_USERID,task_flistp,PIN_FLD_USERID,ebufp);
                PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_POID,task_flistp,MSO_FLD_ORDER_OBJ,ebufp);
                PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_POID,task_flistp,PIN_FLD_POID,ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Task Flist After Modify IS", task_flistp);

                fm_mso_integ_cmts_change_create_job(ctxp,failed_task_id,failed_task_obj,task_flistp, ebufp);

                if(PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error set");
                        PIN_ERRBUF_CLEAR(ebufp);
                }

                /******
                * update the status of the failed job to uncommited
                ******/

                write_iflistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
                PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
                PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
                        PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
                        PIN_ERRBUF_CLEAR(ebufp);
                }

                PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
                PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
                PIN_FLIST_DESTROY_EX(&read_inflist , NULL);
                PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
        }
		if (*flags == 35)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Bulk Account Creation job resubmission " );	
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Flist Is" , in_flistp);
			task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK,PIN_ELEMID_ANY,1,ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Task Flist is", task_flistp);

			PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_PROGRAM_NAME,task_flistp,PIN_FLD_PROGRAM_NAME,ebufp);
			PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_USERID,task_flistp,PIN_FLD_USERID,ebufp);
			PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_POID,task_flistp,MSO_FLD_ORDER_OBJ,ebufp);
			PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_POID,task_flistp,PIN_FLD_POID,ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Task Flist Before Modify is", task_flistp);

			fm_mso_integ_blk_create_acct_update_job(ctxp, failed_task_id, failed_task_obj, task_flistp, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Task Flist After Modify is", task_flistp);

			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error set");
				PIN_ERRBUF_CLEAR(ebufp);
			}

			/******
			* update the status of the failed job to uncommited
			******/

/*			write_iflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
			PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
			PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
				PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
				PIN_ERRBUF_CLEAR(ebufp);
			}

			PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
			PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
*/
			PIN_FLIST_DESTROY_EX(&read_inflist , NULL);
			PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
		}
	if(flags && *flags == 37)
	{
		program_name = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
		user_id = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_USERID, 1, ebufp);
		task_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ORDER_OBJ, 1, ebufp);
		task_no = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ORDER_ID, 1, ebufp);
		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Update GST Info Job resubmission" );
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Flist is" , in_flistp);
		task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK, PIN_ELEMID_ANY, 1, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Task Flist is", task_flistp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Task Flist After Modify is", task_flistp);

		fm_mso_create_update_gst_info_jobs(ctxp, task_no, task_pdp, user_id, program_name, task_flistp , ebufp);

		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error set");
			PIN_ERRBUF_CLEAR(ebufp);
		}

		/******
		* update the status of the failed job to uncommited
		******/

		write_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
			PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
			PIN_ERRBUF_CLEAR(ebufp);
		}
		PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&read_inflist , NULL);
		PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
	}
	if(flags && *flags == 38)
	{
		program_name = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
		user_id = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_USERID, 1, ebufp);
		task_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ORDER_OBJ, 1, ebufp);
		task_no = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ORDER_ID, 1, ebufp);
		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Modify GST Info Job resubmission" );
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Flist is" , in_flistp);
		task_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_TASK, PIN_ELEMID_ANY, 1, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Task Flist is", task_flistp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Task Flist After Modify is", task_flistp);

		fm_mso_create_modify_gst_info_jobs(ctxp, task_no, task_pdp, user_id, program_name, task_flistp , ebufp);

		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error set");
			PIN_ERRBUF_CLEAR(ebufp);
		}

		/******
		* update the status of the failed job to uncommited
		******/

		write_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, failed_job_poid, ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATUS , &status_uncommit , ebufp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_iflistp , &write_oflistp , ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the mso_mta job object " );
			PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
			PIN_ERRBUF_CLEAR(ebufp);
		}
		PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&read_inflist , NULL);
		PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
	}
//=====================================================================	
	*out_flistpp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_POID,*out_flistpp, PIN_FLD_POID ,ebufp);
	/*if(order_id != NULL)
	{
		PIN_FLIST_FLD_SET(*out_flistpp,PIN_FLD_ORDER_ID , order_id , ebufp);
	}
	if(order_po_no != NULL)
	{
		PIN_FLIST_FLD_SET(*out_flistpp,PIN_FLD_ORDER_ID , order_po_no , ebufp);
	}*/
	PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_STATUS , &order_status_success , ebufp);
	task_flistp = PIN_FLIST_ELEM_ADD(*out_flistpp,MSO_FLD_TASK,0, ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_OBJ , failed_task_obj , ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_ID , failed_task_id , ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Bulk job resubmission out flist", *out_flistpp);
	PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
	return;
}
