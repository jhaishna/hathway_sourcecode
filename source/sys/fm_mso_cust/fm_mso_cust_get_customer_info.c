/*******************************************************************
 * File:	fm_mso_cust_get_customer_info.c
 * Opcode:	MSO_OP_CUST_GET_CUSTOMER_INFO 
 * Owner:	Gautam Adak
 * Created:	17-SEPT-2013
 * Description: 
 *
 * Modification History: 
 * Modified By: Pavan Bellala
 * Date: 09-07-2015
 * Modification details : Maintain common structure of out flist when reading customer info before and after activating services
 *
 * Modified By: Jyothirmayi Kachiraju
 * Date: 28-JAN-2020
 * Modification details 
 * Description: Added code to fetch the PP_TYPE for CATV customers and also to fetch the device details for inactive services
 *
 * Input Flist:
 * ------------
 * 0 PIN_FLD_POID             POID [0] 0.0.0.0 0 0
 * 0 PIN_FLD_USERID           POID [0] 0.0.0.1 /account 786433 0 
 * 0 PIN_FLD_ACCOUNT_OBJ      POID [0] 0.0.0.1 /account 67033 0
 * 0 PIN_FLD_FLAGS             INT [0] 0
 *
 *
 * 0 PIN_FLD_POID             POID [0] 0.0.0.0 0 0
 * 0 PIN_FLD_USERID           POID [0] 0.0.0.1 /account 786433 0 
 * 0 PIN_FLD_ACCOUNT_NO        STR [0] "67033"
 * 0 MSO_FLD_DATA_ACCESS       STR [0] "STATE|MAHARASTRA" 
 * 0 PIN_FLD_FLAGS             INT [0] 0
 *		     OR
 * 0 PIN_FLD_POID             POID [0] 0.0.0.0 0 0
 * 0 PIN_FLD_USERID           POID [0] 0.0.0.1 /account 786433 0 
 * 0 MSO_FLD_VC_ID             STR [0] "14456"
 * 0 MSO_FLD_DATA_ACCESS       STR [0] "STATE|MAHARASTRA"
 * 0 PIN_FLD_FLAGS             INT [0] 0
 *		    OR
 * 0 PIN_FLD_POID             POID [0] 0.0.0.0 0 0
 * 0 PIN_FLD_USERID           POID [0] 0.0.0.1 /account 786433 0 
 * 0 PIN_FLD_RMN               STR [0] "99864532123"
 * 0 MSO_FLD_DATA_ACCESS       STR [0] "STATE|MAHARASTRA"
 * 0 PIN_FLD_FLAGS             INT [0] 0
 *
 *
 * Output Flist:
 * ------------
0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 67033 8
0 PIN_FLD_ACCOUNT_NO      STR [0] "67033"
0 PIN_FLD_BUSINESS_TYPE   ENUM [0] 1
0 PIN_FLD_STATUS         ENUM [0] 10100
0 MSO_FLD_ACCOUNT_INFO      SUBSTRUCT [0] allocated 22, used 22
1     PIN_FLD_NAMEINFO      ARRAY [1] allocated 22, used 22
1     PIN_FLD_NAMEINFO      ARRAY [2] allocated 22, used 22
0 PIN_FLD_SERVICE_INFO      SUBSTRUCT [0] allocated 22, used 22
1     PIN_FLD_SERVICES      ARRAY [0] allocated 22, used 22
1     PIN_FLD_SERVICES      ARRAY [1] allocated 22, used 22
1     PIN_FLD_SERVICES      ARRAY [n] allocated 22, used 22
0 PIN_FLD_PLAN_LISTS        ARRAY [0] allocated 22, used 22
1     PIN_FLD_PLAN          ARRAY [0] allocated 22, used 22
...
...

 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_get_customer_info.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "ops/act.h"
#include "pin_cust.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "pin_pymt.h"
#include "ops/ar.h"
#include "ops/bal.h"
#include "mso_ops_flds.h"
#include "mso_cust.h"

#define CORPORATE_PARENT 1
#define CORPORATE_CHILD  2


#define WHOLESALE     "/profile/wholesale"
#define CUSTOMER_CARE "/profile/customer_care"

#define GET_ALL 0
#define GET_INACTIVE_SERVICE 1
#define GET_CANCELLED_SERVICE 2
#define GET_UPASS_ALL 3
#define GET_ALL_SKIP_CAN 5
#define LIGHT_WEIGHT 6

#define MSO_ORG_ACCESS_ALL 0
#define MSO_ORG_ACCESS_SP_JV 1
#define MSO_ORG_ACCESS_DT 2
#define MSO_ORG_ACCESS_SDT 3
#define MSO_ORG_ACCESS_LCO 4

#define MSO_DATA_ACCESS_GLOBAL 0
#define MSO_DATA_ACCESS_STATE 1
#define MSO_DATA_ACCESS_CITY 2
#define MSO_DATA_ACCESS_AREA 3
#define MSO_DATA_ACCESS_LOCATION 4



/**************************************
* External Functions start
***************************************/
/**************************************
* External Functions end
***************************************/
extern void
fm_mso_get_err_desc(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);
 
extern void
fm_mso_get_account_info(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

	
extern void
fm_mso_utils_fetch_ar_details(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        poid_t                  *acct_pdp,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp);

extern void
fm_mso_cust_get_pp_type(
	pcm_context_t	    	*ctxp,
        poid_t              *acct_pdp,
        pin_flist_t         **ret_flistpp,
        pin_errbuf_t        *ebufp);
		
/**************************************
* Local Functions start
***************************************/  
extern void 
replace_char (
	char	*sptr, 
	char	find, 
	char	replace);

static void
fm_mso_get_service_info(
	pcm_context_t		*ctxp,
	poid_t			*user_id, 
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flistp,
	int32			flag,
	pin_errbuf_t		*ebufp);

static void
fm_mso_validate_user_id(
	pcm_context_t		*ctxp,
	poid_t			*user_id, 
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_get_purch_prod_details(
	pcm_context_t		*ctxp,
	poid_t			*user_id,
	pin_flist_t		**service_info,
        int32                   flags,
	pin_errbuf_t		*ebufp);

static void
fm_mso_get_device_details(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_get_bill_details(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_get_acnt_from_vc(
	pcm_context_t		*ctxp,
	char*			card_no,
	poid_t			**acnt_pdp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_get_acnt_from_rmn(
	pcm_context_t		*ctxp,
	char*			rmn,
	poid_t			**acnt_pdp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_check_csr_data_protection(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flist,
	pin_flist_t		**access_info,
	poid_t			**profile_obj,
	pin_errbuf_t		*ebufp);


extern void
fm_mso_get_acnt_from_acntno(
	pcm_context_t		*ctxp,
	char*			acnt_no,
	pin_flist_t		**acnt_details,
	pin_errbuf_t		*ebufp);

static void
fm_mso_get_hierarchy_info(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	int32			account_model,
	int32			corporate_type,
	int32			subscriber_type,
	pin_flist_t		**hierarchy_info,
	pin_errbuf_t		*ebufp);

static void
fm_mso_get_deposit_details(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_flist_t		*srvc_info,
	pin_flist_t		**deposit_info,
	pin_errbuf_t		*ebufp);

static void
fm_mso_get_balance(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	poid_t			*billinfo_obj,
	pin_flist_t		**balance_flist,
	pin_errbuf_t		*ebufp);

static void
fm_mso_populate_org_structure(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flist,
	pin_errbuf_t		*ebufp);

static int
fm_mso_get_mso_plans(
	pcm_context_t		*ctxp,
	poid_t			*account_obj,
	pin_flist_t		**plans_flistp,
	pin_errbuf_t		*ebufp);


static void
fm_mso_get_lco_settlement_balance(
	pcm_context_t		*ctxp,
	poid_t			*account_obj,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static  int
fm_mso_get_commission_model(
	pcm_context_t		*ctxp,
	poid_t			*account_obj,
	pin_errbuf_t		*ebufp);

static void
fm_mso_get_credit_fup_limit(
        pcm_context_t           *ctxp,
	poid_t                 *acnt_pdp,
        char	             	*ser_city,
        int32	             	*ser_billwhen,
        pin_flist_t             **cl_fup,
        pin_errbuf_t            *ebufp);
		
static int
fm_mso_get_cust_corp_type(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_get_mso_purp_details(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_flist_t		*srvc_info,
	pin_errbuf_t		*ebufp);
	
static int
fm_mso_get_ncr_balance(
	pcm_context_t		*ctxp,
	pin_flist_t			*srvc_info,
	pin_flist_t			**cl_fup,
	pin_errbuf_t		*ebufp);

extern void
fm_mso_check_csr_access_level(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*acnt_info,
	pin_flist_t		**ret_flist,
	pin_errbuf_t		*ebufp);

static void
fm_mso_get_device_from_ser(
	pcm_context_t		*ctxp,
	poid_t			*ser_pdp,
	pin_flist_t		**ret_flist,
	pin_errbuf_t		*ebufp);

static void
fm_mso_get_profile_offer(
	pcm_context_t		*ctxp,
	poid_t			*ser_pdp,
	pin_flist_t		**ret_flist,
	pin_errbuf_t		*ebufp);

static void
fm_mso_get_suppression_profile(
	pcm_context_t		*ctxp,
	poid_t			*acc_pdp,
	pin_flist_t		**ret_flist,
	pin_errbuf_t		*ebufp);

static void
fm_mso_get_csr_ar_credit_limit(
	pcm_context_t		*ctxp,
	poid_t			*acc_pdp,
	pin_flist_t		**ret_flist,
	pin_errbuf_t		*ebufp);


static void
fm_mso_get_subcription_plan(
        pcm_context_t           *ctxp,
        poid_t                  *acnt_pdp,
	poid_t 			*plan_pdp,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_mso_populate_gst_info(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**ret_flist,
	pin_errbuf_t	*ebufp);

PIN_IMPORT int
fm_mso_cust_get_gst_profile(
        pcm_context_t           *ctxp,
        poid_t                  *account_pdp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

PIN_IMPORT void
fm_mso_is_child_service(
        pcm_context_t           *ctxp,
        poid_t                  *serv_pdp,
	int32			parent_info_flag,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

//RETRIEVE NETFLIX BALANCES
void
fm_mso_cust_get_netf_bal(
        pcm_context_t           *ctxp,
        poid_t                  *account_pdp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

PIN_EXPORT int32
fm_mso_get_wallet_balance(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	int32			wallet_res_id,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

PIN_IMPORT void
fm_mso_retrieve_service_netflix(
	pcm_context_t	*ctxp, 
	poid_t			*act_pdp, 
	pin_flist_t		**ret_flistpp, 
	pin_errbuf_t	*ebufp);

PIN_IMPORT pin_decimal_t *
get_ar_curr_total(
        pcm_context_t   *ctxp,
        poid_t          *ar_billinfo_pdp,
        pin_errbuf_t    *ebufp);

static void
fm_mso_srch_device_sp_name_comp(
	pcm_context_t           *ctxp,
	char                    *source,
	pin_flist_t             **ret_flistp,
	pin_errbuf_t            *ebufp);

void
fm_mso_get_bill_address_mod(
	pcm_context_t		*ctxp,
	poid_t			*act_pdp,
	pin_flist_t		**ret_flist,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * MSO_OP_CUST_GET_CUSTOMER_INFO 
 *
 *
 *
 * 
 *******************************************************************/

EXPORT_OP void
op_mso_cust_get_customer_info(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_get_customer_info(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

void 
fm_mso_get_cmts_mac(
	pcm_context_t	*ctxp,
	poid_t		*acnt_pdp,
	pin_flist_t	**out_flistp,
	pin_errbuf_t	*ebufp);


/*******************************************************************
 * MSO_OP_CUST_GET_CUSTOMER_INFO  
 *******************************************************************/
void 
op_mso_cust_get_customer_info(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	*r_flistpp		= NULL;
	poid_t			*inp_pdp = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_GET_CUSTOMER_INFO) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_get_customer_info error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_get_customer_info input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	fm_mso_cust_get_customer_info(ctxp, flags, i_flistp, r_flistpp, ebufp);
 	/***********************************************************
	 * Results.
	 ***********************************************************/

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_get_customer_info error", ebufp);
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_get_customer_info output flist", *r_flistpp);
	}
	return;
}



/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_get_customer_info(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{

	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*nameinfo_installation = NULL;
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*srvc_info = NULL; 
	pin_flist_t		*validate_out_flist = NULL; 
	pin_flist_t		*purch_prod_details = NULL; 
	pin_flist_t		*device_details = NULL;
	pin_flist_t		*bill_details = NULL;
	pin_flist_t		*org_structure = NULL;
	pin_flist_t		*hierarchy_info = NULL;
	pin_flist_t		*deposit_info = NULL;
	pin_flist_t		*nameinfo = NULL;
	pin_flist_t		*mso_plans_flistp = NULL;
	pin_flist_t		*pckg_flistp = NULL;
	pin_flist_t		*pckg_elem_flistp = NULL;
	pin_flist_t		*lco_balance_flistp = NULL;
	pin_flist_t             *cl_fup = NULL;
	pin_flist_t             *supp_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*bb_info = NULL;
	pin_flist_t		*access_info_flistp = NULL;
	pin_flist_t		*catv_info = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*csr_ar_info = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*exemp_flistp = NULL;
	pin_flist_t		*acc_info_all = NULL;
	pin_flist_t		*ws_access_info = NULL;
	pin_flist_t		*gst_info_flistp = NULL;
	pin_flist_t		*srch_offer_iflistp = NULL;
	pin_flist_t		*srch_offer_oflistp = NULL;
	pin_flist_t		*mac_flistp = NULL;
	pin_flist_t		*gst_flistp = NULL;
	pin_flist_t		*wallet_flistp = NULL;
    pin_flist_t     *corp_parent_flistp = NULL;

	/* Changes to return service type (BB/CATV) */
	pin_flist_t     *serv_flistp = NULL;
	int32           serv_type = -1;
    int32           pp_type = -1;

	poid_t			*acnt_pdp = NULL;
	poid_t			*user_id = NULL;
	poid_t			*srvc_pdp = NULL;
	poid_t			*profile_obj = NULL;
	poid_t                  *service_pdp = NULL;
    poid_t          *parent_billinfo_pdp = NULL;

	char			*acnt_no = NULL;
	char			*card_no = NULL;
	char			*rmn = NULL;
	char			data_access[100];
	char			*access_str = NULL;
	char			*access_level = NULL;
	char			*access_value = NULL;
	char			*existing_access_value = NULL;
	char			msg[200];
	char			msg1[200];
	char			*address = NULL; 
	char            *ser_city = NULL;
	char            *service_type = NULL;
    char            leg_acc[20] = "";
    char            *program_namep = NULL;
	int             *ser_billwhen = NULL;

	int64			db = -1;
	int32			*flag = NULL;
	int32			check_data_protection = 0;
	int32			srch_customer_failure = 1;
	int32			srch_customer_success = 0;
	int32			account_type = 0;
	int32			customer_type = 0;
	int32			*business_type = NULL;
	int32			tmp_business_type = 0;
	int32			account_model = 0;
	int32			subscriber_type = 0;
	int32			acnt_status_activated =0;
	int32			new_type_csr = 0;  
	int32			corporate_type = 0;
	int32			*serv_status = NULL;
	int32			is_active_status = 0;
	int32			acc_no_val = 0;

	int			elem_id=0;
	int			gstin=0;
	int                     elem_id1=0;
	int			res_cntr = 0;
	int			res_val = 0;
	int			type = 0;
	int			arr_business_type[8];
        char                    *gstin1 = NULL;
        time_t                  *effect_t=NULL;
        time_t          *end_t = NULL;
        int64           *int_end_t = NULL;
	pin_cookie_t		cookie = NULL;
	pin_cookie_t            cookie1 = NULL;
	pin_decimal_t		*zero = pbo_decimal_from_str("0.0",ebufp);
    pin_decimal_t       *parent_curr_balp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_customer_info input flist", i_flistp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_cust_get_customer_info", ebufp);
		goto CLEANUP;
	}

	user_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 1, ebufp);
	acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	acnt_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp );
    program_namep = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
    if (acnt_no == NULL)
    {
        acnt_no = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_LEGACY_ACCOUNT_NO, 1, ebufp);
        if (program_namep && strstr(program_namep, "DVB"))
        {    
            memset(leg_acc, 0, sizeof(leg_acc));
            sprintf(leg_acc, "J-%s", acnt_no);
        }        
    }
	card_no = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_VC_ID, 1, ebufp );
	if (card_no == NULL)
	{
		card_no = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_STB_ID, 1, ebufp);
	}
	rmn = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_RMN, 1, ebufp );
	flag = (int32*)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp );
	db = PIN_POID_GET_DB(user_id);

	if (!(leg_acc && strstr(leg_acc, "J-")) && (acnt_no && (strlen(acnt_no) != 10 || sscanf(acnt_no, "%d", &acc_no_val) != 1)))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Invalid Account Number Format") ;
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, user_id, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51100", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Invalid Account Number", ebufp);
		goto CLEANUP;
	}

	if (!(leg_acc && strstr(leg_acc, "J-")) && acc_no_val != 0 && (acc_no_val < 1000000000 || acc_no_val > 2000000000))	
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Invalid Account Number") ;
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, user_id, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51101", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Invalid Account Number", ebufp);
		goto CLEANUP;
	}	

//	if (flag &&(*flag <0 || *flag >3))
	if(flag && (*flag <0 || *flag > 6))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Invalid Flag") ;
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, user_id, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51108", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Invalid Flag", ebufp);
		goto CLEANUP;
	}

	if (!user_id || (user_id && strcmp(user_id, "")==0))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Missing or NULL PIN_FLD_USERID") ;
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, user_id, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51140", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Missing or NULL PIN_FLD_USERID", ebufp);
		goto CLEANUP;
	}

	/*********************************************************
	* Retrieve Account Poid	to be searched
	* End
	**********************************************************/
	if (acnt_pdp && (!PIN_POID_IS_NULL(acnt_pdp)))
	{
		PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_customer_info acnt_pdp", acnt_pdp);
	}
	else if (acnt_no && strcmp(acnt_no, "") !=0 )
	{
		//acnt_pdp = PIN_POID_CREATE(db, "/account", atoi(acnt_no), ebufp);
        if (leg_acc && strstr(leg_acc, "J-"))
        {
            fm_mso_get_acnt_from_acntno(ctxp, leg_acc, &acnt_info, ebufp);
        }
        else
        {
		    fm_mso_get_acnt_from_acntno(ctxp, acnt_no, &acnt_info, ebufp);
        }
		//acnt_pdp = PIN_POID_COPY( PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_POID, 0, ebufp), ebufp);
		if (acnt_info == NULL)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account Number not in our system") ;
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, user_id, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51102", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Invalid Account Number", ebufp);
			goto CLEANUP;
		}
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "fm_mso_cust_get_customer_info: in flistp", i_flistp);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_cust_get_customer_info:fm_mso_get_acnt_from_acntno error", ebufp);
			goto CLEANUP;
		}
		if (acnt_info)
		{
			acnt_pdp = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_POID, 0, ebufp);
		}
		strcpy(msg, "Account Number: ");
		strcat(msg, acnt_no );
	}
	else if (card_no && strcmp(card_no, "")!=0 )
	{
		strcpy(msg, "Card Number: ");
		strcat(msg, card_no );
		fm_mso_get_acnt_from_vc(ctxp, card_no, &acnt_pdp, ebufp);
		if (acnt_pdp == NULL)
		{
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, user_id, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51103", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account Number not found for this device", ebufp);
			goto CLEANUP;
		}
	}
	else if (rmn && strcmp(rmn, "")!=0 )
	{
		strcpy(msg, "Mobile Number: ");
		strcat(msg, rmn );
		fm_mso_get_acnt_from_rmn(ctxp, rmn, &acnt_pdp, ebufp);
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "One of 'Account_no', 'Card_no', 'rmn' is required") ;
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, user_id, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51141", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "One of 'Account_no', 'Card_no', 'rmn' is required", ebufp);
		goto CLEANUP;
	}
	if (acnt_pdp)
	{
		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		goto CLEANUP;
	}
	if(*flag == 4)
	{
		fm_mso_get_cmts_mac(ctxp, acnt_pdp, &mac_flistp, ebufp);
		ret_flistp = PIN_FLIST_COPY(mac_flistp, ebufp);
		goto CLEANUP;
	}
 	/*********************************************************
	* Retrieve Account Poid	to be searched
	* End
	**********************************************************/

	if ((PIN_POID_COMPARE(user_id, acnt_pdp, 0, ebufp ) != 0) && // CSR is searching another account
	     (flag && (*flag == GET_ALL || *flag == GET_UPASS_ALL || *flag == GET_ALL_SKIP_CAN))	  //All data is required
	   )  
	{
		/*******************************************************************
		* Validate user_id who is going to perform the search
		*******************************************************************/
		fm_mso_validate_user_id(ctxp, user_id, acnt_pdp, &validate_out_flist, ebufp);
		/* validate_out_flist will have USER details */
		if (!validate_out_flist)
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Non existant poid in PIN_FLD_USERID", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, user_id, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51142", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Non existant poid in PIN_FLD_USERID", ebufp);
			goto CLEANUP;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "validate_out_flist", validate_out_flist);
		access_str = (char*)PIN_FLIST_FLD_GET(validate_out_flist, MSO_FLD_DATA_ACCESS, 1, ebufp );
		customer_type = *(int32*)PIN_FLIST_FLD_GET(validate_out_flist, PIN_FLD_CUSTOMER_TYPE, 0, ebufp );

		if ( customer_type == ACCT_TYPE_SALES_PERSON ||
		     customer_type == ACCT_TYPE_COLLECTION_AGENT 
		   )
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"USER has no prevelige for search another account", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, user_id, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51143", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "USER has no prevelige for search another account", ebufp);
			goto CLEANUP;
		}
