/*******************************************************************
 * File:	fm_mso_cust_activate_bb_service.c
 * Opcode:	MSO_OP_CUST_ACTIVATE_BB_SERVICE 
 * Owner:	Pawan
 * Created:	21-JULY-2014
 * Description: Activates broadband service for an account and
 *				purchases the plans stored in /mso_plans_activation
 *
 * Modification History:
 * Modified By: 
 * Date: 
 * Modification details:  Updated PLan type, priority and transaction handling 
  
 * Modified By: Jyothirmayi Kachiraju
 * Date: 11th Dec 2019
 * Modification details: Removal of bad code that writes line numbers in the /temp/stackp in the 
 * calc_static_ip_charge and apply_static_ip_charge functions.
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_activate_bb_service.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "ops/act.h"
#include "ops/cust.h"
#include "ops/bill.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "pin_cust.h"
#include "pin_pymt.h"
#include "pin_inv.h"
#include "mso_ops_flds.h"
#include "mso_prov.h"
#include "mso_ntf.h"
#include "mso_cust.h"
#include "mso_lifecycle_id.h"

#define CORPORATE_PARENT 1
#define CORPORATE_CHILD  2

#define SERVICE_BB    "/service/telco/broadband"
#define SRVC_CATV  "/service/catv"

#define READY_TO_USE 1
#define PREACTIVATED 6
#define REPAIRED_TO_USE 9

#define MSO_FLAG_SRCH_BY_ID 1
#define MSO_FLAG_SRCH_BY_PIN 2
#define MSO_FLAG_SRCH_BY_ACCOUNT 3
#define MSO_FLAG_SRCH_BY_JV 4
#define MSO_FLAG_SRCH_BY_DT 5
#define MSO_FLAG_SRCH_BY_SDT 6
#define MSO_FLAG_SRCH_BY_LCO 7
#define MSO_FLAG_SRCH_LCO_OF_SUBSCRIBER 8

#define WHOLESALE "/profile/wholesale"
#define CUSTOMER_CARE "/profile/customer_care"

#define SUCCESS 0
#define FAILED 1
#define TYPE_FUP 2
#define CURRENCY_INR 356

/**************************************
* External Functions start
***************************************/

/**************************************
* Local Functions start
***************************************/
static void
fm_mso_validate_plan_activation(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**mso_act_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static int32
fm_mso_get_services_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**service_array,
	char			*ser_type,
	pin_errbuf_t		*ebufp);
	
static void
fm_mso_set_bb_service_status(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);
	
extern void
fm_mso_purchase_bb_plans(
	cm_nap_connection_t     *connp,
	pin_flist_t		*mso_act_flistp,
        char                    *prov_tag,
        int                     flags,
	int			grace_flag,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);
	
extern void
fm_mso_create_purchased_plan_object(
	pcm_context_t		*ctxp,
	pin_flist_t		*pur_plan_flistp,
	pin_flist_t		*bb_bp_flistp,
	pin_flist_t		*hardware_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

/*
PIN_IMPORT void
fm_mso_get_account_info(
        pcm_context_t           *ctxp,
        poid_t                  *acnt_pdp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

PIN_IMPORT void
fm_mso_get_acnt_from_acntno(
        pcm_context_t           *ctxp,
        char                    *acnt_no,
        pin_flist_t             **acnt_details,
        pin_errbuf_t            *ebufp);*/

static void
fm_mso_create_device_deposit(
	pcm_context_t		*ctxp,
	pin_flist_t		*hardware_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);
	
static int
fm_mso_set_bb_prov_status(
	pcm_context_t	*ctxp,
	poid_t			*ser_pdp,
	int				status,
	pin_errbuf_t		*ebufp);

static void
fm_mso_call_prov_action(
        pcm_context_t   *ctxp,
        pin_flist_t             *in_flistp,
	poid_t		*subs_plan_pdp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);
		
static void
fm_mso_cust_bb_set_cr_limit(
	pcm_context_t		*ctxp,
	pin_flist_t		*pur_plan_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_set_fup_status(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	poid_t			*subs_plan_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);
	
static void
fm_mso_pur_deal_calc_only(
	cm_nap_connection_t     *connp,	
	pin_flist_t			*pur_in_flistp, 
	pin_flist_t			**calc_out_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_get_ncr_sub_bal(
	pcm_context_t 		*ctxp, 
	pin_flist_t		*in_flistp,
	pin_flist_t		*rr_flistpp,
	int32			found, 
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

extern
void fm_mso_create_lifecycle_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	int32			action,
	pin_errbuf_t		*ebufp);

static void 
fm_mso_cust_activate_amc_plan(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		*mso_pp_flistp,
	pin_errbuf_t		*ebufp );

int32 get_bill_when_from_service(
                pcm_context_t           *ctxp,
                poid_t                  *svc_pdp,
                pin_errbuf_t            *ebufp);

/**************************************
* Local Functions end
***************************************/

extern void
fm_mso_commission_error_processing (
        pcm_context_t           *ctxp,
        int                     error_no,
        char                    *error_in,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);
		
extern void
fm_mso_prov_update_product_status(
	pcm_context_t		*ctxp, 
	poid_t			*acct_pdp, 
	poid_t			*service_obj,
	poid_t			*offering_pdp,
	pin_errbuf_t		*ebufp);

extern int
fm_mso_update_ser_prov_status(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flistp,
	int 				prov_status,
	pin_errbuf_t            *ebufp);
	
extern int
fm_mso_update_mso_purplan_status(
	pcm_context_t           *ctxp,
	poid_t                  *mso_purplan_obj,
	int						status,
	pin_errbuf_t            *ebufp);

static void add_plans_to_device(pcm_context_t 		*ctxp, 
	pin_flist_t			*deal_flistp, 
	pin_flist_t			*hardware_flistp,
	pin_flist_t			*mso_pp_out_flistp,
	int32				plan_type,
	pin_errbuf_t		*ebufp);

extern int32
fm_cust_deac_act_purchased(
        pcm_context_t                   *ctxp,
        poid_t                          *acc_pdp,
        int32                           flag,
        pin_errbuf_t                    *ebufp);
	
static int32
fm_mso_check_grace_period_entry(
pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
	pin_flist_t             *mso_pp_flistp,
        pin_flist_t             **bb_ret_flistp,
        pin_errbuf_t            *ebufp);

static void
fm_mso_search_grace_period_entry(
pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
	pin_flist_t             **r_flistpp,
        pin_flist_t             **bb_ret_flistp,
        pin_errbuf_t            *ebufp);

/* Function to calculate static IP charge */
void  calc_static_ip_charge(cm_nap_connection_t     *connp,
                           pin_flist_t          *pur_in_flistp,
                           pin_flist_t          **r_flistpp,
                           pin_errbuf_t    *ebufp
                          );


void  apply_static_ip_charge(cm_nap_connection_t     *connp,
           pin_flist_t          *pur_in_flistp,
           pin_flist_t          **static_ip_pur_out_flistp,
           pin_errbuf_t    *ebufp
                          );
extern  void
fm_mso_cust_get_base_plan_details(
        pcm_context_t   *ctxp,
        pin_flist_t      *i_flistp,
        pin_errbuf_t    *ebufp);

/*******************************************************************
 * MSO_OP_CUST_ACTIVATE_BB_SERVICE 
 *
 *******************************************************************/
//Input flist
/************************************************************************************
0 PIN_FLD_POID           POID [0] 0.0.0.1 /plan -1 0
0 PIN_FLD_ACCOUNT_OBJ    POID [0] 0.0.0.1 /account 325529 0
0 PIN_FLD_BUSINESS_TYPE   ENUM [0] 99011000
0 PIN_FLD_SERVICES      ARRAY [0] allocated 20, used 4
1     PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/telco/broadband 555557 0
1     MSO_FLD_BB_INFO    SUBSTRUCT [0] allocated 20, used 3
2		  MSO_FLD_ISTAL_FLAG 	ENUM [0] 1
0 MSO_FLD_SUBSCRIPTION_PLANS SUBSTRUCT [0] allocated 20, used 3
1     PIN_FLD_PLAN          ARRAY [0] allocated 20, used 4
2         PIN_FLD_PLAN_OBJ       POID [0] 0.0.0.1 /plan 191188 2
2         MSO_FLD_PLAN_NAME            STR [0] "STB_Zero_Value"
2		  PIN_FLD_ACTION			   STR [0] "ADD"
2		  PIN_FLD_PRODUCTS          	ARRAY [0] allocated 20, used 4
3         	PIN_FLD_PRODUCT_OBJ       	POID [0] 0.0.0.1 /product 191188 2
3         	PIN_FLD_PURCHASE_END_T 		TSTAMP [0] (1398796200) Wed Apr 30 00:00:00 2014
3         	PIN_FLD_PURCHASE_START_T 	TSTAMP [0] (1394562600) Wed Mar 12 00:00:00 2014
3			MSO_FLD_DISC_AMOUNT			DECIMAL [0] NULL
3			MSO_FLD_DISC_PERCENT		DECIMAL [0] NULL
1     PIN_FLD_PLAN          ARRAY [1] allocated 20, used 4
2         PIN_FLD_PLAN_OBJ       POID [0] 0.0.0.1 /plan 191188 2
2         MSO_FLD_PLAN_NAME            STR [0] "STB_Zero_Value"
2		  PIN_FLD_ACTION			   STR [0] "ADD"
2		  PIN_FLD_PRODUCTS          	ARRAY [0] allocated 20, used 4
3         	PIN_FLD_PRODUCT_OBJ       	POID [0] 0.0.0.1 /product 191188 2
3         	PIN_FLD_PURCHASE_END_T 		TSTAMP [0] (1398796200) Wed Apr 30 00:00:00 2014
3         	PIN_FLD_PURCHASE_START_T 	TSTAMP [0] (1394562600) Wed Mar 12 00:00:00 2014
3			MSO_FLD_DISC_AMOUNT			DECIMAL [0] NULL
3			MSO_FLD_DISC_PERCENT		DECIMAL [0] NULL
0 MSO_FLD_INSTALLATION_PLANS SUBSTRUCT [0] allocated 20, used 3
1     PIN_FLD_PLAN          ARRAY [0] allocated 20, used 4
2         PIN_FLD_PLAN_OBJ       POID [0] 0.0.0.1 /plan 191188 2
2         MSO_FLD_PLAN_NAME            STR [0] "STB_Zero_Value"
2		  PIN_FLD_ACTION			   STR [0] "ADD"
2		  PIN_FLD_PRODUCTS          	ARRAY [0] allocated 20, used 4
3         	PIN_FLD_PRODUCT_OBJ       	POID [0] 0.0.0.1 /product 191188 2
3         	PIN_FLD_PURCHASE_END_T 		TSTAMP [0] (1398796200) Wed Apr 30 00:00:00 2014
3         	PIN_FLD_PURCHASE_START_T 	TSTAMP [0] (1394562600) Wed Mar 12 00:00:00 2014
3			MSO_FLD_DISC_AMOUNT			DECIMAL [0] NULL
3			MSO_FLD_DISC_PERCENT		DECIMAL [0] NULL
*******************************************************************************************/ 
 
EXPORT_OP void
op_mso_cust_activate_bb_service(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_activate_bb_service(
	        cm_nap_connection_t     *connp,	
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

extern int
fm_quota_check(
        pcm_context_t                   *ctxp,
        char                            *plan_name,
        char                            *prov_tag,
        int                             *tech,
        int                             *plan_typ,
        pin_decimal_t                   **initial_amount,
        int                             *error_codep,
        pin_errbuf_t                    *ebufp);

	
/*******************************************************************
 * MSO_OP_CUST_ACTIVATE_BB_SERVICE  
 *******************************************************************/
void 
op_mso_cust_activate_bb_service(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*trans_flistp = NULL;
	pin_flist_t		*trans_out_flistp = NULL;
	int32			local = LOCAL_TRANS_OPEN_SUCCESS;
	int32			status_uncommitted =1;
	int                     *status = NULL;
	int32			*ret_status = NULL;
	poid_t			*inp_pdp = NULL;
	char			*prog_name = NULL;
	*r_flistpp		= NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_ACTIVATE_BB_SERVICE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_activate_bb_service error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_activate_bb_service input flist", i_flistp);
	
/**     Added Trasnsaction handling   ***/
                //inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
		//local = fm_mso_trans_open(ctxp, inp_pdp, READWRITE,ebufp);
		inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
                local = fm_mso_trans_open(ctxp, inp_pdp, 3,ebufp);
                if(PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Unable to open Local Transaction: Error", ebufp);
                        PIN_ERRBUF_RESET(ebufp);
                        *r_flistpp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, inp_pdp, ebufp );
                        PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &status_uncommitted, ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "51413", ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, "Unable to open Local Transaction: Error", ebufp);
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
							"Return Flist", *r_flistpp);
                        return;
                }
                if ( local == LOCAL_TRANS_OPEN_FAIL )
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Unable to open Local Transaction: Already Open", ebufp);
                        PIN_ERRBUF_RESET(ebufp);
                        *r_flistpp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, inp_pdp, ebufp );
                        PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &status_uncommitted, ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "51414", ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, "Unable to open Local Transaction: Already Open", ebufp);
                        return;
                }
/**     Added Trasnsaction handling   ***/
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/

	fm_mso_cust_activate_bb_service(connp, flags, i_flistp, r_flistpp, ebufp);

	if ( r_flistpp )
		status = PIN_FLIST_FLD_GET( *r_flistpp, PIN_FLD_STATUS, 1, ebufp);
/*     Added TRansaction Handling   ***/
        /***********************************************************
         * Results.
         ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Return flist after activate service is :", *r_flistpp );       
 	if (PIN_ERRBUF_IS_ERR(ebufp) || (status && (*(int*)status == 1)))
        {
		if((prog_name && strstr(prog_name,"pin_deferred_act")))
        	{
                        pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_VALUE, PIN_FLD_STATUS, 0, 0);
			return;
        	}
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERRBUF_RESET(ebufp);
                }
                //PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_activate_bb_service error", ebufp);
		if(local == LOCAL_TRANS_OPEN_SUCCESS &&
			(!(prog_name && strstr(prog_name,"pin_deferred_act")))
		  )
		{
			fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Transaction NOT aborted");
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_uncommitted, ebufp);
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "After Trans Abort");
        }
        else
        {
                if(local == LOCAL_TRANS_OPEN_SUCCESS &&
                   !(prog_name && strcmp(prog_name,"pin_deferred_act") ==0 )
                  )
                {
                        fm_mso_trans_commit(ctxp, inp_pdp, ebufp);
                }
                else
                {
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Transaction NOT Comitted");
                                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_uncommitted, ebufp);
                }
        }
/*     Added TRansaction Handling   ***/

 	/***********************************************************
	 * Results.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_activate_bb_service output flist", *r_flistpp);
	
	return;
}



static void 
fm_mso_cust_activate_bb_service(
        cm_nap_connection_t     *connp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
        pcm_context_t   *ctxp = connp->dm_ctx;
	pin_flist_t		*ret_flistp;
	pin_flist_t		*ser_flistp=NULL;
	//pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*inher_flistp=NULL;
	pin_flist_t		*bb_info_flistp=NULL;
	pin_flist_t     	*mso_act_flistp = NULL;
	pin_flist_t		*pur_plan_flistp = NULL;
	pin_flist_t		*ott_flistp = NULL;
	pin_flist_t		*dev_serv_flistp = NULL;
	pin_flist_t		*hardware_flistp = NULL;
	pin_flist_t		*temp_flistp = NULL;
	pin_flist_t		*read_in_flistp = NULL;
	pin_flist_t		*read_out_flistp = NULL;
	poid_t			*acc_pdp = NULL;
	//poid_t			*catv_acc_pdp = NULL;
	poid_t			*srvc_pdp = NULL;

	pin_cookie_t		cookie = NULL;
 	int32			*business_type = NULL;
 	int32			bus_type_val = 0;
	int32			account_type = 0;
	int32			account_model = 0;
	int32			subscriber_type = 0;
	int32			service_type = 0;
	int32			errorCode	= 0;
	int32			ret_status	= -1;
	int32			bb_ser_count = 0;
	int				result = -1;
	int			opcode = MSO_OP_CUST_ACTIVATE_BB_SERVICE;
	int32			*tech = NULL;
	//int32			acc_no_val = 0;

	char			*errorDesc	= NULL;
	char			*errorField	= NULL;
	char			errorMsg[80]	= "";
	char			*prog_name = NULL;
	char			temp[80]	= "";
	char			*bb_service_type = "/service/telco/broadband";
	char			*tag_acc_nop = NULL;
    char            *str_rolep = NULL;

	int32			srvc_status_active = PIN_STATUS_ACTIVE;
	int32			srvc_status_flags = 0;
	int32			activate_service_success = 0;
	int32			activate_service_failure = 1;
	int32			prov_in_prog_status = MSO_PROV_IN_PROGRESS;

	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_activate_bb_service input flist", i_flistp);

	acc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);

	if (!acc_pdp)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Missing Account Poid", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, NULL, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &activate_service_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51701", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Missing Account Poid", ebufp);
		goto CLEANUP;
	}

	/*************************************************************************
         * Check CATV service is active or not for Hybrid accounts.

        fm_mso_get_account_info(ctxp, acc_pdp, &acnt_info, ebufp);
        tag_acc_nop = PIN_FLIST_FLD_TAKE(acnt_info, PIN_FLD_AAC_PACKAGE, 1, ebufp);
        if (tag_acc_nop && strlen(tag_acc_nop) != 10 || sscanf(tag_acc_nop, "%d", &acc_no_val) != 1)
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Invalid Tagging Account Number Format");
        }
        else if (tag_acc_nop == NULL)
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "There is no tagging account number");
        }
        else
        {
                PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "This is the proper tagging account number");
                fm_mso_get_acnt_from_acntno(ctxp, tag_acc_nop, &acnt_info, ebufp);

                pin_free(tag_acc_nop);

                if (acnt_info == NULL)
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CATV Account Number not in our system");
                        ret_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acc_pdp, ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &activate_service_failure, ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51702", ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "CATV Account Number not found", ebufp);
                        goto CLEANUP;
                }
                catv_acc_pdp = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_POID, 0, ebufp);
		
		fm_mso_utils_get_service_details(ctxp, catv_acc_pdp, 0, &ser_flistp, ebufp);
                PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
                temp_flistp = PIN_FLIST_ELEM_GET(ser_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
                if (temp_flistp == NULL)
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "No CATV services are activated !!");
                        ret_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acc_pdp, ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &activate_service_failure, ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51703", ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "CATV Service not active", ebufp);
                        PIN_FLIST_DESTROY_EX(&ser_flistp, NULL);
                        goto CLEANUP;
                }
                PIN_FLIST_DESTROY_EX(&ser_flistp, NULL);
        }
         ************************************************************************/

	ser_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_SERVICES, 0, 1, ebufp);
	if(ser_flistp)
	{
		srvc_pdp = PIN_FLIST_FLD_GET(ser_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		if (!srvc_pdp)
		{		
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Missing Service Poid in Services Array", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, NULL, ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &activate_service_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51713", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Service Poid is missing in Services Array.", ebufp);
			goto CLEANUP;
		}
	}
	else if(!srvc_pdp)
	{		
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Missing Services Array", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, NULL, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &activate_service_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51714", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Missing Services Array in input.", ebufp);
		goto CLEANUP;
	}
	
 	/*******************************************************************
	* Derive account_type,account_model & subscriber_type from business_type 
	*******************************************************************/
	business_type = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BUSINESS_TYPE, 1, ebufp);
	if (business_type)
	{
		bus_type_val = *business_type;
	}
	else
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Missing Business Type", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, NULL, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &activate_service_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51702", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Missing Business Type Value", ebufp);
		goto CLEANUP;
	}

	/***************************************
	 *	First 2 digits: Account Type, 
	 *	3rd and 4th digit: Subscriber Type,
	 *	5th digit : Account Model,
	 *	6th digit : Service Type 
	 *	7th and 8th digit : RESERVED 
	 ***************************************/

	account_type = bus_type_val/1000000;
	bus_type_val = bus_type_val%1000000;
	
	subscriber_type = bus_type_val/10000;
	bus_type_val = bus_type_val%10000;
	
	account_model = bus_type_val/1000;
	bus_type_val = bus_type_val%1000;
	
	service_type = bus_type_val/100;



	/*******************************************************************
	* Validate Plan Activation 
	*******************************************************************/
	ret_flistp=NULL;
	
	fm_mso_validate_plan_activation(ctxp, i_flistp, &mso_act_flistp, &ret_flistp, ebufp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Validate Plan activation return flist", ret_flistp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO activation plan flist", mso_act_flistp );
	if (ret_flistp)
	{
		ret_status = *(int32*)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);
		if (ret_status == activate_service_failure)
		{
			goto CLEANUP;
		}
	}
	
	
	/*******************************************************************
	* Set service status to active
	*******************************************************************/
	fm_mso_set_bb_service_status(ctxp, i_flistp, &ret_flistp, ebufp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "SET STATUS return flist", ret_flistp );
	if (ret_flistp)
	{
		ret_status = *(int32*)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);
		if (ret_status == activate_service_failure)
		{
			goto CLEANUP;
		}
	} 
	
        /* call provisioning action 
        fm_mso_call_prov_action(ctxp, i_flistp, &ret_flistp, ebufp );
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Prov action return flist", ret_flistp );
        if (ret_flistp)
        {
                ret_status = *(int32*)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);
                if (ret_status == activate_service_failure)
                {
                        goto CLEANUP;
                }
        } */

	/*******************************************************************
	* Purchase plans
	*******************************************************************/
	//fm_mso_purchase_bb_plans(ctxp, mso_act_flistp, &hardware_flistp, &pur_plan_flistp, &ret_flistp, ebufp );


	/* set service provisioning status */
	//Pawan: Added below block to make the prov status in progress only when  
	// technology is not Fiber.
	//Pavan Bellala - 21-July-2015 - Moving section up for fetching technology details.
	read_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_in_flistp, PIN_FLD_POID, srvc_pdp, ebufp );
	bb_info_flistp = PIN_FLIST_SUBSTR_ADD(read_in_flistp, MSO_FLD_BB_INFO, ebufp );
	PIN_FLIST_FLD_SET(bb_info_flistp, MSO_FLD_TECHNOLOGY, NULL, ebufp);
    PIN_FLIST_FLD_SET(bb_info_flistp, MSO_FLD_ROLES, NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read flds Input Flist", read_in_flistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_in_flistp, &read_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read flds output flist", read_out_flistp);
	bb_info_flistp = PIN_FLIST_SUBSTR_GET(read_out_flistp, MSO_FLD_BB_INFO, 0, ebufp );
    str_rolep = PIN_FLIST_FLD_GET(bb_info_flistp, MSO_FLD_ROLES, 1, ebufp );
	tech = PIN_FLIST_FLD_TAKE(bb_info_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp);	
	PIN_FLIST_DESTROY_EX(&read_in_flistp,NULL);
	PIN_FLIST_DESTROY_EX(&read_out_flistp,NULL);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in reading service technology", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, NULL, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &activate_service_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51731", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in reading service technology", ebufp);
		goto CLEANUP;		
	}
    if(str_rolep == NULL || strcmp(str_rolep, "") == 0)
    {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "ROLE Parameter missing in Input");
        PIN_ERRBUF_RESET(ebufp);
        ret_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acc_pdp, ebufp );
        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &activate_service_failure, ebufp);
        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51090", ebufp);
        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "ROLE Parameter missing in Input", ebufp); 
        *r_flistpp = ret_flistp;
        return;
     }

	//Pavan Bellala - 21-July2-15 Added technology to send SMS based on it
	PIN_FLIST_FLD_SET(mso_act_flistp,MSO_FLD_TECHNOLOGY,tech,ebufp);
	//For checking DISCOUNT OFFER and apply - AD - 26-04-2017
	PIN_FLIST_FLD_SET(mso_act_flistp, PIN_FLD_OPCODE, &opcode, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_act_flistp post tech. addition", mso_act_flistp );

	fm_mso_purchase_bb_plans(connp, mso_act_flistp, (char *)NULL,0,0,&ret_flistp, ebufp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchase Plan return flist", ret_flistp );
	if (ret_flistp)
	{
		ret_status = *(int32*)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);
		if (ret_status == activate_service_failure)
		{
			goto CLEANUP;
		}
	} 
	else
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in purchasing plan.", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, NULL, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &activate_service_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51727", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in purchasing plan", ebufp);
		goto CLEANUP;	
	}
	
	
	//if(*tech!=MSO_FIBER)
	//{
		result = fm_mso_set_bb_prov_status(ctxp, srvc_pdp, prov_in_prog_status, ebufp );
		if (result == activate_service_failure)
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Can't update service provisioning status.", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, NULL, ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &activate_service_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51725", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Can't update service provisioning status", ebufp);
			goto CLEANUP;
		}
	//}
	
	/* Set credit limit as per plan*/
	fm_mso_cust_bb_set_cr_limit(ctxp, i_flistp, &ret_flistp, ebufp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Set Credit Limit return flist", ret_flistp );
	if ( ret_flistp)
	{
		ret_status = *(int32*)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);
		if (ret_status == activate_service_failure)
		{
			goto CLEANUP;
		}
	} 
	
	/*******************************************************************
	* Call Deposit API
	*******************************************************************/
	
/*	fm_mso_create_device_deposit(ctxp, i_flistp, hardware_flistp, &ret_flistp, ebufp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Link device deposit return flist", ret_flistp );
	if (ret_flistp)
	{
		ret_status = *(int32*)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);
		if (ret_status == activate_service_failure)
		{
			goto CLEANUP;
		}
	} 
*/	
	/*******************************************************************
	* Create /mso_purcahsed_plan object for all purchased plans.
	*******************************************************************/	
