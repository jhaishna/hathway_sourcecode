/*******************************************************************
 * File:	fm_mso_cust_allocate_hardware.c
 * Opcode:	MSO_OP_CUST_ALLOCATE_HARDWARE 
 * Owner:	Leela Pratyush Vasireddy
 * Created:	25-JUL-2013
 * Description: This opcode is used for allocation of Device to Plan
 * Modification History:
 * Modified By: Jyothirmayi Kachiraju
 * Date: 12-12-2019
 * Modification details - Added a new function to get the Carrier ID based on CMTS and City.
 *						  Validation for Carrier ID and Device Type for JIO, Hathway Networks.
 *						  Validation for DOCSIS3, DOCSIS2 and GPON Devices
 * 
Input Flist:
0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 2431536 0
0 PIN_FLD_ACCOUNT_OBJ    POID [0] 0.0.0.1 /account 2431536 0
0  MSO_FLD_ISTAL_FLAG    ENUM [0] 1
0 PIN_FLD_FLAGS           INT [0] 0
0     PIN_FLD_PROGRAM_NAME    STR [0] "Automatic Account creation"
0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/telco/broadband 2427945 0
0 MSO_FLD_HARDWARE_PLANS SUBSTRUCT [0] allocated 20, used 1
1     PIN_FLD_PLAN          ARRAY [0] allocated 20, used 4
2         PIN_FLD_PLAN_OBJ       POID [0] 0.0.0.1 /plan 2323909 2
2         MSO_FLD_PLAN_NAME            STR [0] "HARDWARE RENTAL - CABLE MODEM - PLAN"
2         PIN_FLD_ACTION                           STR [0] "ADD"
2         PIN_FLD_PRODUCTS              ARRAY [0] allocated 20, used 4
3               PIN_FLD_PRODUCT_OBJ             POID [0] 0.0.0.1 /product 2330362 2
3               PIN_FLD_PURCHASE_END_T          TSTAMP [0] (1415491200) Wed Apr 30 00:00:00 2014
3               PIN_FLD_PURCHASE_START_T        TSTAMP [0] (1412853267) Wed Mar 12 00:00:00 2014
3               MSO_FLD_DISC_AMOUNT                     DECIMAL [0] NULL
3               MSO_FLD_DISC_PERCENT            DECIMAL [0] 12
2         PIN_FLD_DEVICES       ARRAY [0] allocated 20, used 1
3             PIN_FLD_DEVICE_ID       STR [0] "19:59:33:5c:93:3b"
Output flist:
0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 2431536 0
0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/telco/broadband 2427945 0
0 PIN_FLD_STATUS         ENUM [0] 0
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_cust_allocate_hardware.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "ops/act.h"
#include "ops/cust.h"
#include "ops/device.h"
#include "ops/bill.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "mso_cust.h"
#include "mso_lifecycle_id.h"
#include "mso_device.h"
#include "mso_prov.h"
#include "mso_errs.h"

#define READY_TO_USE 1
#define REPAIRED_TO_USE 9
#define MSO_FLAG_SRCH_BY_ID 1
#define WHOLESALE "/profile/wholesale"
#define MSO_GOOD_STATE 1
#define MSO_ISTAL_FLAG 1
#define MSO_FLAG_SRCH_LCO_OF_SUBSCRIBER 8
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
fm_mso_get_device_info(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

extern void
fm_mso_get_profile_info(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

extern int
fm_mso_purchase_static_ip(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
	int32			flag,
        pin_errbuf_t            *ebufp);

extern
void fm_mso_create_lifecycle_evt(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             *out_flistp,
        int32                   action,
        pin_errbuf_t            *ebufp);
extern void 
fm_mso_search_devices(
        pcm_context_t                   *ctxp,
        char                            *param,
        poid_t                          *service_obj,
        pin_flist_t                     **deviceflistp,
        int32                           *error_code,
        pin_errbuf_t                    *ebufp);
extern void
fm_mso_purchase_bb_plans(
        pcm_context_t           *ctxp,
        pin_flist_t             *mso_act_flistp,
        char                    *prov_tag,
        int                     flags,
        int                     grace_flag,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

/**************************************
* External Functions end
***************************************/
/*******************************************************************
 * MSO_OP_CUST_ALLOCATE_HARDWARE 
 *
 *******************************************************************/

EXPORT_OP void
op_mso_cust_allocate_hardware(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_allocate_hardware(
	cm_nap_connection_t	*connp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static int
fm_mso_get_active_modem(
	pcm_context_t		*ctxp,
	poid_t			*ser_pdp,
	pin_errbuf_t		*ebufp);

int 
validate_device_type(
	pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             *device_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);
		
/* ************************************************************************
 * New function added for JIO-CPS Integration-ISP Enhancement
 * This function gets the flist with the data related to Network Element ID 
 * and the Network Provider from the mso_cfg_cmts_master object based 
 * on the CMTS ID and the City of the service for which the hardware 
 * is getting allocated.
 ************************************************************************/
static void
fm_mso_get_carrier_id(
	pcm_context_t		*ctxp,
	char				*cmts_id,
	char				*city,
	pin_flist_t         **r_flistpp,
	int					*error_codep,
	pin_errbuf_t		*ebufp);

PIN_EXPORT int
fm_mso_chk_device_warehouse(
        pcm_context_t           *ctxp,
        char                    *lco_code,
		char                    *cmts,
        char                    *city,
		pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

/*******************************************************************
 * MSO_OP_CUST_ALLOCATE_HARDWARE  
 *******************************************************************/
void 
op_mso_cust_allocate_hardware(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	*r_flistpp		= NULL;
	int			local = LOCAL_TRANS_OPEN_SUCCESS;
	int			*status = NULL;
	int32			status_uncommitted =2;
	poid_t			*inp_pdp = NULL;
	char            *prog_name = NULL;
	int32			failed_status = 1;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_ALLOCATE_HARDWARE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_allocate_hardware error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_allocate_hardware input flist", i_flistp);
	
	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
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
	fm_mso_cust_allocate_hardware(connp, flags, i_flistp, r_flistpp, ebufp);
	if (r_flistpp && *r_flistpp && *r_flistpp != (pin_flist_t *)NULL) {	
	status = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_STATUS, 1, ebufp);
	}

 	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp) || (*(int*)status == 1)) 
	{		
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_RESET(ebufp);
		}
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_allocate_hardware error", ebufp);
		if(local == LOCAL_TRANS_OPEN_SUCCESS )
		{
			fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
		}
		if(PIN_ERRBUF_IS_ERR(ebufp)) {
		    *r_flistpp = PIN_FLIST_CREATE(ebufp);
		     PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
		     PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failed_status, ebufp);
		     PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Hardware allocation could not be done", ebufp);
		}
	}
	else
	{
		if(local == LOCAL_TRANS_OPEN_SUCCESS  && 
		  !(prog_name && strcmp(prog_name,"pin_deferred_act") ==0)
		  )
		{
			fm_mso_trans_commit(ctxp, inp_pdp, ebufp);
		}
		else
		{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Transaction NOT Comitted");
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_uncommitted, ebufp);
		}

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_allocate_hardware output flist", *r_flistpp);
	}
	return;
}