//		 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST_0001");
		/*******************************************************************
		* Data protection based on org structure
		* Start
		*******************************************************************/
		fm_mso_check_csr_data_protection(ctxp, i_flistp , &ret_flistp, &ws_access_info, &profile_obj, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "CSR Data Prot Return flist", ret_flistp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Self Access Info", ws_access_info);
		PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Profile poid", profile_obj);
//		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST_0002");
		if (!ret_flistp)
		{
			fm_mso_check_csr_access_level(ctxp, i_flistp , acnt_info, &ret_flistp, ebufp);
			if (!ret_flistp)
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in CSR access to this account : ORG_STRUCTURE: returns null", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, user_id, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51144", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "CSR has no access to this account :ORG_STRUCTURE", ebufp);
				goto CLEANUP;
			}
			else
			{
				check_data_protection = 0;
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_check_csr_access_level Return flist", ret_flistp);
			}
		}
		if ( PIN_ERRBUF_IS_ERR(ebufp) )
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in CSR access to this account : ORG_STRUCTURE : error", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, user_id, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51145", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in CSR access to this account : ORG_STRUCTURE", ebufp);
			goto CLEANUP;
		}
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_PROFILE_OBJ, profile_obj, ebufp );
		PIN_FLIST_FLD_PUT(i_flistp, PIN_FLD_PROFILE_OBJ, profile_obj, ebufp );
		org_structure = PIN_FLIST_COPY(ret_flistp, ebufp);
		/* If CSR has MSO_FLD_ACCESS_INFO Array then it is New Type i.e data access 
			validation to be done based on MSO_FLD_ACCESS_INFO array.
			Otherwise it should be done as per old logic. */
		//new_type_csr = PIN_FLIST_ELEM_COUNT(org_structure,MSO_FLD_ACCESS_INFO,ebufp);
		access_info_flistp = PIN_FLIST_ELEM_TAKE(org_structure,MSO_FLD_ACCESS_INFO,0,1,ebufp);
		/* For New Type, set check_data_protection to 0 since it should be check only for Old Type.*/
		if(access_info_flistp)
			check_data_protection = 0;
		PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);


		/*******************************************************************
		* Data protection based on  org structure
		* End
		*******************************************************************/

		/*******************************************************************
		* Data protection based on  MSO_FLD_DATA_ACCESS 
		* Start
		*******************************************************************/
		if(!access_info_flistp && 
			(customer_type == ACCT_TYPE_CSR_ADMIN || customer_type == ACCT_TYPE_CSR) ) 
			// Only for Old CSR (created before BB implementation)
		{
			if (access_str && (strcmp(access_str, "") ==0 || strcmp(access_str, "*") ==0 ) &&
				 ( customer_type == ACCT_TYPE_CSR_ADMIN ||
				   customer_type == ACCT_TYPE_CSR
				 )
			   )
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "access_str:");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, access_str);
				check_data_protection = 0;
			}
			else if (access_str)
			{
				//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST_0001");
				//data_access = pin_malloc(sizeof(data_access));
				memset(data_access, '\0',sizeof(data_access));
				strcpy(data_access, access_str);

				access_level = strtok(data_access, "|");
				access_value = strtok(NULL, "|");

				if (!access_level || !access_value)
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Incorrect Data Access Value", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					ret_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, user_id, ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51146", ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Incorrect Data Access Value", ebufp);
					goto CLEANUP;
				}
				else
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "access_level:");
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, access_level);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "access_value:");
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, access_value);
				}
			}
		}
		/*******************************************************************
		* Data protection based on  MSO_FLD_DATA_ACCESS
		* Start
		*******************************************************************/
	}
	else if ((PIN_POID_COMPARE(user_id, acnt_pdp, 0, ebufp ) == 0) && // Self login
	         (flag && ( *flag == GET_ALL || *flag == GET_UPASS_ALL || *flag == GET_ALL_SKIP_CAN) )	  //All data is required
	        )
	{
		check_data_protection = 0;
		fm_mso_populate_org_structure(ctxp,i_flistp, &ret_flistp,ebufp);
		if (!ret_flistp)
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"No Profile exist for the account", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, user_id, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51147", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "No Profile exist for the account", ebufp);
			goto CLEANUP;
		}
		if ( PIN_ERRBUF_IS_ERR(ebufp) )
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in account profile search", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, user_id, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51148", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in account profile search", ebufp);
			goto CLEANUP;
		}
		PIN_FLIST_FLD_COPY(ret_flistp, PIN_FLD_PROFILE_OBJ, i_flistp, PIN_FLD_PROFILE_OBJ, ebufp );
		org_structure = PIN_FLIST_COPY(ret_flistp, ebufp);
		access_info_flistp = PIN_FLIST_ELEM_TAKE(org_structure,MSO_FLD_ACCESS_INFO,0,1,ebufp);
		PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);

		// Calling  fm_mso_validate_user_id() to get  MSO_FLD_DATA_ACCESS
		fm_mso_validate_user_id(ctxp, user_id, NULL, &validate_out_flist, ebufp);
	}
	else
	{
		check_data_protection = 0;
	}

	/*********************************************************
	* Retrieve Nameinfo -START
	**********************************************************/
	if (!acnt_info)
	{
		fm_mso_get_account_info(ctxp, acnt_pdp, &acnt_info, ebufp );
	}

	// Call to find Billing Address Modified date
	fm_mso_get_bill_address_mod(ctxp, acnt_pdp,&srch_offer_oflistp,ebufp);
	if(srch_offer_oflistp != NULL && PIN_FLIST_ELEM_COUNT(srch_offer_oflistp,PIN_FLD_RESULTS,ebufp) > 0)
	{
		srch_offer_iflistp=PIN_FLIST_ELEM_GET(srch_offer_oflistp,PIN_FLD_RESULTS,0,1,ebufp);
	        end_t = PIN_FLIST_FLD_GET(srch_offer_iflistp,PIN_FLD_MOD_T,0,ebufp);
	        int_end_t = (int64 *)end_t;
       	 	/*Add one second to end_t
        	*int_end_t = *int_end_t + 86400;*/
        	*int_end_t = *int_end_t + 19800;
        	end_t = (time_t *)int_end_t;

		PIN_FLIST_FLD_SET(acnt_info, PIN_FLD_LAST_POSTED_T, end_t, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(acnt_info, PIN_FLD_LAST_POSTED_T, (time_t *)0, ebufp);
	}
	srch_offer_iflistp = (pin_flist_t *)NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error calling fm_mso_get_account_info", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, user_id, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51149", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error calling fm_mso_get_account_info", ebufp);
		goto CLEANUP;
	}
	if (acnt_info  &&
	    (flag && ( *flag == GET_ALL || *flag == GET_UPASS_ALL || *flag == GET_ALL_SKIP_CAN)) 
	   )
	{
	  // PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "00000");
	   if (check_data_protection)
	   {
	//	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "00000");
		nameinfo_installation = PIN_FLIST_ELEM_GET(acnt_info, PIN_FLD_NAMEINFO, 2, 1, ebufp);
		if (!nameinfo_installation) //No installation nameinfo exist for internal users
		{
			nameinfo_installation = PIN_FLIST_ELEM_GET(acnt_info, PIN_FLD_NAMEINFO, PIN_ELEMID_ANY, 0, ebufp);
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"acnt_info",acnt_info);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"nameinfo_installation",nameinfo_installation);
		//Validate Data Access Previlege
		if (strcmp(access_level, "STATE") ==0 )
		{
	//		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "11111");
			existing_access_value = (char*)PIN_FLIST_FLD_GET(nameinfo_installation, PIN_FLD_STATE, 0, ebufp);
	//		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "121212");
		}
		else if (strcmp(access_level, "CITY") ==0 )
		{
	//		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "22222");
			existing_access_value = (char*)PIN_FLIST_FLD_GET(nameinfo_installation, PIN_FLD_CITY, 0, ebufp);
		}
		else if (strcmp(access_level, "ZIP") ==0 )
		{
	//		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "22223");
			existing_access_value = (char*)PIN_FLIST_FLD_GET(nameinfo_installation, PIN_FLD_ZIP, 0, ebufp);
		}
		else
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Invalid Access Level", ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, user_id, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51150", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Invalid Access Level", ebufp);
			goto CLEANUP;
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "access_value:");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, access_value);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "existing_access_value:");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, existing_access_value);
		if ( !existing_access_value || strcmp(access_value, existing_access_value) != 0)
		{
	//		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "33333");
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"CSR has no access to this customer: DATA_ACCESS", ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, user_id, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51151", ebufp);
			strcpy(msg1,"CSR has no access to this customer: DATA_ACCESS" );
			strcat(msg1,msg);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, msg1, ebufp);
			goto CLEANUP;
		}
	//	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "44444");
	   }
		/*******************************************************************
		* Derive account_type,account_model & subscriber_type from business_type 
		*  - Start
		*******************************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"acnt_info",acnt_info);
		business_type = (int32 *)PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_BUSINESS_TYPE, 1, ebufp);
		if (business_type)
		{
			tmp_business_type = *business_type;
		}
//		account_type = tmp_business_type/1000000;
//
//		//reserved_bits = tmp_business_type%1000;
//		tmp_business_type = tmp_business_type/1000;
//
//		account_model = tmp_business_type%10;
//
//		tmp_business_type = tmp_business_type/10;
//		subscriber_type =  tmp_business_type%100;

		num_to_array(tmp_business_type, arr_business_type);
		account_type    = 10*(arr_business_type[0])+arr_business_type[1];	// First 2 digits
		subscriber_type = 10*(arr_business_type[2])+arr_business_type[3];	// next 2 digits
		account_model   = arr_business_type[4];				// 5th field
		corporate_type  = arr_business_type[6];				// 7th field


		/************************************************************************
		* Derive account_type,account_model & subscriber_type from business_type 
		* END
		************************************************************************/
		if ( account_model == ACCOUNT_MODEL_CORP )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling fm_mso_get_hierarchy_info");
			fm_mso_get_hierarchy_info(ctxp, acnt_pdp, account_model, corporate_type, subscriber_type, &hierarchy_info, ebufp);
		}

		//Modify Address to replace \n by |
		while ((nameinfo = PIN_FLIST_ELEM_GET_NEXT(acnt_info, PIN_FLD_NAMEINFO,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			address = PIN_FLIST_FLD_GET(nameinfo, PIN_FLD_ADDRESS, 1, ebufp);
			if (address && strcmp(address, "")!=0)
			{
				replace_char(address, '\n', '|');
			}
		}
	
	}
	else if (!acnt_info)
	{
		strcat(msg, " Does not exist in BRM");
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,msg, ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, user_id, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51152", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, msg, ebufp);
		goto CLEANUP;
	}
	/*********************************************************
	* Retrieve Nameinfo -END
	**********************************************************/

 	/*********************************************************
	* Retrieve Service Details
	* Start
	**********************************************************/
	if (flag && (
	    *flag == GET_ALL || 
	    *flag == GET_UPASS_ALL ||
	    *flag == GET_INACTIVE_SERVICE ||
	    *flag == GET_CANCELLED_SERVICE ||
        *flag == LIGHT_WEIGHT ||
	    *flag == GET_ALL_SKIP_CAN)
	   )
	{
	//	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "0000");
		fm_mso_get_service_info(ctxp, user_id, acnt_pdp, &srvc_info, *flag, ebufp );
	//	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "0000");
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
	//		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "0000");
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error calling fm_mso_get_service_info", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&purch_prod_details, NULL);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, user_id, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51153", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error calling fm_mso_get_service_info", ebufp);
			goto CLEANUP;
		}
		if (srvc_info)
		{
			acnt_status_activated=1;
		}

		
	}
	/*********************************************************
	* Retrieve Service Details
	* End
	**********************************************************/

 	/*********************************************************
	* Retrieve Plan Details
	* Start
	**********************************************************/
	if ((flag && (*flag == GET_ALL ||
	    *flag == GET_UPASS_ALL ||
	    *flag == GET_INACTIVE_SERVICE ||
	    *flag == GET_CANCELLED_SERVICE ||
        *flag == LIGHT_WEIGHT ||
	    *flag == GET_ALL_SKIP_CAN)) && 
	    acnt_status_activated
	   )
	{
		fm_mso_get_purch_prod_details(ctxp, acnt_pdp, &srvc_info, *flag, ebufp );
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "Input flist is ", i_flistp);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_WARNING, "Error calling fm_mso_get_purch_prod_details:", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, user_id, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51154", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error calling fm_mso_get_purch_prod_details", ebufp);
			goto CLEANUP;
		}
		fm_mso_get_mso_purp_details(ctxp, acnt_pdp, srvc_info, ebufp );
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error calling fm_mso_get_mso_purp_details:", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, user_id, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51801", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error calling fm_mso_get_mso_purp_details", ebufp);
			goto CLEANUP;
		}		
		
	}
	/*********************************************************
	* Retrieve Plan Details
	* End
	**********************************************************/

 	/*********************************************************
	* Retrieve Bill Details
	* Start
	**********************************************************/
	if (flag && (*flag == GET_ALL || *flag == GET_UPASS_ALL || *flag == GET_ALL_SKIP_CAN) )
	{
		fm_mso_get_bill_details(ctxp, acnt_pdp, &bill_details, ebufp );
	//	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST_0001");
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error calling fm_mso_get_bill_details", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, user_id, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51155", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error calling fm_mso_get_bill_details", ebufp);
			goto CLEANUP;
		}
	//	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST_0001");
	}
	/*********************************************************
	* Retrieve Bill Details
	* End
	**********************************************************/

	/* Get the tax exemption details */
	if (flag && (
	    *flag == GET_ALL || 
	    *flag == GET_UPASS_ALL ||
	    *flag == GET_INACTIVE_SERVICE ||
	    *flag == GET_CANCELLED_SERVICE ||
	    *flag == GET_ALL_SKIP_CAN)
	   )
	{
		fm_mso_get_account_info(ctxp, acnt_pdp, &acc_info_all, ebufp );
	//	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST_0001");
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error calling fm_mso_get_account_info", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, user_id, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51157", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error calling fm_mso_get_account_info", ebufp);
			goto CLEANUP;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Account info", acc_info_all );
	//	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST_0001");
		if(acc_info_all)
		{
			exemp_flistp = PIN_FLIST_CREATE(ebufp);
			cookie = NULL;
			elem_id = 0;
			while((tmp_flistp = PIN_FLIST_ELEM_GET_NEXT(acc_info_all, PIN_FLD_EXEMPTIONS,
				&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
			{
				PIN_FLIST_ELEM_COPY(acc_info_all, PIN_FLD_EXEMPTIONS, elem_id,
						exemp_flistp, PIN_FLD_EXEMPTIONS, elem_id, ebufp);
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Exemption flist", exemp_flistp );

		}
	}
	/* Get the tax exemption details END */


 	/*********************************************************
	* Retrieve Device details
	* Start
	**********************************************************/
	if (acnt_status_activated)
	{
	/*	fm_mso_get_device_details(ctxp, acnt_pdp, &device_details, ebufp );
			if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, user_id, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51000", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error calling fm_mso_get_device_details:", ebufp);
			goto CLEANUP;
		}
	*/
	}
	/*********************************************************
	* Retrieve Device details
	* End
	**********************************************************/
 	/*********************************************************
	* Retrieve deposit information
	**********************************************************/
	if( account_type == ACCT_TYPE_SUBSCRIBER && acnt_status_activated ) 
	{
		srvc_pdp = PIN_FLIST_FLD_GET(srvc_info, PIN_FLD_POID, 0, ebufp);
		fm_mso_get_deposit_details(ctxp, acnt_pdp, srvc_info, &deposit_info, ebufp );
	//	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST_0001");
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error calling fm_mso_get_deposit_details", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, user_id, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51156", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error calling fm_mso_get_deposit_details", ebufp);
			goto CLEANUP;
		}
	//	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST_0001");
	}
	
	/**********************************************************
	* Get Credit Limit, FUP Limit and Free Usage for Broadband
	***********************************************************/
        if ((flag && (*flag == GET_ALL ||
            *flag == GET_UPASS_ALL ||
            *flag == GET_INACTIVE_SERVICE ||
            *flag == GET_CANCELLED_SERVICE  || 
	    *flag == GET_ALL_SKIP_CAN)) &&
            acnt_status_activated
           )
        {
		res_flistp = NULL;
		bb_info = NULL;
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Service info flist", srvc_info);
		
		res_flistp = PIN_FLIST_ELEM_GET(srvc_info, PIN_FLD_SERVICES, 0, 1,  ebufp );
 		while ((res_flistp = PIN_FLIST_ELEM_GET_NEXT(srvc_info,
                        PIN_FLD_SERVICES, &elem_id1, 1, &cookie1, ebufp)) != NULL)
		{	
		  serv_status = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_STATUS, 1, ebufp); 
		  service_pdp = (poid_t *)PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_POID, 0, ebufp);
                  PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "service_pdp", service_pdp);
                  service_type = (char *)PIN_POID_GET_TYPE(service_pdp);
		  if (serv_status && *serv_status == 10100) {
			
			is_active_status = 1;	  	
	
		  }	
                  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "service_type:");
                  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, service_type);

		  if((serv_status && *serv_status != 10103) || (serv_status && is_active_status != 1 && 
			 service_type && strcmp(service_type,"/service/telco/broadband")== 0 && *serv_status==10103)) 
	  	  {
			bb_info = PIN_FLIST_SUBSTR_GET(res_flistp, MSO_FLD_BB_INFO, 1, ebufp );
			if(bb_info) {
				ser_city = PIN_FLIST_FLD_GET(bb_info, PIN_FLD_CITY, 1, ebufp);
				ser_billwhen = PIN_FLIST_FLD_GET(bb_info, PIN_FLD_BILL_WHEN, 1, ebufp);
                   fm_mso_get_credit_fup_limit(ctxp, acnt_pdp, ser_city,ser_billwhen, &cl_fup, ebufp );
                   if (PIN_ERRBUF_IS_ERR(ebufp))
                   {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error calling fm_mso_get_credit_fup_limit:", ebufp);
                        PIN_ERRBUF_RESET(ebufp);
                        ret_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, user_id, ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51161", ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error calling fm_mso_get_credit_fup_limit", ebufp);
                        goto CLEANUP;
                }
				/* Add current balance for each resource.*/
				res_val = fm_mso_get_ncr_balance(ctxp, srvc_info, &cl_fup, ebufp);
                if (res_val)
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_get_ncr_balance");
                        PIN_ERRBUF_RESET(ebufp);
                        ret_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, user_id, ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51161", ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Fetching Current balance", ebufp);
                        goto CLEANUP;
                }
				
				/* Add current balance for each resource.*/
				fm_mso_get_suppression_profile(ctxp, acnt_pdp, &supp_flistp, ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_get_suppression_profile", ebufp);
                        PIN_ERRBUF_RESET(ebufp);
                        ret_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, user_id, ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51191", ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Fetching Suppression profile", ebufp);
                        goto CLEANUP;
                }				
				
			}
			catv_info = PIN_FLIST_SUBSTR_GET(res_flistp, MSO_FLD_CATV_INFO, 1, ebufp );
			if(catv_info) {
				cl_fup = PIN_FLIST_CREATE(ebufp);
				args_flistp = PIN_FLIST_ELEM_ADD(cl_fup, MSO_FLD_CREDIT_PROFILE, 356, ebufp );
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CREDIT_FLOOR, zero, ebufp);
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CREDIT_LIMIT, zero, ebufp);
				/* Add current balance for each resource.*/
				res_val = fm_mso_get_ncr_balance(ctxp, srvc_info, &cl_fup, ebufp);
                if (res_val)
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_get_ncr_balance");
                        PIN_ERRBUF_CLEAR(ebufp);
                        ret_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, user_id, ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51161", ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Fetching Current balance", ebufp);
                        goto CLEANUP;
                }
			}
			
		}
	     }
      }	
	/*****************************************
	CSR AR cresit limit
	*****************************************/
	if( account_type == ACCT_TYPE_CSR || account_type == ACCT_TYPE_CSR_ADMIN ) 
	{
		fm_mso_get_csr_ar_credit_limit(ctxp, acnt_pdp, &csr_ar_info, ebufp);
	}


	/*******************************************
	* GST INFO
	*******************************************/
	//fm_mso_populate_gst_info(ctxp, i_flistp, &gst_info_flistp, ebufp);
	fm_mso_cust_get_gst_profile(ctxp, acnt_pdp,  &gst_info_flistp, ebufp);
         //gst_flistp = PIN_FLIST_ELEM_GET(gst_info_flistp, MSO_FLD_GST_INFO, 0, PIN_ELEMID_ANY, ebufp);
        // gstin = PIN_FLIST_FLD_GET(gst_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);

	if (gst_info_flistp)
	{
		gst_flistp = PIN_FLIST_ELEM_GET(gst_info_flistp, MSO_FLD_GST_INFO, 0, PIN_ELEMID_ANY, ebufp);
                gstin1 = PIN_FLIST_FLD_GET(gst_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
                PIN_FLIST_FLD_SET(gst_flistp, MSO_FLD_GSTIN, gstin1, ebufp);
                effect_t =PIN_FLIST_FLD_GET(gst_info_flistp,PIN_FLD_EFFECTIVE_T,0,ebufp);
                PIN_FLIST_FLD_SET(gst_flistp,PIN_FLD_EFFECTIVE_T,effect_t, ebufp);
		PIN_FLIST_FLD_DROP(gst_info_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_DROP(gst_info_flistp, PIN_FLD_CREATED_T, ebufp);
		PIN_FLIST_FLD_DROP(gst_info_flistp, PIN_FLD_MOD_T, ebufp);
		PIN_FLIST_FLD_DROP(gst_info_flistp, PIN_FLD_READ_ACCESS, ebufp);
		PIN_FLIST_FLD_DROP(gst_info_flistp, PIN_FLD_WRITE_ACCESS, ebufp);
		PIN_FLIST_FLD_DROP(gst_info_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
		PIN_FLIST_FLD_DROP(gst_info_flistp, PIN_FLD_EFFECTIVE_T, ebufp);
		PIN_FLIST_FLD_DROP(gst_info_flistp, PIN_FLD_NAME, ebufp);
		PIN_FLIST_FLD_DROP(gst_info_flistp, PIN_FLD_OBJECT_CACHE_TYPE, ebufp);
		PIN_FLIST_FLD_DROP(gst_info_flistp, PIN_FLD_SERVICE_OBJ, ebufp);

	}

	PIN_ERR_LOG_FLIST(3, "GST Flistp", gst_info_flistp);

 	/*********************************************************
	* Retrieve Payment History
	**********************************************************/
 	/*********************************************************
	* Retrieve Adjustment History
	**********************************************************/
 	/*********************************************************
	* Retrieve Trouble Ticket History
	**********************************************************/
 	/*********************************************************
	* Transaction History
	**********************************************************/

	/********************************************************* 
	 * Errors..?
	 *********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_cust_get_customer_info error",ebufp);
	}
	

	/*********************************************************
	* Modify Return Flist depending on flag
	* Start
	**********************************************************/
//	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST_0001");
	//Nameinfo
	if (flag && (*flag == GET_ALL || *flag == GET_UPASS_ALL || *flag == GET_ALL_SKIP_CAN))
	{
		ret_flistp = PIN_FLIST_COPY(acnt_info, ebufp);
	}
	else
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
		if ( (*flag == GET_INACTIVE_SERVICE || *flag == GET_CANCELLED_SERVICE) &&
		       !srvc_info
		   )
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG, "No service exist", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51157", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "No service exist", ebufp);
			goto CLEANUP;
		}
	}
	if (validate_out_flist)
	{
		PIN_FLIST_FLD_COPY(validate_out_flist, PIN_FLD_ACCESS_LEVEL, ret_flistp, MSO_FLD_DATA_ACCESS, ebufp);
		PIN_FLIST_FLD_COPY(validate_out_flist, PIN_FLD_LOGIN, ret_flistp, PIN_FLD_LOGIN, ebufp);
		PIN_FLIST_FLD_COPY(validate_out_flist, PIN_FLD_PASSWD, ret_flistp, PIN_FLD_PASSWD, ebufp);
	}

	//Organosation Structure
	if (org_structure)
	{
		PIN_FLIST_SUBSTR_PUT(ret_flistp, org_structure, MSO_FLD_ORG_STRUCTURE, ebufp);
	}
	//Account Hierarchy
	if (hierarchy_info)
	{
		//PIN_FLIST_FLD_DROP(hierarchy_info, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_MOVE(hierarchy_info, PIN_FLD_PARENT_FLAGS, ret_flistp, PIN_FLD_PARENT_FLAGS, ebufp );
		PIN_FLIST_SUBSTR_PUT(ret_flistp, hierarchy_info, PIN_FLD_GROUP_INFO, ebufp);
	}
	//Services
	if (srvc_info)
	{
		PIN_FLIST_FLD_DROP(srvc_info, PIN_FLD_POID, ebufp );
		PIN_FLIST_SUBSTR_SET(ret_flistp, srvc_info, PIN_FLD_SERVICE_INFO, ebufp );

                //Changes to poulate the SERVICE TYPE (0-CATV / 1-BB)
                if (PIN_FLIST_ELEM_COUNT(srvc_info, PIN_FLD_SERVICES, ebufp) != 0) {
                        serv_flistp = PIN_FLIST_ELEM_GET(srvc_info, PIN_FLD_SERVICES, 0, PIN_ELEMID_ANY, ebufp);
                        service_pdp = PIN_FLIST_FLD_GET(serv_flistp, PIN_FLD_POID, 0, ebufp);

                        if ( service_pdp != NULL && strcmp ( PIN_POID_GET_TYPE(service_pdp), "/service/catv") == 0) {
                                serv_type = 0;
                        }
                        else if ( service_pdp != NULL && strcmp ( PIN_POID_GET_TYPE(service_pdp), "/service/telco/broadband") == 0) {
                                serv_type = 1;
                        }
                        PIN_FLIST_FLD_SET(ret_flistp, MSO_FLD_SERVICE_TYPE, &serv_type, ebufp);
		}

	}
	//Devices
	/*if (device_details)
	{
		PIN_FLIST_FLD_RENAME(device_details, PIN_FLD_RESULTS, PIN_FLD_DEVICES, ebufp);
		PIN_FLIST_SUBSTR_PUT(ret_flistp, device_details, PIN_FLD_DEVICE_INFO, ebufp );
	}*/
	// Bill Details
	if (bill_details)
	{
		PIN_FLIST_CONCAT(ret_flistp, bill_details, ebufp);
 		PIN_FLIST_DESTROY_EX(&bill_details, NULL);
	}
	//deposit Details
	if (deposit_info)
	{
		//PIN_FLIST_FLD_DROP(deposit_info, PIN_FLD_POID, ebufp );
		PIN_FLIST_SUBSTR_PUT(ret_flistp, deposit_info, PIN_FLD_DEPOSIT, ebufp );
	}
	
	// Access info
	if (access_info_flistp)
	{
		 PIN_FLIST_ELEM_SET(ret_flistp, access_info_flistp, MSO_FLD_ACCESS_INFO, 0, ebufp);
	}
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROFILE_OBJ, ret_flistp, PIN_FLD_PROFILE_OBJ, ebufp );

	if (csr_ar_info)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "csr_ar_info", csr_ar_info );
		PIN_FLIST_ELEM_PUT(ret_flistp, csr_ar_info, MSO_FLD_AR_INFO, 0, ebufp);
	}

	//Tax Exemption info
	if(exemp_flistp)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Tax Exmption info", exemp_flistp );
		PIN_FLIST_CONCAT(ret_flistp, exemp_flistp, ebufp);
	}
	
	//GST info
	if(gst_info_flistp)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "GST INFO", gst_info_flistp);
		PIN_FLIST_CONCAT(ret_flistp, gst_info_flistp, ebufp);
	}

        /*******************************************
        * POI/POA Details
        *******************************************/
        if(acnt_info)
        {
                PIN_FLIST_FLD_COPY(acnt_info, PIN_FLD_ACCESS_CODE1, ret_flistp, PIN_FLD_ACCESS_CODE1, ebufp);
                PIN_FLIST_FLD_COPY(acnt_info, PIN_FLD_ACCESS_CODE2, ret_flistp, PIN_FLD_ACCESS_CODE2, ebufp);
        }
	

	//Self Access Info
	if (ws_access_info)
	{
		 PIN_FLIST_ELEM_SET(ret_flistp, ws_access_info, MSO_FLD_ACCESS_INFO, 0, ebufp);
	}
        /*******************************************************************
         * Get Wallet resource balance
         ******************************************************************/
	if (account_type == ACCT_TYPE_SUBSCRIBER) 
	{
		/*res_val = fm_mso_get_wallet_balance(ctxp, acnt_pdp, 2300000, &wallet_flistp, ebufp);
		if (res_val)
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error calling fm_mso_get_wallet_balance", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, user_id, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51161", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Fetching Wallet balance", ebufp);
			goto CLEANUP;
		}*/
        wallet_flistp = PIN_FLIST_SUBSTR_GET(ret_flistp, MSO_FLD_ORG_STRUCTURE, 1, ebufp);
		if (wallet_flistp)
		{
            pp_type = *(int32 *)PIN_FLIST_FLD_GET(wallet_flistp, MSO_FLD_PP_TYPE, 1, ebufp);
            if (pp_type == 0)
            {
                wallet_flistp = PIN_FLIST_ELEM_GET(ret_flistp, PIN_FLD_BILLINFO, PIN_ELEMID_ANY, 1, ebufp);
                parent_billinfo_pdp = PIN_FLIST_FLD_GET(wallet_flistp, PIN_FLD_AR_BILLINFO_OBJ, 0, ebufp);
                parent_curr_balp = get_ar_curr_total(ctxp, parent_billinfo_pdp, ebufp);
                wallet_flistp = PIN_FLIST_ELEM_ADD(ret_flistp, PIN_FLD_BALANCES, 2300000, ebufp);
                PIN_FLIST_FLD_PUT(wallet_flistp, PIN_FLD_CURRENT_BAL, parent_curr_balp, ebufp);

                //PIN_FLIST_DESTROY_EX(&corp_parent_flistp, NULL);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "WALLET output flist", wallet_flistp);
            }
		}
    }

	/*********************************************************
	* Modify Return Flist depending on flag
	* end
	**********************************************************/
	// Pawan: Added for Broadband
	// Credit limit, FUP limit, Free Usage and Usage details
	if (cl_fup)
	{
			PIN_FLIST_CONCAT(ret_flistp, cl_fup, ebufp);
			PIN_FLIST_DESTROY_EX(&cl_fup, NULL);
	}

	//RETRIEVE AND STORE NETFLIX BALANCES
	pin_flist_t *nf_flistp = NULL;
	fm_mso_cust_get_netf_bal(ctxp, acnt_pdp, &nf_flistp, ebufp);
	if (nf_flistp)
	{
			PIN_FLIST_CONCAT(ret_flistp, nf_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&nf_flistp, NULL);
	}


	if (supp_flistp)
	{
			PIN_FLIST_CONCAT(ret_flistp, supp_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&supp_flistp, NULL);
	}
	
	res_cntr = fm_mso_get_mso_plans( ctxp, acnt_pdp, &mso_plans_flistp, ebufp);	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Mso plans activation search completed");
	if (PIN_ERRBUF_IS_ERR(ebufp) || res_cntr == 1 )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error calling fm_mso_get_mso_plans", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, user_id, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51643", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in getting mso plans activation list", ebufp);
		goto CLEANUP;
	}

	if ( res_cntr == 2 && mso_plans_flistp )
	{
		pckg_flistp = PIN_FLIST_CREATE(ebufp);
		pckg_elem_flistp = PIN_FLIST_ELEM_ADD(ret_flistp, PIN_FLD_PACKAGE_INFO, 0, ebufp);
		PIN_FLIST_CONCAT(pckg_elem_flistp, mso_plans_flistp, ebufp);
		PIN_FLIST_CONCAT(ret_flistp, pckg_flistp, ebufp);
 		PIN_FLIST_DESTROY_EX(&mso_plans_flistp, NULL);
 		PIN_FLIST_DESTROY_EX(&pckg_flistp, NULL);
	}

	/*** Pavan Bellala 27-07-2015 *******************
	Fix for returning balances for DTR and SDTR also
	************************************************/

//	if( account_type == ACCT_TYPE_LCO ) {
	if( account_type == ACCT_TYPE_LCO || account_type == ACCT_TYPE_DTR || account_type == ACCT_TYPE_SUB_DTR ) {

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Its a LCO account");
		fm_mso_get_lco_settlement_balance (ctxp, acnt_pdp, &lco_balance_flistp, ebufp);

		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error calling fm_mso_get_lco_settlemet_balance", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, user_id, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51158", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error calling fm_mso_get_lco_settlemet_balance", ebufp);
			goto CLEANUP;
		}
	}
	/*appending lco balance details */
	if (lco_balance_flistp) {
		PIN_FLIST_CONCAT(ret_flistp, lco_balance_flistp, ebufp);
 		PIN_FLIST_DESTROY_EX(&lco_balance_flistp, NULL);
	}
	
	/***********************************************************
	* Adding DATA/DISCOUNT offer details if any
	***********************************************************/
	srch_offer_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(srch_offer_iflistp, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_offer_iflistp, PIN_FLD_TYPE, &type, ebufp); // Type == 0 to get all the offer details
	fm_mso_search_offer_entries(ctxp, srch_offer_iflistp, &srch_offer_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_offer_iflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error calling fm_mso_get_lco_settlemet_balance", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, user_id, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51158", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error calling fm_mso_get_lco_settlemet_balance", ebufp);
		goto CLEANUP;
	}
	if(srch_offer_oflistp && PIN_FLIST_ELEM_COUNT(srch_offer_oflistp, PIN_FLD_RESULTS, ebufp) > 0 ){
		PIN_FLIST_FLD_COPY(srch_offer_oflistp, PIN_FLD_RESULTS, ret_flistp, PIN_FLD_OFFER, ebufp);
		PIN_FLIST_DESTROY_EX(&srch_offer_oflistp, NULL);
	}

	/*********************************************************
	* Modify Return Flist to add profile_cust_offers
	* end
	**********************************************************/
	fm_mso_get_profile_offer(ctxp, acnt_pdp, &srch_offer_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_profile_offer output flist", srch_offer_oflistp);
	cookie = NULL;
	elem_id = 0;
	while((tmp_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_offer_oflistp, PIN_FLD_PROFILE_CUST_OFFER,&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		PIN_FLIST_ELEM_COPY(srch_offer_oflistp, PIN_FLD_PROFILE_CUST_OFFER, elem_id,
		ret_flistp, PIN_FLD_PROFILE_CUST_OFFER, elem_id, ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "FINAL OUTPUT FLIST AFTER ADDITION OF PROFILE OFFER INFO", ret_flistp );

        /*********************************************************
        * Modify Return Flist depending on flag
        * end
        **********************************************************/
        //Catch Unknown error
        CLEANUP:
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_cust_activate_customer error",ebufp);
                fm_mso_get_err_desc(ctxp, acnt_pdp, &ret_flistp, ebufp);
                //goto CLEANUP;
        }

	PIN_FLIST_DESTROY_EX(&acnt_info, NULL);	 
//	PIN_FLIST_DESTROY_EX(&org_structure, NULL);	
	*r_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
	 PIN_FLIST_DESTROY_EX(&validate_out_flist, NULL);
	PIN_FLIST_DESTROY_EX(&acc_info_all, NULL);
	PIN_FLIST_DESTROY_EX(&mac_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&srvc_info, NULL);
	if (!pbo_decimal_is_null(zero, ebufp)){
        	pbo_decimal_destroy(&zero);
	}
	return;
}


/**************************************************
* Function: 	fm_mso_get_service_info()
* Owner:        Gautam Adak
* Decription:   For getting service information
*               
*		
* 
* 
***************************************************/
static void
fm_mso_get_service_info(
	pcm_context_t		*ctxp,
	poid_t			*user_id,
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flistp,
	int32			flag,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*out_flistp = NULL;
	pin_flist_t		*catv_flistp = NULL;

	pin_cookie_t		cookie = NULL;

	poid_t			*srch_pdp = NULL;
	poid_t			*service_pdp = NULL;
	poid_t			*acc_pdp = NULL;
	int32			srch_flag = 512;
	int32			srvc_status_active = PIN_STATUS_ACTIVE;
	int32			srvc_status_inactive = PIN_STATUS_INACTIVE;
	int32			srvc_status_cancelled = PIN_STATUS_CLOSED;
	int32			elem_id = 0;
	int32			*conn_typep = NULL;

	int64			db = -1;

	//Netflix Changes
	char			*template = "select X from /service where F1.id = V1 and (F2.type = V2 or F3.type = V3 or F4.type = V4 or F5.type = V5 ) and F6 = V6 order by service_t.status,service_t.created_t asc" ;
	char			*template_1 = "select X from /service where F1.id = V1 and (F2.type = V2 or F3.type = V3 or F4.type = V4 or F5.type = V5) order by service_t.status,service_t.created_t asc" ;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_service_info", ebufp);
		return;
	}
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_service_info acnt_pdp", acnt_pdp);

	db = PIN_POID_GET_DB(acnt_pdp) ;
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/service/catv", -1, ebufp), ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
	PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/service/telco/broadband", -1, ebufp), ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp );
	PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/service/settlement/ws/outcollect", -1, ebufp), ebufp);

        //Netflix Changes
        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 5, ebufp );
        PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/service/netflix", -1, ebufp), ebufp);
        //Netflix Changes

	if (flag == GET_ALL || flag == GET_ALL_SKIP_CAN || flag == LIGHT_WEIGHT)
	{
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template_1 , ebufp);
	}
	else if (flag == GET_UPASS_ALL )
	{
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 6, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STATUS, &srvc_status_active, ebufp);
	}
	else if (flag == GET_INACTIVE_SERVICE)
	{
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 6, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STATUS, &srvc_status_inactive, ebufp);
	}
	else if (flag == GET_CANCELLED_SERVICE)
	{
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 6, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STATUS, &srvc_status_cancelled, ebufp);
	}
	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
    if (flag == LIGHT_WEIGHT)
    {
        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_BAL_GRP_OBJ, NULL, ebufp);
        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_CREATED_T, NULL, ebufp);
        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_CLOSE_WHEN_T, NULL, ebufp);
        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_EFFECTIVE_T, NULL, ebufp);
        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_LOGIN, NULL, ebufp);
        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STATUS, NULL, ebufp);
        PIN_FLIST_SUBSTR_SET(result_flist, NULL, MSO_FLD_CATV_INFO, ebufp);
        PIN_FLIST_ELEM_SET(result_flist,   NULL, MSO_FLD_PERSONAL_BIT_INFO, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_ELEM_SET(result_flist,   NULL, PIN_FLD_ALIAS_LIST, PIN_ELEMID_ANY, ebufp);
    }

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_service_info read input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_service_info error", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_service_info READ output list", srch_oflistp);
	if (PIN_FLIST_ELEM_COUNT(srch_oflistp, PIN_FLD_RESULTS, ebufp) == 0)
	{
		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
		return;
	}
	PIN_FLIST_FLD_RENAME(srch_oflistp, PIN_FLD_RESULTS, PIN_FLD_SERVICES, ebufp);
	while ((result_flist = PIN_FLIST_ELEM_GET_NEXT(srch_oflistp,
		PIN_FLD_SERVICES, &elem_id, 1, &cookie, ebufp)) != NULL)
	{    
		service_pdp = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_POID, 0, ebufp);
		catv_flistp = PIN_FLIST_SUBSTR_GET(result_flist, MSO_FLD_CATV_INFO, 1, ebufp);
		if (catv_flistp)
		{    
			fm_mso_is_child_service(ctxp, service_pdp, 1, &out_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_is_child_service return flist", out_flistp);
			if (out_flistp)
			{    
				conn_typep = PIN_FLIST_FLD_TAKE(out_flistp, PIN_FLD_CONN_TYPE, 0, ebufp);
				PIN_FLIST_FLD_PUT(catv_flistp, PIN_FLD_CHILDREN, conn_typep, ebufp);

				acc_pdp = PIN_FLIST_FLD_TAKE(out_flistp, MSO_FLD_PARENT_ACCT_NO, 0, ebufp);
				PIN_FLIST_FLD_PUT(catv_flistp, MSO_FLD_PARENT_ACCT_NO, acc_pdp, ebufp);
			}    
			PIN_FLIST_DESTROY_EX(&out_flistp, NULL);
		}    
	}     

	*ret_flistp = PIN_FLIST_COPY(srch_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);

	// Add Device details
	fm_mso_get_device_details(ctxp, user_id, ret_flistp, ebufp); 

	CLEANUP:
	return;
}