/*	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, pur_plan_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	ser_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_SERVICES, 0, 1, ebufp);
	PIN_FLIST_FLD_COPY(ser_flistp, PIN_FLD_SERVICE_OBJ, pur_plan_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	fm_mso_create_purchased_plan_object(ctxp, pur_plan_flistp, &ret_flistp, ebufp );
	if (ret_flistp)
	{
		ret_status = *(int32*)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);
		if (ret_status == activate_service_failure)
		{
			goto CLEANUP;
		}
	} */

	//lifecycle event
	fm_mso_create_lifecycle_evt(ctxp, i_flistp, mso_act_flistp, ID_ACTIVATE_BB_SERVICE, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
		PIN_ERRBUF_RESET(ebufp);
	}

	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input flist, At end....", i_flistp );
	ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &activate_service_success, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, ret_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp );
	temp_flistp = PIN_FLIST_ELEM_ADD(ret_flistp, PIN_FLD_SERVICES, 0, ebufp);
	ser_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_SERVICES, 0, 1, ebufp);
	PIN_FLIST_FLD_COPY(ser_flistp, PIN_FLD_SERVICE_OBJ, temp_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(ser_flistp, PIN_FLD_LOGIN, temp_flistp, PIN_FLD_LOGIN, ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Leaving function, At end...." );
	//*r_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
	//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Leaving function, At end...." );
	/*******************************************************************
	* Changes for Auto OTT Swap on Service Recreation.
	*******************************************************************/	
	int		o_elem_id = 0;
	pin_cookie_t	o_cookie = NULL;
	fm_mso_get_device_reg_act(ctxp, acc_pdp, &ott_flistp, ebufp);
	if (ott_flistp !=NULL && PIN_FLIST_ELEM_COUNT(ott_flistp, PIN_FLD_RESULTS, ebufp) > 0)
	{
		while((temp_flistp = PIN_FLIST_ELEM_GET_NEXT(ott_flistp, PIN_FLD_RESULTS,
			&o_elem_id, 1, &o_cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			/*******************************************************************
			* WRITE FIELD CALL TO CHANGE SERVICE POID TO AVOID COMPLICATIONS
			AND AS NON PROVISIONAL DEVICE IT IS NOT MANDATORY.
			*******************************************************************/	
			ser_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ser_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(temp_flistp,PIN_FLD_POID,1,ebufp), ebufp);
			dev_serv_flistp = PIN_FLIST_ELEM_ADD(ser_flistp, PIN_FLD_SERVICES, 0, ebufp);
			PIN_FLIST_FLD_SET(dev_serv_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "WRITE FLDS FOR OTT_REACT INPUT", ser_flistp);
			PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, ser_flistp, &read_out_flistp, ebufp);	
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "WRITE FLDS FOR OTT_REACT OUTPUT", read_out_flistp);
		}
	}
	/*******************************************************************
	* EOC for OTT
	*******************************************************************/	
		
	CLEANUP:
	*r_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&hardware_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&mso_act_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&pur_plan_flistp, NULL);

	if(tech) free(tech);

	return;
}


/**************************************************
* Function: fm_mso_validate_plan_activation()
* 
* 
***************************************************/
static void
fm_mso_validate_plan_activation(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**mso_act_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*ser_flistp = NULL;
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	poid_t			*acc_pdp = NULL;
	poid_t			*srch_pdp = NULL;
	char			*template = "select X from /mso_plans_activation where F1.id = V1 and F2.id = V2 ";
	int64			db=0;
	int32			srch_flag=0;
	int				count=0;
	int				*status = NULL;
	int32			activate_service_failure = 1;
	int32			activate_service_success = 0;
	int                     status_purchased = 1;	
	pin_flist_t             *write_in_flistp = NULL;
	pin_flist_t             *write_out_flistp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_validate_plan_activation", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_plan_activation input flist", in_flistp);	

	acc_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	
	db = PIN_POID_GET_DB(acc_pdp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);
	
	arg_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acc_pdp, ebufp);
	
	ser_flistp = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_SERVICES, 0, 1, ebufp);
	arg_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_COPY(ser_flistp, PIN_FLD_SERVICE_OBJ, arg_flist, PIN_FLD_SERVICE_OBJ, ebufp);
	
	result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Plan Activation search input flist", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Plan Activation search output flist", srch_out_flistp);

	count = PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp);
	if(count == 1)
	{
		result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp );
		status = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_STATUS, 1, ebufp);
		if (status && *((int32 *)status)!= 0)
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Plans are already active. Cannot purchase again.", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			*ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_service_failure, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51715", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Plans are already active. Cannot purchase again.", ebufp);
			goto CLEANUP;
		}
		else
		{
			write_in_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_POID, write_in_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(write_in_flistp, PIN_FLD_STATUS, &status_purchased, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Write flds Input Flist", write_in_flistp);
			PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_in_flistp, &write_out_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Write flds output flist", write_out_flistp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in updating mso_plans_activation status", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				*ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_service_failure, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51719", ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in updating mso_plans_activation status", ebufp);
				goto CLEANUP;
			}
		}
	}
	else if(count > 1)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"More than one instance of Plan activation.", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_service_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51711", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "More than one instance of Plan activation.", ebufp);
		goto CLEANUP;
	}
	else if(count ==0)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"No plans selected. Select a plan and try again.", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_service_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51716", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "No plans selected. Select a plan and try again.", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Plan to purchase.", result_flist);
	*mso_act_flistp = PIN_FLIST_COPY(result_flist, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, *mso_act_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_USERID, *mso_act_flistp, PIN_FLD_USERID, ebufp);
	
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&write_in_flistp, NULL);
	return;
}


/******************************************************
* Function: fm_mso_purchase_bb_plans()
* Description: Purchases each plan for an account 
*			 which is specified in mso_act_flistp.
* 			
*******************************************************/

