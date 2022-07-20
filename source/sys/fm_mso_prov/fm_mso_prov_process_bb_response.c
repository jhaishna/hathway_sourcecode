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
* 0.1    |01/10/2014 |Someshwar         |  BB Provisioning - Response Processing
*--------------------------------------------------------------------------------------------------------------*
*
****************************************************************************************************************/


#ifndef lint
static  char Sccs_Id[] = "@(#)$Id: fm_mso_prov_bb_process_response.c /cgbubrm_7.3.2.rwsmod/1 2009/08/04 23:37:19 pjegan Exp $";
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
#include "ops/bal.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pin_subscription.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "mso_prov.h"
#include "mso_cust.h"
#include "pin_bill.h"
#include "pin_flds.h"
#include "fm_bal.h"
#include "ops/bill.h"
#include "ops/device.h"
#include "mso_errs.h"
#include "mso_lifecycle_id.h"

#define ACTIVATE_SERVICE 14
#define TASK_SET_PROV 1
#define TASK_SET_PLAN_STATUS 2
#define TASK_NOTIFICATION 3

/*******************************************************************************
 * Functions Defined outside this source file
 ******************************************************************************/
extern int32
fm_mso_trans_open(
    pcm_context_t    *ctxp,
    poid_t        *pdp,
    int32        flag,
    pin_errbuf_t    *ebufp);

extern void 
fm_mso_trans_commit(
    pcm_context_t    *ctxp,
    poid_t        *pdp,
    pin_errbuf_t    *ebufp);

extern void 
fm_mso_trans_abort(
    pcm_context_t    *ctxp,
    poid_t        *pdp,
    pin_errbuf_t    *ebufp);

/*******************************************************************
 * Fuctions defined in this code 
 *******************************************************************/

EXPORT_OP void
op_mso_prov_bb_process_response(
    cm_nap_connection_t    *connp,
    int32            opcode,
    int32            flags,
    pin_flist_t        *in_flistp,
    pin_flist_t        **ret_flistpp,
    pin_errbuf_t    *ebufp);

static void
fm_mso_prov_bb_process_response(
    pcm_context_t           *ctxp,
    pin_flist_t        *in_flistp,
    int32            flags,
    pin_flist_t        **out_flistpp,
    pin_errbuf_t    *ebufp);

extern void
fm_mso_prov_update_product_status(
	pcm_context_t		*ctxp, 
	poid_t			*acct_pdp, 
	poid_t			*service_obj,
	poid_t			*offering_pdp,
	char			*program_name,
	char                    *action,
	pin_flist_t             *payload_flistp,
	pin_errbuf_t		*ebufp);
	
static void
fm_mso_prov_failed_response(
	pcm_context_t		*ctxp, 
	pin_flist_t		*order_flistp,
	pin_flist_t		*in_flistp,
	int32			*task_failed, 
	pin_errbuf_t		*ebufp);