/**************************************************
* Function: 	fm_mso_validate_user_id()
* Owner:        Gautam Adak
* Decription:   
*               
*		
* 
* 
***************************************************/
static void
fm_mso_validate_user_id(
	pcm_context_t		*ctxp,
	poid_t			*user_id,
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*res_flist = NULL;

	poid_t			*srch_pdp = NULL;
	poid_t			*result_acnt_pdp = NULL;

	int32			srch_flag = 512;
	int32			searching_self =1;
	int32			elem_id = 0;

	pin_cookie_t		cookie = NULL;

	int64			db = -1;

	char			*template = "select X from /mso_customer_credential where F1.id = V1 " ;
	char			*template_1 = "select X from /mso_customer_credential where F1.id in ( V1, V2 ) " ;
	char			*vp = NULL;
	char			data_access[100] = "";
	char			*passwd = NULL;
	char			*login = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_validate_user_id", ebufp);
		return;
	}
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_user_id user_id", user_id);

	if (acnt_pdp)
	{
		searching_self =0;
	}

	db = PIN_POID_GET_DB(user_id) ;
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	
	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, user_id, ebufp);
	
	if (searching_self)
	{
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template_1 , ebufp);
		
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);
	}


	PIN_FLIST_ELEM_SET(srch_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_user_id read input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_validate_user_id search error", ebufp);
        PIN_ERRBUF_RESET(ebufp);
        PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_user_id READ output list", srch_oflistp);
	
	if (searching_self)
	{
		result_flist = PIN_FLIST_ELEM_TAKE(srch_oflistp, PIN_FLD_RESULTS, 0, 1, ebufp );
		PIN_FLIST_FLD_COPY(result_flist, MSO_FLD_DATA_ACCESS, result_flist, PIN_FLD_ACCESS_LEVEL, ebufp);
	}
	else
	{
		while((res_flist = PIN_FLIST_ELEM_GET_NEXT(srch_oflistp, PIN_FLD_RESULTS,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			result_acnt_pdp = PIN_FLIST_FLD_GET(res_flist, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
			if (PIN_POID_COMPARE(result_acnt_pdp, acnt_pdp, 0, ebufp) ==0 )
			{
				vp = (char *)PIN_FLIST_FLD_GET(res_flist, MSO_FLD_DATA_ACCESS, 0, ebufp);
				memset(data_access,'\0',sizeof(data_access));
				strcpy(data_access,vp);

				login = PIN_FLIST_FLD_GET(res_flist, PIN_FLD_LOGIN, 0, ebufp);
				passwd = PIN_FLIST_FLD_GET(res_flist, PIN_FLD_PASSWD, 0, ebufp);
			}
			else if (PIN_POID_COMPARE(result_acnt_pdp, user_id, 0, ebufp) ==0)
			{
				result_flist = PIN_FLIST_COPY(res_flist, ebufp);
			}
		}
		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCESS_LEVEL, data_access, ebufp);
		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_LOGIN, login, ebufp);
		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_PASSWD, passwd, ebufp);
		//PIN_FLD_ACCESS_LEVEL will contain the data_access of the searched account
		//MSO_FLD_DATA_ACCESS  will contain the data_access of CSR
	}

	
	if (result_flist )
	{
		*ret_flistp = result_flist;
	}

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	return;
}