extern void
fm_mso_purchase_bb_plans(
	cm_nap_connection_t     *connp,	
	pin_flist_t		*mso_act_flistp,
	char			*prov_tag,
	int			flags,
	int			grace_flag,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t   *ctxp = connp->dm_ctx;
	pin_flist_t		*pur_in_flistp = NULL;
	pin_flist_t		*pur_out_flistp = NULL;
	pin_flist_t		*deal_info_flistp = NULL;
	pin_flist_t		*deal_flistp = NULL;
	pin_flist_t		*plan_flistp = NULL;
	pin_flist_t		*deal_array = NULL;
	pin_flist_t		*temp_flistp = NULL;
	pin_flist_t		*write_in_flistp = NULL;
	pin_flist_t		*prod_flistp = NULL;
	pin_flist_t		*device_flistp = NULL;
	pin_flist_t		*pur_plan_flistp = NULL;
	pin_flist_t		*hardware_flistp = NULL;
	pin_flist_t		*deals_deposit = NULL;
	pin_flist_t		*deals_activation = NULL;
	pin_flist_t		*get_prod_flistp = NULL;
	pin_flist_t		*act_prod_flistp = NULL;
	pin_flist_t		*bb_bp_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t     *r_flistp = NULL;
	pin_flist_t		*ser_in_flistp = NULL;
	pin_flist_t     *ser_out_flistp = NULL;
	pin_flist_t		*calc_out_flistp = NULL;
	pin_flist_t		*ncr_sb_flistp = NULL;
	pin_flist_t		*disc_offer_flistp = NULL;
	pin_flist_t		*disc_off_oflistp = NULL;
	pin_flist_t		*rp_oflistp = NULL;
	pin_flist_t		*offer_flistp = NULL;
	poid_t			*hw_plan_pdp = NULL;
	poid_t			*pur_plan_pdp = NULL;
	poid_t			*plan_pdp = NULL;
	poid_t			*act_plan_pdp = NULL;
	poid_t			*prod_pdp = NULL;
	poid_t			*act_prod_pdp = NULL;
	poid_t			*subs_plan_pdp = NULL;
	poid_t			*mso_pp_flistp = NULL;
	poid_t			*base_plan_pdp = NULL;
	int				status_purchased = 1;
	int				elem_id = 0;
	int				p_elem_id = 0;
	int				h_elem_id = 0;
	int				pr_elem_id = 0;
	int				count = 0;
	int				comp_plan = 0;
	int 				c_limit_not_set= 100010;
	int				no_act_arg= 100002;
	int				insuff_lco_bal= 100004;	
	int 			comp_prod = -1;
	int				*plan_type = NULL;
	int				abs_valid = PIN_VALIDITY_ABSOLUTE;
	int				*tech = NULL;
	int				ret_val = 0;
	int				*p_type = NULL;
	int				zero = 0;
	int32                   bb_act_failure = 1;
	int			offer_type = 1;
	int32                   *disc_flags = NULL;
	int32                   *opcode = NULL;
	int32                   *opcode_check = NULL;
	int32			grace_flag_local = 0;
        int32                   discount_override = 0;

	pin_cookie_t		cookie = NULL;
	pin_cookie_t		p_cookie = NULL;
	pin_cookie_t		pr_cookie = NULL;
	pin_cookie_t		h_cookie = NULL;
	pin_cookie_t 		offer_cookie = NULL;
	int32			activate_service_failure = 1;
	int32			activate_service_success = 0;
	int32			status_inactive = 2;
	int32			status_active = 1;
	int32			ret_status = 0;
	int32			one = 1;
	int32			price_calc_flag = 0;
	void			*vp = NULL;
	
	char			tmp[256];
	char			error_code[16];
	char			error_descr[256];
	char			*act_prov_tag = NULL;

	pin_decimal_t		*disc_per = NULL;
	pin_decimal_t		*disc_amt = NULL;
	pin_decimal_t		*disc_off_amt = NULL;
	pin_decimal_t		*add_disc_off_amt = NULL;
	pin_decimal_t		*mod_disc_per = NULL;
	pin_decimal_t		*hundred = NULL;
	pin_decimal_t		*zero_decp = NULL;
	time_t		*purchase_end_t = NULL;
	time_t		current_t = 0;
	time_t		relative_t = 0;
	struct tm	*tt;	
	hundred = pbo_decimal_from_str("100", ebufp );
	
	pin_flist_t             *lco_err_flistp = NULL;
	pin_flist_t             *write_iflistp = NULL;
	pin_flist_t             *read_flistp = NULL;
	pin_flist_t             *read_res_flistp = NULL;
	pin_flist_t             *write_oflistp = NULL;
 	int64                   lco_err_code = 0;
	int32			found = 0;
	poid_t			*account_obj = NULL;
	int			*res_id = NULL;
	int32			base_plan = 0;
	int32			ret_flag = 0;
	int32			*end_t = NULL;
	int32                   *type = NULL;
	char			msg[128] = "";
	time_t		*validity_start_t = 0;
	pin_flist_t             *static_ip_pur_out_flistp = NULL;
	poid_t 			*service_obj=NULL;
	int32                   *ret_plan_type=NULL;
	int32			add_tal_plan = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_purchase_bb_plans", ebufp);
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_purchase_bb_plans input flist", mso_act_flistp );
	account_obj = PIN_FLIST_FLD_GET(mso_act_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp );
	vp = PIN_FLIST_FLD_GET(mso_act_flistp, PIN_FLD_MODE, 1, ebufp );
	if(vp && *(int32 *)vp == 1) {
		price_calc_flag = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Price Calc Flag (PIN_FLD_MODE) is 1");
	}

	//added for Tal_plan addition
	ret_plan_type = PIN_FLIST_FLD_GET(mso_act_flistp, MSO_FLD_PLAN_TYPE, 1, ebufp );
	if (ret_plan_type && *ret_plan_type == 7) {
			add_tal_plan = 1;	
	} 
	
	/* Copy Hardware plans with devices to a separate flist.*/
	hardware_flistp = PIN_FLIST_CREATE(ebufp);

	h_cookie = NULL;h_elem_id= 0;	
	while((plan_flistp = PIN_FLIST_ELEM_GET_NEXT(mso_act_flistp, PIN_FLD_PLAN,
		&h_elem_id, 1, &h_cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		device_flistp = NULL;
		device_flistp = PIN_FLIST_ELEM_GET(plan_flistp, PIN_FLD_DEVICES, PIN_ELEMID_ANY, 1, ebufp );
		// Either DEVICES array (in activation flow)or DEVICE_ID(in add HW plan flow)
		// will be present in plan flist.
		vp=NULL;
		vp = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_DEVICE_ID, 1, ebufp);
		/* If device array is found then copy the plan to hardware plan flist */
		if(device_flistp || vp)
		{
			PIN_FLIST_ELEM_SET(hardware_flistp, plan_flistp, PIN_FLD_PLAN, h_elem_id, ebufp );
		}
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Hardware Plan with devices flist", hardware_flistp );
	

	/* For getting deals for each plan passed in input */
	fm_mso_get_deals_from_plan(ctxp, mso_act_flistp, &deal_array, &temp_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Deal Array output", deal_array);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Credit Limit Array output", temp_flistp);
	PIN_FLIST_DESTROY_EX(&temp_flistp, NULL);
	count = PIN_FLIST_ELEM_COUNT(deal_array, PIN_FLD_DEALS, ebufp);
	if( count == 0)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Deals not found for selected Plans.", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(mso_act_flistp, PIN_FLD_ACCOUNT_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_service_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51717", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Deals not found for selected Plans.", ebufp);
		goto CLEANUP;
	}
	pur_plan_flistp = PIN_FLIST_COPY(deal_array, ebufp);
	get_prod_flistp = PIN_FLIST_COPY(deal_array, ebufp);
	fm_mso_get_deal_info(ctxp, get_prod_flistp, &deals_deposit, &deals_activation,ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Deals Activation Flist", deals_activation);
	if(deals_deposit) {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Deals Deposit Flist", deals_deposit);
	}
	PIN_FLIST_DESTROY_EX(&get_prod_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&deals_deposit, NULL);

	// Update the product customization in purchase deal flist*/
	while((deal_flistp = PIN_FLIST_ELEM_GET_NEXT(deals_activation, PIN_FLD_DEALS,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{

	/* Changes for Validity Extension CR . Update the PURCHASE_START_T & CYCLE_START_T based on input. */	

		plan_pdp = PIN_FLIST_FLD_GET(deal_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
		p_cookie=NULL;
		deal_info_flistp = PIN_FLIST_SUBSTR_GET(deal_flistp, PIN_FLD_DEAL_INFO, 0, ebufp );
		while((prod_flistp = PIN_FLIST_ELEM_GET_NEXT(deal_info_flistp, PIN_FLD_PRODUCTS,
			&p_elem_id, 1, &p_cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			/* Iterate though Plans activation flist to fetch the product 
				customization for this plan */
			prod_pdp = PIN_FLIST_FLD_GET(prod_flistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
			h_cookie=NULL;
			while((plan_flistp = PIN_FLIST_ELEM_GET_NEXT(mso_act_flistp, PIN_FLD_PLAN,
			&h_elem_id, 1, &h_cookie, ebufp)) != (pin_flist_t *)NULL)
			{
				act_plan_pdp = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp );
				comp_plan = PIN_POID_COMPARE(plan_pdp, act_plan_pdp, 0, ebufp );
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG," Mso activation plan flist is :",plan_flistp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG," Deal flist is :",deal_flistp);
				plan_type = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_TYPE, 1, ebufp ); // Subs, Hardware, Inst
				if(comp_plan == 0)
				{
					/* Plan matched. Iterate though product */
					vp = NULL;
					vp = (char *)PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_DESCR, 1, ebufp );
					if ( vp && strcmp((char *)vp,MSO_BB_BP) == 0 ) {
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "This is base plan");
						bb_bp_flistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_COPY(plan_flistp, PIN_FLD_PLAN_OBJ, bb_bp_flistp, PIN_FLD_PLAN_OBJ, ebufp);	
						PIN_FLIST_FLD_SET(bb_bp_flistp, PIN_FLD_DESCR, (char *)vp, ebufp);
					}  
					/* Purchase the product in inactive status if plan_type is Subscription(1)*/
					//For Tal plan addtion we are creating active plans .So exclude Tal plan 
					if(plan_type && *plan_type == 1 && add_tal_plan == 0) 
					{ 
						PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_STATUS, &status_inactive , ebufp );
						PIN_FLIST_FLD_SET(deal_flistp, PIN_FLD_TYPE, &one , ebufp );
					}
					else
					{
						PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_STATUS, &status_active , ebufp );
					}
					
					pr_cookie=NULL;
					while((act_prod_flistp = PIN_FLIST_ELEM_GET_NEXT(plan_flistp, PIN_FLD_PRODUCTS,
						&pr_elem_id, 1, &pr_cookie, ebufp)) != (pin_flist_t *)NULL)
					{
						act_prod_pdp = PIN_FLIST_FLD_GET(act_prod_flistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp );
						comp_prod = PIN_POID_COMPARE(prod_pdp, act_prod_pdp, 0, ebufp );
						if(comp_prod == 0)
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Mtach : prod flist");
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Match: Prod flist", prod_flistp);
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Match: Activation Prod flist", act_prod_flistp);
							/*Product matched. Fetch the discount amount/percent and  
							  start/end dates */
							disc_per = NULL;
							disc_amt = NULL;
							disc_per = (pin_decimal_t *)PIN_FLIST_FLD_GET(act_prod_flistp, MSO_FLD_DISC_PERCENT, 1, ebufp );
							disc_amt = (pin_decimal_t *)PIN_FLIST_FLD_GET(act_prod_flistp, MSO_FLD_DISC_AMOUNT, 1, ebufp );
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Fetched discount values.");
							if(disc_per && !pbo_decimal_is_null(disc_per, ebufp)) {
								/* If discount percent is found then set it in purchase deal -> product array */
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Setting Discount Percent.");
								mod_disc_per = NULL;
								mod_disc_per = pbo_decimal_divide(disc_per, hundred, ebufp );
								PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_CYCLE_DISCOUNT, mod_disc_per, ebufp );
								PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_PURCHASE_DISCOUNT, mod_disc_per, ebufp );
								PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_USAGE_DISCOUNT, mod_disc_per, ebufp );
								discount_override = 1;
								pbo_decimal_destroy(&mod_disc_per);
							} 
							if(disc_amt && !pbo_decimal_is_null(disc_amt, ebufp))	{
								/* If discount amount is found then set it in purchase deal -> product array */
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Setting Discount Amount.");
								PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_CYCLE_DISC_AMT, disc_amt, ebufp );
								PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_PURCHASE_DISC_AMT, disc_amt, ebufp );
								discount_override = 1;
							}
							// DISCOUNT OFFER FUNCTIONALITY BEGINS
							// DISCOUNT offer will be provided only for CHANGE PLAN or RENEWAL of BB 
							opcode = PIN_FLIST_FLD_GET(mso_act_flistp, PIN_FLD_OPCODE, 1, ebufp);
							if (opcode && (//*opcode == MSO_OP_CUST_CHANGE_PLAN || 
									*opcode == MSO_OP_CUST_ACTIVATE_BB_SERVICE || 
									*opcode == MSO_OP_CUST_TOP_UP_BB_PLAN || 
									*opcode == MSO_OP_CUST_RENEW_BB_PLAN))
							{
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CHANGE PLAN or RENWAL Case");
								fm_mso_utils_read_any_object(ctxp, act_prod_pdp, &rp_oflistp, ebufp);
								if(rp_oflistp){
									act_prov_tag = PIN_FLIST_FLD_GET(rp_oflistp, PIN_FLD_PROVISIONING_TAG, 1, ebufp);
									if((discount_override == 0) && act_prov_tag && strcmp(act_prov_tag, "") != 0){//SUBSCRIPTION PRODUCT
										disc_offer_flistp = PIN_FLIST_CREATE(ebufp);
										PIN_FLIST_FLD_COPY(mso_act_flistp, PIN_FLD_ACCOUNT_OBJ, 
													disc_offer_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp );
										offer_type = 1;//Discount Type
										PIN_FLIST_FLD_SET(disc_offer_flistp, PIN_FLD_TYPE, &offer_type, ebufp);
										fm_mso_search_offer_entries(ctxp, disc_offer_flistp, &disc_off_oflistp, ebufp);
										PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Discount Offers outflist", disc_off_oflistp);
										if(disc_off_oflistp && PIN_FLIST_ELEM_COUNT(disc_off_oflistp, 
														PIN_FLD_RESULTS, ebufp) > 0)
										{
											zero_decp = pbo_decimal_from_str("0", ebufp);
											// We would get only one offer at a time based on VALID_TO
											offer_flistp = PIN_FLIST_ELEM_GET(disc_off_oflistp, PIN_FLD_RESULTS,
														PIN_ELEMID_ANY, 1, ebufp);
											if (offer_flistp){
												disc_flags = PIN_FLIST_FLD_GET(offer_flistp, PIN_FLD_DISCOUNT_FLAGS,
															1, ebufp);
												disc_off_amt = PIN_FLIST_FLD_GET(offer_flistp, PIN_FLD_AMOUNT,1,ebufp);
												if (disc_flags && *disc_flags == 1){ //PERCENT
													PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_CYCLE_DISCOUNT, 
																disc_off_amt, ebufp );
													PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_PURCHASE_DISCOUNT, 																	disc_off_amt, ebufp );
													PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_USAGE_DISCOUNT, 																		disc_off_amt, ebufp );
													PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_CYCLE_DISC_AMT,                                                                                                                                          zero_decp, ebufp );
													PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_PURCHASE_DISC_AMT,                                                                                                                                       zero_decp, ebufp );
												}else if(disc_flags && *disc_flags == 2){ //AMOUNT
													PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_CYCLE_DISC_AMT, 																		disc_off_amt, ebufp );
													PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_PURCHASE_DISC_AMT, 																	disc_off_amt, ebufp );
													PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_CYCLE_DISCOUNT,
																zero_decp, ebufp );
													PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_PURCHASE_DISCOUNT,                                                                                                                                       zero_decp, ebufp );
													PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_USAGE_DISCOUNT,                                                                                                                                          zero_decp, ebufp );
												}
											}
											if(!pbo_decimal_is_null(zero_decp, ebufp))
												 pbo_decimal_destroy(&zero_decp);
										}
									}
								}
							}
							PIN_FLIST_DESTROY_EX(&disc_off_oflistp , NULL); 
							PIN_FLIST_DESTROY_EX(&disc_offer_flistp, NULL);
							//END OF DISOCUNT OFFER FUNCTIONALITY

							/*vp = NULL;
							vp = PIN_FLIST_FLD_GET(act_prod_flistp, PIN_FLD_PURCHASE_START_T, 1, ebufp );
							if (vp) {
								PIN_FLIST_FLD_COPY(act_prod_flistp, PIN_FLD_PURCHASE_START_T, prod_flistp, PIN_FLD_PURCHASE_START_T, ebufp);
								PIN_FLIST_FLD_COPY(act_prod_flistp, PIN_FLD_PURCHASE_START_T, prod_flistp, PIN_FLD_CYCLE_START_T, ebufp);
								PIN_FLIST_FLD_COPY(act_prod_flistp, PIN_FLD_PURCHASE_START_T, prod_flistp, PIN_FLD_USAGE_START_T, ebufp);
								//PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_CYCLE_START_DETAILS, &abs_valid , ebufp );
								//PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_USAGE_START_DETAILS, &abs_valid , ebufp );
							}*/
							purchase_end_t = NULL;
							purchase_end_t = PIN_FLIST_FLD_GET(act_prod_flistp, PIN_FLD_PURCHASE_END_T, 1, ebufp );
							if (purchase_end_t && *purchase_end_t>0) {
								current_t = pin_virtual_time(NULL);
								tt = localtime(&current_t);
								sprintf(tmp,"Time: %d, %d, %d, %d, %d, %d ",tt->tm_mday, tt->tm_hour, tt->tm_min, tt->tm_sec);
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,tmp);
								tt->tm_hour=0;
								tt->tm_min=0;
								tt->tm_sec=0;
								sprintf(tmp,"Time: %d, %d, %d, %d, %d, %d ",tt->tm_mday, tt->tm_hour, tt->tm_min, tt->tm_sec);
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,tmp);								
								current_t = mktime (tt);
								relative_t = *purchase_end_t - current_t;								
								PIN_FLIST_FLD_SET( prod_flistp, PIN_FLD_PURCHASE_END_T, &relative_t, ebufp);
								PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_CYCLE_END_T, &relative_t, ebufp);
								PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_USAGE_END_T, &relative_t, ebufp);
								PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_CYCLE_END_UNIT, &zero , ebufp );
								PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_CYCLE_END_OFFSET, &zero , ebufp );
								PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_PURCHASE_END_UNIT, &zero , ebufp );
								PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_PURCHASE_END_OFFSET, &zero , ebufp );
								PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_USAGE_END_UNIT, &zero , ebufp );
								PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_USAGE_END_OFFSET, &zero , ebufp );
							}
							
							

						} // If - compare prod
					} // While - Activation prod
				} // If - compare prod	
			} // While - Activation plan
		}
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in setting Purchase discount.", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(mso_act_flistp, PIN_FLD_ACCOUNT_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_service_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51726", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in setting Purchase discount.", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Deals Activation Flist: After Customization", deals_activation);
	
	/* create the input flist for PURCHASE_DEAL opcode*/
	pur_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(mso_act_flistp, PIN_FLD_ACCOUNT_OBJ, pur_in_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(mso_act_flistp, PIN_FLD_SERVICE_OBJ, pur_in_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(mso_act_flistp, PIN_FLD_PROGRAM_NAME, pur_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	if (add_tal_plan == 1) {
		PIN_FLIST_FLD_COPY(mso_act_flistp, MSO_FLD_PLAN_TYPE, pur_in_flistp, MSO_FLD_PLAN_TYPE, ebufp);
		PIN_FLIST_FLD_COPY(mso_act_flistp, PIN_FLD_ACCOUNT_OBJ, pur_in_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	}
	
	/* Get each deal/plan from deal_array and call purchase deal for each */
	cookie = NULL;
	while((deal_flistp = PIN_FLIST_ELEM_GET_NEXT(deals_activation, PIN_FLD_DEALS,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		deal_info_flistp = PIN_FLIST_SUBSTR_GET(deal_flistp, PIN_FLD_DEAL_INFO, 0, ebufp);
		PIN_FLIST_FLD_COPY(deal_flistp, PIN_FLD_PLAN_OBJ, deal_info_flistp, PIN_FLD_PLAN_OBJ, ebufp);
		PIN_FLIST_SUBSTR_SET(pur_in_flistp, deal_info_flistp, PIN_FLD_DEAL_INFO, ebufp );
		/* Added For Calc-Only */
		p_type = PIN_FLIST_FLD_GET(deal_flistp, PIN_FLD_TYPE, 1, ebufp);
		/* Call in Calc only mode in following cases:
		 * 1) If plan type is subscription(p_type == 1) and this is activation flow (flags == 0) 
		 * 2) If Calc_flag is 1 and this is not activation flow (flags == 1)
		 */
		if((p_type && *p_type == 1 && flags == 0) || (flags == 1))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calc Only OR ACTIVATION CALL");
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Calc Only input flist", pur_in_flistp);
			PIN_FLIST_FLD_SET(pur_in_flistp, PIN_FLD_MODE,&price_calc_flag, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calc Only OR ACTIVATION CALL after mode set");
            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Calc Only input flist", pur_in_flistp);
            
			fm_mso_pur_deal_calc_only(connp, pur_in_flistp, &calc_out_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Calc Only output flist", calc_out_flistp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
                        {
                                 goto ERR;
                        }
			p_cookie = NULL;
			h_cookie = NULL;	
			while ((r_flistp = PIN_FLIST_ELEM_GET_NEXT(calc_out_flistp, PIN_FLD_RESULTS, &p_elem_id, 1, &p_cookie, ebufp)) != NULL)
                        {
				if(base_plan == 1)
					break;

				h_cookie = NULL;	
				while ((temp_flistp = PIN_FLIST_ELEM_GET_NEXT(r_flistp, PIN_FLD_BAL_IMPACTS, &h_elem_id, 1, &h_cookie, ebufp)) != NULL)
                                {
					prod_pdp = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
					res_id = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_RESOURCE_ID, 1, ebufp);
					if(res_id && *res_id == 356)
					{
						base_plan =1;
						break;
					}
				}
				
			}
			if(base_plan ==0)
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        		"this plan doesnt have the balimpacts currency array", ebufp);
				*ret_flistp = PIN_FLIST_CREATE(ebufp);
                		PIN_FLIST_FLD_COPY(mso_act_flistp, PIN_FLD_ACCOUNT_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
                		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_service_failure, ebufp);
                		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51728", ebufp);
                		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Activating BB service- PCM_OP_SUBSCRIPTION_PURCHASE_DEAL error", ebufp);
                		flags =0;
                		goto CLEANUP;
			}
		/*	if(flags == 1 && price_calc_flag == 1)
			{ */
				/* For NON-Activation flow (flag=1) with CALC ONLY mode(price_calc_flag = 1)
					return from here(goto CLEANUP). No other action is required.
					There cannot be more than one deal in input flist for NON-Activation flow.*/
		/*		goto CLEANUP;
			} */
		}
		/* For Calc-Only - till here*/
		if(!price_calc_flag)// Executed only when price calc flag is 0
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "---> Purchase Deal Input Flist", pur_in_flistp);
			if(grace_flag == 1){
				ret_flag = fm_mso_check_grace_period_entry(ctxp, mso_act_flistp,  pur_in_flistp, ret_flistp, ebufp);
			}
			if(ret_flag)
			{
				
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "---> Purchase Deal Input Flist after adding end_t", pur_in_flistp);

				grace_flag_local = 1;
				/* Read service status */
        			read_flistp = PIN_FLIST_CREATE(ebufp);
                                PIN_FLIST_FLD_COPY(pur_in_flistp, PIN_FLD_SERVICE_OBJ, read_flistp, PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_LAST_STATUS_T, NULL, ebufp);
        			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read service input list", read_flistp);
        			PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flistp, &read_res_flistp, ebufp);
        			PIN_FLIST_DESTROY_EX(&read_flistp, NULL);
        			if (PIN_ERRBUF_IS_ERR(ebufp))
        			{
                			PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
                			*ret_flistp = PIN_FLIST_CREATE(ebufp);
                			PIN_FLIST_FLD_COPY(pur_in_flistp, PIN_FLD_SERVICE_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
                			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_service_failure, ebufp);
                			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51431", ebufp);
                			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Add Plan - PCM_OP_READ_FLDS of service poid error", ebufp);
                        		goto CLEANUP;
        			}
        			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read service output flist", read_res_flistp);
				PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);

				write_iflistp = PIN_FLIST_CREATE(ebufp);
                                PIN_FLIST_FLD_COPY(pur_in_flistp, PIN_FLD_SERVICE_OBJ, write_iflistp, PIN_FLD_POID, ebufp);
                                PIN_FLIST_FLD_COPY(pur_in_flistp, PIN_FLD_END_T, write_iflistp, PIN_FLD_LAST_STATUS_T, ebufp);
                                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "service update input flist before opcode call", write_iflistp);
                                PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_iflistp, &write_oflistp, ebufp);
                                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "service update output flist before opcode call", write_oflistp);
                                PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
                                PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchase Deal input list", pur_in_flistp);
			PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_PURCHASE_DEAL, 0, pur_in_flistp, &pur_out_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchase Deal output flist", pur_out_flistp);
			 if(PIN_ERRBUF_IS_ERR(ebufp))
        		 {
                		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SUBSCRIPTION_PURCHASE_DEAL", ebufp);
                		PIN_FLIST_DESTROY_EX(&pur_out_flistp, NULL);
			
                		pin_set_err(ebufp, PIN_ERRLOC_CM,
                                        PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_ERR_BAD_OPCODE, 0, 0);
                        	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in calling Purchase deal.", ebufp);
                        	*ret_flistp = PIN_FLIST_CREATE(ebufp);
                        	PIN_FLIST_FLD_COPY(mso_act_flistp, PIN_FLD_ACCOUNT_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
                        	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_service_failure, ebufp);
                        	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51718", ebufp);
                        	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling Purchase plan.", ebufp);
                        	goto CLEANUP;
                	}

			/* Changes for Static IP. If IS-TAL flag is set purchase the static IP plan */
			service_obj = PIN_FLIST_FLD_GET(pur_in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
			if ( service_obj && !PIN_POID_IS_NULL(service_obj))
			{
			   if(base_plan ==1) {
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"base plan is selected .need to activate static ip");	
			   }
			    if((p_type && *p_type == 1 && add_tal_plan == 0)) {
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"call static ip for subscription plan");
				apply_static_ip_charge(connp,pur_in_flistp, &pur_out_flistp, ebufp);
			    }
		 }
			/*if(ret_flag)
			{
				write_iflistp = PIN_FLIST_CREATE(ebufp);
                                PIN_FLIST_FLD_COPY(pur_in_flistp, PIN_FLD_SERVICE_OBJ, write_iflistp, PIN_FLD_POID, ebufp);
                                PIN_FLIST_FLD_COPY(read_res_flistp, PIN_FLD_LAST_STATUS_T, write_iflistp, PIN_FLD_LAST_STATUS_T, ebufp);
                                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "service update input flist after opcode call", write_iflistp);
                                PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_iflistp, &write_oflistp, ebufp);
                                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "service update output flist after opcode call", write_oflistp);
                                PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
                                PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
                                PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);

			}*/
		}
		else
		{
			pur_out_flistp = PIN_FLIST_COPY(calc_out_flistp, ebufp);
		}
		
		switch (ebufp->pin_err) {
                        case 100002:
                        case 100004:
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "lco error set");
                                lco_err_code = ebufp->pin_err;
                                break;
                        default: lco_err_code = 0;
                }

                if (lco_err_code != 0) {
                        PIN_ERRBUF_RESET(ebufp);
                        lco_err_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_COPY(mso_act_flistp,PIN_FLD_POID,lco_err_flistp,PIN_FLD_ACCOUNT_OBJ,ebufp);
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "lco error flistp", lco_err_flistp);
                }

		ERR:
		if (PIN_ERRBUF_IS_ERR(ebufp) || lco_err_code != 0)
                {
                        /*LCO Error Handling Start*/
                        if (lco_err_code != 0) {
                                fm_mso_commission_error_processing(ctxp, lco_err_code,
                                        "Error in fm_mso_cust_activate_customer", lco_err_flistp, &r_flistp, ebufp);
                        }
                        /*LCO Error Handling End*/
			else if (fm_mso_convert_ebuf_to_msg(ctxp, NULL, NULL, error_code, error_descr, ebufp) )
				 {
				 PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Commission error in calling PCM_OP_SUBSCRIPTION_PURCHASE_DEAL", ebufp);
                                 PIN_ERRBUF_RESET(ebufp);

                                 r_flistp = PIN_FLIST_CREATE(ebufp);
                                 PIN_FLIST_FLD_COPY(mso_act_flistp, PIN_FLD_ACCOUNT_OBJ, r_flistp, PIN_FLD_POID, ebufp );
                                 PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &activate_service_failure, ebufp);
                                 PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, error_code, ebufp);
                                 PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, error_descr, ebufp);

				 }
                        else {
				 PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SUBSCRIPTION_PURCHASE_DEAL", ebufp);
                                 PIN_ERRBUF_RESET(ebufp);

                                 r_flistp = PIN_FLIST_CREATE(ebufp);
                                 PIN_FLIST_FLD_COPY(mso_act_flistp, PIN_FLD_ACCOUNT_OBJ, r_flistp, PIN_FLD_POID, ebufp );
                                 PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &activate_service_failure, ebufp);
                                 PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51426", ebufp);
                                 PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Activating BB service- PCM_OP_SUBSCRIPTION_PURCHASE_DEAL error", ebufp);

                        }
                        goto CLEANUP1;
                }

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchase Deal output flist", pur_out_flistp);
			/* Populate Purchased plan array with product array from PURCHASE_DEAL output*/
			p_cookie=NULL;
			/* Plans have same Array element id (elem_id) in  pur_plan_flistp and deals_activation */
			temp_flistp = PIN_FLIST_ELEM_GET(pur_plan_flistp, PIN_FLD_DEALS, elem_id, 0, ebufp );
			while((prod_flistp = PIN_FLIST_ELEM_GET_NEXT(pur_out_flistp, PIN_FLD_PRODUCTS,
					&p_elem_id, 1, &p_cookie, ebufp)) != (pin_flist_t *)NULL)
			{
					PIN_FLIST_ELEM_SET(temp_flistp, prod_flistp, PIN_FLD_PRODUCTS, p_elem_id, ebufp );
			}

			/* Check if the purchased plan was a hardware plan*/
			pur_plan_pdp = PIN_FLIST_FLD_GET(deal_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp );
			h_elem_id=0;
			h_cookie=NULL;
			while((plan_flistp = PIN_FLIST_ELEM_GET_NEXT(hardware_flistp, PIN_FLD_PLAN,
				&h_elem_id, 1, &h_cookie, ebufp)) != (pin_flist_t *)NULL)
			{
				hw_plan_pdp = NULL;
				hw_plan_pdp = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp );
				comp_plan = PIN_POID_COMPARE(pur_plan_pdp, hw_plan_pdp, 0, ebufp );
				if(comp_plan == 0)
				{
					/* If plan poid matches then copy the results array from output flist 
					   to hardware flist. It will be used to call deposit API */
					p_elem_id=0;
					p_cookie=NULL;
					while((prod_flistp = PIN_FLIST_ELEM_GET_NEXT(pur_out_flistp, PIN_FLD_RESULTS,
						&p_elem_id, 1, &p_cookie, ebufp)) != (pin_flist_t *)NULL)
					{
						PIN_FLIST_ELEM_SET(plan_flistp, prod_flistp, PIN_FLD_RESULTS, p_elem_id, ebufp );
						PIN_FLIST_FLD_COPY(deal_flistp, PIN_FLD_NAME, plan_flistp, PIN_FLD_NAME, ebufp);
						PIN_FLIST_FLD_COPY(deal_flistp, PIN_FLD_CODE, plan_flistp, PIN_FLD_CODE, ebufp);
						
					}
				}
			}
                if (PIN_ERRBUF_IS_ERR(ebufp) && (ebufp->pin_err == 100005)) {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in calling Purchase deal.", ebufp);
                        PIN_ERRBUF_RESET(ebufp);
                        *ret_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_COPY(mso_act_flistp, PIN_FLD_ACCOUNT_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
                        PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_service_failure, ebufp);
                        PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51718", ebufp);
                        PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling Purchase plan - Due to LCO Credit Limit Breach.", ebufp);
                        goto CLEANUP;
                }

		
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in calling Purchase deal.", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			*ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(mso_act_flistp, PIN_FLD_ACCOUNT_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_service_failure, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51718", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling Purchase plan.", ebufp);
			goto CLEANUP;
		}
		
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Hardware flist after purchase", hardware_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchased plan deal info flist", pur_plan_flistp);
	
	/*if ( flags == 1 )
		goto SKIP;*/
	/* Update the status in /mso_plans_activation 
		Executed only for activation flow (flags = 0) */
	/*write_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(mso_act_flistp, PIN_FLD_POID, write_in_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(write_in_flistp, PIN_FLD_STATUS, &status_purchased, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Write flds Input Flist", write_in_flistp);
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_in_flistp, &temp_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Write flds output flist", temp_flistp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in updating mso_plans_activation status", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(mso_act_flistp, PIN_FLD_ACCOUNT_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_service_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51719", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in updating mso_plans_activation status", ebufp);
		goto CLEANUP;
	}*/
	
	if(account_obj && fm_cust_deac_act_purchased(ctxp, account_obj, 0, ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Customer is having two base plans active");
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(mso_act_flistp, PIN_FLD_ACCOUNT_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_service_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51728", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Customer is having two base plans active", ebufp);
		flags =0;
		goto CLEANUP;	
	}
		
	/*******************************************************************
	* Create /mso_purcahsed_plan object for all purchased plans.
	* Executed in All flows where CALC_ONLY mode is 0
	*******************************************************************/	
SKIP:	
	PIN_FLIST_FLD_COPY(mso_act_flistp, PIN_FLD_ACCOUNT_OBJ, pur_plan_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(mso_act_flistp, PIN_FLD_SERVICE_OBJ, pur_plan_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	fm_mso_create_purchased_plan_object(ctxp, pur_plan_flistp, bb_bp_flistp, hardware_flistp, ret_flistp, ebufp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchased plan return flist", *ret_flistp );
	if ( *ret_flistp && *ret_flistp != (pin_flist_t *)NULL )
	{
		ret_status = *(int32*)PIN_FLIST_FLD_GET(*ret_flistp, PIN_FLD_STATUS, 0, ebufp);
		if (ret_status == activate_service_failure)
		{
			goto CLEANUP;
		}
	} 
	mso_pp_flistp = PIN_FLIST_COPY(*ret_flistp, ebufp);
	PIN_FLIST_FLD_DROP(mso_pp_flistp, PIN_FLD_STATUS, ebufp);
	PIN_FLIST_DESTROY_EX(ret_flistp, NULL);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Base Plan flist after pur plan", bb_bp_flistp );
	if(bb_bp_flistp)
	{
		found = 1; 
	}	

	/*Get Non currency resource sub balances from the purchase plan return flist*/
	if(calc_out_flistp)
	{
		fm_mso_get_ncr_sub_bal(ctxp, calc_out_flistp, bb_bp_flistp, found, &ncr_sb_flistp, ebufp );
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in Getting Non Currency Sub Balance.", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			*ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(mso_act_flistp, PIN_FLD_ACCOUNT_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_service_failure, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51728", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Getting Non Currency Sub Balance", ebufp);
			goto CLEANUP;	
		}
	}

	// Call the function to add AMC plan only when flow is service activation (flag == 0)
	if ( flags == 0 )
	{
		fm_mso_cust_activate_amc_plan(ctxp, mso_act_flistp, mso_pp_flistp, ebufp );
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in associating AMC plan.", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			*ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(mso_act_flistp, PIN_FLD_ACCOUNT_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_service_failure, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51781", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in associating AMC plan", ebufp);
			goto CLEANUP;	
		}
	}

	if ( flags == 1 )
		goto DEPOSIT;

	/* call provisioning action and set FUP_STATUS */
	if(bb_bp_flistp && bb_bp_flistp != (pin_flist_t *)NULL ) {
		subs_plan_pdp=PIN_FLIST_FLD_GET(bb_bp_flistp, PIN_FLD_POID, 1, ebufp);
		if(subs_plan_pdp)
		{
				PIN_FLIST_CONCAT(mso_act_flistp, mso_pp_flistp, ebufp );
				/* Add Non currency sub balances to provisioning input flist */
				if(calc_out_flistp)
					PIN_FLIST_CONCAT(mso_act_flistp, ncr_sb_flistp, ebufp );
				/* Call Provisioning action */
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Prov action input flist", mso_act_flistp );
        		fm_mso_call_prov_action(ctxp, mso_act_flistp, subs_plan_pdp,  ret_flistp, ebufp );
        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Prov action return flist", *ret_flistp );
        		if (*ret_flistp)
        		{
                		ret_status = *(int32*)PIN_FLIST_FLD_GET(*ret_flistp, PIN_FLD_STATUS, 0, ebufp);
                		if (ret_status == activate_service_failure)
                		{
                        		goto CLEANUP;
                		}
        		}

				/* Pawan:16-02-2015 Set FUP_STATUS to 1 */
				//base_plan_pdp=PIN_FLIST_FLD_GET(bb_bp_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
        		fm_mso_set_fup_status(ctxp, mso_act_flistp, subs_plan_pdp, ret_flistp, ebufp );
        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Set FUP_STATUS return flist", *ret_flistp );
        		if (*ret_flistp)
        		{
               		ret_status = *(int32*)PIN_FLIST_FLD_GET(*ret_flistp, PIN_FLD_STATUS, 0, ebufp);
                		if (ret_status == activate_service_failure)
                		{
                        		goto CLEANUP;
                		}
        		}				
		}
	}
	
	
	/******************************************************************
	* For FIBER service, perform additional action since provisiong
	* response is not processed
	******************************************************************/
	ser_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(mso_act_flistp, PIN_FLD_SERVICE_OBJ, ser_in_flistp, PIN_FLD_POID, ebufp);
	tmp_flistp = PIN_FLIST_SUBSTR_ADD(ser_in_flistp, MSO_FLD_BB_INFO, ebufp );
	PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_TECHNOLOGY, NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Service Read flds Input Flist", ser_in_flistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, ser_in_flistp, &ser_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Service Read flds output flist", ser_out_flistp);
	
	tmp_flistp = PIN_FLIST_SUBSTR_GET(ser_out_flistp, MSO_FLD_BB_INFO, 0, ebufp );
	tech = PIN_FLIST_FLD_TAKE(tmp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp);
	PIN_FLIST_DESTROY_EX(&ser_in_flistp, NULL);
	/*if(tech && *tech == MSO_FIBER)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Fiber Service");
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_act_flistp", mso_act_flistp);
		ser_in_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(mso_act_flistp, PIN_FLD_ACCOUNT_OBJ, ser_in_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(mso_act_flistp, PIN_FLD_SERVICE_OBJ, ser_in_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(mso_act_flistp, PIN_FLD_PROGRAM_NAME, ser_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "update prov status input flist", ser_in_flistp);
		ret_val = fm_mso_update_ser_prov_status(ctxp, ser_in_flistp, MSO_PROV_ACTIVE, ebufp );
		if(ret_val)
		{
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(mso_act_flistp, PIN_FLD_ACCOUNT_OBJ, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &activate_service_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51754", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error while updating service provisioning status.", ebufp);
			goto CLEANUP;			
		}
		/* Update mso_purhcased_plan_status *
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Updating purchased plan status.");
		if(fm_mso_update_mso_purplan_status(ctxp, subs_plan_pdp, MSO_PROV_ACTIVE, ebufp ) == 0)
		{
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(mso_act_flistp, PIN_FLD_ACCOUNT_OBJ, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &activate_service_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51754", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error while updating mso_purhcased_plan status.", ebufp);
			goto CLEANUP;					
		}
		
		/* Set status as Active for each purchased product *
		fm_mso_prov_update_product_status(ctxp, 
				PIN_FLIST_FLD_GET(mso_act_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp),
				PIN_FLIST_FLD_GET(mso_act_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp),
				subs_plan_pdp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in updating Purchased product status", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(mso_act_flistp, PIN_FLD_ACCOUNT_OBJ, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &activate_service_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51729", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error in updating Purchased product status", ebufp);
			goto CLEANUP;
		}
		PIN_FLIST_DESTROY_EX(&ser_in_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&ser_out_flistp, NULL);
	}*/
	DEPOSIT:
	/*******************************************************************
	* Call Deposit API
	*******************************************************************/
	PIN_FLIST_FLD_COPY(mso_act_flistp, PIN_FLD_PROGRAM_NAME, hardware_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_COPY(mso_act_flistp, PIN_FLD_USERID, hardware_flistp, PIN_FLD_USERID, ebufp);
	PIN_FLIST_FLD_COPY(mso_act_flistp, PIN_FLD_ACCOUNT_OBJ, hardware_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(mso_act_flistp, PIN_FLD_SERVICE_OBJ, hardware_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	fm_mso_create_device_deposit(ctxp, hardware_flistp, ret_flistp, ebufp );
	//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Link device deposit return flist", *ret_flistp );
	if (*ret_flistp && *ret_flistp != (pin_flist_t *)NULL )
	{
		ret_status = *(int32*)PIN_FLIST_FLD_GET(*ret_flistp, PIN_FLD_STATUS, 0, ebufp);
		if (ret_status == activate_service_failure)
		{
			goto CLEANUP;
		}
	}
	*ret_flistp = PIN_FLIST_COPY(pur_out_flistp, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_service_success, ebufp);
	/* If Calc only flist exists then set it in DATA ARRAY of output flist.*/
	if(calc_out_flistp)
	{
		PIN_FLIST_ELEM_SET(*ret_flistp,calc_out_flistp, PIN_FLD_DATA_ARRAY, 0, ebufp);
	}
	
	CLEANUP:
	if ( flags == 1 )
	{
		//if ( price_calc_flag == 0 )
		//{
			/*Actual purchase NOT in CALC ONLY mode*/
			*ret_flistp = PIN_FLIST_COPY(pur_out_flistp, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, MSO_FLD_PROVISIONABLE_FLAG, &grace_flag_local, ebufp);
			tmp_flistp = PIN_FLIST_ELEM_GET(mso_pp_flistp, PIN_FLD_PRODUCTS, 0, 1, ebufp);
			PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, *ret_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, ebufp);
			if(ncr_sb_flistp) {
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Concat NCR balance flist.");
				PIN_FLIST_CONCAT(*ret_flistp, ncr_sb_flistp, ebufp );
			}
			if ( price_calc_flag == 1 )
			{
				PIN_FLIST_ELEM_SET(*ret_flistp, calc_out_flistp, PIN_FLD_DATA_ARRAY, 0, ebufp);
			}
		//}
		//else
		//{
		//	/*Called in CALC ONLY mode*/
		//	*ret_flistp = PIN_FLIST_COPY(calc_out_flistp, ebufp);
		//	PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, *ret_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, ebufp);
		//}
	}
	
	PIN_FLIST_DESTROY_EX(&pur_out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&bb_bp_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&write_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&pur_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&hardware_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&pur_plan_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&deal_array, NULL);
	PIN_FLIST_DESTROY_EX(&mso_pp_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&calc_out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&ser_out_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&lco_err_flistp, NULL);
	if(tech) free(tech);
	return;

	CLEANUP1:
	*ret_flistp = PIN_FLIST_COPY(r_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	 PIN_FLIST_DESTROY_EX(&pur_out_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&bb_bp_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&write_in_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&pur_in_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&hardware_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&pur_plan_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&deal_array, NULL);
        PIN_FLIST_DESTROY_EX(&mso_pp_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&calc_out_flistp, NULL);
        return;
}

static int32
fm_mso_check_grace_period_entry(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*mso_pp_flistp,
	pin_flist_t		**bb_ret_flistp,
	pin_errbuf_t		*ebufp)
{

	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*o_flistp = NULL;
	pin_flist_t		*prod_flist = NULL;
	pin_flist_t		*plan_flist = NULL;
	time_t			*grace_start_t = NULL;
	time_t			*grace_end_t = NULL;
	time_t			purchase_start_t = 0;
	char			msg[50] = "";
	int32			return_flag = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_check_grace_period_entry: input list", in_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_check_grace_period_entry: mso_pp_flistp list", mso_pp_flistp);
	fm_mso_search_grace_period_entry(ctxp, in_flistp, &o_flistp, bb_ret_flistp,  ebufp);
	if(o_flistp)
	{
		r_flistp = PIN_FLIST_ELEM_GET(o_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		if(r_flistp)
		{
			grace_end_t = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_EXIT_T, 0,ebufp);
			grace_start_t = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_PURCHASE_END_T, 0,ebufp);
			*(time_t *)grace_start_t = fm_utils_time_round_to_midnight(*(time_t *)grace_start_t);
			*(time_t *)grace_end_t = fm_utils_time_round_to_midnight(*(time_t *)grace_end_t);
			plan_flist = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_PLAN, PIN_ELEMID_ANY, 1, ebufp);
			prod_flist = PIN_FLIST_ELEM_GET(plan_flist, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, 1, ebufp);
			purchase_start_t = *(time_t*)PIN_FLIST_FLD_GET(prod_flist, PIN_FLD_PURCHASE_START_T, 0,ebufp);
			sprintf(msg," purchase_start_t grace_end_t : %d %d\n", purchase_start_t, *grace_end_t);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
			if(purchase_start_t  <= *grace_end_t)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"condition met");
				PIN_FLIST_FLD_SET(mso_pp_flistp, PIN_FLD_END_T, grace_start_t, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_check_grace_period_entry: after setting date", mso_pp_flistp);
				return_flag = 1;
			}		 
		}
		//PIN_FLIST_DESTROY_EX(&o_flistp, NULL);
	}
	return return_flag;
}