/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_allocate_hardware(
	cm_nap_connection_t	*connp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	pin_flist_t		*hardware_flistp = NULL;
	pin_flist_t		*subscr_flistp = NULL;
	pin_flist_t		*inst_flistp = NULL;
	pin_flist_t		*plan_flistp = NULL;
	pin_flist_t		*products_flistp = NULL;
	pin_flist_t		*devices_flistp = NULL;
	pin_flist_t		*allocate_in_flistp = NULL;
	pin_flist_t		*allocate_out_flistp = NULL;
	pin_flist_t		*search_dev_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*out_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*mng_ret_flistp = NULL;
	pin_flist_t		*srch_profile_iflist = NULL;
	pin_flist_t		*srch_profile_oflist = NULL;
	pin_flist_t		*services_info = NULL;
	pin_flist_t		*new_flistp = NULL;
	pin_flist_t		*device_iflistp = NULL;
	pin_flist_t		*lc_in_flistp = NULL;
	pin_flist_t		*lc_plan_flistp = NULL;
	pin_flist_t		*lc_hardware_flistp = NULL;
	pin_flist_t		*read_flds_iflistp = NULL;
	pin_flist_t		*read_flds_oflistp = NULL;
	pin_flist_t		*dev_srch_iflistp = NULL;
	pin_flist_t		*devices_array = NULL;
	pin_flist_t		*dev_srch_oflistp = NULL;
	pin_flist_t		*dev_iflistp = NULL;
	pin_flist_t		*dev_oflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*dev_plan_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*mso_bb_flistp = NULL;
	pin_flist_t		*create_prov_iflist = NULL;
	pin_flist_t		*create_prov_oflist = NULL;
	pin_flist_t		*rfld_iflistp = NULL;
	pin_flist_t		*rfld_oflistp = NULL;
	pin_flist_t		*temp_flistp = NULL;
	//pin_flist_t		*arg_flist = NULL;

	poid_t			*device_obj = NULL;
	poid_t			*account_obj = NULL;
	poid_t			*service_obj = NULL;
	poid_t			*plan_obj = NULL;
	poid_t			*product_obj = NULL;
	poid_t			*device_lco_pdp = NULL;
	poid_t			*profile_lco_pdp = NULL;
	poid_t			*routing_poid = NULL;
	poid_t			*mso_bb_bp_obj = NULL;
	poid_t			*mso_bb_obj = NULL;
	
	char			*error_descr = NULL;
	char			*device_id = NULL;
	char			*device_type = NULL;
	char			*mac_addr = NULL;
	char			*ip_addr = NULL;
	char			*device_ip_type = "/device/ip/static";

	int				device_state = -1;
	int				plan_is_null = -1;
	int				plan_cnt = 0;
	int				non_ip_flag = 0;
	int				device_arr_count = 0;
	int				ip_found = 0;
	int				ret_val = 0;

	int32			counter   = 0;
	int32			*istalf   = NULL;
	int32			elem_id1 = 0;
	int32			elem_id2 = 0;
	int32			elem_id3 = 0;
	int32			elem_id4 = 0;
	int32			flag	 = 1 ;
	int32			*in_flags	 = NULL ;
	int32			*ret_status	 = NULL ;
	int32			istal_result = 0 ;
	int32			set_err	 = 0 ;
	int32			allocate_failure = 1;
	int32			profile_srch_type = MSO_FLAG_SRCH_LCO_OF_SUBSCRIBER;
	int32			device_search_type = MSO_FLAG_SRCH_BY_ID;	
	int32			res_val = -1;
	int32                   prov_flgs = 215;
	pin_cookie_t		cookie1 = NULL;
	pin_cookie_t		cookie2 = NULL;
	pin_cookie_t		cookie3 = NULL;
	pin_cookie_t		cookie4 = NULL;
	pin_cookie_t		prev_cookie = NULL;
	
	int64			db = 1;
	char			*template = "select X from /device where F1 = V1 ";
	char			*p_template = "select X from /mso_purchased_plan where F1 = V1 and plan_type='base plan' ";
	char			*add_action = "ADD";
	int32			srch_flag = 0;
	int32			hw_plan_type = -1;
	pin_decimal_t		*priority = NULL;
	double			prod_priority_double = 0;
	poid_t			*srch_pdp = NULL;

	void			*vp = NULL;
	int32			is_valid_device_type = 0;
	int32			is_valid_warehouse_type = 0;
	int32			prov_status = 1;
	int			plan_type=-1;
	int			modem_found=0;
	pin_flist_t		*deviceflistp = NULL;
	int			*error_codep = NULL;
	int			*tech = NULL;
	int			*prov_call = NULL;

	char                            param[50];
	char                            tmp[500]="";	
	
	// Declaring the new variables for JIO-CPS Integration-ISP Enhancement
	char			*city = NULL;
	char			*lco_code = NULL;
	char			*cmts_id = NULL;
	char 			*other_ne_id = NULL;
	char			*alloc_device_type_str = NULL;
	int32			*carrier_id	 = NULL;	
	int32			validation_failure = 1;
	int32			alloc_device_type = 0;
	
	pin_flist_t		*jiomso_bb_flistp = NULL;
	pin_flist_t		*readsvc_flds_iflistp = NULL;
	pin_flist_t		*readsvc_flds_oflistp = NULL;
	pin_flist_t		*ne_prov_res_flistp = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_allocate_hardware input flist", i_flistp);	
	lc_in_flistp = PIN_FLIST_COPY(i_flistp, ebufp);

	routing_poid = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
	account_obj = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	service_obj = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	in_flags = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp);
	if ( !routing_poid || !account_obj || !service_obj || !in_flags )
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &allocate_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51400", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Mandatory fields are not passed in input flist", ebufp);
		*r_flistpp = ret_flistp;
		return;		
	}
	hardware_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, MSO_FLD_HARDWARE_PLANS, 1, ebufp);
	if ( !hardware_flistp )
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &allocate_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51611", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Hardware Devices information must be sent in input flist for allocate hardware", ebufp);
		*r_flistpp = ret_flistp;
		return;	
	}
	
	//Read the ISTAL Flag and add it in input flist.
	read_flds_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, read_flds_iflistp, PIN_FLD_POID, ebufp);
	mso_bb_flistp = PIN_FLIST_ELEM_ADD(read_flds_iflistp, MSO_FLD_BB_INFO, 0, ebufp);
	PIN_FLIST_FLD_SET(mso_bb_flistp, MSO_FLD_ISTAL_FLAG, NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read bb info Input flist", read_flds_iflistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flds_iflistp, &read_flds_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read bb info output flist", read_flds_oflistp);
	mso_bb_flistp = PIN_FLIST_ELEM_GET(read_flds_oflistp, MSO_FLD_BB_INFO, 0, 0, ebufp);
	PIN_FLIST_FLD_COPY(mso_bb_flistp, MSO_FLD_ISTAL_FLAG, i_flistp, MSO_FLD_ISTAL_FLAG, ebufp);
	PIN_FLIST_DESTROY_EX(&read_flds_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&read_flds_oflistp, NULL);

	lc_hardware_flistp = PIN_FLIST_SUBSTR_GET(lc_in_flistp, MSO_FLD_HARDWARE_PLANS, 1, ebufp);

	out_flistp = PIN_FLIST_CREATE(ebufp);
        res_flistp = PIN_FLIST_ELEM_ADD(out_flistp, PIN_FLD_RESULTS, 0, ebufp);
	while ((plan_flistp = PIN_FLIST_ELEM_GET_NEXT(hardware_flistp, PIN_FLD_PLAN, &elem_id1, 1, &cookie1, ebufp )) != NULL)
        {
		lc_plan_flistp = PIN_FLIST_ELEM_GET(lc_hardware_flistp, PIN_FLD_PLAN, elem_id1, 1, ebufp );
		plan_cnt++;
		plan_is_null = 0;
		/* 08-APR-15: Add the Action flag since it is removed from input flist as per  
			changes in input flist (Alongwith PRODUCTS array which is also removed) */
		PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_ACTION, add_action, ebufp );
		plan_obj = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);		
		if ( !plan_obj )
		{
			//set_err = 1;
			//goto FINAL;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "PLAN_OBJ is NULL");
			plan_is_null = 1;
		}
		/*Products array is removed so below validation is not required*/
		/*cookie2 = NULL;
		while ((products_flistp = PIN_FLIST_ELEM_GET_NEXT(plan_flistp, PIN_FLD_PRODUCTS, &elem_id2, 1, &cookie2, ebufp )) != NULL)		
		{
			product_obj = PIN_FLIST_FLD_GET(products_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);		
			if ( !product_obj || plan_is_null==1)
			{
				set_err = 1;
				goto FINAL;
			}
		}*/
		vp = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_FLAGS, 1, ebufp);
		if(vp) {
			plan_type=*(int *)vp;
		} else {
			plan_type=-1;
		}

		cookie3 = NULL;
		while ((devices_flistp = PIN_FLIST_ELEM_GET_NEXT(plan_flistp, PIN_FLD_DEVICES, &elem_id3, 1, &cookie3, ebufp )) != NULL)		
		{
			device_id = PIN_FLIST_FLD_GET(devices_flistp, PIN_FLD_DEVICE_ID, 1, ebufp);
			if ( !device_id || device_id == (char *)NULL )
			{
				set_err = 1;
				goto FINAL;
			}
			/*******************************************************************
			* Get Device info from Device Id
			*******************************************************************/
			search_dev_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(search_dev_flistp, PIN_FLD_POID, account_obj, ebufp );
			PIN_FLIST_FLD_SET(search_dev_flistp, PIN_FLD_DEVICE_ID, device_id, ebufp );
			PIN_FLIST_FLD_SET(search_dev_flistp, PIN_FLD_SEARCH_TYPE, &device_search_type, ebufp );

			fm_mso_get_device_info(ctxp, search_dev_flistp, &result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&search_dev_flistp, NULL);
			if (!result_flistp)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Device details not present in BRM");
				PIN_ERRBUF_RESET(ebufp);
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, account_obj, ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &allocate_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51090", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Device details not present in BRM", ebufp);
				PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
				*r_flistpp = ret_flistp;
				return;		
            }
            
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_allocate_hardware: result_flistp", result_flistp);

			is_valid_device_type = validate_device_type(ctxp, i_flistp, result_flistp, r_flistpp, ebufp);
			if(is_valid_device_type == PIN_BOOLEAN_FALSE )
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Validation for device type failed. returning.. ");
				return;
			}
			

			PIN_FLIST_ELEM_DROP(lc_plan_flistp, PIN_FLD_DEVICES, elem_id3, ebufp);
			vp = (pin_flist_t*)PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_ELEM_PUT(lc_plan_flistp, vp, PIN_FLD_DEVICES, elem_id3, ebufp);
		
			/*******************************************************************
			* Get Device info from Device Id
			*******************************************************************/
			// JIO CPS Enhancement
			alloc_device_type_str = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_DEVICE_TYPE, 1, ebufp);
			alloc_device_type = atoi(alloc_device_type_str);

			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Device_Type **** ");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, alloc_device_type_str);
			
			device_obj = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_POID, 0, ebufp);
			device_type = (char *)PIN_POID_GET_TYPE(device_obj);
			//Pawan: Added below validation to match the device tpye with 
			//device type value passed in input to avoid associating device
			// to wrong plan type.
			// 1=MODEM; 2=CABLE ROUTER; 3=WIFI ROUTER
			// 4=NAT;   5=STATIC IP;    6=FRAME IP;
			if(	(strstr(device_type,"/device/modem") && plan_type!=MSO_DVC_TYPE_MODEM )||
				(strstr(device_type,"/device/router/cable") && plan_type!=MSO_DVC_TYPE_ROUTER_CABLE )||
				(strstr(device_type,"/device/router/wifi") && plan_type!=MSO_DVC_TYPE_ROUTER_WIFI )||
				(strstr(device_type,"/device/nat") && plan_type!=MSO_DVC_TYPE_NAT )||
				(strstr(device_type,"/device/ip/static") && plan_type!=MSO_DVC_TYPE_IP_STATIC )||
				(strstr(device_type,"/device/ip/framed") && plan_type!=MSO_DVC_TYPE_IP_FRAMED )   )
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Device does not match device type mentioned in input");
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, account_obj, ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &allocate_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51099", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Device does not match device type mentioned in input", ebufp);
				PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
				*r_flistpp = ret_flistp;
				return;	
			}
			/***adding condition to have only one modem for an acccount*******/
			if(plan_type == MSO_DVC_TYPE_MODEM)
            {
				strcpy(param,"/device/modem");
				fm_mso_search_devices(ctxp, param, service_obj, &deviceflistp, error_codep, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					ret_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &allocate_failure, ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error after searching the device modem", ebufp);
					PIN_FLIST_DESTROY_EX(&deviceflistp,NULL);
                    return;
                }
				
				if(PIN_FLIST_ELEM_COUNT(deviceflistp, PIN_FLD_RESULTS, ebufp))
				{
					ret_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &allocate_failure, ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Already modem  is allocated for this customer", ebufp);
					*r_flistpp = ret_flistp;
					PIN_FLIST_DESTROY_EX(&deviceflistp,NULL);
                    return;
                }
            }
			//Set modem flag of device type is modem to validate for add_device scenario.
			if(strstr(device_type,"/device/modem"))
				modem_found=1;			

			device_state = *(int *)PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_STATE_ID, 0, ebufp);
			if ( !(device_state == READY_TO_USE || device_state == REPAIRED_TO_USE))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Device state not READY_TO_USE");
				PIN_ERRBUF_RESET(ebufp);
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, account_obj, ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &allocate_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51091", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Device state not READY_TO_USE/PREACTIVATED", ebufp);
				PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
				*r_flistpp = ret_flistp;
				return;		
			}
			//non_ip_flag will be ZERO for IP static and Non-Zero for others.
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "test1 ");
			sprintf(tmp,"%s %s",device_type,device_ip_type);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, tmp);
			non_ip_flag = strcmp(device_type,device_ip_type);
			if(non_ip_flag==0)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "test ");
				ip_found=0;
			}

			lco_code = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_SOURCE, 0, ebufp);
		
			/* Apply Device and Account LCO validation for all devices EXCEPT:					
			*	1) customer owned device 
			*	2) and for Static IP  */
			/* Pawan:23-01-2015: LCO validation disabled for all devices. Added 0 zero condition below */
			if(plan_is_null==0 && non_ip_flag!=0 && 1)
			{
				device_lco_pdp = PIN_FLIST_FLD_TAKE(result_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
				srch_profile_iflist = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(srch_profile_iflist, PIN_FLD_POID, account_obj, ebufp );
				PIN_FLIST_FLD_SET(srch_profile_iflist, PIN_FLD_PROFILE_DESCR, WHOLESALE, ebufp );
				PIN_FLIST_FLD_SET(srch_profile_iflist, PIN_FLD_SEARCH_TYPE, &profile_srch_type, ebufp );
				PIN_FLIST_FLD_SET(srch_profile_iflist, PIN_FLD_OBJECT, account_obj, ebufp );

				fm_mso_get_profile_info(ctxp, srch_profile_iflist, &srch_profile_oflist, ebufp );
				PIN_FLIST_DESTROY_EX(&srch_profile_iflist, NULL);
				if (!srch_profile_oflist)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Wholesale LCO Profile details not present in BRM");
					PIN_ERRBUF_RESET(ebufp);
					ret_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, account_obj, ebufp );
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &allocate_failure, ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51097", ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Wholesale LCO Profile details not present in BRM", ebufp);
					PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
					PIN_FLIST_DESTROY_EX(&srch_profile_oflist, NULL);
					*r_flistpp = ret_flistp;
					return;		
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_allocate_hardware: srch_profile_oflist", srch_profile_oflist);
				profile_lco_pdp = PIN_FLIST_FLD_GET(srch_profile_oflist, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
				if ( PIN_POID_COMPARE(device_lco_pdp, profile_lco_pdp, 0, ebufp) != 0 && 0 )
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Registered LCO not matching Device LCO");
					PIN_ERRBUF_RESET(ebufp);
					ret_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, account_obj, ebufp );
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &allocate_failure, ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51098", ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Registered LCO not matching Device LCO", ebufp);
					PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
					PIN_FLIST_DESTROY_EX(&srch_profile_oflist, NULL);
					*r_flistpp = ret_flistp;
					return;
				}
			}
			
			// JIO CPS Enhancement === Added below code to read the BB_INFO field from the Service Object.
			readsvc_flds_iflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(readsvc_flds_iflistp, PIN_FLD_POID, (void *)service_obj, ebufp);
			
			jiomso_bb_flistp = PIN_FLIST_SUBSTR_ADD(readsvc_flds_iflistp, MSO_FLD_BB_INFO, ebufp);
			PIN_FLIST_FLD_SET(jiomso_bb_flistp, MSO_FLD_NETWORK_ELEMENT, NULL, ebufp);
			PIN_FLIST_FLD_SET(jiomso_bb_flistp, PIN_FLD_CITY, NULL, ebufp);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"fm_mso_cust_allocate_hardware JIO: Read Service In flist", readsvc_flds_iflistp);
			PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, readsvc_flds_iflistp, &readsvc_flds_oflistp, ebufp);
			PIN_FLIST_DESTROY_EX(&readsvc_flds_iflistp, NULL);
			
			if (PIN_ERRBUF_IS_ERR(ebufp)) 
			{
				*error_codep = MSO_BB_INTERNAL_ERROR;
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						"fm_mso_cust_allocate_hardware read flds error: Error", ebufp);
				PIN_FLIST_DESTROY_EX(&readsvc_flds_oflistp, NULL);
			}
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"fm_mso_cust_allocate_hardware JIO: Read Service Out flist", readsvc_flds_oflistp);
					
			jiomso_bb_flistp = PIN_FLIST_SUBSTR_GET(readsvc_flds_oflistp, MSO_FLD_BB_INFO, 0, ebufp);		
			
			if (!jiomso_bb_flistp)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"BB_INFO details not present in BRM for the service.");
				PIN_ERRBUF_RESET(ebufp);
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, account_obj, ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &allocate_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51090", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "BB_INFO details not present in BRM", ebufp);
				PIN_FLIST_DESTROY_EX(&readsvc_flds_oflistp, NULL);
				*r_flistpp = ret_flistp;
				return;		
			}
			// Get the CMTS ID and City from the BB_INFO field.
			cmts_id = (char *)PIN_FLIST_FLD_GET(jiomso_bb_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
			city = (char *)PIN_FLIST_FLD_GET(jiomso_bb_flistp, PIN_FLD_CITY, 0, ebufp);

			is_valid_warehouse_type = fm_mso_chk_device_warehouse(ctxp, lco_code, cmts_id, city, r_flistpp, ebufp);

			if(is_valid_warehouse_type == 0 )
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Validation for device source failed. returning.. ");
				return;
			}


			PIN_FLIST_DESTROY_EX(&readsvc_flds_oflistp, NULL);					
			
			/* *********************************************************************
			 * Get the Network Element ID and Network Provider based on CMTS ID 
			 * and City of the Service for which the hardware is getting allocated.
			 * *********************************************************************/						 
			fm_mso_get_carrier_id(ctxp, cmts_id, city, &ne_prov_res_flistp, error_codep, ebufp);
				
			if (!ne_prov_res_flistp)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"CMTS details not present in BRM");
				PIN_ERRBUF_RESET(ebufp);
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, account_obj, ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &allocate_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51090", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "CMTS details not present in BRM", ebufp);
				PIN_FLIST_DESTROY_EX(&ne_prov_res_flistp, NULL);
				*r_flistpp = ret_flistp;
				return;		
			}
							
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"After fm_mso_get_carrier_id call. Printing ne_prov_res_flistp >> ");

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"fm_mso_cust_allocate_hardware JIO: Read Service Out flist", ne_prov_res_flistp);
					
			/* **************************************************************************************
			 *	0 MSO_FLD_OTHER_NE_ID     STR [0] "JCPS01"
			 *  0 PIN_FLD_CARRIER_ID      INT [0] 1
			 *	0 PIN_FLD_POID           POID [0] 0.0.0.1 /mso_cfg_cmts_master 23485176016 1
			 ****************************************************************************************/
				
			other_ne_id = (char *)PIN_FLIST_FLD_GET(ne_prov_res_flistp, MSO_FLD_OTHER_NE_ID, 0, ebufp);	
			carrier_id = PIN_FLIST_FLD_GET(ne_prov_res_flistp, PIN_FLD_CARRIER_ID, 0, ebufp);
							
			/* ****************************************
			 * JIO_NETWORK_PROVIDER = 1
			 * JIO_NETWORK_ELEMENT_ID = "JCPS01"
			 * HW_NETWORK_PROVIDER = 0
			 * HW_NETWORK_ELEMENT_ID = "CPS01"
			 * JIO_GPON_MODEM = "35"
			 ******************************************/
			if (carrier_id != NULL && other_ne_id != NULL && alloc_device_type_str != NULL)
			{
				/* *********************************************************************************
				 * If Account belongs to JIO, then only 35, 11, 21 and 31 device types are allowed.
				 * Otherwise throw error for device type and CMTS mismatch.
				 * If Account belongs to Hathway, then allow all device types other than 35.
				 * Otherwise throw error for device type and CMTS mismatch.
				 ***********************************************************************************/
				if ( *(int32 *)carrier_id == JIO_NETWORK_PROVIDER && 
					    strcmp(other_ne_id, JIO_NETWORK_ELEMENT_ID) == 0 && 
					  ( strcmp(alloc_device_type_str, JIO_GPON_MODEM) != 0  ||
					    strcmp(alloc_device_type_str, HW_ONU_GPON_MODEM) != 0 || 
						alloc_device_type != DOCSIS2_WIFI_MODEM || 
						alloc_device_type != DOCSIS3_WIFI_MODEM ) ||
					 ( *(int32 *)carrier_id == HW_NETWORK_PROVIDER && 
					   strcmp(other_ne_id, HW_NETWORK_ELEMENT_ID) == 0 && 
					   strcmp(alloc_device_type_str, JIO_GPON_MODEM) == 0 ) )
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Invalid Device type for the CMTS associated with the service.");

					ret_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &validation_failure, ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51401", ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Invalid Device type: CMTS and Device mismatch", ebufp);
					*r_flistpp = ret_flistp;
					PIN_FLIST_DESTROY_EX(&ne_prov_res_flistp, NULL);
					return;
				}
			}				
				
			PIN_FLIST_DESTROY_EX(&ne_prov_res_flistp, NULL);
			
			allocate_in_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET( allocate_in_flistp, PIN_FLD_POID, device_obj, ebufp );
			PIN_FLIST_FLD_SET( allocate_in_flistp, PIN_FLD_FLAGS, &flag, ebufp);
			PIN_FLIST_FLD_COPY( i_flistp, PIN_FLD_PROGRAM_NAME, allocate_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
			services_info = PIN_FLIST_ELEM_ADD(allocate_in_flistp, PIN_FLD_SERVICES, 0, ebufp);
			PIN_FLIST_FLD_SET(services_info,PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);	
			PIN_FLIST_FLD_SET(services_info,PIN_FLD_SERVICE_OBJ, service_obj, ebufp);	
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_allocate_hardware: Input flist", allocate_in_flistp);
			PCM_OP(ctxp, PCM_OP_DEVICE_ASSOCIATE, 0, allocate_in_flistp, &allocate_out_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&allocate_in_flistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_allocate_hardware:output flist:", allocate_out_flistp);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in associating Device", ebufp);

			   if(((ebufp)->pin_err == PIN_ERR_IM_CONNECT_FAILED) || (ebufp)->pin_err == PIN_ERR_DM_CONNECT_FAILED ||
				(ebufp)->rec_id == 9999 )
				{
					PIN_ERRBUF_RESET(ebufp);
					ret_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR,"Device is not assigned to Locator .First assign the device to Locator from Ware house before allocating.",ebufp);
				}
				else
				{
					PIN_ERRBUF_RESET(ebufp);
					ret_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in associating Device to Service", ebufp);
				}
				
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, account_obj, ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &allocate_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51612", ebufp);

				PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&srch_profile_oflist, NULL);
				PIN_FLIST_DESTROY_EX(&allocate_out_flistp, NULL);

				*r_flistpp = ret_flistp;
                return;
            }
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_allocate_hardware: output flist:", allocate_out_flistp);
			PIN_FLIST_CONCAT(res_flistp, allocate_out_flistp, ebufp);
			//Added to update the device type
			if ( *(int32 *)in_flags == MSO_ALLOCATE_HARDWARE)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Updating device type.");
				dev_iflistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(allocate_out_flistp, PIN_FLD_POID, dev_iflistp, PIN_FLD_POID, ebufp);
				tmp_flistp = PIN_FLIST_ELEM_ADD(dev_iflistp, PIN_FLD_PLAN, 0, ebufp);
				vp = PIN_FLIST_FLD_GET(devices_flistp, PIN_FLD_TYPE, 1, ebufp);
				if(vp) {
					PIN_FLIST_FLD_COPY(devices_flistp, PIN_FLD_TYPE, tmp_flistp, PIN_FLD_TYPE, ebufp);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Device write flds input flist:", dev_iflistp);
					PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 32, dev_iflistp, &dev_oflistp, ebufp);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Device write flds output flist:", dev_oflistp);
				}
			}
			PIN_FLIST_DESTROY_EX(&dev_iflistp, NULL);
			PIN_FLIST_DESTROY_EX(&dev_oflistp, NULL);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&srch_profile_oflist, NULL);
			PIN_FLIST_DESTROY_EX(&allocate_out_flistp, NULL);
			continue;	
		}

		PIN_FLIST_FLD_DROP(plan_flistp, PIN_FLD_FLAGS, ebufp);
		if ( *(int32 *)in_flags == MSO_ADD_DEVICE)
		{
			//fm_mso_purchase_bb_plans ( ctxp, plan_flistp, (char *)NULL, 1, &mng_ret_flistp, ebufp);
		    //If Device type is modem then perform validation.
		    if(modem_found)
		    {
			    res_val = fm_mso_get_active_modem( ctxp, service_obj, ebufp);
			    if(res_val != 0) {
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &allocate_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51415", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Service already have an active modem.", ebufp);
				*r_flistpp = ret_flistp;
				return;
			    }
		    }
		    dev_iflistp = PIN_FLIST_CREATE(ebufp);
		    if(plan_is_null==0 ) 
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Add Device scenario.");
				device_iflistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_ELEM_COPY(hardware_flistp, PIN_FLD_PLAN, elem_id1, device_iflistp, PIN_FLD_PLAN, 0, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, device_iflistp, PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, device_iflistp, PIN_FLD_PROGRAM_NAME, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, device_iflistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, device_iflistp, PIN_FLD_SERVICE_OBJ, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_purchase_bb_plans input flist", device_iflistp);
				fm_mso_purchase_bb_plans ( connp, device_iflistp, (char *)NULL, 1, 0,  &mng_ret_flistp, ebufp);
				//fm_mso_purchase_bb_plans ( ctxp, device_iflistp, (char *)NULL, 1, 0,  &mng_ret_flistp, ebufp);
				if ( mng_ret_flistp && mng_ret_flistp != (pin_flist_t *)NULL )
				{
					ret_status = PIN_FLIST_FLD_GET(mng_ret_flistp, PIN_FLD_STATUS, 1, ebufp);
				}
				if ( PIN_ERRBUF_IS_ERR(ebufp) || ((ret_status) && *(int32 *)ret_status == 0) )
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in purchasing plan ", ebufp);
					*r_flistpp = mng_ret_flistp;
					PIN_FLIST_DESTROY_EX(&device_iflistp, NULL);
					return ;
				}

				hw_plan_type = 0;
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_purchase_bb_plans output flist", mng_ret_flistp);
				
				read_flds_iflistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(mng_ret_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, read_flds_iflistp, PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_SET(read_flds_iflistp, PIN_FLD_PRIORITY, NULL, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " mso_purchased_plan read flds input flist:", read_flds_iflistp);
				PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flds_iflistp, &read_flds_oflistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " mso_purchased_plan read flds output flist:", read_flds_oflistp);
				PIN_FLIST_DESTROY_EX(&read_flds_iflistp, NULL);

				priority = PIN_FLIST_FLD_GET(read_flds_oflistp, PIN_FLD_PRIORITY, 0, ebufp);
				prod_priority_double = pbo_decimal_to_double(priority, ebufp); 
				if((prod_priority_double >= BB_HW_RENTAL_RANGE_START && prod_priority_double <= BB_HW_RENTAL_RANGE_END) || 
					(prod_priority_double >= BB_HW_DEPOSIT_RANGE_START && prod_priority_double <= BB_HW_DEPOSIT_RANGE_END)) 
				{
					hw_plan_type = RENTAL_DEPOSIT_PLAN;
				} 
				else if(prod_priority_double >= BB_HW_OUTRIGHT_RANGE_START && prod_priority_double <= BB_HW_OUTRIGHT_RANGE_END) 
				{
					hw_plan_type = OUTRIGHT_PLAN;
				}
				plan_flistp = PIN_FLIST_ELEM_ADD(dev_iflistp, PIN_FLD_PLAN, 0, ebufp);
				PIN_FLIST_FLD_COPY(mng_ret_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, plan_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, ebufp);
				PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_TYPE, &hw_plan_type, ebufp)
			} 
			else 
			{
				hw_plan_type = CUSTOMER_DEVICE;
				plan_flistp = PIN_FLIST_ELEM_ADD(dev_iflistp, PIN_FLD_PLAN, 0, ebufp);
				PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_TYPE, &hw_plan_type, ebufp);
		    }
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Searching Device.");
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Device flist:", dev_iflistp);
		    //dev_plan_flistp = PIN_FLIST_ELEM_GET(device_iflistp,PIN_FLD_PLAN, 0, 0, ebufp); 

		    PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_POID, dev_iflistp, PIN_FLD_POID, ebufp);

		    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " device write flds input flist:", dev_iflistp);
			PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 32, dev_iflistp, &dev_oflistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " device write flds output flist:", dev_oflistp);
			/****************************************************************************************
			* Provisiong action for adding multiple IPs for Fiber and /static/ip
			****************************************************************************************/
			// First read the Technology as this is for only Fiber 
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "device type");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, device_type);
			if (strcmp(device_type, "/device/ip/static") == 0)
			{
				rfld_iflistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(rfld_iflistp, PIN_FLD_POID, (void *)service_obj, ebufp);
				//Add the BB Info SUBSTR
				temp_flistp = PIN_FLIST_SUBSTR_ADD(rfld_iflistp, MSO_FLD_BB_INFO, ebufp);
				PIN_FLIST_FLD_SET(temp_flistp, MSO_FLD_TECHNOLOGY, (void *)0, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
						"fm_mso_cust_allocate_hardware prov: Read Service In flist", rfld_iflistp);
				PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, rfld_iflistp, &rfld_oflistp, ebufp);
				PIN_FLIST_DESTROY_EX(&rfld_iflistp, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp)) 
				{
					*error_codep = MSO_BB_INTERNAL_ERROR;
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
							"fm_mso_cust_allocate_hardware read flds error: Error", ebufp);
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
						"fm_mso_cust_allocate_hardware PROV: Read Service Out flist", rfld_oflistp);
				temp_flistp = PIN_FLIST_SUBSTR_GET(rfld_oflistp, MSO_FLD_BB_INFO, 0, ebufp);
				tech = (int *)PIN_FLIST_FLD_GET(temp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp);
			
				if( tech != NULL && (*tech == FIBER || *tech == GPON || *tech == ETHERNET)){
					create_prov_iflist = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, create_prov_iflist, PIN_FLD_POID, ebufp);
					PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, create_prov_iflist, PIN_FLD_SERVICE_OBJ, ebufp);
					PIN_FLIST_FLD_SET(create_prov_iflist, PIN_FLD_FLAGS , &prov_flgs, ebufp);
					PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_PROGRAM_NAME,create_prov_iflist,PIN_FLD_PROGRAM_NAME,ebufp);
					fm_mso_cust_get_bp_obj( ctxp, account_obj, service_obj, -1, &mso_bb_bp_obj, &mso_bb_obj, ebufp);
					PIN_FLIST_FLD_SET(create_prov_iflist, MSO_FLD_PURCHASED_PLAN_OBJ, mso_bb_obj, ebufp);
					PIN_FLIST_ELEM_COPY(hardware_flistp, PIN_FLD_PLAN, elem_id1, create_prov_iflist, PIN_FLD_PLAN, 0, ebufp);
				
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "create_prov_iflist", create_prov_iflist);
					PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION, 0, create_prov_iflist, &create_prov_oflist, ebufp);
					if (PIN_ERRBUF_IS_ERR(ebufp))
					{
			        		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "create_prov error",ebufp);
			        		PIN_ERRBUF_RESET(ebufp);
			        		ret_flistp = PIN_FLIST_CREATE(ebufp);
			        		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID,
                						PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			        		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &prov_status, ebufp);
			       	 		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51000", ebufp);
			        		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR,
			                			"Error Calling MSO_OP_PROV_CREATE_ACTION", ebufp);
						*r_flistpp = ret_flistp;
						PIN_FLIST_DESTROY_EX(&create_prov_iflist, NULL);
						PIN_FLIST_DESTROY_EX(&rfld_oflistp, NULL);
						return;
					}	
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			        			"create_prov_action input list output flist", create_prov_oflist);
					if(create_prov_oflist){
						prov_call = PIN_FLIST_FLD_GET(create_prov_oflist, PIN_FLD_STATUS, 1, ebufp);
						if (prov_call != NULL && *prov_call == 1){
							*r_flistpp = create_prov_oflist;
							PIN_FLIST_DESTROY_EX(&create_prov_oflist, NULL);
							PIN_FLIST_DESTROY_EX(&rfld_oflistp, NULL);
							PIN_FLIST_DESTROY_EX(&create_prov_iflist, NULL);
							return;
						}
					}	
					else{
			        		ret_flistp = PIN_FLIST_CREATE(ebufp);
			        		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID,
                						PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			        		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &prov_status, ebufp);
			       	 		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51751", ebufp);
			        		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR,
								"Error Calling MSO_OP_PROV_CREATE_ACTION", ebufp);
						*r_flistpp = ret_flistp;
						PIN_FLIST_DESTROY_EX(&rfld_oflistp, NULL);
						PIN_FLIST_DESTROY_EX(&create_prov_iflist, NULL);
						return;
					}
					// Added below to update the status of service to in-progress
					ret_val = fm_mso_update_ser_prov_status(ctxp, create_prov_iflist, MSO_PROV_IN_PROGRESS, ebufp );
					if(ret_val)
					{
						ret_flistp = PIN_FLIST_CREATE(ebufp);
			        		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID,
                						PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
						PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &prov_status, ebufp);
						PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51767", ebufp);
						PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error while updating service provisioning status.", ebufp);
						*r_flistpp = ret_flistp;
						PIN_FLIST_DESTROY_EX(&create_prov_oflist, NULL);
						PIN_FLIST_DESTROY_EX(&create_prov_iflist, NULL);
						PIN_FLIST_DESTROY_EX(&rfld_oflistp, NULL);
						return;
					}	
				
					// Added below to update the status of mso_pur_plan to in-progress
					if(fm_mso_update_mso_purplan_status(ctxp,mso_bb_obj, MSO_PROV_IN_PROGRESS, ebufp ) == 0)
					{
						ret_flistp = PIN_FLIST_CREATE(ebufp);
			        		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID,
                						PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
						PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &prov_status, ebufp);
						PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51754", ebufp);
						PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error while updating mso_purhcased_plan status.", ebufp);
						*r_flistpp = ret_flistp;
						PIN_FLIST_DESTROY_EX(&create_prov_oflist, NULL);
						PIN_FLIST_DESTROY_EX(&create_prov_iflist, NULL);
						PIN_FLIST_DESTROY_EX(&rfld_oflistp, NULL);
						return;
					}
					PIN_FLIST_DESTROY_EX(&create_prov_oflist, NULL);
					PIN_FLIST_DESTROY_EX(&create_prov_iflist, NULL);
					PIN_FLIST_DESTROY_EX(&rfld_oflistp, NULL);
				}
			}

		    //PIN_FLIST_DESTROY_EX(&device_iflistp,NULL);
		    PIN_FLIST_DESTROY_EX(&read_flds_oflistp, NULL);
		    //PIN_FLIST_DESTROY_EX(&dev_srch_iflistp,NULL);
		    //PIN_FLIST_DESTROY_EX(&dev_srch_oflistp, NULL);
		    PIN_FLIST_DESTROY_EX(&dev_iflistp,NULL);
		    PIN_FLIST_DESTROY_EX(&dev_oflistp, NULL);
		}
	}
	/* Alocate Static IP if ISTAL flag is true and IP is not passed in input */
	istalf = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_ISTAL_FLAG, 1, ebufp);
	if (( istalf && *(int32 *)istalf == MSO_ISTAL_FLAG ) && ip_found)
	{
		istal_result = fm_mso_purchase_static_ip(ctxp, i_flistp, *in_flags, ebufp );
		if ( istal_result == 1 )
		{
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &allocate_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51400", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Mandatory POID values not passed in input flist", ebufp);
			*r_flistpp = ret_flistp;
			return;		
		}
		else if(istal_result == 2)
		{
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &allocate_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51421", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Static IP not found for Specified CMTS Id and Plan", ebufp);
			*r_flistpp = ret_flistp;
			return;	
		}
		else if ( istal_result == 0 )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Static IP device associated successfully");
		}

	}	
	
	if ( *(int32 *)in_flags == MSO_ALLOCATE_HARDWARE)
	{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Allocate Hardware scenario.");
			tmp_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
			hardware_flistp = PIN_FLIST_SUBSTR_GET(tmp_flistp, MSO_FLD_HARDWARE_PLANS, 1, ebufp);
			cookie3 = NULL;
			while ((plan_flistp = PIN_FLIST_ELEM_GET_NEXT(hardware_flistp, PIN_FLD_PLAN, &elem_id3, 1, 
						&cookie3, ebufp )) != NULL)
			{
			
				plan_obj = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);		
				if ( !plan_obj )
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "PLAN_OBJ is NULL.. Dropping");
					PIN_FLIST_ELEM_DROP(hardware_flistp, PIN_FLD_PLAN, elem_id3, ebufp);
					cookie3 = prev_cookie;
				}
				else
				{
					cookie4 = NULL;
					while ((dev_iflistp = PIN_FLIST_ELEM_GET_NEXT(plan_flistp, PIN_FLD_DEVICES, &elem_id4, 1, 
						&cookie4, ebufp )) != NULL)
					{
						PIN_FLIST_FLD_DROP(dev_iflistp, PIN_FLD_TYPE, ebufp);
					}
				}
				prev_cookie = cookie3;
			}
			if(PIN_FLIST_ELEM_COUNT(hardware_flistp, PIN_FLD_PLAN, ebufp)>0)
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Calling MSO_OP_CUST_MANAGE_PLANS_ACTIVATION opcode", tmp_flistp);
				PCM_OP(ctxp, MSO_OP_CUST_MANAGE_PLANS_ACTIVATION, 0, tmp_flistp, &mng_ret_flistp, ebufp);
				if ((PIN_ERRBUF_IS_ERR(ebufp)) ||  
				(
				PIN_FLIST_FLD_GET(mng_ret_flistp, PIN_FLD_STATUS, 1, ebufp ) != NULL &&
				*(int*)PIN_FLIST_FLD_GET(mng_ret_flistp, PIN_FLD_STATUS, 1, ebufp ) !=0
				)
				)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error in manage plans activation");
					*r_flistpp = mng_ret_flistp;
					return;
				}
			}
	}
        PIN_FLIST_DESTROY_EX(&mng_ret_flistp, NULL); 