/**************************************************
* Function: 	fm_mso_get_purch_prod_details()
* Owner:        Gautam Adak
* Decription:   
*               
*		
* 
* 
***************************************************/
static void
fm_mso_get_purch_prod_details(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_flist_t		**srvc_info,
	int32			flags,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*services_array = NULL;
	pin_flist_t		*purch_prod_array = NULL;
	pin_flist_t		*purchased_product = NULL;
	pin_flist_t		*plan_list = NULL;
	pin_flist_t		*plan_list_active = NULL;
	pin_flist_t		*plan_list_inactive = NULL;
	pin_flist_t		*plan_list_cancel = NULL;
	pin_flist_t		*plans = NULL;
	pin_flist_t		*tmp_flistp = NULL;

	poid_t			*srch_pdp = NULL;
	poid_t			*service_pdp_1 = NULL;
	poid_t			*service_pdp_2 = NULL;
	poid_t			*plan_obj_1 = NULL;
	poid_t			*plan_obj_2 = NULL;
	poid_t			*sprod_pdp = NULL;
	poid_t			*prod_pdp = NULL;
	poid_t			*plan_pdp = NULL;
	poid_t			*curr_service_pdp = NULL;	
	poid_t			*main_service_pdp = NULL;	

	int32			srch_flag = 256;
	int32			pp_status_active = 1;
        int32                   *pur_status= 0;
	int32			index ;
	int32			plan_count = 0;
	int32			pp_count = 0;
	int32			elem_id = 0;
	int32			elem_id_1 = 0;
	int32			elem_id_2 = 0;
	int32			srch_elem_id = 0;
	int32			main_srch_elem_id = 0;
	int32			*purch_prod_status = NULL;
	int32			*purch_prod_status_flags = NULL;
	int32			*pkg_id1 = NULL;
	int32			*pkg_id2 = NULL;

	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie_1 = NULL;
	pin_cookie_t		cookie_2 = NULL;
	pin_cookie_t		srch_cookie = NULL;
	pin_decimal_t		*disc_per = NULL;
	pin_decimal_t		*disc_amt = NULL;
	pin_decimal_t		*dis_per_mod = NULL;
	pin_decimal_t		*hundred = NULL;
	
	int64			db = -1;

	char                    *p_tag = NULL;
//	char			*template = "select X from /purchased_product where F1.id = V1 and F2 = V2 " ;
	char			*template = "select X from /purchased_product where F1.id = V1 " ;
//	char                    *template = "select X from /purchased_product where F1.id = V1 and ( purchased_product_t.status in (1,2) or  (purchased_product_t.status =3 and purchased_product_t.purchase_end_t > otc(to_char(sysdate - 29,'dd-mon-yyyy') ) ) or (purchased_product_t.poid_id0 member of mso_get_pur_bb_prod(purchased_product_t.account_obj_id0)) )  ";
	char                    *template1 = "select X from /purchased_product where F1.id = V1 and ( purchased_product_t.status in (1,2) or  (purchased_product_t.status =3 and purchased_product_t.purchase_end_t > otc(to_char(sysdate - 29,'dd-mon-yyyy') ) ) or (purchased_product_t.poid_id0 member of mso_get_pur_prod(purchased_product_t.account_obj_id0)) )  ";
	pin_flist_t		*s_plan_flistp = NULL;
	pin_flist_t		*s_product_flistp = NULL;
	pin_flist_t		*sort_flistp = NULL;
	pin_flist_t		*sprod_flistp = NULL;	
	pin_flist_t	        *tmp_srch_oflistp = NULL;
	pin_flist_t		*srch_result_array = NULL;
	pin_flist_t			*cycle_fee_flist = NULL;
	pin_flist_t			*main_cycle_fee_flist = NULL;
	pin_flist_t			*main_pp_flist = NULL;
	time_t          	*cycle_end_t = NULL;
	time_t          	*grant_cycle_end_t = NULL;
	time_t          	*purchase_end_t = NULL;
	int32                   s_flags = 0;
	int32                   p_count = 0;
	void			*vp = NULL;
	//Changes to make plan Name 
	pin_flist_t		*read_flistp = NULL;
	pin_flist_t		*read_oflistp = NULL;
	char                 *p_name = NULL;
	char                    *descrp = NULL;

    //PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_purch_prod_details read input list",*srvc_info);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_purch_prod_details", ebufp);
		return;
	}
    PIN_ERRBUF_CLEAR(ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_purch_prod_details read input list",*srvc_info);
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_purch_prod_details acnt_pdp", acnt_pdp);
	hundred = pbo_decimal_from_str("100.0",ebufp);
	db = PIN_POID_GET_DB(acnt_pdp) ;
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
    PIN_ERRBUF_RESET(ebufp);

	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);

        if (flags == GET_ALL_SKIP_CAN )
        {
                PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template1 , ebufp);
        }
        else
        {
                PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template, ebufp);
        }

	//PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STATUS, &pp_status_active, ebufp);
	
	PIN_FLIST_ELEM_SET(srch_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_purch_prod_details read input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_purch_prod_details search error", ebufp);
        PIN_ERRBUF_RESET(ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_purch_prod_details READ output list", srch_oflistp);
 	
	if (PIN_FLIST_ELEM_COUNT(srch_oflistp, PIN_FLD_RESULTS, ebufp) == 0)
	{
		goto CLEANUP;
	}

	 tmp_srch_oflistp = PIN_FLIST_COPY(srch_oflistp,ebufp);
	 while((srch_result_array = PIN_FLIST_ELEM_GET_NEXT(tmp_srch_oflistp, PIN_FLD_RESULTS,
                &srch_elem_id, 1, &srch_cookie, ebufp)) != (pin_flist_t *)NULL)
         {
	     	
	 	prod_pdp = PIN_FLIST_FLD_GET(srch_result_array, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
		plan_pdp = PIN_FLIST_FLD_GET(srch_result_array, PIN_FLD_PLAN_OBJ, 1, ebufp);	
		pur_status = PIN_FLIST_FLD_GET(srch_result_array, PIN_FLD_STATUS, 1, ebufp);
		curr_service_pdp = PIN_FLIST_FLD_GET(srch_result_array, PIN_FLD_SERVICE_OBJ, 0, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch_result_array list", srch_result_array);
		fm_mso_get_subcription_plan(ctxp, acnt_pdp, plan_pdp,&sprod_flistp, ebufp); 		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_subscription_plan return list", sprod_flistp);
		sprod_pdp = PIN_FLIST_FLD_GET(sprod_flistp, PIN_FLD_POID, 0, ebufp);		
		vp = PIN_FLIST_FLD_GET(sprod_flistp,PIN_FLD_STATUS_FLAGS, 0, ebufp);

		if (vp)
	          s_flags = *(int32*)vp;
		p_tag = PIN_FLIST_FLD_GET(sprod_flistp,PIN_FLD_PROVISIONING_TAG,0,ebufp);
		if ((p_tag && (strcmp(p_tag, "" ) == 0)) || (PIN_POID_COMPARE(sprod_pdp,prod_pdp, 0, ebufp) == 0 && s_flags == 1)){
		
			PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_subscription_plan input acnt_pdp", sprod_pdp);		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Found Actual Subscription product");	
			if(pur_status && *pur_status == 1 && p_tag && strcmp(p_tag, "" ) > 0 )
			{
				main_srch_elem_id = srch_elem_id;
				main_pp_flist = PIN_FLIST_COPY(srch_result_array, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "main_pp_flist active", main_pp_flist);				
				if(PIN_FLIST_ELEM_COUNT(srch_result_array,PIN_FLD_CYCLE_FEES,ebufp)>0)
				{
					main_cycle_fee_flist =  PIN_FLIST_ELEM_GET(srch_result_array, PIN_FLD_CYCLE_FEES, PIN_ELEMID_ANY, 0, ebufp);
				}
			}


		} 		
		else 
		{
			if(pur_status && *pur_status == 1)
			{
				if(PIN_FLIST_ELEM_COUNT(srch_result_array,PIN_FLD_CYCLE_FEES,ebufp)>0)
				{
					cycle_fee_flist = PIN_FLIST_ELEM_GET(srch_result_array, PIN_FLD_CYCLE_FEES, PIN_ELEMID_ANY, 0, ebufp);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "cycle_fee_flist ", cycle_fee_flist);
					
					if (cycle_fee_flist)
					{
						grant_cycle_end_t = PIN_FLIST_FLD_GET(cycle_fee_flist, PIN_FLD_CYCLE_FEE_END_T, 0, ebufp);						
					}
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch_result_array list", srch_result_array);
				}
				
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"actual inside else",srch_oflistp);
                        PIN_FLIST_ELEM_DROP(srch_oflistp, PIN_FLD_RESULTS, srch_elem_id, ebufp);	
		}
		if (grant_cycle_end_t &&  main_pp_flist && p_count == 0 )
		{
			main_service_pdp = PIN_FLIST_FLD_GET(main_pp_flist, PIN_FLD_SERVICE_OBJ, 0, ebufp);			
			if (PIN_POID_COMPARE(main_service_pdp, curr_service_pdp, 0, ebufp) == 0 )
			{
				p_count = 1;
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "main_pp_flist before", main_pp_flist);
				PIN_FLIST_FLD_SET(main_cycle_fee_flist,PIN_FLD_EXPIRATION_T, grant_cycle_end_t, ebufp);
				PIN_FLIST_ELEM_SET(main_pp_flist, main_cycle_fee_flist, PIN_FLD_CYCLE_FEES, 1, ebufp ); 
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "main_pp_flist after ", main_pp_flist );
				PIN_FLIST_ELEM_SET(srch_oflistp, main_pp_flist, PIN_FLD_RESULTS, main_srch_elem_id, ebufp);	
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch_oflistp after ", srch_oflistp);
			}
		}


		
	 }	
	 
	
	//Insert plans and purchased_product insise service
	while((services_array = PIN_FLIST_ELEM_GET_NEXT(*srvc_info, PIN_FLD_SERVICES,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Browsing Service");
		plan_list = PIN_FLIST_CREATE(ebufp);
		plan_list_active = PIN_FLIST_CREATE(ebufp);
		plan_list_inactive = PIN_FLIST_CREATE(ebufp);
		plan_list_cancel = PIN_FLIST_CREATE(ebufp);

		service_pdp_1 = PIN_FLIST_FLD_GET(services_array, PIN_FLD_POID, 0, ebufp);
		elem_id_1 = 0;
		cookie_1  = NULL;
		//index = -1;
		while((purch_prod_array = PIN_FLIST_ELEM_GET_NEXT(srch_oflistp, PIN_FLD_RESULTS,
			&elem_id_1, 1, &cookie_1, ebufp)) != (pin_flist_t *)NULL)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Browsing Purchased Product");
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PLAN1 flist", purch_prod_array);
			service_pdp_2 = PIN_FLIST_FLD_GET(purch_prod_array, PIN_FLD_SERVICE_OBJ, 0, ebufp);
			plan_obj_1 =  PIN_FLIST_FLD_GET(purch_prod_array, PIN_FLD_PLAN_OBJ, 0, ebufp);
			pkg_id1 = PIN_FLIST_FLD_GET(purch_prod_array, PIN_FLD_PACKAGE_ID, 0, ebufp);
			purch_prod_status = PIN_FLIST_FLD_GET(purch_prod_array, PIN_FLD_STATUS, 0, ebufp);
			index = -1;
			if (PIN_POID_COMPARE(service_pdp_1, service_pdp_2, 0, ebufp) == 0 )
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "service_pdp_1=service_pdp_2");
				elem_id_2 = 0;
				cookie_2 = NULL;
				if (purch_prod_status && *purch_prod_status == 1)
				{
					while((plans = PIN_FLIST_ELEM_GET_NEXT(plan_list_active, PIN_FLD_PLAN,
						&elem_id_2, 1, &cookie_2, ebufp)) != (pin_flist_t *)NULL)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "browsing plan");
						plan_obj_2 =  PIN_FLIST_FLD_GET(plans, PIN_FLD_PLAN_OBJ, 0, ebufp);
						tmp_flistp = PIN_FLIST_ELEM_GET(plans, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY,1, ebufp );
						pkg_id2 = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_PACKAGE_ID, 0, ebufp);
						if (( PIN_POID_COMPARE(plan_obj_1, plan_obj_2, 0, ebufp) == 0  )
							&& pkg_id1 && pkg_id2 && *pkg_id1 == *pkg_id2 )
						{
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PLAN flist", purch_prod_array);
							PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Plan obj in purch_prod=", plan_obj_1);
							PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Plan obj in existing plan_list_active=", plan_obj_2);
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,  "plan_obj_1=plan_obj_2");
							index = elem_id_2;
							break;
						}
					}
					if (index == -1)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "index == -1");
						plans = PIN_FLIST_ELEM_ADD(plan_list_active, PIN_FLD_PLAN, plan_count, ebufp );
						plan_count++;
					}
					else
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "index != -1");
						plans = PIN_FLIST_ELEM_GET(plan_list_active, PIN_FLD_PLAN, elem_id_2, 0, ebufp );
					}

					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_PLAN_OBJ, plans, PIN_FLD_PLAN_OBJ, ebufp);

						//Changes to add Plan Name
						if(PIN_FLIST_FLD_GET(purch_prod_array, PIN_FLD_PLAN_OBJ, 1, ebufp) && PIN_FLIST_FLD_GET(purch_prod_array, PIN_FLD_PLAN_OBJ, 1, ebufp) != NULL)
						{
						read_flistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_SET(read_flistp,PIN_FLD_POID, PIN_FLIST_FLD_GET(purch_prod_array, PIN_FLD_PLAN_OBJ, 1, ebufp), ebufp);
						PIN_FLIST_FLD_SET(read_flistp,PIN_FLD_NAME,NULL, ebufp);
						PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flistp, &read_oflistp, ebufp);	
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "READ OBJ FOR PLAN ADDITION", read_oflistp);
						PIN_FLIST_FLD_SET(plans, PIN_FLD_NAME,PIN_FLIST_FLD_GET(read_oflistp, PIN_FLD_NAME, 1, ebufp),ebufp);
						}
					
					purchased_product = PIN_FLIST_ELEM_ADD(plans, PIN_FLD_PRODUCTS, pp_count, ebufp );

					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_POID,              purchased_product, PIN_FLD_POID, ebufp);
					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_PURCHASE_START_T,  purchased_product, PIN_FLD_PURCHASE_START_T, ebufp);
					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_PURCHASE_END_T,    purchased_product, PIN_FLD_PURCHASE_END_T, ebufp);
					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_USAGE_START_T,     purchased_product, PIN_FLD_USAGE_START_T, ebufp);
					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_CYCLE_START_T,     purchased_product, PIN_FLD_CYCLE_START_T, ebufp);
					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_EFFECTIVE_T,       purchased_product, PIN_FLD_EFFECTIVE_T, ebufp);
					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_STATUS,            purchased_product, PIN_FLD_STATUS, ebufp);
					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_STATUS_FLAGS,       purchased_product, PIN_FLD_STATUS_FLAGS, ebufp);
					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_DESCR,             purchased_product, PIN_FLD_DESCR, ebufp);
					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_PACKAGE_ID,        purchased_product, PIN_FLD_PACKAGE_ID, ebufp);
					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_DEAL_OBJ,          purchased_product, PIN_FLD_DEAL_OBJ, ebufp);
					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_PRODUCT_OBJ,       purchased_product, PIN_FLD_PRODUCT_OBJ, ebufp);

					purch_prod_status_flags = PIN_FLIST_FLD_GET(purch_prod_array, PIN_FLD_STATUS_FLAGS, 0, ebufp);
                                        if (purch_prod_status_flags && (*purch_prod_status_flags == 0xFFFF))
                                        {    
                                                cycle_end_t = PIN_FLIST_FLD_GET(purch_prod_array, PIN_FLD_CYCLE_END_T, 0, ebufp);
                                                purchase_end_t = PIN_FLIST_FLD_GET(purch_prod_array, PIN_FLD_PURCHASE_END_T, 0, ebufp);
                                                if (cycle_end_t && purchase_end_t && (*purchase_end_t > *cycle_end_t))
                                                {    
                                                        PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_PURCHASE_END_T, purchased_product, PIN_FLD_VALID_END_TO, ebufp);
                                                        PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_CYCLE_END_T, purchased_product, PIN_FLD_PURCHASE_END_T, ebufp);
                                                }            
                                        }
					// Added to set actual grace end
	                                else if (purch_prod_status_flags &&(*purch_prod_status_flags == 0xFFF))
                                        {
                                                cycle_end_t = PIN_FLIST_FLD_GET(purch_prod_array, PIN_FLD_CYCLE_END_T, 0, ebufp);
                                                purchase_end_t = PIN_FLIST_FLD_GET(purch_prod_array, PIN_FLD_PURCHASE_END_T, 0, ebufp);
                                                if (cycle_end_t && purchase_end_t && (*purchase_end_t > *cycle_end_t))
                                                {
                                                        PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_USAGE_END_T, purchased_product, PIN_FLD_VALID_END_TO, ebufp);
                                                        PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_CYCLE_END_T, purchased_product, PIN_FLD_PURCHASE_END_T, ebufp);
                                                }
                                        }
					//Added to set actual grace changed end
					PIN_FLIST_ELEM_COPY(purch_prod_array, PIN_FLD_CYCLE_FEES, 1, purchased_product, PIN_FLD_CYCLE_FEES, 1, ebufp);
					disc_per = PIN_FLIST_FLD_GET(purch_prod_array, PIN_FLD_CYCLE_DISCOUNT, 1, ebufp);
					if (disc_per && !pbo_decimal_is_zero(disc_per,ebufp))
					{
						dis_per_mod = pbo_decimal_multiply(disc_per, hundred, ebufp);
						PIN_FLIST_FLD_PUT(plans, MSO_FLD_DISC_PERCENT, dis_per_mod, ebufp);
					}
					disc_amt = PIN_FLIST_FLD_GET(purch_prod_array, PIN_FLD_CYCLE_DISC_AMT, 1, ebufp);
					if (disc_amt && !pbo_decimal_is_zero(disc_amt,ebufp))
					{
						PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_CYCLE_DISC_AMT, plans, MSO_FLD_DISC_AMOUNT, ebufp);
					}
					pp_count++;
				}
				else if (purch_prod_status && *purch_prod_status == 2)
				{
					while((plans = PIN_FLIST_ELEM_GET_NEXT(plan_list_inactive, PIN_FLD_PLAN,
						&elem_id_2, 1, &cookie_2, ebufp)) != (pin_flist_t *)NULL)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "browsing plan");
						plan_obj_2 =  PIN_FLIST_FLD_GET(plans, PIN_FLD_PLAN_OBJ, 0, ebufp);
						tmp_flistp = PIN_FLIST_ELEM_GET(plans, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY,1, ebufp );
						pkg_id2 = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_PACKAGE_ID, 0, ebufp);
						if (( PIN_POID_COMPARE(plan_obj_1, plan_obj_2, 0, ebufp) == 0  )
							&& pkg_id1 && pkg_id2 && *pkg_id1 == *pkg_id2 )
						{
							PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Plan obj in purch_prod=", plan_obj_1);
							PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Plan obj in existing plan_list_inactive=", plan_obj_2);
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,  "plan_obj_1=plan_obj_2");
							index = elem_id_2;
							break;
						}
					}
					if (index == -1)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "index == -1");
						plans = PIN_FLIST_ELEM_ADD(plan_list_inactive, PIN_FLD_PLAN, plan_count, ebufp );
						plan_count++;
					}
					else
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "index != -1");
						plans = PIN_FLIST_ELEM_GET(plan_list_inactive, PIN_FLD_PLAN, elem_id_2, 0, ebufp );
					}

					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_PLAN_OBJ, plans, PIN_FLD_PLAN_OBJ, ebufp);

						//Changes to add Plan Name
						if(PIN_FLIST_FLD_GET(purch_prod_array, PIN_FLD_PLAN_OBJ, 1, ebufp) && PIN_FLIST_FLD_GET(purch_prod_array, PIN_FLD_PLAN_OBJ, 1, ebufp) != NULL)
						{
						read_flistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_SET(read_flistp,PIN_FLD_POID, PIN_FLIST_FLD_GET(purch_prod_array, PIN_FLD_PLAN_OBJ, 1, ebufp), ebufp);
						PIN_FLIST_FLD_SET(read_flistp,PIN_FLD_NAME,NULL, ebufp);
						PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flistp, &read_oflistp, ebufp);	
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "READ OBJ FOR PLAN ADDITION", read_oflistp);
						PIN_FLIST_FLD_SET(plans, PIN_FLD_NAME,PIN_FLIST_FLD_GET(read_oflistp, PIN_FLD_NAME, 1, ebufp),ebufp);
						}
					
					purchased_product = PIN_FLIST_ELEM_ADD(plans, PIN_FLD_PRODUCTS, pp_count, ebufp );

					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_POID,              purchased_product, PIN_FLD_POID, ebufp);
					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_PURCHASE_START_T,  purchased_product, PIN_FLD_PURCHASE_START_T, ebufp);
					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_PURCHASE_END_T,    purchased_product, PIN_FLD_PURCHASE_END_T, ebufp);
					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_USAGE_START_T,     purchased_product, PIN_FLD_USAGE_START_T, ebufp);
					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_CYCLE_START_T,     purchased_product, PIN_FLD_CYCLE_START_T, ebufp);
					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_EFFECTIVE_T,       purchased_product, PIN_FLD_EFFECTIVE_T, ebufp);
					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_STATUS,            purchased_product, PIN_FLD_STATUS, ebufp);
					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_STATUS_FLAGS,       purchased_product, PIN_FLD_STATUS_FLAGS, ebufp);
					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_DESCR,             purchased_product, PIN_FLD_DESCR, ebufp);
					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_PACKAGE_ID,        purchased_product, PIN_FLD_PACKAGE_ID, ebufp);
					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_DEAL_OBJ,          purchased_product, PIN_FLD_DEAL_OBJ, ebufp);
					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_PRODUCT_OBJ,       purchased_product, PIN_FLD_PRODUCT_OBJ, ebufp);
					
					purch_prod_status_flags = PIN_FLIST_FLD_GET(purch_prod_array, PIN_FLD_STATUS_FLAGS, 0, ebufp);
                                        if (purch_prod_status_flags && (*purch_prod_status_flags == 0xFFFF))
                                        {    
                                                cycle_end_t = PIN_FLIST_FLD_GET(purch_prod_array, PIN_FLD_CYCLE_END_T, 0, ebufp);
                                                purchase_end_t = PIN_FLIST_FLD_GET(purch_prod_array, PIN_FLD_PURCHASE_END_T, 0, ebufp);
                                                if (cycle_end_t && purchase_end_t && (*purchase_end_t > *cycle_end_t))
                                                {    
                                                        PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_PURCHASE_END_T, purchased_product, PIN_FLD_VALID_END_TO, ebufp);
                                                        PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_CYCLE_END_T, purchased_product, PIN_FLD_PURCHASE_END_T, ebufp);
                                                }            
                                        }
					// Actual grace period changes
				        else if (purch_prod_status_flags &&(*purch_prod_status_flags == 0xFFF))
                                        {
                                                cycle_end_t = PIN_FLIST_FLD_GET(purch_prod_array, PIN_FLD_CYCLE_END_T, 0, ebufp);
                                                purchase_end_t = PIN_FLIST_FLD_GET(purch_prod_array, PIN_FLD_PURCHASE_END_T, 0, ebufp);
                                                if (cycle_end_t && purchase_end_t && (*purchase_end_t > *cycle_end_t))
                                                {
                                                        PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_USAGE_END_T, purchased_product, PIN_FLD_VALID_END_TO, ebufp);
                                                        PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_CYCLE_END_T, purchased_product, PIN_FLD_PURCHASE_END_T, ebufp);
                                                }
                                        }
					// Actual grace period changes end
					PIN_FLIST_ELEM_COPY(purch_prod_array, PIN_FLD_CYCLE_FEES, 1, purchased_product, PIN_FLD_CYCLE_FEES, 1, ebufp);
					pp_count++;
				}
				if (purch_prod_status && *purch_prod_status == 3)
				{
					while((plans = PIN_FLIST_ELEM_GET_NEXT(plan_list_cancel, PIN_FLD_PLAN,
						&elem_id_2, 1, &cookie_2, ebufp)) != (pin_flist_t *)NULL)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "browsing plan");
						plan_obj_2 =  PIN_FLIST_FLD_GET(plans, PIN_FLD_PLAN_OBJ, 0, ebufp);
						tmp_flistp = PIN_FLIST_ELEM_GET(plans, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY,1, ebufp );
						pkg_id2 = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_PACKAGE_ID, 0, ebufp);
						if (( PIN_POID_COMPARE(plan_obj_1, plan_obj_2, 0, ebufp) == 0  )
							&& pkg_id1 && pkg_id2 && *pkg_id1 == *pkg_id2 )
						{
							PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Plan obj in purch_prod=", plan_obj_1);
							PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Plan obj in existing plan_list_cancel=", plan_obj_2);
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,  "plan_obj_1=plan_obj_2");
							index = elem_id_2;
							break;
						}
					}
					if (index == -1)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "index == -1");
						plans = PIN_FLIST_ELEM_ADD(plan_list_cancel, PIN_FLD_PLAN, plan_count, ebufp );
						plan_count++;
					}
					else
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "index != -1");
						plans = PIN_FLIST_ELEM_GET(plan_list_cancel, PIN_FLD_PLAN, elem_id_2, 0, ebufp );
					}

					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_PLAN_OBJ, plans, PIN_FLD_PLAN_OBJ, ebufp);

						//Changes to add Plan Name
						if(PIN_FLIST_FLD_GET(purch_prod_array, PIN_FLD_PLAN_OBJ, 1, ebufp) && PIN_FLIST_FLD_GET(purch_prod_array, PIN_FLD_PLAN_OBJ, 1, ebufp) != NULL)
						{
						read_flistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_SET(read_flistp,PIN_FLD_POID, PIN_FLIST_FLD_GET(purch_prod_array, PIN_FLD_PLAN_OBJ, 1, ebufp), ebufp);
						PIN_FLIST_FLD_SET(read_flistp,PIN_FLD_NAME,NULL, ebufp);
						PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flistp, &read_oflistp, ebufp);	
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "READ OBJ FOR PLAN ADDITION", read_oflistp);
						PIN_FLIST_FLD_SET(plans, PIN_FLD_NAME,PIN_FLIST_FLD_GET(read_oflistp, PIN_FLD_NAME, 1, ebufp),ebufp);
						}
					
					purchased_product = PIN_FLIST_ELEM_ADD(plans, PIN_FLD_PRODUCTS, pp_count, ebufp );

					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_POID,              purchased_product, PIN_FLD_POID, ebufp);
					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_PURCHASE_START_T,  purchased_product, PIN_FLD_PURCHASE_START_T, ebufp);
					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_PURCHASE_END_T,    purchased_product, PIN_FLD_PURCHASE_END_T, ebufp);
					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_USAGE_START_T,     purchased_product, PIN_FLD_USAGE_START_T, ebufp);
					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_CYCLE_START_T,     purchased_product, PIN_FLD_CYCLE_START_T, ebufp);
					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_EFFECTIVE_T,       purchased_product, PIN_FLD_EFFECTIVE_T, ebufp);
					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_STATUS,            purchased_product, PIN_FLD_STATUS, ebufp);
					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_STATUS_FLAGS,      purchased_product, PIN_FLD_STATUS_FLAGS, ebufp);
					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_DESCR,             purchased_product, PIN_FLD_DESCR, ebufp);
					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_PACKAGE_ID,        purchased_product, PIN_FLD_PACKAGE_ID, ebufp);
					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_DEAL_OBJ,          purchased_product, PIN_FLD_DEAL_OBJ, ebufp);
					PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_PRODUCT_OBJ,       purchased_product, PIN_FLD_PRODUCT_OBJ, ebufp);

					purch_prod_status_flags = PIN_FLIST_FLD_GET(purch_prod_array, PIN_FLD_STATUS_FLAGS, 0, ebufp);
                                        if (purch_prod_status_flags && (*purch_prod_status_flags == 0xFFFF))
                                        {    
                                                cycle_end_t = PIN_FLIST_FLD_GET(purch_prod_array, PIN_FLD_CYCLE_END_T, 0, ebufp);
                                                purchase_end_t = PIN_FLIST_FLD_GET(purch_prod_array, PIN_FLD_PURCHASE_END_T, 0, ebufp);
                                                if (cycle_end_t && purchase_end_t && (*purchase_end_t > *cycle_end_t))
                                                {    
                                                        PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_PURCHASE_END_T, purchased_product, PIN_FLD_VALID_END_TO, ebufp);
                                                        PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_CYCLE_END_T, purchased_product, PIN_FLD_PURCHASE_END_T, ebufp);
                                                }            
                                        }
					// Actual grace period  changes
				        else if (purch_prod_status_flags &&(*purch_prod_status_flags == 0xFFF))
                                        {
                                                cycle_end_t = PIN_FLIST_FLD_GET(purch_prod_array, PIN_FLD_CYCLE_END_T, 0, ebufp);
                                                purchase_end_t = PIN_FLIST_FLD_GET(purch_prod_array, PIN_FLD_PURCHASE_END_T, 0, ebufp);
                                                if (cycle_end_t && purchase_end_t && (*purchase_end_t > *cycle_end_t))
                                                {
                                                        PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_USAGE_END_T, purchased_product, PIN_FLD_VALID_END_TO, ebufp);
                                                        PIN_FLIST_FLD_COPY(purch_prod_array, PIN_FLD_CYCLE_END_T, purchased_product, PIN_FLD_PURCHASE_END_T, ebufp);
                                                }
                                        }
					// Actual grce period changes end
					PIN_FLIST_ELEM_COPY(purch_prod_array, PIN_FLD_CYCLE_FEES, 1, purchased_product, PIN_FLD_CYCLE_FEES, 1, ebufp);
					pp_count++;
				}

			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "plan_list_active", plan_list_active);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "plan_list_inactive", plan_list_inactive);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "plan_list_cancel", plan_list_cancel);
		}

		/********* Pavan Bellala 06-10-2015 *****************
		 Sort the plans obtained in desc order
		****************************************************/		
		if(PIN_FLIST_ELEM_COUNT(plan_list_active,PIN_FLD_PLAN,ebufp)>0)
		{
			sort_flistp = PIN_FLIST_CREATE(ebufp);
			s_plan_flistp = PIN_FLIST_ELEM_ADD(sort_flistp,PIN_FLD_PLAN,PIN_ELEMID_ANY,ebufp);
			s_product_flistp = PIN_FLIST_ELEM_ADD(s_plan_flistp,PIN_FLD_PRODUCTS,PIN_ELEMID_ANY,ebufp);
			PIN_FLIST_FLD_SET(s_product_flistp,PIN_FLD_PURCHASE_END_T,NULL,ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Sorting template flist", sort_flistp);

			PIN_FLIST_SORT_REVERSE(plan_list_active,sort_flistp,0,ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Sorted plan_list_active",plan_list_active);
			PIN_FLIST_DESTROY_EX(&sort_flistp,NULL);
		}
		if(PIN_FLIST_ELEM_COUNT(plan_list_inactive,PIN_FLD_PLAN,ebufp)>0)
		{
			sort_flistp = PIN_FLIST_CREATE(ebufp);
			s_plan_flistp = PIN_FLIST_ELEM_ADD(sort_flistp,PIN_FLD_PLAN,PIN_ELEMID_ANY,ebufp);
			s_product_flistp = PIN_FLIST_ELEM_ADD(s_plan_flistp,PIN_FLD_PRODUCTS,PIN_ELEMID_ANY,ebufp);
			PIN_FLIST_FLD_SET(s_product_flistp,PIN_FLD_PURCHASE_END_T,NULL,ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Sorting template flist", sort_flistp);

			PIN_FLIST_SORT_REVERSE(plan_list_inactive,sort_flistp,0,ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Sorted plan_list_inactive",plan_list_inactive);
			PIN_FLIST_DESTROY_EX(&sort_flistp,NULL);
		}
		if(PIN_FLIST_ELEM_COUNT(plan_list_cancel,PIN_FLD_PLAN,ebufp)>0)
		{
			sort_flistp = PIN_FLIST_CREATE(ebufp);
			s_plan_flistp = PIN_FLIST_ELEM_ADD(sort_flistp,PIN_FLD_PLAN,PIN_ELEMID_ANY,ebufp);
			s_product_flistp = PIN_FLIST_ELEM_ADD(s_plan_flistp,PIN_FLD_PRODUCTS,PIN_ELEMID_ANY,ebufp);
			PIN_FLIST_FLD_SET(s_product_flistp,PIN_FLD_PURCHASE_END_T,NULL,ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Sorting template flist", sort_flistp);

			PIN_FLIST_SORT_REVERSE(plan_list_cancel,sort_flistp,0,ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Sorted plan_list_cancel",plan_list_cancel);
			PIN_FLIST_DESTROY_EX(&sort_flistp,NULL);
		}

		PIN_FLIST_ELEM_SET(plan_list, plan_list_active, PIN_FLD_PLAN_LISTS, 1, ebufp );
		PIN_FLIST_ELEM_SET(plan_list, plan_list_inactive, PIN_FLD_PLAN_LISTS, 2, ebufp );
		PIN_FLIST_ELEM_SET(plan_list, plan_list_cancel, PIN_FLD_PLAN_LISTS, 3, ebufp );

		PIN_FLIST_CONCAT(services_array, plan_list, ebufp);
		//PIN_FLIST_DESTROY_EX(&plan_list_active, NULL);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "services_array", services_array);
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srvc_info", *srvc_info);

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&tmp_srch_oflistp, NULL);
	//Changes for plan name
	PIN_FLIST_DESTROY_EX(&read_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&plan_list, NULL);
	PIN_FLIST_DESTROY_EX(&plan_list_active, NULL);
	PIN_FLIST_DESTROY_EX(&plan_list_inactive, NULL);
	PIN_FLIST_DESTROY_EX(&plan_list_cancel, NULL);
	PIN_FLIST_DESTROY_EX(&sprod_flistp, NULL);

	if (!pbo_decimal_is_null(hundred, ebufp)){
       	  pbo_decimal_destroy(&hundred);
	}
	return;
}

///**************************************************
//* Function: 	fm_mso_get_device_details()
//* Owner:        Gautam Adak
//* Decription:   
//*               
//*		
//* 
//* 
//***************************************************/
//static void
//fm_mso_get_device_details(
//	pcm_context_t		*ctxp,
//	poid_t			*acnt_pdp,
//	pin_flist_t		**ret_flistp,
//	pin_errbuf_t		*ebufp)
//{
//	pin_flist_t		*srch_flistp = NULL;
//	pin_flist_t		*arg_flist = NULL;
//	pin_flist_t		*result_flist = NULL;
//	pin_flist_t		*srch_oflistp = NULL;
//	pin_flist_t		*services = NULL;
//
//	poid_t			*srch_pdp = NULL;
//	int32			srch_flag = 512;
//
//	int64			db = -1;
//
//	char			*template = "select X from /device where F1.id = V1 " ;
//
//
//	if (PIN_ERRBUF_IS_ERR(ebufp))
//	{
//		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_device_details", ebufp);
//		return;
//	}
//	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_device_details acnt_pdp", acnt_pdp);
//
//	db = PIN_POID_GET_DB(acnt_pdp) ;
//	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
//
//	srch_flistp = PIN_FLIST_CREATE(ebufp);
//	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
//	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
//	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
//
//	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
//
//	services = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_SERVICES, PIN_ELEMID_ANY, ebufp );
//	PIN_FLIST_FLD_SET(services, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);
//
//	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
//	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_DEVICE_ID, NULL, ebufp);
//	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_SERIAL_NO, NULL, ebufp);
//	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_MANUFACTURER, NULL, ebufp);
//	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_MODEL, NULL, ebufp);
//	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_WARRANTY_END, NULL, ebufp);
//	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STATE_ID, NULL, ebufp);
//	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp);
//
//	services = PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_SERVICES, PIN_ELEMID_ANY, ebufp);
//	PIN_FLIST_FLD_SET(services, PIN_FLD_SERVICE_OBJ, NULL, ebufp);
//
//
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_device_details read input list", srch_flistp);
//	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);
//	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
//
//	if (PIN_ERRBUF_IS_ERR(ebufp))
//	{
//		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ", ebufp);
//		return;
//	}
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_device_details READ output list", srch_oflistp);
// 	
//	if (PIN_FLIST_ELEM_COUNT(srch_oflistp, PIN_FLD_RESULTS, ebufp) == 0)
//	{
//		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
//	}
//	else
//	{
//		PIN_FLIST_FLD_DROP(srch_oflistp, PIN_FLD_POID, ebufp);
//		*ret_flistp = srch_oflistp;
//	}
//
//	CLEANUP:
//	return;
//}

/**************************************************
* Function: 	fm_mso_get_bill_details()
* Owner:        Gautam Adak
* Decription:   
*               
*		
* 
* 
***************************************************/
static void
fm_mso_get_bill_details(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*services = NULL;
	pin_flist_t		*read_bill_iflist = NULL;
	pin_flist_t		*read_bill_oflist = NULL;
	pin_flist_t		*inv_info = NULL;
	pin_flist_t		*read_payinfo_iflist = NULL;
	pin_flist_t		*read_payinfo_oflist = NULL;	
	pin_flist_t		*balance_flist = NULL;

	poid_t			*srch_pdp = NULL;
	poid_t			*payinfo_obj = NULL;
	poid_t			*billinfo_obj = NULL;

	int32			srch_flag = 512;
	int32                   status = 10100;
	int32			link_direction = 1;
	int32			pay_type = -1;
	int32			delivery_pref = -1;
	int32			elem_id = 0;
	pin_cookie_t		cookie = NULL;

	int64			db = -1;
	int64			last_bill_obj_id0 = 0;
	
	//pin_decimal_t 		*unbilled_amount = pbo_decimal_from_str("0.0",ebufp);
	//pin_decimal_t 		*current_total = pbo_decimal_from_str("0.0",ebufp);
	//pin_decimal_t 		*Due = pbo_decimal_from_str("0.0",ebufp);
	//pin_decimal_t 		*recvd = pbo_decimal_from_str("0.0",ebufp);
	//pin_decimal_t 		*previous_total = pbo_decimal_from_str("0.0",ebufp);
	pin_decimal_t 		*unbilled_amount = NULL;
	pin_decimal_t 		*current_total = NULL;
	pin_decimal_t 		*Due = NULL;
	pin_decimal_t 		*recvd = NULL;
	pin_decimal_t 		*previous_total = NULL;
	pin_decimal_t		*total_due	= NULL;

	char			*template = "select X from /billinfo  where F1.id = V1 and F2 = V2 " ;
	pin_flist_t		*ar_flistp = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_bill_details", ebufp);
		return;
	}
    PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bill_details acnt_pdp", acnt_pdp);

	db = PIN_POID_GET_DB(acnt_pdp) ;
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);
        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STATUS, &status, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_BILLINFO_ID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_BILL_WHEN, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACTG_CYCLE_DOM, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACTG_FUTURE_T, NULL, ebufp);
//	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACTG_LAST_T, NULL, ebufp);
//	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACTG_NEXT_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_LAST_BILL_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_NEXT_BILL_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_PAY_TYPE, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACTUAL_LAST_BILL_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_PAYINFO_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_BAL_GRP_OBJ, NULL, ebufp);

	/*** Pavan Bellala 19-08-2015 ***************
	Added further fields 
	********************************************/
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACTUAL_LAST_BILL_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist,	PIN_FLD_AR_BILLINFO_OBJ, NULL, ebufp);



	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bill_details search billinfo input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_bill_details search error", ebufp);
        PIN_ERRBUF_RESET(ebufp);
        PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bill_details search billinfo output list", srch_oflistp);

	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	
	/************************************************************
	* Populate LAST Bill info
	************************************************************/
	while((result_flist = PIN_FLIST_ELEM_GET_NEXT(srch_oflistp, PIN_FLD_RESULTS,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		/************************************************************
		* Populate bill in progress and credit limit
		* Start
		************************************************************/
		billinfo_obj = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_POID, 0, ebufp );
		fm_mso_get_balance(ctxp, acnt_pdp, billinfo_obj, &balance_flist, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
        	{
                	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error after returning from fm_mso_get_balance", ebufp);
                	goto CLEANUP;
        	}
		PIN_FLIST_FLD_DROP(balance_flist,PIN_FLD_OPENBILL_DUE,ebufp);
		//unbilled_amount = PIN_FLIST_FLD_GET(balance_flist,PIN_FLD_OPENBILL_DUE, 1 ,ebufp);
		unbilled_amount = PIN_FLIST_FLD_GET(balance_flist,PIN_FLD_PENDINGBILL_DUE, 1 ,ebufp);
		
		/*** Pavan Bellala 20-08-2015 ******
		Added rounding mode
		bug id : 1320	
		************************************/
		PIN_FLIST_FLD_PUT(balance_flist, PIN_FLD_OPENBILL_DUE, 
						 pbo_decimal_round(unbilled_amount,2,ROUND_HALF_UP,ebufp), ebufp);

		if (balance_flist)
		{
			PIN_FLIST_CONCAT(result_flist, balance_flist, ebufp );
		//	PIN_FLIST_DESTROY_EX(&balance_flist, NULL);
		}
		/************************************************************
		* Populate bill in progress and credit limit
		* Start
		************************************************************/
		/************************************************************
		* Populate Invoice Delivery Preference
		* Start
		************************************************************/
		pay_type = *(int32*)PIN_FLIST_FLD_GET(result_flist, PIN_FLD_PAY_TYPE, 0, ebufp);
		if (pay_type == PIN_PAY_TYPE_INVOICE )
		{
			payinfo_obj = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_PAYINFO_OBJ, 0, ebufp);
			read_payinfo_iflist = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(read_payinfo_iflist, PIN_FLD_POID, payinfo_obj, ebufp );
			inv_info = PIN_FLIST_ELEM_ADD(read_payinfo_iflist, PIN_FLD_INV_INFO, 0, ebufp); 
			PIN_FLIST_FLD_SET(inv_info, PIN_FLD_DELIVERY_PREFER, NULL, ebufp);
			PIN_FLIST_FLD_SET(inv_info, PIN_FLD_INDICATOR, NULL, ebufp);
			PIN_FLIST_FLD_SET(inv_info, MSO_FLD_TDS_RECV, NULL, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read payinfo input flist", read_payinfo_iflist);
			PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_payinfo_iflist, &read_payinfo_oflist, ebufp);
			PIN_FLIST_DESTROY_EX(&read_payinfo_iflist, NULL); 
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "payinfo: rflds error", ebufp);
				return;
			}
			inv_info = PIN_FLIST_ELEM_GET(read_payinfo_oflist, PIN_FLD_INV_INFO, 0, 0, ebufp); 
			delivery_pref = *(int32*)PIN_FLIST_FLD_GET(inv_info, PIN_FLD_DELIVERY_PREFER, 0, ebufp); 
			PIN_FLIST_FLD_SET(result_flist, PIN_FLD_DELIVERY_PREFER, &delivery_pref, ebufp );
			PIN_FLIST_FLD_COPY(inv_info, PIN_FLD_INDICATOR, result_flist, PIN_FLD_INDICATOR, ebufp  );
			PIN_FLIST_FLD_COPY(inv_info, MSO_FLD_TDS_RECV, result_flist, MSO_FLD_TDS_RECV, ebufp  );
			PIN_FLIST_DESTROY_EX(&read_payinfo_oflist, NULL);
		}
		else
		{
			read_payinfo_iflist = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(read_payinfo_iflist, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(read_payinfo_iflist, PIN_FLD_FLAGS, &srch_flag, ebufp);
			PIN_FLIST_FLD_SET(read_payinfo_iflist, PIN_FLD_TEMPLATE, "select X from /payinfo/invoice where F1 = V1 and F2.type = V2 " , ebufp);

			arg_flist = PIN_FLIST_ELEM_ADD(read_payinfo_iflist, PIN_FLD_ARGS, 1, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);
			arg_flist = PIN_FLIST_ELEM_ADD(read_payinfo_iflist, PIN_FLD_ARGS, 2, ebufp );
			PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/payinfo/invoice", -1, ebufp), ebufp);

			arg_flist = PIN_FLIST_ELEM_ADD(read_payinfo_iflist, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
			inv_info = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_INV_INFO, 0, ebufp); 
			PIN_FLIST_FLD_SET(inv_info, PIN_FLD_DELIVERY_PREFER, NULL, ebufp);
			PIN_FLIST_FLD_SET(inv_info, PIN_FLD_INDICATOR, NULL, ebufp);
			PIN_FLIST_FLD_SET(inv_info, MSO_FLD_TDS_RECV, NULL, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read payinfo input flist", read_payinfo_iflist);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, read_payinfo_iflist, &read_payinfo_oflist, ebufp);
			PIN_FLIST_DESTROY_EX(&read_payinfo_iflist, NULL); 
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "payinfo: rflds error", ebufp);
				return;
			}
			arg_flist = PIN_FLIST_ELEM_GET(read_payinfo_oflist, PIN_FLD_RESULTS, 0, 1, ebufp); 
			if (arg_flist)
			{
				inv_info = PIN_FLIST_ELEM_GET(arg_flist, PIN_FLD_INV_INFO, 0, 0, ebufp); 
				delivery_pref = *(int32*)PIN_FLIST_FLD_GET(inv_info, PIN_FLD_DELIVERY_PREFER, 0, ebufp); 
				PIN_FLIST_FLD_SET(result_flist, PIN_FLD_DELIVERY_PREFER, &delivery_pref, ebufp );
				PIN_FLIST_FLD_COPY(inv_info, PIN_FLD_INDICATOR, result_flist, PIN_FLD_INDICATOR, ebufp  );
				PIN_FLIST_FLD_COPY(inv_info, MSO_FLD_TDS_RECV, result_flist, MSO_FLD_TDS_RECV, ebufp  );
			}
			PIN_FLIST_DESTROY_EX(&read_payinfo_oflist, NULL);
		}
		/************************************************************
		* Populate Invoice Delivery Preference
		* End
		************************************************************/

		last_bill_obj_id0 = 0;
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "result_flist", result_flist);
		last_bill_obj_id0 = PIN_POID_GET_ID( PIN_FLIST_FLD_GET(result_flist, PIN_FLD_ACTUAL_LAST_BILL_OBJ, 0, ebufp));
		

		if (last_bill_obj_id0 != 0)
		{
//			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST_0001");
			read_bill_iflist = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(read_bill_iflist, PIN_FLD_POID,PIN_POID_CREATE(db, "/bill", last_bill_obj_id0, ebufp), ebufp );
			PIN_FLIST_FLD_SET(read_bill_iflist, PIN_FLD_CREATED_T, NULL, ebufp );
			PIN_FLIST_FLD_SET(read_bill_iflist, PIN_FLD_BILL_NO, NULL, ebufp );
			PIN_FLIST_FLD_SET(read_bill_iflist, PIN_FLD_CURRENT_TOTAL, NULL, ebufp );
			PIN_FLIST_FLD_SET(read_bill_iflist, PIN_FLD_DUE, NULL, ebufp );
			PIN_FLIST_FLD_SET(read_bill_iflist, PIN_FLD_SUBORDS_TOTAL, NULL, ebufp );
			PIN_FLIST_FLD_SET(read_bill_iflist, PIN_FLD_TOTAL_DUE, NULL, ebufp );
			PIN_FLIST_FLD_SET(read_bill_iflist, PIN_FLD_ADJUSTED, NULL, ebufp );
			PIN_FLIST_FLD_SET(read_bill_iflist, PIN_FLD_DISPUTED, NULL, ebufp );
			/**** Changes done to include DUE_T , RECVD , PREVIOUS_TOTAL fields ****/
			PIN_FLIST_FLD_SET(read_bill_iflist,PIN_FLD_DUE_T, NULL, ebufp);
			PIN_FLIST_FLD_SET(read_bill_iflist,PIN_FLD_PREVIOUS_TOTAL, NULL, ebufp);
			PIN_FLIST_FLD_SET(read_bill_iflist,PIN_FLD_RECVD, NULL, ebufp);
			PIN_FLIST_FLD_SET(read_bill_iflist, PIN_FLD_END_T, NULL, ebufp);			

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bill_details read input list", read_bill_iflist);
			PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_bill_iflist, &read_bill_oflist, ebufp);
			PIN_FLIST_DESTROY_EX(&read_bill_iflist, NULL);
			
			current_total = PIN_FLIST_FLD_GET(read_bill_oflist, PIN_FLD_CURRENT_TOTAL, 1, ebufp);

			previous_total = PIN_FLIST_FLD_GET(read_bill_oflist, PIN_FLD_PREVIOUS_TOTAL, 1, ebufp);

			
			Due = PIN_FLIST_FLD_GET(read_bill_oflist,PIN_FLD_DUE, 1, ebufp);
			recvd = PIN_FLIST_FLD_GET(read_bill_oflist,PIN_FLD_RECVD, 1, ebufp);
			
			Due = pbo_decimal_add(current_total, previous_total, ebufp);	
			//Due = pbo_decimal_add(Due, recvd, ebufp);	
			pbo_decimal_add_assign(Due,recvd, ebufp);
			//Commented below line and added next line to send current_total in correct field.
			//PIN_FLIST_FLD_SET(read_bill_oflist, PIN_FLD_CURRENT_TOTAL,
			//					pbo_decimal_round(Due,2,ROUND_HALF_UP,ebufp),ebufp );
			PIN_FLIST_FLD_PUT(read_bill_oflist, PIN_FLD_CURRENT_TOTAL,
								pbo_decimal_round(current_total,2,ROUND_HALF_UP,ebufp),ebufp );
			PIN_FLIST_FLD_SET(read_bill_oflist, PIN_FLD_DUE,current_total ,ebufp );
		
			total_due = pbo_decimal_add(unbilled_amount, Due, ebufp);
			
			PIN_FLIST_FLD_SET(result_flist, PIN_FLD_TOTAL_DUE, total_due, ebufp);

			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ", ebufp);
				goto CLEANUP;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bill_details read output list", read_bill_oflist);
			
			PIN_FLIST_ELEM_PUT(result_flist, read_bill_oflist, PIN_FLD_BILLS, 0, ebufp);
			if(!pbo_decimal_is_null(Due, ebufp))
			{
				pbo_decimal_destroy(&Due);
			}
		} 
		/************ Pavan Bellala 04-08-2015 ********************************
		 Bug ID : 1315 
		 Added else condition to populated last_bill_t = 0 if last_bill_obj = 0
		 means account not yet billed.
		************ Pavan Bellala 04-08-2015 ********************************/
		//else {
		//	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_LAST_BILL_T, NULL, ebufp);
		//}
		PIN_FLIST_ELEM_COPY(srch_oflistp, PIN_FLD_RESULTS, elem_id, *ret_flistp, PIN_FLD_BILLINFO, elem_id, ebufp);
        /******************* RAMA * ROHAN decided to comment this logic
		fm_mso_utils_fetch_ar_details(ctxp,*ret_flistp,acnt_pdp,&ar_flistp,ebufp);
		if((ar_flistp != NULL) && (PIN_FLIST_COUNT(ar_flistp,ebufp)>0)){
			arg_flist = PIN_FLIST_ELEM_GET(*ret_flistp,PIN_FLD_BILLINFO,PIN_ELEMID_ANY,1,ebufp);
			PIN_FLIST_CONCAT(arg_flist,ar_flistp,ebufp);	
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bill_details modified ret_flistp", *ret_flistp);
		}*/
			
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bill_details ret_flistp", *ret_flistp);

	CLEANUP:
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_FLIST_DESTROY_EX(ret_flistp, NULL);
	}
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&balance_flist, NULL);
	
	if(!pbo_decimal_is_null(total_due, ebufp))
	{
		pbo_decimal_destroy(&total_due);
	}
	if(!pbo_decimal_is_null(Due, ebufp))
			{
		pbo_decimal_destroy(&Due);
	}
	
	/*if(pbo_decimal_is_zero(Due,ebufp))
    	{
		pbo_decimal_destroy(&Due);
    	}
	
	if(!pbo_decimal_is_zero(recvd,ebufp))
        {
                pbo_decimal_destroy(&recvd);
        }
	if(!pbo_decimal_is_null(previous_total,ebufp))
        {
                pbo_decimal_destroy(&previous_total);
        }
	if(!pbo_decimal_is_null(current_total,ebufp))
        {
                pbo_decimal_destroy(&current_total);
        }*/
	
//	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST_0001");
	return;
}

