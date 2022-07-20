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
* 0.1    |25/10/2013 |Satya Prakash     |  CATV Provisioning implementation
*--------------------------------------------------------------------------------------------------------------*
*
****************************************************************************************************************/


#ifndef lint
static  char Sccs_Id[] = "@(#)$Id: fm_mso_prov_create_catv_action.c /cgbubrm_7.3.2.rwsmod/1 2009/08/04 23:37:19 pjegan Exp $";
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


#define ADD_PACKAGE     2
#define CANCEL_PACKAGE  1
#define CHANGE_PACKAGE  3

#define FP_NDS_ALL_CHANNEL 65535
#define FP_VM_ALL_CHANNEL 65535		// VERIMATRIX
#define CUSTOMER_CARE "/profile/customer_care"
#define MSO_FLAG_SRCH_BY_SELF_ACCOUNT 9

/*******************************************************************
 * external symbol for catv provisioning config pointer
 *******************************************************************/
extern cm_cache_t *mso_prov_catv_config_ptr;
extern int32 refresh_pt_interval;
extern void
fm_mso_utils_sequence_no(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

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

PIN_IMPORT void
fm_mso_get_account_info(
	pcm_context_t	*ctxp,
	poid_t		*acnt_pdp,
	pin_flist_t	**ret_flistp,
	pin_errbuf_t	*ebufp);

PIN_IMPORT void
fm_mso_populate_bouquet_id(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        int32                   mso_device_type,
        int32                   city_only,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

PIN_IMPORT void
fm_mso_get_base_pdts(
	pcm_context_t		*ctxp,
	poid_t			*acct_pdp,
	poid_t			*srvc_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

PIN_IMPORT void
insert_char (
	char    *srcp,
	char    in_char,
	char    *dstp);

PIN_IMPORT int32
fm_mso_catv_pt_pkg_type(
        pcm_context_t           *ctxp,
        poid_t                  *prd_pdp,
        pin_errbuf_t            *ebufp);

/*******************************************************************
 * Fuctions defined in this code
 *******************************************************************/

EXPORT_OP void
op_mso_prov_create_catv_action(
        cm_nap_connection_t    *connp,
        int32               opcode,
        int32               flags,
        pin_flist_t         *in_flistp,
        pin_flist_t         **ret_flistpp,
        pin_errbuf_t        *ebufp);

static void
fm_mso_prov_create_catv_action(
        pcm_context_t       *ctxp,
        pin_flist_t         *in_flistp,
        int32               flags,
        int32               *error_code,
        pin_flist_t         **out_flistpp,
        pin_errbuf_t        *ebufp);

static void
prepare_catv_activation_flist(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    char                    **act,
    pin_errbuf_t            *ebufp);

static void
prepare_catv_preactivation_payload(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    char                    **act,
    pin_errbuf_t            *ebufp);

static void
prepare_catv_add_package_flist(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    char                    **act,
    pin_errbuf_t            *ebufp);

static void
prepare_catv_plan_change_flist(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    char                    **act,
    pin_errbuf_t            *ebufp);

static void
prepare_catv_change_preactivated_plan_flist(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    char                    **act,
    pin_errbuf_t            *ebufp);

static void
prepare_catv_cancel_plan_flist(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    char                    **act,
    pin_errbuf_t            *ebufp);

static void
prepare_catv_suspend_flist(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    int32                   *error_codep,
    char                    **act,
    pin_errbuf_t            *ebufp);

static void
prepare_catv_reactivate_flist(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    int32                   *error_codep,
    char                    **act,
    pin_errbuf_t            *ebufp);

static void
prepare_catv_terminate_flist(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    char                    **act,
    pin_errbuf_t            *ebufp);

static void
prepare_catv_change_smart_card_flist(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    char                    **act,
    pin_errbuf_t            *ebufp);

static void
prepare_catv_change_STB_flist(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    char                    **act,
    pin_errbuf_t            *ebufp);

static void
prepare_catv_change_location_flist(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    char                    **act,
    pin_errbuf_t            *ebufp);

extern void
mso_prov_get_ca_id(
        pcm_context_t       *ctxp,
        char                *prov_tag,
        char                *node,
        pin_flist_t         **ret_flistpp,
        pin_errbuf_t        *ebufp);

static void
add_package_detail(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    int32                   flag,
    char                    *node,
    int32                   is_change_bq_id,
    pin_errbuf_t            *ebufp);

static void
prepare_catv_email_payload(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    int32                   *error_codep,
    char                    **act,
    pin_errbuf_t            *ebufp);

static void
prepare_catv_retrack_payload(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    int32                   *error_codep,
    char                    **act,
    pin_errbuf_t            *ebufp);

static void
prepare_catv_set_user_params_payload(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    int32                   *error_codep,
    char                    **act,
    pin_errbuf_t            *ebufp);

static void
prepare_catv_fp_nds_payload(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    int32                   *error_codep,
    char                    **act,
    pin_errbuf_t            *ebufp);

static void
prepare_catv_fp_gospel_payload(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    int32                   *error_codep,
    char                    **act,
    pin_errbuf_t            *ebufp);
	
static void
prepare_catv_fp_single_channel_pk_payload(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    int32                   *error_codep,
    char                    **act,
    pin_errbuf_t            *ebufp);

static void
prepare_catv_fp_all_channnels_single_stb_pk_payload(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    int32                   *error_codep,
    char                    **act,
    pin_errbuf_t            *ebufp);

static void
prepare_catv_fp_all_stb_pk_payload(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    int32                   *error_codep,
    char                    **act,
    pin_errbuf_t            *ebufp);

static void
prepare_catv_resend_zipcode_payload(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    int32                   *error_codep,
    char                    **act,
    pin_errbuf_t            *ebufp);

static void
prepare_catv_change_bouquet_id_payload(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    char                    **act,
    pin_errbuf_t            *ebufp);

static void
prepare_catv_set_bit_payload(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    char                    **act,
    pin_errbuf_t            *ebufp);

static void
prepare_catv_unset_bit_payload(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    char                    **act,
    pin_errbuf_t            *ebufp);

static void
prepare_catv_send_osd_payload(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    int32                   *error_codep,
    char                    **act,
    pin_errbuf_t            *ebufp);

static void
prepare_catv_reset_pin_pk_payload(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    int32                   *error_codep,
    char                    **act,
    pin_errbuf_t            *ebufp);

static void
set_error_descr(
    pin_flist_t             **i_flistpp,
    int                     error_code,
    pin_errbuf_t            *ebufp);

static void
get_subscriber_id(
pcm_context_t           *ctxp,
pin_flist_t             **i_flistpp,
pin_errbuf_t            *ebufp);


static void
fm_mso_get_device_details(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    char                    *device_id,
    pin_errbuf_t            *ebufp);
	
	static void
fm_mso_get_installation_customer_credentials(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    pin_errbuf_t            *ebufp);

static void
prepare_catv_preactv_svc_mod_payload(
    pcm_context_t       *ctxp,
    pin_flist_t         **payload_flistpp,
    int32               *error_codep,
    char                **act,
    pin_errbuf_t        *ebufp);

static void
fm_mso_prov_catv_config_cache_update(
        pcm_context_t *ctxp,
        pin_errbuf_t  *ebufp);

static void
get_mso_prov_priority(
        pcm_context_t           *ctxp,
        char                            *action_type,
        int                                     *order_type,
        pin_flist_t         **ret_flistpp,
        pin_errbuf_t            *ebufp);

static  void
fm_mso_get_cust_active_plans_ca_ids(
        pcm_context_t           *ctxp,
        pin_flist_t             **i_flistp,
        int32                   *error_codep,
        int32                   action_flag,
        pin_errbuf_t            *ebufp);

static  void
fm_mso_get_cust_plan_ca_id(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        poid_t                  *mso_plan_obj,
        char                    *mso_node,
        int32                   *error_codep,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

static void
add_catv_bouquet_id_task(
    pcm_context_t       *ctxp,
    pin_flist_t         **i_flistpp,
    int32               flag,
    int32               task_id,
    pin_errbuf_t        *ebufp);

static void
search_serial_no(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
		pin_flist_t             **ret_flistp,
		pin_errbuf_t            *ebufp);

static void
search_name_info(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
	pin_flist_t             **return_flistp,
	pin_errbuf_t            *ebufp);

static void
fm_mso_get_purchased_prod_nds(
        pcm_context_t           *ctxp,
        pin_flist_t             *inp_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);		
/*******************************************************************
 * Main routine for the MSO_OP_CREATE_CATV_ACTION command
 *******************************************************************/
void
op_mso_prov_create_catv_action(
        cm_nap_connection_t    *connp,
        int32               opcode,
        int32               flags,
        pin_flist_t         *in_flistp,
        pin_flist_t         **ret_flistpp,
        pin_errbuf_t        *ebufp)
{
    pin_flist_t             *r_flistp = NULL;
    pcm_context_t           *ctxp = connp->dm_ctx;
    int32                   oap_success = 0;
    int32                   oap_failure = 1;
    int32                   *prov_flag =  NULL;
    int32                   error_code = 0;
    poid_t                  *pdp = NULL;
    int32		    *t_openp = NULL;
    int32                   t_status = -1;
    char                    code[10];


    /***********************************************************
     * Null out results until we have some.
     ***********************************************************/
    *ret_flistpp = NULL;
    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Insanity check.
     ***********************************************************/
    if (opcode != MSO_OP_PROV_CREATE_CATV_ACTION) {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_OPCODE, 0, 0, opcode);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "bad opcode in op_mso_prov_create_catv_action", ebufp);
        return;
    }

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "op_mso_prov_create_catv_action input flist", in_flistp);

    pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
    t_openp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_TRANS_DONE, 1, ebufp);
    if (!t_openp)
    {
    	t_status = fm_mso_trans_open(ctxp, pdp, 1, ebufp);
    }
    else
    {
         PIN_FLIST_FLD_DROP(in_flistp, PIN_FLD_TRANS_DONE, ebufp);
    }

    prov_flag = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_FLAGS, 0 , ebufp);
    if(prov_flag &&  prov_flag != NULL && *prov_flag == CATV_PREACTV_MOD_SVC){
        // This is preactivated service, we need action info
        prov_flag = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACTION_MODE, 0, ebufp);
    }

    /***********************************************************
     * Call main function to do it
     ***********************************************************/
    fm_mso_prov_create_catv_action(ctxp, in_flistp, flags, &error_code, &r_flistp, ebufp);

    /***********************************************************
     * Results.
     ***********************************************************/
    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_mso_prov_create_catv_action error", ebufp);

        if (t_status == 0)
        {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Aborting transaction");
            PIN_ERRBUF_CLEAR(ebufp);
            fm_mso_trans_abort(ctxp, pdp, ebufp);
        }

	if ( prov_flag && prov_flag != NULL )
	{
		if (*prov_flag ==  CATV_EMAIL ||
		    *prov_flag ==  CATV_RETRACK ||
		    *prov_flag ==  CATV_FP_NDS ||
		    *prov_flag ==  CATV_FP_SINGLE_CHANNEL_PK ||
		    *prov_flag ==  CATV_FP_ALL_CHANNELS_SINGLE_STB_PK ||
		    *prov_flag ==  CATV_FP_ALL_STBS_PK ||
		    *prov_flag ==  CATV_SEND_OSD ||
		    *prov_flag ==  CATV_SET_USER_PARAMETERS ||
		    *prov_flag ==  CATV_RESEND_ZIPCODE ||
		    *prov_flag ==  CATV_RESET_PIN_PK ||
		    *prov_flag ==  PREACTV_RESET_PIN ||
		    *prov_flag ==  PREACTV_BMAIL ||
		    *prov_flag ==  PREACTV_OSD ||
		    *prov_flag ==  PREACTV_RETRACK ||
		    *prov_flag ==  PREACTV_RESEND_ZIP_CODE)
		{
	    //      PIN_ERRBUF_CLEAR(ebufp);
		    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Returning error flist");
		    *ret_flistpp = PIN_FLIST_CREATE(ebufp);
		    PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
		    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
		    if (error_code)
		    {
			set_error_descr(ret_flistpp, error_code, ebufp);
		    }
		}
		else
		{
		    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Returning NULL return flist");
		    *ret_flistpp = (pin_flist_t *)NULL;
		}
	}
        else
        {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Returning NULL return flist");
            *ret_flistpp = (pin_flist_t *)NULL;
        }

        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
    }
    else
    {
        if (t_status == 0)
        {
            fm_mso_trans_commit(ctxp, pdp, ebufp);
        }

        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &oap_success, ebufp);
        *ret_flistpp = r_flistp;
        PIN_ERRBUF_CLEAR(ebufp);
    }

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
         "op_mso_prov_create_catv_action return flist", *ret_flistpp);

    return;
}

/*******************************************************************
 * fm_mso_prov_create_catv_action()
 *
 * Perfomrs following action:
 * 1. call action specific function to enrich the input from service
 *        and package detail
 * 2. get provisioning sequence
 * 3. create provisioning order /mso_prov_order
 *******************************************************************/
static void
fm_mso_prov_create_catv_action(
    pcm_context_t       *ctxp,
    pin_flist_t         *in_flistp,
    int32               flags,
    int32               *error_codep,
    pin_flist_t         **out_flistpp,
    pin_errbuf_t        *ebufp)
{
    int32               *action_flag = NULL;
    int64               db_no;
    poid_t              *service_poidp = NULL;
    poid_t              *acc_poidp = NULL;
    poid_t              *objp = NULL;
    pin_flist_t         *payload_flistp = NULL;
    pin_flist_t         *order_i_flistp     = NULL;
    pin_flist_t         *tmp_flistp= NULL;
    pin_flist_t         *tmp1_flistp= NULL;
    pin_flist_t         *tmp2_flistp= NULL;
    pin_flist_t         *task_flist3= NULL;
    pin_flist_t         *order_seq_i_flistp = NULL;
    pin_flist_t         *order_seq_o_flistp = NULL;
    pin_flist_t         *rfld_iflistp = NULL;
    pin_flist_t         *rfld_oflistp = NULL;
    pin_buf_t           *nota_buf     = NULL;
	pin_cookie_t	cookie = NULL;
    char                *flistbuf = NULL;
    char                *nw_node = NULL;
    int                 flistlen=0;
    int32               count = 0;
	int32		elem_id = 0;
    int                 status = MSO_PROV_STATE_NEW;
    char                *act = NULL;
    int                 ord_type = 0;
        int                                     *rank = 0;
	char                *prg_name = NULL;
	char		*stbp = NULL;
    char       *task_namep= NULL;
	void		*vp = NULL;
	char		new_stbp[64] = "";

//    mso_prov_str_buf_t        dbuf;
    char            str[128];
	int32           network_type=0;
	int32		*sub_typep = NULL;
	int32		dvbip = 0;
    pin_flist_t         *check_delpack_flistp = NULL;
    pin_flist_t         *check_addpack_flistp = NULL;
    int32                task_elem_id = 1;
    int32                out_flag = 0;


    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_prov_create_catv_action input flist", in_flistp);

    /***********************************************************
     * Check mandatory fields are available
     ***********************************************************/
    service_poidp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
    acc_poidp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
    action_flag = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_FLAGS, 0, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "fm_mso_prov_create_catv_action error: required field missing in input flist", ebufp);
        return;
    }
    //Check the network node
        /*rfld_iflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(rfld_iflistp, PIN_FLD_POID,
                (void *)service_poidp, ebufp);
        tmp_flistp = PIN_FLIST_SUBSTR_ADD(rfld_iflistp,
                MSO_FLD_CATV_INFO, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_NETWORK_NODE,
                (void *)NULL, ebufp);
        //Log the input flist
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_mso_prov_create_catv_action error: Read fields input flist", rfld_iflistp);
        //Call the PCM_OP_READ_FLDS opcode
        PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, rfld_iflistp, &rfld_oflistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_mso_prov_create_catv_action error: Error calling READ FIELDS", ebufp);
                goto CLEANUP;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_mso_prov_create_catv_action error: Read fields output flist", rfld_oflistp);
        tmp1_flistp = PIN_FLIST_SUBSTR_GET(rfld_oflistp, MSO_FLD_CATV_INFO, 0, ebufp);
        nw_node = (char *)PIN_FLIST_FLD_GET(tmp1_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_mso_prov_create_catv_action error: Error", ebufp);
                goto CLEANUP;
        }*/
    //if(strcmp(nw_node, "NONE") != 0)
    //{
          //payload_flistp = PIN_FLIST_CREATE(ebufp);
          payload_flistp = PIN_FLIST_COPY(in_flistp,ebufp);

          if ( *action_flag ==  CATV_PREACTIVATION )
          {
              PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_OBJ, payload_flistp, PIN_FLD_POID, ebufp);
              PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, payload_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
          }

          /***********************************************************
           * Enrich input specific to action
           ***********************************************************/
          if ( *action_flag ==  CATV_ACTIVATION )
          {
              prepare_catv_activation_flist(ctxp, &payload_flistp, &act, ebufp);
          }
          else if ( *action_flag ==  CATV_PREACTIVATION )
          {
              prepare_catv_preactivation_payload(ctxp, &payload_flistp, &act, ebufp);
          }
          else if ( *action_flag ==  CATV_ADD_PACKAGE )
          {
              prepare_catv_add_package_flist(ctxp, &payload_flistp, &act, ebufp);
          }
          else if ( *action_flag ==  CATV_PLAN_CHANGE )
          {
              prepare_catv_plan_change_flist(ctxp, &payload_flistp, &act, ebufp);
          }
          else if ( *action_flag ==  CATV_CHANGE_PREACTIVATED_PLAN )
          {
              prepare_catv_change_preactivated_plan_flist(ctxp, &payload_flistp, &act, ebufp);
          }
          else if ( *action_flag ==  CATV_CANCEL_PLAN )
          {
              prepare_catv_cancel_plan_flist(ctxp, &payload_flistp, &act, ebufp);
          }
          else if ( *action_flag ==  CATV_SUSPEND )
          {
              prepare_catv_suspend_flist(ctxp, &payload_flistp, error_codep, &act, ebufp);
          }
          else if ( *action_flag ==  CATV_REACTIVATE )
          {
              prepare_catv_reactivate_flist(ctxp, &payload_flistp, error_codep, &act, ebufp);
          }
          else if ( *action_flag ==  CATV_TERMINATE )
          {
              prepare_catv_terminate_flist(ctxp, &payload_flistp, &act, ebufp);
          }
          else if ( *action_flag ==  CATV_CHANGE_VC )
          {
              prepare_catv_change_smart_card_flist(ctxp, &payload_flistp, &act, ebufp);
          }
          else if ( *action_flag ==  CATV_CHANGE_STB )
          {
              prepare_catv_change_STB_flist(ctxp, &payload_flistp, &act, ebufp);
          }
          else if ( *action_flag ==  CATV_LOCATION_CHANGE )
          {
              prepare_catv_change_location_flist(ctxp, &payload_flistp, &act, ebufp);
          }
          else if ( *action_flag ==  CATV_LOCATION_CHANGE )
          {
              prepare_catv_change_location_flist(ctxp, &payload_flistp, &act, ebufp);
          }
          else if ( *action_flag ==  CATV_EMAIL )
          {
              prepare_catv_email_payload(ctxp, &payload_flistp, error_codep, &act, ebufp);
          }
          else if ( *action_flag ==  CATV_RETRACK )
          {
              prepare_catv_retrack_payload(ctxp, &payload_flistp, error_codep, &act, ebufp);
          }
          else if ( *action_flag ==  CATV_SET_USER_PARAMETERS )
          {
              prepare_catv_set_user_params_payload(ctxp, &payload_flistp, error_codep, &act, ebufp);
          }
          else if ( *action_flag ==  CATV_FP_NDS )
          {
              prepare_catv_fp_nds_payload(ctxp, &payload_flistp, error_codep, &act, ebufp);
          }
		  
          else if ( *action_flag ==  CATV_FP_SINGLE_CHANNEL_PK )
          {
              prepare_catv_fp_single_channel_pk_payload(ctxp, &payload_flistp, error_codep, &act, ebufp);
          }
          else if ( *action_flag ==  CATV_FP_ALL_CHANNELS_SINGLE_STB_PK )
          {
              prepare_catv_fp_all_channnels_single_stb_pk_payload(ctxp, &payload_flistp, error_codep, &act, ebufp);
          }
          else if ( *action_flag ==  CATV_FP_ALL_STBS_PK )
          {
              prepare_catv_fp_all_stb_pk_payload(ctxp, &payload_flistp, error_codep, &act, ebufp);
          }
		  else if ( *action_flag ==  CATV_FP_GOSPEL )
          {
              prepare_catv_fp_gospel_payload(ctxp, &payload_flistp, error_codep, &act, ebufp);
          }
          else if ( *action_flag ==  CATV_RESEND_ZIPCODE )
          {
              prepare_catv_resend_zipcode_payload(ctxp, &payload_flistp, error_codep, &act, ebufp);
          }
          else if ( *action_flag ==  CATV_CHANGE_BOUQUET_ID )
          {
              prepare_catv_change_bouquet_id_payload(ctxp, &payload_flistp, &act, ebufp);
          }
          else if ( *action_flag ==  CATV_SET_BIT )
          {
              prepare_catv_set_bit_payload(ctxp, &payload_flistp, &act, ebufp);
          }
          else if ( *action_flag == CATV_UNSET_BIT )
          {
                prepare_catv_unset_bit_payload(ctxp, &payload_flistp, &act, ebufp);
          }
          else if ( *action_flag ==  CATV_SEND_OSD )
          {
              prepare_catv_send_osd_payload(ctxp, &payload_flistp, error_codep, &act, ebufp);
          }
          else if ( *action_flag ==  CATV_RESET_PIN_PK )
          {
              prepare_catv_reset_pin_pk_payload(ctxp, &payload_flistp, error_codep, &act, ebufp);
          }
           /* Sachid: new function to create action for modifying preactivated services */
           else if ( *action_flag ==  CATV_PREACTV_MOD_SVC )
           {
                prepare_catv_preactv_svc_mod_payload(ctxp, &payload_flistp, error_codep, &act, ebufp);
            }
          else
          {
              //Invalid flag : set error
              *error_codep = 52108;
              pin_set_err(ebufp, PIN_ERRLOC_FM,
                              PIN_ERRCLASS_SYSTEM_DETERMINATE,
                              PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
                 PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                              "Invalid Action code", ebufp);
              return;
          }

          /*
           * Return if validation and enrichment returns error
           */
          if (PIN_ERRBUF_IS_ERR(ebufp))
          {
              goto CLEANUP;
          }
	 /*NT - Start of restrict order id generation if all CA_ID are NA*/
         nw_node = PIN_FLIST_FLD_GET(payload_flistp, MSO_FLD_NETWORK_NODE, 1, ebufp);
          if ( nw_node && strstr(nw_node,"CISCO") )
          {
          	task_elem_id = 2;
          }
	switch (*action_flag){
	case CATV_ACTIVATION   :
				if ( nw_node && strstr(nw_node,"NDS") || strstr(nw_node,"VM"))
				{
					task_elem_id = 3;
				}
				else if ( nw_node && strstr(nw_node,"GOSPEL"))
				{
					task_elem_id = 4;
				}
				else if ( nw_node && strstr(nw_node,"NAGRA"))
				{
					task_elem_id = 7;
					check_addpack_flistp = PIN_FLIST_ELEM_GET(payload_flistp, MSO_FLD_TASK, 8, 1, ebufp);
					if(check_addpack_flistp && PIN_FLIST_ELEM_COUNT(check_addpack_flistp, PIN_FLD_OFFERINGS, ebufp) == 0)
					{
						PIN_FLIST_ELEM_DROP(payload_flistp, MSO_FLD_TASK, 8, ebufp);
					}
				}
				check_addpack_flistp = PIN_FLIST_ELEM_GET(payload_flistp, MSO_FLD_TASK, task_elem_id, 1, ebufp);
				if(check_addpack_flistp && PIN_FLIST_ELEM_COUNT(check_addpack_flistp, MSO_FLD_ADD_PACKAGE_INFO , ebufp) == 0)
				{
					PIN_FLIST_ELEM_DROP(payload_flistp, MSO_FLD_TASK, task_elem_id, ebufp);
				}

				break;	
	case CATV_CHANGE_STB  :
                check_addpack_flistp = PIN_FLIST_ELEM_GET(payload_flistp, MSO_FLD_TASK, 3, 1, ebufp);
                if (check_addpack_flistp && PIN_FLIST_ELEM_COUNT(check_addpack_flistp, MSO_FLD_ADD_PACKAGE_INFO , ebufp) == 0)
                {
                    PIN_FLIST_ELEM_DROP(payload_flistp, MSO_FLD_TASK, 3, ebufp);
                    PIN_ERR_LOG_MSG(3, "Change STB action no prov creation due to all CA_ID's are NA");

                    check_addpack_flistp = PIN_FLIST_ELEM_GET(payload_flistp, MSO_FLD_TASK, 4, 1, ebufp);
                    if (check_addpack_flistp)
                    {
                        task_namep = PIN_FLIST_FLD_GET(check_addpack_flistp, MSO_FLD_TASK_NAME, 1, ebufp);
                        if (task_namep && strcmp(task_namep, "AUTHORIZE_LBO_SERVICES") == 0)
                        {
                            PIN_FLIST_ELEM_DROP(payload_flistp, MSO_FLD_TASK, 4, ebufp);
                            PIN_ERR_LOG_MSG(3, "Change STB action Droping LBO SERVICES to all CA_ID'S ARE NA");
                        }
                    }
                    out_flag = 0;
                 }

                break;
    case CATV_ADD_PACKAGE  :
				check_addpack_flistp = PIN_FLIST_ELEM_GET(payload_flistp, MSO_FLD_TASK, 1, 1, ebufp);
				if(check_addpack_flistp && PIN_FLIST_ELEM_COUNT(check_addpack_flistp, MSO_FLD_ADD_PACKAGE_INFO , ebufp) == 0)
				{
					PIN_FLIST_ELEM_DROP(payload_flistp, MSO_FLD_TASK, 1, ebufp);
					PIN_ERR_LOG_MSG(3, "Add plan action no prov creation due to all CA_ID's are NA");
					out_flag = 1;
				}

				check_addpack_flistp = PIN_FLIST_ELEM_GET(payload_flistp, MSO_FLD_TASK, 2, 1, ebufp);
				if (check_addpack_flistp && PIN_FLIST_ELEM_COUNT(check_addpack_flistp, PIN_FLD_SERVICES, ebufp) > 0)
				{
					PIN_ERR_LOG_MSG(3, "Add plan action with NCF prov creation");
					out_flag = 0;
				}
				else if (check_addpack_flistp && PIN_FLIST_ELEM_COUNT(check_addpack_flistp, PIN_FLD_OFFERINGS, ebufp) > 0)
				{
					PIN_ERR_LOG_MSG(3, "Add plan action with ALC prov creation");
					out_flag = 0;
				}
				else if (check_addpack_flistp && PIN_FLIST_COUNT(check_addpack_flistp, ebufp) > 2 && 
                        nw_node && strstr(nw_node, "VM"))
                {
					PIN_ERR_LOG_MSG(3, "Add plan action with NCF prov creation for VM");
					out_flag = 0;
                }
				else if (check_addpack_flistp)
				{
					PIN_FLIST_ELEM_DROP(payload_flistp, MSO_FLD_TASK, 2, ebufp);
				}
				else
				{}
				break;
	case CATV_CANCEL_PLAN  :
				check_delpack_flistp = PIN_FLIST_ELEM_GET(payload_flistp, MSO_FLD_TASK, 1, 1, ebufp);
				if(check_delpack_flistp && PIN_FLIST_ELEM_COUNT(check_delpack_flistp, MSO_FLD_DEL_PACKAGE_INFO, ebufp) == 0)
				{
					PIN_ERR_LOG_MSG(3, "Cancel plan action no prov creation due to all CA_ID's are NA");
					PIN_FLIST_ELEM_DROP(payload_flistp, MSO_FLD_TASK, 1, ebufp);
					out_flag = 1;
				}		
				check_delpack_flistp = PIN_FLIST_ELEM_GET(payload_flistp, MSO_FLD_TASK, 2, 1, ebufp);
				if (check_delpack_flistp && PIN_FLIST_ELEM_COUNT(check_delpack_flistp, PIN_FLD_DUNNING_INFO, ebufp) > 0)
				{
					PIN_ERR_LOG_MSG(3, "Cancel plan action with ALC prov creation");
					out_flag = 0;
				}
				else if (check_delpack_flistp && PIN_FLIST_ELEM_COUNT(check_delpack_flistp, PIN_FLD_DUNNING_INFO, ebufp) == 0)
				{
					PIN_FLIST_ELEM_DROP(payload_flistp, MSO_FLD_TASK, 2, ebufp);
				}
				break;
	case CATV_PLAN_CHANGE  :
				check_addpack_flistp = PIN_FLIST_ELEM_GET(payload_flistp, MSO_FLD_TASK, 1, 1, ebufp);
				check_delpack_flistp = PIN_FLIST_ELEM_GET(payload_flistp, MSO_FLD_TASK, 2, 1, ebufp);
				if(check_addpack_flistp && (PIN_FLIST_ELEM_COUNT(check_addpack_flistp, MSO_FLD_ADD_PACKAGE_INFO , ebufp) == 0) &&
				check_delpack_flistp && (PIN_FLIST_ELEM_COUNT(check_delpack_flistp, MSO_FLD_DEL_PACKAGE_INFO, ebufp) == 0))
				{
					PIN_ERR_LOG_MSG(3, "Change plan action no prov creation due to all CA_ID's are NA");
					out_flag = 1;
				}
				else if (check_addpack_flistp == NULL && check_delpack_flistp == NULL)
				{
                    check_addpack_flistp = PIN_FLIST_ELEM_GET(payload_flistp, MSO_FLD_TASK, 3, 1, ebufp);
                    if (check_addpack_flistp)
                    {
                        task_namep = PIN_FLIST_FLD_GET(check_addpack_flistp, MSO_FLD_TASK_NAME, 1, ebufp);
                        if (task_namep && strcmp(task_namep, "AUTHORIZE_LBO_SERVICES") == 0)
                        {
                            PIN_ERR_LOG_MSG(3, "Change PLAN with plan transition");
                        }
                    }
                    else
                    {
					    PIN_ERR_LOG_MSG(3, "Change plan action no prov creation due to all CA_ID's are same");
					    out_flag = 1;
                    }
				}

				if ( nw_node && strstr(nw_node,"NAGRA"))
				{
					check_addpack_flistp = PIN_FLIST_ELEM_GET(payload_flistp, MSO_FLD_TASK, 3, 1, ebufp);
					if (check_addpack_flistp && PIN_FLIST_ELEM_COUNT(check_addpack_flistp, PIN_FLD_OFFERINGS, ebufp) > 0)
					{
						PIN_ERR_LOG_MSG(3, "Change plan action with ALC prov creation");
						out_flag = 0;
					}
					else if (check_addpack_flistp && PIN_FLIST_ELEM_COUNT(check_addpack_flistp, PIN_FLD_OFFERINGS, ebufp) == 0)
					{
						PIN_FLIST_ELEM_DROP(payload_flistp, MSO_FLD_TASK, 3, ebufp);
					}
					check_delpack_flistp = PIN_FLIST_ELEM_GET(payload_flistp, MSO_FLD_TASK, 4, 1, ebufp);
					if (check_delpack_flistp && PIN_FLIST_ELEM_COUNT(check_delpack_flistp, PIN_FLD_DUNNING_INFO, ebufp) > 0)
					{
						PIN_ERR_LOG_MSG(3, "Change plan action with ALC prov creation");
						out_flag = 0;
					}
					else if (check_delpack_flistp && PIN_FLIST_ELEM_COUNT(check_delpack_flistp, PIN_FLD_DUNNING_INFO, ebufp) == 0)
					{
						PIN_FLIST_ELEM_DROP(payload_flistp, MSO_FLD_TASK, 4, ebufp);
					}
				}
				break;
        case CATV_SUSPEND      :
                    if ( nw_node && (strstr(nw_node,"CISCO") || strstr(nw_node,"JVM")))
                    {
                        if(PIN_FLIST_ELEM_COUNT(payload_flistp, MSO_FLD_TASK, ebufp) == 0)
                        {
                            PIN_ERR_LOG_MSG(3, "Suspend plan action no prov creation due to all CA_ID's are NA or no channels ");
                            out_flag = 2;
                        }
                        if (strstr(nw_node, "CISCO"))
                        {
                           check_delpack_flistp = PIN_FLIST_ELEM_GET(payload_flistp, MSO_FLD_TASK, 1, 1, ebufp);		                                    if(check_delpack_flistp && PIN_FLIST_ELEM_COUNT(check_delpack_flistp, MSO_FLD_DEL_PACKAGE_INFO, ebufp) == 0)
					        {
						        PIN_FLIST_ELEM_DROP(payload_flistp, MSO_FLD_TASK, 1, ebufp);
					        }
                        }
                        else if (strstr(nw_node, "JVM"))
                        {
                            check_delpack_flistp = PIN_FLIST_ELEM_GET(payload_flistp, MSO_FLD_TASK, 3, 1, ebufp);
                            if (check_delpack_flistp == NULL)
                            {
                                out_flag = 1;
                            }
					        if(check_delpack_flistp && PIN_FLIST_ELEM_COUNT(check_delpack_flistp, MSO_FLD_DEL_PACKAGE_INFO, ebufp) == 0)
					        {
						        PIN_FLIST_ELEM_DROP(payload_flistp, MSO_FLD_TASK, 3, ebufp);
                                out_flag = 1;
					        }
                        }
                     }
                    break;

        case CATV_REACTIVATE   :
                    if ( nw_node && (strstr(nw_node,"CISCO") || strstr(nw_node,"JVM")))
                    {
                        if(PIN_FLIST_ELEM_COUNT(payload_flistp, MSO_FLD_TASK, ebufp) == 0)
                        {
                            PIN_ERR_LOG_MSG(3, "Reactivate plan action no prov creation due to all CA_ID's are NA or no channels ");
                            out_flag = 2;
                        }
                        if (strstr(nw_node, "CISCO"))
                        {
					        check_addpack_flistp = PIN_FLIST_ELEM_GET(payload_flistp, MSO_FLD_TASK, 1, 1, ebufp);
                        }
                        else if (strstr(nw_node, "JVM"))
                        {
                            check_addpack_flistp = PIN_FLIST_ELEM_GET(payload_flistp, MSO_FLD_TASK, 2, 1, ebufp);
                            if (check_addpack_flistp == NULL)
                            {
                                out_flag = 1;
                            }
                        }
					    if(strstr(nw_node, "CISCO") && check_addpack_flistp && PIN_FLIST_ELEM_COUNT(check_addpack_flistp, MSO_FLD_ADD_PACKAGE_INFO , ebufp) == 0)
					    {
						    PIN_FLIST_ELEM_DROP(payload_flistp, MSO_FLD_TASK, 1, ebufp);
					    }
                        else if (strstr(nw_node, "JVM") && check_addpack_flistp && PIN_FLIST_ELEM_COUNT(check_addpack_flistp, MSO_FLD_ADD_PACKAGE_INFO , ebufp) == 0)
                        {
                            out_flag = 1;
                        }
                    }
				else if ( nw_node && strstr(nw_node, "NAGRA") )
				{
					check_addpack_flistp = PIN_FLIST_ELEM_GET(payload_flistp, MSO_FLD_TASK, 2, 1, ebufp);
					if (check_addpack_flistp && PIN_FLIST_ELEM_COUNT(check_addpack_flistp, PIN_FLD_OFFERINGS, ebufp) == 0)
					{	
						PIN_FLIST_ELEM_DROP(payload_flistp, MSO_FLD_TASK, 2, ebufp);
					}
				}				
                break;

	case CATV_TERMINATE    :
				if ( nw_node && strstr(nw_node,"NDS") || strstr(nw_node,"VM") || strstr(nw_node,"NAGRA"))
				{
					break;
				}
				else if (nw_node && strstr(nw_node,"GOSPEL"))
				{
					check_delpack_flistp = PIN_FLIST_ELEM_GET(payload_flistp, MSO_FLD_TASK, 1, 1, ebufp);
					if (check_delpack_flistp && PIN_FLIST_ELEM_COUNT(check_delpack_flistp, MSO_FLD_DEL_PACKAGE_INFO, ebufp) == 0)
					{
						PIN_FLIST_ELEM_DROP(payload_flistp, MSO_FLD_TASK, 1, ebufp);
						break;
					}
				}
				else
				{
					check_delpack_flistp = PIN_FLIST_ELEM_GET(payload_flistp, MSO_FLD_TASK, 2, 1, ebufp);
				}
				
				if(check_delpack_flistp && PIN_FLIST_ELEM_COUNT(check_delpack_flistp, MSO_FLD_DEL_PACKAGE_INFO, ebufp) == 0)
				{
					PIN_ERR_LOG_MSG(3, "Terminate action no prov creation due to all CA_ID's are NA");
					out_flag = 1;				
				}				
				break;

	case CATV_RETRACK	:
				if (nw_node && strstr(nw_node,"NAGRA"))
				{
					check_addpack_flistp = PIN_FLIST_ELEM_GET(payload_flistp, MSO_FLD_TASK, 6, 1, ebufp);
					if (check_addpack_flistp && PIN_FLIST_ELEM_COUNT(check_addpack_flistp, PIN_FLD_OFFERINGS, ebufp) == 0)
					{	
						PIN_FLIST_ELEM_DROP(payload_flistp, MSO_FLD_TASK, 6, ebufp);
					}
				}
				break;
	}
	if (out_flag == 1)
	{
		*out_flistpp = PIN_FLIST_COPY(payload_flistp, ebufp);
		goto CLEANUP;
	}
        vp = PIN_FLIST_FLD_GET(payload_flistp, MSO_FLD_STPK_PLAN_OBJ, 1, ebufp);
        if (vp)
        {
                PIN_FLIST_FLD_DROP(payload_flistp, MSO_FLD_STPK_PLAN_OBJ, ebufp);
        }

	if (strcmp(nw_node, "CISCO1") == 0)
	{
		stbp = PIN_FLIST_FLD_TAKE(payload_flistp, MSO_FLD_STB_ID, 1, ebufp);
		if (stbp && !strstr(stbp, ":"))
		{
			insert_char(stbp, ':', new_stbp);
			PIN_ERR_LOG_MSG(3, stbp);
			PIN_ERR_LOG_MSG(3, new_stbp);
			PIN_FLIST_FLD_SET(payload_flistp, MSO_FLD_STB_ID, new_stbp, ebufp);
		}
		else if (stbp)
		{
			PIN_FLIST_FLD_PUT(payload_flistp, MSO_FLD_STB_ID, stbp, ebufp);
		}
		else
		{
		}	

		while ((tmp_flistp = PIN_FLIST_ELEM_GET_NEXT(payload_flistp, MSO_FLD_TASK,
                        &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
                {
			stbp = PIN_FLIST_FLD_TAKE(tmp_flistp, MSO_FLD_STB_ID, 1, ebufp);
			if (stbp && !strstr(stbp, ":"))
			{
				tbpnsert_char(stbp, ':', new_stbp);
				PIN_FLIST_FLD_PUT(tmp_flistp, MSO_FLD_STB_ID, new_stbp, ebufp);
			}
			else if (stbp)
			{
				PIN_FLIST_FLD_PUT(tmp_flistp, MSO_FLD_STB_ID, stbp, ebufp);
			}	
			else
			{
			}
		}
	}
         /*NT - End of restrict order id generation if all CA_ID are NA*/
		
          //task_flist3 = PIN_FLIST_ELEM_GET(payload_flistp, MSO_FLD_TASK, PIN_ELEMID_ANY,1, ebufp);
          /*if(task_flist3 != NULL)
          {
              count = PIN_FLIST_ELEM_COUNT(payload_flistp,MSO_FLD_TASK, ebufp);
          }

          if (count == 0)
          {
              PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " No task to sent for provisioning ");
          }
          else
          {*/
          /*
           * Get Order Id and set it in payload
           */
          db_no = PIN_POID_GET_DB(service_poidp);
          order_seq_i_flistp = PIN_FLIST_CREATE(ebufp);
          PIN_FLIST_FLD_SET(order_seq_i_flistp, PIN_FLD_POID, service_poidp, ebufp);
          PIN_FLIST_FLD_SET(order_seq_i_flistp, PIN_FLD_NAME, "MSO_PROV_SEQUENCE", ebufp);
          //call the function to read the provisioning order sequence
          fm_mso_utils_sequence_no(ctxp, order_seq_i_flistp, &order_seq_o_flistp, ebufp);

          if (PIN_ERRBUF_IS_ERR(ebufp))
          {
              *error_codep = 52110;
              goto CLEANUP;
          }

          PIN_FLIST_FLD_COPY(order_seq_o_flistp, PIN_FLD_STRING, payload_flistp, PIN_FLD_ORDER_ID, ebufp);
          PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_ACTION, act, ebufp);
          PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_prov create output flist payload_flistp", payload_flistp);
          /*
           * Convert flist to Buffer. it will be used by Provising system.
           */
          PIN_FLIST_TO_STR(payload_flistp,&flistbuf,&flistlen,ebufp );

          if (PIN_ERRBUF_IS_ERR(ebufp))
          {
              *error_codep = 52110;
              goto CLEANUP;
          }

          /*
           * Create flist for order creation
           */
          order_i_flistp = PIN_FLIST_CREATE(ebufp);
          objp = PIN_POID_CREATE(db_no, "/mso_prov_order", -1, ebufp);
          PIN_FLIST_FLD_SET(order_i_flistp, PIN_FLD_POID, objp, ebufp);
          if ( *action_flag ==  CATV_PREACTIVATION )
          {
              //PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_OBJ, order_i_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
              PIN_FLIST_FLD_COPY(payload_flistp, PIN_FLD_ACCOUNT_OBJ, order_i_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
              PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, order_i_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
          }
          else
          {   
                if (PIN_POID_GET_ID(acc_poidp) > 1 )
                {
			PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, order_i_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
			PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SERVICE_OBJ, order_i_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
		}
		else
		{
			PIN_FLIST_FLD_COPY(payload_flistp, PIN_FLD_POID, order_i_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
			PIN_FLIST_FLD_COPY(payload_flistp, PIN_FLD_SERVICE_OBJ, order_i_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
		}
          }

          PIN_FLIST_FLD_COPY(order_seq_o_flistp, PIN_FLD_STRING, order_i_flistp, PIN_FLD_ORDER_ID, ebufp);
	  if (out_flag == 2)
          {
		PIN_FLIST_FLD_SET(order_i_flistp, PIN_FLD_STATUS, 0, ebufp); //PROVISIONING
	  }
	  else
	  {
          	PIN_FLIST_FLD_SET(order_i_flistp, PIN_FLD_STATUS, &status, ebufp); //PROVISIONING
	  }
          PIN_FLIST_FLD_SET(order_i_flistp, PIN_FLD_ACTION, act, ebufp);

	sub_typep = PIN_FLIST_FLD_GET(payload_flistp, PIN_FLD_TYPE_OF_SERVICE, 1, ebufp);
	if (sub_typep && *sub_typep == 48)
	{
		dvbip = 1;
		PIN_FLIST_FLD_DROP(payload_flistp, PIN_FLD_TYPE_OF_SERVICE, ebufp);
	}

        /*Chandrakala -- Start -changes done for adding network type*/
          if (strcmp(nw_node, "NDS") == 0)
          {
                network_type = 1;
          }
          else if(strcmp(nw_node, "CISCO") == 0)
          {
                network_type = 2;
          }
          else if (nw_node && strcmp(nw_node, "VM") == 0)
          {
                network_type = 3;
          }
	  else if (nw_node && strcmp(nw_node, "JVM") == 0 && dvbip == 0)
	  {
		network_type = 4;
	  }
	  else if (nw_node && strcmp(nw_node, "NDS1") == 0)
	  {
		network_type = 5;
	  }
	  else if (nw_node && strcmp(nw_node, "CISCO1") == 0)
	  {
		network_type = 6;
	  }
	  else if (dvbip == 0 && (nw_node && strcmp(nw_node, "JVM1") == 0))
	  {
		network_type = 7;
	  }
	  else if (nw_node && strcmp(nw_node, "GOSPELL") == 0)
	  {
		network_type = 8;
	  }
	  else if (nw_node && strcmp(nw_node, "NAGRA") == 0)
	  {
		network_type = 9;
	  }
	  else if (dvbip)
	  {
		network_type = 10;
	  }
	  else if (nw_node && (strcmp(nw_node, "NDS2") == 0))
	  {
		network_type = 11;
	  }
	  else if (nw_node && (strcmp(nw_node, "JVM2") == 0))
	  {
		network_type = 12;
	  }
	  else if (nw_node && (strcmp(nw_node, "GOSPELL2") == 0 ))
	  {
		network_type = 13;
	  }
          else {
                network_type = 14;
          }
          PIN_FLIST_FLD_SET(order_i_flistp, PIN_FLD_TYPE_OF_SERVICE, &network_type, ebufp);
        /*Chandrakala -- Start -changes done for adding network type*/

          /*Set USER_ID, PROGRAM_NAME at level0 of /mso_prov_order object*/
          PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_USERID, order_i_flistp, PIN_FLD_USERID, ebufp);
          PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, order_i_flistp, PIN_FLD_PROGRAM_NAME, ebufp);

                // Getting the Order Type
          /*prg_name = PIN_FLIST_FLD_GET(in_flistp,PIN_FLD_PROGRAM_NAME, 1, ebufp);
          //if(strstr(PIN_FLIST_FLD_GET(in_flistp,PIN_FLD_PROGRAM_NAME, 1, ebufp),"BULK"))
          if(prg_name && strstr(prg_name,"BULK"))
          {
                          ord_type = 1;
          }
          else
          {
                          ord_type = 0;
          }
                  // Getting Rank only when the action is valid. For Device Type "NONE", there wont be any Prov action.
                  if (act)
                  {
                          get_mso_prov_priority(ctxp, act, &ord_type, out_flistpp, ebufp);
                  }

//                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "get_mso_prov_priority output flist", *out_flistpp);
                  if (*out_flistpp)
                  {
                          rank = PIN_FLIST_FLD_GET(*out_flistpp, PIN_FLD_RANK, 1, ebufp);
                  }
                  PIN_FLIST_FLD_SET(order_i_flistp, PIN_FLD_RANK, rank, ebufp); */

          nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
          if ( nota_buf == NULL )
          {
              *error_codep = 52111;
              pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
              PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate nemory for nota_buf", ebufp );
              goto CLEANUP;
          }

          nota_buf->flag   = 0;
          nota_buf->size   = flistlen - 2;
          nota_buf->offset = 0;
          nota_buf->data   = ( caddr_t)flistbuf;
          nota_buf->xbuf_file = ( char *) NULL;

          PIN_FLIST_FLD_PUT( order_i_flistp, PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );

          PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
              "fm_mso_prov_create_catv_action create input flist", order_i_flistp);
          if (PIN_ERRBUF_IS_ERR(ebufp))
          {
              *error_codep = 52112;
              goto CLEANUP;
          }
          PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, order_i_flistp, out_flistpp, ebufp);
          if(PIN_ERRBUF_IS_ERR(ebufp) && (ebufp)->pin_err == PIN_ERR_STORAGE) {
                PIN_ERRBUF_CLEAR(ebufp);
                PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, order_i_flistp, out_flistpp, ebufp);
          }

          PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
              "fm_mso_prov_create_catv_action order create output flist", *out_flistpp);
          PIN_FLIST_FLD_COPY(order_i_flistp, PIN_FLD_ORDER_ID, *out_flistpp, PIN_FLD_ORDER_ID, ebufp);

          if (PIN_ERRBUF_IS_ERR(ebufp))
          {
              *error_codep = 52112;
              goto CLEANUP;
          }

        /*  if (act)
          {
              free(act);
          }*/
 //}
        //else
        //{
                if (PIN_ERRBUF_IS_ERR(ebufp)) {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "fm_mso_prov_create_catv_action error: Error", ebufp);
                        goto CLEANUP;
                }
                tmp2_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, tmp2_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SERVICE_OBJ, tmp2_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
                PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_FLAGS, tmp2_flistp, PIN_FLD_FLAGS, ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp)) {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "fm_mso_prov_create_catv_action error: Error", ebufp);
                        goto CLEANUP;
                }
                *out_flistpp = PIN_FLIST_COPY(tmp2_flistp, ebufp);
                PIN_FLIST_FLD_COPY(order_i_flistp, PIN_FLD_ORDER_ID, *out_flistpp, PIN_FLD_ORDER_ID, ebufp);
        //}
 //}
CLEANUP:
          if (act)
          {
              free(act);
          }

    PIN_FLIST_DESTROY_EX(&payload_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&order_seq_i_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&order_seq_o_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&order_i_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&rfld_iflistp, NULL);
    PIN_FLIST_DESTROY_EX(&rfld_oflistp, NULL);
    PIN_FLIST_DESTROY_EX(&tmp2_flistp, NULL);
    return;
}

/*******************************************************************
 * prepare_catv_activation_flist()
 *
 * Enrich input filst with service details required
 * optionally performa validation on input fields
 *******************************************************************/
static void
prepare_catv_activation_flist(
    pcm_context_t       *ctxp,
    pin_flist_t         **i_flistpp,
    char                **act,
    pin_errbuf_t        *ebufp)
{

    pin_flist_t         *si_flistp = NULL;
    pin_flist_t         *so_flistp = NULL;
    pin_flist_t         *tmp_flistp = NULL;
    pin_flist_t         *sub_flistp = NULL;
    pin_flist_t         *task_flistp = NULL;
	pin_flist_t         *task_flistp2 = NULL;
    pin_flist_t         *result_flistp = NULL;
    pin_flist_t		*i_flistp = NULL;
    pin_flist_t		*write_iflistp = NULL;
    pin_flist_t		*write_oflistp = NULL;
    pin_flist_t		*r_flistp = NULL;
    poid_t		*srch_pdp = NULL;
    int32               srch_flag = SRCH_EXACT;
    void                *vp = NULL;
    char                *population_id = "0001"; //this is the only expected value in downstream system
    char		*pop_idp = NULL;
    char		*area_codep = NULL;
    char		*vip = "0";
    char                *ne = NULL;
    char                *admin_status = "DAS_InServiceOneWay";
    char                *bouquet_id = NULL;
    char                *region_key = NULL;
    char                *vc_id = NULL;
    char                *device_type = NULL;
    char                *area_id = NULL;
    int32		*mso_device_typep = NULL;
    int32		device_hd_flag = 0; 
    int32               *state_id = NULL;
	char				*vm_id = NULL;
	
	// VERIMATRIX Declarations
	pin_flist_t			*arg_flist = NULL;
	pin_flist_t			*tempp_flistp = NULL;
	pin_flist_t			*temp_flistp = NULL;
	pin_flist_t			*in_flistp = NULL;
	pin_flist_t			*ret_flistp	= NULL;
	pin_flist_t			*return_flistp = NULL;
	pin_flist_t			*input_flistp = NULL;
	pin_flist_t			*name_flistp = NULL;
	
    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);


    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_activation_flist input flist", *i_flistpp);

    /*
     * Validate input received
     */

    /*
     * Enrich with additional service details
     */
    si_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_activation_flist PCM_OP_READ_OBJ input flist", si_flistp);
    PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_activation_flist PCM_OP_READ_OBJ output flist", so_flistp);

    tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
    ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);
    bouquet_id = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_BOUQUET_ID, 0, ebufp);
    PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);
	
    if(strcmp(ne, "NONE") == 0)
    {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "No need to generate the prov request ");
    }
    else
    {

            if ( ne && (strstr(ne,"NDS") || strstr(ne,"GOSPEL")))
            {

		if (strstr(ne,"NDS"))
		{
                	*act = (char *) malloc(sizeof("ACTIVATE_SUBSCRIBER_NDS"));
                	strcpy(*act,"ACTIVATE_SUBSCRIBER_NDS");
		}
		else
		{
                	*act = (char *) malloc(sizeof("ACTIVATE_SUBSCRIBER_GOSPEL"));
                	strcpy(*act,"ACTIVATE_SUBSCRIBER_GOSPEL");
		}
               	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );

                    /***************** TASK 1 **********************/
                task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
		if (strstr(ne, "NDS"))
		{
                	PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "CREATE_SUBSCRIBER", ebufp);
		}
		else
		{
                	PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "VALIDATE_CARD", ebufp);
                	PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_VIP_CODE, vip, ebufp);
		}

                        //add region_key
                region_key = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_REGION_KEY, 0, ebufp);

                if ( region_key && strlen(region_key) > 0)
                {
                    PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_REGION_KEY, region_key, ebufp);
                }
                else
                {
                    pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                            PIN_ERR_BAD_VALUE, MSO_FLD_REGION_KEY, 0, 0);
                    goto CLEANUP;
                }

                        // add stb id
                tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
		if (strstr(ne, "NDS"))
		{
                	PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_NAME, task_flistp, MSO_FLD_STB_ID, ebufp);
		}
		else
		{
                	PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_NAME, task_flistp, MSO_FLD_VC_ID, ebufp);
		}

		if (strstr(ne, "NDS"))
		{
                        // add vc id
                	tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 1, 0, ebufp);
               		PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_NAME, task_flistp, MSO_FLD_VC_ID, ebufp);
		}
               	vc_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 1, ebufp);

               	fm_mso_get_device_details(ctxp,i_flistpp,vc_id,ebufp);

                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "prepare_catv_activation_flist device details return", *i_flistpp);

                device_type = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_DEVICE_TYPE, 0, ebufp);
                state_id = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_STATE_ID, 0, ebufp);
                PIN_FLIST_FLD_SET (task_flistp, PIN_FLD_DEVICE_TYPE, device_type, ebufp);
		if (strstr(ne, "GOSPELL"))
		{
                	PIN_FLIST_FLD_COPY(*i_flistpp, MSO_FLD_SERIAL_NO, task_flistp, MSO_FLD_STB_ID, ebufp);
		}
                PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_DEVICE_TYPE, ebufp);
                PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_STATE_ID, ebufp);
                       // add device_type for vc
                //tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_DEVICES, 1, 1, ebufp);
                /*tmp_flistp = PIN_FLIST_ELEM_GET(*i_flistpp, PIN_FLD_DEVICES, 1, 1, ebufp);
                        if(tmp_flistp != NULL)
                PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_DEVICE_TYPE, task_flistp, PIN_FLD_DEVICE_TYPE, ebufp);*/


                tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "prepare_catv_activation_flist catv info flist",tmp_flistp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"error in getting device details",ebufp);
                        goto CLEANUP;
                }

                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, bouquet_id);

		mso_device_typep = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_DEVICE_TYPE, 1, ebufp);
		if (mso_device_typep)
		{
			device_hd_flag = *mso_device_typep;
		}

		i_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(so_flistp, PIN_FLD_ACCOUNT_OBJ, i_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(so_flistp, PIN_FLD_ACCOUNT_OBJ, i_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(*i_flistpp, MSO_FLD_LCO_OBJ, i_flistp, MSO_FLD_LCO_OBJ, ebufp);
		PIN_FLIST_FLD_SET(i_flistp, MSO_FLD_NETWORK_NODE, ne, ebufp);

		search_name_info(ctxp, i_flistp, &return_flistp, ebufp);

		tmp_flistp = PIN_FLIST_ELEM_GET(return_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
		PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_AREA, i_flistp, MSO_FLD_AREA, ebufp);
		PIN_FLIST_ELEM_SET(i_flistp, so_flistp, PIN_FLD_SERVICES, 0, ebufp);

		fm_mso_populate_bouquet_id(ctxp, i_flistp, device_hd_flag, 0, &r_flistp, ebufp);

                PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_LCO_OBJ, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "prepare_catv_activation_flist bouquet flist", i_flistp);

                    /***************** NDS TASK 2 **********************/
		if (strstr(ne, "NDS"))
		{
                	task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 2, ebufp);
                	PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "SET_BOUQUET_ID", ebufp);
		}

		pop_idp = PIN_FLIST_FLD_TAKE(i_flistp, MSO_FLD_POPULATION_ID, 1, ebufp);
		if (pop_idp)
		{
			PIN_FLIST_FLD_PUT(task_flistp, MSO_FLD_POPULATION_ID, pop_idp, ebufp);
		}
		else
		{
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_POPULATION_ID, population_id, ebufp);
		}
	
		if (strstr(ne, "GOSPEL"))
		{	
			area_codep = PIN_FLIST_FLD_TAKE(r_flistp, PIN_FLD_SERVICE_AREA_CODE, 1, ebufp);
			if (area_codep)
			{
				PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_ROUTING_AREA, area_codep, ebufp);
			}
		}
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

                if ( bouquet_id && strlen(bouquet_id) > 0)
                {
                    PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_BOUQUET_ID, bouquet_id, ebufp);
                }
                else
                {
			tmp_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_SERVICES, 0, 0, ebufp);
			sub_flistp = PIN_FLIST_SUBSTR_GET(tmp_flistp, MSO_FLD_CATV_INFO, 0, ebufp); 
			bouquet_id = PIN_FLIST_FLD_TAKE(sub_flistp, MSO_FLD_BOUQUET_ID, 1, ebufp);
			PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
			if ( bouquet_id && strlen(bouquet_id) > 0)
			{
				write_iflistp = PIN_FLIST_CREATE(ebufp);
    				PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, write_iflistp, PIN_FLD_POID, ebufp);
				tmp_flistp = PIN_FLIST_SUBSTR_ADD(write_iflistp, MSO_FLD_CATV_INFO, ebufp);	
				PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_BOUQUET_ID, bouquet_id, ebufp);	

				PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_iflistp, &write_oflistp, ebufp);

				PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
				PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);

				PIN_FLIST_FLD_PUT(task_flistp, MSO_FLD_BOUQUET_ID, bouquet_id, ebufp);
			}
                	else
                	{
                    		pin_set_err(ebufp, PIN_ERRLOC_FM,
                       			PIN_ERRCLASS_SYSTEM_DETERMINATE,
                       			PIN_ERR_BAD_VALUE, MSO_FLD_BOUQUET_ID, 0, 0);
                    		goto CLEANUP;
                	}
                	PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_DEVICE_TYPE, ebufp);
                }

		if (strstr(ne, "GOSPEL"))
		{
                	task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 2, ebufp);
                	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "PAIR_STB_CARD", ebufp);
                
			tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
                	PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_NAME, task_flistp, MSO_FLD_VC_ID, ebufp);
	
                	PIN_FLIST_FLD_COPY(*i_flistpp, MSO_FLD_SERIAL_NO, task_flistp, MSO_FLD_STB_ID, ebufp);
                    
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_BOUQUET_ID, bouquet_id, ebufp);

                	task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 3, ebufp);
                	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "ACTIVATE_CARD", ebufp);
                	PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_NAME, task_flistp, MSO_FLD_VC_ID, ebufp);
                	PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_SERIAL_NO, ebufp);
		}

                add_package_detail(ctxp, i_flistpp, ADD_PACKAGE, ne, 0, ebufp);
            }
            else if (ne  && (strstr(ne,"CISCO") || strstr(ne, "NAGRA")))
            {
		if (strstr(ne, "CISCO"))
		{
                	*act = (char *) malloc(sizeof("ACTIVATE_SUBSCRIBER_PK"));
                	strcpy(*act, "ACTIVATE_SUBSCRIBER_PK");
		}
		else
		{
                	*act = (char *) malloc(sizeof("ACTIVATE_SUBSCRIBER_NGR"));
                	strcpy(*act, "ACTIVATE_SUBSCRIBER_NGR");
		}
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );

                    /***************** TASK 1 **********************/
                task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
		if (strstr(ne, "CISCO"))
		{
			//set mac address
                	tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 2, 0, ebufp);
                	PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_NAME, *i_flistpp, MSO_FLD_STB_ID, ebufp);
                    	/*
                      	 * set admin status - required for CISCO PK
                         * value is always DAS_InServiceOneWay
                         */
                	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "MODIFY_DHCT_ADMIN_STATUS", ebufp);
                	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_ADMIN_STATUS, admin_status, ebufp);
                    
			/***************** TASK 2 **********************/
                	task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 2, ebufp);
                	PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "MODIFY_DHCT_CONFIG", ebufp);
                	PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_DMS_ENABLED, "DMS_Enabled", ebufp);

                	// call function to add package info
               		add_package_detail(ctxp, i_flistpp, ADD_PACKAGE, ne, 0, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " i_flistpp after add package is ", *i_flistpp);

			/***************** TASK 3 **********************/
			task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 3, ebufp);
	                PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "INSTANT_HIT", ebufp);
		}
		else
		{
			//set mac address
                	tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
                	PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_NAME, *i_flistpp, MSO_FLD_STB_ID, ebufp);

                	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "ASSIGN_VIRTUALUA", ebufp);
                	PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_NAME, task_flistp, MSO_FLD_STB_ID, ebufp);
		}

                /***************** TASK 2 for Nagra and TASK 4 for Cisco**********************/
		if (strstr(ne, "NAGRA") || strcmp(ne, "CISCO1") == 0)
		{
			if (strstr(ne, "NAGRA"))
			{
                		task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 2, ebufp);
			}
			else
			{
                		task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 4, ebufp);
			}


                	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "SET_BOUQUET_ID", ebufp);

                	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, bouquet_id);

			mso_device_typep = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_DEVICE_TYPE, 1, ebufp);
			if (mso_device_typep)
			{
				device_hd_flag = *mso_device_typep;
			}

			i_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(so_flistp, PIN_FLD_ACCOUNT_OBJ, i_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(so_flistp, PIN_FLD_ACCOUNT_OBJ, i_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
			PIN_FLIST_FLD_COPY(*i_flistpp, MSO_FLD_LCO_OBJ, i_flistp, MSO_FLD_LCO_OBJ, ebufp);
			PIN_FLIST_FLD_SET(i_flistp, MSO_FLD_NETWORK_NODE, ne, ebufp);

			search_name_info(ctxp, i_flistp, &return_flistp, ebufp);

			tmp_flistp = PIN_FLIST_ELEM_GET(return_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
			PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_AREA, i_flistp, MSO_FLD_AREA, ebufp);
			name_flistp = PIN_FLIST_ELEM_GET(tmp_flistp,PIN_FLD_NAMEINFO,PIN_ELEMID_ANY,0,ebufp);
			area_codep = PIN_FLIST_FLD_GET(name_flistp, PIN_FLD_ZIP, 0, ebufp);
			PIN_FLIST_ELEM_SET(i_flistp, so_flistp, PIN_FLD_SERVICES, 0, ebufp);

			fm_mso_populate_bouquet_id(ctxp, i_flistp, device_hd_flag, 0, &r_flistp, ebufp);

                	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        	"prepare_catv_activation_flist bouquet flist", i_flistp);

                	if ( bouquet_id && strlen(bouquet_id) > 0)
                	{
				PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_BOUQUET_ID, bouquet_id, ebufp);
                	}
                	else
                	{
				tmp_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_SERVICES, 0, 0, ebufp);
				sub_flistp = PIN_FLIST_SUBSTR_GET(tmp_flistp, MSO_FLD_CATV_INFO, 0, ebufp); 
				bouquet_id = PIN_FLIST_FLD_TAKE(sub_flistp, MSO_FLD_BOUQUET_ID, 1, ebufp);
				PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
				if ( bouquet_id && strlen(bouquet_id) > 0)
				{
					write_iflistp = PIN_FLIST_CREATE(ebufp);
    					PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, write_iflistp, PIN_FLD_POID, ebufp);
					tmp_flistp = PIN_FLIST_SUBSTR_ADD(write_iflistp, MSO_FLD_CATV_INFO, ebufp);	
					PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_BOUQUET_ID, bouquet_id, ebufp);	

					PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_iflistp, &write_oflistp, ebufp);

					PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
					PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);

					PIN_FLIST_FLD_PUT(task_flistp, MSO_FLD_BOUQUET_ID, bouquet_id, ebufp);
				}
                		else
                		{
                    			pin_set_err(ebufp, PIN_ERRLOC_FM,
                       				PIN_ERRCLASS_SYSTEM_DETERMINATE,
                       				PIN_ERR_BAD_VALUE, MSO_FLD_BOUQUET_ID, 0, 0);
                    			goto CLEANUP;
                		}
                		PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_DEVICE_TYPE, ebufp);
                	}
		}

		if (strstr(ne, "NAGRA"))
		{
                	task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 3, ebufp);
                	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "SET_NETWORK", ebufp);

                	tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
                	vm_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 1,ebufp);
			fm_mso_get_device_details(ctxp,i_flistpp,vm_id,ebufp);

	                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        	                "prepare_catv_activation_flist device details return", *i_flistpp);

                	device_type = PIN_FLIST_FLD_TAKE(*i_flistpp, PIN_FLD_DEVICE_TYPE, 0, ebufp);
			PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_DEVICE_TYPE, device_type, ebufp);
                	PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_STATE_ID, ebufp);

			pop_idp = PIN_FLIST_FLD_TAKE(r_flistp, MSO_FLD_POPULATION_ID, 1, ebufp);
			if (pop_idp)
			{
				PIN_FLIST_FLD_PUT(task_flistp, MSO_FLD_POPULATION_ID, pop_idp, ebufp);
			}

			if (pop_idp == NULL)
			{
				pop_idp = PIN_FLIST_FLD_TAKE(i_flistp, MSO_FLD_POPULATION_ID, 1, ebufp);
				if (pop_idp)
				{
					PIN_FLIST_FLD_PUT(task_flistp, MSO_FLD_POPULATION_ID, pop_idp, ebufp);
				}
				else
				{
					PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_POPULATION_ID, population_id, ebufp);
				}
			}
		
			task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 4, ebufp);
                	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "INITIALIZE_ICC", ebufp);

			task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 5, ebufp);
                	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "SET_SEGMENT", ebufp);
			
                	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        	"prepare_catv_activation_flist bouquet return", r_flistp);

			task_flistp2 = PIN_FLIST_ELEM_ADD(task_flistp, PIN_FLD_CUSTOMER_SEGMENTS, 1, ebufp);
			PIN_FLIST_FLD_COPY(r_flistp, PIN_FLD_CATEGORY_ID, task_flistp2, PIN_FLD_CATEGORY_ID, ebufp);	
			PIN_FLIST_FLD_COPY(r_flistp, PIN_FLD_CATEGORY_VERSION, task_flistp2, PIN_FLD_CATEGORY_VERSION, ebufp);	
			task_flistp2 = PIN_FLIST_ELEM_ADD(task_flistp, PIN_FLD_CUSTOMER_SEGMENTS, 2, ebufp);
			PIN_FLIST_FLD_COPY(r_flistp, PIN_FLD_REASON_ID, task_flistp2, PIN_FLD_CATEGORY_ID, ebufp);	
			PIN_FLIST_FLD_COPY(r_flistp, PIN_FLD_REASON_DOMAIN_ID, task_flistp2, PIN_FLD_CATEGORY_VERSION, ebufp);	
			task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 6, ebufp);
                	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "SET_ZIPCODE", ebufp);
		 	PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ZIP, area_codep, ebufp);

			
                	// call function to add package info
               		add_package_detail(ctxp, i_flistpp, ADD_PACKAGE, ne, 0, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " i_flistpp after add package is ", *i_flistpp);
		}
		
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
            }
		/******************* VERIMATRIX CHANGES - BEGIN ***********************
		 * Creating provisioning order for activating Verimatrix accounts
		 **********************************************************************/
            else if ( ne && strstr(ne,"VM"))
            {
		if (strstr(ne, "JVM"))
		{
                	*act = (char *) malloc(sizeof("ACTIVATE_SUBSCRIBER_JVM"));
                	strcpy(*act,"ACTIVATE_SUBSCRIBER_JVM");
		}
		else
		{
                	*act = (char *) malloc(sizeof("ACTIVATE_SUBSCRIBER_VM"));
                	strcpy(*act,"ACTIVATE_SUBSCRIBER_VM");
		}
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "hp001");
				
		in_flistp  = PIN_FLIST_CREATE(ebufp);
	        tempp_flistp = PIN_FLIST_ELEM_GET(so_flistp,PIN_FLD_ALIAS_LIST,PIN_ELEMID_ANY, 1, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tempp_flistp", tempp_flistp);
	        PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, in_flistp, PIN_FLD_POID, ebufp);
	        PIN_FLIST_FLD_COPY(tempp_flistp, PIN_FLD_NAME, in_flistp, PIN_FLD_NAME, ebufp);
	            
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "in_flistp", in_flistp);
	            
	        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "hp002");
	            
	        search_serial_no(ctxp, in_flistp, &ret_flistp,ebufp);
	        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "hp003");
	        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                    "vm serial number inflistp", in_flistp);
	            
	        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                    "vm serial number search", ret_flistp);
	            	
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "so_flistp", so_flistp);
	        input_flistp = PIN_FLIST_CREATE(ebufp);
	        PIN_FLIST_FLD_COPY(so_flistp, PIN_FLD_ACCOUNT_OBJ, input_flistp, PIN_FLD_POID, ebufp);
	        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                    "nameinfo inflistp", input_flistp);

	        search_name_info(ctxp, input_flistp, &return_flistp,ebufp);
	            
	        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                    "nameinfo return", return_flistp);
                    /***************** TASK 1 **********************/
                task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
                PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "CREATE_SUBSCRIBER", ebufp);

                        //add region_key
                region_key = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_REGION_KEY, 0, ebufp);

                if ( region_key && strlen(region_key) > 0)
                {
                    PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_REGION_KEY, region_key, ebufp);
                }
                else
                {
                    pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                            PIN_ERR_BAD_VALUE, MSO_FLD_REGION_KEY, 0, 0);
                    goto CLEANUP;
                }

                //add population id
                PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_POPULATION_ID, population_id, ebufp);

                 // add stb id
                tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, PIN_ELEMID_ANY, 0, ebufp);
		vm_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 1,ebufp);
                PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_NAME, task_flistp, MSO_FLD_STB_ID, ebufp);

         	//VM  serial number
		  tmp_flistp = PIN_FLIST_ELEM_GET(ret_flistp,PIN_FLD_RESULTS,0,0,ebufp);
		vc_id = PIN_FLIST_FLD_TAKE(tmp_flistp, MSO_FLD_SERIAL_NO, 0, ebufp);
                PIN_FLIST_FLD_PUT(task_flistp, MSO_FLD_VC_ID, vc_id, ebufp);
					  
		//Zip code 
		 tmp_flistp = PIN_FLIST_ELEM_GET(return_flistp,PIN_FLD_RESULTS,PIN_ELEMID_ANY,0,ebufp);
		 PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_AREA, *i_flistpp, MSO_FLD_AREA, ebufp);
		 name_flistp = PIN_FLIST_ELEM_GET(tmp_flistp,PIN_FLD_NAMEINFO,PIN_ELEMID_ANY,0,ebufp);
		 PIN_FLIST_FLD_COPY(name_flistp, PIN_FLD_ZIP, *i_flistpp, PIN_FLD_ZIP, ebufp);
					   
                 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "ZIPCODE", name_flistp);

                fm_mso_get_device_details(ctxp,i_flistpp,vm_id,ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "prepare_catv_activation_flist device details return", *i_flistpp);

                device_type = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_DEVICE_TYPE, 0, ebufp);
                state_id = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_STATE_ID, 0, ebufp);
                PIN_FLIST_FLD_SET (task_flistp, PIN_FLD_DEVICE_TYPE, device_type, ebufp);
                PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_DEVICE_TYPE, ebufp);
                PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_STATE_ID, ebufp);
                       // add device_type for vc
                //tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_DEVICES, 1, 1, ebufp);
                /*tmp_flistp = PIN_FLIST_ELEM_GET(*i_flistpp, PIN_FLD_DEVICES, 1, 1, ebufp);
                        if(tmp_flistp != NULL)
                PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_DEVICE_TYPE, task_flistp, PIN_FLD_DEVICE_TYPE, ebufp);*/


                    /***************** TASK 2 **********************/
                tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
                task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 2, ebufp);
                PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "SET_BOUQUET_ID", ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "prepare_catv_activation_flist catv info flist",tmp_flistp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"error in getting device details",ebufp);
                        goto CLEANUP;
                }

                bouquet_id = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_BOUQUET_ID, 1, ebufp);

                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, bouquet_id);

                if (  bouquet_id && strlen(bouquet_id) > 0)
                {
                    PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_BOUQUET_ID, bouquet_id, ebufp);
                }
                else
                {

			mso_device_typep = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_DEVICE_TYPE, 1, ebufp);
			if (mso_device_typep)
			{
				device_hd_flag = *mso_device_typep;
			}

			i_flistp = PIN_FLIST_CREATE(ebufp);
	        	PIN_FLIST_FLD_COPY(so_flistp, PIN_FLD_ACCOUNT_OBJ, i_flistp, PIN_FLD_POID, ebufp);
	        	PIN_FLIST_FLD_COPY(so_flistp, PIN_FLD_ACCOUNT_OBJ, i_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);

			tmp_flistp = PIN_FLIST_ELEM_GET(return_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
			PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_AREA, i_flistp, MSO_FLD_AREA, ebufp);
			PIN_FLIST_ELEM_SET(i_flistp, so_flistp, PIN_FLD_SERVICES, 0, ebufp);
			PIN_FLIST_FLD_SET(i_flistp, MSO_FLD_NETWORK_NODE, ne, ebufp);

			fm_mso_populate_bouquet_id(ctxp, i_flistp, device_hd_flag, 0, &r_flistp, ebufp);

			PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

			tmp_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_SERVICES, 0, 0, ebufp);
			sub_flistp = PIN_FLIST_SUBSTR_GET(tmp_flistp, MSO_FLD_CATV_INFO, 0, ebufp); 
			bouquet_id = PIN_FLIST_FLD_TAKE(sub_flistp, MSO_FLD_BOUQUET_ID, 1, ebufp);
			PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
			if ( bouquet_id && strlen(bouquet_id) > 0)
			{
				write_iflistp = PIN_FLIST_CREATE(ebufp);
    				PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, write_iflistp, PIN_FLD_POID, ebufp);
				tmp_flistp = PIN_FLIST_SUBSTR_ADD(write_iflistp, MSO_FLD_CATV_INFO, ebufp);	
				PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_BOUQUET_ID, bouquet_id, ebufp);	

				PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_iflistp, &r_flistp, ebufp);

				PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
				PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

				PIN_FLIST_FLD_PUT(task_flistp, MSO_FLD_BOUQUET_ID, bouquet_id, ebufp);

			}
                	else
                	{
                    		pin_set_err(ebufp, PIN_ERRLOC_FM,
                       			PIN_ERRCLASS_SYSTEM_DETERMINATE,
                       			PIN_ERR_BAD_VALUE, MSO_FLD_BOUQUET_ID, 0, 0);
                    		goto CLEANUP;
                	}
                	PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_DEVICE_TYPE, ebufp);
                }

                add_package_detail(ctxp, i_flistpp, ADD_PACKAGE, ne, 0, ebufp);
        	PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_AREA, ebufp);
            }
		/******************* VERIMATRIX CHANGES - END ***********************
		 * Creating provisioning order for activating Verimatrix accounts
		 **********************************************************************/
            else
            {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                    PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
                goto CLEANUP;
            }
        }
    /*
     * Enrich with additional details required by Provisioning system
     */


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_catv_activation_flist enriched input flist", *i_flistpp);

CLEANUP:
    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&return_flistp, NULL);
	//PIN_FLIST_DESTROY_EX(&in_flistp,NULL);
	//PIN_FLIST_DESTROY_EX(&input_flistp,NULL);
    return;
}




/*******************************************************************
 * prepare_catv_preactivation_payload()
 *
 * Enrich input filst with service details required
 * optionally performa validation on input fields
 *******************************************************************/
static void
prepare_catv_preactivation_payload(
    pcm_context_t       *ctxp,
    pin_flist_t         **i_flistpp,
    char                **act,
    pin_errbuf_t        *ebufp)
{
    pin_flist_t         *tmp_flistp = NULL;
    pin_flist_t         *task_flistp = NULL;
    char                *population_id = "0001"; //this is the only expected value in downstream system
    char                *ne = NULL;
    char                *admin_status = "DAS_InServiceOneWay";
    char                *bouquet_id = NULL;
    char                *region_key = NULL;
    char                *ca_id = NULL;
    char                *stb_id = NULL;
    char                *vc_id = NULL;
    char                *device_type = NULL;

	
    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);


    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_activation_flist input flist", *i_flistpp);

    /*
     * Validate input received
     */
    ne = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_NETWORK_NODE, 0, ebufp);

    if(strcmp(ne, "NONE") == 0)
    {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "No need to generate the prov request ");
    }
    else
    {

    if ( ne && strstr(ne,"NDS") )
    {
       /*
        * Validate input received
        */
        region_key = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_REGION_KEY, 0, ebufp);
        stb_id = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_STB_ID, 0, ebufp);
        vc_id = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_VC_ID, 0, ebufp);
        bouquet_id = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_BOUQUET_ID, 0, ebufp);
        ca_id = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_CA_ID, 0, ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                "prepare_catv_activation_flist error: required field missing in input flist", ebufp);
            return;
        }

        *act = (char *) malloc(sizeof("ACTIVATE_SUBSCRIBER_NDS"));
        strcpy(*act,"ACTIVATE_SUBSCRIBER_NDS");
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );

            /***************** TASK 1 **********************/
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "CREATE_SUBSCRIBER", ebufp);

        PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_REGION_KEY, region_key, ebufp);
        PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_POPULATION_ID, population_id, ebufp);
        PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_STB_ID, stb_id, ebufp);
        PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_VC_ID, vc_id, ebufp);

        fm_mso_get_device_details(ctxp,i_flistpp,vc_id,ebufp);
        device_type = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_DEVICE_TYPE, 0, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, PIN_FLD_DEVICE_TYPE, device_type, ebufp);

            /***************** TASK 2 **********************/
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 2, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "SET_BOUQUET_ID", ebufp);
        PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_BOUQUET_ID, bouquet_id, ebufp);

            /***************** TASK 3 **********************/
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 3, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "AUTHORIZE_SERVICES", ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_ADD_PACKAGE_INFO, 1, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_CA_ID, ca_id, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_TAPING_AUTHORIZATION, "N", ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_EXPIRATION_DATE, "20150101", ebufp);


        PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_STATE_ID, ebufp);
        PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_REGION_KEY, ebufp);
        PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_STB_ID, ebufp);
        PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_VC_ID, ebufp);
        PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_CA_ID, ebufp);
        PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_BOUQUET_ID, ebufp);
//        PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);
    }
    else if (ne  && strstr(ne,"CISCO") )
    {
       /*
        * Validate input received
        */
        stb_id = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_STB_ID, 0, ebufp);
        ca_id = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_CA_ID, 0, ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                "prepare_catv_activation_flist error: required field missing in input flist", ebufp);
            return;
        }

            *act = (char *) malloc(sizeof("ACTIVATE_SUBSCRIBER_PK"));
            strcpy(*act,"ACTIVATE_SUBSCRIBER_PK");
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );

                /***************** TASK 1 **********************/
            task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
            PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "MODIFY_DHCT_ADMIN_STATUS", ebufp);
                /*
                 * set admin status - required for CISCO PK
                 * value is always DAS_InServiceOneWay
                 */
            PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_ADMIN_STATUS, admin_status, ebufp);

                /***************** TASK 2 **********************/
            task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 2, ebufp);
            PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "MODIFY_DHCT_CONFIG", ebufp);
            tmp_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_ADD_PACKAGE_INFO, 1, ebufp);
            PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_CA_ID, ca_id, ebufp);

                /***************** TASK 3 **********************/
            task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 3, ebufp);
            PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "INSTANT_HIT", ebufp);

    //        PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);
            PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_CA_ID, ebufp);
            PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_REGION_KEY, ebufp);
            PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_BOUQUET_ID, ebufp);
        }
    	
        else
        {
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
        }
    }

    /*
     * Enrich with additional details required by Provisioning system
     */


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_catv_preactivation_payload enriched input flist", *i_flistpp);

    return;
}


/*******************************************************************
 * prepare_catv_add_package_flist()
 *
 * Enrich input filst with service details required
 * optionally performa validation on input fields
 *******************************************************************/
static void
prepare_catv_add_package_flist(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    char                    **act,
    pin_errbuf_t            *ebufp)
{

    pin_flist_t             *si_flistp = NULL;
    pin_flist_t             *so_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    pin_flist_t             *name_flistp = NULL;
    pin_flist_t             *task_flistp = NULL;
    pin_flist_t             *tmp_flistp1 = NULL;
    void                    *vp = NULL;
    char                    *ne = NULL;
    char                    *cas_subs_id = NULL;
    char                    *dms_enabled = "DMS_Enabled";
	pin_flist_t		*in_flistp = NULL;
	pin_flist_t		*tempp_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;
	int32		*pkg_typep = NULL;
	int32		package_type = -1;
    int32       renew_flagp = 0;
    int32       task_id = 1;
    int32       *flagp = NULL;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_add_package_flist input flist", *i_flistpp);

    /*
     * Validate input received
     */
    // There should be at least one product to be added
    tmp_flistp = PIN_FLIST_ELEM_GET(*i_flistpp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, 0, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp))
            return;

    /*
     * Enrich with additional service details
     */
    si_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);
    tmp_flistp = PIN_FLIST_SUBSTR_ADD(si_flistp, MSO_FLD_CATV_INFO, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, NULL, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_NETWORK_NODE, NULL, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, 0, ebufp);
    tmp_flistp1 = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, 1, ebufp);
    tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, 2, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_add_package_flist PCM_OP_READ_OBJ input flist", si_flistp);
    PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_add_package_flist PCM_OP_READ_FLDS output flist", so_flistp);

    tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
    ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);
    PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

	pkg_typep = (int32 *)PIN_FLIST_FLD_GET(*i_flistpp,  MSO_FLD_PKG_TYPE, 1, ebufp);
	if ( pkg_typep && *pkg_typep == 0)
	{
		package_type = 0;
	}

    if(strcmp(ne, "NONE") == 0)
    {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "No need to generate the prov request ");
    }
    else
    {
    if ( ne && (strstr(ne,"NDS") || strstr(ne, "NAGRA")))
    {
	if (strstr(ne,"NDS"))
	{
        	*act = (char *) malloc(sizeof("AUTHORIZE_SERVICES_NDS"));
        	strcpy(*act,"AUTHORIZE_SERVICES_NDS");
	}
	else
	{
        	*act = (char *) malloc(sizeof("AUTHORIZE_SERVICES_NGR"));
        	strcpy(*act, "AUTHORIZE_SERVICES_NGR");
	}
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );

                //add cas subscriber id
        cas_subs_id = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, 0, ebufp);

        if ( cas_subs_id && strlen(cas_subs_id) > 0)
        {
            PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_CAS_SUBSCRIBER_ID, cas_subs_id, ebufp);
        }
        else
        {
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    PIN_ERR_BAD_VALUE, MSO_FLD_CAS_SUBSCRIBER_ID, 0, 0);
            goto CLEANUP;
        }

        add_package_detail(ctxp, i_flistpp, ADD_PACKAGE, ne, 1, ebufp);
    }
    else if (ne && (strstr(ne,"CISCO") || strstr(ne, "NAGRA")))
    {
       	*act = (char *) malloc(sizeof("AUTHORIZE_SERVICES_PK"));
       	strcpy(*act,"AUTHORIZE_SERVICES_PK");
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );

            //set mac address
                if ( (PIN_FLIST_ELEM_COUNT(so_flistp, PIN_FLD_ALIAS_LIST, ebufp)) >0)
        {
                        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 2, 0, ebufp);
                        PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_NAME, *i_flistpp, MSO_FLD_STB_ID, ebufp);
                }

            /***************** TASK 1 **********************/
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
        //PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "MODIFY_DHCT_CONFIG", ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "AUTHORIZE_SERVICES", ebufp);
            /*
             * set dms enabled status - required for CISCO PK
             * value is always DMS_Enabled
             */
        //PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_DMS_ENABLED, dms_enabled, ebufp);

        // call function to add package info
        add_package_detail(ctxp, i_flistpp, ADD_PACKAGE, ne, 0, ebufp);

            /***************** TASK 2 **********************/
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 2, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "INSTANT_HIT", ebufp);

    }
	/******************* VERIMATRIX CHANGES - BEGIN ***********************
	 * Creating provisioning order for activating Verimatrix accounts
	 **********************************************************************/	
    else if ( ne && (strstr(ne,"VM") || strstr(ne,"GOSPEL")))
    {
	if (strstr(ne,"GOSPEL"))
	{
                *act = (char *) malloc(sizeof("AUTHORIZE_SERVICES_GOSPEL"));
                strcpy(*act,"AUTHORIZE_SERVICES_GOSPEL");
	}
	else if (strstr(ne, "JVM"))
	{
        	*act = (char *) malloc(sizeof("AUTHORIZE_SERVICES_JVM"));
        	strcpy(*act,"AUTHORIZE_SERVICES_JVM");
	}
	else
	{
        	*act = (char *) malloc(sizeof("AUTHORIZE_SERVICES_VM"));
        	strcpy(*act,"AUTHORIZE_SERVICES_VM");
	}
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );

        in_flistp  = PIN_FLIST_CREATE(ebufp);
	tempp_flistp = PIN_FLIST_ELEM_GET(so_flistp,PIN_FLD_ALIAS_LIST,0, 1, ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, in_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(tempp_flistp, PIN_FLD_NAME, in_flistp, PIN_FLD_NAME, ebufp);
	   
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"s_no input flist", in_flistp);
	   
	search_serial_no(ctxp, in_flistp, &ret_flistp,ebufp);
	   
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"s_no output flist", ret_flistp);
		 //VM STB NUMBER
	tempp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);			
	if (strstr(ne,"VM"))
	{
		PIN_FLIST_FLD_COPY(tempp_flistp, PIN_FLD_NAME, *i_flistpp, MSO_FLD_STB_ID, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_COPY(tempp_flistp, PIN_FLD_NAME, *i_flistpp, MSO_FLD_VC_ID, ebufp);
	}

		//VM  serial number
	tempp_flistp = PIN_FLIST_ELEM_GET(ret_flistp,PIN_FLD_RESULTS,0,0,ebufp);
	if (strstr(ne,"VM"))
	{
		PIN_FLIST_FLD_COPY(tempp_flistp, MSO_FLD_SERIAL_NO, *i_flistpp, MSO_FLD_VC_ID, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_COPY(tempp_flistp, MSO_FLD_SERIAL_NO, *i_flistpp, MSO_FLD_STB_ID, ebufp);
	}
	
	 
		//VM device type
	tempp_flistp = PIN_FLIST_ELEM_GET(ret_flistp,PIN_FLD_RESULTS,0,0,ebufp);
	PIN_FLIST_FLD_COPY(tempp_flistp, PIN_FLD_DEVICE_TYPE, *i_flistpp, PIN_FLD_DEVICE_TYPE, ebufp);

        add_package_detail(ctxp, i_flistpp, ADD_PACKAGE, ne, 1, ebufp);
	if (strstr(ne, "GOSPEL"))
	{
		task_flistp = PIN_FLIST_ELEM_GET(*i_flistpp, MSO_FLD_TASK, 1, 1, ebufp);
		if (task_flistp)
		{
			PIN_FLIST_FLD_COPY(*i_flistpp, MSO_FLD_VC_ID, task_flistp, MSO_FLD_VC_ID, ebufp);
		}

		if (package_type == 0)
		{
			task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 3, ebufp);
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "ACTIVATE_CARD", ebufp);
			PIN_FLIST_FLD_COPY(*i_flistpp, MSO_FLD_VC_ID, task_flistp, MSO_FLD_VC_ID, ebufp);	
		}
	}
    if (strstr(ne, "JVM"))
    {
        if(PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_OVERRIDE_FLAGS, 1, ebufp) != NULL)
        {
            flagp = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_OVERRIDE_FLAGS, 0, ebufp);
            renew_flagp = *(int32 *)flagp;
        }
        if( renew_flagp && renew_flagp == 1)
        {
            task_id = PIN_FLIST_ELEM_COUNT(*i_flistpp, MSO_FLD_TASK, ebufp);
            task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, ++task_id, ebufp);
            PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "RETRACK", ebufp);
        }
    }
    }
	/******************* VERIMATRIX CHANGES - END ***********************
	 * Creating provisioning order for activating Verimatrix accounts
	 **********************************************************************/
    else
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
        goto CLEANUP;
    }
    }


    /*
     * Enrich with additional details required by Provisioning system
     */


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_catv_add_package_flist enriched input flist", *i_flistpp);

CLEANUP:
    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
 //   PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
    return;

}

/**************************************************************************
 * prepare_catv_plan_change_flist()
 *
 * 1. calls add_package action with products array and status_flags <> 1
 * 2. calls cancel_plan action with products array and status_flags == 1
 * optionally performa validation on input fields
 **************************************************************************/
static void
prepare_catv_plan_change_flist(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    char                    **act,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *si_flistp = NULL;
    pin_flist_t             *so_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    pin_flist_t             *task_flistp = NULL;
    pin_flist_t             *sub_flistp = NULL;
//    void                    *vp = NULL;
    char                    *ne = NULL;
    char                    *cas_subs_id = NULL;
	pin_flist_t		*in_flistp = NULL;
	pin_flist_t		*tempp_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;


    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_plan_change_flist input flist", *i_flistpp);

    /*
     * Validate input received
     */


    si_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);
    tmp_flistp = PIN_FLIST_SUBSTR_ADD(si_flistp, MSO_FLD_CATV_INFO, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, NULL, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_NETWORK_NODE, NULL, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, 0, ebufp);
    tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, 2, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_add_package_flist PCM_OP_READ_OBJ input flist", si_flistp);
    PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_add_package_flist PCM_OP_READ_FLDS output flist", so_flistp);

    tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
    ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);
    PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

    if(strcmp(ne, "NONE") == 0)
    {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "No need to generate the prov request ");
    }
    else
    {

    if ( ne && (strstr(ne,"NDS") || strstr(ne,"NAGRA")))
    {
	if (strstr(ne,"NDS"))
	{
        	*act = (char *) malloc(sizeof("CHANGE_SERVICE_NDS"));
        	strcpy(*act,"CHANGE_SERVICE_NDS");
	}	
	else
	{
        	*act = (char *) malloc(sizeof("CHANGE_SERVICE_NGR"));
        	strcpy(*act,"CHANGE_SERVICE_NGR");
	}
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );

                //add cas subscriber id
        cas_subs_id = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, 0, ebufp);

        if ( cas_subs_id && strlen(cas_subs_id) > 0)
        {
            PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_CAS_SUBSCRIBER_ID, cas_subs_id, ebufp);
        }
        else
        {
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    PIN_ERR_BAD_VALUE, MSO_FLD_CAS_SUBSCRIBER_ID, 0, 0);
            goto CLEANUP;
        }
        add_package_detail(ctxp, i_flistpp, CHANGE_PACKAGE, ne, 0, ebufp);

    }
    else if (ne  && strstr(ne,"CISCO") )
    {
        *act = (char *) malloc(sizeof("CHANGE_SERVICE_PK"));
        strcpy(*act,"CHANGE_SERVICE_PK");
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
            //set mac address
        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 2, 0, ebufp);
        PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_NAME, *i_flistpp, MSO_FLD_STB_ID, ebufp);

            /***************** TASK 1 **********************/
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "AUTHORIZE_SERVICES", ebufp);

            /***************** TASK 2 **********************/
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 2, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "UNAUTHORIZE_SERVICES", ebufp);

        // call function to add package info
        add_package_detail(ctxp, i_flistpp, CHANGE_PACKAGE, ne, 0, ebufp);

    }
	/******************* VERIMATRIX CHANGES - BEGIN ***********************
	 * Creating provisioning order to change service for Verimatrix accounts
	 **********************************************************************/	
	else if ( ne && (strstr(ne,"VM") || strstr(ne,"GOSPEL")))
    {
	if (strstr(ne, "GOSPEL"))
	{
		*act = (char *) malloc(sizeof("CHANGE_SERVICE_GOSPEL"));
		strcpy(*act,"CHANGE_SERVICE_GOSPEL");
	}
	else if (strstr(ne, "JVM"))
	{
        	*act = (char *) malloc(sizeof("CHANGE_SERVICE_JVM"));
        	strcpy(*act,"CHANGE_SERVICE_JVM");
	}
	else
	{
        	*act = (char *) malloc(sizeof("CHANGE_SERVICE_VM"));
        	strcpy(*act,"CHANGE_SERVICE_VM");
	}
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		
		in_flistp  = PIN_FLIST_CREATE(ebufp);
	    tempp_flistp = PIN_FLIST_ELEM_GET(so_flistp,PIN_FLD_ALIAS_LIST,0, 1, ebufp);
	    PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, in_flistp, PIN_FLD_POID, ebufp);
	    PIN_FLIST_FLD_COPY(tempp_flistp, PIN_FLD_NAME, in_flistp, PIN_FLD_NAME, ebufp);
	     
	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"s_no input flist", in_flistp);
	     
	    search_serial_no(ctxp, in_flistp, &ret_flistp,ebufp);
	     
	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"s_no output flist", ret_flistp);

                
		//VM STB NUMBER
		tempp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);			
		if (strstr(ne, "VM"))
		{
			PIN_FLIST_FLD_COPY(tempp_flistp, PIN_FLD_NAME, *i_flistpp, MSO_FLD_STB_ID, ebufp);
		}
		else
		{
			PIN_FLIST_FLD_COPY(tempp_flistp, PIN_FLD_NAME, *i_flistpp, MSO_FLD_VC_ID, ebufp);
		}

		PIN_FLIST_FLD_COPY(tempp_flistp, MSO_FLD_LCO_OBJ, *i_flistpp, MSO_FLD_LCO_OBJ, ebufp);
		//VM  serial number
		tempp_flistp = PIN_FLIST_ELEM_GET(ret_flistp,PIN_FLD_RESULTS,0,0,ebufp);
		if (strstr(ne, "VM"))
		{
			PIN_FLIST_FLD_COPY(tempp_flistp, MSO_FLD_SERIAL_NO, *i_flistpp, MSO_FLD_VC_ID, ebufp);
		}
		else
		{
			PIN_FLIST_FLD_COPY(tempp_flistp, MSO_FLD_SERIAL_NO, *i_flistpp, MSO_FLD_STB_ID, ebufp);
		}
	 
		PIN_FLIST_FLD_COPY(tempp_flistp, PIN_FLD_ACCOUNT_OBJ, *i_flistpp, MSO_FLD_LCO_OBJ, ebufp);
		//VM device type
		tempp_flistp = PIN_FLIST_ELEM_GET(ret_flistp,PIN_FLD_RESULTS,0,0,ebufp);
		PIN_FLIST_FLD_COPY(tempp_flistp, PIN_FLD_DEVICE_TYPE, *i_flistpp, PIN_FLD_DEVICE_TYPE, ebufp);

       	add_package_detail(ctxp, i_flistpp, CHANGE_PACKAGE, ne, 0, ebufp);

    }
	/******************* VERIMATRIX CHANGES - BEGIN ***********************
	 * Creating provisioning order to change service for Verimatrix accounts
	 **********************************************************************/	
    else
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
        goto CLEANUP;
    }
    }


    /*
     * Enrich with additional details required by Provisioning system
     */


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_catv_plan_change_flist enriched input flist", *i_flistpp);

CLEANUP:
    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
  // PIN_FLIST_DESTROY_EX(&in_flistp, NULL);

    return;

}

/**************************************************************************
 * prepare_catv_change_preactivated_plan_flist()
 *
 * 1. calls add_package action with products array and status_flags <> 1
 * 2. calls cancel_plan action with products array and status_flags == 1
 * optionally performa validation on input fields
 **************************************************************************/
static void
prepare_catv_change_preactivated_plan_flist(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    char                    **act,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *sub_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    pin_flist_t             *task_flistp = NULL;
    pin_flist_t             *si_flistp = NULL;
    pin_flist_t             *so_flistp = NULL;
    char                    *ne = NULL;
    char                    *cas_subs_id = NULL;
    char                    *ca_id = NULL;
    char                    *stb_id = NULL;
    int32                   args = 0;
    int                     *value = NULL;
    poid_t                  *stb_obj = NULL;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_change_preactivated_plan_flist input flist", *i_flistpp);

    /*
     * Validate input received
     */

//    PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, *i_flistpp, MSO_FLD_CAS_SUBSCRIBER_ID, ebufp);
//    PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);


//    while ( (tmp_flistp = PIN_FLIST_ELEM_GET_NEXT (*i_flistpp,
//                PIN_FLD_PRODUCTS, &rec_id, 1,&cookie, ebufp))
//                                          != (pin_flist_t *)NULL )
//    {
//            value = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_STATUS_FLAGS, 0, ebufp);
//
//            /* status_flags 1 will be considered for de-activation else activation
//                other than 1 is activation */
//            if (  *value == 1 )
//            {
//                sub_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, PIN_FLD_PACKAGE_INFO, 0, ebufp);
//                PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_CA_ID, sub_flistp, MSO_FLD_CA_ID, ebufp);
//                PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, *i_flistpp, MSO_FLD_CAS_SUBSCRIBER_ID, ebufp);
//                PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);
//            }
//        PIN_FLIST_ELEM_DROP(*i_flistpp, PIN_FLD_PRODUCTS, rec_id, ebufp);
//        cookie = prev_cookie;
//    }//end while

    /*
     * opcode expects package details for cancellation in
     * PIN_FLD_PRODUCTS array with elem id 0
     */
    tmp_flistp = PIN_FLIST_ELEM_GET (*i_flistpp, PIN_FLD_PRODUCTS, 0, 0, ebufp);
    ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);
    value = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_STATUS_FLAGS, 0, ebufp);
    ca_id = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_CA_ID, 0, ebufp);

    PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

    if(strcmp(ne, "NONE") == 0)
    {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "No need to generate the prov request ");
    }
    else
    {
    if ( ne && strstr(ne,"NDS") )
    {
        *act = (char *) malloc(sizeof("CHANGE_SERVICE_NDS"));
        strcpy(*act,"CHANGE_SERVICE_NDS");
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );

        cas_subs_id = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, 0, ebufp);

                    //add cas subscriber id
        if ( cas_subs_id && strlen(cas_subs_id) > 0)
        {
            PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_CAS_SUBSCRIBER_ID, cas_subs_id, ebufp);
        }
        else
        {
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    PIN_ERR_BAD_VALUE, MSO_FLD_CAS_SUBSCRIBER_ID, 0, 0);
            return;
        }

            /***************** TASK 2 **********************/
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 2, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "UNAUTHORIZE_SERVICES", ebufp);

        /* status_flags 1 will be considered for de-activation else activation
              other than 1 is activation */
        if (  *value == 1 )
        {
            sub_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_DEL_PACKAGE_INFO, 1, ebufp);

                //add ca_id id
            if ( ca_id && strlen(ca_id) > 0)
            {
                PIN_FLIST_FLD_SET(sub_flistp, MSO_FLD_CA_ID, ca_id, ebufp);
            }
            else
            {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                    PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_VALUE, MSO_FLD_CA_ID, 0, 0);
                return;
            }
        }

        PIN_FLIST_ELEM_DROP(*i_flistpp, PIN_FLD_PRODUCTS, 0, ebufp);

            /***************** TASK 1 **********************/
        add_package_detail(ctxp, i_flistpp, ADD_PACKAGE, ne, 0, ebufp);
    }
    else if (ne  && strstr(ne,"CISCO") )
    {
        *act = (char *) malloc(sizeof("CHANGE_SERVICE_PK"));
        strcpy(*act,"CHANGE_SERVICE_PK");
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );

        stb_obj = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_STB_OBJ, 0, ebufp);

        si_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(si_flistp, PIN_FLD_POID, stb_obj, ebufp);
        PIN_FLIST_FLD_SET(si_flistp, PIN_FLD_DEVICE_ID, NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "prepare_catv_change_preactivated_plan_flist PCM_OP_READ_OBJ input flist", si_flistp);
        PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "prepare_catv_change_preactivated_plan_flist PCM_OP_READ_FLDS output flist", so_flistp);

        stb_id = PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_DEVICE_ID, 0, ebufp);

            //add stb id
        if ( stb_id && strlen(stb_id) > 0)
        {
            PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_STB_ID, stb_id, ebufp);
        }
         else
         {
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    PIN_ERR_BAD_VALUE, MSO_FLD_STB_ID, 0, 0);
            return;
        }

            /***************** TASK 2 **********************/
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 2, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "UNAUTHORIZE_SERVICES", ebufp);

        /* status_flags 1 will be considered for de-activation else activation
             other than 1 is activation */
        if (  *value == 1 )
        {
            sub_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_DEL_PACKAGE_INFO, 1, ebufp);

               //add ca_id id
            if ( ca_id && strlen(ca_id) > 0)
            {
                PIN_FLIST_FLD_SET(sub_flistp, MSO_FLD_CA_ID, ca_id, ebufp);
            }
            else
            {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                    PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_VALUE, MSO_FLD_CA_ID, 0, 0);
                return;
            }
         }

         PIN_FLIST_ELEM_DROP(*i_flistpp, PIN_FLD_PRODUCTS, 0, ebufp);

            /***************** TASK 1 **********************/
         task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
         PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "AUTHORIZE_SERVICES", ebufp);

         add_package_detail(ctxp, i_flistpp, ADD_PACKAGE, ne, 0, ebufp);

         PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
         PIN_FLIST_DESTROY_EX(&so_flistp, NULL);

    }
	
    else
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
        return;
    }
    }


    return;

}

/*******************************************************************
 * prepare_catv_cancel_plan_flist()
 *
 * Enrich input filst with service details required
 * optionally performa validation on input fields
 *******************************************************************/
static void
prepare_catv_cancel_plan_flist(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    char                    **act,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t     *si_flistp = NULL;
    pin_flist_t     *so_flistp = NULL;
    pin_flist_t     *tmp_flistp = NULL;
    pin_flist_t     *tmp_flistp1 = NULL;
    pin_flist_t     *task_flistp = NULL;
    pin_flist_t     *name_flistp = NULL;
    pin_flist_t     *return_flistp = NULL;
    pin_flist_t     *input_flistp = NULL;
    poid_t          *product_pdp = NULL;
    poid_t          *prd_pdp = NULL;
    char            *ne = NULL;
    char            *cas_subs_id = NULL;
    char	    validity[21];
    char            *program_namep = NULL;
    void            *vp = NULL;
    pin_decimal_t   *priority      = NULL;
	pin_flist_t		*in_flistp = NULL;
	pin_flist_t		*tempp_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;
	struct		tm *current_time;
	time_t		now_t = 0;
	int32		year;
	int32		month;
	int32		day;
	int32		*pkg_typep = NULL;
	int32		package_type = -1;
    int32       is_base_pack = -1;
    int32       profile_srch_type = MSO_FLAG_SRCH_BY_SELF_ACCOUNT;



    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_cancel_plan_flist input flist", *i_flistpp);

	pkg_typep = (int32 *)PIN_FLIST_FLD_GET(*i_flistpp,  MSO_FLD_PKG_TYPE, 1, ebufp);
	if ( pkg_typep && *pkg_typep == 0)
	{
		package_type = 0;
	}
    /*
     * Validate input received
     */
    // There should be at least one product to cancel
    tmp_flistp = PIN_FLIST_ELEM_GET(*i_flistpp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, 0, ebufp);
    product_pdp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_POID, 1, ebufp);
    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "prepare_catv_cancel_plan_flist error: required field missing in input flist", ebufp);
        return;
    }
    /*
     * Enrich with additional service details
     */
    si_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);
    tmp_flistp = PIN_FLIST_SUBSTR_ADD(si_flistp, MSO_FLD_CATV_INFO, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, NULL, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_NETWORK_NODE, NULL, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, 0, ebufp);
    tmp_flistp1 = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, 1, ebufp);
    tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, 2, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_cancel_plan_flist PCM_OP_READ_OBJ input flist", si_flistp);
    PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_cancel_plan_flist PCM_OP_READ_FLDS output flist", so_flistp);

    tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
    ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);
    PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

    if(strcmp(ne, "NONE") == 0)
    {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "No need to generate the prov request ");
    }
    else
    {
    if ( ne && (strstr(ne,"NDS") || strstr(ne,"NAGRA")))
    {
	if (strstr(ne, "NDS"))
	{
        	*act = (char *) malloc(sizeof("CANCEL_SERVICES_NDS"));
        	strcpy(*act,"CANCEL_SERVICES_NDS");
	}
	else
	{
        	*act = (char *) malloc(sizeof("CANCEL_SERVICES_NGR"));
        	strcpy(*act,"CANCEL_SERVICES_NGR");
	}
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                    //add cas subscriber id
        cas_subs_id = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, 0, ebufp);

        if ( cas_subs_id && strlen(cas_subs_id) > 0)
        {
            PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_CAS_SUBSCRIBER_ID, cas_subs_id, ebufp);
        }
        else
        {
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    PIN_ERR_BAD_VALUE, MSO_FLD_CAS_SUBSCRIBER_ID, 0, 0);
            goto CLEANUP;
        }
        if (package_type == 0 && strcmp(ne, "NDS2") != 0)
        {
        	add_package_detail(ctxp, i_flistpp, CANCEL_PACKAGE, ne, 1, ebufp);
        }
        else
        {
        	add_package_detail(ctxp, i_flistpp, CANCEL_PACKAGE, ne, 0, ebufp);
        }
    }
    else if (ne  && strstr(ne,"CISCO") )
    {
        *act = (char *) malloc(sizeof("CANCEL_SERVICES_PK"));
        strcpy(*act,"CANCEL_SERVICES_PK");
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );

            //set mac address
        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 2, 0, ebufp);
        PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_NAME, *i_flistpp, MSO_FLD_STB_ID, ebufp);

            /***************** TASK 1 **********************/
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "UNAUTHORIZE_SERVICES", ebufp);

        // call function to add package info
        add_package_detail(ctxp, i_flistpp, CANCEL_PACKAGE, ne, 0, ebufp);

    }
	/******************* VERIMATRIX CHANGES - BEGIN ***********************
	 * Creating provisioning order to cancel service for Verimatrix accounts
	 **********************************************************************/		
	else if ( ne && (strstr(ne,"VM") || strstr(ne,"GOSPEL")))
    {
	if (strstr(ne,"GOSPEL"))
	{
                *act = (char *) malloc(sizeof("CANCEL_SERVICES_GOSPEL"));
                strcpy(*act,"CANCEL_SERVICES_GOSPEL");
	}
	else if (strstr(ne, "JVM"))
	{
        	*act = (char *) malloc(sizeof("CANCEL_SERVICES_JVM"));
        	strcpy(*act,"CANCEL_SERVICES_JVM");
	}
	else
	{
        	*act = (char *) malloc(sizeof("CANCEL_SERVICES_VM"));
        	strcpy(*act,"CANCEL_SERVICES_VM");
	}
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
        
		in_flistp  = PIN_FLIST_CREATE(ebufp);
	    tempp_flistp = PIN_FLIST_ELEM_GET(so_flistp,PIN_FLD_ALIAS_LIST,0, 1, ebufp);
	    PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, in_flistp, PIN_FLD_POID, ebufp);
	    PIN_FLIST_FLD_COPY(tempp_flistp, PIN_FLD_NAME, in_flistp, PIN_FLD_NAME, ebufp);
	    
	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"s_no input flist", in_flistp);
	    
	    search_serial_no(ctxp, in_flistp, &ret_flistp,ebufp);
	    
	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"s_no output flist", ret_flistp);
	    	
	    //VM STB NUMBER
	    tempp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);			
	if (strstr(ne, "VM"))
	{
        	PIN_FLIST_FLD_COPY(tempp_flistp, PIN_FLD_NAME, *i_flistpp, MSO_FLD_STB_ID, ebufp);
	}
	else
	{
        	PIN_FLIST_FLD_COPY(tempp_flistp, PIN_FLD_NAME, *i_flistpp, MSO_FLD_VC_ID, ebufp);
	}
        
        //VM  serial number
        tempp_flistp = PIN_FLIST_ELEM_GET(ret_flistp,PIN_FLD_RESULTS,0,0,ebufp);
	if (strstr(ne, "VM"))
	{
        	PIN_FLIST_FLD_COPY(tempp_flistp, MSO_FLD_SERIAL_NO, *i_flistpp, MSO_FLD_VC_ID, ebufp);
	}
	else
	{
        	PIN_FLIST_FLD_COPY(tempp_flistp, MSO_FLD_SERIAL_NO, *i_flistpp, MSO_FLD_STB_ID, ebufp);
	}
	     
	    //VM device type
	    tempp_flistp = PIN_FLIST_ELEM_GET(ret_flistp,PIN_FLD_RESULTS,0,0,ebufp);
        PIN_FLIST_FLD_COPY(tempp_flistp, PIN_FLD_DEVICE_TYPE, *i_flistpp, PIN_FLD_DEVICE_TYPE, ebufp);		 
        if (package_type == 0 && strcmp(ne, "JVM2") != 0 && strcmp(ne, "GOSPELL2") != 0)
        {
            add_package_detail(ctxp, i_flistpp, CANCEL_PACKAGE, ne, 1, ebufp);
        }
        else
        {
            add_package_detail(ctxp, i_flistpp, CANCEL_PACKAGE, ne, 0, ebufp);
        }
	if (strstr(ne, "GOSPEL") && package_type == 0)
	{
		task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 3, ebufp);
               	PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "DEACTIVATE_CARD", ebufp);
       		PIN_FLIST_FLD_COPY(*i_flistpp, MSO_FLD_VC_ID, task_flistp, MSO_FLD_VC_ID, ebufp);

		now_t = pin_virtual_time((time_t *)NULL);
		current_time = localtime(&now_t);
		year = current_time->tm_year + 1900;
		month = current_time->tm_mon +1;
		day = current_time->tm_mday;

		memset(validity,'\0',21);
		sprintf(validity,"%04d-%02d-%02dT00:00:00",year,month,day);	
                PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_EXPIRATION_DATE, validity, ebufp);
	}
    program_namep = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_PROGRAM_NAME, 0, ebufp);
    if(program_namep && !strstr(program_namep,"pin_mta_cancel_plan") && strstr(ne,"VM"))
    {
        si_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(si_flistp, PIN_FLD_POID, product_pdp,ebufp);
        PIN_FLIST_FLD_SET(si_flistp, PIN_FLD_PRODUCT_OBJ, NULL,ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                    "purchased product PCM_OP_READ_OBJ input flist", si_flistp);
        PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
        prd_pdp = PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
        is_base_pack = fm_mso_catv_pt_pkg_type(ctxp, prd_pdp, ebufp);

        if(is_base_pack == 0)
        {
            in_flistp = PIN_FLIST_CREATE(ebufp);
            PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, in_flistp, PIN_FLD_POID, ebufp);
            PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, in_flistp, PIN_FLD_OBJECT, ebufp);
            PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_PROFILE_DESCR, CUSTOMER_CARE, ebufp);
            PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_SEARCH_TYPE, &profile_srch_type, ebufp);

            fm_mso_get_profile_info(ctxp, in_flistp, &return_flistp, ebufp);
            PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
            input_flistp = PIN_FLIST_SUBSTR_GET(return_flistp, PIN_FLD_CUSTOMER_CARE_INFO, 0, ebufp);
            PIN_FLIST_FLD_COPY(input_flistp, MSO_FLD_LCO_OBJ, *i_flistpp, MSO_FLD_LCO_OBJ, ebufp);
            task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 2, ebufp);
            PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "AUTHORIZE_LBO_SERVICES", ebufp);
            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_mso_populate_bouquet_id input flist", *i_flistpp);
            fm_mso_populate_bouquet_id(ctxp, *i_flistpp, 0, 0, &ret_flistp, ebufp);
            PIN_FLIST_FLD_COPY(ret_flistp, PIN_FLD_VALIDITY_IN_DAYS,
            task_flistp, PIN_FLD_VALIDITY_IN_DAYS, ebufp);
            PIN_FLIST_FLD_COPY(ret_flistp, PIN_FLD_VALID_DOM,
                          task_flistp, PIN_FLD_VALID_DOM, ebufp);
            PIN_FLIST_FLD_COPY(ret_flistp, PIN_FLD_TIMEZONE,
                          task_flistp, PIN_FLD_TIMEZONE, ebufp);
            PIN_FLIST_FLD_COPY(ret_flistp, PIN_FLD_SPEED,
                          task_flistp, PIN_FLD_SPEED, ebufp);
            now_t = pin_virtual_time((time_t *)NULL);
            now_t = now_t - 86400;
            current_time = localtime(&now_t);
            year = current_time->tm_year + 1900;
            month = current_time->tm_mon +1;
            day = current_time->tm_mday;
            memset(validity,'\0',21);
            sprintf(validity,"%04d%02d%02d",year,month,day);
            PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_EXPIRYDATE, validity, ebufp);
        }

    }


    }
	/******************* VERIMATRIX CHANGES - END ***********************
	 * Creating provisioning order to cancel service for Verimatrix accounts
	 **********************************************************************/
    else
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
        goto CLEANUP;
    }
    }

    /*
     * Enrich with additional details required by Provisioning system
     */

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_catv_cancel_plan_flist enriched input flist", *i_flistpp);

        if(PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Error set in add_package_function");
        }


CLEANUP:
    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
  //  PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
    return;

}

/*******************************************************************
 * prepare_catv_suspend_flist()
 *
 * Enrich input filst with service details required
 * optionally performa validation on input fields
 *******************************************************************/
static void
prepare_catv_suspend_flist(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    int32                   *error_codep,
    char                    **act,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t     *si_flistp = NULL;
    pin_flist_t     *so_flistp = NULL;
	pin_flist_t     *si_flistpp = NULL;
    pin_flist_t     *so_flistpp = NULL;
    pin_flist_t     *tmp_flistp = NULL;
    pin_flist_t     *tmp_flistp1 = NULL;
    pin_flist_t     *task_flistp = NULL;
    pin_flist_t     *pkg_flistp = NULL;
    pin_flist_t     *r_flistp = NULL;
    pin_flist_t     *task_flistp1 = NULL;
    pin_flist_t     *result_flistp = NULL;
    pin_flist_t     *acnt_info = NULL; 
    char            *ne = NULL;
    char            *cas_subs_id = NULL;
    char            *DMS_disabled = "DMS_Disabled";
	char            *task_namep = NULL;
    char            *ca_id = NULL;
    char            new_ca_id[128];
    pin_flist_t		*in_flistp = NULL;
	pin_flist_t		*tmmp_flistp = NULL;
	pin_flist_t		*tempp_flistp = NULL;
    pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*return_flistp = NULL;
	pin_flist_t		*input_flistp = NULL;
	pin_flist_t		*name_flistp = NULL;
	struct		tm *current_time;
	time_t		now_t = 0;
	int32		year;
	int32		month;
	int32		day;
    int32       device_hd_flag = 0;
    int32       profile_srch_type = MSO_FLAG_SRCH_BY_SELF_ACCOUNT;
	int32       *pkg_typep = NULL;
    int32       business_type = 0;
    int32       arr_business_type[8];
    int32       lob_exclude_flag = -1;
    int32       b_elem_id = 0;
    int32       subscriber_type = -1;
    char		validity[21];
    char        msg[100];
    void        *vp = NULL;
    pin_cookie_t b_cookie = NULL;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_suspend_flist input flist", *i_flistpp);

    /*
     * Validate input received
     */



    /*
     * Enrich with additional service details
     */
    	   si_flistp = PIN_FLIST_CREATE(ebufp);
	   si_flistpp = PIN_FLIST_CREATE(ebufp);
       PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);
	   
	   PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistpp, PIN_FLD_POID, ebufp);
	   
       tmp_flistp = PIN_FLIST_SUBSTR_ADD(si_flistp, MSO_FLD_CATV_INFO, ebufp);
       PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, NULL, ebufp);
       PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_NETWORK_NODE, NULL, ebufp);
       tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, 0, ebufp);
    tmp_flistp1 = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, 1, ebufp);
       tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, 2, ebufp);	   
	
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_suspend_flist PCM_OP_READ_OBJ input flist", si_flistp);
    PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_suspend_flist PCM_OP_READ_FLDS output flist", so_flistp);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_suspend_flist PCM_OP_READ_FLDS output flist", si_flistpp);
	
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistpp, &so_flistpp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_suspend_flist PCM_OP_READ_FLDS output flist", so_flistpp);
    tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
    ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);

    PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

    if(strcmp(ne, "NONE") == 0)
    {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "No need to generate the prov request ");
    }
    else
    {
    if ( ne && (strstr(ne,"NDS") || strstr(ne, "NAGRA")))
    {
	if (strstr(ne,"NDS"))
	{
        	*act = (char *) malloc(sizeof("SUSPEND_SUBSCRIBER_NDS"));
        	strcpy(*act,"SUSPEND_SUBSCRIBER_NDS");
	}
	else
	{
        	*act = (char *) malloc(sizeof("SUSPEND_SUBSCRIBER_NGR"));
        	strcpy(*act,"SUSPEND_SUBSCRIBER_NGR");
	}
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                    //add cas subscriber id
        cas_subs_id = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, 0, ebufp);

        if ( cas_subs_id && strlen(cas_subs_id) > 0)
        {
            PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_CAS_SUBSCRIBER_ID, cas_subs_id, ebufp);
        }
        else
        {
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    PIN_ERR_BAD_VALUE, MSO_FLD_CAS_SUBSCRIBER_ID, 0, 0);
            goto CLEANUP;
        }

            /***************** TASK 1 **********************/
	task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
	if (strstr(ne,"NAGRA"))
	{
		PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "UNAUTHORIZE_SERVICES", ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "SUSPEND_SUBSCRIBER", ebufp);
        	add_catv_bouquet_id_task(ctxp, i_flistpp, CANCEL_PACKAGE, 1, ebufp);
	}
    }
    else if (ne  && strstr(ne,"CISCO") )
    {
        *act = (char *) malloc(sizeof("SUSPEND_SUBSCRIBER_PK"));
        strcpy(*act,"SUSPEND_SUBSCRIBER_PK");
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );

            //set mac address
        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 2, 0, ebufp);
        PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_NAME, *i_flistpp, MSO_FLD_STB_ID, ebufp);

        /***************** TASK 1 **********************/

        //PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "MODIFY_DHCT_STATE", ebufp);
        //PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_DMS_ENABLED, "DMS_Disabled", ebufp);

        //Changes Added for Suspension Issue which cause damaging the box
       	fm_mso_get_cust_active_plans_ca_ids( ctxp, i_flistpp, error_codep, CATV_SUSPEND, ebufp);

	if (strcmp(ne, "CISCO1") == 0)
	{
		task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 2, ebufp);
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "AUTHORIZE_SERVICES", ebufp);
		pkg_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_ADD_PACKAGE_INFO, 1, ebufp);
		PIN_FLIST_FLD_SET(pkg_flistp, MSO_FLD_CA_ID, "No Channels", ebufp);
	}
    }
	/******************* VERIMATRIX CHANGES - BEGIN ***********************
	 * Creating provisioning order to suspend subscriber for Verimatrix 
	 **********************************************************************/		
	else if ( ne && (strstr(ne,"VM") || strstr(ne,"GOSPEL")))
    {
	if (strstr(ne, "GOSPEL"))
	{
        	*act = (char *) malloc(sizeof("SUSPEND_SUBSCRIBER_GOSPEL"));
        	strcpy(*act,"SUSPEND_SUBSCRIBER_GOSPEL");
	}
	else if (strstr(ne, "JVM"))
	{
        	*act = (char *) malloc(sizeof("SUSPEND_SUBSCRIBER_JVM"));
        	strcpy(*act,"SUSPEND_SUBSCRIBER_JVM");
	}
	else
	{
        	*act = (char *) malloc(sizeof("SUSPEND_SUBSCRIBER_VM"));
        	strcpy(*act,"SUSPEND_SUBSCRIBER_VM");
	}
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
    
		in_flistp  = PIN_FLIST_CREATE(ebufp);
		
	    tempp_flistp = PIN_FLIST_ELEM_GET(so_flistp,PIN_FLD_ALIAS_LIST,PIN_ELEMID_ANY, 1, ebufp);
		
	    PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, in_flistp, PIN_FLD_POID, ebufp);
	    PIN_FLIST_FLD_COPY(tempp_flistp, PIN_FLD_NAME, in_flistp, PIN_FLD_NAME, ebufp);
	    
	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search_serial_number input", in_flistp);
	    
	    search_serial_no(ctxp, in_flistp, &ret_flistp,ebufp);
	    
	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search_serial_number retrun", ret_flistp);
	    
	    
		//VM STB NUMBER
		tempp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);			
		if (strstr(ne, "VM"))
		{
			PIN_FLIST_FLD_COPY(tempp_flistp, PIN_FLD_NAME, *i_flistpp, MSO_FLD_STB_ID, ebufp);
		}
		else
		{
			PIN_FLIST_FLD_COPY(tempp_flistp, PIN_FLD_NAME, *i_flistpp, MSO_FLD_VC_ID, ebufp);
		}

		//VM  serial number
        	tempp_flistp = PIN_FLIST_ELEM_GET(ret_flistp,PIN_FLD_RESULTS,0,0,ebufp);
		if (strstr(ne, "VM"))
		{
        		PIN_FLIST_FLD_COPY(tempp_flistp, MSO_FLD_SERIAL_NO, *i_flistpp, MSO_FLD_VC_ID, ebufp);
		}
		else
		{
        		PIN_FLIST_FLD_COPY(tempp_flistp, MSO_FLD_SERIAL_NO, *i_flistpp, MSO_FLD_STB_ID, ebufp);
		}
	 
		//VM device type
		tempp_flistp = PIN_FLIST_ELEM_GET(ret_flistp,PIN_FLD_RESULTS,0,0,ebufp);
		PIN_FLIST_FLD_COPY(tempp_flistp, PIN_FLD_DEVICE_TYPE, *i_flistpp, PIN_FLD_DEVICE_TYPE, ebufp);

		if (strstr(ne, "VM"))
		{

			input_flistp = PIN_FLIST_CREATE(ebufp);
	    		PIN_FLIST_FLD_COPY(so_flistpp, PIN_FLD_ACCOUNT_OBJ, input_flistp, PIN_FLD_POID, ebufp);
	    		search_name_info(ctxp, input_flistp, &return_flistp,ebufp);
	    
	    		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            			"nameinfo inflistp", input_flistp);
	    
	    		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            			"nameinfo return", return_flistp);

			//Zip code 
			tmp_flistp = PIN_FLIST_ELEM_GET(return_flistp,PIN_FLD_RESULTS,PIN_ELEMID_ANY,0,ebufp);
			name_flistp = PIN_FLIST_ELEM_GET(tmp_flistp,PIN_FLD_NAMEINFO,PIN_ELEMID_ANY,0,ebufp);
			PIN_FLIST_FLD_COPY(name_flistp, PIN_FLD_ZIP, *i_flistpp, PIN_FLD_ZIP, ebufp);

            		/***************** TASK 1 **********************/
        		task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
        		PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "SUSPEND_SUBSCRIBER", ebufp);
		
        		add_catv_bouquet_id_task(ctxp, i_flistpp, CANCEL_PACKAGE, 1, ebufp);
        		
		        if (strstr(ne, "JVM"))
		        {
                    fm_mso_get_cust_active_plans_ca_ids( ctxp, i_flistpp, error_codep, CATV_SUSPEND, ebufp);
                    PIN_ERR_LOG_FLIST(3, "fm_mso_get_cust_active_plans_ca_ids flist in suspend action", *i_flistpp);
                    vp = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_POID, 0, ebufp);
                    fm_mso_get_account_info(ctxp, (poid_t *)vp, &acnt_info, ebufp);
                    vp = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_BUSINESS_TYPE, 0, ebufp);
                    if (vp && vp != NULL)
                    {
                        business_type = *(int32 *)vp;
                        num_to_array(business_type, arr_business_type);
                        lob_exclude_flag = arr_business_type[7];
                        subscriber_type = 10 * (arr_business_type[2]) + arr_business_type[3];
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "business type validation");
                    }
                    if (subscriber_type == 48)
                    {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "before HWIP");
                        PIN_FLIST_FLD_SET(*i_flistpp, PIN_FLD_TYPE_OF_SERVICE, &subscriber_type, ebufp);
                        task_flistp1 = PIN_FLIST_ELEM_GET(*i_flistpp, MSO_FLD_TASK, 3, 0, ebufp);
                        task_namep = PIN_FLIST_FLD_GET(task_flistp1, MSO_FLD_TASK_NAME, 0, ebufp);
                        if (task_namep && strcmp(task_namep, "UNAUTHORIZE_SERVICES") == 0)
                        {
                            while(( result_flistp = PIN_FLIST_ELEM_GET_NEXT(task_flistp1, MSO_FLD_DEL_PACKAGE_INFO,
                                                       &b_elem_id, 1, &b_cookie, ebufp)) != (pin_flist_t *)NULL)
                            {
                                ca_id = PIN_FLIST_FLD_GET(result_flistp, MSO_FLD_CA_ID, 0, ebufp);
                                memset(new_ca_id,'\0',sizeof(new_ca_id));
                                strcpy(new_ca_id, ca_id);

                                if(lob_exclude_flag < 2)
                                 {
                                     strcat(new_ca_id, "_HW_IP");
                                 }
                                 PIN_FLIST_FLD_SET(result_flistp, MSO_FLD_CA_ID, new_ca_id, ebufp);
                            }
                        }
                    }
                }
		}
		else
		{
        
			//Changes Added for Suspension Issue which cause damaging the box
        		fm_mso_get_cust_active_plans_ca_ids( ctxp, i_flistpp, error_codep, CATV_SUSPEND, ebufp);
			//add_package_detail(ctxp, i_flistpp, CANCEL_PACKAGE, ne, 0, ebufp);
                
			task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 2, ebufp);
                	PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "DEACTIVATE_CARD", ebufp);
        		PIN_FLIST_FLD_COPY(*i_flistpp, MSO_FLD_VC_ID, task_flistp, MSO_FLD_VC_ID, ebufp);

			now_t = pin_virtual_time((time_t *)NULL);
			current_time = localtime(&now_t);
			year = current_time->tm_year + 1900;
			month = current_time->tm_mon +1;
			day = current_time->tm_mday;

			memset(validity,'\0',21);
			sprintf(validity,"%04d-%02d-%02dT00:00:00",year,month,day);	
            PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_EXPIRATION_DATE, validity, ebufp);
		}
        if(strstr(ne, "VM"))
        {
            in_flistp = PIN_FLIST_CREATE(ebufp);
            PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, in_flistp, PIN_FLD_POID, ebufp);
            PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, in_flistp, PIN_FLD_OBJECT, ebufp);
            PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_PROFILE_DESCR, CUSTOMER_CARE, ebufp);
            PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_SEARCH_TYPE, &profile_srch_type, ebufp);

            fm_mso_get_profile_info(ctxp, in_flistp, &return_flistp, ebufp);
            PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
            input_flistp = PIN_FLIST_SUBSTR_GET(return_flistp, PIN_FLD_CUSTOMER_CARE_INFO, 0, ebufp);
            PIN_FLIST_FLD_COPY(input_flistp, MSO_FLD_LCO_OBJ, *i_flistpp, MSO_FLD_LCO_OBJ, ebufp);
            task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 4, ebufp);
            PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "AUTHORIZE_LBO_SERVICES", ebufp);
            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                                "fm_mso_populate_bouquet_id input flist", *i_flistpp);
            fm_mso_populate_bouquet_id(ctxp, *i_flistpp, device_hd_flag, 0, &ret_flistp, ebufp);
            PIN_FLIST_FLD_COPY(ret_flistp, PIN_FLD_VALIDITY_IN_DAYS,
                                    task_flistp, PIN_FLD_VALIDITY_IN_DAYS, ebufp);
            PIN_FLIST_FLD_COPY(ret_flistp, PIN_FLD_VALID_DOM,
                                    task_flistp, PIN_FLD_VALID_DOM, ebufp);
            PIN_FLIST_FLD_COPY(ret_flistp, PIN_FLD_TIMEZONE,
                                    task_flistp, PIN_FLD_TIMEZONE, ebufp);
            PIN_FLIST_FLD_COPY(ret_flistp, PIN_FLD_SPEED,
                                    task_flistp, PIN_FLD_SPEED, ebufp);
            now_t = pin_virtual_time((time_t *)NULL);
            now_t = now_t - 86400;
            current_time = localtime(&now_t);
            year = current_time->tm_year + 1900;
            month = current_time->tm_mon +1;
            day = current_time->tm_mday;
            memset(validity,'\0',21);
            sprintf(validity,"%04d%02d%02d",year,month,day);
            PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_EXPIRYDATE, validity, ebufp);


        }

    }
	/******************* VERIMATRIX CHANGES - END ***********************
	 * Creating provisioning order to suspend subscriber for Verimatrix 
	 **********************************************************************/
    else
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
        goto CLEANUP;
    }
    }

    /*
     * Enrich with additional details required by Provisioning system
     */

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_catv_suspend_flist enriched input flist", *i_flistpp);

CLEANUP:
    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
   // PIN_FLIST_DESTROY_EX(&si_flistpp, NULL);
    PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
   // PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
  //  PIN_FLIST_DESTROY_EX(&name_flistp, NULL);

    return;

}

/*******************************************************************
 * prepare_catv_reactivate_flist()
 *
 * Enrich input filst with service details required
 * optionally performa validation on input fields
 *******************************************************************/
static void
prepare_catv_reactivate_flist(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    int32                   *error_codep,
    char                    **act,
    pin_errbuf_t            *ebufp)
{

    pin_flist_t     *si_flistp = NULL;
    pin_flist_t     *so_flistp = NULL;
    pin_flist_t     *tmp_flistp = NULL;
    pin_flist_t     *tmp_flistp1 = NULL;
    pin_flist_t     *sub_flistp = NULL;
    pin_flist_t     *pkg_flistp = NULL;
    void            *vp = NULL;
    pin_flist_t     *task_flistp = NULL;
    char            *ne = NULL;
    char            *cas_subs_id = NULL;
	pin_flist_t		*in_flistp = NULL;
	pin_flist_t		*tempp_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;
    int32           pkg_type = 0;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_reactivate_flist input flist", *i_flistpp);

    /*
     * Validate input received
     */
    // There should be at least one product to be added
//    tmp_flistp = PIN_FLIST_ELEM_GET(*i_flistpp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, 0, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

	if ( (PIN_FLIST_ELEM_COUNT(*i_flistpp, PIN_FLD_PRODUCTS, ebufp)) > 200 )
	{
            	pin_set_err(ebufp, PIN_ERRLOC_FM,
                	PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    	PIN_ERR_PERF_LIMIT_REACHED, PIN_FLD_PRODUCTS, 0, 0);
            goto CLEANUP;
	}
    /*
     * Enrich with additional service details
     */
    si_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);
    tmp_flistp = PIN_FLIST_SUBSTR_ADD(si_flistp, MSO_FLD_CATV_INFO, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, NULL, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_NETWORK_NODE, NULL, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, 0, ebufp);
    tmp_flistp1 = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, 1, ebufp);
    tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, 2, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_reactivate_flist PCM_OP_READ_OBJ input flist", si_flistp);
    PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_reactivate_flist PCM_OP_READ_FLDS output flist", so_flistp);

    tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
    ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);

    PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

    if(strcmp(ne, "NONE") == 0)
    {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "No need to generate the prov request ");
    }
    else
    {
    if ( ne && strstr(ne,"NDS") || strstr(ne,"NAGRA"))
    {
	if (strstr(ne,"NDS"))
	{
        	*act = (char *) malloc(sizeof("REACTIVATE_SERVICE_NDS"));
        	strcpy(*act,"REACTIVATE_SERVICE_NDS");
	}
	else
	{
        	*act = (char *) malloc(sizeof("REACTIVATE_SUBSCRIBER_NGR"));
        	strcpy(*act,"REACTIVATE_SUBSCRIBER_NGR");
	}
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                    //add cas subscriber id
        cas_subs_id = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, 0, ebufp);

        if ( cas_subs_id && strlen(cas_subs_id) > 0)
        {
            PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_CAS_SUBSCRIBER_ID, cas_subs_id, ebufp);
        }
        else
        {
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    PIN_ERR_BAD_VALUE, MSO_FLD_CAS_SUBSCRIBER_ID, 0, 0);
            goto CLEANUP;
        }

            /***************** TASK 1 **********************/
        if ( strstr(ne, "NDS") )
        {
        	task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
        	PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "REACTIVATE_SUBSCRIBER", ebufp);
		if ( (PIN_FLIST_ELEM_COUNT(*i_flistpp, PIN_FLD_PRODUCTS, ebufp)) >0 )
		{
                	add_package_detail(ctxp, i_flistpp, ADD_PACKAGE, ne, 1, ebufp);
		}
        }
	else if (strstr(ne, "NAGRA"))
	{
                add_package_detail(ctxp, i_flistpp, ADD_PACKAGE, ne, 1, ebufp);
	}
	else
	{
		add_catv_bouquet_id_task(ctxp, i_flistpp, ADD_PACKAGE, 1, ebufp);
	}

    }
    else if (ne  && strstr(ne,"CISCO") )
    {
        *act = (char *) malloc(sizeof("REACTIVATE_SERVICE_PK"));
        strcpy(*act,"REACTIVATE_SERVICE_PK");
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );


            //set mac address
        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 2, 0, ebufp);
        PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_NAME, *i_flistpp, MSO_FLD_STB_ID, ebufp);

        /***************** TASK 1 **********************/
        //Changes Added for Suspension Issue which cause damaging the box
        //task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
        //PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "UNAUTHORIZE_SERVICES", ebufp);
        //pkg_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_DEL_PACKAGE_INFO, 1, ebufp);
        //PIN_FLIST_FLD_SET (pkg_flistp, MSO_FLD_CA_ID, "No Channels", ebufp);

        fm_mso_get_cust_active_plans_ca_ids( ctxp, i_flistpp, error_codep, CATV_REACTIVATE, ebufp);

	if (strcmp(ne, "CISCO1") == 0)
	{
        	task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 2, ebufp);
        	PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "UNAUTHORIZE_SERVICES", ebufp);
        	pkg_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_DEL_PACKAGE_INFO, 1, ebufp);
        	PIN_FLIST_FLD_SET (pkg_flistp, MSO_FLD_CA_ID, "No Channels", ebufp);
	}
        //task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
        //PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "MODIFY_DHCT_STATE", ebufp);
        //PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_DMS_ENABLED, "DMS_Enabled", ebufp);

    }
	/******************* VERIMATRIX CHANGES - BEGIN ***********************
	 * Creating provisioning order to reactivate service for Verimatrix 
	 **********************************************************************/	
	else if ( ne && (strstr(ne,"VM") || strstr(ne, "GOSPEL")))
    {
	if (strstr(ne, "GOSPEL"))
	{
        	*act = (char *) malloc(sizeof("REACTIVATE_SERVICE_GOSPEL"));
        	strcpy(*act,"REACTIVATE_SERVICE_GOSPEL");
	}
	else if (strstr(ne, "JVM"))
	{
        	*act = (char *) malloc(sizeof("REACTIVATE_SERVICE_JVM"));
        	strcpy(*act,"REACTIVATE_SERVICE_JVM");
	}
	else
	{
        	*act = (char *) malloc(sizeof("REACTIVATE_SERVICE_VM"));
        	strcpy(*act,"REACTIVATE_SERVICE_VM");
	}
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
       
	    in_flistp  = PIN_FLIST_CREATE(ebufp);
	    tempp_flistp = PIN_FLIST_ELEM_GET(so_flistp,PIN_FLD_ALIAS_LIST,0, 1, ebufp);
	    PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, in_flistp, PIN_FLD_POID, ebufp);
	    PIN_FLIST_FLD_COPY(tempp_flistp, PIN_FLD_NAME, in_flistp, PIN_FLD_NAME, ebufp);
	    
	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"s_no input flist", in_flistp);
	    
	    search_serial_no(ctxp, in_flistp, &ret_flistp,ebufp);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"s_no output flist", ret_flistp);
		
		//VM STB NUMBER
		tempp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);			
		if (strstr(ne, "VM"))
		{
			PIN_FLIST_FLD_COPY(tempp_flistp, PIN_FLD_NAME, *i_flistpp, MSO_FLD_STB_ID, ebufp);
		}
		else
		{
			PIN_FLIST_FLD_COPY(tempp_flistp, PIN_FLD_NAME, *i_flistpp, MSO_FLD_VC_ID, ebufp);
		}

		//VM  serial number
		tempp_flistp = PIN_FLIST_ELEM_GET(ret_flistp,PIN_FLD_RESULTS,0,0,ebufp);
		if (strstr(ne, "VM"))
		{
			PIN_FLIST_FLD_COPY(tempp_flistp, MSO_FLD_SERIAL_NO, *i_flistpp, MSO_FLD_VC_ID, ebufp);
		}
		else
		{
			PIN_FLIST_FLD_COPY(tempp_flistp, MSO_FLD_SERIAL_NO, *i_flistpp, MSO_FLD_STB_ID, ebufp);
		}
	 
		//VM device type
		tempp_flistp = PIN_FLIST_ELEM_GET(ret_flistp,PIN_FLD_RESULTS,0,0,ebufp);
		PIN_FLIST_FLD_COPY(tempp_flistp, PIN_FLD_DEVICE_TYPE, *i_flistpp, PIN_FLD_DEVICE_TYPE, ebufp);

		PIN_FLIST_FLD_COPY(tempp_flistp, PIN_FLD_ACCOUNT_OBJ, *i_flistpp, MSO_FLD_LCO_OBJ, ebufp);
            /***************** TASK 1 **********************/
        	task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
	if (strstr(ne, "GOSPEL"))
	{
        	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "ACTIVATE_CARD", ebufp);
		PIN_FLIST_FLD_COPY(*i_flistpp, MSO_FLD_VC_ID, task_flistp, MSO_FLD_VC_ID, ebufp);
	}
	else
	{
        	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "REACTIVATE_SUBSCRIBER", ebufp);
	}
				
        if ( (PIN_FLIST_ELEM_COUNT(*i_flistpp, PIN_FLD_PRODUCTS, ebufp)) >0 )
        {
                PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_PKG_TYPE, &pkg_type, ebufp);
                add_package_detail(ctxp, i_flistpp, ADD_PACKAGE, ne, 1, ebufp);
        }
	else
	{
		add_catv_bouquet_id_task(ctxp, i_flistpp, ADD_PACKAGE, 1, ebufp);
	}

    }
	/******************* VERIMATRIX CHANGES - END ***********************
	 * Creating provisioning order to reactivate service for Verimatrix 
	 **********************************************************************/
    else
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
        goto CLEANUP;
    }
    }

    /*
     * Enrich with additional details required by Provisioning system
     */


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_catv_reactivate_flist enriched input flist", *i_flistpp);

CLEANUP:
    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
  //  PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
    return;

}

/*******************************************************************
 * prepare_catv_terminate_flist()
 *
 * Enrich input filst with service details required
 * optionally performa validation on input fields
 *******************************************************************/
static void
prepare_catv_terminate_flist(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    char                    **act,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t     *si_flistp = NULL;
    pin_flist_t     *so_flistp = NULL;
    pin_flist_t     *tmp_flistp = NULL;
    pin_flist_t     *tmp_flistp1 = NULL;
    pin_flist_t     *task_flistp = NULL;
    char            *ne = NULL;
    char            *cas_subs_id = NULL;
    char		*old_stbp = NULL;
	char		*bouquet_idp = NULL;
	pin_flist_t		*in_flistp = NULL;
	pin_flist_t		*tempp_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;
    pin_flist_t     *return_flistp = NULL;
    pin_flist_t     *input_flistp = NULL;
	int32		*error_codep = NULL;
	struct		tm *current_time;
	time_t		now_t = 0;
	int32		year;
	int32		month;
	int32		day;
    int32       profile_srch_type = MSO_FLAG_SRCH_BY_SELF_ACCOUNT;
    int32       device_hd_flag = 0;
    int32           *old_statep = NULL;
	char		validity[21];

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;
    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_terminate_flist input flist", *i_flistpp);

    /*
     * Validate input received
     */

    /*
     * Enrich with additional service details
     */
    si_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);
    tmp_flistp = PIN_FLIST_SUBSTR_ADD(si_flistp, MSO_FLD_CATV_INFO, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, NULL, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_NETWORK_NODE, NULL, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_BOUQUET_ID, NULL, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, 0, ebufp);
    tmp_flistp1 = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, 1, ebufp);
    tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, 2, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_terminate_flist PCM_OP_READ_OBJ input flist", si_flistp);
    PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_terminate_flist PCM_OP_READ_FLDS output flist", so_flistp);

    tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
    ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);
    bouquet_idp = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_BOUQUET_ID, 0, ebufp);
    PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);
    if(strcmp(ne, "NONE") == 0)
    {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "No need to generate the prov request ");
    }
    else
    {

        old_stbp = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_OLD_VALUE, 1, ebufp);
    if ( ne && (strstr(ne,"NDS") || strstr(ne,"NAGRA")))
    {
	if (strstr(ne, "NDS"))
	{
        	*act = (char *) malloc(sizeof("TERMINATE_SUBSCRIBER_NDS"));
        	strcpy(*act,"TERMINATE_SUBSCRIBER_NDS");
	}
	else
	{
        	*act = (char *) malloc(sizeof("TERMINATE_SUBSCRIBER_NGR"));
        	strcpy(*act,"TERMINATE_SUBSCRIBER_NGR");
	}	
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                    //add cas subscriber id
        cas_subs_id = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, 0, ebufp);

        if ( cas_subs_id && strlen(cas_subs_id) > 0)
        {
            PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_CAS_SUBSCRIBER_ID, cas_subs_id, ebufp);
        }
        else
        {
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    PIN_ERR_BAD_VALUE, MSO_FLD_CAS_SUBSCRIBER_ID, 0, 0);
            goto CLEANUP;
        }

            /***************** TASK 1 **********************/
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
	if (strstr(ne,"NAGRA"))
	{
		PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "UNAUTHORIZE_SERVICES", ebufp);
	}
	else
	{
        	PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "DELETE_SUBSCRIBER", ebufp);
	}

    }
    else if (ne  && strstr(ne,"CISCO") )
    {
        *act = (char *) malloc(sizeof("TERMINATE_SUBSCRIBER_PK"));
       	strcpy(*act,"TERMINATE_SUBSCRIBER_PK");
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );

            //set mac address
	if (strstr(ne,"CISCO"))
	{
        	tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 2, 1, ebufp);
	}
	else
	{
        	tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 1, ebufp);
	}
	if (old_stbp)
	{
		PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_STB_ID, old_stbp, ebufp);
	}
	else
	{
        	PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_NAME, *i_flistpp, MSO_FLD_STB_ID, ebufp);
	}

            /***************** TASK 1 **********************/
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "MODIFY_DHCT_ADMIN_STATUS", ebufp);
            /*
             * set admin status - required for CISCO PK
             * value is always DAS_InServiceOneWay
             */
        PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_ADMIN_STATUS, "DAS_OutOfService", ebufp);

            /***************** TASK 2 **********************/
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 2, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "MODIFY_DHCT_CONFIG", ebufp);
        PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_DMS_ENABLED, "DMS_Disabled", ebufp);
        // call function to add package info
        add_package_detail(ctxp, i_flistpp, CANCEL_PACKAGE, ne, 0, ebufp);

            /***************** TASK 3 **********************/
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 3, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "INSTANT_HIT", ebufp);

            /***************** TASK 4 **********************/
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 4, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "SET_BOUQUET_ID", ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_BOUQUET_ID, bouquet_idp, ebufp);

    }
	/******************* VERIMATRIX CHANGES - BEGIN ***********************
	 * Creating provisioning order to terminate subscriber for Verimatrix 
	 **********************************************************************/		
	else if ( ne && (strstr(ne,"VM") || strstr(ne, "GOSPEL")))
    {
	if (strstr(ne, "GOSPEL"))
	{
        	*act = (char *) malloc(sizeof("TERMINATE_SUBSCRIBER_GOSPEL"));
        	strcpy(*act,"TERMINATE_SUBSCRIBER_GOSPEL");
	}
	else if (strstr(ne, "JVM"))
	{
        	*act = (char *) malloc(sizeof("TERMINATE_SUBSCRIBER_JVM"));
        	strcpy(*act,"TERMINATE_SUBSCRIBER_JVM");
	}
	else
	{
        	*act = (char *) malloc(sizeof("TERMINATE_SUBSCRIBER_VM"));
        	strcpy(*act,"TERMINATE_SUBSCRIBER_VM");
	}
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
    	in_flistp  = PIN_FLIST_CREATE(ebufp);
	tempp_flistp = PIN_FLIST_ELEM_GET(so_flistp,PIN_FLD_ALIAS_LIST,0, 1, ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, in_flistp, PIN_FLD_POID, ebufp);
		if (old_stbp)
		{
			PIN_FLIST_FLD_SET(*i_flistpp, PIN_FLD_NAME, old_stbp, ebufp);
			PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_NAME, old_stbp, ebufp);
			if (strstr(ne, "VM"))
			{
				PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_STB_ID, old_stbp, ebufp);
			}
			else
			{
				PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_VC_ID, old_stbp, ebufp);
			}
		}
		else
		{
			PIN_FLIST_FLD_COPY(tempp_flistp, PIN_FLD_NAME, in_flistp, PIN_FLD_NAME, ebufp);
			tempp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);			
			if (strstr(ne, "VM"))
			{
				PIN_FLIST_FLD_COPY(tempp_flistp, PIN_FLD_NAME, *i_flistpp, MSO_FLD_STB_ID, ebufp);
			}
			else
			{
				PIN_FLIST_FLD_COPY(tempp_flistp, PIN_FLD_NAME, *i_flistpp, MSO_FLD_VC_ID, ebufp);
			}
		}
	    
	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"s_no input flist", in_flistp);
	    
	    search_serial_no(ctxp, in_flistp, &ret_flistp,ebufp);
	    
	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"s_no output flist", ret_flistp);
		
		//VM  serial number
		tempp_flistp = PIN_FLIST_ELEM_GET(ret_flistp,PIN_FLD_RESULTS,0,0,ebufp);
		if (strstr(ne, "VM"))
		{
			PIN_FLIST_FLD_COPY(tempp_flistp, MSO_FLD_SERIAL_NO, *i_flistpp, MSO_FLD_VC_ID, ebufp);
		}
		else
		{
			PIN_FLIST_FLD_COPY(tempp_flistp, MSO_FLD_SERIAL_NO, *i_flistpp, MSO_FLD_STB_ID, ebufp);
		}
	 
		//VM device type
		tempp_flistp = PIN_FLIST_ELEM_GET(ret_flistp,PIN_FLD_RESULTS,0,0,ebufp);
		PIN_FLIST_FLD_COPY(tempp_flistp, PIN_FLD_DEVICE_TYPE, *i_flistpp, PIN_FLD_DEVICE_TYPE, ebufp);
            /***************** TASK 1 **********************/
	if (strstr(ne, "GOSPEL"))
	{
//        	fm_mso_get_cust_active_plans_ca_ids( ctxp, i_flistpp, error_codep, CATV_SUSPEND, ebufp);
        
        	add_package_detail(ctxp, i_flistpp, CANCEL_PACKAGE, ne, 0, ebufp);

		task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 2, ebufp);
		PIN_FLIST_FLD_COPY(*i_flistpp, MSO_FLD_VC_ID, task_flistp, MSO_FLD_VC_ID, ebufp);
        	PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "DEACTIVATE_CARD", ebufp);
        
		now_t = pin_virtual_time((time_t *)NULL);
		current_time = localtime(&now_t);
		year = current_time->tm_year + 1900;
		month = current_time->tm_mon +1;
		day = current_time->tm_mday;

		memset(validity,'\0',21);
		sprintf(validity,"%04d-%02d-%02dT00:00:00",year,month,day);	
                PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_EXPIRATION_DATE, validity, ebufp);
	}

	else if(strstr(ne, "JVM"))
    {
      //pavan
      error_codep = 0;
      task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
      PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "SUSPEND_SUBSCRIBER", ebufp);
      add_catv_bouquet_id_task(ctxp, i_flistpp, CANCEL_PACKAGE, 1, ebufp);
      //add_package_detail(ctxp, i_flistpp, CANCEL_PACKAGE, ne, 0, ebufp);
      old_statep = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_DELETE_STATUS, 1, ebufp);
      if (old_statep && *old_statep != 10102)
      {
        fm_mso_get_cust_active_plans_ca_ids( ctxp, i_flistpp, error_codep, CATV_TERMINATE, ebufp);
      }

      in_flistp = PIN_FLIST_CREATE(ebufp);
      PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, in_flistp, PIN_FLD_POID, ebufp);
      PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, in_flistp, PIN_FLD_OBJECT, ebufp);
      PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_PROFILE_DESCR, CUSTOMER_CARE, ebufp);
      PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_SEARCH_TYPE, &profile_srch_type, ebufp);

      fm_mso_get_profile_info(ctxp, in_flistp, &return_flistp, ebufp);
      PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
      input_flistp = PIN_FLIST_SUBSTR_GET(return_flistp, PIN_FLD_CUSTOMER_CARE_INFO, 0, ebufp);
      PIN_FLIST_FLD_COPY(input_flistp, MSO_FLD_LCO_OBJ, *i_flistpp, MSO_FLD_LCO_OBJ, ebufp);
      task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 4, ebufp);
      PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "AUTHORIZE_LBO_SERVICES", ebufp);
      PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                          "fm_mso_populate_bouquet_id input flist", *i_flistpp);
      fm_mso_populate_bouquet_id(ctxp, *i_flistpp, device_hd_flag, 0, &ret_flistp, ebufp);
      PIN_FLIST_FLD_COPY(ret_flistp, PIN_FLD_VALIDITY_IN_DAYS,
                              task_flistp, PIN_FLD_VALIDITY_IN_DAYS, ebufp);
      PIN_FLIST_FLD_COPY(ret_flistp, PIN_FLD_VALID_DOM,
                              task_flistp, PIN_FLD_VALID_DOM, ebufp);
      PIN_FLIST_FLD_COPY(ret_flistp, PIN_FLD_TIMEZONE,
                              task_flistp, PIN_FLD_TIMEZONE, ebufp);
      PIN_FLIST_FLD_COPY(ret_flistp, PIN_FLD_SPEED,
                              task_flistp, PIN_FLD_SPEED, ebufp);
      now_t = pin_virtual_time((time_t *)NULL);
      now_t = now_t - 86400;
      current_time = localtime(&now_t);
      year = current_time->tm_year + 1900;
      month = current_time->tm_mon +1;
      day = current_time->tm_mday;
      memset(validity,'\0',21);
      sprintf(validity,"%04d%02d%02d",year,month,day);
      PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_EXPIRYDATE, validity, ebufp);


    }
    else
    {
        	task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
        	PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "DELETE_SUBSCRIBER", ebufp);
	}

    }
	/******************* VERIMATRIX CHANGES - END ***********************
	 * Creating provisioning order to terminate subscriber for Verimatrix 
	 **********************************************************************/
    else
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
        goto CLEANUP;
    }
	if (old_stbp)
	{
		PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_OLD_VALUE, ebufp); 
	}
    }

    /*
     * Enrich with additional details required by Provisioning system
     */


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_catv_terminate_flist enriched input flist", *i_flistpp);

CLEANUP:
    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&in_flistp, NULL);

    return;

}

/*******************************************************************
 * prepare_catv_change_smart_card_flist()
 *
 * Enrich input filst with service details required
 * optionally performa validation on input fields
 *******************************************************************/
static void
prepare_catv_change_smart_card_flist(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    char                    **act,
    pin_errbuf_t            *ebufp)
{

    pin_flist_t     *si_flistp = NULL;
    pin_flist_t     *so_flistp = NULL;
    pin_flist_t     *tmp_flistp = NULL;
    char            *vc_id = NULL;
    pin_flist_t     *task_flistp = NULL;
    char            *ne = NULL;
    char            *cas_subs_id = NULL;
    char            *device_type = NULL;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_change_smart_card_flist input flist", *i_flistpp);

    /*
     * Validate input received
     */
            // Check device is supplied in the input
    vc_id = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_VC_ID, 0, ebufp);
    device_type = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_DEVICE_TYPE, 0, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "prepare_catv_change_smart_card_flist error: required field missing in input flist", ebufp);
        return;
    }

    if (strlen(vc_id) != 12 )
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    PIN_ERR_BAD_VALUE, MSO_FLD_VC_ID, 0, 0);
            return;
    }
    /*
     * Enrich with additional service details
     */
    si_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);
    tmp_flistp = PIN_FLIST_SUBSTR_ADD(si_flistp, MSO_FLD_CATV_INFO, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, NULL, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_NETWORK_NODE, NULL, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_change_smart_card_flist PCM_OP_READ_OBJ input flist", si_flistp);
    PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_change_smart_card_flist PCM_OP_READ_FLDS output flist", so_flistp);

    tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
    ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);

    PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

    if(strcmp(ne, "NONE") == 0)
    {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "No need to generate the prov request ");
    }
    else
    {

    if ( ne && strstr(ne,"NDS") )
    {
        *act = (char *) malloc(sizeof("CHANGE_SMC_NDS"));
        strcpy(*act,"CHANGE_SMC_NDS");
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                    //add cas subscriber id
        cas_subs_id = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, 0, ebufp);

        if ( cas_subs_id && strlen(cas_subs_id) > 0)
        {
            PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_CAS_SUBSCRIBER_ID, cas_subs_id, ebufp);
        }
        else
        {
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    PIN_ERR_BAD_VALUE, MSO_FLD_CAS_SUBSCRIBER_ID, 0, 0);
            goto CLEANUP;
        }

            /***************** TASK 1 **********************/
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "CHANGE_SMC", ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_VC_ID, vc_id, ebufp);
                PIN_FLIST_FLD_SET (task_flistp, PIN_FLD_DEVICE_TYPE, device_type, ebufp);
        PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_VC_ID, ebufp);

    }
//    else if (ne  && strstr(ne,"CISCO") == 0)
//    {
//        *act = (char *) malloc(sizeof("CHANGE_SMC_PK"));
//        strcpy(*act,"CHANGE_SMC_PK");
//        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
//    }
    else
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
        goto CLEANUP;
    }
    }

    /*
     * Enrich with additional details required by Provisioning system
     */


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_catv_change_smart_card_flist enriched input flist", *i_flistpp);

CLEANUP:
    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&so_flistp, NULL);

    return;

}

/*******************************************************************
 * prepare_catv_change_STB_flist()
 *
 * Enrich input filst with service details required
 * optionally performa validation on input fields
 *******************************************************************/
static void
prepare_catv_change_STB_flist(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    char                    **act,
    pin_errbuf_t            *ebufp)
{

    pin_flist_t     *si_flistp = NULL;
    pin_flist_t     *so_flistp = NULL;
    pin_flist_t     *tmp_flistp = NULL;
    pin_flist_t     *catvinfo_flistp = NULL;
    pin_flist_t     *r_flistp = NULL;
    char            *stb_id = NULL;
    pin_flist_t     *task_flistp = NULL;
    char            *ne = NULL;
    char            *cas_subs_id = NULL;
    char            *region_key = NULL;
    char            *population_id = "0001"; //this is the only expected value in downstream system
    char	    *pop_idp = NULL;
    char            *device_type = NULL;
    char            *vc_id = NULL;
    char			*stb_vm_id = NULL;
	char			*sl_no = NULL;
	pin_flist_t		*device_flistp = NULL;
	pin_flist_t		*in_flistp = NULL;
	pin_flist_t		*tempp_flistp = NULL;
	pin_flist_t		*tmpp_flistp = NULL;
	pin_flist_t		*ret_flistpp = NULL;
	pin_flist_t		*ret_flistp = NULL;
	char				template [] = "select X from /purchased_product where F1 = V1 and F2 = V2";
    pin_flist_t		    *srch_flistp = NULL;
    pin_flist_t			*arg_flist = NULL;
    pin_flist_t			*srch_outflistp = NULL;
    pin_flist_t			*result_flistp = NULL;
    poid_t				*pdp= NULL;
    int64                db =0;
    int32			srch_flag = 256;
	int32		device_hd_flag = 0;
	pin_cookie_t    cookie = NULL;
	int32			rec_id = 0;
	int32			id=0;
	int32			status=1;
	pin_flist_t		*prod_flistp = NULL;
	pin_flist_t		*pro_flistp = NULL;
	char            *bouquet_id = NULL;
	pin_flist_t		*return_flistp = NULL;
	pin_flist_t		*input_flistp = NULL;
	pin_flist_t		*name_flistp = NULL;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_change_STB_flist input flist", *i_flistpp);

    /*
     * Validate input received
     */
            // Check device is supplied in the input
    stb_id = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_STB_ID, 0, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "prepare_catv_change_STB_flist error: required field missing in input flist", ebufp);
        return;
    }

    if (strlen(stb_id) == 0 )
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    PIN_ERR_BAD_VALUE, MSO_FLD_VC_ID, 0, 0);
            return;
    }
    /*
     * Enrich with additional service details
     */
    si_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);
    /* Sachid: Changing READ_FLDS input to READ_OBJ */
    //tmp_flistp = PIN_FLIST_SUBSTR_ADD(si_flistp, MSO_FLD_CATV_INFO, ebufp);
    //PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, NULL, ebufp);
    //PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_NETWORK_NODE, NULL, ebufp);
    //PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_REGION_KEY, NULL, ebufp);
    //tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, 1, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_change_STB_flist PCM_OP_READ_OBJ input flist", si_flistp);
        //"prepare_catv_change_STB_flist PCM_OP_READ_FLDS input flist", si_flistp);
    PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
    //PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_change_STB_flist PCM_OP_READ_OBJ output flist", so_flistp);
        //"prepare_catv_change_STB_flist PCM_OP_READ_FLDS output flist", so_flistp);

    //Sachid: Changed tmp_flistp to catvinfo_flistp
    //tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
    catvinfo_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
    ne = PIN_FLIST_FLD_GET(catvinfo_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);
	bouquet_id = PIN_FLIST_FLD_GET(catvinfo_flistp, MSO_FLD_BOUQUET_ID, 0, ebufp);
	cas_subs_id = PIN_FLIST_FLD_GET(catvinfo_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, 1, ebufp);
	region_key = PIN_FLIST_FLD_GET(catvinfo_flistp, MSO_FLD_REGION_KEY, 0, ebufp);
    PIN_FLIST_FLD_COPY(catvinfo_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

    //Sachid: Adding error check in case mandatory fields are missing
    if(PIN_ERRBUF_IS_ERR(ebufp))
        goto CLEANUP;

    if(strcmp(ne, "NONE") == 0)
    {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "No need to generate the prov request ");
    }
    else
    {
		if ( ne && strstr(ne,"NDS") )
		{
			PIN_FLIST_DESTROY_EX(&si_flistp, NULL);

			si_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, si_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, si_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
			PIN_FLIST_FLD_COPY(*i_flistpp, MSO_FLD_LCO_OBJ, si_flistp, MSO_FLD_LCO_OBJ, ebufp);
			PIN_FLIST_FLD_SET(si_flistp, MSO_FLD_NETWORK_NODE, ne, ebufp);	
			search_name_info(ctxp, si_flistp, &return_flistp, ebufp);

			tmp_flistp = PIN_FLIST_ELEM_GET(return_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
			PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_AREA, si_flistp, MSO_FLD_AREA, ebufp);
			PIN_FLIST_ELEM_SET(si_flistp, so_flistp, PIN_FLD_SERVICES, 0, ebufp);

			fm_mso_populate_bouquet_id(ctxp, si_flistp, device_hd_flag, 0, &r_flistp, ebufp);

			PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_LCO_OBJ, ebufp);

    			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        			"prepare_catv_change_STB_flist bouquet ID oflist", si_flistp);

			pop_idp = PIN_FLIST_FLD_TAKE(si_flistp, MSO_FLD_POPULATION_ID, 1, ebufp);

			*act = (char *) malloc(sizeof("CHANGE_STB_NDS"));
			strcpy(*act,"CHANGE_STB_NDS");
			//Sachid: Get cas subscriber id for NDS devices only
			//tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 1, ebufp);
			//cas_subs_id = PIN_FLIST_FLD_GET(catvinfo_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, 1, ebufp);
			if ( cas_subs_id && strlen(cas_subs_id) > 0) {
				PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_CAS_SUBSCRIBER_ID, cas_subs_id, ebufp);
			}
			else {
				pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
						PIN_ERR_BAD_VALUE, MSO_FLD_CAS_SUBSCRIBER_ID, 0, 0);
				goto CLEANUP;
			}
    			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        			"prepare_catv_change_STB_flist alias", so_flistp);

			tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST,1, 1, ebufp);
		}
		else if (ne  && strstr(ne,"CISCO") ) {
				*act = (char *) malloc(sizeof("CHANGE_STB_PK"));
				strcpy(*act,"CHANGE_STB_PK");
				tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST,2, 1, ebufp);
		}
		/******************* VERIMATRIX CHANGES - BEGIN ***********************
		 * Creating provisioning order to change STB for Verimatrix 
		 **********************************************************************/				
		else if ( ne && strstr(ne,"VM") )
		{
			if (strstr(ne, "JVM"))
			{
				*act = (char *) malloc(sizeof("CHANGE_STB_JVM"));
				strcpy(*act,"CHANGE_STB_JVM");
			}
			else
			{
				*act = (char *) malloc(sizeof("CHANGE_STB_VM"));
				strcpy(*act,"CHANGE_STB_VM");
			}
	   
			/*search tempalte from product search*/
			 
			srch_flistp = PIN_FLIST_CREATE(ebufp);
			pdp = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_SERVICE_OBJ, 0 ,ebufp);
			db = PIN_POID_GET_DB(pdp);
		   
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "hp006");
			PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
			PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
		   
			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp);
			PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, arg_flist, PIN_FLD_SERVICE_OBJ, ebufp);
		   
			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp);
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STATUS, &status , ebufp);
		   
			result_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PURCHASE_END_T, NULL, ebufp);			
		   
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
						"product inflist", srch_flistp);
		   
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_outflistp, ebufp);
		   
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
						"product return flist", srch_outflistp);
						
			while ( (prod_flistp = PIN_FLIST_ELEM_GET_NEXT (srch_outflistp,PIN_FLD_RESULTS, &rec_id, 1,&cookie, ebufp)) != (pin_flist_t *)NULL )		   
			{
			   pro_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, PIN_FLD_PRODUCTS,id, ebufp);
			   PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_POID, pro_flistp, PIN_FLD_POID, ebufp);
			   PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_PRODUCT_OBJ, pro_flistp, PIN_FLD_PRODUCT_OBJ, ebufp);
			   PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_PURCHASE_END_T, pro_flistp, PIN_FLD_PURCHASE_END_T, ebufp);
			   id++;
			}			
		
			in_flistp  = PIN_FLIST_CREATE(ebufp);				
			
			tempp_flistp = PIN_FLIST_ELEM_GET(so_flistp,PIN_FLD_ALIAS_LIST,0, 0, ebufp);
			PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, in_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(tempp_flistp, PIN_FLD_NAME, in_flistp, PIN_FLD_NAME, ebufp);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"s_no input flist", in_flistp);

			search_serial_no(ctxp, in_flistp, &ret_flistp,ebufp);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"s_no output flist", ret_flistp);	
			
			device_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, device_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_OLD_VALUE , device_flistp, PIN_FLD_NAME, ebufp);			
			search_serial_no(ctxp, device_flistp, &ret_flistpp,ebufp);
			
			input_flistp = PIN_FLIST_CREATE(ebufp);
			
			PIN_FLIST_FLD_COPY(so_flistp, PIN_FLD_ACCOUNT_OBJ, input_flistp, PIN_FLD_POID, ebufp);
			search_name_info(ctxp, input_flistp, &return_flistp,ebufp);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"nameinfo inflistp", input_flistp);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"nameinfo return", return_flistp);
			
			//VM STB NUMBER
			// tempp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);			
			PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_OLD_VALUE, *i_flistpp, MSO_FLD_STB_ID, ebufp);
			 
			 
			//VM  serial number
			tempp_flistp = PIN_FLIST_ELEM_GET(ret_flistp,PIN_FLD_RESULTS,0,0,ebufp);
			sl_no = PIN_FLIST_FLD_GET(tempp_flistp,MSO_FLD_SERIAL_NO,0,ebufp);
			//     PIN_FLIST_FLD_COPY(tempp_flistp, MSO_FLD_SERIAL_NO, *i_flistpp, MSO_FLD_VC_ID, ebufp);
			 
			//VM  serial number2
			tempp_flistp = PIN_FLIST_ELEM_GET(ret_flistpp,PIN_FLD_RESULTS,0,0,ebufp);
			PIN_FLIST_FLD_COPY(tempp_flistp, MSO_FLD_SERIAL_NO, *i_flistpp, MSO_FLD_VC_ID, ebufp);
			 
			//VM device type
			tempp_flistp = PIN_FLIST_ELEM_GET(ret_flistpp,PIN_FLD_RESULTS,0,0,ebufp);
			PIN_FLIST_FLD_COPY(tempp_flistp, PIN_FLD_DEVICE_TYPE, *i_flistpp, PIN_FLD_DEVICE_TYPE, ebufp);
			
			//ZIPCODE						
			tmpp_flistp = PIN_FLIST_ELEM_GET(return_flistp,PIN_FLD_RESULTS,PIN_ELEMID_ANY,0,ebufp);
			name_flistp = PIN_FLIST_ELEM_GET(tmpp_flistp,PIN_FLD_NAMEINFO,PIN_ELEMID_ANY,0,ebufp);
			PIN_FLIST_FLD_COPY(name_flistp, PIN_FLD_ZIP, *i_flistpp, PIN_FLD_ZIP, ebufp);                

			/****************************************************
			 * Get base pack plan POID
			 ****************************************************/
			fm_mso_get_base_pdts(ctxp, PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_POID, 0, ebufp), 
				pdp, &catvinfo_flistp, ebufp);

			if (catvinfo_flistp)
			{
				prod_flistp = PIN_FLIST_ELEM_GET(catvinfo_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
				if (prod_flistp)
				{
					PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_PLAN_OBJ, *i_flistpp, PIN_FLD_PLAN_OBJ, ebufp);
				}
				PIN_FLIST_DESTROY_EX(&catvinfo_flistp, NULL);
			}
		}  
		/******************* VERIMATRIX CHANGES - END ***********************
		 * Creating provisioning order to change STB for Verimatrix 
		 **********************************************************************/

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		
		if(tmp_flistp) {
		   vc_id = PIN_FLIST_FLD_GET(tmp_flistp,PIN_FLD_NAME,1, ebufp);
		   fm_mso_get_device_details(ctxp,i_flistpp,vc_id,ebufp);
		}           
		
		if ( ne && strstr(ne,"VM") )
		{
			stb_vm_id = PIN_FLIST_FLD_GET(*i_flistpp,MSO_FLD_STB_ID,1, ebufp);
			fm_mso_get_device_details(ctxp,i_flistpp,stb_vm_id,ebufp);
		}     
		
		device_type = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_DEVICE_TYPE, 1, ebufp);

			/***************** TASK 1 **********************/
		task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
		PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "CHANGE_STB", ebufp);
		PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_STB_ID, stb_id, ebufp);
		if ( ne && strstr(ne,"VM") )
		{
			PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_VC_ID, sl_no, ebufp);
		}
		PIN_FLIST_FLD_SET (task_flistp, PIN_FLD_DEVICE_TYPE, device_type, ebufp);

		if ( ne && strstr(ne,"VM") )
		{
			  /***************** TASK 2 **********************/
			task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 2, ebufp);
			PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "SET_BOUQUET_ID", ebufp);
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_BOUQUET_ID, bouquet_id, ebufp);
		}


		 /* Sachid: rename flist */
					//add region_key

		if ( region_key && strlen(region_key) > 0) 
		{
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_REGION_KEY, region_key, ebufp);
		}
		else 
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_BAD_VALUE, MSO_FLD_REGION_KEY, 0, 0);
			goto CLEANUP;
		}

		if (pop_idp)
		{
			PIN_FLIST_FLD_PUT(task_flistp, MSO_FLD_POPULATION_ID, pop_idp, ebufp);
		}
		else
		{
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_POPULATION_ID, population_id, ebufp);
		}
		
		if ( ne && strstr(ne,"VM") )
		{
			add_package_detail(ctxp, i_flistpp, ADD_PACKAGE, ne, 0, ebufp);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"check", *i_flistpp);
		}
	}


    /*
     * Enrich with additional details required by Provisioning system
     */


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_catv_change_STB_flist enriched input flist", *i_flistpp);

CLEANUP:
    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
  //  PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
 //   PIN_FLIST_DESTROY_EX(&in_flistp, NULL);

    return;

}

/*******************************************************************
 * prepare_catv_change_location_flist()
 *
 * Enrich input filst with service details required
 * optionally performa validation on input fields
 *******************************************************************/
static void
prepare_catv_change_location_flist(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    char                    **act,
    pin_errbuf_t            *ebufp)
{

    pin_flist_t     *si_flistp = NULL;
    pin_flist_t     *so_flistp = NULL;
    pin_flist_t     *tmp_flistp = NULL;
    pin_flist_t     *task_flistp = NULL;
    pin_flist_t     *return_flistp = NULL;
	pin_flist_t		*in_flistp = NULL;
	pin_flist_t		*ret_flistp	= NULL;
	pin_flist_t		*serial_flistp = NULL;
	pin_flist_t		*name_flistp = NULL;
    char            *ne = NULL;
    char            *cas_subs_id = NULL;
    char            *region_key = NULL;
    char            *stb_id = NULL;
    char            *vc_id = NULL;
	char            *bouquet_id = NULL;
    char            *population_id = "0001"; //this is the only expected value in downstream system
    char	    *pop_idp = NULL;
    int32	    device_hd_flag = 0;		
	int32		profile_srch_type = MSO_FLAG_SRCH_BY_SELF_ACCOUNT;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_change_location_flist input flist", *i_flistpp);

    /*
     * Validate input received
     */



    /*
     * Enrich with additional service details
     */
    si_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);
    tmp_flistp = PIN_FLIST_SUBSTR_ADD(si_flistp, MSO_FLD_CATV_INFO, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, NULL, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_NETWORK_NODE, NULL, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_REGION_KEY, NULL, ebufp);
    tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, 0, ebufp);
    tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, 1, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_NAME, NULL, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_change_location_flist PCM_OP_READ_FLDS input flist", si_flistp);
    PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_change_location_flist PCM_OP_READ_FLDS output flist", so_flistp);

    tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
    ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);
    bouquet_id = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_BOUQUET_ID, 1, ebufp);
    PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);

    si_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, si_flistp, PIN_FLD_POID, ebufp);
    PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, si_flistp, PIN_FLD_OBJECT, ebufp);
    PIN_FLIST_FLD_SET(si_flistp, PIN_FLD_PROFILE_DESCR, CUSTOMER_CARE, ebufp);
    PIN_FLIST_FLD_SET(si_flistp, PIN_FLD_SEARCH_TYPE, &profile_srch_type, ebufp);

    fm_mso_get_profile_info(ctxp, si_flistp, &return_flistp, ebufp);
    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);

    tmp_flistp = PIN_FLIST_SUBSTR_GET(return_flistp, PIN_FLD_CUSTOMER_CARE_INFO, 0, ebufp);
 
    si_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, si_flistp, PIN_FLD_POID, ebufp);
    PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, si_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
    PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_LCO_OBJ, si_flistp, MSO_FLD_LCO_OBJ, ebufp);
    PIN_FLIST_FLD_COPY(*i_flistpp, MSO_FLD_NETWORK_NODE, si_flistp, MSO_FLD_NETWORK_NODE, ebufp);

    PIN_FLIST_DESTROY_EX(&return_flistp, NULL);

    search_name_info(ctxp, si_flistp, &return_flistp, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_GET(return_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
    PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_AREA, si_flistp, MSO_FLD_AREA, ebufp);
    PIN_FLIST_ELEM_SET(si_flistp, so_flistp, PIN_FLD_SERVICES, 0, ebufp);
 
    fm_mso_populate_bouquet_id(ctxp, si_flistp, device_hd_flag, 0, &return_flistp, ebufp);

    pop_idp = PIN_FLIST_FLD_TAKE(return_flistp, MSO_FLD_POPULATION_ID, 1, ebufp);

    if(strcmp(ne, "NONE") == 0)
    {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "No need to generate the prov request ");
    }
    else
    {
    if ( ne && strstr(ne,"NDS") )
    {
        *act = (char *) malloc(sizeof("CHANGE_LOCATION_NDS"));
        strcpy(*act,"CHANGE_LOCATION_NDS");
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                    //add cas subscriber id
    	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
        cas_subs_id = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, 0, ebufp);

        if ( cas_subs_id && strlen(cas_subs_id) > 0)
        {
            PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_CAS_SUBSCRIBER_ID, cas_subs_id, ebufp);
        }
        else
        {
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    PIN_ERR_BAD_VALUE, MSO_FLD_CAS_SUBSCRIBER_ID, 0, 0);
            goto CLEANUP;
        }

            /***************** TASK 1 **********************/
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "CHANGE_LOCATION", ebufp);

                        //add region_key
        region_key = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_REGION_KEY, 0, ebufp);

        if ( region_key && strlen(region_key) > 0)
        {
            PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_REGION_KEY, region_key, ebufp);
        }
        else
        {
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    PIN_ERR_BAD_VALUE, MSO_FLD_REGION_KEY, 0, 0);
            goto CLEANUP;
        }

        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 1, 1, ebufp);
        vc_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        if (vc_id)
        {
                fm_mso_get_device_details(ctxp,i_flistpp,vc_id,ebufp);
                PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_DEVICE_TYPE, task_flistp, PIN_FLD_DEVICE_TYPE, ebufp );
                PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_DEVICE_TYPE, ebufp);
                PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_STATE_ID, ebufp);
        }
        PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_STB_ID, stb_id, ebufp);

	if (pop_idp)
	{
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_POPULATION_ID, pop_idp, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_POPULATION_ID, population_id, ebufp);
	}

    }
	/******************* VERIMATRIX CHANGES - BEGIN ***********************
	* Creating provisioning order to change Location for Verimatrix 
	**********************************************************************/	
	else if ( ne && strstr(ne,"VM") )
    {
	if (strstr(ne, "JVM"))
	{
        	*act = (char *) malloc(sizeof("CHANGE_LOCATION_JVM"));
        	strcpy(*act,"CHANGE_LOCATION_JVM");
	}
	{
        	*act = (char *) malloc(sizeof("CHANGE_LOCATION_VM"));
        	strcpy(*act,"CHANGE_LOCATION_VM");
	}
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                    

            /***************** TASK 1 **********************/
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "CHANGE_LOCATION", ebufp);

                        //add region_key
        region_key = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_REGION_KEY, 0, ebufp);

        if ( region_key && strlen(region_key) > 0)
        {
            PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_REGION_KEY, region_key, ebufp);
        }
        else
        {
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    PIN_ERR_BAD_VALUE, MSO_FLD_REGION_KEY, 0, 0);
            goto CLEANUP;
        }

        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
        stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);
		
	    in_flistp  = PIN_FLIST_CREATE(ebufp);
	    PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, in_flistp, PIN_FLD_POID, ebufp);
	    PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_NAME, in_flistp, PIN_FLD_NAME, ebufp);
	             
	             
	    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "hp001");
                    
	             
	             
	    searh_serial_no(ctxp, in_flistp, &ret_flistp,ebufp);
	    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "hp003");
	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "vm serial number inflistp", in_flistp);
	             
	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "vm serial number search", ret_flistp);		

        //tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 1, 1, ebufp);
        //vc_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

        if (stb_id)
        {
                fm_mso_get_device_details(ctxp,i_flistpp,stb_id,ebufp);
                PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_DEVICE_TYPE, task_flistp, PIN_FLD_DEVICE_TYPE, ebufp );
                PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_DEVICE_TYPE, ebufp);
                PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_STATE_ID, ebufp);
        }
        PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_STB_ID, stb_id, ebufp);
		
		tmp_flistp = PIN_FLIST_ELEM_GET(ret_flistp,PIN_FLD_RESULTS,0,0,ebufp);
		
		PIN_FLIST_FLD_COPY(tmp_flistp,MSO_FLD_SERIAL_NO ,in_flistp, MSO_FLD_VC_ID, ebufp);
		
		serial_flistp = PIN_FLIST_CREATE(ebufp);
	    PIN_FLIST_FLD_COPY(so_flistp, PIN_FLD_ACCOUNT_OBJ, serial_flistp, PIN_FLD_POID, ebufp);
	    search_name_info(ctxp, serial_flistp, &return_flistp,ebufp);

		//Zip code 
	    tmp_flistp = PIN_FLIST_ELEM_GET(return_flistp,PIN_FLD_RESULTS,PIN_ELEMID_ANY,0,ebufp);
	    name_flistp = PIN_FLIST_ELEM_GET(tmp_flistp,PIN_FLD_NAMEINFO,PIN_ELEMID_ANY,0,ebufp);
	    PIN_FLIST_FLD_COPY(name_flistp, PIN_FLD_ZIP, *i_flistpp, PIN_FLD_ZIP, ebufp);
		
	if (pop_idp)
	{
		PIN_FLIST_FLD_PUT(task_flistp, MSO_FLD_POPULATION_ID, pop_idp, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_POPULATION_ID, population_id, ebufp);
	}		
		/***************** TASK 2 **********************/
		task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 2, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "SET_BOUQUET_ID", ebufp);
        PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_BOUQUET_ID, bouquet_id, ebufp);

    }
	/******************* VERIMATRIX CHANGES - END ***********************
	* Creating provisioning order to change Lcation for Verimatrix 
	**********************************************************************/	
	
//    else if (ne  && strstr(ne,"CISCO") )
//    {
//        *act = (char *) malloc(sizeof("CHANGE_LOCATION_PK"));
//        strcpy(*act,"CHANGE_LOCATION_PK");
//        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
//    }
    else
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
        goto CLEANUP;
    }
    }

    /*
     * Enrich with additional details required by Provisioning system
     */


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_catv_change_location_flist enriched input flist", *i_flistpp);

CLEANUP:
    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&return_flistp, NULL);

    return;

}

/*******************************************************************
 * add_package_detail() :  flag 1 = add package
 *                         flag 0 = cancel pakage
 *
 * 1. reads /purchased_product passed in input to get prouct_obj
 * 2. reads /product to get provisioning tag.
 * 3. calls funciton mso_prov_get_ca_id to get
 *        associated CA_ID.
 * 4. sets the expiration time of the package. infinite = "20990101"
 *******************************************************************/
static void
add_package_detail(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    int32                   flag,
    char                    *node,
    int32                   is_change_bq_id,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t     *tmp_flistp = NULL;
    pin_flist_t     *tmp_flistp1 = NULL;
    pin_flist_t     *i_copy_flistp = NULL;
    pin_flist_t     *sub_flistp = NULL;
    pin_flist_t     *prod_in_flistp = NULL;
    pin_flist_t     *prod_out_flistp = NULL;
    pin_flist_t     *pur_prod_out_flistp = NULL;
    pin_flist_t     *pur_prod_in_flistp = NULL;
    pin_flist_t     *ret_flistp = NULL;
    pin_flist_t     *task_flistp = NULL;
    pin_flist_t     *task_flist3 = NULL;
    pin_flist_t     *acnt_info = NULL;
    pin_flist_t     *task_flistp1 = NULL;
    pin_flist_t     *task_flistp2 = NULL;
    pin_flist_t     *alc_add_taskp = NULL;
    pin_flist_t     *alc_del_taskp = NULL;
    pin_flist_t     *del_flistp = NULL;
    pin_flist_t     *add_flistp = NULL;
    pin_flist_t     *drop_flistp = NULL;
    pin_flist_t     *drop_flistp1 = NULL;
    pin_flist_t     *tmp_args_flistp = NULL;
    pin_flist_t     *si_flistp = NULL;
	poid_t		*pdp = NULL;
    void            *vp = NULL;
    int32           rec_id = 0;
    int32           rec_id1 = 0;
    pin_cookie_t    cookie = NULL;
    pin_cookie_t    prev_cookie = 0;
    pin_cookie_t    cookie1 = NULL;
    char            *tag = NULL;
    char	    *mso_device_typep = NULL;
    time_t          *end_tp = NULL;
    time_t          *start_tp = NULL;
    time_t          now_t = 0;
    struct          tm *current_time;
    int             year;
    int             month;
    int             day;
    char            validity[21];
    char            svalidity[21];
    char            *del_ca_id = NULL;
    char            *add_ca_id = NULL;
    char	    *ca_id = NULL;
	char		*no_ca_idp = "Deleted";
	char		new_ca_id[128];
    char            *prog_namep = NULL;
    int32           task_id = 1;
    int32           *task_count = NULL;
    int32           *status_flag = NULL;
    int32           *flags = NULL;
    int32           add_package_id = 1;
    int32           add_alc_package_id = 1;
    int32           cancel_package_id = 1;
    int32           cancel_alc_package_id = 1;
    int32           package_type = -1;
    int32           alc_pkg_type = -1;
    int32	    *pkg_typep = NULL;
    int32	    business_type = 0;
    int32	    arr_business_type[8];
    int32	    lob_exclude_flag = -1;
    int32	    device_hd_flag = 0; 
	int32		dpo_flags = 0;
	int32		pkg_channel_flag = 1;
	int32		prod_counter = 0;	
	int32		change_flag = -1;
	int32		subscriber_type = -1;
	int32		profile_srch_type = MSO_FLAG_SRCH_BY_SELF_ACCOUNT;
    int32       low_priority = 0;
    //int32       low_priority = 10;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;
    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "add_package_detail input flist", *i_flistpp);

    if(*i_flistpp == NULL){
	return;
    }

    task_id = PIN_FLIST_ELEM_COUNT(*i_flistpp, MSO_FLD_TASK, ebufp);
    prod_counter = PIN_FLIST_ELEM_COUNT(*i_flistpp, PIN_FLD_PRODUCTS, ebufp);

    PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_NETWORK_NODE, node, ebufp);

    pkg_typep = (int32 *)PIN_FLIST_FLD_TAKE(*i_flistpp,  MSO_FLD_PKG_TYPE, 1, ebufp);
	if ( pkg_typep && *pkg_typep == 0)
	{
		package_type = 0;
	}
    prog_namep = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	pdp = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_PLAN_OBJ, 1, ebufp);
	if (pdp)
	{
                prod_in_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(prod_in_flistp, PIN_FLD_POID, pdp, ebufp);
		PIN_FLIST_FLD_SET(prod_in_flistp, PIN_FLD_DESCR, NULL, ebufp);

		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, prod_in_flistp, &prod_out_flistp, ebufp);
		
		PIN_FLIST_DESTROY_EX(&prod_in_flistp, NULL);
		
				
		if(prod_out_flistp != NULL)
		{
			tag = PIN_FLIST_FLD_GET(prod_out_flistp, PIN_FLD_DESCR, 0, ebufp);
		}
		if (tag && strstr(tag, "DPO-"))
		{
			dpo_flags = 1;
		}

		PIN_FLIST_DESTROY_EX(&prod_out_flistp, NULL);
	}

	if (flag == CHANGE_PACKAGE)
	{
        	while ( (tmp_flistp = PIN_FLIST_ELEM_TAKE_NEXT(*i_flistpp,
                                PIN_FLD_PLAN_LISTS, &rec_id, 1,&cookie, ebufp)) != (pin_flist_t *)NULL )
		{
			change_flag = *(int32 *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_FLAGS, 0, ebufp);
			change_flag = change_flag + 1;
			dpo_flags = dpo_flags + 1;
			pdp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
			prod_in_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(prod_in_flistp, PIN_FLD_POID, pdp, ebufp);
			PIN_FLIST_FLD_SET(prod_in_flistp, PIN_FLD_FLAGS, &pkg_channel_flag, ebufp);

			PCM_OP(ctxp, MSO_OP_CUST_GET_PKG_CHANNELS, 0, prod_in_flistp, &prod_out_flistp, ebufp);

			PIN_FLIST_DESTROY_EX(&prod_in_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&tmp_flistp, NULL);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "add_package_detail pkg_channels", prod_out_flistp);
			rec_id1 = 0;
			cookie1 = NULL;
			while ( (tmp_flistp1 = PIN_FLIST_ELEM_GET_NEXT(prod_out_flistp,
				PIN_FLD_PRODUCTS, &rec_id1, 1,&cookie1, ebufp)) != (pin_flist_t *)NULL )
			{
				tag = PIN_FLIST_FLD_GET(tmp_flistp1, PIN_FLD_PROVISIONING_TAG, 0, ebufp);
				if (strcmp(tag, "PROV_NCF_PRO") != 0)
				{
					task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, PIN_FLD_PRODUCTS, prod_counter, ebufp);
					PIN_FLIST_FLD_COPY(tmp_flistp1, PIN_FLD_PRODUCT_OBJ, task_flistp, PIN_FLD_POID, ebufp);
					PIN_FLIST_FLD_COPY(tmp_flistp1, PIN_FLD_PROVISIONING_TAG, task_flistp, PIN_FLD_PROVISIONING_TAG, ebufp);
					PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_STATUS_FLAGS, &change_flag, ebufp);
					prod_counter++;
                        	}
                	}
		}
	}

	if (dpo_flags && flag != CHANGE_PACKAGE)
	{
		prod_in_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(prod_in_flistp, PIN_FLD_POID, pdp, ebufp);
		PIN_FLIST_FLD_SET(prod_in_flistp, PIN_FLD_FLAGS, &pkg_channel_flag, ebufp);

		PCM_OP(ctxp, MSO_OP_CUST_GET_PKG_CHANNELS, 0, prod_in_flistp, &prod_out_flistp, ebufp);

		PIN_FLIST_DESTROY_EX(&prod_in_flistp, NULL);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "add_package_detail pkg_channels", prod_out_flistp);
		rec_id = 0;
		cookie = NULL;
        	while ( (tmp_flistp = PIN_FLIST_ELEM_GET_NEXT(prod_out_flistp,
                                PIN_FLD_PRODUCTS, &rec_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL )
		{
			tag = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_PROVISIONING_TAG, 0, ebufp);	
			if (strcmp(tag, "PROV_NCF_PRO") != 0)
			{
                		task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, PIN_FLD_PRODUCTS, prod_counter, ebufp);
                		PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_PRODUCT_OBJ, task_flistp, PIN_FLD_POID, ebufp);
                		PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_PROVISIONING_TAG, task_flistp, PIN_FLD_PROVISIONING_TAG, ebufp);
        			if (flag == ADD_PACKAGE || flag == CANCEL_PACKAGE)
        			{
					PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_STATUS_FLAGS, &flag, ebufp);	
					PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_STATUS_FLAGS, &dpo_flags, ebufp);	
				}
				prod_counter++;
			}
		}
		PIN_FLIST_DESTROY_EX(&prod_out_flistp, NULL);
	}
	/****** VERIMATRIX CHANGES - Enhanced condition to check
	 * Verimatrix network node
	 *******************************************************/
    if (node && (strstr(node, "NDS") || strstr(node, "VM") || 
	strstr(node, "GOSPEL") || strstr(node,"NAGRA")))
    {
        if ( flag == ADD_PACKAGE )
        {
                task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, ++task_id, ebufp);
                PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "AUTHORIZE_SERVICES", ebufp);

		if (strstr(node,"NDS") || strstr(node, "VM"))
		{
			/*************************************************************
		 	 * Add LBO service task 
		 	 ************************************************************/
			task_flistp1 = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, ++task_id, ebufp);
			PIN_FLIST_FLD_SET (task_flistp1, MSO_FLD_TASK_NAME, "AUTHORIZE_LBO_SERVICES", ebufp);
		}
		else if (strstr(node, "NAGRA"))
		{
			/*************************************************************
		 	 * Add ALC service task 
		 	 ************************************************************/
			alc_add_taskp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, ++task_id, ebufp);
			PIN_FLIST_FLD_SET(alc_add_taskp, MSO_FLD_TASK_NAME, "AUTHORIZE_ALC_SERVICES", ebufp);
		}
        }
        else if ( flag == CANCEL_PACKAGE)
        {
            task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, ++task_id, ebufp);
            PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "UNAUTHORIZE_SERVICES", ebufp);
		if (strstr(node, "NAGRA"))
		{
			/*************************************************************
		 	 * Add ALC service task 
		 	 ************************************************************/
			alc_del_taskp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, ++task_id, ebufp);
			PIN_FLIST_FLD_SET(alc_del_taskp, MSO_FLD_TASK_NAME, "UNAUTHORIZE_ALC_SERVICES", ebufp);
		}
        }
        else if ( flag == CHANGE_PACKAGE)
        {
                task_flistp2 = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, ++task_id, ebufp);
                PIN_FLIST_FLD_SET (task_flistp2, MSO_FLD_TASK_NAME, "AUTHORIZE_SERVICES", ebufp);

                task_flistp1 = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, ++task_id, ebufp);
                PIN_FLIST_FLD_SET (task_flistp1, MSO_FLD_TASK_NAME, "UNAUTHORIZE_SERVICES", ebufp);

		if (strstr(node,"NDS") || strstr(node, "VM"))
		{
			/*************************************************************
		 	* Add LBO service task 
		 	************************************************************/
			task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, ++task_id, ebufp);
			PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "AUTHORIZE_LBO_SERVICES", ebufp);
		}
		else if (strstr(node, "NAGRA"))
		{
              alc_add_taskp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, ++task_id, ebufp);
              PIN_FLIST_FLD_SET(alc_add_taskp, MSO_FLD_TASK_NAME, "AUTHORIZE_ALC_SERVICES", ebufp);

              alc_del_taskp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, ++task_id, ebufp);
              PIN_FLIST_FLD_SET(alc_del_taskp, MSO_FLD_TASK_NAME, "UNAUTHORIZE_ALC_SERVICES", ebufp);
		}
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "add_package_detail products array flist", *i_flistpp);
        /*
         * Enrich with Package details
         */
        rec_id = 0;
        cookie = NULL;
        i_copy_flistp = PIN_FLIST_COPY(*i_flistpp, ebufp);
        while ( (tmp_flistp = PIN_FLIST_ELEM_GET_NEXT (i_copy_flistp,
            PIN_FLD_PRODUCTS, &rec_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL )
        {
            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "add_package_detail products array flist loop ", tmp_flistp);
            pdp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_POID, 1, ebufp);
            status_flag = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_STATUS_FLAGS, 1, ebufp);
            if (pdp && strstr(PIN_POID_GET_TYPE(pdp), "/purchased_product"))
            {
                	pur_prod_in_flistp = PIN_FLIST_CREATE(ebufp);
                	PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_POID, pur_prod_in_flistp, PIN_FLD_POID, ebufp);
                	PIN_FLIST_FLD_SET (pur_prod_in_flistp, PIN_FLD_PRODUCT_OBJ,(void *)NULL, ebufp);
                	PIN_FLIST_FLD_SET (pur_prod_in_flistp, PIN_FLD_PURCHASE_END_T,(void *)NULL, ebufp);
                	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        	"add_package_detail purchased product READ_FLDS input flist", pur_prod_in_flistp);

                	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, pur_prod_in_flistp, &pur_prod_out_flistp, ebufp);

                	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        	"add_package_detail purchased product READ_FLDS output flist", pur_prod_out_flistp);

                    if (dpo_flags)
                    {
                        end_tp = (time_t *)PIN_FLIST_FLD_TAKE(pur_prod_out_flistp, PIN_FLD_PURCHASE_END_T,
                                    0, ebufp);
                    }
                    else
                    {
                        end_tp = (time_t *)PIN_FLIST_FLD_GET(pur_prod_out_flistp, PIN_FLD_PURCHASE_END_T, 0, ebufp);
                    }

                	prod_in_flistp = PIN_FLIST_CREATE(ebufp);

                    if (!pur_prod_out_flistp) {
		    		    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Unable to read purchased product details");
		    		    goto cleanup;
                    }

                	PIN_FLIST_FLD_COPY(pur_prod_out_flistp, PIN_FLD_PRODUCT_OBJ, prod_in_flistp, PIN_FLD_POID, ebufp);

                	PIN_FLIST_FLD_SET (prod_in_flistp, PIN_FLD_PROVISIONING_TAG,(void *)NULL, ebufp);
                	PIN_FLIST_FLD_SET (prod_in_flistp, PIN_FLD_PRIORITY,(void *)NULL, ebufp);

                	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, prod_in_flistp, &prod_out_flistp, ebufp);

                	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        	"add_package_detail product READ_FLDS output flist", prod_out_flistp);

                    if (!prod_out_flistp) {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Unable to read product details");
                        goto cleanup;
                    }

                	tag = (char *)PIN_FLIST_FLD_GET(prod_out_flistp, PIN_FLD_PROVISIONING_TAG, 1, ebufp);
            }
            else
            {
                    tag = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_PROVISIONING_TAG, 0, ebufp);
            }
            if ((tag && strlen(tag) > 0))
            {
                if ((strcmp(tag, "PROV_NCF_PRO") == 0) && (flag == CANCEL_PACKAGE 
                            || (status_flag && *status_flag == CANCEL_PACKAGE)))
                {
                    PIN_FLIST_DESTROY_EX(&pur_prod_out_flistp, NULL);
                    PIN_FLIST_DESTROY_EX(&prod_out_flistp, NULL);
                    continue;
                }
                else if ((strcmp(tag, "PROV_NCF_PRO") == 0) && strstr(node, "VM"))
                {
   
                    vp = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_LCO_OBJ, 1, ebufp);
                    if (vp == NULL)
                    {
                        si_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, si_flistp, PIN_FLD_POID, ebufp);
                        PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, si_flistp, PIN_FLD_OBJECT, ebufp);
                        PIN_FLIST_FLD_SET(si_flistp, PIN_FLD_PROFILE_DESCR, CUSTOMER_CARE, ebufp);
                        PIN_FLIST_FLD_SET(si_flistp, PIN_FLD_SEARCH_TYPE, &profile_srch_type, ebufp);

                        fm_mso_get_profile_info(ctxp, si_flistp, &ret_flistp, ebufp);
                        PIN_FLIST_DESTROY_EX(&si_flistp, NULL);

                        tmp_args_flistp = PIN_FLIST_SUBSTR_GET(ret_flistp, PIN_FLD_CUSTOMER_CARE_INFO, 0, ebufp);
                        PIN_FLIST_FLD_COPY(tmp_args_flistp, MSO_FLD_LCO_OBJ, *i_flistpp, MSO_FLD_LCO_OBJ, ebufp);
                        PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
                    }
                    mso_device_typep = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_DEVICE_TYPE, 1, ebufp);
                    if (mso_device_typep)
                    {
                        device_hd_flag = *mso_device_typep;
                    }
                    vp = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_AREA, 1, ebufp);
		
                    if (!vp)
                    {
        				search_name_info(ctxp, *i_flistpp, &ret_flistp,ebufp);
            
        				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                    					"nameinfo return", ret_flistp);
		
                        tmp_flistp1 = PIN_FLIST_ELEM_GET(ret_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
                        PIN_FLIST_FLD_COPY(tmp_flistp1, MSO_FLD_AREA, *i_flistpp, MSO_FLD_AREA, ebufp);
    				
                        PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
                    }
				    fm_mso_populate_bouquet_id(ctxp, *i_flistpp, device_hd_flag, 0, &ret_flistp, ebufp);
                    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Bouquet ID return", ret_flistp);
                    vp = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_POID, 0, ebufp);
                    fm_mso_get_account_info(ctxp, (poid_t *)vp, &acnt_info, ebufp);

                    vp = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_BUSINESS_TYPE, 0, ebufp);
                    if (vp && vp != NULL)
                    {
                        business_type = *(int32 *)vp;
                        num_to_array(business_type, arr_business_type);
                        lob_exclude_flag = arr_business_type[7];
                        subscriber_type = 10 * (arr_business_type[2]) + arr_business_type[3];
                    }
                    PIN_FLIST_DESTROY_EX(&acnt_info, NULL);

                    if ( flag == ADD_PACKAGE )
                    {
					PIN_FLIST_FLD_COPY(ret_flistp, PIN_FLD_VALIDITY_IN_DAYS, 
						task_flistp1, PIN_FLD_VALIDITY_IN_DAYS, ebufp);
					PIN_FLIST_FLD_COPY(ret_flistp, PIN_FLD_VALID_DOM, 
						task_flistp1, PIN_FLD_VALID_DOM, ebufp);
					PIN_FLIST_FLD_COPY(ret_flistp, PIN_FLD_TIMEZONE, 
						task_flistp1, PIN_FLD_TIMEZONE, ebufp);
					PIN_FLIST_FLD_COPY(ret_flistp, PIN_FLD_SPEED, 
						task_flistp1, PIN_FLD_SPEED, ebufp);
                    }
                    else if ( flag == CHANGE_PACKAGE)
                    {
					    PIN_FLIST_FLD_COPY(ret_flistp, PIN_FLD_VALIDITY_IN_DAYS, 
						    task_flistp, PIN_FLD_VALIDITY_IN_DAYS, ebufp);
					    PIN_FLIST_FLD_COPY(ret_flistp, PIN_FLD_VALID_DOM, 
						task_flistp, PIN_FLD_VALID_DOM, ebufp);
				    	PIN_FLIST_FLD_COPY(ret_flistp, PIN_FLD_TIMEZONE, 
					    	task_flistp, PIN_FLD_TIMEZONE, ebufp);
					    PIN_FLIST_FLD_COPY(ret_flistp, PIN_FLD_SPEED, 
						task_flistp, PIN_FLD_SPEED, ebufp);
				    }
                    if ((end_tp && *end_tp == 0) || (lob_exclude_flag == 1) ||
                            (lob_exclude_flag == 3) || (lob_exclude_flag == 5))
                    {
                        now_t = pin_virtual_time((time_t *)NULL);
                        if ((strcmp(tag, "PROV_NCF_PRO") == 0 && dpo_flags && pkg_channel_flag) || dpo_flags == 0)
                        {
                            /* Temp work around now_t = now_t - 86400; 30-Apr-2021*/
                            now_t = 1703961000;
                            if (pkg_channel_flag)
                            {
                                pkg_channel_flag = 0;
                            }

                        }
                        current_time = localtime(&now_t);
                        year = current_time->tm_year + 1900;
                        month = current_time->tm_mon + 1;
                        day = current_time->tm_mday;
                        if ( (year % 4 != 0) && month == 2 && day == 29)
                        {
                            day = day - 1;
                        }
                        //memset(validity,10,'\0');
                        if (strstr(node,"GOSPEL"))
                        {
                            memset(validity,'\0',21);
                            sprintf(validity,"%04d-%02d-%02dT00:00:00",year,month,day);
                        }
                        else
                        {
                            memset(validity,'\0',10);
                            sprintf(validity,"%04d%02d%02d",year,month,day);
                        }
                    }
                    else
                    {
                        if ((strcmp(tag, "PROV_NCF_PRO") == 0 && dpo_flags && pkg_channel_flag) || dpo_flags == 0)
                        {
                            *end_tp = *end_tp - 86400;
                            if (pkg_channel_flag)
                            {
                                pkg_channel_flag = 0;
                            }

                            if (*end_tp > 1703961000)
                            {
                                *end_tp = 1703961000;
                            }
                        }
                        current_time = localtime(end_tp);
                        year = current_time->tm_year + 1900;
                        month = current_time->tm_mon +1;
                        day = current_time->tm_mday;
                        if ((year % 4 != 0) && month == 2 && day == 29)
                        {
                            day = day - 1;
                        }
                        //memset(validity,10,'\0');
                        if (strstr(node,"GOSPEL"))
                        {
                            memset(validity,'\0',21);
                            sprintf(validity,"%04d-%02d-%02dT00:00:00",year,month,day);
                        }
                        else
                        {
                            memset(validity,'\0',10);
                            sprintf(validity,"%04d%02d%02d",year,month,day);
                        }
                    }
                    if (flag == ADD_PACKAGE && task_flistp1 && strstr(node, "VM"))
                    {
                        PIN_FLIST_FLD_SET(task_flistp1, MSO_FLD_EXPIRYDATE, validity, ebufp);
                    }     
                    else if (flag == CHANGE_PACKAGE && task_flistp && strstr(node, "VM"))
                    {
                        PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_EXPIRYDATE, validity, ebufp);
                    }
                    PIN_FLIST_DESTROY_EX(&prod_out_flistp, NULL);
    				PIN_FLIST_DESTROY_EX(&pur_prod_out_flistp, NULL);
    				PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
				    continue;
                }
                else
                {
                }
                /*NT: Start for changes to restrict CA_ID's with NA value*/
                mso_prov_get_ca_id(ctxp, tag, node, &ret_flistp, ebufp);

                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                    "add_package_detail provisioning tag details flist", ret_flistp);

                if (ret_flistp)
                {
                    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CA_ID check ");

                    ca_id = PIN_FLIST_FLD_GET(ret_flistp, MSO_FLD_CA_ID, 0, ebufp);
                    alc_pkg_type = *(int32 *)PIN_FLIST_FLD_GET(ret_flistp,
					    	MSO_FLD_PKG_TYPE, 0, ebufp);
                    if (package_type != 0)
                    {
					    package_type = alc_pkg_type;
				    }

                    if(strcmp(ca_id, "NA") == 0 || strstr(ca_id, "N/A"))
                    {
                        PIN_FLIST_DESTROY_EX(&pur_prod_out_flistp, NULL);
                        PIN_FLIST_DESTROY_EX(&prod_out_flistp, NULL);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CA_ID is NA");
                        continue;
                    }
                    else
                    {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CA_ID is not NA");
                    }
                }
                else
                {	
                    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
                                "add_package_detail products array CA_ID not found");
                    pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, MSO_FLD_CA_ID, 0, 0);
                    break;
                }
                /*NT: Stop for changes to restrict CA_ID's with NA value*/

                if ( flag == ADD_PACKAGE )
                {
                    if ((strcmp(tag, "PROV_NCF_PRO") == 0) && task_flistp1)
                    {
					    sub_flistp = PIN_FLIST_ELEM_ADD(task_flistp1,
						    PIN_FLD_SERVICES, 1, ebufp);
                    }
                    else if (strstr(node, "NAGRA") && alc_pkg_type == 2)
                    {
					    sub_flistp = PIN_FLIST_ELEM_ADD(alc_add_taskp,
						    PIN_FLD_OFFERINGS, add_alc_package_id++, ebufp);
                    }
                    else
                    {
                        sub_flistp = PIN_FLIST_ELEM_ADD(task_flistp,
                            MSO_FLD_ADD_PACKAGE_INFO, add_package_id++, ebufp);
                        if (strstr(node, "JVM") &&
                                strstr(prog_namep, "BWF") || strstr(prog_namep, "BLK")||strstr(prog_namep, "BULK"))
                        {
                            PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_DB_PRIORITY, &low_priority, ebufp);
                        }

                    }
                }
                else if (flag == CANCEL_PACKAGE)
                {
                    if (strstr(node, "NAGRA") && alc_pkg_type == 2 ) 
                    {
                        sub_flistp = PIN_FLIST_ELEM_ADD(alc_del_taskp,
                            PIN_FLD_DUNNING_INFO, cancel_alc_package_id++, ebufp);
                    }
                    else
                    {
                        sub_flistp = PIN_FLIST_ELEM_ADD(task_flistp,
                            MSO_FLD_DEL_PACKAGE_INFO, cancel_package_id++, ebufp);
                    }
                }
                else if ( flag == CHANGE_PACKAGE)
                {
                    if (status_flag && *status_flag == CANCEL_PACKAGE )
                    {
                        if (strstr(node, "NAGRA") && alc_pkg_type == 2 ) 
                        {
                            sub_flistp = PIN_FLIST_ELEM_ADD(alc_del_taskp,
                                PIN_FLD_DUNNING_INFO, cancel_alc_package_id++, ebufp);
                        }
                        else
                        {
                            sub_flistp = PIN_FLIST_ELEM_ADD(task_flistp1,
                                MSO_FLD_DEL_PACKAGE_INFO, cancel_package_id++, ebufp);
                        }
                        if ((strcmp(tag, "PROV_NCF_PRO") == 0))
                        {
                            PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_PROVISIONING_TAG, "PROV_NCF_PRO", ebufp);
                        }
                    }
                    else
                    {
					    if ((strcmp(tag, "PROV_NCF_PRO") == 0) && task_flistp)
					    {
			    			sub_flistp = PIN_FLIST_ELEM_ADD(task_flistp,
				    			PIN_FLD_SERVICES, 1, ebufp);
					    }
					    else if (strstr(node, "NAGRA") && alc_pkg_type == 2)
					    {
					    	sub_flistp = PIN_FLIST_ELEM_ADD(alc_add_taskp,
						    	PIN_FLD_OFFERINGS, add_alc_package_id++, ebufp);
					    }
					    else
					    {
                           sub_flistp = PIN_FLIST_ELEM_ADD(task_flistp2,
                                MSO_FLD_ADD_PACKAGE_INFO, add_package_id++, ebufp);
                           if (strstr(node, "JVM") &&
                                   strstr(prog_namep, "BWF") || strstr(prog_namep, "BLK")||strstr(prog_namep, "BULK"))
                           {
                               PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_DB_PRIORITY, &low_priority, ebufp);
                           }

					    }
                    }
                }

                if (ret_flistp)
                {
				/*
                                if((flag == CANCEL_PACKAGE || flag == ADD_PACKAGE) && package_type != 0)
                                {
					package_type = *(int32 *)PIN_FLIST_FLD_GET(ret_flistp,
						MSO_FLD_PKG_TYPE, 0, ebufp);
                                }*/

                    PIN_FLIST_FLD_SET(sub_flistp, MSO_FLD_TAPING_AUTHORIZATION, "N", ebufp);

                    /******************************************************
                     * Get business type to check whether its commerial
                     *****************************************************/
                    if ((strcmp(tag, "PROV_NCF_PRO") == 0) || lob_exclude_flag == -1)
                    {
                        vp = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_POID, 0, ebufp);
                        fm_mso_get_account_info(ctxp, (poid_t *)vp, &acnt_info, ebufp);

                        vp = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_BUSINESS_TYPE, 0, ebufp);
                        if (vp && vp != NULL)
                        {
			                business_type = *(int32 *)vp;
						    num_to_array(business_type, arr_business_type);
						    lob_exclude_flag = arr_business_type[7];
						    subscriber_type = 10 * (arr_business_type[2]) + arr_business_type[3];
                        }
                        PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
                    }
                    if (subscriber_type != 48)
                    {
					    PIN_FLIST_FLD_COPY(ret_flistp, MSO_FLD_CA_ID,
						sub_flistp, MSO_FLD_CA_ID, ebufp);
                    }
                    else
                    {
                        PIN_FLIST_FLD_SET(*i_flistpp, PIN_FLD_TYPE_OF_SERVICE, &subscriber_type, ebufp);
                        memset(new_ca_id,'\0',sizeof(new_ca_id));
                        strcpy(new_ca_id, ca_id);
                        if (lob_exclude_flag > 1)
                        {
                            strcat(new_ca_id, "_DN_IP");
                        }
                        else
                        {
                            strcat(new_ca_id, "_HW_IP");
                        }
                        PIN_FLIST_FLD_SET(sub_flistp, MSO_FLD_CA_ID, new_ca_id, ebufp);
                    }
                    if ((end_tp && *end_tp == 0) || (lob_exclude_flag == 1))
                    {
        				now_t = pin_virtual_time((time_t *)NULL);
                        if ((strcmp(tag, "PROV_NCF_PRO") == 0 && dpo_flags && pkg_channel_flag) || dpo_flags == 0)
                        {
                            //now_t = pin_virtual_time((time_t *)NULL);
                            //now_t = now_t - 86400;
                            now_t = 1703961000;
						    if (pkg_channel_flag)
						    {
							    pkg_channel_flag = 0;
						    }
					    }
                        current_time = localtime(&now_t);
                        year = current_time->tm_year + 1900;
                        month = current_time->tm_mon + 1;
                        day = current_time->tm_mday;
                        if ( (year % 4 != 0) && month == 2 && day == 29)
                        {
						    day = day - 1;
                        }
                                        //memset(validity,10,'\0');
                        if (strstr(node,"GOSPEL"))
                        {
                            memset(validity,'\0',21);
                            sprintf(validity,"%04d-%02d-%02dT00:00:00",year,month,day);
                        }
                        else
                        {
                            memset(validity,'\0',10);
                            sprintf(validity,"%04d%02d%02d",year,month,day);
                        }
                        PIN_FLIST_FLD_SET(sub_flistp, MSO_FLD_EXPIRATION_DATE, validity, ebufp);
                    }
                    else 
                    {
                        if ((strcmp(tag, "PROV_NCF_PRO") == 0 && dpo_flags && pkg_channel_flag) || dpo_flags == 0)
                        {
						    *end_tp = *end_tp - 86400;
                            if (pkg_channel_flag)
                            {
							    pkg_channel_flag = 0;
                            }

                            if (*end_tp > 1703961000)
                            {
                                *end_tp = 1703961000;
                            }
                        }
                        current_time = localtime(end_tp);
                        year = current_time->tm_year + 1900;
                        month = current_time->tm_mon +1;
                        day = current_time->tm_mday;
                        if ((year % 4 != 0) && month == 2 && day == 29)
                        {
						    day = day - 1;
                        }
                                        //memset(validity,10,'\0');
                        if (strstr(node,"GOSPEL"))
                        {
                            memset(validity,'\0',21);
                            sprintf(validity,"%04d-%02d-%02dT00:00:00",year,month,day);
                        }
                        else
                        {
                            memset(validity,'\0',10);
                            sprintf(validity,"%04d%02d%02d",year,month,day);
                        }
                        PIN_FLIST_FLD_SET(sub_flistp, MSO_FLD_EXPIRATION_DATE, validity, ebufp);
                    }
    				PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
                }
                else
                {
                    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
                        "add_package_detail products array CA_ID not found");
                    pin_set_err(ebufp, PIN_ERRLOC_FM,
                    PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, MSO_FLD_CA_ID, 0, 0);
                    break;
                }
            }
            else
            {
            }
    		PIN_FLIST_DESTROY_EX(&pur_prod_out_flistp, NULL);
    		PIN_FLIST_DESTROY_EX(&prod_out_flistp, NULL);
         }//end while

        if (i_copy_flistp) PIN_FLIST_DESTROY_EX(&i_copy_flistp, NULL);
	if (end_tp && dpo_flags)
	{
		free(end_tp);
	}
    }
    else if (node && strstr(node,"CISCO") )
    {
        if ( flag == ADD_PACKAGE )
        {
                task_flistp = PIN_FLIST_ELEM_GET(*i_flistpp, MSO_FLD_TASK, task_id, 1, ebufp);

        }
        else if ( flag == CANCEL_PACKAGE)
        {
                task_flistp = PIN_FLIST_ELEM_GET(*i_flistpp, MSO_FLD_TASK, task_id, 1, ebufp);

        }
        else if ( flag == CHANGE_PACKAGE)
        {
                task_flistp1 = PIN_FLIST_ELEM_GET(*i_flistpp, MSO_FLD_TASK, 2, 0, ebufp);
                task_flistp2 = PIN_FLIST_ELEM_GET(*i_flistpp, MSO_FLD_TASK, 1, 0, ebufp);
        }

        /*
         * Enrich with Package details
         */
    	rec_id = 0;
        cookie = NULL;
        while ( (tmp_flistp = PIN_FLIST_ELEM_GET_NEXT (*i_flistpp,
                        PIN_FLD_PRODUCTS, &rec_id, 1,&cookie, ebufp)) != (pin_flist_t *)NULL )
        {
		pdp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_POID, 1, ebufp);
		if (pdp && strstr(PIN_POID_GET_TYPE(pdp), "/purchased_product"))
		{
                	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "inside loop");
			if(pur_prod_in_flistp == NULL)
                		pur_prod_in_flistp = PIN_FLIST_CREATE(ebufp);

                	PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_POID, pur_prod_in_flistp, PIN_FLD_POID, ebufp);
                	PIN_FLIST_FLD_SET (pur_prod_in_flistp, PIN_FLD_PRODUCT_OBJ,(void *)NULL, ebufp);
                	PIN_FLIST_FLD_SET (pur_prod_in_flistp, PIN_FLD_PURCHASE_END_T,(void *)NULL, ebufp);

                	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, pur_prod_in_flistp, &pur_prod_out_flistp, ebufp);
    			PIN_FLIST_DESTROY_EX(&pur_prod_in_flistp, NULL);

                	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        	"add_package_detail purchased product READ_FLDS output flist", pur_prod_out_flistp);

			if(prod_in_flistp == NULL)
                		prod_in_flistp = PIN_FLIST_CREATE(ebufp);

			if(!pur_prod_out_flistp){
		    		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Unable to read purchased product details");
		    		goto cleanup;
			}

                	PIN_FLIST_FLD_COPY(pur_prod_out_flistp, PIN_FLD_PRODUCT_OBJ, prod_in_flistp, PIN_FLD_POID, ebufp);
                	PIN_FLIST_FLD_SET (prod_in_flistp, PIN_FLD_PROVISIONING_TAG,(void *)NULL, ebufp);
                	PIN_FLIST_FLD_SET (prod_in_flistp,PIN_FLD_PRIORITY,(void *)NULL, ebufp);
                	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, prod_in_flistp, &prod_out_flistp, ebufp);
    			PIN_FLIST_DESTROY_EX(&prod_in_flistp, NULL);

                	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        	"add_package_detail product READ_FLDS output flist", prod_out_flistp);

			if(!prod_out_flistp){
		    		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Unable to read product details");
                    		goto cleanup;
			}

                	tag = (char *)PIN_FLIST_FLD_GET(prod_out_flistp, PIN_FLD_PROVISIONING_TAG, 1, ebufp);
		}
		else
		{
                	tag = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_PROVISIONING_TAG, 0, ebufp);
		}
                if ( (tag && strlen(tag) > 0))
                {
                        /*NT: Start for changes to restrict CA_ID's with NA value*/
                        mso_prov_get_ca_id(ctxp, tag, node, &ret_flistp, ebufp);
		        if (ret_flistp)
	        	{
        	        	ca_id = PIN_FLIST_FLD_GET(ret_flistp, MSO_FLD_CA_ID, 0, ebufp);
				if (package_type != 0)
				{
					package_type = *(int32 *)PIN_FLIST_FLD_GET(ret_flistp,
						MSO_FLD_PKG_TYPE, 1, ebufp);
				}

	        	        if(strcmp(ca_id, "NA") == 0|| strstr(ca_id, "N/A"))
        	        	{
		                 	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CA_ID is NA");
					continue;
        	        	}
				else 
				{
                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CA_ID is not NA");

				}
        		}
                        else
                        {
                                pin_set_err(ebufp, PIN_ERRLOC_FM,
                                         PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, MSO_FLD_CA_ID, 0, 0);
				break;
                        }
                        /*NT: Stop for changes to restrict CA_ID's with NA value*/

                        if ( flag == ADD_PACKAGE )
                        {
                                sub_flistp = PIN_FLIST_ELEM_ADD(task_flistp,
                                        MSO_FLD_ADD_PACKAGE_INFO, add_package_id++, ebufp);
                        }
                        else if ( flag == CANCEL_PACKAGE)
                        {
                                sub_flistp = PIN_FLIST_ELEM_ADD(task_flistp,
                                        MSO_FLD_DEL_PACKAGE_INFO, cancel_package_id++, ebufp);
                        }
                        else if ( flag == CHANGE_PACKAGE)
                        {
                                status_flag = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_STATUS_FLAGS, 1, ebufp);
                                if (status_flag && *status_flag == CANCEL_PACKAGE )
                                {
                                        sub_flistp = PIN_FLIST_ELEM_ADD(task_flistp1,
                                                MSO_FLD_DEL_PACKAGE_INFO, cancel_package_id++, ebufp);
                                }
                                else
                                {
                                        sub_flistp = PIN_FLIST_ELEM_ADD(task_flistp2,
                                                MSO_FLD_ADD_PACKAGE_INFO, add_package_id++, ebufp);
                                }

                        }

                        //mso_prov_get_ca_id(ctxp, tag, node, &ret_flistp, ebufp);
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                "add_package_detail provisioning tag details flist", ret_flistp);

			PIN_FLIST_FLD_COPY(ret_flistp, MSO_FLD_CA_ID,
				sub_flistp, MSO_FLD_CA_ID, ebufp);
                }
                else
                {
                }
    		PIN_FLIST_DESTROY_EX(&pur_prod_out_flistp, NULL);
    		PIN_FLIST_DESTROY_EX(&prod_out_flistp, NULL);
        }//end while

         /****** In case of terminate service there is no products array send to provisioning ****/
        flags = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_FLAGS,1, ebufp);
        if((*(int32 *)flags == 7) && ( PIN_FLIST_ELEM_COUNT(*i_flistpp,PIN_FLD_PRODUCTS,ebufp) == 0))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " This is terminate service call ");
                sub_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_DEL_PACKAGE_INFO, cancel_package_id++, ebufp);
		if (strcmp(node, "CISCO1") == 0)
		{
                	PIN_FLIST_FLD_SET(sub_flistp , MSO_FLD_CA_ID , no_ca_idp, ebufp);
		}
		else
		{
                	PIN_FLIST_FLD_SET(sub_flistp , MSO_FLD_CA_ID , "NULL" , ebufp);
		}
        }
    }

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "add_package_detail enriched output flist before modified", *i_flistpp);
		
	/****** VERIMATRIX CHANGES - Enhanced condition to check Verimatrix network node ***************/		
    if ((node && (strstr(node,"NDS") || strstr(node,"VM") || strstr(node,"NAGRA") 
        || strstr(node,"GOSPEL"))) && package_type == 0 && flag == CANCEL_PACKAGE && is_change_bq_id == 1)
    {
        add_catv_bouquet_id_task(ctxp, i_flistpp, flag, 9, ebufp);
    }
	/****** VERIMATRIX CHANGES - Enhanced condition to check Verimatrix network node  **********/
    else if ((node && (strstr(node,"NDS") || strstr(node,"VM") ||
        strstr(node,"NAGRA") || strstr(node,"GOSPEL"))) && package_type == 0 && 
	flag == ADD_PACKAGE && is_change_bq_id == 1)
    {
        add_catv_bouquet_id_task(ctxp, i_flistpp, flag, 9, ebufp);
    }
    else
    {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No Need to Add BQ Id");
    }

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "add_package_detail enriched output flist after adding BQ id", *i_flistpp);

    if ( flag == CHANGE_PACKAGE)
    {
        i_copy_flistp = PIN_FLIST_COPY(*i_flistpp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"copy_flistp",i_copy_flistp);

        if ( (tmp_flistp = PIN_FLIST_ELEM_GET (i_copy_flistp,
          MSO_FLD_TASK, 2, 1, ebufp))!= (pin_flist_t *)NULL )
        {
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"tmp_flistp",tmp_flistp);
                rec_id = 0;
                cookie = NULL;
                while ( (del_flistp = PIN_FLIST_ELEM_GET_NEXT (tmp_flistp,
                        MSO_FLD_DEL_PACKAGE_INFO, &rec_id, 1,&cookie, ebufp)) != (pin_flist_t *)NULL )
                {
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"del_flistp",del_flistp);
                        del_ca_id = PIN_FLIST_FLD_GET(del_flistp, MSO_FLD_CA_ID, 1, ebufp);
                        tag  = PIN_FLIST_FLD_GET(del_flistp, PIN_FLD_PROVISIONING_TAG, 1, ebufp);
			if (tag)
			{
				drop_flistp = PIN_FLIST_ELEM_GET (*i_flistpp,
						MSO_FLD_TASK, 2, 0, ebufp);

				PIN_FLIST_ELEM_DROP(drop_flistp, MSO_FLD_DEL_PACKAGE_INFO, rec_id, ebufp);

				continue;
			}

                        if ( (tmp_flistp1 = PIN_FLIST_ELEM_GET (i_copy_flistp,
                                MSO_FLD_TASK, 1, 0, ebufp))!= (pin_flist_t *)NULL )
                        {
                                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"tmp_flistp1",tmp_flistp1);
                                rec_id1=0;
                                cookie1=NULL;
                                while ( (add_flistp = PIN_FLIST_ELEM_GET_NEXT (tmp_flistp1,
                                        MSO_FLD_ADD_PACKAGE_INFO, &rec_id1, 1,&cookie1, ebufp))
                                        != (pin_flist_t *)NULL )
                                {
                                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"add_flistp",add_flistp);
                                        add_ca_id  = PIN_FLIST_FLD_GET(add_flistp, MSO_FLD_CA_ID,
                                                        1, ebufp);
                                        if (add_ca_id && del_ca_id && strcmp(add_ca_id, del_ca_id) == 0)
                                        {
                                                drop_flistp = PIN_FLIST_ELEM_GET (*i_flistpp,
                                                                MSO_FLD_TASK, 2, 0, ebufp);

                                                PIN_ERR_LOG_FLIST(3,
                                                "drop del flist", drop_flistp);

                                                PIN_FLIST_ELEM_DROP(drop_flistp,
                                                        MSO_FLD_DEL_PACKAGE_INFO, rec_id, ebufp);
                                                PIN_ERR_LOG_FLIST(3,
                                                "drop del flist after drop", drop_flistp);

                                                drop_flistp1 = PIN_FLIST_ELEM_GET (*i_flistpp,
                                                                MSO_FLD_TASK, 1, 0, ebufp);

                                                PIN_ERR_LOG_FLIST(3,
                                                "drop add flist", drop_flistp1);

                                                PIN_FLIST_ELEM_DROP(drop_flistp1,
                                                       MSO_FLD_ADD_PACKAGE_INFO, rec_id1, ebufp);
                                                PIN_ERR_LOG_FLIST(3,
                                                "drop add flist after drop", drop_flistp1);
						if ( PIN_ERRBUF_IS_ERR(ebufp) ) {
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
             "add package error: required field missing in input flist", ebufp);
						}
						
                                        }
                                }
                        }
                }

        	tmp_flistp = PIN_FLIST_ELEM_GET(*i_flistpp, MSO_FLD_TASK, 2, 1, ebufp);
		if (tmp_flistp)
		{
                	rec_id = 0;
                	cookie = NULL;
			rec_id1 = 0;
                	while ( (del_flistp = PIN_FLIST_ELEM_TAKE_NEXT (tmp_flistp,
                        	MSO_FLD_DEL_PACKAGE_INFO, &rec_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL )
                	{
				if (add_flistp == NULL)
				{
					add_flistp = PIN_FLIST_CREATE(ebufp);
				}
		
				rec_id1++;
				PIN_FLIST_ELEM_PUT(add_flistp, del_flistp, MSO_FLD_DEL_PACKAGE_INFO, rec_id1, ebufp);
			}
			
			PIN_FLIST_CONCAT(tmp_flistp, add_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&add_flistp, NULL);
			add_flistp = NULL;
		}

        	tmp_flistp = PIN_FLIST_ELEM_GET(*i_flistpp, MSO_FLD_TASK, 1, 1, ebufp);
		if (tmp_flistp)
		{
                	rec_id = 0;
                	cookie = NULL;
			rec_id1 = 0;
                	while ( (del_flistp = PIN_FLIST_ELEM_TAKE_NEXT (tmp_flistp,
                        	MSO_FLD_ADD_PACKAGE_INFO, &rec_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL )
                	{
				if (add_flistp == NULL)
				{
					add_flistp = PIN_FLIST_CREATE(ebufp);
				}
		
				rec_id1++;
				PIN_FLIST_ELEM_PUT(add_flistp, del_flistp, MSO_FLD_ADD_PACKAGE_INFO, rec_id1, ebufp);
			}
			
			PIN_FLIST_CONCAT(tmp_flistp, add_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&add_flistp, NULL);
		}

        }
	
        if ( strstr(node, "NAGRA") && (tmp_flistp = PIN_FLIST_ELEM_GET (i_copy_flistp,
          MSO_FLD_TASK, 4, 1, ebufp))!= (pin_flist_t *)NULL )
        {
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"tmp_flistp",tmp_flistp);
                rec_id = 0;
                cookie = NULL;
                while ( (del_flistp = PIN_FLIST_ELEM_GET_NEXT (tmp_flistp,
                        PIN_FLD_DUNNING_INFO, &rec_id, 1,&cookie, ebufp)) != (pin_flist_t *)NULL )
                {
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"del_flistp",del_flistp);
                        del_ca_id = PIN_FLIST_FLD_GET(del_flistp, MSO_FLD_CA_ID, 1, ebufp);
                        tag  = PIN_FLIST_FLD_GET(del_flistp, PIN_FLD_PROVISIONING_TAG, 1, ebufp);
			if (tag)
			{
				drop_flistp = PIN_FLIST_ELEM_GET (*i_flistpp,
						MSO_FLD_TASK, 2, 0, ebufp);

				PIN_FLIST_ELEM_DROP(drop_flistp, MSO_FLD_DEL_PACKAGE_INFO, rec_id, ebufp);

				continue;
			}

                        if ( (tmp_flistp1 = PIN_FLIST_ELEM_GET (i_copy_flistp,
                                MSO_FLD_TASK, 3, 0, ebufp))!= (pin_flist_t *)NULL )
                        {
                                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"tmp_flistp1",tmp_flistp1);
                                rec_id1=0;
                                cookie1=NULL;
                                while ( (add_flistp = PIN_FLIST_ELEM_GET_NEXT (tmp_flistp1,
                                        PIN_FLD_OFFERINGS, &rec_id1, 1,&cookie1, ebufp))
                                        != (pin_flist_t *)NULL )
                                {
                                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"add_flistp",add_flistp);
                                        add_ca_id  = PIN_FLIST_FLD_GET(add_flistp, MSO_FLD_CA_ID,
                                                        1, ebufp);
                                        if (add_ca_id && del_ca_id && strcmp(add_ca_id, del_ca_id) == 0)
                                        {
                                                drop_flistp = PIN_FLIST_ELEM_GET (*i_flistpp,
                                                                MSO_FLD_TASK, 4, 0, ebufp);

                                                PIN_ERR_LOG_FLIST(3,
                                                "drop del flist", drop_flistp);

                                                PIN_FLIST_ELEM_DROP(drop_flistp,
                                                        PIN_FLD_DUNNING_INFO, rec_id, ebufp);
                                                PIN_ERR_LOG_FLIST(3,
                                                "drop del flist after drop", drop_flistp);

                                                drop_flistp1 = PIN_FLIST_ELEM_GET (*i_flistpp,
                                                                MSO_FLD_TASK, 3, 0, ebufp);

                                                PIN_ERR_LOG_FLIST(3,
                                                "drop add flist", drop_flistp1);

                                                PIN_FLIST_ELEM_DROP(drop_flistp1,
                                                       PIN_FLD_OFFERINGS, rec_id1, ebufp);
                                                PIN_ERR_LOG_FLIST(3,
                                                "drop add flist after drop", drop_flistp1);
						if ( PIN_ERRBUF_IS_ERR(ebufp) ) {
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
             						"add package error: required field missing in input flist", ebufp);
						}
						
                                        }
                                }
                        }
                }

        	tmp_flistp = PIN_FLIST_ELEM_GET(*i_flistpp, MSO_FLD_TASK, 4, 1, ebufp);
		if (tmp_flistp)
		{
                	rec_id = 0;
                	cookie = NULL;
			rec_id1 = 0;
                	while ( (del_flistp = PIN_FLIST_ELEM_TAKE_NEXT (tmp_flistp,
                        	PIN_FLD_DUNNING_INFO, &rec_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL )
                	{
				if (add_flistp == NULL)
				{
					add_flistp = PIN_FLIST_CREATE(ebufp);
				}
		
				rec_id1++;
				PIN_FLIST_ELEM_PUT(add_flistp, del_flistp, PIN_FLD_DUNNING_INFO, rec_id1, ebufp);
			}
			
			PIN_FLIST_CONCAT(tmp_flistp, add_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&add_flistp, NULL);
			add_flistp = NULL;
		}

        	tmp_flistp = PIN_FLIST_ELEM_GET(*i_flistpp, MSO_FLD_TASK, 3, 1, ebufp);
		if (tmp_flistp)
		{
                	rec_id = 0;
                	cookie = NULL;
			rec_id1 = 0;
                	while ( (del_flistp = PIN_FLIST_ELEM_TAKE_NEXT (tmp_flistp,
                        	PIN_FLD_OFFERINGS, &rec_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL )
                	{
				if (add_flistp == NULL)
				{
					add_flistp = PIN_FLIST_CREATE(ebufp);
				}
		
				rec_id1++;
				PIN_FLIST_ELEM_PUT(add_flistp, del_flistp, PIN_FLD_OFFERINGS, rec_id1, ebufp);
			}
			
			PIN_FLIST_CONCAT(tmp_flistp, add_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&add_flistp, NULL);
		}

        }
        if (*i_flistpp)
        {
                tmp_flistp = PIN_FLIST_ELEM_GET (*i_flistpp, MSO_FLD_TASK, 1, 0, ebufp);
                if(PIN_FLIST_ELEM_COUNT(tmp_flistp, MSO_FLD_ADD_PACKAGE_INFO, ebufp) == 0)
                {
                        PIN_FLIST_ELEM_DROP(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
                }
                else if(PIN_FLIST_ELEM_COUNT(tmp_flistp, MSO_FLD_ADD_PACKAGE_INFO, ebufp) == 1)
                {
                        PIN_ERR_LOG_MSG(3, "HANDLING COMBO PACKS");
                        tmp_args_flistp = PIN_FLIST_ELEM_TAKE(tmp_flistp, MSO_FLD_ADD_PACKAGE_INFO, PIN_ELEMID_ANY, 1, ebufp);
                        PIN_ERR_LOG_FLIST(3, "ADD_PACKAGEINFO flist", tmp_args_flistp);
                        PIN_FLIST_ELEM_PUT(tmp_flistp, tmp_args_flistp, MSO_FLD_ADD_PACKAGE_INFO, 1, ebufp);
                        PIN_ERR_LOG_FLIST(3, "TASK flist", tmp_flistp);
                }
                else
                {
                        //do nothing
                }
                tmp_flistp = PIN_FLIST_ELEM_GET (*i_flistpp, MSO_FLD_TASK, 2, 0, ebufp);
                if(PIN_FLIST_ELEM_COUNT(tmp_flistp, MSO_FLD_DEL_PACKAGE_INFO, ebufp) == 0)
                {
                        PIN_FLIST_ELEM_DROP(*i_flistpp, MSO_FLD_TASK, 2, ebufp);
                }
                else if(PIN_FLIST_ELEM_COUNT(tmp_flistp, MSO_FLD_DEL_PACKAGE_INFO, ebufp) == 1)
                {
                        PIN_ERR_LOG_MSG(3, "HANDLING COMBO PACKS");
                        tmp_args_flistp = PIN_FLIST_ELEM_TAKE(tmp_flistp, MSO_FLD_DEL_PACKAGE_INFO, PIN_ELEMID_ANY, 1, ebufp);
                        PIN_ERR_LOG_FLIST(3, "DEL_PACKAGEINFO flist", tmp_args_flistp);
                        PIN_FLIST_ELEM_PUT(tmp_flistp, tmp_args_flistp, MSO_FLD_DEL_PACKAGE_INFO, 1, ebufp);
                        PIN_ERR_LOG_FLIST(3, "TASK flist", tmp_flistp);
                }
                else
                {
                        // Do nothing
                }
        }
    }

cleanup:

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "add_package_detail enriched output flist", *i_flistpp);
    if (pkg_typep)
    {
	pin_free(pkg_typep);
    }
    PIN_FLIST_DESTROY_EX(&prod_in_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&prod_out_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&pur_prod_in_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&pur_prod_out_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&i_copy_flistp, NULL);

    return;
}

/*******************************************************************
 * mso_prov_get_ca_id()
 *
 * 1. gets CA_ID associated with a provisioning tag from
 *        /mso_cfg_catv_pt cache.
 *******************************************************************/
extern void
mso_prov_get_ca_id(
        pcm_context_t           *ctxp,
        char                    *prov_tag,
        char                    *node,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t     *cache_flistp = NULL;
        pin_flist_t     *tmp_flistp = NULL;
        pin_flist_t     *tmp_flistp_1 = NULL;
        pin_flist_t     *c_flistp = NULL;
        int32            err = PIN_ERR_NONE;
        cm_cache_key_iddbstr_t    kids;
        pin_cookie_t    cookie = NULL;
        int32           rec_id = 0;
        pin_cookie_t    cookie_1 = NULL;
        int32           rec_id_1 = 0;
        char            *network_node =NULL;
        char            *ca_id = NULL;
        int32           flg_ca_id_null = 1;
        time_t          last_mod_t = 0;
        time_t          now_t = 0;

       /******************************************************
         * Null out results until we have some.
         ******************************************************/
        if ( PIN_ERRBUF_IS_ERR(ebufp) )
                return;
        PIN_ERRBUF_CLEAR(ebufp);

	/*if (node && strcmp(node, "JVM1") == 0)
	{
		node[strlen(node)-1] = '\0';
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, node);
	}*/
        /******************************************************
        * If the cache is not enabled, short circuit right away
        ******************************************************/
        if ( mso_prov_catv_config_ptr == (cm_cache_t *)NULL ) {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
                "no config flist for /mso_cfg_catv_pt cached ");
                pin_set_err(ebufp, PIN_ERRLOC_CM,
                                PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_FLD_CACHE_NAME, 0, 0);

        }


        /**************************************************
         * See if the entry is in our data dictionary cache
         * Prepare the cm_cache_key_iddbstr_t structure to search
         **************************************************/
        kids.id = 0;
        kids.db = 0;
        kids.str = "/mso_cfg_catv_pt";
        cache_flistp = cm_cache_find_entry(mso_prov_catv_config_ptr,
                                        &kids, &err);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "cm_cache_find_entry found flist", cache_flistp);
        switch(err) {
        case PIN_ERR_NONE:
                break;
        default:
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
                        "mso_prov_catv_config_object from_cache: error "
                        "accessing data dictionary cache.");
                pin_set_err(ebufp, PIN_ERRLOC_CM,
                                PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_FLD_CACHE_NAME, 0, 0);
                goto cleanup;
        }

	// Addding check to remove core dump
	if(cache_flistp && (time_t*)PIN_FLIST_FLD_GET(cache_flistp, PIN_FLD_MOD_T,0,ebufp) != NULL )
        last_mod_t = *(time_t*)PIN_FLIST_FLD_GET(cache_flistp, PIN_FLD_MOD_T,0,ebufp);

        now_t = pin_virtual_time((time_t *)NULL);
        if(now_t  >= last_mod_t + refresh_pt_interval )
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"calling mso_prov_catv_config_object_update to refresh provisioning tag config");
                fm_mso_prov_catv_config_cache_update(ctxp, ebufp);
                kids.id = 0;
                kids.db = 0;
                kids.str = "/mso_cfg_catv_pt";
                cache_flistp = cm_cache_find_entry(mso_prov_catv_config_ptr,&kids, &err);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                "cm_cache_find_entry found flist", cache_flistp);
                switch(err)
                {
                        case PIN_ERR_NONE:
                                break;
                        default:
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
                                        "mso_prov_catv_config_object from_cache: error "
                                        "accessing data dictionary cache.");
                                pin_set_err(ebufp, PIN_ERRLOC_CM,
                                                PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_FLD_CACHE_NAME, 0, 0);
                                goto cleanup;
                }
        }

        while ( (tmp_flistp = PIN_FLIST_ELEM_GET_NEXT (cache_flistp,
                PIN_FLD_RESULTS, &rec_id, 1,&cookie, ebufp))!= (pin_flist_t *)NULL )
        {
                //PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "inside loop");

		// Addding check for existence of PROVISIONING_TAG to prevent core dump from subsequenct operation 	
		if ((char *) PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_PROVISIONING_TAG, 0, ebufp ) != NULL)	
		{
                if (strcmp((char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_PROVISIONING_TAG, 0, ebufp ),prov_tag) == 0 )
                {
                        c_flistp = PIN_FLIST_ELEM_GET(cache_flistp,PIN_FLD_RESULTS, rec_id, 0, ebufp);
			 
			if (c_flistp == NULL )
			{
				 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
						 "mso_prov_catv_config_object from_cache: error "
						 "accessing data dictionary cache.");
					 pin_set_err(ebufp, PIN_ERRLOC_CM,
                                                 PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_FLD_CACHE_NAME, 0, 0);
					 goto cleanup;
			}

                        rec_id_1 = 0;
                        cookie_1 = NULL;
                        while ((tmp_flistp_1 = PIN_FLIST_ELEM_GET_NEXT (c_flistp,
                                MSO_FLD_CATV_PT_DATA, &rec_id_1, 0,&cookie_1, ebufp))!= (pin_flist_t *)NULL )
                        {
                                network_node = PIN_FLIST_FLD_GET(tmp_flistp_1, MSO_FLD_NETWORK_NODE, 1, ebufp);

                                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"tmp_flistp_1",tmp_flistp_1);
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "network_node");
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, network_node);
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "node");
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, node);
                                if (network_node && (network_node != NULL) && node && (strcmp(network_node,node)==0) )
                                {
                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "network_node=node");
                                        ca_id =  (char *)PIN_FLIST_FLD_GET(tmp_flistp_1, MSO_FLD_CA_ID, 1, ebufp);
                                        if (ca_id && (ca_id != NULL ) && (strcmp(ca_id, "")!=0) )
                                        {
                                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "MSO_FLD_CATV_PT_DATA and CA_ID exist");
                                                PIN_FLIST_FLD_COPY(tmp_flistp_1, MSO_FLD_CA_ID, c_flistp, MSO_FLD_CA_ID, ebufp);
                                                flg_ca_id_null =0;
                                                break;
                                        }
                                }
                        }
                }
		}
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"mso_prov_catv_config_object for the provisioning tag",c_flistp);

        if ( c_flistp == NULL )
        {
                /*
                * Set Error
                */
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "c_flistp is null");
                pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, MSO_FLD_CA_ID, 0, 0);
        }
        else if ( flg_ca_id_null )
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "flg_ca_id_null=1");
                pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, MSO_FLD_CA_ID, 0, 0);
        }

        cleanup:
        if ( !cache_flistp )
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CATV prov cache Information not available in the cache.");
                *ret_flistpp = NULL;
        }
        else
        {
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"c_flistp", c_flistp)
                *ret_flistpp = PIN_FLIST_COPY(c_flistp, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_prov_get_ca_id return flist", *ret_flistpp);
        }
        PIN_FLIST_DESTROY_EX(&cache_flistp, NULL);

        return;
}

static void
prepare_catv_email_payload(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    int32                   *error_codep,
    char                    **act,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *si_flistp = NULL;
    pin_flist_t             *so_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    pin_flist_t             *task_flistp = NULL;
    char                    *ne = NULL;
    char                    *address = NULL;
    char                    *msg = NULL;
    char                    *sub = NULL;
    char                    *emmg_del_date = NULL;
    char                    *stb_del_date = NULL;
    int32                   *mailbox_code = NULL;
    int32                   elem_id = 1;
    int32                   *action_flag = NULL;

    pin_flist_t             *temp_flistp = NULL;
    pin_flist_t             *tempp_flistp= NULL;
    pin_flist_t             *sub_flistp = NULL;
    char                    *msg_grp_name = NULL;
    char                    *subs_id = NULL;
    int32                   rec_id = 0;
    pin_cookie_t            cookie = NULL;
    pin_cookie_t            prev_cookie = 0;
    int32                   msg_count;
    int32                   title_count;
    int32                   stb_count;
	pin_flist_t		*in_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;
    char 			*vm_id = NULL;
	
	char text[255];

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_email_payload input flist", *i_flistpp);

    /*
     * Validate input received
     */

    get_subscriber_id(ctxp, i_flistpp,ebufp);
    ne = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_NETWORK_NODE, 0, ebufp);
	
	sprintf(text, "ne = %s", ne);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, text);
	
    // Sachid: Added action_flag to identify if the req is for preactivated svc or not
    action_flag = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_FLAGS, 0, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        *error_codep = 52101;
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "prepare_catv_email_payload error: required field missing in input flist", ebufp);
        return;
    }
    if(strcmp(ne, "NONE") == 0)
    {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "No need to generate the prov request ");
    }
    else
    {
		if ( ne && strstr(ne,"NDS") )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "I am at NDS");
			/*
			 * Validate input received
			 */

			address = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_ADDRESS, 0, ebufp);

			if((strncmp(address, "T0", 2) == 0 ) || (strncmp(address, "Z0", 2)== 0))
			{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Address is not VC ");
			}
			else
			{
					subs_id = PIN_FLIST_FLD_GET(*i_flistpp,MSO_FLD_CAS_SUBSCRIBER_ID, 1, ebufp);
			}

			mailbox_code = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_CODE_FOR_MAILBOX, 0, ebufp);
			stb_del_date = PIN_FLIST_FLD_GET(*i_flistpp,MSO_FLD_STB_DELETE_DATE, 1, ebufp);
			emmg_del_date = PIN_FLIST_FLD_GET(*i_flistpp,MSO_FLD_EMMG_DELETION_DATE, 1, ebufp);
			tmp_flistp = PIN_FLIST_ELEM_GET(*i_flistpp, PIN_FLD_MESSAGES, PIN_ELEMID_ANY, 0 , ebufp);
			msg = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_MESSAGE, 0, ebufp);
			sub = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_SUBJECT, 0, ebufp);


			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				*error_codep = 52101;
				return;
			}

			*act = (char *) malloc(sizeof("EMAIL_NDS"));
			strcpy(*act,"EMAIL_NDS");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );

				/***************** TASK 1 **********************/
			task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
			PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "EMAIL", ebufp);

					if(subs_id != NULL)
			{
					 PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ADDRESS, subs_id,ebufp);
			}
			else
			{
					PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ADDRESS, address, ebufp);
			}


			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_EMMG_DELETION_DATE, emmg_del_date, ebufp);
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_STB_DELETE_DATE, stb_del_date, ebufp);
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_CODE_FOR_MAILBOX, mailbox_code, ebufp);

			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_EMAIL_DELETABLE, "0", ebufp);
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_REGULAR_EMAIL, "0", ebufp);
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_EMAIL_FORMAT_TYPE, "0", ebufp);
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_EMAIL_LANG_TYPE, "0", ebufp);

					//to insert loop if required
			while ( (temp_flistp = PIN_FLIST_ELEM_GET_NEXT (*i_flistpp,
							PIN_FLD_MESSAGES, &rec_id, 1,&cookie, ebufp))
													  != (pin_flist_t *)NULL )
			{
				  sub_flistp = PIN_FLIST_ELEM_ADD(task_flistp, PIN_FLD_MESSAGES, elem_id++, ebufp);
				  PIN_FLIST_FLD_COPY(temp_flistp, PIN_FLD_MESSAGE, sub_flistp, PIN_FLD_MESSAGE, ebufp);
				  PIN_FLIST_FLD_SET(sub_flistp, MSO_FLD_HIGH_BYTE_APPEAR, "0", ebufp);
				  PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_MESSAGE, msg, ebufp);
				  PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_SUBJECT, sub, ebufp);
				  PIN_FLIST_ELEM_DROP(*i_flistpp, PIN_FLD_MESSAGES, rec_id, ebufp);
				  cookie = prev_cookie;
			}//end while

			PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_ADDRESS, ebufp);
			PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_EMMG_DELETION_DATE, ebufp);
			PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_STB_DELETE_DATE, ebufp);
			PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_CODE_FOR_MAILBOX, ebufp);
	//        PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

		}
		else if (ne  && strstr(ne,"CISCO") )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "I am at CISCO");
			/*
			 * Validate input received
			 */
			stb_count = PIN_FLIST_ELEM_COUNT(*i_flistpp, MSO_FLD_STB_ID_LIST, ebufp);
			msg_grp_name = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_MSG_GRP_NAME, 1, ebufp);
			msg_count = PIN_FLIST_ELEM_COUNT(*i_flistpp, MSO_FLD_MSG_CONTENT_LIST, ebufp);
			title_count = PIN_FLIST_ELEM_COUNT(*i_flistpp, MSO_FLD_MSG_TITLE_LIST, ebufp);

			if ( (stb_count == 0 && !msg_grp_name ) ||//one of the above two input is mandatory
					!msg_count ||
					!title_count )
			{
				*error_codep = 52101;
				pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
						PIN_ERR_BAD_VALUE, 0, 0, 0);
				return;
			}

			*act = (char *) malloc(sizeof("EMAIL_PK"));
			strcpy(*act,"EMAIL_PK");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );

				/***************** TASK 1 **********************/
			task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
			PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "EMAIL", ebufp);

				//default values
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_AUDIO_FLAG, "AF_None", ebufp);
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_DISPLAY_FLAG, "DF_Text", ebufp);
			if (strcmp(ne, "CISCO") == 0)
			{
				PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_MMM_CFG_NAME, "HW_NORMAL", ebufp);
			}
			else
			{
				PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_MMM_CFG_NAME, "BMAIL-30-6", ebufp);
			}

					//to insert loop if required
			while ( (temp_flistp = PIN_FLIST_ELEM_GET_NEXT (*i_flistpp,
							MSO_FLD_STB_ID_LIST, &rec_id, 1,&cookie, ebufp))
													  != (pin_flist_t *)NULL )
			{
				  sub_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_STB_ID_LIST, elem_id++, ebufp);
				  PIN_FLIST_FLD_COPY(temp_flistp, MSO_FLD_STB_ID, sub_flistp, MSO_FLD_STB_ID, ebufp);
				  PIN_FLIST_ELEM_DROP(*i_flistpp, MSO_FLD_STB_ID_LIST, rec_id, ebufp);
				  cookie = prev_cookie;
			}//end while

				//to insert loop if required
			elem_id = 1;
			while ( (temp_flistp = PIN_FLIST_ELEM_GET_NEXT (*i_flistpp,
							MSO_FLD_MSG_CONTENT_LIST, &rec_id, 1,&cookie, ebufp))
													  != (pin_flist_t *)NULL )
			{
				  sub_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_MSG_CONTENT_LIST, elem_id++, ebufp);
				  PIN_FLIST_FLD_SET(sub_flistp, MSO_FLD_LANG_CODE, "ENG", ebufp);
				  PIN_FLIST_FLD_COPY(temp_flistp, PIN_FLD_MESSAGE, sub_flistp, PIN_FLD_MESSAGE, ebufp);
				  PIN_FLIST_ELEM_DROP(*i_flistpp, MSO_FLD_MSG_CONTENT_LIST, rec_id, ebufp);
				  cookie = prev_cookie;
			}//end while

			if (msg_grp_name)
			{
				PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_MSG_GRP_NAME, msg_grp_name, ebufp);
				PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_MSG_GRP_NAME, ebufp);
			}


				//to insert loop if required
			elem_id = 1;
			while ( (temp_flistp = PIN_FLIST_ELEM_GET_NEXT (*i_flistpp,
							MSO_FLD_MSG_TITLE_LIST, &rec_id, 1,&cookie, ebufp))
													  != (pin_flist_t *)NULL )
			{
				  sub_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_MSG_TITLE_LIST, elem_id++, ebufp);
				  PIN_FLIST_FLD_SET(sub_flistp, MSO_FLD_LANG_CODE, "ENG", ebufp);
				  PIN_FLIST_FLD_COPY(temp_flistp, PIN_FLD_TITLE, sub_flistp, PIN_FLD_TITLE, ebufp);
				  PIN_FLIST_ELEM_DROP(*i_flistpp, MSO_FLD_MSG_TITLE_LIST, rec_id, ebufp);
				  cookie = prev_cookie;
			}//end while

	//        PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);
		}
		/**************** VERIMATRIX CHANGES - BEGIN *********************
		 * for catv email payload ***************************************/
	    else if ( ne && strstr(ne,"VM") )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "I am at VM");
			/*
			 * Validate input received
			 */

			in_flistp  = PIN_FLIST_CREATE(ebufp);
			//tempp_flistp = PIN_FLIST_ELEM_GET(so_flistp,PIN_FLD_ALIAS_LIST,0, 1, ebufp);
			PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, in_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_ADDRESS, in_flistp, PIN_FLD_NAME, ebufp);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"s_no input flist", in_flistp);
			
			search_serial_no(ctxp, in_flistp, &ret_flistp,ebufp);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"s_no output flist", ret_flistp);

			address = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_ADDRESS, 0, ebufp);

			if((strncmp(address, "T0", 2) == 0 ) || (strncmp(address, "Z0", 2)== 0))
			{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Address is not VM ");
			}
			else
			{
				   // subs_id = PIN_FLIST_FLD_GET(*i_flistpp,MSO_FLD_CAS_SUBSCRIBER_ID, 1, ebufp);
			}

			mailbox_code = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_CODE_FOR_MAILBOX, 0, ebufp);
			stb_del_date = PIN_FLIST_FLD_GET(*i_flistpp,MSO_FLD_STB_DELETE_DATE, 1, ebufp);
			emmg_del_date = PIN_FLIST_FLD_GET(*i_flistpp,MSO_FLD_EMMG_DELETION_DATE, 1, ebufp);
			tmp_flistp = PIN_FLIST_ELEM_GET(*i_flistpp, PIN_FLD_MESSAGES, PIN_ELEMID_ANY, 0 , ebufp);
			msg = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_MESSAGE, 0, ebufp);
			sub = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_SUBJECT, 0, ebufp);
			
			tempp_flistp = PIN_FLIST_ELEM_GET(ret_flistp,PIN_FLD_RESULTS,0,1,ebufp);
			
			if((tempp_flistp = PIN_FLIST_ELEM_GET(ret_flistp,PIN_FLD_RESULTS,0,1,ebufp)) != NULL)
			{
			   vm_id = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_ADDRESS, 1, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " indise if not null loop VM ");
			}
			
			if(vm_id)
			{
			 //VM STB NUMBER
			 PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_ADDRESS, *i_flistpp, MSO_FLD_STB_ID, ebufp);
			 PIN_FLIST_FLD_SET(*i_flistpp,MSO_FLD_STB_ID,vm_id,ebufp);

			  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " indise if loop VM ");
			  
			 //VM  serial number
			  PIN_FLIST_FLD_COPY(tempp_flistp, MSO_FLD_SERIAL_NO, *i_flistpp, MSO_FLD_VC_ID, ebufp);
			 
			 //VM device type	 
			 PIN_FLIST_FLD_COPY(tempp_flistp, PIN_FLD_DEVICE_TYPE, *i_flistpp, PIN_FLD_DEVICE_TYPE, ebufp);

			}
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				*error_codep = 52101;
				return;
			}

			if (strstr(ne, "JVM"))
			{
				*act = (char *) malloc(sizeof("EMAIL_JVM"));
				strcpy(*act,"EMAIL_JVM");
			}
			else
			{
				*act = (char *) malloc(sizeof("EMAIL_VM"));
				strcpy(*act,"EMAIL_VM");
			}
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );

				/***************** TASK 1 **********************/
			task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
			PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "EMAIL", ebufp);

					if(subs_id != NULL)
			{
					 PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ADDRESS, subs_id,ebufp);
			}
			else
			{
					PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ADDRESS, address, ebufp);
			}


			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_EMMG_DELETION_DATE, emmg_del_date, ebufp);
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_STB_DELETE_DATE, stb_del_date, ebufp);
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_CODE_FOR_MAILBOX, mailbox_code, ebufp);

			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_EMAIL_DELETABLE, "0", ebufp);
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_REGULAR_EMAIL, "0", ebufp);
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_EMAIL_FORMAT_TYPE, "0", ebufp);
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_EMAIL_LANG_TYPE, "0", ebufp);

					//to insert loop if required
			while ( (temp_flistp = PIN_FLIST_ELEM_GET_NEXT (*i_flistpp,
							PIN_FLD_MESSAGES, &rec_id, 1,&cookie, ebufp))
													  != (pin_flist_t *)NULL )
			{
				  sub_flistp = PIN_FLIST_ELEM_ADD(task_flistp, PIN_FLD_MESSAGES, elem_id++, ebufp);
				  PIN_FLIST_FLD_COPY(temp_flistp, PIN_FLD_MESSAGE, sub_flistp, PIN_FLD_MESSAGE, ebufp);
				  PIN_FLIST_FLD_SET(sub_flistp, MSO_FLD_HIGH_BYTE_APPEAR, "0", ebufp);
				  PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_MESSAGE, msg, ebufp);
				  PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_SUBJECT, sub, ebufp);
				  PIN_FLIST_ELEM_DROP(*i_flistpp, PIN_FLD_MESSAGES, rec_id, ebufp);
				  cookie = prev_cookie;
			}//end while

			PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_ADDRESS, ebufp);
			PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_EMMG_DELETION_DATE, ebufp);
			PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_STB_DELETE_DATE, ebufp);
			PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_CODE_FOR_MAILBOX, ebufp);
	//        PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

		}
		/**************** VERIMATRIX CHANGES - END *********************
		 * for catv email payload ***************************************/		
		else
		{
			*error_codep = 52102;
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
		}
    }

    /*
     * Enrich with additional details required by Provisioning system
     */


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_catv_email_payload enriched input flist", *i_flistpp);
				

// PIN_FLIST_DESTROY_EX(&in_flistp,ebufp);
 
 return;

}

static void
prepare_catv_retrack_payload(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    int32                   *error_codep,
    char                    **act,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *search_i_flistp = NULL;
    pin_flist_t             *search_o_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    pin_flist_t             *tmp1_flistp = NULL;
    pin_flist_t             *task_flistp = NULL;
    pin_flist_t             *task_flistp2 = NULL;
    pin_flist_t		*lbo_flistp = NULL;
    pin_flist_t		*task_serflistp = NULL;
    char                    *ne = NULL;
    char                    *cas_subs_id = NULL;
    char		*den_nwp = "NDS1";
	char		*dsn_nwp = "NDS2";
    char		*hw_nwp = "NDS";
    poid_t                  *s_pdp = NULL;
    poid_t                  *pdp = NULL;
    poid_t                  *pop_idp = NULL;
    char                    template[1024] = {0};
    int32                   s_flags = 0;
    int64                   database = 0;
    int32		den_sp = 0;
    char                    *vc_id = NULL;
    char                    *stb_id = NULL;
    char                    *node = NULL;
    char                    *device_type = NULL;
    char                *population_id = "0001";
    char		*area_codep = NULL;
    char            *region_key = NULL;
    char		*vip = "0";
    char		*bouquet_id = NULL;
    char		*exp_bouquet_id = NULL;
    int32                   *state_id = NULL;
    int32                   status_active = 0;
    void                     *vp = NULL;
    pin_flist_t		         *in_flistp = NULL;
	pin_flist_t	 *tempp_flistp = NULL;
	pin_flist_t	 *ret_flistp = NULL;
	
	char 		msg[255];
	int32       task_id = 0;
	int32		profile_srch_type = MSO_FLAG_SRCH_BY_SELF_ACCOUNT;
	int32		device_hd_flag = 0;
	
	struct		tm *current_time;
	time_t		now_t = 0;
	int32		year;
	int32		month;
	int32		day;
	char		validity[21];
	pin_flist_t		*name_flistp = NULL;
	pin_flist_t		*name_res_flistp = NULL;
	pin_flist_t		*return_flistp = NULL;
	pin_flist_t		*input_flistp = NULL;


    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_retrack_payload input flist", *i_flistpp);

    /*
     * Validate input received
     */
   
	stb_id = PIN_FLIST_FLD_GET( *i_flistpp, MSO_FLD_STB_ID, 1, ebufp);
	vc_id = (char *)PIN_FLIST_FLD_GET( *i_flistpp, MSO_FLD_VC_ID, 1, ebufp);
	node = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_NETWORK_NODE, 1, ebufp);
	

    if ((!stb_id && !vc_id) || PIN_ERRBUF_IS_ERR(ebufp) )
    {
        *error_codep = 52101;
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
            "prepare_catv_retrack_payload error: required field missing in input flist");
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NOT_FOUND, PIN_FLD_DEVICE_ID, 0, 0);
        return;
    }

    if(node && strcmp(node, "NONE") == 0)
    {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "No need to generate the prov request ");
    }
    else
    {
		if ((node && strstr(node, "NDS") == NULL) && !stb_id && vc_id)
		{
			stb_id = PIN_FLIST_FLD_GET( *i_flistpp, MSO_FLD_VC_ID, 0, ebufp);
			PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_STB_ID, stb_id, ebufp);
		}
		if (vc_id && strstr(node, "NDS"))
		{
			fm_mso_get_device_details(ctxp,i_flistpp,vc_id,ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"prepare_catv_activation_flist device details return", *i_flistpp);
		}
		else if (stb_id)
		{
			fm_mso_get_device_details(ctxp,i_flistpp,stb_id,ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"prepare_catv_activation_flist device details return", *i_flistpp);
		}
		else
		{
			*error_codep = 52104;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
				"prepare_catv_retrack_payload error: incorrect input in input flist");
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_NOT_FOUND, MSO_FLD_NETWORK_NODE, 0, 0);
			return;
		}
		state_id = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_STATE_ID, 0, ebufp);
		stb_id = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_STB_ID, 1, ebufp);

		//PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_DEVICE_ID, ebufp);
		PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_STATE_ID, ebufp);		
		
		
		if(state_id && *state_id == 6)
		{
			/*
			* Enrich with additional service details
			*/
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Entered in pre activated block");
			pdp = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_POID, 0, ebufp);
			database = PIN_POID_GET_DB(pdp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR ," Error set " , ebufp);
				PIN_ERRBUF_RESET(ebufp);
			}
			if (vc_id)
			{
				search_i_flistp = PIN_FLIST_CREATE(ebufp);
				s_pdp = (poid_t *)PIN_POID_CREATE(database, "/search", -1, ebufp);
				PIN_FLIST_FLD_PUT(search_i_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
				s_flags = SRCH_DISTINCT ;
				PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

				strcpy(template, "select X from /mso_catv_preactivated_svc 1, /device/vc 2  where 2.F1 = V1 and 1.F2 = 2.F3 and 1.F4 = V4 ");
				PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, template, ebufp);
				tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
				PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_DEVICE_ID, vc_id, ebufp);

				tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
				PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_VC_OBJ, NULL, ebufp);

				tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 3, ebufp);
				PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_POID, NULL, ebufp);

				tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 4, ebufp);
				PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STATUS, &status_active, ebufp);

				tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
						"prepare_catv_retrack_payload PCM_OP_SEARCH input flist", search_i_flistp);
				PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_i_flistp, &search_o_flistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
						"prepare_catv_retrack_payload PCM_OP_SEARCH output flist", search_o_flistp);

				tmp_flistp = PIN_FLIST_ELEM_GET(search_o_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1,ebufp);

				ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);
				PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_NETWORK_NODE, ne,ebufp )
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, " error ");
						*error_codep = 52104;
						pin_set_err(ebufp, PIN_ERRLOC_FM,
								PIN_ERRCLASS_SYSTEM_DETERMINATE,
										PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
						goto CLEANUP;
				}
				cas_subs_id = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, 0, ebufp);

			}
			else if(stb_id)
			{
				ne = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_NETWORK_NODE, 0, ebufp);
			}
		}
		else
		{
			den_sp = fm_mso_is_den_sp(ctxp, *i_flistpp, ebufp);
			/*
			 * Enrich with additional service details
			 */ 
		 
			pdp = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_POID, 0, ebufp);
			database = PIN_POID_GET_DB(pdp);

			search_i_flistp = PIN_FLIST_CREATE(ebufp);
			s_pdp = (poid_t *)PIN_POID_CREATE(database, "/search", -1, ebufp);
			PIN_FLIST_FLD_PUT(search_i_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

			s_flags = SRCH_DISTINCT ;
			PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

			if (vc_id && strncmp(vc_id, "000", 3) == 0)
			{
				tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
				tmp1_flistp = PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_ALIAS_LIST, 1, ebufp); //NDS - Smart card
				PIN_FLIST_FLD_SET(tmp1_flistp, PIN_FLD_NAME, vc_id, ebufp);

				tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
				if (den_sp == 1)
				{
					PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_MANUFACTURER, den_nwp, ebufp);
				}
				else if (den_sp == 2)
				{
					PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_MANUFACTURER, dsn_nwp, ebufp);
				}
				else
				{
					PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_MANUFACTURER, hw_nwp, ebufp);
				}

				tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 3, ebufp);
				PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_DEVICE_ID, vc_id, ebufp);
			
				tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 4, ebufp);
				PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_POID, NULL, ebufp);

				tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 5, ebufp);
				tmp1_flistp = PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_SERVICES, 0, ebufp);
				PIN_FLIST_FLD_SET(tmp1_flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);

				strcpy(template, "select X from /service 1, /device 2 where 1.F1 = V1 and 2.F2 = V2 and 2.F3 = 1.F1 and 1.F4 = 2.F5 ");
			}
			else
			{
				tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
				tmp1_flistp = PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_ALIAS_LIST, 2, ebufp); //CISCO - Mac adddress
				if (vc_id)
				{
					PIN_FLIST_FLD_SET(tmp1_flistp, PIN_FLD_NAME, vc_id, ebufp);
				}
				else
				{
					PIN_FLIST_FLD_SET(tmp1_flistp, PIN_FLD_NAME, stb_id, ebufp);
				}
	
				strcpy(template, "select X from /service where F1 = V1 ");
			}
			PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, template, ebufp);

			if (!vc_id)
			{
				tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
				tmp1_flistp = PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_ALIAS_LIST, 2, ebufp); //CISCO - Mac adddress
				PIN_FLIST_FLD_SET(tmp1_flistp, PIN_FLD_NAME, stb_id, ebufp);
			}

			tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
			PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
			
			tmp1_flistp = PIN_FLIST_SUBSTR_ADD(tmp_flistp, MSO_FLD_CATV_INFO, ebufp);
			PIN_FLIST_FLD_SET(tmp1_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, NULL, ebufp);
			PIN_FLIST_FLD_SET(tmp1_flistp, MSO_FLD_NETWORK_NODE, NULL, ebufp);
			PIN_FLIST_FLD_SET(tmp1_flistp, MSO_FLD_BOUQUET_ID, NULL, ebufp);
            PIN_FLIST_FLD_SET(tmp1_flistp, MSO_FLD_BOUQUET_ID_HEX, NULL, ebufp);
			PIN_FLIST_FLD_SET(tmp1_flistp, MSO_FLD_REGION_KEY, NULL, ebufp);
			PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_ALIAS_LIST, 0, ebufp);
			PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_ALIAS_LIST, 1, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"prepare_catv_retrack_payload PCM_OP_SEARCH input flist", search_i_flistp);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_i_flistp, &search_o_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"prepare_catv_retrack_payload PCM_OP_SEARCH output flist", search_o_flistp);

			tmp1_flistp = PIN_FLIST_SUBSTR_GET(search_o_flistp, PIN_FLD_RESULTS, 0, ebufp);
			tmp_flistp = PIN_FLIST_SUBSTR_GET(tmp1_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
			ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);
			PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_NETWORK_NODE, ne,ebufp )

			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
					*error_codep = 52104;
					pin_set_err(ebufp, PIN_ERRLOC_FM,
							PIN_ERRCLASS_SYSTEM_DETERMINATE,
									PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
					goto CLEANUP;
			}
			cas_subs_id = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, 0, ebufp);
			bouquet_id = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_BOUQUET_ID, 0, ebufp);
			exp_bouquet_id = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_BOUQUET_ID_HEX, 1, ebufp);
		}

		if ( ne && strstr(ne,"NDS") && (vc_id || stb_id))
		{
			*act = (char *) malloc(sizeof("RETRACK_NDS"));
			strcpy(*act,"RETRACK_NDS");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
						//add cas subscriber id

			if ( cas_subs_id && strlen(cas_subs_id) > 0)
			{
				PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_CAS_SUBSCRIBER_ID, cas_subs_id, ebufp);
			}
			else
			{
				*error_codep = 52103;
				pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
						PIN_ERR_BAD_VALUE, MSO_FLD_CAS_SUBSCRIBER_ID, 0, 0);
				goto CLEANUP;
			}

			/***************** TASK 1 **********************/
			task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
			PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "RETRACK", ebufp);

			fm_mso_get_purchased_prod_nds(ctxp,tmp1_flistp,&lbo_flistp,ebufp);
                        if (PIN_FLIST_ELEM_COUNT(lbo_flistp, PIN_FLD_RESULTS, ebufp) > 0)
                        {

                        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 2, ebufp);
                        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "AUTHORIZE_LBO_SERVICES", ebufp);
                                                task_serflistp = PIN_FLIST_ELEM_ADD(task_flistp, PIN_FLD_SERVICES, 1, ebufp);
                                                PIN_FLIST_FLD_COPY(lbo_flistp, MSO_FLD_CA_ID, task_serflistp, MSO_FLD_CA_ID, ebufp);
                                                PIN_FLIST_FLD_COPY(lbo_flistp, MSO_FLD_TAPING_AUTHORIZATION, task_serflistp, MSO_FLD_TAPING_AUTHORIZATION, ebufp);
                                                PIN_FLIST_FLD_COPY(lbo_flistp, MSO_FLD_EXPIRATION_DATE, task_serflistp, MSO_FLD_EXPIRATION_DATE, ebufp);


                        }

			vp = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_VC_ID, 1, ebufp);

			if (vp)
			{
				PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_VC_ID, ebufp);
			}
	//        PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);
		}
		else if (ne  && strstr(ne,"CISCO") && stb_id)
		{
			*act = (char *) malloc(sizeof("RETRACK_PK"));
			strcpy(*act,"RETRACK_PK");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );

				//set mac address
	//        tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 2, 0, ebufp);
	//        PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_NAME, *i_flistpp, MSO_FLD_STB_ID, ebufp);

				/***************** TASK 1 **********************/
			task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
			PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "INSTANT_HIT", ebufp);

	//        PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);
		}
		else if (ne  && strstr(ne, "NAGRA") && stb_id)
		{
			*act = (char *) malloc(sizeof("RETRACK_NGR"));
			strcpy(*act,"RETRACK_NGR");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );	

			if ( cas_subs_id && strlen(cas_subs_id) > 0)
			{
				PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_CAS_SUBSCRIBER_ID, cas_subs_id, ebufp);
			}
			else
			{
				*error_codep = 52103;
				pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_BAD_VALUE, MSO_FLD_CAS_SUBSCRIBER_ID, 0, 0);
				goto CLEANUP;
			}

			task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "INITIALIZE_ICC", ebufp);

			fm_mso_get_device_details(ctxp, i_flistpp, stb_id, ebufp);

			device_type = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_DEVICE_TYPE, 1, ebufp);

			if (device_type)
			{
				device_hd_flag = *device_type;
			}
			PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_DEVICE_TYPE, ebufp);

			in_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(tmp1_flistp, PIN_FLD_ACCOUNT_OBJ, in_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(tmp1_flistp, PIN_FLD_ACCOUNT_OBJ, in_flistp, PIN_FLD_OBJECT, ebufp);
			PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_PROFILE_DESCR, CUSTOMER_CARE, ebufp);
			PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_SEARCH_TYPE, &profile_srch_type, ebufp);

			fm_mso_get_profile_info(ctxp, in_flistp, &return_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&in_flistp, NULL);

			input_flistp = PIN_FLIST_SUBSTR_GET(return_flistp, PIN_FLD_CUSTOMER_CARE_INFO, 0, ebufp);

                        in_flistp = PIN_FLIST_CREATE(ebufp);;
                        PIN_FLIST_FLD_COPY(tmp1_flistp, PIN_FLD_ACCOUNT_OBJ, in_flistp, PIN_FLD_POID, ebufp);
                        PIN_FLIST_FLD_COPY(tmp1_flistp, PIN_FLD_POID, *i_flistpp, PIN_FLD_SERVICE_OBJ, ebufp);
                        PIN_FLIST_FLD_COPY(tmp1_flistp, PIN_FLD_ACCOUNT_OBJ, *i_flistpp, PIN_FLD_ACCOUNT_OBJ, ebufp);
                        PIN_FLIST_FLD_COPY(tmp1_flistp, PIN_FLD_ACCOUNT_OBJ, *i_flistpp, PIN_FLD_POID, ebufp);
                        PIN_FLIST_FLD_COPY(tmp1_flistp, PIN_FLD_ACCOUNT_OBJ, in_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
                        PIN_FLIST_FLD_COPY(input_flistp, MSO_FLD_LCO_OBJ, in_flistp, MSO_FLD_LCO_OBJ, ebufp);
                        tmp_flistp = PIN_FLIST_SUBSTR_GET(tmp1_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
                        PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, in_flistp, MSO_FLD_NETWORK_NODE, ebufp);

                        PIN_FLIST_DESTROY_EX(&return_flistp, NULL);
                        search_name_info(ctxp, in_flistp, &return_flistp, ebufp);

                        input_flistp = PIN_FLIST_ELEM_GET(return_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
                        PIN_FLIST_FLD_COPY(input_flistp, MSO_FLD_AREA, in_flistp, MSO_FLD_AREA, ebufp);
                        name_flistp = PIN_FLIST_ELEM_GET(input_flistp,PIN_FLD_NAMEINFO,PIN_ELEMID_ANY,0,ebufp);
			            area_codep = PIN_FLIST_FLD_TAKE(name_flistp, PIN_FLD_ZIP, 0, ebufp);
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                "prepare_catv_retrack_payload service flist", tmp1_flistp);
                        PIN_FLIST_ELEM_SET(in_flistp, tmp1_flistp, PIN_FLD_SERVICES, 0, ebufp);

                        PIN_FLIST_DESTROY_EX(&return_flistp, NULL);
                        fm_mso_populate_bouquet_id(ctxp, in_flistp, device_hd_flag, 0, &return_flistp, ebufp);

			            task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 2, ebufp);
			            PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "SET_BOUQUET_ID", ebufp);

			            if ( exp_bouquet_id && strlen(exp_bouquet_id) > 0)
			            {
				            PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_BOUQUET_ID, exp_bouquet_id, ebufp);
			            }
                        else if ( bouquet_id && strlen(bouquet_id) > 0)
			            {
				            PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_BOUQUET_ID, bouquet_id, ebufp);
			            }
		
			            task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 3, ebufp);
			            PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "SET_SEGMENT", ebufp);
		
			            task_flistp2 = PIN_FLIST_ELEM_ADD(task_flistp, PIN_FLD_CUSTOMER_SEGMENTS, 1, ebufp);
                        PIN_FLIST_FLD_COPY(return_flistp, PIN_FLD_CATEGORY_ID, task_flistp2, PIN_FLD_CATEGORY_ID, ebufp);
                        PIN_FLIST_FLD_COPY(return_flistp, PIN_FLD_CATEGORY_VERSION, task_flistp2, PIN_FLD_CATEGORY_VERSION, ebufp);
                        task_flistp2 = PIN_FLIST_ELEM_ADD(task_flistp, PIN_FLD_CUSTOMER_SEGMENTS, 2, ebufp);
                        PIN_FLIST_FLD_COPY(return_flistp, PIN_FLD_REASON_ID, task_flistp2, PIN_FLD_CATEGORY_ID, ebufp);
                        PIN_FLIST_FLD_COPY(return_flistp, PIN_FLD_REASON_DOMAIN_ID, task_flistp2, PIN_FLD_CATEGORY_VERSION, ebufp);

			            task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 4, ebufp);
			            PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "SET_ZIPCODE", ebufp);
			            PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_ZIP, area_codep, ebufp);

			            PIN_FLIST_DESTROY_EX(&return_flistp, NULL);
			            PIN_FLIST_DESTROY_EX(&in_flistp, NULL);

        		        fm_mso_get_cust_active_plans_ca_ids( ctxp, i_flistpp, error_codep, CATV_REACTIVATE, ebufp);

		}
		/**************** VERIMATRIX CHANGES - BEGIN - Retrack  ************/		
		else if ( ne && strstr(ne,"VM") && stb_id)
		{
			if (strstr(ne, "JVM"))
			{
				*act = (char *) malloc(sizeof("RETRACK_JVM"));
				strcpy(*act,"RETRACK_JVM");
			}
			else
			{
				*act = (char *) malloc(sizeof("RETRACK_VM"));
				strcpy(*act,"RETRACK_VM");
			}
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		 
			in_flistp  = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, in_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET (in_flistp, PIN_FLD_NAME,stb_id, ebufp);
			//PIN_FLIST_FLD_COPY(*i_flistpp, MSO_FLD_STB_ID, in_flistp, PIN_FLD_NAME, ebufp);
		
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"s_no input flist", in_flistp);
		
			search_serial_no(ctxp, in_flistp, &ret_flistp,ebufp);
		
			//VM STB NUMBER
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"retrack iflist incomming", *i_flistpp);
					
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"retrack input flistp", in_flistp);	
			//VM_STB_ID
			
			//PIN_FLIST_FLD_COPY(*i_flistpp, MSO_FLD_VC_ID, *i_flistpp, MSO_FLD_STB_ID, ebufp);
		 
			
			//VM  serial number
			tempp_flistp = PIN_FLIST_ELEM_GET(ret_flistp,PIN_FLD_RESULTS,0,0,ebufp);
			PIN_FLIST_FLD_COPY(tempp_flistp, MSO_FLD_SERIAL_NO, *i_flistpp, MSO_FLD_VC_ID, ebufp);
		 
			PIN_FLIST_FLD_COPY(tmp1_flistp, PIN_FLD_POID, *i_flistpp, PIN_FLD_SERVICE_OBJ, ebufp);			
			
			/**** Sending CHANGE_BOUQUET_ID in RETRACK request payload. BEGIN - Date: 11-Dec-2018 ******/
			/* Fetch ZIP code */
			input_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(tmp1_flistp, PIN_FLD_ACCOUNT_OBJ, input_flistp, PIN_FLD_POID, ebufp);
			search_name_info(ctxp, input_flistp, &return_flistp,ebufp);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "nameinfo inflistp", input_flistp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "nameinfo return", return_flistp);
			
			name_res_flistp = PIN_FLIST_ELEM_GET(return_flistp,PIN_FLD_RESULTS,PIN_ELEMID_ANY,0,ebufp);
			name_flistp = PIN_FLIST_ELEM_GET(name_res_flistp,PIN_FLD_NAMEINFO,PIN_ELEMID_ANY,0,ebufp);
			
			/* Set ZIP code in payload */
			PIN_FLIST_FLD_COPY(name_flistp, PIN_FLD_ZIP, *i_flistpp, PIN_FLD_ZIP, ebufp);
			
			add_catv_bouquet_id_task(ctxp, i_flistpp, 0, task_id, ebufp);
			
			task_id = PIN_FLIST_ELEM_COUNT(*i_flistpp, MSO_FLD_TASK, ebufp);

			/**** Sending CHANGE_BOUQUET_ID in RETRACK request payload. END - Date: 11-Dec-2018 ******/

				/***************** TASK 1 **********************/
			task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, ++task_id, ebufp);
			PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "RETRACK", ebufp);
    
			PIN_FLIST_DESTROY_EX(&input_flistp, NULL);
            if (strstr(ne,"VM"))
            {
               fm_mso_get_purchased_prod_nds(ctxp, tmp1_flistp, &lbo_flistp, ebufp);
                if (PIN_FLIST_ELEM_COUNT(lbo_flistp, PIN_FLD_RESULTS, ebufp) > 0)
                {
                    in_flistp = PIN_FLIST_CREATE(ebufp);
                    PIN_FLIST_FLD_COPY(tmp1_flistp, PIN_FLD_ACCOUNT_OBJ, in_flistp, PIN_FLD_POID, ebufp);
                    PIN_FLIST_FLD_COPY(tmp1_flistp, PIN_FLD_ACCOUNT_OBJ, in_flistp, PIN_FLD_OBJECT, ebufp);
                    PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_PROFILE_DESCR, CUSTOMER_CARE, ebufp);
                    PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_SEARCH_TYPE, &profile_srch_type, ebufp);

                    fm_mso_get_profile_info(ctxp, in_flistp, &return_flistp, ebufp);
                    PIN_FLIST_DESTROY_EX(&in_flistp, NULL);

                    input_flistp = PIN_FLIST_SUBSTR_GET(return_flistp, PIN_FLD_CUSTOMER_CARE_INFO, 0, ebufp);

                    PIN_FLIST_FLD_COPY(input_flistp, MSO_FLD_LCO_OBJ, *i_flistpp, MSO_FLD_LCO_OBJ, ebufp);
                    PIN_FLIST_FLD_COPY(name_res_flistp, MSO_FLD_AREA, *i_flistpp, MSO_FLD_AREA, ebufp);

                    PIN_FLIST_DESTROY_EX(&return_flistp, NULL);
                
                    fm_mso_populate_bouquet_id(ctxp, *i_flistpp, device_hd_flag, 0, &ret_flistp, ebufp);
                    task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, ++task_id, ebufp);
                    PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "AUTHORIZE_LBO_SERVICES", ebufp);
                    PIN_FLIST_FLD_COPY(lbo_flistp, MSO_FLD_EXPIRATION_DATE, task_flistp, MSO_FLD_EXPIRYDATE, ebufp);
                    PIN_FLIST_FLD_COPY(ret_flistp, PIN_FLD_VALIDITY_IN_DAYS,
                                                task_flistp, PIN_FLD_VALIDITY_IN_DAYS, ebufp);
                    PIN_FLIST_FLD_COPY(ret_flistp, PIN_FLD_VALID_DOM,
                                                task_flistp, PIN_FLD_VALID_DOM, ebufp);
                    PIN_FLIST_FLD_COPY(ret_flistp, PIN_FLD_TIMEZONE,
                                                task_flistp, PIN_FLD_TIMEZONE, ebufp);
                    PIN_FLIST_FLD_COPY(ret_flistp, PIN_FLD_SPEED,
                                                task_flistp, PIN_FLD_SPEED, ebufp);
                    PIN_FLIST_DESTROY_EX(&ret_flistp, NULL); 
                }
            }
		}
		/**************** VERIMATRIX CHANGES - END - Retrack  ************/
		else if (ne  && strstr(ne,"GOSPEL") && stb_id )
		{
			*act = (char *) malloc(sizeof("RETRACK_GOSPEL"));
			strcpy(*act,"RETRACK_GOSPEL");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );                

	                /***************** TASK 1 **********************/
			tmp_flistp = PIN_FLIST_SUBSTR_GET(tmp1_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
			task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
			PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "VALIDATE_CARD", ebufp);

                        //add region_key
			region_key = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_REGION_KEY, 0, ebufp);
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_REGION_KEY, region_key, ebufp);
			if ( exp_bouquet_id && strlen(exp_bouquet_id) > 0)
			{
			    PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_BOUQUET_ID, exp_bouquet_id, ebufp);
			}
            else if ( bouquet_id && strlen(bouquet_id) > 0)
			{
			    PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_BOUQUET_ID, bouquet_id, ebufp);
			}
		
			//PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_BOUQUET_ID, task_flistp, MSO_FLD_BOUQUET_ID, ebufp);
	
    			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                		"prepare_catv_retrack_payload task flist", task_flistp);
			PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_VIP_CODE, vip, ebufp);	

                        // add stb id
			tmp_flistp = PIN_FLIST_ELEM_GET(tmp1_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
			PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_NAME, task_flistp, MSO_FLD_VC_ID, ebufp);

			fm_mso_get_device_details(ctxp, i_flistpp, stb_id, ebufp);

			PIN_FLIST_FLD_COPY(*i_flistpp, MSO_FLD_SERIAL_NO, task_flistp, MSO_FLD_STB_ID, ebufp);
			PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_DEVICE_TYPE, task_flistp, PIN_FLD_DEVICE_TYPE, ebufp);

			device_type = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_DEVICE_TYPE, 1, ebufp);
			if (device_type)
			{
				device_hd_flag = *device_type;
			}
			PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_DEVICE_TYPE, ebufp);
			PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_SERIAL_NO, ebufp);

			in_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(tmp1_flistp, PIN_FLD_ACCOUNT_OBJ, in_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(tmp1_flistp, PIN_FLD_ACCOUNT_OBJ, in_flistp, PIN_FLD_OBJECT, ebufp);
			PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_PROFILE_DESCR, CUSTOMER_CARE, ebufp);
			PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_SEARCH_TYPE, &profile_srch_type, ebufp);
					
			fm_mso_get_profile_info(ctxp, in_flistp, &return_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&in_flistp, NULL);

			input_flistp = PIN_FLIST_SUBSTR_GET(return_flistp, PIN_FLD_CUSTOMER_CARE_INFO, 0, ebufp);
		
			in_flistp = PIN_FLIST_CREATE(ebufp);;
			PIN_FLIST_FLD_COPY(tmp1_flistp, PIN_FLD_ACCOUNT_OBJ, in_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(tmp1_flistp, PIN_FLD_POID, *i_flistpp, PIN_FLD_SERVICE_OBJ, ebufp);
			PIN_FLIST_FLD_COPY(tmp1_flistp, PIN_FLD_ACCOUNT_OBJ, *i_flistpp, PIN_FLD_ACCOUNT_OBJ, ebufp);
			PIN_FLIST_FLD_COPY(tmp1_flistp, PIN_FLD_ACCOUNT_OBJ, *i_flistpp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(tmp1_flistp, PIN_FLD_ACCOUNT_OBJ, in_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
			PIN_FLIST_FLD_COPY(input_flistp, MSO_FLD_LCO_OBJ, in_flistp, MSO_FLD_LCO_OBJ, ebufp);
			tmp_flistp = PIN_FLIST_SUBSTR_GET(tmp1_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
			PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, in_flistp, MSO_FLD_NETWORK_NODE, ebufp);	

			PIN_FLIST_DESTROY_EX(&return_flistp, NULL);
			search_name_info(ctxp, in_flistp, &return_flistp, ebufp);

			input_flistp = PIN_FLIST_ELEM_GET(return_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
			PIN_FLIST_FLD_COPY(input_flistp, MSO_FLD_AREA, in_flistp, MSO_FLD_AREA, ebufp);
    			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                		"prepare_catv_retrack_payload service flist", tmp1_flistp);
			PIN_FLIST_ELEM_SET(in_flistp, tmp1_flistp, PIN_FLD_SERVICES, 0, ebufp);

			PIN_FLIST_DESTROY_EX(&return_flistp, NULL);
			fm_mso_populate_bouquet_id(ctxp, in_flistp, device_hd_flag, 0, &return_flistp, ebufp);

			pop_idp = PIN_FLIST_FLD_TAKE(return_flistp, MSO_FLD_POPULATION_ID, 1, ebufp);
			if (pop_idp)
			{
				PIN_FLIST_FLD_PUT(task_flistp, MSO_FLD_POPULATION_ID, pop_idp, ebufp);	
			}	

			if (pop_idp == NULL)
			{
				pop_idp = PIN_FLIST_FLD_TAKE(in_flistp, MSO_FLD_POPULATION_ID, 1, ebufp);
				if (pop_idp)
				{
					PIN_FLIST_FLD_PUT(task_flistp, MSO_FLD_POPULATION_ID, pop_idp, ebufp);
				}
				else
				{
					PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_POPULATION_ID, population_id, ebufp);
				}
			}
			
			area_codep = PIN_FLIST_FLD_TAKE(return_flistp, PIN_FLD_SERVICE_AREA_CODE, 1, ebufp);
			if (area_codep)
			{
				PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_ROUTING_AREA, area_codep, ebufp);
			}	

			PIN_FLIST_DESTROY_EX(&return_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
                
			now_t = pin_virtual_time((time_t *)NULL);
			/***************************************
			 * Add 15 days expiry of recycle command
			 **************************************/
			now_t = now_t + 1296000;
			current_time = localtime(&now_t);
			year = current_time->tm_year + 1900;
			month = current_time->tm_mon +1;
			day = current_time->tm_mday;

			memset(validity,'\0',21);
			sprintf(validity,"%04d-%02d-%02dT00:00:00",year,month,day);	
                        PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_EXPIRATION_DATE, validity, ebufp);
                	/***************** TASK 2 **********************/
        		fm_mso_get_cust_active_plans_ca_ids( ctxp, i_flistpp, error_codep, CATV_REACTIVATE, ebufp);
                	//add_package_detail(ctxp, i_flistpp, ADD_PACKAGE, ne, 0, ebufp);
			/*task_flistp = PIN_FLIST_ELEM_GET(*i_flistpp, MSO_FLD_TASK, 3, 1, ebufp);
			if(task_flistp)
			{
				PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_NAME, task_flistp, MSO_FLD_VC_ID, ebufp);
			}*/
		}
		else
		{
			*error_codep = 52107;
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
			goto CLEANUP;
		}
		if (tmp1_flistp)
		{
    			s_pdp = PIN_FLIST_FLD_TAKE(tmp1_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
        		PIN_FLIST_FLD_PUT(*i_flistpp, PIN_FLD_POID, s_pdp, ebufp);
		}
    }

    /*
     * Enrich with additional details required by Provisioning system
     */

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_catv_retrack_payload enriched input flist", *i_flistpp);

CLEANUP:
    PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&search_o_flistp, NULL);

    return;
}

static void
prepare_catv_set_user_params_payload(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    int32                   *error_codep,
    char                    **act,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *search_i_flistp = NULL;
    pin_flist_t             *search_o_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    pin_flist_t             *tmp1_flistp = NULL;
    pin_flist_t             *task_flistp = NULL;
    char                    *ne = NULL;
    char                    *cas_subs_id = NULL;
    char                    *pin_control = NULL;
    char                    *pin = NULL;
    char                    *vc_id = NULL;
    poid_t                  *s_pdp = NULL;
    poid_t                  *pdp = NULL;
    char                    template[1024] = {0};
    int32                   s_flags = 0;
    int64                   database = 0;
	pin_flist_t		*in_flistp = NULL;
	pin_flist_t		*tempp_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;



    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_set_user_params_payload input flist", *i_flistpp);

    /*
     * Validate input received
     */
    
	
	
	
     vc_id = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_VC_ID, 1, ebufp);
	
	
	
	
    pin_control = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_PIN_CONTROL, 0, ebufp);
    pin = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_PIN_NUMBER, 0, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        *error_codep = 52101;
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "prepare_catv_set_user_params_payload error: required field missing in input flist", ebufp);
        return;
    }

    /*
     * Enrich with additional service details
     */
    pdp = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_POID, 0, ebufp);
    database = PIN_POID_GET_DB(pdp);

    search_i_flistp = PIN_FLIST_CREATE(ebufp);
    s_pdp = (poid_t *)PIN_POID_CREATE(database, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(search_i_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

    s_flags = SRCH_DISTINCT ;
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

    strcpy(template,  "select X from /service where F1 = V1 ");
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, template, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
    tmp1_flistp = PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_ALIAS_LIST, 1, ebufp); //NDS - Smart card
    PIN_FLIST_FLD_SET(tmp1_flistp, PIN_FLD_NAME, vc_id, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
    tmp1_flistp = PIN_FLIST_SUBSTR_ADD(tmp_flistp, MSO_FLD_CATV_INFO, ebufp);
    PIN_FLIST_FLD_SET(tmp1_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, NULL, ebufp);
    PIN_FLIST_FLD_SET(tmp1_flistp, MSO_FLD_NETWORK_NODE, NULL, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_set_user_params_payload PCM_OP_SEARCH input flist", search_i_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_i_flistp, &search_o_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_set_user_params_payload PCM_OP_SEARCH output flist", search_o_flistp);

    tmp1_flistp = PIN_FLIST_SUBSTR_GET(search_o_flistp, PIN_FLD_RESULTS, 0, ebufp);
    tmp_flistp = PIN_FLIST_SUBSTR_GET(tmp1_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
    ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);
    PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        *error_codep = 52105;
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
        goto CLEANUP;
    }

    if(strcmp(ne, "NONE") == 0)
    {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "No need to generate the prov request ");
    }
    else if ( ne && strstr(ne,"NDS") )
    {
        *act = (char *) malloc(sizeof("SET_USER_PARAMETERS_NDS"));
        strcpy(*act,"SET_USER_PARAMETERS_NDS");
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                    //add cas subscriber id
        cas_subs_id = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, 0, ebufp);

        if ( cas_subs_id && strlen(cas_subs_id) > 0)
        {
            PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_CAS_SUBSCRIBER_ID, cas_subs_id, ebufp);
        }
        else
        {
            *error_codep = 52103;
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    PIN_ERR_BAD_VALUE, MSO_FLD_CAS_SUBSCRIBER_ID, 0, 0);
            goto CLEANUP;
        }

            /***************** TASK 1 **********************/
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "SET_USER_PARAMETERS", ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_PIN_CONTROL, pin_control, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_PIN_NUMBER, pin, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_USER_CONTROL, "0", ebufp);

        PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_PIN_CONTROL, ebufp);
        PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_PIN_NUMBER, ebufp);
        PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_VC_ID, ebufp);
    }
	
	/**************** VERIMATRIX CHANGES - BEGIN - Set User Parameters ************/	
	else if ( ne && strstr(ne,"VM") )
    {
	if (strstr(ne, "JVM"))
	{
        	*act = (char *) malloc(sizeof("SET_USER_PARAMETERS_JVM"));
        	strcpy(*act,"SET_USER_PARAMETERS_JVM");
	}
	else
	{
        	*act = (char *) malloc(sizeof("SET_USER_PARAMETERS_VM"));
        	strcpy(*act,"SET_USER_PARAMETERS_VM");
	}
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
        
		in_flistp  = PIN_FLIST_CREATE(ebufp);
	    tempp_flistp = PIN_FLIST_ELEM_GET(tmp_flistp,PIN_FLD_ALIAS_LIST,0, 1, ebufp);
	    PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, in_flistp, PIN_FLD_POID, ebufp);
	    PIN_FLIST_FLD_SET(in_flistp,PIN_FLD_NAME,vc_id,ebufp);
        
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"s_no input flist", in_flistp);
	    
		search_serial_no(ctxp, in_flistp, &ret_flistp,ebufp);
	       
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"s_no output flist", ret_flistp);	
		   
		//VM_STB_ID
		   
		PIN_FLIST_FLD_COPY(*i_flistpp, MSO_FLD_VC_ID, *i_flistpp, MSO_FLD_STB_ID, ebufp);
		   
		//VM  serial number
		tempp_flistp = PIN_FLIST_ELEM_GET(ret_flistp,PIN_FLD_RESULTS,0,0,ebufp);
		PIN_FLIST_FLD_COPY(tempp_flistp, MSO_FLD_SERIAL_NO, *i_flistpp, MSO_FLD_VC_ID, ebufp);
	 
		//VM device type
		tempp_flistp = PIN_FLIST_ELEM_GET(ret_flistp,PIN_FLD_RESULTS,0,0,ebufp);
		PIN_FLIST_FLD_COPY(tempp_flistp, PIN_FLD_DEVICE_TYPE, *i_flistpp, PIN_FLD_DEVICE_TYPE, ebufp);
            /***************** TASK 1 **********************/
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "SET_USER_PARAMETERS", ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_PIN_CONTROL, pin_control, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_PIN_NUMBER, pin, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_USER_CONTROL, "0", ebufp);

        PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_PIN_CONTROL, ebufp);
        PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_PIN_NUMBER, ebufp);
        //PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_VC_ID, ebufp);
    }
	/**************** VERIMATRIX CHANGES - END - Set User Parameters ************/		
	
    else
    {
        *error_codep = 52107;
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
        goto CLEANUP;
    }
   

    /*
     * Enrich with additional details required by Provisioning system
     */


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_catv_set_user_params_payload enriched input flist", *i_flistpp);

CLEANUP:
    PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&search_o_flistp, NULL);
 //   PIN_FLIST_DESTROY_EX(&in_flistp, NULL);

    return;

}

static void
prepare_catv_fp_nds_payload(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    int32                   *error_codep,
    char                    **act,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t     *task_flistp = NULL;
    char            *address = NULL;
    char                    *subs_id = NULL;
    int32           *duration = NULL;
    int32           channel_no = FP_NDS_ALL_CHANNEL;
    char                *ne = NULL;
    pin_flist_t		*in_flistp = NULL;
	pin_flist_t		*tempp_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;
    char 			*vm_id = NULL;
	
	char		msg[255];


    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_fp_nds_payload input flist", *i_flistpp);

    /*
     * Validate input received
     */
	 
	 
    address = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_ADDRESS, 0, ebufp);
    duration = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_DURATION_SECONDS, 0, ebufp);
	
    get_subscriber_id(ctxp, i_flistpp,ebufp);

	ne = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_NETWORK_NODE, 1, ebufp);
	sprintf(msg, "ne = %s", ne);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, ne);
	
 if ( ne && strstr(ne,"NDS"))
  {	
    *act = (char *) malloc(sizeof("FINGERPRINTING_NDS"));
    strcpy(*act,"FINGERPRINTING_NDS");
    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
        if((strncmp(address, "T0", 2) == 0 ) || (strncmp(address, "Z0", 2)== 0))
        {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Address is not VC ");
        }
        else
        {
                       subs_id = PIN_FLIST_FLD_GET(*i_flistpp,MSO_FLD_CAS_SUBSCRIBER_ID, 1, ebufp);
        }


        /***************** TASK 1 **********************/
    task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
    PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "FINGERPRINTING", ebufp);
        if(subs_id != NULL)
        {
                         PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ADDRESS, subs_id,ebufp);
        }
        else
        {
                PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ADDRESS, address, ebufp);
        }
    PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DURATION_SECONDS, duration, ebufp);
    PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_CHANNEL_NUMBER, &channel_no, ebufp);

    PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_ADDRESS, ebufp);
    PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_DURATION_SECONDS, ebufp);


    /*
     * Enrich with additional details required by Provisioning system
     */


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_catv_fp_nds_payload enriched input flist", *i_flistpp);
  }
  
	/**************** VERIMATRIX CHANGES - BEGIN - FingerPrinting ************/	  
    if ( ne && strstr(ne,"VM"))
	{
	if (strstr(ne, "JVM"))
	{
        	*act = (char *) malloc(sizeof("FINGERPRINTING_JVM"));
		strcpy(*act,"FINGERPRINTING_JVM");
	}
	else
	{
        	*act = (char *) malloc(sizeof("FINGERPRINTING_VM"));
		strcpy(*act,"FINGERPRINTING_VM");
	}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		
        if((strncmp(address, "T0", 2) == 0 ) || (strncmp(address, "Z0", 2)== 0))
        {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Address is not VM ");
        }
        else
        {
                        //subs_id = PIN_FLIST_FLD_GET(*i_flistpp,MSO_FLD_CAS_SUBSCRIBER_ID, 1, ebufp);
        }
        
		in_flistp  = PIN_FLIST_CREATE(ebufp);
	    //tempp_flistp = PIN_FLIST_ELEM_GET(so_flistp,PIN_FLD_ALIAS_LIST,0, 1, ebufp);
	    PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, in_flistp, PIN_FLD_POID, ebufp);
	    PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_ADDRESS, in_flistp, PIN_FLD_NAME, ebufp);
	    
	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"s_no input flist", in_flistp);
	    
	    search_serial_no(ctxp, in_flistp, &ret_flistp,ebufp);
	    
	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"s_no output flist", ret_flistp);

        address = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_ADDRESS, 0, ebufp);
		
		tempp_flistp = PIN_FLIST_ELEM_GET(ret_flistp,PIN_FLD_RESULTS,0,1,ebufp);
		
		if((tempp_flistp = PIN_FLIST_ELEM_GET(ret_flistp,PIN_FLD_RESULTS,0,1,ebufp)) != NULL)
		{
		   vm_id = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_ADDRESS, 1, ebufp);
		    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " indise if not null loop VM ");
		}
		
	    if(vm_id)
	    {
	    	//VM STB NUMBER
			PIN_FLIST_FLD_SET(*i_flistpp,MSO_FLD_STB_ID,vm_id,ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " indise if loop VM ");
			
			//VM  serial number
			PIN_FLIST_FLD_COPY(tempp_flistp, MSO_FLD_SERIAL_NO, *i_flistpp, MSO_FLD_VC_ID, ebufp);
			 
			//VM device type
			PIN_FLIST_FLD_COPY(tempp_flistp, PIN_FLD_DEVICE_TYPE, *i_flistpp, PIN_FLD_DEVICE_TYPE, ebufp);        
	    }

        /***************** TASK 1 **********************/
		task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
		PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "FINGERPRINTING", ebufp);
        if(subs_id != NULL)
        {
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ADDRESS, subs_id,ebufp);
        }
        else
        {
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ADDRESS, address, ebufp);
        }
		PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DURATION_SECONDS, duration, ebufp);
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_CHANNEL_NUMBER, &channel_no, ebufp);

		PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_ADDRESS, ebufp);
		PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_DURATION_SECONDS, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_catv_fp_nds_payload enriched input flist", *i_flistpp);
	}
	/**************** VERIMATRIX CHANGES - END - FingerPrinting ************/	  

 // PIN_FLIST_DESTROY_EX(&in_flistp,ebufp);
  
    return;
}


static void
prepare_catv_fp_gospel_payload(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    int32                   *error_codep,
    char                    **act,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t     *task_flistp = NULL;
    pin_flist_t     *si_flistp = NULL;
    pin_flist_t     *so_flistp = NULL;
    pin_flist_t     *tmp_flistp = NULL;
    char            *bg_color = NULL;
    char            *fg_color = NULL;
    char            *expiry_date = NULL;
    char            *font_size = NULL;
    char            *show_freq = NULL;
    char            *show_time = NULL;
    char            *start_time = NULL;
    char            *style = NULL;
    char            *style_value = NULL;
    char            *interval_time = NULL;
    char            *show_fp = NULL;
    char            *fp_xpos = NULL;
    char            *fp_ypos = NULL;
    char            *fp_id = NULL;
    int32           *duration = NULL;
    


    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_fp_gospel_payload input flist", *i_flistpp);

    /*
     * Validate input received
     */
    interval_time = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_CONNECT_TIME, 0, ebufp);
    duration = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_DURATION_SECONDS, 0, ebufp);
    fp_id = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_ID_VALUE, 0, ebufp);
    bg_color = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_ACCESS_CODE1, 0, ebufp);
    fg_color = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_ACCESS_CODE2, 0, ebufp);
    expiry_date = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_EXPIRATION_DATE, 0, ebufp);
    font_size = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_STEP_SIZE, 0, ebufp);
    fp_xpos = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_OPEN_ITEM_POSITION_ID, 0, ebufp);
    fp_ypos = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_CLEARING_ITEM_POSITION_ID, 0, ebufp);
    show_freq = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_TIME_OFFSET, 0, ebufp);
    show_time = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_TIMEMODEL, 0, ebufp);
    start_time = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_START_CREATION_DATE, 0, ebufp);
    style = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_PARAM_NAME, 0, ebufp);
    style_value = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_PARAM_VALUE, 0, ebufp);
    show_fp = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_NOTE_STR, 0, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        *error_codep = 52101;
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "prepare_catv_fp_gospel_payload error: required field missing in input flist", ebufp);
        return;
    }
	
	si_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);
    tmp_flistp = PIN_FLIST_SUBSTR_ADD(si_flistp, MSO_FLD_CATV_INFO, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, NULL, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_NETWORK_NODE, NULL, ebufp);
    tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, 1, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_fp_flist PCM_OP_READ_OBJ input flist", si_flistp);
    PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_fp_flist PCM_OP_READ_FLDS output flist", so_flistp);

    *act = (char *) malloc(sizeof("SEND_FINGERPRINT_GOSPEL"));
    strcpy(*act,"SEND_FINGERPRINT_GOSPEL");
    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
        
        /***************** TASK 1 **********************/
    task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
    PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "FINGERPRINTING", ebufp);
    
    PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_CONNECT_TIME, interval_time, ebufp);
    PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DURATION_SECONDS, duration, ebufp);
    PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_ID_VALUE, fp_id, ebufp);
    PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ACCESS_CODE1, bg_color, ebufp);
    PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ACCESS_CODE2, fg_color, ebufp);
    PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_EXPIRATION_DATE, expiry_date, ebufp);
    PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_STEP_SIZE, font_size, ebufp);
    PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_OPEN_ITEM_POSITION_ID, fp_xpos, ebufp);
    PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_CLEARING_ITEM_POSITION_ID, fp_ypos, ebufp);
    PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_TIME_OFFSET, show_freq, ebufp);
    PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_TIMEMODEL, show_time, ebufp);
    PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_START_CREATION_DATE, start_time, ebufp);
    PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PARAM_NAME, style, ebufp);
    PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PARAM_VALUE, style_value, ebufp);
    PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_NOTE_STR, show_fp, ebufp);
	
    tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
    PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, task_flistp, MSO_FLD_NETWORK_NODE, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 1, 0, ebufp);
    PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_NAME, task_flistp, MSO_FLD_VC_ID, ebufp);


    PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_CONNECT_TIME, ebufp);
    PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_DURATION_SECONDS, ebufp);
    PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_ID_VALUE, ebufp);
    PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_ACCESS_CODE1, ebufp);
    PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_ACCESS_CODE2, ebufp);
    PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_EXPIRATION_DATE, ebufp);
    PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_NOTE_STR, ebufp);
    PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_OPEN_ITEM_POSITION_ID, ebufp);
    PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_CLEARING_ITEM_POSITION_ID, ebufp);
    PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_STEP_SIZE, ebufp);
    PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_TIME_OFFSET, ebufp);
    PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_TIMEMODEL, ebufp);
    PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_START_CREATION_DATE, ebufp);
    PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_PARAM_NAME, ebufp);
    PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_PARAM_VALUE, ebufp);
    
    /*
     * Enrich with additional details required by Provisioning system
     */


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_catv_fp_gospel_payload enriched input flist", *i_flistpp);

    return;

}


static void
prepare_catv_fp_single_channel_pk_payload(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    int32                   *error_codep,
    char                    **act,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t     *task_flistp = NULL;
    char            *seg_name = NULL;
    int32           *source = NULL;


    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_fp_single_channel_pk_payload input flist", *i_flistpp);

    /*
     * Validate input received
     */
    seg_name = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_SEG_NAME, 0, ebufp);
    source = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_SOURCE, 0, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        *error_codep = 52101;
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "prepare_catv_fp_single_channel_pk_payload error: required field missing in input flist", ebufp);
        return;
    }

    *act = (char *) malloc(sizeof("FINGERPRINTING_SINGLE_CHANNEL_PK"));
    strcpy(*act,"FINGERPRINTING_SINGLE_CHANNEL_PK");
    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );

        /***************** TASK 1 **********************/
    task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
    PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "DEFINE_SEGMENT", ebufp);

    // default value
    PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_FP_ENABLE, "FPE_Visible", ebufp);

    PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_SEG_NAME, seg_name, ebufp);
    PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_SOURCE, source, ebufp);

    PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_SEG_NAME, ebufp);
    PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_SOURCE, ebufp);

    /*
     * Enrich with additional details required by Provisioning system
     */


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_catv_fp_single_channel_pk_payload enriched input flist", *i_flistpp);

    return;
}

static void
prepare_catv_fp_all_channnels_single_stb_pk_payload(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    int32                   *error_codep,
    char                    **act,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t     *task_flistp = NULL;
    pin_flist_t     *tmp_flistp = NULL;
    pin_flist_t     *sub_flistp = NULL;
    char            *stb_id = NULL;
    int32           add_package_id = 1;
    int32           del_package_id = 1;
    int32           rec_id = 0;
    pin_cookie_t    cookie = NULL;
    pin_cookie_t    prev_cookie = 0;
    int32           add_count = 0;
    int32           del_count = 0;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_fp_all_channnels_single_stb_pk_payload input flist", *i_flistpp);

    /*
     * Validate input received
     */
    stb_id = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_STB_ID, 0, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp) )
    {
        *error_codep = 52101;
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "prepare_catv_fp_all_channnels_single_stb_pk_payload error: required field missing in input flist", ebufp);
        return;
    }

    add_count = PIN_FLIST_ELEM_COUNT(*i_flistpp, MSO_FLD_ADD_PACKAGE_INFO, ebufp);
    del_count = PIN_FLIST_ELEM_COUNT(*i_flistpp, MSO_FLD_DEL_PACKAGE_INFO, ebufp);

    if ( !add_count  || !del_count )
    {
        *error_codep = 52101;
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NOT_FOUND, PIN_FLD_PACKAGE_INFO, 0, 0);

        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "prepare_catv_fp_all_channnels_single_stb_pk_payload error: required field missing in input flist", ebufp);
        return;
    }


    *act = (char *) malloc(sizeof("FINGERPRINTING_ALL_CHANNELS_SINGLE_STB_PK"));
    strcpy(*act,"FINGERPRINTING_ALL_CHANNELS_SINGLE_STB_PK");
    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );

        /***************** TASK 1 **********************/
    task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
    PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "AUTHORIZE_SERVICES", ebufp);

    while ( (tmp_flistp = PIN_FLIST_ELEM_GET_NEXT (*i_flistpp,
                    MSO_FLD_ADD_PACKAGE_INFO, &rec_id, 1,&cookie, ebufp))
                                              != (pin_flist_t *)NULL )
    {
          sub_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_ADD_PACKAGE_INFO, add_package_id++, ebufp);
          PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_CA_ID, sub_flistp, MSO_FLD_CA_ID, ebufp);
          PIN_FLIST_ELEM_DROP(*i_flistpp, MSO_FLD_ADD_PACKAGE_INFO, rec_id, ebufp);
          cookie = prev_cookie;
    }//end while


        /***************** TASK 1 **********************/
    task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 2, ebufp);
    PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "UNAUTHORIZE_SERVICES", ebufp);

    while ( (tmp_flistp = PIN_FLIST_ELEM_GET_NEXT (*i_flistpp,
                    MSO_FLD_DEL_PACKAGE_INFO, &rec_id, 1,&cookie, ebufp))
                                              != (pin_flist_t *)NULL )
    {
          sub_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_DEL_PACKAGE_INFO, del_package_id++, ebufp);
          PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_CA_ID, sub_flistp, MSO_FLD_CA_ID, ebufp);
          PIN_FLIST_ELEM_DROP(*i_flistpp, MSO_FLD_DEL_PACKAGE_INFO, rec_id, ebufp);
          cookie = prev_cookie;
    }//end while

    /*
     * Enrich with additional details required by Provisioning system
     */


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_catv_fp_all_channnels_single_stb_pk_payload enriched input flist", *i_flistpp);

    return;
}

static void
prepare_catv_fp_all_stb_pk_payload(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    int32                   *error_codep,
    char                    **act,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t     *task_flistp = NULL;
    pin_flist_t     *tmp_flistp = NULL;
    pin_flist_t     *sub_flistp = NULL;
    char            *package_name = NULL;
    int32           *source = NULL;
    int32           rec_id = 0;
    pin_cookie_t    cookie = NULL;
    pin_cookie_t    prev_cookie = 0;
    int32           add_package_id = 1;
    int32           count;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_fp_all_stb_pk_payload input flist", *i_flistpp);

    /*
     * Validate input received
     */
    package_name = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_PACKAGE_NAME, 0, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        *error_codep = 52101;
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "prepare_catv_fp_all_stb_pk_payload error: required field missing in input flist", ebufp);
        return;
    }

    count = PIN_FLIST_ELEM_COUNT(*i_flistpp, MSO_FLD_ADD_PACKAGE_INFO, ebufp);

    if ( !count )
    {
        *error_codep = 52101;
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NOT_FOUND, PIN_FLD_PACKAGE_INFO, 0, 0);

        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "prepare_catv_fp_all_stb_pk_payload error: required field missing in input flist", ebufp);
        return;
    }


    *act = (char *) malloc(sizeof("FINGERPRINTING_ALL_STBS_PK"));
    strcpy(*act,"FINGERPRINTING_ALL_STBS_PK");
    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );

        /***************** TASK 1 **********************/
    task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
    PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "DEFINE_PACKAGE", ebufp);

    PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_PACKAGE_NAME, package_name, ebufp);

    while ( (tmp_flistp = PIN_FLIST_ELEM_GET_NEXT (*i_flistpp,
                    MSO_FLD_ADD_PACKAGE_INFO, &rec_id, 1,&cookie, ebufp))
                                              != (pin_flist_t *)NULL )
    {
          sub_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_ADD_PACKAGE_INFO, add_package_id++, ebufp);
          PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_CA_ID, sub_flistp, MSO_FLD_CA_ID, ebufp);
          PIN_FLIST_ELEM_DROP(*i_flistpp, MSO_FLD_ADD_PACKAGE_INFO, rec_id, ebufp);
          cookie = prev_cookie;
    }//end while


    PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_PACKAGE_NAME, ebufp);


    /*
     * Enrich with additional details required by Provisioning system
     */


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_catv_fp_all_stb_pk_payload enriched input flist", *i_flistpp);


    return;
}

static void
prepare_catv_resend_zipcode_payload(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    int32                   *error_codep,
    char                    **act,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *search_i_flistp = NULL;
    pin_flist_t             *search_o_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    pin_flist_t             *tmp1_flistp = NULL;
    pin_flist_t             *task_flistp = NULL;
    char                    *ne = NULL;
    char                    *cas_subs_id = NULL;
    char                    *vc_id = NULL;
    poid_t                  *s_pdp = NULL;
    poid_t                  *pdp = NULL;
    char                    template[1024] = {0};
    int32                   s_flags = 0;
    /* Sachid: Adding action_flag */
    int32                   *action_flag = 0;
    int64                   database = 0;
    pin_flist_t		*in_flistp = NULL;
	pin_flist_t		*tempp_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;



    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_resend_zipcode_payload input flist", *i_flistpp);

    /*
     * Validate input received
     */
	

       vc_id = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_VC_ID, 1, ebufp);

    /* Sachid: Get action code*/
    action_flag = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_FLAGS, 0, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        *error_codep = 52101;
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "prepare_catv_resend_zipcode_payload error: required field missing in input flist", ebufp);
        return;
    }

    /*
     * Enrich with additional service details
     */
    pdp = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_POID, 0, ebufp);
    database = PIN_POID_GET_DB(pdp);

    search_i_flistp = PIN_FLIST_CREATE(ebufp);
    s_pdp = (poid_t *)PIN_POID_CREATE(database, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(search_i_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

    s_flags = SRCH_DISTINCT ;
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

    if(*action_flag == CATV_RESEND_ZIPCODE){
    strcpy(template, "select X from /service where F1 = V1 ");
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, template, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
    tmp1_flistp = PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_ALIAS_LIST, 1, ebufp); //NDS - Smart card
    PIN_FLIST_FLD_SET(tmp1_flistp, PIN_FLD_NAME, vc_id, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
    tmp1_flistp = PIN_FLIST_SUBSTR_ADD(tmp_flistp, MSO_FLD_CATV_INFO, ebufp);
    PIN_FLIST_FLD_SET(tmp1_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, NULL, ebufp);
    PIN_FLIST_FLD_SET(tmp1_flistp, MSO_FLD_NETWORK_NODE, NULL, ebufp);
    }
    else{
        strcpy(template, "select X from /mso_catv_preactivated_svc 1, /device/vc 2 where 1.F1 = 2.F2 and 2.F3 = V3 ");
        PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, template, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_VC_OBJ, NULL, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_POID, NULL, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_DEVICE_ID, vc_id, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, NULL, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_NETWORK_NODE, NULL, ebufp);
    }



    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_set_user_params_payload PCM_OP_SEARCH input flist", search_i_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_i_flistp, &search_o_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_set_user_params_payload PCM_OP_SEARCH output flist", search_o_flistp);

    tmp1_flistp = PIN_FLIST_SUBSTR_GET(search_o_flistp, PIN_FLD_RESULTS, 0, ebufp);
    if(*action_flag == CATV_RESEND_ZIPCODE){
    tmp_flistp = PIN_FLIST_SUBSTR_GET(tmp1_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
    ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);
    PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);
    
	if ( ne && strstr(ne,"NDS") )
	{
        cas_subs_id = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, 0, ebufp);
	   
	}
    }
    else
	{
        ne = PIN_FLIST_FLD_GET(tmp1_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);
        PIN_FLIST_FLD_COPY(tmp1_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);
		
	    if ( ne && strstr(ne,"NDS") )
	    {
            cas_subs_id = PIN_FLIST_FLD_GET(tmp1_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, 0, ebufp);
	    }
    }

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        *error_codep = 52105;
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
        goto CLEANUP;
    }

    if(strcmp(ne, "NONE") == 0)
    {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "No need to generate the prov request ");
    }
    else
    {

    if ( ne && strstr(ne,"NDS") )
    {
        *act = (char *) malloc(sizeof("RESEND_ZIPCODE_NDS"));
        strcpy(*act,"RESEND_ZIPCODE_NDS");
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                    //add cas subscriber id
        cas_subs_id = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, 0, ebufp);

        if ( cas_subs_id && strlen(cas_subs_id) > 0)
        {
            PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_CAS_SUBSCRIBER_ID, cas_subs_id, ebufp);
        }
        else
        {
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    PIN_ERR_BAD_VALUE, MSO_FLD_CAS_SUBSCRIBER_ID, 0, 0);
            goto CLEANUP;
        }

            /***************** TASK 1 **********************/
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "RESEND_ZIPCODE", ebufp);

        PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_VC_ID, ebufp);
    }
	/**************** VERIMATRIX CHANGES - BEGIN - Resend Zipcode ************/	  	
	else if ( ne && strstr(ne,"VM") )
    {
	if (strstr(ne, "JVM"))
	{
        	*act = (char *) malloc(sizeof("RESEND_ZIPCODE_JVM"));
        	strcpy(*act,"RESEND_ZIPCODE_JVM");
	}
	else
	{
        	*act = (char *) malloc(sizeof("RESEND_ZIPCODE_VM"));
        	strcpy(*act,"RESEND_ZIPCODE_VM");
	}
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
        
		 in_flistp  = PIN_FLIST_CREATE(ebufp);
	    tempp_flistp = PIN_FLIST_ELEM_GET(tmp_flistp,PIN_FLD_ALIAS_LIST,0, 1, ebufp);
	    PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, in_flistp, PIN_FLD_POID, ebufp);
	    PIN_FLIST_FLD_SET(in_flistp,PIN_FLD_NAME,vc_id,ebufp);
	    //PIN_FLIST_FLD_COPY(tempp_flistp, PIN_FLD_NAME, in_flistp, PIN_FLD_NAME, ebufp);
	    
	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"s_no input flist", in_flistp);
	    
	    search_serial_no(ctxp, in_flistp, &ret_flistp,ebufp);
	
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"s_no output flist", ret_flistp);
	 
		//VM_STB_ID
		PIN_FLIST_FLD_COPY(*i_flistpp, MSO_FLD_VC_ID, *i_flistpp, MSO_FLD_STB_ID, ebufp);
	  
		//VM  serial number
		tempp_flistp = PIN_FLIST_ELEM_GET(ret_flistp,PIN_FLD_RESULTS,0,0,ebufp);
		PIN_FLIST_FLD_COPY(tempp_flistp, MSO_FLD_SERIAL_NO, *i_flistpp, MSO_FLD_VC_ID, ebufp);
	 
		//VM device type
		tempp_flistp = PIN_FLIST_ELEM_GET(ret_flistp,PIN_FLD_RESULTS,0,0,ebufp);
		PIN_FLIST_FLD_COPY(tempp_flistp, PIN_FLD_DEVICE_TYPE, *i_flistpp, PIN_FLD_DEVICE_TYPE, ebufp);

            /***************** TASK 1 **********************/
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "RESEND_ZIPCODE", ebufp);        
    }
	/**************** VERIMATRIX CHANGES - END - Resend Zipcode ************/	 	
    else
    {
        *error_codep = 52107;
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
        goto CLEANUP;
    }
    }

    /*
     * Enrich with additional details required by Provisioning system
     */


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_catv_resend_zipcode_payload enriched input flist", *i_flistpp);

CLEANUP:
    PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&search_o_flistp, NULL);
  //  PIN_FLIST_DESTROY_EX(&in_flistp, NULL);

    return;
}

static void
prepare_catv_change_bouquet_id_payload(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    char                    **act,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t     *si_flistp = NULL;
    pin_flist_t     *so_flistp = NULL;
    pin_flist_t     *tmp_flistp = NULL;
    pin_flist_t     *task_flistp = NULL;
    pin_flist_t     *task_flistp2 = NULL;
    char            *ne = NULL;
    char            *cas_subs_id = NULL;
    char            *bouquet_id = NULL;
	char		*vip = "0";	
	char		*region_key = NULL;
	char		*vc_id = NULL;
	char		*stb_id = NULL;
	char		*device_type = NULL;	
	char		*pop_idp = NULL;
	char		*area_codep = NULL;
	struct		tm *current_time;
char				template [] = "select X from /device where F1 = V1 ";
pin_flist_t		    *srch_flistp = NULL;
pin_flist_t			*arg_flist = NULL;
pin_flist_t			*srch_outflistp = NULL;
pin_flist_t			*result_flistp = NULL;
pin_flist_t         *device_flistp = NULL;
pin_flist_t			*input_flistp = NULL;
pin_flist_t			*in_flistp = NULL;
pin_flist_t			*return_flistp = NULL;
pin_flist_t			*tmp1_flistp = NULL;
pin_flist_t			*name_flistp = NULL;
poid_t				*pdp= NULL;
pin_flist_t			*args1_flistp;
	time_t		now_t = 0;
	int32		year;
	int32		month;
	int32		day;
	int32		profile_srch_type = MSO_FLAG_SRCH_BY_SELF_ACCOUNT;
	int32		device_hd_flag = 0;
	char		validity[21];
int64                db =0;
int32			srch_flag = 256;
    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_change_bouquet_id_payload input flist", *i_flistpp);

    /*
     * Validate input received
     */



    /*
     * Enrich with additional service details
     */
    si_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);
    tmp_flistp = PIN_FLIST_SUBSTR_ADD(si_flistp, MSO_FLD_CATV_INFO, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, NULL, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_NETWORK_NODE, NULL, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_BOUQUET_ID, NULL, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_REGION_KEY, NULL, ebufp);
    tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, 0, ebufp);
    tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, 1, ebufp);
    tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, 2, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_change_bouquet_id_payload PCM_OP_READ_OBJ input flist", si_flistp);
    PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_change_bouquet_id_payload PCM_OP_READ_FLDS output flist", so_flistp);

    tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
    ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);
    PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

    if(strcmp(ne, "NONE") == 0)
    {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "No need to generate the prov request ");
    }
    else
    {

    if ( ne && (strstr(ne,"NDS")))
    {
       	*act = (char *) malloc(sizeof("CHANGE_BOUQUET_ID_NDS"));
       	strcpy(*act,"CHANGE_BOUQUET_ID_NDS");
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                    //add cas subscriber id
        cas_subs_id = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, 0, ebufp);

        if ( cas_subs_id && strlen(cas_subs_id) > 0)
        {
            PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_CAS_SUBSCRIBER_ID, cas_subs_id, ebufp);
        }
        else
        {
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    PIN_ERR_BAD_VALUE, MSO_FLD_CAS_SUBSCRIBER_ID, 0, 0);
            goto CLEANUP;
        }

            /***************** TASK 1 **********************/
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "CHANGE_BOUQUET_ID", ebufp);

        bouquet_id = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_BOUQUET_ID, 0, ebufp);

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, bouquet_id);

        if (  bouquet_id && strlen(bouquet_id) > 0)
        {
            PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_BOUQUET_ID, bouquet_id, ebufp);
        }
        else
        {
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    PIN_ERR_BAD_VALUE, MSO_FLD_BOUQUET_ID, 0, 0);
            goto CLEANUP;
        }

    }
	else if ( ne && strcmp(ne, "CISCO1") == 0)
	{
		*act = (char *) malloc(sizeof("CHANGE_BOUQUET_ID_PK"));
		strcpy(*act, "CHANGE_BOUQUET_ID_PK");

		//set mac address
		tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 2, 0, ebufp);
		PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_NAME, *i_flistpp, MSO_FLD_STB_ID, ebufp);

		/***************** TASK 1 **********************/
		task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "REMOVE_BOUQUET_ID", ebufp);
		PIN_FLIST_FLD_COPY(*i_flistpp, MSO_FLD_BOUQUET_ID_HEX, task_flistp, PIN_FLD_CONTENT_ITEM_ID, ebufp);

		/***************** TASK 2 **********************/
		task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 2, ebufp);
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "SET_BOUQUET_ID", ebufp);
		PIN_FLIST_FLD_COPY(*i_flistpp, MSO_FLD_BOUQUET_ID, task_flistp, MSO_FLD_BOUQUET_ID, ebufp);
	}
	else if (ne && strcmp(ne, "GOSPELL") == 0 || strcmp(ne, "GOSPELL2") == 0 )
	{
		*act = (char *) malloc(sizeof("CHANGE_BOUQUET_ID_GOSPEL"));
		strcpy(*act, "CHANGE_BOUQUET_ID_GOSPEL");

		task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "VALIDATE_CARD", ebufp);
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_VIP_CODE, vip, ebufp);

		region_key = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_REGION_KEY, 0, ebufp);
		if ( region_key && strlen(region_key) > 0)
		{
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_REGION_KEY, region_key, ebufp);
		}		
		else
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_BAD_VALUE, MSO_FLD_REGION_KEY, 0, 0);
			goto CLEANUP;
		}
		PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_BOUQUET_ID, task_flistp, MSO_FLD_BOUQUET_ID, ebufp);
			
		tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
		PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_NAME, task_flistp, MSO_FLD_VC_ID, ebufp);
		vc_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);
	
		fm_mso_get_device_details(ctxp, i_flistpp, vc_id, ebufp);	
		PIN_FLIST_FLD_COPY(*i_flistpp, MSO_FLD_SERIAL_NO, task_flistp, MSO_FLD_STB_ID, ebufp);
		PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_DEVICE_TYPE, task_flistp, PIN_FLD_DEVICE_TYPE, ebufp);				
		device_type = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_DEVICE_TYPE, 1, ebufp);
		if (device_type)
		{
			device_hd_flag = *device_type;
		}

		PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_DEVICE_TYPE, ebufp);
		PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_SERIAL_NO, ebufp);

		in_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, in_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, in_flistp, PIN_FLD_OBJECT, ebufp);
		PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_PROFILE_DESCR, CUSTOMER_CARE, ebufp);
		PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_SEARCH_TYPE, &profile_srch_type, ebufp);

		fm_mso_get_profile_info(ctxp, in_flistp, &return_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&in_flistp, NULL);

		input_flistp = PIN_FLIST_SUBSTR_GET(return_flistp, PIN_FLD_CUSTOMER_CARE_INFO, 0, ebufp);

		in_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, in_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, *i_flistpp, PIN_FLD_ACCOUNT_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(input_flistp, MSO_FLD_LCO_OBJ, in_flistp, MSO_FLD_LCO_OBJ, ebufp);
		PIN_FLIST_FLD_SET(in_flistp, MSO_FLD_NETWORK_NODE, ne, ebufp);
		
		PIN_FLIST_DESTROY_EX(&return_flistp, NULL);

		search_name_info(ctxp, in_flistp, &return_flistp, ebufp);

		input_flistp = PIN_FLIST_ELEM_GET(return_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
		PIN_FLIST_FLD_COPY(input_flistp, MSO_FLD_AREA, in_flistp, MSO_FLD_AREA, ebufp);
		PIN_FLIST_ELEM_SET(in_flistp, so_flistp, PIN_FLD_SERVICES, 0, ebufp);

		PIN_FLIST_DESTROY_EX(&return_flistp, NULL);
		fm_mso_populate_bouquet_id(ctxp, in_flistp, device_hd_flag, 0, &return_flistp, ebufp);

		pop_idp = PIN_FLIST_FLD_TAKE(return_flistp, MSO_FLD_POPULATION_ID, 1, ebufp);
		if (pop_idp)
		{
			PIN_FLIST_FLD_PUT(task_flistp, MSO_FLD_POPULATION_ID, pop_idp, ebufp);
		}	

		area_codep = PIN_FLIST_FLD_TAKE(return_flistp, PIN_FLD_SERVICE_AREA_CODE, 1, ebufp);
		if (area_codep)
		{
			PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_ROUTING_AREA, area_codep, ebufp);
		}
	
		PIN_FLIST_DESTROY_EX(&return_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&in_flistp, NULL);

		now_t = pin_virtual_time((time_t *)NULL);
		/***************************************
		 * Add 15 days expiry of recycle command
		 **************************************/
		now_t = now_t + 1296000;
		current_time = localtime(&now_t);
		year = current_time->tm_year + 1900;
		month = current_time->tm_mon +1;
		day = current_time->tm_mday;
		
		memset(validity,'\0',21);
		sprintf(validity,"%04d-%02d-%02dT00:00:00", year, month, day);
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_EXPIRATION_DATE, validity, ebufp);
	}
	else if (ne && strcmp(ne, "NAGRA") == 0)
	{
		*act = (char *) malloc(sizeof("CHANGE_BOUQUET_ID_NGR"));
		strcpy(*act, "CHANGE_BOUQUET_ID_NGR");

        	cas_subs_id = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, 0, ebufp);

        	if ( cas_subs_id && strlen(cas_subs_id) > 0)
        	{
            		PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_CAS_SUBSCRIBER_ID, cas_subs_id, ebufp);
        	}
        	else
        	{
            		pin_set_err(ebufp, PIN_ERRLOC_FM,
                		PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    		PIN_ERR_BAD_VALUE, MSO_FLD_CAS_SUBSCRIBER_ID, 0, 0);
            		goto CLEANUP;
        	}

		task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "SET_BOUQUET_ID", ebufp);
		PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_BOUQUET_ID, task_flistp, MSO_FLD_BOUQUET_ID, ebufp);

		task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 2, ebufp);
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "SET_NETWORK", ebufp);
		tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
		stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 1,ebufp);	
		fm_mso_get_device_details(ctxp, i_flistpp, stb_id, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"prepare_catv_change_bouquet_flist device details return", *i_flistpp);

		device_type = PIN_FLIST_FLD_TAKE(*i_flistpp, PIN_FLD_DEVICE_TYPE, 0, ebufp);
		PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_DEVICE_TYPE, device_type, ebufp);
		PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_STATE_ID, ebufp);

		in_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, in_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, in_flistp, PIN_FLD_OBJECT, ebufp);
		PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_PROFILE_DESCR, CUSTOMER_CARE, ebufp);
		PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_SEARCH_TYPE, &profile_srch_type, ebufp);

		fm_mso_get_profile_info(ctxp, in_flistp, &return_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&in_flistp, NULL);

		input_flistp = PIN_FLIST_SUBSTR_GET(return_flistp, PIN_FLD_CUSTOMER_CARE_INFO, 0, ebufp);

		in_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, in_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, *i_flistpp, PIN_FLD_ACCOUNT_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(input_flistp, MSO_FLD_LCO_OBJ, in_flistp, MSO_FLD_LCO_OBJ, ebufp);
		PIN_FLIST_FLD_SET(in_flistp, MSO_FLD_NETWORK_NODE, ne, ebufp);
		
		PIN_FLIST_DESTROY_EX(&return_flistp, NULL);

		search_name_info(ctxp, in_flistp, &return_flistp, ebufp);

		input_flistp = PIN_FLIST_ELEM_GET(return_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
		PIN_FLIST_FLD_COPY(input_flistp, MSO_FLD_AREA, in_flistp, MSO_FLD_AREA, ebufp);
		PIN_FLIST_ELEM_SET(in_flistp, so_flistp, PIN_FLD_SERVICES, 0, ebufp);

		PIN_FLIST_DESTROY_EX(&return_flistp, NULL);
		fm_mso_populate_bouquet_id(ctxp, in_flistp, device_hd_flag, 0, &return_flistp, ebufp);

		pop_idp = PIN_FLIST_FLD_TAKE(return_flistp, MSO_FLD_POPULATION_ID, 1, ebufp);
		if (pop_idp)
		{
			PIN_FLIST_FLD_PUT(task_flistp, MSO_FLD_POPULATION_ID, pop_idp, ebufp);
		}	

		task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 3, ebufp);
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "SET_SEGMENT", ebufp);

		task_flistp2 = PIN_FLIST_ELEM_ADD(task_flistp, PIN_FLD_CUSTOMER_SEGMENTS, 1, ebufp);
		PIN_FLIST_FLD_COPY(return_flistp, PIN_FLD_CATEGORY_ID, task_flistp2, PIN_FLD_CATEGORY_ID, ebufp);
		PIN_FLIST_FLD_COPY(return_flistp, PIN_FLD_CATEGORY_VERSION, task_flistp2, PIN_FLD_CATEGORY_VERSION, ebufp);
	
		task_flistp2 = PIN_FLIST_ELEM_ADD(task_flistp, PIN_FLD_CUSTOMER_SEGMENTS, 2, ebufp);
		PIN_FLIST_FLD_COPY(return_flistp, PIN_FLD_REASON_ID, task_flistp2, PIN_FLD_CATEGORY_ID, ebufp);
		PIN_FLIST_FLD_COPY(return_flistp, PIN_FLD_REASON_DOMAIN_ID, task_flistp2, PIN_FLD_CATEGORY_VERSION, ebufp);
		
		PIN_FLIST_DESTROY_EX(&return_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&in_flistp, NULL);

	}
	/**************** VERIMATRIX CHANGES - BEGIN - Change Bouquet ID ************/		
	else if ( ne && strstr(ne,"VM") )
    {
	if (strstr(ne, "JVM"))
	{
        	*act = (char *) malloc(sizeof("CHANGE_BOUQUET_ID_JVM"));
        	strcpy(*act,"CHANGE_BOUQUET_ID_JVM");
	}
	else
	{
        	*act = (char *) malloc(sizeof("CHANGE_BOUQUET_ID_VM"));
        	strcpy(*act,"CHANGE_BOUQUET_ID_VM");
	}
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		
		input_flistp = PIN_FLIST_CREATE(ebufp);
	    
		PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, input_flistp, PIN_FLD_POID, ebufp);
	    search_name_info(ctxp, input_flistp, &return_flistp,ebufp);
	            
	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                    "nameinfo inflistp", input_flistp);
					
	            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                    "nameinfo return", return_flistp);
		
		srch_flistp = PIN_FLIST_CREATE(ebufp);
        pdp = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_POID, 1 ,ebufp);
        db = PIN_POID_GET_DB(pdp);
        
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "hp004");
        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
        
        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp);
        args1_flistp = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_SERVICES, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, args1_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
        
        result_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_DEVICE_ID, NULL, ebufp);
		PIN_FLIST_FLD_SET(result_flistp, MSO_FLD_SERIAL_NO, NULL, ebufp);
        
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                     "device inflistp", srch_flistp);
	     			
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_outflistp, ebufp);
        
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                     "device_service", srch_outflistp);		
      
            /***************** TASK 1 **********************/
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "CHANGE_BOUQUET_ID", ebufp);

        bouquet_id = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_BOUQUET_ID, 0, ebufp);
		
         device_flistp = PIN_FLIST_ELEM_GET(srch_outflistp, PIN_FLD_RESULTS, 0, 0, ebufp);
		 
		 //VM serial number and stb id
		 PIN_FLIST_FLD_COPY(device_flistp, PIN_FLD_DEVICE_ID, task_flistp, MSO_FLD_STB_ID, ebufp);
		 PIN_FLIST_FLD_COPY(device_flistp, MSO_FLD_SERIAL_NO, task_flistp, MSO_FLD_VC_ID, ebufp);
		 
		 //Zip code 
		tmp1_flistp = PIN_FLIST_ELEM_GET(return_flistp,PIN_FLD_RESULTS,PIN_ELEMID_ANY,0,ebufp);
		name_flistp = PIN_FLIST_ELEM_GET(tmp1_flistp,PIN_FLD_NAMEINFO,PIN_ELEMID_ANY,0,ebufp);
		PIN_FLIST_FLD_COPY(name_flistp, PIN_FLD_ZIP, *i_flistpp, PIN_FLD_ZIP, ebufp);
		 
		 
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, bouquet_id);

        if (  bouquet_id && strlen(bouquet_id) > 0)
        {
            PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_BOUQUET_ID, bouquet_id, ebufp);
        }
        else
        {
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    PIN_ERR_BAD_VALUE, MSO_FLD_BOUQUET_ID, 0, 0);
            goto CLEANUP;
        }

    }
	/**************** VERIMATRIX CHANGES - BEGIN - Change Bouquet ID ************/			
    else
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
        goto CLEANUP;
    }
    }

    /*
     * Enrich with additional details required by Provisioning system
     */

	PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_BOUQUET_ID, ebufp);
	PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_BOUQUET_ID_HEX, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_catv_change_bouquet_id_payload enriched input flist", *i_flistpp);

CLEANUP:
    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&so_flistp, NULL);

    return;

}

static void
prepare_catv_set_bit_payload(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    char                    **act,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t     *si_flistp = NULL;
    pin_flist_t     *so_flistp = NULL;
    pin_flist_t     *tmp_flistp = NULL;
    pin_flist_t     *task_flistp = NULL;
    char            *ne = NULL;
    char            *cas_subs_id = NULL;
    char            *region_number = NULL;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_set_bit_payload input flist", *i_flistpp);

    /*
     * Validate input received
     */

    region_number = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_REGION_NUMBER, 0, ebufp);
    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "prepare_catv_set_bit_payload error: required field missing in input flist", ebufp);
        return;
    }


    /*
     * Enrich with additional service details
     */
    si_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);
    tmp_flistp = PIN_FLIST_SUBSTR_ADD(si_flistp, MSO_FLD_CATV_INFO, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, NULL, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_NETWORK_NODE, NULL, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_set_bit_payload PCM_OP_READ_OBJ input flist", si_flistp);
    PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_set_bit_payload PCM_OP_READ_FLDS output flist", so_flistp);

    tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
    ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);
    PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

    if(strcmp(ne, "NONE") == 0)
    {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "No need to generate the prov request ");
    }
    else
    {
    if ( ne && strstr(ne,"NDS") )
    {
        *act = (char *) malloc(sizeof("SET_BIT_NDS"));
        strcpy(*act,"SET_BIT_NDS");
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                    //add cas subscriber id
        cas_subs_id = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, 0, ebufp);

        if ( cas_subs_id && strlen(cas_subs_id) > 0)
        {
            PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_CAS_SUBSCRIBER_ID, cas_subs_id, ebufp);
        }
        else
        {
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    PIN_ERR_BAD_VALUE, MSO_FLD_CAS_SUBSCRIBER_ID, 0, 0);
            goto CLEANUP;
        }

            /***************** TASK 1 **********************/
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "SET_BIT", ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_REGION_NUMBER, region_number, ebufp);

        PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_REGION_NUMBER, ebufp);

    }
	/**************** VERIMATRIX CHANGES - BEGIN - SET_BIT_VM ************/				
	else if ( ne && strstr(ne,"VM") )
    {
	if (strstr(ne, "JVM"))
	{
        	*act = (char *) malloc(sizeof("SET_BIT_JVM"));
        	strcpy(*act,"SET_BIT_JVM");
	}
	else
	{
        	*act = (char *) malloc(sizeof("SET_BIT_VM"));
        	strcpy(*act,"SET_BIT_VM");
	}
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act ); 

            /***************** TASK 1 **********************/
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "SET_BIT", ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_REGION_NUMBER, region_number, ebufp);

        PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_REGION_NUMBER, ebufp);

    }
	/**************** VERIMATRIX CHANGES - END - SET_BIT_VM ************/		
    else
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
        goto CLEANUP;
    }
    }

    /*
     * Enrich with additional details required by Provisioning system
     */


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_catv_set_bit_payload enriched input flist", *i_flistpp);

CLEANUP:
    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&so_flistp, NULL);

    return;

}

static void
prepare_catv_send_osd_payload(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    int32                   *error_codep,
    char                    **act,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t     *task_flistp = NULL;
    char            *ne = NULL;
    char            *address = NULL;
    char            *msg = NULL;
	char            *bg_color = NULL;
    char            *fg_color = NULL;
    char            *expiry_date = NULL;
    char            *font_size = NULL;
    char            *show_freq = NULL;
    char            *show_time = NULL;
    char            *start_time = NULL;
    char            *style = NULL;
    char            *style_value = NULL;
        char                    *subs_id = NULL;
    int32            *duration = NULL;
    char            *osd_no = NULL;

    pin_flist_t     *temp_flistp = NULL;
    pin_flist_t     *sub_flistp = NULL;
    pin_flist_t     *tmp_flistp = NULL;
    pin_flist_t     *si_flistp = NULL;
    pin_flist_t     *so_flistp = NULL;
    char            *msg_grp_name = NULL;
    int32           rec_id = 0;
    pin_cookie_t    cookie = NULL;
    pin_cookie_t    prev_cookie = 0;
    int32           msg_count;
    int32           title_count;
    int32           stb_count;
	pin_flist_t		*in_flistp = NULL;
	pin_flist_t		*tempp_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;
    char 			*vm_id = NULL;


    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_send_osd_payload input flist", *i_flistpp);

    /*
     * Validate input received
     */



    /*
     * Enrich with additional service details
     */
    get_subscriber_id(ctxp, i_flistpp,ebufp);
    ne = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_NETWORK_NODE, 0, ebufp);
	
	si_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);
    tmp_flistp = PIN_FLIST_SUBSTR_ADD(si_flistp, MSO_FLD_CATV_INFO, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, NULL, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_NETWORK_NODE, NULL, ebufp);
    tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, 1, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_osd_flist PCM_OP_READ_OBJ input flist", si_flistp);
    PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_osd_flist PCM_OP_READ_FLDS output flist", so_flistp);

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        *error_codep = 52101;
        return;
    }

    if(strcmp(ne, "NONE") == 0)
    {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "No need to generate the prov request ");
    }
    else
    {

    if ( ne && strstr(ne,"NDS") )
    {
        /*
         * Enrich with additional service details
         */
        address = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_ADDRESS, 0, ebufp);
                if((strncmp(address, "T0", 2) == 0 ) || (strncmp(address, "Z0", 2)== 0))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Address is not VC ");
        }
        else
        {
                subs_id = PIN_FLIST_FLD_GET(*i_flistpp,MSO_FLD_CAS_SUBSCRIBER_ID, 1, ebufp);
        }

        msg = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_MESSAGE, 0, ebufp);
        duration = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_DURATION_SECONDS, 0, ebufp);
        osd_no = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_OSD_NUMBER, 0, ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
            *error_codep = 52101;
            return;
        }

        *act = (char *) malloc(sizeof("SEND_OSD_NDS"));
        strcpy(*act,"SEND_OSD_NDS");
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );

            /***************** TASK 1 **********************/
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "OSD", ebufp);
                if(subs_id != NULL)
        {
                 PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ADDRESS, subs_id,ebufp);
        }
                else
                {
                        PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ADDRESS, address, ebufp);
                }
        PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_MESSAGE, msg, ebufp);
        PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DURATION_SECONDS, duration, ebufp);
        PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_OSD_NUMBER, osd_no, ebufp);

        PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_ADDRESS, ebufp);
        PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_MESSAGE, ebufp);
        PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_DURATION_SECONDS, ebufp);
        PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_OSD_NUMBER, ebufp);
//        PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

    }
    else if (ne  && strstr(ne,"CISCO"))
    {
        /*
         * Validate input received
         */
//        temp_flistp = PIN_FLIST_ELEM_GET(*i_flistpp, MSO_FLD_STB_ID_LIST, PIN_ELEMID_ANY, 1, ebufp);
        stb_count = PIN_FLIST_ELEM_COUNT(*i_flistpp, MSO_FLD_STB_ID_LIST, ebufp);
        msg_grp_name = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_MSG_GRP_NAME, 1, ebufp);
        msg_count = PIN_FLIST_ELEM_COUNT(*i_flistpp, MSO_FLD_MSG_CONTENT_LIST, ebufp);
        title_count = PIN_FLIST_ELEM_COUNT(*i_flistpp, MSO_FLD_MSG_TITLE_LIST, ebufp);


        if ( (stb_count == 0 && !msg_grp_name ) ||//one of the above two input is mandatory
                !msg_count ||
                !title_count )
        {
            *error_codep = 52101;
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    PIN_ERR_BAD_VALUE, 0, 0, 0);
            return;
        }

        *act = (char *) malloc(sizeof("SEND_OSD_PK"));
        strcpy(*act,"SEND_OSD_PK");
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );

            /***************** TASK 1 **********************/
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "OSD", ebufp);

            //default values
        PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_AUDIO_FLAG, "AF_None", ebufp);
        PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_DISPLAY_FLAG, "DF_Text", ebufp);
	if (strcmp(ne, "CISCO") == 0)
	{
        	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_MMM_CFG_NAME, "HW_EMERGENCY", ebufp);
	}
	else
	{
        	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_MMM_CFG_NAME, "OSD-30-6", ebufp);
	}

                //to insert loop if required
        while ( (temp_flistp = PIN_FLIST_ELEM_GET_NEXT (*i_flistpp,
                        MSO_FLD_STB_ID_LIST, &rec_id, 1,&cookie, ebufp))
                                                  != (pin_flist_t *)NULL )
        {
              sub_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_STB_ID_LIST, rec_id + 1, ebufp);
              PIN_FLIST_FLD_COPY(temp_flistp, MSO_FLD_STB_ID, sub_flistp, MSO_FLD_STB_ID, ebufp);
              PIN_FLIST_ELEM_DROP(*i_flistpp, MSO_FLD_STB_ID_LIST, rec_id, ebufp);
              cookie = prev_cookie;
        }//end while

            //to insert loop if required
        while ( (temp_flistp = PIN_FLIST_ELEM_GET_NEXT (*i_flistpp,
                        MSO_FLD_MSG_CONTENT_LIST, &rec_id, 1,&cookie, ebufp))
                                                  != (pin_flist_t *)NULL )
        {
              sub_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_MSG_CONTENT_LIST, rec_id + 1, ebufp);
              PIN_FLIST_FLD_SET(sub_flistp, MSO_FLD_LANG_CODE, "ENG", ebufp);
              PIN_FLIST_FLD_COPY(temp_flistp, PIN_FLD_MESSAGE, sub_flistp, PIN_FLD_MESSAGE, ebufp);
              PIN_FLIST_ELEM_DROP(*i_flistpp, MSO_FLD_MSG_CONTENT_LIST, rec_id, ebufp);
              cookie = prev_cookie;
        }//end while

        if (msg_grp_name)
        {
            PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_MSG_GRP_NAME, msg_grp_name, ebufp);
            PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_MSG_GRP_NAME, ebufp);
        }

            //to insert loop if required
        while ( (temp_flistp = PIN_FLIST_ELEM_GET_NEXT (*i_flistpp,
                        MSO_FLD_MSG_TITLE_LIST, &rec_id, 1,&cookie, ebufp))
                                                  != (pin_flist_t *)NULL )
        {
              sub_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_MSG_TITLE_LIST, rec_id + 1, ebufp);
              PIN_FLIST_FLD_SET(sub_flistp, MSO_FLD_LANG_CODE, "ENG", ebufp);
              PIN_FLIST_FLD_COPY(temp_flistp, PIN_FLD_TITLE, sub_flistp, PIN_FLD_TITLE, ebufp);
              PIN_FLIST_ELEM_DROP(*i_flistpp, MSO_FLD_MSG_TITLE_LIST, rec_id, ebufp);
              cookie = prev_cookie;
        }//end while

//        PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

    }
	/**************** VERIMATRIX CHANGES - BEGIN - Send OSD Payload ************/		
    else if ( ne && strstr(ne,"VM") )
    {
        /*
         * Enrich with additional service details
         */
        address = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_ADDRESS, 0, ebufp);
                if((strncmp(address, "T0", 2) == 0 ) || (strncmp(address, "Z0", 2)== 0))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Address is not VM ");
        }
        else
        {
        }
        
		in_flistp  = PIN_FLIST_CREATE(ebufp);
	    //tempp_flistp = PIN_FLIST_ELEM_GET(so_flistp,PIN_FLD_ALIAS_LIST,0, 1, ebufp);
	    PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, in_flistp, PIN_FLD_POID, ebufp);
	    PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_ADDRESS, in_flistp, PIN_FLD_NAME, ebufp);
	    
	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"s_no input flist", in_flistp);
	    
	    search_serial_no(ctxp, in_flistp, &ret_flistp,ebufp);
		
		tempp_flistp = PIN_FLIST_ELEM_GET(ret_flistp,PIN_FLD_RESULTS,0,1,ebufp);
		
		if((tempp_flistp = PIN_FLIST_ELEM_GET(ret_flistp,PIN_FLD_RESULTS,0,1,ebufp)) != NULL)
		{
		   vm_id = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_ADDRESS, 1, ebufp);
		    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " indise if not null loop VM ");
		}
		
		if(vm_id)
	    {
	    	//VM STB NUMBER
	    
	     PIN_FLIST_FLD_SET(*i_flistpp,MSO_FLD_STB_ID,vm_id,ebufp);
        
	      PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " indise if loop VM ");
	      
         //VM  serial number
          
          PIN_FLIST_FLD_COPY(tempp_flistp, MSO_FLD_SERIAL_NO, *i_flistpp, MSO_FLD_VC_ID, ebufp);
	     
	     //VM device type
	     
         PIN_FLIST_FLD_COPY(tempp_flistp, PIN_FLD_DEVICE_TYPE, *i_flistpp, PIN_FLD_DEVICE_TYPE, ebufp);
        
	    }
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"s_no output flist", ret_flistp);
		
        msg = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_MESSAGE, 0, ebufp);
        duration = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_DURATION_SECONDS, 0, ebufp);
        osd_no = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_OSD_NUMBER, 0, ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
            *error_codep = 52101;
            return;
        }

	if (strstr(ne, "JVM"))
	{
        	*act = (char *) malloc(sizeof("SEND_OSD_JVM"));
        	strcpy(*act,"SEND_OSD_JVM");
	}
	else
	{
        	*act = (char *) malloc(sizeof("SEND_OSD_VM"));
        	strcpy(*act,"SEND_OSD_VM");
	}
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );

            /***************** TASK 1 **********************/
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "OSD", ebufp);
                if(subs_id != NULL)
        {
                 PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ADDRESS, subs_id,ebufp);
        }
                else
                {
                        PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ADDRESS, address, ebufp);
                }
        PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_MESSAGE, msg, ebufp);
        PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DURATION_SECONDS, duration, ebufp);
        PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_OSD_NUMBER, osd_no, ebufp);

        PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_ADDRESS, ebufp);
        PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_MESSAGE, ebufp);
        PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_DURATION_SECONDS, ebufp);
        PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_OSD_NUMBER, ebufp);
//        PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

    }
	/**************** VERIMATRIX CHANGES - END - Send OSD Payload ************/
    else if ( ne && strstr(ne,"GOSPEL") )
    {
        msg = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_MESSAGE, 0, ebufp);
        duration = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_DURATION_SECONDS, 0, ebufp);
		osd_no = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_OSD_NUMBER, 0, ebufp);
		bg_color = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_ACCESS_CODE1, 0, ebufp);
		fg_color = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_ACCESS_CODE2, 0, ebufp);
		expiry_date = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_EXPIRATION_DATE, 0, ebufp);
		font_size = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_STEP_SIZE, 0, ebufp);
		show_freq = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_TIME_OFFSET, 0, ebufp);
		show_time = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_TIMEMODEL, 0, ebufp);
		start_time = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_START_CREATION_DATE, 0, ebufp);
		style = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_PARAM_NAME, 0, ebufp);
		style_value = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_PARAM_VALUE, 0, ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
            *error_codep = 52101;
            return;
        }

        *act = (char *) malloc(sizeof("SEND_OSD_GOSPEL"));
        strcpy(*act,"SEND_OSD_GOSPEL");
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );

            /***************** TASK 1 **********************/
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "OSD", ebufp);

        PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_MESSAGE, msg, ebufp);
        PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DURATION_SECONDS, duration, ebufp);
        PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_OSD_NUMBER, osd_no, ebufp);
		PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ACCESS_CODE1, bg_color, ebufp);
		PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ACCESS_CODE2, fg_color, ebufp);
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_EXPIRATION_DATE, expiry_date, ebufp);
		PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_STEP_SIZE, font_size, ebufp);
		PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_TIME_OFFSET, show_freq, ebufp);
		PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_TIMEMODEL, show_time, ebufp);
		PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_START_CREATION_DATE, start_time, ebufp);
		PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PARAM_NAME, style, ebufp);
		PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PARAM_VALUE, style_value, ebufp);
				
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, ne, ebufp);
		
		tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 1, 0, ebufp);
        PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_NAME, task_flistp, MSO_FLD_VC_ID, ebufp);


        PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_MESSAGE, ebufp);
        PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_DURATION_SECONDS, ebufp);
        PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_OSD_NUMBER, ebufp);
		PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_ACCESS_CODE1, ebufp);
        PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_ACCESS_CODE2, ebufp);
        PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_EXPIRATION_DATE, ebufp);
		PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_STEP_SIZE, ebufp);
        PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_TIME_OFFSET, ebufp);
        PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_TIMEMODEL, ebufp);
		PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_START_CREATION_DATE, ebufp);
        PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_PARAM_NAME, ebufp);
        PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_PARAM_VALUE, ebufp);
//        PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

    }
    else
    {
       *error_codep = 52102;
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
    }
    }

    /*
     * Enrich with additional details required by Provisioning system
     */


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_catv_send_osd_payload enriched input flist", *i_flistpp);

				
//PIN_FLIST_DESTROY_EX(&in_flistp,ebufp);
				
return;

}

static void
prepare_catv_reset_pin_pk_payload(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    int32                   *error_codep,
    char                    **act,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t     *si_flistp = NULL;
    pin_flist_t     *so_flistp = NULL;
    pin_flist_t     *tmp_flistp = NULL;
    pin_flist_t     *task_flistp = NULL;
    pin_flist_t     *search_iflist = NULL;
    pin_flist_t     *arg_flist = NULL;
    pin_flist_t     *alias_list = NULL;
    pin_flist_t     *results_flist = NULL;
    pin_flist_t     *catv_info = NULL;
    pin_flist_t     *search_oflist = NULL;

    poid_t          *pdp = NULL;
    poid_t          *search_pdp = NULL;

    char            *stb_id = NULL;
    char            *pin = NULL;

    int             srch_flags = 768;
    int             *act_flagp = NULL;
    int64           database = -1;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_reset_pin_pk_payload input flist", *i_flistpp);

    /*
     * Validate input received
     */

    stb_id = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_STB_ID, 0, ebufp);
    pin = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_BLOCKING_PIN, 0, ebufp);
    act_flagp = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_FLAGS, 0, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        *error_codep = 52101;
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "prepare_catv_reset_pin_pk_payload error: required field missing in input flist", ebufp);
        return;
    }

    /*
     * Enrich with additional service details
     */
       search_iflist = PIN_FLIST_CREATE(ebufp);
       pdp = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_POID, 0, ebufp);
       database = PIN_POID_GET_DB(pdp);

       search_pdp = (poid_t *)PIN_POID_CREATE(database, "/search", -1, ebufp);
       PIN_FLIST_FLD_PUT(search_iflist, PIN_FLD_POID, search_pdp, ebufp );
       PIN_FLIST_FLD_SET(search_iflist, PIN_FLD_TEMPLATE, "select X from /service/catv where F1 = V1 ", ebufp );
       PIN_FLIST_FLD_SET(search_iflist, PIN_FLD_FLAGS, &srch_flags, ebufp );

       arg_flist = PIN_FLIST_ELEM_ADD(search_iflist, PIN_FLD_ARGS, 1, ebufp);
       alias_list = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_ALIAS_LIST, PIN_ELEMID_ANY, ebufp);

       PIN_FLIST_FLD_COPY(*i_flistpp, MSO_FLD_STB_ID, alias_list, PIN_FLD_NAME, ebufp );

       results_flist =   PIN_FLIST_ELEM_ADD(search_iflist, PIN_FLD_RESULTS, 0, ebufp);
       catv_info = PIN_FLIST_SUBSTR_ADD(results_flist, MSO_FLD_CATV_INFO, ebufp);
       PIN_FLIST_FLD_SET(catv_info, MSO_FLD_NETWORK_NODE, NULL, ebufp);

       PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "prepare_catv_reset_pin_pk_payload PCM_OP_SEARCH input flist", search_iflist);
       PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_iflist, &search_oflist, ebufp);
       PIN_FLIST_DESTROY_EX(&search_iflist, NULL);

       PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "prepare_catv_reset_pin_pk_payload PCM_OP_SEARCH output flist", search_oflist);

       results_flist =   PIN_FLIST_ELEM_GET(search_oflist, PIN_FLD_RESULTS, 0, 1, ebufp);
       if (results_flist)
       {
                catv_info = PIN_FLIST_SUBSTR_GET(results_flist, MSO_FLD_CATV_INFO, 0, ebufp);
                PIN_FLIST_FLD_COPY(catv_info, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);
       }
       PIN_FLIST_DESTROY_EX(&search_oflist, NULL);

        *act = (char *) malloc(sizeof("RESET_PIN_PK"));
        strcpy(*act,"RESET_PIN_PK");
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );

            /***************** TASK 1 **********************/
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "SET_PIN", ebufp);

        PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_BLOCKING_PIN, pin, ebufp);

        PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_BLOCKING_PIN, ebufp);
    /*
     * Enrich with additional details required by Provisioning system
     */


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_catv_reset_pin_pk_payload enriched input flist", *i_flistpp);

CLEANUP:


    return;

}

void
set_error_descr(
    pin_flist_t             **ret_flistpp,
    int                     error_code,
    pin_errbuf_t            *ebufp)
{
    char            code[10];


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "set_error_descr input flist", *ret_flistpp);

    sprintf(code,"%d",error_code);
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, code, ebufp);

    if (error_code == 52101)
    {
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR,
            "Mandatory Field missing in input",ebufp);
    }
    else if (error_code == 52102)
    {
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR,
            "Bad Value : Network_Node",ebufp);
    }
    else if (error_code == 52103)
    {
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR,
            "BRM Data Errror: Subscriber Id not set",ebufp);
    }
    else if (error_code == 52104)
    {
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR,
            "Invalid Device Id",ebufp);
    }
    else if (error_code == 52105)
    {
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR,
            "Invalid VC Id",ebufp);
    }
    else if (error_code == 52106)
    {
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR,
            "Invalid STB Id",ebufp);
    }
    else if (error_code == 52107)
    {
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR,
            "BRM Data Errror: Invalid Network Node",ebufp);
    }
    else if (error_code == 52108)
    {
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR,
            "Invalid Provisioning Action Flag",ebufp);
    }
    else if (error_code == 52109)
    {
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR,
            "BRM: Unknown Error",ebufp);
    }
    else if (error_code == 52110)
    {
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR,
            "Buffer Creation Failed",ebufp);
    }
    else if (error_code == 52111)
    {
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR,
            "No Memory",ebufp);
    }
    else if (error_code == 52112)
    {
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR,
            "Order Object Creation Failed",ebufp);
    }
}


static void
get_subscriber_id(
pcm_context_t           *ctxp,
pin_flist_t             **i_flistpp,
pin_errbuf_t            *ebufp)
{
    pin_flist_t             *search_i_flistp = NULL;
    pin_flist_t             *search_o_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    pin_flist_t             *tmp1_flistp = NULL;
    pin_flist_t             *task_flistp = NULL;
    char                    *ne = NULL;
    char                    *cas_subs_id = NULL;
    char                    *in_nep = NULL;
    char                    *vc_id = NULL;
    char					*den_nwp = "NDS1";
	char					*dsn_nwp = "NDS2";
    char					*hw_nwp = "NDS";
    poid_t                  *s_pdp = NULL;
    poid_t                  *pdp = NULL;
    char                    template[1024] = {0};
    int32                   s_flags = 0;
    int32                   *action_flag = NULL;
    int64                   database = 0;
    int32		den_sp = 0;


    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;
    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_set_user_params_payload input flist",*i_flistpp);

    /*
     * Validate input received
     */

    vc_id = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_ADDRESS, 1, ebufp);

	if (vc_id == NULL)
	{
		vc_id = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_DEVICE_ID, 0, ebufp);
	}

        if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                      "get_subscriber_id error: required field missing in input flist", ebufp);
        return;
    }

    //Sachid:Adding flag to identify if this preactivated svc
    action_flag = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_FLAGS, 0, ebufp);
    /*
     * Enrich with additional service details
     */
    pdp = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_POID, 0, ebufp);
    database = PIN_POID_GET_DB(pdp);

    search_i_flistp = PIN_FLIST_CREATE(ebufp);
    s_pdp = (poid_t *)PIN_POID_CREATE(database, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(search_i_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

    s_flags = SRCH_DISTINCT ;
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

    if((*action_flag == PREACTV_BMAIL) ||
                (*action_flag == PREACTV_OSD)){
        strcpy(template, "select X from /mso_catv_preactivated_svc 1, /device 2 where 1.F1 = 2.F2 and 2.F3 = V3 ");
        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_VC_OBJ, NULL, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_POID, NULL, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_DEVICE_ID, vc_id, ebufp);

        tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, NULL, ebufp);
    }
    else{

	tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
	tmp1_flistp = PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_ALIAS_LIST, 1, ebufp); //NDS - Smart card
	PIN_FLIST_FLD_SET(tmp1_flistp, PIN_FLD_NAME, vc_id, ebufp);
	if (strncmp(vc_id, "000", 3) == 0)
	{
		den_sp = fm_mso_is_den_sp(ctxp, *i_flistpp, ebufp);

		tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
		if (den_sp == 1)
		{
			PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_MANUFACTURER, den_nwp, ebufp);
		}
		else if (den_sp == 2)
		{
			PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_MANUFACTURER, dsn_nwp, ebufp);
		}
		else
		{
			PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_MANUFACTURER, hw_nwp, ebufp);
		}

		tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_DEVICE_ID, vc_id, ebufp);
			
		tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 4, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_POID, NULL, ebufp);

		tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 5, ebufp);
		tmp1_flistp = PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_SERVICES, 0, ebufp);
		PIN_FLIST_FLD_SET(tmp1_flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);

		strcpy(template, "select X from /service 1, /device 2 where 1.F1 = V1 and 2.F2 = V2 and 2.F3 = 1.F1 and 1.F4 = 2.F5 ");
		
	}
	else
	{
    		strcpy(template,  "select X from /service where F1 = V1 ");
	}
    	tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
    	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
    tmp1_flistp = PIN_FLIST_SUBSTR_ADD(tmp_flistp, MSO_FLD_CATV_INFO, ebufp);
    PIN_FLIST_FLD_SET(tmp1_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, NULL, ebufp);
    PIN_FLIST_FLD_SET(tmp1_flistp, MSO_FLD_NETWORK_NODE, NULL, ebufp);
    }
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, template, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "get_subscriber_id PCM_OP_SEARCH input flist", search_i_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_i_flistp, &search_o_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "get_subscriber_id PCM_OP_SEARCH output flist", search_o_flistp);

    tmp1_flistp = PIN_FLIST_SUBSTR_GET(search_o_flistp, PIN_FLD_RESULTS, 1, ebufp);
	if (tmp1_flistp)
	{
    		s_pdp = PIN_FLIST_FLD_TAKE(tmp1_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
        	PIN_FLIST_FLD_PUT(*i_flistpp, PIN_FLD_POID, s_pdp, ebufp);

    		s_pdp = PIN_FLIST_FLD_TAKE(tmp1_flistp, PIN_FLD_POID, 0, ebufp);
        	PIN_FLIST_FLD_PUT(*i_flistpp, PIN_FLD_SERVICE_OBJ, s_pdp, ebufp);
	}
    if((*action_flag == PREACTV_BMAIL) ||
                (*action_flag == PREACTV_OSD)){
        cas_subs_id = PIN_FLIST_FLD_GET(tmp1_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, 1, ebufp);
    }
    else{
    tmp_flistp = PIN_FLIST_SUBSTR_GET(tmp1_flistp, MSO_FLD_CATV_INFO,1, ebufp);
    ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 1, ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
    {
         pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
    //    goto CLEANUP;
    }

        cas_subs_id = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, 1, ebufp);
    }

        PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_CAS_SUBSCRIBER_ID, cas_subs_id, ebufp);
	in_nep = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_NETWORK_NODE, 1, ebufp);
	if (in_nep)
	{
        	PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_NETWORK_NODE, ne, ebufp);
	}

        PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&search_o_flistp, NULL);

        return ;
}



void
fm_mso_get_device_details(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    char                    *device_id,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *search_i_flistp = NULL;
    pin_flist_t             *search_o_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    poid_t                  *s_pdp = NULL;
    poid_t                  *pdp = NULL;
    poid_t                  *account_obj = NULL;
    char                    template[1024] = {0};
    char                    *device_type =  NULL;
    char                    *den_nwp = "NDS1";
    char		    *dsn_nwp = "NDS2";
    char		    *hw_nwp = "NDS";
    char	            *manup = NULL;
    int32                   s_flags = 0;
    int32                   *state_id = NULL;
    int32                   *prov_flag =  NULL;
    int64                   database = 0;
    int32		den_sp = 0;

    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR(ebufp);

    pdp = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_POID, 0, ebufp);
    database = PIN_POID_GET_DB(pdp);

    prov_flag = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_FLAGS, 0, ebufp);

    search_i_flistp = PIN_FLIST_CREATE(ebufp);
    s_pdp = (poid_t *)PIN_POID_CREATE(database, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(search_i_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
    s_flags = SRCH_DISTINCT ;
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_DEVICE_ID, device_id, ebufp);
	if (strncmp(device_id, "000", 3) == 0)
	{
		den_sp = fm_mso_is_den_sp(ctxp, *i_flistpp, ebufp);

		tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
		if (den_sp == 1)
		{
			PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_MANUFACTURER, den_nwp, ebufp);
		}
		else if (den_sp == 2)
		{
			PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_MANUFACTURER, dsn_nwp, ebufp);
		}
		else
		{
			PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_MANUFACTURER, hw_nwp, ebufp);
		}
		
		strcpy(template, "select X from /device where F1 = V1 and F2 = V2 ");	
	}
	else
	{
		tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_SERIAL_NO, device_id, ebufp);

		strcpy(template, "select X from /device where F1 = V1 or F2 = V2 ");
	}
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, template, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_get_device_details search input flist ", search_i_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_i_flistp,
        &search_o_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_get_device_details search output flist", search_o_flistp);
    tmp_flistp = PIN_FLIST_ELEM_GET(search_o_flistp,PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
    if(tmp_flistp)
    {
        device_type = PIN_FLIST_FLD_GET(tmp_flistp,PIN_FLD_DEVICE_TYPE,0,ebufp);
        manup = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_MANUFACTURER, 0, ebufp);
        state_id = PIN_FLIST_FLD_GET(tmp_flistp,PIN_FLD_STATE_ID,0,ebufp);
        PIN_FLIST_FLD_SET(*i_flistpp,PIN_FLD_DEVICE_TYPE,device_type,ebufp);
        PIN_FLIST_FLD_SET(*i_flistpp,PIN_FLD_STATE_ID,state_id,ebufp);
        PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_SERIAL_NO, *i_flistpp, MSO_FLD_SERIAL_NO, ebufp);
	if (strncmp(device_id, "000", 3) != 0 && prov_flag && *prov_flag == CATV_RETRACK && strstr(manup, "VM"))
	{
        	PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_DEVICE_ID, *i_flistpp, MSO_FLD_STB_ID, ebufp);
	}
        account_obj = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp );

        //if (PIN_POID_GET_ID((poid_t*) PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp )) == -1)
        if (account_obj)
        {
                if (PIN_POID_GET_ID(account_obj) == -1 )
                {
                        PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, *i_flistpp, PIN_FLD_POID, ebufp);
                        PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, *i_flistpp, PIN_FLD_ACCOUNT_OBJ, ebufp);
                }
        }
    }

    PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&search_o_flistp, NULL);

    return;
}

void
fm_mso_get_installation_customer_credentials(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *search_i_flistp = NULL;
    pin_flist_t             *search_o_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    pin_flist_t             *name_flistp = NULL;
    poid_t                  *s_pdp = NULL;
    poid_t                  *pdp = NULL;
    poid_t                  *account_obj = NULL;
    char                    template[1024] = {0};
    char                    *device_type =  NULL;
    int32                   s_flags = 0;
    int32                   *state_id = NULL;
    int64                   database = 0;


    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR(ebufp);

    pdp = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_POID, 0, ebufp);
    database = PIN_POID_GET_DB(pdp);

    search_i_flistp = PIN_FLIST_CREATE(ebufp);
    s_pdp = (poid_t *)PIN_POID_CREATE(database, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(search_i_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
    s_flags = SRCH_DISTINCT ;
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

    strcpy(template, "select X from /mso_customer_credential where F1 = V1 ");
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, template, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, pdp, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_get_device_details search input flist ", search_i_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_i_flistp,
        &search_o_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_get_device_details search output flist", search_o_flistp);
    tmp_flistp = PIN_FLIST_ELEM_GET(search_o_flistp,PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
    if(tmp_flistp)
    {
	name_flistp = PIN_FLIST_ELEM_GET(tmp_flistp, PIN_FLD_NAMEINFO, 2, 1, ebufp);
	if(name_flistp)
	{
		PIN_FLIST_ELEM_COPY(tmp_flistp, PIN_FLD_NAMEINFO, 2, *i_flistpp, PIN_FLD_NAMEINFO, 2, ebufp);
	}

    }

    PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&search_o_flistp, NULL);

    return;
}

/*******************************************************************
 * Sachid: prepare_catv_preactv_svc_mod_payload()
 *
 * Enrich input filst with service details required
 * optionally performa validation on input fields
 *******************************************************************/
static void
prepare_catv_preactv_svc_mod_payload(
    pcm_context_t       *ctxp,
    pin_flist_t         **payload_flistpp,
    int32               *error_codep,
    char                **act,
    pin_errbuf_t        *ebufp)
{


        char                   func_name[] = "prepare_catv_preactv_svc_mod_payload";
        char                   msg[1024];
        char                   *manufacturer = NULL;
        char                   *device_typep = NULL;
        int                    preactv_flag = 101;
        int                    *subaction_p = NULL;
        int                    subaction;
        //int32                  *error_codep;

        pin_flist_t            *task_flp = NULL;

        if(PIN_ERRBUF_IS_ERR(ebufp)){
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in prepare_catv_preactv_svc_mod_payload", ebufp);
                return;
        }
        PIN_ERRBUF_CLEAR(ebufp);

        sprintf(msg,"%s: Input flist:",func_name);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, *payload_flistpp);

        //PIN_FLIST_FLD_DROP(*payload_flistpp,PIN_FLD_USERID, ebufp);
        //PIN_FLIST_FLD_DROP(*payload_flistpp,PIN_FLD_PROGRAM_NAME, ebufp);

        //PIN_FLIST_FLD_SET(*payload_flistpp,PIN_FLD_FLAGS, &preactv_flag, ebufp);

        subaction_p  = PIN_FLIST_FLD_GET(*payload_flistpp, PIN_FLD_ACTION_MODE, 0, ebufp);
        PIN_FLIST_FLD_SET(*payload_flistpp,PIN_FLD_FLAGS, subaction_p, ebufp);
        subaction = *subaction_p;
        PIN_FLIST_FLD_DROP(*payload_flistpp,PIN_FLD_ACTION_MODE, ebufp);
        manufacturer = PIN_FLIST_FLD_GET(*payload_flistpp, MSO_FLD_NETWORK_NODE, 1, ebufp);

        switch(subaction){
                case PREACTV_CHANGE_PLAN:
                        sprintf(msg, "%s: Subaction PREACTV_CHANGE_PLAN : %d",func_name, PREACTV_CHANGE_PLAN);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
            /* Input payload_flistpp
            0 PIN_FLD_POID               POID [0] 0.0.0.1 /account -1 0
            0 PIN_FLD_USERID                  POID [0] 0.0.0.1 /account 1542 1
            0 PIN_FLD_PROGRAM_NAME        STR [0] .OAP|CSRADMIN.
            0 PIN_FLD_SERVICE_OBJ   POID [0] 0.0.0.1 /mso_catv_preactivated_svc 824 0
            0 MSO_FLD_NETWORK_NODE        STR [0] "<NDS|CISCO|CISCO1>"
            0 PIN_FLD_FLAGS                INT [0] <CATV_MODIFY_PREACTIVATED_SERVICE>
            0 MSO_FLD_CAS_SUBSCRIBER_ID    STR [0] "E12345"
            0 MSO_FLD_TASK               ARRAY [1] allocated 20, used 2
            1     MSO_FLD_TASK_NAME       STR [0] "AUTHORIZE_SERVICES"
            1     MSO_FLD_DEL_PACKAGE_INFO  ARRAY [1] allocated 20, used 1
            2         MSO_FLD_CA_ID           STR [0] "2986"
            2         MSO_FLD_TAPING_AUTHORIZATION    STR [0] "N"
            2         MSO_FLD_EXPIRATION_DATE    STR [0] "00000000"
            0 MSO_FLD_TASK          ARRAY [2] allocated 20, used 2
            1     MSO_FLD_TASK_NAME       STR [0] "UNAUTHORIZE_SERVICES"
            1     MSO_FLD_DEL_PACKAGE_INFO  ARRAY [1] allocated 20, used 1
            2         MSO_FLD_CA_ID           STR [0] "252"
            2         MSO_FLD_TAPING_AUTHORIZATION    STR [0] "N"
            2         MSO_FLD_EXPIRATION_DATE    STR [0] "20140401"
            */

                              if((manufacturer != NULL) && strstr(manufacturer,"NDS")){
                                        *act = (char *) malloc(sizeof("CHANGE_SERVICE_NDS"));
                                        strcpy(*act,"CHANGE_SERVICE_NDS");
                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                              }
                              else if((manufacturer != NULL) && strstr(manufacturer,"CISCO")){
                                        *act = (char *) malloc(sizeof("CHANGE_SERVICE_PK"));
                                        strcpy(*act,"CHANGE_SERVICE_PK");
                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                              }
							  // VERIMATRIX
			  	else if((manufacturer != NULL) && strstr(manufacturer,"VM")){
					if (strstr(manufacturer, "JVM"))
					{
                                        	*act = (char *) malloc(sizeof("CHANGE_SERVICE_JVM"));
                                        	strcpy(*act,"CHANGE_SERVICE_JVM");
					}
					else
					{
                                        	*act = (char *) malloc(sizeof("CHANGE_SERVICE_VM"));
                                        	strcpy(*act,"CHANGE_SERVICE_VM");
					}
                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                              }

                              break;
   case PREACTV_SUSPEND_SERVICE:
                        sprintf(msg, "%s: Subaction PREACTV_SUSPEND_SERVICE : %d",func_name, PREACTV_SUSPEND_SERVICE);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
            /* Input payload flistpp
            0 PIN_FLD_POID               POID [0] 0.0.0.1 /account -1 0
            0 PIN_FLD_USERID                  POID [0] 0.0.0.1 /account 1542 1
            0 PIN_FLD_PROGRAM_NAME        STR [0] âOAP|CSRADMINâ

            0 PIN_FLD_SERVICE_OBJ        POID [0] 0.0.0.1 /service/catv -1 0
            0 MSO_FLD_NETWORK_NODE        STR [0] "<NDS|CISCO|CISCO1>"
            0 PIN_FLD_FLAGS                INT [0] 102
            0 MSO_FLD_CAS_SUBSCRIBER_ID    STR [0] "E12345"
            */
                              if((manufacturer != NULL) && strstr(manufacturer,"NDS")){
                                                task_flp = PIN_FLIST_ELEM_ADD(*payload_flistpp,MSO_FLD_TASK,1,ebufp);
                                                PIN_FLIST_FLD_SET(task_flp,MSO_FLD_TASK_NAME,"SUSPEND_SUBSCRIBER", ebufp);
                                        *act = (char *) malloc(sizeof("SUSPEND_SUBSCRIBER_NDS"));
                                        strcpy(*act,"SUSPEND_SUBSCRIBER_NDS");
                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                              }
                              else if((manufacturer != NULL) && strstr(manufacturer,"CISCO")){
                                                task_flp = PIN_FLIST_ELEM_ADD(*payload_flistpp,MSO_FLD_TASK,1,ebufp);
                                                PIN_FLIST_FLD_SET(task_flp,MSO_FLD_TASK_NAME,"MODIFY_DHCT_STATE", ebufp);
                                                PIN_FLIST_FLD_SET(task_flp,MSO_FLD_DMS_ENABLED,"DMS_Disabled", ebufp);
                                        *act = (char *) malloc(sizeof("SUSPEND_SUBSCRIBER_PK"));
                                        strcpy(*act,"SUSPEND_SUBSCRIBER_PK");
                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                              }
			  // VERIMATRIX
			      else if((manufacturer != NULL) && strstr(manufacturer,"VM")){
                              		task_flp = PIN_FLIST_ELEM_ADD(*payload_flistpp,MSO_FLD_TASK,1,ebufp);
                                        PIN_FLIST_FLD_SET(task_flp,MSO_FLD_TASK_NAME,"SUSPEND_SUBSCRIBER", ebufp);
					if (strstr(manufacturer, "JVM"))
					{
                                        	*act = (char *) malloc(sizeof("SUSPEND_SUBSCRIBER_JVM"));
                                        	strcpy(*act,"SUSPEND_SUBSCRIBER_JVM");
					}
					else
					{
                                        	*act = (char *) malloc(sizeof("SUSPEND_SUBSCRIBER_VM"));
                                        	strcpy(*act,"SUSPEND_SUBSCRIBER_VM");
					}
                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                              }
                              break;
   case PREACTV_REACTIVATE_SUBSCRIBER:
                        sprintf(msg, "%s: Subaction PREACTV_REACTIVATE_SUBSCRIBER : %d",func_name, PREACTV_REACTIVATE_SUBSCRIBER);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
            /* Input payload flistpp
            0 PIN_FLD_POID               POID [0] 0.0.0.1 /account -1 0
            0 PIN_FLD_USERID                  POID [0] 0.0.0.1 /account 1542 1
            0 PIN_FLD_PROGRAM_NAME        STR [0] âOAP|CSRADMINâ

            0 PIN_FLD_SERVICE_OBJ        POID [0] 0.0.0.1 /service/catv -1 0
            0 MSO_FLD_NETWORK_NODE        STR [0] "<NDS|CISCO|CISCO1>"
            0 PIN_FLD_FLAGS                INT [0] 102
            0 MSO_FLD_CAS_SUBSCRIBER_ID    STR [0] "E12345"
            */
                              if((manufacturer != NULL) && strstr(manufacturer,"NDS")){
                                                task_flp = PIN_FLIST_ELEM_ADD(*payload_flistpp,MSO_FLD_TASK,1,ebufp);
                                                PIN_FLIST_FLD_SET(task_flp,MSO_FLD_TASK_NAME,"REACTIVATE_SUBSCRIBER", ebufp);
                                                task_flp = PIN_FLIST_ELEM_ADD(*payload_flistpp,MSO_FLD_TASK,2,ebufp);
                                                PIN_FLIST_FLD_SET(task_flp,MSO_FLD_TASK_NAME,"AUTHORIZE_SERVICES", ebufp);
                                        *act = (char *) malloc(sizeof("REACTIVATE_SERVICE_NDS"));
                                        strcpy(*act,"REACTIVATE_SERVICE_NDS");
                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                              }
                              else if((manufacturer != NULL) && strstr(manufacturer,"CISCO")){
                                                task_flp = PIN_FLIST_ELEM_ADD(*payload_flistpp,MSO_FLD_TASK,1,ebufp);
                                                PIN_FLIST_FLD_SET(task_flp,MSO_FLD_TASK_NAME,"MODIFY_DHCT_STATE", ebufp);
                                                PIN_FLIST_FLD_SET(task_flp,MSO_FLD_DMS_ENABLED,"DMS_Enabled", ebufp);
                                        *act = (char *) malloc(sizeof("REACTIVATE_SERVICE_PK"));
                                        strcpy(*act,"REACTIVATE_SERVICE_PK");
                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                              }
							  // VERIMATRIX
			      else if((manufacturer != NULL) && strstr(manufacturer,"VM")){
                              		task_flp = PIN_FLIST_ELEM_ADD(*payload_flistpp,MSO_FLD_TASK,1,ebufp);
                              		PIN_FLIST_FLD_SET(task_flp,MSO_FLD_TASK_NAME,"REACTIVATE_SUBSCRIBER", ebufp);
                                        task_flp = PIN_FLIST_ELEM_ADD(*payload_flistpp,MSO_FLD_TASK,2,ebufp);
                                        PIN_FLIST_FLD_SET(task_flp,MSO_FLD_TASK_NAME,"AUTHORIZE_SERVICES", ebufp);
					if (strstr(manufacturer, "JVM"))
					{
                                        	*act = (char *) malloc(sizeof("REACTIVATE_SERVICE_JVM"));
                                        	strcpy(*act,"REACTIVATE_SERVICE_JVM");
					}
					else
					{
                                        	*act = (char *) malloc(sizeof("REACTIVATE_SERVICE_VM"));
                                        	strcpy(*act,"REACTIVATE_SERVICE_VM");
					}
                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                              }
                              break;

   case PREACTV_SWAP_STB:
                        sprintf(msg, "%s: Subaction PREACTV_SWAP_STB : %d",func_name, PREACTV_SWAP_STB);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
            /* Input payload flistpp
                                0 PIN_FLD_POID               POID [0] 0.0.0.1 /account -1 0
                                0 PIN_FLD_USERID              POID [0] 0.0.0.1 /account 1542 1
                                0 PIN_FLD_FLAGS                INT [0] 104
                                0 MSO_FLD_PREACTV_SRVC_OBJ   POID [0] 0.0.0.1 /mso_catv_preactivated_svc 824 0
                                0 PIN_FLD_PROGRAM_NAME        STR [0] âOAP|CSRADMINâ

                                                                0 MSO_FLD_NETWORK_NODE        STR [0] "<NDS|CISCO|CISCO1>"
                                0 MSO_FLD_CAS_SUBSCRIBER_ID    STR [0] "002A2E9C"
                                0 MSO_FLD_TASK          ARRAY [1] allocated 20, used 5
                                1     MSO_FLD_STB_ID          STR [0] "C763713441777128"
                                1     PIN_FLD_DEVICE_TYPE     STR [0] "P1"
                                1     MSO_FLD_REGION_KEY      STR [0] "IN400030"
            */
                        task_flp = PIN_FLIST_ELEM_GET(*payload_flistpp,MSO_FLD_TASK,1,1,ebufp);
                        PIN_FLIST_FLD_SET(task_flp,MSO_FLD_TASK_NAME,"CHANGE_STB", ebufp);
                        PIN_FLIST_FLD_SET(task_flp,MSO_FLD_POPULATION_ID,"0001", ebufp);

                        if((manufacturer != NULL) && strstr(manufacturer,"NDS")){
                                *act = (char *) malloc(sizeof("CHANGE_STB_NDS"));
                                strcpy(*act,"CHANGE_STB_NDS");
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                        }
                        else if((manufacturer != NULL) && strstr(manufacturer,"CISCO")){
                                *act = (char *) malloc(sizeof("CHANGE_STB_PK"));
                                strcpy(*act,"CHANGE_STB_PK");
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                        }
						// VERIMATRIX
			else if((manufacturer != NULL) && strstr(manufacturer,"VM")){
				if (strstr(manufacturer, "JVM"))
				{
                                	*act = (char *) malloc(sizeof("CHANGE_STB_JVM"));
                                	strcpy(*act,"CHANGE_STB_JVM");
				}
				else
				{
                                	*act = (char *) malloc(sizeof("CHANGE_STB_VM"));
                                	strcpy(*act,"CHANGE_STB_VM");
				}
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                        }
                        break;
        case PREACTV_SWAP_VC:
                        sprintf(msg, "%s: Subaction PREACTV_SWAP_VC : %d",func_name, PREACTV_SWAP_VC);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
            /* Input payload flistpp
                                0 PIN_FLD_POID               POID [0] 0.0.0.1 /account -1 0
                                0 PIN_FLD_USERID              POID [0] 0.0.0.1 /account 1542 1
                                0 PIN_FLD_FLAGS                INT [0] 104
                                0 MSO_FLD_PREACTV_SRVC_OBJ   POID [0] 0.0.0.1 /mso_catv_preactivated_svc 824 0
                                0 PIN_FLD_PROGRAM_NAME        STR [0] âOAP|CSRADMINâ

                                                                0 MSO_FLD_NETWORK_NODE        STR [0] "<NDS|CISCO|CISCO1>"
                                0 MSO_FLD_CAS_SUBSCRIBER_ID    STR [0] "002A2E9C"
                                0 MSO_FLD_TASK          ARRAY [1] allocated 20, used 5
                                1     MSO_FLD_STB_ID          STR [0] "C763713441777128"
                                1     PIN_FLD_DEVICE_TYPE     STR [0] "P1"
                                1     MSO_FLD_POPULATION_ID    STR [0] "0001"
                                1     MSO_FLD_REGION_KEY      STR [0] "IN400030"
            */
                        task_flp = PIN_FLIST_ELEM_GET(*payload_flistpp,MSO_FLD_TASK,1,1,ebufp);
                        PIN_FLIST_FLD_SET(task_flp,MSO_FLD_TASK_NAME,"CHANGE_VC", ebufp);
                        PIN_FLIST_FLD_SET(task_flp,MSO_FLD_POPULATION_ID,"0001", ebufp);

                        *act = (char *) malloc(sizeof("CHANGE_VC_NDS"));
                        strcpy(*act,"CHANGE_VC_NDS");
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                        break;
        case PREACTV_LOCATION_CHANGE:
                        sprintf(msg, "%s: Subaction PREACTV_LOCATION_CHANGE : %d",func_name, PREACTV_LOCATION_CHANGE);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
            /* Input payload flistpp
                                0 PIN_FLD_POID               POID [0] 0.0.0.1 /account -1 0
                                0 PIN_FLD_USERID              POID [0] 0.0.0.1 /account 1542 1
                                0 PIN_FLD_FLAGS                INT [0] 104
                                0 MSO_FLD_PREACTV_SRVC_OBJ   POID [0] 0.0.0.1 /mso_catv_preactivated_svc 824 0
                                0 PIN_FLD_PROGRAM_NAME        STR [0] âOAP|CSRADMINâ

                                                                0 MSO_FLD_NETWORK_NODE        STR [0] "<NDS|CISCO|CISCO1>"
                                0 MSO_FLD_CAS_SUBSCRIBER_ID    STR [0] "002A2E9C"
                                0 MSO_FLD_TASK          ARRAY [1] allocated 20, used 5
                                1     MSO_FLD_STB_ID          STR [0] "C763713441777128"
                                1     PIN_FLD_DEVICE_TYPE     STR [0] "P1"
                                1     MSO_FLD_REGION_KEY      STR [0] "IN400030"
            */
                        task_flp = PIN_FLIST_ELEM_GET(*payload_flistpp,MSO_FLD_TASK,1,1,ebufp);
                        PIN_FLIST_FLD_SET(task_flp,MSO_FLD_TASK_NAME,"CHANGE_LOCATION", ebufp);
                        PIN_FLIST_FLD_SET(task_flp,MSO_FLD_POPULATION_ID,"0001", ebufp);

                        *act = (char *) malloc(sizeof("CHANGE_LOCATION_NDS"));
                        strcpy(*act,"CHANGE_LOCATION_NDS");
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                break;
        case PREACTV_CHANGE_BOUQUET_ID:
                        sprintf(msg, "%s: Subaction PREACTV_CHANGE_BOUQUET_ID : %d",func_name, PREACTV_CHANGE_BOUQUET_ID);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
            /* Input payload flistpp
                                0 PIN_FLD_POID               POID [0] 0.0.0.1 /account -1 0
                                0 PIN_FLD_USERID              POID [0] 0.0.0.1 /account 1542 1
                                0 PIN_FLD_FLAGS                INT [0] 104
                                0 MSO_FLD_PREACTV_SRVC_OBJ   POID [0] 0.0.0.1 /mso_catv_preactivated_svc 824 0
                                0 PIN_FLD_PROGRAM_NAME        STR [0] âOAP|CSRADMINâ

                                                                0 MSO_FLD_NETWORK_NODE        STR [0] "<NDS|CISCO|CISCO1>"
                                0 MSO_FLD_CAS_SUBSCRIBER_ID    STR [0] "002A2E9C"
                                0 MSO_FLD_TASK          ARRAY [1] allocated 20, used 5
                                1     MSO_FLD_BOUQUET_ID      STR [0] "IN400030"
            */
                        task_flp = PIN_FLIST_ELEM_GET(*payload_flistpp,MSO_FLD_TASK,1,1,ebufp);
                        PIN_FLIST_FLD_SET(task_flp,MSO_FLD_TASK_NAME,"CHANGE_BOUQUET_ID", ebufp);

                        *act = (char *) malloc(sizeof("CHANGE_BOUQUET_ID_NDS"));
                        strcpy(*act,"CHANGE_BOUQUET_ID_NDS");
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                break;
        case PREACTV_RESEND_ZIP_CODE:
                        sprintf(msg, "%s: Subaction PREACTV_RESEND_ZIP_CODE : %d",func_name, PREACTV_RESEND_ZIP_CODE);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
                        prepare_catv_resend_zipcode_payload(ctxp, payload_flistpp, error_codep, act, ebufp);
                break;
        case PREACTV_RESET_PIN:
                        sprintf(msg, "%s: Subaction PREACTV_RESET_PIN : %d",func_name, PREACTV_RESET_PIN);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
                        task_flp = PIN_FLIST_ELEM_ADD(*payload_flistpp,MSO_FLD_TASK,1,ebufp);
                        PIN_FLIST_FLD_SET(task_flp,MSO_FLD_TASK_NAME,"SET_PIN", ebufp);
                        PIN_FLIST_FLD_COPY(*payload_flistpp, MSO_FLD_BLOCKING_PIN, task_flp, MSO_FLD_BLOCKING_PIN, ebufp);
                        PIN_FLIST_FLD_DROP(*payload_flistpp, MSO_FLD_BLOCKING_PIN,ebufp);
                        device_typep = PIN_FLIST_FLD_GET(*payload_flistpp, MSO_FLD_STB_ID, 1, ebufp);
                        if(device_typep){// Must be reset pin request for CISCO devices
                            *act = (char *) malloc(sizeof("RESET_PIN_PK"));
                            strcpy(*act,"RESET_PIN_PK");
                        }
                        else if((device_typep = PIN_FLIST_FLD_GET(*payload_flistpp,MSO_FLD_VC_ID, 1, ebufp))
                                                != NULL){//Must be reset pin request for NDS device
                            *act = (char *) malloc(sizeof("RESET_PIN_NDS"));
                            strcpy(*act,"RESET_PIN_NDS");
                        }
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                break;
        case PREACTV_BMAIL:
                        sprintf(msg, "%s: Subaction PREACTV_BMAIL : %d",func_name, PREACTV_BMAIL);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
                        prepare_catv_email_payload(ctxp, payload_flistpp, error_codep, act, ebufp);
                break;
        case PREACTV_OSD:
                        sprintf(msg, "%s: Subaction PREACTV_OSD : %d",func_name, PREACTV_OSD);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
                        prepare_catv_send_osd_payload(ctxp, payload_flistpp, error_codep, act, ebufp);
                break;
        case PREACTV_RETRACK:
                        sprintf(msg, "%s: Subaction PREACTV_RETRACK : %d",func_name, PREACTV_RETRACK);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
                        prepare_catv_retrack_payload(ctxp, payload_flistpp, error_codep, act, ebufp);
                break;

        }
}


/*******************************************************************
 * Sachid: prepare_catv_preactv_svc_mod_payload()
 *
 * Enrich input filst with service details required
 * optionally performa validation on input fields
 *******************************************************************/
static void
fm_mso_prov_catv_config_cache_update(
        pcm_context_t *ctxp,
        pin_errbuf_t  *ebufp)
{
        poid_t                  *s_pdp = NULL;
        poid_t                                  *pdp = NULL;
        pin_flist_t             *s_flistp = NULL;
        pin_flist_t             *r_flistp = NULL;
        pin_flist_t             *res_flistp = NULL;
        pin_flist_t             *arg_flistp = NULL;
        int32                   err = PIN_ERR_NONE;
        int32                   cnt;
        int64                                   database;
        cm_cache_key_iddbstr_t  kids;
        int32                   s_flags = 256;
        int                                             no_of_buckets = 1;
        time_t                  now = 0;


        s_flistp = PIN_FLIST_CREATE(ebufp);

        /***********************************************************
         * assume db matches userid found in pin.conf
         ***********************************************************/
        pdp = PCM_GET_USERID(ctxp);
        database = PIN_POID_GET_DB(pdp);

        s_pdp = PIN_POID_CREATE(database, "/search", -1, ebufp);
        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE,
                        "select X from /mso_cfg_catv_pt where F1.db = V1 ", ebufp);
        arg_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

        PIN_FLIST_ELEM_SET( s_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "cm_cache_find_entry input flist", s_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "cm_cache_find_entry output flist", r_flistp)
        if (PIN_ERRBUF_IS_ERR(ebufp)) {
             PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_mso_prov_catv_config_cache_update: error loading /mso_cfg_catv_pt object",
                                                ebufp);
             goto cleanup;
     }

        cnt = PIN_FLIST_COUNT(r_flistp, ebufp);

        if (cnt <= 1) {
                        goto cleanup;
        }

        PIN_FLIST_FLD_DROP(r_flistp, PIN_FLD_POID, ebufp);
        now = pin_virtual_time((time_t *)NULL);
        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_MOD_T,&now,ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "cm_cache_find_entry res_flist", res_flistp)

        /*
         * Get the number of items in the flist so we can create an
         * appropriately sized cache
         */
        cnt  = 0;
        if (r_flistp) {
                cnt = PIN_FLIST_COUNT(r_flistp, ebufp);
        }

        /*
         * If there's no data, there's no point initializing it, is there.
         * Especially since this is optional!
         */
        if (cnt < 1) {
                goto cleanup;
        }



        if (!mso_prov_catv_config_ptr)
        {
                goto cleanup;
        }

        kids.id  = 0;
        kids.db  = 0;
        kids.str = "/mso_cfg_catv_pt";
        cm_cache_update_entry(mso_prov_catv_config_ptr, &kids, r_flistp, &err);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_prov_catv_config_cache_update error", ebufp);
        }
        else
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_prov_catv_config_cache_update done successfully");
        }

 cleanup:
        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&res_flistp, NULL);
                PIN_POID_DESTROY(pdp, NULL);
}

static void
prepare_catv_unset_bit_payload(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    char                    **act,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t     *si_flistp = NULL;
    pin_flist_t     *so_flistp = NULL;
    pin_flist_t     *tmp_flistp = NULL;
    pin_flist_t     *task_flistp = NULL;
    char            *ne = NULL;
    char            *cas_subs_id = NULL;
    char            *region_number = NULL;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_unset_bit_payload input flist", *i_flistpp);

    /*
     * Validate input received
     */

    region_number = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_REGION_NUMBER, 0, ebufp);
    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "prepare_catv_unset_bit_payload error: required field missing in input flist", ebufp);
        return;
    }


    /*
     * Enrich with additional service details
     */
    si_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);
    tmp_flistp = PIN_FLIST_SUBSTR_ADD(si_flistp, MSO_FLD_CATV_INFO, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, NULL, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_NETWORK_NODE, NULL, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_unset_bit_payload PCM_OP_READ_OBJ input flist", si_flistp);
    PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_catv_unset_bit_payload PCM_OP_READ_FLDS output flist", so_flistp);

    tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
    ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);
    PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

    if(strcmp(ne, "NONE") == 0)
    {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "No need to generate the prov request ");
    }
    else
    {
    if ( ne && strstr(ne,"NDS") )
    {
        *act = (char *) malloc(sizeof("UNSET_BIT_NDS"));
        strcpy(*act,"UNSET_BIT_NDS");
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                    //add cas subscriber id
        cas_subs_id = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, 0, ebufp);

        if ( cas_subs_id && strlen(cas_subs_id) > 0)
        {
            PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_CAS_SUBSCRIBER_ID, cas_subs_id, ebufp);
        }
        else
        {
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    PIN_ERR_BAD_VALUE, MSO_FLD_CAS_SUBSCRIBER_ID, 0, 0);
            goto CLEANUP;
        }

            /***************** TASK 1 **********************/
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "UNSET_BIT", ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_REGION_NUMBER, region_number, ebufp);

        PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_REGION_NUMBER, ebufp);

    }
	/**************** VERIMATRIX CHANGES - BEGIN - UNSET_BIT_VM ************/		
	else if ( ne && strstr(ne,"VM") )
    {
	if (strstr(ne, "JVM"))
	{
        	*act = (char *) malloc(sizeof("UNSET_BIT_JVM"));
        	strcpy(*act,"UNSET_BIT_JVM");
	}
	else
	{
        	*act = (char *) malloc(sizeof("UNSET_BIT_VM"));
        	strcpy(*act,"UNSET_BIT_VM");
	}
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                    //add cas subscriber id
        
            /***************** TASK 1 **********************/
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, "UNSET_BIT", ebufp);
        PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_REGION_NUMBER, region_number, ebufp);

        PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_REGION_NUMBER, ebufp);

    }
	/**************** VERIMATRIX CHANGES - END - UNSET_BIT_VM ************/
    else
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, MSO_FLD_NETWORK_NODE, 0, 0);
        goto CLEANUP;
    }
    }

    /*
     * Enrich with additional details required by Provisioning system
     */
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_catv_unset_bit_payload enriched input flist", *i_flistpp);

CLEANUP:
    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&so_flistp, NULL);

    return;
}


/*******************************************************************
 * get_mso_prov_priority()
 *
 * 1. gets Provisioning Order Priority from Config
 *        /mso_cfg_prov_order_priority cache.
 *******************************************************************/
static void
get_mso_prov_priority(
        pcm_context_t           *ctxp,
        char                            *action_type,
        int                                     *order_type,
        pin_flist_t         **ret_flistpp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t     *cache_flistp = NULL;
        pin_flist_t     *res_flistp = NULL;
        pin_flist_t     *action_info_flistp = NULL;
                pin_flist_t     *pt_data_flistp = NULL;
        pin_flist_t     *c_flistp = NULL;
        int32            err = PIN_ERR_NONE;
        cm_cache_key_iddbstr_t    kids;
        pin_cookie_t    cookie = NULL;
        int32           rec_id = 0;
        pin_cookie_t    cookie_1 = NULL;
        int32           rec_id_1 = 0;
        char            *network_node =NULL;
        char            *ca_id = NULL;
        int32           flg_ca_id_null = 1;
        int                     *tmp_ord = NULL;
        time_t          last_mod_t = 0;
        time_t          now_t = 0;

       /******************************************************
         * Null out results until we have some.
         ******************************************************/
        if ( PIN_ERRBUF_IS_ERR(ebufp) )
                return;
        PIN_ERRBUF_CLEAR(ebufp);

        /******************************************************
        * If the cache is not enabled, short circuit right away
        ******************************************************
        if ( mso_prov_priority_config_ptr == (cm_cache_t *)NULL ) {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
                "no config flist for /mso_cfg_prov_order_priority cached ");
                pin_set_err(ebufp, PIN_ERRLOC_CM,
                                PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_FLD_CACHE_NAME, 0, 0);

        }*/


        /**************************************************
         * See if the entry is in our data dictionary cache
         * Prepare the cm_cache_key_iddbstr_t structure to search
         **************************************************
        kids.id = 0;
        kids.db = 0;
        kids.str = "/mso_cfg_prov_order_priority";
        cache_flistp = cm_cache_find_entry(mso_prov_priority_config_ptr,
                                        &kids, &err);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "cm_cache_find_entry found flist", cache_flistp);
        switch(err) {
        case PIN_ERR_NONE:
                break;
        default:
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
                        "mso_prov_catv_config_object from_cache: error "
                        "accessing data dictionary cache.");
                pin_set_err(ebufp, PIN_ERRLOC_CM,
                                PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_FLD_CACHE_NAME, 0, 0);
                goto cleanup;
        }*/
        /*last_mod_t = *(time_t*)PIN_FLIST_FLD_GET(cache_flistp, PIN_FLD_MOD_T,0,ebufp);
        now_t = pin_virtual_time((time_t *)NULL);
        if(now_t  >= last_mod_t + refresh_pt_interval )
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"calling mso_prov_catv_config_object_update to refresh provisioning tag config");
                fm_mso_prov_catv_config_cache_update(ctxp, ebufp);
                kids.id = 0;
                kids.db = 0;
                kids.str = "/mso_cfg_prov_order_priority";
                cache_flistp = cm_cache_find_entry(mso_prov_priority_config_ptr,&kids, &err);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                "cm_cache_find_entry found flist", cache_flistp);
                switch(err)
                {
                        case PIN_ERR_NONE:
                                break;
                        default:
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
                                        "mso_prov_catv_config_object from_cache: error "
                                        "accessing data dictionary cache.");
                                pin_set_err(ebufp, PIN_ERRLOC_CM,
                                                PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_FLD_CACHE_NAME, 0, 0);
                                goto cleanup;
                }
        } */
/*if (!action_type || !order_type)
        {
                goto cleanup;
        }
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, action_type);
        while ( (res_flistp = PIN_FLIST_ELEM_GET_NEXT (cache_flistp,
                PIN_FLD_RESULTS, &rec_id, 1,&cookie, ebufp))!= (pin_flist_t *)NULL )
        {
                if (strcmp((char *)PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_ACTION_TYPE, 1, ebufp ),action_type) == 0 )
                {
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"res_flistp ",res_flistp);
                        c_flistp = PIN_FLIST_ELEM_GET(cache_flistp,PIN_FLD_RESULTS, rec_id, 1, ebufp);

                        rec_id_1 = 0;
                        cookie_1 = NULL;
                        action_info_flistp = PIN_FLIST_ELEM_GET(c_flistp, PIN_FLD_ACTION_INFO, PIN_ELEMID_ANY, 1, ebufp);
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"action_info_flistp ",action_info_flistp);

                        while ((pt_data_flistp = PIN_FLIST_ELEM_GET_NEXT (action_info_flistp,
                        MSO_FLD_CATV_PT_DATA, &rec_id_1, 1,&cookie_1, ebufp))!= (pin_flist_t *)NULL )
                        {
                                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"pt_data_flistp ",pt_data_flistp);
                                tmp_ord = PIN_FLIST_FLD_GET(pt_data_flistp, PIN_FLD_TYPE, 1, ebufp);
                                if ( tmp_ord && *tmp_ord == *order_type)
                                {
                                        *ret_flistpp = PIN_FLIST_COPY(c_flistp, ebufp);
                                        PIN_FLIST_FLD_COPY(pt_data_flistp, PIN_FLD_RANK, *ret_flistpp, PIN_FLD_RANK, ebufp);
//                                      rank = PIN_FLIST_FLD_GET(pt_data_flistp, PIN_FLD_RANK, 1, ebufp);
                                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"pt_data_flistp RANK",*ret_flistpp);
                                        goto cleanup;
                                }
                        }
                }
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"mso_cfg_prov_order_priority for priority of Prov Orders",c_flistp);

        cleanup:
        if ( !cache_flistp )
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CATV prov priority cache Information not available in the cache.");
                *ret_flistpp = NULL;
        }
        else
        {
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"c_flistp", c_flistp)

                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_prov_priority return flist", *ret_flistpp);
        }
        PIN_FLIST_DESTROY_EX(&cache_flistp, NULL);*/

        return;
}

static  void
fm_mso_get_cust_active_plans_ca_ids(
        pcm_context_t           *ctxp,
        pin_flist_t             **i_flistpp,
        int32                   *error_codep,
        int32                   action_flag,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *read_flistp = NULL;
        pin_flist_t             *read_o_flistp = NULL;
        pin_flist_t             *srch_iflistp = NULL;
        pin_flist_t             *srch_oflistp = NULL;
        pin_flist_t             *args_flistp = NULL;
        pin_flist_t             *result_flistp = NULL;
        pin_flist_t             *temp_flistp = NULL;
        pin_flist_t             *plan_flistp = NULL;
        pin_flist_t             *ca_id_flistp = NULL;
        pin_flist_t             *rflistp = NULL;
        pin_flist_t             *r_flistp = NULL;
        pin_flist_t             *flistp = NULL;
        pin_flist_t             *task_flistp = NULL;
        pin_flist_t             *alc_add_taskp = NULL;

        poid_t                  *plan_pdp = NULL;
        poid_t                  *serv_pdp = NULL;

        char                    msg[1024];
        int32                   srch_flags = 1792;
        char                    template [] = "select X from /purchased_product where  F1 = V1 and F2 = V2 and F3 = V3 ";
        char                    template1 [] = "select X from /purchased_product where  F1 = V1 and F2 = V2 and F3 = V3 and F4 >= V4";
        char                    func_name[] = "fm_mso_get_cust_active_plans";
        char                    *ca_id = NULL;
        char                    *task_name = NULL;
        char                    *node = NULL;

        int64                   db = 1;
        int32                   prod_status = 0;
        int32                   status_failure =-1;
        int32                   rec_id = 0;
        int32                   rec_id1 = 0;
        int32                   elem_id = 1;
        int32                   add_alc_package_id = 1;
        int32                   task_id = 1;
        int32                   alc_pkg_type = 1;
        time_t          now_t = 0;

        pin_cookie_t            cookie = NULL;
        pin_cookie_t            cookie1 = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp)){
                sprintf(msg,"%s: Error before processing");
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, msg, ebufp);
                *error_codep = 52110;
                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch purchased product poids iflist:", *i_flistpp);
	task_id = PIN_FLIST_ELEM_COUNT(*i_flistpp, MSO_FLD_TASK, ebufp);
	task_id = task_id + 1;
        if (action_flag == CATV_SUSPEND)
        {
                task_name = "UNAUTHORIZE_SERVICES";
                prod_status = 2;
        }
        else if (action_flag == CATV_TERMINATE)
        {
                task_name = "UNAUTHORIZE_SERVICES";
                prod_status = 3;
        }
        else if (action_flag == CATV_REACTIVATE)
        {
                task_name = "AUTHORIZE_SERVICES";
                prod_status = 1;
        }
        sprintf(msg,"%s: Prepare search flist to search active purchased plan", func_name);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);

        serv_pdp = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_SERVICE_OBJ, 1 ,ebufp);
        if (serv_pdp)
        {
                read_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_POID, serv_pdp, ebufp);
                PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_flistp, &read_o_flistp, ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                   PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in getting service details");
                   *error_codep = 52110;
                   goto CLEANUP;
                }
                if (read_o_flistp)
                {
                        rflistp = PIN_FLIST_SUBSTR_GET(read_o_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
                        node = PIN_FLIST_FLD_GET(rflistp, MSO_FLD_NETWORK_NODE, 1 ,ebufp);
                }
        }
        srch_iflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_iflistp, PIN_FLD_POID,PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &srch_flags, ebufp);
        if (action_flag == CATV_TERMINATE)
        {
            PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, template1, ebufp);
        }
        else
        {
            PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, template, ebufp);
        }

        args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, args_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp );

        args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, args_flistp, PIN_FLD_SERVICE_OBJ, ebufp );

        args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &prod_status, ebufp );
        if (action_flag == CATV_TERMINATE)
        {
            now_t = pin_virtual_time((time_t *)NULL);
            now_t = now_t - 1;
            args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 4, ebufp);
            PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CYCLE_END_T, &now_t, ebufp );
        }

        result_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PLAN_OBJ, NULL, ebufp);
        PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);
        PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch purchased product poids iflist:", srch_iflistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in calling searching purchase products");
                *error_codep = 52110;
                goto CLEANUP;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch purchased product poids oflist:", srch_oflistp);
        if (PIN_FLIST_ELEM_COUNT(srch_oflistp, PIN_FLD_RESULTS, ebufp) > 0)
        {
        	task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, task_id, ebufp);
        	PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, task_name, ebufp);

	if (action_flag == CATV_REACTIVATE && strstr(node, "NAGRA"))
	{ 
               	alc_add_taskp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, ++task_id, ebufp);
               	PIN_FLIST_FLD_SET(alc_add_taskp, MSO_FLD_TASK_NAME, "AUTHORIZE_ALC_SERVICES", ebufp);
	}
        while ((r_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_oflistp,
                PIN_FLD_RESULTS, &rec_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"PP IN");
                plan_pdp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
                if  (plan_pdp)
                {
                        fm_mso_get_cust_plan_ca_id(ctxp, *i_flistpp, plan_pdp, node, error_codep,
                                                &plan_flistp, ebufp);
                }
                rec_id1=0;
                cookie1=NULL;
                while ((flistp = PIN_FLIST_ELEM_GET_NEXT(plan_flistp,
                                PIN_FLD_RESULTS, &rec_id1, 1, &cookie1,
                                ebufp)) != (pin_flist_t *)NULL)
                {
			alc_pkg_type = *(int32 *)PIN_FLIST_FLD_GET(flistp, MSO_FLD_PKG_TYPE, 0, ebufp);
                        ca_id_flistp = PIN_FLIST_ELEM_GET(flistp, MSO_FLD_CATV_PT_DATA,
                                                        PIN_ELEMID_ANY, 0, ebufp);
                        if (ca_id_flistp != NULL)
                        {
                                ca_id = PIN_FLIST_FLD_GET(ca_id_flistp, MSO_FLD_CA_ID, 1 ,ebufp);
                                if (ca_id && !(strcmp(ca_id, "NA") == 0 || strstr(ca_id, "N/A")))
                                {
                                        if (action_flag == CATV_SUSPEND || action_flag == CATV_TERMINATE)
					{
                                                temp_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_DEL_PACKAGE_INFO, elem_id++, ebufp);
					}
                                        else if (action_flag == CATV_REACTIVATE)
					{
						if (strstr(node, "NAGRA") && alc_pkg_type == 2)
						{
                                                	temp_flistp = PIN_FLIST_ELEM_ADD(alc_add_taskp,
                                                          PIN_FLD_OFFERINGS, add_alc_package_id++, ebufp);
						}
						else
						{
                                                	temp_flistp = PIN_FLIST_ELEM_ADD(task_flistp,
                                                          MSO_FLD_ADD_PACKAGE_INFO, elem_id++, ebufp);
						}
					}
                                        PIN_FLIST_FLD_SET(temp_flistp, MSO_FLD_CA_ID, ca_id, ebufp );
                                }
                        }
                }
        }
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_get_cust_active_plans return flist", *i_flistpp);

        CLEANUP:
                PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
                PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
                PIN_FLIST_DESTROY_EX(&read_flistp, NULL);
                PIN_FLIST_DESTROY_EX(&read_o_flistp, NULL);
}

static  void
fm_mso_get_cust_plan_ca_id(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        poid_t                  *mso_plan_obj,
        char                    *mso_node,
        int32                   *error_codep,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *srch_flistp = NULL;
        pin_flist_t             *srch_out_flistp = NULL;
        pin_flist_t             *arg_flist = NULL;
        pin_flist_t             *result_flist = NULL;
        pin_flist_t             *tmp_flp = NULL;
        poid_t                  *pdp = NULL;

        char                    msg[1024];
        int32                   srch_flag = 768;
        char                    template1 [] = "select X from /mso_cfg_catv_pt 1 , /product 2, /deal 3, /plan 4 where 1.F1 = 2.F2 and 2.F3 = 3.F4 and 3.F5 = 4.F6 and 4.F7 = V7 and 1.F8 = V8 ";
        //char                    template2 [] = "select X from /mso_cfg_catv_pt 1 , /mso_cfg_catv_pt_channel_map 2 where 1.F1 = 2.F2 and 2.F3 = V3 and 1.F4 = V4 ";
        char                    template2 [] = "select X from /mso_cfg_catv_pt 1 , /mso_cfg_plan_products_map 2, /mso_cfg_product_channel_map 3  where 1.F1 = 3.F2 and 2.F3 = V3 and 1.F4 = V4 and 2.F5 = 3.F6 ";
        char                    func_name[] = "fm_mso_get_cust_plan_ca_id";
        char                    *descrp = NULL;
        int32                   args_cnt = 1;
        int32                   search_fail = 1;
        int32                   dpo_flags = 0;
        int64                   db = 0;

        if (PIN_ERRBUF_IS_ERR(ebufp)){
                sprintf(msg,"%s: Error before processing");
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, msg, ebufp);
                *error_codep = 52110;
                return;
        }

	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_POID, mso_plan_obj, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_DESCR, NULL, ebufp);

	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, srch_flistp, &srch_out_flistp, ebufp);

	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

	descrp = PIN_FLIST_FLD_GET(srch_out_flistp, PIN_FLD_DESCR, 0, ebufp);

	if (strstr(descrp, "DPO-"))
        {
                dpo_flags = 1;
	}	

	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);

        sprintf(msg,"%s: Prepare search flist to search CA_ID using PLAN", func_name);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
        srch_flistp = PIN_FLIST_CREATE(ebufp);
        pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1 ,ebufp);
        db = PIN_POID_GET_DB(pdp);
        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	if (dpo_flags)
	{
        	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template2, ebufp);
	}
	else
	{
        	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template1, ebufp);
	}

        // Add ARGS array
        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, args_cnt++, ebufp);
        PIN_FLIST_FLD_SET(arg_flist,PIN_FLD_PROVISIONING_TAG,"",ebufp);

        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, args_cnt++, ebufp);
        PIN_FLIST_FLD_SET(arg_flist,PIN_FLD_PROVISIONING_TAG,"",ebufp);

	if (!dpo_flags)
	{
        	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, args_cnt++, ebufp);
        	PIN_FLIST_FLD_SET(arg_flist,PIN_FLD_POID,NULL,ebufp);

        	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, args_cnt++, ebufp);
        	tmp_flp = PIN_FLIST_ELEM_ADD(arg_flist,PIN_FLD_PRODUCTS,PIN_ELEMID_ANY,ebufp);
        	PIN_FLIST_FLD_SET(tmp_flp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);

        	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, args_cnt++, ebufp);
        	PIN_FLIST_FLD_SET(arg_flist,PIN_FLD_POID,NULL,ebufp);

        	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, args_cnt++, ebufp);
        	tmp_flp = PIN_FLIST_ELEM_ADD(arg_flist,PIN_FLD_SERVICES,PIN_ELEMID_ANY,ebufp);
        	PIN_FLIST_FLD_SET(tmp_flp, PIN_FLD_DEAL_OBJ, NULL, ebufp);
	}

        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, args_cnt++, ebufp);
	if (dpo_flags)
	{
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_PLAN_OBJ, mso_plan_obj, ebufp);
	}
	else
	{
        	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, mso_plan_obj, ebufp);
	}

        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, args_cnt++, ebufp);
        tmp_flp = PIN_FLIST_ELEM_ADD(arg_flist,MSO_FLD_CATV_PT_DATA,PIN_ELEMID_ANY,ebufp);
        PIN_FLIST_FLD_SET(tmp_flp, MSO_FLD_NETWORK_NODE, mso_node, ebufp);

	if (dpo_flags)
	{
        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, args_cnt++, ebufp);
        arg_flist = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);

        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, args_cnt++, ebufp);
        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
    }
        result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);

        sprintf(msg, "%s: PCM_OP_SEARCH input flist", func_name);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, srch_flistp);

        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
                *error_codep = 52110;
                goto CLEANUP;
        }
        sprintf(msg,"%s: PCM_OP_SEARCH return flist", func_name);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg, srch_out_flistp);
        *ret_flistp = PIN_FLIST_COPY(srch_out_flistp, ebufp);

        CLEANUP:
                PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
                PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
}

static void
add_catv_bouquet_id_task(
    pcm_context_t       *ctxp,
    pin_flist_t         **i_flistpp,
    int32               flag,
    int32               task_id,
    pin_errbuf_t        *ebufp)
{
        pin_flist_t     *si_flistp = NULL;
        pin_flist_t     *so_flistp = NULL;
        pin_flist_t     *tmp_flistp = NULL;
        pin_flist_t     *task_flistp = NULL;
        pin_flist_t     *task_flistp2 = NULL;
        pin_flist_t     *srch_iflistp = NULL;
        pin_flist_t     *srch_oflistp = NULL;
        pin_flist_t     *result_flistp = NULL;
        pin_flist_t     *write_iflistp = NULL;
        pin_flist_t     *write_oflistp = NULL;
        pin_flist_t     *args_flistp = NULL;
        poid_t          *srch_pdp = NULL;
        char            *nep = NULL;
        char            *bouquet_id = NULL;
        char            *orig_bouquet_id = NULL;
        char            *exp_bouquet_id = NULL;
        char            *task_name = "CHANGE_BOUQUET_ID";
        char            *zip = NULL;
        char            *stb_id = NULL;
        char            *vc_id = NULL;
        char            *region_key = NULL;
        char            *vip = "0";
        char            *pop_idp = NULL;
        char            *area_codep = NULL;
        char            *device_type = NULL;
        char            validity[21];
        struct          tm *current_time;
        time_t          now_t = 0;
        int32           year;
        int32           month;
        int32           day;
        int32           *mso_device_typep = NULL;
        int32           device_hd_flag = 0;
        int32           srch_flag = 256;
        int32           profile_srch_type = MSO_FLAG_SRCH_BY_SELF_ACCOUNT;
        int32		    bq_change_required = 1;
        int32           action_flags = 0;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "add_catv_bouquet_id_task input flist", *i_flistpp);

        action_flags = *(int32 *) PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_FLAGS, 0, ebufp);

        si_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);
        tmp_flistp = PIN_FLIST_SUBSTR_ADD(si_flistp, MSO_FLD_CATV_INFO, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, NULL, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_NETWORK_NODE, NULL, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_BOUQUET_ID, NULL, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_BOUQUET_ID_HEX, NULL, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_REGION_KEY, NULL, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_ALIAS_LIST, 0, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "add_catv_bouquet_id_task PCM_OP_READ_OBJ input flist", si_flistp);
        PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "add_catv_bouquet_id_task PCM_OP_READ_FLDS output flist", so_flistp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error in getting service details");
                goto CLEANUP;
        }
        if (so_flistp)
        {
                tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
                orig_bouquet_id = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_BOUQUET_ID, 1 ,ebufp);
                exp_bouquet_id = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_BOUQUET_ID_HEX, 1 ,ebufp);
                nep = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 1 ,ebufp);

                if (orig_bouquet_id && (flag == 0 || action_flags == 6 || action_flags == 5))
                {
                     if (exp_bouquet_id && strlen(exp_bouquet_id) > 1)
                     {
                        bouquet_id = (char *) malloc(sizeof(exp_bouquet_id));
                        strcpy(bouquet_id, exp_bouquet_id);
                     }
                     else
                     {
                        bouquet_id = (char *) malloc(sizeof(orig_bouquet_id));
                        strcpy(bouquet_id, orig_bouquet_id);
                     }
                }
        }
        if (orig_bouquet_id)
        {
                if (action_flags != 5 && 
                        (flag == CANCEL_PACKAGE || (nep && (strcmp(nep, "NAGRA") == 0 || strstr(nep, "GOSPEL")))))
                {
                        srch_iflistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, srch_iflistp, PIN_FLD_POID, ebufp);
                        PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, srch_iflistp, PIN_FLD_OBJECT, ebufp);
                        PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_PROFILE_DESCR, CUSTOMER_CARE, ebufp);
                        PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_SEARCH_TYPE, &profile_srch_type, ebufp);

                        fm_mso_get_profile_info(ctxp, srch_iflistp, &srch_oflistp, ebufp);
                        PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);

                        args_flistp = PIN_FLIST_SUBSTR_GET(srch_oflistp, PIN_FLD_CUSTOMER_CARE_INFO, 0, ebufp);

                        srch_iflistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, srch_iflistp, PIN_FLD_POID, ebufp);
                        PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, srch_iflistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
                        PIN_FLIST_FLD_COPY(args_flistp, MSO_FLD_LCO_OBJ, srch_iflistp, MSO_FLD_LCO_OBJ, ebufp);
                        PIN_FLIST_FLD_COPY(*i_flistpp, MSO_FLD_NETWORK_NODE, srch_iflistp, MSO_FLD_NETWORK_NODE, ebufp);

                        PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);

                        search_name_info(ctxp, srch_iflistp, &srch_oflistp, ebufp);

                        result_flistp = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
			
			            /* Set ZIP code in payload */
                        PIN_FLIST_FLD_COPY(result_flistp, MSO_FLD_AREA, srch_iflistp, MSO_FLD_AREA, ebufp);
			            args_flistp = PIN_FLIST_ELEM_GET(result_flistp, PIN_FLD_NAMEINFO, PIN_ELEMID_ANY, 0, ebufp);
                        zip = PIN_FLIST_FLD_TAKE(args_flistp, PIN_FLD_ZIP, 0, ebufp);
                        PIN_FLIST_ELEM_SET(srch_iflistp, so_flistp, PIN_FLD_SERVICES, 0, ebufp);

                        PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);

                        mso_device_typep = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_DEVICE_TYPE, 1, ebufp);
                        if (mso_device_typep)
                        {
                            device_hd_flag = *mso_device_typep;
                        }

                        fm_mso_populate_bouquet_id(ctxp, srch_iflistp, device_hd_flag, 0, &srch_oflistp, ebufp);
                        PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
                        if (PIN_ERRBUF_IS_ERR(ebufp))
                        {
                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "xpiry BQ id output list- Error in calling SEARCH", ebufp);
                                goto CLEANUP;
                        }
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search expiry BQ id output list", srch_oflistp);
                        if (srch_oflistp)
                        {
                            if (flag == CANCEL_PACKAGE)
                            {
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Selecting Expiry bouquet ID");
                                if (device_hd_flag == 1)
                                {
                                    bouquet_id = PIN_FLIST_FLD_GET(srch_oflistp, MSO_FLD_BOUQUET_ID_HD_HEX, 1, ebufp);
                                }
                                else
                                {
                                    bouquet_id = PIN_FLIST_FLD_GET(srch_oflistp, MSO_FLD_BOUQUET_ID_HEX, 1, ebufp);
                                }
                            }
                        }

                        if(!bouquet_id)
                        {
                                bouquet_id = (char *) malloc(sizeof(orig_bouquet_id));
                                strcpy(bouquet_id, orig_bouquet_id);
                        }
                }
                else if (action_flags != 6 && flag == ADD_PACKAGE && exp_bouquet_id && strlen(exp_bouquet_id) > 1)
                {
                        if (strcmp(orig_bouquet_id, exp_bouquet_id) != 0)
                        {
                            bouquet_id = (char *) malloc(sizeof(orig_bouquet_id));
                            strcpy(bouquet_id, orig_bouquet_id);
                        }
                        else
                        {
                            bq_change_required = 0;
                        }

		                if (bq_change_required && nep && strcmp(nep, "VM") == 0)
		                {
                            srch_iflistp = PIN_FLIST_CREATE(ebufp);
                            PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, srch_iflistp, PIN_FLD_POID, ebufp);
                            PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, srch_iflistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
                        
                            search_name_info(ctxp, srch_iflistp, &srch_oflistp, ebufp);

                            result_flistp = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
			                args_flistp = PIN_FLIST_ELEM_GET(result_flistp, PIN_FLD_NAMEINFO, PIN_ELEMID_ANY, 0, ebufp);
                            zip = PIN_FLIST_FLD_TAKE(args_flistp, PIN_FLD_ZIP, 0, ebufp);
                        }
                }
                else if (flag == ADD_PACKAGE && action_flags != 6 && (exp_bouquet_id == NULL || 
                            (exp_bouquet_id && strlen(exp_bouquet_id) == 0)))
                {
                    bq_change_required = 0;
                }
				
                if (bouquet_id && strlen(bouquet_id) > 0 && bq_change_required)
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Changing bouquet ID");
                        if (nep && strcmp(nep, "NAGRA") != 0 && !strstr(nep, "GOSPEL"))
                        {
                            task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, ++task_id, ebufp);
                            PIN_FLIST_FLD_SET (task_flistp, MSO_FLD_TASK_NAME, task_name, ebufp);
                            PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_BOUQUET_ID, bouquet_id, ebufp);
		                    if (nep && strcmp(nep, "VM") == 0 && zip)
		                    {
                                PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ZIP, zip, ebufp);
                            }
                        }
                        else if (nep && strcmp(nep, "NAGRA") == 0)
                        {
                            task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, ++task_id, ebufp);
                            PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "SET_BOUQUET_ID", ebufp);
                            PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_BOUQUET_ID, bouquet_id, ebufp);
                            
                            task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, ++task_id, ebufp);
                            PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "SET_NETWORK", ebufp);
                            tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
                            stb_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 1,ebufp);
                            fm_mso_get_device_details(ctxp, i_flistpp, stb_id, ebufp);

                            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                "add_catv_bouquet_id_task device details return", *i_flistpp);

                            device_type = PIN_FLIST_FLD_TAKE(*i_flistpp, PIN_FLD_DEVICE_TYPE, 0, ebufp);
                            PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_DEVICE_TYPE, device_type, ebufp);
                            PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_STATE_ID, ebufp);

                            pop_idp = PIN_FLIST_FLD_TAKE(srch_oflistp, MSO_FLD_POPULATION_ID, 1, ebufp);
                            if (pop_idp)
                            {
                                PIN_FLIST_FLD_PUT(task_flistp, MSO_FLD_POPULATION_ID, pop_idp, ebufp);
                            }
                            
                            task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, ++task_id, ebufp);
                            PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "SET_SEGMENT", ebufp);

                            task_flistp2 = PIN_FLIST_ELEM_ADD(task_flistp, PIN_FLD_CUSTOMER_SEGMENTS, 1, ebufp);
                            PIN_FLIST_FLD_COPY(srch_oflistp, PIN_FLD_CATEGORY_ID, task_flistp2, PIN_FLD_CATEGORY_ID, ebufp);
                            PIN_FLIST_FLD_COPY(srch_oflistp, PIN_FLD_CATEGORY_VERSION, task_flistp2, PIN_FLD_CATEGORY_VERSION, ebufp);
                            task_flistp2 = PIN_FLIST_ELEM_ADD(task_flistp, PIN_FLD_CUSTOMER_SEGMENTS, 2, ebufp);
                            PIN_FLIST_FLD_COPY(srch_oflistp, PIN_FLD_REASON_ID, task_flistp2, PIN_FLD_CATEGORY_ID, ebufp);
                            PIN_FLIST_FLD_COPY(srch_oflistp, PIN_FLD_REASON_DOMAIN_ID, task_flistp2, PIN_FLD_CATEGORY_VERSION, ebufp);
                        }
                        else if (nep && strstr(nep, "GOSPEL"))
                        {
                            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Add Validate Card Task for Gospel bouquet ID");
                            task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, ++task_id, ebufp);
                            PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "VALIDATE_CARD", ebufp);
                            PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_VIP_CODE, vip, ebufp);

                            region_key = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_REGION_KEY, 0, ebufp);
                            if ( region_key && strlen(region_key) > 0)
                            {
                                PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_REGION_KEY, region_key, ebufp);
                            }
                            PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_BOUQUET_ID, bouquet_id, ebufp);

                            tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
                            PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_NAME, task_flistp, MSO_FLD_VC_ID, ebufp);
                            vc_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

                            fm_mso_get_device_details(ctxp, i_flistpp, vc_id, ebufp);
                            PIN_FLIST_FLD_COPY(*i_flistpp, MSO_FLD_SERIAL_NO, task_flistp, MSO_FLD_STB_ID, ebufp);
                           
                            PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_DEVICE_TYPE, ebufp);
                            PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_SERIAL_NO, ebufp); 

                            pop_idp = PIN_FLIST_FLD_TAKE(srch_oflistp, MSO_FLD_POPULATION_ID, 1, ebufp);
                            if (pop_idp)
                            {
                                PIN_FLIST_FLD_PUT(task_flistp, MSO_FLD_POPULATION_ID, pop_idp, ebufp);
                            }

                            area_codep = PIN_FLIST_FLD_TAKE(srch_oflistp, PIN_FLD_SERVICE_AREA_CODE, 1, ebufp);
                            if (area_codep)
                            {
                                PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_ROUTING_AREA, area_codep, ebufp);
                            }

                            now_t = pin_virtual_time((time_t *)NULL);
                            /***************************************
                             * Add 15 days expiry of recycle command
                             **************************************/
                            now_t = now_t + 1296000;
                            current_time = localtime(&now_t);
                            year = current_time->tm_year + 1900;
                            month = current_time->tm_mon +1;
                            day = current_time->tm_mday;

                            memset(validity,'\0',21);
                            sprintf(validity,"%04d-%02d-%02dT00:00:00", year, month, day);
                            PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_EXPIRATION_DATE, validity, ebufp);
                        }
                        if (flag != 0 && action_flags != 6 && action_flags != 5)
                        {
                            write_iflistp = PIN_FLIST_CREATE(ebufp);
                            PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, write_iflistp, PIN_FLD_POID, ebufp);
                            tmp_flistp = PIN_FLIST_SUBSTR_ADD(write_iflistp, MSO_FLD_CATV_INFO, ebufp);
                            if (flag == CANCEL_PACKAGE)
                            {   
                                PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_BOUQUET_ID_HEX, bouquet_id, ebufp);
                            }
                            else
                            {
                                PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_BOUQUET_ID_HEX, "", ebufp);
                            }      
                            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                "add_catv_bouquet_id_task wflds input flist", write_iflistp);
                            PCM_OP(ctxp, PCM_OP_WRITE_FLDS, flag, write_iflistp, &write_oflistp, ebufp);
                        }

                        PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
                        PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
                }
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "add_catv_bouquet_id_task enriched input flist", *i_flistpp);

CLEANUP:
    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
    if (srch_oflistp) PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
    if (zip) pin_free(zip);
    return;
}

/******************** VERIMATRIX CHANGES - BEGIN - New functions *************/
static void
search_serial_no(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
		pin_flist_t             **ret_flistp,
		pin_errbuf_t            *ebufp)
{


	char				template [] = "select X from /device where F1 = V1 ";
	pin_flist_t		    *srch_flistp = NULL;
	pin_flist_t			*arg_flist = NULL;
	pin_flist_t			*srch_outflistp = NULL;
	pin_flist_t			*result_flistp = NULL;
	poid_t				*pdp= NULL;
	int64                db =0;
	int32			srch_flag = 256;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "i_flistp", i_flistp);

   srch_flistp = PIN_FLIST_CREATE(ebufp);
   pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1 ,ebufp);
   db = PIN_POID_GET_DB(pdp);
   
   PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "hp004");
   PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
   PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
   PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
   
   arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp);
   PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_NAME, arg_flist, PIN_FLD_DEVICE_ID, ebufp);
   
   result_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
   PIN_FLIST_FLD_SET(result_flistp, MSO_FLD_SERIAL_NO, NULL, ebufp);
   PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_DEVICE_TYPE, NULL, ebufp);
   PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
   
   PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "device inflistp", srch_flistp);
				
   PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_outflistp, ebufp);
   
   *ret_flistp = PIN_FLIST_COPY(srch_outflistp, ebufp);
   
//   PIN_FLIST_DESTROY_EX(&srch_flistp,ebufp);
}

static void
search_name_info(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
	pin_flist_t             **return_flistp,
	pin_errbuf_t            *ebufp)
{
    char				template [] = "select X from /account where F1 = V1 ";
    pin_flist_t		    *srch_flistp = NULL;
    pin_flist_t			*arg_flist = NULL;
    pin_flist_t			*srch_outflistp = NULL;
    pin_flist_t			*result_flistp = NULL;
    pin_flist_t			*name_flistp = NULL;
    poid_t				*pdp= NULL;
    int64                db =0;
    int32			srch_flag = 256;

    srch_flistp = PIN_FLIST_CREATE(ebufp);
    pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1 ,ebufp);
    db = PIN_POID_GET_DB(pdp);
    
    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "hp004");
    PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
    PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
    PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
    
    arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, arg_flist, PIN_FLD_POID, ebufp);
    
    result_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
    PIN_FLIST_FLD_SET(result_flistp, MSO_FLD_AREA, NULL, ebufp);
    name_flistp = PIN_FLIST_ELEM_ADD(result_flistp, PIN_FLD_NAMEINFO, 1, ebufp);
    PIN_FLIST_FLD_SET(name_flistp, PIN_FLD_ZIP, NULL, ebufp);
    
    
     PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                 "naminfo inflistp", srch_flistp);
    
    PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_outflistp, ebufp);
    
            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                 "search return", srch_outflistp);
    *return_flistp = PIN_FLIST_COPY(srch_outflistp, ebufp);
	
	//PIN_FLIST_DESTROY_EX(&srch_flistp,ebufp);

	}
/******************** VERIMATRIX CHANGES - END - New functions *************/

static void
fm_mso_get_purchased_prod_nds(
        pcm_context_t           *ctxp,
        pin_flist_t             *inp_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *s_flistp = NULL;
        pin_flist_t             *args_flistp = NULL;
        pin_flist_t             *res_flistp = NULL;
            pin_flist_t         *tmp_flistp= NULL;
		pin_flist_t         *tmp_flistp_ne= NULL;
        poid_t                  *s_pdp = NULL;
        int32                   flags = 256;
        int64                   db = 0;
        int64                   id = -1;
        char                    *s_template = " select X from /purchased_product 1,  /product 2 where 1.F1 = V1 and 1.F2 = V2 and 1.F3 = 2.F4  and product_t.provisioning_tag = 'PROV_NCF_PRO' ";
        int32                   act_status = 1;
                 char                    *tag = "PROV_NCF_PRO";
                 char                    *node = NULL;
                 char                    *ca_id = NULL;
        poid_t                  *serv_pdp = NULL;
        poid_t                  *act_pdp = NULL;
        pin_flist_t             *ret_flistp = NULL;
        pin_flist_t             *caid_flistp = NULL;
    pin_flist_t     *acnt_info = NULL;

        int32       business_type = 0;
    int32           arr_business_type[8];
    int32           lob_exclude_flag = -1;

    time_t          *end_tp = NULL;
    time_t          now_t = 0;
    struct          tm *current_time;
    int             year;
    int             month;
    int             day;
    char            validity[10];
    void            *vp = NULL;



        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

        act_pdp = (poid_t *)PIN_FLIST_FLD_GET(inp_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
        serv_pdp = (poid_t *)PIN_FLIST_FLD_GET(inp_flistp, PIN_FLD_POID, 0, ebufp);
		tmp_flistp = PIN_FLIST_SUBSTR_GET(inp_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
		node = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);
        db = PIN_POID_GET_DB(serv_pdp);
        s_flistp = PIN_FLIST_CREATE(ebufp);
        s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, serv_pdp, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &act_status, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);

        res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
                PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, NULL, ebufp);
                PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
                PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_PURCHASE_END_T, NULL, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_purchased_prod_nds_active search input list", s_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &ret_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_purchased_prod_nds_active - Error in calling SEARCH", ebufp);
                PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
                PIN_ERRBUF_CLEAR(ebufp);
                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_purchased_prod_nds_active search output flist", ret_flistp);

                if (PIN_FLIST_ELEM_COUNT(ret_flistp, PIN_FLD_RESULTS, ebufp) > 0)
                {
                        tmp_flistp = PIN_FLIST_SUBSTR_GET(ret_flistp, PIN_FLD_RESULTS, 0, ebufp);

                mso_prov_get_ca_id(ctxp, tag, node, &caid_flistp, ebufp);

                if(caid_flistp)
                {
                    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CA_ID check ");

                    ca_id = PIN_FLIST_FLD_GET(caid_flistp, MSO_FLD_CA_ID, 0, ebufp);
                    PIN_FLIST_FLD_SET(ret_flistp, MSO_FLD_CA_ID, ca_id, ebufp);
                    PIN_FLIST_FLD_SET(ret_flistp, MSO_FLD_TAPING_AUTHORIZATION, "N", ebufp);
                    fm_mso_get_account_info(ctxp, act_pdp, &acnt_info, ebufp);

                    vp = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_BUSINESS_TYPE, 0, ebufp);
                    if (vp && vp != NULL)
                    {
                        business_type = *(int32 *)vp;
                        num_to_array(business_type, arr_business_type);
                        lob_exclude_flag = arr_business_type[7];
                    }

                    end_tp = (time_t *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_PURCHASE_END_T, 0, ebufp);
                    if ((end_tp && *end_tp == 0) || (lob_exclude_flag == 1) ||
                            (lob_exclude_flag == 3) || (lob_exclude_flag == 5))
                    {
                        //now_t = pin_virtual_time((time_t *)NULL);
                        /* Temp work around now_t = now_t - 86400; 30-Apr-2021*/
                        now_t = 1703961000;
                        current_time = localtime(&now_t);
                        year = current_time->tm_year + 1900;
                        month = current_time->tm_mon + 1;
                        day = current_time->tm_mday;
                        if ((year % 4 != 0) && month == 2 && day == 29)
                        {
                            day = day - 1;
                        }
                        memset(validity,'\0',10);
                        sprintf(validity,"%04d%02d%02d",year,month,day);
                        PIN_FLIST_FLD_SET(ret_flistp, MSO_FLD_EXPIRATION_DATE, validity, ebufp);
                    }
                    else
                    {
                        *end_tp = *end_tp - 86400;
                        if (*end_tp > 1703961000)
                        {
                            *end_tp = 1703961000;
                        }
                        current_time = localtime(end_tp);
                        year = current_time->tm_year + 1900;
                        month = current_time->tm_mon +1;
                        day = current_time->tm_mday;
                        //memset(validity,10,'\0');
                        if ((year % 4 != 0) && month == 2 && day == 29)
                        {
                            day = day - 1;
                        }
                        memset(validity,'\0',10);
                        sprintf(validity,"%04d%02d%02d",year,month,day);
                        PIN_FLIST_FLD_SET(ret_flistp, MSO_FLD_EXPIRATION_DATE, validity, ebufp);
                    }


                    *ret_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
                    PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
                }
                }
                else
                {
                        *ret_flistpp = PIN_FLIST_COPY(inp_flistp, ebufp);

                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_purchased_prod_nds_active final output flist", *ret_flistpp);

}