static void
fm_mso_search_grace_period_entry(
	pcm_context_t	*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		**r_flistpp,
	pin_flist_t		**bb_ret_flistp,
	pin_errbuf_t	*ebufp)
{
	int			status = 1;
	int32		database = 0;
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*s_iflistp = NULL;
	pin_flist_t	*s_oflistp = NULL;
	pin_flist_t	*result_flistp = NULL;
	poid_t		*s_pdp = NULL;
	poid_t		*pdp = NULL;
	void		*vp = NULL;
	int			s_flags = 0;
	int32		op_success = 1;
	int			status_change_failure = 1;
	char		*ptemplate = "select X from /mso_bb_grace_job where (F1 = V1 and F2 = V2 and F3 = V3) order by created_t desc ";
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	s_iflistp = PIN_FLIST_CREATE(ebufp);
	pdp = PCM_GET_USERID(ctxp);
	database = PIN_POID_GET_DB(pdp);
	s_pdp = PIN_POID_CREATE(database, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(s_iflistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_iflistp, PIN_FLD_FLAGS, &s_flags, ebufp);
	PIN_FLIST_FLD_SET(s_iflistp, PIN_FLD_TEMPLATE, ptemplate, ebufp );
	args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_ACCOUNT_OBJ, args_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PLAN_OBJ, args_flistp, PIN_FLD_PLAN_OBJ, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &op_success, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_grace_period_entry input ", s_iflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_iflistp, &s_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&s_iflistp, NULL);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		pin_set_err(ebufp, PIN_ERRLOC_CM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_ERR_BAD_OPCODE, 0, 0);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		PIN_FLIST_DESTROY_EX(&s_oflistp, NULL);
		*bb_ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, *bb_ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(*bb_ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(*bb_ret_flistp, PIN_FLD_ERROR_CODE, "51645", ebufp);
		PIN_FLIST_FLD_SET(*bb_ret_flistp, PIN_FLD_ERROR_DESCR, "Error in searching /mso_bb_grace_job for this account", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_grace_period_entry output ", s_oflistp);
	*r_flistpp = PIN_FLIST_COPY(s_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&s_oflistp, NULL);
}

/******************************************************
* Function: fm_mso_set_bb_service_status()
* Description: Purchases each plan for an account 
*			 which is specified in mso_act_flistp.
* 			
*******************************************************/

static void
fm_mso_set_bb_service_status(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*ser_in_flistp = NULL;
	pin_flist_t		*ser_out_flistp = NULL;
	pin_flist_t		*ser_flistp = NULL;
	pin_flist_t		*status_flistp = NULL;
	int				service_status_active = PIN_STATUS_ACTIVE;
	int				status_flag = 0;
	int32			activate_service_failure = 1;
	int32			activate_service_success = 0;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_set_bb_service_status", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_set_bb_service_status input flist", in_flistp );
	ser_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_OBJ, ser_in_flistp, PIN_FLD_POID, ebufp);
	
	ser_flistp = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_SERVICES, 0, 1, ebufp);
	PIN_FLIST_FLD_COPY(ser_flistp, PIN_FLD_SERVICE_OBJ, ser_in_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, ser_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	
	status_flistp = PIN_FLIST_ELEM_ADD(ser_in_flistp, PIN_FLD_STATUSES, 0, ebufp );
	PIN_FLIST_FLD_SET(status_flistp, PIN_FLD_STATUS,&service_status_active , ebufp);
	PIN_FLIST_FLD_SET(status_flistp, PIN_FLD_STATUS_FLAGS, &status_flag, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "---> Set status Input Flist", ser_in_flistp);
	PCM_OP(ctxp, PCM_OP_CUST_SET_STATUS, 0, ser_in_flistp, &ser_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Set status output flist", ser_out_flistp);
	PIN_FLIST_DESTROY_EX(&ser_in_flistp, NULL);

	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SET STATUS", ebufp);
		return;
	}
	
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&ser_out_flistp, NULL);
	return;
}

/*********************************************************************
*  Function: fm_mso_create_purchased_plan_object
*  Description: Creates an object of type /mso_purchased_plan
*				to store all the plans purchased for account 
*				and corresponding products.
**********************************************************************/
extern void
fm_mso_create_purchased_plan_object(
	pcm_context_t		*ctxp,
	pin_flist_t		*pur_plan_flistp,
	pin_flist_t		*bb_bp_flistp,
	pin_flist_t		*hardware_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	int64			db = -1;
	pin_flist_t		*obj_in_flistp = NULL;
	pin_flist_t		*obj_out_flistp = NULL;
	pin_flist_t		*deal_flistp  = NULL;
	pin_flist_t		*prod_flistp = NULL;
	pin_flist_t		*pp_flistp = NULL;
	pin_flist_t		*priority_flistp = NULL;
	pin_flist_t		*priority_oflistp = NULL;
	pin_flist_t		*mso_pp_flistp = NULL;

	poid_t			*acc_pdp = NULL;
	poid_t			*mso_pur_plan_pdp = NULL;
	poid_t			*bb_plan_obj = NULL;
	poid_t			*bb_bp_plan_obj = NULL;
	int32			activate_customer_failure = 1;
	int32			activate_customer_success = 0;	
	int			elem_id = 0;
	int			p_elem_id = 0;
	int			elem_ctr = 0;
	int			prov_in_progress = 1;
	int			prov_active = 2;
	int			base_plan_found = 0;
	char			*prov_tag = NULL;
	pin_decimal_t		*priority = NULL;
	pin_decimal_t		*priority_val = NULL;
	pin_cookie_t		cookie = NULL;
	pin_cookie_t		p_cookie = NULL;
	double			prio_double = 0.0;
	int32			hw_plan_type = 0;
	int32			*bill_when = NULL;
	int32			srv_bill_when = 0;
	poid_t			*srv_pdp = NULL;
	pin_decimal_t           *priority_amc = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_create_purchased_plan_object", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_purchased_plan_object input flist", pur_plan_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_purchased_plan_object temp flist", bb_bp_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_purchased_plan_object harware flist", hardware_flistp);

	
	acc_pdp = PIN_FLIST_FLD_GET(pur_plan_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	db = PIN_POID_GET_DB(acc_pdp);
	mso_pp_flistp = PIN_FLIST_CREATE(ebufp);
	while((deal_flistp = PIN_FLIST_ELEM_GET_NEXT(pur_plan_flistp, PIN_FLD_DEALS,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		obj_in_flistp = PIN_FLIST_CREATE(ebufp);
		mso_pur_plan_pdp = PIN_POID_CREATE(db, "/mso_purchased_plan", (int64)-1, ebufp);
		PIN_FLIST_FLD_PUT(obj_in_flistp, PIN_FLD_POID, mso_pur_plan_pdp,ebufp);
		PIN_FLIST_FLD_COPY(pur_plan_flistp,PIN_FLD_ACCOUNT_OBJ, obj_in_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(pur_plan_flistp,PIN_FLD_SERVICE_OBJ, obj_in_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
		bill_when = PIN_FLIST_FLD_GET(pur_plan_flistp, PIN_FLD_BILL_WHEN, 1, ebufp);
		/*set bill_when */
		if(bill_when)
		{
			PIN_FLIST_FLD_COPY(pur_plan_flistp,PIN_FLD_BILL_WHEN, obj_in_flistp, PIN_FLD_BILL_WHEN, ebufp);
		}
		else
		{
			srv_pdp = PIN_FLIST_FLD_GET(pur_plan_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
			srv_bill_when = get_bill_when_from_service(ctxp, srv_pdp, ebufp);
			PIN_FLIST_FLD_SET(obj_in_flistp, PIN_FLD_BILL_WHEN, &srv_bill_when, ebufp);
		} 
		PIN_FLIST_FLD_COPY(deal_flistp,PIN_FLD_PLAN_OBJ, obj_in_flistp, PIN_FLD_PLAN_OBJ, ebufp);
		bb_plan_obj = PIN_FLIST_FLD_GET(deal_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
		if ( bb_plan_obj && !PIN_POID_IS_NULL(bb_plan_obj))
		{
			if ( bb_bp_flistp && bb_bp_flistp != (pin_flist_t *)NULL )
			{
				bb_bp_plan_obj = PIN_FLIST_FLD_GET(bb_bp_flistp,PIN_FLD_PLAN_OBJ, 1, ebufp);
				if ( bb_bp_plan_obj && (PIN_POID_COMPARE( bb_bp_plan_obj,bb_plan_obj,0,ebufp ) == 0)  )
				{
					PIN_FLIST_FLD_COPY(bb_bp_flistp, PIN_FLD_DESCR, obj_in_flistp, PIN_FLD_DESCR, ebufp);
					base_plan_found = 1;
				}
			}
		}
		PIN_FLIST_FLD_SET(obj_in_flistp, PIN_FLD_STATUS, &prov_active, ebufp); 

		p_elem_id = 0;
		p_cookie = NULL;
		while((prod_flistp = PIN_FLIST_ELEM_GET_NEXT(deal_flistp, PIN_FLD_PRODUCTS,
		&p_elem_id, 1, &p_cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			pp_flistp = PIN_FLIST_ELEM_ADD(obj_in_flistp, PIN_FLD_PRODUCTS, p_elem_id, ebufp);
			PIN_FLIST_FLD_COPY(prod_flistp,PIN_FLD_PRODUCT_OBJ, pp_flistp, PIN_FLD_PRODUCT_OBJ, ebufp);
			priority_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(priority_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(prod_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(priority_flistp, PIN_FLD_PRIORITY, (pin_decimal_t *)NULL, ebufp);
			PIN_FLIST_FLD_SET(priority_flistp, PIN_FLD_PROVISIONING_TAG, (char *)NULL, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Reading Product priority input flist", priority_flistp);
			PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, priority_flistp, &priority_oflistp, ebufp);	
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Reading Product priority output flist", priority_oflistp);
			if ( !priority_oflistp || PIN_ERRBUF_IS_ERR(ebufp) )
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Product does not exist ", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				*ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acc_pdp, ebufp );
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51634", ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Product does not exist", ebufp);
				goto CLEANUP;
			} 	
			priority = PIN_FLIST_FLD_GET(priority_oflistp, PIN_FLD_PRIORITY, 1, ebufp);
			prov_tag = PIN_FLIST_FLD_GET(priority_oflistp, PIN_FLD_PROVISIONING_TAG, 1, ebufp);
			if(priority)
                        {
                           prio_double = pbo_decimal_to_double(priority, ebufp);
                           if(prio_double == BB_HW_CABLE_MODEM_RENT_PRIO
                                                                || prio_double == BB_HW_DCM_RENT_PRIO)
                           {
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "AMC Product");
                                priority_amc = priority;
                           }

                           else
                           {
                              PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "NOT AMC Product");
                              priority_val = priority;
                           }
                        }

			PIN_FLIST_FLD_COPY(prod_flistp,PIN_FLD_OFFERING_OBJ, pp_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, ebufp);
			if ( prov_tag && strcmp(prov_tag,"") != 0 )
			{
				PIN_FLIST_FLD_SET(pp_flistp, PIN_FLD_PROVISIONING_TAG, prov_tag, ebufp);
				/*Pawan:20-01-2015: For subscription plan, set the prov status to In-Progress */
				PIN_FLIST_FLD_SET(obj_in_flistp, PIN_FLD_STATUS, &prov_in_progress, ebufp); 
			}
		}
		if(priority_amc != NULL)
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "AMC PLAN CHECK2");
                        PIN_FLIST_FLD_SET(obj_in_flistp, PIN_FLD_PRIORITY, priority_amc, ebufp);
                        priority_amc = NULL;
                }
                else
                {
                	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "OTHERTHAN AMC PLAN CHECK2");
			PIN_FLIST_FLD_SET(obj_in_flistp, PIN_FLD_PRIORITY, priority_val, ebufp);
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchased Plan CREATE_OBJ input flist", obj_in_flistp);
		PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, obj_in_flistp, &obj_out_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchased Plan CREATE_OBJ output flist", obj_out_flistp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Can't create /mso_purchased_plan object ", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			*ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acc_pdp, ebufp );
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51721", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Can't create Purchased Plan object", ebufp);
			goto CLEANUP;
		}
		PIN_FLIST_DESTROY_EX(&obj_in_flistp, NULL);
		if(base_plan_found)
		{
			base_plan_found = 0;
			PIN_FLIST_FLD_COPY(obj_out_flistp, PIN_FLD_POID, bb_bp_flistp, PIN_FLD_POID, ebufp);
		}
		else
		{
			pp_flistp = PIN_FLIST_ELEM_ADD(mso_pp_flistp, PIN_FLD_PRODUCTS, elem_id, ebufp);
			PIN_FLIST_FLD_COPY(obj_out_flistp, PIN_FLD_POID, pp_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, ebufp);
		}

		
		// Add plans flist to device for hardware plans
		//prio_double = pbo_decimal_to_double(priority_val, ebufp);
			if((prio_double >= BB_HW_RENTAL_RANGE_START && prio_double <= BB_HW_RENTAL_RANGE_END) || 
				(prio_double >= BB_HW_DEPOSIT_RANGE_START && prio_double <= BB_HW_DEPOSIT_RANGE_END)) {
			    hw_plan_type = RENTAL_DEPOSIT_PLAN;
			} else if(prio_double >= BB_HW_OUTRIGHT_RANGE_START && prio_double <= BB_HW_OUTRIGHT_RANGE_END) {
			    hw_plan_type = OUTRIGHT_PLAN;
			}
		if(hw_plan_type == RENTAL_DEPOSIT_PLAN || hw_plan_type == OUTRIGHT_PLAN) {
			add_plans_to_device(ctxp, deal_flistp, hardware_flistp,obj_out_flistp, hw_plan_type, ebufp);
		}
			PIN_FLIST_DESTROY_EX(&obj_out_flistp, NULL);		

	}
	PIN_FLIST_FLD_SET(mso_pp_flistp, PIN_FLD_STATUS, &activate_customer_success, ebufp);
	*ret_flistp = PIN_FLIST_COPY(mso_pp_flistp,ebufp);
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&obj_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&obj_out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&priority_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&priority_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&mso_pp_flistp, NULL);
	return;
}	

/******************************************************
* Function: fm_mso_get_services_info()
* Description: Return the count and services array
*			for active and inactive service for an 
* 			account. Service type specified as input.
*******************************************************/

static int32
fm_mso_get_services_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**service_array,
	char			*ser_type,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	
	int64			db = -1;
	int32			srch_flag = 256;
	int32			count_bb = 0;
	int				service_status_active = PIN_STATUS_ACTIVE;
	int				service_status_inactive = PIN_STATUS_INACTIVE;

	poid_t			*srch_pdp = NULL;
	poid_t			*acnt_pdp = NULL;
	poid_t			*ser_pdp = NULL;
	char			*template = "select X from /service where F1.id = V1 and ( F2 = V2 or F3 = V3 ) and F4.type like V4 ";

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_services_info", ebufp);
	}
	
	acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG,"acnt_pdp", acnt_pdp);
	db = PIN_POID_GET_DB(acnt_pdp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
	
	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);
	
	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STATUS, &service_status_active, ebufp);
	
	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STATUS, &service_status_inactive, ebufp);
	
	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp );
	ser_pdp = PIN_POID_CREATE(db, ser_type, 1, ebufp);
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, ser_pdp, ebufp);
	PIN_POID_DESTROY(ser_pdp, NULL);
	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STATUS, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "BB Service Search input flist", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, service_array, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "BB Service Search output flist", *service_array);

	count_bb = PIN_FLIST_ELEM_COUNT(*service_array, PIN_FLD_RESULTS, ebufp);
	
	CLEANUP:
	return count_bb;
}

/*********************************************************************
*  Function: fm_mso_create_device_deposit
*  Description: Call Deposit API
*
*
**********************************************************************/
static void
fm_mso_create_device_deposit(
	pcm_context_t		*ctxp,
	pin_flist_t		*hardware_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*dep_in_flistp = NULL;
	pin_flist_t		*dep_out_flistp = NULL;
	pin_flist_t		*ser_flistp  = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*bi_flistp  = NULL;
	pin_flist_t		*plan_flistp  = NULL;
	pin_flist_t		*args_flistp  = NULL;
	poid_t			*acc_pdp = NULL;
	poid_t			*mso_pur_plan_pdp = NULL;
	poid_t			*item_pdp = NULL;
	int32			activate_customer_failure = 1;
	int			elem_id = 0;
	int			r_elem_id = 0;
	int			b_elem_id = 0;
	int			dep_result = 0;
	char		*item_type = NULL;
	char		deposit_item_type[]="/item/mso_hw_deposit";
	//char		st_item_type[]="/item/Service_Tax";
	pin_cookie_t		cookie = NULL;
	pin_cookie_t		r_cookie = NULL;
	pin_cookie_t		b_cookie = NULL;
	void			*vp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_create_device_deposit", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_device_deposit HARDWARE flist", hardware_flistp);
	dep_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(hardware_flistp,PIN_FLD_ACCOUNT_OBJ, dep_in_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(hardware_flistp,PIN_FLD_PROGRAM_NAME, dep_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	//ser_flistp = PIN_FLIST_ELEM_GET(hardware_flistp, PIN_FLD_SERVICES, 0, 1, ebufp);
	PIN_FLIST_FLD_COPY(hardware_flistp, PIN_FLD_SERVICE_OBJ, dep_in_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(hardware_flistp, PIN_FLD_USERID, dep_in_flistp, PIN_FLD_USERID, ebufp);
	/* One plan can have only one deposit product so only one result array
		can have item type for deposit. */	
	while((plan_flistp = PIN_FLIST_ELEM_GET_NEXT(hardware_flistp, PIN_FLD_PLAN,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		dep_result = -1;
		r_cookie = NULL;
		while((res_flistp = PIN_FLIST_ELEM_GET_NEXT(plan_flistp, PIN_FLD_RESULTS,
			&r_elem_id, 1, &r_cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			b_cookie = NULL;
			while((bi_flistp = PIN_FLIST_ELEM_GET_NEXT(res_flistp, PIN_FLD_BAL_IMPACTS,
				&b_elem_id, 1, &b_cookie, ebufp)) != (pin_flist_t *)NULL)
			{
				item_pdp = NULL;
				item_pdp = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_ITEM_OBJ, 1,ebufp);
				if(item_pdp) {
					item_type = (char *)PIN_POID_GET_TYPE(item_pdp);
					if(item_type && strncmp(item_type,deposit_item_type,strlen(deposit_item_type))==0) {
						dep_result = r_elem_id;
					}
				}
			}
		}
		/* if Deposit balance impact if found then copy it in deposit input flist */
		if(dep_result != -1)
		{
			PIN_FLIST_FLD_COPY(plan_flistp, PIN_FLD_NAME, dep_in_flistp, PIN_FLD_NAME, ebufp);
			PIN_FLIST_FLD_COPY(plan_flistp, PIN_FLD_CODE, dep_in_flistp, PIN_FLD_CODE, ebufp);
			PIN_FLIST_ELEM_COPY(plan_flistp, PIN_FLD_RESULTS, dep_result, dep_in_flistp, PIN_FLD_RESULTS, 0, ebufp );

			// Added below IF condition to add DEVICE array in Add HW plan flow to fix the issue
			// related to deposit not getting created. ELSE part is invokded in activation flow.
			vp=NULL;
			vp = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_DEVICE_ID, 1, ebufp);
			if(vp) {
				// Add HW plan flow
				args_flistp = PIN_FLIST_ELEM_ADD(dep_in_flistp, PIN_FLD_DEVICES, 0, ebufp );
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DEVICE_ID, vp, ebufp);
			} else {
				// Activation flow
				PIN_FLIST_ELEM_COPY(plan_flistp, PIN_FLD_DEVICES, 0, dep_in_flistp, PIN_FLD_DEVICES, 0, ebufp );
			}
		
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_PYMT_CREATE_DEPOSIT input flist", dep_in_flistp);
			PCM_OP(ctxp, MSO_OP_PYMT_CREATE_DEPOSIT, 0, dep_in_flistp, &dep_out_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_PYMT_CREATE_DEPOSIT output flist", dep_out_flistp);
		}
	}
		
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&dep_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&dep_out_flistp, NULL);
	return;
}	

/*********************************************************************
*  Function: fm_mso_set_bb_prov_status
*  Description: Updates the provisioning status of broadband
*				service with the value passed in input.
*				
**********************************************************************/
static int
fm_mso_set_bb_prov_status(
	pcm_context_t	*ctxp,
	poid_t			*ser_pdp,
	int			status,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*mod_in_flistp = NULL;
	pin_flist_t		*mod_out_flistp = NULL;
	pin_flist_t		*inher_flistp  = NULL;
	pin_flist_t		*tf_flistp = NULL;
	int			result = -1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_set_bb_prov_status", ebufp);
	}
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_set_bb_prov_status service POID", ser_pdp);

	mod_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(mod_in_flistp,PIN_FLD_POID, ser_pdp, ebufp);
	inher_flistp = PIN_FLIST_SUBSTR_ADD(mod_in_flistp, PIN_FLD_INHERITED_INFO, ebufp );
	tf_flistp = PIN_FLIST_ELEM_ADD(inher_flistp, PIN_FLD_TELCO_FEATURES, 0, ebufp );
	PIN_FLIST_FLD_SET(tf_flistp, PIN_FLD_STATUS, &status, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_CUST_MODIFY_SERVICE input flist", mod_in_flistp);
	PCM_OP(ctxp, PCM_OP_CUST_MODIFY_SERVICE, 0, mod_in_flistp, &mod_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_CUST_MODIFY_SERVICE output flist", mod_out_flistp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Can't update provisioning status in service. ", ebufp);
		result = 1;
	}
	else 
	{
		result = 0;
	}
	
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&mod_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&mod_out_flistp, NULL);
	return result;
}	

/*********************************************************************
*  Function: fm_mso_call_prov_action
*  Description:
*
*
**********************************************************************/
static void
fm_mso_call_prov_action(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	poid_t			*subs_plan_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t     *pro_in_flistp = NULL;
	pin_flist_t     *pro_out_flistp = NULL;
	pin_flist_t     *ser_flistp = NULL;
	pin_flist_t     *plan_flistp = NULL;	
	int		*prov_call = NULL;
	int		action_flag = DOC_BB_ACTIVATION;
	int		elem_id = 0;
	int32		activate_customer_failure = 1;
	int32		status_flags = 0;
	pin_cookie_t	cookie = NULL;


	/*** Pavan Bellala 21-July-2015 *****
	 Added tech field for notifications
	************************************/
	int32 		*tech = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_call_prov_action", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_call_prov_action input flist", in_flistp);

	pro_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_OBJ, pro_in_flistp, PIN_FLD_POID, ebufp);
	//ser_flistp = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_SERVICES, 0, 1, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SERVICE_OBJ, pro_in_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_SET(pro_in_flistp, PIN_FLD_FLAGS, &action_flag, ebufp);
	PIN_FLIST_FLD_SET(pro_in_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, subs_plan_pdp, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, pro_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_USERID, pro_in_flistp, PIN_FLD_USERID, ebufp);
	while((plan_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_PRODUCTS,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		PIN_FLIST_ELEM_COPY(in_flistp, PIN_FLD_PRODUCTS, elem_id, pro_in_flistp, PIN_FLD_PRODUCTS, elem_id, ebufp );
	}
	elem_id = 0;
	cookie = NULL;
	while((plan_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_SUB_BAL_IMPACTS,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		PIN_FLIST_ELEM_COPY(in_flistp, PIN_FLD_SUB_BAL_IMPACTS, elem_id, pro_in_flistp, PIN_FLD_SUB_BAL_IMPACTS, elem_id, ebufp );
	}
	/************* Call Provisioning action ****************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_PROV_CREATE_ACTION input flist", pro_in_flistp);
	PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION, 0, pro_in_flistp, &pro_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_PROV_CREATE_ACTION output flist", pro_out_flistp);
	//PIN_FLIST_DESTROY_EX(&pro_in_flistp, NULL);
	
	if(pro_out_flistp)
	{
		prov_call = PIN_FLIST_FLD_GET(pro_out_flistp, PIN_FLD_STATUS, 1, ebufp);
		if(prov_call && (*prov_call == 1))
		{
			*ret_flistp = PIN_FLIST_COPY(pro_out_flistp, ebufp);
			goto CLEANUP;
		}
	}
	else
	{
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_OBJ, *ret_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51751", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Provisioning action.", ebufp);
		goto CLEANUP;
	}
	PIN_FLIST_DESTROY_EX(&pro_out_flistp, NULL);
	
	/************* Call notification ****************/
	/* Pavan Bellala - 21-July-2015 
	** Added Notification only for Fiber customers **/
	/*tech = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp); 
	if(tech && *tech == MSO_FIBER)
	{
		status_flags = NTF_WELCOME_NOTE;
		PIN_FLIST_FLD_SET(pro_in_flistp, PIN_FLD_FLAGS, &status_flags, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Notification input list", pro_in_flistp);
		PCM_OP(ctxp, MSO_OP_NTF_CREATE_NOTIFICATION , 0, pro_in_flistp, &pro_out_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&pro_in_flistp, NULL);
		
		if(pro_out_flistp)
		{
			prov_call = PIN_FLIST_FLD_GET(pro_out_flistp, PIN_FLD_STATUS, 1, ebufp);
			if(prov_call && (*prov_call == 1))
		{
			*ret_flistp = PIN_FLIST_COPY(pro_out_flistp, ebufp);
			goto CLEANUP;
		}
		}
		else
		{
			*ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_OBJ, *ret_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51752", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Provisioning notification.", ebufp);
			goto CLEANUP;
		}
	
	}*/
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&pro_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&pro_out_flistp, NULL);
	return;
}