/**************************************************
* Function: 	fm_mso_get_acnt_from_vc()
* Owner:        Gautam Adak
* Decription:   
*               
*		
* 
* 
***************************************************/
static void
fm_mso_get_acnt_from_vc(
	pcm_context_t		*ctxp,
	char*			card_no,
	poid_t			**acnt_pdp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;

	poid_t			*srch_pdp = NULL;
	int32			srch_flag = 512;

	int64			db = 1;

	char			*template = "select X from /device  where F1 = V1 or F2 = V2 " ;


	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_acnt_from_vc", ebufp);
		return;
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_acnt_from_vc card_id:");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, card_no);

	//db = PIN_POID_GET_DB(acnt_pdp) ;
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_DEVICE_ID, card_no, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, MSO_FLD_SERIAL_NO, card_no, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	result_flist = PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_SERVICES, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_acnt_from_vc search billinfo input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_acnt_from_vc search billinfo output list", srch_oflistp);

	result_flist = PIN_FLIST_ELEM_TAKE(srch_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp );
	if (result_flist)
	{
		arg_flist = PIN_FLIST_ELEM_GET(result_flist, PIN_FLD_SERVICES, 0, 1, ebufp);
		if (arg_flist)
		{
			*acnt_pdp = PIN_FLIST_FLD_TAKE(arg_flist, PIN_FLD_ACCOUNT_OBJ, 0, ebufp );
		}
	}
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);

	CLEANUP:
	return;
}

/**************************************************
* Function: 	fm_mso_get_acnt_from_rmn()
* Owner:        Gautam Adak
* Decription:   
*               
*		
* 
* 
***************************************************/
static void
fm_mso_get_acnt_from_rmn(
	pcm_context_t		*ctxp,
	char*			rmn,
	poid_t			**acnt_pdp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;

	poid_t			*srch_pdp = NULL;
	int32			srch_flag = 512;

	int64			db = 1;

	char			*template = "select X from /account  where F1 = V1 " ;


	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_acnt_from_rmn", ebufp);
		return;
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_acnt_from_rmn rmn:");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, rmn);

	//db = PIN_POID_GET_DB(acnt_pdp) ;
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, MSO_FLD_RMN, rmn, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_acnt_from_rmn search billinfo input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_acnt_from_rmn search billinfo output list", srch_oflistp);

	result_flist = PIN_FLIST_ELEM_TAKE(srch_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp );
	if (result_flist)
	{
		*acnt_pdp = PIN_FLIST_FLD_TAKE(result_flist, PIN_FLD_ACCOUNT_OBJ, 0, ebufp );
	}
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);

	CLEANUP:
	return;
}



/**************************************************
* Function: 	fm_mso_check_csr_data_protection()
* Owner:        Gautam Adak
* Decription:   
*               
*		
* 
* 
***************************************************/
static void
fm_mso_check_csr_data_protection(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flist,
	pin_flist_t		**access_info,
	poid_t			**profile_obj,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*cc_info = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*profie_parent_acnt = NULL;
	pin_flist_t		*profie_csr = NULL; 
	pin_flist_t		*wholesale_info = NULL;
	pin_flist_t		*profile_info = NULL;
	pin_flist_t		*jv_nameinfo = NULL;
	pin_flist_t		*jv_details = NULL;
	pin_flist_t		*prof_nameinfo = NULL;

	poid_t			*acnt_pdp = NULL;
	poid_t			*csr_acnt_pdp = NULL;
	poid_t			*srch_pdp = NULL;
	poid_t			*parent_pdp = NULL;
	poid_t			*jv_obj = NULL;
	poid_t			*dt_obj = NULL;
	poid_t			*sdt_obj = NULL;
	poid_t			*lco_obj = NULL;
	poid_t			*parent_of_csr = NULL;
	poid_t			*profile_pdp = NULL;
	
	int32			srch_flag = 768;
	int32			parent_type_of_csr = -1;
	int32			parent_type_of_acnt = -1;
	int32			access_customer = 0;
	int32			searched_acnt_type = -1;
	int32			*business_type = NULL;
	int32			tmp_business_type = -1;
	int32			account_type = -1;

	int64			db = 1;

	char			*template_srch_acnt = "select X from /profile where F1.id = V1 and (F2.type = V2 or F3.type = V3) " ;
	char			*template_srch_csr = "select X from /profile/wholesale  where 1.F1.id = V1 and F2.type = V2 " ;
	char			*profile_type = NULL;

	/*
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS           INT [0] 512
	0 PIN_FLD_TEMPLATE        STR [0] "select X from /profile/wholesale 1,  /profile/customer_care where ( 2.F2.id = V2 and 2.F1.id = 1.F3.id ) "
	0 PIN_FLD_ARGS          ARRAY [1] allocated 20, used 1
	1     PIN_FLD_CUSTOMER_CARE_INFO SUBSTRUCT [0] allocated 20, used 1
	2         PIN_FLD_PARENT        POID [0] NULL poid pointer
	0 PIN_FLD_ARGS          ARRAY [2] allocated 20, used 1
	1     PIN_FLD_ACCOUNT_OBJ    POID [0] 0.0.0.1 /account 63542 0
	0 PIN_FLD_ARGS          ARRAY [3] allocated 20, used 1
	1     PIN_FLD_ACCOUNT_OBJ    POID [0] NULL poid pointer
	0 PIN_FLD_RESULTS       ARRAY [*] allocated 20, used 0
	*/

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_check_csr_data_protection", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_check_csr_data_protection", i_flistp);

	csr_acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 0, ebufp);
	acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "fm_mso_check_csr_data_protection in flist", i_flistp);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Missing mandatory fields in input FLIST", ebufp);
		return;
	}
	db = PIN_POID_GET_DB(acnt_pdp) ;
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

	/**************************************************************************
	*  Search wholesale profile csr
	**************************************************************************/
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template_srch_csr , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, csr_acnt_pdp, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/profile/wholesale", -1, ebufp), ebufp);

	PIN_FLIST_ELEM_SET(srch_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_check_csr_data_protection search CSR profile input", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "fm_mso_check_csr_data_protection in flist", i_flistp);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "m_mso_check_csr_data_protection search error", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_check_csr_data_protection search CSR profile output", srch_oflistp);

	result_flist = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, 0, 1, ebufp );
	if (result_flist)
	{
		wholesale_info = PIN_FLIST_SUBSTR_GET(result_flist, MSO_FLD_WHOLESALE_INFO, 0, ebufp);
		parent_of_csr = PIN_POID_COPY( PIN_FLIST_FLD_GET(wholesale_info, PIN_FLD_PARENT, 0, ebufp), ebufp);
		jv_obj = PIN_FLIST_FLD_GET(wholesale_info, MSO_FLD_JV_OBJ, 0, ebufp );
		dt_obj = PIN_FLIST_FLD_GET(wholesale_info, MSO_FLD_DT_OBJ, 0, ebufp );
		sdt_obj = PIN_FLIST_FLD_GET(wholesale_info, MSO_FLD_SDT_OBJ, 0, ebufp );
		lco_obj = PIN_FLIST_FLD_GET(wholesale_info, MSO_FLD_LCO_OBJ, 0, ebufp );


		if ( parent_of_csr && 
		     PIN_POID_GET_ID(parent_of_csr) == 0
		   )
		{
			parent_type_of_csr = ACCT_TYPE_CSR_ADMIN;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ACCT_TYPE_CSR_ADMIN");
		}
		else if ( parent_of_csr && 
		     PIN_POID_COMPARE(parent_of_csr, jv_obj, 0, ebufp ) == 0)
		{
			//csr_under_jv = 1;
			parent_type_of_csr = ACCT_TYPE_JV;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ACCT_TYPE_JV");
		}
		else if ( parent_of_csr &&
			  PIN_POID_COMPARE(parent_of_csr, dt_obj, 0, ebufp ) == 0)
		{
			//csr_under_dt = 1;
			parent_type_of_csr = ACCT_TYPE_DTR;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ACCT_TYPE_DTR");
		}
		else if ( parent_of_csr &&
			  PIN_POID_COMPARE(parent_of_csr, sdt_obj, 0, ebufp ) == 0)
		{
			//csr_under_sdt = 1;
			parent_type_of_csr = ACCT_TYPE_SUB_DTR;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ACCT_TYPE_SUB_DTR");
		}
		else if ( parent_of_csr &&
			  PIN_POID_COMPARE(parent_of_csr, lco_obj, 0, ebufp ) == 0)
		{
			//csr_under_lco = 1;
			parent_type_of_csr = ACCT_TYPE_LCO;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ACCT_TYPE_LCO");
		}
	}
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);

	/**************************************************************************
	*  Search profile of to be searched account
	**************************************************************************/
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template_srch_acnt , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp );

        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
        PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/profile/wholesale", -1, ebufp), ebufp);

        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
        PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/profile/customer_care", -1, ebufp), ebufp);

	PIN_FLIST_ELEM_SET(srch_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp );
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_check_csr_data_protection search parent profile input", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_check_csr_data_protection search parent error", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_check_csr_data_protection search parent profile output", srch_oflistp);

	result_flist = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, 0, 1, ebufp );
	if (result_flist)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_check_csr_data_protection result_flist", result_flist);
		profile_pdp = (poid_t *)PIN_FLIST_FLD_GET(result_flist, PIN_FLD_POID, 0, ebufp);
		PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "profile_pdp", profile_pdp);
		profile_type = (char *)PIN_POID_GET_TYPE(profile_pdp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "profile_type:");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, profile_type);
		
		//if ( profile_type && (strcmp(profile_type, WHOLESALE ) == 0))
		if ( strstr(profile_type, "wholesale" ) )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "test_001");
			profile_info = PIN_FLIST_SUBSTR_GET(result_flist, MSO_FLD_WHOLESALE_INFO, 0, ebufp);
			parent_pdp = PIN_FLIST_FLD_GET(profile_info, PIN_FLD_PARENT, 0, ebufp );
			jv_obj = PIN_FLIST_FLD_GET(profile_info, MSO_FLD_JV_OBJ, 0, ebufp );
			dt_obj = PIN_FLIST_FLD_GET(profile_info, MSO_FLD_DT_OBJ, 0, ebufp );
			sdt_obj = PIN_FLIST_FLD_GET(profile_info, MSO_FLD_SDT_OBJ, 0, ebufp );
			lco_obj = PIN_FLIST_FLD_GET(profile_info, MSO_FLD_LCO_OBJ, 0, ebufp );
			searched_acnt_type = *(int32*)PIN_FLIST_FLD_GET(profile_info, PIN_FLD_CUSTOMER_TYPE, 0, ebufp );

			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "test_001");
			//Pawan: Commented from here to avoid access based on Parent of CSR
			/*
			switch (parent_type_of_csr)
			{
			  case ACCT_TYPE_CSR_ADMIN :
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ACCT_TYPE_CSR_ADMIN");
				access_customer = 1;
				break;
			  case ACCT_TYPE_JV :
				if (PIN_POID_COMPARE(parent_of_csr, jv_obj, 0, ebufp) == 0)
				{
					access_customer = 1;
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ACCT_TYPE_JV");
				}
				break;
			  case ACCT_TYPE_DTR :
				if (PIN_POID_COMPARE(parent_of_csr, dt_obj, 0, ebufp) == 0)
				{
					access_customer = 1;
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ACCT_TYPE_DTR");
				}
				break;
			  case ACCT_TYPE_SUB_DTR :
				if (PIN_POID_COMPARE(parent_of_csr, sdt_obj, 0, ebufp) == 0)
				{
					access_customer = 1;
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ACCT_TYPE_SUB_DTR");
				}
				break;
			  case ACCT_TYPE_LCO :
				if (PIN_POID_COMPARE(parent_of_csr, lco_obj, 0, ebufp) == 0)
				{
					access_customer = 1;
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ACCT_TYPE_LCO");
				}
				break;
			  default :
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Not matching");
			}
			*/
			//Commented till here

			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "test_001");

			//Compare the parent acnt poid of searched account to get its correct parent_type
			if (access_customer && (searched_acnt_type == ACCT_TYPE_CSR_ADMIN || searched_acnt_type == ACCT_TYPE_SALES_PERSON) 
					&& PIN_POID_GET_ID(jv_obj) ==0 )
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "test_000");
				//*ret_flist = PIN_FLIST_CREATE(ebufp);
				jv_obj = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_ACCOUNT_OBJ, 0, ebufp );
				PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "jv_obj", jv_obj);
			}
			if (access_customer && (searched_acnt_type == ACCT_TYPE_JV)  )
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "test_000");
				*ret_flist = PIN_FLIST_CREATE(ebufp);
			}

			if (access_customer && (searched_acnt_type == ACCT_TYPE_HTW )  )
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "test_000");
				*ret_flist = PIN_FLIST_CREATE(ebufp);
			}
			else if (access_customer && PIN_POID_GET_ID(jv_obj) !=0 )
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "test_001");
			/*	if (PIN_POID_COMPARE(parent_pdp, jv_obj, 0, ebufp ) ==0 )
				{
					PIN_FLIST_FLD_DROP(profile_info, MSO_FLD_DT_OBJ, ebufp);
					PIN_FLIST_FLD_DROP(profile_info, MSO_FLD_SDT_OBJ, ebufp);
					PIN_FLIST_FLD_DROP(profile_info, PIN_FLD_CUSTOMER_TYPE, ebufp);
					PIN_FLIST_FLD_DROP(profile_info, PIN_FLD_PARENT, ebufp);
				}
				else if (PIN_POID_COMPARE(parent_pdp, dt_obj, 0, ebufp ) ==0)
				{
					PIN_FLIST_FLD_DROP(profile_info, MSO_FLD_JV_OBJ, ebufp);
					PIN_FLIST_FLD_DROP(profile_info, MSO_FLD_SDT_OBJ, ebufp);
					PIN_FLIST_FLD_DROP(profile_info, PIN_FLD_CUSTOMER_TYPE, ebufp);
					PIN_FLIST_FLD_DROP(profile_info, PIN_FLD_PARENT, ebufp);
				}
				else if (PIN_POID_COMPARE(parent_pdp, sdt_obj, 0, ebufp ) ==0 )
				{
					PIN_FLIST_FLD_DROP(profile_info, MSO_FLD_DT_OBJ, ebufp);
					PIN_FLIST_FLD_DROP(profile_info, MSO_FLD_JV_OBJ, ebufp);
					PIN_FLIST_FLD_DROP(profile_info, PIN_FLD_CUSTOMER_TYPE, ebufp);
					PIN_FLIST_FLD_DROP(profile_info, PIN_FLD_PARENT, ebufp);
				}
			 */
				fm_mso_get_account_info(ctxp, jv_obj, &jv_details, ebufp );

				business_type = PIN_FLIST_FLD_GET(jv_details, PIN_FLD_BUSINESS_TYPE, 1, ebufp);
				tmp_business_type = *business_type;
				account_type = tmp_business_type/1000000;

				PIN_FLIST_FLD_COPY(jv_details, PIN_FLD_ACCOUNT_NO, profile_info, PIN_FLD_ACCOUNT_NO, ebufp );
				jv_nameinfo = PIN_FLIST_ELEM_GET(jv_details, PIN_FLD_NAMEINFO, 1, 1, ebufp);

				prof_nameinfo = PIN_FLIST_ELEM_ADD(profile_info, PIN_FLD_NAMEINFO, 1, ebufp);
				PIN_FLIST_FLD_COPY(jv_nameinfo, PIN_FLD_FIRST_NAME, prof_nameinfo, PIN_FLD_FIRST_NAME, ebufp );
				PIN_FLIST_FLD_COPY(jv_nameinfo, PIN_FLD_MIDDLE_NAME, prof_nameinfo, PIN_FLD_MIDDLE_NAME, ebufp );
				PIN_FLIST_FLD_COPY(jv_nameinfo, PIN_FLD_LAST_NAME, prof_nameinfo, PIN_FLD_LAST_NAME, ebufp );
				PIN_FLIST_FLD_COPY(jv_nameinfo, PIN_FLD_COMPANY, prof_nameinfo, PIN_FLD_COMPANY, ebufp );
				PIN_FLIST_FLD_SET(prof_nameinfo, PIN_FLD_ACCOUNT_TYPE, &account_type, ebufp);
				
				PIN_FLIST_DESTROY_EX(&jv_details, NULL);
				*ret_flist = PIN_FLIST_COPY(profile_info, ebufp);
				//Pawan:30-12-2014: Added below to add MSO_FLD_ACCESS_INFO in output.
				PIN_FLIST_ELEM_COPY(result_flist,MSO_FLD_ACCESS_INFO,0,*ret_flist,MSO_FLD_ACCESS_INFO,0,ebufp );
				PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_POID, *ret_flist, PIN_FLD_PROFILE_OBJ, ebufp );
			}
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "test_001");
			*access_info = PIN_FLIST_ELEM_TAKE(result_flist, MSO_FLD_ACCESS_INFO, 0, 1, ebufp);
  		}
		//if( ( strcmp( profile_type, CUSTOMER_CARE )) == 0 )
		//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, strstr(profile_type, "customer_care" ));

		if ( strstr(profile_type, "customer_care" ) )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "customer_care");
			profile_info = PIN_FLIST_SUBSTR_GET(result_flist, PIN_FLD_CUSTOMER_CARE_INFO, 0, ebufp);
			parent_pdp = PIN_FLIST_FLD_GET(profile_info, PIN_FLD_PARENT, 0, ebufp );
			jv_obj = PIN_FLIST_FLD_GET(profile_info, MSO_FLD_JV_OBJ, 0, ebufp );
			dt_obj = PIN_FLIST_FLD_GET(profile_info, MSO_FLD_DT_OBJ, 0, ebufp );
			sdt_obj = PIN_FLIST_FLD_GET(profile_info, MSO_FLD_SDT_OBJ, 0, ebufp );
			lco_obj = PIN_FLIST_FLD_GET(profile_info, MSO_FLD_LCO_OBJ, 0, ebufp );
			//Pawan: Commented from here to avoid access based on Parent of CSR
			/*
			switch (parent_type_of_csr)
			{
			  case ACCT_TYPE_CSR_ADMIN :
				access_customer = 1;
				break;
			  case ACCT_TYPE_JV :
				if (PIN_POID_COMPARE(parent_of_csr, jv_obj, 0, ebufp) == 0)
				{
					access_customer = 1;
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ACCT_TYPE_JV");
				}
				break;
			  case ACCT_TYPE_DTR :
				if (PIN_POID_COMPARE(parent_of_csr, dt_obj, 0, ebufp) == 0)
				{
					access_customer = 1;
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ACCT_TYPE_DTR");
				}
				break;
			  case ACCT_TYPE_SUB_DTR :
				if (PIN_POID_COMPARE(parent_of_csr, sdt_obj, 0, ebufp) == 0)
				{
					access_customer = 1;
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ACCT_TYPE_SUB_DTR");
				}
				break;
			  case ACCT_TYPE_LCO :
				if (PIN_POID_COMPARE(parent_of_csr, lco_obj, 0, ebufp) == 0)
				{
					access_customer = 1;
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ACCT_TYPE_LCO");
				}
				break;
			}
			*/
			//Commented till here

			if (access_customer )
			{
			/*	PIN_FLIST_FLD_DROP(profile_info, MSO_FLD_DT_OBJ, ebufp);
				PIN_FLIST_FLD_DROP(profile_info, MSO_FLD_JV_OBJ, ebufp);
				PIN_FLIST_FLD_DROP(profile_info, MSO_FLD_SDT_OBJ, ebufp);
				PIN_FLIST_FLD_DROP(profile_info, PIN_FLD_CUSTOMER_TYPE, ebufp);
				PIN_FLIST_FLD_DROP(profile_info, PIN_FLD_PARENT, ebufp);
			*/	
				if ( jv_obj && !(PIN_POID_IS_NULL(jv_obj)))
					fm_mso_get_account_info(ctxp, jv_obj, &jv_details, ebufp );
				else if ( dt_obj && !(PIN_POID_IS_NULL(dt_obj)))
					fm_mso_get_account_info(ctxp, dt_obj, &jv_details, ebufp );
				else if ( sdt_obj && !(PIN_POID_IS_NULL(sdt_obj)))
					fm_mso_get_account_info(ctxp, sdt_obj, &jv_details, ebufp );
				else if ( lco_obj && !(PIN_POID_IS_NULL(lco_obj)))
					fm_mso_get_account_info(ctxp, lco_obj, &jv_details, ebufp );

				business_type = PIN_FLIST_FLD_GET(jv_details, PIN_FLD_BUSINESS_TYPE, 1, ebufp);
				tmp_business_type = *business_type;
				account_type = tmp_business_type/1000000;

				PIN_FLIST_FLD_COPY(jv_details, PIN_FLD_ACCOUNT_NO, profile_info, PIN_FLD_ACCOUNT_NO, ebufp );
				jv_nameinfo = PIN_FLIST_ELEM_GET(jv_details, PIN_FLD_NAMEINFO, 1, 1, ebufp);

				prof_nameinfo = PIN_FLIST_ELEM_ADD(profile_info, PIN_FLD_NAMEINFO, 1, ebufp);
				PIN_FLIST_FLD_COPY(jv_nameinfo, PIN_FLD_FIRST_NAME, prof_nameinfo, PIN_FLD_FIRST_NAME, ebufp );
				PIN_FLIST_FLD_COPY(jv_nameinfo, PIN_FLD_MIDDLE_NAME, prof_nameinfo, PIN_FLD_MIDDLE_NAME, ebufp );
				PIN_FLIST_FLD_COPY(jv_nameinfo, PIN_FLD_LAST_NAME, prof_nameinfo, PIN_FLD_LAST_NAME, ebufp );
				PIN_FLIST_FLD_COPY(jv_nameinfo, PIN_FLD_COMPANY, prof_nameinfo, PIN_FLD_COMPANY, ebufp );
				PIN_FLIST_FLD_SET(prof_nameinfo, PIN_FLD_ACCOUNT_TYPE, &account_type, ebufp);
				
				PIN_FLIST_DESTROY_EX(&jv_details, NULL);
				if(!ret_flist)
				{
					*ret_flist = PIN_FLIST_COPY(profile_info, ebufp);
					PIN_FLIST_ELEM_COPY(result_flist,MSO_FLD_ACCESS_INFO,0,*ret_flist,MSO_FLD_ACCESS_INFO,0,ebufp );
				}
			}
		}

		PIN_POID_DESTROY(parent_of_csr, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "test_001");
	}
	*profile_obj = (poid_t *)PIN_FLIST_FLD_TAKE(result_flist, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "test_001");
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_check_csr_data_protection return flist", *ret_flist);

	CLEANUP:
	return;
}

/**************************************************
* Function: 	fm_mso_get_hierarchy_info()
* Owner:        Gautam Adak
* Decription:   
*               
*		
* 
* 
***************************************************/
static void
fm_mso_get_hierarchy_info(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	int32			account_model,
	int32			corp_type,
	int32			subscriber_type,
	pin_flist_t		**hierarchy_info,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*members = NULL;

	poid_t			*srch_pdp = NULL;
	int32			srch_flag = 768;
	
	int64			db = -1;
//	int				corp_type=0;
	int				child=2;
	int				parent=1;
	char			*template = "select X from /account 1, /group/billing 2 where 2.F1 = V1 and 2.F2 = 1.F3 " ;


	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_hierarchy_info", ebufp);
		return;
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_hierarchy_info :");
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_hierarchy_info acnt_pdp", acnt_pdp);
	//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, acnt_pdp );

	//corp_type = fm_mso_get_cust_corp_type(ctxp,	acnt_pdp, ebufp);
	/* return value may be:  0:End Customer, 1:Parent, 2: Child */
	if (account_model ==ACCOUNT_MODEL_CORP &&(corp_type == 0 || corp_type == -1))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account type is End Customer or Error:");
		return;
	}
	
	db = PIN_POID_GET_DB(acnt_pdp) ;
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	
	if ( account_model ==ACCOUNT_MODEL_CORP && (corp_type == CORPORATE_PARENT || corp_type == 1))
	{
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acnt_pdp , ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		members =  PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_MEMBERS, PIN_ELEMID_ANY, ebufp );
		PIN_FLIST_FLD_SET(members, PIN_FLD_OBJECT, NULL , ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL , ebufp);
	}

	if ( account_model ==ACCOUNT_MODEL_CORP && (corp_type == CORPORATE_CHILD || corp_type == 2))
	{
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		members =  PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_MEMBERS, PIN_ELEMID_ANY, ebufp );
		PIN_FLIST_FLD_SET(members, PIN_FLD_OBJECT, acnt_pdp , ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, NULL , ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL , ebufp);
	}

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_NO, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_GROUP_OBJ, NULL, ebufp);
	members =  PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_NAMEINFO, 1, ebufp );
	PIN_FLIST_FLD_SET(members, PIN_FLD_LAST_NAME, NULL, ebufp);
	PIN_FLIST_FLD_SET(members, PIN_FLD_FIRST_NAME, NULL, ebufp);


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_hierarchy_info search billinfo input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_hierarchy_info search error", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_hierarchy_info search billinfo output list", srch_oflistp);

	if (PIN_FLIST_ELEM_COUNT(srch_oflistp, PIN_FLD_RESULTS, ebufp) > 0)
	{

	if (  account_model ==ACCOUNT_MODEL_CORP && (corp_type == CORPORATE_PARENT || corp_type == 1))
	{
		PIN_FLIST_FLD_RENAME(srch_oflistp, PIN_FLD_RESULTS, PIN_FLD_MEMBERS, ebufp);
		PIN_FLIST_FLD_SET(srch_oflistp, PIN_FLD_PARENT, acnt_pdp, ebufp);
		PIN_FLIST_FLD_SET(srch_oflistp, PIN_FLD_PARENT_FLAGS, &parent, ebufp);
		*hierarchy_info = PIN_FLIST_COPY(srch_oflistp, ebufp );	
	}
	if (  account_model ==ACCOUNT_MODEL_CORP && (corp_type == CORPORATE_CHILD || corp_type == 2))
	{
		result_flist = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, 0, 1, ebufp);
		PIN_FLIST_FLD_RENAME(result_flist, PIN_FLD_POID, PIN_FLD_PARENT, ebufp);
		
		members =  PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_MEMBERS, 0, ebufp );
		PIN_FLIST_FLD_SET(members, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);
		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_PARENT_FLAGS, &child, ebufp);
		*hierarchy_info = PIN_FLIST_COPY(result_flist, ebufp );	
	}
	}
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);

	CLEANUP:
	return;
}

/**************************************************
* Function: 	fm_mso_get_deposit_details()
* Owner:        Gautam Adak
* Decription:   
*               
*		
* 
* 
***************************************************/
static void
fm_mso_get_deposit_details(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_flist_t		*srvc_info,
	pin_flist_t		**deposit_info,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*search_depoist_iflist = NULL;
	pin_flist_t		*search_depoist_oflist = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*members = NULL;
	pin_flist_t		*service_flist = NULL;
	pin_flist_t		*deposit_flist = NULL;

	poid_t			*srvc_pdp = NULL;

	int32			status_flag = 0;
	int			rec_id =0;

	int64			db = -1;
	pin_cookie_t		cookie = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_deposit_details", ebufp);
		return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_deposit_details srvc_info ", srvc_info);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, acnt_pdp );

	while ( (service_flist = PIN_FLIST_ELEM_GET_NEXT (srvc_info,
			PIN_FLD_SERVICES, &rec_id, 1,&cookie, ebufp)) != (pin_flist_t *)NULL ) 
	{
		srvc_pdp = PIN_FLIST_FLD_GET(service_flist, PIN_FLD_POID, 0, ebufp);
		search_depoist_iflist = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(search_depoist_iflist, PIN_FLD_POID, acnt_pdp, ebufp);
		PIN_FLIST_FLD_SET(search_depoist_iflist, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
		PIN_FLIST_FLD_SET(search_depoist_iflist, PIN_FLD_STATUS, &status_flag, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_deposit_details input", search_depoist_iflist);
		PCM_OP(ctxp, MSO_OP_PYMT_GET_DEPOSITS, 0, search_depoist_iflist, &search_depoist_oflist, ebufp);
		PIN_FLIST_DESTROY_EX(&search_depoist_iflist, NULL);

		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_deposit_details error", ebufp);
			return;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_deposit_details output ", search_depoist_oflist);

		deposit_flist = PIN_FLIST_ELEM_TAKE(search_depoist_oflist, PIN_FLD_RESULTS, 0, 1, ebufp);

		if (deposit_flist)
		{
			PIN_FLIST_SUBSTR_PUT(service_flist, deposit_flist, PIN_FLD_DEPOSIT, ebufp );
		}

		if (!(*deposit_info))
		{
			*deposit_info = PIN_FLIST_COPY(deposit_flist, ebufp);
		}

		PIN_FLIST_DESTROY_EX(&search_depoist_oflist, NULL);
	}

	CLEANUP:
	return;
}

/**************************************************
* Function: 	fm_mso_get_balance()
* Owner:        Gautam Adak
* Decription:   
*               
*		
* 
* 
***************************************************/
static void
fm_mso_get_balance(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	poid_t			*billinfo_obj,
	pin_flist_t		**balance_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*get_bal_iflist = NULL;
	pin_flist_t		*get_bal_oflist = NULL;

	int32			include_children = 1;

	pin_decimal_t		*unapplied_amt = NULL;
	pin_decimal_t		*pending_bill_due = NULL;
	pin_decimal_t		*open_bill_due = NULL;
	pin_decimal_t		*bill_in_progress = pbo_decimal_from_str("0.0", ebufp );

	int64			db = -1;

	/*
	Bill In Progress		<PENDINGBILL_DUE>
	Due Amount			<OPENBILL_DUE> 
	Adjustment/Payment Unapplied	<UNAPPLIED_AMOUNT>
	Total Amount			<CURRENT_BAL>  = {PENDINGBILL_DUE + OPENBILL_DUE + UNAPPLIED_AMOUNT }
	Disputes			<DISPUTED>
	*/

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_balance", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_balance :");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, acnt_pdp );

	get_bal_iflist = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(get_bal_iflist, PIN_FLD_POID, billinfo_obj, ebufp);
	PIN_FLIST_FLD_SET(get_bal_iflist, PIN_FLD_INCLUDE_CHILDREN, &include_children, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_balance input", get_bal_iflist);
	PCM_OP(ctxp, PCM_OP_AR_GET_BAL_SUMMARY, 0, get_bal_iflist, &get_bal_oflist, ebufp);
	PIN_FLIST_DESTROY_EX(&get_bal_iflist, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "PCM_OP_AR_GET_BAL_SUMMARY error", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_balance output ", get_bal_oflist);

	unapplied_amt = PIN_FLIST_FLD_GET(get_bal_oflist, PIN_FLD_UNAPPLIED_AMOUNT, 0, ebufp);
	pending_bill_due = PIN_FLIST_FLD_GET(get_bal_oflist, PIN_FLD_PENDINGBILL_DUE, 0, ebufp);
	open_bill_due = PIN_FLIST_FLD_GET(get_bal_oflist, PIN_FLD_OPENBILL_DUE, 0, ebufp);

	pbo_decimal_add_assign(bill_in_progress, unapplied_amt, ebufp);
	pbo_decimal_add_assign(bill_in_progress, pending_bill_due, ebufp);
	pbo_decimal_add_assign(bill_in_progress, open_bill_due, ebufp);

	*balance_flist = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(*balance_flist, PIN_FLD_CURRENT_BAL, bill_in_progress, ebufp );
	PIN_FLIST_FLD_COPY(get_bal_oflist, PIN_FLD_UNAPPLIED_AMOUNT, *balance_flist, PIN_FLD_UNAPPLIED_AMOUNT, ebufp);
	PIN_FLIST_FLD_COPY(get_bal_oflist, PIN_FLD_PENDINGBILL_DUE, *balance_flist, PIN_FLD_PENDINGBILL_DUE, ebufp);
	PIN_FLIST_FLD_COPY(get_bal_oflist, PIN_FLD_OPENBILL_DUE, *balance_flist, PIN_FLD_OPENBILL_DUE, ebufp);
	/**** Pavan Bellala 20-08-2015 *****
	Commented as per bug id :1214
	************************************/
	//PIN_FLIST_FLD_COPY(get_bal_oflist, PIN_FLD_DISPUTED, *balance_flist, PIN_FLD_DISPUTED, ebufp);
	PIN_FLIST_FLD_SET(*balance_flist, PIN_FLD_CREDIT_LIMIT, NULL, ebufp);

	PIN_FLIST_DESTROY_EX(&get_bal_oflist, NULL);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "balance_flist ", *balance_flist);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error balance_flist", ebufp);
	}
CLEANUP:
	return;
}