static void
fm_mso_update_provisioning_and_order_status(
	pcm_context_t		*ctxp,
	poid_t			*order_pdp,
	int32			ord_status,
	pin_flist_t		*payload_flistp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*tmp_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_update_purchased_plan_status(
	pcm_context_t		*ctxp,
	pin_flist_t		*payload_flistp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*tmp_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_prov_send_notifcation(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	int32			notification_flag,
	pin_errbuf_t		*ebufp);

static void
fm_mso_prov_apply_deferred_purchase_fees(
	pcm_context_t           *ctxp,
	poid_t			*acct_pdp,
	poid_t			*serv_pdp,
	char			*program_name,
	pin_errbuf_t            *ebufp);

static void
fm_mso_upd_prov_failed_response(
	pcm_context_t           *ctxp,
	pin_flist_t             *order_flistp,
	pin_flist_t             *in_flistp,
	int32                   *task_failed,
	pin_errbuf_t            *ebufp);

static void
fm_mso_search_failed_prov_response(
	pcm_context_t           *ctxp,
	pin_flist_t             *in_flistp,
	pin_flist_t             **ret_flistpp,
	pin_errbuf_t            *ebufp);
	
static void
fm_mso_update_ncr_validity(
	pcm_context_t		*ctxp,
	pin_flist_t		*payload_flistp,
	pin_errbuf_t		*ebufp);
	
static void
fm_mso_update_base_pp_obj(
	pcm_context_t           *ctxp,
	pin_flist_t             *in_flistp,
	pin_errbuf_t            *ebufp);

static void
get_topup_ncr_valid_to_fup(
	pcm_context_t           *ctxp,
	poid_t                  *svc_obj,
	poid_t                  *plan_obj,
	pin_flist_t             *in_flistp,
	time_t			*fup_valid_to,
	pin_errbuf_t            *ebufp);

static void
fm_mso_terminate_service_ip_dissociate(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_errbuf_t    *ebufp);

void
fm_mso_prov_update_product_cycle_fee_charge_dates(
        pcm_context_t   *ctxp,
        poid_t          *acct_pdp,
        poid_t          *service_obj,
        poid_t          *offering_obj,
	pin_flist_t     **ret_flistpp,
        pin_errbuf_t    *ebufp);

static void
fm_mso_modify_bal_grp(
        pcm_context_t           *ctxp,
        poid_t                  *bal_grp_pdp,
        pin_errbuf_t            *ebufp);

extern time_t
fm_prov_get_expiry(
        pcm_context_t                   *ctxp,
        poid_t                          *mso_plan,
        poid_t                          *ac_pdp,
        poid_t                          *srv_pdp,
        int                             srv_typ,
        time_t                          *start,
        pin_errbuf_t                    *ebufp);

extern void
fm_mso_cust_get_bp_obj(
        pcm_context_t           *ctxp,
        poid_t                  *account_obj,
        poid_t                  *service_obj,
        int                     mso_status,
        poid_t                  **bp_obj,
        poid_t                  **mso_obj,
        pin_errbuf_t            *ebufp);

/**function to get the active purchased_products's********/
extern void
fm_mso_get_purchased_prod_active(
        pcm_context_t           *ctxp,
        poid_t                  *inp_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_mso_update_only_order_status(
	pcm_context_t		*ctxp,
	poid_t			*order_pdp,
	pin_flist_t		*in_flistp,
	pin_errbuf_t		*ebufp);		

extern void
fm_mso_get_balances(
        pcm_context_t           *ctxp,
        poid_t                  *account_pdp,
        time_t                  bal_srch_t,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp);

void
fm_mso_change_plan_check_val_extn(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t             **r_flistpp,
	pin_errbuf_t            *ebufp);

/*******************************************************************
 * Main routine for the MSO_OP_PROV_BB_PROCESS_RESPONSE command
 *******************************************************************/
void
op_mso_prov_bb_process_response(
	cm_nap_connection_t    *connp,
	int32            opcode,
	int32            flags,
	pin_flist_t        *in_flistp,
	pin_flist_t        **ret_flistpp,
	pin_errbuf_t    *ebufp)
{
    pin_flist_t         *r_flistp = NULL;
    pcm_context_t       *ctxp = connp->dm_ctx;
    poid_t              *pdp = NULL;

	int32		*prov_direct_callp = NULL;
	int32		*task_levelp = NULL;
	
    /***********************************************************
     * Null out results until we have some.
     ***********************************************************/
    *ret_flistpp = NULL;
    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Insanity check.
     ***********************************************************/
    if (opcode != MSO_OP_PROV_BB_PROCESS_RESPONSE) {
	pin_set_err(ebufp, PIN_ERRLOC_FM,
	    PIN_ERRCLASS_SYSTEM_DETERMINATE,
		PIN_ERR_BAD_OPCODE, 0, 0, opcode);
	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
	    "bad opcode in op_mso_prov_bb_process_response", ebufp);
	return;
    }

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"op_mso_prov_bb_process_response input flist", in_flistp);

    /***********************************************************
     * Call main function to do it
     ***********************************************************/
    fm_mso_prov_bb_process_response(ctxp, in_flistp, flags, &r_flistp, ebufp);


    /***********************************************************
     * Results.
     ***********************************************************/
    if (PIN_ERRBUF_IS_ERR(ebufp)) 
    {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_prov_bb_process_response error", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,"op_mso_prov_bb_process_response modified input",in_flistp);
		prov_direct_callp = PIN_FLIST_FLD_GET(in_flistp,PIN_FLD_RESUBMIT,1,ebufp);
		task_levelp = PIN_FLIST_FLD_GET(in_flistp,MSO_FLD_TASK_FAILED,1,ebufp);
		if(prov_direct_callp && task_levelp)
		{
			if( *(int32 *)prov_direct_callp == 1)
			{
				fm_mso_prov_failed_response(ctxp, in_flistp, in_flistp, task_levelp, ebufp);
                        }
                        else
                   	{ 
                                fm_mso_upd_prov_failed_response(ctxp, in_flistp, in_flistp, task_levelp, ebufp);
                        }
		}			
		
		*ret_flistpp = PIN_FLIST_COPY(in_flistp,ebufp);
		//*ret_flistpp = (pin_flist_t *)NULL;
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
    } 
    else 
    {
		*ret_flistpp = r_flistp;
		PIN_ERRBUF_CLEAR(ebufp);
    }

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_mso_prov_bb_process_response return flist", *ret_flistpp);
    return;
}

/*******************************************************************
 * fm_mso_prov_bb_process_response()
 *
 * Perfomrs following action:
 * 1. Search order_id in /mso_bb_prov_order to get action and service_obj
 *        and package detail
 * 2. loop through the TASK array to check the STATUS of the order
 * 3. if all TASK element has STATUS successful, update order as 
 *     successful. else failed. 
 * 4. for CATV activation, additionally updates CAS_SUBSCRIBER_ID
 *      in service class
 *******************************************************************/
static void
fm_mso_prov_bb_process_response(
    pcm_context_t           *ctxp,
    pin_flist_t             *in_flistp,
    int32                   flags,
    pin_flist_t             **out_flistpp,
    pin_errbuf_t            *ebufp)
{
    int32                   *action_flag = NULL;
    int                     *status = NULL;
    int                     *activation_status = NULL;
    char                    *order_id = NULL;
    char                    *error_code = NULL;
    char                    *error_descr = NULL;
    char                    *cas_id = NULL;
    poid_t                  *pdp = NULL;
    poid_t                  *service_obj = NULL;
    poid_t                  *order_poid = NULL;
    poid_t                  *offering_obj = NULL;
    poid_t                  *acct_pdp = NULL;
    char                    *service_type = NULL;
    int32                   s_flags;
    int64                   db_no;
    char                    *template = NULL;
    poid_t                  *s_pdp = NULL;
    poid_t                  *order_pdp = NULL;
    poid_t                  *acc_pdp = NULL;
    poid_t                  *dummy = NULL;
    int32                   count;
    pin_flist_t             *search_i_flistp = NULL;
    pin_flist_t             *w_i_flistp = NULL;
    pin_flist_t             *w_o_flistp = NULL;
    pin_flist_t             *u_i_flistp = NULL;
    pin_flist_t             *u_o_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    pin_flist_t             *task_flistp = NULL;
    pin_flist_t             *telco_feature = NULL;
    pin_flist_t             *payload_flistp = NULL;
    pin_flist_t             *inherited = NULL;
    pin_flist_t             *services = NULL;
    pin_flist_t			*products = NULL;
    pin_flist_t			*bb_info = NULL;
    pin_flist_t			*cobj_iflistp = NULL;
    pin_flist_t			*cobj_oflistp = NULL;
    char			*ext_ref_no = NULL;
    char			*group = NULL;
    char			*login = NULL;
    time_t			*login_t = NULL;
    char			*source = NULL;
    char                    *action = NULL;
    char                    *flist = NULL;
    char		    *neid = NULL;
    char		    *mac_id = NULL;
    int32                   task_id = 0;
    int32		    rec_id1 = 0;
    int32		    elem_id = 0;
    pin_cookie_t            cookie = NULL;
    pin_cookie_t	    cookie1 = NULL;
    pin_cookie_t	    cookie2 = NULL;
    int32                   prov_status  = MSO_PROV_STATE_SUCCESS;
    int32                   ord_status = MSO_PROV_STATE_SUCCESS;
    int32                   srv_status = 1;
    int32                   srv_status_act = 2;
    int32                   prov_status_failed = 3;
    int32		    tmp_svc_status = 0;
    int32		    status_act_flag = 4;
    int32		    notification_flag = ACTIVATE_SERVICE;
    int			    *act_flag = NULL;
    int64                   db = 0;
    char		    *program_name = NULL;
    pin_buf_t               *nota_buf = NULL;
    int32		    task_level = 0;
    int32                   t_status;
    void		    *vp = NULL;
    int			    prov_direct_call = 0;
    char		    *action_nm = NULL;	
    char		    *prog_name = NULL;
	int                     local = 1;
	pcm_context_t           *new_ctxp = NULL;
	time_t          current_t = 0;
	pin_flist_t		*cycle_fwd_iflistp = NULL;
	pin_flist_t		*cycle_fwd_oflistp = NULL;
	pin_flist_t		*products_flist = NULL;
	     pin_flist_t             *robj_oflistp = NULL;
	     pin_flist_t             *robj_iflistp = NULL;
     int32                   rate_flag  = 0;
	int32			billinfo_type = BB;	
	pin_flist_t             *billinfo_details = NULL;
	pin_flist_t             *result_flist  = NULL;
	poid_t			*billinfo_pdp = NULL;
	//VALIDITY EXTENSION VARIABLES
	pin_flist_t		*val_temp_flistp = NULL;
	pin_flist_t		*subscr_prods_flistp=NULL;
	pin_flist_t		*ret_valid_flistp=NULL;
	pin_flist_t		*last_plan_flistp=NULL;
	pin_flist_t		*vex_ip_flistp=NULL;
	pin_flist_t		*ext_val_ret_flistp=NULL;
	pin_flist_t		*ret_ext_flistp=NULL;
	poid_t			*serv_vex_poid = NULL;
	poid_t			*act_vex_poid = NULL;
	poid_t			*plan_pdp = NULL;
	poid_t			*prov_ord_poid =NULL;
	time_t			*prod_end_t = NULL;
	time_t			*val_ext_end = NULL;
	char			*val_ext_valid = NULL;
	time_t		       current_t1 = pin_virtual_time(NULL);
	pin_decimal_t		*ext_dec = pbo_decimal_from_str("0.0", ebufp);
	pin_decimal_t		*val_ext_v = pbo_decimal_from_str("0.0", ebufp);
	double			ext_days=0;
	int32 			allow_ext_flag=0;
	int32 			prov_val_status=0;
	int32			acct_update_extv=0;
	char	  		*s1=NULL;
	char	  		*s2=NULL;


    if (PIN_ERRBUF_IS_ERR(ebufp))
	return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"fm_mso_prov_bb_process_response input flist", in_flistp);

    /***********************************************************
     * Check mandatory fields are available
     ***********************************************************/
    order_id = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ORDER_ID, 0, ebufp);
    count = PIN_FLIST_ELEM_COUNT(in_flistp, MSO_FLD_TASK, ebufp);
    if (PIN_ERRBUF_IS_ERR(ebufp) ) {
	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
	    "fm_mso_prov_bb_process_response error: required field missing in input flist", ebufp);
	return;
    }

    if (count == 0)
    {
	pin_set_err(ebufp, PIN_ERRLOC_FM,
	    PIN_ERRCLASS_SYSTEM_DETERMINATE,
		PIN_ERR_NOT_FOUND, MSO_FLD_TASK, 0, 0);
	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
	    "fm_mso_prov_bb_process_response error: required field missing in input flist", ebufp);
	return;
    }

    /***********************************************************
     * Search provisioning order object
     ***********************************************************/
    pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
    db_no = PIN_POID_GET_DB(pdp); 

    search_i_flistp = PIN_FLIST_CREATE(ebufp);
    s_pdp = (poid_t *)PIN_POID_CREATE(db_no, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(search_i_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

    s_flags = SRCH_DISTINCT ;
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

    template = "select X from /mso_prov_bb_order where F1 = V1 ";
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ORDER_ID, order_id, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_ACTION, NULL, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_INPUT_FLIST, NULL, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_POID, NULL, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_PROGRAM_NAME, NULL, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_STATUS, NULL, ebufp);


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"fm_mso_prov_bb_process_response search input flist ", search_i_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_i_flistp,
	out_flistpp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"fm_mso_prov_bb_process_response search output flist", *out_flistpp);

    PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);


    /************CHANGES FOR VALIDITY EXTN PART 1************/		

	if(*out_flistpp && *out_flistpp != NULL && PIN_FLIST_ELEM_COUNT(*out_flistpp, PIN_FLD_RESULTS, ebufp) > 0)
	{
	   val_temp_flistp = PIN_FLIST_ELEM_GET(*out_flistpp,PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
	   prov_ord_poid = PIN_FLIST_FLD_GET(val_temp_flistp, PIN_FLD_POID, 1, ebufp);
	   PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "OUTPUT FLIST FROM PROV", val_temp_flistp);
	   
	   if (!((strstr(PIN_FLIST_FLD_GET(val_temp_flistp, PIN_FLD_ACTION, 1, ebufp),"CHANGE_SERVICES"))==0) || 
		!((strstr(PIN_FLIST_FLD_GET(val_temp_flistp, PIN_FLD_ACTION, 1, ebufp),"TOPUP_WO_PLAN"))==0) ||
		!((strstr(PIN_FLIST_FLD_GET(val_temp_flistp, PIN_FLD_ACTION, 1, ebufp),"RENEW_SUBSCRIBER"))==0))
	   {
		serv_vex_poid = PIN_FLIST_FLD_GET(val_temp_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		act_vex_poid = PIN_FLIST_FLD_GET(val_temp_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
		prov_val_status =*(int32 *)PIN_FLIST_FLD_GET(val_temp_flistp, PIN_FLD_STATUS, 1, ebufp);
		fm_mso_get_subscr_purchased_products(ctxp, act_vex_poid, serv_vex_poid, &subscr_prods_flistp, ebufp);
       
	       if (subscr_prods_flistp && subscr_prods_flistp != NULL && PIN_FLIST_ELEM_COUNT(subscr_prods_flistp, PIN_FLD_RESULTS, ebufp) > 0 && prov_val_status && prov_val_status != 0)
              {
			last_plan_flistp = PIN_FLIST_ELEM_GET(subscr_prods_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
			plan_pdp = PIN_FLIST_FLD_GET(last_plan_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
			prod_end_t = PIN_FLIST_FLD_GET(last_plan_flistp, PIN_FLD_CYCLE_END_T, 1, ebufp);
			fm_mso_change_plan_check_val_extn(ctxp,last_plan_flistp,&ret_valid_flistp,ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "CHANGE PLAN CHECK VAL OUTPUT", ret_valid_flistp);
			
			if(ret_valid_flistp && ret_valid_flistp != NULL && PIN_FLIST_ELEM_COUNT(ret_valid_flistp, PIN_FLD_RESULTS, ebufp) > 0)
			{
				val_temp_flistp = PIN_FLIST_ELEM_GET(ret_valid_flistp,PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
				s1=PIN_FLIST_FLD_GET(val_temp_flistp,PIN_FLD_CLASSMARK,1,ebufp);
				val_ext_valid=PIN_FLIST_FLD_GET(val_temp_flistp,MSO_FLD_EXPIRATION_DATE,1,ebufp);
				s2=s1+1;
			    if(s1 && s1 != NULL && val_ext_valid && val_ext_valid != NULL && s2 != NULL)
			    {		
				ext_dec=pbo_decimal_from_str(s2, ebufp);
				ext_days=pbo_decimal_to_double(ext_dec,ebufp);
				val_ext_v=pbo_decimal_from_str(val_ext_valid, ebufp);
				val_ext_end= (time_t *)(*prod_end_t + abs(ext_days*60.0*60.0*24.0));

				if((double)fm_utils_time_round_to_midnight(current_t1) <= pbo_decimal_to_double(val_ext_v,ebufp))
				{
				  allow_ext_flag = 1;
				  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"FLAG SETTED TO ALLOW");
				}
				/*******************************************************************
				* Input Preparation for Validity Extension Customization
				*******************************************************************/
				PIN_ERRBUF_CLEAR(ebufp);
	 			vex_ip_flistp = PIN_FLIST_CREATE(ebufp);
	  			PIN_FLIST_FLD_COPY(last_plan_flistp,PIN_FLD_ACCOUNT_OBJ,vex_ip_flistp,PIN_FLD_POID,ebufp);
				PIN_FLIST_FLD_COPY(last_plan_flistp,PIN_FLD_ACCOUNT_OBJ,vex_ip_flistp,PIN_FLD_USERID,ebufp);
	  			PIN_FLIST_FLD_COPY(last_plan_flistp,PIN_FLD_SERVICE_OBJ,vex_ip_flistp,PIN_FLD_SERVICE_OBJ,ebufp);
				PIN_FLIST_FLD_SET(vex_ip_flistp, PIN_FLD_REASON_CODE, "VAL_EXT_PROMO_OFFER", ebufp);
	  			PIN_FLIST_FLD_SET(vex_ip_flistp, PIN_FLD_PROGRAM_NAME, "PROMO_INIT_OFFER", ebufp);
				PIN_FLIST_FLD_SET(vex_ip_flistp, PIN_FLD_END_T, &val_ext_end, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "IP FLIST FOR VALIDITY EXTENSION", vex_ip_flistp);
			    }
			    else 
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"NOT A VALIDITY EXTENSION OFFER...");	
			}
	       }
		else
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"PROV STATUS IS SUCCESS ALREADY HENCE BYPASSING FLOW...");
	   }
	}
    /*************************************************/

 	/****** Pavan Bellala 18-11-2015 *********
	Add more fields to input flist to use later
	******************************************/
	tmp_flistp = PIN_FLIST_ELEM_GET(*out_flistpp,PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
	if(tmp_flistp != NULL)
	{
		PIN_FLIST_FLD_COPY(tmp_flistp,PIN_FLD_ACCOUNT_OBJ,in_flistp,PIN_FLD_ACCOUNT_OBJ,ebufp);
		PIN_FLIST_FLD_COPY(tmp_flistp,PIN_FLD_ACCOUNT_OBJ,in_flistp,PIN_FLD_POID,ebufp);
		PIN_FLIST_FLD_COPY(tmp_flistp,PIN_FLD_SERVICE_OBJ,in_flistp,PIN_FLD_SERVICE_OBJ,ebufp);
		PIN_FLIST_FLD_COPY(tmp_flistp,PIN_FLD_ACTION,in_flistp,PIN_FLD_ACTION,ebufp);
		PIN_FLIST_FLD_COPY(tmp_flistp,PIN_FLD_PROGRAM_NAME,in_flistp,PIN_FLD_PROGRAM_NAME,ebufp);
		PIN_FLIST_FLD_COPY(tmp_flistp,PIN_FLD_INPUT_FLIST,in_flistp,PIN_FLD_INPUT_FLIST,ebufp);
		
	}

    count = PIN_FLIST_ELEM_COUNT(*out_flistpp, PIN_FLD_RESULTS, ebufp);


    /***********************************************************
     * There should be exactly on matching result
     ***********************************************************/
    if (count == 1)
    {
		prog_name = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
		order_pdp = (poid_t *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_POID, 0, ebufp);
		if (prog_name && strcmp(prog_name, "BB_GRACE_JOB") == 0)
		{
			fm_mso_update_only_order_status(ctxp, order_pdp, in_flistp, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "BB grace dont update anything");
			goto cleanup;
		}
		if (prog_name && strcmp(prog_name, "Bulk_Change_Service_Code") == 0)
		{
			// Update only order Status for bulk utility
			while ( (tmp_flistp = PIN_FLIST_ELEM_GET_NEXT (in_flistp,
					MSO_FLD_TASK, &task_id, 1,&cookie, ebufp))
								  != (pin_flist_t *)NULL ) 
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "inside TASK loop");
				status = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_STATUS, 0, ebufp);
				if (*status == MSO_PROV_STATE_NEW)
				{
					pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_BAD_VALUE, PIN_FLD_STATUS, 0, 0);
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"fm_mso_prov_bb_process_response error: Wrong status for bulk in input flist", ebufp);
					return;					
				}
			}




			fm_mso_update_only_order_status(ctxp, order_pdp, in_flistp, ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "order found");
		status = &prov_status;

		while ( (tmp_flistp = PIN_FLIST_ELEM_GET_NEXT (in_flistp,
				MSO_FLD_TASK, &task_id, 1,&cookie, ebufp))
							  != (pin_flist_t *)NULL ) 
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "inside TASK loop");
			status = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_STATUS, 0, ebufp);
			if (*status != MSO_PROV_STATE_SUCCESS)
			{
			error_code = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_ERROR_CODE, 0, ebufp);
			error_descr = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_ERROR_DESCR, 0, ebufp);
			break;
			}
		}

		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_prov_bb_process_response error: required field missing in input flist", ebufp);
			return;
		}

		tmp_flistp = PIN_FLIST_ELEM_GET(*out_flistpp, PIN_FLD_RESULTS, 0, 0, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Temp Flist", tmp_flistp);

		if (*status == MSO_PROV_STATE_SUCCESS)
		{
			prov_status = MSO_PROV_STATE_SUCCESS;
			ord_status = MSO_PROV_STATE_SUCCESS;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Successful at NE");
			service_obj = (poid_t *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp); 
			action = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_ACTION, 0, ebufp);
			PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_ACTION, action, ebufp);
			acct_pdp = (poid_t *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
			nota_buf = (pin_buf_t *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_INPUT_FLIST, 0, ebufp);
			order_pdp = (poid_t *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_POID, 0, ebufp);
			vp = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_TASK_FAILED, 1, ebufp);
			program_name = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_PROGRAM_NAME, 0, ebufp);
			PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_PROGRAM_NAME, program_name, ebufp);
			//PIN_ERR_LOG_MSG(3,"Debug1");
			if (vp == NULL)
			{
				prov_direct_call = 1;
				task_level = TASK_SET_PROV;
			}
			else
			{
				task_level = *(int32 *)vp;
			}
			//PIN_ERR_LOG_MSG(3,"Debug2");
			PIN_FLIST_FLD_SET(in_flistp,PIN_FLD_RESUBMIT,&prov_direct_call,ebufp);
			PIN_FLIST_FLD_SET(in_flistp,MSO_FLD_TASK_FAILED,&task_level,ebufp);
	
			if (PIN_ERRBUF_IS_ERR(ebufp)) {
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"fm_mso_prov_bb_process_response error: Required field missing", ebufp);
				goto cleanup;
			}
			
			//Get the Purchased Plan obj
			if(nota_buf){
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, (char *)nota_buf->data);
				PIN_STR_TO_FLIST((char *)nota_buf->data, 1, &payload_flistp, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp)) {
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						"fm_mso_prov_bb_process_response error: Error", ebufp);
					goto cleanup;
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
					"fm_mso_prov_bb_process_response: Payload Flist", payload_flistp);
				offering_obj = (poid_t *)PIN_FLIST_FLD_GET(payload_flistp,
					MSO_FLD_PURCHASED_PLAN_OBJ, 0, ebufp);
			}

			
			if(task_level == TASK_SET_PROV)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Inside TASK_SET_PROV");
				pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
				/* Changing the status back to read write since no poid will be passed */
				t_status = fm_mso_trans_open(ctxp, pdp, READWRITE, ebufp);

				// Update the Order and Provisioning Status
				fm_mso_update_provisioning_and_order_status(ctxp, order_pdp, ord_status, payload_flistp,
												in_flistp, tmp_flistp, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERRBUF_CLEAR(ebufp);
					if(t_status == 0)
					{
						fm_mso_trans_abort(ctxp, pdp, ebufp);
					}
					if(prov_direct_call)
					{	// Create a new failed class for the first time
						fm_mso_prov_failed_response(ctxp, payload_flistp, in_flistp, &task_level, ebufp);
					}
					else
					{	// Update the existing failed class
						fm_mso_upd_prov_failed_response(ctxp, payload_flistp, in_flistp, &task_level, ebufp);
					}
					goto cleanup;
				}
				else
				{
					if(t_status == 0)
					{
						fm_mso_trans_commit(ctxp, pdp, ebufp);
					}
					task_level = TASK_SET_PLAN_STATUS;
				}
			}

			if(task_level == TASK_SET_PLAN_STATUS)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Inside TASK_SET_PLAN_STATUS");
				pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);



				t_status = fm_mso_trans_open(ctxp, pdp, READWRITE, ebufp);

				// Update the Purchase Plan and Product Status
				fm_mso_update_purchased_plan_status(ctxp, payload_flistp,
											in_flistp, tmp_flistp, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERRBUF_CLEAR(ebufp);
					if(t_status == 0)
					{
						fm_mso_trans_abort(ctxp, pdp, ebufp);
					}
					if(prov_direct_call)
					{
						fm_mso_prov_failed_response(ctxp, payload_flistp, in_flistp, &task_level, ebufp);
					}
					else
					{
						fm_mso_upd_prov_failed_response(ctxp, payload_flistp, in_flistp, &task_level, ebufp);
					}
					goto cleanup;
				}
				else
				{
					if(t_status == 0)
					{
						fm_mso_trans_commit(ctxp, pdp, ebufp);
					}
					task_level = TASK_NOTIFICATION;
				}
			}

			if(task_level == TASK_NOTIFICATION)
			{
			    if(strstr(action, "ACTIVATE_SUBSCRIBER") && !strstr(action, "DEACTIVATE_SUBSCRIBER"))
			    {
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Inside TASK_NOTIFICATION");
				pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
				t_status = fm_mso_trans_open(ctxp, pdp, READWRITE, ebufp);
				// Send Notification and welcome-note
				fm_mso_prov_send_notifcation(ctxp, payload_flistp, notification_flag, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERRBUF_CLEAR(ebufp);
					if(t_status == 0)
					{
						fm_mso_trans_abort(ctxp, pdp, ebufp);
					}
					if(prov_direct_call)
					{
						fm_mso_prov_failed_response(ctxp, payload_flistp, in_flistp, &task_level, ebufp);
					}
					else
					{
						fm_mso_upd_prov_failed_response(ctxp, payload_flistp, in_flistp, &task_level, ebufp);
					}
					goto cleanup;
				}
				else
				{
					task_level = 0;
					// If the order is successful in re-processing, then update the task_level to 0
					if(!prov_direct_call)
						fm_mso_upd_prov_failed_response(ctxp, payload_flistp, in_flistp, &task_level, ebufp);

					if(t_status == 0)
					{
						fm_mso_trans_commit(ctxp, pdp, ebufp);
					}
				}
			    }
			}

			/******* Pavan Bellala 06-11-2015 *********
			In case of terminate order, the Logical IPs
			are to be freed
			******************************************/
			if(strstr(action,"TERMINATE_SUSBCRIBER_DOCSIS") || strstr(action,"TERMINATE_SUSBCRIBER_FIBER"))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Terminate service order");
				fm_mso_terminate_service_ip_dissociate(ctxp, tmp_flistp, ebufp);
				if(PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in dissociate IPs",ebufp);
					goto cleanup;

				}
			}

		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Failed at NE");
			prov_status = MSO_PROV_STATE_FAILED_PROVISIONING;
			ord_status = MSO_PROV_STATE_FAILED_PROVISIONING;
			service_obj = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp); 
			action = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_ACTION, 0, ebufp);
			order_poid = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_POID, 0, ebufp);
			acct_pdp = (poid_t *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
			nota_buf = (pin_buf_t *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_INPUT_FLIST, 0, ebufp);

			/***********************************************************
			 * Update /service/telco/broadband class with status failed
			 *  and error_descr
			 ***********************************************************/
			w_i_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(w_i_flistp, PIN_FLD_POID, (void *)acct_pdp, ebufp);
			PIN_FLIST_FLD_SET(w_i_flistp, PIN_FLD_PROGRAM_NAME, program_name, ebufp);
			services = PIN_FLIST_ELEM_ADD(w_i_flistp, PIN_FLD_SERVICES, 0, ebufp);
			PIN_FLIST_FLD_SET(services, PIN_FLD_POID, service_obj, ebufp);
			inherited = PIN_FLIST_SUBSTR_ADD(services, PIN_FLD_INHERITED_INFO, ebufp);
			telco_feature = PIN_FLIST_ELEM_ADD(inherited, PIN_FLD_TELCO_FEATURES, 0, ebufp);
			//PIN_FLIST_FLD_SET(telco_feature, PIN_FLD_STATUS, (void *)&srv_status, ebufp);
			//Fix done to set correct service provisioning status
			PIN_FLIST_FLD_SET(telco_feature, PIN_FLD_STATUS, (void *)&prov_status_failed, ebufp);
			PIN_FLIST_FLD_SET(telco_feature, PIN_FLD_NAME, "PROVISIONING_STATUS", ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"fm_mso_prov_bb_process_response: Write Fields Input Flist", w_i_flistp);
			PCM_OP(ctxp, PCM_OP_CUST_UPDATE_SERVICES, 0, w_i_flistp, &w_o_flistp, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp)) {
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"fm_mso_prov_bb_process_response error: Error", ebufp);
				goto cleanup;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"fm_mso_prov_bb_process_response: Write Fields Output Flist", w_o_flistp);
			PIN_FLIST_DESTROY_EX(&w_i_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&w_o_flistp, NULL);
			
			//Get the Purchased Plan obj
			PIN_STR_TO_FLIST((char *)nota_buf ->data, 1, &payload_flistp, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp)) {
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"fm_mso_prov_bb_process_response error: Error", ebufp);
				goto cleanup;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
				"fm_mso_prov_bb_process_response: Payload Flist", payload_flistp);
			offering_obj = (poid_t *)PIN_FLIST_FLD_GET(payload_flistp,
				MSO_FLD_PURCHASED_PLAN_OBJ, 0, ebufp);
			action_nm = (char *)PIN_FLIST_FLD_GET(payload_flistp,
                                PIN_FLD_ACTION, 0, ebufp);	
			if(action_nm && (strstr(action_nm, "UNHOLD_SERVICES") || strstr(action_nm,"CHANGE_MODEM") 
                             || strstr(action_nm,"CHANGE_SERVICES") || strstr(action_nm,"RENEW_SUBSCRIBER") || strstr(action_nm, "ACTIVATE_SUBSCRIBER")))
			{
				//Update the status in purchased plan
				w_i_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(w_i_flistp, PIN_FLD_POID, (void *)offering_obj, ebufp);
				PIN_FLIST_FLD_SET(w_i_flistp, PIN_FLD_STATUS, (void *)&srv_status, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"fm_mso_prov_bb_process_response: Write Fields Input Flist", w_i_flistp);
				PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 32, w_i_flistp, &w_o_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&w_i_flistp, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp)) {
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						"fm_mso_prov_bb_process_response error: Error", ebufp);
					goto cleanup;
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"fm_mso_prov_bb_process_response: Write Fields Output Flist", w_o_flistp);
				PIN_FLIST_DESTROY_EX(&w_o_flistp, NULL);
			}
			else
			{
				w_i_flistp = PIN_FLIST_CREATE(ebufp);
                                PIN_FLIST_FLD_SET(w_i_flistp, PIN_FLD_POID, (void *)offering_obj, ebufp);
                                PIN_FLIST_FLD_SET(w_i_flistp, PIN_FLD_STATUS, (void *)&srv_status_act, ebufp);
                                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                        "fm_mso_prov_bb_process_response: Write Fields Input Flist", w_i_flistp);
                                PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 32, w_i_flistp, &w_o_flistp, ebufp);
                                PIN_FLIST_DESTROY_EX(&w_i_flistp, NULL);
                                if (PIN_ERRBUF_IS_ERR(ebufp)) {
                                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                                "fm_mso_prov_bb_process_response error: Error", ebufp);
                                        goto cleanup;
                                }
                                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                        "fm_mso_prov_bb_process_response: Write Fields Output Flist", w_o_flistp);
                                PIN_FLIST_DESTROY_EX(&w_o_flistp, NULL);
			}
			//Update the status in the order
			w_i_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(w_i_flistp, PIN_FLD_POID, (void *)order_poid, ebufp);
			PIN_FLIST_FLD_SET(w_i_flistp, PIN_FLD_STATUS, (void *)&ord_status, ebufp);
			PIN_FLIST_FLD_SET(w_i_flistp, PIN_FLD_ERROR_CODE, (void *)error_code, ebufp);
			PIN_FLIST_FLD_SET(w_i_flistp, PIN_FLD_ERROR_DESCR, (void *)error_descr, ebufp);
			PIN_FLIST_FLD_SET(w_i_flistp, MSO_FLD_TASK_FAILED, (void *)&task_id, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"fm_mso_prov_bb_process_response: Write Fields Input Flist", w_i_flistp);
			PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 32, w_i_flistp, &w_o_flistp, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp)) {
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"fm_mso_prov_bb_process_response error: Error", ebufp);
				goto cleanup;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"fm_mso_prov_bb_process_response: Write Fields Output Flist", w_o_flistp);
			PIN_FLIST_DESTROY_EX(&w_i_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&w_o_flistp, NULL);
		}
    }
    else
    {
	pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_ORDER_ID, 0, 0);
	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"Invalid Order_Id", ebufp);
    }