FINAL:
	if ( set_err == 1 )
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &allocate_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51400", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Mandatory POID values not passed in input flist", ebufp);
		*r_flistpp = ret_flistp;
		return;		
	}	
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Got error \n");
	}
	else
	{
		fm_mso_create_lifecycle_evt(ctxp, lc_in_flistp, NULL, ID_ALLOCATE_HARDWARE, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in lifecycle event creation", ebufp);
		}
		PIN_FLIST_DESTROY_EX(&lc_in_flistp, NULL);
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_allocate_hardware output flist", out_flistp);
	new_flistp = PIN_FLIST_CREATE(ebufp);
//	PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_POID, new_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(new_flistp, PIN_FLD_POID, account_obj, ebufp);
	PIN_FLIST_FLD_SET(new_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
        allocate_failure = 0;        
	PIN_FLIST_FLD_SET(new_flistp, PIN_FLD_STATUS, &allocate_failure, ebufp);
	*r_flistpp = new_flistp;;
       	PIN_FLIST_DESTROY_EX(&out_flistp, NULL);
	return;
}

//extern int
//fm_mso_purchase_static_ip(
//        pcm_context_t           *ctxp,
//        pin_flist_t             *in_flistp,
//        pin_errbuf_t            *ebufp)
//{
//
//	pin_flist_t		*search_flistp = NULL;
//	pin_flist_t		*search_res_flistp = NULL;
//	pin_flist_t		*args_flistp = NULL;
//	pin_flist_t		*result_flistp = NULL;
//	pin_flist_t		*services_info = NULL;
//	pin_flist_t		*allocate_in_flistp = NULL;
//	pin_flist_t		*allocate_out_flistp = NULL;
//		
//	poid_t 			*service_obj = NULL;
//	poid_t			*account_obj = NULL;
//	poid_t 			*search_pdp = NULL;
//	poid_t 			*device_obj = NULL;
//
//	int64                   db = -1;
//	int32                   search_flags = 256;	
//	int32                   flags = 1;	
//	int32                   good_state = MSO_GOOD_STATE;	
//	/*Pawan:23-01-2015: Added F1=V1 condition below*/
//	char			search_template[150] = "select X from /device/ip/static where F1 = V1 and ROWNUM = 1 and poid_type like '/device/ip/static' order by device_id asc";	
//	
//	if (PIN_ERRBUF_IS_ERR(ebufp)) {
//		return;
//	}
//	service_obj = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp); 
//	account_obj = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp); 
//	db = PIN_POID_GET_DB(service_obj);
//
//	search_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
//	search_flistp = PIN_FLIST_CREATE(ebufp);
//	PIN_FLIST_FLD_PUT(search_flistp, PIN_FLD_POID, search_pdp, ebufp);
//	PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_FLAGS, &search_flags, ebufp);
//	PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_TEMPLATE, search_template, ebufp);
//
//	args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 1, ebufp);
//	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATE_ID, &good_state, ebufp);
//	result_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_RESULTS, 0, ebufp);
//	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
//	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_DEVICE_ID, (char *)NULL, ebufp);
//	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_DEVICE_TYPE, (char *)NULL, ebufp);
//	
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_allocate_hardware search static device ip input list", search_flistp);
//	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_flistp, &search_res_flistp, ebufp);
//	PIN_FLIST_DESTROY_EX(&search_flistp, NULL);
//	if (PIN_ERRBUF_IS_ERR(ebufp))
//	{
//		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in searching static device ip", ebufp);
//		PIN_ERRBUF_RESET(ebufp);
//		PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
//		return 0;
//	}
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_allocate_hardware search static device ip output list", search_res_flistp);
//	
//	result_flistp = PIN_FLIST_ELEM_GET(search_res_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);	
//	if ( !result_flistp )
//	{
//		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in searching static device ip", ebufp);
//		PIN_ERRBUF_RESET(ebufp);
//		PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
//		return 0;
//	}
//
//	device_obj = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_POID, 1, ebufp); 
//	allocate_in_flistp = PIN_FLIST_CREATE(ebufp);
//	PIN_FLIST_FLD_SET( allocate_in_flistp, PIN_FLD_POID, device_obj, ebufp );
//	PIN_FLIST_FLD_SET( allocate_in_flistp, PIN_FLD_FLAGS, &flags, ebufp);
//	PIN_FLIST_FLD_COPY( in_flistp, PIN_FLD_PROGRAM_NAME, allocate_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
//	services_info = PIN_FLIST_ELEM_ADD(allocate_in_flistp, PIN_FLD_SERVICES, 0, ebufp);
//	PIN_FLIST_FLD_SET(services_info,PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
//	PIN_FLIST_FLD_SET(services_info,PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_allocate_hardware: Input flist", allocate_in_flistp);
//	PCM_OP(ctxp, PCM_OP_DEVICE_ASSOCIATE, 0, allocate_in_flistp, &allocate_out_flistp, ebufp);
//	PIN_FLIST_DESTROY_EX(&allocate_in_flistp, NULL);
//	if (PIN_ERRBUF_IS_ERR(ebufp))
//	{
//		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in associating Static IP Device", ebufp);
//		PIN_ERRBUF_RESET(ebufp);
//		PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
//		PIN_FLIST_DESTROY_EX(&allocate_out_flistp, NULL);
//		return 0;
//	}
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_allocate_hardware: Static IP device associate output flist:", allocate_out_flistp);
//
//	PIN_FLIST_DESTROY_EX(&search_flistp, NULL);
//	PIN_FLIST_DESTROY_EX(&allocate_out_flistp, NULL);
//	return 1;
//}

int 
validate_device_type(
	pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             *device_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp
		) 
{
	int32		device_type = 0;
	char		*device_type_str = NULL;
	int32		validation_failure = 1;
	int32		validation_success = 0;
	int32		technology = 0;
	pin_flist_t	*ret_flistp = NULL;
	pin_flist_t	*svc_read_flistp = NULL;
	pin_flist_t	*svc_read_oflistp = NULL;
	pin_flist_t	*mso_bb_flistp = NULL;
	char		*device_poid_type = NULL;
	int		i =0;
	poid_t		*device_poid = NULL;

	PIN_ERR_LOG_FLIST ( PIN_ERR_LEVEL_DEBUG, "Inside validate_device_type i_flistp",i_flistp);
	PIN_ERR_LOG_FLIST ( PIN_ERR_LEVEL_DEBUG, "Inside validate_device_type device_flistp",device_flistp);

	/****** Pavan Bellala 30-July-2015 *********************
	Defect ID 1294 :Fixed the bug during validation
	********************************************************/
	//device_poid_type = PIN_FLIST_FLD_GET(device_flistp, PIN_FLD_POID, 0, ebufp);

	device_poid = PIN_FLIST_FLD_GET(device_flistp, PIN_FLD_POID, 0, ebufp);
	if(device_poid) device_poid_type = (char *)PIN_POID_GET_TYPE(device_poid);

	if(strcmp(device_poid_type,MSO_OBJ_TYPE_DEVICE_MODEM) == 0) // Currently validation addeonly for modem
	{	
		device_type_str = PIN_FLIST_FLD_GET(device_flistp, PIN_FLD_DEVICE_TYPE, 0, ebufp);

		if(!device_type_str || (strcmp(device_type_str, "") == 0)) 
		{
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &validation_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51400", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Invalid Device type:NULL string found", ebufp);
			*r_flistpp = ret_flistp;
			return PIN_BOOLEAN_FALSE;		
		} else {
			for( i = 0; i < strlen(device_type_str);i++)
			{
				// Device type string should contain only numbers.
				if( device_type_str[i] < 48 || device_type_str[i] > 57 )
				{
					ret_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &validation_failure, ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51400", ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Invalid Device type: Non-numeric value found",ebufp)
					*r_flistpp = ret_flistp;

					return PIN_BOOLEAN_FALSE;
				}
			}

		}
	
		device_type = atoi(device_type_str);

		svc_read_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, svc_read_flistp, PIN_FLD_POID, ebufp);
		mso_bb_flistp = PIN_FLIST_ELEM_ADD(svc_read_flistp, MSO_FLD_BB_INFO, 0, ebufp);
		PIN_FLIST_FLD_SET(mso_bb_flistp, MSO_FLD_TECHNOLOGY, NULL, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read bb info technology Input flist", svc_read_flistp);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, svc_read_flistp, &svc_read_oflistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read bb info technology output flist", svc_read_oflistp);

		mso_bb_flistp = PIN_FLIST_ELEM_GET(svc_read_oflistp, MSO_FLD_BB_INFO, 0, 0, ebufp);
		technology = *(int32 *)PIN_FLIST_FLD_GET(mso_bb_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp);
        PIN_FLIST_DESTROY_EX(&svc_read_flistp, NULL);
		
		//Jyothirmayi === Code for JIO-CPS Integration-ISP Enhancement
		// Check if the device type is valid for the Docsis3 technology
		if(technology == MSO_DOCSIS_3) 
		{			
			// if the device type is DOCSIS2 Modem (10) / JIO GPON Modem (35) / Hathway GPON Modem (30), throw error
			if( (device_type == DOCSIS2_CABLE_MODEM) || 
			    (strcmp(device_type_str, JIO_GPON_MODEM) == 0) || 
				(strcmp(device_type_str, HW_GPON_MODEM) == 0) ) 
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Invalid device type for DOCSIS 3 technology");
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &validation_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51402", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Invalid Device type for DOCSIS 3: Technology and Device mismatch", ebufp);
				*r_flistpp = ret_flistp;
				return PIN_BOOLEAN_FALSE;
			}
		}
		// Check if the device type is valid for the Docsis2 technology
		else if(technology == MSO_DOCSIS_2) 
		{
			// if the device type is JIO GPON Modem (35) / Hathway GPON Modem (30), throw error
			if( (strcmp(device_type_str, JIO_GPON_MODEM) == 0) || 
				(strcmp(device_type_str, HW_GPON_MODEM) == 0) ) 
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Invalid device type for DOCSIS 2 technology");
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &validation_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51403", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Invalid Device type for DOCSIS 2: Technology and Device mismatch", ebufp);
				*r_flistpp = ret_flistp;
				return PIN_BOOLEAN_FALSE;
			}
		} 
		// Check if the device type is valid for the Modems related to GPON technology
		else if(technology == GPON_TECH) 
		{			
			if( (device_type == DOCSIS2_CABLE_MODEM) || 
				(device_type == DOCSIS3_CABLE_MODEM) ) 
			{
				// if the device type is DOCSIS2 Modem (10) / DOCSIS3 Modem (20), throw error
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Invalid device type for GPON technology");
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &validation_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51404", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Invalid Device type for GPON: Technology and Device mismatch", ebufp);
				*r_flistpp = ret_flistp;
				return PIN_BOOLEAN_FALSE;
			}
		} 
	}

	PIN_ERR_LOG_FLIST ( PIN_ERR_LEVEL_DEBUG, "Inside validate_device_type r_flistpp",*r_flistpp);
	PIN_FLIST_DESTROY_EX(&svc_read_oflistp, NULL);
	return PIN_BOOLEAN_TRUE;
}