/**************************************************
* Function: 	fm_mso_populate_org_structure()
* Owner:        Gautam Adak
* Decription:   This function will be called 
*               only when an user doing self login
* 
***************************************************/
static void
fm_mso_populate_org_structure(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*profile_info = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*prof_nameinfo = NULL;
	pin_flist_t		*jv_details = NULL;
	pin_flist_t		*jv_nameinfo = NULL;
	pin_flist_t		*access_info = NULL;

	poid_t			*acnt_pdp = NULL;
	poid_t			*srch_pdp = NULL;
	poid_t			*jv_obj = NULL;
	poid_t			*profile_pdp = NULL;
	poid_t			*user_id = NULL;

	int32			srch_flag = 768;
	int32			*business_type = NULL;
	int32			tmp_business_type = -1;
	int32			account_type = -1;

	int64			db = 1;

	char			*template = "select X from /profile where F1.id = V1 and profile_t.poid_type <> '/profile/gst_info' " ;




	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_populate_org_structure", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_populate_org_structure", i_flistp);

	acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	user_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 0, ebufp);

	db = PIN_POID_GET_DB(acnt_pdp) ;
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

	/**************************************************************************
	*  Search profile 
	**************************************************************************/
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);


	PIN_FLIST_ELEM_SET(srch_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_populate_org_structure search profile input", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "m_mso_populate_org_structure search error", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_populate_org_structure search CSR profile output", srch_oflistp);

	result_flist = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, 0, 1, ebufp );
	if (result_flist)
	{
		profile_info = PIN_FLIST_SUBSTR_GET(result_flist, MSO_FLD_WHOLESALE_INFO, 1, ebufp);
		if(!profile_info)
		{
			profile_info = PIN_FLIST_SUBSTR_GET(result_flist, PIN_FLD_CUSTOMER_CARE_INFO, 1, ebufp);
		}
		else
		{
			if(PIN_POID_COMPARE(user_id, acnt_pdp, 0, ebufp )==0)
			{
				access_info = PIN_FLIST_ELEM_TAKE(result_flist,MSO_FLD_ACCESS_INFO, 0, 1,ebufp);
			}
		}
		jv_obj = PIN_FLIST_FLD_GET(profile_info, MSO_FLD_JV_OBJ, 0, ebufp );
		if (PIN_POID_GET_ID(jv_obj) !=0)
		{
			fm_mso_get_account_info(ctxp, jv_obj, &jv_details, ebufp );

			business_type = PIN_FLIST_FLD_GET(jv_details, PIN_FLD_BUSINESS_TYPE, 1, ebufp);
			tmp_business_type = *business_type;
			account_type = tmp_business_type/1000000;

			PIN_FLIST_FLD_COPY(jv_details, PIN_FLD_ACCOUNT_NO, profile_info, PIN_FLD_ACCOUNT_NO, ebufp );
			jv_nameinfo = PIN_FLIST_ELEM_GET(jv_details, PIN_FLD_NAMEINFO, 1, 1, ebufp);

			prof_nameinfo = PIN_FLIST_ELEM_ADD(profile_info, PIN_FLD_NAMEINFO, 1, ebufp);
			PIN_FLIST_FLD_COPY(jv_nameinfo, PIN_FLD_FIRST_NAME, prof_nameinfo, PIN_FLD_FIRST_NAME, ebufp );
			PIN_FLIST_FLD_COPY(jv_nameinfo, PIN_FLD_MIDDLE_NAME, prof_nameinfo, PIN_FLD_MIDDLE_NAME, ebufp );
			PIN_FLIST_FLD_COPY(jv_nameinfo, PIN_FLD_LAST_NAME, prof_nameinfo, PIN_FLD_LAST_NAME, ebufp );
			PIN_FLIST_FLD_COPY(jv_nameinfo, PIN_FLD_COMPANY, prof_nameinfo, PIN_FLD_COMPANY, ebufp );
			PIN_FLIST_FLD_SET(prof_nameinfo, PIN_FLD_ACCOUNT_TYPE, &account_type, ebufp);
			
			PIN_FLIST_DESTROY_EX(&jv_details, NULL);
		}
	}

	*ret_flist = PIN_FLIST_COPY( profile_info, ebufp );
	PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_POID, *ret_flist, PIN_FLD_PROFILE_OBJ, ebufp );
	if(access_info) {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Adding Access info", access_info);
		PIN_FLIST_ELEM_PUT(*ret_flist, access_info, MSO_FLD_ACCESS_INFO, 0, ebufp);
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "test_001");

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	return;
}



/**************************************************
* Function: 	fm_mso_get_device_details()
* Owner:        Gautam Adak
* Decription:   
*               
* Performance can be improved by replacing the searching 
* in device from loop to a single search
* 
* 
***************************************************/
static void
fm_mso_get_device_details(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*services = NULL;
	pin_flist_t		*alias_list_stb = NULL;
	pin_flist_t		*dev_info_iflist  = NULL;
	pin_flist_t		*dev_info_oflist = NULL;
	pin_flist_t		*alias_flist = NULL;
	pin_flist_t		*dev_det_flistp = NULL;
	pin_flist_t		*profile_flistp = NULL;
	
	poid_t			*srch_pdp = NULL;
	poid_t			*svc_pdp = NULL;
	
	//int				i;
	//int64			db = -1;
	//int32			elem_id = 0;
	//int32			alias_cnt = 0;
	int32			srch_flag = 512;
	int32			srvc_count = 1;
	int32			*status = NULL;
//	char			*SVC_BB_TYPE = "/service/telco/broadband";
//	char			*SVC_CATV_TYPE = "/service/catv";
	int32			*pp_type = NULL;
	int32			dvc_required = 1;
	
	char			*svc_type = NULL;
	char			*logmsg = NULL;
	int64			db = -1;
	int32			elem_id = 0;
	int32			alias_cnt=0;
	pin_cookie_t	cookie = NULL;	
    int			i;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_device_details", ebufp);
		return;
	}
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_device_details acnt_pdp", acnt_pdp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "*ret_flistp", *ret_flistp);
	
	srch_pdp = PIN_FLIST_FLD_GET(*ret_flistp, PIN_FLD_POID, 0, ebufp);
	db = PIN_POID_GET_DB(srch_pdp) ;

	srvc_count = PIN_FLIST_ELEM_COUNT(*ret_flistp, PIN_FLD_SERVICES, ebufp);
	/* Performance can be improved by replacing the searching 
	   in device from loop to a single search
	*/
	for (i=0; i <srvc_count ; i++ )
	{
		services = PIN_FLIST_ELEM_GET(*ret_flistp, PIN_FLD_SERVICES, i, 0, ebufp);
		svc_pdp=(poid_t *)PIN_FLIST_FLD_GET(services, PIN_FLD_POID, 1, ebufp );
		if (svc_pdp && !PIN_POID_IS_NULL(svc_pdp))
		{
			  svc_type = (char *)PIN_POID_GET_TYPE(svc_pdp);
		}
		status = PIN_FLIST_FLD_GET(services,PIN_FLD_STATUS,1,ebufp);
		
		if(svc_type && strcmp(svc_type,MSO_CATV)==0)
		{ 
			/* Commented below if condition on cancelled status to facilitate 
			 * the Manual GRV process for CATV customers
			 */
			/* if(*status == 10103)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Status is closed hence no device details ");
			}
			else 
			{ */
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Before calling fm_mso_cust_get_pp_type ");
			fm_mso_cust_get_pp_type(ctxp, acnt_pdp, &profile_flistp, ebufp);
			
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in fetching PP_TYPE");
				PIN_ERRBUF_RESET(ebufp);
			}
			
			if(profile_flistp != NULL)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Printing profile_flistp... ");
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_pp_type profile_flistp", profile_flistp);
				
				pp_type = PIN_FLIST_FLD_GET(profile_flistp, MSO_FLD_PP_TYPE, 1, ebufp);

				// For CATV Secondary Point Customers, as part of auto GRV, Device is disconnected.
				if ( pp_type != NULL && *pp_type == PP_TYPE_SECONDARY )
				{
					if (*status == 10103)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " CATV Secondary Point Customer, Service is Terminated. Therefore, Device Details not required ");
						dvc_required = 0;
					}
					else
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " CATV Secondary Point Customer, Service is not Terminated, fetching Device Information ");
						dvc_required = 1;
					}
				}
				else if ( pp_type != NULL && *pp_type == PP_TYPE_PRIMARY )
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG ," CATV Primary Point Customer, fetching Device Information");
					dvc_required = 1;
				}
			}
			else 
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " PP_TYPE not available for this account ");
				dvc_required = 0;
			}

			if (dvc_required == 1)
			{					
				dev_info_iflist = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(dev_info_iflist, PIN_FLD_POID, acnt_pdp, ebufp);
				PIN_FLIST_FLD_SET(dev_info_iflist, PIN_FLD_USERID, acnt_pdp, ebufp);
				
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Printing Services Flist ", services);
				
				alias_cnt = 0;
				alias_cnt = PIN_FLIST_ELEM_COUNT(services, PIN_FLD_ALIAS_LIST, ebufp);					
				
				if(alias_cnt > 0)
				{
					PIN_FLIST_ELEM_COPY(services, PIN_FLD_ALIAS_LIST, 0, dev_info_iflist, PIN_FLD_ALIAS_LIST, 0, ebufp);
					PIN_FLIST_ELEM_COPY(services, PIN_FLD_ALIAS_LIST, 1, dev_info_iflist, PIN_FLD_ALIAS_LIST, 1, ebufp);
				PIN_FLIST_ELEM_COPY(services, PIN_FLD_ALIAS_LIST, 2, dev_info_iflist, PIN_FLD_ALIAS_LIST, 2, ebufp);
		 
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_GET_DEVICE_DETAILS input list", dev_info_iflist);
				PCM_OP(ctxp, MSO_OP_CUST_GET_DEVICE_DETAILS, 0, dev_info_iflist, &dev_info_oflist, ebufp);
				PIN_FLIST_DESTROY_EX(&dev_info_iflist, NULL);

				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_CUST_GET_DEVICE_DETAILS", ebufp);
					return;
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_GET_DEVICE_DETAILS output list", dev_info_oflist);
			
				if (dev_info_oflist)
				{
					PIN_FLIST_FLD_RENAME(dev_info_oflist, PIN_FLD_RESULTS, PIN_FLD_DEVICES, ebufp);
					PIN_FLIST_FLD_DROP(dev_info_oflist, PIN_FLD_POID, ebufp) ;
					PIN_FLIST_CONCAT(services, dev_info_oflist, ebufp );
					}
					PIN_FLIST_DESTROY_EX(&dev_info_oflist, NULL);
				}
			}
		}
		else if(svc_type && strcmp(svc_type,MSO_BB)==0)
		{ 
			fm_mso_get_device_from_ser(ctxp, svc_pdp, &dev_det_flistp, ebufp);
			alias_cnt = 0;
			alias_cnt = PIN_FLIST_ELEM_COUNT(dev_det_flistp,PIN_FLD_RESULTS, ebufp);
			if(alias_cnt>0)
			{
				// Pawan:27-02-2015: commented below since it is now handled directly 
				// in function fm_mso_get_device_from_ser()
				/*dev_info_iflist = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(dev_info_iflist, PIN_FLD_POID, acnt_pdp, ebufp);
				PIN_FLIST_FLD_SET(dev_info_iflist, PIN_FLD_USERID, acnt_pdp, ebufp);
				
				cookie=NULL;
				while((alias_flist = PIN_FLIST_ELEM_GET_NEXT (dev_det_flistp,
				PIN_FLD_RESULTS, &elem_id, 1,&cookie, ebufp)) != (pin_flist_t *)NULL ) 
				{
					PIN_FLIST_FLD_RENAME(alias_flist, PIN_FLD_DEVICE_ID, PIN_FLD_NAME, ebufp);
					PIN_FLIST_ELEM_SET(dev_info_iflist, alias_flist, PIN_FLD_ALIAS_LIST, elem_id, ebufp);
					
				}
				
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_GET_DEVICE_DETAILS input list", dev_info_iflist);
				PCM_OP(ctxp, MSO_OP_CUST_GET_DEVICE_DETAILS, 0, dev_info_iflist, &dev_info_oflist, ebufp);
				PIN_FLIST_DESTROY_EX(&dev_info_iflist, NULL);

				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_CUST_GET_DEVICE_DETAILS", ebufp);
					return;
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_GET_DEVICE_DETAILS output list", dev_info_oflist);
				*/
				if (dev_det_flistp)
				{
					PIN_FLIST_FLD_RENAME(dev_det_flistp, PIN_FLD_RESULTS, PIN_FLD_DEVICES, ebufp);
					PIN_FLIST_FLD_DROP(dev_det_flistp, PIN_FLD_POID, ebufp) ;
					PIN_FLIST_CONCAT(services, dev_det_flistp, ebufp );
				}
				PIN_FLIST_DESTROY_EX(&dev_det_flistp, NULL);
			}
		}		
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_device_details final flist", *ret_flistp);

	CLEANUP:
	return;
}

static int
fm_mso_get_mso_plans(
	pcm_context_t		*ctxp,
	poid_t			*account_obj,
	pin_flist_t		**plans_flistp,
	pin_errbuf_t		*ebufp)
{

	pin_flist_t 		*mso_search_in_flistp = NULL;
	pin_flist_t 		*mso_search_res_flistp = NULL;
	pin_flist_t 		*args_flistp = NULL;
	pin_flist_t 		*results_flistp = NULL;

	poid_t 			*mso_search_pdp = NULL;
	poid_t			*mso_plan_pdp = NULL;

	char 			*mso_srch_tmpl = "select X from /mso_plans_activation where F1 = V1 and F2 = V2";

	int64			db = 0;
	int                     status_flags = 768;
	int 			ret_flg = 0;

	/****** Pavan Bellala 09/07/2015 *****************************
	Fix for maintaining same structure of out flist before and
	after activation
	******* Pavan Bellala 09/07/2015 ****************************/
	pin_flist_t		*prod_flistp = NULL;
	pin_flist_t		*plan_flistp = NULL;
	
	int			pelemid = 0;
	int			prelemid = 0;

	pin_cookie_t		pcookie = NULL;
	pin_cookie_t		prcookie = NULL;

	pin_decimal_t		*disc_amtp = NULL;
	pin_decimal_t		*disc_percentp = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_mso_plans", ebufp);
		return;
	}
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_mso_plans acnt_pdp", account_obj);

	db = PIN_POID_GET_DB(account_obj);
	mso_search_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	mso_search_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(mso_search_in_flistp, PIN_FLD_POID, mso_search_pdp, ebufp);
	PIN_FLIST_FLD_SET(mso_search_in_flistp, PIN_FLD_FLAGS, &status_flags, ebufp);
	PIN_FLIST_FLD_SET(mso_search_in_flistp, PIN_FLD_TEMPLATE, mso_srch_tmpl, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(mso_search_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);				
	status_flags = 0;
	args_flistp = PIN_FLIST_ELEM_ADD(mso_search_in_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &status_flags, ebufp);	
	results_flistp = PIN_FLIST_ELEM_ADD(mso_search_in_flistp, PIN_FLD_RESULTS, 0, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_mso_plans search input list", mso_search_in_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, mso_search_in_flistp, &mso_search_res_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&mso_search_in_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
		PIN_FLIST_DESTROY_EX(&mso_search_res_flistp, NULL);
		ret_flg = 1;
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_mso_plans search result list", mso_search_res_flistp);
	if ( mso_search_res_flistp )
	{
		if (PIN_FLIST_ELEM_COUNT(mso_search_res_flistp, PIN_FLD_RESULTS, ebufp) > 1 )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"More than 1 /mso_plans_activation is present for this account");
			PIN_FLIST_DESTROY_EX(&mso_search_res_flistp, NULL);
			ret_flg = 1;
			goto CLEANUP;
		}
		results_flistp = NULL;
		results_flistp = PIN_FLIST_ELEM_GET(mso_search_res_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		if ( !results_flistp )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"No /mso_plans_activation object created for this account");
			PIN_FLIST_DESTROY_EX(&mso_search_res_flistp, NULL);
			ret_flg = 0;
			goto CLEANUP;
		}
		mso_plan_pdp = PIN_FLIST_FLD_GET( mso_search_res_flistp, PIN_FLD_POID, 1, ebufp);
		if ( mso_plan_pdp && !PIN_POID_IS_NULL(mso_plan_pdp))
		{
			results_flistp  = PIN_FLIST_ELEM_GET(mso_search_res_flistp,PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
			/****** Pavan Bellala 09/07/2015 *****************************
			Fix for maintaining same structure of out flist before and
			after activation
			******* Pavan Bellala 09/07/2015 ****************************/
			pelemid = 0; pcookie = NULL;
			while((plan_flistp = PIN_FLIST_ELEM_GET_NEXT(results_flistp, PIN_FLD_PLAN,
								 &pelemid, 1, &pcookie, ebufp)) != (pin_flist_t *)NULL){
				prelemid = 0; prcookie = NULL;
				while((prod_flistp = PIN_FLIST_ELEM_GET_NEXT(plan_flistp, PIN_FLD_PRODUCTS,
                                                                 &prelemid, 1, &prcookie, ebufp)) != (pin_flist_t *)NULL){
					disc_amtp = PIN_FLIST_FLD_GET(prod_flistp,MSO_FLD_DISC_AMOUNT,1,ebufp);
					disc_percentp = PIN_FLIST_FLD_GET(prod_flistp,MSO_FLD_DISC_PERCENT,1,ebufp);
			
					if(!pbo_decimal_is_zero(disc_amtp,ebufp) || !pbo_decimal_is_zero(disc_percentp,ebufp))
					{
						PIN_FLIST_FLD_COPY(prod_flistp,MSO_FLD_DISC_AMOUNT,plan_flistp,MSO_FLD_DISC_AMOUNT,ebufp);
						PIN_FLIST_FLD_COPY(prod_flistp,MSO_FLD_DISC_PERCENT,plan_flistp,MSO_FLD_DISC_PERCENT,ebufp);
						break;
					}
				}					
			}
	
			*plans_flistp = PIN_FLIST_COPY(results_flistp,ebufp);
			PIN_FLIST_DESTROY_EX(&mso_search_res_flistp, NULL);
			ret_flg = 2;
			goto CLEANUP;
		}
	}
CLEANUP:
	PIN_FLIST_DESTROY_EX(&mso_search_res_flistp, NULL);
	return ret_flg;;
}


/**************************************************************
Function : fm_mso_get_lco_settlement_balance

Description :
        Function to get the LCO Balance and LCO Commission Balance bucket

***************************************************************/

static void
fm_mso_get_lco_settlement_balance (
	pcm_context_t		*ctxp,
	poid_t			*account_obj,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_i_flp = NULL;
	pin_flist_t		*srch_o_flp = NULL;
	pin_flist_t		*args_flp = NULL;
	pin_flist_t		*results_flp = NULL;
	pin_flist_t		*read_flp = NULL;
	pin_flist_t		*read_o_flp = NULL;
	pin_flist_t		*bal_flistp = NULL;
	pin_flist_t		*sub_bal_flistp = NULL;
	pin_flist_t		*balance_flistp = NULL;
	pin_flist_t		*srch_results_flp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*get_balances_flp = NULL;
	pin_flist_t		*get_balances_o_flp = NULL;
	int			flags = 256;
	/******* Pavan Bellala 14-10-2015 ******************
	Changes made to query to fetch LCO balance groups
	***************************************************/
	//char			*template = "select X from /balance_group 1, /billinfo 2 where 2.F1 = V1 and 2.F2 = V2 and 2.F3 = 1.F4 " ;
	char			*template = "select X from /balance_group where F1 = V1 and ( F2 = V2 or F3 = V3 ) " ;
	poid_t			*srch_pdp = NULL;
	int			elem_id = 0;
	pin_cookie_t		cookie = NULL;
	pin_decimal_t		*zero_p = pbo_decimal_from_str("0.0",ebufp);
	char msg[200];
	char *bal_grp_name = NULL;
	int commision_model;

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_lco_settlement_balance", ebufp);
                return;
        }

	srch_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(account_obj),"/search",-1,ebufp);
	srch_i_flp = PIN_FLIST_CREATE(ebufp);
	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_i_flp,PIN_FLD_POID,srch_pdp,ebufp);
	PIN_FLIST_FLD_SET(srch_i_flp,PIN_FLD_FLAGS,&flags,ebufp);
	PIN_FLIST_FLD_SET(srch_i_flp,PIN_FLD_TEMPLATE,template,ebufp);
	args_flp = PIN_FLIST_ELEM_ADD(srch_i_flp,PIN_FLD_ARGS,1,ebufp);
	PIN_FLIST_FLD_SET(args_flp,PIN_FLD_ACCOUNT_OBJ,account_obj,ebufp);
	args_flp = PIN_FLIST_ELEM_ADD(srch_i_flp,PIN_FLD_ARGS,2,ebufp);
	PIN_FLIST_FLD_SET(args_flp,PIN_FLD_NAME,"LCO Commission",ebufp);
	args_flp = PIN_FLIST_ELEM_ADD(srch_i_flp,PIN_FLD_ARGS,3,ebufp);
	PIN_FLIST_FLD_SET(args_flp,PIN_FLD_NAME ,"LCO Balance",ebufp);
	results_flp = PIN_FLIST_ELEM_ADD(srch_i_flp,PIN_FLD_RESULTS,0,ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Settlement Balance Search input flist",srch_i_flp);
	PCM_OP(ctxp,PCM_OP_SEARCH,0,srch_i_flp,&srch_o_flp,ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Settlement Balance Search output flist",srch_o_flp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling search", ebufp);
		goto cleanup;
	}

	srch_results_flp = PIN_FLIST_ELEM_GET(srch_o_flp,PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
	
	if(srch_results_flp != NULL) {
		while ((res_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_o_flp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) {
			get_balances_flp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(res_flistp,PIN_FLD_ACCOUNT_OBJ,get_balances_flp,PIN_FLD_POID,ebufp);
			PIN_FLIST_FLD_COPY(res_flistp,PIN_FLD_POID,get_balances_flp,PIN_FLD_BAL_GRP_OBJ,ebufp);
			sub_bal_flistp = PIN_FLIST_ELEM_ADD(get_balances_flp,PIN_FLD_BALANCES,356,ebufp);
			PIN_FLIST_FLD_SET(sub_bal_flistp,PIN_FLD_CURRENT_BAL,(void *)zero_p,ebufp);
			PIN_FLIST_FLD_SET(sub_bal_flistp,PIN_FLD_CREDIT_LIMIT,(void *)zero_p,ebufp);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "LCO get balances input flist",get_balances_flp);
			PCM_OP(ctxp,PCM_OP_BAL_GET_BALANCES,0,get_balances_flp,&get_balances_o_flp,ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "LCO get balances output flist",get_balances_o_flp);

			if (PIN_ERRBUF_IS_ERR(ebufp)) {
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_BAL_GET_BALANCES", ebufp);
				goto cleanup;
			}

			bal_flistp = PIN_FLIST_ELEM_GET(get_balances_o_flp,PIN_FLD_BALANCES,356, 1,ebufp);
			args_flp  = PIN_FLIST_ELEM_ADD(*ret_flistp,PIN_FLD_BALANCES,elem_id,ebufp);
			PIN_FLIST_FLD_COPY(res_flistp,PIN_FLD_POID,args_flp,PIN_FLD_BAL_GRP_OBJ,ebufp);
			PIN_FLIST_FLD_COPY(res_flistp,PIN_FLD_NAME,args_flp,PIN_FLD_NAME,ebufp);
			bal_grp_name = (char*) PIN_FLIST_FLD_GET( res_flistp, PIN_FLD_NAME, 1, ebufp);

			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, bal_grp_name);
			if ( strcmp ( bal_grp_name, "LCO Commission") == 0 ) {
				commision_model = fm_mso_get_commission_model(ctxp,account_obj,ebufp);
				PIN_FLIST_FLD_SET(args_flp,MSO_FLD_COMMISION_MODEL,&commision_model,ebufp);
			}

			if ( bal_flistp != NULL ) {
				PIN_FLIST_FLD_COPY(bal_flistp,PIN_FLD_CURRENT_BAL,args_flp,PIN_FLD_CURRENT_BAL,ebufp);
				PIN_FLIST_FLD_COPY(bal_flistp,PIN_FLD_CREDIT_LIMIT,args_flp,PIN_FLD_CREDIT_LIMIT,ebufp);
			}
			else {
				PIN_FLIST_FLD_SET(args_flp,PIN_FLD_CURRENT_BAL, (void*)zero_p, ebufp);
				PIN_FLIST_FLD_SET(args_flp,PIN_FLD_CREDIT_LIMIT, (void*)zero_p, ebufp);
			}

			PIN_FLIST_DESTROY_EX(&get_balances_flp,NULL);
			PIN_FLIST_DESTROY_EX(&get_balances_o_flp,NULL);
		}
	}
cleanup:
	PIN_FLIST_DESTROY_EX(&get_balances_flp,NULL);
	PIN_FLIST_DESTROY_EX(&get_balances_o_flp,NULL);
	PIN_FLIST_DESTROY_EX(&srch_i_flp,NULL);
	PIN_FLIST_DESTROY_EX(&srch_o_flp,NULL);
	pbo_decimal_destroy(&zero_p);

}