cleanup:
	PIN_ERRBUF_CLEAR(ebufp);
	//PIN_FLIST_DESTROY_EX(&w_i_flistp, NULL);
	//PIN_FLIST_DESTROY_EX(&w_o_flistp, NULL);
	//PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);
	//PIN_FLIST_DESTROY_EX(&payload_flistp, NULL);
	//PIN_FLIST_DESTROY_EX(&cobj_oflistp, NULL);
	//PIN_FLIST_DESTROY_EX(&cobj_iflistp, NULL);
	//if(!PIN_POID_IS_NULL(dummy))
	//	PIN_POID_DESTROY(dummy, ebufp);

    /************CHANGES FOR VALIDITY EXTN PART 2************/	
	robj_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(robj_iflistp, PIN_FLD_POID,prov_ord_poid, ebufp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, robj_iflistp, &robj_oflistp, ebufp);	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "READ OBJ BEFORE VALIDITY EXTENSION OUTPUT", robj_oflistp);
	prov_val_status =*(int32 *)PIN_FLIST_FLD_GET(robj_oflistp, PIN_FLD_STATUS, 1, ebufp);
	if(allow_ext_flag && !PIN_ERRBUF_IS_ERR(ebufp) && prov_val_status == 0)
	{
		acct_update_extv = fm_mso_extend_validity(ctxp, vex_ip_flistp, &ext_val_ret_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "VALIDITY EXTENSION OUTPUT INSIDE PROV", ext_val_ret_flistp);
	
		if ( acct_update_extv != 2 || !ext_val_ret_flistp || PIN_ERRBUF_IS_ERR(ebufp))
		{
			if ( acct_update_extv == 0)
			{		
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account information update failed");
			}
			if ( acct_update_extv == 3)
			{		
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_extend_validity insufficient data provided");	
			}
			if ( acct_update_extv == 4)
			{		
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_extend_validity no service existing with this service poid");	
			}
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "VALIDITY EXTENSION FAILED INSIDE PROV FOR SOME REASON.");
		}
		else
		{		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_extend_validity status change successful");
			fm_mso_create_lifecycle_evt(ctxp, vex_ip_flistp, ret_ext_flistp, ID_VALIDITY_EXTENTION_FOR_A_SERVICE_PLAN, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "LIFECYCLE CREATION OUTPUT INSIDE PROV", ret_ext_flistp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
				PIN_ERRBUF_RESET(ebufp);
			}
		}			   	
	}
	else
	{
          PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "EITHER OFFER IS NOT PRESENT FOR THIS PLAN OR ERROR WHILE PROV.");
	}
       PIN_FLIST_DESTROY_EX(&robj_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&robj_oflistp, NULL);
    /************CHANGES FOR VALIDITY EXTN PART 2************/	

	return;
}


/***********************************************************
* fm_mso_prov_update_product_status: Function to set
* the Product status
***********************************************************/
extern void
fm_mso_prov_update_product_status(
	pcm_context_t		*ctxp, 
	poid_t			*acct_pdp, 
	poid_t			*service_obj,
	poid_t			*offering_pdp,
	char			*program_name,
	char                    *action,
        pin_flist_t             *payload_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*sp_iflistp = NULL;
	pin_flist_t		*sp_oflistp = NULL;
	pin_flist_t		*robj_iflistp = NULL;
	pin_flist_t		*robj_oflistp = NULL;
	pin_flist_t		*prods = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*purch_flistp = NULL;
	pin_flist_t		*product_oflistp = NULL;
	pin_flist_t		*cfee_flistp = NULL;
	pin_flist_t		*usage_flistp = NULL;
	pin_flist_t		*wflds_iflistp = NULL;
	pin_flist_t		*wflds_oflistp = NULL;
	pin_flist_t		*pp_read_iflist = NULL;
	pin_flist_t		*pp_read_oflist = NULL;
	pin_flist_t		*read_res_flistp = NULL;
	pin_flist_t		*read_flistp = NULL;
	pin_flist_t		*write_iflistp = NULL;
	pin_flist_t		*write_oflistp = NULL;
	pin_flist_t		*rslt_oflistp = NULL;

	poid_t			*pp_pdp = NULL;
	poid_t			*prod_pdp = NULL;
	//poid_t			*s_pdp = NULL;
	//int32			s_flags = SRCH_DISTINCT;
	int32			elem_id = 0;
	int64			db_no = 0;
	pin_cookie_t		cookie = NULL;
	time_t			now = 0;
	time_t			*grace_start = NULL;
	char			*prog_namep="set_prod_status_after_prov";
	int			status = PIN_PRODUCT_STATUS_ACTIVE;
	int			status_flags = 64;
	int                     prty = 0;
	int32                   mod_prty=0;
	pin_decimal_t           *priority = NULL;
	char                    *strp = NULL;
	char                    *evt_type = NULL;
	time_t                  *purch_end_t = NULL;
	int32                   *prov_flag = NULL;
	pin_flist_t             *cycle_fwd_oflistp = NULL;
	
	time_t			valid_from = 0;
	int32                   elem_id1 = 0;
        pin_cookie_t            cookie1 = NULL;
	int32                   elem_id2 = 0;
        pin_cookie_t            cookie2 = NULL;
	time_t			time_now = 0;
	pin_flist_t		*write_out_flistp = NULL;

	
	
				
	//char			*tmpl = "select X from /purchased_product 1, /mso_plans_activation 2 where 2.F1 = V1 and 2.F2 = V2 and 1.F3 = 2.F4 ";

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);






	//Read the /mso_purchased_plan object
	robj_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(robj_iflistp, PIN_FLD_POID, (void *)offering_pdp, ebufp);
	//Call the PCM_OP_READ_OBJ opcode
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, robj_iflistp, &robj_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_prov_update_product_status: Error", ebufp);
		PIN_FLIST_DESTROY_EX(&robj_iflistp, NULL);
		return;
	}

//	db_no = PIN_POID_GET_DB(acct_pdp);
//	s_pdp = (poid_t *)PIN_POID_CREATE(db_no, "/search", -1, ebufp);
//	robj_iflistp = PIN_FLIST_CREATE(ebufp);
//	PIN_FLIST_FLD_PUT(robj_iflistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
//	PIN_FLIST_FLD_SET(robj_iflistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
//	PIN_FLIST_FLD_SET(robj_iflistp, PIN_FLD_TEMPLATE, (void *)tmpl, ebufp);
//	//-----Add 1st Arg-----
//	args_flistp = PIN_FLIST_ELEM_ADD(robj_iflistp, PIN_FLD_ARGS, 1, ebufp);
//	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, (void *)acct_pdp, ebufp);
//	//-----Add 2nd Arg-----
//	args_flistp = PIN_FLIST_ELEM_ADD(robj_iflistp, PIN_FLD_ARGS, 2, ebufp);
//	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, (void *)service_obj, ebufp);
//	//-----Add 3rd Arg-----
//	args_flistp = PIN_FLIST_ELEM_ADD(robj_iflistp, PIN_FLD_ARGS, 3, ebufp);
//	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, (void *)NULL, ebufp);
//	//-----Add 4th Arg-----
//	args_flistp = PIN_FLIST_ELEM_ADD(robj_iflistp, PIN_FLD_ARGS, 4, ebufp);
//	tmp_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_PLAN, PIN_ELEMID_ANY, ebufp);
//	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_PLAN_OBJ, (void *)NULL, ebufp);
//	//-----Add Result Arr
//	args_flistp = PIN_FLIST_ELEM_ADD(robj_iflistp, PIN_FLD_RESULTS, 0, ebufp);
//	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, (void *)NULL, ebufp);
//	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACTG_NEXT_T, (void *)NULL, ebufp);
//	if(PIN_ERRBUF_IS_ERR(ebufp)){
//		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_prov_update_product_status: Error", ebufp);
//		goto cleanup;
//	}
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_prov_update_product_status: Search Input Flist ", robj_iflistp);
//	//Call the PCM_OP_SEARCH
//	PCM_OP(ctxp, PCM_OP_SEARCH, 0, robj_iflistp, &robj_oflistp, ebufp);
//	if(PIN_ERRBUF_IS_ERR(ebufp)){
//		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_prov_update_product_status: Error", ebufp);
//		goto cleanup;
//	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_prov_update_product_status: Read Obj Output Flist ", robj_oflistp);

	  
	prov_flag = (int32 *)PIN_FLIST_FLD_GET(payload_flistp, MSO_FLD_PROVISIONABLE_FLAG, 1, ebufp);	

		cookie = NULL;elem_id = 0;
	//Itrate through the Products Array and call the set product status opcode
	while ( (prods = PIN_FLIST_ELEM_GET_NEXT (robj_oflistp, PIN_FLD_PRODUCTS, 
		&elem_id, 1,&cookie, ebufp)) != (pin_flist_t *)NULL )
	{
		now = pin_virtual_time(NULL);

		pp_pdp = (poid_t *)PIN_FLIST_FLD_GET(prods, MSO_FLD_PURCHASED_PRODUCT_OBJ, 0, ebufp);
		prod_pdp = (poid_t *)PIN_FLIST_FLD_GET(prods, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)){
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_prov_update_product_status: Error", ebufp);
			goto cleanup;
		}
		//Call the SET_PRODUCT_STATUS opcode
		sp_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(sp_iflistp, PIN_FLD_POID, (void *)acct_pdp, ebufp);
		if (program_name != NULL)
		{
			PIN_FLIST_FLD_SET(sp_iflistp, PIN_FLD_PROGRAM_NAME, program_name, ebufp);
		}
		else
		{
			PIN_FLIST_FLD_SET(sp_iflistp, PIN_FLD_PROGRAM_NAME, prog_namep, ebufp);
		}
		PIN_FLIST_FLD_SET(sp_iflistp, PIN_FLD_DESCR, (void *)"Set status after provisioning", ebufp);
		PIN_FLIST_FLD_SET(sp_iflistp, PIN_FLD_SERVICE_OBJ, (void *)service_obj, ebufp);

		/* Set the purchase product cycle_forward_t as current_t . Fix for duplicte charging */
		cookie1 = NULL;elem_id1 = 0;
		if(valid_from == 0)
		{
			while ((args_flistp = PIN_FLIST_ELEM_GET_NEXT (payload_flistp,PIN_FLD_SUB_BAL_IMPACTS, &elem_id1, 1,&cookie1, ebufp))
			!= (pin_flist_t *)NULL )
			{
				cookie2 = NULL;elem_id2 = 0;
				while ((tmp_flistp = PIN_FLIST_ELEM_GET_NEXT (args_flistp,PIN_FLD_SUB_BALANCES, &elem_id2, 1,&cookie2, ebufp))
				!= (pin_flist_t *)NULL )
				{
					if(PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_VALID_FROM, 0, ebufp) != NULL)
					valid_from = *(time_t *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_VALID_FROM, 0, ebufp);
					if(valid_from > 0)break;
				}

			}
		}		

//		if(valid_from)
		{

			/* Round to mid night*/
			PIN_FLIST_FLD_SET(sp_iflistp, PIN_FLD_END_T, (void *)&now, ebufp);

//			if((valid_from-now) <= 0)
			{
				tmp_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_CYCLE_START_T, &now, ebufp);
				PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_POID,pp_pdp,ebufp);
				PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, tmp_flistp, &write_out_flistp, ebufp);

				PIN_FLIST_DESTROY_EX(&write_out_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&tmp_flistp, NULL);
			}
		}