/*********************************************************************
*  Function: fm_mso_cust_bb_set_cr_limit
*  Description: Sets credit limit for currency and 
*				non currency resources
*
**********************************************************************/
static void
fm_mso_cust_bb_set_cr_limit(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*cl_in_flistp = NULL;
	pin_flist_t		*cl_out_flistp = NULL;
	pin_flist_t		*ser_flistp = NULL;
	pin_flist_t		*limit_flistp = NULL;
	pin_decimal_t	*zero = NULL;
	int				i = 0;
	int32			activate_service_failure = 1;
	int32			activate_service_success = 0;
	int64			res_arr[] = {356,1100010,1000103,1000104,1000105};
	char			tmp[100]="";
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_cust_bb_set_cr_limit", ebufp);
	}
	zero = pbo_decimal_from_str("0", ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_bb_set_cr_limit input flist", in_flistp );
	cl_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_OBJ, cl_in_flistp, PIN_FLD_POID, ebufp);
	ser_flistp = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_SERVICES, 0, 1, ebufp);
	PIN_FLIST_FLD_COPY(ser_flistp, PIN_FLD_SERVICE_OBJ, cl_in_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, cl_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	
	for(i=0;i<(sizeof(res_arr)/sizeof(int64));i++)
	{
		limit_flistp = PIN_FLIST_ELEM_ADD(cl_in_flistp, PIN_FLD_LIMIT, res_arr[i], ebufp );
		PIN_FLIST_FLD_SET(limit_flistp, PIN_FLD_CREDIT_LIMIT,(pin_decimal_t *)NULL , ebufp);
		PIN_FLIST_FLD_SET(limit_flistp, PIN_FLD_CREDIT_FLOOR, (pin_decimal_t *)NULL, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "---> Set Limit Input Flist", cl_in_flistp);
		PCM_OP(ctxp, PCM_OP_BILL_SET_LIMIT_AND_CR, 0, cl_in_flistp, &cl_out_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Set Limit output flist", cl_out_flistp);	
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SET LIMIT", ebufp);
			*ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_OBJ, *ret_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_service_failure, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51789", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in setting Credit Limit.", ebufp);
			goto CLEANUP;
		}
		PIN_FLIST_ELEM_DROP(cl_in_flistp, PIN_FLD_LIMIT, res_arr[i], ebufp );
	}
	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_OBJ, *ret_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_service_success, ebufp);
	
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&cl_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&cl_out_flistp, NULL);
	return;
	
}

/*********************************************************************
*  Function: fm_mso_set_fup_status
*  Description: Sets the FUP_STAUS to 1 for unlimited FUP plans
*
**********************************************************************/
void
fm_mso_set_fup_status(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	poid_t			*subs_plan_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*read_in_flistp = NULL;
	pin_flist_t		*read_out_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_flist_t		*prod_flistp = NULL;
	int64			db = 0;
	int32			flags = 512;
	int32			one = 1;
	int32			zero = 0;
	int32			success = 0;
	int32			failure = 1;
	int32			status = 0;
	int32			*quota_flag = NULL;
	poid_t			*srch_pdp = NULL;
	poid_t			*acnt_pdp = NULL;
	char			blank[4] = "";
	char			*template = " select X from /mso_config_bb_pt 1, /mso_purchased_plan 2 where 1.F1 = 2.F2 and 2.F3 = V3 ";
	pin_flist_t		*arg_flistp1 = NULL;
	pin_flist_t             *arg_flistp2 = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_set_fup_status", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_set_fup_status input flist", in_flistp);
	acnt_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	db = PIN_POID_GET_DB(acnt_pdp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	read_in_flistp = PIN_FLIST_CREATE(ebufp);

	PIN_FLIST_FLD_PUT(read_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(read_in_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(read_in_flistp, PIN_FLD_TEMPLATE, template, ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(read_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_PROVISIONING_TAG, blank, ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(read_in_flistp, PIN_FLD_ARGS, 2, ebufp );
	prod_flistp = PIN_FLIST_ELEM_ADD(arg_flistp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_PROVISIONING_TAG, blank, ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(read_in_flistp, PIN_FLD_ARGS, 3, ebufp );
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, subs_plan_pdp, ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(read_in_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, NULL, ebufp);
	//PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_ISQUOTA_FLAG, NULL, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_TYPE, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "FUP status search input flist", read_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, read_in_flistp, &read_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&read_in_flistp, NULL);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "FUP status search output flist", read_out_flistp);

	arg_flistp = PIN_FLIST_ELEM_GET(read_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp );
	if(arg_flistp) {
		//quota_flag = PIN_FLIST_FLD_GET(arg_flistp, MSO_FLD_ISQUOTA_FLAG, 0, ebufp);
		quota_flag = PIN_FLIST_FLD_GET(arg_flistp, MSO_FLD_TYPE, 0, ebufp);
	}
	
	if(quota_flag)
	{
		read_in_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_OBJ, read_in_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, read_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
		arg_flistp = PIN_FLIST_ELEM_ADD(read_in_flistp, PIN_FLD_SERVICES, 1,ebufp );
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SERVICE_OBJ, arg_flistp, PIN_FLD_POID, ebufp);
		arg_flistp1 = PIN_FLIST_SUBSTR_ADD(arg_flistp, PIN_FLD_INHERITED_INFO, ebufp );
		arg_flistp2 = PIN_FLIST_SUBSTR_ADD(arg_flistp1, MSO_FLD_BB_INFO, ebufp );
		if(*quota_flag == TYPE_FUP)
		{
			status=BEF_FUP;
			PIN_FLIST_FLD_SET(arg_flistp2, MSO_FLD_FUP_STATUS, &status, ebufp);
		}
		else
		{
			status=FUP_NA;
			PIN_FLIST_FLD_SET(arg_flistp2, MSO_FLD_FUP_STATUS, &status, ebufp);
		}
	
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "FUP status Update services input flist", read_in_flistp);
		PCM_OP(ctxp, PCM_OP_CUST_UPDATE_SERVICES, 0, read_in_flistp, &read_out_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&read_in_flistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling Write flds", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			*ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_OBJ, *ret_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51790", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling Update services for updating FUP status.", ebufp);
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "FUP status Update services output flist", read_out_flistp);
	}
	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_OBJ, *ret_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &success, ebufp);
}
/******************************************
* fm_mso_pur_deal_calc_only()
*		Purchases deal in calc only mode
*******************************************/
static void
fm_mso_pur_deal_calc_only(
         cm_nap_connection_t     *connp,
	pin_flist_t			*pd_flistp, 
	pin_flist_t			**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	       pcm_context_t           *ctxp = connp->dm_ctx;
	pin_flist_t			*deal_info_flistp = NULL;
	pin_flist_t			*prod_flistp = NULL;
	pin_flist_t			*pur_in_flistp = NULL;
	pin_flist_t			*ser_in_flistp = NULL;
	pin_flist_t			*ser_out_flistp = NULL;
	pin_flist_t			*ser_flistp = NULL;
	pin_flist_t			*in_flistp = NULL;
	poid_t				*inp_pdp = NULL;
	int32				status_active = 1;
	int32				local = -1;
	int32				activate_service_failure = 1;
	int					elem_id = 0;
	pin_cookie_t		cookie = NULL;
	void                    *vp = NULL;
	int32                   price_calc_flag = 0;
	int32			*ret_plan_type = NULL;
	int32			add_tal_plan = 0;
	//pcm_context_t   *ctxp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_pur_deal_calc_only", ebufp);
			return;
	}
	PIN_ERRBUF_CLEAR(ebufp);
	
	//PCM_CONTEXT_OPEN(&ctxp, (pin_flist_t *)0, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_pur_deal_calc_only input flist", pd_flistp);
	vp = PIN_FLIST_FLD_TAKE(pd_flistp, PIN_FLD_MODE, 1, ebufp );
	//For Tal ip ADD Plan get MSO_FLD_PLAN_TYPE
	ret_plan_type = PIN_FLIST_FLD_TAKE(pd_flistp, MSO_FLD_PLAN_TYPE, 1, ebufp );
        if(ret_plan_type && *ret_plan_type == 7) {
                add_tal_plan = 1;
        }

	in_flistp =  PIN_FLIST_COPY(pd_flistp,ebufp);
	inp_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
    if(vp && *(int32 *)vp == 1) {
    	price_calc_flag = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Price Calc Flag (PIN_FLD_MODE) is 1");
    } else {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Price Calc Flag (PIN_FLD_MODE) is 0");
	}
	//local = fm_mso_trans_open(ctxp, inp_pdp, READWRITE,ebufp);
	
	ser_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, ser_in_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, ser_in_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, ser_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	ser_flistp = PIN_FLIST_ELEM_ADD(ser_in_flistp, PIN_FLD_SERVICES, 0, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SERVICE_OBJ, ser_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	fm_mso_set_bb_service_status(ctxp,ser_in_flistp,&ser_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&ser_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&ser_out_flistp, NULL);
	
	pur_in_flistp = PIN_FLIST_COPY(in_flistp, ebufp);
/*
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Unable to open Local Transaction: Error", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			*r_flistpp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, inp_pdp, ebufp );
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &activate_service_failure, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "51413", ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, "Unable to open Local Transaction: Error", ebufp);
			return;
	}
	if ( local == LOCAL_TRANS_OPEN_FAIL )
	{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Unable to open Local Transaction: Already Open", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			*r_flistpp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, inp_pdp, ebufp );
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &activate_service_failure, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "51414", ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, "Unable to open Local Transaction: Already Open", ebufp);
			return;
	}
	*/
	deal_info_flistp = PIN_FLIST_SUBSTR_GET(pur_in_flistp, PIN_FLD_DEAL_INFO, 0, ebufp);
	/* Set the status to Active in every product array */
	while((prod_flistp = PIN_FLIST_ELEM_GET_NEXT(deal_info_flistp, PIN_FLD_PRODUCTS,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_STATUS, &status_active, ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "---> Purchase Deal Input Flist", pur_in_flistp);
	PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_PURCHASE_DEAL, PCM_OPFLG_CALC_ONLY, pur_in_flistp, r_flistpp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchase Deal output flist", *r_flistpp);
	/*
	if(local == LOCAL_TRANS_OPEN_SUCCESS )
	{
			fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "After Trans Abort");
	}
	PCM_CONTEXT_CLOSE(ctxp, 0, ebufp);
	*/
		//No need to call below function for MSO_OP_CUST_ADD_PLAN
	if (price_calc_flag == 1 && add_tal_plan == 0) {
        calc_static_ip_charge(
                                        connp,
                                        pd_flistp,
                                        r_flistpp,
                                        ebufp);
	}

	 if(local == LOCAL_TRANS_OPEN_SUCCESS )
        {
                        fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "After Trans Abort");
        }

       if (PIN_ERRBUF_IS_ERR(ebufp))
        {

                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_pur_deal_calc_only for static IP", ebufp);
                        return;
        }
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchase Deal Calc Only return flist", *r_flistpp);	
CLEANUP:	

	/* Changes for static ip charging . Check if is_tal flag is set */
	PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&ser_in_flistp , NULL);
	PIN_FLIST_DESTROY_EX(&ser_out_flistp , NULL);
	PIN_FLIST_DESTROY_EX(&pur_in_flistp,  NULL);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Returning...");
	return;
}

/***************************************************
* fm_mso_get_ncr_sub_bal()
*		This function finds the sub balances
*		for resource id 1000010 and 1000104 (if any)
*		in input flist and copies to return flist
****************************************************/
void
fm_mso_get_ncr_sub_bal(
	pcm_context_t 		*ctxp, 
	pin_flist_t			*in_flistp,
	pin_flist_t			*rr_flistpp,
	int32                     	foundd,
	pin_flist_t			**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t			*data_flistp = NULL;
	pin_flist_t			*res_flistp = NULL;
	pin_flist_t			*sb_flistp = NULL;
	int					elem_id = 0;
	int					sb_elem_id = 0;
	int					found = 0;
	int					ctr = 0;
	int32				*res_id = NULL;
	pin_cookie_t		cookie = NULL;
	pin_cookie_t		sb_cookie = NULL;
	char				tmp[200]="";
	char                            tmp1[200]="";
	pin_flist_t		*prod_flist = NULL;
	int			elem_id_1 = 0;
	pin_cookie_t		cookie_1 = NULL;
	pin_flist_t		*mso_prod_obj = NULL;
	pin_flist_t		*read_mso_plan_flistp = NULL;
	pin_flist_t		*read_mso_plan_oflistp = NULL;
	pin_flist_t		*prod_flistp = NULL;
	char			*prov_tag = NULL;	
	pin_flist_t		*service_obj = NULL;
	int			rec_id_p = 0;
	pin_cookie_t		cookie_p = NULL;
	pin_flist_t		*rfld_iflistp = NULL;
	pin_flist_t		*temp_flistp = NULL;
	pin_flist_t		*rfld_oflistp = NULL;
	int32			*tech = NULL;
	pin_decimal_t   *initial_amount = NULL;
	int32               error_codep = 0;
	char            *plan_name = NULL;
	int32             plan_type = 5;
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_ncr_sub_bal", ebufp);
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_ncr_sub_bal input flist", in_flistp);
	//data_flistp = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_DATA_ARRAY,0, 1, ebufp);
	while((res_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_RESULTS,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Result flist", res_flistp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Result flist of purchased product", rr_flistpp);
		if(rr_flistpp && foundd == 1)
                {
                   mso_prod_obj = PIN_FLIST_FLD_GET(rr_flistpp, PIN_FLD_POID, 1, ebufp);
                }
                else 
                {
			
			goto SKIP;	
                }
		read_mso_plan_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(read_mso_plan_flistp, PIN_FLD_POID, mso_prod_obj, ebufp);
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_cust_activate mso_plan: Read input flist", read_mso_plan_flistp);
      	  	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_mso_plan_flistp, &read_mso_plan_oflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&read_mso_plan_flistp, NULL);
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_cust_activate mso_plan: Read output flist", read_mso_plan_oflistp);
		rec_id_p = 0;
		cookie_p = NULL;
          while ( (prod_flistp = PIN_FLIST_ELEM_GET_NEXT (read_mso_plan_oflistp,
                       PIN_FLD_PRODUCTS, &rec_id_p, 1,&cookie_p, ebufp)) != (pin_flist_t *)NULL )
             {
                prov_tag = PIN_FLIST_FLD_GET(prod_flistp, PIN_FLD_PROVISIONING_TAG, 0, ebufp);
                if ( prov_tag && strlen(prov_tag) > 0)
                {
                    break;
                }
             }
		
	    
		service_obj = PIN_FLIST_FLD_GET(res_flistp,PIN_FLD_SERVICE_OBJ,1,ebufp);
		rfld_iflistp = PIN_FLIST_CREATE(ebufp);
        	PIN_FLIST_FLD_SET(rfld_iflistp, PIN_FLD_POID, (void *)service_obj, ebufp);
        	//Add the BB Info SUBSTR
        	temp_flistp = PIN_FLIST_SUBSTR_ADD(rfld_iflistp, MSO_FLD_BB_INFO, ebufp);
        	PIN_FLIST_FLD_SET(temp_flistp, MSO_FLD_TECHNOLOGY, (void *)0, ebufp);
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                	"fm_mso_renew: Read Service In flist", rfld_iflistp);
        	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, rfld_iflistp, &rfld_oflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&rfld_iflistp, NULL);
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                	"fm_mso_renew: Read Service Out flist", rfld_oflistp);
        	temp_flistp = PIN_FLIST_SUBSTR_GET(rfld_oflistp, MSO_FLD_BB_INFO, 0, ebufp);
	        tech = (int *)PIN_FLIST_FLD_GET(temp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp);
		fm_quota_check(ctxp, plan_name, prov_tag, tech, &plan_type, &initial_amount, &error_codep, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_quota_check:", ebufp);
			PIN_FLIST_DESTROY_EX(&rfld_oflistp, NULL);
			PIN_FLIST_DESTROY_EX(&read_mso_plan_oflistp, NULL);
			return;
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " RETURNED FROM PROV ");
		SKIP:
		sb_cookie = NULL;
		while((sb_flistp = PIN_FLIST_ELEM_GET_NEXT(res_flistp, PIN_FLD_SUB_BAL_IMPACTS,
			&sb_elem_id, 1, &sb_cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Sub Balance flist", sb_flistp);
			res_id = PIN_FLIST_FLD_GET(sb_flistp,PIN_FLD_RESOURCE_ID,1,ebufp);
			sprintf(tmp1,"Resource Id is %d and %d, %d, %d",*res_id,MSO_FREE_MB,MSO_FUP_TRACK,MSO_TRCK_USG);
			sprintf(tmp,"plan_type : %d",plan_type);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, tmp1 );
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, tmp );
			
			if(res_id && (*res_id == MSO_FREE_MB || *res_id == MSO_FUP_TRACK  || (plan_type == 0 && *res_id == MSO_TRCK_USG))) 
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " inside lopp ");
				if(found == 0)
				{
					*r_flistpp = PIN_FLIST_CREATE(ebufp);
					found = 1;
				}
				PIN_FLIST_ELEM_SET(*r_flistpp,sb_flistp,PIN_FLD_SUB_BAL_IMPACTS,ctr,ebufp);
				ctr++;
			}
		}
	}
	PIN_FLIST_DESTROY_EX(&rfld_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&read_mso_plan_oflistp, NULL);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_ncr_sub_bal return flist", *r_flistpp);
}


static void add_plans_to_device(pcm_context_t 		*ctxp, 
	pin_flist_t			*deal_flistp, 
	pin_flist_t			*hardware_flistp,
	pin_flist_t			*mso_pp_out_flistp,
	int32				plan_type,
	pin_errbuf_t		*ebufp) 
{
	pin_flist_t *plan_flistp = NULL;
	pin_flist_t *dev_flistp = NULL;
	pin_flist_t *dev_srch_flistp = NULL;
	pin_flist_t *dev_srch_oflistp = NULL;
	pin_flist_t *args_flistp = NULL;
	pin_flist_t *result_flistp = NULL;
	pin_flist_t *dev_plan_flistp = NULL;
	pin_flist_t *dev_plan_oflistp = NULL;
	pin_flist_t *mso_plan_flistp = NULL;
	poid_t		*plan_pdp = NULL;
	poid_t		*mso_pp_pdp = NULL;
	poid_t		*srch_pdp = NULL;
	poid_t		*pp_plan_pdp = NULL;
	int32		srch_flag = 0;
	int32		elem_id = 0;
	pin_cookie_t	cookie = NULL;
	int64		db = 1;
	char		*srch_template = "select X from /device where F1 = V1 ";
	void		*vp=NULL;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "DEALS flist", deal_flistp);
	plan_pdp = PIN_FLIST_FLD_GET(deal_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
	mso_pp_pdp = PIN_FLIST_FLD_GET(mso_pp_out_flistp, PIN_FLD_POID, 0, ebufp);


	while((plan_flistp = PIN_FLIST_ELEM_GET_NEXT(hardware_flistp, PIN_FLD_PLAN,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PLAN flist", plan_flistp);
			// Added below condition and to NOT update mso_pp obj in device 
			// if the flow is HW Add plan, since it is done in Add plan API.
			vp=NULL;
			vp = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_DEVICE_ID, 1, ebufp);
			if(vp)
			    continue;

			pp_plan_pdp = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
			if(!PIN_POID_COMPARE(pp_plan_pdp, plan_pdp, 0, ebufp)) {
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Matching plan found. Adding plan entry for the device.");
				dev_flistp = PIN_FLIST_ELEM_GET(plan_flistp, PIN_FLD_DEVICES, 0, 0, ebufp);
				
				dev_srch_flistp = PIN_FLIST_CREATE(ebufp);
				srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
				PIN_FLIST_FLD_SET(dev_srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
				PIN_FLIST_FLD_SET(dev_srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
				PIN_FLIST_FLD_SET(dev_srch_flistp, PIN_FLD_TEMPLATE, srch_template, ebufp);
				args_flistp = PIN_FLIST_ELEM_ADD(dev_srch_flistp, PIN_FLD_ARGS, 1, ebufp);
				PIN_FLIST_FLD_COPY(dev_flistp, PIN_FLD_DEVICE_ID, args_flistp, PIN_FLD_DEVICE_ID,ebufp);
				result_flistp = PIN_FLIST_ELEM_ADD(dev_srch_flistp, PIN_FLD_RESULTS, 0, ebufp);
				PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "DEV Search input flist", dev_srch_flistp);
				PCM_OP(ctxp, PCM_OP_SEARCH, 0, dev_srch_flistp, &dev_srch_oflistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "DEV Search output flist", dev_srch_oflistp);
				PIN_POID_DESTROY(srch_pdp, NULL);
				result_flistp = PIN_FLIST_ELEM_GET(dev_srch_oflistp, PIN_FLD_RESULTS, 0, 0, ebufp);
				
				dev_plan_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_POID, dev_plan_flistp, PIN_FLD_POID, ebufp);
				mso_plan_flistp = PIN_FLIST_ELEM_ADD(dev_plan_flistp, PIN_FLD_PLAN, 0, ebufp);
				PIN_FLIST_FLD_COPY(mso_pp_out_flistp, PIN_FLD_POID, mso_plan_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, ebufp);
				PIN_FLIST_FLD_SET(mso_plan_flistp, PIN_FLD_TYPE, &plan_type, ebufp);

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "DEVICE update input flist", dev_plan_flistp);
				PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 32, dev_plan_flistp, &dev_plan_oflistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "DEVICE update output flist", dev_plan_oflistp);

				PIN_FLIST_DESTROY_EX(&dev_srch_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&dev_srch_oflistp, NULL);
				PIN_FLIST_DESTROY_EX(&dev_plan_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&dev_plan_oflistp, NULL);
				break;
			}
		}

}

static void 
fm_mso_cust_activate_amc_plan(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		*mso_pp_flistp,
	pin_errbuf_t		*ebufp )
{
	pin_flist_t	*read_in_flistp = NULL;
	pin_flist_t	*read_out_flistp = NULL;
	pin_flist_t	*tmp_flistp = NULL;
	pin_flist_t	*amc_flistp = NULL;
	pin_flist_t	*amc_out_flistp = NULL;
	pin_flist_t	*args_flistp = NULL;

	poid_t		*acc_obj = NULL;
	poid_t		*svc_obj = NULL;
	poid_t		*user_obj = NULL;
	int32		*indicator = NULL;
	int32		amc_flag = AMC_ON_POSTPAID_HW_PURCHASE;
	int32		hw_plan_type = -1;
	int32		mode = 0; 
	int32		elem_id = 0;
	pin_cookie_t	cookie = NULL;
	char		msg[256] = {"\0"};
	pin_decimal_t	*priority_val = NULL;
	double		prio_double = 0.0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_activate_amc_plan input flist", in_flist);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_PP_FLIST input flist", mso_pp_flistp);
	
	acc_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_ACCOUNT_OBJ, 0, ebufp );
	svc_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_SERVICE_OBJ, 0, ebufp );
	user_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_USERID, 0, ebufp );
	//Read service indicator.
	read_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, read_in_flistp, PIN_FLD_POID, ebufp);
	args_flistp = PIN_FLIST_SUBSTR_ADD(read_in_flistp, MSO_FLD_BB_INFO, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_INDICATOR, NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read service input", read_in_flistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_in_flistp, &read_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&read_in_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read service output", read_out_flistp);
	args_flistp = PIN_FLIST_SUBSTR_GET(read_out_flistp, MSO_FLD_BB_INFO, 0, ebufp );
	indicator = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_INDICATOR, 0, ebufp);
	
	
	// AMC plan should only be associated for Postpaid
	if(!indicator || *indicator!=PAYINFO_BB_POSTPAID)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Plan is Not Postpaid. AMC not applicable.");
		PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);
		return;
	}

	PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Plan is Postpaid.");

	cookie = NULL;
	while ((tmp_flistp = PIN_FLIST_ELEM_GET_NEXT(mso_pp_flistp, PIN_FLD_PRODUCTS, 
			&elem_id, 1, &cookie, ebufp)) != NULL)
	{	

		read_in_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, read_in_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(read_in_flistp, PIN_FLD_PRIORITY, NULL, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read Purchased plan input", read_in_flistp);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_in_flistp, &read_out_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&read_in_flistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
			return;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read Purchased plan output", read_out_flistp);
		priority_val = PIN_FLIST_FLD_GET(read_out_flistp, PIN_FLD_PRIORITY, 0, ebufp);
		if (priority_val)
		{
			prio_double = pbo_decimal_to_double(priority_val, ebufp);
			prio_double = (int)prio_double;
			if((prio_double >= BB_HW_RENTAL_RANGE_START && prio_double <= BB_HW_RENTAL_RANGE_END) || 
				(prio_double >= BB_HW_DEPOSIT_RANGE_START && prio_double <= BB_HW_DEPOSIT_RANGE_END)) {
			    hw_plan_type = RENTAL_DEPOSIT_PLAN;
			} 
		}
		PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);
	}
	if(hw_plan_type != RENTAL_DEPOSIT_PLAN)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Rental Hardware Plan not found . AMC not applicable.");
		return;
	}


	//Call AMC opcode
	amc_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(amc_flistp, PIN_FLD_POID, acc_obj, ebufp);
	PIN_FLIST_FLD_SET(amc_flistp, PIN_FLD_SERVICE_OBJ, svc_obj, ebufp);
	PIN_FLIST_FLD_SET(amc_flistp, PIN_FLD_USERID, user_obj, ebufp);
	PIN_FLIST_FLD_SET(amc_flistp, PIN_FLD_FLAGS, &amc_flag, ebufp);
	PIN_FLIST_FLD_SET(amc_flistp, PIN_FLD_MODE, &mode, ebufp);
		
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_BB_HW_AMC input", amc_flistp);
        PCM_OP(ctxp, MSO_OP_CUST_BB_HW_AMC, 0, amc_flistp, &amc_out_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_CUST_BB_HW_AMC", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_BB_HW_AMC output", amc_out_flistp);
	PIN_FLIST_DESTROY_EX(&amc_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&amc_out_flistp, NULL);
	return;
}