/**************************************************
* Function: fm_mso_get_plan_activation()
* 
* 
***************************************************/
static int
fm_mso_get_plan_activation(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	poid_t			**mso_act_pdp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*ser_flistp = NULL;
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*plan_flistp = NULL;
	poid_t			*acc_pdp = NULL;
	poid_t			*ser_pdp = NULL;
	poid_t			*srch_pdp = NULL;
	char			*template = "select X from /mso_plans_activation where F1.id = V1 and F2.id = V2 ";
	int64			db=0;
	int32			srch_flag=0;
	int				count=0;
	int				*status = NULL;
	pin_cookie_t		cookie=NULL;
	int			elem_id = 0;
	void			*vp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_plan_activation", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_plan_activation input flist", in_flistp);	
	*mso_act_pdp = NULL;
	acc_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	ser_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
	
	db = PIN_POID_GET_DB(acc_pdp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);
	
	arg_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acc_pdp, ebufp);
	
	arg_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_SERVICE_OBJ, ser_pdp, ebufp);
	
	result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Plan Activation search input flist", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
	        PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);	 
		return 1;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Plan Activation search output flist", srch_out_flistp);
	count = PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp);
	if(count == 1)
	{
		result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp );
		status = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_STATUS, 0, ebufp);
		if (status && *((int32 *)status)!= 0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Status of MSO purchased plan object is not New.");
			PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);	
			return 1;
		}
		cookie = NULL;
		while ((plan_flistp = PIN_FLIST_ELEM_GET_NEXT(result_flist, PIN_FLD_PLAN, &elem_id, 1, &cookie, ebufp )) != NULL)		
		{
			vp = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_DESCR, 0, ebufp);
			if(strcmp((char *)vp,"base plan")==0)
			{
				*mso_act_pdp = PIN_FLIST_FLD_TAKE(plan_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
				break;
			}
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Cannot find MSO purchased plan object.");
	        PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);	
		return 1;
	}

	        PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);	
	return 0;
}