/**************************************************************
Function : fm_mso_get_commission_model

Description :
	Function to get the commission model for an LCO account

***************************************************************/
static  int
fm_mso_get_commission_model(
        pcm_context_t           *ctxp,
        poid_t                  *account_obj,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t		*srch_i_flp = NULL;
	pin_flist_t		*srch_o_flp = NULL;
	pin_flist_t		*args_flp = NULL;
	pin_flist_t		*results_flp = NULL;
	pin_flist_t		*srch_results_flp = NULL;
	int			flags = 256;
	char			*template = "select X from /profile/wholesale where F1 = V1 and F2.type = V2 " ;
	poid_t			*srch_pdp = NULL;
	pin_flist_t		*mso_info = NULL;
	int			*temp_com = NULL;
	char msg[100];
	int com_model = -1;

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_commission_model", ebufp);
                return;
        }

	srch_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(account_obj),"/search",-1,ebufp);
	srch_i_flp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_i_flp,PIN_FLD_POID,srch_pdp,ebufp);
	PIN_FLIST_FLD_SET(srch_i_flp,PIN_FLD_FLAGS,&flags,ebufp);
	PIN_FLIST_FLD_SET(srch_i_flp,PIN_FLD_TEMPLATE,template,ebufp);

	args_flp = PIN_FLIST_ELEM_ADD(srch_i_flp,PIN_FLD_ARGS,1,ebufp);
	PIN_FLIST_FLD_SET(args_flp,PIN_FLD_ACCOUNT_OBJ,account_obj,ebufp);

        args_flp = PIN_FLIST_ELEM_ADD(srch_i_flp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(args_flp, PIN_FLD_POID, PIN_POID_CREATE(PIN_POID_GET_DB(account_obj),"/profile/wholesale",-1,ebufp), ebufp);


	results_flp = PIN_FLIST_ELEM_ADD(srch_i_flp,PIN_FLD_RESULTS,0,ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"LCO Commission Model Search input flist",srch_i_flp);
	PCM_OP(ctxp, PCM_OP_SEARCH,0,srch_i_flp,&srch_o_flp,ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"LCO Commission Model Search output flist",srch_o_flp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling search", ebufp);
		goto cleanup;
	}

	srch_results_flp = PIN_FLIST_ELEM_GET(srch_o_flp,PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
	if(srch_results_flp != NULL) {
		mso_info = PIN_FLIST_SUBSTR_GET(srch_results_flp,MSO_FLD_WHOLESALE_INFO,0,ebufp);
		temp_com =  (int32 *)PIN_FLIST_FLD_GET(mso_info,MSO_FLD_COMMISION_MODEL,0,ebufp);
		if(temp_com) {
			com_model = *temp_com;
		}
	}
	sprintf(msg,"commision_model =%d ",com_model);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
	PIN_ERRBUF_CLEAR(ebufp);

cleanup:
	PIN_FLIST_DESTROY_EX(&srch_i_flp,NULL);
	PIN_FLIST_DESTROY_EX(&srch_o_flp,NULL);
	return com_model;
}

static void
fm_mso_get_credit_fup_limit(
        pcm_context_t           *ctxp,
        poid_t                 *acnt_pdp,
        char	             	*ser_city,
	int32	             	*ser_billwhen,
        pin_flist_t             **cl_fup,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *srch_in_flistp = NULL;
	pin_flist_t             *srch_out_flistp = NULL;
	pin_flist_t             *cl_out_flistp = NULL;
        pin_flist_t             *args_flistp = NULL;
        pin_flist_t             *results_flistp = NULL;
	pin_flist_t             *pp_flistp = NULL;
	pin_flist_t             *city_flistp = NULL;
	pin_flist_t             *cp_flistp = NULL;
	pin_flist_t             *tmp_cp_flistp = NULL;
	pin_flist_t				*tmp_flistp = NULL;
	pin_flist_t				*srch_acc_type_flistp = NULL;
	pin_flist_t				*srch_acc_type_o_flistp = NULL;
	pin_flist_t				*srch_pp_flistp = NULL;
	pin_flist_t				*srch_pp_o_flistp = NULL;
	pin_flist_t				*fup_flistp = NULL;
	pin_flist_t				*crd_flistp = NULL;
        poid_t                  *srch_pdp = NULL;
		poid_t                  *serv_poid = NULL;
	poid_t                  *plan_obj = NULL;
        //char                    *srch_template = "select X from /mso_purchased_plan where F1 = V1 and F2 != V2 ";
	char                    *srch_template = "select X from /mso_purchased_plan where F1 = V1 and F2 = V2 ";
	char                    *cl_srch_template = "select X from /mso_cfg_credit_limit 1, /plan 2 where 2.F1 = V1 and 2.F2 = 1.F3 ";
	char                    *fup_srch_template = "select X from /service 1 where F1 = V1 and F2 <> V2 ";
	char                    *pp_srch_template = "select X from /purchased_product where F1 = V1 and F2 <> V2 and F3 = V3 and purchased_product_t.descr like '%-GRANT' ";	
	char			blank[10] = "";
	char			*city = NULL;
	char			*prty_str = NULL;
	char			*plan_name = NULL;
	int32			*billwhen = NULL;
        int64                   db = 0;
        int                     status_flags = 768;
	int			prov_terminate = MSO_PROV_TERMINATE;
	int			prov_active = MSO_PROV_ACTIVE;
	int			elem_id = 0;
	int                     c_elem_id = 0;
	int                     fup_elem_id = 0;
	int			RES_ID_FREE_US = 1100010;
	int			RES_ID_FUP = 1000104;
	int			RES_ID_USAGE = 1000103;
	int			RES_ID_INR = 356;
	int32			prty_int = 0;
	int32			mod_prty = 0;
        pin_cookie_t            cookie = NULL;
	pin_cookie_t            c_cookie = NULL;
	pin_cookie_t            fup_cookie = NULL;
        pin_decimal_t           *zero = NULL;
        pin_decimal_t           *cl_amt = NULL;
        pin_decimal_t           *tmp_cl_amt = NULL;
        pin_decimal_t           *sum_amt = NULL;
	pin_decimal_t           *priority = NULL;
	int32			serv_status = 10103;
	int32			pp_status = 3;
	int				month;
	int32		    *indicator = NULL;
	int32		    *fup_status = NULL;
	time_t			*charged_from_t = NULL;
	time_t			*charged_to_t = NULL;
	struct tm  		*ts;
	pin_decimal_t	*new_crd_floor = NULL;
	pin_decimal_t	*crd_floor = NULL;
	pin_decimal_t	*diff_dec = NULL;
	pin_decimal_t	*day_in_month_dec = NULL;
	pin_decimal_t	*temp_dec = NULL;
	int32 			diff;
	int32			day_in_month;
	int32			day;
	int			prty = 0;
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_credit_fup_limit", ebufp);
                return;
        }
        PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_credit_fup_limit acnt_pdp", acnt_pdp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, ser_city);
	zero  = pbo_decimal_from_str("0.0",ebufp);

        db = PIN_POID_GET_DB(acnt_pdp);
        srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
        srch_in_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
        PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &status_flags, ebufp);
        PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, srch_template, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);
        
        args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 2, ebufp );
        //PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &prov_terminate, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &prov_active, ebufp);
        results_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, 0, ebufp );

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_credit_fup_limit search input list", srch_in_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
                return;
        }
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_credit_fup_limit search input list", srch_out_flistp);

	/* Create output flist with zero value for all resource ids */
	*cl_fup = PIN_FLIST_CREATE(ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(*cl_fup, MSO_FLD_CREDIT_PROFILE, 356, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CREDIT_FLOOR, zero, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CREDIT_LIMIT, zero, ebufp);
	
	PIN_FLIST_ELEM_COPY(*cl_fup, MSO_FLD_CREDIT_PROFILE, 356, *cl_fup, MSO_FLD_CREDIT_PROFILE, 1100010,  ebufp );
	PIN_FLIST_ELEM_COPY(*cl_fup, MSO_FLD_CREDIT_PROFILE, 356, *cl_fup, MSO_FLD_CREDIT_PROFILE, 1000103,  ebufp );
	PIN_FLIST_ELEM_COPY(*cl_fup, MSO_FLD_CREDIT_PROFILE, 356, *cl_fup, MSO_FLD_CREDIT_PROFILE, 1000104,  ebufp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_credit_fup_limit CL Initial flist", *cl_fup);

	/* Iterate through each plan and search it in /mso_cfg_credit_limit */	
        while ((pp_flistp = PIN_FLIST_ELEM_GET_NEXT (srch_out_flistp,
                        PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL )
        {
		mod_prty = 0;
		priority = PIN_FLIST_FLD_GET(pp_flistp, PIN_FLD_PRIORITY, 0, ebufp);
		if(priority) {
			prty_str = pbo_decimal_to_str(priority, ebufp);
			prty_int = atoi(prty_str);
			mod_prty = prty_int%1000;
		}
		plan_obj = PIN_FLIST_FLD_GET(pp_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
		fm_mso_utils_read_any_object(ctxp, plan_obj, &srch_in_flistp, ebufp);
		if(srch_in_flistp){
			plan_name = PIN_FLIST_FLD_GET(srch_in_flistp, PIN_FLD_NAME, 1, ebufp);
		}
		if((BB_ADD_MB_START <= mod_prty && BB_ADD_MB_END >= mod_prty)
			|| (BB_FUP_START <= mod_prty && BB_FUP_END >= mod_prty)){
			billwhen = &prty; //For ADD MB/GB plan, pAYTERM is zero.
		}
		else{
			billwhen = ser_billwhen;
		}
		fm_mso_get_service_quota_codes(ctxp, plan_name, billwhen, ser_city, &city_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_credit_fup_limit search error", ebufp);
        		PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
        		PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
                        return;
                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_credit_fup_limit search CL output", city_flistp);
		
		if(city_flistp)
		{
			cp_flistp = NULL;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CITY Matched & payterm matched.");
			cp_flistp = PIN_FLIST_ELEM_GET(city_flistp, MSO_FLD_CREDIT_PROFILE, RES_ID_FREE_US, 1, ebufp);
			if(cp_flistp) {
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside Free Usage.");
				cl_amt = PIN_FLIST_FLD_GET(cp_flistp, PIN_FLD_CREDIT_FLOOR, 0, ebufp);
				tmp_cp_flistp = PIN_FLIST_ELEM_GET(*cl_fup, MSO_FLD_CREDIT_PROFILE, RES_ID_FREE_US, 1, ebufp);
				tmp_cl_amt = PIN_FLIST_FLD_GET(tmp_cp_flistp, PIN_FLD_CREDIT_FLOOR, 0, ebufp);
				sum_amt = pbo_decimal_add(tmp_cl_amt, cl_amt,  ebufp);
				PIN_FLIST_FLD_PUT(tmp_cp_flistp, PIN_FLD_CREDIT_FLOOR, sum_amt, ebufp);
				//pbo_decimal_destroy(sum_amt);
			}
			cp_flistp = NULL;
			cp_flistp = PIN_FLIST_ELEM_GET(city_flistp, MSO_FLD_CREDIT_PROFILE, RES_ID_FUP, 1, ebufp);
			if(cp_flistp) {
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside FUP");
				cl_amt = PIN_FLIST_FLD_GET(cp_flistp, PIN_FLD_CREDIT_FLOOR, 0, ebufp);
				tmp_cp_flistp = PIN_FLIST_ELEM_GET(*cl_fup, MSO_FLD_CREDIT_PROFILE, RES_ID_FUP, 1, ebufp);
				tmp_cl_amt = PIN_FLIST_FLD_GET(tmp_cp_flistp, PIN_FLD_CREDIT_FLOOR, 0, ebufp);
				sum_amt = pbo_decimal_add(tmp_cl_amt, cl_amt,  ebufp);
				PIN_FLIST_FLD_PUT(tmp_cp_flistp, PIN_FLD_CREDIT_FLOOR, sum_amt, ebufp);
				//pbo_decimal_destroy(sum_amt);
			}
			cp_flistp = NULL;
			cp_flistp = PIN_FLIST_ELEM_GET(city_flistp, MSO_FLD_CREDIT_PROFILE, RES_ID_USAGE, 1, ebufp);
			if(cp_flistp) {
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside FUP");
				cl_amt = PIN_FLIST_FLD_GET(cp_flistp, PIN_FLD_CREDIT_FLOOR, 0, ebufp);
				tmp_cp_flistp = PIN_FLIST_ELEM_GET(*cl_fup, MSO_FLD_CREDIT_PROFILE, RES_ID_USAGE, 1, ebufp);
				tmp_cl_amt = PIN_FLIST_FLD_GET(tmp_cp_flistp, PIN_FLD_CREDIT_FLOOR, 0, ebufp);
				sum_amt = pbo_decimal_add(tmp_cl_amt, cl_amt,  ebufp);
				PIN_FLIST_FLD_PUT(tmp_cp_flistp, PIN_FLD_CREDIT_FLOOR, sum_amt, ebufp);
			}
			cp_flistp = NULL;
			cp_flistp = PIN_FLIST_ELEM_GET(city_flistp, MSO_FLD_CREDIT_PROFILE, RES_ID_INR, 1, ebufp);
			if(cp_flistp) {
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside FUP");
				cl_amt = PIN_FLIST_FLD_GET(cp_flistp, PIN_FLD_CREDIT_FLOOR, 0, ebufp);
				tmp_cp_flistp = PIN_FLIST_ELEM_GET(*cl_fup, MSO_FLD_CREDIT_PROFILE, RES_ID_INR, 1, ebufp);
				tmp_cl_amt = PIN_FLIST_FLD_GET(tmp_cp_flistp, PIN_FLD_CREDIT_LIMIT, 0, ebufp);
				sum_amt = pbo_decimal_add(tmp_cl_amt, cl_amt,  ebufp);
				PIN_FLIST_FLD_PUT(tmp_cp_flistp, PIN_FLD_CREDIT_LIMIT, sum_amt, ebufp);
			}
		}
		PIN_FLIST_DESTROY_EX(&city_flistp, NULL);
        	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_credit_fup_limit CL flist after ADD old", *cl_fup);
	srch_acc_type_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(srch_acc_type_flistp, PIN_FLD_POID, srch_pdp, ebufp);
        PIN_FLIST_FLD_SET(srch_acc_type_flistp, PIN_FLD_FLAGS, &status_flags, ebufp);
        PIN_FLIST_FLD_SET(srch_acc_type_flistp, PIN_FLD_TEMPLATE, fup_srch_template, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(srch_acc_type_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(srch_acc_type_flistp, PIN_FLD_ARGS, 2, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &serv_status, ebufp);

        results_flistp = PIN_FLIST_ELEM_ADD(srch_acc_type_flistp, PIN_FLD_RESULTS, 0, ebufp );

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search fup type input", srch_acc_type_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_acc_type_flistp, &srch_acc_type_o_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search fup type input", srch_acc_type_o_flistp);
        results_flistp = PIN_FLIST_ELEM_GET(srch_acc_type_o_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
        if(results_flistp)
        {
		serv_poid = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_POID, 1, ebufp);
		tmp_flistp = PIN_FLIST_ELEM_GET(results_flistp, MSO_FLD_BB_INFO, 0, 1, ebufp);
		if (tmp_flistp){
		 indicator = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_INDICATOR, 1, ebufp);
		 //fup_status = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_FUP_STATUS, 1, ebufp);
		}
		if (indicator && *indicator == 1)
		{
		        PIN_ERR_LOG_MSG(3, "It's Postpaid Customer");
				srch_pp_flistp = PIN_FLIST_CREATE(ebufp);
		        PIN_FLIST_FLD_SET(srch_pp_flistp, PIN_FLD_POID, srch_pdp, ebufp);
		        PIN_FLIST_FLD_SET(srch_pp_flistp, PIN_FLD_FLAGS, &status_flags, ebufp);
		        PIN_FLIST_FLD_SET(srch_pp_flistp, PIN_FLD_TEMPLATE, 
								pp_srch_template, ebufp);

		        args_flistp = PIN_FLIST_ELEM_ADD(srch_pp_flistp,PIN_FLD_ARGS, 1, ebufp );
		        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);
		        args_flistp = PIN_FLIST_ELEM_ADD(srch_pp_flistp, PIN_FLD_ARGS, 2, ebufp );
		        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &pp_status, ebufp);
                        args_flistp = PIN_FLIST_ELEM_ADD(srch_pp_flistp, PIN_FLD_ARGS, 3, ebufp );
                        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, serv_poid, ebufp);

		        results_flistp = PIN_FLIST_ELEM_ADD(srch_pp_flistp,PIN_FLD_RESULTS, 0, ebufp );

		        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search fup type input", 
									srch_pp_flistp);
		        PCM_OP(ctxp, PCM_OP_SEARCH, 0, 
				srch_pp_flistp, &srch_pp_o_flistp, ebufp);
		        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search pp output", 
									srch_pp_o_flistp);
		        results_flistp = PIN_FLIST_ELEM_GET(srch_pp_o_flistp,
							 PIN_FLD_RESULTS, 0, 1, ebufp);
			if (results_flistp)
			{
				PIN_ERR_LOG_MSG(3, "It's Grant Products");
				tmp_flistp = PIN_FLIST_ELEM_GET(results_flistp,
                                                 PIN_FLD_CYCLE_FEES, PIN_ELEMID_ANY, 1, ebufp);	
				if (tmp_flistp){
				   PIN_ERR_LOG_MSG(3, "It's Cycle Fee Details");
				  charged_from_t = PIN_FLIST_FLD_GET(tmp_flistp,
								 PIN_FLD_CHARGED_FROM_T, 1, ebufp);
				  charged_to_t = PIN_FLIST_FLD_GET(tmp_flistp,
                                                                 PIN_FLD_CHARGED_TO_T, 1, ebufp);
    			  ts = localtime(charged_from_t);
				  month = ts->tm_mon;
				  day = ts->tm_mday;
				  if (month == 0 || month == 2 || month == 4 || month == 6 || month == 7 || month == 9 || month == 11)
					day_in_month = 31;
				  else if (month == 1)
					day_in_month = 28;
				  else
					day_in_month = 30;
				
				 diff =  day_in_month - day + 1;
				 if (diff != day_in_month)
				 {
					 PIN_ERR_LOG_MSG(3, "Short Cycle");
					 diff_dec = pbo_decimal_from_double((double)diff, ebufp);
					 day_in_month_dec = pbo_decimal_from_double((double)day_in_month, ebufp);
					 while ((fup_flistp = PIN_FLIST_ELEM_GET_NEXT(*cl_fup, MSO_FLD_CREDIT_PROFILE, &fup_elem_id, 1, &fup_cookie, ebufp)) != NULL)
					 {
						if (fup_elem_id == 1000104 || fup_elem_id == 1100010)
						{
							PIN_ERR_LOG_MSG(3,"It has FUP or Limited Resorce");
							crd_floor = PIN_FLIST_FLD_GET (fup_flistp , PIN_FLD_CREDIT_FLOOR, 1, ebufp);
							if ((pbo_decimal_is_null(crd_floor, ebufp) == 0) && 
								(pbo_decimal_is_zero (crd_floor, ebufp) == 0)){
								PIN_ERR_LOG_MSG(3,"It has Credit Floor");
								temp_dec = pbo_decimal_multiply(crd_floor, diff_dec,ebufp);
								if (pbo_decimal_is_null(temp_dec, ebufp) == 0)
								{
									new_crd_floor = pbo_decimal_divide(temp_dec, day_in_month_dec,ebufp);
								}
								crd_flistp = PIN_FLIST_ELEM_GET(*cl_fup, MSO_FLD_CREDIT_PROFILE, fup_elem_id, 1, ebufp);
								if (crd_flistp && pbo_decimal_is_null(new_crd_floor, ebufp) == 0){
									PIN_ERR_LOG_MSG(3,"It has new credit floor");
									PIN_FLIST_FLD_SET (crd_flistp, PIN_FLD_CREDIT_FLOOR, new_crd_floor, ebufp);
								}
							}
						}
					}
				}
				else
					PIN_ERR_LOG_MSG(3, "Full Cycle");
			}
		}
	}	
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_credit_fup_limit CL flist after ADD", *cl_fup);

	PIN_POID_DESTROY(srch_pdp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	if (!pbo_decimal_is_null(zero, ebufp)){
        	pbo_decimal_destroy(&zero);
	}
	return;
}

/**************************************************
* Function: 	fm_mso_get_cust_corp_type()
* Owner:       	Pawan
* Decription:   Finds if the input customer is
*      Corp Parent or Corp child.
*		Returns:
*			   -1: ERROR
*				0: End Customer
* 				1: Corp Parent
* 				2: Corp Child
***************************************************/
static int
fm_mso_get_cust_corp_type(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*members = NULL;
	poid_t			*srch_pdp = NULL;
	int32			srch_flag = 768;
	int64			db = -1;

	char			*template = "select X from /account 1, /group/billing 2 where 2.F1 = V1 and 2.F2 = 1.F3 " ;


	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_cust_corp_type", ebufp);
		return -1;
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_cust_corp_type :");
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_cust_corp_type acnt_pdp", acnt_pdp);

	db = PIN_POID_GET_DB(acnt_pdp) ;
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	/* Check if the account is Parent: If input poid owns a group /group/billing 
		then it is corporate parent */
	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acnt_pdp , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
	members =  PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_MEMBERS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(members, PIN_FLD_OBJECT, NULL , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL , ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_NO, NULL, ebufp);


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_cust_corp_type search group input1", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
		return -1;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_cust_corp_type search group output1", srch_oflistp);

	if (PIN_FLIST_ELEM_COUNT(srch_oflistp, PIN_FLD_RESULTS, ebufp) > 0 )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Group billing found for this account: It is Parent");
		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
		return 1;
	}
	
	/* Check if the account is Child: If input poid is a member in /group/billing 
		then it is corporate child */
	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	members =  PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_MEMBERS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(members, PIN_FLD_OBJECT, acnt_pdp , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, NULL , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_cust_corp_type search group input2", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
		return -1;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_cust_corp_type search group output2", srch_oflistp);

	if (PIN_FLIST_ELEM_COUNT(srch_oflistp, PIN_FLD_RESULTS, ebufp) > 0 )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Group found where this account is member: It is Child");
		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
		return 2;
	}

	/* This is does not have a Group and also is not a member in any group.
		It is End customer. */
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	return 0;
}

static void
fm_mso_get_mso_purp_details(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_flist_t		*srvc_info,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*mso_pp_flistp = NULL;
	pin_flist_t		*plan_list_array = NULL;
	pin_flist_t		*plan_array = NULL;
	pin_flist_t		*ser_flistp = NULL;
	pin_flist_t		*srch_pt_oflistp = NULL;
	pin_flist_t		*plan_det_flistp = NULL;
	poid_t			*srch_pdp = NULL;
	poid_t			*plan_obj = NULL;
	poid_t			*pp_plan_obj = NULL;
	int32			srch_flag = 768;
	int64			db = -1;
	int			elem_id = 0;
	int			p_elem_id = 0;
	int			pl_elem_id = 0;
	int			s_elem_id = 0;
	pin_cookie_t            cookie = NULL;
	pin_cookie_t            p_cookie = NULL;
	pin_cookie_t            pl_cookie = NULL;
	pin_cookie_t            s_cookie = NULL;


	/**** Pavan Bellala 16/07/2015 - Adding provisioning tag ********/
	int			prod_elemid = 0;
	int 			*bill_when = NULL;
	pin_cookie_t		prod_cookie = NULL;
	pin_flist_t		*product_flistp = NULL;
	char			*descrp = NULL;
	char			*prov_tagp = NULL;
	char			*city = NULL;
	char			*plan_name = NULL;
	/**** Pavan Bellala 16/07/2015 - Adding provisioning tag ********/

	char			*template = "select X from /mso_purchased_plan where F1 = V1 " ;
	char			*pt_template = "select X from /mso_config_bb_pt where F1 = V1 " ;
	int			prov_added = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_mso_purp_details", ebufp);
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_mso_purp_details acnt_pdp", acnt_pdp);

	db = PIN_POID_GET_DB(acnt_pdp) ;
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acnt_pdp , ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_mso_purp_details search input", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
		return;
	}
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_mso_purp_details search output", srch_oflistp);

	//ser_flistp= PIN_FLIST_ELEM_GET(srvc_info, PIN_FLD_SERVICES, 0, 1, ebufp);
	while((mso_pp_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_oflistp, PIN_FLD_RESULTS,
				&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			// Plan obj from /mso_purchased_plan
			pp_plan_obj = PIN_FLIST_FLD_GET(mso_pp_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
			s_cookie = NULL; s_elem_id =0;
			//Netflix Changes
			while((ser_flistp = PIN_FLIST_ELEM_GET_NEXT(srvc_info, PIN_FLD_SERVICES,
				&s_elem_id, 1, &s_cookie, ebufp)) != (pin_flist_t *)NULL )
			{
				if(PIN_FLIST_SUBSTR_GET(ser_flistp, MSO_FLD_BB_INFO, 1, ebufp) != (pin_flist_t *)NULL)
				{
					pl_cookie = NULL; pl_elem_id = 0;
					srch_pt_oflistp = PIN_FLIST_SUBSTR_GET(ser_flistp, MSO_FLD_BB_INFO, 0, ebufp);
					if(srch_pt_oflistp){
						city = PIN_FLIST_FLD_GET(srch_pt_oflistp, PIN_FLD_CITY, 0, ebufp);
					}
					// Iterate through each PIN_FLD_PLAN_LISTS element in service array
					while((plan_list_array = PIN_FLIST_ELEM_GET_NEXT(ser_flistp, PIN_FLD_PLAN_LISTS,
							&pl_elem_id, 1, &pl_cookie, ebufp)) != (pin_flist_t *)NULL)
					{
						// Iterate through each PIN_FLD_PLAN element in PIN_FLD_PLAN_LISTS array.
						p_cookie = NULL; p_elem_id =0;
						while((plan_array = PIN_FLIST_ELEM_GET_NEXT(plan_list_array, PIN_FLD_PLAN,
								&p_elem_id, 1, &p_cookie, ebufp)) != (pin_flist_t *)NULL)
						{
							plan_obj = PIN_FLIST_FLD_GET(plan_array, PIN_FLD_PLAN_OBJ, 0, ebufp);
							// If poid matches that copy the DESCR from mso_purchased_plan flist.
							if(PIN_POID_COMPARE(pp_plan_obj, plan_obj, 0, ebufp) == 0) {
								PIN_FLIST_FLD_COPY(mso_pp_flistp, PIN_FLD_DESCR, plan_array, PIN_FLD_DESCR, ebufp);
		
								/**** Pavan Bellala 16/07/2015 - Adding provisioning tag ********/
								descrp = PIN_FLIST_FLD_GET(mso_pp_flistp,PIN_FLD_DESCR,1,ebufp);
								if(strstr(descrp,"base"))
								{
									PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Base plan");
									prod_elemid = 0, prod_cookie = NULL; prov_added = 0;
									while((product_flistp = PIN_FLIST_ELEM_GET_NEXT(mso_pp_flistp,PIN_FLD_PRODUCTS,
											&prod_elemid, 1, &prod_cookie, ebufp)) != (pin_flist_t *)NULL) {
										prov_tagp = PIN_FLIST_FLD_GET(product_flistp,PIN_FLD_PROVISIONING_TAG,1,ebufp);
										PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"product_flistp array",product_flistp);
										//if(prov_tagp){
										if(prov_tagp && (strlen(prov_tagp)>0)){
											PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,prov_tagp);
											prov_added = 1;
											PIN_FLIST_FLD_COPY(product_flistp,PIN_FLD_PROVISIONING_TAG,
													plan_array, PIN_FLD_PROVISIONING_TAG, ebufp);	
											fm_mso_utils_read_any_object(ctxp, pp_plan_obj, &srch_flistp, ebufp);
											if(srch_flistp){
												plan_name = PIN_FLIST_FLD_GET(srch_flistp, PIN_FLD_NAME,
																0, ebufp);
											}
											bill_when = PIN_FLIST_FLD_GET(mso_pp_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);
											result_flist = NULL;
											fm_mso_get_service_quota_codes(ctxp, plan_name, bill_when, city,&result_flist, ebufp);
											if (PIN_ERRBUF_IS_ERR(ebufp))
											{
												PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in getting SERVICE CODE", ebufp);
												PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
												return;
											}
											/*db = PIN_POID_GET_DB(acnt_pdp) ;
											srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
											srch_flistp = PIN_FLIST_CREATE(ebufp);
											PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
											PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
											PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, pt_template , ebufp);
		
											arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
											PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_PROVISIONING_TAG, prov_tagp , ebufp);
											result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
											PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "/mso_config_bb_pt search input", srch_flistp);
											PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_pt_oflistp, ebufp);
											if (PIN_ERRBUF_IS_ERR(ebufp))
											{
												PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
												PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
												return;
											}
											PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
											PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "/mso_config_bb_pt search output", srch_pt_oflistp);
											result_flist = PIN_FLIST_ELEM_GET(srch_pt_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY,1, ebufp );*/
											if(result_flist) {
												PIN_FLIST_FLD_COPY(result_flist,MSO_FLD_DL_SPEED,plan_array,MSO_FLD_DL_SPEED, ebufp);
												PIN_FLIST_FLD_COPY(result_flist,MSO_FLD_UL_SPEED,plan_array,MSO_FLD_UL_SPEED, ebufp);
												PIN_FLIST_FLD_COPY(result_flist,MSO_FLD_FUP_DL_SPEED,plan_array,MSO_FLD_FUP_DL_SPEED, ebufp);
												PIN_FLIST_FLD_COPY(result_flist,MSO_FLD_FUP_UL_SPEED,plan_array,MSO_FLD_FUP_UL_SPEED, ebufp);	
											}
											PIN_FLIST_DESTROY_EX(&result_flist, NULL);
											PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
										}
										if(prov_added == 1)
										{
											break;
										}
									}
								}
								/**** Pavan Bellala 16/07/2015 - Adding provisioning tag ********/
							}				
						}
					}
				}
			}
		}


	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_mso_purp_details services output", srvc_info);
}		

/************************************************
* Function: fm_mso_get_ncr_balance
*			Fetches the Non currency resource
*			balances and adds in flist.
*************************************************/
static int
fm_mso_get_ncr_balance(
	pcm_context_t		*ctxp,
	pin_flist_t		*srvc_info,
	pin_flist_t		**cl_fup,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*ser_flistp = NULL;
	pin_flist_t		*bal_in_flistp = NULL;
	pin_flist_t		*bal_out_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*cp_flistp = NULL;
	int32			srch_flag = 0;
	int32			lock = 0;
	int				elem_id = 0;
	pin_cookie_t    cookie = NULL;
	pin_decimal_t	*zero = NULL;
	pin_decimal_t	*curr_bal = NULL;
	pin_decimal_t	*res_amt = NULL;
	pin_decimal_t	*total_amt = NULL;
	char          *service_type = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_ncr_balance", ebufp);
		return 1;
	}
	zero = pbo_decimal_from_str("0.0", ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_ncr_balance input flist", srvc_info);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_ncr_balance CL flist", *cl_fup);
	ser_flistp= PIN_FLIST_ELEM_GET(srvc_info, PIN_FLD_SERVICES, 0, 1, ebufp);
	service_type = (char *)PIN_POID_GET_TYPE(PIN_FLIST_FLD_GET(ser_flistp,PIN_FLD_POID,1,ebufp));
	if(strstr(service_type,"/service/netflix"))
	{
		ser_flistp= PIN_FLIST_ELEM_GET(srvc_info, PIN_FLD_SERVICES, 1, 1, ebufp);
	}
	if(ser_flistp)
	{
		bal_in_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(ser_flistp, PIN_FLD_ACCOUNT_OBJ, bal_in_flistp,PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(ser_flistp, PIN_FLD_BAL_GRP_OBJ, bal_in_flistp,PIN_FLD_BAL_GRP_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(ser_flistp, PIN_FLD_POID, bal_in_flistp,PIN_FLD_SERVICE_OBJ, ebufp);
		PIN_FLIST_FLD_SET(bal_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
		PIN_FLIST_FLD_SET(bal_in_flistp, PIN_FLD_READ_BALGRP_MODE, &lock, ebufp);
		PIN_FLIST_ELEM_SET(bal_in_flistp, NULL, PIN_FLD_BALANCES,PIN_ELEMID_ANY , ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "get_balance input flist", bal_in_flistp);
		PCM_OP(ctxp, PCM_OP_BAL_GET_BALANCES, 0, bal_in_flistp, &bal_out_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_BAL_GET_BALANCES", ebufp);
			PIN_FLIST_DESTROY_EX(&bal_in_flistp, NULL);
			return 1;
		}
		PIN_FLIST_DESTROY_EX(&bal_in_flistp, NULL);

		/*Iterate through each Resource array*/
		while((cp_flistp = PIN_FLIST_ELEM_GET_NEXT(*cl_fup, MSO_FLD_CREDIT_PROFILE,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			arg_flist = PIN_FLIST_ELEM_GET(bal_out_flistp, PIN_FLD_BALANCES, elem_id, 1, ebufp);
			if(arg_flist)
			{
				// Commented below line and added the logic to consider reserved amount
				// for calculating current balance.
				// PIN_FLIST_FLD_COPY(arg_flist, PIN_FLD_CURRENT_BAL, cp_flistp, PIN_FLD_CURRENT_BAL, ebufp);
				curr_bal = PIN_FLIST_FLD_GET(arg_flist, PIN_FLD_CURRENT_BAL, 0, ebufp);
				res_amt = PIN_FLIST_FLD_GET(arg_flist, PIN_FLD_RESERVED_AMOUNT, 0, ebufp);
				if(curr_bal && res_amt) {
					total_amt = pbo_decimal_add(curr_bal, res_amt, ebufp);
					PIN_FLIST_FLD_SET(cp_flistp, PIN_FLD_CURRENT_BAL, total_amt, ebufp );
				} else {
					PIN_FLIST_FLD_COPY(arg_flist, PIN_FLD_CURRENT_BAL, cp_flistp, PIN_FLD_CURRENT_BAL, ebufp);
				}
			}
			else 
			{
				PIN_FLIST_FLD_SET(cp_flistp, PIN_FLD_CURRENT_BAL, zero, ebufp);
			}
			if (!pbo_decimal_is_null(total_amt, ebufp)){
        			pbo_decimal_destroy(&total_amt);
			}
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_ncr_balance output flist", *cl_fup);
	}
	cleanup:
	if (!pbo_decimal_is_null(zero, ebufp)){
       		pbo_decimal_destroy(&zero);
	}
	PIN_FLIST_DESTROY_EX(&bal_out_flistp, NULL);
	return 0;
}

/**************************************************
* Function: 	fm_mso_get_device_from_ser()
* Decription:   Returns the device id of all the 
*            devices associated with service
*			  passed in input. 
***************************************************/
static void
fm_mso_get_device_from_ser(
	pcm_context_t		*ctxp,
	poid_t			*ser_pdp,
	pin_flist_t		**ret_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_flist_t		*ser_flistp = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*res_cmp_flist =NULL;
	pin_flist_t		*tmp_flistp =NULL;
	pin_flist_t		*acct_flistp =NULL;
	pin_flist_t		*nameimfo =NULL;
	char			*source = NULL;
	int		       elem_id = 0;
	pin_cookie_t	       tcookie = NULL;
	char			*company = NULL;

	poid_t			*srch_pdp = NULL;
	int32			srch_flag = 512;
	int64			db = 1;

	char			*template = "select X from /device  where F1 = V1 " ;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_device_from_ser", ebufp);
		return;
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_device_from_ser service poid:");
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, ser_pdp, ebufp);

	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	ser_flistp = PIN_FLIST_ELEM_ADD(arg_flistp, PIN_FLD_SERVICES, 0, ebufp );
	PIN_FLIST_FLD_SET(ser_flistp, PIN_FLD_SERVICE_OBJ, ser_pdp, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_DEVICE_ID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_SERIAL_NO, NULL, ebufp );
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_WARRANTY_END, NULL, ebufp );
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_WARRANTY_END_OFFSET, NULL, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_DESCR, NULL, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_DEVICE_TYPE, NULL, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_MANUFACTURER, NULL, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_MODEL, NULL, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_SOURCE, NULL, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STATE_ID, NULL, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_CATEGORY, NULL, ebufp );
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_VENDOR_WARRANTY_END, NULL, ebufp );
        PIN_FLIST_FLD_SET(result_flist, MSO_FLD_CHANNEL_NAME, NULL, ebufp );
	PIN_FLIST_ELEM_SET(result_flist,NULL, PIN_FLD_PLAN, PIN_ELEMID_ANY, ebufp );
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_device_from_ser search device input flist", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_device_from_ser search device output flist", srch_out_flistp);

	tcookie = NULL;
	elem_id = 0;
	while((tmp_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_out_flistp, PIN_FLD_RESULTS,&elem_id, 1, &tcookie, ebufp)) != (pin_flist_t *)NULL)
	{
	 source = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_SOURCE, 1, ebufp);	
	 fm_mso_srch_device_sp_name_comp(ctxp,source,&acct_flistp, ebufp); 
	    if(!PIN_ERRBUF_IS_ERR(ebufp))
	    {
		if(acct_flistp)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "acct_flistp::", acct_flistp);
			nameimfo =  PIN_FLIST_ELEM_TAKE(acct_flistp, PIN_FLD_NAMEINFO,PIN_ELEMID_ANY,1, ebufp);
			if(nameimfo)
			{
				company = PIN_FLIST_FLD_TAKE(nameimfo, PIN_FLD_COMPANY, 1, ebufp);
			}
			if(company)
			{
				PIN_FLIST_FLD_PUT(tmp_flistp, PIN_FLD_COMPANY, company, ebufp);
			}
			else
			{
				PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_COMPANY, "", ebufp);
			}
		
		    PIN_FLIST_DESTROY_EX(&nameimfo, NULL);
		} else
		{
		    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_COMPANY, "", ebufp);
		}
	    }
	    PIN_FLIST_DESTROY_EX(&acct_flistp, NULL);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "TMP FLIST", tmp_flistp);
	//PIN_FLIST_ELEM_COPY(tmp_flistp, PIN_FLD_RESULTS, elem_id,srch_out_flistp, PIN_FLD_RESULTS, elem_id, ebufp);
      }

	//result_flist = PIN_FLIST_ELEM_TAKE(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_device_from_ser search device Modified output flist", srch_out_flistp);
	*ret_flist = PIN_FLIST_COPY(srch_out_flistp, ebufp );
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);

	return;
}

