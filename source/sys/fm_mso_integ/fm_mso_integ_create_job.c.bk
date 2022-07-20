/*******************************************************************
 * File:	fm_mso_integ_create_job.c
 * Opcode:	MSO_OP_INTEG_CREATE_JOB 
 * Owner:	Phani Jandhyala	
 * Created:	10-APR-2014
 * Description: This opcode is for creating the job for device loading 
 * Modification History: Bulk org structure movement
 * Modified By:Sisir Samanta
 * Date:10-Oct-14
 * Modification details : Recreated fm_mso_integ_bulk_hierarchy_change,fm_mso_integ_create_hierarchy_job to handle bulk org structure movement

	r << xx 1
	0 PIN_FLD_POID          POID [0] 0.0.0.1 /account -1 0
	0 PIN_FLD_ORDER_ID      STR [0] "Order-1"
	0 PIN_FLD_ORDER_OBJ     POID [0] 0.0.0.1 /mso_order/stb 507473
	0 PIN_FLD_FLAGS         INT [0] 1
	0 PIN_FLD_DEVICE_TYPE   STR [0] "STB"
	0 MSO_FLD_TASK_NAME     STR [0] "Task-1"
	0 MSO_FLD_TASK          ARRAY [0]
	1	PIN_FLD_POID            POID [0] 0.0.0.1 /device/stb -1 0
	1	MSO_FLD_SERIAL_NO       STR [0] "V7000025"
	1	PIN_FLD_MANUFACTURER    STR [0] "NDS"
	1	PIN_FLD_MODEL           STR [0] "1000C"
	1	PIN_FLD_DEVICE_ID       STR [0] "0000161241230136"
	1	PIN_FLD_DEVICE_TYPE     STR [0] "HD"
	1	PIN_FLD_DESCR           STR [0] "Bulk upload"
	1	PIN_FLD_CATEGORY        STR [0] "NEW"
	1	MSO_FLD_WARRANTY_END_OFFSET   INT [0] 12
	1	MSO_FLD_WARRANTY_END     TSTAMP [0] (0)
	1	PIN_FLD_STATE_ID        INT [0] 1
	1	PIN_FLD_PROGRAM_NAME    STR [0] "device_loader"
	xx
	xop 11005 0 1

	xop: opcode 11005, flags 0
	# number of field entries allocated 20, used 4
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /mso_order/device_XXX 41345 11
	0 PIN_FLD_STATUS         ENUM [0] 0
	0 PIN_FLD_DESCR           STR [0] "Device loaded sucessfully"
	0 MSO_FLD_TASK          ARRAY [0]
	1	PIN_FLD_ORDER_ID      STR [0] "Order-1"
	1	PIN_FLD_ORDER_OBJ     POID [0] 0.0.0.1 /mso_task/stb 507473
	
 *********************************************************************************************
 * Modification History:
 * Modified By: Jyothirmayi Kachiraju 
 * Date: 12/02/2020
 * Modification details: Changes to include the Termination reason type, subtype
 *                       and remarks for CATV Bulk Service Termination job creation cases
 *********************************************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_integ_create_job.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <regex.h>
#include "pcm.h"
#include "pin_cust.h"
#include "ops/act.h"
#include "ops/cust.h"
#include "ops/bill.h"
#include "ops/device.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "mso_cust.h"
#include "mso_bulk_load.h"
#include "mso_device.h"

#define READONLY 0
#define READWRITE 1
#define LOCK_OBJ 2

#define LOCAL_TRANS_OPEN_SUCCESS 0
#define LOCAL_TRANS_OPEN_FAIL 1

#define DEVICE_LOADER 1
#define DEVICE_LOCATION_UPDATE 2 
#define DEVICE_STATE_UPDATE    3

#define MAX_BU_LIMIT   1000
#define MAX_CUST_LIMIT 50000

#define CALLED_BY_OAP  0
#define CALLED_BY_MTA  1

#define NEW_DEVICE_CATEGORY "NEW"


/**************************************
* External Functions start
***************************************/


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
	
extern void
fm_mso_get_account_info(
	pcm_context_t	*ctxp,
	poid_t		*acnt_pdp,
	pin_flist_t	**ret_flistp,
	pin_errbuf_t	*ebufp);
	
extern void
fm_mso_utils_sequence_no(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

PIN_IMPORT int32
fm_mso_is_den_sp(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_errbuf_t		*ebufp);

/**************************************
* External Functions end
***************************************/

/*******************************************************************
 * MSO_OP_INTEG_CREATE_JOB  Functions
 *******************************************************************/

EXPORT_OP void
op_mso_integ_create_job(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp);

static void
fm_mso_integ_create_job(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp);
	
static void
fm_mso_get_bulkacnt_from_acntno(
        pcm_context_t           *ctxp,
        char*                   acnt_no,
        pin_flist_t             **acnt_details,
        pin_errbuf_t            *ebufp);

static void 
fm_mso_integ_device_loader(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp);

static void 
fm_mso_integ_bulk_hierarchy_change(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp);

static void 
fm_mso_integ_device_loaction_updater(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp);

static void
fm_mso_integ_device_state_update(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp);

static void
fm_mso_integ_catv_pre_activation(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp);

static void
fm_mso_integ_add_plan(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp);

static void
fm_mso_integ_change_plan(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp);

static void
fm_mso_integ_cancel_plan(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp);

void
fm_mso_search_lco_account(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_errbuf_t	*ebufp);

extern void
fm_mso_search_device_object(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_errbuf_t	*ebufp);

static void
fm_mso_integ_ar_get_details(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_errbuf_t	*ebufp);

void
fm_mso_integ_create_add_plan_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

void
fm_mso_integ_create_cancel_plan_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

void
fm_mso_integ_create_change_plan_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

void
fm_mso_integ_create_state_change_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

void
fm_mso_integ_create_location_update_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

void
fm_mso_integ_create_catv_preactivation_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

void
fm_mso_integ_create_service_actions_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

static void
fm_mso_get_service_agreement_no(
	pcm_context_t	*ctxp,
	poid_t		*service_pdp,
	pin_flist_t	*i_flistp,
	pin_errbuf_t	*ebufp);



extern void
fm_mso_get_bb_service_agreement_no(
	pcm_context_t	*ctxp,
	char			*agreement_no,
	pin_flist_t		**task_flistpp,
	pin_errbuf_t	*ebufp);

static void
fm_mso_get_account_account_no(
	pcm_context_t	*ctxp,
	poid_t		*account_no,
	pin_flist_t	**task_flistpp,
	pin_errbuf_t	*ebufp);

static void 
fm_mso_integ_service_suspension(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp);

static void 
fm_mso_integ_service_reconnection(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp);

static void 
fm_mso_integ_bulk_crf(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp);

static void 
fm_mso_integ_service_termination(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp);

static void 
fm_mso_integ_bulk_retrack(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp);

static void 
fm_mso_integ_change_bouquet_id(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp);

static void 
fm_mso_integ_set_personnel_bit(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp);

static void 
fm_mso_integ_bulk_osd(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp);

static void 
fm_mso_integ_bulk_bmail(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp);

static void 
fm_mso_integ_bulk_adjustment(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp);

/*void
fm_mso_integ_create_service_actions_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);*/

void
fm_mso_integ_create_retrack_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

void
fm_mso_integ_create_hierarchy_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	int32		caller,
	pin_errbuf_t	*ebufp);	
	
void
fm_mso_integ_create_osd_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

void
fm_mso_integ_create_bmail_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

void
fm_mso_integ_create_adjustment_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

/*void
fm_mso_integ_create_bmail_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);*/

/*void
fm_mso_integ_create_osd_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);*/

void
fm_mso_integ_set_personnel_bit_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

void
fm_mso_integ_create_bouquet_id_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);
	
static void 
fm_mso_integ_device_attribute_updater(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp);

static void 
fm_mso_integ_device_ippool_updater(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp);

static void 
fm_mso_integ_device_grv_uploader(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp);

static void 
fm_mso_integ_device_pair(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp);


void
fm_mso_search_device_poid(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_errbuf_t	*ebufp);

void
fm_mso_integ_create_attribute_update_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

void
fm_mso_integ_create_grv_uploader_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

void
fm_mso_integ_create_grv_uploader_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

void
fm_mso_integ_create_device_pair_job(
        pcm_context_t   *ctxp,
        char                    *task_no,
        poid_t                  *task_pdp,
        pin_flist_t             *task_flistp,
        pin_errbuf_t    *ebufp);

	
EXPORT_OP void
fm_mso_integ_create_ippool_update_job(
        pcm_context_t   *ctxp,
        char                    *task_no,
        poid_t                  *task_pdp,
        pin_flist_t             *task_flistp,
        pin_errbuf_t    *ebufp);

static void
fm_mso_integ_bulk_create_task_generic(
        pcm_context_t   *ctxp,
        int32           task_no,
        pin_flist_t     *i_flistp,
        pin_flist_t     **ret_flistp,
        pin_errbuf_t    *ebufp);

void
fm_mso_integ_create_job_generic(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        int             task_no,
        char            *order_id,
        poid_t          *task_pdp,
        pin_flist_t     *task_flistp,
        poid_t          *user_pdp,
        pin_errbuf_t    *ebufp);

static void 
fm_mso_integ_bulk_cr_dr_note(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp);

void
fm_mso_integ_create_crdr_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

	static void 
fm_validate_sp_code(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp);

void
fm_mso_integ_set_bulk_crf(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

int32
fm_mso_get_impacted_cust_count(
	pcm_context_t		*ctxp,
	pin_flist_t		*search_inflistp,
	int32			child_account_type,
	pin_errbuf_t		*ebufp);

// Added for Bulk Account Creation
static void 
fm_mso_integ_bulk_create_account(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp);

void
fm_mso_integ_blk_create_acct_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

void
fm_mso_search_device_detail(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_errbuf_t	*ebufp);

//  New function added for each BB bulk action

void 
fm_mso_integ_topup_create_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

void 
fm_mso_integ_renew_create_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

void 
fm_mso_integ_add_mb_gb_create_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

void 
fm_mso_integ_extend_validity_create_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

void 
fm_mso_integ_bill_hold_create_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

void 
fm_mso_integ_swaping_cpe_create_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

void 
fm_mso_integ_cpe_writeoff_create_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

void 
fm_mso_integ_cheque_bounce_create_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

void 
fm_mso_integ_refund_load_create_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

void 
fm_mso_integ_refund_reversal_load_create_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);

void 
fm_mso_integ_cmts_change_create_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp);


EXPORT_OP void
fm_mso_search_bb_device_object(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_errbuf_t	*ebufp);

static void
fm_mso_get_plan_poid(
	pcm_context_t	*ctxp,
	poid_t		*plan_name,
	pin_flist_t	**task_flistpp,
	pin_errbuf_t	*ebufp);
// Added for renew Plan CATV----

static void
fm_mso_get_purchase_poid(
	pcm_context_t	*ctxp,
	poid_t		*plan_name,
	pin_flist_t	**task_flistpp,
	pin_errbuf_t	*ebufp);

static void
fm_mso_get_bb_billinfo(
        pcm_context_t           *ctxp,
	pin_flist_t	**task_flistpp,
        pin_errbuf_t            *ebufp);


int
fm_mso_integ_match_patterns(
	char* pattern_string,
	char* input_string);
/****************************************
* Update GST Info
*****************************************/
void fm_mso_bulk_update_gst_info(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp);

PIN_EXPORT void fm_mso_create_update_gst_info_jobs(
        pcm_context_t   *ctxp,
        char            *task_no,
        poid_t          *task_pdp,
        poid_t          *user_id,
        char            *program_name,
        pin_flist_t     *in_flistp,
        pin_errbuf_t    *ebufp);	

/****************************************
* Modify GST Info
*****************************************/
void fm_mso_bulk_modify_gst_info(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp);

PIN_EXPORT void fm_mso_create_modify_gst_info_jobs(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	poid_t		*user_id,
	char		*program_name,
	pin_flist_t	*in_flistp,
	pin_errbuf_t	*ebufp);
/*******************************************************************
 * MSO_OP_INTEG_CREATE_JOB  
 *******************************************************************/
void 
op_mso_integ_create_job(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	*r_flistpp		= NULL;
	int			local = 1;
	int32			*status = NULL;
	//int32			status_uncommitted =2;
	poid_t			*inp_pdp = NULL;
	char            	*prog_name = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERRBUF_CLEAR(ebufp);
	}
	

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_INTEG_CREATE_JOB) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_integ_create_job error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_integ_create_job input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
//  No account locking for integ create job
	inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	local = fm_mso_trans_open(ctxp, inp_pdp, READWRITE,ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Err before calling fm_mso_integ_create_job()", ebufp);
		return;
	}
	
	*r_flistpp = PIN_FLIST_CREATE(ebufp);

	fm_mso_integ_create_job(ctxp, i_flistp, r_flistpp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_integ_create_job output flist", *r_flistpp);

	status = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_STATUS, 1, ebufp);

 	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp) || (*(int*)status == 1)) 
	{		
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
		}
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_integ_create_job error", ebufp);
		if(local == LOCAL_TRANS_OPEN_SUCCESS )
		{
			fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
		}
	}
	else
	{
		if(local == LOCAL_TRANS_OPEN_SUCCESS )
		{
			fm_mso_trans_commit(ctxp, inp_pdp, ebufp);
		}
		else
		{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Transaction NOT Comitted");
				//PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_uncommitted, ebufp);
		}		
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_integ_create_job output flist", *r_flistpp);
	return;
}


/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_integ_create_job(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*ret_flistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*search_inflistp = NULL;
	pin_flist_t	*search_outflistp = NULL;
	pin_flist_t	*results_flistp = NULL;
	pin_flist_t	*change_status_inflistp = NULL;
	pin_flist_t	*order_flistp = NULL;
	pin_flist_t	*order_oflistp = NULL;
	pin_flist_t	*sp_oflistp = NULL;

	poid_t	*user_id = NULL;
	poid_t	*order_pdp = NULL;
	poid_t	*poid_p		= NULL;
	
	char	*account_no = NULL;
	char	*order_no = NULL;
	char	*plan_name = NULL;	
	char	*network_node = NULL;
		
	int32	order_status_sucess = 0;
	int32	order_status_failure = 1;
	int64	db = 0;
	//int64	db = 1;
	int	result_status = 0;
	int	*service_type = NULL;
	int	search_flags = 768;
	int	elem_id = 0;
	int	count = 0;
	char	*device_type = NULL;
	//char	*sp_account_no = NULL;
	int32	*action_flags = NULL;
	int32	*adjustment_type = NULL;
	pin_cookie_t	cookie = NULL;
	
	char	*order_type = NULL;
	char	*order_descr = NULL;
	int32	*status = NULL;	
	*r_flistpp	= NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_create_job input flist", i_flistp);

	order_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ORDER_ID,1,ebufp);	

	poid_p	= PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1 , ebufp);
	
	if(poid_p)
	{
	db = PIN_POID_GET_DB(poid_p);
	}
	order_pdp = PIN_FLIST_FLD_GET(i_flistp , PIN_FLD_ORDER_OBJ , 1, ebufp);

	count = PIN_FLIST_ELEM_COUNT(i_flistp, MSO_FLD_TASK , ebufp);

	device_type = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEVICE_TYPE, 1, ebufp);

	action_flags = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp);

	network_node = PIN_FLIST_FLD_GET(i_flistp,MSO_FLD_NETWORK_NODE, 1 , ebufp);

	adjustment_type = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ADJUSTMENT_TYPE, 1, ebufp);

	if((count == 0) && (((order_pdp) && (PIN_POID_IS_NULL(order_pdp))) || (!order_pdp)))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " First time call hence need to create the order and sent back ");

		if((device_type) && (!strcmp(device_type,"STB")))
		{
			order_pdp = PIN_POID_CREATE(db,"/mso_order/stb", -1, ebufp);
			order_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , "Bulk STB Device loading" , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_USERID , order_flistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_PROGRAM_NAME , order_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
			if(order_no)
			{
				PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
			}
			else
			{
				PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_PO_NO , order_flistp , MSO_FLD_PO_NO , ebufp);
			}
			PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_STOCK_POINT_CODE , order_flistp , MSO_FLD_STOCK_POINT_CODE , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_SUPPLIER_CODE , order_flistp , MSO_FLD_SUPPLIER_CODE , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_INVOICE_NO , order_flistp , MSO_FLD_INVOICE_NO , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_INVOICE_DATE , order_flistp , MSO_FLD_INVOICE_DATE , ebufp);
			if((action_flags) && (*action_flags == 1))
			{
				fm_validate_sp_code(ctxp,order_flistp,&sp_oflistp,ebufp);

				
				//result_status = *(int32*)PIN_FLIST_FLD_GET(sp_oflistp, PIN_FLD_STATUS, 0, ebufp);
				status = PIN_FLIST_FLD_GET(sp_oflistp, PIN_FLD_STATUS, 0, ebufp);
				if(status)
     				{
					result_status = *(int32*)status;
				}
				if(result_status != 0)
				{
					*r_flistpp = sp_oflistp;
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_create_job sp valodation result flist", *r_flistpp);
					return;			

				}
				PIN_FLIST_DESTROY_EX(&sp_oflistp, NULL);
			}
			
			
		}
		else if((device_type) && (strcmp(device_type, "VC") == 0))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " In the vc order block");
			order_pdp = PIN_POID_CREATE(db,"/mso_order/vc", -1,ebufp);
			order_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , "Bulk VC Device loading" , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_USERID , order_flistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_PROGRAM_NAME , order_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
			if(order_no)
			{
				PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
			}
			else
			{
				PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_PO_NO , order_flistp , MSO_FLD_PO_NO , ebufp);
			}
			PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_STOCK_POINT_CODE , order_flistp , MSO_FLD_STOCK_POINT_CODE , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_SUPPLIER_CODE , order_flistp , MSO_FLD_SUPPLIER_CODE , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_INVOICE_NO , order_flistp , MSO_FLD_INVOICE_NO , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_INVOICE_DATE , order_flistp , MSO_FLD_INVOICE_DATE , ebufp);
			 /*Inventory Changes*/
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "order_flistp input flist VC", order_flistp);
			if((action_flags) && (*action_flags == BULK_DEVICE_LOAD))
			{
				fm_validate_sp_code(ctxp,order_flistp,&sp_oflistp,ebufp);

				//result_status = *(int32*)PIN_FLIST_FLD_GET(sp_oflistp, PIN_FLD_STATUS, 0, ebufp);
                                status = PIN_FLIST_FLD_GET(sp_oflistp, PIN_FLD_STATUS, 1, ebufp);
                                if(status)
                                {
                                        result_status = *(int32*)status;
                                }

				if(result_status != 0)
				{
					*r_flistpp = sp_oflistp;
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_create_job sp valodation result flist", *r_flistpp);
					return;			

				}
				PIN_FLIST_DESTROY_EX(&sp_oflistp, NULL);
			}
		}
		else if ((device_type) && (strcmp(device_type,"MODEM") == 0))
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_create_job order flist", i_flistp);
			order_pdp = PIN_POID_CREATE(db,"/mso_order/modem", -1,ebufp);
			order_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , "Bulk modem Device loading" , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_USERID , order_flistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_PROGRAM_NAME , order_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
			if(order_no)
			{
				PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
			}
			else
			{
				PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_PO_NO , order_flistp , MSO_FLD_PO_NO , ebufp);
			}
			PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_STOCK_POINT_CODE , order_flistp , MSO_FLD_STOCK_POINT_CODE , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_SUPPLIER_CODE , order_flistp , MSO_FLD_SUPPLIER_CODE , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_INVOICE_NO , order_flistp , MSO_FLD_INVOICE_NO , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_INVOICE_DATE , order_flistp , MSO_FLD_INVOICE_DATE , ebufp);
			 /*Inventory Changes Nim*/
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "order_flistp input flist MODEM", order_flistp);
			if((action_flags) && (*action_flags == BULK_DEVICE_LOAD))
			{
				fm_validate_sp_code(ctxp,order_flistp,&sp_oflistp,ebufp);

				//result_status = *(int32*)PIN_FLIST_FLD_GET(sp_oflistp, PIN_FLD_STATUS, 0, ebufp);
                                status = PIN_FLIST_FLD_GET(sp_oflistp, PIN_FLD_STATUS, 0, ebufp);
                                if(status)
                                {
                                        result_status = *(int32*)status;
                                }
	
				if(result_status != 0)
				{
					*r_flistpp = sp_oflistp;
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_create_job sp valodation result flist", *r_flistpp);
					return;			

				}
				PIN_FLIST_DESTROY_EX(&sp_oflistp, NULL);
			}
		}
		else if ((device_type) && (strcmp(device_type,"ROUTER/CABLE") == 0))
		{
			order_pdp = PIN_POID_CREATE(db,"/mso_order/router/cable", -1,ebufp);
			order_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , "Bulk Cable router Device loading" , ebufp);
			if(order_no)
			{
				PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
			}
			else
			{
				PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_PO_NO , order_flistp , MSO_FLD_PO_NO , ebufp);
			}
			PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_STOCK_POINT_CODE , order_flistp , MSO_FLD_STOCK_POINT_CODE , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_SUPPLIER_CODE , order_flistp , MSO_FLD_SUPPLIER_CODE , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_INVOICE_NO , order_flistp , MSO_FLD_INVOICE_NO , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_INVOICE_DATE , order_flistp , MSO_FLD_INVOICE_DATE , ebufp);
			 /*Inventory Changes*/
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);
			if((action_flags) && (*action_flags == BULK_DEVICE_LOAD))
			{
				fm_validate_sp_code(ctxp,order_flistp,&sp_oflistp,ebufp);

				//result_status = *(int32*)PIN_FLIST_FLD_GET(sp_oflistp, PIN_FLD_STATUS, 0, ebufp);
                                status = PIN_FLIST_FLD_GET(sp_oflistp, PIN_FLD_STATUS, 1, ebufp);
                                if(status)
                                {
                                        result_status = *(int32*)status;
                                }

				if(result_status != 0)
				{
					*r_flistpp = sp_oflistp;
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_create_job sp valodation result flist", *r_flistpp);
					return;			

				}
				PIN_FLIST_DESTROY_EX(&sp_oflistp, NULL);
			}
		}
		else if ((device_type) && (strcmp(device_type,"ROUTER/WIFI") == 0))
		{
			order_pdp = PIN_POID_CREATE(db,"/mso_order/router/wifi", -1,ebufp);
			order_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , "Bulk WIFI router Device loading" , ebufp);
			if(order_no)
			{
				PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
			}
			else
			{
				PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_PO_NO , order_flistp , MSO_FLD_PO_NO , ebufp);
			}
			PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_STOCK_POINT_CODE , order_flistp , MSO_FLD_STOCK_POINT_CODE , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_SUPPLIER_CODE , order_flistp , MSO_FLD_SUPPLIER_CODE , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_INVOICE_NO , order_flistp , MSO_FLD_INVOICE_NO , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_INVOICE_DATE , order_flistp , MSO_FLD_INVOICE_DATE , ebufp);
			 /*Inventory Changes*/
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);
			if((action_flags) && (*action_flags == BULK_DEVICE_LOAD))
			{
				fm_validate_sp_code(ctxp,order_flistp,&sp_oflistp,ebufp);

				//result_status = *(int32*)PIN_FLIST_FLD_GET(sp_oflistp, PIN_FLD_STATUS, 0, ebufp);
	                        status = PIN_FLIST_FLD_GET(sp_oflistp, PIN_FLD_STATUS, 1, ebufp);
                                if(status)
                                {
                                        result_status = *(int32*)status;
                                }

				if(result_status != 0)
				{
					*r_flistpp = sp_oflistp;
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_create_job sp valodation result flist", *r_flistpp);
					return;			

				}
				PIN_FLIST_DESTROY_EX(&sp_oflistp, NULL);
			}
		}
		else if ((device_type) && (strcmp(device_type,"NAT") == 0))
		{
			order_pdp = PIN_POID_CREATE(db,"/mso_order/nat", -1,ebufp);
			order_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , "Bulk NAT Device loading" , ebufp);
			if(order_no)
			{
				PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
			}
			else
			{
				PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_PO_NO , order_flistp , MSO_FLD_PO_NO , ebufp);
			}
			PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_STOCK_POINT_CODE , order_flistp , MSO_FLD_STOCK_POINT_CODE , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_SUPPLIER_CODE , order_flistp , MSO_FLD_SUPPLIER_CODE , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_INVOICE_NO , order_flistp , MSO_FLD_INVOICE_NO , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_INVOICE_DATE , order_flistp , MSO_FLD_INVOICE_DATE , ebufp);
			 /*Inventory Changes*/
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);
			if((action_flags) && (*action_flags == BULK_DEVICE_LOAD))
			{
				fm_validate_sp_code(ctxp,order_flistp,&sp_oflistp,ebufp);

				//result_status = *(int32*)PIN_FLIST_FLD_GET(sp_oflistp, PIN_FLD_STATUS, 0, ebufp);
                                status = PIN_FLIST_FLD_GET(sp_oflistp, PIN_FLD_STATUS, 1, ebufp);
                                if(status)
                                {
                                        result_status = *(int32*)status;
                                }

				if(result_status != 0)
				{
					*r_flistpp = sp_oflistp;
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_create_job sp valodation result flist", *r_flistpp);
					return;			

				}
				PIN_FLIST_DESTROY_EX(&sp_oflistp, NULL);
			}
		}
		else if ((device_type) && (strcmp(device_type,"IP/STATIC") == 0))
		{
			order_pdp = PIN_POID_CREATE(db,"/mso_ip_order/ip/static", -1,ebufp);
			order_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , "Bulk ip/static Device loading" , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_IP_POOL_NAME, order_flistp , MSO_FLD_IP_POOL_NAME, ebufp);
		//	data_flistp = PIN_FLIST_ELEM_ADD(order_flistp, PIN_FLD_DATA_ARRAY, PIN_ELEMID_ASSIGN, ebufp);
			 /*Inventory Changes*/
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);
			if(order_no)
			{
				PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
			}
			else
			{
				PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_REF_NO , order_flistp , MSO_FLD_REF_NO , ebufp);
			}
			//PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_IP_START_RANGE , data_flistp , MSO_FLD_IP_START_RANGE , ebufp);
			//PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_IP_END_RANGE , data_flistp , MSO_FLD_IP_END_RANGE , ebufp);
			//PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_IP_VERSION , data_flistp , MSO_FLD_IP_VERSION , ebufp);
			//PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_ORDER_DATE , data_flistp , PIN_FLD_ORDER_DATE , ebufp);
			//PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_DESCR , order_flistp , PIN_FLD_DESCR , ebufp);
		}
		else if ((device_type) && (strcmp(device_type,"IP/FRAMED")) == 0)
		{
			order_pdp = PIN_POID_CREATE(db,"/mso_ip_order/ip/framed", -1,ebufp);
			order_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , "Bulk ip/framed Device loading" , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_IP_POOL_NAME, order_flistp , MSO_FLD_IP_POOL_NAME, ebufp);
		//	data_flistp = PIN_FLIST_ELEM_ADD(order_flistp, PIN_FLD_DATA_ARRAY, PIN_ELEMID_ASSIGN, ebufp);
			 /*Inventory Changes*/
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);
		
			if(order_no)
			{
				PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
			}
			else
			{
				PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_REF_NO , order_flistp , MSO_FLD_REF_NO , ebufp);
			}
			//PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_IP_START_RANGE , data_flistp , MSO_FLD_IP_START_RANGE , ebufp);
			//PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_IP_END_RANGE , data_flistp , MSO_FLD_IP_END_RANGE , ebufp);
			//PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_IP_VERSION , data_flistp , MSO_FLD_IP_VERSION , ebufp);
			//PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_ORDER_DATE , data_flistp , PIN_FLD_ORDER_DATE , ebufp);
			//PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_DESCR , order_flistp , PIN_FLD_DESCR , ebufp);
		}
		else if((action_flags) && (*action_flags == BULK_DEVICE_CATV_PREACTIVATION))
		{
			order_pdp = PIN_POID_CREATE(db,"/mso_order/catv_preactivation", -1,ebufp);
			order_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , "Bulk Catv Pre activation" , ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_USERID , order_flistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_PROGRAM_NAME , order_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
			 /*Inventory Changes*/
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);
		}
		else if((action_flags) && (*action_flags == BULK_ADD_PLAN))
		{
			order_pdp = PIN_POID_CREATE(db,"/mso_order/bulk_add_plan", -1,ebufp);
			order_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , "Bulk add plan" , ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_USERID , order_flistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_PROGRAM_NAME , order_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
			 /*Inventory Changes*/
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);
		}
		else if((action_flags) && (*action_flags == BULK_CHANGE_PLAN))
		{
			order_pdp = PIN_POID_CREATE(db,"/mso_order/bulk_change_plan", -1,ebufp);
			order_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , "Bulk change plan" , ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_USERID , order_flistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_PROGRAM_NAME , order_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
			 /*Inventory Changes*/
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);
		}
		else if((action_flags) && (*action_flags == BULK_CANCEL_PLAN))
		{
			order_pdp = PIN_POID_CREATE(db,"/mso_order/bulk_remove_plan", -1,ebufp);
			order_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , "Bulk cancel plan" , ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_USERID , order_flistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_PROGRAM_NAME , order_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
			 /*Inventory Changes*/
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);
		}
		else if((action_flags) && (*action_flags == BULK_SERVICE_SUSPENSION))
		{
			order_pdp = PIN_POID_CREATE(db,"/mso_order/bulk_suspension", -1,ebufp);
			order_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , "Bulk suspend service" , ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_USERID , order_flistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_PROGRAM_NAME , order_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
			 /*Inventory Changes*/
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);
		}
		else if((action_flags) && (*action_flags == BULK_SERVICE_REACTIVATION))
		{
			order_pdp = PIN_POID_CREATE(db,"/mso_order/bulk_reconnection", -1,ebufp);
			order_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , "Bulk service reconnection" , ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_USERID , order_flistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_PROGRAM_NAME , order_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
			 /*Inventory Changes*/
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);
		}
		else if((action_flags) && (*action_flags == BULK_SERVICE_TERMINATION))
		{
			order_pdp = PIN_POID_CREATE(db,"/mso_order/bulk_termination", -1,ebufp);
			order_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID, order_pdp, ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR, "Bulk Service Termination", ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_USERID , order_flistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_PROGRAM_NAME , order_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
			 /*Inventory Changes*/
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);
		}
		else if((action_flags) && (*action_flags == BULK_ADJUSTMENT))
		{
			if((adjustment_type) && (*adjustment_type == 0))  /**** 0 means account level adjustmnet ****/
			{
				order_pdp = PIN_POID_CREATE(db,"/mso_order/bulk_act_adj", -1,ebufp);
				order_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
				PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , "Bulk account adjustment" , ebufp);
				PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_USERID , order_flistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_PROGRAM_NAME , order_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				 /*Inventory Changes*/
				PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);
			}
			else if((adjustment_type) && (*adjustment_type == 1))
			{
				order_pdp = PIN_POID_CREATE(db,"/mso_order/bulk_bill_adj", -1,ebufp);
				order_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
				PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , "Bulk bill adjustment" , ebufp);
				PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_USERID , order_flistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_PROGRAM_NAME , order_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				 /*Inventory Changes*/
				PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);
			}
		}
		else if((action_flags) && (*action_flags == BULK_RETRACK))
		{
			if((network_node) && (strcmp(network_node, "NDS") == 0))
			{
				order_pdp = PIN_POID_CREATE(db,"/mso_order/bulk_nds_retrack", -1,ebufp);
				order_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
				PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , "Bulk nds retrack" , ebufp);
				PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_USERID , order_flistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_PROGRAM_NAME , order_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				 /*Inventory Changes*/
				PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);
			}
			else if ((network_node) && (strcmp(network_node, "CISCO") == 0))
			{
				order_pdp = PIN_POID_CREATE(db,"/mso_order/bulk_cisco_retrack", -1,ebufp);
				order_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
				PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , "Bulk cisco retrack" , ebufp);
				PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_USERID , order_flistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_PROGRAM_NAME , order_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
							 /*Inventory Changes*/
				PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);
			}
			else if ((network_node) && (strcmp(network_node, "CISCO1") == 0))
			{
				order_pdp = PIN_POID_CREATE(db,"/mso_order/bulk_cisco1_retrack", -1,ebufp);
				order_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
				PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , "Bulk cisco1 retrack" , ebufp);
				PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_USERID , order_flistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_PROGRAM_NAME , order_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
							 /*Inventory Changes*/
				PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);
			}
			/******* VERIMATRIX CHANGES - BEGIN - BULK VM RETRACK ****************/
			else if ((network_node) && strstr(network_node, "VM"))
			{
				order_pdp = PIN_POID_CREATE(db,"/mso_order/bulk_vm_retrack", -1,ebufp);
				order_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
				PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , "Bulk vm retrack" , ebufp);
				PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_USERID , order_flistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_PROGRAM_NAME , order_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
							 /*Inventory Changes*/
				PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);
			}
			/******* VERIMATRIX CHANGES - END - BULK VM RETRACK ****************/
		}
		else if((action_flags) && (*action_flags == BULK_OSD))
		{
			if((network_node) && (strcmp(network_node, "CISCO") == 0))
			{
				order_pdp = PIN_POID_CREATE(db,"/mso_order/bulk_cisco_osd", -1,ebufp);
				order_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
				PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , "Bulk CISCO OSD PK" , ebufp);
				PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_USERID , order_flistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_PROGRAM_NAME , order_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
							 /*Inventory Changes*/
				PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);
			}
			else if((network_node) && (strcmp(network_node, "CISCO1") == 0))
			{
				order_pdp = PIN_POID_CREATE(db,"/mso_order/bulk_cisco1_osd", -1,ebufp);
				order_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
				PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , "Bulk CISCO1 OSD PK" , ebufp);
				PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_USERID , order_flistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_PROGRAM_NAME , order_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
							 /*Inventory Changes*/
				PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);
			}
			/******* VERIMATRIX CHANGES - BEGIN - BULK VM OSD ****************/			
			else if((network_node) && strstr(network_node, "VM"))
			{
				order_pdp = PIN_POID_CREATE(db,"/mso_order/bulk_vm_osd", -1,ebufp);
				order_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
				PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , "Bulk VM OSD PK" , ebufp);
				PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_USERID , order_flistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_PROGRAM_NAME , order_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
							 /*Inventory Changes*/
				PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);
			}
			/******* VERIMATRIX CHANGES - END - BULK VM OSD ****************/		
		}
		else if((action_flags) && (*action_flags == BULK_BMAIL))
		{
			if((network_node) && (strcmp(network_node, "CISCO") == 0))
			{
				order_pdp = PIN_POID_CREATE(db,"/mso_order/bulk_cisco_bmail", -1,ebufp);
				order_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
				PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , "Bulk CISCO Bmail PK" , ebufp);
				PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_USERID , order_flistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_PROGRAM_NAME , order_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
							 /*Inventory Changes*/
				PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);
			}
			else if((network_node) && (strcmp(network_node, "CISCO1") == 0))
			{
				order_pdp = PIN_POID_CREATE(db,"/mso_order/bulk_cisco1_bmail", -1,ebufp);
				order_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
				PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , "Bulk CISCO1 Bmail PK" , ebufp);
				PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_USERID , order_flistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_PROGRAM_NAME , order_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
							 /*Inventory Changes*/
				PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);
			}
			/******* VERIMATRIX CHANGES - BEGIN - BULK VM BMAIL ****************/		
			else if((network_node) && strstr(network_node, "VM"))
			{
				order_pdp = PIN_POID_CREATE(db,"/mso_order/bulk_vm_bmail", -1,ebufp);
				order_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
				PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , "Bulk VM Bmail PK" , ebufp);
				PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_USERID , order_flistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_PROGRAM_NAME , order_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
							 /*Inventory Changes*/
				PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);
			}
			/******* VERIMATRIX CHANGES - END - BULK VM BMAIL ****************/		
		}
		/*else if((action_flags) && (*action_flags == BULK_CHANGE_BOUQUET_ID))
		{
			order_pdp = PIN_POID_CREATE(db,"/mso_order/bulk_change_bouquet_id", -1,ebufp);
			order_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , "Bulk change bouquet id" , ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_USERID , order_flistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_PROGRAM_NAME , order_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
		}*/
		else if((action_flags) && (*action_flags == BULK_SET_PERSONAL_BIT))
		{
			order_pdp = PIN_POID_CREATE(db,"/mso_order/bulk_set_personnel_bit", -1,ebufp);
			order_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , "Bulk set personal bit" , ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_USERID , order_flistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_PROGRAM_NAME , order_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
						 /*Inventory Changes*/
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);
		}
		else if((action_flags) && (*action_flags == BULK_CRF))
                {
			 order_pdp = PIN_POID_CREATE(db,"/mso_order/bulk_crf", -1,ebufp);
			 order_flistp = PIN_FLIST_CREATE(ebufp);
			 PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
			 PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , "Bulk set CRF" , ebufp);
			 PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
			 PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_USERID , order_flistp ,PIN_FLD_USERID , ebufp);
			 PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_PROGRAM_NAME , order_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
			 			 /*Inventory Changes*/
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);
                }
 		else if((action_flags) && (*action_flags == BULK_HIERARCHY))
		{
			order_pdp = PIN_POID_CREATE(db,"/mso_order/bulk_hierarchy", -1,ebufp);
			order_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , "Bulk set bulk_hierarchy_change" , ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_USERID , order_flistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_PROGRAM_NAME , order_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);	
							 /*Inventory Changes*/
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);
		}
		else if((action_flags) && (*action_flags == BULK_UPLOAD_GRV))
		{
			order_pdp = PIN_POID_CREATE(db,"/mso_order/grv_upload", -1,ebufp);
			order_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , "GRV Uploadt" , ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_USERID , order_flistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_PROGRAM_NAME , order_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);	
						 /*Inventory Changes*/
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);
		}
		else if((action_flags) && (*action_flags == BULK_DEVICE_PAIR))
		{
			order_pdp = PIN_POID_CREATE(db,"/mso_order/device_pair", -1,ebufp);
			order_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , "Device Pairing" , ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_USERID , order_flistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_PROGRAM_NAME , order_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
						
		}
                else if((action_flags) && (*action_flags == BULK_CMTS_CHANGE))
                {
                        PIN_ERR_LOG_MSG(3,"ENTERRING");
                        order_pdp = PIN_POID_CREATE(db,"/mso_order/bulk_cmts_change", -1,ebufp);
                        order_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
                        PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , "Bulk CMTS Change" , ebufp);
                        PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
                        PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_USERID , order_flistp ,PIN_FLD_USERID , ebufp);
                        PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_PROGRAM_NAME , order_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
									 /*Inventory Changes*/
						PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
						PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
						PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);
                }


		else if(action_flags && (*action_flags > BULK_IPPOOL_UPDATER && *action_flags < 30)) 
		{
                	/*Create Job - Generic - Start*/
                	switch (*action_flags) {
                            case BULK_TOPUP: 
				order_type = BULK_TOPUP_ORDER_TYPE;
				order_descr = BULK_TOPUP_DESCR;
				break;
                            case BULK_RENEW: 
				order_type = BULK_RENEW_ORDER_TYPE;
				order_descr = BULK_RENEW_ORDER_TYPE;
				break;
                            case BULK_EXTEND_VALIDITY: 
				order_type = BULK_EXTEND_VALIDITY_ORDER_TYPE;
				order_descr = BULK_EXTEND_VALIDITY_DESCR;
				break;
                       	    case BULK_ADDITION_MB_GB: 
				order_type = BULK_ADDITION_MB_GB_ORDER_TYPE;
				order_descr = BULK_ADDITION_MB_GB_DESCR;
				break;
                            case BULK_WRITEOFF_CPE: 
				order_type = BULK_WRITEOFF_CPE_ORDER_TYPE;
				order_descr = BULK_WRITEOFF_CPE_DESCR;
				break;
                            case BULK_SWAPPING_CPE: 
				order_type = BULK_SWAPPING_CPE_ORDER_TYPE;
				order_descr = BULK_SWAPPING_CPE_DESCR;
				break;
                            case BULK_CHECK_BOUNCE: 
				order_type = BULK_CHECK_BOUNCE_ORDER_TYPE;
				order_descr = BULK_CHECK_BOUNCE_DESCR;
				break;
                            case BULK_BILL_HOLD: 
				order_type = BULK_BILL_HOLD_ORDER_TYPE;
				order_descr = BULK_BILL_HOLD_DESCR;
				break;
                            case BULK_REFUND_LOAD: 
				order_type = BULK_REFUND_LOAD_ORDER_TYPE;
				order_descr = BULK_REFUND_LOAD_DESCR;
				break;
                            case BULK_REFUND_REV_LOAD: 
				order_type = BULK_REFUND_REV_LOAD_ORDER_TYPE;
				order_descr = BULK_REFUND_REV_LOAD_DESCR;
				break;

                	}
                        order_pdp = PIN_POID_CREATE(db,order_type, -1,ebufp);
                        order_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
                        PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , order_descr , ebufp);
                        PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_USERID , order_flistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_PROGRAM_NAME , order_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
            			 /*Inventory Changes*/
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);			
						
		}
		else if((action_flags) && (*action_flags == BULK_CR_DR_NOTE))
		{
			order_pdp = PIN_POID_CREATE(db,"/mso_order/bulk_cr_dr_note", -1,ebufp);
			order_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , "Credit-Debit Note" , ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_USERID , order_flistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_PROGRAM_NAME , order_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
            			 /*Inventory Changes*/
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);			
		}
		else if((action_flags) && (*action_flags == BULK_CREATE_ACCOUNT))
		{
			order_pdp = PIN_POID_CREATE(db, BULK_CREATE_ACCT_ORDER_TYPE, -1,ebufp);
			order_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID , order_pdp, ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR , "Bulk Account Creation" , ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_USERID , order_flistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , PIN_FLD_PROGRAM_NAME , order_flistp ,PIN_FLD_PROGRAM_NAME , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_PO_NO , order_flistp ,MSO_FLD_PO_NO , ebufp);
						 /*Inventory Changes*/
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);
		}
		else if((action_flags) && (*action_flags == BULK_UPDATE_GST_INFO))
		{
			order_pdp = PIN_POID_CREATE(db, "/mso_order/bulk_update_gst_info", -1, ebufp);
			order_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID, order_pdp, ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR, "Bulk Update GST Info", ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, order_flistp, PIN_FLD_USERID, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, order_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_PO_NO, order_flistp, MSO_FLD_PO_NO, ebufp);
						 /*Inventory Changes*/
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);
		}
		else if((action_flags) && (*action_flags == BULK_MODIFY_GST_INFO))
		{
			order_pdp = PIN_POID_CREATE(db, "/mso_order/bulk_modify_gst_info", -1, ebufp);
			order_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(order_flistp, PIN_FLD_POID, order_pdp, ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_DESCR, "Bulk Modify GST Info", ebufp);
			PIN_FLIST_FLD_SET(order_flistp, PIN_FLD_ORDER_ID, order_no, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, order_flistp, PIN_FLD_USERID, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, order_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_PO_NO, order_flistp, MSO_FLD_PO_NO, ebufp);
			 /*Inventory Changes*/
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);
			
		}
		/*Create Job - Generic - End*/

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Create object input flist " , order_flistp);
		
		PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, order_flistp, &order_oflistp, ebufp);

		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in creating the order object", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			//PIN_FLIST_DESTROY_EX(&order_oflistp, NULL);
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID,poid_p, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS , &order_status_failure , ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error in creating Order object", ebufp);
			*r_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&r_flistp , NULL);
			return;
		}
		/******* 
		 * Prepare the return flist and sent the order deatils 
        	 ******/
		 *r_flistpp = PIN_FLIST_COPY(order_oflistp, ebufp);
		 if(order_no)
		 {
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ORDER_ID , order_no , ebufp);
		 }
		 else
		 {
			PIN_FLIST_FLD_COPY(i_flistp , MSO_FLD_PO_NO , *r_flistpp , MSO_FLD_PO_NO , ebufp);
		 }
		 PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS , &order_status_sucess , ebufp);
		 //PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Order created Sucessfully", ebufp);
		 PIN_FLIST_DESTROY_EX(&order_flistp , NULL);
		 PIN_FLIST_DESTROY_EX(&order_oflistp , NULL);
		 return;			
	}
	else
	{	
	
		action_flags = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 0, ebufp);
	
		if((action_flags) && (*action_flags == BULK_DEVICE_LOAD))
		{
			fm_mso_integ_device_loader(ctxp,i_flistp,&r_flistp , ebufp);
		}	
		else if ((action_flags) && (*action_flags == BULK_DEVICE_LOCATION_UPDATER))
		{
			fm_mso_integ_device_loaction_updater(ctxp, i_flistp , &r_flistp, ebufp);
		}
		else if ((action_flags) && (*action_flags == BULK_DEVICE_STATE_UPDATE))
		{
			fm_mso_integ_device_state_update(ctxp , i_flistp, &r_flistp , ebufp);
		}
		else if((action_flags) && (*action_flags == BULK_DEVICE_CATV_PREACTIVATION))
		{
			fm_mso_integ_catv_pre_activation(ctxp , i_flistp, &r_flistp , ebufp);
		}
		else if((action_flags) && (*action_flags == BULK_ADD_PLAN))
		{
			fm_mso_integ_add_plan(ctxp , i_flistp, &r_flistp , ebufp);
		}
		else if((action_flags) && (*action_flags == BULK_CHANGE_PLAN))
		{
			fm_mso_integ_change_plan(ctxp , i_flistp, &r_flistp , ebufp);
		}
		else if((action_flags) && (*action_flags == BULK_CANCEL_PLAN))
		{
			fm_mso_integ_cancel_plan(ctxp , i_flistp, &r_flistp , ebufp);
		}
		else if((action_flags) && (*action_flags == BULK_SERVICE_SUSPENSION))
		{
			fm_mso_integ_service_suspension(ctxp , i_flistp, &r_flistp , ebufp);
		}
		else if((action_flags) && (*action_flags == BULK_SERVICE_REACTIVATION))
		{
			fm_mso_integ_service_reconnection(ctxp , i_flistp, &r_flistp , ebufp);
		}
		else if((action_flags) && (*action_flags == BULK_SERVICE_TERMINATION))
		{
			fm_mso_integ_service_termination(ctxp , i_flistp, &r_flistp , ebufp);
		}
		else if((action_flags) && (*action_flags == BULK_ADJUSTMENT))
		{
			fm_mso_integ_bulk_adjustment(ctxp , i_flistp, &r_flistp , ebufp);
		}
		else if((action_flags) && (*action_flags == BULK_RETRACK))
		{
			fm_mso_integ_bulk_retrack(ctxp , i_flistp, &r_flistp , ebufp);
		}
		else if((action_flags) && (*action_flags == BULK_OSD))
		{
			fm_mso_integ_bulk_osd(ctxp , i_flistp, &r_flistp , ebufp);
		}
		else if((action_flags) && (*action_flags == BULK_BMAIL))
		{
			fm_mso_integ_bulk_bmail(ctxp , i_flistp, &r_flistp , ebufp);
		}
		/*else if(*action_flags == BULK_CHANGE_BOUQUET_ID)
		{
			fm_mso_integ_change_bouquet_id(ctxp , i_flistp, &r_flistp , ebufp);
		}*/
		else if((action_flags) && (*action_flags == BULK_SET_PERSONAL_BIT))
		{
			fm_mso_integ_set_personnel_bit(ctxp , i_flistp, &r_flistp , ebufp);
		}
		else if((action_flags) && (*action_flags == BULK_CRF))
		{
			fm_mso_integ_bulk_crf(ctxp , i_flistp, &r_flistp , ebufp);
		}
		else if((action_flags) && (*action_flags == BULK_HIERARCHY))
		{
			fm_mso_integ_bulk_hierarchy_change(ctxp , i_flistp, &r_flistp , ebufp);
		}
		else if ((action_flags) && (*action_flags == BULK_IPPOOL_UPDATER))
		{
			fm_mso_integ_device_ippool_updater(ctxp, i_flistp , &r_flistp, ebufp);
		}		

		else if((action_flags) && (*action_flags == BULK_CR_DR_NOTE))
		{
			fm_mso_integ_bulk_cr_dr_note(ctxp , i_flistp, &r_flistp , ebufp);
		}
		else if ((action_flags) && (*action_flags == BULK_UPLOAD_GRV))
		{
			fm_mso_integ_device_grv_uploader(ctxp, i_flistp , &r_flistp, ebufp);
		}
		else if ((action_flags) && (*action_flags == BULK_DEVICE_PAIR))
		{
			fm_mso_integ_device_pair(ctxp, i_flistp , &r_flistp, ebufp);
		}
		else if ((action_flags) && (*action_flags == BULK_DEVICE_ATTR_UPDATER))
		{
			fm_mso_integ_device_attribute_updater(ctxp, i_flistp , &r_flistp, ebufp);
		}
		else if ((action_flags) && (*action_flags == BULK_TOPUP))
		{
			fm_mso_integ_bulk_create_task_generic(ctxp, *action_flags, i_flistp, &r_flistp, ebufp);
		}
		else if ((action_flags) && (*action_flags == BULK_RENEW))
		{
			fm_mso_integ_bulk_create_task_generic(ctxp, *action_flags, i_flistp, &r_flistp, ebufp);
		}
		else if ((action_flags) && (*action_flags == BULK_EXTEND_VALIDITY))
		{
			fm_mso_integ_bulk_create_task_generic(ctxp, *action_flags, i_flistp, &r_flistp, ebufp);
		}
		else if ((action_flags) && (*action_flags == BULK_ADDITION_MB_GB))
		{
			fm_mso_integ_bulk_create_task_generic(ctxp, *action_flags, i_flistp, &r_flistp, ebufp);
		}
		else if ((action_flags) && (*action_flags == BULK_WRITEOFF_CPE))
		{
			fm_mso_integ_bulk_create_task_generic(ctxp, *action_flags, i_flistp, &r_flistp, ebufp);
		}
		else if ((action_flags) && (*action_flags == BULK_SWAPPING_CPE))
		{
			fm_mso_integ_bulk_create_task_generic(ctxp, *action_flags, i_flistp, &r_flistp, ebufp);
		}
		else if ((action_flags) && (*action_flags == BULK_CHECK_BOUNCE))
		{
			fm_mso_integ_bulk_create_task_generic(ctxp, *action_flags, i_flistp, &r_flistp, ebufp);
		}
		else if ((action_flags) && (*action_flags == BULK_BILL_HOLD))
		{
			fm_mso_integ_bulk_create_task_generic(ctxp, *action_flags, i_flistp, &r_flistp, ebufp);
		}
		else if ((action_flags) && (*action_flags == BULK_REFUND_LOAD))
		{
			fm_mso_integ_bulk_create_task_generic(ctxp, *action_flags, i_flistp, &r_flistp, ebufp);
		}
		else if ((action_flags) && (*action_flags == BULK_REFUND_REV_LOAD))
		{
			fm_mso_integ_bulk_create_task_generic(ctxp, *action_flags, i_flistp, &r_flistp, ebufp);
		}
		else if ((action_flags) && (*action_flags == BULK_CMTS_CHANGE))
		{
			fm_mso_integ_bulk_create_task_generic(ctxp, *action_flags, i_flistp, &r_flistp, ebufp);
		}
		else if((action_flags) && (*action_flags == BULK_CREATE_ACCOUNT))
		{
			fm_mso_integ_bulk_create_account(ctxp , i_flistp, &r_flistp , ebufp);
		}
		else if((action_flags) && (*action_flags == BULK_UPDATE_GST_INFO))
		{
			fm_mso_bulk_update_gst_info(ctxp, i_flistp, &r_flistp , ebufp);
		}
		else if((action_flags) && (*action_flags == BULK_MODIFY_GST_INFO))
		{
			fm_mso_bulk_modify_gst_info(ctxp, i_flistp, &r_flistp , ebufp);
		}
//		/*Create Task - Generic - Start*/
//		if (action_flags && (*action_flags > BULK_IPPOOL_UPDATER && *action_flags < 30 )) {
//		    fm_mso_integ_bulk_create_task_generic(ctxp, *action_flags, i_flistp, &r_flistp, ebufp);
//		}
//		/*Create Task - Generic - End*/
		
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_create_job reactivation error", ebufp);
			*r_flistpp = PIN_FLIST_COPY(ret_flistp , ebufp);
			PIN_FLIST_DESTROY_EX(&ret_flistp , NULL);
			return;
		}
		else
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Return flist from after each function call is " , r_flistp);
			*r_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
		}
		
		PIN_FLIST_DESTROY_EX(&r_flistp , NULL);
		return;
	}

	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_create_job  error", ebufp);
		*r_flistpp = PIN_FLIST_COPY(ret_flistp , ebufp);
		PIN_FLIST_DESTROY_EX(&ret_flistp , NULL);
		return;
	}
	return;
}

//============================================================================
//
// FUNTION NAME     : fm_mso_integ_match_patterns
//
// DESCRIPTION      : Perform a regular expression match on a string against
//                    a regular expression
//
// INPUT PARAMETERS :
//      pattern_string              : string containing the regular expression
//      input_string                : the string possibly containing the pattern
//
// RETURN VALUE     :
//      0 - no match
//      1 - match
//     -1 - error
//
// AUTHOR           : Tim Zhao
//
// LAST UPDATE      : 02/03/2015
//
//============================================================================
int
fm_mso_integ_match_patterns(char* pattern_string, char* input_string)
{
    regex_t *regex;
    regmatch_t *result;
    int return_val = 0;

    /* Make space for the regular expression */

    if ((regex = (regex_t *) malloc(sizeof(regex_t))) == NULL)
    {
        fprintf(stdout, "No more memory - aaaagh! (Die kicking and screaming.)\n");
        return -1;
    }
    else
    {
        memset(regex, 0, sizeof(regex_t));
    }

    if ((regex != NULL) && (regcomp(regex, pattern_string, REG_EXTENDED) == 0))
    {
        if((result = (regmatch_t *) malloc(sizeof(regmatch_t)))==0)
        {
            fprintf(stdout, "No more memory - aaaagh! (Die kicking and screaming.)\n");
            return_val = -1;
        }

        if (regexec(regex, input_string, 1, result, 0)==0) // found a match
        {
            if (strlen(input_string) == result->rm_eo) return_val = 1;
            else 0;
        }
        else
        {
            return_val = 0;
        }
    }

    if (regex != NULL)
    {
        regfree(regex); /* Free the regular expression data structure */
        free(regex);
    }
    if (result != NULL)
    {
        free(result);
    }

    return return_val;
} // fm_mso_integ_match_patterns

static void 
fm_mso_integ_device_loader(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**ret_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*in_flistp = NULL;
	pin_flist_t	*task_flistp = NULL;
	pin_flist_t	*job_oflistp = NULL;
	pin_flist_t	*job_iflistp = NULL;
	pin_flist_t	*order_iflistp = NULL;
	pin_flist_t	*order_oflistp = NULL;
	pin_flist_t	*r_flistp = NULL;

	pin_flist_t	*ippool_iflistp = NULL;
	pin_flist_t	*ippool_oflistp = NULL;
	pin_flist_t	*data_flistp = NULL;

	poid_t		*order_pdp	= NULL;
	poid_t		*task_pdp	= NULL;
	poid_t		*job_pdp	= NULL;
	poid_t		*user_id	= NULL;
	char		*program_name	= NULL;

	char		*device_type	= NULL;
	char		*task_no	= NULL;
	char		*order_id	= NULL;
        /*Inventory Changes*/
    char            *origin_country = NULL;
    char            *lot_number     = NULL;
    time_t          shipment_date   = 0;
	

	//int64		db = 1;
	int64		db = 0;
	int32		order_status_sucess = 0;
	int32		order_status_failure = 1;
	pin_buf_t	*nota_buf = NULL;
	char		*flistbuf	= NULL;
	int32		flistlen	= 0;
	int		elem_id = 0;
	int32		status = 0;
	int32		opcode = PCM_OP_DEVICE_CREATE;
	int32		status_fail = 2;
	pin_cookie_t	cookie = NULL;

	char		*ip_start_range		= NULL;
	char		*ip_end_range		= NULL;
	char		*category = NEW_DEVICE_CATEGORY;
	unsigned int	ip_start = 0;

	int32		a = 0;
	int32		b = 0;
	int32		c = 0;
	int32		d = 0;
	unsigned int	ip_end = 0;
	unsigned int	i = 0;
	int32		state_id = 1;
	pin_flist_t	*my_task_flistp = NULL;
	pin_flist_t	*ipdata_flistp = NULL;
	pin_flist_t	*read_order_iflistp = NULL;
	pin_flist_t	*read_order_oflistp = NULL;
	char		ip_addr[128];
	char		*device_id = NULL;
	char		*device_tech_type = NULL;
	char		message[1024];
	char		msg[250];
	int32           *input_status = NULL;
	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_device_loader input flist", i_flistp);

	in_flistp = PIN_FLIST_COPY(i_flistp , ebufp);

	order_pdp	= PIN_FLIST_FLD_GET(in_flistp , PIN_FLD_ORDER_OBJ , 1 , ebufp);
	
	db = PIN_POID_GET_DB(order_pdp);

	//input_status = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_STATUS, 1 , ebufp);
	task_no		= PIN_FLIST_FLD_GET(in_flistp , MSO_FLD_TASK_NAME , 1, ebufp);

	order_id	= PIN_FLIST_FLD_GET(in_flistp ,MSO_FLD_PO_NO , 1, ebufp);
	user_id		= PIN_FLIST_FLD_GET(in_flistp ,PIN_FLD_USERID , 1, ebufp);
	program_name	= PIN_FLIST_FLD_GET(in_flistp ,PIN_FLD_PROGRAM_NAME, 1 , ebufp);

	device_type = PIN_FLIST_FLD_GET(in_flistp ,PIN_FLD_DEVICE_TYPE, 1 , ebufp);
	

	/******
	 * Create the task object in the brm 
	 *****/

	order_iflistp = PIN_FLIST_CREATE(ebufp);

	if((device_type) && (strcmp(device_type , "STB") == 0))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/device/stb", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "STB_Device_loading" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_PO_NO , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_USERID , user_id , ebufp);
		 /*Inventory Changes*/
		/*PIN_FLIST_FLD_SET(order_iflistp, MSO_FLD_CR_ADJ_DATE, &shipment_date, ebufp);
		PIN_FLIST_FLD_SET(order_iflistp, PIN_FLD_COUNTRY, origin_country, ebufp);
		PIN_FLIST_FLD_SET(order_iflistp, MSO_FLD_LICENSE_NO, lot_number, ebufp);*/
						/*Inventory Changes Nim*/
		/*PIN_FLIST_FLD_COPY(order_iflistp, MSO_FLD_CR_ADJ_DATE, order_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, order_flistp, PIN_FLD_COUNTRY, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LICENSE_NO, order_flistp, MSO_FLD_LICENSE_NO, ebufp);*/
	}
	else if ((device_type) && (strcmp(device_type , "VC") == 0))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/device/vc", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "VC_Device_loading" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_PO_NO , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_USERID , user_id , ebufp);
                 /*Inventory Changes*/
                /*PIN_FLIST_FLD_SET(order_iflistp, MSO_FLD_CR_ADJ_DATE, &shipment_date, ebufp);
                PIN_FLIST_FLD_SET(order_iflistp, PIN_FLD_COUNTRY, origin_country, ebufp);
                PIN_FLIST_FLD_SET(order_iflistp, MSO_FLD_LICENSE_NO, lot_number, ebufp);*/
	}
	else if ((device_type) && (strcmp(device_type , "MODEM") == 0))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/device/modem", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Modem_Device_loading" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_PO_NO , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
        /*Inventory Changes*/
        /*PIN_FLIST_FLD_SET(order_iflistp, MSO_FLD_CR_ADJ_DATE, &shipment_date, ebufp);
        PIN_FLIST_FLD_SET(order_iflistp, PIN_FLD_COUNTRY, origin_country, ebufp);
        PIN_FLIST_FLD_SET(order_iflistp, MSO_FLD_LICENSE_NO, lot_number, ebufp);*/
	}

	else if ((device_type) && (strcmp(device_type , "ROUTER") == 0))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/device/router", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Router_Device_loading" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_PO_NO , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_USERID , user_id , ebufp);
                 /*Inventory Changes*/
                /*PIN_FLIST_FLD_SET(order_iflistp, MSO_FLD_CR_ADJ_DATE, &shipment_date, ebufp);
                PIN_FLIST_FLD_SET(order_iflistp, PIN_FLD_COUNTRY, origin_country, ebufp);
                PIN_FLIST_FLD_SET(order_iflistp, MSO_FLD_LICENSE_NO, lot_number, ebufp);*/
	}

	else if ((device_type) && (strcmp(device_type , "ROUTER/CABLE") == 0))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/device/router/cable", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Cable_Router_Device_loading" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_PO_NO , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
                 /*Inventory Changes*/
                /*PIN_FLIST_FLD_SET(order_iflistp, MSO_FLD_CR_ADJ_DATE, &shipment_date, ebufp);
                PIN_FLIST_FLD_SET(order_iflistp, PIN_FLD_COUNTRY, origin_country, ebufp);
                PIN_FLIST_FLD_SET(order_iflistp, MSO_FLD_LICENSE_NO, lot_number, ebufp);*/
	}
	else if ((device_type) && (strcmp(device_type , "ROUTER/WIFI") == 0))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/device/router/wifi", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "WIFI_Router_Device_loading" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_PO_NO , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
                 /*Inventory Changes*/
                /*PIN_FLIST_FLD_SET(order_iflistp, MSO_FLD_CR_ADJ_DATE, &shipment_date, ebufp);
                PIN_FLIST_FLD_SET(order_iflistp, PIN_FLD_COUNTRY, origin_country, ebufp);
                PIN_FLIST_FLD_SET(order_iflistp, MSO_FLD_LICENSE_NO, lot_number, ebufp);*/
	}
	else if ((device_type) && (strcmp(device_type , "NAT") == 0))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/device/nat", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "NAT_Device_loading" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_PO_NO , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
                 /*Inventory Changes*/
                /*PIN_FLIST_FLD_SET(order_iflistp, MSO_FLD_CR_ADJ_DATE, &shipment_date, ebufp);
                PIN_FLIST_FLD_SET(order_iflistp, PIN_FLD_COUNTRY, origin_country, ebufp);
                PIN_FLIST_FLD_SET(order_iflistp, MSO_FLD_LICENSE_NO, lot_number, ebufp);*/
	}

	else if ((device_type) && (strcmp(device_type , "IP/STATIC") == 0))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/device/ip/static", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Static_IP_Device_loading" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_PO_NO , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
                 /*Inventory Changes*/
                /*PIN_FLIST_FLD_SET(order_iflistp, MSO_FLD_CR_ADJ_DATE, &shipment_date, ebufp);
                PIN_FLIST_FLD_SET(order_iflistp, PIN_FLD_COUNTRY, origin_country, ebufp);
                PIN_FLIST_FLD_SET(order_iflistp, MSO_FLD_LICENSE_NO, lot_number, ebufp);*/
	}
	else if((device_type) && (strcmp(device_type , "IP/FRAMED") == 0))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/device/ip/framed", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Framed_IP_Device_loading" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_PO_NO , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
                 /*Inventory Changes*/
                /*PIN_FLIST_FLD_SET(order_iflistp, MSO_FLD_CR_ADJ_DATE, &shipment_date, ebufp);
                PIN_FLIST_FLD_SET(order_iflistp, PIN_FLD_COUNTRY, origin_country, ebufp);
                PIN_FLIST_FLD_SET(order_iflistp, MSO_FLD_LICENSE_NO, lot_number, ebufp);*/
	}


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create input flist is " , order_iflistp);
	PCM_OP(ctxp,PCM_OP_CREATE_OBJ,0 , order_iflistp , &order_oflistp , ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the task object " );
		PIN_FLIST_DESTROY_EX(&order_oflistp , NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_create_job task object create failed", ebufp);
		*ret_flistp = PIN_FLIST_COPY(r_flistp , ebufp);
		PIN_FLIST_DESTROY_EX(&r_flistp , NULL);
		PIN_FLIST_DESTROY_EX(&order_iflistp , NULL);
		//return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create output flist is " , order_oflistp);
	input_status = PIN_FLIST_FLD_GET(order_iflistp,PIN_FLD_STATUS, 1 , ebufp);	
	if(order_oflistp)
	{
		/*
		 * if MSO_FLD_TASK == NULL for IP devices, 
		 * 1. Read Start&End IPs from /mso_ip_order
		 * 2. Add task array as per Start&End IPs
		 */


     
		if ((((device_type) && (strcmp(device_type , "IP/STATIC") == 0)) || ((device_type) && (strcmp(device_type , "IP/FRAMED") == 0))) && (PIN_FLIST_ELEM_COUNT(i_flistp, MSO_FLD_TASK , ebufp) == 0) )
		{
			read_order_iflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(read_order_iflistp, PIN_FLD_POID,order_pdp, ebufp );
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Read Pool obj input flist" , read_order_iflistp);

			PCM_OP(ctxp,PCM_OP_READ_OBJ,0 , read_order_iflistp , &read_order_oflistp , ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Read Pool obj output flist" , read_order_oflistp);
			PIN_FLIST_DESTROY_EX(&read_order_iflistp , NULL);

			ip_start_range = PIN_FLIST_FLD_GET(in_flistp , MSO_FLD_IP_START_RANGE, 1 , ebufp);
			ip_end_range = PIN_FLIST_FLD_GET(in_flistp , MSO_FLD_IP_END_RANGE, 1 , ebufp);
			

			// Read Start&End IPs from /mso_ip_order
			ippool_iflistp = PIN_FLIST_CREATE(ebufp);
			//PIN_FLIST_FLD_SET(ippool_iflistp, PIN_FLD_POID, order_pdp, ebufp );
			PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ORDER_OBJ, ippool_iflistp, PIN_FLD_POID, ebufp );
			data_flistp = PIN_FLIST_ELEM_ADD(ippool_iflistp, PIN_FLD_DATA_ARRAY, PIN_ELEMID_ASSIGN, ebufp);
			PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_IP_START_RANGE, data_flistp, MSO_FLD_IP_START_RANGE, ebufp );
			PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_IP_END_RANGE, data_flistp, MSO_FLD_IP_END_RANGE, ebufp );
			PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_IP_VERSION, data_flistp, MSO_FLD_IP_VERSION, ebufp );
			PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ORDER_DATE, data_flistp, PIN_FLD_ORDER_DATE, ebufp );

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Create IP Pool input flist" , ippool_iflistp);
			PCM_OP(ctxp,PCM_OP_WRITE_FLDS,32 , ippool_iflistp , &ippool_oflistp , ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating ip pool " );
				PIN_FLIST_DESTROY_EX(&ippool_oflistp , NULL);
				r_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Failed to Create IP Pool", ebufp);
				*ret_flistp = PIN_FLIST_COPY(r_flistp , ebufp);
				PIN_FLIST_DESTROY_EX(&r_flistp , NULL);
				PIN_FLIST_DESTROY_EX(&ippool_iflistp , NULL);
				return;
			}
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Create IP Pool output flist " , ippool_oflistp);
			

			/*creating ip without negative value*/
			ip_start += atoi(strtok(ip_start_range, "."))*256*256*256;
                        ip_start += atoi(strtok(NULL, "."))*256*256;
                        ip_start += atoi(strtok(NULL, "."))*256;
                        ip_start += atoi(strtok(NULL, "."))*1;
			
			 sprintf(msg, "start range after mod   %u is",ip_start);
	                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , msg );


			ip_end += atoi(strtok(ip_end_range, "."))*256*256*256;
			ip_end += atoi(strtok(NULL, "."))*256*256;
			ip_end += atoi(strtok(NULL, "."))*256;
			ip_end += atoi(strtok(NULL, "."))*1;
			 sprintf(msg, "start range after mod   %u is",ip_end);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , msg );
		
				
			// Add task array
			for (i=ip_start; i<=ip_end; i++)
			{
				sprintf(ip_addr, "%u.%u.%u.%u", i/(256*256*256), (i % (256*256*256))/(256*256), ( (i % (256*256*256)) % (256*256) )/256, ( (i % (256*256*256)) % (256*256) ) % 256);      

				/*changes ended here*/

				 sprintf(msg, "start range after mod  %s is", ip_addr);
                        	 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , msg );
				my_task_flistp = PIN_FLIST_ELEM_ADD(in_flistp, MSO_FLD_TASK, i-ip_start, ebufp );
				PIN_FLIST_FLD_SET(my_task_flistp, PIN_FLD_DEVICE_ID , ip_addr,ebufp);
				PIN_FLIST_FLD_SET(my_task_flistp, PIN_FLD_MANUFACTURER , "",ebufp);
				PIN_FLIST_FLD_SET(my_task_flistp, PIN_FLD_MODEL , "",ebufp);
				PIN_FLIST_FLD_SET(my_task_flistp, PIN_FLD_DESCR , "",ebufp);
				PIN_FLIST_FLD_SET(my_task_flistp, PIN_FLD_STATE_ID , &state_id,ebufp);

				/**** Pavan Bellala 06-11-2015 ********************
				Always make IP device provisionable.Set value to 1
				***************************************************/
				PIN_FLIST_FLD_SET(my_task_flistp, MSO_FLD_PROVISIONABLE_FLAG, &state_id,ebufp);

				ipdata_flistp = PIN_FLIST_ELEM_ADD(my_task_flistp, MSO_FLD_IP_DATA, 0, ebufp );
				PIN_FLIST_FLD_SET(ipdata_flistp, MSO_FLD_IP_ADDRS , ip_addr,ebufp);
				//PIN_FLIST_FLD_SET(ipdata_flistp, MSO_FLD_IP_VERSION , "IPv4",ebufp);
				PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_IP_VERSION , ipdata_flistp , MSO_FLD_IP_VERSION , ebufp);
				PIN_FLIST_FLD_COPY(read_order_oflistp, MSO_FLD_IP_POOL_NAME, ipdata_flistp , MSO_FLD_IP_POOL_NAME , ebufp);
				PIN_FLIST_FLD_SET(ipdata_flistp, MSO_FLD_IP_MASK_VALUE , "",ebufp);
				PIN_FLIST_FLD_SET(ipdata_flistp, PIN_FLD_SEGMENT_NAME , "",ebufp);
			}

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Input Task Array created: " , in_flistp);
		
			PIN_FLIST_DESTROY_EX(&ippool_oflistp , NULL);
			PIN_FLIST_DESTROY_EX(&read_order_oflistp , NULL);
		}

		task_pdp = PIN_FLIST_FLD_GET(order_oflistp , PIN_FLD_POID , 1, ebufp);
		/******
		 *Read all the task arrays from the input and prepare the mso_mta_jobs
		 *****/
		 while ((task_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp,MSO_FLD_TASK, &elem_id, 1, &cookie, ebufp))!= NULL)
		 {
			 char		*flistbuf	= NULL;
			 pin_buf_t	*nota_buf	= NULL;
			 flistlen = 0;
			 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " task_flistp is " ,task_flistp);
			 PIN_FLIST_FLD_SET(task_flistp,MSO_FLD_ORDER_OBJ ,order_pdp , ebufp);
			 PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_CATEGORY, (void *)category, ebufp);
			 
			 device_id = PIN_FLIST_FLD_GET(task_flistp ,PIN_FLD_DEVICE_ID, 1 , ebufp);
			 device_tech_type = PIN_FLIST_FLD_GET(task_flistp ,PIN_FLD_DEVICE_TYPE, 1 , ebufp);
			 
/*			if ( (strcmp(device_type , "MODEM") == 0 || strcmp(device_type , "ROUTER/CABLE") == 0 || strcmp(device_type , "ROUTER/WIFI") == 0 || strcmp(device_type , "NAT") == 0) && 
				(fm_mso_integ_match_patterns("^([0-9a-fA-F]{2})(([/\\s:-][0-9a-fA-F]{2}){5})$", device_id) <=0 ) )
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the task object " );
				PIN_FLIST_DESTROY_EX(&order_oflistp , NULL);
				r_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
				
				char	message[1024];
				sprintf(message, "Invalid Device ID: %s, it is NOT a MAC address.", device_id);
				
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, message, ebufp);
				*ret_flistp = PIN_FLIST_COPY(r_flistp , ebufp);
				PIN_FLIST_DESTROY_EX(&r_flistp , NULL);
				return;
			} */
			 
			 if((device_type) && (strcmp(device_type , "STB") == 0))
			{
				PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_POID , PIN_POID_CREATE(db ,"/device/stb", -1 ,ebufp),ebufp);
				PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME , task_flistp , PIN_FLD_PROGRAM_NAME , ebufp);
	 		  }
			  else if((device_type) && (strcmp(device_type , "VC") == 0))
			  {
				PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_POID , PIN_POID_CREATE(db ,"/device/vc", -1,ebufp),ebufp);
				PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME , task_flistp , PIN_FLD_PROGRAM_NAME , ebufp);
			  }
			  else if((device_type) && (strcmp(device_type , "MODEM") == 0))
			  {
				PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_POID , PIN_POID_CREATE(db ,"/device/modem", -1,ebufp),ebufp);
				PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME , task_flistp , PIN_FLD_PROGRAM_NAME , ebufp);
			  }
			  else if((device_type) && (strcmp(device_type , "ROUTER/CABLE") == 0))
			  {
				PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_POID , PIN_POID_CREATE(db ,"/device/router/cable", -1,ebufp),ebufp);
				PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME , task_flistp , PIN_FLD_PROGRAM_NAME , ebufp);
			  }
			  else if((device_type) && (strcmp(device_type , "ROUTER/WIFI") == 0))
			  {
				PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_POID , PIN_POID_CREATE(db ,"/device/router/wifi", -1,ebufp),ebufp);
				PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME , task_flistp , PIN_FLD_PROGRAM_NAME , ebufp);
			  }
			  else if((device_type) && (strcmp(device_type , "NAT") == 0))
			  {
				PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_POID , PIN_POID_CREATE(db ,"/device/nat", -1,ebufp),ebufp);
				PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME , task_flistp , PIN_FLD_PROGRAM_NAME , ebufp);
			  }
			  else if((device_type) && (strcmp(device_type , "IP/STATIC") == 0))
			  {
				PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_POID , PIN_POID_CREATE(db ,"/device/ip/static", -1,ebufp),ebufp);
				PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME , task_flistp , PIN_FLD_PROGRAM_NAME , ebufp);
			  }
			  else if((device_type) && (strcmp(device_type , "IP/FRAMED") == 0))
			  {
				PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_POID , PIN_POID_CREATE(db ,"/device/ip/framed", -1,ebufp),ebufp);
				PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME , task_flistp , PIN_FLD_PROGRAM_NAME , ebufp);
			  }
 
			  PIN_FLIST_TO_STR( task_flistp, &flistbuf, &flistlen, ebufp );

				nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
				if ( nota_buf == NULL )
				{
					pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
					 PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
					 return;
				}
				/*
				 * Create flist for job order creation
				 */

				job_iflistp	= PIN_FLIST_CREATE(ebufp);
				
				
				if((device_type) && (strcmp(device_type ,"STB") == 0))
				{
					job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/device/stb", -1 ,ebufp);
					PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_USERID , user_id , ebufp);
					/*Inventory Changes*/
					PIN_FLIST_FLD_COPY(job_iflistp, MSO_FLD_CR_ADJ_DATE, task_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
					PIN_FLIST_FLD_COPY(job_iflistp, PIN_FLD_COUNTRY, task_flistp, PIN_FLD_COUNTRY, ebufp);
					PIN_FLIST_FLD_COPY(job_iflistp, MSO_FLD_LICENSE_NO, task_flistp, MSO_FLD_LICENSE_NO, ebufp);
				
				}
				else if((device_type) && (strcmp(device_type ,"VC") == 0))
				{
					job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/device/vc", -1 ,ebufp);
					PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_USERID , user_id , ebufp);
					/*Inventory Changes*/
					PIN_FLIST_FLD_COPY(job_iflistp, MSO_FLD_CR_ADJ_DATE, task_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
					PIN_FLIST_FLD_COPY(job_iflistp, PIN_FLD_COUNTRY, task_flistp, PIN_FLD_COUNTRY, ebufp);
					PIN_FLIST_FLD_COPY(job_iflistp, MSO_FLD_LICENSE_NO, task_flistp, MSO_FLD_LICENSE_NO, ebufp);
				}
				else if((device_type) && (strcmp(device_type ,"MODEM") == 0))
				{
					job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/device/modem", -1 ,ebufp);
					PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_USERID , user_id , ebufp);
					/*Inventory Changes*/
					PIN_FLIST_FLD_COPY(job_iflistp, MSO_FLD_CR_ADJ_DATE, task_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
					PIN_FLIST_FLD_COPY(job_iflistp, PIN_FLD_COUNTRY, task_flistp, PIN_FLD_COUNTRY, ebufp);
					PIN_FLIST_FLD_COPY(job_iflistp, MSO_FLD_LICENSE_NO, task_flistp, MSO_FLD_LICENSE_NO, ebufp);
					if((device_tech_type && !(strcmp(device_tech_type, HW_GPON_MODEM) == 0 ||
						strcmp(device_tech_type, HW_ONU_GPON_MODEM) == 0 ||
						strcmp(device_tech_type, JIO_GPON_MODEM) == 0 )) 
						&& (device_id && (fm_mso_integ_match_patterns("^([0-9a-fA-F]{2})(([/\\s:-][0-9a-fA-F]{2}){5})$", device_id) <=0)))
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the task object " );
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS, &status_fail, ebufp);
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
						sprintf(message, "Invalid Device ID: %s, it is NOT a MAC address.", device_id);				
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, message, ebufp);

					}
				}
				else if((device_type) && (strcmp(device_type ,"ROUTER/CABLE") == 0))
				{
					job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/device/router/cable", -1 ,ebufp);
					PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_USERID , user_id , ebufp);
					/*Inventory Changes*/
					PIN_FLIST_FLD_COPY(job_iflistp, MSO_FLD_CR_ADJ_DATE, task_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
					PIN_FLIST_FLD_COPY(job_iflistp, PIN_FLD_COUNTRY, task_flistp, PIN_FLD_COUNTRY, ebufp);
					PIN_FLIST_FLD_COPY(job_iflistp, MSO_FLD_LICENSE_NO, task_flistp, MSO_FLD_LICENSE_NO, ebufp);
					if(device_id && (fm_mso_integ_match_patterns("^([0-9a-fA-F]{2})(([/\\s:-][0-9a-fA-F]{2}){5})$", device_id) <=0) )
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the task object " );
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS, &status_fail, ebufp);
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
						sprintf(message, "Invalid Device ID: %s, it is NOT a MAC address.", device_id);				
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, message, ebufp);

					}
				
				}
				else if((device_type) && (strcmp(device_type ,"ROUTER/WIFI") == 0))
				{
					job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/device/router/wifi", -1 ,ebufp);
					PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_USERID , user_id , ebufp);
					/*Inventory Changes*/
					PIN_FLIST_FLD_COPY(job_iflistp, MSO_FLD_CR_ADJ_DATE, task_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
					PIN_FLIST_FLD_COPY(job_iflistp, PIN_FLD_COUNTRY, task_flistp, PIN_FLD_COUNTRY, ebufp);
					PIN_FLIST_FLD_COPY(job_iflistp, MSO_FLD_LICENSE_NO, task_flistp, MSO_FLD_LICENSE_NO, ebufp);
					if((device_tech_type && !(strcmp(device_tech_type, "30") == 0 || strcmp(device_tech_type, "31") == 0 ))
						&& (device_id && (fm_mso_integ_match_patterns("^([0-9a-fA-F]{2})(([/\\s:-][0-9a-fA-F]{2}){5})$", device_id) <=0 )))
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the task object " );
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS, &status_fail, ebufp);
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
						sprintf(message, "Invalid Device ID: %s, it is NOT a MAC address.", device_id);				
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, message, ebufp);

					}
				}
				else if((device_type) && (strcmp(device_type ,"NAT") == 0))
				{
					job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/device/nat", -1 ,ebufp);
					PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_USERID , user_id , ebufp);
					/*Inventory Changes*/
					PIN_FLIST_FLD_COPY(job_iflistp, MSO_FLD_CR_ADJ_DATE, task_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
					PIN_FLIST_FLD_COPY(job_iflistp, PIN_FLD_COUNTRY, task_flistp, PIN_FLD_COUNTRY, ebufp);
					PIN_FLIST_FLD_COPY(job_iflistp, MSO_FLD_LICENSE_NO, task_flistp, MSO_FLD_LICENSE_NO, ebufp);
					if(device_id && (fm_mso_integ_match_patterns("^([0-9a-fA-F]{2})(([/\\s:-][0-9a-fA-F]{2}){5})$", device_id) <=0 ))
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the task object " );
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS, &status_fail, ebufp);
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
						sprintf(message, "Invalid Device ID: %s, it is NOT a MAC address.", device_id);				
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, message, ebufp);

					}
				}
				else if((device_type) && (strcmp(device_type ,"IP/STATIC") == 0))
				{
					job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/device/ip/static", -1 ,ebufp);
					PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_USERID , user_id , ebufp);
					/*Inventory Changes*/
					PIN_FLIST_FLD_COPY(job_iflistp, MSO_FLD_CR_ADJ_DATE, task_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
					PIN_FLIST_FLD_COPY(job_iflistp, PIN_FLD_COUNTRY, task_flistp, PIN_FLD_COUNTRY, ebufp);
					PIN_FLIST_FLD_COPY(job_iflistp, MSO_FLD_LICENSE_NO, task_flistp, MSO_FLD_LICENSE_NO, ebufp);
				}
				else if((device_type) && (strcmp(device_type ,"IP/FRAMED") == 0))
				{
					job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/device/ip/framed", -1 ,ebufp);
					PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_USERID , user_id , ebufp);
					/*Inventory Changes*/
					PIN_FLIST_FLD_COPY(job_iflistp, MSO_FLD_CR_ADJ_DATE, task_flistp, MSO_FLD_CR_ADJ_DATE, ebufp);
					PIN_FLIST_FLD_COPY(job_iflistp, PIN_FLD_COUNTRY, task_flistp, PIN_FLD_COUNTRY, ebufp);
					PIN_FLIST_FLD_COPY(job_iflistp, MSO_FLD_LICENSE_NO, task_flistp, MSO_FLD_LICENSE_NO, ebufp);
				}
				nota_buf->flag   = 0;
				nota_buf->size   = flistlen - 2;
				nota_buf->offset = 0;
				nota_buf->data   = ( caddr_t)flistbuf;
				nota_buf->xbuf_file = ( char *) NULL;

				PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );

				//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Job create input flist " ,job_iflistp);
			
				        /* OAP sometimes send error in input */
			        if(input_status && (*input_status == status_fail))
        			{
                			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                			PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                			PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        			}
        			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist" ,job_iflistp);

        			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

				PCM_OP(ctxp,PCM_OP_CREATE_OBJ ,0, job_iflistp , &job_oflistp , ebufp);

				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the mso_mta job object " );
					PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
					PIN_ERRBUF_CLEAR(ebufp);
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " job create output flist is " , job_oflistp);
				PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
				if(nota_buf)
				{
					free(nota_buf);
				}
		 }
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " No task object created ");
	}
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID , order_pdp, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp,MSO_FLD_PO_NO , order_id , ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS , &order_status_sucess , ebufp);
		//PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Device loaded sucessfully", ebufp);
		task_flistp = PIN_FLIST_ELEM_ADD(*ret_flistp,MSO_FLD_TASK,0, ebufp);
		PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_ID , task_no , ebufp);

		PIN_FLIST_DESTROY_EX(&order_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&order_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
		PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
		return ;
}

static void 
fm_mso_integ_device_loaction_updater(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*task_flistp = NULL;
	pin_flist_t		*write_oflistp = NULL;
	pin_flist_t		*write_iflistp = NULL;
	pin_flist_t		*order_iflistp = NULL;
	pin_flist_t		*order_oflistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*in_flistp = NULL;

	poid_t		*order_pdp	=	NULL;
	poid_t		*task_pdp	=	NULL;
	poid_t		*user_id	= NULL;
	char		*program_name	= NULL;

	char		*device_type	= NULL;
	char		*task_no	= NULL;
	char		*order_id	= NULL;
	int64		db = 0;
	//int64		db = 1;
	int32		order_status_sucess = 0;
	int32		order_status_failure = 1;
	int			elem_id = 0;

	
	pin_cookie_t	cookie = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_device_location_update input flist", i_flistp);

	in_flistp = PIN_FLIST_COPY(i_flistp , ebufp);

	order_pdp	= PIN_FLIST_FLD_GET(in_flistp , PIN_FLD_ORDER_OBJ , 1 , ebufp);

	db 		= PIN_POID_GET_DB(order_pdp);

	task_no		= PIN_FLIST_FLD_GET(in_flistp , MSO_FLD_TASK_NAME , 1, ebufp);

	order_id	= PIN_FLIST_FLD_GET(in_flistp ,PIN_FLD_ORDER_ID , 1, ebufp);

	user_id		= PIN_FLIST_FLD_GET(in_flistp ,PIN_FLD_USERID , 1, ebufp);
	program_name	= PIN_FLIST_FLD_GET(in_flistp ,PIN_FLD_PROGRAM_NAME, 1 , ebufp);

	device_type = PIN_FLIST_FLD_GET(in_flistp ,PIN_FLD_DEVICE_TYPE, 1 , ebufp);

	//account_no = PIN_FLIST_FLD_GET(in_flistp , PIN_FLD_ACCOUNT_NO , 1 , ebufp);

	/******
	 * Create the task object in the brm 
	 *****/

	order_iflistp = PIN_FLIST_CREATE(ebufp);

	if((device_type) && (strcmp(device_type , "STB") == 0))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/location_update/stb", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "STB_Device_location_update" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_USERID , user_id , ebufp);
	}
	else if ((device_type) && (strcmp(device_type , "VC") == 0))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/location_update/vc", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "VC_Device_location_update" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_USERID , user_id , ebufp);
	}
	else if ((device_type) && (strcmp(device_type , "MODEM") == 0))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/location_update/modem", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Modem_Device_location_update" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
	}
	else if ((device_type) && (strcmp(device_type , "ROUTER/CABLE") == 0))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/location_update/router/cable", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Cable_Router_Device_location_update" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
	}
	else if ((device_type) && (strcmp(device_type , "ROUTER/WIFI") == 0))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/location_update/router/wifi", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "WIFI_Router_Device_location_update" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
	}
	else if ((device_type) && (strcmp(device_type , "NAT") == 0))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/location_update/nat", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "NAT_Device_location_update" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
	}
	else if ((device_type) && (strcmp(device_type , "IP/STATIC") == 0))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/location_update/ip/static", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Static_IP_Device_location_update" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
	}
	else if ((device_type) && (strcmp(device_type , "IP/FRAMED") == 0))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/location_update/ip/framed", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Framed_IP_Device_location_update" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create for device updater input flist is " , order_iflistp);
	PCM_OP(ctxp,PCM_OP_CREATE_OBJ,0 , order_iflistp , &order_oflistp , ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the task object " );
		PIN_FLIST_DESTROY_EX(&order_oflistp , NULL);
		PIN_FLIST_DESTROY_EX(&order_iflistp , NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_create_job task object for device location update failed", ebufp);
		*ret_flistp = r_flistp;
		//return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create output for device location update flist is " , order_oflistp);
	
	if(order_oflistp)
	{
		task_pdp = PIN_FLIST_FLD_GET(order_oflistp , PIN_FLD_POID , 1, ebufp);
	
		/******
		*Read all the task arrays from the input and prepare the mso_mta_jobs
		*****/

		 while ((task_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp,MSO_FLD_TASK, &elem_id, 1, &cookie, ebufp))!= NULL)
		 {
			PIN_FLIST_FLD_SET(task_flistp,MSO_FLD_ORDER_OBJ ,order_pdp , ebufp); 
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_POID, order_pdp , ebufp); 
			PIN_FLIST_FLD_SET(task_flistp,PIN_FLD_DEVICE_TYPE,device_type,ebufp);
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_USERID , user_id , ebufp);
			fm_mso_integ_create_location_update_job(ctxp,task_no,task_pdp,task_flistp, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"error in creating the job object for location update ");
				PIN_ERRBUF_CLEAR(ebufp);
			}
		 }
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "No task created");
		return;
	}
	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID , order_pdp, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp,PIN_FLD_ORDER_ID , order_id , ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS , &order_status_sucess , ebufp);
	//PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Device location updated sucessfully", ebufp);
	task_flistp = PIN_FLIST_ELEM_ADD(*ret_flistp,MSO_FLD_TASK,0, ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_ID , task_no , ebufp);

	PIN_FLIST_DESTROY_EX(&order_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&order_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
	return ;
}

static void 
fm_mso_integ_device_attribute_updater(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*task_flistp = NULL;
	pin_flist_t		*write_oflistp = NULL;
	pin_flist_t		*write_iflistp = NULL;
	pin_flist_t		*order_iflistp = NULL;
	pin_flist_t		*order_oflistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*in_flistp = NULL;

	poid_t		*order_pdp	=	NULL;
	poid_t		*task_pdp	=	NULL;

	char		*device_type	= NULL;
	char		*task_no		= NULL;
	char		*order_id		= NULL;
	int64		db = 0;
	//int64		db = 1;
	int32		order_status_sucess = 0;
	int32		order_status_failure = 1;
	int			elem_id = 0;

	
	pin_cookie_t	cookie = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_device_attribute_update input flist", i_flistp);

	in_flistp = PIN_FLIST_COPY(i_flistp , ebufp);

	order_pdp	= PIN_FLIST_FLD_GET(in_flistp , PIN_FLD_ORDER_OBJ , 1 , ebufp);

	db		= PIN_POID_GET_DB(order_pdp);

	task_no		= PIN_FLIST_FLD_GET(in_flistp , MSO_FLD_TASK_NAME , 1, ebufp);

	order_id	= PIN_FLIST_FLD_GET(in_flistp ,PIN_FLD_ORDER_ID , 1, ebufp);

	device_type = PIN_FLIST_FLD_GET(in_flistp ,PIN_FLD_DEVICE_TYPE, 1 , ebufp);

	//account_no = PIN_FLIST_FLD_GET(in_flistp , PIN_FLD_ACCOUNT_NO , 1 , ebufp);

	/******
	 * Create the task object in the brm 
	 *****/

	order_iflistp = PIN_FLIST_CREATE(ebufp);

	if((device_type) && (strcmp(device_type , "STB") == 0))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/attribute_update/stb", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "STB_Device_attribute_update" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
	}
	else if ((device_type) && (strcmp(device_type , "VC") == 0))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/attribute_update/vc", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "VC_Device_attribute_update" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
	}
	else if ((device_type) && (strcmp(device_type , "MODEM") == 0))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/attribute_update/modem", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Modem_Device_attribute_update" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
	}
	else if ((device_type) && (strcmp(device_type , "ROUTER/CABLE") == 0))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/attribute_update/router/cable", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Cable_Router_Device_attribute_update" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
	}
	else if ((device_type) && (strcmp(device_type , "ROUTER/WIFI") == 0))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/attribute_update/router/wifi", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "WIFI_Router_Device_attribute_update" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
	}
	else if ((device_type) && (strcmp(device_type , "NAT") == 0))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/attribute_update/nat", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "NAT_Device_attribute_update" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
	}
	else if ((device_type) && (strcmp(device_type , "IP/STATIC") == 0))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/attribute_update/ip/static", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Static_IP_Device_attribute_update" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
	}
	else if ((device_type) && (strcmp(device_type , "IP/FRAMED") == 0))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/attribute_update/ip/framed", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Framed_IP_Device_attribute_update" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create for device updater input flist is " , order_iflistp);
	PCM_OP(ctxp,PCM_OP_CREATE_OBJ,0 , order_iflistp , &order_oflistp , ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the task object " );
		PIN_FLIST_DESTROY_EX(&order_oflistp , NULL);
		PIN_FLIST_DESTROY_EX(&order_iflistp , NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_create_job task object for device attribute update failed", ebufp);
		*ret_flistp = r_flistp;
		//return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create output for device attribute update flist is " , order_oflistp);
	
	if(order_oflistp)
	{
		task_pdp = PIN_FLIST_FLD_GET(order_oflistp , PIN_FLD_POID , 1, ebufp);
	
		/******
		*Read all the task arrays from the input and prepare the mso_mta_jobs
		*****/

		 while ((task_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp,MSO_FLD_TASK, &elem_id, 1, &cookie, ebufp))!= NULL)
		 {
			PIN_FLIST_FLD_SET(task_flistp,PIN_FLD_POID,order_pdp , ebufp); 
			PIN_FLIST_FLD_SET(task_flistp,PIN_FLD_DEVICE_TYPE,device_type,ebufp);
			PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, task_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
			fm_mso_integ_create_attribute_update_job(ctxp,task_no,task_pdp,task_flistp, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"error in creating the job object for attribute update ");
				PIN_ERRBUF_CLEAR(ebufp);
			}
		 }
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "No task created");
	}
	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID , order_pdp, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp,PIN_FLD_ORDER_ID , order_id , ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS , &order_status_sucess , ebufp);
	//PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Device attribute updated sucessfully", ebufp);
	task_flistp = PIN_FLIST_ELEM_ADD(*ret_flistp,MSO_FLD_TASK,0, ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_ID , task_no , ebufp);

	PIN_FLIST_DESTROY_EX(&order_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&order_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
	return ;
}

static void 
fm_mso_integ_device_ippool_updater(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*task_flistp = NULL;
	pin_flist_t		*write_oflistp = NULL;
	pin_flist_t		*write_iflistp = NULL;
	pin_flist_t		*order_iflistp = NULL;
	pin_flist_t		*order_oflistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*in_flistp = NULL;

	poid_t		*order_pdp	=	NULL;
	poid_t		*task_pdp	=	NULL;

	char		*device_type	= NULL;
	char		*task_no		= NULL;
	char		*order_id		= NULL;
	int64		db = 0;
	//int64		db = 1;
	int32		order_status_sucess = 0;
	int32		order_status_failure = 1;
	int			elem_id = 0;

	
	pin_cookie_t	cookie = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_device_ippool_update input flist", i_flistp);

	in_flistp = PIN_FLIST_COPY(i_flistp , ebufp);

	order_pdp	= PIN_FLIST_FLD_GET(in_flistp , PIN_FLD_ORDER_OBJ , 1 , ebufp);
	
	db		= PIN_POID_GET_DB(order_pdp);

	task_no		= PIN_FLIST_FLD_GET(in_flistp , MSO_FLD_TASK_NAME , 1, ebufp);

	order_id	= PIN_FLIST_FLD_GET(in_flistp ,PIN_FLD_ORDER_ID , 1, ebufp);

	device_type = PIN_FLIST_FLD_GET(in_flistp ,PIN_FLD_DEVICE_TYPE, 1 , ebufp);

	//account_no = PIN_FLIST_FLD_GET(in_flistp , PIN_FLD_ACCOUNT_NO , 1 , ebufp);

	/******
	 * Create the task object in the brm 
	 *****/

	order_iflistp = PIN_FLIST_CREATE(ebufp);

	if ((device_type) && (strcmp(device_type , "IP/STATIC") == 0))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/attribute_update/ip/static", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Static_IP_Device_ippool_update" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
	}
	else if ((device_type) && (strcmp(device_type , "IP/FRAMED") == 0))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/attribute_update/ip/framed", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Framed_IP_Device_ippool_update" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
	}
	else 
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error Device Type" );
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error Device Type", ebufp);
		*ret_flistp = r_flistp;
		return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create for device updater input flist is " , order_iflistp);
	PCM_OP(ctxp,PCM_OP_CREATE_OBJ,0 , order_iflistp , &order_oflistp , ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the task object " );
		PIN_FLIST_DESTROY_EX(&order_oflistp , NULL);
		PIN_FLIST_DESTROY_EX(&order_iflistp , NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_create_job task object for device ippool update failed", ebufp);
		*ret_flistp = r_flistp;
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create output for device ippool update flist is " , order_oflistp);
	
	if(order_oflistp)
	{
		task_pdp = PIN_FLIST_FLD_GET(order_oflistp , PIN_FLD_POID , 1, ebufp);
	
		/******
		*Read all the task arrays from the input and prepare the mso_mta_jobs
		*****/

		 while ((task_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp,MSO_FLD_TASK, &elem_id, 1, &cookie, ebufp))!= NULL)
		 {
			PIN_FLIST_FLD_SET(task_flistp,MSO_FLD_ORDER_OBJ ,order_pdp , ebufp); 
			PIN_FLIST_FLD_SET(task_flistp,PIN_FLD_DEVICE_TYPE,device_type,ebufp);
			fm_mso_integ_create_ippool_update_job(ctxp,task_no,task_pdp,task_flistp, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"error in creating the job object for ippool update ");
				PIN_ERRBUF_CLEAR(ebufp);
			}
		 }
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "No task created");
	}
	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID , order_pdp, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp,PIN_FLD_ORDER_ID , order_id , ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS , &order_status_sucess , ebufp);
	//PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Device ippool updated sucessfully", ebufp);
	task_flistp = PIN_FLIST_ELEM_ADD(*ret_flistp,MSO_FLD_TASK,0, ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_ID , task_no , ebufp);

	PIN_FLIST_DESTROY_EX(&order_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&order_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
	return ;
}

static void 
fm_mso_integ_device_grv_uploader(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*task_flistp = NULL;
	pin_flist_t		*write_oflistp = NULL;
	pin_flist_t		*write_iflistp = NULL;
	pin_flist_t		*order_iflistp = NULL;
	pin_flist_t		*order_oflistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*in_flistp = NULL;

	poid_t		*order_pdp	=	NULL;
	poid_t		*task_pdp	=	NULL;

	char		*device_type	= NULL;
	char		*task_no		= NULL;
	char		*order_id		= NULL;
	int64		db = 0;
	//int64		db = 1;
	int32		order_status_sucess = 0;
	int32		order_status_failure = 1;
	int			elem_id = 0;

	
	pin_cookie_t	cookie = NULL;

	pin_flist_t	*seq_o_flistp = NULL;
	pin_flist_t	*seq_flistp = NULL;	

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_device_grv_uploader input flist", i_flistp);

	in_flistp = PIN_FLIST_COPY(i_flistp , ebufp);

	order_pdp	= PIN_FLIST_FLD_GET(in_flistp , PIN_FLD_ORDER_OBJ , 1 , ebufp);
	db		= PIN_POID_GET_DB(order_pdp);
	task_no		= PIN_FLIST_FLD_GET(in_flistp , MSO_FLD_TASK_NAME , 1, ebufp);
	order_id	= PIN_FLIST_FLD_GET(in_flistp ,PIN_FLD_ORDER_ID , 1, ebufp);

	//device_type = PIN_FLIST_FLD_GET(in_flistp ,PIN_FLD_DEVICE_TYPE, 1 , ebufp);

	//account_no = PIN_FLIST_FLD_GET(in_flistp , PIN_FLD_ACCOUNT_NO , 1 , ebufp);

       /***** Pavan Bellala 06-11-2015 ********
        Added GRV reference no. from sequence
        ***************************************/	
	while ((task_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp,MSO_FLD_TASK, &elem_id, 1, &cookie, ebufp))!= NULL)
	{
		seq_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, seq_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(seq_flistp, PIN_FLD_NAME, "MSO_GRV_SEQUENCE",ebufp);
    		//call the function to read the grv order sequence
    		fm_mso_utils_sequence_no(ctxp, seq_flistp, &seq_o_flistp, ebufp);	
		PIN_FLIST_DESTROY_EX(&seq_flistp, NULL);
		PIN_FLIST_FLD_MOVE(seq_o_flistp, PIN_FLD_STRING, task_flistp, MSO_FLD_REF_NO, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51040", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, 
						"fm_mso_integ_create_job - create ref id for device grv uploader failed", ebufp);
			*ret_flistp = r_flistp;
		}
	
		PIN_FLIST_DESTROY_EX(&seq_o_flistp, NULL);	
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Modified input with GRV sequence",in_flistp);

	/******
	 * Create the task object in the brm 
	 *****/

	order_iflistp = PIN_FLIST_CREATE(ebufp);

	task_pdp = PIN_POID_CREATE(db , "/mso_task/grv_uploader", -1 , ebufp);
	PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "GRV_Uploader" , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create for device updater input flist is " , order_iflistp);
	PCM_OP(ctxp,PCM_OP_CREATE_OBJ,0 , order_iflistp , &order_oflistp , ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the task object " );
		PIN_FLIST_DESTROY_EX(&order_oflistp , NULL);
		PIN_FLIST_DESTROY_EX(&order_iflistp , NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_create_job task object for device grv uploader failed", ebufp);
		*ret_flistp = r_flistp;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create output for device grv uploader flist is " , order_oflistp);
	
	if(order_oflistp)
	{
		task_pdp = PIN_FLIST_FLD_GET(order_oflistp , PIN_FLD_POID , 1, ebufp);
	
		/******
		*Read all the task arrays from the input and prepare the mso_mta_jobs
		*****/
		elem_id = 0; cookie = NULL;
		 while ((task_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp,MSO_FLD_TASK, &elem_id, 1, &cookie, ebufp))!= NULL)
		 {
			PIN_FLIST_FLD_SET(task_flistp,MSO_FLD_ORDER_OBJ ,order_pdp , ebufp); 
			PIN_FLIST_FLD_SET(task_flistp,PIN_FLD_POID ,order_pdp , ebufp); 
			PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, task_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
			//PIN_FLIST_FLD_SET(task_flistp,PIN_FLD_DEVICE_TYPE,device_type,ebufp);
			fm_mso_integ_create_grv_uploader_job(ctxp,task_no,task_pdp,task_flistp, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"error in creating the job object for grv uploader ");
				PIN_ERRBUF_CLEAR(ebufp);
			}
		 }
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "No task created");
	}
	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID , order_pdp, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp,PIN_FLD_ORDER_ID , order_id , ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS , &order_status_sucess , ebufp);
	//PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Device grv uploaded sucessfully", ebufp);
	task_flistp = PIN_FLIST_ELEM_ADD(*ret_flistp,MSO_FLD_TASK,0, ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_ID , task_no , ebufp);

	PIN_FLIST_DESTROY_EX(&order_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&order_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
	return ;
}

static void 
fm_mso_integ_device_pair(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*task_flistp = NULL;
	pin_flist_t		*write_oflistp = NULL;
	pin_flist_t		*write_iflistp = NULL;
	pin_flist_t		*order_iflistp = NULL;
	pin_flist_t		*order_oflistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*in_flistp = NULL;

	poid_t		*order_pdp	=	NULL;
	poid_t		*task_pdp	=	NULL;

	char		*device_type	= NULL;
	char		*task_no		= NULL;
	char		*order_id		= NULL;
	int64		db = 0;
	//int64		db = 1;
	int32		order_status_sucess = 0;
	int32		order_status_failure = 1;
	int			elem_id = 0;

	
	pin_cookie_t	cookie = NULL;

	pin_flist_t	*seq_o_flistp = NULL;
	pin_flist_t	*seq_flistp = NULL;	

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_device_pair input flist", i_flistp);

	in_flistp = PIN_FLIST_COPY(i_flistp , ebufp);

	order_pdp	= PIN_FLIST_FLD_GET(in_flistp , PIN_FLD_ORDER_OBJ , 1 , ebufp);
	db		= PIN_POID_GET_DB(order_pdp);
	task_no		= PIN_FLIST_FLD_GET(in_flistp , MSO_FLD_TASK_NAME , 1, ebufp);
	order_id	= PIN_FLIST_FLD_GET(in_flistp ,PIN_FLD_ORDER_ID , 1, ebufp);

	//device_type = PIN_FLIST_FLD_GET(in_flistp ,PIN_FLD_DEVICE_TYPE, 1 , ebufp);

	//account_no = PIN_FLIST_FLD_GET(in_flistp , PIN_FLD_ACCOUNT_NO , 1 , ebufp);

	/******
	 * Create the task object in the brm 
	 *****/

	order_iflistp = PIN_FLIST_CREATE(ebufp);

	task_pdp = PIN_POID_CREATE(db , "/mso_task/device_pair", -1 , ebufp);
	PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Device Pairing" , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create for device pair input flist is " , order_iflistp);
	PCM_OP(ctxp,PCM_OP_CREATE_OBJ,0 , order_iflistp , &order_oflistp , ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the task object " );
		PIN_FLIST_DESTROY_EX(&order_oflistp , NULL);
		PIN_FLIST_DESTROY_EX(&order_iflistp , NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_create_job task object for device pair failed", ebufp);
		*ret_flistp = r_flistp;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create output for device pair flist is " , order_oflistp);
	
	if(order_oflistp)
	{
		task_pdp = PIN_FLIST_FLD_GET(order_oflistp , PIN_FLD_POID , 1, ebufp);
	
		/******
		*Read all the task arrays from the input and prepare the mso_mta_jobs
		*****/
		elem_id = 0; cookie = NULL;
		 while ((task_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp,MSO_FLD_TASK, &elem_id, 1, &cookie, ebufp))!= NULL)
		 {
			PIN_FLIST_FLD_SET(task_flistp,MSO_FLD_ORDER_OBJ ,order_pdp , ebufp); 
			PIN_FLIST_FLD_SET(task_flistp,PIN_FLD_POID ,order_pdp , ebufp); 
			PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, task_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
			//PIN_FLIST_FLD_SET(task_flistp,PIN_FLD_DEVICE_TYPE,device_type,ebufp);
			fm_mso_integ_create_device_pair_job(ctxp,task_no,task_pdp,task_flistp, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"error in creating the job object for device pairing");
				PIN_ERRBUF_CLEAR(ebufp);
			}
		 }
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "No task created");
	}
	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID , order_pdp, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp,PIN_FLD_ORDER_ID , order_id , ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS , &order_status_sucess , ebufp);
	//PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Device grv uploaded sucessfully", ebufp);
	task_flistp = PIN_FLIST_ELEM_ADD(*ret_flistp,MSO_FLD_TASK,0, ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_ID , task_no , ebufp);

	PIN_FLIST_DESTROY_EX(&order_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&order_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
	return ;
}


static void 
fm_mso_integ_device_state_update(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*task_flistp = NULL;
	pin_flist_t		*write_oflistp = NULL;
	pin_flist_t		*write_iflistp = NULL;
	pin_flist_t		*order_iflistp = NULL;
	pin_flist_t		*order_oflistp = NULL;
	pin_flist_t		*r_flistp = NULL;

	poid_t		*order_pdp	=	NULL;
	poid_t		*task_pdp	=	NULL;
	
	char		*device_type	= NULL;
	char		*task_no		= NULL;
	char		*order_id		= NULL;
	poid_t		*user_id	= NULL;
	char		*program_name	= NULL;
	int64		db = 0;	
	//int64		db = 1;
	int32		order_status_sucess = 0;
	int32		order_status_failure = 1;
	int			elem_id = 0;
	int32		status = 0;

		
	pin_cookie_t	cookie = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_device_state_update input flist", i_flistp);

	order_pdp	= PIN_FLIST_FLD_GET(i_flistp , PIN_FLD_ORDER_OBJ , 1 , ebufp);
	
	db		= PIN_POID_GET_DB(order_pdp);

	task_no		= PIN_FLIST_FLD_GET(i_flistp , MSO_FLD_TASK_NAME , 1, ebufp);

	order_id	= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_ORDER_ID , 1, ebufp);

	device_type = PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_DEVICE_TYPE, 1 , ebufp);

	user_id		= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_USERID , 1, ebufp);
	program_name	= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_PROGRAM_NAME, 1 , ebufp);

	/******
	 * Create the task object in the brm 
	 *****/

	order_iflistp = PIN_FLIST_CREATE(ebufp);

	if((device_type) && (strcmp(device_type , "STB")) == 0)
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/state_change/stb", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Bulk STB_Device_state_change" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_USERID , user_id , ebufp);
	}
	else if ((device_type) && (strcmp(device_type , "VC")) == 0)
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/state_change/vc", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , " Bulk VC_Device_state_change" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_USERID , user_id , ebufp);
	}
	else if ((device_type) && (strcmp(device_type , "MODEM")) == 0)
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/state_change/modem", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , " Bulk Modem_Device_state_change" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
	}
	else if ((device_type) && (strcmp(device_type , "ROUTER/CABLE")) == 0)
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/state_change/router/cable", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , " Bulk Cable_Router_Device_state_change" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
	}
	else if ((device_type) && (strcmp(device_type , "ROUTER/WIFI")) == 0)
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/state_change/router/wifi", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , " Bulk WIFI_Router_Device_state_change" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
	}
	else if ((device_type) && (strcmp(device_type , "NAT")) == 0)
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/state_change/nat", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , " Bulk NAT_Device_state_change" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
	}
	else if ((device_type) && (strcmp(device_type , "IP/STATIC")) == 0)
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/state_change/ip/static", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , " Bulk Static_IP_Device_state_change" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
	}
	else if ((device_type) && (strcmp(device_type , "IP/FRAMED")) == 0)
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/state_change/ip/framed", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , " Bulk Framed_IP_Device_state_change" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create for device state updater input flist is " , order_iflistp);
	PCM_OP(ctxp,PCM_OP_CREATE_OBJ,0 , order_iflistp , &order_oflistp , ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the task object " );
		PIN_FLIST_DESTROY_EX(&order_oflistp , NULL);
		PIN_FLIST_DESTROY_EX(&order_iflistp , NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_create_job task object for device state update failed", ebufp);
		*ret_flistp = r_flistp;
		//return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create output for device state update flist is " , order_oflistp);
		
	if(order_oflistp)
	{
		task_pdp = PIN_FLIST_FLD_GET(order_oflistp , PIN_FLD_POID , 1, ebufp);
		
		/******
		 *Read all the task arrays from the input and prepare the mso_mta_jobs
		 *****/
		 while ((task_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp,MSO_FLD_TASK, &elem_id, 1, &cookie, ebufp))!= NULL)
		 {
			PIN_FLIST_FLD_SET(task_flistp,MSO_FLD_ORDER_OBJ ,order_pdp , ebufp);
//			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_POID,order_pdp, ebufp);
			PIN_FLIST_FLD_SET(task_flistp,PIN_FLD_DEVICE_TYPE,device_type,ebufp);
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_USERID , user_id , ebufp);
			fm_mso_integ_create_state_change_job(ctxp,task_no,task_pdp,task_flistp,ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error while creating the state change job poid");
				PIN_ERRBUF_CLEAR(ebufp);
			}
			
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " No Task created ");
	}
	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID , order_pdp, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp,PIN_FLD_ORDER_ID , order_id , ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS , &order_status_sucess , ebufp);
	//PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Device state updated sucessfully", ebufp);
	task_flistp = PIN_FLIST_ELEM_ADD(*ret_flistp,MSO_FLD_TASK,0, ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_ID , task_no , ebufp);

	PIN_FLIST_DESTROY_EX(&order_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&order_oflistp, NULL);
	return ;
}

EXPORT_OP void
fm_mso_search_lco_account(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_errbuf_t	*ebufp)
{
		
		pin_flist_t		*args_flistp = NULL;
		pin_flist_t		*search_inflistp = NULL;
		pin_flist_t		*search_outflistp = NULL;
		pin_flist_t		*results_flistp = NULL;

		char		*account_no = NULL;		
		char		search_template[100] = " select X from /account where F1 = V1 ";
		int32			*business_typep = NULL;
		int			search_flags = 768;
		int64		db = 1;
		//int64		db = 0;
		poid_t		*account_pdp = NULL;
		poid_t		*pdp = NULL;
		if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_lco_account input flist", i_flistp);
		
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
		}

		account_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp);
		if (!account_no)
		{
			account_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SOURCE, 1, ebufp);
		}

		/*************
		 * search flist to search account poid
		 ************/

		search_inflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
		PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
		results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
		PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID,NULL , ebufp);
		PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_BUSINESS_TYPE, NULL , ebufp);
		PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_AAC_PROMO_CODE, NULL , ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_lco_account search account input list", search_inflistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
		
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
				PIN_ERRBUF_CLEAR(ebufp);
				PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_lco_account search account output flist", search_outflistp);

		if(PIN_FLIST_ELEM_COUNT(search_outflistp , PIN_FLD_RESULTS , ebufp) > 0)
		{
			results_flistp = PIN_FLIST_ELEM_GET(search_outflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
			account_pdp = PIN_FLIST_FLD_GET(results_flistp , PIN_FLD_POID, 1, ebufp);
			business_typep = PIN_FLIST_FLD_GET(results_flistp , PIN_FLD_BUSINESS_TYPE, 1, ebufp);;

			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_ACCOUNT_OBJ , account_pdp , ebufp);
			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_BUSINESS_TYPE, business_typep, ebufp);
			PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_AAC_PROMO_CODE, i_flistp, PIN_FLD_AAC_PROMO_CODE, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_lco_account search return flist", i_flistp);
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Account object not found");
			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_ACCOUNT_OBJ , NULL , ebufp);
		}
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
}

extern void
fm_mso_search_device_object(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_errbuf_t	*ebufp)
{
		
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*search_inflistp = NULL;
	pin_flist_t	*search_outflistp = NULL;
	pin_flist_t	*results_flistp = NULL;
	pin_flist_t	*services_flistp = NULL;
	
	char		*device_id = NULL;		
	char		search_template[100] = "";
	char            checkstr[3];
	char		*den_nwp = "NDS1";
	char		*dsn_nwp = "NDS2";
	char		*hw_nwp = "NDS";
	int		search_flags = 768;
	int             agreementnoflag = 0;
	int64		db = 1;
	int32		den_sp = 0;
	//int64		db = 0;
	poid_t		*device_pdp = NULL;
	poid_t		*service_pdp = NULL;
	poid_t		*account_pdp = NULL;
	poid_t		*pdp = NULL;
	void		*vp = NULL;
	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_device_object input flist", i_flistp);

	den_sp = fm_mso_is_den_sp(ctxp, i_flistp, ebufp);

	device_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEVICE_ID , 1, ebufp);
	memset(checkstr, '\0', sizeof(checkstr));

        if(device_id){
            strncpy(checkstr, device_id, 2);
            if(strcmp(checkstr, "BB") == 0){
               agreementnoflag = 1;
            }
        }

	/*************
	 * search flist to search account poid
	 ************/
		// For ETHERNET, since MAC ID is optional, we send an agreement no for DEVICE_ID
	if(device_id && (agreementnoflag == 1)){
		fm_mso_get_bb_service_agreement_no(ctxp, device_id, &i_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_device_object search agreement flist", i_flistp);
	}
	else{
		search_inflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DEVICE_ID, device_id, ebufp);
		if (strncmp(device_id, "000", 3) == 0)
		{
			args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp);	
			if (den_sp == 1)
			{
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_MANUFACTURER, den_nwp, ebufp);
			}
			else if (den_sp == 2)
			{
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_MANUFACTURER, dsn_nwp, ebufp);
			}
			else
			{
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_MANUFACTURER, hw_nwp, ebufp);
			}
			sprintf(search_template, "%s", "select X from /device where F1 = V1 and F2 = V2 ");
		}
		else
		{
			sprintf(search_template, "%s", "select X from /device where F1 = V1 ");
		}
		PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DEVICE_ID, device_id, ebufp);
		results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_device_object search device input list", search_inflistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
		
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_device_object search device output flist", search_outflistp);

		if(PIN_FLIST_ELEM_COUNT(search_outflistp , PIN_FLD_RESULTS , ebufp) > 0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Device object found");

			results_flistp = PIN_FLIST_ELEM_GET(search_outflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
			device_pdp = PIN_FLIST_FLD_GET(results_flistp , PIN_FLD_POID , 1, ebufp);

			services_flistp = PIN_FLIST_ELEM_GET(results_flistp, PIN_FLD_SERVICES , PIN_ELEMID_ANY, 1, ebufp);
			if(services_flistp)
			{
			  service_pdp = PIN_FLIST_FLD_GET(services_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
			  account_pdp = PIN_FLIST_FLD_GET(services_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
			}
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERRBUF_CLEAR(ebufp);
			}

			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_DEVICE_OBJ , device_pdp , ebufp);
			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_SERVICE_OBJ , service_pdp , ebufp);
			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_ACCOUNT_OBJ , account_pdp , ebufp);

			PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_STATE_ID, i_flistp, PIN_FLD_OLD_STATE, ebufp);

			PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_ACCOUNT_OBJ, i_flistp, MSO_FLD_STPK_PLAN_OBJ, ebufp);

            /*Below changes done for fixing repair counter issue- Chandrakala K*/

            PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_SOURCE, i_flistp, PIN_FLD_SOURCE, ebufp);

            PIN_FLIST_FLD_COPY(results_flistp, MSO_FLD_WARRANTY_END, i_flistp, MSO_FLD_WARRANTY_END, ebufp);

            PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_COUNT, i_flistp, PIN_FLD_COUNT, ebufp);

			/*vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp);
			if (vp)
			{
				PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_SOURCE, i_flistp, PIN_FLD_SOURCE, ebufp);
			}

			vp = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_WARRANTY_END, 1, ebufp);
			if (vp)
			{
				PIN_FLIST_FLD_COPY(results_flistp, MSO_FLD_WARRANTY_END, i_flistp, MSO_FLD_WARRANTY_END, ebufp);
			}

			vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_COUNT, 1, ebufp);
			if (vp)
			{
				PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_COUNT, i_flistp, PIN_FLD_COUNT, ebufp);
			}*/

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_device_object search return flist", i_flistp);
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Device object not found");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " 1");
			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_DEVICE_OBJ , NULL , ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " 2");
			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_SERVICE_OBJ , NULL , ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " 3");
		}

		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	}
	return;
}

EXPORT_OP void
fm_mso_search_device_poid(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_errbuf_t	*ebufp)
{
		
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*search_inflistp = NULL;
	pin_flist_t	*search_outflistp = NULL;
	pin_flist_t	*results_flistp = NULL;
	
	char		*device_id = NULL;		
	char		search_template[100] = " select X from /device where F1 = V1 ";
	int		search_flags = 768;
	int64		db = 1;
	//int64		db = 0;
	poid_t		*pdp = NULL;
	poid_t		*device_pdp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_device_poid input flist", i_flistp);

	device_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEVICE_ID , 1, ebufp);
	
	

	/*************
	 * search flist to search device poid
	 ************/
	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DEVICE_ID, device_id, ebufp);
	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_device_poid search device input list", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_device_poid search device output flist", search_outflistp);

	if(PIN_FLIST_ELEM_COUNT(search_outflistp , PIN_FLD_RESULTS , ebufp) > 0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Device object found");

		results_flistp = PIN_FLIST_ELEM_GET(search_outflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		device_pdp = PIN_FLIST_FLD_GET(results_flistp , PIN_FLD_POID , 1, ebufp);

		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
		}

		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_DEVICE_OBJ , device_pdp , ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_device_poid search return flist", i_flistp);
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Device object not found");
		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_DEVICE_OBJ , NULL , ebufp);
		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_SERVICE_OBJ , NULL , ebufp);
	}

	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
}

static void
fm_mso_get_service_agreement_no(
	pcm_context_t	*ctxp,
	poid_t			*service_pdp,
	pin_flist_t		*i_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*search_inflistp = NULL;
	pin_flist_t	*search_outflistp = NULL;
	pin_flist_t	*results_flistp = NULL;
	pin_flist_t	*services_flistp = NULL;
	pin_flist_t	*catv_info_flist = NULL;
	
	char		search_template[100] = " select X from /service where F1 = V1 ";
	char		*agreement_no	= NULL;
	int		search_flags = 768;
	//int64		db = 1;
	int64		db = 0;
	poid_t		*device_pdp = NULL;
	poid_t		*service_poidp = NULL;
	char 		*city = NULL;
	int 		*technologyp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_service_agreement_no input flist", i_flistp);

	/*************
	 * search flist to search account poid
	 ************/
	db = PIN_POID_GET_DB(service_pdp);
	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, service_pdp, ebufp);
	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_service_agreement_no search service input list", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_service_agreement_no search device output flist", search_outflistp);
	if(PIN_FLIST_ELEM_COUNT(search_outflistp , PIN_FLD_RESULTS , ebufp) > 0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " service object found");

		results_flistp = PIN_FLIST_ELEM_GET(search_outflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);

		
		if(strstr((char*)PIN_POID_GET_TYPE(service_pdp),"catv"))
		{
			catv_info_flist = PIN_FLIST_SUBSTR_GET(results_flistp,MSO_FLD_CATV_INFO,1,ebufp);
		}
		else if (strstr((char*)PIN_POID_GET_TYPE(service_pdp),"broadband"))
		{
			catv_info_flist = PIN_FLIST_SUBSTR_GET(results_flistp,MSO_FLD_BB_INFO,1,ebufp);
		}
		
		if(catv_info_flist)
		{
			agreement_no = PIN_FLIST_FLD_GET(catv_info_flist, MSO_FLD_AGREEMENT_NO, 1, ebufp);
			PIN_FLIST_FLD_SET(i_flistp, MSO_FLD_AGREEMENT_NO ,agreement_no , ebufp); 

			/* changes as a part of BULK CMTS CHANGE START */
			if (strstr((char*)PIN_POID_GET_TYPE(service_pdp),"broadband"))
			{
				city = PIN_FLIST_FLD_GET(catv_info_flist, PIN_FLD_CITY, 1, ebufp);
				if(city)
					PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CITY, city, ebufp); 
				technologyp = PIN_FLIST_FLD_GET(catv_info_flist, MSO_FLD_TECHNOLOGY, 1, ebufp);
				if(technologyp)
					PIN_FLIST_FLD_SET(i_flistp, MSO_FLD_TECHNOLOGY, technologyp, ebufp);
			}
			/* changes as a part of BULK CMTS CHANGE END  */
		
		}
		else
		{
			PIN_FLIST_FLD_SET(i_flistp, MSO_FLD_AGREEMENT_NO , NULL , ebufp);

		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_service_agreement_no search return flist", i_flistp);
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " agreement number not found");
		PIN_FLIST_FLD_SET(i_flistp, MSO_FLD_AGREEMENT_NO , NULL , ebufp);
		
	}

	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
}


static void
fm_mso_search_bill(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*search_inflistp = NULL;
	pin_flist_t	*search_outflistp = NULL;
	pin_flist_t	*results_flistp = NULL;
	pin_flist_t	*services_flistp = NULL;
	pin_flist_t	*catv_info_flist = NULL;
	
	char	search_template[100] = " select X from /bill where F1 = V1 ";
	char	*agreement_no	= NULL;
	char	*bill_no = NULL;
	int	search_flags = 768;
	int64	db = 1;

	poid_t	*device_pdp = NULL;
	poid_t	*service_poidp = NULL;
	poid_t	*bill_pdp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_bill input flist", i_flistp);

	bill_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BILL_NO , 1 , ebufp);

	/*************
	 * search flist to search account poid
	 ************/
	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BILL_NO, bill_no, ebufp);
	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_bill search input list", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_bill search output flist", search_outflistp);
	if(PIN_FLIST_ELEM_COUNT(search_outflistp , PIN_FLD_RESULTS , ebufp) > 0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Bill object found");

		results_flistp = PIN_FLIST_ELEM_GET(search_outflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		bill_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_POID, 1 , ebufp);
		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_POID,bill_pdp, ebufp); 
		
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " bill poid found");
		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_POID , NULL , ebufp);
		
	}

	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	return;
}

static void
fm_mso_integ_ar_get_details(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_errbuf_t	*ebufp)
{
	
	pin_flist_t	*search_inflistp = NULL;
	pin_flist_t	*search_outflistp = NULL;
	pin_flist_t	*bal_grp_flistp = NULL;
	
	int32	service_type = 1;
	int32	flags  = 3;
	
	int	elem_id = 0;
	char	*name = NULL;
	pin_cookie_t    cookie = NULL;

	poid_t	*account_pdp = NULL;
	poid_t	*bal_grp_pdp = NULL;
	

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_ar_get_details input flist", i_flistp);

	account_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ , 1 , ebufp);
	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_POID, account_pdp , ebufp);
	PIN_FLIST_FLD_SET(search_inflistp,MSO_FLD_SERVICE_TYPE , &service_type , ebufp);
	PIN_FLIST_FLD_SET(search_inflistp,PIN_FLD_FLAGS , &flags , ebufp);
	PIN_FLIST_FLD_SET(search_inflistp,PIN_FLD_BILL_NO, "1" , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ar_get_details input flist", search_inflistp);
	PCM_OP(ctxp, 13150 , 0 , search_inflistp , &search_outflistp , ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ar_get_details output flist", search_outflistp);

	if(PIN_FLIST_ELEM_COUNT(search_outflistp,PIN_FLD_RESULTS, ebufp) >= 2)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " more than 2 balance groups available");
		while ((bal_grp_flistp = PIN_FLIST_ELEM_GET_NEXT(search_outflistp,PIN_FLD_RESULTS,&elem_id, 1, &cookie, ebufp))!= NULL)
                {
			name = PIN_FLIST_FLD_GET(bal_grp_flistp, PIN_FLD_NAME , 1 , ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "name is ");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG ,name);
			if((name) && (strcmp(name , "Service Balance Group") == 0))
			{
				bal_grp_pdp = PIN_FLIST_FLD_GET(bal_grp_flistp, PIN_FLD_POID , 1 , ebufp);
			}
		}
	}
	else if(PIN_FLIST_ELEM_COUNT(search_outflistp,PIN_FLD_RESULTS, ebufp) == 1)
	{
		bal_grp_flistp = PIN_FLIST_ELEM_GET(search_outflistp, PIN_FLD_RESULTS , PIN_ELEMID_ANY, 1 , ebufp);
		bal_grp_pdp = PIN_FLIST_FLD_GET(bal_grp_flistp, PIN_FLD_POID , 1 , ebufp);
	}
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
	}

	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_POID , account_pdp , ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_BAL_GRP_OBJ,bal_grp_pdp , ebufp);
	PIN_FLIST_FLD_DROP(i_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_ar_get_details return flist is ", i_flistp);
	PIN_FLIST_DESTROY_EX(&search_outflistp , NULL);
	PIN_FLIST_DESTROY_EX(&search_inflistp , NULL);

	return;
}

static void 
fm_mso_integ_catv_pre_activation(
pcm_context_t	*ctxp,
pin_flist_t		*i_flistp,
pin_flist_t		**ret_flistp,
pin_errbuf_t	*ebufp)
{
	pin_flist_t	*task_flistp = NULL;
	pin_flist_t	*order_iflistp = NULL;
	pin_flist_t	*order_oflistp = NULL;
	pin_flist_t	*r_flistp = NULL;

	poid_t		*order_pdp	= NULL;
	poid_t		*task_pdp	= NULL;
	char		*task_no	= NULL;
	char		*order_id	= NULL;
	char		*device_type	= NULL;
	int64		db = 0;
	//int64		db = 1;
	int32		order_status_sucess = 0;
	int32		order_status_failure = 1;
	int		elem_id = 0;
	int32		status = 0;
	int32		*state_id = NULL;
	int32		status_fail = 2;
	poid_t		*user_id	= NULL;
	char		*program_name	= NULL;

	pin_cookie_t	cookie = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_catv_pre_activation input flist", i_flistp);

	order_pdp	= PIN_FLIST_FLD_GET(i_flistp , PIN_FLD_ORDER_OBJ , 1 , ebufp);
	
	db		= PIN_POID_GET_DB(order_pdp);

	task_no		= PIN_FLIST_FLD_GET(i_flistp , MSO_FLD_TASK_NAME , 1, ebufp);

	order_id	= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_ORDER_ID , 1, ebufp);

	device_type = PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_DEVICE_TYPE, 1 , ebufp);

	user_id		= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_USERID , 1, ebufp);

	program_name	= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_PROGRAM_NAME, 1 , ebufp);

	//account_no = PIN_FLIST_FLD_GET(i_flistp , PIN_FLD_ACCOUNT_NO , 1 , ebufp);

	/******
	 * Create the task object in the brm 
	 *****/

	order_iflistp = PIN_FLIST_CREATE(ebufp);

	
	task_pdp = PIN_POID_CREATE(db , "/mso_task/catv_preactivation", -1 , ebufp);
	PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "catv preactivation" , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_USERID , user_id , ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create for catv_preactivation input flist is " , order_iflistp);
	PCM_OP(ctxp,PCM_OP_CREATE_OBJ,0 , order_iflistp , &order_oflistp , ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the task object " );
		PIN_FLIST_DESTROY_EX(&order_oflistp , NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_create_job task object for catv_preactivation failed", ebufp);
		//*ret_flistp = r_flistp;
		//return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create output for catv_preactivation flist is " , order_oflistp);

	if(order_oflistp)
	{
		task_pdp = PIN_FLIST_FLD_GET(order_oflistp , PIN_FLD_POID , 1, ebufp);
		/******
		*Read all the task arrays from the input and prepare the mso_mta_jobs
		*****/

		while ((task_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp,MSO_FLD_TASK,&elem_id, 1, &cookie, ebufp))!= NULL)
		{
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_USERID , user_id , ebufp);
			fm_mso_integ_create_catv_preactivation_job(ctxp,task_no,task_pdp,task_flistp, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error while creating the catv preactivation job poid");
				PIN_ERRBUF_CLEAR(ebufp);
			}
			
		}
	}
	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID , order_pdp, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp,PIN_FLD_ORDER_ID , order_id , ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS , &order_status_sucess , ebufp);
	//PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Device state updated sucessfully", ebufp);
	task_flistp = PIN_FLIST_ELEM_ADD(*ret_flistp,MSO_FLD_TASK,0, ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_ID , task_no , ebufp);

	PIN_FLIST_DESTROY_EX(&order_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&order_oflistp, NULL);
	return ;
}

static void
fm_mso_integ_add_plan(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*task_flistp = NULL;
	pin_flist_t	*order_iflistp = NULL;
	pin_flist_t	*order_oflistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	
	poid_t		*job_pdp	=	NULL;
	poid_t		*order_pdp	=	NULL;
	poid_t		*task_pdp	=	NULL;
	char		*task_no	= NULL;
	char		*order_id	= NULL;
	int64		db = 0;
	//int64		db = 1;
	int32		order_status_sucess = 0;
	int32		order_status_failure = 1;
	int		elem_id = 0;
	int32		status = 0;
	int32		*state_id = NULL;
	int32		status_sucess = 0;
	int32		status_fail = 2;
	pin_cookie_t	cookie = NULL;
	poid_t		*user_id	= NULL;
	char		*program_name	= NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_add_plan input flist", i_flistp);

	order_pdp	= PIN_FLIST_FLD_GET(i_flistp , PIN_FLD_ORDER_OBJ , 1 , ebufp);
	
	db 		= PIN_POID_GET_DB(order_pdp);

	task_no		= PIN_FLIST_FLD_GET(i_flistp , MSO_FLD_TASK_NAME , 1, ebufp);

	order_id	= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_ORDER_ID , 1, ebufp);

	user_id		= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_USERID , 1, ebufp);

	program_name	= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_PROGRAM_NAME, 1 , ebufp);

	/******
	 * Create the task object in the brm 
	 *****/

	order_iflistp = PIN_FLIST_CREATE(ebufp);

	
	task_pdp = PIN_POID_CREATE(db , "/mso_task/bulk_add_plan", -1 , ebufp);
	PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Bulk Add plan" , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_USERID , user_id , ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create for bulk add plan input flist is " , order_iflistp);
	PCM_OP(ctxp,PCM_OP_CREATE_OBJ,0 , order_iflistp , &order_oflistp , ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the task object " );
		PIN_FLIST_DESTROY_EX(&order_oflistp , NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
		PIN_FLIST_FLD_SET(r_flistp,PIN_FLD_ERROR_DESCR,"fm_mso_integ_create_job task object for bulk add plan failed",ebufp);
		//*ret_flistp = r_flistp;
		//return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create output for bulk add plan flist is " , order_oflistp);

	if(order_oflistp)
	{
		task_pdp = PIN_FLIST_FLD_GET(order_oflistp , PIN_FLD_POID , 1, ebufp);

		while ((task_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp,MSO_FLD_TASK,&elem_id, 1, &cookie, ebufp))!= NULL)
		{
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_USERID , user_id , ebufp);
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_DESCR , "Bulk_Add_Plan" , ebufp);
			fm_mso_integ_create_add_plan_job(ctxp,task_no,task_pdp,task_flistp, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for add plan " );
				PIN_ERRBUF_CLEAR(ebufp);
			}

		}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Task object failed ");
	}
	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID , order_pdp, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp,PIN_FLD_ORDER_ID , order_id , ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS , &order_status_sucess , ebufp);
	//PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Device state updated sucessfully", ebufp);
	task_flistp = PIN_FLIST_ELEM_ADD(*ret_flistp,MSO_FLD_TASK,0, ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_ID , task_no , ebufp);

	PIN_FLIST_DESTROY_EX(&order_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&order_oflistp, NULL);
	//PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
	
	return ;

}

static void
fm_mso_integ_change_plan(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*task_flistp = NULL;
	
	pin_flist_t		*order_iflistp = NULL;
	pin_flist_t		*order_oflistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	
	poid_t		*job_pdp	=	NULL;
	poid_t		*order_pdp	=	NULL;
	poid_t		*task_pdp	=	NULL;

	char		*task_no	= NULL;
	char		*order_id	= NULL;
	poid_t		*user_id	= NULL;
	char		*program_name	= NULL;
	int64		db = 0;
	//int64		db = 1;
	int32		order_status_sucess = 0;
	int32		order_status_failure = 1;
	int		elem_id = 0;
	int32		status = 0;
	int32		*state_id = NULL;
	int32		status_sucess = 0;
	int32		status_fail = 2;
	pin_cookie_t	cookie = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_change_plan input flist", i_flistp);

	order_pdp	= PIN_FLIST_FLD_GET(i_flistp , PIN_FLD_ORDER_OBJ , 1 , ebufp);
	
	db		= PIN_POID_GET_DB(order_pdp);

	task_no		= PIN_FLIST_FLD_GET(i_flistp , MSO_FLD_TASK_NAME , 1, ebufp);

	order_id	= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_ORDER_ID , 1, ebufp);

	user_id		= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_USERID , 1, ebufp);

	program_name	= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_PROGRAM_NAME, 1 , ebufp);

	/******
	 * Create the task object in the brm 
	 *****/

	order_iflistp = PIN_FLIST_CREATE(ebufp);
	
	task_pdp = PIN_POID_CREATE(db , "/mso_task/bulk_change_plan", -1 , ebufp);
	PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Bulk change plan" , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_USERID , user_id , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create for bulk change plan input flist is " , order_iflistp);
	PCM_OP(ctxp,PCM_OP_CREATE_OBJ,0 , order_iflistp , &order_oflistp , ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the task object " );
		PIN_FLIST_DESTROY_EX(&order_oflistp , NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_create_job task object for bulk change plan failed", ebufp);
		//*ret_flistp = r_flistp;
		//return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create output for bulk change plan flist is " , order_oflistp);

	if(order_oflistp)
	{
		task_pdp = PIN_FLIST_FLD_GET(order_oflistp , PIN_FLD_POID , 1, ebufp);

		while ((task_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp,MSO_FLD_TASK,&elem_id, 1, &cookie, ebufp))!= NULL)
		{
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_USERID , user_id , ebufp);
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_DESCR , "Bulk_Change_Plan" , ebufp);
			fm_mso_integ_create_change_plan_job(ctxp,task_no,task_pdp, task_flistp , ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the change plan job object " );
				PIN_ERRBUF_CLEAR(ebufp);
			}			
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Task object failed ");
	}
	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID , order_pdp, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp,PIN_FLD_ORDER_ID , order_id , ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS , &order_status_sucess , ebufp);
	//PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Device state updated sucessfully", ebufp);
	task_flistp = PIN_FLIST_ELEM_ADD(*ret_flistp,MSO_FLD_TASK,0, ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_ID , task_no , ebufp);

	PIN_FLIST_DESTROY_EX(&order_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&order_oflistp, NULL);
	return ;
}

static void
fm_mso_integ_cancel_plan(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**ret_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*task_flistp = NULL;
	
	pin_flist_t	*order_iflistp = NULL;
	pin_flist_t	*order_oflistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	
	poid_t		*job_pdp	= NULL;
	poid_t		*order_pdp	= NULL;
	poid_t		*task_pdp	= NULL;

	char		*task_no	= NULL;
	char		*order_id	= NULL;
	int64		db = 0;
	//int64		db = 1;
	int32		order_status_sucess = 0;
	int32		order_status_failure = 1;
	int		elem_id = 0;
	int32		status = 0;
	int32		*state_id = NULL;
	int32		status_sucess = 0;
	int32		status_fail = 2;
	pin_cookie_t	cookie = NULL;
	poid_t		*user_id	= NULL;
	char		*program_name	= NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_add_plan input flist", i_flistp);

	order_pdp	= PIN_FLIST_FLD_GET(i_flistp , PIN_FLD_ORDER_OBJ , 1 , ebufp);
	
	db		= PIN_POID_GET_DB(order_pdp);

	task_no		= PIN_FLIST_FLD_GET(i_flistp , MSO_FLD_TASK_NAME , 1, ebufp);

	order_id	= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_ORDER_ID , 1, ebufp);

	user_id		= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_USERID , 1, ebufp);

	program_name	= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_PROGRAM_NAME, 1 , ebufp);

	/******
	 * Create the task object in the brm 
	 *****/

	order_iflistp = PIN_FLIST_CREATE(ebufp);

	task_pdp = PIN_POID_CREATE(db , "/mso_task/bulk_remove_plan", -1 , ebufp);
	PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Bulk Add plan" , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_USERID , user_id , ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create for bulk cancel plan input flist is " , order_iflistp);
	PCM_OP(ctxp,PCM_OP_CREATE_OBJ,0 , order_iflistp , &order_oflistp , ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the task object " );
		PIN_FLIST_DESTROY_EX(&order_oflistp , NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_create_job task object for bulk add plan failed", ebufp);
		//*ret_flistp = r_flistp;
		//return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create output for bulk remove plan flist is " , order_oflistp);

	if(order_oflistp)
	{
		task_pdp = PIN_FLIST_FLD_GET(order_oflistp , PIN_FLD_POID , 1, ebufp);

		while ((task_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp,MSO_FLD_TASK,&elem_id, 1, &cookie, ebufp))!= NULL)
		{
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_USERID , user_id , ebufp);
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_DESCR , "Bulk_Cancel_Plan" , ebufp);
			fm_mso_integ_create_cancel_plan_job(ctxp,task_no,task_pdp, task_flistp , ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the cancel plan job object " );
				PIN_ERRBUF_CLEAR(ebufp);
			}

		}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Task object failed ");
	}
	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID , order_pdp, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp,PIN_FLD_ORDER_ID , order_id , ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS , &order_status_sucess , ebufp);
	//PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Device state updated sucessfully", ebufp);
	task_flistp = PIN_FLIST_ELEM_ADD(*ret_flistp,MSO_FLD_TASK,0, ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_ID , task_no , ebufp);

	PIN_FLIST_DESTROY_EX(&order_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&order_oflistp, NULL);
	return ;
}

EXPORT_OP void
fm_mso_integ_create_add_plan_job(
	pcm_context_t	*ctxp,
	char			*task_no,
	poid_t			*task_pdp,
	pin_flist_t		*task_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*order_iflistp = NULL;
	pin_flist_t		*order_oflistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*job_oflistp = NULL;
	pin_flist_t		*job_iflistp = NULL;
	pin_flist_t		*plan_flist = NULL;
	
	poid_t		*job_pdp	=	NULL;
	poid_t		*order_pdp	=	NULL;

	poid_t		*lco_account_pdp	= NULL;
	poid_t		*device_pdp		= NULL;
	poid_t		*service_pdp	= NULL;
	poid_t		*account_pdp	= NULL;

	char		*device_type	= NULL;
	char		*order_id		= NULL;
	char		*account_no		= NULL;
	char		*device_id		= NULL;
	char		*agreement_no	= NULL;

	//int64		db = 1;
	int64		db = 0;
	int32		order_status_sucess = 0;
	int32		order_status_failure = 1;
	int			elem_id = 0;
	int32		status = 0;
	int32		*state_id = NULL;
	int32		status_sucess = 0;
	int32		status_fail = 2;
	int32		add_plan_flag = 2;
	int32		service_type = 1;
	pin_buf_t	*nota_buf	= NULL;
	char		*flistbuf	 = NULL;
	int		flistlen	 =0;
	int32		error_code = 51041;
	int32		device_error_code = 51040;
	int32		opcode = MSO_OP_INTEG_ADD_PLAN;
	char		*plan_name = NULL;
	
	pin_cookie_t	cookie = NULL;
	int32           *input_status = NULL;	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_add_plan input flist", task_flistp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " task_flistp is " ,task_flistp);
	
	input_status = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_STATUS, 1 , ebufp);
	account_no = PIN_FLIST_FLD_GET(task_flistp , PIN_FLD_ACCOUNT_NO, 1 , ebufp);
	plan_name  = PIN_FLIST_FLD_GET(task_flistp , MSO_FLD_PLAN_NAME , 1 , ebufp); //changes the field name from PIN_FLD_NAME to MSO_FLD_PLAN_NAME
	device_id =  PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_ID , 1 , ebufp);
	if(device_id)
	{
		fm_mso_search_device_object(ctxp, task_flistp , ebufp);
		service_pdp = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		db = PIN_POID_GET_DB(service_pdp);
		account_pdp = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
		PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, " service poid is " ,service_pdp); 
		if(PIN_POID_IS_NULL(service_pdp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Service poid is null");
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " error set");
			}
			PIN_ERRBUF_CLEAR(ebufp);
			char		*flistbuf	= NULL;
			pin_buf_t	*nota_buf	= NULL;
			flistlen = 0;
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_SERVICE_TYPE,&service_type,ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_POID, NULL , ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_FLAGS , &add_plan_flag, ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ACCOUNT_NO ,account_no , ebufp); 
			//PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_AGREEMENT_NO , NULL , ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DESCR , "Bulk_add_plan" , ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DEVICE_ID, device_id , ebufp);
			plan_flist = PIN_FLIST_ELEM_ADD(task_flistp,PIN_FLD_PLAN, 0, ebufp);
			PIN_FLIST_FLD_SET(plan_flist, PIN_FLD_NAME, plan_name , ebufp);

			
			PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
			PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
			//PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
			//PIN_FLIST_FLD_DROP(task_flistp , PIN_FLD_NAME, ebufp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERRBUF_CLEAR(ebufp);
			}

			PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
			nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
			if ( nota_buf == NULL )
			{
				pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
				PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
				return;
			}
			nota_buf->flag   = 0;
			nota_buf->size   = flistlen - 2;
			nota_buf->offset = 0;
			nota_buf->data   = ( caddr_t)flistbuf;
			nota_buf->xbuf_file = ( char *) NULL;
			
			/********
			 * Update the status of the mso_mta_job to 2 to indiacte the failure record
			 ******/
			job_iflistp = PIN_FLIST_CREATE(ebufp);
			job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_add_plan", -1 ,ebufp);
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51040" , ebufp)
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid VC number no service is associated to VC" , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
	
			/* OAP sometimes send error in input */
		        if(input_status && (*input_status == status_fail))
        		{
               			 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                		 PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                		 PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                		 PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        		}

        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist",job_iflistp);

        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);
	
				
			PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for Add plan " );
				PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
				PIN_ERRBUF_CLEAR(ebufp);
			}
			PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
			
			if(job_oflistp)
			{
				job_pdp = PIN_FLIST_FLD_GET(job_oflistp , PIN_FLD_POID, 1, ebufp);
			}
			pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"Invalid Device ID Number", ebufp);
			if(nota_buf)
				free(nota_buf);
		}
		else
		{
			/**** 
			 * get the agreement number 
			 *****/

			 if(strstr((char*)PIN_POID_GET_TYPE(service_pdp),"catv"))
			 {
				service_type = 0;
				fm_mso_get_service_agreement_no(ctxp,service_pdp, task_flistp, ebufp);
			 }
			 else if (strstr((char*)PIN_POID_GET_TYPE(service_pdp),"broadband"))
			 {
			 	service_type = 1;
				fm_mso_get_service_agreement_no(ctxp,service_pdp, task_flistp, ebufp);

			 }
			 agreement_no = PIN_FLIST_FLD_GET(task_flistp, MSO_FLD_AGREEMENT_NO,1, ebufp);
			 //PIN_FLIST_FLD_DROP(task_flistp, MSO_FLD_AGREEMENT_NO , ebufp);
		}
	}
	if(account_no)
	{
		fm_mso_search_lco_account(ctxp, task_flistp , ebufp);
		lco_account_pdp = PIN_FLIST_FLD_GET(task_flistp , PIN_FLD_ACCOUNT_OBJ,1, ebufp);
		PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG , " Account_poid is ",lco_account_pdp);
		if(PIN_POID_IS_NULL(lco_account_pdp) || (PIN_POID_COMPARE(account_pdp,lco_account_pdp,0,ebufp) != 0))
		{
			if(PIN_POID_IS_NULL(job_pdp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," no job created prior hence creating ");
				PIN_ERRBUF_CLEAR(ebufp);
				char		*flistbuf	= NULL;
				pin_buf_t	*nota_buf	= NULL;
				flistlen = 0;
				PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_SERVICE_TYPE,&service_type,ebufp);
				PIN_FLIST_FLD_SET(task_flistp,PIN_FLD_POID, NULL , ebufp);
				PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_FLAGS , &add_plan_flag, ebufp);
				PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DESCR , "Bulk_add_plan" , ebufp);
				PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ACCOUNT_NO , account_no , ebufp);
				PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_AGREEMENT_NO , agreement_no , ebufp);
				PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DEVICE_ID , device_id , ebufp);
				plan_flist = PIN_FLIST_ELEM_ADD(task_flistp,PIN_FLD_PLAN, 0, ebufp);
				PIN_FLIST_FLD_SET(plan_flist, PIN_FLD_NAME, plan_name , ebufp);
				PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
				PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
				//PIN_FLIST_FLD_DROP(task_flistp , PIN_FLD_NAME, ebufp);
				//PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_ACCOUNT_OBJ , ebufp);
				if(PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERRBUF_CLEAR(ebufp);
				}

				PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
				nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
				if ( nota_buf == NULL )
				{
					pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
					PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
					return;
				}
				nota_buf->flag   = 0;
				nota_buf->size   = flistlen - 2;
				nota_buf->offset = 0;
				nota_buf->data   = ( caddr_t)flistbuf;
				nota_buf->xbuf_file = ( char *) NULL;
				
				/********
				 * Update the status of the mso_mta_job to 2 to indiacte the failure record
				 ******/
				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_add_plan", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_CODE , "51041" , ebufp)
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid Account number or not matching account no with device owner " , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_ACCOUNT_OBJ , job_iflistp ,PIN_FLD_ACCOUNT_OBJ , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ACCOUNT_OBJ , lco_account_pdp , ebufp);
	
				        /* OAP sometimes send error in input */
			        if(input_status && (*input_status == status_fail))
        			{
                			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                			PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                			PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        			}

        			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist", job_iflistp);

        			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);


				PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
				if(PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for Add plan " );
					PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
					PIN_ERRBUF_CLEAR(ebufp);
				}
				PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
				
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " ALready job created for the record with error status");
			}

			pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"Invalid Account Number", ebufp);
			if(nota_buf)
				free(nota_buf);
		}
		else
		{
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_POID,lco_account_pdp , ebufp);
		}
	}
	else 
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " No Account found for in input ");
		if(PIN_POID_IS_NULL(job_pdp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," no job created prior hence creating ");
			PIN_ERRBUF_CLEAR(ebufp);
			char		*flistbuf	= NULL;
			pin_buf_t	*nota_buf	= NULL;
			flistlen = 0;
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_SERVICE_TYPE,&service_type,ebufp);
			PIN_FLIST_FLD_SET(task_flistp,PIN_FLD_POID, NULL , ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_FLAGS , &add_plan_flag, ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DESCR , "Bulk_add_plan" , ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ACCOUNT_NO , account_no , ebufp);
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_AGREEMENT_NO , agreement_no , ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DEVICE_ID , device_id , ebufp);
			plan_flist = PIN_FLIST_ELEM_ADD(task_flistp,PIN_FLD_PLAN, 0, ebufp);
			PIN_FLIST_FLD_SET(plan_flist, PIN_FLD_NAME, plan_name , ebufp);
			PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
			PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
			//PIN_FLIST_FLD_DROP(task_flistp , PIN_FLD_NAME, ebufp);
			//PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_ACCOUNT_OBJ , ebufp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERRBUF_CLEAR(ebufp);
			}

			PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
			nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
			if ( nota_buf == NULL )
			{
				pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
				PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
				return;
			}
			nota_buf->flag   = 0;
			nota_buf->size   = flistlen - 2;
			nota_buf->offset = 0;
			nota_buf->data   = ( caddr_t)flistbuf;
			nota_buf->xbuf_file = ( char *) NULL;
			
			/********
			 * Update the status of the mso_mta_job to 2 to indiacte the failure record
			 ******/
			job_iflistp = PIN_FLIST_CREATE(ebufp);
			job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_add_plan", -1 ,ebufp);
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_CODE , "51041" , ebufp)
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "No input Account number " , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_ACCOUNT_OBJ , job_iflistp ,PIN_FLD_ACCOUNT_OBJ , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ACCOUNT_OBJ , lco_account_pdp , ebufp);
			        /* OAP sometimes send error in input */
		        if(input_status && (*input_status == status_fail))
        		{
                		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        		}

       		        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist" ,job_iflistp);

       		     	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);


			PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for Add plan " );
				PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
				PIN_ERRBUF_CLEAR(ebufp);
			}
			PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
			
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " ALready job created for the record with error status");
		}

		pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"NO input Account Number", ebufp);
		if(nota_buf)
			free(nota_buf);
	}

	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "Error in creatong job", ebufp );
		PIN_ERRBUF_CLEAR(ebufp);
	}
	else
	{
		if(PIN_POID_IS_NULL(job_pdp))
		{
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_SERVICE_TYPE,&service_type,ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_FLAGS , &add_plan_flag, ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DESCR , "Bulk_add_plan" , ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ACCOUNT_NO , account_no , ebufp);
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_AGREEMENT_NO , agreement_no , ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DEVICE_ID,device_id , ebufp);
			plan_flist = PIN_FLIST_ELEM_ADD(task_flistp,PIN_FLD_PLAN, 0, ebufp);
			PIN_FLIST_FLD_SET(plan_flist, PIN_FLD_NAME, plan_name , ebufp);
			PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
			//PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
			//PIN_FLIST_FLD_DROP(task_flistp ,PIN_FLD_NAME, ebufp);
			//PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_ACCOUNT_OBJ , ebufp);

			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERRBUF_CLEAR(ebufp);
			}

			PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
			nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
			if ( nota_buf == NULL )
			{
				pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
				PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
				return;
			}
		
			/*
			* Create flist for job order creation
			*/
			job_iflistp	= PIN_FLIST_CREATE(ebufp);
		
			job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_add_plan", -1 ,ebufp);
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_sucess , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_POID , job_iflistp ,PIN_FLD_ACCOUNT_OBJ , ebufp);
			//PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ACCOUNT_OBJ , lco_account_pdp , ebufp);
			
			nota_buf->flag   = 0;
			nota_buf->size   = flistlen - 2;
			nota_buf->offset = 0;
			nota_buf->data   = ( caddr_t)flistbuf;
			nota_buf->xbuf_file = ( char *) NULL;
			PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Job create input flist for add plan is " ,job_iflistp);
			
			        /* OAP sometimes send error in input */
        		if(input_status && (*input_status == status_fail))
        		{
                		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        		}

        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist" ,job_iflistp);

			PCM_OP(ctxp,PCM_OP_CREATE_OBJ ,0, job_iflistp , &job_oflistp , ebufp);

			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the mso_mta job object for bulk add plan " );
				PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
				/*r_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_create_job job object create failed for location_updater", ebufp);
				*ret_flistp = r_flistp;
				return;*/
			}
			else
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " job create output flist is " , job_oflistp);
				PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
				PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
			}
		}
	}
	if(nota_buf)
		free(nota_buf);

	return;
}

EXPORT_OP void
fm_mso_integ_create_cancel_plan_job(
	pcm_context_t	*ctxp,
	char			*task_no,
	poid_t			*task_pdp,
	pin_flist_t		*task_flistp,
	pin_errbuf_t	*ebufp)

{
	pin_flist_t		*order_iflistp = NULL;
	pin_flist_t		*order_oflistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*job_oflistp = NULL;
	pin_flist_t		*job_iflistp = NULL;
	pin_flist_t		*plan_flist = NULL;

	poid_t		*job_pdp	=	NULL;
	poid_t		*order_pdp	=	NULL;

	poid_t		*lco_account_pdp	= NULL;
	poid_t		*device_pdp		= NULL;
	poid_t		*service_pdp	= NULL;
	poid_t		*account_pdp	= NULL;

	char		*device_type	= NULL;
	char		*order_id		= NULL;
	char		*account_no		= NULL;
	char		*device_id		= NULL;
	char		*agreement_no	= NULL;
	int64		db = 0;
	//int64		db = 1;
	int32		order_status_sucess = 0;
	int32		order_status_failure = 1;
	int		elem_id = 0;
	int32		status = 0;
	int32		*state_id = NULL;
	int32		status_sucess = 0;
	int32		status_fail = 2;
	int32		add_plan_flag = 2;
	int32		service_type = 1;
	pin_buf_t	*nota_buf	= NULL;
	char		*flistbuf	= NULL;
	int		flistlen	=0;
	int32		error_code = 51041;
	int32		device_error_code = 51040;
	int32		opcode = MSO_OP_INTEG_CANCEL_PLAN;
	char		*plan_name = NULL;
	
	pin_cookie_t	cookie = NULL;
	int32           *input_status = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_create_cancel_plan_job input flist", task_flistp);
	input_status = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_STATUS, 1 , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " task_flistp is " ,task_flistp);

	account_no = PIN_FLIST_FLD_GET(task_flistp , PIN_FLD_ACCOUNT_NO, 1 , ebufp);
	plan_name  = PIN_FLIST_FLD_GET(task_flistp , PIN_FLD_NAME, 1 , ebufp);
	device_id =  PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_ID , 1 , ebufp);
	if(device_id)
	{
		fm_mso_search_device_object(ctxp, task_flistp , ebufp);
		service_pdp = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		db = PIN_POID_GET_DB(service_pdp);
		PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, " service poid is " ,service_pdp); 
		if(PIN_POID_IS_NULL(service_pdp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Service poid is null");
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " error set");
			}
			PIN_ERRBUF_CLEAR(ebufp);
			char		*flistbuf	= NULL;
			pin_buf_t	*nota_buf	= NULL;
			flistlen = 0;
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_SERVICE_TYPE,&service_type,ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_POID, NULL , ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_FLAGS , &add_plan_flag, ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ACCOUNT_NO ,account_no , ebufp); 
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_AGREEMENT_NO , NULL , ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DESCR , "Bulk_cancel_plan" , ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ACTION_NAME,"Bulk_cancel_plan", ebufp );
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DEVICE_ID, device_id , ebufp);
			plan_flist = PIN_FLIST_ELEM_ADD(task_flistp,PIN_FLD_PLAN, 0, ebufp);
			PIN_FLIST_FLD_SET(plan_flist, PIN_FLD_NAME, plan_name , ebufp);
			PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
			PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
			PIN_FLIST_FLD_DROP(task_flistp , PIN_FLD_NAME, ebufp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERRBUF_CLEAR(ebufp);
			}

			PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
			nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
			if ( nota_buf == NULL )
			{
				pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
				PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
				return;
			}
			nota_buf->flag   = 0;
			nota_buf->size   = flistlen - 2;
			nota_buf->offset = 0;
			nota_buf->data   = ( caddr_t)flistbuf;
			nota_buf->xbuf_file = ( char *) NULL;
			
			/********
			 * Update the status of the mso_mta_job to 2 to indiacte the failure record
			 ******/
			job_iflistp = PIN_FLIST_CREATE(ebufp);
			job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_remove_plan", -1 ,ebufp);
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51040" , ebufp)
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid VC number no service is associated to VC" , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
			//PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ACCOUNT_OBJ , account_pdp , ebufp);
			       
			 /* OAP sometimes send error in input */
		        if(input_status && (*input_status == status_fail))
        		{
                		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        		}
        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist" ,job_iflistp);

        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

			PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for cancel plan " );
				PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
				PIN_ERRBUF_CLEAR(ebufp);
			}
			PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
			
			if(job_oflistp)
				job_pdp = PIN_FLIST_FLD_GET(job_oflistp , PIN_FLD_POID, 1, ebufp);

			pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"Invalid VC Number", ebufp);
			if(nota_buf)
				free(nota_buf);
		}
		else
		{
			/**** 
			 * get the agreement number 
			 *****/

			 if(strstr((char*)PIN_POID_GET_TYPE(service_pdp),"catv"))
			 {
				service_type = 0;
				fm_mso_get_service_agreement_no(ctxp,service_pdp, task_flistp, ebufp);
			 }
			 else if (strstr((char*)PIN_POID_GET_TYPE(service_pdp),"broadband"))
			 {
			 	service_type = 1;
				//fm_mso_get_bb_service_agreement_no(ctxp,service_pdp, &task_flistp, ebufp);
				fm_mso_get_service_agreement_no(ctxp,service_pdp, task_flistp, ebufp);

			 }
			 agreement_no = PIN_FLIST_FLD_GET(task_flistp, MSO_FLD_AGREEMENT_NO,1, ebufp);
			 //PIN_FLIST_FLD_DROP(task_flistp, MSO_FLD_AGREEMENT_NO , ebufp);
		}
	}
	if(account_no)
	{
		
		fm_mso_search_lco_account(ctxp, task_flistp , ebufp);
		lco_account_pdp = PIN_FLIST_FLD_GET(task_flistp , PIN_FLD_ACCOUNT_OBJ,1, ebufp);
		PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, " Account_poid for setting is ", lco_account_pdp);
		if(PIN_POID_IS_NULL(lco_account_pdp))
		{
			if(PIN_POID_IS_NULL(job_pdp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," no job created prior hence creating ");
				PIN_ERRBUF_CLEAR(ebufp);
				char		*flistbuf	= NULL;
				pin_buf_t	*nota_buf	= NULL;
				flistlen = 0;
				PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_SERVICE_TYPE,&service_type,ebufp);
				PIN_FLIST_FLD_SET(task_flistp,PIN_FLD_POID, NULL , ebufp);
				PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_FLAGS , &add_plan_flag, ebufp);
				PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DESCR , "Bulk_cancel_plan" , ebufp);
				PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ACTION_NAME,"Bulk_cancel_plan", ebufp );
				PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ACCOUNT_NO , account_no , ebufp);
				PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_AGREEMENT_NO , agreement_no , ebufp);
				PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DEVICE_ID , device_id , ebufp);
				plan_flist = PIN_FLIST_ELEM_ADD(task_flistp,PIN_FLD_PLAN, 0, ebufp);
				PIN_FLIST_FLD_SET(plan_flist, PIN_FLD_NAME, plan_name , ebufp);
				//PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
				//PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
				//PIN_FLIST_FLD_DROP(task_flistp , PIN_FLD_NAME, ebufp);
				//PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_ACCOUNT_OBJ , ebufp);
				
				if(PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERRBUF_CLEAR(ebufp);
				}
				PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
				nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
				if ( nota_buf == NULL )
				{
					pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
					PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
					return;
				}
				nota_buf->flag   = 0;
				nota_buf->size   = flistlen - 2;
				nota_buf->offset = 0;
				nota_buf->data   = ( caddr_t)flistbuf;
				nota_buf->xbuf_file = ( char *) NULL;
				
				/********
				 * Update the status of the mso_mta_job to 2 to indiacte the failure record
				 ******/
				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_remove_plan", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_CODE , "51041" , ebufp)
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid Account number" , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				        /* OAP sometimes send error in input */
			        if(input_status && (*input_status == status_fail))
        			{
                			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                			PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                			PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        			}
        			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist" ,job_iflistp);

        			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);
	
				PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
				if(PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for cancel plan " );
					PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
					PIN_ERRBUF_CLEAR(ebufp);
				}
				PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
				
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " ALready job created for the record with error status");
			}

				pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"Invalid Account Number", ebufp);
			if(nota_buf)
				free(nota_buf);
		}
		else
		{
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_POID,lco_account_pdp , ebufp);
		}
	}
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " No Account found for the account no ");
		PIN_ERRBUF_CLEAR(ebufp);
		
	}
	else
	{
		if(PIN_POID_IS_NULL(job_pdp))
		{
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_SERVICE_TYPE,&service_type,ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_FLAGS , &add_plan_flag, ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DESCR , "Bulk_cancel_plan" , ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ACTION_NAME,"Bulk_cancel_plan", ebufp );
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ACCOUNT_NO , account_no , ebufp);
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_AGREEMENT_NO , agreement_no , ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DEVICE_ID,device_id , ebufp);
			plan_flist = PIN_FLIST_ELEM_ADD(task_flistp,PIN_FLD_PLAN, 0, ebufp);
			PIN_FLIST_FLD_SET(plan_flist, PIN_FLD_NAME, plan_name , ebufp);
			//PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
			//PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
			PIN_FLIST_FLD_DROP(task_flistp ,PIN_FLD_NAME, ebufp);
			//PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_ACCOUNT_OBJ , ebufp);

			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERRBUF_CLEAR(ebufp);
			}

			PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
			nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
			if ( nota_buf == NULL )
			{
				pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
				PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
				return;
			}
		
			/*
			* Create flist for job order creation
			*/
			job_iflistp	= PIN_FLIST_CREATE(ebufp);
		
			job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_remove_plan", -1 ,ebufp);
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_sucess , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_POID , job_iflistp ,PIN_FLD_ACCOUNT_OBJ , ebufp);
			//PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ACCOUNT_OBJ , lco_account_pdp , ebufp);

			nota_buf->flag   = 0;
			nota_buf->size   = flistlen - 2;
			nota_buf->offset = 0;
			nota_buf->data   = ( caddr_t)flistbuf;
			nota_buf->xbuf_file = ( char *) NULL;
			PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Job create input flist for cancel plan is " ,job_iflistp);
			        /* OAP sometimes send error in input */
		        if(input_status && (*input_status == status_fail))
        		{
               			 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                		 PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                		 PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                		 PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        		}
        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist" ,job_iflistp);

        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

			PCM_OP(ctxp,PCM_OP_CREATE_OBJ ,0, job_iflistp , &job_oflistp , ebufp);

			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the mso_mta job object for bulk cancel plan " );
				PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
				/*r_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_create_job job object create failed for location_updater", ebufp);
				*ret_flistp = r_flistp;
				return;*/
			}
			else
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " job create output flist is " , job_oflistp);
				PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
				PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
			}
		}
	}
	if(nota_buf)
		free(nota_buf);

	return;
}

EXPORT_OP void
fm_mso_integ_create_change_plan_job(
	pcm_context_t	*ctxp,
	char			*task_no,
	poid_t			*task_pdp,
	pin_flist_t		*task_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*order_iflistp = NULL;
	pin_flist_t		*order_oflistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*job_oflistp = NULL;
	pin_flist_t		*job_iflistp = NULL;
	pin_flist_t		*plan_flist_old = NULL;
	pin_flist_t		*plan_flist_new = NULL;

	poid_t		*job_pdp	=	NULL;
	poid_t		*order_pdp	=	NULL;
	
	poid_t		*lco_account_pdp	= NULL;
	poid_t		*device_pdp		= NULL;
	poid_t		*service_pdp	= NULL;
	poid_t		*account_pdp	= NULL;

	char		*device_type	= NULL;
	char		*order_id		= NULL;
	char		*account_no		= NULL;
	char		*device_id		= NULL;
	char		*agreement_no	= NULL;
	int64		db = 0;
	//int64		db = 1;
	int32		order_status_sucess = 0;
	int32		order_status_failure = 1;
	int			elem_id = 0;
	int32		status = 0;
	int32		*state_id = NULL;
	int32		status_sucess = 0;
	int32		status_fail = 2;
	int32		add_plan_flag = 2;
	int32		service_type = 1;
	pin_buf_t	*nota_buf	= NULL;
	char		*flistbuf	 = NULL;
	int			flistlen	 =0;
	int32		error_code = 51041;
	int32		device_error_code = 51040;
	int32		opcode = MSO_OP_INTEG_CHANGE_PLAN;
	char		*old_plan_name = NULL;
	char		*new_plan_name = NULL;
	
	pin_cookie_t	cookie = NULL;
	int32           *input_status = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_create_change_plan_job input flist", task_flistp);
	
	input_status = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_STATUS, 1 , ebufp);
	account_no = PIN_FLIST_FLD_GET(task_flistp , PIN_FLD_ACCOUNT_NO, 1 , ebufp);
	old_plan_name  = PIN_FLIST_FLD_GET(task_flistp , PIN_FLD_OLD_VALUE , 1 , ebufp);
	new_plan_name  = PIN_FLIST_FLD_GET(task_flistp , PIN_FLD_NEW_VALUE , 1 , ebufp);
	device_id =  PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_ID , 1 , ebufp);
	if(device_id)
	{
		fm_mso_search_device_object(ctxp, task_flistp , ebufp);
		service_pdp = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		db = PIN_POID_GET_DB(service_pdp);
		if(PIN_POID_IS_NULL(service_pdp))
		{
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_SERVICE_TYPE,&service_type,ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_POID, NULL , ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_FLAGS , &add_plan_flag, ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DESCR , "Bulk_change_plan" , ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DEVICE_ID, device_id , ebufp);
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_AGREEMENT_NO , NULL , ebufp);
		//	PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ACCOUNT_OBJ, account_no , ebufp);
			plan_flist_old = PIN_FLIST_ELEM_ADD(task_flistp,PIN_FLD_PLAN, 0, ebufp);
			PIN_FLIST_FLD_SET(plan_flist_old, PIN_FLD_NAME, old_plan_name , ebufp);
			plan_flist_new = PIN_FLIST_ELEM_ADD(task_flistp,PIN_FLD_PLAN, 1, ebufp);
			PIN_FLIST_FLD_SET(plan_flist_new, PIN_FLD_NAME, new_plan_name , ebufp);
			PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
			PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
			PIN_FLIST_FLD_DROP(task_flistp , PIN_FLD_OLD_VALUE, ebufp);
			PIN_FLIST_FLD_DROP(task_flistp , PIN_FLD_NEW_VALUE, ebufp);
			//PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);

			PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
			nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
			if ( nota_buf == NULL )
			{
				pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
				PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
				return;
			}
			nota_buf->flag   = 0;
			nota_buf->size   = flistlen - 2;
			nota_buf->offset = 0;
			nota_buf->data   = ( caddr_t)flistbuf;
			nota_buf->xbuf_file = ( char *) NULL;
			
			/********
			 * Update the status of the mso_mta_job to 2 to indiacte the failure record
			 ******/
			job_iflistp	= PIN_FLIST_CREATE(ebufp);
			job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_change_plan", -1 ,ebufp);
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_STATUS , &status_fail , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51040" , ebufp)
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid VC number no service is associated to VC" , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
			
			        /* OAP sometimes send error in input */
		        if(input_status && (*input_status == status_fail))
        		{
                		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        		}
        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist" ,job_iflistp);

        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);
	
			PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for change plan " );
				PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
				PIN_ERRBUF_CLEAR(ebufp);
			}
			PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);

			if(job_oflistp)
				job_pdp = PIN_FLIST_FLD_GET(job_oflistp , PIN_FLD_POID, 1, ebufp);
			
			pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"Invalid VC Number", ebufp);
		}
		else if(device_id && strstr(device_id, "BB")) //In MEN case, we pass AGREEMENT NO so not required incase AGREEMENT PASSED
		{
			agreement_no = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_ID, 1, ebufp);
		}
		else
		{
			/**** 
			 * get the agreement number 
			 *****/

			 if(strstr((char*)PIN_POID_GET_TYPE(service_pdp),"catv"))
			 {
				service_type = 0;
				fm_mso_get_service_agreement_no(ctxp,service_pdp, task_flistp, ebufp);
			 }
			 else if (strstr((char*)PIN_POID_GET_TYPE(service_pdp),"broadband"))
			 {
			 	service_type = 1;
				fm_mso_get_service_agreement_no(ctxp,service_pdp, task_flistp, ebufp);

			 }
			 agreement_no = PIN_FLIST_FLD_GET(task_flistp, MSO_FLD_AGREEMENT_NO,1, ebufp);
			 //PIN_FLIST_FLD_DROP(task_flistp, MSO_FLD_AGREEMENT_NO , ebufp);
		}
	}
	if(account_no)
	{
		fm_mso_search_lco_account(ctxp, task_flistp , ebufp);
		lco_account_pdp = PIN_FLIST_FLD_GET(task_flistp , PIN_FLD_ACCOUNT_OBJ,1, ebufp);
		PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, " Account_poid for setting is ", lco_account_pdp);
		if(PIN_POID_IS_NULL(lco_account_pdp))
		{
			if(PIN_POID_IS_NULL(job_pdp))
			{
				PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_SERVICE_TYPE,&service_type,ebufp);
				PIN_FLIST_FLD_SET(task_flistp,PIN_FLD_POID, NULL , ebufp);
				PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_FLAGS , &add_plan_flag, ebufp);
				PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DESCR , "Bulk_change_plan" , ebufp);
				PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ACCOUNT_NO , account_no , ebufp);
				PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_AGREEMENT_NO , agreement_no , ebufp);
				PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DEVICE_ID , device_id , ebufp);
				plan_flist_old = PIN_FLIST_ELEM_ADD(task_flistp,PIN_FLD_PLAN, 0, ebufp);
				PIN_FLIST_FLD_SET(plan_flist_old, PIN_FLD_NAME, old_plan_name , ebufp);
				plan_flist_new = PIN_FLIST_ELEM_ADD(task_flistp,PIN_FLD_PLAN, 1, ebufp);
				PIN_FLIST_FLD_SET(plan_flist_new, PIN_FLD_NAME, new_plan_name , ebufp);
				PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
				PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
				PIN_FLIST_FLD_DROP(task_flistp , PIN_FLD_OLD_VALUE, ebufp);
				PIN_FLIST_FLD_DROP(task_flistp , PIN_FLD_NEW_VALUE, ebufp);
				
				//PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_ACCOUNT_OBJ , ebufp);

				PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
				nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
				if ( nota_buf == NULL )
				{
					pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
					PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
					return;
				}
				nota_buf->flag   = 0;
				nota_buf->size   = flistlen - 2;
				nota_buf->offset = 0;
				nota_buf->data   = ( caddr_t)flistbuf;
				nota_buf->xbuf_file = ( char *) NULL;
				
				/********
				 * Update the status of the mso_mta_job to 2 to indiacte the failure record
				 ******/
				 job_iflistp	= PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_change_plan", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_CODE , "51041" , ebufp)
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid Account number" , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				//PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ACCOUNT_OBJ , lco_account_pdp , ebufp);
				
        			/* OAP sometimes send error in input */
        			if(input_status && (*input_status == status_fail))
        			{
                			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                			PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                			PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        			}
        			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist" ,job_iflistp);

        			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

				PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
				if(PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for change plan " );
					PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
					PIN_ERRBUF_CLEAR(ebufp);
				}
				PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
				PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
			}

			pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"Invalid Account Number", ebufp);
		}
		else
		{
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_POID,lco_account_pdp , ebufp);
		}
	}
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " No Account found for the account no ");
		PIN_ERRBUF_CLEAR(ebufp);
	}
	else
	{	
		if(PIN_POID_IS_NULL(job_pdp))
		{
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_SERVICE_TYPE,&service_type,ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_FLAGS , &add_plan_flag, ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DESCR , "Bulk_change_plan" , ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ACCOUNT_NO , account_no , ebufp);
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_AGREEMENT_NO , agreement_no , ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DEVICE_ID,device_id , ebufp);
			plan_flist_old = PIN_FLIST_ELEM_ADD(task_flistp,PIN_FLD_PLAN, 0, ebufp);
			PIN_FLIST_FLD_SET(plan_flist_old, PIN_FLD_NAME, old_plan_name , ebufp);
			plan_flist_new = PIN_FLIST_ELEM_ADD(task_flistp,PIN_FLD_PLAN, 1, ebufp);
			PIN_FLIST_FLD_SET(plan_flist_new, PIN_FLD_NAME, new_plan_name , ebufp);
			if(device_id && !strstr(device_id, "BB")) //IN MEN Case, we don't set DEVICE_OBJ
				PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
			PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
			PIN_FLIST_FLD_DROP(task_flistp , PIN_FLD_OLD_VALUE, ebufp);
			PIN_FLIST_FLD_DROP(task_flistp , PIN_FLD_NEW_VALUE, ebufp);
			PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_ACCOUNT_OBJ , ebufp);

			PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
			nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
			if ( nota_buf == NULL )
			{
				pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
				PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
				return;
			}
		
			/*
			* Create flist for job order creation
			*/
			job_iflistp	= PIN_FLIST_CREATE(ebufp);
		
			job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_change_plan", -1 ,ebufp);
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_sucess , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_POID , job_iflistp ,PIN_FLD_ACCOUNT_OBJ , ebufp);
			//PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ACCOUNT_OBJ , lco_account_pdp , ebufp);
			
			nota_buf->flag   = 0;
			nota_buf->size   = flistlen - 2;
			nota_buf->offset = 0;
			nota_buf->data   = ( caddr_t)flistbuf;
			nota_buf->xbuf_file = ( char *) NULL;
			PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Job create input flist for bulk change plan is " ,job_iflistp);
			
			        /* OAP sometimes send error in input */
		        if(input_status && (*input_status == status_fail))
        		{
                		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        		}
        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist" ,job_iflistp);

        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

			PCM_OP(ctxp,PCM_OP_CREATE_OBJ ,0, job_iflistp , &job_oflistp , ebufp);

			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the mso_mta job object for bulk change plan " );
				PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
				/*r_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_create_job job object create failed for location_updater", ebufp);
				*ret_flistp = r_flistp;
				return;*/
			}
			else
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " job create output flist is " , job_oflistp);
				PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
				PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
			}
		}
	}
	if(nota_buf)
	{
		free(nota_buf);
	}
	return;
}

EXPORT_OP void
fm_mso_integ_create_state_change_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*in_flistp = NULL;
	pin_flist_t		*write_oflistp = NULL;
	pin_flist_t		*write_iflistp = NULL;
	pin_flist_t		*order_iflistp = NULL;
	pin_flist_t		*order_oflistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*job_oflistp = NULL;
	pin_flist_t		*job_iflistp = NULL;

	poid_t		*job_pdp	= NULL;
	poid_t		*order_pdp	= NULL;
	poid_t		*lco_account_pdp = NULL;
	poid_t		*device_pdp	= NULL;

	char		*device_type	= NULL;
	char		*order_id	= NULL;
	char		*account_no	= NULL;
	char		*device_id	= NULL;

	//int64		db = 1;
	int64		db = 0;	
	int32		status = 0;
	int32		*state_id = NULL;
	int32		status_sucess = 0;
	int32		status_fail = 2;
	pin_buf_t	*nota_buf   = NULL;
	char		*flistbuf   = NULL;
	int		flistlen    =0;
        pin_cookie_t	cookie = NULL;
	int32		opcode = MSO_OP_DEVICE_SET_STATE;
	int32           *input_status = NULL;
		
	if (PIN_ERRBUF_IS_ERR(ebufp))
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_create_state_change_job input flist", task_flistp);
	input_status = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_STATUS, 1 , ebufp);
	db = PIN_POID_GET_DB(task_pdp);
	device_type = PIN_FLIST_FLD_GET(task_flistp ,PIN_FLD_DEVICE_TYPE, 1 , ebufp);
	/* why drop ? commented so that task flist still contain it */
	/*PIN_FLIST_FLD_DROP(task_flistp,PIN_FLD_DEVICE_TYPE,ebufp); */ 
	PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_POID , task_pdp , ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_create_state_change_job input after modification flist", task_flistp);
	PIN_FLIST_TO_STR( task_flistp, &flistbuf, &flistlen, ebufp );
	nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ));
	if ( nota_buf == NULL )
	{
		pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
		return;
	}
		/*
		 * Create flist for job order creation
		 */
	job_iflistp	= PIN_FLIST_CREATE(ebufp);

	if((device_type) && (strcmp(device_type ,"STB") == 0))
	{
		job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/state_change/stb", -1 ,ebufp);
		PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_sucess , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
		PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
		PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
	}
	else if((device_type) && (strcmp(device_type ,"VC") == 0))
	{
		job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/state_change/vc", -1 ,ebufp);
		PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_sucess , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
		PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
		PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
	}
	else if((device_type) && (strcmp(device_type ,"MODEM") == 0))
	{
		job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/state_change/modem", -1 ,ebufp);
		PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_sucess , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
	}
	else if((device_type) && (strcmp(device_type ,"ROUTER/CABLE") == 0))
	{
		job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/state_change/router/cable", -1 ,ebufp);
		PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_sucess , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
	}
	else if((device_type) && (strcmp(device_type ,"ROUTER/WIFI") == 0))
	{
		job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/state_change/router/wifi", -1 ,ebufp);
		PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_sucess , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
	}
	else if((device_type) && (strcmp(device_type ,"NAT") == 0))
	{
		job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/state_change/nat", -1 ,ebufp);
		PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_sucess , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
	}
	else if((device_type) && (strcmp(device_type ,"IP/STATIC") == 0))
	{
		job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/state_change/ip/static", -1 ,ebufp);
		PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_sucess , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
	}
	else if((device_type) && (strcmp(device_type ,"IP/FRAMED") == 0))
	{
		job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/state_change/ip/framed", -1 ,ebufp);
		PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_sucess , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
	}
	nota_buf->flag   = 0;
	nota_buf->size   = flistlen - 2;
	nota_buf->offset = 0;
	nota_buf->data   = ( caddr_t)flistbuf;
	nota_buf->xbuf_file = ( char *) NULL;
	PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
	//PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, " State change sucessfull" , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Job create input flist for state change is " ,job_iflistp);
	        /* OAP sometimes send error in input */
        if(input_status && (*input_status == status_fail))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist" ,job_iflistp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);
	PCM_OP(ctxp,PCM_OP_CREATE_OBJ ,0, job_iflistp , &job_oflistp , ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the mso_mta job object for state change " );
		PIN_ERRBUF_CLEAR(ebufp);
		/*r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_create_job job object create failed for location_updater", ebufp);
		*ret_flistp = r_flistp;
		return;*/
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " job create output flist is " , job_oflistp);
//		if(job_oflistp != NULL)
//		{
//			job_pdp = PIN_FLIST_FLD_GET(job_oflistp , PIN_FLD_POID, 1, ebufp);
//		}
//		account_no = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_ACCOUNT_NO , 1, ebufp);
//		state_id	= PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_STATE_ID, 1, ebufp);
//
//		/*if(account_no)
//		{
//			fm_mso_search_lco_account(ctxp, task_flistp , ebufp);
//			lco_account_pdp = PIN_FLIST_FLD_GET(task_flistp , PIN_FLD_ACCOUNT_OBJ,1, ebufp);
//		}*/
//		device_id = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_ID , 1, ebufp);
//		if(device_id)
//		{
//			fm_mso_search_device_object(ctxp,task_flistp, ebufp);
//			device_pdp = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_OBJ,1, ebufp);
//		}
//
//	/***** 
//	 * Prepare flist to update the device state
//	 *****/
//	 if(PIN_POID_IS_NULL(device_pdp))
//	 {
//		 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Device object not found ");
//		 pin_set_err(ebufp, PIN_ERRLOC_FM,
//			PIN_ERRCLASS_SYSTEM_DETERMINATE,
//			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
//			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
//			"Invalid Device Id", ebufp);
//
//		if (PIN_ERRBUF_IS_ERR(ebufp))
//		{
//			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the state of the device " );
//			PIN_ERRBUF_CLEAR(ebufp);
//			/********
//			 * Update the status of the mso_mta_job to 2 to indiacte the failure record
//			 ******/
//			job_iflistp	= PIN_FLIST_CREATE(ebufp);
//			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_POID ,job_pdp , ebufp);
//			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
//			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51040" , ebufp);
//			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR , " Device id not found " , ebufp);
//			PCM_OP(ctxp, PCM_OP_WRITE_FLDS,0 ,job_iflistp , &job_oflistp , ebufp);
//			if(PIN_ERRBUF_IS_ERR(ebufp))
//			{
//				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the job object for state change " );
//				PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
//				PIN_ERRBUF_CLEAR(ebufp);
//			}
//			PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
//		}
//	 }
//	 else
//	 {
//		 write_iflistp = PIN_FLIST_CREATE(ebufp);
//		 PIN_FLIST_FLD_SET(write_iflistp , PIN_FLD_POID , device_pdp , ebufp);
//		 PIN_FLIST_FLD_SET(write_iflistp , PIN_FLD_NEW_STATE, state_id , ebufp);
//		 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Device state update input flist is  " ,write_iflistp);
//		 PCM_OP(ctxp,2701,0 ,write_iflistp , &write_oflistp , ebufp);
//	 }
//		if (PIN_ERRBUF_IS_ERR(ebufp))
//		{
//			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the state of the device " );
//			PIN_ERRBUF_CLEAR(ebufp);
//			/********
//			 * Update the status of the mso_mta_job to 2 to indiacte the failure record
//			 ******/
//			job_iflistp	= PIN_FLIST_CREATE(ebufp);
//			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_POID ,job_pdp , ebufp);
//			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
//			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_CODE, "51042", ebufp);
//			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, " State change not possible from current state to given state", ebufp);
//			PCM_OP(ctxp, PCM_OP_WRITE_FLDS,0 ,job_iflistp , &job_oflistp , ebufp);
//			if(PIN_ERRBUF_IS_ERR(ebufp))
//			{
//				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the job object for state change " );
//				PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
//				PIN_ERRBUF_CLEAR(ebufp);
//			}
//				PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
//		}
//		else
//		{
//			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Device state update output flist is  " ,write_oflistp);
//			PIN_FLIST_DESTROY_EX(&write_iflistp , NULL);
//			PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
//		
//		}
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Device state update output flist is  " ,write_oflistp);
		PIN_FLIST_DESTROY_EX(&write_iflistp , NULL);
		PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Job create output flist for state change is " ,job_oflistp);

	if(nota_buf)
		free(nota_buf);
	PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
	PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);

return;

}

EXPORT_OP void
fm_mso_integ_create_location_update_job(
	pcm_context_t	*ctxp,
	char			*task_no,
	poid_t			*task_pdp,
	pin_flist_t		*task_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*write_oflistp = NULL;
	pin_flist_t		*write_iflistp = NULL;
	pin_flist_t		*order_iflistp = NULL;
	pin_flist_t		*order_oflistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*job_oflistp = NULL;
	pin_flist_t		*job_iflistp = NULL;

	poid_t		*job_pdp	=	NULL;
	poid_t		*order_pdp	=	NULL;
	poid_t		*lco_account_pdp	= NULL;
	poid_t		*device_pdp		= NULL;

	char		*device_type	= NULL;
	char		*order_id		= NULL;
	char		*account_no		= NULL;
	char		*device_id		= NULL;
	int64		db = 0;
	//int64		db = 1;
	int32		order_status_sucess = 0;
	int32		order_status_failure = 1;
	int32		status = 0;
	int32		*state_id = NULL;
	int32		status_sucess = 1;
	int32		status_fail = 2;
	pin_buf_t	*nota_buf	= NULL;
	char		*flistbuf	 = NULL;
	int		flistlen	 =0;
	int32           opcode = MSO_OP_DEVICE_SET_ATTR;
		
	pin_cookie_t	cookie = NULL;
	int32           *input_status = NULL;	

	if (PIN_ERRBUF_IS_ERR(ebufp))
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_create_location_update_job input flist", task_flistp);
	input_status = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_STATUS, 1 , ebufp);
	device_type = PIN_FLIST_FLD_GET(task_flistp ,PIN_FLD_DEVICE_TYPE, 1 , ebufp);
	/* why drop ? let's carry forward */
	/*PIN_FLIST_FLD_DROP(task_flistp,PIN_FLD_DEVICE_TYPE,ebufp);*/
	db = PIN_POID_GET_DB(task_pdp);
	PIN_FLIST_TO_STR( task_flistp, &flistbuf, &flistlen, ebufp );

	nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
	if ( nota_buf == NULL )
	{
		pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
		 PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
		 return;
	}
	/*
	 * Create flist for job order creation
	 */
	job_iflistp	= PIN_FLIST_CREATE(ebufp);

	if((device_type) && (strcmp(device_type ,"STB") == 0))
	{
		job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/location_update/stb", -1 ,ebufp);
		PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &order_status_sucess , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
		PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
                PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);	
	}
	else if((device_type) && (strcmp(device_type ,"VC") == 0))
	{
		job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/location_update/vc", -1 ,ebufp);
		PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &order_status_sucess , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
		PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
                PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
	}
	else if((device_type) && (strcmp(device_type ,"MODEM") == 0))
	{
		job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/location_update/modem", -1 ,ebufp);
		PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &order_status_sucess , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
	}
	else if((device_type) && (strcmp(device_type ,"ROUTER/CABLE") == 0))
	{
		job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/location_update/router/cable", -1 ,ebufp);
		PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &order_status_sucess , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
	}
	else if((device_type) && (strcmp(device_type ,"ROUTER/WIFI") == 0))
	{
		job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/location_update/router/wifi", -1 ,ebufp);
		PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &order_status_sucess , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
	}
	else if((device_type) && (strcmp(device_type ,"NAT") == 0))
	{
		job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/location_update/nat", -1 ,ebufp);
		PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &order_status_sucess , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
	}
	else if((device_type) && (strcmp(device_type ,"IP/STATIC") == 0))
	{
		job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/location_update/ip/static", -1 ,ebufp);
		PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &order_status_sucess , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
	}
	else if((device_type) && (strcmp(device_type ,"IP/FRAMED") == 0))
	{
		job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/location_update/ip/framed", -1 ,ebufp);
		PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &order_status_sucess , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
	}
	nota_buf->flag   = 0;
	nota_buf->size   = flistlen - 2;
	nota_buf->offset = 0;
	nota_buf->data   = ( caddr_t)flistbuf;
	nota_buf->xbuf_file = ( char *) NULL;
	PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
	//PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, " location update sucessfull" , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Job create input flist for location_update is " ,job_iflistp);
	
	        /* OAP sometimes send error in input */
        if(input_status && (*input_status == status_fail))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist" ,job_iflistp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

	PCM_OP(ctxp,PCM_OP_CREATE_OBJ ,0, job_iflistp , &job_oflistp , ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the mso_mta job object for location updater " );
		PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
		PIN_ERRBUF_CLEAR(ebufp);
		/*r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_create_job job object create failed for location_updater", ebufp);
		*ret_flistp = r_flistp;
		return;*/
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " job create output flist is " , job_oflistp);
//		PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
//
//		if(job_oflistp != NULL)
//		{
//			job_pdp = PIN_FLIST_FLD_GET(job_oflistp , PIN_FLD_POID, 1, ebufp);
//		}
//
//		account_no = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_ACCOUNT_NO , 1, ebufp);
//
//		if(account_no)
//		{
//			fm_mso_search_lco_account(ctxp, task_flistp , ebufp);
//			lco_account_pdp = PIN_FLIST_FLD_GET(task_flistp , PIN_FLD_ACCOUNT_OBJ,1, ebufp);
//		}
//		if((PIN_POID_IS_NULL(lco_account_pdp)))
//		{
//			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " account object not found ");
//			pin_set_err(ebufp, PIN_ERRLOC_FM,
//			PIN_ERRCLASS_SYSTEM_DETERMINATE,
//			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
//			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
//			"Invalid account number", ebufp);
//		}
//
//		if (PIN_ERRBUF_IS_ERR(ebufp))
//		{
//			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the device object for location update " );
//			PIN_ERRBUF_CLEAR(ebufp);
//			/********
//			 * Update the status of the mso_mta_job to 2 to indiacte the failure record
//			 ******/
//			job_iflistp	= PIN_FLIST_CREATE(ebufp);
//			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_POID ,job_pdp , ebufp);
//			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
//			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE, "51041" , ebufp);
//			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR , "Account Number Not found" , ebufp);
//			PCM_OP(ctxp, PCM_OP_WRITE_FLDS,0 ,job_iflistp , &job_oflistp , ebufp);
//			if(PIN_ERRBUF_IS_ERR(ebufp))
//			{
//				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the job object for location update " );
//				PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
//				PIN_ERRBUF_CLEAR(ebufp);
//			}
//			PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
//			/*r_flistp = PIN_FLIST_CREATE(ebufp);
//			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
//			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
//			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
//			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_create_job job object create failed", ebufp);
//			*ret_flistp = r_flistp;
//			return;*/
//		}
//
//		device_id = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_ID , 1, ebufp);
//		if(device_id)
//		{
//			fm_mso_search_device_object(ctxp,task_flistp, ebufp);
//			device_pdp = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_OBJ,1, ebufp);
//		}
//
//		 if((PIN_POID_IS_NULL(device_pdp)) )
//		 {
//			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Device object not found ");
//
//			pin_set_err(ebufp, PIN_ERRLOC_FM,
//			PIN_ERRCLASS_SYSTEM_DETERMINATE,
//			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
//			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
//			"Invalid Device Id", ebufp);
//
//			if (PIN_ERRBUF_IS_ERR(ebufp))
//			{
//				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the device object for location update " );
//				PIN_ERRBUF_CLEAR(ebufp);
//				/********
//				 * Update the status of the mso_mta_job to 2 to indiacte the failure record
//				 ******/
//				job_iflistp	= PIN_FLIST_CREATE(ebufp);
//				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_POID ,job_pdp , ebufp);
//				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
//				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51040" , ebufp);
//				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, " Device Id not found " , ebufp);
//				PCM_OP(ctxp, PCM_OP_WRITE_FLDS,0 ,job_iflistp , &job_oflistp , ebufp);
//				if(PIN_ERRBUF_IS_ERR(ebufp))
//				{
//					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the job object for location update " );
//					PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
//					PIN_ERRBUF_CLEAR(ebufp);
//				}
//				PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
//				
//			}
//		 }
//		 else
//		 {
//			/***** 
//			 * Prepare flist to update the device poid 
//			 *****/
//			 write_iflistp = PIN_FLIST_CREATE(ebufp);
//			 PIN_FLIST_FLD_SET(write_iflistp , PIN_FLD_POID , device_pdp , ebufp);
//			 PIN_FLIST_FLD_SET(write_iflistp , PIN_FLD_ACCOUNT_OBJ , lco_account_pdp , ebufp);
//			 PIN_FLIST_FLD_SET(write_iflistp , PIN_FLD_SOURCE, account_no , ebufp);
//			 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Device write fields input flist is  " ,write_iflistp);
//			 PCM_OP(ctxp,PCM_OP_WRITE_FLDS,0 , write_iflistp , &write_oflistp , ebufp);
//		 }
//
//		if (PIN_ERRBUF_IS_ERR(ebufp))
//		{
//			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the device object for location update " );
//			PIN_ERRBUF_CLEAR(ebufp);
//			/********
//			 * Update the status of the mso_mta_job to 2 to indiacte the failure record
//			 ******/
//			job_iflistp	= PIN_FLIST_CREATE(ebufp);
//			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_POID ,job_pdp , ebufp);
//			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
//			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE, "51042" , ebufp);
//			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, " Location update failed " , ebufp);
//			PCM_OP(ctxp, PCM_OP_WRITE_FLDS,0 ,job_iflistp , &job_oflistp , ebufp);
//			if(PIN_ERRBUF_IS_ERR(ebufp))
//			{
//				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the job object for location update " );
//				PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
//				PIN_ERRBUF_CLEAR(ebufp);
//			}
//			PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
//			/*r_flistp = PIN_FLIST_CREATE(ebufp);
//			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
//			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
//			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
//			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_create_job job object create failed", ebufp);
//			*ret_flistp = r_flistp;
//			return;*/
//		}
//		else
//		{
//			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Device location updated sucessfully ");
//			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Device write fields output flist is  " ,write_oflistp);
//			PIN_FLIST_DESTROY_EX(&write_iflistp , NULL);
//			PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
//			
//		}
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Job create output flist for location_update is " ,job_oflistp);

	if(nota_buf)
		free(nota_buf);

	PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
	PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);

	return;
}

EXPORT_OP void
fm_mso_integ_create_attribute_update_job(
	pcm_context_t	*ctxp,
	char			*task_no,
	poid_t			*task_pdp,
	pin_flist_t		*task_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*write_oflistp = NULL;
	pin_flist_t		*write_iflistp = NULL;
	pin_flist_t		*order_iflistp = NULL;
	pin_flist_t		*order_oflistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*job_oflistp = NULL;
	pin_flist_t		*job_iflistp = NULL;

	poid_t		*job_pdp	=	NULL;
	poid_t		*order_pdp	=	NULL;
	poid_t		*device_pdp		= NULL;

	char		*device_type	= NULL;
	char		*order_id		= NULL;
	char		*device_id		= NULL;
	//char		*category = NEW_DEVICE_CATEGORY;
	int64		db = 0;
	//int64		db = 1;
	int32		order_status_sucess = 0;
	int32		order_status_failure = 1;
	int32		status = 0;
	int32		*state_id = NULL;
	int32		status_sucess = 0;
	int32		status_fail = 2;
	pin_buf_t	*nota_buf	= NULL;
	char		*flistbuf	 = NULL;
	int			flistlen	 =0;
	int32           opcode = MSO_OP_DEVICE_SET_ATTR;
		
	pin_cookie_t	cookie = NULL;
	int32           *input_status = NULL;
	

	if (PIN_ERRBUF_IS_ERR(ebufp))
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_create_attribute_update_job input flist", task_flistp);
	db = PIN_POID_GET_DB(task_pdp);
	device_type = PIN_FLIST_FLD_GET(task_flistp ,PIN_FLD_DEVICE_TYPE, 1 , ebufp);
	input_status = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_STATUS, 1 , ebufp);
	/*Why drop ? commenting this code so device type remain in task */
	//PIN_FLIST_FLD_DROP(task_flistp,PIN_FLD_DEVICE_TYPE,ebufp); 

	PIN_FLIST_TO_STR( task_flistp, &flistbuf, &flistlen, ebufp );

	nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
	if ( nota_buf == NULL )
	{
		pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
		 PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
		 return;
	}
	/*
	 * Create flist for job order creation
	 */
	job_iflistp	= PIN_FLIST_CREATE(ebufp);

	if((device_type) && (strcmp(device_type ,"STB") == 0))
	{
		job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/attribute_update/stb", -1 ,ebufp);
		PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_sucess , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
	
	}
	else if((device_type) && (strcmp(device_type ,"VC") == 0))
	{
		job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/attribute_update/vc", -1 ,ebufp);
		PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_sucess , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
	}
	else if((device_type) && (strcmp(device_type ,"MODEM") == 0))
	{
		job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/attribute_update/modem", -1 ,ebufp);
		PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_sucess , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
	}
	else if((device_type) && (strcmp(device_type ,"ROUTER/CABLE") == 0))
	{
		job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/attribute_update/router/cable", -1 ,ebufp);
		PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_sucess , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
	}
	else if((device_type) && (strcmp(device_type ,"ROUTER/WIFI") == 0))
	{
		job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/attribute_update/router/wifi", -1 ,ebufp);
		PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_sucess , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
	}
	else if((device_type) && (strcmp(device_type ,"NAT") == 0))
	{
		job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/attribute_update/nat", -1 ,ebufp);
		PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_sucess , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
	}
	else if((device_type) && (strcmp(device_type ,"IP/STATIC") == 0))
	{
		job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/attribute_update/ip/static", -1 ,ebufp);
		PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_sucess , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
	}
	else if((device_type) && (strcmp(device_type ,"IP/FRAMED") == 0))
	{
		job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/attribute_update/ip/framed", -1 ,ebufp);
		PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_sucess , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
	}
	nota_buf->flag   = 0;
	nota_buf->size   = flistlen - 2;
	nota_buf->offset = 0;
	nota_buf->data   = ( caddr_t)flistbuf;
	nota_buf->xbuf_file = ( char *) NULL;
	PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );

	/* OAP sometimes send error status in input */
	if(input_status && (*input_status == status_fail))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        }	

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Job create input flist for attribute_update is " ,job_iflistp);

	PCM_OP(ctxp,PCM_OP_CREATE_OBJ ,0, job_iflistp , &job_oflistp , ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the mso_mta job object for attribute updater " );
		PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
		PIN_ERRBUF_CLEAR(ebufp);
		/*r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_create_job job object create failed for attribute_updater", ebufp);
		*ret_flistp = r_flistp;
		return;*/
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Job create output flist for attribute_update is " ,job_oflistp);
	if(nota_buf)
		free(nota_buf);
        PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
        PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);

	return;
}

EXPORT_OP void
fm_mso_integ_create_ippool_update_job(
	pcm_context_t	*ctxp,
	char			*task_no,
	poid_t			*task_pdp,
	pin_flist_t		*task_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*write_oflistp = NULL;
	pin_flist_t		*write_iflistp = NULL;
	pin_flist_t		*order_iflistp = NULL;
	pin_flist_t		*order_oflistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*job_oflistp = NULL;
	pin_flist_t		*job_iflistp = NULL;

	poid_t		*job_pdp	=	NULL;
	poid_t		*order_pdp	=	NULL;
	poid_t		*device_pdp		= NULL;

	char		*device_type	= NULL;
	char		*order_id		= NULL;
	char		*device_id		= NULL;
	int64		db = 0;
	//int64		db = 1;
	int32		order_status_sucess = 0;
	int32		order_status_failure = 1;
	int32		status = 0;
	int32		*state_id = NULL;
	int32		status_sucess = 1;
	int32		status_fail = 2;
	pin_buf_t	*nota_buf	= NULL;
	char		*flistbuf	 = NULL;
	int			flistlen	 =0;
		
	pin_cookie_t	cookie = NULL;
	int32           *input_status = NULL;	

	if (PIN_ERRBUF_IS_ERR(ebufp))
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_create_ippool_update_job input flist", task_flistp);
	input_status = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_STATUS, 1 , ebufp);
	db = PIN_POID_GET_DB(task_pdp);
	device_type = PIN_FLIST_FLD_GET(task_flistp ,PIN_FLD_DEVICE_TYPE, 1 , ebufp);
	/* why drop let's carry forward */
	/*PIN_FLIST_FLD_DROP(task_flistp,PIN_FLD_DEVICE_TYPE,ebufp);*/ 

	PIN_FLIST_TO_STR( task_flistp, &flistbuf, &flistlen, ebufp );

	nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
	if ( nota_buf == NULL )
	{
		pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
		 PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
		 return;
	}
	/*
	 * Create flist for job order creation
	 */
	job_iflistp	= PIN_FLIST_CREATE(ebufp);

	if((device_type) && (strcmp(device_type ,"IP/STATIC") == 0))
	{
		job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/attribute_update/ip/static", -1 ,ebufp);
		PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_sucess , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
	}
	else if((device_type) && (strcmp(device_type ,"IP/FRAMED") == 0))
	{
		job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/attribute_update/ip/framed", -1 ,ebufp);
		PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_sucess , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
	} 
	else 
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error Device Type used for ippool updater " );
	}
	nota_buf->flag   = 0;
	nota_buf->size   = flistlen - 2;
	nota_buf->offset = 0;
	nota_buf->data   = ( caddr_t)flistbuf;
	nota_buf->xbuf_file = ( char *) NULL;
	PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, " ippool update sucessfull" , ebufp);

	//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Job create input flist for attribute_update is " ,job_iflistp);
	
	        /* OAP sometimes send error in input */
        if(input_status && (*input_status == status_fail))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

	PCM_OP(ctxp,PCM_OP_CREATE_OBJ ,0, job_iflistp , &job_oflistp , ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the mso_mta job object for ippool updater " );
		PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
		PIN_ERRBUF_CLEAR(ebufp);
		/*r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_create_job job object create failed for attribute_updater", ebufp);
		*ret_flistp = r_flistp;
		return;*/
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " job create output flist is " , job_oflistp);
		PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);

		if(job_oflistp != NULL)
		{
			job_pdp = PIN_FLIST_FLD_GET(job_oflistp , PIN_FLD_POID, 1, ebufp);
		}

		device_id = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_ID , 1, ebufp);
		if(device_id)
		{
			fm_mso_search_device_poid(ctxp,task_flistp, ebufp);
			device_pdp = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_OBJ,1, ebufp);
		}

		 if((PIN_POID_IS_NULL(device_pdp)) )
		 {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Device object not found ");

			pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"Invalid Device Id", ebufp);

			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the device object for ippool update " );
				PIN_ERRBUF_CLEAR(ebufp);
				/********
				 * Update the status of the mso_mta_job to 2 to indiacte the failure record
				 ******/
				job_iflistp	= PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_POID ,job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51040" , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, " Device Id not found " , ebufp);
				PCM_OP(ctxp, PCM_OP_WRITE_FLDS,0 ,job_iflistp , &job_oflistp , ebufp);
				if(PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the job object for ippool update " );
					PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
					PIN_ERRBUF_CLEAR(ebufp);
				}
				PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
				
			}
		 }
		 else
		 {
			/***** 
			 * Prepare flist to update attributes according to device poid 
			 *
			 * 0 PIN_FLD_POID           POID [0] 0.0.0.1 /device/ip/xxx 2311406 6
			 * only for static/framed IP:
			 * 0 MSO_FLD_IP_POOL_NAME    STR [0] "TestPool01"
			 *****/
			 write_iflistp = PIN_FLIST_CREATE(ebufp);
			 PIN_FLIST_FLD_SET(write_iflistp , PIN_FLD_POID , device_pdp , ebufp);
			 PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_PROGRAM_NAME , "Bulk ippool update" , ebufp);
			 
			 char *device_obj_type = (char *)PIN_POID_GET_TYPE(device_pdp);
			 if(strstr(device_obj_type, "/ip/")) 
			 {
				pin_flist_t *ipdata_flistp = PIN_FLIST_ELEM_ADD(write_iflistp, MSO_FLD_IP_DATA, 0, ebufp );
				PIN_FLIST_FLD_COPY(task_flistp, MSO_FLD_IP_POOL_NAME , ipdata_flistp , MSO_FLD_IP_POOL_NAME , ebufp);
			 }
			 else
			 {
			 	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error Device Type used for ippool updater " );
			 }

			 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Device write fields input flist is  " ,write_iflistp);
			 //PCM_OP(ctxp,PCM_OP_WRITE_FLDS,0 , write_iflistp , &write_oflistp , ebufp);
			 PCM_OP(ctxp,PCM_OP_DEVICE_SET_ATTR,0 , write_iflistp , &write_oflistp , ebufp);
		 }

		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the device object for ippool update " );
			PIN_ERRBUF_CLEAR(ebufp);
			/********
			 * Update the status of the mso_mta_job to 2 to indiacte the failure record
			 ******/
			job_iflistp	= PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_POID ,job_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE, "51042" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, " Attribute update failed " , ebufp);
			PCM_OP(ctxp, PCM_OP_WRITE_FLDS,0 ,job_iflistp , &job_oflistp , ebufp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the job object for ippool update " );
				PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
				PIN_ERRBUF_CLEAR(ebufp);
			}
			PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
			/*r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_create_job job object create failed", ebufp);
			*ret_flistp = r_flistp;
			return;*/
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Device ippool updated sucessfully ");
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Device write fields output flist is  " ,write_oflistp);
			PIN_FLIST_DESTROY_EX(&write_iflistp , NULL);
			PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
			
		}
	}
	if(nota_buf)
	free(nota_buf);
	return;
}

EXPORT_OP void
fm_mso_integ_create_grv_uploader_job(
	pcm_context_t	*ctxp,
	char			*task_no,
	poid_t			*task_pdp,
	pin_flist_t		*task_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*refund_oflistp = NULL;
	pin_flist_t		*refund_iflistp = NULL;
	pin_flist_t		*order_iflistp = NULL;
	pin_flist_t		*order_oflistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*job_oflistp = NULL;
	pin_flist_t		*job_iflistp = NULL;

	poid_t		*job_pdp	=	NULL;
	poid_t		*order_pdp	=	NULL;
	poid_t		*device_pdp		= NULL;
	poid_t		*service_pdp		= NULL;
	poid_t		*account_pdp		= NULL;

	char		*device_type	= NULL;
	char		*order_id		= NULL;
	char		*device_id		= NULL;
	char		*service_obj_type = NULL;
	int64		db = 0;
	//int64		db = 1;
	int32		order_status_sucess = 0;
	int32		order_status_failure = 1;
	int32		status = 0;
	int32		*state_id = NULL;
	int32		status_sucess = 1;
	int32		status_fail = 2;
	int32           opcode = MSO_OP_GRV_UPLOAD; // MSO_OP_GRV_UPLOAD
	pin_buf_t	*nota_buf	= NULL;
	char		*flistbuf	 = NULL;
	int			flistlen	 =0;
	int32           *input_status = NULL;
		
	pin_cookie_t	cookie = NULL;
	int	elem_id = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_create_grv_uploader_job input flist", task_flistp);
	input_status = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_STATUS, 1 , ebufp);
	db = PIN_POID_GET_DB(task_pdp);
	PIN_FLIST_TO_STR( task_flistp, &flistbuf, &flistlen, ebufp );

	nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
	if ( nota_buf == NULL )
	{
		pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
		 PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
		 return;
	}
	//
	// Create flist for job order creation
	//
	job_iflistp	= PIN_FLIST_CREATE(ebufp);

	job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/grv_uploader", -1 ,ebufp);
	PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " DEBUG 000 " ,job_iflistp);

	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status, ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " DEBUG 001 " ,job_iflistp);

	nota_buf->flag   = 0;
	nota_buf->size   = flistlen - 2;
	nota_buf->offset = 0;
	nota_buf->data   = ( caddr_t)flistbuf;
	nota_buf->xbuf_file = ( char *) NULL;
	PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, " grv upload sucessfull" , ebufp);

	/* OAP Sometimes send error in input */
	if(input_status && (*input_status == status_fail))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        }

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Job create input flist for grv_uploader is " ,job_iflistp);

	PCM_OP(ctxp,PCM_OP_CREATE_OBJ ,0, job_iflistp , &job_oflistp , ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the mso_mta job object for grv uploadr " );
		PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
		PIN_ERRBUF_CLEAR(ebufp);
		/*r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_create_job job object create failed for grv_uploaderr", ebufp);
		*ret_flistp = r_flistp;
		return;*/
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " job create output flist is " , job_oflistp);
	PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
	PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);

	if(nota_buf)
		free(nota_buf);
	return;
}

void
fm_mso_integ_create_device_pair_job(
	pcm_context_t	*ctxp,
	char			*task_no,
	poid_t			*task_pdp,
	pin_flist_t		*task_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*refund_oflistp = NULL;
	pin_flist_t		*refund_iflistp = NULL;
	pin_flist_t		*order_iflistp = NULL;
	pin_flist_t		*order_oflistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*job_oflistp = NULL;
	pin_flist_t		*job_iflistp = NULL;

	poid_t		*job_pdp	=	NULL;
	poid_t		*order_pdp	=	NULL;
	poid_t		*device_pdp		= NULL;
	poid_t		*service_pdp		= NULL;
	poid_t		*account_pdp		= NULL;

	char		*device_type	= NULL;
	char		*order_id		= NULL;
	char		*device_id		= NULL;
	char		*service_obj_type = NULL;
	int64		db = 0;
	//int64		db = 1;
	int32		order_status_sucess = 0;
	int32		order_status_failure = 1;
	int32		status = 0;
	int32		*state_id = NULL;
	int32		status_sucess = 1;
	int32		status_fail = 2;
	int32           opcode = MSO_OP_DEVICE_PAIR;
	pin_buf_t	*nota_buf	= NULL;
	char		*flistbuf	 = NULL;
	int			flistlen	 =0;
	int32           *input_status = NULL;
		
	pin_cookie_t	cookie = NULL;
	int	elem_id = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_create_device_pair_job input flist", task_flistp);
	input_status = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_STATUS, 1 , ebufp);
	db = PIN_POID_GET_DB(task_pdp);
	PIN_FLIST_TO_STR( task_flistp, &flistbuf, &flistlen, ebufp );

	nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
	if ( nota_buf == NULL )
	{
		pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
		 PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
		 return;
	}
	//
	// Create flist for job order creation
	//
	job_iflistp	= PIN_FLIST_CREATE(ebufp);

	job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/device_pair", -1 ,ebufp);
	PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " DEBUG 000 " ,job_iflistp);

	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status, ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID , task_no , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " DEBUG 001 " ,job_iflistp);

	nota_buf->flag   = 0;
	nota_buf->size   = flistlen - 2;
	nota_buf->offset = 0;
	nota_buf->data   = ( caddr_t)flistbuf;
	nota_buf->xbuf_file = ( char *) NULL;
	PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, " device pair sucessfull" , ebufp);

	/* OAP Sometimes send error in input */
	if(input_status && (*input_status == status_fail))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        }

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Job create input flist for device_pair is " ,job_iflistp);

	PCM_OP(ctxp,PCM_OP_CREATE_OBJ ,0, job_iflistp , &job_oflistp , ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the mso_mta job object for device pair " );
		PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
		PIN_ERRBUF_CLEAR(ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " job create output flist is " , job_oflistp);
	PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
	PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);

	if(nota_buf)
		free(nota_buf);
	return;
}


EXPORT_OP void
fm_mso_integ_create_catv_preactivation_job(
	pcm_context_t	*ctxp,
	char			*task_no,
	poid_t			*task_pdp,
	pin_flist_t		*task_flistp,
	pin_errbuf_t	*ebufp)

{
	pin_flist_t		*write_oflistp = NULL;
	pin_flist_t		*write_iflistp = NULL;
	pin_flist_t		*order_iflistp = NULL;
	pin_flist_t		*order_oflistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*job_oflistp = NULL;
	pin_flist_t		*job_iflistp = NULL;

	poid_t		*job_pdp	=NULL;
	poid_t		*order_pdp	=NULL;
	poid_t		*lco_account_pdp = NULL;
	poid_t		*device_pdp	= NULL;

	char		*device_type	= NULL;
	char		*order_id	= NULL;
	char		*account_no	= NULL;
	char		*device_id	= NULL;
	int64		db = 0;
	//int64		db = 1;
	int32		order_status_sucess = 0;
	int32		order_status_failure = 1;
	int32		status = 0;
	int32		*state_id = NULL;
	int32		status_sucess = 1;
	int32		status_fail = 2;
	int32		opcode = MSO_OP_DEVICE_CATV_PREACTIVATION;
	pin_buf_t	*nota_buf	= NULL;
	char		*flistbuf	= NULL;
	int		flistlen	=0;
		
	pin_cookie_t	cookie = NULL;
	int32           *input_status = NULL;		

	if (PIN_ERRBUF_IS_ERR(ebufp))
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_create_catv_preactivation_job input flist", task_flistp);
	db = PIN_POID_GET_DB(task_pdp);
		input_status = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_STATUS, 1 , ebufp);
		account_no = PIN_FLIST_FLD_GET(task_flistp , PIN_FLD_ACCOUNT_NO, 1 , ebufp);
		if(account_no)
		{
			fm_mso_search_lco_account(ctxp, task_flistp , ebufp);
			lco_account_pdp = PIN_FLIST_FLD_GET(task_flistp , PIN_FLD_ACCOUNT_OBJ,1, ebufp);
			if(PIN_POID_IS_NULL(lco_account_pdp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " No account found in BRM ");

				pin_set_err(ebufp, PIN_ERRLOC_FM,
						PIN_ERRCLASS_SYSTEM_DETERMINATE,
						PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						"Invalid Account Number", ebufp);
			}
			else
			{
				PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ACCOUNT_OBJ,lco_account_pdp , ebufp);
			}
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " No account Number found");
		}
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " No LCO found for the account no ");
			PIN_ERRBUF_CLEAR(ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_POID, PIN_POID_CREATE(db,"/catv_preactivation",-1,ebufp),ebufp);
			
			PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
			
			nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
			if ( nota_buf == NULL )
			{
				pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
				PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
				return;
			}
			nota_buf->flag   = 0;
			nota_buf->size   = flistlen - 2;
			nota_buf->offset = 0;
			nota_buf->data   = ( caddr_t)flistbuf;
			nota_buf->xbuf_file = ( char *) NULL;
			
			/********
			 * Update the status of the mso_mta_job to 2 to indiacte the failure record
			 ******/
			 
			 job_iflistp	= PIN_FLIST_CREATE(ebufp);
			
			job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/catv_preactivation", -1 ,ebufp);
			
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
			
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
			
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
			
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
			
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
			
			PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
			
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51041" , ebufp);
			
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid LCO Account number" , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
		
			        /* OAP sometimes send error in input */
		        if(input_status && (*input_status == status_fail))
        		{
                		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        		}
        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist" ,job_iflistp);

        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);
	
			PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Job create output flist is ", job_oflistp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for catv pre activation " );
				PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
				PIN_ERRBUF_CLEAR(ebufp);
			}
			PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
		}
		else
		{
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_POID, PIN_POID_CREATE(db,"/catv_preactivation",-1,ebufp),ebufp);
			PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
			nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
			if ( nota_buf == NULL )
			{
				pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
				PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
				return;
			}
		
			/*
			* Create flist for job order creation
			*/
			job_iflistp	= PIN_FLIST_CREATE(ebufp);
			job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/catv_preactivation", -1 ,ebufp);
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);

			nota_buf->flag   = 0;
			nota_buf->size   = flistlen - 2;
			nota_buf->offset = 0;
			nota_buf->data   = ( caddr_t)flistbuf;
			nota_buf->xbuf_file = ( char *) NULL;
			PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Job create input flist for catv preactivation is " ,job_iflistp);
			
			        /* OAP sometimes send error in input */
		        if(input_status && (*input_status == status_fail))
        		{
                		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        		}
        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist" ,job_iflistp);

        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

			PCM_OP(ctxp,PCM_OP_CREATE_OBJ ,0, job_iflistp , &job_oflistp , ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the mso_mta job object for catv pre activation " );
				PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
				/*r_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_create_job job object create failed for location_updater", ebufp);
				*ret_flistp = r_flistp;
				return;*/
			}
			else
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " job create output flist is " , job_oflistp);
				PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
			}
		}
		if(nota_buf)
			free(nota_buf);
}

static void 
fm_mso_integ_service_suspension(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**ret_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*task_flistp = NULL;	
	pin_flist_t	*order_iflistp = NULL;
	pin_flist_t	*order_oflistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	
	poid_t		*job_pdp	= NULL;
	poid_t		*order_pdp	= NULL;
	poid_t		*task_pdp	= NULL;

	char		*task_no	= NULL;
	char		*order_id	= NULL;
	int64		db = 0;
	//int64		db = 1;
	int32		order_status_sucess = 0;
	int32		order_status_failure = 1;
	int			elem_id = 0;
	int32		status = 0;
	int32		*state_id = NULL;
	int32		status_sucess = 0;
	int32		suspend_status = 10102;
	int32		opcode	= MSO_OP_CUST_SUSPEND_SERVICE;
	int32		status_fail = 2;
	pin_cookie_t	cookie = NULL;
	poid_t		*user_id	= NULL;
	char		*program_name	= NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_service_suspension input flist", i_flistp);

	order_pdp	= PIN_FLIST_FLD_GET(i_flistp , PIN_FLD_ORDER_OBJ , 1 , ebufp);
	
	db = PIN_POID_GET_DB(order_pdp);

	task_no		= PIN_FLIST_FLD_GET(i_flistp , MSO_FLD_TASK_NAME , 1, ebufp);

	order_id	= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_ORDER_ID , 1, ebufp);
	user_id		= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_USERID , 1, ebufp);
	program_name	= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_PROGRAM_NAME, 1 , ebufp);

	/******
	 * Create the task object in the brm 
	 *****/

	order_iflistp = PIN_FLIST_CREATE(ebufp);
	
	task_pdp = PIN_POID_CREATE(db , "/mso_task/bulk_suspension", -1 , ebufp);
	PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Bulk service suspension" , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_USERID , user_id , ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create for bulk service suspension input flist is " , order_iflistp);
	PCM_OP(ctxp,PCM_OP_CREATE_OBJ,0 , order_iflistp , &order_oflistp , ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the task object " );
		PIN_FLIST_DESTROY_EX(&order_oflistp , NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_service_suspension task object for bulk service suspension failed", ebufp);
		//*ret_flistp = r_flistp;
		//return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create output for bulk service suspension flist is " , order_oflistp);

	if(order_oflistp)
	{
		task_pdp = PIN_FLIST_FLD_GET(order_oflistp , PIN_FLD_POID , 1, ebufp);

		while ((task_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp,MSO_FLD_TASK,&elem_id, 1, &cookie, ebufp))!= NULL)
		{
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_STATUS , &suspend_status , ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_OPCODE,&opcode, ebufp);
			//PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_DESCR , "Bulk_Service_Suspension" , ebufp);
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_USERID , user_id , ebufp);
			fm_mso_integ_create_service_actions_job(ctxp,task_no,task_pdp, task_flistp , ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the service suspension job object " );
				PIN_ERRBUF_CLEAR(ebufp);
			}			
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Task object failed ");
	}
	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID , order_pdp, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp,PIN_FLD_ORDER_ID , order_id , ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS , &order_status_sucess , ebufp);
	//PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Device state updated sucessfully", ebufp);
	task_flistp = PIN_FLIST_ELEM_ADD(*ret_flistp,MSO_FLD_TASK,0, ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_ID , task_no , ebufp);

	PIN_FLIST_DESTROY_EX(&order_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&order_oflistp, NULL);
	return ;
}

static void 
fm_mso_integ_service_reconnection(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**ret_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*task_flistp = NULL;	
	pin_flist_t	*order_iflistp = NULL;
	pin_flist_t	*order_oflistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	
	poid_t		*job_pdp	= NULL;
	poid_t		*order_pdp	= NULL;
	poid_t		*task_pdp	= NULL;

	char		*task_no	= NULL;
	char		*order_id	= NULL;
	int64		db = 0;
	//int64		db = 1;
	int32		order_status_sucess = 0;
	int32		order_status_failure = 1;
	int		elem_id = 0;
	int32		status = 0;
	int32		*state_id = NULL;
	int32		status_sucess = 0;
	int32		suspend_status = 10100;
	int32		opcode	= MSO_OP_CUST_REACTIVATE_SERVICE;
	int32		status_fail = 2;
	pin_cookie_t	cookie = NULL;
	poid_t		*user_id	= NULL;
	char		*program_name	= NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_service_reconnection input flist", i_flistp);

	order_pdp	= PIN_FLIST_FLD_GET(i_flistp , PIN_FLD_ORDER_OBJ , 1 , ebufp);

	db		= PIN_POID_GET_DB(order_pdp);	

	task_no		= PIN_FLIST_FLD_GET(i_flistp , MSO_FLD_TASK_NAME , 1, ebufp);

	order_id	= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_ORDER_ID , 1, ebufp);
	user_id		= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_USERID , 1, ebufp);
	program_name	= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_PROGRAM_NAME, 1 , ebufp);

	/******
	 * Create the task object in the brm 
	 *****/

	order_iflistp = PIN_FLIST_CREATE(ebufp);
	
	task_pdp = PIN_POID_CREATE(db , "/mso_task/bulk_reconnection", -1 , ebufp);
	PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Bulk service reconnection" , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_USERID , user_id , ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create for bulk service reconnection input flist is " , order_iflistp);
	PCM_OP(ctxp,PCM_OP_CREATE_OBJ,0 , order_iflistp , &order_oflistp , ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the task object " );
		PIN_FLIST_DESTROY_EX(&order_oflistp , NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_service_reconnection task object for bulk service reconnection failed", ebufp);
		//*ret_flistp = r_flistp;
		//return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create output for bulk service reconnection flist is " , order_oflistp);

	if(order_oflistp)
	{
		task_pdp = PIN_FLIST_FLD_GET(order_oflistp , PIN_FLD_POID , 1, ebufp);

		while ((task_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp,MSO_FLD_TASK,&elem_id, 1, &cookie, ebufp))!= NULL)
		{
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_STATUS , &suspend_status , ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_OPCODE,&opcode, ebufp);
			//PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_DESCR , "Bulk_Service_Reconnection" , ebufp);
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_USERID , user_id , ebufp);
			fm_mso_integ_create_service_actions_job(ctxp,task_no,task_pdp, task_flistp , ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the service reconnection job object " );
				PIN_ERRBUF_CLEAR(ebufp);
			}			
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Task object failed ");
	}
	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID , order_pdp, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp,PIN_FLD_ORDER_ID , order_id , ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS , &order_status_sucess , ebufp);
	//PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Device state updated sucessfully", ebufp);
	task_flistp = PIN_FLIST_ELEM_ADD(*ret_flistp,MSO_FLD_TASK,0, ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_ID , task_no , ebufp);

	PIN_FLIST_DESTROY_EX(&order_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&order_oflistp, NULL);
	return ;
}

static void 
fm_mso_integ_service_termination(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**ret_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*task_flistp = NULL;	
	pin_flist_t	*order_iflistp = NULL;
	pin_flist_t	*order_oflistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	
	poid_t		*job_pdp	= NULL;
	poid_t		*order_pdp	= NULL;
	poid_t		*task_pdp	= NULL;

	char			*task_no = NULL;
	char			*order_id = NULL;
	char			*program_name = NULL;
	char			*reason = NULL;
	
	int				elem_id = 0;
	int64			db = 0;
	//int64			db = 1;
	int32			order_status_sucess = 0;
	int32			order_status_failure = 1;
	int32			status = 0;
	int32			*state_id = NULL;
	int32			status_sucess = 0;
	int32			suspend_status = 10103;
	int32			opcode = MSO_OP_CUST_TERMINATE_SERVICE;
	int32			status_fail = 2;
	int32			release_flag = 0;
	
	pin_cookie_t	cookie = NULL;
	
	poid_t			*user_id = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_service_termination input flist", i_flistp);

	order_pdp	= PIN_FLIST_FLD_GET(i_flistp , PIN_FLD_ORDER_OBJ , 1 , ebufp);
	
	db		= PIN_POID_GET_DB(order_pdp);

	task_no		= PIN_FLIST_FLD_GET(i_flistp , MSO_FLD_TASK_NAME , 1, ebufp);

	order_id	= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_ORDER_ID , 1, ebufp);
	user_id		= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_USERID , 1, ebufp);
	program_name	= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_PROGRAM_NAME, 1 , ebufp);

	/******
	 * Create the task object in the brm 
	 *****/

	order_iflistp = PIN_FLIST_CREATE(ebufp);
	
	task_pdp = PIN_POID_CREATE(db , "/mso_task/bulk_termination", -1 , ebufp);
	PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Bulk service termination" , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_USERID , user_id , ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create for bulk service termination input flist is " , order_iflistp);
	PCM_OP(ctxp,PCM_OP_CREATE_OBJ,0 , order_iflistp , &order_oflistp , ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the task object " );
		PIN_FLIST_DESTROY_EX(&order_oflistp , NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_service_termination task object for bulk service termination failed", ebufp);
		//*ret_flistp = r_flistp;
		//return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create output for bulk service termination flist is " , order_oflistp);

	if(order_oflistp)
	{
		task_pdp = PIN_FLIST_FLD_GET(order_oflistp , PIN_FLD_POID , 1, ebufp);

		while ((task_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp,MSO_FLD_TASK,&elem_id, 1, &cookie, ebufp))!= NULL)
		{
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_STATUS, &suspend_status, ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_OPCODE, &opcode, ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_FLAGS, &release_flag, ebufp);
			// CATV GRV AD === Included the Termination reason for Bulk Termination cases
			reason = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DESCR, 1, ebufp);
			if(reason && strlen(reason) != 0)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "reason >>  ");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, reason);
			}
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DESCR, reason, ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PROGRAM_NAME, program_name, ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_USERID, user_id, ebufp);
			
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Printing task_flistp .... " );
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_service_termination task_flistp >> ", task_flistp);
			fm_mso_integ_create_service_actions_job(ctxp, task_no, task_pdp, task_flistp, ebufp);
			
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the service termination job object " );
				PIN_ERRBUF_CLEAR(ebufp);
			}
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Task object failed ");
	}
	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID , order_pdp, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp,PIN_FLD_ORDER_ID , order_id , ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS , &order_status_sucess , ebufp);
	//PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Device state updated sucessfully", ebufp);
	task_flistp = PIN_FLIST_ELEM_ADD(*ret_flistp,MSO_FLD_TASK,0, ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_ID , task_no , ebufp);

	PIN_FLIST_DESTROY_EX(&order_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&order_oflistp, NULL);
	return ;
}

EXPORT_OP void
fm_mso_integ_create_service_actions_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp)

{
	pin_flist_t	*order_iflistp = NULL;
	pin_flist_t	*order_oflistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	pin_flist_t	*job_oflistp = NULL;
	pin_flist_t	*job_iflistp = NULL;
	pin_flist_t	*plan_flist = NULL;

	poid_t		*job_pdp	= NULL;
	poid_t		*order_pdp	= NULL;

	poid_t		*lco_account_pdp = NULL;
	poid_t		*device_pdp	 = NULL;
	poid_t		*service_pdp	 = NULL;
	poid_t		*account_pdp	 = NULL;

	char		*device_type	= NULL;
	char		*order_id	= NULL;
	char		*account_no	= NULL;
	char		*device_id	= NULL;
	char		*agreement_no	= NULL;
	int64		db = 0;
	//int64		db = 1;
	int32		order_status_sucess = 0;
	int32		order_status_failure = 1;
	
	int32		status_sucess = 0;
	int32		status_fail = 2;
	
	pin_buf_t	*nota_buf = NULL;
	char		*flistbuf = NULL;
	int		flistlen =0;

	int32		*opcode = NULL ;
	int32		*suspend_status = NULL;
	char		*descr = NULL;
	char		*task_pdp_type	= NULL;
	
	pin_cookie_t	cookie = NULL;
	int32           *input_status = NULL;
	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_create_service_actions_job input flist", task_flistp);
	input_status = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_STATUS, 1 , ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " task_flistp is " ,task_flistp);
	db = PIN_POID_GET_DB(task_pdp);
	task_pdp_type = (char *)PIN_POID_GET_TYPE(task_pdp);

	account_no = PIN_FLIST_FLD_GET(task_flistp , PIN_FLD_ACCOUNT_NO, 1 , ebufp);
	descr = PIN_FLIST_FLD_GET(task_flistp , PIN_FLD_DESCR , 1 , ebufp);
	device_id =  PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_ID , 1 , ebufp);
	opcode = PIN_FLIST_FLD_TAKE(task_flistp,PIN_FLD_OPCODE, 1 , ebufp);
	
	if(device_id)
	{
		fm_mso_search_device_object(ctxp, task_flistp , ebufp);
		service_pdp = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		//account_pdp = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
		PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, " service poid is " ,service_pdp); 
		if(PIN_POID_IS_NULL(service_pdp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Service poid is null");
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " error set");
			}
			PIN_ERRBUF_CLEAR(ebufp);
			char		*flistbuf	= NULL;
			pin_buf_t	*nota_buf	= NULL;
			flistlen = 0;
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_POID, NULL , ebufp);
			//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ACCOUNT_NO ,account_no , ebufp); 
			//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PROGRAM_NAME , "Bulk_Service_action" , ebufp);
			//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DEVICE_ID, device_id , ebufp);
			//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_STATUS , suspend_status, ebufp);
			//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_USERID , PIN_POID_CREATE(db,"/account",1, ebufp), ebufp);

			PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
			PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_OPCODE, ebufp);
			
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERRBUF_CLEAR(ebufp);
			}

			PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
			nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
			if ( nota_buf == NULL )
			{
				pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
				PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
				return;
			}
			nota_buf->flag   = 0;
			nota_buf->size   = flistlen - 2;
			nota_buf->offset = 0;
			nota_buf->data   = ( caddr_t)flistbuf;
			nota_buf->xbuf_file = ( char *) NULL;

			/********
			 * Update the status of the mso_mta_job to 2 to indiacte the failure record
			 ******/
			if(strstr(task_pdp_type,"suspension"))
			{
				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_suspension", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_OPCODE , opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
				PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51040" , ebufp)
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "No service found with given Device ID" , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				//PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ACCOUNT_OBJ , account_pdp , ebufp);
			}
			else if(strstr(task_pdp_type,"reconnection"))
			{
				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_reconnection", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_OPCODE , opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
				PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51040" , ebufp)
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "No service found with given Device ID" , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				//PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ACCOUNT_OBJ , account_pdp , ebufp);

			}
			else if(strstr(task_pdp_type,"termination"))
			{
				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_termination", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_OPCODE , opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
				PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51040" , ebufp)
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "No service found with given Device ID" , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				//PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ACCOUNT_OBJ , account_pdp , ebufp);

			}
			        /* OAP sometimes send error in input */
			if(input_status && (*input_status == status_fail))
        		{
                		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        		}
        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist" ,job_iflistp);

        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

			PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for cancel plan " );
				PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
				PIN_ERRBUF_CLEAR(ebufp);
			}
			PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
			
			if(job_oflistp)
				job_pdp = PIN_FLIST_FLD_GET(job_oflistp , PIN_FLD_POID, 1, ebufp);

			pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"INVALID DEVICE ID", ebufp);
			if(nota_buf)
				free(nota_buf);
		}
		/*else
		{
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_SERVICE_OBJ,service_pdp, ebufp);
		}*/
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Service poid is null");
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " error set");
		}
		PIN_ERRBUF_CLEAR(ebufp);
		char		*flistbuf	= NULL;
		pin_buf_t	*nota_buf	= NULL;
		flistlen = 0;
		PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_POID, NULL , ebufp);
		//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ACCOUNT_NO ,account_no , ebufp); 
		//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PROGRAM_NAME , "Bulk_Service_action" , ebufp);
		//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DEVICE_ID, device_id , ebufp);
		//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_STATUS , suspend_status, ebufp);
		//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_USERID , PIN_POID_CREATE(db,"/account",1, ebufp), ebufp);

		PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
		PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_OPCODE, ebufp);
		
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
		}

		PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
		nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
		if ( nota_buf == NULL )
		{
			pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
			PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
			return;
		}
		nota_buf->flag   = 0;
		nota_buf->size   = flistlen - 2;
		nota_buf->offset = 0;
		nota_buf->data   = ( caddr_t)flistbuf;
		nota_buf->xbuf_file = ( char *) NULL;

		/********
		 * Update the status of the mso_mta_job to 2 to indiacte the failure record
		 ******/
		if(strstr(task_pdp_type,"suspension"))
		{
			job_iflistp = PIN_FLIST_CREATE(ebufp);
			job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_suspension", -1 ,ebufp);
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_OPCODE , opcode , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51041" , ebufp)
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "NO device ID in input" , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
			//PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ACCOUNT_OBJ , account_pdp , ebufp);
		}
		else if(strstr(task_pdp_type,"reconnection"))
		{
			job_iflistp = PIN_FLIST_CREATE(ebufp);
			job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_reconnection", -1 ,ebufp);
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_OPCODE , opcode , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51041" , ebufp)
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "NO device ID in input" , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
			//PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ACCOUNT_OBJ , account_pdp , ebufp);

		}
		else if(strstr(task_pdp_type,"termination"))
		{
			job_iflistp = PIN_FLIST_CREATE(ebufp);
			job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_termination", -1 ,ebufp);
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_OPCODE , opcode , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51041" , ebufp)
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "NO device ID in input" , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
			//PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ACCOUNT_OBJ , account_pdp , ebufp);

		}
		        /* OAP sometimes send error in input */
	        if(input_status && (*input_status == status_fail))
        	{
                	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                	PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                	PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                	PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        	}
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist" ,job_iflistp);

        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

		PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for cancel plan " );
			PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
			PIN_ERRBUF_CLEAR(ebufp);
		}
		PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
		
		if(job_oflistp)
			job_pdp = PIN_FLIST_FLD_GET(job_oflistp , PIN_FLD_POID, 1, ebufp);

		pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"Invalid VC Number", ebufp);
		if(nota_buf)
			free(nota_buf);
		
	}
	if(account_no)
	{
		fm_mso_search_lco_account(ctxp, task_flistp , ebufp);
		lco_account_pdp = PIN_FLIST_FLD_GET(task_flistp , PIN_FLD_ACCOUNT_OBJ,1, ebufp);
		if(PIN_POID_IS_NULL(lco_account_pdp))
		{
			if(PIN_POID_IS_NULL(job_pdp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," no job created prior hence creating ");
				PIN_ERRBUF_CLEAR(ebufp);
				char		*flistbuf	= NULL;
				pin_buf_t	*nota_buf	= NULL;
				flistlen = 0;
				PIN_FLIST_FLD_SET(task_flistp,PIN_FLD_POID, NULL , ebufp);
				//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PROGRAM_NAME , "Bulk_service_actions" , ebufp);
				//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ACCOUNT_NO , account_no , ebufp);
				//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DEVICE_ID , device_id , ebufp);

				PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
				PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
				PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_OPCODE, ebufp);
				
				if(PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERRBUF_CLEAR(ebufp);
				}
				PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
				nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
				if ( nota_buf == NULL )
				{
					pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
					PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
					return;
				}
				nota_buf->flag   = 0;
				nota_buf->size   = flistlen - 2;
				nota_buf->offset = 0;
				nota_buf->data   = ( caddr_t)flistbuf;
				nota_buf->xbuf_file = ( char *) NULL;
				
				/********
				 * Update the status of the mso_mta_job to 2 to indiacte the failure record
				 ******/
				if(strstr(task_pdp_type,"suspension"))
				{
					job_iflistp = PIN_FLIST_CREATE(ebufp);
					job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_suspension", -1 ,ebufp);
					PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
					PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_OPCODE , opcode , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
					PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51041" , ebufp)
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid Account number" , ebufp);
					PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
					PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
					//PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ACCOUNT_OBJ , lco_account_pdp , ebufp);
				}
				else if(strstr(task_pdp_type,"reconnection"))
				{
					job_iflistp = PIN_FLIST_CREATE(ebufp);
					job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_reconnection", -1 ,ebufp);
					PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
					PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_OPCODE , opcode , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
					PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51041" , ebufp)
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid Account number" , ebufp);
					PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
					PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
					//PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ACCOUNT_OBJ , lco_account_pdp , ebufp);
				}
				else if(strstr(task_pdp_type,"termination"))
				{
					job_iflistp = PIN_FLIST_CREATE(ebufp);
					job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_termination", -1 ,ebufp);
					PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
					PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_OPCODE , opcode , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
					PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51041" , ebufp)
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid Account number" , ebufp);
					PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
					PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
					//PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ACCOUNT_OBJ , lco_account_pdp , ebufp);
				}
				        /* OAP sometimes send error in input */
			        if(input_status && (*input_status == status_fail))
        			{
                			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                			PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                			PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        			}
        			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist" ,job_iflistp);

        			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

				PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
				if(PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for service actions " );
					PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
					PIN_ERRBUF_CLEAR(ebufp);
				}
				PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
				
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " ALready job created for the record with error status");
			}
			pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"Invalid Account Number", ebufp);
			if(nota_buf)
				free(nota_buf);
		}
		else
		{
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_POID,lco_account_pdp , ebufp);
			PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
		}
	}
	else
	{
		if(PIN_POID_IS_NULL(job_pdp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," no job created prior hence creating ");
			PIN_ERRBUF_CLEAR(ebufp);
			char		*flistbuf	= NULL;
			pin_buf_t	*nota_buf	= NULL;
			flistlen = 0;
			PIN_FLIST_FLD_SET(task_flistp,PIN_FLD_POID, NULL , ebufp);
			//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PROGRAM_NAME , "Bulk_service_actions" , ebufp);
			//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ACCOUNT_NO , account_no , ebufp);
			//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DEVICE_ID , device_id , ebufp);

			PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
			PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
			PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_OPCODE, ebufp);
			
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERRBUF_CLEAR(ebufp);
			}
			PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
			nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
			if ( nota_buf == NULL )
			{
				pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
				PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
				return;
			}
			nota_buf->flag   = 0;
			nota_buf->size   = flistlen - 2;
			nota_buf->offset = 0;
			nota_buf->data   = ( caddr_t)flistbuf;
			nota_buf->xbuf_file = ( char *) NULL;
			
			/********
			 * Update the status of the mso_mta_job to 2 to indiacte the failure record
			 ******/
			if(strstr(task_pdp_type,"suspension"))
			{
				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_suspension", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_OPCODE , opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
				PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51042" , ebufp)
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Missing Account number in input" , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				//PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ACCOUNT_OBJ , lco_account_pdp , ebufp);
			}
			else if(strstr(task_pdp_type,"reconnection"))
			{
				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_reconnection", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_OPCODE , opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
				PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51042" , ebufp)
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Missing Account number in input" , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				//PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ACCOUNT_OBJ , lco_account_pdp , ebufp);
			}
			else if(strstr(task_pdp_type,"termination"))
			{
				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_termination", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_OPCODE , opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
				PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51042" , ebufp)
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Missing Account number in input" , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				//PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ACCOUNT_OBJ , lco_account_pdp , ebufp);
			}
			        /* OAP sometimes send error in input */
		        if(input_status && (*input_status == status_fail))
        		{
                		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
               			PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        		}
        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist" ,job_iflistp);

        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

			PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for service actions " );
				PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
				PIN_ERRBUF_CLEAR(ebufp);
			}
			PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
			
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " ALready job created for the record with error status");
		}
		pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"Invalid Account Number", ebufp);
		if(nota_buf)
			free(nota_buf);		
	}

	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " No Account found for the account no ");
		PIN_ERRBUF_CLEAR(ebufp);
		
	}
	else
	{
		if(PIN_POID_IS_NULL(job_pdp))
		{
			//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PROGRAM_NAME , "Bulk_service_actions" , ebufp);
			//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_USERID , PIN_POID_CREATE(db,"/account",1, ebufp), ebufp);
			PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
			PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_OPCODE, ebufp);

			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERRBUF_CLEAR(ebufp);
			}

			PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
			nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
			if ( nota_buf == NULL )
			{
				pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
				PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
				return;
			}
			/*
			* Create flist for job order creation
			*/
			if(strstr(task_pdp_type,"suspension"))
			{
				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_suspension", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_OPCODE , opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_sucess , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_POID , job_iflistp ,PIN_FLD_ACCOUNT_OBJ , ebufp);
				//PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ACCOUNT_OBJ , lco_account_pdp , ebufp);
			}
			else if(strstr(task_pdp_type,"reconnection"))
			{
				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_reconnection", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_OPCODE , opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_sucess , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_POID , job_iflistp ,PIN_FLD_ACCOUNT_OBJ , ebufp);
				//PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ACCOUNT_OBJ , lco_account_pdp , ebufp);
			}
			else if(strstr(task_pdp_type,"termination"))
			{
				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_termination", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_OPCODE , opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_sucess , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_POID , job_iflistp ,PIN_FLD_ACCOUNT_OBJ , ebufp);
				//PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ACCOUNT_OBJ , lco_account_pdp , ebufp);
			}
			
			nota_buf->flag   = 0;
			nota_buf->size   = flistlen - 2;
			nota_buf->offset = 0;
			nota_buf->data   = ( caddr_t)flistbuf;
			nota_buf->xbuf_file = ( char *) NULL;
			PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );

			//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Job create input flist for bulk service actions is " ,job_iflistp);
			        /* OAP sometimes send error in input */
		        if(input_status && (*input_status == status_fail))
        		{
                		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        		}
        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist" ,job_iflistp);

        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);


			PCM_OP(ctxp,PCM_OP_CREATE_OBJ ,0, job_iflistp , &job_oflistp , ebufp);

			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the mso_mta job object for bulk service actions " );
				PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
				/*r_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_create_job job object create failed for location_updater", ebufp);
				*ret_flistp = r_flistp;
				return;*/
			}
			else
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " job create output flist is " , job_oflistp);
				PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
				PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
			}
		}
	}
	if(nota_buf)
	free(nota_buf);
	return;
}

static void 
fm_mso_integ_bulk_retrack(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**ret_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*task_flistp = NULL;	
	pin_flist_t	*order_iflistp = NULL;
	pin_flist_t	*order_oflistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	
	poid_t		*job_pdp	= NULL;
	poid_t		*order_pdp	= NULL;
	poid_t		*task_pdp	= NULL;

	char		*task_no	= NULL;
	char		*order_id	= NULL;
	char		*network_node	= NULL;
	int		elem_id = 0;
	int64		db = 0;
	//int64		db = 1;
	int32		order_status_sucess = 0;
	int32		order_status_failure = 1;
	int32		*state_id = NULL;
	int32		status_sucess = 0;
	int32		status_fail = 2;
	pin_cookie_t	cookie = NULL;
	poid_t		*user_id	= NULL;
	char		*program_name	= NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_bulk_retrack input flist", i_flistp);

	order_pdp	= PIN_FLIST_FLD_GET(i_flistp , PIN_FLD_ORDER_OBJ , 1 , ebufp);

	db		= PIN_POID_GET_DB(order_pdp);

	task_no		= PIN_FLIST_FLD_GET(i_flistp , MSO_FLD_TASK_NAME , 1, ebufp);

	order_id	= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_ORDER_ID , 1, ebufp);

	network_node	= PIN_FLIST_FLD_GET(i_flistp ,MSO_FLD_NETWORK_NODE, 1 , ebufp);

	user_id		= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_USERID , 1, ebufp);
	program_name	= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_PROGRAM_NAME, 1 , ebufp);

	order_iflistp = PIN_FLIST_CREATE(ebufp);

	if((network_node) && (strcmp(network_node,"NDS") == 0))
	{	
		task_pdp = PIN_POID_CREATE(db , "/mso_task/bulk_nds_retrack", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Bulk nds retrack" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_USERID , user_id , ebufp);
	}
	else if((network_node) && (strcmp(network_node,"CISCO") == 0))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/bulk_cisco_retrack", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Bulk cisco retrack" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_USERID , user_id , ebufp);
	}
	else if((network_node) && (strcmp(network_node,"CISCO1") == 0))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/bulk_cisco1_retrack", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Bulk cisco1 retrack" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_USERID , user_id , ebufp);
	}
	/******* VERIMATRIX CHANGES - BEGIN - BULK VM RETRACK ****************/		
	else if((network_node) && strstr(network_node,"VM"))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/bulk_vm_retrack", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Bulk vm retrack" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_USERID , user_id , ebufp);
	}
	/******* VERIMATRIX CHANGES - END - BULK VM RETRACK ****************/	
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create for bulk retrack input flist is " , order_iflistp);
	PCM_OP(ctxp,PCM_OP_CREATE_OBJ,0 , order_iflistp , &order_oflistp , ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the task object " );
		PIN_FLIST_DESTROY_EX(&order_oflistp , NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_bulk_retrack task object for bulk retrack failed", ebufp);
		//*ret_flistp = r_flistp;
		//return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create output for bulk retrack flist is " , order_oflistp);

	if(order_oflistp)
	{
		task_pdp = PIN_FLIST_FLD_GET(order_oflistp , PIN_FLD_POID , 1, ebufp);

		while ((task_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp,MSO_FLD_TASK,&elem_id, 1, &cookie, ebufp))!= NULL)
		{
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE,network_node, ebufp);
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_DESCR , "Bulk Retrack" , ebufp);
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_USERID , user_id , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_FLAGS , task_flistp , PIN_FLD_FLAGS , ebufp);
			fm_mso_integ_create_retrack_job(ctxp,task_no,task_pdp, task_flistp , ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the bulk retarck job object " );
				PIN_ERRBUF_CLEAR(ebufp);
			}			
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Task object failed ");
	}
	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID , order_pdp, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp,PIN_FLD_ORDER_ID , order_id , ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS , &order_status_sucess , ebufp);
	//PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Device state updated sucessfully", ebufp);
	task_flistp = PIN_FLIST_ELEM_ADD(*ret_flistp,MSO_FLD_TASK,0, ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_ID , task_no , ebufp);

	PIN_FLIST_DESTROY_EX(&order_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&order_oflistp, NULL);
	return ;
}

/********************************** fm_bulk_hierarchy_change **************************/
static void 
fm_mso_integ_bulk_hierarchy_change(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**ret_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*task_flistp = NULL;	
	pin_flist_t	*order_iflistp = NULL;
	pin_flist_t	*order_oflistp = NULL;
	pin_flist_t	*parent_flistp =NULL;
	pin_flist_t	*r_flistp = NULL;
	
	poid_t		*job_pdp	= NULL;
	poid_t		*order_pdp	= NULL;
	poid_t		*task_pdp	= NULL;

	char		*task_no	= NULL;
	char		*order_id	= NULL;
	char		*network_node	= NULL;
	char		*par_account_no = NULL;
	char		*child_account_no = NULL; 
	int		elem_id = 0;
	int 		elem_par_id = 0;
	//int64		db = 1;
	int64		db = 0;
	int32		order_status_sucess = 0;
	int32		order_status_failure = 1;
	int32		*state_id = NULL;
	int32		status_sucess = 0;
	int32		osd_flags = 23;
	int32		status_fail = 2;
	int32		*callerp = NULL;
	int32		caller = CALLED_BY_OAP;
//	int32		*ord_status = NULL;
	
	pin_cookie_t	cookie = NULL;
	pin_cookie_t    cookie_par = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "bulk_hierarchy_change input flist", i_flistp);

	order_pdp	= PIN_FLIST_FLD_GET(i_flistp , PIN_FLD_ORDER_OBJ , 1 , ebufp);
	
	db              = PIN_POID_GET_DB(order_pdp);
	
	task_no		= PIN_FLIST_FLD_GET(i_flistp , MSO_FLD_TASK_NAME , 1, ebufp);

	order_id	= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_ORDER_ID , 1, ebufp);

	order_iflistp = PIN_FLIST_CREATE(ebufp);
	
	task_pdp = PIN_POID_CREATE(db , "/mso_task/bulk_hierarchy", -1 , ebufp);
	PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Bulk Hierarchy" , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
	//PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
	//PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_USERID , user_id , ebufp);
	
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create for bulk fm_mso_integ_bulk_hierarchy_change input flist is " , order_iflistp);
	PCM_OP(ctxp,PCM_OP_CREATE_OBJ,0 , order_iflistp , &order_oflistp , ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the task object " );
		PIN_FLIST_DESTROY_EX(&order_oflistp , NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_bulk_osd task object for bulk fm_mso_integ_bulk_hierarchy_change failed", ebufp);
		//*ret_flistp = r_flistp;
		//return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create output for bulk fm_mso_integ_bulk_hierarchy_change flist is " , order_oflistp);

	if(order_oflistp)
	{
		task_pdp = PIN_FLIST_FLD_GET(order_oflistp , PIN_FLD_POID , 1, ebufp);

		while ((task_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp,MSO_FLD_TASK,&elem_id, 1, &cookie, ebufp))!= NULL)
		{
		
		    /*while ((parent_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp,PIN_FLD_PARENTS,&elem_par_id, 1, &cookie_par, ebufp))!= NULL)
			{
			       par_account_no	= PIN_FLIST_FLD_GET(parent_flistp ,PIN_FLD_ACCOUNT_NO , 1, ebufp);
				   child_account_no	= PIN_FLIST_FLD_GET(parent_flistp ,PIN_FLD_DESCR , 1, ebufp);
				   
				   PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ACCOUNT_NO,par_account_no, ebufp);
			       PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DESCR,child_account_no, ebufp);
			}*/
		

			//PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_PROGRAM_NAME , "Bulk hierarchy_change" , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, task_flistp, PIN_FLD_PROGRAM_NAME, ebufp );
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, task_flistp, PIN_FLD_USERID, ebufp );
			callerp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PARENT_FLAGS, 1, ebufp);
			if (callerp && *(int32*)callerp == CALLED_BY_MTA )
			{
				caller = CALLED_BY_MTA;
			}

			// Creates the new job object
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , 
			" Task create output for bulk before create obj fm_mso_integ_bulk_hierarchy_change flist is " , task_flistp);
			fm_mso_integ_create_hierarchy_job(ctxp,task_no,task_pdp, task_flistp ,caller, ebufp);
			//ord_status = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_ORDER_STATUS, 1, ebufp);
			
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the bulk bulk_hierarchy_change job object " );
				PIN_ERRBUF_CLEAR(ebufp);
			}			
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Task object failed ");
	}

	

	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID , order_pdp, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp,PIN_FLD_ORDER_ID , order_id , ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS , &order_status_sucess , ebufp);
	//PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Device state updated sucessfully", ebufp);
	task_flistp = PIN_FLIST_ELEM_ADD(*ret_flistp,MSO_FLD_TASK,0, ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_ID , task_no , ebufp);

	PIN_FLIST_DESTROY_EX(&order_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&order_oflistp, NULL);
	return ;
}

/**************************************************************************************/

static void 
fm_mso_integ_bulk_osd(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**ret_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*task_flistp = NULL;	
	pin_flist_t	*order_iflistp = NULL;
	pin_flist_t	*order_oflistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	
	poid_t		*job_pdp	= NULL;
	poid_t		*order_pdp	= NULL;
	poid_t		*task_pdp	= NULL;

	char		*task_no	= NULL;
	char		*order_id	= NULL;
	char		*network_node	= NULL;
	int		elem_id = 0;
	int64		db = 0;
	//int64		db = 1;
	int32		order_status_sucess = 0;
	int32		order_status_failure = 1;
	int32		*state_id = NULL;
	int32		status_sucess = 0;
	int32		osd_flags = 23;
	int32		status_fail = 2;
	pin_cookie_t	cookie = NULL;
	poid_t		*user_id	= NULL;
	char		*program_name	= NULL;
	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_bulk_osd input flist", i_flistp);

	order_pdp	= PIN_FLIST_FLD_GET(i_flistp , PIN_FLD_ORDER_OBJ , 1 , ebufp);

	db		= PIN_POID_GET_DB(order_pdp);

	task_no		= PIN_FLIST_FLD_GET(i_flistp , MSO_FLD_TASK_NAME , 1, ebufp);

	order_id	= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_ORDER_ID , 1, ebufp);

	network_node	= PIN_FLIST_FLD_GET(i_flistp ,MSO_FLD_NETWORK_NODE , 1, ebufp);
	user_id		= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_USERID , 1, ebufp);
	program_name	= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_PROGRAM_NAME, 1 , ebufp);

	order_iflistp = PIN_FLIST_CREATE(ebufp);

	if((network_node) && (strcmp(network_node,"CISCO") == 0))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/bulk_cisco_osd", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Bulk OSD PK" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_USERID , user_id , ebufp);
	}
	else if((network_node) && (strcmp(network_node,"CISCO1") == 0))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/bulk_cisco1_osd", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Bulk CISCO1 OSD" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_USERID , user_id , ebufp);
	}
	/******* VERIMATRIX CHANGES - BEGIN - BULK VM OSD ****************/	
	else if((network_node) && strstr(network_node, "VM"))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/bulk_vm_osd", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Bulk VM OSD" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_USERID , user_id , ebufp);
	}
	/******* VERIMATRIX CHANGES - END - BULK VM OSD ****************/	
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create for bulk osd input flist is " , order_iflistp);
	PCM_OP(ctxp,PCM_OP_CREATE_OBJ,0 , order_iflistp , &order_oflistp , ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the task object " );
		PIN_FLIST_DESTROY_EX(&order_oflistp , NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_bulk_osd task object for bulk osd failed", ebufp);
		//*ret_flistp = r_flistp;
		//return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create output for bulk osd flist is " , order_oflistp);

	if(order_oflistp)
	{
		task_pdp = PIN_FLIST_FLD_GET(order_oflistp , PIN_FLD_POID , 1, ebufp);

		while ((task_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp,MSO_FLD_TASK,&elem_id, 1, &cookie, ebufp))!= NULL)
		{
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE,network_node, ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_FLAGS,&osd_flags, ebufp);
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_DESCR , "Bulk OSD" , ebufp);
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_USERID , user_id , ebufp);
			fm_mso_integ_create_osd_job(ctxp,task_no,task_pdp, task_flistp , ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the bulk osd job object " );
				PIN_ERRBUF_CLEAR(ebufp);
			}			
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Task object failed ");
	}
	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID , order_pdp, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp,PIN_FLD_ORDER_ID , order_id , ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS , &order_status_sucess , ebufp);
	//PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Device state updated sucessfully", ebufp);
	task_flistp = PIN_FLIST_ELEM_ADD(*ret_flistp,MSO_FLD_TASK,0, ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_ID , task_no , ebufp);

	PIN_FLIST_DESTROY_EX(&order_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&order_oflistp, NULL);
	return ;
}

static void 
fm_mso_integ_bulk_cr_dr_note(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**ret_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*task_flistp = NULL;	
	pin_flist_t	*order_iflistp = NULL;
	pin_flist_t	*order_oflistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	
	poid_t		*job_pdp	= NULL;
	poid_t		*order_pdp	= NULL;
	poid_t		*task_pdp	= NULL;

	char		*task_no	= NULL;
	char		*order_id	= NULL;
	int			elem_id = 0;
	int64		db = 0;
	//int64		db = 1;
	int32		currency_inr = 356;
	int32		service_type = 1;
	int32		order_status_sucess = 0;
	int32		order_status_failure = 1;
	pin_cookie_t	cookie = NULL;
	poid_t		*user_id	= NULL;
	char		*program_name	= NULL;
	int32		opcode	= MSO_OP_AR_DEBIT_NOTE;

	pin_flist_t	*arg_flistp = NULL;
	pin_flist_t	*tmp_arg_flistp = NULL;
	char		*template = "select X from /account where F1 = V1 ";
	char		*tmp_data = NULL;
	poid_t		*acct_pdp = NULL;
	time_t          end_t;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_bulk_cr_dr_note input flist", i_flistp);

	order_pdp	= PIN_FLIST_FLD_GET(i_flistp , PIN_FLD_ORDER_OBJ , 0 , ebufp);
	db 		= PIN_POID_GET_DB(order_pdp);
	task_no		= PIN_FLIST_FLD_GET(i_flistp , MSO_FLD_TASK_NAME , 1, ebufp);
	order_id	= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_ORDER_ID , 0, ebufp);
	user_id		= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_USERID , 1, ebufp);
	program_name	= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_PROGRAM_NAME, 1 , ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Mandatory Fields Missing !!");
		goto CLEANUP;
	}

	order_iflistp = PIN_FLIST_CREATE(ebufp);

	task_pdp = PIN_POID_CREATE(db , "/mso_task/bulk_cr_dr_note", -1 , ebufp);
	PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Bulk CR-DR Note" , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_USERID , user_id , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , 
		"Task create for bulk cr-dr note input flist: " , order_iflistp);
	PCM_OP(ctxp,PCM_OP_CREATE_OBJ,0 , order_iflistp , &order_oflistp , ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the task object " );
		PIN_ERRBUF_CLEAR(ebufp);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, 
			"fm_mso_integ_bulk_cr_dr_note task object for bulk bmail failed", ebufp);
		*ret_flistp = r_flistp;
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , 
		"Task create output for cr-dr note flist: " , order_oflistp);

	if(order_oflistp)
	{
		task_pdp = PIN_FLIST_FLD_GET(order_oflistp , PIN_FLD_POID , 1, ebufp);
		while ((task_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp,MSO_FLD_TASK,&elem_id, 1, &cookie, ebufp))!= NULL)
		{

			/*Populating the Account POID*/
			arg_flistp = PIN_FLIST_CREATE(ebufp);
			tmp_arg_flistp = PIN_FLIST_ELEM_ADD (arg_flistp, PIN_FLD_ARGS, 1, ebufp);
			tmp_data = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
			PIN_FLIST_FLD_SET(tmp_arg_flistp, PIN_FLD_ACCOUNT_NO, tmp_data, ebufp);
			fm_mso_get_object_poid(ctxp, template, arg_flistp, &acct_pdp, ebufp);
			PIN_FLIST_DESTROY_EX(&arg_flistp, NULL);
			arg_flistp = NULL;
			if(!PIN_POID_IS_NULL(acct_pdp)){
				PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_POID, acct_pdp, ebufp);
			}

			//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DESCR , "Bulk CR-DR Note ", ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_OPCODE, &opcode, ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PROGRAM_NAME , program_name , ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_USERID , user_id , ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_CURRENCY , &currency_inr , ebufp);
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_SERVICE_TYPE , &service_type , ebufp);
			//tp = (time_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 1, ebufp);
			end_t = pin_virtual_time(NULL);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_END_T , &end_t , ebufp);

			/* Create the Job Object */
			fm_mso_integ_create_crdr_job(ctxp,task_no,task_pdp, task_flistp , ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , 
					"Error in creating the CR-DR job object for task:", task_flistp);
				PIN_ERRBUF_CLEAR(ebufp);
			}
			if (!PIN_POID_IS_NULL(acct_pdp))
			{
				PIN_POID_DESTROY(acct_pdp, ebufp);
				acct_pdp = NULL;
			}
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Task object failed ");
	}
	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID , order_pdp, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp,PIN_FLD_ORDER_ID , order_id , ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS , &order_status_sucess , ebufp);
	//PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Device state updated sucessfully", ebufp);
	task_flistp = PIN_FLIST_ELEM_ADD(*ret_flistp,MSO_FLD_TASK,0, ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_ID , task_no , ebufp);

	CLEANUP:
		PIN_FLIST_DESTROY_EX(&order_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&order_oflistp, NULL);
	return ;
}

void
fm_mso_integ_create_crdr_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*order_iflistp = NULL;
	pin_flist_t	*order_oflistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	pin_flist_t	*job_oflistp = NULL;
	pin_flist_t	*job_iflistp = NULL;
	//int64		db = 1;
	int64		db = 0;	
	int32		status_sucess = 0;
	int32		status_fail = 2;
	int32		*input_status = NULL;
	
	pin_buf_t	*nota_buf = NULL;
	char		*flistbuf = NULL;
	int		flistlen =0;

	int32		*opcode = NULL ;
	int32		*suspend_status = NULL;
	char		*descr = NULL;
	char		*task_pdp_type	= NULL;
	poid_t		*acct_pdp = NULL;
	poid_t		*job_pdp = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_create_crdr_job input task flist", task_flistp);

	opcode = PIN_FLIST_FLD_TAKE(task_flistp,PIN_FLD_OPCODE, 1 , ebufp);
	acct_pdp = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_POID, 1 , ebufp);
	db = PIN_POID_GET_DB(acct_pdp);
	input_status = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_STATUS, 1 , ebufp);

	PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
	nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
	if ( nota_buf == NULL )
	{
		pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
		return;
	}
	nota_buf->flag   = 0;
	nota_buf->size   = flistlen - 2;
	nota_buf->offset = 0;
	nota_buf->data   = ( caddr_t)flistbuf;
	nota_buf->xbuf_file = ( char *) NULL;

	job_iflistp = PIN_FLIST_CREATE(ebufp);
	job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_crdr_note", -1 ,ebufp);
	PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_OPCODE , opcode , ebufp);
	if (PIN_POID_IS_NULL(acct_pdp))
	{
		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51040" , ebufp)
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, 
			"Invalid Account Number" , ebufp);
	}
	else if(input_status && (*input_status == status_fail))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_sucess , ebufp);
	}
	PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
	PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
	PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_integ_create_crdr_job PCM_OP_CREATE_OBJ Input flist", job_iflistp);
	PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for service actions " );
		PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
		PIN_ERRBUF_CLEAR(ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_integ_create_crdr_job PCM_OP_CREATE_OBJ Output flist", job_oflistp);

	PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
	if(nota_buf)
		free(nota_buf);

	return;
}

static void 
fm_mso_integ_bulk_bmail(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**ret_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*task_flistp = NULL;	
	pin_flist_t	*order_iflistp = NULL;
	pin_flist_t	*order_oflistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	
	poid_t		*job_pdp	= NULL;
	poid_t		*order_pdp	= NULL;
	poid_t		*task_pdp	= NULL;

	char		*task_no	= NULL;
	char		*order_id	= NULL;
	char		*network_node	= NULL;
	int		elem_id = 0;
	int64		db = 0;
	//int64		db = 1;
	int32		order_status_sucess = 0;
	int32		order_status_failure = 1;
	int32		*state_id = NULL;
	int32		status_sucess = 0;
	int32		status_fail = 2;
	int32		bmail_flags = 11;
	pin_cookie_t	cookie = NULL;
	poid_t		*user_id	= NULL;
	char		*program_name	= NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_bulk_email input flist", i_flistp);

	order_pdp	= PIN_FLIST_FLD_GET(i_flistp , PIN_FLD_ORDER_OBJ , 1 , ebufp);
	
	db		= PIN_POID_GET_DB(order_pdp);

	task_no		= PIN_FLIST_FLD_GET(i_flistp , MSO_FLD_TASK_NAME , 1, ebufp);

	order_id	= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_ORDER_ID , 1, ebufp);

	network_node	= PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_NETWORK_NODE, 1 , ebufp);
	user_id		= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_USERID , 1, ebufp);
	program_name	= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_PROGRAM_NAME, 1 , ebufp);

	order_iflistp = PIN_FLIST_CREATE(ebufp);

	if((network_node) && (strcmp(network_node,"CISCO") == 0))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/bulk_cisco_bmail", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Bulk cisco Bmail Pk" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_USERID , user_id , ebufp);
	}
	else if((network_node) && (strcmp(network_node,"CISCO1") == 0))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/bulk_cisco1_bmail", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Bulk cisco1 Bmail Pk" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_USERID , user_id , ebufp);
	}
	/******* VERIMATRIX CHANGES - BEGIN - BULK VM BMAIL ****************/	
	else if((network_node) && strstr(network_node, "VM"))
	{
		task_pdp = PIN_POID_CREATE(db , "/mso_task/bulk_vm_bmail", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Bulk vm Bmail Pk" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_USERID , user_id , ebufp);
	}
	/******* VERIMATRIX CHANGES - END - BULK VM BMAIL ****************/	
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create for bulk bmail input flist is " , order_iflistp);
	PCM_OP(ctxp,PCM_OP_CREATE_OBJ,0 , order_iflistp , &order_oflistp , ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the task object " );
		PIN_FLIST_DESTROY_EX(&order_oflistp , NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_bulk_bmail task object for bulk bmail failed", ebufp);
		//*ret_flistp = r_flistp;
		//return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create output for bulk bmail flist is " , order_oflistp);

	if(order_oflistp)
	{
		task_pdp = PIN_FLIST_FLD_GET(order_oflistp , PIN_FLD_POID , 1, ebufp);

		while ((task_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp,MSO_FLD_TASK,&elem_id, 1, &cookie, ebufp))!= NULL)
		{
			PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NETWORK_NODE,network_node, ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_FLAGS,&bmail_flags, ebufp);
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_DESCR , " Bulk Bmail ", ebufp);
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_USERID , user_id , ebufp);
			fm_mso_integ_create_bmail_job(ctxp,task_no,task_pdp, task_flistp , ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the bulk bmail job object " );
				PIN_ERRBUF_CLEAR(ebufp);
			}			
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Task object failed ");
	}
	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID , order_pdp, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp,PIN_FLD_ORDER_ID , order_id , ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS , &order_status_sucess , ebufp);
	//PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Device state updated sucessfully", ebufp);
	task_flistp = PIN_FLIST_ELEM_ADD(*ret_flistp,MSO_FLD_TASK,0, ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_ID , task_no , ebufp);

	PIN_FLIST_DESTROY_EX(&order_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&order_oflistp, NULL);
	return ;
}


static void 
fm_mso_integ_bulk_adjustment(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**ret_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*task_flistp = NULL;	
	pin_flist_t	*order_iflistp = NULL;
	pin_flist_t	*order_oflistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	
	poid_t		*job_pdp	= NULL;
	poid_t		*order_pdp	= NULL;
	poid_t		*task_pdp	= NULL;

	char		*task_no	= NULL;
	char		*order_id	= NULL;
	int32		*adjustment_type = NULL;
	int		elem_id = 0;
	int64		db = 0;
	//int64		db = 1;
	int32		order_status_sucess = 0;
	int32		order_status_failure = 1;
	int32		*state_id = NULL;
	int32		status_sucess = 0;
	int32		status_fail = 2;
	pin_cookie_t	cookie = NULL;
	poid_t		*user_id	= NULL;
	char		*program_name	= NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_bulk_adjustment input flist", i_flistp);

	order_pdp	= PIN_FLIST_FLD_GET(i_flistp , PIN_FLD_ORDER_OBJ , 1 , ebufp);

	db		= PIN_POID_GET_DB(order_pdp);

	task_no		= PIN_FLIST_FLD_GET(i_flistp , MSO_FLD_TASK_NAME , 1, ebufp);

	order_id	= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_ORDER_ID , 1, ebufp);

	adjustment_type	= PIN_FLIST_FLD_GET(i_flistp , PIN_FLD_ADJUSTMENT_TYPE, 1 , ebufp);
	user_id		= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_USERID , 1, ebufp);
	program_name	= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_PROGRAM_NAME, 1 , ebufp);

	order_iflistp = PIN_FLIST_CREATE(ebufp);

	if((adjustment_type) && (*adjustment_type == 0))
	{	
		order_iflistp = PIN_FLIST_CREATE(ebufp);
		task_pdp = PIN_POID_CREATE(db , "/mso_task/bulk_act_adj", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Bulk account_adjustment" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_USERID , user_id , ebufp);
	}
	else if((adjustment_type) && (*adjustment_type == 1))
	{
		order_iflistp = PIN_FLIST_CREATE(ebufp);
		task_pdp = PIN_POID_CREATE(db , "/mso_task/bulk_bill_adj", -1 , ebufp);
		PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Bulk bill adjustment" , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
		PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_USERID , user_id , ebufp);
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create for bulk adjustment input flist is " , order_iflistp);
	PCM_OP(ctxp,PCM_OP_CREATE_OBJ,0 , order_iflistp , &order_oflistp , ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the task object " );
		PIN_FLIST_DESTROY_EX(&order_oflistp , NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_bulk_adjustment task object for bulk adjustment failed", ebufp);
		//*ret_flistp = r_flistp;
		//return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create output for bulk adjustment flist is " , order_oflistp);

	if(order_oflistp)
	{
		task_pdp = PIN_FLIST_FLD_GET(order_oflistp , PIN_FLD_POID , 1, ebufp);

		while ((task_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp,MSO_FLD_TASK,&elem_id, 1, &cookie, ebufp))!= NULL)
		{
			//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DESCR," Bulk_Adjustment" , ebufp);
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_USERID , user_id , ebufp);
			fm_mso_integ_create_adjustment_job(ctxp,task_no,task_pdp, task_flistp , ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the bulk adjustment job object " );
				PIN_ERRBUF_CLEAR(ebufp);
			}			
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Task object failed ");
	}
	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID , order_pdp, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp,PIN_FLD_ORDER_ID , order_id , ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS , &order_status_sucess , ebufp);
	//PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Device state updated sucessfully", ebufp);
	task_flistp = PIN_FLIST_ELEM_ADD(*ret_flistp,MSO_FLD_TASK,0, ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_ID , task_no , ebufp);

	PIN_FLIST_DESTROY_EX(&order_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&order_oflistp, NULL);
	return ;
}



/*static void 
fm_mso_integ_change_bouquet_id(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**ret_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*task_flistp = NULL;	
	pin_flist_t	*order_iflistp = NULL;
	pin_flist_t	*order_oflistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	
	poid_t		*job_pdp	= NULL;
	poid_t		*order_pdp	= NULL;
	poid_t		*task_pdp	= NULL;

	char		*task_no	= NULL;
	char		*order_id	= NULL;
	int32		*adjustment_type = NULL;
	int		elem_id = 0;
	int64		db = 1;
	int32		order_status_sucess = 0;
	int32		order_status_failure = 1;
	int32		*state_id = NULL;
	int32		status_sucess = 0;
	int32		status_fail = 2;
	pin_cookie_t	cookie = NULL;

	poid_t		*user_id	= NULL;
	char		*program_name	= NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_change_bouquet_id input flist", i_flistp);

	order_pdp	= PIN_FLIST_FLD_GET(i_flistp , PIN_FLD_ORDER_OBJ , 1 , ebufp);

	task_no		= PIN_FLIST_FLD_GET(i_flistp , MSO_FLD_TASK_NAME , 1, ebufp);

	order_id	= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_ORDER_ID , 1, ebufp);
	user_id		= PIN_FLIST_FLD_GET(in_flistp ,PIN_FLD_USERID , 1, ebufp);
	program_name	= PIN_FLIST_FLD_GET(in_flistp ,PIN_FLD_PROGRAM_NAME, 1 , ebufp);

	order_iflistp = PIN_FLIST_CREATE(ebufp);

	order_iflistp = PIN_FLIST_CREATE(ebufp);
	task_pdp = PIN_POID_CREATE(db , "/mso_task/bulk_change_bouquet_id", -1 , ebufp);
	PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Bulk Change Bouquet id" , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_USERID , user_id , ebufp);
		
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create for bulk change bouquet id input flist is " , order_iflistp);
	PCM_OP(ctxp,PCM_OP_CREATE_OBJ,0 , order_iflistp , &order_oflistp , ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the task object " );
		PIN_FLIST_DESTROY_EX(&order_oflistp , NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_change_bouquet_id task object for bulk change bouquet id failed", ebufp);
		//*ret_flistp = r_flistp;
		//return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create output for bulk change bouquet id flist is " , order_oflistp);

	if(order_oflistp)
	{
		task_pdp = PIN_FLIST_FLD_GET(order_oflistp , PIN_FLD_POID , 1, ebufp);

		while ((task_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp,MSO_FLD_TASK,&elem_id, 1, &cookie, ebufp))!= NULL)
		{
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PROGRAM_NAME,"Bulk Change Bouquet id" , ebufp);
			fm_mso_integ_create_bouquet_id_job(ctxp,task_no,task_pdp, task_flistp , ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the bulk bouquet id job object " );
				PIN_ERRBUF_CLEAR(ebufp);
			}			
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Task object failed ");
	}
	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID , order_pdp, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp,PIN_FLD_ORDER_ID , order_id , ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS , &order_status_sucess , ebufp);
	//PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Device state updated sucessfully", ebufp);
	task_flistp = PIN_FLIST_ELEM_ADD(*ret_flistp,MSO_FLD_TASK,0, ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_ID , task_no , ebufp);

	PIN_FLIST_DESTROY_EX(&order_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&order_oflistp, NULL);
	return ;
}*/

static void 
fm_mso_integ_set_personnel_bit(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**ret_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*task_flistp = NULL;	
	pin_flist_t	*order_iflistp = NULL;
	pin_flist_t	*order_oflistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	
	poid_t		*job_pdp	= NULL;
	poid_t		*order_pdp	= NULL;
	poid_t		*task_pdp	= NULL;

	char		*task_no	= NULL;
	char		*order_id	= NULL;
	int32		*adjustment_type = NULL;
	int		elem_id = 0;
	int64		db = 0;
	//int64		db = 1;
	int32		order_status_sucess = 0;
	int32		order_status_failure = 1;
	int32		*state_id = NULL;
	int32		status_sucess = 0;
	int32		status_fail = 2;
	pin_cookie_t	cookie = NULL;
	poid_t		*user_id	= NULL;
	char		*program_name	= NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_set_personnel_bit input flist", i_flistp);

	order_pdp	= PIN_FLIST_FLD_GET(i_flistp , PIN_FLD_ORDER_OBJ , 1 , ebufp);

	db		= PIN_POID_GET_DB(order_pdp);

	task_no		= PIN_FLIST_FLD_GET(i_flistp , MSO_FLD_TASK_NAME , 1, ebufp);

	order_id	= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_ORDER_ID , 1, ebufp);
	user_id		= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_USERID , 1, ebufp);
	program_name	= PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_PROGRAM_NAME, 1 , ebufp);

	order_iflistp = PIN_FLIST_CREATE(ebufp);

	order_iflistp = PIN_FLIST_CREATE(ebufp);
	task_pdp = PIN_POID_CREATE(db , "/mso_task/bulk_set_personnel_bit", -1 , ebufp);
	PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Bulk set personnel bit" , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
	PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_USERID , user_id , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create for bulk set personnel bit input flist is " , order_iflistp);
	PCM_OP(ctxp,PCM_OP_CREATE_OBJ,0 , order_iflistp , &order_oflistp , ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the task object " );
		PIN_FLIST_DESTROY_EX(&order_oflistp , NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_set_personnel_bit task object for bulk set personnel bit failed", ebufp);
		//*ret_flistp = r_flistp;
		//return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create output for bulk set personnel bit flist is " , order_oflistp);

	if(order_oflistp)
	{
		task_pdp = PIN_FLIST_FLD_GET(order_oflistp , PIN_FLD_POID , 1, ebufp);

		while ((task_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp,MSO_FLD_TASK,&elem_id, 1, &cookie, ebufp))!= NULL)
		{
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DESCR,"Bulk_set_personnel_bit" , ebufp);
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
			PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_USERID , user_id , ebufp);
			fm_mso_integ_set_personnel_bit_job(ctxp,task_no,task_pdp, task_flistp , ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the bulk set personnel bit job object ");
				PIN_ERRBUF_CLEAR(ebufp);
			}			
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Task object failed ");
	}
	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID , order_pdp, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp,PIN_FLD_ORDER_ID , order_id , ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS , &order_status_sucess , ebufp);
	//PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Device state updated sucessfully", ebufp);
	task_flistp = PIN_FLIST_ELEM_ADD(*ret_flistp,MSO_FLD_TASK,0, ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_ID , task_no , ebufp);

	PIN_FLIST_DESTROY_EX(&order_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&order_oflistp, NULL);
	return ;
}


EXPORT_OP void
fm_mso_integ_create_retrack_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp)

{
	pin_flist_t	*order_iflistp = NULL;
	pin_flist_t	*order_oflistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	pin_flist_t	*job_oflistp = NULL;
	pin_flist_t	*job_iflistp = NULL;
	pin_flist_t	*plan_flist = NULL;
	pin_flist_t	*read_oflistp = NULL;
	pin_flist_t	*read_iflistp = NULL;

	poid_t		*job_pdp	= NULL;
	poid_t		*order_pdp	= NULL;

	poid_t		*lco_account_pdp = NULL;
	poid_t		*device_pdp	 = NULL;
	poid_t		*service_pdp	 = NULL;
	poid_t		*account_pdp	= NULL;

	char		*order_id	= NULL;
	char		*account_no	= NULL;
	char		*device_id	= NULL;
	
	//int64		db = 1;
	int64		db = 0;	
	int32		status_sucess = 0;
	int32		status_fail = 2;
	
	pin_buf_t	*nota_buf = NULL;
	char		*flistbuf = NULL;
	int		flistlen =0;

	int32		opcode = MSO_OP_PROV_CREATE_ACTION ;
	char		*task_pdp_type	= NULL;
	int32           *input_status = NULL;
	
	pin_cookie_t	cookie = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_create_retrack_job input flist", task_flistp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " task_flistp is " ,task_flistp);

	db = PIN_POID_GET_DB(task_pdp);
	task_pdp_type = (char *)PIN_POID_GET_TYPE(task_pdp);

	
	device_id =  PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_ID , 1 , ebufp);
	input_status = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_STATUS, 1 , ebufp);

	if(device_id)
	{
		fm_mso_search_device_object(ctxp, task_flistp , ebufp);
		service_pdp = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		//account_pdp = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_ACCOUNT_OBJ,1,ebufp);
		PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, " service poid is " ,service_pdp); 
		if(PIN_POID_IS_NULL(service_pdp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Service poid is null");
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " error set");
			}
			PIN_ERRBUF_CLEAR(ebufp);
			char		*flistbuf	= NULL;
			pin_buf_t	*nota_buf	= NULL;
			flistlen = 0;
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_POID, NULL , ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_USERID , PIN_POID_CREATE(db,"/account",1, ebufp), ebufp);

			PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERRBUF_CLEAR(ebufp);
			}

			PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
			nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
			if ( nota_buf == NULL )
			{
				pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
				PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
				return;
			}
			nota_buf->flag   = 0;
			nota_buf->size   = flistlen - 2;
			nota_buf->offset = 0;
			nota_buf->data   = ( caddr_t)flistbuf;
			nota_buf->xbuf_file = ( char *) NULL;
			
			/********
			 * Update the status of the mso_mta_job to 2 to indiacte the failure record
			 ******/
			if(strstr(task_pdp_type,"nds"))
			{
				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_nds_retrack", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
				PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51040" , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				//PIN_FLSIT_FLD_SET(job_iflistp, PIN_FLD_ACCOUNT_OBJ,account_pdp,ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid VC number no service is associated to VC" , ebufp);
			}
			else if(!strcmp(task_pdp_type,"/mso_task/bulk_cisco_retrack"))
			{
				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_cisco_retrack", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
				PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51040" , ebufp)
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid VC number no service is associated to VC" , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				//PIN_FLSIT_FLD_SET(job_iflistp, PIN_FLD_ACCOUNT_OBJ,account_pdp,ebufp);

			}
			else if(!strcmp(task_pdp_type,"/mso_task/bulk_cisco1_retrack"))
			{
				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_cisco1_retrack", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
				PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51040" , ebufp)
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid VC number no service is associated to VC" , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				//PIN_FLSIT_FLD_SET(job_iflistp, PIN_FLD_ACCOUNT_OBJ,account_pdp,ebufp);

			}
			/******* VERIMATRIX CHANGES - BEGIN - BULK VM RETRACK ****************/	
			else if(strstr(task_pdp_type,"/mso_task/bulk_vm_retrack"))
			{
				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_vm_retrack", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
				PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51040" , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				//PIN_FLSIT_FLD_SET(job_iflistp, PIN_FLD_ACCOUNT_OBJ,account_pdp,ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid VC and STB number no service is associated to VC" , ebufp);
			}
			/******* VERIMATRIX CHANGES - END - BULK VM RETRACK ****************/	
			
			PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for bulk retrack " );
				PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
				PIN_ERRBUF_CLEAR(ebufp);
			}
			PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
			
			if(job_oflistp)
				job_pdp = PIN_FLIST_FLD_GET(job_oflistp , PIN_FLD_POID, 1, ebufp);

			pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"Invalid VC Number", ebufp);
			if(nota_buf)
				free(nota_buf);
		}
		else
		{
			/*** read the service poid to get the account number ****/
			read_iflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(read_iflistp, PIN_FLD_POID,service_pdp, ebufp);
			PIN_FLIST_FLD_SET(read_iflistp, PIN_FLD_ACCOUNT_OBJ , NULL , ebufp);
			PCM_OP(ctxp, PCM_OP_READ_FLDS , 0, read_iflistp , &read_oflistp , ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " read service output flist is ", read_oflistp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " read flds error");
				PIN_FLIST_DESTROY_EX(&read_oflistp , NULL);
				PIN_ERRBUF_CLEAR(ebufp);
			}
			if(read_oflistp)
			{
				PIN_FLIST_FLD_COPY(read_oflistp ,PIN_FLD_ACCOUNT_OBJ, task_flistp, PIN_FLD_POID, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " task flist after putting poid is " , task_flistp);
			}
		}
	}
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " No Account found for the account no ");
		PIN_ERRBUF_CLEAR(ebufp);
		
	}
	else
	{
		if(PIN_POID_IS_NULL(job_pdp))
		{
			//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_USERID , PIN_POID_CREATE(db,"/account",1, ebufp), ebufp);
			PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERRBUF_CLEAR(ebufp);
			}
			if(strstr(task_pdp_type,"nds"))
			{
				PIN_FLIST_FLD_RENAME(task_flistp, PIN_FLD_DEVICE_ID , MSO_FLD_VC_ID, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERRBUF_CLEAR(ebufp);
				}
			}
			else if(strstr(task_pdp_type,"vm")) // VERIMATRIX
			{
				PIN_FLIST_FLD_RENAME(task_flistp, PIN_FLD_DEVICE_ID , MSO_FLD_STB_ID, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERRBUF_CLEAR(ebufp);
				}
			}			
			else if(strstr(task_pdp_type,"cisco"))
			{
				PIN_FLIST_FLD_RENAME(task_flistp, PIN_FLD_DEVICE_ID , MSO_FLD_STB_ID, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERRBUF_CLEAR(ebufp);
				}
			}
			else if(strstr(task_pdp_type,"cisco1"))
			{
				PIN_FLIST_FLD_RENAME(task_flistp, PIN_FLD_DEVICE_ID , MSO_FLD_STB_ID, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERRBUF_CLEAR(ebufp);
				}
			}
			
			PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
			nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
			if ( nota_buf == NULL )
			{
				pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
				PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
				return;
			}
			/*
			* Create flist for job order creation
			*/
			if(strstr(task_pdp_type,"nds"))
			{
				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_nds_retrack", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_sucess , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_POID , job_iflistp ,PIN_FLD_ACCOUNT_OBJ , ebufp);
				//PIN_FLSIT_FLD_SET(job_iflistp, PIN_FLD_ACCOUNT_OBJ,account_pdp,ebufp);
			}
			else if(!strcmp(task_pdp_type,"/mso_task/bulk_cisco_retrack"))
			{
				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_cisco_retrack", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_sucess , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_POID , job_iflistp ,PIN_FLD_ACCOUNT_OBJ , ebufp);
			}
			else if(!strcmp(task_pdp_type,"/mso_task/bulk_cisco1_retrack"))
			{
				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_cisco1_retrack", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_sucess , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_POID , job_iflistp ,PIN_FLD_ACCOUNT_OBJ , ebufp);
			}
			/******* VERIMATRIX CHANGES - BEGIN - BULK VM RETRACK ****************/	
			else if(strstr(task_pdp_type,"/mso_task/bulk_vm_retrack"))
			{
				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_vm_retrack", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_sucess , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_POID , job_iflistp ,PIN_FLD_ACCOUNT_OBJ , ebufp);
				//PIN_FLSIT_FLD_SET(job_iflistp, PIN_FLD_ACCOUNT_OBJ,account_pdp,ebufp);
			}	
			/******* VERIMATRIX CHANGES - END - BULK VM RETRACK ****************/	
			
			nota_buf->flag   = 0;
			nota_buf->size   = flistlen - 2;
			nota_buf->offset = 0;
			nota_buf->data   = ( caddr_t)flistbuf;
			nota_buf->xbuf_file = ( char *) NULL;
			PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
			//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Job create input flist for bulk retrack actions is " ,job_iflistp);
			/* OAP sometimes send error in input */
			if(input_status && (*input_status == status_fail))
        		{
                		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        		}

			PCM_OP(ctxp,PCM_OP_CREATE_OBJ ,0, job_iflistp , &job_oflistp , ebufp);

			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the mso_mta job object for bulk retrack actions " );
				PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
				/*r_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_create_job job object create failed for location_updater", ebufp);
				*ret_flistp = r_flistp;
				return;*/
			}
			else
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " job create output flist is " , job_oflistp);
				PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
				PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
				PIN_FLIST_DESTROY_EX(&read_oflistp , NULL);
				PIN_FLIST_DESTROY_EX(&read_iflistp , NULL);

			}
		}
	}
	if(nota_buf)
	free(nota_buf);
	return;
}


EXPORT_OP void
fm_mso_integ_create_adjustment_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp)

{
	pin_flist_t	*order_iflistp = NULL;
	pin_flist_t	*order_oflistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	pin_flist_t	*job_oflistp = NULL;
	pin_flist_t	*job_iflistp = NULL;
	pin_flist_t	*plan_flist = NULL;
	pin_flist_t	*read_oflistp = NULL;
	pin_flist_t	*read_iflistp = NULL;

	poid_t		*job_pdp	= NULL;
	poid_t		*order_pdp	= NULL;

	poid_t		*bill_pdp	= NULL;
	poid_t		*device_pdp	= NULL;
	poid_t		*service_pdp	= NULL;
	poid_t		*account_pdp	= NULL;

	char		*order_id	= NULL;
	char		*account_no	= NULL;
	char		*bill_no	= NULL;
	int64		db = 0;
	//int64		db = 1;
	
	int32		status_sucess = 0;
	int32		status_fail = 2;
	int32		*input_status = NULL;
	int32		tax_flag = 1;
	int32		reason_domain_id = 1;
	int32		status = 0;
	int32		acct_flags = 3;
	int32		bill_flag = 0;
	int32		acct_reason_id = 1;
	int32		bill_reason_id = 2;
	int32		*reason_id = NULL;
	int32		*serv_type = NULL;

	pin_buf_t	*nota_buf = NULL;
	char		*flistbuf = NULL;
	int		flistlen =0;

	int32		opcode = MSO_OP_AR_ADJUSTMENT ;
	char		*task_pdp_type	= NULL;
	
	pin_cookie_t	cookie = NULL;
	pin_decimal_t   *ip_amount = NULL;
	int32		no_error_till_now = 1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_create_adjustment_job input flist", task_flistp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " task_flistp is " ,task_flistp);
	db = PIN_POID_GET_DB(task_pdp);
	task_pdp_type = (char *)PIN_POID_GET_TYPE(task_pdp);
	reason_id = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_REASON, 1 , ebufp);
	input_status = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_STATUS, 1 , ebufp);
	PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_REASON_DOMAIN_ID , &reason_domain_id , ebufp);
	//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_TAX_FLAGS, &tax_flag , ebufp);
	PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_STATUS , &status , ebufp);
	//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_USERID , PIN_POID_CREATE(db,"/account",1, ebufp), ebufp);
	serv_type = (int32 *)PIN_FLIST_FLD_GET(task_flistp, MSO_FLD_SERVICE_TYPE, 1 , ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Entered Block adjustment Reason " );

	if(task_pdp_type && strstr(task_pdp_type, "act")) /**** Account level adjustment ****/
	{

		/*
		0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 3148841 0
		0 PIN_FLD_USERID         POID [0] 0.0.0.1 /account 58135 8
		0 PIN_FLD_ACCOUNT_OBJ    POID [0] 0.0.0.1 /account 3148841 0
		0 PIN_FLD_REASON         ENUM [0] 2
		0 PIN_FLD_AMOUNT       DECIMAL [0] 11.5
		0 PIN_FLD_PROGRAM_NAME    STR [0] "Testnap"
		0 PIN_FLD_REASON_CODE     STR [0] "Customer Not Satisfied With Service"
		0 MSO_FLD_SERVICE_TYPE   ENUM [0] 1
		0 PIN_FLD_FLAGS           INT [0] 3
		0 PIN_FLD_DESCR           STR [0] "Broadband test"
		0 PIN_FLD_TAX_FLAGS       INT [0] 2
		*/
		if(reason_id && *reason_id == 1)
		{
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_REASON_CODE,"Settlement against Retention", ebufp);
		}
		else if (reason_id && *reason_id == 2)
		{
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_REASON_CODE,"Double Top Up", ebufp);
		}
		else if (reason_id && *reason_id == 3)
		{
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_REASON_CODE,"Plan Change Done", ebufp);
		}
		else if (reason_id && *reason_id == 4)
		{
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_REASON_CODE,"Wrongly Activated&Terminated", ebufp);
		}
		else if (reason_id && *reason_id == 5)
		{
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_REASON_CODE,"Non Usage", ebufp);
		}
		else if (reason_id && *reason_id == 6)
		{
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_REASON_CODE,"Auto Activation Done", ebufp);
		}	
		else if (reason_id && *reason_id == 7)
		{
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_REASON_CODE,"Delay In Plan Change", ebufp);
		}
		else if (reason_id && *reason_id == 8)
		{
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_REASON_CODE,"Wrong Plan Given", ebufp);
		}
		else if (reason_id && *reason_id == 9)
		{
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_REASON_CODE,"New Wi-Fi router deposit - NWIFID", ebufp);
		}
		else if (reason_id && *reason_id == 10)
		{
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_REASON_CODE,"TDS  - TDS", ebufp);
		}
		else if (reason_id && *reason_id == 11)
		{
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_REASON_CODE,"Penalty - Penalty", ebufp);
		}
		else if (reason_id && *reason_id == 12)
		{
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_REASON_CODE,"CPE charges - CPEADJ", ebufp);
		}	
		else if (reason_id && *reason_id == 13)
		{
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_REASON_CODE,"LPF Charges - LPEADJ", ebufp);
		}
		else if (reason_id && *reason_id == 14)
		{
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_REASON_CODE,"Static IP - AMC", ebufp);
		}
		else if (reason_id && *reason_id == 15)
		{
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_REASON_CODE,"Modem Deposit - DEP", ebufp);
		}
		else if (reason_id && *reason_id == 16)
		{
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_REASON_CODE,"Additional uses waiver - AUCW", ebufp);
		}
		else if (reason_id && *reason_id == 17)
		{
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_REASON_CODE,"CrNoteagainstDrNote(HO)", ebufp);
		}
		else if (reason_id && *reason_id == 18)
		{
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_REASON_CODE,"Discount related", ebufp);
		}
		else if (reason_id && *reason_id == 19)
		{
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_REASON_CODE,"Initial payment refund(Subscribition)", ebufp);
		}
		else if (reason_id && *reason_id == 20)
		{
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_REASON_CODE,"Cr Note for GPON migration", ebufp);
		}
		else if (reason_id && *reason_id == 21)
		{
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_REASON_CODE,"Service Issue", ebufp);
		}
		else if (reason_id && *reason_id == 22)
		{
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_REASON_CODE,"GST", ebufp);
		}
		else if (reason_id && *reason_id == 23)
		{
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_REASON_CODE,"Cheque bounce billing reversal", ebufp);
		}
		else
		{
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_REASON_CODE,"Others", ebufp);
		}
		PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_REASON, reason_id , ebufp);
		
		PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_FLAGS, &acct_flags , ebufp);
		account_no = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_ACCOUNT_NO, 1 , ebufp);

		if(account_no)
		{
			ip_amount = (pin_decimal_t *) PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_AMOUNT, 1, ebufp);
			if (serv_type && *serv_type != 0)
			{
			    if (ip_amount && pbo_decimal_sign(ip_amount, ebufp) == 1)
			    {
				/* Don't Allow +ve amount as credit */
				*input_status = status_fail;
				PIN_FLIST_FLD_SET(task_flistp ,PIN_FLD_ERROR_CODE , "51050" , ebufp);
				PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ERROR_DESCR, "Credit Adjustment can't be +ve", ebufp);
				no_error_till_now = 0;
			    }
			}
			if(no_error_till_now)
			{
				fm_mso_search_lco_account(ctxp, task_flistp , ebufp);
				account_pdp = PIN_FLIST_FLD_GET(task_flistp , PIN_FLD_ACCOUNT_OBJ,1, ebufp);
			}
			
			if(PIN_POID_IS_NULL(account_pdp) && no_error_till_now)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Account poid is null");
				if(PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " error set");
				}
				PIN_ERRBUF_CLEAR(ebufp);
				PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
				nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
				if ( nota_buf == NULL )
				{
					pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
					PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
					return;
				}
				nota_buf->flag   = 0;
				nota_buf->size   = flistlen - 2;
				nota_buf->offset = 0;
				nota_buf->data   = ( caddr_t)flistbuf;
				nota_buf->xbuf_file = ( char *) NULL;

				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_act_adj", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
				PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51041" , ebufp)
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid Account number" , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ACCOUNT_OBJ ,account_pdp, ebufp);

				PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " job create output flistp is " , job_oflistp);
				if(PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for account adjustment" );
					PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
					PIN_ERRBUF_CLEAR(ebufp);
				}
				PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
				
				if(job_oflistp)
					job_pdp = PIN_FLIST_FLD_GET(job_oflistp , PIN_FLD_POID, 1, ebufp);

				/*pin_set_err(ebufp, PIN_ERRLOC_FM,
						PIN_ERRCLASS_SYSTEM_DETERMINATE,
						PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						"Invalid bill Number", ebufp);*/
			}
			else if(input_status && (*input_status == status_fail))
			{

				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input validation failed @ OAP");
				PIN_ERRBUF_CLEAR(ebufp);
				PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
				nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
				if ( nota_buf == NULL )
				{
					pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
					PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
					return;
				}
				nota_buf->flag   = 0;
				nota_buf->size   = flistlen - 2;
				nota_buf->offset = 0;
				nota_buf->data   = ( caddr_t)flistbuf;
				nota_buf->xbuf_file = ( char *) NULL;

				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_act_adj", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
				PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
				PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ACCOUNT_OBJ ,account_pdp, ebufp);

				PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " job create output flistp is " , job_oflistp);
				if(PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for account adjustment" );
					PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
					PIN_ERRBUF_CLEAR(ebufp);
				}
				PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
				
				if(job_oflistp)
					job_pdp = PIN_FLIST_FLD_GET(job_oflistp , PIN_FLD_POID, 1, ebufp);

			}
			else
			{
				fm_mso_integ_ar_get_details(ctxp, task_flistp, ebufp);

				if(PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERRBUF_CLEAR(ebufp);
				}
				PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
				nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
				if ( nota_buf == NULL )
				{
					pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
					PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
					return;
				}
				/*
				* Create flist for job order creation
				*/
				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_act_adj", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_sucess , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ACCOUNT_OBJ ,account_pdp, ebufp);

				nota_buf->flag   = 0;
				nota_buf->size   = flistlen - 2;
				nota_buf->offset = 0;
				nota_buf->data   = ( caddr_t)flistbuf;
				nota_buf->xbuf_file = ( char *) NULL;
				PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );

				//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Job create input flist for bulk account adjustment is " ,job_iflistp);

				PCM_OP(ctxp,PCM_OP_CREATE_OBJ ,0, job_iflistp , &job_oflistp , ebufp);

				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the mso_mta job object for bulk account adjustment " );
					PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
					/*r_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_create_job job object create failed for location_updater", ebufp);
					*ret_flistp = r_flistp;
					return;*/
				}
				else
				{
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " job create output flist is " , job_oflistp);
					PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
					PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
				}
			}
		}
		if(nota_buf)
		free(nota_buf);
	}
	else if(task_pdp_type && strstr(task_pdp_type, "bill")) /**** Bill Level Adjustment****/
	{
		
		//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_DESCR, "Bulk_bill_Adjustment" , ebufp);
		PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_FLAGS, &bill_flag , ebufp);
		PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_REASON_ID , reason_id, ebufp);
		bill_no = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_BILL_NO, 1 , ebufp);

		if(bill_no)
		{
			ip_amount = (pin_decimal_t *) PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_AMOUNT, 1, ebufp);
			if (serv_type && *serv_type != 0)
                        {
			   if (ip_amount && pbo_decimal_sign(ip_amount, ebufp) == 1)
			   {
				/* Don't Allow +ve amount as credit */
				*input_status = status_fail;
				PIN_FLIST_FLD_SET(task_flistp ,PIN_FLD_ERROR_CODE , "51050" , ebufp);
				PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ERROR_DESCR, "Credit Adjustment can't be +ve", ebufp);
				no_error_till_now = 0;
			   }
			}
			if(no_error_till_now)
			{
				fm_mso_search_bill(ctxp,task_flistp , ebufp);
				bill_pdp = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_POID, 1, ebufp);
			}

			if(PIN_POID_IS_NULL(bill_pdp) && no_error_till_now)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
				PIN_ERRBUF_CLEAR(ebufp);
				PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
				nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
				if ( nota_buf == NULL )
				{
					pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
					PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
					return;
				}
				nota_buf->flag   = 0;
				nota_buf->size   = flistlen - 2;
				nota_buf->offset = 0;
				nota_buf->data   = ( caddr_t)flistbuf;
				nota_buf->xbuf_file = ( char *) NULL;

				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_bill_adj", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
				PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51045" , ebufp)
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid Bill number" , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				//PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ACCOUNT_OBJ ,account_pdp, ebufp);
				PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " job create output flistp is " , job_oflistp);
				if(PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for bill adjustment" );
					PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
					PIN_ERRBUF_CLEAR(ebufp);
				}
				PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
				
				if(job_oflistp)
					job_pdp = PIN_FLIST_FLD_GET(job_oflistp , PIN_FLD_POID, 1, ebufp);

				/*pin_set_err(ebufp, PIN_ERRLOC_FM,
						PIN_ERRCLASS_SYSTEM_DETERMINATE,
						PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						"Invalid bill Number", ebufp);*/
			}
			else if(input_status && (*input_status == status_fail))
			{
				PIN_ERRBUF_CLEAR(ebufp);
				PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
				nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
				if ( nota_buf == NULL )
				{
					pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
					PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
					return;
				}
				nota_buf->flag   = 0;
				nota_buf->size   = flistlen - 2;
				nota_buf->offset = 0;
				nota_buf->data   = ( caddr_t)flistbuf;
				nota_buf->xbuf_file = ( char *) NULL;

				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_bill_adj", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
				PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
				PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				//PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ACCOUNT_OBJ ,account_pdp, ebufp);
				PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " job create output flistp is " , job_oflistp);
				if(PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for bill adjustment" );
					PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
					PIN_ERRBUF_CLEAR(ebufp);
				}
				PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
				
				if(job_oflistp)
					job_pdp = PIN_FLIST_FLD_GET(job_oflistp , PIN_FLD_POID, 1, ebufp);
			}
			else
			{
				if(PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERRBUF_CLEAR(ebufp);
				}
				PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
				nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
				if ( nota_buf == NULL )
				{
					pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
					PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
					return;
				}
				/*
				* Create flist for job order creation
				*/
				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_bill_adj", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_sucess , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				//PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ACCOUNT_OBJ ,account_pdp, ebufp);

				nota_buf->flag   = 0;
				nota_buf->size   = flistlen - 2;
				nota_buf->offset = 0;
				nota_buf->data   = ( caddr_t)flistbuf;
				nota_buf->xbuf_file = ( char *) NULL;
				PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );

				//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Job create input flist for bulk bill adjustmnet is " ,job_iflistp);

				PCM_OP(ctxp,PCM_OP_CREATE_OBJ ,0, job_iflistp , &job_oflistp , ebufp);

				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the mso_mta job object for bulk bill adjustment " );
					PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
					/*r_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_create_job job object create failed for location_updater", ebufp);
					*ret_flistp = r_flistp;
					return;*/
				}
				else
				{
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " job create output flist is " , job_oflistp);
					PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
					PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
				}
			}
		}
		if(nota_buf)
		free(nota_buf);
	}
	return;
}

EXPORT_OP void
fm_mso_integ_create_osd_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp)

{
	pin_flist_t	*order_iflistp = NULL;
	pin_flist_t	*order_oflistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	pin_flist_t	*job_oflistp = NULL;
	pin_flist_t	*job_iflistp = NULL;
	pin_flist_t	*tmp_flistp = NULL;
	pin_flist_t	*tmp1_flistp = NULL;
	pin_flist_t	*tmp2_flistp = NULL;
	pin_flist_t	*read_oflistp = NULL;
	pin_flist_t	*read_iflistp = NULL;

	poid_t		*job_pdp	= NULL;
	poid_t		*order_pdp	= NULL;

	poid_t		*lco_account_pdp = NULL;
	poid_t		*device_pdp	 = NULL;
	poid_t		*service_pdp	 = NULL;
	poid_t		*account_pdp	= NULL;

	char		*order_id	= NULL;
	char		*account_no	= NULL;
	char		*device_id	= NULL;
	
	//int64		db = 1;
	int64		db = 0;
	int32		status_sucess = 0;
	int32		status_fail = 2;
	
	pin_buf_t	*nota_buf = NULL;
	char		*flistbuf = NULL;
	int		flistlen =0;

	int32		opcode = MSO_OP_PROV_CREATE_ACTION ;
	char		*task_pdp_type	= NULL;
	char		*title	= NULL;
	char		*message = NULL;
	//char		*grp_name = NULL;
	int32           *input_status = NULL;
	
	pin_cookie_t	cookie = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_create_osd_job input flist", task_flistp);

	db = PIN_POID_GET_DB(task_pdp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " task_flistp is " ,task_flistp);
	input_status = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_STATUS, 1 , ebufp);

	device_id =  PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_ID , 1 , ebufp);
	title = (char *)PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_TITLE, 1, ebufp);
	message = (char *)PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_MESSAGE, 1, ebufp);
	//grp_name = (char *)PIN_FLIST_FLD_GET(task_flistp, MSO_FLD_MSG_GRP_NAME, 1, ebufp);
	task_pdp_type = (char *)PIN_POID_GET_TYPE(task_pdp);

	if(device_id)
	{
		fm_mso_search_device_object(ctxp, task_flistp , ebufp);
		service_pdp = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		//account_pdp = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_ACCOUNT_OBJ,1 , ebufp);
		PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, " service poid is " ,service_pdp); 
		if(PIN_POID_IS_NULL(service_pdp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Service poid is null");
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " error set");
			}
			PIN_ERRBUF_CLEAR(ebufp);
			char		*flistbuf	= NULL;
			pin_buf_t	*nota_buf	= NULL;
			flistlen = 0;
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_POID, NULL , ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_USERID , PIN_POID_CREATE(db,"/account",1, ebufp), ebufp);

			tmp_flistp = PIN_FLIST_ELEM_ADD (task_flistp, MSO_FLD_MSG_TITLE_LIST, 0, ebufp);
			PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_TITLE ,title, ebufp);

			tmp1_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_MSG_CONTENT_LIST, 0, ebufp);
			PIN_FLIST_FLD_SET(tmp1_flistp, PIN_FLD_MESSAGE ,message, ebufp);

			tmp2_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_STB_ID_LIST, 0, ebufp);
			PIN_FLIST_FLD_SET(tmp2_flistp, MSO_FLD_STB_ID ,device_id, ebufp);

			PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERRBUF_CLEAR(ebufp);
			}

			PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
			nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
			if ( nota_buf == NULL )
			{
				pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
				PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
				return;
			}
			nota_buf->flag   = 0;
			nota_buf->size   = flistlen - 2;
			nota_buf->offset = 0;
			nota_buf->data   = ( caddr_t)flistbuf;
			nota_buf->xbuf_file = ( char *) NULL;
			
			/********
			 * Update the status of the mso_mta_job to 2 to indiacte the failure record
			 ******/
			 if(!strcmp(task_pdp_type,"/mso_task/bulk_cisco_osd"))
			{
				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_cisco_osd", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
				PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51040" , ebufp)
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid MAC ID Entered No Service Exists for it" , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				//PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ACCOUNT_OBJ ,account_pdp, ebufp);
				
			}
			else if(!strcmp(task_pdp_type,"/mso_task/bulk_cisco1_osd"))
			{
				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_cisco1_osd", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
				PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51040" , ebufp)
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid MAC ID Entered No Service Exists for it" , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				//PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ACCOUNT_OBJ ,account_pdp, ebufp);
			}
			/******* VERIMATRIX CHANGES - BEGIN - BULK VM OSD ****************/	
			else if(!strcmp(task_pdp_type,"/mso_task/bulk_vm_osd"))
			{
				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_vm_osd", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
				PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51040" , ebufp)
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid MAC ID Entered No Service Exists for it" , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				//PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ACCOUNT_OBJ ,account_pdp, ebufp);
			}	
			/******* VERIMATRIX CHANGES - END - BULK VM OSD ****************/	
			
			PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for bulk osd " );
				PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
				PIN_ERRBUF_CLEAR(ebufp);
			}
			PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
			
			if(job_oflistp)
				job_pdp = PIN_FLIST_FLD_GET(job_oflistp , PIN_FLD_POID, 1, ebufp);

			pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"Invalid VC Number", ebufp);
			if(nota_buf)
				free(nota_buf);
		}
		else
		{
			/*** read the service poid to get the account number ****/
			read_iflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(read_iflistp, PIN_FLD_POID,service_pdp, ebufp);
			PIN_FLIST_FLD_SET(read_iflistp, PIN_FLD_ACCOUNT_OBJ , NULL , ebufp);
			PCM_OP(ctxp, PCM_OP_READ_FLDS , 0, read_iflistp , &read_oflistp , ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " read service output flist is ", read_oflistp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " read flds error");
				PIN_FLIST_DESTROY_EX(&read_oflistp , NULL);
				PIN_ERRBUF_CLEAR(ebufp);
			}
			if(read_oflistp)
			{
				PIN_FLIST_FLD_COPY(read_oflistp ,PIN_FLD_ACCOUNT_OBJ, task_flistp, PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_SERVICE_OBJ , service_pdp , ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " task flist after putting poid is " , task_flistp);
			}
		}
	}
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " No Account found for the account no ");
		PIN_ERRBUF_CLEAR(ebufp);
		
	}
	else
	{
		PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_USERID , PIN_POID_CREATE(db,"/account",1, ebufp), ebufp);
		if(device_id)
		{
			PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
		}
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
		}
		
		tmp_flistp = PIN_FLIST_ELEM_ADD (task_flistp, MSO_FLD_MSG_TITLE_LIST, 0, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_TITLE ,title, ebufp);

		tmp1_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_MSG_CONTENT_LIST, 0, ebufp);
		PIN_FLIST_FLD_SET(tmp1_flistp, PIN_FLD_MESSAGE ,message, ebufp);

		if(device_id)
		{
			tmp2_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_STB_ID_LIST, 0, ebufp);
			PIN_FLIST_FLD_SET(tmp2_flistp, MSO_FLD_STB_ID ,device_id, ebufp);
		}
		else
		{
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_POID , PIN_POID_CREATE(db,"/account",1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_SERVICE_OBJ , PIN_POID_CREATE(db,"/service/catv",1, ebufp), ebufp);
		}
		
		PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
		nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
		if ( nota_buf == NULL )
		{
			pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
			PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
			return;
		}
		/*
		* Create flist for job order creation
		*/
		if(!strcmp(task_pdp_type,"/mso_task/bulk_cisco_osd"))
		{
			job_iflistp = PIN_FLIST_CREATE(ebufp);
			job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_cisco_osd", -1 ,ebufp);
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_sucess , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_POID , job_iflistp ,PIN_FLD_ACCOUNT_OBJ , ebufp);
			//PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ACCOUNT_OBJ ,account_pdp, ebufp);
		}
		else if(!strcmp(task_pdp_type,"/mso_task/bulk_cisco1_osd"))
		{
			job_iflistp = PIN_FLIST_CREATE(ebufp);
			job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_cisco1_osd", -1 ,ebufp);
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_sucess , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_POID , job_iflistp ,PIN_FLD_ACCOUNT_OBJ , ebufp);
			//PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ACCOUNT_OBJ ,account_pdp, ebufp);
		}
		/******* VERIMATRIX CHANGES - BEGIN - BULK VM OSD ****************/	
		else if(!strcmp(task_pdp_type,"/mso_task/bulk_vm_osd"))
		{
			job_iflistp = PIN_FLIST_CREATE(ebufp);
			job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_vm_osd", -1 ,ebufp);
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51040" , ebufp)
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid MAC ID Entered No Service Exists for it" , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
			//PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ACCOUNT_OBJ ,account_pdp, ebufp);
		}		
		/******* VERIMATRIX CHANGES - END - BULK VM OSD ****************/	
		
		nota_buf->flag   = 0;
		nota_buf->size   = flistlen - 2;
		nota_buf->offset = 0;
		nota_buf->data   = ( caddr_t)flistbuf;
		nota_buf->xbuf_file = ( char *) NULL;
		PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );

		//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Job create input flist for bulk osd actions is " ,job_iflistp);
		/* OAP sometimes send error in input */
		if(input_status && (*input_status == status_fail))
        	{
                	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                	PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                	PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                	PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        	}

		PCM_OP(ctxp,PCM_OP_CREATE_OBJ ,0, job_iflistp , &job_oflistp , ebufp);

		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the mso_mta job object for bulk osd actions " );
			PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
			/*r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_create_job job object create failed for location_updater", ebufp);
			*ret_flistp = r_flistp;
			return;*/
		}
		else
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " job create output flist is " , job_oflistp);
			PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
			PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
			PIN_FLIST_DESTROY_EX(&read_oflistp , NULL);
			PIN_FLIST_DESTROY_EX(&read_iflistp , NULL);
		}		
	}
	if(nota_buf)
	free(nota_buf);

	return;
}


EXPORT_OP void
fm_mso_integ_create_hierarchy_job(
	pcm_context_t		*ctxp,
	char			*task_no,
	poid_t			*task_pdp,
	pin_flist_t		*task_flistp,
	int32			caller,
	pin_errbuf_t		*ebufp)

{

	char			*order_id	= NULL;
	char			*par_account_no	= NULL;
	char			*child_account_no	= NULL;
	char			search_template[150]=" select X from /profile/wholesale where F1 = V1 and profile_t.poid_type='/profile/wholesale' ";
	char			search_cc_template[150]=" select X from /profile/customer_care where F1 = V1 and profile_t.poid_type='/profile/customer_care' ";
	char			search_self_template[200]=" select X from /profile where F1 = V1 and profile_t.poid_type = '/profile/customer_care' ";
	char			*flistbuf = NULL;
	char			*task_pdp_type	= NULL;
	char			*area_code=NULL;
	char			*job_par_acnt_no = NULL;
	char			*job_ch_acnt_no  = NULL;
		
	pin_flist_t		*parent_flistp=NULL;
	pin_flist_t		*search_inflistp=NULL;
	pin_flist_t		*args_flistp=NULL;
	pin_flist_t		*results_flistp=NULL;
	pin_flist_t		*search_outflistp=NULL;
	pin_flist_t		*args_acct_obj_flistp = NULL;
	pin_flist_t		*results_wholesale_flistp=NULL;
	pin_flist_t		*args_poid_flistp = NULL;
	pin_flist_t		*args_acc_no_flistp = NULL;
	pin_flist_t		*results_output_flistp =NULL;
	pin_flist_t		*cust_modify_wholesale_flistp=NULL;
	pin_flist_t		*cust_mod_wholesale_flistp=NULL;
	pin_flist_t		*search_custprof_inflistp=NULL;
	pin_flist_t		*args_cc_acct_obj_flistp=NULL;
	pin_flist_t		*args_cc_poid_flistp=NULL;
	pin_flist_t		*args_cc_acc_no_flistp=NULL;
	pin_flist_t		*results_cc_flistp=NULL;
	pin_flist_t		*results_ccare_flistp=NULL;
	pin_flist_t		*par_acnt_info = NULL;
	pin_flist_t		*child_acnt_info = NULL;
	pin_flist_t		*cust_modify_cc_flistp =NULL;
	pin_flist_t		*results_cc_out_flistp=NULL;
	pin_flist_t		*cust_mod_cc_flistp=NULL;
	pin_flist_t		*search_custprof_outflistp=NULL;
	pin_flist_t		*job_iflistp=NULL;
	pin_flist_t		*job_oflistp=NULL;
	pin_flist_t		*search_self_inflistp=NULL;
	pin_flist_t		*args_self_acct_obj_flistp=NULL;
	pin_flist_t		*results_self_flistp=NULL;
	pin_flist_t		*search_self_outflistp=NULL;
	pin_flist_t		*results_self_out_flistp=NULL;
	pin_flist_t		*cust_modify_self_flistp=NULL;
	pin_flist_t		*cust_mod_self_flistp=NULL; 
	pin_flist_t		*err_par_arry=NULL;
	pin_flist_t		*billing_nameinfo=NULL;
	pin_flist_t		*installation_nameinfo=NULL;
	pin_flist_t		*nameinfo_flistp=NULL;
	pin_flist_t		*add_task_arry_flist=NULL;
	pin_flist_t		*resultsprofile_flistp=NULL;
	pin_flist_t		*results_cc_prof = NULL;
	pin_flist_t		*job_par_acnt_info = NULL;
	pin_flist_t		*job_ch_acnt_info = NULL;
	pin_flist_t		*resubmit_job_arr = NULL;
	
	
	int			status_flags = 256;
	int			flags = 256;
	
	int			flistlen =0;
	int32			elem_id=0;
	int32			elem_resluts_id=0;
	int32			elem_resluts_self_id=0;
	int32			*par_business_type=NULL;
	int32			*child_business_type=NULL;
	int32			*pp_type=NULL;
	int32			tmp_par_business_type=-1;
	int32			tmp_child_business_type=-1;
	int32			par_account_type=0;
	int32			child_account_type=0;
	int32			local_child_account_type=0;
	
	int32			elem_resluts_cc_id=0;
	int32			elem_taskid=0;
	int64			db = 0;
	int32			status_sucess = 0;
	int32			status_fail = 2;

	int32			count_bu = 0;
	int32			count_cust = 0;


	poid_t			*acnt_par_pdp=NULL;
	poid_t			*acnt_child_pdp=NULL;
	poid_t			*account_obj=NULL;
	poid_t			*lco_account_obj = NULL;
	poid_t			*profile_obj=NULL;
	poid_t			*dt_obj=NULL;
	poid_t			*jv_obj=NULL;
	poid_t			*lco_obj=NULL;
	poid_t			*sdt_obj=NULL;
	poid_t			*dt_obj_pr=NULL;
	poid_t			*jv_obj_pr=NULL;
	poid_t			*lco_obj_pr=NULL;
	poid_t			*sdt_obj_pr=NULL;
	poid_t			*job_pdp=NULL;
	poid_t			*srch_pdp=NULL;
	
	
	pin_buf_t		*nota_buf = NULL;
	
	int32			opcode = MSO_OP_CUST_MODIFY_CUSTOMER;
	int32			flag_lco_of_cust = 1;

	//int32			*bu_limitp = NULL;
	//int32			*cust_limitp = NULL;
	int32			bu_limit = MAX_BU_LIMIT;
	int32			cust_limit = MAX_CUST_LIMIT;
	int32			err = 0;
	int32			err1 = 0;

	int			parent_null=0;
	int			child_null=0;
	int32			oap_status_succes=0;
	int32			oap_status_fail=2;
	int32			fail_max_cust_exceed=1;
	int32			oap_status = 0;
	int32			*oap_status_ptr = NULL;
	int32			error_status =0;

	pin_cookie_t		cookie = NULL;
	pin_cookie_t 		cookie_results = NULL;
	pin_cookie_t		cookie_self_results=NULL;
	pin_cookie_t		cookie_cc_results=NULL;
	pin_cookie_t		cookie_task=NULL;

	pin_flist_t		*srch_flistp=NULL;
	pin_flist_t		*srch_oflistp=NULL;	
	char			template_parent[128] = "select X from /profile/wholesale where F1 = V1 and profile_t.poid_type = '/profile/wholesale' ";
	int32           *input_status = NULL;
	char			*das_type = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_create_hierarchy_job input flist", task_flistp);
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "task_pdp", task_pdp );
	input_status = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_STATUS, 1 , ebufp);	
	db = PIN_POID_GET_DB(task_pdp);
	oap_status_ptr = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_STATUS , 1 , ebufp);
	if (oap_status_ptr)
	{
		oap_status = *(int32*)oap_status_ptr;
	}

	// Inpiut validation start
	parent_flistp = PIN_FLIST_ELEM_GET(task_flistp,PIN_FLD_PARENTS,PIN_ELEMID_ANY, 1, ebufp);
	if(parent_flistp)
	{
		par_account_no	= PIN_FLIST_FLD_GET(parent_flistp ,PIN_FLD_ACCOUNT_NO , 1, ebufp);
		child_account_no	= PIN_FLIST_FLD_GET(parent_flistp ,PIN_FLD_DESCR , 1, ebufp);
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "NO parent array found!" );
		goto CLEANUP;
	}

	//Read limits from pin.conf
/*	if (!bu_limitp)
	{
		pin_conf("fm_mso_integ", "max_bu_limit", PIN_FLDT_INT, (caddr_t*)&bu_limitp, &err );
		if (bu_limitp != (int32 *) NULL)
		{
			bu_limit = *bu_limitp;
			free(bu_limitp);
		}
		else
		{
			bu_limit = MAX_BU_LIMIT;
		}
	}


	pin_conf("fm_mso_integ", "max_cust_limit", PIN_FLDT_INT, (caddr_t*)&cust_limitp, &err1 );
	if (cust_limitp != (int32 *) NULL)
	{
		cust_limit = *cust_limitp;
		free(cust_limitp);
	}
	else
	{
		cust_limit = MAX_CUST_LIMIT;
	}
*/


	// If input not [passed or oap status is fail(2)
	if((!child_account_no) || (strcmp(child_account_no,"")==0) || (!par_account_no) || (strcmp(par_account_no,"")==0) 
		|| oap_status == oap_status_fail )
	{
		// Error handling start
		if(PIN_POID_IS_NULL(job_pdp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," no job child is null prior hence creating ");
			PIN_ERRBUF_CLEAR(ebufp);
			char		*flistbuf	= NULL;
			pin_buf_t	*nota_buf	= NULL;
			flistlen = 0;
			
			
						
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERRBUF_CLEAR(ebufp);
			}
			add_task_arry_flist= PIN_FLIST_ELEM_ADD(task_flistp, PIN_FLD_RESULTS, 0, ebufp);
			PIN_FLIST_ELEM_COPY(task_flistp, PIN_FLD_PARENTS, 0, add_task_arry_flist, PIN_FLD_PARENTS, 0, ebufp);
			PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_POID, (PIN_POID_CREATE(db, "/", -1 ,ebufp)), ebufp);
			
			PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
			nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
			if ( nota_buf == NULL )
			{
				pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
				PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
				goto CLEANUP;;
			}
			nota_buf->flag   = 0;
			nota_buf->size   = flistlen - 2;
			nota_buf->offset = 0;
			nota_buf->data   = ( caddr_t)flistbuf;
			nota_buf->xbuf_file = ( char *) NULL;
			
			/********
			 * Update the status of the mso_mta_job to 2 to indiacte the failure record
			 ******/
			job_iflistp = PIN_FLIST_CREATE(ebufp);
			job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_hierarchy", -1 ,ebufp);
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_CODE , "51046" , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp  , PIN_FLD_ACCOUNT_OBJ , acnt_child_pdp , ebufp);


			if(oap_status == oap_status_fail)
			{
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Passed as invalid from OAP" , ebufp);
			}
			else
			{
				if ((!child_account_no) || (strcmp(child_account_no,"")==0))
				{
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid Child Account number" , ebufp);
				}
				else if ((!par_account_no) || (strcmp(par_account_no,"")==0))
				{
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid parent Account number" , ebufp);
				}
				
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks failed profile input list", job_iflistp);
			 /* OAP sometimes send error in input */
		        if(input_status && (*input_status == status_fail))
        		{
               	 		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        		}
        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist" ,job_iflistp);

        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

			PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks failed profile output list", job_oflistp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for cancel plan " );
				PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
				PIN_ERRBUF_CLEAR(ebufp);
			}
				PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
		}
		goto CLEANUP;;
		// Error handling end
	}

	// Verification of child and parent accont No
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Par act info task_flistp is " ,par_acnt_info);
	
	// Child account verification
	fm_mso_get_bulkacnt_from_acntno(ctxp, child_account_no, &child_acnt_info, ebufp);
	
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		error_status = 1;
	}
	else
	{
		acnt_child_pdp = PIN_POID_COPY( PIN_FLIST_FLD_GET(child_acnt_info, PIN_FLD_POID, 1, ebufp), ebufp);
		
		child_business_type =PIN_FLIST_FLD_GET(child_acnt_info, PIN_FLD_BUSINESS_TYPE, 1, ebufp);
		
		if(child_acnt_info)
		{
			tmp_child_business_type = *child_business_type;
			child_account_type = tmp_child_business_type/1000000;
		}
	}

	/***************************************************************
	 * Check whether child account is a subscriber accounr or not
	 ***************************************************************/
	if (child_account_type < 90)
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_BAD_VALUE, 0, 0 ,0);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Child account is not a subscriber account", ebufp);
		return;
	}
	
	// parent account verification
	if (error_status == 0)
	{
	
		fm_mso_get_bulkacnt_from_acntno(ctxp, par_account_no, &par_acnt_info, ebufp);
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
			error_status = 2;
		}
		else
		{
			acnt_par_pdp = PIN_POID_COPY( PIN_FLIST_FLD_GET(par_acnt_info, PIN_FLD_POID, 1, ebufp), ebufp);
			par_business_type =PIN_FLIST_FLD_GET(par_acnt_info, PIN_FLD_BUSINESS_TYPE, 1, ebufp);
			

			if(par_acnt_info)
			{
				tmp_par_business_type = *par_business_type;
				par_account_type = tmp_par_business_type/1000000;
			}
		}
	}	
		 
	/***************************************************************
	 * Check whether Parent account is a LCO accounr or not
	 ***************************************************************/
	if (par_account_type != ACCT_TYPE_LCO)
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_BAD_VALUE, 0, 0 ,0);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Parent account is not a LCO account", ebufp);
		return;
	}
	// Allow to change org structure where child is BU only by backend perl utility

/*	if (caller == CALLED_BY_OAP && 
	    (par_account_type != ACCT_TYPE_LCO or child_account_type != ACCT_TYPE_SUBSCRIBER )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Only Customer's LCO is allowed to change using OAP. Please use backent utility for other options", ebufp);
		PIN_ERRBUF_CLEAR(ebufp);

		char		*flistbuf	= NULL;
		pin_buf_t	*nota_buf	= NULL;
		flistlen = 0;

		add_task_arry_flist= PIN_FLIST_ELEM_ADD(task_flistp, PIN_FLD_RESULTS, 0, ebufp);
		PIN_FLIST_ELEM_COPY(task_flistp, PIN_FLD_PARENTS, 0, add_task_arry_flist, PIN_FLD_PARENTS, 0, ebufp);
		PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_POID, (PIN_POID_CREATE(db, "/", -1 ,ebufp)), ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"task_flistp : ",task_flistp);

		PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );

		nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
		if ( nota_buf == NULL )
		{
			pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
			PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
			return;
		}

		nota_buf->flag   = 0;
		nota_buf->size   = flistlen - 2;
		nota_buf->offset = 0;
		nota_buf->data   = ( caddr_t)flistbuf;
		nota_buf->xbuf_file = ( char *) NULL;


		job_iflistp = PIN_FLIST_CREATE(ebufp);
		job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_hierarchy", -1 ,ebufp);
		PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_CODE , "51047" , ebufp)
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Only Customer's LCO is allowed to change using OAP. Please use backent utility for other options" , ebufp);
		PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
		PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ACCOUNT_OBJ , acnt_child_pdp , ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks failed profile input list", job_iflistp);
		PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks failed profile output list", job_oflistp);
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for hierarchy change" );
			PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
			PIN_ERRBUF_CLEAR(ebufp);
		}
		PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
		goto CLEANUP;
	}
*/	
	// Failcase to check for account poid for the input account no
	
	if(error_status == 1 || error_status == 2  || PIN_POID_IS_NULL(acnt_child_pdp) || PIN_POID_IS_NULL(acnt_par_pdp) )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," no job created prior parent type LCO and pptype is null creating ");
		if(PIN_POID_IS_NULL(job_pdp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," no job created prior hence creating ");
			PIN_ERRBUF_CLEAR(ebufp);
			char		*flistbuf	= NULL;
			pin_buf_t	*nota_buf	= NULL;
			flistlen = 0;
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"task_flistp1 : ",task_flistp);
			err_par_arry = PIN_FLIST_ELEM_GET(task_flistp,PIN_FLD_PARENTS,0,1,ebufp);//Fix as PARENTS elem was getting overwritten
			if(err_par_arry)
			{
				PIN_FLIST_FLD_SET(err_par_arry,PIN_FLD_ACCOUNT_NO,par_account_no,ebufp);
				PIN_FLIST_FLD_SET(err_par_arry,PIN_FLD_DESCR,child_account_no,ebufp);
			}
	
			/****************/
			/*1 PIN_FLD_PARENTS ARRAY [0] allocated 20, used 2
			  2 PIN_FLD_ACCOUNT_NO STR [0] "1000001002"
			  2 PIN_FLD_DESCR STR [0] "1000001102"

			/****************/
						
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERRBUF_CLEAR(ebufp);
			}
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"task_flistp2 : ",task_flistp);	
			add_task_arry_flist= PIN_FLIST_ELEM_ADD(task_flistp, PIN_FLD_RESULTS, 0, ebufp);
			PIN_FLIST_ELEM_COPY(task_flistp, PIN_FLD_PARENTS, 0, add_task_arry_flist, PIN_FLD_PARENTS, 0, ebufp);
			PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_POID, (PIN_POID_CREATE(db, "/", -1 ,ebufp)), ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"task_flistp3 : ",task_flistp);

			PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
			nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
			if ( nota_buf == NULL )
			{
				pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
				PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
				goto CLEANUP;;
			}
			nota_buf->flag   = 0;
			nota_buf->size   = flistlen - 2;
			nota_buf->offset = 0;
			nota_buf->data   = ( caddr_t)flistbuf;
			nota_buf->xbuf_file = ( char *) NULL;
			
			/********
			 * Update the status of the mso_mta_job to 2 to indiacte the failure record
			 ******/
			db = PIN_POID_GET_DB(task_pdp);
			job_iflistp = PIN_FLIST_CREATE(ebufp);
			job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_hierarchy", -1 ,ebufp);
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_CODE , "51046" , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ACCOUNT_OBJ , acnt_child_pdp , ebufp);


			if(error_status == 1)
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid Child Account number" , ebufp);

			if(error_status == 2)
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid Parnet Account number" , ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks failed profile input list", job_iflistp);
		        /* OAP sometimes send error in input */
        		if(input_status && (*input_status == status_fail))
        		{
                		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        		}
        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist" ,job_iflistp);

        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

			PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks failed profile output list", job_oflistp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for cancel plan " );
				PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
				PIN_ERRBUF_CLEAR(ebufp);
			}
			PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
		}
	}
	else
	{
		//Search to get all profiles under the Passed value
		
		/*
			
			Input flist for profile search
			
			0 PIN_FLD_POID     POID [0] 0.0.0.1 /search -1 0
			0 PIN_FLD_FLAGS     INT [0] 256
			0 PIN_FLD_TEMPLATE STR [0] "select X from /profile/wholesale 1,/account 2 where 1.F1 = 2.F2 and 2.F3 = V3 "
			0 PIN_FLD_ARGS ARRAY [1] allocated 20, used 1
			1    PIN_FLD_ACCOUNT_OBJ POID [0] NULL
			0 PIN_FLD_ARGS ARRAY [2] allocated 20, used 1
			1    PIN_FLD_POID POID [0] NULL
			0 PIN_FLD_ARGS ARRAY [3] allocated 20, used 1
			1     PIN_FLD_ACCOUNT_NO STR [0] "48450"
			0 PIN_FLD_RESULTS ARRAY [*]
			1 PIN_FLD_ACCOUNT_OBJ        POID [0] NULL
			1 MSO_FLD_WHOLESALE_INFO SUBSTRUCT [0] allocated 20, used 13
			2     MSO_FLD_DT_OBJ         POID [0] NULL
			2     MSO_FLD_JV_OBJ         POID [0] NULL
			2     MSO_FLD_LCO_OBJ        POID [0] NULL
			2     MSO_FLD_SDT_OBJ        POID [0] NULL
			2     PIN_FLD_PARENT         POID [0] NULL
			2     PIN_FLD_CUSTOMER_TYPE   ENUM [0] 0
			
		*/

		
		// start of Fetching Parent profile information
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Identified that child customer type is  non Subscriber");
		db = PIN_POID_GET_DB(task_pdp);
		srch_pdp = PIN_POID_CREATE(db, "/search", (int64)-1, ebufp);
		srch_flistp = PIN_FLIST_CREATE(ebufp);

		PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &flags, ebufp);
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template_parent, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ,acnt_par_pdp, ebufp);

		results_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS,0,ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search  parent profile input list", srch_flistp);
		PCM_OP(ctxp, PCM_OP_SEARCH,0,srch_flistp,&srch_oflistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search  parent profile in output list", srch_oflistp);
		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);	
		
		results_flistp = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS,PIN_ELEMID_ANY,0,ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Invalid Parent account type", ebufp);
			PIN_ERRBUF_CLEAR(ebufp);

			char		*flistbuf	= NULL;
			pin_buf_t	*nota_buf	= NULL;
			flistlen = 0;

			add_task_arry_flist= PIN_FLIST_ELEM_ADD(task_flistp, PIN_FLD_RESULTS, 0, ebufp);
			PIN_FLIST_ELEM_COPY(task_flistp, PIN_FLD_PARENTS, 0, add_task_arry_flist, PIN_FLD_PARENTS, 0, ebufp);
			PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_POID, (PIN_POID_CREATE(db, "/", -1 ,ebufp)), ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"task_flistp : ",task_flistp);

			PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );

			nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
			if ( nota_buf == NULL )
			{
				pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
				PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
				return;
			}

			nota_buf->flag   = 0;
			nota_buf->size   = flistlen - 2;
			nota_buf->offset = 0;
			nota_buf->data   = ( caddr_t)flistbuf;
			nota_buf->xbuf_file = ( char *) NULL;


			job_iflistp = PIN_FLIST_CREATE(ebufp);
			job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_hierarchy", -1 ,ebufp);
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_CODE , "51047" , ebufp)
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid Parent account type" , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ACCOUNT_OBJ , acnt_child_pdp , ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks failed profile input list", job_iflistp);
			 /* OAP sometimes send error in input */
		        if(input_status && (*input_status == status_fail))
        		{
                		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        		}
        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist" ,job_iflistp);

        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

			PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks failed profile output list", job_oflistp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for hierarchy change" );
				PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
				PIN_ERRBUF_CLEAR(ebufp);
			}
			PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
			goto CLEANUP;
		}
		resultsprofile_flistp = PIN_FLIST_SUBSTR_GET(results_flistp, MSO_FLD_WHOLESALE_INFO, 1, ebufp);
			
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks MSO_FLD_WHOLESALE_INFO list", resultsprofile_flistp);
		pp_type = PIN_FLIST_FLD_GET(resultsprofile_flistp, MSO_FLD_PP_TYPE, 1, ebufp );
		// Adding das_type 
		das_type = PIN_FLIST_FLD_GET(resultsprofile_flistp, MSO_FLD_DAS_TYPE, 1, ebufp);
		if (par_account_type == ACCT_TYPE_JV || par_account_type == ACCT_TYPE_HTW )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Identified that parent customer type is JV/HTW");
			jv_obj_pr = PIN_FLIST_FLD_GET(resultsprofile_flistp, MSO_FLD_JV_OBJ, 1, ebufp );
		}
		else if (par_account_type == ACCT_TYPE_DTR)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Identified that parent customer type is DT");
			jv_obj_pr = PIN_FLIST_FLD_GET(resultsprofile_flistp, MSO_FLD_JV_OBJ, 1, ebufp );
			dt_obj_pr = PIN_FLIST_FLD_GET(resultsprofile_flistp, MSO_FLD_DT_OBJ, 1, ebufp );
		}
		else if (par_account_type == ACCT_TYPE_SUB_DTR)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Identified that parent customer type is SDT");
			jv_obj_pr = PIN_FLIST_FLD_GET(resultsprofile_flistp, MSO_FLD_JV_OBJ, 1, ebufp );
			dt_obj_pr = PIN_FLIST_FLD_GET(resultsprofile_flistp, MSO_FLD_DT_OBJ, 1, ebufp );
			sdt_obj_pr = PIN_FLIST_FLD_GET(resultsprofile_flistp, MSO_FLD_SDT_OBJ, 1, ebufp );
		}
		else if (par_account_type == ACCT_TYPE_LCO)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Identified that parent customer type is LCO");
			jv_obj_pr = PIN_FLIST_FLD_GET(resultsprofile_flistp, MSO_FLD_JV_OBJ, 1, ebufp );
			dt_obj_pr = PIN_FLIST_FLD_GET(resultsprofile_flistp, MSO_FLD_DT_OBJ, 1, ebufp );
			sdt_obj_pr = PIN_FLIST_FLD_GET(resultsprofile_flistp, MSO_FLD_SDT_OBJ, 1, ebufp );
			lco_obj_pr = PIN_FLIST_FLD_GET(resultsprofile_flistp, MSO_FLD_LCO_OBJ, 1, ebufp );
			
		}
		else
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Wrong Business type of the Entity", ebufp);
			PIN_ERRBUF_RESET(ebufp);

			char		*flistbuf	= NULL;
			pin_buf_t	*nota_buf	= NULL;
			flistlen = 0;

			add_task_arry_flist= PIN_FLIST_ELEM_ADD(task_flistp, PIN_FLD_RESULTS, 0, ebufp);
			PIN_FLIST_ELEM_COPY(task_flistp, PIN_FLD_PARENTS, 0, add_task_arry_flist, PIN_FLD_PARENTS, 0, ebufp);
			PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_POID, (PIN_POID_CREATE(db, "/", -1 ,ebufp)), ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"task_flistp : ",task_flistp);

			PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );

			nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
			if ( nota_buf == NULL )
			{
				pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
				PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
				goto CLEANUP;;
			}
 
			nota_buf->flag   = 0;
			nota_buf->size   = flistlen - 2;
			nota_buf->offset = 0;
			nota_buf->data   = ( caddr_t)flistbuf;
			nota_buf->xbuf_file = ( char *) NULL;


			job_iflistp = PIN_FLIST_CREATE(ebufp);
			job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_hierarchy", -1 ,ebufp);
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_CODE , "51047" , ebufp)
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid business_type for parent" , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp  , PIN_FLD_ACCOUNT_OBJ , acnt_child_pdp , ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks failed profile input list", job_iflistp);
			/* OAP sometimes send error in input */
		        if(input_status && (*input_status == status_fail))
        		{
                		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        		}
        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist" ,job_iflistp);

        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

			PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks failed profile output list", job_oflistp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for hierarchy change" );
				PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
				PIN_ERRBUF_CLEAR(ebufp);
			}
			goto CLEANUP;
			
		}
		// End of Fetching Parent profile information

//		if( child_account_type != ACCT_TYPE_SUBSCRIBER )
		if( child_account_type != ACCT_TYPE_SUBSCRIBER && child_account_type != ACCT_TYPE_SME_SUBS_BB && child_account_type != ACCT_TYPE_RETAIL_BB && child_account_type != ACCT_TYPE_CORP_SUBS_BB)
		{
			
		
			// start of fetching Child/self/csr profile
			
			db = PIN_POID_GET_DB(task_pdp);
			search_inflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);


			args_acct_obj_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp );


			//PIN_FLIST_FLD_SET(args_acc_no_flistp, MSO_FLD_LCO_OBJ,acnt_child_pdp, ebufp);
			/*1     MSO_FLD_WHOLESALE_INFO SUBSTRUCT [0] allocated 20, used 4
			  2     MSO_FLD_DT_OBJ              POID [0] 0.0.0.0  0 0
			*/

			if(child_account_type == ACCT_TYPE_LCO)
			{
				args_acc_no_flistp = PIN_FLIST_SUBSTR_ADD(args_acct_obj_flistp, MSO_FLD_WHOLESALE_INFO, ebufp );
				PIN_FLIST_FLD_SET(args_acc_no_flistp, MSO_FLD_LCO_OBJ,acnt_child_pdp, ebufp);
				PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);
			}
			else if (child_account_type == ACCT_TYPE_DTR )
			{
				args_acc_no_flistp = PIN_FLIST_SUBSTR_ADD(args_acct_obj_flistp, MSO_FLD_WHOLESALE_INFO, ebufp );
				PIN_FLIST_FLD_SET(args_acc_no_flistp, MSO_FLD_DT_OBJ,acnt_child_pdp, ebufp);
				PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);
			}	
			else if (child_account_type == ACCT_TYPE_SUB_DTR )
			{
				args_acc_no_flistp = PIN_FLIST_SUBSTR_ADD(args_acct_obj_flistp, MSO_FLD_WHOLESALE_INFO, ebufp );
				PIN_FLIST_FLD_SET(args_acc_no_flistp, MSO_FLD_SDT_OBJ,acnt_child_pdp, ebufp);
				PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);
			}
			else if (child_account_type == ACCT_TYPE_CSR)
			{
				PIN_FLIST_FLD_SET(args_acct_obj_flistp, PIN_FLD_ACCOUNT_OBJ,acnt_child_pdp, ebufp);
				PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);				
			}
			else
			{
				PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Child cannot be other than DT/SDT/LCO/CSR/CUST", ebufp);

				char		*flistbuf	= NULL;
				pin_buf_t	*nota_buf	= NULL;
				flistlen = 0;

				add_task_arry_flist= PIN_FLIST_ELEM_ADD(task_flistp, PIN_FLD_RESULTS, 0, ebufp);
				PIN_FLIST_ELEM_COPY(task_flistp, PIN_FLD_PARENTS, 0, add_task_arry_flist, PIN_FLD_PARENTS, 0, ebufp);
				PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_POID, (PIN_POID_CREATE(db, "/", -1 ,ebufp)), ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"task_flistp : ",task_flistp);

				PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );

				nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
				if ( nota_buf == NULL )
				{
					pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
					PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
					goto CLEANUP;
				}

				nota_buf->flag   = 0;
				nota_buf->size   = flistlen - 2;
				nota_buf->offset = 0;
				nota_buf->data   = ( caddr_t)flistbuf;
				nota_buf->xbuf_file = ( char *) NULL;


				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_hierarchy", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_CODE , "51047" , ebufp)
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Child cannot be other than DT/SDT/LCO/CSR/CUST" , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ACCOUNT_OBJ , acnt_child_pdp , ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks failed profile input list", job_iflistp);
				
				 /* OAP sometimes send error in input */
			        if(input_status && (*input_status == status_fail))
        			{
                			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                			PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                			PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        			}
        			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist" ,job_iflistp);

        			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);


				PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks failed profile output list", job_oflistp);
				if(PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for hierarchy change" );
					PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
					PIN_ERRBUF_CLEAR(ebufp);
				}
				goto CLEANUP;
			}

			results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS,PIN_ELEMID_ANY, ebufp );
			PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_ACCOUNT_OBJ,NULL, ebufp);
			PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID,NULL, ebufp);
			results_wholesale_flistp = PIN_FLIST_SUBSTR_ADD(results_flistp, MSO_FLD_WHOLESALE_INFO, ebufp );
			PIN_FLIST_FLD_SET(results_wholesale_flistp, MSO_FLD_DT_OBJ,NULL, ebufp);
			PIN_FLIST_FLD_SET(results_wholesale_flistp, MSO_FLD_JV_OBJ,NULL, ebufp);
			PIN_FLIST_FLD_SET(results_wholesale_flistp, MSO_FLD_LCO_OBJ,NULL, ebufp);
			PIN_FLIST_FLD_SET(results_wholesale_flistp, PIN_FLD_PARENT,NULL, ebufp);
			PIN_FLIST_FLD_SET(results_wholesale_flistp, MSO_FLD_SDT_OBJ,NULL, ebufp);
			PIN_FLIST_FLD_SET(results_wholesale_flistp, PIN_FLD_CUSTOMER_TYPE,NULL, ebufp);
			
						
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks search profile input list", search_inflistp);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks search profile output list", search_outflistp);

			count_cust = fm_mso_get_impacted_cust_count(ctxp, search_inflistp, child_account_type, ebufp);
			count_bu = PIN_FLIST_ELEM_COUNT(search_outflistp, PIN_FLD_RESULTS, ebufp); 

			PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);

			if (count_bu > bu_limit && caller == CALLED_BY_OAP )
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "High count of Business Unit/Users going to be impacted", ebufp);

				char		*flistbuf	= NULL;
				pin_buf_t	*nota_buf	= NULL;
				flistlen = 0;

				add_task_arry_flist= PIN_FLIST_ELEM_ADD(task_flistp, PIN_FLD_RESULTS, 0, ebufp);
				PIN_FLIST_ELEM_COPY(task_flistp, PIN_FLD_PARENTS, 0, add_task_arry_flist, PIN_FLD_PARENTS, 0, ebufp);
				PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_POID, (PIN_POID_CREATE(db, "/", -1 ,ebufp)), ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"task_flistp : ",task_flistp);

				PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );

				nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
				if ( nota_buf == NULL )
				{
					pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
					PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
					return;
				}

				nota_buf->flag   = 0;
				nota_buf->size   = flistlen - 2;
				nota_buf->offset = 0;
				nota_buf->data   = ( caddr_t)flistbuf;
				nota_buf->xbuf_file = ( char *) NULL;


				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_hierarchy", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_CODE , "51047" , ebufp)
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "High count of Business Unit/Users going to be impacted" , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ACCOUNT_OBJ , NULL , ebufp);

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks failed profile input list", job_iflistp);
				        /* OAP sometimes send error in input */
			        if(input_status && (*input_status == status_fail))
        			{
                			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                			PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                			PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        			}
        			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist" ,job_iflistp);

        			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

				PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks failed profile output list", job_oflistp);
				if(PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "High count of Business Unit/Users going to be impacted" );
					PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
					PIN_ERRBUF_CLEAR(ebufp);
				}
				goto CLEANUP;
			}

			if (count_cust > cust_limit && caller == CALLED_BY_OAP )
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "High count of CUSTOMERS going to be impacted", ebufp);

				char		*flistbuf	= NULL;
				pin_buf_t	*nota_buf	= NULL;
				flistlen = 0;

				PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ORDER_STATUS, &fail_max_cust_exceed , ebufp);
				add_task_arry_flist= PIN_FLIST_ELEM_ADD(task_flistp, PIN_FLD_RESULTS, 0, ebufp);
				PIN_FLIST_ELEM_COPY(task_flistp, PIN_FLD_PARENTS, 0, add_task_arry_flist, PIN_FLD_PARENTS, 0, ebufp);
				PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_POID, (PIN_POID_CREATE(db, "/", -1 ,ebufp)), ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"task_flistp : ",task_flistp);

				PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );

				nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
				if ( nota_buf == NULL )
				{
					pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
					PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
					return;
				}

				nota_buf->flag   = 0;
				nota_buf->size   = flistlen - 2;
				nota_buf->offset = 0;
				nota_buf->data   = ( caddr_t)flistbuf;
				nota_buf->xbuf_file = ( char *) NULL;


				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_hierarchy", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_CODE , "51047" , ebufp)
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "High count of CUST going to be impacted" , ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks failed profile input list", job_iflistp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
				 /* OAP sometimes send error in input */
			        if(input_status && (*input_status == status_fail))
        			{
               				 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                			 PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                			 PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                			 PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        			}
        			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation input" ,job_iflistp);

        			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

				PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks failed profile output list", job_oflistp);
				if(PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "High count of CUST going to be impacted" );
					PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
					PIN_ERRBUF_CLEAR(ebufp);
				}
				goto CLEANUP;
			}
			
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
				goto CLEANUP;
			}
						
			//Looping through child/self profile result list
			while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(search_outflistp,
				PIN_FLD_RESULTS,&elem_resluts_id, 1, &cookie_results, ebufp))!= NULL)
			{
			
				char		*flistbuf	= NULL;
				pin_buf_t	*nota_buf	= NULL;
				flistlen = 0;
			
				
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks wholesale search profile input list", results_flistp);
				account_obj = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp );
				profile_obj = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_POID, 1, ebufp );
				if (account_obj )  //except self profile
				{
					fm_mso_get_account_info(ctxp, account_obj, &job_ch_acnt_info, ebufp);
					if (job_ch_acnt_info)
					{
						job_ch_acnt_no  = PIN_FLIST_FLD_GET(job_ch_acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
					}
				}

				results_wholesale_flistp = PIN_FLIST_SUBSTR_GET(results_flistp, MSO_FLD_WHOLESALE_INFO, 1, ebufp);

				/*******************Flist for Modify Customer*************************/
				/*0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 48450 14
				0 PIN_FLD_USERID         POID [0] 0.0.0.1 /account 1 8
				0 PIN_FLD_ACCOUNT_NO      STR [0] "48450"
				0 PIN_FLD_ACCOUNT_OBJ    POID [0] 0.0.0.1 /account 48450 14
				0 PIN_FLD_PROFILE_OBJ    POID [0] 0.0.0.1 /profile/wholesale 48322 14
				0 PIN_FLD_PROGRAM_NAME    STR [0] "OAP|csradmin"
				0 MSO_FLD_WHOLESALE_INFO SUBSTRUCT [0] allocated 20, used 1
				1     MSO_FLD_JV_OBJ        POID [0] 0.0.0.1 /account 264110 17
				1     MSO_FLD_DT_OBJ        POID [0] 0.0.0.1 /account 264110 17
				1     MSO_FLD_SDT_OBJ        POID [0] 0.0.0.1 /account 264110 17
				1     MSO_FLD_LCO_OBJ        POID [0] 0.0.0.1 /account 264110 17
				1     PIN_FLD_PARENT        POID [0] 0.0.0.1 /account 264110 17*/
				/**********************************************************************/					
					
				
				db = PIN_POID_GET_DB(task_pdp);
				cust_modify_wholesale_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks wholesale profile input list", cust_modify_wholesale_flistp);
				PIN_FLIST_FLD_PUT(cust_modify_wholesale_flistp, PIN_FLD_USERID, PIN_POID_CREATE(db, "/account", 1, ebufp), ebufp);
				PIN_FLIST_FLD_SET(cust_modify_wholesale_flistp, PIN_FLD_POID,account_obj, ebufp);
				PIN_FLIST_FLD_SET(cust_modify_wholesale_flistp, PIN_FLD_ACCOUNT_NO,job_ch_acnt_no, ebufp);
				PIN_FLIST_FLD_SET(cust_modify_wholesale_flistp, PIN_FLD_ACCOUNT_OBJ,account_obj, ebufp);
				PIN_FLIST_FLD_SET(cust_modify_wholesale_flistp, PIN_FLD_PROFILE_OBJ,profile_obj, ebufp);
				PIN_FLIST_FLD_SET(cust_modify_wholesale_flistp, PIN_FLD_PROGRAM_NAME,"OAP|BulkHierarchy", ebufp);					
				cust_mod_wholesale_flistp = PIN_FLIST_SUBSTR_ADD(cust_modify_wholesale_flistp, MSO_FLD_WHOLESALE_INFO, ebufp );
				// Copy existing whplesale profile field values 
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks wholesale profile input list1", results_wholesale_flistp);
				PIN_FLIST_FLD_COPY(results_wholesale_flistp, MSO_FLD_DT_OBJ, cust_mod_wholesale_flistp, MSO_FLD_DT_OBJ, ebufp);
				PIN_FLIST_FLD_COPY(results_wholesale_flistp, MSO_FLD_JV_OBJ, cust_mod_wholesale_flistp, MSO_FLD_JV_OBJ, ebufp);
				PIN_FLIST_FLD_COPY(results_wholesale_flistp, MSO_FLD_LCO_OBJ, cust_mod_wholesale_flistp, MSO_FLD_LCO_OBJ, ebufp);
				PIN_FLIST_FLD_COPY(results_wholesale_flistp, MSO_FLD_SDT_OBJ, cust_mod_wholesale_flistp, MSO_FLD_SDT_OBJ, ebufp);
				PIN_FLIST_FLD_COPY(results_wholesale_flistp, PIN_FLD_PARENT, cust_mod_wholesale_flistp, PIN_FLD_PARENT, ebufp);
				local_child_account_type = *(int32*)PIN_FLIST_FLD_GET(results_wholesale_flistp, PIN_FLD_CUSTOMER_TYPE,0, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks wholesale profile input list2", cust_modify_wholesale_flistp);
				if(par_account_type == ACCT_TYPE_LCO)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Parent account type LCO");
					if(   child_account_type == ACCT_TYPE_CSR ||
					      child_account_type == ACCT_TYPE_OPERATION ||
					      child_account_type == ACCT_TYPE_FIELD_SERVICE ||
					      child_account_type == ACCT_TYPE_LOGISTICS ||
					      child_account_type == ACCT_TYPE_COLLECTION_AGENT ||
					      child_account_type == ACCT_TYPE_SALES_PERSON
					   )
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "child account type CSR or OPERATION or FIELD_SERVICE or LOGISTICS or COLLECTION_AGENT or SALES_PERSON");
						PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_JV_OBJ,jv_obj_pr, ebufp);
						PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_DT_OBJ,dt_obj_pr, ebufp);
						PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_SDT_OBJ,sdt_obj_pr, ebufp);
						PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_LCO_OBJ,lco_obj_pr, ebufp);
						PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, PIN_FLD_PARENT,acnt_par_pdp, ebufp);
					}
					else
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Parent is LCO child can only be CSR/CUSTOMER", ebufp);
						PIN_ERRBUF_RESET(ebufp);
						char		*flistbuf	= NULL;
						pin_buf_t	*nota_buf	= NULL;
						flistlen = 0;

						add_task_arry_flist= PIN_FLIST_ELEM_ADD(task_flistp, PIN_FLD_RESULTS, 0, ebufp);
						
						if ( PIN_POID_COMPARE(account_obj, acnt_child_pdp, 0, ebufp ) ==0 )
						{
							PIN_FLIST_ELEM_COPY(task_flistp, PIN_FLD_PARENTS, 0, add_task_arry_flist, PIN_FLD_PARENTS, 0, ebufp);
						}
						else
						{
							resubmit_job_arr = PIN_FLIST_ELEM_ADD(add_task_arry_flist, PIN_FLD_PARENTS, 0, ebufp );
							PIN_FLIST_FLD_SET(resubmit_job_arr,  PIN_FLD_ACCOUNT_NO, child_account_no, ebufp );
							PIN_FLIST_FLD_SET(resubmit_job_arr,  PIN_FLD_DESCR, job_ch_acnt_no , ebufp );
						}
						PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_POID, (PIN_POID_CREATE(db, "/", -1 ,ebufp)), ebufp);
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"task_flistp : ",task_flistp);

						PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );

						nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
						if ( nota_buf == NULL )
						{
							pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
							PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
							goto CLEANUP;
						}

						nota_buf->flag   = 0;
						nota_buf->size   = flistlen - 2;
						nota_buf->offset = 0;
						nota_buf->data   = ( caddr_t)flistbuf;
						nota_buf->xbuf_file = ( char *) NULL;


						job_iflistp = PIN_FLIST_CREATE(ebufp);
						job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_hierarchy", -1 ,ebufp);
						PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
						PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_CODE , "51047" , ebufp)
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Parent is LCO child can only be CSR/CUSTOMER" , ebufp);
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks failed profile input list", job_iflistp);
						PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
						PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
						PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ACCOUNT_OBJ , account_obj , ebufp);
						 /* OAP sometimes send error in input */
        					if(input_status && (*input_status == status_fail))
		        			{
                					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                					PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                					PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        					}
        					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist" ,job_iflistp);

        					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);
						
						PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks failed profile output list", job_oflistp);
						if(PIN_ERRBUF_IS_ERR(ebufp))
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for hierarchy change" );
							PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
							PIN_ERRBUF_CLEAR(ebufp);
						}
						goto CLEANUP_1;
					}
				}

				if(par_account_type == ACCT_TYPE_SUB_DTR)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Parent account type SDT");
					if (child_account_type == ACCT_TYPE_CSR ||
					    child_account_type == ACCT_TYPE_OPERATION ||
					    child_account_type == ACCT_TYPE_FIELD_SERVICE ||
					    child_account_type == ACCT_TYPE_LOGISTICS ||
					    child_account_type == ACCT_TYPE_COLLECTION_AGENT ||
					    child_account_type == ACCT_TYPE_SALES_PERSON
					   )
					{
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks wholesale profile input list3",
							cust_modify_wholesale_flistp);

						PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "SDT POID print",sdt_obj_pr);
						
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "child account type CSR or OPERATION or FIELD_SERVICE or LOGISTICS or COLLECTION_AGENT or SALES_PERSON");
						PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_JV_OBJ,jv_obj_pr, ebufp);
						PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_DT_OBJ,dt_obj_pr, ebufp);
						PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_SDT_OBJ,sdt_obj_pr, ebufp);
						PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_LCO_OBJ,NULL, ebufp);
						PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, PIN_FLD_PARENT,acnt_par_pdp, ebufp);
					
					}
					else if(child_account_type == ACCT_TYPE_LCO)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "child account type LCO");
						if (local_child_account_type == ACCT_TYPE_CSR)
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "working child account type CSR");
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_JV_OBJ,jv_obj_pr, ebufp);
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_DT_OBJ,dt_obj_pr, ebufp);
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_SDT_OBJ,sdt_obj_pr, ebufp);
						}
						else if (local_child_account_type == ACCT_TYPE_LCO)
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "working child account type LCO");
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_JV_OBJ,jv_obj_pr, ebufp);
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_DT_OBJ,dt_obj_pr, ebufp);
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_SDT_OBJ,sdt_obj_pr, ebufp);
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, PIN_FLD_PARENT,acnt_par_pdp, ebufp);
						}
						else
						{
							goto CLEANUP_1;
						}
					}
					else
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Parent is SDT child can only be CSR or OPERATION or FIELD_SERVICE or LOGISTICS or COLLECTION_AGENT or SALES_PERSON or LCO", ebufp);
						PIN_ERRBUF_RESET(ebufp);
						char		*flistbuf	= NULL;
						pin_buf_t	*nota_buf	= NULL;
						flistlen = 0;

						add_task_arry_flist= PIN_FLIST_ELEM_ADD(task_flistp, PIN_FLD_RESULTS, 0, ebufp);
						if (PIN_POID_COMPARE(account_obj, acnt_child_pdp, 0, ebufp ) ==0)
						{
							PIN_FLIST_ELEM_COPY(task_flistp, PIN_FLD_PARENTS, 0, add_task_arry_flist, PIN_FLD_PARENTS, 0, ebufp);
						}
						else
						{
							resubmit_job_arr = PIN_FLIST_ELEM_ADD(add_task_arry_flist, PIN_FLD_PARENTS, 0, ebufp );
							PIN_FLIST_FLD_SET(resubmit_job_arr,  PIN_FLD_ACCOUNT_NO, child_account_no, ebufp );
							PIN_FLIST_FLD_SET(resubmit_job_arr,  PIN_FLD_DESCR, job_ch_acnt_no , ebufp );
						}

						PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_POID, (PIN_POID_CREATE(db, "/", -1 ,ebufp)), ebufp);
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"task_flistp : ",task_flistp);

						PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );

						nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
						if ( nota_buf == NULL )
						{
							pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
							PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
							goto CLEANUP;
						}

						nota_buf->flag   = 0;
						nota_buf->size   = flistlen - 2;
						nota_buf->offset = 0;
						nota_buf->data   = ( caddr_t)flistbuf;
						nota_buf->xbuf_file = ( char *) NULL;


						job_iflistp = PIN_FLIST_CREATE(ebufp);
						job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_hierarchy", -1 ,ebufp);
						PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
						PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_CODE , "51047" , ebufp)
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Parent is SDT child can only be CSR/LCO" , ebufp);
						PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
						PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
						PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ACCOUNT_OBJ , acnt_child_pdp , ebufp);
						
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks failed profile input list", job_iflistp);
						        /* OAP sometimes send error in input */
					        if(input_status && (*input_status == status_fail))
        					{
                					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                					PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                					PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        					}
        					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist" ,job_iflistp);

        					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

						PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks failed profile output list", job_oflistp);
						if(PIN_ERRBUF_IS_ERR(ebufp))
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for hierarchy change" );
							PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
							PIN_ERRBUF_CLEAR(ebufp);
						}
						goto CLEANUP_1;
					}
				}

				if(par_account_type == ACCT_TYPE_DTR)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Parent account type DT");
					if (	child_account_type == ACCT_TYPE_CSR ||
						child_account_type == ACCT_TYPE_OPERATION ||
						child_account_type == ACCT_TYPE_FIELD_SERVICE ||
						child_account_type == ACCT_TYPE_LOGISTICS ||
						child_account_type == ACCT_TYPE_COLLECTION_AGENT ||
						child_account_type == ACCT_TYPE_SALES_PERSON
					   )
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "child account type CSR or OPERATION or FIELD_SERVICE or LOGISTICS or COLLECTION_AGENT or SALES_PERSON");
						PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_JV_OBJ,jv_obj_pr, ebufp);
						PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_LCO_OBJ,NULL, ebufp);
						PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_SDT_OBJ,NULL, ebufp);
						PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_DT_OBJ,dt_obj_pr, ebufp);							
						PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, PIN_FLD_PARENT,acnt_par_pdp, ebufp);							
					}
					else if(child_account_type == ACCT_TYPE_LCO)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "child account type LCO");
						if (local_child_account_type == ACCT_TYPE_CSR)
						{							
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "working child account type CSR");
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_JV_OBJ,jv_obj_pr, ebufp);
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_DT_OBJ,dt_obj_pr, ebufp);
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_SDT_OBJ,NULL, ebufp);

						}
						else if (local_child_account_type == ACCT_TYPE_LCO)
						{  
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "working child account type LCO");
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_JV_OBJ,jv_obj_pr, ebufp);
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_DT_OBJ,dt_obj_pr, ebufp);
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_SDT_OBJ,NULL, ebufp);
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, PIN_FLD_PARENT,acnt_par_pdp, ebufp);							
						}
						else
						{
							goto CLEANUP_1;
						}

					}
					else if (child_account_type == ACCT_TYPE_SUB_DTR)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "child account type SDT");
						if (local_child_account_type == ACCT_TYPE_CSR ||
						    local_child_account_type == ACCT_TYPE_OPERATION ||
						    local_child_account_type == ACCT_TYPE_FIELD_SERVICE ||
						    local_child_account_type == ACCT_TYPE_LOGISTICS ||
						    local_child_account_type == ACCT_TYPE_COLLECTION_AGENT ||
						    local_child_account_type == ACCT_TYPE_SALES_PERSON
						   )
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "working child account type CSR or OPERATION or FIELD_SERVICE or LOGISTICS or COLLECTION_AGENT or SALES_PERSON");
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_JV_OBJ,jv_obj_pr, ebufp);
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_DT_OBJ,dt_obj_pr, ebufp);
						}
						else if (local_child_account_type == ACCT_TYPE_LCO)
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "working child account type LCO");
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_JV_OBJ,jv_obj_pr, ebufp);
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_DT_OBJ,dt_obj_pr, ebufp);
								}
						else if (local_child_account_type == ACCT_TYPE_SUB_DTR)
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "working child account type SDT");
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_JV_OBJ,jv_obj_pr, ebufp);
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_DT_OBJ,dt_obj_pr, ebufp);
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, PIN_FLD_PARENT,acnt_par_pdp, ebufp);
						}
						else
						{
							goto CLEANUP_1;
						}
					}
					else
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Parent is DTR child can only be CSR/OPERATION/FIELD_SERVICE/LOGISTICS/COLLECTION_AGENT/SALES_PERSON/SDT/LCO", ebufp);
						PIN_ERRBUF_RESET(ebufp);

						char		*flistbuf	= NULL;
						pin_buf_t	*nota_buf	= NULL;
						flistlen = 0;

						add_task_arry_flist= PIN_FLIST_ELEM_ADD(task_flistp, PIN_FLD_RESULTS, 0, ebufp);
						if (PIN_POID_COMPARE(account_obj, acnt_child_pdp, 0, ebufp ) ==0)
						{
							PIN_FLIST_ELEM_COPY(task_flistp, PIN_FLD_PARENTS, 0, add_task_arry_flist, PIN_FLD_PARENTS, 0, ebufp);
						}
						else
						{
							resubmit_job_arr = PIN_FLIST_ELEM_ADD(add_task_arry_flist, PIN_FLD_PARENTS, 0, ebufp );
							PIN_FLIST_FLD_SET(resubmit_job_arr,  PIN_FLD_ACCOUNT_NO, child_account_no, ebufp );
							PIN_FLIST_FLD_SET(resubmit_job_arr,  PIN_FLD_DESCR, job_ch_acnt_no , ebufp );
						}
						PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_POID, (PIN_POID_CREATE(db, "/", -1 ,ebufp)), ebufp);
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"task_flistp : ",task_flistp);

						PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );

						nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
						if ( nota_buf == NULL )
						{
							pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
							PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
							goto CLEANUP;;
						}

						nota_buf->flag   = 0;
						nota_buf->size   = flistlen - 2;
						nota_buf->offset = 0;
						nota_buf->data   = ( caddr_t)flistbuf;
						nota_buf->xbuf_file = ( char *) NULL;


						job_iflistp = PIN_FLIST_CREATE(ebufp);
						job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_hierarchy", -1 ,ebufp);
						PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
						PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_CODE , "51047" , ebufp)
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Parent is DTR child can only be CSR/OPERATION/FIELD_SERVICE/LOGISTICS/COLLECTION_AGENT/SALES_PERSON/SDT/LCO" , ebufp);
						PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
						PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
						PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ACCOUNT_OBJ , acnt_child_pdp , ebufp);
						
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks failed profile input list", job_iflistp);
						        /* OAP sometimes send error in input */
					        if(input_status && (*input_status == status_fail))
        					{
                					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                					PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                					PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        					}
        					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist" ,job_iflistp);

        					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

						PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks failed profile output list", job_oflistp);
						if(PIN_ERRBUF_IS_ERR(ebufp))
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for hierarchy change" );
							PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
							PIN_ERRBUF_CLEAR(ebufp);
						}
							goto CLEANUP_1;
					}		
				}

				if(par_account_type == ACCT_TYPE_JV || par_account_type == ACCT_TYPE_HTW )
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Parent account type JV");
					if (  child_account_type == ACCT_TYPE_CSR ||
					      child_account_type == ACCT_TYPE_OPERATION ||
					      child_account_type == ACCT_TYPE_FIELD_SERVICE ||
					      child_account_type == ACCT_TYPE_LOGISTICS ||
					      child_account_type == ACCT_TYPE_COLLECTION_AGENT ||
					      child_account_type == ACCT_TYPE_SALES_PERSON
					   )
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "child account type CSR or OPERATION or FIELD_SERVICE or LOGISTICS or COLLECTION_AGENT or SALES_PERSON");
						PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_LCO_OBJ,NULL, ebufp);
						PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_SDT_OBJ,NULL, ebufp);
						PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_DT_OBJ,NULL, ebufp);
						PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_JV_OBJ,jv_obj_pr, ebufp);							
						PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, PIN_FLD_PARENT,acnt_par_pdp, ebufp);							
					}
					else if(child_account_type == ACCT_TYPE_LCO)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "child account type LCO");
						if (local_child_account_type == ACCT_TYPE_CSR)
						{							
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "working child account type CSR");
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_JV_OBJ,jv_obj_pr, ebufp);
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_DT_OBJ,NULL, ebufp);
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_SDT_OBJ,NULL, ebufp);
						}
						else if (local_child_account_type == ACCT_TYPE_LCO)
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "working child account type LCO");
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_JV_OBJ,jv_obj_pr, ebufp);
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_DT_OBJ,NULL, ebufp);
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_SDT_OBJ,NULL, ebufp);
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, PIN_FLD_PARENT,acnt_par_pdp, ebufp);							
						}
						else
						{
							goto CLEANUP_1;
						}

					}
					else if (child_account_type == ACCT_TYPE_SUB_DTR)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "child account type DST");
						if (  local_child_account_type == ACCT_TYPE_CSR ||
						      local_child_account_type == ACCT_TYPE_OPERATION ||
						      local_child_account_type == ACCT_TYPE_FIELD_SERVICE ||
						      local_child_account_type == ACCT_TYPE_LOGISTICS ||
						      local_child_account_type == ACCT_TYPE_COLLECTION_AGENT ||
						      local_child_account_type == ACCT_TYPE_SALES_PERSON
						   )
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "working child account type CSR or OPERATION or FIELD_SERVICE or LOGISTICS or COLLECTION_AGENT or SALES_PERSON");
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_JV_OBJ,jv_obj_pr, ebufp);
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_DT_OBJ,NULL, ebufp);
						}
						else if (local_child_account_type == ACCT_TYPE_LCO)
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "working child account type LCO");
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_JV_OBJ,jv_obj_pr, ebufp);
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_DT_OBJ,NULL, ebufp);
						}
						else if (local_child_account_type == ACCT_TYPE_SUB_DTR)
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "working child account type SDT");
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_JV_OBJ,jv_obj_pr, ebufp);
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_DT_OBJ,NULL, ebufp);
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, PIN_FLD_PARENT,acnt_par_pdp, ebufp);
						}
						else
						{
							goto CLEANUP_1;
						}
					}
					else if (child_account_type == ACCT_TYPE_DTR)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "child account type DT");
						if (  local_child_account_type == ACCT_TYPE_CSR ||
						      local_child_account_type == ACCT_TYPE_OPERATION ||
						      local_child_account_type == ACCT_TYPE_FIELD_SERVICE ||
						      local_child_account_type == ACCT_TYPE_LOGISTICS ||
						      local_child_account_type == ACCT_TYPE_COLLECTION_AGENT ||
						      local_child_account_type == ACCT_TYPE_SALES_PERSON
						   )
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "working child account type CSR or OPERATION or FIELD_SERVICE or LOGISTICS or COLLECTION_AGENT or SALES_PERSON");
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_JV_OBJ,jv_obj_pr, ebufp);
							//PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_DT_OBJ,dt_obj_pr, ebufp);
						}
						else if (local_child_account_type == ACCT_TYPE_LCO)
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "working child account type LCO");
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_JV_OBJ,jv_obj_pr, ebufp);
						}
						else if (local_child_account_type == ACCT_TYPE_SUB_DTR)
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "working child account type SDT");
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_JV_OBJ,jv_obj_pr, ebufp);
						}
						else if (local_child_account_type == ACCT_TYPE_DTR)
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "working child account type DT");
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, MSO_FLD_JV_OBJ,jv_obj_pr, ebufp);
							PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, PIN_FLD_PARENT,acnt_par_pdp, ebufp);
						}
						else
						{
							goto CLEANUP_1;
						}
					}
					else
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Parent is JV/OPERATOR child can only be CSR/DTR/SDT/LCO/OPERATION/FIELD_SERVICE/LOGISTICS/COLLECTION_AGENT/SALES_PERSON", ebufp);
						char		*flistbuf	= NULL;
						pin_buf_t	*nota_buf	= NULL;
						flistlen = 0;

						add_task_arry_flist= PIN_FLIST_ELEM_ADD(task_flistp, PIN_FLD_RESULTS, 0, ebufp);
						PIN_FLIST_ELEM_COPY(task_flistp, PIN_FLD_PARENTS, 0, add_task_arry_flist, PIN_FLD_PARENTS, 0, ebufp);
						PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_POID, (PIN_POID_CREATE(db, "/", -1 ,ebufp)), ebufp);
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"task_flistp : ",task_flistp);

						PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );

						nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
						if ( nota_buf == NULL )
						{
							pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
							PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
							goto CLEANUP;
						}

						nota_buf->flag   = 0;
						nota_buf->size   = flistlen - 2;
						nota_buf->offset = 0;
						nota_buf->data   = ( caddr_t)flistbuf;
						nota_buf->xbuf_file = ( char *) NULL;


						job_iflistp = PIN_FLIST_CREATE(ebufp);
						job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_hierarchy", -1 ,ebufp);
						PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
						PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_CODE , "51047" , ebufp)
						PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Parent is JV/OPERATOR child can only be CSR/DTR/SDT/LCO/OPERATION/FIELD_SERVICE/LOGISTICS/COLLECTION_AGENT/SALES_PERSON" , ebufp);
						PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
						PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
						PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ACCOUNT_OBJ , acnt_child_pdp , ebufp);
						
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks failed profile input list", job_iflistp);
						        /* OAP sometimes send error in input */
					        if(input_status && (*input_status == status_fail))
        					{
                					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                					PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                					PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        					}
        					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation input" ,job_iflistp);

        					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

						PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks failed profile output list", job_oflistp);
						if(PIN_ERRBUF_IS_ERR(ebufp))
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for hierarchy change" );
							PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
							PIN_ERRBUF_CLEAR(ebufp);
						}
						goto CLEANUP_1;
					}
				}
				PIN_FLIST_FLD_SET(cust_mod_wholesale_flistp, PIN_FLD_CUSTOMER_TYPE,&par_account_type, ebufp);

					
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks modify cust profile input list", cust_modify_wholesale_flistp);	
				
				flistlen = 0;
				
				add_task_arry_flist= PIN_FLIST_ELEM_ADD(cust_modify_wholesale_flistp, PIN_FLD_RESULTS, 0, ebufp);
				if (PIN_POID_COMPARE(account_obj, acnt_child_pdp, 0, ebufp ) ==0)
				{
					PIN_FLIST_ELEM_COPY(task_flistp, PIN_FLD_PARENTS, 0, add_task_arry_flist, PIN_FLD_PARENTS, 0, ebufp);
				}
				else
				{
					resubmit_job_arr = PIN_FLIST_ELEM_ADD(add_task_arry_flist, PIN_FLD_PARENTS, 0, ebufp );
					PIN_FLIST_FLD_SET(resubmit_job_arr,  PIN_FLD_ACCOUNT_NO, child_account_no, ebufp );
					PIN_FLIST_FLD_SET(resubmit_job_arr,  PIN_FLD_DESCR, job_ch_acnt_no , ebufp );
				}
				
					
				PIN_FLIST_TO_STR(cust_modify_wholesale_flistp, &flistbuf, &flistlen, ebufp );
				
				nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
				if ( nota_buf == NULL )
				{
					pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
					PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
					goto CLEANUP;
				}
									
				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_hierarchy", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_sucess , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp  , PIN_FLD_ACCOUNT_OBJ , account_obj , ebufp);

				
				nota_buf->flag   = 0;
				nota_buf->size   = flistlen - 2;
				nota_buf->offset = 0;
				nota_buf->data   = ( caddr_t)flistbuf;
				nota_buf->xbuf_file = ( char *) NULL;
				PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "input flist for jobs", job_iflistp);
				        /* OAP sometimes send error in input */
			        if(input_status && (*input_status == status_fail))
        			{
                			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                			PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                			PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        			}
       	 			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist" ,job_iflistp);

       				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

				PCM_OP(ctxp,PCM_OP_CREATE_OBJ ,0, job_iflistp , &job_oflistp , ebufp);		
				
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "success_job flist for jobs", job_oflistp);
				
				CLEANUP_1:
				PIN_FLIST_DESTROY_EX(&job_iflistp,NULL);
				PIN_FLIST_DESTROY_EX(&job_oflistp,NULL);
				PIN_FLIST_DESTROY_EX(&cust_modify_wholesale_flistp,NULL);
				PIN_FLIST_DESTROY_EX(&job_ch_acnt_info,NULL);
			} //End while loop 1
			PIN_FLIST_DESTROY_EX(&search_outflistp,NULL);
			//PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);

		}// End of if not End customer			
		
		/************************* Search End Customer profiles *************************/
			
		if (child_account_type != ACCT_TYPE_SUBSCRIBER && 
		    child_account_type != ACCT_TYPE_CSR &&
		    child_account_type != ACCT_TYPE_OPERATION &&
		    child_account_type != ACCT_TYPE_FIELD_SERVICE &&
		    child_account_type != ACCT_TYPE_LOGISTICS &&
		    child_account_type != ACCT_TYPE_COLLECTION_AGENT &&
		    child_account_type != ACCT_TYPE_SALES_PERSON &&
		    child_account_type != ACCT_TYPE_SME_SUBS_BB &&
		    child_account_type != ACCT_TYPE_RETAIL_BB &&
		    child_account_type != ACCT_TYPE_CORP_SUBS_BB)
		 {
				 
			db = PIN_POID_GET_DB(task_pdp);
			search_custprof_inflistp = PIN_FLIST_CREATE(ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search_custprof_inflistp ", search_custprof_inflistp);
			PIN_FLIST_FLD_PUT(search_custprof_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(search_custprof_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);
			PIN_FLIST_FLD_SET(search_custprof_inflistp, PIN_FLD_TEMPLATE, search_cc_template, ebufp);

			args_cc_acct_obj_flistp = PIN_FLIST_ELEM_ADD(search_custprof_inflistp, PIN_FLD_ARGS, 1, ebufp );
			args_cc_poid_flistp = PIN_FLIST_SUBSTR_ADD(args_cc_acct_obj_flistp,PIN_FLD_CUSTOMER_CARE_INFO, ebufp );
			
			// based on the type of account populate the below field
			if(child_account_type == ACCT_TYPE_LCO )
			{
				PIN_FLIST_FLD_SET(args_cc_poid_flistp, MSO_FLD_LCO_OBJ,acnt_child_pdp, ebufp);
			}
			if (child_account_type == ACCT_TYPE_JV || child_account_type == ACCT_TYPE_HTW )
			{
				PIN_FLIST_FLD_SET(args_cc_poid_flistp, MSO_FLD_JV_OBJ,acnt_child_pdp, ebufp);		
			}
			if (child_account_type == ACCT_TYPE_DTR )
			{
				PIN_FLIST_FLD_SET(args_cc_poid_flistp, MSO_FLD_DT_OBJ,acnt_child_pdp, ebufp);
			}	
			if (child_account_type == ACCT_TYPE_SUB_DTR )
			{
				PIN_FLIST_FLD_SET(args_cc_poid_flistp, MSO_FLD_SDT_OBJ,acnt_child_pdp, ebufp);
			}
	
			
		/**************************************************************/			
			results_cc_flistp = PIN_FLIST_ELEM_ADD(search_custprof_inflistp, PIN_FLD_RESULTS,PIN_ELEMID_ANY, ebufp );
			PIN_FLIST_FLD_SET(results_cc_flistp, PIN_FLD_POID,NULL, ebufp);
			PIN_FLIST_FLD_SET(results_cc_flistp, PIN_FLD_ACCOUNT_OBJ,NULL, ebufp);

			cust_mod_cc_flistp = PIN_FLIST_SUBSTR_ADD(results_cc_flistp, PIN_FLD_CUSTOMER_CARE_INFO, ebufp );
			PIN_FLIST_FLD_SET(cust_mod_cc_flistp, MSO_FLD_DT_OBJ,NULL, ebufp);
			PIN_FLIST_FLD_SET(cust_mod_cc_flistp, MSO_FLD_JV_OBJ,NULL, ebufp);
			PIN_FLIST_FLD_SET(cust_mod_cc_flistp, MSO_FLD_LCO_OBJ,NULL, ebufp);
			PIN_FLIST_FLD_SET(cust_mod_cc_flistp, PIN_FLD_PARENT,NULL, ebufp);
			PIN_FLIST_FLD_SET(cust_mod_cc_flistp, MSO_FLD_SDT_OBJ,NULL, ebufp);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks search profile input list", search_custprof_inflistp);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_custprof_inflistp, &search_custprof_outflistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks search profile output list", search_custprof_outflistp);
			PIN_FLIST_DESTROY_EX(&search_custprof_inflistp, NULL);
		}
		else if(child_account_type == ACCT_TYPE_SUBSCRIBER || (child_account_type == ACCT_TYPE_SME_SUBS_BB || child_account_type == ACCT_TYPE_RETAIL_BB || child_account_type == ACCT_TYPE_CORP_SUBS_BB))
		{
			if (par_account_type == ACCT_TYPE_LCO )
			{
				db = PIN_POID_GET_DB(task_pdp);
				search_custprof_inflistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_PUT(search_custprof_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
				PIN_FLIST_FLD_SET(search_custprof_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);
				PIN_FLIST_FLD_SET(search_custprof_inflistp, PIN_FLD_TEMPLATE, search_self_template, ebufp);

				args_self_acct_obj_flistp = PIN_FLIST_ELEM_ADD(search_custprof_inflistp, PIN_FLD_ARGS, 1, ebufp );
				PIN_FLIST_FLD_SET(args_self_acct_obj_flistp, PIN_FLD_ACCOUNT_OBJ,acnt_child_pdp , ebufp);
				
				results_self_flistp = PIN_FLIST_ELEM_ADD(search_custprof_inflistp, PIN_FLD_RESULTS,PIN_ELEMID_ANY, ebufp );
				
				PIN_FLIST_FLD_SET(results_self_flistp, PIN_FLD_POID,NULL, ebufp);
				PIN_FLIST_FLD_SET(results_self_flistp, PIN_FLD_ACCOUNT_OBJ,NULL, ebufp);
				cust_mod_cc_flistp = PIN_FLIST_SUBSTR_ADD(results_self_flistp, PIN_FLD_CUSTOMER_CARE_INFO, ebufp );
				PIN_FLIST_FLD_SET(cust_mod_cc_flistp, MSO_FLD_DT_OBJ,NULL, ebufp);
				PIN_FLIST_FLD_SET(cust_mod_cc_flistp, MSO_FLD_JV_OBJ,NULL, ebufp);
				PIN_FLIST_FLD_SET(cust_mod_cc_flistp, MSO_FLD_LCO_OBJ,NULL, ebufp);
				PIN_FLIST_FLD_SET(cust_mod_cc_flistp, PIN_FLD_PARENT,NULL, ebufp);
				PIN_FLIST_FLD_SET(cust_mod_cc_flistp, MSO_FLD_SDT_OBJ,NULL, ebufp);
				
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks search profile input list", search_custprof_inflistp);
				PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_custprof_inflistp, &search_custprof_outflistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks search profile output list", search_custprof_outflistp);
				PIN_FLIST_DESTROY_EX(&search_custprof_inflistp, NULL);
			}
			else
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Non LCO parent passed for CUSTOMER", ebufp);

				char		*flistbuf	= NULL;
				pin_buf_t	*nota_buf	= NULL;
				flistlen = 0;

				add_task_arry_flist= PIN_FLIST_ELEM_ADD(task_flistp, PIN_FLD_RESULTS, 0, ebufp);
				PIN_FLIST_ELEM_COPY(task_flistp, PIN_FLD_PARENTS, 0, add_task_arry_flist, PIN_FLD_PARENTS, 0, ebufp);
				PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_POID, (PIN_POID_CREATE(db, "/", -1 ,ebufp)), ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"task_flistp : ",task_flistp);

				PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );

				nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
				if ( nota_buf == NULL )
				{
					pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
					PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
					return;
				}

				nota_buf->flag   = 0;
				nota_buf->size   = flistlen - 2;
				nota_buf->offset = 0;
				nota_buf->data   = ( caddr_t)flistbuf;
				nota_buf->xbuf_file = ( char *) NULL;


				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_hierarchy", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_CODE , "51047" , ebufp)
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Non LCO parent passed for CUSTOMER" , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ACCOUNT_OBJ , account_obj , ebufp);

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks failed profile input list", job_iflistp);
				        /* OAP sometimes send error in input */
			        if(input_status && (*input_status == status_fail))
        			{
                			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                			PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                			PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        			}
        			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist" ,job_iflistp);

        			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

				PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks failed profile output list", job_oflistp);
				if(PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for hierarchy change" );
					PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
					PIN_ERRBUF_CLEAR(ebufp);
				}
				goto CLEANUP;
			}
		}
		//count_cust = PIN_FLIST_ELEM_COUNT(search_custprof_outflistp, PIN_FLD_RESULTS, ebufp); 
		
			

		//Action for each search result
		if( child_account_type != ACCT_TYPE_CSR && 
		    child_account_type != ACCT_TYPE_OPERATION &&
		    child_account_type != ACCT_TYPE_FIELD_SERVICE &&
		    child_account_type != ACCT_TYPE_LOGISTICS &&
		    child_account_type != ACCT_TYPE_COLLECTION_AGENT &&
		    child_account_type != ACCT_TYPE_SALES_PERSON
		 )
		{
			while ((results_cc_out_flistp = PIN_FLIST_ELEM_GET_NEXT(search_custprof_outflistp,
				PIN_FLD_RESULTS,&elem_resluts_cc_id, 1, &cookie_cc_results, ebufp))!= NULL)
			{
				    
				char		*flistbuf	= NULL;
				pin_buf_t	*nota_buf	= NULL;
				flistlen = 0;
				
				if (results_cc_out_flistp)
				{
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job customer care search profile input list", results_cc_out_flistp);
					results_cc_prof = PIN_FLIST_SUBSTR_GET(results_cc_out_flistp, PIN_FLD_CUSTOMER_CARE_INFO, 0, ebufp );

					account_obj = PIN_FLIST_FLD_GET(results_cc_out_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp );
					
					if (account_obj && child_account_type != ACCT_TYPE_SUBSCRIBER)
					{
						fm_mso_get_account_info(ctxp, account_obj, &job_ch_acnt_info, ebufp);
						if (job_ch_acnt_info)
						{
							job_ch_acnt_no  = PIN_FLIST_FLD_GET(job_ch_acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
						}
					}
					else if (child_account_type == ACCT_TYPE_SUBSCRIBER )
					{
						job_ch_acnt_no = PIN_FLIST_FLD_GET(parent_flistp ,PIN_FLD_DESCR, 1, ebufp);
					}

					profile_obj = PIN_FLIST_FLD_GET(results_cc_out_flistp, PIN_FLD_POID, 1, ebufp );
				   
					db = PIN_POID_GET_DB(task_pdp);
					cust_modify_cc_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job customer care profile input list", cust_modify_cc_flistp);
					PIN_FLIST_FLD_PUT(cust_modify_cc_flistp, PIN_FLD_USERID, PIN_POID_CREATE(db, "/account", 1, ebufp), ebufp);
					PIN_FLIST_FLD_SET(cust_modify_cc_flistp, PIN_FLD_POID,account_obj, ebufp);
					PIN_FLIST_FLD_SET(cust_modify_cc_flistp, PIN_FLD_ACCOUNT_NO,job_ch_acnt_no, ebufp);
					PIN_FLIST_FLD_SET(cust_modify_cc_flistp, PIN_FLD_ACCOUNT_OBJ,account_obj, ebufp);
					PIN_FLIST_FLD_SET(cust_modify_cc_flistp, PIN_FLD_PROFILE_OBJ,profile_obj, ebufp);
					PIN_FLIST_FLD_SET(cust_modify_cc_flistp, PIN_FLD_PROGRAM_NAME,"OAP|BulkHierarchy", ebufp);
					
					//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job customer care profile input list1", cust_mod_cc_flistp);
					cust_mod_cc_flistp = PIN_FLIST_SUBSTR_ADD(cust_modify_cc_flistp, PIN_FLD_CUSTOMER_CARE_INFO, ebufp );
					PIN_FLIST_FLD_COPY(results_cc_prof, MSO_FLD_DT_OBJ, cust_mod_cc_flistp, MSO_FLD_DT_OBJ, ebufp);
					PIN_FLIST_FLD_COPY(results_cc_prof, MSO_FLD_JV_OBJ, cust_mod_cc_flistp, MSO_FLD_JV_OBJ, ebufp);
					PIN_FLIST_FLD_COPY(results_cc_prof, MSO_FLD_LCO_OBJ, cust_mod_cc_flistp, MSO_FLD_LCO_OBJ, ebufp);
					PIN_FLIST_FLD_COPY(results_cc_prof, MSO_FLD_SDT_OBJ, cust_mod_cc_flistp, MSO_FLD_SDT_OBJ, ebufp);
					PIN_FLIST_FLD_COPY(results_cc_prof, PIN_FLD_PARENT, cust_mod_cc_flistp, PIN_FLD_PARENT, ebufp);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "cust_modify_cc_flistp", cust_modify_cc_flistp);
					
					if(par_account_type == ACCT_TYPE_LCO)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Parent account type LCO and child is Customer");
						PIN_FLIST_FLD_SET(cust_mod_cc_flistp, MSO_FLD_JV_OBJ,jv_obj_pr, ebufp);
						PIN_FLIST_FLD_SET(cust_mod_cc_flistp, MSO_FLD_DT_OBJ,dt_obj_pr, ebufp);
						PIN_FLIST_FLD_SET(cust_mod_cc_flistp, MSO_FLD_SDT_OBJ,sdt_obj_pr, ebufp);
						PIN_FLIST_FLD_SET(cust_mod_cc_flistp, MSO_FLD_LCO_OBJ,lco_obj_pr, ebufp);
						PIN_FLIST_FLD_SET(cust_mod_cc_flistp, PIN_FLD_PARENT,acnt_par_pdp, ebufp);
						PIN_FLIST_FLD_SET(cust_mod_cc_flistp, MSO_FLD_PP_TYPE,pp_type, ebufp);
						PIN_FLIST_FLD_SET(cust_mod_cc_flistp, MSO_FLD_DAS_TYPE, das_type, ebufp)
					}
					else if (par_account_type == ACCT_TYPE_SUB_DTR )
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Parent account type Subdistributor");
						if(child_account_type == ACCT_TYPE_LCO )
						{
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Child account type LCO", cust_modify_cc_flistp);
							PIN_FLIST_FLD_SET(cust_mod_cc_flistp, MSO_FLD_JV_OBJ,jv_obj_pr, ebufp);
							PIN_FLIST_FLD_SET(cust_mod_cc_flistp, MSO_FLD_DT_OBJ,dt_obj_pr, ebufp);
							PIN_FLIST_FLD_SET(cust_mod_cc_flistp, MSO_FLD_SDT_OBJ,sdt_obj_pr, ebufp);
							//PIN_FLIST_FLD_SET(cust_mod_cc_flistp, MSO_FLD_LCO_OBJ,lco_obj_pr, ebufp);
						}
						else
						{
							goto CLEANUP_2;
						}
					}
					else if (par_account_type == ACCT_TYPE_DTR )
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Parent account type Distributor");
						if(child_account_type == ACCT_TYPE_LCO )
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Child account type LCO");
							PIN_FLIST_FLD_SET(cust_mod_cc_flistp, MSO_FLD_JV_OBJ,jv_obj_pr, ebufp);
							PIN_FLIST_FLD_SET(cust_mod_cc_flistp, MSO_FLD_DT_OBJ,dt_obj_pr, ebufp);
							PIN_FLIST_FLD_SET(cust_mod_cc_flistp, MSO_FLD_SDT_OBJ,NULL, ebufp);
						}
						else if (child_account_type == ACCT_TYPE_SUB_DTR)
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Child account type Subdistributor");
							PIN_FLIST_FLD_SET(cust_mod_cc_flistp, MSO_FLD_JV_OBJ,jv_obj_pr, ebufp);
							PIN_FLIST_FLD_SET(cust_mod_cc_flistp, MSO_FLD_DT_OBJ,dt_obj_pr, ebufp);
						}
						else
						{
							goto CLEANUP_2;
						}
					}
					else if (par_account_type == ACCT_TYPE_JV || par_account_type == ACCT_TYPE_HTW )
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Parent account type JV/HTW ");
						if(child_account_type == ACCT_TYPE_LCO )
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Child account type LCO");
							PIN_FLIST_FLD_SET(cust_mod_cc_flistp, MSO_FLD_JV_OBJ,jv_obj_pr, ebufp);
							PIN_FLIST_FLD_SET(cust_mod_cc_flistp, MSO_FLD_DT_OBJ,NULL, ebufp);
							PIN_FLIST_FLD_SET(cust_mod_cc_flistp, MSO_FLD_SDT_OBJ,NULL, ebufp);
						}
						else if (child_account_type == ACCT_TYPE_SUB_DTR)
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Child account type Subdistributor");
							PIN_FLIST_FLD_SET(cust_mod_cc_flistp, MSO_FLD_JV_OBJ,jv_obj_pr, ebufp);
							PIN_FLIST_FLD_SET(cust_mod_cc_flistp, MSO_FLD_DT_OBJ,NULL, ebufp);
						}
						else if (child_account_type == ACCT_TYPE_DTR)
						{	
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Child account type Distributor");
							PIN_FLIST_FLD_SET(cust_mod_cc_flistp, MSO_FLD_JV_OBJ,jv_obj_pr, ebufp);
						}
						else
						{
							goto CLEANUP_2;
						}
					}
					else
					{
						goto CLEANUP_2;
					}
						
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , "Final customer modify flist bef buf is " , cust_modify_cc_flistp);
					
					cust_mod_cc_flistp = PIN_FLIST_SUBSTR_GET(cust_modify_cc_flistp, PIN_FLD_CUSTOMER_CARE_INFO, 1, ebufp );
					if (cust_mod_cc_flistp)
					{
						lco_account_obj = PIN_FLIST_FLD_GET(cust_mod_cc_flistp, MSO_FLD_LCO_OBJ, 1, ebufp);
						fm_mso_get_account_info(ctxp, lco_account_obj, &job_par_acnt_info, ebufp);
						if (job_par_acnt_info)
						{
							job_par_acnt_no = PIN_FLIST_FLD_GET(job_par_acnt_info,  PIN_FLD_ACCOUNT_NO, 1, ebufp);
						}
					}


					flistlen = 0;
						
					add_task_arry_flist= PIN_FLIST_ELEM_ADD(cust_modify_cc_flistp, PIN_FLD_RESULTS, 0, ebufp);
					//PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "account_obj", account_obj);
					//PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "acnt_child_pdp", acnt_child_pdp);
					
					if (PIN_POID_COMPARE(account_obj, acnt_child_pdp, 0, ebufp ) ==0)   //child account is LCO
					{
						//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "111");
						PIN_FLIST_ELEM_COPY(task_flistp, PIN_FLD_PARENTS, 0, add_task_arry_flist, PIN_FLD_PARENTS, 0, ebufp);
						PIN_FLIST_FLD_SET(add_task_arry_flist, PIN_FLD_FLAGS, &flag_lco_of_cust, ebufp);
					}
					else
					{
						//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "222");
						resubmit_job_arr = PIN_FLIST_ELEM_ADD(add_task_arry_flist, PIN_FLD_PARENTS, 0, ebufp );
						PIN_FLIST_FLD_SET(resubmit_job_arr,  PIN_FLD_ACCOUNT_NO, job_par_acnt_no, ebufp );
						PIN_FLIST_FLD_SET(resubmit_job_arr,  PIN_FLD_DESCR, job_ch_acnt_no , ebufp );
					}
					//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , "add_task_arry_flist" , add_task_arry_flist);
					PIN_FLIST_TO_STR(cust_modify_cc_flistp, &flistbuf, &flistlen, ebufp );
					
					nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
					if ( nota_buf == NULL )
					{
						pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
						PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
						goto CLEANUP;
					}

					job_iflistp = PIN_FLIST_CREATE(ebufp);
					job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_hierarchy", -1 ,ebufp);
					PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_sucess , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
					PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
					PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp  , PIN_FLD_ACCOUNT_OBJ , account_obj , ebufp);
					

					nota_buf->flag   = 0;
					nota_buf->size   = flistlen - 2;
					nota_buf->offset = 0;
					nota_buf->data   = ( caddr_t)flistbuf;
					nota_buf->xbuf_file = ( char *) NULL;
					PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
					        /* OAP sometimes send error in input */
				        if(input_status && (*input_status == status_fail))
        				{
                				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                				PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                				PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        				}
        				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist" ,job_iflistp);

        				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);


					PCM_OP(ctxp,PCM_OP_CREATE_OBJ ,0, job_iflistp , &job_oflistp , ebufp);		
					
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "success_job flist for jobs", job_oflistp);
					
					PIN_FLIST_DESTROY_EX(&job_iflistp,NULL);					
					
					if (PIN_ERRBUF_IS_ERR(ebufp))
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the mso_mta job object " );
						goto CLEANUP_2;
					}
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " job create output flist is " , job_oflistp);
					PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
					
					if(nota_buf)
					{
						free(nota_buf);
					}
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tasks modify cust profile input list", cust_modify_cc_flistp);	
				}
				CLEANUP_2:
				PIN_FLIST_DESTROY_EX(&job_ch_acnt_info,NULL);
			} //End while loop 2
		}
		PIN_FLIST_DESTROY_EX(&search_custprof_outflistp,NULL);
		PIN_FLIST_DESTROY_EX(&cust_modify_cc_flistp,NULL);
		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	}

	CLEANUP:
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
	}
	PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
	PIN_FLIST_DESTROY_EX(&cust_modify_cc_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&par_acnt_info, NULL);
	PIN_FLIST_DESTROY_EX(&child_acnt_info, NULL); 
	return;
}


EXPORT_OP void
fm_mso_integ_create_bmail_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*order_iflistp = NULL;
	pin_flist_t	*order_oflistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	pin_flist_t	*job_oflistp = NULL;
	pin_flist_t	*job_iflistp = NULL;
	pin_flist_t	*tmp_flistp = NULL;
	pin_flist_t	*tmp1_flistp = NULL;
	pin_flist_t	*tmp2_flistp = NULL;
	pin_flist_t	*read_oflistp = NULL;
	pin_flist_t	*read_iflistp = NULL;

	poid_t		*job_pdp	= NULL;
	poid_t		*order_pdp	= NULL;

	poid_t		*lco_account_pdp = NULL;
	poid_t		*device_pdp	 = NULL;
	poid_t		*service_pdp	 = NULL;
	poid_t		*account_pdp	= NULL;

	char		*order_id	= NULL;
	char		*account_no	= NULL;
	char		*device_id	= NULL;
	
	int64		db = 1;
	
	int32		status_sucess = 0;
	int32		status_fail = 2;
	
	pin_buf_t	*nota_buf = NULL;
	char		*flistbuf = NULL;
	int		flistlen =0;

	int32		opcode = MSO_OP_PROV_CREATE_ACTION;
	char		*task_pdp_type	= NULL;
	char		*title	= NULL;
	char		*message = NULL;
	int32           *input_status = NULL;
	
	pin_cookie_t	cookie = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_create_bmail_job input flist", task_flistp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " task_flistp is " ,task_flistp);
	input_status = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_STATUS, 1 , ebufp);

	device_id =  PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_ID , 1 , ebufp);
	title = (char *)PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_TITLE, 1, ebufp);
	message = (char *)PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_MESSAGE, 1, ebufp);

	task_pdp_type = (char *)PIN_POID_GET_TYPE(task_pdp);

	if(device_id)
	{
		fm_mso_search_device_object(ctxp, task_flistp , ebufp);
		service_pdp = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, " service poid is " ,service_pdp); 
		if(PIN_POID_IS_NULL(service_pdp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Service poid is null");
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " error set");
			}
			PIN_ERRBUF_CLEAR(ebufp);
			char		*flistbuf	= NULL;
			pin_buf_t	*nota_buf	= NULL;
			flistlen = 0;
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_POID, NULL , ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_USERID , PIN_POID_CREATE(db,"/account",1, ebufp), ebufp);

			tmp_flistp = PIN_FLIST_ELEM_ADD (task_flistp, MSO_FLD_MSG_TITLE_LIST, 0, ebufp);
			PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_TITLE ,title, ebufp);

			tmp1_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_MSG_CONTENT_LIST, 0, ebufp);
			PIN_FLIST_FLD_SET(tmp1_flistp, PIN_FLD_MESSAGE ,message, ebufp);

			tmp2_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_STB_ID_LIST, 0, ebufp);
			PIN_FLIST_FLD_SET(tmp2_flistp, MSO_FLD_STB_ID ,device_id, ebufp);

			PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERRBUF_CLEAR(ebufp);
			}

			PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
			nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
			if ( nota_buf == NULL )
			{
				pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
				PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
				return;
			}
			nota_buf->flag   = 0;
			nota_buf->size   = flistlen - 2;
			nota_buf->offset = 0;
			nota_buf->data   = ( caddr_t)flistbuf;
			nota_buf->xbuf_file = ( char *) NULL;
			
			/********
			 * Update the status of the mso_mta_job to 2 to indiacte the failure record
			 ******/
			 if(!strcmp(task_pdp_type,"/mso_task/bulk_cisco_bmail"))
			{
				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_cisco_bmail", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
				PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51040" , ebufp)
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid MAC ID Entered No Service Exists for it" , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				//PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ACCOUNT_OBJ ,account_pdp, ebufp);
			}
			else if(!strcmp(task_pdp_type,"/mso_task/bulk_cisco1_bmail"))
			{
				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_cisco1_bmail", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
				PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51040" , ebufp)
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid MAC ID Entered No Service Exists for it" , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				//PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ACCOUNT_OBJ ,account_pdp, ebufp);
			}
			/******* VERIMATRIX CHANGES - BEGIN - BULK VM BMAIL ****************/	
			else if(!strcmp(task_pdp_type,"/mso_task/bulk_vm_bmail"))
			{
				job_iflistp = PIN_FLIST_CREATE(ebufp);
				job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_vm_bmail", -1 ,ebufp);
				PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
				PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51040" , ebufp)
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid MAC ID Entered No Service Exists for it" , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
				PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
				//PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ACCOUNT_OBJ ,account_pdp, ebufp);
			}			
			/******* VERIMATRIX CHANGES - END - BULK VM BMAIL ****************/	
			
			PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for bulk bmail " );
				PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
				PIN_ERRBUF_CLEAR(ebufp);
			}
			PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
			
			if(job_oflistp)
				job_pdp = PIN_FLIST_FLD_GET(job_oflistp , PIN_FLD_POID, 1, ebufp);

			pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"Invalid VC Number", ebufp);
			if(nota_buf)
				free(nota_buf);
		}
		else
		{
			/*** read the service poid to get the account number ****/
			read_iflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(read_iflistp, PIN_FLD_POID,service_pdp, ebufp);
			PIN_FLIST_FLD_SET(read_iflistp, PIN_FLD_ACCOUNT_OBJ , NULL , ebufp);
			PCM_OP(ctxp, PCM_OP_READ_FLDS , 0, read_iflistp , &read_oflistp , ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " read service output flist is ", read_oflistp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " read flds error");
				PIN_FLIST_DESTROY_EX(&read_oflistp , NULL);
				PIN_ERRBUF_CLEAR(ebufp);
			}
			if(read_oflistp)
			{
				PIN_FLIST_FLD_COPY(read_oflistp ,PIN_FLD_ACCOUNT_OBJ, task_flistp, PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_SERVICE_OBJ , service_pdp , ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " task flist after putting poid is " , task_flistp);
			}
		}
	}
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " No Device found ");
		PIN_ERRBUF_CLEAR(ebufp);
		
	}
	else
	{
		PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_USERID , PIN_POID_CREATE(db,"/account",1, ebufp), ebufp);
		PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
		}
		
		tmp_flistp = PIN_FLIST_ELEM_ADD (task_flistp, MSO_FLD_MSG_TITLE_LIST, 0, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_TITLE ,title, ebufp);

		tmp1_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_MSG_CONTENT_LIST, 0, ebufp);
		PIN_FLIST_FLD_SET(tmp1_flistp, PIN_FLD_MESSAGE ,message, ebufp);

		tmp2_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_STB_ID_LIST, 0, ebufp);
		PIN_FLIST_FLD_SET(tmp2_flistp, MSO_FLD_STB_ID ,device_id, ebufp);

		
		PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
		nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
		if ( nota_buf == NULL )
		{
			pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
			PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
			return;
		}
		/*
		* Create flist for job order creation
		*/
		if(!strcmp(task_pdp_type,"/mso_task/bulk_cisco_bmail"))
		{
			job_iflistp = PIN_FLIST_CREATE(ebufp);
			job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_cisco_bmail", -1 ,ebufp);
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_sucess , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_POID , job_iflistp , PIN_FLD_ACCOUNT_OBJ,ebufp);
			//PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ACCOUNT_OBJ ,account_pdp, ebufp);
		}
		else if(!strcmp(task_pdp_type,"/mso_task/bulk_cisco1_bmail"))
		{
			job_iflistp = PIN_FLIST_CREATE(ebufp);
			job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_cisco1_bmail", -1 ,ebufp);
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_sucess , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_POID , job_iflistp , PIN_FLD_ACCOUNT_OBJ,ebufp);
			//PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ACCOUNT_OBJ ,account_pdp, ebufp);
		}
		/******* VERIMATRIX CHANGES - BEGIN - BULK VM BMAIL ****************/	
		else if(!strcmp(task_pdp_type,"/mso_task/bulk_vm_bmail"))
		{
			job_iflistp = PIN_FLIST_CREATE(ebufp);
			job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_vm_bmail", -1 ,ebufp);
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_sucess , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_POID , job_iflistp , PIN_FLD_ACCOUNT_OBJ,ebufp);
			//PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ACCOUNT_OBJ ,account_pdp, ebufp);
		}		
		/******* VERIMATRIX CHANGES - END - BULK VM BMAIL ****************/	
		
		nota_buf->flag   = 0;
		nota_buf->size   = flistlen - 2;
		nota_buf->offset = 0;
		nota_buf->data   = ( caddr_t)flistbuf;
		nota_buf->xbuf_file = ( char *) NULL;
		PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );

		//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Job create input flist for bulk bmail actions is " ,job_iflistp);

		/* OAP sometimes send error in input */
		if(input_status && (*input_status == status_fail))
        	{
                	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                	PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                	PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                	PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        	}

		PCM_OP(ctxp,PCM_OP_CREATE_OBJ ,0, job_iflistp , &job_oflistp , ebufp);

		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the mso_mta job object for bulk bmail actions " );
			PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
			/*r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_create_job job object create failed for location_updater", ebufp);
			*ret_flistp = r_flistp;
			return;*/
		}
		else
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " job create output flist is " , job_oflistp);
			PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
			PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
			PIN_FLIST_DESTROY_EX(&read_oflistp , NULL);
			PIN_FLIST_DESTROY_EX(&read_iflistp , NULL);
		}		
	}
	if(nota_buf)
	free(nota_buf);

	return;
}


EXPORT_OP void
fm_mso_integ_set_personnel_bit_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp)

{
	pin_flist_t	*order_iflistp = NULL;
	pin_flist_t	*order_oflistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	pin_flist_t	*job_oflistp = NULL;
	pin_flist_t	*job_iflistp = NULL;
	pin_flist_t	*plan_flist = NULL;
	pin_flist_t	*read_oflistp = NULL;
	pin_flist_t	*read_iflistp = NULL;
	pin_flist_t	*inherited_info = NULL;
	pin_flist_t	*bit_flist = NULL;

	poid_t		*job_pdp	= NULL;
	poid_t		*order_pdp	= NULL;
	poid_t		*account_pdp	= NULL;

	poid_t		*lco_account_pdp = NULL;
	poid_t		*device_pdp	 = NULL;
	poid_t		*service_pdp	 = NULL;

	char		*order_id	= NULL;
	char		*account_no	= NULL;
	char		*device_id	= NULL;
	int64		db = 0;
	//int64		db = 1;
	
	int32		status_sucess = 0;
	int32		status_fail = 2;
	
	pin_buf_t	*nota_buf = NULL;
	char		*flistbuf = NULL;
	int		flistlen =0;

	int32		opcode = MSO_OP_CUST_SET_CUSTOMER_BIT ;
	char		*task_pdp_type	= NULL;
	char		*param_name = NULL;
	char		*param_value = NULL;
	int32		*flag_value = NULL;
	
	pin_cookie_t	cookie = NULL;
	int32           *input_status = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_set_personnel_bit_job input flist", task_flistp);
	input_status = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_STATUS, 1 , ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " task_flistp is " ,task_flistp);
	db = PIN_POID_GET_DB(task_pdp);
	task_pdp_type = (char *)PIN_POID_GET_TYPE(task_pdp);

	device_id =  PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_ID , 1 , ebufp);
	param_name = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_PARAM_NAME , 1 , ebufp);
	param_value = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_PARAM_VALUE , 1 , ebufp);
	flag_value =  PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_FLAGS , 1 , ebufp);

	if(device_id)
	{
		fm_mso_search_device_object(ctxp, task_flistp , ebufp);
		service_pdp = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, " service poid is " ,service_pdp); 
		if(PIN_POID_IS_NULL(service_pdp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Service poid is null");
			PIN_ERRBUF_CLEAR(ebufp);
			char		*flistbuf	= NULL;
			pin_buf_t	*nota_buf	= NULL;
			flistlen = 0;
			
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_POID, NULL , ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_USERID , PIN_POID_CREATE(db,"/account",1, ebufp), ebufp);

			PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERRBUF_CLEAR(ebufp);
			}
			PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
			nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
			if ( nota_buf == NULL )
			{
				pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
				PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
				return;
			}
			nota_buf->flag   = 0;
			nota_buf->size   = flistlen - 2;
			nota_buf->offset = 0;
			nota_buf->data   = ( caddr_t)flistbuf;
			nota_buf->xbuf_file = ( char *) NULL;
			
			/********
			 * Update the status of the mso_mta_job to 2 to indiacte the failure record
			 ******/
			job_iflistp = PIN_FLIST_CREATE(ebufp);
			job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_set_personnel_bit", -1 ,ebufp);
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51040" , ebufp)
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid VC number no service is associated to VC" , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
			//PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_POID , job_iflistp , PIN_FLD_ACCOUNT_OBJ,ebufp);
			
		        /* OAP sometimes send error in input */
       			 if(input_status && (*input_status == status_fail))
        		 {
                	 	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        		}
        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist" ,job_iflistp);

        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);
	
			PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for bulk retrack " );
				PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
				PIN_ERRBUF_CLEAR(ebufp);
			}
			PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
			
			if(job_oflistp)
				job_pdp = PIN_FLIST_FLD_GET(job_oflistp , PIN_FLD_POID, 1, ebufp);

			pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"Invalid VC Number", ebufp);
			if(nota_buf)
				free(nota_buf);
		}
		else
		{
			/*** read the service poid to get the account number ****/
			read_iflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(read_iflistp, PIN_FLD_POID,service_pdp, ebufp);
			PIN_FLIST_FLD_SET(read_iflistp, PIN_FLD_ACCOUNT_OBJ , NULL , ebufp);
			PCM_OP(ctxp, PCM_OP_READ_FLDS , 0, read_iflistp , &read_oflistp , ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " read service output flist is ", read_oflistp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " read flds error");
				PIN_FLIST_DESTROY_EX(&read_oflistp , NULL);
				PIN_ERRBUF_CLEAR(ebufp);
			}
			if(read_oflistp)
			{
				PIN_FLIST_FLD_COPY(read_oflistp ,PIN_FLD_ACCOUNT_OBJ, task_flistp, PIN_FLD_POID, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " task flist after putting poid is " , task_flistp);
			}
		}
	}
	if(*flag_value == 2)
	{
		if(PIN_POID_IS_NULL(job_pdp))
		{
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_POID, NULL , ebufp);
			//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_USERID , PIN_POID_CREATE(db,"/account",1, ebufp), ebufp);

			PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERRBUF_CLEAR(ebufp);
			}
			PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
			nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
			if ( nota_buf == NULL )
			{
				pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
				PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
				return;
			}
			nota_buf->flag   = 0;
			nota_buf->size   = flistlen - 2;
			nota_buf->offset = 0;
			nota_buf->data   = ( caddr_t)flistbuf;
			nota_buf->xbuf_file = ( char *) NULL;
			
			/********
			 * Update the status of the mso_mta_job to 2 to indiacte the failure record
			 ******/
			job_iflistp = PIN_FLIST_CREATE(ebufp);
			job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_set_personnel_bit", -1 ,ebufp);
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51045" , ebufp)
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid Action value Passed" , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
			//PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_POID , job_iflistp , PIN_FLD_ACCOUNT_OBJ,ebufp);
			
			        /* OAP sometimes send error in input */
		        if(input_status && (*input_status == status_fail))
        		{		
                		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        		}
        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist" ,job_iflistp);

        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);
	
			PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for bulk set personnel bit " );
				PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
				PIN_ERRBUF_CLEAR(ebufp);
			}
			PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
			
			if(job_oflistp)
				job_pdp = PIN_FLIST_FLD_GET(job_oflistp , PIN_FLD_POID, 1, ebufp);

			pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"Invalid Action value", ebufp);
			if(nota_buf)
				free(nota_buf);
		}
	}
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " No Device found ");
		PIN_ERRBUF_CLEAR(ebufp);
		
	}
	else
	{
		//PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_USERID , PIN_POID_CREATE(db,"/account",1, ebufp), ebufp);
		PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
		inherited_info = PIN_FLIST_SUBSTR_ADD(task_flistp,PIN_FLD_INHERITED_INFO, ebufp);
		bit_flist = PIN_FLIST_ELEM_ADD(inherited_info,MSO_FLD_PERSONAL_BIT_INFO, 0, ebufp);
		PIN_FLIST_FLD_SET(bit_flist , PIN_FLD_PARAM_NAME , param_name , ebufp);
		PIN_FLIST_FLD_SET(bit_flist, PIN_FLD_PARAM_VALUE, param_value , ebufp);
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
		}
		PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
		nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
		if ( nota_buf == NULL )
		{
			pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
			PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
			return;
		}
		/*
		* Create flist for job order creation
		*/
		job_iflistp = PIN_FLIST_CREATE(ebufp);
		job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_set_personnel_bit", -1 ,ebufp);
		PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_sucess , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
		PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID , job_iflistp ,PIN_FLD_USERID , ebufp);
		PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME , job_iflistp ,PIN_FLD_PROGRAM_NAME , ebufp);
		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_POID , job_iflistp , PIN_FLD_ACCOUNT_OBJ,ebufp);
		//PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ACCOUNT_OBJ ,account_pdp, ebufp);
			
		nota_buf->flag   = 0;
		nota_buf->size   = flistlen - 2;
		nota_buf->offset = 0;
		nota_buf->data   = ( caddr_t)flistbuf;
		nota_buf->xbuf_file = ( char *) NULL;
		PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );

		//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Job create input flist for bulk set personnel bit actions is " ,job_iflistp);
		
		        /* OAP sometimes send error in input */
	        if(input_status && (*input_status == status_fail))
        	{
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        	}
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist" ,job_iflistp);

        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

		PCM_OP(ctxp,PCM_OP_CREATE_OBJ ,0, job_iflistp , &job_oflistp , ebufp);

		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the mso_mta job object for bulk set personnel bit actions " );
			PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
			/*r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_create_job job object create failed for location_updater", ebufp);
			*ret_flistp = r_flistp;
			return;*/
		}
		else
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " job create output flist is " , job_oflistp);
			PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
			PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
			PIN_FLIST_DESTROY_EX(&read_oflistp , NULL);
			PIN_FLIST_DESTROY_EX(&read_iflistp , NULL);
		}
		
	}
	if(nota_buf)
	free(nota_buf);
	return;
}

/* EXPORT_OP void
fm_mso_integ_create_bouquet_id_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp)

{
	pin_flist_t	*order_iflistp = NULL;
	pin_flist_t	*order_oflistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	pin_flist_t	*job_oflistp = NULL;
	pin_flist_t	*job_iflistp = NULL;
	pin_flist_t	*plan_flist = NULL;
	pin_flist_t	*read_oflistp = NULL;
	pin_flist_t	*read_iflistp = NULL;
	pin_flist_t	*catv_flist = NULL;
	pin_flist_t	*services_flist = NULL;

	poid_t		*job_pdp	= NULL;
	poid_t		*order_pdp	= NULL;

	poid_t		*lco_account_pdp = NULL;
	poid_t		*device_pdp	 = NULL;
	poid_t		*service_pdp	 = NULL;

	char		*order_id	= NULL;
	char		*account_no	= NULL;
	char		*device_id	= NULL;
	
	int64		db = 1;
	
	int32		status_sucess = 0;
	int32		status_fail = 2;
	
	pin_buf_t	*nota_buf = NULL;
	char		*flistbuf = NULL;
	int		flistlen =0;

	int32		opcode = 11004 ;
	int32		*bouquet_id = NULL;
	char		*task_pdp_type	= NULL;
	int32           *input_status = NULL;	
	pin_cookie_t	cookie = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_create_bouquet_id_job input flist", task_flistp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " task_flistp is " ,task_flistp);
	input_status = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_STATUS, 1 , ebufp);
	task_pdp_type = (char *)PIN_POID_GET_TYPE(task_pdp);

	device_id =  PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_ID , 1 , ebufp);
	bouquet_id = PIN_FLIST_FLD_GET(task_flistp , MSO_FLD_BOUQUET_ID , 1 , ebufp);

	if(device_id)
	{
		fm_mso_search_device_object(ctxp, task_flistp , ebufp);
		service_pdp = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);

		PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, " service poid is " ,service_pdp); 
		if(PIN_POID_IS_NULL(service_pdp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Service poid is null");
			PIN_ERRBUF_CLEAR(ebufp);
			char		*flistbuf	= NULL;
			pin_buf_t	*nota_buf	= NULL;
			flistlen = 0;
			
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_POID, NULL , ebufp);
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_USERID , PIN_POID_CREATE(db,"/account",1, ebufp), ebufp);

			PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
			PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERRBUF_CLEAR(ebufp);
			}
			PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
			nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
			if ( nota_buf == NULL )
			{
				pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
				PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
				return;
			}
			nota_buf->flag   = 0;
			nota_buf->size   = flistlen - 2;
			nota_buf->offset = 0;
			nota_buf->data   = ( caddr_t)flistbuf;
			nota_buf->xbuf_file = ( char *) NULL;
			
			/********
			 * Update the status of the mso_mta_job to 2 to indiacte the failure record
			 ******/
			/*job_iflistp = PIN_FLIST_CREATE(ebufp);
			job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_change_bouquet_id", -1 ,ebufp);
			PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51040" , ebufp)
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid VC number no service is associated to VC" , ebufp);
			        /* OAP sometimes send error in input */
		        /*if(input_status && (*input_status == status_fail))
        		{
                		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        		}
        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg ,job_iflistp);

        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);
	
			PCM_OP(ctxp, PCM_OP_CREATE_OBJ,0 ,job_iflistp , &job_oflistp , ebufp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for bulk retrack " );
				PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
				PIN_ERRBUF_CLEAR(ebufp);
			}
			PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
			
			if(job_oflistp)
				job_pdp = PIN_FLIST_FLD_GET(job_oflistp , PIN_FLD_POID, 1, ebufp);

			pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"Invalid VC Number", ebufp);
			if(nota_buf)
				free(nota_buf);
		}
		else
		{
			/*** read the service poid to get the account number ****/
			/*read_iflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(read_iflistp, PIN_FLD_POID,service_pdp, ebufp);
			PIN_FLIST_FLD_SET(read_iflistp, PIN_FLD_ACCOUNT_OBJ , NULL , ebufp);
			PCM_OP(ctxp, PCM_OP_READ_FLDS , 0, read_iflistp , &read_oflistp , ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " read service output flist is ", read_oflistp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " read flds error");
				PIN_FLIST_DESTROY_EX(&read_oflistp , NULL);
				PIN_ERRBUF_CLEAR(ebufp);
			}
			if(read_oflistp)
			{
				PIN_FLIST_FLD_COPY(read_oflistp ,PIN_FLD_ACCOUNT_OBJ, task_flistp, PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_COPY(read_oflistp ,PIN_FLD_ACCOUNT_OBJ, task_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " task flist after putting poid is " , task_flistp);
			}
		}
	}
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " No Device found ");
		PIN_ERRBUF_CLEAR(ebufp);
		
	}
	else
	{
		PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_USERID , PIN_POID_CREATE(db,"/account",1, ebufp), ebufp);
		PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
		PIN_FLIST_FLD_DROP(task_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
		services_flist = PIN_FLIST_ELEM_ADD(task_flistp,PIN_FLD_SERVICES, 0, ebufp);
		PIN_FLIST_FLD_SET(services_flist , PIN_FLD_SERVICE_OBJ,service_pdp , ebufp);
		catv_flist = PIN_FLIST_SUBSTR_ADD(services_flist,MSO_FLD_CATV_INFO, ebufp);
		PIN_FLIST_FLD_SET(catv_flist , MSO_FLD_BOUQUET_ID , bouquet_id , ebufp);
		
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
		}
		PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
		nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
		if ( nota_buf == NULL )
		{
			pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
			PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
			return;
		}
		/*
		* Create flist for job order creation
		*/
		/*job_iflistp = PIN_FLIST_CREATE(ebufp);
		job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_change_bouquet_id", -1 ,ebufp);
		PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_sucess , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
			
		nota_buf->flag   = 0;
		nota_buf->size   = flistlen - 2;
		nota_buf->offset = 0;
		nota_buf->data   = ( caddr_t)flistbuf;
		nota_buf->xbuf_file = ( char *) NULL;
		PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Job create input flist for bulk change bouquet id actions is " ,job_iflistp);
		        /* OAP sometimes send error in input */
		        /*if(input_status && (*input_status == status_fail))
        	{
               		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
               		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
               		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
               		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        	}
        	sprintf(msg, "Job create input flist for %s", job_name);
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg ,job_iflistp);

        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

		PCM_OP(ctxp,PCM_OP_CREATE_OBJ ,0, job_iflistp , &job_oflistp , ebufp);

		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the mso_mta job object for bulk change bouquet id actions " );
			PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);*/
			/*r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_create_job job object create failed for location_updater", ebufp);
			*ret_flistp = r_flistp;
			return;*/
		/*}
		else
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " job create output flist is " , job_oflistp);
			PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
			PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
			PIN_FLIST_DESTROY_EX(&read_oflistp , NULL);
			PIN_FLIST_DESTROY_EX(&read_iflistp , NULL);
		}
		
	}
	if(nota_buf)
	free(nota_buf);
	return;
}*/


static void
fm_mso_integ_bulk_crf(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_flist_t     **ret_flistp,
        pin_errbuf_t    *ebufp)
{
        pin_flist_t     *task_flistp = NULL;
        pin_flist_t     *order_iflistp = NULL;
        pin_flist_t     *order_oflistp = NULL;
        pin_flist_t     *r_flistp = NULL;

        poid_t          *job_pdp        = NULL;
        poid_t          *order_pdp      = NULL;
        poid_t          *task_pdp       = NULL;
        poid_t          *user_id        = NULL;

        char            *program_name   = NULL;
        char            *task_no        = NULL;
        char            *order_id       = NULL;
        int32           *adjustment_type = NULL;
        int             elem_id = 0;
   	int64		db = 0;
      //  int64           db = 1;
        int32           order_status_sucess = 0;
        int32           order_status_failure = 1;
        int32           *state_id = NULL;
        int32           status_sucess = 0;
        int32           status_fail = 2;
        pin_cookie_t    cookie = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_bulk_crf input flist", i_flistp);

        order_pdp       = PIN_FLIST_FLD_GET(i_flistp , PIN_FLD_ORDER_OBJ , 1 , ebufp);
	
	db 		= PIN_POID_GET_DB(order_pdp);

        task_no         = PIN_FLIST_FLD_GET(i_flistp , MSO_FLD_TASK_NAME , 1, ebufp);

        order_id        = PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_ORDER_ID , 1, ebufp);

        user_id         = PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_USERID , 1, ebufp);
        program_name    = PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_PROGRAM_NAME, 1 , ebufp);

        order_iflistp = PIN_FLIST_CREATE(ebufp);

        order_iflistp = PIN_FLIST_CREATE(ebufp);
        task_pdp = PIN_POID_CREATE(db , "/mso_task/bulk_crf", -1 , ebufp);
        PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
        PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
        PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Bulk CRF" , ebufp);
        PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
        PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
        PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_sucess , ebufp);
        PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
        PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_USERID , user_id , ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create for bulk crf input flist is " , order_iflistp);
        PCM_OP(ctxp,PCM_OP_CREATE_OBJ,0 , order_iflistp , &order_oflistp , ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the task object " );
                PIN_FLIST_DESTROY_EX(&order_oflistp , NULL);
                r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_bulk_crf task object for bulk set personnel bit failed", ebufp);
                //*ret_flistp = r_flistp;
                //return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create output for bulk set crf flist is " , order_oflistp);

        if(order_oflistp)
        {
                task_pdp = PIN_FLIST_FLD_GET(order_oflistp , PIN_FLD_POID , 1, ebufp);

                while ((task_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp,MSO_FLD_TASK,&elem_id, 1, &cookie, ebufp))!= NULL)
                {
                        PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PROGRAM_NAME,program_name , ebufp);
                        PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_USERID,user_id , ebufp);

                        fm_mso_integ_set_bulk_crf(ctxp,task_no,task_pdp, task_flistp , ebufp);
                        if (PIN_ERRBUF_IS_ERR(ebufp))
                        {
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the bulk set crf job object ");
                                PIN_ERRBUF_CLEAR(ebufp);
                        }
                }
        }
        else
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Task object failed ");
        }
        *ret_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID , order_pdp, ebufp);
        PIN_FLIST_FLD_SET(*ret_flistp,PIN_FLD_ORDER_ID , order_id , ebufp);
        PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS , &order_status_sucess , ebufp);
        //PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Device state updated sucessfully", ebufp);
        task_flistp = PIN_FLIST_ELEM_ADD(*ret_flistp,MSO_FLD_TASK,0, ebufp);
        PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
        PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_ID , task_no , ebufp);

        PIN_FLIST_DESTROY_EX(&order_iflistp, NULL);
        PIN_FLIST_DESTROY_EX(&order_oflistp, NULL);
        return ;
}

void
fm_mso_integ_set_bulk_crf(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*order_iflistp = NULL;
	pin_flist_t	*order_oflistp = NULL;
	pin_flist_t	*job_oflistp = NULL;
	pin_flist_t	*job_iflistp = NULL;
	pin_flist_t	*tmp_flistp = NULL;
	pin_flist_t	*read_oflistp =	NULL;
	pin_flist_t	*read_iflistp =	NULL;
	pin_flist_t	*nameinfo = NULL;
	pin_flist_t	*catv_info = NULL;
	pin_flist_t	*results_buff_flistp = NULL;
	pin_flist_t	*alias_list = NULL;
	pin_flist_t	*catv_info_substr = NULL;
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*search_inflistp = NULL;
	pin_flist_t	*search_outflistp = NULL;
	pin_flist_t	*results_flistp	= NULL;
	pin_flist_t	*services_flistp = NULL;
	pin_flist_t	*catv_info_flistp = NULL;
	pin_flist_t	*results_serv_flistp = NULL;
	pin_flist_t	*task_results_flistp = NULL;


	poid_t		*job_pdp	= NULL;
	poid_t		*order_pdp	= NULL;

	poid_t		*lco_account_pdp = NULL;
	poid_t		*device_pdp	 = NULL;
	poid_t		*service_pdp	 = NULL;

	char		*order_id	= NULL;
	char		*account_no	= NULL;
	char		*device_id	= NULL;
	char		*zip		= NULL;
	char		*network_node	= NULL;

	//int64		db = 1;
	int64		db = 0;
	int32		status_sucess =	0;
	int32		status_fail = 2;
	int32		*oap_status = NULL;
	int32		oap_status_fail	=2;
	int32		oap_status_success = 0;

	pin_buf_t	*nota_buf = NULL;
	char		*flistbuf = NULL;
	int		flistlen =0;

	int32		opcode = MSO_OP_CUST_MODIFY_CUSTOMER ;
	int32		flg_res_arr_passed = 1;
	//char		region_key[20];

//	char		search_template1[100] = " select X from /device 1 , /account 2 where  1.F1 = V1 and 1.F2 = 2.F3 ";
	char		search_template1[100] = " select X from /service where F1 = V1 ";
	char		search_template2[100] = " select X from /account where F1 = V1 ";
	char		msg[256];
	char		*bouquet_id = NULL;
	char		*region_key = NULL;
	int		search_flags = 768;
	int32		flg_fail = 0;
	int32		srvc_status_check = 0;
	int32		flg_srvc_status_invalid = 0;
	int32		*srvc_status = NULL;
	int32		flag_srch_by_device = 0;
	int32		flag_srch_by_acnt = 0;
	int32		*caf = NULL;
	int32           *input_status = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_set_bulk_crf input	flist",	task_flistp);
	input_status = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_STATUS, 1 , ebufp);	
	db = PIN_POID_GET_DB(task_pdp);
	/* Read	device_id or account_no	*/
	device_id =  PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_ID , 1 , ebufp);
	account_no =  PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_ACCOUNT_NO , 1 , ebufp);
	/* Read	and drop OAP status */
	oap_status =  PIN_FLIST_FLD_TAKE(task_flistp, PIN_FLD_STATUS , 1 , ebufp);
//	PIN_FLIST_FLD_DROP(task_flistp,PIN_FLD_STATUS,ebufp);
	

	//If BOUQUET ID	to be changed then service identifier to be send.
	catv_info_substr = PIN_FLIST_SUBSTR_GET(task_flistp, MSO_FLD_CATV_INFO,	1, ebufp);
	if (catv_info_substr)
	{
		srvc_status_check = 1;
		bouquet_id = PIN_FLIST_FLD_GET(catv_info_substr, MSO_FLD_BOUQUET_ID, 1, ebufp);
		caf = PIN_FLIST_FLD_GET(catv_info_substr, MSO_FLD_CARF_RECEIVED, 1, ebufp) ;
		region_key = PIN_FLIST_FLD_GET(catv_info_substr, MSO_FLD_REGION_KEY, 1, ebufp);

		if (bouquet_id && !device_id)
		{
			flg_fail = 1;
			memset(msg,'\0',strlen(msg));
			strcpy(msg, "DEVICE_ID mandatory for BOUQUET_ID change");
		}
 		else if (caf && !device_id)
		{
			flg_fail = 1;
			memset(msg,'\0',strlen(msg));
			strcpy(msg, "DEVICE_ID mandatory for CAF modification");
		}
		else if (region_key && !device_id)
		{
			flg_fail = 1;
			memset(msg,'\0',strlen(msg));
			strcpy(msg, "DEVICE_ID mandatory for REGION_KEY modification");
		}
	}



	search_inflistp	= PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search",	-1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
	if (device_id)
	{
		flag_srch_by_device = 1;
		PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template1, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
		alias_list = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_ALIAS_LIST, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(alias_list, PIN_FLD_NAME, device_id, ebufp);

		results_serv_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
		PIN_FLIST_FLD_SET(results_serv_flistp, PIN_FLD_POID, NULL, ebufp);
		PIN_FLIST_FLD_SET(results_serv_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
		PIN_FLIST_FLD_SET(results_serv_flistp, PIN_FLD_STATUS, NULL, ebufp);
	}
	else if	( (!device_id) && (account_no) )
	{
		flag_srch_by_acnt = 1;
		PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template2, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS,	1, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_NO,account_no, ebufp);
		/* Set result flist */
		results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
		PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID,	NULL, ebufp);
	}


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_device_object search device input	list", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp,	&search_outflistp, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in	calling	PCM_OP_SEARCH",	ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp,	NULL);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_device_object search device output flist", search_outflistp);



	if( (PIN_FLIST_ELEM_COUNT(search_outflistp , PIN_FLD_RESULTS , ebufp) <= 0)
		|| ((oap_status) && *oap_status	== oap_status_fail) 
		|| flg_fail == 1 )
	{
		/* Error case. Write in	buffer with failed status. */
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "No data found for given input ");
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "	error set");
		}
		PIN_ERRBUF_CLEAR(ebufp);
		char		*flistbuf	= NULL;
		pin_buf_t	*nota_buf	= NULL;
		flistlen = 0;

		// To keep OAP input data for later use	in failed cases
		task_results_flistp = PIN_FLIST_ELEM_GET(task_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
		if (!task_results_flistp)
		{
			flg_res_arr_passed = 0;
			results_buff_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_ELEM_SET(results_buff_flistp,task_flistp,PIN_FLD_RESULTS,0,ebufp);
		}
		else
		{
				results_buff_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_ELEM_SET(results_buff_flistp,task_results_flistp,PIN_FLD_RESULTS,0,ebufp);
		}
		PIN_FLIST_TO_STR(results_buff_flistp, &flistbuf, &flistlen, ebufp );
		nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t	) );
		if ( nota_buf == NULL )
		{
			pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
			PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf",	ebufp );
			return;
		}
		nota_buf->flag	 = 0;
		nota_buf->size	 = flistlen - 2;
		nota_buf->offset = 0;
		nota_buf->data	 = ( caddr_t)flistbuf;
		nota_buf->xbuf_file = (	char *)	NULL;

		/********
		 * Update the status of	the mso_mta_job	to 2 to	indiacte the failure record
		 ******/
		job_iflistp = PIN_FLIST_CREATE(ebufp);
		job_pdp	= PIN_POID_CREATE(db, "/mso_mta_job/bulk_crf", -1 ,ebufp);
		PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE ,	&opcode	, ebufp);
		PIN_FLIST_FLD_SET(job_iflistp ,	PIN_FLD_STATUS , &status_fail ,	ebufp);
		PIN_FLIST_FLD_SET(job_iflistp ,	PIN_FLD_ORDER_ID , task_no , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
		PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51040" , ebufp) ;
		PIN_FLIST_FLD_COPY(task_flistp,	PIN_FLD_PROGRAM_NAME, job_iflistp, PIN_FLD_PROGRAM_NAME, ebufp);
		PIN_FLIST_FLD_COPY(task_flistp,	PIN_FLD_USERID,	job_iflistp, PIN_FLD_USERID, ebufp);
		results_serv_flistp = PIN_FLIST_ELEM_GET(search_outflistp,PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
		if (results_serv_flistp)
		{
			if (flag_srch_by_device)
			{
				PIN_FLIST_FLD_COPY(results_serv_flistp, PIN_FLD_ACCOUNT_OBJ, job_iflistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
			}
			else if (flag_srch_by_acnt)
			{
				PIN_FLIST_FLD_COPY(results_serv_flistp,PIN_FLD_POID, job_iflistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
			}
			/*srvc_status = PIN_FLIST_FLD_GET(results_serv_flistp, PIN_FLD_STATUS, 1, ebufp);
			if (srvc_status && (*(int32*)srvc_status == PIN_STATUS_INACTIVE || *(int32*)srvc_status == PIN_STATUS_CLOSED )
			 && srvc_status_check == 1)
			{	
				if (bouquet_id)
				{
					memset(msg,'\0',strlen(msg));
					strcpy(msg, "Service Status should be ACTIVE for BOUQUET_ID change");
				}
				flg_srvc_status_invalid = 1;
			}*/
		}

		if((oap_status)	&& *oap_status == oap_status_fail)
		{
			//PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid Zip or Area code	in input." , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp,	PIN_FLD_ERROR_DESCR, ebufp) ;
		}
		else if (flg_fail )
		{
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, msg , ebufp);
		}
		else	
		{
			if (flag_srch_by_acnt)
			{
				memset(msg,'\0',strlen(msg));
				strcpy(msg, "Account no: '");
				strcat(msg, account_no);
				strcat(msg, "' does not exist");
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, msg, ebufp);
			}
			else if(flag_srch_by_device)
			{
				memset(msg,'\0',strlen(msg));
				strcpy(msg, "Device ID: '");
				strcat(msg, device_id);
				strcat(msg, "' does not exist / not associated with any service");
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, msg, ebufp);
			}

		}


		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Failed job creation input flist:	",job_iflistp);
		        /* OAP sometimes send error in input */
	        if(input_status && (*input_status == status_fail))
        	{
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        	}
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist" ,job_iflistp);

        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

		PCM_OP(ctxp,PCM_OP_CREATE_OBJ,0,job_iflistp,&job_oflistp,ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Failed job creation output flist: ",job_oflistp);

		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in	creating the job object	for bulk crf " );
			PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
			PIN_ERRBUF_CLEAR(ebufp);
		}
		PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);

		if(job_oflistp)
			job_pdp	= PIN_FLIST_FLD_GET(job_oflistp	, PIN_FLD_POID,	1, ebufp);

		pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"Invalid VC Number", ebufp);
		if(nota_buf)
			free(nota_buf);
	}
	else
	{
		/*
		* Non-erroneous	case.
		*/

		// To keep OAP input data for later use	in failed cases
		results_buff_flistp = PIN_FLIST_ELEM_GET(task_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
		if (!results_buff_flistp)
		{
			flg_res_arr_passed = 0;
			results_buff_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_ELEM_SET(results_buff_flistp,task_flistp,PIN_FLD_RESULTS,0,ebufp);
		}
		/*Set poid and account_obj before writing in buffer */
		//results_flistp = PIN_FLIST_ELEM_GET(search_outflistp,PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
		
		results_serv_flistp = PIN_FLIST_ELEM_GET(search_outflistp,PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
		if(results_serv_flistp)
		{
			
			srvc_status = PIN_FLIST_FLD_GET(results_serv_flistp, PIN_FLD_STATUS, 1, ebufp);
			if (srvc_status && (*(int32*)srvc_status == PIN_STATUS_INACTIVE || *(int32*)srvc_status == PIN_STATUS_CLOSED )
			 && srvc_status_check == 1)
			{	
				if (bouquet_id)
				{
					memset(msg,'\0',strlen(msg));
					strcpy(msg, "Service Status should be ACTIVE for BOUQUET_ID change");
				}
				else if (caf)
				{
					memset(msg,'\0',strlen(msg));
					strcpy(msg, "Service Status should be ACTIVE for CAF change");
				}
				else if (region_key)
				{
					memset(msg,'\0',strlen(msg));
					strcpy(msg, "Service Status should be ACTIVE for REGION_KEY change");
				}
				flg_srvc_status_invalid = 1;
			}

			/*Also set Services array in task flist	if MSO_FLD_CATV_INFO is	present*/
			catv_info_flistp = PIN_FLIST_SUBSTR_GET(task_flistp,MSO_FLD_CATV_INFO,1,ebufp);
			if(catv_info_flistp)
			{
				network_node = PIN_FLIST_FLD_GET(catv_info_flistp,MSO_FLD_NETWORK_NODE,1,ebufp);
				if(network_node)
				{
					/* Drop	network_node for the time being	*/
					PIN_FLIST_FLD_DROP(catv_info_flistp,MSO_FLD_NETWORK_NODE,ebufp);
				}

				services_flistp	= PIN_FLIST_ELEM_ADD(task_flistp,PIN_FLD_SERVICES,0,ebufp);
				PIN_FLIST_FLD_COPY(results_serv_flistp,PIN_FLD_POID, services_flistp,PIN_FLD_SERVICE_OBJ ,ebufp);
				PIN_FLIST_SUBSTR_SET(services_flistp,catv_info_flistp,MSO_FLD_CATV_INFO,ebufp);
				PIN_FLIST_SUBSTR_DROP(task_flistp,MSO_FLD_CATV_INFO,ebufp);
			}
			
			//PIN_FLIST_FLD_COPY(results_serv_flistp,PIN_FLD_POID,task_flistp,PIN_FLD_POID,ebufp);
			//PIN_FLIST_FLD_COPY(results_serv_flistp,PIN_FLD_POID,task_flistp,PIN_FLD_ACCOUNT_OBJ,ebufp);
		}

		


		/*Write	task details in	buffer */
		nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t	) );
		if ( nota_buf == NULL )
		{
			pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
			PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf",	ebufp );
			return;
		} 
		/*
		* Create flist for job order creation
		*/
		job_iflistp = PIN_FLIST_CREATE(ebufp);
		job_pdp	= PIN_POID_CREATE(db, "/mso_mta_job/bulk_crf", -1 ,ebufp);
		PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE ,	&opcode	, ebufp);
		if (flg_srvc_status_invalid)
		{
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_STATUS , &status_fail, ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, msg, ebufp);
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51040" , ebufp) ;
		}
		else
		{
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_STATUS , &status_sucess, ebufp);
		}

		if (flag_srch_by_device && results_serv_flistp )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "flag_srch_by_device");
			PIN_FLIST_FLD_COPY(results_serv_flistp,PIN_FLD_ACCOUNT_OBJ, job_iflistp, PIN_FLD_ACCOUNT_OBJ,ebufp);
			PIN_FLIST_FLD_COPY(results_serv_flistp,PIN_FLD_ACCOUNT_OBJ, task_flistp, PIN_FLD_ACCOUNT_OBJ,ebufp);
			PIN_FLIST_FLD_COPY(results_serv_flistp,PIN_FLD_ACCOUNT_OBJ, task_flistp, PIN_FLD_POID,ebufp);
		}
		else if (flag_srch_by_acnt && results_serv_flistp )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "flag_srch_by_device");
			PIN_FLIST_FLD_COPY(results_serv_flistp,PIN_FLD_POID, job_iflistp, PIN_FLD_ACCOUNT_OBJ,ebufp);
			PIN_FLIST_FLD_COPY(results_serv_flistp,PIN_FLD_POID, task_flistp, PIN_FLD_ACCOUNT_OBJ,ebufp);
			PIN_FLIST_FLD_COPY(results_serv_flistp,PIN_FLD_POID, task_flistp, PIN_FLD_POID,ebufp);
		}

		PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ORDER_ID , task_no , ebufp);
		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_PROGRAM_NAME, job_iflistp, PIN_FLD_PROGRAM_NAME, ebufp);
		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_USERID,	job_iflistp, PIN_FLD_USERID, ebufp);

		if ( flg_res_arr_passed	== 0 )
		{
			PIN_FLIST_CONCAT(task_flistp,results_buff_flistp,ebufp); // To keep OAP	input data for later use in failed cases
		}
		PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );

		nota_buf->flag	 = 0;
		nota_buf->size	 = flistlen - 2;
		nota_buf->offset = 0;
		nota_buf->data	 = ( caddr_t)flistbuf;
		nota_buf->xbuf_file = (	char *)	NULL;
		PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Job create input flist for bulk crf actions is	" ,job_iflistp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Final task_flistp" ,task_flistp);
		        /* OAP sometimes send error in input */
        	if(input_status && (*input_status == status_fail))
        	{
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        	}
       		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Validation flist" ,job_iflistp);

        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);
		PCM_OP(ctxp,PCM_OP_CREATE_OBJ ,0, job_iflistp ,	&job_oflistp , ebufp);

		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in	creating the mso_mta job object	for bulk crf actions " );
			PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
			/*r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE,	"51039", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_create_job job object create failed for location_updater", ebufp);
			*ret_flistp = r_flistp;
			return;*/
		}
		else
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG ," job create output flist is " , job_oflistp);
			PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
			PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
			PIN_FLIST_DESTROY_EX(&read_oflistp , NULL);
			PIN_FLIST_DESTROY_EX(&read_iflistp , NULL);
		}

		if(nota_buf)
		free(nota_buf);

	}
	if (flg_res_arr_passed ==0 )
	{
		PIN_FLIST_DESTROY_EX(&results_buff_flistp , NULL);
	}
	
	return;
}


static void
fm_mso_get_bulkacnt_from_acntno(
        pcm_context_t           *ctxp,
        char*                   acnt_no,
        pin_flist_t             **acnt_details,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *srch_flistp = NULL;
        pin_flist_t             *srch_oflistp = NULL;
        pin_flist_t             *arg_flist = NULL;
        pin_flist_t             *result_flist = NULL;
        pin_flist_t             *nameinfo = NULL;

        poid_t                  *srch_pdp = NULL;
        int32                   srch_flag = 512;

        int64                   db = 1;
	//int64			db = 0;
        char                    *template = "select X from /account  where F1 = V1 " ;
	poid_t			*pdp = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_bulkacnt_from_acntno", ebufp);
				return;
        }
		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bulkacnt_from_acntno :");
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, acnt_no);

	
        
        srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

        srch_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_NO, acnt_no, ebufp);

        result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp);
        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_NO, NULL, ebufp);
        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_BUSINESS_TYPE, NULL, ebufp);
        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STATUS, NULL, ebufp);
        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_CREATED_T, NULL, ebufp);
        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_MOD_T, NULL, ebufp);
        PIN_FLIST_FLD_SET(result_flist, MSO_FLD_RMN, NULL, ebufp);
        PIN_FLIST_FLD_SET(result_flist, MSO_FLD_CONTACT_PREF, NULL, ebufp);

        nameinfo= PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_NAMEINFO, PIN_ELEMID_ANY, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bulkacnt_from_acntno search input list", srch_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
		        
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ", ebufp);
                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_acnt_from_acntno search billinfo output list", srch_oflistp);

        result_flist = PIN_FLIST_ELEM_TAKE(srch_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp );
        if (result_flist)
        {
                *acnt_details = PIN_FLIST_COPY(result_flist,ebufp );
                PIN_FLIST_DESTROY_EX(&result_flist, NULL);

        }
	    /*else
		{
		      *acnt_details = PIN_FLIST_COPY(srch_flistp,ebufp );
		}*/
        PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);

        CLEANUP:
        return;
}

int32
fm_mso_get_impacted_cust_count(
	pcm_context_t		*ctxp,
	pin_flist_t		*search_inflistp,
	int32			child_account_type,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*search_outflistp = NULL;
	pin_flist_t		*prof_info = NULL;
	pin_flist_t		*arg_flist = NULL;

	int32			elemid = 0;
	pin_cookie_t		cookie = NULL;

	char			*template = "select X from /profile/customer_care where F1 = V1 and profile_t.poid_type='/profile/customer_care' " ;


	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_impacted_cust_count", ebufp);
		return 0 ;
	}
		
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_impacted_cust_count search flist :", search_inflistp);
//	PIN_FLIST_FLD_RENAME(search_inflistp, MSO_FLD_WHOLESALE_INFO, PIN_FLD_CUSTOMER_CARE_INFO, ebufp);

	arg_flist = PIN_FLIST_ELEM_GET(search_inflistp, PIN_FLD_ARGS, 1, 1, ebufp);
	prof_info = PIN_FLIST_SUBSTR_TAKE(arg_flist, MSO_FLD_WHOLESALE_INFO, 1, ebufp);
	PIN_FLIST_SUBSTR_PUT(arg_flist, prof_info, PIN_FLD_CUSTOMER_CARE_INFO, ebufp);

	PIN_FLIST_FLD_DROP(search_inflistp, PIN_FLD_TEMPLATE, ebufp);
	PIN_FLIST_ELEM_DROP(search_inflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_ELEM_SET(search_inflistp, NULL, PIN_FLD_RESULTS, 0, ebufp);

	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, template, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_impacted_cust_count modified search flist :", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_COUNT_ONLY, search_inflistp, &search_outflistp, ebufp);
	//PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);

	(void)PIN_FLIST_ELEM_GET_NEXT(search_outflistp, PIN_FLD_RESULTS, &elemid, 0, &cookie, ebufp);
	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);

	return elemid;
}


static void
fm_mso_integ_bulk_create_task_generic(
        pcm_context_t   *ctxp,
        int32           task_no,
        pin_flist_t     *i_flistp,
        pin_flist_t     **ret_flistp,
        pin_errbuf_t    *ebufp) 
{

	char		*task_name=NULL;
	char		*task_number=NULL;
	char		*task_type=NULL;
	char		*order_id=NULL;

        int32           order_status_success = 0;
        int32           order_status_failure = 1;
        int32           *svc_type = NULL;

	poid_t		*order_pdp = NULL;
	poid_t		*task_pdp = NULL;
	poid_t		*user_id = NULL;
	char		*program_name=NULL;
	char            prev_acct_agr[50] ;
	char            *acct_agr_no = NULL;
	char            prev_plan_name[256] ;
	char            *plan_name = NULL;
	char            prev_dev_id[100] ;
	char            *device_id = NULL;
	
	int             elem_id = 0;
	//int64		db = 1;
	int64		db = 0;
	char		msg[256];
	pin_cookie_t    cookie = NULL;

	pin_flist_t	*order_iflistp = NULL;
	pin_flist_t	*order_oflistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	pin_flist_t	*task_flistp = NULL;
	pin_flist_t     *sort_flistp = NULL;
	pin_flist_t     *tmp_flistp = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_bulk_create_task_generic input flist", i_flistp);


	/* Read the required data from the input flist*/
        order_pdp = PIN_FLIST_FLD_GET(i_flistp , PIN_FLD_ORDER_OBJ , 1 , ebufp);
	db = PIN_POID_GET_DB(order_pdp);
        task_number   = PIN_FLIST_FLD_GET(i_flistp , MSO_FLD_TASK_NAME , 1, ebufp);
        order_id  = PIN_FLIST_FLD_GET(i_flistp , PIN_FLD_ORDER_ID , 1, ebufp);
        user_id   = PIN_FLIST_FLD_GET(i_flistp , PIN_FLD_USERID , 1, ebufp);
	program_name = PIN_FLIST_FLD_GET(i_flistp , PIN_FLD_PROGRAM_NAME , 1, ebufp);
	svc_type = PIN_FLIST_FLD_GET(i_flistp , MSO_FLD_SERVICE_TYPE, 1, ebufp);

	/*Create the input flist for the order */
	order_iflistp = PIN_FLIST_CREATE(ebufp);

	/*Create task - Generic - Start*/
	switch (task_no) {
		case BULK_TOPUP: 
			task_name = "Bulk TOPUP";
			task_type = BULK_TOPUP_TASK_TYPE;
			break;
		case BULK_RENEW: 
			task_name = "Bulk RENEW";
			task_type = BULK_RENEW_TASK_TYPE;
			break;
		case BULK_EXTEND_VALIDITY: 
			task_name = "Bulk Extend Validity";
			task_type = BULK_EXTEND_VALIDITY_TASK_TYPE;
			break;
		case BULK_ADDITION_MB_GB: 
			task_name = "Bulk Addition MB_GB";
			task_type = BULK_ADDITION_MB_GB_TASK_TYPE;
			break;
		case BULK_WRITEOFF_CPE: 
			task_name = "Bulk Writeoff CPE";
			task_type = BULK_WRITEOFF_CPE_TASK_TYPE;
			break;
		case BULK_SWAPPING_CPE: 
			task_name = "Bulk Swapping CPE";
			task_type = BULK_SWAPPING_CPE_TASK_TYPE;
			break;
		case BULK_CHECK_BOUNCE: 
			task_name = "Bulk Check Bounce";
			task_type = BULK_CHECK_BOUNCE_TASK_TYPE;
			break;
		case BULK_BILL_HOLD: 
			task_name = "Bulk Bill Hold";
			task_type = BULK_BILL_HOLD_TASK_TYPE;
			break;
		case BULK_REFUND_LOAD: 
			task_name = "Bulk Refund Load";
			task_type = BULK_REFUND_LOAD_TASK_TYPE;
			break;
		case BULK_REFUND_REV_LOAD: 
			task_name = "Bulk Refund Reversal Load";
			task_type = BULK_REFUND_REV_LOAD_TASK_TYPE;
			break;
		case BULK_CMTS_CHANGE:
			task_name = "Bulk CMTS Change";
			task_type = BULK_CMTS_CHANGE_TASK_TYPE;
			break;

	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, task_name);
	/*Create Job - Generic - End*/

        task_pdp = PIN_POID_CREATE(db, task_type, -1, ebufp);
        PIN_FLIST_FLD_PUT(order_iflistp, PIN_FLD_POID, task_pdp, ebufp);
        PIN_FLIST_FLD_SET(order_iflistp, MSO_FLD_ORDER_OBJ, order_pdp, ebufp);
        PIN_FLIST_FLD_SET(order_iflistp, MSO_FLD_ORDER_TYPE, task_type, ebufp);
        PIN_FLIST_FLD_SET(order_iflistp, PIN_FLD_ORDER_ID, order_id, ebufp);
        PIN_FLIST_FLD_SET(order_iflistp, PIN_FLD_NUMBER, task_number, ebufp);
        PIN_FLIST_FLD_SET(order_iflistp, PIN_FLD_STATUS, &order_status_success, ebufp);
        PIN_FLIST_FLD_SET(order_iflistp, PIN_FLD_USERID, user_id, ebufp);
	PIN_FLIST_FLD_SET(order_iflistp, PIN_FLD_PROGRAM_NAME, program_name, ebufp);
	
	sprintf(msg, "Task create for %s input flist is", task_name);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , msg , order_iflistp);

        PCM_OP(ctxp,PCM_OP_CREATE_OBJ,0 , order_iflistp , &order_oflistp , ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp)) {
	    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the task object " );
	    PIN_FLIST_DESTROY_EX(&order_oflistp , NULL);

	    r_flistp = PIN_FLIST_CREATE(ebufp);
	    PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
	    PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
	    PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
	    sprintf(msg, "fm_mso_integ_bulk_create_task_generic task object for %s failed", task_name);
	    PIN_FLIST_FLD_SET(r_flistp,PIN_FLD_ERROR_DESCR, msg, ebufp);
        }
	sprintf(msg, "Task create output for %s flist is", task_name);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , msg , order_oflistp);

        if(order_oflistp)
	{
		task_pdp = PIN_FLIST_FLD_GET(order_oflistp , PIN_FLD_POID , 1, ebufp);
		sort_flistp = PIN_FLIST_CREATE(ebufp);
		tmp_flistp = PIN_FLIST_ELEM_ADD(sort_flistp, MSO_FLD_TASK, 0, ebufp);
		if ( svc_type != NULL && *svc_type == 0 ){ //CATV bulk renewal input has ACCOUNT_NO
			PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_NO, NULL, ebufp);
		}
		else{//BB renewal
			PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_AGREEMENT_NO, NULL, ebufp);
		}
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_DEVICE_ID, NULL, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_PLAN_NAME, NULL, ebufp);
		PIN_FLIST_SORT(i_flistp, sort_flistp, 1, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Task Array Sorted", i_flistp);
		PIN_FLIST_DESTROY_EX(&sort_flistp, NULL);

		while ((task_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp,MSO_FLD_TASK,&elem_id, 1, &cookie, ebufp))!= NULL) {
		
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PROGRAM_NAME, program_name, ebufp);		
			PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_USERID, user_id, ebufp);
			
			if (task_no == BULK_TOPUP)
			{
				//fm_mso_integ_create_job_generic(ctxp,task_flistp, task_no,task_number, task_pdp,task_flistp,user_id, ebufp);
				fm_mso_integ_topup_create_job(ctxp,task_number,task_pdp,task_flistp,ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
				    sprintf(msg, "Error in creating the job object for %s", task_name);
				    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , msg );
				    PIN_ERRBUF_CLEAR(ebufp);
				}
			}
			else if (task_no == BULK_RENEW)
			{
				if ( svc_type != NULL && *svc_type == 0 ){ //CATV bulk renewal input has ACCOUNT_NO
					acct_agr_no =  PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp);
				}
				else{ // IF BB renewal, get AGREEMENT_NO
					acct_agr_no =  PIN_FLIST_FLD_GET(task_flistp, MSO_FLD_AGREEMENT_NO, 1, ebufp);
				}
				device_id =  PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_ID, 1, ebufp);
				plan_name =  PIN_FLIST_FLD_GET(task_flistp, MSO_FLD_PLAN_NAME, 1, ebufp);
					// Added to fix the duplicate renewal issue
				if ( (acct_agr_no != NULL && prev_acct_agr != NULL && strcmp(prev_acct_agr, acct_agr_no) == 0)&& 											(device_id != NULL && prev_dev_id != NULL && strcmp(prev_dev_id, device_id) == 0 )&& 											(plan_name !=  NULL &&prev_plan_name != NULL  && strcmp(prev_plan_name, plan_name) == 0 )) 
				{
					sprintf(msg, "Duplicate Record Exists.: %s", acct_agr_no);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG ,msg);
					continue;
				}
				fm_mso_integ_renew_create_job(ctxp,task_number,task_pdp,task_flistp,ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
				    sprintf(msg, "Error in creating the job object for %s", task_name);
				    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , msg );
				    PIN_ERRBUF_CLEAR(ebufp);
				}
				if ( acct_agr_no != NULL && device_id != NULL && plan_name != NULL )
				{
					strncpy(prev_acct_agr, acct_agr_no, sizeof(prev_acct_agr));
					strncpy(prev_dev_id, device_id, sizeof(prev_dev_id));
					strncpy(prev_plan_name, plan_name, sizeof(prev_plan_name));
				}
				
			}
			else if (task_no == BULK_EXTEND_VALIDITY)
			{
				fm_mso_integ_extend_validity_create_job(ctxp,task_number,task_pdp,task_flistp,ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
				    sprintf(msg, "Error in creating the job object for %s", task_name);
				    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , msg );
				    PIN_ERRBUF_CLEAR(ebufp);
				}
			}
			else if (task_no == BULK_ADDITION_MB_GB)
			{
	//			fm_mso_integ_add_mb_gb_create_job(ctxp,task_number,task_pdp,task_flistp,ebufp);
	//			if (PIN_ERRBUF_IS_ERR(ebufp))
	//			{
	//			    sprintf(msg, "Error in creating the job object for %s", task_name);
	//			    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , msg );
	//			    PIN_ERRBUF_CLEAR(ebufp);
	//			}
				fm_mso_integ_create_add_plan_job(ctxp,task_number,task_pdp,task_flistp, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the job object for add bm & GB plan " );
					PIN_ERRBUF_CLEAR(ebufp);
				}
			}
			else if (task_no == BULK_WRITEOFF_CPE)
			{
				fm_mso_integ_cpe_writeoff_create_job(ctxp,task_number,task_pdp,task_flistp,ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
				    sprintf(msg, "Error in creating the job object for %s", task_name);
				    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , msg );
				    PIN_ERRBUF_CLEAR(ebufp);
				}
			}
			else if (task_no == BULK_SWAPPING_CPE)
			{
				fm_mso_integ_swaping_cpe_create_job(ctxp,task_number,task_pdp,task_flistp,ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
				    sprintf(msg, "Error in creating the job object for %s", task_name);
				    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , msg );
				    PIN_ERRBUF_CLEAR(ebufp);
				}
			}
			else if (task_no == BULK_CHECK_BOUNCE)
			{
				fm_mso_integ_cheque_bounce_create_job(ctxp,task_number,task_pdp,task_flistp,ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
				    sprintf(msg, "Error in creating the job object for %s", task_name);
				    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , msg );
				    PIN_ERRBUF_CLEAR(ebufp);
				}
			}
			else if (task_no == BULK_BILL_HOLD)
			{
				fm_mso_integ_bill_hold_create_job(ctxp,task_number,task_pdp,task_flistp,ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
				    sprintf(msg, "Error in creating the job object for %s", task_name);
				    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , msg );
				    PIN_ERRBUF_CLEAR(ebufp);
				}
			}
			else if (task_no == BULK_REFUND_LOAD)
			{
				fm_mso_integ_refund_load_create_job(ctxp,task_number,task_pdp,task_flistp,ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
				    sprintf(msg, "Error in creating the job object for %s", task_name);
				    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , msg );
				    PIN_ERRBUF_CLEAR(ebufp);
				}
			}
			else if (task_no == BULK_REFUND_REV_LOAD)
			{
				fm_mso_integ_refund_reversal_load_create_job(ctxp,task_number,task_pdp,task_flistp,ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
				    sprintf(msg, "Error in creating the job object for %s", task_name);
				    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , msg );
				    PIN_ERRBUF_CLEAR(ebufp);
				}
			}
			else if (task_no == BULK_CMTS_CHANGE)
			{
				fm_mso_integ_cmts_change_create_job(ctxp,task_number,task_pdp,task_flistp,ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
				    sprintf(msg, "Error in creating the job object for %s", task_name);
				    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , msg );
				    PIN_ERRBUF_CLEAR(ebufp);
				}
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Task object failed ");
			}
        }
	}else {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Task object failed ");
        }


        *ret_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID , order_pdp, ebufp);
        PIN_FLIST_FLD_SET(*ret_flistp,PIN_FLD_ORDER_ID , order_id , ebufp);
        PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS , &order_status_success , ebufp);
        task_flistp = PIN_FLIST_ELEM_ADD(*ret_flistp,MSO_FLD_TASK,0, ebufp);
        PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
        PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_ID , task_number , ebufp);

        PIN_FLIST_DESTROY_EX(&order_iflistp, NULL);
        PIN_FLIST_DESTROY_EX(&order_oflistp, NULL);
	
}


//void
//fm_mso_integ_create_job_generic(
//        pcm_context_t   *ctxp,
//        pin_flist_t     *i_flistp,
//        int             task_no,
//        char            *order_id,
//        poid_t          *task_pdp,
//        pin_flist_t     *task_flistp,
//        poid_t          *user_pdp,
//        pin_errbuf_t    *ebufp) 
//{
//
//	char		*job_name = NULL;
//	char		*job_type = NULL;
//	char		*flistbuf = NULL;
//	pin_buf_t	*nota_buf = NULL;
//
//	int		flistlen = 0;
//	int		db = 1;
//	int32		opcode = 1;
//
//	pin_flist_t	*job_iflistp = NULL; 
//	pin_flist_t	*job_oflistp = NULL; 
//	poid_t		*job_pdp = NULL; 
//	poid_t		*dummy_pdp = NULL; 
//        int32           status_success = 0;
//        int32           status_fail = 2;
//	char		msg[256];
//	char		*tmp_data=NULL;
//	char		*program_name=NULL;
//	char		*template=NULL;
//	void		*generic_ptr = NULL;
//	pin_flist_t	*arg_flistp = NULL;
//	pin_flist_t	*tmp_arg_flistp = NULL;
//	pin_flist_t	*tmp_bb_info_flistp = NULL;
//	pin_flist_t	*tmp_flistp = NULL;
//	pin_flist_t	*plan_flistp = NULL;
//	pin_flist_t	*product_flistp = NULL;
//	int32		elem_id = 0;
//	int32		reason = 1;
//	pin_cookie_t	cookie = NULL;
//	int		invalid_task_no = 0;
//
//        if (PIN_ERRBUF_IS_ERR(ebufp))
//                PIN_ERRBUF_CLEAR(ebufp);
//
//        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_create_job_generic input flist", task_flistp);
//
//        switch (task_no) {
//            case BULK_TOPUP:
//                job_name = BULK_TOPUP_DESCR;
//		job_type = BULK_TOPUP_JOB_TYPE;
//		opcode = OPCODE_BULK_TOPUP; 
//		break;
//            case BULK_RENEW:
//                job_name = BULK_RENEW_DESCR;
//		job_type = BULK_RENEW_JOB_TYPE;
//		opcode = OPCODE_BULK_RENEW; 
//		break;
//            case BULK_ADDITION_MB_GB:
//                job_name = BULK_ADDITION_MB_GB_DESCR;
//                job_type = BULK_ADDITION_MB_GB_JOB_TYPE;
//                opcode = OPCODE_BULK_ADDITION_MB_GB;
//		break;
//            case BULK_EXTEND_VALIDITY:
//                job_name = BULK_EXTEND_VALIDITY_DESCR;
//		job_type = BULK_EXTEND_VALIDITY_JOB_TYPE;
//		opcode = OPCODE_BULK_EXTEND_VALIDITY; 
//		break;
//            case BULK_BILL_HOLD:
//                job_name = BULK_BILL_HOLD_DESCR;
//                job_type = BULK_BILL_HOLD_JOB_TYPE;
//                opcode = OPCODE_BULK_BILL_HOLD;
//		break;
//            case BULK_SWAPPING_CPE:
//                job_name = BULK_SWAPPING_CPE_DESCR;
//                job_type = BULK_SWAPPING_CPE_JOB_TYPE;
//                opcode = OPCODE_BULK_SWAPPING_CPE;
//		break;
//            case BULK_WRITEOFF_CPE:
//                job_name = BULK_WRITEOFF_CPE_DESCR;
//                job_type = BULK_WRITEOFF_CPE_JOB_TYPE;
//                opcode = OPCODE_BULK_WRITEOFF_CPE;
//
//
//		break;
//            case BULK_CHECK_BOUNCE:
//		job_name = BULK_CHECK_BOUNCE_DESCR;
//		job_type = BULK_CHECK_BOUNCE_JOB_TYPE;
//		opcode = OPCODE_BULK_CHECK_BOUNCE; 
//		dummy_pdp = PIN_POID_CREATE(db, "/account", -1, ebufp);
//		PIN_FLIST_FLD_PUT(task_flistp,PIN_FLD_POID,dummy_pdp,ebufp);
//		break;
//            case BULK_REFUND_LOAD:
//		job_name = BULK_REFUND_LOAD_DESCR;
//		job_type = BULK_REFUND_LOAD_JOB_TYPE;
//		opcode = OPCODE_BULK_REFUND_LOAD; 
//		dummy_pdp = PIN_POID_CREATE(db, "/account", -1, ebufp);
//		PIN_FLIST_FLD_PUT(task_flistp,PIN_FLD_POID,dummy_pdp,ebufp);
//		break;
//            case BULK_REFUND_REV_LOAD:
//		job_name = BULK_REFUND_REV_LOAD_DESCR;
//		job_type = BULK_REFUND_REV_LOAD_JOB_TYPE;
//		opcode = OPCODE_BULK_REFUND_REV_LOAD; 
//		dummy_pdp = PIN_POID_CREATE(db, "/account", -1, ebufp);
//		PIN_FLIST_FLD_PUT(task_flistp,PIN_FLD_POID,dummy_pdp,ebufp);
//		break;
//	    default:
//		invalid_task_no = 1;
//		break;
//	}


//	if (invalid_task_no != 1) {
//		PIN_FLIST_FLD_SET(task_flistp,PIN_FLD_USERID,user_pdp,ebufp);
//		if (task_no != BULK_CHECK_BOUNCE && task_no != BULK_REFUND_LOAD && 
//			task_no != BULK_REFUND_REV_LOAD && task_no != BULK_WRITEOFF_CPE) {
//			sprintf(msg, "OAP|%s", job_name);
//			program_name = msg;
//			PIN_FLIST_FLD_SET(task_flistp,PIN_FLD_PROGRAM_NAME, program_name,ebufp);
//
//			/*Populating the respective POIDs required - Start*/
//			template = "select X from /account where F1 = V1 ";
//			arg_flistp = PIN_FLIST_CREATE(ebufp);
//			tmp_arg_flistp = PIN_FLIST_ELEM_ADD (arg_flistp, PIN_FLD_ARGS, 1, ebufp);
//			tmp_data = PIN_FLIST_FLD_TAKE (task_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
//			PIN_FLIST_FLD_PUT(tmp_arg_flistp, PIN_FLD_ACCOUNT_NO, tmp_data, ebufp);
//			fm_mso_get_object_poid(ctxp, template, arg_flistp, &generic_ptr, ebufp);
//			PIN_FLIST_DESTROY_EX(&arg_flistp, NULL);
//			if (generic_ptr != NULL) {
//				if (task_no != BULK_EXTEND_VALIDITY && 
//					task_no != BULK_SWAPPING_CPE && task_no != BULK_WRITEOFF_CPE)
//					PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ACCOUNT_OBJ, generic_ptr, ebufp);
//				PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_POID, generic_ptr, ebufp);
//			
//				if (task_no == BULK_BILL_HOLD) {
//					/* Populating billinfo poid */
//                                	arg_flistp = PIN_FLIST_CREATE(ebufp);
//                                	tmp_arg_flistp = PIN_FLIST_ELEM_ADD (arg_flistp, PIN_FLD_ARGS, 1, ebufp);
//					PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_ACCOUNT_OBJ, 
//							tmp_arg_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
//                                	tmp_arg_flistp = PIN_FLIST_ELEM_ADD (arg_flistp, PIN_FLD_ARGS, 2, ebufp);
//                                	PIN_FLIST_FLD_SET(tmp_arg_flistp, PIN_FLD_BILLINFO_ID, "BB", ebufp);
//                                	template = "select X from /billinfo where F1 = V1 and F2 = V2 ";
//                                	generic_ptr = NULL;
//                                	fm_mso_get_object_poid(ctxp, template, arg_flistp, &generic_ptr, ebufp);
//                                	if (generic_ptr != NULL) {
//						tmp_flistp = PIN_FLIST_ELEM_ADD(task_flistp, PIN_FLD_BILLINFO, 0, ebufp);
//                                        	PIN_FLIST_FLD_PUT(tmp_flistp, PIN_FLD_POID, generic_ptr, ebufp);
//                                        	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_REASON, &reason, ebufp);
//						PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_FLAGS, tmp_flistp, PIN_FLD_FLAGS, ebufp); 
//                                	}
//				}
//				else if (task_no != BULK_WRITEOFF_CPE) {
//					/* Populating service poid */
//					tmp_data = PIN_FLIST_FLD_TAKE (task_flistp, MSO_FLD_AGREEMENT_NO, 0, ebufp);
//					arg_flistp = PIN_FLIST_CREATE(ebufp);
//					tmp_arg_flistp = PIN_FLIST_ELEM_ADD (arg_flistp, PIN_FLD_ARGS, 1, ebufp);
//					tmp_bb_info_flistp = PIN_FLIST_SUBSTR_ADD (tmp_arg_flistp, MSO_FLD_BB_INFO, ebufp);
//					PIN_FLIST_FLD_PUT(tmp_bb_info_flistp, MSO_FLD_AGREEMENT_NO, tmp_data, ebufp);
//					template = "select X from /service/telco/broadband where F1 = V1 ";
//					generic_ptr = NULL;
//					fm_mso_get_object_poid(ctxp, template, arg_flistp, &generic_ptr, ebufp);
//					if (generic_ptr != NULL) {
//						PIN_FLIST_FLD_PUT(task_flistp,PIN_FLD_SERVICE_OBJ, generic_ptr, ebufp);
//					}
//
//					if (task_no == BULK_TOPUP || task_no == BULK_RENEW || task_no == BULK_ADDITION_MB_GB) {
//                                            /* Populating plan poid */
//                                            plan_flistp = PIN_FLIST_ELEM_GET (task_flistp, PIN_FLD_PLAN, 0, 0, ebufp);
//                                            tmp_data = PIN_FLIST_FLD_TAKE (plan_flistp, PIN_FLD_NAME, 0, ebufp);
//                                            arg_flistp = PIN_FLIST_CREATE(ebufp);
//                                            tmp_arg_flistp = PIN_FLIST_ELEM_ADD (arg_flistp, PIN_FLD_ARGS, 1, ebufp);
//                                            PIN_FLIST_FLD_PUT(tmp_arg_flistp, PIN_FLD_NAME, tmp_data, ebufp);
//                                            template = "select X from /plan where F1 = V1 ";
//                                            generic_ptr = NULL;
//                                            fm_mso_get_object_poid(ctxp, template, arg_flistp, &generic_ptr, ebufp);
//                                            if (generic_ptr != NULL) {
//                                                PIN_FLIST_FLD_PUT(plan_flistp, PIN_FLD_PLAN_OBJ, generic_ptr, ebufp);
//                                            }
//
//                                            /* Populating product poid */
//					    while (product_flistp = PIN_FLIST_ELEM_GET_NEXT(plan_flistp, PIN_FLD_PRODUCTS,
//                                        	&elem_id, 1, &cookie, ebufp)) {
//                                        	    tmp_data = PIN_FLIST_FLD_TAKE (product_flistp, PIN_FLD_NAME, 0, ebufp);
//                                        	    arg_flistp = PIN_FLIST_CREATE(ebufp);
//                                        	    tmp_arg_flistp = PIN_FLIST_ELEM_ADD (arg_flistp, PIN_FLD_ARGS, 1, ebufp);
//                                        	    PIN_FLIST_FLD_PUT(tmp_arg_flistp, PIN_FLD_NAME, tmp_data, ebufp);
//                                        	    template = "select X from /product where F1 = V1 ";
//                                        	    generic_ptr = NULL;
//                                        	    fm_mso_get_object_poid(ctxp, template, arg_flistp, &generic_ptr, ebufp);
//                                        	    if (generic_ptr != NULL) {
//                                                	PIN_FLIST_FLD_PUT(product_flistp, PIN_FLD_PRODUCT_OBJ, generic_ptr, ebufp);
//                                        	    }
//                                            }
//                                        }
//
//				}
//			}
//			/*Populating the respective POIDs required - End*/
//        	}
//	}
//        if (PIN_ERRBUF_IS_ERR(ebufp))
//		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Error Here1");
//
//	if (task_no == BULK_WRITEOFF_CPE) {
//		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "BULK_WRITEOFF_CPE i_flistp is:",i_flistp);
//		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, task_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
//	}
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "task_flistp is:",i_flistp);
//        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, job_type);
//	
//        PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
//        nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
//        if ( nota_buf == NULL ) {
//                pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
//                PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
//                return;
//        }
//
//	PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "Error buffer", ebufp);
//
//        if (PIN_ERRBUF_IS_ERR(ebufp))
//		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Error Here2");
//	job_iflistp = PIN_FLIST_CREATE(ebufp);
//	job_pdp = PIN_POID_CREATE(db, job_type, -1 ,ebufp);
//	PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
//	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_success , ebufp);
//	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
//	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
//	PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
//
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp1:", job_iflistp);
//	nota_buf->flag   = 0;
//	nota_buf->size   = flistlen - 2;
//	nota_buf->offset = 0;
//	nota_buf->data   = ( caddr_t)flistbuf;
//	nota_buf->xbuf_file = ( char *) NULL;
//	PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
//
//	sprintf(msg, "Job create input flist for %s", job_name);
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg ,job_iflistp);
//
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);
//
//	PCM_OP(ctxp,PCM_OP_CREATE_OBJ ,0, job_iflistp , &job_oflistp , ebufp);
//
//	if (PIN_ERRBUF_IS_ERR(ebufp)) {
//		sprintf(msg, "Error in creating the mso_mta job object for %s", job_name );
//		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , msg);
//		PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
//	}
//	else {
//		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " job create output flist is " , job_oflistp);
//		PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
//		PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
//	}
//	
//	if(nota_buf)
//		free(nota_buf);
//	
//	return;
//
//}




static void 
fm_validate_sp_code(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp)
{
	//int64		db = 0;
	int		db = 1;
	int32		srch_flag = 512;
	pin_flist_t	*arg_flist = NULL;
	pin_flist_t	*result_flist = NULL;
	pin_flist_t	*r_flistp = NULL;
	pin_flist_t	*srch_flistp = NULL;
	pin_flist_t	*srch_oflistp = NULL;
	char		*sp_account_no = NULL;
	poid_t		*srch_pdp = NULL;
	int32           order_status_success = 0;
        int32           order_status_failure = 1;
	int32		sp_btype = 19000000;
	int32		*pdp = NULL;
	char		*template = "select X from /account  where F1 = V1 and F2 = V2 " ;


	

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_validate_sp_code  input list", i_flistp);
	
	
	
	sp_account_no = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_STOCK_POINT_CODE, 0, ebufp);
        srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

        srch_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_NO, sp_account_no, ebufp);
	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_BUSINESS_TYPE, &sp_btype, ebufp);

        result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_validate_sp_code search input list", srch_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

        if (PIN_ERRBUF_IS_ERR(ebufp) || (PIN_FLIST_ELEM_COUNT(srch_oflistp, PIN_FLD_RESULTS,ebufp ) <1))
        {
		        
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in searching SP account", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID,PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp), ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51049", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS , &order_status_failure , ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error in searching Stock point account", ebufp);
		*r_flistpp = r_flistp;
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_validate_sp_code output list", *r_flistpp);
		//PIN_FLIST_DESTROY_EX(&r_flistp , NULL);
		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
                return;
        }

	r_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID,PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp), ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS , &order_status_success , ebufp);
	*r_flistpp = r_flistp;
	//*r_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_validate_sp_code output list", *r_flistpp);
	
	//PIN_FLIST_DESTROY_EX(&r_flistp , NULL);
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);

	return;    

}

// BB New bilk action related job creation


void 
fm_mso_integ_topup_create_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp)
{
	char		*job_name = NULL;
	char		*job_type = NULL;
	char		*flistbuf = NULL;
	pin_buf_t	*nota_buf = NULL;

	int		flistlen = 0;
	int64		db = 0;
	//int		db = 1;
	int32		opcode = 1;

	pin_flist_t	*job_iflistp = NULL; 
	pin_flist_t	*job_oflistp = NULL; 
	poid_t		*job_pdp = NULL; 
        int32           status_success = 0;
        int32           status_fail = 2;
	int32           validation_status = 1;
	char		msg[256];
	char		*device_id = NULL;
	char		*agreement_no = NULL;
	char		*plan_name = NULL;
	int32           *input_status = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_topup_create_job input flist", task_flistp);
	input_status = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_STATUS, 1 , ebufp);
	db = PIN_POID_GET_DB(task_pdp);
	job_name = BULK_TOPUP_DESCR;
	job_type = BULK_TOPUP_JOB_TYPE;
	opcode = OPCODE_BULK_TOPUP; 

	job_iflistp = PIN_FLIST_CREATE(ebufp);
	job_pdp = PIN_POID_CREATE(db, job_type, -1 ,ebufp);

	PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_success , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "task_flistp is:",task_flistp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, job_type);
	device_id = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_ID,1, ebufp);
	agreement_no = PIN_FLIST_FLD_GET(task_flistp, MSO_FLD_AGREEMENT_NO,1, ebufp);
	plan_name = PIN_FLIST_FLD_GET(task_flistp, MSO_FLD_PLAN_NAME,1, ebufp);
	if((device_id )&& (strcmp(device_id,"") != 0))
	{
		fm_mso_search_bb_device_object(ctxp, task_flistp , ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51041" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid Mac address" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
			validation_status = 2;
		}
	}
	else if ((agreement_no )&& (strcmp(agreement_no,"") != 0))
	{
		fm_mso_get_bb_service_agreement_no(ctxp,agreement_no, &task_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51040" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid Agreement No" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
			validation_status = 2;
		}
	}
	else
	{
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51042" , ebufp)
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "VC or Agreement No missing in input" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
			validation_status = 2;
	}

	if(validation_status == 1 && ((plan_name) && (strcmp(plan_name,"") != 0)))
	{
		PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_ACCOUNT_OBJ,task_flistp , PIN_FLD_POID , ebufp);
		fm_mso_get_plan_poid(ctxp,plan_name, &task_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51043" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Incorrect Plan input" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
			validation_status = 2;
		}

	}
	else
	{
		if(validation_status == 1)
		{
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51043" , ebufp)
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Plan name not passed in input" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
			validation_status = 2;
		}
	}


	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "task_flistp after modiifcation:",task_flistp);
	
	PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
	nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
	if ( nota_buf == NULL )
	{
		pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
		return;
	}

	PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "Error buffer", ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Error Here2");


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp1:", job_iflistp);
	nota_buf->flag   = 0;
	nota_buf->size   = flistlen - 2;
	nota_buf->offset = 0;
	nota_buf->data   = ( caddr_t)flistbuf;
	nota_buf->xbuf_file = ( char *) NULL;
	PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );

	/* OAP sometimes send error in input */
	if(input_status && (*input_status == status_fail))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        }

	sprintf(msg, "Job create input flist for %s", job_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg ,job_iflistp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

	PCM_OP(ctxp,PCM_OP_CREATE_OBJ ,0, job_iflistp , &job_oflistp , ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		sprintf(msg, "Error in creating the mso_mta job object for %s", job_name );
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , msg);
		PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
		PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
	}
	else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " job create output flist is " , job_oflistp);
		PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
		PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
	}
	
	if(nota_buf)
		free(nota_buf);
	
	return;
}


void 
fm_mso_integ_renew_create_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp)
{
	char		*job_name = NULL;
	char		*job_type = NULL;
	char		*flistbuf = NULL;
	pin_buf_t	*nota_buf = NULL;

	int		flistlen = 0;
	int64		db = 0;
	//int		db = 1;
	int32		opcode = 1;

	pin_flist_t	*job_iflistp = NULL; 
	pin_flist_t	*job_oflistp = NULL; 
	poid_t		*job_pdp = NULL; 
        int32           status_success = 0;
        int32           status_fail = 2;
	int32           validation_status = 1;
	char		msg[256];
	char		*device_id = NULL;
	char		*agreement_no = NULL;
	char		*plan_name = NULL;
	int32		*renew_catv_flag = NULL;
	int32		*renew_catv_srv_type = NULL;
	pin_flist_t 	*plan_flistp = NULL;
	time_t		current_time ;
	time_t		*purchase_end_t = NULL;
	char 		renew_msg[256];
	int32           *input_status = NULL;
	///Added for CATV renewal

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_renew_create_job input flist", task_flistp);
       renew_catv_flag = (int32 *) PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_FLAGS,1,ebufp);
	input_status = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_STATUS, 1 , ebufp);
	db = PIN_POID_GET_DB(task_pdp);
	job_name = BULK_RENEW_DESCR;
	job_type = BULK_RENEW_JOB_TYPE;
	if (renew_catv_flag && *renew_catv_flag == 1)
	{
		opcode = MSO_OP_INTEG_ADD_PLAN ; 
	}
	else
	{
		opcode = OPCODE_BULK_RENEW; 
	}

	job_iflistp = PIN_FLIST_CREATE(ebufp);
	job_pdp = PIN_POID_CREATE(db, job_type, -1 ,ebufp);
	PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_success , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "task_flistp is:",task_flistp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, job_type);
	device_id = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_ID,1, ebufp);
	agreement_no = PIN_FLIST_FLD_GET(task_flistp, MSO_FLD_AGREEMENT_NO,1, ebufp);
	plan_name = PIN_FLIST_FLD_GET(task_flistp, MSO_FLD_PLAN_NAME,1, ebufp);
	if((device_id )&& (strcmp(device_id,"") != 0))
	{
		fm_mso_search_bb_device_object(ctxp, task_flistp , ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "Error in search of device", ebufp );
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51041" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid Mac address" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
			validation_status = 2;
		}
	}
	else if ((agreement_no )&& (strcmp(agreement_no,"") != 0))
	{
		fm_mso_get_bb_service_agreement_no(ctxp,agreement_no, &task_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "Error in search of agreement no", ebufp );
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51040" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid Agreement No" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
			validation_status = 2;
		}
	}
	else
	{
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51042" , ebufp)
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "VC or Agreement No missing in input" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
			validation_status = 2;
	}

	if(validation_status == 1 && ((plan_name) && (strcmp(plan_name,"") != 0)))
	{
		PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_ACCOUNT_OBJ,task_flistp , PIN_FLD_POID , ebufp);
		plan_flistp = PIN_FLIST_ELEM_ADD(task_flistp,PIN_FLD_PLAN,0,ebufp);
		PIN_FLIST_FLD_SET(plan_flistp,PIN_FLD_NAME,plan_name,ebufp);
		PIN_FLIST_FLD_SET(task_flistp,PIN_FLD_DESCR,"Bulk_renew_plan",ebufp);
	       if (renew_catv_flag && *renew_catv_flag == 1)
		{	
			purchase_end_t = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_PURCHASE_END_T,1,ebufp);
			sprintf(renew_msg,"purchase end_T %d",*purchase_end_t);
			PIN_ERR_LOG_MSG(3,renew_msg);
			if(purchase_end_t && *purchase_end_t !=0)
			{
				current_time = pin_virtual_time(NULL);
				sprintf(renew_msg,"current time %d",current_time);
				PIN_ERR_LOG_MSG(3,renew_msg);
				if(*purchase_end_t < current_time)
				{
				              	PIN_ERRBUF_CLEAR(ebufp);
	                                        PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51044" , ebufp);
        	                                PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Renewal date cannot be less than current date" , ebufp);
                	                        PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
                        	                validation_status = 2;
                       		
				}
			}
				
			fm_mso_get_purchase_poid(ctxp,plan_name,&task_flistp,ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
					PIN_ERRBUF_CLEAR(ebufp);
					PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51043" , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Incorrect Plan input" , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
					validation_status = 2;
			}

		}
		else
		{		
			fm_mso_get_plan_poid(ctxp,plan_name, &task_flistp, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERRBUF_CLEAR(ebufp);
				PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51043" , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Incorrect Plan input" , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
				validation_status = 2;
			}
		}
	}
	else
	{
		if(validation_status == 1)
		{
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51043" , ebufp)
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Plan name not passed in input" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
			validation_status = 2;
		}
	}


	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "task_flistp after modiifcation:",task_flistp);
	
	PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
	nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
	if ( nota_buf == NULL )
	{
		pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
		return;
	}

	PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "Error buffer", ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Error Here2");


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp1:", job_iflistp);
	nota_buf->flag   = 0;
	nota_buf->size   = flistlen - 2;
	nota_buf->offset = 0;
	nota_buf->data   = ( caddr_t)flistbuf;
	nota_buf->xbuf_file = ( char *) NULL;
	PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );

	/* OAP sometimes send error in input */
	if(input_status && (*input_status == status_fail))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        }

	sprintf(msg, "Job create input flist for %s", job_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg ,job_iflistp);
	PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ACCOUNT_OBJ,job_iflistp,PIN_FLD_ACCOUNT_OBJ,ebufp);
	PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_PROGRAM_NAME,job_iflistp,PIN_FLD_PROGRAM_NAME,ebufp);
	PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_USERID,job_iflistp,PIN_FLD_USERID,ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

	PCM_OP(ctxp,PCM_OP_CREATE_OBJ ,0, job_iflistp , &job_oflistp , ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		sprintf(msg, "Error in creating the mso_mta job object for %s", job_name );
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , msg);
		PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
		PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
	}
	else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " job create output flist is " , job_oflistp);
		PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
		PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
	}
	
	if(nota_buf)
		free(nota_buf);
	return;
}



void 
fm_mso_integ_add_mb_gb_create_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp)
{
	char		*job_name = NULL;
	char		*job_type = NULL;
	char		*flistbuf = NULL;
	pin_buf_t	*nota_buf = NULL;

	int		flistlen = 0;
	int64		db = 0;
	//int		db = 1;
	int32		opcode = 1;

	pin_flist_t	*job_iflistp = NULL; 
	pin_flist_t	*job_oflistp = NULL; 
	poid_t		*job_pdp = NULL; 
        int32           status_success = 0;
        int32           status_fail = 2;
	int32           validation_status = 1;
	char		msg[256];
	char		*device_id = NULL;
	char		*agreement_no = NULL;
	char		*plan_name = NULL;
	int		service_type=1;
	int32		add_plan_flag = 2;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_add_mb_gb_create_job input flist", task_flistp);
	db = PIN_POID_GET_DB(task_pdp);
	job_name = BULK_ADDITION_MB_GB_DESCR;
	job_type = BULK_ADDITION_MB_GB_JOB_TYPE;
	opcode = MSO_OP_INTEG_ADD_PLAN;

	job_iflistp = PIN_FLIST_CREATE(ebufp);
	job_pdp = PIN_POID_CREATE(db, job_type, -1 ,ebufp);
	PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_success , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "task_flistp is:",task_flistp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, job_type);
	device_id = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_ID,1, ebufp);
	agreement_no = PIN_FLIST_FLD_GET(task_flistp, MSO_FLD_AGREEMENT_NO,1, ebufp);
	plan_name = PIN_FLIST_FLD_GET(task_flistp, MSO_FLD_PLAN_NAME,1, ebufp);
	PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_FLAGS , &add_plan_flag, ebufp);
	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_SERVICE_TYPE , &service_type, ebufp);
// Nagaraj: changed to fix failed_get_job API flow
	if((plan_name) && (strcmp(plan_name,"") != 0))
	{
		PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_ACCOUNT_OBJ,task_flistp , PIN_FLD_POID , ebufp);
		fm_mso_get_plan_poid(ctxp,plan_name, &task_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51043" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Incorrect Plan input" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
			validation_status = 2;
		}

	}
	else
	{
		PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51043" , ebufp)
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Plan name not passed in input" , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
		validation_status = 2;	
	}
	if((device_id )&& (strcmp(device_id,"") != 0))
	{
		fm_mso_search_bb_device_object(ctxp, task_flistp , ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51041" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid Mac address" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
			validation_status = 2;
		}
	}
	else if ((agreement_no )&& (strcmp(agreement_no,"") != 0))
	{
		fm_mso_get_bb_service_agreement_no(ctxp,agreement_no, &task_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51040" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid Agreement No" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
			validation_status = 2;
		}
	}
	else
	{
		if(validation_status == 1)
		{	
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51042" , ebufp)
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "VC or Agreement No missing in input" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
			validation_status = 2;
		}
	}



	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "task_flistp after modiifcation:",task_flistp);
	
	PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
	nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
	if ( nota_buf == NULL )
	{
		pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
		return;
	}

	PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "Error buffer", ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Error Here2");


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp1:", job_iflistp);
	nota_buf->flag   = 0;
	nota_buf->size   = flistlen - 2;
	nota_buf->offset = 0;
	nota_buf->data   = ( caddr_t)flistbuf;
	nota_buf->xbuf_file = ( char *) NULL;
	PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );

	sprintf(msg, "Job create input flist for %s", job_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg ,job_iflistp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

	PCM_OP(ctxp,PCM_OP_CREATE_OBJ ,0, job_iflistp , &job_oflistp , ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		sprintf(msg, "Error in creating the mso_mta job object for %s", job_name );
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , msg);
		PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
		PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
	}
	else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " job create output flist is " , job_oflistp);
		PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
		PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
	}
	
	if(nota_buf)
		free(nota_buf);
	
	return;
}

void 
fm_mso_integ_extend_validity_create_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp)
{
	char		*job_name = NULL;
	char		*job_type = NULL;
	char		*flistbuf = NULL;
	pin_buf_t	*nota_buf = NULL;

	int		flistlen = 0;
	int64		db = 0;
	//int		db = 1;
	int32		opcode = 1;

	pin_flist_t	*job_iflistp = NULL; 
	pin_flist_t	*job_oflistp = NULL; 
	poid_t		*job_pdp = NULL; 
        int32           status_success = 0;
        int32           status_fail = 2;
	int32           validation_status = 1;
	char		msg[256];
	char		*device_id = NULL;
	char		*agreement_no = NULL;
	char		*plan_name = NULL;
	int32           *input_status = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_extend_validity_create_job input flist", task_flistp);
	input_status = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_STATUS, 1 , ebufp);
	
	db = PIN_POID_GET_DB(task_pdp);
	job_name = BULK_EXTEND_VALIDITY_DESCR;
	job_type = BULK_EXTEND_VALIDITY_JOB_TYPE;
	opcode = OPCODE_BULK_EXTEND_VALIDITY; 

	job_iflistp = PIN_FLIST_CREATE(ebufp);
	job_pdp = PIN_POID_CREATE(db, job_type, -1 ,ebufp);
	PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_success , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "task_flistp is:",task_flistp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, job_type);
	device_id = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_ID,1, ebufp);
	agreement_no = PIN_FLIST_FLD_GET(task_flistp, MSO_FLD_AGREEMENT_NO,1, ebufp);
	if((device_id )&& (strcmp(device_id,"") != 0))
	{
		fm_mso_search_bb_device_object(ctxp, task_flistp , ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51041" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid Mac address" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
			validation_status = 2;
		}
	}
	else if ((agreement_no )&& (strcmp(agreement_no,"") != 0))
	{
		fm_mso_get_bb_service_agreement_no(ctxp,agreement_no, &task_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51040" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid Agreement No" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
			validation_status = 2;
		}
	}
	else
	{
		if(validation_status == 1)
		{
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51042" , ebufp)
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "VC or Agreement No missing in input" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
			validation_status = 2;
		}
	}

	if(validation_status == 1)
	{
		PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_ACCOUNT_OBJ,task_flistp , PIN_FLD_POID , ebufp);
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "task_flistp after modiifcation:",task_flistp);
	
	
	PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
	nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
	if ( nota_buf == NULL )
	{
		pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
		return;
	}

	PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "Error buffer", ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Error Here2");


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp1:", job_iflistp);
	nota_buf->flag   = 0;
	nota_buf->size   = flistlen - 2;
	nota_buf->offset = 0;
	nota_buf->data   = ( caddr_t)flistbuf;
	nota_buf->xbuf_file = ( char *) NULL;
	PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );

	/* OAP sometimes send error in input */
	if(input_status && (*input_status == status_fail))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        }

	sprintf(msg, "Job create input flist for %s", job_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg ,job_iflistp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

	PCM_OP(ctxp,PCM_OP_CREATE_OBJ ,0, job_iflistp , &job_oflistp , ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		sprintf(msg, "Error in creating the mso_mta job object for %s", job_name );
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , msg);
		PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
		PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
	}
	else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " job create output flist is " , job_oflistp);
		PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
		PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
	}
	
	if(nota_buf)
		free(nota_buf);
	
	return;
}

void 
fm_mso_integ_bill_hold_create_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp)
{
	char		*job_name = NULL;
	char		*job_type = NULL;
	char		*flistbuf = NULL;
	pin_buf_t	*nota_buf = NULL;

	int		flistlen = 0;
	int64		db = 0;
	//int		db = 1;
	int32		opcode = 1;

	pin_flist_t	*job_iflistp = NULL; 
	pin_flist_t	*job_oflistp = NULL; 
	poid_t		*job_pdp = NULL;
	pin_flist_t	*billinfo_flistp = NULL; 
	poid_t		*billinfo_obj = NULL;
        int32           status_success = 0;
        int32           status_fail = 2;
	int32           validation_status = 1;
	char		msg[256];
	char		*device_id = NULL;
	char		*agreement_no = NULL;
	char		*plan_name = NULL;
	int32           *input_status = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_bill_hold_create_job input flist", task_flistp);
	input_status = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_STATUS, 1 , ebufp);
	
	db = PIN_POID_GET_DB(task_pdp);
	job_name = BULK_BILL_HOLD_DESCR;
	job_type = BULK_BILL_HOLD_JOB_TYPE;
	opcode = OPCODE_BULK_BILL_HOLD;

	job_iflistp = PIN_FLIST_CREATE(ebufp);
	job_pdp = PIN_POID_CREATE(db, job_type, -1 ,ebufp);
	PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_success , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "task_flistp is:",task_flistp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, job_type);
	device_id = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_ID,1, ebufp);
	agreement_no = PIN_FLIST_FLD_GET(task_flistp, MSO_FLD_AGREEMENT_NO,1, ebufp);
	if((device_id )&& (strcmp(device_id,"") != 0))
	{
		fm_mso_search_bb_device_object(ctxp, task_flistp , ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Invalid Mac address");
			PIN_ERRBUF_CLEAR(ebufp);
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51041" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid Mac address" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
			validation_status = 2;
		}
	}
	else if ((agreement_no )&& (strcmp(agreement_no,"") != 0))
	{
		fm_mso_get_bb_service_agreement_no(ctxp,agreement_no, &task_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Invalid Agreement No");
			PIN_ERRBUF_CLEAR(ebufp);
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51040" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid Agreement No" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
			validation_status = 2;
		}
	}
	else
	{
		if(validation_status == 1)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "VC or Agreement No missing in input");
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51042" , ebufp)
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "VC or Agreement No missing in input" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
			validation_status = 2;
		}
	}

	

	if(validation_status == 1)
	{
		PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_ACCOUNT_OBJ,task_flistp , PIN_FLD_POID , ebufp);
		fm_mso_get_bb_billinfo(ctxp,&task_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51045" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "No BB Billinfo Found" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
			validation_status = 2;
		}

	}

	billinfo_flistp = PIN_FLIST_ELEM_GET(task_flistp, PIN_FLD_BILLINFO,PIN_ELEMID_ANY,0, ebufp);
	billinfo_obj  = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_BILLINFO_OBJ,1,ebufp);
	PIN_FLIST_FLD_SET(billinfo_flistp , PIN_FLD_POID ,billinfo_obj, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "task_flistp after modiifcation:",task_flistp);
	
	PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
	nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
	if ( nota_buf == NULL )
	{
		pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
		return;
	}

	PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "Error buffer", ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Error Here2");


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp1:", job_iflistp);
	nota_buf->flag   = 0;
	nota_buf->size   = flistlen - 2;
	nota_buf->offset = 0;
	nota_buf->data   = ( caddr_t)flistbuf;
	nota_buf->xbuf_file = ( char *) NULL;
	PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );

	/* OAP sometimes send error in input */
	if(input_status && (*input_status == status_fail))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        }
	sprintf(msg, "Job create input flist for %s", job_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg ,job_iflistp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

	PCM_OP(ctxp,PCM_OP_CREATE_OBJ ,0, job_iflistp , &job_oflistp , ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		sprintf(msg, "Error in creating the mso_mta job object for %s", job_name );
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , msg);
		PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
		PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
	}
	else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " job create output flist is " , job_oflistp);
		PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
		PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
	}
	
	if(nota_buf)
		free(nota_buf);
	
	return;
}

void 
fm_mso_integ_cheque_bounce_create_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp)
{
	char		*job_name = NULL;
	char		*job_type = NULL;
	char		*flistbuf = NULL;
	pin_buf_t	*nota_buf = NULL;

	int		flistlen = 0;
	int		db = 0;
	//int		db = 1;
	int32		opcode = 1;

	pin_flist_t	*job_iflistp = NULL; 
	pin_flist_t	*job_oflistp = NULL; 
	poid_t		*job_pdp = NULL; 
        int32           status_success = 0;
        int32           status_fail = 2;
	char		msg[256];
	int32           *input_status = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_create_job_generic input flist", task_flistp);
	input_status = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_STATUS, 1 , ebufp);
	
	db = PIN_POID_GET_DB(task_pdp);
	job_name = BULK_CHECK_BOUNCE_DESCR;
	job_type = BULK_CHECK_BOUNCE_JOB_TYPE;
	opcode = OPCODE_BULK_CHECK_BOUNCE; 

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "task_flistp is:",task_flistp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, job_type);
	PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID,task_flistp , PIN_FLD_POID , ebufp);
	
	PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
	nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
	if ( nota_buf == NULL )
	{
		pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
		return;
	}

	PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "Error buffer", ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Error Here2");
	job_iflistp = PIN_FLIST_CREATE(ebufp);
	job_pdp = PIN_POID_CREATE(db, job_type, -1 ,ebufp);
	PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_success , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp1:", job_iflistp);
	nota_buf->flag   = 0;
	nota_buf->size   = flistlen - 2;
	nota_buf->offset = 0;
	nota_buf->data   = ( caddr_t)flistbuf;
	nota_buf->xbuf_file = ( char *) NULL;
	PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );

	/*OAP sometime send error in input */
	if(input_status && (*input_status == status_fail))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        }

	sprintf(msg, "Job create input flist for %s", job_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg ,job_iflistp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

	PCM_OP(ctxp,PCM_OP_CREATE_OBJ ,0, job_iflistp , &job_oflistp , ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		sprintf(msg, "Error in creating the mso_mta job object for %s", job_name );
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , msg);
		PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
		PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
	}
	else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " job create output flist is " , job_oflistp);
		PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
		PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
	}
	
	if(nota_buf)
		free(nota_buf);
	
	return;
}


void 
fm_mso_integ_refund_load_create_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp)
{
	char		*job_name = NULL;
	char		*job_type = NULL;
	char		*flistbuf = NULL;
	pin_buf_t	*nota_buf = NULL;

	int		flistlen = 0;
	int64		db = 0;
	//int		db = 1;
	int32		opcode = 1;

	pin_flist_t	*job_iflistp = NULL; 
	pin_flist_t	*job_oflistp = NULL; 
	poid_t		*job_pdp = NULL; 
        int32           status_success = 0;
        int32           status_fail = 2;
	char		msg[256];
	int32           *input_status = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_create_job_generic input flist", task_flistp);
	input_status = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_STATUS, 1 , ebufp);
	db = PIN_POID_GET_DB(task_pdp);
	job_name = BULK_REFUND_LOAD_DESCR;
	job_type = BULK_REFUND_LOAD_JOB_TYPE;
	opcode = OPCODE_BULK_REFUND_LOAD; 

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "task_flistp is:",task_flistp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, job_type);
	PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID,task_flistp , PIN_FLD_POID , ebufp);
	
	PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
	nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
	if ( nota_buf == NULL )
	{
		pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
		return;
	}

	PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "Error buffer", ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Error Here2");
	job_iflistp = PIN_FLIST_CREATE(ebufp);
	job_pdp = PIN_POID_CREATE(db, job_type, -1 ,ebufp);
	PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_success , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp1:", job_iflistp);
	nota_buf->flag   = 0;
	nota_buf->size   = flistlen - 2;
	nota_buf->offset = 0;
	nota_buf->data   = ( caddr_t)flistbuf;
	nota_buf->xbuf_file = ( char *) NULL;
	PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );

	/*OAP sometime send error in input*/
	if(input_status && (*input_status == status_fail))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        }
	sprintf(msg, "Job create input flist for %s", job_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg ,job_iflistp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

	PCM_OP(ctxp,PCM_OP_CREATE_OBJ ,0, job_iflistp , &job_oflistp , ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		sprintf(msg, "Error in creating the mso_mta job object for %s", job_name );
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , msg);
		PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
		PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
	}
	else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " job create output flist is " , job_oflistp);
		PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
		PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
	}
	
	if(nota_buf)
		free(nota_buf);
	
	return;
}

void 
fm_mso_integ_refund_reversal_load_create_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp)
{
	char		*job_name = NULL;
	char		*job_type = NULL;
	char		*flistbuf = NULL;
	pin_buf_t	*nota_buf = NULL;

	int		flistlen = 0;
	int64		db = 0;
	//int		db = 1;
	int32		opcode = 1;

	pin_flist_t	*job_iflistp = NULL; 
	pin_flist_t	*job_oflistp = NULL; 
	poid_t		*job_pdp = NULL; 
        int32           status_success = 0;
        int32           status_fail = 2;
	char		msg[256];
	int32           *input_status = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_create_job_generic input flist", task_flistp);
	input_status = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_STATUS, 1 , ebufp);
	db = PIN_POID_GET_DB(task_pdp);
	job_name = BULK_REFUND_REV_LOAD_DESCR;
	job_type = BULK_REFUND_REV_LOAD_JOB_TYPE;
	opcode = OPCODE_BULK_REFUND_REV_LOAD; 

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "task_flistp is:",task_flistp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, job_type);
	PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_USERID,task_flistp , PIN_FLD_POID , ebufp);
	
	PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
	nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
	if ( nota_buf == NULL )
	{
		pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
		return;
	}

	PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "Error buffer", ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Error Here2");
	job_iflistp = PIN_FLIST_CREATE(ebufp);
	job_pdp = PIN_POID_CREATE(db, job_type, -1 ,ebufp);
	PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_success , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp1:", job_iflistp);
	nota_buf->flag   = 0;
	nota_buf->size   = flistlen - 2;
	nota_buf->offset = 0;
	nota_buf->data   = ( caddr_t)flistbuf;
	nota_buf->xbuf_file = ( char *) NULL;
	PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );

	/*OAP sometimes send error in input */
	if(input_status && (*input_status == status_fail))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        }

	sprintf(msg, "Job create input flist for %s", job_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg ,job_iflistp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

	PCM_OP(ctxp,PCM_OP_CREATE_OBJ ,0, job_iflistp , &job_oflistp , ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		sprintf(msg, "Error in creating the mso_mta job object for %s", job_name );
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , msg);
		PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
		PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
	}
	else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " job create output flist is " , job_oflistp);
		PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
		PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
	}
	
	if(nota_buf)
		free(nota_buf);
	
	return;
}

void 
fm_mso_integ_cpe_writeoff_create_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp)
{
	char		*job_name = NULL;
	char		*job_type = NULL;
	char		*flistbuf = NULL;
	pin_buf_t	*nota_buf = NULL;

	int		flistlen = 0;
	int64		db = 0;
	//int		db = 1;
	int32		opcode = 1;

	pin_flist_t	*job_iflistp = NULL; 
	pin_flist_t	*job_oflistp = NULL; 
	poid_t		*job_pdp = NULL;
	poid_t		*dummy_pdp = NULL; 
        int32           status_success = 0;
        int32           status_fail = 2;
	char		msg[256];
	int32           *input_status = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_create_job_generic input flist", task_flistp);
	input_status = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_STATUS, 1 , ebufp);
	
	db = PIN_POID_GET_DB(task_pdp);
	job_name = BULK_WRITEOFF_CPE_DESCR;
	job_type = BULK_WRITEOFF_CPE_JOB_TYPE;
	opcode = OPCODE_BULK_WRITEOFF_CPE;

	dummy_pdp = PIN_POID_CREATE(db, job_type, -1 ,ebufp);

	PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_POID, dummy_pdp , ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "task_flistp is:",task_flistp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, job_type);
	
	PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
	nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
	if ( nota_buf == NULL )
	{
		pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
		return;
	}

	PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "Error buffer", ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Error Here2");
	job_iflistp = PIN_FLIST_CREATE(ebufp);
	job_pdp = PIN_POID_CREATE(db, job_type, -1 ,ebufp);
	PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_success , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp1:", job_iflistp);
	nota_buf->flag   = 0;
	nota_buf->size   = flistlen - 2;
	nota_buf->offset = 0;
	nota_buf->data   = ( caddr_t)flistbuf;
	nota_buf->xbuf_file = ( char *) NULL;
	PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );

	/* OAP sometimes send error in input */
	if(input_status && (*input_status == status_fail))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        }

	sprintf(msg, "Job create input flist for %s", job_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg ,job_iflistp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

	PCM_OP(ctxp,PCM_OP_CREATE_OBJ ,0, job_iflistp , &job_oflistp , ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		sprintf(msg, "Error in creating the mso_mta job object for %s", job_name );
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , msg);
		PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
		PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
	}
	else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " job create output flist is " , job_oflistp);
		PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
		PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
	}
	
	if(nota_buf)
		free(nota_buf);
	
	return;
}


void 
fm_mso_integ_swaping_cpe_create_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp)
{
	char		*job_name = NULL;
	char		*job_type = NULL;
	char		*flistbuf = NULL;
	pin_buf_t	*nota_buf = NULL;

	int		flistlen = 0;
	int64		db = 0;
	//int		db = 1;
	int32		opcode = 1;

	pin_flist_t	*job_iflistp = NULL; 
	pin_flist_t	*job_oflistp = NULL; 
	poid_t		*job_pdp = NULL;
        int32           status_success = 0;
	int32           status = 0;
        int32           status_fail = 2;
	char		msg[256];
	char		*agreement_no = NULL;
	char		*account_no = NULL;
	int32           *input_status = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_swaping_cpe_create_job input flist", task_flistp);
	job_name = BULK_SWAPPING_CPE_DESCR;
	job_type = BULK_SWAPPING_CPE_JOB_TYPE;
	opcode = OPCODE_BULK_SWAPPING_CPE;

	db = PIN_POID_GET_DB(task_pdp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "task_flistp is:",task_flistp);
	input_status = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_STATUS, 1 , ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, job_type);
	job_iflistp = PIN_FLIST_CREATE(ebufp);
	job_pdp = PIN_POID_CREATE(db, job_type, -1 ,ebufp);
	PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
	
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);

	agreement_no = PIN_FLIST_FLD_GET(task_flistp, MSO_FLD_AGREEMENT_NO,0, ebufp);
	fm_mso_get_bb_service_agreement_no(ctxp,agreement_no, &task_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51040" , ebufp)
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Invalid Agreement No" , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_success , ebufp);
		PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_ACCOUNT_OBJ,task_flistp , PIN_FLD_POID , ebufp);
	}

	PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_POID , task_pdp , ebufp);


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "task_flistp before conversion:",task_flistp);	
	PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
	nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
	if ( nota_buf == NULL )
	{
		pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
		return;
	}

	PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "Error buffer", ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Error Here2");

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp1:", job_iflistp);
	nota_buf->flag   = 0;
	nota_buf->size   = flistlen - 2;
	nota_buf->offset = 0;
	nota_buf->data   = ( caddr_t)flistbuf;
	nota_buf->xbuf_file = ( char *) NULL;
	PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );

	/* OAP sometimes send error in input */
	if(input_status && (*input_status == status_fail))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        }

	sprintf(msg, "Job create input flist for %s", job_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg ,job_iflistp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

	PCM_OP(ctxp,PCM_OP_CREATE_OBJ ,0, job_iflistp , &job_oflistp , ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		sprintf(msg, "Error in creating the mso_mta job object for %s", job_name );
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , msg);
		PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
		PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
	}
	else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " job create output flist is " , job_oflistp);
		PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
		PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
	}
	
	if(nota_buf)
		free(nota_buf);
	
	return;
}


/********************************************
*  Function  : BULK_CMTS_CHANGE 34  
*  Developer : Ayyapa Raju
*  Creates   :
*	      /mso_order/bulk_cmts_change
*             /mso_task/bulk_cmts_change
*	      /mso_mta_job/bulk_cmts_change
*********************************************/

void 
fm_mso_integ_cmts_change_create_job(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	pin_flist_t	*task_flistp,
	pin_errbuf_t	*ebufp)
{
	char		*job_name = NULL;
	char		*job_type = NULL;
	char		*flistbuf = NULL;
	pin_buf_t	*nota_buf = NULL;

	int		flistlen = 0;
	int64		db = 0;
	//int		db = 1;
	int32		opcode = 1;

	pin_flist_t	*job_iflistp = NULL; 
	pin_flist_t	*job_oflistp = NULL; 
	poid_t		*job_pdp = NULL;
        int32           status_success = 0;
	int32           status = 0;
        int32           status_fail = 2;
	char		msg[256];
	char            *device_id = NULL;
	char		*account_no = NULL;
	poid_t		*service_pdp = NULL;
	poid_t		*account_pdp = NULL;
	int32		bal_index = 0;
	pin_flist_t     *services_flistp = NULL;
	pin_flist_t     *inherited_info_flistp = NULL;
	pin_flist_t     *actual_task_flistp = NULL;
	char            *network_node = NULL;
	char            *network_amp = NULL;
	char 		*network_element = NULL;
	poid_t          *user_pdp = NULL;
	char            *program_name = NULL;
	pin_flist_t     *bb_info_flistp = NULL;
        char            *city = NULL;
        int             *technologyp = NULL;
	int32		common_flags = 1;
	pin_flist_t	*common_oflistp = NULL;
	int              elem_common = 0;
	pin_cookie_t    cookie_common = NULL;
	char		*result_cmts_id = NULL;
	int32		cmts_matching_flag = 0;
	int32		node_matching_flag = 0;
	int32           amp_matching_flag = 0;
	pin_flist_t	*results_flistp = NULL;
	char            *result_node_id = NULL;
	char            *result_amp_id = NULL;
	pin_flist_t     *copy_task_flistp = NULL;
	int32           *input_status = NULL;
	



	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_cmts_change_create_job input flist", task_flistp);

	/* Set JOB Parameters and Ready to Create JOB */

	job_name = BULK_CMTS_CHANGE_DESCR;
	job_type = BULK_CMTS_CHANGE_JOB_TYPE;
	opcode = OPCODE_BULK_CMTS_CHANGE;

	db = PIN_POID_GET_DB(task_pdp);
	input_status = PIN_FLIST_FLD_GET(task_flistp,PIN_FLD_STATUS, 1 , ebufp);
	copy_task_flistp = PIN_FLIST_COPY(task_flistp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "BULK CMTS CHANGE task_flistp is:",task_flistp);


        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, job_type);
        job_iflistp = PIN_FLIST_CREATE(ebufp);
        job_pdp = PIN_POID_CREATE(db, job_type, -1 ,ebufp);
        PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);

        PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
        PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
        PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);


	/* Get Mandatory and Optional Fields from Input TASK FLIST*/

        network_node = PIN_FLIST_FLD_GET(task_flistp, MSO_FLD_NETWORK_NODE, 1, ebufp);
        network_amp = PIN_FLIST_FLD_GET(task_flistp, MSO_FLD_NETWORK_AMPLIFIER, 1, ebufp);
	network_element = PIN_FLIST_FLD_GET(task_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
	account_no = PIN_FLIST_FLD_GET(task_flistp , PIN_FLD_ACCOUNT_NO, 0 , ebufp);
	device_id =  PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_ID , 0 , ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51005" , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Mandatory Inputs Missing" , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
	}

        /*OAP sometimes send error in input */
        if(input_status && (*input_status == status_fail))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Input Validation failed @ OAP");
                PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
                PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
        }


	/* Fetch Account and Service Details from Input Device ID */
	if((device_id ) && (strcmp(device_id,"") != 0) && 	
			(NULL == PIN_FLIST_FLD_GET(job_iflistp, PIN_FLD_ERROR_CODE, 1, ebufp)))
	{
		fm_mso_search_bb_device_object(ctxp, task_flistp , ebufp);
		service_pdp = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
		account_pdp = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51006" , ebufp);
                        PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "INVALID Device ID" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
		}
	}


	/*get City and Technology Information */

	if(NULL == PIN_FLIST_FLD_GET(job_iflistp, PIN_FLD_ERROR_CODE, 1 , ebufp))
	{
		fm_mso_get_service_agreement_no(ctxp,service_pdp, task_flistp, ebufp);
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
				"BULK CMTS CHANGE After calling  Agreement task_flistp is " ,task_flistp);
		city = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_CITY, 0, ebufp);
		technologyp = PIN_FLIST_FLD_GET(task_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp);

		if (PIN_ERRBUF_IS_ERR(ebufp))
        	{
        		PIN_ERRBUF_CLEAR(ebufp);
                	PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51007" , ebufp);
                	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "ERROR FETCHING CUSTOMER DETAILS" , ebufp);
                	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
		
        	}
	}

        /***********************************************
         * Three Major Validations of Input
         * 1. CMTS and CITY Validation
         * 2. NODE Validation if exists in input 
         * 3. Amplifier Validation if exists in input
        ********************************************/


	/*Step 1 : CMTS and CITY Validation */

	if(NULL == PIN_FLIST_FLD_GET(job_iflistp, PIN_FLD_ERROR_CODE, 1, ebufp))
	{
		PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_POID, account_pdp, ebufp);
		PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_FLAGS, &common_flags, ebufp);
		PCM_OP(ctxp, MSO_OP_CUST_BB_GET_INFO ,0, task_flistp , &common_oflistp , ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
			"BULK CMTS CHANGE After calling CMTS Info Opcode", common_oflistp);

		while ((results_flistp = 
				PIN_FLIST_ELEM_GET_NEXT(common_oflistp, PIN_FLD_RESULTS, 
					&elem_common, 1, &cookie_common, ebufp)) != NULL)
		{
			result_cmts_id = PIN_FLIST_FLD_GET(results_flistp, MSO_FLD_CMTS_ID, 1, ebufp);
			if((result_cmts_id) && (!strcmp(result_cmts_id, network_element)))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CMTS ID is Matching with the CITY");
				cmts_matching_flag = 1;
			}
		}
		if (PIN_ERRBUF_IS_ERR(ebufp) || cmts_matching_flag == 0)
        	{
                	PIN_ERRBUF_CLEAR(ebufp);
                	PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51008" , ebufp);
                	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Issue with the input CMTS ID or CITY " , ebufp);
                	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
		}
        }

        /*Step 2 : NODE ID VALIDATION */


	if(cmts_matching_flag == 1  && network_node &&
			(NULL == PIN_FLIST_FLD_GET(job_iflistp, PIN_FLD_ERROR_CODE, 1, ebufp)))
	{
		common_flags = 2;
		elem_common  = 0;
		cookie_common = NULL;
		PIN_FLIST_DESTROY_EX(&common_oflistp, NULL);
		results_flistp = NULL;

		PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_FLAGS, &common_flags, ebufp);
		PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_CMTS_ID, network_element, ebufp);  
        	PCM_OP(ctxp, MSO_OP_CUST_BB_GET_INFO ,0, task_flistp , &common_oflistp , ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "BULK CMTS CHANGE After calling NODE Info Opcode", common_oflistp);

        	while ((results_flistp =
                                PIN_FLIST_ELEM_GET_NEXT(common_oflistp, PIN_FLD_RESULTS,
                                        &elem_common, 1, &cookie_common, ebufp)) != NULL)
        	{
                	result_node_id = PIN_FLIST_FLD_GET(results_flistp, MSO_FLD_NODE_ID, 1, ebufp);
                	if((result_node_id) && (!strcmp(result_node_id, network_node)))
                	{
                        	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "NODE ID Details are matching");
                        	node_matching_flag = 1;
                	}
        	}
		if (PIN_ERRBUF_IS_ERR(ebufp) || node_matching_flag == 0)
		{
			PIN_ERRBUF_CLEAR(ebufp);
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51009" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Issue with the input NODE ID " , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
		}
	}


        /*Step 2 : AMPLIFIER ID VALIDATION */

        if(network_amp && 
			(NULL == PIN_FLIST_FLD_GET(job_iflistp, PIN_FLD_ERROR_CODE, 1, ebufp)))
        {
		if(node_matching_flag == 1)
		{ 
                	common_flags = 3;
			elem_common  = 0;
			cookie_common = NULL;
			PIN_FLIST_DESTROY_EX(&common_oflistp, NULL);
			results_flistp = NULL;

                	PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_FLAGS, &common_flags, ebufp);
                	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_NODE_ID, network_node, ebufp);
                	PCM_OP(ctxp, MSO_OP_CUST_BB_GET_INFO ,0, task_flistp , &common_oflistp , ebufp);
                	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "BULK CMTS CHANGE After calling NODE Info Opcode", common_oflistp);

                	while ((results_flistp =
                                PIN_FLIST_ELEM_GET_NEXT(common_oflistp, PIN_FLD_RESULTS,
                                        &elem_common, 1, &cookie_common, ebufp)) != NULL)
                	{
                        	result_amp_id = PIN_FLIST_FLD_GET(results_flistp, MSO_FLD_AMPLIFIER_ID, 1, ebufp);
                        	if((result_amp_id) && (!strcmp(result_amp_id, network_amp)))
                        	{
                                	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "AMP ID Details are matching");
                                	amp_matching_flag = 1;
                        	}
                	}
                	if (PIN_ERRBUF_IS_ERR(ebufp) || amp_matching_flag == 0)
                	{
                        	PIN_ERRBUF_CLEAR(ebufp);
                        	PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51010" , ebufp);
                        	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Issue with the input AMPLIFIER ID " , ebufp);
                        	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
                	}
		}
		else if ( node_matching_flag == 0)
		{
			PIN_ERRBUF_CLEAR(ebufp);
			PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51011" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Input Also Require NODE Details" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
		}


	}


	/****************************************
	*	MAIN JOB CREATION LOGIC		* 
        *       ALL VALIDATIONS DONE .		*
        *	/mso_mta_job/bulk_cmts_change	*
 	*	created now			* 
	****************************************/ 
		

	/* If Any error code set then set the original Task Flist to the JOB */
	if(NULL == PIN_FLIST_FLD_GET(job_iflistp, PIN_FLD_ERROR_CODE, 1, ebufp))
	{
		actual_task_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(actual_task_flistp ,PIN_FLD_POID , account_pdp , ebufp);

		user_pdp = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_USERID, 0, ebufp);
		PIN_FLIST_FLD_SET(actual_task_flistp , PIN_FLD_USERID, user_pdp, ebufp);
		PIN_FLIST_FLD_SET(actual_task_flistp , PIN_FLD_ACCOUNT_OBJ , account_pdp , ebufp);
		PIN_FLIST_FLD_SET(actual_task_flistp ,PIN_FLD_ACCOUNT_NO, account_no, ebufp);
		PIN_FLIST_FLD_SET(actual_task_flistp ,PIN_FLD_DEVICE_ID, device_id, ebufp);
		PIN_FLIST_FLD_SET(actual_task_flistp ,MSO_FLD_NETWORK_NODE, network_node, ebufp);
		PIN_FLIST_FLD_SET(actual_task_flistp ,MSO_FLD_NETWORK_AMPLIFIER, network_amp, ebufp);
		PIN_FLIST_FLD_SET(actual_task_flistp ,MSO_FLD_NETWORK_ELEMENT, network_element, ebufp);
	
	
		program_name = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_PROGRAM_NAME, 0, ebufp);
		PIN_FLIST_FLD_SET(actual_task_flistp , PIN_FLD_PROGRAM_NAME, program_name, ebufp);
	
 		
		services_flistp = PIN_FLIST_ELEM_ADD(actual_task_flistp, PIN_FLD_SERVICES, 1, ebufp);
		PIN_FLIST_FLD_SET(services_flistp ,PIN_FLD_SERVICE_OBJ , service_pdp , ebufp);
		PIN_FLIST_FLD_SET(services_flistp, PIN_FLD_BAL_INFO_INDEX , &bal_index , ebufp);
		inherited_info_flistp = PIN_FLIST_SUBSTR_ADD(services_flistp,PIN_FLD_INHERITED_INFO, ebufp);
		bb_info_flistp = PIN_FLIST_SUBSTR_ADD(inherited_info_flistp, MSO_FLD_BB_INFO, ebufp);

		if(network_node)
		{
			PIN_FLIST_FLD_SET(bb_info_flistp, MSO_FLD_NETWORK_NODE, network_node, ebufp);
		}
		if(network_amp)
		{
			PIN_FLIST_FLD_SET(bb_info_flistp, MSO_FLD_NETWORK_AMPLIFIER, network_amp, ebufp);
		}
		PIN_FLIST_FLD_SET(bb_info_flistp, MSO_FLD_NETWORK_ELEMENT, network_element, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "actual_task_flistp before conversion:",actual_task_flistp);

		PIN_FLIST_TO_STR(actual_task_flistp, &flistbuf, &flistlen, ebufp );
	}
	else
	{
		PIN_FLIST_TO_STR(copy_task_flistp, &flistbuf, &flistlen, ebufp );
	}

	nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
	if ( nota_buf == NULL )
	{
		pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51012" , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "couldn't allocate memory for BUF" , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
	}


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp1:", job_iflistp);
	nota_buf->flag   = 0;
	nota_buf->size   = flistlen - 2;
	nota_buf->offset = 0;
	nota_buf->data   = ( caddr_t)flistbuf;
	nota_buf->xbuf_file = ( char *) NULL;
	PIN_FLIST_FLD_SET(job_iflistp,PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );

	sprintf(msg, "Job create input flist for %s", job_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg ,job_iflistp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "job_iflistp2:", job_iflistp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
		PIN_ERRBUF_RESET(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "job_iflistp2:", job_iflistp);
                PIN_FLIST_FLD_SET(job_iflistp ,PIN_FLD_ERROR_CODE , "51013" , ebufp);
                PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "JOB Creation Error.Resubmit JOB" , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
        }

	
	PCM_OP(ctxp,PCM_OP_CREATE_OBJ ,0, job_iflistp , &job_oflistp , ebufp);


	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		sprintf(msg, "Error in creating the mso_mta job object for %s", job_name );
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR , msg);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "CMTS CHANGE job_iflist", job_iflistp);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Err before calling fm_mso_integ_create_job()", ebufp);
		
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , "job create output flist is " , job_oflistp);
	}
	

	if(nota_buf)
		free(nota_buf);

	PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
	PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
	PIN_FLIST_DESTROY_EX(&actual_task_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&copy_task_flistp, NULL);
	
	return;
}


extern void
fm_mso_get_bb_service_agreement_no(
	pcm_context_t	*ctxp,
	char			*agreement_no,
	pin_flist_t		**task_flistpp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*search_inflistp = NULL;
	pin_flist_t	*search_outflistp = NULL;
	pin_flist_t	*results_flistp = NULL;
	pin_flist_t	*services_flistp = NULL;
	pin_flist_t	*bb_info_flist = NULL;
	
	char		search_template[100] = " select X from /service/telco/broadband where F1 = V1 ";
	int		search_flags = 512;
	int64		db = 1;
	//int64		db = 0;
	poid_t		*device_pdp = NULL;
	poid_t		*service_poidp = NULL;
	poid_t		*pdp = NULL;
	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bb_service_agreement_no input flist", *task_flistpp);

	/*************
	 * search flist to search account poid
	 ************/
	
	
	
	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
	bb_info_flist = PIN_FLIST_SUBSTR_ADD(args_flistp,MSO_FLD_BB_INFO,ebufp);
	PIN_FLIST_FLD_SET(bb_info_flist, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bb_service_agreement_no search service input list", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bb_service_agreement_no search device output flist", search_outflistp);
	results_flistp = PIN_FLIST_ELEM_GET(search_outflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
	PIN_FLIST_FLD_COPY(results_flistp,PIN_FLD_POID, *task_flistpp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(results_flistp,PIN_FLD_ACCOUNT_OBJ, *task_flistpp, PIN_FLD_ACCOUNT_OBJ, ebufp);

	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);

	return;
}


static void
fm_mso_get_account_account_no(
	pcm_context_t	*ctxp,
	poid_t		*account_no,
	pin_flist_t	**task_flistpp,
	pin_errbuf_t	*ebufp)
{
        pin_flist_t             *srch_flistp = NULL;
        pin_flist_t             *srch_oflistp = NULL;
        pin_flist_t             *arg_flist = NULL;
        pin_flist_t             *result_flist = NULL;
        poid_t                  *srch_pdp = NULL;
        int32                   srch_flag = 512;
	poid_t			*pdp = NULL;
        int64                   db = 1;
	//int64			db = 0;
        char                    *template = "select X from /account  where F1 = V1 " ;


        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_account_account_no", ebufp);
				return;
        }
		
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_account_account_no :");
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, account_no);
	
	
        srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

        srch_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_NO, account_no, ebufp);

        result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bulkacnt_from_acntno search input list", srch_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
		        
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ", ebufp);
                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_acnt_from_acntno search billinfo output list", srch_oflistp);

        result_flist = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp );
        if (result_flist)
        {
		PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_POID,*task_flistpp, PIN_FLD_ACCOUNT_OBJ, ebufp);
        }
        PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
        return;
}


EXPORT_OP void
fm_mso_search_bb_device_object(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_errbuf_t	*ebufp)
{
		
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*search_inflistp = NULL;
	pin_flist_t	*search_outflistp = NULL;
	pin_flist_t	*results_flistp = NULL;
	pin_flist_t	*services_flistp = NULL;
	
	char		*device_id = NULL;
	char            checkstr[3];
	char		search_template[100] = " select X from /device where F1 = V1 ";
	int		search_flags = 768;
	int             agreementnoflag = 0;
	int64		db = 1;
	//int64		db = 0;
	poid_t		*device_pdp = NULL;
	poid_t		*service_pdp = NULL;
	poid_t		*account_pdp = NULL;
	poid_t		*pdp = NULL;
	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_bb_device_objectv input flist", i_flistp);

	device_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEVICE_ID , 1, ebufp);
	memset(checkstr, '\0', sizeof(checkstr));

        if(device_id){
            strncpy(checkstr, device_id, 2);
            if(strcmp(checkstr, "BB") == 0){
               agreementnoflag = 1;
            }
        }

	/*************
	 * search flist to search account poid
	 ************/
	// For ETHERNET, since MAC ID is optional, we send an agreement no for DEVICE_ID
	if(device_id && (agreementnoflag == 1)){
		fm_mso_get_bb_service_agreement_no(ctxp, device_id, &i_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_device_object search agreement flist", i_flistp);
	}
	else{
		search_inflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
		PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DEVICE_ID, device_id, ebufp);
		results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_bb_device_object search device input list", search_inflistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
		
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_bb_device_object search device output flist", search_outflistp);


		results_flistp = PIN_FLIST_ELEM_GET(search_outflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
		device_pdp = PIN_FLIST_FLD_GET(results_flistp , PIN_FLD_POID , 1, ebufp);

		services_flistp = PIN_FLIST_ELEM_GET(results_flistp, PIN_FLD_SERVICES , PIN_ELEMID_ANY, 0, ebufp);
		if(services_flistp)
		{
			service_pdp = PIN_FLIST_FLD_GET(services_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
			account_pdp = PIN_FLIST_FLD_GET(services_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
		}
		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_SERVICE_OBJ , service_pdp , ebufp);
		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_ACCOUNT_OBJ , account_pdp , ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_bb_device_object search return flist", i_flistp);

		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	}
	return;
}

static void
fm_mso_get_plan_poid(
	pcm_context_t	*ctxp,
	poid_t		*plan_name,
	pin_flist_t	**task_flistpp,
	pin_errbuf_t	*ebufp)
{
        pin_flist_t             *srch_flistp = NULL;
        pin_flist_t             *srch_oflistp = NULL;
        pin_flist_t             *arg_flist = NULL;
        pin_flist_t             *result_flist = NULL;
        poid_t                  *srch_pdp = NULL;
        int32                   srch_flag = 512;
	//int64			db = 0;
        int64                   db = 1;
	poid_t			*pdp = NULL;
        char                    *template = "select X from /plan  where F1 = V1 " ;


        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_plan_poid", ebufp);
				return;
        }
		
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_plan_poid :");
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, plan_name);

        srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	

        srch_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_NAME, plan_name, ebufp);

        result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_plan_poid search input list", srch_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
		        
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ", ebufp);
                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_plan_poid search billinfo output list", srch_oflistp);

        result_flist = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp );
        if (result_flist)
        {
		PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_POID,*task_flistpp, PIN_FLD_PLAN_OBJ, ebufp);
        }
        PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
        return;
}

static void
fm_mso_get_bb_billinfo(
        pcm_context_t           *ctxp,
	pin_flist_t	**task_flistpp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *srch_flistp = NULL;
        pin_flist_t             *srch_oflistp = NULL;
        pin_flist_t             *arg_flist = NULL;
        pin_flist_t             *result_flist = NULL;
        pin_flist_t             *nameinfo = NULL;

        poid_t                  *srch_pdp = NULL;
	poid_t                  *account_pdp = NULL;
        int32                   srch_flag = 512;
	char                    *bb_bilinfo = "BB";
	//int64			db = 0;
        int64                   db = 1;
	poid_t			*pdp = NULL;
        char                    *template = "select X from /billinfo  where F1 = V1 and F2 = V2 " ;


        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_bb_billinfo_from_acntno", ebufp);
				return;
        }
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bb_billinfo_from_acntno :");

        account_pdp = PIN_FLIST_FLD_GET(*task_flistpp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
        srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

        srch_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, account_pdp, ebufp);
	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_BILLINFO_ID, bb_bilinfo, ebufp);

        result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bulkacnt_from_acntno search input list", srch_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
		        
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ", ebufp);
                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_acnt_from_acntno search billinfo output list", srch_oflistp);

        result_flist = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp );	
	PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_POID,*task_flistpp, PIN_FLD_BILLINFO_OBJ, ebufp);
        
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
        CLEANUP:
        return;
}

static void
fm_mso_get_purchase_poid(
	pcm_context_t	*ctxp,
	poid_t		*plan_name,
	pin_flist_t	**task_flistpp,
	pin_errbuf_t	*ebufp)
{
        pin_flist_t             *srch_flistp = NULL;
        pin_flist_t             *srch_oflistp = NULL;
        pin_flist_t             *arg_flist = NULL;
        pin_flist_t             *result_flist = NULL;
		pin_flist_t				*plan_flist = NULL;
        poid_t                  *srch_pdp = NULL;
        int32                   srch_flag = 512;

        int64                   db = 1;
		int32				active_status = 1;

        char                    *template = "select X from /purchased_product 1 , /plan 2  where 1.F1 = V1 and 1.F2 = V2 and 1.F3 = V3 and 1.F4 = 2.F5 and 2.F6 = V6" ;


        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_purchase_poid", ebufp);
				return;
        }
		
	   PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_purchase_poid :");
       PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, plan_name);

        srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

        srch_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);


		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
        PIN_FLIST_FLD_COPY(*task_flistpp,PIN_FLD_ACCOUNT_OBJ,arg_flist, PIN_FLD_ACCOUNT_OBJ, ebufp);
   		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
        PIN_FLIST_FLD_COPY(*task_flistpp,PIN_FLD_SERVICE_OBJ,arg_flist, PIN_FLD_SERVICE_OBJ, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STATUS,&active_status, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp );
        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_PLAN_OBJ, NULL, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 5, ebufp );
        PIN_FLIST_FLD_SET(arg_flist,PIN_FLD_POID, NULL, ebufp);
		 arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 6, ebufp );
        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_NAME, plan_name, ebufp);

        result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

	    PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp);
		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_PLAN_OBJ, NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_purchase_poid search input list", srch_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
		        PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Search ", ebufp);
                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_purchase_poid search billinfo output list", srch_oflistp);

        result_flist = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp );
        if (result_flist)
        {
			PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_POID,*task_flistpp, PIN_FLD_OFFERING_OBJ, ebufp);
			plan_flist = PIN_FLIST_ELEM_GET(*task_flistpp , PIN_FLD_PLAN,0,1,ebufp);
			PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_PLAN_OBJ,plan_flist, PIN_FLD_PLAN_OBJ, ebufp);
        }
        PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
        return; 
}

static void
fm_mso_integ_bulk_create_account(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_flist_t     **ret_flistp,
        pin_errbuf_t    *ebufp)
{
        pin_flist_t     *task_flistp = NULL;
        pin_flist_t     *order_iflistp = NULL;
        pin_flist_t     *order_oflistp = NULL;
        pin_flist_t     *r_flistp = NULL;

        poid_t          *job_pdp        = NULL;
        poid_t          *order_pdp      = NULL;
        poid_t          *task_pdp       = NULL;
        poid_t          *user_id        = NULL;

        char            *program_name   = NULL;
        char            *task_no        = NULL;
        char            *order_id       = NULL;
        int32           *adjustment_type = NULL;
        int             elem_id = 0;
        int64           db = 0;
      //  int64           db = 1;
        int32           order_status_success = 0;
        int32           order_status_failure = 1;
        int32           *state_id = NULL;
        int32           status_success = 0;
        int32           status_fail = 2;
        pin_cookie_t    cookie = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_bulk_create_account input flist", i_flistp);

        order_pdp       = PIN_FLIST_FLD_GET(i_flistp , PIN_FLD_ORDER_OBJ , 1 , ebufp);
        db              = PIN_POID_GET_DB(order_pdp);
        task_no         = PIN_FLIST_FLD_GET(i_flistp , MSO_FLD_TASK_NAME , 1, ebufp);
        order_id        = PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_ORDER_ID , 1, ebufp);
        user_id         = PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_USERID , 1, ebufp);
        program_name    = PIN_FLIST_FLD_GET(i_flistp ,PIN_FLD_PROGRAM_NAME, 1 , ebufp);

        order_iflistp = PIN_FLIST_CREATE(ebufp);
        task_pdp = PIN_POID_CREATE(db , BULK_CREATE_ACCT_TASK_TYPE, -1 , ebufp);
        PIN_FLIST_FLD_PUT(order_iflistp , PIN_FLD_POID , task_pdp , ebufp);
        PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_OBJ , order_pdp , ebufp);
        PIN_FLIST_FLD_SET(order_iflistp , MSO_FLD_ORDER_TYPE , "Bulk Account Creation" , ebufp);
        PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_ORDER_ID , order_id , ebufp);
        PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_NUMBER , task_no , ebufp);
        PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_STATUS , &order_status_success , ebufp);
        PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_PROGRAM_NAME , program_name , ebufp);
        PIN_FLIST_FLD_SET(order_iflistp , PIN_FLD_USERID , user_id , ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create for bulk account creation input flist is " , order_iflistp);
        PCM_OP(ctxp,PCM_OP_CREATE_OBJ,0 , order_iflistp , &order_oflistp , ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the task object " );
                PIN_FLIST_DESTROY_EX(&order_oflistp , NULL);
                r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, order_pdp, ebufp );
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &order_status_failure, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_bulk_create_account task object failed", ebufp);
                //*ret_flistp = r_flistp;
                //return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " Task create output for bulk account creation flist is " , order_oflistp);

        if(order_oflistp)
        {
                task_pdp = PIN_FLIST_FLD_GET(order_oflistp , PIN_FLD_POID , 1, ebufp);

                while ((task_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp,MSO_FLD_TASK,&elem_id, 1, &cookie, ebufp))!= NULL)
                {
                        PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_PROGRAM_NAME,program_name , ebufp);
                        PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_USERID,user_id , ebufp);
						PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_POID, task_pdp , ebufp);

                        fm_mso_integ_blk_create_acct_job(ctxp, task_no, task_pdp, task_flistp , ebufp);
                        if (PIN_ERRBUF_IS_ERR(ebufp))
                        {
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in creating the bulk account creation job object ");
                                PIN_ERRBUF_CLEAR(ebufp);
                        }
                }
        }
        else
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Task object failed ");
        }
        *ret_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID , order_pdp, ebufp);
        PIN_FLIST_FLD_SET(*ret_flistp,PIN_FLD_ORDER_ID , order_id , ebufp);
        PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS , &order_status_success , ebufp);
        task_flistp = PIN_FLIST_ELEM_ADD(*ret_flistp,MSO_FLD_TASK,0, ebufp);
        PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
        PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ORDER_ID , task_no , ebufp);

        PIN_FLIST_DESTROY_EX(&order_iflistp, NULL);
        PIN_FLIST_DESTROY_EX(&order_oflistp, NULL);
        return ;
}

EXPORT_OP void
fm_mso_integ_blk_create_acct_job(
	pcm_context_t	*ctxp,
	char			*task_no,
	poid_t			*task_pdp,
	pin_flist_t		*task_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*job_oflistp = NULL;
	pin_flist_t	*job_iflistp = NULL;
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*search_inflistp = NULL;
	pin_flist_t	*search_outflistp = NULL;
	pin_flist_t	*results_flistp = NULL;
	pin_flist_t	*services_flistp = NULL;

	poid_t		*job_pdp	= NULL;
	poid_t		*lco_account_pdp = NULL;
	poid_t		*device_pdp	 = NULL;

	char		*account_no	= NULL;
	char		*stb_no	= NULL;
	char		*vc_no	= NULL;
	char		*zip		= NULL;
	char		*network_node	= NULL;

	//int64		db = 1;
	int64		db = 0;
	int32		status_success = 0;
	int32		status_fail = 2;
	int32		*oap_status = NULL;
	int32		oap_status_fail =2;
	int32		oap_status_success = 0;

	pin_buf_t	*nota_buf = NULL;
	char		*flistbuf = NULL;
	int			flistlen =0;

	int32		opcode = MSO_OP_CUST_BLK_REGISTER_CUSTOMER ;
	int32		flg_res_arr_passed = 1;
	//char		region_key[20];

	char		msg[256];
	char		*dev_type = NULL;
	int			search_flags = 768;
	int			devices_validation = 0; //Flag for Device Validation : 0 - Failed, 1 - Success
	int			*dev_status = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_blk_create_acct_job input flist", task_flistp);
	db = PIN_POID_GET_DB(task_pdp);

	PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
	nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
	if ( nota_buf == NULL )
	{
		pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
		return;
	}	
	
	job_iflistp = PIN_FLIST_CREATE(ebufp);
	job_pdp = PIN_POID_CREATE(db, BULK_CREATE_ACCT_JOB_TYPE, -1 ,ebufp);
	PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);

	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
	PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_USERID,job_iflistp, PIN_FLD_USERID, ebufp);
	PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_PROGRAM_NAME,job_iflistp, PIN_FLD_PROGRAM_NAME, ebufp);

	nota_buf->flag   = 0;
	nota_buf->size   = flistlen - 2;
	nota_buf->offset = 0;
	nota_buf->data   = ( caddr_t)flistbuf;
	nota_buf->xbuf_file = ( char *) NULL;
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );


	/* Read stb_no, vc_no and Account no */
	stb_no =  PIN_FLIST_FLD_GET(task_flistp, MSO_FLD_STB_ID , 1 , ebufp);
	account_no =  PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_ACCOUNT_NO , 1 , ebufp);
	/* Read and drop OAP status */
	oap_status =  PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_STATUS , 1 , ebufp);

	if((oap_status) && (*oap_status == oap_status_fail))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR , "Input Validation failed @ OAP");
		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
		goto CLEANUP;
	}

//  Validating STB No
	if (stb_no)
	{
		PIN_FLIST_FLD_COPY(task_flistp, MSO_FLD_STB_ID, task_flistp, PIN_FLD_DEVICE_ID, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Prior to fm_mso_search_device_detail for STB", task_flistp);
		fm_mso_search_device_detail(ctxp, task_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "After fm_mso_search_device_detail for STB", task_flistp);
		device_pdp = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_OBJ, 1, ebufp);
	}
	else
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "STB Not Passed in Input", ebufp);
		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51040" , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, "STB Not Passed in Input" , ebufp);
		PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0 ,job_iflistp , &job_oflistp , ebufp);
		goto CLEANUP;
	}

	if((PIN_POID_IS_NULL(device_pdp)))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "STB Device not found in DB", ebufp);
		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51040" , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, "STB Device not found in DB" , ebufp);
		PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0 ,job_iflistp , &job_oflistp , ebufp);
		goto CLEANUP;
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "STB Device object found in DB");
		// Validation of New Device (State Id = 1)
		dev_status = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_STATE_ID, 1, ebufp);
		if (dev_status && (*dev_status != 1) && (*dev_status != 9)) // VERIMATRIX - added Repaired device state
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR , "STB Device State Mismatch", ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51040" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, "STB Device State Mismatch" , ebufp);
			PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0 ,job_iflistp , &job_oflistp , ebufp);
			goto CLEANUP;
		}

		// Getting STB Manufacturer to identify NDS / CISCO . For CISCO no need to validate the VC No
		dev_type = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_MANUFACTURER, 1, ebufp);
		if (!dev_type) 
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR , "STB Device Manafacturer Error", ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51040" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, "STB Device Manafacturer Error" , ebufp);
			PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0 ,job_iflistp , &job_oflistp , ebufp);
			goto CLEANUP;
		}

		if (strstr(dev_type, "NDS"))
		{
			vc_no =  PIN_FLIST_FLD_GET(task_flistp, MSO_FLD_VC_ID , 1 , ebufp);
			//  Validating VC No
			if (vc_no)
			{
				PIN_FLIST_FLD_COPY(task_flistp, MSO_FLD_VC_ID, task_flistp, PIN_FLD_DEVICE_ID, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Before fm_mso_search_device_detail for VC", task_flistp);
				fm_mso_search_device_detail(ctxp,task_flistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "After fm_mso_search_device_detail for VC", task_flistp);
				device_pdp = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_OBJ, 1, ebufp);
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR , "VC Not Passed in Input" );
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51040" , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, "VC Not Passed in Input" , ebufp);
				PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0 ,job_iflistp , &job_oflistp , ebufp);
				goto CLEANUP;
			}

			if (PIN_POID_IS_NULL(device_pdp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR , "VC Device Id not found in DB" );
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51040" , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, "VC Device Id not found in DB" , ebufp);
				PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0 ,job_iflistp , &job_oflistp , ebufp);
				goto CLEANUP;
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "VC Device object Found" );
				dev_status = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_STATE_ID, 1, ebufp);
				if (dev_status && (*dev_status != 1) && (*dev_status != 9)) // VERIMATRIX - added Repaired device state
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR , "VC Device State Mismatch" );
					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51040" , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, "VC Device State Mismatch" , ebufp);
					PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0 ,job_iflistp , &job_oflistp , ebufp);
					goto CLEANUP;
				}
			}
				// VC Validation is also successful for NDS
				//devices_validation = 1;
			
		}
/*		else
		{
			// CISCO device. Only STB validation is sufficient
			devices_validation = 1;
		}*/
	}
	PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_success , ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MTA JOB Creation input flist", job_iflistp);
	PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0 ,job_iflistp , &job_oflistp , ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MTA JOB Creation output flist", job_oflistp);

CLEANUP:
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the job object for bulk account creation " );
		PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
		PIN_ERRBUF_CLEAR(ebufp);
	}
	PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
	return;

}

void
fm_mso_search_device_detail(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_errbuf_t	*ebufp)
{
		
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*search_inflistp = NULL;
	pin_flist_t	*search_outflistp = NULL;
	pin_flist_t	*results_flistp = NULL;
	
	char		*device_id = NULL;		
	char		search_template[100] = " select X from /device where F1 = V1 ";
	char		*manufacturer = NULL;
	int			state_id = 0;
	int			search_flags = 768;
	int64		db = 1;
	//int64		db = 0;
	poid_t		*pdp = NULL;
	poid_t		*device_pdp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_device_detail input flist", i_flistp);

	device_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEVICE_ID , 1, ebufp);

	/*************
	 * search flist to search device poid
	 ************/
	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DEVICE_ID, device_id, ebufp);
	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_device_detail search device input list", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_device_detail search device output flist", search_outflistp);

	if(PIN_FLIST_ELEM_COUNT(search_outflistp , PIN_FLD_RESULTS , ebufp) > 0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Device object found");

		results_flistp = PIN_FLIST_ELEM_GET(search_outflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		device_pdp = PIN_FLIST_FLD_GET(results_flistp , PIN_FLD_POID , 1, ebufp);
//		manufacturer = (char *)PIN_FLIST_FLD_GET(results_flistp , PIN_FLD_MANUFACTURER , 1, ebufp);

		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_RESET(ebufp);
		}

		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_DEVICE_OBJ , device_pdp , ebufp);
		PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_MANUFACTURER, i_flistp, PIN_FLD_MANUFACTURER , ebufp);
		PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_STATE_ID, i_flistp, PIN_FLD_STATE_ID , ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_device_detail search return flist", i_flistp);
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Device object not found");
		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_DEVICE_OBJ , NULL , ebufp);
	}

	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
}

EXPORT_OP void
fm_mso_integ_blk_create_acct_update_job(
	pcm_context_t	*ctxp,
	char			*task_no,
	poid_t			*task_pdp,
	pin_flist_t		*task_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*job_oflistp = NULL;
	pin_flist_t	*job_iflistp = NULL;
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*search_inflistp = NULL;
	pin_flist_t	*search_outflistp = NULL;
	pin_flist_t	*results_flistp = NULL;
	pin_flist_t	*services_flistp = NULL;

	poid_t		*job_pdp	= NULL;
	poid_t		*lco_account_pdp = NULL;
	poid_t		*device_pdp	 = NULL;

	char		*account_no	= NULL;
	char		*stb_no	= NULL;
	char		*vc_no	= NULL;
	char		*zip		= NULL;
	char		*network_node	= NULL;

	//int64		db = 1;
	int64		db = 0;
	int32		status_success = 0;
	int32		status_fail = 2;
	int32		*oap_status = NULL;
	int32		oap_status_fail =2;
	int32		oap_status_success = 0;

	pin_buf_t	*nota_buf = NULL;
	char		*flistbuf = NULL;
	int			flistlen =0;

	int32		opcode = MSO_OP_CUST_BLK_REGISTER_CUSTOMER ;
	int32		flg_res_arr_passed = 1;
	//char		region_key[20];

	char		msg[256];
	char		*dev_type = NULL;
	int			search_flags = 768;
	int			devices_validation = 0; //Flag for Device Validation : 0 - Failed, 1 - Success
	int			*dev_status = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_blk_create_acct_job input flist", task_flistp);
	db = PIN_POID_GET_DB(task_pdp);

	PIN_FLIST_TO_STR(task_flistp, &flistbuf, &flistlen, ebufp );
	nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
	if ( nota_buf == NULL )
	{
		pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
		return;
	}	
	
	job_iflistp = PIN_FLIST_CREATE(ebufp);
//	job_pdp = PIN_POID_CREATE(db, "/mso_mta_job/bulk_create_account", -1 ,ebufp);
//	PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, job_pdp , ebufp);
	PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_POID, job_iflistp, PIN_FLD_POID, ebufp);

	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ , task_pdp , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE , &opcode , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_ID , task_no , ebufp);
	PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_USERID,job_iflistp, PIN_FLD_USERID, ebufp);
	PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_PROGRAM_NAME,job_iflistp, PIN_FLD_PROGRAM_NAME, ebufp);

	nota_buf->flag   = 0;
	nota_buf->size   = flistlen - 2;
	nota_buf->offset = 0;
	nota_buf->data   = ( caddr_t)flistbuf;
	nota_buf->xbuf_file = ( char *) NULL;
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );


	/* Read stb_no, vc_no and Account no */
	stb_no =  PIN_FLIST_FLD_GET(task_flistp, MSO_FLD_STB_ID , 1 , ebufp);
	account_no =  PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_ACCOUNT_NO , 1 , ebufp);
	/* Read and drop OAP status */
	oap_status =  PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_STATUS , 1 , ebufp);

	if((oap_status) && (*oap_status == oap_status_fail))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR , "Input Validation failed @ OAP");
		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_CODE,job_iflistp ,PIN_FLD_ERROR_CODE, ebufp);
		PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ERROR_DESCR,job_iflistp ,PIN_FLD_ERROR_DESCR, ebufp);
		goto CLEANUP;
	}

//  Validating STB No
	if (stb_no)
	{
		PIN_FLIST_FLD_COPY(task_flistp, MSO_FLD_STB_ID, task_flistp, PIN_FLD_DEVICE_ID, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Prior to fm_mso_search_device_detail for STB", task_flistp);
		fm_mso_search_device_detail(ctxp, task_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "After fm_mso_search_device_detail for STB", task_flistp);
		device_pdp = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_OBJ, 1, ebufp);
	}
	else
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "STB Not Passed in Input", ebufp);
		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51040" , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, "STB Not Passed in Input" , ebufp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0 ,job_iflistp , &job_oflistp , ebufp);
		goto CLEANUP;
	}

	if((PIN_POID_IS_NULL(device_pdp)))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "STB Device not found in DB", ebufp);
		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51040" , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, "STB Device not found in DB" , ebufp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0 ,job_iflistp , &job_oflistp , ebufp);
		goto CLEANUP;
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "STB Device object found in DB");
		// Validation of New Device (State Id = 1)
		dev_status = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_STATE_ID, 1, ebufp);
		if (dev_status && (*dev_status != 1) && (*dev_status != 9)) // VERIMATRIX - added Repaired device state
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR , "STB Device State Mismatch", ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51040" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, "STB Device State Mismatch" , ebufp);
			PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0 ,job_iflistp , &job_oflistp , ebufp);
			goto CLEANUP;
		}

		// Getting STB Manufacturer to identify NDS / CISCO . For CISCO no need to validate the VC No
		dev_type = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_MANUFACTURER, 1, ebufp);
		if (!dev_type) 
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR , "STB Device Manafacturer Error", ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51040" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, "STB Device Manafacturer Error" , ebufp);
			PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0 ,job_iflistp , &job_oflistp , ebufp);
			goto CLEANUP;
		}

		if (strstr(dev_type, "NDS"))
		{
			vc_no =  PIN_FLIST_FLD_GET(task_flistp, MSO_FLD_VC_ID , 1 , ebufp);
			//  Validating VC No
			if (vc_no)
			{
				PIN_FLIST_FLD_COPY(task_flistp, MSO_FLD_VC_ID, task_flistp, PIN_FLD_DEVICE_ID, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Before fm_mso_search_device_detail for VC", task_flistp);
				fm_mso_search_device_detail(ctxp,task_flistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "After fm_mso_search_device_detail for VC", task_flistp);
				device_pdp = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_OBJ, 1, ebufp);
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR , "VC Not Passed in Input" );
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51040" , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, "VC Not Passed in Input" , ebufp);
				PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0 ,job_iflistp , &job_oflistp , ebufp);
				goto CLEANUP;
			}

			if (PIN_POID_IS_NULL(device_pdp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR , "VC Device Id not found in DB" );
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51040" , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, "VC Device Id not found in DB" , ebufp);
				PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0 ,job_iflistp , &job_oflistp , ebufp);
				goto CLEANUP;
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "VC Device object Found" );
				dev_status = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_STATE_ID, 1, ebufp);
				if (dev_status && (*dev_status != 1) && (*dev_status != 9)) // VERIMATRIX - added Repaired device state
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR , "VC Device State Mismatch" );
					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51040" , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, "VC Device State Mismatch" , ebufp);
					PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0 ,job_iflistp , &job_oflistp , ebufp);
					goto CLEANUP;
				}
			}
				// VC Validation is also successful for NDS
				//devices_validation = 1;
			
		}
/*		else
		{
			// CISCO device. Only STB validation is sufficient
			devices_validation = 1;
		}*/
	}
	PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_success , ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MTA JOB Creation input flist", job_iflistp);
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0 ,job_iflistp , &job_oflistp , ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MTA JOB Creation output flist", job_oflistp);

CLEANUP:
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the job object for bulk account creation " );
		PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
		PIN_ERRBUF_CLEAR(ebufp);
	}
	PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
	return;
}

void fm_mso_bulk_update_gst_info(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*task_flistp = NULL;
	pin_flist_t		*order_iflistp = NULL;
	pin_flist_t		*order_oflistp = NULL;
	pin_flist_t		*r_flistp = NULL;

	poid_t			*order_pdp = NULL;
	poid_t			*task_pdp = NULL;
	poid_t			*user_id = NULL;

	char			*program_name = NULL;
	char			*task_no = NULL;
	char			*order_id = NULL;
	int64			db = 1;
	int32			order_status_success = 0;
	int32			order_status_failure = 1;

	pin_cookie_t	cookie = NULL;
	int32			elem_id = 0;
	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bulk_update_gst_info input flist", i_flistp);

	order_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ORDER_OBJ, 1, ebufp);
	db = PIN_POID_GET_DB(order_pdp);
	task_no = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_TASK_NAME, 1, ebufp);
	order_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ORDER_ID, 1, ebufp);
	user_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 1, ebufp);
	program_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);

	order_iflistp = PIN_FLIST_CREATE(ebufp);
	task_pdp = PIN_POID_CREATE(db, "/mso_task/bulk_update_gst_info", -1, ebufp);
	PIN_FLIST_FLD_PUT(order_iflistp, PIN_FLD_POID, task_pdp, ebufp);
	PIN_FLIST_FLD_SET(order_iflistp, MSO_FLD_ORDER_OBJ, order_pdp, ebufp);
	PIN_FLIST_FLD_SET(order_iflistp, MSO_FLD_ORDER_TYPE, "Bulk Update GST Info", ebufp);
	PIN_FLIST_FLD_SET(order_iflistp, PIN_FLD_ORDER_ID, order_id, ebufp);
	PIN_FLIST_FLD_SET(order_iflistp, PIN_FLD_NUMBER, task_no, ebufp);
	PIN_FLIST_FLD_SET(order_iflistp, PIN_FLD_STATUS, &order_status_success, ebufp);
	PIN_FLIST_FLD_SET(order_iflistp, PIN_FLD_PROGRAM_NAME, program_name, ebufp);
	PIN_FLIST_FLD_SET(order_iflistp, PIN_FLD_USERID, user_id, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Task create for bulk update gst info input flist is", order_iflistp);
	PCM_OP(ctxp,PCM_OP_CREATE_OBJ, 0, order_iflistp, &order_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error in creating the task object" );
		PIN_FLIST_DESTROY_EX(&order_oflistp, NULL);
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, order_pdp, ebufp );
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &order_status_failure, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "50001", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "fm_mso_bulk_update_gst_info task object failed", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Task create output for bulk update gst info flist is", order_oflistp);

	if(order_oflistp)
	{
		task_pdp = PIN_FLIST_FLD_GET(order_oflistp, PIN_FLD_POID, 1, ebufp);
		while ((task_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, MSO_FLD_TASK, &elem_id, 1, &cookie, ebufp))!= NULL)
		{
			fm_mso_create_update_gst_info_jobs(ctxp, task_no, task_pdp, user_id, program_name, task_flistp , ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error in creating the bulk update gst info job object");
				PIN_ERRBUF_CLEAR(ebufp);
			}
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error in creating the task object" );
		PIN_FLIST_DESTROY_EX(&order_oflistp, NULL);
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, order_pdp, ebufp );
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &order_status_failure, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "50001", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "fm_mso_bulk_update_gst_info task object failed", ebufp);
		goto CLEANUP;
	}
	*r_flistpp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, order_pdp, ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ORDER_ID, order_id, ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &order_status_success, ebufp);
	task_flistp = PIN_FLIST_ELEM_ADD(*r_flistpp, MSO_FLD_TASK, 0, ebufp);
	PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ORDER_OBJ, task_pdp, ebufp);
	PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ORDER_ID, task_no, ebufp);

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&order_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&order_oflistp, NULL);
	return ;
}

void fm_mso_create_update_gst_info_jobs(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	poid_t		*user_id,
	char		*program_name,
	pin_flist_t	*in_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*job_iflistp = NULL;
	pin_flist_t		*job_oflistp = NULL;
	pin_flist_t		*acct_details = NULL;

	poid_t			*acct_pdp = NULL;
	char			*flistbuf       = NULL;
	pin_buf_t		*nota_buf       = NULL;
	int32			initial_status  = 0;
	int32			flistlen = 0;
	int32			opcode = MSO_OP_CUST_UPDATE_GST_INFO;
	void			*vp = NULL;
	int64			db = 1;
	char			*error_descr = "Customer not found";
	char			*error_code = "50001";
	char			*acct_no = NULL;
	
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
	}

	db = PIN_POID_GET_DB(task_pdp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "GST Info job create input flist: ", in_flistp);

	acct_no = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	if(acct_no)
		fm_mso_get_acnt_from_acntno(ctxp, acct_no, &acct_details, ebufp);

        if(PIN_ERRBUF_IS_ERR(ebufp))
        {
       		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in searching customer" );
       		PIN_ERRBUF_CLEAR(ebufp);
		initial_status = 2;
        }
	if (acct_details)
	{
		acct_pdp = PIN_FLIST_FLD_GET(acct_details, PIN_FLD_POID, 0, ebufp);	
	}
	else
	{
		initial_status = 2;
	}

	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_PROGRAM_NAME, program_name, ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_USERID, user_id, ebufp);
	PIN_FLIST_FLD_PUT(in_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/account", 1, ebufp), ebufp);
	
	PIN_FLIST_TO_STR(in_flistp, &flistbuf, &flistlen, ebufp );
	nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
	if ( nota_buf == NULL )
	{
		pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
		goto CLEANUP;
	}
	nota_buf->flag = 0;
	nota_buf->size = flistlen - 2;
	nota_buf->offset = 0;
	nota_buf->data = ( caddr_t)flistbuf;
	nota_buf->xbuf_file = ( char *) NULL;

	job_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/mso_mta_job/bulk_update_gst_info", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ, task_pdp, ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID, task_no, ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE, &opcode, ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &initial_status , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_USERID, user_id, ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_PROGRAM_NAME, program_name, ebufp);
	if(initial_status == 2)
	{
		PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_ACCOUNT_OBJ, PIN_POID_CREATE(db, "/account", 1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, error_descr, ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_CODE, error_code, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Job create flist", job_iflistp);
	PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, job_iflistp, &job_oflistp, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the job object for bulk gst info" );
		PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
		PIN_ERRBUF_CLEAR(ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Job create output flist", job_oflistp);
	CLEANUP :
	PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
	PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
	return;
}


void fm_mso_bulk_modify_gst_info(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*task_flistp = NULL;
	pin_flist_t		*order_iflistp = NULL;
	pin_flist_t		*order_oflistp = NULL;
	pin_flist_t		*r_flistp = NULL;

	poid_t			*order_pdp = NULL;
	poid_t			*task_pdp = NULL;
	poid_t			*user_id = NULL;

	char			*program_name = NULL;
	char			*task_no = NULL;
	char			*order_id = NULL;
	int64			db = 1;
	int32			order_status_success = 0;
	int32			order_status_failure = 1;

	pin_cookie_t	cookie = NULL;
	int32			elem_id = 0;
	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bulk_modify_gst_info input flist", i_flistp);

	order_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ORDER_OBJ, 1, ebufp);
	db = PIN_POID_GET_DB(order_pdp);
	task_no = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_TASK_NAME, 1, ebufp);
	order_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ORDER_ID, 1, ebufp);
	user_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 1, ebufp);
	program_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);

	order_iflistp = PIN_FLIST_CREATE(ebufp);
	task_pdp = PIN_POID_CREATE(db, "/mso_task/bulk_modify_gst_info", -1, ebufp);
	PIN_FLIST_FLD_PUT(order_iflistp, PIN_FLD_POID, task_pdp, ebufp);
	PIN_FLIST_FLD_SET(order_iflistp, MSO_FLD_ORDER_OBJ, order_pdp, ebufp);
	PIN_FLIST_FLD_SET(order_iflistp, MSO_FLD_ORDER_TYPE, "Bulk Update GST Info", ebufp);
	PIN_FLIST_FLD_SET(order_iflistp, PIN_FLD_ORDER_ID, order_id, ebufp);
	PIN_FLIST_FLD_SET(order_iflistp, PIN_FLD_NUMBER, task_no, ebufp);
	PIN_FLIST_FLD_SET(order_iflistp, PIN_FLD_STATUS, &order_status_success, ebufp);
	PIN_FLIST_FLD_SET(order_iflistp, PIN_FLD_PROGRAM_NAME, program_name, ebufp);
	PIN_FLIST_FLD_SET(order_iflistp, PIN_FLD_USERID, user_id, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Task create for bulk modify gst info input flist is", order_iflistp);
	PCM_OP(ctxp,PCM_OP_CREATE_OBJ, 0, order_iflistp, &order_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error in creating the task object" );
		PIN_FLIST_DESTROY_EX(&order_oflistp, NULL);
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, order_pdp, ebufp );
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &order_status_failure, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "50001", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "fm_mso_bulk_modify_gst_info task object failed", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Task create output for bulk modify gst info flist is", order_oflistp);

	if(order_oflistp)
	{
		task_pdp = PIN_FLIST_FLD_GET(order_oflistp, PIN_FLD_POID, 1, ebufp);
		while ((task_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, MSO_FLD_TASK, &elem_id, 1, &cookie, ebufp))!= NULL)
		{
			fm_mso_create_modify_gst_info_jobs(ctxp, task_no, task_pdp, user_id, program_name, task_flistp , ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error in creating the bulk modify gst info job object");
				PIN_ERRBUF_CLEAR(ebufp);
			}
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error in creating the task object" );
		PIN_FLIST_DESTROY_EX(&order_oflistp, NULL);
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, order_pdp, ebufp );
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &order_status_failure, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "50001", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "fm_mso_bulk_modify_gst_info task object failed", ebufp);
		goto CLEANUP;
	}
	*r_flistpp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, order_pdp, ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ORDER_ID, order_id, ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &order_status_success, ebufp);
	task_flistp = PIN_FLIST_ELEM_ADD(*r_flistpp, MSO_FLD_TASK, 0, ebufp);
	PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ORDER_OBJ, task_pdp, ebufp);
	PIN_FLIST_FLD_SET(task_flistp, PIN_FLD_ORDER_ID, task_no, ebufp);

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&order_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&order_oflistp, NULL);
	return ;
}

void fm_mso_create_modify_gst_info_jobs(
	pcm_context_t	*ctxp,
	char		*task_no,
	poid_t		*task_pdp,
	poid_t		*user_id,
	char		*program_name,
	pin_flist_t	*in_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*job_iflistp = NULL;
	pin_flist_t		*job_oflistp = NULL;
	pin_flist_t		*acct_details = NULL;

	poid_t			*acct_pdp = NULL;
	char			*flistbuf       = NULL;
	pin_buf_t		*nota_buf       = NULL;
	int32			initial_status  = 0;
	int32			flistlen = 0;
	int32			opcode = MSO_OP_CUST_MODIFY_CUSTOMER;
	void			*vp = NULL;
	int64			db = 1;
	char			*error_descr = "Customer not found";
	char			*error_code = "50001";
	char			*acct_no = NULL;
	
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
	}

	db = PIN_POID_GET_DB(task_pdp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "GST Info job create input flist: ", in_flistp);

	acct_no = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	if(acct_no)
		fm_mso_get_acnt_from_acntno(ctxp, acct_no, &acct_details, ebufp);

        if(PIN_ERRBUF_IS_ERR(ebufp))
        {
       		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in searching customer" );
       		PIN_ERRBUF_CLEAR(ebufp);
			initial_status = 2;
        }
	if (acct_details)
	{
		acct_pdp = PIN_FLIST_FLD_GET(acct_details, PIN_FLD_POID, 0, ebufp);	
	}
	else
	{
		initial_status = 2;
	}

	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_PROGRAM_NAME, program_name, ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_USERID, user_id, ebufp);
	if(initial_status == 2)
	{
		PIN_FLIST_FLD_PUT(in_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/account", 1, ebufp), ebufp);
		PIN_FLIST_FLD_PUT(in_flistp, PIN_FLD_ACCOUNT_OBJ, PIN_POID_CREATE(db, "/account", 1, ebufp), ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, acct_pdp, ebufp);
		PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	}
	
	PIN_FLIST_TO_STR(in_flistp, &flistbuf, &flistlen, ebufp );
	nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
	if ( nota_buf == NULL )
	{
		pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate memory for nota_buf", ebufp );
		goto CLEANUP;
	}
	nota_buf->flag = 0;
	nota_buf->size = flistlen - 2;
	nota_buf->offset = 0;
	nota_buf->data = ( caddr_t)flistbuf;
	nota_buf->xbuf_file = ( char *) NULL;

	job_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/mso_mta_job/bulk_modify_gst_info", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_OBJ, task_pdp, ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ORDER_ID, task_no, ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_OPCODE, &opcode, ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &initial_status , ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_USERID, user_id, ebufp);
	PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_PROGRAM_NAME, program_name, ebufp);
	if(initial_status == 2)
	{
		PIN_FLIST_FLD_PUT(job_iflistp, PIN_FLD_ACCOUNT_OBJ, PIN_POID_CREATE(db, "/account", 1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, error_descr, ebufp);
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_CODE, error_code, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Job create flist", job_iflistp);
	PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, job_iflistp, &job_oflistp, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the job object for bulk gst info" );
		PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
		PIN_ERRBUF_CLEAR(ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Job create output flist", job_oflistp);
	CLEANUP :
	PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
	PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
	return;
}