/**************************************************
* Function: fm_mso_get_mso_pur_bp()
* 
* 
***************************************************/
static int
fm_mso_get_mso_pur_bp(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	poid_t			**mso_pur_pdp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*ser_flistp = NULL;
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	poid_t			*acc_pdp = NULL;
	poid_t			*ser_pdp = NULL;
	poid_t			*srch_pdp = NULL;
	char			*template = "select X from /mso_purchased_plan where F1.id = V1 and F2.id = V2 and F3 = V3 and ( F4 = V4 or F5 = V5) ";
	int64			db=0;
	int32			srch_flag=0;
	int			count=0;
	int			status = MSO_PROV_ACTIVE;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_mso_pur_bp", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_mso_pur_bp input flist", in_flistp);	
	*mso_pur_pdp = NULL;
	acc_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	ser_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
	
	db = PIN_POID_GET_DB(acc_pdp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);
	
	arg_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acc_pdp, ebufp);
	
	arg_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_SERVICE_OBJ, ser_pdp, ebufp);
	
	arg_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 3, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_DESCR, "base plan", ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 4, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STATUS, &status, ebufp);
	//Added "In progress" in or condition. It will be the case if this 
	// function is called from change plan.
	status = MSO_PROV_IN_PROGRESS;
	arg_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 5, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STATUS, &status, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchased Plan search input flist", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		PIN_ERRBUF_CLEAR(ebufp);
		return 1;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchased Plan search output flist", srch_out_flistp);
	count = PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp);
	if(count == 1)
	{
		result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp );
		*mso_pur_pdp = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_PLAN_OBJ, 0, ebufp);
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Cannot find MSO purchased plan object.");
		return 1;
	}
	return 0;
}
/*************************************************************************************
* function: fm_mso_purchase_static_ip()
*	Also called from modify customer when Auth mech
*	is changed to TAL.
*	Flag value may be : 0 = called before service activation (allocate hardware)
*			    1 = called after service activation (Non TAL to TAL)
***************************************************************************************/
extern int
fm_mso_purchase_static_ip(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
	int32			flag,
        pin_errbuf_t            *ebufp)
{

	pin_flist_t		*read_in_flistp = NULL;
	pin_flist_t		*read_out_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*services_info = NULL;
	pin_flist_t		*allocate_in_flistp = NULL;
	pin_flist_t		*allocate_out_flistp = NULL;
	pin_flist_t		*bb_flistp = NULL;
		
	poid_t 			*service_obj = NULL;
	poid_t			*account_obj = NULL;
	poid_t 			*search_pdp = NULL;
	poid_t 			*device_obj = NULL;
	poid_t 			*plan_obj = NULL;

	int64                   db = -1;
	int64			pkg_id = 0;
	int32                   search_flags = 256;	
	int32                   flags = 1;	
	int32                   good_state = MSO_GOOD_STATE;
	int32			num_of_ip = 1;
	int32			flag_val = 200; // Flag value in MSO_OP_SEARCH to search static IP address
	int32			*ret_val = NULL;
	char			*cmts_id = NULL;

	char			*scenariop = NULL;
	char			*new_ip_addrp = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		return;
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_purchase_static_ip: input",in_flistp);
	
	service_obj = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp); 
	account_obj = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp); 
	
	read_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_in_flistp, PIN_FLD_POID, service_obj, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read Flds input", read_in_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_in_flistp, &read_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&read_in_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Reading Service Object", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);
		return 1;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read Flds input output ", read_out_flistp);
	
	bb_flistp = PIN_FLIST_SUBSTR_GET(read_out_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	cmts_id = PIN_FLIST_FLD_TAKE(bb_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp); 
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in reading CMTS ID for service", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);
		return 1;
	}
	if(!cmts_id || strlen(cmts_id)==0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "CMTS ID is NULL for service");
		PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);
		return 1;
	}
	PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);

	// Get Base plan obj for service.
	if(flag == 0)
	{
		fm_mso_get_plan_activation(ctxp, in_flistp, &plan_obj, ebufp);
	}
	else
	{
		/**** Pavan Bellala 04-09-2015 ***************************
		Base plan from /mso_purchased_plan is considered for 
		status = 2,4,5,6 in CHANGE CMTS scenario
		*********************************************************/
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Call from add device after activation");
		scenariop = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SCENARIO_NAME,1,ebufp);
		fm_mso_get_mso_pur_bp(ctxp, in_flistp, &plan_obj, ebufp);
		if(PIN_POID_IS_NULL(plan_obj))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"mso_purchased_plan is NULL here");	
			if(scenariop && (strcmp(scenariop,"CHANGE CMTS")== 0))
			{
				plan_obj = PIN_FLIST_FLD_GET(in_flistp,PIN_FLD_PLAN_OBJ,1,ebufp);	
				PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG,"Purchased plan object",plan_obj);
			}				
		}
	}
	
	if(!plan_obj)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Error in Getting base plan object");
		PIN_ERRBUF_RESET(ebufp);
		return 1;
	}
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG,"Plan Obj is ",plan_obj);
	// Search Static IP for Plan and CMTS
	db = PIN_POID_GET_DB(account_obj);
	search_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	read_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(read_in_flistp, PIN_FLD_POID, search_pdp, ebufp);
	//pkg_id = PIN_POID_GET_ID(plan_obj);
	PIN_FLIST_FLD_SET(read_in_flistp, PIN_FLD_PLAN_OBJ, plan_obj, ebufp );
	PIN_FLIST_FLD_PUT(read_in_flistp, MSO_FLD_CMTS_ID, cmts_id, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, read_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp );
	PIN_FLIST_FLD_SET(read_in_flistp, PIN_FLD_FLAGS, &flag_val, ebufp );
	PIN_FLIST_FLD_SET(read_in_flistp, PIN_FLD_TYPE, &num_of_ip, ebufp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_SEARCH input flist", read_in_flistp);
	PCM_OP(ctxp, MSO_OP_SEARCH, 0, read_in_flistp, &read_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_SEARCH output flist", read_out_flistp);
	PIN_FLIST_DESTROY_EX(&read_in_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in Searching Static IP ", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);
		return 1;
	}
	ret_val = PIN_FLIST_FLD_GET(read_out_flistp, PIN_FLD_STATUS, 1, ebufp);
	if(ret_val && *ret_val==1)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Static IP not found for Specified CMTS Id and Plan");
		PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);
		return 2;
	}

	result_flistp = PIN_FLIST_ELEM_GET(read_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp );
	device_obj = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_POID, 1, ebufp); 

	/*********** Pavan Bellala 07-09-2015 *********************************
	Changes made to fetch the IP address that is newly assigned
	in case of IS_TAL customers
	**********************************************************************/
	new_ip_addrp = PIN_FLIST_FLD_TAKE(result_flistp,PIN_FLD_DEVICE_ID,1,ebufp);

	allocate_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET( allocate_in_flistp, PIN_FLD_POID, device_obj, ebufp );
	PIN_FLIST_FLD_SET( allocate_in_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_COPY( in_flistp, PIN_FLD_PROGRAM_NAME, allocate_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	services_info = PIN_FLIST_ELEM_ADD(allocate_in_flistp, PIN_FLD_SERVICES, 0, ebufp);
	PIN_FLIST_FLD_SET(services_info,PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
	PIN_FLIST_FLD_SET(services_info,PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_DEVICE_ASSOCIATE : Input flist", allocate_in_flistp);
	PCM_OP(ctxp, PCM_OP_DEVICE_ASSOCIATE, 0, allocate_in_flistp, &allocate_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&allocate_in_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in associating Static IP Device", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&allocate_out_flistp, NULL);
		if(new_ip_addrp) free(new_ip_addrp);
		return 1;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_DEVICE_ASSOCIATE: output flist:", allocate_out_flistp);
	if(new_ip_addrp)
	{
		PIN_FLIST_FLD_PUT(in_flistp, MSO_FLD_NETWORKID, new_ip_addrp,ebufp);	
	}

	PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&allocate_out_flistp, NULL);
	return 0;
}

/**************************************************
* Function: fm_mso_get_active_modem()
*	Finds if there is an active modem already 
*	associated with service
***************************************************/
static int
fm_mso_get_active_modem(
	pcm_context_t		*ctxp,
	poid_t			*ser_pdp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*ser_flistp = NULL;
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	poid_t			*srch_pdp = NULL;
	char			*template = "select X from /device where F1 = V1 and F2 = V2 ";
	int64			db=0;
	int32			srch_flag=512;
	int			count=0;
	int			state_allocated = 2;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_active_modem", ebufp);
	}
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Service Poid", ser_pdp);	

	db = PIN_POID_GET_DB(ser_pdp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);
	
	arg_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	ser_flistp = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_SERVICES, 0, ebufp );
	PIN_FLIST_FLD_SET(ser_flistp, PIN_FLD_SERVICE_OBJ, ser_pdp, ebufp);
	
	arg_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STATE_ID, &state_allocated, ebufp);
	
	result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Device input flist", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		return 1;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Device output flist", srch_out_flistp);
	count = PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp);
	if(count >= 1)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Active modem found.");
		return 1;
	}
	return 0;
}