/*		else {
			PIN_FLIST_FLD_SET(sp_iflistp, PIN_FLD_END_T, (void *)&now, ebufp);
		} */
		/* Fix for duplicate charging . If the month end has passed , update cycle_start_t to the beginning of the current month.*/
		tmp_flistp = PIN_FLIST_ELEM_ADD(sp_iflistp, PIN_FLD_STATUSES, 1, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_OFFERING_OBJ, (void *)pp_pdp, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STATUS, &status, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STATUS_FLAGS, &status_flags, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
			"fm_mso_prov_update_product_status: Set Status Input Flist ", sp_iflistp);
		
		if((prov_flag && *prov_flag == 1) && strstr(action, "RENEW_SUBSCRIBER"))
                {
                        pp_read_iflist = PIN_FLIST_CREATE(ebufp);
			  rslt_oflistp  = PIN_FLIST_ELEM_GET(sp_iflistp, PIN_FLD_STATUSES,PIN_ELEMID_ANY,1,ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
				"fm_mso_prov_update_product_status: Set Status Input Flist ", rslt_oflistp);
			PIN_FLIST_FLD_COPY(rslt_oflistp, PIN_FLD_OFFERING_OBJ, pp_read_iflist, PIN_FLD_POID, ebufp);
                        PIN_FLIST_FLD_SET(pp_read_iflist, PIN_FLD_CYCLE_START_T, NULL, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "purchased_product read obj input Flist", pp_read_iflist);
                        PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, pp_read_iflist, &pp_read_oflist, ebufp);
			PIN_FLIST_DESTROY_EX(&pp_read_iflist, NULL);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "purchased_product read obj output Flist ", pp_read_oflist);

                        write_iflistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, (void *)service_obj, ebufp);
                        PIN_FLIST_FLD_COPY(pp_read_oflist, PIN_FLD_CYCLE_START_T, write_iflistp, PIN_FLD_LAST_STATUS_T, ebufp);
                        PIN_FLIST_FLD_COPY(pp_read_oflist, PIN_FLD_CYCLE_START_T, write_iflistp, PIN_FLD_EFFECTIVE_T, ebufp);
			grace_start = PIN_FLIST_FLD_GET(pp_read_oflist, PIN_FLD_CYCLE_START_T, 0, ebufp);
			PIN_FLIST_FLD_SET(sp_iflistp, PIN_FLD_START_T, grace_start, ebufp);
			PIN_FLIST_FLD_SET(sp_iflistp, PIN_FLD_END_T, grace_start, ebufp);
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "service update input flist before opcode call", write_iflistp);
                        PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_iflistp, &write_oflistp, ebufp);
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "service update output flist before opcode call", write_oflistp);
                        PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
                        PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
                        PIN_FLIST_DESTROY_EX(&pp_read_oflist, NULL);

                }


		

		//Call the Set status opcode
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_prov_update_product_status: Set Status input Flist", sp_iflistp);
		PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_SET_PRODUCT_STATUS, 0, sp_iflistp, &sp_oflistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)){
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_prov_update_product_status: Error", ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
			"fm_mso_prov_update_product_status: Set Status Output Flist ", sp_oflistp);
               
		if(sp_oflistp){
			if(program_name && strstr(program_name, "banner")){
				fm_mso_utils_read_any_object(ctxp, pp_pdp, &purch_flistp, ebufp);	
				if(purch_flistp){
					purch_end_t = PIN_FLIST_FLD_GET(purch_flistp, PIN_FLD_PURCHASE_END_T, 1, ebufp);
					prod_pdp = PIN_FLIST_FLD_GET(purch_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
					fm_mso_utils_read_any_object(ctxp, prod_pdp, &product_oflistp, ebufp);
					if(product_oflistp){
						priority = PIN_FLIST_FLD_GET(product_oflistp, PIN_FLD_PRIORITY, 1, ebufp);
						strp = pbo_decimal_to_str(priority, ebufp);
						prty = atoi(strp);
						mod_prty = prty%1000;
						usage_flistp = PIN_FLIST_ELEM_GET(product_oflistp, PIN_FLD_USAGE_MAP, PIN_ELEMID_ANY, 1, ebufp);
						if(usage_flistp){
							evt_type = PIN_FLIST_FLD_GET(usage_flistp, PIN_FLD_EVENT_TYPE, 1, ebufp);
						}
					}
					if(!(mod_prty >= BB_UNLIMITED_FUP_RANGE_START && mod_prty <= BB_UNLIMITED_FUP_RANGE_END && 
							(evt_type && strstr(evt_type, "mso_grant")))){
						cfee_flistp = PIN_FLIST_ELEM_GET(purch_flistp, PIN_FLD_CYCLE_FEES, PIN_ELEMID_ANY, 1, ebufp);
						if(cfee_flistp){
							wflds_iflistp = PIN_FLIST_CREATE(ebufp);
							PIN_FLIST_FLD_SET(wflds_iflistp, PIN_FLD_POID, pp_pdp, ebufp);
							PIN_FLIST_FLD_SET(cfee_flistp, PIN_FLD_CHARGED_TO_T, purch_end_t, ebufp);
							PIN_FLIST_FLD_SET(cfee_flistp, PIN_FLD_CYCLE_FEE_END_T, purch_end_t, ebufp);
							PIN_FLIST_FLD_COPY(purch_flistp, PIN_FLD_CYCLE_FEES, wflds_iflistp, PIN_FLD_CYCLE_FEES, ebufp);
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Update Banner offer products charge dates", wflds_iflistp);
							PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, wflds_iflistp, &wflds_oflistp, ebufp);
							PIN_FLIST_DESTROY_EX(&wflds_iflistp, NULL);
							if (PIN_ERRBUF_IS_ERR(ebufp)) {
								PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                					"fm_mso_prov_update_product_status: Error", ebufp);
                        					goto cleanup;
							}
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Update Banner offer products charge dates out", wflds_oflistp);
							PIN_FLIST_DESTROY_EX(&product_oflistp, NULL);
							PIN_FLIST_DESTROY_EX(&wflds_oflistp, NULL);
						}
					}
				}
				PIN_FLIST_DESTROY_EX(&purch_flistp, NULL);
			}
		}
		
		PIN_FLIST_DESTROY_EX(&sp_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&sp_oflistp, NULL);
	}
	
cleanup:
	PIN_FLIST_DESTROY_EX(&sp_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&sp_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&purch_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&product_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&robj_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&robj_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&cycle_fwd_oflistp, NULL);
	return;
}

/***********************************************************
* fm_mso_prov_failed_response: 
* 		Creates an object of type /mso_failed_prov_response
*		when the provisioning response fails to update 
* 		service/product status in BRM
***********************************************************/
static void
fm_mso_prov_failed_response(
	pcm_context_t		*ctxp, 
	pin_flist_t		*order_flistp,
	pin_flist_t		*in_flistp,
	int32			*task_failed, 
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*obj_in_flistp = NULL;
	pin_flist_t		*obj_out_flistp = NULL;
	poid_t			*acc_pdp = NULL;
	poid_t			*fr_pdp = NULL;
	int64			db = 0;
	int32			status = 0;
    pin_buf_t           *nota_buf     = NULL;
    char                *flistbuf = NULL;
    int                 flistlen=0;	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
			"fm_mso_prov_failed_response input flist", order_flistp);
	obj_in_flistp = PIN_FLIST_CREATE(ebufp);
	acc_pdp = PIN_FLIST_FLD_GET(order_flistp, PIN_FLD_POID, 0, ebufp);
	db = PIN_POID_GET_DB(acc_pdp);
	fr_pdp = PIN_POID_CREATE(db, "/mso_failed_prov_response", -1, ebufp);
	PIN_FLIST_FLD_PUT(obj_in_flistp, PIN_FLD_POID, fr_pdp, ebufp);
	PIN_FLIST_FLD_COPY(order_flistp, PIN_FLD_POID, obj_in_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(order_flistp, PIN_FLD_SERVICE_OBJ, obj_in_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(order_flistp, PIN_FLD_PROGRAM_NAME, obj_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_COPY(order_flistp, PIN_FLD_ORDER_ID, obj_in_flistp, PIN_FLD_ORDER_ID, ebufp);
	PIN_FLIST_FLD_COPY(order_flistp, PIN_FLD_ACTION, obj_in_flistp, PIN_FLD_ACTION, ebufp);

	//PIN_FLIST_FLD_SET(obj_in_flistp, PIN_FLD_STATUS, &status, ebufp);
	PIN_FLIST_FLD_SET(obj_in_flistp, MSO_FLD_TASK_FAILED, task_failed, ebufp);

	
	PIN_FLIST_TO_STR(in_flistp, &flistbuf, &flistlen, ebufp );
    nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ));
    if ( nota_buf == NULL ) 
    {
        pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
        PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate nemory for nota_buf", ebufp ); 
        return;
    }   

    nota_buf->flag   = 0;
    nota_buf->size   = flistlen ;
    nota_buf->offset = 0;
    nota_buf->data   = (caddr_t)flistbuf;
    nota_buf->xbuf_file = ( char *) NULL;

    PIN_FLIST_FLD_PUT( obj_in_flistp, PIN_FLD_BUFFER, ( void *) nota_buf, ebufp );
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"Create Failed Response object flist", obj_in_flistp);

	//Call the Create Obj opcode
	PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, obj_in_flistp, &obj_out_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_prov_failed_response: Error", ebufp);
		goto CLEANUP;
	}			
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"Leaving function fm_mso_prov_failed_response after create",obj_out_flistp);
	//Temporarily set error
	//pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
        CLEANUP:
                PIN_FLIST_DESTROY_EX(&obj_in_flistp, NULL);
                PIN_FLIST_DESTROY_EX(&obj_out_flistp, NULL);

        return;
}

/***********************************************************
* Function: fm_mso_prov_send_notifcation()
* Author: Harish Kulkarni
* Description: Sends the notification for the customers
***********************************************************/
static void
fm_mso_prov_send_notifcation(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flistp,
	int32				notification_flag,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*notif_iflistp = NULL;
	pin_flist_t		*notif_oflistp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
			"fm_mso_prov_send_notifcation input flist", in_flistp);

	notif_iflistp = PIN_FLIST_CREATE(ebufp);
	if (notification_flag && notification_flag == ACTIVATE_SERVICE)
	{
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, notif_iflistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SERVICE_OBJ, notif_iflistp, PIN_FLD_SERVICE_OBJ, ebufp);
		PIN_FLIST_FLD_SET(notif_iflistp, PIN_FLD_FLAGS, &notification_flag, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, notif_iflistp, PIN_FLD_PROGRAM_NAME, ebufp);
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_mso_prov_send_notifcation MSO_OP_NTF_CREATE_NOTIFICATION input flist", notif_iflistp);

	//Call the MSO_OP_NTF_CREATE_NOTIFICATION opcode
	PCM_OP(ctxp, MSO_OP_NTF_CREATE_NOTIFICATION, 0, notif_iflistp, &notif_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_prov_send_notifcation: Error", ebufp);
		goto CLEANUP;
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_mso_prov_send_notifcation MSO_OP_NTF_CREATE_NOTIFICATION output flist", notif_oflistp);

	CLEANUP:
		PIN_FLIST_DESTROY_EX(&notif_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&notif_oflistp, NULL);

	return;
}

static void
fm_mso_update_provisioning_and_order_status(
	pcm_context_t		*ctxp,
	poid_t				*order_pdp,
	int32				ord_status,
	pin_flist_t			*payload_flistp,
	pin_flist_t			*in_flistp,
	pin_flist_t			*tmp_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*w_i_flistp = NULL;
	pin_flist_t		*w_o_flistp = NULL;
	pin_flist_t		*services = NULL;
	pin_flist_t		*inherited = NULL;
	pin_flist_t		*telco_feature = NULL;
	pin_flist_t		*bb_info = NULL;
	char			*action = NULL;
	poid_t			*service_obj = NULL;
	poid_t			*acct_pdp = NULL;
	char			*neid = NULL;
	int32			srv_status = 1;
	int			*act_flag = NULL;
	char			*mac_id = NULL;
    	int32		    	tmp_svc_status = 0;
	int32			status_act_flag = 4;

	int			*svc_statusp = NULL;
	void			*vp = NULL;
	poid_t			*mso_bb_bp_obj = NULL;	
	poid_t			*mso_bb_obj = NULL;
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"fm_mso_update_provisioning_and_order_status: Input Flist", in_flistp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"fm_mso_update_provisioning_and_order_status: payload_flistp", payload_flistp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"fm_mso_update_provisioning_and_order_status: tmp_flistp", tmp_flistp);

	action = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_ACTION, 0, ebufp);
	service_obj = (poid_t *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
	acct_pdp = (poid_t *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);

	//Update the status in the order
	w_i_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(w_i_flistp, PIN_FLD_POID, (void *)order_pdp, ebufp);
	PIN_FLIST_FLD_SET(w_i_flistp, PIN_FLD_STATUS, (void *)&ord_status, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_update_provisioning_and_order_status: Order Write Fields Input Flist", w_i_flistp);
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 32, w_i_flistp, &w_o_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_update_provisioning_and_order_status: Order Update Error", ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_update_provisioning_and_order_status: Order Write Fields Output Flist", w_o_flistp);
	PIN_FLIST_DESTROY_EX(&w_i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&w_o_flistp, NULL);

	w_i_flistp = NULL;
	w_o_flistp = NULL;

	//Update the status /service/telco/broadband
	if(strstr(action, "HOLD_SUBSCRIBER"))
		srv_status = MSO_PROV_STATE_HOLD;
	else if(strstr(action, "SUSPEND_SUBSCRIBER"))
		srv_status = MSO_PROV_STATE_SUSPENDED;
	/* Spelling "SUSBCRIBER" below is wrong but that is how it is 
		in interface doc and prov_create_bb_action opcode */
	else if(strstr(action, "TERMINATE_SUSBCRIBER"))
		srv_status = MSO_PROV_STATE_TERMINATED;
	else if(strstr(action, "DEACTIVATE_SUBSCRIBER"))
		srv_status = MSO_PROV_STATE_DEACTIVE;

	/***** Pavan Bellala 08-09-2015 ******************************
	In case of Change CMTS of HOLD/UNHOLD service,update 
	correct action status
	***************************************************************/
	else if((strstr(action, "CHANGE_CMTS_NONTAL_DOCSIS"))||(strstr(action,"CHANGE_CMTS_TAL_DOCSIS")))
	{
		svc_statusp = PIN_FLIST_FLD_GET(payload_flistp,PIN_FLD_STATUS_FLAGS,1,ebufp);
		if(svc_statusp && (*svc_statusp != PIN_STATUS_ACTIVE))
		{
			vp = PIN_FLIST_FLD_GET(payload_flistp,PIN_FLD_STATUS,1,ebufp);
			if(vp) srv_status = *(int32*)vp;
		} else {
			srv_status = MSO_PROV_STATE_ACTIVE;
		}

	}
	else if(strstr(action, "UPDATE_SUBSCRIBER-FUP"))
	{
		fm_mso_cust_get_bp_obj( ctxp, acct_pdp, service_obj, MSO_PROV_SUSPEND, &mso_bb_bp_obj, &mso_bb_obj, ebufp);
               if(PIN_POID_IS_NULL(mso_bb_bp_obj)){
                  srv_status = MSO_PROV_STATE_ACTIVE;
               }
               else
               {
                 srv_status = MSO_PROV_STATE_SUSPENDED;
               }
	}		
	else
		srv_status = MSO_PROV_STATE_ACTIVE;
	if(strstr(action, "ACTIVATE_SUBSCRIBER")){
		neid = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_ID, 1, ebufp);
	}
	act_flag = (int *)PIN_FLIST_FLD_GET(payload_flistp, PIN_FLD_FLAGS, 0, ebufp);
	//Update the service telco features status
	w_i_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(w_i_flistp, PIN_FLD_POID, (void *)acct_pdp, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_PROGRAM_NAME,w_i_flistp, PIN_FLD_PROGRAM_NAME,ebufp);
	services = PIN_FLIST_ELEM_ADD(w_i_flistp, PIN_FLD_SERVICES, 0, ebufp);
	PIN_FLIST_FLD_SET(services, PIN_FLD_POID, service_obj, ebufp);
	if(strstr(action, "RENEW_SUBSCRIBER") ||
		//strstr(action, "CHANGE_SERVICE") ||
		strstr(action, "UNHOLD_SERVICE"))
	{
		tmp_svc_status = PIN_STATUS_ACTIVE;
		PIN_FLIST_FLD_SET(services, PIN_FLD_STATUS, &tmp_svc_status, ebufp);
		PIN_FLIST_FLD_SET(services, PIN_FLD_STATUS_FLAGS, &status_act_flag, ebufp);
	}
	inherited = PIN_FLIST_SUBSTR_ADD(services, PIN_FLD_INHERITED_INFO, ebufp);
	telco_feature = PIN_FLIST_ELEM_ADD(inherited, PIN_FLD_TELCO_FEATURES, 0, ebufp);
	PIN_FLIST_FLD_SET(telco_feature, PIN_FLD_STATUS, &srv_status, ebufp);
	PIN_FLIST_FLD_SET(telco_feature, PIN_FLD_NAME, "PROVISIONING_STATUS", ebufp);
	if(neid){
		bb_info = PIN_FLIST_SUBSTR_ADD(inherited, MSO_FLD_BB_INFO, ebufp);
		PIN_FLIST_FLD_SET(bb_info, MSO_FLD_ID, (void *)neid, ebufp);
	}
	if(*act_flag == ETH_BB_GET_USER_DETAIL){
		mac_id = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_MAC_ID, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_update_provisioning_and_order_status error: Required field missing", ebufp);
			goto cleanup;
		}
		if(bb_info){
			PIN_FLIST_FLD_SET(bb_info, MSO_FLD_MAC_ID, (void *)mac_id, ebufp);
		}
		else{
			bb_info = PIN_FLIST_SUBSTR_ADD(inherited, MSO_FLD_BB_INFO, ebufp);
			PIN_FLIST_FLD_SET(bb_info, MSO_FLD_MAC_ID, (void *)mac_id, ebufp);
		}
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_update_provisioning_and_order_status error: Error", ebufp);
			goto cleanup;
		}
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_update_provisioning_and_order_status: Update Service Input Flist", w_i_flistp);
	PCM_OP(ctxp, PCM_OP_CUST_UPDATE_SERVICES, 0, w_i_flistp, &w_o_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_update_provisioning_and_order_status: PCM_OP_CUST_UPDATE_SERVICES Error", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_update_provisioning_and_order_status: Update Service Output Flist", w_o_flistp);

	cleanup:
	PIN_FLIST_DESTROY_EX(&w_i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&w_o_flistp, NULL);

	return;
}