/**************************************************
* Function: 	fm_mso_get_suppression_profile()
* Decription:   Returns the bill suppression 
*            profile for account.
*			  
* 
***************************************************/
static void
fm_mso_get_suppression_profile(
	pcm_context_t		*ctxp,
	poid_t			*acc_pdp,
	pin_flist_t		**ret_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_flist_t		*ser_flistp = NULL;
	pin_flist_t		*result_flist = NULL;

	poid_t			*srch_pdp = NULL;
	int32			srch_flag = 512;
	int64			db = 1;

	char			*template = "select X from /mso_bill_suppression_profile  where F1 = V1 " ;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_suppression_profile", ebufp);
		return;
	}
    PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_suppression_profile account poid:",acc_pdp);

	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACCOUNT_OBJ, acc_pdp, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_REASON, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_FLAGS, NULL, ebufp );
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_suppression_profile search input flist", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
        PIN_ERRBUF_RESET(ebufp);
        PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_suppression_profile search output flist", srch_out_flistp);
	
	result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp );
	if(result_flist)
	{
		*ret_flist = PIN_FLIST_CREATE(ebufp);
		arg_flistp = PIN_FLIST_SUBSTR_ADD(*ret_flist, PIN_FLD_SUPPRESSION_INFO, ebufp );
		PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_REASON, arg_flistp, PIN_FLD_REASON, ebufp);
		PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_FLAGS, arg_flistp, PIN_FLD_FLAGS, ebufp);
	}
	
	//*ret_flist = PIN_FLIST_COPY(srch_out_flistp, ebufp );
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);

	return;
}

/**************************************************
* Function: 	fm_mso_get_csr_ar_credit_limit()
* Decription:   Returns the bill suppression 
*            profile for account.
*			  
* 
***************************************************/
static void
fm_mso_get_csr_ar_credit_limit(
	pcm_context_t		*ctxp,
	poid_t			*acc_pdp,
	pin_flist_t		**ret_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_flist_t		*ser_flistp = NULL;
	pin_flist_t		*result_flist = NULL;

	poid_t			*srch_pdp = NULL;
	int32			srch_flag = 512;
	int64			db = 1;

	char			*template = "select X from /mso_ar_limit  where F1.id = V1 " ;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_csr_ar_credit_limit", ebufp);
		return;
	}
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_csr_ar_credit_limit account poid:",acc_pdp);

	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACCOUNT_OBJ, acc_pdp, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_ADJ_CURRENT_VALUE, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_ADJ_LIMIT, NULL, ebufp );
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_CURRENT_MONTH, NULL, ebufp );
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_MONTHLY_ADJ_LIMIT, NULL, ebufp );
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_csr_ar_credit_limit search input flist", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_csr_ar_credit_limit search output flist", srch_out_flistp);
	
	result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp );
	
	if(result_flist)
	{
		*ret_flist = PIN_FLIST_COPY(result_flist, ebufp);
	}
	
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);

	return;
}


static void
fm_mso_get_subcription_plan(
        pcm_context_t           *ctxp,
        poid_t                  *acnt_pdp,
	poid_t 			*plan_pdp,	
	pin_flist_t     	**o_flistpp,
        pin_errbuf_t            *ebufp)
{


	pin_flist_t             *srch_flistp = NULL;
        pin_flist_t             *arg_flist = NULL;
        pin_flist_t             *result_flist = NULL;
        pin_flist_t             *srch_oflistp = NULL;
	pin_flist_t             *subs_prod_flistp = NULL;
	pin_flist_t             *mso_pur_plan_flist = NULL;
	pin_flist_t             *mso_pur_flistp = NULL;	
	pin_flist_t             *mso_pur_out_flistp = NULL;
	pin_flist_t		*product_flistp = NULL;

        poid_t                  *srch_pdp = NULL;
	poid_t                  *mso_pur_plan_poid = NULL;
	poid_t			*subs_prod_poid = NULL;	

        int32                   srch_flag = 512;
        int32                   is_sub_plan = 0;
	int64                   db = 1;	

	int                     prod_elemid = 0;
        pin_cookie_t            prod_cookie = NULL;	
	char                    *prov_tagp = NULL;
	char   *template = "select X from /mso_purchased_plan where F1 = V1 and F2 = V2 and F3 = V3 " ;	

	subs_prod_flistp = PIN_FLIST_CREATE(ebufp);
	
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_subscription_plan input acnt_pdp", acnt_pdp);
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_subscription_plan input acnt_pdp", plan_pdp);

	db = PIN_POID_GET_DB(acnt_pdp) ;
        srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

        srch_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
	
        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_DESCR, "base plan", ebufp);
	
        PIN_FLIST_ELEM_SET(srch_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp );

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_subscription_plan search input flist", srch_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);	

	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_subscription_plan search error", ebufp);
                goto CLEANUP;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_subscription_plan output list", srch_oflistp);

        if (PIN_FLIST_ELEM_COUNT(srch_oflistp, PIN_FLD_RESULTS, ebufp) == 0)
        {
                PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
		PIN_FLIST_FLD_SET(subs_prod_flistp, PIN_FLD_POID, srch_pdp, ebufp);
		PIN_FLIST_FLD_SET(subs_prod_flistp,PIN_FLD_STATUS_FLAGS,&is_sub_plan,ebufp);
		PIN_FLIST_FLD_SET(subs_prod_flistp, PIN_FLD_PROVISIONING_TAG, "", ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_get_actual_subscription plan output flist inside function", subs_prod_flistp);
        }
	else {

		is_sub_plan = 1;
		mso_pur_plan_flist = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp );	
	        mso_pur_plan_poid  = PIN_FLIST_FLD_GET(mso_pur_plan_flist, PIN_FLD_POID, 0, ebufp);	
		mso_pur_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(mso_pur_flistp,PIN_FLD_POID,mso_pur_plan_poid, ebufp );
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read object input:", mso_pur_flistp);
                PCM_OP(ctxp, PCM_OP_READ_OBJ, 0,mso_pur_flistp, &mso_pur_out_flistp, ebufp);
                PIN_FLIST_DESTROY_EX(&mso_pur_flistp,NULL);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read object output:", mso_pur_out_flistp);
	
		while((product_flistp = PIN_FLIST_ELEM_GET_NEXT(mso_pur_out_flistp,PIN_FLD_PRODUCTS,
                                &prod_elemid, 1, &prod_cookie, ebufp)) != (pin_flist_t *)NULL) {	

				 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_get_actual_subscription plan output flist inside loop", subs_prod_flistp);	
			
				prov_tagp = PIN_FLIST_FLD_GET(product_flistp,PIN_FLD_PROVISIONING_TAG,1,ebufp);	
				if(prov_tagp && (strlen(prov_tagp)>0)) {

                                 	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"valid mso_pp_product_flistp array",product_flistp);
                                 	subs_prod_poid  = PIN_FLIST_FLD_GET(product_flistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
                                 	PIN_FLIST_FLD_SET(subs_prod_flistp,PIN_FLD_POID,subs_prod_poid, ebufp);
                                	PIN_FLIST_FLD_SET(subs_prod_flistp,PIN_FLD_STATUS_FLAGS,&is_sub_plan,ebufp);
                                        PIN_FLIST_FLD_SET(subs_prod_flistp, PIN_FLD_PROVISIONING_TAG, prov_tagp , ebufp);
                                 	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_get_actual_subscription plan output flist inside loop", subs_prod_flistp);
                                 	break;
                       		}
		}
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_get_actual_subscription plan output flist inside function", subs_prod_flistp);
	*o_flistpp = PIN_FLIST_COPY(subs_prod_flistp, ebufp);

	CLEANUP:		
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&mso_pur_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&subs_prod_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&mso_pur_out_flistp, NULL);

	return; 

}

void
fm_mso_get_cmts_mac(
        pcm_context_t   *ctxp,
        poid_t     	*acnt_pdp,
        pin_flist_t     **out_flistp,
        pin_errbuf_t    *ebufp)
{
	pin_flist_t	*srch_i_flistp = NULL;
	pin_flist_t	*srch_o_flistp = NULL;
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*res_flistp = NULL;
	pin_flist_t	*srch_res_flistp = NULL;
	pin_flist_t	*results_flistp = NULL;
	pin_flist_t	*isp_flistp = NULL;
	poid_t		*srch_pdp = NULL;
	poid_t		*srvc_pdp = NULL;
	char		*template = "select X from /service 1 where 1.F1 = V1 and 1.F2 = V2  order by created_t desc";
	int32		flags = 256;
	int32		failure = 1;
	int32		success = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
		PIN_ERRBUF_CLEAR(ebufp);
	}
	
	srch_pdp = PIN_POID_CREATE(1, "/search", -1, ebufp);
	srvc_pdp = PIN_POID_CREATE(1, "/service/telco/broadband", -1, ebufp);
	srch_i_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_i_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_i_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(srch_i_flistp, PIN_FLD_TEMPLATE, template, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_PUT(args_flistp, PIN_FLD_POID, srvc_pdp, ebufp);
	res_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_STATUS, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_CREATED_T,  NULL, ebufp);
	isp_flistp = PIN_FLIST_SUBSTR_ADD(res_flistp, MSO_FLD_BB_INFO, ebufp);
	PIN_FLIST_FLD_SET(isp_flistp, MSO_FLD_NETWORK_ELEMENT, NULL, ebufp);
	PIN_FLIST_FLD_SET(isp_flistp, MSO_FLD_MAC_ID, NULL, ebufp);
	PIN_FLIST_FLD_SET(isp_flistp, MSO_FLD_ID, NULL, ebufp);
	
	PIN_ERR_LOG_FLIST(3, "Search input flistp", srch_i_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_i_flistp, &srch_o_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
		PIN_ERR_LOG_MSG(1, "Error during search");
		PIN_ERRBUF_CLEAR(ebufp);
		*out_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*out_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
		PIN_FLIST_FLD_SET(*out_flistp, PIN_FLD_STATUS, &failure, ebufp);
		PIN_FLIST_FLD_SET(*out_flistp, PIN_FLD_ERROR_CODE, "50001", ebufp);
		PIN_FLIST_FLD_SET(*out_flistp, PIN_FLD_ERROR_DESCR, "Error during MAC/CMTS search", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(3, "Search output flist", srch_o_flistp);
	results_flistp = PIN_FLIST_ELEM_GET(srch_o_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	if(results_flistp)
	{
		isp_flistp = PIN_FLIST_SUBSTR_GET(results_flistp, MSO_FLD_BB_INFO, 1, ebufp);
		*out_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*out_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
                PIN_FLIST_FLD_SET(*out_flistp, PIN_FLD_STATUS, &success, ebufp);
		PIN_FLIST_FLD_COPY(isp_flistp, MSO_FLD_NETWORK_ELEMENT, *out_flistp, MSO_FLD_NETWORK_ELEMENT, ebufp);
		PIN_FLIST_FLD_COPY(isp_flistp, MSO_FLD_MAC_ID, *out_flistp, MSO_FLD_MAC_ID, ebufp);
		PIN_ERR_LOG_FLIST(3, "search results flist", *out_flistp);
		goto CLEANUP;
	}
	*out_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*out_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
	PIN_FLIST_FLD_SET(*out_flistp, PIN_FLD_STATUS, &success, ebufp);
	PIN_FLIST_FLD_SET(*out_flistp, PIN_FLD_ERROR_CODE, "50002", ebufp);
	PIN_FLIST_FLD_SET(*out_flistp, PIN_FLD_ERROR_DESCR, "Error during MAC/CMTS search", ebufp);

	CLEANUP:
		PIN_FLIST_DESTROY_EX(&srch_i_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_o_flistp, NULL);
		return;

}


/**************************************************
* Function: 	fm_mso_get_profile_offer()
* Decription:   Returns the offers in profile_cust_offer table 
***************************************************/
static void
fm_mso_get_profile_offer(
	pcm_context_t		*ctxp,
	poid_t			*ser_pdp,
	pin_flist_t		**ret_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_flist_t		*result_flist = NULL;

	poid_t			*srch_pdp = NULL;
	int32			srch_flag = 512;
	int64			db = 1;

	char			*template = "select X from /profile_cust_offer  where F1 = V1 " ;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_device_from_ser", ebufp);
		return;
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_profile_offer account poid:");
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, ser_pdp, ebufp);

	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACCOUNT_OBJ, ser_pdp, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_OFFER_VALUE, NULL, ebufp );
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_OFFER_DESCR, NULL, ebufp );
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_PAI, NULL, ebufp );
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_TOKENID, NULL, ebufp );
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_ISREGISTERED, NULL, ebufp );
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_RMN, NULL, ebufp );
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_profile_offer search profile input flist", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_profile_offer output flist", srch_out_flistp);	

	//result_flist = PIN_FLIST_ELEM_TAKE(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp );
	PIN_FLIST_FLD_RENAME(srch_out_flistp,PIN_FLD_RESULTS,PIN_FLD_PROFILE_CUST_OFFER,ebufp);
	*ret_flist = PIN_FLIST_COPY(srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);

	return;
}



/**************************************************
* Function:     fm_mso_srch_device_sp_name_comp()
* Owner:        Harish
* Decription:   Search the account associated to device
***************************************************/
static void
fm_mso_srch_device_sp_name_comp(
	pcm_context_t           *ctxp,
	char                    *source,
	pin_flist_t             **ret_flistp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t             *srch_flistp = NULL;
	pin_flist_t             *out_flistp = NULL;
	pin_flist_t             *srch_out_flistp = NULL;
	pin_flist_t             *arg_flist = NULL;
	pin_flist_t             *result_flist = NULL;
	pin_flist_t             *serv_flist = NULL;
	pin_flist_t             *nameinfo_flist = NULL;
	int32                   srch_flag = 256;
	poid_t                  *s_pdp = NULL;
	poid_t                  *new_d_pdp = NULL;
	int64                   db = 1;
	int32                   id = -1;
	char                    *s_template = "select x from /account where F1 = V1 ";

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_device_sp_name input SP code :");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,source);

	srch_flistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
	
	/*
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS           INT [0] 256
	0 PIN_FLD_TEMPLATE        STR [0] "select x from /account where F1 = V1 "
	0 PIN_FLD_ARGS          ARRAY [1] allocated 20, used 1
	1     PIN_FLD_ACCOUNT_NO      STR [0] "112332555"
	0 PIN_FLD_RESULTS       ARRAY [0] allocated 20, used 1
	1     PIN_FLD_ACCOUNT_NO      STR [0] NULL str ptr
	1     PIN_FLD_NAMEINFO	      ARRAY [*]
	2	PIN_FLD_COMPANY		STR [0] NULL str ptr

	nap(32695)> xop 7 0 1
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_RESULTS       ARRAY [0] allocated 20, used 2
	1     PIN_FLD_ACCOUNT_NO      STR [0] "1000015876"
	1     PIN_FLD_POID           POID [0] 0.0.0.1 /account 4870555 7
	1     PIN_FLD_NAMEINFO		[1]
	2	PIN_FLD_COMPANY		STR [0] "HATHWAY"	
	*/

	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, s_pdp , ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag , ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, s_template , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_NO, source , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_NO, "" , ebufp);
	nameinfo_flist = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_NAMEINFO, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(nameinfo_flist, PIN_FLD_COMPANY, NULL , ebufp);
	

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_device_sp_name search input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_srch_device_sp_name - Error in calling SEARCH", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_device_sp_name search output flist", srch_out_flistp);
	if(PIN_FLIST_ELEM_COUNT(srch_out_flistp,PIN_FLD_RESULTS,ebufp) > 0)
	{
		result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
		*ret_flistp = PIN_FLIST_COPY(result_flist, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_device_sp_name return flist", *ret_flistp);
	}

	CLEANUP:
		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	return;
}

/**************************************************
* Function:     fm_mso_cust_get_netf_bal()
* Owner:        Suresh Nadar
* Decription:   Retrieves the Netflix Balances.
***************************************************/

void fm_mso_cust_get_netf_bal(
        pcm_context_t           *ctxp,
        poid_t                  *account_pdp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t		*rf_flistp = NULL;
	pin_flist_t		*nc_bal_flistp = NULL;
	pin_flist_t		*nf_flistp = NULL;
	pin_flist_t		*nc_bal_oflistp = NULL;
	pin_flist_t		*sub_bal_flistp = NULL;
	pin_decimal_t		*zero_p = pbo_decimal_from_str("0.0",ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	//BALANCE OF NETFLIX BG
	fm_mso_retrieve_service_netflix(ctxp,account_pdp,&rf_flistp,ebufp);
	if(rf_flistp && rf_flistp !=NULL && PIN_FLIST_ELEM_COUNT(rf_flistp, PIN_FLD_RESULTS, ebufp) > 0)
	{
	nf_flistp = PIN_FLIST_ELEM_GET(rf_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	nc_bal_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(nc_bal_flistp, PIN_FLD_BAL_GRP_OBJ,PIN_FLIST_FLD_GET(nf_flistp, PIN_FLD_BAL_GRP_OBJ,1,ebufp), ebufp);
	PIN_FLIST_FLD_SET(nc_bal_flistp, PIN_FLD_POID,account_pdp, ebufp);
	sub_bal_flistp = PIN_FLIST_ELEM_ADD(nc_bal_flistp,PIN_FLD_BALANCES,PIN_ELEMID_ANY,ebufp);
	PIN_FLIST_FLD_SET(sub_bal_flistp,PIN_FLD_CURRENT_BAL,(void *)zero_p,ebufp);
	PIN_FLIST_FLD_SET(sub_bal_flistp,PIN_FLD_CREDIT_LIMIT,(void *)zero_p,ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Netflix NCR balances input flist",nc_bal_flistp);
	PCM_OP(ctxp,PCM_OP_BAL_GET_BALANCES,0,nc_bal_flistp,&nc_bal_oflistp,ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Netflix NCR balances output flist",nc_bal_oflistp);

	nf_flistp = PIN_FLIST_ELEM_GET(nc_bal_oflistp, PIN_FLD_BALANCES, 1000358, 1, ebufp);

	if (nf_flistp)
	{
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		sub_bal_flistp = PIN_FLIST_ELEM_ADD(*r_flistpp,MSO_FLD_CREDIT_PROFILE,1000358,ebufp);
		
		PIN_FLIST_FLD_COPY(nf_flistp, PIN_FLD_CURRENT_BAL, sub_bal_flistp, PIN_FLD_CURRENT_BAL, ebufp);
		PIN_FLIST_FLD_COPY(nf_flistp, PIN_FLD_CREDIT_LIMIT, sub_bal_flistp, PIN_FLD_CREDIT_LIMIT, ebufp);
		PIN_FLIST_FLD_COPY(nf_flistp, PIN_FLD_CREDIT_FLOOR, sub_bal_flistp, PIN_FLD_CREDIT_FLOOR, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Netflix NCR balances return flist",*r_flistpp);
	}
	PIN_FLIST_DESTROY_EX(&nc_bal_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&rf_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&nc_bal_oflistp, NULL);
	pbo_decimal_destroy(&zero_p);
	}
	return;
}

/*************************************************************************************
 * 
 * This function retrieves Wallet balance
 *************************************************************************************/
int32
fm_mso_get_wallet_balance(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	int32			wallet_res_id,	
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*bal_flistp = NULL;

	pin_cookie_t		cookie = NULL;
	pin_cookie_t		o_cookie = NULL;

	poid_t			*srch_pdp = NULL;
	poid_t			*plan_obj = NULL;

	pin_decimal_t		*c_balp = NULL;
	pin_decimal_t		*t_balp = NULL;
				
	char			*templatep = "select X from /balance_group where F1 = V1 " ;

	int32			srch_flag = 256;
	int32			ret_val = 0;
	int32			elemid = 0;
	int32			o_elemid = 0;

	int64			db = -1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return ret_val;
	PIN_ERR_CLEAR_ERR(ebufp);

	db = PIN_POID_GET_DB(acnt_pdp) ;
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, templatep, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
	result_flist = PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_BALANCES, wallet_res_id, ebufp );
	result_flist = PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_SUB_BALANCES, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_CURRENT_BAL, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_wallet_balance search input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_wallet_balance error", ebufp);
		ret_val = 1;
		goto CLEANUP;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_wallet_balance search output list", srch_oflistp);

	while ((result_flist = PIN_FLIST_ELEM_TAKE_NEXT(srch_oflistp, PIN_FLD_RESULTS,
				&o_elemid, 1, &o_cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		bal_flistp = PIN_FLIST_ELEM_TAKE(result_flist, PIN_FLD_BALANCES, wallet_res_id, 1, ebufp);
		if (bal_flistp)
		{
			/***********************************************************
			 * Initialise total balane variable only once to deal with
			 * multile balance groups
			 ***********************************************************/
			if (pbo_decimal_is_null(t_balp, ebufp))
			{
				t_balp = pbo_decimal_from_str("0.0", ebufp);
			}

			while ((arg_flist = PIN_FLIST_ELEM_TAKE_NEXT(bal_flistp, PIN_FLD_SUB_BALANCES,
				&elemid, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
			{
				c_balp = PIN_FLIST_FLD_GET(arg_flist, PIN_FLD_CURRENT_BAL, 0, ebufp);
				pbo_decimal_add_assign(t_balp, c_balp, ebufp);	

				PIN_FLIST_DESTROY_EX(&arg_flist, ebufp);
			}			
		
			PIN_FLIST_FLD_PUT(bal_flistp, PIN_FLD_CURRENT_BAL, t_balp, ebufp);	
			*r_flistpp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_ELEM_PUT(*r_flistpp, bal_flistp, PIN_FLD_BALANCES, wallet_res_id, ebufp);
		}
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_wallet_balance return list", *r_flistpp);
CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	return ret_val;
}


/**************************************************
* Function: 	fm_mso_get_bill_address_mod()
* Decription:   Returns the last modified date of billling address 
***************************************************/
void
fm_mso_get_bill_address_mod(
	pcm_context_t		*ctxp,
	poid_t			*act_pdp,
	pin_flist_t		**ret_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_flist_t		*result_flist = NULL;

	poid_t			*srch_pdp = NULL;
	int32			srch_flag = 512;
	int64			db = 1;

	char			*template = "select X from /payinfo where F1 = V1 and F2.type = V2 and F3 is not NULL " ;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_bill_address_mod", ebufp);
		return;
	}
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACCOUNT_OBJ, act_pdp, ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/payinfo/invoice", -1, ebufp), ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 3, ebufp );
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_MOD_T, NULL, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_MOD_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_CREATED_T, NULL, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bill_address_mod input flist", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bill_address_mod output flist", srch_out_flistp);	

	*ret_flist = PIN_FLIST_COPY(srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	return;
}