/* ************************************************************************
 * Changes for JIO-CPS Integration-ISP Enhancement
 * This function gets the flist with the data related to Network Element ID 
 * and the Network Provider from the mso_cfg_cmts_master object based 
 * on the CMTS ID and the City of the service for which the hardware 
 * is getting allocated.
 ************************************************************************/
static void
fm_mso_get_carrier_id(
	pcm_context_t		*ctxp,
	char				*cmts_id,
	char				*city,
	pin_flist_t         **r_flistpp,
	int					*error_codep,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t			*search_cmts_flistp = NULL;
	pin_flist_t			*search_cmts_oflistp = NULL;
	pin_flist_t			*arg_flistp = NULL;
	pin_flist_t			*res_iflistp = NULL;
	pin_flist_t			*result_flistp = NULL;
	poid_t				*pdp = NULL;
	poid_t				*s_pdp = NULL;
	int32				s_flags = 768;
    int32				database = 0;

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_carrier_id: inputs:");
	
	if(cmts_id != NULL)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "\nCMTS ID:");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, cmts_id);
	}
	
	if(city != NULL)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "\nCity:");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, city);
	}

	/* ***************************************************************************
	 * Input flist for Search Opcode
	 * 0 PIN_FLD_POID                      POID [0] 0.0.0.1 /search -1 0
	 * 0 PIN_FLD_FLAGS                      INT [0] 768
	 * 0 PIN_FLD_TEMPLATE                   STR [0] "select X from /mso_cfg_cmts_master where F1 = V1 and F2 = V2 "
	 * 0 PIN_FLD_RESULTS                  ARRAY [0] allocated 2, used 2
	 * 1     MSO_FLD_OTHER_NE_ID              STR [0] NULL
	 * 1     PIN_FLD_CARRIER_ID		       INT [0] 0
	 * 0 PIN_FLD_ARGS                     ARRAY [1] allocated 1, used 1
	 * 1     MSO_FLD_CMTS_ID           	   STR [0] "JIO_TEST_GPON"
	 * 0 PIN_FLD_ARGS                     ARRAY [2] allocated 1, used 1
	 * 1     PIN_FLD_CITY           		   STR [0] "MUMBAI"
	 *****************************************************************************/
	
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
	if(cmts_id != NULL)
	{
		PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_CMTS_ID, cmts_id, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_CMTS_ID, NULL, ebufp);
	}
	
	arg_flistp = PIN_FLIST_ELEM_ADD(search_cmts_flistp, PIN_FLD_ARGS, 2, ebufp);
	if(city != NULL)
	{
		PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_CITY, city, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_CITY, NULL, ebufp);
	}

	res_iflistp = PIN_FLIST_ELEM_ADD( search_cmts_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(res_iflistp, MSO_FLD_OTHER_NE_ID, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_iflistp, PIN_FLD_CARRIER_ID, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Allocate Hardware - fm_mso_get_carrier_id: Search CMTS Master Input Flist ", search_cmts_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_cmts_flistp, &search_cmts_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Allocate Hardware - fm_mso_get_carrier_id: Search CMTS Master Output Flist", search_cmts_oflistp);
	PIN_FLIST_DESTROY_EX(&search_cmts_flistp, NULL);
						
	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		*error_codep = MSO_BB_INTERNAL_ERROR;
             PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "Error getting the CMTS Master data", ebufp);        
		PIN_FLIST_DESTROY_EX(&search_cmts_oflistp, NULL);
	}
	else
	{	
		result_flistp = PIN_FLIST_ELEM_TAKE(search_cmts_oflistp,PIN_FLD_RESULTS,0,1,ebufp);
		
		if(result_flistp != NULL)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Allocate Hardware - fm_mso_get_carrier_id: Search CMTS Master Return Flist", result_flistp);
			*r_flistpp = result_flistp;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Returning from fm_mso_get_carrier_id for Allocate Hardware Opcode >> ");
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Allocate Hardware - fm_mso_get_carrier_id: Search CMTS Master Return Flist", *r_flistpp);
		}
		
		PIN_FLIST_DESTROY_EX(&search_cmts_oflistp, NULL);
	}	
	return;
}