static void
fm_mso_update_purchased_plan_status(
	pcm_context_t		*ctxp,
	pin_flist_t		*payload_flistp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*tmp_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t			*w_i_flistp = NULL;
	pin_flist_t			*w_o_flistp = NULL;
	pin_flist_t			*products = NULL;
	pin_flist_t			*cobj_iflistp = NULL;
	pin_flist_t			*cobj_oflistp = NULL;
	char				*action = NULL;
	poid_t				*service_obj = NULL;
	poid_t				*acct_pdp = NULL;
	poid_t				*offering_obj = NULL;
	poid_t				*offering_obj2 = NULL;
	poid_t				*dummy = NULL;
	int32				srv_status = 1;
	int					*act_flag = NULL;
	int64				db = 0;
	pin_cookie_t                cookie1 = NULL;
	int32                       rec_id1 = 0;
	char				*program_name = NULL;

        int                     *svc_statusp = NULL;
        void                    *vp = NULL;
	poid_t			*mso_bb_bp_obj = NULL;
	poid_t			*mso_bb_obj = NULL;
	int32			susp_flag = -1;
	int32			*mso_status = NULL;
	int32			*fup_status = NULL;
	int32			*indicator = NULL;
        time_t                  *pur_prod_crea = NULL;
        pin_flist_t             *ret_flistp = NULL;
	pin_flist_t             *prod_array = NULL;
        poid_t                  *pur_pdp = NULL;
	time_t			*pur_prod_start = NULL;
	char                    tmp[1000];
	int32			update = 0;
	time_t			*chrg_to = NULL;
	time_t			*pur_end = NULL;
	char			*prov_tag = NULL;
	pin_flist_t		*product_flistp = NULL;
	pin_flist_t		*ret_flistpp = NULL;
	pin_flist_t		*serv_bb = NULL;
	pin_cookie_t		cookie = NULL;
	int32           	elem_id = 0;
	pin_flist_t		*rett_flistpp = NULL;
	int32                   *prov_flag = NULL;
 
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"fm_mso_update_purchased_plan_status: Input Flist", in_flistp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"fm_mso_update_purchased_plan_status: payload_flistp", payload_flistp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"fm_mso_update_purchased_plan_status: tmp_flistp", tmp_flistp);

	action = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_ACTION, 0, ebufp);
	service_obj = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
	acct_pdp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	offering_obj = PIN_FLIST_FLD_GET(payload_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, 0, ebufp);
	program_name = PIN_FLIST_FLD_GET(payload_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	fup_status = PIN_FLIST_FLD_GET(payload_flistp, MSO_FLD_FUP_STATUS, 1, ebufp);
	//indicator = PIN_FLIST_FLD_GET(payload_flistp, PIN_FLD_INDICATOR, 1, ebufp);
	if(strstr(action, "UPDATE_SUBSCRIBER-FUP"))
	{
		fm_mso_get_poid_details(ctxp,service_obj,&ret_flistpp,ebufp);
        	serv_bb = PIN_FLIST_SUBSTR_GET(ret_flistpp, MSO_FLD_BB_INFO, 0, ebufp);
        	if(serv_bb != NULL)
        	{
                	indicator = PIN_FLIST_FLD_TAKE(serv_bb, PIN_FLD_INDICATOR, 1, ebufp);
			PIN_FLIST_DESTROY_EX(&ret_flistpp, NULL);
        	}
	}
	if(strstr(action, "HOLD_SUBSCRIBER"))
		srv_status = MSO_PROV_STATE_HOLD;
	else if(strstr(action, "SUSPEND_SUBSCRIBER"))
		srv_status = MSO_PROV_STATE_SUSPENDED;
	/* Spelling "SUSBCRIBER" below is wrong but that is how it is
		in interface doc and prov_create_bb_action opcode */
	else if(strstr(action, "TERMINATE_SUSBCRIBER"))
		srv_status = MSO_PROV_STATE_TERMINATED;
	else if(strstr(action, "DEACTIVATE_SUBSCRIBER"))
		srv_status = MSO_PROV_STATE_DEACTIVE;
        /***** Pavan Bellala 08-09-2015 ******************************
        In case of Change CMTS of HOLD/UNHOLD service,update
        correct action status
        ***************************************************************/
        else if((strstr(action, "CHANGE_CMTS_NONTAL_DOCSIS"))||(strstr(action,"CHANGE_CMTS_TAL_DOCSIS")))
        {
                svc_statusp = PIN_FLIST_FLD_GET(payload_flistp,PIN_FLD_STATUS_FLAGS,1,ebufp);
                if(svc_statusp && (*svc_statusp != PIN_STATUS_ACTIVE))
                {
                        vp = PIN_FLIST_FLD_GET(payload_flistp,PIN_FLD_STATUS,1,ebufp);
                        if(vp) srv_status = *(int32*)vp;
                } else {
                        srv_status = MSO_PROV_STATE_ACTIVE;
                }

        }
	/*added condition for to reset the quota for fup even if it is suspended*/
	else if(strstr(action, "UPDATE_SUBSCRIBER-FUP") || strstr(action, "ADD_DATA_DOCSIS"))
	{
	       fm_mso_cust_get_bp_obj( ctxp, acct_pdp, service_obj, MSO_PROV_SUSPEND, &mso_bb_bp_obj, &mso_bb_obj, ebufp);
	       if(PIN_POID_IS_NULL(mso_bb_bp_obj)){
		  srv_status = MSO_PROV_STATE_ACTIVE;
		  susp_flag = 0;
	       }
	       else
	       {
		 srv_status = MSO_PROV_STATE_SUSPENDED;
		 susp_flag = 1;
	       }
	}
	else
		srv_status = MSO_PROV_STATE_ACTIVE;

	act_flag = (int *)PIN_FLIST_FLD_GET(payload_flistp, PIN_FLD_FLAGS, 0, ebufp);

	//Update the status in purchased plan
	w_i_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(w_i_flistp, PIN_FLD_POID, (void *)offering_obj, ebufp);
	PIN_FLIST_FLD_SET(w_i_flistp, PIN_FLD_STATUS, (void *)&srv_status, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_update_purchased_plan_status: Write Fields Input Flist", w_i_flistp);
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 32, w_i_flistp, &w_o_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_update_purchased_plan_status error: Error", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_update_purchased_plan_status: Write Fields Output Flist", w_o_flistp);
	PIN_FLIST_DESTROY_EX(&w_i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&w_o_flistp, NULL);
	
	//For Add MB-GB and FUP Topup call this function  to update the MSO Pur plan
	//status since the MSO PP poid passed in input is not of the base plan
	if(susp_flag == 0)
	{ 
	if(strstr(action, "UPDATE_SUBSCRIBER-FUP") ||
		strstr(action, "ADD_DATA_DOCSIS")	)
	{
		fm_mso_update_base_pp_obj(ctxp, payload_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_update_purchased_plan_status Update Product status: Error", ebufp);
			goto cleanup;
		}
	}
	}
	if(strstr(action, "HOLD_SUBSCRIBER"))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Calling fm_mso_update_ncr_validity");
		fm_mso_update_ncr_validity(ctxp, payload_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_update_ncr_validity : Error", ebufp);
			goto cleanup;
		}
	}
	
	
	//Update the status in all purchased plan objects passed in the input
	// Not Required
	/*
	while ( (products = PIN_FLIST_ELEM_GET_NEXT (in_flistp,
		PIN_FLD_PRODUCTS, &elem_id, 1,&cookie2, ebufp))
		!= (pin_flist_t *)NULL )
	{
		offering_obj2 = (poid_t*)PIN_FLIST_FLD_GET(products,
			MSO_FLD_PURCHASED_PLAN_OBJ, 0, ebufp);
		w_i_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(w_i_flistp, PIN_FLD_POID, (void *)offering_obj2, ebufp);
		PIN_FLIST_FLD_SET(w_i_flistp, PIN_FLD_STATUS, (void *)&srv_status, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_mso_update_purchased_plan_status: Update purchased plan Input Flist", w_i_flistp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 32, w_i_flistp, &w_o_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_update_purchased_plan_status: Update purchased plan error", ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_mso_update_purchased_plan_status: Update purchased plan output Flist", w_o_flistp);
		PIN_FLIST_DESTROY_EX(&w_i_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&w_o_flistp, NULL);
	}
	*/

	//For some scenarios update the status of the products
	/*RAVI added changes here*/
	if(strstr(action, "ACTIVATE_SUBSCRIBER")||
		strstr(action, "CHANGE_SERVICE") ||
		strstr(action, "RENEW_SUBSCRIBER") ||
		strstr(action, "UNHOLD_SERVICE") ||
		strstr(action, "UPDATE_SUBSCRIBER-FUP") ||
		strstr(action, "TOPUP_WO_PLAN") ||
		strstr(action, "ADD_DATA_DOCSIS") ||
		strstr(action, "AUTORENEW_SUBSCRIBER_DOCSIS") ||
		strstr(action, "EXTEND_VALIDITY_DOCSIS") 
	//	||
	//	strstr(action, "CHANGE_CMTS_NONTAL_DOCSIS")
	)

	{
		// Update the Product Status
		
		fm_mso_prov_update_product_status(ctxp, acct_pdp, service_obj, offering_obj, program_name, action, payload_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_update_purchased_plan_status Update Product status: Error", ebufp);
			goto cleanup;
		}
		rec_id1 = 0;
		cookie1 = NULL;
		while((product_flistp = PIN_FLIST_ELEM_GET_NEXT(payload_flistp, 
			PIN_FLD_PRODUCTS, &rec_id1, 1, &cookie1, ebufp)) != (pin_flist_t *)NULL)
		{
			offering_obj = PIN_FLIST_FLD_GET(product_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, 0, ebufp);
			fm_mso_prov_update_product_status(ctxp, acct_pdp, service_obj, offering_obj, program_name, action, payload_flistp, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"fm_mso_update_purchased_plan_status Update Product status: Error", ebufp);
				goto cleanup;
			}
		}
		// Call the function to apply deferred purchase fees
		fm_mso_prov_apply_deferred_purchase_fees(ctxp, acct_pdp, service_obj, program_name, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_update_purchased_plan_status Apply deferred purchase fees: Error", ebufp);
			goto cleanup;
		}
		/***update the purchase_start_t to created_t during activation and unhold and renew and topup and change plan scenarios**********/
		rec_id1 = 0;
                cookie1 = NULL;
		if(action && (strstr(action,"UNHOLD_SERVICE") || strstr(action, "ACTIVATE_SUBSCRIBER") || strstr(action, "CHANGE_SERVICES")|| 
			strstr(action, "RENEW_SUBSCRIBER") || strstr(action,"TOPUP_WO_PLAN_CHANGE_SUBSCRIBER")))
        	{
                	fm_mso_get_purchased_prod_active(ctxp, payload_flistp, &ret_flistp, ebufp);
                	while((prod_array = PIN_FLIST_ELEM_GET_NEXT(ret_flistp, PIN_FLD_RESULTS,
                        	&rec_id1, 1, &cookie1, ebufp)) != (pin_flist_t *)NULL)
                	{
                        	pur_pdp = PIN_FLIST_FLD_GET(prod_array, PIN_FLD_POID, 1, ebufp);
                        	pur_prod_crea = PIN_FLIST_FLD_GET(prod_array, PIN_FLD_CREATED_T, 1, ebufp);
				pur_prod_start = PIN_FLIST_FLD_GET(prod_array, PIN_FLD_PURCHASE_START_T, 1, ebufp);
				*pur_prod_crea = fm_utils_time_round_to_midnight(*pur_prod_crea);
				sprintf(tmp, "purchased_created_t %d", *pur_prod_crea);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, tmp);
				if(pur_prod_crea && pur_prod_start && *pur_prod_crea == *pur_prod_start)
				   continue;
				prov_flag = (int32 *)PIN_FLIST_FLD_GET(payload_flistp, MSO_FLD_PROVISIONABLE_FLAG, 1, ebufp);
				if(prov_flag && *prov_flag != 1)
                		{

                        	w_i_flistp = PIN_FLIST_CREATE(ebufp);
                        	PIN_FLIST_FLD_SET(w_i_flistp, PIN_FLD_POID, pur_pdp, ebufp);
                        	PIN_FLIST_FLD_SET(w_i_flistp, PIN_FLD_PURCHASE_START_T, pur_prod_crea,ebufp);
                        	PIN_FLIST_FLD_SET(w_i_flistp, PIN_FLD_USAGE_START_T, pur_prod_crea, ebufp);
                        	PIN_FLIST_FLD_SET(w_i_flistp, PIN_FLD_CYCLE_START_T, pur_prod_crea, ebufp);
                        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_provisioning_and_order_status purchased_product WriteFlds input flist", w_i_flistp);
                        	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, w_i_flistp, &w_o_flistp, ebufp);
                        	PIN_FLIST_DESTROY_EX(&w_i_flistp, NULL);
                        	if (PIN_ERRBUF_IS_ERR(ebufp))
                        	{
                                	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in write flds in updating Product information", ebufp);
                                	PIN_FLIST_DESTROY_EX(&w_o_flistp, NULL);
                                	return;
                        	}
                        	PIN_FLIST_DESTROY_EX(&w_o_flistp, NULL);
				}
                	}
        	}
		/* Call function to extend the validity of non currency resource created 
			above by update_product_status */
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Action value:");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,action);
		if(strstr(action, "UPDATE_SUBSCRIBER-FUP") && !fup_status && *indicator == 2)
		{
			product_flistp = NULL;
			fm_mso_get_poid_details(ctxp, offering_obj, &ret_flistpp, ebufp);
			while((product_flistp = PIN_FLIST_ELEM_GET_NEXT(ret_flistpp, PIN_FLD_PRODUCTS,
                		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
        		{
                		prov_tag = PIN_FLIST_FLD_GET(product_flistp, PIN_FLD_PROVISIONING_TAG, 1, ebufp);
                		if(prov_tag && strlen(prov_tag) > 0 )
                		{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"checkk");
                        		pur_pdp = PIN_FLIST_FLD_GET(product_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, 1, ebufp);
					fm_mso_get_poid_details(ctxp, pur_pdp, &rett_flistpp, ebufp);
					pur_end = PIN_FLIST_FLD_GET(rett_flistpp, PIN_FLD_PURCHASE_END_T, 0, ebufp);	
					chrg_to = PIN_FLIST_FLD_GET(payload_flistp, PIN_FLD_CHARGED_TO_T, 0, ebufp);
					if(chrg_to && pur_end && (*chrg_to >= *pur_end))
					{
						update = 1;
					}
					PIN_FLIST_DESTROY_EX(&rett_flistpp, ebufp);
                                        break;	
                		}
        		}
			PIN_FLIST_DESTROY_EX(&ret_flistpp, ebufp);
		}
		if(strstr(action, "TOPUP_WO_PLAN")||
			//(strstr(action, "UPDATE_SUBSCRIBER-FUP") && fup_status != NULL && *indicator == 2) ||
			(strstr(action, "UPDATE_SUBSCRIBER-FUP") && !fup_status && update == 1) ||
			strstr(action, "UNHOLD_SERVICE"))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Calling fm_mso_update_ncr_validity");
			fm_mso_update_ncr_validity(ctxp, payload_flistp, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"fm_mso_update_ncr_validity : Error", ebufp);
				goto cleanup;
			}
		}
	}
	if(*act_flag == ETH_BB_GET_LAST_SESSION_DETAIL)
	{
		//Create the object for last session details
		cobj_iflistp = PIN_FLIST_CREATE(ebufp);
		db = PIN_POID_GET_DB(acct_pdp);
		dummy = PIN_POID_CREATE(db, "/mso_last_session_detail", -1, ebufp);
		PIN_FLIST_FLD_PUT(cobj_iflistp, PIN_FLD_POID, dummy, ebufp);
		PIN_FLIST_FLD_SET(cobj_iflistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_LOGIN, cobj_iflistp, PIN_FLD_LOGIN, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_STARTDATE, cobj_iflistp, MSO_FLD_STARTDATE, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_ID, cobj_iflistp, MSO_FLD_ID, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_EXTERNAL_REF_NO, cobj_iflistp, MSO_FLD_EXTERNAL_REF_NO, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_MAC_ID, cobj_iflistp, MSO_FLD_MAC_ID, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_GROUP, cobj_iflistp, MSO_FLD_GROUP, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SOURCE, cobj_iflistp, PIN_FLD_SOURCE, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_update_purchased_plan_status Last Session Create Field Missing: Error", ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
			"fm_mso_update_purchased_plan_status: Last Session Create Input", cobj_iflistp);
		PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, cobj_iflistp, &cobj_oflistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_update_purchased_plan_status error: Error", ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
			"fm_mso_update_purchased_plan_status: Last Session Create Output", cobj_oflistp);
	}

	cleanup:
	PIN_FLIST_DESTROY_EX(&w_i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&w_o_flistp, NULL);

	PIN_FLIST_DESTROY_EX(&cobj_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&cobj_oflistp, NULL);

	return;
}