/* get bill_when from service */
int32 get_bill_when_from_service(pcm_context_t   *ctxp,
                                 poid_t          *svc_pdp,
                                 pin_errbuf_t    *ebufp)
{
        int32   bill_when = 0;
        pin_flist_t     *read_in_flistp = PIN_FLIST_CREATE(ebufp);
        pin_flist_t     *read_out_flistp = NULL;
        pin_flist_t     *bb_info_flistp = NULL;

        PIN_FLIST_FLD_SET(read_in_flistp, PIN_FLD_POID, svc_pdp, ebufp);
        bb_info_flistp = PIN_FLIST_ELEM_ADD(read_in_flistp, MSO_FLD_BB_INFO, 0, ebufp);
        PIN_FLIST_FLD_SET(bb_info_flistp, PIN_FLD_BILL_WHEN, NULL, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input flist to read bill_when from service", read_in_flistp);
        PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_in_flistp, &read_out_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Output flist to read bill_when from service", read_out_flistp);
        bb_info_flistp = PIN_FLIST_ELEM_GET(read_out_flistp, MSO_FLD_BB_INFO, 0 ,0, ebufp);
        bill_when = *(int32 *) PIN_FLIST_FLD_GET(bb_info_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);
        PIN_FLIST_DESTROY_EX(&read_out_flistp,NULL);
        PIN_FLIST_DESTROY_EX(&read_in_flistp, NULL);

        return bill_when;
}

/* Changes for static IP charging */
/* Read IS tal FLAG */
void  apply_static_ip_charge(cm_nap_connection_t     *connp,
							pin_flist_t          	*in_flistp,
							pin_flist_t 			**r_flistpp,
							pin_errbuf_t    		*ebufp)
{
    // Changes for static IP
	int32                           flags = 512;
	//char                          *s_template = " select X from /mso_cfg_credit_limit 1 ,/config/product/tal 2 where 1.F1 = 2.F2 and 1.F3 = V3 and not exists (select 1 from config_tal_plan_excempt c where c.plan_obj_id0 = config_product_tal_plans_t.plan_obj_id0) ";
	char                            s_template[300] = " select X from /mso_cfg_credit_limit 1 ,/config/product/tal 2 where 1.F1 = 2.F2 and 1.F3 = V3  and 2.F4 = V4  and 1.F5 != V5 and not exists (select 1 from config_tal_plan_exempt c where c.plan_obj_id0 = ";
	char                            s_template_zero[300] = " select X from /mso_cfg_credit_limit 1 ,/config/product/tal 2 where 1.F1 = 2.F2 and 1.F3 = V3  and 2.F4 = V4 and 1.F5 = V5 and not exists (select 1 from config_tal_plan_exempt c where c.plan_obj_id0 = ";
	pin_flist_t                     *deal_array = NULL;
	int32                           *is_tal;
	int32                           *bill_when;
	int32                           *indicator;
	pin_cookie_t                    cookie = NULL;
	int32                           elem_id = 0;
	pin_cookie_t                    cookie1 = NULL;
	int32                           elem_id1 = 0;
	int                             *p_type = NULL;
	pin_flist_t                     *robj_iflp = NULL;
	char                            msg[100];
	pin_flist_t                     *args_flistp= NULL;
	pin_flist_t                     *readsvc_inflistp = NULL;
	pin_flist_t                     *robj_oflp = NULL;
	pin_flist_t                     *s_flistp = NULL;
	poid_t                          *s_pdp = NULL;
	int32                           db = 1;
	int32                           id = -1;
	poid_t                          *plan_pdp=NULL;
	pin_flist_t                     *city_flistp=NULL;
	pin_flist_t                     *s_oflistp=NULL;
	pin_flist_t                     *res_flistp=NULL;
	poid_t                          *plan_obj=NULL;
	pin_flist_t                     *plans_flistp = NULL;
	pin_flist_t                     *temp_flistp = NULL;
	pin_flist_t                     *deals_deposit = NULL;
	pin_flist_t                     *deals_activation = NULL;
	pin_flist_t                     *deal_flistp = NULL;
	pin_flist_t                     *r1_flistp = NULL;
	pin_flist_t                     *plan_array_flistp = NULL;
	pin_flist_t                     *plans_flist = NULL;
	pin_flist_t                     *get_prod_flistp = NULL;
	pin_flist_t						*deal_info_flistp = NULL;
	pin_flist_t						*prod_flistp = NULL;
	int32                           status_active = 1;
	pin_flist_t						*pur_in_flistp = NULL;
	char                     		*search_prt_template = "select X from /mso_purchased_plan where F1 = V1 and F2 = V2 and F3 = V3";
	poid_t                          *offering_obj=NULL;
	poid_t                          *product_obj=NULL;
	time_t							validity_extension_t = 0;
	pin_flist_t						*srch_prt_out_flistp = NULL;
	time_t							*pp_end_t = NULL;
	time_t                          current_t = 0;
	char           			  		msg_v[1000];	
	int32                   		srch_flag = 512;
	int                     		plan_status = 2;
	poid_t							*service_obj = NULL;
	pin_flist_t                     *prod_out_flistp = NULL;
	pin_flist_t                     *read_in_flistp = NULL;
	pin_flist_t                     *srch_flistp = NULL;
	pin_flist_t                     *results_flistp = NULL;
	pin_flist_t                     *read_pp_res_flistp = NULL;
	pin_flist_t						*cycle_info_flistp = NULL;
	time_t                          new_end_t = 0;
	time_t                          *cycle_end_t = NULL;
	pin_flist_t						*arg_flistp = NULL;

	poid_t                          *base_plan_obj=NULL;
	char                            plan_poid_id[50];
    char                            *str_rolep = NULL;
	int                     		status = PIN_PRODUCT_STATUS_INACTIVE;
	int								pp_status = 2;
	int                     		pp_status_inctv = 1;
	int                     		status_flags = 64;
	time_t							now = 0;
	pin_flist_t                     *sp_iflistp = NULL;
	pin_flist_t                     *tmp_flistp = NULL;
	pin_flist_t                     *sp_oflistp = NULL;
	pin_flist_t                     *read_pp_flistp = NULL;
	pin_flist_t                     *read_pp_oflistp = NULL;
	poid_t                          *pp_pdp=NULL;
	int32                           *tal_override = NULL;
	char        					srch_template1[200] = "select X from /mso_purchased_plan where F1 = V1 and F2 = V2 and F3 = V3 ";
	pin_flist_t                     *srch_o_flistp = NULL;
	pin_flist_t                     *srch_i_flistp = NULL;
	int32           				base_pack = 0;
	int32							pd_id = 0;
	char							pdids[50];
	int32							srch_flags= 256;
	pin_flist_t                     *srch_res_flistp = NULL;
	char							base_plan[20]="base plan";
	pin_decimal_t   				*one_dp = pbo_decimal_from_str("1", ebufp);
	int             				tax_imp_type = 1;
	pin_decimal_t   				*zero_decimalp = pbo_decimal_from_str("0", ebufp);
	char							*city = NULL;
	poid_t							*orig_pp_pdp = NULL;
	time_t							*charged_from_t = NULL;
	pin_flist_t                     *prod_oflistp = NULL;
	pin_flist_t                     *usage_flistp = NULL;
	char							*prod_event_type = NULL;
	pin_flist_t                     *topup_prod_flistp = NULL;
	poid_t							*orig_cancel_prod_obj = NULL;
	pin_flist_t                     *plan_ret_flistp = NULL;
	char							*plan_name = NULL;
	poid_t							*act_pdp = NULL;
	poid_t							*service_pdp = NULL;
	poid_t							*orig_cancel_offer_obj = NULL;
	time_t							created_t = 0;
	pin_flist_t     				*srch_plan_retflistp = NULL;
	pin_flist_t     				*purch_prod_oflistp = NULL;
	pin_flist_t     				*event_det_flistp = NULL;
	pin_flist_t     				*subscr_prods_flistp = NULL;
	int32							relem_id = 0;
	pin_cookie_t    				rcookie = NULL;
	int32           				belem_id = 0;
	pin_cookie_t    				bcookie = NULL;
	pin_flist_t     				*balimp_flistp = NULL;
	poid_t							*bal_prod_obj = NULL;
	poid_t							*vp = NULL;
	int32           				resource_id = 0;
	char							*tax_code = NULL;
	poid_t							*bal_rate_obj = NULL;
	poid_t							*bal_grp_obj = NULL;
	int32           				bal_gl_id = 0;
	pin_decimal_t   				*ch_amt = NULL;
	pin_decimal_t   				*bal_disc_amt = NULL;
	pin_flist_t     				*ri_flistp = NULL;
	double          				orig_quantity=0;
	double          				quantity=0;
	double          				scale=1;
	int             				tax_index = 6;
	pin_decimal_t   				*cf_amt = NULL;
	pin_decimal_t   				*pf_amt = NULL;
	pin_decimal_t   				*tot_amt = NULL;
	pin_decimal_t   				*total_disc_perc = pbo_decimal_from_str("0", ebufp);
	pin_decimal_t   				*zero_amt = pbo_decimal_from_str("0", ebufp);
	pin_flist_t                     *pack_flistp = NULL;
	pin_flist_t                     *taxes_flistp = NULL;
	pin_flist_t                     *prods_flistp = NULL;
	pin_flist_t                     *axes_flist_flistp = NULL;
	pin_decimal_t   				*scalep = NULL;
	pin_flist_t     				*tax_flistp = NULL;
	pin_flist_t     				*total_flistp = NULL;
	pin_flist_t     				*ro_flistp = NULL;
	pin_flist_t             		*bb_bp_flistp = NULL;
	pin_flist_t             		*pur_plan_flistp = NULL;
	pin_flist_t            			*hardware_flistp = NULL;
	pin_flist_t             		*ret_flistp = NULL;
	int32							ret_status = 0;
	int32                   		activate_service_failure = 1;
	poid_t							*orig_offer_obj = NULL;
	poid_t							*orig_plan_obj = NULL;
	poid_t							*deal_pdp = NULL;
	poid_t							*act_poid = NULL;
	pin_flist_t             		*read_flds_iflisp = NULL;
	pin_flist_t             		*read_flds_oflisp = NULL;
	pin_flist_t            			*prod_array = NULL;
	pin_flist_t             		*robj_cpp_oflp = NULL;
	pin_flist_t						*pln_type_flistp = NULL;
	pin_flist_t                     *pur_out_flistp = NULL;
	pin_flist_t                     *prod_temp_flistp = NULL;
	pin_flist_t                     *pr_flistp = NULL;
	pin_flist_t         			*event_type_flistp = NULL;
	pin_flist_t         			*evt_usage_flistp = NULL;
	pin_flist_t         			*last_plan_flistp = NULL;
	int32           				elem_id_2 = 0;
	pin_cookie_t    				cookie_2 = NULL;
	int32							mode = 0;
	int								zero_type = 0;
	char							*code="TAL IP Prepaid Plan";
	char							*agreement_no = NULL;
	char                			*event_typ=NULL;
	int32           				*tech = NULL;
	int32							trial_flag =0;

	/* Changes for QPS to CPS Migration.  */
	pcm_context_t                   *new_ctxp;
	int32                           *old_bill_when;
	pcm_context_t   				*ctxp = connp->dm_ctx;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling apply_static_ip_charge", ebufp);
			return;
	}
	PIN_ERRBUF_CLEAR(ebufp);
	sprintf(msg, "apply_static_ip_charge: PCM_OP_READ_FLDS input flist");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
	
	// Check if the product is a base back.
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Validating Base Pack");		
	robj_iflp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_SERVICE_OBJ,robj_iflp,PIN_FLD_POID, ebufp);

	deal_info_flistp = PIN_FLIST_SUBSTR_GET(in_flistp, PIN_FLD_DEAL_INFO, 0, ebufp);
	base_plan_obj = PIN_FLIST_FLD_GET(deal_info_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);	
	act_poid =  PIN_FLIST_SUBSTR_GET(*r_flistpp,PIN_FLD_POID, 0, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(robj_iflp, MSO_FLD_BB_INFO, 0, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_ISTAL_FLAG, NULL, ebufp);
    PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_ROLES, NULL, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BILL_WHEN, NULL, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_INDICATOR, NULL, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_REAUTH_FLAG, NULL, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CITY, NULL, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_AGREEMENT_NO, NULL, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_TECHNOLOGY, NULL, ebufp);
   
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read Is_tal input flist:",robj_iflp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, robj_iflp, &robj_oflp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read Is_tal output flist:",robj_oflp);
	PIN_FLIST_DESTROY_EX(&robj_iflp, NULL);

	args_flistp = PIN_FLIST_ELEM_GET(robj_oflp, MSO_FLD_BB_INFO, 0, 1, ebufp );
	is_tal = (int *)PIN_FLIST_FLD_GET(args_flistp, MSO_FLD_ISTAL_FLAG, 1, ebufp );
    str_rolep =(char *) PIN_FLIST_FLD_GET(args_flistp, MSO_FLD_ROLES, 1, ebufp );
	bill_when = (int *)PIN_FLIST_FLD_GET(args_flistp,PIN_FLD_BILL_WHEN, 1, ebufp); 	
	indicator = (int *)PIN_FLIST_FLD_GET(args_flistp,PIN_FLD_INDICATOR, 1, ebufp);
	city 	  = (char *)PIN_FLIST_FLD_GET(args_flistp,PIN_FLD_CITY,1, ebufp);
	agreement_no = (char *)PIN_FLIST_FLD_GET(args_flistp,MSO_FLD_AGREEMENT_NO,1, ebufp);
	tal_override = (int *) PIN_FLIST_FLD_GET(args_flistp,PIN_FLD_REAUTH_FLAG,1, ebufp);
	tech = PIN_FLIST_FLD_GET(args_flistp, MSO_FLD_TECHNOLOGY, 1, ebufp);
	
	//check the plan is Trial
	fm_mso_get_event_from_plan(ctxp,base_plan_obj,&event_type_flistp,ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "NEW PLAN EVENT TYPE FOR VALIDATION OF TRIAL", event_type_flistp);
	if (event_type_flistp && event_type_flistp != NULL && PIN_FLIST_ELEM_COUNT(event_type_flistp, PIN_FLD_RESULTS, ebufp) > 0)
	{
	   last_plan_flistp = PIN_FLIST_ELEM_GET(event_type_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	   evt_usage_flistp = PIN_FLIST_ELEM_GET(last_plan_flistp, PIN_FLD_USAGE_MAP, 0, 1, ebufp);
	   event_typ = PIN_FLIST_FLD_GET(evt_usage_flistp, PIN_FLD_EVENT_TYPE, 1, ebufp);
    }

	if (event_typ && event_typ != NULL && strstr(event_typ, "trial")!= NULL && (strlen(strstr(event_typ, "trial"))>0 )) 
	{
		trial_flag = 1;
	}
   
    if((str_rolep && strstr(str_rolep, "-Static")) && indicator != NULL && tal_override != NULL && tech && *tech != MSO_FIBER && trial_flag == 0)
	{
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_search_plan_details PCM_OP_SEARCH error.", ebufp);
			goto CLEANUP;
		}
		PIN_ERRBUF_CLEAR(ebufp);

        sprintf(plan_poid_id,"%lld ) ",PIN_POID_GET_ID(base_plan_obj));
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,plan_poid_id);	

		strcat(s_template,plan_poid_id);
		strcat(s_template_zero,plan_poid_id);

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Inside Is_tal = 1");
		
		// Retrieve all TAL products.
		s_flistp = PIN_FLIST_CREATE(ebufp);
		s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

		PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
		//PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
		city_flistp = PIN_FLIST_ELEM_ADD(args_flistp,PIN_FLD_PLAN, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);

		/*if(connp->coip->opcode == MSO_OP_CUST_CHANGE_PLAN) //Kali Comment Start
        {                                        
		   // Changes for change plan inclusion in static IP  
			PCM_CONTEXT_OPEN(&new_ctxp, (pin_flist_t *)0, ebufp);
			robj_iflp = PIN_FLIST_CREATE(ebufp);

			PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_SERVICE_OBJ,robj_iflp,PIN_FLD_POID, ebufp);
			args_flistp = PIN_FLIST_ELEM_ADD(robj_iflp, MSO_FLD_BB_INFO, 0, ebufp );
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BILL_WHEN, NULL, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read Is_tal input flist:",robj_iflp);
			PCM_OP(new_ctxp, PCM_OP_READ_FLDS, 0, robj_iflp, &robj_cpp_oflp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read Is_tal output flist:",robj_cpp_oflp);
			PIN_FLIST_DESTROY_EX(&robj_iflp, NULL);

			args_flistp = PIN_FLIST_ELEM_GET(robj_cpp_oflp, MSO_FLD_BB_INFO, 0, 1, ebufp );
			old_bill_when = (int *)PIN_FLIST_FLD_GET(args_flistp,PIN_FLD_BILL_WHEN, 1, ebufp);
			args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
			city_flistp = PIN_FLIST_ELEM_ADD(args_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp);
			
			// Retrieve the old bill when using alternate connection
			PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_BILL_WHEN, old_bill_when, ebufp);
		}else
		{*/// Kali Comment end

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
		city_flistp = PIN_FLIST_ELEM_ADD(args_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_BILL_WHEN, bill_when, ebufp);

		//} 
        //args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
		//PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, base_plan_obj, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
		pln_type_flistp = PIN_FLIST_ELEM_ADD(args_flistp,PIN_FLD_PLAN, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(pln_type_flistp, MSO_FLD_PLAN_TYPE, indicator, ebufp);
		
		if (*tal_override != 1) 
		{
            //PIN_FLIST_FLD_SET(pln_type_flistp, MSO_FLD_PLAN_TYPE, indicator, ebufp);
			PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);
		}
		
		if (*tal_override == 1) 
		{
			//  PIN_FLIST_FLD_SET(pln_type_flistp, MSO_FLD_PLAN_TYPE, &zero_type, ebufp);
			//	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
			//	city_flistp = PIN_FLIST_ELEM_ADD(args_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp);
			//  PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_CHARGE_AMT, zero_amt, ebufp);
			PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template_zero, ebufp);
        }

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
		city_flistp = PIN_FLIST_ELEM_ADD(args_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_CHARGE_AMT, zero_amt, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
		//	PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_PLAN_OBJ,plan_pdp,ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search mso_cfg_credit_limit Input flist:",s_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 512, s_flistp, &s_oflistp, ebufp);

	 	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);

		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_search_plan_details PCM_OP_SEARCH error.", ebufp);
			goto CLEANUP;
		}
		
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search mso_cfg_credit_limit Output flist:",s_oflistp);

		res_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " After reading PIN_FLD_RESULTS");

		if (res_flistp != NULL)
        {
			plan_obj = (poid_t *) PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);	
		}	
		else
		{
			return;
		}

		/*if(connp->coip->opcode == MSO_OP_CUST_CHANGE_PLAN) //Kali Comment Start
		{
			// Retrieve the new plan object for purchase.

			plans_flistp = PIN_FLIST_CREATE(ebufp);
			plan_array_flistp = (pin_flist_t *)PIN_FLIST_ELEM_ADD(plans_flistp, PIN_FLD_PLAN, PIN_ELEMID_ANY, ebufp);
			PIN_FLIST_FLD_SET(plan_array_flistp, PIN_FLD_PLAN_OBJ, orig_plan_obj, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "IP Plan with devices flist input", plans_flistp );

			// For getting deals for each plan passed in input 
			fm_mso_get_deals_from_plan(ctxp, plans_flistp, &deal_array, &temp_flistp, ebufp);

			if(deal_array) 
				deal_flistp = PIN_FLIST_ELEM_GET(deal_array, PIN_FLD_DEALS, PIN_ELEMID_ANY, 1, ebufp);

			if(deal_flistp) 
				deal_pdp = PIN_FLIST_FLD_GET(deal_flistp, PIN_FLD_DEAL_OBJ, 1, ebufp); 

			read_flds_iflisp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(read_flds_iflisp, PIN_FLD_POID, (void *)deal_pdp, ebufp);
			PIN_FLIST_ELEM_SET(read_flds_iflisp, NULL, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_READ_FLDS input flist", read_flds_iflisp);

			PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flds_iflisp, &read_flds_oflisp, ebufp);
			PIN_FLIST_DESTROY_EX(&read_flds_iflisp, NULL);
			if(deal_array)PIN_FLIST_DESTROY_EX(&deal_array,NULL);

			if (PIN_ERR_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
				goto CLEANUP;
			}

			elem_id_2 = 0;
			cookie_2 = NULL;
			while((prod_array = PIN_FLIST_ELEM_GET_NEXT(read_flds_oflisp, PIN_FLD_PRODUCTS,
			&elem_id_2, 1, &cookie_2, ebufp)) != (pin_flist_t *)NULL)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Fetch Old product");
				orig_cancel_prod_obj = PIN_FLIST_FLD_GET(prod_array, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
			}

			s_flistp = PIN_FLIST_CREATE(ebufp);
			s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
			PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
			PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
			PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);

			args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);

			args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
			city_flistp = PIN_FLIST_ELEM_ADD(args_flistp,PIN_FLD_PLAN, PIN_ELEMID_ANY, ebufp);
			PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);

			args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
			city_flistp = PIN_FLIST_ELEM_ADD(args_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp);
			PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_BILL_WHEN, bill_when, ebufp);

			args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, base_plan_obj, ebufp);

			args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
			PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_PLAN_OBJ,plan_pdp,ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search mso_cfg_credit_limit Input flist:",s_flistp);
			PCM_OP(ctxp, PCM_OP_SEARCH, 512, s_flistp, &s_oflistp, ebufp);
			PIN_FLIST_DESTROY_EX(&s_flistp, NULL);

			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"fm_mso_search_plan_details PCM_OP_SEARCH error.", ebufp);
				goto CLEANUP;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search mso_cfg_credit_limit Output flist:",s_oflistp);

			res_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " After reading PIN_FLD_RESULTS");

			if (res_flistp != NULL)
			{
				plan_obj = (poid_t *) PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);	
			}	
			else
			{
				return;
			}
		}*/

		if (plan_obj == NULL )
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"fm_mso_search_plan_details PCM_OP_SEARCH error.", ebufp);
			goto CLEANUP;
		}

		orig_plan_obj = PIN_POID_COPY(plan_obj,ebufp);

		/* Expire existing TAL PP object */
		// Search PP object

		service_obj = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp );

		plan_status = 2;
		db = PIN_POID_GET_DB(service_obj);
		srch_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, search_prt_template, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );

        /*if(connp->coip->opcode == MSO_OP_CUST_CHANGE_PLAN) //Kali Comment start
        {
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, orig_plan_obj, ebufp);

		} else
		{*///Kali Comment end
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, plan_obj, ebufp);
		//}//
		args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &plan_status, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);

		results_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

		args_flistp = PIN_FLIST_ELEM_ADD(results_flistp, PIN_FLD_PRODUCTS, 0, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, NULL, ebufp);

		PIN_FLIST_FLD_SET(results_flistp,PIN_FLD_PLAN_OBJ, NULL, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "product type search input list", srch_flistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_prt_out_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

	    if (PIN_ERRBUF_IS_ERR(ebufp) )
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
			goto CLEANUP;
		}
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "product type search output flist", srch_prt_out_flistp);
		results_flistp = PIN_FLIST_ELEM_GET(srch_prt_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);

