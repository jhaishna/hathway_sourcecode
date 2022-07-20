/*******************************************************************
 * File:	fm_mso_cust_modify_customer.c
 * Opcode:	MSO_OP_CUST_MODIFY_CUSTOMER 
 * Owner:	Rohini Mogili
 * Created:	08-NOV-2013
 * Description: This is Customer modification for below-
 * Modification History: updated fm_mso_update_lcoinfo
 * Modified By:Sisir Samanta
 * Date:10-Oct-14
 * Modification details : Updated fm_mso_update_lcoinfo to handle bulk & online action
	1. Name Info
	2. Billing Address
	3. Installtion Address
	4. Invoice Delivery Method
	5. Contact Preferences
	6.Contacts
	7. Payment Information
	8. Billing Information
	9. LCO information
	10. Service Information
	11. Credit Limit
	12. Deactivate business user.
	
	-------------------------
	function calling sequence
	-------------------------
	op_mso_cust_modify_customer
		fm_mso_cust_modify_customer
			fm_mso_validate_csr_role
			fm_mso_get_account_info
			fm_mso_get_business_type_values
			fm_mso_populate_mod_tax_info
			fm_mso_update_custinfo
			fm_mso_update_businessuserinfo
			fm_mso_update_billinfo
			fm_mso_update_serviceinfo
			fm_mso_update_payinfo
			fm_mso_update_lcoinfo
			fm_mso_update_roles
                        //BB
			fm_mso_update_user_status
			fm_mso_create_lco_change_evt
			fm_mso_get_outstanding_bal
			fm_mso_acc_model_change
			fm_mso_acc_type_change
			fm_mso_update_bb_service
			fm_mso_update_bb_billinfo
			fm_mso_update_bal_info
			fm_mso_update_payinfo_inv
			fm_mso_update_tax_info
			fm_mso_update_bususer_ar_info

	r << xx 1
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 47228 10
	0 PIN_FLD_USERID         POID [0] 0.0.0.1 /service/admin_client 43523 10
	0 PIN_FLD_ACCOUNT_OBJ    POID [0] 0.0.0.1 /account 47228 10
	0 PIN_FLD_ACCOUNT_NO      STR [0] "0.0.0.1-47228"
	0 PIN_FLD_PROGRAM_NAME    STR [0] "Customer Center"
	0 MSO_FLD_CONTACT_PREF   ENUM [0] 5
	0 MSO_FLD_AREA            STR [0] "area"
	0 MSO_FLD_RMAIL           STR [0] ""
	0 MSO_FLD_RMN             STR [0] ""
	0 PIN_FLD_NAMEINFO      ARRAY [1] allocated 20, used 2
	1     PIN_FLD_ADDRESS         STR [0] "flat no,phase\nbuilding\nstreet"
	1     PIN_FLD_CANON_COMPANY    STR [0] ""
	1     PIN_FLD_CANON_COUNTRY    STR [0] "IN"
	1     PIN_FLD_CITY            STR [0] "KOLKATA"
	1     PIN_FLD_COMPANY         STR [0] ""
	1     PIN_FLD_CONTACT_TYPE    STR [0] ""
	1     PIN_FLD_COUNTRY         STR [0] "INDIA"
	1     PIN_FLD_EMAIL_ADDR      STR [0] "ramesh@hathwat.co.in"
	1     PIN_FLD_FIRST_NAME      STR [0] "TEST1"
	1     PIN_FLD_LAST_NAME       STR [0] "ACCT"
	1     PIN_FLD_MIDDLE_NAME     STR [0] "TEST2"
	1     PIN_FLD_SALUTATION      STR [0] "Mr."
	1     PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.0  0 0
	1     PIN_FLD_STATE           STR [0] "MAHARASTRA"
	1     PIN_FLD_TITLE           STR [0] "SME"
	1     PIN_FLD_ZIP             STR [0] ""
	1     PIN_FLD_PHONES        ARRAY [0] allocated 20, used 2
	2         PIN_FLD_PHONE           STR [0] "98893552151"
	2         PIN_FLD_TYPE           ENUM [0] 1
	1     PIN_FLD_PHONES        ARRAY [1] allocated 20, used 2
	2         PIN_FLD_PHONE           STR [0] "98893557151"
	2         PIN_FLD_TYPE           ENUM [0] 2
	1     PIN_FLD_PHONES        ARRAY [3] allocated 20, used 2
	2         PIN_FLD_PHONE           STR [0] "98893552158"
	2         PIN_FLD_TYPE           ENUM [0] 5
	0 PIN_FLD_PAYINFO                         ARRAY [0] allocated 20, used 6
	1     PIN_FLD_INV_INFO                 ARRAY [0] allocated 20, used 20
	2         PIN_FLD_DELIVERY_PREFER           ENUM [0] 1
	0 PIN_FLD_BILLINFO      ARRAY [*] allocated 20, used 1
	1     PIN_FLD_BILL_WHEN       INT [0] 2
	0 PIN_FLD_CUSTOMER_CARE_INFO   SUBSTRUCT [0] allocated 7, used 7
	1     MSO_FLD_LCO_OBJ               POID [0] 0.0.0.1 /account 47906 1
	0 PIN_FLD_SERVICES                         ARRAY [0] allocated 20, used 6
	1     PIN_FLD_SERVICE_OBJ                   POID [0] 0.0.0.1 /service/catv -1 0
	1     MSO_FLD_CATV_INFO                SUBSTRUCT [0] allocated 20, used 20
	2         MSO_FLD_BOUQUET_ID                 STR [0] "bouquet_id"
	2         MSO_FLD_REGION_KEY             STR [0] "IN-700135"
	1     PIN_FLD_CREDIT_LIMIT		DECIMAL [0] 1000
	xx
	xop 11004 0 1




	0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 47228 10
	0 PIN_FLD_USERID         POID [0] 0.0.0.1 /service/admin_client 43523 10
	0 PIN_FLD_ACCOUNT_NO      STR [0] "0.0.0.1-47228"
	0 PIN_FLD_DESCR           STR [0] "ACCOUNT Modification done successfully"
	0 PIN_FLD_DATA_ARRAY	ARRAY [0] 
	1     MSO_FLD_CONTACT_PREF   ENUM [0] 5
	1     MSO_FLD_AREA            STR [0] "area"
	1     MSO_FLD_RMAIL           STR [0] ""
	1     MSO_FLD_RMN             STR [0] ""
	1     PIN_FLD_NAMEINFO      ARRAY [1] allocated 20, used 2
	2	    PIN_FLD_ADDRESS         STR [0] "flat no,phase\nbuilding\nstreet"
	2	    PIN_FLD_CANON_COMPANY    STR [0] ""
	2	    PIN_FLD_CANON_COUNTRY    STR [0] "IN"
	2	    PIN_FLD_CITY            STR [0] "KOLKATA"
	2	    PIN_FLD_COMPANY         STR [0] ""
	2	    PIN_FLD_CONTACT_TYPE    STR [0] ""
	2	    PIN_FLD_COUNTRY         STR [0] "INDIA"
	2	    PIN_FLD_EMAIL_ADDR      STR [0] "ramesh@hathwat.co.in"
	2	    PIN_FLD_FIRST_NAME      STR [0] "TEST1"
	2	    PIN_FLD_LAST_NAME       STR [0] "ACCT"
	2	    PIN_FLD_MIDDLE_NAME     STR [0] "TEST2"
	2	    PIN_FLD_SALUTATION      STR [0] "Mr."
	2	    PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.0  0 0
	2	    PIN_FLD_STATE           STR [0] "MAHARASTRA"
	2	    PIN_FLD_TITLE           STR [0] "SME"
	2	    PIN_FLD_ZIP             STR [0] ""
	2	    PIN_FLD_PHONES        ARRAY [0] allocated 20, used 2
	3	        PIN_FLD_PHONE           STR [0] "98893552151"
	3	        PIN_FLD_TYPE           ENUM [0] 1
	2	    PIN_FLD_PHONES        ARRAY [1] allocated 20, used 2
	3	        PIN_FLD_PHONE           STR [0] "98893557151"
	3	        PIN_FLD_TYPE           ENUM [0] 2
	2	    PIN_FLD_PHONES        ARRAY [3] allocated 20, used 2
	3	        PIN_FLD_PHONE           STR [0] "98893552158"
	3	        PIN_FLD_TYPE           ENUM [0] 5
	1     PIN_FLD_PAYINFO                         ARRAY [0] allocated 20, used 6
	2         PIN_FLD_INV_INFO                 ARRAY [0] allocated 20, used 20
	3             PIN_FLD_DELIVERY_PREFER           ENUM [0] 1
	1     PIN_FLD_BILLINFO      ARRAY [*] allocated 20, used 1
	2         PIN_FLD_BILL_WHEN       INT [0] 2
	1     PIN_FLD_CUSTOMER_CARE_INFO   SUBSTRUCT [0] allocated 7, used 7
	2         MSO_FLD_LCO_OBJ               POID [0] 0.0.0.1 /account 47906 1
	1     PIN_FLD_SERVICES                         ARRAY [0] allocated 20, used 6
	2         PIN_FLD_SERVICE_OBJ                   POID [0] 0.0.0.1 /service/catv 37393 9
	2         MSO_FLD_CATV_INFO                SUBSTRUCT [0] allocated 20, used 20
	3             MSO_FLD_BOUQUET_ID                 STR [0] "bouquet_id"
	3             MSO_FLD_REGION_KEY             STR [0] "IN-700135"
	1     PIN_FLD_SERVICES                         ARRAY [0] allocated 20, used 6
	2         PIN_FLD_SERVICE_OBJ                   POID [0] 0.0.0.1 /service/catv 37393 9
	2         PIN_FLD_CREDIT_LIMIT		DECIMAL [0] 1000
	0 PIN_FLD_DATA_ARRAY	ARRAY [1] 
	1     MSO_FLD_CONTACT_PREF   ENUM [0] 4
	1     MSO_FLD_AREA            STR [0] "area1"
	1     MSO_FLD_RMAIL           STR [0] "abcd"
	1     MSO_FLD_RMN             STR [0] "1234"
	1     PIN_FLD_NAMEINFO      ARRAY [1] allocated 20, used 2
	2	    PIN_FLD_ADDRESS         STR [0] "flat no,phase\nbuilding\nstreet"
	2	    PIN_FLD_CANON_COMPANY    STR [0] ""
	2	    PIN_FLD_CANON_COUNTRY    STR [0] "IN"
	2	    PIN_FLD_CITY            STR [0] "KOLKATA"
	2	    PIN_FLD_COMPANY         STR [0] ""
	2	    PIN_FLD_CONTACT_TYPE    STR [0] ""
	2	    PIN_FLD_COUNTRY         STR [0] "INDIA"
	2	    PIN_FLD_EMAIL_ADDR      STR [0] "ramesh@hathwat.co.in"
	2	    PIN_FLD_FIRST_NAME      STR [0] "TEST1"
	2	    PIN_FLD_LAST_NAME       STR [0] "ACCT"
	2	    PIN_FLD_MIDDLE_NAME     STR [0] "TEST2"
	2	    PIN_FLD_SALUTATION      STR [0] "Mr."
	2	    PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.0  0 0
	2	    PIN_FLD_STATE           STR [0] "MAHARASTRA"
	2	    PIN_FLD_TITLE           STR [0] "SME"
	2	    PIN_FLD_ZIP             STR [0] ""
	2	    PIN_FLD_PHONES        ARRAY [0] allocated 20, used 2
	3	        PIN_FLD_PHONE           STR [0] "98893552152"
	3	        PIN_FLD_TYPE           ENUM [0] 1
	2	    PIN_FLD_PHONES        ARRAY [1] allocated 20, used 2
	3	        PIN_FLD_PHONE           STR [0] "98893557151"
	3	        PIN_FLD_TYPE           ENUM [0] 2
	2	    PIN_FLD_PHONES        ARRAY [3] allocated 20, used 2
	3	        PIN_FLD_PHONE           STR [0] "98893552158"
	3	        PIN_FLD_TYPE           ENUM [0] 5
	1     PIN_FLD_PAYINFO                         ARRAY [0] allocated 20, used 6
	2         PIN_FLD_INV_INFO                 ARRAY [0] allocated 20, used 20
	3             PIN_FLD_DELIVERY_PREFER           ENUM [0] 0
	1     PIN_FLD_BILLINFO      ARRAY [*] allocated 20, used 1
	2         PIN_FLD_BILL_WHEN       INT [0] 1
	1     PIN_FLD_CUSTOMER_CARE_INFO   SUBSTRUCT [0] allocated 7, used 7
	2         MSO_FLD_LCO_OBJ               POID [0] 0.0.0.1 /account 47906 1
	1     PIN_FLD_SERVICES                         ARRAY [0] allocated 20, used 6
	2         PIN_FLD_SERVICE_OBJ                   POID [0] 0.0.0.1 /service/catv 37393 9
	2         MSO_FLD_CATV_INFO                SUBSTRUCT [0] allocated 20, used 20
	3             MSO_FLD_BOUQUET_ID                 STR [0] "bouquet_id1"
	3             MSO_FLD_REGION_KEY             STR [0] "IN-700136"
	1     PIN_FLD_SERVICES                         ARRAY [0] allocated 20, used 6
	2         PIN_FLD_SERVICE_OBJ                   POID [0] 0.0.0.1 /service/catv 37393 9
	2         PIN_FLD_CREDIT_LIMIT		DECIMAL [0] 10000
	

	Deactivate Business User: Input
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 3714431 26
	0 PIN_FLD_USERID         POID [0] 0.0.0.1 /account 3714431 0
	0 MSO_FLD_AR_INFO       ARRAY [0]     NULL array ptr
	0 PIN_FLD_STATUS         ENUM [0] 10102
	0 PIN_FLD_PROGRAM_NAME    STR [0] "OAP|UatUser"
	0 PIN_FLD_DESCR           STR [0] "Moving Out"

 * Modification History:
 * Modified By:	Leela Pratyush
 * Date:	01-AUG-2013
 * Description: Added code for handling modify customer cases for BB
 *
 * Modified By: Jyothirmayi Kachiraju
 * Date: 12-DEC-2019
 * Modification details 
 * Description: Added code handling the validation for not allowing
 * JIO CMTS to be changed to Non JIO and vice-versa
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_modify_customer.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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
#include "pin_cust.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "mso_cust.h"
#include "mso_prov.h"
#include "mso_ntf.h"
#include "ops/device.h"
#include "mso_lifecycle_id.h"
#include "mso_device.h"
#include "ops/bal.h"

#define MSO_DEVICE_TYPE_SD  0
#define MSO_DEVICE_TYPE_HD  1
#define INVOICE_TYPE  10001

#define CREDIT_LIMIT_MAX_VAL     9999999999
#define CREDIT_LIMIT_MAX_VAL_STR "9999999999"  //must be same as  CREDIT_LIMIT_MAX_VAL
#define CREDIT_FLOOR_MIN_VAL     -9999999999
#define CREDIT_FLOOR_MIN_VAL_STR "-9999999999"  //must be same as  CREDIT_FLOOR_MIN_VAL

#define ACCESS_GLOBAL 0
#define DEVICE_DISSASSOCIATE 4
#define MSO_FLAG_SRCH_BY_SELF_ACCOUNT 9
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
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

extern void
fm_mso_get_business_type_values(
	pcm_context_t		*ctxp,
	int32			business_type,
	int32			*account_type,
	int32			*account_model,
	int32			*subscriber_type,
	pin_errbuf_t		*ebufp);

extern void
fm_mso_get_state_city_area_code(
	pcm_context_t		*ctxp,
	char			*state_city_area_code,
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);


extern void
fm_mso_get_bill_segment(
	pcm_context_t		*ctxp,
	char			*city_code,
	pin_flist_t		**r_flistp,
	pin_errbuf_t		*ebufp);
extern int
fm_mso_update_mso_purplan_status(
	pcm_context_t           *ctxp,
	poid_t                  *mso_purplan_obj,
	int						status,
	pin_errbuf_t            *ebufp);

extern int
fm_mso_update_ser_prov_status(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flistp,
	int 				prov_status,
	pin_errbuf_t            *ebufp);

extern
void fm_mso_create_lifecycle_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	int32			action,
	pin_errbuf_t		*ebufp);

extern int
fm_mso_purchase_static_ip(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
	int32			flag,
        pin_errbuf_t            *ebufp);

/**************************************
* External Functions end
***************************************/

/**************************************
* Local Functions start
***************************************/
EXPORT_OP int 
fm_mso_validate_csr_role(
	pcm_context_t		*ctxp,
	int64			db,
	poid_t			*user_id,
	pin_errbuf_t		*ebufp);

static int 
fm_mso_update_nameinfo(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		*out_flist,
	pin_errbuf_t		*ebufp);

static void
fm_mso_populate_tax_info(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        int32                   account_model,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp); 
		
static int 
fm_mso_update_custinfo(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		*out_flist,
	pin_errbuf_t		*ebufp);

static int 
fm_mso_update_billinfo(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		*out_flist,
	pin_errbuf_t		*ebufp);

static int 
fm_mso_update_serviceinfo(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		*out_flist,
	pin_errbuf_t		*ebufp);

static int 
fm_mso_update_payinfo(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		*out_flist,
	pin_errbuf_t		*ebufp);

static int 
fm_mso_update_lcoinfo(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		*out_flist,
	pin_flist_t		**comm_err_flist,
	pin_errbuf_t		*ebufp);

static int 
fm_mso_update_businessuserinfo(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		*out_flist,
	pin_errbuf_t		*ebufp);

static int 
fm_mso_update_roles(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		*out_flist,
	pin_errbuf_t		*ebufp);
	
/* ************************************************************************
 * Added the below code for JIO-CPS Integration-ISP Enhancement
 * This function gets the flist with the data related to Carrier ID 
 * from the mso_cfg_cmts_master object based on the CMTS ID
 ************************************************************************/
static void
fm_mso_get_carrier_id(
	pcm_context_t		*ctxp,
	char				*cmts_id,
	pin_flist_t         **r_flistpp,
	pin_errbuf_t		*ebufp);
  
/**************************************
* New Local Functions for BB Service
***************************************/
static int 
fm_mso_update_bususer_access(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		*accessinfo,
	pin_flist_t		*out_flist,
	pin_errbuf_t		*ebufp);

static int 
fm_mso_update_cntinfo(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_errbuf_t		*ebufp);

static int 
fm_mso_update_cust_credential(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		*out_flist,
	pin_errbuf_t		*ebufp);

static int
fm_mso_update_bususer_ar_info(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flist,
	pin_flist_t		*out_flist,
        pin_errbuf_t            *ebufp);

static int 
fm_mso_update_tax_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp);

static int 
fm_mso_update_payinfo_inv(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_errbuf_t		*ebufp);

static int 
fm_mso_update_bal_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*svc_flistp,
	pin_flist_t		*out_flist,
	pin_errbuf_t		*ebufp);

static int 
fm_mso_update_bb_billinfo(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		*out_flist,
	pin_errbuf_t		*ebufp);

static int
fm_mso_acc_type_change(
        pcm_context_t           *ctxp,
        int64	                *bus_type,
        pin_flist_t             *in_flist,
	pin_flist_t		*out_flist,
        pin_errbuf_t            *ebufp);

static int
fm_mso_acc_model_change(
        pcm_context_t           *ctxp,
        int64	                *bus_type,
	int64			*old_bus_type,
        pin_flist_t             *in_flist,
	pin_flist_t		*out_flistp,
        pin_errbuf_t            *ebufp);

static int
fm_mso_subs_type_change(
        pcm_context_t           *ctxp,
        int64	                *bus_type,
        pin_flist_t             *in_flist,
	pin_flist_t		*out_flist,
        pin_errbuf_t            *ebufp);

static int 
fm_mso_update_bb_service(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		*out_flist,
	pin_errbuf_t		*ebufp);

extern void
fm_mso_cust_get_bp_obj(
        pcm_context_t           *ctxp,
        poid_t                  *account_obj,
        poid_t                  *service_obj,
	int			mso_status,
        poid_t                  **bp_obj,
        poid_t                  **mso_obj,
        pin_errbuf_t            *ebufp);
		
static void 
fm_mso_get_outstanding_bal(
	pcm_context_t		*ctxp,
	poid_t		*acc_pdp,
	char			*bi_type,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static int 
fm_mso_create_lco_change_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**comm_err_flist,
	pin_errbuf_t		*ebufp);

extern void
fm_mso_search_devices(
        pcm_context_t                   *ctxp,
        char                            *param,
        poid_t                          *service_obj,
        pin_flist_t                     **deviceflistp,
        int                             *error_codep,
        pin_errbuf_t                    *ebufp);

extern void
get_evt_lifecycle_template(
	pcm_context_t			*ctxp,
	int64				db,
	int32				string_id, 
	int32				string_version,
	char				**lc_template, 
	pin_errbuf_t			*ebufp);

static void 
fm_mso_update_device_lco(
	pcm_context_t		*ctxp,
	poid_t			*acct_pdp,
	poid_t			*lco_obj,
	int32			den_hw_flag,
	pin_errbuf_t		*ebufp);

void
fm_mso_get_splitted_name(
	pcm_context_t		*ctxp,
	char			*name,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);
		
static void
fm_mso_update_user_status(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,	
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp);


void
fm_mso_get_mso_cust_credentials(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_get_lco_info(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static int 
fm_mso_update_salesman(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		*out_flist,
	pin_errbuf_t		*ebufp);

static int 
fm_mso_update_bu_profile(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		*out_flist,
	pin_errbuf_t		*ebufp);

int32
fm_mso_get_cust_of_lco_count(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	int32			*r_flist,
	pin_errbuf_t		*ebufp);

static int
fm_mso_bus_type_change(
	pcm_context_t           *ctxp,
	int64	                *bus_type,
	int64	                *old_bus_type,
	pin_flist_t             *in_flistp,
	pin_flist_t             *out_flistp,	
	pin_errbuf_t            *ebufp);
static int
fm_mso_get_serv_type(
        pcm_context_t           *ctxp,
        poid_t                  *acct_pdp,
        pin_errbuf_t            *ebufp);	
static int
fm_mso_cust_cmts_ip_change(
	pcm_context_t		*ctxp,
	poid_t			*account_obj,
	poid_t			*service_obj,
	poid_t			*mso_obj,
	char			*ne,
	char			*old_ne,
	char			*prog_name,
	char			**login,
	pin_errbuf_t		*ebufp);

int
fm_mso_cust_release_service_ip(
	pcm_context_t		*ctxp,
	poid_t			*account_obj,
	poid_t			*service_obj,
	char			*prog_name,
	int			*count,
	char			**login,
	pin_errbuf_t		*ebufp);

static int
fm_mso_cust_dup_cntinfo(
	pcm_context_t		*ctxp,
	pin_flist_t		*cnt_flistp,
	pin_flist_t		*w_in_flistp,
	char			*new_val,
	time_t			current_t,
	pin_errbuf_t		*ebufp);

static int 
fm_mso_modify_prod_discount_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		*out_flist,
	pin_errbuf_t		*ebufp);
	
void
fm_mso_modify_netflix_service (
        pcm_context_t   *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t    *ebufp);

void fm_mso_modify_addon_services (
        pcm_context_t   *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t    *ebufp);

/**************************************
* Local Functions end
***************************************/

/*******************************************************************
 * MSO_OP_CUST_MODIFY_CUSTOMER 
 *
 * This policy provides a hooks for enhancing OP_ACT_POL_AUTHORIZE
 * before aclling the actual opcode.
 *******************************************************************/

EXPORT_OP void
op_mso_cust_modify_customer(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);
	

static void 
fm_mso_populate_mod_tax_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	int32			account_model,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);
	
static void
fm_mso_cust_modify_customer(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

extern void
fm_mso_create_ar_limit(
        pcm_context_t           *ctxp,
        poid_t                  *account_obj,
        pin_flist_t             **ar_limit,
        pin_errbuf_t            *ebufp);

static void
fm_mso_update_region_key(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flist,
        pin_errbuf_t            *ebufp);

static int32
fm_mso_technology_change(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flist,
        pin_errbuf_t            *ebufp);

static int32
fm_mso_get_services(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flist,
        pin_errbuf_t            *ebufp);

extern void
fm_mso_get_poid_details(
        pcm_context_t           *ctxp,
        poid_t                  *poid_pdp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_mso_cust_modify_gst_info(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

PIN_IMPORT int
fm_mso_cust_get_gst_profile(
        pcm_context_t           *ctxp,
        poid_t                  *account_pdp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

PIN_IMPORT void
fm_mso_search_zip_lco_account(
	pcm_context_t		*ctxp,
	char			*zip,
	int32			new_cust,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_update_lco_contact(
        pcm_context_t           *ctxp,
	poid_t			*acct_obj,
        char			*rmn,
	char			*rmail,
        pin_errbuf_t            *ebufp);

void
fm_mso_get_eprofile_offer(
	pcm_context_t		*ctxp,
	poid_t			*ser_pdp,
	char 			*ofr_desc,
	pin_flist_t		**ret_flist,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * MSO_OP_CUST_MODIFY_CUSTOMER  
 *******************************************************************/
void 
op_mso_cust_modify_customer(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	*r_flistpp		= NULL;
        pin_flist_t             *addon_r_flistpp        = NULL;
        pin_flist_t             *srch_flistp        = NULL;
        pin_flist_t             *srch_out_flistp        = NULL;
        pin_flist_t             *result_flist = NULL;
	int			local = LOCAL_TRANS_OPEN_SUCCESS;
	int			*status = NULL;
	int32			status_uncommitted =2;
	int32			srch_flag = 122;
	int32			*t_openp = NULL;
	int64			db = -1;
	poid_t			*inp_pdp = NULL;
	char            *prog_name = NULL;
	char		*stb_idp = NULL;
    char        *account_no_str = NULL;        
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_MODIFY_CUSTOMER) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_modify_customer error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_modify_customer input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 ******************************************************************
	inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	local = fm_mso_trans_open(ctxp, inp_pdp, READWRITE,ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Err before calling fm_mso_cust_modify_customer()", ebufp);
		return;
	}*/

	t_openp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_TRANS_DONE, 1, ebufp);
	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp));
	stb_idp = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_STB_ID, 1, ebufp );
    account_no_str = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_LEGACY_ACCOUNT_NO, 1, ebufp);

	if (stb_idp)
	{
                srch_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, srch_flistp, PIN_FLD_USERID, ebufp);
                PIN_FLIST_FLD_SET(srch_flistp, MSO_FLD_SERIAL_NO, stb_idp, ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, srch_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
                PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);

                PCM_OP(ctxp, MSO_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
                PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

                result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
                if (result_flist)
                {
                        result_flist = PIN_FLIST_ELEM_GET(result_flist, PIN_FLD_SERVICES, 0, 1, ebufp);
                        if (result_flist)
                        {
                                PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_ACCOUNT_OBJ, i_flistp, PIN_FLD_POID, ebufp);
                                PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_ACCOUNT_OBJ, i_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
                                PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_SERVICE_OBJ, i_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
                        }
                }
		else
                {
                        PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Device serial number not found");
			*r_flistpp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_uncommitted, ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51430", ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Device serial number not found", ebufp);
                        return;
                }
		PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
        }

    if (account_no_str)
    {
        srch_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LEGACY_ACCOUNT_NO, srch_flistp, MSO_FLD_LEGACY_ACCOUNT_NO, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, srch_flistp, PIN_FLD_USERID, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, srch_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
        srch_flag = 13; 
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);

        PCM_OP(ctxp, MSO_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_suspend_service get_subscriber flist", srch_out_flistp);

        result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
        if (result_flist)
        {
            PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_POID, i_flistp, PIN_FLD_POID, ebufp);
            PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_POID, i_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
        }
        else
        {
            *r_flistpp = PIN_FLIST_CREATE(ebufp);
            PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
            PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_uncommitted, ebufp);
            PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51437", ebufp);
            PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Jio customer ID passed not found", ebufp);
            return;
        }
		PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
    }
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_modify_customer enriched input flist", i_flistp);
	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
//	if (!(prog_name && strcmp(prog_name,"pin_deferred_act") ==0 ))
        if (!(prog_name && strstr(prog_name,"pin_deferred_act")))
	{
                if (PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp) && PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp) != NULL)
		  inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
		else	
		  inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
		if (!t_openp)
		{
			local = fm_mso_trans_open(ctxp, inp_pdp, 3,ebufp);
		}
		else
		{
			PIN_FLIST_FLD_DROP(i_flistp, PIN_FLD_TRANS_DONE, ebufp);
		}
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
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Called by 'pin_deferred_act' so transaction will not be handled at API level");
	}

	fm_mso_cust_modify_customer(ctxp, flags, i_flistp, r_flistpp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"fm_mso_cust_modify_customer output flist", *r_flistpp);
	status = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_STATUS, 1, ebufp);

        /**Check for addon services like netflix **/
        fm_mso_modify_addon_services(ctxp, i_flistp, &addon_r_flistpp, ebufp);
        status = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_STATUS, 1, ebufp);

 	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp) || (status && *(int*)status == 1)) 
	{		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "fm_mso_cust_modify_customer in flist", i_flistp);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "op_mso_cust_modify_customer error", ebufp);
		if(local == LOCAL_TRANS_OPEN_SUCCESS )
		{
			fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
		}
	}
	else
	{
		if(local == LOCAL_TRANS_OPEN_SUCCESS  && 
                        (!(prog_name && strstr(prog_name,"pin_deferred_act")))
		  )
		{
			fm_mso_trans_commit(ctxp, inp_pdp, ebufp);
		}
		else
		{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Transaction NOT Comitted");
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_uncommitted, ebufp);
		}

		//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_modify_customer output flist", *r_flistpp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_modify_customer output flist", *r_flistpp);
	return;
}

/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_modify_customer(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*wholesale_flistp = NULL;
	pin_flist_t		*custcare_flistp = NULL;
	pin_flist_t		*out_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*child_i_flistp = NULL;
	pin_flist_t		*child_o_flistp = NULL;
	pin_flist_t		*billinfo = NULL;
	pin_flist_t		*members = NULL;	
	pin_flist_t		*acctinfo_flistp = NULL;	
        pin_flist_t             *customer_careinfo=NULL;
        pin_flist_t             *get_lco_flistp=NULL;
        pin_flist_t             *write_et_vat_zones_ret_flistp=NULL;	
	pin_flist_t		*accessinfo_flistp = NULL;	
	pin_flist_t		*arinfo_flistp = NULL;	
	pin_flist_t		*commission_err_flist = NULL;
	pin_flist_t		*update_po_in_flistp = NULL;
	pin_flist_t		*update_po_out_flistp = NULL;
	//DIGITAL OFFERS VARIABLES
	pin_flist_t		*profile_offer_flistp = NULL;
	pin_flist_t		*profile_offer_oflistp = NULL;
	pin_flist_t		*srch_offer_oflistp = NULL;
	pin_flist_t		*temp_flistp = NULL;
	pin_flist_t		*rdfld_flistp = NULL;
	pin_flist_t		*rdfld_oflistp = NULL;

	poid_t			*routing_poid = NULL;
	poid_t			*account_obj = NULL;
	poid_t			*user_id = NULL;
	poid_t			*last_bill_obj = NULL;
	poid_t			*lco_acct_pdp = NULL;
	poid_t			*account_pdp = NULL;
	poid_t			*profile_pdp = NULL;
	
	char			*account_no_str = NULL;
	char			*area = NULL;
	char			*rmail = NULL;
	char			*rmn = NULL;
	char			*error_descr = NULL;
	char			*vat_zone = NULL;
	char			*et_zone = NULL;	
	int64 			*bus_type = NULL;

	int32			modify_customer_success = 0;
	int32			modify_customer_failure = 1;
	int32			account_type      = -1;
	int32			account_model     = -1;
	int32			subscriber_type   = -1;
	int32			elem_id = 0;
	int32			corp_child=CORPORATE_CHILD;
	int32			srch_flag = 768 ;
	int32			srvc_status_cancel = 10103;
	int32			pay_type_suboord = 10007;
	int32			*action_mode = NULL;
	int32			non_entity_change = 99;
	int32			corporate_type=0;
	int32			profile_srch_type = MSO_FLAG_SRCH_BY_SELF_ACCOUNT;

	int64			orig_bus_type = 0;
	int64			db = -1;
	int64			account_no_int = 0;
	int			csr_access = 0;
	int			acct_update = 0;
	int			cnt_upd = 0;
	int			*contact_pref = NULL;
	int			arr_business_type[8];

	pin_flist_t	        *acctinfo_ret_flistp=NULL;
        pin_flist_t	        *acct_info_flistp=NULL;
        pin_flist_t	        *write_et_vat_zones_flistp=NULL;
        pin_flist_t 		*lco_acctinfo_ret_flistp=NULL;
	pin_flist_t		*gst_info_flistp = NULL;
	pin_flist_t		*gst_out_flistp = NULL;
	pin_flist_t		*findprofile_inflistp = NULL;
	pin_flist_t		*findprofile_outflistp = NULL;
	pin_flist_t		*profile_in_flistp = NULL;
	pin_flist_t		*profile_out_flistp = NULL;
	pin_flist_t		*profile_flistp = NULL;
	pin_flist_t		*inherited_flistp = NULL;
	pin_flist_t		*wholsale_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;
	int32                   *business_type = NULL;
	int32                   tmp_business_type = -1;
	int32                   account_ret_type = -1;
	int32                   account_ret_model = -1;	
        char                    *area_code=NULL;
        char                    *area_code_lco=NULL;
	char			*msg = NULL;

	char			*poi = NULL;
	char			*poa = NULL;
	char			*curr_poi = NULL;
	char			*curr_poa = NULL;
	char			*tan_no = NULL;
	char			*cin_no = NULL;
	char			*lic_no = NULL;
	char			*vat_no = NULL;
	char			*et_no = NULL;
	char			*postal_no= NULL;
	char			*st_no = NULL;
	char			*pan_no = NULL;		
	char			*ic_agr_no = NULL;
    char            *lco_location = NULL;
    char            *lco_code = NULL;
    char            *lco_geocode = NULL;
    char            *lco_link_obj = NULL;
    char            *lco_equip = NULL;
 
	time_t			*ic_agr_t = NULL;
	time_t			*post_reg_t = NULL;
	time_t			*post_exp_t = NULL;
	time_t			*dob = NULL;
    time_t          *lco_agr_start_t = NULL;
    time_t          *lco_agr_end_t = NULL;
    time_t          *lco_renew_start_t = NULL;
    time_t          *lco_renew_end_t = NULL;

	pin_cookie_t		cookie = NULL;
	int 			tax_mod_failure=1;

	void			*vp = NULL;
	pin_flist_t		*results_data_flistp = NULL;

	char			*vendor = NULL;
	pin_flist_t		*write_addr_id_proof_iflistp = NULL;
	pin_flist_t		*write_addr_id_proof_oflistp = NULL;
	pin_flist_t		*read_i_flistp = NULL;
	pin_flist_t		*read_o_flistp = NULL;
	int			tech_change = 0;
	int32			nothing_to_change = 0;
	char			*action = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer input flist", i_flistp);	

	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp));

	routing_poid = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
	if (routing_poid && routing_poid != NULL)
	{	
		db = PIN_POID_GET_DB(routing_poid);
		account_obj = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
		user_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 1, ebufp); 
	}
	else
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		//PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                routing_poid = PIN_POID_CREATE(1 , "/account", -1, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, routing_poid , ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51400", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "POID value not passed in input as it is mandatory field", ebufp);
		goto CLEANUP;
	}

	if (!account_obj && account_obj != NULL)
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		//PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                routing_poid = PIN_POID_CREATE(1 , "/account", -1, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, routing_poid , ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51401", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account POID value not passed in input as it is mandatory field", ebufp);
		goto CLEANUP;
	}

	/*******************************************************************
	* Verifying whether customer initiated or CSR initiated  - Start
	*******************************************************************/
	
	out_flistp = PIN_FLIST_CREATE(ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(out_flistp, PIN_FLD_DATA_ARRAY, 0, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(out_flistp, PIN_FLD_DATA_ARRAY, 1, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer output flist", out_flistp);	


	/*******************************************************************
	* Verifying whether customer initiated or CSR initiated  - Start
	*******************************************************************/

	/*******************************************************************
	* Verifying whether customer initiated or CSR initiated  - Start
	*******************************************************************/

	/*******************************************************************
	* Verifying whether customer initiated or CSR initiated  - End
	*******************************************************************/

	/*******************************************************************
	* Validation for CSR roles - Start
	*******************************************************************/

	if (user_id && user_id != NULL)
	{
		csr_access = fm_mso_validate_csr_role(ctxp, db, user_id, ebufp);

		if ( csr_access == 0)
		{		
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51403", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR,"Provided CSR does not have permissions to operate the given input",ebufp);
			goto CLEANUP;
		}else 
		{		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_service_status CSR haas permission to change account information");	
		}
	}else
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51404", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "User ID value not passed in input as it is mandatory field", ebufp);
		goto CLEANUP;
	}
	
	/*******************************************************************
	* Validation for CSR roles  - End
	*******************************************************************/
	
	/*****************************************************************
	* POI/POA Modify
	*****************************************************************/

	action = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTION, 1, ebufp);

	if(action && (strcmp(action, "LCO_MODIFY") == 0))
	{
		/*
		r << 1 1
		0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 7085510847 46
		0 PIN_FLD_USERID         POID [0] 0.0.0.1 /account 1290705 8
		0 PIN_FLD_ACCOUNT_OBJ    POID [0] 0.0.0.1 /account 7085510847 46
		0 PIN_FLD_PROGRAM_NAME   STR [0] "OAP|csradmin"
		0 PIN_FLD_ACTION         STR [0] "LCO_MODIFY"
		0 MSO_FLD_TAN   		 STR [0] "TANNO"
		0 MSO_FLD_CIN   		 STR [0] "CINNO"
		0 MSO_FLD_LICENSE_NO     STR [0] "MSO_LIC_NO"
		0 MSO_FLD_LCO_IC_AGR_NO  STR [0] "LCO_IC_NO"
		0 MSO_FLD_IC_AGR_T   	TIMESTAMP [0] (1492972200)
		0 MSO_FLD_POST_REG_T   	TIMESTAMP [0] (1492972200)
		0 MSO_FLD_POST_EXP_T   	TIMESTAMP [0] (1492972200)
		0 MSO_FLD_DOB   		TIMESTAMP [0] (1492972200)
		0 MSO_FLD_ST_REG_NO                    STR [0] "TANNO"
		0 MSO_FLD_POSTAL_REG_NO                    STR [0] "TANNO"
		0 MSO_FLD_ENT_TAX_NO                    STR [0] "TANNO"
		0 MSO_FLD_PAN_NO                    STR [0] "TANNO"
		0 MSO_FLD_VAT_TAX_NO                    STR [0] "TANNO"
		0 MSO_FLD_RMN                    STR [0] "1111111111"
		0 MSO_FLD_RMAIL                    STR [0] "a@gmail.com"		
		1
		xop MSO_OP_CUST_MODIFY_CUSTOMER 0 1
		*/
		account_obj = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
		tan_no = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_TAN, 1, ebufp);
		cin_no = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_CIN, 1, ebufp);
		lic_no = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_LICENSE_NO, 1, ebufp);
		ic_agr_no = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_LCO_IC_AGR_NO, 1, ebufp);
		ic_agr_t = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_IC_AGR_T, 1, ebufp);
		post_reg_t = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_POST_REG_T, 1, ebufp);
		post_exp_t = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_POST_EXP_T, 1, ebufp);
		dob = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_DOB, 1, ebufp);
		vat_no = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_VAT_TAX_NO, 1, ebufp);
		st_no = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_ST_REG_NO, 1, ebufp);
		postal_no = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_POSTAL_REG_NO, 1, ebufp);
		et_no = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_ENT_TAX_NO, 1, ebufp);
		pan_no = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_PAN_NO, 1, ebufp);
		rmn = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_RMN, 1, ebufp);
		rmail = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_RMAIL, 1, ebufp);
        //adding for lco requirement
        lco_agr_start_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CYCLE_START_T, 1, ebufp);
        lco_agr_end_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CYCLE_END_T, 1, ebufp);
        lco_renew_start_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USAGE_START_T, 1, ebufp);
        lco_renew_end_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USAGE_END_T, 1, ebufp);
        lco_code = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_CODE, 1, ebufp);
        lco_location = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_LOCATION_NAME, 1, ebufp);
        lco_equip = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_EQUIP_TYPE, 1, ebufp);
        lco_geocode = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_GEOCODE, 1, ebufp);
        lco_link_obj = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_LINK_OBJECT, 1, ebufp);
		
		if (!tan_no && !cin_no && !lic_no && !ic_agr_no && !ic_agr_t && !post_reg_t  && !post_exp_t && !dob && !vat_no &&
			!st_no && !et_no && !postal_no && !pan_no && !rmn && !rmail && !lco_agr_start_t && !lco_agr_end_t && 
            !lco_renew_start_t && !lco_renew_end_t && !lco_code && !lco_location && !lco_equip && !lco_geocode && !lco_link_obj)
		{
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "52000", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "No Input received", ebufp);
			goto CLEANUP;
		}
		
		/************************************** ***************************************
		*      Search LCO wholesale profile
		******************************************************************************/

		if (tan_no || cin_no || lic_no || ic_agr_no || ic_agr_t || post_reg_t || post_exp_t || dob || vat_no || 
                         st_no || et_no || postal_no || pan_no || lco_agr_start_t || lco_agr_end_t || lco_renew_start_t ||
                         lco_renew_end_t || lco_code || lco_location || lco_equip || lco_geocode || lco_link_obj)
                {
			findprofile_inflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(findprofile_inflistp, PIN_FLD_POID, account_obj, ebufp);
			PIN_FLIST_FLD_SET(findprofile_inflistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
			PIN_FLIST_FLD_SET(findprofile_inflistp, PIN_FLD_TYPE_STR, "/profile/wholesale", ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "LCO_MODIFY :  find profile input list", findprofile_inflistp);
			PCM_OP(ctxp, PCM_OP_CUST_FIND_PROFILE , PCM_OPFLG_READ_RESULT, findprofile_inflistp, &findprofile_outflistp, ebufp);
			PIN_FLIST_DESTROY_EX(&findprofile_inflistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "LCO_MODIFY : Error in calling PCM_OP_CUST_FIND_PROFILE for LCO", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "52001", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error Reading Profile", ebufp);
				goto CLEANUP;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "LCO_MODIFY: output flist", findprofile_outflistp);

			results_flistp = PIN_FLIST_ELEM_GET(findprofile_outflistp, PIN_FLD_RESULTS, 0, 1, ebufp );

			if (!results_flistp && results_flistp != NULL)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lcoinfo: No LCO profile found!!");
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "52002", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "No LCO profile found", ebufp);
				goto CLEANUP;
			}

			profile_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_POID, 0 , ebufp);
			profile_in_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(profile_in_flistp, PIN_FLD_POID, profile_pdp, ebufp);
			PIN_FLIST_FLD_SET(profile_in_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
			profile_flistp = PIN_FLIST_ELEM_ADD(profile_in_flistp, PIN_FLD_PROFILES, 0, ebufp);
			PIN_FLIST_FLD_SET(profile_flistp, PIN_FLD_PROFILE_OBJ, profile_pdp, ebufp);
			inherited_flistp = PIN_FLIST_SUBSTR_ADD(profile_flistp, PIN_FLD_INHERITED_INFO, ebufp);
			wholsale_flistp = PIN_FLIST_SUBSTR_ADD(inherited_flistp, MSO_FLD_WHOLESALE_INFO, ebufp);
			PIN_FLIST_FLD_SET(wholsale_flistp, PIN_FLD_POID, account_obj, ebufp);
			if(tan_no && tan_no != NULL)
				PIN_FLIST_FLD_SET(wholsale_flistp, MSO_FLD_TAN, tan_no, ebufp);
			if(cin_no && cin_no != NULL)
				PIN_FLIST_FLD_SET(wholsale_flistp, MSO_FLD_CIN, cin_no, ebufp);
			if(lic_no && lic_no != NULL)
				PIN_FLIST_FLD_SET(wholsale_flistp, MSO_FLD_LICENSE_NO, lic_no, ebufp);
			if(ic_agr_no && ic_agr_no != NULL)
				PIN_FLIST_FLD_SET(wholsale_flistp, MSO_FLD_LCO_IC_AGR_NO, ic_agr_no, ebufp);
			if(ic_agr_t && ic_agr_t != NULL)
				PIN_FLIST_FLD_SET(wholsale_flistp, MSO_FLD_IC_AGR_T, ic_agr_t, ebufp);
			if(post_reg_t && post_reg_t != NULL)
				PIN_FLIST_FLD_SET(wholsale_flistp, MSO_FLD_POST_REG_T, post_reg_t, ebufp);
			if(post_exp_t && post_exp_t != NULL)
				PIN_FLIST_FLD_SET(wholsale_flistp, MSO_FLD_POST_EXP_T, post_exp_t, ebufp);
			if(dob && dob != NULL)
				PIN_FLIST_FLD_SET(wholsale_flistp, MSO_FLD_DOB, dob, ebufp);
			if(vat_no && vat_no != NULL)
				PIN_FLIST_FLD_SET(wholsale_flistp, MSO_FLD_VAT_TAX_NO, vat_no, ebufp);
			if(st_no && st_no != NULL)
				PIN_FLIST_FLD_SET(wholsale_flistp, MSO_FLD_ST_REG_NO, st_no, ebufp);
			if(postal_no && postal_no != NULL)
				PIN_FLIST_FLD_SET(wholsale_flistp, MSO_FLD_POSTAL_REG_NO, postal_no, ebufp);
			if(et_no && et_no != NULL)
				PIN_FLIST_FLD_SET(wholsale_flistp, MSO_FLD_ENT_TAX_NO, et_no, ebufp);
			if(pan_no && pan_no != NULL)
				PIN_FLIST_FLD_SET(wholsale_flistp, MSO_FLD_PAN_NO, pan_no, ebufp);
            //adding for lco requirement
            if(lco_agr_start_t && lco_agr_start_t != NULL)
                PIN_FLIST_FLD_SET(wholsale_flistp, PIN_FLD_CYCLE_START_T, lco_agr_start_t, ebufp);
            if(lco_agr_end_t && lco_agr_end_t != NULL)
                PIN_FLIST_FLD_SET(wholsale_flistp, PIN_FLD_CYCLE_END_T, lco_agr_end_t, ebufp);
            if(lco_renew_start_t && lco_renew_start_t != NULL)
                PIN_FLIST_FLD_SET(wholsale_flistp, PIN_FLD_USAGE_START_T, lco_renew_start_t, ebufp);
            if(lco_renew_end_t && lco_renew_end_t != NULL)
                PIN_FLIST_FLD_SET(wholsale_flistp, PIN_FLD_USAGE_END_T, lco_renew_end_t, ebufp);
            if(lco_code && lco_code != NULL)
                PIN_FLIST_FLD_SET(wholsale_flistp, PIN_FLD_ACCOUNT_CODE, lco_code, ebufp);
            if(lco_location && lco_location != NULL)
                PIN_FLIST_FLD_SET(wholsale_flistp, MSO_FLD_LOCATION_NAME, lco_location, ebufp);
            if(lco_equip && lco_equip != NULL)
                PIN_FLIST_FLD_SET(wholsale_flistp, PIN_FLD_EQUIP_TYPE, lco_equip, ebufp);
            if(lco_geocode && lco_geocode != NULL)
                PIN_FLIST_FLD_SET(wholsale_flistp, PIN_FLD_GEOCODE, lco_geocode, ebufp);
            if(lco_link_obj && lco_link_obj != NULL)
                PIN_FLIST_FLD_SET(wholsale_flistp, PIN_FLD_LINK_OBJECT, lco_link_obj, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Modify Profile input flist", profile_in_flistp);
			PCM_OP(ctxp, PCM_OP_CUST_MODIFY_PROFILE, 0, profile_in_flistp, &profile_out_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&profile_in_flistp, NULL);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Modify Profile output flist", profile_out_flistp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "52003", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in updating LCO details", ebufp);
				goto CLEANUP;
			}
		}
		if(rmn || rmail)
		{
			PIN_ERR_LOG_MSG(3, "Update RMN");
			fm_mso_update_lco_contact(ctxp, account_obj, rmn, rmail, ebufp);
		}
			
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_success, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "00", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Modified LCO Details Successfully", ebufp);
		goto CLEANUP;
	}

	if(action && (strcmp(action, "POI_POA_CHANGE") == 0))
	{
		poi = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCESS_CODE1, 1, ebufp);
		poa = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCESS_CODE2, 1, ebufp);
		if (!poi && !poa)
		{
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "52000", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "No Input received", ebufp);
			goto CLEANUP;
		}

		fm_mso_get_account_info(ctxp, account_obj, &acnt_info, ebufp);
		if(PIN_ERRBUF_IS_ERR(ebufp) || !acnt_info)
		{
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "52005", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account not found", ebufp);
			goto CLEANUP;
		}

		curr_poi = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCESS_CODE1, 1, ebufp);
		curr_poa = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCESS_CODE2, 1, ebufp);

		if(curr_poi && curr_poa && poi && poa && strcmp(poi, curr_poi) == 0 && strcmp(poa, curr_poa) == 0)
		{
			nothing_to_change = 1;
		}
		else if (!poa && poi && curr_poi &&  strcmp(poi, curr_poi) == 0)
		{
			nothing_to_change = 1;
		}
		else if(!poi && curr_poa && poa && strcmp(poa, curr_poa) == 0)
		{
			nothing_to_change = 1;
		}

		if(nothing_to_change)
		{
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "52001", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "POI/POA both are same as current value, Nothing to Modify", ebufp);
			goto CLEANUP;
		}


		update_po_in_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(update_po_in_flistp, PIN_FLD_POID, account_obj, ebufp);
		if(poi && poi != NULL)
			PIN_FLIST_FLD_SET(update_po_in_flistp, PIN_FLD_ACCESS_CODE1, poi, ebufp);
		if(poa && poa != NULL)
			PIN_FLIST_FLD_SET(update_po_in_flistp, PIN_FLD_ACCESS_CODE2, poa, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "update input flist", update_po_in_flistp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 32, update_po_in_flistp, &update_po_out_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&update_po_in_flistp, NULL);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "update output flist", update_po_out_flistp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "52002", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in updating POI/POA details", ebufp);
			goto CLEANUP;
		}
		fm_mso_create_lifecycle_evt(ctxp, i_flistp, acnt_info, ID_UPDATE_POI_POA, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "52003", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in creating lifecycle", ebufp);
			goto CLEANUP;
		}
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_success, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "00", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Modified POI/POA Details Successfully", ebufp);
		goto CLEANUP;
	}
	/*****************************************************************
	* Technology Change
	*****************************************************************/
	action = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTION, 1, ebufp);
	if(action && (strcmp(action, "TECH_CHANGE") == 0))
	//if(PIN_FLIST_ELEM_COUNT(i_flistp,PIN_FLD_DATA_ARRAY,ebufp) == 2)
	{
		tech_change = 1;
		read_i_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(read_i_flistp, PIN_FLD_POID, account_obj, ebufp);
                PIN_FLIST_FLD_SET(read_i_flistp, PIN_FLD_ACCOUNT_NO, NULL, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Account Read Flds Input ", read_i_flistp);
                PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_i_flistp, &read_o_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Account Read Flds Output ", read_o_flistp);
		PIN_FLIST_DESTROY_EX(&read_i_flistp, NULL);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
			ret_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51610", ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_technology_change - Error in Readflds of account_no", ebufp);
                        goto CLEANUP;
                }
		PIN_FLIST_FLD_COPY(read_o_flistp, PIN_FLD_ACCOUNT_NO, i_flistp, PIN_FLD_ACCOUNT_NO, ebufp );
		PIN_FLIST_DESTROY_EX(&read_o_flistp, NULL);	
		acct_update = fm_mso_technology_change(ctxp, i_flistp, ebufp);	
		if(acct_update == 0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error while updating service technology");
			ret_flistp = PIN_FLIST_CREATE(ebufp);
                	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
                	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51610", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_technology_change - Error in updating service technology", ebufp);
			goto CLEANUP;
		}
		else if(acct_update == 1)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error where both technologies are same ");
			ret_flistp = PIN_FLIST_CREATE(ebufp);
                	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
                	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51620", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_technology_change:Error either both technologies are same or isssue in input flist", ebufp);
			goto CLEANUP;
		}
		else if(acct_update == 2)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Service Technology change Succesful");
			fm_mso_create_lifecycle_evt(ctxp, i_flistp, i_flistp, ID_TECHNOLOGY_CHANGE, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
                	{
                        	ret_flistp = PIN_FLIST_CREATE(ebufp);
                        	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                        	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
                        	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51740", ebufp);
                        	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in updating lifecycle event", ebufp);
                        	goto CLEANUP;
                	}
			goto RETURN_FLIST;
		}
		else if(acct_update == 3)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error before calling function fm_mso_technology_change");
			ret_flistp = PIN_FLIST_CREATE(ebufp);
                	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
                	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51630", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_technology_change - Error before calling this function", ebufp);
			goto CLEANUP;
		}
		else if(acct_update == 4)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Not a valid case to change technology");
                        ret_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51630", ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_technology_change - Error Not a valid status to change technology", ebufp);
                        goto CLEANUP;
		}
		else if(acct_update == 5)
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "chnage in old and new technology  is not applicable");
                        ret_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51630", ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_technology_change - Error technology change is not applicable", ebufp);
                        goto CLEANUP;
                }
		else if(acct_update == 6)
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Wrong technology in input flist");
                        ret_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51630", ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_technology_change - Error Mismatch between service and old tech in input", ebufp);
                        goto CLEANUP;
                }
	}
	/******************************************************************
	* Updatation of GST Profile Start
	******************************************************************/
	gst_info_flistp = PIN_FLIST_ELEM_GET(i_flistp, MSO_FLD_GST_INFO, PIN_ELEMID_ANY, 1, ebufp);
	if (gst_info_flistp && gst_info_flistp != NULL)
	{
		fm_mso_cust_modify_gst_info(ctxp, i_flistp, &gst_out_flistp, ebufp);
               	if (PIN_ERRBUF_IS_ERR(ebufp) || gst_out_flistp == NULL)
                {
                        ret_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51002", ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Modify GST Info Failed", ebufp);
                        goto CLEANUP;
                }
		ret_flistp = PIN_FLIST_COPY(gst_out_flistp, ebufp);
		goto CLEANUP;
	}
	/*******************************************************************
	* Updation of GST Profile End
	*******************************************************************/

 	/*******************************************************************
	* Set PIN_FLD_BUSINESS_TYPE in inputflist  - Start
	*******************************************************************/
	fm_mso_get_account_info(ctxp, account_obj, &acnt_info, ebufp);
	//fm_mso_get_business_type_values(ctxp, *(int32*)PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_BUSINESS_TYPE, 0, ebufp), 
	//                        &account_type, &account_model, &subscriber_type, ebufp );

	vp =  (int32*)PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_BUSINESS_TYPE, 0, ebufp);
	if (vp && vp != NULL)
	{
		tmp_business_type = *(int32*)vp;
	}

	num_to_array(tmp_business_type, arr_business_type);
	account_type    = 10*(arr_business_type[0])+arr_business_type[1];// First 2 digits
	subscriber_type = 10*(arr_business_type[2])+arr_business_type[3];// next 2 digits
	account_model   = arr_business_type[4];				// 5th field
	corporate_type  = arr_business_type[6];				// 7th field
	
	PIN_FLIST_FLD_SET(i_flistp, MSO_FLD_SUBSCRIBER_TYPE, &subscriber_type, ebufp );
	PIN_FLIST_FLD_SET(i_flistp, MSO_FLD_CORPORATE_TYPE, &corporate_type, ebufp );
	/* Change the status of Business User - Start
		To be called only for Business User*/
    if (account_type==ACCT_TYPE_CSR_ADMIN || account_type==ACCT_TYPE_CSR || account_type==ACCT_TYPE_OPERATION 
		|| account_type==ACCT_TYPE_FIELD_SERVICE || account_type==ACCT_TYPE_COLLECTION_AGENT
		|| account_type==ACCT_TYPE_SALES_PERSON || account_type==ACCT_TYPE_PRE_SALES || account_type==ACCT_TYPE_SUBSCRIBER || account_type==ACCT_TYPE_RETAIL_BB)
	{
		fm_mso_update_user_status(ctxp, i_flistp, out_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51705", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Failed to update accout status.", ebufp);
			goto CLEANUP;
		}
	}
	
	/* Change the status of Business User - End */
	/*******************************************************************
	* Billing and installation address change - Start
	*******************************************************************/
	
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "@@tax input flist", i_flistp);

	customer_careinfo = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_CUSTOMER_CARE_INFO, 1, ebufp);
        if(customer_careinfo && customer_careinfo != NULL)
        {
                lco_acct_pdp = PIN_FLIST_FLD_GET(customer_careinfo,MSO_FLD_LCO_OBJ, 1, ebufp);
        	if (lco_acct_pdp && lco_acct_pdp != NULL)
        	{
                    // Read AREA of LCO
                    get_lco_flistp = PIN_FLIST_CREATE(ebufp);
        	    PIN_FLIST_FLD_SET(get_lco_flistp,PIN_FLD_POID,lco_acct_pdp, ebufp );  
		    PCM_OP(ctxp, PCM_OP_READ_OBJ, 0,get_lco_flistp, &lco_acctinfo_ret_flistp, ebufp);

                    PIN_FLIST_DESTROY_EX(&get_lco_flistp,NULL);
        	    area_code_lco = PIN_FLIST_FLD_GET(lco_acctinfo_ret_flistp,MSO_FLD_AREA, 1, ebufp);
        	}
		else
		{
			srch_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_POID, account_obj, ebufp);
			PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_PROFILE_DESCR, "/profile/customer_care", ebufp);
			PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_SEARCH_TYPE, &profile_srch_type, ebufp);
			PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_OBJECT, account_obj, ebufp);

			fm_mso_get_profile_info(ctxp, srch_flistp, &srch_out_flistp, ebufp );
			PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

			if (srch_out_flistp)
			{
				if(PIN_FLIST_FLD_GET(customer_careinfo, PIN_FLD_DOC_NO, 1, ebufp) || PIN_FLIST_FLD_GET(customer_careinfo, PIN_FLD_AUTH_CODE, 1, ebufp) || PIN_FLIST_FLD_GET(customer_careinfo, PIN_FLD_AUTH_DATE, 1, ebufp) || PIN_FLIST_FLD_GET(customer_careinfo, MSO_FLD_OTP_STATUS, 1, ebufp))
				{

						profile_offer_flistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_COPY(srch_out_flistp, PIN_FLD_POID, profile_offer_flistp, PIN_FLD_POID, ebufp);	
						PIN_FLIST_FLD_SET(profile_offer_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);

						profile_flistp = PIN_FLIST_ELEM_ADD(profile_offer_flistp, PIN_FLD_PROFILES, 0, ebufp);
						PIN_FLIST_FLD_COPY(srch_out_flistp, PIN_FLD_POID, profile_flistp, PIN_FLD_PROFILE_OBJ, ebufp);

						inherited_flistp = PIN_FLIST_SUBSTR_ADD(profile_flistp, PIN_FLD_INHERITED_INFO, ebufp);
						wholsale_flistp = PIN_FLIST_SUBSTR_ADD(inherited_flistp, PIN_FLD_CUSTOMER_CARE_INFO, ebufp);


						vp = PIN_FLIST_FLD_GET(customer_careinfo, PIN_FLD_DOC_NO, 1, ebufp);
						if (vp) PIN_FLIST_FLD_SET(wholsale_flistp, PIN_FLD_DOC_NO, vp, ebufp);
						
						vp = PIN_FLIST_FLD_GET(customer_careinfo, PIN_FLD_AUTH_CODE, 1, ebufp);
						if (vp) PIN_FLIST_FLD_SET(wholsale_flistp, PIN_FLD_AUTH_CODE, vp, ebufp);

						vp = PIN_FLIST_FLD_GET(customer_careinfo, PIN_FLD_AUTH_DATE, 1, ebufp);
						if (vp) PIN_FLIST_FLD_SET(wholsale_flistp, PIN_FLD_AUTH_DATE, vp, ebufp);

						vp = PIN_FLIST_FLD_GET(customer_careinfo, MSO_FLD_OTP_STATUS, 1, ebufp);
						if (vp) PIN_FLIST_FLD_SET(wholsale_flistp, MSO_FLD_OTP_STATUS, vp, ebufp);

						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Modify Profile input flist for OTP", profile_offer_flistp);
									PCM_OP(ctxp, PCM_OP_CUST_MODIFY_PROFILE, 0, profile_offer_flistp, &profile_offer_oflistp, ebufp);
									PIN_FLIST_DESTROY_EX(&profile_offer_flistp, NULL);
						
						 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Modify Profile output flist for OTP", profile_offer_oflistp);
						if (PIN_ERRBUF_IS_ERR(ebufp))
						{
							ret_flistp = PIN_FLIST_CREATE(ebufp);
											PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
											PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
											PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "52003", ebufp);
											PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in updating OTP details", ebufp);
											goto CLEANUP;
						}

									PIN_FLIST_DESTROY_EX(&profile_offer_oflistp, NULL);
									PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
				}
			}
		}
        }
        else
        {
                area_code_lco = PIN_FLIST_FLD_GET(i_flistp,MSO_FLD_AREA, 1, ebufp);
        }

	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, i_flistp, &acctinfo_ret_flistp, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "@@tax ret flist",acctinfo_ret_flistp);
	business_type = PIN_FLIST_FLD_GET(acctinfo_ret_flistp, PIN_FLD_BUSINESS_TYPE, 1, ebufp);
	orig_bus_type = *business_type;
	tmp_business_type = *business_type;
	tmp_business_type = tmp_business_type/1000;
	account_ret_model = tmp_business_type%10;

        acctinfo_flistp = PIN_FLIST_ELEM_ADD(i_flistp,PIN_FLD_ACCTINFO, 0, ebufp);
        //area_code = PIN_FLIST_FLD_GET(acctinfo_ret_flistp,MSO_FLD_AREA, 1, ebufp);
        area_code = PIN_FLIST_FLD_GET(i_flistp,MSO_FLD_AREA, 1, ebufp);
	if(area_code != NULL && strcmp(area_code, "") != 0)
        {
                PIN_FLIST_FLD_SET(acctinfo_flistp,MSO_FLD_AREA,area_code, ebufp );  
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "@@tax inp flist",i_flistp);
		if(account_type == ACCT_TYPE_SUBSCRIBER)
                {
                        /*******************************************************************
                        * Call tax code population-Start
                        *******************************************************************/
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "@@tax input flist", i_flistp);
				
			fm_mso_populate_mod_tax_info(ctxp, i_flistp,account_ret_model,&ret_flistp, ebufp);
                        // fm_mso_populate_tax_info(ctxp, i_flistp, account_model, &ret_flistp, ebufp) ;
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_populate_tax_info input flist", i_flistp);
			PIN_FLIST_DESTROY_EX(&acctinfo_ret_flistp,NULL);
                        if (ret_flistp && ret_flistp != NULL)
                        {
                             PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                             PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &tax_mod_failure, ebufp);
			     PIN_FLIST_DESTROY_EX(&acctinfo_ret_flistp,NULL);
                             goto CLEANUP;
                        }
		}
		else if (account_type == ACCT_TYPE_LCO)
                {
                        //if LCO account
                        //Case LCO change

                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_populate_tax_info input flist after", i_flistp);


                        acct_info_flistp = PIN_FLIST_ELEM_GET(i_flistp,PIN_FLD_ACCTINFO,PIN_ELEMID_ANY, 1, ebufp);

                        if(acct_info_flistp && acct_info_flistp != NULL)
                        {
                                write_et_vat_zones_flistp = PIN_FLIST_CREATE(ebufp);

                                account_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
                                area_code_lco = PIN_FLIST_FLD_GET(acct_info_flistp,MSO_FLD_AREA, 1, ebufp);

                                if(area_code_lco &&  account_pdp)
                                {
                                        PIN_FLIST_FLD_SET(write_et_vat_zones_flistp,PIN_FLD_POID,account_pdp, ebufp );
                                        PIN_FLIST_FLD_SET(write_et_vat_zones_flistp,MSO_FLD_AREA,area_code_lco, ebufp );
                                        PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0,write_et_vat_zones_flistp,&write_et_vat_zones_ret_flistp, ebufp);
                                }
                                PIN_FLIST_DESTROY_EX(&write_et_vat_zones_flistp,NULL);
                                PIN_FLIST_DESTROY_EX(&write_et_vat_zones_ret_flistp,NULL);
                        }
                        PIN_FLIST_DESTROY_EX(&acctinfo_ret_flistp,NULL);
                }

		
        //PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "@@tax input flist",i_flistp );
        //fm_mso_populate_tax_info(ctxp, i_flistp,account_ret_model, &ret_flistp, ebufp) ;
        //PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "@@tax input flist", i_flistp);
        //PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "@@tax output flist", ret_flistp);
        }
	else if(area_code_lco != NULL && strcmp(area_code_lco, "") != 0)
        {

	     PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, i_flistp, &acctinfo_ret_flistp, ebufp);

             PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "@@tax ret flist lco",acctinfo_ret_flistp);
		business_type = PIN_FLIST_FLD_GET(acctinfo_ret_flistp, PIN_FLD_BUSINESS_TYPE, 1, ebufp);
		tmp_business_type = *business_type;
		tmp_business_type = tmp_business_type/1000;
		account_ret_model = tmp_business_type%10;

        	acctinfo_flistp = PIN_FLIST_ELEM_ADD(i_flistp,PIN_FLD_ACCTINFO, 0, ebufp);

        	PIN_FLIST_FLD_SET(acctinfo_flistp,MSO_FLD_AREA,area_code_lco, ebufp );  

		if(account_type == ACCT_TYPE_SUBSCRIBER)
                {
                      //if LCO account
                      //Case LCO change  
                     
                      /*******************************************************************
                      * Call tax code population-Start
                      *******************************************************************/
		       PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_populate_tax_info input flist before", i_flistp);
				
		       fm_mso_populate_mod_tax_info(ctxp, i_flistp,account_ret_model,&ret_flistp, ebufp);
                       // fm_mso_populate_tax_info(ctxp, i_flistp, account_model, &ret_flistp, ebufp) ;
                       PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_populate_tax_info input flist", i_flistp);

                                                              
 		    acct_info_flistp = PIN_FLIST_ELEM_GET(i_flistp,PIN_FLD_ACCTINFO,PIN_ELEMID_ANY, 1, ebufp);
                 
                    if(acct_info_flistp && acct_info_flistp != NULL)                  
                    { 
                    write_et_vat_zones_flistp = PIN_FLIST_CREATE(ebufp);
    
		    account_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
		    et_zone = PIN_FLIST_FLD_GET(acct_info_flistp,MSO_FLD_ET_ZONE, 1, ebufp);
		    vat_zone = PIN_FLIST_FLD_GET(acct_info_flistp,MSO_FLD_VAT_ZONE, 1, ebufp);

                    if(et_zone && vat_zone && account_pdp != NULL)
                    {
        	    PIN_FLIST_FLD_SET(write_et_vat_zones_flistp,PIN_FLD_POID,account_pdp, ebufp );  
        	    PIN_FLIST_FLD_SET(write_et_vat_zones_flistp,MSO_FLD_ET_ZONE,et_zone, ebufp );  
        	    PIN_FLIST_FLD_SET(write_et_vat_zones_flistp,MSO_FLD_AREA,area_code_lco, ebufp );  
        	    PIN_FLIST_FLD_SET(write_et_vat_zones_flistp,MSO_FLD_VAT_ZONE,vat_zone, ebufp );  
		    PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0,write_et_vat_zones_flistp,&write_et_vat_zones_ret_flistp, ebufp);
                    }

                    PIN_FLIST_DESTROY_EX(&write_et_vat_zones_flistp,NULL);
                    PIN_FLIST_DESTROY_EX(&write_et_vat_zones_ret_flistp,NULL);

                    }
				PIN_FLIST_DESTROY_EX(&acctinfo_ret_flistp,NULL);
                if (ret_flistp && ret_flistp != NULL)
                {
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &tax_mod_failure, ebufp);
			PIN_FLIST_DESTROY_EX(&acctinfo_ret_flistp,NULL);
                        goto CLEANUP;
                }
	}
        else if (account_type == ACCT_TYPE_LCO)
        {
              //if LCO account
              //Case LCO change

                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_populate_tax_info input flist after", i_flistp);


                        acct_info_flistp = PIN_FLIST_ELEM_GET(i_flistp,PIN_FLD_ACCTINFO,PIN_ELEMID_ANY, 1, ebufp);

                        if(acct_info_flistp && acct_info_flistp != NULL)
                        {
                                write_et_vat_zones_flistp = PIN_FLIST_CREATE(ebufp);

                                account_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
                                area_code_lco = PIN_FLIST_FLD_GET(acct_info_flistp,MSO_FLD_AREA, 1, ebufp);

                                if(area_code_lco &&  account_pdp)
                                {
                                        PIN_FLIST_FLD_SET(write_et_vat_zones_flistp,PIN_FLD_POID,account_pdp, ebufp );
                                        PIN_FLIST_FLD_SET(write_et_vat_zones_flistp,MSO_FLD_AREA,area_code_lco, ebufp );
                                        PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0,write_et_vat_zones_flistp,&write_et_vat_zones_ret_flistp, ebufp);
                                }
                                PIN_FLIST_DESTROY_EX(&write_et_vat_zones_flistp,NULL);
                                PIN_FLIST_DESTROY_EX(&write_et_vat_zones_ret_flistp,NULL);
                        }
                        PIN_FLIST_DESTROY_EX(&acctinfo_ret_flistp,NULL);
                }
		






        }

    	PIN_FLIST_DESTROY_EX(&acctinfo_ret_flistp,NULL);
    	PIN_FLIST_DESTROY_EX(&lco_acctinfo_ret_flistp,NULL);
	//Update tax in billinfo completed
	
	acct_update = 0;

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling Customer Info update function");
	acct_update = fm_mso_update_custinfo(ctxp, i_flistp, out_flistp, ebufp);

	if ( acct_update == 0 || acct_update >= 3 )
	{		
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51405", ebufp);

		if ( acct_update == 0)
		{		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer update failed");	
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account - custinfo information update failed", ebufp);
		}
		else if ( acct_update == 3)
		{		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer area code is mandatory if city, state or area_code is modified");	
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account - custinfo information update failed - area code is mandatory if city, state or area_code is modified", ebufp);
		}		
		else if ( acct_update == 4)
		{		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer Contact history update failed");	
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account - custinfo information update failed - Contact history update failed", ebufp);
		}		
		else if ( acct_update == 7 )
		{		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer Payinfo Invoice update failed");	
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account - custinfo information update failed - Payinfo Invoice update failed", ebufp);
		}	
		else if ( acct_update == 8 )
		{		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer: Update failed due to outstanding balance.");	
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Address change failed - Outstanding amount must be 0 for add. change", ebufp);
		}
		else if ( acct_update == 9 )
		{		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer: Error in fetching outstanding balance.");	
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Address change failed - Error in fetching outstanding balance", ebufp);
		}			
		else if ( acct_update == 10 )
		{		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer: Error in fetching zip code.");	
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Address change failed - Error no LCO for this zip code", ebufp);
		}			
        else if ( acct_update == 11 )
        {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer: You can't move out of circle");
            PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Address change not allowed  in different circle", ebufp);
        }

		goto CLEANUP;
	}
	else if ( acct_update == 1)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer no need of account information change");	
	}
	else if ( acct_update == 2)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer account information change successful");	
		//fm_mso_cust_mod_cust_lc_event(ctxp, i_flistp, acnt_info, ebufp);
		fm_mso_create_lifecycle_evt(ctxp, i_flistp, acnt_info, ID_MODIFY_CUSTOMER_ADDRESS, ebufp);
	}	

	/*******************************************************************
	* Billing and installation address change  - End
	*******************************************************************/


	wholesale_flistp = PIN_FLIST_ELEM_GET(i_flistp, MSO_FLD_WHOLESALE_INFO, 0, 1, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "bu_profile input_flist", i_flistp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "wholesale flist", wholesale_flistp);
	action_mode = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTION_MODE, 1, ebufp);
	if (wholesale_flistp && (action_mode && *action_mode == non_entity_change ) )
	{
		acct_update = 0;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "The customer is Business user");	
		acct_update = fm_mso_update_bu_profile(ctxp, i_flistp, out_flistp, ebufp);
		msg = PIN_FLIST_FLD_GET(out_flistp, PIN_FLD_DESCR, 1, ebufp);

		if ( acct_update == 1)
		{		
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51406", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, msg, ebufp);
			goto CLEANUP;
		}
		else if ( acct_update == 3)
		{		
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51406", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, msg, ebufp);
			goto CLEANUP;
		}
		else if ( acct_update == 2)
		{		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer parent information change successful");
		}
		PIN_FLIST_FLD_DROP(out_flistp, PIN_FLD_ACTION_MODE, ebufp);
		PIN_FLIST_FLD_DROP(out_flistp, PIN_FLD_DESCR, ebufp);
		goto RETURN_FLIST;
	}

	/*****************************************************************************
	* Verifying whether end customer or JV/LCO/Distributor/Sub-distributor - Start
	******************************************************************************/

	wholesale_flistp = PIN_FLIST_ELEM_GET(i_flistp, MSO_FLD_WHOLESALE_INFO, 0, 1, ebufp);
	accessinfo_flistp = PIN_FLIST_ELEM_GET(i_flistp, MSO_FLD_ACCESS_INFO, 0, 1, ebufp);
	arinfo_flistp = PIN_FLIST_ELEM_GET(i_flistp, MSO_FLD_AR_INFO, 0, 1, ebufp);

	if (wholesale_flistp && wholesale_flistp != NULL)
	{
		acct_update = 0;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "The customer is Business user");	
		acct_update = fm_mso_update_businessuserinfo(ctxp, i_flistp, out_flistp, ebufp);

		if ( acct_update == 0)
		{		
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51406", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account - fm_mso_update_businessuserinfo update failed", ebufp);
			goto CLEANUP;
		}
		else if ( acct_update == 1)
		{		
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51406", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Old and new parent of BU are same", ebufp);
			goto CLEANUP;
		}
		else if ( acct_update == 2)
		{		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer parent information change successful");	
		}
		if ( !accessinfo_flistp && !arinfo_flistp )
			goto RETURN_FLIST;
	}

	/***************************************************************************
	* Verifying whether end customer or JV/LCO/Distributor/Sub-distributor - End
	***************************************************************************/

	/***************************************************************************
	* Access Info Change
	***************************************************************************/
	if (accessinfo_flistp && accessinfo_flistp != NULL)
	{
		acct_update = 0;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Updating Access info for Business user");	
		acct_update = fm_mso_update_bususer_access(ctxp, i_flistp, accessinfo_flistp, out_flistp, ebufp);

		if ( acct_update == 0)
		{		
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51607", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account - fm_mso_update_businessuser_credential update failed", ebufp);
			goto CLEANUP;
		}
		else if ( acct_update == 2)
		{		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer access information change successful");
			fm_mso_create_lifecycle_evt(ctxp, i_flistp, out_flistp, ID_MODIFY_ACCESSCONTROL_FOR_CSR, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
				PIN_ERRBUF_RESET(ebufp);
			}
		}
		if ( !arinfo_flistp && arinfo_flistp != NULL)
			goto RETURN_FLIST;
	}

	/***************************************************************************
	* Access Info Change
	***************************************************************************/

	/***************************************************************************
	* AR Info Change
	***************************************************************************/
	if (arinfo_flistp && arinfo_flistp != NULL)
	{
		acct_update = 0;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Updating AR limit for Business user");	
		acct_update = fm_mso_update_bususer_ar_info(ctxp, i_flistp, out_flistp, ebufp);

		if ( acct_update == 0)
		{		
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51608", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account - fm_mso_update_businessuser_arinfo update failed", ebufp);
			goto CLEANUP;
		}
		else if ( acct_update == 2)
		{		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer ar information change successful");
			fm_mso_create_lifecycle_evt(ctxp, i_flistp, out_flistp, ID_MODIFY_CUSTOMER_AR_LIMIT_BUS_USER, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
				PIN_ERRBUF_RESET(ebufp);
			}
		}
		goto RETURN_FLIST;
	}
	
	/***************************************************************************
	* AR Info Change
	***************************************************************************/
	/********************************** Updating the customer credential info *****************************************/
		
	cnt_upd = 0;
	cnt_upd = fm_mso_update_cust_credential(ctxp, i_flistp, out_flistp, ebufp);
	if ( cnt_upd == 3 )
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51604", ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer Credentials update failed");
		PIN_FLIST_FLD_SET(ret_flistp,PIN_FLD_ERROR_DESCR, "Account - custinfo information update failed - Credentials update failed", ebufp);
		goto CLEANUP;		
	}
	if ( cnt_upd == 2 )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_custinfo : Customer Credential info updated");
		fm_mso_create_lifecycle_evt(ctxp, i_flistp, out_flistp, ID_MODIFY_CUSTOMER_CHANGE_USERNAME_PASSWORD, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
			PIN_ERRBUF_RESET(ebufp);
		}
	}
	/********************************** Updating the customer credential info *****************************************/
	/********************************** Updating the business type info *****************************************/
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Validating business type change\n");
	bus_type = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BUSINESS_TYPE, 1, ebufp);
	if (bus_type && bus_type != NULL)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Validating business type change 2\n");
		cnt_upd = 0;
		cnt_upd = fm_mso_acc_type_change(ctxp, bus_type,i_flistp, out_flistp, ebufp);
		
		if ( cnt_upd == 3 )
		{
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
                	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51605", ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer Business model update failed");
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account - custinfo information update failed - Business model update failed", ebufp);
			goto CLEANUP;		
		}
		else if ( cnt_upd == 1 )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Business Type Change validation successful");
			fm_mso_create_lifecycle_evt(ctxp, out_flistp, i_flistp, ID_BUSINESS_TYPE_MODIFICATION, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
				PIN_ERRBUF_RESET(ebufp);
			}
		}
		else if (cnt_upd == 0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Old and new account type are same");
		}
		else if(cnt_upd == 4)
                {
                        ret_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51606", ebufp);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer Account model update failed");
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Not a valid status to change the Account type for the given service", ebufp);
                        goto CLEANUP;
                }
		cnt_upd = 0;
		cnt_upd = fm_mso_subs_type_change(ctxp, bus_type,i_flistp, out_flistp, ebufp);
		if ( cnt_upd == 3 )
		{
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
                	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51607", ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer Subscriber type update failed");
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account - custinfo information update failed - Business type update failed", ebufp);
			goto CLEANUP;		
		}
		else if ( cnt_upd == 1 )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Business Type Change validation successful");
			fm_mso_create_lifecycle_evt(ctxp, out_flistp, i_flistp, ID_BUSINESS_TYPE_MODIFICATION, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
				PIN_ERRBUF_RESET(ebufp);
			}
		}
		else if (cnt_upd == 0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Old and new subs type are same or Acc type/model has changed.");
		}

		cnt_upd = 0;
		cnt_upd = fm_mso_acc_model_change(ctxp, bus_type, &orig_bus_type, i_flistp, out_flistp, ebufp);
		if ( cnt_upd == 3 )
		{
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
                	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51606", ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer Account model update failed");
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account - custinfo information update failed - Account model update failed", ebufp);
			goto CLEANUP;		
		}
		else if (cnt_upd == 0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Old and new account model are same");
		}

		else if ( cnt_upd == 1 ) 
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account Type/Model/ServiceType/SubscriberType Change validation successful");	
			fm_mso_create_lifecycle_evt(ctxp, out_flistp, i_flistp, ID_BUSINESS_TYPE_MODIFICATION, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
				PIN_ERRBUF_RESET(ebufp);
			}
		}
	} 
	//Added to fix Business Type update issue.
	cnt_upd = 0;
	cnt_upd = fm_mso_bus_type_change(ctxp, bus_type, &orig_bus_type, i_flistp, out_flistp, ebufp);
	if ( cnt_upd == 3 )
	{
	  ret_flistp = PIN_FLIST_CREATE(ebufp);
	  PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp), ebufp);
 	  PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
	  PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51606", ebufp);
	  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer Account model update failed");
	  PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account-custinfo information update failed", ebufp);
	  goto CLEANUP;		
	}
	else if (cnt_upd == 0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Old and new busiess types are same");
	}
	else if ( cnt_upd == 2 ) 
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Business Type Change validation successful");	
		fm_mso_create_lifecycle_evt(ctxp, out_flistp, i_flistp, ID_BUSINESS_TYPE_MODIFICATION, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
			PIN_ERRBUF_RESET(ebufp);
		}
	}
	/********************************** Updating the business type info *****************************************/
	/********************************** Updating the Tax Exmption info *****************************************/

	cnt_upd = 0;
	cnt_upd = fm_mso_update_tax_info( ctxp, i_flistp, out_flistp, ebufp);
	if ( cnt_upd == 3 )
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
               	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
               	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51603", ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer Tax Exemption update failed");
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account - customer Tax Exemption update failed", ebufp);
		goto CLEANUP;		
	}
	else if ( cnt_upd == 2 ) 
	{	
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Tax EXEMPTION Array updated :");
		fm_mso_create_lifecycle_evt(ctxp, out_flistp, i_flistp, ID_MODIFY_CUSTOMER_TAX_EXEMPTION, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
			PIN_ERRBUF_RESET(ebufp);
		}

	}
	else if ( cnt_upd == 1 ) 
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No need to update Tax EXEMPTION Array :");	
/*	else  
        {
	        elem_id = 0;
                cookie = NULL;
                while ((args_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_EXEMPTIONS, &elem_id, 1, &cookie, ebufp)) != NULL)
                {
                        PIN_FLIST_ELEM_COPY(i_flistp, PIN_FLD_EXEMPTIONS, elem_id, acct_flist, PIN_FLD_EXEMPTIONS, elem_id, ebufp);
                }
	}	*/
	/********************************** Updating the Tax Exmption info *****************************************/

	/*******************************************************************
	* Billing cycle change - Start
	*******************************************************************/
	acct_update = 0;
	acct_update = fm_mso_update_billinfo(ctxp, i_flistp, out_flistp, ebufp);	

	if ( acct_update == 0)
	{	
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51407", ebufp);
		error_descr = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ERROR_DESCR, 1, ebufp) ;
		if (error_descr && error_descr != NULL)
		{
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, error_descr, ebufp);
		}
		else
		{
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account - billinfo information update failed", ebufp);
		}
		goto CLEANUP;
	}
	else if ( acct_update == 1)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer no need of billinfo information change");	
	}
	else if ( acct_update == 2)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer billinfo information change successful");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer bill suppression information updated");
		fm_mso_create_lifecycle_evt(ctxp, out_flistp, i_flistp, ID_BILL_INFORMATION_MODIFICATION, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
			PIN_ERRBUF_RESET(ebufp);
		}

	}

	acct_update = 0;
	acct_update = fm_mso_update_bb_billinfo(ctxp, i_flistp, out_flistp, ebufp);	

	if ( acct_update == 3)
	{	
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51601", ebufp);
		error_descr = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ERROR_DESCR, 1, ebufp) ;
		if (error_descr && error_descr != NULL)
		{
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, error_descr, ebufp);
		}
		else
		{
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account - bill suppression information updation failed", ebufp);
		}
		goto CLEANUP;
	}
	else if ( acct_update == 1)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer no need of updating bill suppresiion info");	
	}
	else if ( acct_update == 2)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer bill suppression information updated");
		fm_mso_create_lifecycle_evt(ctxp, out_flistp, i_flistp, ID_MODIFY_CUSTOMER_BILL_SUPPRESSION, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
			PIN_ERRBUF_RESET(ebufp);
		}
	}

	/*******************************************************************
	* Billing cycle change  - End
	*******************************************************************/
 	billinfo = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_BILLINFO, 0, 1, ebufp );
	if (billinfo && corporate_type == CORPORATE_PARENT)
	{
		db = PIN_POID_GET_DB(routing_poid);
		srch_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
		srch_flag = 768;
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, "select X from /billinfo 1, /group/billing 2 , /service 3 where 1.F1 = 2.F2 and 1.F1 = 3.F3 and 2.F4 = V4 and 1.F6 = V6 and 3.F5!=V5 " , ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		members = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_MEMBERS, PIN_ELEMID_ANY, ebufp );
		PIN_FLIST_FLD_SET(members, PIN_FLD_OBJECT, NULL, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp)
		 
		args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp );
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID ,args_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
		
		args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 5, ebufp );
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &srvc_status_cancel, ebufp)

		args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 6, ebufp );
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PAY_TYPE, &pay_type_suboord, ebufp)

		result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_LAST_BILL_OBJ, NULL, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "update_billinfo input list", srch_flistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
			goto CLEANUP;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "update_billinfo output flist", srch_out_flistp);

		if (srch_out_flistp && srch_out_flistp != NULL)
		{
		  while ((result_flist = PIN_FLIST_ELEM_GET_NEXT(srch_out_flistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp )) != NULL)
		  {
		     last_bill_obj = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_LAST_BILL_OBJ, 1, ebufp );
		     if (last_bill_obj && PIN_POID_GET_ID(last_bill_obj) == 0 )
		     {
			PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51407", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "At least one child a/c has no Bills : Not allowed to change Billing information", ebufp);
			goto CLEANUP;
		     }
		     
		  }
		  PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
		}
	}
 	/*******************************************************************
	* Billing cycle change  - End
	*******************************************************************/

	/*******************************************************************
	* Service information change - Start
	*******************************************************************/
	
	acct_update = 0;
	acct_update = fm_mso_update_serviceinfo(ctxp, i_flistp, out_flistp, ebufp);	

	if ( acct_update == 0)
	{		
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51408", ebufp);
		if (PIN_FLIST_FLD_GET(out_flistp, PIN_FLD_ERROR_DESCR, 1, ebufp ) )
                {
			PIN_FLIST_FLD_COPY(out_flistp, PIN_FLD_ERROR_DESCR, ret_flistp,PIN_FLD_ERROR_DESCR,ebufp);
			PIN_FLIST_FLD_DROP(out_flistp, PIN_FLD_ERROR_DESCR, ebufp);
		}
		else
		{
		      PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account - services information update failed", ebufp);
                 }
		goto CLEANUP;
	}
	else if ( acct_update == 1)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer no need of services information change");	
	}
	else if ( acct_update == 2)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer services information change successful");
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, out_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, out_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_NO, out_flistp, PIN_FLD_ACCOUNT_NO, ebufp);
		//fm_mso_cust_mod_srvc_lc_event(ctxp, out_flistp, NULL, ebufp );
		fm_mso_create_lifecycle_evt(ctxp, out_flistp, NULL, ID_MODIFY_CATV_SERVICE_REGION_KEY, ebufp );
	}
	
	acct_update = 0;
	acct_update = fm_mso_update_bb_service(ctxp, i_flistp, out_flistp, ebufp);	

	// Added below condition for JIO CMTS validation - acct_update = 16
	if ( acct_update == 0 || acct_update == 3 || acct_update == 10 || acct_update == 11 || acct_update == 15 || acct_update == 16)
	{		
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
		if ( acct_update == 0 )
		{

                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,"fm_mso_update_bb_service failed-inp",i_flistp);
                        if(out_flistp!= NULL){
                                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,"fm_mso_update_bb_service failed-out",out_flistp);
                                if(PIN_FLIST_ELEM_COUNT(out_flistp,PIN_FLD_RESULTS_DATA,ebufp)>0)
                                {
					results_data_flistp = PIN_FLIST_ELEM_GET(out_flistp,PIN_FLD_RESULTS_DATA,PIN_ELEMID_ANY,1,ebufp);
					PIN_FLIST_FLD_COPY(results_data_flistp,PIN_FLD_ERROR_CODE,ret_flistp, PIN_FLD_ERROR_CODE,ebufp);
					PIN_FLIST_FLD_COPY(results_data_flistp,PIN_FLD_ERROR_DESCR,ret_flistp, PIN_FLD_ERROR_DESCR,ebufp);
                                } else {
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51602", ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account - BB services information update failed",ebufp);				     }

                        } else { 
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51602", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account - BB services information update failed", ebufp);
			}
		}
		if ( acct_update == 3 )
		{
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51609", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account - Credit limit information update failed", ebufp);
		}
		if ( acct_update == 10 )
		{
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51632", ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in purchasing static IP device for the service", ebufp);
		}
		if ( acct_update == 11 )
		{
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51648", ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "MSO_FLD_PASSWORD is mandatory field for password change case", ebufp);
		}
		if ( acct_update == 15 )
		{
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51779", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Disassociation Failed for Existing Device.", ebufp);
		}
		// Added the below condition for JIO-CPS Integration-ISP Enhancement.
		if ( acct_update == 16 )
		{
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51800", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "CMTS ID change failed due to Network mismatch.", ebufp);
		}
		goto CLEANUP;
	}
	else if ( acct_update == 1)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer no need of services information change");	
	}
	else if ( acct_update == 2)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer services information change successful");
		fm_mso_create_lifecycle_evt(ctxp, out_flistp, i_flistp, ID_MODIFY_BB_SEVICE_CMTS, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
			PIN_ERRBUF_RESET(ebufp);
		}
	}

	/*******************************************************************
	* Service information change - End
	*******************************************************************/


	// Discount info change start
	acct_update = 0;
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling Product Discount info function");
	acct_update = fm_mso_modify_prod_discount_info(ctxp, i_flistp, out_flistp, ebufp);

	if ( acct_update == 0)
	{		
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51405", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account - Discount information update failed", ebufp);
		goto CLEANUP;
	}
	else if ( acct_update == 1)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer no need of discount info change");	
	}
	else if ( acct_update == 2)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer discount info change successful");	
	}
	// Discount info Change End


	/*******************************************************************
	* Payinfo information change - Start
	*******************************************************************/
	acct_update = 0;
	acct_update = fm_mso_update_payinfo(ctxp, i_flistp, out_flistp, ebufp);	

	if ( acct_update == 0)
	{		
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51409", ebufp);
		error_descr = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ERROR_DESCR, 1, ebufp );
		if (error_descr && error_descr != NULL)
		{
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, error_descr , ebufp);
		}
		else
		{
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account - payinfo information update failed", ebufp);
		}
		goto CLEANUP;
	}
	else if ( acct_update == 1)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer no need of payinfo information change");	
	}
	else if ( acct_update == 2)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer payinfo information change successful");	
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, out_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, out_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_NO, out_flistp, PIN_FLD_ACCOUNT_NO, ebufp);
		fm_mso_create_lifecycle_evt(ctxp, out_flistp, NULL, ID_PAYMENT_INFORMATION_MODIFICATION, ebufp );
	}
	

	/*******************************************************************
	* Payinfo information change - End
	*******************************************************************/

	/*******************************************************************
	* LCO information change - Start
	*******************************************************************/
	
	acct_update = 0;
	acct_update = fm_mso_update_lcoinfo(ctxp, i_flistp, out_flistp, &commission_err_flist, ebufp);	

	if ( acct_update == 0)
	{	
		if (commission_err_flist && commission_err_flist != NULL)
		{
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
			PIN_FLIST_FLD_COPY(commission_err_flist, PIN_FLD_ERROR_CODE,  ret_flistp, PIN_FLD_ERROR_CODE,ebufp);
			PIN_FLIST_FLD_COPY(commission_err_flist, PIN_FLD_ERROR_DESCR, ret_flistp, PIN_FLD_ERROR_DESCR, ebufp);
			goto CLEANUP;
		}
		else
		{
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51410", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account - LCO information update failed", ebufp);
			goto CLEANUP;
		}
	}
	else if ( acct_update == 1)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer no need of LCO information change");	
	}
	else if ( acct_update == 2)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer LCO information change successful");	
		fm_mso_create_lifecycle_evt(ctxp, out_flistp, i_flistp, ID_HIERARCHY_MODIFICATION, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
			PIN_ERRBUF_RESET(ebufp);
		}
	}
	else if ( acct_update == 3)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer LCO information Insufficient data");	
	}
	else if ( acct_update == 4)
	{		
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51410", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Commision model for Old and New LCO does not match.", ebufp);
		goto CLEANUP;	
	}	
	else if ( acct_update == 5)
	{		
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51410", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Existing LCO and new LCO are same", ebufp);
		goto CLEANUP;
	}
	else if ( acct_update == 6)
	{		
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51411", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Source acount is not a subscriber account", ebufp);
		goto CLEANUP;
	}
	else if ( acct_update == 7)
	{		
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51412", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Parent acount is not a LCO account", ebufp);
		goto CLEANUP;
	}
    else if ( acct_update == 8)
    {
        ret_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51408", ebufp);
        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "LCO account passed is of another service provider", ebufp);
        goto CLEANUP;
    }
	
	

	/*******************************************************************
	* LCO information change - End
	*******************************************************************/	

	/***** Sales Obj update start ******/
	acct_update = 0;
	acct_update = fm_mso_update_salesman(ctxp, i_flistp, out_flistp, ebufp);	

	if ( acct_update == 0)
	{		
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51421", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account - Salesman information update failed", ebufp);
		goto CLEANUP;
	}
	else if ( acct_update == 1)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer no need of Salesman information change");	
	}
	else if ( acct_update == 2)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer Salesman information change successful");	
	}
	
	/****** Salesman obj update end ******/
	
	
	
	/*******************************************************************
	* Roles change - Start
	*******************************************************************/
	
	acct_update = 0;
	acct_update = fm_mso_update_roles(ctxp, i_flistp, out_flistp, ebufp);	

	if ( acct_update == 0)
	{		
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51412", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account - Roles update failed", ebufp);
		goto CLEANUP;
	}
	else if ( acct_update == 1)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer no need of Roles change");	
	}
	else if ( acct_update == 2)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer Roles change successful");
		fm_mso_create_lifecycle_evt(ctxp, out_flistp, i_flistp, ID_MODIFY_ROLES_OF_CSR, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
			PIN_ERRBUF_RESET(ebufp);
		}
	}
	

	/*******************************************************************
	* Roles change - End
	*******************************************************************/


	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_cust_modify_customer", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51411", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account - Modififcation failed - Unknwn error in BRM ", ebufp);
		goto CLEANUP;
	}


	/*******************************************************************
	* Create Entry in profile_cust_offer_t when Offer is Present in IP
	*******************************************************************/
	if (!PIN_ERRBUF_IS_ERR(ebufp))
	{
		elem_id=0; 
		cookie= NULL;
		int telem_id=0; 
		pin_cookie_t tcookie= NULL;
		char *ofr_desc= NULL;
		char pai[20]="";
		poid_t *srch_login_pdp=NULL;
		while((temp_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, MSO_FLD_REGISTER_CUSTOMER,&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			profile_offer_flistp = PIN_FLIST_CREATE(ebufp);
			srch_login_pdp = PIN_POID_CREATE(db, "/profile_cust_offer", (int64)-1, ebufp);
			PIN_FLIST_FLD_PUT(profile_offer_flistp, PIN_FLD_POID, srch_login_pdp, ebufp );
			PIN_FLIST_FLD_SET(profile_offer_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp );
			PIN_FLIST_FLD_SET(profile_offer_flistp, MSO_FLD_OFFER_DESCR, PIN_FLIST_FLD_GET(temp_flistp, MSO_FLD_OFFER_DESCR, 1, ebufp), ebufp );
			PIN_FLIST_FLD_SET(profile_offer_flistp, MSO_FLD_OFFER_VALUE, PIN_FLIST_FLD_GET(temp_flistp, MSO_FLD_OFFER_VALUE, 1, ebufp), ebufp );
			if(PIN_FLIST_FLD_GET(temp_flistp, MSO_FLD_RMN, 1, ebufp) !=NULL)
			{
				PIN_FLIST_FLD_SET(profile_offer_flistp, MSO_FLD_RMN, PIN_FLIST_FLD_GET(temp_flistp, MSO_FLD_RMN, 1, ebufp), ebufp );
			}
			if(PIN_FLIST_FLD_GET(temp_flistp, MSO_FLD_RMAIL, 1, ebufp) !=NULL)
			{
				PIN_FLIST_FLD_SET(profile_offer_flistp, MSO_FLD_RMAIL, PIN_FLIST_FLD_GET(temp_flistp, MSO_FLD_RMAIL, 1, ebufp), ebufp );
			}
			if(PIN_FLIST_FLD_GET(temp_flistp, MSO_FLD_ISREGISTERED, 1, ebufp) !=NULL)
			{
				PIN_FLIST_FLD_SET(profile_offer_flistp, MSO_FLD_ISREGISTERED, PIN_FLIST_FLD_GET(temp_flistp, MSO_FLD_ISREGISTERED, 1, ebufp), ebufp );
			}
			ofr_desc=PIN_FLIST_FLD_GET(temp_flistp, MSO_FLD_OFFER_DESCR, 1, ebufp);
			if(ofr_desc && (strstr(ofr_desc,"Netflix") ||strstr(ofr_desc,"NETFLIX")))
			{
				rdfld_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(rdfld_flistp, PIN_FLD_POID, account_obj, ebufp );	
				PIN_FLIST_FLD_SET(rdfld_flistp, PIN_FLD_ACCOUNT_NO, NULL, ebufp );
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_READ_FLDS input flist for profile_cust_offer creation", rdfld_flistp);
				PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, rdfld_flistp, &rdfld_oflistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_READ_FLDS output flist for profile_cust_offer creation", rdfld_oflistp);
				sprintf(pai,"%s%s","NF",PIN_FLIST_FLD_GET(rdfld_oflistp, PIN_FLD_ACCOUNT_NO, 1, ebufp));	
				PIN_FLIST_FLD_SET(profile_offer_flistp, MSO_FLD_PAI, pai, ebufp );
				PIN_FLIST_DESTROY_EX(&rdfld_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&rdfld_oflistp, NULL);
			}
			
			/****************************************************************************************
			* SEARCH IF EXISTING OFFER IS PRESENT FOR SAME DESCR AND OVERRIDE TO AVOID MULTIPLE ROWS
			*****************************************************************************************/
			fm_mso_get_eprofile_offer(ctxp, account_obj, ofr_desc, &srch_offer_oflistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_profile_offer output flist", srch_offer_oflistp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Searching Profile_Cust_offer Obj", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, routing_poid, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51882", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Searching Profile_Cust_offer Obj", ebufp);
				goto CLEANUP;
			}
			if (PIN_FLIST_ELEM_COUNT(srch_offer_oflistp, PIN_FLD_RESULTS, ebufp)  >0)
			{
				rdfld_flistp = PIN_FLIST_CREATE(ebufp);
				while((rdfld_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_offer_oflistp, PIN_FLD_RESULTS,&telem_id, 1, &tcookie, ebufp)) != (pin_flist_t *)NULL)
				{
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Fields from Search profile", rdfld_flistp);
					PIN_FLIST_FLD_SET(rdfld_flistp, MSO_FLD_OFFER_VALUE, PIN_FLIST_FLD_GET(temp_flistp, MSO_FLD_OFFER_VALUE, 1, ebufp), ebufp);
					if(PIN_FLIST_FLD_GET(temp_flistp, MSO_FLD_ISREGISTERED, 1, ebufp) !=NULL)
					{
						PIN_FLIST_FLD_SET(rdfld_flistp, MSO_FLD_ISREGISTERED, PIN_FLIST_FLD_GET(temp_flistp, MSO_FLD_ISREGISTERED, 1, ebufp), ebufp );
					}
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_WRITE_FLDS input flist for profile_cust_offer creation", rdfld_flistp);
					PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, rdfld_flistp, &rdfld_oflistp, ebufp);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_WRITE_FLDS output flist for profile_cust_offer creation", rdfld_oflistp);
				}
				PIN_FLIST_DESTROY_EX(&rdfld_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&rdfld_oflistp, NULL);
			}
			else 
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_CREATE_OBJ input flist for profile_cust_offer creation", profile_offer_flistp);
				PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, profile_offer_flistp, &profile_offer_oflistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_CREATE_OBJ output flist for profile_cust_offer creation", profile_offer_oflistp);
			}

			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Generating Profile_Cust_offer Obj", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, routing_poid, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51881", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Generating Profile_Cust_offer Obj", ebufp);
				goto CLEANUP;
			}
			PIN_FLIST_DESTROY_EX(&profile_offer_flistp, NULL);
		}
	   	PIN_FLIST_DESTROY_EX(&profile_offer_oflistp, NULL);
	}

	/*******************************************************************
	* Create Output flist - Start
	*******************************************************************/
RETURN_FLIST:
	ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, account_obj, ebufp );
//	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ACCOUNT_NO, account_no_str, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_NO, ret_flistp, PIN_FLD_ACCOUNT_NO, ebufp);
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_customer_success, ebufp);
	if (msg && strlen(msg)>1)
	{
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_DESCR, msg, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_DESCR, "ACCOUNT Modification done successfully", ebufp);
	}
	if(tech_change == 0)
	{
		PIN_FLIST_CONCAT(ret_flistp, out_flistp, ebufp); 
	}
	PIN_FLIST_DESTROY_EX(&out_flistp, NULL);

	
	/*******************************************************************
	* Create Output flist - Start
	*******************************************************************/

CLEANUP:	
	PIN_FLIST_DESTROY_EX(&out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&update_po_out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&findprofile_outflistp, NULL);
	PIN_FLIST_DESTROY_EX(&profile_out_flistp, NULL);
	*r_flistpp = ret_flistp;
		
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_cust_modify_customer - return flist",*r_flistpp);	
	return;
}

/**************************************************
* Function: fm_mso_validate_csr_role()
* 
* 
***************************************************/
EXPORT_OP int 
fm_mso_validate_csr_role(
	pcm_context_t		*ctxp,
	int64			db,
	poid_t			*user_id,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_out_flistp = NULL;

	poid_t			*srch_pdp = NULL;

	int32			srch_flag = 256;
	int				csr_access = 0;
	int				*customer_type = NULL;
	char			*template = "select X from /mso_customer_credential where  F1 = V1 ";


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;

	if (PIN_POID_GET_ID(user_id) == 1)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_service_status CSR haas permission to change account information");
		return 1;
	}
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_csr_role :");

	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, user_id, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_CUSTOMER_TYPE, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_csr_role SEARCH input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_csr_role SEARCH output flist", srch_out_flistp);

	if (srch_out_flistp)
	{
		result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp );
		if (result_flist)
		{
			customer_type = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_CUSTOMER_TYPE, 0, ebufp);
			//if ((*(int *)customer_type == ACCT_TYPE_CSR_ADMIN) || (*(int *)customer_type == ACCT_TYPE_CSR))
			if ((*(int *)customer_type != ACCT_TYPE_SUBSCRIBER))
			{
				csr_access = 1;
			}
		}
	}

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in fetching results from SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
		return 0;
	}
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	return csr_access;
}

/**************************************************
* Function: fm_mso_update_nameinfo()
* 
* 
***************************************************/
static int 
fm_mso_update_nameinfo(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		*out_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*nameinfo_flistp = NULL;
	pin_flist_t		*updacct_inflistp = NULL;
	pin_flist_t		*updacct_outflistp = NULL;
	pin_flist_t		*updnameinfo_flist = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_nameinfo :");	

	nameinfo_flistp = PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_NAMEINFO, NAMEINFO_BILLING, 1, ebufp );

	if (!nameinfo_flistp)
	{
		return 1;
	}

	updacct_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, updacct_inflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_ACCOUNT_OBJ, updacct_inflistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, updacct_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);
	//PIN_FLIST_ELEM_COPY(in_flist, PIN_FLD_NAMEINFO, PIN_ELEMID_ANY, updacct_inflistp, PIN_FLD_NAMEINFO, PIN_ELEMID_ANY, ebufp);

	updnameinfo_flist = PIN_FLIST_ELEM_ADD(updacct_inflistp, PIN_FLD_NAMEINFO, NAMEINFO_BILLING, ebufp);
	PIN_FLIST_FLD_COPY(nameinfo_flistp, PIN_FLD_FIRST_NAME, updnameinfo_flist, PIN_FLD_FIRST_NAME, ebufp);
	PIN_FLIST_FLD_COPY(nameinfo_flistp, PIN_FLD_MIDDLE_NAME, updnameinfo_flist, PIN_FLD_MIDDLE_NAME, ebufp);
	PIN_FLIST_FLD_COPY(nameinfo_flistp, PIN_FLD_LAST_NAME, updnameinfo_flist, PIN_FLD_LAST_NAME, ebufp);

 	//fm_mso_cust_mod_cust_lc_event(ctxp, in_flist, out_flist, ebufp);
	fm_mso_create_lifecycle_evt(ctxp, in_flist, out_flist, ID_MODIFY_CUSTOMER_ADDRESS, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_nameinfo input list", updacct_inflistp);
	PCM_OP(ctxp, PCM_OP_CUST_UPDATE_CUSTOMER, 0, updacct_inflistp, &updacct_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&updacct_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_CUST_UPDATE_CUSTOMER", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_nameinfo output flist", updacct_outflistp);
	PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);
	return 2;
}

/**************************************************
* Function: fm_mso_update_custinfo()
* 
* 
***************************************************/
static int 
fm_mso_update_custinfo(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		*out_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*nameinfo_flistp = NULL;
	pin_flist_t		*updacct_inflistp = NULL;
	pin_flist_t		*updacct_outflistp = NULL;
	pin_flist_t		*readacct_inflistp = NULL;
	pin_flist_t		*readacct_outflistp = NULL;
	pin_flist_t     *racct_outflistp = NULL;
	pin_flist_t		*acct_flist = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*provaction_inflistp = NULL;
	pin_flist_t		*provaction_outflistp = NULL;
	pin_flist_t		*search_inflistp = NULL;
	pin_flist_t		*search_outflistp = NULL;
	pin_flist_t		*svcwrite_inflistp = NULL;
	pin_flist_t		*svcwrite_outflistp = NULL;
	pin_flist_t		*results_flistp = NULL;
	pin_flist_t		*catv_info = NULL;
	pin_flist_t		*acct_info_flistp = NULL;
	pin_flist_t		*out_bal_flistp = NULL;
	pin_flist_t		*payinfo_flistp = NULL;
	pin_flist_t		*inst_addr_inp = NULL;
	pin_flist_t		*alias_list = NULL;
	pin_flist_t		*srch_payinfo_iflist = NULL;
	pin_flist_t		*srch_payinfo_oflist = NULL;
	pin_flist_t		*payinfo_array = NULL;
	pin_flist_t		*inv_info_arr = NULL;
	pin_flist_t		*nameinfo_billing_inp = NULL;
	pin_flist_t		*inherited_info = NULL;


	int			elem_id = 0;
	int			counter = 0;
	int			status_flags = 768;
	int			status = PIN_STATUS_ACTIVE;
	int			status_inactive = PIN_STATUS_INACTIVE;
	int			*contact_pref = NULL;
	int32			billing_addr_passed =1;
	int32			*srvc_status = NULL;
	int			cnt_upd = 0;
	int			cr = 0;
	int64			db = 0;
	char			*area = NULL;
	char			*city_state = NULL;
	char			*old_city = NULL;
	char			*new_city = NULL;
	char			*rmail = NULL;
	char			*rmn = NULL;
	char			*zip_code = NULL;
	char			*old_zip_code = NULL;
	char			*poap = NULL;
	char			*poip = NULL;
	char			search_template[150] = " select X from /service where F1 = V1 and ( F2 = V2 or F3 = V3 ) and service_t.poid_type = '/service/catv' ";
	char			region_key[20];
	char			updated_name[200];
	char			*network_node = NULL;
	char			*template = "select X from /service/catv where F1 = V1 and ( F2 =V2 or F3 = V3 )";
	char			*srch_template = "select X from /payinfo where F1 = V1 and payinfo_t.poid_type ='/payinfo/invoice' ";
	char			*new_bouquet_id = NULL;
	char			*device_id = NULL;
	char			*billing_addr = NULL;
	char			*billing_city = NULL;
	char			*billing_country = NULL;
	char			*email_addr = NULL;
	char			*billing_state = NULL;
	char			*billing_zip = NULL;
    char            *actionp = NULL;
	char			*fn = NULL;
	char			*mn = NULL;
	char			*ln = NULL;
	// Added for payinfo_inv_t updates
	char			*building_name = NULL;
    char            *building_idp = NULL;
	char			*landmark  = NULL;
	char			*street = NULL;
	char  			*location = NULL;
    char            *old_circlep = NULL;
    char            *new_circlep = NULL;

	void			*vp = NULL;

	poid_t			*service_obj = NULL;
	poid_t			*srch_pdp = NULL;
	poid_t			*acct_obj = NULL;
	poid_t			*payinfo_obj = NULL;
	pin_cookie_t	        cookie = NULL;
	pin_decimal_t	        *pend_amt =  NULL;
	pin_decimal_t	        *zero = NULL;
	/*** CATV Changes ***/
	int64			bus_type = 0;
	char			*nmail = NULL;
	poid_t			*accct_obj = NULL;
	pin_flist_t		*accct_info_flistp = NULL;
    char            *nmaill = NULL;
	pin_flist_t		*wflds_acc_inp_flistp = NULL;
	pin_flist_t		*wflds_acc_out_flistp = NULL;
	int32			*business_type = NULL;
    int32           home_move_flag = 0;
	char                    bus_type_str[8];

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_custinfo :", in_flist);
	zero = pbo_decimal_from_str("0", ebufp);
	nameinfo_flistp = PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_NAMEINFO, NAMEINFO_BILLING, 1, ebufp );

	acct_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp);
	actionp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_ACTION, 1, ebufp);

	contact_pref = PIN_FLIST_FLD_GET(in_flist, MSO_FLD_CONTACT_PREF, 1, ebufp);
	area = PIN_FLIST_FLD_GET(in_flist, MSO_FLD_AREA, 1, ebufp);
	rmail = PIN_FLIST_FLD_GET(in_flist, MSO_FLD_RMAIL, 1, ebufp);
	rmn = PIN_FLIST_FLD_GET(in_flist, MSO_FLD_RMN, 1, ebufp);

	if (!nameinfo_flistp)
	{
		billing_addr_passed = 0;
		nameinfo_flistp = PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_NAMEINFO, NAMEINFO_INSTALLATION, 1, ebufp );
		if (!nameinfo_flistp)
		{
			nameinfo_flistp = PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_NAMEINFO, NAMEINFO_COMPANY, 1, ebufp );
			if (!nameinfo_flistp)
			{
			
				if (!contact_pref && !area && !rmail && !rmn)
				{
					return 1;
				}
				//city_state = PIN_FLIST_FLD_GET(nameinfo_flistp, PIN_FLD_CITY, 1, ebufp);
			}
			else
			{
				zip_code = PIN_FLIST_FLD_GET(nameinfo_flistp, PIN_FLD_ZIP, 1, ebufp);
				city_state = PIN_FLIST_FLD_GET(nameinfo_flistp, PIN_FLD_CITY, 1, ebufp);
				if (!city_state)
				{
					city_state = PIN_FLIST_FLD_GET(nameinfo_flistp, PIN_FLD_STATE, 1, ebufp);
				}
			}
		}
		else
		{
			/* Pawan:11-DEC-2014: Installation address change is considered as relocation
					Added below function to ensure that there is  NO outstanding balance 
					while relocation */
			/*** CATV Changes ***/			
			if (bus_type && (bus_type > 90000000) && (bus_type < 99000000))
            {
				fm_mso_get_outstanding_bal(ctxp, acct_obj, "BB", &out_bal_flistp, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in getting outstanding balance for account", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				return 9;
				}	
				if(out_bal_flistp)
				{
					pend_amt = PIN_FLIST_FLD_GET(out_bal_flistp, PIN_FLD_PENDING_RECV, 1, ebufp);
					if(pbo_decimal_compare(pend_amt,zero,ebufp)>0)
					{
						return 8;
					}
					PIN_FLIST_DESTROY_EX(&out_bal_flistp, NULL);
					pbo_decimal_destroy(&zero);
				}
			}

			zip_code = PIN_FLIST_FLD_GET(nameinfo_flistp, PIN_FLD_ZIP, 1, ebufp);
			vp = PIN_FLIST_FLD_GET(in_flist, MSO_FLD_STB_ID, 1, ebufp);
			if (vp && actionp && strstr(actionp, "HOME MOVE"))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "DVBIP customer address update");

				fm_mso_search_zip_lco_account(ctxp, old_zip_code, 1, &search_outflistp, ebufp);
                if (search_outflistp)
                {
                    old_circlep = PIN_FLIST_FLD_TAKE(search_outflistp, MSO_FLD_AREA_CODE, 1, ebufp);
                }
                
                PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
                fm_mso_search_zip_lco_account(ctxp, zip_code, 1, &search_outflistp, ebufp);

				if (search_outflistp)
				{
                    new_circlep = PIN_FLIST_FLD_TAKE(search_outflistp, MSO_FLD_AREA_CODE, 1, ebufp);
                    if (old_circlep && new_circlep && strcmp(new_circlep, old_circlep) != 0)
                    {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Operating Circle is not matching", ebufp);
                        PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
                        pin_free(old_circlep);
                        pin_free(new_circlep);
                        return 11;
                    }

					new_city = PIN_FLIST_FLD_TAKE(search_outflistp, MSO_FLD_DISTRICT, 0, ebufp);
					PIN_FLIST_FLD_PUT(nameinfo_flistp, PIN_FLD_CITY, new_city, ebufp);

					city_state = PIN_FLIST_FLD_TAKE(search_outflistp, PIN_FLD_STATE, 0, ebufp);
					PIN_FLIST_FLD_PUT(nameinfo_flistp, PIN_FLD_STATE, city_state, ebufp);

					area = PIN_FLIST_FLD_TAKE(search_outflistp, MSO_FLD_AREA, 0, ebufp);
					PIN_FLIST_FLD_PUT(in_flist, MSO_FLD_AREA, area, ebufp);
				    if (strcmp(old_zip_code, zip_code) != 0)    	
                    {
	                    acct_info_flistp = PIN_FLIST_SUBSTR_GET(in_flist, PIN_FLD_CUSTOMER_CARE_INFO, 1, ebufp );
                        if (!acct_info_flistp)
                        {
                            acct_info_flistp = PIN_FLIST_SUBSTR_ADD(in_flist, PIN_FLD_CUSTOMER_CARE_INFO, ebufp );
                        }
                        PIN_FLIST_FLD_COPY(search_outflistp, PIN_FLD_ACCOUNT_OBJ, acct_info_flistp, MSO_FLD_LCO_OBJ, ebufp);
                        home_move_flag = 1;
                    }
				}
                else
                {
		            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"No zip code mapping found for this account", ebufp);
                    PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
                    if (old_circlep) pin_free(old_circlep);
                    return 10;
                }

				PIN_FLIST_ELEM_SET(in_flist, nameinfo_flistp, PIN_FLD_NAMEINFO, NAMEINFO_BILLING, ebufp)
				billing_addr_passed = 1;
                
                if (old_circlep) pin_free(old_circlep);
                if (new_circlep) pin_free(new_circlep);	
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_custinfo : DVBIP modified", in_flist);
			}

			city_state = PIN_FLIST_FLD_GET(nameinfo_flistp, PIN_FLD_CITY, 1, ebufp);
			if (!city_state)
			{
				city_state = PIN_FLIST_FLD_GET(nameinfo_flistp, PIN_FLD_STATE, 1, ebufp);
			}
		}
	}
	else
	{
		city_state = PIN_FLIST_FLD_GET(nameinfo_flistp, PIN_FLD_CITY, 1, ebufp);
		if (!city_state)
		{
			city_state = PIN_FLIST_FLD_GET(nameinfo_flistp, PIN_FLD_STATE, 1, ebufp);
		}

		nameinfo_flistp = PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_NAMEINFO, NAMEINFO_INSTALLATION, 1, ebufp );
		if (nameinfo_flistp)
		{
			zip_code = PIN_FLIST_FLD_GET(nameinfo_flistp, PIN_FLD_ZIP, 1, ebufp);
		}
		cr = 1;
	}
	if (city_state)
	{
		if (!area && billing_addr_passed ==1 )
		{
			return 3;
		}
	}	

      /**************************** Read ACCT object **********************************/
      readacct_inflistp = PIN_FLIST_CREATE(ebufp);
      PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, readacct_inflistp, PIN_FLD_POID, ebufp);
      PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_custinfo read account input list", readacct_inflistp);
      PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, readacct_inflistp, &readacct_outflistp, ebufp);
      PIN_FLIST_DESTROY_EX(&readacct_inflistp, NULL);
      if (PIN_ERRBUF_IS_ERR(ebufp))
      {
          PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in accessing account or the account does not exist", ebufp);
          PIN_ERRBUF_RESET(ebufp);
          PIN_FLIST_DESTROY_EX(&readacct_outflistp, NULL);
           return 0;
      }
      PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_custinfo read account output flist", readacct_outflistp);
      accct_info_flistp = PIN_FLIST_ELEM_GET(in_flist,PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
      if(accct_info_flistp)
      {
           nmail = PIN_FLIST_FLD_GET(accct_info_flistp, PIN_FLD_EMAIL_ADDR, 1, ebufp);
      }

	accct_obj = PIN_FLIST_FLD_GET(readacct_outflistp, PIN_FLD_POID, 1, ebufp);
    nmaill = PIN_FLIST_FLD_GET(readacct_outflistp, MSO_FLD_RMAIL, 1, ebufp);
	poip = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_ACCESS_CODE1, 1, ebufp);	
	poap = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_ACCESS_CODE2, 1, ebufp);	
	if (nmail || poip || poap)
	{
		wflds_acc_inp_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(wflds_acc_inp_flistp, PIN_FLD_POID, accct_obj, ebufp);
		if (nmail)
		{
			PIN_FLIST_FLD_SET(wflds_acc_inp_flistp, MSO_FLD_RMAIL, nmail, ebufp);
		}
		if (poip)
		{
			PIN_FLIST_FLD_SET(wflds_acc_inp_flistp, PIN_FLD_ACCESS_CODE1, poip, ebufp);
		}
		if (poap)
		{
			PIN_FLIST_FLD_SET(wflds_acc_inp_flistp, PIN_FLD_ACCESS_CODE2, poap, ebufp);
		}
		PIN_FLIST_FLD_SET(wflds_acc_inp_flistp, MSO_FLD_RMAIL, nmail, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_cntinfo write flds acc nmail input list", wflds_acc_inp_flistp);
        	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, wflds_acc_inp_flistp, &wflds_acc_out_flistp, ebufp);
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_cntinfo write flds acc nmail output list", wflds_acc_out_flistp);
		PIN_FLIST_DESTROY_EX(&wflds_acc_inp_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&wflds_acc_out_flistp, NULL);
	}
	
	args_flistp = PIN_FLIST_ELEM_GET(out_flist, PIN_FLD_DATA_ARRAY, 0, 1, ebufp );
	PIN_FLIST_FLD_COPY(readacct_outflistp, MSO_FLD_CONTACT_PREF, args_flistp, MSO_FLD_CONTACT_PREF, ebufp);
	PIN_FLIST_FLD_COPY(readacct_outflistp, MSO_FLD_AREA, args_flistp, MSO_FLD_AREA, ebufp);
	PIN_FLIST_FLD_COPY(readacct_outflistp, MSO_FLD_RMAIL, args_flistp, MSO_FLD_RMAIL, ebufp);
	PIN_FLIST_FLD_COPY(readacct_outflistp, MSO_FLD_RMN, args_flistp, MSO_FLD_RMN, ebufp);

	PIN_FLIST_ELEM_COPY(readacct_outflistp, PIN_FLD_NAMEINFO, 1, args_flistp, PIN_FLD_NAMEINFO, 1, ebufp);
	PIN_FLIST_ELEM_COPY(readacct_outflistp, PIN_FLD_NAMEINFO, 2, args_flistp, PIN_FLD_NAMEINFO, 2, ebufp);
	PIN_FLIST_ELEM_COPY(readacct_outflistp, PIN_FLD_NAMEINFO, 3, args_flistp, PIN_FLD_NAMEINFO, 3, ebufp);

	nameinfo_flistp = PIN_FLIST_ELEM_GET(readacct_outflistp, PIN_FLD_NAMEINFO, NAMEINFO_INSTALLATION, 1, ebufp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_custinfo inst_addr in input ", nameinfo_flistp);
	inst_addr_inp = PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_NAMEINFO, NAMEINFO_INSTALLATION, 1, ebufp );

	if (inst_addr_inp)
	{
		//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "00000");
		old_zip_code = PIN_FLIST_FLD_GET(nameinfo_flistp, PIN_FLD_ZIP, 1, ebufp);
		old_city = PIN_FLIST_FLD_GET( (pin_flist_t*)PIN_FLIST_ELEM_GET(readacct_outflistp, PIN_FLD_NAMEINFO, NAMEINFO_INSTALLATION, 1, ebufp )
		                              ,PIN_FLD_CITY, 1, ebufp);

		new_city = PIN_FLIST_FLD_GET( (pin_flist_t*)PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_NAMEINFO, NAMEINFO_INSTALLATION, 1, ebufp )
		                              ,PIN_FLD_CITY, 1, ebufp);
		
		//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "OLD_CITY:");
		//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, old_city);
		//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "NEW_CITY:");
		//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, new_city);

		if (home_move_flag == 0 && old_city && new_city && strcmp(old_city, new_city) != 0 )
		{
			/*======================================================================
			 Return if the count of active/inactive services is >0
			 
			 if OLD and NEW city are different then the change is allowed 
			 only when the account is having all the services in cancelled state
			 ======================================================================*/

			PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG,"acct_obj", acct_obj);
			srch_pdp = PIN_POID_CREATE( 1, "/search", -1, ebufp);
			search_inflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, srch_pdp, ebufp);
			PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);
			PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, template , ebufp);

			args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp );
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acct_obj, ebufp);

			args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp );
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &status, ebufp);
			
			args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 3, ebufp );
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &status_inactive, ebufp);
			
			results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp );
			PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID, NULL, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_service_info SEARCH input flist", search_inflistp);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
			PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
				PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
				return 0;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_service_info SEARCH output flist", search_outflistp);
			
			if (PIN_FLIST_ELEM_COUNT(search_outflistp, PIN_FLD_RESULTS, ebufp)  >0 )
			{
				PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
				PIN_FLIST_DESTROY_EX(&readacct_outflistp, NULL);
				return 4;
			}
			PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
			/*============================================================
			 Return if the count of active/inactive services is >0 
			 ===========================================================*/
		}

	}
	args_flistp = PIN_FLIST_ELEM_GET(out_flist, PIN_FLD_DATA_ARRAY, 1, 1, ebufp );
	PIN_FLIST_FLD_COPY(in_flist, MSO_FLD_CONTACT_PREF, args_flistp, MSO_FLD_CONTACT_PREF, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, MSO_FLD_AREA, args_flistp, MSO_FLD_AREA, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, MSO_FLD_RMAIL, args_flistp, MSO_FLD_RMAIL, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, MSO_FLD_RMN, args_flistp, MSO_FLD_RMN, ebufp);


	counter = PIN_FLIST_ELEM_COUNT(in_flist, PIN_FLD_NAMEINFO, ebufp);
	rmn = PIN_FLIST_FLD_GET(in_flist, MSO_FLD_RMN, 1, ebufp);
	rmail = PIN_FLIST_FLD_GET(in_flist, MSO_FLD_RMAIL, 1, ebufp);

	if ( rmn || rmail || counter > 0 )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling update contact info");
		cnt_upd = fm_mso_update_cntinfo(ctxp, in_flist, ebufp);
	}
	
	if ( cnt_upd == 3 )
		return 4;
	if ( cnt_upd == 2 )
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_custinfo : Contact history audit updated");	

	PIN_FLIST_ELEM_COPY(in_flist, PIN_FLD_NAMEINFO, 1, args_flistp, PIN_FLD_NAMEINFO, 1, ebufp);
	PIN_FLIST_ELEM_COPY(in_flist, PIN_FLD_NAMEINFO, 2, args_flistp, PIN_FLD_NAMEINFO, 2, ebufp);
	PIN_FLIST_ELEM_COPY(in_flist, PIN_FLD_NAMEINFO, 3, args_flistp, PIN_FLD_NAMEINFO, 3, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_custinfo input list 3", args_flistp);

	/********************************** Updating the nameinfo *****************************************/

	updacct_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, updacct_inflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_ACCOUNT_OBJ, updacct_inflistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, updacct_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);

	acct_flist = PIN_FLIST_ELEM_ADD(updacct_inflistp, PIN_FLD_ACCTINFO, 0, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, acct_flist, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, MSO_FLD_CONTACT_PREF, acct_flist, MSO_FLD_CONTACT_PREF, ebufp);
	if (PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_NAMEINFO, 2, 1, ebufp)) //if installation address is passed
	{
	       PIN_FLIST_FLD_COPY(in_flist, MSO_FLD_AREA, acct_flist, MSO_FLD_AREA, ebufp);
	}
	PIN_FLIST_FLD_COPY(in_flist, MSO_FLD_RMAIL, acct_flist, MSO_FLD_RMAIL, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, MSO_FLD_RMN, acct_flist, MSO_FLD_RMN, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_AAC_VENDOR, acct_flist, PIN_FLD_AAC_VENDOR, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_AAC_ACCESS, acct_flist, PIN_FLD_AAC_ACCESS, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_AAC_SERIAL_NUM, acct_flist, PIN_FLD_AAC_SERIAL_NUM, ebufp);

        acct_info_flistp = PIN_FLIST_ELEM_GET(in_flist,PIN_FLD_ACCTINFO, 0, 1, ebufp );       

        if(acct_info_flistp)
        { 
        	PIN_FLIST_FLD_COPY(acct_info_flistp,MSO_FLD_ET_ZONE, acct_flist,MSO_FLD_ET_ZONE, ebufp);
        	PIN_FLIST_FLD_COPY(acct_info_flistp,MSO_FLD_VAT_ZONE, acct_flist,MSO_FLD_VAT_ZONE, ebufp); 
        }	
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_LASTSTAT_CMNT, acct_flist, PIN_FLD_LASTSTAT_CMNT, ebufp);

	if ( PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_NAMEINFO, 1, 1, ebufp) )
	{
	       PIN_FLIST_ELEM_COPY(in_flist, PIN_FLD_NAMEINFO, 1, updacct_inflistp, PIN_FLD_NAMEINFO, 1, ebufp);
	}
	if ( PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_NAMEINFO, 2, 1, ebufp) )
	{
	      PIN_FLIST_ELEM_COPY(in_flist, PIN_FLD_NAMEINFO, 2, updacct_inflistp, PIN_FLD_NAMEINFO, 2, ebufp);
	}
 	if ( PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_NAMEINFO, 3, 1, ebufp) )
        {
               PIN_FLIST_ELEM_COPY(in_flist, PIN_FLD_NAMEINFO, 3, updacct_inflistp, PIN_FLD_NAMEINFO, 3, ebufp);
        }

	/****** 
         * Code to change the address in payinfo 
	 *****/
	if((PIN_FLIST_ELEM_COUNT(in_flist , PIN_FLD_PAYINFO,ebufp)) > 0)
	{
		PIN_FLIST_ELEM_COPY(in_flist, PIN_FLD_PAYINFO, 0, updacct_inflistp, PIN_FLD_PAYINFO, 0, ebufp);
		payinfo_flistp = PIN_FLIST_ELEM_GET(updacct_inflistp, PIN_FLD_PAYINFO, 0,1, ebufp);
		PIN_FLIST_FLD_RENAME(payinfo_flistp,PIN_FLD_PAYINFO_OBJ,PIN_FLD_POID,ebufp);
	}
	else if ( contact_pref || billing_addr_passed )
	{
		nameinfo_billing_inp = PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_NAMEINFO, NAMEINFO_BILLING, 1, ebufp );
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "nameinfo_billing_inp", nameinfo_billing_inp);

		billing_addr = PIN_FLIST_FLD_GET(nameinfo_billing_inp, PIN_FLD_ADDRESS, 1, ebufp);
		billing_city = PIN_FLIST_FLD_GET(nameinfo_billing_inp, PIN_FLD_CITY, 1, ebufp);
		billing_country = PIN_FLIST_FLD_GET(nameinfo_billing_inp, PIN_FLD_COUNTRY, 1, ebufp);
		email_addr = PIN_FLIST_FLD_GET(nameinfo_billing_inp, PIN_FLD_EMAIL_ADDR, 1, ebufp);
		billing_state = PIN_FLIST_FLD_GET(nameinfo_billing_inp, PIN_FLD_STATE, 1, ebufp);
		billing_zip = PIN_FLIST_FLD_GET(nameinfo_billing_inp, PIN_FLD_ZIP, 1, ebufp);
		// Added to populate the extra fields in payinfo_inv_t 
		area = PIN_FLIST_FLD_GET(nameinfo_billing_inp, MSO_FLD_AREA_NAME, 1, ebufp);
		building_name = PIN_FLIST_FLD_GET(nameinfo_billing_inp, MSO_FLD_BUILDING_NAME, 1, ebufp);
        building_idp = PIN_FLIST_FLD_GET(nameinfo_billing_inp, MSO_FLD_BUILDING_ID, 1, ebufp);
		landmark = PIN_FLIST_FLD_GET(nameinfo_billing_inp, MSO_FLD_LANDMARK, 1, ebufp);
		street = PIN_FLIST_FLD_GET(nameinfo_billing_inp, MSO_FLD_STREET_NAME, 1, ebufp);
		location = PIN_FLIST_FLD_GET(nameinfo_billing_inp, MSO_FLD_LOCATION_NAME, 1, ebufp);

		fn =  PIN_FLIST_FLD_GET(nameinfo_billing_inp, PIN_FLD_FIRST_NAME, 1, ebufp);
		mn =  PIN_FLIST_FLD_GET(nameinfo_billing_inp, PIN_FLD_MIDDLE_NAME, 1, ebufp);
		ln =  PIN_FLIST_FLD_GET(nameinfo_billing_inp, PIN_FLD_LAST_NAME, 1, ebufp);

		if (billing_addr || billing_city || billing_country || email_addr || billing_state || billing_zip || fn || mn || ln)
		{
			srch_payinfo_iflist = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(srch_payinfo_iflist, PIN_FLD_POID, (PIN_POID_CREATE( 1, "/search", -1, ebufp)), ebufp);
			PIN_FLIST_FLD_SET(srch_payinfo_iflist, PIN_FLD_FLAGS, &status_flags, ebufp);
			PIN_FLIST_FLD_SET(srch_payinfo_iflist, PIN_FLD_TEMPLATE, srch_template , ebufp);

			args_flistp = PIN_FLIST_ELEM_ADD(srch_payinfo_iflist, PIN_FLD_ARGS, 1, ebufp );
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acct_obj, ebufp);

			results_flistp = PIN_FLIST_ELEM_ADD(srch_payinfo_iflist, PIN_FLD_RESULTS, 0, ebufp );
			PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID, NULL, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search profile input flist", srch_payinfo_iflist);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_payinfo_iflist, &srch_payinfo_oflist, ebufp);
			PIN_FLIST_DESTROY_EX(&srch_payinfo_iflist, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in searching profile", ebufp);
				PIN_FLIST_DESTROY_EX(&srch_payinfo_oflist, NULL);
				return 0;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search profile output flist", srch_payinfo_oflist);

			results_flistp = PIN_FLIST_ELEM_GET(srch_payinfo_oflist, PIN_FLD_RESULTS, 0, 1, ebufp);
			if (results_flistp)
			{
				payinfo_obj =  PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_POID, 1, ebufp);
				if (payinfo_obj)
				{
					payinfo_array = PIN_FLIST_ELEM_ADD(updacct_inflistp, PIN_FLD_PAYINFO, 0, ebufp);
					PIN_FLIST_FLD_SET(payinfo_array, PIN_FLD_POID, payinfo_array, ebufp);
					inherited_info = PIN_FLIST_SUBSTR_ADD(payinfo_array, PIN_FLD_INHERITED_INFO, ebufp);
					PIN_FLIST_FLD_SET(payinfo_array, PIN_FLD_POID, payinfo_obj, ebufp);

					inv_info_arr = PIN_FLIST_ELEM_ADD(inherited_info, PIN_FLD_INV_INFO, 0, ebufp);
					if (contact_pref)
					{
						PIN_FLIST_FLD_SET(inv_info_arr, PIN_FLD_DELIVERY_PREFER, contact_pref, ebufp);
					}
					nameinfo_billing_inp = PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_NAMEINFO, NAMEINFO_BILLING, 1, ebufp );
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "nameinfo_billing_inp", nameinfo_billing_inp);
					if (nameinfo_billing_inp)
					{
						if (billing_addr)
						{
							PIN_FLIST_FLD_COPY(nameinfo_billing_inp, PIN_FLD_ADDRESS ,inv_info_arr, PIN_FLD_ADDRESS, ebufp);
						}
						if (billing_city)
						{
							PIN_FLIST_FLD_COPY(nameinfo_billing_inp, PIN_FLD_CITY ,inv_info_arr, PIN_FLD_CITY, ebufp);
						}
						if (billing_country)
						{
							PIN_FLIST_FLD_COPY(nameinfo_billing_inp, PIN_FLD_COUNTRY ,inv_info_arr, PIN_FLD_COUNTRY, ebufp);
						}
						if (email_addr)
						{
							PIN_FLIST_FLD_COPY(nameinfo_billing_inp, PIN_FLD_EMAIL_ADDR ,inv_info_arr, PIN_FLD_EMAIL_ADDR, ebufp);
						}
						if (billing_state)
						{
							PIN_FLIST_FLD_COPY(nameinfo_billing_inp, PIN_FLD_STATE ,inv_info_arr, PIN_FLD_STATE, ebufp);
						}
						if (billing_zip)
						{
							PIN_FLIST_FLD_COPY(nameinfo_billing_inp, PIN_FLD_ZIP ,inv_info_arr, PIN_FLD_ZIP, ebufp);
						}
						if(area)
						{
							PIN_FLIST_FLD_SET(inv_info_arr, MSO_FLD_AREA_NAME, area, ebufp);
						}
						if(building_name)
						{						
							PIN_FLIST_FLD_SET(inv_info_arr, MSO_FLD_BUILDING_NAME, building_name, ebufp);
						}	
                        if (building_idp)
                        {
                            PIN_FLIST_FLD_SET(inv_info_arr, MSO_FLD_BUILDING_ID, building_idp, ebufp);
                        }
						if(landmark)
						{			
							PIN_FLIST_FLD_SET(inv_info_arr, MSO_FLD_LANDMARK_NAME, landmark, ebufp);
						}		
						if(street)
						{		
							PIN_FLIST_FLD_SET(inv_info_arr, MSO_FLD_STREET_NAME, street, ebufp);
						}		
						if(location)
						{
							PIN_FLIST_FLD_SET(inv_info_arr, MSO_FLD_LOCATION_NAME, location, ebufp);
						}		
						if (fn && mn && ln )
						{	memset(updated_name,'\0',strlen(updated_name)); 
							strcpy(updated_name, fn);
							strcat(updated_name, " ");
							strcat(updated_name, mn);
							strcat(updated_name, " ");
							strcat(updated_name, ln);
							PIN_FLIST_FLD_SET(inv_info_arr, PIN_FLD_NAME, updated_name, ebufp);
						}
						else if (fn && ln)
						{
							memset(updated_name,'\0',strlen(updated_name)); 
							strcpy(updated_name, fn);
							strcat(updated_name, " ");
							vp = (char*)PIN_FLIST_FLD_GET( (pin_flist_t*)PIN_FLIST_ELEM_GET(readacct_outflistp, PIN_FLD_NAMEINFO, NAMEINFO_BILLING, 1, ebufp )
								,PIN_FLD_MIDDLE_NAME, 1, ebufp);
							strcat(updated_name, vp);
							strcat(updated_name, " ");
							strcat(updated_name, ln);
							PIN_FLIST_FLD_SET(inv_info_arr, PIN_FLD_NAME, updated_name, ebufp);
						}
						else if (fn && mn)
						{
							memset(updated_name,'\0',strlen(updated_name)); 
							strcpy(updated_name, fn);
							strcat(updated_name, " ");
							strcat(updated_name, mn);
							vp = (char*)PIN_FLIST_FLD_GET( (pin_flist_t*)PIN_FLIST_ELEM_GET(readacct_outflistp, PIN_FLD_NAMEINFO, NAMEINFO_BILLING, 1, ebufp )
								,PIN_FLD_LAST_NAME, 1, ebufp);
							strcat(updated_name, " ");
							strcat(updated_name, vp);
							PIN_FLIST_FLD_SET(inv_info_arr, PIN_FLD_NAME, updated_name, ebufp);
						}
						else if (mn && ln)
						{
							memset(updated_name,'\0',strlen(updated_name)); 
							vp = (char*)PIN_FLIST_FLD_GET( (pin_flist_t*)PIN_FLIST_ELEM_GET(readacct_outflistp, PIN_FLD_NAMEINFO, NAMEINFO_BILLING, 1, ebufp )
								,PIN_FLD_FIRST_NAME, 1, ebufp);
							strcpy(updated_name, vp);
							strcat(updated_name, " ");
							strcat(updated_name, mn);
							strcat(updated_name, " ");
							strcat(updated_name, ln);
							PIN_FLIST_FLD_SET(inv_info_arr, PIN_FLD_NAME, updated_name, ebufp);
						}
						else if (fn)
						{
							memset(updated_name,'\0',strlen(updated_name)); 
							strcpy(updated_name, fn);
							strcat(updated_name, " ");
							vp = (char*)PIN_FLIST_FLD_GET( (pin_flist_t*)PIN_FLIST_ELEM_GET(readacct_outflistp, PIN_FLD_NAMEINFO, NAMEINFO_BILLING, 1, ebufp )
								,PIN_FLD_MIDDLE_NAME, 1, ebufp);
							strcat(updated_name, vp);
							strcat(updated_name, " ");
							vp = (char*)PIN_FLIST_FLD_GET( (pin_flist_t*)PIN_FLIST_ELEM_GET(readacct_outflistp, PIN_FLD_NAMEINFO, NAMEINFO_BILLING, 1, ebufp )
								,PIN_FLD_LAST_NAME, 1, ebufp);
							strcat(updated_name, vp);
							PIN_FLIST_FLD_SET(inv_info_arr, PIN_FLD_NAME, updated_name, ebufp);
						}
						else if (mn)
						{
							memset(updated_name,'\0',strlen(updated_name)); 
							vp = (char*)PIN_FLIST_FLD_GET( (pin_flist_t*)PIN_FLIST_ELEM_GET(readacct_outflistp, PIN_FLD_NAMEINFO, NAMEINFO_BILLING, 1, ebufp )
								,PIN_FLD_FIRST_NAME, 1, ebufp);
							strcpy(updated_name, vp);
							strcat(updated_name, " ");
							strcat(updated_name, mn);
							strcat(updated_name, " ");
							vp = (char*)PIN_FLIST_FLD_GET( (pin_flist_t*)PIN_FLIST_ELEM_GET(readacct_outflistp, PIN_FLD_NAMEINFO, NAMEINFO_BILLING, 1, ebufp )
								,PIN_FLD_LAST_NAME, 1, ebufp);
							strcat(updated_name, vp);
							PIN_FLIST_FLD_SET(inv_info_arr, PIN_FLD_NAME, updated_name, ebufp);
						}
						else if (ln)
						{
							memset(updated_name,'\0',strlen(updated_name)); 
							vp = (char*)PIN_FLIST_FLD_GET( (pin_flist_t*)PIN_FLIST_ELEM_GET(readacct_outflistp, PIN_FLD_NAMEINFO, NAMEINFO_BILLING, 1, ebufp )
								,PIN_FLD_FIRST_NAME, 1, ebufp);
							strcpy(updated_name, vp);
							strcat(updated_name, " ");
							vp = (char*)PIN_FLIST_FLD_GET( (pin_flist_t*)PIN_FLIST_ELEM_GET(readacct_outflistp, PIN_FLD_NAMEINFO, NAMEINFO_BILLING, 1, ebufp )
								,PIN_FLD_MIDDLE_NAME, 1, ebufp);
							strcat(updated_name, vp);
							strcat(updated_name, " ");
							strcat(updated_name, ln);
							PIN_FLIST_FLD_SET(inv_info_arr, PIN_FLD_NAME, updated_name, ebufp);
						}
					}
				}
			}
			PIN_FLIST_DESTROY_EX(&srch_payinfo_oflist, NULL);
		}
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_custinfo input list", updacct_inflistp);
	PCM_OP(ctxp, PCM_OP_CUST_UPDATE_CUSTOMER, 0, updacct_inflistp, &updacct_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&updacct_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in updating /payinfo : PCM_OP_CUST_UPDATE_CUSTOMER", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);
		PIN_FLIST_DESTROY_EX(&readacct_outflistp, NULL);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_custinfo output flist", updacct_outflistp);
	PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);
	
	if ( cr == 1 )
	{
		cnt_upd = fm_mso_update_payinfo_inv( ctxp, in_flist, ebufp);	
		if ( cnt_upd == 3 )
			return 7;
		else if ( cnt_upd == 2 ) 
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Payinfo Invoice info updated :");	
	}	

	/*************************** Provisoning calling *************************************/	

	if (zip_code && (strcmp(zip_code, "") != 0))
	{
		if (old_zip_code && (strcmp(zip_code, old_zip_code) != 0))
		{				
			db = PIN_POID_GET_DB(acct_obj);
			search_inflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);
			PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);

			args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp );
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, args_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);

			args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp );
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &status, ebufp);

			args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 3, ebufp );
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &status_inactive, ebufp);

			results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_custinfo search service input list", search_inflistp);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
			PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&readacct_outflistp, NULL);
				PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
				PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);
				return 0;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_custinfo search service output flist", search_outflistp);	
			
			results_flistp = PIN_FLIST_ELEM_GET(search_outflistp, PIN_FLD_RESULTS, 0, 1, ebufp );
			if (results_flistp)
			{
				service_obj = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_POID, 1, ebufp );
			}
			while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(search_outflistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp )) != NULL)
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"results_flistp", results_flistp);
				srvc_status = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_STATUS ,1, ebufp);
				if (srvc_status && *(int32*)srvc_status == PIN_STATUS_INACTIVE )
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST_000000");
					PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
					PIN_FLIST_DESTROY_EX(&readacct_outflistp, NULL);
					return 5;
				}
			}


			elem_id = 0;
			cookie = NULL;
			while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(search_outflistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp )) != NULL)
			{
				catv_info = PIN_FLIST_SUBSTR_GET(results_flistp, MSO_FLD_CATV_INFO, 1, ebufp);
				network_node = PIN_FLIST_FLD_GET(catv_info, MSO_FLD_NETWORK_NODE, 1, ebufp );
				/*old_bouquet_id = PIN_FLIST_FLD_GET(catv_info, MSO_FLD_BOUQUET_ID, 1, ebufp );

				if (network_node && strstr(network_node, "NDS"))
				{
					alias_list = PIN_FLIST_ELEM_GET(search_outflistp, PIN_FLD_ALIAS_LIST, 1, 1, ebufp);
					if (alias_list)
					{
						device_id = PIN_FLIST_FLD_GET(alias_list, PIN_FLD_NAME, 1, ebufp);
						if (device_id)
						{
							fm_mso_get_bouquet_id(ctxp, device_id, in_flist, ebufp);
						}
					}
				}*/

				svcwrite_inflistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, svcwrite_inflistp, PIN_FLD_POID, ebufp);

				args_flistp = PIN_FLIST_ELEM_ADD(svcwrite_inflistp, MSO_FLD_CATV_INFO, 0, ebufp );

				/**************** Read ACCT object *******************/
				readacct_inflistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, 
						readacct_inflistp, PIN_FLD_POID, ebufp);

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
					"Account read account input list", readacct_inflistp);
				PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, readacct_inflistp, 
									&racct_outflistp, ebufp);
				PIN_FLIST_DESTROY_EX(&readacct_inflistp, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
				"Error in accessing account or the account does not exist", ebufp);
						PIN_ERRBUF_RESET(ebufp);
						PIN_FLIST_DESTROY_EX(&racct_outflistp, NULL);
						return 0;
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
					"Account read account output flist", racct_outflistp);
				  
				if(racct_outflistp)
				{
					business_type = (int32 *)PIN_FLIST_FLD_GET(racct_outflistp, 
								PIN_FLD_BUSINESS_TYPE, 1, ebufp);
				}
				sprintf(bus_type_str, "%ld", *business_type);

				if ( bus_type_str  
					&& strncmp(bus_type_str, "9930", strlen("9930")) == 0)
				{
          			  strcpy(region_key, "HB");
				}
        			else
				{
				  strcpy(region_key, "IN");
				}
        			strcat(region_key, zip_code);			
				PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_REGION_KEY, region_key, ebufp);		

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_custinfo write svc input list", svcwrite_inflistp);
				PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, svcwrite_inflistp, &svcwrite_outflistp, ebufp);
				PIN_FLIST_DESTROY_EX(&svcwrite_inflistp, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_WRITE_FLDS svc for bouquet ID", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					PIN_FLIST_DESTROY_EX(&svcwrite_outflistp, NULL);
					PIN_FLIST_DESTROY_EX(&readacct_outflistp, NULL);
					PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
					PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);
					return 0;
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_custinfo write svc output flist", svcwrite_outflistp);
				PIN_FLIST_DESTROY_EX(&svcwrite_outflistp, NULL);

				/***************************** provisioning call ******************************************/

				status_flags = 10;

				provaction_inflistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);				
				PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, provaction_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
				PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);	

				if (network_node && strstr(network_node, "NDS"))
				{
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_custinfo provisioning input list", provaction_inflistp);
					PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
					PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
					if (PIN_ERRBUF_IS_ERR(ebufp))
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_CREATE_ACTION ", ebufp);
						PIN_ERRBUF_RESET(ebufp);
						PIN_FLIST_DESTROY_EX(&readacct_outflistp, NULL);
						PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
						PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);
						PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
						return 0;
					}
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_custinfo provisioning output flist", provaction_outflistp);
					PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
				}
				else
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "MSO_OP_PROV_CREATE_ACTION is not being called as MSO_FLD_NETWORK_NODE != NDS");
				}
			}			

			if (!service_obj)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_custinfo there is no service attached to this account");
				goto CLEANUP;
			}

			
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_custinfo there is no need of provisoning as ZIP number is same");
		}
	}

CLEANUP:
	PIN_FLIST_DESTROY_EX(&readacct_outflistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
	PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);
	return 2;
}

/**************************************************
* Function: fm_mso_update_billinfo()
* 
* 
***************************************************/
static int 
fm_mso_update_billinfo(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		*out_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*updacct_inflistp = NULL;
	pin_flist_t		*updacct_outflistp = NULL;
	pin_flist_t		*readbillinfo_inflistp = NULL;
	pin_flist_t		*readbillinfo_outflistp = NULL;
	pin_flist_t		*billinfo_flist = NULL;
	pin_flist_t		*set_billinfo_flist = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*args_flistp2 = NULL;
	pin_flist_t		*read_payinfo_iflist = NULL;
	pin_flist_t		*read_payinfo_oflist = NULL;
	pin_flist_t		*result_flist = NULL;

	int			*bill_when = NULL;
	int			*old_bill_when = NULL;

	int32			*indicator = NULL;
	int32			subscriber_type = -1;
	int32			*pay_type = NULL;
	int32			*called_by_parent = NULL;
	int32			actgcycles_left = 1;
	int32			corporate_type = 0;

	poid_t			*acct_obj = NULL;
	poid_t			*billinfo_obj = NULL;
	poid_t			*last_bill_obj = NULL;

	time_t			*next_bill_t = NULL;
	time_t			future_bill_t = 0;
	struct tm       *timeeff;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_billinfo :", in_flist);	
	
	billinfo_flist = PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_BILLINFO, PIN_ELEMID_ANY, 1, ebufp );
	acct_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp );

	if (!billinfo_flist)
	{
		return 1;		
	}
	else 
	{
		bill_when = PIN_FLIST_FLD_GET(billinfo_flist, PIN_FLD_BILL_WHEN, 1, ebufp );
		if (!bill_when)
		{
			return 1;
		}
		billinfo_obj = PIN_FLIST_FLD_GET(billinfo_flist, PIN_FLD_BILLINFO_OBJ, 1, ebufp );
		if (!billinfo_obj)
		{
			return 1;
		}
	}

	indicator = (int32*)PIN_FLIST_FLD_GET(billinfo_flist, PIN_FLD_INDICATOR, 1, ebufp );
	if (indicator && *indicator == 0)
	{
	 	PIN_FLIST_FLD_SET(in_flist, PIN_FLD_ERROR_DESCR, "PAY-TERM change allowed for prepaid accounts only", ebufp);
		PIN_FLIST_DESTROY_EX(&readbillinfo_outflistp, NULL);
		return 0;
	}

	/************************************** Read Billinfo object ****************************************/

	readbillinfo_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(readbillinfo_inflistp, PIN_FLD_POID, billinfo_obj, ebufp);
	PIN_FLIST_FLD_SET(readbillinfo_inflistp, PIN_FLD_BILL_WHEN, NULL, ebufp);
	PIN_FLIST_FLD_SET(readbillinfo_inflistp, PIN_FLD_LAST_BILL_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(readbillinfo_inflistp, PIN_FLD_PAYINFO_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(readbillinfo_inflistp, PIN_FLD_PAY_TYPE, NULL, ebufp);
	PIN_FLIST_FLD_SET(readbillinfo_inflistp, PIN_FLD_NEXT_BILL_T, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_billinfo read billinfo input list", readbillinfo_inflistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, readbillinfo_inflistp, &readbillinfo_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&readbillinfo_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS billinfo", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&readbillinfo_outflistp, NULL);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_billinfo read billinfo output flist", readbillinfo_outflistp);

	last_bill_obj = PIN_FLIST_FLD_GET(readbillinfo_outflistp, PIN_FLD_LAST_BILL_OBJ, 1, ebufp );
	pay_type = (int32*)PIN_FLIST_FLD_GET(readbillinfo_outflistp, PIN_FLD_PAY_TYPE, 1, ebufp );
	subscriber_type = *(int32*)PIN_FLIST_FLD_GET(in_flist, MSO_FLD_SUBSCRIBER_TYPE, 0, ebufp );
	next_bill_t = PIN_FLIST_FLD_GET(readbillinfo_outflistp, PIN_FLD_NEXT_BILL_T, 1, ebufp );
	timeeff = localtime(next_bill_t);

//	called_by_parent = (int32*)PIN_FLIST_FLD_GET(in_flist, PIN_FLD_PARENT_FLAGS, 1, ebufp );
	
	if (pay_type && *pay_type == 10007 )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "pay_type=10007");
		PIN_FLIST_FLD_SET(in_flist, PIN_FLD_ERROR_DESCR, "PAY-TERM change for non-paying child is allowed from corporate parent profile only", ebufp);
		PIN_FLIST_DESTROY_EX(&readbillinfo_outflistp, NULL);
		return 0;
	}

	if (  corporate_type != CORPORATE_PARENT &&
	     (PIN_POID_GET_ID(last_bill_obj) == 0) 
	   )
	{
		PIN_FLIST_FLD_SET(in_flist, PIN_FLD_ERROR_DESCR, "No Bills Exist: Not allowed to change Billing information", ebufp);
		PIN_FLIST_DESTROY_EX(&readbillinfo_outflistp, NULL);
		return 0;
	}

	old_bill_when = PIN_FLIST_FLD_GET(readbillinfo_outflistp, PIN_FLD_BILL_WHEN, 1, ebufp );

	if (*(int*)bill_when == *(int*)old_bill_when)
	{
		PIN_FLIST_DESTROY_EX(&readbillinfo_outflistp, NULL);
		return 1;
	}
	/**********************************Allow bill when modification only if account is prepaid ****************************************/

	if (pay_type && *pay_type != 10007 )
	{
		read_payinfo_iflist = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(readbillinfo_outflistp, PIN_FLD_PAYINFO_OBJ, read_payinfo_iflist, PIN_FLD_POID, ebufp);
		PIN_FLIST_ELEM_SET(read_payinfo_iflist, NULL, PIN_FLD_INV_INFO, 0, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_billinfo read_payinfo_iflist", read_payinfo_iflist);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_payinfo_iflist, &read_payinfo_oflist, ebufp);
		PIN_FLIST_DESTROY_EX(&read_payinfo_iflist, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&read_payinfo_oflist, NULL);
			return 0;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_billinfo read_payinfo_oflist", read_payinfo_oflist);

		result_flist = PIN_FLIST_ELEM_GET(read_payinfo_oflist, PIN_FLD_INV_INFO, 0, 1, ebufp);
		if (result_flist && bill_when )
		{
			indicator = (int32*)PIN_FLIST_FLD_GET(result_flist, PIN_FLD_INDICATOR, 1, ebufp);
			if (indicator && *indicator != 1)
			{
				PIN_FLIST_FLD_SET(in_flist, PIN_FLD_ERROR_DESCR, "PAY TERM modification allowed only for prepaid accounts", ebufp);
				PIN_FLIST_DESTROY_EX(&read_payinfo_oflist, NULL);
				PIN_FLIST_DESTROY_EX(&readbillinfo_outflistp, NULL);
				return 0;
			}
		}
		PIN_FLIST_DESTROY_EX(&read_payinfo_oflist, NULL);
		PIN_FLIST_DESTROY_EX(&readbillinfo_outflistp, NULL);
	}

	args_flistp = PIN_FLIST_ELEM_GET(out_flist, PIN_FLD_DATA_ARRAY, 0, 1, ebufp );
	args_flistp2 = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_BILLINFO, 0, ebufp );
	PIN_FLIST_FLD_SET(args_flistp2, PIN_FLD_BILLINFO_OBJ, billinfo_obj, ebufp);
	PIN_FLIST_FLD_SET(args_flistp2, PIN_FLD_BILL_WHEN, old_bill_when, ebufp );

	args_flistp = PIN_FLIST_ELEM_GET(out_flist, PIN_FLD_DATA_ARRAY, 1, 1, ebufp );
	args_flistp2 = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_BILLINFO, 0, ebufp );
	PIN_FLIST_FLD_SET(args_flistp2, PIN_FLD_BILLINFO_OBJ, billinfo_obj, ebufp);
	PIN_FLIST_FLD_SET(args_flistp2, PIN_FLD_BILL_WHEN, bill_when, ebufp );

	PIN_FLIST_DESTROY_EX(&readbillinfo_outflistp, NULL);
	if((bill_when) && (*bill_when > 0))
	{	int count ;
		count = *bill_when ;
		timeeff->tm_mon = timeeff->tm_mon + count;
		future_bill_t = mktime(timeeff);

	}
	updacct_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(billinfo_flist, PIN_FLD_BILLINFO_OBJ, updacct_inflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(updacct_inflistp, PIN_FLD_BILL_WHEN,bill_when, ebufp);
	PIN_FLIST_FLD_SET(updacct_inflistp, PIN_FLD_FUTURE_BILL_T,&future_bill_t, ebufp);

	/* Commented call to CUST_MODIFY_CUSTOMER as it immediately changes next_Bill_T 
	*  Expectation is to change next_bill_T after then one billing after payterm change */

	/*PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, updacct_inflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, updacct_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);
	//PIN_FLIST_ELEM_COPY(in_flist, PIN_FLD_BILLINFO, PIN_ELEMID_ANY, updacct_inflistp, PIN_FLD_BILLINFO, PIN_ELEMID_ANY, ebufp);

	set_billinfo_flist = PIN_FLIST_ELEM_ADD(updacct_inflistp, PIN_FLD_BILLINFO, 0, ebufp);
	PIN_FLIST_FLD_COPY(billinfo_flist, PIN_FLD_BILL_WHEN, set_billinfo_flist, PIN_FLD_BILL_WHEN, ebufp);
	PIN_FLIST_FLD_COPY(billinfo_flist, PIN_FLD_BILLINFO_OBJ, set_billinfo_flist, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(set_billinfo_flist, PIN_FLD_BILL_ACTGCYCLES_LEFT, &actgcycles_left, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_billinfo input list", updacct_inflistp);
	PCM_OP(ctxp, PCM_OP_CUST_SET_BILLINFO, 0, updacct_inflistp, &updacct_outflistp, ebufp);*/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_billinfo input list", updacct_inflistp);
    PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, updacct_inflistp, &updacct_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&updacct_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_WRITE_FLDS", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_billinfo output flist", updacct_outflistp);
	PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);
	return 2;
}


/**************************************************
* Function: fm_mso_update_serviceinfo()
* 
* 
***************************************************/
static int 
fm_mso_update_serviceinfo(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		*out_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*updacct_inflistp = NULL;
	pin_flist_t		*updacct_outflistp = NULL;
	pin_flist_t		*services_flist = NULL;
	pin_flist_t		*updservices_flist = NULL;
	pin_flist_t		*inheritedinfo_flist = NULL;
	pin_flist_t		*service_catinfo_flistp = NULL;
	pin_flist_t		*readsvc_inflistp = NULL;
	pin_flist_t		*readsvc_outflistp = NULL;
	pin_flist_t		*read_catvinfo_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*provaction_inflistp = NULL;
	pin_flist_t		*provaction_outflistp = NULL;
	pin_flist_t		*personal_bit_flist = NULL;
	pin_flist_t		*bal_grp_oflist	= NULL;
	pin_flist_t		*credit_limit_array = NULL;
  pin_flist_t   *bitfld_inflistp = NULL;
  pin_flist_t   *bitfld_outflistp = NULL;



	poid_t			*service_obj = NULL;
	poid_t			*bal_grp_obj = NULL;
	int			elem_id = 0;
	int			elem_id2 = 0;
	int			*status = NULL;
	int			status_flags = 0;
	int32			errorCode= 0;
	int32			*credit_profile_id = NULL;

	char			*new_bouquet_id = NULL;
	char			*old_bouquet_id = NULL;
	char			errorMsg[80]    = "";
  char      *bit_name_fld = NULL;
  char      *bit_val_fld = NULL;
	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie1 = NULL;

	pin_decimal_t		*credit_limit     = NULL;
	pin_decimal_t		*credit_floor	  = NULL;
	pin_decimal_t		*new_credit_limit_brm     = NULL;
	pin_decimal_t		*new_credit_floor_brm  = NULL;
	pin_decimal_t		*credit_limit_old = NULL;
	pin_decimal_t		*credit_floor_old = NULL;
	pin_decimal_t		*old_credit_limit_brm = NULL;
	pin_decimal_t		*old_credit_floor_brm = NULL;
	pin_decimal_t		*limit_max = pbo_decimal_from_str(CREDIT_LIMIT_MAX_VAL_STR,ebufp);
	pin_decimal_t		*floor_min = pbo_decimal_from_str(CREDIT_FLOOR_MIN_VAL_STR,ebufp); 

	pin_fld_num_t		field_no = 0;

	void			*vp  = NULL;
	void			*vp1 = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_serviceinfo :");	

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Input flist in update service info is ", in_flist);
	
	services_flist = PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_SERVICES, PIN_ELEMID_ANY, 1, ebufp );

	if (!services_flist)
	{
		return 1;		
	}
	else 
	{
		service_obj = PIN_FLIST_FLD_GET(services_flist, PIN_FLD_SERVICE_OBJ, 1, ebufp );
		if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_BB)) == 0 )
		return 1; 
		if (!service_obj)
		{
			bal_grp_obj = PIN_FLIST_FLD_GET(services_flist, PIN_FLD_BAL_GRP_OBJ, 1, ebufp );
			if (!bal_grp_obj)
			{
				return 1;
			}			
		}
	}
	
	updacct_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, updacct_inflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, updacct_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);

	while ((services_flist = PIN_FLIST_ELEM_GET_NEXT(in_flist, PIN_FLD_SERVICES, &elem_id, 1, &cookie, ebufp )) != NULL)
	{
		/*********************************** CReading service details********************************/

		if (service_obj)
		{
			status = PIN_FLIST_FLD_GET(services_flist, PIN_FLD_STATUS, 1, ebufp );
			
			//personal_bit_flist = PIN_FLIST_ELEM_GET(services_flist, MSO_FLD_PERSONAL_BIT_INFO, PIN_ELEMID_ANY, ebufp);

			readsvc_inflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(readsvc_inflistp, PIN_FLD_POID, service_obj, ebufp);
			PIN_FLIST_FLD_SET(readsvc_inflistp, PIN_FLD_BAL_GRP_OBJ, NULL, ebufp);
			if (status)
			{
				PIN_FLIST_FLD_SET(readsvc_inflistp, PIN_FLD_STATUS, NULL, ebufp);
			}
			inheritedinfo_flist = PIN_FLIST_ELEM_ADD(readsvc_inflistp, MSO_FLD_CATV_INFO, 0, ebufp );
			PIN_FLIST_FLD_SET(inheritedinfo_flist, MSO_FLD_AGREEMENT_NO, NULL, ebufp);
			PIN_FLIST_FLD_SET(inheritedinfo_flist, MSO_FLD_BOUQUET_ID, NULL, ebufp);
			PIN_FLIST_FLD_SET(inheritedinfo_flist, MSO_FLD_CARF_RECEIVED, NULL, ebufp);
			PIN_FLIST_FLD_SET(inheritedinfo_flist, MSO_FLD_CAS_SUBSCRIBER_ID, NULL, ebufp);
			PIN_FLIST_FLD_SET(inheritedinfo_flist, MSO_FLD_NETWORK_NODE, NULL, ebufp);
			PIN_FLIST_FLD_SET(inheritedinfo_flist, MSO_FLD_REGION_KEY, NULL, ebufp);
			PIN_FLIST_FLD_SET(inheritedinfo_flist, PIN_FLD_CONN_TYPE, NULL, ebufp);
			//PIN_FLIST_ELEM_SET(readsvc_inflistp, NULL, MSO_FLD_PERSONAL_BIT_INFO, PIN_ELEMID_ANY, ebufp );

			elem_id2 = 0;
			cookie1 = NULL;
			while ((personal_bit_flist = PIN_FLIST_ELEM_GET_NEXT(services_flist, MSO_FLD_PERSONAL_BIT_INFO,&elem_id2,1,&cookie1,ebufp))!= NULL)
			{
				//inheritedinfo_flist = PIN_FLIST_ELEM_ADD(readsvc_inflistp, MSO_FLD_PERSONAL_BIT_INFO, 0, ebufp );		
				inheritedinfo_flist = PIN_FLIST_ELEM_ADD(readsvc_inflistp, MSO_FLD_PERSONAL_BIT_INFO,elem_id2,ebufp );
			}		

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_serviceinfo read svc for bgp input list", readsvc_inflistp);
			PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, readsvc_inflistp, &readsvc_outflistp, ebufp);
			PIN_FLIST_DESTROY_EX(&readsvc_inflistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS svc for bgp", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
				return 0;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_serviceinfo read svc for bgp output flist", readsvc_outflistp);

			if (!readsvc_outflistp)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_serviceinfo read svc output flist is not having any service details");
				return 0;
			}

			read_catvinfo_flistp = PIN_FLIST_SUBSTR_GET(readsvc_outflistp, MSO_FLD_CATV_INFO, 0, ebufp );
			old_bouquet_id = PIN_FLIST_FLD_GET(read_catvinfo_flistp, MSO_FLD_BOUQUET_ID, 1, ebufp );
			PIN_FLIST_FLD_COPY(read_catvinfo_flistp, MSO_FLD_AGREEMENT_NO, out_flist, MSO_FLD_AGREEMENT_NO, ebufp);

			args_flistp = PIN_FLIST_ELEM_GET(out_flist, PIN_FLD_DATA_ARRAY, 0, 1, ebufp );
			inheritedinfo_flist = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_SERVICES, elem_id, ebufp );
			PIN_FLIST_FLD_SET(inheritedinfo_flist, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
			PIN_FLIST_SUBSTR_SET(inheritedinfo_flist, read_catvinfo_flistp, MSO_FLD_CATV_INFO, ebufp );

			//read_catvinfo_flistp = PIN_FLIST_ELEM_GET(readsvc_outflistp, MSO_FLD_PERSONAL_BIT_INFO, 0, 1, ebufp );
			read_catvinfo_flistp = PIN_FLIST_ELEM_GET(readsvc_outflistp, MSO_FLD_PERSONAL_BIT_INFO,PIN_ELEMID_ANY, 1, ebufp );
			//PIN_FLIST_ELEM_SET(inheritedinfo_flist, read_catvinfo_flistp, MSO_FLD_PERSONAL_BIT_INFO, 0, ebufp );
			if(read_catvinfo_flistp)
			PIN_FLIST_ELEM_SET(inheritedinfo_flist, read_catvinfo_flistp, MSO_FLD_PERSONAL_BIT_INFO,elem_id2, ebufp );
			PIN_FLIST_FLD_COPY(readsvc_outflistp, PIN_FLD_STATUS, inheritedinfo_flist, PIN_FLD_STATUS, ebufp);		

			args_flistp = PIN_FLIST_ELEM_GET(out_flist, PIN_FLD_DATA_ARRAY, 1, 1, ebufp );
			inheritedinfo_flist = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_SERVICES, elem_id, ebufp );
			PIN_FLIST_FLD_SET(inheritedinfo_flist, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);

			service_catinfo_flistp = PIN_FLIST_ELEM_GET(services_flist, MSO_FLD_CATV_INFO, 0, 1, ebufp );
			PIN_FLIST_ELEM_SET(inheritedinfo_flist, service_catinfo_flistp, MSO_FLD_CATV_INFO, 0, ebufp );

			service_catinfo_flistp = PIN_FLIST_ELEM_GET(services_flist, MSO_FLD_PERSONAL_BIT_INFO,PIN_ELEMID_ANY ,1, ebufp );
			if(service_catinfo_flistp != NULL ) 
			{
				PIN_FLIST_ELEM_SET(inheritedinfo_flist, service_catinfo_flistp, MSO_FLD_PERSONAL_BIT_INFO,elem_id2, ebufp );
			}
			PIN_FLIST_FLD_COPY(services_flist, PIN_FLD_STATUS, inheritedinfo_flist, PIN_FLD_STATUS, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_serviceinfo prepared output flist", out_flist);

			/************************************************************************************************************/

			updservices_flist = PIN_FLIST_ELEM_ADD(updacct_inflistp, PIN_FLD_SERVICES, elem_id, ebufp);
			PIN_FLIST_FLD_COPY(services_flist, PIN_FLD_SERVICE_OBJ, updservices_flist, PIN_FLD_POID, ebufp);

			inheritedinfo_flist = PIN_FLIST_SUBSTR_ADD(updservices_flist, PIN_FLD_INHERITED_INFO , ebufp);

			service_catinfo_flistp = PIN_FLIST_ELEM_GET(services_flist, MSO_FLD_CATV_INFO, 0, 1, ebufp );
			if (service_catinfo_flistp)
			{
				new_bouquet_id = PIN_FLIST_FLD_GET(service_catinfo_flistp, MSO_FLD_BOUQUET_ID, 1, ebufp );
				PIN_FLIST_ELEM_SET(inheritedinfo_flist, service_catinfo_flistp, MSO_FLD_CATV_INFO, 0, ebufp);	
			}

			//service_catinfo_flistp = PIN_FLIST_ELEM_GET(services_flist, MSO_FLD_PERSONAL_BIT_INFO, 0, 1, ebufp );
			service_catinfo_flistp = PIN_FLIST_ELEM_GET(services_flist, MSO_FLD_PERSONAL_BIT_INFO,PIN_ELEMID_ANY, 1, ebufp );
			if (service_catinfo_flistp)
			{
				//PIN_FLIST_ELEM_SET(inheritedinfo_flist, service_catinfo_flistp, MSO_FLD_PERSONAL_BIT_INFO, 0, ebufp);	
				PIN_FLIST_ELEM_SET(inheritedinfo_flist, service_catinfo_flistp, MSO_FLD_PERSONAL_BIT_INFO,elem_id2, ebufp);	
			}

			if (inheritedinfo_flist && PIN_FLIST_COUNT(inheritedinfo_flist, ebufp) >0)
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_serviceinfo input list", updacct_inflistp);
				PCM_OP(ctxp, PCM_OP_CUST_UPDATE_SERVICES, 0, updacct_inflistp, &updacct_outflistp, ebufp);				
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_CUST_UPDATE_SERVICES", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);
					return 0;
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_serviceinfo output flist", updacct_outflistp);
				PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);
			}
			PIN_FLIST_DESTROY_EX(&updacct_inflistp, NULL);

			/*********************************** status change ********************************/

			status = PIN_FLIST_FLD_GET(services_flist, PIN_FLD_STATUS, 1, ebufp );

			if (status)
			{
				updacct_inflistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, updacct_inflistp, PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_USERID, updacct_inflistp, PIN_FLD_USERID, ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, updacct_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);
				PIN_FLIST_FLD_COPY(services_flist, PIN_FLD_SERVICE_OBJ, updacct_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
				PIN_FLIST_FLD_COPY(services_flist, PIN_FLD_STATUS, updacct_inflistp, PIN_FLD_STATUS, ebufp);
				PIN_FLIST_FLD_SET(updacct_inflistp, PIN_FLD_DESCR, "Service status change from Modify customer opcode", ebufp );

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_serviceinfo Modify Service status", updacct_inflistp);
					
				if (*(int*)status == PIN_STATUS_ACTIVE)
				{
					PCM_OP(ctxp, MSO_OP_CUST_REACTIVATE_SERVICE, 0, updacct_inflistp, &updacct_outflistp, ebufp);
				}
				else if (*(int*)status == PIN_STATUS_INACTIVE)
				{
					PCM_OP(ctxp, MSO_OP_CUST_SUSPEND_SERVICE, 0, updacct_inflistp, &updacct_outflistp, ebufp);
				}
				else if (*(int*)status == PIN_STATUS_CLOSED)
				{
					PCM_OP(ctxp, MSO_OP_CUST_TERMINATE_SERVICE, 0, updacct_inflistp, &updacct_outflistp, ebufp);
				}					
				
				PIN_FLIST_DESTROY_EX(&updacct_inflistp, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_CUST_MODIFY_SERVICE_STATUS", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);
					return 0;
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_serviceinfo Modify Service status", updacct_outflistp);
				PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);			

			}
			/********************************************** 
				Provisoning calling for PERSONAL BIT 
			**********************************************/	
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "service_catinfo_flistp list", service_catinfo_flistp);
			if( service_catinfo_flistp != NULL )
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "set BIT");
				bit_val_fld = PIN_FLIST_FLD_GET(service_catinfo_flistp, PIN_FLD_PARAM_VALUE, 1, ebufp);
				bit_name_fld = PIN_FLIST_FLD_GET(service_catinfo_flistp, PIN_FLD_PARAM_NAME, 1, ebufp);

				if (bit_val_fld && (strcmp(bit_val_fld,"A") == 0))
				{
					status_flags = CATV_SET_BIT;
				}
				else if (bit_val_fld && (strcmp(bit_val_fld,"D") == 0))
				{
					status_flags = CATV_UNSET_BIT;
				}
       			else
        		{
        		}
				bitfld_inflistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, bitfld_inflistp, PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_SET(bitfld_inflistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
				if(bit_name_fld) PIN_FLIST_FLD_SET(bitfld_inflistp, MSO_FLD_REGION_NUMBER, bit_name_fld, ebufp);	
				PIN_FLIST_FLD_SET(bitfld_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);	

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "personal bit input list", bitfld_inflistp);
				PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, bitfld_inflistp, & bitfld_outflistp, ebufp);
				PIN_FLIST_DESTROY_EX(&bitfld_inflistp, NULL);
				PIN_FLIST_DESTROY_EX(&bitfld_outflistp, NULL);
			}
			/*************************** 
				Provisoning calling bouquet_id
			*************************************/	
			if (new_bouquet_id && (strcmp(new_bouquet_id, "") != 0))
			{
				if (old_bouquet_id && (strcmp(new_bouquet_id, old_bouquet_id) != 0))
				{	
					status_flags = 19;

					provaction_inflistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, provaction_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);
					PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
					PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);	
					PIN_FLIST_FLD_SET(provaction_inflistp, MSO_FLD_BOUQUET_ID, new_bouquet_id, ebufp);
					PIN_FLIST_FLD_SET(provaction_inflistp, MSO_FLD_BOUQUET_ID_HEX, old_bouquet_id, ebufp);

					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_serviceinfo provisioning input list", provaction_inflistp);
					PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
					PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
					if (PIN_ERRBUF_IS_ERR(ebufp))
					{
						errorCode = ebufp->pin_err;
						//errorDesc = (char *)PIN_PINERR_TO_STR(errorCode);
						field_no = ebufp->field;
						//errorField = (char *)PIN_FIELD_GET_NAME(field_no);

						memset(errorMsg,'\0',strlen(errorMsg));
						//strcpy(errorMsg,errorField);
						//strcat(errorMsg,": ");
						//strcat(errorMsg,errorDesc);
						  
						if (errorCode == PIN_ERR_BAD_VALUE && field_no == MSO_FLD_CAS_SUBSCRIBER_ID )
						{
							strcpy(errorMsg,"CAS_SUBSCRIBER_ID is Invalid or NULL");
							PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, errorMsg, ebufp);
							PIN_ERRBUF_RESET(ebufp);
							PIN_FLIST_FLD_SET(out_flist, PIN_FLD_ERROR_DESCR, errorMsg, ebufp );
							PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
							return 0;
						}
						else
						{
							PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_CREATE_ACTION ", ebufp);
						}		
						PIN_ERRBUF_RESET(ebufp);
						PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
						return 0;
					}
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_serviceinfo provisioning output flist", provaction_outflistp);
					PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
				}
				else
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_serviceinfo there is no need of provisoning as Bouquet ID is same");
				}
			}
		}

		/*********************************** Credit limit setting********************************/

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Credit limit setting");
		credit_limit = PIN_FLIST_FLD_GET(services_flist, PIN_FLD_CREDIT_LIMIT, 1, ebufp );
		credit_floor = PIN_FLIST_FLD_GET(services_flist, PIN_FLD_CREDIT_FLOOR, 1, ebufp );
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "services_flist", services_flist);


		if (credit_limit && credit_floor && !pbo_decimal_is_null(credit_limit, ebufp) && !pbo_decimal_is_null(credit_floor, ebufp) )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "11111");
			fm_mso_read_bal_grp(ctxp, NULL, bal_grp_obj, &bal_grp_oflist, ebufp );
			if (bal_grp_oflist)
			{
				vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(bal_grp_oflist, PIN_FLD_BALANCES, 356, 1, ebufp);
				credit_profile_id = PIN_FLIST_FLD_GET(vp, PIN_FLD_CREDIT_PROFILE, 1, ebufp);
				if (credit_profile_id)
				{
					fm_mso_get_credit_profile(ctxp, *credit_profile_id, &credit_limit_array, ebufp);
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "credit_limit_array", credit_limit_array);
				if (credit_limit_array)
				{
					old_credit_limit_brm = PIN_FLIST_FLD_GET(credit_limit_array, PIN_FLD_CREDIT_LIMIT, 1, ebufp );
					old_credit_floor_brm = PIN_FLIST_FLD_GET(credit_limit_array, PIN_FLD_CREDIT_FLOOR, 1, ebufp );
				}
			}

			//new values
			if (pbo_decimal_compare(credit_limit, limit_max, ebufp) ==0)
			{
				new_credit_limit_brm = NULL;
			}
			else
			{
				new_credit_limit_brm = pbo_decimal_copy(credit_limit, ebufp);
			}
			
			if (pbo_decimal_compare(credit_floor, floor_min, ebufp) ==0)
			{
				new_credit_floor_brm = NULL;
			}
			else
			{
				new_credit_floor_brm = pbo_decimal_copy(credit_floor, ebufp);
			}

			//Old values
			if (!old_credit_limit_brm || pbo_decimal_is_null(old_credit_limit_brm, ebufp))
			{
				credit_limit_old = pbo_decimal_copy(limit_max, ebufp);
			}
			else
			{
				credit_limit_old = pbo_decimal_copy(old_credit_limit_brm, ebufp);
			}
  			if (!old_credit_floor_brm || pbo_decimal_is_null(old_credit_floor_brm, ebufp))
			{
				credit_floor_old = pbo_decimal_copy(floor_min, ebufp);
			}
			else
			{
				credit_floor_old = pbo_decimal_copy(old_credit_floor_brm, ebufp);
			}

			if ((pbo_decimal_compare(credit_limit, credit_limit_old, ebufp) !=0 ) ||
			    (pbo_decimal_compare(credit_floor, credit_floor_old, ebufp) !=0)
			   )
			{
				updacct_inflistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, updacct_inflistp, PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, updacct_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);
				PIN_FLIST_FLD_COPY(services_flist, PIN_FLD_BAL_GRP_OBJ, updacct_inflistp, PIN_FLD_BAL_GRP_OBJ, ebufp);

				inheritedinfo_flist = PIN_FLIST_ELEM_ADD(updacct_inflistp, PIN_FLD_LIMIT, 356, ebufp);
				//PIN_FLIST_FLD_COPY(services_flist, PIN_FLD_CREDIT_LIMIT, inheritedinfo_flist, PIN_FLD_CREDIT_LIMIT, ebufp);
				//PIN_FLIST_FLD_COPY(services_flist, PIN_FLD_CREDIT_FLOOR, inheritedinfo_flist, PIN_FLD_CREDIT_FLOOR, ebufp);
				PIN_FLIST_FLD_SET(inheritedinfo_flist, PIN_FLD_CREDIT_LIMIT, new_credit_limit_brm, ebufp);
				PIN_FLIST_FLD_SET(inheritedinfo_flist, PIN_FLD_CREDIT_FLOOR, new_credit_floor_brm, ebufp);

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_serviceinfo update credit limit input list", updacct_inflistp);
				PCM_OP(ctxp, PCM_OP_BILL_SET_LIMIT_AND_CR, 0, updacct_inflistp, &updacct_outflistp, ebufp);
				PIN_FLIST_DESTROY_EX(&updacct_inflistp, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_BILL_SET_LIMIT_AND_CR", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);
					PIN_FLIST_DESTROY_EX(&bal_grp_oflist, NULL);
					PIN_FLIST_DESTROY_EX(&credit_limit_array, NULL);
					return 0;
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_serviceinfo update credit limit output flist", updacct_outflistp);
				PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);

				if (credit_limit_array)
				{
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "credit_limit_array", credit_limit_array);
					vp1 = (pin_flist_t*)PIN_FLIST_ELEM_GET(out_flist, PIN_FLD_DATA_ARRAY, 0, 1, ebufp);
					vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(vp1, PIN_FLD_SERVICES, 0, 1, ebufp);
					if (!vp)
					{
						vp = PIN_FLIST_ELEM_ADD(vp1, PIN_FLD_SERVICES, 0, ebufp);
					}
					if (vp && credit_limit_old)
					{
						//PIN_FLIST_FLD_COPY(credit_limit_array, PIN_FLD_CREDIT_LIMIT, vp , PIN_FLD_CREDIT_LIMIT, ebufp);
						PIN_FLIST_FLD_SET(vp, PIN_FLD_CREDIT_LIMIT, old_credit_limit_brm,  ebufp);
					}
					if (vp && credit_floor_old)
					{	
						//PIN_FLIST_FLD_COPY(credit_limit_array, PIN_FLD_CREDIT_FLOOR, vp , PIN_FLD_CREDIT_FLOOR, ebufp);
						PIN_FLIST_FLD_SET(vp, PIN_FLD_CREDIT_FLOOR, old_credit_floor_brm,  ebufp);
					}
				}
				if (services_flist)
				{
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "services_flist", services_flist);
					vp1 = (pin_flist_t*)PIN_FLIST_ELEM_GET(out_flist, PIN_FLD_DATA_ARRAY, 1, 1, ebufp);
					vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(vp1, PIN_FLD_SERVICES, 0, 1, ebufp);
					if (!vp)
					{
						vp = PIN_FLIST_ELEM_ADD(vp1, PIN_FLD_SERVICES, 0, ebufp);
					}
					if (vp && credit_limit)
					{
						//PIN_FLIST_FLD_COPY(services_flist, PIN_FLD_CREDIT_LIMIT, vp , PIN_FLD_CREDIT_LIMIT, ebufp);
						PIN_FLIST_FLD_SET(vp, PIN_FLD_CREDIT_LIMIT, new_credit_limit_brm,  ebufp);
					}
					if (vp && credit_floor)
					{
						//PIN_FLIST_FLD_COPY(services_flist, PIN_FLD_CREDIT_FLOOR, vp , PIN_FLD_CREDIT_FLOOR, ebufp);
						PIN_FLIST_FLD_SET(vp, PIN_FLD_CREDIT_FLOOR, new_credit_floor_brm,  ebufp);
					}
				}
			}
			PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
		}
		if (limit_max)
		{
			pbo_decimal_destroy(&limit_max);
		}
		if (floor_min)
		{
			pbo_decimal_destroy(&floor_min);
		}
		if (new_credit_limit_brm)
		{
			pbo_decimal_destroy(&new_credit_limit_brm);
		}
		if (new_credit_floor_brm)
		{
			pbo_decimal_destroy(&new_credit_floor_brm);
		}
		if (credit_limit_old)
		{
			pbo_decimal_destroy(&credit_limit_old);
		}	
		if (credit_floor_old)
		{
			pbo_decimal_destroy(&credit_floor_old);
		}
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "out_flist", out_flist);

	PIN_FLIST_DESTROY_EX(&bal_grp_oflist, NULL);
	PIN_FLIST_DESTROY_EX(&credit_limit_array, NULL);
	return 2;
}


/**************************************************
* Function: fm_mso_update_payinfo()
* 
* 
***************************************************/
static int 
fm_mso_update_payinfo(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		*out_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*updacct_inflistp = NULL;
	pin_flist_t		*updacct_outflistp = NULL;
	pin_flist_t		*payinfo_flistp = NULL;
	pin_flist_t		*invinfo_flist = NULL;
	pin_flist_t		*inheritedinfo_flist = NULL;
	pin_flist_t		*updpayinfo_flistp = NULL;
	pin_flist_t		*srch_iflist = NULL;
	pin_flist_t		*srch_oflist = NULL;
	pin_flist_t		*args_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*members = NULL;
	pin_flist_t		*update_child_payinfo_iflist = NULL;
	pin_flist_t		*update_child_payinfo_oflist = NULL;
	pin_flist_t		*child_payinfo = NULL;
	pin_flist_t		*child_inherited_info = NULL;
	pin_flist_t		*child_inv_info = NULL;
	pin_flist_t		*srch_payinfo_iflist = NULL;
	pin_flist_t		*srch_payinfo_oflist = NULL;

	pin_flist_t		*readpayinfo_inflistp = NULL;
	pin_flist_t		*readpayinfo_outflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*args_flistp2 = NULL;
	pin_flist_t		*results_flistp =NULL;

	pin_flist_t		*pi_search_flistp = NULL;
	pin_flist_t		*pi_search_res_flistp = NULL;
	pin_flist_t		*modif_flistp = NULL;
	pin_flist_t		*modif_out_flistp = NULL;
	pin_flist_t		*inher_flistp = NULL;

	poid_t			*service_obj = NULL;
	poid_t			*payinfo_obj = NULL;
	poid_t			*account_obj = NULL;
	poid_t			*srch_pdp = NULL;

	int32			*indicator = NULL;
	int32			status_closed= PIN_STATUS_CLOSED;
	int32			status_cancelled = 3;
	int32			srch_flags = 512;
	int32			flags = 256;
	int32			payment_term = 0;
	int32			*subscriber_type = NULL;
	int32			elemid=0;
	int32			corporate_type = 0;
	pin_decimal_t		*priority  = pbo_decimal_from_str("0.0",ebufp);

	char			*poid_type = NULL;
	char                    search_template[100] = " select X from /payinfo where F1 = V1 and F2 = V2 ";
	char			*srch_template = "select X from /payinfo where F1 = V1 and payinfo_t.poid_type = '/payinfo/invoice' ";


	int64			db = -1;

	pin_cookie_t		cookie = NULL;

	void			*vp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_payinfo :");

	account_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp);
	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 0, ebufp));
	
	payinfo_flistp = PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_PAYINFO, PIN_ELEMID_ANY, 1, ebufp );

	if (!payinfo_flistp)
	{
		return 1;
	}
	else 
	{
		invinfo_flist = PIN_FLIST_ELEM_GET(payinfo_flistp, PIN_FLD_INV_INFO, 0, 1, ebufp );
		payinfo_obj = PIN_FLIST_FLD_GET(payinfo_flistp, PIN_FLD_PAYINFO_OBJ, 1, ebufp);
		poid_type = (char*)PIN_POID_GET_TYPE(payinfo_obj);
		if (!invinfo_flist && (strcmp(poid_type, MSO_CC_TYPE) != 0 ) && (strcmp(poid_type, MSO_DD_TYPE) != 0 ))
		{
			return 1;
		}
	}
/****************************************CC or DD Pay Type *********************************/
	if ( ( strcmp(poid_type, MSO_CC_TYPE) == 0 ) || ( strcmp(poid_type, MSO_DD_TYPE) == 0 ) )
	{
		if ( PIN_POID_GET_ID(payinfo_obj) == -1 )
		{
			fm_mso_cust_set_payinfo(ctxp, account_obj, payinfo_flistp, ebufp );
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in setting CC/DD payinfo", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				return 0;
			}
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_payinfo : Payinfo info updated");	
			return 2;
		}
		else if ( PIN_POID_GET_ID(payinfo_obj) > 0 )
		{
			srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
			pi_search_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(pi_search_flistp, PIN_FLD_POID, srch_pdp, ebufp);
			PIN_FLIST_FLD_SET(pi_search_flistp, PIN_FLD_FLAGS, &flags, ebufp);
			PIN_FLIST_FLD_SET(pi_search_flistp, PIN_FLD_TEMPLATE, search_template, ebufp);

			args_flistp = PIN_FLIST_ELEM_ADD(pi_search_flistp, PIN_FLD_ARGS, 1, ebufp);
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, payinfo_obj, ebufp);
			args_flistp = PIN_FLIST_ELEM_ADD(pi_search_flistp, PIN_FLD_ARGS, 2, ebufp);
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
			result_flist = PIN_FLIST_ELEM_ADD(pi_search_flistp, PIN_FLD_RESULTS, 0, ebufp);
			PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_payinfo search payinfo input list", pi_search_flistp);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, pi_search_flistp, &pi_search_res_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&pi_search_flistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in searching CC/DD payinfo", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&pi_search_res_flistp, NULL);
				return 0;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_payinfo search payinfo output list", pi_search_res_flistp);

			result_flist = PIN_FLIST_ELEM_GET(pi_search_res_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
			if ( result_flist == (pin_flist_t *) NULL )
			{
				PIN_FLIST_DESTROY_EX(&pi_search_res_flistp, NULL);
				return 1;
			}
			modif_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(modif_flistp, PIN_FLD_POID, payinfo_obj, ebufp);
			PIN_FLIST_FLD_SET(modif_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
			inher_flistp = PIN_FLIST_SUBSTR_GET(modif_flistp, PIN_FLD_INHERITED_INFO, 1, ebufp);
			PIN_FLIST_SUBSTR_SET(modif_flistp, inher_flistp, PIN_FLD_INHERITED_INFO, ebufp);
/*			if (  strcmp(poid_type, MSO_CC_TYPE) == 0 )	
				args_flistp = PIN_FLIST_ELEM_ADD(inher_flistp, PIN_FLD_CC_INFO, 0, ebufp);
			if (  strcmp(poid_type, MSO_DD_TYPE) == 0 )
				args_flistp = PIN_FLIST_ELEM_ADD(inher_flistp, PIN_FLD_DD_INFO, 0, ebufp);   */
		        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_payinfo input list", modif_flistp);
        		PCM_OP(ctxp, PCM_OP_CUST_MODIFY_PAYINFO, 0, modif_flistp, &modif_out_flistp, ebufp);
        		PIN_FLIST_DESTROY_EX(&modif_flistp, NULL);
        		if (PIN_ERRBUF_IS_ERR(ebufp))
        		{
                		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_CUST_MODIFY_PAYINFO", ebufp);
                		PIN_ERRBUF_RESET(ebufp);
                		PIN_FLIST_DESTROY_EX(&modif_out_flistp, NULL);
                		return 0;
        		}
        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_payinfo output flist", modif_out_flistp);
        		PIN_FLIST_DESTROY_EX(&modif_out_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&pi_search_res_flistp, NULL);
			return 2;
		}
	}
/****************************************CC or DD Pay Type *********************************/
	subscriber_type =  (int32*)PIN_FLIST_FLD_GET(in_flist, MSO_FLD_SUBSCRIBER_TYPE, 1, ebufp);
	vp =  (int32*)PIN_FLIST_FLD_GET(in_flist, MSO_FLD_CORPORATE_TYPE, 1, ebufp);
	if (vp)
	{
		corporate_type = *(int32*)vp;
	}

	/************************************** Reading profile object for LCO info ***************************************/

	payinfo_obj = PIN_FLIST_FLD_GET(payinfo_flistp, PIN_FLD_PAYINFO_OBJ, 1, ebufp);
	if (payinfo_obj)
	{
         	poid_type = (char*)PIN_POID_GET_TYPE(payinfo_obj);	

	        if (poid_type && strcmp(poid_type, "/account") == 0)
	        {
		      PIN_FLIST_FLD_SET(in_flist, PIN_FLD_ERROR_DESCR, "Payment information change for non-paying child is allowed from parent's profile only", ebufp );
		      return 0;
	        }

	        readpayinfo_inflistp = PIN_FLIST_CREATE(ebufp);
	        PIN_FLIST_FLD_COPY(payinfo_flistp, PIN_FLD_PAYINFO_OBJ, readpayinfo_inflistp, PIN_FLD_POID, ebufp);

	        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_payinfo read payinfo input list", readpayinfo_inflistp);
	        PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, readpayinfo_inflistp, &readpayinfo_outflistp, ebufp);
	}
	else
	{
		srch_payinfo_iflist = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(srch_payinfo_iflist, PIN_FLD_POID, (PIN_POID_CREATE( 1, "/search", -1, ebufp)), ebufp);
		PIN_FLIST_FLD_SET(srch_payinfo_iflist, PIN_FLD_FLAGS, &srch_flags, ebufp);
		PIN_FLIST_FLD_SET(srch_payinfo_iflist, PIN_FLD_TEMPLATE, srch_template , ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(srch_payinfo_iflist, PIN_FLD_ARGS, 1, ebufp );
		//PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acct_obj, ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_ACCOUNT_OBJ,args_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);

		results_flistp = PIN_FLIST_ELEM_ADD(srch_payinfo_iflist, PIN_FLD_RESULTS, 0, ebufp );
		//PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID, NULL, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search profile input flist", srch_payinfo_iflist);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_payinfo_iflist, &srch_payinfo_oflist, ebufp);
		PIN_FLIST_DESTROY_EX(&srch_payinfo_iflist, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in searching profile", ebufp);
			PIN_FLIST_DESTROY_EX(&srch_payinfo_oflist, NULL);
			return 0;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search profile output flist", srch_payinfo_oflist);

		readpayinfo_outflistp = PIN_FLIST_ELEM_GET(srch_payinfo_oflist, PIN_FLD_RESULTS, 0, 1, ebufp);
		PIN_FLIST_FLD_COPY(readpayinfo_outflistp, PIN_FLD_POID, payinfo_flistp, PIN_FLD_PAYINFO_OBJ, ebufp );
	}

        PIN_FLIST_DESTROY_EX(&readpayinfo_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_OBJ payinfo", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&readpayinfo_outflistp, NULL);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_payinfo read payinfo output flist", readpayinfo_outflistp);

	updpayinfo_flistp = PIN_FLIST_ELEM_GET(readpayinfo_outflistp, PIN_FLD_INV_INFO, 0, 1, ebufp );

	/*************************************** adding base value to output flist ****************************************/

	args_flistp = PIN_FLIST_ELEM_GET(out_flist, PIN_FLD_DATA_ARRAY, 0, 1, ebufp );
	inheritedinfo_flist = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_PAYINFO, 0, ebufp );
	PIN_FLIST_FLD_COPY(payinfo_flistp, PIN_FLD_PAYINFO_OBJ, inheritedinfo_flist, PIN_FLD_PAYINFO_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(payinfo_flistp, PIN_FLD_PAY_TYPE, inheritedinfo_flist, PIN_FLD_PAY_TYPE, ebufp);
	PIN_FLIST_FLD_COPY(payinfo_flistp, PIN_FLD_PAYMENT_TERM, inheritedinfo_flist, PIN_FLD_PAYMENT_TERM, ebufp);	

	args_flistp2 = PIN_FLIST_ELEM_ADD(inheritedinfo_flist, PIN_FLD_INV_INFO, 0, ebufp );
	PIN_FLIST_FLD_COPY(updpayinfo_flistp, PIN_FLD_DELIVERY_PREFER, args_flistp2, PIN_FLD_DELIVERY_PREFER, ebufp);	
 	PIN_FLIST_FLD_COPY(updpayinfo_flistp, PIN_FLD_INDICATOR, args_flistp2, PIN_FLD_INDICATOR, ebufp);	
	PIN_FLIST_FLD_COPY(updpayinfo_flistp, MSO_FLD_TDS_RECV, args_flistp2, MSO_FLD_TDS_RECV, ebufp);	

	args_flistp = PIN_FLIST_ELEM_GET(out_flist, PIN_FLD_DATA_ARRAY, 1, 1, ebufp );
	PIN_FLIST_ELEM_SET(args_flistp, payinfo_flistp, PIN_FLD_PAYINFO, 0, ebufp );

	payment_term = *(int32*)PIN_FLIST_FLD_GET(readpayinfo_outflistp, PIN_FLD_PAYMENT_TERM, 0, ebufp);
	
	PIN_FLIST_DESTROY_EX(&readpayinfo_outflistp, NULL);

	/*************************************** updating payinfo ***********************************************************/
	
	updacct_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, updacct_inflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, updacct_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);

	
	updpayinfo_flistp = PIN_FLIST_ELEM_ADD(updacct_inflistp, PIN_FLD_PAYINFO, 0, ebufp);
	PIN_FLIST_FLD_SET( updpayinfo_flistp, PIN_FLD_PAYMENT_TERM, &payment_term, ebufp);
	PIN_FLIST_FLD_COPY(payinfo_flistp, PIN_FLD_PAYINFO_OBJ, updpayinfo_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(payinfo_flistp, PIN_FLD_PAY_TYPE, updpayinfo_flistp, PIN_FLD_PAY_TYPE, ebufp);
	
	inheritedinfo_flist = PIN_FLIST_SUBSTR_ADD(updpayinfo_flistp, PIN_FLD_INHERITED_INFO, ebufp);
	PIN_FLIST_ELEM_SET(inheritedinfo_flist, invinfo_flist, PIN_FLD_INV_INFO, 0, ebufp);

	indicator = (int32*)PIN_FLIST_FLD_GET(invinfo_flist, PIN_FLD_INDICATOR, 1, ebufp );
	if (indicator )
	{
		srch_iflist = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(srch_iflist, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(srch_iflist, PIN_FLD_FLAGS, &srch_flags, ebufp);
		PIN_FLIST_FLD_SET(srch_iflist, PIN_FLD_TEMPLATE, "select X from /purchased_product 1, /product 2  where  1.F1 = V1 and 1.F2 != V2 and 1.F3 = 2.F4 and 2.F5  not in (0,10,20,30,40,170) and 1.F6.type like V6 ", ebufp);

		args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_ACCOUNT_OBJ, args_flist, PIN_FLD_ACCOUNT_OBJ, ebufp);

		args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(args_flist, PIN_FLD_STATUS, &status_cancelled, ebufp);
		
		args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(args_flist, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
		args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 4, ebufp);
		PIN_FLIST_FLD_SET(args_flist, PIN_FLD_POID, NULL, ebufp);
		args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 5, ebufp);
		PIN_FLIST_FLD_PUT(args_flist, PIN_FLD_PRIORITY, priority, ebufp);
		service_obj = PIN_POID_CREATE(db, "/service/catv", 0, ebufp);
		args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 6, ebufp);
		PIN_FLIST_FLD_PUT(args_flist, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);		

		result_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_purchased_product srch_iflist", srch_iflist);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflist, &srch_oflist, ebufp);
		PIN_FLIST_DESTROY_EX(&srch_iflist, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_FLIST_FLD_SET(in_flist, PIN_FLD_ERROR_DESCR, "Error in calling Search", ebufp );
			return 0;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_purchased_product srch_oflist:", srch_oflist);
		result_flist = PIN_FLIST_ELEM_GET(srch_oflist, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);

		if (result_flist)
		{
			PIN_FLIST_FLD_SET(in_flist, PIN_FLD_ERROR_DESCR, "Except FDP and Hardware Plans, cancel all plans before changing pay-type", ebufp );
			PIN_FLIST_DESTROY_EX(&srch_oflist, NULL);
			return 0;
		}
		PIN_FLIST_DESTROY_EX(&srch_oflist, NULL);
	}
	//if (corporate_type == CORPORATE_PARENT  )
	if (corporate_type == CORPORATE_PARENT && (indicator) )
	{
		srch_iflist = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(srch_iflist, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(srch_iflist, PIN_FLD_FLAGS, &srch_flags, ebufp);
		PIN_FLIST_FLD_SET(srch_iflist, PIN_FLD_TEMPLATE, "select X from /service 1, /group/billing 2 where 1.F1 = 2.F2 and 2.F3 = V3 and 1.F4 != V4 ", ebufp);

		args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(args_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);

		args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 2, ebufp);
		members =  PIN_FLIST_ELEM_ADD(args_flist, PIN_FLD_MEMBERS, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(members, PIN_FLD_OBJECT, NULL, ebufp);

		args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_ACCOUNT_OBJ, args_flist, PIN_FLD_ACCOUNT_OBJ, ebufp);

		args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 4, ebufp);
		PIN_FLIST_FLD_SET(args_flist, PIN_FLD_STATUS, &status_closed, ebufp);

		result_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STATUS, NULL, ebufp );

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_payinfo srch_iflist", srch_iflist);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflist, &srch_oflist, ebufp);
		PIN_FLIST_DESTROY_EX(&srch_iflist, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_FLIST_FLD_SET(in_flist, PIN_FLD_ERROR_DESCR, "Error in calling Search", ebufp );
			return 0;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_payinfo srch_oflist:", srch_oflist);
		result_flist = PIN_FLIST_ELEM_GET(srch_oflist, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);

		if (result_flist)
		{
			PIN_FLIST_FLD_SET(in_flist, PIN_FLD_ERROR_DESCR, "Cancel services for all child accounts before changing pay-type", ebufp );
			PIN_FLIST_DESTROY_EX(&srch_oflist, NULL);
			return 0;
		}
		else
		{
			PIN_FLIST_DESTROY_EX(&srch_oflist, NULL);
			//Modify pay_type for all cancelled child accounts
			srch_iflist = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(srch_iflist, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(srch_iflist, PIN_FLD_FLAGS, &srch_flags, ebufp);
			PIN_FLIST_FLD_SET(srch_iflist, PIN_FLD_TEMPLATE, "select X from /payinfo/invoice 1, /group/billing 2 where 1.F1 = 2.F2 and 2.F3 = V3  ", ebufp);

			args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 1, ebufp);
			PIN_FLIST_FLD_SET(args_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);

			args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 2, ebufp);
			members =  PIN_FLIST_ELEM_ADD(args_flist, PIN_FLD_MEMBERS, PIN_ELEMID_ANY, ebufp);
			PIN_FLIST_FLD_SET(members, PIN_FLD_OBJECT, NULL, ebufp);

			args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 3, ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_ACCOUNT_OBJ, args_flist, PIN_FLD_ACCOUNT_OBJ, ebufp);

			result_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_payinfo srch_iflist", srch_iflist);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflist, &srch_oflist, ebufp);
			PIN_FLIST_DESTROY_EX(&srch_iflist, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_FLIST_FLD_SET(in_flist, PIN_FLD_ERROR_DESCR, "Error in fetch- modify paytype for child accounts", ebufp );
				return 0;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_payinfo srch_oflist:", srch_oflist);
			while ((result_flist = PIN_FLIST_ELEM_GET_NEXT(srch_oflist, PIN_FLD_RESULTS, &elemid, 1, &cookie, ebufp )) != NULL)
			{
				update_child_payinfo_iflist = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_ACCOUNT_OBJ,  update_child_payinfo_iflist, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_COPY(in_flist,     PIN_FLD_PROGRAM_NAME, update_child_payinfo_iflist, PIN_FLD_PROGRAM_NAME, ebufp );
				PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_ACCOUNT_OBJ,  update_child_payinfo_iflist, PIN_FLD_POID, ebufp );

				child_payinfo = PIN_FLIST_ELEM_ADD(update_child_payinfo_iflist, PIN_FLD_PAYINFO, 0, ebufp);
				PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_PAYMENT_TERM, child_payinfo, PIN_FLD_PAYMENT_TERM, ebufp );
				PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_POID,         child_payinfo, PIN_FLD_POID, ebufp );
 
 				child_inherited_info = PIN_FLIST_SUBSTR_ADD(child_payinfo, PIN_FLD_INHERITED_INFO, ebufp);
				child_inv_info = PIN_FLIST_ELEM_ADD(child_inherited_info, PIN_FLD_INV_INFO, 0, ebufp);	
				PIN_FLIST_FLD_SET(child_inv_info, PIN_FLD_INDICATOR, indicator, ebufp); 

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "update_child_payinfo_iflist ", update_child_payinfo_iflist);
				PCM_OP(ctxp, PCM_OP_CUST_SET_PAYINFO, 0, update_child_payinfo_iflist, &update_child_payinfo_oflist, ebufp);
				PIN_FLIST_DESTROY_EX(&update_child_payinfo_iflist, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_CUST_SET_PAYINFO for child ", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					PIN_FLIST_DESTROY_EX(&update_child_payinfo_oflist, NULL);
					PIN_FLIST_DESTROY_EX(&srch_oflist, NULL);
					return 0;
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "update_child_payinfo_oflist", update_child_payinfo_oflist);
				PIN_FLIST_DESTROY_EX(&update_child_payinfo_oflist, NULL); 
			}
			PIN_FLIST_DESTROY_EX(&srch_oflist, NULL);
		}
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_payinfo input list", updacct_inflistp);
	PCM_OP(ctxp, PCM_OP_CUST_SET_PAYINFO, 0, updacct_inflistp, &updacct_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&updacct_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_CUST_SET_PAYINFO", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);
		PIN_FLIST_FLD_SET(in_flist, PIN_FLD_ERROR_DESCR, "Error in modify payinfo for account", ebufp );
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_payinfo output flist", updacct_outflistp);
	PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);
	return 2;
}


/**************************************************
* Function: fm_mso_update_lcoinfo()
* updates customer care profile
* updates bill info if impacted
* updates customer associated devices if impacted 
***************************************************/
static int 
fm_mso_update_lcoinfo(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		*out_flist,
	pin_flist_t		**comm_err_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*updacct_inflistp = NULL;
	pin_flist_t		*updacct_outflistp = NULL;
	pin_flist_t		*customecare_flistp = NULL;
	pin_flist_t		*inheritedinfo_flist = NULL;
	pin_flist_t		*updcustomecare_flistp = NULL;
	pin_flist_t		*wholesale_flistp = NULL;
	pin_flist_t		*profiles_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*readprofile_inflistp = NULL;
	pin_flist_t		*readprofile_outflistp = NULL;

	pin_flist_t		*findprofile_inflistp = NULL;
	pin_flist_t		*findprofile_outflistp = NULL;
	pin_flist_t		*results_flistp = NULL;
	pin_flist_t		*updbillinfo_inflistp = NULL;
	pin_flist_t		*updbillinfo_outflistp = NULL;
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t             *customercare_flistp=NULL;
        pin_flist_t             *customercareupd_flistp=NULL;
	pin_flist_t		*acctinfo=NULL;
	pin_flist_t		*code_info=NULL;
	pin_flist_t		*r_flistp=NULL;
	pin_flist_t		*acnt_info=NULL;
	pin_flist_t		*nameinfo=NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*billinfo = NULL;

	pin_flist_t             *lco_event_flistp=NULL;

	poid_t			*lco_obj = NULL;
	poid_t			*old_lco_obj = NULL;
    poid_t          *cust_companyp = NULL;
    poid_t          *new_lco_companyp = NULL;
	poid_t                  *sdt_obj=NULL;
	poid_t                  *dt_obj=NULL;
	poid_t                  *jv_obj=NULL;
	poid_t			*parent_obj=NULL;	
	poid_t                  *profile_obj =NULL;
	poid_t			*srch_pdp = NULL;
	poid_t			*profile_pdp = NULL;
	poid_t			*prof_pdp = NULL;
	poid_t			*acct_pdp = NULL;
	poid_t			*billinfo_pdp = NULL;

	int			actg_dom = 0;
	int			ret_val = 0;
	int32			db=0;
	int32			flags = 768;
	int32			*flg = NULL;
	int32			*billing_segment_ptr = NULL;
	int32			*lco_bill_segment_ptr = NULL;
	int32			*lco_pp_type_ptr = NULL;
	int32			billing_segment = 0;
	int32			lco_pp_type = -1;
	int32			lco_bill_segment = -1;
	int32			pp_bdom = 1;
	int32			pp_bill_segment = 5000;
	int32			pp_no_bill_segment = 104;
	int32			sp_no_bill_segment = 104;
	int32			business_type = 0;
	int32			arr_business_type[8];
	int32			den_hw_flag = 0;
	int32			account_type = -1;
	int                     *pp_type = NULL;
		int32 			serv_type = 1;
	char			*template = " select X from /profile where F1 = V1 and (profile_t.poid_type = '/profile/customer_care' or profile_t.poid_type = '/profile/wholesale') ";
	char			*state_city_area_code = NULL;
	char			*state_code = NULL;
	void			*vp = NULL;
	char			customer_city[100] = "";
	char			lco_city[100] = "";

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lcoinfo: input flist", in_flist);	
	customecare_flistp = PIN_FLIST_SUBSTR_GET(in_flist, PIN_FLD_CUSTOMER_CARE_INFO, 1, ebufp );

	if ((PIN_ERRBUF_IS_ERR(ebufp)) || (!customecare_flistp))
	{
		PIN_ERRBUF_RESET(ebufp);
		return 1;
	}
	
	acct_pdp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	prof_pdp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_PROFILE_OBJ, 1, ebufp);

	lco_obj = PIN_FLIST_FLD_GET(customecare_flistp, MSO_FLD_LCO_OBJ, 1, ebufp );
	if (!lco_obj)
	{
		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lcoinfo: NO LCO objct passed in input flist");
		return 1;
	}	
 
	/************************************** ***************************************
	 *	Match LCO city with Customer City 
	 ******************************************************************************/
	fm_mso_get_account_info(ctxp , acct_pdp, &acnt_info, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_update_lcoinfo: Error in calling fm_mso_get_account_info", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
		return 0;
	}
	business_type = *(int32*)PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_BUSINESS_TYPE, 0, ebufp);
	num_to_array(business_type, arr_business_type);
	account_type    = 10*(arr_business_type[0])+arr_business_type[1];// First 2 digits
	if (account_type < 90)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lcoinfo: This is not a subscriber account");
		return 6;
	}
	if (arr_business_type[7] == 2 || arr_business_type[7] == 3)
	{
		den_hw_flag = 1;
	}
    else if (arr_business_type[7] == 4 || arr_business_type[7] == 5)
	{
		den_hw_flag = 2;
	}

	nameinfo = PIN_FLIST_ELEM_GET(acnt_info, PIN_FLD_NAMEINFO, 2, 1, ebufp);

	if (nameinfo)
	{
		vp= (char*)PIN_FLIST_FLD_GET(nameinfo, PIN_FLD_CITY, 1, ebufp);
		if (vp)
		{
			strcpy(customer_city, vp);
		}
	}
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL);

	fm_mso_get_account_info(ctxp , lco_obj, &acnt_info, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_update_lcoinfo: Error in calling fm_mso_get_account_info", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
		return 0;
	}

	business_type = *(int32*)PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_BUSINESS_TYPE, 0, ebufp);
	num_to_array(business_type, arr_business_type);
	account_type    = 10*(arr_business_type[0])+arr_business_type[1];// First 2 digits
	if (account_type != ACCT_TYPE_LCO)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lcoinfo: This is not a LCO account");
		return 7;
	}

	nameinfo = PIN_FLIST_ELEM_GET(acnt_info, PIN_FLD_NAMEINFO, 1, 1, ebufp);
	if (nameinfo)
	{
		vp= (char*)PIN_FLIST_FLD_GET(nameinfo, PIN_FLD_CITY, 1, ebufp);
		if (vp)
		{
			strcpy(lco_city, vp);
		}
	}
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
	
        if (strcmp(customer_city,lco_city) !=0 )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lcoinfo: Customer city not matching with LCO city");
		return 4;// Customer city not matching with LCO city
	}

 	/************************************** ***************************************
	 *	Fetch existing LCO object
	 ******************************************************************************/

	db = PIN_POID_GET_DB(lco_obj);
	srch_pdp = PIN_POID_CREATE(db, "/search", (int64)-1, ebufp);
	srch_flistp = PIN_FLIST_CREATE(ebufp);

	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template, ebufp); //select X from /profile where F1 = V1 
	args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ,acct_pdp, ebufp);
	results_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS,0,ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lcoinfo: search  customer profile input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH,0,srch_flistp,&srch_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_update_lcoinfo: Error in calling PCM_OP_SEARCH profile", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
		return 0;
	}	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lcoinfo: search customer profile output flist", srch_oflistp);
	
	results_flistp = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
	
	if(results_flistp)
	{
		profile_pdp = PIN_FLIST_FLD_GET(results_flistp,PIN_FLD_POID, 0, ebufp); 

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lcoinfo: read profile output flist", results_flistp);

		args_flistp = PIN_FLIST_SUBSTR_GET(results_flistp, PIN_FLD_CUSTOMER_CARE_INFO, 0, ebufp );
		vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(out_flist, PIN_FLD_DATA_ARRAY, 0, 1, ebufp);
		PIN_FLIST_SUBSTR_SET(vp, args_flistp, PIN_FLD_CUSTOMER_CARE_INFO, ebufp);

		old_lco_obj = PIN_FLIST_FLD_GET(args_flistp, MSO_FLD_LCO_OBJ, 0, ebufp );
        cust_companyp = PIN_FLIST_FLD_GET(args_flistp, MSO_FLD_COMPANY_OBJ, 1, ebufp);
		if ((PIN_ERRBUF_IS_ERR(ebufp)) || (!old_lco_obj))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_update_lcoinfo:  Error in getting LCO object from profile", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
			return 0;
		}

	}
	else
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_update_lcoinfo NO customer care profile found", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
		return 0;	
	}

 	/************************************** ***************************************
	 *	Compare existing & NEW LCO object
	 ******************************************************************************/
	if (PIN_POID_COMPARE(old_lco_obj, lco_obj, 0, ebufp) == 0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lcoinfo : Inside update profile other than LCO");

		res_flistp = PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_RESULTS, 0, 1, ebufp );
		if (!res_flistp)
		{
			PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
			return 5;
		}
		else
		{
			flg = (int32*)PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_FLAGS, 1, ebufp);
			if (flg && *flg == 1)
			{
				PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
				return 5;
			}
		}

 		// Get inputs from input customercare profile values
		lco_obj = PIN_FLIST_FLD_GET(customecare_flistp, MSO_FLD_LCO_OBJ, 1, ebufp );
		dt_obj = PIN_FLIST_FLD_GET(customecare_flistp, MSO_FLD_DT_OBJ, 1, ebufp );
		jv_obj = PIN_FLIST_FLD_GET(customecare_flistp, MSO_FLD_JV_OBJ, 1, ebufp );
		sdt_obj = PIN_FLIST_FLD_GET(customecare_flistp, MSO_FLD_SDT_OBJ, 1, ebufp );
		parent_obj = PIN_FLIST_FLD_GET(customecare_flistp, PIN_FLD_PARENT , 1, ebufp );
		pp_type = PIN_FLIST_FLD_GET(customecare_flistp,MSO_FLD_PP_TYPE,1, ebufp );
		
		// Prepare input flist for customercare profile update
		updacct_inflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(updacct_inflistp, PIN_FLD_POID,prof_pdp, ebufp);
		profiles_flistp = PIN_FLIST_ELEM_ADD(updacct_inflistp, PIN_FLD_PROFILES, 0, ebufp);
		if(prof_pdp)
		{
			PIN_FLIST_FLD_SET(profiles_flistp, PIN_FLD_PROFILE_OBJ,prof_pdp, ebufp);
		}
		else
		{
			PIN_FLIST_FLD_SET(profiles_flistp, PIN_FLD_PROFILE_OBJ,profile_pdp, ebufp);
		}
		inheritedinfo_flist = PIN_FLIST_SUBSTR_ADD(profiles_flistp, PIN_FLD_INHERITED_INFO, ebufp);
		customercareupd_flistp = PIN_FLIST_SUBSTR_ADD(inheritedinfo_flist, PIN_FLD_CUSTOMER_CARE_INFO, ebufp);

		PIN_FLIST_FLD_SET(customercareupd_flistp, MSO_FLD_LCO_OBJ, NULL, ebufp);
		PIN_FLIST_FLD_SET(customercareupd_flistp, MSO_FLD_SDT_OBJ, NULL, ebufp);
		PIN_FLIST_FLD_SET(customercareupd_flistp, MSO_FLD_DT_OBJ, NULL, ebufp);
		PIN_FLIST_FLD_SET(customercareupd_flistp, MSO_FLD_JV_OBJ, NULL, ebufp);

		if (lco_obj)// if present
		{
			PIN_FLIST_FLD_SET(customercareupd_flistp, MSO_FLD_LCO_OBJ, lco_obj, ebufp);
		}
		
		if(sdt_obj)// if present
		{
			PIN_FLIST_FLD_SET(customercareupd_flistp, MSO_FLD_SDT_OBJ, sdt_obj, ebufp);
		}
		
		if(dt_obj)// if present
		{
			PIN_FLIST_FLD_SET(customercareupd_flistp, MSO_FLD_DT_OBJ, dt_obj, ebufp);
		}
		if(jv_obj)// if present
		{
			PIN_FLIST_FLD_SET(customercareupd_flistp, MSO_FLD_JV_OBJ, jv_obj, ebufp);
		}
		if(parent_obj)// if present
		{
			PIN_FLIST_FLD_SET(customercareupd_flistp, PIN_FLD_PARENT, parent_obj, ebufp);
		}

		if(pp_type)// if present
		{
			PIN_FLIST_FLD_SET(customercareupd_flistp, MSO_FLD_PP_TYPE, pp_type, ebufp);
		}

		vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(out_flist, PIN_FLD_DATA_ARRAY, 1,1, ebufp);
		PIN_FLIST_SUBSTR_SET(vp, customercareupd_flistp, PIN_FLD_CUSTOMER_CARE_INFO, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lcoinfo:  profile update input flist", updacct_inflistp);
	 
		PCM_OP(ctxp, PCM_OP_CUST_MODIFY_PROFILE , 0, updacct_inflistp, &updacct_outflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&updacct_inflistp, NULL);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lcoinfo :  LCO profile update output flist", updacct_outflistp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_update_lcoinfo: Error in calling PCM_OP_CUST_MODIFY_PROFILE for end users", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);
			PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
			return 0;
		} 

		PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);	
		PIN_FLIST_DESTROY_EX(&readprofile_outflistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);

	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lcoinfo : Inside update profile with LCO");

		/************************************** ***************************************
		 *	Search LCO wholesale profile
		 ******************************************************************************/

		findprofile_inflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(findprofile_inflistp, PIN_FLD_POID, lco_obj, ebufp);
		PIN_FLIST_FLD_SET(findprofile_inflistp, PIN_FLD_ACCOUNT_OBJ, lco_obj, ebufp);
		PIN_FLIST_FLD_SET(findprofile_inflistp, PIN_FLD_TYPE_STR, "/profile/wholesale", ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lcoinfo :  find profile input list", findprofile_inflistp);
		PCM_OP(ctxp, PCM_OP_CUST_FIND_PROFILE , PCM_OPFLG_READ_RESULT, findprofile_inflistp, &findprofile_outflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&findprofile_inflistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_update_lcoinfo : Error in calling PCM_OP_CUST_FIND_PROFILE for LCO", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&findprofile_outflistp, NULL);
			return 0;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lcoinfo: output flist", findprofile_outflistp);

		results_flistp = PIN_FLIST_ELEM_GET(findprofile_outflistp, PIN_FLD_RESULTS, 0, 1, ebufp );

		if (!results_flistp)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lcoinfo: No LCO profile found!!");
			PIN_FLIST_DESTROY_EX(&findprofile_outflistp, NULL);
			return 3;
		}

		wholesale_flistp = PIN_FLIST_SUBSTR_GET(results_flistp, MSO_FLD_WHOLESALE_INFO, 0 , ebufp );	
        
        new_lco_companyp = PIN_FLIST_FLD_GET(wholesale_flistp, MSO_FLD_COMPANY_OBJ, 1, ebufp);
        /************************************** ***************************************
         *  Compare existing SP account & NEW LCO SP account poids
         ******************************************************************************/
        PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "new_lco_company_poid", new_lco_companyp);
        PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "new_cust_company_poid", cust_companyp);
        if (cust_companyp && new_lco_companyp &&  (PIN_POID_COMPARE(cust_companyp, new_lco_companyp, 0, ebufp) != 0))
        {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lcoinfo: LCO of different service provider!!");
            PIN_FLIST_DESTROY_EX(&findprofile_outflistp, NULL);
            return 8;
        }

		if(prof_pdp)
		{
			updacct_inflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(updacct_inflistp, PIN_FLD_POID, prof_pdp, ebufp);
		
			profiles_flistp = PIN_FLIST_ELEM_ADD(updacct_inflistp, PIN_FLD_PROFILES, 0, ebufp);
			PIN_FLIST_FLD_SET(profiles_flistp, PIN_FLD_PROFILE_OBJ,prof_pdp, ebufp);
			inheritedinfo_flist = PIN_FLIST_SUBSTR_ADD(profiles_flistp, PIN_FLD_INHERITED_INFO, ebufp);
			customercareupd_flistp = PIN_FLIST_SUBSTR_ADD(inheritedinfo_flist, PIN_FLD_CUSTOMER_CARE_INFO, ebufp);
			PIN_FLIST_FLD_COPY(customecare_flistp, MSO_FLD_LCO_OBJ,  customercareupd_flistp, MSO_FLD_LCO_OBJ, ebufp);
			PIN_FLIST_FLD_COPY(customecare_flistp, MSO_FLD_LCO_OBJ,  customercareupd_flistp, PIN_FLD_PARENT, ebufp);

			PIN_FLIST_FLD_COPY(customecare_flistp, MSO_FLD_JV_OBJ,  customercareupd_flistp, MSO_FLD_JV_OBJ, ebufp);
			PIN_FLIST_FLD_COPY(customecare_flistp, MSO_FLD_DT_OBJ,  customercareupd_flistp, MSO_FLD_DT_OBJ, ebufp);
			PIN_FLIST_FLD_COPY(customecare_flistp, MSO_FLD_SDT_OBJ, customercareupd_flistp, MSO_FLD_SDT_OBJ, ebufp);
			PIN_FLIST_FLD_COPY(customecare_flistp, MSO_FLD_PP_TYPE, customercareupd_flistp, MSO_FLD_PP_TYPE, ebufp);
			PIN_FLIST_FLD_COPY(customecare_flistp, MSO_FLD_DAS_TYPE, customercareupd_flistp, MSO_FLD_DAS_TYPE, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lcoinfo:  profile update input flist from bulk", updacct_inflistp);
		}
		else
		{
			/************************************** ***************************************
			 *	Prepare update customer profile by copying wholesale profile informaiton
			 ******************************************************************************/
			updacct_inflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(updacct_inflistp, PIN_FLD_POID,profile_pdp, ebufp);		
			profiles_flistp = PIN_FLIST_ELEM_ADD(updacct_inflistp, PIN_FLD_PROFILES, 0, ebufp);
			PIN_FLIST_FLD_SET(profiles_flistp, PIN_FLD_PROFILE_OBJ,profile_pdp, ebufp);
			inheritedinfo_flist = PIN_FLIST_SUBSTR_ADD(profiles_flistp, PIN_FLD_INHERITED_INFO, ebufp);
			updcustomecare_flistp = PIN_FLIST_SUBSTR_ADD(inheritedinfo_flist, PIN_FLD_CUSTOMER_CARE_INFO, ebufp);
			PIN_FLIST_FLD_COPY(customecare_flistp, MSO_FLD_LCO_OBJ, updcustomecare_flistp, MSO_FLD_LCO_OBJ, ebufp);
			PIN_FLIST_FLD_COPY(customecare_flistp, MSO_FLD_LCO_OBJ, updcustomecare_flistp, PIN_FLD_PARENT, ebufp);

			PIN_FLIST_FLD_COPY(wholesale_flistp, MSO_FLD_JV_OBJ, updcustomecare_flistp, MSO_FLD_JV_OBJ, ebufp);
			PIN_FLIST_FLD_COPY(wholesale_flistp, MSO_FLD_DT_OBJ, updcustomecare_flistp, MSO_FLD_DT_OBJ, ebufp);
			PIN_FLIST_FLD_COPY(wholesale_flistp, MSO_FLD_SDT_OBJ, updcustomecare_flistp, MSO_FLD_SDT_OBJ, ebufp);
			PIN_FLIST_FLD_COPY(wholesale_flistp, MSO_FLD_PP_TYPE, updcustomecare_flistp, MSO_FLD_PP_TYPE, ebufp);
			PIN_FLIST_FLD_COPY(wholesale_flistp, MSO_FLD_DAS_TYPE, updcustomecare_flistp, MSO_FLD_DAS_TYPE, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lcoinfo:  profile update input flist from online", updacct_inflistp);

		}

 		vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(out_flist, PIN_FLD_DATA_ARRAY, 1,1, ebufp);
		PIN_FLIST_SUBSTR_SET(vp, updcustomecare_flistp, PIN_FLD_CUSTOMER_CARE_INFO, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lcoinfo: customer care profile update input list", updacct_inflistp);
		PCM_OP(ctxp, PCM_OP_CUST_MODIFY_PROFILE , 0, updacct_inflistp, &updacct_outflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&updacct_inflistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_CUST_MODIFY_PROFILE ", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);
			PIN_FLIST_DESTROY_EX(&findprofile_outflistp, NULL);
			return 0;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lcoinfo output flist", updacct_outflistp);
		PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);


		/************************************** ***************************************
		 *	Billinfo DOM & SEGMENT Updation
		 ******************************************************************************/
		fm_mso_get_all_billinfo(ctxp, acct_pdp, CATV, &billinfo, ebufp);
		res_flistp = PIN_FLIST_ELEM_GET(billinfo, PIN_FLD_RESULTS, 0, 1, ebufp); 
		if (res_flistp)
		{
			billinfo_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_POID, 1, ebufp);
		}


		if (billinfo_pdp)
		{
			lco_pp_type_ptr = PIN_FLIST_FLD_GET(wholesale_flistp, MSO_FLD_PP_TYPE, 1, ebufp);// get LCO PP Type
			if (lco_pp_type_ptr)
			{
				lco_pp_type = *(int32*)lco_pp_type_ptr;
			}

			lco_bill_segment_ptr = PIN_FLIST_FLD_GET(wholesale_flistp, PIN_FLD_CUSTOMER_SEGMENT, 1, ebufp);// get LCO Bill segment type
			if (lco_bill_segment_ptr)
			{
				lco_bill_segment = *(int32*)lco_bill_segment_ptr;
			}

			if (lco_pp_type == PP_TYPE_PRIMARY &&  lco_bill_segment == LCO_IN_BILL_SEGMENT )
			{
				//BDOM=1, SEGMENT=5000
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"PP and BILL_SEGMENT" );
				actg_dom = pp_bdom ;
				billing_segment = pp_bill_segment;
			}

			if (lco_pp_type == PP_TYPE_PRIMARY &&  lco_bill_segment == LCO_IN_NO_BILL_SEGMENT  )
			{
				//BDOM=1, SEGMENT=5050	 --PIN_FLD_BILLING_SEGMENT
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"PP and NO_BILL_SEGMENT" );
				actg_dom = pp_bdom ;
				billing_segment = pp_no_bill_segment;
			}


			if (lco_pp_type == PP_TYPE_SECONDARY &&lco_bill_segment == LCO_IN_NO_BILL_SEGMENT )
			{
				//BDOM from /mso_cfg_city_billing_dom, SEGMENT=104
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"SP and NO_BILL_SEGMENT" );
				actg_dom = *(int32 *)PIN_FLIST_FLD_GET(wholesale_flistp, MSO_FLD_PREF_DOM, 1, ebufp);
				billing_segment = sp_no_bill_segment;

			}

			if (lco_pp_type == PP_TYPE_SECONDARY && lco_bill_segment == LCO_IN_BILL_SEGMENT  )
			{
				//BDOM from /mso_cfg_city_billing_dom, SEGMENT from /config/billing_segment
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"SP and BILL_SEGMENT" );
				actg_dom = *(int32 *)PIN_FLIST_FLD_GET(wholesale_flistp, MSO_FLD_PREF_DOM, 1, ebufp);

				acctinfo = PIN_FLIST_ELEM_GET(in_flist,PIN_FLD_ACCTINFO, 0, 1, ebufp);
				state_city_area_code = PIN_FLIST_FLD_GET(acctinfo, MSO_FLD_AREA, 1, ebufp);
		
				fm_mso_get_state_city_area_code(ctxp, state_city_area_code, NULL, &code_info, ebufp );
				
				if (code_info && (PIN_FLIST_FLD_GET(code_info, PIN_FLD_ERROR_CODE, 1, ebufp)))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_get_state_city_area_code() ", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					PIN_FLIST_DESTROY_EX(&code_info, NULL);
					return 0;
				} 
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "state_city_area_code", code_info);
				state_code = PIN_FLIST_FLD_GET(code_info, PIN_FLD_STATE, 1, ebufp );
				
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "state_city_area_code1", code_info);				
				r_flistp = PIN_FLIST_CREATE(ebufp);

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "state_city_area_code2", code_info);
				fm_mso_get_bill_segment(ctxp, state_code ,&r_flistp, ebufp);
				billing_segment_ptr = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_BILLING_SEGMENT, 1, ebufp);
				if (billing_segment_ptr )
				{
					billing_segment = *(int32*)billing_segment_ptr;
				}
				else
				{
				        billing_segment = 2000;	
				}
			}
			// set NO bill segment for Free type customer
			if (business_type == 99281000 || business_type == 99282000 || business_type == 99281000)
			{
				billing_segment = pp_no_bill_segment;
			}
			// Prepare input to update billinfo
			updbillinfo_inflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, updbillinfo_inflistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, updbillinfo_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);

			profiles_flistp = PIN_FLIST_ELEM_ADD(updbillinfo_inflistp, PIN_FLD_BILLINFO, 0, ebufp);
			PIN_FLIST_FLD_SET(profiles_flistp, PIN_FLD_ACTG_FUTURE_DOM, &actg_dom, ebufp);
			PIN_FLIST_FLD_SET(profiles_flistp, PIN_FLD_BILLING_SEGMENT, &billing_segment, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lcoinfo :  update DOM  & Billing Segment input list", updbillinfo_inflistp);
			PCM_OP(ctxp, PCM_OP_CUST_SET_BILLINFO , 0, updbillinfo_inflistp, &updbillinfo_outflistp, ebufp);
			PIN_FLIST_DESTROY_EX(&updbillinfo_inflistp, NULL);


			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_CUST_SET_BILLINFO ", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&updbillinfo_outflistp, NULL);
				PIN_FLIST_DESTROY_EX(&code_info, NULL);
				PIN_FLIST_DESTROY_EX(&findprofile_outflistp, NULL);
				return 0;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lcoinfo :  update DOM  & Billing Segment output list", updbillinfo_outflistp);
		}

		PIN_FLIST_DESTROY_EX(&updbillinfo_outflistp, NULL);
		PIN_FLIST_DESTROY_EX(&findprofile_outflistp, NULL);
		PIN_FLIST_DESTROY_EX(&billinfo, NULL);


		/************************************** ***************************************
		 *	Update LCO information oin devices 
		 ******************************************************************************/
		fm_mso_update_device_lco(ctxp, acct_pdp, lco_obj, den_hw_flag, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_update_device_lco() ", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&findprofile_outflistp, NULL);
			PIN_FLIST_DESTROY_EX(&code_info, NULL);
			return 0;
		}

	}

	serv_type = fm_mso_get_serv_type(ctxp, acct_pdp, ebufp);
	/* Call function to generate custom event for LCO change notification */
	if (serv_type != 0) 
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Creating LCO Change event.");
		lco_event_flistp = PIN_FLIST_COPY(in_flist,ebufp);
		PIN_FLIST_ELEM_COPY(out_flist, PIN_FLD_DATA_ARRAY, 0, lco_event_flistp, PIN_FLD_DATA_ARRAY, 0, ebufp);
		PIN_FLIST_ELEM_COPY(out_flist, PIN_FLD_DATA_ARRAY, 1, lco_event_flistp, PIN_FLD_DATA_ARRAY, 1, ebufp);
		ret_val = fm_mso_create_lco_change_evt(ctxp, lco_event_flistp, comm_err_flist, ebufp);
		PIN_FLIST_DESTROY_EX(&lco_event_flistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp) )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in creating LCO Change event.");
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);
			return 0;
		}
		else if (ret_val == 1)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "Error in creating LCO Change event.", *comm_err_flist);
			PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);
			return 0;
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Skipping Creation of LCO Change event for CATV.");
	}

	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&code_info, NULL);
	return 2;
}

//PK	if (!prof_pdp)
//PK	{
//PK			
//PK			 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "@@fm_mso_update_lcoinfo input flist", in_flist);
//PK				if (!customecare_flistp)
//PK				{
//PK					return 1;		
//PK				}
//PK				else 
//PK				{
//PK					lco_obj = PIN_FLIST_FLD_GET(customecare_flistp, MSO_FLD_LCO_OBJ, 1, ebufp );
//PK					if (!lco_obj)
//PK					{
//PK						return 1;
//PK					}
//PK				}
//PK
//PK				/************************************** Reading profile object for LCO info ***************************************/
//PK				
//PK				/************ search the LCO profile from the LCO object **************/
//PK				
//PK				db = PIN_POID_GET_DB(lco_obj);
//PK					srch_pdp = PIN_POID_CREATE(db, "/search", (int64)-1, ebufp);
//PK				srch_flistp = PIN_FLIST_CREATE(ebufp);
//PK
//PK					PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
//PK					PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &flags, ebufp);
//PK					PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template, ebufp);
//PK
//PK					args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp);
//PK					PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ,acct_pdp, ebufp);
//PK
//PK				results_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS,0,ebufp);
//PK
//PK				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lcoinfo search  profile input list", srch_flistp);
//PK				PCM_OP(ctxp, PCM_OP_SEARCH,0,srch_flistp,&srch_oflistp, ebufp);
//PK				PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);	
//PK				
//PK				if (PIN_ERRBUF_IS_ERR(ebufp))
//PK					{
//PK							PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH profile", ebufp);
//PK							PIN_ERRBUF_RESET(ebufp);
//PK							PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
//PK							return 0;
//PK					}
//PK				
//PK				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lcoinfo search profile output flist", srch_oflistp);
//PK				
//PK				results_flistp = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
//PK				
//PK				if(results_flistp != NULL)
//PK				{
//PK					profile_pdp = PIN_FLIST_FLD_GET(results_flistp,PIN_FLD_POID, 0, ebufp); 
//PK				
//PK
//PK				readprofile_inflistp = PIN_FLIST_CREATE(ebufp);
//PK				PIN_FLIST_FLD_SET(readprofile_inflistp,PIN_FLD_POID,profile_pdp, ebufp);
//PK
//PK				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lcoinfo read profile input list", readprofile_inflistp);
//PK				PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, readprofile_inflistp, &readprofile_outflistp, ebufp);
//PK				PIN_FLIST_DESTROY_EX(&readprofile_inflistp, NULL);
//PK				if (PIN_ERRBUF_IS_ERR(ebufp))
//PK				{
//PK					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_OBJ profile", ebufp);
//PK					PIN_ERRBUF_RESET(ebufp);
//PK					PIN_FLIST_DESTROY_EX(&readprofile_outflistp, NULL);
//PK					return 0;
//PK				}
//PK				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lcoinfo read profile output flist", readprofile_outflistp);
//PK
//PK				updcustomecare_flistp = PIN_FLIST_ELEM_GET(readprofile_outflistp, PIN_FLD_CUSTOMER_CARE_INFO, 0, 1, ebufp );
//PK
//PK				old_lco_obj = PIN_FLIST_FLD_GET(updcustomecare_flistp, MSO_FLD_LCO_OBJ, 1, ebufp );
//PK
//PK
//PK				/*************************************** adding base value to output flist ****************************************/
//PK				if (!PIN_POID_COMPARE(old_lco_obj, lco_obj, 0, ebufp))
//PK				{
//PK					PIN_FLIST_DESTROY_EX(&readprofile_outflistp, NULL);
//PK					return 1;
//PK				}
//PK
//PK				args_flistp = PIN_FLIST_ELEM_GET(out_flist, PIN_FLD_DATA_ARRAY, 0, 1, ebufp );
//PK				inheritedinfo_flist = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_CUSTOMER_CARE_INFO, 0, ebufp );
//PK				PIN_FLIST_FLD_COPY(customecare_flistp, PIN_FLD_PROFILE_OBJ, inheritedinfo_flist, PIN_FLD_PROFILE_OBJ, ebufp);
//PK				PIN_FLIST_FLD_COPY(updcustomecare_flistp, MSO_FLD_LCO_OBJ, inheritedinfo_flist, MSO_FLD_LCO_OBJ, ebufp);	
//PK
//PK				args_flistp = PIN_FLIST_ELEM_GET(out_flist, PIN_FLD_DATA_ARRAY, 1, 1, ebufp );
//PK				inheritedinfo_flist = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_CUSTOMER_CARE_INFO, 0, ebufp );
//PK				PIN_FLIST_FLD_COPY(customecare_flistp, PIN_FLD_PROFILE_OBJ, inheritedinfo_flist, PIN_FLD_PROFILE_OBJ, ebufp);
//PK				PIN_FLIST_FLD_COPY(customecare_flistp, MSO_FLD_LCO_OBJ, inheritedinfo_flist, MSO_FLD_LCO_OBJ, ebufp);
//PK
//PK				PIN_FLIST_DESTROY_EX(&readprofile_outflistp, NULL);
//PK
//PK				/*************************************** Profile reading ***********************************************************/
//PK
//PK				findprofile_inflistp = PIN_FLIST_CREATE(ebufp);
//PK				PIN_FLIST_FLD_SET(findprofile_inflistp, PIN_FLD_POID, lco_obj, ebufp);
//PK				PIN_FLIST_FLD_SET(findprofile_inflistp, PIN_FLD_ACCOUNT_OBJ, lco_obj, ebufp);
//PK				PIN_FLIST_FLD_SET(findprofile_inflistp, PIN_FLD_TYPE_STR, "/profile/wholesale", ebufp);
//PK
//PK				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lcoinfo find profile input list", findprofile_inflistp);
//PK				PCM_OP(ctxp, PCM_OP_CUST_FIND_PROFILE , PCM_OPFLG_READ_RESULT, findprofile_inflistp, &findprofile_outflistp, ebufp);
//PK				PIN_FLIST_DESTROY_EX(&findprofile_inflistp, NULL);
//PK				if (PIN_ERRBUF_IS_ERR(ebufp))
//PK				{
//PK					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_CUST_FIND_PROFILE ", ebufp);
//PK					PIN_ERRBUF_RESET(ebufp);
//PK					PIN_FLIST_DESTROY_EX(&findprofile_outflistp, NULL);
//PK					return 0;
//PK				}
//PK				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lcoinfo output flist", findprofile_outflistp);
//PK
//PK				results_flistp = PIN_FLIST_ELEM_GET(findprofile_outflistp, PIN_FLD_RESULTS, 0, 1, ebufp );
//PK
//PK				if (!results_flistp)
//PK				{
//PK					return 3;
//PK				}
//PK
//PK				wholesale_flistp = PIN_FLIST_ELEM_GET(results_flistp, MSO_FLD_WHOLESALE_INFO, 0, 1, ebufp );	
//PK
//PK				actg_dom = (int *)PIN_FLIST_FLD_GET(wholesale_flistp, MSO_FLD_PREF_DOM, 1, ebufp);
//PK
//PK				/*************************************** Profile Updation **********************************************************/
//PK				
//PK				updacct_inflistp = PIN_FLIST_CREATE(ebufp);
//PK				//PIN_FLIST_FLD_COPY(customecare_flistp, PIN_FLD_PROFILE_OBJ, updacct_inflistp, PIN_FLD_POID, ebufp);
//PK				PIN_FLIST_FLD_SET(updacct_inflistp, PIN_FLD_POID,profile_pdp, ebufp);
//PK				
//PK				profiles_flistp = PIN_FLIST_ELEM_ADD(updacct_inflistp, PIN_FLD_PROFILES, 0, ebufp);
//PK				//PIN_FLIST_FLD_COPY(customecare_flistp, PIN_FLD_PROFILE_OBJ, profiles_flistp, PIN_FLD_PROFILE_OBJ, ebufp);
//PK				PIN_FLIST_FLD_SET(profiles_flistp, PIN_FLD_PROFILE_OBJ,profile_pdp, ebufp);
//PK
//PK				inheritedinfo_flist = PIN_FLIST_ELEM_ADD(profiles_flistp, PIN_FLD_INHERITED_INFO, 0, ebufp);
//PK				updcustomecare_flistp = PIN_FLIST_ELEM_ADD(inheritedinfo_flist, PIN_FLD_CUSTOMER_CARE_INFO, 0, ebufp);
//PK				PIN_FLIST_FLD_COPY(customecare_flistp, MSO_FLD_LCO_OBJ, updcustomecare_flistp, MSO_FLD_LCO_OBJ, ebufp);
//PK				PIN_FLIST_FLD_COPY(customecare_flistp, MSO_FLD_LCO_OBJ, updcustomecare_flistp, PIN_FLD_PARENT, ebufp);
//PK				PIN_FLIST_FLD_COPY(wholesale_flistp, MSO_FLD_JV_OBJ, updcustomecare_flistp, MSO_FLD_JV_OBJ, ebufp);
//PK				PIN_FLIST_FLD_COPY(wholesale_flistp, MSO_FLD_DT_OBJ, updcustomecare_flistp, MSO_FLD_DT_OBJ, ebufp);
//PK				PIN_FLIST_FLD_COPY(wholesale_flistp, MSO_FLD_SDT_OBJ, updcustomecare_flistp, MSO_FLD_SDT_OBJ, ebufp);
//PK				PIN_FLIST_FLD_COPY(wholesale_flistp, MSO_FLD_PP_TYPE, updcustomecare_flistp, MSO_FLD_PP_TYPE, ebufp);
//PK
//PK				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lcoinfo input list", updacct_inflistp);
//PK				PCM_OP(ctxp, PCM_OP_CUST_MODIFY_PROFILE , 0, updacct_inflistp, &updacct_outflistp, ebufp);
//PK				PIN_FLIST_DESTROY_EX(&updacct_inflistp, NULL);
//PK				if (PIN_ERRBUF_IS_ERR(ebufp))
//PK				{
//PK					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_CUST_MODIFY_PROFILE ", ebufp);
//PK					PIN_ERRBUF_RESET(ebufp);
//PK					PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);
//PK					return 0;
//PK				}
//PK				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lcoinfo output flist", updacct_outflistp);
//PK				PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);
//PK
//PK				/*************************************** Billinfo DOM Updation **********************************************************/
//PK
//PK				updbillinfo_inflistp = PIN_FLIST_CREATE(ebufp);
//PK				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, updbillinfo_inflistp, PIN_FLD_POID, ebufp);
//PK				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, updbillinfo_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);
//PK
//PK				profiles_flistp = PIN_FLIST_ELEM_ADD(updbillinfo_inflistp, PIN_FLD_BILLINFO, 0, ebufp);
//PK				PIN_FLIST_FLD_SET(profiles_flistp, PIN_FLD_ACTG_CYCLE_DOM, actg_dom, ebufp);
//PK
//PK				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lcoinfo update DOM input list", updbillinfo_inflistp);
//PK				PCM_OP(ctxp, PCM_OP_CUST_SET_BILLINFO , 0, updbillinfo_inflistp, &updbillinfo_outflistp, ebufp);
//PK				PIN_FLIST_DESTROY_EX(&updbillinfo_inflistp, NULL);
//PK				
//PK				
//PK				if (PIN_ERRBUF_IS_ERR(ebufp))
//PK				{
//PK					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_CUST_SET_BILLINFO ", ebufp);
//PK					PIN_ERRBUF_RESET(ebufp);
//PK					PIN_FLIST_DESTROY_EX(&updbillinfo_outflistp, NULL);
//PK					return 0;
//PK				}
//PK				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lcoinfo update DOM output flist", updbillinfo_outflistp);
//PK				}
//PK				else
//PK				{
//PK					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " No LCO to change ");
//PK				}
//PK				PIN_FLIST_DESTROY_EX(&updbillinfo_outflistp, NULL);
//PK
//PK				PIN_FLIST_DESTROY_EX(&findprofile_outflistp, NULL);
//PK				PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
//PK	 }
//PK	 else
//PK	 {
//PK	 
//PK	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "@@inside find LCO profile update input flist", in_flist);
//PK	 
//PK	    profile_obj=PIN_FLIST_FLD_GET(in_flist, PIN_FLD_PROFILE_OBJ, 1, ebufp );
//PK		//jv_obj = PIN_FLIST_FLD_GET(orgwholesale_flistp, MSO_FLD_JV_OBJ, 1, ebufp );
//PK		if (!customecare_flistp)
//PK		{
//PK			return 1;		
//PK		}
//PK		else 
//PK		{
//PK			lco_obj = PIN_FLIST_FLD_GET(customecare_flistp, MSO_FLD_LCO_OBJ, 1, ebufp );
//PK			dt_obj = PIN_FLIST_FLD_GET(customecare_flistp, MSO_FLD_DT_OBJ, 1, ebufp );
//PK			jv_obj = PIN_FLIST_FLD_GET(customecare_flistp, MSO_FLD_JV_OBJ, 1, ebufp );
//PK			sdt_obj = PIN_FLIST_FLD_GET(customecare_flistp, MSO_FLD_SDT_OBJ, 1, ebufp );
//PK			pp_type = PIN_FLIST_FLD_GET(in_flist, MSO_FLD_PP_TYPE, 1, ebufp );
//PK			
//PK		    updacct_inflistp = PIN_FLIST_CREATE(ebufp);
//PK			//profile_obj = PIN_FLIST_FLD_GET(resultsprofile_flistp, PIN_FLD_POID, 1, ebufp );
//PK			PIN_FLIST_FLD_SET(updacct_inflistp, PIN_FLD_POID,profile_obj, ebufp);
//PK			customercareupd_flistp = PIN_FLIST_ELEM_ADD(updacct_inflistp, PIN_FLD_CUSTOMER_CARE_INFO, 0, ebufp);
//PK	     			
//PK			if (lco_obj)
//PK			{
//PK			    PIN_FLIST_FLD_SET(customercareupd_flistp, MSO_FLD_LCO_OBJ, lco_obj, ebufp);
//PK		        PIN_FLIST_FLD_SET(customercareupd_flistp, MSO_FLD_PP_TYPE, pp_type, ebufp);
//PK			}
//PK			else if(dt_obj)
//PK			{
//PK			    PIN_FLIST_FLD_SET(customercareupd_flistp, MSO_FLD_DT_OBJ, dt_obj, ebufp);
//PK		    }
//PK			else if(jv_obj)
//PK			{
//PK			    PIN_FLIST_FLD_SET(customercareupd_flistp, MSO_FLD_JV_OBJ, jv_obj, ebufp);
//PK		    }
//PK			else if(sdt_obj)
//PK			{
//PK			    PIN_FLIST_FLD_SET(customercareupd_flistp, MSO_FLD_SDT_OBJ, sdt_obj, ebufp);
//PK		    }
//PK		}
//PK	 
//PK	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "@@fm_mso_update_businessuserinfo find profile update input flist", updacct_inflistp);
//PK	 
//PK		PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, updacct_inflistp, &updacct_outflistp, ebufp);
//PK		PIN_FLIST_DESTROY_EX(&updacct_inflistp, NULL);
//PK		
//PK		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "@@fm_mso_update_businessuserinfo find LCO profile update output flist", updacct_outflistp);
//PK		if (PIN_ERRBUF_IS_ERR(ebufp))
//PK		{
//PK			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH for end users", ebufp);
//PK			PIN_ERRBUF_RESET(ebufp);
//PK			PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);
//PK			return 0;
//PK		}	 
//PK		
//PK		PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);	
//PK		 
//PK	}
//PK	
//PK	/* Call function to generate custom event for LCO change notification */
//PK	lco_event_flistp = PIN_FLIST_COPY(in_flist,ebufp);
//PK	PIN_FLIST_ELEM_COPY(out_flist, PIN_FLD_DATA_ARRAY, 0, lco_event_flistp, PIN_FLD_DATA_ARRAY, 0, ebufp);
//PK	ret_val = fm_mso_create_lco_change_evt(ctxp, lco_event_flistp, ebufp);
//PK	PIN_FLIST_DESTROY_EX(&lco_event_flistp, NULL);
//PK	if (PIN_ERRBUF_IS_ERR(ebufp) || ret_val == 1)
//PK	{
//PK		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in creating LCO Change event.");
//PK		PIN_ERRBUF_RESET(ebufp);
//PK		PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);
//PK		return 0;
//PK	}
//PK	return 2;
//PK}


/**************************************************
* Function: fm_mso_update_businessuserinfo()
* 
* 
***************************************************/
// Modifications by aditya for business user changes 
static int 
fm_mso_update_businessuserinfo(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		*out_flist,
	pin_errbuf_t		*ebufp)
{

	poid_t			*parent_obj = NULL;
	poid_t                  *account_obj=NULL;
	poid_t 			*jv_obj=NULL;
	int32			customer_type = 0;
	poid_t 			*lco_obj=NULL;
	poid_t 			*dt_obj=NULL;
	poid_t 			*sdt_obj=NULL;
	poid_t                  *profile_obj=NULL;
	poid_t                  *old_parent_obj=NULL;
	poid_t			*srch_pdp=NULL;
	
	pin_flist_t             *readprofile_inflistp = NULL;
	pin_flist_t             *readprofile_outflistp=NULL;
	pin_flist_t             *findprofile_inflistp=NULL;
	pin_flist_t             *readwholesale_flistp=NULL;
	pin_flist_t             *orgwholesale_flistp = NULL;
	pin_flist_t             *findprofile_outflistp=NULL;
	pin_flist_t             *resultsprofile_flistp=NULL;
	pin_flist_t             *wholesale_flistp=NULL;
	pin_flist_t             *updacct_inflistp=NULL;
	pin_flist_t             *updwholesale_flistp=NULL;
	pin_flist_t             *updacct_outflistp=NULL;
	pin_flist_t		*srch_flistp=NULL;
	pin_flist_t		*srch_oflistp=NULL;	
	pin_flist_t		*args_flistp=NULL;
	pin_flist_t		*results_flistp=NULL;
	pin_flist_t		*parents=NULL;
	pin_flist_t		*profiles_flistp=NULL;
	pin_flist_t		*inheritedinfo_flist=NULL;
	pin_flist_t		*tagging_correction_flist = NULL;
	pin_flist_t		*tag_out_flist = NULL;
	int64			db = 1;
	int32			flags = 512;
	int32			*reason_id = NULL;
	char			template[128] = "select X from /profile/wholesale where F1 = V1 and profile_t.poid_type = '/profile/wholesale' ";


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_businessuserinfo search  profile input list", in_flist);

	account_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp );
	
	profile_obj=PIN_FLIST_FLD_GET(in_flist, PIN_FLD_PROFILE_OBJ, 1, ebufp );

	orgwholesale_flistp = PIN_FLIST_SUBSTR_GET(in_flist, MSO_FLD_WHOLESALE_INFO, 1, ebufp );
	if (!orgwholesale_flistp)
	{
		return 1;		
	}
	customer_type = *(int32*)PIN_FLIST_FLD_GET(orgwholesale_flistp, PIN_FLD_CUSTOMER_TYPE, 0, ebufp );
	parent_obj = PIN_FLIST_FLD_GET(orgwholesale_flistp, PIN_FLD_PARENT, 0, ebufp );
	jv_obj = PIN_FLIST_FLD_GET(orgwholesale_flistp, MSO_FLD_JV_OBJ, 1, ebufp );
	dt_obj = PIN_FLIST_FLD_GET(orgwholesale_flistp, MSO_FLD_DT_OBJ, 1, ebufp );
	sdt_obj = PIN_FLIST_FLD_GET(orgwholesale_flistp, MSO_FLD_SDT_OBJ, 1, ebufp );
	lco_obj = PIN_FLIST_FLD_GET(orgwholesale_flistp, MSO_FLD_LCO_OBJ, 1, ebufp );

	
	
	//Search profile of child to compare the new and old parent
/*
	db = PIN_POID_GET_DB(account_obj);
	srch_pdp = PIN_POID_CREATE(db, "/search", (int64)-1, ebufp);
	srch_flistp = PIN_FLIST_CREATE(ebufp);

	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ,account_obj, ebufp);

	results_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS,0,ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_businessuserinfo search  profile input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH,0,srch_flistp,&srch_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_businessuserinfo search  profile output list", srch_oflistp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);	
	
	results_flistp = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS,PIN_ELEMID_ANY,0,ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH profile", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
		return 0;
	}
	resultsprofile_flistp = PIN_FLIST_ELEM_GET(results_flistp, MSO_FLD_WHOLESALE_INFO, 0, 1, ebufp );

	if (resultsprofile_flistp)
	{
		old_parent_obj = PIN_FLIST_FLD_GET(resultsprofile_flistp, PIN_FLD_PARENT, 1, ebufp );
		if (old_parent_obj)
		{
			if ( PIN_POID_COMPARE(old_parent_obj, parent_obj, 0, ebufp) == 0)
			{
				PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
				return 1;
			}
		}

	}
*/

	/*readprofile_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(readprofile_inflistp, PIN_FLD_POID, jv_obj, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_businessuserinfo JV read profile input list", readprofile_inflistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ , 0, readprofile_inflistp, &readprofile_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&findprofile_inflistp, NULL);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_OBJ ", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&readprofile_outflistp, NULL);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_businessuserinfo read profile output flist", readprofile_outflistp);

	*/

	/*************************************** Profile reading for DT/SDT/LCO ***********************************************************/
    /*
	findprofile_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(findprofile_inflistp, PIN_FLD_POID, account_obj, ebufp);
	PIN_FLIST_FLD_SET(findprofile_inflistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
	PIN_FLIST_FLD_SET(findprofile_inflistp, PIN_FLD_TYPE_STR, "/profile/wholesale", ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_businessuserinfo find profile input list", findprofile_inflistp);
	PCM_OP(ctxp, PCM_OP_CUST_FIND_PROFILE , PCM_OPFLG_READ_RESULT, findprofile_inflistp, &findprofile_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&findprofile_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_CUST_FIND_PROFILE ", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&findprofile_outflistp, NULL);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_businessuserinfo find JV profile output flist", findprofile_outflistp);

	resultsprofile_flistp = PIN_FLIST_ELEM_GET(findprofile_outflistp, PIN_FLD_RESULTS, 0, 1, ebufp );

	if (!resultsprofile_flistp)
	{
		return 3;
	}

	wholesale_flistp = PIN_FLIST_ELEM_GET(resultsprofile_flistp, MSO_FLD_WHOLESALE_INFO, 0, 1, ebufp );	

	if (!wholesale_flistp)
	{
		return 3;
	}
	
	*/

	/*************************************** Profile reading for JV ***********************************************************/
	
	/*************************************** Updating JV for distributor ******************************************************/
	updacct_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(updacct_inflistp, PIN_FLD_POID,profile_obj, ebufp);
	profiles_flistp = PIN_FLIST_ELEM_ADD(updacct_inflistp, PIN_FLD_PROFILES, 0, ebufp);
	PIN_FLIST_FLD_SET(profiles_flistp, PIN_FLD_PROFILE_OBJ,profile_obj, ebufp);
	inheritedinfo_flist = PIN_FLIST_SUBSTR_ADD(profiles_flistp, PIN_FLD_INHERITED_INFO, ebufp);
	updwholesale_flistp = PIN_FLIST_ELEM_ADD(inheritedinfo_flist, MSO_FLD_WHOLESALE_INFO, 0, ebufp);	
	//if(jv_obj)
	PIN_FLIST_FLD_SET(updwholesale_flistp, MSO_FLD_JV_OBJ, jv_obj, ebufp);
	//if(dt_obj)
	PIN_FLIST_FLD_SET(updwholesale_flistp, MSO_FLD_DT_OBJ, dt_obj, ebufp);
	//if(sdt_obj)
	PIN_FLIST_FLD_SET(updwholesale_flistp, MSO_FLD_SDT_OBJ, sdt_obj, ebufp);
	//if(lco_obj)
	PIN_FLIST_FLD_SET(updwholesale_flistp, MSO_FLD_LCO_OBJ, lco_obj, ebufp);
	//if(parent_obj)
	PIN_FLIST_FLD_SET(updwholesale_flistp, PIN_FLD_PARENT, parent_obj, ebufp);
	
	results_flistp = PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_RESULTS, 0, 1, ebufp);
	if (results_flistp)
	{
		parents =  PIN_FLIST_ELEM_GET(results_flistp, PIN_FLD_PARENTS, 0, 1, ebufp);
		if (parents)
		{
			reason_id = PIN_FLIST_FLD_GET(parents, PIN_FLD_REASON_ID, 1,ebufp); 
		}
	}
	if(reason_id && (*reason_id == MSO_DT_SDT_CHNG_WRNG_TAGGING))
	{
		tagging_correction_flist = PIN_FLIST_COPY(in_flist, ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, tagging_correction_flist, PIN_FLD_PROGRAM_NAME, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_COMMISSION_RECTIFY_SDT_DT_WRONG_TAGGING input flist", tagging_correction_flist);
		PCM_OP(ctxp, MSO_OP_COMMISSION_RECTIFY_SDT_DT_WRONG_TAGGING, 0, in_flist, &tag_out_flist, ebufp);
		PIN_FLIST_DESTROY_EX(&tagging_correction_flist, NULL);	
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_COMMISSION_RECTIFY_SDT_DT_WRONG_TAGGING output flist", tag_out_flist);
		//clear error if any from commission
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_RESET(ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Commission Error");
		}
		PIN_FLIST_DESTROY_EX(&tag_out_flist, NULL);
	}


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_businessuserinfo input list", updacct_inflistp);

		
	PCM_OP(ctxp, PCM_OP_CUST_MODIFY_PROFILE , 0, updacct_inflistp, &updacct_outflistp, ebufp);
	
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_businessuserinfo profile update output flist", updacct_outflistp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in upading profile information ", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&updacct_inflistp, NULL);
		PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);
		return 0;
	}


	PIN_FLIST_DESTROY_EX(&updacct_inflistp, NULL);	
	PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);	
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	return 2;
	

}

/**************************************************
* Function: fm_mso_update_roles()
* 
* 
***************************************************/
static int 
fm_mso_update_roles(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_flist_t			*out_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*updsvc_inflistp = NULL;
	pin_flist_t		*updsvc_outflistp = NULL;
	pin_flist_t		*searchsvc_inflistp = NULL;
	pin_flist_t		*searchsvc_outflistp = NULL;
	pin_flist_t		*roles_flist = NULL;
	pin_flist_t		*set_roles_flist = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;

	char			*new_role = NULL;
	char			*login = NULL;
	char			search_template[100] = " select X from /mso_customer_credential where F1 = V1 ";
	poid_t			*acct_obj = NULL;
	poid_t			*billinfo_obj = NULL;

	int64			db = 0;
	int			search_flags = 768;
	int			account_type = 0;

	void			*vp = NULL;
	void			*vp1 = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_roles :");	
	
	roles_flist = PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_ROLE, PIN_ELEMID_ANY, 1, ebufp );
	acct_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp );
	db = PIN_POID_GET_DB(acct_obj);

	if (!roles_flist)
	{
		return 1;		
	}
	else 
	{
		new_role = PIN_FLIST_FLD_GET(roles_flist, MSO_FLD_ROLES, 1, ebufp );
		if (!new_role)
		{
			return 1;
		}
		login = PIN_FLIST_FLD_GET(roles_flist, PIN_FLD_LOGIN, 1, ebufp );
		if (!login)
		{
			return 1;
		}
	}

	/************************************** Search serivce object ****************************************/

	searchsvc_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(searchsvc_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db,"/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(searchsvc_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
	PIN_FLIST_FLD_SET(searchsvc_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(searchsvc_inflistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_LOGIN, login, ebufp);

	results_flistp = PIN_FLIST_ELEM_ADD(searchsvc_inflistp, PIN_FLD_RESULTS, 1, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_roles Search customer credentials input list", searchsvc_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, searchsvc_inflistp, &searchsvc_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&searchsvc_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH mso customer credentials", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&searchsvc_outflistp, NULL);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_roles Search customer credentials output flist", searchsvc_outflistp);

	results_flistp = PIN_FLIST_ELEM_GET(searchsvc_outflistp, PIN_FLD_RESULTS, 0, 1, ebufp );

	vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(out_flist, PIN_FLD_DATA_ARRAY, 1, 1, ebufp);
	vp1 = (pin_flist_t*)PIN_FLIST_ELEM_ADD(vp, PIN_FLD_ROLE, 0, ebufp);
	PIN_FLIST_FLD_COPY(roles_flist, MSO_FLD_ROLES, vp1, MSO_FLD_ROLES, ebufp);

	vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(out_flist, PIN_FLD_DATA_ARRAY, 0, 1, ebufp);
	vp1 = (pin_flist_t*)PIN_FLIST_ELEM_ADD(vp, PIN_FLD_ROLE, 0, ebufp);
	PIN_FLIST_FLD_COPY(results_flistp, MSO_FLD_ROLES, vp1, MSO_FLD_ROLES, ebufp);

	if (results_flistp)
	{
		updsvc_inflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, updsvc_inflistp, PIN_FLD_POID, ebufp);		
		
		if (strcmp(new_role, "CSR_ADMIN") == 0)
		{
			PIN_FLIST_FLD_SET(updsvc_inflistp, MSO_FLD_ROLES, new_role, ebufp);
			account_type = ACCT_TYPE_CSR_ADMIN;			
		}
		else if (strcmp(new_role, "CSR") == 0)
		{
			PIN_FLIST_FLD_SET(updsvc_inflistp, MSO_FLD_ROLES, new_role, ebufp);
			account_type = ACCT_TYPE_CSR;			
		}
		else if (strcmp(new_role, "LCO") == 0)
		{
			PIN_FLIST_FLD_SET(updsvc_inflistp, MSO_FLD_ROLES, new_role, ebufp);
			account_type = ACCT_TYPE_LCO;			
		}
		else if (strcmp(new_role, "JV") == 0)
		{
			PIN_FLIST_FLD_SET(updsvc_inflistp, MSO_FLD_ROLES, new_role, ebufp);
			account_type = ACCT_TYPE_JV;			
		}
		else if (strcmp(new_role, "DISTRIBUTOR") == 0)
		{
			PIN_FLIST_FLD_SET(updsvc_inflistp, MSO_FLD_ROLES, new_role, ebufp);
			account_type = ACCT_TYPE_DTR;			
		}
		else if (strcmp(new_role, "SUB_DISTRIBUTOR") == 0)
		{
			PIN_FLIST_FLD_SET(updsvc_inflistp, MSO_FLD_ROLES, new_role, ebufp);
			account_type = ACCT_TYPE_SUB_DTR;			
		}
		else if (strcmp(new_role, "OPERATION") == 0)
		{
			PIN_FLIST_FLD_SET(updsvc_inflistp, MSO_FLD_ROLES, new_role, ebufp);
			account_type = ACCT_TYPE_OPERATION;			
		}
		else if (strcmp(new_role, "FIELD_SERVICE") == 0)
		{
			PIN_FLIST_FLD_SET(updsvc_inflistp, MSO_FLD_ROLES, new_role, ebufp);
			account_type = ACCT_TYPE_FIELD_SERVICE;			
		}
		else if (strcmp(new_role, "LOGISTICS") == 0)
		{
			PIN_FLIST_FLD_SET(updsvc_inflistp, MSO_FLD_ROLES, new_role, ebufp);
			account_type = ACCT_TYPE_LOGISTICS;			
		}
		else if (strcmp(new_role, "COLLECTION_AGENT") == 0)
		{
			PIN_FLIST_FLD_SET(updsvc_inflistp, MSO_FLD_ROLES, new_role, ebufp);
			account_type = ACCT_TYPE_COLLECTION_AGENT;			
		}
		else if (strcmp(new_role, "SALES_PERSON") == 0)
		{
			PIN_FLIST_FLD_SET(updsvc_inflistp, MSO_FLD_ROLES, new_role, ebufp);
			account_type = ACCT_TYPE_SALES_PERSON;			
		}
		else if (strcmp(new_role, "SERVICE_PROVIDER") == 0)
		{
			PIN_FLIST_FLD_SET(updsvc_inflistp, MSO_FLD_ROLES, new_role, ebufp);
			account_type = ACCT_TYPE_HTW;			
		}
		else
		{
			PIN_FLIST_FLD_SET(updsvc_inflistp, MSO_FLD_ROLES, new_role, ebufp);
			account_type = ACCT_TYPE_CSR;		
		}
		PIN_FLIST_FLD_SET(updsvc_inflistp, PIN_FLD_CUSTOMER_TYPE, &account_type , ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_roles input list", updsvc_inflistp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, updsvc_inflistp, &updsvc_outflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&updsvc_inflistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_WRITE_FLDS", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&updsvc_outflistp, NULL);
			return 0;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_roles output flist", updsvc_outflistp);
		PIN_FLIST_DESTROY_EX(&updsvc_outflistp, NULL);
		return 2;
	}
	else
	{
		PIN_FLIST_DESTROY_EX(&searchsvc_outflistp, NULL);
		return 1;
	}	
}

static void 
fm_mso_populate_mod_tax_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	int32			account_model,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*acctinfo = NULL;
	pin_flist_t		*profiles = NULL;
	pin_flist_t		*inherited_info = NULL;
	pin_flist_t		*et_zones  = NULL;
	pin_flist_t		*vat_zones = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*cc_info = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*code_info = NULL;
	pin_flist_t		*srch_prof_flistp=NULL;
	pin_flist_t		*arg_prof_flist=NULL;
	pin_flist_t		*result_prof_flist=NULL;
	pin_flist_t		*result_prof_out_flist=NULL;
	pin_flist_t		*srch_prof_out_flistp=NULL;
	pin_flist_t		*result_cc_flist=NULL;
	pin_flist_t		*installation_addr = NULL;

	poid_t			*srch_pdp = NULL;
	poid_t			*srch_pdp_1 = NULL;
	poid_t			*srch_prof_pdp =NULL;

	int32			srch_flag = 512;
	int32			srch_prof_flag =256;
	int32			parent_account_type = -1;
	int32			*pp_type = NULL;
	int32			et_zone_elem_id = -1;

	char			*state = NULL;
	char			*city = NULL;
	char			*et_zone_code = NULL;
	char			*vat_zone_code = NULL;
	char			*state_city_area_code = NULL;

	char			*template_state = "select X from /mso_cfg_state_tax_zones where  F1 = V1 ";
	char			*template_city =  "select X from /mso_cfg_city_tax_zones  where  F1 = V1 and F2 = V2 ";
	char			*template_prof =  "select X from /profile where  F1 = V1 and profile_t.poid_type = '/profile/customer_care' ";	
	int64			db = -1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error before fm_mso_cust_modify_tax_info :");
		return;
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_populate_mod_tax_info :", i_flistp);

  	/**********************************************************************
	* Proceed only if Installation address is passed
	****************************************************************/
	installation_addr = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_NAMEINFO, 2, 1, ebufp);
	if ( !installation_addr )
	{
		return;
	}


	/**********************************************************************
	* Parse Input Flist to get state and city from Installation address
	* Start
	**********************************************************************/
	//acctinfo = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_ACCTINFO, 0, 0, ebufp);
	//state_city_area_code = PIN_FLIST_FLD_GET(acctinfo, MSO_FLD_AREA, 1, ebufp);
	state_city_area_code = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_AREA, 1, ebufp);

        //if(state_city_area_code) 
        //{
	 acctinfo = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_ACCTINFO, 0, 0, ebufp);
         state_city_area_code = PIN_FLIST_FLD_GET(acctinfo, MSO_FLD_AREA, 1, ebufp);
	
	 fm_mso_get_state_city_area_code(ctxp, state_city_area_code, NULL, &code_info, ebufp );
        //} 	
	if (code_info && (PIN_FLIST_FLD_GET(code_info, PIN_FLD_ERROR_CODE, 1, ebufp)))
	{
		*ret_flistp = PIN_FLIST_COPY(code_info, ebufp);
		goto CLEANUP;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "state_city_area_code", code_info);
	state = (char*)PIN_FLIST_FLD_GET(code_info, PIN_FLD_STATE, 1, ebufp);  //State code
	city = (char*)PIN_FLIST_FLD_GET(code_info, MSO_FLD_CITY_CODE, 1, ebufp);
	PIN_FLIST_FLD_COPY(code_info, MSO_FLD_CITY_CODE, i_flistp, MSO_FLD_CITY_CODE, ebufp );

	/**********************************************************************
	* Parse Input Flist to get state and city from Installation address
	* End
	**********************************************************************/

	/**********************************************************************
	* Parse Input Flist to populate element id for MSO_FLD_ET_ZONES array
	* Start
	**********************************************************************/
	//Search for profile object 
	
	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp)) ;
	srch_prof_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_prof_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_prof_flistp, PIN_FLD_POID, srch_prof_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_prof_flistp, PIN_FLD_FLAGS, &srch_prof_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_prof_flistp, PIN_FLD_TEMPLATE, template_prof , ebufp);

	arg_prof_flist = PIN_FLIST_ELEM_ADD(srch_prof_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_prof_flist, PIN_FLD_ACCOUNT_OBJ,PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp), ebufp);

	result_prof_flist = PIN_FLIST_ELEM_ADD(srch_prof_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	result_cc_flist = PIN_FLIST_SUBSTR_ADD(result_prof_flist, PIN_FLD_CUSTOMER_CARE_INFO, ebufp );
        PIN_FLIST_FLD_SET(result_cc_flist, MSO_FLD_PP_TYPE,NULL, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_tax_info SEARCH input flist:PROFILE", srch_prof_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_prof_flistp, &srch_prof_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_prof_flistp,NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH: fm_mso_cust_modify_tax_info()", ebufp);
		goto CLEANUP;;
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_tax_info SEARCH profile output flist", srch_prof_out_flistp);

	result_prof_out_flist = PIN_FLIST_ELEM_GET(srch_prof_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if (!result_prof_out_flist)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"No profile found for this account", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51035", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "No profile found for this account", ebufp);
		goto CLEANUP;
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_tax_info SEARCH profile output flist",result_prof_out_flist);
	
	cc_info = PIN_FLIST_SUBSTR_GET(result_prof_out_flist, PIN_FLD_CUSTOMER_CARE_INFO, 1, ebufp);
	if (!cc_info)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"No profiles.PIN_FLD_CUSTOMER_CARE_INFO for this account", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51034", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "No profiles .PIN_FLD_CUSTOMER_CARE_INFO for this account", ebufp);
		goto CLEANUP;
	}

	pp_type = (int32 *)PIN_FLIST_FLD_GET(cc_info, MSO_FLD_PP_TYPE, 0, ebufp );

	/**********************************************************************
	* Populate element id for MSO_FLD_ET_ZONES array as:
	*    ELEM_ID   MEANING
	*    -------   -------
	*    00        pp & residential
	*    01        pp & Commercial
	*    10        sp & residential
	*    11        sp & Commercial
	**********************************************************************/
	if (account_model == ACCOUNT_MODEL_RES  )
	{
		et_zone_elem_id = (*pp_type)*10 + 0;
	}
	else if (account_model == ACCOUNT_MODEL_CORP || account_model == ACCOUNT_MODEL_MDU  )
	{
		et_zone_elem_id = (*pp_type)*10 + 1;
	}
	/**********************************************************************
	* Parse Input Flist to populate element id for MSO_FLD_ET_ZONES array
	* Start
	**********************************************************************/

	/**********************************************************************
	* Search for  State Level Tax
	**********************************************************************/
	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp)) ;
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template_state , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STATE, state, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_populate_tax_info SEARCH input flist:STATE", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH: fm_mso_populate_tax_info()", ebufp);
		goto CLEANUP;;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_populate_tax_info SEARCH output flist", srch_out_flistp);

	result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if (!result_flist)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"No Tax configured for this state", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51035", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "No Tax configured for this state", ebufp);
		goto CLEANUP;
	}
	et_zones =  PIN_FLIST_ELEM_GET(result_flist, MSO_FLD_ET_ZONES, et_zone_elem_id, 1, ebufp);
	vat_zones =  PIN_FLIST_ELEM_GET(result_flist, MSO_FLD_VAT_ZONES, PIN_ELEMID_ANY, 1, ebufp);

	if (et_zones && vat_zones)
	{
		et_zone_code = PIN_FLIST_FLD_TAKE(et_zones, MSO_FLD_ET_ZONE_CODE, 0, ebufp );
		vat_zone_code = PIN_FLIST_FLD_TAKE(vat_zones, MSO_FLD_VAT_ZONE_CODE, 0, ebufp );
	}
	else
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "No MSO_FLD_ET_ZONE_CODE/MSO_FLD_VAT_ZONE_CODE", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51036", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "No MSO_FLD_ET_ZONE_CODE/MSO_FLD_VAT_ZONE_CODE", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "et_zones", et_zones);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "vat_zones", vat_zones);

	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);

	/**********************************************************************
	* Search for City Level Tax
	**********************************************************************/

	PIN_FLIST_FLD_DROP(srch_flistp, PIN_FLD_TEMPLATE, ebufp );

	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template_city , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_CITY, city, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_populate_tax_info SEARCH input flist: CITY", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH 1: fm_mso_populate_tax_info()", ebufp);
		goto CLEANUP;;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_populate_tax_info SEARCH output flist:CITY", srch_out_flistp);

	result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
//	if (!result_flist)
//	{
//		*ret_flistp = PIN_FLIST_CREATE(ebufp);
//		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51000", ebufp);
//		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "No Tax configured for this city", ebufp);
//		goto CLEANUP;
//	}
	if (result_flist)
	{
		et_zones =  PIN_FLIST_ELEM_GET(result_flist, MSO_FLD_ET_ZONES, et_zone_elem_id, 1, ebufp);
	
		if (et_zones )
		{
			free(et_zone_code);
			et_zone_code = PIN_FLIST_FLD_TAKE(et_zones, MSO_FLD_ET_ZONE_CODE, 0, ebufp );
		}
	}

	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);

	//acctinfo = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_ACCTINFO, 0, 1, ebufp); 

	PIN_FLIST_FLD_PUT(acctinfo, MSO_FLD_ET_ZONE, et_zone_code, ebufp);
	PIN_FLIST_FLD_PUT(acctinfo, MSO_FLD_VAT_ZONE, vat_zone_code, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "modified input_flist ", i_flistp);


	CLEANUP:
	PIN_FLIST_DESTROY_EX(&code_info, NULL); 
	PIN_FLIST_DESTROY_EX(&srch_prof_flistp,NULL);
	return;
}

/**************************************************
* Function: fm_mso_update_device_lco()
* 
* 
***************************************************/

static void 
fm_mso_update_device_lco(
	pcm_context_t		*ctxp,
	poid_t			*acct_pdp,
	poid_t			*lco_obj,
	int32			den_hw_flag,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;
	pin_flist_t		*results_flistp1 = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*alias_list = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*srch_device_flistp = NULL;
	pin_flist_t		*srch_device_oflistp = NULL;
	pin_flist_t		*update_device_iflist = NULL;   
	pin_flist_t		*update_device_oflist = NULL;
	pin_flist_t		*readaccount_iflist = NULL;   
	pin_flist_t		*readaccount_oflist = NULL;
	
	poid_t			*srch_pdp = NULL;

	int64			db=1;
	int32			flags = 256;
	int32			srvc_status_active = PIN_STATUS_ACTIVE;
	int32			srvc_status_inactive = PIN_STATUS_INACTIVE;
	int32			elem_id = 0;
	int32			elem_id1 = 0;

	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie1 = NULL;

	char			*template = " select X from /service where F1 = V1 and  ( F2 = V2 OR F3 = V3 ) ";
	char			*den_nwp = "NDS1";
	char			*hw_nwp = "NDS";
    char            *dsn_nwp = "NDS2";
	char			*namep = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_device_lco :" );	

	db = PIN_POID_GET_DB(lco_obj);
	/*
	0 PIN_FLD_POID                POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS                INT [0] 256
	0 PIN_FLD_TEMPLATE             STR [0] "select X from /service where F1 = V1 and  ( F2 = V2 OR F3 = V3 ) "
	0 PIN_FLD_ARGS               ARRAY [1] allocated 20, used 1
	1     PIN_FLD_ACCOUNT_OBJ     POID [0] 0.0.0.1 /account 122184 0
	0 PIN_FLD_ARGS               ARRAY [2] allocated 20, used 1
	1     PIN_FLD_STATUS          ENUM [0] 10100
	0 PIN_FLD_ARGS               ARRAY [3] allocated 20, used 1
	1     PIN_FLD_STATUS          ENUM [0] 10101
	0 PIN_FLD_RESULTS            ARRAY [*] 
	1     PIN_FLD_ALIAS_LIST     ARRAY [*] NULL
	*/

	srch_pdp = PIN_POID_CREATE(db, "/search", (int64)-1, ebufp);
	srch_flistp = PIN_FLIST_CREATE(ebufp);

	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ,acct_pdp, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS ,&srvc_status_active, ebufp);

 	args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &srvc_status_inactive, ebufp);

	results_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY ,ebufp);
	alias_list = PIN_FLIST_ELEM_ADD(results_flistp, PIN_FLD_ALIAS_LIST, PIN_ELEMID_ANY ,ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_device_lco search  profile input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH,0,srch_flistp,&srch_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH srvc", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
		return;
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_device_lco search profile output flist", srch_oflistp);
	
	while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_oflistp, PIN_FLD_RESULTS, 
	        &elem_id, 1, &cookie, ebufp)) != NULL)
	{
		elem_id1 = 0;
		cookie1 = NULL;
		while ((alias_list = PIN_FLIST_ELEM_GET_NEXT(results_flistp, PIN_FLD_ALIAS_LIST, 
		     &elem_id1, 1, &cookie1, ebufp)) != NULL)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"results_flistp",results_flistp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"alias_list",alias_list);
			namep = PIN_FLIST_FLD_GET(alias_list, PIN_FLD_NAME, 0, ebufp);
			srch_pdp = PIN_POID_CREATE(db, "/search", (int64)-1, ebufp);
			srch_device_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(srch_device_flistp, PIN_FLD_POID, srch_pdp, ebufp);
			PIN_FLIST_FLD_SET(srch_device_flistp, PIN_FLD_FLAGS, &flags, ebufp);
			if (namep && strncmp(namep, "000", 3) == 0)
			{
				PIN_FLIST_FLD_SET(srch_device_flistp, PIN_FLD_TEMPLATE, "select X from /device where F1 = V1 and F2 = V2 ", ebufp);
			
				args_flistp = PIN_FLIST_ELEM_ADD(srch_device_flistp, PIN_FLD_ARGS, 2, ebufp);
				if (den_hw_flag == 1)
				{
					PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_MANUFACTURER, den_nwp, ebufp);
				}
                else if (den_hw_flag == 2)
				{
					PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_MANUFACTURER, dsn_nwp, ebufp);
				}
				else
				{
					PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_MANUFACTURER, hw_nwp, ebufp);
				}
			}
			else
			{
				PIN_FLIST_FLD_SET(srch_device_flistp, PIN_FLD_TEMPLATE, "select X from /device where F1 = V1 ", ebufp);
			}

			args_flistp = PIN_FLIST_ELEM_ADD(srch_device_flistp, PIN_FLD_ARGS, 1, ebufp);
			PIN_FLIST_FLD_COPY(alias_list, PIN_FLD_NAME,args_flistp, PIN_FLD_DEVICE_ID, ebufp);

			results_flistp1 = PIN_FLIST_ELEM_ADD(srch_device_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY ,ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "get_device_poid search input list", srch_device_flistp);
			PCM_OP(ctxp, PCM_OP_SEARCH,0,srch_device_flistp, &srch_device_oflistp, ebufp);
			PIN_FLIST_DESTROY_EX(&srch_device_flistp, NULL);

			results_flistp1 = PIN_FLIST_FLD_GET(srch_device_oflistp, PIN_FLD_RESULTS, 0, ebufp);
			if (results_flistp1)
			{
				readaccount_iflist = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(readaccount_iflist, PIN_FLD_POID,lco_obj,ebufp );
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read account object input", readaccount_oflist);
				PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, readaccount_iflist, &readaccount_oflist, ebufp);
				PIN_FLIST_DESTROY_EX(&readaccount_iflist, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_WRITE_FLDS", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					PIN_FLIST_DESTROY_EX(&readaccount_oflist, NULL);
					PIN_FLIST_DESTROY_EX(&srch_device_oflistp, NULL);
					PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
					return;
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read account object output", update_device_oflist);
				//Update device	 LCO
				update_device_iflist = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(results_flistp1, PIN_FLD_POID, update_device_iflist, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(update_device_iflist, PIN_FLD_ACCOUNT_OBJ, lco_obj, ebufp );
				PIN_FLIST_FLD_COPY(readaccount_oflist, PIN_FLD_ACCOUNT_NO, update_device_iflist, PIN_FLD_SOURCE, ebufp );

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "update device writeflds input", update_device_iflist);
				PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, update_device_iflist, &update_device_oflist, ebufp);
				PIN_FLIST_DESTROY_EX(&update_device_iflist, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_WRITE_FLDS", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					PIN_FLIST_DESTROY_EX(&update_device_oflist, NULL);
					PIN_FLIST_DESTROY_EX(&srch_device_oflistp, NULL);
					PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
					PIN_FLIST_DESTROY_EX(&readaccount_oflist, NULL);
					return;
				}
				PIN_FLIST_DESTROY_EX(&update_device_oflist, NULL);
			}
			PIN_FLIST_DESTROY_EX(&srch_device_oflistp, NULL);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"results_flistp",results_flistp);
		}
	}
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);  
	PIN_FLIST_DESTROY_EX(&readaccount_oflist, NULL);
	return;
}
/**************************************************
* Function: fm_mso_update_bususer_access()
* 
***************************************************/
static int 
fm_mso_update_bususer_access(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		*accessinfo_flistp, 
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t 		*profile_search_flistp = NULL;
	pin_flist_t 		*profile_out_flistp = NULL;
	pin_flist_t 		*accessinfo_flist = NULL;
	pin_flist_t 		*args_flistp = NULL;
	pin_flist_t 		*tmp_flistp = NULL;
	pin_flist_t 		*results_flistp = NULL;
	pin_flist_t 		*inherited_info_flistp = NULL;
	pin_flist_t 		*modify_profile_iflistp = NULL;
	pin_flist_t 		*modify_profile_oflistp = NULL;
	pin_flist_t		*old_access_info = NULL;
	pin_flist_t		*del_flistp = NULL;
	char			search_template[100] = " select X from /profile/wholesale where F1 = V1 and F2 = V2 ";
	char			*old_data_access = NULL;
	char			*new_data_access = NULL;
	poid_t			*acct_obj = NULL;
	poid_t			*prof_obj = NULL;
	poid_t			*srch_pdp = NULL;
	poid_t			*wholesale_obj = NULL;
	poid_t			*old_org_obj = NULL;
	poid_t			*new_org_obj = NULL;

	int			flags = 256;
	int64			db = -1;
	int32			counter = 0;
	int32			modification_required = 0;
	int32			elem_id = 0;
	int32			new_org_level = -1;
	int32			new_data_level = -1;
	pin_cookie_t		cookie = NULL;

	void			*vp =NULL;
	void			*vp1 =NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "Error before calling fm_mso_update_bususer_access", ebufp);
                return 0;
        }
	acct_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 0, ebufp);
	db = PIN_POID_GET_DB(acct_obj);
	accessinfo_flist = PIN_FLIST_ELEM_GET(in_flist, MSO_FLD_ACCESS_INFO, 0, 1, ebufp );
	prof_obj = PIN_FLIST_FLD_GET(accessinfo_flistp, PIN_FLD_PROFILE_OBJ, 1, ebufp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	
	profile_search_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(profile_search_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(profile_search_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(profile_search_flistp, PIN_FLD_TEMPLATE, search_template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(profile_search_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, prof_obj, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(profile_search_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acct_obj, ebufp);
	results_flistp = PIN_FLIST_ELEM_ADD(profile_search_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID, NULL, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(results_flistp, MSO_FLD_ACCESS_INFO, PIN_ELEMID_ANY, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bususer_access search profile input list", profile_search_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, profile_search_flistp, &profile_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&profile_search_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH for business users", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&profile_out_flistp, NULL);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bususer_access search profile output list", profile_out_flistp);

	results_flistp = PIN_FLIST_ELEM_GET(profile_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	old_access_info	= PIN_FLIST_ELEM_GET(results_flistp, MSO_FLD_ACCESS_INFO, 0, 1, ebufp);

	if (results_flistp)
	{
		wholesale_obj = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_POID, 1, ebufp);
		if( !wholesale_obj || (strcmp(PIN_POID_GET_TYPE(wholesale_obj),"/profile/wholesale")) != 0 )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bususer_access : customer is not a business user or have a different profile");	
			PIN_FLIST_DESTROY_EX(&profile_out_flistp, NULL);
			return 1;
		}
		counter = PIN_FLIST_ELEM_COUNT(results_flistp, MSO_FLD_ACCESS_INFO, ebufp);
		if ( counter = 0 )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bususer_access : No Access info to modify for this business user");	
			PIN_FLIST_DESTROY_EX(&profile_out_flistp, NULL);
			return 1;
		}
		else
		{
			vp = (int32*)PIN_FLIST_FLD_GET(old_access_info, MSO_FLD_ORG_ACCESS_LEVEL , 1, ebufp);
			vp1 = (int32*)PIN_FLIST_FLD_GET(accessinfo_flistp, MSO_FLD_ORG_ACCESS_LEVEL , 1, ebufp);
			if (vp && vp1 && *(int32*)vp != *(int32*)vp1)
			{
				modification_required = 1;
				new_org_level = *(int32*)vp1;	
			}
			
			vp = (int32*)PIN_FLIST_FLD_GET(old_access_info, MSO_FLD_DATA_ACCESS_LEVEL , 1, ebufp);
			vp1 = (int32*)PIN_FLIST_FLD_GET(accessinfo_flistp, MSO_FLD_DATA_ACCESS_LEVEL , 1, ebufp);
			if (vp && vp1 && *(int32*)vp != *(int32*)vp1)
			{
				modification_required = 1;
				new_data_level = *(int32*)vp1;	
			}


			if ( modification_required ==0 &&
			     (PIN_FLIST_ELEM_COUNT(accessinfo_flistp, MSO_FLD_ORG_ACCESS_LIST, ebufp) >0)
			    )
			{
				modification_required = 1;	
			}

			if ( modification_required ==0 &&
			     ( PIN_FLIST_ELEM_COUNT(accessinfo_flistp, MSO_FLD_DATA_ACCESS_LIST, ebufp)>0)
			    )
			{
				modification_required = 1;	
			}

			/*

			if ( modification_required ==0 &&
			     (PIN_FLIST_ELEM_COUNT(old_access_info,   MSO_FLD_ORG_ACCESS_LIST, ebufp) != 
			     PIN_FLIST_ELEM_COUNT(accessinfo_flistp, MSO_FLD_ORG_ACCESS_LIST, ebufp))
			    )
			{
				modification_required = 1;	
			}

			if ( modification_required ==0 &&
			     (PIN_FLIST_ELEM_COUNT(old_access_info,   MSO_FLD_DATA_ACCESS_LIST, ebufp) != 
			      PIN_FLIST_ELEM_COUNT(accessinfo_flistp, MSO_FLD_DATA_ACCESS_LIST, ebufp))
			    )
			{
				modification_required = 1;	
			}*/



			//Pawan: Commented below comparision to avoid the elem id mismatch error
			//	This level of comparision is not required as we are replacing 
			//	old list with new list. 
			/*
			if ( modification_required ==0 &&
			     (PIN_FLIST_ELEM_COUNT(old_access_info,   MSO_FLD_ORG_ACCESS_LIST, ebufp) == 
			     PIN_FLIST_ELEM_COUNT(accessinfo_flistp,  MSO_FLD_ORG_ACCESS_LIST, ebufp)) &&
			     (PIN_FLIST_ELEM_COUNT(old_access_info,   MSO_FLD_DATA_ACCESS_LIST, ebufp) == 
			     PIN_FLIST_ELEM_COUNT(accessinfo_flistp,  MSO_FLD_DATA_ACCESS_LIST, ebufp))
			   )
			{	
				elem_id	=0;
				cookie = NULL;
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "List number same");
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "New List", accessinfo_flistp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Old list", old_access_info);
				while(( vp = (pin_flist_t*)PIN_FLIST_ELEM_GET_NEXT( old_access_info, MSO_FLD_ORG_ACCESS_LIST, 
					&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL )
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Getting 1");
					old_org_obj = PIN_FLIST_FLD_GET(vp, MSO_FLD_ORG_OBJ, 1, ebufp);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Getting 2");
					vp1 = (pin_flist_t*)PIN_FLIST_ELEM_GET(accessinfo_flistp, MSO_FLD_ORG_ACCESS_LIST, elem_id, 1, ebufp);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Getting 3");
					new_org_obj = PIN_FLIST_FLD_GET(vp1, MSO_FLD_ORG_OBJ, 1, ebufp);

					if ( new_org_obj && old_org_obj && 
					     PIN_POID_COMPARE(old_org_obj, new_org_obj, 0, ebufp) != 0
					   )
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Poid different");
						modification_required = 1;
						break;
					}
				}

				elem_id	=0;
				cookie = NULL;
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "List number same 2");
				while(( vp = (pin_flist_t*)PIN_FLIST_ELEM_GET_NEXT( old_access_info, MSO_FLD_DATA_ACCESS_LIST, 
					&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL )
				{
					old_data_access = PIN_FLIST_FLD_GET(vp, MSO_FLD_DATA_ACCESS, 1, ebufp);

					vp1 = (pin_flist_t*)PIN_FLIST_ELEM_GET(accessinfo_flistp, MSO_FLD_DATA_ACCESS_LIST, elem_id, 1, ebufp);
					new_data_access = PIN_FLIST_FLD_GET(vp1, MSO_FLD_DATA_ACCESS, 1, ebufp);

					if ( old_data_access && new_data_access && 
					     strcmp(old_data_access, new_data_access) != 0
					   )
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Data value change");
						modification_required = 1;
						break;
					}

				}
			}
			*/

			if (modification_required==1)
			{
			/*	modify_profile_iflistp  = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(modify_profile_iflistp, PIN_FLD_POID, prof_obj, ebufp);
				args_flistp = PIN_FLIST_ELEM_ADD(modify_profile_iflistp, PIN_FLD_PROFILES, 0, ebufp);
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PROFILE_OBJ, prof_obj, ebufp);
				inherited_info_flistp = PIN_FLIST_SUBSTR_ADD(args_flistp, PIN_FLD_INHERITED_INFO, ebufp);
				PIN_FLIST_ELEM_COPY(in_flist, MSO_FLD_ACCESS_INFO, PIN_ELEMID_ANY, 
					inherited_info_flistp, MSO_FLD_ACCESS_INFO, 0, ebufp);
				tmp_flistp = PIN_FLIST_ELEM_TAKE(inherited_info_flistp, MSO_FLD_ACCESS_INFO, 0, 1, ebufp);
				PIN_FLIST_FLD_DROP(tmp_flistp, PIN_FLD_PROFILE_OBJ, ebufp);
				PIN_FLIST_ELEM_PUT(inherited_info_flistp, tmp_flistp, MSO_FLD_ACCESS_INFO, 0, ebufp); 
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bususer_access modify profile input list", modify_profile_iflistp);
				PCM_OP(ctxp, PCM_OP_CUST_MODIFY_PROFILE, 0, modify_profile_iflistp, &modify_profile_oflistp, ebufp);
				PIN_FLIST_DESTROY_EX(&modify_profile_iflistp, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_MODIFY_PROFILE for business users", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					PIN_FLIST_DESTROY_EX(&modify_profile_oflistp, NULL);
					PIN_FLIST_DESTROY_EX(&profile_out_flistp, NULL);
					return 0;
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bususer_access search profile output list", modify_profile_oflistp); */

				//Anyone of MSO_FLD_ORG_ACCESS_LIST or MSO_FLD_DATA_ACCESS_LIST is sent for 
				//modification at a time. Delete the existing entry for array to be modified.
				del_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(del_flistp, PIN_FLD_POID, prof_obj, ebufp);
				args_flistp = PIN_FLIST_ELEM_ADD(del_flistp, MSO_FLD_ACCESS_INFO, 0, ebufp);
				if(PIN_FLIST_ELEM_COUNT(accessinfo_flistp, MSO_FLD_ORG_ACCESS_LIST, ebufp)>0 
					|| new_org_level == ACCESS_GLOBAL)
				{
					PIN_FLIST_ELEM_SET(args_flistp, NULL, MSO_FLD_ORG_ACCESS_LIST, PCM_RECID_ALL, ebufp);
				} 
				if(PIN_FLIST_ELEM_COUNT(accessinfo_flistp, MSO_FLD_DATA_ACCESS_LIST, ebufp)>0
					|| new_data_level == ACCESS_GLOBAL)
				{
					PIN_FLIST_ELEM_SET(args_flistp, NULL, MSO_FLD_DATA_ACCESS_LIST, PCM_RECID_ALL, ebufp);
				} 
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bususer_access delete flds input", del_flistp);
				PCM_OP(ctxp, PCM_OP_DELETE_FLDS, 0, del_flistp, &modify_profile_oflistp, ebufp);
				PIN_FLIST_DESTROY_EX(&del_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&modify_profile_oflistp, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_DELETE_FLDS for business users", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					PIN_FLIST_DESTROY_EX(&profile_out_flistp, NULL);
					return 0;
				}

				//Add the array using write fields as the old one is deleted above. 
				modify_profile_iflistp  = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(modify_profile_iflistp, PIN_FLD_POID, prof_obj, ebufp);
				PIN_FLIST_ELEM_COPY(in_flist, MSO_FLD_ACCESS_INFO, PIN_ELEMID_ANY, 
					modify_profile_iflistp, MSO_FLD_ACCESS_INFO, 0, ebufp);
				tmp_flistp = PIN_FLIST_ELEM_GET(modify_profile_iflistp, MSO_FLD_ACCESS_INFO, 0, 1, ebufp);
				PIN_FLIST_FLD_DROP(tmp_flistp, PIN_FLD_PROFILE_OBJ, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bususer_access write flds input list", modify_profile_iflistp);
				PCM_OP(ctxp, PCM_OP_WRITE_FLDS, PCM_OPFLG_ADD_ENTRY, modify_profile_iflistp, &modify_profile_oflistp, ebufp);
				PIN_FLIST_DESTROY_EX(&modify_profile_iflistp, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_WRITE_FLDS for business users", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					PIN_FLIST_DESTROY_EX(&modify_profile_oflistp, NULL);
					PIN_FLIST_DESTROY_EX(&profile_out_flistp, NULL);
					return 0;
				}
				
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Modification Not required.");
				return 1;
			}
		}	
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "At End 1");
	vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_DATA_ARRAY , 1, 1, ebufp);
	PIN_FLIST_CONCAT(vp, accessinfo_flistp, ebufp);
	PIN_FLIST_FLD_DROP(vp, PIN_FLD_PROFILE_OBJ, ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "At End 2");
	vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_DATA_ARRAY , 0, 1, ebufp);
	PIN_FLIST_CONCAT(vp, old_access_info, ebufp);
	

	PIN_FLIST_DESTROY_EX(&modify_profile_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&profile_out_flistp, NULL);
	return 2;
}

static int
fm_mso_update_cntinfo(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t 		*search_flistp = NULL;
	pin_flist_t 		*search_res_flistp = NULL;
	pin_flist_t 		*rflds_inp_flistp = NULL;
	pin_flist_t 		*rflds_res_flistp = NULL;
	pin_flist_t 		*write_iflistp = NULL;
	pin_flist_t 		*write_res_flistp = NULL;
	pin_flist_t 		*create_flistp = NULL;
	pin_flist_t 		*create_res_flistp = NULL;
	pin_flist_t 		*results_flistp = NULL;
	pin_flist_t 		*args_flistp = NULL;
	pin_flist_t 		*phone_flistp = NULL;
	pin_flist_t 		*new_phone_flistp = NULL;
	pin_flist_t 		*nameinfo_flistp = NULL;
	pin_flist_t 		*new_nameinfo_flistp = NULL;
	char			search_template[100] = " select X from /mso_contact_history where F1 = V1 ";
	poid_t			*cnt_obj = NULL;
	poid_t			*acct_obj = NULL;
	poid_t			*srch_pdp = NULL;
	poid_t			*new_cnt_obj = NULL;
	int			flags = 256;
	int64			db = -1;
	int			counter = 0;
	int			new_counter = 0;
	int			ph_counter = 0;
	int32			*ph_type = NULL;
	int32			*new_ph_type = NULL;
	char			*phonep = NULL;
	char			*new_phonep = NULL;
	int			phone = 0;
	int			mail = 0;
	int			elem_id = 0;
	int			elem_id1 = 0;
	int			elem_id2 = 0;
	int			new_elem_id = 0;
	int			cntr = 0;
	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie1 = NULL;
	pin_cookie_t		cookie2 = NULL;
	pin_cookie_t		new_cookie = NULL;
	char			*old_rmn = NULL;
	char			*old_rmail = NULL;
	char			*rmn = NULL;
	char			*rmail = NULL;
	char			*email_addr = NULL;
	char			*new_email_addr = NULL;
	time_t			current_t = 0;
	pin_flist_t             *result_flist = NULL;
    pin_cookie_t            cookie3 = NULL;
    int                     elem_id3 = 0;
    pin_flist_t             *wflds_acc_inp_flistp = NULL;
    char                    *phn = NULL;
    int32                   *phn_typ = NULL;
    pin_flist_t             *wflds_acc_out_flistp = NULL;
	char			*email = NULL;
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_cntinfo input flist", in_flistp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "Error before calling fm_mso_update_cust_cntinfo", ebufp);
                return 3;
        }
	current_t = pin_virtual_time(NULL);
	phone = MSO_PHONE;
	mail = MSO_EMAIL;
	acct_obj = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	db = PIN_POID_GET_DB(acct_obj);
	rmn = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_RMN, 1, ebufp);			
	rmail = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_RMAIL, 1, ebufp);			
	counter = PIN_FLIST_ELEM_COUNT(in_flistp, PIN_FLD_NAMEINFO, ebufp);
	new_nameinfo_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_NAMEINFO, &elem_id1, 1, &cookie1, ebufp);
	if ( new_nameinfo_flistp != NULL )
	{
		ph_counter = PIN_FLIST_ELEM_COUNT(new_nameinfo_flistp, PIN_FLD_PHONES, ebufp);
		new_email_addr = PIN_FLIST_FLD_GET(new_nameinfo_flistp, PIN_FLD_EMAIL_ADDR, 1, ebufp);
	}
	if ( !rmn && !rmail && !email_addr && ( ph_counter == 0 ))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_cnt_info : Nothing to update");	
		return 1;	
	}	
	rflds_inp_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(rflds_inp_flistp, PIN_FLD_POID, acct_obj, ebufp);
	PIN_FLIST_FLD_SET(rflds_inp_flistp, MSO_FLD_RMN, "(null)", ebufp);					
	PIN_FLIST_FLD_SET(rflds_inp_flistp, MSO_FLD_RMAIL, "(null)", ebufp);					
	PIN_FLIST_ELEM_ADD(rflds_inp_flistp, PIN_FLD_NAMEINFO, PIN_ELEMID_ANY, ebufp);					
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_cntinfo read flds input list", rflds_inp_flistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, rflds_inp_flistp, &rflds_res_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&rflds_inp_flistp, NULL);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_cntinfo read flds result list", rflds_res_flistp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS account", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&rflds_res_flistp, NULL);
		return 3;
	}

	old_rmn = PIN_FLIST_FLD_GET(rflds_res_flistp, MSO_FLD_RMN, 1, ebufp);
	old_rmail = PIN_FLIST_FLD_GET(rflds_res_flistp, MSO_FLD_RMAIL, 1, ebufp);

	counter = 0;
	counter = PIN_FLIST_ELEM_COUNT(in_flistp, PIN_FLD_NAMEINFO, ebufp);
	if ( counter > 0 || rmn || rmail || email_addr )
	{
		elem_id = 0;
		cookie = NULL;
		nameinfo_flistp = PIN_FLIST_ELEM_GET_NEXT(rflds_res_flistp, PIN_FLD_NAMEINFO, &elem_id, 1, &cookie, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_cntinfo read flds Nameinfo result list", nameinfo_flistp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_cntinfo input Nameinfo result list", new_nameinfo_flistp);
                if ( ( nameinfo_flistp != (pin_flist_t *)NULL && new_nameinfo_flistp != (pin_flist_t *)NULL ) || 
									( rmn != (char *)NULL ) || ( rmail != (char *)NULL ) )
                {
					while ((result_flist = PIN_FLIST_ELEM_GET_NEXT(new_nameinfo_flistp, PIN_FLD_PHONES, &elem_id3, 1, &cookie3, ebufp )) != NULL)
					{
						 phn = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_PHONE, 1, ebufp );
						 phn_typ = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_TYPE, 1, ebufp );
						 if(*phn_typ == 5) 
						 {
						   wflds_acc_inp_flistp = PIN_FLIST_CREATE(ebufp);
						   PIN_FLIST_FLD_SET(wflds_acc_inp_flistp, PIN_FLD_POID, acct_obj, ebufp);
						   PIN_FLIST_FLD_SET(wflds_acc_inp_flistp, MSO_FLD_RMN, phn, ebufp);
						   PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_cntinfo write flds acc rmn input list", wflds_acc_inp_flistp);
						   PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, wflds_acc_inp_flistp, &wflds_acc_out_flistp, ebufp);
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_cntinfo write flds acc rmn output list", wflds_acc_out_flistp);
							PIN_FLIST_DESTROY_EX(&wflds_acc_inp_flistp, NULL);
							PIN_FLIST_DESTROY_EX(&wflds_acc_out_flistp, NULL);
						 }
					}
						counter = 0;
                        new_counter = 0;
                        counter = PIN_FLIST_ELEM_COUNT(nameinfo_flistp, PIN_FLD_PHONES, ebufp);
                        new_counter = PIN_FLIST_ELEM_COUNT(new_nameinfo_flistp, PIN_FLD_PHONES, ebufp);
                        if (( counter > 0 && new_counter > 0 ) || ( email_addr != (char *)NULL ) || 
							( rmn != (char *)NULL ) || ( rmail != (char *)NULL ))
                        {
				search_flistp = PIN_FLIST_CREATE(ebufp);
				srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
				PIN_FLIST_FLD_PUT(search_flistp, PIN_FLD_POID, srch_pdp, ebufp);
				PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_FLAGS, &flags, ebufp);
				PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_TEMPLATE, search_template, ebufp);

				args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 1, ebufp);
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acct_obj, ebufp);
				results_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_RESULTS, 0, ebufp);
				PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID, NULL, ebufp);
				PIN_FLIST_ELEM_ADD(results_flistp, MSO_FLD_CONTACT_LIST, PIN_ELEMID_ANY, ebufp);

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_cntinfo search contact history input list", search_flistp);
				PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_flistp, &search_res_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&search_flistp, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling contact history search for end users", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
					PIN_FLIST_DESTROY_EX(&rflds_res_flistp, NULL);
					return 3;
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_cntinfo search contact history output output list", search_res_flistp);

				results_flistp = PIN_FLIST_ELEM_GET(search_res_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
				if (!results_flistp)
				{
					counter = 0;
					create_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_PUT(create_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/mso_contact_history", -1, ebufp), ebufp);
					PIN_FLIST_FLD_SET(create_flistp, PIN_FLD_ACCOUNT_OBJ, acct_obj, ebufp);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_cntinfo create obj input list", create_flistp);
					PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, create_flistp, &create_res_flistp, ebufp);
					PIN_FLIST_DESTROY_EX(&create_flistp, NULL);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_cntinfo create obj result list", create_res_flistp);
					new_cnt_obj = PIN_FLIST_FLD_GET(create_res_flistp, PIN_FLD_POID, 1, ebufp);
					write_iflistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, new_cnt_obj, ebufp);
				}
				else
				{
					cnt_obj = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_POID, 1, ebufp);
					counter = PIN_FLIST_ELEM_COUNT(results_flistp, MSO_FLD_CONTACT_LIST, ebufp);
					write_iflistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, cnt_obj, ebufp);
				}
				elem_id2 = 0;
				cookie2 = NULL;
                                while(( phone_flistp = PIN_FLIST_ELEM_GET_NEXT( nameinfo_flistp, PIN_FLD_PHONES, 
								&elem_id2, 1, &cookie2, ebufp)) != (pin_flist_t *)NULL )
                                {
                                        ph_type = PIN_FLIST_FLD_GET(phone_flistp, PIN_FLD_TYPE, 1, ebufp);
                                        phonep = PIN_FLIST_FLD_GET(phone_flistp, PIN_FLD_PHONE, 1, ebufp);
                                	new_elem_id = 0;
                                	new_cookie = NULL;
                                	while((new_phone_flistp = PIN_FLIST_ELEM_GET_NEXT(new_nameinfo_flistp, PIN_FLD_PHONES,
                                        	&new_elem_id, 1, &new_cookie, ebufp)) != (pin_flist_t *)NULL )
                                	{
                                        	new_ph_type = PIN_FLIST_FLD_GET(new_phone_flistp, PIN_FLD_TYPE, 1, ebufp);
                                        	new_phonep = PIN_FLIST_FLD_GET(new_phone_flistp, PIN_FLD_PHONE, 1, ebufp);
						if (ph_type && new_ph_type && phonep && new_phonep )
						{
							if ( *(int *)ph_type == *(int *)new_ph_type && strcmp(phonep,new_phonep) != 0 && *(int *)ph_type != 16 )
							{
								if ( phonep == (char *)NULL || strlen(phonep) == 0 )
									break;
								if(fm_mso_cust_dup_cntinfo(ctxp,results_flistp,write_iflistp,phonep,current_t,ebufp)==1)
								{
									args_flistp = PIN_FLIST_ELEM_ADD(write_iflistp, MSO_FLD_CONTACT_LIST, counter, ebufp);
									/******* Pavan Bellala -  Fix Wrong Phone type*************
									PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_TYPE, &phone, ebufp);
									***********************************************************/
									PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_TYPE, ph_type, ebufp);
									PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_CONTACT_VALUE, phonep, ebufp);
									PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_EFFECTIVE_T, &current_t, ebufp);
									PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, args_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
									counter = counter + 1;
								}
								break;
							}
							else if ( *(int *)ph_type == *(int *)new_ph_type && strcmp(phonep,new_phonep) != 0 && *(int *)ph_type == 16 )
							{
								if ( phonep == (char *)NULL || strlen(phonep) == 0 )
									break;
								if(fm_mso_cust_dup_cntinfo(ctxp,results_flistp,write_iflistp,phonep,current_t,ebufp)==1)
								{
									args_flistp = PIN_FLIST_ELEM_ADD(write_iflistp, MSO_FLD_CONTACT_LIST, counter, ebufp);
									/******* Pavan Bellala - Fixed wrong type ****
									PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_TYPE, &mail, ebufp);
									******** *************************************/
									PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_TYPE, ph_type, ebufp);
									PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_CONTACT_VALUE, phonep, ebufp);
									PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_EFFECTIVE_T, &current_t, ebufp);
									PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, args_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
									counter = counter + 1;
								}
								break;
							}
							else if ( *(int *)ph_type == *(int *)new_ph_type && strcmp(phonep,new_phonep) == 0 )
							{
								break;
							}
						}
					}
				}	
				email_addr = PIN_FLIST_FLD_GET(nameinfo_flistp, PIN_FLD_EMAIL_ADDR, 1, ebufp);
				if ( email_addr && new_email_addr && strcmp(email_addr,new_email_addr) != 0  && email_addr != (char *)NULL )
				{
					if(fm_mso_cust_dup_cntinfo(ctxp,results_flistp,write_iflistp,email_addr,current_t,ebufp)==1)
					{
						args_flistp = PIN_FLIST_ELEM_ADD(write_iflistp, MSO_FLD_CONTACT_LIST, counter, ebufp);
						PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_TYPE, &mail, ebufp);
						PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_CONTACT_VALUE, email_addr, ebufp);
						PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_EFFECTIVE_T, &current_t, ebufp);
						PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, args_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
						counter = counter + 1;
					}
				}						
				if ( rmn && old_rmn )
				{
					if ( strcmp(rmn,old_rmn) != 0  && old_rmn != (char *)NULL )
					{
						if(fm_mso_cust_dup_cntinfo(ctxp,results_flistp,write_iflistp,old_rmn,current_t,ebufp)==1)
						{
							args_flistp = PIN_FLIST_ELEM_ADD(write_iflistp, MSO_FLD_CONTACT_LIST, counter, ebufp);
							PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_TYPE, &phone, ebufp);
							PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_CONTACT_VALUE, old_rmn, ebufp);
							PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_EFFECTIVE_T, &current_t, ebufp);
							PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, args_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
							counter = counter + 1;
						}
					}
				}
				if ( rmail && old_rmail )
				{
					if ( strcmp(rmail,old_rmail) != 0  && old_rmail != (char *)NULL )
					{
						if(fm_mso_cust_dup_cntinfo(ctxp,results_flistp,write_iflistp,old_rmail,current_t,ebufp)==1)
						{
							args_flistp = PIN_FLIST_ELEM_ADD(write_iflistp, MSO_FLD_CONTACT_LIST, counter, ebufp);
							PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_TYPE, &mail, ebufp);
							PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_CONTACT_VALUE, old_rmail, ebufp);
							PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_EFFECTIVE_T, &current_t, ebufp);
							PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, args_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
							counter = counter + 1;
						}
					}
				}
				cntr = PIN_FLIST_COUNT(write_iflistp,ebufp);
				if ( cntr > 1)
				{
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_cntinfo write flds input list", write_iflistp);
					PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 32, write_iflistp, &write_res_flistp, ebufp);
					PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
					if (PIN_ERRBUF_IS_ERR(ebufp))
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in updating contact info for user", ebufp);
						PIN_ERRBUF_RESET(ebufp);
						PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
						PIN_FLIST_DESTROY_EX(&rflds_res_flistp, NULL);
						PIN_FLIST_DESTROY_EX(&create_res_flistp, NULL);
						PIN_FLIST_DESTROY_EX(&write_res_flistp, NULL);
						return 3;
					}
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_cntinfo write flds result list", write_res_flistp);
				}	
			}
		}
	}
	PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&rflds_res_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&create_res_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&write_res_flistp, NULL);
	return 2;		
}

static int 
fm_mso_update_cust_credential(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*temp_flistp = NULL;
	pin_flist_t 		*search_flistp = NULL;
	pin_flist_t 		*search_res_flistp = NULL;
	pin_flist_t 		*args_flistp = NULL;
	pin_flist_t 		*result_flistp = NULL;
	pin_flist_t 		*write_flistp = NULL;
	pin_flist_t 		*write_res_flistp = NULL;
	pin_flist_t 		*tmp_flistp = NULL;
	
	int32			elem_id = 0;
	int32			srch_flag = 768;
	int32			flg_login_pwsd_change = 0;
	int32			caller = MSO_OP_CUST_MODIFY_CUSTOMER;
	int64			db = -1;
	int			counter = 0;
	
	pin_cookie_t		cookie = NULL;	

	char			*area_name = NULL;
	char			*location_name = NULL;
	char			*street_name = NULL;
	char			*building_name = NULL;
	char			*building_idp = NULL;
	char			*login = NULL;
	char			*passwd = NULL;
	char			*old_login = NULL;
	char			*old_passwd = NULL;
	char			*landmark = NULL;
	char			*template = "select X from /mso_customer_credential where F1 = V1 ";

	poid_t			*acct_obj = NULL;
	poid_t			*srch_pdp = NULL;
	poid_t			*cc_obj = NULL;

	void			*vp = NULL;
	void			*vp1 = NULL;



	elem_id = 0;
	cookie = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "Error before calling fm_mso_update_cust_credential", ebufp);
                return 3;
        }
	login = PIN_FLIST_FLD_GET( in_flistp, PIN_FLD_LOGIN, 1, ebufp);
	passwd = PIN_FLIST_FLD_GET( in_flistp, PIN_FLD_PASSWD, 1, ebufp);
	counter = PIN_FLIST_ELEM_COUNT(in_flistp, PIN_FLD_NAMEINFO, ebufp);
	if ( !login && !passwd && ( counter == 0 ) )
		return 1;
	counter = 0;
	acct_obj = PIN_FLIST_FLD_GET( in_flistp, PIN_FLD_POID, 1, ebufp);
	search_flistp = PIN_FLIST_CREATE(ebufp);
	db = PIN_POID_GET_DB(acct_obj);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
        PIN_FLIST_FLD_PUT(search_flistp, PIN_FLD_POID, srch_pdp, ebufp);
        PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_TEMPLATE, template , ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acct_obj, ebufp);
	result_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_RESULTS, 0, ebufp);
	//PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_cust_credential search list", search_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_flistp, &search_res_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling customer credential search for account", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
		return 3;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_cust_credential:search result flist", search_res_flistp);
	result_flistp = PIN_FLIST_ELEM_GET(search_res_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	if ( !result_flistp)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "No customer credentialis are associated for this user", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
		return 3;
	}	
	cc_obj = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_POID, 1, ebufp );
	write_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(write_flistp, PIN_FLD_POID, cc_obj, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_LOGIN, write_flistp, PIN_FLD_LOGIN, ebufp);		
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PASSWD, write_flistp, PIN_FLD_PASSWD, ebufp);		
	while((temp_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_NAMEINFO,
                    &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
        {
		area_name = PIN_FLIST_FLD_TAKE(temp_flistp, MSO_FLD_AREA_NAME, 1, ebufp);
		location_name = PIN_FLIST_FLD_TAKE(temp_flistp, MSO_FLD_LOCATION_NAME, 1, ebufp);
		street_name = PIN_FLIST_FLD_TAKE(temp_flistp, MSO_FLD_STREET_NAME, 1, ebufp);
		building_name = PIN_FLIST_FLD_TAKE(temp_flistp, MSO_FLD_BUILDING_NAME, 1, ebufp);
        building_idp = PIN_FLIST_FLD_TAKE(temp_flistp, MSO_FLD_BUILDING_ID, 1, ebufp);
		landmark = PIN_FLIST_FLD_TAKE(temp_flistp, MSO_FLD_LANDMARK, 1, ebufp);
	
		if ( area_name || location_name || street_name || building_name || landmark )
		{
			 tmp_flistp= PIN_FLIST_ELEM_ADD(write_flistp, PIN_FLD_NAMEINFO, elem_id, ebufp);
			 if ( area_name )
				PIN_FLIST_FLD_PUT ( tmp_flistp, MSO_FLD_AREA_NAME, area_name, ebufp);			
			 if ( location_name )
				PIN_FLIST_FLD_PUT ( tmp_flistp, MSO_FLD_LOCATION_NAME, location_name, ebufp);			
			 if ( street_name )
				PIN_FLIST_FLD_PUT ( tmp_flistp, MSO_FLD_STREET_NAME, street_name, ebufp);			
			 if ( building_name )
				PIN_FLIST_FLD_PUT ( tmp_flistp, MSO_FLD_BUILDING_NAME, building_name, ebufp);			
             if ( building_idp )
                 PIN_FLIST_FLD_PUT ( tmp_flistp, MSO_FLD_BUILDING_ID, building_idp, ebufp);
			 if ( landmark )
				PIN_FLIST_FLD_PUT ( tmp_flistp, MSO_FLD_LANDMARK, landmark, ebufp);			
		}
		else
		{	
			continue;		
		}
	
	}

	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_ACTION_MODE, &caller, ebufp);
	old_login  = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_LOGIN, 1, ebufp);
	old_passwd = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_PASSWD, 1, ebufp);

	if ((login && old_login && strcmp(login, old_login)!=0) ||
	    (passwd && old_passwd && strcmp(passwd, old_passwd)!=0)
	 )
	{
		flg_login_pwsd_change = 1;
		vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_DATA_ARRAY, 0, 1, ebufp);
		if (vp)
		{
			PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_LOGIN, vp, PIN_FLD_LOGIN, ebufp);
			PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_PASSWD, vp, PIN_FLD_PASSWD, ebufp);
			PIN_FLIST_ELEM_COPY(result_flistp, PIN_FLD_NAMEINFO, 1, vp, PIN_FLD_NAMEINFO, 1, ebufp);
			PIN_FLIST_ELEM_COPY(result_flistp, PIN_FLD_NAMEINFO, 2, vp, PIN_FLD_NAMEINFO, 2, ebufp);
			PIN_FLIST_ELEM_COPY(result_flistp, PIN_FLD_NAMEINFO, 3, vp, PIN_FLD_NAMEINFO, 3, ebufp);
		}
		vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_DATA_ARRAY, 1, 1, ebufp);
		if (vp)
		{
			PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_LOGIN, vp, PIN_FLD_LOGIN, ebufp);
			PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PASSWD_CLEAR, vp, PIN_FLD_PASSWD, ebufp);
			PIN_FLIST_ELEM_COPY(in_flistp, PIN_FLD_NAMEINFO, 1, vp, PIN_FLD_NAMEINFO, 1, ebufp);
			PIN_FLIST_ELEM_COPY(in_flistp, PIN_FLD_NAMEINFO, 2, vp, PIN_FLD_NAMEINFO, 2, ebufp);
			PIN_FLIST_ELEM_COPY(in_flistp, PIN_FLD_NAMEINFO, 3, vp, PIN_FLD_NAMEINFO, 3, ebufp);
		}

	}

	counter = PIN_FLIST_ELEM_COUNT(write_flistp, PIN_FLD_NAMEINFO, ebufp);
	if ( counter > 0 )
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_cust_credential:write_flds input flist", write_flistp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 32, write_flistp, &write_res_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&write_flistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in updating customer credential info for account", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&write_res_flistp, NULL);
			return 3;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_cust_credential:write flds result flist", write_res_flistp);
	}
	PIN_FLIST_DESTROY_EX(&write_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&write_res_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
	return 2;	
}

/**************************************************
* Function: 	fm_mso_get_splitted_name()
* Owner:        Gautam Adak
* Decription:   
*               
* 
***************************************************/
void
fm_mso_get_splitted_name(
	pcm_context_t		*ctxp,
	char			*name,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*acnt_info = NULL;


	char			tmp_name[200];
	char			*first_name   = NULL;
	char			*middle_name = NULL;
	char			*last_name   = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_splitted_name", ebufp);
		return;
	}

	memset(tmp_name, '\0',strlen(tmp_name));
	strcpy(tmp_name, name);
	
	first_name = strtok(tmp_name, " ");
	middle_name = strtok(NULL, " ");
	last_name = strtok(NULL, " ");


	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "first_name");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,  first_name);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "middle_name");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,  middle_name);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "last_name");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,  last_name);

	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_FIRST_NAME, first_name, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_MIDDLE_NAME, middle_name, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_LAST_NAME, last_name, ebufp);

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
	return;
}
/**************************************************
* Function: fm_mso_update_bususer_ar_info()
* 
***************************************************/
static int 
fm_mso_update_bususer_ar_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t 		*search_flistp = NULL;
	pin_flist_t 		*search_res_flistp = NULL;
	pin_flist_t 		*result_flistp = NULL;
	pin_flist_t 		*temp_flistp = NULL;
	pin_flist_t 		*args_flistp = NULL;
	pin_flist_t 		*write_flistp = NULL;
	pin_flist_t 		*write_res_flistp = NULL;
        int64                   db = -1;
        int32                   srch_flag = 256;
        char                    *template = "select X from /mso_ar_limit where F1 = V1 ";
	poid_t			*srch_pdp = NULL;
	poid_t			*ar_obj = NULL;
	poid_t			*acct_obj = NULL;

	pin_decimal_t		*old_daily_adj_limit = NULL;
	pin_decimal_t		*new_daily_adj_limit = NULL;
	pin_decimal_t		*old_monthly_adj_limit = NULL;
	pin_decimal_t		*new_monthly_adj_limit = NULL;



	void			*vp = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "Error before calling fm_mso_create_ar_info", ebufp);
                return 0;
        }
	acct_obj = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1, ebufp);
	db = PIN_POID_GET_DB(acct_obj);
	search_flistp = PIN_FLIST_CREATE(ebufp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
        PIN_FLIST_FLD_PUT(search_flistp, PIN_FLD_POID, srch_pdp, ebufp);
        PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_TEMPLATE, template , ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acct_obj, ebufp);
	result_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_RESULTS, 0, ebufp);
//	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bususer_ar_info search list", search_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_flistp, &search_res_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in searching ar info for business user", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_cust_ar_info:search result flist", search_res_flistp);
	result_flistp = PIN_FLIST_ELEM_GET(search_res_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	if ( !result_flistp )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG, "No ar limit is associated for this user", ebufp);
		//PIN_ERRBUF_RESET(ebufp);
		//PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
		//return 0;
		
		temp_flistp = PIN_FLIST_ELEM_GET( in_flistp, MSO_FLD_AR_INFO, PIN_ELEMID_ANY, 1, ebufp);
		write_flistp = (pin_flist_t*)PIN_FLIST_COPY(temp_flistp, ebufp);
		fm_mso_create_ar_limit(ctxp, acct_obj, &write_flistp, ebufp);
		//PIN_FLIST_DESTROY_EX(&write_flistp, NULL);
		//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "00000");
		//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "out_flistp", out_flistp);

		vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_DATA_ARRAY, 1, 1, ebufp);
		if (vp)
		{
			PIN_FLIST_FLD_COPY(temp_flistp, MSO_FLD_ADJ_CURRENT_VALUE, vp, MSO_FLD_ADJ_CURRENT_VALUE, ebufp);
			PIN_FLIST_FLD_COPY(temp_flistp, MSO_FLD_ADJ_LIMIT, vp, MSO_FLD_ADJ_LIMIT, ebufp);
			PIN_FLIST_FLD_COPY(temp_flistp, MSO_FLD_CURRENT_MONTH, vp, MSO_FLD_CURRENT_MONTH, ebufp);
			PIN_FLIST_FLD_COPY(temp_flistp, MSO_FLD_MONTHLY_ADJ_LIMIT, vp, MSO_FLD_MONTHLY_ADJ_LIMIT, ebufp);
		}
		//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "00000");
	}
	else
	{
		ar_obj = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_POID, 1, ebufp );
		temp_flistp = PIN_FLIST_ELEM_GET( in_flistp, MSO_FLD_AR_INFO, PIN_ELEMID_ANY, 1, ebufp);
		write_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(write_flistp, PIN_FLD_POID, ar_obj, ebufp);
		PIN_FLIST_FLD_COPY(temp_flistp, MSO_FLD_MONTHLY_ADJ_LIMIT, write_flistp, MSO_FLD_MONTHLY_ADJ_LIMIT, ebufp);
		PIN_FLIST_FLD_COPY(temp_flistp, MSO_FLD_ADJ_LIMIT, write_flistp, MSO_FLD_ADJ_LIMIT, ebufp);

		old_daily_adj_limit = PIN_FLIST_FLD_GET(result_flistp, MSO_FLD_ADJ_LIMIT, 1, ebufp );
		new_daily_adj_limit = PIN_FLIST_FLD_GET(temp_flistp, MSO_FLD_ADJ_LIMIT, 1, ebufp );
		old_monthly_adj_limit = PIN_FLIST_FLD_GET(result_flistp, MSO_FLD_MONTHLY_ADJ_LIMIT, 1, ebufp );
		new_monthly_adj_limit = PIN_FLIST_FLD_GET(temp_flistp, MSO_FLD_MONTHLY_ADJ_LIMIT, 1, ebufp );

		if ((old_daily_adj_limit && new_daily_adj_limit && pbo_decimal_compare(old_daily_adj_limit, new_daily_adj_limit, ebufp)!=0)  ||
		    (old_monthly_adj_limit && new_monthly_adj_limit && pbo_decimal_compare(old_monthly_adj_limit, new_monthly_adj_limit, ebufp)!=0)
		   )
		{
			vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_DATA_ARRAY, 0, 1, ebufp);
			PIN_FLIST_FLD_COPY(result_flistp, MSO_FLD_ADJ_CURRENT_VALUE, vp, MSO_FLD_ADJ_CURRENT_VALUE, ebufp);
			PIN_FLIST_FLD_COPY(result_flistp, MSO_FLD_ADJ_LIMIT, vp, MSO_FLD_ADJ_LIMIT, ebufp);
			PIN_FLIST_FLD_COPY(result_flistp, MSO_FLD_CURRENT_MONTH, vp, MSO_FLD_CURRENT_MONTH, ebufp);
			PIN_FLIST_FLD_COPY(result_flistp, MSO_FLD_MONTHLY_ADJ_LIMIT, vp, MSO_FLD_MONTHLY_ADJ_LIMIT, ebufp);

			vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_DATA_ARRAY, 1, 1, ebufp);
			PIN_FLIST_FLD_COPY(temp_flistp, MSO_FLD_ADJ_CURRENT_VALUE, vp, MSO_FLD_ADJ_CURRENT_VALUE, ebufp);
			PIN_FLIST_FLD_COPY(temp_flistp, MSO_FLD_ADJ_LIMIT, vp, MSO_FLD_ADJ_LIMIT, ebufp);
			PIN_FLIST_FLD_COPY(temp_flistp, MSO_FLD_CURRENT_MONTH, vp, MSO_FLD_CURRENT_MONTH, ebufp);
			PIN_FLIST_FLD_COPY(temp_flistp, MSO_FLD_MONTHLY_ADJ_LIMIT, vp, MSO_FLD_MONTHLY_ADJ_LIMIT, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_cust_ar_info write input list", write_flistp);
			PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_flistp, &write_res_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&write_flistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in updating ar info for business user", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&write_res_flistp, NULL);
				return 0;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_cust_ar_info:write result flist", write_res_flistp);
		}
		else
		{
			PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&write_res_flistp, NULL);	
			return 1;
		}
	}
	
	PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&write_res_flistp, NULL);	
	return 2;
}

static int 
fm_mso_update_tax_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp)
{

	pin_flist_t 		*read_flistp = NULL;
	pin_flist_t 		*read_res_flistp = NULL;
	pin_flist_t 		*tax_flistp = NULL;
	pin_flist_t 		*tax_res_flistp = NULL;
	pin_flist_t 		*args_flistp = NULL;
	int32			counter = 0;	
	int			elem_id = 0;	
	pin_cookie_t		cookie = NULL;
	poid_t			*acct_obj = 0;

	void			*vp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "Error before calling fm_mso_update_tax_info", ebufp);
                return 3;
        }
	
	counter = PIN_FLIST_ELEM_COUNT(in_flistp, PIN_FLD_EXEMPTIONS, ebufp);
	if ( counter == 0 )
	{
		return 1;
	}	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Tax EXEMPTION Array Function :");	
	acct_obj = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1, ebufp);

	read_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_POID, acct_obj, ebufp);
	PIN_FLIST_ELEM_ADD(read_flistp, PIN_FLD_EXEMPTIONS, PIN_ELEMID_ANY, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_tax_info: Read exemptions flist",read_flistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flistp, &read_res_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&read_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in reading tax exemption for user", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
		return 3;
	}	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_tax_info: Read exemptions flist",read_res_flistp); 
	vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_DATA_ARRAY, 0, 1, ebufp);
	if (vp)
	{
		PIN_FLIST_CONCAT( vp, read_res_flistp, ebufp);
	}
	PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);

	vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_DATA_ARRAY, 1, 1, ebufp);

	/*counter = PIN_FLIST_ELEM_COUNT(read_res_flistp, PIN_FLD_EXEMPTIONS, ebufp);
	if ( counter > 0 )
	{
		PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
		return 1;
	} 
	else if ( counter == 0 )
	{ */
 
	tax_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(tax_flistp, PIN_FLD_POID, acct_obj, ebufp);

	elem_id	= 0;
	cookie = NULL;
	while ((args_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_EXEMPTIONS, &elem_id, 1, &cookie, ebufp)) != NULL)
	{
		PIN_FLIST_ELEM_COPY(in_flistp, PIN_FLD_EXEMPTIONS, elem_id, tax_flistp,	PIN_FLD_EXEMPTIONS, elem_id, ebufp);
		if (vp)
		{
			PIN_FLIST_ELEM_COPY(in_flistp, PIN_FLD_EXEMPTIONS, elem_id, vp, PIN_FLD_EXEMPTIONS, elem_id, ebufp );
		}

	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_mso_update_tax_info: Input Flist", tax_flistp);
	PCM_OP(ctxp, PCM_OP_CUST_SET_TAXINFO, 0, tax_flistp, &tax_res_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&tax_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in updating tax info", ebufp);
		PIN_ERRBUF_RESET(ebufp);
	//	PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&tax_res_flistp, NULL);
		return 3;
	}	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_mso_update_tax_info: Output Flist", tax_res_flistp);
//}  
//	PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&tax_res_flistp, NULL);
	return 2;
}

static int 
fm_mso_update_payinfo_inv(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_errbuf_t		*ebufp)
{

	pin_flist_t 		*search_flistp = NULL;
	pin_flist_t 		*search_res_flistp = NULL;
	pin_flist_t 		*result_flistp = NULL;
	pin_flist_t 		*args_flistp = NULL;
	pin_flist_t		*update_child_payinfo_iflist = NULL;
	pin_flist_t		*update_child_payinfo_oflist = NULL;
	pin_flist_t		*child_payinfo = NULL;
	pin_flist_t		*child_inherited_info = NULL;
	pin_flist_t		*child_inv_info = NULL;
	pin_flist_t		*nameinfo = NULL;
        int64                   db = -1;
        int32                   srch_flag = 256;
	int32			pay_type = 0;
	int			elem_id = 0;
        pin_cookie_t		cookie = NULL;
	//char                    *template = "select X from /payinfo/invoice where F1 = V1 ";
	char                    *template = "select X from /payinfo where F1 = V1 and F2 = V2";
	char                    name[200];
	poid_t			*srch_pdp = NULL;
	poid_t			*payinfo_obj = NULL;
	poid_t			*acct_obj = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "Error before calling fm_mso_update_payinfo", ebufp);
                return 3;
        }

	acct_obj = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1, ebufp);
	db = PIN_POID_GET_DB(acct_obj);
	search_flistp = PIN_FLIST_CREATE(ebufp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
        PIN_FLIST_FLD_PUT(search_flistp, PIN_FLD_POID, srch_pdp, ebufp);
        PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_TEMPLATE, template , ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acct_obj, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 2, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, (PIN_POID_CREATE(db, "/payinfo/invoice", -1, ebufp)), ebufp);
	result_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_payinfo_inv search flist", search_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_flistp, &search_res_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in searching payinfo invoice for user", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
		return 3;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_payinfo_inv:search result flist", search_res_flistp);
	result_flistp = PIN_FLIST_ELEM_GET(search_res_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	if ( !result_flistp )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_payinfo_inv : Nothing to update");	
		PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
		return 1;	
	}

	payinfo_obj = PIN_FLIST_FLD_GET ( result_flistp, PIN_FLD_POID, 1, ebufp);
	update_child_payinfo_iflist = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY( in_flistp, PIN_FLD_ACCOUNT_OBJ,  update_child_payinfo_iflist, PIN_FLD_POID, ebufp );
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, update_child_payinfo_iflist, PIN_FLD_PROGRAM_NAME, ebufp );

	child_payinfo = PIN_FLIST_ELEM_ADD(update_child_payinfo_iflist, PIN_FLD_PAYINFO, 0, ebufp);
	PIN_FLIST_FLD_SET(child_payinfo, PIN_FLD_POID, payinfo_obj, ebufp);
	pay_type = INVOICE_TYPE;
	PIN_FLIST_FLD_SET(child_payinfo, PIN_FLD_PAY_TYPE, &pay_type, ebufp);

	child_inherited_info = PIN_FLIST_SUBSTR_ADD(child_payinfo, PIN_FLD_INHERITED_INFO, ebufp);
	child_inv_info = PIN_FLIST_ELEM_ADD(child_inherited_info, PIN_FLD_INV_INFO, 0, ebufp);
        nameinfo = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_NAMEINFO, NAMEINFO_BILLING, 1, ebufp) ;
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Nameinfo flist", nameinfo);
        PIN_FLIST_FLD_COPY(nameinfo, PIN_FLD_ADDRESS, child_inv_info, PIN_FLD_ADDRESS, ebufp);
        PIN_FLIST_FLD_COPY(nameinfo, PIN_FLD_CITY, child_inv_info, PIN_FLD_CITY, ebufp);
        PIN_FLIST_FLD_COPY(nameinfo, PIN_FLD_COUNTRY, child_inv_info, PIN_FLD_COUNTRY, ebufp);
        PIN_FLIST_FLD_COPY(nameinfo, PIN_FLD_STATE, child_inv_info, PIN_FLD_STATE, ebufp);
        PIN_FLIST_FLD_COPY(nameinfo, PIN_FLD_ZIP, child_inv_info, PIN_FLD_ZIP, ebufp);
        PIN_FLIST_FLD_COPY(nameinfo, PIN_FLD_EMAIL_ADDR, child_inv_info, PIN_FLD_EMAIL_ADDR, ebufp);
	/*
        strcpy(name, (char*)PIN_FLIST_FLD_GET(nameinfo, PIN_FLD_FIRST_NAME, 0, ebufp));
        strcat(name, " ");
        strcat(name, (char*)PIN_FLIST_FLD_GET(nameinfo, PIN_FLD_MIDDLE_NAME, 0, ebufp));
        strcat(name, " ");
        strcat(name, (char*)PIN_FLIST_FLD_GET(nameinfo, PIN_FLD_LAST_NAME, 0, ebufp));
        PIN_FLIST_FLD_SET(child_inv_info, PIN_FLD_NAME, name, ebufp );
	*/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "update_child_payinfo_iflist ", update_child_payinfo_iflist);
	PCM_OP(ctxp, PCM_OP_CUST_SET_PAYINFO, 0, update_child_payinfo_iflist, &update_child_payinfo_oflist, ebufp);
	PIN_FLIST_DESTROY_EX(&update_child_payinfo_iflist, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in updating payinfo invoice for user", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&update_child_payinfo_oflist, NULL);
		return 3;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "update_child_payinfo_iflist ", update_child_payinfo_iflist);
	PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&update_child_payinfo_oflist, NULL);
	return 2;
}

static int 
fm_mso_update_bal_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*svc_flistp,
	pin_flist_t		*out_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t 		*set_cl_flistp = NULL;
	pin_flist_t 		*set_cl_res_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*limit_flistp = NULL;
	pin_flist_t		*bal_grp_oflist     = NULL;
	pin_flist_t		*credit_limit_array = NULL;
	pin_flist_t		*new_bal_info     = NULL;
	pin_flist_t		*old_bal_info     = NULL;

	pin_decimal_t		*credit_limit_old = NULL;
	pin_decimal_t		*credit_limit_new = NULL;
	pin_decimal_t		*credit_floor_new = NULL;
	pin_decimal_t		*credit_floor_old = NULL;

	pin_decimal_t		*limit_max = pbo_decimal_from_str(CREDIT_LIMIT_MAX_VAL_STR,ebufp);
	pin_decimal_t		*floor_min = pbo_decimal_from_str(CREDIT_FLOOR_MIN_VAL_STR,ebufp);

	int32			elem_id = 0;
	int32			counter = 0;
	int32			*credit_profile_id = NULL;
	int32			retval = 2;

	int64			*resource_id = NULL;

	pin_cookie_t		cookie = NULL; 

	poid_t			*bal_obj = NULL;
	poid_t			*svc_obj = NULL;

	void			*vp = NULL;
	void			*vp1 = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "Error before calling fm_mso_update_bal_info", ebufp);
                return 3;
        }

        svc_obj = PIN_FLIST_FLD_GET(svc_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp );
        bal_obj = PIN_FLIST_FLD_GET(svc_flistp, PIN_FLD_BAL_GRP_OBJ, 1, ebufp );
	counter = PIN_FLIST_ELEM_COUNT(svc_flistp, PIN_FLD_BAL_INFO, ebufp);
	
        if ( !svc_obj || !bal_obj || (counter == 0) )
		return 1;

	fm_mso_read_bal_grp(ctxp, NULL, bal_obj, &bal_grp_oflist, ebufp );
	fm_mso_get_credit_profile(ctxp, -1, &credit_limit_array, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"credit_limit_array",credit_limit_array);

	vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(out_flist, PIN_FLD_DATA_ARRAY, 0, 1, ebufp);
	if (vp)
	{
		old_bal_info = PIN_FLIST_ELEM_ADD(vp, PIN_FLD_BAL_IMPACTS, 0, ebufp);
	}
	vp = PIN_FLIST_ELEM_GET(out_flist, PIN_FLD_DATA_ARRAY, 1, 1, ebufp);
	if (vp)
	{
		new_bal_info = PIN_FLIST_ELEM_ADD(vp, PIN_FLD_BAL_IMPACTS, 0, ebufp);
	}

	elem_id = 0;
	cookie = NULL;
	counter = 0;
	while ((tmp_flistp = PIN_FLIST_ELEM_GET_NEXT(svc_flistp, PIN_FLD_BAL_INFO, &elem_id, 1, &cookie, ebufp )) != NULL)
	{
		resource_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_RESOURCE_ID, 1, ebufp );

		if (bal_grp_oflist && resource_id)
		{
			vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(bal_grp_oflist, PIN_FLD_BALANCES, *resource_id, 1, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"vp", vp);

			credit_profile_id = PIN_FLIST_FLD_GET(vp, PIN_FLD_CREDIT_PROFILE, 1, ebufp);
			if (credit_profile_id)
			{
				vp1 = PIN_FLIST_ELEM_GET(credit_limit_array, PIN_FLD_PROFILES, *credit_profile_id, 1, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"vp1",vp1);
				if (vp1)
				{
					vp = (pin_decimal_t*)PIN_FLIST_FLD_GET(vp1, PIN_FLD_CREDIT_LIMIT, 1, ebufp );
					if (pbo_decimal_is_null(vp, ebufp))
					{
						credit_limit_old = pbo_decimal_copy(limit_max, ebufp);
						
					}
					else
					{
						credit_limit_old = pbo_decimal_copy(vp, ebufp);
					}

					vp = (pin_decimal_t*)PIN_FLIST_FLD_GET(vp1, PIN_FLD_CREDIT_FLOOR, 1, ebufp );
					if (pbo_decimal_is_null(vp, ebufp))
					{
						credit_floor_old = pbo_decimal_copy(floor_min, ebufp);
					}
					else
					{
						credit_floor_old = pbo_decimal_copy(vp, ebufp);
					}
				}
			}
		}

		credit_limit_new = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CREDIT_LIMIT, 1, ebufp );
		credit_floor_new = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CREDIT_FLOOR, 1, ebufp );
		if (!credit_limit_new || !credit_floor_new)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "00000");
			retval=3;
			goto CLEANUP;
		}

		if ( !pbo_decimal_is_null(credit_limit_new, ebufp) && !pbo_decimal_is_null(credit_floor_new, ebufp) &&
		     (pbo_decimal_compare(credit_limit_new, credit_limit_old, ebufp) !=0 ) ||
		     (pbo_decimal_compare(credit_floor_new, credit_floor_old, ebufp) !=0)
		   )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "00000");
			vp = (pin_flist_t*)PIN_FLIST_ELEM_ADD(new_bal_info, PIN_FLD_BAL_INFO, *resource_id, ebufp);

			set_cl_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, set_cl_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, set_cl_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
			PIN_FLIST_FLD_COPY(svc_flistp, PIN_FLD_SERVICE_OBJ, set_cl_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
			PIN_FLIST_FLD_COPY(svc_flistp, PIN_FLD_BAL_GRP_OBJ, set_cl_flistp, PIN_FLD_BAL_GRP_OBJ, ebufp);

			limit_flistp = PIN_FLIST_ELEM_ADD(set_cl_flistp, PIN_FLD_LIMIT, *resource_id, ebufp);
			if (pbo_decimal_compare(credit_limit_new, limit_max, ebufp) ==0)
			{
				PIN_FLIST_FLD_SET(limit_flistp, PIN_FLD_CREDIT_LIMIT, NULL, ebufp);
				PIN_FLIST_FLD_SET(vp, PIN_FLD_CREDIT_LIMIT, NULL, ebufp);
			}
			else
			{
				PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_CREDIT_LIMIT, limit_flistp, PIN_FLD_CREDIT_LIMIT, ebufp);
				PIN_FLIST_FLD_SET(vp, PIN_FLD_CREDIT_LIMIT, credit_limit_new, ebufp);
			}

			if (pbo_decimal_compare(credit_floor_new, floor_min, ebufp) ==0)
			{
				PIN_FLIST_FLD_SET(limit_flistp, PIN_FLD_CREDIT_FLOOR, NULL, ebufp);
				PIN_FLIST_FLD_SET(vp, PIN_FLD_CREDIT_FLOOR, NULL, ebufp);
			}
			else
			{
				PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_CREDIT_FLOOR, limit_flistp, PIN_FLD_CREDIT_FLOOR, ebufp);
				PIN_FLIST_FLD_SET(vp, PIN_FLD_CREDIT_FLOOR, credit_floor_new, ebufp);
			}

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bal_info update credit limit input list", set_cl_flistp);
			PCM_OP(ctxp, PCM_OP_BILL_SET_LIMIT_AND_CR, 0, set_cl_flistp, &set_cl_res_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&set_cl_flistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_BILL_SET_LIMIT_AND_CR", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&set_cl_res_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&bal_grp_oflist, NULL);
				PIN_FLIST_DESTROY_EX(&credit_limit_array, NULL);
				retval=3;
				goto CLEANUP;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bal_info update credit limit output flist", set_cl_res_flistp);
			counter = counter + 1; 
			PIN_FLIST_DESTROY_EX(&set_cl_res_flistp, NULL);

			vp = (pin_flist_t*)PIN_FLIST_ELEM_ADD(old_bal_info, PIN_FLD_BAL_INFO, *resource_id, ebufp);
			if (pbo_decimal_compare(credit_limit_old, limit_max, ebufp) ==0)
			{
				PIN_FLIST_FLD_SET(vp, PIN_FLD_CREDIT_LIMIT, NULL, ebufp);
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "00000");
				PIN_FLIST_FLD_SET(vp, PIN_FLD_CREDIT_LIMIT, credit_limit_old, ebufp);
			}

			if (pbo_decimal_compare(credit_floor_old, floor_min, ebufp) ==0)
			{
				PIN_FLIST_FLD_SET(vp, PIN_FLD_CREDIT_FLOOR, NULL, ebufp);
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "00000");
				PIN_FLIST_FLD_SET(vp, PIN_FLD_CREDIT_FLOOR, credit_floor_old, ebufp);
			}
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "OLD and new value are same");
			retval =1;
		}

		if (credit_limit_old)
		{
			pbo_decimal_destroy(&credit_limit_old);
		}
		if (credit_floor_old)
		{
			pbo_decimal_destroy(&credit_floor_old);
		}
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "00000");
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "out_flist", out_flist);
	

 CLEANUP:
	PIN_FLIST_DESTROY_EX(&set_cl_res_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&bal_grp_oflist, NULL);
	PIN_FLIST_DESTROY_EX(&credit_limit_array, NULL);
	if (limit_max)
	{
		pbo_decimal_destroy(&limit_max);
	}
	if (floor_min)
	{
		pbo_decimal_destroy(&floor_min);
	}	return retval;
}

static int 
fm_mso_update_bb_billinfo(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp)
{
	
	pin_flist_t 		*billinfo_flistp = NULL;
	pin_flist_t 		*create_flistp = NULL;
	pin_flist_t 		*create_res_flistp = NULL;
	pin_flist_t 		*search_flistp = NULL;
	pin_flist_t 		*search_res_flistp = NULL;
	pin_flist_t 		*result_flistp = NULL;
	pin_flist_t 		*write_flistp = NULL;
	pin_flist_t 		*write_res_flistp = NULL;
	pin_flist_t 		*args_flistp = NULL;
	poid_t			*account_obj = NULL;
	poid_t			*billinfo_obj = NULL;
	poid_t			*supr_obj = NULL;
	poid_t			*srch_pdp = NULL;
	int32			status = 0;
	int32			cnt = 0;
	int32			*reason = NULL;
	int32			*flag = NULL;
	int			elem_id = 0;
        int64                   db = -1;
        int32                   srch_flag = 256;
        char                    *template = "select X from /mso_bill_suppression_profile where F1 = V1 and F2 = V2";
	pin_cookie_t		cookie = NULL;

	void			*vp = NULL;
	void			*vp1 = NULL;
	
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "Error before calling fm_mso_update_bb_billinfo", ebufp);
                return 3;
        }

	account_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp);
        while ((billinfo_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flist, PIN_FLD_BILLINFO, &elem_id, 1, &cookie, ebufp )) != NULL)
        {
		billinfo_obj = PIN_FLIST_FLD_GET(billinfo_flistp, PIN_FLD_POID, 1, ebufp);
		reason = PIN_FLIST_FLD_GET(billinfo_flistp, PIN_FLD_REASON, 1, ebufp);
		flag = PIN_FLIST_FLD_GET(billinfo_flistp, PIN_FLD_FLAGS, 1, ebufp);
		if ( !reason && !flag )
			continue;
		else
			cnt = cnt + 1;
		db = PIN_POID_GET_DB(account_obj);
		search_flistp = PIN_FLIST_CREATE(ebufp);
		srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
		PIN_FLIST_FLD_PUT(search_flistp, PIN_FLD_POID, srch_pdp, ebufp);
		PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
		PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_TEMPLATE, template , ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 1, ebufp );
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BILLINFO_OBJ, billinfo_obj, ebufp);
		result_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_RESULTS, 0, ebufp);
		//PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_billinfo search input flist", search_flistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_flistp, &search_res_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&search_flistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, " fm_mso_update_bb_billinfo: Error in searching bill suppression info", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
			return 3;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_billinfo:search result flist", search_res_flistp);
		result_flistp = PIN_FLIST_ELEM_GET(search_res_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
		if ( !result_flistp )
		{
		
			create_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(create_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/mso_bill_suppression_profile", -1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(create_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
			PIN_FLIST_FLD_SET(create_flistp, PIN_FLD_BILLINFO_OBJ, billinfo_obj, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_billinfo create obj input list", create_flistp);
			PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, create_flistp, &create_res_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&create_flistp, NULL);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_billinfo create obj result list", create_res_flistp);
			supr_obj = PIN_FLIST_FLD_GET(create_res_flistp, PIN_FLD_POID, 1, ebufp);
		}
		else
		{
			supr_obj = PIN_FLIST_FLD_GET ( result_flistp, PIN_FLD_POID, 1, ebufp);
			vp  = (pin_flist_t*)PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_DATA_ARRAY, 0, 1, ebufp);
			vp1 = (pin_flist_t*)PIN_FLIST_ELEM_ADD(vp, PIN_FLD_BILLINFO, 0, ebufp);
			PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_REASON, vp1, PIN_FLD_REASON, ebufp);
			PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_FLAGS, vp1, PIN_FLD_FLAGS, ebufp);

		}
		if ( supr_obj && !PIN_POID_IS_NULL(supr_obj))
		{
			write_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(write_flistp, PIN_FLD_POID, supr_obj, ebufp);
			PIN_FLIST_FLD_COPY(billinfo_flistp, PIN_FLD_REASON, write_flistp, PIN_FLD_REASON, ebufp);
			PIN_FLIST_FLD_COPY(billinfo_flistp, PIN_FLD_FLAGS, write_flistp, PIN_FLD_FLAGS, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_billinfo bill suppression write flist", write_flistp);
			PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_flistp, &write_res_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&write_flistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in write flds of bill suppression info for billinfo", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&write_res_flistp, NULL);
				return 3;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_acc_type_change business type update successful", write_res_flistp);
			PIN_FLIST_DESTROY_EX(&write_res_flistp, NULL);
		}
		else
		{
			PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
			return 1;
		}
	}
	if ( cnt >= 1)
	{
		vp  = (pin_flist_t*)PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_DATA_ARRAY, 1, 1, ebufp);
		vp1 = (pin_flist_t*)PIN_FLIST_ELEM_ADD(vp, PIN_FLD_BILLINFO, 0, ebufp);
		PIN_FLIST_FLD_SET(vp1, PIN_FLD_REASON, reason, ebufp);
		PIN_FLIST_FLD_SET(vp1, PIN_FLD_FLAGS, flag, ebufp);

		PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&write_res_flistp, NULL);
		return 2;
	}
	return 1;
}


static int
fm_mso_update_bb_service(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flist,
        pin_flist_t             *out_flist,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *updacct_inflistp = NULL;
        pin_flist_t             *updacct_outflistp = NULL;
        pin_flist_t             *services_flist = NULL;
        pin_flist_t             *updservices_flist = NULL;
        pin_flist_t             *inheritedinfo_flist = NULL;
        pin_flist_t             *inherited_flistp = NULL;
        pin_flist_t             *bb_flistp = NULL;
        pin_flist_t             *old_bb_flistp = NULL;
        pin_flist_t             *new_bb_flistp = NULL;
        pin_flist_t             *new_flistp = NULL;
        pin_flist_t             *readsvc_inflistp = NULL;
        pin_flist_t             *readsvc_outflistp = NULL;
        pin_flist_t             *read_bb_flistp = NULL;
        pin_flist_t             *args_flistp = NULL;
        pin_flist_t             *provaction_inflistp = NULL;
        pin_flist_t             *provaction_outflistp = NULL;
        pin_flist_t             *static_in_flistp = NULL;
        pin_flist_t             *device_flistp = NULL;
        pin_flist_t             *del_flistp = NULL;
        pin_flist_t             *deviceflistp = NULL;
        pin_flist_t             *device_out_flistp = NULL;
        pin_flist_t             *del_out_flistp = NULL;
        pin_flist_t             *temp_flistp = NULL;
        pin_flist_t             *arg_flistp = NULL;
        pin_flist_t             *tmp_flistp = NULL;
		pin_flist_t             *mac_flistp = NULL;
        pin_flist_t             *qmac_flistp = NULL;
        pin_flist_t             *read_alias_flistp= NULL;
		pin_flist_t             *a_flistp = NULL;
		pin_flist_t             *new_cmts_res_flistp = NULL;
		pin_flist_t             *old_cmts_res_flistp = NULL;

        poid_t                  *service_obj = NULL;
        poid_t                  *account_obj = NULL;
        poid_t                  *bp_obj = NULL;
        poid_t                  *mso_obj = NULL;
        
		int                     elem_id = 0;
        int                     *status = NULL;
        int                     *error_codep = NULL;
        int                     status_flags = 0;
        int                     cl_update = 0;
        int                     dev_dis = 0;
        int                     istal_result = 0;
        int                     rec_id = 0;
        int                     disas_flag = 4;
		int						prov_flag = 0;
		int						ret_val = 0;
		int                     aelem_id = 101;
		int						*prov_call = NULL;
		
		int32					*old_carrier_id	 = NULL;
		int32					*new_carrier_id	 = NULL;
        int32                   *istalf = NULL;
        int32                   *old_istalf = NULL;
        int32                   *waiver_flag = NULL;
        int32                   *old_waiver_flag = NULL;
        int32                   *tech = NULL;
        int32                   *old_tech = NULL;
		int32					*is1Click = NULL;
		int32					*old_is1Click = NULL;

		char                   *old_role = NULL;
        char                   *new_role = NULL;
        char                    *login = NULL;
        char                    *passwd = NULL;
        char                    *old_login = NULL;
        char                    *old_passwd = NULL;
        char                    *ne = NULL;
        char                    *old_ne = NULL;
        char                    param[50] = {'\0'};
		char					*prog_name = NULL;
		char					*ip_add_list = NULL;
		char                    *mac_id1 = NULL;
        char                    *mac_id2 = NULL;
        char                    *old_mac_id1 = NULL;
        char                    *old_mac_id2 = NULL;
		char 					*old_other_ne_id = NULL;
		char 					*new_other_ne_id = NULL;

        pin_cookie_t    		cookie = NULL;
        pin_cookie_t    		cookie1 = NULL;
        pin_cookie_t    		cookie2 = NULL;
		pin_cookie_t    		acookie = NULL;

        pin_decimal_t   		*credit_limit = NULL;

		int						mso_plan_status =  0;

        void                    *vp = NULL;
        void                    *vp1 = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return 0;

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_service :");

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Input flist in update service info is ", in_flist);

        services_flist = PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_SERVICES, PIN_ELEMID_ANY, 1, ebufp );
	prog_name = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_PROGRAM_NAME, 0, ebufp);

        if (!services_flist)
        {
                return 1;
        }
        else
        {
                service_obj = PIN_FLIST_FLD_GET(services_flist, PIN_FLD_SERVICE_OBJ, 1, ebufp );
		if( !service_obj || (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_BB)) != 0 )
               		return 1; 
        }
        account_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_ACCOUNT_OBJ, 1, ebufp );

        updacct_inflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, updacct_inflistp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, updacct_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);

        if ( service_obj && strcmp(PIN_POID_GET_TYPE(service_obj),MSO_BB) == 0 )
        {
		login = PIN_FLIST_FLD_GET(services_flist, PIN_FLD_LOGIN, 1, ebufp);
		passwd = PIN_FLIST_FLD_GET(services_flist, PIN_FLD_PASSWD_CLEAR, 1, ebufp);
		if( passwd )
		{
			// Pawan: Commented below since MSO_FLD_PASSWORD comes in SERVICES array
			//inherited_flistp = PIN_FLIST_SUBSTR_GET(services_flist, PIN_FLD_INHERITED_INFO, 1, ebufp);
			//args_flistp = PIN_FLIST_SUBSTR_GET(inherited_flistp, MSO_FLD_BB_INFO, 1, ebufp);
			//if ( !args_flistp )
			//	return 11;
			passwd = PIN_FLIST_FLD_GET(services_flist, MSO_FLD_PASSWORD, 1, ebufp);
			if ( !passwd || strcmp(passwd,"") == 0 )
			{
                        	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "MSO_FLD_PASSWORD is manadatory when there is a change in password", ebufp);
                        	PIN_ERRBUF_RESET(ebufp);
                        	PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
                        	return 11;
			}
			//Pawan: Added below block to move MSO_FLD_PASSWORD to inside BB_INFO
			inherited_flistp = PIN_FLIST_SUBSTR_GET(services_flist, PIN_FLD_INHERITED_INFO, 1, ebufp);
			if(!inherited_flistp)
			{
				inherited_flistp = PIN_FLIST_SUBSTR_ADD(services_flist, PIN_FLD_INHERITED_INFO, ebufp);
			}
			args_flistp = PIN_FLIST_SUBSTR_ADD(inherited_flistp, MSO_FLD_BB_INFO, ebufp);
			PIN_FLIST_FLD_MOVE(services_flist, MSO_FLD_PASSWORD, args_flistp, MSO_FLD_PASSWORD, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Service flist after add ", services_flist);
		}
		status = PIN_FLIST_FLD_GET(services_flist, PIN_FLD_STATUS, 1, ebufp );
		inherited_flistp = PIN_FLIST_SUBSTR_GET(services_flist, PIN_FLD_INHERITED_INFO, 1, ebufp);
		if ( inherited_flistp )	
		{
			bb_flistp = PIN_FLIST_SUBSTR_GET(inherited_flistp, MSO_FLD_BB_INFO, 1, ebufp);
			if ( bb_flistp )
			{
				istalf = PIN_FLIST_FLD_GET(bb_flistp, MSO_FLD_ISTAL_FLAG, 1, ebufp);
			    new_role = PIN_FLIST_FLD_GET(bb_flistp, MSO_FLD_ROLES, 1, ebufp);
            	ne = PIN_FLIST_FLD_GET(bb_flistp, MSO_FLD_NETWORK_ELEMENT, 1, ebufp);
				// Added the below code for JIO-CPS Integration-ISP Enhancement
				if (ne != NULL)
				{
					// Fetch the Carrier ID information using the new CMTS ID from the /mso_cfg_cmts_master object
					fm_mso_get_carrier_id(ctxp, ne, &new_cmts_res_flistp, ebufp);
				}
				tech = PIN_FLIST_FLD_GET(bb_flistp, MSO_FLD_TECHNOLOGY, 1, ebufp);
				is1Click = PIN_FLIST_FLD_GET(bb_flistp, MSO_FLD_IS1CLICK_FLAG, 1, ebufp);
				waiver_flag = PIN_FLIST_FLD_GET(bb_flistp, PIN_FLD_REAUTH_FLAG, 1, ebufp);
			}
		}
		// This is for GPON implementation. We would store the QPS MAC ID 1 & 2 in BRM.
		if (inherited_flistp && PIN_FLIST_ELEM_COUNT(inherited_flistp, PIN_FLD_ALIAS_LIST, ebufp) > 0){
			mac_flistp = PIN_FLIST_ELEM_GET(inherited_flistp, PIN_FLD_ALIAS_LIST, 101, 1, ebufp);
			if (mac_flistp){
				mac_id1 = PIN_FLIST_FLD_GET(mac_flistp, PIN_FLD_NAME, 1, ebufp);
			}
			qmac_flistp = PIN_FLIST_ELEM_GET(inherited_flistp, PIN_FLD_ALIAS_LIST, 102, 1, ebufp);
			if (qmac_flistp){
				mac_id2 = PIN_FLIST_FLD_GET(qmac_flistp, PIN_FLD_NAME, 1, ebufp);
			}
		}
		readsvc_inflistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(readsvc_inflistp, PIN_FLD_POID, service_obj, ebufp);
                PIN_FLIST_FLD_SET(readsvc_inflistp, PIN_FLD_BAL_GRP_OBJ, NULL, ebufp);

                if ( login )
			PIN_FLIST_FLD_SET( readsvc_inflistp, PIN_FLD_LOGIN, NULL, ebufp ); 
                if ( passwd )
			PIN_FLIST_FLD_SET( readsvc_inflistp, PIN_FLD_PASSWD, NULL, ebufp ); 
                if (status)
                {
                        PIN_FLIST_FLD_SET(readsvc_inflistp, PIN_FLD_STATUS, NULL, ebufp);
                }
		old_bb_flistp = PIN_FLIST_SUBSTR_ADD(readsvc_inflistp, MSO_FLD_BB_INFO, ebufp);
                PIN_FLIST_FLD_SET(old_bb_flistp, MSO_FLD_AGREEMENT_NO, NULL, ebufp);

        if ( new_role )
            PIN_FLIST_FLD_SET(old_bb_flistp, MSO_FLD_ROLES, NULL, ebufp);

		//Pawan: Commmented below if to always fetch ISTAL flag.
		//if ( istalf )
		PIN_FLIST_FLD_SET(old_bb_flistp, MSO_FLD_ISTAL_FLAG, NULL, ebufp);
		if ( ne )
			PIN_FLIST_FLD_SET(old_bb_flistp, MSO_FLD_NETWORK_ELEMENT, NULL, ebufp);
		if ( tech )
			PIN_FLIST_FLD_SET(old_bb_flistp, MSO_FLD_TECHNOLOGY, NULL, ebufp);
		if ( is1Click )
			PIN_FLIST_FLD_SET(old_bb_flistp, MSO_FLD_IS1CLICK_FLAG, NULL, ebufp);
                if ( waiver_flag )
                        PIN_FLIST_FLD_SET(old_bb_flistp, PIN_FLD_REAUTH_FLAG, NULL, ebufp);
		if ( passwd )
			PIN_FLIST_FLD_SET(old_bb_flistp, MSO_FLD_PASSWORD, NULL, ebufp);			
		// If mac_id is provided, then this is to update MAC_ID in BB_INFO
		//if ( mac_id1 && (strcmp(mac_id1, "") != 0))
		if(mac_flistp)
		{
			tmp_flistp = PIN_FLIST_ELEM_ADD(readsvc_inflistp, PIN_FLD_ALIAS_LIST, 101, ebufp);
			PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_NAME, NULL, ebufp);
		}
		//if ( mac_id2 && (strcmp(mac_id2, "") != 0))
		if (qmac_flistp)
		{
			tmp_flistp = PIN_FLIST_ELEM_ADD(readsvc_inflistp, PIN_FLD_ALIAS_LIST, 102, ebufp);
			PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_NAME, NULL, ebufp);
		}

                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_service read svc input list", readsvc_inflistp);
                PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, readsvc_inflistp, &readsvc_outflistp, ebufp);
                PIN_FLIST_DESTROY_EX(&readsvc_inflistp, NULL);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS svc for bgp", ebufp);
                        PIN_ERRBUF_RESET(ebufp);
                        PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
                        return 0;
                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_service read svc for bgp output flist", readsvc_outflistp);

                if (!readsvc_outflistp)
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_service read svc output flist is not having any service details");
                        return 0;
                }

		read_bb_flistp = PIN_FLIST_SUBSTR_GET(readsvc_outflistp, MSO_FLD_BB_INFO, 1, ebufp);
		old_ne = PIN_FLIST_FLD_GET(read_bb_flistp, MSO_FLD_NETWORK_ELEMENT, 1, ebufp );
		// Added the below code for JIO-CPS Integration-ISP Enhancement
		if (old_ne != NULL)
		{
			// Fetch the Carrier ID information using the old CMTS ID from the /mso_cfg_cmts_master object
			fm_mso_get_carrier_id(ctxp, old_ne, &old_cmts_res_flistp, ebufp);
		}
		
		if(old_cmts_res_flistp != NULL && new_cmts_res_flistp)
		{
			old_carrier_id = PIN_FLIST_FLD_GET(old_cmts_res_flistp, PIN_FLD_CARRIER_ID, 1, ebufp );
			new_carrier_id = PIN_FLIST_FLD_GET(new_cmts_res_flistp, PIN_FLD_CARRIER_ID, 1, ebufp );
			
			old_other_ne_id = PIN_FLIST_FLD_GET(old_cmts_res_flistp, MSO_FLD_OTHER_NE_ID, 1, ebufp );
			new_other_ne_id = PIN_FLIST_FLD_GET(new_cmts_res_flistp, MSO_FLD_OTHER_NE_ID, 1, ebufp );
			
			if(old_carrier_id != NULL && new_carrier_id != NULL && old_other_ne_id != NULL && new_other_ne_id != NULL)
			{	
				// Compare the old and new Network IDs & Other_ne_id values
				if ( (*(int32 *)old_carrier_id != *(int32 *)new_carrier_id) &&
					 (strcmp(old_other_ne_id, new_other_ne_id) != 0) )
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Network Provider for old and new CMTS ID does not match", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					PIN_FLIST_DESTROY_EX(&old_cmts_res_flistp, NULL);
					PIN_FLIST_DESTROY_EX(&new_cmts_res_flistp, NULL);
					return 16;
				}
			}
			
			PIN_FLIST_DESTROY_EX(&old_cmts_res_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&new_cmts_res_flistp, NULL);
		}
		
		old_istalf = PIN_FLIST_FLD_GET(read_bb_flistp, MSO_FLD_ISTAL_FLAG, 1, ebufp );
		old_tech = PIN_FLIST_FLD_GET(read_bb_flistp, MSO_FLD_TECHNOLOGY, 1, ebufp );
		old_login = PIN_FLIST_FLD_GET(readsvc_outflistp, PIN_FLD_LOGIN, 1, ebufp );
		old_passwd = PIN_FLIST_FLD_GET(read_bb_flistp, MSO_FLD_PASSWORD, 1, ebufp );
		old_is1Click = PIN_FLIST_FLD_GET(read_bb_flistp, MSO_FLD_IS1CLICK_FLAG, 1, ebufp );
        old_role = PIN_FLIST_FLD_GET(read_bb_flistp, MSO_FLD_ROLES, 1, ebufp );
		old_waiver_flag = PIN_FLIST_FLD_GET(read_bb_flistp, PIN_FLD_REAUTH_FLAG, 1, ebufp );

		PIN_FLIST_FLD_COPY(read_bb_flistp, MSO_FLD_AGREEMENT_NO, out_flist, MSO_FLD_AGREEMENT_NO, ebufp);
		
		tmp_flistp = PIN_FLIST_ELEM_GET(readsvc_outflistp, PIN_FLD_ALIAS_LIST, 101, 1, ebufp);
		if(tmp_flistp)
			old_mac_id1 = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 1, ebufp);
		tmp_flistp = PIN_FLIST_ELEM_GET(readsvc_outflistp, PIN_FLD_ALIAS_LIST, 102, 1,ebufp);
		if (tmp_flistp)
			old_mac_id2 = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 1, ebufp);

/*		if ( ( *(int32 *)old_tech == MSO_DOCSIS_2 && *(int32 *)tech != MSO_DOCSIS_2) || 
						( *(int32 *)tech == MSO_DOCSIS_2 && *(int32 *)old_tech != MSO_DOCSIS_2 ) )
		{	
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_service technology change");
                        return 4;
		}
		if ( tech = MSO_DOCSIS_2 && tech != MSO_DOCSIS_2 )
		{	
		
		}
		if ( old_tech = MSO_DOCSIS_2 && tech != MSO_DOCSIS_2 )
		{	
		
		}  */

                args_flistp = PIN_FLIST_ELEM_GET(out_flist, PIN_FLD_DATA_ARRAY, 0, 1, ebufp );
                inheritedinfo_flist = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_SERVICES, elem_id, ebufp );
                PIN_FLIST_FLD_SET(inheritedinfo_flist, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
		if (!mac_id1 && !mac_id2)
		{
			PIN_FLIST_ELEM_SET(inheritedinfo_flist, read_bb_flistp, MSO_FLD_BB_INFO, 0, ebufp );
			PIN_FLIST_FLD_COPY(readsvc_outflistp, PIN_FLD_STATUS, inheritedinfo_flist, PIN_FLD_STATUS, ebufp);
			PIN_FLIST_FLD_COPY(readsvc_outflistp, PIN_FLD_LOGIN, inheritedinfo_flist, PIN_FLD_LOGIN, ebufp);
			PIN_FLIST_FLD_COPY(readsvc_outflistp, PIN_FLD_PASSWD, inheritedinfo_flist, PIN_FLD_PASSWD, ebufp);
		}
		else{
			if((mac_id1 && strcmp(mac_id1, "") != 0 ) || ((old_mac_id1 && strcmp(old_mac_id1, "") != 0)
                                        && (!mac_id1 || strcmp(mac_id1, "") == 0 )))
			{
				if((mac_id1 && strcmp(mac_id1, "") != 0) && (old_mac_id2 && strcmp(old_mac_id2, "") != 0)  
						&& strcmp(mac_id1, old_mac_id2) == 0){ //This is for avoiding updation error due to data
					PIN_FLIST_ELEM_COPY(inherited_flistp, PIN_FLD_ALIAS_LIST, 101, inheritedinfo_flist, PIN_FLD_ALIAS_LIST, 102, ebufp);
				}
				else{
					PIN_FLIST_ELEM_COPY(inherited_flistp, PIN_FLD_ALIAS_LIST, 101, inheritedinfo_flist, PIN_FLD_ALIAS_LIST, 101, ebufp);
				}
			}
		}

                args_flistp = PIN_FLIST_ELEM_GET(out_flist, PIN_FLD_DATA_ARRAY, 1, 1, ebufp );
                inheritedinfo_flist = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_SERVICES, elem_id, ebufp );
                PIN_FLIST_FLD_SET(inheritedinfo_flist, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
		if (!mac_id1 && !mac_id2){
			PIN_FLIST_FLD_COPY(services_flist, PIN_FLD_LOGIN, inheritedinfo_flist, PIN_FLD_LOGIN, ebufp);
			PIN_FLIST_FLD_COPY(services_flist, PIN_FLD_PASSWD_CLEAR, inheritedinfo_flist, PIN_FLD_PASSWD, ebufp);

			PIN_FLIST_ELEM_SET(inheritedinfo_flist, bb_flistp, MSO_FLD_BB_INFO, 0, ebufp );
			PIN_FLIST_FLD_COPY(services_flist, PIN_FLD_STATUS, inheritedinfo_flist, PIN_FLD_STATUS, ebufp);
		}
		else{
			if((mac_id2 && strcmp(mac_id2, "") != 0) || ((old_mac_id2 && strcmp(old_mac_id2, "") != 0)
                                        && (!mac_id2 || strcmp(mac_id2, "") == 0 )))

			{
				if((mac_id2 && strcmp(mac_id2, "") != 0) && (old_mac_id1 && strcmp(old_mac_id1, "") != 0)
						&& strcmp(mac_id2, old_mac_id1) == 0){
					PIN_FLIST_ELEM_COPY(inherited_flistp, PIN_FLD_ALIAS_LIST, 102, inheritedinfo_flist, PIN_FLD_ALIAS_LIST, 101, ebufp);
				}
				else{
					PIN_FLIST_ELEM_COPY(inherited_flistp, PIN_FLD_ALIAS_LIST, 102, inheritedinfo_flist, PIN_FLD_ALIAS_LIST, 102, ebufp);
				}
			}
			
		}

                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_service prepared output flist", out_flist);

                updservices_flist = PIN_FLIST_ELEM_ADD(updacct_inflistp, PIN_FLD_SERVICES, elem_id, ebufp);
                PIN_FLIST_FLD_COPY(services_flist, PIN_FLD_SERVICE_OBJ, updservices_flist, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_COPY(services_flist, PIN_FLD_BAL_GRP_OBJ, updservices_flist, PIN_FLD_BAL_GRP_OBJ, ebufp);
                PIN_FLIST_FLD_COPY(services_flist, PIN_FLD_LOGIN, updservices_flist, PIN_FLD_LOGIN, ebufp);
                PIN_FLIST_FLD_COPY(services_flist, PIN_FLD_PASSWD_CLEAR, updservices_flist, PIN_FLD_PASSWD_CLEAR, ebufp);
                PIN_FLIST_FLD_COPY(services_flist, PIN_FLD_PASSWD, updservices_flist, PIN_FLD_PASSWD, ebufp);

                new_flistp = PIN_FLIST_SUBSTR_ADD(updservices_flist, PIN_FLD_INHERITED_INFO, ebufp);

		if (inherited_flistp)
		{
			new_bb_flistp = PIN_FLIST_SUBSTR_GET(inherited_flistp, MSO_FLD_BB_INFO, 1, ebufp );
			if (new_bb_flistp)
			{
				PIN_FLIST_SUBSTR_SET(new_flistp, new_bb_flistp, MSO_FLD_BB_INFO, ebufp);
			}
			//if(mac_id1 && strcmp(mac_id1, "") != 0 )
			while ((a_flistp = PIN_FLIST_ELEM_GET_NEXT (inherited_flistp,
                                                PIN_FLD_ALIAS_LIST, &aelem_id, 1,&acookie, ebufp)) != NULL )
			{
				if ((!mac_id1 || strcmp(mac_id1, "") == 0) && (old_mac_id1 && strcmp(old_mac_id1, "") != 0) && aelem_id == 101)
				{
					PIN_FLIST_ELEM_SET(new_flistp, NULL, PIN_FLD_ALIAS_LIST, 101, ebufp);
				}
				else if( mac_id1 && strcmp(mac_id1, "") != 0 ) 
				{
					if((mac_id1 && strcmp(mac_id1, "") != 0) && (old_mac_id2 && strcmp(old_mac_id2, "") != 0)  
							&& strcmp(mac_id1, old_mac_id2) == 0){ //This is to align with correct elem id 
						PIN_FLIST_ELEM_COPY(inherited_flistp, PIN_FLD_ALIAS_LIST, 101, new_flistp, PIN_FLD_ALIAS_LIST, 102, ebufp);
					}
					else{
						PIN_FLIST_ELEM_COPY(inherited_flistp, PIN_FLD_ALIAS_LIST, 101, new_flistp, PIN_FLD_ALIAS_LIST, 101, ebufp);
					}
				}
				if ((!mac_id2 || strcmp(mac_id2, "") == 0) && ((old_mac_id2 && strcmp(old_mac_id2, "") != 0)) && aelem_id == 102)
				{
					PIN_FLIST_ELEM_SET(new_flistp, NULL, PIN_FLD_ALIAS_LIST, 102, ebufp);
				}
				else if(mac_id2 && strcmp(mac_id2, "") != 0 )
				{
					if((mac_id2 && strcmp(mac_id2, "") != 0) && (old_mac_id1 && strcmp(old_mac_id1, "") != 0)
							&& strcmp(mac_id2, old_mac_id1) == 0){
						PIN_FLIST_ELEM_COPY(inherited_flistp, PIN_FLD_ALIAS_LIST, 102, new_flistp, PIN_FLD_ALIAS_LIST, 101, ebufp);
					}
					else{
						PIN_FLIST_ELEM_COPY(inherited_flistp, PIN_FLD_ALIAS_LIST, 102, new_flistp, PIN_FLD_ALIAS_LIST, 102, ebufp);
					}
				}
			}
		}

                if (inheritedinfo_flist)
                {
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_service input list", updacct_inflistp);
                        PCM_OP(ctxp, PCM_OP_CUST_UPDATE_SERVICES, 0, updacct_inflistp, &updacct_outflistp, ebufp);
                        if (PIN_ERRBUF_IS_ERR(ebufp))
                        {
                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_CUST_UPDATE_SERVICES", ebufp);
                                PIN_ERRBUF_RESET(ebufp);
                                PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);
                                return 0;
                        }
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_service output flist", updacct_outflistp);
                        PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);
                }
                PIN_FLIST_DESTROY_EX(&updacct_inflistp, NULL);

      		/* commenting for now 
	          *********************************** status change ********************************
                status = PIN_FLIST_FLD_GET(services_flist, PIN_FLD_STATUS, 1, ebufp );
                if (status)
                {
                        updacct_inflistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, updacct_inflistp, PIN_FLD_POID, ebufp);
                        PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_USERID, updacct_inflistp, PIN_FLD_USERID, ebufp);
                        PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, updacct_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);
                        PIN_FLIST_FLD_COPY(services_flist, PIN_FLD_SERVICE_OBJ, updacct_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
                        PIN_FLIST_FLD_COPY(services_flist, PIN_FLD_STATUS, updacct_inflistp, PIN_FLD_STATUS, ebufp);
                        PIN_FLIST_FLD_SET(updacct_inflistp, PIN_FLD_DESCR, "Service status change from Modify customer opcode", ebufp );

                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_service Modify Service status", updacct_inflistp);

                        if (*(int*)status == PIN_STATUS_ACTIVE)
                        {
                                PCM_OP(ctxp, MSO_OP_CUST_REACTIVATE_SERVICE, 0, updacct_inflistp, &updacct_outflistp, ebufp);
                        }
                        else if (*(int*)status == PIN_STATUS_INACTIVE)
                        {
                                PCM_OP(ctxp, MSO_OP_CUST_SUSPEND_SERVICE, 0, updacct_inflistp, &updacct_outflistp, ebufp);
                        }
                        else if (*(int*)status == PIN_STATUS_CLOSED)
                        {
                                PCM_OP(ctxp, MSO_OP_CUST_TERMINATE_SERVICE, 0, updacct_inflistp, &updacct_outflistp, ebufp);
                        }

                        PIN_FLIST_DESTROY_EX(&updacct_inflistp, NULL);
                        if (PIN_ERRBUF_IS_ERR(ebufp))
                        {
                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_CUST_MODIFY_SERVICE_STATUS", ebufp);
                                PIN_ERRBUF_RESET(ebufp);
        			PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
                                PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);
                                return 0;
                        }
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_service Modify Service status", updacct_outflistp);
                        PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);
                }
      		* commenting for now ***/ 

                if ( login )
                {
                        if ( old_login && (*(int32 *)old_login != *(int32 *)login ) )
                        {
	                        status_flags = DOC_BB_CHANGE_NAME;
                                provaction_inflistp = PIN_FLIST_CREATE(ebufp);
                                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);
                                PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
                                PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);
				PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_LOGIN, old_login, ebufp);
				fm_mso_cust_get_bp_obj(ctxp, account_obj, service_obj,-1, &bp_obj, &mso_obj, ebufp);
				if ( mso_obj )
					PIN_FLIST_FLD_SET(provaction_inflistp, MSO_FLD_PURCHASED_PLAN_OBJ, mso_obj, ebufp);

                                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_service provisioning input list", provaction_inflistp);
                                PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
                                PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
                                if (PIN_ERRBUF_IS_ERR(ebufp))
                                {
                                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_CREATE_ACTION in change login", ebufp);
                                        PIN_ERRBUF_RESET(ebufp);
        				PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
                                        PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
                                        return 0;
                                }
                                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_service provisioning output flist", provaction_outflistp);
                                PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
                        }
                        else
                        {
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
						"fm_mso_update_bb_service there is no need of provisoning as both usernames are same");
                        }
                }
                if ( passwd )
                {
                        if ( old_passwd && strcmp(old_passwd,passwd) != 0 )
                        {
	                        status_flags = DOC_BB_CHANGE_PASS;
                                provaction_inflistp = PIN_FLIST_CREATE(ebufp);
                                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);
                                PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
                                PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);
				PIN_FLIST_FLD_SET(provaction_inflistp, MSO_FLD_PASSWORD, old_passwd, ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, provaction_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);
				fm_mso_cust_get_bp_obj(ctxp, account_obj, service_obj, -1, &bp_obj, &mso_obj, ebufp);
				if ( mso_obj )
					PIN_FLIST_FLD_SET(provaction_inflistp, MSO_FLD_PURCHASED_PLAN_OBJ, mso_obj, ebufp);

                                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_service provisioning input list", provaction_inflistp);
                                PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, provaction_inflistp, &provaction_outflistp, ebufp);

                                if (PIN_ERRBUF_IS_ERR(ebufp))
                                {
                                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_CREATE_ACTION ", ebufp);
                                        PIN_ERRBUF_RESET(ebufp);
        				PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
                                	PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);

					if(out_flist == NULL){
						out_flist = PIN_FLIST_CREATE(ebufp);
					}
					PIN_FLIST_ELEM_SET(out_flist,provaction_outflistp,PIN_FLD_RESULTS_DATA,0,ebufp);
                                        PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
					if(!PIN_POID_IS_NULL(mso_obj)) PIN_POID_DESTROY(mso_obj,ebufp);
					if(!PIN_POID_IS_NULL(bp_obj)) PIN_POID_DESTROY(bp_obj,ebufp);
		
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,"Error in change password in",in_flist);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,"Error in change password out",out_flist);
					
                                        return 0;
                                }
				if(provaction_outflistp)
				{
					prov_call = PIN_FLIST_FLD_GET(provaction_outflistp, PIN_FLD_STATUS, 1, ebufp);
					if(prov_call && (*prov_call == 1))
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Prov error MSO_OP_PROV_CREATE_ACTION- istal ");
						PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
						PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);

						if(out_flist == NULL){
							out_flist = PIN_FLIST_CREATE(ebufp);
						}
						PIN_FLIST_ELEM_SET(out_flist,provaction_outflistp,PIN_FLD_RESULTS_DATA,0,ebufp);
						PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
						if(!PIN_POID_IS_NULL(mso_obj)) PIN_POID_DESTROY(mso_obj,ebufp);
                                                if(!PIN_POID_IS_NULL(bp_obj)) PIN_POID_DESTROY(bp_obj,ebufp);
					
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,"Error in change password in",in_flist);
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,"Error in change password out",out_flist);
                                                return 0;
					}
				}

                                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_service provisioning output flist", provaction_outflistp);
                                PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
				/* Update the service provisioning status */
				ret_val = -1;
				ret_val = fm_mso_update_ser_prov_status(ctxp, provaction_inflistp, MSO_PROV_IN_PROGRESS, ebufp);
				if(ret_val!=0)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in setting Service Prov Status ");
					PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
					PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
					PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
					return 0;
				}
				/* Update MSO purchased plan status */
				ret_val = -1;
				ret_val = fm_mso_update_mso_purplan_status(ctxp, mso_obj, MSO_PROV_IN_PROGRESS, ebufp);
				if(ret_val!=1)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in updating purchased plan status ");
					PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
					PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
					PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
					return 0;
				}
				/************* Call notification ****************/					
				status_flags = NTF_BB_CHANGE_PASSWORD;
				PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);
				/*Pawan:16-02-2015 Added below to send new password for notification */
				PIN_FLIST_FLD_SET(provaction_inflistp, MSO_FLD_PASSWORD, passwd, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Notification input list", provaction_inflistp);
				PCM_OP(ctxp, MSO_OP_NTF_CREATE_NOTIFICATION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
				PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
				PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
			}
			else
                        {
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_service there is no need of provisoning as password is same");
                        }
                }
                if ((istalf && old_istalf && (*(int32 *)istalf != *(int32 *)old_istalf)) ||
                        (old_role && new_role && strcmp(old_role, new_role) != 0))
                {
                    prov_flag = 0;
                            /* ISTAL_FLAG has changed from 0 to 1 */
                     if ((istalf && *(int32 *)istalf == 1) || (new_role && strstr(new_role, "-Static")))

				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"@@BB-Moving from isTal False to isTal True");

					static_in_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_ACCOUNT_OBJ, static_in_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
					PIN_FLIST_FLD_SET(static_in_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, static_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Static device input flist is :",static_in_flistp);
					istal_result = fm_mso_purchase_static_ip(ctxp, static_in_flistp, MSO_ADD_DEVICE, ebufp );
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Static device return flist is :",static_in_flistp);
        	                        if ( istal_result == 1 ||  istal_result == 2)
                	                {
                        	                return 10;
                                	}
	                                else if ( istal_result == 0 )
        	                        {
                	                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Static IP device associated successfully");
                        	        }
					PIN_FLIST_DESTROY_EX(&static_in_flistp, NULL);
				
					//status_flags = DOC_BB_CAM_TI;
					/* Pawan:01-01-2015: Added Scenario: IS1CLICK to ISTAL
						OLD VALUES: Is1Click = 1 and ISTAL = 0 
						NEW VALUES: Is1Click = 0 and ISTAL = 1 */
					if( is1Click && old_is1Click && *(int32 *)old_is1Click ==1 && *(int32 *)is1Click == 0)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"@@BB-Moving from isClick True to isClick False");
						status_flags = DOC_BB_CAM_IT;
						prov_flag = 1;
					}
							
					/* Pawan:01-01-2015: Added Scenario: PORTAL to ISTAL
						OLD VALUES: Is1Click = 0 and ISTAL = 0 --> PORTAL
						NEW VALUES: Is1Click = 0 and ISTAL = 1 */
					if( is1Click && old_is1Click && *(int32 *)old_is1Click ==0 && *(int32 *)is1Click == 0)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"@@BB-Moving from isClick False to isClick False");
						status_flags = DOC_BB_CAM_PT;
						prov_flag = 1;
					}											
				
					/* Muthu :19-10-2015: Added NoClick Case
						OLD VALUES: Is1Click = 2 and ISTAL = 0 
						NEW VALUES: Is1Click = 0 and ISTAL = 1 */
					if( is1Click && old_is1Click && *(int32 *)old_is1Click ==2 && *(int32 *)is1Click == 0)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"@@BB-Moving from isClick noclick to isClick False");
						status_flags = DOC_BB_CAM_NT;
						prov_flag = 1;
					}											
				 
                 if (new_role && strstr(new_role, "-Static"))
                    {
                        status_flags = DOC_BB_CAM_NT;
                        prov_flag = 1;
                    }
                }
				/* ISTAL_FLAG has changed from 1 to 0 */
			    else if (( istalf && *(int32 *)istalf == 0) || (new_role && strstr(new_role, "-Dynamic")))
            	{	
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"@@BB-Moving from isTal True to isTal False");

					/* Pawan:02-01-2015: Added Scenario: ISTAL to PORTAL
						OLD VALUES: Is1Click = 0 and ISTAL = 1 
						NEW VALUES: Is1Click = 0 and ISTAL = 0 --> PORTAL */
					if( is1Click && old_is1Click && *(int32 *)old_is1Click ==0 && *(int32 *)is1Click == 0)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"@@BB-Moving from isClick False to isClick False");
						status_flags = DOC_BB_CAM_TP;
						prov_flag = 1;
						dev_dis = 1;
					}
				
					/* Muthu: 19-10-2015: Added Scenario: ISTAL to PORTAL
						OLD VALUES: Is1Click = 0 and ISTAL = 1 
						NEW VALUES: Is1Click = 1 and ISTAL = 0 */
					if( is1Click && old_is1Click && *(int32 *)old_is1Click ==0 && *(int32 *)is1Click == 1)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"@@BB-Moving from isClick False to isClick True");
						status_flags = DOC_BB_CAM_TI;
						prov_flag = 1;
						dev_dis = 1;
					}

					/* Muthu: 19-10-2015: Added Scenario: ISTAL to NOCLICK
						OLD VALUES: Is1Click = 0 and ISTAL = 1 
						NEW VALUES: Is1Click = 2 and ISTAL = 0  */
					if( is1Click && old_is1Click && *(int32 *)old_is1Click ==0 && *(int32 *)is1Click == 2)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"@@BB-Moving from isClick False to isClick noclick");
						status_flags = DOC_BB_CAM_TN;
						prov_flag = 1;
						dev_dis = 1;
					}
                    if (new_role && strstr(new_role, "-Dynamic"))
                    {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"@@BB-Moving from Static to Dynamic");
                        status_flags = DOC_BB_CAM_TI;
                        prov_flag = 1;
                        dev_dis = 1;
                    }

				}
				if(prov_flag)
				{
					provaction_inflistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);
					PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
					PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, provaction_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);
					fm_mso_cust_get_bp_obj(ctxp, account_obj, service_obj,-1, &bp_obj, &mso_obj, ebufp);
					if ( mso_obj )
						PIN_FLIST_FLD_SET(provaction_inflistp, MSO_FLD_PURCHASED_PLAN_OBJ, mso_obj, ebufp);

					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_service provisioning input list",
														 provaction_inflistp);
					PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
					//PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
					if (PIN_ERRBUF_IS_ERR(ebufp))
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in calling MSO_OP_PROV_CREATE_ACTION-istal ", ebufp);
						PIN_ERRBUF_RESET(ebufp);
						PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
						PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);

						if(out_flist == NULL){
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Reseting the out flist");
							out_flist = PIN_FLIST_CREATE(ebufp);
						}
						PIN_FLIST_ELEM_SET(out_flist,provaction_outflistp,PIN_FLD_RESULTS_DATA,0,ebufp);
						PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
						if(!PIN_POID_IS_NULL(mso_obj)) PIN_POID_DESTROY(mso_obj,ebufp);
						if(!PIN_POID_IS_NULL(bp_obj)) PIN_POID_DESTROY(bp_obj,ebufp);
						return 0;
					}
					if(provaction_outflistp)
					{
						prov_call = PIN_FLIST_FLD_GET(provaction_outflistp, PIN_FLD_STATUS, 1, ebufp);
						if(prov_call && (*prov_call == 1))
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Prov error MSO_OP_PROV_CREATE_ACTION- istal ");
							PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
							PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);

							if(out_flist == NULL){
								out_flist = PIN_FLIST_CREATE(ebufp);
							}
							PIN_FLIST_ELEM_SET(out_flist,provaction_outflistp,PIN_FLD_RESULTS_DATA,0,ebufp);
							PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
							if(!PIN_POID_IS_NULL(mso_obj)) PIN_POID_DESTROY(mso_obj,ebufp);
							if(!PIN_POID_IS_NULL(bp_obj)) PIN_POID_DESTROY(bp_obj,ebufp);
							return 0;
						}
					}										
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_service provisioning"
											"output flist", provaction_outflistp);
					PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
					/* Update the service provisioning status */
					ret_val = -1;
					ret_val = fm_mso_update_ser_prov_status(ctxp, provaction_inflistp, MSO_PROV_IN_PROGRESS, ebufp);
					if(ret_val!=0)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in setting Service Prov Status ");
						PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
						PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
						return 0;
					}
					/* Update MSO purchased plan status */
					ret_val = -1;
					ret_val = fm_mso_update_mso_purplan_status(ctxp, mso_obj, MSO_PROV_IN_PROGRESS, ebufp);
					if(ret_val!=1)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in updating purchased plan status ");
						PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
						PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
						return 0;
					}					
				}
				if ( dev_dis == 1 )
				{
					strcpy(param,"/device/ip/static");
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "calling Static IP device search");
					fm_mso_search_devices(ctxp,param,service_obj,&deviceflistp, error_codep, ebufp);
					if (PIN_ERRBUF_IS_ERR(ebufp))
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in static ip device search", ebufp);
						PIN_ERRBUF_RESET(ebufp);
						PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
						PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
						return 15;
					}
					cookie = NULL;
					rec_id = 0;
					while ( (temp_flistp = PIN_FLIST_ELEM_GET_NEXT (deviceflistp, 
						PIN_FLD_RESULTS, &rec_id, 1,&cookie, ebufp)) != (pin_flist_t *)NULL )
					{
						device_flistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_COPY(temp_flistp, PIN_FLD_POID, device_flistp, PIN_FLD_POID, ebufp);
						PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, device_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
						PIN_FLIST_FLD_SET(device_flistp, PIN_FLD_FLAGS, &disas_flag, ebufp);
						arg_flistp = PIN_FLIST_ELEM_ADD(device_flistp, PIN_FLD_SERVICES, 0,ebufp);
						PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACCOUNT_OBJ, (void *) account_obj, ebufp);
						PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_SERVICE_OBJ, (void *) service_obj, ebufp);
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
										"Static IP device disassociate input flist ", device_flistp);
						PCM_OP(ctxp, PCM_OP_DEVICE_ASSOCIATE, 0, device_flistp, &device_out_flistp, ebufp);
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
										"Static IP device disassociate input flist output flist", device_out_flistp);
						if (PIN_ERRBUF_IS_ERR(ebufp))
						{
							PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in static ip device disassociate", ebufp);
							PIN_ERRBUF_RESET(ebufp);
							PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
							PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
							PIN_FLIST_DESTROY_EX(&device_flistp, NULL);
							PIN_FLIST_DESTROY_EX(&device_out_flistp, NULL);
							return 15;
						}
						PIN_FLIST_DESTROY_EX(&device_flistp, NULL);
						PIN_FLIST_DESTROY_EX(&device_out_flistp, NULL);
					
						tmp_flistp = PIN_FLIST_ELEM_GET(temp_flistp, PIN_FLD_PLAN, 0, 1,ebufp);
						if ( tmp_flistp )
						{
							del_flistp = PIN_FLIST_CREATE(ebufp);
							PIN_FLIST_FLD_COPY(temp_flistp, PIN_FLD_POID, del_flistp, PIN_FLD_POID, ebufp);
							PIN_FLIST_ELEM_COPY(temp_flistp, PIN_FLD_PLAN, 0, del_flistp, PIN_FLD_PLAN, 0, ebufp);
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
											"delete plan input flist ", del_flistp);
							PCM_OP(ctxp, PCM_OP_DELETE_FLDS, 0, del_flistp, &del_out_flistp, ebufp);
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
											"delete plan output flist", del_out_flistp);
							if (PIN_ERRBUF_IS_ERR(ebufp))
							{
								PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in deleting plan from device", ebufp);
								PIN_ERRBUF_RESET(ebufp);
								PIN_FLIST_DESTROY_EX(&del_flistp, NULL);
								PIN_FLIST_DESTROY_EX(&del_out_flistp, NULL);
								return 15;
							}
						}
					}

				}

				// Kunal - create LC event for change of talclass

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"@@MM-Going for LC Create",out_flist);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"@@MM-Going for LC Create");
				fm_mso_create_lifecycle_evt(ctxp,out_flist,in_flist, ID_MODIFY_BB_SERVICE_AUTHENTICATION, ebufp);

			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"fm_mso_update_bb_service thr is no need of provisoning as istal flag is same");
			}
		/*Pawan:02-01-2015: Added below block to trigger provisioning on change of is1click */
		if ( is1Click )
                {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"@@BB-Is Click Modify cases");
			prov_flag = 0;
                        if ( old_is1Click && (*(int32 *)is1Click != *(int32 *)old_is1Click ) )
                        {		

				/* IS1CLICK_FLAG has changed from 0 to 1 */
                                if ( *(int32 *)is1Click == 1 )
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"@@BB-Is Click Modify cases");
					/* ISTAL to IS1CLICK
					OLD VALUES: Is1Click = 0 and ISTAL = 1 
					NEW VALUES: Is1Click = 1 and ISTAL = 0 */
					/* Commented as its handled in the TAL Class itself

					if( istalf && old_istalf && *(int32 *)old_istalf ==1 && *(int32 *)istalf == 0 
								&& old_is1Click && *(int32 *)old_is1Click == 0)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"@@BB-Move from Tal to Non Tal - to isClick True");
						status_flags = DOC_BB_CAM_TI;
						prov_flag = 1;
						dev_dis = 1;
					}											
					*/

					/* PORTAL to IS1CLICK
					OLD VALUES: Is1Click = 0 and ISTAL = 0 --> PORTAL
					NEW VALUES: Is1Click = 1 and ISTAL = 0 */
					if( istalf && old_istalf && *(int32 *)old_istalf ==0 && *(int32 *)istalf == 0 
							&& old_is1Click && *(int32 *)old_is1Click == 0)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"@@BB-Move to isClick True from old isClick False");
						status_flags = DOC_BB_CAM_PI;
						prov_flag = 1;
					}										
                                        /* Kunal NOCLICK to Is1CLICK
                                        OLD VALUES: Is1Click = 2 and ISTAL = 0
                                        NEW VALUES: Is1Click = 1 and ISTAL = 0 */
                                        if( istalf && old_istalf && *(int32 *)old_istalf ==0 && *(int32 *)istalf == 0 
							&& old_is1Click && *(int32 *)old_is1Click == 2)
                                        {
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"@@BB-Move to isClick True from old isClick noClick");
                                                status_flags = DOC_BB_CAM_NI;
                                                prov_flag = 1;
                                        }

				}
				/* IS1CLICK_FLAG has changed from 1 to 0 */
				else if ( *(int32 *)is1Click == 0 ) 
				{
					/* Pawan:02-01-2015: Added Scenario: IS1CLICK to PORTAL
					OLD VALUES: Is1Click = 1 and ISTAL = 0 
					NEW VALUES: Is1Click = 0 and ISTAL = 0 --> PORTAL */
					if( istalf && old_istalf && *(int32 *)old_istalf ==0 && *(int32 *)istalf == 0 
							&& old_is1Click && *(int32 *)old_is1Click == 1)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"@@BB-Move to isClick False from old isClick True");
						status_flags = DOC_BB_CAM_IP;
						prov_flag = 1;
					}
                                        /* Kunal: Added Scenario: NOCLICK to PORTAL
                                        OLD VALUES: Is1Click = 2 and ISTAL = 0
                                        NEW VALUES: Is1Click = 0 and ISTAL = 0 --> PORTAL */
                                        if( istalf && old_istalf && *(int32 *)old_istalf ==0 && *(int32 *)istalf == 0 
							&& old_is1Click && *(int32 *)old_is1Click == 2)
                                        {
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"@@BB-Move to isClick False from old isClick noClick");
                                                status_flags = DOC_BB_CAM_NP;
                                                prov_flag = 1;
                                        }
						
				}
				// Kunal Added to handle Noclick change 
                                /* IS1CLICK_FLAG has changed from 1 to 0 */
                                else if ( *(int32 *)is1Click == 2 )
                                {
                                        /* Kunal: Added Scenario:  PORTAL to NOCLICK
                                        OLD VALUES: Is1Click = 0 and ISTAL = 0 --> PORTAL
                                        NEW VALUES: Is1Click = 2 and ISTAL = 0 */
                                        if( istalf && old_istalf && *(int32 *)old_istalf ==0 && *(int32 *)istalf == 0 
							&& old_is1Click && *(int32 *)old_is1Click == 0)
                                        {
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"@@BB-Move to isClick Noclick from old false");
                                                status_flags = DOC_BB_CAM_PN;
                                                prov_flag = 1;
                                        }
                                        /* Kunal: Added Scenario: Is1Click  to NOCLICK
                                        OLD VALUES: Is1Click = 1 and ISTAL = 0 
                                        NEW VALUES: Is1Click = 2 and ISTAL = 0 */
                                        if( istalf && old_istalf && *(int32 *)old_istalf ==0 && *(int32 *)istalf == 0 
							&& old_is1Click && *(int32 *)old_is1Click == 1)
                                        {
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"@@BB-Move to isClick Noclick from old True");
                                                status_flags = DOC_BB_CAM_IN;
                                                prov_flag = 1;
                                        }
                                }
				if(prov_flag)
				{
					provaction_inflistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, provaction_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);
					PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
					PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);
					fm_mso_cust_get_bp_obj(ctxp, account_obj, service_obj,-1, &bp_obj, &mso_obj, ebufp);
					if ( mso_obj )
						PIN_FLIST_FLD_SET(provaction_inflistp, MSO_FLD_PURCHASED_PLAN_OBJ, mso_obj, ebufp);

					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_update_bb_service provisioning inp flist",provaction_inflistp);
					PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
                                        if (PIN_ERRBUF_IS_ERR(ebufp))
                                        {
                                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in calling MSO_OP_PROV_CREATE_ACTION-isclick", ebufp);
                                                PIN_ERRBUF_RESET(ebufp);
                                                PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
                                                PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);

                                                if(out_flist == NULL){
                                                        out_flist = PIN_FLIST_CREATE(ebufp);
                                                }
                                                PIN_FLIST_ELEM_SET(out_flist,provaction_outflistp,PIN_FLD_RESULTS_DATA,0,ebufp);
                                                PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
                                                if(!PIN_POID_IS_NULL(mso_obj)) PIN_POID_DESTROY(mso_obj,ebufp);
                                                if(!PIN_POID_IS_NULL(bp_obj)) PIN_POID_DESTROY(bp_obj,ebufp);
                                                return 0;
                                        }
                                        if(provaction_outflistp)
                                        {
                                                prov_call = PIN_FLIST_FLD_GET(provaction_outflistp, PIN_FLD_STATUS, 1, ebufp);
                                                if(prov_call && (*prov_call == 1))
                                                {
                                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Prov error MSO_OP_PROV_CREATE_ACTION- isclick");
                                                        PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
                                                        PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);

                                                        if(out_flist == NULL){
                                                                out_flist = PIN_FLIST_CREATE(ebufp);
                                                        }
                                                        PIN_FLIST_ELEM_SET(out_flist,provaction_outflistp,PIN_FLD_RESULTS_DATA,0,ebufp);
                                                        PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
                                                        if(!PIN_POID_IS_NULL(mso_obj)) PIN_POID_DESTROY(mso_obj,ebufp);
                                                        if(!PIN_POID_IS_NULL(bp_obj)) PIN_POID_DESTROY(bp_obj,ebufp);
                                                        return 0;
                                                }
                                        }

					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_update_bb_service provisiong out flist",provaction_outflistp);
					PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
							
					/* Update the service provisioning status */
					ret_val = -1;
					ret_val = fm_mso_update_ser_prov_status(ctxp, provaction_inflistp, MSO_PROV_IN_PROGRESS, ebufp);
					if(ret_val!=0)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in setting Service Prov Status ");
						PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
						PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
						return 0;
					}
					/* Update MSO purchased plan status */
					ret_val = -1;
					ret_val = fm_mso_update_mso_purplan_status(ctxp, mso_obj, MSO_PROV_IN_PROGRESS, ebufp);
					if(ret_val!=1)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in updating purchased plan status ");
						PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
						PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
						return 0;
					}
				}

				if ( dev_dis == 1 )
				{
					strcpy(param,"/device/ip/static");
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "calling Static IP device search");
					fm_mso_search_devices(ctxp,param,service_obj,&deviceflistp, error_codep, ebufp);
					if (PIN_ERRBUF_IS_ERR(ebufp))
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in static ip device search", ebufp);
						PIN_ERRBUF_RESET(ebufp);
						PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
						PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
						return 15;
					}
					cookie1 = NULL;
					rec_id = 0;
					while ( (temp_flistp = PIN_FLIST_ELEM_GET_NEXT (deviceflistp, 
							PIN_FLD_RESULTS, &rec_id, 1,&cookie1, ebufp)) != (pin_flist_t *)NULL )
					{
						device_flistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_COPY(temp_flistp, PIN_FLD_POID, device_flistp, PIN_FLD_POID, ebufp);
						PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, device_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
						PIN_FLIST_FLD_SET(device_flistp, PIN_FLD_FLAGS, &disas_flag, ebufp);
						arg_flistp = PIN_FLIST_ELEM_ADD(device_flistp, PIN_FLD_SERVICES, 0,ebufp);
						PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACCOUNT_OBJ, (void *) account_obj, ebufp);
						PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_SERVICE_OBJ, (void *) service_obj, ebufp);
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
									"Static IP device disassociate input flist ", device_flistp);
						PCM_OP(ctxp, PCM_OP_DEVICE_ASSOCIATE, 0, device_flistp, &device_out_flistp, ebufp);
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
									"Static IP device disassociate input flist output flist", device_out_flistp);
						if (PIN_ERRBUF_IS_ERR(ebufp))
						{
							PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in static ip device disassociate", ebufp);
							PIN_ERRBUF_RESET(ebufp);
							PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
							PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
							PIN_FLIST_DESTROY_EX(&device_flistp, NULL);
							PIN_FLIST_DESTROY_EX(&device_out_flistp, NULL);
							return 15;
						}
						PIN_FLIST_DESTROY_EX(&device_flistp, NULL);
						PIN_FLIST_DESTROY_EX(&device_out_flistp, NULL);
				
						tmp_flistp = PIN_FLIST_ELEM_GET(temp_flistp, PIN_FLD_PLAN, 0, 1,ebufp);
						if ( tmp_flistp )
						{
							del_flistp = PIN_FLIST_CREATE(ebufp);
							PIN_FLIST_FLD_COPY(temp_flistp, PIN_FLD_POID, del_flistp, PIN_FLD_POID, ebufp);
							PIN_FLIST_ELEM_COPY(temp_flistp, PIN_FLD_PLAN, 0, del_flistp, PIN_FLD_PLAN, 0, ebufp);
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
										"delete plan input flist ", del_flistp);
							PCM_OP(ctxp, PCM_OP_DELETE_FLDS, 0, del_flistp, &del_out_flistp, ebufp);
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
										"delete plan output flist", del_out_flistp);
							if (PIN_ERRBUF_IS_ERR(ebufp))
							{
								PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in deleting plan from device", ebufp);
								PIN_ERRBUF_RESET(ebufp);
								PIN_FLIST_DESTROY_EX(&del_flistp, NULL);
								PIN_FLIST_DESTROY_EX(&del_out_flistp, NULL);
								return 15;
							}
						}
					}

				}
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"@@MM-Going for LC Create");
				fm_mso_create_lifecycle_evt(ctxp,out_flist,in_flist, ID_MODIFY_BB_SERVICE_AUTHENTICATION, ebufp);
                        }
                        else
                        {
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
					 "fm_mso_update_bb_service there is no need of provisoning as is1click flag is same");
                        }
                }
				
                if ( ne && strcmp(ne,"") != 0 )
                {
                        if ( old_ne && strcmp(ne,old_ne ) != 0 )
                        {
				/* Pawan:05-01-2015: Added below condition to set different
					 prov action for CMTS change in case of TAL and NON-TAL */
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"CMTS Change scenario");
				ret_val=0;
				fm_mso_cust_get_bp_obj(ctxp, account_obj, service_obj,-1, &bp_obj, &mso_obj, ebufp);
				if (!PIN_POID_IS_NULL(mso_obj))
				{
					mso_plan_status = MSO_PROV_ACTIVE;
				} 
				else
				{	
				/***** Pavan Bellala 21-08-2015 ****************************************
				 mso_purchased_plan object is NULL, hence not active. Fetch other status
				 as they are valid for CMTS change scenario
				************************************************************************/
                                  /** Call with mso_pur_plan status as Deactive **/
                                	fm_mso_cust_get_bp_obj(ctxp, account_obj, service_obj, MSO_PROV_DEACTIVE,&bp_obj,&mso_obj, ebufp);
					mso_plan_status = MSO_PROV_DEACTIVE;
					if(PIN_POID_IS_NULL(mso_obj)){
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"mso_pur_plan is not deactive");
						/** Call with mso_pur_plan status as Suspend ***/
						mso_plan_status = MSO_PROV_SUSPEND;
						fm_mso_cust_get_bp_obj(ctxp, account_obj, service_obj,MSO_PROV_SUSPEND,
												&bp_obj,&mso_obj, ebufp);
						if(PIN_POID_IS_NULL(mso_obj)){
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"mso_pur_plan is not suspend");
							/** Call with mso_pur_plan status as hold ***/
							mso_plan_status = MSO_PROV_HOLD;
							fm_mso_cust_get_bp_obj(ctxp, account_obj, service_obj,MSO_PROV_HOLD,
												&bp_obj,&mso_obj, ebufp);
							if(PIN_POID_IS_NULL(mso_obj)){
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
									"mso_pur_plan is not valid for CMTS chng");
								PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
								return 0;
							}
						}
					}
				}

				if( old_istalf && *(int32 *)old_istalf ==1)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"CMTS Change scenario,IS_TAL is set.");
					//prog_name = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_PROGRAM_NAME, 0, ebufp);
					ret_val = fm_mso_cust_cmts_ip_change(ctxp, account_obj, service_obj, 
									mso_obj, ne, old_ne, prog_name, &ip_add_list, ebufp);
					if(ret_val==0){
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,"Failure: "
									"in fm_mso_cust_cmts_ip_change:input to modify",in_flist);
						if(ip_add_list) free(ip_add_list);
						if(!PIN_POID_IS_NULL(mso_obj)) PIN_POID_DESTROY(mso_obj,ebufp);
						if(!PIN_POID_IS_NULL(bp_obj)) PIN_POID_DESTROY(bp_obj,ebufp);  

						return 0;
					}
					status_flags = DOC_BB_CMTS_CHANGE_T;
				}
				else
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"CMTS Change scenario,IS_TAL is not set.");
					status_flags = DOC_BB_CMTS_CHANGE_NT;
				}
                                provaction_inflistp = PIN_FLIST_CREATE(ebufp);
                                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, provaction_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);
                                PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
                                PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);
				//Set login only for TAL CMTS change
				if(ret_val>0)
					//PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_LOGIN, ip_add_list, ebufp);
					PIN_FLIST_FLD_PUT(provaction_inflistp, PIN_FLD_LOGIN, ip_add_list, ebufp);
				//fm_mso_cust_get_bp_obj(ctxp, account_obj, service_obj,-1, &bp_obj, &mso_obj, ebufp);
				if ( mso_obj )
				{
					PIN_FLIST_FLD_SET(provaction_inflistp, MSO_FLD_PURCHASED_PLAN_OBJ, mso_obj, ebufp);
					PIN_FLIST_FLD_SET(provaction_inflistp,PIN_FLD_STATUS,&mso_plan_status,ebufp);
				} 
				else
				{
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,"fm_mso_update_bb_service : Error before"
									" calling provisioning for CMTS change",provaction_inflistp);
					PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
					if(!PIN_POID_IS_NULL(mso_obj)) PIN_POID_DESTROY(mso_obj,ebufp);
					if(!PIN_POID_IS_NULL(bp_obj)) PIN_POID_DESTROY(bp_obj,ebufp);  

					return 0;
				}
                                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_service provisioning input list", provaction_inflistp);
                                PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
                                //PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in calling MSO_OP_PROV_CREATE_ACTION-ne ", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
					PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);

					if(out_flist == NULL){
						out_flist = PIN_FLIST_CREATE(ebufp);
					}
					PIN_FLIST_ELEM_SET(out_flist,provaction_outflistp,PIN_FLD_RESULTS_DATA,0,ebufp);
					PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
					if(!PIN_POID_IS_NULL(mso_obj)) PIN_POID_DESTROY(mso_obj,ebufp);
					if(!PIN_POID_IS_NULL(bp_obj)) PIN_POID_DESTROY(bp_obj,ebufp);
					return 0;
				}
				if(provaction_outflistp)
				{
					prov_call = PIN_FLIST_FLD_GET(provaction_outflistp, PIN_FLD_STATUS, 1, ebufp);
					if(prov_call && (*prov_call == 1))
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Prov error MSO_OP_PROV_CREATE_ACTION-ne ");
						PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
						PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);

						if(out_flist == NULL){
							out_flist = PIN_FLIST_CREATE(ebufp);
						}
						PIN_FLIST_ELEM_SET(out_flist,provaction_outflistp,PIN_FLD_RESULTS_DATA,0,ebufp);
						PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
                                                if(!PIN_POID_IS_NULL(mso_obj)) PIN_POID_DESTROY(mso_obj,ebufp);
                                                if(!PIN_POID_IS_NULL(bp_obj)) PIN_POID_DESTROY(bp_obj,ebufp);
                                                return 0;
                                        }
                                }
                                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_service provisioning output flist", provaction_outflistp);
                                PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);

				/* Update the service provisioning status */
				ret_val = -1;
				ret_val = fm_mso_update_ser_prov_status(ctxp, provaction_inflistp, MSO_PROV_IN_PROGRESS, ebufp);
				if(ret_val!=0)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in setting Service Prov Status ");
					PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
                                        if(!PIN_POID_IS_NULL(mso_obj)) PIN_POID_DESTROY(mso_obj,ebufp);
                                        if(!PIN_POID_IS_NULL(bp_obj)) PIN_POID_DESTROY(bp_obj,ebufp);
					return 0;
				}
				/* Update MSO purchased plan status */
				ret_val = -1;
				ret_val = fm_mso_update_mso_purplan_status(ctxp, mso_obj, MSO_PROV_IN_PROGRESS, ebufp);
				if(ret_val!=1)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in updating purchased plan status ");
					PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
                                        if(!PIN_POID_IS_NULL(mso_obj)) PIN_POID_DESTROY(mso_obj,ebufp);
                                        if(!PIN_POID_IS_NULL(bp_obj)) PIN_POID_DESTROY(bp_obj,ebufp);
					return 0;
				}
				PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
                        }
                        else
                        {
	                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_service there is no need of provisoning as network element is same");
                        }
                }
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Waiver flag processing");
                if ( waiver_flag )
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Waiver flag present");
                        if ( old_waiver_flag && (*(int32 *)waiver_flag != *(int32 *)old_waiver_flag ) )
                        {               /* WAIVER_FLAG changed from 0 to 1 */
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Lifecycle event create for waiver flag");
                                vp  = (pin_flist_t*)PIN_FLIST_ELEM_GET(out_flist, PIN_FLD_DATA_ARRAY, 0, 1, ebufp );
                                if (vp)
                                {
                                        vp1 = (pin_flist_t*)PIN_FLIST_ELEM_GET(vp, PIN_FLD_SERVICES, 0, 1, ebufp );
                                        if (vp1)
                                        {
                                                PIN_FLIST_FLD_SET(vp1, PIN_FLD_REAUTH_FLAG, 0, ebufp);
                                                if ( *(int32 *)waiver_flag == 1 )
                                                {
                                                        PIN_FLIST_FLD_SET(vp1, PIN_FLD_REAUTH_FLAG, old_waiver_flag, ebufp);
                                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Customer given waiver on Static IP Charge");
                                                }
                                                else
                                                {
                                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Customer removed from waiver of Static IP Charge");
                                                        PIN_FLIST_FLD_SET(vp1, PIN_FLD_REAUTH_FLAG, waiver_flag, ebufp);
                                                }
                                        }
                                }
                                vp  = (pin_flist_t*)PIN_FLIST_ELEM_GET(out_flist, PIN_FLD_DATA_ARRAY, 1, 1, ebufp );
                                if (vp)
                                {
                                        vp1 = (pin_flist_t*)PIN_FLIST_ELEM_GET(vp, PIN_FLD_SERVICES, 0, 1, ebufp );
                                        if (vp1)
                                        {
                                                if ( *(int32 *)waiver_flag == 1 )
                                                {
                                                        PIN_FLIST_FLD_SET(vp1, PIN_FLD_REAUTH_FLAG, waiver_flag, ebufp);
                                                }
                                                else
                                                {
                                                        PIN_FLIST_FLD_SET(vp1, PIN_FLD_REAUTH_FLAG, old_waiver_flag, ebufp);
                                                }
                                        }
                                }
                                fm_mso_create_lifecycle_evt(ctxp,out_flist,in_flist, ID_MODIFY_BB_SERVICE_AUTHENTICATION, ebufp);
                        }
                }
                if ( tech )
                {
                        if ( old_tech && (*(int32 *)old_tech != *(int32 *)tech ) )
                        {
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Technology change not allowed ");
				PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
				return 0;
							// Commented below to not allow technology change. If some tech. change
							// combination are required then add that specific condition.
                          /*      status_flags = DOC_BB_CMTS_CHANGE_T;
                                provaction_inflistp = PIN_FLIST_CREATE(ebufp);
                                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);
                                PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
                                PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);
								fm_mso_cust_get_bp_obj(ctxp, account_obj, service_obj,-1, &bp_obj, &mso_obj, ebufp);
								if ( mso_obj )
									PIN_FLIST_FLD_SET(provaction_inflistp, MSO_FLD_PURCHASED_PLAN_OBJ, mso_obj, ebufp);

                                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_service provisioning input list", provaction_inflistp);
                                PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
                                PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
                                if (PIN_ERRBUF_IS_ERR(ebufp))
                                {
                                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_CREATE_ACTION ", ebufp);
                                        PIN_ERRBUF_RESET(ebufp);
										PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
                                        PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
                                        return 0;
                                }
                                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_service provisioning output flist", provaction_outflistp);
                                PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
							*/
                        }
                        else
                        {
       		                 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"fm_mso_update_bb_service there is no need of provisoning as tech is same");
                        }
                }
		/********************************************************************************************* 
		* This is to update MAC ADDRESS which got from QPS - Two things to be done here:
		* 1. If first time that we are updating, no provisioning is required
		* 2. If this is DELETE MAC request, then will provision the action
		**********************************************************************************************/
		if (prog_name && strcmp(prog_name, "DELETE_GPON_MAC") == 0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "DELETE GPON MAC Provisioning enter");
			status_flags = GPON_MAC_CHANGE;
			provaction_inflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
			PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, provaction_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);
			if (old_mac_id1 ){
				PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_LOGIN, old_mac_id1, ebufp);
				tmp_flistp = PIN_FLIST_ELEM_ADD(provaction_inflistp, PIN_FLD_ALIAS_LIST, 101, ebufp);
				PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_LOGIN, old_mac_id1, ebufp);
			}
			else if (old_mac_id2 ){
				PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_LOGIN, old_mac_id2, ebufp);
				tmp_flistp = PIN_FLIST_ELEM_ADD(provaction_inflistp, PIN_FLD_ALIAS_LIST, 102, ebufp);
				PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_LOGIN, old_mac_id2, ebufp);
			}
			fm_mso_cust_get_bp_obj(ctxp, account_obj, service_obj,-1, &bp_obj, &mso_obj, ebufp);
			if ( mso_obj )
				PIN_FLIST_FLD_SET(provaction_inflistp, MSO_FLD_PURCHASED_PLAN_OBJ, mso_obj, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_service provisioning input list", provaction_inflistp);
			PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
			PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_CREATE_ACTION ", ebufp);
				PIN_ERRBUF_RESET(ebufp);
									PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
				PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
				return 0;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_service provisioning output flist", provaction_outflistp);
			PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
						
		}
		/*else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"fm_mso_update_bb_service there is no need of provision as both macs are same");
		}*/
        }

	cl_update = fm_mso_update_bal_info ( ctxp, in_flist, services_flist, out_flist, ebufp );

        if ( cl_update == 3 )
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_BILL_SET_LIMIT_AND_CR", ebufp);
                PIN_ERRBUF_RESET(ebufp);
        	PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
                return 3;
        }
	else if ( cl_update == 1 )
		 return 1;
	else if ( cl_update == 2 )
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_service: Credit Profile update done");
       	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_service update credit limit output flist", updacct_outflistp);
        //PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);
        PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
	

	if(!PIN_POID_IS_NULL(mso_obj)) PIN_POID_DESTROY(mso_obj,ebufp);
	if(!PIN_POID_IS_NULL(bp_obj)) PIN_POID_DESTROY(bp_obj,ebufp);
	
        return 2;
}

static int
fm_mso_acc_type_change(
        pcm_context_t		*ctxp,
        int64			*bus_type,
        pin_flist_t		*in_flist,
	pin_flist_t		*out_flistp,
        pin_errbuf_t		*ebufp)
{
	pin_flist_t		*accnt_info = NULL;
	pin_flist_t		*search_flistp = NULL;
	pin_flist_t		*search_res_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*write_flistp = NULL;
	pin_flist_t		*write_res_flistp = NULL;
	pin_flist_t             *srch_cc_flistp = NULL;
	pin_flist_t             *srch_cc_out_flistp = NULL;
	pin_flist_t             *result_cc_flist = NULL;
	pin_flist_t             *arg_flist = NULL;
	
	int32			subscriber_type   = -1;
	int64			*old_bus_type   = NULL;
	int32			account_type   = -1;
	int32			account_model   = -1;
	int32			new_subscriber_type   = -1;
	int32			new_account_type   = -1;
	int32			new_account_model   = -1;
	int64			db = -1;
	int32			srch_flag = 256;
	int32			pay_type = 0;
	pin_decimal_t		*min_pr = NULL;
	pin_decimal_t		*max_pr = NULL;
	poid_t			*account_obj = NULL;
	poid_t			*srch_pdp = NULL;
	poid_t                  *srch_cc_pdp = NULL;
	poid_t                  *cc_pdp = NULL;
	
	int32			status = 0;
	char			*template = "select X from /purchased_product 1,/product 2 where 1.F1 = V1 and 1.F2 = 2.F3 and 1.F4 != V4 and ( 2.F5 > V5 and 2.F6 < V6)";
	char                    *template_cc ="select X from /mso_customer_credential where  F1 = V1 ";
	char			tmp_str[512];
	int64			min = 0;
	int64			max = 0;

	void			*vp = NULL;
	void			*vp1 = NULL;
	int32			tmp_business_type = -1;
	int                     arr_business_type[8];
	int32			customer_type = -1;
	int			ret_flag = -1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"Error before calling fm_mso_acc_type_change", ebufp);
		return 3;
	}

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Updating Account Type");
	account_obj = PIN_FLIST_FLD_GET( in_flist, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	if ( !bus_type )
		return 1;	
	fm_mso_get_account_info(ctxp, account_obj, &accnt_info, ebufp);
	old_bus_type = PIN_FLIST_FLD_GET(accnt_info, PIN_FLD_BUSINESS_TYPE, 0, ebufp);	

	fm_mso_get_business_type_values(ctxp, *(int32 *)old_bus_type, &account_type, &account_model, &subscriber_type, ebufp ); 
	fm_mso_get_business_type_values(ctxp, *(int32 *)bus_type,   &new_account_type, &new_account_model, &new_subscriber_type, ebufp ); 
	
	if ( new_account_type == account_type )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Same Account Type");
		return 0;
	}

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "Error in validating account type change", ebufp);
                return 3;
        }

	if ( account_type == ACCT_TYPE_SME_SUBS_BB )
	{
		min = SME_MIN;
		max = SME_MAX;
	}
	if ( account_type == ACCT_TYPE_RETAIL_BB )
	{
		min = RTL_MIN;
		max = RTL_MAX;
	}
	/***calling function to validate the service********/
        ret_flag = fm_mso_get_services(ctxp, in_flist, ebufp);
	if(ret_flag == 3)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Valid case to change business type");
	}	
	else if(ret_flag == 2)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error while changing Business type");
		return 3;

	}
	else if(ret_flag == 1)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Not a Valid case to change business type");
		return 4;
	}
	//Pavan : Commented below types as cyber cafe is removed
	//if ( account_type == ACCT_TYPE_CYBER_CAFE_BB )
	//{
	//	min = CC_MIN;
	//	max = CC_MAX;
	//}
	if ( account_type == ACCT_TYPE_CORP_SUBS_BB )
	{
		min = CS_MIN;
		max = CS_MAX;
	}
	sprintf(tmp_str, "%d", min );
	min_pr = pbo_decimal_from_str(tmp_str,ebufp);
	sprintf(tmp_str, "%d", max );
	max_pr = pbo_decimal_from_str(tmp_str,ebufp);
	status = MSO_CLOSED;
	db = PIN_POID_GET_DB(account_obj);
	search_flistp = PIN_FLIST_CREATE(ebufp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
        PIN_FLIST_FLD_PUT(search_flistp, PIN_FLD_POID, srch_pdp, ebufp);
        PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_TEMPLATE, template , ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 3, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 4, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &status, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 5, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRIORITY, min_pr, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 6, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRIORITY, max_pr, ebufp);
	result_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_payinfo_inv search flist", search_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_flistp, &search_res_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, " fm_mso_acc_type_change: Error in searching purchased plan info", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
		free( min_pr );	
		free( max_pr );	
		return 3;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_acc_type_change:search result flist", search_res_flistp);
	result_flistp = PIN_FLIST_ELEM_GET(search_res_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	//if ( !result_flistp )
	//{
		write_flistp = PIN_FLIST_CREATE(ebufp);
        	PIN_FLIST_FLD_SET(write_flistp, PIN_FLD_POID, account_obj, ebufp);
        	PIN_FLIST_FLD_SET(write_flistp, PIN_FLD_BUSINESS_TYPE, bus_type , ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_acc_type_change write flist", write_flistp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_flistp, &write_res_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&write_flistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in write flds of business type for account", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&write_res_flistp, NULL);
			if  (min_pr)
		         free( min_pr );	
			
			if (max_pr)
			  free( max_pr );	

			return 3;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_acc_type_change business type update successful", write_res_flistp);
		PIN_FLIST_DESTROY_EX(&write_res_flistp, NULL);

		//update customer_credential_t
		tmp_business_type = *bus_type;
	        num_to_array(tmp_business_type, arr_business_type);
        	customer_type = 10*(arr_business_type[0])+arr_business_type[1];  // First 2 digit

		srch_cc_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
		srch_cc_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(srch_cc_flistp, PIN_FLD_POID, srch_cc_pdp, ebufp);
		PIN_FLIST_FLD_SET(srch_cc_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
		PIN_FLIST_FLD_SET(srch_cc_flistp, PIN_FLD_TEMPLATE, template_cc , ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_cc_flistp, PIN_FLD_ARGS, 1, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);

		result_cc_flist = PIN_FLIST_ELEM_ADD(srch_cc_flistp, PIN_FLD_RESULTS, 0, ebufp );
		PIN_FLIST_FLD_SET(result_cc_flist, PIN_FLD_CUSTOMER_TYPE, NULL, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "customer credential search input list", 
				srch_cc_flistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_cc_flistp, &srch_cc_out_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&srch_cc_flistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in calling SEARCH", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&srch_cc_out_flistp, NULL);
			if  (min_pr)
                          free( min_pr );

			if  (max_pr)
                          free( max_pr );
                        return 3;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "customer credential search output flist", 
					srch_cc_out_flistp);
		if (srch_cc_out_flistp)
		{
		  result_cc_flist = PIN_FLIST_ELEM_GET(srch_cc_out_flistp, PIN_FLD_RESULTS,0, 1,ebufp);
		  if (result_cc_flist)
		  {
			cc_pdp = PIN_FLIST_FLD_GET(result_cc_flist, PIN_FLD_POID, 0, ebufp);
		  }
		}
		PIN_FLIST_DESTROY_EX(&srch_cc_out_flistp, NULL);

                write_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(write_flistp, PIN_FLD_POID, cc_pdp, ebufp);
                PIN_FLIST_FLD_SET(write_flistp, PIN_FLD_CUSTOMER_TYPE, &customer_type , ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "customer type update write flist", 
					write_flistp);
                PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_flistp, &write_res_flistp, ebufp);
                PIN_FLIST_DESTROY_EX(&write_flistp, NULL);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
				"Error in write flds of customer type for account", ebufp);
                        PIN_ERRBUF_RESET(ebufp);
                        PIN_FLIST_DESTROY_EX(&write_res_flistp, NULL);
			if  (min_pr)
                          free( min_pr );
			if  (max_pr)
                          free( max_pr );
                        return 3;
                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
			"fm_mso_acc_type_change business type update successful", write_res_flistp);
                PIN_FLIST_DESTROY_EX(&write_res_flistp, NULL);
		
		vp  = (pin_flist_t*)PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_DATA_ARRAY, 0, 1, ebufp);
		vp1 = (pin_flist_t*)PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_DATA_ARRAY, 1, 1, ebufp); 

		if (vp)
		{
			PIN_FLIST_FLD_SET(vp, PIN_FLD_BUSINESS_TYPE, old_bus_type, ebufp);
		}
		if (vp1)
		{
			PIN_FLIST_FLD_SET(vp1, PIN_FLD_BUSINESS_TYPE, bus_type, ebufp);
		}
		return 1;
	//}
	/*else
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Cannot allow account type change as there is a plan still active for this customer", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
		return 3;
	}*/	
	
	free( min_pr );	
	free( max_pr );	
	return 1;
} 

static int
fm_mso_acc_model_change(
        pcm_context_t           *ctxp,
        int64	                *bus_type,
	int64	                *old_bus_type,
        pin_flist_t             *in_flistp,
	pin_flist_t		*out_flistp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t 		*accnt_info = NULL;
	pin_flist_t 		*search_flistp = NULL;
	pin_flist_t 		*search_res_flistp = NULL;
	pin_flist_t 		*inherited_flistp = NULL;
	pin_flist_t 		*inv_info = NULL;
	pin_flist_t 		*read_flds_flistp = NULL;
	pin_flist_t 		*read_flds_res_flistp = NULL;
	pin_flist_t 		*temp_in_flistp = NULL;
	pin_flist_t 		*nameinfo_flistp = NULL;
	pin_flist_t 		*create_flistp = NULL;
	pin_flist_t 		*create_res_flistp = NULL;
	pin_flist_t 		*result_flistp = NULL;
	pin_flist_t 		*args_flistp = NULL;
	pin_flist_t 		*billinfo_flistp = NULL;
	pin_flist_t 		*grp_flistp = NULL;
	pin_flist_t 		*group_res_flistp = NULL;
	pin_flist_t 		*set_flistp = NULL;
	pin_flist_t 		*set_res_flistp = NULL;
	pin_flist_t 		*new_bi_flistp = NULL;
	pin_flist_t 		*write_flistp = NULL;
	pin_flist_t 		*write_res_flistp = NULL;
	int32			subscriber_type   = -1;
	int32			account_type   = -1;
	int32			account_model   = -1;
	int32			new_subscriber_type   = -1;
	int32			new_account_type   = -1;
	int32			new_account_model   = -1;
        int64			db = -1;
        int			elem_id = 0;
        int			mem_count = 0;
        int32			srch_flag = 256;
	int32			pay_type = 0;
	int32			status = 0;
	int32			child = 0;
	int32			cnt_upd = 0;
	int32			inv_type = 0;
	int32			*corp_type = NULL;
	poid_t			*account_obj = NULL;
	poid_t			*srch_pdp = NULL;
	poid_t			*billinfo_obj = NULL;
	poid_t			*parent_obj = NULL;
	poid_t			*parent_billinfo_obj = NULL;
	poid_t			*ar_billinfo_obj = NULL;
	poid_t			*payinfo_obj = NULL;
        pin_cookie_t		cookie = NULL;
	char                    *template = "select X from /group/billing where F1 = V1 ";
	char                    *pay_template = "select X from /payinfo/invoice where F1 = V1 and F2.type = V2";

	void			*vp = NULL;
	void			*vp1 = NULL;


        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_mso_acc_model_change : Error before calling fm_mso_acc_model", ebufp);
                return 3;
        }

	account_obj = PIN_FLIST_FLD_GET( in_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Updating Account model");
	if ( !bus_type )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No Business Type");
		return 1;	
	}
	fm_mso_get_account_info(ctxp, account_obj, &accnt_info, ebufp);
	/* Pawan:15-DEC-2014: Commented below function call to use the value of old_bus_type passed
			in input. Because AccountType change which is called before this function already updates
			 the business type value to new business type. So account model was getting changed. */
	//fm_mso_get_business_type_values(ctxp, *(int32*)PIN_FLIST_FLD_GET(accnt_info, PIN_FLD_BUSINESS_TYPE, 0, ebufp), 
	//                        &account_type, &account_model, &subscriber_type, ebufp );
	fm_mso_get_business_type_values(ctxp, *(int32*)old_bus_type,&account_type, &account_model, &subscriber_type, ebufp );

	fm_mso_get_business_type_values(ctxp, *(int32*)bus_type, &new_account_type, &new_account_model, &new_subscriber_type, ebufp ); 
	if ( new_account_model == account_model )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Same Account model");
		return 0;
	}

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_mso_acc_model_change: Error in validating account model change", ebufp);
                return 3;
        }

	vp  = (pin_flist_t*)PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_DATA_ARRAY, 0, 1, ebufp);
	vp1 = (pin_flist_t*)PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_DATA_ARRAY, 1, 1, ebufp); 

	if (vp)
	{
		PIN_FLIST_FLD_SET(vp, PIN_FLD_BUSINESS_TYPE, old_bus_type, ebufp);
	}
	if (vp1)
	{
		PIN_FLIST_FLD_SET(vp1, PIN_FLD_BUSINESS_TYPE, bus_type, ebufp);
	}
	return 1;


	billinfo_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_BILLINFO, &elem_id, 1, &cookie, ebufp);
	billinfo_obj = PIN_FLIST_FLD_GET( billinfo_flistp, PIN_FLD_POID, 1, ebufp);
	corp_type = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_TYPE, 1, ebufp);
	if ( ( account_model == 1 ) && ( new_account_model == 2 ) )
	{
		if ( !billinfo_obj || !corp_type )
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_acc_model_change: Billinfo and corporate type details must come in i/p flist", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			return 3;
		}
		if ( *(int32 *)corp_type == 0 )
		{
			parent_obj = PIN_FLIST_FLD_GET(billinfo_flistp, PIN_FLD_PARENT, 1, ebufp);
			parent_billinfo_obj = PIN_FLIST_FLD_GET( billinfo_flistp, PIN_FLD_PARENT_BILLINFO_OBJ, 1, ebufp);			
			ar_billinfo_obj = PIN_FLIST_FLD_GET( billinfo_flistp, PIN_FLD_AR_BILLINFO_OBJ, 1, ebufp);		
			if ( !parent_obj || !parent_billinfo_obj )
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "For Corporate child, parent account and billinfo details should come in i/p flist", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				return 3;
			}
			grp_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET( grp_flistp, PIN_FLD_POID, account_obj, ebufp);
			PIN_FLIST_FLD_SET( grp_flistp, PIN_FLD_PARENT, parent_obj, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_acc_model_change move member i/p flist", grp_flistp);
			PCM_OP(ctxp, PCM_OP_BILL_GROUP_MOVE_MEMBER, 0, grp_flistp, &group_res_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&grp_flistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in updating corporate group for account", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&group_res_flistp, NULL);
				return 3;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_acc_model_change group billing update successful", group_res_flistp);
			set_flistp = PIN_FLIST_CREATE( ebufp );
			PIN_FLIST_FLD_SET(set_flistp, PIN_FLD_POID, account_obj, ebufp);
			PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, set_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
			new_bi_flistp = PIN_FLIST_ELEM_ADD(set_flistp, PIN_FLD_BILLINFO, elem_id, ebufp );	
			child = MSO_TYPE_CHILD;
			PIN_FLIST_FLD_SET(new_bi_flistp, PIN_FLD_PAY_TYPE, &child, ebufp);			
			PIN_FLIST_FLD_SET(new_bi_flistp, PIN_FLD_POID, billinfo_obj, ebufp);
			PIN_FLIST_FLD_SET(new_bi_flistp, PIN_FLD_PARENT_BILLINFO_OBJ, parent_billinfo_obj, ebufp);
			if ( !ar_billinfo_obj )
			{
				PIN_FLIST_FLD_SET(new_bi_flistp, PIN_FLD_AR_BILLINFO_OBJ, parent_billinfo_obj, ebufp);
			}
			else
				PIN_FLIST_FLD_SET(new_bi_flistp, PIN_FLD_AR_BILLINFO_OBJ, ar_billinfo_obj, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_acc_model_change: set billinfo i/p flist", set_flistp);
			PCM_OP(ctxp, PCM_OP_CUST_SET_BILLINFO, 0, set_flistp, &set_res_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&set_flistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_acc_model_change: Error in updating billinfo info", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&group_res_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&set_res_flistp, NULL);
				return 3;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_acc_model_change: billinfo update success flist", set_flistp);
			PIN_FLIST_DESTROY_EX(&group_res_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&set_res_flistp, NULL);
		}
		else if ( *(int32 *)corp_type == 1 )
		{
			grp_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET( grp_flistp, PIN_FLD_POID, account_obj, ebufp);
			PIN_FLIST_FLD_SET( grp_flistp, PIN_FLD_PARENT, account_obj, ebufp);
			PIN_FLIST_FLD_SET( grp_flistp, PIN_FLD_NAME, MSO_TYPE_CORP, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_acc_model_change group create flist", grp_flistp);
			PCM_OP(ctxp, PCM_OP_BILL_GROUP_CREATE, 0, grp_flistp, &group_res_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&grp_flistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in creating corporate group for account", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&group_res_flistp, NULL);
				return 3;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_acc_model_change group billing creation successful", group_res_flistp);
			PIN_FLIST_DESTROY_EX(&group_res_flistp, NULL);
		}	
	}
	if ( account_model == 2 && new_account_model == 1 )
	{
		if ( *(int32 *)corp_type == 0 )
		{	
			set_flistp = PIN_FLIST_CREATE( ebufp );
			PIN_FLIST_FLD_SET(set_flistp, PIN_FLD_POID, account_obj, ebufp);
			PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, set_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
			new_bi_flistp = PIN_FLIST_ELEM_ADD(set_flistp, PIN_FLD_BILLINFO, elem_id, ebufp );	
			PIN_FLIST_FLD_SET(new_bi_flistp, PIN_FLD_POID, billinfo_obj, ebufp);
			inv_type = MSO_INV_TYPE;

			db = PIN_POID_GET_DB(account_obj);
			search_flistp = PIN_FLIST_CREATE(ebufp);
			srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
			PIN_FLIST_FLD_PUT(search_flistp, PIN_FLD_POID, srch_pdp, ebufp);
			PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
			PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_TEMPLATE, pay_template , ebufp);
			args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 1, ebufp );
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
			args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 2, ebufp );
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/payinfo/invoice", -1, ebufp), ebufp);
			result_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_RESULTS, 0, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_account_model search payinfo input flist", search_flistp);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_flistp, &search_res_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&search_flistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, " fm_mso_update_bb_billinfo: Error in searching bill suppression info", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&set_flistp, NULL);
				return 3;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_billinfo:search result flist", search_res_flistp);
			result_flistp = PIN_FLIST_ELEM_GET(search_res_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
			if ( result_flistp )
			{
				payinfo_obj = PIN_FLIST_FLD_GET( result_flistp, PIN_FLD_POID, 1, ebufp);		
			}
			else
			{	
				create_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(create_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/payinfo/invoice", -1, ebufp), ebufp);
				PIN_FLIST_FLD_SET(create_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
				inv_type = MSO_INV_TYPE;
				PIN_FLIST_FLD_SET(create_flistp, PIN_FLD_PAY_TYPE, &inv_type, ebufp);
				inherited_flistp = PIN_FLIST_SUBSTR_GET(create_flistp, PIN_FLD_INHERITED_INFO, 1, ebufp);
				inv_info = PIN_FLIST_ELEM_ADD(inherited_flistp, PIN_FLD_INV_INFO, 0, ebufp);	
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_account_model create payinfo input flist", create_flistp);
				PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, create_flistp, &create_res_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&create_flistp, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, " fm_mso_update_bb_billinfo: Error in searching bill suppression info", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
					PIN_FLIST_DESTROY_EX(&create_res_flistp, NULL);
					PIN_FLIST_DESTROY_EX(&set_flistp, NULL);
					return 3;
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_billinfo:search result flist", search_res_flistp);
				payinfo_obj = PIN_FLIST_FLD_GET( create_res_flistp, PIN_FLD_POID, 1, ebufp);		
				PIN_FLIST_DESTROY_EX(&create_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
				
				read_flds_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET( read_flds_flistp, PIN_FLD_POID, account_obj, ebufp);
				PIN_FLIST_ELEM_ADD(read_flds_flistp, PIN_FLD_NAMEINFO, 1, ebufp);
				PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flds_flistp, &read_flds_res_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&read_flds_flistp, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, " fm_mso_update_account_model: Error in reading nameinfo", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
					PIN_FLIST_DESTROY_EX(&create_res_flistp, NULL);
					PIN_FLIST_DESTROY_EX(&read_flds_res_flistp, NULL);
					PIN_FLIST_DESTROY_EX(&set_flistp, NULL);
					return 3;
				}
				nameinfo_flistp = PIN_FLIST_ELEM_GET(read_flds_res_flistp, PIN_FLD_NAMEINFO, 1, 1, ebufp);
				if ( !nameinfo_flistp )
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, " fm_mso_update_account_model: No billing address present", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
					PIN_FLIST_DESTROY_EX(&create_res_flistp, NULL);
					PIN_FLIST_DESTROY_EX(&read_flds_res_flistp, NULL);
					PIN_FLIST_DESTROY_EX(&set_flistp, NULL);
					return 3;
				}
				temp_in_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET( temp_in_flistp, PIN_FLD_POID, account_obj, ebufp);
                		PIN_FLIST_ELEM_SET(temp_in_flistp, nameinfo_flistp, PIN_FLD_NAMEINFO, 1, ebufp );
				cnt_upd = fm_mso_update_payinfo_inv( ctxp, temp_in_flistp, ebufp);	
				if ( cnt_upd == 3 )
					return 3;
				PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&create_res_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&read_flds_res_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&temp_in_flistp, NULL);
			}
			PIN_FLIST_FLD_SET(new_bi_flistp, PIN_FLD_PAY_TYPE, &inv_type, ebufp);			
			PIN_FLIST_FLD_SET(new_bi_flistp, PIN_FLD_PAYINFO_OBJ, payinfo_obj, ebufp);
			PIN_FLIST_FLD_SET(new_bi_flistp, PIN_FLD_PARENT_BILLINFO_OBJ, billinfo_obj, ebufp);
			PIN_FLIST_FLD_SET(new_bi_flistp, PIN_FLD_AR_BILLINFO_OBJ, billinfo_obj, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_acc_model_change: set billinfo i/p flist", set_flistp);
			PCM_OP(ctxp, PCM_OP_CUST_SET_BILLINFO, 0, set_flistp, &set_res_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&set_flistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_acc_model_change: Error in updating billinfo info", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&set_res_flistp, NULL);
			}
			grp_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET (grp_flistp,PIN_FLD_POID, account_obj, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_acc_model_change delete group member i/p flist", grp_flistp);
			PCM_OP(ctxp, PCM_OP_BILL_GROUP_MOVE_MEMBER, 0, grp_flistp, &group_res_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&grp_flistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in removing account from corp group", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&group_res_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&set_res_flistp, NULL);
				return 3;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_acc_model_change group billing update successful", group_res_flistp);


			PIN_FLIST_DESTROY_EX(&group_res_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&set_res_flistp, NULL);
		}
		else if ( *(int32 *)corp_type == 1 )
		{
			db = PIN_POID_GET_DB(account_obj);
			search_flistp = PIN_FLIST_CREATE(ebufp);
			srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
			PIN_FLIST_FLD_PUT(search_flistp, PIN_FLD_POID, srch_pdp, ebufp);
			PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
			PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	                args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 1, ebufp );
        	        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PARENT, account_obj, ebufp);
			result_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_RESULTS, 0, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
			PIN_FLIST_ELEM_ADD(result_flistp, PIN_FLD_MEMBERS, PIN_ELEMID_ANY, ebufp );
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_payinfo_inv search flist", search_flistp);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_flistp, &search_res_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&search_flistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, " fm_mso_acc_model_change: Error in searching hierarchy", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
				return 3;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_acc_model_change:search result flist", search_res_flistp);
			result_flistp = PIN_FLIST_ELEM_GET(search_res_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
			if ( !result_flistp )
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No Hierarchy to remove for this account");
				PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
				return 1;
			}
			mem_count = PIN_FLIST_ELEM_COUNT(result_flistp, PIN_FLD_MEMBERS, ebufp);
			if ( mem_count > 0 )
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_acc_model_change: Group members still exist in this hierarchy", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
				return 3;
			}
			grp_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET (grp_flistp,PIN_FLD_POID, account_obj, ebufp);
			PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_POID, grp_flistp, PIN_FLD_GROUP_OBJ, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_acc_model_change delete group i/p flist", grp_flistp);
			PCM_OP(ctxp, PCM_OP_BILL_GROUP_MOVE_MEMBER, 0, grp_flistp, &group_res_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&grp_flistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in deleting corporate or complex group", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&group_res_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
				return 3;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_acc_model_change delete group o/p flist", group_res_flistp);
			PIN_FLIST_DESTROY_EX(&group_res_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
		}
	}
	write_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(write_flistp, PIN_FLD_POID, account_obj, ebufp);
        PIN_FLIST_FLD_SET(write_flistp, PIN_FLD_BUSINESS_TYPE, bus_type , ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_acc_type_change write flist", write_flistp);
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_flistp, &write_res_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&write_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in write flds of business type for account", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&write_res_flistp, NULL);
		return 3;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_acc_type_change business type update successful", write_res_flistp);
	PIN_FLIST_DESTROY_EX(&write_res_flistp, NULL);

	return 2;
}

/*****************************************
* Function: fm_mso_get_outstanding_bal()
*	Takes account poid and billinfo id
*	(BB or CATV) as input and returns 
*	outstanding amount.
*****************************************/
static void 
fm_mso_get_outstanding_bal(
	pcm_context_t		*ctxp,
	poid_t		*acc_pdp,
	char			*bi_type,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t 		*srch_in_flistp = NULL;
	pin_flist_t 		*srch_out_flistp = NULL;
	pin_flist_t 		*args_flistp = NULL;
	pin_flist_t 		*result_flistp = NULL;

	poid_t			*account_obj = NULL;
	poid_t			*billinfo_obj = NULL;
	poid_t			*supr_obj = NULL;
	poid_t			*srch_pdp = NULL;
	int32			status = 0;
	int32			cnt = 0;
	int32			*reason = NULL;
	int32			*flag = NULL;
	int			elem_id = 0;
        int64                   db = -1;
        int32                   srch_flag = 256;
        char                    *template = "select X from /billinfo where  F1 = V1 and F2 = V2 ";
	pin_cookie_t		cookie = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"Error before calling fm_mso_get_outstanding_bal", ebufp);
			return;
	}

	db = PIN_POID_GET_DB(acc_pdp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acc_pdp, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BILLINFO_ID, bi_type, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_outstanding_bal search input", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, " fm_mso_get_outstanding_bal: Error in searching billinfo", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_outstanding_bal: search output", srch_out_flistp);
	result_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	if ( !result_flistp )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Billinfo Not found for specified account poid and billinfo type.");
		PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
		return;
	}

	*r_flistpp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, acc_pdp, ebufp);
	PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_BILLINFO_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_BILLINFO_ID, *r_flistpp, PIN_FLD_BILLINFO_ID, ebufp);
	PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_PENDING_RECV, *r_flistpp, PIN_FLD_PENDING_RECV, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_outstanding_bal output flist", *r_flistpp);
}


/*****************************************
* Function: fm_mso_create_lco_change_evt()
*	Generates an event of type 
*	/event/mso_cust_lco_change whenever  
*	customer LCO is changed.
*****************************************/
static int 
fm_mso_create_lco_change_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**comm_err_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t 		*evt_in_flistp = NULL;
	pin_flist_t 		*evt_out_flistp = NULL;
	pin_flist_t 		*lco_flistp = NULL;
	pin_flist_t 		*elem_flistp = NULL;
	pin_flist_t 		*data_flistp = NULL;
	pin_flist_t 		*cc_flistp = NULL;
	pin_flist_t 		*res_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	
	
	poid_t			*evt_pdp = NULL;
	poid_t			*acc_pdp = NULL;
	poid_t			*old_lco_pdp = NULL;
	poid_t			*new_lco_pdp = NULL;
	int64			db = -1;
	int			*old_comm_model = NULL;
	int			*new_comm_model = NULL;
	int			*ret_status = NULL;
	time_t			now_t = pin_virtual_time((time_t *)NULL);


	/*** Changes for LCO change reason code ***/
	int			*reason_id = NULL;
	/*** Changes for LCO change reason code ***/

	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_create_lco_change_evt");
		return 1;
	}	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_lco_change_evt input flist", in_flistp);
	/*acc_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	db = PIN_POID_GET_DB(acc_pdp);
	evt_pdp = PIN_POID_CREATE(db, "/event/mso_cust_lco_change", -1, ebufp);
	
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, evt_in_flistp, PIN_FLD_POID, ebufp);	
	sub_flistp = PIN_FLIST_SUBSTR_ADD(evt_in_flistp, PIN_FLD_EVENT, ebufp);
	PIN_FLIST_FLD_PUT(sub_flistp, PIN_FLD_POID, evt_pdp, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, sub_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_NAME, "LCO_CHANGE", ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, sub_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_END_T, (void *)&now_t, ebufp);
	PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_START_T, (void *)&now_t, ebufp);
	elem_flistp = PIN_FLIST_ELEM_ADD(sub_flistp,PIN_FLD_EVENT_MISC_DETAILS,0,ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_REASON_ID, elem_flistp, PIN_FLD_REASON_ID, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, elem_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	data_flistp = PIN_FLIST_ELEM_GET(in_flistp,PIN_FLD_DATA_ARRAY,0, 1, ebufp);
	if(data_flistp) {
		cc_flistp = PIN_FLIST_SUBSTR_GET(data_flistp,PIN_FLD_CUSTOMER_CARE_INFO, 1, ebufp);
		if(cc_flistp) {
			PIN_FLIST_FLD_COPY(cc_flistp, MSO_FLD_LCO_OBJ, elem_flistp, MSO_FLD_LCO_OBJ, ebufp);
		}
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "LCO change event creation input flist", evt_in_flistp);
	PCM_OP(ctxp, PCM_OP_ACT_USAGE, 0, evt_in_flistp, &evt_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "LCO change creation output flist", evt_out_flistp);
	*/
	
	/* Fetch details of OLD LCO */
	cc_flistp = PIN_FLIST_SUBSTR_GET(in_flistp,PIN_FLD_CUSTOMER_CARE_INFO, 1, ebufp);
	if(cc_flistp) {
		new_lco_pdp = (poid_t *)PIN_FLIST_FLD_GET(cc_flistp, MSO_FLD_LCO_OBJ, 0, ebufp);
	}
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "New LCO poid",new_lco_pdp);
	if(new_lco_pdp)
	{
		fm_mso_get_lco_info(ctxp, new_lco_pdp, &lco_flistp,ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Cannot fetch old information");
			return 1;
		}
		res_flistp = PIN_FLIST_ELEM_GET(lco_flistp,PIN_FLD_RESULTS, 0, 1, ebufp);
		if(res_flistp) {
			cc_flistp = PIN_FLIST_SUBSTR_GET(res_flistp,MSO_FLD_WHOLESALE_INFO, 1, ebufp);
			if(cc_flistp) {
				 new_comm_model = PIN_FLIST_FLD_TAKE(cc_flistp, MSO_FLD_COMMISION_MODEL, 0, ebufp);
			}
		}
	}
	PIN_FLIST_DESTROY_EX(&lco_flistp, NULL);
	
	/* Fetch details of New LCO */
	data_flistp = PIN_FLIST_ELEM_GET(in_flistp,PIN_FLD_DATA_ARRAY,0, 1, ebufp);
	if(data_flistp) {
		cc_flistp = PIN_FLIST_SUBSTR_GET(data_flistp,PIN_FLD_CUSTOMER_CARE_INFO, 1, ebufp);
		if(cc_flistp) {
			old_lco_pdp = (poid_t *)PIN_FLIST_FLD_GET(cc_flistp, MSO_FLD_LCO_OBJ, 0, ebufp);
		}
	}
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "OLD LCO poid",old_lco_pdp);
	if(old_lco_pdp)
	{
		fm_mso_get_lco_info(ctxp, old_lco_pdp, &lco_flistp,ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Cannot fetch old information");
			return 1;
		}
		res_flistp = PIN_FLIST_ELEM_GET(lco_flistp,PIN_FLD_RESULTS, 0, 1, ebufp);
		if(res_flistp) {
			cc_flistp = PIN_FLIST_SUBSTR_GET(res_flistp,MSO_FLD_WHOLESALE_INFO, 1, ebufp);
			if(cc_flistp) {
				 old_comm_model = PIN_FLIST_FLD_TAKE(cc_flistp, MSO_FLD_COMMISION_MODEL, 0, ebufp);
			}
		}
	}
	PIN_FLIST_DESTROY_EX(&lco_flistp, NULL);
	/******* Pavan Bellala - Fix for LCO change Wrong tagging *******
	Add new field PIN_FLD_REASON_ID 
	****************************************************************/
	reason_id = PIN_FLIST_FLD_GET(in_flistp,PIN_FLD_REASON_ID,1,ebufp);
//	if(old_lco_pdp && new_lco_pdp && *old_comm_model == *new_comm_model)
//	{
		
		if(reason_id && (*reason_id == MSO_LCO_CHNG_WRNG_TAGGING))
		{
			evt_in_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, evt_in_flistp, PIN_FLD_POID, ebufp);
			elem_flistp = PIN_FLIST_ELEM_ADD(evt_in_flistp,PIN_FLD_ACCOUNTS, 0,ebufp);
			PIN_FLIST_FLD_SET(elem_flistp, PIN_FLD_POID, (void *)old_lco_pdp, ebufp);
			elem_flistp = PIN_FLIST_ELEM_ADD(evt_in_flistp,PIN_FLD_ACCOUNTS, 1,ebufp);
			PIN_FLIST_FLD_SET(elem_flistp, PIN_FLD_POID, (void *)new_lco_pdp, ebufp);

			PIN_FLIST_ELEM_COPY(in_flistp, PIN_FLD_DATA_ARRAY, 0, evt_in_flistp, PIN_FLD_DATA_ARRAY, 0, ebufp);
			PIN_FLIST_ELEM_COPY(in_flistp, PIN_FLD_DATA_ARRAY, 1, evt_in_flistp, PIN_FLD_DATA_ARRAY, 1, ebufp);
			PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, evt_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_COMMISSION_RECTIFY_LCO_WRONG_TAGGING input flist", evt_in_flistp);
			PCM_OP(ctxp, MSO_OP_COMMISSION_RECTIFY_LCO_WRONG_TAGGING, 0, evt_in_flistp, &evt_out_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_COMMISSION_RECTIFY_LCO_WRONG_TAGGING output flist", evt_out_flistp);

			if (evt_out_flistp)
			{
				ret_status = PIN_FLIST_FLD_GET(evt_out_flistp, PIN_FLD_STATUS, 1, ebufp);
				if (ret_status && *ret_status == 1)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "returning");
					*comm_err_flist = PIN_FLIST_COPY(evt_out_flistp, ebufp);
					PIN_FLIST_DESTROY_EX(&evt_in_flistp, NULL);
					PIN_FLIST_DESTROY_EX(&evt_out_flistp, NULL);
					return 1;
				}
			}
		}
//	}
//	else
//	{
//		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Commission model does not match");
//		return 1;
//	}
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_FLIST_DESTROY_EX(&evt_in_flistp, NULL);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "LCO change event creation failed");
		return 1;
	}
	
	PIN_FLIST_DESTROY_EX(&evt_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&evt_out_flistp, NULL);
	return 0;
}
/****************************************************
* Function: fm_mso_update_user_status()
*		Updates the status of Business User account
*
*****************************************************/
static void
fm_mso_update_user_status(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,	
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t 		*act_in_flistp = NULL;
	pin_flist_t 		*act_out_flistp = NULL;
	pin_flist_t 		*status_flistp = NULL;
	pin_flist_t 		*lc_in_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*args_flistp = NULL;

	poid_t			*acct_pdp = NULL;

	char			*account_no = NULL;
	char			status_msg[128]="";
	
	int			status_flag = 0;
	int32			*status = NULL;
	int32			*old_status = NULL;

	void			*vp = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_update_user_status");
		return;
	}	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_user_status input flist", in_flistp);

	status = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_STATUS, 1, ebufp);
	if (!status)
	{
		return;
	}
	acct_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1, ebufp);
	account_no = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	//if (acct_pdp && !account_no)
	if (acct_pdp)
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
		old_status =  PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_STATUS, 1, ebufp);
	}

	if (status && old_status && *status != *old_status )
	{
		act_in_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, act_in_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, act_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_DESCR, act_in_flistp, PIN_FLD_DESCR, ebufp);
		
		status_flistp = PIN_FLIST_ELEM_ADD(act_in_flistp, PIN_FLD_STATUSES, 0, ebufp );
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_STATUS, status_flistp, PIN_FLD_STATUS, ebufp);
		PIN_FLIST_FLD_SET(status_flistp, PIN_FLD_STATUS_FLAGS, &status_flag, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Set status Input Flist", act_in_flistp);
		PCM_OP(ctxp, PCM_OP_CUST_SET_STATUS, 0, act_in_flistp, &act_out_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Set status output flist", act_out_flistp);
		PIN_FLIST_DESTROY_EX(&act_in_flistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "CSR Status update error", ebufp);
			return;
		}
		lc_in_flistp = PIN_FLIST_CREATE(ebufp);
		vp = (pin_flist_t*)PIN_FLIST_ELEM_ADD(lc_in_flistp, MSO_FLD_DEACTIVATE_ACCOUNT, 0, ebufp);
		PIN_FLIST_FLD_SET(vp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
		PIN_FLIST_FLD_SET(vp, PIN_FLD_STATUS, old_status, ebufp);

		vp = (pin_flist_t*)PIN_FLIST_ELEM_ADD(lc_in_flistp, MSO_FLD_DEACTIVATE_ACCOUNT, 1, ebufp);
		PIN_FLIST_FLD_SET(vp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
		PIN_FLIST_FLD_SET(vp, PIN_FLD_STATUS, status, ebufp);

		if (*old_status == 10100 && *status == 10102 )
		{
			strcpy(status_msg, "from ACTIVE to INACTIVE");
		}
		else if (*old_status == 10100 && *status == 10103 )
		{
			strcpy(status_msg, "from ACTIVE to CANCEL");
		}
		else if (*old_status == 10102 && *status == 10100 )
		{
			strcpy(status_msg, "from INACTIVE to ACTIVE");
		}
		else if (*old_status == 10102 && *status == 10103 )
		{
			strcpy(status_msg, "from INACTIVE to CANCEL");
		}
		else if (*old_status == 10103 && *status == 10100 )
		{
			strcpy(status_msg, "from CANCEL to ACTIVE");
		}
		else if (*old_status == 10103 && *status == 10102 )
		{
			strcpy(status_msg, "from CANCEL to INACTIVE");
		}
		
		PIN_FLIST_FLD_SET(lc_in_flistp, PIN_FLD_MESSAGE, status_msg, ebufp ); 

		fm_mso_create_lifecycle_evt(ctxp, in_flistp, lc_in_flistp, ID_MODIFY_CUSTOMER_DEACTIVATE_ACTIVATE_BUS_USER, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
			PIN_ERRBUF_RESET(ebufp);
		}
		PIN_FLIST_DESTROY_EX(&lc_in_flistp, NULL);

		// Added below to populate the DATA_ARRAY
		args_flistp = PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_DATA_ARRAY, 0,0, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, old_status, ebufp);
		args_flistp = PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_DATA_ARRAY, 1,0, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, status, ebufp);

	}
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
}



void
fm_mso_get_lco_info(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*profile_substr = NULL;

	int32			srch_flag = 512;
	int32			srch_type = -1;

	poid_t			*srch_pdp = NULL;
	poid_t			*obj = NULL;

	char			*template = "select X from /profile where F1.id = V1 ";
	char			*profile_type = NULL;
	int64			db = -1;


	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_lco_info", ebufp);
	}
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_lco_info acnt_pdp", acnt_pdp);

	db = PIN_POID_GET_DB(acnt_pdp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template, ebufp);
	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);
	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_lco_info SEARCH input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_lco_info SEARCH output list", *ret_flistp);


	CLEANUP:
	return;
}

/**************************************************
* Function: fm_mso_update_salesman()
* 	Updates salesman obj in customer care profile
*
***************************************************/
static int 
fm_mso_update_salesman(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		*out_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*updacct_inflistp = NULL;
	pin_flist_t		*updacct_outflistp = NULL;
	pin_flist_t		*customecare_flistp = NULL;
	pin_flist_t		*inheritedinfo_flist = NULL;
	pin_flist_t		*cc_mod_flistp = NULL;
	pin_flist_t		*curr_profile_flistp = NULL;
	pin_flist_t		*old_cc_flistp = NULL;
	pin_flist_t		*profiles_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*svc_mod_flistp = NULL;
	pin_flist_t		*svc_out_flistp = NULL;
	pin_flist_t		*inh_flist = NULL;
	pin_flist_t		*bb_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*svc_flistp = NULL;

	pin_cookie_t	cookie = NULL;
	poid_t			*new_sales_obj = NULL;
	poid_t			*old_sales_obj = NULL;
	poid_t			*acct_pdp = NULL;
	poid_t			*prof_pdp = NULL;
	poid_t			*svc_pdp = NULL;
	void 			*vp = NULL;
	int32			rec_id = 0;
	int32			srch_flag = 512;
	int32			srch_type = -1;
	poid_t			*srch_pdp = NULL;
	char			*template = "select X from /service where F1 = V1 and F2.type like V2 ";
	int64			db = -1;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_salesman: input flist", in_flist);	
	customecare_flistp = PIN_FLIST_SUBSTR_GET(in_flist, PIN_FLD_CUSTOMER_CARE_INFO, 1, ebufp );
	if ((PIN_ERRBUF_IS_ERR(ebufp)) || (!customecare_flistp))
	{
		PIN_ERRBUF_RESET(ebufp);
		return 1;
	}
	new_sales_obj = PIN_FLIST_FLD_GET(customecare_flistp, MSO_FLD_SALESMAN_OBJ, 1, ebufp);
	if (!new_sales_obj)
	{
		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_salesman: NO Salesman obj passed in input flist");
		return 1;
	}	
	acct_pdp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	fm_mso_get_lco_info(ctxp, acct_pdp, &ret_flistp, ebufp);
	curr_profile_flistp = PIN_FLIST_ELEM_GET(ret_flistp, PIN_FLD_RESULTS, 0, 1, ebufp );
	if(curr_profile_flistp)
	{
		prof_pdp = PIN_FLIST_FLD_GET(curr_profile_flistp, PIN_FLD_POID, 1, ebufp);
	}
	else 
	{
		return 1;
	}
	old_cc_flistp = PIN_FLIST_SUBSTR_GET(curr_profile_flistp, PIN_FLD_CUSTOMER_CARE_INFO, 1, ebufp );
	old_sales_obj = PIN_FLIST_FLD_GET(customecare_flistp, MSO_FLD_SALESMAN_OBJ, 1, ebufp);
	
	// Prepare input flist for customercare profile update
	updacct_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(updacct_inflistp, PIN_FLD_POID,prof_pdp, ebufp);
	profiles_flistp = PIN_FLIST_ELEM_ADD(updacct_inflistp, PIN_FLD_PROFILES, 0, ebufp);
	PIN_FLIST_FLD_SET(profiles_flistp, PIN_FLD_PROFILE_OBJ,prof_pdp, ebufp);
	inheritedinfo_flist = PIN_FLIST_SUBSTR_ADD(profiles_flistp, PIN_FLD_INHERITED_INFO, ebufp);
	cc_mod_flistp = PIN_FLIST_SUBSTR_ADD(inheritedinfo_flist, PIN_FLD_CUSTOMER_CARE_INFO, ebufp);

	PIN_FLIST_FLD_SET(cc_mod_flistp, MSO_FLD_SALESMAN_OBJ, new_sales_obj, ebufp);

	vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(out_flist, PIN_FLD_DATA_ARRAY, 1,1, ebufp);
	PIN_FLIST_SUBSTR_SET(vp, cc_mod_flistp, PIN_FLD_CUSTOMER_CARE_INFO, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_salesman:  profile update input flist", updacct_inflistp);
	PCM_OP(ctxp, PCM_OP_CUST_MODIFY_PROFILE , 0, updacct_inflistp, &updacct_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&updacct_inflistp, NULL);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_salesman :  LCO profile update output flist", updacct_outflistp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_update_salesman: Error in calling PCM_OP_CUST_MODIFY_PROFILE for end users", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);
		return 0;
	} 

	/* Fetch the broadband service for this account*/
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_salesman acnt_pdp", acct_pdp);
	db = PIN_POID_GET_DB(acct_pdp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template, ebufp);
	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
	svc_pdp = PIN_POID_CREATE(db, "/service/telco/broadband", 0, ebufp);
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, svc_pdp, ebufp);
	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_salesman SEARCH input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_salesman SEARCH output list", ret_flistp);
	
	while ( (svc_flistp = PIN_FLIST_ELEM_GET_NEXT (ret_flistp, 
				PIN_FLD_RESULTS, &rec_id, 1,&cookie, ebufp)) != (pin_flist_t *)NULL )
	{
		svc_mod_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_POID, svc_mod_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_USERID, svc_mod_flistp, PIN_FLD_USERID, ebufp);
		PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_ACCOUNT_OBJ, svc_mod_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_PROGRAM_NAME, svc_mod_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(svc_mod_flistp, PIN_FLD_SERVICES, 0, ebufp );
		PIN_FLIST_FLD_COPY(svc_flistp,PIN_FLD_POID, arg_flist, PIN_FLD_POID, ebufp);
		inh_flist = PIN_FLIST_SUBSTR_ADD(arg_flist, PIN_FLD_INHERITED_INFO, ebufp );
		bb_flistp = PIN_FLIST_SUBSTR_ADD(inh_flist, MSO_FLD_BB_INFO, ebufp );
		PIN_FLIST_FLD_SET(bb_flistp, MSO_FLD_SALESMAN_OBJ, new_sales_obj, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_salesman:Service update input", svc_mod_flistp);
		PCM_OP(ctxp, PCM_OP_CUST_UPDATE_SERVICES, 0, svc_mod_flistp, &svc_out_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&svc_mod_flistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_CUST_UPDATE_SERVICES", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&svc_mod_flistp, NULL);
				return 0;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_salesman:Service update output", svc_out_flistp);
		PIN_FLIST_DESTROY_EX(&svc_out_flistp, NULL);
	}

	PIN_FLIST_DESTROY_EX(&updacct_outflistp, NULL);	
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
        PIN_POID_DESTROY(svc_pdp, NULL);
	return 2;
}


/*****************************************************************************************
This Function will modify in /profile/wholesale only and not /profile/customer_care:
	CUSTOMER_TYPE
	PREF_DOM
	PP_TYPE
	ERP_CONTROL_ACCT_ID
	ENT_TAX_NO
	ERP_CONTROL_ACCT_ID
	PAN_NO
	ST_REG_NO
	VAT_TAX_NO
	COMMISION_MODEL
	COMMISION_SERVICE
	CUSTOMER_SEGMENT

*****************************************************************************************/
static int 
fm_mso_update_bu_profile(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		*out_flist,
	pin_errbuf_t		*ebufp)
{

	pin_flist_t		*wholesale_flistp     = NULL;
	pin_flist_t		*read_profile_iflist  = NULL;
	pin_flist_t		*read_profile_oflist  = NULL;
	pin_flist_t		*update_profile_iflist= NULL;
	pin_flist_t		*update_profile_oflist= NULL;
	pin_flist_t		*wholesale_info       = NULL;
	pin_flist_t		*old_wholesale_flistp = NULL;

	poid_t			*account_obj = NULL;
	poid_t			*profile_obj =NULL;

	int32			*customer_type       = NULL;
	int32			*old_customer_type       = NULL;
	int32			*pref_dom            = NULL;
	int32			*old_pref_dom            = NULL;
	int32			*pp_type             = NULL;
	int32			*old_pp_type             = NULL;
	int32			*erp_control_acct_id = NULL;
	int32			*old_erp_control_acct_id = NULL;
	int32			*commision_model     = NULL;
	int32			*old_commision_model     = NULL;
	int32			*commision_service   = NULL;
	int32			*old_commision_service   = NULL;
	int32			*customer_segment    = NULL;
	int32			*old_customer_segment    = NULL;
	int32			srch_by_acnt = 9;
	int32			allow_update = 0;
	int32			action_mode  = 99;
	int32			ret_status = 2;
	int32			count_cust = 0;
	
	int64			db = 1;
	char			template[128] = "select X from /profile/wholesale where F1 = V1 and profile_t.poid_type = '/profile/wholesale' ";
	char			*ent_tax_no   = NULL;
	char			*pan_no       = NULL;
	char			*st_reg_no    = NULL;
	char			*vat_tax_no   = NULL;
	char			*old_ent_tax_no   = NULL;
	char			*old_pan_no       = NULL;
	char			*old_st_reg_no    = NULL;
	char			*old_vat_tax_no   = NULL;
	char			err_msg[255]  = "";
	char			*das_type = NULL;
	char			*old_das_type = NULL;
	char			*post_reg_no = NULL;
	char			*old_post_reg_no = NULL;

	void			*vp = NULL;
	void			*vp1 = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bu_profile search  profile input list", in_flist);

	account_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp );
	profile_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_PROFILE_OBJ, 1, ebufp );

	wholesale_flistp = PIN_FLIST_SUBSTR_GET(in_flist, MSO_FLD_WHOLESALE_INFO, 1, ebufp );
	if (!wholesale_flistp)
	{
		return 1;
	}

	customer_type         = PIN_FLIST_FLD_GET(wholesale_flistp, PIN_FLD_CUSTOMER_TYPE       , 1, ebufp );
	pref_dom              = PIN_FLIST_FLD_GET(wholesale_flistp, MSO_FLD_PREF_DOM            , 1, ebufp );
	pp_type               = PIN_FLIST_FLD_GET(wholesale_flistp, MSO_FLD_PP_TYPE             , 1, ebufp );
	erp_control_acct_id   = PIN_FLIST_FLD_GET(wholesale_flistp, MSO_FLD_ERP_CONTROL_ACCT_ID , 1, ebufp );
	ent_tax_no            = PIN_FLIST_FLD_GET(wholesale_flistp, MSO_FLD_ENT_TAX_NO          , 1, ebufp );
	pan_no                = PIN_FLIST_FLD_GET(wholesale_flistp, MSO_FLD_PAN_NO              , 1, ebufp );
	st_reg_no             = PIN_FLIST_FLD_GET(wholesale_flistp, MSO_FLD_ST_REG_NO           , 1, ebufp );
	vat_tax_no            = PIN_FLIST_FLD_GET(wholesale_flistp, MSO_FLD_VAT_TAX_NO          , 1, ebufp );
	commision_model       = PIN_FLIST_FLD_GET(wholesale_flistp, MSO_FLD_COMMISION_MODEL     , 1, ebufp );
	commision_service     = PIN_FLIST_FLD_GET(wholesale_flistp, MSO_FLD_COMMISION_SERVICE   , 1, ebufp );
	customer_segment      = PIN_FLIST_FLD_GET(wholesale_flistp, PIN_FLD_CUSTOMER_SEGMENT    , 1, ebufp );
	das_type              = PIN_FLIST_FLD_GET(wholesale_flistp, MSO_FLD_DAS_TYPE            , 1, ebufp );
	post_reg_no           = PIN_FLIST_FLD_GET(wholesale_flistp, MSO_FLD_POSTAL_REG_NO       , 1, ebufp );

	PIN_FLIST_FLD_SET(out_flist, PIN_FLD_ACTION_MODE, &action_mode, ebufp);

	update_profile_iflist = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(update_profile_iflist, PIN_FLD_POID, profile_obj, ebufp);  
	
	vp = (pin_flist_t*)PIN_FLIST_ELEM_ADD(update_profile_iflist, PIN_FLD_PROFILES, 0, ebufp);
	PIN_FLIST_FLD_SET(vp, PIN_FLD_PROFILE_OBJ, profile_obj, ebufp);
	
	vp1 = (pin_flist_t*)PIN_FLIST_SUBSTR_ADD(vp, PIN_FLD_INHERITED_INFO, ebufp);
	wholesale_info = PIN_FLIST_SUBSTR_ADD(vp1, MSO_FLD_WHOLESALE_INFO, ebufp);

	/***********************************************
	Validate Input -Start
	***********************************************/
	if (pref_dom)
	{
		if (*pref_dom < 1 || *pref_dom >28 )
		{
			sprintf(err_msg, "Invalid PREF_DOM '%d'. Allowed 1-28.", *pref_dom);
			ret_status = 1;
			goto CLEANUP;
		}
		strcpy(err_msg, "ACCOUNT Modification done successfully. PREF_DOM type modified for BU only. Underlying customer's BDOM not modified.");
		PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_PREF_DOM ,pref_dom, ebufp);
	}
	if (pp_type)
	{
		if (*pp_type < 0 || *pp_type > 1 )
		{
			sprintf(err_msg, "Invalid PP_TYPE '%d'. Valid values PRIMARY and SECONDARY", *pp_type);
			ret_status = 1;
			goto CLEANUP;
		}
		if (pref_dom)
		{
			strcat(err_msg, "PP type modified for BU only. Underlying customer's PP type not modified." );
		}
		else
		{
			strcpy(err_msg, "ACCOUNT Modification done successfully. PP type modified for BU only. Underlying customer's PP type not modified. ");
		}
		PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_PP_TYPE ,pp_type, ebufp);
	}
	if (customer_segment)
	{
		if (*customer_segment < 0 || *customer_segment > 1 )
		{
			sprintf(err_msg, "Invalid CUSTOMER_SEGMENT '%d'. Valid values 0 and 1", *customer_segment);
			ret_status = 1;
			goto CLEANUP;
		}
		if (pp_type || pref_dom )
		{
			strcat(err_msg, "SEGMENT FLAG modified for BU only. Underlying customer's BILL_SEGMENT not modified." );
		}
		else
		{
			sprintf(err_msg, "ACCOUNT Modification done successfully.\nSEGMENT FLAG modified for BU only. Underlying customer's BILL_SEGMENT not modified.");
		}
		PIN_FLIST_FLD_SET(wholesale_info, PIN_FLD_CUSTOMER_SEGMENT ,customer_segment, ebufp);
	}
	if (erp_control_acct_id)
	{
		PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_ERP_CONTROL_ACCT_ID ,erp_control_acct_id, ebufp);
	}
	if (ent_tax_no)
	{
		PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_ENT_TAX_NO ,ent_tax_no, ebufp);
	}
	if (pan_no)
	{
		PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_PAN_NO ,pan_no, ebufp);
	}
	if (st_reg_no)
	{
		PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_ST_REG_NO ,st_reg_no, ebufp);
	}
	if (vat_tax_no)
	{
		PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_VAT_TAX_NO ,vat_tax_no, ebufp);
	}
	if (commision_model)
	{
		if (*commision_model <0 || *commision_model > 5 )
		{
			sprintf(err_msg, "Invalid COMMISION_MODEL '%d'. Allowed 1-28", *commision_model);
			ret_status = 1;
			goto CLEANUP;
		}
		PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_COMMISION_MODEL ,commision_model, ebufp);
	}
	if (commision_service)
	{
		if (*commision_service < 0 || *commision_service > 2 )
		{
			sprintf(err_msg, "Invalid COMMISION_SERVICE '%d'. Allowed 0-2", *commision_service );
			ret_status = 1;
			goto CLEANUP;
		}
		PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_COMMISION_SERVICE ,commision_service, ebufp);
	}
	if (das_type)
	{
		PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_DAS_TYPE, das_type, ebufp);
	}
	if (post_reg_no)
	{
		PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_POSTAL_REG_NO, post_reg_no, ebufp);
	}
	/***********************************************
	Validate Input -End
	***********************************************/

	/***********************************************
	 Prevent modification when there will be 
	 de-synchronisation
	***********************************************/
	if (pref_dom || pp_type || customer_segment )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "12345");
		count_cust = fm_mso_get_cust_of_lco_count(ctxp, in_flist, NULL, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_RESET(ebufp);
			strcpy(err_msg, "Error in fm_mso_get_cust_of_lco_count");
			ret_status = 1;
			goto CLEANUP;
		}
		if (count_cust > 0)
		{
 			sprintf(err_msg, "This BU has %d customers. PREF_DOM, PP_TYPE or SEGMENT_FLAG modification not allowed", count_cust);
			ret_status = 1;
			goto CLEANUP;
		}
	}
	
	/***********************************************
	Read Existing profile -Start
	***********************************************/
	read_profile_iflist = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist ,PIN_FLD_POID,read_profile_iflist, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(in_flist ,PIN_FLD_POID,read_profile_iflist, PIN_FLD_OBJECT, ebufp);
	PIN_FLIST_FLD_SET(read_profile_iflist, PIN_FLD_PROFILE_DESCR, "/profile/wholesale", ebufp);
	PIN_FLIST_FLD_SET(read_profile_iflist, PIN_FLD_SEARCH_TYPE, &srch_by_acnt, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read_profile_iflist", read_profile_iflist);
	fm_mso_get_profile_info(ctxp, read_profile_iflist, &read_profile_oflist, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read_profile_oflist", read_profile_oflist);

	old_wholesale_flistp = PIN_FLIST_SUBSTR_GET(read_profile_oflist, MSO_FLD_WHOLESALE_INFO, 1, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "old_wholesale_flistp", old_wholesale_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "wholesale_flistp", wholesale_flistp);
	
	old_customer_type         = PIN_FLIST_FLD_GET(old_wholesale_flistp, PIN_FLD_CUSTOMER_TYPE       , 1, ebufp );
	old_pref_dom              = PIN_FLIST_FLD_GET(old_wholesale_flistp, MSO_FLD_PREF_DOM            , 1, ebufp );
	old_pp_type               = PIN_FLIST_FLD_GET(old_wholesale_flistp, MSO_FLD_PP_TYPE             , 1, ebufp );
	old_erp_control_acct_id   = PIN_FLIST_FLD_GET(old_wholesale_flistp, MSO_FLD_ERP_CONTROL_ACCT_ID , 1, ebufp );
	old_ent_tax_no            = PIN_FLIST_FLD_GET(old_wholesale_flistp, MSO_FLD_ENT_TAX_NO          , 1, ebufp );
	old_pan_no                = PIN_FLIST_FLD_GET(old_wholesale_flistp, MSO_FLD_PAN_NO              , 1, ebufp );
	old_st_reg_no             = PIN_FLIST_FLD_GET(old_wholesale_flistp, MSO_FLD_ST_REG_NO           , 1, ebufp );
	old_vat_tax_no            = PIN_FLIST_FLD_GET(old_wholesale_flistp, MSO_FLD_VAT_TAX_NO          , 1, ebufp );
	old_commision_model       = PIN_FLIST_FLD_GET(old_wholesale_flistp, MSO_FLD_COMMISION_MODEL     , 1, ebufp );
	old_commision_service     = PIN_FLIST_FLD_GET(old_wholesale_flistp, MSO_FLD_COMMISION_SERVICE   , 1, ebufp );
	old_customer_segment      = PIN_FLIST_FLD_GET(old_wholesale_flistp, PIN_FLD_CUSTOMER_SEGMENT    , 1, ebufp );
	old_das_type              = PIN_FLIST_FLD_GET(old_wholesale_flistp, MSO_FLD_DAS_TYPE            , 1, ebufp );
	old_post_reg_no           = PIN_FLIST_FLD_GET(old_wholesale_flistp, MSO_FLD_POSTAL_REG_NO       , 1, ebufp );


	/***********************************************
	Read Existing profile -End
	***********************************************/
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&read_profile_iflist, NULL);	
		PIN_FLIST_DESTROY_EX(&read_profile_oflist, NULL);	
		strcpy(err_msg, "Error in reading Profile");
		ret_status = 0;
		goto CLEANUP;
	}
 
 	/***********************************************
	Compare OLD and NEW values and call update
	***********************************************/
	if (old_customer_type && customer_type && *old_customer_type != *customer_type)
	{
		allow_update = 1;
	}
	else if (old_pref_dom && pref_dom && *old_pref_dom != *pref_dom)
	{
		allow_update = 1;
	}
	else if (old_pp_type && pp_type && *old_pp_type != *pp_type)
	{
		allow_update = 1;
	}
	else if (old_erp_control_acct_id && erp_control_acct_id && *old_erp_control_acct_id != *erp_control_acct_id)
	{
		allow_update = 1;
	}
	else if (old_commision_model && commision_model && *old_commision_model != *commision_model)
	{
		allow_update = 1;
	}
	else if (old_commision_service && commision_service && *old_commision_service != *commision_service)
	{
		allow_update = 1;
	}
	else if (old_customer_segment && customer_segment && *old_customer_segment != *customer_segment)
	{
		allow_update = 1;
	}
	else if (old_ent_tax_no && ent_tax_no && strcmp(old_ent_tax_no, ent_tax_no)!=0)
	{
		allow_update = 1;
	}
	else if (old_pan_no && pan_no && strcmp(old_pan_no, pan_no)!=0)
	{
		allow_update = 1;
	}
	else if (old_st_reg_no && st_reg_no && strcmp(old_st_reg_no, st_reg_no)!=0)
	{
		allow_update = 1;
	}
	else if (old_vat_tax_no && vat_tax_no && strcmp(old_vat_tax_no, vat_tax_no)!=0)
	{
		allow_update = 1;
	}
	if (old_das_type && das_type && strcmp(old_das_type, das_type)!=0)
	{
		allow_update = 1;
	}
	else if (old_post_reg_no && post_reg_no && strcmp(old_post_reg_no, post_reg_no)!=0)
	{
		allow_update = 1;
	}

	if (allow_update)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bu_profile input list", update_profile_iflist);
		PCM_OP(ctxp, PCM_OP_CUST_MODIFY_PROFILE , 0, update_profile_iflist, &update_profile_oflist, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bu_profile profile update output flist", update_profile_oflist);
		
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&update_profile_iflist, NULL);	
			PIN_FLIST_DESTROY_EX(&update_profile_oflist, NULL);	
			strcpy(err_msg, "Error in Updating Profile");
			ret_status = 0;
			goto CLEANUP;
		}

		/***********************************************
		For Lifecycle
		***********************************************/

		vp  = (pin_flist_t*) PIN_FLIST_ELEM_GET(out_flist, PIN_FLD_DATA_ARRAY, 0, 1, ebufp);
		vp1 = (pin_flist_t*)PIN_FLIST_SUBSTR_ADD(vp, MSO_FLD_WHOLESALE_INFO, ebufp); 

		//Copy into PIN_FLD_DATA_ARRAY old values only if passed in input 
		if (pref_dom)
		{
			PIN_FLIST_FLD_COPY(old_wholesale_flistp, MSO_FLD_PREF_DOM ,vp1, MSO_FLD_PREF_DOM, ebufp);
		}
		if (pp_type)
		{
			PIN_FLIST_FLD_COPY(old_wholesale_flistp, MSO_FLD_PP_TYPE ,vp1, MSO_FLD_PP_TYPE, ebufp);
		}
		if (erp_control_acct_id)
		{
			PIN_FLIST_FLD_COPY(old_wholesale_flistp, MSO_FLD_ERP_CONTROL_ACCT_ID ,vp1, MSO_FLD_ERP_CONTROL_ACCT_ID, ebufp);
		}
		if (ent_tax_no)
		{
			PIN_FLIST_FLD_COPY(old_wholesale_flistp, MSO_FLD_ENT_TAX_NO ,vp1, MSO_FLD_ENT_TAX_NO, ebufp);
		}
		if (pan_no)
		{
			PIN_FLIST_FLD_COPY(old_wholesale_flistp, MSO_FLD_PAN_NO ,vp1, MSO_FLD_PAN_NO, ebufp);
		}
		if (st_reg_no)
		{
			PIN_FLIST_FLD_COPY(old_wholesale_flistp, MSO_FLD_ST_REG_NO ,vp1, MSO_FLD_ST_REG_NO, ebufp);
		}
		if (vat_tax_no)
		{
			PIN_FLIST_FLD_COPY(old_wholesale_flistp, MSO_FLD_VAT_TAX_NO ,vp1, MSO_FLD_VAT_TAX_NO, ebufp);
		}
		if (commision_model)
		{
			PIN_FLIST_FLD_COPY(old_wholesale_flistp, MSO_FLD_COMMISION_MODEL ,vp1, MSO_FLD_COMMISION_MODEL, ebufp);
		}
		if (commision_service)
		{
			PIN_FLIST_FLD_COPY(old_wholesale_flistp, MSO_FLD_COMMISION_SERVICE ,vp1, MSO_FLD_COMMISION_SERVICE, ebufp);
		}	
		if (customer_segment)
		{
			PIN_FLIST_FLD_COPY(old_wholesale_flistp, PIN_FLD_CUSTOMER_SEGMENT ,vp1, PIN_FLD_CUSTOMER_SEGMENT, ebufp);
		}	
		if (customer_type)
		{
			PIN_FLIST_FLD_COPY(old_wholesale_flistp, PIN_FLD_CUSTOMER_TYPE ,vp1, PIN_FLD_CUSTOMER_TYPE, ebufp);
		}
		if (das_type)
		{
			PIN_FLIST_FLD_COPY(old_wholesale_flistp, MSO_FLD_DAS_TYPE ,vp1, MSO_FLD_DAS_TYPE, ebufp);
		}
		if (post_reg_no)
		{
			PIN_FLIST_FLD_COPY(old_wholesale_flistp, MSO_FLD_POSTAL_REG_NO ,vp1, MSO_FLD_POSTAL_REG_NO, ebufp);
		}
		vp = (pin_flist_t*) PIN_FLIST_ELEM_GET(out_flist, PIN_FLD_DATA_ARRAY, 1, 1, ebufp);
		PIN_FLIST_SUBSTR_SET(vp, wholesale_flistp, MSO_FLD_WHOLESALE_INFO, ebufp);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "out_flist", out_flist);
		fm_mso_create_lifecycle_evt(ctxp, out_flist, in_flist, ID_MODIFY_PROFILE_INFO_FOR_BUSINESS_UNIT, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
			PIN_ERRBUF_RESET(ebufp);
		}
	}
	else
	{
		strcpy(err_msg, "Update Not required, old and new values are same");
		ret_status = 3;
		goto CLEANUP;
	}


CLEANUP:
	PIN_FLIST_FLD_SET(out_flist, PIN_FLD_DESCR, err_msg, ebufp );
	PIN_FLIST_DESTROY_EX(&update_profile_iflist, NULL);
	PIN_FLIST_DESTROY_EX(&update_profile_oflist, NULL);
	PIN_FLIST_DESTROY_EX(&read_profile_iflist, NULL);
	PIN_FLIST_DESTROY_EX(&read_profile_oflist, NULL);
	return ret_status;
}



int32
fm_mso_get_cust_of_lco_count(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	int32			*r_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*search_outflistp = NULL;
	pin_flist_t		*search_inflistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*cc_info = NULL;

	poid_t			*lco_obj = NULL;

	int32			flags = 256;
	int32			retval =0;
//	int32			elemid = 0;

	int64			db = 1;

	void			*vp = NULL;
	
	char			*template = "select count( PROFILE_T.POID_ID0 ) from /profile/customer_care where F1.id = V1 and F2.type = V2  " ;
	char			buff[255]="";

	pin_cookie_t		cookie = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_impacted_cust_count", ebufp);
		return 0 ;
	}
	
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_cust_of_lco_count search flist :", in_flist);
	lco_obj  = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp );

	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, template, ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &flags, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
	cc_info  = PIN_FLIST_SUBSTR_ADD(arg_flist, PIN_FLD_CUSTOMER_CARE_INFO, ebufp);
	PIN_FLIST_FLD_SET(cc_info, MSO_FLD_LCO_OBJ, lco_obj, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/profile/customer_care", -1, ebufp), ebufp);

	PIN_FLIST_ELEM_SET(search_inflistp, NULL, PIN_FLD_RESULTS, 0, ebufp);
//	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_AMOUNT, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_cust_of_lco_count search input flist :", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_COUNT_ONLY, search_inflistp, &search_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_cust_of_lco_count search output flist :", search_outflistp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_cust_of_lco_count search creation error", ebufp);
		PIN_ERRBUF_RESET(ebufp);
	}

	(void)PIN_FLIST_ELEM_GET_NEXT(search_outflistp, PIN_FLD_RESULTS, &retval, 0, &cookie, ebufp);
	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);

	sprintf(buff, "impacted customers = %d", retval );
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, buff );

	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);

	return retval;
}

static int
fm_mso_bus_type_change(
    	pcm_context_t           *ctxp,
   	int64	                *bus_type,
	int64	                *old_bus_type,
    	pin_flist_t             *in_flistp,
	pin_flist_t             *out_flistp,
    	pin_errbuf_t            *ebufp)
{
	pin_flist_t 		*write_flistp = NULL;
	pin_flist_t 		*write_res_flistp = NULL;
	poid_t			*account_obj = NULL;
	void			*vp = NULL;
	void			*vp1 = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_bus_type_change : Error before calling fm_mso_bus_type_change", ebufp);
		return 3;
	}

	account_obj = PIN_FLIST_FLD_GET( in_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Updating Business Type");
	if ( !bus_type )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No Business Type provided in input");
		return 1;	
	}
	
	if (*bus_type == *old_bus_type){
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Old and New Same Business Types are same");
		return 0;
	}
	
	write_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(write_flistp, PIN_FLD_POID, account_obj, ebufp);
	PIN_FLIST_FLD_SET(write_flistp, PIN_FLD_BUSINESS_TYPE, bus_type , ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bus_type_change write flist", write_flistp);
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_flistp, &write_res_flistp, ebufp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in write flds of business type for account", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&write_res_flistp, NULL);
		return 3;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bus_type_change business type update success", write_res_flistp);
	vp  = (pin_flist_t*)PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_DATA_ARRAY, 0, 1, ebufp);
	
	vp1 = (pin_flist_t*)PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_DATA_ARRAY, 1, 1, ebufp); 
	if (vp)
	{
		PIN_FLIST_FLD_SET(vp, PIN_FLD_BUSINESS_TYPE, old_bus_type, ebufp);
	}
	if (vp1)
	{
		PIN_FLIST_FLD_SET(vp1, PIN_FLD_BUSINESS_TYPE, bus_type, ebufp);
	}
	
	PIN_FLIST_DESTROY_EX(&write_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&write_res_flistp, NULL);

	return 2;
}

static int32
fm_mso_get_serv_type(
        pcm_context_t           *ctxp,
        poid_t                  *acct_pdp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *srch_flistp = NULL;
        pin_flist_t             *srch_oflistp = NULL;
        pin_flist_t             *results_flistp = NULL;
        pin_flist_t             *args_flistp = NULL;

        poid_t                  *srch_pdp = NULL;
        poid_t                  *serv_pdp = NULL;

        int64                   db=1;
        int32                   flags = 256;
        int32                   elem_id = 0;
        int32                   serv_type = 1;
        pin_cookie_t            cookie = NULL;

        char                    *template = " select X from /service where F1 = V1 ";
	void			*vp = NULL;
        if (PIN_ERRBUF_IS_ERR(ebufp))
           return;

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_serv_type" );

        db = PIN_POID_GET_DB(acct_pdp);

		
        srch_pdp = PIN_POID_CREATE(db, "/search", (int64)-1, ebufp);
        srch_flistp = PIN_FLIST_CREATE(ebufp);

        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &flags, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ,acct_pdp, ebufp);

        results_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY ,ebufp);
        PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID ,NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_services input list", srch_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH,0,srch_flistp,&srch_oflistp, ebufp);


        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "fm_mso_get_services:Error in calling PCM_OP_SEARCH service");
                goto CLEANUP;
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_services search output flist", srch_oflistp);

        while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_oflistp, PIN_FLD_RESULTS,
                &elem_id, 1, &cookie, ebufp)) != NULL)
        {
                vp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_POID, 1, ebufp);
                if (vp)
                        serv_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_POID, 1, ebufp);

                if (serv_pdp && strcmp((char*)PIN_POID_GET_TYPE(serv_pdp),MSO_CATV)==0 ){
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"CATV service  Type");
                        serv_type = 0;
		}
                else{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"BB service  Type");
                        serv_type = 1;
		}
        }

        CLEANUP:
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
        return serv_type;
}

static int
fm_mso_cust_cmts_ip_change(
	pcm_context_t		*ctxp,
	poid_t			*account_obj,
	poid_t			*service_obj,
	poid_t			*mso_obj,
	char			*ne,
	char			*old_ne,
	char			*prog_name,
	char			**login,
	pin_errbuf_t		*ebufp)
{

	
	pin_flist_t		*read_in_flistp = NULL;
	pin_flist_t		*read_out_flistp = NULL;
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*cc_flistp = NULL;
	pin_flist_t		*prod_flistp = NULL;
	pin_flist_t		*static_in_flistp = NULL;
	poid_t			*plan_obj = NULL;
	char			*p_tag = NULL;
	char			*old_pool_name = NULL;
	char			*new_pool_name = NULL;
	char			prov_tag[255];
	char			*ip_add_list = NULL;
	int32			elem_id=0;
	int32			flags=512;
	int32			ip_cc_id = IP_POOL;
	int32			ip_change_req = 0;
	int32			ctr = 0;
	int64			db = 0;
	int32			ret_val = -1;
	int32			count = 0;
	pin_cookie_t		cookie = NULL;
	char			template1[255] = "select X from /mso_cfg_cmts_cc_mapping where F1 = V1 and F2 = V2 and F3 = V3 ";

	pin_flist_t		*ass_dev_flistp = NULL;
	int			*error_codep = NULL;

	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_cust_cmts_ip_change", ebufp);
		return 0;
	}
	memset(prov_tag, '\0', sizeof(prov_tag));
	db = PIN_POID_GET_DB(account_obj);
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Acc Poid :", account_obj);
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "MSO PP obj :", mso_obj);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, ne);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, old_ne);

	// Read MSO PP object
	read_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_in_flistp,PIN_FLD_POID, mso_obj, ebufp );  
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0,read_in_flistp, &read_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read MSO PP object output flist", read_out_flistp);
	PIN_FLIST_DESTROY_EX(&read_in_flistp, NULL);
	
	plan_obj = PIN_FLIST_FLD_TAKE(read_out_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp );
	// Loop through the product array in input flist to find the prov tag
	while ( (prod_flistp = PIN_FLIST_ELEM_GET_NEXT (read_out_flistp, 
				PIN_FLD_PRODUCTS, &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL )
	{
		p_tag = (char *)PIN_FLIST_FLD_GET(prod_flistp, PIN_FLD_PROVISIONING_TAG, 1, ebufp );
		if(p_tag && strlen(p_tag)>0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, p_tag);
			strcpy(prov_tag,p_tag);
			break;
		}
	}
	PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);

	//Fetch the IP pool name for Old CMTS
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template1, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &flags, ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_CMTS_ID, old_ne, ebufp );
	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 2, ebufp);
	cc_flistp = PIN_FLIST_ELEM_ADD(arg_flistp, MSO_FLD_CC_MAPPING, 0, ebufp);
	PIN_FLIST_FLD_SET(cc_flistp, MSO_FLD_CLIENT_CLASS_ID, &ip_cc_id, ebufp );
	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 3, ebufp);
	cc_flistp = PIN_FLIST_ELEM_ADD(arg_flistp, MSO_FLD_CC_MAPPING, 0, ebufp);
	PIN_FLIST_FLD_SET(cc_flistp, PIN_FLD_PROVISIONING_TAG, prov_tag, ebufp );
	PIN_FLIST_ELEM_SET(srch_in_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Old CMTS CC map input flist :", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Old CMTS CC map output flist :", srch_out_flistp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_cust_cmts_ip_change search CMTS error", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		if(!PIN_POID_IS_NULL(plan_obj)) PIN_POID_DESTROY(plan_obj,ebufp);
		return 0;
	}
	if(PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp) > 1)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CMTS search return more than 1 result. Invalid Configuration");
		if(!PIN_POID_IS_NULL(plan_obj)) PIN_POID_DESTROY(plan_obj,ebufp);
		return 0;
	}
	if(PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp) == 0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CMTS search return 0 result. Invalid Configuration");
		if(!PIN_POID_IS_NULL(plan_obj)) PIN_POID_DESTROY(plan_obj,ebufp);
		return 0;
	}
	result_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
	cc_flistp = PIN_FLIST_ELEM_GET(result_flistp, MSO_FLD_CC_MAPPING, PIN_ELEMID_ANY, 0, ebufp);
	old_pool_name = PIN_FLIST_FLD_TAKE(cc_flistp, MSO_FLD_CLIENT_CLASS_VALUE, 0, ebufp );
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);

	// Fetch the IP pool name for New CMTS
	// Use the same inpput flist used inn above search.
	arg_flistp = PIN_FLIST_ELEM_GET(srch_in_flistp, PIN_FLD_ARGS, 1, 0, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_CMTS_ID, ne, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search New CMTS CC map input flist :", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search New CMTS CC map output flist :", srch_out_flistp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_cust_cmts_ip_change search CMTS error", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		if(!PIN_POID_IS_NULL(plan_obj)) PIN_POID_DESTROY(plan_obj,ebufp);
		return 0;
	}
	if(PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp) > 1)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CMTS search return more than 1 result. Invalid Configuration");
		if(!PIN_POID_IS_NULL(plan_obj)) PIN_POID_DESTROY(plan_obj,ebufp);
		return 0;
	}
	if(PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp) == 0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CMTS search return 0 result. Invalid Configuration");
		if(!PIN_POID_IS_NULL(plan_obj)) PIN_POID_DESTROY(plan_obj,ebufp);
		return 0;
	}
	result_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
	cc_flistp = PIN_FLIST_ELEM_GET(result_flistp, MSO_FLD_CC_MAPPING, PIN_ELEMID_ANY, 0, ebufp);
	new_pool_name = PIN_FLIST_FLD_TAKE(cc_flistp, MSO_FLD_CLIENT_CLASS_VALUE, 0, ebufp );
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);

	// Compare the old and new pool name
	if(old_pool_name && new_pool_name)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, old_pool_name);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, new_pool_name);
		if(strcmp(new_pool_name,old_pool_name)==0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Old and new pool name are same. No need to change IP.");
		
			/****** Pavan Bellala 03-09-2015 ***************
			Added function call to set the old ip address
			***********************************************/	
			fm_mso_search_devices(ctxp,MSO_OBJ_TYPE_DEVICE_IP_STATIC,service_obj,&ass_dev_flistp, error_codep, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_cust_cmts_ip_change:"
									"Error in searching Static IP for service", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				if(!PIN_POID_IS_NULL(plan_obj)) PIN_POID_DESTROY(plan_obj,ebufp);
				return 0;
			}
			result_flistp = PIN_FLIST_ELEM_GET(ass_dev_flistp,PIN_FLD_RESULTS,PIN_ELEMID_ANY, 0, ebufp);
			*login = PIN_FLIST_FLD_TAKE(result_flistp,PIN_FLD_DEVICE_ID,1,ebufp);
			PIN_FLIST_DESTROY_EX(&ass_dev_flistp,NULL);
			if(error_codep) free(error_codep);
			return 2;
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Old and new pool name different. Change IP");
			ip_change_req = 1;
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "IP Pool name is NULL for CMTS");
		if(!PIN_POID_IS_NULL(plan_obj)) PIN_POID_DESTROY(plan_obj,ebufp);
		return 0;
	}

	//IP pool is different so Static IP must be changed.
	if(ip_change_req)
	{
		ret_val = fm_mso_cust_release_service_ip(ctxp, account_obj, service_obj, prog_name, &count, &ip_add_list, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "IP Add List");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, ip_add_list);
		*login = ip_add_list;
		if(ret_val == 0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Error in releasing existing IP.");
			return 0;
		}
		// Purchase the same number of Static IPs which service has.
		for(ctr=0;ctr<count;ctr++)
		{
			srch_in_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
			PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
			PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_PROGRAM_NAME, prog_name, ebufp);
			/**** Pavan Bellala 04-09-2015 *********************
			Added scenario field to identify that the it is 			
			CMTS change scenario
			*****************************************************/
			PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_SCENARIO_NAME, "CHANGE CMTS", ebufp);
			PIN_FLIST_FLD_SET(srch_in_flistp, MSO_FLD_PURCHASED_PLAN_OBJ,mso_obj,ebufp);
			PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_PLAN_OBJ,plan_obj,ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchase Static IP input flist",srch_in_flistp);
			ret_val = fm_mso_purchase_static_ip(ctxp, srch_in_flistp, MSO_ADD_DEVICE, ebufp );
			if ( ret_val == 1 ||  ret_val == 2)
			{
				return 0;
			}
			else if ( ret_val == 0 )
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Static IP device associated successfully");
			}
			PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
		}
	}
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	//if(plan_obj) free(plan_obj);

	return 1;
}

/***********************************************
* Function : fm_mso_cust_release_service_ip()
*	This function disassociates all the
*	static IP associated to the service 
*	poid passed in input.
***********************************************/
int
fm_mso_cust_release_service_ip(
	pcm_context_t		*ctxp,
	poid_t			*account_obj,
	poid_t			*service_obj,
	char			*prog_name,
	int			*count,
	char			**login,
	pin_errbuf_t		*ebufp)

{

	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*dev_flistp = NULL;
	pin_flist_t		*device_flistp = NULL;
	pin_flist_t		*device_out_flistp = NULL;
	pin_flist_t		*del_flistp = NULL;
	pin_flist_t		*del_out_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_cookie_t		cookie = NULL;
	int			elem_id=0;
	int			ctr=0;
	int			flag = 0;
	int                     disas_flag = DEVICE_DISSASSOCIATE;
	int			*error_codep=NULL;
	char			dev_type[255];
	char			*ip_add = NULL;
	char			*ip_add_list = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_cust_release_service_ip", ebufp);
		return 0;
	}
	memset(dev_type, '\0', sizeof(dev_type));
	strcpy(dev_type,"/device/ip/static");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "calling Static IP device search");
	fm_mso_search_devices(ctxp,dev_type,service_obj,&res_flistp, error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in searching Static IP for service", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		return 0;
	}
	cookie = NULL;
	elem_id = 0;
	while ( (dev_flistp = PIN_FLIST_ELEM_GET_NEXT (res_flistp, 
		PIN_FLD_RESULTS, &elem_id, 1,&cookie, ebufp))
						  != (pin_flist_t *)NULL )
	{
		device_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(dev_flistp, PIN_FLD_POID, device_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(device_flistp, PIN_FLD_PROGRAM_NAME, prog_name, ebufp);
		PIN_FLIST_FLD_SET(device_flistp, PIN_FLD_FLAGS, &disas_flag, ebufp);
		arg_flistp = PIN_FLIST_ELEM_ADD(device_flistp, PIN_FLD_SERVICES, 0,ebufp);
		PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACCOUNT_OBJ, (void *) account_obj, ebufp);
		PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_SERVICE_OBJ, (void *) service_obj, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
						"Static IP disassociate input flist ", device_flistp);
		PCM_OP(ctxp, PCM_OP_DEVICE_ASSOCIATE, 0, device_flistp, &device_out_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
						"Static IP disassociate output flist", device_out_flistp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in static ip disassociate", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&device_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&device_out_flistp, NULL);
			return 0;
		}
		PIN_FLIST_DESTROY_EX(&device_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&device_out_flistp, NULL);
	
		tmp_flistp = PIN_FLIST_ELEM_GET(dev_flistp, PIN_FLD_PLAN, 0, 1,ebufp);
		if ( tmp_flistp )
		{
			del_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(dev_flistp, PIN_FLD_POID, del_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_ELEM_COPY(dev_flistp, PIN_FLD_PLAN, 0, del_flistp, PIN_FLD_PLAN, 0, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
							"delete plan input flist ", del_flistp);
			PCM_OP(ctxp, PCM_OP_DELETE_FLDS, 0, del_flistp, &del_out_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
							"delete plan output flist", del_out_flistp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in deleting plan from device", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&del_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&del_out_flistp, NULL);
				return 0;
			}
			PIN_FLIST_DESTROY_EX(&del_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&del_out_flistp, NULL);
		}
		//Keep the old IP address to pass into provisioning flist.
		ip_add = PIN_FLIST_FLD_GET(dev_flistp, PIN_FLD_DEVICE_ID, 0, ebufp);
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
		//Count of static IPs
		ctr++;
	}
	*login = ip_add_list;
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ip_add_list: ");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, ip_add_list);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, *login);
	PIN_FLIST_DESTROY_EX(&res_flistp, NULL);
	*count = ctr;
	return 1;
}

static int
fm_mso_cust_dup_cntinfo(
	pcm_context_t		*ctxp,
	pin_flist_t		*cnt_flistp,
	pin_flist_t		*w_in_flistp,
	char			*new_val,
	time_t			current_t,
	pin_errbuf_t		*ebufp	)

{
	pin_flist_t		*tmp_flistp;
	time_t			*eff_t = NULL;
	char			*value = NULL;
	int32			elem_id = 0;
	pin_cookie_t		cookie = NULL;
	
	if(cnt_flistp)
	{
		while ( (tmp_flistp = PIN_FLIST_ELEM_GET_NEXT (cnt_flistp, 
			MSO_FLD_CONTACT_LIST, &elem_id, 1,&cookie, ebufp))
							  != (pin_flist_t *)NULL )
		{
			value = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_CONTACT_VALUE, 1,ebufp);
			eff_t = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_EFFECTIVE_T, 1,ebufp);
			if(value && eff_t)
			{
				if(strcmp(new_val,value)==0 && current_t==*eff_t)
					return 0;
			}
		}
	}
	cookie = NULL;
	while ( (tmp_flistp = PIN_FLIST_ELEM_GET_NEXT (w_in_flistp, 
		MSO_FLD_CONTACT_LIST, &elem_id, 1,&cookie, ebufp))
						  != (pin_flist_t *)NULL )
	{
		value = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_CONTACT_VALUE, 1,ebufp);
		eff_t = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_EFFECTIVE_T, 1,ebufp);
		if(value && eff_t)
		{
			if(strcmp(new_val,value)==0 && current_t==*eff_t)
				return 0;
		}
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Not matched.");
	return 1;
}


/**************************************************
* Function: fm_mso_modify_prod_discount_info()
* 	Updates the discount information for
*	purchased products.
*
***************************************************/
static int 
fm_mso_modify_prod_discount_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		*out_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*ser_flistp = NULL;
	pin_flist_t		*prod_flistp = NULL;
	pin_flist_t		*prod_out_flistp = NULL;
	pin_flist_t		*plan_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*new_ser_flistp = NULL;
	pin_flist_t		*old_ser_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*read_in_flistp = NULL;
	pin_flist_t		*read_out_flistp = NULL;

	poid_t			*account_obj = NULL;
	poid_t			*service_obj = NULL;
	poid_t			*srch_pdp = NULL;
	pin_decimal_t		*new_disc_per = NULL;
	pin_decimal_t		*disc_per = NULL;
	pin_decimal_t		*new_disc_amt = NULL;
	pin_decimal_t		*disc_amt = NULL;
	pin_decimal_t		*zero = NULL;

	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie1 = NULL;
	int32			rec_id = 0;
	int32			rec_id1 = 0;
	int32			srch_flag = 512;
	int32			srch_type = -1;
	int32			update_flag = 0;
	int32			SUBS_PLAN_FLAG = 0;
	char			*template = "select X from /purchased_product where F1 = V1 and F2 = V2 and F3 = V3 ";
	char			*template1 = "select X from /mso_purchased_plan where F1 = V1 and F2 = V2 ";
	void			*vp = NULL;
	int64			db = -1;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;
	zero = pbo_decimal_from_str("0", ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_prod_discount_info: input flist", in_flist);
	account_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	ser_flistp = PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_SERVICES, PIN_ELEMID_ANY, 1, ebufp );
	if ((PIN_ERRBUF_IS_ERR(ebufp)) || (!ser_flistp))
	{
		PIN_ERRBUF_RESET(ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No Service Array. Purchased plan update not required.");
		return 1;
	}
	service_obj = PIN_FLIST_FLD_GET(ser_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	if (!service_obj || PIN_FLIST_ELEM_COUNT(ser_flistp, PIN_FLD_PLAN, ebufp) == 0 )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Purchased plan update not required.");
		return 1;
	}

	tmp_flistp = PIN_FLIST_ELEM_GET(out_flist, PIN_FLD_DATA_ARRAY, 0, 0, ebufp);
	old_ser_flistp = PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_SERVICES, 0, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_GET(out_flist, PIN_FLD_DATA_ARRAY, 1, 0, ebufp);
	new_ser_flistp = PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_SERVICES, 0, ebufp);
	
	while ( (plan_flistp = PIN_FLIST_ELEM_GET_NEXT (ser_flistp, 
				PIN_FLD_PLAN, &rec_id, 1,&cookie, ebufp)) != (pin_flist_t *)NULL )
	{

		/* Fetch new discount values */
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Plan array", plan_flistp);
		new_disc_per = NULL;
		new_disc_amt = NULL;
		new_disc_per = PIN_FLIST_FLD_GET(plan_flistp, MSO_FLD_DISC_PERCENT, 1, ebufp);
		new_disc_amt = PIN_FLIST_FLD_GET(plan_flistp, MSO_FLD_DISC_AMOUNT, 1, ebufp);
		if( (!new_disc_per || pbo_decimal_is_null(new_disc_per, ebufp)) 
			&& (!new_disc_amt || pbo_decimal_is_null(new_disc_amt, ebufp)) )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Discount percent and amount are NULL.");
			continue;
		}

		/* Fetch the mso_purchased_plan for every plan in input*/
/*		db = PIN_POID_GET_DB(account_obj);
		srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
		srch_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template1, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_COPY(plan_flistp,PIN_FLD_PLAN_OBJ, arg_flist, PIN_FLD_PLAN_OBJ, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_DESCR, NULL, ebufp);
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_PRIORITY, NULL, ebufp);


		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search MSO PP search input flist", srch_flistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			return 0;
		}
		if (PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp) == 0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "No purchased plan found for given plan");
			return 0;
		}
		arg_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY,0, ebufp );
		vp = NULL;
		vp = PIN_FLIST_FLD_GET(arg_flist,PIN_FLD_DESCR, 0, ebufp);
		if (vp && strcmp(vp,"base plan")==0)
		{
			SUBS_PLAN_FLAG = 1;
		}
		*/

		/* Fetch the Purchased product for every plan in input*/
		db = PIN_POID_GET_DB(account_obj);
		srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
		srch_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_COPY(plan_flistp,PIN_FLD_DEAL_OBJ, arg_flist, PIN_FLD_DEAL_OBJ, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
		PIN_FLIST_FLD_COPY(plan_flistp,PIN_FLD_PACKAGE_ID, arg_flist, PIN_FLD_PACKAGE_ID, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_CYCLE_DISCOUNT, NULL, ebufp);
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_CYCLE_DISC_AMT, NULL, ebufp);
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_PURCHASE_DISCOUNT, NULL, ebufp);
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_PURCHASE_DISC_AMT, NULL, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_prod_discount_info search input list", srch_flistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &ret_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			return 0;
		}
		if (PIN_FLIST_ELEM_COUNT(ret_flistp, PIN_FLD_RESULTS, ebufp) == 0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "No purchased product found for given deal and package id.");
			return 0;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_prod_discount_info search output list", ret_flistp);
		cookie1 = NULL;
		while ( (prod_flistp = PIN_FLIST_ELEM_GET_NEXT (ret_flistp, 
					PIN_FLD_RESULTS, &rec_id1, 1,&cookie1, ebufp)) != (pin_flist_t *)NULL )
		{

			update_flag = 0;
			// Checking only PIN_FLD_CYCLE_XXX values since we store discount 
			//	in both CYCLE and PURCHASE fields.
			disc_per = PIN_FLIST_FLD_GET(prod_flistp, PIN_FLD_CYCLE_DISCOUNT, 1, ebufp);
			disc_amt = PIN_FLIST_FLD_GET(prod_flistp, PIN_FLD_CYCLE_DISC_AMT, 1, ebufp);

			//If existing product has discount then update the discount value
			if(disc_per && !pbo_decimal_is_null(disc_per, ebufp) && !pbo_decimal_is_zero(disc_per, ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Discount Percent exists.");
				PIN_FLIST_FLD_COPY(prod_flistp,PIN_FLD_CYCLE_DISCOUNT, old_ser_flistp, MSO_FLD_DISC_PERCENT, ebufp);
				update_flag = 1;
			}
			if(disc_amt && !pbo_decimal_is_null(disc_amt, ebufp) && !pbo_decimal_is_zero(disc_amt, ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Discount amount exists.");
				PIN_FLIST_FLD_COPY(prod_flistp,PIN_FLD_CYCLE_DISC_AMT, old_ser_flistp, MSO_FLD_DISC_AMOUNT, ebufp);
				update_flag = 1;
			}
			// This is the scenario when purchased product does not have discount (zero value)
			// and now the discount value is updated (to > zero) to provide the discount.
			if(!update_flag)
			{
				read_in_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(prod_flistp,PIN_FLD_PRODUCT_OBJ, read_in_flistp, PIN_FLD_POID, ebufp);
				arg_flist = PIN_FLIST_ELEM_ADD(read_in_flistp, PIN_FLD_USAGE_MAP, 0, ebufp );
				PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_EVENT_TYPE, NULL, ebufp);

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Product read field input flist", read_in_flistp);
				PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_in_flistp, &read_out_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&read_in_flistp, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS product", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);
					return 0;
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Product read field input flist", read_out_flistp);
				arg_flist = PIN_FLIST_ELEM_GET(read_out_flistp, PIN_FLD_USAGE_MAP, PIN_ELEMID_ANY,0, ebufp );
				vp = NULL;
				vp = PIN_FLIST_FLD_GET(arg_flist,PIN_FLD_EVENT_TYPE, 0, ebufp);
				if(vp && strstr(vp,"mso_grant"))
				{
					continue;
				}
				if(vp && strstr(vp,"/event/billing/product/fee/cycle/cycle_forward"))
				{
					update_flag = 1;
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Setting zero in data array");
					PIN_FLIST_FLD_SET(old_ser_flistp, MSO_FLD_DISC_PERCENT, zero, ebufp);
				}
				PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);
			}

			//If update flag is 1 then update the purchased product with discount details.
			if(update_flag)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Updating Discount");
				srch_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_ACCOUNT_OBJ, srch_flistp, PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_PROGRAM_NAME, srch_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
				arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_PRODUCTS, 0, ebufp );
				PIN_FLIST_FLD_COPY(prod_flistp,PIN_FLD_POID, arg_flist, PIN_FLD_OFFERING_OBJ, ebufp);
				PIN_FLIST_FLD_COPY(prod_flistp,PIN_FLD_PRODUCT_OBJ, arg_flist, PIN_FLD_PRODUCT_OBJ, ebufp);
				if(new_disc_per) 
				{
					// Update discount percent to new value pased in input
					// and set discount amount values to zero
					PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_CYCLE_DISCOUNT, new_disc_per, ebufp);
					PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_PURCHASE_DISCOUNT, new_disc_per, ebufp);
					PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_USAGE_DISCOUNT, new_disc_per, ebufp);
					PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_CYCLE_DISC_AMT, zero, ebufp);
					PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_PURCHASE_DISC_AMT, zero, ebufp);
					//Set new discount amount in DATA_ARRAY 
					PIN_FLIST_FLD_COPY(plan_flistp,MSO_FLD_DISC_PERCENT, new_ser_flistp, MSO_FLD_DISC_PERCENT, ebufp);
				}
				else if(new_disc_amt) 
				{
					// Update discount amount to new value pased in input 
					// and set discount percent values to zero
					PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_CYCLE_DISCOUNT, zero, ebufp);
					PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_PURCHASE_DISCOUNT, zero, ebufp);
					PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_USAGE_DISCOUNT, zero, ebufp);
					PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_CYCLE_DISC_AMT, new_disc_amt, ebufp);
					PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_PURCHASE_DISC_AMT, new_disc_amt, ebufp);
					//Set new discount amount in DATA_ARRAY 
					PIN_FLIST_FLD_COPY(plan_flistp,MSO_FLD_DISC_AMOUNT, new_ser_flistp, MSO_FLD_DISC_AMOUNT, ebufp);
				}

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_SUBSCRIPTION_SET_PRODINFO input", srch_flistp);
				PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_SET_PRODINFO, 0, srch_flistp, &prod_out_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SUBSCRIPTION_SET_PRODINFO", ebufp);
						PIN_ERRBUF_RESET(ebufp);
						PIN_FLIST_DESTROY_EX(&prod_out_flistp, NULL);
						return 0;
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_SUBSCRIPTION_SET_PRODINFO output", prod_out_flistp);
				PIN_FLIST_DESTROY_EX(&prod_out_flistp, NULL);
			}
		}
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_prod_discount_info output flist", out_flist);
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
	return 2;
}


static int
fm_mso_subs_type_change(
        pcm_context_t		*ctxp,
        int64			*bus_type,
        pin_flist_t		*in_flist,
	pin_flist_t		*out_flistp,
        pin_errbuf_t		*ebufp)
{
	pin_flist_t		*accnt_info = NULL;
	pin_flist_t		*write_flistp = NULL;
	pin_flist_t		*write_res_flistp = NULL;
	int32			subscriber_type   = -1;
	int64			*old_bus_type   = NULL;
	int32			account_type   = -1;
	int32			account_model   = -1;
	int32			new_subscriber_type   = -1;
	int32			new_account_type   = -1;
	int32			new_account_model   = -1;
	poid_t			*account_obj = NULL;
	void			*vp = NULL;
	void			*vp1 = NULL;
	//char                    bus_type_str[8];
	//char                    old_bus_str[8];

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"Error before calling fm_mso_subs_type_change", ebufp);
		return 3;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_subs_type_change", in_flist);

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Updating Subscriber Type");
	account_obj = PIN_FLIST_FLD_GET( in_flist, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	if ( !bus_type )
		return 1;	
	fm_mso_get_account_info(ctxp, account_obj, &accnt_info, ebufp);
	old_bus_type = PIN_FLIST_FLD_GET(accnt_info, PIN_FLD_BUSINESS_TYPE, 0, ebufp);	

	fm_mso_get_business_type_values(ctxp, *(int32 *)old_bus_type, &account_type, &account_model, &subscriber_type, ebufp ); 
	fm_mso_get_business_type_values(ctxp, *(int32 *)bus_type,   &new_account_type, &new_account_model, &new_subscriber_type, ebufp ); 
	
	if ( new_account_type != account_type || new_account_model != account_model)
	{
		// If old and new Account Type or account model is Not Same then return from here.
		// Subscriber type will be changed with those values
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Old and new account type or model is not same.");
		return 0;
	}
	if ( new_subscriber_type == subscriber_type )
	{
		// Old and new Subscriber type is same. No need to update
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Old and new subscriber type are same.");
		return 0;
	}

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "Error in validating account type change", ebufp);
                return 3;
        }

	write_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(write_flistp, PIN_FLD_POID, account_obj, ebufp);
	PIN_FLIST_FLD_SET(write_flistp, PIN_FLD_BUSINESS_TYPE, bus_type , ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_subs_type_change write flist", write_flistp);
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_flistp, &write_res_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&write_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in write flds of business type for account", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&write_res_flistp, NULL);	
		return 3;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_subs_type_change business type update successful", write_res_flistp);
	PIN_FLIST_DESTROY_EX(&write_res_flistp, NULL);

	vp  = (pin_flist_t*)PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_DATA_ARRAY, 0, 1, ebufp);
	vp1 = (pin_flist_t*)PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_DATA_ARRAY, 1, 1, ebufp); 

	if (vp)
	{
		PIN_FLIST_FLD_SET(vp, PIN_FLD_BUSINESS_TYPE, old_bus_type, ebufp);
	}
	if (vp1)
	{
		PIN_FLIST_FLD_SET(vp1, PIN_FLD_BUSINESS_TYPE, bus_type, ebufp);
	}
	if((subscriber_type == 30 || new_subscriber_type == 30) && subscriber_type != new_subscriber_type)
	{
		//update Region Key and generate provisioning order for the same
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Creating Provisioning Action for Hotel");
		fm_mso_update_region_key(ctxp, in_flist, ebufp);
        	if (PIN_ERRBUF_IS_ERR(ebufp))
        	{
               	 	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        	"Error in updating region key change", ebufp);
			return 3;
        	}

	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No Need of Provisioning Action for Hotel");
	}	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Out Flistp", out_flistp); 

	return 1;
} 

static void
fm_mso_update_region_key(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flist,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *readacct_inflistp = NULL;
        pin_flist_t             *readacct_outflistp = NULL;
        pin_flist_t             *args_flistp = NULL;

	pin_flist_t             *provaction_inflistp = NULL;
        pin_flist_t             *provaction_outflistp = NULL;
        pin_flist_t             *search_inflistp = NULL;
        pin_flist_t             *search_outflistp = NULL;
        pin_flist_t             *svcwrite_inflistp = NULL;
        pin_flist_t             *svcwrite_outflistp = NULL;
        pin_flist_t             *results_flistp = NULL;
        pin_flist_t             *catv_info_flistp = NULL;
	pin_flist_t             *nameinfo_flistp = NULL;
	pin_flist_t             *catv_flistp = NULL;

        int32                   elem_id = 0;
        int32                   status_flags = 768;
        int32                   status = PIN_STATUS_ACTIVE;
        int32                   status_inactive = PIN_STATUS_INACTIVE;
        int64                   db = 0;
        char                    *zip_code = NULL;
        char                    search_template[150] = " select X from /service where F1 = V1 and ( F2 = V2 or F3 = V3 ) and service_t.poid_type = '/service/catv' ";
        char                    region_key[20];
        char                    *network_node = NULL;

        poid_t                  *srch_pdp = NULL;
        pin_cookie_t            cookie = NULL;
        int32                   business_type = -1;
        char                    bus_type_str[8];

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_region_key start:", in_flist);
       	readacct_inflistp = PIN_FLIST_CREATE(ebufp);
       	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, readacct_inflistp, PIN_FLD_POID, ebufp);

       	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
       	                      "fm_mso_update_region_key : Account read account input list", readacct_inflistp);
       	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, readacct_inflistp, &readacct_outflistp, ebufp);
       	if (PIN_ERRBUF_IS_ERR(ebufp))
       	{
       	 	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
               		               "Error in accessing account or the account does not exist", ebufp);
               	PIN_ERRBUF_RESET(ebufp);
              	goto CLEANUP;
      	}
       	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
       		                "Account read account output flist", readacct_outflistp);
       	if(readacct_outflistp)
     	{ 
       		business_type = *(int32 *)PIN_FLIST_FLD_GET(readacct_outflistp, PIN_FLD_BUSINESS_TYPE, 1, ebufp);
             	nameinfo_flistp = PIN_FLIST_ELEM_GET(readacct_outflistp, PIN_FLD_NAMEINFO, 1, 1, ebufp);
              	if (nameinfo_flistp)
                	zip_code = PIN_FLIST_FLD_GET(nameinfo_flistp, PIN_FLD_ZIP, 1, ebufp);
   	}
	else
	{
		goto CLEANUP;
	}
       	sprintf(bus_type_str, "%d", business_type);
       	if ( bus_type_str && strncmp(bus_type_str, "9930", strlen("9930")) == 0)
   	{
       		PIN_ERR_LOG_MSG(3, "Update Hotel");
  		strcpy(region_key, "HB");
 	}
      	else
      	{
       		PIN_ERR_LOG_MSG(3, "Update Normal");
    		strcpy(region_key, "IN");
    	}
       	if (zip_code)
      	{
       		strcat(region_key, zip_code);
      	}
       	else
       	{
        	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,  "Missing Zip Code");
              	PIN_ERRBUF_RESET(ebufp);
              	goto CLEANUP;
      	}

	
	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 0, ebufp));
	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, args_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &status, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 3, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &status_inactive, ebufp);

	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_region_key : search service input list", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_region_key : search service output flist", 
			search_outflistp);
	while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(search_outflistp, PIN_FLD_RESULTS, &elem_id, 
		1, &cookie, ebufp )) != (pin_flist_t *) NULL)
	{
		catv_info_flistp = PIN_FLIST_SUBSTR_GET(results_flistp, MSO_FLD_CATV_INFO, 1, ebufp);
		network_node = PIN_FLIST_FLD_GET(catv_info_flistp, MSO_FLD_NETWORK_NODE, 1, ebufp );
		svcwrite_inflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, svcwrite_inflistp, PIN_FLD_POID, ebufp);

		catv_flistp = PIN_FLIST_SUBSTR_ADD(svcwrite_inflistp, MSO_FLD_CATV_INFO, ebufp );
		PIN_FLIST_FLD_SET(catv_flistp, MSO_FLD_REGION_KEY, region_key, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
					"fm_mso_update_region_key : write svc input list", svcwrite_inflistp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, svcwrite_inflistp, &svcwrite_outflistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
						"Error in calling PCM_OP_WRITE_FLDS svc for region key", ebufp);
			goto CLEANUP;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
					"fm_mso_update_region_key : write svc output flist", svcwrite_outflistp);


		status_flags = 10;

		provaction_inflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);     
		PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, provaction_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
		PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);

		/********************** VERIMATRIX CHANGES **********************
		 * Enhanced the condition to check for Verimatrix network node
		 ****************************************************************/
		if (network_node && strstr(network_node, "NDS") || strstr(network_node, "VM"))
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_region_key : provisioning input list", 
						provaction_inflistp);
			PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
							"Error in calling MSO_OP_PROV_CREATE_ACTION ", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				goto CLEANUP;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_region_key : provisioning output flist", 
						provaction_outflistp);
		}
	}
CLEANUP:
        PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
        PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
        PIN_FLIST_DESTROY_EX(&readacct_inflistp, NULL);
        PIN_FLIST_DESTROY_EX(&readacct_outflistp, NULL);
        PIN_FLIST_DESTROY_EX(&svcwrite_inflistp, NULL);
        PIN_FLIST_DESTROY_EX(&svcwrite_outflistp, NULL);
	PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
	PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);	
        return;
}

static int32
fm_mso_technology_change(
	pcm_context_t           *ctxp,
        pin_flist_t             *in_flist,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t		*data_array1 = NULL;
	pin_flist_t             *data_array2 = NULL;
	int			*old_tech = NULL;
	int			*new_tech = NULL;
	poid_t			*serv_obj = NULL;
	pin_flist_t		*update_serv_i = NULL;
	pin_flist_t             *update_serv_o = NULL;
	pin_flist_t		*temp_flistp = NULL;
	int			ret_flag = 0;
	pin_flist_t		*ret_flistpp = NULL;
	int			*prov_status = NULL;
	int			*status = NULL;
	pin_flist_t		*bb_flist = NULL;
	pin_flist_t		*args_flist = NULL;
	poid_t			*acc_pdp = NULL;
	int32			*bb_tech = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "Error before calling fm_mso_technology_change", ebufp);
                return 3;
        }

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Updating Service Technology");
        serv_obj = PIN_FLIST_FLD_GET( in_flist, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	acc_pdp = PIN_FLIST_FLD_GET( in_flist, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	data_array1 = PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_DATA_ARRAY, 0, 1, ebufp);
	data_array2 = PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_DATA_ARRAY, 1, 1, ebufp);
	if(data_array1)
	{
		old_tech = (int *)PIN_FLIST_FLD_GET(data_array1, MSO_FLD_TECHNOLOGY, 1, ebufp);
	}
	if(data_array2)
	{
		new_tech = (int *)PIN_FLIST_FLD_GET(data_array2, MSO_FLD_TECHNOLOGY, 1, ebufp);
	}
	if(old_tech && new_tech && (((*old_tech == 1 || *old_tech == 2 || *old_tech == 4) && (*new_tech == 3 || *new_tech == 5)) || ((*old_tech == 5 || *old_tech == 3) && (*new_tech == 1 || *new_tech == 2 || *new_tech == 4))))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Invalid tech change for service", ebufp);
                                return 5;
	}
	if(old_tech && new_tech && *old_tech != *new_tech )
	{
			fm_mso_get_poid_details(ctxp, serv_obj, &ret_flistpp, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
                        {
                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in after calling from function fm_mso_get_poid_details", ebufp);
                                PIN_ERRBUF_RESET(ebufp);
                                return 0;
                        }
			status = PIN_FLIST_FLD_GET(ret_flistpp, PIN_FLD_STATUS, 1, ebufp);
			temp_flistp = PIN_FLIST_ELEM_GET(ret_flistpp, PIN_FLD_TELCO_FEATURES, 0, 1, ebufp);
			prov_status = PIN_FLIST_FLD_TAKE(temp_flistp, PIN_FLD_STATUS, 1, ebufp);
			temp_flistp = PIN_FLIST_ELEM_GET(ret_flistpp, MSO_FLD_BB_INFO, 0, 1, ebufp);
			bb_tech = PIN_FLIST_FLD_GET(temp_flistp, MSO_FLD_TECHNOLOGY, 1, ebufp);
			if(bb_tech && old_tech && *old_tech != *bb_tech)
			{
				PIN_FLIST_DESTROY_EX(&ret_flistpp, NULL);
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Service technology and old technology in input are not matching", ebufp);
                                return 6;
			}
			PIN_FLIST_DESTROY_EX(&ret_flistpp, NULL);
			if((status && *status != 10103) && prov_status && *prov_status == 2 || *prov_status == 0)
			{
				update_serv_i = PIN_FLIST_CREATE(ebufp);
        			PIN_FLIST_FLD_SET(update_serv_i, PIN_FLD_POID, (void *)acc_pdp, ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, update_serv_i, PIN_FLD_PROGRAM_NAME, ebufp);
				args_flist = PIN_FLIST_ELEM_ADD(update_serv_i, PIN_FLD_SERVICES, 0, ebufp);
				PIN_FLIST_FLD_SET(args_flist, PIN_FLD_POID, (void *)serv_obj, ebufp);
				temp_flistp = PIN_FLIST_SUBSTR_ADD(args_flist, PIN_FLD_INHERITED_INFO, ebufp);
        			//Add the BB Info SUBSTR
        			bb_flist = PIN_FLIST_SUBSTR_ADD(temp_flistp, MSO_FLD_BB_INFO, ebufp);
        			PIN_FLIST_FLD_SET(bb_flist, MSO_FLD_TECHNOLOGY, new_tech, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_technology_change update input list", update_serv_i);
                		PCM_OP(ctxp, PCM_OP_CUST_UPDATE_SERVICES, 0, update_serv_i, &update_serv_o, ebufp);
				PIN_FLIST_DESTROY_EX(&update_serv_i, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
                		{
                                	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_CUST_UPDATE_SERVICES", ebufp);
                                	PIN_ERRBUF_RESET(ebufp);
                                	return 0;
                		}
                		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_technology_change update output flist", update_serv_o);
                		PIN_FLIST_DESTROY_EX(&update_serv_o, NULL);
			}
			else
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Not a valid status to change the technology", ebufp);
                		return 4;
			}
	}
	else
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error either both technologies are same or issue in the input flist", ebufp);
		return 1;
	}
	return 2;
}


/****Get the associated services for the given account********/

static int32
fm_mso_get_services(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flist,
        pin_errbuf_t            *ebufp)
{
        poid_t                  *serv_pdp = NULL;
        pin_flist_t             *read_serv_iflist = NULL;
        pin_flist_t             *read_serv_oflist = NULL;
        pin_flist_t             *args_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	int			*serv_status = NULL;
	int			*prov_status = NULL;
	pin_flist_t		*read_serv_rflist = NULL;
	int                             h_elem_id = 0;
	pin_cookie_t            h_cookie = NULL;
	int32			flag = 0;
	int32			ret_flag = 1;
	poid_t			*acc_pdp = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "Error before calling fm_mso_get_services", ebufp);
                return 0;
        }
	acc_pdp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_ACCOUNT_OBJ, 1, ebufp );

	/****Get the associated services for the given account********/
        read_serv_iflist = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(read_serv_iflist, PIN_FLD_POID, acc_pdp, ebufp);
        PIN_FLIST_FLD_SET(read_serv_iflist, PIN_FLD_FLAGS, &flag, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_BAL_GET_ACCT_BAL_GRP_AND_SVC input flist", read_serv_iflist);
        PCM_OP(ctxp, PCM_OP_BAL_GET_ACCT_BAL_GRP_AND_SVC, 0, read_serv_iflist, &read_serv_oflist, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_BAL_GET_ACCT_BAL_GRP_AND_SVC output flist", read_serv_oflist);
        PIN_FLIST_DESTROY_EX(&read_serv_iflist, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error while calling PCM_OP_BAL_GET_ACCT_BAL_GRP_AND_SVC", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                ret_flag = 2;
		goto CLEANUP;
        }
	if(PIN_FLIST_ELEM_COUNT(read_serv_oflist, PIN_FLD_RESULTS, ebufp) == 1)
	{
		ret_flag = 3;
		goto CLEANUP;
	}	
	if(PIN_FLIST_ELEM_COUNT(read_serv_oflist, PIN_FLD_RESULTS, ebufp) >0)
	{
        	while((result_flistp = PIN_FLIST_ELEM_GET_NEXT(read_serv_oflist, PIN_FLD_RESULTS,
                	&h_elem_id, 1, &h_cookie, ebufp)) != (pin_flist_t *)NULL)
        	{
                	serv_pdp = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
			serv_status = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_STATUS, 1, ebufp);
			
			if(serv_pdp && (serv_status && *serv_status != 10103))
			{
				read_serv_iflist = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(read_serv_iflist, PIN_FLD_POID, serv_pdp, ebufp);
        			args_flistp = PIN_FLIST_ELEM_ADD(read_serv_iflist, PIN_FLD_TELCO_FEATURES, 0, ebufp);
        			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, NULL, ebufp);
        			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read service input", read_serv_iflist);
        			PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_serv_iflist, &read_serv_rflist, ebufp);
        			PIN_FLIST_DESTROY_EX(&read_serv_iflist, NULL);
        			if (PIN_ERRBUF_IS_ERR(ebufp))
        			{
                			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
					PIN_ERRBUF_RESET(ebufp);
                			ret_flag = 2;
					goto CLEANUP;
        			}
        			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read service output", read_serv_rflist);
				args_flistp = PIN_FLIST_ELEM_GET(read_serv_rflist, PIN_FLD_TELCO_FEATURES, 0, 1, ebufp);
				if(args_flistp)
				{
					prov_status = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_STATUS, 1, ebufp);
				}
				if(prov_status && (*prov_status == 2 || *prov_status == 0))
				{
					ret_flag = 3;
					PIN_FLIST_DESTROY_EX(&read_serv_rflist, NULL);
					goto CLEANUP;
				}
				PIN_FLIST_DESTROY_EX(&read_serv_rflist, NULL);

			}
        	}
	}
	else
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "No service exists for this account", ebufp);
		ret_flag = 1;
		goto CLEANUP;
	}
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&read_serv_oflist, NULL);
	return ret_flag;
}

static void
fm_mso_cust_modify_gst_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*profile_in_flistp = NULL;
	pin_flist_t		*profile_out_flistp = NULL;
	pin_flist_t		*profile_flistp = NULL;
	pin_flist_t		*gst_info_flistp = NULL;
	pin_flist_t		*inherited_flistp = NULL;
	pin_flist_t		*existing_profile_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;


	poid_t			*profile_pdp = NULL;
	poid_t			*existing_profile_pdp = NULL;
	poid_t			*account_pdp = NULL;
	char			*state = NULL;
	char			*existing_state = NULL;
	char			*pgm_name  = NULL;
	int32			failure = 1;
	int32			success = 0;
	int32			ret_flag = 0;
	int32			elem_id = 0;
	int32			state_found = 0;
	pin_cookie_t		cookie = NULL;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_gst_info input flist", i_flistp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_cust_modify_gst_info", ebufp);
		return;
	}

	account_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
	*r_flistpp = PIN_FLIST_CREATE(ebufp);
	pgm_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	gst_info_flistp = PIN_FLIST_ELEM_GET(i_flistp, MSO_FLD_GST_INFO, PIN_ELEMID_ANY, 1, ebufp);
	state = PIN_FLIST_FLD_GET(gst_info_flistp, PIN_FLD_STATE, 1, ebufp);
	//ret_flag = fm_mso_cust_get_gst_profile(ctxp, account_pdp, &existing_profile_flistp, state, 1, ebufp);
        if (!state || !pgm_name || !account_pdp)
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Missing Mandatory Information");
                PIN_ERRBUF_RESET(ebufp);
                ret_flag = 0;
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51007", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Missing Mandatory Information", ebufp);
                goto CLEANUP;
        }

	ret_flag = fm_mso_cust_get_gst_profile(ctxp, account_pdp, &existing_profile_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Error searching profile");
		PIN_ERRBUF_RESET(ebufp);
		ret_flag = 0;
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51001", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Error searching profile", ebufp);
		goto CLEANUP;
	}
	if(existing_profile_flistp)
	{
		PIN_ERR_LOG_FLIST(3, "existing_profile_flistp", existing_profile_flistp);
		while((tmp_flistp = PIN_FLIST_ELEM_GET_NEXT(existing_profile_flistp, MSO_FLD_GST_INFO, &elem_id, 1, &cookie, ebufp)) != NULL)
		{
			PIN_ERR_LOG_FLIST(3, "GST flist", tmp_flistp);
			existing_state = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_STATE, 1, ebufp);
			PIN_ERR_LOG_FLIST(3, "GST flist", tmp_flistp);
			if (existing_state && state && strcmp(existing_state, state) == 0)
			{
				PIN_ERR_LOG_MSG(3, "State Found");
				state_found = 1;
				break;
			}
		}
		if (PIN_ERRBUF_IS_ERR(ebufp))
                        PIN_ERR_LOG_MSG(3, "check 3");
		if(state_found ==  1)
		{
			profile_pdp = PIN_FLIST_FLD_GET(existing_profile_flistp, PIN_FLD_POID, 0, ebufp);

			profile_in_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(profile_in_flistp, PIN_FLD_POID, profile_pdp, ebufp);
			PIN_FLIST_FLD_SET(profile_in_flistp, PIN_FLD_ACCOUNT_OBJ, account_pdp, ebufp);
			profile_flistp = PIN_FLIST_ELEM_ADD(profile_in_flistp, PIN_FLD_PROFILES, 0, ebufp);
			PIN_FLIST_FLD_SET(profile_flistp, PIN_FLD_PROFILE_OBJ, profile_pdp, ebufp);
			inherited_flistp = PIN_FLIST_SUBSTR_ADD(profile_flistp, PIN_FLD_INHERITED_INFO, ebufp);
			PIN_FLIST_ELEM_SET(inherited_flistp, gst_info_flistp, MSO_FLD_GST_INFO, elem_id, ebufp);

			PIN_ERR_LOG_FLIST(3, "Update Profile Input Flist: ", profile_in_flistp);
			PCM_OP(ctxp, PCM_OP_CUST_MODIFY_PROFILE, 0, profile_in_flistp, &profile_out_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&profile_in_flistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Error in updating GST profile");
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51002", ebufp);
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Error in updating GST profile", ebufp);
				goto CLEANUP;
			}
			PIN_ERR_LOG_FLIST(3, "Update Profile Output Flist: ", profile_out_flistp);
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"GST Details not found for the state");
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51003", ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "GST Details not found for the state", ebufp);
			goto CLEANUP;
		}
		fm_mso_update_gst_info_lc_event(ctxp, i_flistp, tmp_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Error in creating lifecycle event");
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51004", ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Error in creating lifecyle event", ebufp);
			goto CLEANUP;
		}
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &success, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "00", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "GST Information updates successfully", ebufp);
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"GST Details not found for the state");
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51003", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "GST Details not found for the state", ebufp);
		goto CLEANUP;
	}

	
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&profile_out_flistp, NULL);

	return;
}

static void
fm_mso_update_lco_contact(
        pcm_context_t           *ctxp,
	poid_t			*acct_obj,
        char			*rmn,
	char			*rmail,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t 		*update_inp_flistp = NULL;
	pin_flist_t 		*update_out_flistp = NULL;
	pin_flist_t 		*results_flistp = NULL;
	pin_flist_t 		*results_phn_flistp = NULL;
	pin_flist_t 		*nameinfo_flistp = NULL;
	pin_flist_t 		*rflds_inp_flistp = NULL;
	pin_flist_t 		*rflds_res_flistp = NULL;
	pin_flist_t 		*phn_flistp = NULL;
	int32			*ph_type = NULL;
	int			phone = 0;
	int			elem_id = 0;
	int			elem_id1 = 0;
	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie1 = NULL;

    	char                    *phn = NULL;
    	int32                   *phn_typ = NULL;

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lco_contact");

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_update_lco_contact");
		goto CLEANUP;
	}
	rflds_inp_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(rflds_inp_flistp, PIN_FLD_POID, acct_obj, ebufp);					
	PIN_FLIST_ELEM_ADD(rflds_inp_flistp, PIN_FLD_NAMEINFO, PIN_ELEMID_ANY, ebufp);					
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lco_contact read flds input list", rflds_inp_flistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, rflds_inp_flistp, &rflds_res_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&rflds_inp_flistp, NULL);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_lco_contact read flds result list", rflds_res_flistp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS account");
		goto CLEANUP;
	}


	update_inp_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(update_inp_flistp, PIN_FLD_POID, acct_obj, ebufp);
	if (rmn)
	{
		PIN_FLIST_FLD_SET(update_inp_flistp, MSO_FLD_RMN, rmn, ebufp);
	}
	if(rmail)
	{
		PIN_FLIST_FLD_SET(update_inp_flistp, MSO_FLD_RMAIL, rmail, ebufp);	
	}
	while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(rflds_res_flistp, PIN_FLD_NAMEINFO, &elem_id, 1, &cookie, ebufp )) != NULL)
	{
		nameinfo_flistp = PIN_FLIST_ELEM_ADD(update_inp_flistp, PIN_FLD_NAMEINFO, elem_id, ebufp);
		if(rmail)
		{
			PIN_FLIST_FLD_SET(nameinfo_flistp, PIN_FLD_EMAIL_ADDR, rmail, ebufp);
		}
		if (rmn)
		{
			elem_id1 = 0;
			cookie1 =  NULL;
			while ((results_phn_flistp = PIN_FLIST_ELEM_GET_NEXT(results_flistp, PIN_FLD_PHONES, &elem_id1, 1, &cookie1, ebufp )) != NULL)
			{
				 phn = PIN_FLIST_FLD_GET(results_phn_flistp, PIN_FLD_PHONE, 1, ebufp );
				 phn_typ = PIN_FLIST_FLD_GET(results_phn_flistp, PIN_FLD_TYPE, 1, ebufp );
				 if(phn_typ && *phn_typ == 5) 
				 {
					phn_flistp = PIN_FLIST_ELEM_ADD(nameinfo_flistp, PIN_FLD_PHONES, elem_id1, ebufp);
					PIN_FLIST_FLD_SET(phn_flistp, PIN_FLD_PHONE, rmn, ebufp);
				 }
			}
		}
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Contact Update Input Flist", update_inp_flistp);
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, update_inp_flistp, &update_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Contact Update Output Flist", update_out_flistp);
	PIN_FLIST_DESTROY_EX(&update_inp_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in updating contact info", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		goto CLEANUP;
	}

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&rflds_res_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&update_out_flistp, NULL);
	return;		
}

/**
**Function to check if any add on services like Netflix has to created
**/
void fm_mso_modify_addon_services (
        pcm_context_t   *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t    *ebufp)

{

         int32                   elem_id = 0;
        pin_cookie_t            cookie = NULL;
        pin_flist_t     *temp_flistp = NULL;
        pin_flist_t     *args_flistp = NULL;
        pin_flist_t     *srch_flistp = NULL;
        pin_flist_t     *srch_out_flistp = NULL;
        pin_flist_t     *results_flistp = NULL;
        poid_t          *account_obj = NULL;
        poid_t          *svc_pdp = NULL;
        char     *offer_descr=  NULL;
        int     *offer_val =  NULL;
        int     net_ofr_val = 1;
        int32           srch_flag = 256;

        int32   status_error = 1;
        int64           db = 0;
        elem_id=0;
        cookie= NULL;
        char *ofr_desc= NULL;
        char            *search_template= " select X from /service where F1 = V1 AND F2.type = V2 ";

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_addon_services  input list", i_flistp);
        while((temp_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, MSO_FLD_REGISTER_CUSTOMER,&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
        {
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_addon_services temp_flistp ", temp_flistp);
                offer_descr = PIN_FLIST_FLD_GET(temp_flistp, MSO_FLD_OFFER_DESCR, 1, ebufp );
                offer_val = PIN_FLIST_FLD_GET(temp_flistp, MSO_FLD_OFFER_VALUE, 1, ebufp );
                if ((offer_descr && strcmp(offer_descr,"Netflix") == 0) && (*offer_val == 1)){
                        //Request is to create inactive netflix service.Below we will check if any netflix service already exists
                        account_obj = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp );
                        db = PIN_POID_GET_DB(account_obj);
                        srch_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
                        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
                        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, search_template, ebufp);
                        args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
                        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
                        args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
                        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/service/netflix", 0, ebufp), ebufp);
                        results_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
                        PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID, NULL, ebufp);
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Existing netflix service  search input list", srch_flistp);
                        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
                        if (PIN_ERRBUF_IS_ERR(ebufp)  || !srch_out_flistp)
                        {
                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error while searching existing netflix service Obj", ebufp);
                                PIN_ERRBUF_RESET(ebufp);
                                *r_flistpp= PIN_FLIST_CREATE(ebufp);
                                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, account_obj, ebufp);
                                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_error, ebufp);
                                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "71002", ebufp);
                                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Error while searching existing netflix service Obj", ebufp);
                                goto CLEANUP;
                        }
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Existing netflix service  search output flist", srch_out_flistp);
                        results_flistp= PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "customer profile search result flist", results_flistp);
                        if (results_flistp) {
                                svc_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_POID, 1, ebufp);
                                if (!svc_pdp) {
                                        fm_mso_modify_netflix_service(ctxp, i_flistp,r_flistpp,ebufp);
                                } else {
                                        return;
                                }
                        } else {
                                fm_mso_modify_netflix_service(ctxp, i_flistp,r_flistpp,ebufp);
                        }

                }
        }

        CLEANUP:
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
        return;

}

/****
***Function to create Netflix Addon service
**/

void fm_mso_modify_netflix_service(
        pcm_context_t   *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t    *ebufp)

{
        pin_flist_t     *all_billinfo = NULL;
        pin_flist_t     *bi_out_flist = NULL;
        pin_flist_t     *bal_grp_flistp = NULL;
        pin_flist_t     *bal_grp_o_flistp = NULL;
        pin_flist_t     *srvc_flistp = NULL;
        pin_flist_t     *srvc_o_flistp = NULL;
        pin_flist_t     *srvc_arrflistp = NULL;
        pin_flist_t     *statuses_array =  NULL;


        poid_t          *account_obj = NULL;
        poid_t          *billinfo_pdp = NULL;
        poid_t          *bal_obj = NULL;
        poid_t          *svc_obj = NULL;

        int64           db = 0;
        int32                  status = 10100;
        int32                   netflix_failed = 1;
        int32                   netflix_success = 0;
        int32           serv_status = PIN_STATUS_INACTIVE;
        int32           status_flags = 0;
        char                    *login = NULL;
        char                    *pwd = NULL;
        int32   buf_len =80;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_netflix_service inputflist", i_flistp);

        account_obj = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp );
        db = PIN_POID_GET_DB(account_obj);
        login = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_LOGIN, 1, ebufp);
        pwd =  PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PASSWD_CLEAR, 1, ebufp);
         if (!login) {
                PIN_POID_TO_STR(account_obj,&login,&buf_len,ebufp);

        }
        if (!pwd) {
                PIN_POID_TO_STR(account_obj,&pwd,&buf_len,ebufp);

        }

        //find out the existing billinfo poid
        fm_mso_get_all_billinfo(ctxp, account_obj, ALL, &all_billinfo, ebufp);
        if (all_billinfo)
        {
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_netflix_service:fm_mso_get_all_billinfo outputflist", all_billinfo);
                bi_out_flist = PIN_FLIST_ELEM_GET(all_billinfo, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
                if (bi_out_flist)
                {
                        billinfo_pdp = PIN_FLIST_FLD_GET(bi_out_flist, PIN_FLD_POID, 1, ebufp);

                }
        }

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in populating billinfo", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                *r_flistpp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, account_obj, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &netflix_failed, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "70000", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Error in populating billinfo for netflix service", ebufp);
                goto CLEANUP;
        }


        bal_grp_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(bal_grp_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/balance_group", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(bal_grp_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
        PIN_FLIST_FLD_SET(bal_grp_flistp, PIN_FLD_BILLINFO_OBJ, billinfo_pdp, ebufp);
        //Creating seperate balancegroup for netflix
        fm_mso_create_balgrp(ctxp, bal_grp_flistp, &bal_grp_o_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Validate  bal_grp_flistp created", bal_grp_o_flistp );
        if (bal_grp_o_flistp)
        {
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "new balance created for Netflix service", bal_grp_o_flistp);
                bal_obj = PIN_FLIST_FLD_GET(bal_grp_o_flistp, PIN_FLD_POID, 0, ebufp );
                //Creating netflixservice
                srvc_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_PUT(srvc_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/service/netflix", -1, ebufp), ebufp);
                PIN_FLIST_FLD_SET(srvc_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
                PIN_FLIST_FLD_SET(srvc_flistp, PIN_FLD_BAL_GRP_OBJ, bal_obj, ebufp);
       //         srvc_arrflistp = PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_SERVICES, 0, 1, ebufp);
         //       PIN_FLIST_FLD_COPY(srvc_arrflistp, PIN_FLD_LOGIN, srvc_flistp, PIN_FLD_LOGIN, ebufp);
                PIN_FLIST_FLD_SET(srvc_flistp, PIN_FLD_LOGIN, login, ebufp);
                PIN_FLIST_FLD_SET(srvc_flistp, PIN_FLD_PASSWD_CLEAR, pwd, ebufp);//remove hardcoding if this is passed in the input flist
                PIN_FLIST_FLD_SET(srvc_flistp, PIN_FLD_STATUS, &status, ebufp);
                statuses_array = PIN_FLIST_ELEM_ADD(srvc_flistp, PIN_FLD_STATUSES, 0, ebufp);
 PIN_FLIST_FLD_SET(statuses_array, PIN_FLD_STATUS, &serv_status, ebufp);

                PIN_FLIST_FLD_SET(statuses_array, PIN_FLD_STATUS_FLAGS, &status_flags, ebufp);

                fm_mso_create_service(ctxp, srvc_flistp, &srvc_o_flistp, ebufp);
                if (srvc_o_flistp) {
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "new service object  created for Netflix service", srvc_o_flistp);
                        svc_obj = PIN_FLIST_FLD_GET(srvc_o_flistp, PIN_FLD_POID, 0, ebufp );
                }

                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in creating inactive  for netflix service", ebufp);
                        PIN_ERRBUF_RESET(ebufp);
                        *r_flistpp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, account_obj, ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &netflix_failed, ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "70001", ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Error in creating inactive  for netflix service", ebufp);
                        goto CLEANUP;
                } else {

                        *r_flistpp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, account_obj, ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &netflix_success, ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_DESCR, "SUccessfully created  inactive   netflix service", ebufp);

                }


        }

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in creating bal_grp_obj for netflix service", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                *r_flistpp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, account_obj, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &netflix_failed, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "70001", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Error in creating bal_grp_obj for netflix service", ebufp);
                goto CLEANUP;
        }
 CLEANUP:
        PIN_FLIST_DESTROY_EX(&bal_grp_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&srvc_flistp, NULL);

        return;
}

/**************************************************
* Function: 	fm_mso_get_eprofile_offer()
* Decription:   Returns the offers in profile_cust_offer table 
***************************************************/
void
fm_mso_get_eprofile_offer(
	pcm_context_t		*ctxp,
	poid_t			*ser_pdp,
	char			*ofr_desc,
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

	char			*template = "select X from /profile_cust_offer  where F1 = V1 and upper(F2) = upper(V2) ";

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_eprofile_offer", ebufp);
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
	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_OFFER_DESCR, ofr_desc, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp );
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_OFFER_VALUE, NULL, ebufp );
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_OFFER_DESCR, NULL, ebufp );
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_ISREGISTERED, NULL, ebufp );
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_profile_offer search profile input flist", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_profile_offer output flist", srch_out_flistp);	

	*ret_flist = PIN_FLIST_COPY(srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);

	return;
}

/* *******************************************************************************
 * Added the below code for JIO-CPS Integration-ISP Enhancement
 * This function gets the flist with the data related to Carrier ID 
 * from the mso_cfg_cmts_master object based on the CMTS ID
 *********************************************************************************/
static void
fm_mso_get_carrier_id(
	pcm_context_t		*ctxp,
	char				*cmts_id,
	pin_flist_t         **r_flistpp,
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
	
	/* ***************************************************************************
	 * Input flist for Search Opcode
	 * 0 PIN_FLD_POID                      POID [0] 0.0.0.1 /search -1 0
	 * 0 PIN_FLD_FLAGS                      INT [0] 768
	 * 0 PIN_FLD_TEMPLATE                   STR [0] "select X from /mso_cfg_cmts_master where F1 = V1 "
	 * 0 PIN_FLD_RESULTS                  ARRAY [0] allocated 2, used 2
	 * 1     MSO_FLD_OTHER_NE_ID              STR [0] NULL
	 * 1     PIN_FLD_CARRIER_ID		       INT [0] 0
	 * 0 PIN_FLD_ARGS                     ARRAY [1] allocated 1, used 1
	 * 1     MSO_FLD_CMTS_ID           	   STR [0] "JIO_TEST_GPON"	 
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
	"select X from /mso_cfg_cmts_master where F1 = V1 " , ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(search_cmts_flistp, PIN_FLD_ARGS, 1, ebufp);
	if(cmts_id != NULL)
	{
		PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_CMTS_ID, cmts_id, ebufp);
	}
	
	res_iflistp = PIN_FLIST_ELEM_ADD( search_cmts_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(res_iflistp, MSO_FLD_OTHER_NE_ID, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_iflistp, PIN_FLD_CARRIER_ID, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Modify Customer - fm_mso_get_carrier_id: Search CMTS Master Input Flist ", search_cmts_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_cmts_flistp, &search_cmts_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Modify Customer - fm_mso_get_carrier_id: Search CMTS Master Output Flist", search_cmts_oflistp);
						
	PIN_FLIST_DESTROY_EX(&search_cmts_flistp, NULL);
						
	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "Error getting the CMTS Master data ", ebufp);        
		PIN_FLIST_DESTROY_EX(&search_cmts_oflistp, NULL);
	}
	else
	{	
		result_flistp = PIN_FLIST_ELEM_TAKE(search_cmts_oflistp,PIN_FLD_RESULTS,0,1,ebufp);
		
		if(result_flistp != NULL)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Modify Customer - fm_mso_get_carrier_id: Search CMTS Master Return Flist", result_flistp);
			*r_flistpp = result_flistp;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Returning from fm_mso_get_carrier_id for Modify Customer Opcode >> ");
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Modify Customer - fm_mso_get_carrier_id: Search CMTS Master Return Flist", *r_flistpp);
		}
		
		PIN_FLIST_DESTROY_EX(&search_cmts_oflistp, NULL);
	}	
	return;
}