/************************************************
* fm_mso_prov_apply_deferred_purchase_fees()
* Author: Harish  Kulkarni
* For applying the deferred purchase Fees
***********************************************/

static void
fm_mso_prov_apply_deferred_purchase_fees(
	pcm_context_t           *ctxp,
	poid_t			*acct_pdp,
	poid_t			*serv_pdp,
	char			*program_name,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t		*update_srvc_status_input = NULL;
	pin_flist_t		*update_srvc_status_output = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
		"Inside the function fm_mso_prov_apply_deferred_purchase_fees");

	/******************************************************************
	* apply any deferred purchase fee
	*******************************************************************/
	update_srvc_status_input = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(update_srvc_status_input, PIN_FLD_POID, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(update_srvc_status_input, PIN_FLD_SERVICE_OBJ, serv_pdp, ebufp);
	if (!program_name)
	{
		PIN_FLIST_FLD_SET(update_srvc_status_input, PIN_FLD_PROGRAM_NAME, "oap|csradmin", ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(update_srvc_status_input, PIN_FLD_PROGRAM_NAME, program_name, ebufp);
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_prov_apply_deferred_purchase_fees: Apply deferred purchase fee input flist", 
		update_srvc_status_input);
	PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_PURCHASE_FEES, 0, update_srvc_status_input, &update_srvc_status_output, ebufp);

	PIN_FLIST_DESTROY_EX(&update_srvc_status_input, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_FLIST_DESTROY_EX(&update_srvc_status_output, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_prov_apply_deferred_purchase_fees: Apply deferred purchase fee output flist", 
		update_srvc_status_output);

	PIN_FLIST_DESTROY_EX(&update_srvc_status_output, NULL);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_prov_apply_deferred_purchase_fees: Successful");

	return;
}

/***********************************************************
* fm_mso_upd_prov_failed_response:
*		   Updates an object of type /mso_failed_prov_response
*		   when the provisioning response fails to update
*		   service/product status in BRM
***********************************************************/
static void
fm_mso_upd_prov_failed_response(
	pcm_context_t           *ctxp,
	pin_flist_t             *order_flistp,
	pin_flist_t             *in_flistp,
	int32                   *task_failed,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t             *obj_in_flistp = NULL;
	pin_flist_t             *obj_out_flistp = NULL;
	pin_flist_t             *srch_oflistp = NULL;
	pin_flist_t             *rslt_oflistp = NULL;
	poid_t                  *fr_pdp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_upd_prov_failed_response input flist", order_flistp);

	// Call the function to get the fetch the /mso_failed_prov_response object
	// for a given order_id.
	fm_mso_search_failed_prov_response(ctxp, in_flistp, &srch_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_upd_prov_failed_response: fm_mso_search_failed_prov_response Error", ebufp);
		goto CLEANUP;
	}
	rslt_oflistp = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, 0, PIN_ELEMID_ANY, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_upd_prov_failed_response: fm_mso_search_failed_prov_response - No Results Error", ebufp);
		goto CLEANUP;
	}
	fr_pdp = PIN_FLIST_FLD_GET(rslt_oflistp, PIN_FLD_POID, 0, ebufp);

	obj_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(obj_in_flistp, PIN_FLD_POID, fr_pdp, ebufp);
	PIN_FLIST_FLD_SET(obj_in_flistp, MSO_FLD_TASK_FAILED, task_failed, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"Update Failed Response object flist", obj_in_flistp);

	//Call the PCM_OP_WRITE_FLDS opcode
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, obj_in_flistp, &obj_out_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_upd_prov_failed_response: Error", ebufp);
		goto CLEANUP;
	}

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
		"Returning from function fm_mso_upd_prov_failed_response");

	CLEANUP:
		PIN_FLIST_DESTROY_EX(&obj_in_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&obj_out_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);		

	return;
}