/**		validity_extension_t = 0;
		
		if ( results_flistp && results_flistp != NULL )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Inside condition to retrieve pp_end_t");
			
			// Get the product purchase_end_t and expire the current purchased_product
	
			elem_id = 0; cookie = NULL;
			while ((prod_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_prt_out_flistp,PIN_FLD_RESULTS,
			 &elem_id, 1, &cookie, ebufp))!= NULL)
			{
				read_pp_res_flistp = PIN_FLIST_ELEM_GET(prod_flistp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, 1, ebufp);

				if(read_pp_res_flistp != NULL )
				{
					orig_pp_pdp   = PIN_FLIST_FLD_GET(read_pp_res_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ,1, ebufp);
				}	
			}

			if(orig_pp_pdp != NULL)
			{
				// Read purchase_end_t of the purchased_product			
				read_pp_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(read_pp_flistp, PIN_FLD_POID, orig_pp_pdp, ebufp);
				PIN_FLIST_FLD_SET(read_pp_flistp, PIN_FLD_CYCLE_END_T,0,ebufp);
				PIN_FLIST_FLD_SET(read_pp_flistp, PIN_FLD_CYCLE_START_T,0,ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read Purchased plan input flist", read_pp_flistp);
				PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_pp_flistp, &read_pp_oflistp, ebufp);
				PIN_FLIST_DESTROY_EX(&read_pp_flistp, NULL);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_cust_activate mso_plan: Read output flist", read_pp_oflistp);
		
				if(read_pp_oflistp != NULL )
				{
					pp_end_t = PIN_FLIST_FLD_GET(read_pp_oflistp, PIN_FLD_CYCLE_END_T, 1, ebufp);
					charged_from_t = PIN_FLIST_FLD_GET(read_pp_oflistp, PIN_FLD_CYCLE_START_T, 1, ebufp);
					created_t = *charged_from_t - 100;
				}
			}

			current_t = pin_virtual_time(NULL);
			current_t = fm_utils_time_round_to_midnight(current_t);
			current_t = current_t + 86400;

			if(pp_end_t != NULL && *pp_end_t >= 0)
			{		
				sprintf(msg_v,"days remaining is  : %ld", ((*pp_end_t - fm_utils_time_round_to_midnight(current_t))/60/60/24));
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg_v);
				if(*pp_end_t == 0) 
				{
					validity_extension_t = 1; // Just to force expiry of current TAL product
				} 
				else 
				{
					validity_extension_t = *pp_end_t - fm_utils_time_round_to_midnight(current_t);
				}
			}
		}**/
	
		// Purchase the IP product	

		plans_flistp = PIN_FLIST_CREATE(ebufp);
		plan_array_flistp = (pin_flist_t *)PIN_FLIST_ELEM_ADD(plans_flistp, PIN_FLD_PLAN, PIN_ELEMID_ANY, ebufp);	
		PIN_FLIST_FLD_SET(plan_array_flistp, PIN_FLD_PLAN_OBJ, plan_obj, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "IP Plan with devices flist input", plans_flistp );

		/* For getting deals for each plan passed in input */
		fm_mso_get_deals_from_plan(ctxp, plans_flistp, &deal_array, &temp_flistp, ebufp);

		PIN_FLIST_DESTROY_EX(&plans_flistp, NULL);

		cycle_info_flistp = PIN_FLIST_ELEM_GET(res_flistp,PIN_FLD_CYCLE_INFO,PIN_ELEMID_ANY,1,ebufp);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Deal Array output", deal_array);
		PIN_FLIST_DESTROY_EX(&temp_flistp, NULL);

		fm_mso_get_deal_info(ctxp, deal_array, &deals_deposit, &deals_activation,ebufp);
		PIN_FLIST_DESTROY_EX(&deals_deposit, NULL);

		if(deals_activation) 
		{
            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Deals Activation Flist", deals_activation);
		}
	
		/* create the input flist for PURCHASE_DEAL opcode*/

		pur_in_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SERVICE_OBJ, pur_in_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, pur_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, pur_in_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, pur_in_flistp, PIN_FLD_POID, ebufp);
	
		act_pdp = PIN_FLIST_FLD_GET(in_flistp,PIN_FLD_POID,1,ebufp);
		service_pdp = PIN_FLIST_FLD_GET(in_flistp,PIN_FLD_SERVICE_OBJ,1,ebufp);

		/* Get each deal/plan from deal_array and call purchase deal for each */
		cookie = NULL;
		elem_id = 0;
		while((deal_flistp = PIN_FLIST_ELEM_GET_NEXT(deals_activation, PIN_FLD_DEALS,
					&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Inside Deals Array");

			deal_info_flistp = PIN_FLIST_SUBSTR_GET(deal_flistp, PIN_FLD_DEAL_INFO, 0, ebufp);
			PIN_FLIST_FLD_COPY(deal_flistp, PIN_FLD_PLAN_OBJ, deal_info_flistp, PIN_FLD_PLAN_OBJ, ebufp);
			PIN_FLIST_SUBSTR_SET(pur_in_flistp, deal_info_flistp, PIN_FLD_DEAL_INFO, ebufp );

			/* Added For Calc-Only */
			p_type = PIN_FLIST_FLD_GET(deal_flistp, PIN_FLD_TYPE, 1, ebufp);
			deal_info_flistp = PIN_FLIST_SUBSTR_GET(pur_in_flistp, PIN_FLD_DEAL_INFO, 0, ebufp);

			/* Set the status to Active in every product array */
			elem_id1 = 0 ; cookie1 = NULL;
			while((prod_flistp = PIN_FLIST_ELEM_GET_NEXT(deal_info_flistp, PIN_FLD_PRODUCTS,
						&elem_id1, 1, &cookie1, ebufp)) != (pin_flist_t *)NULL)
			{
				PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_STATUS, &status_active, ebufp);
			}

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "---> Purchase Deal Static IP Input Flist", pur_in_flistp);
			PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_PURCHASE_DEAL, 0, pur_in_flistp, &r1_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchat_cm.pinloge Deal Static IP output flist", r1_flistp);

			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"PCM_OP_SUBSCRIPTION_PURCHASE_DEAL for Static IP error.", ebufp);
				goto CLEANUP;
			}

			pur_out_flistp = PIN_FLIST_COPY(r1_flistp,ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchase Deal output flist", pur_out_flistp);

			// Create Purchasd plan obj
			pur_plan_flistp = PIN_FLIST_COPY(deal_array, ebufp);
			
			//Append purchased product details with Deals info
			/* Populate Purchased plan array with product array from PURCHASE_DEAL output*/
			/* Plans have same Array element id (elem_id) in  pur_plan_flistp and deals_activation */
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Static IP Purchase Array output 2");
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Deal Array output", pur_plan_flistp);

			prod_temp_flistp = PIN_FLIST_ELEM_GET(pur_plan_flistp, PIN_FLD_DEALS, PIN_ELEMID_ANY, 0, ebufp );
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Static Purchase Array output 2 test");
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Deal Array output", pur_plan_flistp);
			pr_flistp = PIN_FLIST_ELEM_GET(pur_out_flistp, PIN_FLD_PRODUCTS,PIN_ELEMID_ANY, 0, ebufp );
			PIN_FLIST_ELEM_SET(prod_temp_flistp, pr_flistp, PIN_FLD_PRODUCTS, 0, ebufp );
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Purchase Array output 2.1");
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Deal Array output", pr_flistp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Purchase Array output 3");
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Deal Array output", pur_plan_flistp);
			PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, pur_plan_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
			PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SERVICE_OBJ, pur_plan_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
			fm_mso_create_purchased_plan_object(ctxp, pur_plan_flistp, bb_bp_flistp, hardware_flistp, &ret_flistp, ebufp );
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchased plan return flist", ret_flistp );
			if ( ret_flistp && ret_flistp != (pin_flist_t *)NULL )
			{
				ret_status = *(int32*)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);
				if (ret_status == activate_service_failure)
				{
					goto CLEANUP;
				}
			}

			if(ret_flistp)
				PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);	
			if(pur_plan_flistp)
				PIN_FLIST_DESTROY_EX(&pur_plan_flistp, NULL);	

			if ( validity_extension_t > 0 )
			{
				/* Expire the current purchased plan */
				sp_iflistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, sp_iflistp, PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, sp_iflistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
				PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, sp_iflistp, PIN_FLD_PROGRAM_NAME, ebufp);
				PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SERVICE_OBJ, sp_iflistp, PIN_FLD_SERVICE_OBJ, ebufp);
				PIN_FLIST_FLD_SET(sp_iflistp, PIN_FLD_DESCR, (void *)"Set status after TAL re-purchase", ebufp);
				
				now = pin_virtual_time(NULL);

				PIN_FLIST_FLD_SET(sp_iflistp, PIN_FLD_END_T, (void *)&now, ebufp);
				tmp_flistp = PIN_FLIST_ELEM_ADD(sp_iflistp, PIN_FLD_STATUSES, 1, ebufp);
				PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_OFFERING_OBJ, (void *)orig_pp_pdp, ebufp);
				PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STATUS, &status, ebufp);
				PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STATUS_FLAGS, &status_flags, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"apply_static_ip_charge: Set Status Input Flist", sp_iflistp);
				//Call the Set status opcode
				PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_SET_PRODUCT_STATUS, 0, sp_iflistp, &sp_oflistp, ebufp);
				
				if (PIN_ERR_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						"apply_static_ip_charge: Error", ebufp);
					goto CLEANUP;
				}
				
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"apply_static_ip_charge: Set Status Output Flist", sp_oflistp);
				PIN_FLIST_DESTROY_EX(&sp_iflistp, NULL);
				if(sp_oflistp != NULL )PIN_FLIST_DESTROY_EX(&sp_oflistp, NULL);

				// Do validity extension
				//  Retrieve the offring OBJ from results flist.

				if(r1_flistp != NULL )
			         relem_id = 0;rcookie=NULL;
				 
				cycle_info_flistp = NULL;
				
				while ((res_flistp = PIN_FLIST_ELEM_GET_NEXT(r1_flistp, PIN_FLD_RESULTS,
						&relem_id, 1, &rcookie, ebufp)) != NULL)
				{
	
					cycle_info_flistp = PIN_FLIST_SUBSTR_GET(res_flistp,PIN_FLD_CYCLE_INFO,1,ebufp);
					if(cycle_info_flistp != NULL ) 
						break;
				}

				if(cycle_info_flistp != NULL)
				{	
					//Call the SET_PRODUCT_STATUS opcode

					/*if(connp->coip->opcode == MSO_OP_CUST_CHANGE_PLAN) //Kali Comment start
					{
						// Apply prorata refund
						//Now trigger DISCONNECT CREDIT CHARGE (Refund) for prepaid subscription

						// Get EVENT TYPE for refund to be created
						fm_mso_utils_read_any_object(ctxp, orig_cancel_prod_obj, &prod_oflistp, ebufp);


						if ( prod_oflistp && prod_oflistp !=(pin_flist_t *)NULL)
						{
							usage_flistp = PIN_FLIST_ELEM_GET(prod_oflistp, PIN_FLD_USAGE_MAP, PIN_ELEMID_ANY, 1, ebufp);
							if (usage_flistp && usage_flistp !=(pin_flist_t *)NULL)
							{
								prod_event_type = PIN_FLIST_FLD_GET(usage_flistp, PIN_FLD_EVENT_TYPE, 1, ebufp);
							}

						}
						
						fm_mso_utils_read_any_object(ctxp, orig_plan_obj, &plan_ret_flistp, ebufp);
						if(plan_ret_flistp && plan_ret_flistp !=(pin_flist_t *)NULL)
						plan_name = PIN_FLIST_FLD_GET(plan_ret_flistp, PIN_FLD_NAME, 1, ebufp);
						fm_mso_utils_get_event_balimpacts(ctxp, act_pdp, service_pdp, orig_pp_pdp, &created_t, prod_event_type, &srch_plan_retflistp, ebufp);

						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "SEARCH CHARGE DETAILS RETURN", srch_plan_retflistp);
						if  (PIN_ERRBUF_IS_ERR(ebufp))
						{
							PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in while searching charge details", ebufp);
							PIN_FLIST_DESTROY_EX(&plan_ret_flistp, NULL);
							PIN_FLIST_DESTROY_EX(&srch_plan_retflistp, NULL);
							PIN_FLIST_DESTROY_EX(&prod_oflistp, NULL);
							PIN_FLIST_DESTROY_EX(&purch_prod_oflistp, NULL);
							PIN_FLIST_DESTROY_EX(&event_det_flistp, NULL);
							PIN_FLIST_DESTROY_EX(&subscr_prods_flistp, NULL);
							return;
						}
				
						if(srch_plan_retflistp && srch_plan_retflistp !=(pin_flist_t *)NULL && 
							PIN_FLIST_ELEM_COUNT(srch_plan_retflistp, PIN_FLD_RESULTS, ebufp) > 0)
						{
							relem_id = 0;
							rcookie=NULL;
							
							while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_plan_retflistp, PIN_FLD_RESULTS,
							&relem_id, 1, &rcookie, ebufp)) != NULL)
							{
								belem_id = 0; 
								bcookie = NULL;
								
								while ((balimp_flistp = PIN_FLIST_ELEM_GET_NEXT(results_flistp, PIN_FLD_BAL_IMPACTS,
								&belem_id, 1, &bcookie, ebufp)) != NULL)
								{
									bal_prod_obj =  PIN_FLIST_FLD_GET(balimp_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
									vp = (int32 *)PIN_FLIST_FLD_GET(balimp_flistp, PIN_FLD_RESOURCE_ID, 1, ebufp);
									if(vp) resource_id  = *(int32 *)vp;
									tax_code =  PIN_FLIST_FLD_GET(balimp_flistp, PIN_FLD_TAX_CODE, 1, ebufp);
									bal_rate_obj =  PIN_FLIST_FLD_GET(balimp_flistp, PIN_FLD_RATE_OBJ, 1, ebufp);
									bal_grp_obj =  PIN_FLIST_FLD_GET(balimp_flistp, PIN_FLD_BAL_GRP_OBJ, 1, ebufp);
									vp =  (int32 *)PIN_FLIST_FLD_GET(balimp_flistp, PIN_FLD_GL_ID, 1, ebufp);
									
									if(vp && vp != NULL) 
										bal_gl_id = *(int32 *)vp;
									
									if((orig_cancel_prod_obj && orig_cancel_prod_obj != NULL && 
										bal_prod_obj && bal_prod_obj != NULL && 
										PIN_POID_COMPARE(orig_cancel_prod_obj, bal_prod_obj, 0,ebufp) == 0)
										&& resource_id == 356)
									{
										ch_amt = PIN_FLIST_FLD_TAKE(balimp_flistp, PIN_FLD_AMOUNT, 1, ebufp);
										bal_disc_amt = PIN_FLIST_FLD_GET(balimp_flistp, PIN_FLD_DISCOUNT, 1, ebufp);

										//If any discount is given during the plan purchase, consider to get full charge amount.
										if(bal_disc_amt && bal_disc_amt != NULL && !pbo_decimal_is_null(ch_amt, ebufp))
											pbo_decimal_add_assign(ch_amt, bal_disc_amt, ebufp);
									}
								}
							}
						}

						ri_flistp = PIN_FLIST_CREATE(ebufp);

						act_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1, ebufp);	

						PIN_FLIST_FLD_SET(ri_flistp, PIN_FLD_POID, act_pdp, ebufp);
						PIN_FLIST_FLD_SET(ri_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
						PIN_FLIST_FLD_SET(ri_flistp, PIN_FLD_OFFERING_OBJ, orig_pp_pdp, ebufp);
						PIN_FLIST_FLD_SET(ri_flistp, PIN_FLD_PRODUCT_OBJ, orig_cancel_prod_obj, ebufp);
						PIN_FLIST_FLD_SET(ri_flistp, PIN_FLD_PROGRAM_NAME, "STATIC_IP_DISCONNECTION_CREDIT", ebufp);
						PIN_FLIST_FLD_SET(ri_flistp, PIN_FLD_START_T, &current_t, ebufp);
						PIN_FLIST_FLD_SET(ri_flistp, PIN_FLD_END_T, pp_end_t, ebufp);
						PIN_FLIST_FLD_SET(ri_flistp, PIN_FLD_EVENT_TYPE, prod_event_type, ebufp);
						PIN_FLIST_FLD_PUT(ri_flistp, PIN_FLD_PERCENT, pbo_decimal_from_str("0", ebufp), ebufp); // If any discount, Set PERCENT to zero as we handle it in scale
						
						// Calculate SCALE to be refunded
						orig_quantity = (*pp_end_t - *charged_from_t)/60/60/24;
						quantity = (*pp_end_t - fm_utils_time_round_to_midnight(current_t))/60/60/24;
						scale = quantity/orig_quantity;
						scalep = pbo_decimal_from_double(scale,ebufp);

						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "REFUND SCALE calculation22:", ebufp);
						//We arrive with some cases where scale could become -ve like charged_to_t < current or some toup and extension cases
						sprintf(msg_v,"Value of scale is  : %ld\n",scale);	
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg_v);

						if(pbo_decimal_sign(scalep, ebufp) == -1 || pbo_decimal_is_zero(scalep, ebufp))
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No refund is required in case of charged_to_t is < current_date or some topup cases");
							PIN_ERRBUF_CLEAR(ebufp);
							PIN_FLIST_DESTROY_EX(&plan_ret_flistp, NULL);
							PIN_FLIST_DESTROY_EX(&srch_plan_retflistp, NULL);
							PIN_FLIST_DESTROY_EX(&prod_oflistp, NULL);
							PIN_FLIST_DESTROY_EX(&purch_prod_oflistp, NULL);
							PIN_FLIST_DESTROY_EX(&event_det_flistp, NULL);
							PIN_FLIST_DESTROY_EX(&subscr_prods_flistp, NULL);
							PIN_FLIST_DESTROY_EX(&ri_flistp, NULL);
							if(!pbo_decimal_is_null(ch_amt, ebufp))
								pbo_decimal_destroy(&ch_amt);
							if(!pbo_decimal_is_null(total_disc_perc, ebufp))
								pbo_decimal_destroy(&total_disc_perc);
							goto CLEANUP;
						}

						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "REFUND SCALE calculation:");
						if (ch_amt && (!pbo_decimal_is_null(ch_amt, ebufp) || !pbo_decimal_is_zero(ch_amt, ebufp)))
						{
							pf_amt = pbo_decimal_from_str("0", ebufp);

							pbo_decimal_multiply_assign(ch_amt, scalep, ebufp);
							pbo_decimal_negate_assign(ch_amt, ebufp);

							cf_amt = pbo_decimal_copy(ch_amt, ebufp);
							pack_flistp = PIN_FLIST_CREATE(ebufp);
							city_flistp = PIN_FLIST_ELEM_ADD(pack_flistp, MSO_FLD_CITIES, 0,  ebufp);
							PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_CYCLE_FEE_AMT, cf_amt, ebufp);
							PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_PURCHASE_FEE_AMT, pf_amt, ebufp);
							PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_CHARGE_AMT, ch_amt, ebufp);
							PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_CITY, city, ebufp);
							fm_mso_search_tax_code(ctxp, pack_flistp, act_pdp, bill_when, ch_amt, cf_amt, pf_amt, plan_name, &tot_amt, &taxes_flistp, ebufp);
							PIN_FLIST_DESTROY_EX(&pack_flistp, NULL);
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "SEARCH TAX DETAILS RETURN", taxes_flistp);

							prods_flistp = PIN_FLIST_ELEM_ADD(ri_flistp, PIN_FLD_BAL_IMPACTS, 0, ebufp);
							PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_TAX_CODE, tax_code, ebufp);
							PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_ACCOUNT_OBJ, act_pdp, ebufp);
							PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_OFFERING_OBJ, orig_pp_pdp, ebufp);
							PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_RATE_OBJ, bal_rate_obj, ebufp);
							PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_RATE_TAG, "New Rate", ebufp);
							PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_GL_ID, &bal_gl_id, ebufp);
							PIN_FLIST_FLD_PUT(prods_flistp, PIN_FLD_QUANTITY, one_dp, ebufp);
							PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_IMPACT_TYPE, &tax_imp_type, ebufp);
							PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_PERCENT, scalep, ebufp);
							PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_AMOUNT, ch_amt, ebufp);
							PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_RESOURCE_ID, &resource_id, ebufp);
							PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_AMOUNT_DEFERRED, zero_decimalp, ebufp);
							PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_obj, ebufp);

							if(taxes_flistp && axes_flist_flistp !=(pin_flist_t *)NULL)
							{
								elem_id = 0;
								cookie = NULL;
								tax_index = 6; //This is due taxation in hathway
								tax_imp_type = 4;
								while ((tax_flistp = PIN_FLIST_ELEM_GET_NEXT(taxes_flistp, PIN_FLD_TAXES,
											&elem_id, 1, &cookie, ebufp)) != NULL) 
								{
									prods_flistp = PIN_FLIST_ELEM_ADD(ri_flistp, PIN_FLD_BAL_IMPACTS, tax_index, ebufp);

									PIN_FLIST_FLD_COPY(tax_flistp, PIN_FLD_TAX_CODE, prods_flistp, PIN_FLD_TAX_CODE, ebufp);
									PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_ACCOUNT_OBJ, act_pdp, ebufp);
									PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_RATE_TAG, "Tax", ebufp);
									PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_QUANTITY, ch_amt, ebufp);
									PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_IMPACT_TYPE, &tax_imp_type, ebufp);
									PIN_FLIST_FLD_COPY(tax_flistp, PIN_FLD_TAX_CODE, prods_flistp, PIN_FLD_TAX_CODE, ebufp);
									PIN_FLIST_FLD_COPY(tax_flistp, PIN_FLD_PERCENT, prods_flistp, PIN_FLD_PERCENT, ebufp);
									PIN_FLIST_FLD_COPY(tax_flistp, PIN_FLD_TAX, prods_flistp, PIN_FLD_AMOUNT, ebufp);
									PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_RESOURCE_ID, &resource_id, ebufp);
									PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_AMOUNT_DEFERRED, zero_decimalp, ebufp);
									PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_obj, ebufp);
									tax_index = tax_index+1;
								}
							}
							
							if (!pbo_decimal_is_null(tot_amt, ebufp) && ri_flistp != (pin_flist_t *)NULL)
							{
								total_flistp = PIN_FLIST_ELEM_ADD(ri_flistp, PIN_FLD_TOTAL, 356, ebufp);
								PIN_FLIST_FLD_PUT(total_flistp, PIN_FLD_AMOUNT, tot_amt, ebufp);
							}

							PIN_FLIST_FLD_PUT(ri_flistp, PIN_FLD_SCALE, scalep, ebufp); // If any discount given while purchase
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_generate_credit_charge refund iflist:", ri_flistp);
							fm_mso_create_refund_for_cancel_plan(ctxp, ri_flistp, &ro_flistp, ebufp );
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_generate_credit_charge refund oflist:", ro_flistp);
						}*/ 
						
					relem_id = 0;
					rcookie=NULL;

					while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(r1_flistp, PIN_FLD_PRODUCTS,
						&relem_id, 1, &rcookie, ebufp)) != NULL)
					{
				
						offering_obj = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_OFFERING_OBJ, 1, ebufp);
						product_obj = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PRODUCT_OBJ , 1 , ebufp);
						if(offering_obj != NULL) 
							break;
					}
					
					if(offering_obj != NULL)
					{
						cycle_end_t = PIN_FLIST_FLD_GET(cycle_info_flistp,PIN_FLD_CYCLE_END_T,1,ebufp);
						new_end_t =  validity_extension_t + *cycle_end_t   ;

						read_in_flistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, read_in_flistp, PIN_FLD_POID, ebufp);
						PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, read_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
						arg_flistp = PIN_FLIST_ELEM_ADD(read_in_flistp, PIN_FLD_PRODUCTS, 0, ebufp );
						PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_OFFERING_OBJ, offering_obj, ebufp);
						PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_PRODUCT_OBJ, product_obj, ebufp);

						PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_PURCHASE_END_T, &new_end_t, ebufp);
						PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_CYCLE_END_T, &new_end_t, ebufp);
						PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_USAGE_END_T, &new_end_t, ebufp);
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PROD SETINFO input flist", read_in_flistp);
						PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_SET_PRODINFO, 0, read_in_flistp, &prod_out_flistp, ebufp);
						PIN_FLIST_DESTROY_EX(&read_in_flistp, NULL);
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PROD SETINFO output flist", prod_out_flistp);
						PIN_FLIST_DESTROY_EX(&prod_out_flistp, NULL);
					}						
				}
			}

			/* Concatenate balance array from r1_flistpp to r_flistpp */
			PIN_FLIST_ELEM_DROP(r1_flistp, PIN_FLD_PRODUCTS, 0, ebufp);
            PIN_FLIST_CONCAT(*r_flistpp, r1_flistp, ebufp);

			PIN_FLIST_DESTROY_EX(&r1_flistp,ebufp);
			PIN_FLIST_DESTROY_EX(&get_prod_flistp, NULL);
			if(pur_in_flistp) 
				PIN_FLIST_DESTROY_EX(&pur_in_flistp, NULL);
		}
		if(deal_array)
			PIN_FLIST_DESTROY_EX(&deal_array, NULL);
		PIN_FLIST_DESTROY_EX(&plans_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&s_oflistp, NULL);
		if(read_flds_oflisp) 
			PIN_FLIST_DESTROY_EX(&read_flds_oflisp, NULL);
		if(read_pp_oflistp) 
			PIN_FLIST_DESTROY_EX(&read_pp_oflistp, NULL);
	}

	/***********************************************************
	* Lifecycle events
	***********************************************************/
	if (!(PIN_ERRBUF_IS_ERR(ebufp)))
    {
		PIN_FLIST_FLD_SET(in_flistp,MSO_FLD_AGREEMENT_NO,agreement_no ,ebufp);
		city_flistp = PIN_FLIST_ELEM_ADD(in_flistp,PIN_FLD_PLAN, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(city_flistp,PIN_FLD_PLAN_OBJ,orig_plan_obj ,ebufp);
		
		//Kali added for tal plan change
		if(connp->coip->opcode == MSO_OP_CUST_CHANGE_PLAN)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Changeplan call No need to create add event");
	    }
	    else
	    {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Create Life Cycle Event for TAL plans");
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Create Lifecycle Input flist", in_flistp);
			fm_mso_create_lifecycle_evt(ctxp, in_flistp, *r_flistpp, ID_ADD_PLAN, ebufp);
	    }
		
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "Error in Lifecycle event creation",pur_in_flistp);
			//PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Lifecycle event creation", ebufp);
			PIN_ERRBUF_RESET(ebufp);
		}
    }

CLEANUP:	
	if(robj_oflp) 
		PIN_FLIST_DESTROY_EX(&robj_oflp, NULL);
	if(srch_prt_out_flistp) 
		PIN_FLIST_DESTROY_EX(&srch_prt_out_flistp , NULL);
	
	return;
}

/* Changes for static IP charging */
        /* Read IS tal FLAG */
