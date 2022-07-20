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
* 0.1    | 23/07/2014 | Someshwar         	  | Broadband Provisioning implementation 
* 0.2    | 12/12/2019 | Jyothirmayi Kachiraju | Added code to skip MAC validation for ONU (GPON) modem for JIO 
											    and Hathway Networks.
											    Added code to include the JIO Network Element ID check for setting 
											    MSO_FLD_AVP_NEW_VALUE field for different actions.
*--------------------------------------------------------------------------------------------------------------*
*
****************************************************************************************************************/

#ifndef lint
static  char Sccs_Id[] = "@(#)$Id: fm_mso_prov_create_bb_action.c /cgbubrm_7.3.2.rwsmod/1 2009/08/04 23:37:19 pjegan Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/timeb.h>
#include <time.h>
#include "pin_sys.h"
#include "pin_os.h"
#include "pcm.h"
#include "ops/cust.h"
#include "ops/bal.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "mso_prov.h"
#include "cm_cache.h"
#include "mso_errs.h"
#include "mso_cust.h"
#include "mso_utils.h"
#include "mso_device.h"
#include <pthread.h>

#define HW_NETWORK_STER_ID "STERLITEAAA"
#define MSO_FREE_MB 1100010
#define MSO_FUP_TRACK 1000104
#define TECH_NEW_CPS 9984

static int32		task_id = 1;

pthread_mutex_t cmts_update_mutex;
pthread_mutex_t bb_pt_update_mutex;


/*******************************************************************
 * external variable for bb provisioning config pointer
 *******************************************************************/
extern cm_cache_t *mso_prov_bb_config_ptr;
extern cm_cache_t *mso_prov_bb_cmts_ptr;
extern int32 refresh_pt_interval;
extern int32     *fup_capp_flag;
extern cm_cache_t *mso_prov_cap_plans_ptr;

/* Global variable for QPS to CPS upgrade */
char     other_ne_id[64]="";

extern void 
fm_mso_utils_sequence_no(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);


extern int
fm_mso_integ_match_patterns(
        char* pattern_string,
        char* input_string);

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

extern int
fm_mso_fetch_purchased_plan(
        pcm_context_t           *ctxp,
	int			*flags,
        pin_flist_t		*in_flistp,
        poid_t                  **o_poidpp,
        pin_errbuf_t            *ebufp);

extern int
mso_get_days_from_timestamp(
        time_t                  timeStamp);
extern int
mso_get_months_from_timestamp(
        time_t                  timeStamp);

/*******************************************************************
 * Fuctions defined in this code 
 *******************************************************************/

EXPORT_OP void
op_mso_prov_create_bb_action(
        cm_nap_connection_t    *connp,
        int32               opcode,
        int32               flags,
        pin_flist_t         *in_flistp,
        pin_flist_t         **ret_flistpp,
        pin_errbuf_t        *ebufp);

static void
fm_mso_prov_create_bb_action(
        pcm_context_t       *ctxp,
        pin_flist_t         *in_flistp,
        int32               flags,
        int32               *error_code,
        pin_flist_t         **out_flistpp,
        pin_errbuf_t        *ebufp);

void
set_error_descr(
    pin_flist_t             **ret_flistpp,
    int                     error_code,
    pin_errbuf_t            *ebufp);

static void
prepare_bb_activation_flist(
    pcm_context_t           *ctxp,
    pin_flist_t             **i_flistpp,
    char                    **act,
	int32			*error_code,
    pin_errbuf_t            *ebufp);
    
static void
prepare_bb_update_cap_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	**i_flistpp,
	char		**act,
	int32			*error_code,
	pin_errbuf_t	*ebufp);

static void    
prepare_bb_fup_reversal_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	**i_flistpp,
	char		**act,
	int32			*error_code,
	pin_errbuf_t	*ebufp);

/*RAVI added function*/
static void
prepare_bb_add_mb_flist(
        pcm_context_t   *ctxp,
        pin_flist_t     **i_flistpp,
        char            **act,
        int32           *error_code,
        pin_errbuf_t    *ebufp);


static void    
prepare_bb_deactivate_pkg_exp_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	**i_flistpp,
	char		**act,
	int32			*error_code,
	pin_errbuf_t	*ebufp);

static void    
prepare_bb_deactivate_pquota_exp_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	**i_flistpp,
	char		**act,
	int32			*error_code,
	pin_errbuf_t	*ebufp);

static void    
prepare_bb_suspend_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	**i_flistpp,
	char		**act,
	int32			*error_code,
	pin_errbuf_t	*ebufp);

static void    
prepare_bb_hold_serv_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	**i_flistpp,
	char		**act,
	int32			*error_code,
	pin_errbuf_t	*ebufp);

static void    
prepare_bb_topup_bexpiry_npc_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	**i_flistpp,
	char		**act,
	int32			*error_code,
	pin_errbuf_t	*ebufp);

static void    
prepare_bb_topup_bexpiry_pc_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	**i_flistpp,
	char		**act,
	int32			*error_code,
	pin_errbuf_t	*ebufp);

static void    
prepare_bb_renew_aexpiry_pre_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	**i_flistpp,
	char		**act,
	int32			*error_code,
	pin_errbuf_t	*ebufp);

static void    
prepare_bb_auto_renew_post_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	**i_flistpp,
	char		**act,
	int32			*error_code,
	pin_errbuf_t	*ebufp);

static void    
prepare_bb_reactivate_serv_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	**i_flistpp,
	char		**act,
	int32			*error_code,
	pin_errbuf_t	*ebufp);

static void    
prepare_bb_unhold_serv_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	**i_flistpp,
	char		**act,
	int32			*error_code,
	pin_errbuf_t	*ebufp);

static void    
prepare_bb_pc_quota_change_flist(
	pcm_context_t	*ctxp,
	int32		*action_flag,
	pin_flist_t	**i_flistpp,
	char		**act,
	int32		*error_code,
	pin_errbuf_t	*ebufp);

/*static void    
prepare_bb_pc_quota_nquota_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	**i_flistpp,
	char		**act,
	int32		*error_code,
	pin_errbuf_t	*ebufp);

static void    
prepare_bb_pc_nquota_quota_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	**i_flistpp,
	char		**act,
	int32		*error_code,
	pin_errbuf_t	*ebufp);

static void    
prepare_bb_pc_quota_quota_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	**i_flistpp,
	char		**act,
	int32		*error_code,
	pin_errbuf_t	*ebufp);

static void    
prepare_bb_pc_nquota_nquota_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	**i_flistpp,
	char		**act,
	int32		*error_code,
	pin_errbuf_t	*ebufp);*/

static void    
prepare_bb_pc_pre_post_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	**i_flistpp,
	char		**act,
	int32		*error_code,
	pin_errbuf_t	*ebufp);

static void    
prepare_bb_pc_post_pre_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	**i_flistpp,
	char		**act,
	int32		*error_code,
	pin_errbuf_t	*ebufp);

static void    
prepare_bb_cam_ti_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	**i_flistpp,
	char		**act,
	int32		*error_code,
	pin_errbuf_t	*ebufp);

static void    
prepare_bb_cam_tp_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	**i_flistpp,
	char		**act,
	int32		*error_code,
	pin_errbuf_t	*ebufp);

static void    
prepare_bb_cam_it_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	**i_flistpp,
	char		**act,
	int32		*error_code,
	pin_errbuf_t	*ebufp);

static void    
prepare_bb_cam_pt_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	**i_flistpp,
	char		**act,
	int32		*error_code,
	pin_errbuf_t	*ebufp);

static void    
prepare_bb_cam_ip_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	**i_flistpp,
	char		**act,
	int32			*error_code,
	pin_errbuf_t	*ebufp);

static void    
prepare_bb_cam_pi_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	**i_flistpp,
	char		**act,
	int32			*error_code,
	pin_errbuf_t	*ebufp);
//Added Kunal for NoCLICK Change
static void
prepare_bb_cam_tn_flist(
        pcm_context_t   *ctxp,
        pin_flist_t     **i_flistpp,
        char            **act,
        int32                   *error_code,
        pin_errbuf_t    *ebufp);

static void
prepare_bb_cam_nt_flist(
        pcm_context_t   *ctxp,
        pin_flist_t     **i_flistpp,
        char            **act,
        int32                   *error_code,
        pin_errbuf_t    *ebufp);

static void
prepare_bb_cam_in_flist(
        pcm_context_t   *ctxp,
        pin_flist_t     **i_flistpp,
        char            **act,
        int32                   *error_code,
        pin_errbuf_t    *ebufp);

static void
prepare_bb_cam_pn_flist(
        pcm_context_t   *ctxp,
        pin_flist_t     **i_flistpp,
        char            **act,
        int32                   *error_code,
        pin_errbuf_t    *ebufp);

static void
prepare_bb_cam_ni_flist(
        pcm_context_t   *ctxp,
        pin_flist_t     **i_flistpp,
        char            **act,
        int32                   *error_code,
        pin_errbuf_t    *ebufp);

static void
prepare_bb_cam_np_flist(
        pcm_context_t   *ctxp,
        pin_flist_t     **i_flistpp,
        char            **act,
        int32                   *error_code,
        pin_errbuf_t    *ebufp);

//END
static void    
prepare_bb_modem_change_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	**i_flistpp,
	char		**act,
	int32			*error_code,
	pin_errbuf_t	*ebufp);

static void    
prepare_bb_ip_change_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	**i_flistpp,
	char		**act,
	int32			*error_code,
	pin_errbuf_t	*ebufp);

static void    
prepare_bb_cmts_change_t_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	**i_flistpp,
	char		**act,
	int32			*error_code,
	pin_errbuf_t	*ebufp);

static void    
prepare_bb_cmts_change_nt_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	**i_flistpp,
	char		**act,
	int32			*error_code,
	pin_errbuf_t	*ebufp);

static void    
prepare_bb_change_name_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	**i_flistpp,
	char		**act,
	int32			*error_code,
	pin_errbuf_t	*ebufp);

static void    
prepare_bb_change_pass_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	**i_flistpp,
	char		**act,
	int32			*error_code,
	pin_errbuf_t	*ebufp);

static void    
prepare_bb_end_subs_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	**i_flistpp,
	char		**act,
	int32			*error_code,
	pin_errbuf_t	*ebufp);

static void    
prepare_bb_reset_device_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	**i_flistpp,
	char		**act,
	int32			*error_code,
	pin_errbuf_t	*ebufp);    

static void
prepare_docsis_qps_activation_flist(
	pcm_context_t			*ctxp, 
	pin_flist_t			**i_flistp, 
	pin_flist_t			*so_flistp,
	int32				*error_code,
	pin_errbuf_t			*ebufp);

static void
ethernet_details_flist(
	pcm_context_t			*ctxp, 
	pin_flist_t			**i_flistp, 
	pin_flist_t			*so_flistp,
	char				*task,
	int32				*error_code,
	pin_errbuf_t			*ebufp);
	
static void
prepare_docsis_bcc_flist(
	pcm_context_t			*ctxp, 
	pin_flist_t			**i_flistp, 
	pin_flist_t			*so_flistp,
	int32				*error_code,
	pin_errbuf_t			*ebufp);

static void
prepare_bb_add_multiple_ip_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	**i_flistpp,
	char		**act,
	int		*error_codep,
	pin_errbuf_t	*ebufp);

static void
prepare_bb_remove_cred_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	**i_flistpp,
	char		**act,
	int		*error_codep,
	pin_errbuf_t	*ebufp);

static void
fm_mso_get_device_details(
    pcm_context_t			*ctxp,
    pin_flist_t				**i_flistpp,
    char				*device_id,
    int32				*error_code,
    pin_errbuf_t			*ebufp);

static void
fm_mso_get_account_details(
	pcm_context_t			*ctxp,
	poid_t				*ac_pdp,
	char				**f_name,
	char				**l_name,
	char				**acc_no,
	int32				*error_code,
	pin_errbuf_t			*ebufp);
	
static void
fm_mso_add_create_bal(
	pcm_context_t			*ctxp,
	pin_flist_t			**i_flistpp,
	poid_t				*ac_pdp,
	pin_flist_t			**createbalflistp,
	int				*plan_type,
	pin_decimal_t			**initial_amount,
	int				srv_typ,
	int				*error_codep,
	pin_errbuf_t			*ebufp);	
	
static void
fm_mso_add_cable_modem(
	pcm_context_t			*ctxp,
	pin_flist_t			**i_flistpp,
	pin_flist_t			*so_flistp,
	int32				*action,
	pin_flist_t			**cablemodemflistp,
	int32				*error_code,
	pin_errbuf_t			*ebufp);	

int
fm_quota_check(
	pcm_context_t			*ctxp, 
	char				*plan_name, 
	char				*prov_tag, 
	int				*tech,
	int				*plan_typ,
	pin_decimal_t			**initial_amount,
	int				*error_codep, 
	pin_errbuf_t			*ebufp);

int static
fm_quota_check_old(
	pcm_context_t	*ctxp,
	poid_t		*prod_pdp,
	int		*error_codep,
	pin_errbuf_t	*ebufp);

void
fm_mso_search_devices(
	pcm_context_t			*ctxp,
	char				*param,
	poid_t				*service_obj,
	pin_flist_t			**deviceflistp,
	int32				*error_code,
	pin_errbuf_t			*ebufp);

/* Changes for QPS to CPS Migration */
static void
prepare_switch_service_flist(
	pcm_context_t			*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp,
	char				*act,
	int32				*error_code,
	pin_errbuf_t			*ebufp);

/* Changes for QPS to CPS upgrade. Passing the act list for making activity specific decision */	
static void
prepare_docsis_subscriberavps_flist(
	pcm_context_t			*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp,
	char				*act,
	int32				*error_code,
	pin_errbuf_t			*ebufp);

static void
prepare_docsis_update_avp_qnq(
	pcm_context_t			*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp,
	char				*act,
	int32				*error_code,
	pin_errbuf_t			*ebufp);

static void
prepare_docsis_update_avp_qq(
	pcm_context_t			*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp,
	int32				*error_code,
	pin_errbuf_t			*ebufp);

static void
prepare_docsis_subscriberavps2_flist(
	pcm_context_t			*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp,
	char				*act,
	int32				*error_code,
	pin_errbuf_t			*ebufp);
	
static void
delete_cred_qps_flist(
	pcm_context_t			*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp, 
	int32				*error_code,
	pin_errbuf_t			*ebufp);	
	
static void
add_cred_qps_flist(
	pcm_context_t			*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp,
	int32				*error_code,
	pin_errbuf_t			*ebufp);
	
static void
fm_mso_create_bal_qps(
	pcm_context_t			*ctxp, 
	pin_flist_t			**i_flistp, 
	pin_flist_t			*so_flistp,
	int32				*error_code,
	pin_errbuf_t			*ebufp);

static void
fm_mso_change_username_qps(
	pcm_context_t			*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp,
	int32				*error_code,
	pin_errbuf_t			*ebufp);

static void
fm_mso_delete_subscriber(
	pcm_context_t			*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp,
	int32				*error_code,
	pin_errbuf_t			*ebufp);

static void
fm_mso_delete_records(
	pcm_context_t			*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp, 
	int32				*error_code,
	pin_errbuf_t			*ebufp);
	

static void
fm_mso_search_client_classes(
	pcm_context_t			*ctxp,
	char				*prov_tag,
	char				*node_name,
	char				*srv_code,
    char            *str_rolep,
	int				*action,
	int				*isTAL,
	int				ip_count,
	int				*tech,
	char				*dev_type,
	char				**client_class,
	int				*bill_when,
	char				*city,
	int32				*error_code,
	pin_errbuf_t			*ebufp);

static void
fm_mso_get_tal_class(
	pcm_context_t			*ctxp, 
	char				*node_name, 
	char				*prov_tag, 
	char				**tal_class,
    char            *str_rolep,
	int				*isTAL,
	int				*tech,
	int32				*error_code,
	pin_errbuf_t			*ebufp);

static void
fm_mso_get_hw_class(
	pcm_context_t			*ctxp, 
	char				*node_name, 
	int				flag, 
	char				*dev_type, 
	char				**hardware_class, 
	int32				*error_code,
	pin_errbuf_t			*ebufp);

static void
fm_mso_get_speed(
	pcm_context_t			*ctxp, 
	char				*prov_tag, 
	int				*tech, 
	char				*srv_code, 
	//Added new parameter
	int				*action,
	char				**package_class, 
	int				*bill_when,
	char				*city,
	int32				*error_code,
	pin_errbuf_t			*ebufp);


static void
fm_mso_add_personal_detail(
	pcm_context_t			*ctxp,
	poid_t				*ac_pdp,
	pin_flist_t			**account_detail_flistp,
	int32				*error_code,
	pin_errbuf_t			*ebufp);

static void
fm_mso_add_acc_detail(
	pcm_context_t			*ctxp,
	pin_flist_t			*i_flistpp,
	pin_flist_t			*so_flistp,
	pin_flist_t			**account_detail_flistp,
	char				*subnet,
	int32				*error_code,
	pin_errbuf_t			*ebufp);

static void
fm_mso_add_group_detail(
	pcm_context_t			*ctxp,
	pin_flist_t			*in_flistp,
	pin_flist_t			*so_flistp,
	pin_flist_t			**group_detail_flistp,
	int32				*error_code,
	pin_errbuf_t			*ebufp);

static void
fm_mso_add_user_attribute(
	pcm_context_t			*ctxp,
	pin_flist_t			*so_flistp,
	pin_flist_t			**user_att_flistp,
	char				*subnet,
	int32				*error_code,
	pin_errbuf_t			*ebufp);

static void
get_last_plan(
	pcm_context_t			*ctxp,
	poid_t				*ac_pdp,
	char				**plan_name,
	time_t				**end_t,
	poid_t				**prod_pdp,
	int32				*error_code,
	pin_errbuf_t			*ebufp);

static void
get_cancelled_plan(
	pcm_context_t			*ctxp,
	poid_t				*ac_pdp,
	char				*city,
	char				*plan_name,
	time_t				**end_t,
	poid_t				**prod_pdp,
	int32				*error_code,
	int				*tech,
	int                             *action_flag,
	int32			status,
	pin_errbuf_t			*ebufp);
	
static void
ethernet_new_plan(
	pcm_context_t			*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp,
	int32				*error_code,
	pin_errbuf_t			*ebufp);

static void
fm_mso_change_password(
	pcm_context_t			*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp,
	int32				*error_code,
	pin_errbuf_t			*ebufp);

static void
prepare_bb_mac_change_flist(
	pcm_context_t			*ctxp, 
	pin_flist_t			**i_flistpp, 
	char				**act, 
	int32				*error_code,
	pin_errbuf_t			*ebufp);

static void
prepare_bb_add_subnet_flist(
	pcm_context_t			*ctxp, 
	pin_flist_t			**i_flistpp, 
	char				**act,
	int32				*error_code,
	pin_errbuf_t			*ebufp);

static void
prepare_bb_get_user_detail_flist(
	pcm_context_t			*ctxp, 
	pin_flist_t			**i_flistpp, 
	char				**act,
	int32				*error_code,
	pin_errbuf_t			*ebufp);

static void
prepare_bb_get_last_session_flist(
	pcm_context_t			*ctxp, 
	pin_flist_t			**i_flistpp, 
	char				**act,
	int32				*error_code,
	pin_errbuf_t			*ebufp);

static void
prepare_bb_update_subs_pool_flist(
	pcm_context_t			*ctxp, 
	pin_flist_t			**i_flistpp, 
	char				**act,
	int32				*error_code,
	pin_errbuf_t			*ebufp);

static void
prepare_bb_release_ip_flist(
	pcm_context_t			*ctxp, 
	pin_flist_t			**i_flistpp, 
	char				**act,
	int32				*error_code,
	pin_errbuf_t			*ebufp);

static void
fm_mso_change_username_nsure(
	pcm_context_t			*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp,
	int32				*error_code,
	pin_errbuf_t			*ebufp);

static void
fm_mso_delete_ethernet_subs(
	pcm_context_t			*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp, 
	int32				*error_code,
	pin_errbuf_t			*ebufp);

static void
fm_mso_get_ethernet_details(
	pcm_context_t			*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp, 
	char				*task,
	int32				*error_code,
	pin_errbuf_t			*ebufp);

static void
fm_mso_tal_release_ip(
	pcm_context_t			*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp,
	int32				*error_code,
	pin_errbuf_t			*ebufp);

static int
fm_mso_get_srv_type(
	pcm_context_t			*ctxp, 
	char				*plan_name, 
	char				*prov_tag, 
	int				*tech,
	int				*bill_when,
	char				*city,
	int32				*error_code,
	char				*svc_code,
	char				*quota_code,
	pin_errbuf_t			*ebufp);

static void
fm_mso_get_network_node(
	pcm_context_t			*ctxp,
	char				*node_name,
	char				*city,
	char				**ne_add,
	int				flag,
	int32				*error_code,
	pin_errbuf_t			*ebufp);

static void
fm_mso_get_login(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	char			**login,
	pin_flist_t		*so_flistp,
	int			*error_codep,
	pin_errbuf_t		*errbufp);

static void
fm_mso_get_ip(
	pcm_context_t		*ctxp,
	char			**login,
	pin_flist_t		*i_flistp,
	pin_flist_t		*so_flistp,
	int			*error_codep,
	pin_errbuf_t		*ebufp);

static void
fm_mso_get_mac(
	pcm_context_t			*ctxp,
	poid_t				*serv_poid,
	char				**mac_id,
	int				*error_codep,
	pin_errbuf_t			*ebufp);

time_t
fm_prov_get_expiry(
	pcm_context_t			*ctxp, 
	poid_t				*mso_plan,
	poid_t				*ac_pdp,
	poid_t				*srv_pdp,
	int				srv_typ, 
	time_t				*start,
	pin_errbuf_t			*ebufp);

static time_t
fm_prov_get_expiry_from_calc_only_out(
	pcm_context_t			*ctxp, 
	pin_flist_t			*in_flist, 
	int				srv_typ,
	pin_errbuf_t			*ebufp);

static void
fm_mso_prov_bb_config_cache_update(
	pcm_context_t *ctxp,
	pin_errbuf_t  *ebufp);

static void
fm_mso_prov_cmts_config_cache_update(
	pcm_context_t *ctxp,
	pin_errbuf_t  *ebufp);

/*RAVI added function*/
static void
prepare_bb_plan_validity_ext_flist(
        pcm_context_t   *ctxp,
        pin_flist_t     **i_flistpp,
        char            **act,
        int32           *error_code,
        pin_errbuf_t    *ebufp);

void
mso_prov_fetch_quota_amount(
        pcm_context_t   *ctxp,
        poid_t          *product_pdp,
        int32           req_res_id,
        pin_decimal_t   **total_amtp,
        pin_errbuf_t    *ebufp);

static void
get_old_group(
	pcm_context_t			*ctxp,
	pin_flist_t			*i_flist,
	char				**plan_name,
	time_t				**end_t,
	poid_t				**prod_pdp,
	int				*error_codep,
	pin_errbuf_t			*ebufp);

static void
ethernet_extend_validity(
	pcm_context_t			*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp,
	int				*error_codep,
	pin_errbuf_t			*ebufp);

extern void
fm_mso_get_service_quota_codes(
        pcm_context_t                   *ctxp,
        char                            *plan_name,
        int32                           *bill_when,
        char                            *city,
        pin_flist_t                     **r_flistpp,
        pin_errbuf_t                    *ebufp);

static int32
fm_mso_prov_quota_cap_eligibility(
        pcm_context_t   *ctxp,
        pin_flist_t     *payload_flistp,
        int32           tech,
        char          **initial_amtp,
        pin_errbuf_t    *ebufp);

extern void num_to_array(
        int     num,
        int     arr[]);


/*******************************************************************
 * Main routine for the MSO_OP_CREATE_BB_ACTION command
 *******************************************************************/
void
op_mso_prov_create_bb_action(
	cm_nap_connection_t		*connp,
	int32				opcode,
	int32				flags,
	pin_flist_t			*in_flistp,
	pin_flist_t			**ret_flistpp,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*r_flistp = NULL;
	pcm_context_t			*ctxp = connp->dm_ctx;
	int32				oap_success = 0;
	int32				oap_failure = 1;
	int32				*prov_flag =  NULL;
	int32				error_code = 0;
	poid_t				*pdp = NULL;
	int32				t_status;
	char				code[10];


    /***********************************************************
     * Null out results until we have some.
     ***********************************************************/
    *ret_flistpp = NULL;
    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Insanity check.
     ***********************************************************/
    if (opcode != MSO_OP_PROV_CREATE_BB_ACTION) {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_OPCODE, 0, 0, opcode);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "bad opcode in op_mso_prov_create_bb_action", ebufp);
        return;
    }

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "op_mso_prov_create_bb_action input flist", in_flistp);

	/* Passing service obj for locking */
    pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
    t_status = fm_mso_trans_open(ctxp, pdp, 3, ebufp);

    prov_flag = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_FLAGS, 0 , ebufp);

    /***********************************************************
     * Call main function to do it
     ***********************************************************/
    fm_mso_prov_create_bb_action(ctxp, in_flistp, flags, &error_code, &r_flistp, ebufp);

    /***********************************************************
     * Results.
     ***********************************************************/
    if (PIN_ERRBUF_IS_ERR(ebufp)) 
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_mso_prov_create_bb_action error", ebufp);
	
	PIN_ERRBUF_CLEAR(ebufp);
        if (t_status == 0)
        {
            fm_mso_trans_abort(ctxp, pdp, ebufp);
        }

//        PIN_ERRBUF_CLEAR(ebufp);
	*ret_flistpp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
	if (error_code)
	{
	    set_error_descr(ret_flistpp, error_code, ebufp);
	}
	else
	{
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
         "op_mso_prov_create_bb_action return flist", *ret_flistpp);

    return;
}

/*******************************************************************
 * fm_mso_prov_create_bb_action()
 *
 * Perfomrs following action:
 * 1. call action specific function to enrich the input from service
 *        and package detail
 * 2. get provisioning sequence 
 * 3. create provisioning order /mso_prov_bb_order
 *******************************************************************/
static void
fm_mso_prov_create_bb_action(
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
    poid_t              *purchased_plan_obj = NULL;
    poid_t              *plan_pdp = NULL;
    pin_flist_t         *payload_flistp = NULL;
    pin_flist_t         *order_i_flistp     = NULL;
    pin_flist_t         *tmp_flistp= NULL;
    pin_flist_t         *task_flist3= NULL;
    pin_flist_t         *read_mso_plan_flistp= NULL;
    pin_flist_t         *read_mso_plan_oflistp= NULL;
    pin_flist_t         *read_oflistp= NULL;
    pin_flist_t         *prod_flistp= NULL;
    pin_flist_t         *order_seq_i_flistp = NULL;
    pin_flist_t         *order_seq_o_flistp = NULL;
    pin_flist_t	    	*rfld_iflistp = NULL;
    pin_flist_t		    *rfld_oflistp = NULL;
    pin_flist_t	    	*temp_flistp = NULL;
    pin_buf_t           *nota_buf     = NULL;
    char                *flistbuf = NULL;
    char                *prov_tag = NULL;
    int                 flistlen=0;
    int32       		count = 0;
    pin_cookie_t    	cookie = NULL;
    int32	        	rec_id = 0;
    int                 status = MSO_PROV_STATE_NEW;
    int		        	pri = 1;
    char                *act = NULL;
    int		        	*tech = 0;
//  mso_prov_str_buf_t  dbuf;
    char	        	str[128];

    int	            	rv = 0;
    int		            flg = 0;
    int 	            srch_plan_type = 0;	
    pin_flist_t		    *create_bal_flist = NULL;
    char		        *initial_amt = NULL;	
    char		        *cap_quota = "21474836480";
    pin_decimal_t	    *initial_amt_dec = NULL;
    pin_decimal_t	    *cap_quota_dec = pbo_decimal_from_str("21474836480", ebufp); 
    int			        task_id_p = 0;
    pin_cookie_t	    cookie_p = NULL;
    int                 task_id_c = 0;
    pin_cookie_t        cookie_c = NULL;

    int32		        is_eligible = 0;
    int32		        is_capped = 0;
    struct tm           *timeeff;
    time_t		        *start_t = NULL;
    struct tm           *gm_time;
    time_t		        expiry;
    time_t              *sub_expiryp = NULL;
    char                time_buf[30] = {'\0'};
    time_t		        now = 0;
    char                *ne_add = NULL;
    char                *city = NULL;
    char                *node_name = NULL;
    char                *str_rolep = NULL;
    char	            msg[100];



    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_prov_create_bb_action input flist", in_flistp);
    
    task_id = 1;

    /***********************************************************
     * Check mandatory fields are available
     ***********************************************************/
    service_poidp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
	//Read the Service Poid for Technology
	rfld_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(rfld_iflistp, PIN_FLD_POID, (void *)service_poidp, ebufp);
	//Add the BB Info SUBSTR
	temp_flistp = PIN_FLIST_SUBSTR_ADD(rfld_iflistp, MSO_FLD_BB_INFO, ebufp);
        PIN_FLIST_FLD_SET(temp_flistp, MSO_FLD_TECHNOLOGY, (void *)0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_prov_create_bb_action: Read Service In flist", rfld_iflistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, rfld_iflistp, &rfld_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_prov_create_bb_action error: Error", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_prov_create_bb_action: Read Service Out flist", rfld_oflistp);

	if(rfld_oflistp && rfld_oflistp != NULL)
	temp_flistp = PIN_FLIST_SUBSTR_GET(rfld_oflistp, MSO_FLD_BB_INFO, 0, ebufp);
	
	if (temp_flistp && temp_flistp != NULL)
	tech = (int *)PIN_FLIST_FLD_GET(temp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp);
    str_rolep = (char *)PIN_FLIST_FLD_GET(temp_flistp, MSO_FLD_ROLES, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_prov_create_bb_action: Error", ebufp);
		goto cleanup;
	}
	if(*tech != DOCSIS2 && *tech != DOCSIS3 && *tech != ETHERNET && *tech != GPON && *tech != FIBER)
	{
		*error_codep = MSO_BB_INV_TECH;
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, MSO_FLD_TECHNOLOGY, 0, 0);
		goto cleanup;
	}
	//Free the memory
	if(rfld_iflistp)PIN_FLIST_DESTROY_EX(&rfld_iflistp, NULL);

    purchased_plan_obj = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, 0, ebufp);
    action_flag = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_FLAGS, 0, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp)) {
	    *error_codep = MSO_BB_INTERNAL_ERROR;
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "fm_mso_prov_create_bb_action error: required field missing in input flist", ebufp);
        goto cleanup;
    }

	PIN_ERR_LOG_POID (PIN_ERR_LEVEL_DEBUG,"fm_mso_prov_create_bb_action error: purchased_plan poid",purchased_plan_obj);

	/* Changes for QPS to CPS migration. Set the global variable to indicate CPS */
	if(temp_flistp && temp_flistp != NULL)
	{
		node_name = (char *)PIN_FLIST_FLD_GET(temp_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
		city = (char *)PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_CITY, 0, ebufp);
	}

        //Call the function to get the NE Address
        fm_mso_get_network_node(ctxp, node_name, city, &ne_add, 1, error_codep, ebufp);

       if(ne_add && ne_add != NULL){
                /* Changes for QPS to CPS upgrade . Set global variable capturing other_ne_id */
                strncpy(other_ne_id,ne_add,11);
        }
        sprintf(msg,"Value of other_ne_id %s ",other_ne_id);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);




	/******* Pavan Bellala 17-08-2015 *******************************
	Added functionality for reset device provisioning action
	The mso_purchased_plan is -1 here, hence search and fetch from DB
	******** Pavan Bellala 17-08-2015 *******************************/
	if((int64)-1 == (int64)PIN_POID_GET_ID(purchased_plan_obj)){
		flg = MSO_FLAG_SRCH_BY_ACCOUNT;
		srch_plan_type = PLAN_TYPE_SUBS;
		PIN_FLIST_FLD_SET(in_flistp,MSO_FLD_PLAN_TYPE,&srch_plan_type,ebufp);
		rv = fm_mso_fetch_purchased_plan(ctxp,&flg,in_flistp,&purchased_plan_obj,ebufp);
		if(rv == PIN_RESULT_FAIL){
			*error_codep = MSO_BB_RESET_DEVICE_ERROR;
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_BAD_VALUE, PIN_FLD_PLAN_OBJ, 0, 0);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_prov_create_bb_action error: required field missing in input flist", ebufp);
        			goto cleanup;
		} else {
			PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG,"fm_mso_prov_create_bb_action:found purchased_plan poid",purchased_plan_obj);
		}
	}		
    if (*action_flag == DOC_BB_FUP_REVERSAL)
    {
        purchased_plan_obj = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_OLD_PURCHASED_PLAN_OBJ, 1, ebufp);
    }
    if (purchased_plan_obj == NULL)
    {
         purchased_plan_obj = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, 1, ebufp);
    }

	read_mso_plan_flistp = PIN_FLIST_CREATE(ebufp);
	
	PIN_FLIST_FLD_SET(read_mso_plan_flistp, PIN_FLD_POID, purchased_plan_obj, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_prov_create_bb_action: Read input flist", read_mso_plan_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_mso_plan_flistp, &read_mso_plan_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"fm_mso_prov_create_bb_action error: read mso plan", ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_prov_create_bb_action: Read output flist", read_mso_plan_oflistp);
	while ( (prod_flistp = PIN_FLIST_ELEM_GET_NEXT (read_mso_plan_oflistp,
                       PIN_FLD_PRODUCTS, &rec_id, 1,&cookie, ebufp)) != (pin_flist_t *)NULL ) 
        {
        	prov_tag = PIN_FLIST_FLD_GET(prod_flistp, PIN_FLD_PROVISIONING_TAG, 0, ebufp);
        	if ( prov_tag && strlen(prov_tag) > 0)
		{
		    PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_PROVISIONING_TAG, prov_tag, ebufp);
            if ((strcmp(other_ne_id, HW_NETWORK_STER_ID) == 0))
            {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"DEBUG:::::::::: INSIDE STR CONDITION");
                PIN_FLIST_DESTROY_EX(&read_mso_plan_flistp, NULL);
                read_mso_plan_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(prod_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, read_mso_plan_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(read_mso_plan_flistp, PIN_FLD_PURCHASE_END_T, NULL, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"readobj: Read input  flist", read_mso_plan_flistp);
	            if (PIN_ERRBUF_IS_ERR(ebufp)) {
	             	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
	          		"fm_mso_prov_create_bb_action error: Error", ebufp);
	             	goto cleanup;
                	}
                PCM_OP(ctxp, PCM_OP_READ_FLDS,0, read_mso_plan_flistp, &read_oflistp, ebufp);
                 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"readobj: Read input  flist", read_oflistp);
                PIN_FLIST_FLD_COPY(read_oflistp, PIN_FLD_PURCHASE_END_T, in_flistp, PIN_FLD_PURCHASE_END_T, ebufp);
	            PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
            }
		    break;
		}
	}
	plan_pdp = PIN_FLIST_FLD_GET(read_mso_plan_oflistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
	if(PIN_POID_IS_NULL(plan_pdp))
	{
		*error_codep = MSO_BB_PLAN_OBJ_MISSING;
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_PLAN_OBJ, 0, 0);
		goto cleanup;
	}
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_PLAN_OBJ_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_prov_create_bb_action error: Error", ebufp);
		goto cleanup;
	}
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);

    purchased_plan_obj = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, 0, ebufp);
	
	
    //payload_flistp = PIN_FLIST_CREATE(ebufp);
    payload_flistp = PIN_FLIST_COPY(in_flistp,ebufp);



    if ( *action_flag ==  CATV_PREACTIVATION )
    {
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_OBJ, payload_flistp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, payload_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
    }
    
    //Copy the Purchased Plan poid
    PIN_FLIST_FLD_SET(payload_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, 
	    (void *)purchased_plan_obj, ebufp);
    /***********************************************************
     * Enrich input specific to action 
     ***********************************************************/
    if ( *action_flag ==  DOC_BB_ACTIVATION )
    {
        prepare_bb_activation_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag ==  DOC_BB_UPDATE_CAP )
    {
        prepare_bb_update_cap_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag ==  DOC_BB_FUP_REVERSAL )
    {
        prepare_bb_fup_reversal_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag ==  DOC_BB_DEACTIVATE_PKG_EXP )
    {
        prepare_bb_deactivate_pkg_exp_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag ==  DOC_BB_DEACTIVATE_PQUOTA_EXP )
    {
        prepare_bb_deactivate_pquota_exp_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag ==  DOC_BB_SUSPEND )
    {
        prepare_bb_suspend_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag ==  DOC_BB_HOLD_SERV )
    {
        prepare_bb_hold_serv_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag ==  DOC_BB_TOPUP_BEXPIRY_NPC )
    {
        prepare_bb_topup_bexpiry_npc_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag ==  ETH_BB_TOPUP_BEXPIRY_LIMITED )
    {
        prepare_bb_topup_bexpiry_npc_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag ==  DOC_BB_TOPUP_BEXPIRY_PC )
    {
        prepare_bb_topup_bexpiry_pc_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag ==  DOC_BB_RENEW_AEXPIRY_PRE )
    {
        prepare_bb_renew_aexpiry_pre_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag ==  DOC_BB_AUTO_RENEW_POST )
    {
	prepare_bb_fup_reversal_flist(ctxp, &payload_flistp, &act, error_codep, ebufp); 
        //prepare_bb_auto_renew_post_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag ==  DOC_BB_REACTIVATE_SERV )
    {
        prepare_bb_reactivate_serv_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag ==  DOC_BB_UNHOLD_SERV )
    {
        prepare_bb_unhold_serv_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
	else if ( *action_flag ==  DOC_BB_PC_QUOTA_NQUOTA || *action_flag ==  DOC_BB_PC_NQUOTA_QUOTA ||
			*action_flag ==  DOC_BB_PC_QUOTA_QUOTA || *action_flag ==  DOC_BB_PC_NQUOTA_NQUOTA)
    {
	prepare_bb_pc_quota_change_flist(ctxp, action_flag, &payload_flistp, &act, error_codep, ebufp);
    }
    /*else if ( *action_flag ==  DOC_BB_PC_QUOTA_NQUOTA )
    {
        prepare_bb_pc_quota_nquota_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag ==  DOC_BB_PC_NQUOTA_QUOTA )
    {
        prepare_bb_pc_nquota_quota_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag ==  DOC_BB_PC_QUOTA_QUOTA )
    {
        prepare_bb_pc_quota_quota_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag ==  DOC_BB_PC_NQUOTA_NQUOTA )
    {
        prepare_bb_pc_nquota_nquota_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }*/
    else if ( *action_flag ==  DOC_BB_PC_PRE_POST )
    {
        prepare_bb_pc_pre_post_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag ==  DOC_BB_PC_POST_PRE )
    {
        prepare_bb_pc_post_pre_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag ==  DOC_BB_CAM_TI )
    {
        prepare_bb_cam_ti_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag ==  DOC_BB_CAM_TP )
    {
        prepare_bb_cam_tp_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag ==  DOC_BB_CAM_IT )
    {
        prepare_bb_cam_it_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag ==  DOC_BB_CAM_PT )
    {
        prepare_bb_cam_pt_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag ==  DOC_BB_CAM_IP )
    {
        prepare_bb_cam_ip_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag ==  DOC_BB_CAM_PI )
    {
        prepare_bb_cam_pi_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    //Kunal : Added to handle NOCLICK Changes
    else if ( *action_flag ==  DOC_BB_CAM_TN )
    {
        prepare_bb_cam_tn_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag ==  DOC_BB_CAM_NT )
    {
        prepare_bb_cam_nt_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag ==  DOC_BB_CAM_IN )
    {
        prepare_bb_cam_in_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag ==  DOC_BB_CAM_PN )
    {
        prepare_bb_cam_pn_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag ==  DOC_BB_CAM_NI )
    {
        prepare_bb_cam_ni_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag ==  DOC_BB_CAM_NP )
    {
        prepare_bb_cam_np_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
   //END
    else if ( *action_flag ==  DOC_BB_MODEM_CHANGE )
    {
        prepare_bb_modem_change_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag ==  DOC_BB_IP_CHANGE )
    {
        prepare_bb_ip_change_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag ==  DOC_BB_CMTS_CHANGE_T )
    {
        prepare_bb_cmts_change_t_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag ==  DOC_BB_CMTS_CHANGE_NT )
    {
        prepare_bb_cmts_change_nt_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag ==  DOC_BB_CHANGE_NAME )
    {
        prepare_bb_change_name_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag ==  DOC_BB_CHANGE_PASS )
    {
        prepare_bb_change_pass_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag ==  DOC_BB_END_SUBS )
    {
        prepare_bb_end_subs_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag ==  DOC_BB_RESET_DEVICE )
    {
        prepare_bb_reset_device_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag == ETH_BB_MAC_CHANGE )
    {
	    prepare_bb_mac_change_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag == ETH_BB_ADD_SUBNET_MASK )
    {
	    prepare_bb_add_subnet_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag == ETH_BB_GET_USER_DETAIL )
    {
	    prepare_bb_get_user_detail_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag == ETH_BB_GET_LAST_SESSION_DETAIL )
    {
	    prepare_bb_get_last_session_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag == ETH_BB_UPDATE_SUBS_POOL )
    {
	    prepare_bb_update_subs_pool_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    else if ( *action_flag == ETH_BB_RELEASE_IP_ADDRESS )
    {
	    prepare_bb_release_ip_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    /*RAVI added a flag */
    else if ( *action_flag ==  DOC_BB_ADD_MB )	
    {	
		prepare_bb_add_mb_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
    /*RAVI added a flag */
    else if ( *action_flag ==  DOC_BB_PLAN_VALIDITY_EXTENSION )	
    {	
		prepare_bb_plan_validity_ext_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
	else if ( *action_flag ==  FIBER_BB_ADD_IP)	
    {	
		prepare_bb_add_multiple_ip_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
    }
	else if ( *action_flag ==  GPON_MAC_CHANGE)	
	{	
		prepare_bb_remove_cred_flist(ctxp, &payload_flistp, &act, error_codep, ebufp);
	}
    else
    {
        //Invalid flag : set error
        *error_codep = MSO_BB_INVALID_ACTION_CODE;
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
        goto cleanup;
    }
	
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
        *error_codep = MSO_BB_SEQNO_FAIL_CODE;
        goto cleanup;
    }

    if (str_rolep)
    {
        PIN_FLIST_FLD_SET(payload_flistp, MSO_FLD_ROLES, str_rolep, ebufp);
    }
    PIN_FLIST_FLD_COPY(order_seq_o_flistp, PIN_FLD_STRING, payload_flistp, PIN_FLD_ORDER_ID, ebufp);
    PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_ACTION, act, ebufp); 
    /*
     * Convert flist to Buffer. it will be used by Provising system.
     */
	if (PIN_ERRBUF_IS_ERR(ebufp) || payload_flistp == NULL) {
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"fm_mso_prov_create_bb_action error: null payload", ebufp);
		goto cleanup;
	}

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"payload_flistp",payload_flistp);
    /**condition to check for the Initial amount in the payload flist before creating prov order***/ 

    //change for 300 GB CAP Start
    is_eligible = fm_mso_prov_quota_cap_eligibility(ctxp, payload_flistp, *tech, &cap_quota, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp)) {
       	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
        "fm_mso_prov_create_bb_action error:  error in checking cap", ebufp);
        goto cleanup;
    }

    //change for 300 GB CAP End

    while ( (tmp_flistp = PIN_FLIST_ELEM_GET_NEXT (payload_flistp,
                                MSO_FLD_TASK, &task_id_p, 1,&cookie_p, ebufp))
                                                          != (pin_flist_t *)NULL )
    {
	cookie_c = NULL;
	task_id_c = 0;
	while ( (create_bal_flist = PIN_FLIST_ELEM_GET_NEXT (tmp_flistp,
                                MSO_FLD_CREATEBALANCE, &task_id_c, 1,&cookie_c, ebufp))
                                                          != (pin_flist_t *)NULL )
	{
		initial_amt = PIN_FLIST_FLD_GET(create_bal_flist, MSO_FLD_INITIALAMOUNT, 1, ebufp);
		start_t = PIN_FLIST_FLD_GET(create_bal_flist, MSO_FLD_STARTDATE, 1, ebufp);	
		//change for 300 GB CAP start
		if(initial_amt)
		{
			initial_amt_dec = pbo_decimal_from_str(initial_amt, ebufp);
		}
		if(is_eligible && !pbo_decimal_is_null(initial_amt_dec, ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Eligible for 300 GB Cap");
			is_capped = 1;
			PIN_FLIST_FLD_SET(create_bal_flist, MSO_FLD_INITIALAMOUNT, cap_quota, ebufp);
			if(start_t){
				now = pin_virtual_time(NULL);
				now = fm_utils_time_round_to_midnight(now);
                timeeff = localtime(&now);
                if ((strcmp(other_ne_id,HW_NETWORK_STER_ID) != 0))
                {
				    strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%T.000+0530", timeeff);
                }
                else
                {
                    sprintf(time_buf, "%d000", now);
                }
			 	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Start Time in capp:");
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, time_buf);
				PIN_FLIST_FLD_SET(create_bal_flist, MSO_FLD_STARTDATE, (void *)time_buf, ebufp);
				timeeff->tm_mday = timeeff->tm_mday + 1;
				expiry = mktime (timeeff);
                expiry = expiry -1;
                if ((strcmp(other_ne_id,HW_NETWORK_STER_ID) != 0))
                {
				    gm_time = localtime(&expiry);
				    strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%T.000+0530", gm_time);
                }
                else
                {
                    sprintf(time_buf, "%d000", expiry);
                }
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Expiry Time in capp:");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, time_buf);
				PIN_FLIST_FLD_SET(create_bal_flist, MSO_FLD_EXPIRATIONDATE, (void *)time_buf, ebufp);
			}
		}
		//change for 300 GB CAP end
	}
    }
    
    PIN_FLIST_TO_STR(payload_flistp, &flistbuf, &flistlen, ebufp );
    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        *error_codep = MSO_BB_INTERNAL_ERROR;
        goto cleanup;
    }

    /*
     * Create flist for order creation
     */

    order_i_flistp = PIN_FLIST_CREATE(ebufp);
    objp = PIN_POID_CREATE(db_no, "/mso_prov_bb_order", -1, ebufp);
    PIN_FLIST_FLD_PUT(order_i_flistp, PIN_FLD_POID, objp, ebufp);
	//Pawan:27-02-2015: Added program name in input flist of create obj.
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, order_i_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
    if ( *action_flag ==  CATV_PREACTIVATION )
    {
        //PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_OBJ, order_i_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
        PIN_FLIST_FLD_COPY(payload_flistp, PIN_FLD_ACCOUNT_OBJ, order_i_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, order_i_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
    }
    else
    {
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, order_i_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SERVICE_OBJ, order_i_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
    }

    PIN_FLIST_FLD_COPY(order_seq_o_flistp, PIN_FLD_STRING, order_i_flistp, PIN_FLD_ORDER_ID, ebufp);
    PIN_FLIST_FLD_SET(order_i_flistp, PIN_FLD_STATUS, &status, ebufp); //PROVISIONING
    PIN_FLIST_FLD_SET(order_i_flistp, PIN_FLD_ACTION, act, ebufp);
    //pri = pbo_decimal_from_str("1.0", ebufp);
    PIN_FLIST_FLD_SET(order_i_flistp, PIN_FLD_RANK, (void *)&pri, ebufp);

    if(initial_amt != NULL)
    {
	//300 GB Cap Changes Start
	if (is_capped == 1)
	{
        	PIN_FLIST_FLD_SET(order_i_flistp, MSO_FLD_INITIALAMOUNT, cap_quota, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(order_i_flistp, MSO_FLD_INITIALAMOUNT, initial_amt, ebufp); 
	}
	//300 GB Cap Changes End
    }

    /*
     * Convert flist to xml. 
     */
//	PIN_FLIST_TO_XML( payload_flistp, PIN_XML_BY_NAME, 0,
//			&flistbuf, &flistlen, "flist", ebufp );

    nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ));
    if ( nota_buf == NULL ) 
    {
        *error_codep = 52111;
        pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
        PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate nemory for nota_buf", ebufp ); 
        goto cleanup;
    }   

    nota_buf->flag   = 0;
    nota_buf->size   = flistlen ;
    nota_buf->offset = 0;
    nota_buf->data   = ( caddr_t)flistbuf;
    nota_buf->xbuf_file = ( char *) NULL;

    PIN_FLIST_FLD_PUT( order_i_flistp, PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
    PIN_FLIST_FLD_SET(order_i_flistp, MSO_FLD_TECHNOLOGY, tech, ebufp);
   // PIN_FLIST_FLD_SET( order_i_flistp, PIN_FLD_INPUT_FLIST, flistbuf, ebufp );

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_prov_create_bb_action create input flist", order_i_flistp);
    PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, order_i_flistp, out_flistpp, ebufp);
    PIN_FLIST_FLD_COPY(order_i_flistp, PIN_FLD_ORDER_ID, *out_flistpp, PIN_FLD_ORDER_ID, ebufp);
     PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_prov_create_bb_action order create output flist", *out_flistpp);

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        *error_codep = MSO_BB_INTERNAL_ERROR;
        goto cleanup;
    }

//    if (act)
//    {
//        free(act);
//    }
	
 //
cleanup:
    PIN_FLIST_DESTROY_EX(&payload_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&order_i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&order_seq_i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&order_seq_o_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&read_mso_plan_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&read_mso_plan_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&rfld_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&rfld_oflistp, NULL);

	if(!pbo_decimal_is_null(initial_amt_dec, ebufp))
		pbo_decimal_destroy(&initial_amt_dec);

	if(!pbo_decimal_is_null(cap_quota_dec, ebufp))
		pbo_decimal_destroy(&cap_quota_dec);
	
	if (act)
	{
		free(act);
	}
	return;
}

void
set_error_descr(
    pin_flist_t             **ret_flistpp,
    int                     error_code,
    pin_errbuf_t            *ebufp)
{
    char            code[100] = {'\0'};


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "set_error_descr input flist", *ret_flistpp);

    sprintf(code,"%d",error_code);
    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, code, ebufp);

    if (error_code == MSO_BB_MANDATORY_FIELD_CODE)
    {
	    strcpy(code,MSO_BB_MANDATORY_FIELD_DESCR);
    }
    else if (error_code == MSO_BB_INVALID_ACTION_CODE)
    {
        strcpy(code,MSO_BB_INVALID_ACTION_DESCR);
    }
    else if (error_code == MSO_BB_PLAN_OBJ_MISSING)
    {
        strcpy(code,MSO_BB_PLAN_OBJ_MISSING_ERROR);
    }
    else if (error_code == MSO_BB_SEQNO_FAIL_CODE)
    {
        strcpy(code, MSO_BB_SEQNO_FAIL_DESCR);
    }
    else if (error_code == MSO_BB_INTERNAL_ERROR)
    {
        strcpy(code,MSO_BB_INTERNAL_ERROR_DESCR);
    }
    else if (error_code == MSO_BB_LOGIN_MISSING)
    {
        strcpy(code, MSO_BB_LOGIN_MISSING_DESCR);
    }
    else if (error_code == MSO_BB_TECH_MISSING)
    {
        strcpy(code, MSO_BB_TECH_MISSING_DESCR);
    }
    else if (error_code == MSO_BB_NE_ELEMENT_MISSING)
    {
        strcpy(code, MSO_BB_NE_ELEMENT_MISSING_DESCR);
    }
    else if (error_code == MSO_NO_PROV_ACTION)
    {
	strcpy(code, MSO_NO_PROV_ACTION_DESCR);
    }
    else if (error_code == MSO_BB_TAL_FLAG_MISSING)
    {
        strcpy(code, MSO_BB_TAL_FLAG_MISSING_DESCR);
    }
    else if (error_code == MSO_BB_1CLICK_FLAG_MISSING)
    {
        strcpy(code, MSO_BB_1CLICK_FLAG_MISSING_DESCR);
    }
    else if (error_code == MSO_BB_INDICATOR_MISSING)
    {
        strcpy(code, MSO_BB_INDICATOR_MISSING_DESCR);
    }
    else if (error_code == MSO_BB_NAME_MISSING)
    {
        strcpy(code, MSO_BB_NAME_MISSING_DESCR);
    }
    else if (error_code == MSO_BB_PHONE_MISSING)
    {
        strcpy(code, MSO_BB_PHONE_MISSING_DESCR);
    }
    else if (error_code == MSO_BB_EMAIL_MISSING)
    {
        strcpy(code, MSO_BB_EMAIL_MISSING_DESCR);
    }
    else if (error_code == MSO_BB_NW_ID_MISSING)
    {
        strcpy(code, MSO_BB_NW_ID_MISSING_DESCR);
    }
    else if (error_code == MSO_BB_QUOTA_ERROR)
    {
        strcpy(code, MSO_BB_QUOTA_ERROR_DESCR);
    }
    else if (error_code == MSO_BB_OLD_PLAN_ERROR)
    {
        strcpy(code, MSO_BB_OLD_PLAN_ERROR_DESCR);
    }
    else if (error_code == MSO_BB_CONFIG_PT_ERROR)
    {
        strcpy(code, MSO_BB_CONFIG_PT_ERROR_DESCR);
    }
    else if (error_code == MSO_BB_ACCOUNTNO_ERROR)
    {
        strcpy(code, MSO_BB_ACCOUNTNO_ERROR_DESCR);
    }
    else if (error_code == MSO_BB_FIRSTNAME_ERROR)
    {
        strcpy(code, MSO_BB_FIRSTNAME_ERROR_DESCR);
    }
    else if (error_code == MSO_BB_LASTNAME_ERROR)
    {
        strcpy(code, MSO_BB_LASTNAME_ERROR_DESCR);
    }
    else if (error_code == MSO_BB_CLIENT_CLASS_ERROR)
    {
        strcpy(code, MSO_BB_CLIENT_CLASS_ERROR_DESCR);
    }
    else if (error_code == MSO_BB_PROV_TAG_ERROR)
    {
        strcpy(code, MSO_BB_PROV_TAG_ERROR_DESCR);
    }
    else if (error_code == MSO_BB_OLD_LOGIN_MISSING)
    {
        strcpy(code, MSO_BB_OLD_LOGIN_MISSING_DESCR);
    }
    else if (error_code == MSO_BB_SAME_LOGIN_ERROR)
    {
        strcpy(code, MSO_BB_SAME_LOGIN_ERROR_DESCR);
    }
    else if (error_code == MSO_BB_SAME_IP_ERROR)
    {
        strcpy(code, MSO_BB_SAME_IP_ERROR_DESCR);
    }
    else if (error_code == MSO_BB_OLDPASS_ERROR)
    {
        strcpy(code, MSO_BB_OLDPASS_ERROR_DESCR);
    }
    else if (error_code == MSO_BB_SAME_PASS_ERROR)
    {
        strcpy(code, MSO_BB_SAME_PASS_ERROR_DESCR);
    }
    else if (error_code == MSO_BB_CMTS_MASTER_ERROR)
    {
        strcpy(code, MSO_BB_CMTS_MASTER_ERROR_DESCR);
    }
    else if (error_code == MSO_BB_CMTS_DEVICE_CLASS_ERROR)
    {
        strcpy(code, MSO_BB_CMTS_DEVICE_CLASS_ERROR_DESCR);
    }
    else if (error_code == MSO_BB_SPEED_ERROR)
    {
        strcpy(code, MSO_BB_SPEED_ERROR_DESCR);
    }
    else if (error_code == MSO_BB_ULSPEED_ERROR)
    {
        strcpy(code, MSO_BB_ULSPEED_ERROR_DESCR);
    }
    else if (error_code == MSO_BB_DLSPEED_ERROR)
    {
        strcpy(code, MSO_BB_DLSPEED_ERROR_DESCR);
    }
    else if (error_code == MSO_BB_TAL_ERROR)
    {
        strcpy(code, MSO_BB_TAL_ERROR_DESCR);
    }
    else if (error_code == MSO_BB_COMPANY_MISSING)
    {
        strcpy(code, MSO_BB_COMPANY_MISSING_DESCR);
    }
    else if (error_code == MSO_BB_ADDRESS_MISSING)
    {
        strcpy(code, MSO_BB_ADDRESS_MISSING_DESCR);
    }
    else if (error_code == MSO_BB_IP_ADD_ERROR)
    {
        strcpy(code, MSO_BB_IP_ADD_ERROR_DESCR);
    }
    else if (error_code == MSO_BB_MAC_ADD_ERROR)
    {
        strcpy(code, MSO_BB_MAC_ADD_ERROR_DESCR);
    }
    else if (error_code == MSO_BB_ID_ERROR)
    {
        strcpy(code, MSO_BB_ID_ERROR_DESCR);
    }
    else if (error_code == MSO_BB_CITY_MISSING)
    {
        strcpy(code, MSO_BB_CITY_MISSING_DESCR);
    }
    else if (error_code == MSO_BB_AGG_MISSING)
    {
        strcpy(code, MSO_BB_AGG_MISSING_DESCR);
    }
    else if (error_code == MSO_BB_CC_ID_ERROR)
    {
        strcpy(code, MSO_BB_CC_ID_ERROR_DESCR);
    }
    else if (error_code == MSO_BB_CC_CONFIG_ERROR)
    {
        strcpy(code, MSO_BB_CC_CONFIG_ERROR_DESCR);
    }
    else if (error_code == MSO_BB_TAL_AC_ERROR)
    {
        strcpy(code, MSO_BB_TAL_AC_ERROR_DESCR);
    }
    else if (error_code == MSO_BB_NTAL_ERROR)
    {
        strcpy(code, MSO_BB_NTAL_ERROR_DESCR);
    }
    else if (error_code == MSO_BB_INV_TECH)
    {
        strcpy(code, MSO_BB_INV_TECH_DESCR);
    }
    else if (error_code == MSO_BB_IP_DEVICE_ERROR)
    {
        strcpy(code, MSO_BB_IP_DEVICE_ERROR_DESCR);
    }
	/**** Pavan Bellala 29-07-2015 ***********************************
	 Defect 1136 : Validate device string pattern before provisioning
	*****************************************************************/
    else if (error_code == MSO_BB_INVALID_MAC)
    {
        strcpy(code, MSO_BB_INVALID_MAC_DESCR);
    }
	/**** Pavan Bellala 17-08-2015 ***********************************
	 Reset device error
	*****************************************************************/
    else if (error_code == MSO_BB_RESET_DEVICE_ERROR)
    {
        strcpy(code, MSO_BB_RESET_DEVICE_ERROR_DESCR);
    } 	
	/**** Pavan Bellala 24-11-2015 ***********************************
	 HOLD -UNHOLD error
	*****************************************************************/
    else if (error_code == MSO_BB_CONFIG_PT_TECH_ERROR)
    {
        strcpy(code, MSO_BB_CONFIG_PT_TECH_ERROR_DESCR);
    } 	
    else if (error_code == MSO_BB_NO_MODEM_FOUND)
    {
        strcpy(code, MSO_BB_NO_MODEM_FOUND_ERROR_DESCR);
    } 	


    PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, (void *)code, ebufp);
    return;
}

/*******************************************************************
 * prepare_bb_activation_flist()
 *
 * Enrich input filst with service details required
 * optionally performa validation on input fields
 *******************************************************************/
static void
prepare_bb_activation_flist(
	pcm_context_t			*ctxp,
	pin_flist_t			**i_flistpp,
	char				**act,
	int32				*error_codep,
	pin_errbuf_t			*ebufp)
{

	pin_flist_t			*si_flistp = NULL;
	pin_flist_t			*so_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*sub_flistp = NULL;
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*cred_flistp = NULL;
	pin_flist_t			*subscriber = NULL;
	
	
	pin_flist_t			*mso_avp = NULL;
	pin_flist_t			*bb_info = NULL;

	poid_t				*ac_pdp = NULL;
	void				*vp = NULL;
	char				*population_id = "0001"; //this is the only expected value in downstream system
	char				*ne = NULL;
	char				*task = "ACTIVATE_SUBSCRIBER_NSURE";
	
	
	char				*user_id = NULL;
	char				*passwd = NULL;
	char				*user_pass = "Username/Password";
	char				*ip_add = "IPAddress";
	char				*mac_add = "MACAddress";
	char				*serv_stat = "True";
	char 				*vc_id = NULL;
	char				*device_type = NULL;
	char				*acc_no = NULL;
	char				*node_name = NULL;
	char				*serv_city = NULL;

	int32				*state_id = NULL;
	int				    *tech = NULL;
	int				    *isTAL = 0;
	int				    *is1Click = 0;
	int				    *ind = 0;
	int				    *idle_timeout = 0;
	int				    *session_timeout = 0;
    
    char                *str_role = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);


	/***********************************************************
	* Log input flist
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_activation_flist input flist", *i_flistpp);

	/*
	* Validate input received
	*/

	/*
	* Enrich with additional service details 
	*/
	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_activation_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "prepare_bb_activation_flist: Error", ebufp);
		*error_codep = MSO_BB_INTERNAL_ERROR;
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_activation_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp);
    str_role = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_ROLES, 1, ebufp);
    if( (strcmp(other_ne_id,HW_NETWORK_STER_ID) == 0))
    {
        PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_ROLES, str_role, ebufp);
    }
    if(*tech == 1 || *tech == 2 || *tech == 5 || *tech == 3 || *tech == 4)
	{
		if ( *tech == 4 ){
			*act = (char *) malloc(strlen("ACTIVATE_SUBSCRIBER_FIBER")+1);
			strcpy(*act,"ACTIVATE_SUBSCRIBER_FIBER");
		}
		else{
			*act = (char *) malloc(strlen("ACTIVATE_SUBSCRIBER_DOCSIS")+1);
			strcpy(*act,"ACTIVATE_SUBSCRIBER_DOCSIS");
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		if (*tech == 1 || *tech == 2 ){
			prepare_docsis_bcc_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
					"prepare_bb_activation_flist: Error", ebufp);
				goto cleanup;
			}
		}
		prepare_docsis_qps_activation_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
				"prepare_bb_activation_flist: Error", ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"input flist", *i_flistpp);
	}
	/*else
	{
		*act = (char *) malloc(strlen("ACTIVATE_SUBSCRIBER_ETHERNET")+1);
		strcpy(*act,"ACTIVATE_SUBSCRIBER_ETHERNET");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		ethernet_details_flist(ctxp, i_flistpp, so_flistp, task, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
				"prepare_bb_activation_flist: Error", ebufp);
			goto cleanup;
		}
	}*/
	else
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}

    if( (strcmp(other_ne_id,HW_NETWORK_STER_ID) == 0))
    {
           PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_ROLES, str_role, ebufp);
    }
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_flist: Error",ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_flist enriched input flist", *i_flistpp);

	cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	return;
}

static void
prepare_bb_update_cap_flist(
	pcm_context_t			*ctxp,
	pin_flist_t			**i_flistpp,
	char				**act,
	int32				*error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*si_flistp = NULL;
	pin_flist_t			*so_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*sub_flistp = NULL;
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*cred_flistp = NULL;
	pin_flist_t			*subscriber = NULL;
	
	
	pin_flist_t			*mso_avp = NULL;
	pin_flist_t			*bb_info = NULL;

	poid_t				*ac_pdp = NULL;
	void				*vp = NULL;
	char				*population_id = "0001"; //this is the only expected value in downstream system
	char				*ne = NULL;
	char				*user_id = NULL;
	char				*passwd = NULL;
	char				*user_pass = "Username/Password";
	char				*ip_add = "IPAddress";
	char				*mac_add = "MACAddress";
	char				*serv_stat = "True";
	char 				*vc_id = NULL;
	char				*device_type = NULL;
	char				*acc_no = NULL;
	char				*node_name = NULL;
	char				*serv_city = NULL;

	int32				*state_id = NULL;
	int				    *tech = NULL;
	int				    *isTAL = 0;
	int				    *is1Click = 0;
	int				    *ind = 0;
	int				    *idle_timeout = 0;
    int				    *session_timeout = 0;
	
	
	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);


	/***********************************************************
	* Log input flist
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_update_cap_flist input flist", *i_flistpp);

	/*
	* Validate input received
	*/

	/*
	* Enrich with additional service details 
	*/
	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_update_cap_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_update_cap_flist: Error",ebufp);
		*error_codep = MSO_BB_INTERNAL_ERROR;
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_update_cap_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp); 
	//ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 
	//PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);
	
	if (*tech == 4){
		*act = (char *) malloc(strlen("UPDATE_SUBSCRIBER-CAPPING_FIBER")+1);
		strcpy(*act,"UPDATE_SUBSCRIBER-CAPPING_FIBER");
	}
	else{
		*act = (char *) malloc(strlen("UPDATE_SUBSCRIBER-CAPPING_DOCSIS")+1);
		strcpy(*act,"UPDATE_SUBSCRIBER-CAPPING_DOCSIS");
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
	if(tech && (*tech == 1 || *tech == 2 || *tech == 5 || *tech == 3 || *tech == 4))
	{
		if (*tech == 1 || *tech == 2)
			prepare_docsis_bcc_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
		//prepare_switch_service_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_update_cap_flist: prepare_bb_flist: Error",ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_bb_update_cap_flist: input flist", *i_flistpp);
	
		if(*fup_capp_flag == 1)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"fup loop");	
			prepare_switch_service_flist(ctxp, i_flistpp, so_flistp,*act, error_codep, ebufp);
		}
	}
	else
	{
		*error_codep = MSO_NO_PROV_ACTION;
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
		goto cleanup;
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_update_cap_flist: prepare_bb_flist: Error",ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_update_cap_flist: enriched input flist", *i_flistpp);

cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	
	return;
}

static void    
prepare_bb_fup_reversal_flist(
	pcm_context_t			*ctxp,
	pin_flist_t			**i_flistpp,
	char				**act,
	int				*error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*si_flistp = NULL;
	pin_flist_t			*so_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*sub_flistp = NULL;
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*cred_flistp = NULL;
	pin_flist_t			*subscriber = NULL;
	
	
	pin_flist_t			*mso_avp = NULL;
	pin_flist_t			*bb_info = NULL;

	poid_t				*ac_pdp = NULL;
	void				*vp = NULL;
	char				*population_id = "0001"; //this is the only expected value in downstream system
	char				*ne = NULL;
	char				*user_id = NULL;
	char				*passwd = NULL;
	char				*user_pass = "Username/Password";
	char				*ip_add = "IPAddress";
	char				*mac_add = "MACAddress";
	char				*serv_stat = "True";
	char 				*vc_id = NULL;
	char				*device_type = NULL;
	char				*acc_no = NULL;
	char				*node_name = NULL;
	char				*serv_city = NULL;

	int32				*state_id = NULL;
	int				    *tech = NULL;
	int				    *isTAL = 0;
	int				    *is1Click = 0;
	int				    *ind = 0;
	int				    *idle_timeout = 0;
	int				    *session_timeout = 0;
	
	
	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);


	/***********************************************************
	* Log input flist
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_fup_reversal_flist: input flist", *i_flistpp);

	/*
	* Validate input received
	*/

	/*
	* Enrich with additional service details 
	*/
	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_fup_reversal_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_fup_reversal_flist: Error",ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_fup_reversal_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 1, ebufp);
	if(tmp_flistp)
		tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, 
			MSO_FLD_TECHNOLOGY, 0, ebufp); 
	//ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 
	//PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_TECH_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_fup_reversal_flist: Error",ebufp);
		goto cleanup;
	}
	
	if(*tech == 1 || *tech == 2 || *tech == 5 || *tech == 3)
	{      
		/*RAVI- Changes made for sending correct action flag for FUP reversal*/
		*act = (char *) malloc(strlen("UPDATE_SUBSCRIBER-FUP_DOCSIS")+1);
		strcpy(*act,"UPDATE_SUBSCRIBER-FUP_DOCSIS");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		/***** Added for FUP reversal *******************/
		if (*tech == 1 || *tech == 2 )
			prepare_docsis_bcc_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
		/***** Added for FUP reversal *******************/

		prepare_switch_service_flist(ctxp, i_flistpp, so_flistp,*act, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_fup_reversal_flist: Error",ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_bb_fup_reversal_flist: input flist", *i_flistpp);
	}
	else if(*tech == 4){
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "For fiber, no FUP Reversal prov");
	}
	/*else
	{
		*act = (char *) malloc(strlen("UPDATE_SUBSCRIBER-FUP_ETHERNET")+1);
		strcpy(*act,"UPDATE_SUBSCRIBER-FUP_ETHERNET");
		ethernet_new_plan(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_fup_reversal_flist: Error",ebufp);
			goto cleanup;
		}
	}*/
	else
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, MSO_FLD_TECHNOLOGY, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_fup_reversal_flist: Error",ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_fup_reversal_flist enriched input flist", *i_flistpp);
cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	return;
}

static void    
prepare_bb_deactivate_pkg_exp_flist(
	pcm_context_t			*ctxp,
	pin_flist_t			    **i_flistpp,
	char				    **act,
	int				        *error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*si_flistp = NULL;
	pin_flist_t			*so_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*sub_flistp = NULL;
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*cred_flistp = NULL;
	pin_flist_t			*subscriber = NULL;
	pin_flist_t			*grp_detail_flistp = NULL;
	pin_flist_t			*mso_avp = NULL;
	pin_flist_t			*bb_info = NULL;
	pin_flist_t			*plan_iflistp = NULL;
	pin_flist_t			*plan_oflistp = NULL;
	poid_t				*ac_pdp = NULL;
	poid_t				*plan_pdp = NULL;
	void				*vp = NULL;
	char				*population_id = "0001"; //this is the only expected value in downstream system
	char				*ne = NULL;
	char				*plan_name = NULL;
	char				*serv_stat = "True";
	char 				*vc_id = NULL;
	char				*device_type = NULL;
	char				*acc_no = NULL;
	char				*node_name = NULL;
	char				*serv_city = NULL;
	char				*ne_add = NULL;
	char				*city = NULL;
	int32				*state_id = NULL;
	int				    *tech = NULL;
	
	
	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);


	/***********************************************************
	* Log input flist
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_deactivate_pkg_exp_flist input flist", *i_flistpp);

	/*
	* Validate input received
	*/

	/*
	* Enrich with additional service details 
	*/
	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_deactivate_pkg_exp_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_deactivate_pkg_exp_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	node_name = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NE_ELEMENT_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_deactivate_pkg_exp_flist: Network Element Missing",ebufp);
		goto cleanup;
	}
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp); 
	//ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 
	//PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

	
	if(tech && (*tech == 1 || *tech == 2 || *tech == 5 || *tech == 3 || *tech == 4))
	{
		if (*tech == 4){
			*act = (char *) malloc(strlen("DEACTIVATE_SUBSCRIBER_FIBER")+1);
			strcpy(*act,"DEACTIVATE_SUBSCRIBER_FIBER");
		}
		else{
			*act = (char *) malloc(strlen("DEACTIVATE_SUBSCRIBER_DOCSIS")+1);
			strcpy(*act,"DEACTIVATE_SUBSCRIBER_DOCSIS");
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		if (*tech == 1 || *tech == 2){
			prepare_docsis_bcc_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"prepare_bb_deactivate_pkg_exp_flist: Error",ebufp);
				goto cleanup;
			}
		}
		prepare_docsis_subscriberavps_flist(ctxp, i_flistpp, so_flistp, (char *)*act, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_deactivate_pkg_exp_flist: Error",ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"prepare_bb_deactivate_pkg_exp_flist: input flist", *i_flistpp);
	}
	/*else
	{
		*act = (char *) malloc(strlen("DEACTIVATE_SUBSCRIBER_ETHERNET")+1);
		strcpy(*act,"DEACTIVATE_SUBSCRIBER_ETHERNET");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "SUSPEND_SUBSCRIBER_NSURE", ebufp);	
		PIN_FLIST_FLD_COPY(so_flistp, PIN_FLD_LOGIN, task_flistp, PIN_FLD_LOGIN, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			*error_codep = MSO_BB_LOGIN_MISSING;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_deactivate_pkg_exp_flist: Error",ebufp);
			goto cleanup;
		}
		grp_detail_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_GROUP_DETAIL, 1, ebufp);
		plan_pdp = (poid_t *)PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_PLAN_OBJ, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			*error_codep = MSO_BB_PLAN_OBJ_MISSING;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_deactivate_pkg_exp_flist: Error",ebufp);
			goto cleanup;
		}
		plan_iflistp = PIN_FLIST_CREATE(ebufp);
		
		PIN_FLIST_FLD_SET(plan_iflistp, PIN_FLD_POID, plan_pdp, ebufp);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_bb_deactivate_pkg_exp_flist:Read input flist", plan_iflistp);
		PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, plan_iflistp, &plan_oflistp, ebufp);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_bb_deactivate_pkg_exp_flist:Read output flist", plan_oflistp);
		
		plan_name = (char *)PIN_FLIST_FLD_GET(plan_oflistp, PIN_FLD_NAME, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			*error_codep = MSO_BB_PLAN_OBJ_MISSING;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_deactivate_pkg_exp_flist: Error",ebufp);
			goto cleanup;
		}
	
		//Call the function to get the NE Address
		//fm_mso_get_network_node(ctxp, node_name, &ne_add, 2, error_codep, ebufp);
		city = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			*error_codep = MSO_BB_CITY_MISSING;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_switch_service_flist: Network Element Missing",ebufp);
			goto cleanup;
		}
		//Call the function to get the NE Address
		fm_mso_get_network_node(ctxp, node_name, city, &ne_add, 2, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_docsis_bcc_flist: Not able to get Network Element ID",ebufp);
			goto cleanup;
		}
		if(ne_add){
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, (void *)ne_add, ebufp);
		}
		PIN_FLIST_FLD_SET(grp_detail_flistp, MSO_FLD_GROUP, plan_name, ebufp);
		PIN_FLIST_FLD_SET(grp_detail_flistp, MSO_FLD_STATUS, "DEACTIVATE", ebufp);
		
	}*/
	else
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, MSO_FLD_TECHNOLOGY, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}	
		
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_deactivate_pkg_exp_flist: Error",ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_deactivate_pkg_exp_flist: Payload flist", *i_flistpp);
	cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&plan_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&plan_oflistp, NULL);
	return;
}

static void    
prepare_bb_deactivate_pquota_exp_flist(
	pcm_context_t		*ctxp,
	pin_flist_t			**i_flistpp,
	char				**act,
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*si_flistp = NULL;
	pin_flist_t			*so_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*sub_flistp = NULL;
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*cred_flistp = NULL;
	pin_flist_t			*subscriber = NULL;
	pin_flist_t			*grp_detail_flistp = NULL;
	pin_flist_t			*plan_iflistp = NULL;
	pin_flist_t			*plan_oflistp = NULL;
	pin_flist_t			*mso_avp = NULL;
	pin_flist_t			*bb_info = NULL;
	poid_t				*ac_pdp = NULL;
	poid_t				*plan_pdp = NULL;
	void				*vp = NULL;
	char				*population_id = "0001"; //this is the only expected value in downstream system
	char				*ne = NULL;
	char				*plan_name = NULL;
	char				*serv_stat = "True";
	char 				*vc_id = NULL;
	char				*device_type = NULL;
	char				*acc_no = NULL;
	char				*node_name = NULL;
	char				*serv_city = NULL;
	char				*ne_add = NULL;
	char				*city = NULL;
	int32				*state_id = NULL;
	int				    *tech = NULL;
	
	
	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);


	/***********************************************************
	* Log input flist
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_deactivate_pquota_exp_flist input flist", *i_flistpp);

	/*
	* Validate input received
	*/

	/*
	* Enrich with additional service details 
	*/
	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_deactivate_pquota_exp_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_deactivate_pquota_exp_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 1, ebufp);
	node_name = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NE_ELEMENT_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_deactivate_pkg_exp_flist: Network Element Missing",ebufp);
		goto cleanup;
	}
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp); 
	
	//ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 
	//PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_deactivate_pquota_exp_flist: Error",ebufp);
		goto cleanup;
	}
	
	if(*tech == 1 || *tech == 2 || *tech == 5 || *tech == 3)
	{
		*act = (char *) malloc(strlen("DEACTIVATE_SUBSCRIBER_DOCSIS")+1);
		strcpy(*act,"DEACTIVATE_SUBSCRIBER_DOCSIS");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		if (*tech == 1 || *tech == 2){
			prepare_docsis_bcc_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{			
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"prepare_bb_deactivate_pquota_exp_flist: Error",ebufp);
				goto cleanup;
			}
		}
		prepare_docsis_subscriberavps_flist(ctxp, i_flistpp, so_flistp, (char *) *act,error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{			
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_deactivate_pquota_exp_flist: Error",ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"input flist", *i_flistpp);
	}
	/*else
	{
		*act = (char *) malloc(strlen("DEACTIVATE_SUBSCRIBER_ETHERNET")+1);
		strcpy(*act,"DEACTIVATE_SUBSCRIBER_ETHERNET");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "SUSPEND_SUBSCRIBER_NSURE", ebufp);	
		PIN_FLIST_FLD_COPY(so_flistp, PIN_FLD_LOGIN, task_flistp, PIN_FLD_LOGIN, ebufp);
		
		grp_detail_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_GROUP_DETAIL, 1, ebufp);
		plan_pdp = (poid_t *)PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_PLAN_OBJ, 0, ebufp);
	
		plan_iflistp = PIN_FLIST_CREATE(ebufp);
		
		PIN_FLIST_FLD_SET(plan_iflistp, PIN_FLD_POID, plan_pdp, ebufp);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_bb_deactivate_pquota_exp_flist Read input flist", plan_iflistp);
		PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, plan_iflistp, &plan_oflistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_bb_deactivate_pquota_exp_flist Read output flist", plan_oflistp);
		
		plan_name = (char *)PIN_FLIST_FLD_GET(plan_oflistp, PIN_FLD_NAME, 0, ebufp);
		//Call the function to get the NE Address
		//fm_mso_get_network_node(ctxp, node_name, &ne_add, 2, error_codep, ebufp);
		city = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			*error_codep = MSO_BB_CITY_MISSING;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_deactivate_pquota_exp_flist: Network Element Missing",ebufp);
			goto cleanup;
		}
		//Call the function to get the NE Address
		fm_mso_get_network_node(ctxp, node_name, city, &ne_add, 2, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_deactivate_pquota_exp_flist: Not able to get Network Element ID",ebufp);
			goto cleanup;
		}
		if(ne_add){
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, (void *)ne_add, ebufp);
		}
		PIN_FLIST_FLD_SET(grp_detail_flistp, MSO_FLD_GROUP, plan_name, ebufp);
		PIN_FLIST_FLD_SET(grp_detail_flistp, MSO_FLD_STATUS, "DEACTIVATE", ebufp);
	}*/
	else
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, MSO_FLD_TECHNOLOGY, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_deactivate_pquota_exp_flist: Error",ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_deactivate_pquota_exp_flist enriched input flist", *i_flistpp);

cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&plan_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&plan_oflistp, NULL);
	return;
}


static void    
prepare_bb_suspend_flist(
	pcm_context_t		*ctxp,
	pin_flist_t			**i_flistpp,
	char				**act,
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*si_flistp = NULL;
	pin_flist_t			*so_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*sub_flistp = NULL;
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*cred_flistp = NULL;
	pin_flist_t			*subscriber = NULL;
	
	
	pin_flist_t			*mso_avp = NULL;
	pin_flist_t			*bb_info = NULL;
	pin_flist_t			*grp_detail_flistp = NULL;
	pin_flist_t			*plan_iflistp = NULL;
	pin_flist_t			*plan_oflistp = NULL;
	poid_t				*plan_pdp = NULL;
	char				*plan_name = NULL;
	poid_t				*ac_pdp = NULL;
	void				*vp = NULL;
	char				*population_id = "0001"; //this is the only expected value in downstream system
	char				*ne = NULL;
	
	char				*serv_stat = "True";
	char 				*vc_id = NULL;
	char				*device_type = NULL;
	char				*acc_no = NULL;
	char				*node_name = NULL;
	char				*serv_city = NULL;
	char				*ne_add = NULL;
	char				*city = NULL;
	int32				*state_id = NULL;
	int				    *tech = NULL;
	
	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);


	/***********************************************************
	* Log input flist
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_suspend_flist input flist", *i_flistpp);

	/*
	* Validate input received
	*/

	/*
	* Enrich with additional service details 
	*/
	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_suspend_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_suspend_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	node_name = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NE_ELEMENT_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_suspend_flist: Network Element Missing",ebufp);
		goto cleanup;
	}
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp); 
	//ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 
	//PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

	
	if(tech && (*tech == 1 || *tech == 2 || *tech == 5 || *tech == 3 || *tech == 4))
	{
		if (*tech == 4){
			*act = (char *) malloc(strlen("SUSPEND_SUBSCRIBER_FIBER")+1);
			strcpy(*act,"SUSPEND_SUBSCRIBER_FIBER");
		}
		else{
			*act = (char *) malloc(strlen("SUSPEND_SUBSCRIBER_DOCSIS")+1);
			strcpy(*act,"SUSPEND_SUBSCRIBER_DOCSIS");
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		if (*tech == 1 || *tech == 2){
			prepare_docsis_bcc_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"prepare_bb_suspend_flist: Network Element Missing",ebufp);
				goto cleanup;
			}
		}
		prepare_docsis_subscriberavps_flist(ctxp, i_flistpp, so_flistp,(char *) *act, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_suspend_flist: Network Element Missing",ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_bb_suspend_flist: input flist", *i_flistpp);
	}
	/*else
	{
		*act = (char *) malloc(strlen("SUSPEND_SUBSCRIBER_ETHERNET")+1);
		strcpy(*act,"SUSPEND_SUBSCRIBER_ETHERNET");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "SUSPEND_SUBSCRIBER_NSURE", ebufp);	
		PIN_FLIST_FLD_COPY(so_flistp, PIN_FLD_LOGIN, task_flistp, PIN_FLD_LOGIN, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			*error_codep = MSO_BB_LOGIN_MISSING;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_suspend_flist: Login Missing",ebufp);
			goto cleanup;
		}
		grp_detail_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_GROUP_DETAIL, 1, ebufp);
		plan_pdp = (poid_t *)PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_PLAN_OBJ, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			*error_codep = MSO_BB_PLAN_OBJ_MISSING;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_suspend_flist: Plan OBJ Missing",ebufp);
			goto cleanup;
		}
		plan_iflistp = PIN_FLIST_CREATE(ebufp);
		
		PIN_FLIST_FLD_SET(plan_iflistp, PIN_FLD_POID, plan_pdp, ebufp);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read input flist", plan_iflistp);
		PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, plan_iflistp, &plan_oflistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read output flist", plan_oflistp);
		
		plan_name = (char *)PIN_FLIST_FLD_GET(plan_oflistp, PIN_FLD_NAME, 0, ebufp);
		//Call the function to get the NE Address
		//fm_mso_get_network_node(ctxp, node_name, &ne_add, 2, error_codep, ebufp);
		city = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			*error_codep = MSO_BB_CITY_MISSING;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_switch_service_flist: Network Element Missing",ebufp);
			goto cleanup;
		}
		//Call the function to get the NE Address
		fm_mso_get_network_node(ctxp, node_name, city, &ne_add, 2, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_deactivate_pquota_exp_flist: Not able to get Network Element ID",ebufp);
			goto cleanup;
		}
		if(ne_add){
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, (void *)ne_add, ebufp);
		}
		PIN_FLIST_FLD_SET(grp_detail_flistp, MSO_FLD_GROUP, plan_name, ebufp);
		PIN_FLIST_FLD_SET(grp_detail_flistp, MSO_FLD_STATUS, "DEACTIVATE", ebufp);
	}*/
	else
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, MSO_FLD_TECHNOLOGY, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NE_ELEMENT_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_flist: Error",ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_flist enriched input flist", *i_flistpp);

cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&plan_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&plan_oflistp, NULL);
	return;
}


static void    
prepare_bb_hold_serv_flist(
	pcm_context_t		*ctxp,
	pin_flist_t			**i_flistpp,
	char				**act,
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*si_flistp = NULL;
	pin_flist_t			*so_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*sub_flistp = NULL;
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*cred_flistp = NULL;
	pin_flist_t			*subscriber = NULL;
	pin_flist_t			*grp_detail_flistp = NULL;
	pin_flist_t			*plan_iflistp = NULL;
	pin_flist_t			*plan_oflistp = NULL;
	poid_t				*plan_pdp = NULL;
	char				*plan_name = NULL;
	pin_flist_t			*mso_avp = NULL;
	pin_flist_t			*bb_info = NULL;
	poid_t				*ac_pdp = NULL;
	void				*vp = NULL;
	char				*population_id = "0001"; //this is the only expected value in downstream system
	char				*ne = NULL;
	char				*serv_stat = "True";
	char 				*vc_id = NULL;
	char				*device_type = NULL;
	char				*acc_no = NULL;
	char				*node_name = NULL;
	char				*serv_city = NULL;
	char				*ne_add = NULL;
	char				*city = NULL;
	int32				*state_id = NULL;
	int				    *tech = NULL;
	
	
	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);


	/***********************************************************
	* Log input flist
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_hold_serv_flist input flist", *i_flistpp);

	/*
	* Validate input received
	*/

	/*
	* Enrich with additional service details 
	*/
	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_hold_serv_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_hold_serv_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	node_name = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NE_ELEMENT_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_hold_serv_flist: Network Element Missing",ebufp);
		goto cleanup;
	}
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp); 
	//ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 
	//PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

	
	if(tech && (*tech == 1 || *tech == 2 || *tech == 5 || *tech == 3 || *tech == 4))
	{
		if (*tech == 4){
			*act = (char *) malloc(strlen("HOLD_SUBSCRIBER_FIBER")+1);
			strcpy(*act,"HOLD_SUBSCRIBER_FIBER");
		}
		else{
			*act = (char *) malloc(strlen("HOLD_SUBSCRIBER_DOCISIS")+1);
			strcpy(*act,"HOLD_SUBSCRIBER_DOCISIS");
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		if (*tech == 1 || *tech == 2){
			prepare_docsis_bcc_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"prepare_bb_hold_serv_flist: Error",ebufp);
				goto cleanup;
			}
		}
		prepare_docsis_subscriberavps_flist(ctxp, i_flistpp, so_flistp,(char *) *act, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_hold_serv_flist: Error",ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"input flist", *i_flistpp);
	}
	/*else
	{
		*act = (char *) malloc(strlen("HOLD_SUBSCRIBER_ETHERNET")+1);
		strcpy(*act,"HOLD_SUBSCRIBER_ETHERNET");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "SUSPEND_SUBSCRIBER_NSURE", ebufp);	
		PIN_FLIST_FLD_COPY(so_flistp, PIN_FLD_LOGIN, task_flistp, PIN_FLD_LOGIN, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			*error_codep = MSO_BB_LOGIN_MISSING;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_hold_serv_flist: Login Missing",ebufp);
			goto cleanup;
		}
		grp_detail_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_GROUP_DETAIL, 1, ebufp);
		plan_pdp = (poid_t *)PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_PLAN_OBJ, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			*error_codep = MSO_BB_PLAN_OBJ_MISSING;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_hold_serv_flist: Plan OBJ Missing",ebufp);
			goto cleanup;
		}
		plan_iflistp = PIN_FLIST_CREATE(ebufp);
		
		PIN_FLIST_FLD_SET(plan_iflistp, PIN_FLD_POID, plan_pdp, ebufp);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_bb_hold_serv_flist: Read input flist", plan_iflistp);
		PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, plan_iflistp, &plan_oflistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_bb_hold_serv_flist: Read output flist", plan_oflistp);
		
		plan_name = (char *)PIN_FLIST_FLD_GET(plan_oflistp, PIN_FLD_NAME, 0, ebufp);
		//Call the function to get the NE Address
		//fm_mso_get_network_node(ctxp, node_name, &ne_add, 2, error_codep, ebufp);
		city = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			*error_codep = MSO_BB_CITY_MISSING;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_switch_service_flist: Network Element Missing",ebufp);
			goto cleanup;
		}
		//Call the function to get the NE Address
		fm_mso_get_network_node(ctxp, node_name, city, &ne_add, 2, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_deactivate_pquota_exp_flist: Not able to get Network Element ID",ebufp);
			goto cleanup;
		}
		if(ne_add){
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, (void *)ne_add, ebufp);
		}
		PIN_FLIST_FLD_SET(grp_detail_flistp, MSO_FLD_GROUP, plan_name, ebufp);
		PIN_FLIST_FLD_SET(grp_detail_flistp, MSO_FLD_STATUS, "DEACTIVATE", ebufp);
	}*/
	else
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, MSO_FLD_TECHNOLOGY, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_flist: Error",ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_flist enriched input flist", *i_flistpp);

cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&plan_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&plan_oflistp, NULL);
	return;
}

static void    
prepare_bb_topup_bexpiry_npc_flist(
	pcm_context_t		*ctxp,
	pin_flist_t			**i_flistpp,
	char				**act,
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*si_flistp = NULL;
	pin_flist_t			*so_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*sub_flistp = NULL;
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*cred_flistp = NULL;
	pin_flist_t			*subscriber = NULL;
	pin_flist_t			*grp_detail_flistp = NULL;
	pin_flist_t			*group_flistp = NULL;
	
	
	pin_flist_t			*mso_avp = NULL;
	pin_flist_t			*bb_info = NULL;
	poid_t				*ac_pdp = NULL;
	void				*vp = NULL;
	char				*population_id = "0001"; //this is the only expected value in downstream system
	char				*ne = NULL;
	
	char				*serv_stat = "True";
	char 				*vc_id = NULL;
	char				*device_type = NULL;
	char				*acc_no = NULL;
	char				*node_name = NULL;
	char				*serv_city = NULL;
	char				*ne_add = NULL;
	char				*login = NULL;
	char				*city = NULL;
	int32				*state_id = NULL;
	int				    *tech = NULL;
	int				    *flag = NULL;
	
	char				*mac_id = NULL;	
	poid_t				*svc_pdp = NULL;	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);


	/***********************************************************
	* Log input flist
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_topup_bexpiry_npc_flist input flist", *i_flistpp);

	/*
	* Validate input received
	*/

	/*
	* Enrich with additional service details 
	*/
	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_topup_bexpiry_npc_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_topup_bexpiry_npc_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	node_name = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NE_ELEMENT_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_topup_bexpiry_npc_flist: Network Element Missing",ebufp);
		goto cleanup;
	}
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp); 
	//ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 
	//PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

	
	if(*tech == 1 || *tech == 2 || *tech == 5 || *tech == 3)
	{

		/***** Pavan Bellala 24-11-2015 ******************************
		Sanity check added to ensure that device is available always
		**************************************************************/
		svc_pdp = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_SERVICE_OBJ,0,ebufp);
		if (*tech != 3 ){
			fm_mso_get_mac(ctxp, svc_pdp, &mac_id, error_codep, ebufp);
			if(mac_id == NULL || (mac_id && (strcmp(mac_id,"")==0)))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR , "Error:Invalid MAC id");	
				pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE,PIN_ERR_BAD_VALUE, PIN_FLD_DEVICE_ID, 0, 0);
				*error_codep = MSO_BB_INVALID_MAC;
				goto cleanup;
			}
		}
		*act = (char *) malloc(strlen("TOPUP_WO_PLAN_CHANGE_SUBSCRIBER_DOCSIS")+1);
		strcpy(*act,"TOPUP_WO_PLAN_CHANGE_SUBSCRIBER_DOCSIS");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		if (*tech == 1 || *tech == 2){
			prepare_docsis_bcc_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"prepare_bb_topup_bexpiry_npc_flist: Network Element Missing",ebufp);
				goto cleanup;
			}
		}
		prepare_switch_service_flist(ctxp, i_flistpp, so_flistp,*act, error_codep,ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_topup_bexpiry_npc_flist: Network Element Missing",ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"input flist", *i_flistpp);
	}
	/*else
	{
		*act = (char *) malloc(strlen("TOPUP_WO_PLAN_CHANGE_SUBSCRIBER_ETHERNET")+1);
		strcpy(*act,"TOPUP_WO_PLAN_CHANGE_SUBSCRIBER_ETHERNET");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
		flag = (int *)PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_FLAGS, 0, ebufp);
		if(*flag == ETH_BB_TOPUP_BEXPIRY_LIMITED){
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "RENEW_USER_NSURE", ebufp);
		}
		else{
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "INCREASE_LIMIT_NSURE", ebufp);
		}
		PIN_FLIST_FLD_COPY(so_flistp, PIN_FLD_LOGIN, task_flistp, PIN_FLD_LOGIN, ebufp);
		//Call the function to get the NE Address
		//fm_mso_get_network_node(ctxp, node_name, &ne_add, 2, error_codep, ebufp);
		city = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			*error_codep = MSO_BB_CITY_MISSING;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_switch_service_flist: Network Element Missing",ebufp);
			goto cleanup;
		}
		//Call the function to get the NE Address
		fm_mso_get_network_node(ctxp, node_name, city, &ne_add, 2, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_topup_bexpiry_npc_flist: Not able to get Network Element ID",ebufp);
			goto cleanup;
		}
		if(ne_add){
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, (void *)ne_add, ebufp);
		}
		fm_mso_add_group_detail(ctxp, *i_flistpp, so_flistp, &grp_detail_flistp, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_topup_bexpiry_npc_flist: Not able to get Network Element ID",ebufp);
			goto cleanup;
		}
		group_flistp = PIN_FLIST_ELEM_GET(grp_detail_flistp,MSO_FLD_GROUP_DETAIL,1,0,ebufp);
		PIN_FLIST_FLD_SET(group_flistp, MSO_FLD_METERING_OVERRIDE, "n", ebufp);
		
		PIN_FLIST_ELEM_COPY(grp_detail_flistp, MSO_FLD_GROUP_DETAIL,1,
			 task_flistp, MSO_FLD_GROUP_DETAIL, 1, ebufp);
		 
	}*/
	else
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, MSO_FLD_TECHNOLOGY, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_topup_bexpiry_npc_flist: Error",ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_topup_bexpiry_npc_flist enriched input flist", *i_flistpp);

cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	
	if(mac_id) pin_free(mac_id);
	return;
}

static void    
prepare_bb_topup_bexpiry_pc_flist(
	pcm_context_t		*ctxp,
	pin_flist_t			**i_flistpp,
	char				**act,
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*si_flistp = NULL;
	pin_flist_t			*so_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*sub_flistp = NULL;
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*cred_flistp = NULL;
	pin_flist_t			*subscriber = NULL;
	
	
	pin_flist_t			*mso_avp = NULL;
	pin_flist_t			*bb_info = NULL;

	poid_t				*ac_pdp = NULL;
	void				*vp = NULL;
	char				*population_id = "0001"; //this is the only expected value in downstream system
	char				*ne = NULL;
	
	char				*serv_stat = "True";
	char 				*vc_id = NULL;
	char				*device_type = NULL;
	char				*acc_no = NULL;
	char				*node_name = NULL;
	char				*serv_city = NULL;
	char				*ne_add = NULL;

	int32				*state_id = NULL;
	int				    *tech = NULL;
	
	
	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);


	/***********************************************************
	* Log input flist
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_topup_bexpiry_pc_flist: input flist", *i_flistpp);

	/*
	* Validate input received
	*/

	/*
	* Enrich with additional service details 
	*/
	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_topup_bexpiry_pc_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_topup_bexpiry_pc_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp); 
	//ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 
	//PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

	if(*tech == 1 || *tech == 2 || *tech == 5)
	{
		*act = (char *) malloc(strlen("TOPUP_PLAN_CHANGE_SUBSCRIBER_DOCSIS")+1);
		strcpy(*act,"TOPUP_PLAN_CHANGE_SUBSCRIBER_DOCSIS");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		if(*tech == 1 || *tech == 2){
			prepare_docsis_bcc_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"prepare_bb_topup_bexpiry_pc_flist: Error",ebufp);
				goto cleanup;
			}
		}
		prepare_switch_service_flist(ctxp, i_flistpp, so_flistp,*act, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_topup_bexpiry_pc_flist: Error",ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"input flist", *i_flistpp);
	}
	else
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_topup_bexpiry_pc_flist: Error",ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_topup_bexpiry_pc_flist enriched input flist", *i_flistpp);

cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	return;
}


static void    
prepare_bb_renew_aexpiry_pre_flist(
	pcm_context_t		*ctxp,
	pin_flist_t			**i_flistpp,
	char				**act,
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*si_flistp = NULL;
	pin_flist_t			*so_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*sub_flistp = NULL;
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*cred_flistp = NULL;
	pin_flist_t			*subscriber = NULL;
	
	
	pin_flist_t			*mso_avp = NULL;
	pin_flist_t			*bb_info = NULL;

	poid_t				*ac_pdp = NULL;
	void				*vp = NULL;
	char				*population_id = "0001"; //this is the only expected value in downstream system
	char				*ne = NULL;
	
	char				*serv_stat = "True";
	char 				*vc_id = NULL;
	char				*device_type = NULL;
	char				*acc_no = NULL;
	char				*node_name = NULL;
	char				*serv_city = NULL;

	int32				*state_id = NULL;
	int				    *tech = NULL;
	
	
	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);


	/***********************************************************
	* Log input flist
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_renew_aexpiry_pre_flist input flist", *i_flistpp);

	/*
	* Validate input received
	*/

	/*
	* Enrich with additional service details 
	*/
	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_renew_aexpiry_pre_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_renew_aexpiry_pre_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp); 
	//ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 
	//PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

	
	if(*tech == 1 || *tech == 2 || *tech == 5 || *tech == 3)
	{
		*act = (char *) malloc(strlen("RENEW_SUBSCRIBER_DOCSIS")+1);
		strcpy(*act,"RENEW_SUBSCRIBER_DOCSIS");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		if(*tech == 1 || *tech == 2){
			prepare_docsis_bcc_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"prepare_bb_renew_aexpiry_pre_flist: Error",ebufp);
				goto cleanup;
			}
		}
		prepare_switch_service_flist(ctxp, i_flistpp, so_flistp,*act, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_renew_aexpiry_pre_flist: Error",ebufp);
			goto cleanup;
		}
		prepare_docsis_subscriberavps_flist(ctxp, i_flistpp, so_flistp, (char *)*act ,error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_renew_aexpiry_pre_flist: Error",ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_bb_renew_aexpiry_pre_flist: input flist", *i_flistpp);
	}
	else
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_renew_aexpiry_pre_flist: Error",ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_renew_aexpiry_pre_flist enriched input flist", *i_flistpp);

cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	return;
}


static void    
prepare_bb_auto_renew_post_flist(
	pcm_context_t		*ctxp,
	pin_flist_t			**i_flistpp,
	char				**act,
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*si_flistp = NULL;
	pin_flist_t			*so_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*sub_flistp = NULL;
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*cred_flistp = NULL;
	pin_flist_t			*subscriber = NULL;
	
	
	pin_flist_t			*mso_avp = NULL;
	pin_flist_t			*bb_info = NULL;

	poid_t				*ac_pdp = NULL;
	void				*vp = NULL;
	char				*population_id = "0001"; //this is the only expected value in downstream system
	char				*ne = NULL;
	
	char				*serv_stat = "True";
	char 				*vc_id = NULL;
	char				*device_type = NULL;
	char				*acc_no = NULL;
	char				*node_name = NULL;
	char				*serv_city = NULL;

	int32				*state_id = NULL;
	int				    *tech = NULL;
	
	
	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);


	/***********************************************************
	* Log input flist
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_auto_renew_post_flist input flist", *i_flistpp);

	/*
	* Validate input received
	*/

	/*
	* Enrich with additional service details 
	*/
	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_auto_renew_post_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_auto_renew_post_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp); 
	//ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 
	//PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

	
	if(*tech == 1 || *tech == 2 || *tech == 5)
	{
		*act = (char *) malloc(strlen("AUTORENEW_SUBSCRIBER_DOCSIS")+1);
		strcpy(*act,"AUTORENEW_SUBSCRIBER_DOCSIS");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		if(*tech == 1 || *tech == 2){
			prepare_docsis_bcc_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"prepare_bb_auto_renew_post_flist: Error",ebufp);
				goto cleanup;
			}
		}
		prepare_docsis_subscriberavps_flist(ctxp, i_flistpp, so_flistp, (char *) *act ,error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_auto_renew_post_flist: Error",ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_bb_auto_renew_post_flist input flist", *i_flistpp);
	}
	else
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_auto_renew_post_flist: Error",ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_auto_renew_post_flist enriched input flist", *i_flistpp);

	cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	return;
}


static void    
prepare_bb_reactivate_serv_flist(
	pcm_context_t		*ctxp,
	pin_flist_t			**i_flistpp,
	char				**act,
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*si_flistp = NULL;
	pin_flist_t			*so_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*sub_flistp = NULL;
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*cred_flistp = NULL;
	pin_flist_t			*subscriber = NULL;
	pin_flist_t			*grp_detail_flistp = NULL;
	pin_flist_t			*plan_iflistp = NULL;
	pin_flist_t			*plan_oflistp = NULL;
	poid_t				*plan_pdp = NULL;
	char				*plan_name = NULL;
	
	pin_flist_t			*mso_avp = NULL;
	pin_flist_t			*bb_info = NULL;

	poid_t				*ac_pdp = NULL;
	void				*vp = NULL;
	char				*population_id = "0001"; //this is the only expected value in downstream system
	char				*ne = NULL;
	
	char				*serv_stat = "True";
	char 				*vc_id = NULL;
	char				*device_type = NULL;
	char				*acc_no = NULL;
	char				*node_name = NULL;
	char				*serv_city = NULL;
	char				*ne_add = NULL;
	char				*city = NULL;
	int32				*state_id = NULL;
	int				    *tech = NULL;
	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);


	/***********************************************************
	* Log input flist
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_reactivate_serv_flist input flist", *i_flistpp);

	/*
	* Validate input received
	*/

	/*
	* Enrich with additional service details 
	*/
	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_reactivate_serv_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_reactivate_serv_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	node_name = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NE_ELEMENT_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_reactivate_serv_flist: Network Element Missing",ebufp);
		goto cleanup;
	}
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp); 
	//ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 
	//PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

	
	if(tech && (*tech == 1 || *tech == 2 || *tech == 5 || *tech == 3 || *tech == 4))
	{
		if (*tech == 4){
			*act = (char *) malloc(strlen("REACTIVATE_SERVICES_FIBER")+1);
			strcpy(*act,"REACTIVATE_SERVICES_FIBER");
		}
		else{
			*act = (char *) malloc(strlen("REACTIVATE_SERVICES_DOCSIS")+1);
			strcpy(*act,"REACTIVATE_SERVICES_DOCSIS");
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		if(*tech == 1 || *tech == 2){
			prepare_docsis_bcc_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"prepare_bb_reactivate_serv_flist: Error",ebufp);
				goto cleanup;
			}
		}
		prepare_docsis_subscriberavps_flist(ctxp, i_flistpp, so_flistp, (char *) *act ,error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_reactivate_serv_flist: Error",ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_bb_activation_flist: input flist", *i_flistpp);
	}
	/*else
	{		
		*act = (char *) malloc(strlen("REACTIVATE_SERVICES_ETHERNET")+1);
		strcpy(*act,"REACTIVATE_SERVICES_ETHERNET");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "RENEW_SUBSCRIBER_NSURE", ebufp);	
		PIN_FLIST_FLD_COPY(so_flistp, PIN_FLD_LOGIN, task_flistp, PIN_FLD_LOGIN, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			*error_codep = MSO_BB_LOGIN_MISSING;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_reactivate_serv_flist: Login Missing",ebufp);
			goto cleanup;
		}
		grp_detail_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_GROUP_DETAIL, 1, ebufp);
		plan_pdp = (poid_t *)PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_PLAN_OBJ, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			*error_codep = MSO_BB_PLAN_OBJ_MISSING;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_reactivate_serv_flist: Plan Missing",ebufp);
			goto cleanup;
		}
		plan_iflistp = PIN_FLIST_CREATE(ebufp);
		
		PIN_FLIST_FLD_SET(plan_iflistp, PIN_FLD_POID, plan_pdp, ebufp);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_bb_reactivate_serv_flist: Read input flist", plan_iflistp);
		PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, plan_iflistp, &plan_oflistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_bb_reactivate_serv_flist: Read output flist", plan_oflistp);
		
		plan_name = (char *)PIN_FLIST_FLD_GET(plan_oflistp, PIN_FLD_NAME, 0, ebufp);
		//Call the function to get the NE Address
		//fm_mso_get_network_node(ctxp, node_name, &ne_add, 2, error_codep, ebufp);
		city = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			*error_codep = MSO_BB_CITY_MISSING;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_switch_service_flist: Network Element Missing",ebufp);
			goto cleanup;
		}
		//Call the function to get the NE Address
		fm_mso_get_network_node(ctxp, node_name, city, &ne_add, 2, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_reactivate_serv_flist: Not able to get Network Element ID",ebufp);
			goto cleanup;
		}
		if(ne_add){
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, (void *)ne_add, ebufp);
		}
		PIN_FLIST_FLD_SET(grp_detail_flistp, MSO_FLD_GROUP, plan_name, ebufp);
		PIN_FLIST_FLD_SET(grp_detail_flistp, MSO_FLD_STATUS, "ACTIVATE", ebufp);
	}*/
	else
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, MSO_FLD_TECHNOLOGY, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_reactivate_serv_flist: Error",ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_reactivate_serv_flist enriched input flist", *i_flistpp);

	cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&plan_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&plan_iflistp, NULL);
	return;
}

static void    
prepare_bb_unhold_serv_flist(
	pcm_context_t		*ctxp,
	pin_flist_t			**i_flistpp,
	char				**act,
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*si_flistp = NULL;
	pin_flist_t			*so_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*sub_flistp = NULL;
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*cred_flistp = NULL;
	pin_flist_t			*subscriber = NULL;
	pin_flist_t			*grp_detail_flistp = NULL;
	pin_flist_t			*plan_iflistp = NULL;
	pin_flist_t			*plan_oflistp = NULL;
	poid_t				*plan_pdp = NULL;
	char				*plan_name = NULL;
	pin_flist_t			*mso_avp = NULL;
	pin_flist_t			*bb_info = NULL;
	poid_t				*ac_pdp = NULL;
	void				*vp = NULL;
	char				*population_id = "0001"; //this is the only expected value in downstream system
	char				*ne = NULL;
	char				*serv_stat = "True";
	char 				*vc_id = NULL;
	char				*device_type = NULL;
	char				*acc_no = NULL;
	char				*node_name = NULL;
	char				*serv_city = NULL;
	char				*ne_add = NULL;
	char				*city = NULL;
	int32				*state_id = NULL;
	int				    *tech = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);


	/***********************************************************
	* Log input flist
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_unhold_serv_flist input flist", *i_flistpp);

	/*
	* Validate input received
	*/

	/*
	* Enrich with additional service details 
	*/
	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_unhold_serv_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_unhold_serv_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	node_name = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NE_ELEMENT_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_unhold_serv_flist: Network Element Missing",ebufp);
		goto cleanup;
	}
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp); 
	//ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 
	//PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

	
	if(tech && (*tech == 1 || *tech == 2 || *tech == 5 || *tech == 3 || *tech == 4))
	{
		if (*tech == 4){
			*act = (char *) malloc(strlen("UNHOLD_SERVICES_FIBER")+1);
			strcpy(*act,"UNHOLD_SERVICES_FIBER");
		}
		else{
			*act = (char *) malloc(strlen("UNHOLD_SERVICES_DOCSIS")+1);
			strcpy(*act,"UNHOLD_SERVICES_DOCSIS");
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		if(*tech == 1 || *tech == 2){
			prepare_docsis_bcc_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"prepare_bb_unhold_serv_flist: Error",ebufp);
				goto cleanup;
			}
		}
		prepare_docsis_subscriberavps_flist(ctxp, i_flistpp, so_flistp, (char *) *act ,error_codep, ebufp);
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_unhold_serv_flist: Error",ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"input flist", *i_flistpp);
	}
	/*else
	{
		*act = (char *) malloc(strlen("UNHOLD_SERVICES_ETHERNET")+1);
		strcpy(*act,"UNHOLD_SERVICES_ETHERNET");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, 1, ebufp);
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "RENEW_SUBSCRIBER_NSURE", ebufp);	
		PIN_FLIST_FLD_COPY(so_flistp, PIN_FLD_LOGIN, task_flistp, PIN_FLD_LOGIN, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			*error_codep = MSO_BB_LOGIN_MISSING;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_unhold_serv_flist: Login Missing",ebufp);
			goto cleanup;
		}
		grp_detail_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_GROUP_DETAIL, 1, ebufp);
		plan_pdp = (poid_t *)PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_PLAN_OBJ, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			*error_codep = MSO_BB_PLAN_OBJ_MISSING;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_unhold_serv_flist: Plan Missing",ebufp);
			goto cleanup;
		}
		plan_iflistp = PIN_FLIST_CREATE(ebufp);
		
		PIN_FLIST_FLD_SET(plan_iflistp, PIN_FLD_POID, plan_pdp, ebufp);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read input flist", plan_iflistp);
		PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, plan_iflistp, &plan_oflistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read output flist", plan_oflistp);
		
		plan_name = (char *)PIN_FLIST_FLD_GET(plan_oflistp, PIN_FLD_NAME, 0, ebufp);
		//Call the function to get the NE Address
		//fm_mso_get_network_node(ctxp, node_name, &ne_add, 2, error_codep, ebufp);
		city = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			*error_codep = MSO_BB_CITY_MISSING;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_switch_service_flist: Network Element Missing",ebufp);
			goto cleanup;
		}
		//Call the function to get the NE Address
		fm_mso_get_network_node(ctxp, node_name, city, &ne_add, 2, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_unhold_serv_flist: Not able to get Network Element ID",ebufp);
			goto cleanup;
		}
		if(ne_add){
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, (void *)ne_add, ebufp);
		}
		PIN_FLIST_FLD_SET(grp_detail_flistp, MSO_FLD_GROUP, plan_name, ebufp);
		PIN_FLIST_FLD_SET(grp_detail_flistp, MSO_FLD_STATUS, "ACTIVATE", ebufp);
	}*/
	else
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, MSO_FLD_TECHNOLOGY, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_unhold_serv_flist: Error",ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_unhold_serv_flist enriched input flist", *i_flistpp);

	cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&plan_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&plan_iflistp, NULL);
	return;
}
static void    
prepare_bb_pc_quota_change_flist(
	pcm_context_t	    *ctxp,
	int32				*action_flag,
	pin_flist_t			**i_flistpp,
	char				**act,
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*si_flistp = NULL;
	pin_flist_t			*so_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*sub_flistp = NULL;
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*cred_flistp = NULL;
	pin_flist_t			*subscriber = NULL;
	
	
	pin_flist_t			*mso_avp = NULL;
	pin_flist_t			*bb_info = NULL;

	poid_t				*ac_pdp = NULL;
	void				*vp = NULL;
	char				*population_id = "0001"; //this is the only expected value in downstream system
	char				*ne = NULL;
	char				*serv_stat = "True";
	char 				*vc_id = NULL;
	char				*device_type = NULL;
	char				*acc_no = NULL;
	char				*node_name = NULL;
	char				*serv_city = NULL;
	char				*ne_add = NULL;
	char				*city = NULL;
	int32				*state_id = NULL;
	int				    *tech = NULL;
	pin_flist_t			*nw_flistp = NULL;
	int				    *nw_flag = NULL;
	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);


	/***********************************************************
	* Log input flist
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_pc_quota_change_flist input flist", *i_flistpp);

	nw_flistp = PIN_FLIST_ELEM_TAKE(*i_flistpp,PIN_FLD_IP_ADDRESSES,0,1,ebufp);
	if(nw_flistp!= NULL)
	{
		PIN_FLIST_FLD_SET(nw_flistp,MSO_FLD_TASK_NAME,"CHANGE_CREDENTIAL_QPS",ebufp);	
		nw_flag = PIN_FLIST_FLD_TAKE(nw_flistp,MSO_FLD_ISTAL_FLAG,1,ebufp);
		PIN_FLIST_FLD_SET(nw_flistp,MSO_FLD_NETWORK_NODE,"QPS01",ebufp);
	}


	/*
	* Validate input received
	*/

	/*
	* Enrich with additional service details 
	*/
	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_pc_quota_change_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_pc_quota_change_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	node_name = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NE_ELEMENT_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_pc_quota_change_flist: Network Element Missing",ebufp);
		goto cleanup;
	}
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp); 
	//ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 
	//PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

	
	if(tech && (*tech == 1 || *tech == 2 || *tech == 5 || *tech == 3 || *tech == 4) )
	{
		if (*tech == 4){
			*act = (char *) malloc(strlen("CHANGE_SERVICES_FIBER")+1);
			strcpy(*act,"CHANGE_SERVICES_FIBER");
		}
		else{
			*act = (char *) malloc(strlen("CHANGE_SERVICES_DOCSIS")+1);
			strcpy(*act,"CHANGE_SERVICES_DOCSIS");
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );

		if (*tech == 1 || *tech == 2 ){
			prepare_docsis_bcc_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"prepare_bb_pc_quota_change_flist: Error",ebufp);
				goto cleanup;
			}
		}
		prepare_switch_service_flist(ctxp, i_flistpp, so_flistp,*act, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_pc_quota_change_flist: Error",ebufp);
			goto cleanup;
		}
		
		/***** Pavan Bellala 04-09-2015 ********************
		* Commented below sections as per new design from ASAP
		* UPDATE_AVQ_PNQ would be called only in case of QUOTA_QUOTA
		*  and NQUOTA_NQUOTA
		****************************************************/
		if ( NULL != action_flag && *action_flag ==  DOC_BB_PC_QUOTA_QUOTA || 
					*action_flag ==  DOC_BB_PC_NQUOTA_NQUOTA ) {
			if (*tech != 4){
				prepare_docsis_update_avp_qnq(ctxp, i_flistpp, so_flistp, *act,error_codep, ebufp);
				if(PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						"pt_cm.pinlogepare_bb_pc_quota_change_flist: Error",ebufp);
					goto cleanup;
				}
			}
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"input flist", *i_flistpp);
	}
	/*else if(tech && *tech == 3)
	{
		*act = (char *) malloc(strlen("CHANGE_SERVICES_ETHERNET")+1);
		strcpy(*act,"CHANGE_SERVICES_ETHERNET");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		ethernet_new_plan(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_pc_quota_change_flist: Error",ebufp);
			goto cleanup;
		}
		task_flistp = PIN_FLIST_ELEM_GET(*i_flistpp, MSO_FLD_TASK, 1, 0, ebufp);
		PIN_FLIST_FLD_SET(task_flistp,MSO_FLD_TASK_NAME, "CHANGE_SERVICE_NSURE", ebufp);
		//Call the function to get the NE Address
		//fm_mso_get_network_node(ctxp, node_name, &ne_add, 2, error_codep, ebufp);
		city = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			*error_codep = MSO_BB_CITY_MISSING;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_switch_service_flist: Network Element Missing",ebufp);
			goto cleanup;
		}
		//Call the function to get the NE Address
		fm_mso_get_network_node(ctxp, node_name, city, &ne_add, 2, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_pc_quota_change_flist: Not able to get Network Element ID",ebufp);
			goto cleanup;
		}
		if(ne_add){
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, (void *)ne_add, ebufp);
		}
		
	}*/
	else
	{
                *error_codep = MSO_NO_PROV_ACTION;
		pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
                goto cleanup;
	}
	if(nw_flistp != NULL && (*nw_flag == 1))
	{
		PIN_FLIST_ELEM_SET(*i_flistpp,nw_flistp,MSO_FLD_TASK,3,ebufp);
	}

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_pc_quota_change_flist: Error",ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_pc_quota_change_flist enriched input flist", *i_flistpp);

cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);

	if(nw_flag) free(nw_flag);
	if(nw_flistp) PIN_FLIST_DESTROY_EX(&nw_flistp,NULL);
	return;
}

static void    
prepare_bb_pc_pre_post_flist(
	pcm_context_t		*ctxp,
	pin_flist_t			**i_flistpp,
	char				**act,
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*si_flistp = NULL;
	pin_flist_t			*so_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*sub_flistp = NULL;
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*cred_flistp = NULL;
	pin_flist_t			*subscriber = NULL;
	
	
	pin_flist_t			*mso_avp = NULL;
	pin_flist_t			*bb_info = NULL;

	poid_t				*ac_pdp = NULL;
	void				*vp = NULL;
	char				*population_id = "0001"; //this is the only expected value in downstream system
	char				*ne = NULL;
	
	char				*serv_stat = "True";
	char 				*vc_id = NULL;
	char				*device_type = NULL;
	char				*acc_no = NULL;
	char				*node_name = NULL;
	char				*serv_city = NULL;
	char				*ne_add = NULL;
	int32				*state_id = NULL;
	int				    *tech = NULL;
	char				*city = NULL;
	
	
	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);


	/***********************************************************
	* Log input flist
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_pc_pre_post_flist input flist", *i_flistpp);

	/*
	* Validate input received
	*/

	/*
	* Enrich with additional service details 
	*/
	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_pc_pre_post_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_pc_pre_post_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	node_name = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NE_ELEMENT_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_pc_pre_post_flist: Network Element Missing",ebufp);
		goto cleanup;
	}
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp); 
	//ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 
	//PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

	
	if(*tech == 1 || *tech == 2)
	{
//		*act = (char *) malloc(strlen("CHANGE_SERVICES_DOCSIS")+1);
//		strcpy(*act,"CHANGE_SERVICES_DOCSIS");
//		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
//		prepare_docsis_bcc_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
//		if (PIN_ERRBUF_IS_ERR(ebufp))
//		{
//			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
//				"prepare_bb_pc_pre_post_flist: Error",ebufp);
//			goto cleanup;
//		}
//		prepare_switch_service_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
//		if (PIN_ERRBUF_IS_ERR(ebufp))
//		{
//			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
//				"prepare_bb_pc_pre_post_flist: Error",ebufp);
//			goto cleanup;
//		}
//		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"input flist", *i_flistpp);
		*error_codep = MSO_NO_PROV_ACTION;
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
		goto cleanup;
	}
	/*else
	{
		*act = (char *) malloc(strlen("CHANGE_SERVICES_ETHERNET")+1);
		strcpy(*act,"CHANGE_SERVICES_ETHERNET");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		ethernet_new_plan(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_pc_pre_post_flist: Error",ebufp);
			goto cleanup;
		}
		task_flistp = PIN_FLIST_ELEM_GET(*i_flistpp, MSO_FLD_TASK, 1, 0, ebufp);
		PIN_FLIST_FLD_SET(task_flistp,MSO_FLD_TASK_NAME, "CHANGE_SERVICE_NSURE", ebufp);
		//Call the function to get the NE Address
		//fm_mso_get_network_node(ctxp, node_name, &ne_add, 2, error_codep, ebufp);
		city = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			*error_codep = MSO_BB_CITY_MISSING;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_switch_service_flist: Network Element Missing",ebufp);
			goto cleanup;
		}
		//Call the function to get the NE Address
		fm_mso_get_network_node(ctxp, node_name, city, &ne_add, 2, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_pc_pre_post_flist: Not able to get Network Element ID",ebufp);
			goto cleanup;
		}
		if(ne_add){
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, (void *)ne_add, ebufp);
		}
	}*/
	else
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_pc_pre_post_flist: Error",ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_pc_pre_post_flist enriched input flist", *i_flistpp);

cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	return;
}

static void    
prepare_bb_pc_post_pre_flist(
	pcm_context_t	     *ctxp,
	pin_flist_t			**i_flistpp,
	char				**act,
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*si_flistp = NULL;
	pin_flist_t			*so_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*sub_flistp = NULL;
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*cred_flistp = NULL;
	pin_flist_t			*subscriber = NULL;
	pin_flist_t			*mso_avp = NULL;
	pin_flist_t			*bb_info = NULL;
	poid_t				*ac_pdp = NULL;
	void				*vp = NULL;
	char				*population_id = "0001"; //this is the only expected value in downstream system
	char				*ne = NULL;
	char				*serv_stat = "True";
	char 				*vc_id = NULL;
	char				*device_type = NULL;
	char				*acc_no = NULL;
	char				*node_name = NULL;
	char				*serv_city = NULL;
	char				*ne_add = NULL;
	int32				*state_id = NULL;
	int				    *tech = NULL;
	char				*city = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);


	/***********************************************************
	* Log input flist
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_pc_post_pre_flist input flist", *i_flistpp);

	/*
	* Validate input received
	*/

	/*
	* Enrich with additional service details 
	*/
	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_pc_post_pre_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_pc_post_pre_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	node_name = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NE_ELEMENT_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_pc_post_pre_flist: Network Element Missing",ebufp);
		goto cleanup;
	}
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp); 
	//ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 
	PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

	
	if(*tech == 1 || *tech == 2)
	{
//		*act = (char *) malloc(strlen("CHANGE_SERVICES_DOCSIS")+1);
//		strcpy(*act,"CHANGE_SERVICES_DOCSIS");
//		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
//		prepare_docsis_bcc_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
//		if (PIN_ERRBUF_IS_ERR(ebufp))
//		{
//			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
//				"prepare_bb_pc_post_pre_flist: Error",ebufp);
//			goto cleanup;
//		}
//		prepare_switch_service_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
//		if (PIN_ERRBUF_IS_ERR(ebufp))
//		{
//			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
//				"prepare_bb_pc_post_pre_flist: Error",ebufp);
//			goto cleanup;
//		}
//		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"input flist", *i_flistpp);
		*error_codep = MSO_NO_PROV_ACTION;
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
		goto cleanup;
	}
	/*else
	{
		*act = (char *) malloc(strlen("CHANGE_SERVICES_ETHERNET")+1);
		strcpy(*act,"CHANGE_SERVICES_ETHERNET");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		ethernet_new_plan(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
		task_flistp = PIN_FLIST_ELEM_GET(*i_flistpp, MSO_FLD_TASK, 1, 0, ebufp);
		PIN_FLIST_FLD_SET(task_flistp,MSO_FLD_TASK_NAME, "CHANGE_SERVICE_NSURE", ebufp);
		//Call the function to get the NE Address
		//fm_mso_get_network_node(ctxp, node_name, &ne_add, 2, error_codep, ebufp);
		city = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			*error_codep = MSO_BB_CITY_MISSING;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_switch_service_flist: Network Element Missing",ebufp);
			goto cleanup;
		}
		//Call the function to get the NE Address
		fm_mso_get_network_node(ctxp, node_name, city, &ne_add, 2, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_pc_post_pre_flist: Not able to get Network Element ID",ebufp);
			goto cleanup;
		}
		if(ne_add){
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, (void *)ne_add, ebufp);
		}
	}*/
	else
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_pc_post_pre_flist: Error",ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_pc_post_pre_flist enriched input flist", *i_flistpp);

	cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	return;
}

static void    
prepare_bb_cam_ti_flist(
	pcm_context_t		*ctxp,
	pin_flist_t			**i_flistpp,
	char				**act,
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*si_flistp = NULL;
	pin_flist_t			*so_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*sub_flistp = NULL;
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*cred_flistp = NULL;
	pin_flist_t			*subscriber = NULL;
	
	
	pin_flist_t			*mso_avp = NULL;
	pin_flist_t			*bb_info = NULL;

	poid_t				*ac_pdp = NULL;
	void				*vp = NULL;
	char				*population_id = "0001"; //this is the only expected value in downstream system
	char				*ne = NULL;
	
	char				*serv_stat = "True";
	char 				*vc_id = NULL;
	char				*device_type = NULL;
	char				*acc_no = NULL;
	char				*node_name = NULL;
	char				*serv_city = NULL;
	char				*task = "MODIFY_USER_NSURE";
	char				*ne_add = NULL;

	int32				*state_id = NULL;
	int				    *tech = NULL;
    
    char                *str_role = NULL;
    char                *city = NULL;
    char                msg[100];
    
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);


	/***********************************************************
	* Log input flist
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_cam_ti_flist input flist", *i_flistpp);

	/*
	* Validate input received
	*/

	/*
	* Enrich with additional service details 
	*/
	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_cam_ti_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_cam_ti_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	node_name = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
    str_role = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_ROLES, 1, ebufp);
    
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NE_ELEMENT_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_cam_ti_flist: Network Element Missing",ebufp);
		goto cleanup;
	}
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp); 
	//ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 
	//PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

	
	if(*tech == 1 || *tech == 2 || *tech == 5 || *tech == 3)
	{
		*act = (char *) malloc(strlen("CHANGE_AUTH_DOCSIS")+1);
		strcpy(*act,"CHANGE_AUTH_DOCSIS");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		if(*tech == 1 || *tech == 2){
			prepare_docsis_bcc_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"prepare_bb_cam_ti_flist:prepare_docsis_bcc_flist Error",ebufp);
				goto cleanup;
			}
		}
		prepare_docsis_subscriberavps2_flist(ctxp, i_flistpp, so_flistp,*act, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_cam_ti_flist:prepare_docsis_subscriberavps2_flist Error",ebufp);
			goto cleanup;
		}
		delete_cred_qps_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_cam_ti_flist:delete_cred_qps_flist Error",ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"input flist modified", *i_flistpp);
	}
	/*else
	{
		*act = (char *) malloc(strlen("CHANGE_AUTH_ETHERNET")+1);
		strcpy(*act,"CHANGE_AUTH_ETHERNET");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		ethernet_details_flist(ctxp, i_flistpp, so_flistp, task, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_cam_ti_flist:ethernet_details_flist Error",ebufp);
			goto cleanup;
		}
	}*/
	else
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}

    if( (strcmp(other_ne_id,HW_NETWORK_STER_ID) == 0))
    {
            PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_ROLES, str_role, ebufp);
    }
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_cam_ti_flist: Error",ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_cam_ti_flist enriched input flist", *i_flistpp);

	cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	return;
}

static void    
prepare_bb_cam_tp_flist(
	pcm_context_t		*ctxp,
	pin_flist_t			**i_flistpp,
	char				**act,
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*si_flistp = NULL;
	pin_flist_t			*so_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*sub_flistp = NULL;
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*cred_flistp = NULL;
	pin_flist_t			*subscriber = NULL;
	
	
	pin_flist_t			*mso_avp = NULL;
	pin_flist_t			*bb_info = NULL;

	poid_t				*ac_pdp = NULL;
	void				*vp = NULL;
	char				*population_id = "0001"; //this is the only expected value in downstream system
	char				*ne = NULL;
	
	char				*serv_stat = "True";
	char 				*vc_id = NULL;
	char				*device_type = NULL;
	char				*acc_no = NULL;
	char				*node_name = NULL;
	char				*serv_city = NULL;
	char				*task = "MODIFY_USER_NSURE";
    char                *str_role = NULL;
    char                *city = NULL; 
	int32				*state_id = NULL;
	int				    *tech = NULL;
    char                *ne_add = NULL;
    char                msg[100];
	
	
	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);


	/***********************************************************
	* Log input flist
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_cam_tp_flist input flist", *i_flistpp);

	/*
	* Validate input received
	*/

	/*
	* Enrich with additional service details 
	*/
    si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_cam_tp_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_cam_tp_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp);
    str_role = PIN_FLIST_FLD_GET(so_flistp, MSO_FLD_ROLES, 1, ebufp);
	//ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 
	//PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

	
	if(*tech == 1 || *tech == 2 || *tech == 5 || *tech == 3)
	{
		*act = (char *) malloc(strlen("CHANGE_AUTH_DOCSIS")+1);
		strcpy(*act,"CHANGE_AUTH_DOCSIS");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		if(*tech == 1 || *tech == 2){
			prepare_docsis_bcc_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"prepare_bb_cam_tp_flist:Error",ebufp);
				goto cleanup;
			}
		}
		prepare_docsis_subscriberavps2_flist(ctxp, i_flistpp, so_flistp, *act ,error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_cam_tp_flist:Error",ebufp);
			goto cleanup;
		}
		delete_cred_qps_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_cam_tp_flist:Error",ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_bb_cam_tp_flist: input flist", *i_flistpp);
	}
	/*else
	{
		*act = (char *) malloc(strlen("CHANGE_AUTH_ETHERNET")+1);
		strcpy(*act,"CHANGE_AUTH_ETHERNET");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		ethernet_details_flist(ctxp, i_flistpp, so_flistp, task, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_cam_tp_flist:Error",ebufp);
			goto cleanup;
		}
	}*/
	else
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}
    if( (strcmp(other_ne_id,HW_NETWORK_STER_ID) == 0))
    {
            PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_ROLES, str_role, ebufp);
    }
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_cam_tp_flist: Error",ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_cam_tp_flist enriched input flist", *i_flistpp);

	cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	return;
}

static void    
prepare_bb_cam_it_flist(
	pcm_context_t		*ctxp,
	pin_flist_t			**i_flistpp,
	char				**act,
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t		*si_flistp = NULL;
	pin_flist_t		*so_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*sub_flistp = NULL;
	pin_flist_t		*task_flistp = NULL;
	pin_flist_t		*cred_flistp = NULL;
	pin_flist_t		*subscriber = NULL;
	
	
	pin_flist_t		*mso_avp = NULL;
	pin_flist_t		*bb_info = NULL;

	poid_t			*ac_pdp = NULL;
	void			*vp = NULL;
	char			*population_id = "0001"; //this is the only expected value in downstream system
	char			*ne = NULL;
	
	char			*serv_stat = "True";
	char 			*vc_id = NULL;
	char			*device_type = NULL;
	char			*acc_no = NULL;
	char			*node_name = NULL;
	char			*serv_city = NULL;
	char			*task = "MODIFY_USER_NSURE";

	int32			*state_id = NULL;
	int			    *tech = NULL;
	
	
	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);


	/***********************************************************
	* Log input flist
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_cam_it_flist input flist", *i_flistpp);

	/*
	* Validate input received
	*/

	/*
	* Enrich with additional service details 
	*/
	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_cam_it_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_cam_it_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp); 
	//ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 
	//PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

	if(*tech == 1 || *tech == 2 || *tech == 5 || *tech == 3)
	{
		*act = (char *) malloc(strlen("CHANGE_AUTH_TO_TAL_DOCSIS")+1);
		strcpy(*act,"CHANGE_AUTH_TO_TAL_DOCSIS");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		if(*tech == 1 || *tech == 2){
			prepare_docsis_bcc_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"prepare_bb_cam_it_flist:Error",ebufp);
				goto cleanup;
			}
		}
		prepare_docsis_subscriberavps2_flist(ctxp, i_flistpp, so_flistp, *act ,error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_cam_it_flist:Error",ebufp);
			goto cleanup;
		}
		add_cred_qps_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_cam_tp_flist:Error",ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_bb_cam_it_flist: input flist", *i_flistpp);
	}
	/*else
	{
		*act = (char *) malloc(strlen("CHANGE_AUTH_ETHERNET")+1);
		strcpy(*act,"CHANGE_AUTH_ETHERNET");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		ethernet_details_flist(ctxp, i_flistpp, so_flistp, task, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_cam_it_flist:Error",ebufp);
			goto cleanup;
		}
		//prepare_mqnsure_activation_flist(ctxp, i_flistpp, so_flistp, ebufp);
	}*/
	else
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_cam_it_flist: Error",ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_cam_it_flist enriched input flist", *i_flistpp);

	cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	return;
}


static void    
prepare_bb_cam_pt_flist(
	pcm_context_t		*ctxp,
	pin_flist_t			**i_flistpp,
	char				**act,
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t		*si_flistp = NULL;
	pin_flist_t		*so_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*sub_flistp = NULL;
	pin_flist_t		*task_flistp = NULL;
	pin_flist_t		*cred_flistp = NULL;
	pin_flist_t		*subscriber = NULL;
	
	
	pin_flist_t		*mso_avp = NULL;
	pin_flist_t		*bb_info = NULL;

	poid_t			*ac_pdp = NULL;
	void			*vp = NULL;
	char			*population_id = "0001"; //this is the only expected value in downstream system
	char			*ne = NULL;
	
	char			*serv_stat = "True";
	char 			*vc_id = NULL;
	char			*device_type = NULL;
	char			*acc_no = NULL;
	char			*node_name = NULL;
	char			*serv_city = NULL;
	char			*task = "MODIFY_USER_NSURE";


	int32			*state_id = NULL;
	int			    *tech = NULL;
	
	
	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);


	/***********************************************************
	* Log input flist
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_cam_pt_flist input flist", *i_flistpp);

	/*
	* Validate input received
	*/

	/*
	* Enrich with additional service details 
	*/
	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_cam_pt_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_cam_pt_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp); 
	//ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 
	//PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

	if(*tech == 1 || *tech == 2 || *tech == 5 || *tech == 3)
	{
		*act = (char *) malloc(strlen("CHANGE_AUTH_TO_TAL_DOCSIS")+1);
		strcpy(*act,"CHANGE_AUTH_TO_TAL_DOCSIS");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		if(*tech == 1 || *tech == 2){
			prepare_docsis_bcc_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"prepare_bb_cam_pt_flist:Error",ebufp);
				goto cleanup;
			}
		}
		prepare_docsis_subscriberavps2_flist(ctxp, i_flistpp, so_flistp, *act ,error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_cam_pt_flist:Error",ebufp);
			goto cleanup;
		}
		add_cred_qps_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_cam_pt_flist:Error",ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_bb_cam_pt_flist: input flist", *i_flistpp);
	}
	/*else
	{
		*act = (char *) malloc(strlen("CHANGE_AUTH_ETHERNET")+1);
		strcpy(*act,"CHANGE_AUTH_ETHERNET");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		ethernet_details_flist(ctxp, i_flistpp, so_flistp, task, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_cam_pt_flist:Error",ebufp);
			goto cleanup;
		}
		//prepare_mqnsure_activation_flist(ctxp, i_flistpp, so_flistp, ebufp);
	}*/
	else
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_cam_pt_flist: Error",ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_cam_pt_flist enriched input flist", *i_flistpp);

	cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	return;
}


static void    
prepare_bb_cam_ip_flist(
	pcm_context_t	    *ctxp,
	pin_flist_t	        **i_flistpp,
	char		        **act,
	int		            *error_codep,
	pin_errbuf_t	    *ebufp)
{
	pin_flist_t		*si_flistp = NULL;
	pin_flist_t		*so_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*sub_flistp = NULL;
	pin_flist_t		*task_flistp = NULL;
	pin_flist_t		*cred_flistp = NULL;
	pin_flist_t		*subscriber = NULL;
	
	
	pin_flist_t		*mso_avp = NULL;
	pin_flist_t		*bb_info = NULL;

	poid_t			*ac_pdp = NULL;
	void			*vp = NULL;
	char			*population_id = "0001"; //this is the only expected value in downstream system
	char			*ne = NULL;
	
	char			*serv_stat = "True";
	char 			*vc_id = NULL;
	char			*device_type = NULL;
	char			*acc_no = NULL;
	char			*node_name = NULL;
	char			*serv_city = NULL;

	int32			*state_id = NULL;
	int			    *tech = NULL;
	
	
	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);


	/***********************************************************
	* Log input flist
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_activation_flist input flist", *i_flistpp);

	/*
	* Validate input received
	*/

	/*
	* Enrich with additional service details 
	*/
	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_activation_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_activation_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp); 
	//ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 
	//PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

	if(*tech == 1 || *tech == 2 || *tech == 5)
	{
		*act = (char *) malloc(strlen("CHANGE_AUTH_TO_PORTAL_DOCSIS")+1);
		strcpy(*act,"CHANGE_AUTH_TO_PORTAL_DOCSIS");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		prepare_docsis_subscriberavps2_flist(ctxp, i_flistpp, so_flistp, *act ,error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_cam_ip_flist:Error",ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"input flist", *i_flistpp);
	}
	else
	{
		//prepare_mqnsure_activation_flist(ctxp, i_flistpp, so_flistp, ebufp);
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_cam_ip_flist: Error",ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_cam_ip_flist enriched input flist", *i_flistpp);

	cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	return;
}

static void
prepare_bb_cam_pi_flist(
	pcm_context_t    *ctxp,
	pin_flist_t	    **i_flistpp,
	char		    **act,
	int		        *error_codep,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*si_flistp = NULL;
	pin_flist_t		*so_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*sub_flistp = NULL;
	pin_flist_t		*task_flistp = NULL;
	pin_flist_t		*cred_flistp = NULL;
	pin_flist_t		*subscriber = NULL;
	
	
	pin_flist_t		*mso_avp = NULL;
	pin_flist_t		*bb_info = NULL;

	poid_t			*ac_pdp = NULL;
	void			*vp = NULL;
	char			*population_id = "0001"; //this is the only expected value in downstream system
	char			*ne = NULL;
	
	char			*serv_stat = "True";
	char 			*vc_id = NULL;
	char			*device_type = NULL;
	char			*acc_no = NULL;
	char			*node_name = NULL;
	char			*serv_city = NULL;

	int32			*state_id = NULL;
	int			    *tech = NULL;
	
	
	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);


	/***********************************************************
	* Log input flist
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_activation_flist input flist", *i_flistpp);

	/*
	* Validate input received
	*/

	/*
	* Enrich with additional service details 
	*/
	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_activation_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_activation_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp); 
	//ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 
	//PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

	if(*tech == 1 || *tech == 2)
	{
		*act = (char *) malloc(strlen("CHANGE_AUTH_TO_CLICK_DOCSIS")+1);
		strcpy(*act,"CHANGE_AUTH_TO_CLICK_DOCSIS");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		prepare_docsis_subscriberavps2_flist(ctxp, i_flistpp, so_flistp, *act ,error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_cam_ip_flist:Error",ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"input flist", *i_flistpp);
	}
	else
	{
		//Not required for Ethernet
		//prepare_mqnsure_activation_flist(ctxp, i_flistpp, so_flistp, ebufp);
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_flist: Error",ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_flist enriched input flist", *i_flistpp);

	cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	return;
}

static void
prepare_bb_modem_change_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	    **i_flistpp,
	char		    **act,
	int		        *error_codep,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*si_flistp = NULL;
	pin_flist_t		*so_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*sub_flistp = NULL;
	pin_flist_t		*task_flistp = NULL;
	pin_flist_t		*cred_flistp = NULL;
	pin_flist_t		*subscriber = NULL;
	
	
	pin_flist_t		*mso_avp = NULL;
	pin_flist_t		*bb_info = NULL;

	poid_t			*ac_pdp = NULL;
	void			*vp = NULL;
	char			*population_id = "0001"; //this is the only expected value in downstream system
	char			*ne = NULL;
	
	char			*serv_stat = "True";
	char 			*vc_id = NULL;
	char			*device_type = NULL;
	char			*acc_no = NULL;
	char			*node_name = NULL;
	char			*serv_city = NULL;

	int32			*state_id = NULL;
	int			    *tech = NULL;
	int			    *is_tal = NULL;
    char            *str_rolep = NULL;
	char            *user_id = NULL;
    char            *passwd = NULL;
    char            *user_pass = "Username/Password";
    char            *city = NULL;
    char            *ne_add = NULL;
    char             msg[100];
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);


	/***********************************************************
	* Log input flist
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_activation_flist input flist", *i_flistpp);

	/*
	* Validate input received
	*/

	/*
	* Enrich with additional service details 
	*/
	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_activation_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_activation_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp);
	is_tal = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_ISTAL_FLAG, 0, ebufp);
    str_rolep = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_ROLES, 0, ebufp);
	//ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 
	//PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

	if(tech && (*tech == 1 || *tech == 2 || *tech == 5 || *tech == 4))
	{
		if(str_rolep && strstr(str_rolep, "-Dynamic") && (tech && *tech != 4 ))
		{
			*act = (char *) malloc(strlen("CHANGE_MODEM_DOCSIS")+1);
			strcpy(*act,"CHANGE_MODEM_DOCSIS");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		}
		/***Added function to pass the terminate task1 to TAL service as task1***/
		else if(str_rolep && strstr(str_rolep, "-Static") && (tech && *tech != 4 ))
		{
			*act = (char *) malloc(strlen("CHANGE_MODEM_TAL_DOCSIS")+1);
                        strcpy(*act,"CHANGE_MODEM_TAL_DOCSIS");
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
			if(tech && (*tech == 1 || *tech == 2))
			{
				fm_mso_delete_records(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
                		{
                        		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                		"prepare_bb_modem_change_flist:Error returning from function fm_mso_delete_records",ebufp);
                        		goto cleanup;
                		}
			}
		}
		else if(tech && *tech == 4){
			*act = (char *) malloc(strlen("CHANGE_MODEM_FIBER")+1);
			strcpy(*act,"CHANGE_MODEM_FIBER");
		}
		if(tech && (*tech == 1 || *tech == 2)){
			prepare_docsis_bcc_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"prepare_bb_modem_change_flist :Error",ebufp);
				goto cleanup;
			}
		}
		fm_mso_change_username_qps(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_cam_ip_flist:Error",ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"input flist", *i_flistpp);
	}
	else
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}
    
    if( (strcmp(other_ne_id,HW_NETWORK_STER_ID) == 0))
    { 
        /* Changes for Sterlite upgrade */
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, task_id, ebufp);
        task_id = task_id + 1;
        
        cred_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_CREDENTIALS, 1, ebufp);
        
        user_id = (char *)PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_LOGIN, 0, ebufp);
        
        if ( user_id && strlen(user_id) > 0)
        {
            PIN_FLIST_FLD_SET(cred_flistp, MSO_FLD_NETWORKID, (void *)user_id, ebufp);
        }
        else
        {
            *error_codep = MSO_BB_MANDATORY_FIELD_CODE;
            pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, PIN_FLD_USER_NAME, 0, 0);
            goto cleanup;
        }
        //Add Password
        
        passwd = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_PASSWORD, 0, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
            *error_codep = MSO_BB_MANDATORY_FIELD_CODE;
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                "prepare_bb_activation_flist: Password is missing",ebufp);
            goto cleanup;
        }
        if ( passwd && strlen(passwd) > 0)
        {
            PIN_FLIST_FLD_SET(cred_flistp, MSO_FLD_PASSWORD, (void *)passwd, ebufp);
            //Add the Description
            PIN_FLIST_FLD_SET(cred_flistp, PIN_FLD_DESCR, (void *)user_pass, ebufp);
        }
        else
        {
            *error_codep = MSO_BB_MANDATORY_FIELD_CODE;
            pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
            PIN_ERR_BAD_VALUE, MSO_FLD_PASSWORD, 0, 0);
            goto cleanup;
        }
    }
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_flist: Error",ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_flist enriched input flist", *i_flistpp);

	cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	return;
}

static void
prepare_bb_ip_change_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	    **i_flistpp,
	char		    **act,
	int		        *error_codep,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*si_flistp = NULL;
	pin_flist_t		*so_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*sub_flistp = NULL;
	pin_flist_t		*task_flistp = NULL;
	pin_flist_t		*cred_flistp = NULL;
	pin_flist_t		*subscriber = NULL;
	pin_flist_t		*mso_avp = NULL;

	poid_t			*ac_pdp = NULL;
	void			*vp = NULL;
	char			*population_id = "0001"; //this is the only expected value in downstream system
	char			*ne = NULL;
	
	char			*serv_stat = "True";
	char 			*vc_id = NULL;
	char			*device_type = NULL;
	char			*acc_no = NULL;
	char			*node_name = NULL;
	char			*serv_city = NULL;
	char			*task = "MODIFY_USER_NSURE";

	int32			*state_id = NULL;
	int			    *tech = NULL;
	int		    	*tal = NULL;
    char            *user_id = NULL;
    char            *passwd = NULL;
    char            *user_pass = "Username/Password";
	
    char            *city = NULL;
    char            *ne_add = NULL;
    char            msg[100]; 	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);


	/***********************************************************
	* Log input flist
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_activation_flist input flist", *i_flistpp);

	/*
	* Validate input received
	*/

	/*
	* Enrich with additional service details 
	*/
	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_activation_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_activation_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp); 
	//PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

	if(tech && (*tech == 1 || *tech == 2 || *tech == 5 || *tech == 3 || *tech == 4))
	{
		if (*tech == 4){
			*act = (char *) malloc(strlen("CHANGE_IP_FIBER")+1);
			strcpy(*act,"CHANGE_IP_FIBER");
		}
		else{
			*act = (char *) malloc(strlen("CHANGE_IP_DOCSIS")+1);
			strcpy(*act,"CHANGE_IP_DOCSIS");
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		if(*tech == 1 || *tech == 2){
			prepare_docsis_bcc_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"prepare_bb_cam_ip_flist:Error",ebufp);
				goto cleanup;
			}
		}
		fm_mso_change_username_qps(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_cam_ip_flist:Error",ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"input flist", *i_flistpp);
	}
	/*else
	{
		//Check the TAL flag
		tal = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_ISTAL_FLAG, 0, ebufp);
		if(*tal == 1)
		{
			*act = (char *) malloc(strlen("CHANGE_TAL_IP_ETHERNET")+1);
			strcpy(*act,"CHANGE_TAL_IP_ETHERNET");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
			fm_mso_change_username_nsure(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"prepare_bb_flist: Error",ebufp);
				goto cleanup;
			}
		}
		else
		{
			//Call the Non-TAL function
			*act = (char *) malloc(strlen("CHANGE_NONTAL_IP_ETHERNET")+1);
			strcpy(*act,"CHANGE_NONTAL_IP_ETHERNET");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
			ethernet_details_flist(ctxp, i_flistpp, so_flistp, task, error_codep, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"prepare_bb_flist: Error",ebufp);
				goto cleanup;
			}
		}
	}*/
	else
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}
    
    if( (strcmp(other_ne_id,HW_NETWORK_STER_ID) == 0))
    {
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, task_id, ebufp);
        task_id = task_id + 1;

        cred_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_CREDENTIALS, 1, ebufp);

        user_id = (char *)PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_LOGIN, 0, ebufp);

        if ( user_id && strlen(user_id) > 0)
        {
            PIN_FLIST_FLD_SET(cred_flistp, MSO_FLD_NETWORKID, (void *)user_id, ebufp);
        }
        else
        {
            *error_codep = MSO_BB_MANDATORY_FIELD_CODE;
            pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, PIN_FLD_USER_NAME, 0, 0);
            goto cleanup;
        }
        //Add Password

        passwd = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_PASSWORD, 0, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
            *error_codep = MSO_BB_MANDATORY_FIELD_CODE;
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                "prepare_bb_activation_flist: Password is missing",ebufp);
            goto cleanup;
        }
        if ( passwd && strlen(passwd) > 0)
        {
            PIN_FLIST_FLD_SET(cred_flistp, MSO_FLD_PASSWORD, (void *)passwd, ebufp);
            //Add the Description
            PIN_FLIST_FLD_SET(cred_flistp, PIN_FLD_DESCR, (void *)user_pass, ebufp);
        }
        else
        {
            *error_codep = MSO_BB_MANDATORY_FIELD_CODE;
            pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, MSO_FLD_PASSWORD, 0, 0);
            goto cleanup;
        }
    } 
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_flist: Error",ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_flist enriched input flist", *i_flistpp);

cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	return;
}

static void
prepare_bb_cmts_change_t_flist(
	pcm_context_t	*ctxp,
	pin_flist_t 	**i_flistpp,
	char		    **act,
	int		        *error_codep,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*si_flistp = NULL;
	pin_flist_t		*so_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*sub_flistp = NULL;
	pin_flist_t		*task_flistp = NULL;
	pin_flist_t		*cred_flistp = NULL;
	pin_flist_t		*subscriber = NULL;
	
	
	pin_flist_t		*mso_avp = NULL;
	pin_flist_t		*bb_info = NULL;

	poid_t			*ac_pdp = NULL;
	void			*vp = NULL;
	char			*population_id = "0001"; //this is the only expected value in downstream system
	char			*ne = NULL;
	
	char			*serv_stat = "True";
	char 			*vc_id = NULL;
	char			*device_type = NULL;
	char			*acc_no = NULL;
	char			*node_name = NULL;
	char			*serv_city = NULL;
	int			    *isTAL = 0;
	int32			*state_id = NULL;
	int			    *tech = NULL;
	int32			sub_pro = 0;
    int32           *status = 0;
    
	int		    	*svc_statusp = NULL;
	
    char            *user_id = NULL;
    char            *passwd = NULL;
    char            *user_pass = "Username/Password";
    char            *city = NULL;
    char            *ne_add = NULL;
    char            msg[100];
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);


	/***********************************************************
	* Log input flist
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_cmts_change_t_flist input flist", *i_flistpp);

	/*
	* Validate input received
	*/

	/*
	* Enrich with additional service details 
	*/
	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_cmts_change_t_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_cmts_change_t_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp); 
	isTAL = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_ISTAL_FLAG, 0, ebufp);
	//ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 
	//PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);
	if(*isTAL != 1){
		*error_codep = MSO_BB_TAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_cmts_change_t_flist: Error",ebufp);
		goto cleanup;
	}
	if (tech && *tech == 4){
		*act = (char *) malloc(strlen("CHANGE_CMTS_TAL_FIBER")+1);
		strcpy(*act,"CHANGE_CMTS_TAL_FIBER");
	}
	else{
		*act = (char *) malloc(strlen("CHANGE_CMTS_TAL_DOCSIS")+1);
		strcpy(*act,"CHANGE_CMTS_TAL_DOCSIS");
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );

	if(tech && (*tech == 1 || *tech == 2 || *tech == 3 || *tech == 5 || *tech == 4))
	{
		/*Payload flist changes as per plan status*/
                status = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_STATUS, 0, ebufp);
                if(*status == MSO_PROV_ACTIVE && (*tech == 1 || *tech == 2))
                {
			prepare_docsis_bcc_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
			
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_cmts_change_t_flist:Error1",ebufp);
			goto cleanup;
		}
		}
		else 
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"status of the mso_purchased_plan is either  4 or 5 or 6, no bcc task");
		}
		/*RAVI Payload flist changes as per plan status*/
		prepare_docsis_subscriberavps_flist(ctxp, i_flistpp, so_flistp, (char *) *act ,error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_cmts_change_t_flist:Error2",ebufp);
			goto cleanup;
		}
		fm_mso_change_username_qps(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_cmts_change_t_flist:Error3",ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"input flist", *i_flistpp);
	}
	else
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}

	/******* Pavan Bellala 08-09-2015 ********************
	Add service status of the  service poid to be used
	later during processing response
	*****************************************************/
	svc_statusp = PIN_FLIST_FLD_GET(so_flistp,PIN_FLD_STATUS,1,ebufp);
	PIN_FLIST_FLD_SET(*i_flistpp,PIN_FLD_STATUS_FLAGS,svc_statusp,ebufp);
    
    if( (strcmp(other_ne_id,HW_NETWORK_STER_ID) == 0))
    {
        task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, task_id, ebufp);
        task_id = task_id + 1;
        
        cred_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_CREDENTIALS, 1, ebufp);

        user_id = (char *)PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_LOGIN, 0, ebufp);
    
        if ( user_id && strlen(user_id) > 0)
        {
            PIN_FLIST_FLD_SET(cred_flistp, MSO_FLD_NETWORKID, (void *)user_id, ebufp);
        }
        else
        {
            *error_codep = MSO_BB_MANDATORY_FIELD_CODE;
            pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, PIN_FLD_USER_NAME, 0, 0);
            goto cleanup;
        }
        //Add Password
    
        passwd = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_PASSWORD, 0, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
            *error_codep = MSO_BB_MANDATORY_FIELD_CODE;
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                "prepare_bb_activation_flist: Password is missing",ebufp);
            goto cleanup;
        }
        if ( passwd && strlen(passwd) > 0)
        {
            PIN_FLIST_FLD_SET(cred_flistp, MSO_FLD_PASSWORD, (void *)passwd, ebufp);
            //Add the Description
            PIN_FLIST_FLD_SET(cred_flistp, PIN_FLD_DESCR, (void *)user_pass, ebufp);
        }
        else
        {
            *error_codep = MSO_BB_MANDATORY_FIELD_CODE;
            pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
            PIN_ERR_BAD_VALUE, MSO_FLD_PASSWORD, 0, 0);
            goto cleanup;
        }
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_cmts_change_t_flist: Error",ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_cmts_change_t_flist enriched input flist", *i_flistpp);

cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	return;
}

static void
prepare_bb_cmts_change_nt_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	    **i_flistpp,
	char		    **act,
	int		        *error_codep,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*si_flistp = NULL;
	pin_flist_t		*so_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*sub_flistp = NULL;
	pin_flist_t		*task_flistp = NULL;
	pin_flist_t		*cred_flistp = NULL;
	pin_flist_t		*subscriber = NULL;
	
	
	pin_flist_t		*mso_avp = NULL;
	pin_flist_t		*bb_info = NULL;

	poid_t			*ac_pdp = NULL;
	void			*vp = NULL;
	char			*population_id = "0001"; //this is the only expected value in downstream system
	char			*ne = NULL;
	
	char			*serv_stat = "True";
	char 			*vc_id = NULL;
	char			*device_type = NULL;
	char			*acc_no = NULL;
	char			*node_name = NULL;
	char			*serv_city = NULL;

	int32			*state_id = NULL;
	int			    *tech = NULL;
	int			    *isTAL = 0;
	int32			*status = 0; 	
	
    int             *svc_statusp = NULL;
	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);


	/***********************************************************
	* Log input flist
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_cmts_change_nt_flist input flist", *i_flistpp);

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Inside prepare_bb_cmts_change_nt_flist");
	/*
	* Validate input received
	*/

	/*
	* Enrich with additional service details 
	*/
	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_cmts_change_nt_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_cmts_change_nt_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp); 
	isTAL = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_ISTAL_FLAG, 0, ebufp); 
	//ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 
	//PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);
	if(*isTAL == 1){
		*error_codep = MSO_BB_NTAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_flist: Error",ebufp);
		goto cleanup;
	}
	*act = (char *) malloc(strlen("CHANGE_CMTS_NONTAL_DOCSIS")+1);
	strcpy(*act,"CHANGE_CMTS_NONTAL_DOCSIS");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
	if(*tech == 1 || *tech == 2 || *tech == 3 || *tech == 5)
	{
		/*RAVI changes done */
		status = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_STATUS, 0, ebufp);
		if(*status == MSO_PROV_ACTIVE && (*tech == 1 || *tech == 2))
		{
			prepare_docsis_bcc_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"prepare_bb_cmts_change_nt_flist:Error",ebufp);
				goto cleanup;
			}
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"status of the mso_purchased_plan is either  4 or 5 or 6, no bcc task");
		}
		/*ended changes*/
		prepare_docsis_subscriberavps_flist(ctxp, i_flistpp, so_flistp,(char *) *act, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_cmts_change_nt_flist:Error",ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_bb_cmts_change_nt_flistinput flist", *i_flistpp);
	}
	else
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}

        /******* Pavan Bellala 08-09-2015 ********************
        Add service status of the  service poid to be used
        later during processing response
        *****************************************************/
        svc_statusp = PIN_FLIST_FLD_GET(so_flistp,PIN_FLD_STATUS,1,ebufp);
        PIN_FLIST_FLD_SET(*i_flistpp,PIN_FLD_STATUS_FLAGS,svc_statusp,ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_cmts_change_nt_flist: Error",ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_cmts_change_nt_flist enriched input flist", *i_flistpp);

	cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	return;
}

static void
prepare_bb_change_name_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	    **i_flistpp,
	char		    **act,
	int		        *error_codep,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*si_flistp = NULL;
	pin_flist_t		*so_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*sub_flistp = NULL;
	pin_flist_t		*task_flistp = NULL;
	pin_flist_t		*cred_flistp = NULL;
	pin_flist_t		*subscriber = NULL;
	
	
	pin_flist_t		*mso_avp = NULL;
	pin_flist_t		*bb_info = NULL;

	poid_t			*ac_pdp = NULL;
	void			*vp = NULL;
	char			*population_id = "0001"; //this is the only expected value in downstream system
	char			*ne = NULL;
	
	char			*serv_stat = "True";
	char 			*vc_id = NULL;
	char			*device_type = NULL;
	char			*acc_no = NULL;
	char			*node_name = NULL;
	char			*serv_city = NULL;

	int32			*state_id = NULL;
	int			    *tech = NULL;
    
    poid_t              *serv_poid = NULL;
    char                param[50] = {'\0'};
    pin_flist_t         *deviceflistp = NULL;
    pin_cookie_t        cookie3 = NULL;
    int32               rec_id3 = 0;
    int                 flag = 0;
    int32               ip_count = 0;
    int                 cred_arrayid =0;
    int                 *isTAL = 0;
    pin_flist_t         *temp_flistp = NULL;
    pin_cookie_t        cookie = NULL;
    int32               rec_id = 0;
    char                *mac_add = NULL;
    char                *dev_type = NULL;
	
    char                *city = NULL;
    char                *ne_add = NULL;
    char                msg[100];	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);


	/***********************************************************
	* Log input flist
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_activation_flist input flist", *i_flistpp);

	/*
	* Validate input received
	*/

	/*
	* Enrich with additional service details 
	*/
	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_activation_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_activation_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp); 
	//ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 
	//PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

	*act = (char *) malloc(strlen("CHANGE_USERNAME_DOCSIS")+1);
	strcpy(*act,"CHANGE_USERNAME_DOCSIS");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
	if(*tech == 1 || *tech == 2 || *tech == 5)
	{
		fm_mso_change_username_qps(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_cam_ip_flist:Error",ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"input flist", *i_flistpp);
	}
	else
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}
    
    if( (strcmp(other_ne_id,HW_NETWORK_STER_ID) == 0))
    {
    isTAL = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_ISTAL_FLAG, 0, ebufp);
    task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, task_id, ebufp);
    task_id = task_id + 1;
    strcpy(param,"/device/ip/static");
    /* get the ip/static device if any */
    fm_mso_search_devices(ctxp,param,serv_poid,&deviceflistp, error_codep, ebufp);
    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "prepare_bb_activation_flist: Error",ebufp);
        goto cleanup;
    }
    cookie3 = NULL;
    rec_id3 = 0;
    flag = 0;
    ip_count = PIN_FLIST_ELEM_COUNT(deviceflistp, PIN_FLD_RESULTS, ebufp);
    
    cred_arrayid = 0;
    if(*isTAL)
    {
        if(PIN_FLIST_ELEM_COUNT(deviceflistp, PIN_FLD_RESULTS, ebufp) > 0)
        {
            while ( (temp_flistp = PIN_FLIST_ELEM_GET_NEXT (deviceflistp,
                       PIN_FLD_RESULTS, &rec_id3, 1,&cookie3, ebufp)) != (pin_flist_t *)NULL )
            {
                cred_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_CREDENTIALS, cred_arrayid, ebufp);
            
                PIN_FLIST_FLD_COPY(temp_flistp, PIN_FLD_DEVICE_ID,cred_flistp, MSO_FLD_NETWORKID, ebufp);
                PIN_FLIST_FLD_SET(cred_flistp, PIN_FLD_DESCR, "IP Address", ebufp);
                cred_arrayid ++;
            }

        } else{
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR , "Ip does'nt exist for this tal account");
                        pin_set_err(ebufp, PIN_ERRLOC_FM,
                                PIN_ERRCLASS_SYSTEM_DETERMINATE,PIN_ERR_BAD_VALUE, PIN_FLD_DEVICE_ID, 0, 0);
                        *error_codep = MSO_BB_IP_DEVICE_ERROR;
                        goto cleanup;
        }
    }
    if(deviceflistp != NULL)PIN_FLIST_DESTROY_EX(&deviceflistp,NULL);
    strcpy(param,"/device/modem");
    /* get the modem device if any */
    fm_mso_search_devices(ctxp,param,serv_poid,&deviceflistp, error_codep, ebufp);
    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "prepare_bb_activation_flist: Error",ebufp);
        goto cleanup;
    }
    cookie = NULL;
    rec_id =0;
    while ( (temp_flistp = PIN_FLIST_ELEM_GET_NEXT (deviceflistp,
                       PIN_FLD_RESULTS, &rec_id, 1,&cookie, ebufp))
                                                  != (pin_flist_t *)NULL )
        {
            mac_add = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_DEVICE_ID, 0,ebufp);
            dev_type = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_DEVICE_TYPE, 0,ebufp);
            ip_count = 0;
    }
    if(mac_add)
    {
        
        cred_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_CREDENTIALS, cred_arrayid, ebufp);
         if( (dev_type &&
             !(strcmp(dev_type, HW_GPON_MODEM) == 0 ||
               strcmp(dev_type, HW_ONU_GPON_MODEM) == 0 ||
               strcmp(dev_type, JIO_GPON_MODEM) == 0) ) &&
               fm_mso_integ_match_patterns("^([0-9a-fA-F]{2})(([/\\s:-][0-9a-fA-F]{2}){5})$", mac_add) <=0 )
        {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR , "Error:Invalid MAC id");
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,PIN_ERR_BAD_VALUE, PIN_FLD_DEVICE_ID, 0, 0);
            *error_codep = MSO_BB_INVALID_MAC;
            goto cleanup;
        }
        PIN_FLIST_FLD_SET(cred_flistp, MSO_FLD_NETWORKID, (void *)mac_add, ebufp);
        PIN_FLIST_FLD_SET(cred_flistp, PIN_FLD_DESCR, "MAC Address", ebufp);
    }
    else if( ((PIN_FLIST_ELEM_COUNT(deviceflistp, PIN_FLD_RESULTS, ebufp) == 0)
            || mac_add == NULL ) && (*tech == 1 || *tech == 2 || *tech == 5))
    {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR , "Error:No Device Id found");
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,PIN_ERR_BAD_VALUE, PIN_FLD_DEVICE_ID, 0, 0);
            *error_codep = MSO_BB_NO_MODEM_FOUND;
            goto cleanup;
    }
    }
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_flist: Error",ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_flist enriched input flist", *i_flistpp);

	cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	return;
}

static void
prepare_bb_change_pass_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	    **i_flistpp,
	char		    **action,
	int		        *error_codep,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*si_flistp = NULL;
	pin_flist_t		*so_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*sub_flistp = NULL;
	pin_flist_t		*task_flistp = NULL;
	pin_flist_t		*cred_flistp = NULL;
	pin_flist_t		*subscriber = NULL;
	
	
	pin_flist_t		*mso_avp = NULL;
	pin_flist_t		*bb_info = NULL;

	poid_t			*ac_pdp = NULL;
	void			*vp = NULL;
	char			*population_id = "0001"; //this is the only expected value in downstream system
	char			*ne = NULL;
	
	char			*serv_stat = "True";
	char 			*vc_id = NULL;
	char			*device_type = NULL;
	char			*acc_no = NULL;
	char			*node_name = NULL;
	char			*serv_city = NULL;

	int32			*state_id = NULL;
	int			    *tech = NULL;
	
	
	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);


	/***********************************************************
	* Log input flist
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"Begin prepare_bb_change_pass_flist input flist", *i_flistpp);

	/*
	* Validate input received
	*/

	/*
	* Enrich with additional service details 
	*/
	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_change_pass_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_change_pass_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp); 
	//ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 
	//PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

	if(*tech == 1 || *tech == 2 || *tech == 5 || *tech == 3)
	{
		*action = (char *) malloc(strlen("CHANGE_PASSWORD_DOCSIS")+1);
		strcpy(*action,"CHANGE_PASSWORD_DOCSIS");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*action );
		 fm_mso_change_password(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"input flist", *i_flistpp);
	}
	/*else
	{
		*action = (char *) malloc(strlen("CHANGE_PASSWORD_ETHERNET")+1);
		strcpy(*action,"CHANGE_PASSWORD_ETHERNET");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*action );
		fm_mso_change_password(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_change_pass_flist:fm_mso_change_password Error",ebufp);
			goto cleanup;
		}
		//prepare_mqnsure_activation_flist(ctxp, i_flistpp, so_flistp, ebufp);
	}*/
	else
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_change_pass_flist: Error",ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_change_pass_flist enriched input flist", *i_flistpp);

	cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	return;
}

static void    
prepare_bb_end_subs_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	    **i_flistpp,
	char		    **act,
	int		        *error_codep,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*si_flistp = NULL;
	pin_flist_t		*so_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*sub_flistp = NULL;
	pin_flist_t		*task_flistp = NULL;
	pin_flist_t		*cred_flistp = NULL;
	pin_flist_t		*subscriber = NULL;
	
	
	pin_flist_t		*mso_avp = NULL;
	pin_flist_t		*bb_info = NULL;

	poid_t			*ac_pdp = NULL;
	void			*vp = NULL;
	char			*population_id = "0001"; //this is the only expected value in downstream system
	char			*ne = NULL;
    char            *stl_rolep = NULL;	
	char			*serv_stat = "True";
	char 			*vc_id = NULL;
	char			*device_type = NULL;
	char			*acc_no = NULL;
	char			*node_name = NULL;
	char			*serv_city = NULL;

	int32			*state_id = NULL;
	int			    *tech = NULL;
	
    poid_t              *serv_pdp = NULL;
    char                param[50] = {'\0'};
    pin_flist_t         *deviceflistp = NULL;
    pin_cookie_t        cookie3 = NULL;
    int32               rec_id3 = 0;
    int                 flag = 0;
    int32               ip_count = 0;
    int                 cred_arrayid =0;
    int                 *isTAL = NULL;
    pin_flist_t         *temp_flistp = NULL;
    pin_cookie_t        cookie = NULL;
    int32               rec_id = 0;
    char                *mac_add = NULL;
    char                *dev_type = NULL;
    char                *city = NULL;
    char                *ne_add = NULL;
    char                msg[100];
    char                han1[100];
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);


	/***********************************************************
	* Log input flist
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_end_subs_flist input flist", *i_flistpp);

	/*
	* Validate input received
	*/

	/*
	* Enrich with additional service details 
	*/
	si_flistp = PIN_FLIST_CREATE(ebufp);
    serv_pdp = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_end_subs_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_end_subs_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp); 
	stl_rolep = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_ROLES, 1, ebufp); 
	//ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 
	//PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

	if(tech && (*tech == 1 || *tech == 2 || *tech == 5 || *tech == 3 || *tech == 4))
	{
		if (*tech == 4){
			*act = (char *) malloc(strlen("TERMINATE_SUSBCRIBER_FIBER")+1);
			strcpy(*act,"TERMINATE_SUSBCRIBER_FIBER");
		}
		else{
			*act = (char *) malloc(strlen("TERMINATE_SUSBCRIBER_DOCSIS")+1);
			strcpy(*act,"TERMINATE_SUSBCRIBER_DOCSIS");
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		if(*tech == 1 || *tech == 2){
			fm_mso_delete_records(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"prepare_bb_end_subs_flist:Error",ebufp);
				goto cleanup;
			}
		}
		fm_mso_delete_subscriber(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_end_subs_flist:Error",ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"input flist", *i_flistpp);
	}
	/*else
	{
		*act = (char *) malloc(strlen("TERMINATE_SUSBCRIBER_ETHERNET")+1);
		strcpy(*act,"TERMINATE_SUSBCRIBER_ETHERNET");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		fm_mso_delete_ethernet_subs(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_end_subs_flist:Error",ebufp);
			goto cleanup;
		}
		//prepare_mqnsure_activation_flist(ctxp, i_flistpp, so_flistp, ebufp);
	}*/
	else
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}
    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "check issue han: ");
    sprintf(han1,"Value of other_ne_id %s ",other_ne_id);
    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,han1);
    //PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, other_ne_id);

    if( (strcmp(other_ne_id,HW_NETWORK_STER_ID) == 0))
    {
        task_flistp = PIN_FLIST_ELEM_GET(*i_flistpp, MSO_FLD_TASK, 2, 1, ebufp);
        if (!task_flistp)
        {
            task_flistp = PIN_FLIST_ELEM_GET(*i_flistpp, MSO_FLD_TASK, 1, 0, ebufp);
        }
        if (stl_rolep && strstr(stl_rolep, "-Static") || strstr(stl_rolep, "Leaseline"))
        {
            strcpy(param,"/device/ip/static");
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "check issue han1: ");
            /* get the ip/static device if any */
            fm_mso_search_devices(ctxp, param, serv_pdp, &deviceflistp, error_codep, ebufp);
            if (PIN_ERRBUF_IS_ERR(ebufp))
            {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                    "prepare_bb_activation_flist: Error",ebufp);
                goto cleanup;
            }
            cookie3 = NULL;
            rec_id3 = 0;
            flag = 0;
            ip_count = PIN_FLIST_ELEM_COUNT(deviceflistp, PIN_FLD_RESULTS, ebufp);
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "check issue han2: ");
            cred_arrayid = 1;
            if (PIN_FLIST_ELEM_COUNT(deviceflistp, PIN_FLD_RESULTS, ebufp) > 0)
            {
                while ( (temp_flistp = PIN_FLIST_ELEM_GET_NEXT (deviceflistp,
                       PIN_FLD_RESULTS, &rec_id3, 1,&cookie3, ebufp)) != (pin_flist_t *)NULL )
                {
                    cred_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_CREDENTIALS, cred_arrayid, ebufp);
            
                    PIN_FLIST_FLD_COPY(temp_flistp, PIN_FLD_DEVICE_ID,cred_flistp, MSO_FLD_NETWORKID, ebufp);
                    PIN_FLIST_FLD_SET(cred_flistp, PIN_FLD_DESCR, "IP Address", ebufp);
                    cred_arrayid ++;
                }

            } 
            else
            {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR , "Ip does'nt exist for this tal account");
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                               PIN_ERRCLASS_SYSTEM_DETERMINATE,PIN_ERR_BAD_VALUE, PIN_FLD_DEVICE_ID, 0, 0);
                *error_codep = MSO_BB_IP_DEVICE_ERROR;
                goto cleanup;
            } 
            PIN_FLIST_DESTROY_EX(&deviceflistp, NULL);
        } 
        if (*tech == 1 || *tech == 2)
        {
            strcpy(param,"/device/modem");
            /* get the modem device if any */
            fm_mso_search_devices(ctxp, param, serv_pdp, &deviceflistp, error_codep, ebufp);
            if (PIN_ERRBUF_IS_ERR(ebufp))
            {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                "prepare_bb_activation_flist: Error",ebufp);
                goto cleanup;
            }
            cookie = NULL;
            rec_id =0;
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "check issue modem: ");
            while ( (temp_flistp = PIN_FLIST_ELEM_GET_NEXT (deviceflistp,
                       PIN_FLD_RESULTS, &rec_id, 1,&cookie, ebufp))
                                                  != (pin_flist_t *)NULL )
            {
                mac_add = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_DEVICE_ID, 0,ebufp);
                dev_type = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_DEVICE_TYPE, 0,ebufp);
                ip_count = 0;
            }
        
            cred_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_CREDENTIALS, cred_arrayid, ebufp);
            if ( (dev_type &&
             !(strcmp(dev_type, HW_GPON_MODEM) == 0 ||
               strcmp(dev_type, HW_ONU_GPON_MODEM) == 0 ||
               strcmp(dev_type, JIO_GPON_MODEM) == 0) ) &&
                fm_mso_integ_match_patterns("^([0-9a-fA-F]{2})(([/\\s:-][0-9a-fA-F]{2}){5})$", mac_add) <=0 )
            {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR , "Error:Invalid MAC id");
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,PIN_ERR_BAD_VALUE, PIN_FLD_DEVICE_ID, 0, 0);
                *error_codep = MSO_BB_INVALID_MAC;
                goto cleanup;
            }
            PIN_FLIST_FLD_SET(cred_flistp, MSO_FLD_NETWORKID, (void *)mac_add, ebufp);
            PIN_FLIST_FLD_SET(cred_flistp, PIN_FLD_DESCR, "MAC Address", ebufp);
            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"check cred flistp", cred_flistp);
            if( ((PIN_FLIST_ELEM_COUNT(deviceflistp, PIN_FLD_RESULTS, ebufp) == 0)
                || mac_add == NULL ) && (*tech == 3))
            {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR , "Error:No Device Id found");
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,PIN_ERR_BAD_VALUE, PIN_FLD_DEVICE_ID, 0, 0);
                *error_codep = MSO_BB_NO_MODEM_FOUND;
                goto cleanup;
            } 
            PIN_FLIST_DESTROY_EX(&deviceflistp, NULL);
        }
    }
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_end_subs_flist: Error",ebufp);
		goto cleanup;
	}
    PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_ROLES, stl_rolep, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_end_subs_flist enriched input flist", *i_flistpp);

	cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	return;
}

static void    
prepare_bb_reset_device_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	    **i_flistpp,
	char		    **act,
	int		        *error_codep,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*si_flistp = NULL;
	pin_flist_t		*so_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*sub_flistp = NULL;
	pin_flist_t		*task_flistp = NULL;
	pin_flist_t		*cred_flistp = NULL;
	pin_flist_t		*subscriber = NULL;
	pin_flist_t		*deviceflistp = NULL;
	pin_flist_t		*temp_flistp = NULL;
	
	pin_flist_t		*mso_avp = NULL;
	pin_flist_t		*bb_info = NULL;

	poid_t			*ac_pdp = NULL;
	poid_t			*serv_poid = NULL;
	void			*vp = NULL;
	char			*population_id = "0001"; //this is the only expected value in downstream system
	char			*ne = NULL;
	
	char			*serv_stat = "True";
	char 			*vc_id = NULL;
	char			*device_type = NULL;
	char			*acc_no = NULL;
	char			*node_name = NULL;
	char			*serv_city = NULL;
	char			param[50] = {'\0'};
	char			*mac_add = NULL;
	char			*city = NULL;
	char			*ne_add = NULL;

	int32			*state_id = NULL;
	int			    *tech = NULL;
	pin_cookie_t	cookie = NULL;
	int32			rec_id =0;
	
	
	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);


	/***********************************************************
	* Log input flist
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_reset_device_flist input flist", *i_flistpp);

	/*
	* Validate input received
	*/

	/*
	* Enrich with additional service details 
	*/
	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_reset_device_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_reset_device_flist PCM_OP_READ_OBJ output flist", so_flistp);
	serv_poid = PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_POID, 0, ebufp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp); 
	//ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 
	//PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

	*act = (char *) malloc(strlen("RESET_DEVICE_DOCSIS")+1);
	strcpy(*act,"RESET_DEVICE_DOCSIS");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
	if(*tech == 1 || *tech == 2)
	{
		node_name = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			*error_codep = MSO_BB_NE_ELEMENT_MISSING;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_reset_device_flist: Network Element Missing",ebufp);
			goto cleanup;
		}
		//Call the function to get the NE Address
		//fm_mso_get_network_node(ctxp, node_name, &ne_add, 2, error_codep, ebufp);
		city = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			*error_codep = MSO_BB_CITY_MISSING;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_reset_device_flist: Network Element Missing",ebufp);
			goto cleanup;
		}
		//Call the function to get the NE Address
		fm_mso_get_network_node(ctxp, node_name, city, &ne_add, 2, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_reset_device_flist: Error",ebufp);
			goto cleanup;
		}
		task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, task_id, ebufp);
		task_id = task_id + 1;
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "RESET_DEVICE_BCC", ebufp);
		if(ne_add){
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, (void *)ne_add, ebufp);
		}
		strcpy(param,"/device/modem");
		/* get the modem device if any */
		fm_mso_search_devices(ctxp, param, serv_poid, &deviceflistp, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_reset_device_flist: Error",ebufp);
			goto cleanup;
		}
		cookie = NULL;
		rec_id =0;
		while ( (temp_flistp = PIN_FLIST_ELEM_GET_NEXT (deviceflistp,
			       PIN_FLD_RESULTS, &rec_id, 1,&cookie, ebufp))
							  != (pin_flist_t *)NULL ) 
		{
			mac_add = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_DEVICE_ID, 0,ebufp);
			//dev_type = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_DEVICE_TYPE, 0,ebufp);
			//ip_count = 0;
			//strcpy(device_typ, "/device/modem");
			/**** Pavan Bellala 29-07-2015 ***********************************
			 Defect 1136 : Validate device string pattern before provisioning
			*****************************************************************/
			if(mac_add && strcmp(mac_add,"")!=0)
			{
				if(fm_mso_integ_match_patterns("^([0-9a-fA-F]{2})(([/\\s:-][0-9a-fA-F]{2}){5})$", mac_add) <=0 )
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR , "Error:Invalid MAC id");
					pin_set_err(ebufp, PIN_ERRLOC_FM,
								PIN_ERRCLASS_SYSTEM_DETERMINATE,PIN_ERR_BAD_VALUE, PIN_FLD_DEVICE_ID, 0, 0);
					*error_codep = MSO_BB_INVALID_MAC;
					goto cleanup;
				}
			}

		}
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_MAC_ADDRESS, (void *)mac_add, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"next prepare_bb_reset_device_flist :input flist", *i_flistpp);
	}
	else
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_reset_device_flist: Error",ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_reset_device_flist enriched input flist", *i_flistpp);

	cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	if(ne_add)pin_free(ne_add);
	return;
}

static void
prepare_bb_mac_change_flist(
	pcm_context_t		*ctxp, 
	pin_flist_t			**i_flistpp, 
	char				**act,
	int				    *error_codep,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t			*si_flistp = NULL;
	pin_flist_t			*so_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	int				    *tech = NULL;
	char				*task = "MODIFY_USER_NSURE";


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_activation_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_activation_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp);
	if(*tech != 1 && *tech != 2)
	{
		//ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 
		//PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);
		*act = (char *) malloc(strlen("CHANGE_MAC_ETHERNET")+1);
		strcpy(*act,"CHANGE_MAC_ETHERNET");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, *act );
		ethernet_details_flist(ctxp, i_flistpp, so_flistp, task, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_mac_change_flist: Error",ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"prepare_bb_mac_change_flist enriched input flist", *i_flistpp);
	}
	else{
		//Raise Error
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, MSO_FLD_TECHNOLOGY, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}
cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	return;
}

static void
prepare_bb_add_subnet_flist(
	pcm_context_t		*ctxp, 
	pin_flist_t			**i_flistpp, 
	char				**act,
	int				    *error_codep,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t			*si_flistp = NULL;
	pin_flist_t			*so_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	int				    *tech = NULL;
	char				*task = "ACTIVATE_SUBSCRIBER_NSURE";

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_add_subnet_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_add_subnet_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp);
	if(*tech != 1 && *tech != 2)
	{
		//ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 
		//PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);
		*act = (char *) malloc(strlen("ACTIVATE_SUBSCRIBER_ETHERNET")+1);
		strcpy(*act,"ACTIVATE_SUBSCRIBER_ETHERNET");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, *act );
		ethernet_details_flist(ctxp, i_flistpp, so_flistp, task, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_add_subnet_flist: Error",ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"prepare_bb_add_subnet_flist enriched input flist", *i_flistpp);
	}
	else{
		//Raise Error
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, MSO_FLD_TECHNOLOGY, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}
cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	return;
}

static void
prepare_bb_get_user_detail_flist(
	pcm_context_t		*ctxp, 
	pin_flist_t			**i_flistpp, 
	char				**act,
	int				    *error_codep,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t			*si_flistp = NULL;
	pin_flist_t			*so_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	int				    *tech = NULL;
	char				*task = "QUERY_USERDETAILS_NSURE";

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_get_user_detail_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_get_user_detail_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp);
	if(*tech != 1 && *tech != 2)
	{
		*act = (char *) malloc(strlen("GET_MAC_ETHERNET")+1);
		strcpy(*act,"GET_MAC_ETHERNET");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, *act );
		//PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);
		fm_mso_get_ethernet_details(ctxp, i_flistpp, so_flistp, task, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_get_user_detail_flist: Error",ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"prepare_bb_get_user_detail_flist enriched input flist", *i_flistpp);
	}
	else
	{
		//Raise error
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, MSO_FLD_TECHNOLOGY, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}
	cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	return;
}

static void
prepare_bb_get_last_session_flist(
	pcm_context_t		*ctxp, 
	pin_flist_t			**i_flistpp, 
	char				**act, 
	int				    *error_codep,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t			*si_flistp = NULL;
	pin_flist_t			*so_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	int				    *tech = NULL;
	char				*task = "QUERY_SESSION_DETAILS_NSURE";

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_get_last_session_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_get_last_session_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp);
	if(*tech != 1 && *tech != 2)
	{
		*act = (char *) malloc(strlen("GET_LASTSESSION_DETAILS_ETHERNET")+1);
		strcpy(*act,"GET_LASTSESSION_DETAILS_ETHERNET");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, *act );
		//PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);
		fm_mso_get_ethernet_details(ctxp, i_flistpp, so_flistp, task, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_get_last_session_flist: Error",ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"prepare_bb_get_last_session_flist enriched input flist", *i_flistpp);
	}
	else
	{
		//Raise error
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, MSO_FLD_TECHNOLOGY, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}
	cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	return;
}

static void
prepare_bb_update_subs_pool_flist(
	pcm_context_t		*ctxp, 
	pin_flist_t			**i_flistpp, 
	char				**act, 
	int				    *error_codep,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t			*si_flistp = NULL;
	pin_flist_t			*so_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	int				    *tech = NULL;
	char				*task = "MODIFY_USER_NSURE";

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_update_subs_pool_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_update_subs_pool_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp);
	if(*tech != 1 && *tech != 2)
	{
		//ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp); 
		PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);
		*act = (char *) malloc(strlen("CHANGE_SUBSCRIBERPOOL_ETHERNET")+1);
		strcpy(*act,"CHANGE_SUBSCRIBERPOOL_ETHERNET");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, *act );
		ethernet_details_flist(ctxp, i_flistpp, so_flistp, task, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_update_subs_pool_flist: Error",ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"prepare_bb_update_subs_pool_flist enriched input flist", *i_flistpp);
	}
	else{
		//Raise Error
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, MSO_FLD_TECHNOLOGY, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}
cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	return;
}

static void
prepare_bb_release_ip_flist(
	pcm_context_t		*ctxp, 
	pin_flist_t			**i_flistpp, 
	char				**act, 
	int				    *error_codep,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t			*si_flistp = NULL;
	pin_flist_t			*so_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	int				    *tech = NULL;
	int				    *isTAL = NULL;
	char				*task = "MODIFY_USER_NSURE";
	//char				*taltask = "MODIFY_USER_ETHERNET_NSURE";


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_release_ip_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_release_ip_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp);
	isTAL = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_ISTAL_FLAG, 0, ebufp);
	if(*tech != 1 && *tech != 2)
	{
		//PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);
		if(isTAL && *isTAL == 1 )
		{
			*act = (char *) malloc(strlen("CHANGE_AUTH_ETHERNET")+1);
			strcpy(*act,"CHANGE_AUTH_ETHERNET");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, *act );
			fm_mso_tal_release_ip(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"prepare_bb_release_ip_flist: Error",ebufp);
				goto cleanup;
			}
		}
		else if ( isTAL && *isTAL == 0 )
		{
			*act = (char *) malloc(strlen("RELEASE_IP_ETHERNET")+1);
			strcpy(*act,"RELEASE_IP_ETHERNET");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, *act );
			ethernet_details_flist(ctxp, i_flistpp, so_flistp, task, error_codep, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"prepare_bb_release_ip_flist: Error",ebufp);
				goto cleanup;
			}
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"prepare_bb_release_ip_flist enriched input flist", *i_flistpp);
	}
	else
	{
		//Raise Error
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, MSO_FLD_TECHNOLOGY, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}
cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	return;
}

/**********************************************************************************************/

static void
prepare_docsis_qps_activation_flist(
	pcm_context_t		*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp,
	int				    *error_codep,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*cred_flistp = NULL;
	pin_flist_t			*quota_s_flistp = NULL;
	pin_flist_t			*mso_avp = NULL;
	pin_flist_t			*quota_so_flistp = NULL;
	pin_flist_t			*arg_flistp = NULL;
	pin_flist_t			*quota_res_flistp = NULL;
	pin_flist_t			*createbalflistp = NULL;
	pin_flist_t			*plan_iflistp = NULL;
	pin_flist_t			*plan_oflistp = NULL;
	pin_flist_t			*sub_avp_flistp = NULL;
	pin_flist_t			*acc_iflistp = NULL;
	pin_flist_t			*acc_nameinfo_flistp = NULL;
	pin_flist_t			*acc_phones_flistp = NULL;
	pin_flist_t			*acc_oflistp = NULL;
	pin_flist_t			*acc_name_flistp = NULL;
	pin_flist_t			*acc_ph_flistp = NULL;
	pin_flist_t			*deviceflistp = NULL;
	pin_flist_t			*temp_flistp = NULL;
	pin_flist_t			*ip_flistp = NULL;
	pin_flist_t			*mso_subavp = NULL;
    pin_flist_t         *pwd_chng_flistp = NULL;
	poid_t				*ac_pdp = NULL;
	poid_t				*plan_pdp = NULL;
	poid_t				*serv_poid = NULL;
	poid_t				*mso_plan = NULL;
	char				*node_name = NULL;
	char				*acc_no = NULL;
	char				*ph_no = NULL;
	char				*serv_city = NULL;
	char				*f_name = NULL;
	char				*l_name = NULL;
	char				name[265];
	char				*user_id = NULL;
	char				*passwd = NULL;
	char				*user_pass = "Username/Password";
	char				*ip_add = NULL;
	char				*mac_add = NULL;
	char				*plan_name = NULL;
	char				*email = NULL;
	char				*ne_add = NULL;
	char				*dev_type = NULL;
	//char				*ip_add = NULL;
	char				*ip_add_list = NULL;
	char				*mso_ip_add = NULL;
	char				ip_count_str[4];
	char				*agg_no = NULL;
	char 				svc_code[100] ="\0";
	char 				quota_code[100] ="\0";
	char				time_buf[30] = {'\0'};
	char				param[50] = {'\0'};
	char				*prov_tag = NULL;
	char				*city = NULL;
	char				unltd_quota[20] = UNLIMITED_QUOTA;
    char                *stl_rolep = NULL;
    char                *no_days = NULL;
	int				*isTAL = 0;
	int				*is1Click = 0;
	int				*tech = 0;
	int				*ind = NULL;
	int				*bill_when = NULL;
	int				*ph_type = NULL;
	int				*idle_timeout = NULL;
	int				*session_timeout = NULL;
	int				typ = 5;
	int				flag = 0;
	int				srv_typ = 0;
	int				plan_type = 0;

	int32         	s_flags = 0;
	int32          	quota_flag = 0;
	int32			ip_count = 0;
	int32			rec_id = 0;
	int32			rec_id1 = 0;
	int32			acticate_bb_srvc  = MSO_OP_CUST_ACTIVATE_BB_SERVICE;
	int32			rec_id3 = 0;
	int32			rec_id4 = 0;

	pin_cookie_t	cookie = NULL;
	pin_cookie_t	cookie1 = NULL;
	pin_cookie_t	cookie3 = NULL;
	pin_cookie_t	cookie4 = NULL;
	int64          	database = 0;
	time_t			now = 0;
	struct tm		*gm_time;
	pin_decimal_t	*initial_amount = NULL;
	pin_decimal_t	*amount = NULL;
	time_t			expiry = 0;
	time_t			*sub_expiryp = NULL;

	int				cred_arrayid =2;

	FILE			*filep = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	filep = fopen("/tmp/stackt","a+");	

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_docsis_qps_activation input flist", *i_flistpp);
	serv_poid = (poid_t *)PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_POID, 0, ebufp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	agg_no = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_AGREEMENT_NO, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_AGG_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_flist: Error getting value",ebufp);
		goto cleanup;
	}
	/******Add the Task Array for QPS*******************/
	task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, task_id, ebufp);
	task_id = task_id + 1;
	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "ACTIVATE_SUBSCRIBER_QPS", ebufp);
	//PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, (void *)"QPS01", ebufp);
	
		
	//Get the isTAL, is1CLICK and Indicator flags from the BB_INFO
	isTAL = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_ISTAL_FLAG, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_TAL_FLAG_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_flist: Error getting value",ebufp);
		goto cleanup;
	}
	is1Click = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_IS1CLICK_FLAG, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_1CLICK_FLAG_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_flist: Error getting value",ebufp);
		goto cleanup;
	}
	//ind = (int *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_INDICATOR, 0, ebufp);
	//if (PIN_ERRBUF_IS_ERR(ebufp))
//	{
//		*error_codep = MSO_BB_INDICATOR_MISSING;
//		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
//			"prepare_bb_flist: Error getting value",ebufp);
//		goto cleanup;
//	}
	//Get the Name
	ac_pdp = (poid_t *)PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_POID, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_MANDATORY_FIELD_CODE;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_flist: Error getting poid",ebufp);
		goto cleanup;
	}
	//Get the Network Node Name
	node_name = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NE_ELEMENT_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_activation_flist: Network Element Missing",ebufp);
		goto cleanup;
	}
	//Call the function to get the NE Address
	//fm_mso_get_network_node(ctxp, node_name, &ne_add, 1, error_codep, ebufp);
	city = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_CITY_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_switch_service_flist: Network Element Missing",ebufp);
		goto cleanup;
	}
	//Call the function to get the NE Address
	fm_mso_get_network_node(ctxp, node_name, city, &ne_add, 1, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_activation_flist: Not able to get Network Node",ebufp);
		goto cleanup;
	}
	if(ne_add){

		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, (void *)ne_add, ebufp);
	}
	//Get the Technology
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_TECH_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_activation_flist: Technology missing",ebufp);
		goto cleanup;
	}

	//Get the Service City
	serv_city = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_MANDATORY_FIELD_CODE;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_activation_flist: City Missing",ebufp);
		goto cleanup;
	}
	bill_when = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);
		
	//Get the idle timeout
	
	//Get the session timeout
	//session_timeout = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_SESSION_TIMEOUT, 0, ebufp);
//	if (PIN_ERRBUF_IS_ERR(ebufp))
//	{
//		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
//			"prepare_bb_activation_flist: Session Timeout Missing",ebufp);
//		goto cleanup;
//	}
	
	//Call the function to get the First Name and Last Name
	fm_mso_get_account_details(ctxp, ac_pdp, &f_name, &l_name, &acc_no, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_flist: Error getting name",ebufp);
		goto cleanup;
	}

	
	//Concatenating the First and Last Names
	if(f_name && sizeof(f_name) > 1 && l_name && sizeof(l_name) > 1)
	{
		sprintf(name,"%s %s",f_name,l_name);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Name: ");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, name);
		if ( name && strlen(name) > 0) {
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_NAME,
				(void *)name, ebufp);
		}
		else
		{
			*error_codep = MSO_BB_NAME_MISSING;
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_BAD_VALUE, PIN_FLD_NAME, 0, 0);
			goto cleanup;
		}
	}
	//If Name is not found or is null set error 
	else
	{
		*error_codep = MSO_BB_NAME_MISSING;
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_NAME, 0, 0);
		goto cleanup;
	}
	 
	//Add User ID 
	cred_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_CREDENTIALS, 1, ebufp);
//	if(*isTAL == 1)
//	{
//		fm_mso_get_login(ctxp, &user_id, so_flistp, error_codep, ebufp);
//		if (PIN_ERRBUF_IS_ERR(ebufp))
//		{
//			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
//				"prepare_bb_flist: Error getting name",ebufp);
//			goto cleanup;
//		}
//	}
//	else{
	user_id = (char *)PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_LOGIN, 0, ebufp);
	//}
	if ( user_id && strlen(user_id) > 0)
	{
	    PIN_FLIST_FLD_SET(cred_flistp, MSO_FLD_NETWORKID, (void *)user_id, ebufp);
	}
	else
	{
		*error_codep = MSO_BB_MANDATORY_FIELD_CODE;
	    pin_set_err(ebufp, PIN_ERRLOC_FM,
		PIN_ERRCLASS_SYSTEM_DETERMINATE,
		    PIN_ERR_BAD_VALUE, PIN_FLD_USER_NAME, 0, 0);
	    goto cleanup;
	}
	//Add Password
	//passwd = (char *)PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_PASSWD, 0, ebufp);
	passwd = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_PASSWORD, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_MANDATORY_FIELD_CODE;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_activation_flist: Password is missing",ebufp);
		goto cleanup;
	}
	if ( passwd && strlen(passwd) > 0)
	{
        if((other_ne_id && strcmp(other_ne_id,HW_NETWORK_STER_ID) == 0))
        {
                 stl_rolep = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_ROLES, 0, ebufp);
                 if (stl_rolep && ((strstr(stl_rolep, "MEN-Dynamic") !=0 || strstr(stl_rolep, "GPON-Dynamic")!=0)))
                 {
                    PIN_FLIST_FLD_SET(cred_flistp, MSO_FLD_PASSWORD, (void *)passwd, ebufp);
                 }     
        }
        else
        {
                   PIN_FLIST_FLD_SET(cred_flistp, MSO_FLD_PASSWORD, (void *)passwd, ebufp);
        }
        PIN_FLIST_FLD_SET(cred_flistp, PIN_FLD_DESCR, (void *)user_pass, ebufp);
	}
	else
	{
	    *error_codep = MSO_BB_MANDATORY_FIELD_CODE;
	    pin_set_err(ebufp, PIN_ERRLOC_FM,
		PIN_ERRCLASS_SYSTEM_DETERMINATE,
		    PIN_ERR_BAD_VALUE, MSO_FLD_PASSWORD, 0, 0);
	    goto cleanup;
	}
	
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_docsis_qps_activation input flist", *i_flistpp);	
	//Add expiration date
	now = pin_virtual_time(NULL);
	gm_time = gmtime(&now);
	strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%T.000+0530", gm_time);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Time :");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, time_buf);
	
	//PIN_FLIST_FLD_SET(cred_flistp, MSO_FLD_EXPIRATION_DATE, (void *)time_buf, ebufp);
	//Add IP Address
	//get_ip_mac_address(ctxp, so_flistp, &ip_add, &mac_add, ebufp);
	
	
	
	
	
	strcpy(param,"/device/ip/static");
	/* get the ip/static device if any */
	fm_mso_search_devices(ctxp,param,serv_poid,&deviceflistp, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_activation_flist: Error",ebufp);
		goto cleanup;
	}
	cookie3 = NULL;
	rec_id3 = 0;
	flag = 0;
	ip_count = PIN_FLIST_ELEM_COUNT(deviceflistp, PIN_FLD_RESULTS, ebufp);

	/****** Pavan Bellala 05-11-2015 ****************************
	QPS IP address Structure change

	OLD STRUCTURE (for two IPs):-                                      NEW STRUCTURE: 
	----------------------------                                       --------------  
    <MSO_FLD_CREDENTIALS elem="1">                                        <MSO_FLD_CREDENTIALS elem="1"> 
    <MSO_FLD_NETWORKID>Priya11310244</MSO_FLD_NETWORKID>            	    <MSO_FLD_NETWORKID>Priya11310244</MSO_FLD_NETWORKID>     
	<MSO_FLD_PASSWORD>BroadBand@123</MSO_FLD_PASSWORD>                  <MSO_FLD_PASSWORD>BroadBand@123</MSO_FLD_PASSWORD> 
	<DESCR>Username/Password</DESCR>                                    <DESCR>Username/Password</DESCR>   
    </MSO_FLD_CREDENTIALS>                                                </MSO_FLD_CREDENTIALS>                                     
    <MSO_FLD_CREDENTIALS elem="2">                                          <MSO_FLD_CREDENTIALS elem="2">                            
	<MSO_FLD_NETWORKID>60.243.6.237,60.243.6.251</MSO_FLD_NETWORKID>    <MSO_FLD_NETWORKID>60.243.6.237</MSO_FLD_NETWORKID> 
	<DESCR>IP Address</DESCR>                                           <DESCR>IP Address</DESCR>                           
    </MSO_FLD_CREDENTIALS>                                                </MSO_FLD_CREDENTIALS>                                      
    <MSO_FLD_CREDENTIALS elem="3">                                          <MSO_FLD_CREDENTIALS elem="3">                             
	<MSO_FLD_NETWORKID>FC:4A:E9:0B:D7:9A</MSO_FLD_NETWORKID>       	    <MSO_FLD_NETWORKID>60.243.6.251</MSO_FLD_NETWORKID> 
	<DESCR>MAC Address</DESCR>                                     	    <DESCR>IP Address</DESCR>                            
    </MSO_FLD_CREDENTIALS>                                                </MSO_FLD_CREDENTIALS>                                      
									    <MSO_FLD_CREDENTIALS elem="4">                             
								            <MSO_FLD_NETWORKID>FC:4A:E9:0B:D7:9A</MSO_FLD_NETWORKID> 
									    <DESCR>MAC Address</DESCR> 
									  </MSO_FLD_CREDENTIALS> 
	************************************************************/

	//while ( (temp_flistp = PIN_FLIST_ELEM_GET_NEXT (deviceflistp,
        //              PIN_FLD_RESULTS, &rec_id3, 1,&cookie3, ebufp))
        //                                          != (pin_flist_t *)NULL ) 
//	{
        //	ip_add = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_DEVICE_ID, 0,ebufp);
        //	dev_type = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_DEVICE_TYPE, 0,ebufp);
        	//client_class = "in_progress";
        	//ip_add_list = (char *)pin_malloc(1);
			//Pawan:17-03-2015 Added below and Commented next block 
	//		if(flag == 1)
	//		{ 
	//			ip_add_list = (char *)realloc(ip_add_list,(strlen(ip_add_list)+strlen(ip_add)+strlen(",")+1));
	//			strcat(ip_add_list, ",");
	//			strcat(ip_add_list, ip_add);
	//		}
	//		else{
	///			ip_add_list = (char *)malloc(strlen(ip_add)+1);
	///			sprintf(ip_add_list, "%s", ip_add);
	///		}
	//		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ip_add_list: ");
	//		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, ip_add_list);
	//		flag = 1;
		/* 
		flag = 0;
		cookie4 = NULL;
		rec_id4 = 0;
		ip_flistp = PIN_FLIST_ELEM_GET(temp_flistp, MSO_FLD_IP_DATA, 0, 1,ebufp);
		if(ip_flistp)
		{
			mso_ip_add = PIN_FLIST_FLD_GET(ip_flistp, MSO_FLD_IP_ADDRS, 0,ebufp);
			
			if(flag == 1)
			{ 
				ip_add_list = (char *)realloc(ip_add_list,(strlen(ip_add_list)+strlen(mso_ip_add)+strlen(",")+1));
				strcat(ip_add_list, ",");
				strcat(ip_add_list, mso_ip_add);
			}
			else{
				ip_add_list = (char *)malloc(strlen(mso_ip_add)+1);
				sprintf(ip_add_list, "%s", mso_ip_add);
			}
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ip_add_list: ");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, ip_add_list);
			flag = 1;
		}
		*/
//	}
	//Check if the customer is TAL
	cred_arrayid = 2;
	if(*isTAL)
	{
		if(PIN_FLIST_ELEM_COUNT(deviceflistp, PIN_FLD_RESULTS, ebufp) > 0)
		{
			while ( (temp_flistp = PIN_FLIST_ELEM_GET_NEXT (deviceflistp,
				       PIN_FLD_RESULTS, &rec_id3, 1,&cookie3, ebufp)) != (pin_flist_t *)NULL ) 
			{
				cred_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_CREDENTIALS, cred_arrayid, ebufp);
			//Add the list of IP Address 
			//if(ip_add_list){
			//	PIN_FLIST_FLD_PUT(cred_flistp, MSO_FLD_NETWORKID, (void *)ip_add_list, ebufp);
			//}
				PIN_FLIST_FLD_COPY(temp_flistp, PIN_FLD_DEVICE_ID,cred_flistp, MSO_FLD_NETWORKID, ebufp);
				PIN_FLIST_FLD_SET(cred_flistp, PIN_FLD_DESCR, "IP Address", ebufp);
				cred_arrayid ++;
			}

		} else{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR , "Ip does'nt exist for this tal account");
                        pin_set_err(ebufp, PIN_ERRLOC_FM,
                                PIN_ERRCLASS_SYSTEM_DETERMINATE,PIN_ERR_BAD_VALUE, PIN_FLD_DEVICE_ID, 0, 0);
                        *error_codep = MSO_BB_IP_DEVICE_ERROR;
                        goto cleanup;

			/*cred_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_CREDENTIALS, cred_arrayid, ebufp);
			PIN_FLIST_FLD_SET(cred_flistp, MSO_FLD_NETWORKID, (void *)NULL, ebufp);
			PIN_FLIST_FLD_SET(cred_flistp, PIN_FLD_DESCR, "IP Address", ebufp);
			cred_arrayid ++;*/
		}
	}
	if(deviceflistp != NULL)PIN_FLIST_DESTROY_EX(&deviceflistp,NULL);
	strcpy(param,"/device/modem");
	/* get the modem device if any */
	fm_mso_search_devices(ctxp,param,serv_poid,&deviceflistp, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_activation_flist: Error",ebufp);
		goto cleanup;
	}
	cookie = NULL;
	rec_id =0;
	while ( (temp_flistp = PIN_FLIST_ELEM_GET_NEXT (deviceflistp,
                       PIN_FLD_RESULTS, &rec_id, 1,&cookie, ebufp))
                                                  != (pin_flist_t *)NULL ) 
        {
        	mac_add = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_DEVICE_ID, 0,ebufp);
        	dev_type = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_DEVICE_TYPE, 0,ebufp);
        	ip_count = 0;
	}
	//Add MAC Address
	if(mac_add)
	{
		/****
		if(*isTAL == 0){
			cred_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_CREDENTIALS, 2, ebufp);
		}
		else{
			cred_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_CREDENTIALS, 3, ebufp);
		}
		****/
		cred_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_CREDENTIALS, cred_arrayid, ebufp);
		/**** Pavan Bellala 29-07-2015 ***********************************
		 Defect 1136 : Validate device string pattern before provisioning
		*****************************************************************/

		// Added the below code for JIO-CPS Integration-ISP Enhancement
		//Skip MAC validation for ONU (GPON) modem for JIO and Hathway Networks and validate the device_id pattern for other device types.
		if( (dev_type && 
			 !(strcmp(dev_type, HW_GPON_MODEM) == 0 || 
			   strcmp(dev_type, HW_ONU_GPON_MODEM) == 0 || 
			   strcmp(dev_type, JIO_GPON_MODEM) == 0) ) && 
			   fm_mso_integ_match_patterns("^([0-9a-fA-F]{2})(([/\\s:-][0-9a-fA-F]{2}){5})$", mac_add) <=0 )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR , "Error:Invalid MAC id");
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,PIN_ERR_BAD_VALUE, PIN_FLD_DEVICE_ID, 0, 0);
			*error_codep = MSO_BB_INVALID_MAC;
			goto cleanup;
		}		
		PIN_FLIST_FLD_SET(cred_flistp, MSO_FLD_NETWORKID, (void *)mac_add, ebufp);
		PIN_FLIST_FLD_SET(cred_flistp, PIN_FLD_DESCR, "MAC Address", ebufp);
	}
	else if( ((PIN_FLIST_ELEM_COUNT(deviceflistp, PIN_FLD_RESULTS, ebufp) == 0)
			|| mac_add == NULL ) && (*tech == 1 || *tech == 2 || *tech == 5))
	{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR , "Error:No Device Id found");
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,PIN_ERR_BAD_VALUE, PIN_FLD_DEVICE_ID, 0, 0);
			*error_codep = MSO_BB_NO_MODEM_FOUND;
			goto cleanup;
	}
	plan_pdp = (poid_t *)PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_PLAN_OBJ, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Plan Name not found",ebufp);
		*error_codep = MSO_BB_PLAN_OBJ_MISSING; 
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_NAME, 0, 0);
		goto cleanup;
	}
	plan_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(plan_iflistp, PIN_FLD_POID, plan_pdp, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read input flist", plan_iflistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, plan_iflistp, &plan_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read output flist", plan_oflistp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"prepare_docsis_qps_activation_flist: Error", ebufp);
		goto cleanup;
	}
	plan_name = (char *)PIN_FLIST_FLD_GET(plan_oflistp, PIN_FLD_NAME, 0, ebufp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Plan Name not found",ebufp);
		*error_codep = MSO_BB_PLAN_OBJ_MISSING;
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_NAME, 0, 0);
		goto cleanup;
	}
	

	prov_tag = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_PROVISIONING_TAG, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Provisioning Tag not found",ebufp);
		*error_codep = MSO_BB_PROV_TAG_ERROR;
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_NAME, 0, 0);
		goto cleanup;
	}
	srv_typ = fm_mso_get_srv_type(ctxp, plan_name, prov_tag, tech, bill_when, serv_city, error_codep, svc_code,quota_code, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
             PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "prepare_docsis_qps_activation_flist: Error", ebufp);
             goto cleanup;
	}

	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_SERVICE_CODE, svc_code, ebufp);
	mso_subavp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_SUB_AVP, 1, ebufp);
	PIN_FLIST_FLD_SET(mso_subavp, MSO_FLD_CODE, "ServiceType", ebufp);
	switch (srv_typ)
	{
	case 0:
		PIN_FLIST_FLD_SET(mso_subavp, MSO_FLD_VALUE, "POST", ebufp);
		break;
	case 1:
		PIN_FLIST_FLD_SET(mso_subavp, MSO_FLD_VALUE, "PRE", ebufp);
		break;
	default:
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Invalid Service Type" );
		*error_codep = MSO_BB_MANDATORY_FIELD_CODE;
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, MSO_FLD_VALUE, 0, 0);
		goto cleanup;
	}

	//sub_avp_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_SUB_AVP, 1, ebufp);
	//PIN_FLIST_FLD_SET(sub_avp_flistp, MSO_FLD_CODE, "ServiceType", ebufp);
	//PIN_FLIST_FLD_SET(sub_avp_flistp, MSO_FLD_VALUE, "PRE", ebufp);
	
	quota_flag = fm_quota_check(ctxp, plan_name, prov_tag, tech, &plan_type, &initial_amount, error_codep, ebufp);


	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"prepare_bb_activation_flist: Error getting Quota",ebufp);
		PIN_ERRBUF_RESET(ebufp);
		goto cleanup;
	}
	sub_avp_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_SUB_AVP, 2, ebufp);
	PIN_FLIST_FLD_SET(sub_avp_flistp, MSO_FLD_CODE, "isQuota", ebufp);
	if(quota_flag == 1)
	{
		PIN_FLIST_FLD_SET(sub_avp_flistp, MSO_FLD_VALUE, "true", ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(sub_avp_flistp, MSO_FLD_VALUE, "false", ebufp);
	}
	
	//Add task_flistp Status
	//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_STATUS, "Active", ebufp);
	//Add the start date
	now = pin_virtual_time(NULL);
    if ((strcmp(other_ne_id,HW_NETWORK_STER_ID) != 0))
    {
	    gmtime_r (&now, gm_time);
	    strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%T.000+0530", gm_time);
    }
    else
    {
        now = now - 600;
        sprintf(time_buf, "%d000", now);
    }
	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_START_DATE, (void *)time_buf, ebufp);
		
	sub_expiryp = (time_t *)PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_PURCHASE_END_T, 1, ebufp);
    if (sub_expiryp)
    {
        if (*sub_expiryp == 0)
        {
            sprintf(time_buf, "1893436200000");
        }
        else
        {
            sprintf(time_buf, "%d000", *sub_expiryp);
        }
	    PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_EXPIRYDATE, (void *)time_buf, ebufp);
    }
	//Add the Account Number
	if(agg_no && sizeof(agg_no) > 1)
	{
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_EXTERNAL_ID, (void *)agg_no, ebufp);
	}
	else
	{
		*error_codep = MSO_BB_MANDATORY_FIELD_CODE;
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_ACCOUNT_NO, 0, 0);
		goto cleanup;
	}
	//Add the AVP Array
	mso_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_AVP, 1, ebufp);
	//Add the Account Number Code and Value
	//Pawan:27-02-2015: Updated below to set the Account number instead of agr no.
	PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_CODE, "accountNumber", ebufp);
	PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_VALUE, (void *)agg_no, ebufp);
	
	//Add the AVP Array
	mso_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_AVP, 2, ebufp);
	//Add the IS1CLICK code and value
	PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_CODE, "IS1CLICK", ebufp);
	if((tech && *tech != 5 ) && *is1Click)
	{
		// Added by Muthu to handle noclick on 19th Oct 2015
		if (*is1Click == 1 )
		{
			PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_VALUE, "true", ebufp);
		}
		else
		{
			PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_VALUE, "noclick", ebufp);
		}
	}
	else{
		PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_VALUE, "false", ebufp);
	}
		
	//Add the AVP Array
	mso_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_AVP, 3, ebufp);
	//Add the Framed Net Mask code and value
	PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_CODE, "Framed-IP-Netmask", ebufp);
	PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_VALUE, "255.255.255.255", ebufp);
	
	//Add the AVP Array
	mso_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_AVP, 4, ebufp);
	//Add the CMTS Location
	PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_CODE, "CMTSLocation", ebufp);
	PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_VALUE, (void *)node_name, ebufp);

	//Add the AVP Array
	mso_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_AVP, 5, ebufp);
	//Add the Service City
	PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_CODE, "ServiceCity", ebufp);
	PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_VALUE, (void *)serv_city, ebufp);

	//Add the AVP Array
	mso_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_AVP, 6, ebufp);
	//Add the isSuspended code and value
	PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_CODE, "isSuspended", ebufp);
	PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_VALUE, "false", ebufp);

	//Add the AVP Array
	mso_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_AVP, 7, ebufp);
	//Add the isTAL code and value
	PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_CODE, "isTAL", ebufp);
	if(*isTAL){
		PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_VALUE, "true", ebufp);
	}
	else{
		PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_VALUE, "false", ebufp);
	}
	
	//Add the AVP Array
	mso_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_AVP, 8, ebufp);
	//Add the isSuspended code and value
	if (tech && *tech == 4){ //Added for FIBER, For FIBER, Idle-Timeout is 1 year
		PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_CODE, "Idle-Timeout", ebufp);
		PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_VALUE, "31556952", ebufp);
	}
	else{
		PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_CODE, "Idle-Timeout", ebufp);
		PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_VALUE, "1800", ebufp);
	}
	
	//Add the AVP Array
	mso_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_AVP, 9, ebufp);
	//Add the isSuspended code and value
	if (tech && *tech == 4){ //Added for FIBER, For FIBER, Session-Timeout is 2 years
		PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_CODE, "Session-Timeout", ebufp);
		PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_VALUE, "63113904", ebufp);
	}else{
		PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_CODE, "Session-Timeout", ebufp);
		PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_VALUE, "43200", ebufp);
	}
	
	acc_iflistp = PIN_FLIST_CREATE(ebufp);
	
	PIN_FLIST_FLD_SET(acc_iflistp, PIN_FLD_POID, ac_pdp, ebufp);
	acc_nameinfo_flistp = PIN_FLIST_ELEM_ADD(
		acc_iflistp, PIN_FLD_NAMEINFO, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(acc_nameinfo_flistp, PIN_FLD_EMAIL_ADDR, NULL, ebufp);
	acc_phones_flistp = PIN_FLIST_ELEM_ADD(
		acc_nameinfo_flistp, PIN_FLD_PHONES, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(acc_phones_flistp, PIN_FLD_PHONE, NULL, ebufp);
	PIN_FLIST_FLD_SET(acc_phones_flistp, PIN_FLD_TYPE, (void *)&typ, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read input flist", acc_iflistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, acc_iflistp, &acc_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read output flist", acc_oflistp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"fm_mso_prov_create_bb_action error: error reading account", ebufp);
		goto cleanup;
	}

	
	rec_id =0;
	cookie = NULL;
	
	while ( (acc_name_flistp = PIN_FLIST_ELEM_GET_NEXT (acc_oflistp,
                       PIN_FLD_NAMEINFO, &rec_id, 1,&cookie, ebufp)) != (pin_flist_t *)NULL ) 
        {
        	email = PIN_FLIST_FLD_GET(acc_name_flistp, PIN_FLD_EMAIL_ADDR, 0, ebufp);
        	if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			*error_codep = MSO_BB_EMAIL_MISSING;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
				"prepare_bb_activation_flist: Error",ebufp);
			PIN_ERRBUF_RESET(ebufp);
			goto cleanup;
		}
		rec_id1 =0;
		cookie1 = NULL;
        	while ( (acc_ph_flistp = PIN_FLIST_ELEM_GET_NEXT (acc_name_flistp,
                       PIN_FLD_PHONES, &rec_id1, 1,&cookie1, ebufp)) != (pin_flist_t *)NULL ) 
                {
                	ph_type = (int32 *)PIN_FLIST_FLD_GET(acc_ph_flistp, PIN_FLD_TYPE, 0, ebufp);
                	
                	if(*ph_type == 5)
                	{
                		ph_no = (char *)PIN_FLIST_FLD_GET(acc_ph_flistp, PIN_FLD_PHONE, 0, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					*error_codep = MSO_BB_PHONE_MISSING;
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
						"prepare_bb_activation_flist: Error",ebufp);
					PIN_ERRBUF_RESET(ebufp);
					goto cleanup;
				}
                		break;
                	}
                }
		if(!ph_no){
			*error_codep = MSO_BB_PHONE_MISSING;
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_BAD_VALUE, PIN_FLD_EMAIL_ADDR, 0, 0);
			goto cleanup;
		}
        	
	}	
	
	mso_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_AVP, 10, ebufp);
	PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_CODE, "EMAIL", ebufp);
	if(email && sizeof(email) > 1)
	{
		PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_VALUE, email, ebufp);
	}
	else
	{
		*error_codep = MSO_BB_EMAIL_MISSING;
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_EMAIL_ADDR, 0, 0);
		goto cleanup;
	}
	
	
	mso_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_AVP, 11, ebufp);
	//Add the isSuspended code and value
	PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_CODE, "PHONE", ebufp);
	if(ph_no && sizeof(ph_no) > 1)
	{
		PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_VALUE, ph_no, ebufp);
	}
	else
	{
		*error_codep = MSO_BB_PHONE_MISSING;
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_PHONE, 0, 0);
		goto cleanup;
	}
	
	mso_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_AVP, 12, ebufp);
	//Add the isSuspended code and value
	PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_CODE, "forceChangePassword", ebufp);
	// Added the condition of 2 for NoClick by Muthu - 19th Oct 2015
	if((tech && *tech == 5 ) || (*isTAL == 1 || *is1Click == 1 || *is1Click == 2 ))
	{
		PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_VALUE, "false", ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_VALUE, "true", ebufp);
	}
    if((other_ne_id && strcmp(other_ne_id,HW_NETWORK_STER_ID) == 0))
    {
        stl_rolep = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_ROLES, 0, ebufp);
        pwd_chng_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_AVP, 13, ebufp);
        PIN_FLIST_FLD_SET(pwd_chng_flistp, MSO_FLD_CODE, "PASSWORD CHECK", ebufp);
        if (stl_rolep && ((strstr(stl_rolep, "MEN-Dynamic") || strstr(stl_rolep, "GPON-Dynamic"))))
        {
            PIN_FLIST_FLD_SET(pwd_chng_flistp, MSO_FLD_VALUE, "true", ebufp);
        }
        else
        {
            PIN_FLIST_FLD_SET(pwd_chng_flistp, MSO_FLD_VALUE, "false", ebufp);
        }
    }

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_activation_flist device details return", *i_flistpp);

	createbalflistp = PIN_FLIST_ELEM_ADD(task_flistp, 
		MSO_FLD_CREATEBALANCE, 1, ebufp);
	PIN_FLIST_FLD_SET(createbalflistp, MSO_FLD_QUOTACODE, "DATA", ebufp);
	PIN_FLIST_FLD_SET(createbalflistp, MSO_FLD_CODE, quota_code, ebufp);
     if ((strcmp(other_ne_id,HW_NETWORK_STER_ID) == 0))
    {
        if(quota_code && strstr(quota_code,"FUPDR"))
        {
            no_days = "DAILY";
        }
        else
        {
            no_days= "BILLING_CYCLE";
        }
    }
    PIN_FLIST_FLD_SET(createbalflistp, PIN_FLD_VALUE, no_days, ebufp);

	if(quota_flag == 1 )
	{
		//Call the function to populate the create balance array
		PIN_FLIST_FLD_SET(*i_flistpp, PIN_FLD_OPCODE, &acticate_bb_srvc, ebufp );
		fm_mso_add_create_bal(ctxp, i_flistpp, ac_pdp, &createbalflistp, &plan_type, &initial_amount, srv_typ, error_codep, ebufp); 
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
				"prepare_docsis_qps_activation_flist: Error", ebufp);
			goto cleanup;
		}
		//PIN_FLIST_ELEM_COPY(createbalflistp, MSO_FLD_CREATEBALANCE,1,
		//	 task_flistp, MSO_FLD_CREATEBALANCE,1, ebufp);
	}
	else{
		
		
		//Pawan: Added below to add Start date in payload
		now = pin_virtual_time (NULL);
        if ((strcmp(other_ne_id,HW_NETWORK_STER_ID) != 0))
        {
		    gm_time = gmtime(&now);
		    strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%T.000+0530", gm_time);
        }
        else
        {
            sprintf(time_buf, "%d000", now);
        }
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Time :");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, time_buf);
		PIN_FLIST_FLD_SET(createbalflistp, MSO_FLD_STARTDATE, (void *)time_buf, ebufp);

		//Pawan: Added below to add Expiry date in payload
		mso_plan = (poid_t *)PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_PURCHASED_PLAN_OBJ, 0, ebufp);
		//expiry = fm_prov_get_expiry(ctxp, mso_plan, ac_pdp, srv_pdp, srv_typ, ebufp);
		expiry = fm_prov_get_expiry_from_calc_only_out(ctxp, *i_flistpp,srv_typ, ebufp); 
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			*error_codep = MSO_BB_INTERNAL_ERROR;
		     PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"error getting Balances", ebufp);
		     goto cleanup;
		}
		if(expiry){
			//gm_time = gmtime(&expiry);
			expiry = expiry -1;
            if ((strcmp(other_ne_id,HW_NETWORK_STER_ID) != 0))
            {
			    gm_time = localtime(&expiry);
			    strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%T.000+0530", gm_time);
            }
            else
            {
                sprintf(time_buf, "%d000", expiry);
            }
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Expiry Time :");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, time_buf);
			PIN_FLIST_FLD_SET(createbalflistp, MSO_FLD_EXPIRATIONDATE, (void *)time_buf, ebufp);
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Amount is:");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, unltd_quota);
		//Pawan: Changed the below filed from MSO_FLD_INITIAL_AMOUNT to MSO_FLD_INITIALAMOUNT
		PIN_FLIST_FLD_SET(createbalflistp, MSO_FLD_INITIALAMOUNT, (void *)unltd_quota, ebufp); 

	}

                        
cleanup:

//	PIN_FLIST_DESTROY_EX(&createbalflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acc_iflistp, NULL);
	if(acc_oflistp) PIN_FLIST_DESTROY_EX(&acc_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&plan_iflistp, NULL);
	if(plan_oflistp)PIN_FLIST_DESTROY_EX(&plan_oflistp, NULL);
	if(deviceflistp)PIN_FLIST_DESTROY_EX(&deviceflistp, NULL);
	if(ne_add) pin_free(ne_add);	
	if(f_name)
		pin_free(f_name);
	if(l_name)
		pin_free(l_name);
	if(acc_no)
		pin_free(acc_no);
//	if(!pbo_decimal_is_null(initial_amount, ebufp))
//		pbo_decimal_destroy(&initial_amount);
	return;
}



static void
prepare_docsis_bcc_flist(
	pcm_context_t			*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp,
	int				*error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*cablemodemflistp = NULL;
	pin_flist_t			*temp_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	char				*rec_name = NULL;
	char				*rec_descr = NULL;
	char				*node_name = NULL;
	char				*ne_add = NULL;
	int32              		*action_flag = NULL;
	int				*tech = NULL;
	char				*city = NULL;
	char				*new_name = NULL;
	char				*login = NULL;
	int32				*is_tal = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		return;
	}
	/******Add the Task Array for BBC*******************/
	task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, task_id, ebufp);
	task_id = task_id + 1;
//	//Record Name 
//	temp_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 1, 1, ebufp);
//	if(temp_flistp)
//		rec_name = (char *)PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_NAME, 0, ebufp);
//	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_RECORD_NAME, (void *)rec_name, ebufp);

	//Record DESCR
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	action_flag = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_FLAGS, 0, ebufp);
	is_tal = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_ISTAL_FLAG, 0, ebufp);

	/***added condition to pass the ACTIVATE BCC for change modem to TAL service***/
	if ( *action_flag ==  DOC_BB_ACTIVATION || ( *action_flag == DOC_BB_MODEM_CHANGE && *is_tal == 1 ))
	{
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "ACTIVATE_SUBSCRIBER_BCC", ebufp);
		//PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, (void *)"BCC-BANG", ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "UPDATE_SUBSCRIBER_BCC", ebufp);
		//PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, (void *)"BCC01", ebufp);
	}
	//Get the Network Node Name
	node_name = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_docsis_bcc_flist: Network Element Missing",ebufp);
		goto cleanup;
	}
	//Call the function to get the NE Address
	//fm_mso_get_network_node(ctxp, node_name, &ne_add, 2, error_codep, ebufp);
	city = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_CITY_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_switch_service_flist: Network Element Missing",ebufp);
		goto cleanup;
	}
	//Call the function to get the NE Address
	fm_mso_get_network_node(ctxp, node_name, city, &ne_add, 2, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_docsis_bcc_flist: Not able to get Network Element ID",ebufp);
		goto cleanup;
	}
	if(ne_add){
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, (void *)ne_add, ebufp);
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_docsis_bcc_flist: Error",ebufp);
		goto cleanup;
	}
	//cablemodemflistp = PIN_FLIST_CREATE(ebufp);
	cablemodemflistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_CABLE_MODEM, 1, ebufp);
//	if(*action_flag == DOC_BB_IP_CHANGE)
//	{
//		//Add the ip addres
//		fm_mso_get_ip(ctxp, &new_name, so_flistp, error_codep, ebufp);
//		if (PIN_ERRBUF_IS_ERR(ebufp))
//		{
//			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
//				"prepare_docsis_bcc_flist: Error getting IP",ebufp);
//			goto cleanup;
//		}
//		if(new_name){
//			PIN_FLIST_FLD_SET(cablemodemflistp, MSO_FLD_IPADDRESSES, (void *)new_name, ebufp);
//		}
//	}
	fm_mso_add_cable_modem(ctxp,i_flistpp, so_flistp, action_flag, &cablemodemflistp, error_codep, ebufp); 
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_docsis_bcc_flist: Error",ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_MSG(3, "RETURN!!!!");
	//PIN_FLIST_ELEM_COPY(cablemodemflistp, MSO_FLD_CABLE_MODEM,1, task_flistp, MSO_FLD_CABLE_MODEM,1, ebufp);
	/***added condition to pass the OLDER MAC  for change modem to NON-TAL service***/
	if(action_flag && *action_flag == DOC_BB_MODEM_CHANGE && *is_tal == 0 )
	{
		login = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_LOGIN, 0, ebufp);	
		if (!login)
        	{
                  	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        	  "prepare_docsis_bcc_flist: Manditory field login is missing",ebufp);
                 	 goto cleanup;
        	}
	
		PIN_FLIST_FLD_SET(cablemodemflistp, MSO_FLD_ID, login, ebufp);
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_docsis_bcc_flist: Error",ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"input flist", *i_flistpp);
cleanup:
	//PIN_FLIST_DESTROY_EX(&cablemodemflistp, NULL);
	if(ne_add)pin_free(ne_add);
	return;
}


//Function to set the network node
//static void
//fm_mso_get_network_node(
//	pcm_context_t			*ctxp,
//	char				*node_name,
//	char				*city,
//	char				**ne_add,
//	int				flag,
//	int				*error_codep,
//	pin_errbuf_t			*ebufp)
//{
//	pin_flist_t			*tmp_flistp = NULL;
//	pin_flist_t			*cache_flistp = NULL;
//	cm_cache_key_iddbstr_t		kids;
//        pin_cookie_t			cookie = NULL;
//        int32				rec_id = 0;
//	int32				err = PIN_ERR_NONE;
//	//int				*tec = 0;
//	//int				*srv_typ = (int *)3;
//	time_t				last_mod_t = 0;
//	time_t				now_t = 0;
//
//	/******************************************************
//        * If the cache is not enabled, short circuit right away
//        ******************************************************/
//	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Inside function fm_mso_get_network_node");
//        if ( mso_prov_bb_cmts_ptr == (cm_cache_t *)NULL ) {
//                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
//                "no config flist for /mso_cfg_cmts_master cached ");
//		*error_codep = MSO_BB_CMTS_MASTER_ERROR;
//                pin_set_err(ebufp, PIN_ERRLOC_CM,
//                                PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_FLD_CACHE_NAME, 0, 0);
//
//        }
//        /**************************************************
//         * See if the entry is in our data dictionary cache
//         * Prepare the cm_cache_key_iddbstr_t structure to search
//         **************************************************/
//        kids.id = 0;
//        kids.db = 0;
//        kids.str = "/mso_cfg_cmts_master";
//        cache_flistp = cm_cache_find_entry(mso_prov_bb_cmts_ptr, &kids, &err);
//        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
//                        "cm_cache_find_entry found flist", cache_flistp);
//        switch(err) {
//        case PIN_ERR_NONE:
//                break;
//        default:
//                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
//                        "mso_prov_bb_cmts_object from_cache: error "
//                        "accessing data dictionary cache.");
//			*error_codep = MSO_BB_CMTS_MASTER_ERROR;
//                pin_set_err(ebufp, PIN_ERRLOC_CM,
//                                PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_FLD_CACHE_NAME, 0, 0);
//                goto cleanup;
//        }
//	last_mod_t = *(time_t*)PIN_FLIST_FLD_GET(cache_flistp, PIN_FLD_MOD_T,0,ebufp);
//	now_t = pin_virtual_time((time_t *)NULL);
//	if(now_t  >= last_mod_t + refresh_pt_interval )
//	{
//		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"calling mso_cfg_cmts_master to refresh provisioning tag config");
//		//lock mutex
//		pthread_mutex_lock(&cmts_update_mutex);
//		fm_mso_prov_cmts_config_cache_update(ctxp, ebufp);
//		kids.id = 0;
//		kids.db = 0;
//		kids.str = "/mso_cfg_cmts_master";
//		//unlock mutex
//		pthread_mutex_unlock(&cmts_update_mutex);
//		
//		cache_flistp = cm_cache_find_entry(mso_prov_bb_cmts_ptr,&kids, &err);
//		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
//				"cm_cache_find_entry found flist updated", cache_flistp);
//		switch(err)
//		{
//			case PIN_ERR_NONE:
//				break;
//			default:
//				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
//					"mso_cfg_cmts_master from_cache: error "
//					"accessing data dictionary cache.");
//				pin_set_err(ebufp, PIN_ERRLOC_CM,
//						PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_FLD_CACHE_NAME, 0, 0);
//				goto cleanup;
//		}
//	}
//	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_network_node: Network Element to compare:");
//	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, node_name);
//	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "City");
//	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, city);
//	while ( (tmp_flistp = PIN_FLIST_ELEM_GET_NEXT (cache_flistp,
//		PIN_FLD_RESULTS, &rec_id, 1,&cookie, ebufp))!= (pin_flist_t *)NULL )
//	{
//		if (strcmp((char *)PIN_FLIST_FLD_GET(tmp_flistp, 
//			MSO_FLD_CMTS_ID, 0, ebufp ),node_name) == 0 
//			&& strcmp((char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 0, ebufp ),city) == 0)
//		{
//			//Check the flag
//			if(flag == 1)
//			{
//				//For QPS
//				*ne_add = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_OTHER_NE_ID, 0, ebufp);
//			}
//			else
//			{
//				//For BCC
//				*ne_add = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NE_ID, 0, ebufp);
//			}
//			if(PIN_ERRBUF_IS_ERR(ebufp)) {
//				*error_codep = MSO_BB_CMTS_MASTER_ERROR;
//				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_network_node: Error", ebufp);
//				PIN_FLIST_DESTROY_EX(&cache_flistp, NULL);
//				return;
//			}
//			break;
//		}
//
//	}
//	if(!*ne_add){
//		*error_codep = MSO_BB_CMTS_MASTER_ERROR;
//		PIN_ERRBUF_CLEAR(ebufp);
//		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "fm_mso_get_network_node: CMTS_ID invalid");
//		pin_set_err(ebufp, PIN_ERRLOC_FM,
//			PIN_ERRCLASS_SYSTEM_DETERMINATE,
//			PIN_ERR_BAD_VALUE , MSO_FLD_CMTS_ID, 0, 0);
//	}
//cleanup:	
//	if(PIN_ERRBUF_IS_ERR(ebufp)) {
//		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_network_node: Error", ebufp);
//		PIN_FLIST_DESTROY_EX(&cache_flistp, NULL);
//		return;
//	}
//	PIN_FLIST_DESTROY_EX(&cache_flistp, NULL);
//	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Returning from fm_mso_get_network_node");
//	return;
//}

static void
fm_mso_get_network_node(
	pcm_context_t		*ctxp,
	char				*node_name,
	char				*city,
	char				**ne_add,
	int				    flag,
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*search_cmts_flistp = NULL;
	pin_flist_t			*search_cmts_oflistp = NULL;
	pin_flist_t			*arg_flistp = NULL;
	pin_flist_t			*res_iflistp = NULL;
	pin_flist_t			*result_flistp = NULL;
	poid_t				*pdp = NULL;
	poid_t				*s_pdp = NULL;
	int32				s_flags = 0;
    int32			    database = 0;



	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_network_node: inputs:");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "\nNode:");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, node_name);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "\nCity:");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, city);

	/*
	0 PIN_FLD_POID                     POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS                    INT [0] 768
	0 PIN_FLD_TEMPLATE                 STR [0] "select X from /mso_cfg_cmts_master where F1 = V1 and F2 = V2 "
	0 PIN_FLD_RESULTS                  ARRAY [0] allocated 2, used 2
	1     MSO_FLD_OTHER_NE_ID          STR [0] NULL
	1     MSO_FLD_NE_ID                STR [0] NULL
	0 PIN_FLD_ARGS                     ARRAY [1] allocated 1, used 1
	1     MSO_FLD_CMTS_ID              STR [0] "UAT KAMLA" 0
	0 PIN_FLD_ARGS                     ARRAY [2] allocated 1, used 1
	1     PIN_FLD_CITY                 STR [0] "MUMBAI" 0
	*/
	
	search_cmts_flistp = PIN_FLIST_CREATE(ebufp);

	/***********************************************************
	 * assume db matches userid found in pin.conf
	 ***********************************************************/
	pdp = PCM_GET_USERID(ctxp);
	database = PIN_POID_GET_DB(pdp);

	s_pdp = PIN_POID_CREATE(database, "/search", -1, ebufp);
	
	PIN_FLIST_FLD_PUT(search_cmts_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_FLD_SET(search_cmts_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
	
	PIN_FLIST_FLD_SET(search_cmts_flistp, PIN_FLD_TEMPLATE,
	"select X from /mso_cfg_cmts_master where F1 = V1 and F2 = V2 " , ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(search_cmts_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_CMTS_ID, node_name, ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(search_cmts_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_CITY, city, ebufp);

	res_iflistp = PIN_FLIST_ELEM_ADD( search_cmts_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(res_iflistp, MSO_FLD_OTHER_NE_ID, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_iflistp, MSO_FLD_NE_ID, NULL, ebufp);


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Get cmts input flist", search_cmts_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_cmts_flistp, &search_cmts_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Get cmts output flist", search_cmts_oflistp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_INTERNAL_ERROR;
             PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "error getting last plan ", ebufp);
             goto cleanup;
	}	
	
	result_flistp = PIN_FLIST_ELEM_GET(search_cmts_oflistp,PIN_FLD_RESULTS,0,1,ebufp);
	if(result_flistp)
	{

		//Check the flag
		if(flag == 1)
		{
			//For QPS
			*ne_add = (char *)PIN_FLIST_FLD_TAKE(result_flistp, MSO_FLD_OTHER_NE_ID, 0, ebufp);
		}
		else
		{
			//For BCC
			*ne_add = (char *)PIN_FLIST_FLD_TAKE(result_flistp, MSO_FLD_NE_ID, 0, ebufp);
		}
	}
	else
	{
		*error_codep = MSO_BB_CMTS_MASTER_ERROR;
		PIN_ERRBUF_CLEAR(ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "fm_mso_get_network_node: CMTS_ID invalid");
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE , MSO_FLD_CMTS_ID, 0, 0);

		goto cleanup;
	}



	if(!*ne_add){
		*error_codep = MSO_BB_CMTS_MASTER_ERROR;
		PIN_ERRBUF_CLEAR(ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "fm_mso_get_network_node: CMTS_ID invalid");
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE , MSO_FLD_CMTS_ID, 0, 0);
	}
	
cleanup:
	PIN_FLIST_DESTROY_EX(&search_cmts_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_cmts_oflistp, NULL);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Returning from fm_mso_get_network_node");
	return;
}


static void
prepare_docsis_subscriberavps_flist(
	pcm_context_t		*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp,
	char				*act,
	int			    	*error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*mod_avp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	poid_t				*serv_poid = NULL;
	int32            	*action_flag = NULL;
	int32				*isTAL = 0;
	char				*node_name = NULL;
	char				*ne_add = NULL;
	char				*nw_id = NULL;
	char				*city = NULL;
	/* Changes for QPS to CPS Migration.  */
	pin_flist_t			*read_flistp = NULL;
	pin_flist_t			*read_o_flistp = NULL;
	pin_flist_t			*args_flistp = NULL;
	int32				curr_status = 0;

    pcm_context_t       *new_ctxp;
	char                *node_name_old = NULL;
	int32               *isTAL_old = 0;
	


	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"inside prepare_docsis_subscriberavps_flist");
	
	/******Add the Task Array for BBC*******************/
	task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, task_id, ebufp);
	task_id = task_id + 1;
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	isTAL = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_ISTAL_FLAG, 0, ebufp);
	serv_poid = PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_POID, 0, ebufp);
	node_name = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NE_ELEMENT_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_docsis_subscriberavps2_flist: Network Element Missing",ebufp);
		return;
	}

       /* Changes for QPS to CPS Migration.  */
	PCM_CONTEXT_OPEN(&new_ctxp, (pin_flist_t *)0, ebufp);
	//Call the function to get the NE Address
	//fm_mso_get_network_node(ctxp, node_name, &ne_add, 2, error_codep, ebufp);
	city = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_CITY_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_switch_service_flist: Network Element Missing",ebufp);
		return;
	}
	//Call the function to get the NE Address
	fm_mso_get_network_node(ctxp, node_name, city, &ne_add, 1, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_docsis_subscriberavps_flist: Error",ebufp);
		PCM_CONTEXT_CLOSE(new_ctxp, 0, ebufp);
		return;
	}
	if(ne_add){
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, (void *)ne_add, ebufp);
	}
	action_flag = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_FLAGS, 0, ebufp);
	//nw_id = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORKID, 0, ebufp);
//	if(*isTAL == 1)
//	{
//		fm_mso_get_login(ctxp, &nw_id, so_flistp, error_codep, ebufp);
//		if (PIN_ERRBUF_IS_ERR(ebufp))
//		{
//			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
//				"prepare_bb_flist: Error getting name",ebufp);
//			return;
//		}
//	}
	//else{

	// Pawan: Added below if condition to set the nw_id as static IP (login in input)  
	// in case of CMTTS change for TAL. Defect #1309. 
	if(*action_flag == DOC_BB_CMTS_CHANGE_T)
		nw_id = (char *)PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_LOGIN, 0, ebufp);
	else
		nw_id = (char *)PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_LOGIN, 0, ebufp);
	//}
	//if ( nw_id && strlen(nw_id) > 0)
	//{
	    //PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_NETWORKID, (void *)nw_id, ebufp);
	//}
//	else
//	{
//		*error_codep = MSO_BB_MANDATORY_FIELD_CODE;
//	    pin_set_err(ebufp, PIN_ERRLOC_FM,
//		PIN_ERRCLASS_SYSTEM_DETERMINATE,
//		    PIN_ERR_BAD_VALUE, PIN_FLD_USER_NAME, 0, 0);
//	    goto cleanup;
//	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NW_ID_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_docsis_subscriberavps2_flist: Network Element Missing",ebufp);
		PCM_CONTEXT_CLOSE(new_ctxp, 0, ebufp);
		return;
	}

	if ( *action_flag ==  DOC_BB_RENEW_AEXPIRY_PRE || *action_flag == DOC_BB_AUTO_RENEW_POST 
		|| *action_flag == DOC_BB_REACTIVATE_SERV || *action_flag == DOC_BB_UNHOLD_SERV)
	{
		//PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "RENEW_SUBSCRIBER_QPS", ebufp);
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "UPDATE_AVP_QPS", ebufp);
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORKID, (void *)nw_id, ebufp);
		mod_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_MOD_AVP, 1, ebufp);
		PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_CODE, "isSuspended", ebufp);

		/* Changes for QPS to CPS upgrade */
		// Added the below code for JIO-CPS Integration-ISP Enhancement
		if(other_ne_id != NULL)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Network ELement ID >> ");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, other_ne_id);		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "REACTIVATE_SERVICES_FIBER / REACTIVATE_SERVICES_DOCSIS / UNHOLD_SERVICES_DOCSIS / UNHOLD_SERVICES_FIBER / RENEW_SUBSCRIBER_DOCSIS / AUTORENEW_SUBSCRIBER_DOCSIS ");
	
			if( (strcmp(other_ne_id,HW_NETWORK_ELEMENT_ID) == 0 || strcmp(other_ne_id,JIO_NETWORK_ELEMENT_ID) == 0 ||
                        strcmp(other_ne_id, HW_NETWORK_STER_ID) == 0) 
			//if( (strcmp(other_ne_id,HW_NETWORK_ELEMENT_ID) == 0) 
				  && (strcmp((char *)act,"REACTIVATE_SERVICES_FIBER") == 0 || 
					  strcmp((char *)act,"REACTIVATE_SERVICES_DOCSIS") == 0 ||
					  strcmp((char *)act,"UNHOLD_SERVICES_DOCSIS") == 0 ||
					  strcmp((char *)act,"UNHOLD_SERVICES_FIBER")  == 0 ||
					  strcmp((char *)act,"RENEW_SUBSCRIBER_DOCSIS")  == 0 ||
					  strcmp((char *)act,"AUTORENEW_SUBSCRIBER_DOCSIS") == 0
				  ) ) 
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Action >> ");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, (char *)act);					
			
				/* Read old service status value from service_bb_info_t */
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_AVP_NEW_VALUE, "false", ebufp);

				read_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_POID, serv_poid, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read BB service obj input flist",read_flistp);
				PCM_OP(new_ctxp, PCM_OP_READ_OBJ, 0, read_flistp, &read_o_flistp, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
				  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in reading service obj ");
				  pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_BAD_VALUE, PIN_POID_GET_ID(serv_poid), 0, 0);
				}

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read BB service obj output flist",read_o_flistp);

				if(read_o_flistp != NULL)
				{
					args_flistp = PIN_FLIST_SUBSTR_GET(read_o_flistp, PIN_FLD_TELCO_FEATURES, 0, ebufp );
					curr_status = *(int32 *)PIN_FLIST_FLD_GET(args_flistp,PIN_FLD_STATUS,0,ebufp );
				}

				if( curr_status >= 4 && curr_status <= 7 )
				{
					PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_VALUE , "true" , ebufp); 
				}
				else 
					PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_VALUE , "false" , ebufp);
			}
			else
			{
				//Add the isSuspended code and value
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_VALUE, "false", ebufp);
			}
		}
	}
	else if(*action_flag == DOC_BB_CMTS_CHANGE_T || *action_flag == DOC_BB_CMTS_CHANGE_NT)
	{
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "UPDATE_AVP_QPS", ebufp);
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORKID, (void *)nw_id, ebufp);
		mod_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_MOD_AVP, 1, ebufp);
		PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_CODE, "CMTSLocation", ebufp);

        //Added the below code for JIO-CPS Integration-ISP Enhancement
		if(other_ne_id != NULL)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Network ELement ID >> ");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, other_ne_id);		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CHANGE_CMTS_NONTAL_DOCSIS or CHANGE_CMTS_TAL_DOCSIS ");
			
			if( (strcmp(other_ne_id,HW_NETWORK_ELEMENT_ID) == 0 || strcmp(other_ne_id,JIO_NETWORK_ELEMENT_ID) == 0 ||
                        strcmp(other_ne_id, HW_NETWORK_STER_ID) == 0) 
				//if((strcmp(other_ne_id,HW_NETWORK_ELEMENT_ID) == 0) 
				&& ( strcmp((char *)act,"CHANGE_CMTS_NONTAL_DOCSIS") == 0 || 
					 strcmp((char *)act,"CHANGE_CMTS_TAL_DOCSIS") == 0) )
			{
				/* Read old CMTS_ID value */

				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Action >> ");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, (char *)act);		

				read_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_POID, serv_poid, ebufp);
				read_o_flistp = NULL;
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read BB service obj input flist",read_flistp);
				PCM_OP(new_ctxp, PCM_OP_READ_OBJ, 0, read_flistp, &read_o_flistp, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in reading service obj ");
					pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_BAD_VALUE, PIN_POID_GET_ID(serv_poid), 0, 0);
					return;
				}

				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Read BB service obj output flist");
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read BB service obj output flist",read_o_flistp);

				if(read_o_flistp != NULL )
				{
					args_flistp = PIN_FLIST_SUBSTR_GET(read_o_flistp, MSO_FLD_BB_INFO, 0, ebufp );
					if(args_flistp != NULL)
							node_name_old = (char *)PIN_FLIST_FLD_GET(args_flistp,MSO_FLD_NETWORK_ELEMENT,0,ebufp );

					PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_AVP_NEW_VALUE, node_name, ebufp);
					PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_VALUE, node_name_old, ebufp);
				}
			
			} // End of CPS Upgrade change
			else
			{
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_VALUE, node_name, ebufp);
			}
		}
	}
	else
	{
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "UPDATE_AVP_QPS", ebufp);
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORKID, nw_id, ebufp);
		mod_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_MOD_AVP, 1, ebufp);
		//Add the isSuspended code and value
		PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_CODE, "isSuspended", ebufp);
		
		// Changes for QPS to CPS upgrade
		// Added the below code for JIO-CPS Integration-ISP Enhancement
		if(other_ne_id != NULL)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Network ELement ID >> ");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, other_ne_id);		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "DEACTIVATE_SUBSCRIBER_DOCSIS / DEACTIVATE_SUBSCRIBER_FIBER / HOLD_SUBSCRIBER_DOCISIS / SUSPEND_SUBSCRIBER_DOCSIS / SUSPEND_SUBSCRIBER_FIBER / HOLD_SUBSCRIBER_FIBER ");
			
			if( (strcmp(other_ne_id,HW_NETWORK_ELEMENT_ID) == 0 || strcmp(other_ne_id,JIO_NETWORK_ELEMENT_ID) == 0 ||
                        strcmp(other_ne_id, HW_NETWORK_STER_ID) == 0) 
			//if((strcmp(other_ne_id,HW_NETWORK_ELEMENT_ID) == 0) 
					&& ((strcmp((char *)act,"DEACTIVATE_SUBSCRIBER_DOCSIS") == 0) ||
						(strcmp((char *)act,"DEACTIVATE_SUBSCRIBER_FIBER") == 0) ||
						(strcmp((char *)act,"HOLD_SUBSCRIBER_DOCISIS") == 0) || 
						(strcmp((char *)act,"SUSPEND_SUBSCRIBER_DOCSIS") == 0) ||
						(strcmp((char *)act,"SUSPEND_SUBSCRIBER_FIBER") == 0) ||
						(strcmp((char *)act,"HOLD_SUBSCRIBER_FIBER") == 0)
					   ) )
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Action >> ");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, (char *)act);			
			
				/* Read old service status value */
				
				read_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_POID, serv_poid, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read BB service obj input flist",read_flistp);
				PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_flistp, &read_o_flistp, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
				  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in reading service obj ");
				  pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_BAD_VALUE, PIN_POID_GET_ID(serv_poid), 0, 0);
				}

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read BB service obj output flist",read_o_flistp);

				if(read_o_flistp && read_o_flistp != NULL)
				{
					args_flistp = PIN_FLIST_SUBSTR_GET(read_o_flistp, PIN_FLD_TELCO_FEATURES, 0, ebufp );
					curr_status = *(int32 *)PIN_FLIST_FLD_GET(args_flistp,PIN_FLD_STATUS,0,ebufp );
				}

				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_AVP_NEW_VALUE, "true", ebufp);

				if( curr_status >= 4 && curr_status <= 7 )
				{
					PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_VALUE , "true" , ebufp); 
				}
				else 
					PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_VALUE , "false" , ebufp);
			}
			else
			{
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_VALUE, "true", ebufp);
			}
		}
	}

	PCM_CONTEXT_CLOSE(new_ctxp, 0, ebufp);	

CLEANUP:	
	PIN_FLIST_DESTROY_EX(&read_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&read_o_flistp, NULL);

	/******Add the Subscriber Array*******************/
	return;
}



static void
prepare_docsis_subscriberavps2_flist(
	pcm_context_t		*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp,
	char				*act,
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*mod_avp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	poid_t				*serv_poid = NULL;
	char				*node_name = NULL;
	char				*ne_add = NULL;
	int				    *isTAL = NULL;
	int				    *isClick = NULL;
	int				    *tech = NULL;
	char				*nw_id = NULL;
	int				    *flag = NULL;
	char				*city = NULL;
    char                *stl_rolep = NULL;
    char                *passwd  = NULL;
	int32              	*action_flag = NULL;

        /* Changes for QPS to CPS Migration.  */
    pcm_context_t                   *new_ctxp;
    char                            *node_name_old = NULL;
	pin_flist_t                     *read_flistp = NULL;
    pin_flist_t                     *read_o_flistp = NULL;
	pin_flist_t                     *args_flistp = NULL;
	int                             *isClick_old = NULL;
    int                             *isTAL_old = NULL;
	int			                 	*technology_old = NULL;
	FILE			             	*filep = NULL;




	
	/******Add the Task Array for BBC*******************/
	task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, task_id, ebufp);
	task_id = task_id + 1;
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	serv_poid = PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_POID, 0, ebufp);
	isTAL = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_ISTAL_FLAG, 0, ebufp);
	isClick = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_IS1CLICK_FLAG, 0, ebufp);
	node_name = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
	tech = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NE_ELEMENT_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_docsis_subscriberavps2_flist: Network Element Missing",ebufp);
		goto cleanup;
	}
	//Call the function to get the NE Address
	//fm_mso_get_network_node(ctxp, node_name, &ne_add, 2, error_codep, ebufp);
	city = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_CITY_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_switch_service_flist: Network Element Missing",ebufp);
		goto cleanup;
	}
	//Call the function to get the NE Address
	fm_mso_get_network_node(ctxp, node_name, city, &ne_add, 1, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_docsis_subscriberavps2_flist: Error",ebufp);
		goto cleanup;
	}
	if(ne_add){
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, (void *)ne_add, ebufp);
	}
//	if(*isTAL == 1)
//	{
//		fm_mso_get_login(ctxp, &nw_id, so_flistp, error_codep, ebufp);
//		if (PIN_ERRBUF_IS_ERR(ebufp))
//		{
//			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
//				"prepare_bb_flist: Error getting name",ebufp);
//			goto cleanup;
//		}
//	}
	//else{
	nw_id = (char *)PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_LOGIN, 0, ebufp);
	//}
	if ( nw_id && strlen(nw_id) > 0)
	{
	    PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORKID, (void *)nw_id, ebufp);
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NW_ID_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_docsis_subscriberavps2_flist: Network ID Missing",ebufp);
		return;
	}
	action_flag = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_FLAGS, 0, ebufp);
	/*	
	if ( *action_flag ==  DOC_BB_CAM_IT || *action_flag == DOC_BB_CAM_PT 
		|| *action_flag == DOC_BB_CAM_IP || *action_flag == DOC_BB_CAM_PI)
	{
		//PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "CHANGE_SUBSCRIBERAVPS_QPS", ebufp);
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "UPDATE_AVP_QPS", ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "UPDATE_AVP_QPS", ebufp);
	}*/

	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "UPDATE_AVP_QPS", ebufp);
	
	
	/******Add the Subscriber Array*******************/


        mod_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_MOD_AVP, 1, ebufp);


	/* Changes for  QPS to CPS Migration. Retrieve  old plan using an alternate connection. */
	// Added the below code for JIO-CPS Integration-ISP Enhancement
	if(other_ne_id != NULL)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Network ELement ID >> ");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, other_ne_id);		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CHANGE_AUTH_DOCSIS / CHANGE_AUTH_TO_CLICK_DOCSIS / CHANGE_AUTH_TO_TAL_DOCSIS / CHANGE_AUTH_TO_PORTAL_DOCSIS");

		if( ( strcmp(other_ne_id,HW_NETWORK_ELEMENT_ID) == 0 || strcmp(other_ne_id,JIO_NETWORK_ELEMENT_ID) == 0 ||
                    strcmp(other_ne_id, HW_NETWORK_STER_ID) == 0)
		           //if((strcmp(other_ne_id,HW_NETWORK_ELEMENT_ID) == 0) 
				&& ((strcmp((char *)act,"CHANGE_AUTH_DOCSIS") == 0) ||
					strcmp((char *)act,"CHANGE_AUTH_TO_CLICK_DOCSIS") == 0 ||
					strcmp((char *)act,"CHANGE_AUTH_TO_TAL_DOCSIS") == 0 ||	
					strcmp((char *)act,"CHANGE_AUTH_TO_PORTAL_DOCSIS") == 0
			 ) )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Action >> ");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, (char *)act);	
			
		    /* Changes for QPS to CPS Migration.  */
			PCM_CONTEXT_OPEN(&new_ctxp, (pin_flist_t *)0, ebufp);

			/* Read old service status value from service_bb_info_t */

			PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_AVP_NEW_VALUE, "false", ebufp);

			read_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_POID, serv_poid, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read BB service obj input flist",read_flistp);
			PCM_OP(new_ctxp, PCM_OP_READ_OBJ, 0, read_flistp, &read_o_flistp, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in reading service obj ");
				pin_set_err(ebufp, PIN_ERRLOC_FM,
									PIN_ERRCLASS_SYSTEM_DETERMINATE,
									PIN_ERR_BAD_VALUE, PIN_POID_GET_ID(serv_poid), 0, 0);
			}
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read BB service obj output flist",read_o_flistp);

			if(read_o_flistp && read_o_flistp != NULL)
                 args_flistp = PIN_FLIST_SUBSTR_GET(read_o_flistp, MSO_FLD_BB_INFO, 0, ebufp );

			if(args_flistp && args_flistp != NULL)
			{
				isTAL_old = (int *)PIN_FLIST_FLD_GET(args_flistp, MSO_FLD_ISTAL_FLAG, 0, ebufp);
				isClick_old = (int *)PIN_FLIST_FLD_GET(args_flistp, MSO_FLD_IS1CLICK_FLAG, 0, ebufp);
				technology_old = (int *)PIN_FLIST_FLD_GET(args_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp);
			}

			if(*isClick_old == 1)
			{ 
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_VALUE, "true", ebufp); 
			}
			else if (*isClick_old == 2)
			{ 
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_VALUE, "noclick", ebufp);
			}
			else 
			{
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_VALUE, "false", ebufp);
			}
		
			// FOR GPON, IS1CLICK is always set to FALSE
			if((tech && *tech == 5) || *isClick == 0)
			{
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_CODE, "IS1CLICK", ebufp);
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_AVP_NEW_VALUE, "false", ebufp);
			}
			else if(*isClick == 1)
			{
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_CODE, "IS1CLICK", ebufp);
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_AVP_NEW_VALUE, "true", ebufp);
			}
			else if(*isClick == 2)
			{
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_CODE, "IS1CLICK", ebufp);
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_AVP_NEW_VALUE, "noclick", ebufp);
			}

	        mod_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_MOD_AVP, 2, ebufp);

			if(*isTAL == 1)
			{
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_CODE, "isTAL", ebufp);
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_AVP_NEW_VALUE, "true", ebufp);
			}
			else
			{
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_CODE, "isTAL", ebufp);
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_AVP_NEW_VALUE, "false", ebufp);
			}

			if(*isTAL_old == 1)
			{
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_VALUE, "true", ebufp);
			}
			else
			{
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_VALUE, "false", ebufp);
			}

			//Kunal - Changes to reset forcechangepassword
			mod_avp = NULL;

			if (tech && *tech == 5)
			{
				mod_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_MOD_AVP, 3, ebufp);
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_CODE, "forceChangePassword", ebufp);
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_AVP_NEW_VALUE, "false", ebufp);
			}
			else if ( *action_flag ==  DOC_BB_CAM_PI || *action_flag == DOC_BB_CAM_PN
						|| *action_flag == DOC_BB_CAM_PT )
			{
				mod_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_MOD_AVP, 3, ebufp);
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_CODE, "forceChangePassword", ebufp);
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_AVP_NEW_VALUE, "false", ebufp);

			}
			else if ( *action_flag == DOC_BB_CAM_IP || *action_flag == DOC_BB_CAM_NP )
			{
				mod_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_MOD_AVP, 3, ebufp);
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_CODE, "forceChangePassword", ebufp);
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_AVP_NEW_VALUE, "true", ebufp);
			}

			if(technology_old != NULL && *technology_old == 5) {
				if(mod_avp != NULL)	
									PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_VALUE, "false", ebufp);	
			}
			else if ( isTAL_old != NULL && isClick_old != NULL && (*action_flag == DOC_BB_CAM_IP || *action_flag == DOC_BB_CAM_NP) && *isTAL_old != 1 && *isClick_old == 0 )
			{
								   if(mod_avp != NULL)
									PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_VALUE, "true", ebufp);
			}
			else 
			{
				   if(mod_avp != NULL)
									PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_VALUE, "false", ebufp);
			}
			PIN_ERRBUF_CLEAR(ebufp);


			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Checking if Force change password value is the same");
			// if AVP_NEW_VALUE and AVP_VALUE are the same , do not send the AVP
			if(mod_avp != NULL && PIN_FLIST_FLD_GET(mod_avp, MSO_FLD_AVP_NEW_VALUE, 0, ebufp) != NULL &&
				PIN_FLIST_FLD_GET(mod_avp, MSO_FLD_VALUE, 0, ebufp) != NULL)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Checking if Force change password value is the same inside if");
				
				if(strcmp((char *)PIN_FLIST_FLD_GET(mod_avp, MSO_FLD_AVP_NEW_VALUE, 0, ebufp),(char *)PIN_FLIST_FLD_GET(mod_avp, MSO_FLD_VALUE, 0, ebufp)) == 0 )
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Checking if Force change password value is the same . Before drop");
					PIN_FLIST_ELEM_DROP(task_flistp, MSO_FLD_MOD_AVP, 3, ebufp);
				}
			}
			PIN_ERRBUF_CLEAR(ebufp);

			PCM_CONTEXT_CLOSE(new_ctxp, 0, ebufp);
		}
		else 
		{	
			// FOR GPON, IS1CLICK is always set to FALSE
			if((tech && *tech == 5) || *isClick == 0)
			{
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_CODE, "IS1CLICK", ebufp);
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_VALUE, "false", ebufp);
			}
			else if(*isClick == 1)
			{
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_CODE, "IS1CLICK", ebufp);
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_VALUE, "true", ebufp);
			}
			else if(*isClick == 2)
			{
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_CODE, "IS1CLICK", ebufp);
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_VALUE, "noclick", ebufp);
			}

			mod_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_MOD_AVP, 2, ebufp);

			if(*isTAL == 1)
			{
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_CODE, "isTAL", ebufp);
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_VALUE, "true", ebufp);
			}
			else
			{
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_CODE, "isTAL", ebufp);
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_VALUE, "false", ebufp);
			}

			//Kunal - Changes to reset forcechangepassword
			if (tech && *tech == 5)
			{
				mod_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_MOD_AVP, 3, ebufp);
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_CODE, "forceChangePassword", ebufp);
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_VALUE, "false", ebufp);
			}
			else if ( *action_flag ==  DOC_BB_CAM_PI || *action_flag == DOC_BB_CAM_PN 
						|| *action_flag == DOC_BB_CAM_PT )
			{
				mod_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_MOD_AVP, 3, ebufp);
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_CODE, "forceChangePassword", ebufp);
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_VALUE, "false", ebufp);

			}
			else if ( *action_flag == DOC_BB_CAM_IP || *action_flag == DOC_BB_CAM_NP )
			{
				mod_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_MOD_AVP, 3, ebufp);
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_CODE, "forceChangePassword", ebufp);
				PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_VALUE, "true", ebufp);
			}
	    }
	}
    if(other_ne_id && tmp_flistp &&(strcmp(other_ne_id,HW_NETWORK_STER_ID) == 0))
    {
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"temporary flist ", tmp_flistp);
        stl_rolep = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_ROLES, 1, ebufp);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, stl_rolep);
        passwd = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_PASSWORD, 0, ebufp);
        mod_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_MOD_AVP, 3, ebufp);
        PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_CODE, "PASSWORD CHECK", ebufp);
        if (stl_rolep && ((strstr(stl_rolep, "MEN-Dynamic") || strstr(stl_rolep, "GPON-Dynamic"))))
        {
            PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_AVP_NEW_VALUE, "true", ebufp);
            mod_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_MOD_AVP, 4, ebufp);
            PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_CODE, "PASSWORD", ebufp);
            PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_AVP_NEW_VALUE, (void *)passwd, ebufp);
        }
        else
        {
            PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_AVP_NEW_VALUE, "false", ebufp);
        }
     }

/* Commented because we need to send both flag in change auth - ASAP requwsted for this to handle few scenario in network API		
	if(*action_flag == DOC_BB_CAM_IP || *action_flag == DOC_BB_CAM_PI){
		mod_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_MOD_AVP, 1, ebufp);
		flag = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_IS1CLICK_FLAG, 0, ebufp);
		if(*flag == 1){
			PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_CODE, "IS1CLICK", ebufp);
			PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_VALUE, "true", ebufp);
		}
		else{
			PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_CODE, "IS1CLICK", ebufp);
			PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_VALUE, "false", ebufp);
		}
		return;

	}
	else if( *action_flag ==  DOC_BB_CAM_IT || *action_flag == DOC_BB_CAM_PT){
		mod_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_MOD_AVP, 1, ebufp);
		flag = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_ISTAL_FLAG, 0, ebufp);
		if(*flag == 1){
			PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_CODE, "isTal", ebufp);
			PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_VALUE, "true", ebufp);
		}
		else{
			PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_CODE, "isTal", ebufp);
			PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_VALUE, "false", ebufp);
		}
		return;
	}
	//PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORKID, nw_id, ebufp);
	mod_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_MOD_AVP, 1, ebufp);
	flag = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_ISTAL_FLAG, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_TAL_FLAG_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_docsis_subscriberavps2_flist: Error",ebufp);
		goto cleanup;
	}
	//Add the isSuspended code and value
	if(*flag == 1){
		PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_CODE, "isTal", ebufp);
		PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_VALUE, "true", ebufp);
	}
	else{
		PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_CODE, "isTal", ebufp);
		PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_VALUE, "false", ebufp);
	}
	flag = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_IS1CLICK_FLAG, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_1CLICK_FLAG_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_docsis_subscriberavps2_flist: Error",ebufp);
		goto cleanup;
	}
	mod_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_MOD_AVP, 2, ebufp);
	//Add the isSuspended code and value
	if(*flag == 1){
		PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_CODE, "IS1CLICK", ebufp);
		PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_VALUE, "true", ebufp);
	}
	else{
		PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_CODE, "IS1CLICK", ebufp);
		PIN_FLIST_FLD_SET(mod_avp, MSO_FLD_VALUE, "false", ebufp);
	}*/
cleanup:
	return;
	PIN_FLIST_DESTROY_EX(&read_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&read_o_flistp, NULL);

}


static void
delete_cred_qps_flist(
	pcm_context_t		*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp,
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*tmp_flistp =  NULL;
	char				*node_name = NULL;
	char				*ne_add = NULL;
	char				*nw_id = NULL;
	int				    *flag = NULL;
	int				    *isTAL = 0;
	char				*city = NULL;
	poid_t				*serv_poid = NULL;
	int				    *act = 0;

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"delete_cred_qps_flist: Error in beginning",ebufp);
	}
	/******Add the Task Array for BBC*******************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"delete_cred_qps_flist: input begin", *i_flistpp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"delete_cred_qps_flist: so_flistp begin", so_flistp);

	task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, task_id, ebufp);
	act = (int *)PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_FLAGS, 0, ebufp);
	//task_id = task_id + 1;
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	isTAL = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_ISTAL_FLAG, 0, ebufp);
	node_name = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
	serv_poid = PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_POID, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NE_ELEMENT_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"delete_cred_qps_flist: Network Element Missing",ebufp);
		goto cleanup;
	}
	//Call the function to get the NE Address
	//fm_mso_get_network_node(ctxp, node_name, &ne_add, 2, error_codep, ebufp);
	city = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_CITY_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"delete_cred_qps_flist: PIN_FLD_CITY Missing",ebufp);
		goto cleanup;
	}
	//Call the function to get the NE Address
	fm_mso_get_network_node(ctxp, node_name, city, &ne_add, 1, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"delete_cred_qps_flist: Error",ebufp);
		goto cleanup;
	}
	if(ne_add){
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, (void *)ne_add, ebufp);
	}
	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "DELETE_CREDENTIAL_QPS", ebufp);
	/******Add the Subscriber Array*******************/
	//nw_id = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORKID, 0, ebufp);
	//if(*isTAL == 1)
	//{
	if(*act == GPON_MAC_CHANGE){//This is to delete GPON MAC request
		nw_id = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_LOGIN, 1, ebufp);
	}
	else if(*act != DOC_BB_CHANGE_PASS){
		fm_mso_get_ip(ctxp, &nw_id, *i_flistpp, so_flistp, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"delete_cred_qps_flist: Error getting name",ebufp);
			goto cleanup;
		}
	}
	//}
	else{
		nw_id = (char *)PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_LOGIN, 0, ebufp);
	}
	if ( nw_id && strlen(nw_id) > 0)
	{
		if(*act == DOC_BB_CHANGE_PASS){
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_OLD_NETWORKID, (void *)nw_id, ebufp);
		} else {
	    		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORKID, (void *)nw_id, ebufp);
		}
	}
	else
	{
		*error_codep = MSO_BB_MANDATORY_FIELD_CODE;
	    pin_set_err(ebufp, PIN_ERRLOC_FM,
		PIN_ERRCLASS_SYSTEM_DETERMINATE,
		    PIN_ERR_BAD_VALUE, PIN_FLD_USER_NAME, 0, 0);
	    goto cleanup;
	}

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NW_ID_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"delete_cred_qps_flist: Network ID Missing",ebufp);
		return;
	}
	//PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORKID, (void *)nw_id, ebufp);
cleanup:
	return;
}

static void
add_cred_qps_flist(
	pcm_context_t		*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp,
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*cred_flistp = NULL;
	pin_flist_t			*deviceflistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*temp_flistp = NULL;
	pin_flist_t			*ip_flistp = NULL;
    pin_flist_t         *plan_flistp = NULL;
	poid_t				*serv_poid = NULL;
	char				*user_id = NULL;
	char				*node_name = NULL;
	char				*ne_add = NULL;
	char				*nw_id = NULL;
	char				param[20] = {'\0'};
	char				*ip_add = NULL;
	char				*dev_typ = NULL;
	char				*ip_add_list = NULL;
	char				*mso_ip_add = NULL;
	char				*mac_id = NULL;
	char				*pass = NULL;
	char				*city = NULL;
	char				*login = NULL;
    char                *str_rolep = NULL;
	int				    *tal = 0;
	int				    flag = 0;
	int32				rec_id = 0;
	int32				rec_id1 = 0;
	int				    *act = 0;
	int				    ip_count = 0;
	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie1 = NULL;
	
	int				    cred_arrayid = 1;	
	char				*token = NULL;
	/******Add the Task Array for BBC*******************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"add_cred_qps_flist:input begin",*i_flistpp);

	//task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, (task_id+1), ebufp);
	//task_id = task_id + 1;
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	serv_poid = (poid_t *)PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_POID, 0, ebufp);
	act = (int *)PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_FLAGS, 0, ebufp);
	// To keep old change intact, just adding taks_id as 1 in case ADD_MULTIPLE_IP action
	if (*act == FIBER_BB_ADD_IP){
		task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, task_id, ebufp);
		task_id = task_id + 1;
	}
	else{
		task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, (task_id+1), ebufp);
	}
    str_rolep = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_ROLES, 0, ebufp);
    PIN_ERR_LOG_MSG(3, str_rolep);
	node_name = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NE_ELEMENT_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"add_cred_qps_flist: Network Element Missing",ebufp);
		goto cleanup;
	}
	//Call the function to get the NE Address
	//fm_mso_get_network_node(ctxp, node_name, &ne_add, 2, error_codep, ebufp);
	city = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_CITY_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"add_cred_qps_flist: Network Element Missing",ebufp);
		goto cleanup;
	}
	//Call the function to get the NE Address
	fm_mso_get_network_node(ctxp, node_name, city, &ne_add, 1, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"add_cred_qps_flist: fm_mso_get_network_node return Error",ebufp);
		goto cleanup;
	}
	if(ne_add){
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, (void *)ne_add, ebufp);
	}
	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "ADD_CREDENTIAL_QPS", ebufp);
	//nw_id = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORKID, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NW_ID_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"add_cred_qps_flist: Network ID Missing",ebufp);
		return;
	}
	/******Add the Subscriber Array*******************/
    strcpy(param,"/device/ip/static");
    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "check issue han1: ");
    /* get the ip/static device if any */
    fm_mso_search_devices(ctxp, param, serv_poid, &deviceflistp, error_codep, ebufp);
    if (PIN_ERRBUF_IS_ERR(ebufp))
            {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                    "add_cred_qps_flist:fm_mso_search_devices Error",ebufp);
                goto cleanup;
            }
	tal = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_ISTAL_FLAG, 0, ebufp);
	//cred_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_CREDENTIALS, 1, ebufp);
	//cred_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_CREDENTIALS, cred_arrayid, ebufp);
	if(*act == DOC_BB_CAM_IT || *act == DOC_BB_CAM_PT || *act == DOC_BB_CAM_NT || *act == FIBER_BB_ADD_IP)  //Kunal: For NoClick Change
	{	
		fm_mso_get_login(ctxp, *i_flistpp, &user_id, so_flistp, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"add_cred_qps_flist: fm_mso_get_login Error getting name",ebufp);
			goto cleanup;
		}

        /****** Pavan Bellala 05-11-2015 ****************************
        QPS IP address Structure change

        OLD STRUCTURE (for two IPs):-                                      NEW STRUCTURE:
        ----------------------------                                       --------------
    <MSO_FLD_CREDENTIALS elem="1">                                        <MSO_FLD_CREDENTIALS elem="1">
    <MSO_FLD_NETWORKID>Priya11310244</MSO_FLD_NETWORKID>                    <MSO_FLD_NETWORKID>Priya11310244</MSO_FLD_NETWORKID>
        <MSO_FLD_PASSWORD>BroadBand@123</MSO_FLD_PASSWORD>                  <MSO_FLD_PASSWORD>BroadBand@123</MSO_FLD_PASSWORD>
        <DESCR>Username/Password</DESCR>                                    <DESCR>Username/Password</DESCR>
    </MSO_FLD_CREDENTIALS>                                                </MSO_FLD_CREDENTIALS>
    <MSO_FLD_CREDENTIALS elem="2">                                          <MSO_FLD_CREDENTIALS elem="2">
        <MSO_FLD_NETWORKID>60.243.6.237,60.243.6.251</MSO_FLD_NETWORKID>    <MSO_FLD_NETWORKID>60.243.6.237</MSO_FLD_NETWORKID>
        <DESCR>IP Address</DESCR>                                           <DESCR>IP Address</DESCR>
    </MSO_FLD_CREDENTIALS>                                                </MSO_FLD_CREDENTIALS>
    <MSO_FLD_CREDENTIALS elem="3">                                          <MSO_FLD_CREDENTIALS elem="3">
        <MSO_FLD_NETWORKID>FC:4A:E9:0B:D7:9A</MSO_FLD_NETWORKID>            <MSO_FLD_NETWORKID>60.243.6.251</MSO_FLD_NETWORKID>
        <DESCR>MAC Address</DESCR>                                          <DESCR>IP Address</DESCR>
    </MSO_FLD_CREDENTIALS>                                                </MSO_FLD_CREDENTIALS>
                                                                            <MSO_FLD_CREDENTIALS elem="4">
                                                                            <MSO_FLD_NETWORKID>FC:4A:E9:0B:D7:9A</MSO_FLD_NETWORKID>
                                                                            <DESCR>MAC Address</DESCR>
                                                                          </MSO_FLD_CREDENTIALS>
        ************************************************************/
		/*****
		PIN_FLIST_FLD_SET(cred_flistp, MSO_FLD_NETWORKID, (void *)user_id, ebufp);
		PIN_FLIST_FLD_SET(cred_flistp, PIN_FLD_DESCR, "IP Address", ebufp);
		login = PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_LOGIN, 0, ebufp);
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORKID, login, ebufp);
		******/
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"add_cred_qps_flist: SREEKANTH IPs");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,user_id);
        if(str_rolep && (!strstr(str_rolep, "Leaseline")))
        {
            cookie1 = NULL;
            rec_id1 = 0;
            ip_count = PIN_FLIST_ELEM_COUNT(deviceflistp, PIN_FLD_RESULTS, ebufp);
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "check issue han2: ");
            if (PIN_FLIST_ELEM_COUNT(deviceflistp, PIN_FLD_RESULTS, ebufp) > 0)
            {
                while ( (temp_flistp = PIN_FLIST_ELEM_GET_NEXT (deviceflistp,
                       PIN_FLD_RESULTS, &rec_id1, 1,&cookie1, ebufp)) != (pin_flist_t *)NULL )
                {
                    cred_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_CREDENTIALS, cred_arrayid, ebufp);

                    PIN_FLIST_FLD_COPY(temp_flistp, PIN_FLD_DEVICE_ID,cred_flistp, MSO_FLD_NETWORKID, ebufp);
                    PIN_FLIST_FLD_SET(cred_flistp, PIN_FLD_DESCR, "IP Address", ebufp);
                    cred_arrayid ++;
                }

            }
            else
            {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR , "Ip does'nt exist for this tal account");
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                               PIN_ERRCLASS_SYSTEM_DETERMINATE,PIN_ERR_BAD_VALUE, PIN_FLD_DEVICE_ID, 0, 0);
                *error_codep = MSO_BB_IP_DEVICE_ERROR;
                goto cleanup;
            }
        }
        else
        {
            plan_flistp = PIN_FLIST_ELEM_GET(*i_flistpp, PIN_FLD_PLAN, 0, 0, ebufp);
            cookie1 = NULL;
            rec_id1 = 0;
            ip_count = PIN_FLIST_ELEM_COUNT(plan_flistp, PIN_FLD_DEVICES, ebufp);
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "check issue han2: ");
            if (PIN_FLIST_ELEM_COUNT(plan_flistp, PIN_FLD_DEVICES, ebufp) > 0)
            {
                while ( (temp_flistp = PIN_FLIST_ELEM_GET_NEXT (plan_flistp,
                       PIN_FLD_DEVICES, &rec_id1, 1,&cookie1, ebufp)) != (pin_flist_t *)NULL )
                {
                    cred_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_CREDENTIALS, cred_arrayid, ebufp);

                    PIN_FLIST_FLD_COPY(temp_flistp, PIN_FLD_DEVICE_ID,cred_flistp, MSO_FLD_NETWORKID, ebufp);
                    PIN_FLIST_FLD_SET(cred_flistp, PIN_FLD_DESCR, "IP Address", ebufp);
                    cred_arrayid ++;
                }
            }
            else
            {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR , "Ip does'nt exist for this tal account");
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                               PIN_ERRCLASS_SYSTEM_DETERMINATE,PIN_ERR_BAD_VALUE, PIN_FLD_DEVICE_ID, 0, 0);
                *error_codep = MSO_BB_IP_DEVICE_ERROR;
                goto cleanup;
            }
        }
        PIN_FLIST_DESTROY_EX(&deviceflistp, NULL);
        login = NULL;	
	    login = PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_LOGIN, 0, ebufp);
	   	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORKID, login, ebufp);
	}
	else{
		cred_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_CREDENTIALS, cred_arrayid, ebufp);
		user_id = PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_LOGIN, 0, ebufp);
		PIN_FLIST_FLD_SET(cred_flistp, MSO_FLD_NETWORKID, (void *)user_id, ebufp);
		PIN_FLIST_FLD_SET(cred_flistp, PIN_FLD_DESCR, "Username/Password", ebufp);
		pass = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_PASSWORD, 0, ebufp);
		PIN_FLIST_FLD_SET(cred_flistp, MSO_FLD_PASSWORD, pass, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"add_cred_qps_flist:call fm_mso_get_mac");
		fm_mso_get_mac(ctxp, serv_poid, &mac_id, error_codep, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"add_cred_qps_flist:return  fm_mso_get_mac");
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"add_cred_qps_flist:fm_mso_get_mac Error mac",ebufp);
			goto cleanup;
		}
		PIN_FLIST_FLD_PUT(task_flistp, MSO_FLD_NETWORKID, mac_id, ebufp);
	}
cleanup:
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"add_cred_qps_flist:input leave",*i_flistpp);
return;
	
}

static void
fm_mso_get_mac(
	pcm_context_t		*ctxp,
	poid_t				*serv_poid,
	char				**mac_idpp,
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*deviceflistp = NULL;
	pin_flist_t			*temp_flistp = NULL;
	char				param[50] = {'\0'};
	char				*mac = NULL;
	char				*device_type = NULL;
	int32				rec_id = 0;
	int32				count = 0;
	pin_cookie_t		cookie = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Inside fm_mso_get_mac");
	strcpy(param,"/device/modem");
	/* get the modem device if any */
	fm_mso_search_devices(ctxp, param, serv_poid, &deviceflistp, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_mso_get_mac: Error",ebufp);
		return;
	}
	count = PIN_FLIST_ELEM_COUNT(deviceflistp, PIN_FLD_RESULTS, ebufp);
	if( count == 0)
	{
		strcpy(param,"/device/router/cable");
		/* get the router device if any */
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Checking for router");
		fm_mso_search_devices(ctxp,param,serv_poid,&deviceflistp, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_mso_get_mac: Error",ebufp);
			return;
		}
		count = PIN_FLIST_ELEM_COUNT(deviceflistp, PIN_FLD_RESULTS, ebufp);
		if ( count == 0){
			strcpy(param,"/device/ip/static");
			/* get the router device if any */
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Checking for IP");
			fm_mso_search_devices(ctxp,param,serv_poid,&deviceflistp, error_codep, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_mso_get_mac: Error",ebufp);
				return;
			}
			count = PIN_FLIST_ELEM_COUNT(deviceflistp, PIN_FLD_RESULTS, ebufp);
		}
	}
	if(count > 0)
	{
		cookie = NULL;
		rec_id =0;
		while ( (temp_flistp = PIN_FLIST_ELEM_GET_NEXT (deviceflistp,
			       PIN_FLD_RESULTS, &rec_id, 1,&cookie, ebufp))
							  != (pin_flist_t *)NULL ) 
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_get_mac:result flist for device",temp_flistp);
			mac = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_DEVICE_ID, 0,ebufp);
			device_type = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_DEVICE_TYPE, 0,ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,mac);
			/**** Pavan Bellala 29-07-2015 ***********************************
			 Defect 1136 : Validate device string pattern before provisioning
			*****************************************************************/
			if(mac && strlen(mac) > 0)
			{				
				// Added the below code for JIO-CPS Integration-ISP Enhancement
				//Skip MAC validation for ONU (GPON) modem for JIO and Hathway Networks and validate the device_id pattern for other device types.
				if((device_type && 
				   !(strcmp(device_type, HW_GPON_MODEM) == 0 || 
					 strcmp(device_type, HW_ONU_GPON_MODEM) == 0 || 
					 strcmp(device_type, JIO_GPON_MODEM) == 0))
					&& fm_mso_integ_match_patterns("^([0-9a-fA-F]{2})(([/\\s:-][0-9a-fA-F]{2}){5})$", mac) <=0 )
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR , "Error:Invalid MAC id");
					pin_set_err(ebufp, PIN_ERRLOC_FM,
                                        PIN_ERRCLASS_SYSTEM_DETERMINATE,PIN_ERR_BAD_VALUE, PIN_FLD_DEVICE_ID, 0, 0);
                			*error_codep = MSO_BB_INVALID_MAC;
                			return;
				} else {
			
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"valid mac id");	
					if (*mac_idpp == NULL)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Double pointer null");
						/*** Check if first device and add ****/	
						*mac_idpp = (char*)malloc((sizeof (char) * strlen(mac))+1);
						if (*mac_idpp == NULL)
						{
							pin_set_err(ebufp, PIN_ERRLOC_FM,PIN_ERRCLASS_SYSTEM_DETERMINATE,
									PIN_ERR_NO_MEM, 0, 0, 0);
							PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_mso_get_mac: memory error", ebufp);
							*error_codep = MSO_BB_INTERNAL_ERROR;
							return;
						}
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Mem allocated");
						strcpy(*mac_idpp,mac);
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"mac copied");
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*mac_idpp);
						if ( strcmp(param, "/device/ip/static") == 0 && mac_idpp != NULL){
							break;
						}
					} else {
						/***** Add further devices by allocating more mem *****/
						*mac_idpp = (char *) realloc(*mac_idpp, (strlen(mac)+strlen(*mac_idpp))+2);
						if (*mac_idpp == NULL)
						{
							pin_set_err(ebufp, PIN_ERRLOC_FM,PIN_ERRCLASS_SYSTEM_DETERMINATE,
									PIN_ERR_NO_MEM, 0, 0, 0);
								PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_mso_get_mac: memory error", ebufp);
								*error_codep = MSO_BB_INTERNAL_ERROR;
								return;
						}
						strcat(*mac_idpp,",");
						strcat(*mac_idpp,mac);
					}

				}

			}

		}

	}

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"fm_mso_get_mac: mac device");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*mac_idpp);

	PIN_FLIST_DESTROY_EX(&deviceflistp,NULL);	
	return;
}


static void
prepare_switch_service_flist(
	pcm_context_t		*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp,
	char				*act,
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*createbalflistp = NULL;
	pin_flist_t			*mso_subavp = NULL;
	pin_flist_t			*grp_detail_flistp = NULL;
	pin_flist_t			*plan_iflistp = NULL;
	pin_flist_t			*plan_oflistp = NULL;
	pin_flist_t			*bal_flistp = NULL;
	pin_flist_t			*bal_o_flistp = NULL;
	pin_flist_t			*balance_flistp = NULL;
	pin_flist_t			*sub_bal_flistp = NULL;
	//pin_flist_t			*srch_iflistp = NULL;
	//pin_flist_t			*s = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	//poid_t				*s_pdp = NULL;
	poid_t				*plan_pdp = NULL;
	poid_t				*old_prod_pdp = NULL;
	poid_t				*prod_pdp = NULL;
	char				*plan_name = NULL;
	char				old_plan_name[100] = "\0";
	char				*templ = NULL;
	char				*prov_tag = NULL;
	char				*node_name = NULL;
	char				*ne_add = NULL;
	char 				svc_code[100] ="\0";
	char 				quota_code[100] ="\0";
	char				time_buf[30] = {'\0'};
	int				    *tech = NULL;
	int32              	quota_flag = 0;
	int32              	*bill_when = NULL;
	int				    old_quota = 0;
	int	               	flag = 0;
	int				    srv_typ = 3;
	int				    plan_typ = 0;
	poid_t				*ac_pdp = NULL;
	int32              	*action_flag = NULL;
	char				*user_id = NULL;
	char				*data_code = "DATA";
	char				*city = NULL;
	time_t				*end_t = NULL;
	time_t				*valid_from = NULL;
	time_t				*valid_to = NULL;
	time_t				*pvt = NULL;
    time_t              now = 0;
    time_t              *sub_expiryp = 0;
	pin_decimal_t		*curr_bal = NULL;
	pin_decimal_t		*total_bal = NULL;
	pin_cookie_t		cookie = NULL;
	pin_decimal_t		*initial_amount = NULL;
    	int32			rec_id = 0;
	int				    *isTAL = 0;
	char				*zero_amount = "0";
	poid_t				*mso_bb_bp_obj = NULL;	
	poid_t				*mso_bb_obj = NULL;
	poid_t              *base_plan_pdp = NULL;
	pin_flist_t			*mso_iflistp = NULL;
	pin_flist_t			*mso_oflistp = NULL;
	pin_flist_t         *plan_flistp = NULL;
	int32				*plan_status = NULL;
	char				*action = NULL;
	char				msg[100];

	FILE 				*filep = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	filep = fopen("/tmp/stackt","a+");

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_switch_service_flist: input flist", *i_flistpp);
	/******Add the Task Array for BBC*******************/
	task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, task_id, ebufp);
	task_id = task_id + 1;
	
	action = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_ACTION, 1, ebufp);	
	action_flag = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_FLAGS, 0, ebufp);

	now = pin_virtual_time(NULL);
    if ((strcmp(other_ne_id, HW_NETWORK_STER_ID) == 0))
    {
        now = now - 600;
        sprintf(time_buf, "%d000", now);
	    PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_START_DATE, (void *)time_buf, ebufp);
    }
		
	sub_expiryp = (time_t *)PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_PURCHASE_END_T, 1, ebufp);
    if (sub_expiryp)
    {
        if (*sub_expiryp == 0)
        {
            sprintf(time_buf, "1893436200000");
        }
        else
        {
            sprintf(time_buf, "%d000", *sub_expiryp);
        }
	    PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_EXPIRYDATE, (void *)time_buf, ebufp);
    }
	
	if ( *action_flag ==  DOC_BB_FUP_REVERSAL )
	{
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "FUP_REVERSAL_QPS", ebufp);
	}
	else if(*action_flag ==  DOC_BB_UPDATE_CAP)
	{
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "FUP_UPDATE_QPS", ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "CHANGE_SERVICES_QPS", ebufp);
	}
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	/******Add the Subscriber Array*******************/

	ac_pdp = (poid_t *)PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_POID, 0, ebufp);
	isTAL = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_ISTAL_FLAG, 0, ebufp);
//	if(*isTAL == 1){
//		fm_mso_get_login(ctxp, &user_id, so_flistp, error_codep, ebufp);
//		if (PIN_ERRBUF_IS_ERR(ebufp))
//		{
//			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
//				"prepare_bb_flist: Error getting name",ebufp);
//			goto cleanup;
//		}
//	}
	//else{
	user_id = (char *)PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_LOGIN, 0, ebufp);
	//}
	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORKID, user_id, ebufp);
	//Get the Network Node Name
	node_name = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NE_ELEMENT_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_switch_service_flist: Network Element Missing",ebufp);
		goto cleanup;
	}
	city = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_CITY_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_switch_service_flist: Network Element Missing",ebufp);
		goto cleanup;
	}
	//Call the function to get the NE Address
	fm_mso_get_network_node(ctxp, node_name, city, &ne_add, 1, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_switch_service_flist: Not able to get Network Element ID",ebufp);
		goto cleanup;
	}
	if(ne_add){
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, (void *)ne_add, ebufp);
	}

	plan_pdp = (poid_t *)PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_PLAN_OBJ, 0, ebufp);
	//---------------------------------------------------------------------------
	plan_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(plan_iflistp, PIN_FLD_POID, plan_pdp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_switch_service_flist: Read input flist", plan_iflistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, plan_iflistp, &plan_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_switch_service_flist: Error Calling Read OBJ", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_switch_service_flist: Read output flist", plan_oflistp);
	
	plan_name = (char *)PIN_FLIST_FLD_GET(plan_oflistp, PIN_FLD_NAME, 0, ebufp);
	//---------------------------------------------------------------------------------
	/* Pawan:19-01-2015: Commented the get_last_plan function and added new function 
		get_cancelled_plan to fetch the details of cancelled plan*/
	//get_last_plan(ctxp, ac_pdp, &old_plan_name, &end_t, &prod_pdp, error_codep, ebufp);
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp);
	bill_when = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);
	if ( *action_flag == DOC_BB_RENEW_AEXPIRY_PRE)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Renew scenario");
		get_cancelled_plan(ctxp, ac_pdp, city, old_plan_name, &end_t, &prod_pdp, error_codep, tech,action_flag,MSO_PROV_STATE_DEACTIVE, ebufp);
	} 
	else if ( *action_flag == DOC_BB_FUP_REVERSAL)
	{
		mso_bb_obj = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_PURCHASED_PLAN_OBJ, 0, ebufp);	
		mso_iflistp = PIN_FLIST_CREATE(ebufp);
        	PIN_FLIST_FLD_SET(mso_iflistp, PIN_FLD_POID, mso_bb_obj, ebufp);
		PIN_FLIST_FLD_SET(mso_iflistp, PIN_FLD_STATUS, 0, ebufp);
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_switch_service_flist: MSO Read flds input flist", mso_iflistp);
        	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, mso_iflistp, &mso_oflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&mso_iflistp, NULL);	
        	if (PIN_ERRBUF_IS_ERR(ebufp))
        	{
                *error_codep = MSO_BB_INTERNAL_ERROR;
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "prepare_switch_service_flist: Error Calling Read flds", ebufp);
                goto cleanup;
        	}
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_switch_service_flist: Read output flist", mso_oflistp);
		plan_status = PIN_FLIST_FLD_GET(mso_oflistp, PIN_FLD_STATUS, 0, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"FUP Reversal scenario");
		//Below added to get plan name as base plan name
                base_plan_pdp = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_OLD_PLAN_OBJ, 1, ebufp);
                if(base_plan_pdp)
                        fm_mso_utils_read_any_object(ctxp, base_plan_pdp, &plan_flistp, ebufp);
                if(plan_flistp){
                        plan_name = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_NAME, 1, ebufp);
                }
		if(plan_status && *plan_status == MSO_PROV_STATE_SUSPENDED)
			get_cancelled_plan(ctxp, ac_pdp, city, old_plan_name, &end_t, &prod_pdp, error_codep, tech,action_flag,MSO_PROV_STATE_SUSPENDED, ebufp);
		else if(plan_status && *plan_status == MSO_PROV_STATE_IN_PROGRESS)
			get_cancelled_plan(ctxp, ac_pdp, city, old_plan_name, &end_t, &prod_pdp, error_codep, tech,action_flag,MSO_PROV_STATE_IN_PROGRESS, ebufp);
		else
			get_cancelled_plan(ctxp, ac_pdp, city, old_plan_name, &end_t, &prod_pdp, error_codep, tech,action_flag,MSO_PROV_STATE_ACTIVE, ebufp);
		
		PIN_FLIST_DESTROY_EX(&mso_oflistp, NULL);
	}
	/**** Added more scenarios *************/
	else if ( *action_flag == DOC_BB_ADD_MB)
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"ADD MB scenario");
		//Below added to get plan name as base plan name
		base_plan_pdp = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_OLD_PLAN_OBJ, 1, ebufp);
		if(base_plan_pdp)
			fm_mso_utils_read_any_object(ctxp, base_plan_pdp, &plan_flistp, ebufp);
		if(plan_flistp){
			plan_name = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_NAME, 1, ebufp);
		}
                get_cancelled_plan(ctxp, ac_pdp, city, old_plan_name, &end_t, &prod_pdp, error_codep, tech,action_flag,MSO_PROV_STATE_ACTIVE, ebufp);
        }
	else if ( *action_flag == DOC_BB_PLAN_VALIDITY_EXTENSION)
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Validity Extension Scenario");
                get_cancelled_plan(ctxp, ac_pdp, city, old_plan_name, &end_t, &prod_pdp, error_codep, tech,action_flag,MSO_PROV_STATE_ACTIVE, ebufp);
        }
	else if ( *action_flag == DOC_BB_AUTO_RENEW_POST)
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Auto Renew Post Scenario");
		mso_bb_obj = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_PURCHASED_PLAN_OBJ, 0, ebufp);
                mso_iflistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(mso_iflistp, PIN_FLD_POID, mso_bb_obj, ebufp);
                PIN_FLIST_FLD_SET(mso_iflistp, PIN_FLD_STATUS, 0, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_switch_service_flist: MSO Read flds input flist", mso_iflistp);
                PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, mso_iflistp, &mso_oflistp, ebufp);
                PIN_FLIST_DESTROY_EX(&mso_iflistp, NULL);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                *error_codep = MSO_BB_INTERNAL_ERROR;
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "prepare_switch_service_flist: Error Calling Read flds", ebufp);
                goto cleanup;
                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_switch_service_flist: Read output flist", mso_oflistp);
                plan_status = PIN_FLIST_FLD_GET(mso_oflistp, PIN_FLD_STATUS, 0, ebufp);

                if(plan_status && *plan_status == MSO_PROV_STATE_SUSPENDED)
                        get_cancelled_plan(ctxp, ac_pdp, city, old_plan_name, &end_t, &prod_pdp, error_codep, tech,action_flag,MSO_PROV_STATE_SUSPENDED, ebufp);
                else if(plan_status && *plan_status == MSO_PROV_STATE_IN_PROGRESS)
                        get_cancelled_plan(ctxp, ac_pdp, city, old_plan_name, &end_t, &prod_pdp, error_codep, tech,action_flag,MSO_PROV_STATE_IN_PROGRESS, ebufp);
                else
                        get_cancelled_plan(ctxp, ac_pdp, city, old_plan_name, &end_t, &prod_pdp, error_codep, tech,action_flag,MSO_PROV_STATE_ACTIVE, ebufp);

                PIN_FLIST_DESTROY_EX(&mso_oflistp, NULL);
        }
	else if ( *action_flag == DOC_BB_UPDATE_CAP)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"FUP CAP UPDATE scenario");
		get_cancelled_plan(ctxp, ac_pdp, city, old_plan_name, &end_t, &prod_pdp, error_codep, tech,action_flag,MSO_PROV_STATE_ACTIVE, ebufp);
	}
	else 
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Cancel scenario");
		get_cancelled_plan(ctxp, ac_pdp, city, old_plan_name, &end_t, &prod_pdp, error_codep, tech,action_flag,MSO_PROV_STATE_TERMINATED, ebufp);
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_switch_service_flist: Error", ebufp);
		goto cleanup;
	}
	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_OLD_SERVICECODE, old_plan_name, ebufp);
	
	//-------------------------------------------------------------------
	bal_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_POID, (void *)ac_pdp, ebufp);
	PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_FLAGS, (void *)&flag, ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp,PIN_FLD_SERVICE_OBJ,bal_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_ELEM_SET( bal_flistp, NULL, PIN_FLD_BALANCES, PIN_ELEMID_ANY, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "prepare_switch_service_flist: Get Balances input flist", bal_flistp);
	PCM_OP(ctxp, PCM_OP_BAL_GET_BALANCES, 0, bal_flistp, &bal_o_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
             *error_codep = MSO_BB_INTERNAL_ERROR;
	     PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "prepare_switch_service_flist: error getting Balances", ebufp);
             goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "prepare_switch_service_flist: Get Balances output flist", bal_o_flistp);
	//*pvt = pin_virtual_time(NULL);
	if(*action_flag ==  DOC_BB_FUP_REVERSAL){
		balance_flistp = PIN_FLIST_ELEM_GET(bal_o_flistp, 
			PIN_FLD_BALANCES, 1000104, 1, ebufp);
	}
	else{
		balance_flistp = PIN_FLIST_ELEM_GET(bal_o_flistp, 
			PIN_FLD_BALANCES, 1100010, 1, ebufp);
	}
	//quota_code = "DATA";
	if(balance_flistp)
	{
		// Commenetd as flag = 0 being pased to get summed up current balance
//		while ( (sub_bal_flistp = PIN_FLIST_ELEM_GET_NEXT (balance_flistp,
//                       PIN_FLD_SUB_BALANCES, &rec_id, 1,&cookie, ebufp))
//                                                  != (pin_flist_t *)NULL ) 
//	        {
//	        	valid_from = PIN_FLIST_FLD_GET(sub_bal_flistp, PIN_FLD_VALID_FROM, 0,ebufp);
//			valid_to = PIN_FLIST_FLD_GET(sub_bal_flistp, PIN_FLD_VALID_TO, 0,ebufp);
//			if(*valid_to == *end_t )
//			{
//				curr_bal = PIN_FLIST_FLD_GET(sub_bal_flistp, PIN_FLD_CURRENT_BAL, 0,ebufp);
//				total_bal = pbo_decimal_add(total_bal,curr_bal,ebufp);
//			}
//		}
	
		curr_bal = PIN_FLIST_FLD_GET(balance_flistp, PIN_FLD_CURRENT_BAL, 0,ebufp);
	}
	else
	{
		//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Quota Balance not found " );	
		//*error_codep = MSO_BB_QUOTA_ERROR;
		//pin_set_err(ebufp, PIN_ERRLOC_FM,
		//	PIN_ERRCLASS_SYSTEM_DETERMINATE,
		//	PIN_ERR_BAD_VALUE, PIN_FLD_BALANCES, 0, 0);	
		//goto cleanup;
	}
	/*Pawan:19-01-2015: Commented below function call since MSO_FLD_OLD_BALANCECODE
		is required in all types of change plan scenarios */
	//old_quota = fm_quota_check_old(ctxp, prod_pdp, error_codep, ebufp);
	//if(old_quota)
	//{
	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_OLD_BALANCECODE, old_plan_name, ebufp);
	//}
	
		//------------------------------------------------------------------------------------------
	//Get the Service Type
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp);
	prov_tag = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_PROVISIONING_TAG, 1, ebufp);
	srv_typ = fm_mso_get_srv_type(ctxp, plan_name, prov_tag, tech, bill_when, city, error_codep, svc_code,quota_code, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
             PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "prepare_switch_service_flist: Error", ebufp);
             goto cleanup;
	}
	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_SERVICE_CODE, svc_code, ebufp);
	
	mso_subavp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_SUB_AVP, 1, ebufp);
	PIN_FLIST_FLD_SET(mso_subavp, MSO_FLD_CODE, "ServiceType", ebufp);

	switch (srv_typ)
	{
	case 0:
			PIN_FLIST_FLD_SET(mso_subavp, MSO_FLD_VALUE, "POST", ebufp);
		
		PIN_FLIST_FLD_SET(*i_flistpp, PIN_FLD_INDICATOR, &srv_typ, ebufp);
		break;
	case 1:
			PIN_FLIST_FLD_SET(mso_subavp, MSO_FLD_VALUE, "PRE", ebufp);
		
		PIN_FLIST_FLD_SET(*i_flistpp, PIN_FLD_INDICATOR, &srv_typ, ebufp);
		break;
	default:
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Invalid Service Type" );
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, MSO_FLD_VALUE, 0, 0);
		goto cleanup;
	}
	

	mso_subavp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_SUB_AVP, 2, ebufp);
	PIN_FLIST_FLD_SET(mso_subavp, MSO_FLD_CODE, "isQuota", ebufp);
	createbalflistp = PIN_FLIST_ELEM_ADD(task_flistp, 
		MSO_FLD_CREATEBALANCE, 1, ebufp);
	PIN_FLIST_FLD_SET(createbalflistp, MSO_FLD_CODE, 
		(void *)quota_code, ebufp);
	PIN_FLIST_FLD_SET(createbalflistp, MSO_FLD_QUOTACODE, 
		(void *)data_code, ebufp);
	quota_flag = fm_quota_check(ctxp, plan_name, prov_tag, tech, &plan_typ, &initial_amount, error_codep, ebufp);

	

	if(quota_flag == 1 || quota_flag == 0)
	{
		if(quota_flag == 1)
		{
				PIN_FLIST_FLD_SET(mso_subavp, MSO_FLD_VALUE, "true", ebufp);

		}
		else if(quota_flag == 0)
		{

			PIN_FLIST_FLD_SET(mso_subavp, MSO_FLD_VALUE, "false", ebufp);
		}
		//createbalflistp = PIN_FLIST_CREATE(ebufp);
		//Call the function to populate the create balance array
		if(*action_flag != DOC_BB_UPDATE_CAP)
		{
		  initial_amount  = curr_bal;
		  fm_mso_add_create_bal(ctxp,i_flistpp, ac_pdp, &createbalflistp, &plan_typ, &initial_amount, srv_typ, error_codep, ebufp); 
		  if (PIN_ERRBUF_IS_ERR(ebufp)) {
		      PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_switch_service_flist: Error", ebufp);
		     goto cleanup;
		  }
		}
		else
		{
		    PIN_FLIST_FLD_SET(createbalflistp, MSO_FLD_INITIALAMOUNT, zero_amount, ebufp);	
		}
		//PIN_FLIST_ELEM_COPY(createbalflistp, MSO_FLD_CREATEBALANCE,1,
		//	 task_flistp, MSO_FLD_CREATEBALANCE,1, ebufp);
	}
	else
	{
		initial_amount = pbo_decimal_from_str((char *)UNLIMITED_QUOTA, ebufp);
		PIN_FLIST_FLD_SET(mso_subavp, MSO_FLD_VALUE, "false", ebufp);
		PIN_FLIST_FLD_PUT(createbalflistp, MSO_FLD_INITIALAMOUNT, pbo_decimal_to_str(initial_amount, ebufp), ebufp); 
	}

	if(base_plan_pdp)
        {
                PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_OLD_PLAN_OBJ, ebufp);
                PIN_FLIST_FLD_DROP(*i_flistpp, MSO_FLD_OLD_PURCHASED_PLAN_OBJ, ebufp);
        }
		
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_switch_service_flist: Payload flist", *i_flistpp);
	
cleanup :
	if(bal_flistp && bal_flistp != NULL )PIN_FLIST_DESTROY_EX(&bal_flistp, NULL);
	if(bal_o_flistp && bal_o_flistp != NULL)PIN_FLIST_DESTROY_EX(&bal_o_flistp, NULL);
	if(plan_iflistp && plan_iflistp != NULL) PIN_FLIST_DESTROY_EX(&plan_iflistp, NULL);
	if(plan_oflistp && plan_oflistp != NULL )PIN_FLIST_DESTROY_EX(&plan_oflistp, NULL);
	if(ne_add != NULL && ne_add)pin_free(ne_add);
	if(prod_pdp != NULL && prod_pdp) PIN_POID_DESTROY(prod_pdp, ebufp); 
//	if(!pbo_decimal_is_null(initial_amount, ebufp))
//		pbo_decimal_destroy(&initial_amount); 
	return;
}

static void
prepare_docsis_update_avp_qnq(
	pcm_context_t			*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp,
	char				*act,
	int32				*error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*plan_iflistp = NULL;
	pin_flist_t			*plan_oflistp = NULL;
	pin_flist_t			*mso_avp = NULL;
	poid_t				*plan_pdp = NULL;
	int			    	*tech = 0;
	int32				*bill_when = NULL;
	int				    srv_typ = 0;
	int				    quota_flag = 0;
	int				    plan_typ = 0;
	char				*prov_tag = NULL;
	char				*node_name = NULL;
	char				*city = NULL;
	char				*ne_add = NULL;
	char				*plan_name = NULL;
	char				*user_id = NULL;
	char 				svc_code[100] ="\0";
	char 				quota_code[100] ="\0";
	pin_decimal_t		*initial_amount = NULL;

	// Changes for QPS to CPS upgrade.
	char            search_template[150] = " select X from /purchased_product 1 ,/mso_purchased_plan 2 where 2.F1 = V1 and 1.F2 = V2 and 2.F3 = 1.F4 and 2.F5 = V5 ";
	int                 search_flags = 768;
    int64               db = 1;
	int32               act_status = 1;
	char	         	base_plan[20]="base plan";
    int                 quota_flag_new = 0;
    int                 *tech_new = 0;
	int 			    sub_tech_new = TECH_NEW_CPS;
	char                    *prov_tag_new = NULL;
	char                    *plan_name_new = NULL;
	int32                   *bill_when_new = NULL;
	pcm_context_t           *new_ctxp;
	poid_t			       	*acc_pdp;
	poid_t				    *srv_pdp;
	poid_t				    *prod_pdp;
 	pin_flist_t             *search_inflistp = NULL;
	pin_flist_t             *tmp_flistp_new = NULL;
 	pin_flist_t             *args_flistp = NULL;
	pin_flist_t             *temp_flistp = NULL;
	pin_flist_t             *results_flistp = NULL;
	pin_flist_t             *search_outflistp = NULL;
	pin_flist_t             **r_flistpp = NULL;
    pin_flist_t			    *srv_iflistp=NULL;
    pin_flist_t			    *srv_oflistp=NULL;
    pin_flist_t			    *plan_new_iflistp=NULL;
    pin_flist_t			    *plan_new_oflistp=NULL;
    pin_flist_t			    *prod_iflistp=NULL;
    pin_flist_t			    *prod_oflistp=NULL;
	int32				    elem_id=0;
	pin_cookie_t            cookie = NULL;
	char			    	*city_new = NULL;
	int				        srv_typ_new = 0;

	FILE				    *filep = NULL;
	
	filep = fopen("/tmp/stackt","a+");	
	

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	/******Add the Task Array for BBC*******************/
	task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, task_id, ebufp);
	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "UPDATE_AVP_QPS", ebufp);
	task_id = task_id + 1;
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	user_id = (char *)PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_LOGIN, 0, ebufp);
	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORKID, user_id, ebufp);
	//Get the Network Node Name
	node_name = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NE_ELEMENT_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_docsis_update_avp_qnq: Network Element Missing",ebufp);
		goto cleanup;
	}
	city = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_CITY_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_docsis_update_avp_qnq: Network Element Missing",ebufp);
		goto cleanup;
	}
	//Call the function to get the NE Address
	fm_mso_get_network_node(ctxp, node_name, city, &ne_add, 1, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_docsis_update_avp_qnq: Not able to get Network Element ID",ebufp);
		goto cleanup;
	}
	if(ne_add){
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, (void *)ne_add, ebufp);
	}

	plan_pdp = (poid_t *)PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_PLAN_OBJ, 0, ebufp);
	//---------------------------------------------------------------------------
	plan_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(plan_iflistp, PIN_FLD_POID, plan_pdp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_docsis_update_avp_qnq: Read input flist", plan_iflistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, plan_iflistp, &plan_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_docsis_update_avp_qnq: Error Calling Read OBJ", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_docsis_update_avp_qnq: Read output flist", plan_oflistp);
	
	plan_name = (char *)PIN_FLIST_FLD_GET(plan_oflistp, PIN_FLD_NAME, 0, ebufp);
	//Get the Service Type
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp);
	prov_tag = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_PROVISIONING_TAG, 1, ebufp);
	bill_when = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);
	srv_typ = fm_mso_get_srv_type(ctxp, plan_name, prov_tag, tech, bill_when, city, error_codep, svc_code,quota_code, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_docsis_update_avp_qnq: Error", ebufp);
		goto cleanup;
	}
	quota_flag = fm_quota_check(ctxp, plan_name, prov_tag, tech, &plan_typ, &initial_amount, error_codep, ebufp);


	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_docsis_update_avp_qnq: Error", ebufp);
		goto cleanup;
	}
	/*** Pavan Bellala : 14/07/2015 Change tag name in Payload flist to MSO_FLD_MOD_AVP ****/
	//mso_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_AVP, 1, ebufp);
	/*** Pavan Bellala : 14/07/2015 Change tag name in Payload flist to MSO_FLD_MOD_AVP ****/
	mso_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_MOD_AVP, 1, ebufp);
	PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_CODE, "isQuota", ebufp);

	/* Changes for  QPS to CPS Migration. Retrieve  old plan using an alternate connection. */

    //Added the below code for JIO-CPS Integration-ISP Enhancement
	if(other_ne_id != NULL)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Network ELement ID >> ");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, other_ne_id);		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CHANGE_SERVICES_DOCSIS or  CHANGE_SERVICES_FIBER ");
	
		if( (strcmp(other_ne_id,HW_NETWORK_ELEMENT_ID) == 0 || strcmp(other_ne_id,JIO_NETWORK_ELEMENT_ID) == 0 ||
                   strcmp(other_ne_id, HW_NETWORK_STER_ID) == 0) 
		//   if((strcmp(other_ne_id,HW_NETWORK_ELEMENT_ID) == 0) 
			&& ( strcmp((char *)act,"CHANGE_SERVICES_DOCSIS") == 0 || 
				 strcmp((char *)act,"CHANGE_SERVICES_FIBER") == 0 ) )
		{			
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Action >> ");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, (char *)act);
			
			PCM_CONTEXT_OPEN(&new_ctxp, (pin_flist_t *)0, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling Get Current Active Plan");

			acc_pdp = (poid_t *) PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
			srv_pdp = (poid_t *) PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_POID, 0, ebufp);

			db = PIN_POID_GET_DB(acc_pdp);

			search_inflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
			PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);
			args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acc_pdp, ebufp);
			args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp);
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &act_status, ebufp);
			args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 3, ebufp);
			temp_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, ebufp);
			PIN_FLIST_FLD_SET(temp_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, NULL, ebufp);
			args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 4, ebufp);
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);
			args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 5, ebufp);
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DESCR, base_plan, ebufp);

			results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"search mso_purchased_plan input list", search_inflistp);

			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				//On Error Return Cleanly
				PIN_ERRBUF_CLEAR(ebufp);
			}

			PCM_OP(new_ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);

			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				//On Error Return Cleanly
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,
									"search mso_purchased_plan output list", search_outflistp);
				return;
			}
			else
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"mso search output flist", search_outflistp);
			}

			// Retrieve provisioning tag from product obj
			elem_id = 0;cookie = NULL;
			while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(search_outflistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
			{
				prod_pdp = (poid_t*)PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp ); 
				plan_pdp = (poid_t *)PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);

				prod_iflistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(prod_iflistp, PIN_FLD_POID, prod_pdp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG," Read  product input flist", prod_iflistp);
				PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, prod_iflistp, &prod_oflistp, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					*error_codep = MSO_BB_INTERNAL_ERROR;
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						"prepare_docsis_update_avp_qnq: Error Calling Read OBJ", ebufp);
					goto cleanup;
				}
				
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG," Read Prod output flist", plan_oflistp);
				if(prod_oflistp && prod_oflistp != NULL)
				{
					prov_tag_new = PIN_FLIST_FLD_GET(prod_oflistp, PIN_FLD_PROVISIONING_TAG, 1, ebufp);
				}
				
				if(prov_tag_new != NULL && strlen(prov_tag_new) > 5)
				{
					break;
				}
	
				PIN_FLIST_DESTROY_EX(&prod_oflistp,NULL);
			}

			//---------------------------------------------------------------------------
			plan_new_iflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(plan_new_iflistp, PIN_FLD_POID, plan_pdp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_docsis_update_avp_qnq: Read input flist", plan_new_iflistp);
			PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, plan_new_iflistp, &plan_new_oflistp, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
                *error_codep = MSO_BB_INTERNAL_ERROR;
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "prepare_docsis_update_avp_qnq: Error Calling Read OBJ", ebufp);
				goto cleanup;
			}
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_docsis_update_avp_qnq: Read output flist", plan_new_oflistp);

			plan_name_new = (char *)PIN_FLIST_FLD_GET(plan_new_oflistp, PIN_FLD_NAME, 0, ebufp);

			/* Retrieve current SERVICE_BB_INFO using alrernate connection */

			//---------------------------------------------------------------------------
			srv_iflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(srv_iflistp, PIN_FLD_POID, srv_pdp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_docsis_update_avp_qnq: Read service input flist", srv_iflistp);
			PCM_OP(new_ctxp, PCM_OP_READ_OBJ, 0, srv_iflistp, &srv_oflistp, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				*error_codep = MSO_BB_INTERNAL_ERROR;
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"prepare_docsis_update_avp_qnq: Error Calling Read OBJ", ebufp);
				goto cleanup;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_docsis_update_avp_qnq: Read service output flist", srv_oflistp);

			tmp_flistp_new = PIN_FLIST_SUBSTR_GET(srv_oflistp, MSO_FLD_BB_INFO, 0, ebufp);

			tech_new = &sub_tech_new;
			bill_when_new = PIN_FLIST_FLD_GET(tmp_flistp_new, PIN_FLD_BILL_WHEN, 0, ebufp);
			city_new = (char *)PIN_FLIST_FLD_GET(tmp_flistp_new, PIN_FLD_CITY, 0, ebufp);

			quota_flag_new = fm_quota_check(ctxp, plan_name_new, prov_tag_new, tech_new, &plan_typ, &initial_amount, error_codep, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				*error_codep = MSO_BB_INTERNAL_ERROR;
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"prepare_docsis_update_avp_qnq: Error Calling Read OBJ", ebufp);
				goto cleanup;
			}

			if(quota_flag == 1)
			{
				if(mso_avp != NULL ) 
					PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_AVP_NEW_VALUE, "true", ebufp);
			}
			else
			{
				PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_AVP_NEW_VALUE, "false", ebufp);
			}

			if(quota_flag_new == 1)
			{
				PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_VALUE, "true", ebufp);
			}
			else
			{
				PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_VALUE, "false", ebufp);
			}
			srv_typ_new = fm_mso_get_srv_type(ctxp, plan_name_new, prov_tag_new, tech_new, bill_when_new, city_new, error_codep, svc_code,quota_code, ebufp);

			if(srv_typ == 1)
			{
				PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_AVP_NEW_VALUE, "PRE", ebufp);
			}
			else
			{
				PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_AVP_NEW_VALUE, "POST", ebufp);
			}

			if(srv_typ_new == 1)
			{
				PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_VALUE, "PRE", ebufp);
			}
			else
			{
				PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_VALUE, "POST", ebufp);
			}
			
			PCM_CONTEXT_CLOSE(new_ctxp, 0, ebufp);
		}
	}	
	// End of change for QPS TO CPS Upgrade
	else
	{
		if(quota_flag == 1)
		{
			PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_VALUE, "true", ebufp);
		}
		else
		{
			PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_VALUE, "false", ebufp);
		}
		/*** Pavan Bellala : 14/07/2015 Change tag name in Payload flist to MSO_FLD_MOD_AVP ****/
		//mso_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_AVP, 2, ebufp);
		/*** Pavan Bellala : 14/07/2015 Change tag name in Payload flist to MSO_FLD_MOD_AVP ****/
		mso_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_MOD_AVP, 2, ebufp);
		PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_CODE, "ServiceType", ebufp);
		if(srv_typ == 1)
		{
			PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_VALUE, "PRE", ebufp);
		}
		else
		{
			PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_VALUE, "POST", ebufp);
		}
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_docsis_update_avp_qnq: Payload flist", *i_flistpp);
	
cleanup:

	PIN_FLIST_DESTROY_EX(&plan_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&plan_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_inflistp,NULL);
	PIN_FLIST_DESTROY_EX(&search_outflistp,NULL);
	if(srv_iflistp) 
		PIN_FLIST_DESTROY_EX(&srv_iflistp,NULL);
	if(srv_oflistp != NULL )
		PIN_FLIST_DESTROY_EX(&srv_oflistp,NULL);
	if(plan_new_iflistp != NULL )
		PIN_FLIST_DESTROY_EX(&plan_new_iflistp,NULL);
	if(plan_new_oflistp != NULL)
		PIN_FLIST_DESTROY_EX(&plan_new_oflistp,NULL);
	if(prod_iflistp) 
		PIN_FLIST_DESTROY_EX(&prod_iflistp,NULL);
	if(prod_oflistp) 
		PIN_FLIST_DESTROY_EX(&prod_oflistp,NULL);

//	if(!pbo_decimal_is_null(initial_amount, ebufp))
//		pbo_decimal_destroy(&initial_amount);
	return;
}

static void
prepare_docsis_update_avp_qq(
	pcm_context_t		*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp,
	int32				*error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*plan_iflistp = NULL;
	pin_flist_t			*plan_oflistp = NULL;
	pin_flist_t			*mso_avp = NULL;
	poid_t				*plan_pdp = NULL;
	int				    *tech = 0;
	int32				*bill_when = NULL;
	int				    srv_typ = 0;
	//int				quota_flag = 0;
	int				    plan_typ = 0;
	char				*prov_tag = NULL;
	char				*node_name = NULL;
	char				*city = NULL;
	char				*ne_add = NULL;
	char				*plan_name = NULL;
	char				*user_id = NULL;
	char 				svc_code[100] ="\0";
	char 				quota_code[100] ="\0";
	//pin_decimal_t			*initial_amount = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	/******Add the Task Array for BBC*******************/
	task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, task_id, ebufp);
	task_id = task_id + 1;
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	user_id = (char *)PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_LOGIN, 0, ebufp);
	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORKID, user_id, ebufp);
	//Get the Network Node Name
	node_name = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NE_ELEMENT_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_docsis_update_avp_qq: Network Element Missing",ebufp);
		goto cleanup;
	}
	city = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_CITY_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_docsis_update_avp_qq: Network Element Missing",ebufp);
		goto cleanup;
	}
	//Call the function to get the NE Address
	fm_mso_get_network_node(ctxp, node_name, city, &ne_add, 1, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_docsis_update_avp_qq: Not able to get Network Element ID",ebufp);
		goto cleanup;
	}
	if(ne_add){
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, (void *)ne_add, ebufp);
	}

	plan_pdp = (poid_t *)PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_PLAN_OBJ, 0, ebufp);
	//---------------------------------------------------------------------------
	plan_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(plan_iflistp, PIN_FLD_POID, plan_pdp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_docsis_update_avp_qq: Read input flist", plan_iflistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, plan_iflistp, &plan_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_docsis_update_avp_qq: Error Calling Read OBJ", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_docsis_update_avp_qq: Read output flist", plan_oflistp);
	
	plan_name = (char *)PIN_FLIST_FLD_GET(plan_oflistp, PIN_FLD_NAME, 0, ebufp);
	//Get the Service Type
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp);
	prov_tag = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_PROVISIONING_TAG, 1, ebufp);
	bill_when = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);
	srv_typ = fm_mso_get_srv_type(ctxp, plan_name, prov_tag, tech, bill_when, city, error_codep, svc_code,quota_code, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_docsis_update_avp_qq: Error", ebufp);
		goto cleanup;
	}
	/*** Pavan Bellala : 14/07/2015 Change tag name in Payload flist to MSO_FLD_MOD_AVP ****/
	//mso_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_AVP, 1, ebufp);
	/*** Pavan Bellala : 14/07/2015 Change tag name in Payload flist to MSO_FLD_MOD_AVP ****/
	mso_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_MOD_AVP, 1, ebufp);
	PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_CODE, "ServiceType", ebufp);
	if(srv_typ == 1){
		PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_VALUE, "PRE", ebufp);
	}
	else{
		PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_VALUE, "POST", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_docsis_update_avp_qnq: Payload flist", *i_flistpp);
cleanup:
	PIN_FLIST_DESTROY_EX(&plan_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&plan_oflistp, NULL);
	if(ne_add != NULL && ne_add) pin_free(ne_add);
	return;
}


static void
get_old_group(
	pcm_context_t		*ctxp,
	pin_flist_t			*i_flist,
	char				**plan_name,
	time_t				**end_t,
	poid_t				**prod_pdp,
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	
	pin_flist_t		*grp_detail_flistp = NULL;
	pin_flist_t		*plan_iflistp = NULL;
	pin_flist_t		*plan_oflistp = NULL;
	pin_flist_t		*search_plan_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_flist_t		*res_iflistp = NULL;
	pin_flist_t		*search_plan_o_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*products = NULL;

	poid_t			*plan_pdp = NULL;
	poid_t			*old_plan_pdp = NULL;
	poid_t			*s_pdp = NULL;
	poid_t			*pdp = NULL;

	int32			s_flags = 0;
	int64			database = 0;

	char			*name = NULL;
	char			*dummy_pt="";
	
	void			*vp = NULL;
	/*
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS           INT [0] 0
	0 PIN_FLD_TEMPLATE        STR [0] "select X from /purchased_product 1, /mso_purchased_plan 2 where 1.F2 = 2.F1 and 2.F3 = V3 and 2.F4 = V4 and 2.F5 = V5 and 2.F6 is NOT NULL order by mso_purchased_plan_t.created_t desc "
	0 PIN_FLD_ARGS          ARRAY [1] allocated 20, used 1
	1    PIN_FLD_PRODUCTS      ARRAY [*] allocated 20, used 3
	2       MSO_FLD_PURCHASED_PRODUCT_OBJ   POID [0] NULL
	0 PIN_FLD_ARGS          ARRAY [2] allocated 20, used 1
	1    PIN_FLD_POID           POID [0] NULL
	0 PIN_FLD_ARGS          ARRAY [3] allocated 20, used 1
	1     PIN_FLD_ACCOUNT_OBJ    POID [0] 0.0.0.1 /account 7302061 9
	0 PIN_FLD_ARGS          ARRAY [4] allocated 20, used 1
	1     PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/telco/broadband 7304838 9
	0 PIN_FLD_ARGS          ARRAY [5] allocated 20, used 1
	1     PIN_FLD_PLAN_OBJ        POID [0] 0.0.0.1 /plan 7321889 0
	0 PIN_FLD_ARGS          ARRAY [6] allocated 20, used 1
	1    PIN_FLD_PRODUCTS      ARRAY [*] allocated 20, used 3
	2       PIN_FLD_PROVISIONING_TAG    STR [0] ""
	0 PIN_FLD_RESULTS       ARRAY [0] allocated 20, used 3
	1     PIN_FLD_PLAN_OBJ       POID [0] NULL poid pointer
	1     PIN_FLD_PRODUCT_OBJ    POID [0] NULL poid pointer
	1     PIN_FLD_PURCHASE_END_T TSTAMP [0] (0) <null>
	1     PIN_FLD_STATUS          ENUM [0] 0
	1     PIN_FLD_PURCHASE_END_T TSTAMP [0] (0)
	*/
	
	search_plan_flistp = PIN_FLIST_CREATE(ebufp);

	/***********************************************************
	 * assume db matches userid found in pin.conf
	 ***********************************************************/
	pdp = PCM_GET_USERID(ctxp);
	database = PIN_POID_GET_DB(pdp);

	plan_iflistp = PIN_FLIST_ELEM_GET(i_flist, PIN_FLD_PLAN_LISTS, 1, 1, ebufp);
	if (plan_iflistp)
	{
		vp = PIN_FLIST_FLD_GET(plan_iflistp, PIN_FLD_FLAGS, 1, ebufp);
		if (vp && *(int32*)vp == 0 )
		{
			old_plan_pdp = PIN_FLIST_FLD_GET(plan_iflistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
		}
		if (!old_plan_pdp)
		{
			plan_iflistp = PIN_FLIST_ELEM_GET(i_flist, PIN_FLD_PLAN_LISTS, 0, 1, ebufp);
			if (plan_iflistp)
			{
				vp = PIN_FLIST_FLD_GET(plan_iflistp, PIN_FLD_FLAGS, 1, ebufp);
				if (vp && *(int32*)vp == 0 )
				{
					old_plan_pdp = PIN_FLIST_FLD_GET(plan_iflistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
				}
			}
		}
	}


	s_pdp = PIN_POID_CREATE(database, "/search", -1, ebufp);
	
	PIN_FLIST_FLD_PUT(search_plan_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_FLD_SET(search_plan_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
	
	PIN_FLIST_FLD_SET(search_plan_flistp, PIN_FLD_TEMPLATE,
	"select X from /purchased_product 1, /mso_purchased_plan 2 where 1.F2 = 2.F1 and 2.F3 = V3 and 2.F4 = V4 and 2.F5 = V5 and 2.F6 is NOT NULL order by mso_purchased_plan_t.created_t desc " , ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(search_plan_flistp, PIN_FLD_ARGS, 1, ebufp);
	products =  PIN_FLIST_ELEM_ADD(arg_flistp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(products, MSO_FLD_PURCHASED_PRODUCT_OBJ, NULL, ebufp);
 
	arg_flistp = PIN_FLIST_ELEM_ADD(search_plan_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, NULL, ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(search_plan_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_COPY(i_flist, PIN_FLD_POID, arg_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(search_plan_flistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_COPY(i_flist, PIN_FLD_SERVICE_OBJ, arg_flistp, PIN_FLD_SERVICE_OBJ, ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(search_plan_flistp, PIN_FLD_ARGS, 5, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_PLAN_OBJ, old_plan_pdp, ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(search_plan_flistp, PIN_FLD_ARGS, 6, ebufp);
	products =  PIN_FLIST_ELEM_ADD(arg_flistp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(products, PIN_FLD_PROVISIONING_TAG, dummy_pt, ebufp);

	res_iflistp = PIN_FLIST_ELEM_ADD( search_plan_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(res_iflistp, PIN_FLD_PLAN_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_iflistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_iflistp, PIN_FLD_PURCHASE_END_T, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Get old plan input flist", search_plan_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_plan_flistp, &search_plan_o_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Get old plan output flist", search_plan_o_flistp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_INTERNAL_ERROR;
             PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "error getting old plan ", ebufp);
             goto cleanup;
	}	
	
	result_flistp = PIN_FLIST_ELEM_GET(search_plan_o_flistp,PIN_FLD_RESULTS,0,1,ebufp);
	if(result_flistp)
	{
		plan_pdp = PIN_FLIST_FLD_GET(result_flistp,PIN_FLD_PLAN_OBJ,0,ebufp);
		*prod_pdp = PIN_FLIST_FLD_TAKE(result_flistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
		*end_t = PIN_FLIST_FLD_TAKE(result_flistp, PIN_FLD_PURCHASE_END_T, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			*error_codep = MSO_BB_INTERNAL_ERROR;
		     PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"error getting last plan ", ebufp);
		     goto cleanup;
		}
		plan_iflistp = PIN_FLIST_CREATE(ebufp);
	
		PIN_FLIST_FLD_SET(plan_iflistp, PIN_FLD_POID, old_plan_pdp, ebufp);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read input flist", plan_iflistp);
		PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, plan_iflistp, &plan_oflistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read output flist", plan_oflistp);
		
		*plan_name = (char *)PIN_FLIST_FLD_TAKE(plan_oflistp, PIN_FLD_NAME, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			*error_codep = MSO_BB_INTERNAL_ERROR;
		     PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"error getting last plan ", ebufp);
		     goto cleanup;
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"OLD PLAN not found" );
		*error_codep = MSO_BB_OLD_PLAN_ERROR; 
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_PLAN_OBJ, 0, 0);
		goto cleanup;
	}

cleanup:
	PIN_FLIST_DESTROY_EX(&search_plan_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_plan_o_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&plan_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&plan_oflistp, NULL);
	return;
	
}





static void
get_last_plan(
	pcm_context_t		*ctxp,
	poid_t				*ac_pdp,
	char				**plan_name,
	time_t				**end_t,
	poid_t				**prod_pdp,
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	
	pin_flist_t		*grp_detail_flistp = NULL;
	pin_flist_t		*plan_iflistp = NULL;
	pin_flist_t		*plan_oflistp = NULL;
	pin_flist_t		*search_plan_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_flist_t		*res_iflistp = NULL;
	pin_flist_t		*search_plan_o_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	poid_t			*plan_pdp = NULL;
	poid_t			*s_pdp = NULL;
	poid_t			*pdp = NULL;
	int32           s_flags = 0;
	int64           database = 0;
	char			*name = NULL;
	
	/*
	0 PIN_FLD_POID                      POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS                      INT [0] 768
	0 PIN_FLD_TEMPLATE                   STR [0] "select X from /purchased_product where F1 = V1 order by purchase_end_t desc "
	0 PIN_FLD_RESULTS                  ARRAY [0] allocated 2, used 2
	1     PIN_FLD_PLAN_OBJ              POID [0] NULL
	1     PIN_FLD_PURCHASE_END_T      TSTAMP [0] NULL
	0 PIN_FLD_ARGS                     ARRAY [1] allocated 1, used 1
	1     PIN_FLD_ACCOUNT_OBJ           POID [0] 0.0.0.1 /account 266861 0
	*/
	
	search_plan_flistp = PIN_FLIST_CREATE(ebufp);

	/***********************************************************
	 * assume db matches userid found in pin.conf
	 ***********************************************************/
	pdp = PCM_GET_USERID(ctxp);
	database = PIN_POID_GET_DB(pdp);

	s_pdp = PIN_POID_CREATE(database, "/search", -1, ebufp);
	
	PIN_FLIST_FLD_PUT(search_plan_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_FLD_SET(search_plan_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
	
	PIN_FLIST_FLD_SET(search_plan_flistp, PIN_FLD_TEMPLATE,
	"select X from /purchased_product where F1 = V1 order by purchase_end_t desc " , ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(search_plan_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACCOUNT_OBJ, ac_pdp, ebufp);
	res_iflistp = PIN_FLIST_ELEM_ADD( search_plan_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(res_iflistp, PIN_FLD_PLAN_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_iflistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_iflistp, PIN_FLD_PURCHASE_END_T, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Get last plan input flist", search_plan_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_plan_flistp, &search_plan_o_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Get last plan output flist", search_plan_o_flistp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_INTERNAL_ERROR;
             PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "error getting last plan ", ebufp);
             goto cleanup;
	}	
	
	result_flistp = PIN_FLIST_ELEM_GET(search_plan_o_flistp,PIN_FLD_RESULTS,0,1,ebufp);
	if(result_flistp)
	{
		plan_pdp = PIN_FLIST_FLD_GET(result_flistp,PIN_FLD_PLAN_OBJ,0,ebufp);
		*prod_pdp = PIN_FLIST_FLD_TAKE(result_flistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
		*end_t = PIN_FLIST_FLD_TAKE(result_flistp, PIN_FLD_PURCHASE_END_T, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			*error_codep = MSO_BB_INTERNAL_ERROR;
		     PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"error getting last plan ", ebufp);
		     goto cleanup;
		}
		plan_iflistp = PIN_FLIST_CREATE(ebufp);
	
		PIN_FLIST_FLD_SET(plan_iflistp, PIN_FLD_POID, plan_pdp, ebufp);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read input flist", plan_iflistp);
		PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, plan_iflistp, &plan_oflistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read output flist", plan_oflistp);
		
		*plan_name = (char *)PIN_FLIST_FLD_TAKE(plan_oflistp, PIN_FLD_NAME, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			*error_codep = MSO_BB_INTERNAL_ERROR;
		     PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"error getting last plan ", ebufp);
		     goto cleanup;
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"OLD PLAN not found" );
		*error_codep = MSO_BB_OLD_PLAN_ERROR; 
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_PLAN_OBJ, 0, 0);
		goto cleanup;
	}

cleanup:
	PIN_FLIST_DESTROY_EX(&search_plan_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_plan_o_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&plan_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&plan_oflistp, NULL);
	return;
	
}

/*******************************************************************
 * fm_mso_get_srv_type()
 *
 * Get the Service Type.
 *******************************************************************/
static int
fm_mso_get_srv_type(
	pcm_context_t	*ctxp, 
	char			*plan_name, 
	char			*prov_tag, 
	int				*tech,
	int				*bill_when,
	char			*city,
	int				*error_codep,
	char			*svc_code,
	char			*quota_code,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*cache_flistp = NULL;
	pin_flist_t			*cfg_cr_flistp = NULL;
	cm_cache_key_iddbstr_t		kids;
    pin_cookie_t			    cookie = NULL;
    int32				        rec_id = 0;
	int32				        err = PIN_ERR_NONE;
	int				            *tec = 0;
	int				            srv_typ = 0;
	time_t				        last_mod_t = 0;
	time_t				        now_t = 0;
	char			*svc_code_tmp = NULL;
	char			*quota_code_tmp = NULL;

	if(PIN_ERRBUF_IS_ERR(ebufp)) {
		return 3;
	}
        /******************************************************
        * If the cache is not enabled, short circuit right away
        ******************************************************/
        if ( mso_prov_bb_config_ptr == (cm_cache_t *)NULL ) {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
                "no config flist for /mso_config_bb_pt cached ");
                pin_set_err(ebufp, PIN_ERRLOC_CM,
                                PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_FLD_CACHE_NAME, 0, 0);

        }
        /**************************************************
         * See if the entry is in our data dictionary cache
         * Prepare the cm_cache_key_iddbstr_t structure to search
         **************************************************/
        kids.id = 0;
        kids.db = 0;
        kids.str = "/mso_config_bb_pt";
        cache_flistp = cm_cache_find_entry(mso_prov_bb_config_ptr,
                                        &kids, &err);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "cm_cache_find_entry found flist", cache_flistp);
        switch(err) {
        case PIN_ERR_NONE:
                break;
        default:
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
                        "mso_prov_bb_config_object from_cache: error "
                        "accessing data dictionary cache.");
			*error_codep = MSO_BB_CONFIG_PT_ERROR;
                pin_set_err(ebufp, PIN_ERRLOC_CM,
                                PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_FLD_CACHE_NAME, 0, 0);
                goto cleanup;
        }
	last_mod_t = *(time_t*)PIN_FLIST_FLD_GET(cache_flistp, PIN_FLD_MOD_T,0,ebufp);
	now_t = pin_virtual_time((time_t *)NULL);
	if(now_t  >= last_mod_t + refresh_pt_interval )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"calling mso_prov_bb_config_object_update to refresh provisioning tag config");
		// lock mutex
		pthread_mutex_lock(&bb_pt_update_mutex);
		fm_mso_prov_bb_config_cache_update(ctxp, ebufp);
		kids.id = 0;
		kids.db = 0;
		kids.str = "/mso_config_bb_pt";
		// unlock mutex
		pthread_mutex_unlock(&bb_pt_update_mutex);
		if(cache_flistp != NULL)PIN_FLIST_DESTROY_EX(&cache_flistp, NULL);
		cache_flistp = cm_cache_find_entry(mso_prov_bb_config_ptr,&kids, &err);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"cm_cache_find_entry found flist", cache_flistp);
		switch(err)
		{
			case PIN_ERR_NONE:
				break;
			default:
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
					"mso_prov_bb_config_object from_cache: error "
					"accessing data dictionary cache.");
				pin_set_err(ebufp, PIN_ERRLOC_CM,
						PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_FLD_CACHE_NAME, 0, 0);
				goto cleanup;
		}
	}
	PIN_ERR_LOG_MSG(3, "Fields to compare");
	if(prov_tag)
	PIN_ERR_LOG_MSG(3, prov_tag);
	if(plan_name)
	PIN_ERR_LOG_MSG(3, plan_name);
	while ( (tmp_flistp = PIN_FLIST_ELEM_GET_NEXT (cache_flistp,
		PIN_FLD_RESULTS, &rec_id, 1,&cookie, ebufp))!= (pin_flist_t *)NULL )
	{
		if (prov_tag && strcmp((char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_PROVISIONING_TAG, 0, ebufp ),prov_tag) == 0 )
			//&& strcmp((char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_SERVICE_CODE, 0, ebufp ), plan_name) == 0)
		{
			tec = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_BB_TECHNOLOGY, 0, ebufp);
			if (*tech == TECH_NEW_CPS)
			{
				tech = tec;
			}
			if(*tec == *tech)
			{
				srv_typ = *(int *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_INDICATOR, 0, ebufp);
				fm_mso_get_service_quota_codes(ctxp, plan_name, bill_when, city, &cfg_cr_flistp, ebufp); 
				if(cfg_cr_flistp){
					svc_code_tmp = PIN_FLIST_FLD_GET(cfg_cr_flistp, PIN_FLD_SERVICE_CODE, 0, ebufp );
					quota_code_tmp = PIN_FLIST_FLD_GET(cfg_cr_flistp, MSO_FLD_QUOTA_CODE, 0, ebufp );
				}
				if(svc_code_tmp != NULL && strcmp(svc_code_tmp, "") != 0) {
					strcpy(svc_code,svc_code_tmp);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Service Code is:");
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,svc_code);
				}else{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Service Code is not found for given plan and city");
					*error_codep = MSO_BB_INTERNAL_ERROR;
					pin_set_err(ebufp, PIN_ERRLOC_FM,
							PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_FLD_SERVICE_CODE, 0, 0);
					goto cleanup;
				} 
				if(quota_code_tmp != NULL && strcmp(quota_code_tmp, "") != 0) {
					strcpy(quota_code,quota_code_tmp);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Quota Code is:");
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,quota_code);
				}
				else{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Quota Code is not found for given plan and city");
					*error_codep = MSO_BB_INTERNAL_ERROR;
					pin_set_err(ebufp, PIN_ERRLOC_FM,
							PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, MSO_FLD_QUOTA_CODE, 0, 0);
					goto cleanup;
				}
				break;
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Technology mismatch in customer's account and plan");
                                pin_set_err(ebufp, PIN_ERRLOC_FM,
                                         PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, MSO_BB_TECH_MISSING, 0, 0);
                                goto cleanup;	
			}
		}

	}
cleanup:
	if(PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_srv_type: Error", ebufp);
		PIN_FLIST_DESTROY_EX(&cache_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&cfg_cr_flistp, NULL);
		return 3;
	}
	if(cache_flistp != NULL )PIN_FLIST_DESTROY_EX(&cache_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&cfg_cr_flistp, NULL);
	return srv_typ;
}

/*******************************************************************
 * fm_mso_get_account_details()
 *
 * Get the account details.
 *******************************************************************/
static void
fm_mso_get_account_details(
	pcm_context_t			*ctxp,
	poid_t				*ac_pdp,
	char				**f_name,
	char				**l_name,
	char				**acc_no,
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*i_flistp = NULL;
	pin_flist_t			*r_flistp = NULL;
	pin_flist_t			*temp_flistp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	//Create the input flist.
	i_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_POID, (void *)ac_pdp, ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_ACCOUNT_NO, (void *)NULL, ebufp);
	//Add the nameinfo array
	temp_flistp = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_NAMEINFO, 1, ebufp);
	PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_FIRST_NAME, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_LAST_NAME, (void *)NULL, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_get_account_details: Error preparing input flist", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_get_account_details: Read Fields Input Flist", i_flistp);
	//Call the PCM_OP_READ_FLDS opcode
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, i_flistp, &r_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_get_account_details: Error calling PCM_OP_READ_FLDS", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_get_account_details: Read Fields Output Flist", r_flistp);
	*acc_no = (char *)PIN_FLIST_FLD_TAKE(r_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_ACCOUNTNO_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_get_account_details: Error getting the name", ebufp);
		goto cleanup;
	}
	temp_flistp = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_NAMEINFO, 1, 0, ebufp);
	//Get the first name and last name
	*f_name = (char *)PIN_FLIST_FLD_TAKE(temp_flistp,
		PIN_FLD_FIRST_NAME, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_FIRSTNAME_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_get_account_details: Error getting the name", ebufp);
		goto cleanup;
	}
	*l_name = (char *)PIN_FLIST_FLD_TAKE(temp_flistp,
		PIN_FLD_LAST_NAME, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_LASTNAME_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_get_account_details: Error getting the name", ebufp);
		goto cleanup;
	}
cleanup:
	PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	return;
}



/*******************************************************************
 * fm_mso_add_create_bal()
 *
 * returns CREATEBALANCE ARRAY.
  *******************************************************************/
static void
fm_mso_add_create_bal(
	pcm_context_t		*ctxp,
	pin_flist_t			**i_flistpp,
	poid_t				*ac_pdp,
	pin_flist_t			**createbalflistp,
	int				    *plan_type,
	pin_decimal_t		**initial_amount,
	int				    srv_typ,
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*bal_flistp = NULL;
	pin_flist_t			*bal_o_flistp = NULL;
	pin_flist_t			*balance_flistp = NULL;
	pin_flist_t			*sub_bal_flistp = NULL;
	pin_flist_t			*balflistp = NULL;
	pin_flist_t			*grp_detail_flistp = NULL;
	pin_flist_t			*plan_iflistp = NULL;
	pin_flist_t			*plan_oflistp = NULL;
	pin_flist_t			*sub_bal_imp_flistp = NULL;
	pin_flist_t			*sb_flistp = NULL;
	pin_flist_t			*srch_iflistp = NULL;
	pin_flist_t			*srch_oflistp = NULL;
	pin_flist_t			*offer_flistp = NULL;
	poid_t				*plan_pdp = NULL;
	poid_t				*mso_plan = NULL;
	poid_t				*srv_pdp = NULL;
	char				*plan_name = NULL;
//	char				*action_flag = NULL;
	int				    flag = 4;
	time_t				*valid_from = NULL;
	time_t				*valid_to = NULL;
	time_t				now = 0;
	time_t				expiry = 0;
	time_t				start = 0;
	time_t				pvt;
	pin_decimal_t		*curr_bal = NULL;
	pin_decimal_t		*total_bal = NULL;
	pin_decimal_t		*zerop = NULL;
	pin_decimal_t		*addl_data = NULL;
	char				*quota_code = NULL;
	struct tm			*gm_time;
	char				time_buf[30] = {'\0'};
	pin_cookie_t		cookie = NULL;
	int32				rec_id = 0;
	int32				ncr_found = 0;
	int32				req_res_id = 0;
	int32				*res_id = NULL;
	int32				offer_type = -1;
	char				tmp[200]="";
	pin_decimal_t		*mb_bytes = NULL;
	pin_decimal_t		*tmp_val = NULL;	
	pin_decimal_t		*tmp_val1 = NULL;	

	int32			    *action_flag = NULL;
	pin_flist_t		    *read_oflistp = NULL;
	pin_flist_t		    *bal_obj = NULL;
	pin_flist_t		    *arg_flist = NULL;

	pin_decimal_t		*quota_amtp = NULL;
	poid_t			    *product_pdp = NULL;

	int32			    caller = 0;
	void			    *vp = NULL;
	int32			    *fup_flag = NULL;
	char			    *action = NULL;
	int			        *indicator = NULL;
	pin_decimal_t		*day_dec = NULL;
	pin_decimal_t		*dim_dec = NULL;
	pin_decimal_t		*final_amtp = NULL;
	pin_decimal_t		*diq_val = NULL;
	int32			    day;
	int32			    day_in_month;
	int32			    month;
	time_t			    *chrg_frm = NULL;
	int32			    day_diff;
	pin_decimal_t		*amount = NULL;	
	struct tm       	*timeeff;
	char			    *action_type = NULL;
	char			    *tmp_str = NULL;
    char                *plan_code= NULL;
    char                *no_days = NULL;
	time_t			    *purch_end_t = NULL;
	int			        *grace_days = NULL;
	int                	perr = 0;
	/*
	PCM_OP_BAL_GET_BALANCES
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 2317687 0
	0 PIN_FLD_FLAGS           INT [0] 4
	0 PIN_FLD_BALANCES      ARRAY [*]     NULL array ptr
	
	Out Flist : 
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /balance_group 2315511 3
	0 PIN_FLD_ACCOUNT_OBJ    POID [0] 0.0.0.1 /account 2317687 0
	0 PIN_FLD_BILLINFO_OBJ   POID [0] 0.0.0.1 /billinfo 2316151 0
	0 PIN_FLD_EFFECTIVE_T  TSTAMP [0] (1408121443) Fri Aug 15 22:20:43 2014
	0 PIN_FLD_BALANCES      ARRAY [356] allocated 20, used 12
	1     PIN_FLD_RESERVED_AMOUNT DECIMAL [0] 0
	1     PIN_FLD_CONSUMED_RESERVED_AMOUNT DECIMAL [0] 0
	1     PIN_FLD_NEXT_BAL     DECIMAL [0] 0
	1     PIN_FLD_CREDIT_PROFILE    INT [0] 0
	1     PIN_FLD_CONSUMPTION_RULE   ENUM [0] 0
	1     PIN_FLD_CURRENT_BAL  DECIMAL [0] 3400
	1     PIN_FLD_SUB_BALANCES  ARRAY [0] allocated 20, used 12
	2         PIN_FLD_CONTRIBUTOR_STR    STR [0] ""
	2         PIN_FLD_VALID_TO     TSTAMP [0] (0) <null>
	2         PIN_FLD_VALID_TO_DETAILS    INT [0] 0
	2         PIN_FLD_VALID_FROM   TSTAMP [0] (1408041000) Fri Aug 15 00:00:00 2014
	2         PIN_FLD_VALID_FROM_DETAILS    INT [0] 0
	2         PIN_FLD_CURRENT_BAL  DECIMAL [0] 3400
	2         PIN_FLD_NEXT_BAL     DECIMAL [0] 0
	2         PIN_FLD_DELAYED_BAL  DECIMAL [0] 0
	2         PIN_FLD_ROLLOVER_DATA    INT [0] 0
	2         PIN_FLD_GRANTOR_OBJ    POID [0] 0.0.0.1 /product 2301266 0
	2         PIN_FLD_STATUS         ENUM [0] 1
	2         PIN_FLD_FLAGS           INT [0] 2
	1     PIN_FLD_CURRENT_TOTAL DECIMAL [0] 3400
	1     PIN_FLD_CREDIT_FLOOR DECIMAL [0] NULL
	1     PIN_FLD_CREDIT_LIMIT DECIMAL [0] NULL
	1     PIN_FLD_CREDIT_THRESHOLDS    INT [0] 0
	1     PIN_FLD_CREDIT_THRESHOLDS_FIXED    STR [0] ""
	0 PIN_FLD_REALTIME_CNTR    INT [0] 0
	*/
	//ac_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistpp, PIN_FLD_POID, 0, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_mso_add_create_bal input flist", *i_flistpp);
	action_flag = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_FLAGS, 0, ebufp);

	product_pdp = PIN_FLIST_FLD_GET(*i_flistpp,PIN_FLD_PRODUCT_OBJ, 1, ebufp);
	
	fup_flag = PIN_FLIST_FLD_GET(*i_flistpp,MSO_FLD_FUP_STATUS, 1, ebufp);	

	sb_flistp = PIN_FLIST_ELEM_GET( *i_flistpp, PIN_FLD_SUB_BAL_IMPACTS, PIN_ELEMID_ANY, 1, ebufp);

	vp = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_OPCODE, 1, ebufp );
	if (vp)
	{
		caller = *(int32*)vp;
		PIN_FLIST_FLD_DROP(*i_flistpp, PIN_FLD_OPCODE, ebufp);
	}
	
	mb_bytes = pbo_decimal_from_str("1048576.0",ebufp);
	//Pawan:27-03-2015 Commented below to take the amount from input flist.
	// Uncommented below to add the existing balance to new balance.
	bal_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_POID, (void *)ac_pdp, ebufp);
	PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_FLAGS, (void *)&flag, ebufp);
	/* RAVI adding code for validity*/
	if(*action_flag == DOC_BB_PLAN_VALIDITY_EXTENSION)
	{
		bal_obj = PIN_FLIST_CREATE(ebufp);		
		arg_flist = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_BAL_GRP_OBJ, 0, ebufp);
		PIN_FLIST_FLD_SET(bal_obj, PIN_FLD_POID, (void *)arg_flist, ebufp);
		PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, bal_obj, &bal_o_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "READ_OBJ_FLIST",bal_o_flistp );
	}
	/*end of changes*/	
	else
	{	
	//	if(*action_flag == DOC_BB_FUP_REVERSAL)
	//	{
	//		PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, bal_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	//	}
	//	else
	//	{	
		if(sb_flistp)
				PIN_FLIST_FLD_COPY(sb_flistp, PIN_FLD_BAL_GRP_OBJ, bal_flistp, PIN_FLD_BAL_GRP_OBJ, ebufp);
		//}		
		PIN_FLIST_ELEM_SET( bal_flistp, NULL, PIN_FLD_BALANCES, PIN_ELEMID_ANY, ebufp);
	
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        	                "Get Balances input flist", bal_flistp);
		PCM_OP(ctxp, PCM_OP_BAL_GET_BALANCES, 0, bal_flistp, &bal_o_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                	        "Get Balances output flist", bal_o_flistp)
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			*error_codep = MSO_BB_INTERNAL_ERROR;
             	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                	        "error getting Balances", ebufp);
             	goto cleanup;
		}
	
	}
	

	pvt = pin_virtual_time(NULL);
//	if(*action_flag ==  DOC_BB_FUP_REVERSAL){
//		balance_flistp = PIN_FLIST_ELEM_GET(bal_o_flistp, PIN_FLD_BALANCES, 1000104, 1, ebufp);
//		req_res_id = MSO_FUP_TRACK;
//	}
//	
//	else 
//	{
		if(*plan_type == 1){
			balance_flistp = PIN_FLIST_ELEM_GET(bal_o_flistp, PIN_FLD_BALANCES, 1100010, 1, ebufp);
			req_res_id = MSO_FREE_MB;
		}
		else if(*plan_type == 2){
			balance_flistp = PIN_FLIST_ELEM_GET(bal_o_flistp, PIN_FLD_BALANCES, 1000104, 1, ebufp);
			req_res_id = MSO_FUP_TRACK;
		}
		else if(*plan_type == 0){
			balance_flistp = PIN_FLIST_ELEM_GET(bal_o_flistp, PIN_FLD_BALANCES, 1000103, 1, ebufp);
                        req_res_id = MSO_TRCK_USG;
		}
//	}
	sprintf(tmp,"Req Resource Id is %d",req_res_id);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,tmp);
	quota_code = "DATA";
	//total_bal = pbo_decimal_copy(*initial_amount, ebufp);
	total_bal = pbo_decimal_from_str("0.0",ebufp);
	// Fetch the existing balance for non currency resource which is
	zerop = pbo_decimal_from_str("0.0",ebufp);
	// valid for current date.
	if(balance_flistp)
	{
		while ( (sub_bal_flistp = PIN_FLIST_ELEM_GET_NEXT (balance_flistp,
					   PIN_FLD_SUB_BALANCES, &rec_id, 1,&cookie, ebufp)) != (pin_flist_t *)NULL )
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
							"Sub Bal flist", sub_bal_flistp);

			valid_from = PIN_FLIST_FLD_GET(sub_bal_flistp, PIN_FLD_VALID_FROM, 0,ebufp);
			valid_to = PIN_FLIST_FLD_GET(sub_bal_flistp, PIN_FLD_VALID_TO, 0,ebufp);
			if(*valid_to > pvt )
			{
				//ncr_found = 1;
				curr_bal = PIN_FLIST_FLD_GET(sub_bal_flistp, PIN_FLD_CURRENT_BAL, 0,ebufp);
				if (pbo_decimal_compare(curr_bal, zerop, ebufp) < 0)
				{
					tmp_val = pbo_decimal_multiply(curr_bal,mb_bytes,ebufp);
					pbo_decimal_add_assign(total_bal,tmp_val,ebufp);
					//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Total Balance");
					//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,pbo_decimal_to_str(total_bal,ebufp));
					// Added below to remove the decimal 
					pbo_decimal_round_assign(total_bal, 0, ROUND_UP, ebufp);
					//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,pbo_decimal_to_str(total_bal,ebufp));
				}
			}
		}
	}
	//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Existing Balance");
	//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,pbo_decimal_to_str(total_bal,ebufp));
	/*RAVI added changes for validity ext*/
        if(*action_flag == DOC_BB_PLAN_VALIDITY_EXTENSION)
        {
		goto SKIP;
	}
	/*ended changes*/

	cookie = NULL;
	//if(balance_flistp)
	//{
	while ( (sub_bal_imp_flistp = PIN_FLIST_ELEM_GET_NEXT (*i_flistpp,
				   PIN_FLD_SUB_BAL_IMPACTS, &rec_id, 1,&cookie, ebufp)) != (pin_flist_t *)NULL )
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Sub Bal Impact flist", sub_bal_imp_flistp);
		res_id = PIN_FLIST_FLD_GET(sub_bal_imp_flistp, PIN_FLD_RESOURCE_ID, 0,ebufp);
		sprintf(tmp,"Resource Id is %d",*res_id);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,tmp);
		if(res_id && *res_id == req_res_id)
		{
			sprintf(tmp,"Resource Id matched %d and %d",*res_id,req_res_id);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,tmp);
			sub_bal_flistp = PIN_FLIST_ELEM_GET (sub_bal_imp_flistp,
									PIN_FLD_SUB_BALANCES, PIN_ELEMID_ANY, 1, ebufp); 
			if(sub_bal_flistp)
			{
				valid_from = PIN_FLIST_FLD_GET(sub_bal_flistp, PIN_FLD_VALID_FROM, 0,ebufp);
				valid_to = PIN_FLIST_FLD_GET(sub_bal_flistp, PIN_FLD_VALID_TO, 0,ebufp);
				if(*valid_to > pvt )
				{
					ncr_found = 1;
					//Added below condition to add double amount
					if(caller != MSO_OP_CUST_CREATE_OFFERS){
						curr_bal = PIN_FLIST_FLD_GET(sub_bal_flistp, PIN_FLD_AMOUNT, 0,ebufp);
						tmp_val = pbo_decimal_multiply(curr_bal,mb_bytes,ebufp);
						pbo_decimal_add_assign(total_bal,tmp_val,ebufp);
						//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Total Balance");
						//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,pbo_decimal_to_str(total_bal,ebufp));
						// Added below to remove the decimal 
						pbo_decimal_round_assign(total_bal, 0, ROUND_UP, ebufp);
						//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,pbo_decimal_to_str(total_bal,ebufp));
					}
				}
			}
		}
	}

	/*** check the balance is available for the accounts for which there is grace period***/
	action_type = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_ACTION_TYPE, 1, ebufp);
	if(action_type && (strcmp(action_type,"bb_grace_order") ==0))
        {
                if(pbo_decimal_compare(total_bal, zerop, ebufp) >= 0)
                {
			tmp_val = pbo_decimal_from_str("0.0",ebufp);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"In the renew grace zero balance loop");
                        pin_conf("fm_mso_prov", "bb_grace_amount", PIN_FLDT_DECIMAL, (caddr_t*)&amount, &perr);
			ncr_found = 1;
			if (perr != PIN_ERR_NONE) {
                        	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
                                	"Error in BB grace order while adding GB");
				*error_codep = MSO_BB_INTERNAL_ERROR;
				pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
						PIN_ERR_BAD_VALUE, 0, 0, 0);
				goto cleanup;
                	}
		//	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,pbo_decimal_to_str(amount,ebufp));
			tmp_val = pbo_decimal_multiply(amount,mb_bytes,ebufp);	
			pbo_decimal_add_assign(total_bal, tmp_val,ebufp);
			//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Total Balance after addition");
                      //  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,pbo_decimal_to_str(total_bal,ebufp));

                }
        }
	//}
	/*else
	{
		if(*action_flag != DOC_BB_ACTIVATION)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Quota Balance not found " );	
			*error_codep = MSO_BB_INTERNAL_ERROR;
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_BAD_VALUE, PIN_FLD_BALANCES, 0, 0);	
			goto cleanup;
		}
	}*/


//	balance_flistp = PIN_FLIST_ELEM_GET(bal_o_flistp, PIN_FLD_BALANCES, 1100010, 1, ebufp);
//	if(balance_flistp)
//	{
//		quota_code = "DATA";
//		while ( (sub_bal_flistp = PIN_FLIST_ELEM_GET_NEXT (balance_flistp,
//                       PIN_FLD_SUB_BALANCES, &rec_id, 1,&cookie, ebufp))
//                                                  != (pin_flist_t *)NULL ) 
//	        {
//	        	valid_from = PIN_FLIST_FLD_GET(sub_bal_flistp, PIN_FLD_VALID_FROM, 0,ebufp);
//			valid_to = PIN_FLIST_FLD_GET(sub_bal_flistp, PIN_FLD_VALID_TO, 0,ebufp);
//			if(*valid_to > *pvt )
//			{
//				curr_bal = PIN_FLIST_FLD_GET(sub_bal_flistp, PIN_FLD_CURRENT_BAL, 0,ebufp);
//				total_bal = pbo_decimal_add(total_bal,curr_bal,ebufp);
//			}
//		}
//	}
	
	//Setting the fields to output flist.
	//balflistp = PIN_FLIST_ELEM_ADD(*createbalflistp, MSO_FLD_CREATEBALANCE, 1, ebufp);
SKIP:
	plan_pdp = (poid_t *)PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_PLAN_OBJ, 0, ebufp);
	plan_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(plan_iflistp, PIN_FLD_POID, plan_pdp, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read input flist", plan_iflistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, plan_iflistp, &plan_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read output flist", plan_oflistp);
	
	plan_name = (char *)PIN_FLIST_FLD_GET(plan_oflistp, PIN_FLD_NAME, 0, ebufp);
	mso_plan = (poid_t *)PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_PURCHASED_PLAN_OBJ, 0, ebufp);
	srv_pdp = (poid_t *)PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
    plan_code = (char *)PIN_FLIST_FLD_GET(plan_oflistp,PIN_FLD_CODE,0,ebufp);
	//PIN_FLIST_FLD_SET(balflistp, PIN_FLD_CODE, plan_name , ebufp);
	//PIN_FLIST_FLD_SET(balflistp, MSO_FLD_QUOTACODE, quota_code , ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, plan_name);
	now = pin_virtual_time (NULL);
    if ((strcmp(other_ne_id,HW_NETWORK_STER_ID) != 0))
    {
	    gm_time = gmtime(&now);
	    strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%T.000+0530", gm_time);
    }
    else
    {
        sprintf(time_buf, "%d000", now);
    }
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Time :");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, time_buf);
	PIN_FLIST_FLD_SET(*createbalflistp, MSO_FLD_STARTDATE, (void *)time_buf, ebufp);
	
		
	if ( action_flag && (!fup_flag) &&(*action_flag == DOC_BB_FUP_REVERSAL || *action_flag == DOC_BB_AUTO_RENEW_POST))
	{
		vp = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_CHARGED_FROM_T, 1, ebufp );
                if (vp)
                {
                        start = *(time_t*)vp ;
                }
                vp = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_CHARGED_TO_T, 1, ebufp );
                if (vp)
                {
                        expiry = *(time_t*)vp;
                }
	}
	else if( action_type && (strcmp(action_type,"bb_grace_order") ==0))
        {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Add the grace date to the account");
                purch_end_t = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_PURCHASE_END_T, 0, ebufp );
                pin_conf("fm_mso_prov", "bb_grace_period", PIN_FLDT_INT, (caddr_t*)&grace_days, &perr);
                timeeff = localtime(purch_end_t);
                timeeff->tm_mday = timeeff->tm_mday + *grace_days;
                expiry = mktime (timeeff);
		if (perr != PIN_ERR_NONE) {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
                                "Error in BB grace order while adding grace date");
			*error_codep = MSO_BB_INTERNAL_ERROR;
                        pin_set_err(ebufp, PIN_ERRLOC_FM,
                                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                                	PIN_ERR_BAD_VALUE, 0, 0, 0);
			goto cleanup;
                }
    //aggregation value
        }
	else if(*action_flag == DOC_BB_PLAN_VALIDITY_EXTENSION || *plan_type == 0)
	{
		expiry = fm_prov_get_expiry(ctxp, mso_plan, ac_pdp, srv_pdp, srv_typ, &start, ebufp);
	}
	else
	{
		expiry = fm_prov_get_expiry_from_calc_only_out(ctxp, *i_flistpp,srv_typ, ebufp);
	}
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_INTERNAL_ERROR;
             PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "error getting Balances", ebufp);
             goto cleanup;
	}
	if(expiry){
		//gm_time = gmtime(&expiry);
		expiry = expiry -1;
        if ((strcmp(other_ne_id,HW_NETWORK_STER_ID) != 0))
        {
		    gm_time = localtime(&expiry);
		    strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%T.000+0530", gm_time);
        }
        else
        {
            sprintf(time_buf, "%d000", expiry);
        }
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Expiry Time :");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, time_buf);
		PIN_FLIST_FLD_SET(*createbalflistp, MSO_FLD_EXPIRATIONDATE, (void *)time_buf, ebufp);
	}
	if(start){
        if ((strcmp(other_ne_id,HW_NETWORK_STER_ID) != 0))
        {
		    gm_time = gmtime(&start);
		    strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%T.000+0530", gm_time);
        }
        else
        {
            sprintf(time_buf, "%d000", start);
        }
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Start Time :");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, time_buf);
		PIN_FLIST_FLD_SET(*createbalflistp, MSO_FLD_STARTDATE, (void *)time_buf, ebufp);
	}
    plan_code = NULL;
    plan_code = PIN_FLIST_FLD_GET(*createbalflistp, MSO_FLD_CODE, 1, ebufp);
    if (other_ne_id && (strcmp(other_ne_id,HW_NETWORK_STER_ID) == 0))
    {
        if(plan_code && strstr(plan_code,"FUPDR"))
        {
            no_days = "DAILY";
        }
        else
        {
            no_days= "BILLING_CYCLE";
        }
    }
    PIN_FLIST_FLD_SET(*createbalflistp, PIN_FLD_VALUE, no_days, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Create_bal_validity", *createbalflistp);
	if(PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_INTERNAL_ERROR;
             PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "error getting Balances", ebufp);
             goto cleanup;
	}
	/*RAVI added changes*/
        if(*action_flag == DOC_BB_PLAN_VALIDITY_EXTENSION && *plan_type == 0)
        {
	   if(srv_typ == 1)
	   {
		*initial_amount = NULL;
		*initial_amount = pbo_decimal_from_str((char *)UNLIMITED_QUOTA, ebufp);
		tmp_str = pbo_decimal_to_str(*initial_amount, ebufp);
		PIN_FLIST_FLD_SET(*createbalflistp, MSO_FLD_INITIALAMOUNT, tmp_str, ebufp);
		if(tmp_str != NULL && tmp_str)
			pin_free(tmp_str);
	   }
	   else
	   {
		 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"VALIDITY EXTENSION IS NOT APPLICABLE FOR POSTPAID UNLIMITED CASE" );
                        *error_codep = MSO_BB_INTERNAL_ERROR;
                        pin_set_err(ebufp, PIN_ERRLOC_FM,
                                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                                PIN_ERR_BAD_VALUE, 0, 0, 0);
		 PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "Validity Extension is not applicable for Postpaid Unlimited Plan", ebufp);
                        goto cleanup;
	   }
	
        }
	else if(*action_flag == DOC_BB_PLAN_VALIDITY_EXTENSION)
	{
		pbo_decimal_negate_assign(total_bal,ebufp);
		if (pbo_decimal_compare(total_bal, zerop, ebufp) == 1)
		{			
					tmp_str = pbo_decimal_to_str(total_bal, ebufp);
                	PIN_FLIST_FLD_SET(*createbalflistp, MSO_FLD_INITIALAMOUNT, tmp_str, ebufp);
					if(tmp_str != NULL && tmp_str)
						pin_free(tmp_str);
		}	 		
	}
	else if(*plan_type == 0 )
	{
		*initial_amount = NULL;
                *initial_amount = pbo_decimal_from_str((char *)UNLIMITED_QUOTA, ebufp);
				tmp_str = pbo_decimal_to_str(*initial_amount, ebufp);
                PIN_FLIST_FLD_SET(*createbalflistp, MSO_FLD_INITIALAMOUNT, tmp_str, ebufp);
				if(tmp_str != NULL && tmp_str)
					pin_free(tmp_str);
	}	
	/*ended changes*/
	else
	{
		if(ncr_found){
			//pbo_decimal_add_assign(*initial_amount, total_bal, ebufp);
			pbo_decimal_negate_assign(total_bal,ebufp);
			tmp_str =  pbo_decimal_to_str(total_bal, ebufp);
			PIN_FLIST_FLD_SET(*createbalflistp, MSO_FLD_INITIALAMOUNT, tmp_str, ebufp);
			if(tmp_str != NULL && tmp_str)
					pin_free(tmp_str);
		}
		else {
			// Commented below line and added error when the sub balance is not present in provisioning input 
			// flist in order to AVOID sending amount configured in mso_config_bb_pt_t.

			mso_prov_fetch_quota_amount(ctxp,product_pdp,req_res_id,&quota_amtp,ebufp);
			/*prorating the amount incase odf hold*/			
			action = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_ACTION, 1, ebufp);			
			indicator = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_INDICATOR, 1, ebufp);
			if(action && indicator && *indicator == 0 && strstr(action, "UNHOLD_SERVICES_DOCSIS"))
			{
			   chrg_frm = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_CHARGED_FROM_T, 1, ebufp);	
			   day = mso_get_days_from_timestamp(*chrg_frm);
			   
			   month = mso_get_months_from_timestamp(*chrg_frm);
			   
			   if (month== 0 || month== 2 || month == 4 || month == 6 || month == 7 || month == 9 || month == 11)
                                        day_in_month = 31;
                                  else if (month == 1)
                                        day_in_month = 28;
                                  else
                                        day_in_month = 30;
			   day_diff = day_in_month - day + 1;
			   			    
			   day_dec = pbo_decimal_from_double((double)day_diff, ebufp);
			   dim_dec = pbo_decimal_from_double((double)day_in_month, ebufp);
			   diq_val = pbo_decimal_multiply(day_dec,quota_amtp,ebufp);
			   if(pbo_decimal_is_null(diq_val, ebufp)==0)
			   {
				   	final_amtp= pbo_decimal_divide(diq_val, dim_dec,ebufp);
			   }
			   if(pbo_decimal_is_null(final_amtp, ebufp)==0)
			   {
				
			   	tmp_val1 = pbo_decimal_multiply(final_amtp,mb_bytes,ebufp);
				pbo_decimal_round_assign(tmp_val1, 0, ROUND_UP, ebufp);
			   }
			   	
			 }
			else
			{
				tmp_val1 = pbo_decimal_multiply(quota_amtp,mb_bytes,ebufp);
			}
			//Added for DATA OFFERS check - AD - 26-04-2017
			srch_iflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_POID, srch_iflistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
			offer_type = 2;
			PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TYPE, &offer_type, ebufp);
			fm_mso_search_offer_entries(ctxp, srch_iflistp, &srch_oflistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PROV FUP OFFER RESULTS", srch_oflistp);
			if (srch_oflistp && PIN_FLIST_ELEM_COUNT(srch_oflistp, PIN_FLD_RESULTS, ebufp) > 0){
				offer_flistp = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1,ebufp);
				if(offer_flistp){
					addl_data = PIN_FLIST_FLD_GET(offer_flistp, PIN_FLD_AMOUNT, 1, ebufp);
					if(!pbo_decimal_is_null(addl_data,  ebufp) || !pbo_decimal_is_zero(addl_data,  ebufp)){
						//Add additional offer data to existing balance
						pbo_decimal_add_assign(tmp_val1, pbo_decimal_multiply(addl_data, mb_bytes,ebufp), ebufp);
					}
				}
			}
			PIN_FLIST_FLD_PUT(*createbalflistp, MSO_FLD_INITIALAMOUNT, pbo_decimal_to_str(tmp_val1, ebufp) , ebufp);
			PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"createbalance flist",*createbalflistp);
			//PIN_FLIST_FLD_SET(*createbalflistp, MSO_FLD_INITIALAMOUNT, pbo_decimal_to_str(*initial_amount, ebufp) , ebufp);

			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Quota Sub Balance not found in input flist for provisioning." );	
		     }	
		
		}
	
	//if(!pbo_decimal_is_null(curr_bal, ebufp))
	//{
	
	
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			*error_codep = MSO_BB_INTERNAL_ERROR;
             	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                 	       "error getting Balances", ebufp);
             	goto cleanup;
		}
	//}
	//}
cleanup :
	PIN_FLIST_DESTROY_EX(&bal_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&bal_o_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&plan_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&plan_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&bal_obj, NULL);
	if(srch_iflistp)PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
        pbo_decimal_destroy(&zerop);
	if (!pbo_decimal_is_null(day_dec, ebufp))
        {
                pbo_decimal_destroy(&day_dec);
        }
	if (!pbo_decimal_is_null(dim_dec, ebufp))
        {
		pbo_decimal_destroy(&dim_dec);
	}
	if (!pbo_decimal_is_null(diq_val, ebufp))
        {
                pbo_decimal_destroy(&diq_val);
        }
	if (!pbo_decimal_is_null(final_amtp, ebufp))
        {
                pbo_decimal_destroy(&final_amtp);
        }
	if(!pbo_decimal_is_null(quota_amtp,ebufp)) pbo_decimal_destroy(&quota_amtp);
	if (!pbo_decimal_is_null(amount, ebufp))
        {
                pbo_decimal_destroy(&amount);
        }
	if (!pbo_decimal_is_null(tmp_val, ebufp))
        {
                pbo_decimal_destroy(&tmp_val);
        }
	if(grace_days)
		free(grace_days);
	return ;
	
}

/*******************************************************************
 * fm_mso_add_cable_modem()
 *
 * returns CABLE_MODEM ARRAY.
  *******************************************************************/
static void
fm_mso_add_cable_modem(
	pcm_context_t			*ctxp,
	pin_flist_t			**i_flistpp,
	pin_flist_t			*so_flistp,
	int32				*action,
	pin_flist_t			**cablemodemflistp,
	int				*error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*balflistp = NULL;
	pin_flist_t			*deviceflistp = NULL;
	pin_flist_t			*temp_flistp = NULL;
	pin_flist_t			*ip_flistp = NULL;
	pin_flist_t			*cableflistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*rfld_iflistp = NULL;
	pin_flist_t			*rfld_oflistp = NULL;
	pin_flist_t			*test_flistp = NULL;
	poid_t				*ac_pdp = NULL;
	poid_t				*serv_poid = NULL;
	poid_t				*plan_pdp = NULL;
	char				param[50];
	char				*mac_add = NULL;
	char				*prov_tag = NULL;
	char				*dev_type = NULL;
	char				*srv_code = NULL;
	int32				ip_count = 0;
	int				    *isTAL = 0;
	int				    flag = 0;
	int				    *tech = NULL;
	char				*client_class = NULL;
	char				*ip_add = NULL;
	char				*ip_add_list = NULL;
	char				*mso_ip_add = NULL;
	char				ip_count_str[4];
	char				*node_name = NULL;
	char				device_typ[50];
	char				*mso_id = NULL;
	char				*device_tech_type = NULL;
    char                *str_rolep = NULL;
	pin_cookie_t		cookie = NULL;
	int32				rec_id = 0;
	pin_cookie_t		cookie1 = NULL;
	int32				rec_id1 = 0;
	//int				*action = 0;
	int                 *bill_when = NULL;
	char				*city = NULL;
	int32				*fup_status = NULL;
	poid_t				*old_plan_pdp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_add_cable_modem:Input flist",*i_flistpp);

	ac_pdp = (poid_t *)PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_POID, 0, ebufp);
	serv_poid = (poid_t *)PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
	plan_pdp = (poid_t *)PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_PLAN_OBJ, 0, ebufp);
	fup_status = PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_FUP_STATUS, 1, ebufp);
	old_plan_pdp = (poid_t *)PIN_FLIST_FLD_GET(*i_flistpp, MSO_FLD_OLD_PLAN_OBJ, 1, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_add_cable_modem:so flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	isTAL = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_ISTAL_FLAG, 0, ebufp);
    str_rolep = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_ROLES, 0, ebufp);
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp);
	city = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 0, ebufp);
	bill_when = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);	

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_MANDATORY_FIELD_CODE;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG, "fm_mso_add_cable_modem: Error", ebufp);
		return;
	}
	//action = PIN_FLIST_FLD_GET(*i_flistp, PIN_FLD_FLAGS, 0, ebufp);
	if(*action != DOC_BB_ACTIVATION)
	{
		//mso_id = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_ID, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			*error_codep = MSO_BB_ID_ERROR;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG, "fm_mso_add_cable_modem: Error", ebufp);
			return;
		}
	}
	//Read the Plan Object to get the Plan Name
	rfld_iflistp = PIN_FLIST_CREATE(ebufp);
	if(old_plan_pdp)
	{
		PIN_FLIST_FLD_SET(rfld_iflistp, PIN_FLD_POID, (void *)old_plan_pdp, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(rfld_iflistp, PIN_FLD_POID, (void *)plan_pdp, ebufp);
	}
	PIN_FLIST_FLD_SET(rfld_iflistp, PIN_FLD_NAME, (void *)NULL, ebufp);

	//Call the Read Fields Opcode
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, rfld_iflistp, &rfld_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG, "fm_mso_add_cable_modem: Error", ebufp);
		goto cleanup;
	}
	//Get the Plan name
	srv_code = (char *)PIN_FLIST_FLD_GET(rfld_oflistp, PIN_FLD_NAME, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG, "fm_mso_add_cable_modem: Error", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_add_cable_modem: Plan Name");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, srv_code);

	//Free memory
	PIN_FLIST_DESTROY_EX(&rfld_iflistp, NULL);
	//PIN_FLIST_DESTROY_EX(&rfld_oflistp, NULL);

	strcpy(param,"/device/modem");
	/* get the modem device if any */
	fm_mso_search_devices(ctxp, param, serv_poid, &deviceflistp, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_add_cable_modem: Error",ebufp);
		goto cleanup;
	}
	cookie = NULL;
	rec_id =0;
	while ( (temp_flistp = PIN_FLIST_ELEM_GET_NEXT (deviceflistp,
                       PIN_FLD_RESULTS, &rec_id, 1,&cookie, ebufp))
                                                  != (pin_flist_t *)NULL ) 
    {
        mac_add = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_DEVICE_ID, 0,ebufp);
        device_tech_type = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_DEVICE_TYPE, 0,ebufp);
		/**** Pavan Bellala 29-07-2015 ***********************************
		 Defect 1136 : Validate device string pattern before provisioning
		*****************************************************************/
		if(mac_add && strcmp(mac_add,"")!=0)
		{
			//Added the below code for JIO-CPS Integration-ISP Enhancement
			//Skip MAC validation for ONU (GPON) modem for JIO and Hathway Networks and validate the device_id pattern for other device types.
			if((device_tech_type && 
			   !(strcmp(device_tech_type, HW_GPON_MODEM) == 0 || 
				strcmp(device_tech_type, HW_ONU_GPON_MODEM) == 0 || 
			    strcmp(device_tech_type, JIO_GPON_MODEM) == 0))
				&& fm_mso_integ_match_patterns("^([0-9a-fA-F]{2})(([/\\s:-][0-9a-fA-F]{2}){5})$", mac_add) <=0 )
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR , "Error:Invalid MAC id");
				pin_set_err(ebufp, PIN_ERRLOC_FM,
							PIN_ERRCLASS_SYSTEM_DETERMINATE,PIN_ERR_BAD_VALUE, PIN_FLD_DEVICE_ID, 0, 0);
				*error_codep = MSO_BB_INVALID_MAC;
				goto cleanup;
			}
		}



        	dev_type = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_DEVICE_TYPE, 0,ebufp);
        	ip_count = 0;
		strcpy(device_typ, "/device/modem");
	}
	
	strcpy(param,"/device/router/cable");
	/* get the router device if any */
	fm_mso_search_devices(ctxp,param,serv_poid,&deviceflistp, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_activation_flist: Error",ebufp);
		goto cleanup;
	}
	cookie = NULL;
	rec_id =0;
	while ( (temp_flistp = PIN_FLIST_ELEM_GET_NEXT (deviceflistp,
                       PIN_FLD_RESULTS, &rec_id, 1,&cookie, ebufp))
                                                  != (pin_flist_t *)NULL ) 
        {
        	mac_add = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_DEVICE_ID, 0,ebufp);
		/**** Pavan Bellala 29-07-2015 ***********************************
		 Defect 1136 : Validate device string pattern before provisioning
		*****************************************************************/
		if(mac_add && strcmp(mac_add,"")!=0)
		{
			if((tech && *tech != 5 ) && fm_mso_integ_match_patterns("^([0-9a-fA-F]{2})(([/\\s:-][0-9a-fA-F]{2}){5})$", mac_add) <=0 )
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error:Invalid MAC id");
				pin_set_err(ebufp, PIN_ERRLOC_FM,
							PIN_ERRCLASS_SYSTEM_DETERMINATE,PIN_ERR_BAD_VALUE, PIN_FLD_DEVICE_ID, 0, 0);
				*error_codep = MSO_BB_INVALID_MAC;
				goto cleanup;
			}
		}

        	dev_type = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_DEVICE_TYPE, 0,ebufp);
        	ip_count = 0;
		//strcpy(device_typ, "/device/modem");
		strcpy(device_typ, "/device/router/cable");
	}
	
	//param = "/device/ip/static";
	strcpy(param,"/device/ip/static");
	/* get the ip/static device if any */
	fm_mso_search_devices(ctxp,param,serv_poid,&deviceflistp, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_activation_flist: Error",ebufp);
		goto cleanup;
	}
	cookie = NULL;
	rec_id = 0;
	flag = 0;
	ip_count = PIN_FLIST_ELEM_COUNT(deviceflistp, PIN_FLD_RESULTS, ebufp);
	while ( (temp_flistp = PIN_FLIST_ELEM_GET_NEXT (deviceflistp,
                       PIN_FLD_RESULTS, &rec_id, 1,&cookie, ebufp))
                                                  != (pin_flist_t *)NULL ) 
        {
        	ip_add = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_DEVICE_ID, 0,ebufp);
        	dev_type = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_DEVICE_TYPE, 0,ebufp);
			//Pawan:17-03-2015: Added below and commented next block
			if(flag == 1)
			{ 
				ip_add_list = (char *)realloc(ip_add_list,(strlen(ip_add_list)+strlen(ip_add)+2));
				strcat(ip_add_list, ",");
				strcat(ip_add_list, ip_add);
			}
			else{
				ip_add_list = malloc(strlen(ip_add)+1);
				sprintf(ip_add_list, "%s", ip_add);
			}
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ip_add_list: ");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, ip_add_list);
			flag = 1;
			
        	//client_class = "in_progress";
        	//ip_add_list = (char *)pin_malloc(1);
        /*	cookie1 = NULL;
		rec_id1 = 0;
        	ip_flistp = PIN_FLIST_ELEM_GET(temp_flistp, MSO_FLD_IP_DATA, 0, 1, ebufp);
                if(ip_flistp)
		{
			mso_ip_add = PIN_FLIST_FLD_GET(ip_flistp, MSO_FLD_IP_ADDRS, 0,ebufp);
			if(flag == 1)
			{ 
				ip_add_list = (char *)realloc(ip_add_list,(strlen(ip_add_list)+strlen(mso_ip_add)+2));
				strcat(ip_add_list, ",");
				strcat(ip_add_list, mso_ip_add);
			}
			else{
				ip_add_list = malloc(strlen(mso_ip_add)+1);
				sprintf(ip_add_list, "%s", mso_ip_add);
			}
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ip_add_list: ");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, ip_add_list);
			flag = 1;
		}
		*/
	}
	
//	strcpy(param,"/device/ip/framed");
//	/* get the ip/static device if any */
//	fm_mso_search_devices(ctxp,param,serv_poid,&deviceflistp, error_codep, ebufp);
//	if (PIN_ERRBUF_IS_ERR(ebufp))
//	{
//		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
//			"prepare_bb_activation_flist: Error",ebufp);
//		goto cleanup;
//	}
//	cookie = NULL;
//	rec_id = 0;
//	while ( (temp_flistp = PIN_FLIST_ELEM_GET_NEXT (deviceflistp,
//                       PIN_FLD_RESULTS, &rec_id, 1,&cookie, ebufp))
//                                                  != (pin_flist_t *)NULL ) 
//        {
//        	ip_add = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_DEVICE_ID, 0,ebufp);
//        	dev_type = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_DEVICE_TYPE, 0,ebufp);
//        	//client_class = "in_progress";
//        	ip_count = PIN_FLIST_ELEM_COUNT(temp_flistp, MSO_FLD_IP_ADDRS, ebufp);
//        	flag = 0;
//        	ip_add_list = (char *)pin_malloc(1);
//        	cookie1 = NULL;
//		rec_id1 = 0;
//        	while ( (ip_flistp = PIN_FLIST_ELEM_GET_NEXT (temp_flistp,
//                       MSO_FLD_IP_DATA, &rec_id1, 1,&cookie1, ebufp))
//                                                  != (pin_flist_t *)NULL )
//		{
//			mso_ip_add = PIN_FLIST_FLD_GET(ip_flistp, MSO_FLD_IP_ADDRS, 0,ebufp);
//			ip_add_list = (char *)realloc(ip_add_list,(sizeof(ip_add_list)+sizeof(mso_ip_add))+2);
//			if(flag == 1)
//			{ 
//				strcat(ip_add_list, ",");
//			}
//			strcat(ip_add_list, mso_ip_add);
//			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ip_add_list: ");
//			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, ip_add_list);
//			flag = 1;
//			//device_typ = param;
//		}
//	}
	
	

	node_name = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NE_ELEMENT_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_activation_flist: Network Element Missing",ebufp);
		goto cleanup;
	}
	
	prov_tag = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_PROVISIONING_TAG, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_PROV_TAG_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_activation_flist: Prov Tag missing",ebufp);
		goto cleanup;
	}

	/**** Pavan Bellala 24-Nov-2015 *************************
	Added condition to check macid is not NULL else throw err
	*********************************************************/
	if(mac_add == NULL || (mac_add && strcmp(mac_add,"")==0))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR , "Error:Invalid MAC id NULL string");
		pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,PIN_ERR_BAD_VALUE, PIN_FLD_DEVICE_ID, 0, 0);
		*error_codep = MSO_BB_INVALID_MAC;
		goto cleanup;
	}

	fm_mso_search_client_classes(ctxp, prov_tag, node_name, srv_code, str_rolep, action, isTAL, 
		ip_count, tech, device_typ, &client_class, bill_when, city, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_activation_flist: Error",ebufp);
		goto cleanup;
	}
	if(client_class){
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Client Class");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, client_class);
	}
	else{
		*error_codep = MSO_BB_CLIENT_CLASS_ERROR;
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_NOT_FOUND , MSO_FLD_CLIENT_CLASSES, 0, 0);
		goto cleanup;
	}
	sprintf(ip_count_str,"%d",ip_count);
	//test_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "cablemodemflistp flist", *cablemodemflistp);
	//cableflistp = PIN_FLIST_ELEM_ADD(*cablemodemflistp, MSO_FLD_CABLE_MODEM, 1, ebufp);
	PIN_FLIST_FLD_SET(*cablemodemflistp, MSO_FLD_RECORD_NAME, mac_add, ebufp);
	//PIN_FLIST_FLD_SET(cableflistp, MSO_FLD_RECORD_DESCRIPTION, (void *)rec_descr, ebufp);
	if(*tech == 1)
	{
		PIN_FLIST_FLD_SET(*cablemodemflistp, MSO_FLD_RECORD_DESCRIPTION, "DOCSIS-2", ebufp);
	}
	else{
		PIN_FLIST_FLD_SET(*cablemodemflistp, MSO_FLD_RECORD_DESCRIPTION, "DOCSIS-3", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "cablemodemflistp flist", *cablemodemflistp);
	//Set the ID
	//if(mso_id)
	//	PIN_FLIST_FLD_SET(*cablemodemflistp, MSO_FLD_ID, mso_id, ebufp);
	PIN_FLIST_FLD_SET(*cablemodemflistp, MSO_FLD_ENTITYID, "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF", ebufp);
	PIN_FLIST_FLD_SET(*cablemodemflistp, MSO_FLD_MAC_ADDRESS, mac_add, ebufp);
    if(str_rolep && strstr(str_rolep, "-Static"))
    {
       PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "INSIDE:::IF");
        if(ip_add_list){
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "INNER:::IF");
            PIN_FLIST_FLD_SET(*cablemodemflistp,
                MSO_FLD_SUBSCRIBED_STATICADDRESS_COUNT, ip_count_str, ebufp);
            PIN_FLIST_FLD_SET(*cablemodemflistp,
                MSO_FLD_IPADDRESSES, ip_add_list, ebufp);
        }
        else{
            *error_codep = MSO_BB_IP_DEVICE_ERROR;
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NOT_FOUND , PIN_FLD_IP_ADDRESS, 0, 0);
            goto cleanup;
        }
    }

	else if(*isTAL && other_ne_id && (strcmp(other_ne_id,HW_NETWORK_STER_ID) != 0))
	{
		if(ip_add_list){
			PIN_FLIST_FLD_SET(*cablemodemflistp, 
				MSO_FLD_SUBSCRIBED_STATICADDRESS_COUNT, ip_count_str, ebufp);
			PIN_FLIST_FLD_SET(*cablemodemflistp, 
				MSO_FLD_IPADDRESSES, ip_add_list, ebufp);
		}
		else{
			*error_codep = MSO_BB_IP_DEVICE_ERROR;
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_NOT_FOUND , PIN_FLD_IP_ADDRESS, 0, 0);
			goto cleanup;
		}
	}
	else
	{
		PIN_FLIST_FLD_SET(*cablemodemflistp,
			MSO_FLD_SUBSCRIBED_STATICADDRESS_COUNT, "0", ebufp);
	}
	if(client_class){
		PIN_FLIST_FLD_SET(*cablemodemflistp, 
			MSO_FLD_CLIENT_CLASS_VALUE, (void *)client_class, ebufp);
	}
	//*cablemodemflistp = PIN_FLIST_COPY(test_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "cablemodemflistp flist", *cablemodemflistp);
	
cleanup :
	PIN_FLIST_DESTROY_EX(&rfld_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&rfld_oflistp, NULL);
	//PIN_FLIST_DESTROY_EX(&test_flistp, NULL);
	if(client_class)
		free(client_class);
	if(ip_add_list)
		free(ip_add_list);
	return ;
	
}

/*******************************************************************
 * fm_mso_search_devices()
 *
 * returns Associated devices.
  *******************************************************************/
void
fm_mso_search_devices(
	pcm_context_t		*ctxp,
	char				*param,
	poid_t				*service_obj,
	pin_flist_t			**deviceflistp,
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*search_dev_flistp = NULL;
	pin_flist_t			*arg_flistp = NULL;
	pin_flist_t			*arg2_flistp = NULL;
	pin_flist_t			*search_dev_o_flistp = NULL;
	poid_t				*pdp = NULL;
	poid_t				*s_pdp = NULL;
	poid_t				*dev_poid = NULL;
	//char				s_template[200] = "\0";
	int64              	database = 0;
	int32              	s_flags = 0;
	
	/*Search flist to get the device details
	     
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS           INT [0] 768
	0 PIN_FLD_TEMPLATE        STR [0] " select X from /device/$1 where F1 = V1 "
	0 PIN_FLD_PARAMETERS      STR [0] "ip/static"
	0 PIN_FLD_RESULTS       ARRAY [0]     NULL array ptr
	0 PIN_FLD_ARGS          ARRAY [1] allocated 20, used 1
	1     PIN_FLD_SERVICES  ARRAY [1] allocated 20, used 1
	2     PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/telco/broadband 2397687 0
	*/
	
	search_dev_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside fm_mso_search_devices");
	/***********************************************************
	 * assume db matches userid found in pin.conf
	 ***********************************************************/
	pdp = PCM_GET_USERID(ctxp);
	database = PIN_POID_GET_DB(pdp);

	s_pdp = PIN_POID_CREATE(database, "/search", -1, ebufp);
	dev_poid = PIN_POID_CREATE(database, param, -1, ebufp);
	
	PIN_FLIST_FLD_PUT(search_dev_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_FLD_SET(search_dev_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
	
	PIN_FLIST_FLD_SET(search_dev_flistp, PIN_FLD_TEMPLATE,
		"select X from /device where F1 = V1 and F2.type = V2 " , ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(search_dev_flistp, PIN_FLD_ARGS, 1, ebufp);
	arg2_flistp = PIN_FLIST_ELEM_ADD(arg_flistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(arg2_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(search_dev_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_PUT(arg_flistp, PIN_FLD_POID, dev_poid, ebufp);
	if (strstr(param,"/device/modem") !=NULL)
	{
		PIN_FLIST_FLD_SET(search_dev_flistp, PIN_FLD_TEMPLATE,
			"select X from /device where F1 = V1 and F2.type = V2 and F3 != V3 " , ebufp);
		arg_flistp = PIN_FLIST_ELEM_ADD(search_dev_flistp, PIN_FLD_ARGS, 3, ebufp );
		PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_DEVICE_TYPE, OTT_MODEM, ebufp);
	}
       PIN_FLIST_ELEM_SET( search_dev_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Get Associated Device input flist", search_dev_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_dev_flistp, &search_dev_o_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Get Associated Device output flist", search_dev_o_flistp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_INTERNAL_ERROR;
             PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "error getting Associated Device", ebufp);
             goto cleanup;
	}
	
	*deviceflistp = PIN_FLIST_COPY(search_dev_o_flistp, ebufp);	
	
cleanup:
	PIN_FLIST_DESTROY_EX(&search_dev_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_dev_o_flistp, NULL);
	return;
}

int
fm_quota_check(
	pcm_context_t		*ctxp, 
	char				*plan_name, 
	char				*prov_tag, 
	int			    	*tech,
	int				    *plan_typ,
	pin_decimal_t		**initial_amount,
	int				    *error_codep, 
	pin_errbuf_t		*ebufp)
{
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*cache_flistp = NULL;
	cm_cache_key_iddbstr_t		kids;
        pin_cookie_t			cookie = NULL;
        int32			    	rec_id = 0;
	int32				err = PIN_ERR_NONE;
	pin_decimal_t		*amount = NULL;
	int				    *tec = 0;
	int				    srv_typ = 0;
	int				    *type = NULL;
	int 			    found = 0;
	time_t				last_mod_t = 0;
	time_t				now_t = 0;
	char 				temp[100]="";

	FILE				*filep = NULL;

	filep 		= fopen("/tmp/stackt","a+");

	if(PIN_ERRBUF_IS_ERR(ebufp)) {
		return 3;
	}
        /******************************************************
        * If the cache is not enabled, short circuit right away
        ******************************************************/
        if ( mso_prov_bb_config_ptr == (cm_cache_t *)NULL ) {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
                "no config flist for /mso_config_bb_pt cached ");
                pin_set_err(ebufp, PIN_ERRLOC_CM,
                                PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_FLD_CACHE_NAME, 0, 0);

        }
        /**************************************************
         * See if the entry is in our data dictionary cache
         * Prepare the cm_cache_key_iddbstr_t structure to search
         **************************************************/
        kids.id = 0;
        kids.db = 0;
        kids.str = "/mso_config_bb_pt";
        cache_flistp = cm_cache_find_entry(mso_prov_bb_config_ptr,
                                        &kids, &err);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "cm_cache_find_entry found flist", cache_flistp);
        switch(err) {
        case PIN_ERR_NONE:
                break;
        default:
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
                        "mso_prov_bb_config_object from_cache: error "
                        "accessing data dictionary cache.");
			*error_codep = MSO_BB_CONFIG_PT_ERROR;
                pin_set_err(ebufp, PIN_ERRLOC_CM,
                                PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_FLD_CACHE_NAME, 0, 0);
                goto cleanup;
        }
	last_mod_t = *(time_t*)PIN_FLIST_FLD_GET(cache_flistp, PIN_FLD_MOD_T,0,ebufp);
	now_t = pin_virtual_time((time_t *)NULL);
	if(now_t  >= last_mod_t + refresh_pt_interval )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"calling mso_prov_bb_config_object_update to refresh provisioning tag config");
		fm_mso_prov_bb_config_cache_update(ctxp, ebufp);
		kids.id = 0;
		kids.db = 0;
		kids.str = "/mso_config_bb_pt";

		if(cache_flistp != NULL)PIN_FLIST_DESTROY_EX(&cache_flistp, NULL);
		cache_flistp = cm_cache_find_entry(mso_prov_bb_config_ptr,&kids, &err);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"cm_cache_find_entry found flist", cache_flistp);
		switch(err)
		{
			case PIN_ERR_NONE:
				break;
			default:
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
					"mso_prov_bb_config_object from_cache: error "
					"accessing data dictionary cache.");
				pin_set_err(ebufp, PIN_ERRLOC_CM,
						PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_FLD_CACHE_NAME, 0, 0);
				goto cleanup;
		}
	}
	PIN_ERR_LOG_MSG(3, "Fields to compare");
	if(prov_tag)
		PIN_ERR_LOG_MSG(3, prov_tag);
	if (plan_name)
		PIN_ERR_LOG_MSG(3, plan_name);
	while ( (tmp_flistp = PIN_FLIST_ELEM_GET_NEXT (cache_flistp,
		PIN_FLD_RESULTS, &rec_id, 1,&cookie, ebufp))!= (pin_flist_t *)NULL )
	{
		if (prov_tag && strcmp((char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_PROVISIONING_TAG, 0, ebufp ),prov_tag) == 0 )
			//&& strcmp((char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_SERVICE_CODE, 0, ebufp ), plan_name) == 0)
		{
			PIN_ERR_LOG_MSG(3, "Prov tag matched.");
			tec = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_BB_TECHNOLOGY, 0, ebufp);
			sprintf(temp,"%d and %d",*tec,*tech);
			PIN_ERR_LOG_MSG(3, temp);
			if(*tech == TECH_NEW_CPS)
			{
				tech=tec;
			}
			if(*tec == *tech)
			{
				srv_typ = *(int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_ISQUOTA_FLAG, 0, ebufp);
				type = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TYPE, 0, ebufp);
				amount = (pin_decimal_t *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_INITIAL_AMOUNT, 0, ebufp);
				found = 1;
				PIN_ERR_LOG_MSG(3, "Prov tag and Tech matched 2.");
				break;
			}
		}

	}
	if(found) {
		*plan_typ = *type;
		*initial_amount = pbo_decimal_copy(amount, ebufp);
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
			"mso_prov_bb_config_object from_cache: error "
			"Matching provisioning entry not found.");
		pin_set_err(ebufp, PIN_ERRLOC_CM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, MSO_FLD_BB_TECHNOLOGY, 0, 0);			
	}
	PIN_ERR_LOG_MSG(3, "Returning from fm_quota_check.");
cleanup:
	if(PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_quota_check: Error", ebufp);
		if(cache_flistp != NULL )PIN_FLIST_DESTROY_EX(&cache_flistp, NULL);
		return 3;
	}
		if(cache_flistp != NULL) PIN_FLIST_DESTROY_EX(&cache_flistp, NULL);
	return srv_typ;
}


int static
fm_quota_check_old(
	pcm_context_t	*ctxp,
	poid_t		*prod_pdp,
	int		*error_codep,
	pin_errbuf_t	*ebufp)
{
	pin_decimal_t			*prod_priority = pbo_decimal_from_str("999.0",ebufp);
	pin_decimal_t			*divident = pbo_decimal_from_str("500.0",ebufp);
	pin_decimal_t			*priority_check = pbo_decimal_from_str("80.0",ebufp);
	pin_decimal_t			*priority_calc = NULL;
	pin_decimal_t			*priority_rem = NULL;
	
	pin_flist_t			*quota_s_flistp = NULL;
	pin_flist_t			*quota_so_flistp = NULL;
	pin_flist_t			*arg_flistp = NULL;
	pin_flist_t			*quota_res_flistp = NULL;
	
	int64               database = 0;
	int32              	s_flags = 0;
	poid_t				*pdp = NULL;
	poid_t				*s_pdp = NULL;
	int				    prod_status = 1;
	int32              	quota_flag = 0;
	
	/*Search flist to get the priority which will determine whether the
	  customer is a quota customer or not     
	  	0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
		0 PIN_FLD_FLAGS           INT [0] 768
		0 PIN_FLD_TEMPLATE        STR [0] " select X from /product F1 = V1 "
		0 PIN_FLD_RESULTS       ARRAY [0]     NULL array ptr
		0 PIN_FLD_ARGS          ARRAY [1] allocated 20, used 1
		1     PIN_FLD_ACCOUNT_OBJ    POID [0] 0.0.0.1 /product 2317687 0
	*/
	
	quota_s_flistp = PIN_FLIST_CREATE(ebufp);

	/***********************************************************
	 * assume db matches userid found in pin.conf
	 ***********************************************************/
	pdp = PCM_GET_USERID(ctxp);
	database = PIN_POID_GET_DB(pdp);

	s_pdp = PIN_POID_CREATE(database, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(quota_s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_FLD_SET(quota_s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
	PIN_FLIST_FLD_SET(quota_s_flistp, PIN_FLD_TEMPLATE, 
	"select X from /product where F1 = V1 ", ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(quota_s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, (void *)prod_pdp, ebufp);
	PIN_FLIST_ELEM_SET( quota_s_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Get Product priority input flist", quota_s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, quota_s_flistp, &quota_so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Get Product priority output flist", quota_so_flistp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_INTERNAL_ERROR;
             PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "error getting product priority", ebufp);
             goto cleanup;
	}
     
	quota_res_flistp = PIN_FLIST_ELEM_GET(quota_so_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	if(quota_res_flistp){
		prod_priority = PIN_FLIST_FLD_GET(quota_res_flistp, PIN_FLD_PRIORITY, 0,ebufp);
		
		//priority_rem = prod_priority%500;
		priority_calc = pbo_decimal_multiply(pbo_decimal_round(pbo_decimal_divide(prod_priority,divident,ebufp),0,ROUND_DOWN,ebufp),divident,ebufp);
		priority_rem = pbo_decimal_subtract(prod_priority,priority_calc,ebufp);
		
		if(pbo_decimal_compare(priority_rem, priority_check,ebufp) <= 0 )
		{
			quota_flag = 1;
		}
	}
cleanup: 
	PIN_FLIST_DESTROY_EX(&quota_s_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&quota_so_flistp, NULL);
	return quota_flag;
}

/*******************************************************************
 * fm_mso_create_bal_qps()
 *
 * returns CREATEBALANCE ARRAY.
  *******************************************************************/
static void
fm_mso_create_bal_qps(
	pcm_context_t		*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp,
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*plan_iflistp = NULL;
	pin_flist_t			*createbalflistp = NULL;
	pin_flist_t			*plan_oflistp = NULL;
	int32              	quota_flag = 0;
	poid_t				*ac_pdp = NULL;
	poid_t				*plan_pdp = NULL;
	char				*node_name = NULL;
	char				*ne_add = NULL;
	char				*city = NULL;
	int			    	plan_typ = 0;
	char				*prov_tag = NULL;
	char				*plan_name = NULL;
	int				    *tech = 0;
	int				    srv_typ = 0;
	pin_decimal_t		*initial_amount = NULL;

	FILE				*filep = NULL;	

	filep = fopen("/tmp/stackt","a+");

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_create_bal_qps input flist", *i_flistpp);
	
	prov_tag = (char *)PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_PROVISIONING_TAG, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_PROV_TAG_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_create_bal_qps: Technology Missing",ebufp);
		goto cleanup;
	}

	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_TECH_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_create_bal_qps: Technology Missing",ebufp);
		goto cleanup;
	}

	plan_pdp = (poid_t *)PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_PLAN_OBJ, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Plan Name not found",ebufp);
		*error_codep = MSO_BB_PLAN_OBJ_MISSING; 
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_NAME, 0, 0);
		goto cleanup;
	}
	plan_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(plan_iflistp, PIN_FLD_POID, plan_pdp, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read input flist", plan_iflistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, plan_iflistp, &plan_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read output flist", plan_oflistp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_create_bal_qps: Error", ebufp);
		goto cleanup;
	}
	plan_name = (char *)PIN_FLIST_FLD_GET(plan_oflistp, PIN_FLD_NAME, 0, ebufp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Plan Name not found",ebufp);
		*error_codep = MSO_BB_PLAN_OBJ_MISSING;
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_NAME, 0, 0);
		goto cleanup;
	}

	/******Add the Task Array for QPS*******************/
	task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, task_id, ebufp);
	task_id = task_id + 1;
	node_name = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NE_ELEMENT_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_create_bal_qps: Network Element Missing",ebufp);
		goto cleanup;
	}
	city = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_CITY_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_create_bal_qps: Network Element Missing",ebufp);
		goto cleanup;
	}
	//Call the function to get the NE Address
	fm_mso_get_network_node(ctxp, node_name, city, &ne_add, 1, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_create_bal_qps: Not able to get Network Element ID",ebufp);
		goto cleanup;
	}
	if(ne_add){
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, (void *)ne_add, ebufp);
	}
	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "CREATE_BALANCE_QPS", ebufp);
	/******Add the Subscriber Array*******************/
	
	ac_pdp = (poid_t *)PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_POID, 0, ebufp);

	quota_flag = fm_quota_check(ctxp, plan_name, prov_tag, tech, &plan_typ, &initial_amount, error_codep, ebufp);
	createbalflistp = PIN_FLIST_ELEM_ADD(task_flistp, 
		MSO_FLD_CREATEBALANCE, 1, ebufp);
	if(quota_flag == 1 )
	{
		//createbalflistp = PIN_FLIST_CREATE(ebufp);
		//Call the function to populate the create balance array
		fm_mso_add_create_bal(ctxp,i_flistpp,ac_pdp, &createbalflistp, &plan_typ, &initial_amount, srv_typ, error_codep, ebufp); 
		PIN_FLIST_ELEM_COPY(createbalflistp, MSO_FLD_CREATEBALANCE,1,
			 task_flistp, MSO_FLD_CREATEBALANCE,1, ebufp);
	}
	
                        
cleanup:
	if(plan_oflistp != NULL )PIN_FLIST_DESTROY_EX(&plan_oflistp, NULL);
	if(plan_iflistp != NULL )PIN_FLIST_DESTROY_EX(&plan_iflistp, NULL);
	if(createbalflistp != NULL )PIN_FLIST_DESTROY_EX(&createbalflistp, NULL);
//	if(!pbo_decimal_is_null(initial_amount, ebufp))
//		pbo_decimal_destroy(&initial_amount);
	
	return;
}


static void
fm_mso_change_username_qps(
	pcm_context_t		*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp,
	int				    *error_codep,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*deviceflistp = NULL;
	pin_flist_t			*temp_flistp = NULL;
	pin_flist_t			*new_flistp = NULL;
	poid_t				*serv_poid = NULL;
	char				*old_name = NULL;
	char				*new_name = NULL;
	char				*node_name = NULL;
	char				*ne_add = NULL;
	char				*city = NULL;
	char				*device_tech_type = NULL;
	char				param[50] = {'\0'};
	int				    *act = 0;
	pin_cookie_t		cookie = NULL;
	int32				rec_id =0;
	int32               elem_id = 101;
	pin_cookie_t        qcookie = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_change_username_qps input flist", *i_flistpp);
	
	/******Add the Task Array for QPS*******************/
	//act = (int *)PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_FLAGS, 0, ebufp);
	task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, task_id, ebufp);
	task_id = task_id + 1;
	serv_poid = (poid_t *)PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_POID, 0, ebufp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	node_name = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NE_ELEMENT_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_change_username_qps: Network Element Missing",ebufp);
		goto cleanup;
	}
	city = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_CITY_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_change_username_qps: Network Element Missing",ebufp);
		goto cleanup;
	}
	//Call the function to get the NE Address
	fm_mso_get_network_node(ctxp, node_name, city, &ne_add, 1, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_change_username_qps: Not able to get Network Element ID",ebufp);
		goto cleanup;
	}
	if(ne_add){
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, (void *)ne_add, ebufp);
	}
	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "CHANGE_CREDENTIAL_QPS", ebufp);
	/******Add the Subscriber Array*******************/
	old_name = (char *)PIN_FLIST_FLD_TAKE(*i_flistpp, PIN_FLD_LOGIN, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_OLD_LOGIN_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_change_username_qps: Error",ebufp);
		goto cleanup;
	}
	act = (int *)PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_FLAGS, 0, ebufp);
	if(*act == DOC_BB_MODEM_CHANGE)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Modem Change");
		strcpy(param,"/device/modem");
		/* get the modem device if any */
		fm_mso_search_devices(ctxp, param, serv_poid, &deviceflistp, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_change_username_qps: Error",ebufp);
			goto cleanup;
		}
		cookie = NULL;
		rec_id =0;
		while ( (temp_flistp = PIN_FLIST_ELEM_GET_NEXT (deviceflistp,
			       PIN_FLD_RESULTS, &rec_id, 1,&cookie, ebufp))
							  != (pin_flist_t *)NULL ) 
		{
			new_name = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_DEVICE_ID, 0,ebufp);
			device_tech_type = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_DEVICE_TYPE, 0,ebufp);
			//dev_type = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_DEVICE_TYPE, 0,ebufp);
			//ip_count = 0;
			//strcpy(device_typ, "/device/modem");
			/**** Pavan Bellala 29-07-2015 ***********************************
			 Defect 1136 : Validate device string pattern before provisioning
			*****************************************************************/
			if(new_name && strcmp(new_name,"")!=0)
			{
				//Added the below code for JIO-CPS Integration-ISP Enhancement
				//Skip MAC validation for ONU (GPON) modem for JIO and Hathway Networks and validate the device_id pattern for other device types.
 				if( ( device_tech_type && 
					  !( strcmp(device_tech_type, HW_GPON_MODEM) == 0 || 
					     strcmp(device_tech_type, HW_ONU_GPON_MODEM) == 0 ||
					     strcmp(device_tech_type, JIO_GPON_MODEM) == 0 ) )
					&& fm_mso_integ_match_patterns("^([0-9a-fA-F]{2})(([/\\s:-][0-9a-fA-F]{2}){5})$", new_name) <=0 )
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR , "Error:Invalid MAC id");
					pin_set_err(ebufp, PIN_ERRLOC_FM,
								PIN_ERRCLASS_SYSTEM_DETERMINATE,PIN_ERR_BAD_VALUE, PIN_FLD_DEVICE_ID, 0, 0);
					*error_codep = MSO_BB_INVALID_MAC;
					goto cleanup;
				}
			}
		}
	}
	else if(*act == DOC_BB_IP_CHANGE || *act == DOC_BB_CMTS_CHANGE_T || *act == DOC_BB_CMTS_CHANGE_NT)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"IP or CMTS change");
		fm_mso_get_ip(ctxp, &new_name, *i_flistpp, so_flistp, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_change_username_qps: Error getting name",ebufp);
			goto cleanup;
		}
	}
	/*else if (*act == GPON_MAC_CHANGE)
	{
		while ( (temp_flistp = PIN_FLIST_ELEM_GET_NEXT (*i_flistpp,
			       PIN_FLD_ALIAS_LIST, &elem_id, 1,&qcookie, ebufp))
							  != NULL ) {
			new_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, elem_id, 1, ebufp);
			if( new_flistp)
				new_name = (char *)PIN_FLIST_FLD_GET(new_flistp, PIN_FLD_NAME, 1, ebufp);
		}
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			*error_codep = MSO_BB_MAC_ADD_ERROR;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_change_username_qps: Error",ebufp);
			goto cleanup;
		}
	}*/
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"No (Modem , IP or CMTS) change");
		new_name = (char *)PIN_FLIST_FLD_TAKE(so_flistp, PIN_FLD_LOGIN, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			*error_codep = MSO_BB_LOGIN_MISSING;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_change_username_qps: Error",ebufp);
			goto cleanup;
		}
	}
	if(new_name && strcmp(new_name, old_name)!= 0){
		PIN_FLIST_FLD_PUT(task_flistp, MSO_FLD_OLD_NETWORKID, old_name, ebufp);
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORKID, new_name, ebufp);
	}
	else{
		*error_codep = MSO_BB_SAME_LOGIN_ERROR;
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_LOGIN, 0, 0);
		goto cleanup;
	}
cleanup:
	return;
}

static void
fm_mso_change_username_nsure(
	pcm_context_t		*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp,
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*alias_list = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*ip_flistp = NULL;
	pin_flist_t			*temp_flistp = NULL;
	pin_flist_t			*deviceflistp = NULL;
	poid_t				*serv_poid = NULL;
	int				    flag = 0;
	char				*ip_addr = NULL;
	char				*old_ip = NULL;
	char				*node_name = NULL;
	char				*ne_add = NULL;
	char				param[20] = {'\0'};
	char				*ip_add_list = NULL;
	char				*mso_ip_add = NULL;
	char				*ip_add = NULL;
	char				*city = NULL;
	pin_cookie_t		cookie1 = NULL;
	int32				rec_id1 = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	//alias_list = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
	//ip_addr = (char *)PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_LOGIN, 0, ebufp);
	old_ip = (char *)PIN_FLIST_FLD_TAKE(*i_flistpp, PIN_FLD_LOGIN, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_change_username_nsure: Error", ebufp);
		return;
	}
	serv_poid = (poid_t *)PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_POID, 0, ebufp);
	strcpy(param,"/device/ip/static");
	/* get the modem device if any */
	fm_mso_search_devices(ctxp,param,serv_poid,&deviceflistp, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_change_username_nsure: Error", ebufp);
		goto cleanup;
	}
	flag = 0;
	while((temp_flistp = PIN_FLIST_ELEM_GET_NEXT(deviceflistp, 
		PIN_FLD_RESULTS, &rec_id1, 1 , &cookie1, ebufp)) != (pin_flist_t *)NULL)
	{
		ip_add = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_DEVICE_ID, 0,ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			*error_codep = MSO_BB_INTERNAL_ERROR;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_change_username_nsure: Error", ebufp);
			goto cleanup;
		}
		//Pawan:17-03-2015 Added below block and commented next block
		if(flag == 1)
		{ 
			ip_add_list = (char *)realloc(ip_add_list,(strlen(ip_add_list)+strlen(ip_add))+2);
			strcat(ip_add_list, ",");
			strcat(ip_add_list, ip_add);
		}
		else{
			ip_add_list = (char*)malloc(strlen(ip_add)+1);
			strcpy(ip_add_list, ip_add);
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ip_add_list: ");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, ip_add_list);
		flag = 1;
			
		//ip_add_list = (char *)pin_malloc(1);
		//strcpy(ip_add_list, '\0');
		//cookie1 = NULL;
		//rec_id1 = 0;
		/*
		ip_flistp = PIN_FLIST_ELEM_GET (temp_flistp, MSO_FLD_IP_DATA, 0, 1,ebufp);
		if(ip_flistp)
		{
			mso_ip_add = PIN_FLIST_FLD_GET(ip_flistp, MSO_FLD_IP_ADDRS, 0,ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_add_list: ");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, mso_ip_add);
			if(flag == 1)
			{ 
				ip_add_list = (char *)realloc(ip_add_list,(strlen(ip_add_list)+strlen(mso_ip_add))+2);
				strcat(ip_add_list, ",");
				strcat(ip_add_list, mso_ip_add);
			}
			else{
				ip_add_list = (char*)malloc(strlen(mso_ip_add)+1);
				strcpy(ip_add_list, mso_ip_add);
			}
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ip_add_list: ");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, ip_add_list);
			flag = 1;
			//ip_pool = PIN_FLIST_FLD_GET(ip_flistp, MSO_FLD_IP_POOL_NAME, 0,ebufp);
			//ip_mask = PIN_FLIST_FLD_GET(ip_flistp, MSO_FLD_IP_MASK_VALUE, 0,ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp)) {
				*error_codep = MSO_BB_INTERNAL_ERROR;
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_add_user_attribute: Error", ebufp);
				goto cleanup;
			}

		}
		*/
	}
	
//		if(ip_add_list)
//		{
//			PIN_FLIST_FLD_PUT(user_flistp, MSO_FLD_USER_IP, ip_add_list, ebufp);
//			PIN_FLIST_FLD_SET(user_flistp, MSO_FLD_IP_POOL_NAME, ip_pool, ebufp);
//			acc_id = PIN_POID_GET_ID(ac_pdp);
//			sprintf(acc_id_str,"%d",acc_id);
//			
//			//PIN_FLIST_FLD_SET(user_flistp, MSO_FLD_EXTERNAL_REF_NO, acc_id_str, ebufp);
//			PIN_FLIST_FLD_SET(user_flistp, MSO_FLD_SUBNET_MASK, ip_mask, ebufp);
//			if (PIN_ERRBUF_IS_ERR(ebufp)) {
//				*error_codep = MSO_BB_INTERNAL_ERROR;
//				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_add_user_attribute: Error", ebufp);
//				goto cleanup;
//			}
//		}
	if(ip_add_list){
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "T1");
		//PIN_FLIST_FLD_SET(detail_flistp, PIN_FLD_LOGIN, (void *)ip_add_list, ebufp);
	}
	else{
		*error_codep = MSO_BB_IP_ADD_ERROR;
		pin_set_err(ebufp, PIN_ERRLOC_CM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_FLD_DEVICE_ID, 0, 0);
	}
	if(strcmp(ip_add_list, old_ip)!= 0)
	{
		/******Add the Task Array for QPS*******************/
		task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, task_id, ebufp);
		task_id = task_id + 1;
		tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
		node_name = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			*error_codep = MSO_BB_NE_ELEMENT_MISSING;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_change_username_nsure: Network Element Missing",ebufp);
			goto cleanup;
		}
		//Call the function to get the NE Address
		//fm_mso_get_network_node(ctxp, node_name, &ne_add, 2, error_codep, ebufp);
		city = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			*error_codep = MSO_BB_CITY_MISSING;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_change_username_nsure: Network Element Missing",ebufp);
			goto cleanup;
		}
		//Call the function to get the NE Address
		fm_mso_get_network_node(ctxp, node_name, city, &ne_add, 2, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_change_username_nsure: Not able to get Network Element ID",ebufp);
			goto cleanup;
		}
		if(ne_add){
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, (void *)ne_add, ebufp);
		}
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "MODIFY_USERNAME_NSURE", ebufp);
		PIN_FLIST_FLD_PUT(task_flistp, MSO_FLD_OLD_USER_NAME, (void *)old_ip, ebufp);
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NEW_USER_NAME, (void *)ip_add_list, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			*error_codep = MSO_BB_INTERNAL_ERROR;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_change_username_nsure: Error", ebufp);
			return;
		}
	}
	else
	{
		*error_codep = MSO_BB_SAME_IP_ERROR;
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_LOGIN, 0, 0);
		goto cleanup;
	}
cleanup:
	return;
}

static void
fm_mso_change_password(
	pcm_context_t		*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp,
	int				    *error_codep,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*bb_info = NULL;
	char				*old_pass = NULL;
	char				*new_pass = NULL;
	char				*node_name = NULL;
	char				*ne_add = NULL;
	char				*login = NULL;
	int				    *isTAL = 0;
	//pin_flist_t		*bb_info = NULL;
	pin_flist_t			*deviceflistp = NULL;
	pin_flist_t			*temp_flistp = NULL;
	pin_flist_t			*ip_flistp = NULL;
	poid_t				*serv_poid = NULL;
	char				*ip_add = NULL;
	char				param[20] = {'\0'};
	int				    flag = 0;
	int32				rec_id1 = 0;
	pin_cookie_t		cookie1 = NULL;
	char				*mso_ip_add = NULL;
	char				*ip_add_list = NULL;
	char				*user_id = NULL;
	char				*city = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	bb_info = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	new_pass = PIN_FLIST_FLD_GET(bb_info, MSO_FLD_PASSWORD, 0, ebufp);
	old_pass = PIN_FLIST_FLD_TAKE(*i_flistpp, MSO_FLD_PASSWORD, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_OLDPASS_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_change_password: Error", ebufp);
		return;
	}
	if(strcmp(new_pass, old_pass)!= 0)
	{
		task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, task_id, ebufp);
		task_id = task_id + 1;
		node_name = (char *)PIN_FLIST_FLD_GET(bb_info, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			*error_codep = MSO_BB_NE_ELEMENT_MISSING;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_change_password: Network Element Missing",ebufp);
			goto cleanup;
		}
		//Call the function to get the NE Address
		//fm_mso_get_network_node(ctxp, node_name, &ne_add, 2, error_codep, ebufp);
		city = (char *)PIN_FLIST_FLD_GET(bb_info, PIN_FLD_CITY, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			*error_codep = MSO_BB_CITY_MISSING;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_change_password: Network Element Missing",ebufp);
			goto cleanup;
		}
		//Call the function to get the NE Address
		fm_mso_get_network_node(ctxp, node_name, city, &ne_add, 1, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_change_password: Not able to get Network Element ID",ebufp);
			goto cleanup;
		}
		if(ne_add){
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, (void *)ne_add, ebufp);
		}
		
		serv_poid = PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_POID, 0, ebufp);
		user_id = (char *)PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_LOGIN, 0, ebufp);
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORKID, (void *)user_id, ebufp); 
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			*error_codep = MSO_BB_INTERNAL_ERROR;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_change_password: Error", ebufp);
			return;
		}
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "CHANGE_PASSWORD_QPS", ebufp);
		PIN_FLIST_FLD_PUT(task_flistp, MSO_FLD_OLD_PASSWORD, (void *)old_pass, ebufp);
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NEW_PASSWORD, (void *)new_pass, ebufp);
		
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			*error_codep = MSO_BB_INTERNAL_ERROR;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_change_password: Error", ebufp);
			return;
		}
	}
	else
	{
		*error_codep = MSO_BB_SAME_PASS_ERROR;
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_LOGIN, 0, 0);
		goto cleanup;
	}
cleanup:
	return;
}

static void
fm_mso_delete_subscriber(
	pcm_context_t		*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp,
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*bb_info = NULL;
	pin_flist_t			*deviceflistp = NULL;
	pin_flist_t			*temp_flistp = NULL;
	pin_flist_t			*ip_flistp = NULL;
	char				*node_name = NULL;
	char				*ne_add = NULL;
	char				*login = NULL;
	char				param[20] = {'\0'};
	char				*ip_add = NULL;
	char				*mso_ip_add = NULL;
	char				*ip_add_list = NULL;
	poid_t				*serv_poid = NULL;
	int				    *isTAL = 0;
	int				    flag = 0;
	pin_cookie_t		cookie1 = NULL;
	int32				rec_id1 = 0;
	char				*city = NULL;
	int32				*tech = NULL;
    char                *user_id = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_delete_subscriber input flist", *i_flistpp);
	bb_info = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	isTAL = PIN_FLIST_FLD_GET(bb_info, MSO_FLD_ISTAL_FLAG, 0, ebufp);
	tech = PIN_FLIST_FLD_GET(bb_info, MSO_FLD_TECHNOLOGY, 0, ebufp);
	/******Add the Task Array for QPS*******************/
	task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, task_id, ebufp);
	task_id = task_id + 1;
	// This would fail in the case of MULTIPLE IP.. Instead of sending IPs, we would send NETWORK LOGIN for FIBER
	if(*isTAL == 1 && *tech != 4 && (strcmp(other_ne_id,HW_NETWORK_STER_ID) != 0))
	{
		serv_poid = (poid_t *)PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_POID, 0, ebufp);
		strcpy(param,"/device/ip/static");
		/* get the modem device if any */
		fm_mso_search_devices(ctxp,param,serv_poid,&deviceflistp, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_delete_subscriber: Error", ebufp);
			goto cleanup;
		}
		flag = 0;
		while((temp_flistp = PIN_FLIST_ELEM_GET_NEXT(deviceflistp, 
			PIN_FLD_RESULTS, &rec_id1, 1 , &cookie1, ebufp)) != (pin_flist_t *)NULL)
		{
			ip_add = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_DEVICE_ID, 0,ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp)) {
				*error_codep = MSO_BB_INTERNAL_ERROR;
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_delete_subscriber: Error", ebufp);
				goto cleanup;
			}
			/* Pawan-17-03-2015: Added below to take IP Add from device id */
			if(flag == 1)
			{ 
				ip_add_list = (char *)realloc(ip_add_list,(strlen(ip_add_list)+strlen(ip_add))+2);
				strcat(ip_add_list, ",");
				strcat(ip_add_list, ip_add);
			}
			else{
				ip_add_list = (char*)malloc(strlen(ip_add) + 1);
				strcpy(ip_add_list, ip_add);
			}
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ip_add_list: ");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, ip_add_list);
			flag = 1;
			
			//ip_add_list = (char *)pin_malloc(1);
			//strcpy(ip_add_list, '\0');
			
			//cookie1 = NULL;
			//rec_id1 = 0;
			/*
			ip_flistp = PIN_FLIST_ELEM_GET(temp_flistp, MSO_FLD_IP_DATA, 0, 1,ebufp);
			if(ip_flistp)
			{
				mso_ip_add = PIN_FLIST_FLD_GET(ip_flistp, MSO_FLD_IP_ADDRS, 0,ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_add_list: ");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, mso_ip_add);
				if(flag == 1)
				{ 
					ip_add_list = (char *)realloc(ip_add_list,(strlen(ip_add_list)+strlen(mso_ip_add))+2);
					strcat(ip_add_list, ",");
					strcat(ip_add_list, mso_ip_add);
				}
				else{
					ip_add_list = (char*)malloc(strlen(mso_ip_add) + 1);
					strcpy(ip_add_list, mso_ip_add);
				}
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ip_add_list: ");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, ip_add_list);
				flag = 1;
				//ip_pool = PIN_FLIST_FLD_GET(ip_flistp, MSO_FLD_IP_POOL_NAME, 0,ebufp);
				//ip_mask = PIN_FLIST_FLD_GET(ip_flistp, MSO_FLD_IP_MASK_VALUE, 0,ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp)) {
					*error_codep = MSO_BB_INTERNAL_ERROR;
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_add_user_attribute: Error", ebufp);
					goto cleanup;
				}

			}
			*/
		}
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORKID, ip_add_list, ebufp);
	}
	else{
		// Pawan: Commented below and added next line to take the login from input flist
		//login = PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_LOGIN, 0, ebufp);
		//PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORKID, login, ebufp);
		PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_LOGIN, task_flistp, MSO_FLD_NETWORKID, ebufp);
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_LOGIN_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_delete_subscriber: Network Element Missing",ebufp);
		goto cleanup;
	}
	//tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	//node_name = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NE_ELEMENT_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_delete_subscriber: Network Element Missing",ebufp);
		goto cleanup;
	}
	//Call the function to get the NE Address
	//fm_mso_get_network_node(ctxp, node_name, &ne_add, 1, error_codep, ebufp);
	//city = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_CITY_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_delete_subscriber: Network Element Missing",ebufp);
		goto cleanup;
	}
	//Call the function to get the NE Address
	/*fm_mso_get_network_node(ctxp, node_name, city, &ne_add, 1, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_delete_subscriber: Not able to get Network Element ID",ebufp);
		goto cleanup;
	}*/
	if (other_ne_id) {
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, (void *)other_ne_id, ebufp);
	}
	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "DELETE_SUBSCRIBER_QPS", ebufp);
	/******Add the Subscriber Array*******************/
	
	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_HARD_DELETE, "true", ebufp);
	
        
	cleanup:
	return;
}

static void
fm_mso_delete_ethernet_subs(
	pcm_context_t		*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp,
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*task_flistp = NULL;
	//pin_flist_t		  *tmp_flistp = NULL;
	char				*node_name = NULL;
	char				*ne_add = NULL;
	char				*login = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*bb_info = NULL;
	pin_flist_t			*deviceflistp = NULL;
	pin_flist_t			*temp_flistp = NULL;
	pin_flist_t			*ip_flistp;
	//char				*node_name = NULL;
	//char				*ne_add = NULL;
	//char				*login = NULL;
	char				param[20] = {'\0'};
	char				*ip_add = NULL;
	char				*mso_ip_add = NULL;
	char				*ip_add_list = NULL;
	char				*city = NULL;
	poid_t				*serv_poid = NULL;
	int				    *isTAL = 0;
	int				    flag = 0;
	pin_cookie_t		cookie1 = NULL;
	int32				rec_id1 = 0;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	//login = (char *)PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_LOGIN, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{	
		*error_codep = MSO_BB_LOGIN_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_delete_ethernet_subs: Error", ebufp);
		return;
	}
	task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, task_id, ebufp);
	task_id = task_id + 1;
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	node_name = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NE_ELEMENT_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_delete_ethernet_subs: Network Element Missing",ebufp);
		goto cleanup;
	}
	//Call the function to get the NE Address
	//fm_mso_get_network_node(ctxp, node_name, &ne_add, 2, error_codep, ebufp);
	city = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_CITY_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_delete_ethernet_subs: Network Element Missing",ebufp);
		goto cleanup;
	}
	//Call the function to get the NE Address
	fm_mso_get_network_node(ctxp, node_name, city, &ne_add, 2, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_delete_ethernet_subs: Not able to get Network Element ID",ebufp);
		goto cleanup;
	}
	if(ne_add){
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, (void *)ne_add, ebufp);
	}
	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "DELETE_SUBSCRIBER_NSURE", ebufp);
	bb_info = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	isTAL = PIN_FLIST_FLD_GET(bb_info, MSO_FLD_ISTAL_FLAG, 0, ebufp);
	/******Add the Task Array for QPS*******************/
	//task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, task_id, ebufp);
	//task_id = task_id + 1;
	if(*isTAL == 1)
	{
		serv_poid = (poid_t *)PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_POID, 0, ebufp);
		strcpy(param,"/device/ip/static");
		/* get the modem device if any */
		fm_mso_search_devices(ctxp,param,serv_poid,&deviceflistp, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_delete_ethernet_subs: Error", ebufp);
			goto cleanup;
		}
		flag = 0;
		cookie1 = NULL;
		rec_id1 = 0;
		while((temp_flistp = PIN_FLIST_ELEM_GET_NEXT(deviceflistp, 
			PIN_FLD_RESULTS, &rec_id1, 1 ,&cookie1, ebufp)) != (pin_flist_t *)NULL)
		{
			ip_add = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_DEVICE_ID, 0,ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp)) {
				*error_codep = MSO_BB_INTERNAL_ERROR;
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_delete_ethernet_subs: Error", ebufp);
				goto cleanup;
			}
			//Pawan:17-03-2015: Added below block and commented next block
			if(flag == 1)
			{ 
				ip_add_list = (char *)realloc(ip_add_list,(strlen(ip_add_list)+strlen(ip_add))+2);
				strcat(ip_add_list, ",");
				strcat(ip_add_list, ip_add);
			}
			else{
				ip_add_list = (char*)malloc(strlen(ip_add)+ 1);
				strcpy(ip_add_list, ip_add);
			}
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ip_add_list: ");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, ip_add_list);
			flag = 1;
			/*
			//ip_add_list = (char *)pin_malloc(1);
			//strcpy(ip_add_list, '\0');
			ip_flistp = PIN_FLIST_ELEM_GET (temp_flistp, MSO_FLD_IP_DATA, 0, 1, ebufp);
			if(ip_flistp)
			{
				mso_ip_add = PIN_FLIST_FLD_GET(ip_flistp, MSO_FLD_IP_ADDRS, 0,ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_add_list: ");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, mso_ip_add);
				if(flag == 1)
				{ 
					ip_add_list = (char *)realloc(ip_add_list,(strlen(ip_add_list)+strlen(mso_ip_add))+2);
					strcat(ip_add_list, ",");
					strcat(ip_add_list, mso_ip_add);
				}
				else{
					ip_add_list = (char*)malloc(strlen(mso_ip_add)+ 1);
					strcpy(ip_add_list, mso_ip_add);
				}
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ip_add_list: ");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, ip_add_list);
				flag = 1;
				//ip_pool = PIN_FLIST_FLD_GET(ip_flistp, MSO_FLD_IP_POOL_NAME, 0,ebufp);
				//ip_mask = PIN_FLIST_FLD_GET(ip_flistp, MSO_FLD_IP_MASK_VALUE, 0,ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp)) {
					*error_codep = MSO_BB_INTERNAL_ERROR;
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_add_user_attribute: Error", ebufp);
					goto cleanup;
				}

			}*/
		}
		PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_LOGIN, ip_add_list, ebufp);
	}
	else{
		login = (char *)PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_LOGIN, 0, ebufp);
		PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_LOGIN, (void *)login, ebufp);
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_delete_ethernet_subs: Error", ebufp);
		return;
	}
cleanup:
	return;
}

static void
fm_mso_get_ethernet_details(
	pcm_context_t		*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp, 
	char				*task,
	int				    *error_codep,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	char				*node_name = NULL;
	char				*ne_add = NULL;
	char				*login = NULL;
	char				*city = NULL;
	poid_t				*serv_poid = NULL;
	int32				*isTAL = NULL;
	
	//login = (char *)PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_LOGIN, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_LOGIN_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_ethernet_details: Error", ebufp);
		return;
	}
	task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, task_id, ebufp);
	task_id = task_id + 1;
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	node_name = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NE_ELEMENT_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_get_ethernet_details: Network Element Missing",ebufp);
		goto cleanup;
	}
	//Call the function to get the NE Address
	//fm_mso_get_network_node(ctxp, node_name, &ne_add, 2, error_codep, ebufp);
	city = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_CITY_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_get_ethernet_details: Network Element Missing",ebufp);
		goto cleanup;
	}
	//Call the function to get the NE Address
	fm_mso_get_network_node(ctxp, node_name, city, &ne_add, 2, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_get_ethernet_details: Not able to get Network Element ID",ebufp);
		goto cleanup;
	}
	if(ne_add){
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, (void *)ne_add, ebufp);
	}
	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, (void *)task, ebufp);
	serv_poid = PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_POID, 0, ebufp);
	isTAL = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_ISTAL_FLAG, 0, ebufp);
	if(*isTAL == 1)
	{
		fm_mso_get_login(ctxp, *i_flistpp, &login, so_flistp, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_ethernet_details: Error", ebufp);
			return;
		}
		PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_LOGIN, (void *)login, ebufp);
	}
	else{
		login = (char *)PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_LOGIN, 0, ebufp);
		PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_LOGIN, (void *)login, ebufp); 
	}
	//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_LOGIN, (void *)login, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_ethernet_details: Error", ebufp);
		return;
	}
cleanup:
	return;
}

static void
fm_mso_tal_release_ip(
	pcm_context_t		*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp,
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	char				*node_name = NULL;
	char				*ne_add = NULL;
	char				*old_ip = NULL;
	char				*new_ip = NULL;
	char				*login = NULL;
	char				*city = NULL;
	poid_t				*serv_poid = NULL;
	int32				*isTAL = NULL;

	old_ip = (char *)PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_LOGIN, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_OLD_LOGIN_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_tal_release_ip: Error", ebufp);
		return;
	}
	//new_ip = (char *)PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_LOGIN, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_LOGIN_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_tal_release_ip: Error", ebufp);
		return;
	}
	task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, task_id, ebufp);
	task_id = task_id + 1;
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	node_name = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NE_ELEMENT_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_tal_release_ip: Network Element Missing",ebufp);
		goto cleanup;
	}
	//Call the function to get the NE Address
	//fm_mso_get_network_node(ctxp, node_name, &ne_add, 2, error_codep, ebufp);
	city = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_CITY_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_tal_release_ip: Network Element Missing",ebufp);
		goto cleanup;
	}
	//Call the function to get the NE Address
	fm_mso_get_network_node(ctxp, node_name, city, &ne_add, 2, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_tal_release_ip: Not able to get Network Element ID",ebufp);
		goto cleanup;
	}
	if(ne_add){
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, (void *)ne_add, ebufp);
	}
	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "MODIFY_USERNAME_NSURE", ebufp);
	serv_poid = PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_POID, 0, ebufp);
	//fm_mso_get_login(ctxp, &new_ip, so_flistp, error_codep, ebufp);
	new_ip = PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_LOGIN, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_tal_release_ip: Error", ebufp);
		return;
	}
	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NEW_USER_NAME, (void *)new_ip, ebufp);
	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_OLD_USER_NAME, (void *)old_ip, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_tal_release_ip: Error", ebufp);
		return;
	}
	login = PIN_FLIST_FLD_TAKE(*i_flistpp, PIN_FLD_LOGIN, 1, ebufp);
	if(login)
		pin_free(login);
cleanup:
	return;
}

static void
fm_mso_delete_records(
	pcm_context_t		*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp, 
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*keys_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	char				*node_name = NULL;
	char				*ne_add = NULL;
	char				*id = NULL;
	char				*city = NULL;
	pin_flist_t         *cablemodemflistp = NULL;
    char                *mac_add = NULL;
    int                 *action_flag = NULL;	
	poid_t				*service_obj = NULL;
	pin_flist_t			*deviceflistp = NULL;
	pin_cookie_t		sb_cookie = NULL;
	int				    sb_elem_id = 0;
	char                param[50];	
	char                *login = NULL;	
	char                *device_tech_type = NULL;	

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_delete_records input flist", *i_flistpp);
	
	/******Add the Task Array for QPS*******************/
	task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, task_id, ebufp);
	task_id = task_id + 1;
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	node_name = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
	action_flag = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_FLAGS, 0, ebufp);
	login = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_LOGIN, 1, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NE_ELEMENT_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_delete_records: Network Element Missing",ebufp);
		goto cleanup;
	}
	//Call the function to get the NE Address
	//fm_mso_get_network_node(ctxp, node_name, &ne_add, 1, error_codep, ebufp);
	city = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_CITY_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_delete_records: Network Element Missing",ebufp);
		goto cleanup;
	}
	//Call the function to get the NE Address
	fm_mso_get_network_node(ctxp, node_name, city, &ne_add, 2, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_delete_records: Not able to get Network Element ID",ebufp);
		goto cleanup;
	}
	service_obj = PIN_FLIST_FLD_GET(*i_flistpp,PIN_FLD_SERVICE_OBJ,0,ebufp);
	/***added condition to pass the termibate subscriber task for chnage modem for TAL customers****/
	if(action_flag && *action_flag != DOC_BB_MODEM_CHANGE)
	{
		/****checking for the modem for the service if anything available*********/
		strcpy(param,"/device/modem");
        fm_mso_search_devices(ctxp, param, service_obj, &deviceflistp, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_delete_records: Error",ebufp);
			goto cleanup;
		}
		sb_cookie = NULL;
		tmp_flistp = NULL;
		while ( (tmp_flistp = PIN_FLIST_ELEM_GET_NEXT (deviceflistp,
                       	PIN_FLD_RESULTS, &sb_elem_id, 1,&sb_cookie, ebufp))
                                                  != (pin_flist_t *)NULL )
        {
			mac_add = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_DEVICE_ID, 0,ebufp);
			device_tech_type = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_DEVICE_TYPE, 0,ebufp);
			if(mac_add && strcmp(mac_add,"")!=0)
			{
				//Added the below code for JIO-CPS Integration-ISP Enhancement
				//Skip MAC validation for ONU (GPON) modem for JIO and Hathway Networks and validate the device_id pattern for other device types.
				if((device_tech_type && 
					!(strcmp(device_tech_type, HW_GPON_MODEM) == 0 || 
					  strcmp(device_tech_type, HW_ONU_GPON_MODEM) == 0 || 
					  strcmp(device_tech_type, JIO_GPON_MODEM) == 0)) && 
					 fm_mso_integ_match_patterns("^([0-9a-fA-F]{2})(([/\\s:-][0-9a-fA-F]{2}){5})$", mac_add) <=0 )
				{                                	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR , "Error:Invalid MAC id");
					pin_set_err(ebufp, PIN_ERRLOC_FM,
										PIN_ERRCLASS_SYSTEM_DETERMINATE,PIN_ERR_BAD_VALUE, PIN_FLD_DEVICE_ID, 0, 0);
					*error_codep = MSO_BB_INVALID_MAC;
					goto cleanup;
				}
			}
		}	
	}
	if(ne_add)
	{
		PIN_FLIST_FLD_PUT(task_flistp, MSO_FLD_NETWORK_NODE, (void *)ne_add, ebufp);
	}
	//id = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_ID, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NW_ID_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_delete_records: Network ID Missing",ebufp);
		goto cleanup;
	}
	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "DELETE_SUBSCRIBER_BCC", ebufp);
	
	/******Add the Subscriber Array*******************/
	
	keys_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_KEYS, 1, ebufp);
	if(mac_add != NULL && (action_flag && *action_flag != DOC_BB_MODEM_CHANGE))
	{
	    PIN_FLIST_FLD_SET(keys_flistp, MSO_FLD_MAC_ADDRESS, mac_add, ebufp);	
	}
	else if(login != NULL && (action_flag && *action_flag == DOC_BB_MODEM_CHANGE))
	{
	    PIN_FLIST_FLD_SET(keys_flistp, MSO_FLD_MAC_ADDRESS, login, ebufp);
	}
	else
	{
	     PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR , "Error:MAC id is NULL");
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                                        PIN_ERRCLASS_SYSTEM_DETERMINATE,PIN_ERR_BAD_VALUE, PIN_FLD_DEVICE_ID, 0, 0);
                *error_codep = MSO_BB_INVALID_MAC;
                goto cleanup;
	
	}
	//PIN_FLIST_FLD_SET(keys_flistp, MSO_FLD_ID, id, ebufp);
	cleanup:
	return;
}

/*******************************************************************
 * fm_mso_search_devices()
 *
 * returns Associated devices.
  *******************************************************************/
static void
fm_mso_search_client_classes(
	pcm_context_t		*ctxp,
	char				*prov_tag,
	char				*node_name,
	char				*srv_code,
    char                *str_rolep,
	int				    *action,
	int				    *isTAL,
	int				    ip_count,
	int				    *tech,
	char				*dev_type,
	char				**client_class,
	int				    *bill_when,
	char				*city,
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*search_dev_flistp = NULL;
	pin_flist_t			*arg_flistp = NULL;
	pin_flist_t			*arg2_flistp = NULL;
	pin_flist_t			*temp_flistp = NULL;
	pin_flist_t			*class_flistp = NULL;
	pin_flist_t			*search_dev_o_flistp = NULL;
	poid_t				*pdp = NULL;
	poid_t				*s_pdp = NULL;
	//char				s_template[200] = "\0";
	int64               database = 0;
	int32               s_flags = 0;
	int				    flag = 0;
	char				*class_name = NULL;
	char				*tal_class = NULL;
	char				*package_class = NULL;
	char				*max_cpe = NULL;
	char				*hardware_class = NULL;
	//char				*class_list = NULL;
	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie2 = NULL;
	int32				rec_id = 0;
	int32				rec_id2 = 0;
	
	/*Search flist to get the device details
	     
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS           INT [0] 768
	0 PIN_FLD_TEMPLATE        STR [0] "select X from /mso_cfg_cmts_cc_mapping where F1 = V1 and F2 = V2 "
	0 PIN_FLD_RESULTS       ARRAY [0]     NULL array ptr
	0 PIN_FLD_ARGS          ARRAY [1] allocated 20, used 1
	1     MSO_FLD_CC_MAPPING    ARRAY [1] allocated 20, used 1
	2         PIN_FLD_PROVISIONING_TAG    STR [0] "prov_tag"
	0 PIN_FLD_ARGS          ARRAY [2] allocated 20, used 1
	1     MSO_FLD_CMTS_ID    STR [0] "cmts_id"
	*/
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	if(*action == DOC_BB_DEACTIVATE_PKG_EXP || *action == DOC_BB_DEACTIVATE_PQUOTA_EXP || 
		*action == DOC_BB_SUSPEND || *action == DOC_BB_HOLD_SERV)
	{
		flag = DOC_BB_SUSPEND;
		fm_mso_get_hw_class(ctxp, node_name, flag, dev_type, &hardware_class, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_search_client_classes: Error", ebufp);
			goto cleanup;
		}
		class_name = malloc(strlen(hardware_class)+1);
		strcpy(class_name, hardware_class);
	}
	else
	{
		if(*tech == 2)
		{
			if(*isTAL)
			{
				//get the upload/download speed
				fm_mso_get_speed(ctxp, prov_tag, tech, srv_code, action,&package_class, bill_when, city, error_codep, ebufp);
				//fm_mso_get_speed(ctxp, prov_tag, tech, srv_code, &package_class, error_codep, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_search_client_classes: Error", ebufp);
					goto cleanup;
				}
				max_cpe = (char *)malloc(strlen("MAX-CPEs-")+ sizeof(ip_count)+1);
				//Populate MAX CPE with IP Count
				sprintf(max_cpe, "MAX-CPEs-%d", ip_count);
				//Get the Hardware Class
				fm_mso_get_hw_class(ctxp, node_name, flag, dev_type, &hardware_class, error_codep, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_search_client_classes: Error", ebufp);
					goto cleanup;
				}
				/* Pawan:12-03-2015 Added below to add the client class for DOC3-TAL */
				//Get the DHCP class
				fm_mso_get_tal_class(ctxp, node_name, prov_tag, &tal_class, str_rolep, isTAL, tech, error_codep, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_search_client_classes: Error", ebufp);
					goto cleanup;
				}
			}
			else
			{
				//get the upload/download speed
				fm_mso_get_speed(ctxp, prov_tag, tech, srv_code, action, &package_class, bill_when, city,error_codep, ebufp);
				//fm_mso_get_speed(ctxp, prov_tag, tech, srv_code, &package_class, error_codep, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_search_client_classes: Error", ebufp);
					goto cleanup;
				}
				//Populate MAX CPE with IP Count
				max_cpe = (char *)malloc(strlen("MAX-CPEs-4")+1);
				strcpy(max_cpe, "MAX-CPEs-4");
				//Get the Hardware Class
				fm_mso_get_hw_class(ctxp, node_name, flag, dev_type, &hardware_class, error_codep, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_search_client_classes: Error", ebufp);
					goto cleanup;
				}
				//Get the DHCP class
				fm_mso_get_tal_class(ctxp, node_name, prov_tag, &tal_class, str_rolep, isTAL, tech, error_codep, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_search_client_classes: Error", ebufp);
					goto cleanup;
				}
			}
			if(max_cpe){
				//strcpy(*client_class, tal_class);
				class_name = malloc(strlen(max_cpe)+1);
				sprintf(class_name, "%s", max_cpe);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, class_name);
			}
			if(tal_class){
				class_name = (char *)realloc(class_name, strlen(class_name)+ strlen(tal_class)+ 4);
				strcat(class_name, ",");
				strcat(class_name, tal_class);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, class_name);
			}
			if(package_class){
				class_name = (char *)realloc(class_name, strlen(class_name)+ strlen(package_class)+4);
				strcat(class_name, ",");
				strcat(class_name, package_class);
			}
			if(hardware_class){
				class_name = (char *)realloc(class_name, strlen(class_name)+ strlen(hardware_class)+ 4);
				strcat(class_name, ",");
				strcat(class_name, hardware_class);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, class_name);
			}
		}
		else
		{
			
			//get the upload/download speed
			//fm_mso_get_speed(ctxp, prov_tag, tech, srv_code, &package_class, error_codep, ebufp);
			fm_mso_get_speed(ctxp, prov_tag, tech, srv_code, action, &package_class, bill_when, city,error_codep, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_search_client_classes: Error", ebufp);
				goto cleanup;
			}
			if(*isTAL){	
				//Populate MAX CPE with IP Count
				max_cpe = (char *)malloc(strlen("MAX-CPEs-")+sizeof(ip_count)+ 1);
				sprintf(max_cpe, "MAX-CPEs-%d", ip_count);
			}
			else{
				max_cpe = (char *)malloc(strlen("MAX-CPEs-4") + 1);
				strcpy(max_cpe, "MAX-CPEs-4");
			}
			//Get the Hardware Class
			fm_mso_get_hw_class(ctxp, node_name, flag, dev_type, &hardware_class, error_codep, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_search_client_classes: Error", ebufp);
				goto cleanup;
			}
			//Get the DHCP class
			fm_mso_get_tal_class(ctxp, node_name, prov_tag, &tal_class, str_rolep, isTAL, tech, error_codep, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_search_client_classes: Error", ebufp);
				goto cleanup;
			}
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Before tal classs.. ");
			if(tal_class){
				class_name = malloc(strlen(tal_class)+1);
				sprintf(class_name, "%s", tal_class);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, class_name);
			}
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Before Max CPE.. ");
			if(max_cpe){
				//Pawan:25-02-2015: Added below condition to use Malloc when NULL
				if(class_name) {
					class_name = (char *)realloc(class_name, strlen(class_name)+ strlen(max_cpe)+ 4);
					strcat(class_name, ",");
					strcat(class_name, max_cpe);
				} else {
					class_name = malloc(strlen(max_cpe)+ 1);
					sprintf(class_name, "%s", max_cpe);
				}
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, class_name);
			}
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Before Package class.. ");
			if(package_class){
				class_name = (char *)realloc(class_name, strlen(class_name)+ strlen(package_class)+4);
				strcat(class_name, ",");
				strcat(class_name, package_class);
			}
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Before HW class.. ");
			if(hardware_class){
				class_name = (char *)realloc(class_name, strlen(class_name)+ strlen(hardware_class)+ 4);
				strcat(class_name, ",");
				strcat(class_name, hardware_class);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, class_name);
			}
		}
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_client_classes: Client Class:");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, class_name);
	*client_class = strdup(class_name);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "After client class strdup");
cleanup:
	if(tal_class)
		free(tal_class);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "After free tal_class");
	/*if(package_class)
		free(package_class);*/
	if(hardware_class)
		free(hardware_class);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "After free hardware_class");
	if(max_cpe)
		free(max_cpe);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "After free max_cpe");
	if(class_name)
		free(class_name);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "After free class_name");
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_client_classes: Client Class:");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, *client_class);
	return;
}

static void
fm_mso_get_hw_class(
	pcm_context_t		*ctxp, 
	char			    *node_name, 
	int			        flag, 
	char			    *dev_type, 
	char			    **hardware_class,
	int			        *error_codep,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*cache_flistp = NULL;
	cm_cache_key_iddbstr_t		kids;
        pin_cookie_t			cookie = NULL;
        int32			    	rec_id = 0;
	int32				err = PIN_ERR_NONE;
	char				*susp = NULL;
	char				*device = NULL;
	//int				*tec = 0;
	//int				*srv_typ = (int *)3;
	time_t				last_mod_t = 0;
	time_t				now_t = 0;
	
	if(PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	/******************************************************
        * If the cache is not enabled, short circuit right away
        ******************************************************/
        if ( mso_prov_bb_cmts_ptr == (cm_cache_t *)NULL ) {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
                "no config flist for /mso_cfg_cmts_master cached ");
		*error_codep = MSO_BB_CMTS_MASTER_ERROR;
                pin_set_err(ebufp, PIN_ERRLOC_CM,
                                PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_FLD_CACHE_NAME, 0, 0);
        }
        /**************************************************
         * See if the entry is in our data dictionary cache
         * Prepare the cm_cache_key_iddbstr_t structure to search
         **************************************************/
        kids.id = 0;
        kids.db = 0;
        kids.str = "/mso_cfg_cmts_master";
        cache_flistp = cm_cache_find_entry(mso_prov_bb_cmts_ptr,
                                        &kids, &err);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "cm_cache_find_entry found flist", cache_flistp);
        switch(err) {
        case PIN_ERR_NONE:
                break;
        default:
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
                        "mso_prov_bb_cmts_object from_cache: error "
                        "accessing data dictionary cache.");
			*error_codep = MSO_BB_CMTS_MASTER_ERROR;
                pin_set_err(ebufp, PIN_ERRLOC_CM,
                                PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_FLD_CACHE_NAME, 0, 0);
                goto cleanup;
        }
	last_mod_t = *(time_t*)PIN_FLIST_FLD_GET(cache_flistp, PIN_FLD_MOD_T,0,ebufp);
	now_t = pin_virtual_time((time_t *)NULL);
	if(now_t  >= last_mod_t + refresh_pt_interval )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"calling mso_cfg_cmts_master to refresh provisioning tag config");
		fm_mso_prov_bb_config_cache_update(ctxp, ebufp);
		kids.id = 0;
		kids.db = 0;
		kids.str = "/mso_cfg_cmts_master";
		if(cache_flistp != NULL)PIN_FLIST_DESTROY_EX(&cache_flistp, NULL);
		cache_flistp = cm_cache_find_entry(mso_prov_bb_cmts_ptr,&kids, &err);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"cm_cache_find_entry found flist", cache_flistp);
		switch(err)
		{
			case PIN_ERR_NONE:
				break;
			default:
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
					"mso_cfg_cmts_master from_cache: error "
					"accessing data dictionary cache.");
				pin_set_err(ebufp, PIN_ERRLOC_CM,
						PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_FLD_CACHE_NAME, 0, 0);
				goto cleanup;
		}
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_hw_class: Network Element to compare:");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, node_name);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_hw_class: Device Type:");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, dev_type);
	while ( (tmp_flistp = PIN_FLIST_ELEM_GET_NEXT (cache_flistp,
		PIN_FLD_RESULTS, &rec_id, 1,&cookie, ebufp))!= (pin_flist_t *)NULL )
	{
		if (strcmp((char *)PIN_FLIST_FLD_GET(tmp_flistp, 
			MSO_FLD_CMTS_ID, 0, ebufp ),node_name) == 0 )
		{
			//Check the flag
			if(flag == DOC_BB_SUSPEND)
			{
				susp = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_SUSPEND_POLICY, 0, ebufp);
				//Pawan:26-02-2015: Commented below to send only suspend class  
				/*if(strcmp(dev_type, "/device/modem") == 0)
				{
					device = (char *)PIN_FLIST_FLD_GET(tmp_flistp, 
							MSO_FLD_CLASS_CM, 0, ebufp);
				}
				else if(strcmp(dev_type, "/device/router/cable") == 0)
				{
					device = (char *)PIN_FLIST_FLD_GET(tmp_flistp, 
							MSO_FLD_CLASS_CR, 0, ebufp);
				}
				*hardware_class = pin_malloc(strlen(susp)+strlen(device)+1);
				sprintf(*hardware_class, "%s,%s", susp, device);
				*/
				//Pawan:26-02-2015: Added below
				*hardware_class = pin_malloc(strlen(susp)+1);
				sprintf(*hardware_class, "%s", susp);
			}
			else
			{
				if(strcmp(dev_type, "/device/modem") == 0)
				{
					device = (char *)PIN_FLIST_FLD_GET(tmp_flistp, 
							MSO_FLD_CLASS_CM, 0, ebufp);
				}
				else if(strcmp(dev_type, "/device/router/cable") == 0)
				{
					device = (char *)PIN_FLIST_FLD_GET(tmp_flistp, 
							MSO_FLD_CLASS_CR, 0, ebufp);
				}
				if(device){
					*hardware_class = malloc(strlen(device)+1);
					sprintf(*hardware_class, "%s", device);
				}
			}
			if(PIN_ERRBUF_IS_ERR(ebufp)) {
				*error_codep = MSO_BB_CMTS_DEVICE_CLASS_ERROR;
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_hw_class: Error", ebufp);
				PIN_FLIST_DESTROY_EX(&cache_flistp, NULL);
				return;
			}
			break;
		}

	}
	if(*hardware_class){
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_hw_class: Client Class");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, *hardware_class);
	}
	else
	{
		*error_codep = MSO_BB_CMTS_DEVICE_CLASS_ERROR;
		pin_set_err(ebufp, PIN_ERRLOC_CM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, MSO_FLD_DEVICE_TYPE, 0, 0);
		goto cleanup;
	}
cleanup:	
	if(PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_CMTS_DEVICE_CLASS_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_hw_class: Error", ebufp);
		if(cache_flistp != NULL) PIN_FLIST_DESTROY_EX(&cache_flistp, NULL);
		return;
	}
	if(cache_flistp != NULL)PIN_FLIST_DESTROY_EX(&cache_flistp, NULL);
	return;
}

static void
fm_mso_get_speed(
	pcm_context_t		*ctxp, 
	char				*prov_tag, 
	int				    *tech, 
	char				*srv_code, 
	//Added new parameter
	int				    *action,
	char				**package_class,
	int				    *bill_when,
	char				*city,
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*cache_flistp = NULL;
	cm_cache_key_iddbstr_t		kids;
    pin_cookie_t		      	cookie = NULL;
    int32				rec_id = 0;
	int32				err = PIN_ERR_NONE;
	int				    *tec = 0;
	int				    *srv_typ = (int *)3;
	pin_decimal_t		*upload = NULL;
	pin_decimal_t		*dload = NULL;
	char				*up_speed = NULL;
	char				*dl_speed = NULL;
	time_t				last_mod_t = 0;
	time_t				now_t = 0;
	pin_flist_t			*cfg_cr_flistp = NULL;

	if(PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
        /******************************************************
        * If the cache is not enabled, short circuit right away
        ******************************************************/
        if ( mso_prov_bb_config_ptr == (cm_cache_t *)NULL ) {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
                "no config flist for /mso_config_bb_pt cached ");
		*error_codep = MSO_BB_CONFIG_PT_ERROR;
                pin_set_err(ebufp, PIN_ERRLOC_CM,
                                PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_FLD_CACHE_NAME, 0, 0);

        }
        /**************************************************
         * See if the entry is in our data dictionary cache
         * Prepare the cm_cache_key_iddbstr_t structure to search
         **************************************************/
        kids.id = 0;
        kids.db = 0;
        kids.str = "/mso_config_bb_pt";
        cache_flistp = cm_cache_find_entry(mso_prov_bb_config_ptr,
                                        &kids, &err);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "cm_cache_find_entry found flist", cache_flistp);
        switch(err) {
        case PIN_ERR_NONE:
                break;
        default:
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
                        "mso_prov_bb_config_object from_cache: error "
                        "accessing data dictionary cache.");
		    	*error_codep = MSO_BB_CONFIG_PT_ERROR;
                pin_set_err(ebufp, PIN_ERRLOC_CM,
                   PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_FLD_CACHE_NAME, 0, 0);
                goto cleanup;
        }
	last_mod_t = *(time_t*)PIN_FLIST_FLD_GET(cache_flistp, PIN_FLD_MOD_T,0,ebufp);
	now_t = pin_virtual_time((time_t *)NULL);
	if(now_t  >= last_mod_t + refresh_pt_interval )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"calling mso_prov_bb_config_object_update to refresh provisioning tag config");
		fm_mso_prov_bb_config_cache_update(ctxp, ebufp);
		kids.id = 0;
		kids.db = 0;
		kids.str = "/mso_config_bb_pt";
		if(cache_flistp != NULL)PIN_FLIST_DESTROY_EX(&cache_flistp, NULL);
		cache_flistp = cm_cache_find_entry(mso_prov_bb_config_ptr,&kids, &err);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"cm_cache_find_entry found flist", cache_flistp);
		switch(err)
		{
			case PIN_ERR_NONE:
				break;
			default:
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
					"mso_prov_bb_config_object from_cache: error "
					"accessing data dictionary cache.");
				pin_set_err(ebufp, PIN_ERRLOC_CM,
						PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_FLD_CACHE_NAME, 0, 0);
				goto cleanup;
		}
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, prov_tag);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, srv_code);
	while ( (tmp_flistp = PIN_FLIST_ELEM_GET_NEXT (cache_flistp,
		PIN_FLD_RESULTS, &rec_id, 1,&cookie, ebufp))!= (pin_flist_t *)NULL )
	{
		if (strcmp((char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_PROVISIONING_TAG, 0, ebufp ),prov_tag) == 0 
		//	&& strcmp((char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_SERVICE_CODE, 0, ebufp ), srv_code) == 0
		   )
		{
			tec = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_BB_TECHNOLOGY, 0, ebufp);
			if(*tec == *tech)
			{
				/**** Pavan Bellala 25-11-2015 ********************
				Added condition for getting speed based on action
				***************************************************/
				fm_mso_get_service_quota_codes(ctxp, srv_code, bill_when, city, &cfg_cr_flistp, ebufp);
				if(cfg_cr_flistp && *action != DOC_BB_UPDATE_CAP){
					upload = PIN_FLIST_FLD_GET(cfg_cr_flistp, MSO_FLD_UL_SPEED, 0, ebufp);
					dload = PIN_FLIST_FLD_GET(cfg_cr_flistp, MSO_FLD_DL_SPEED, 0, ebufp);
				}
				/*if( *action != DOC_BB_UPDATE_CAP)
				{
					upload = (pin_decimal_t *)PIN_FLIST_FLD_GET(tmp_flistp, 
						MSO_FLD_UL_SPEED, 0, ebufp);
					dload = (pin_decimal_t *)PIN_FLIST_FLD_GET(tmp_flistp, 
						MSO_FLD_DL_SPEED, 0, ebufp);
				}*/ 
				else if(cfg_cr_flistp) {
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"FUP throttle action");
					upload = (pin_decimal_t *)PIN_FLIST_FLD_GET(cfg_cr_flistp, MSO_FLD_FUP_UL_SPEED, 0, ebufp);
					dload = (pin_decimal_t *)PIN_FLIST_FLD_GET(cfg_cr_flistp, MSO_FLD_FUP_DL_SPEED, 0, ebufp);
				}
				if(PIN_ERRBUF_IS_ERR(ebufp)){
					*error_codep = MSO_BB_SPEED_ERROR;
					goto cleanup;
				}
				break;
			} else {
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,"Technology mismatch in configuration",tmp_flistp);
				*error_codep = MSO_BB_CONFIG_PT_TECH_ERROR;
				pin_set_err(ebufp, PIN_ERRLOC_CM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,PIN_ERR_NOT_FOUND,MSO_FLD_TECHNOLOGY,0,0);
				goto cleanup;
			}
		}

	}
	if(!pbo_decimal_is_null(upload, ebufp)){
		up_speed = pbo_decimal_to_str(upload, ebufp);
	}
	else{
		 *error_codep = MSO_BB_ULSPEED_ERROR;
		 pin_set_err(ebufp, PIN_ERRLOC_CM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, MSO_FLD_UL_SPEED, 0, 0);
		 goto cleanup;
	}
	if(!pbo_decimal_is_null(dload, ebufp)){
		dl_speed = pbo_decimal_to_str(dload, ebufp);
	}
	else{
		*error_codep = MSO_BB_DLSPEED_ERROR;
		pin_set_err(ebufp, PIN_ERRLOC_CM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, MSO_FLD_DL_SPEED, 0, 0);
		goto cleanup;
	}
	if(PIN_ERRBUF_IS_ERR(ebufp)){
		goto cleanup;
	}
	*package_class = malloc(strlen(up_speed)+strlen(dl_speed)+strlen("_")+3);
	//sprintf(*package_class, "%s_%s", up_speed, dl_speed);
	sprintf(*package_class, "%s_%s", dl_speed, up_speed);
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_speed: Speed");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, *package_class);

cleanup:
	if(PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_speed: Error", ebufp);
		if(cache_flistp != NULL)PIN_FLIST_DESTROY_EX(&cache_flistp, NULL);
		if(up_speed)
			pin_free(up_speed);
		if(dl_speed)
			pin_free(dl_speed);
		return;
	}
	if(up_speed)
		pin_free(up_speed);
	if(dl_speed)
		pin_free(dl_speed);
	if(cache_flistp != NULL)PIN_FLIST_DESTROY_EX(&cache_flistp, NULL);
	return;
}

static void
fm_mso_get_tal_class(
	pcm_context_t	*ctxp, 
	char			*node_name, 
	char			*prov_tag, 
	char			**tal_class,
    char            *str_rolep,
	int			    *isTAL,
	int			    *tech,
	int			    *error_codep,
	pin_errbuf_t		*ebufp)
{
	poid_t			*s_pdp = NULL;
	poid_t			*pdp = NULL;
	int64			database = 0;
	int32			rec_id = 0;
	int32			s_flags = SRCH_DISTINCT + SRCH_EXACT;//Pawan:27-02-2015 Added SRCH_EXACT to fix search issue.
	pin_cookie_t	cookie = NULL;
	int32			rec_id2 = 0;
	int			    flag = 0;
	pin_cookie_t	cookie2 = NULL;
	pin_flist_t		*search_dev_flistp = NULL;
	pin_flist_t		*search_dev_o_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*class_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_flist_t		*arg2_flistp = NULL;
	pin_flist_t		*temp_flistp = NULL;
	char			*class_name = NULL;
	char			*temp_class = NULL;
	char			test[10] = {'\0'};
	int			    *cc_id = 0;
	int			    elem_count = 0;

	if(PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	
	sprintf(test, "%d", *isTAL);
	PIN_ERR_LOG_MSG(3, "Is Tal Flag Value:");
	PIN_ERR_LOG_MSG(3, test);
	
	sprintf(test, "%d", *tech);
	PIN_ERR_LOG_MSG(3, "Tech Value:");
	PIN_ERR_LOG_MSG(3, test);
	*tal_class = NULL;
	/***********************************************************
	 * assume db matches userid found in pin.conf
	 ***********************************************************/
	pdp = PCM_GET_USERID(ctxp);
	database = PIN_POID_GET_DB(pdp);
	search_dev_flistp = PIN_FLIST_CREATE(ebufp);
	//Get the TAL/DHCP class
	s_pdp = PIN_POID_CREATE(database, "/search", -1, ebufp);
	
	PIN_FLIST_FLD_PUT(search_dev_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_FLD_SET(search_dev_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
	
	PIN_FLIST_FLD_SET(search_dev_flistp, PIN_FLD_TEMPLATE,
		"select X from /mso_cfg_cmts_cc_mapping where F1 = V1 and F2 = V2 " , ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(search_dev_flistp, PIN_FLD_ARGS, 1, ebufp);
	arg2_flistp = PIN_FLIST_ELEM_ADD(arg_flistp, MSO_FLD_CC_MAPPING, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(arg2_flistp, PIN_FLD_PROVISIONING_TAG, prov_tag, ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(search_dev_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_CMTS_ID, node_name, ebufp);
	PIN_FLIST_ELEM_SET( search_dev_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Get client classes input flist", search_dev_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_dev_flistp, &search_dev_o_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Get client classes output flist", search_dev_o_flistp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_TAL_ERROR;
             PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "error getting client classes", ebufp);
             goto cleanup;
	}
	elem_count = PIN_FLIST_ELEM_COUNT(search_dev_o_flistp, PIN_FLD_RESULTS, ebufp);
	if(elem_count == 0 || elem_count > 1)
	{
		*error_codep = MSO_BB_CC_CONFIG_ERROR;
		pin_set_err(ebufp, PIN_ERRLOC_CM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, MSO_FLD_CLIENT_CLASS_VALUE, 0, 0);
		goto cleanup;
	}
	rec_id = 0;
	cookie = NULL;
	while ( (temp_flistp = PIN_FLIST_ELEM_GET_NEXT (search_dev_o_flistp,
                       PIN_FLD_RESULTS, &rec_id, 1,&cookie, ebufp))
                                                  != (pin_flist_t *)NULL ) 
        {
        	//class_list = (char *)malloc(sizeof(char));
        	cookie2 = NULL;
		rec_id2 = 0;
        	while ( (class_flistp = PIN_FLIST_ELEM_GET_NEXT (temp_flistp,
                       MSO_FLD_CC_MAPPING, &rec_id2, 1,&cookie2, ebufp))
                                                  != (pin_flist_t *)NULL )
		{
			if (str_rolep && strstr(str_rolep, "-Static"))
			{
				cc_id = PIN_FLIST_FLD_GET(class_flistp, MSO_FLD_CLIENT_CLASS_ID, 0, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp)) {
					*error_codep = MSO_BB_CC_ID_ERROR;
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						"error getting client classes", ebufp);
					goto cleanup;
				}
				if(*cc_id == TAL_CLASS)
				{
					temp_class = (char *)PIN_FLIST_FLD_GET(class_flistp, 
						MSO_FLD_CLIENT_CLASS_VALUE, 0,ebufp);
					if (PIN_ERRBUF_IS_ERR(ebufp)) {
						*error_codep = MSO_BB_TAL_ERROR;
					     PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
							"error getting client classes", ebufp);
					     goto cleanup;
					}
				}
				else if(*cc_id == IP_POOL)
				{
					class_name = (char *)PIN_FLIST_FLD_GET(class_flistp, 
						MSO_FLD_CLIENT_CLASS_VALUE, 0,ebufp);
					if (PIN_ERRBUF_IS_ERR(ebufp)) {
						*error_codep = MSO_BB_TAL_ERROR;
					     PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
							"error getting client classes", ebufp);
					     goto cleanup;
					}
				}
				PIN_ERR_LOG_MSG(3, "Test 1!!!");
			}
			else
			{
				PIN_ERR_LOG_MSG(3, "Test 1!!!");
				cc_id = PIN_FLIST_FLD_GET(class_flistp, MSO_FLD_CLIENT_CLASS_ID, 0, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp)) {
					*error_codep = MSO_BB_CC_ID_ERROR;
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						"error getting client classes", ebufp);
					goto cleanup;
				}
				if(*cc_id == DHCP_CLASS)
				{
					PIN_ERR_LOG_MSG(3, "Test 2!!!");
					temp_class = (char *)PIN_FLIST_FLD_GET(class_flistp, 
						MSO_FLD_CLIENT_CLASS_VALUE, 0,ebufp);
					if (PIN_ERRBUF_IS_ERR(ebufp)) {
						*error_codep = MSO_BB_TAL_ERROR;
					     PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
							"error getting client classes", ebufp);
					     goto cleanup;
					}

					break;
					//class_name = (char *)malloc(strlen(temp_class)+1);
					//sprintf(class_name, "%s", temp_class);
				}

			}
		}

//		class_flistp = PIN_FLIST_ELEM_GET(temp_flistp, MSO_FLD_CC_MAPPING, 0, 0, ebufp);
//		if (PIN_ERRBUF_IS_ERR(ebufp)) {
//			*error_codep = MSO_BB_TAL_ERROR;
//		     PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
//				"error getting client classes", ebufp);
//		     goto cleanup;
//		}
//		class_name = (char *)PIN_FLIST_FLD_GET(class_flistp, MSO_FLD_CLIENT_CLASS_VALUE, 0,ebufp);
//		if (PIN_ERRBUF_IS_ERR(ebufp)) {
//			*error_codep = MSO_BB_TAL_ERROR;
//		     PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
//				"error getting client classes", ebufp);
//		     goto cleanup;
//		}
		if(str_rolep && strstr(str_rolep, "-Static"))
		{
			PIN_ERR_LOG_MSG(3, "Test 2!!!");
			//Pawan:25-02-2015: Added below "if" condition to avoid check for Docsis2 
			//if(*tech != 1)
			//{
				if(class_name && temp_class )
				{
					*tal_class = (char *)malloc(strlen(class_name)+strlen(temp_class)+2);
					// IP Pool value is removed from client class
					//sprintf(*tal_class, "%s,%s", class_name, temp_class);
					sprintf(*tal_class, "%s", temp_class);
					PIN_ERR_LOG_MSG(3, "Test 3!!!");
				}
				else{
					*error_codep = MSO_BB_CC_CONFIG_ERROR;
					pin_set_err(ebufp, PIN_ERRLOC_CM, 
						PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, 
						MSO_FLD_CLIENT_CLASS_VALUE, 0, 0);
					goto cleanup;
				}
			//}
		}
		else{
			if(temp_class){
				*tal_class = (char *)malloc(strlen(temp_class) + 1);
				sprintf(*tal_class, "%s", temp_class);
			}
			else
			{
				PIN_ERR_LOG_MSG(3, "Test 4!!!");
				*error_codep = MSO_BB_CC_CONFIG_ERROR;
				pin_set_err(ebufp, PIN_ERRLOC_CM, 
					PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, 
					MSO_FLD_CLIENT_CLASS_VALUE, 0, 0);
				goto cleanup;
			}
		}
		
		if(*tal_class) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "class_list: ");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, *tal_class);
		}
	}
	//Pawan:15-02-2015: Added "tech" in below "if" since tal_class is not mandatory for Docsis2
	if(*tech != 1 && !*tal_class){
		*error_codep = MSO_BB_CLIENT_CLASS_ERROR;
		pin_set_err(ebufp, PIN_ERRLOC_CM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, MSO_FLD_CLIENT_CLASS_VALUE, 0, 0);
		goto cleanup;
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "At the End.. ");
cleanup:
	PIN_FLIST_DESTROY_EX(&search_dev_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_dev_o_flistp, NULL);
	//if(class_name)
	//	free(class_name);
	return;
}


static void
ethernet_details_flist(
	pcm_context_t			*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp,
	char				*task,
	int				*error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*acc_detail_flistp = NULL;
	pin_flist_t			*personal_detail_flistp = NULL;
	pin_flist_t			*grp_detail_flistp = NULL;
	pin_flist_t			*user_att_flistp = NULL;
	pin_flist_t			*acc_nameinfo_flistp = NULL;
	pin_flist_t			*alias_list = NULL;

	poid_t				*ac_pdp = NULL;
	pin_cookie_t			cookie = NULL;
	int32				rec_id = 0;
	int32				*prov_flag = NULL;
	char				*node_name = NULL;
	char				*ne_add = NULL;
	char				subnet[100] = {'\0'};
	char				*city = NULL;

	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	/******Add the Task Array for QPS*******************/
	task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, task_id, ebufp);
	task_id = task_id + 1;
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	prov_flag = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_FLAGS, 1, ebufp);
	alias_list = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_ALIAS_LIST, 0, 1, ebufp );
	if ( prov_flag && *prov_flag == DOC_BB_CAM_TP && alias_list)
	{
		PIN_FLIST_FLD_COPY(alias_list, PIN_FLD_NAME, task_flistp, MSO_FLD_OLD_USER_NAME, ebufp);
	}

	node_name = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NE_ELEMENT_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"ethernet_details_flist: Network Element Missing",ebufp);
		goto cleanup;
	}
	//Call the function to get the NE Address
	//fm_mso_get_network_node(ctxp, node_name, &ne_add, 2, error_codep, ebufp);
	city = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_CITY_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"ethernet_details_flist: Network Element Missing",ebufp);
		goto cleanup;
	}
	//Call the function to get the NE Address
	fm_mso_get_network_node(ctxp, node_name, city, &ne_add, 2, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"ethernet_details_flist: Not able to get Network Element ID",ebufp);
		goto cleanup;
	}
	if(ne_add){
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, (void *)ne_add, ebufp);
	}
	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, (void *)task, ebufp);
	//PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, "NSURE01", ebufp);
	ac_pdp = (poid_t *)PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_POID, 0, ebufp);
	
	acc_detail_flistp = PIN_FLIST_CREATE(ebufp);
	personal_detail_flistp = PIN_FLIST_CREATE(ebufp);
	grp_detail_flistp = PIN_FLIST_CREATE(ebufp);
	user_att_flistp = PIN_FLIST_CREATE(ebufp);
	//Call the function to populate the create balance array
	fm_mso_add_personal_detail(ctxp,ac_pdp, &personal_detail_flistp, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "ethernet_details_flist: Error", ebufp);
		goto cleanup;
	}
	fm_mso_add_acc_detail(ctxp,*i_flistpp, so_flistp, &acc_detail_flistp, subnet, error_codep, ebufp); 
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "ethernet_details_flist: Error", ebufp);
		goto cleanup;
	}
	fm_mso_add_group_detail(ctxp, *i_flistpp, so_flistp, &grp_detail_flistp, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "ethernet_details_flist: Error", ebufp);
		goto cleanup;
	}
	fm_mso_add_user_attribute(ctxp, so_flistp, &user_att_flistp, subnet, error_codep,ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "ethernet_details_flist: Error", ebufp);
		goto cleanup;
	}
	
	PIN_FLIST_ELEM_COPY(personal_detail_flistp, MSO_FLD_PERSONAL_DETAIL,1,
		 task_flistp, MSO_FLD_PERSONAL_DETAIL, 1, ebufp);
	PIN_FLIST_ELEM_COPY(acc_detail_flistp, MSO_FLD_ACCOUNT_DETAIL,1,
		 task_flistp, MSO_FLD_ACCOUNT_DETAIL, 1, ebufp);
	PIN_FLIST_ELEM_COPY(grp_detail_flistp, MSO_FLD_GROUP_DETAIL,1,
		 task_flistp, MSO_FLD_GROUP_DETAIL, 1, ebufp);
	PIN_FLIST_ELEM_COPY(user_att_flistp, MSO_FLD_USER_ATTRIBUTES,1,
		 task_flistp, MSO_FLD_USER_ATTRIBUTES, 1, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "ethernet_details_flist: Error", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ethernet_details_flist: Return Flist", *i_flistpp);
cleanup:
	PIN_FLIST_DESTROY_EX(&acc_detail_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&personal_detail_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&grp_detail_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&user_att_flistp, NULL);
	return;
}

static void
ethernet_new_plan(
	pcm_context_t		*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp,
	int			    	*error_codep,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*acc_detail_flistp = NULL;
	pin_flist_t			*personal_detail_flistp = NULL;
	pin_flist_t			*grp_detail_flistp = NULL;
	pin_flist_t			*group_flistp = NULL;
	pin_flist_t			*user_att_flistp = NULL;
	pin_flist_t			*acc_nameinfo_flistp = NULL;
	pin_flist_t			*bb_flistp = NULL;
	poid_t				*ac_pdp = NULL;
	poid_t				*prod_pdp = NULL;
	pin_cookie_t		cookie = NULL;
	int32				rec_id = 0;
	char				*user_id = NULL;
	char				*old_plan_name = NULL;
	char				*plan_name = NULL;
	char				*node_name = NULL;
	char				*ne_add = NULL;
	char				*city = NULL;
	time_t				*end_t = NULL;
	int32               *action_flag = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	/******Add the Task Array for QPS*******************/
	task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, task_id, ebufp);
	task_id = task_id + 1;
	bb_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	node_name = (char *)PIN_FLIST_FLD_GET(bb_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NE_ELEMENT_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"ethernet_new_plan: Network Element Missing",ebufp);
		return;
	}
	//Call the function to get the NE Address
	//fm_mso_get_network_node(ctxp, node_name, &ne_add, 2, error_codep, ebufp);
	city = (char *)PIN_FLIST_FLD_GET(bb_flistp, PIN_FLD_CITY, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_CITY_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"ethernet_new_plan: Network Element Missing",ebufp);
		return;
	}
	//Call the function to get the NE Address
	fm_mso_get_network_node(ctxp, node_name, city, &ne_add, 2, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"ethernet_new_plan: Not able to get Network Element ID",ebufp);
		return;
	}
	if(ne_add){
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, (void *)ne_add, ebufp);
	}

	/* get the action flage to cpmpare and set the task name */
    	action_flag = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_FLAGS, 0, ebufp);
        if ( *action_flag ==  DOC_BB_ADD_MB)
        {
                 PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "INCREASE_LIMIT_NSURE", ebufp);
        }
	else
	{
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, "FUP_REVERSAL_NSURE", ebufp);
	}
	ac_pdp = (poid_t *)PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_POID, 0, ebufp);
	user_id = (char *)PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_LOGIN, 0, ebufp);
	
	PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_LOGIN, user_id, ebufp);
	
	fm_mso_add_group_detail(ctxp, *i_flistpp, so_flistp, &grp_detail_flistp, error_codep, ebufp); 
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "ethernet_new_plan: Error", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Add Group Return Flist: ", grp_detail_flistp);
	group_flistp = PIN_FLIST_ELEM_GET(grp_detail_flistp,MSO_FLD_GROUP_DETAIL,1,0,ebufp);
	
	//grp_detail_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_GROUP_DETAIL, 1, ebufp);
	
	//get_last_plan(ctxp, ac_pdp, &old_plan_name, &end_t, &prod_pdp, error_codep, ebufp);
	get_old_group(ctxp, *i_flistpp, &old_plan_name, &end_t, &prod_pdp, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "ethernet_new_plan: Error", ebufp);
		return;
	}
	PIN_FLIST_FLD_SET(group_flistp, MSO_FLD_OLDGROUP, old_plan_name, ebufp);
	
	plan_name = (char *)PIN_FLIST_FLD_GET(group_flistp, MSO_FLD_GROUP, 0, ebufp);
	
	PIN_FLIST_FLD_SET(group_flistp, MSO_FLD_NEWGROUP, plan_name, ebufp);
	PIN_FLIST_FLD_SET(group_flistp, MSO_FLD_METERING_OVERRIDE, "n", ebufp);
	
	PIN_FLIST_FLD_DROP(group_flistp, MSO_FLD_GROUP, ebufp);
	PIN_FLIST_FLD_DROP(group_flistp, MSO_FLD_MAX_SESSION, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Group Flist: ", group_flistp);
	PIN_FLIST_ELEM_COPY(grp_detail_flistp, MSO_FLD_GROUP_DETAIL,1,
		 task_flistp, MSO_FLD_GROUP_DETAIL, 1, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "ethernet_new_plan: Error", ebufp);
		if(prod_pdp){
			PIN_POID_DESTROY(prod_pdp, ebufp);
		}
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Add Group Return Flist: ", task_flistp);
	if(prod_pdp){
		PIN_POID_DESTROY(prod_pdp, ebufp);
	}
	return ;	
}

static void
fm_mso_add_personal_detail(
	pcm_context_t		*ctxp,
	poid_t				*ac_pdp,
	pin_flist_t			**personal_flistp,
	int  				*error_codep,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t			*acc_iflistp = NULL;
	pin_flist_t			*acc_oflistp = NULL;
	pin_flist_t			*phones = NULL;
	pin_flist_t			*nameinfo_flistp = NULL;
	pin_flist_t			*per_flistp = NULL;
	pin_flist_t			*srch_iflist = NULL;
	pin_flist_t			*srch_oflist = NULL;
	pin_flist_t			*arg_flist = NULL;
	pin_flist_t			*results = NULL;
	pin_flist_t			*cc_nameinfo_inst = NULL;

	char				*template = "select X from /mso_customer_credential where F1.id = V1 "	 ;
	char				*b_num = NULL;
	char				*b_name = NULL;
	char				*s_name = NULL;
	char				address[1024]="";
	int				    *type = 0;
	int32				srch_flags = 512;
	int64				db_no = 1;
	pin_cookie_t		cookie = NULL;
	int32				rec_id = 0;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	per_flistp = PIN_FLIST_ELEM_ADD(*personal_flistp, MSO_FLD_PERSONAL_DETAIL, 1, ebufp);
	
	acc_iflistp = PIN_FLIST_CREATE(ebufp);
	
	PIN_FLIST_FLD_SET(acc_iflistp, PIN_FLD_POID, ac_pdp, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_add_personal_detail: Read input flist", acc_iflistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, acc_iflistp, &acc_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_add_personal_detail: Read output flist", acc_oflistp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_add_personal_detail: Error calling read obj", ebufp);
		goto cleanup;
	}
	nameinfo_flistp = PIN_FLIST_ELEM_GET(acc_oflistp, PIN_FLD_NAMEINFO, 2, 0,ebufp);
	//Copy the fields
	PIN_FLIST_FLD_COPY(nameinfo_flistp, PIN_FLD_FIRST_NAME, per_flistp, PIN_FLD_FIRST_NAME, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_NAME_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_add_personal_detail: Error", ebufp);
		goto cleanup;
	}
	PIN_FLIST_FLD_COPY(nameinfo_flistp, PIN_FLD_LAST_NAME, per_flistp, PIN_FLD_LAST_NAME, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_NAME_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_add_personal_detail: Error", ebufp);
		goto cleanup;
	}
	PIN_FLIST_FLD_COPY(nameinfo_flistp, PIN_FLD_COMPANY, per_flistp, PIN_FLD_COMPANY, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_COMPANY_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_add_personal_detail: Error", ebufp);
		goto cleanup;
	}
	//PIN_FLIST_FLD_COPY(nameinfo_flistp, PIN_FLD_ADDRESS, per_flistp, PIN_FLD_ADDRESS, ebufp);
	/******************************************************************************
	 Get the Address(LOCATION, BUILDING) by searching /mso_customer_credential
	******************************************************************************/
	srch_iflist = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_iflist, PIN_FLD_POID, PIN_POID_CREATE(db_no, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_iflist, PIN_FLD_FLAGS, &srch_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_iflist, PIN_FLD_TEMPLATE, template, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, ac_pdp, ebufp);

	results =  PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch input flist", srch_iflist);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflist, &srch_oflist, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch onput flist", srch_oflist);
	
	if (srch_oflist)
	{
		results = PIN_FLIST_ELEM_GET(srch_oflist, PIN_FLD_RESULTS, 0, 1, ebufp);
		if (results)
		{
			cc_nameinfo_inst = PIN_FLIST_ELEM_GET(results, PIN_FLD_NAMEINFO, NAMEINFO_INSTALLATION, 1, ebufp);
			if (cc_nameinfo_inst)
			{
		    	b_num  = PIN_FLIST_FLD_GET(nameinfo_flistp, PIN_FLD_ADDRESS, 1, ebufp );
	    		b_name = PIN_FLIST_FLD_GET(cc_nameinfo_inst, MSO_FLD_BUILDING_NAME, 1, ebufp );
		    	s_name = PIN_FLIST_FLD_GET(cc_nameinfo_inst, MSO_FLD_STREET_NAME, 1, ebufp );
		    	sprintf(address, "%s,%s,%s", b_num, b_name, s_name);
				PIN_FLIST_FLD_SET(per_flistp, PIN_FLD_ADDRESS, address, ebufp);
			}
		}
	}


	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_ADDRESS_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_add_personal_detail: Error", ebufp);
		goto cleanup;
	}
	PIN_FLIST_FLD_COPY(nameinfo_flistp, PIN_FLD_CITY, per_flistp, PIN_FLD_CITY, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_MANDATORY_FIELD_CODE;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_add_personal_detail: Error", ebufp);
		goto cleanup;
	}
	PIN_FLIST_FLD_COPY(nameinfo_flistp, PIN_FLD_STATE, per_flistp, PIN_FLD_STATE, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_MANDATORY_FIELD_CODE;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_add_personal_detail: Error", ebufp);
		goto cleanup;
	}
	PIN_FLIST_FLD_COPY(nameinfo_flistp, PIN_FLD_COUNTRY, per_flistp, PIN_FLD_COUNTRY, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_MANDATORY_FIELD_CODE;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_add_personal_detail: Error", ebufp);
		goto cleanup;
	}
	PIN_FLIST_FLD_COPY(nameinfo_flistp, PIN_FLD_ZIP, per_flistp, PIN_FLD_ZIP, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_MANDATORY_FIELD_CODE;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_add_personal_detail: Error", ebufp);
		goto cleanup;
	}
	PIN_FLIST_FLD_COPY(nameinfo_flistp, PIN_FLD_EMAIL_ADDR, per_flistp, PIN_FLD_EMAIL_ADDR, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_EMAIL_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_add_personal_detail: Error", ebufp);
		goto cleanup;
	}
	//Get the phone number
	while ((phones = PIN_FLIST_ELEM_GET_NEXT (nameinfo_flistp,
		PIN_FLD_PHONES, &rec_id, 1,&cookie, ebufp)) != (pin_flist_t *)NULL )
	{
		type = (int *)PIN_FLIST_FLD_GET(phones, PIN_FLD_TYPE, 0, ebufp);
		if(*type == 1)
			PIN_FLIST_FLD_COPY(phones, PIN_FLD_PHONE, per_flistp, MSO_FLD_PHONE_NO, ebufp);
		else if (*type == 5)
		{
			PIN_FLIST_FLD_COPY(phones, PIN_FLD_PHONE, per_flistp, MSO_FLD_MOBILE, ebufp);
		}
	}
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_PHONE_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_add_personal_detail: Error", ebufp);
		goto cleanup;
	}
	PIN_FLIST_FLD_SET(per_flistp, MSO_FLD_BUSINESS_ENTITY, "CORP", ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_add_personal_detail: Personal Details", *personal_flistp); 
cleanup:
	PIN_FLIST_DESTROY_EX(&acc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acc_oflistp, NULL);
	if(srch_iflist)PIN_FLIST_DESTROY_EX(&srch_iflist, NULL);
	if(srch_oflist)PIN_FLIST_DESTROY_EX(&srch_oflist, NULL);
	return ;
	
}

static void
fm_mso_add_acc_detail(
	pcm_context_t		*ctxp,
	pin_flist_t			*i_flistpp,
	pin_flist_t			*so_flistp,
	pin_flist_t			**acc_detail_flistp,
	char				*subnet,
	int			    	*error_codep,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t			*detail_flistp = NULL;
	pin_flist_t			*alias_list = NULL;
	pin_flist_t			*bb_info = NULL;
	pin_flist_t			*ip_flistp = NULL;
	pin_flist_t			*temp_flistp = NULL;
	pin_flist_t			*deviceflistp = NULL;
	poid_t				*serv_poid = NULL;
	struct tm			*gm_time;
	int			    	*isTAL = 0;
	char				*pass = NULL;
	time_t			   	pvt = 0;
	char				time_buf[30] = {'\0'};
	char				param[20] = {'\0'};
	char				*ip_add = NULL;
	char				*ip_add_list = NULL;
	int			    	flag = 0;
	char				*mso_ip_add = NULL;
	char				*ip_pool = NULL;
	char				*ip_mask = NULL;
	char				*mac_add = NULL;
	char				*dev_type = NULL;
	//char				subnet[100] = {'\0'};
	int32				ip_count = 0;
	pin_cookie_t		cookie = NULL;
	int32				rec_id = 0;
	pin_cookie_t		cookie1 = NULL;
	int32				rec_id1 = 0;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	detail_flistp = PIN_FLIST_ELEM_ADD(*acc_detail_flistp, MSO_FLD_ACCOUNT_DETAIL, 1, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_add_acc_detail: Service Input Flist", so_flistp);
	//Set the fields in the array
	//Check the isTAL flag:
	serv_poid = (poid_t *)PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_POID, 0, ebufp);
	bb_info = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	isTAL = (int *) PIN_FLIST_FLD_GET(bb_info, MSO_FLD_ISTAL_FLAG, 0, ebufp);
	pass = (char *)PIN_FLIST_FLD_GET(bb_info, MSO_FLD_PASSWORD, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_MANDATORY_FIELD_CODE;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_add_acc_detail: Error", ebufp);
		return;
	}
	
	if(*isTAL == 1)
	{
		PIN_FLIST_FLD_SET(detail_flistp, MSO_FLD_PASSWORD, "cisco", ebufp);
		strcpy(param,"/device/ip/static");
		/* get the modem device if any */
		fm_mso_search_devices(ctxp,param,serv_poid,&deviceflistp, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_add_user_attribute: Error", ebufp);
			goto cleanup;
		}
		flag = 0;
		ip_count = PIN_FLIST_ELEM_COUNT(deviceflistp, PIN_FLD_RESULTS, ebufp);
		while ((temp_flistp = PIN_FLIST_ELEM_GET_NEXT(deviceflistp, PIN_FLD_RESULTS, 
			&rec_id1, 1 , &cookie1, ebufp)) != (pin_flist_t *)NULL)
		{
			ip_add = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_DEVICE_ID, 0,ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp)) {
				*error_codep = MSO_BB_INTERNAL_ERROR;
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_add_user_attribute: Error", ebufp);
				goto cleanup;
			}
			//Pawan:17-03-2015: Added below block and commented next block
			if(flag == 1)
			{ 
				PIN_ERR_LOG_MSG(3, ip_add_list);
				ip_add_list = (char *)realloc(ip_add_list,(strlen(ip_add_list)+strlen(ip_add)+strlen(",")+ 3));
				//ip_add_list = (char *)realloc(ip_add_list, 150);
				PIN_ERR_LOG_MSG(3, "After Realloc");
				strcat(ip_add_list, ",");
				strcat(ip_add_list, ip_add);
			}
			else{
				ip_add_list = (char*)malloc(strlen(ip_add)+1);
				strcpy(ip_add_list, ip_add);
			}
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ip_add_list: ");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, ip_add_list);
			flag = 1;
				
			//ip_add_list = (char *)pin_malloc(1);
			//strcpy(ip_add_list, '\0');
			//cookie1 = NULL;
			//rec_id1 = 0;
			ip_flistp = PIN_FLIST_ELEM_GET(temp_flistp, MSO_FLD_IP_DATA, 0, 1, ebufp);
			if(ip_flistp)
			{
				/*
				mso_ip_add = PIN_FLIST_FLD_GET(ip_flistp, MSO_FLD_IP_ADDRS, 0,ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_add_list: ");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, mso_ip_add);
				if(flag == 1)
				{ 
					PIN_ERR_LOG_MSG(3, ip_add_list);
					ip_add_list = (char *)realloc(ip_add_list,(strlen(ip_add_list)+strlen(mso_ip_add)+strlen(",")+ 3));
					//ip_add_list = (char *)realloc(ip_add_list, 150);
					PIN_ERR_LOG_MSG(3, "After Realloc");
					strcat(ip_add_list, ",");
					strcat(ip_add_list, mso_ip_add);
				}
				else{
					ip_add_list = (char*)malloc(strlen(mso_ip_add)+1);
					strcpy(ip_add_list, mso_ip_add);
				}
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ip_add_list: ");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, ip_add_list);
				flag = 1;
				*/
				ip_pool = PIN_FLIST_FLD_GET(ip_flistp, MSO_FLD_IP_POOL_NAME, 1,ebufp);
				//ip_mask = PIN_FLIST_FLD_GET(ip_flistp, MSO_FLD_IP_MASK_VALUE, 1,ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp)) {
					*error_codep = MSO_BB_INTERNAL_ERROR;
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_add_user_attribute: Error", ebufp);
					goto cleanup;
				}

			}
		}
		
//		if(ip_add_list)
//		{
//			PIN_FLIST_FLD_PUT(user_flistp, MSO_FLD_USER_IP, ip_add_list, ebufp);
//			PIN_FLIST_FLD_SET(user_flistp, MSO_FLD_IP_POOL_NAME, ip_pool, ebufp);
//			acc_id = PIN_POID_GET_ID(ac_pdp);
//			sprintf(acc_id_str,"%d",acc_id);
//			
//			//PIN_FLIST_FLD_SET(user_flistp, MSO_FLD_EXTERNAL_REF_NO, acc_id_str, ebufp);
//			PIN_FLIST_FLD_SET(user_flistp, MSO_FLD_SUBNET_MASK, ip_mask, ebufp);
//			if (PIN_ERRBUF_IS_ERR(ebufp)) {
//				*error_codep = MSO_BB_INTERNAL_ERROR;
//				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_add_user_attribute: Error", ebufp);
//				goto cleanup;
//			}
//		}
		if(ip_add_list){
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "T1");
			PIN_FLIST_FLD_SET(detail_flistp, PIN_FLD_LOGIN, (void *)ip_add_list, ebufp);
			//PIN_FLIST_FLD_SET(detail_flistp, MSO_FLD_IP_POOL_NAME, ip_pool, ebufp);
			//acc_id = PIN_POID_GET_ID(ac_pdp);
			//sprintf(acc_id_str,"%d",acc_id);
			//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "T2");
			//PIN_FLIST_FLD_SET(user_flistp, MSO_FLD_EXTERNAL_REF_NO, acc_id_str, ebufp);
			//num = num - ip_count + 1;
			//sprintf(subnet, "255.255.255.%d", num);
			if(ip_count == 4)
			{
				strcpy(subnet, MSO_SUBNET_4);
				//PIN_FLIST_FLD_SET(detail_flistp, MSO_FLD_SUBNET_MASK, subnet, ebufp);
			}
			else if(ip_count == 8)
			{
				strcpy(subnet, MSO_SUBNET_8);
				//PIN_FLIST_FLD_SET(detail_flistp, MSO_FLD_SUBNET_MASK, subnet, ebufp);
			}
			else if(ip_count == 16)
			{
				strcpy(subnet, MSO_SUBNET_16);
				//PIN_FLIST_FLD_SET(detail_flistp, MSO_FLD_SUBNET_MASK, subnet, ebufp);
			}
			else if(ip_count == 32)
			{
				strcpy(subnet, MSO_SUBNET_32);
				//PIN_FLIST_FLD_SET(detail_flistp, MSO_FLD_SUBNET_MASK, subnet, ebufp);
			}
			else if(ip_count == 64)
			{
				strcpy(subnet, MSO_SUBNET_64);
				//PIN_FLIST_FLD_SET(detail_flistp, MSO_FLD_SUBNET_MASK, subnet, ebufp);
			}
			//else
			//{
			//	strcpy(subnet, ip_mask);
			//}
			//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Subnet Mask");
			//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, subnet);
			
			if (PIN_ERRBUF_IS_ERR(ebufp)) {
				*error_codep = MSO_BB_INTERNAL_ERROR;
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_add_user_attribute: Error", ebufp);
				goto cleanup;
			}
		
		}
		else{
			*error_codep = MSO_BB_IP_ADD_ERROR;
			pin_set_err(ebufp, PIN_ERRLOC_CM, 
				PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_FLD_DEVICE_ID, 0, 0);
		}
		PIN_ERR_LOG_MSG(3, "Test 1223");
	}
	else{
		PIN_FLIST_FLD_SET(detail_flistp, MSO_FLD_PASSWORD, (void *)pass, ebufp);
		PIN_FLIST_FLD_COPY(so_flistp, PIN_FLD_LOGIN, detail_flistp, PIN_FLD_LOGIN, ebufp);
	}
	strcpy(param,"/device/modem");
	/* get the modem device if any */
	fm_mso_search_devices(ctxp,param,serv_poid,&deviceflistp, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_activation_flist: Error",ebufp);
		goto cleanup;
	}
	cookie = NULL;
	rec_id =0;
	while ( (temp_flistp = PIN_FLIST_ELEM_GET_NEXT (deviceflistp,
                       PIN_FLD_RESULTS, &rec_id, 1,&cookie, ebufp))
                                                  != (pin_flist_t *)NULL ) 
        {
        	mac_add = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_DEVICE_ID, 0,ebufp);
        	dev_type = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_DEVICE_TYPE, 0,ebufp);
        	//ip_count = 0;
	}
	//Add MAC Address
	if(mac_add)
	{
		//cred_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_CREDENTIALS, 3, ebufp);
		PIN_FLIST_FLD_SET(detail_flistp, MSO_FLD_MAC_ID, (void *)mac_add, ebufp);
		//PIN_FLIST_FLD_SET(cred_flistp, PIN_FLD_DESCR, "MAC Address", ebufp);
		/**** Pavan Bellala 29-07-2015 ***********************************
		 Defect 1136 : Validate device string pattern before provisioning
		*****************************************************************/
		if(mac_add && strcmp(mac_add,"")!=0)
		{
			//Added the below code for JIO-CPS Integration-ISP Enhancement
			//Skip MAC validation for ONU (GPON) modem for JIO and Hathway Networks and validate the device_id pattern for other device types.
			if((dev_type && 
				!(strcmp(dev_type, HW_GPON_MODEM) == 0 || 
				  strcmp(dev_type, HW_ONU_GPON_MODEM) == 0 ||
				  strcmp(dev_type, JIO_GPON_MODEM) == 0 )) &&
				 fm_mso_integ_match_patterns("^([0-9a-fA-F]{2})(([/\\s:-][0-9a-fA-F]{2}){5})$", mac_add) <=0 )
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR , "Error:Invalid MAC id");
				pin_set_err(ebufp, PIN_ERRLOC_FM,
							PIN_ERRCLASS_SYSTEM_DETERMINATE,PIN_ERR_BAD_VALUE, PIN_FLD_DEVICE_ID, 0, 0);
				*error_codep = MSO_BB_INVALID_MAC;
				goto cleanup;
			}
		}
	}
	//PIN_FLIST_FLD_COPY(so_flistp, PIN_FLD_PASSWD, detail_flistp, MSO_FLD_PASSWORD, ebufp);
	//Get the MAC Addr
	alias_list = PIN_FLIST_ELEM_GET (so_flistp, PIN_FLD_ALIAS_LIST, 1, 1, ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "T2");
	if(alias_list)
	{
		
		PIN_FLIST_FLD_COPY(alias_list,  PIN_FLD_NAME, detail_flistp, MSO_FLD_MAC_ID, ebufp);
	}
	
	//Get the PASSWORD Expiration time
	pvt = pin_virtual_time(NULL);
	// Added below to add 1 day since expiry date has to be greater than current date.
	pvt = pvt + 86400;
	gm_time = gmtime(&pvt);
	strftime(time_buf, sizeof(time_buf), "%d%m%Y", gm_time);
	
	PIN_FLIST_FLD_SET(detail_flistp, MSO_FLD_EXPIRYDATE, (void *)time_buf, ebufp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_MANDATORY_FIELD_CODE;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_add_acc_detail: Error", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_add_acc_detail: Acc Detail", detail_flistp);
cleanup :
	if(ip_add_list)
		free(ip_add_list);
	return ;
	
}


static void
fm_mso_add_group_detail(
	pcm_context_t		*ctxp,
	pin_flist_t			*i_flistpp,
	pin_flist_t			*so_flistp,
	pin_flist_t			**group_detail_flistp,
	int				    *error_codep,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t			*bal_flistp = NULL;
	pin_flist_t			*bal_o_flistp = NULL;
	pin_flist_t			*balance_flistp = NULL;
	pin_flist_t			*sub_bal_flistp = NULL;
	pin_flist_t			*grp_detail_flistp = NULL;
	pin_flist_t			*plan_iflistp = NULL;
	pin_flist_t			*plan_oflistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*bb_info = NULL;
	pin_flist_t			*sub_bal_imp_flistp = NULL;

	poid_t				*plan_pdp = NULL;
	poid_t				*ac_pdp = NULL;

	int				    flag = 1;
	int			     	plan_type = 0;

	pin_decimal_t		*curr_bal = NULL;
	pin_decimal_t		*total_bal = pbo_decimal_from_str("0.0",ebufp);
	pin_decimal_t		*initial_amount = NULL;

	struct tm			*gm_time;

	time_t				*valid_from = NULL;
	time_t				*valid_to = NULL;
	time_t				pvt = 0;

	char				time_buf[30] = {'\0'};
	char				*plan_name = NULL;
	char				*prov_tag = NULL;
	char				*tmp_str = NULL;

	pin_cookie_t		cookie = NULL;
	int32				rec_id = 0;
	int32				tech = ETHERNET;
	int32				*res_id = NULL;
	int32				req_res_id = 0;

	void				*vp = NULL;

	FILE				*filep = NULL;

	filep = fopen("/tmp/stackp","a+");
	
	tmp_flistp = PIN_FLIST_CREATE(ebufp);
	grp_detail_flistp = PIN_FLIST_ELEM_ADD(tmp_flistp, MSO_FLD_GROUP_DETAIL, 1, ebufp);
	ac_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistpp, PIN_FLD_POID, 0, ebufp);
	plan_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistpp, PIN_FLD_PLAN_OBJ, 0, ebufp);
//	bb_info = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 1, ebufp);
//	if (bb_info)
//	{
//		vp = PIN_FLIST_FLD_GET(bb_info, MSO_FLD_TECHNOLOGY, 1, ebufp);
//		if (vp)
//		{
//			technology = *(int32*)vp;
//		}
//	}
//	
	plan_iflistp = PIN_FLIST_CREATE(ebufp);
	
	PIN_FLIST_FLD_SET(plan_iflistp, PIN_FLD_POID, plan_pdp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"i_flistpp", i_flistpp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read input flist", plan_iflistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, plan_iflistp, &plan_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read output flist", plan_oflistp);
	
	plan_name = (char *)PIN_FLIST_FLD_GET(plan_oflistp, PIN_FLD_NAME, 0, ebufp);

	PIN_FLIST_FLD_SET(grp_detail_flistp, MSO_FLD_GROUP, plan_name, ebufp);
	
	bal_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_POID, (void *)ac_pdp, ebufp);
	PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_FLAGS, (void *)&flag, ebufp);
	PIN_FLIST_ELEM_SET( bal_flistp, NULL, PIN_FLD_BALANCES, PIN_ELEMID_ANY, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Get Balances input flist", bal_flistp);
	PCM_OP(ctxp, PCM_OP_BAL_GET_BALANCES, 0, bal_flistp, &bal_o_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Get Balances output flist", bal_o_flistp)
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_MANDATORY_FIELD_CODE;
             PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "error getting Balances", ebufp);
             goto cleanup;
	}

	prov_tag = PIN_FLIST_FLD_GET(i_flistpp, PIN_FLD_PROVISIONING_TAG, 1, ebufp);
	if (!prov_tag)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"PROV TAG missing in input", ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "group_detail_flistp",*group_detail_flistp)
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, plan_name);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, prov_tag);
	fm_quota_check(ctxp, plan_name, prov_tag, &tech, &plan_type, &initial_amount, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"error in fm_quota_check()", ebufp);
		goto cleanup;
	}
	
	pvt = pin_virtual_time(NULL);
	if (plan_type == 1)
	{
		balance_flistp = PIN_FLIST_ELEM_GET(bal_o_flistp, PIN_FLD_BALANCES, 1100010, 1, ebufp);
		req_res_id = MSO_FREE_MB;
	}
	else if(plan_type == 2)
	{
		balance_flistp = PIN_FLIST_ELEM_GET(bal_o_flistp, PIN_FLD_BALANCES, 1000104, 1, ebufp);
		req_res_id = MSO_FUP_TRACK;
	}


	if(balance_flistp)
	{
		while ( (sub_bal_flistp = PIN_FLIST_ELEM_GET_NEXT (balance_flistp, PIN_FLD_SUB_BALANCES, 
			&rec_id, 1,&cookie, ebufp)) != (pin_flist_t *)NULL ) 
		{
			valid_from = PIN_FLIST_FLD_GET(sub_bal_flistp, PIN_FLD_VALID_FROM, 0,ebufp);
			valid_to = PIN_FLIST_FLD_GET(sub_bal_flistp, PIN_FLD_VALID_TO, 0,ebufp);
			if(*valid_to > pvt )
			{
				curr_bal = PIN_FLIST_FLD_GET(sub_bal_flistp, PIN_FLD_CURRENT_BAL, 0,ebufp);
				pbo_decimal_add_assign(total_bal,curr_bal,ebufp);
				
				//curr_bal = pbo_decimal_multiply(curr_bal, pbo_decimal_from_str("1024.0",ebufp),ebufp);
	
				//PIN_FLIST_FLD_SET(grp_detail_flistp, MSO_FLD_KBBANK, pbo_decimal_to_str(curr_bal,ebufp), ebufp);
	
				//gm_time = gmtime(valid_to);
				//strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%T.000+0530", gm_time);
				//strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%T.000+0530", gm_time);
				//PIN_FLIST_FLD_SET(grp_detail_flistp, MSO_FLD_EXPIRYDATE, (void *)time_buf, ebufp);
			}
		}
	}
	//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Existing Balance");
	//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,pbo_decimal_to_str(total_bal,ebufp));

	cookie = NULL;
	rec_id = 0;
	while ( (sub_bal_imp_flistp = PIN_FLIST_ELEM_GET_NEXT (i_flistpp, PIN_FLD_SUB_BAL_IMPACTS, 
			&rec_id, 1,&cookie, ebufp)) != (pin_flist_t *)NULL ) 
	{
		res_id = PIN_FLIST_FLD_GET(sub_bal_imp_flistp, PIN_FLD_RESOURCE_ID, 1,ebufp);
		if (res_id && *res_id == req_res_id)
		{
			sub_bal_flistp = PIN_FLIST_ELEM_GET (sub_bal_imp_flistp,PIN_FLD_SUB_BALANCES, PIN_ELEMID_ANY, 1, ebufp);
			valid_from = PIN_FLIST_FLD_GET(sub_bal_flistp, PIN_FLD_VALID_FROM, 0,ebufp);
			valid_to = PIN_FLIST_FLD_GET(sub_bal_flistp, PIN_FLD_VALID_TO, 0,ebufp);
			if(*valid_to > pvt )
			{
				curr_bal = PIN_FLIST_FLD_GET(sub_bal_flistp, PIN_FLD_AMOUNT, 0,ebufp);
				pbo_decimal_add_assign(total_bal,curr_bal,ebufp);
			}
		}
	}
	//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,pbo_decimal_to_str(total_bal,ebufp));
	
	if (pbo_decimal_is_zero(total_bal, ebufp) && plan_type == 0)
	{
		pbo_decimal_add_assign(total_bal, pbo_decimal_from_str("999999999.0",ebufp),ebufp);
		//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Total Balance");
		//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,pbo_decimal_to_str(total_bal,ebufp));
		tmp_str = pbo_decimal_to_str(total_bal,ebufp);
		PIN_FLIST_FLD_SET(grp_detail_flistp, MSO_FLD_KBBANK, tmp_str, ebufp);
		if(tmp_str)
			pin_free(tmp_str);
	}
	else
	{
		pbo_decimal_negate_assign(total_bal,ebufp);
		pbo_decimal_multiply_assign(total_bal, pbo_decimal_from_str("1024.0",ebufp),ebufp);
		pbo_decimal_round_assign(total_bal, 0, ROUND_UP, ebufp);
		//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Total Balance");
		//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,pbo_decimal_to_str(total_bal,ebufp));
		tmp_str = pbo_decimal_to_str(total_bal,ebufp);
		PIN_FLIST_FLD_SET(grp_detail_flistp, MSO_FLD_KBBANK, tmp_str, ebufp);
		if(tmp_str)
			pin_free(tmp_str);
	}


	if (valid_to)
	{
		gm_time = gmtime(valid_to);
		strftime(time_buf, sizeof(time_buf), "%d%m%Y", gm_time);
		PIN_FLIST_FLD_SET(grp_detail_flistp, MSO_FLD_EXPIRYDATE, (void *)time_buf, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(grp_detail_flistp, MSO_FLD_EXPIRYDATE, "31122030", ebufp);
	}

	PIN_FLIST_FLD_SET(grp_detail_flistp, MSO_FLD_MAX_SESSION, "1", ebufp);
	*group_detail_flistp = PIN_FLIST_COPY(tmp_flistp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "group_detail_flistp", *group_detail_flistp);
   	
cleanup :
	PIN_FLIST_DESTROY_EX(&tmp_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&plan_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&plan_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&bal_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&bal_o_flistp, NULL);
//	if(!pbo_decimal_is_null(initial_amount, ebufp))
//	{	pbo_decimal_destroy(&initial_amount);}
	return ;
	
}

static void
fm_mso_add_user_attribute(
	pcm_context_t		*ctxp,
	pin_flist_t			*so_flistp,
	pin_flist_t			**user_att_flistp,
	char				*subnet,
	int				    *error_codep,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t			*user_flistp = NULL;
	pin_flist_t			*deviceflistp = NULL;
	pin_flist_t			*temp_flistp = NULL;
	pin_flist_t			*sub_bal_flistp = NULL;
	pin_flist_t			*ip_flistp = NULL;
	pin_flist_t			*rfld_iflistp = NULL;
	pin_flist_t			*rfld_oflistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	int			    	flag = 1;
	int64				acc_id = 0;
	char				*mso_ip_add = NULL;
	char				*ip_add = NULL;
	char				*ip_add_list = NULL;	
	char				*ip_pool = NULL;	
	char				*ip_mask = NULL;	
	char				acc_id_str[50] = {'\0'};	
	char				param[50] = {'\0'};
	char				*agg_no = NULL;
	//char				subnet[20] = {'\0'};
	poid_t				*serv_poid = NULL;
	poid_t				*ac_pdp = NULL;
	pin_cookie_t		cookie1 = NULL;
	int32				rec_id1 = 0;
	int32				ip_count = 0;
	int32				num = 255;
	int				    *tal = 0;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	serv_poid = PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_POID, 0, ebufp);
	ac_pdp = PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	user_flistp = PIN_FLIST_ELEM_ADD(*user_att_flistp, MSO_FLD_USER_ATTRIBUTES, 1, ebufp);
//	//Get the Agreement No
//	rfld_iflistp = PIN_FLIST_CREATE(ebufp);
//	PIN_FLIST_FLD_SET(rfld_iflistp, PIN_FLD_POID, (void *)serv_poid, ebufp);
//	tmp_flistp = PIN_FLIST_SUBSTR_ADD(rfld_iflistp, MSO_FLD_BB_INFO, ebufp);
//	
//	PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_AGREEMENT_NO, NULL, ebufp);
//	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, rfld_iflistp, &rfld_oflistp, ebufp);
//	if (PIN_ERRBUF_IS_ERR(ebufp)) {
//		*error_codep = MSO_BB_INTERNAL_ERROR;
//		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_add_user_attribute: Error", ebufp);
//		goto cleanup;
//	}
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
//		"fm_mso_add_user_attribute: Read Fields Output", rfld_oflistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	agg_no = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_AGREEMENT_NO, 0, ebufp);
	tal = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_ISTAL_FLAG, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_MANDATORY_FIELD_CODE;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_add_user_attribute: Error", ebufp);
		goto cleanup;
	}
	PIN_FLIST_FLD_SET(user_flistp, MSO_FLD_EXTERNAL_REF_NO, (void *)agg_no, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_add_user_attribute: Error", ebufp);
		goto cleanup;
	}
	PIN_FLIST_DESTROY_EX(&rfld_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&rfld_iflistp, NULL);
	if(subnet){
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Subnet Mask");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, subnet);
		PIN_FLIST_FLD_SET(user_flistp, MSO_FLD_SUBNET_MASK, subnet, ebufp);
	}
	if(*tal == 0)
	{
		strcpy(param,"/device/ip/framed");
		/* get the modem device if any */
		fm_mso_search_devices(ctxp,param,serv_poid,&deviceflistp, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_add_user_attribute: Error", ebufp);
			goto cleanup;
		}
		flag = 0;
		cookie1 = NULL;
		rec_id1 = 0;
		ip_count = PIN_FLIST_ELEM_COUNT(deviceflistp, PIN_FLD_RESULTS, ebufp);
		while((temp_flistp = PIN_FLIST_ELEM_GET_NEXT(deviceflistp, 
			PIN_FLD_RESULTS, &rec_id1, 1 , &cookie1, ebufp)) != (pin_flist_t *)NULL)
		{
			ip_add = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_DEVICE_ID, 0,ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp)) {
				*error_codep = MSO_BB_INTERNAL_ERROR;
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_add_user_attribute: Error", ebufp);
				goto cleanup;
			}
			if(flag == 1)
			{ 
				ip_add_list = (char *)realloc(ip_add_list,(strlen(ip_add_list)+strlen(ip_add)+strlen(",")+1));
				strcat(ip_add_list, ",");
				strcat(ip_add_list, ip_add);
			}
			else{
				ip_add_list = (char *)malloc(strlen(ip_add)+ 1);
				sprintf(ip_add_list, "%s", ip_add);
			}
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ip_add_list: ");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, ip_add_list);
			flag = 1;
			//ip_add_list = (char *)pin_malloc(1);
			ip_flistp = PIN_FLIST_ELEM_GET(temp_flistp, MSO_FLD_IP_DATA, 0, 1, ebufp);
			if(ip_flistp)
			{
				mso_ip_add = PIN_FLIST_FLD_GET(ip_flistp, MSO_FLD_IP_ADDRS, 0,ebufp);
				/*PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_ip_add: ");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, mso_ip_add);
				if(flag == 1)
				{ 
					ip_add_list = (char *)realloc(ip_add_list,(strlen(ip_add_list)+strlen(mso_ip_add)+strlen(",")+1));
					strcat(ip_add_list, ",");
					strcat(ip_add_list, mso_ip_add);
				}
				else{
					ip_add_list = (char *)malloc(strlen(mso_ip_add)+ 1);
					sprintf(ip_add_list, "%s", mso_ip_add);
				}
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ip_add_list: ");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, ip_add_list);
				flag = 1;
				*/
				ip_pool = PIN_FLIST_FLD_GET(ip_flistp, MSO_FLD_IP_POOL_NAME, 0,ebufp);
				ip_mask = PIN_FLIST_FLD_GET(ip_flistp, MSO_FLD_IP_MASK_VALUE, 0,ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp)) {
					*error_codep = MSO_BB_INTERNAL_ERROR;
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_add_user_attribute: Error", ebufp);
					goto cleanup;
				}

			}
		}
		
		if(ip_add_list)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "T1");
			PIN_FLIST_FLD_SET(user_flistp, MSO_FLD_USER_IP, ip_add_list, ebufp);
			PIN_FLIST_FLD_SET(user_flistp, MSO_FLD_IP_POOL_NAME, ip_pool, ebufp);
			acc_id = PIN_POID_GET_ID(ac_pdp);
			sprintf(acc_id_str,"%d",acc_id);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "T2");
			//PIN_FLIST_FLD_SET(user_flistp, MSO_FLD_EXTERNAL_REF_NO, acc_id_str, ebufp);
			//num = num - ip_count + 1;
			//sprintf(subnet, "255.255.255.%d", num);
//			if(ip_count == 4)
//			{
//				strcpy(subnet, MSO_SUBNET_4);
//				PIN_FLIST_FLD_SET(user_flistp, MSO_FLD_SUBNET_MASK, subnet, ebufp);
//			}
//			else if(ip_count == 8)
//			{
//				strcpy(subnet, MSO_SUBNET_8);
//				PIN_FLIST_FLD_SET(user_flistp, MSO_FLD_SUBNET_MASK, subnet, ebufp);
//			}
//			else if(ip_count == 16)
//			{
//				strcpy(subnet, MSO_SUBNET_16);
//				PIN_FLIST_FLD_SET(user_flistp, MSO_FLD_SUBNET_MASK, subnet, ebufp);
//			}
//			else if(ip_count == 32)
//			{
//				strcpy(subnet, MSO_SUBNET_32);
//				PIN_FLIST_FLD_SET(user_flistp, MSO_FLD_SUBNET_MASK, subnet, ebufp);
//			}
//			else if(ip_count == 64)
//			{
//				strcpy(subnet, MSO_SUBNET_64);
//				PIN_FLIST_FLD_SET(user_flistp, MSO_FLD_SUBNET_MASK, subnet, ebufp);
//			}
			//else
			//{
			//	strcpy(subnet, ip_mask);
			//}
			
			if (PIN_ERRBUF_IS_ERR(ebufp)) {
				*error_codep = MSO_BB_INTERNAL_ERROR;
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_add_user_attribute: Error", ebufp);
				goto cleanup;
			}
		}
	}

cleanup :
	PIN_FLIST_DESTROY_EX(&rfld_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&rfld_oflistp, NULL);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "T3");
	if(ip_add_list)
		free(ip_add_list);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "T4");
	return ;
	
}


static void
fm_mso_get_login(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	char			**login,
	pin_flist_t		*so_flistp,
	int		    	*error_codep,
	pin_errbuf_t	*ebufp)
{
	int		    	*isTAL = 0;
	pin_flist_t		*bb_info = NULL;
	pin_flist_t		*deviceflistp = NULL;
	pin_flist_t		*temp_flistp = NULL;
	pin_flist_t		*ip_flistp = NULL;
	pin_flist_t		*plan_flistp = NULL;
	poid_t			*serv_poid = NULL;
	char			*ip_add = NULL;
	char			*new_ip = NULL;
	char			param[20] = {'\0'};
	int			    flag = 0;
	int		    	is_ip_found = 0;
	int32			rec_id1 = 0;
	int32			rec_id2 = 0;
	pin_cookie_t	cookie1 = NULL;
	pin_cookie_t	cookie2 = NULL;
	char			*mso_ip_add = NULL;
	char			*ip_add_list = NULL;
	char			*user_id = NULL;
	int32			*tech = NULL;
	int32			*prov_flag = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	serv_poid = PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_POID, 0, ebufp);
	bb_info = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	isTAL = PIN_FLIST_FLD_GET(bb_info, MSO_FLD_ISTAL_FLAG, 0, ebufp);
	tech = PIN_FLIST_FLD_GET(bb_info, MSO_FLD_TECHNOLOGY, 0, ebufp);
	prov_flag = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp);
	plan_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PLAN, PIN_ELEMID_ANY, 1, ebufp);
	if(*isTAL == 1)
	{
		//PIN_FLIST_FLD_SET(detail_flistp, MSO_FLD_PASSWORD, "cisco", ebufp);
		strcpy(param,"/device/ip/static");
		/* get the modem device if any */
		fm_mso_search_devices(ctxp,param,serv_poid,&deviceflistp, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_add_user_attribute: Error", ebufp);
			goto cleanup;
		}
		flag = 0;
		cookie1 = NULL;
		rec_id1 = 0;
		while((temp_flistp = PIN_FLIST_ELEM_GET_NEXT(deviceflistp, 
			PIN_FLD_RESULTS, &rec_id1, 1 , &cookie1, ebufp)) != (pin_flist_t *)NULL)
		{
			ip_add = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_DEVICE_ID, 0,ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp)) {
				*error_codep = MSO_BB_INTERNAL_ERROR;
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_add_user_attribute: Error", ebufp);
				goto cleanup;
			}
			if (*prov_flag == FIBER_BB_ADD_IP ){ //In case of MULTIPLE_IP_ADD case
				rec_id2 = 0;
				cookie2 =NULL;
				if(plan_flistp)
				{
					while ((ip_flistp = PIN_FLIST_ELEM_GET_NEXT(plan_flistp,
							PIN_FLD_DEVICES, &rec_id2, 1 , &cookie2, ebufp)) != NULL)
					{
						new_ip = PIN_FLIST_FLD_GET(ip_flistp, PIN_FLD_DEVICE_ID, 1, ebufp);
						is_ip_found = 0; 
						if (*tech == 4 && *prov_flag == FIBER_BB_ADD_IP && (new_ip && ip_add && strcmp(new_ip, ip_add) != 0))
						{
							is_ip_found = 1;
							continue;
						}
						else if(flag == 1)
						{ 
							ip_add_list = (char *)realloc(ip_add_list,(strlen(ip_add_list)+strlen(ip_add))+2);
							strcat(ip_add_list, ",");
							strcat(ip_add_list, ip_add);
						}
						else{
							ip_add_list = (char*)malloc(strlen(ip_add)+1);
							strcpy(ip_add_list, ip_add);
							flag = 1;
						}
					
					}
				}
				// If IP is not equal to the one adding, then do nothing and continue the loop.
				if(is_ip_found == 1)
					continue;
			}
			else{ //In all other cases, intact with old logic
				if(flag == 1)
				{ 
					ip_add_list = (char *)realloc(ip_add_list,(strlen(ip_add_list)+strlen(ip_add))+2);
					strcat(ip_add_list, ",");
					strcat(ip_add_list, ip_add);
				}
				else{
					ip_add_list = (char*)malloc(strlen(ip_add)+1);
					strcpy(ip_add_list, ip_add);
					flag = 1;
				}
			}
			//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ip_add_list: ");
			//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, ip_add_list);
			//flag = 1;
			//ip_add_list = (char *)pin_malloc(1);
			//strcpy(ip_add_list, '\0');
			/*
			ip_flistp = PIN_FLIST_ELEM_GET (temp_flistp, MSO_FLD_IP_DATA, 0, 1,ebufp);
			if(ip_flistp)
			{
				mso_ip_add = PIN_FLIST_FLD_GET(ip_flistp, MSO_FLD_IP_ADDRS, 0,ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_add_list: ");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, mso_ip_add);
				if(flag == 1)
				{ 
					ip_add_list = (char *)realloc(ip_add_list,(strlen(ip_add_list)+strlen(mso_ip_add))+2);
					strcat(ip_add_list, ",");
					strcat(ip_add_list, mso_ip_add);
				}
				else{
					ip_add_list = (char*)malloc(strlen(mso_ip_add)+1);
					strcpy(ip_add_list, mso_ip_add);
				}
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ip_add_list: ");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, ip_add_list);
				flag = 1;
				//ip_pool = PIN_FLIST_FLD_GET(ip_flistp, MSO_FLD_IP_POOL_NAME, 0,ebufp);
				//ip_mask = PIN_FLIST_FLD_GET(ip_flistp, MSO_FLD_IP_MASK_VALUE, 0,ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp)) {
					*error_codep = MSO_BB_INTERNAL_ERROR;
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_add_user_attribute: Error", ebufp);
					goto cleanup;
				}

			}*/
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ip_add_list: ");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, ip_add_list);
		
		
//		if(ip_add_list)
//		{
//			PIN_FLIST_FLD_PUT(user_flistp, MSO_FLD_USER_IP, ip_add_list, ebufp);
//			PIN_FLIST_FLD_SET(user_flistp, MSO_FLD_IP_POOL_NAME, ip_pool, ebufp);
//			acc_id = PIN_POID_GET_ID(ac_pdp);
//			sprintf(acc_id_str,"%d",acc_id);
//			
//			//PIN_FLIST_FLD_SET(user_flistp, MSO_FLD_EXTERNAL_REF_NO, acc_id_str, ebufp);
//			PIN_FLIST_FLD_SET(user_flistp, MSO_FLD_SUBNET_MASK, ip_mask, ebufp);
//			if (PIN_ERRBUF_IS_ERR(ebufp)) {
//				*error_codep = MSO_BB_INTERNAL_ERROR;
//				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_add_user_attribute: Error", ebufp);
//				goto cleanup;
//			}
//		}
		if(ip_add_list){
			//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "T1");
			//PIN_FLIST_FLD_SET(detail_flistp, PIN_FLD_LOGIN, (void *)ip_add_list, ebufp);
			*login = malloc(strlen(ip_add_list)+1);
			sprintf(*login, "%s", ip_add_list);
		}
		else{
			*error_codep = MSO_BB_IP_ADD_ERROR;
			pin_set_err(ebufp, PIN_ERRLOC_CM, 
				PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_FLD_DEVICE_ID, 0, 0);
		}
	}
	else{
		//PIN_FLIST_FLD_SET(detail_flistp, MSO_FLD_PASSWORD, (void *)pass, ebufp);
		//PIN_FLIST_FLD_COPY(so_flistp, PIN_FLD_LOGIN, detail_flistp, PIN_FLD_LOGIN, ebufp);
		user_id = (char *)PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_LOGIN, 0, ebufp);
		*login = malloc(strlen(user_id)+1);
		sprintf(*login, "%s", user_id);
		//*login = strdup(user_id);
	}
cleanup:
	return;
}

static void
fm_mso_get_ip(
	pcm_context_t	*ctxp,
	char			**login,
	pin_flist_t		*i_flistp,
	pin_flist_t		*so_flistp,
	int			    *error_codep,
	pin_errbuf_t	*ebufp)
{
	int			    *isTAL = 0;
	pin_flist_t		*bb_info = NULL;
	pin_flist_t		*deviceflistp = NULL;
	pin_flist_t		*temp_flistp = NULL;
	pin_flist_t		*ip_flistp = NULL;
	poid_t			*serv_poid = NULL;
	char			*ip_add = NULL;
	char			param[20] = {'\0'};
	int			    flag = 0;
	int32			rec_id1 = 0;
	int32			*tech = NULL;
	int32			*prov_flag = NULL;
	pin_cookie_t	cookie1 = NULL;
	char			*mso_ip_add = NULL;
	char			*ip_add_list = NULL;
	char			*user_id = NULL;
	char			*new_ip = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	serv_poid = PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_POID, 0, ebufp);
	bb_info = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	isTAL = PIN_FLIST_FLD_GET(bb_info, MSO_FLD_ISTAL_FLAG, 0, ebufp);
	tech = PIN_FLIST_FLD_GET(bb_info, MSO_FLD_TECHNOLOGY, 0, ebufp);
	prov_flag = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp);
	if (*prov_flag == DOC_BB_IP_CHANGE)
		new_ip = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEVICE_ID, 1, ebufp);
	//if(*isTAL == 1)
	//{
	//PIN_FLIST_FLD_SET(detail_flistp, MSO_FLD_PASSWORD, "cisco", ebufp);
	strcpy(param,"/device/ip/static");
	/* get the modem device if any */
	fm_mso_search_devices(ctxp,param,serv_poid,&deviceflistp, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_add_user_attribute: Error", ebufp);
		goto cleanup;
	}
	flag = 0;
	cookie1 = NULL;
	rec_id1 = 0;
	while((temp_flistp = PIN_FLIST_ELEM_GET_NEXT(deviceflistp, 
		PIN_FLD_RESULTS, &rec_id1, 1 , &cookie1, ebufp)) != (pin_flist_t *)NULL)
	{
		ip_add = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_DEVICE_ID, 0,ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			*error_codep = MSO_BB_INTERNAL_ERROR;
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_add_user_attribute: Error", ebufp);
			goto cleanup;
		}
		// Below condition is to add only new IP and excluding existing ones for IP SWAP for FIBER
		if (*tech == 4 && *prov_flag == DOC_BB_IP_CHANGE && (new_ip && ip_add && strcmp(new_ip, ip_add) != 0))
		{
			continue;
		}
		else if(flag == 1)
		{ 
			ip_add_list = (char *)realloc(ip_add_list,(strlen(ip_add_list)+strlen(ip_add))+2);
			strcat(ip_add_list, ",");
			strcat(ip_add_list, ip_add);
		}
		else{
			ip_add_list = (char*)malloc(strlen(ip_add)+1);
			strcpy(ip_add_list, ip_add);
			if (*tech == 4 && *prov_flag == DOC_BB_IP_CHANGE && (new_ip && ip_add && strcmp(new_ip, ip_add) == 0))
				break;
		}
		flag = 1;
		//ip_add_list = (char *)pin_malloc(1);
		//strcpy(ip_add_list, '\0');
		/*
		ip_flistp = PIN_FLIST_ELEM_GET (temp_flistp, MSO_FLD_IP_DATA, 0, 1,ebufp);
		if(ip_flistp)
		{
			mso_ip_add = PIN_FLIST_FLD_GET(ip_flistp, MSO_FLD_IP_ADDRS, 0,ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_add_list: ");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, mso_ip_add);
			if(flag == 1)
			{ 
				ip_add_list = (char *)realloc(ip_add_list,(strlen(ip_add_list)+strlen(mso_ip_add))+2);
				strcat(ip_add_list, ",");
				strcat(ip_add_list, mso_ip_add);
			}
			else{
				ip_add_list = (char*)malloc(strlen(mso_ip_add)+1);
				strcpy(ip_add_list, mso_ip_add);
			}
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ip_add_list: ");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, ip_add_list);
			flag = 1;
			//ip_pool = PIN_FLIST_FLD_GET(ip_flistp, MSO_FLD_IP_POOL_NAME, 0,ebufp);
			//ip_mask = PIN_FLIST_FLD_GET(ip_flistp, MSO_FLD_IP_MASK_VALUE, 0,ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp)) {
				*error_codep = MSO_BB_INTERNAL_ERROR;
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_add_user_attribute: Error", ebufp);
				goto cleanup;
			}

		} */
	}
	
//		if(ip_add_list)
//		{
//			PIN_FLIST_FLD_PUT(user_flistp, MSO_FLD_USER_IP, ip_add_list, ebufp);
//			PIN_FLIST_FLD_SET(user_flistp, MSO_FLD_IP_POOL_NAME, ip_pool, ebufp);
//			acc_id = PIN_POID_GET_ID(ac_pdp);
//			sprintf(acc_id_str,"%d",acc_id);
//			
//			//PIN_FLIST_FLD_SET(user_flistp, MSO_FLD_EXTERNAL_REF_NO, acc_id_str, ebufp);
//			PIN_FLIST_FLD_SET(user_flistp, MSO_FLD_SUBNET_MASK, ip_mask, ebufp);
//			if (PIN_ERRBUF_IS_ERR(ebufp)) {
//				*error_codep = MSO_BB_INTERNAL_ERROR;
//				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_add_user_attribute: Error", ebufp);
//				goto cleanup;
//			}
//		}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ip_add_list: ");
	if(ip_add_list){
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, ip_add_list);
		//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "T1");
		//PIN_FLIST_FLD_SET(detail_flistp, PIN_FLD_LOGIN, (void *)ip_add_list, ebufp);
		*login = malloc(strlen(ip_add_list)+1);
		sprintf(*login, "%s", ip_add_list);
	}
	else{
		*error_codep = MSO_BB_IP_ADD_ERROR;
		pin_set_err(ebufp, PIN_ERRLOC_CM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_FLD_DEVICE_ID, 0, 0);
	}
	//}
//	else{
//		//PIN_FLIST_FLD_SET(detail_flistp, MSO_FLD_PASSWORD, (void *)pass, ebufp);
//		//PIN_FLIST_FLD_COPY(so_flistp, PIN_FLD_LOGIN, detail_flistp, PIN_FLD_LOGIN, ebufp);
//		user_id = (char *)PIN_FLIST_FLD_GET(so_flistp, PIN_FLD_LOGIN, 0, ebufp);
//		*login = malloc(strlen(user_id)+1);
//		sprintf(*login, "%s", user_id);
//		//*login = strdup(user_id);
//	}
cleanup:
	return;
}

time_t
fm_prov_get_expiry(
	pcm_context_t		*ctxp, 
	poid_t				*mso_plan,
	poid_t				*ac_pdp,
	poid_t				*srv_pdp,
	int			    	srv_typ, 
	time_t				*start,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t			*s_iflistp = NULL;
	pin_flist_t			*s_oflistp = NULL;
	pin_flist_t			*args_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	poid_t				*s_pdp = NULL;
	time_t				*start_t = NULL;
	time_t				*end_t = NULL;
	time_t				end = 0;
	int64				db_no = 0;
	int32				s_flags = SRCH_DISTINCT;
	char				*pre_tmpl = "select X from /purchased_product 1, /mso_purchased_plan 2, /product 3 where 2.F1 = V1 and 1.F2 = 2.F3 and 2.F4 = 3.F5 and 3.F6 like V6 ";
	char				*post_tmpl = "select X from /billinfo where F1 = V1";

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	//Check the Service Type
	if(srv_typ == 0)
	{
//		//For Postpaid get the ACTG_NEXT_T


		db_no = PIN_POID_GET_DB(ac_pdp);
		s_pdp = (poid_t *)PIN_POID_CREATE(db_no, "/search", -1, ebufp);
		s_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(s_iflistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
		PIN_FLIST_FLD_SET(s_iflistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
		PIN_FLIST_FLD_SET(s_iflistp, PIN_FLD_TEMPLATE, (void *)pre_tmpl, ebufp);
		//-----Add 1st Arg-----
		args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, (void *)mso_plan, ebufp);
		//----Add 2nd Arg----
		args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 3, ebufp);
		tmp_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, (void *)NULL, ebufp);
		//----Add 3rd Arg-----
		args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, (void *)NULL, ebufp);
		//----Add 4th Arg-----
		args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 4, ebufp);
		tmp_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_PRODUCT_OBJ, (void *)NULL, ebufp);
		//----Add 5th Arg-----
		args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 5, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, (void *)NULL, ebufp);
		//----Add 6th Arg-----
		args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 6, ebufp);
		tmp_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_USAGE_MAP, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_EVENT_TYPE, "%mso_grant", ebufp);
		//-----Add Result Arr
		args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_RESULTS, 0, ebufp);
		if(PIN_ERRBUF_IS_ERR(ebufp)){
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_prov_get_expiry: Error", ebufp);
			goto cleanup;
		}
		if(PIN_ERRBUF_IS_ERR(ebufp)){
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_prov_get_expiry: Error", ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_prov_get_expiry: Search Input Flist ", s_iflistp);
		//Call the PCM_OP_SEARCH
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_iflistp, &s_oflistp, ebufp);
		if(PIN_ERRBUF_IS_ERR(ebufp)){
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_prov_get_expiry: Error", ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_prov_get_expiry: Search Output Flist ", s_oflistp);
		args_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_RESULTS, 0, 1, ebufp);
		if(args_flistp)
		{
			tmp_flistp = PIN_FLIST_ELEM_GET(args_flistp, PIN_FLD_CYCLE_FEES, 1, 1, ebufp);
			if(tmp_flistp)
			{
				end_t = (time_t *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CHARGED_TO_T, 0, ebufp);
				start_t = (time_t *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CHARGED_FROM_T, 0, ebufp);
			}
		}
	}
	else
	{
		//For Prepaid get the purchased product end_t
		db_no = PIN_POID_GET_DB(ac_pdp);
		s_pdp = (poid_t *)PIN_POID_CREATE(db_no, "/search", -1, ebufp);
		s_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(s_iflistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
		PIN_FLIST_FLD_SET(s_iflistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
		PIN_FLIST_FLD_SET(s_iflistp, PIN_FLD_TEMPLATE, (void *)pre_tmpl, ebufp);
		//-----Add 1st Arg-----
		args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, (void *)mso_plan, ebufp);
		//----Add 2nd Arg----
		args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 3, ebufp);
		tmp_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, (void *)NULL, ebufp);
		//----Add 3rd Arg-----
		args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, (void *)NULL, ebufp);
		//----Add 4th Arg-----
		args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 4, ebufp);
		tmp_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_PRODUCT_OBJ, (void *)NULL, ebufp);
		//----Add 5th Arg-----
		args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 5, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, (void *)NULL, ebufp);
		//----Add 6th Arg-----
		args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 6, ebufp);
		tmp_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_USAGE_MAP, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_EVENT_TYPE, "%mso_grant", ebufp);
		//-----Add Result Arr
		args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_RESULTS, 0, ebufp);
		if(PIN_ERRBUF_IS_ERR(ebufp)){
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_prov_get_expiry: Error", ebufp);
			goto cleanup;
		}
		if(PIN_ERRBUF_IS_ERR(ebufp)){
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_prov_get_expiry: Error", ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_prov_get_expiry: Search Input Flist ", s_iflistp);
		//Call the PCM_OP_SEARCH
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_iflistp, &s_oflistp, ebufp);
		if(PIN_ERRBUF_IS_ERR(ebufp)){
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_prov_get_expiry: Error", ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_prov_get_expiry: Search Output Flist ", s_oflistp);
		args_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_RESULTS, 0, 1, ebufp);
		if(args_flistp)
		{
			tmp_flistp = PIN_FLIST_ELEM_GET(args_flistp, PIN_FLD_CYCLE_FEES, 1, 1, ebufp);
			if(tmp_flistp)
			{
				end_t = (time_t *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CHARGED_TO_T, 0, ebufp);
				start_t = (time_t *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CHARGED_FROM_T, 0, ebufp);
			}
		}
	}
	if(PIN_ERRBUF_IS_ERR(ebufp)){
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_prov_get_expiry: Error", ebufp);
		goto cleanup;
	}
	if(end_t){
		end = *end_t;
	}
	if(start_t){
		*start = *start_t;
	}
cleanup:
	PIN_FLIST_DESTROY_EX(&s_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&s_oflistp, NULL);
	return end;
}

static time_t
fm_prov_get_expiry_from_calc_only_out(
	pcm_context_t		*ctxp, 
	pin_flist_t			*in_flist, 
	int			      	srv_typ,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t			*sub_bal_imp = NULL;
	poid_t				*s_pdp = NULL;

	time_t				*end_t = NULL;
	time_t				end = 0;
	
	int32				res_id = 0;

	void				*vp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	//Check the Service Type
	if(srv_typ == 0)
	{
		sub_bal_imp = PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_SUB_BAL_IMPACTS, 0, 1, ebufp);
		if (sub_bal_imp)
		{
			vp = PIN_FLIST_FLD_GET(sub_bal_imp, PIN_FLD_RESOURCE_ID, 1, ebufp);
			if (vp)
			{
				res_id =  *(int32*)vp;
			}
			if (res_id == MSO_FUP_TRACK)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "FUP");
			}
			else if (res_id == MSO_FREE_MB)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "FREE MB");
			}
			else if(res_id == MSO_TRCK_USG)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "USAGE TRACK");
			}

			vp = PIN_FLIST_ELEM_GET(sub_bal_imp, PIN_FLD_SUB_BALANCES, PIN_ELEMID_ANY, 1, ebufp);
			if (vp)
			{
				end_t = PIN_FLIST_FLD_GET((pin_flist_t*)vp, PIN_FLD_VALID_TO, 1, ebufp);
			}

		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Missing PIN_FLD_SUB_BAL_IMPACTS");
		}
	}
	else
	{
		sub_bal_imp = PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_SUB_BAL_IMPACTS, 0, 1, ebufp);
		if (sub_bal_imp)
		{
			vp = PIN_FLIST_FLD_GET(sub_bal_imp, PIN_FLD_RESOURCE_ID, 1, ebufp);
			if (vp)
			{
				res_id =  *(int32*)vp;
			}
			if (res_id == MSO_FUP_TRACK)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "FUP");
			}
			else if (res_id == MSO_FREE_MB)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "FREE MB");
			}
			else if(res_id == MSO_TRCK_USG)
                        {
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "USAGE TRACK");
                        }			

			vp = PIN_FLIST_ELEM_GET(sub_bal_imp, PIN_FLD_SUB_BALANCES, PIN_ELEMID_ANY, 1, ebufp);
			if (vp)
			{
				end_t = PIN_FLIST_FLD_GET((pin_flist_t*)vp, PIN_FLD_VALID_TO, 1, ebufp);
			}

		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Missing PIN_FLD_SUB_BAL_IMPACTS");
		}
	}
	if(PIN_ERRBUF_IS_ERR(ebufp)){
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_prov_get_expiry: Error", ebufp);
		goto cleanup;
	}


	if(end_t){
		end = *end_t;
	}
cleanup:
	return end;
}


static void
fm_mso_prov_bb_config_cache_update(
	pcm_context_t *ctxp,
	pin_errbuf_t  *ebufp)
{
	poid_t                  *s_pdp = NULL;
	poid_t					*pdp = NULL;
	pin_flist_t             *s_flistp = NULL;
	pin_flist_t             *r_flistp = NULL;
	pin_flist_t             *res_flistp = NULL;
	pin_flist_t             *arg_flistp = NULL;
	int32                   err = PIN_ERR_NONE;
	int32                   cnt;
	int64					database;
	cm_cache_key_iddbstr_t  kids;
	int32                   s_flags = 256;
	int						no_of_buckets = 1;
	time_t			        now = 0;

		
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
			"select X from /mso_config_bb_pt where F1.db = V1 ", ebufp);
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
                        "fm_mso_prov_bb_config_cache_update: error loading /mso_config_bb_pt object",
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



	if (!mso_prov_bb_config_ptr) 
	{
		goto cleanup;
	}

	kids.id  = 0; 
	kids.db  = 0;
	kids.str = "/mso_config_bb_pt";
	cm_cache_update_entry(mso_prov_bb_config_ptr, &kids, r_flistp, &err);

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_prov_bb_config_cache_update error", ebufp);
	}
	else 
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_prov_bb_config_cache_update done successfully");
	}
	
 cleanup:
        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&res_flistp, NULL);
		PIN_POID_DESTROY(pdp, NULL);
}

static void
fm_mso_prov_cmts_config_cache_update(
	pcm_context_t *ctxp,
	pin_errbuf_t  *ebufp)
{
	poid_t                  *s_pdp = NULL;
	poid_t					*pdp = NULL;
	pin_flist_t             *s_flistp = NULL;
	pin_flist_t             *r_flistp = NULL;
	pin_flist_t             *res_flistp = NULL;
	pin_flist_t             *arg_flistp = NULL;
	int32                   err = PIN_ERR_NONE;
	int32                   cnt;
	int64					database;
	cm_cache_key_iddbstr_t  kids;
	int32                   s_flags = 256;
	int						no_of_buckets = 1;
	time_t			        now = 0;

		
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
			"select X from /mso_cfg_cmts_master where F1.db = V1 ", ebufp);
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
                        "fm_mso_prov_cmts_config_cache_update: error loading /mso_cfg_cmts_master object",
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



	if (!mso_prov_bb_cmts_ptr) 
	{
		goto cleanup;
	}

	kids.id  = 0; 
	kids.db  = 0;
	kids.str = "/mso_cfg_cmts_master";
	cm_cache_update_entry(mso_prov_bb_cmts_ptr, &kids, r_flistp, &err);

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_prov_cmts_config_cache_update error", ebufp);
	}
	else 
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_prov_cmts_config_cache_update done successfully");
	}
	
 cleanup:
        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&res_flistp, NULL);
		PIN_POID_DESTROY(pdp, NULL);
}

static void
get_cancelled_plan(
	pcm_context_t		*ctxp,
	poid_t				*ac_pdp,
	char				*city,
	char				*svc_code,
	time_t				**end_t,
	poid_t				**prod_pdp,
	int				    *error_codep,
	int				    *tech,
	int                 *action_flag,
	int32			    status,
	pin_errbuf_t		*ebufp)
{
	
	pin_flist_t		*grp_detail_flistp = NULL;
	pin_flist_t		*plan_iflistp = NULL;
	pin_flist_t		*plan_oflistp = NULL;
	pin_flist_t		*search_plan_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_flist_t		*res_iflistp = NULL;
	pin_flist_t		*search_plan_o_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*plan_det_flistp = NULL;
	pin_flist_t		*cfg_cr_flistp = NULL;
	poid_t			*plan_pdp = NULL;
	poid_t			*s_pdp = NULL;
	poid_t			*pdp = NULL;
	int32           s_flags = 0;
	int32           *bill_when  = NULL;
	int64           database = 0;
	int				*tec = NULL;
	char			*name = NULL;
	char			*plan_name = NULL;
	char			*template = "select X from /mso_purchased_plan where F1 = V1 and F2 = V2 and F3 = V3 order by mod_t desc ";
	char			*template1 = "select X from /mso_purchased_plan where F1 = V1 and (F2 = V2 or F3 = V3) and F4 = V4 order by mod_t desc ";
	int32			cancel_prov_status = 0;
	char			quota_code[100]="\0";
	int32			terminated_status = MSO_PROV_STATE_TERMINATED;

	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*cache_flistp = NULL;
	cm_cache_key_iddbstr_t		kids;
	pin_cookie_t	cookie = NULL;
    int32			rec_id = 0;
	int32			err = PIN_ERR_NONE;
	char			*svc_code_tmp = NULL;	
	char 			*prov_tag = NULL;
	
	/*
	0 PIN_FLD_POID                      POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS                      INT [0] 768
	0 PIN_FLD_TEMPLATE                   STR [0] "select X from /mso_purchased_plan where F1 = V1 and F2 = V2 order by mod_t desc "
	0 PIN_FLD_RESULTS                  ARRAY [0] allocated 2, used 2
	1     PIN_FLD_PLAN_OBJ              POID [0] NULL
	1     PIN_FLD_PURCHASE_END_T      TSTAMP [0] NULL
	0 PIN_FLD_ARGS                     ARRAY [1] allocated 1, used 1
	1     PIN_FLD_ACCOUNT_OBJ           POID [0] 0.0.0.1 /account 266861 0
	*/
	
	search_plan_flistp = PIN_FLIST_CREATE(ebufp);
	cancel_prov_status = status;
	/***********************************************************
	 * assume db matches userid found in pin.conf
	 ***********************************************************/
	pdp = PCM_GET_USERID(ctxp);
	database = PIN_POID_GET_DB(pdp);

	s_pdp = PIN_POID_CREATE(database, "/search", -1, ebufp);
	
	PIN_FLIST_FLD_PUT(search_plan_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_FLD_SET(search_plan_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
	/* adding fix for change plan after expiry*/
	if (status == MSO_PROV_STATE_DEACTIVE)
	{
		PIN_FLIST_FLD_SET(search_plan_flistp, PIN_FLD_TEMPLATE, template1 , ebufp);
		arg_flistp = PIN_FLIST_ELEM_ADD(search_plan_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACCOUNT_OBJ, ac_pdp, ebufp);
		arg_flistp = PIN_FLIST_ELEM_ADD(search_plan_flistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_STATUS, &cancel_prov_status, ebufp);
		arg_flistp = PIN_FLIST_ELEM_ADD(search_plan_flistp, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_STATUS, &terminated_status, ebufp);
		arg_flistp = PIN_FLIST_ELEM_ADD(search_plan_flistp, PIN_FLD_ARGS, 4, ebufp);
		PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_DESCR, "base plan", ebufp);

	}
	else
	{
		PIN_FLIST_FLD_SET(search_plan_flistp, PIN_FLD_TEMPLATE, template , ebufp);
		arg_flistp = PIN_FLIST_ELEM_ADD(search_plan_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACCOUNT_OBJ, ac_pdp, ebufp);
		arg_flistp = PIN_FLIST_ELEM_ADD(search_plan_flistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_STATUS, &cancel_prov_status, ebufp);
		arg_flistp = PIN_FLIST_ELEM_ADD(search_plan_flistp, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_DESCR, "base plan", ebufp);

	}
	

	res_iflistp = PIN_FLIST_ELEM_ADD( search_plan_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(res_iflistp, PIN_FLD_PLAN_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_iflistp, PIN_FLD_BILL_WHEN, NULL, ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD( res_iflistp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, ebufp);//MSO_FLD_PURCHASED_PRODUCT_OBJ
	PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_PROVISIONING_TAG, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Get Purchased plan input flist", search_plan_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_plan_flistp, &search_plan_o_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_plan_flistp, NULL);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Get Purchased plan output flist", search_plan_o_flistp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_INTERNAL_ERROR;
             PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "error getting last plan ", ebufp);
             goto cleanup;
	}
	
	result_flistp = PIN_FLIST_ELEM_GET(search_plan_o_flistp,PIN_FLD_RESULTS,0,1,ebufp);
	if(result_flistp)
	{
		//Pawan:25-02-2015: Commented below since plan name is not required. 
		/*plan_pdp = PIN_FLIST_FLD_GET(result_flistp,PIN_FLD_PLAN_OBJ,0,ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			*error_codep = MSO_BB_INTERNAL_ERROR;
		     PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"error getting last plan ", ebufp);
		     goto cleanup;
		}
		plan_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(plan_iflistp, PIN_FLD_POID, plan_pdp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read plan input flist", plan_iflistp);
		PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, plan_iflistp, &plan_oflistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read plan output flist", plan_oflistp);
		
		*plan_name = (char *)PIN_FLIST_FLD_TAKE(plan_oflistp, PIN_FLD_NAME, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			*error_codep = MSO_BB_INTERNAL_ERROR;
		     PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"error getting last plan ", ebufp);
		     goto cleanup;
		}

		PIN_FLIST_DESTROY_EX(&plan_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&plan_oflistp, NULL); */
		plan_pdp = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
		bill_when = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);
		if(plan_pdp){
			fm_mso_utils_read_any_object(ctxp, plan_pdp, &plan_det_flistp, ebufp);
			if(plan_det_flistp){
				plan_name = PIN_FLIST_FLD_GET(plan_det_flistp, PIN_FLD_NAME, 1, ebufp);
			}
		}
		
		plan_iflistp = PIN_FLIST_CREATE(ebufp);
		res_iflistp = PIN_FLIST_ELEM_GET(result_flistp,PIN_FLD_PRODUCTS,0,1,ebufp);
		if(res_iflistp)
		{
			PIN_FLIST_FLD_COPY(res_iflistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, plan_iflistp, PIN_FLD_POID,ebufp );
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read Pur Prod input flist", plan_iflistp);
			PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, plan_iflistp, &plan_oflistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read Pur Prod flist", plan_oflistp);
			PIN_FLIST_DESTROY_EX(&plan_iflistp, NULL);
			*prod_pdp = PIN_FLIST_FLD_TAKE(plan_oflistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
			*end_t = PIN_FLIST_FLD_TAKE(plan_oflistp, PIN_FLD_PURCHASE_END_T, 0, ebufp);			
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"OLD PLAN not found" );
			*error_codep = MSO_BB_OLD_PLAN_ERROR; 
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_BAD_VALUE, PIN_FLD_PLAN_OBJ, 0, 0);
			goto cleanup;		
		} 
		/* Pawan:25-02-2015: Added from here to fetch the service code of old plan */
		while((res_iflistp = PIN_FLIST_ELEM_GET_NEXT(result_flistp, 
		PIN_FLD_PRODUCTS, &rec_id, 1 , &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			prov_tag = PIN_FLIST_FLD_GET(res_iflistp, PIN_FLD_PROVISIONING_TAG, 0, ebufp);
			if(prov_tag && strlen(prov_tag)>0) {
				break;
			}
		}
		if(prov_tag && strlen(prov_tag)>0)
		{
			kids.id = 0;
			kids.db = 0;
			kids.str = "/mso_config_bb_pt";
			cache_flistp = cm_cache_find_entry(mso_prov_bb_config_ptr,&kids, &err);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"cm_cache_find_entry found flist", cache_flistp);
			switch(err)
			{
				case PIN_ERR_NONE:
					break;
				default:
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
						"mso_prov_bb_config_object from_cache: error "
						"accessing data dictionary cache.");
					pin_set_err(ebufp, PIN_ERRLOC_CM,
							PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_FLD_CACHE_NAME, 0, 0);
					goto cleanup;
			}
		
			PIN_ERR_LOG_MSG(3, "Fields to compare");
			PIN_ERR_LOG_MSG(3, prov_tag);
			cookie = NULL;
			while ( (tmp_flistp = PIN_FLIST_ELEM_GET_NEXT (cache_flistp,
				PIN_FLD_RESULTS, &rec_id, 1,&cookie, ebufp))!= (pin_flist_t *)NULL )
			{
				if (strcmp((char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_PROVISIONING_TAG, 0, ebufp ),prov_tag) == 0 )
				{
					tec = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_BB_TECHNOLOGY, 0, ebufp);
					if(*tec == *tech || *action_flag == DOC_BB_PC_QUOTA_NQUOTA || *action_flag == DOC_BB_PC_NQUOTA_QUOTA
                                                || *action_flag == DOC_BB_PC_QUOTA_QUOTA || *action_flag == DOC_BB_PC_NQUOTA_NQUOTA)
					{
						fm_mso_get_service_quota_codes(ctxp, plan_name, bill_when, city, &cfg_cr_flistp, ebufp);
						if (PIN_ERRBUF_IS_ERR(ebufp)) {
							*error_codep = MSO_BB_INTERNAL_ERROR;
							PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "service_quota_code fetching details error: Error", ebufp);
							goto cleanup;
						}
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "CFG_CR_FLISTP", cfg_cr_flistp);
						if(cfg_cr_flistp)
							svc_code_tmp = PIN_FLIST_FLD_GET(cfg_cr_flistp, PIN_FLD_SERVICE_CODE, 0, ebufp );
						if(svc_code_tmp != NULL && strcmp(svc_code_tmp, "") != 0) {
							strcpy(svc_code,svc_code_tmp);
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Service Code is:");
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,svc_code);
						}
						else{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"SERVICE_CODE is not found for cancelled plan" );
							*error_codep = MSO_BB_INTERNAL_ERROR; 
							pin_set_err(ebufp, PIN_ERRLOC_FM,
								PIN_ERRCLASS_SYSTEM_DETERMINATE,
								PIN_ERR_NOT_FOUND, PIN_FLD_SERVICE_CODE, 0, 0);
							goto cleanup;
						}
					}
				}

			}
		}
		/* Pawan:25-02-2015: Added till here */
		
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"OLD PLAN not found" );
		*error_codep = MSO_BB_OLD_PLAN_ERROR; 
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_PLAN_OBJ, 0, 0);
		goto cleanup;
	}

	
	
cleanup:
	PIN_FLIST_DESTROY_EX(&search_plan_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_plan_o_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&plan_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&plan_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&cfg_cr_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&plan_det_flistp, NULL);
	if(cache_flistp != NULL)PIN_FLIST_DESTROY_EX(&cache_flistp, NULL);
	return;
	
}


static void
prepare_bb_add_mb_flist(
        pcm_context_t                   *ctxp,
        pin_flist_t                     **i_flistpp,
        char                            **act,
        int                             *error_codep,
        pin_errbuf_t                    *ebufp)
{
        pin_flist_t                     *si_flistp = NULL;
        pin_flist_t                     *so_flistp = NULL;
        pin_flist_t                     *tmp_flistp = NULL;
        pin_flist_t                     *sub_flistp = NULL;
        pin_flist_t                     *task_flistp = NULL;
        pin_flist_t                     *cred_flistp = NULL;
        pin_flist_t                     *subscriber = NULL;


        pin_flist_t                     *mso_avp = NULL;
        pin_flist_t                     *bb_info = NULL;

        poid_t                          *ac_pdp = NULL;
        void                            *vp = NULL;
        char                            *population_id = "0001"; //this is the only expected value in downstream system
        char                            *ne = NULL;
        char                            *user_id = NULL;
        char                            *passwd = NULL;
        char                            *user_pass = "Username/Password";
        char                            *ip_add = "IPAddress";
        char                            *mac_add = "MACAddress";
        char                            *serv_stat = "True";
        char                            *vc_id = NULL;
        char                            *device_type = NULL;
        char                            *acc_no = NULL;
        char                            *node_name = NULL;
        char                            *serv_city = NULL;

        int32                           *state_id = NULL;
        int                             *tech = NULL;
        int                             *isTAL = 0;
        int                             *is1Click = 0;
        int                             *ind = 0;
        int                             *idle_timeout = 0;
    	int                             *session_timeout = 0;




        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;

        PIN_ERRBUF_CLEAR(ebufp);


        /***********************************************************
        * Log input flist
        ***********************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_bb_add_mb_flist: input flist", *i_flistpp);

        /*
        * Validate input received
        */

        /*
        * Enrich with additional service details
        */
        si_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_bb_add_mb_flist PCM_OP_READ_OBJ input flist", si_flistp);
        PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                *error_codep = MSO_BB_INTERNAL_ERROR;
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "prepare_bb_add_mb_flist: Error",ebufp);
                goto cleanup;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_bb_add_mb_flist PCM_OP_READ_OBJ output flist", so_flistp);
        tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 1, ebufp);
        if(tmp_flistp)
                tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp,
                        MSO_FLD_TECHNOLOGY, 0, ebufp);
	 //ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);
        //PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                *error_codep = MSO_BB_TECH_MISSING;
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "prepare_bb_add_mb_flist: Error",ebufp);
                goto cleanup;
        }

        if(*tech == 1 || *tech == 2 || *tech == 5 || *tech == 3)
        {
                *act = (char *) malloc(strlen("ADD_DATA_DOCSIS")+1);
                strcpy(*act,"ADD_DATA_DOCSIS");
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		if(*tech == 1 || *tech == 2)
                	prepare_docsis_bcc_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
                prepare_switch_service_flist(ctxp, i_flistpp, so_flistp,*act, error_codep, ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "prepare_bb_add_mb_flist: Error",ebufp);
                        goto cleanup;
                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_bb_add_mb_flist: input flist", *i_flistpp);
        }
        /*else
        {
                *act = (char *) malloc(strlen("EXTEND_VALIDITY_ETHERNET")+1);
                strcpy(*act,"EXTEND_VALIDITY_ETHERNET");
                ethernet_new_plan(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "prepare_bb_add_mb_flist: Error",ebufp);
                        goto cleanup;
                }
        }*/
	else
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                *error_codep = MSO_BB_INTERNAL_ERROR;
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "prepare_bb_add_mb_flist: Error",ebufp);
                goto cleanup;
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_bb_add_mb_flist enriched input flist", *i_flistpp);
cleanup:
        PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
        return;
}



static void
prepare_bb_plan_validity_ext_flist(
        pcm_context_t                   *ctxp,
        pin_flist_t                     **i_flistpp,
        char                            **act,
        int                             *error_codep,
        pin_errbuf_t                    *ebufp)
{
        pin_flist_t                     *si_flistp = NULL;
        pin_flist_t                     *so_flistp = NULL;
        pin_flist_t                     *tmp_flistp = NULL;
        pin_flist_t                     *sub_flistp = NULL;
        pin_flist_t                     *task_flistp = NULL;
        pin_flist_t                     *cred_flistp = NULL;
        pin_flist_t                     *subscriber = NULL;


        pin_flist_t                     *mso_avp = NULL;
        pin_flist_t                     *bb_info = NULL;

        poid_t                          *ac_pdp = NULL;
        void                            *vp = NULL;
        char                            *population_id = "0001"; //this is the only expected value in downstream system
        char                            *ne = NULL;
        char                            *user_id = NULL;
        char                            *passwd = NULL;
        char                            *user_pass = "Username/Password";
        char                            *ip_add = "IPAddress";
        char                            *mac_add = "MACAddress";
        char                            *serv_stat = "True";
        char                            *vc_id = NULL;
        char                            *device_type = NULL;
        char                            *acc_no = NULL;
        char                            *node_name = NULL;
        char                            *serv_city = NULL;

        int32                           *state_id = NULL;
        int                             *tech = NULL;
        int                             *isTAL = 0;
        int                             *is1Click = 0;
        int                             *ind = 0;
        int                             *idle_timeout = 0;
	    int                             *session_timeout = 0;


        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;

        PIN_ERRBUF_CLEAR(ebufp);


        /***********************************************************
        * Log input flist
        ***********************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_bb_plan_validity_ext_flist: input flist", *i_flistpp);

        /*
        * Validate input received
        */

        /*
        * Enrich with additional service details
        */
        si_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_bb_plan_validity_ext_flist PCM_OP_READ_OBJ input flist", si_flistp);
        PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                *error_codep = MSO_BB_INTERNAL_ERROR;
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "prepare_bb_plan_validity_ext_flist: Error",ebufp);
                goto cleanup;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_bb_plan_validity_ext_flist PCM_OP_READ_OBJ output flist", so_flistp);
        tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 1, ebufp);
        if(tmp_flistp)
                tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp,
                        MSO_FLD_TECHNOLOGY, 0, ebufp);
	 //ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);
        //PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                *error_codep = MSO_BB_TECH_MISSING;
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "prepare_bb_plan_validity_ext_flist: Error",ebufp);
                goto cleanup;
        }

        if(*tech == 1 || *tech == 2 || *tech == 5 || *tech == 3)
        {
                *act = (char *) malloc(strlen("EXTEND_VALIDITY_DOCSIS")+1);
                strcpy(*act,"EXTEND_VALIDITY_DOCSIS");
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		if(*tech == 1 || *tech == 2)
                	prepare_docsis_bcc_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
                prepare_switch_service_flist(ctxp, i_flistpp, so_flistp, *act,error_codep, ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "prepare_bb_plan_validity_ext_flist: Error",ebufp);
                        goto cleanup;
                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_bb_plan_validity_ext_flist: input flist", *i_flistpp);
        }
        /*else
        {
                *act = (char *) malloc(strlen("EXTEND_VALIDITY_ETHERNET")+1);
                strcpy(*act,"EXTEND_VALIDITY_ETHERNET");
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                ethernet_extend_validity (ctxp, i_flistpp, so_flistp, error_codep, ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "prepare_bb_plan_validity_ext_flist: Error",ebufp);
                        goto cleanup;
                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_bb_plan_validity_ext_flist: input flist", *i_flistpp);
        }*/
	else
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                *error_codep = MSO_BB_INTERNAL_ERROR;
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "prepare_bb_plan_validity_ext_flist: Error",ebufp);
                goto cleanup;
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_bb_plan_validity_ext_flist enriched input flist", *i_flistpp);
cleanup:
        PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
        return;
}

static void
prepare_bb_cam_in_flist(
        pcm_context_t   *ctxp,
        pin_flist_t     **i_flistpp,
        char            **act,
        int             *error_codep,
        pin_errbuf_t    *ebufp)
{
        pin_flist_t             *si_flistp = NULL;
        pin_flist_t             *so_flistp = NULL;
        pin_flist_t             *tmp_flistp = NULL;
        pin_flist_t             *sub_flistp = NULL;
        pin_flist_t             *task_flistp = NULL;
        pin_flist_t             *cred_flistp = NULL;
        pin_flist_t             *subscriber = NULL;


        pin_flist_t             *mso_avp = NULL;
        pin_flist_t             *bb_info = NULL;

        poid_t                  *ac_pdp = NULL;
        void                    *vp = NULL;
        char                    *population_id = "0001"; //this is the only expected value in downstream system
        char                    *ne = NULL;

        char                    *serv_stat = "True";
        char                    *vc_id = NULL;
        char                    *device_type = NULL;
        char                    *acc_no = NULL;
        char                    *node_name = NULL;
        char                    *serv_city = NULL;

        int32                   *state_id = NULL;
        int                     *tech = NULL;




        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;

        PIN_ERRBUF_CLEAR(ebufp);


        /***********************************************************
        * Log input flist
        ***********************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_bb_cam_in_flist input flist", *i_flistpp);

        /*
        * Validate input received
        */

        /*
        * Enrich with additional service details
        */
        si_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_bb_cam_in_flist PCM_OP_READ_OBJ input flist", si_flistp);
        PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_bb_cam_in_flist PCM_OP_READ_OBJ output flist", so_flistp);
        tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
        tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp);
        //ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);
        //PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

        if(*tech == 1 || *tech == 2)
        {
                *act = (char *) malloc(strlen("CHANGE_AUTH_TO_CLICK_DOCSIS")+1);
                strcpy(*act,"CHANGE_AUTH_TO_CLICK_DOCSIS");
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                prepare_docsis_subscriberavps2_flist(ctxp, i_flistpp, so_flistp, *act ,error_codep, ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "prepare_bb_cam_in_flist:Error",ebufp);
                        goto cleanup;
                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"input flist", *i_flistpp);
        }
        else
        {
                //Not required for Ethernet
                //prepare_mqnsure_activation_flist(ctxp, i_flistpp, so_flistp, ebufp);
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
                *error_codep = MSO_NO_PROV_ACTION;
                goto cleanup;
        }
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                *error_codep = MSO_BB_INTERNAL_ERROR;
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "prepare_bb_cam_in_flist: Error",ebufp);
                goto cleanup;
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_bb_cam_in_flist enriched input flist", *i_flistpp);

        cleanup:
        PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
        return;
}

static void
prepare_bb_cam_pn_flist(
        pcm_context_t   *ctxp,
        pin_flist_t     **i_flistpp,
        char            **act,
        int             *error_codep,
        pin_errbuf_t    *ebufp)
{
        pin_flist_t             *si_flistp = NULL;
        pin_flist_t             *so_flistp = NULL;
        pin_flist_t             *tmp_flistp = NULL;
        pin_flist_t             *sub_flistp = NULL;
        pin_flist_t             *task_flistp = NULL;
        pin_flist_t             *cred_flistp = NULL;
        pin_flist_t             *subscriber = NULL;


        pin_flist_t             *mso_avp = NULL;
        pin_flist_t             *bb_info = NULL;

        poid_t                  *ac_pdp = NULL;
        void                    *vp = NULL;
        char                    *population_id = "0001"; //this is the only expected value in downstream system
        char                    *ne = NULL;

        char                    *serv_stat = "True";
        char                    *vc_id = NULL;
        char                    *device_type = NULL;
        char                    *acc_no = NULL;
        char                    *node_name = NULL;
        char                    *serv_city = NULL;

        int32                   *state_id = NULL;
        int                     *tech = NULL;




        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;

        PIN_ERRBUF_CLEAR(ebufp);


        /***********************************************************
        * Log input flist
        ***********************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_bb_cam_pn_flist input flist", *i_flistpp);

        /*
        * Validate input received
        */

        /*
        * Enrich with additional service details
        */
        si_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_bb_cam_pn_flist PCM_OP_READ_OBJ input flist", si_flistp);
        PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_bb_cam_pn_flist PCM_OP_READ_OBJ output flist", so_flistp);
        tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
        tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp);
        //ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);
        //PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

        if(*tech == 1 || *tech == 2)
        {
                *act = (char *) malloc(strlen("CHANGE_AUTH_TO_CLICK_DOCSIS")+1);
                strcpy(*act,"CHANGE_AUTH_TO_CLICK_DOCSIS");
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                prepare_docsis_subscriberavps2_flist(ctxp, i_flistpp, so_flistp, *act ,error_codep, ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "prepare_bb_cam_pn_flist:Error",ebufp);
                        goto cleanup;
                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"input flist", *i_flistpp);
        }
        else
        {
                //Not required for Ethernet
                //prepare_mqnsure_activation_flist(ctxp, i_flistpp, so_flistp, ebufp);
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
                *error_codep = MSO_NO_PROV_ACTION;
                goto cleanup;
        }
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                *error_codep = MSO_BB_INTERNAL_ERROR;
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "prepare_bb_cam_pn_flist: Error",ebufp);
                goto cleanup;
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_bb_cam_pn_flist enriched input flist", *i_flistpp);

        cleanup:
        PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
        return;
}

static void
prepare_bb_cam_ni_flist(
        pcm_context_t   *ctxp,
        pin_flist_t     **i_flistpp,
        char            **act,
        int             *error_codep,
        pin_errbuf_t    *ebufp)
{
        pin_flist_t             *si_flistp = NULL;
        pin_flist_t             *so_flistp = NULL;
        pin_flist_t             *tmp_flistp = NULL;
        pin_flist_t             *sub_flistp = NULL;
        pin_flist_t             *task_flistp = NULL;
        pin_flist_t             *cred_flistp = NULL;
        pin_flist_t             *subscriber = NULL;


        pin_flist_t             *mso_avp = NULL;
        pin_flist_t             *bb_info = NULL;

        poid_t                  *ac_pdp = NULL;
        void                    *vp = NULL;
        char                    *population_id = "0001"; //this is the only expected value in downstream system
        char                    *ne = NULL;

        char                    *serv_stat = "True";
        char                    *vc_id = NULL;
        char                    *device_type = NULL;
        char                    *acc_no = NULL;
        char                    *node_name = NULL;
        char                    *serv_city = NULL;

        int32                   *state_id = NULL;
        int                     *tech = NULL;




        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;

        PIN_ERRBUF_CLEAR(ebufp);


        /***********************************************************
        * Log input flist
        ***********************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_bb_cam_ni_flist input flist", *i_flistpp);

        /*
        * Validate input received
        */

        /*
        * Enrich with additional service details
        */
        si_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_bb_cam_ni_flist PCM_OP_READ_OBJ input flist", si_flistp);
        PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_bb_cam_ni_flist PCM_OP_READ_OBJ output flist", so_flistp);
        tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
        tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp);
        //ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);
        //PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

        if(*tech == 1 || *tech == 2)
        {
                *act = (char *) malloc(strlen("CHANGE_AUTH_TO_CLICK_DOCSIS")+1);
                strcpy(*act,"CHANGE_AUTH_TO_CLICK_DOCSIS");
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                prepare_docsis_subscriberavps2_flist(ctxp, i_flistpp, so_flistp, *act ,error_codep, ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "prepare_bb_cam_ni_flist:Error",ebufp);
                        goto cleanup;
                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"input flist", *i_flistpp);
        }
        else
        {
                //Not required for Ethernet
                //prepare_mqnsure_activation_flist(ctxp, i_flistpp, so_flistp, ebufp);
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
                *error_codep = MSO_NO_PROV_ACTION;
                goto cleanup;
        }
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                *error_codep = MSO_BB_INTERNAL_ERROR;
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "prepare_bb_cam_ni_flist: Error",ebufp);
                goto cleanup;
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_bb_cam_ni_flist enriched input flist", *i_flistpp);

        cleanup:
        PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
        return;
}

static void
prepare_bb_cam_np_flist(
        pcm_context_t   *ctxp,
        pin_flist_t     **i_flistpp,
        char            **act,
        int             *error_codep,
        pin_errbuf_t    *ebufp)
{
        pin_flist_t             *si_flistp = NULL;
        pin_flist_t             *so_flistp = NULL;
        pin_flist_t             *tmp_flistp = NULL;
        pin_flist_t             *sub_flistp = NULL;
        pin_flist_t             *task_flistp = NULL;
        pin_flist_t             *cred_flistp = NULL;
        pin_flist_t             *subscriber = NULL;


        pin_flist_t             *mso_avp = NULL;
        pin_flist_t             *bb_info = NULL;

        poid_t                  *ac_pdp = NULL;
        void                    *vp = NULL;
        char                    *population_id = "0001"; //this is the only expected value in downstream system
        char                    *ne = NULL;

        char                    *serv_stat = "True";
        char                    *vc_id = NULL;
        char                    *device_type = NULL;
        char                    *acc_no = NULL;
        char                    *node_name = NULL;
        char                    *serv_city = NULL;

        int32                   *state_id = NULL;
        int                     *tech = NULL;




        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;

        PIN_ERRBUF_CLEAR(ebufp);


        /***********************************************************
        * Log input flist
        ***********************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_bb_cam_np_flist input flist", *i_flistpp);

        /*
        * Validate input received
        */

        /*
        * Enrich with additional service details
        */
        si_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_bb_cam_np_flist PCM_OP_READ_OBJ input flist", si_flistp);
        PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_bb_cam_np_flist PCM_OP_READ_OBJ output flist", so_flistp);
        tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
        tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp);
        //ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);
        //PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

        if(*tech == 1 || *tech == 2)
        {
                *act = (char *) malloc(strlen("CHANGE_AUTH_TO_CLICK_DOCSIS")+1);
                strcpy(*act,"CHANGE_AUTH_TO_CLICK_DOCSIS");
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                prepare_docsis_subscriberavps2_flist(ctxp, i_flistpp, so_flistp, *act ,error_codep, ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "prepare_bb_cam_np_flist:Error",ebufp);
                        goto cleanup;
                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"input flist", *i_flistpp);
        }
        else
        {
                //Not required for Ethernet
                //prepare_mqnsure_activation_flist(ctxp, i_flistpp, so_flistp, ebufp);
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
                *error_codep = MSO_NO_PROV_ACTION;
                goto cleanup;
        }
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                *error_codep = MSO_BB_INTERNAL_ERROR;
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "prepare_bb_cam_np_flist: Error",ebufp);
                goto cleanup;
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_bb_cam_np_flist enriched input flist", *i_flistpp);

        cleanup:
        PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
        return;
}

static void
prepare_bb_cam_tn_flist(
        pcm_context_t                   *ctxp,
        pin_flist_t                     **i_flistpp,
        char                            **act,
        int                             *error_codep,
        pin_errbuf_t                    *ebufp)
{
        pin_flist_t                     *si_flistp = NULL;
        pin_flist_t                     *so_flistp = NULL;
        pin_flist_t                     *tmp_flistp = NULL;
        pin_flist_t                     *sub_flistp = NULL;
        pin_flist_t                     *task_flistp = NULL;
        pin_flist_t                     *cred_flistp = NULL;
        pin_flist_t                     *subscriber = NULL;


        pin_flist_t                     *mso_avp = NULL;
        pin_flist_t                     *bb_info = NULL;

        poid_t                          *ac_pdp = NULL;
        void                            *vp = NULL;
        char                            *population_id = "0001"; //this is the only expected value in downstream system
        char                            *ne = NULL;

        char                            *serv_stat = "True";
        char                            *vc_id = NULL;
        char                            *device_type = NULL;
        char                            *acc_no = NULL;
        char                            *node_name = NULL;
        char                            *serv_city = NULL;
        char                            *task = "MODIFY_USER_NSURE";

        int32                           *state_id = NULL;
        int                             *tech = NULL;
        
        char                            *str_role = NULL;
        char                            *city = NULL;
        char                            *ne_add = NULL;
        char                            msg[100];
        
    if (PIN_ERRBUF_IS_ERR(ebufp))
                return;

        PIN_ERRBUF_CLEAR(ebufp);


        /***********************************************************
        * Log input flist
        ***********************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_bb_cam_tn_flist input flist", *i_flistpp);

        /*
        * Validate input received
        */

        /*
        * Enrich with additional service details
        */
        si_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_bb_cam_tn_flist PCM_OP_READ_OBJ input flist", si_flistp);
        PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_bb_cam_tn_flist PCM_OP_READ_OBJ output flist", so_flistp);
        tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
        node_name = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
        str_role = PIN_FLIST_FLD_GET(so_flistp, MSO_FLD_ROLES, 1, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                *error_codep = MSO_BB_NE_ELEMENT_MISSING;
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "prepare_bb_cam_tn_flist: Network Element Missing",ebufp);
                goto cleanup;
        }
        tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp);
        //ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);
        //PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);


        if(*tech == 1 || *tech == 2 || *tech == 5 || *tech == 3)
        {
                *act = (char *) malloc(strlen("CHANGE_AUTH_DOCSIS")+1);
                strcpy(*act,"CHANGE_AUTH_DOCSIS");
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		if (*tech == 1 || *tech == 2){
			prepare_docsis_bcc_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"prepare_bb_cam_tn_flist:Error",ebufp);
				goto cleanup;
			}
		}
                prepare_docsis_subscriberavps2_flist(ctxp, i_flistpp, so_flistp, *act ,error_codep, ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "prepare_bb_cam_tn_flist:Error",ebufp);
                        goto cleanup;
                }
                delete_cred_qps_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "prepare_bb_cam_tn_flist:Error",ebufp);
                        goto cleanup;
                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"input flist", *i_flistpp);
        }
        /*else
        {
                *act = (char *) malloc(strlen("CHANGE_AUTH_ETHERNET")+1);
                strcpy(*act,"CHANGE_AUTH_ETHERNET");
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                ethernet_details_flist(ctxp, i_flistpp, so_flistp, task, error_codep, ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "prepare_bb_cam_tn_flist:Error",ebufp);
                        goto cleanup;
                }
        }*/
	else
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}
        if( (strcmp(other_ne_id, HW_NETWORK_STER_ID) == 0))
        {
            PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_ROLES, str_role, ebufp);
        }
        
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                *error_codep = MSO_BB_INTERNAL_ERROR;
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "prepare_bb_cam_tn_flist: Error",ebufp);
                goto cleanup;
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_bb_cam_tn_flist enriched input flist", *i_flistpp);

        cleanup:
        PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
        return;
}

static void
prepare_bb_cam_nt_flist(
        pcm_context_t                   *ctxp,
        pin_flist_t                     **i_flistpp,
        char                            **act,
        int                             *error_codep,
        pin_errbuf_t                    *ebufp)
{
        pin_flist_t             *si_flistp = NULL;
        pin_flist_t             *so_flistp = NULL;
        pin_flist_t             *tmp_flistp = NULL;
        pin_flist_t             *sub_flistp = NULL;
        pin_flist_t             *task_flistp = NULL;
        pin_flist_t             *cred_flistp = NULL;
        pin_flist_t             *subscriber = NULL;


        pin_flist_t             *mso_avp = NULL;
        pin_flist_t             *bb_info = NULL;

        poid_t                  *ac_pdp = NULL;
        void                    *vp = NULL;
        char                    *population_id = "0001"; //this is the only expected value in downstream system
        char                    *ne = NULL;

        char                    *serv_stat = "True";
        char                    *vc_id = NULL;
        char                    *device_type = NULL;
        char                    *acc_no = NULL;
        char                    *node_name = NULL;
        char                    *serv_city = NULL;
        char                    *task = "MODIFY_USER_NSURE";

        int32                   *state_id = NULL;
        int                     *tech = NULL;
        char                    *city = NULL;
        char                    *ne_add = NULL;
        char                    *str_rolep = NULL;
        char                    msg[100];


        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;

        PIN_ERRBUF_CLEAR(ebufp);


        /***********************************************************
        * Log input flist
        ***********************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_bb_cam_nt_flist input flist", *i_flistpp);

        /*
        * Validate input received
        */

        /*
        * Enrich with additional service details
        */
        si_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_bb_cam_it_flist PCM_OP_READ_OBJ input flist", si_flistp);
        PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "prepare_bb_cam_it_flist PCM_OP_READ_OBJ output flist", so_flistp);
        tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
        tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp);
        str_rolep = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_ROLES, 0, ebufp);
        //ne = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);
        //PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_NETWORK_NODE, *i_flistpp, MSO_FLD_NETWORK_NODE, ebufp);

        if(*tech == 1 || *tech == 2 || *tech == 5 || *tech == 3)
        {
                *act = (char *) malloc(strlen("CHANGE_AUTH_TO_TAL_DOCSIS")+1);
                strcpy(*act,"CHANGE_AUTH_TO_TAL_DOCSIS");
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
		if (*tech == 1 || *tech == 2){
			prepare_docsis_bcc_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"prepare_bb_cam_it_flist:Error",ebufp);
				goto cleanup;
			}
		}
                prepare_docsis_subscriberavps2_flist(ctxp, i_flistpp, so_flistp, *act,error_codep, ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "prepare_bb_cam_nt_flist:Error",ebufp);
                        goto cleanup;
                }
                add_cred_qps_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "prepare_bb_cam_nt_flist:Error",ebufp);
                        goto cleanup;
                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"prepare_bb_cam_it_flist: input flist", *i_flistpp);
        }
        /*if( (strcmp(other_ne_id, HW_NETWORK_STER_ID) == 0))
        {
            PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_ROLES, str_rolep, ebufp);
        }*/
        /*else
        {
                *act = (char *) malloc(strlen("CHANGE_AUTH_ETHERNET")+1);
                strcpy(*act,"CHANGE_AUTH_ETHERNET");
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act );
                ethernet_details_flist(ctxp, i_flistpp, so_flistp, task, error_codep, ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "prepare_bb_cam_nt_flist:Error",ebufp);
                        goto cleanup;
                }
                //prepare_mqnsure_activation_flist(ctxp, i_flistpp, so_flistp, ebufp);
        }*/
	else
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
		*error_codep = MSO_NO_PROV_ACTION;
		goto cleanup;
	}

        if( (strcmp(other_ne_id, HW_NETWORK_STER_ID) == 0))
        {
            PIN_FLIST_FLD_SET(*i_flistpp, MSO_FLD_ROLES, str_rolep, ebufp);
        }
    
        if( (strcmp(other_ne_id,HW_NETWORK_STER_ID) == 0))
        {   
            task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, task_id, ebufp);
            task_id = task_id + 1;
            mso_avp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_AVP, 1, ebufp);
            //Add the Framed Net Mask code and value
            PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_CODE, "Framed-IP-Netmask", ebufp);
            PIN_FLIST_FLD_SET(mso_avp, MSO_FLD_VALUE, "255.255.255.255", ebufp);
        }
        else
            {
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                    "prepare_bb_cam_nt_flist CHANGE_AUTH_TO_TAL_DOCSIS Error", task_flistp);
            }
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                *error_codep = MSO_BB_INTERNAL_ERROR;
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "prepare_bb_cam_nt_flist: Error",ebufp);
                goto cleanup;
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "prepare_bb_cam_nt_flist enriched input flist", *i_flistpp);

        cleanup:
        PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
        return;
}

/****************************************************
* This function is to add multiple IP for Fiber
* and can be enhance if needs for more
****************************************************/
static void
prepare_bb_add_multiple_ip_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	    **i_flistpp,
	char		    **act,
	int		        *error_codep,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*si_flistp = NULL;
	pin_flist_t		*so_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*sub_flistp = NULL;
	pin_flist_t		*task_flistp = NULL;
	pin_flist_t		*cred_flistp = NULL;
	pin_flist_t		*subscriber = NULL;
	
	
	pin_flist_t		*mso_avp = NULL;
	pin_flist_t		*bb_info = NULL;

	poid_t			*ac_pdp = NULL;
	void			*vp = NULL;
	char			*population_id = "0001"; //this is the only expected value in downstream system
	char			*ne = NULL;
	
	char			*serv_stat = "True";
	char 			*vc_id = NULL;
	char			*device_type = NULL;
	char			*acc_no = NULL;
	char			*node_name = NULL;
	char			*serv_city = NULL;
	char                    *str_rolep = NULL;

	int32			*state_id = NULL;
	int32			*is_tal = NULL;
	int			    *tech = NULL;
	
	
	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);


	/***********************************************************
	* Log input flist
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"Begin prepare_bb_add_mulitple_ip_flist input flist", *i_flistpp);

	/*
	* Validate input received
	*/

	/*
	* Enrich with additional service details 
	*/
	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_add_multiple_ip_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_add_multiple_ip_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp); 
	is_tal = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_ISTAL_FLAG, 0, ebufp);
	str_rolep = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_ROLES, 0, ebufp);

	if((*tech == 4 || *tech == 5 ) && str_rolep && strstr(str_rolep, "-Static"))
	{
		*act = (char *) malloc(strlen("CHANGE_SERVICE_ADD_MULTIPLE_IP_FIBER")+1);
		if (*tech == 4)
			strcpy(*act, "CHANGE_SERVICE_ADD_MULTIPLE_IP_FIBER");
		else if(*tech == 5)
			strcpy(*act, "CHANGE_SERVICE_ADD_MULTIPLE_IP_DOCSIS");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act);
		add_cred_qps_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_add_multiple_ip_flist:add_cred_qps_flist Error",ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"input flist", *i_flistpp);
	}
	else
	{
		if (*is_tal == 0)
                	*error_codep = MSO_BB_TAL_AC_ERROR;
		else
                	*error_codep = MSO_NO_PROV_ACTION;
		pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
                goto cleanup;
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_add_multiple_ip_flist: Error",ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_add_multiple_ip_flist enriched input flist", *i_flistpp);

	cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	return;
}

/****************************************************
* This function is to delete ip credentials
* and can be enhance if needs for more
****************************************************/
static void
prepare_bb_remove_cred_flist(
	pcm_context_t	*ctxp,
	pin_flist_t	    **i_flistpp,
	char		    **act,
	int		        *error_codep,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*si_flistp = NULL;
	pin_flist_t		*so_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*sub_flistp = NULL;
	pin_flist_t		*task_flistp = NULL;
	pin_flist_t		*cred_flistp = NULL;
	pin_flist_t		*subscriber = NULL;
	
	
	pin_flist_t		*mso_avp = NULL;
	pin_flist_t		*bb_info = NULL;

	poid_t			*ac_pdp = NULL;
	void			*vp = NULL;
	char			*population_id = "0001"; //this is the only expected value in downstream system
	char			*ne = NULL;
	
	char			*serv_stat = "True";
	char 			*vc_id = NULL;
	char			*device_type = NULL;
	char			*acc_no = NULL;
	char			*node_name = NULL;
	char			*serv_city = NULL;

	int32			*state_id = NULL;
	int32			*action= NULL;
	int			    *tech = NULL;
	
	
	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);


	/***********************************************************
	* Log input flist
	***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"Begin prepare_bb_remove_cred_flist input flist", *i_flistpp);

	/*
	* Validate input received
	*/

	/*
	* Enrich with additional service details 
	*/
	action = (int *)PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_FLAGS, 0, ebufp); 
	si_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(*i_flistpp, PIN_FLD_SERVICE_OBJ, si_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_remove_cred_flist PCM_OP_READ_OBJ input flist", si_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, si_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"prepare_bb_remove_cred_flist PCM_OP_READ_OBJ output flist", so_flistp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	tech = (int *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp); 

	if(tech && (*tech == 4 || *tech == 5 || *tech == 3))
	{
		if (*action == GPON_MAC_CHANGE){
			*act = (char *) malloc(strlen("CHANGE_AUTH_DOCSIS")+1);
			strcpy(*act, "CHANGE_AUTH_DOCSIS");
		}
		else{
			*act = (char *) malloc(strlen("CHANGE_SERVICE_REMOVE_IP_FIBER")+1);
			if (*tech == 4){
				strcpy(*act, "CHANGE_SERVICE_REMOVE_IP_FIBER");
			}
			else if(*tech == 5){
				strcpy(*act, "CHANGE_SERVICE_REMOVE_IP_DOCSIS");
			}
		}
		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*act);
		delete_cred_qps_flist(ctxp, i_flistpp, so_flistp, error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"prepare_bb_remove_cred_flist:add_cred_qps_flist Error",ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"input flist", *i_flistpp);
	}
	else
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_VALUE, MSO_FLD_TECHNOLOGY, 0, 0);
                goto cleanup;
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"prepare_bb_remove_cred_flist: Error",ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"prepare_bb_remove_cred_flist enriched input flist", *i_flistpp);

	cleanup:
	PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
	return;
}

void
mso_prov_fetch_quota_amount(
	pcm_context_t   *ctxp,
	poid_t		    *product_pdp,
	int32		    req_res_id,
	pin_decimal_t   **total_amtp,
	pin_errbuf_t    *ebufp)
{

        pin_flist_t     *args_flistp = NULL;
        pin_flist_t     *search_inflistp = NULL;
        pin_flist_t     *search_outflistp = NULL;
        pin_flist_t     *results_flistp = NULL;
        pin_flist_t     *bal_impact_flistp = NULL;
        pin_flist_t     *quantity_tier_flistp = NULL;

        char            search_template[150] = "select X from /rate 1, /product 2, /rate_plan 3 where 2.F1 = V1 and 2.F2 = 3.F3 and 3.F4 = 1.F5 ";
        int             search_flags = 256;
        int64           db = 1;
        int             elem_id = 0;
        pin_cookie_t    cookie = NULL;
        pin_cookie_t    q_cookie = NULL;
        int             q_elem_id = 0;
        pin_cookie_t    b_cookie = NULL;
        int             b_elem_id = 0;
     	int		        res_id = 0;
	    void		    *vp = NULL;

	pin_decimal_t	*scaled_amtp = pbo_decimal_from_str("0.0",ebufp);
	pin_decimal_t	*fixed_amtp = pbo_decimal_from_str("0.0",ebufp);
	pin_decimal_t	*amtp = pbo_decimal_from_str("0.0",ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling mso_prov_fetch_quota_amount", ebufp);
                return;
        }

	db = PIN_POID_GET_DB(product_pdp);
	search_inflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);
        PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, product_pdp, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 4, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 5, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_RATE_PLAN_OBJ, NULL, ebufp);
        results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "search rate input list", search_inflistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"fm_mso_prov_create_bb_action error: searching rate error", ebufp);
		return;
	}

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                               "search rate out list", search_outflistp);
        while ((results_flistp =
                                PIN_FLIST_ELEM_GET_NEXT(search_outflistp, PIN_FLD_RESULTS,
                               &elem_id, 1, &cookie, ebufp)) != NULL)
        {
		q_elem_id =0; q_cookie = NULL;
		while ((quantity_tier_flistp = PIN_FLIST_ELEM_GET_NEXT(results_flistp,PIN_FLD_QUANTITY_TIERS,
				&q_elem_id, 1, &q_cookie, ebufp)) != NULL)
		{
			b_elem_id  = 0; b_cookie  = NULL;
			while ((bal_impact_flistp = PIN_FLIST_ELEM_GET_NEXT(quantity_tier_flistp,PIN_FLD_BAL_IMPACTS,
				&b_elem_id, 1, &b_cookie, ebufp)) != NULL)
			{
				vp = PIN_FLIST_FLD_GET(bal_impact_flistp,PIN_FLD_ELEMENT_ID,1,ebufp);
				res_id = *(int32*)vp;
				if(req_res_id == res_id)
				{
					scaled_amtp = PIN_FLIST_FLD_GET(bal_impact_flistp,PIN_FLD_SCALED_AMOUNT,1,ebufp);
					fixed_amtp =  PIN_FLIST_FLD_GET(bal_impact_flistp,PIN_FLD_FIXED_AMOUNT,1,ebufp);
					amtp = pbo_decimal_add(scaled_amtp,fixed_amtp,ebufp);
					if(pbo_decimal_is_null(*total_amtp,ebufp))
					{
						*total_amtp =  pbo_decimal_from_str("0.0",ebufp);
					}
					*total_amtp = pbo_decimal_add(*total_amtp,amtp,ebufp);
				}
			}	

		}

	}

	PIN_FLIST_DESTROY_EX(&search_inflistp,NULL);
	PIN_FLIST_DESTROY_EX(&search_outflistp,NULL);
	if(!pbo_decimal_is_null(*total_amtp,ebufp))
	{
		pbo_decimal_negate_assign(*total_amtp,ebufp);
	}

	if(!pbo_decimal_is_null(amtp, ebufp))
        {       pbo_decimal_destroy(&amtp);}
}


static void
ethernet_extend_validity(
	pcm_context_t		*ctxp, 
	pin_flist_t			**i_flistpp, 
	pin_flist_t			*so_flistp,
	int				    *error_codep,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*task_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	pin_flist_t			*acc_detail_flistp = NULL;
	pin_flist_t			*personal_detail_flistp = NULL;
	pin_flist_t			*grp_detail_flistp = NULL;
	pin_flist_t			*user_att_flistp = NULL;
	pin_flist_t			*acc_nameinfo_flistp = NULL;
	pin_flist_t			*alias_list = NULL;

	poid_t				*ac_pdp = NULL;
	pin_cookie_t			cookie = NULL;
	int32				rec_id = 0;
	int32				*prov_flag = NULL;
	char				*node_name = NULL;
	char				*ne_add = NULL;
	char				subnet[100] = {'\0'};
	char				*city = NULL;
	char				*task = "INCREASE_LIMIT_NSURE";
	void				*vp = NULL;

	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	/******Add the Task Array for QPS*******************/
	task_flistp = PIN_FLIST_ELEM_ADD(*i_flistpp, MSO_FLD_TASK, task_id, ebufp);
	task_id = task_id + 1;
	tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	prov_flag = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_FLAGS, 1, ebufp);

	node_name = (char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_NE_ELEMENT_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"ethernet_extend_validity: Network Element Missing",ebufp);
		goto cleanup;
	}
	//Call the function to get the NE Address
	//fm_mso_get_network_node(ctxp, node_name, &ne_add, 2, error_codep, ebufp);
	city = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_codep = MSO_BB_CITY_MISSING;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"ethernet_extend_validity: Network Element Missing",ebufp);
		goto cleanup;
	}
	//Call the function to get the NE Address
	fm_mso_get_network_node(ctxp, node_name, city, &ne_add, 2, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"ethernet_extend_validity: Not able to get Network Element ID",ebufp);
		goto cleanup;
	}
	if(ne_add){
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, (void *)ne_add, ebufp);
	}
	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, (void *)task, ebufp);
	//PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE, "NSURE01", ebufp);
	ac_pdp = (poid_t *)PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_POID, 0, ebufp);
	
	acc_detail_flistp = PIN_FLIST_CREATE(ebufp);
	//Call the function to populate the create balance array
	fm_mso_add_acc_detail(ctxp,*i_flistpp, so_flistp, &acc_detail_flistp, subnet, error_codep, ebufp); 
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "acc_detail_flistp", acc_detail_flistp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "ethernet_extend_validity: Error", ebufp);
		goto cleanup;
	}

	grp_detail_flistp = PIN_FLIST_CREATE(ebufp);
	fm_mso_add_group_detail(ctxp, *i_flistpp, so_flistp, &grp_detail_flistp, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "ethernet_extend_validity: Error", ebufp);
		goto cleanup;
	}

	if (acc_detail_flistp)
	{
		vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(acc_detail_flistp, MSO_FLD_ACCOUNT_DETAIL, PIN_ELEMID_ANY, 1, ebufp );
		if (vp)
		{
			PIN_FLIST_FLD_COPY((pin_flist_t*)vp, PIN_FLD_LOGIN, task_flistp, PIN_FLD_LOGIN, ebufp);
		}
	}

	if (grp_detail_flistp)
	{
		vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(grp_detail_flistp, MSO_FLD_GROUP_DETAIL, PIN_ELEMID_ANY, 1, ebufp );
		if (vp)
		{
			PIN_FLIST_FLD_SET((pin_flist_t*)vp, MSO_FLD_METERING_OVERRIDE, "n", ebufp);
		}
	}

	PIN_FLIST_ELEM_COPY(grp_detail_flistp, MSO_FLD_GROUP_DETAIL,1,
		 task_flistp, MSO_FLD_GROUP_DETAIL, 1, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "ethernet_extend_validity: Error", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ethernet_extend_validity: Return Flist", *i_flistpp);
cleanup:
	PIN_FLIST_DESTROY_EX(&acc_detail_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&grp_detail_flistp, NULL);
	if(ne_add)pin_free(ne_add);
	return;
}

static int32
fm_mso_prov_quota_cap_eligibility(
	pcm_context_t	*ctxp,
	pin_flist_t	*payload_flistp,
	int32		tech,
    char        **initial_amtp,
	pin_errbuf_t	*ebufp)
{

	pin_flist_t	     *read_act_in_flistp = NULL;
	pin_flist_t	     *read_act_out_flistp = NULL;
	pin_flist_t      *tmp_flistp = NULL;
	pin_flist_t	     *cache_flistp = NULL;
	poid_t		     *plan_pdp = NULL;
	poid_t		     *tmp_plan_pdp = NULL;
	char		     *prov_tag = NULL;
	int32		     plan_type = -1 ;
	int32		     quota_flag = 0;
	int32		     is_eligible = 0;
	int32		     eligible_plan = 0;
	int32		     *error_codep = 0;
	int32		     business_type = -1;
	int32		     arr_business_type[8];
	int32		     rec_id = 0;
	cm_cache_key_iddbstr_t	kids;
	int32		     err = PIN_ERR_NONE;
	pin_cookie_t	 cookie = NULL;
	pin_decimal_t 	 *initial_amount = NULL;
	int32	         account_type = 0;
	char		     msg[100] = {'\0'};

	FILE		     *filep = NULL;	

	filep = fopen("/tmp/stackt","a+");

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return 0;
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_prov_quota_cap_eligibility payload_flistp", payload_flistp);

	plan_pdp = PIN_FLIST_FLD_GET(payload_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);

	if ( mso_prov_cap_plans_ptr == (cm_cache_t *)NULL ) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "no config flist for /mso_cfg_cap_plans cached ");
		*error_codep = MSO_BB_CMTS_MASTER_ERROR;
		pin_set_err(ebufp, PIN_ERRLOC_CM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_FLD_CACHE_NAME, 0, 0);
	}
	/**************************************************
	 * See if the entry is in our data dictionary cache
	 * Prepare the cm_cache_key_iddbstr_t structure to search
	 **************************************************/
	kids.id = 0;
	kids.db = 0;
	kids.str = "/mso_cfg_cap_plans";
	cache_flistp = cm_cache_find_entry(mso_prov_cap_plans_ptr, &kids, &err);
	switch(err) 
	{
		case PIN_ERR_NONE:
			break;
		default:
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "mso_cfg_cap_plans from_cache: error accessing data dictionary cache.");
			*error_codep = MSO_BB_CMTS_MASTER_ERROR;
			pin_set_err(ebufp, PIN_ERRLOC_CM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_FLD_CACHE_NAME, 0, 0);
			goto CLEANUP;
	}
	while ((tmp_flistp = PIN_FLIST_ELEM_GET_NEXT (cache_flistp, PIN_FLD_RESULTS, &rec_id, 1,&cookie, ebufp))!= (pin_flist_t *)NULL )
	{
		tmp_plan_pdp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
		if(PIN_POID_COMPARE(plan_pdp, tmp_plan_pdp, 0, ebufp)==0)
		{
			eligible_plan = 1;
            *initial_amtp  = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_INITIALAMOUNT, 1, ebufp);
            sprintf(msg, "INITIAL AMOUNT: %s", *initial_amtp);
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Eligible Plan");
			break;
		}
	}

	if(eligible_plan != 1)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Not Eligible");
		goto CLEANUP;
	}

	prov_tag = PIN_FLIST_FLD_GET(payload_flistp, PIN_FLD_PROVISIONING_TAG, 1, ebufp);

	quota_flag = fm_quota_check(ctxp, NULL, prov_tag, &tech, &plan_type, &initial_amount, error_codep, ebufp);

	if(plan_type != 0)
	{
		goto CLEANUP;
	}

	read_act_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(payload_flistp, PIN_FLD_POID, read_act_in_flistp, PIN_FLD_POID, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_prov_quota_cap_eligibility Account Read input flist", read_act_in_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_act_in_flistp, &read_act_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_prov_quota_cap_eligibility Account Read output flist", read_act_out_flistp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*error_codep = MSO_BB_INTERNAL_ERROR;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "error reading account");
		goto CLEANUP;
	}
	
	if (read_act_out_flistp)
	{	
		business_type = *(int32*) PIN_FLIST_FLD_GET(read_act_out_flistp, PIN_FLD_BUSINESS_TYPE, 0, ebufp);
		num_to_array(business_type, arr_business_type);
		account_type = 10*(arr_business_type[0])+arr_business_type[1];
        	if (account_type != ACCT_TYPE_CORP_SUBS_BB && plan_type == 0 && eligible_plan)
        	{
                	is_eligible = 1;
        	}
	}

	sprintf(msg,"Business Type : %ld",business_type);
	PIN_ERR_LOG_MSG(3, msg);
	sprintf(msg,"Plan Type : %d",plan_type);
	PIN_ERR_LOG_MSG(3, msg);
	
	CLEANUP:
	if(read_act_in_flistp != NULL)PIN_FLIST_DESTROY_EX(&read_act_in_flistp, NULL);
	if(read_act_out_flistp != NULL)PIN_FLIST_DESTROY_EX(&read_act_out_flistp, NULL);

	    if(!pbo_decimal_is_null(initial_amount, ebufp))
{       pbo_decimal_destroy(&initial_amount);}

	if(cache_flistp != NULL )PIN_FLIST_DESTROY_EX(&cache_flistp, NULL);
	return is_eligible;
}