static void
fm_mso_search_failed_prov_response(
	pcm_context_t           *ctxp,
	pin_flist_t             *in_flistp,
	pin_flist_t             **ret_flistpp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t             *srch_iflistp = NULL;
	pin_flist_t             *srch_oflistp = NULL;
	pin_flist_t             *args_flist = NULL;
	int32                   srch_flag = 256;
	char                    *s_template = "select X from /mso_failed_prov_response where F1 = V1 ";
	poid_t                  *pdp = NULL;
	int32                   failed_level = 0;
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_search_failed_prov_response", ebufp);
		return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_failed_prov_response input flist", in_flistp);

	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	pdp = PIN_POID_CREATE(1, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(srch_iflistp, PIN_FLD_POID, pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, s_template, ebufp);

	args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ORDER_ID, args_flist, PIN_FLD_ORDER_ID, ebufp);

	args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(args_flist, PIN_FLD_POID, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_failed_prov_response search input list", srch_iflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_search_failed_prov_response - Error in calling SEARCH", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_failed_prov_response search output flist", srch_oflistp);

	*ret_flistpp = PIN_FLIST_COPY(srch_oflistp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_failed_prov_response output flist", srch_oflistp);
	CLEANUP:
		PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	return;
}

static void
fm_mso_update_ncr_validity(
	pcm_context_t		*ctxp,
	pin_flist_t		*payload_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t			*r_i_flistp = NULL;
	pin_flist_t			*r_o_flistp = NULL;
	pin_flist_t			*set_in_flistp = NULL;
	pin_flist_t			*set_res_flistp = NULL;
	pin_flist_t			*args_flistp = NULL;
	pin_flist_t			*bal_flistp = NULL;
	pin_flist_t			*sub_bal_flistp = NULL;
	pin_flist_t			*bb_info_flistp = NULL;
	pin_flist_t			*cyc_fee_flistp = NULL;
	pin_flist_t			*r_out_flistp = NULL;
	pin_flist_t			*r_in_flistp = NULL;
	poid_t				*bal_grp_obj = NULL;
	poid_t				*service_obj = NULL;
	poid_t				*acct_pdp = NULL;
	poid_t				*offering_obj = NULL;
	poid_t				*pur_prod_obj = NULL;
	poid_t				*pp_obj = NULL;
	poid_t				*plan_obj = NULL;
	int32				srv_status = 1;
	int32				*act_flag = NULL;
	int32				bal_flags = 0;
	int32				counter = 0;
	int32				cntr = 0;
	int64				db = 0;
	int32				bal_elem_id = 0;
	int32				sub_bal_id = 0;
	int32				action_topup=0;
	int32				action_hold=0;
	int32				action_unhold=0;
	int32				bill_when=0;
	int32				*status=NULL;

	pin_cookie_t		cookie = NULL;
	pin_cookie_t		sub_cookie = NULL;
	time_t				current_t;
	time_t				*pur_end_t = NULL;
	time_t				*chg_to = NULL;
	char				*prog_name = NULL;
	char				*action = NULL;
	struct tm               *timeeff;
	/***** Pavan Bellala - Defect Fix - While doing Topup customer credict values Not refelected *****/
	time_t 			now_t =  fm_utils_time_round_to_midnight(pin_virtual_time(NULL));
	time_t			*valid_from_tp = NULL;
	time_t			*valid_to_tp = NULL;
	time_t			fup_valid_to = 0;
	time_t			valid_to_t = 0;
	int32			*fup_status = NULL;
	int32			*indicator = NULL;
	time_t			end_date = 0;
	time_t			start = 0;
	poid_t			*mso_bb_bp_obj = NULL;
	poid_t			*mso_bb_obj = NULL;
	poid_t			*product_pdp = NULL;
	pin_flist_t		*ret_flistp = NULL;
	int32			fup_top = 0;
	int32			inc = 1;
	int32			action_fup = 0;
	time_t			pvt = pin_virtual_time(NULL);
	/******Pavan Bellala - Defect Fix - While doing Topup customer credict values Not refelected *****/



	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"fm_mso_update_ncr_validity: payload_flistp", payload_flistp);


	action = PIN_FLIST_FLD_GET(payload_flistp, PIN_FLD_ACTION, 0, ebufp);
	service_obj = PIN_FLIST_FLD_GET(payload_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
	acct_pdp = PIN_FLIST_FLD_GET(payload_flistp, PIN_FLD_POID, 0, ebufp);
	offering_obj = PIN_FLIST_FLD_GET(payload_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, 0, ebufp);
	fup_status = PIN_FLIST_FLD_GET(payload_flistp, MSO_FLD_FUP_STATUS, 1, ebufp);
	if(strstr(action, "TOPUP_WO_PLAN"))
		action_topup=1;
	if(strstr(action, "HOLD_SUBSCRIBER"))
		action_hold=1;
	if(strstr(action, "UNHOLD_SERVICE"))
		action_unhold=1;
	if(strstr(action, "UPDATE_SUBSCRIBER-FUP"))	
		action_fup=1;
	
	//Read the balance group of service
	r_i_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(r_i_flistp, PIN_FLD_POID, (void *)service_obj, ebufp);
	PIN_FLIST_FLD_SET(r_i_flistp, PIN_FLD_BAL_GRP_OBJ, (void *)NULL, ebufp);
	bb_info_flistp = PIN_FLIST_ELEM_ADD(r_i_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	PIN_FLIST_FLD_SET(bb_info_flistp, PIN_FLD_BILL_WHEN, NULL, ebufp);
	PIN_FLIST_FLD_SET(bb_info_flistp, PIN_FLD_INDICATOR, NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"Read service Input Flist", r_i_flistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 32, r_i_flistp, &r_o_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"Read service error: Error", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"Read service Output Flist", r_o_flistp);
	PIN_FLIST_DESTROY_EX(&r_i_flistp, NULL);
	bb_info_flistp = PIN_FLIST_ELEM_GET(r_o_flistp, MSO_FLD_BB_INFO, 0 ,0, ebufp);
	bill_when = *(int32 *) PIN_FLIST_FLD_GET(bb_info_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);
	bal_grp_obj = PIN_FLIST_FLD_TAKE(r_o_flistp, PIN_FLD_BAL_GRP_OBJ, 0, ebufp);
	indicator = PIN_FLIST_FLD_TAKE(bb_info_flistp, PIN_FLD_INDICATOR, 1, ebufp);
	PIN_FLIST_DESTROY_EX(&r_i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_o_flistp, NULL);

	/***Bal_grp change for the fup topup product during hold only for prepaid****/
	if(action_hold && *indicator == 2)
	{
		fm_mso_modify_bal_grp(ctxp, bal_grp_obj, ebufp);
	}
	/* Read the /mso_purchased_plan object */
	r_i_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(r_i_flistp, PIN_FLD_POID, (void *)offering_obj, ebufp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, r_i_flistp, &r_o_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_update_ncr_validity: Error", ebufp);
		PIN_FLIST_DESTROY_EX(&r_i_flistp, NULL);
		return;
	}
	args_flistp = PIN_FLIST_ELEM_GET(r_o_flistp, PIN_FLD_PRODUCTS,PIN_ELEMID_ANY,0, ebufp);
	pur_prod_obj = PIN_FLIST_FLD_TAKE(args_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, 0, ebufp);
	plan_obj = PIN_FLIST_FLD_TAKE(r_o_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG,"Purchased product poid: ",pur_prod_obj);
	PIN_FLIST_DESTROY_EX(&r_i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_o_flistp, NULL);
	
	/* Read the /purchased_product object to get the end date*/
	r_i_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(r_i_flistp, PIN_FLD_POID, (void *)pur_prod_obj, ebufp);
	
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, r_i_flistp, &r_o_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_update_ncr_validity: Error", ebufp);
		PIN_FLIST_DESTROY_EX(&r_i_flistp, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read Purchased product output flist", r_o_flistp);
	pur_end_t = PIN_FLIST_FLD_TAKE(r_o_flistp, PIN_FLD_PURCHASE_END_T, 0, ebufp);
	PIN_FLIST_DESTROY_EX(&r_i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_o_flistp, NULL);
	/***added changes for setting the base plan grant product charged_to date to the valid_to date of fuptop product***/
	if((fup_status != NULL && *indicator == 2) || (action_hold && *indicator == 2) || (action_fup && !fup_status) || action_topup)
	{  
		//getting the mso_purchased_plan base plan
		fm_mso_cust_get_bp_obj( ctxp, acct_pdp, service_obj, -1, &mso_bb_bp_obj, &mso_bb_obj, ebufp);
		if(PIN_POID_IS_NULL(mso_bb_obj)){
                      PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"mso_pur_plan is not active");
		      fm_mso_cust_get_bp_obj( ctxp, acct_pdp, service_obj, MSO_PROV_HOLD, &mso_bb_bp_obj, &mso_bb_obj, ebufp);
		      if(PIN_POID_IS_NULL(mso_bb_obj)){
                           PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"mso_pur_plan is not in hold");
			   fm_mso_cust_get_bp_obj( ctxp, acct_pdp, service_obj, MSO_PROV_IN_PROGRESS, &mso_bb_bp_obj, &mso_bb_obj, ebufp);
                	   if(PIN_POID_IS_NULL(mso_bb_obj)){
                              PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"mso_pur_plan is not in progress");
			      pin_set_err(ebufp, PIN_ERRLOC_FM,
                              PIN_ERRCLASS_SYSTEM_DETERMINATE,
                              MSO_ERR_PLAN_NOT_FOUND, 0, 0, 0);
                              PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                              "mso_purchased_plan not found", ebufp);
                              return;
			   }
		      }
		 }

		if (PIN_ERRBUF_IS_ERR(ebufp)){
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_mso_update_ncr_validity : Error after returning function fm_mso_cust_get_bp_obj", ebufp);
		}
		if(action_topup && (action_fup && !fup_status))
		{
			mso_bb_obj = PIN_FLIST_FLD_GET(payload_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, 0, ebufp);
		}
		if(mso_bb_obj != NULL)
		{
			//get the base plan grant product's charged_to date 
			end_date = fm_prov_get_expiry(ctxp, mso_bb_obj, acct_pdp, service_obj, inc, &start, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp)){
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        	"fm_mso_update_ncr_validity : Error after returning function fm_prov_get_expiry", ebufp);
			}
		}
			
	}
	/* Get the sub balances*/
	/*r_i_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(r_i_flistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_obj, ebufp);
	PIN_FLIST_FLD_SET(r_i_flistp, PIN_FLD_POID, acct_pdp, ebufp);*/
	/***** Pavan Bellala : Defect - While doing Topup customer credict values Not refelected ***
	bal_flags = 1;
	***** Pavan Bellala : Defect - While doing Topup customer credict values Not refelected ***/
	//bal_flags = PIN_BAL_GET_ALL_BARE_RESULTS;

	current_t=pin_virtual_time ((time_t *) NULL);
	/*PIN_FLIST_FLD_SET(r_i_flistp, PIN_FLD_FLAGS, &bal_flags, ebufp);
	PIN_FLIST_FLD_SET(r_i_flistp, PIN_FLD_END_T, &current_t, ebufp);
	PIN_FLIST_ELEM_SET(r_i_flistp, NULL, PIN_FLD_BALANCES, PIN_ELEMID_ANY, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read balance group input list", r_i_flistp);
	PCM_OP(ctxp, PCM_OP_BAL_GET_BALANCES , 0, r_i_flistp, &r_o_flistp, ebufp);*/
	fm_mso_get_balances(ctxp, acct_pdp, current_t, &r_o_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read balance group output list", r_o_flistp);
	//PIN_FLIST_DESTROY_EX(&r_i_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_update_ncr_validity - Error in calling BAL_GET_BALANCES", ebufp);
		goto CLEANUP;
	}
	// Call function to get the validity end date in case of topup for FUP plan.
	if(action_topup) {
		get_topup_ncr_valid_to_fup(ctxp, service_obj, plan_obj, r_o_flistp, &fup_valid_to, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Get FUP balance validity error: Error", ebufp);
			goto CLEANUP;
		}
	}

	bal_elem_id = 0;
	cookie = NULL;
	while ((bal_flistp = PIN_FLIST_ELEM_GET_NEXT(r_o_flistp, PIN_FLD_BALANCES, &bal_elem_id, 1, &cookie, ebufp)) != NULL)
	{
		//vp = (pin_flist_t*)PIN_FLIST_ELEM_ADD(lc_out_flistp, PIN_FLD_BALANCES, bal_elem_id, ebufp );
		if (  bal_elem_id != MSO_FREE_MB && bal_elem_id != MSO_FUP_TRACK )
		{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Currency resource\n");
				continue;
		}
		// Added below to avoid changing validity in case of UnHold for FUP plan
		if((action_unhold) &&
		     bal_elem_id == MSO_FUP_TRACK) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "FUP balance for Hold service");
			continue;
		}

		if(fup_status && *fup_status == 1)
                {
                        if(bal_elem_id != MSO_FUP_TRACK)
                        {
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Not a FUP resource\n");
                                continue;
                        }
                }		
	
		// Pawan: Commented below logic since it is not correct in a scenario when topup 
		// in done within 1st month for a quarterly plan. Now fup_valid_to is calculated above.

		// In TOPUP flow for FUP resource end date must be set to 1 month from the expiry date
		// of current plan. Purcahse end date - bill when = expiry date of current plan
		// So we subtract bill_when and add 1 month to it.
		//timeeff = localtime(pur_end_t);
		//timeeff->tm_mon = timeeff->tm_mon - bill_when + 1;
		//fup_valid_to = mktime (timeeff);

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Non Currency resource\n");
		set_in_flistp = PIN_FLIST_CREATE(ebufp);
		counter = PIN_FLIST_ELEM_COUNT(bal_flistp, PIN_FLD_SUB_BALANCES, ebufp);
		if ( counter < 1 )
				continue;
		sub_bal_id = 0;
		sub_cookie = NULL;
		while ((sub_bal_flistp = PIN_FLIST_ELEM_GET_NEXT(bal_flistp, PIN_FLD_SUB_BALANCES, &sub_bal_id, 1, 
					&sub_cookie, ebufp )) != NULL)
		{
			/****  Pavan Bellala: Defect - While doing Topup customer credict values Not refelected ***
				Added if condition
			****  Pavan Bellala: Defect - While doing Topup customer credict values Not refelected ***/
			if(action_hold)
			{
				pp_obj = PIN_FLIST_FLD_GET(sub_bal_flistp,PIN_FLD_GRANTOR_OBJ,1,ebufp);
				if(pp_obj)
				{
					r_in_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(r_in_flistp, PIN_FLD_POID, (void *)pp_obj, ebufp);
					
					PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, r_in_flistp, &r_out_flistp, ebufp);
					if (PIN_ERRBUF_IS_ERR(ebufp)){
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
							"fm_mso_update_ncr_validity: Error", ebufp);
						PIN_FLIST_DESTROY_EX(&r_in_flistp, NULL);
						return;
					}
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read Grantor product output flist", r_out_flistp);
					status = PIN_FLIST_FLD_GET(r_out_flistp, PIN_FLD_STATUS, 0, ebufp);
					product_pdp = PIN_FLIST_FLD_GET(r_out_flistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
					if(status && *status == PIN_BUNDLE_STATUS_CANCELLED)
					{
						// This is carried forward balance. No need to change validity.
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "stauts cancelled..");
						continue;
					}
					cyc_fee_flistp = PIN_FLIST_ELEM_GET(r_out_flistp, PIN_FLD_CYCLE_FEES, 1,1, ebufp);
					if(cyc_fee_flistp) {
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "feting charged_to..");
						chg_to = PIN_FLIST_FLD_TAKE(cyc_fee_flistp, PIN_FLD_CHARGED_TO_T, 0, ebufp);
					}
					PIN_FLIST_DESTROY_EX(&r_in_flistp, NULL);
					PIN_FLIST_DESTROY_EX(&r_out_flistp, NULL);
				}
			}

			valid_from_tp = PIN_FLIST_FLD_GET(sub_bal_flistp,PIN_FLD_VALID_FROM,1,ebufp);
			valid_to_tp = PIN_FLIST_FLD_GET(sub_bal_flistp,PIN_FLD_VALID_TO,1,ebufp);
			if(fup_status != NULL)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "This is fup topup");
			}
			else if ((( now_t >= (*valid_from_tp)) || (action_topup & pvt >= (*valid_from_tp))) && ( now_t <= (*valid_to_tp))) 
			{
				args_flistp = PIN_FLIST_ELEM_ADD(set_in_flistp, PIN_FLD_SUB_BALANCES, cntr, ebufp);
				PIN_FLIST_FLD_COPY(sub_bal_flistp, PIN_FLD_VALID_FROM, args_flistp, PIN_FLD_VALID_FROM, ebufp );
				PIN_FLIST_FLD_COPY(sub_bal_flistp, PIN_FLD_VALID_TO, args_flistp, PIN_FLD_VALID_TO, ebufp );
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ELEMENT_ID, &sub_bal_id, ebufp);
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_RESOURCE_ID, &bal_elem_id, ebufp);
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_obj, ebufp);
				cntr = cntr + 1;
				args_flistp = PIN_FLIST_ELEM_ADD(set_in_flistp, PIN_FLD_SUB_BALANCES, cntr, ebufp);
				PIN_FLIST_FLD_COPY(sub_bal_flistp, PIN_FLD_VALID_FROM, args_flistp, PIN_FLD_VALID_FROM, ebufp );
				if(action_topup && bal_elem_id == MSO_FUP_TRACK) {
					PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_VALID_TO, &end_date, ebufp);
				}
				else if(action_hold && bal_elem_id == MSO_FUP_TRACK && chg_to) {
					/**added changes not to set the charged_to_t to the bal_grp valid_to for the fup topup products*******/
					fup_top = fm_mso_get_product_priority(ctxp, product_pdp, &ret_flistp, ebufp);
					if(fup_top == 1 && *indicator == 2)
					{
						if(*valid_from_tp >= start) 
						{
							PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_VALID_TO, &end_date, ebufp);
						}
						else
						{
							PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_VALID_TO, &start, ebufp);
						}
					}
					else if(fup_top == 1 && *indicator == 1)
					{
						PIN_FLIST_FLD_COPY(sub_bal_flistp, PIN_FLD_VALID_TO, args_flistp, PIN_FLD_VALID_TO, ebufp );		
					}
					else
					{
						PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_VALID_TO, chg_to, ebufp);
					}
				}
				else if(action_fup && bal_elem_id == MSO_FUP_TRACK && end_date)
				{
					PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_VALID_TO, &end_date, ebufp);
				}
				else {
					PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_VALID_TO, pur_end_t, ebufp);
				}
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ELEMENT_ID, &sub_bal_id, ebufp);
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_RESOURCE_ID, &bal_elem_id, ebufp);
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_obj, ebufp);
				cntr = cntr + 1;
			} 
			else if ((*valid_from_tp) > now_t && action_topup == 0) 
			{
				args_flistp = PIN_FLIST_ELEM_ADD(set_in_flistp, PIN_FLD_SUB_BALANCES, cntr, ebufp);
				//PIN_FLIST_FLD_COPY(sub_bal_flistp, PIN_FLD_VALID_FROM, args_flistp, PIN_FLD_VALID_FROM, ebufp );
				PIN_FLIST_FLD_COPY(sub_bal_flistp, PIN_FLD_VALID_TO, args_flistp, PIN_FLD_VALID_TO, ebufp );
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_VALID_FROM, (void *)&now_t, ebufp);
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ELEMENT_ID, &sub_bal_id, ebufp);
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_RESOURCE_ID, &bal_elem_id, ebufp);
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_obj, ebufp);
				cntr = cntr + 1;
				args_flistp = PIN_FLIST_ELEM_ADD(set_in_flistp, PIN_FLD_SUB_BALANCES, cntr, ebufp);
				//PIN_FLIST_FLD_COPY(sub_bal_flistp, PIN_FLD_VALID_FROM, args_flistp, PIN_FLD_VALID_FROM, ebufp );
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_VALID_FROM, (void *)&now_t, ebufp);
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_VALID_TO, pur_end_t, ebufp);
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ELEMENT_ID, &sub_bal_id, ebufp);
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_RESOURCE_ID, &bal_elem_id, ebufp);
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_obj, ebufp);
				cntr = cntr + 1;
			}
			//lc_sub_bal_array = PIN_FLIST_COPY(args_flistp, ebufp);
			//PIN_FLIST_FLD_COPY(sub_bal_flistp, PIN_FLD_CURRENT_BAL, lc_sub_bal_array, PIN_FLD_CURRENT_BAL, ebufp );
			//PIN_FLIST_ELEM_PUT(vp, lc_sub_bal_array, PIN_FLD_SUB_BALANCES, cntr, ebufp);
		}
		if(fup_status != NULL)
		{
				args_flistp = PIN_FLIST_ELEM_ADD(set_in_flistp, PIN_FLD_SUB_BALANCES, cntr, ebufp);
				sub_bal_flistp = PIN_FLIST_ELEM_GET(bal_flistp, PIN_FLD_SUB_BALANCES, sub_bal_id, 1, ebufp);	
				valid_to_tp = PIN_FLIST_FLD_GET(sub_bal_flistp,PIN_FLD_VALID_TO,1,ebufp);	
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_VALID_TO, valid_to_tp, ebufp);
                                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_VALID_FROM, (void *)valid_from_tp, ebufp);
                                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ELEMENT_ID, &sub_bal_id, ebufp);
                                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_RESOURCE_ID, &bal_elem_id, ebufp);
                                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_obj, ebufp);
                                cntr = cntr + 1;
                                args_flistp = PIN_FLIST_ELEM_ADD(set_in_flistp, PIN_FLD_SUB_BALANCES, cntr, ebufp);
				valid_from_tp = PIN_FLIST_FLD_GET(sub_bal_flistp, PIN_FLD_VALID_FROM, 1, ebufp);
				timeeff = localtime(valid_from_tp);
				timeeff->tm_mon = timeeff->tm_mon + 1;
				valid_to_t = mktime (timeeff);
                                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_VALID_FROM, (void *)valid_from_tp, ebufp);
				if(end_date)
				{
					PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_VALID_TO, &end_date, ebufp);
				}
				else
				{
					pin_set_err(ebufp, PIN_ERRLOC_FM,
                                	PIN_ERRCLASS_SYSTEM_DETERMINATE,
                                	PIN_ERR_BAD_VALUE, PIN_FLD_PLAN_OBJ, 0, 0);
				}
                                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ELEMENT_ID, &sub_bal_id, ebufp);
                                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_RESOURCE_ID, &bal_elem_id, ebufp);
                                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_obj, ebufp);

		}
		PIN_FLIST_FLD_SET(set_in_flistp, PIN_FLD_POID, acct_pdp, ebufp);
		PIN_FLIST_FLD_COPY(payload_flistp, PIN_FLD_PROGRAM_NAME, set_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp );
		PIN_FLIST_FLD_SET(set_in_flistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_obj, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_ncr_validity: change validity input flist", set_in_flistp);
		PCM_OP(ctxp, PCM_OP_BAL_CHANGE_VALIDITY , 0, set_in_flistp, &set_res_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&set_in_flistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_BAL_CHANGE_VALIDITY", ebufp);
			goto CLEANUP;
		}
	}

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&r_i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_o_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&set_in_flistp, NULL);

	return;
}
/*************************************************
* Funtion : fm_mso_update_base_pp_obj()
*	This function is called only in case of
*	Add MB-GB and FUP Topup where mso_pp_obj for
*	base plan is not passed in input.
*************************************************/
static void
fm_mso_update_base_pp_obj(
	pcm_context_t           *ctxp,
	pin_flist_t             *in_flistp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t             *srch_iflistp = NULL;
	pin_flist_t             *srch_oflistp = NULL;
	pin_flist_t             *args_flist = NULL;
	int32                   srch_flag = 256;
	// Added or condition in below template to handle bulk add plan. See details below while setting value.
	char                    *s_template = "select X from /mso_purchased_plan where F1 = V1 and ( F2 = V2 or F3 = V3 ) and F4 = V4 ";
	poid_t                  *pdp = NULL;
	poid_t                  *mso_bp_obj = NULL;
	int32					status = 0;
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_update_base_pp_obj", ebufp);
		return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_base_pp_obj input flist", in_flistp);
	status = MSO_PROV_STATE_IN_PROGRESS;
	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	pdp = PIN_POID_CREATE(1, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(srch_iflistp, PIN_FLD_POID, pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, s_template, ebufp);

	args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, args_flist, PIN_FLD_ACCOUNT_OBJ, ebufp);

	args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(args_flist, PIN_FLD_STATUS, &status, ebufp);

	// Added 3rd argument as OR condition to handle bulk add plan where same file has 2 or more
	// add plan for same account. In that case first respone is processed but after that  
	// remaining respone fails since it cannot find mso_pp with In-Progress status.
	status = MSO_PROV_ACTIVE;
	args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 3, ebufp );
	PIN_FLIST_FLD_SET(args_flist, PIN_FLD_STATUS, &status, ebufp);

	args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 4, ebufp );
	PIN_FLIST_FLD_SET(args_flist, PIN_FLD_DESCR, "base plan", ebufp);
	
	args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(args_flist, PIN_FLD_POID, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_base_pp_obj search input", srch_iflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_update_base_pp_obj - Error in calling SEARCH", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_update_base_pp_obj search output", srch_oflistp);
	args_flist = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp );
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"MSO Purchased Plan (with In progress status) Not Found", ebufp);
		goto CLEANUP;
	}
	
	mso_bp_obj = PIN_FLIST_FLD_TAKE(args_flist, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	
	status = MSO_PROV_STATE_ACTIVE;
	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_POID, mso_bp_obj, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_STATUS, &status, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_update_base_pp_obj Write Flds input", srch_iflistp);

	//Call the PCM_OP_WRITE_FLDS opcode
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, srch_iflistp, &srch_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_update_base_pp_obj: Error", ebufp);
		goto CLEANUP;
	}

	CLEANUP:
		PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	return;
}

static void
get_topup_ncr_valid_to_fup(
	pcm_context_t           *ctxp,
	poid_t                  *svc_obj,
	poid_t                  *plan_obj,
	pin_flist_t             *in_flistp,
	time_t			*fup_valid_to,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t             *srch_iflistp = NULL;
	pin_flist_t             *srch_oflistp = NULL;
	pin_flist_t             *args_flist = NULL;
	pin_flist_t             *res_flistp = NULL;
	pin_flist_t             *bal_flistp = NULL;
	pin_flist_t             *sub_bal_flistp = NULL;
	pin_flist_t             *prod_flistp = NULL;
	int32                   srch_flag = 0;
	char                    *s_template = "select X from /mso_purchased_plan where F1 = V1 and F2 = V2 and F3 = V3 order by mod_t desc ";

	poid_t                  *pdp = NULL;
	poid_t                  *pp_obj = NULL;
	poid_t                  *grantor_obj = NULL;
	int32			status = 0;
	int32			bal_elem_id = 0;
	int32			sub_bal_id = 0;
	int32			elem_id = 0;
	void			*vp = NULL;
	pin_cookie_t		cookie = NULL;
	pin_cookie_t		bal_cookie = NULL;
	pin_cookie_t		sub_cookie = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling get_topup_ncr_valid_to_fup", ebufp);
		return;
	}
	*fup_valid_to = 0;
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "get_topup_ncr_valid_to_fup input flist", in_flistp);
	// 1.Find the latest cancelled plan with given plan_obj since it is topup flow.
	status = MSO_PROV_TERMINATE;
	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	pdp = PIN_POID_CREATE(1, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(srch_iflistp, PIN_FLD_POID, pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, s_template, ebufp);

	args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(args_flist, PIN_FLD_SERVICE_OBJ, svc_obj, ebufp);
	
	args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(args_flist, PIN_FLD_PLAN_OBJ, plan_obj, ebufp);

	args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 3, ebufp );
	PIN_FLIST_FLD_SET(args_flist, PIN_FLD_STATUS, &status, ebufp);

	PIN_FLIST_ELEM_SET(srch_iflistp, NULL, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	//PIN_FLIST_FLD_SET(args_flist, PIN_FLD_POID, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "get_topup_ncr_valid_to_fup search input", srch_iflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"get_topup_ncr_valid_to_fup - Error in calling SEARCH", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"get_topup_ncr_valid_to_fup search output", srch_oflistp);
	// Fetch the first element in search output.
	res_flistp = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, 0, 0, ebufp );

	// 2. Find the FUP sub balance that was granted by cancelled plan (grant product)
	//	The validity of this sub balance will be extended during topup as 
	//	existing + 1month. So the newly granted sub bal must have same validity.
	while ((bal_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_BALANCES, 
			&bal_elem_id, 1, &bal_cookie, ebufp)) != NULL)
	{
		if (bal_elem_id != MSO_FUP_TRACK )
		{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Non an FUP resource");
				continue;
		}
		// Loop through each sub balance of FUP resource.
		sub_cookie = NULL;
		while ((sub_bal_flistp = PIN_FLIST_ELEM_GET_NEXT(bal_flistp, PIN_FLD_SUB_BALANCES, 
			&sub_bal_id, 1, &sub_cookie, ebufp )) != NULL)
		{
			grantor_obj = PIN_FLIST_FLD_GET(sub_bal_flistp, PIN_FLD_GRANTOR_OBJ, 0, ebufp);
			PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Grantor Obj :",grantor_obj);
			cookie = NULL;
			while ((prod_flistp = PIN_FLIST_ELEM_GET_NEXT(res_flistp, PIN_FLD_PRODUCTS, 
					&elem_id, 1, &cookie, ebufp)) != NULL)
			{
				pp_obj = PIN_FLIST_FLD_GET(prod_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, 0, ebufp );
				PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Pur Prod Obj :",pp_obj);
				if(PIN_POID_COMPARE(grantor_obj, pp_obj, 0, ebufp)==0)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Poid matched...");
					vp = PIN_FLIST_FLD_GET(sub_bal_flistp, PIN_FLD_VALID_TO, 0, ebufp);
					if(vp) 
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Assigning valid to..");
						*fup_valid_to = *(time_t *)vp;
					}
				}
			}
		}
	}
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
}