void  calc_static_ip_charge(cm_nap_connection_t     *connp,	
							pin_flist_t          	*in_flistp,
							pin_flist_t 			**r_flistpp,
							pin_errbuf_t    		*ebufp)
{
	// Changes for static IP
	int32                           flags = 512;
	//char                          *s_template = " select X from /mso_cfg_credit_limit 1 ,/config/product/tal 2 where 1.F1 = 2.F2 and 1.F3 = V3 and not exists (select 1 from config_tal_plan_excempt c where c.plan_obj_id0 = config_product_tal_plans_t.plan_obj_id0) ";
	char                            s_template[300] = " select X from /mso_cfg_credit_limit 1 ,/config/product/tal 2 where 1.F1 = 2.F2 and 1.F3 = V3  and 2.F4 = V4 and 1.F5 != V5  and not exists (select 1 from config_tal_plan_exempt c where c.plan_obj_id0 = ";
	char                            s_template_zero[300] = " select X from /mso_cfg_credit_limit 1 ,/config/product/tal 2 where 1.F1 = 2.F2 and 1.F3 = V3  and 2.F4 = V4 and 1.F5 = V5 and not exists (select 1 from config_tal_plan_exempt c where c.plan_obj_id0 = ";
	pin_flist_t                     *deal_array = NULL;
	int32                           *is_tal;
	int32                           *bill_when;
	int32                           *indicator;
	poid_t                          *inp_pdp = NULL;
	pin_cookie_t                    cookie = NULL;
	int32                           elem_id = 0;
	pin_cookie_t                    cookie1 = NULL;
	int32                           elem_id1 = 0;
	int                             *p_type = NULL;
	pin_flist_t                     *robj_iflp = NULL;
	char                            msg[100];
	pin_flist_t                     *args_flistp= NULL;
	pin_flist_t                     *readsvc_inflistp = NULL;
	pin_flist_t                     *robj_oflp = NULL;
	pin_flist_t                     *s_flistp = NULL;
	poid_t                          *s_pdp = NULL;
	int32                           db = 1;
	int32                           id = -1;
	poid_t                          *plan_pdp=NULL;
	pin_flist_t                     *city_flistp=NULL;
	pin_flist_t                     *s_oflistp=NULL;
	pin_flist_t                     *res_flistp=NULL;
	poid_t                          *plan_obj=NULL;
	pin_flist_t                     *plans_flistp = NULL;
	pin_flist_t                     *temp_flistp = NULL;
	pin_flist_t                     *deals_deposit = NULL;
	pin_flist_t                     *deals_activation = NULL;
	pin_flist_t                     *deal_flistp = NULL;
	pin_flist_t                     *r1_flistp = NULL;
	pin_flist_t                     *r2_flistp = NULL;
	pin_flist_t                     *plan_array_flistp = NULL;
	pin_flist_t                     *plans_flist = NULL;
	pin_flist_t                     *get_prod_flistp = NULL;
	pin_flist_t						*deal_info_flistp = NULL;
	pin_flist_t						*prod_flistp = NULL;
	int32                           status_active = 1;
	pin_flist_t						*pur_in_flistp = NULL;
	char                    		*search_prt_template = "select X from /mso_purchased_plan where F1 = V1 and F2 = V2 and F3 = V3";
	poid_t                          *offering_obj=NULL;
	poid_t                          *product_obj=NULL;
	time_t							validity_extension_t = 0;
	pin_flist_t						*srch_prt_out_flistp = NULL;
	time_t							*pp_end_t = NULL;
	time_t                          current_t = 0;
	char           			 		msg_v[1000];	
	int32                   		srch_flag = 512;
	int                     		plan_status = 2;
	poid_t							*service_obj = NULL;
	pin_flist_t                     *prod_out_flistp = NULL;
	pin_flist_t                     *read_in_flistp = NULL;
	pin_flist_t                     *srch_flistp = NULL;
	pin_flist_t                     *results_flistp = NULL;
	pin_flist_t                     *read_pp_res_flistp = NULL;
	pin_flist_t						*cycle_info_flistp = NULL;
	time_t                          new_end_t = 0;
	time_t                          *cycle_end_t = NULL;
	pin_flist_t						*arg_flistp = NULL;
	pin_flist_t                     *pur_out_flistp = NULL;
	pin_flist_t                     *prod_temp_flistp = NULL;
	pin_flist_t                     *pr_flistp = NULL;
	poid_t                          *base_plan_obj=NULL;
	char                            plan_poid_id[50];
	int                     		status = PIN_PRODUCT_STATUS_INACTIVE;
	int								pp_status = 2;
	int                     		status_flags = 64;
	time_t							now = 0;
	pin_flist_t                     *sp_iflistp = NULL;
	pin_flist_t                     *tmp_flistp = NULL;
	pin_flist_t                     *sp_oflistp = NULL;
	pin_flist_t                     *read_pp_flistp = NULL;
	pin_flist_t                     *read_pp_oflistp = NULL;
	poid_t                          *pp_pdp=NULL;
	int32                           *tal_override = NULL;
	char        					srch_template1[200] = "select X from /mso_purchased_plan where F1 = V1 and F2 = V2 and F3 = V3 ";
	pin_flist_t                     *srch_o_flistp = NULL;
	pin_flist_t                     *srch_i_flistp = NULL;

	int32           				base_pack = 0;
	int32							pd_id = 0;
	char							pdids[50];
	int32							srch_flags= 256;
	pin_flist_t                     *srch_res_flistp = NULL;
	char							base_plan[20]="base plan";
	pin_decimal_t   				*one_dp = pbo_decimal_from_str("1", ebufp);
	int             				tax_imp_type = 1;
	pin_decimal_t   				*zero_decimalp = pbo_decimal_from_str("0", ebufp);
	char							*city = NULL;
	poid_t							*orig_pp_pdp = NULL;
	time_t							*charged_from_t = NULL;
	pin_flist_t                     *prod_oflistp = NULL;
	pin_flist_t                     *usage_flistp = NULL;
	char							*prod_event_type = NULL;
	pin_flist_t                     *topup_prod_flistp = NULL;
	poid_t							*orig_cancel_prod_obj = NULL;
	pin_flist_t                     *plan_ret_flistp = NULL;
	char							*plan_name = NULL;
	poid_t							*act_pdp = NULL;
	poid_t          				*act_poid = NULL;
	poid_t							*service_pdp = NULL;
	poid_t							*orig_cancel_offer_obj = NULL;
	time_t							created_t = 0;
	pin_flist_t     				*srch_plan_retflistp = NULL;
	pin_flist_t     				*purch_prod_oflistp = NULL;
	pin_flist_t     				*event_det_flistp = NULL;
	pin_flist_t     				*subscr_prods_flistp = NULL;
	int32							relem_id = 0;
	pin_cookie_t    				rcookie = NULL;
	int32          					belem_id = 0;
	pin_cookie_t    				bcookie = NULL;
	pin_flist_t     				*balimp_flistp = NULL;
	poid_t							*bal_prod_obj = NULL;
	poid_t							*vp = NULL;
	int32           				resource_id = 0;
	char							*tax_code = NULL;
    char                            *str_rolep = NULL;
	poid_t							*bal_rate_obj = NULL;
	poid_t							*bal_grp_obj = NULL;
	int32           				bal_gl_id = 0;
	pin_decimal_t   				*ch_amt = NULL;
	pin_decimal_t   				*bal_disc_amt = NULL;
	pin_flist_t     				*ri_flistp = NULL;
	double          				orig_quantity=0;
	double          				quantity=0;
	double          				scale=1;
	int             				tax_index = 6;
	pin_decimal_t   				*cf_amt = NULL;
	pin_decimal_t   				*pf_amt = NULL;
	pin_decimal_t   				*tot_amt = NULL;
	pin_decimal_t   				*total_disc_perc = pbo_decimal_from_str("0", ebufp);
	pin_decimal_t   				*zero_amt = pbo_decimal_from_str("0", ebufp);
	pin_flist_t                     *pack_flistp = NULL;
	pin_flist_t                     *taxes_flistp = NULL;
	pin_flist_t                     *prods_flistp = NULL;
	pin_flist_t                     *axes_flist_flistp = NULL;
	pin_decimal_t   				*scalep = NULL;
	pin_flist_t     				*tax_flistp = NULL;
	pin_flist_t    					*total_flistp = NULL;
	pin_flist_t     				*ro_flistp = NULL;
	pin_flist_t             		*bb_bp_flistp = NULL;
	pin_flist_t             		*pur_plan_flistp = NULL;
	pin_flist_t             		*hardware_flistp = NULL;
	pin_flist_t             		*ret_flistp = NULL;
	int32							ret_status = 0;
	int32                   		activate_service_failure = 1;
	poid_t							*orig_offer_obj = NULL;
	poid_t							*orig_plan_obj = NULL;
	poid_t							*deal_pdp = NULL;
	pin_flist_t             		*read_flds_iflisp = NULL;
	pin_flist_t             		*read_flds_oflisp = NULL;
	pin_flist_t             		*prod_array = NULL;
	pin_flist_t             		*robj_cpp_oflp = NULL;
	pin_flist_t             		*pln_type_flistp = NULL;
	int32           				elem_id_2 = 0;
	pin_cookie_t    				cookie_2 = NULL;
	int32           				bal_elem_id = 0;
	pin_cookie_t    				bal_cookie = NULL;
	int32           				*tech = NULL;
	/* Changes for QPS to CPS Migration.  */
	pcm_context_t                   *new_ctxp;
	int32                           *old_bill_when;
	pcm_context_t   				*ctxp = connp->dm_ctx;
	int	 							local = 0;
   
	robj_iflp = PIN_FLIST_CREATE(ebufp);
	sprintf(msg, "%s: PCM_OP_READ_FLDS input flist", apply_static_ip_charge);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, robj_iflp);
	
	// Check if the product is a base back.
		
	deal_info_flistp = PIN_FLIST_SUBSTR_GET(in_flistp, PIN_FLD_DEAL_INFO, 0, ebufp);
	base_plan_obj = PIN_FLIST_FLD_GET(deal_info_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
	act_poid =  PIN_FLIST_SUBSTR_GET(*r_flistpp,PIN_FLD_POID, 0, ebufp);
	inp_pdp = PIN_FLIST_SUBSTR_GET(*r_flistpp,PIN_FLD_POID, 0, ebufp);

	PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_SERVICE_OBJ,robj_iflp,PIN_FLD_POID, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(robj_iflp, MSO_FLD_BB_INFO, 0, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_ISTAL_FLAG, NULL, ebufp);
    PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_ROLES, NULL, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BILL_WHEN, NULL, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_INDICATOR, NULL, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_REAUTH_FLAG, NULL, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CITY, NULL, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_TECHNOLOGY, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read Is_tal input flist:",robj_iflp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, robj_iflp, &robj_oflp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read Is_tal output flist:",robj_oflp);

	PIN_FLIST_DESTROY_EX(&robj_iflp, NULL);

	args_flistp = PIN_FLIST_ELEM_GET(robj_oflp, MSO_FLD_BB_INFO, 0, 1, ebufp );
	is_tal = (int *)PIN_FLIST_FLD_GET(args_flistp, MSO_FLD_ISTAL_FLAG, 1, ebufp );
    str_rolep = (char *)PIN_FLIST_FLD_GET(args_flistp, MSO_FLD_ROLES, 1, ebufp );
	bill_when = (int *)PIN_FLIST_FLD_GET(args_flistp,PIN_FLD_BILL_WHEN, 1, ebufp); 	
	indicator = (int *)PIN_FLIST_FLD_GET(args_flistp,PIN_FLD_INDICATOR, 1, ebufp);
	city 	  = (char *)PIN_FLIST_FLD_GET(args_flistp,PIN_FLD_CITY,1, ebufp);
	tal_override = (int *) PIN_FLIST_FLD_GET(args_flistp,PIN_FLD_REAUTH_FLAG,1, ebufp);
	tech = PIN_FLIST_FLD_GET(args_flistp, MSO_FLD_TECHNOLOGY, 1, ebufp);
    

    if ((str_rolep && strstr(str_rolep, "-Static")) && indicator != NULL && tal_override != NULL && tech && *tech != MSO_FIBER)
	{
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_search_plan_details PCM_OP_SEARCH error.", ebufp);
			goto CLEANUP;
		}
		PIN_ERRBUF_CLEAR(ebufp);

		sprintf(plan_poid_id,"%lld ) ",PIN_POID_GET_ID(base_plan_obj));
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,plan_poid_id);	

        strcat(s_template,plan_poid_id);
		strcat(s_template_zero,plan_poid_id);

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Inside Is_tal = 1");
		
		// Retrieve all TAL products.
		s_flistp = PIN_FLIST_CREATE(ebufp);
		s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

		PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
		city_flistp = PIN_FLIST_ELEM_ADD(args_flistp,PIN_FLD_PLAN, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
		
		/*if(connp->coip->opcode == MSO_OP_CUST_CHANGE_PLAN) //Kali Comment start
		{                                        
			// Changes for change plan inclusion in static IP  
			PCM_CONTEXT_OPEN(&new_ctxp, (pin_flist_t *)0, ebufp);
			robj_iflp = PIN_FLIST_CREATE(ebufp);

			PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_SERVICE_OBJ,robj_iflp,PIN_FLD_POID, ebufp);
			args_flistp = PIN_FLIST_ELEM_ADD(robj_iflp, MSO_FLD_BB_INFO, 0, ebufp );
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BILL_WHEN, NULL, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read Is_tal input flist:",robj_iflp);
			PCM_OP(new_ctxp, PCM_OP_READ_FLDS, 0, robj_iflp, &robj_cpp_oflp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read Is_tal output flist:",robj_cpp_oflp);

			PIN_FLIST_DESTROY_EX(&robj_iflp, NULL);

			args_flistp = PIN_FLIST_ELEM_GET(robj_cpp_oflp, MSO_FLD_BB_INFO, 0, 1, ebufp );
			old_bill_when = (int *)PIN_FLIST_FLD_GET(args_flistp,PIN_FLD_BILL_WHEN, 1, ebufp);

			args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
			city_flistp = PIN_FLIST_ELEM_ADD(args_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp);
			// Retrieve the old bill when using alternate connection

			PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_BILL_WHEN, old_bill_when, ebufp);
		}
		else
		{*///Kali Comment start

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
		city_flistp = PIN_FLIST_ELEM_ADD(args_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_BILL_WHEN, bill_when, ebufp);

		//}//
        //args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
		//PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, base_plan_obj, ebufp);
		
		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
		pln_type_flistp = PIN_FLIST_ELEM_ADD(args_flistp,PIN_FLD_PLAN, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(pln_type_flistp, MSO_FLD_PLAN_TYPE, indicator, ebufp);
		
		if (*tal_override != 1) 
		{
			//PIN_FLIST_FLD_SET(pln_type_flistp, MSO_FLD_PLAN_TYPE, indicator, ebufp);
			PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);
		}
		
		if (*tal_override == 1) 
		{
			//    PIN_FLIST_FLD_SET(pln_type_flistp, MSO_FLD_PLAN_TYPE, &zero_type, ebufp);
			//    args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
			//    city_flistp = PIN_FLIST_ELEM_ADD(args_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp);
			//    PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_CHARGE_AMT, zero_amt, ebufp);
			PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template_zero, ebufp);
		}

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
		city_flistp = PIN_FLIST_ELEM_ADD(args_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_CHARGE_AMT, zero_amt, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
		//	PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_PLAN_OBJ,plan_pdp,ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search mso_cfg_credit_limit Input flist:",s_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 512, s_flistp, &s_oflistp, ebufp);
	 	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);

		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_search_plan_details PCM_OP_SEARCH error.", ebufp);
			goto CLEANUP;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search mso_cfg_credit_limit Output flist:",s_oflistp);

		res_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " After reading PIN_FLD_RESULTS");

		if (res_flistp != NULL)
		{
			plan_obj = (poid_t *) PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);	
		}	
		else
		{
			return;
		}

		if (plan_obj == NULL )
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_search_plan_details PCM_OP_SEARCH error.", ebufp);
			goto CLEANUP;
		}

		/* Expire existing TAL PP object */

		// Search PP object
		
		service_obj = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp );

		plan_status = 2;
		db = PIN_POID_GET_DB(service_obj);
		srch_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, search_prt_template, ebufp);
		
		args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, plan_obj, ebufp);		
		args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &plan_status, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
		
		results_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

		args_flistp = PIN_FLIST_ELEM_ADD(results_flistp, PIN_FLD_PRODUCTS, 0, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, NULL, ebufp);

		PIN_FLIST_FLD_SET(results_flistp,PIN_FLD_PLAN_OBJ, NULL, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "product type search input list", srch_flistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_prt_out_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

	    if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
			goto CLEANUP;
		}
			
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "product type search output flist", srch_prt_out_flistp);
		results_flistp = PIN_FLIST_ELEM_GET(srch_prt_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);

		validity_extension_t = 0;
		
		if ( results_flistp && results_flistp != NULL )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Inside condition to retrieve pp_end_t");
		
			// Get the product purchase_end_t and expire the current purchased_product	
			elem_id = 0; cookie = NULL;
			while ((prod_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_prt_out_flistp,PIN_FLD_RESULTS,
			     &elem_id, 1, &cookie, ebufp))!= NULL)
			{
				read_pp_res_flistp = PIN_FLIST_ELEM_GET(prod_flistp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, 1, ebufp);

				if(read_pp_res_flistp != NULL )
				{
					orig_pp_pdp   = PIN_FLIST_FLD_GET(read_pp_res_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ,1, ebufp);			
				}	
			}

			if(orig_pp_pdp != NULL)
			{
				// Read purchase_end_t of the purchased_product			
				read_pp_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(read_pp_flistp, PIN_FLD_POID, orig_pp_pdp, ebufp);
				PIN_FLIST_FLD_SET(read_pp_flistp, PIN_FLD_CYCLE_END_T,0,ebufp);
				PIN_FLIST_FLD_SET(read_pp_flistp, PIN_FLD_CYCLE_START_T,0,ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read Purchased plan input flist", read_pp_flistp);
				PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_pp_flistp, &read_pp_oflistp, ebufp);
				PIN_FLIST_DESTROY_EX(&read_pp_flistp, NULL);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_cust_activate mso_plan: Read output flist", read_pp_oflistp);
		
				if(read_pp_oflistp != NULL )
				{
					pp_end_t = PIN_FLIST_FLD_GET(read_pp_oflistp, PIN_FLD_CYCLE_END_T, 1, ebufp);
					charged_from_t = PIN_FLIST_FLD_GET(read_pp_oflistp, PIN_FLD_CYCLE_START_T, 1, ebufp);
					created_t = *charged_from_t - 100;
				}
			}

			current_t = pin_virtual_time(NULL);
			current_t = fm_utils_time_round_to_midnight(current_t);
			current_t = current_t + 86400;

			if(pp_end_t != NULL && *pp_end_t >= 0)
			{
				printf(msg_v,"days remaining is  : %ld", ((*pp_end_t - fm_utils_time_round_to_midnight(current_t))/60/60/24));
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg_v);
				if (*pp_end_t == 0)
				{
					validity_extension_t = 1; // Just to force expiry of current TAL product
				} 
				else 
				{
					validity_extension_t = *pp_end_t - fm_utils_time_round_to_midnight(current_t);
				}
			}
		}
	
		// Purchase the IP product	
		plans_flistp = PIN_FLIST_CREATE(ebufp);
		plan_array_flistp = (pin_flist_t *)PIN_FLIST_ELEM_ADD(plans_flistp, PIN_FLD_PLAN, PIN_ELEMID_ANY, ebufp);	
		PIN_FLIST_FLD_SET(plan_array_flistp, PIN_FLD_PLAN_OBJ, plan_obj, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "IP Plan with devices flist input", plans_flistp );

		/* For getting deals for each plan passed in input */
		fm_mso_get_deals_from_plan(ctxp, plans_flistp, &deal_array, &temp_flistp, ebufp);

		PIN_FLIST_DESTROY_EX(&plans_flistp, NULL);

		cycle_info_flistp = PIN_FLIST_ELEM_GET(res_flistp,PIN_FLD_CYCLE_INFO,PIN_ELEMID_ANY,1,ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Deal Array output", deal_array);
		PIN_FLIST_DESTROY_EX(&temp_flistp, NULL);

		fm_mso_get_deal_info(ctxp, deal_array, &deals_deposit, &deals_activation,ebufp);

		PIN_FLIST_DESTROY_EX(&deals_deposit, NULL);

		if(deals_activation) 
		{
            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Deals Activation Flist", deals_activation);
		}
	
		/* create the input flist for PURCHASE_DEAL opcode*/
		pur_in_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SERVICE_OBJ, pur_in_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, pur_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, pur_in_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, pur_in_flistp, PIN_FLD_POID, ebufp);
	
		act_pdp = PIN_FLIST_FLD_GET(in_flistp,PIN_FLD_POID,1,ebufp);
		service_pdp = PIN_FLIST_FLD_GET(in_flistp,PIN_FLD_SERVICE_OBJ,1,ebufp);
		
		/* Get each deal/plan from deal_array and call purchase deal for each */
		cookie = NULL;
		elem_id = 0;
		while((deal_flistp = PIN_FLIST_ELEM_GET_NEXT(deals_activation, PIN_FLD_DEALS,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Inside Deals Array");

			deal_info_flistp = PIN_FLIST_SUBSTR_GET(deal_flistp, PIN_FLD_DEAL_INFO, 0, ebufp);
			PIN_FLIST_FLD_COPY(deal_flistp, PIN_FLD_PLAN_OBJ, deal_info_flistp, PIN_FLD_PLAN_OBJ, ebufp);
			PIN_FLIST_SUBSTR_SET(pur_in_flistp, deal_info_flistp, PIN_FLD_DEAL_INFO, ebufp );

			/* Added For Calc-Only */
			p_type = PIN_FLIST_FLD_GET(deal_flistp, PIN_FLD_TYPE, 1, ebufp);

			deal_info_flistp = PIN_FLIST_SUBSTR_GET(pur_in_flistp, PIN_FLD_DEAL_INFO, 0, ebufp);

			/* Set the status to Active in every product array */
			elem_id1 = 0 ; cookie1 = NULL;
			while((prod_flistp = PIN_FLIST_ELEM_GET_NEXT(deal_info_flistp, PIN_FLD_PRODUCTS,
				&elem_id1, 1, &cookie1, ebufp)) != (pin_flist_t *)NULL)
			{
				PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_STATUS, &status_active, ebufp);
			}

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "---> Purchase Deal Static IP Input Flist", pur_in_flistp);
			PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_PURCHASE_DEAL, PCM_OPFLG_CALC_ONLY, pur_in_flistp, &r1_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchat_cm.pinloge Deal Static IP output flist", r1_flistp);
			
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"PCM_OP_SUBSCRIPTION_PURCHASE_DEAL for Static IP error.", ebufp);
				goto CLEANUP;
			}

			elem_id1 = 0;
			cookie1 = NULL;

			while((res_flistp = PIN_FLIST_ELEM_GET_NEXT(r1_flistp, PIN_FLD_RESULTS,
			&elem_id1, 1, &cookie1, ebufp)) != (pin_flist_t *)NULL) 
			{
				bal_cookie = NULL; 
				bal_elem_id = 0;
				while((temp_flistp = PIN_FLIST_ELEM_GET_NEXT(res_flistp, PIN_FLD_BAL_IMPACTS,
				&bal_elem_id, 1, &bal_cookie, ebufp)) != (pin_flist_t *)NULL) 
				{
					PIN_FLIST_FLD_SET(temp_flistp, MSO_FLD_IP_MASK_VALUE, plan_poid_id, ebufp);
				}
			}
			pur_out_flistp = PIN_FLIST_COPY(r1_flistp,ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchase Deal output flist", pur_out_flistp);
			
			PIN_FLIST_ELEM_DROP(r1_flistp, PIN_FLD_PRODUCTS, 0, ebufp);
			PIN_FLIST_CONCAT(*r_flistpp, r1_flistp, ebufp);

			// Create Purchased plan obj
            pur_plan_flistp = PIN_FLIST_COPY(deal_array, ebufp);
			//Append purchased product details with Deals info

			/* Populate Purchased plan array with product array from PURCHASE_DEAL output*/
			/* Plans have same Array element id (elem_id) in  pur_plan_flistp and deals_activation */
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Static IP Purchase Array output 2");
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Deal Array output", pur_plan_flistp);

			prod_temp_flistp = PIN_FLIST_ELEM_GET(pur_plan_flistp, PIN_FLD_DEALS, PIN_ELEMID_ANY, 0, ebufp );
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Static Purchase Array output 2 test");
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Deal Array output", pur_plan_flistp);
			pr_flistp = PIN_FLIST_ELEM_GET(pur_out_flistp, PIN_FLD_PRODUCTS,PIN_ELEMID_ANY, 0, ebufp );
			PIN_FLIST_ELEM_SET(prod_temp_flistp, pr_flistp, PIN_FLD_PRODUCTS, 0, ebufp ); 
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Purchase Array output 2.1");
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Deal Array output", pr_flistp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Purchase Array output 3");
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Deal Array output", pur_plan_flistp);

			// Append the static ip  charges to the output
			//PIN_FLIST_ELEM_DROP(r1_flistp, PIN_FLD_PRODUCTS, 0, ebufp);
            //PIN_FLIST_CONCAT(*r_flistpp, r1_flistp, ebufp);
			// Create Purchasd plan obj
			// pur_plan_flistp = PIN_FLIST_COPY(deal_array, ebufp);

		    PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, pur_plan_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
			PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SERVICE_OBJ, pur_plan_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
			fm_mso_create_purchased_plan_object(ctxp, pur_plan_flistp, bb_bp_flistp, hardware_flistp, &ret_flistp, ebufp );
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchased plan return flist", ret_flistp );
			if ( ret_flistp && ret_flistp != (pin_flist_t *)NULL )
			{
				ret_status = *(int32*)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);
				if (ret_status == activate_service_failure)
				{
					goto CLEANUP;
				}
			}

			if(ret_flistp)PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);	
			if(pur_plan_flistp)PIN_FLIST_DESTROY_EX(&pur_plan_flistp, NULL);	


			if ( validity_extension_t > 0 )
			{
				//  Retrieve the offring OBJ from results flist.
			    relem_id = 0;rcookie=NULL;
				cycle_info_flistp = NULL;
				while ((res_flistp = PIN_FLIST_ELEM_GET_NEXT(r1_flistp, PIN_FLD_RESULTS,
						&relem_id, 1, &rcookie, ebufp)) != NULL)
				{
					cycle_info_flistp = PIN_FLIST_SUBSTR_GET(res_flistp,PIN_FLD_CYCLE_INFO,1,ebufp);
			
					if(cycle_info_flistp != NULL ) 
						break;
				}	

				if(cycle_info_flistp != NULL)
				{	
					//Call the SET_PRODUCT_STATUS opcode

				}
			}

			PIN_FLIST_DESTROY_EX(&get_prod_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&r1_flistp,ebufp);
			if(pur_in_flistp) PIN_FLIST_DESTROY_EX(&pur_in_flistp, NULL);

		}
		
		if(deal_array)
			PIN_FLIST_DESTROY_EX(&deal_array, NULL);
		PIN_FLIST_DESTROY_EX(&plans_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&s_oflistp, NULL);
		if(read_flds_oflisp) 
			PIN_FLIST_DESTROY_EX(&read_flds_oflisp, NULL);
		if(read_pp_oflistp) 
			PIN_FLIST_DESTROY_EX(&read_pp_oflistp, NULL);	
	}
	
CLEANUP:	
	if(robj_oflp) 
		PIN_FLIST_DESTROY_EX(&robj_oflp , NULL);
	if(srch_prt_out_flistp)
		PIN_FLIST_DESTROY_EX(&srch_prt_out_flistp , NULL);
	return;
}