int
fm_mso_chk_device_warehouse(
        pcm_context_t           *ctxp,
        char                    *lco_code,
		char                    *cmts,
        char                    *city,
		pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *srch_in_flistp = NULL;
        pin_flist_t             *srch_out_flistp = NULL;
		pin_flist_t             *srch_out_flistp1 = NULL;
        pin_flist_t             *arg_flistp = NULL;
        pin_flist_t             *tmp_flistp = NULL;
        pin_flist_t             *result_flist = NULL;
        pin_flist_t             *result_flist1 = NULL;
		pin_flist_t	*ret_flistp = NULL;
        int32                   srch_flag = 512;
		int32                   *mig_flag = 0;
        int                     db = 1;
		int32			validation_failure = 1;
        char                    *template = "select X from /mso_cfg_wh_cmts_map 1, /account 2 where 2.F1 = V1 and 1.F2 = V2 and 1.F3 = 2.F4 and rownum<2 ";

		pin_cookie_t		cookie=NULL;
		int			elem_id = 0;

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, city);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, cmts);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, lco_code);

        srch_in_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);

        arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACCOUNT_NO, (void *) "ROOT", ebufp);
		
        arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 2, ebufp );	
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_CITY, city, ebufp);

		arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 3, ebufp );	
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);

		arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 4, ebufp );	
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, NULL, ebufp);


        result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp );
		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_FLAGS, NULL, ebufp );

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_chk_device_warehouse search input flist", srch_in_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_chk_device_warehouse output flist", srch_out_flistp);


		while ((result_flist1 = PIN_FLIST_ELEM_GET_NEXT(srch_out_flistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp )) != NULL)		
		{
			mig_flag = PIN_FLIST_FLD_GET(result_flist1, PIN_FLD_FLAGS, 1, ebufp); 
			if(*mig_flag == 0)
			{
				return 1;
			}
			else
			{
				arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
		        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACCOUNT_NO, lco_code, ebufp);

	            arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 2, ebufp );	
		        PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_CMTS_ID, cmts, ebufp);

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_chk_device_warehouse search input flist1", srch_in_flistp);
				PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp1, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_chk_device_warehouse output flist1", srch_out_flistp1);
			

				if ( PIN_ERRBUF_IS_ERR(ebufp) || PIN_FLIST_ELEM_COUNT(srch_out_flistp1, PIN_FLD_RESULTS, ebufp)==0)
				{
						PIN_ERRBUF_CLEAR(ebufp);
						ret_flistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_COPY(srch_out_flistp1, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp);
						PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &validation_failure, ebufp);
						PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51405", ebufp);
						PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Device source is invalid: cmts mismatch ", ebufp);
						*r_flistpp = ret_flistp;
						return 0;
				}
				else
				{
						return 1;
				}
			}
			
		}
                                                        

        PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_out_flistp1, NULL);

		return 1;
}