static void
fm_mso_terminate_service_ip_dissociate(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_errbuf_t    *ebufp)
{
        pin_flist_t     *search_inflistp = NULL;
        pin_flist_t     *search_outflistp = NULL;
        pin_flist_t     *args_flistp = NULL;
        pin_flist_t     *results_flistp = NULL;
        pin_flist_t     *dissociate_inflistp = NULL;
        pin_flist_t     *dissociate_outflistp = NULL;
        pin_flist_t     *device_flistp = NULL;
        pin_flist_t     *device_oflistp = NULL;

	int64		db = 0;
	int		search_flags = 256;
	int		elem_id = 0;
	pin_cookie_t	cookie = NULL;
	char		*search_template = "select X from /device where F1 = V1 and F2.type like V2 ";

	poid_t		*service_obj = NULL;

	
	service_obj = PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_SERVICE_OBJ,1,ebufp);
	db = PIN_POID_GET_DB(service_obj);

        search_inflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
        PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
        results_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_SERVICES, 1, ebufp);
        PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_PUT(args_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/device/ip/%", -1, ebufp), ebufp);

        results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_terminate_service_ip_dissociate search device input list", search_inflistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
        PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
                PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_terminate_service_ip_dissociate search device output flist", search_outflistp);

        search_flags = 0;
        while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(search_outflistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
        {
                dissociate_inflistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, dissociate_inflistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(dissociate_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, dissociate_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);
                PIN_FLIST_FLD_SET(dissociate_inflistp, PIN_FLD_DESCR, "Terminate(Svc)- Dissociate IP", ebufp);

                args_flistp = PIN_FLIST_ELEM_ADD(dissociate_inflistp, PIN_FLD_SERVICES, 0, ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, args_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, args_flistp, PIN_FLD_SERVICE_OBJ, ebufp);

                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_terminate_service_device_deassociate device remove input list", dissociate_inflistp);
                PCM_OP(ctxp, PCM_OP_DEVICE_ASSOCIATE , 0, dissociate_inflistp, &dissociate_outflistp, ebufp);
                PIN_FLIST_DESTROY_EX(&dissociate_inflistp, NULL);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_DEVICE_ASSOCIATE device remove", ebufp);
                        PIN_FLIST_DESTROY_EX(&dissociate_outflistp, NULL);
                        return;
                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
				"fm_mso_terminate_service_device_deassociate device remove output flist", dissociate_outflistp);
                PIN_FLIST_DESTROY_EX(&dissociate_outflistp, NULL);
		/**** Remove plan from device *********/
		device_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, device_flistp, PIN_FLD_POID, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(device_flistp, PIN_FLD_PLAN, 0, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, NULL, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_TYPE, NULL, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Delete plans input flist ", device_flistp);
		PCM_OP(ctxp, PCM_OP_DELETE_FLDS, 0, device_flistp, &device_oflistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Delete plans output flist", device_oflistp);
		PIN_FLIST_DESTROY_EX(&device_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&device_oflistp, NULL);
        }


}

/**************************************************
* Function  : fm_mso_prov_update_product_cycle_fee_charge_dates()
* Decription: Purchased_product READ_OBJ 
*
*
***************************************************/

void
fm_mso_prov_update_product_cycle_fee_charge_dates(
	pcm_context_t	*ctxp,
	poid_t		*acct_pdp, 
	poid_t		*service_obj, 
	poid_t		*offering_obj,
	pin_flist_t	**ret_flistpp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*read_in_flistp = NULL;
	pin_flist_t	*read_out_flistp = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_prov_update_product_cycle_fee_charge_dates", ebufp);
                return;
        }
 	read_in_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(read_in_flistp, PIN_FLD_POID, offering_obj, ebufp );
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_prov_update_product_cycle_fee_charge_dates read input", read_in_flistp);
        PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_in_flistp, &read_out_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&read_in_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
        	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
        	PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);
        	return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_prov_update_product_cycle_fee_charge_dates read output", read_out_flistp);
	*ret_flistpp = PIN_FLIST_COPY(read_out_flistp,ebufp);	
}

/**************************************************
* Function  : fm_mso_modify_bal_grp()
* Decription: setting the fuptopup product bal_grp
*             valid_from exactly one month to 
*	      valid_to date
*
*
***************************************************/
static void
fm_mso_modify_bal_grp(
        pcm_context_t           *ctxp,
        poid_t	                *bal_grp_pdp,
        pin_errbuf_t            *ebufp)
{
	int64			bal_elem_id = 1000104;
	pin_flist_t		*r_i_flistp = NULL;
	pin_flist_t		*r_o_flistp = NULL;
	poid_t			*bal_grp_obj = NULL;	
	pin_flist_t		*bal_flistp = NULL;
	pin_flist_t		*sub_bal_flistp = NULL;
	pin_flist_t		*set_in_flistp = NULL;
	pin_flist_t		*r_in_flistp = NULL;
	pin_flist_t		*r_out_flistp = NULL;
	pin_cookie_t            cookie = NULL;
	poid_t			*pp_obj = NULL;
	int32			*status = NULL;
	poid_t			*product_pdp = NULL;
	poid_t			*acct_pdp = NULL;
	int32			fup_top = 0;
	time_t			*valid_frm = NULL;
	time_t			valid_to = 0;
	pin_flist_t		*set_res_flistp = NULL;
	int32			cntr = 0;
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	int32                   sub_bal_id = 0;
	struct tm       	*timeeff;
	int			count = -1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_modify_bal_grp", ebufp);
                return;
        }	
	
	r_i_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(r_i_flistp, PIN_FLD_POID, (void *)bal_grp_pdp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "Read Bal_Grp Input Flist", r_i_flistp);
        PCM_OP(ctxp, PCM_OP_READ_OBJ, 32, r_i_flistp, &r_o_flistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp)) {
		
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "Read Bal_Grp error: Error", ebufp);
		PIN_FLIST_DESTROY_EX(&r_o_flistp, NULL);
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "Read Bal_Grp Output Flist", r_o_flistp);	
	bal_grp_obj = PIN_FLIST_FLD_GET(r_o_flistp, PIN_FLD_POID, 1, ebufp);
	bal_flistp = PIN_FLIST_ELEM_GET(r_o_flistp, PIN_FLD_BALANCES, 1000104, 1, ebufp);
	if(!bal_flistp)
	 return;

	set_in_flistp = PIN_FLIST_CREATE(ebufp);	
	while ((sub_bal_flistp = PIN_FLIST_ELEM_GET_NEXT(bal_flistp, PIN_FLD_SUB_BALANCES, &sub_bal_id, 1, &cookie, ebufp)) != NULL)
        {
		pp_obj = PIN_FLIST_FLD_GET(sub_bal_flistp,PIN_FLD_GRANTOR_OBJ,1,ebufp);
		if(pp_obj)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "NOT FUP TOPUP PLAN");
			r_in_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(r_in_flistp, PIN_FLD_POID, (void *)pp_obj, ebufp);
			PIN_FLIST_FLD_SET(r_in_flistp, PIN_FLD_STATUS, NULL, ebufp);
			PIN_FLIST_FLD_SET(r_in_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
			PIN_FLIST_FLD_SET(r_in_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
                        PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, r_in_flistp, &r_out_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&r_in_flistp, NULL);
                        if (PIN_ERRBUF_IS_ERR(ebufp)){
                                  PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                                        "fm_mso_modify_bal_grp: Error", ebufp);
                                  PIN_FLIST_DESTROY_EX(&r_out_flistp, NULL);
                                  return;
                        }
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read Grantor product output flist", r_out_flistp);
                        status = PIN_FLIST_FLD_GET(r_out_flistp, PIN_FLD_STATUS, 0, ebufp);
                        product_pdp = PIN_FLIST_FLD_GET(r_out_flistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
			acct_pdp = PIN_FLIST_FLD_GET(r_out_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
                        if(status && *status == PIN_BUNDLE_STATUS_CANCELLED)
                        {
                        	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "stauts cancelled..");
				PIN_FLIST_DESTROY_EX(&r_out_flistp, NULL);
                        	continue;
                        }
			fup_top = fm_mso_get_product_priority(ctxp, product_pdp, &ret_flistp, ebufp);
			if(fup_top == -1)
			{
			   PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "NOT FUP TOPUP PLAN");	
			   PIN_FLIST_DESTROY_EX(&r_out_flistp, NULL);
			   continue;
			}
			PIN_FLIST_DESTROY_EX(&r_out_flistp, NULL);
	
                        args_flistp = PIN_FLIST_ELEM_ADD(set_in_flistp, PIN_FLD_SUB_BALANCES, cntr, ebufp);
			valid_frm = PIN_FLIST_FLD_GET(sub_bal_flistp, PIN_FLD_VALID_FROM, 1, ebufp);
			PIN_FLIST_FLD_COPY(sub_bal_flistp, PIN_FLD_VALID_TO, args_flistp, PIN_FLD_VALID_TO, ebufp );
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_VALID_FROM, (void *)valid_frm, ebufp);
			if(valid_frm != NULL)
                        {
                               timeeff = localtime(valid_frm);
				
			       timeeff->tm_mon = timeeff->tm_mon + 1;

                               valid_to = mktime (timeeff);
			}
                        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ELEMENT_ID, &sub_bal_id, ebufp);
                        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_RESOURCE_ID, &bal_elem_id, ebufp);
                        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_obj, ebufp);
                        cntr = cntr + 1;
                        args_flistp = PIN_FLIST_ELEM_ADD(set_in_flistp, PIN_FLD_SUB_BALANCES, cntr, ebufp);
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_VALID_FROM, (void *)valid_frm, ebufp);
                        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_VALID_TO, &valid_to, ebufp);
                        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ELEMENT_ID, &sub_bal_id, ebufp);
                        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_RESOURCE_ID, &bal_elem_id, ebufp);
                        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_obj, ebufp);
                        cntr = cntr + 1;			
	
		}
	    }
	    count = PIN_FLIST_ELEM_COUNT(set_in_flistp, PIN_FLD_SUB_BALANCES, ebufp);
	    if(count == 0)
	    {
	    	PIN_FLIST_DESTROY_EX(&set_in_flistp, NULL);
		return;
	    }
	    PIN_FLIST_FLD_SET(set_in_flistp, PIN_FLD_POID, acct_pdp, ebufp);
            PIN_FLIST_FLD_SET(set_in_flistp, PIN_FLD_PROGRAM_NAME, "FUP TOPUP BAL VAL CNG", ebufp);
            PIN_FLIST_FLD_SET(set_in_flistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_obj, ebufp);
	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_bal_grp change validity input flist", set_in_flistp);
            PCM_OP(ctxp, PCM_OP_BAL_CHANGE_VALIDITY , 0, set_in_flistp, &set_res_flistp, ebufp);
            PIN_FLIST_DESTROY_EX(&set_in_flistp, NULL);
            if (PIN_ERRBUF_IS_ERR(ebufp))
            {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_BAL_CHANGE_VALIDITY", ebufp);
		PIN_FLIST_DESTROY_EX(&set_res_flistp, NULL);
                return; 
            }
	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_bal_grp: change validity output flist", set_res_flistp);
	    PIN_FLIST_DESTROY_EX(&set_res_flistp, NULL);
}

static void
fm_mso_update_only_order_status(
	pcm_context_t		*ctxp,
	poid_t			*order_pdp,
	pin_flist_t		*in_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*w_i_flistp = NULL;
	pin_flist_t		*w_o_flistp = NULL;
	pin_flist_t 		*tmp_flistp = NULL;
	
	char			*error_code = NULL;
	char			*error_descr = NULL;
	
	int32			*status = NULL;
	


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"fm_mso_update_provisioning_and_order_status: Input Flist", in_flistp);

	if ( (tmp_flistp = PIN_FLIST_ELEM_GET (in_flistp,
			MSO_FLD_TASK, PIN_ELEMID_ANY, 1, ebufp))
						  != (pin_flist_t *)NULL ) 
	{
		status = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_STATUS, 0, ebufp);
		if (status && *status != MSO_PROV_STATE_SUCCESS)
		{
			error_code = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_ERROR_CODE, 0, ebufp);
			error_descr = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_ERROR_DESCR, 0, ebufp);
		}
	}
	
	//Update the status in the order
	w_i_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(w_i_flistp, PIN_FLD_POID, (void *)order_pdp, ebufp);
	PIN_FLIST_FLD_SET(w_i_flistp, PIN_FLD_STATUS, (void *)status, ebufp);
	if (status && *status != MSO_PROV_STATE_SUCCESS)
	{
		PIN_FLIST_FLD_SET(w_i_flistp, PIN_FLD_ERROR_CODE, error_code, ebufp);
		PIN_FLIST_FLD_SET(w_i_flistp, PIN_FLD_ERROR_DESCR, error_descr, ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_update_only_order_status: Order Write Fields Input Flist", w_i_flistp);
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 32, w_i_flistp, &w_o_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_update_only_order_status: Order Update Error", ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_update_only_order_status: Order Write Fields Output Flist", w_o_flistp);

	cleanup:
	PIN_FLIST_DESTROY_EX(&w_i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&w_o_flistp, NULL);
}

/****************************************************************
* Function: fm_mso_change_plan_check_val_extn()
* Function Checks if any Validity Extension Product
* is configured in higher_arpu table for given acct in given plan
*****************************************************************/
void 
fm_mso_change_plan_check_val_extn(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t             **r_flistpp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t             *s_flistp = NULL;
	pin_flist_t             *args_flistp = NULL;
	pin_flist_t             *res_flistp = NULL;
	pin_flist_t             *robj_iflistp = NULL;
	pin_flist_t             *robj_oflistp = NULL;
	poid_t                  *s_pdp = NULL;
	poid_t                  *plan_pdp = NULL;
	poid_t                  *act_pdp = NULL;
	int32                   flags = 256;
	int64                   db = 0;
	int64                   id = -1;
	char   		   *city;
	char                    *si_template = "select X from /higher_arpu_promotion_plan 1 where 1.F1 = V1 and 1.F2 = V2 order by EXPIRATION_MONTH desc ";
	char                    *sj_template = "select X from /higher_arpu_promotion_plan 1 where 1.F1 = V1 and 1.F2 = V2 and 1.F3 = V3 order by EXPIRATION_MONTH desc ";
	pin_flist_t		*ret_flistp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	
	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp));
	s_flistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, si_template, ebufp);
	plan_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
	act_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, act_pdp, ebufp);

	res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_PUT(res_flistp, PIN_FLD_CLASSMARK, NULL, ebufp);
	PIN_FLIST_FLD_PUT(res_flistp, PIN_FLD_PLAN_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_PUT(res_flistp, PIN_FLD_PLAN_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_PUT(res_flistp, MSO_FLD_EXPIRATION_DATE, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Validity Product search input list", s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &ret_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Validity Product search - Error in calling SEARCH", ebufp);
		PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_change_plan_check_val_extn", ret_flistp);
       if(PIN_FLIST_ELEM_COUNT(ret_flistp, PIN_FLD_RESULTS, ebufp) == 0)
	{
		PIN_FLIST_ELEM_DROP(s_flistp,PIN_FLD_ARGS,2,ebufp);
		robj_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(robj_iflistp, PIN_FLD_POID,act_pdp, ebufp);
		PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, robj_iflistp, &robj_oflistp, ebufp);	
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "READ OBJ IN SEARCH OF CITY", robj_oflistp);
		robj_iflistp=PIN_FLIST_ELEM_GET(robj_oflistp, PIN_FLD_NAMEINFO,PIN_ELEMID_ANY,1,ebufp);
		city =PIN_FLIST_FLD_GET(robj_iflistp, PIN_FLD_CITY, 1, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, sj_template, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CITY, city, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, PIN_POID_CREATE(db, "/account", 1, ebufp), ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Validity Product search input list for city", s_flistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &ret_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Validity Product search - Error in calling SEARCH", ebufp);
			PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
			return;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_change_plan_check_val_extn for city", ret_flistp);
	PIN_FLIST_DESTROY_EX(&robj_iflistp, NULL);
	}
	*r_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&args_flistp, NULL);
}

