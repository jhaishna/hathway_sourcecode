/*******************************************************************
 * File:	fm_mso_search.c
 * Opcode:	MSO_OP_SEARCH 
 * Owner:	Gautam Adak
 * Created:	17-SEPT-2013
 * Description: 
 *
 * Modification History: 
 * Modified By    |   Date     |Modification details
 *-------------------------------------------------------------------
 * Sachidanand    |23-Jul-2014 |Added search for preactivated devices
 * Sachidanand    |25-Jul-2014 |Update preactivated device search to get
 *				device_id
 *
 * Input Flist
 *
1. Search Login Availability 
	0 PIN_FLD_POID               POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS               INT [0] 0
	0 PIN_FLD_LOGIN               STR [0] "self_care_1"
2. Search by account number
	0 PIN_FLD_POID               POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS               INT [0] 1
	0 PIN_FLD_ACCOUNT_NO          STR [0] "58006"
2. Search by account poid
	0 PIN_FLD_POID               POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS               INT [0] 1
	0 PIN_FLD_ACCOUNT_OBJ        POID [0] 0.0.0.1 /account 58006 0
3. Search by RMN
	0 PIN_FLD_POID               POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS               INT [0] 2
	0 MSO_FLD_RMN                 STR [0] "9983882151"
4.Search by FIRSTNAME Only
	0 PIN_FLD_POID               POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS               INT [0] 3
	0 PIN_FLD_FIRST_NAME          STR [0] "%GAU%"
5.Search by MIDDLENAME only
	0 PIN_FLD_POID              POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS              INT [0] 3
	0 PIN_FLD_MIDDLE_NAME        STR [0] "%TE%"
6. Search by LASTNAME only
	0 PIN_FLD_POID               POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS               INT [0] 3
	0 PIN_FLD_LAST_NAME           STR [0] "%ADAK%"
7.Search by FIRSTNAME and LASTNAME
	0 PIN_FLD_POID               POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS               INT [0] 3
	0 PIN_FLD_FIRST_NAME          STR [0] "%GAU%"
	0 PIN_FLD_LAST_NAME           STR [0] "%AD%"
8.Search by FIRSTTNAME and MIDDLENAME
	0 PIN_FLD_POID               POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS               INT [0] 3
	0 PIN_FLD_FIRST_NAME          STR [0] "%GAU%"
	0 PIN_FLD_MIDDLE_NAME         STR [0] "%KUM%"
9. Search by FIRSTNAME ,MIDDLENAME and LASTNAME
	0 PIN_FLD_POID               POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS               INT [0] 3
	0 PIN_FLD_FIRST_NAME          STR [0] "%GAU%"
	0 PIN_FLD_MIDDLE_NAME         STR [0] "%KUM%"
	0 PIN_FLD_LAST_NAME           STR [0] "%KUM%"
10.Search by Company
	0 PIN_FLD_POID               POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS               INT [0] 5
	0 PIN_FLD_COMPANY          STR [0] "%ORA%"
11.Search by Card Number
	0 PIN_FLD_POID               POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS               INT [0] 6
	0 PIN_FLD_NAME                STR [0] "180920130030"
12. Transaction History
	0 PIN_FLD_POID               POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS               INT [0] 4
	0 PIN_FLD_TYPE			ENUM [0] 1
	0 PIN_FLD_ACCOUNT_OBJ        POID [0] 0.0.0.1 /account 307101 0
	0 PIN_FLD_START_T           TSTAMP [0] (1409300201)
	0 PIN_FLD_END_T             TSTAMP [0] (1409380201)
15.Search by Card Number
	0 PIN_FLD_POID                         POID [0] 0.0.0.1 /mso_cfg_refund_rule -1 4
	0 PIN_FLD_USERID		       POID [0] 0.0.0.1 /account 44029 10
	0 PIN_FLD_SEARCH_TYPE			INT [0] 1
	0 PIN_FLD_FLAGS				INT [0] 51
16.Order Search by Order id
        0 PIN_FLD_POID                         POID [0] 0.0.0.1 /search -1
        0 PIN_FLD_USERID                       POID [0] 0.0.0.1 /account 44029 10
        0 PIN_FLD_FLAGS                         INT [0] 101
        0 PIN_FLD_ORDER_ID                      STR [0] "PROV-101"
17.Order Search by Error Code
        0 PIN_FLD_POID                         POID [0] 0.0.0.1 /search -1
        0 PIN_FLD_USERID                       POID [0] 0.0.0.1 /account 44029 10
        0 PIN_FLD_FLAGS                         INT [0] 102
        0 PIN_FLD_ERROR_CODE                    STR [0] "ADAPTER_TRANS_FAILED"
18.Order Search by Error Description
        0 PIN_FLD_POID                         POID [0] 0.0.0.1 /search -1
        0 PIN_FLD_USERID                       POID [0] 0.0.0.1 /account 44029 10
        0 PIN_FLD_FLAGS                         INT [0] 103
        0 PIN_FLD_ERROR_DESCR			STR [0] "%field trasformation failed%"
19.Order Search by Provisioning Action
        0 PIN_FLD_POID                         POID [0] 0.0.0.1 /search -1
        0 PIN_FLD_USERID                       POID [0] 0.0.0.1 /account 44029 10
        0 PIN_FLD_FLAGS                         INT [0] 104
        0 PIN_FLD_ACTION     			STR [0] "SUSPEND_SUBSCRIBER_PK"
20.Order Search by Order creation date 
        0 PIN_FLD_POID                         POID [0] 0.0.0.1 /search -1
        0 PIN_FLD_USERID                       POID [0] 0.0.0.1 /account 44029 10
        0 PIN_FLD_FLAGS                         INT [0] 105
        0 PIN_FLD_START_CREATION_DATE		STR [0] "20140101"
        0 PIN_FLD_END_CREATION_DATE 		STR [0] "20140101"
21.Order Search by Order status & Account OBj
        0 PIN_FLD_POID                         POID [0] 0.0.0.1 /search -1
        0 PIN_FLD_USERID                       POID [0] 0.0.0.1 /account 44029 10
	0 PIN_FLD_ACCOUNT_OBJ                  POID [0] 0.0.0.1 /account 44029 10
        0 PIN_FLD_FLAGS                         INT [0] 106
        0 PIN_FLD_STATUS           ENUM [0] 1
22.Order Search by Order id & Account OBj
        0 PIN_FLD_POID                         POID [0] 0.0.0.1 /search -1
        0 PIN_FLD_USERID                       POID [0] 0.0.0.1 /account 44029 10
	0 PIN_FLD_ACCOUNT_OBJ                  POID [0] 0.0.0.1 /account 44029 10
        0 PIN_FLD_FLAGS                         INT [0] 107
        0 PIN_FLD_ORDER_ID                      STR [0] "PROV-101"
23.Order Search by Error Code & Account OBj
        0 PIN_FLD_POID                         POID [0] 0.0.0.1 /search -1
        0 PIN_FLD_USERID                       POID [0] 0.0.0.1 /account 44029 10
	0 PIN_FLD_ACCOUNT_OBJ                  POID [0] 0.0.0.1 /account 44029 10
        0 PIN_FLD_FLAGS                         INT [0] 108
        0 PIN_FLD_ERROR_CODE                    STR [0] "ADAPTER_TRANS_FAILED"
24.Order Search by Error Description & Account OBj
        0 PIN_FLD_POID                         POID [0] 0.0.0.1 /search -1
        0 PIN_FLD_USERID                       POID [0] 0.0.0.1 /account 44029 10
	0 PIN_FLD_ACCOUNT_OBJ                  POID [0] 0.0.0.1 /account 44029 10
        0 PIN_FLD_FLAGS                         INT [0] 109
        0 PIN_FLD_ERROR_DESCR			STR [0] "%field trasformation failed%"
25.Order Search by Provisioning Action & Account OBj
        0 PIN_FLD_POID                         POID [0] 0.0.0.1 /search -1
        0 PIN_FLD_USERID                       POID [0] 0.0.0.1 /account 44029 10
	0 PIN_FLD_ACCOUNT_OBJ                  POID [0] 0.0.0.1 /account 44029 10
        0 PIN_FLD_FLAGS                         INT [0] 110
        0 PIN_FLD_ACTION     			STR [0] "SUSPEND_SUBSCRIBER_PK"
26.Order Search by Order creation date & Account OBj
        0 PIN_FLD_POID                         POID [0] 0.0.0.1 /search -1
        0 PIN_FLD_USERID                       POID [0] 0.0.0.1 /account 44029 10
	0 PIN_FLD_ACCOUNT_OBJ                  POID [0] 0.0.0.1 /account 44029 10
        0 PIN_FLD_FLAGS                         INT [0] 111
        0 PIN_FLD_START_CREATION_DATE		STR [0] "20140101"
        0 PIN_FLD_END_CREATION_DATE 		STR [0] "20140101"
27.Order Search by Order status & Account OBj
        0 PIN_FLD_POID                         POID [0] 0.0.0.1 /search -1
        0 PIN_FLD_USERID                       POID [0] 0.0.0.1 /account 44029 10
	0 PIN_FLD_ACCOUNT_OBJ                  POID [0] 0.0.0.1 /account 44029 10
        0 PIN_FLD_FLAGS                         INT [0] 112
        0 PIN_FLD_STATUS           ENUM [0] 1
28.VC Search by card_no 
        0 PIN_FLD_POID                         POID [0] 0.0.0.1 /search -1
        0 PIN_FLD_USERID                       POID [0] 0.0.0.1 /account 44029 10
	0 PIN_FLD_DEVICE_ID			STR [0] "000072316029"
        0 PIN_FLD_FLAGS                         INT [0] 120
29.STB Search by NDS No 
        0 PIN_FLD_POID                         POID [0] 0.0.0.1 /search -1
        0 PIN_FLD_USERID                       POID [0] 0.0.0.1 /account 44029 10
	0 PIN_FLD_DEVICE_ID			STR [0] "5944627736367130"
        0 PIN_FLD_FLAGS                         INT [0] 121
30.STB Search by serial No 
        0 PIN_FLD_POID                         POID [0] 0.0.0.1 /search -1
        0 PIN_FLD_USERID                       POID [0] 0.0.0.1 /account 44029 10
	0 PIN_FLD_SERIAL_NO			STR [0] "67A19A03163824614"
        0 PIN_FLD_FLAGS                         INT [0] 122
31.VC Search by state and LCO a/c no 
        0 PIN_FLD_POID                         POID [0] 0.0.0.1 /search -1
        0 PIN_FLD_USERID                       POID [0] 0.0.0.1 /account 44029 10
	0 PIN_FLD_ACCOUNT_NO			STR [0] "1000034827"
	0 PIN_FLD_STATE				INT [0] 1
        0 PIN_FLD_FLAGS                         INT [0] 123
32.STB Search by state and LCO a/c no 
        0 PIN_FLD_POID                         POID [0] 0.0.0.1 /search -1
        0 PIN_FLD_USERID                       POID [0] 0.0.0.1 /account 44029 10
	0 PIN_FLD_ACCOUNT_NO			STR [0] "1000034827"
	0 PIN_FLD_STATE				INT [0] 1
        0 PIN_FLD_FLAGS                         INT [0] 124
33.Search for Device LIFECYCLE
        0 PIN_FLD_POID                         POID [0] 0.0.0.1 /search -1
        0 PIN_FLD_USERID                       POID [0] 0.0.0.1 /account 44029 10
     	0 PIN_FLD_FLAGS           		INT [0] 125
	0 PIN_FLD_SEARCH_TYPE     		INT [0] <new macro may need to bedefined >
     	0 PIN_FLD_ACCOUNT_NO      		STR [0] "58006"
	0 MSO_FLD_VC_ID           		STR [0] "000072316029"
	0 MSO_FLD_STB_ID          		STR [0] "000072316029"
	0 PIN_FLD_DEVICE_ID          		STR [0] "000072316029"
	0 PIN_FLD_ACTION          		STR [0] "CREATED"
	0 PIN_FLD_START_T      		     TSTAMP [0] (9864332176)
	0 PIN_FLD_END_T        		     TSTAMP [0] (9864332176

34. Preactivated STB Search by device id 
        0 PIN_FLD_POID                         POID [0] 0.0.0.1 /search -1
        0 PIN_FLD_FLAGS                         INT [0] 125
		0 MSO_FLD_STB_ID         				STR [0] "5944627736367130"
35.Preactivated VC Search by device id 
        0 PIN_FLD_POID                         POID [0] 0.0.0.1 /search -1
        0 PIN_FLD_FLAGS                         INT [0] 126
		0 MSO_FLD_VC_ID         				STR [0] "5944627736367130"

34. Search Contact History 
        0 PIN_FLD_POID                         POID [0] 0.0.0.1 /search -1
        0 PIN_FLD_FLAGS                         INT [0] 128
        0 PIN_FLD_ACCOUNT_OBJ	     	       POID [0] 0.0.0.1 /account 2306428 0 

35. Search BB Service Usage 
        0 PIN_FLD_POID                         POID [0] 0.0.0.1 /search -1
        0 PIN_FLD_FLAGS                         INT [0] 129
        0 PIN_FLD_ACCOUNT_OBJ	     	       POID [0] 0.0.0.1 /account 2306428 0 
        0 PIN_FLD_SERVICE_OBJ	     	       POID [0] 0.0.0.1 /service 1306428 0 
	0 PIN_FLD_START_T      		     TSTAMP [0] (9864332176)
	0 PIN_FLD_END_T        		     TSTAMP [0] (9864332176)

36. Search for BB service status change reasonc codes:
	0 PIN_FLD_POID                      POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_DOMAIN                    STR [0] "Reason Codes-Service Suspension"
	0 PIN_FLD_FLAGS                     INT [0] 130

37. Search Modem by MAC ID 
	0 PIN_FLD_POID                         POID [0] 0.0.0.1 /search -1
	0 PIN_FLD_USERID                       POID [0] 0.0.0.1 /account 44029 10
	0 PIN_FLD_DEVICE_ID                     STR [0] "18:59:33:5c:93:5a"
	0 PIN_FLD_FLAGS                         INT [0] 131

38. Search Router by MAC ID 
	0 PIN_FLD_POID                         POID [0] 0.0.0.1 /search -1
	0 PIN_FLD_USERID                       POID [0] 0.0.0.1 /account 44029 10
	0 PIN_FLD_DEVICE_ID                     STR [0] "18:59:33:5c:93:5a"
	0 PIN_FLD_FLAGS                         INT [0] 132

39. Search Cable Router by MAC ID 
	0 PIN_FLD_POID                         POID [0] 0.0.0.1 /search -1
	0 PIN_FLD_USERID                       POID [0] 0.0.0.1 /account 44029 10
	0 PIN_FLD_DEVICE_ID                     STR [0] "18:59:33:5c:93:5a"
	0 PIN_FLD_FLAGS                         INT [0] 133
	
40. Search modem by State ID and LCO Account no.
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_USERID         POID [0] 0.0.0.1 /account 44029 10
	0 PIN_FLD_ACCOUNT_NO      STR [0] "1000000038"
	0 PIN_FLD_STATE_ID        INT [0] 2
	0 PIN_FLD_FLAGS           INT [0] 134
	
41. Search router by State ID and LCO Account no.
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_USERID         POID [0] 0.0.0.1 /account 44029 10
	0 PIN_FLD_ACCOUNT_NO      STR [0] "1000000038"
	0 PIN_FLD_STATE_ID        INT [0] 1
	0 PIN_FLD_FLAGS           INT [0] 135

42. Search cable router by State ID and LCO Account no.
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_USERID         POID [0] 0.0.0.1 /account 44029 10
	0 PIN_FLD_ACCOUNT_NO      STR [0] "1000000038"
	0 PIN_FLD_STATE_ID        INT [0] 1
	0 PIN_FLD_FLAGS           INT [0] 136	
42. Search device details by serial No.
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_USERID         POID [0] 0.0.0.1 /account 44029 10
	0 MSO_FLD_SERIAL_NO      STR [0] "1000000038"
	0 PIN_FLD_STATE_ID        INT [0] 1
	0 PIN_FLD_FLAGS           INT [0] 139	
	
Modification Details:
Name :  Leela Pratyush
Description:  Added Reason codes search, Get contact history, 
		Updated Get Usage History functions
********************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_search.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "ops/act.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "mso_cust.h"
#include "mso_errs.h"
#include "mso_utils.h"
#include "mso_device.h"

/**************************************
* External Functions start
***************************************/

/**************************************
* External Functions end
***************************************/
void
fm_mso_srch_login(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_srch_by_acntno_or_poid(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);
	
extern void
fm_mso_get_acnt_from_acntno(
    pcm_context_t           *ctxp,
    char*                   acnt_no,
    pin_flist_t             **acnt_details,
    pin_errbuf_t            *ebufp);

PIN_IMPORT int32
fm_mso_is_den_sp(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_srch_by_rmn(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_srch_by_rmn_new(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_srch_by_name(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_srch_by_salesperson_firstname(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_srch_by_company(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_srch_by_card_no(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);
void
fm_mso_srch_trn_history(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_get_csr_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_srch_for_opt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_srch_prov_order(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

void
fm_mso_search_device(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);


void
fm_mso_search_device_lifecycle(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_search_schedule (
	pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);
		
void
fm_mso_search_preactivated_service(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

void
fm_mso_search_acct_bal(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);


void
fm_mso_search_contact_history(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

void
fm_mso_search_bb_usage_history(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

void
fm_mso_srch_by_mobile(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

void 
fm_mso_search_bb_reason_code(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);
		
void
fm_mso_srch_by_email(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);
		
void
fm_mso_srch_bb_devices(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

void
fm_mso_search_ip_device(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);
		
void 
fm_mso_get_prod_price_details(
	pcm_context_t           *ctxp,
	pin_flist_t             *in_flistp,
	pin_flist_t             **ret_flistp,
	pin_errbuf_t            *ebufp);		

void
fm_mso_get_payterm_for_plan(
	pcm_context_t           *ctxp,
	pin_flist_t             *in_flistp,
	pin_flist_t             **ret_flistp,
	pin_errbuf_t            *ebufp);

static void
fm_mso_srch_device_acct(
	pcm_context_t           *ctxp,
	char                    *dev_id,
	int32                   *den_sp,
	pin_flist_t             **ret_flistp,
	pin_errbuf_t            *ebufp);

static void
fm_mso_srch_device_sp_name(
	pcm_context_t           *ctxp,
	char                    *source,
	pin_flist_t             **ret_flistp,
	pin_errbuf_t            *ebufp);

static void
fm_mso_search_ip_pool(
	pcm_context_t           *ctxp,
	pin_flist_t             *in_flistp,
	pin_flist_t             **ret_flistpp,
	pin_errbuf_t            *ebufp);

static void
fm_mso_search_failed_prov_resp(
	pcm_context_t           *ctxp,
	pin_flist_t             *in_flistp,
	pin_flist_t             **ret_flistpp,
	pin_errbuf_t            *ebufp);

static void
fm_mso_search_prov_tags(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_mso_search_cmts_cc_mapping(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

void
fm_mso_srch_by_legacy_acc_no(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

/*******************************************************************
 * MSO_OP_SEARCH 
 *
 * 
 *******************************************************************/

EXPORT_OP void
op_mso_search(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_search(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

PIN_IMPORT void fm_mso_utils_get_event_action(
	char		*poid_type,
	char		**descr,
	pin_errbuf_t	*ebufp);

static void
fm_mso_search_credit_limit_for_plan(
         pcm_context_t           *ctxp,
         pin_flist_t             *in_flistp,
         pin_flist_t             **ret_flistpp,
         pin_errbuf_t            *ebufp);

//Pavan Bellala 25-11-2015 Added to search by aggreement no.
void
fm_mso_srch_by_agreement_no(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

void 
fm_mso_search_ott_swap(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

/*NT: added for channel master*/
void fm_mso_sort_and_filter(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_fld_num_t     field,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp);

void fm_mso_srch_channel_master_data(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp);


/*******************************************************************
 * MSO_OP_SEARCH  
 *******************************************************************/
void 
op_mso_search(
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
	if (opcode != MSO_OP_SEARCH) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_search error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_search input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	fm_mso_search(ctxp, flags, i_flistp, r_flistpp, ebufp);
 	/***********************************************************
	 * Results.
	 ***********************************************************/

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_search error", ebufp);
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_search output flist", *r_flistpp);
	}
	return;
}



/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_search(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*ret_flistp = NULL;

	poid_t			*srch_pdp = NULL;

	int32			*flag_ptr = NULL;
	int32			flag = -1;
	int32			search_fail = 1;

	int64			db = -1;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search input flist", i_flistp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_search", ebufp);
		goto CLEANUP;
	}

	flag_ptr = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp ); 
	srch_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ); 
	 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST001 --> BEFORE IF");
	/*******************************************************************
	* Mandatory fields validation
	*******************************************************************/
	if (flag_ptr && (*flag_ptr <0 || *flag_ptr >MAX_VAL) )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Search: Invalid Flag", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, srch_pdp, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51220", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Search: Invalid Flag", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST001 --> After IF");

	flag = *flag_ptr;
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST001 --> After assign ");
	switch(flag)
	{
          case SRCH_CHANNEL_MASTER: //New Tariff change
                fm_mso_srch_channel_master_data(ctxp, i_flistp, &ret_flistp, ebufp);
                break;
          case SRCH_DISTINCT_CHANNEL_ATTR: //New Tariff change
                fm_mso_srch_channel_master_data(ctxp, NULL, &ret_flistp, ebufp);
                break;
	  case SRCH_LOGIN :
		fm_mso_srch_login(ctxp, i_flistp, &ret_flistp, ebufp);
		break;
	  case SRCH_BY_ACNT_NO :
		fm_mso_srch_by_acntno_or_poid(ctxp, i_flistp, &ret_flistp, ebufp);
		break;
	  case SRCH_BY_RMN :
		fm_mso_srch_by_rmn(ctxp, i_flistp, &ret_flistp, ebufp);
		break;
	  case SRCH_BY_NAME :
		fm_mso_srch_by_name(ctxp, i_flistp, &ret_flistp, ebufp);
		break;
	  case SRCH_BY_COMPANY :
		fm_mso_srch_by_company(ctxp, i_flistp, &ret_flistp, ebufp);
		break;
	  case SRCH_BY_CARD_NO :
		fm_mso_srch_by_card_no(ctxp, i_flistp, &ret_flistp, ebufp);
		break;
	  case SRCH_TRN_HIST :
		fm_mso_srch_trn_history(ctxp, i_flistp, &ret_flistp, ebufp);
		break;
	  case SRCH_ORDER_BY_ORDER_ID:
	  case SRCH_ORDER_BY_ERROR_CODE:
	  case SRCH_ORDER_BY_ERROR_DESCR:
	  case SRCH_ORDER_BY_PROV_ACTION:
	  case SRCH_ORDER_BY_STATUS:
	  case SRCH_ORDER_BY_CREATION_DATE:
 	  case SRCH_ORDER_BY_ACCNT_ORDER_ID:
	  case SRCH_ORDER_BY_ACCNT_ERROR_CODE:
	  case SRCH_ORDER_BY_ACCNT_ERROR_DESCR:
	  case SRCH_ORDER_BY_ACCNT_PROV_ACTION:
	  case SRCH_ORDER_BY_ACCNT_STATUS:
	  case SRCH_ORDER_BY_ACCNT_CREATION_DATE:
		fm_mso_srch_prov_order(ctxp, i_flistp, &ret_flistp, ebufp);
		break;
	 case SRCH_PREACTV_STB:
	 case SRCH_PREACTV_VC:
		 fm_mso_search_preactivated_service(ctxp, i_flistp, &ret_flistp, ebufp);
		break;
	 case SRCH_VC_BY_CARD_NO:
	 case SRCH_STB_BY_NDS_NO:
	 case SRCH_STB_BY_SERIAL_NO:
	 case SRCH_VC_BY_STATE_LCO:
	 case SRCH_STB_BY_STATE_LCO:
	 case SRCH_DEVICE_BY_PO_NO:
		 fm_mso_search_device(ctxp, i_flistp, &ret_flistp, ebufp);
		break;
	  case SRCH_DEVICE_IP :
		fm_mso_search_ip_device(ctxp, i_flistp, &ret_flistp, ebufp);
		break;
	  case SRCH_IP_POOL :
		fm_mso_search_ip_pool(ctxp, i_flistp, &ret_flistp, ebufp);
		break;
	  case SRCH_FAILED_PROV_RESP :
		fm_mso_search_failed_prov_resp(ctxp, i_flistp, &ret_flistp, ebufp);
		break;
	  case SRCH_CMTS_PROV_TAGS :
		fm_mso_search_prov_tags(ctxp, i_flistp, &ret_flistp, ebufp);
		break;
	  case SRCH_CMTS_CC_MAPPING :
		fm_mso_search_cmts_cc_mapping(ctxp, i_flistp, &ret_flistp, ebufp);
		break;
         case SRCH_CREDIT_LIMIT_FOR_PLAN :
                fm_mso_search_credit_limit_for_plan(ctxp, i_flistp, &ret_flistp, ebufp);
                break;
     case SRCH_BY_LIFECYCLE: 
	    fm_mso_search_device_lifecycle(ctxp,i_flistp, &ret_flistp, ebufp);
        break;
	case SRCH_SCHEDULE_BY_DATES:
		fm_mso_search_schedule(ctxp,i_flistp, &ret_flistp, ebufp);
		break;
	case SRCH_ACCT_BAL:
		fm_mso_search_acct_bal(ctxp,i_flistp, &ret_flistp, ebufp);
		break;
	case SRCH_CONTACT_HISTORY:
		fm_mso_search_contact_history(ctxp,i_flistp, &ret_flistp, ebufp);
		break;
	case SRCH_BB_USAGE_HISTORY:
		fm_mso_search_bb_usage_history(ctxp,i_flistp, &ret_flistp, ebufp);
		break;
	case SRCH_BB_REASON_CODE:
		fm_mso_search_bb_reason_code(ctxp,i_flistp, &ret_flistp, ebufp);
		break;
	 case SRCH_BY_MOBILE_NO:
		 fm_mso_srch_by_mobile(ctxp, i_flistp, &ret_flistp, ebufp);
		break;	
	case SRCH_BY_EMAIL:
		fm_mso_srch_by_email(ctxp, i_flistp, &ret_flistp, ebufp);
		break;
	case SRCH_BY_LEGACY_ACC_NO:
		fm_mso_srch_by_legacy_acc_no(ctxp, i_flistp, &ret_flistp, ebufp);
		break;
	 case SRCH_MODEM_BY_MAC_ID:
	 case SRCH_ROUTER_BY_MAC_ID:
	 case SRCH_CABLE_ROUTER_BY_MAC_ID:
	 case SRCH_MODEM_BY_STATE_LCO:
	 case SRCH_ROUTER_BY_STATE_LCO:
	 case SRCH_ROUTER_CAB_BY_STATE_LCO:
	 case SRCH_NAT_BY_DEVICE_ID:
	 case SRCH_NAT_BY_STATE_LCO:
	 case SRCH_DEVICE_BY_SL_NO:
		 fm_mso_srch_bb_devices(ctxp, i_flistp, &ret_flistp, ebufp);
		break;
	 case SRCH_PROD_PRICE_BY_POID:
		 fm_mso_get_prod_price_details(ctxp, i_flistp, &ret_flistp, ebufp);
		 break;		
	 case SRCH_PAYTERM_FOR_PLAN:
		 fm_mso_get_payterm_for_plan(ctxp, i_flistp, &ret_flistp, ebufp);
		 break;

	// Added by Muthu on 14th Oct 2015 - Search by Sales Person First Name
	case SRCH_SALES_PERSON_FIRST_NAME:
		fm_mso_srch_by_salesperson_firstname(ctxp, i_flistp, &ret_flistp, ebufp);
		break;
	//Added by Pavan Bellala 25-11-2015 - Search by Agreement no
	case SRCH_BY_AGREEMENT_NO:
		fm_mso_srch_by_agreement_no(ctxp, i_flistp, &ret_flistp, ebufp);
		break;
	case SRCH_OTT_SWAP_DETAILS:
		fm_mso_search_ott_swap(ctxp, i_flistp, &ret_flistp, ebufp);
		break;
	case SRCH_BY_RMN_NEW :
		fm_mso_srch_by_rmn_new(ctxp, i_flistp, &ret_flistp, ebufp);
		break;

	  default :
		if (SRCH_FOR_OPT_START <= flag && SRCH_FOR_OPT_END >= flag)
		{
			fm_mso_srch_for_opt(ctxp, i_flistp, &ret_flistp, ebufp);
		}
		break;
	}

	//Set error for return
	if (PIN_ERRBUF_IS_ERR(ebufp) || (!ret_flistp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in calling mso_op_search", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51221", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling mso_op_search", ebufp);
		goto CLEANUP;
	}
		
	CLEANUP:
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search output flist", ret_flistp);
	*r_flistp = PIN_FLIST_COPY(ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
	return;
}

/**************************************************
* Function: 	fm_mso_srch_login()
* Owner:        Gautam Adak
* Decription:   Search the login availability
*		
* 
* 
***************************************************/
void
fm_mso_srch_login(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{

	
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	
	int32			srch_flag = 256;

	char			*template = "select X from /mso_customer_credential where F1 = V1 " ;
	char			*loginp = NULL;
 	int64			db = -1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_login input flist", in_flistp);

	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp));
	loginp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_LOGIN, 0, ebufp );
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "Error: Mandatory input fields are missing", in_flistp);
		goto CLEANUP;
	}
	/*******************************************************************
	* Root Login Validation - Start
	*******************************************************************/
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, srch_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_LOGIN, arg_flist, PIN_FLD_LOGIN, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_login input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_login output flist", srch_out_flistp);

	
	CLEANUP:
	*ret_flistp = srch_out_flistp;
	return;
}


/**************************************************
* Function: 	fm_mso_srch_by_acntno_or_poid()
* Owner:        Gautam Adak
* Decription:   Search the login availability
*		
* 
* 
***************************************************/
void
fm_mso_srch_by_acntno_or_poid(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp)
{


        pin_flist_t             *srch_flistp = NULL;
        pin_flist_t             *arg_flist = NULL;
        pin_flist_t             *result_flist = NULL;
        pin_flist_t             *srch_out_flistp = NULL;
        pin_flist_t             *nameinfo_billing = NULL;

        poid_t                  *acnt_pdp = NULL;

        int32                   srch_flag = 768;

        char                    *template = "select X from /account where F1 = V1 " ;
        char                    *template_1 = "select X from /account where F1 = V1 and F2 = V2 " ;
        char                    *template_2 = "select X from /account where F1 = V1 and substr(account_t.business_type,1,2) = 13  " ;
        char                    *acnt_no = NULL;
        int64                   db = -1;
        int32                   *bus_type_ptr = NULL;
        int32                   business_type = -1;
	int			count = 0;
	int			search_fail = 1;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
	PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_acntno input flist", in_flistp);

        db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp));
        acnt_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp );
        acnt_no =  PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp );
	if (!acnt_pdp && !acnt_no)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "Error: Mandatory input fields are missing", in_flistp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp), ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51789", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "At least one of account_poid and account_no is required", ebufp);
		return;
	}

        /*******************************************************************
        * Root Login Validation - Start
        *******************************************************************/
        srch_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, srch_flistp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
        if (acnt_pdp)
        {
                PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, acnt_pdp, ebufp);
        }
        else if (acnt_no)
        {
                PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_NO, acnt_no, ebufp);
        }

	bus_type_ptr =  PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_BUSINESS_TYPE, 1, ebufp );
	if (bus_type_ptr)
        {
		business_type = *bus_type_ptr;
	}

        if(business_type == -1)
        {
                PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
        }
        else
        {
		if(business_type == 13000000 ) //for commercial LCO
		{
       		        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template_2 , ebufp);
                	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
                	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_BUSINESS_TYPE, &business_type, ebufp);
		}
		else
		{
               		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template_1 , ebufp);
                	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
                	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_BUSINESS_TYPE, &business_type, ebufp);
		}
        }


        result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
        PIN_FLIST_FLD_SET(result_flist, MSO_FLD_RMN, NULL , ebufp);
        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_NO, NULL , ebufp);
        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_BUSINESS_TYPE, NULL , ebufp);

        nameinfo_billing = PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_NAMEINFO, 1, ebufp );
        PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_FIRST_NAME, NULL , ebufp);
        PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_MIDDLE_NAME, NULL , ebufp);
        PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_LAST_NAME, NULL , ebufp);
        PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_COMPANY, NULL , ebufp);
        PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_CITY, NULL , ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_acntno input list", srch_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
                goto CLEANUP;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_acntno output flist", srch_out_flistp);

		/* Pawan: 21-NOV-2014 : Added below block to return failure if account is not found */
		count = PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp);
		if(!count) {
			PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
			srch_out_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, srch_out_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(srch_out_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
			PIN_FLIST_FLD_SET(srch_out_flistp, PIN_FLD_ERROR_CODE, "51789", ebufp);
			PIN_FLIST_FLD_SET(srch_out_flistp, PIN_FLD_ERROR_DESCR, "Account number not found.", ebufp);
		}
		/* Added till here */
	
	CLEANUP:
	*ret_flistp = srch_out_flistp;
	return;
}

/**************************************************
* Function: 	fm_mso_srch_by_rmn()
* Owner:        Gautam Adak
* Decription:   Search the login availability
*		
* 
* 
***************************************************/
void
fm_mso_srch_by_rmn(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{

	
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*nameinfo_billing = NULL;
	
	int32			srch_flag = 768;

	char			*template = "select X from /account where F1 = V1 " ;
 	int64			db = -1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_rmn input flist", in_flistp);

	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp));

	/*******************************************************************
	* Root Login Validation - Start
	*******************************************************************/
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, srch_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_RMN, arg_flist, MSO_FLD_RMN, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_NO, NULL , ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_BUSINESS_TYPE, NULL , ebufp);

	nameinfo_billing = PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_NAMEINFO, 1, ebufp );
	PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_FIRST_NAME, NULL , ebufp);
	PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_MIDDLE_NAME, NULL , ebufp);
	PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_LAST_NAME, NULL , ebufp);
	PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_COMPANY, NULL , ebufp);
	PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_CITY, NULL , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_rmn input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_rmn output flist", srch_out_flistp);

	
	CLEANUP:
	*ret_flistp = srch_out_flistp;
	return;
}

/**************************************************
* Function: 	fm_mso_srch_by_name()
* Owner:        Gautam Adak
* Decription:   Search the login availability
*		
* 
* 
***************************************************/
void
fm_mso_srch_by_name(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{

	
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*nameinfo_billing = NULL;
	pin_flist_t		*csr_info = NULL;
	pin_flist_t		*srch_out_flistp_1 = NULL;
	
	int32			srch_flag = 768;

	char			template[300];
	char			template_1[300];

	char			*firstname  = NULL;
	char			*middlename = NULL;
	char			*lastname   = NULL; 

 	int64			db = -1;
	int32			search_fail = 1;
	int32			parent_type_of_csr =0;
	int32			data_access_required =0;
	int32			count =0;
	int32			elem_id =0;
	int32			elem_id_rename =-1;

	pin_cookie_t		cookie = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_name input flist", in_flistp);

	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp));
	firstname  = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_FIRST_NAME, 1, ebufp);
	middlename = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_MIDDLE_NAME, 1, ebufp);
	lastname   = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_LAST_NAME, 1, ebufp);

	csr_info = PIN_FLIST_CREATE(ebufp);
	fm_mso_get_csr_info(ctxp, in_flistp, &csr_info, ebufp);
	parent_type_of_csr = *(int32*)PIN_FLIST_FLD_GET(csr_info, PIN_FLD_ACCOUNT_TYPE, 0, ebufp);
	data_access_required = *(int32*)PIN_FLIST_FLD_GET(csr_info, PIN_FLD_TYPE, 0, ebufp);
	/*******************************************************************
	* Root Login Validation - Start
	*******************************************************************/
	/*******************************************************************
	* Mandatory fields validation
	*******************************************************************/
	if (!firstname && !middlename && !lastname)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "At least one of firstname, middlename & lastname is required") ;
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp), ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51223", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "At least one of firstname, middlename & lastname is required", ebufp);
		return;
	}

	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, srch_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);

	if (firstname && middlename && lastname)
	{
		//template = pin_malloc(300);
		memset(template, '\0', 300);
		
		if (parent_type_of_csr == ACCT_TYPE_CSR_ADMIN )
		{
			strcpy(template, "select X from /account where upper(F1) like V1 and upper(F2) like V2 and upper(F3) like V3 ");
		}
		else if (data_access_required)
		{
			strcpy(template,    "select X from /account 1, /profile/wholesale 2 where upper(1.F1) like V1 and upper(1.F2) like V2 and upper(1.F3) like V3 and 1.F4 = 2.F5 and 2.F6 = V6 and 1.F7 = V7 ");
			//template_1 = pin_malloc(300);
			memset(template_1, '\0', 300);
			strcpy(template_1,    "select X from /account 1, /profile/customer_care 2 where upper(1.F1) like V1 and upper(1.F2) like V2 and upper(1.F3) like V3 and 1.F4 = 2.F5 and 2.F6 = V6 and 1.F7 = V7 ");
			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL, ebufp );

			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 5, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp );

			PIN_FLIST_ELEM_COPY(csr_info, PIN_FLD_ARGS, 1, srch_flistp, PIN_FLD_ARGS, 6, ebufp);
			PIN_FLIST_ELEM_COPY(csr_info, PIN_FLD_ARGS, 2, srch_flistp, PIN_FLD_ARGS, 7, ebufp);

		}
		else
		{
			strcpy(template,    "select X from /account 1, /profile/wholesale 2 where upper(1.F1) like V1 and upper(1.F2) like V2 and upper(1.F3) like V3 and 1.F4 = 2.F5 and 2.F6 = V6 ");
			//template_1 = pin_malloc(300);
			memset(template_1, '\0', 300);
			strcpy(template_1,    "select X from /account 1, /profile/customer_care 2 where upper(1.F1) like V1 and upper(1.F2) like V2 and upper(1.F3) like V3 and 1.F4 = 2.F5 and 2.F6 = V6 ");
			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL, ebufp );

			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 5, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp );

			PIN_FLIST_ELEM_COPY(csr_info, PIN_FLD_ARGS, 1, srch_flistp, PIN_FLD_ARGS, 6, ebufp);
		}
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );	
		nameinfo_billing = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_NAMEINFO, 1, ebufp );
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_FIRST_NAME, nameinfo_billing, PIN_FLD_FIRST_NAME, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );	
		nameinfo_billing = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_NAMEINFO, 1, ebufp );
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_MIDDLE_NAME, nameinfo_billing, PIN_FLD_MIDDLE_NAME, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );	
		nameinfo_billing = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_NAMEINFO, 1, ebufp );
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_LAST_NAME, nameinfo_billing, PIN_FLD_LAST_NAME, ebufp);

		elem_id_rename =6;
	}
	else if (firstname && middlename)
	{
		//template = pin_malloc(300);
		memset(template, '\0', 300);
		
		if (parent_type_of_csr == ACCT_TYPE_CSR_ADMIN )
		{
			strcpy(template, "select X from /account where upper(F1) like V1 and upper(F2) like V2 ");
		}
		else if (data_access_required)
		{
			strcpy(template,    "select X from /account 1, /profile/wholesale 2 where upper(1.F1) like V1 and upper(1.F2) like V2 and 1.F3 = 2.F4 and 2.F5 = V5 and 1.F6 = V6 ");
			//template_1 = pin_malloc(300);
			memset(template_1, '\0', 300);
			strcpy(template_1,  "select X from /account 1, /profile/customer_care 2 where upper(1.F1) like V1 and upper(1.F2) like V2 and 1.F3 = 2.F4 and 2.F5 = V5 and 1.F6 = V6 ");
			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL, ebufp );

			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp );

			PIN_FLIST_ELEM_COPY(csr_info, PIN_FLD_ARGS, 1, srch_flistp, PIN_FLD_ARGS, 5, ebufp);
			PIN_FLIST_ELEM_COPY(csr_info, PIN_FLD_ARGS, 2, srch_flistp, PIN_FLD_ARGS, 6, ebufp);

		}
		else
		{
			strcpy(template,    "select X from /account 1, /profile/wholesale 2 where upper(1.F1) like V1 and upper(1.F2) like V2 and 1.F3 = 2.F4 and 2.F5 = V5 ");
			//template_1 = pin_malloc(300);
			memset(template_1, '\0', 300);
			strcpy(template_1,    "select X from /account 1, /profile/customer_care 2 where upper(1.F1) like V1 and upper(1.F2) like V2 and 1.F3 = 2.F4 and 2.F5 = V5 ");
			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL, ebufp );

			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp );

			PIN_FLIST_ELEM_COPY(csr_info, PIN_FLD_ARGS, 1, srch_flistp, PIN_FLD_ARGS, 5, ebufp);
		}
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );	
		nameinfo_billing = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_NAMEINFO, 1, ebufp );
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_FIRST_NAME, nameinfo_billing, PIN_FLD_FIRST_NAME, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );	
		nameinfo_billing = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_NAMEINFO, 1, ebufp );
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_MIDDLE_NAME, nameinfo_billing, PIN_FLD_MIDDLE_NAME, ebufp);

		elem_id_rename =5;
	}
	else if (firstname && lastname)
	{
		//template = pin_malloc(300);
		memset(template, '\0', 300);
		
		if (parent_type_of_csr == ACCT_TYPE_CSR_ADMIN )
		{
			strcpy(template, "select X from /account where upper(F1) like V1 and upper(F2) like V2 ");
		}
		else if (data_access_required)
		{
			strcpy(template,    "select X from /account 1, /profile/wholesale 2 where upper(1.F1) like V1 and upper(1.F2) like V2 and 1.F3 = 2.F4 and 2.F5 = V5 and 1.F6 = V6 ");
			//template_1 = pin_malloc(300);
			memset(template_1, '\0', 300);
			strcpy(template_1,  "select X from /account 1, /profile/customer_care 2 where upper(1.F1) like V1 and upper(1.F2) like V2 and 1.F3 = 2.F4 and 2.F5 = V5 and 1.F6 = V6 ");
			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL, ebufp );

			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp );

			PIN_FLIST_ELEM_COPY(csr_info, PIN_FLD_ARGS, 1, srch_flistp, PIN_FLD_ARGS, 5, ebufp);
			PIN_FLIST_ELEM_COPY(csr_info, PIN_FLD_ARGS, 2, srch_flistp, PIN_FLD_ARGS, 6, ebufp);

		}
		else
		{
			strcpy(template,    "select X from /account 1, /profile/wholesale 2 where upper(1.F1) like V1 and upper(1.F2) like V2 and 1.F3 = 2.F4 and 2.F5 = V5 ");
			//template_1 = pin_malloc(300);
			memset(template_1, '\0', 300);
			strcpy(template_1,    "select X from /account 1, /profile/customer_care 2 where upper(1.F1) like V1 and upper(1.F2) like V2 and 1.F3 = 2.F4 and 2.F5 = V5 ");
			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL, ebufp );

			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp );

			PIN_FLIST_ELEM_COPY(csr_info, PIN_FLD_ARGS, 1, srch_flistp, PIN_FLD_ARGS, 5, ebufp);
		}
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );	
		nameinfo_billing = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_NAMEINFO, 1, ebufp );
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_FIRST_NAME, nameinfo_billing, PIN_FLD_FIRST_NAME, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );	
		nameinfo_billing = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_NAMEINFO, 1, ebufp );
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_LAST_NAME, nameinfo_billing, PIN_FLD_LAST_NAME, ebufp);

		elem_id_rename =5;
	}
	else if (firstname )
	{
		//template = pin_malloc(300);
		memset(template, '\0', 300);
		
		if (parent_type_of_csr == ACCT_TYPE_CSR_ADMIN )
		{
			strcpy(template, "select X from /account where upper(F1) like V1 ");
		}
		else if (data_access_required)
		{
			strcpy(template,    "select X from /account 1, /profile/wholesale 2 where upper(1.F1) like V1 and 1.F2 = 2.F3 and 2.F4 = V4 and 1.F5 = V5 ");
			//template_1 = pin_malloc(300);
			memset(template_1, '\0', 300);
			strcpy(template_1, "select X from /account 1, /profile/customer_care 2 where upper(1.F1) like V1 and 1.F2 = 2.F3 and 2.F4 = V4 and 1.F5 = V5 ");
			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL, ebufp );

			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp );

			PIN_FLIST_ELEM_COPY(csr_info, PIN_FLD_ARGS, 1, srch_flistp, PIN_FLD_ARGS, 4, ebufp);
			PIN_FLIST_ELEM_COPY(csr_info, PIN_FLD_ARGS, 2, srch_flistp, PIN_FLD_ARGS, 5, ebufp);

		}
		else
		{
			strcpy(template,    "select X from /account 1, /profile/wholesale 2 where upper(1.F1) like V1 and 1.F2 = 2.F3 and 2.F4 = V4 ");
			//template_1 = pin_malloc(300);
			memset(template_1, '\0', 300);
			strcpy(template_1, "select X from /account 1, /profile/customer_care 2 where upper(1.F1) like V1 and 1.F2 = 2.F3 and 2.F4 = V4 ");
			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL, ebufp );

			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp );

			PIN_FLIST_ELEM_COPY(csr_info, PIN_FLD_ARGS, 1, srch_flistp, PIN_FLD_ARGS, 4, ebufp);
		}
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );	
		nameinfo_billing = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_NAMEINFO, 1, ebufp );
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_FIRST_NAME, nameinfo_billing, PIN_FLD_FIRST_NAME, ebufp);

		elem_id_rename =4;
		
	}
	else if (middlename )
	{
		//template = pin_malloc(300);
		memset(template, '\0', 300);
		
		if (parent_type_of_csr == ACCT_TYPE_CSR_ADMIN )
		{
			strcpy(template, "select X from /account where upper(F1) like V1 ");
		}
		else if (data_access_required)
		{
			strcpy(template,    "select X from /account 1, /profile/wholesale 2 where upper(1.F1) like V1 and 1.F2 = 2.F3 and 2.F4 = V4 and 1.F5 = V5 ");
			//template_1 = pin_malloc(300);
			memset(template_1, '\0', 300);
			strcpy(template_1, "select X from /account 1, /profile/customer_care 2 where upper(1.F1) like V1 and 1.F2 = 2.F3 and 2.F4 = V4 and 1.F5 = V5 ");
			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL, ebufp );

			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp );

			PIN_FLIST_ELEM_COPY(csr_info, PIN_FLD_ARGS, 1, srch_flistp, PIN_FLD_ARGS, 4, ebufp);
			PIN_FLIST_ELEM_COPY(csr_info, PIN_FLD_ARGS, 2, srch_flistp, PIN_FLD_ARGS, 5, ebufp);

		}
		else
		{
			strcpy(template,    "select X from /account 1, /profile/wholesale 2 where upper(1.F1) like V1 and 1.F2 = 2.F3 and 2.F4 = V4 ");
			//template_1 = pin_malloc(300);
			memset(template_1, '\0', 300);
			strcpy(template_1, "select X from /account 1, /profile/customer_care 2 where upper(1.F1) like V1 and 1.F2 = 2.F3 and 2.F4 = V4 ");
			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL, ebufp );

			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp );

			PIN_FLIST_ELEM_COPY(csr_info, PIN_FLD_ARGS, 1, srch_flistp, PIN_FLD_ARGS, 4, ebufp);
		}
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );	
		nameinfo_billing = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_NAMEINFO, 1, ebufp );
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_MIDDLE_NAME, nameinfo_billing, PIN_FLD_MIDDLE_NAME, ebufp);

		elem_id_rename =4;
	}
	else if (lastname )
	{
		//template = pin_malloc(300);
		memset(template, '\0', 300);
		
		if (parent_type_of_csr == ACCT_TYPE_CSR_ADMIN )
		{
			strcpy(template, "select X from /account where upper(F1) like V1 ");
		}
		else if (data_access_required)
		{
			strcpy(template,    "select X from /account 1, /profile/wholesale 2 where upper(1.F1) like V1 and 1.F2 = 2.F3 and 2.F4 = V4 and 1.F5 = V5 ");
			//template_1 = pin_malloc(300);
			memset(template_1, '\0', 300);
			strcpy(template_1, "select X from /account 1, /profile/customer_care 2 where upper(1.F1) like V1 and 1.F2 = 2.F3 and 2.F4 = V4 and 1.F5 = V5 ");
			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL, ebufp );

			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp );

			PIN_FLIST_ELEM_COPY(csr_info, PIN_FLD_ARGS, 1, srch_flistp, PIN_FLD_ARGS, 4, ebufp);
			PIN_FLIST_ELEM_COPY(csr_info, PIN_FLD_ARGS, 2, srch_flistp, PIN_FLD_ARGS, 5, ebufp);

		}
		else
		{
			strcpy(template,    "select X from /account 1, /profile/wholesale 2 where upper(1.F1) like V1 and 1.F2 = 2.F3 and 2.F4 = V4 ");
			//template_1 = pin_malloc(300);
			memset(template_1, '\0', 300);
			strcpy(template_1, "select X from /account 1, /profile/customer_care 2 where upper(1.F1) like V1 and 1.F2 = 2.F3 and 2.F4 = V4 ");
			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL, ebufp );

			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp );

			PIN_FLIST_ELEM_COPY(csr_info, PIN_FLD_ARGS, 1, srch_flistp, PIN_FLD_ARGS, 4, ebufp);
		}
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );	
		nameinfo_billing = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_NAMEINFO, 1, ebufp );
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_LAST_NAME, nameinfo_billing, PIN_FLD_LAST_NAME, ebufp);

		elem_id_rename =4;
	}
	PIN_FLIST_DESTROY_EX(&csr_info, NULL);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, SRCH_BY_NAME_LIMIT, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_NO, NULL , ebufp);
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_RMN, NULL , ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_BUSINESS_TYPE, NULL , ebufp);

	nameinfo_billing = PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_NAMEINFO, 1, ebufp );
	PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_FIRST_NAME, NULL , ebufp);
	PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_MIDDLE_NAME, NULL , ebufp);
	PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_LAST_NAME, NULL , ebufp);
	PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_COMPANY, NULL , ebufp);
	PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_CITY, NULL , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_name input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
	//PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_name output flist", srch_out_flistp);

	/*******************************************************
	* call for /profile/customer_care -Start
	*******************************************************/
	if (parent_type_of_csr != ACCT_TYPE_CSR_ADMIN )
	{
		PIN_FLIST_FLD_DROP(srch_flistp, PIN_FLD_TEMPLATE, ebufp);
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template_1 , ebufp);
		arg_flist = PIN_FLIST_ELEM_GET(srch_flistp, PIN_FLD_ARGS, elem_id_rename, 0, ebufp);
		PIN_FLIST_FLD_RENAME(arg_flist, MSO_FLD_WHOLESALE_INFO, PIN_FLD_CUSTOMER_CARE_INFO, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_name input list", srch_flistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp_1, ebufp);
		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
			goto CLEANUP;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_name output flist", srch_out_flistp_1);
		/*******************************************************
		* call for /profile/customer_care -End
		* Merge the results
		*******************************************************/
		count = PIN_FLIST_ELEM_COUNT(srch_out_flistp_1, PIN_FLD_RESULTS, ebufp);
		while((result_flist = PIN_FLIST_ELEM_GET_NEXT(srch_out_flistp, PIN_FLD_RESULTS,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			PIN_FLIST_ELEM_COPY(srch_out_flistp, PIN_FLD_RESULTS, elem_id, srch_out_flistp_1, PIN_FLD_RESULTS, count, ebufp);
			count ++;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_name output flist", srch_out_flistp_1);
	}
	else
	{
		srch_out_flistp_1 = PIN_FLIST_COPY(srch_out_flistp, ebufp);
	}
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	CLEANUP:
	*ret_flistp = srch_out_flistp_1;
	return;
}

/**************************************************
* Function: 	fm_mso_srch_by_company()
* Owner:        Gautam Adak
* Decription:   Search the login availability
*		
* 
* 
***************************************************/
void
fm_mso_srch_by_company(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{

	
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*nameinfo_billing = NULL;
	
	int32			srch_flag = 768;

	// Changed by Muthu on 14th Oct 2015 - Include business type for company search
	// Currently the value is hardcoded 
	//char			*template = "select X from /account where upper(F1) like V1 " ;
	char			*template = "select X from /account where substr(account_t.business_type,1,2) = 13 and upper(F1) like V1 " ;
 	int64			db = -1;
	void			*vp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	// Field validation for mandatory field
	vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_COMPANY, 0, ebufp);
	if ( vp == NULL )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"Error in SEARCH for company name - Company Name field missing in input", ebufp);
		return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_company input flist", in_flistp);

	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp));

	/*******************************************************************
	* Root Login Validation - Start
	*******************************************************************/
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, srch_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	nameinfo_billing = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_NAMEINFO, 1, ebufp );
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_COMPANY, nameinfo_billing, PIN_FLD_COMPANY, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_NO, NULL , ebufp);
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_RMN, NULL , ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_BUSINESS_TYPE, NULL , ebufp);

	nameinfo_billing = PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_NAMEINFO, 1, ebufp );
	PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_FIRST_NAME, NULL , ebufp);
	PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_MIDDLE_NAME, NULL , ebufp);
	PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_LAST_NAME, NULL , ebufp);
	PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_COMPANY, NULL , ebufp);
	PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_CITY, NULL , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_company input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_company output flist", srch_out_flistp);

	
	CLEANUP:
	*ret_flistp = srch_out_flistp;
	return;
}

/**************************************************
* Function: 	fm_mso_srch_by_card_no()
* Owner:        Gautam Adak
* Decription:   Search the login availability
*		
* 
* 
***************************************************/
void
fm_mso_srch_by_card_no(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{

	
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*nameinfo_billing = NULL;
	pin_flist_t		*alias_list = NULL;
	
	int32			srch_flag = 768;
	int32			den_sp = 0;
	char			*template = "select X from /account 1, /service 2 where 2.F2 = 1.F3 and  2.F1 = V1 " ;
	char			*template1 = "select X from /account 1, /device 2 where 2.F4 = 1.F3 and  2.F1 = V1 and 2.F2 = V2 " ;
	char			*den_nwp = "NDS1";
	char			*dsn_nwp = "NDS2";
	char			*hw_nwp = "NDS";
	char			*namep = NULL;
 	int64			db = -1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_card_no input flist", in_flistp);

	den_sp = fm_mso_is_den_sp(ctxp, in_flistp, ebufp);

	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp));
	namep = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_NAME, 0, ebufp);

	/*******************************************************************
	* Root Login Validation - Start
	*******************************************************************/
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, srch_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);

	if (namep && strncmp(namep, "000", 3) == 0)
        {
                PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template1, ebufp);

                arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
                PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_DEVICE_ID, namep, ebufp);

                arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
                if (den_sp == 1)
                {
                        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_MANUFACTURER, den_nwp, ebufp);
                }
				else if (den_sp == 2)
                {
                        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_MANUFACTURER, dsn_nwp, ebufp);
                }
                else
                {
                        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_MANUFACTURER, hw_nwp, ebufp);
                }

                arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp );
                arg_flist = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_SERVICES, 0, ebufp );
                PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, NULL , ebufp);
        }
        else
        {
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template, ebufp);

                arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
                alias_list = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_ALIAS_LIST, PIN_ELEMID_ANY, ebufp );
                PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_NAME, alias_list, PIN_FLD_NAME, ebufp);

                arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
                PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, NULL , ebufp);
	}
	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL , ebufp);
	
	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_NO, NULL , ebufp);
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_RMN, NULL , ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_BUSINESS_TYPE, NULL , ebufp);

	nameinfo_billing = PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_NAMEINFO, 1, ebufp );
	PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_FIRST_NAME, NULL , ebufp);
	PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_MIDDLE_NAME, NULL , ebufp);
	PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_LAST_NAME, NULL , ebufp);
	PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_COMPANY, NULL , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_card_no input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_card_no output flist", srch_out_flistp);

	
	CLEANUP:
	*ret_flistp = srch_out_flistp;
	return;
}

/**************************************************
* Function: 	fm_mso_srch_trn_history()
* Owner:        Gautam Adak
* Decription:   Search the login availability
*		
* 
* 
***************************************************/
void
fm_mso_srch_trn_history(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{

	
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*bal_impacts = NULL;
        pin_flist_t             *result_out_flist = NULL;
        pin_flist_t             *bal_out_flist =  NULL;
	pin_flist_t		*total_flist = NULL;

	int32			*inp_flag = 0;
	
	int32			srch_flag = 768;

	char			template[512];
 	int64			db = -1;
        int32                   elem_id =0;
        int32                   elem_id_bal =0;
        int32           	sign_positive = 1;
        int32           	sign_negative = 0;
        int32           	res_id_INR = 356;
        int32           	res_id=0;
	int32			wallet_res_id=2300000;


        pin_cookie_t            cookie = NULL;
        pin_cookie_t            cookie_bal = NULL;
	pin_decimal_t   	*amountp = (pin_decimal_t *)NULL;
	pin_decimal_t   	*totalp = (pin_decimal_t *)NULL;
	pin_decimal_t		*wallet_balp = (pin_decimal_t *)NULL;

	double          	zero_bal = 0;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_trn_history input flist", in_flistp);


	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp));
	inp_flag = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_TYPE, 0, ebufp);

	/*******************************************************************
	* Root Login Validation - Start
	*******************************************************************/
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, srch_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_OBJ, arg_flist, PIN_FLD_ACCOUNT_OBJ, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_START_T, arg_flist, PIN_FLD_END_T, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_END_T, arg_flist, PIN_FLD_END_T, ebufp);
	if(*inp_flag == TRANS_SRCH_ONETIME )
	{
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp );
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/event/billing/product/fee/purchase%", -1, ebufp), ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 5, ebufp);
		bal_impacts = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_BAL_IMPACTS, 0, ebufp);
		PIN_FLIST_FLD_SET(bal_impacts, PIN_FLD_RESOURCE_ID, &res_id_INR, ebufp);

		memset(template,256,'\0');
		strcpy(template,"select X from /event where F1 = V1 and F2 >= V2 and F3 <= V3 and F4 like V4 and F5 = V5 order by end_t asc ");
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
	}
	else if (*inp_flag == TRANS_SRCH_MONTHLY)
	{
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp );
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/event/billing/product/fee/cycle%", -1, ebufp), ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 5, ebufp);
		bal_impacts = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_BAL_IMPACTS, 0, ebufp);
		PIN_FLIST_FLD_SET(bal_impacts, PIN_FLD_RESOURCE_ID, &res_id_INR, ebufp);

		memset(template,256,'\0');
		strcpy(template,"select X from /event where F1 = V1 and F2 >= V2 and F3 <= V3 and F4 like V4 and F5 = V5 order by end_t asc ");
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
	}
	else if (*inp_flag == TRANS_SRCH_PAYMENT)
	{
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp );
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/event/billing/payment%", -1, ebufp), ebufp);
                arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 5, ebufp );
                PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/event/billing/reversal%", -1, ebufp), ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 6, ebufp);
		bal_impacts = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_BAL_IMPACTS, 0, ebufp);
		PIN_FLIST_FLD_SET(bal_impacts, PIN_FLD_RESOURCE_ID, &res_id_INR, ebufp);

		memset(template,256,'\0');
		strcpy(template,"select X from /event where F1 = V1 and F2 >= V2 and F3 <= V3 and ( F4 like V4 or F5 like V5 ) and F6 = V6 order by end_t asc ");
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
	}
	else if (*inp_flag == TRANS_SRCH_ADJUSTMENT)
	{
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp );
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/event/billing/adjustment%", -1, ebufp), ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 5, ebufp);
		bal_impacts = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_BAL_IMPACTS, 0, ebufp);
		PIN_FLIST_FLD_SET(bal_impacts, PIN_FLD_RESOURCE_ID, &res_id_INR, ebufp);
		memset(template,256,'\0');
		strcpy(template,"select X from /event where F1 = V1 and F2 >= V2 and F3 <= V3 and F4 like V4 and F5 = V5 order by end_t asc ");
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
	}
	else if (*inp_flag == TRANS_SRCH_ALL)
	{
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp );
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/event/billing/product/fee/purchase%", -1, ebufp), ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 5, ebufp );
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/event/billing/product/fee/cycle%", -1, ebufp), ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 6, ebufp );
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/event/billing/payment%", -1, ebufp), ebufp);
                arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 7, ebufp );
                PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/event/billing/reversal%", -1, ebufp), ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 8, ebufp );
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/event/billing/adjustment%", -1, ebufp), ebufp);
                arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 9, ebufp );
                PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/event/billing/cycle/tax%", -1, ebufp), ebufp);
                arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 10, ebufp );
                PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/event/billing/dispute%", -1, ebufp), ebufp);
                arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 11, ebufp );
                PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/event/billing/mso_catv_commitment", -1, ebufp), ebufp);
                arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 12, ebufp );
                PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/event/billing/settlement%", -1, ebufp), ebufp);
                arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 13, ebufp );
                PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/event/mso_et%", -1, ebufp), ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 14, ebufp);
		bal_impacts = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_BAL_IMPACTS, 0, ebufp);
		PIN_FLIST_FLD_SET(bal_impacts, PIN_FLD_RESOURCE_ID, &res_id_INR, ebufp);

		memset(template,512,'\0');
		strcpy(template,"select X from /event where F1 = V1 and F2 >= V2 and F3 <= V3 and ( F4 like V4 or F5 like V5 or F6 like V6 or F7 like V7 or F8 like V8 or F9 like V9 or F10 like V10 or F11 like V11 or F12 like V12 or F13 like V13 ) and F14 = V14 order by end_t asc ");
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
	}
	else if (*inp_flag == TRANS_SRCH_WALLET)
	{
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp );
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/event/billing/product/fee/purchase%", -1, ebufp), ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 5, ebufp );
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/event/billing/product/fee/cycle%", -1, ebufp), ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 6, ebufp );
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/event/billing/payment%", -1, ebufp), ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 7, ebufp );
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/event/billing/reversal%", -1, ebufp), ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 8, ebufp );
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/event/billing/adjustment%", -1, ebufp), ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 9, ebufp );
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/event/mso_et%", -1, ebufp), ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 10, ebufp );
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/event/billing/debit", -1, ebufp), ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 11, ebufp);
		bal_impacts = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_BAL_IMPACTS, 0, ebufp);
		PIN_FLIST_FLD_SET(bal_impacts, PIN_FLD_RESOURCE_ID, &wallet_res_id, ebufp);

		memset(template,512,'\0');
		strcpy(template,"select X from /event where F1 = V1 and F2 >= V2 and F3 <= V3 and ( F4 like V4 or F5 like V5 or F6 like V6 or F7 like V7 or F8 like V8 or F9 like V9 or F10 = V10 ) and F11 = V11 order by end_t asc ");
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
	}
	else if (*inp_flag == TRANS_SRCH_LIFECYCLE)
	{
                arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp );
                PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/event/activity/mso_lifecycle%", -1, ebufp), ebufp);
		memset(template,512,'\0');
		strcpy(template,"select X from /event where F1 = V1 and F2 >= V2 and F3 <= V3 and ( F4 like V4 ) order by end_t asc ");
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_DESCR, NULL , ebufp);
	}
	else
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		goto CLEANUP;	
	}
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_PROGRAM_NAME, NULL , ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_CREATED_T, NULL , ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_SYS_DESCR, NULL , ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_END_T, NULL , ebufp);
	bal_impacts = PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_BAL_IMPACTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(bal_impacts, PIN_FLD_AMOUNT, NULL , ebufp);
	PIN_FLIST_FLD_SET(bal_impacts, PIN_FLD_RESOURCE_ID, NULL , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_trn_history search input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_trn_history search output flist", srch_out_flistp);
		
	if(srch_out_flistp)
	{
		totalp = pbo_decimal_from_double(zero_bal, ebufp);
		while((result_out_flist = PIN_FLIST_ELEM_GET_NEXT(srch_out_flistp, PIN_FLD_RESULTS,
                        &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
                {
			while((bal_out_flist = PIN_FLIST_ELEM_GET_NEXT(result_out_flist, PIN_FLD_BAL_IMPACTS,
				&elem_id_bal, 1, &cookie_bal, ebufp)) != (pin_flist_t *)NULL)
			{
				res_id = *((int32 *)PIN_FLIST_FLD_GET(bal_out_flist, PIN_FLD_RESOURCE_ID, 0, ebufp));
				if(res_id == res_id_INR)
				{
					amountp = (pin_decimal_t *)PIN_FLIST_FLD_GET(bal_out_flist, PIN_FLD_AMOUNT, 0, ebufp);
					pbo_decimal_add_assign(totalp, amountp, ebufp);
				}
				else if (res_id == wallet_res_id)
				{
					wallet_balp = (pin_decimal_t *)PIN_FLIST_FLD_GET(bal_out_flist, PIN_FLD_AMOUNT, 0, ebufp);
				}
			}
			if (*inp_flag != TRANS_SRCH_WALLET)
			{
			total_flist = PIN_FLIST_ELEM_ADD(result_out_flist, PIN_FLD_TOTAL, 0, ebufp );
			PIN_FLIST_FLD_SET(total_flist, PIN_FLD_AMOUNT, totalp, ebufp);
			PIN_FLIST_FLD_SET(total_flist, PIN_FLD_RESOURCE_ID, &res_id_INR, ebufp);
			if(pbo_decimal_sign(totalp, ebufp) == -1)
			{
				PIN_FLIST_FLD_SET(total_flist, PIN_FLD_TYPE, &sign_negative, ebufp);
			}
			else 
			{
				PIN_FLIST_FLD_SET(total_flist, PIN_FLD_TYPE, &sign_positive, ebufp);
			}
			}
			if (wallet_balp && !pbo_decimal_is_zero(wallet_balp, ebufp))
			{
				total_flist = PIN_FLIST_ELEM_ADD(result_out_flist, PIN_FLD_TOTAL, 1, ebufp );
				PIN_FLIST_FLD_SET(total_flist, PIN_FLD_AMOUNT, wallet_balp, ebufp);
				PIN_FLIST_FLD_SET(total_flist, PIN_FLD_RESOURCE_ID, &wallet_res_id, ebufp);
				if (pbo_decimal_sign(wallet_balp, ebufp) == -1)
				{
					PIN_FLIST_FLD_SET(total_flist, PIN_FLD_TYPE, &sign_negative, ebufp);
				}
				else
				{
					PIN_FLIST_FLD_SET(total_flist, PIN_FLD_TYPE, &sign_positive, ebufp);
				}	
			}	
			pbo_decimal_destroy(&totalp);
			totalp = pbo_decimal_from_double(zero_bal, ebufp);
			elem_id_bal = 0;
			cookie_bal = NULL;
			if(PIN_FLIST_ELEM_COUNT(result_out_flist,PIN_FLD_BAL_IMPACTS,ebufp) > 0)
			{
				PIN_FLIST_ELEM_DROP(result_out_flist, PIN_FLD_BAL_IMPACTS, PIN_ELEMID_ANY, ebufp);
			}
		}
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_trn_history output flist", srch_out_flistp);
		
	
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	*ret_flistp = srch_out_flistp;
	return;
}

/**************************************************
* Function: 	fm_mso_get_csr_info()
* Owner:        Gautam Adak
* Decription:   Search the login availability
*		
* 
* 
***************************************************/
void
fm_mso_get_csr_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{

	
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*bal_impacts = NULL;
	pin_flist_t		*wholesale_info = NULL;
	pin_flist_t		*nameinfo = NULL;
	pin_flist_t		*profile_info = NULL;
	
	int32			srch_flag = 768;

	poid_t			*userid = NULL;
	poid_t			*parent_of_csr = NULL;
	poid_t			*jv_obj = NULL;
	poid_t			*dt_obj = NULL;
	poid_t			*sdt_obj = NULL;

	char			*template_1 = "select X from /mso_customer_credential where F1.id = V1 " ;
	char			*template_2 = "select X from /profile/wholesale where F1.id = V1 and profile_t.poid_type='/profile/wholesale' " ;
	char			*access_str = NULL;
	char			data_access[100];
	char			*access_level = NULL;
	char			*access_value = NULL;

 	int64			db = -1;
	int32			parent_type_of_csr =0;
	int32			data_access_required =0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_csr_info input flist", in_flistp);

	userid = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_USERID, 0, ebufp);
	db = PIN_POID_GET_DB(userid);

	/*******************************************************************
	* Search Data Access - Start
	*******************************************************************/
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template_1 , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, userid , ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_DATA_ACCESS, NULL , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_csr_info input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_csr_info output flist", srch_out_flistp);

	result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	if (result_flist)
	{
		access_str = (char*)PIN_FLIST_FLD_GET(result_flist, MSO_FLD_DATA_ACCESS, 1, ebufp );
		if (access_str && !( strcmp(access_str, "")==0 || strcmp(access_str, "*")==0 ))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "access_str");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, access_str);
			data_access_required =1;
			arg_flist = PIN_FLIST_ELEM_ADD(*ret_flistp, PIN_FLD_ARGS, 2, ebufp);
			nameinfo =  PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_NAMEINFO, 1, ebufp);

			//data_access = pin_malloc(sizeof(access_str));
			memset(data_access, '\0',sizeof(access_str));
			strcpy(data_access, access_str);

			access_level = strtok(data_access, "|");
			access_value = strtok(NULL, "|");

			if (strcmp(access_level , "STATE") == 0)
			{
				PIN_FLIST_FLD_SET(nameinfo, PIN_FLD_STATE, access_value, ebufp);
			}
			else if (strcmp(access_level , "CITY") == 0)
			{
				PIN_FLIST_FLD_SET(nameinfo, PIN_FLD_CITY, access_value, ebufp);
			}
			else if (strcmp(access_level , "ZIP") == 0)
			{
				PIN_FLIST_FLD_SET(nameinfo, PIN_FLD_ZIP, access_value, ebufp);
			}
		}
	}
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	/*******************************************************************
	* Search Data Access - End
	*******************************************************************/
	/*******************************************************************
	* Search wholesale Profile - Start
	*******************************************************************/
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template_2 , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, userid , ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_WHOLESALE_INFO, NULL , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_csr_info input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_csr_info output flist", srch_out_flistp);

	result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	if (result_flist)
	{
		wholesale_info = PIN_FLIST_SUBSTR_GET(result_flist, MSO_FLD_WHOLESALE_INFO, 0, ebufp);
		parent_of_csr = PIN_POID_COPY( PIN_FLIST_FLD_GET(wholesale_info, PIN_FLD_PARENT, 0, ebufp), ebufp);
		jv_obj = PIN_FLIST_FLD_GET(wholesale_info, MSO_FLD_JV_OBJ, 0, ebufp );
		dt_obj = PIN_FLIST_FLD_GET(wholesale_info, MSO_FLD_DT_OBJ, 0, ebufp );
		sdt_obj = PIN_FLIST_FLD_GET(wholesale_info, MSO_FLD_SDT_OBJ, 0, ebufp );

		arg_flist = PIN_FLIST_ELEM_ADD(*ret_flistp, PIN_FLD_ARGS, 1, ebufp);
		profile_info =  PIN_FLIST_SUBSTR_ADD(arg_flist, MSO_FLD_WHOLESALE_INFO, ebufp);

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
			parent_type_of_csr = ACCT_TYPE_JV;
			PIN_FLIST_FLD_SET(profile_info, MSO_FLD_JV_OBJ, parent_of_csr, ebufp );
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ACCT_TYPE_JV");
		}
		else if ( parent_of_csr &&
			  PIN_POID_COMPARE(parent_of_csr, dt_obj, 0, ebufp ) == 0)
		{
			parent_type_of_csr = ACCT_TYPE_DTR;
			PIN_FLIST_FLD_SET(profile_info, MSO_FLD_DT_OBJ, parent_of_csr, ebufp );
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ACCT_TYPE_DTR");
		}
		else if ( parent_of_csr &&
			  PIN_POID_COMPARE(parent_of_csr, sdt_obj, 0, ebufp ) == 0)
		{
			parent_type_of_csr = ACCT_TYPE_SUB_DTR;
			PIN_FLIST_FLD_SET(profile_info, MSO_FLD_SDT_OBJ, parent_of_csr, ebufp );
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ACCT_TYPE_SUB_DTR");
		}
		else
		{
			parent_type_of_csr = ACCT_TYPE_LCO;
			PIN_FLIST_FLD_SET(profile_info, PIN_FLD_PARENT, parent_of_csr, ebufp );
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ACCT_TYPE_LCO");
		}
	}
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	/*******************************************************************
	* Search wholesale Profile - End
	*******************************************************************/
	/*******************************************************************
	* Create Return Flist- Start
	*******************************************************************/
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_TYPE, &data_access_required, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ACCOUNT_TYPE, &parent_type_of_csr, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ret_flistp", *ret_flistp);
	/*******************************************************************
	* Create Return Flist- End
	*******************************************************************/
	
	CLEANUP:
	return;
}

/**************************************************
* Function: 	fm_mso_srch_for_opt()
* Owner:        Rohini Mogili
* Decription:   Search the operations
*		
* 
* 
***************************************************/
void
fm_mso_srch_for_opt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	
	int32			srch_flag = 0;
	int			*flags = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_for_opt input flist", in_flistp);
	flags = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_FLAGS, 1, ebufp);

	srch_flag = *flags - 50;
	/*******************************************************************
	* Root Login Validation - Start
	*******************************************************************/
	
	PCM_OP(ctxp, MSO_OP_OPERATIONS_SEARCH, srch_flag, in_flistp, ret_flistp, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_OPERATIONS_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
	}	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_for_opt output flist", *ret_flistp);
	return;
}

/**************************************************
* Function:     fm_mso_srch_prov_order()
* Owner:        Satya Prakash
* Decription:   Search the provisioning Order
*
*
*
***************************************************/
void
fm_mso_srch_prov_order(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp)
{

        int32                   srch_flag = 0;
        int                     *flags = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_prov_order input flist", in_flistp);
        flags = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_FLAGS, 0, ebufp);

        srch_flag = *flags - 100;
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);

        PCM_OP(ctxp, MSO_OP_PROV_GET_FAILED_ORDERS, srch_flag, in_flistp, ret_flistp, ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_GET_FAILED_ORDERS", ebufp);
                PIN_ERRBUF_RESET(ebufp);
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_prov_order output flist", *ret_flistp);
        return;
}

/**************************************************
* Function:     fm_mso_srch_prov_order()
* Owner:        Sisir
* Decription:   Search device with varities of input
* flag :
* 120 - search vc details by card no
* 121 - search stb details by nds no
* 122 - search stb details by serial no
* 123 - search stb details by state & lco a/c no
* 124 - search vc details by state & lco a/c no
* 140 - search by device by PO NO
***************************************************/
void
fm_mso_search_device(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*out_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*nameimfo = NULL;
	int32			srch_flag = 768;
	poid_t			*pdp = NULL;
	poid_t			*device_pdp = NULL;
	int32			*flag_ptr = NULL;
	int32			flag = -1;
	int32			*state_id = NULL;
	int32			search_fail = 1;
	int64			db = -1;
        int32                   elem_id =0;
	int32			den_sp = 2;
	pin_cookie_t		cookie = NULL;
	char			*device_id = NULL;
	char			*source = NULL;
	char			*company = NULL;
	char            *den_nwp = "NDS1";
    char            *dsn_nwp = "NDS2";
    char            *hw_nwp = "NDS";
	pin_flist_t		*acct_flistp = NULL;
	pin_flist_t		*device_paired_flistp = NULL;
	char			template[256];
	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search input flist", i_flistp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_search", ebufp);
		goto CLEANUP;
	}

	flag_ptr = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp ); 
	pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ); 
	db = PIN_POID_GET_DB(pdp);
	flag = *flag_ptr;
	/* Search flist preperation*/
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, srch_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	memset(template,256,'\0');
	if(flag == SRCH_VC_BY_CARD_NO)
	{
		den_sp = fm_mso_is_den_sp(ctxp, i_flistp, ebufp);
		device_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEVICE_ID, 0, ebufp );
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		device_pdp = PIN_POID_CREATE(db, "/device/vc", -1, ebufp);
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, device_pdp, ebufp);
		
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_DEVICE_ID, device_id, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
		if (den_sp == 1)
		{
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_MANUFACTURER, den_nwp, ebufp);
		
        }
        else if (den_sp == 2)
        {
            PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_MANUFACTURER, dsn_nwp, ebufp);

        }
		else
		{
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_MANUFACTURER, hw_nwp, ebufp);
		}	
		sprintf(template,"%s","select x from /device where F1.type = '/device/vc' and F2 = V2 and F3 = V3 ");
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
	}
	else if (flag == SRCH_STB_BY_NDS_NO)
        {		
		sprintf(template,"%s","select x from /device where F1.type = '/device/stb' and F2 = V2 " );
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
		device_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEVICE_ID, 1, ebufp ); 
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		device_pdp = PIN_POID_CREATE(db, "/device/stb", -1, ebufp);
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, device_pdp, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_DEVICE_ID, device_id, ebufp);
	}
	else if (flag == SRCH_STB_BY_SERIAL_NO)
	{
		sprintf(template,"%s","select x from /device where F1.type = '/device/stb' and F2 = V2 " );
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
		device_id = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SERIAL_NO, 1, ebufp ); 
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		device_pdp = PIN_POID_CREATE(db, "/device/stb", -1, ebufp);
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, device_pdp, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, MSO_FLD_SERIAL_NO, device_id, ebufp);
	}
	else if (flag == SRCH_VC_BY_STATE_LCO)
	{
		sprintf(template,"%s","select x from /device where F1.type = '/device/vc' and F2 = V2 and F3 = V3 " );
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
		device_id = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SERIAL_NO, 1, ebufp );
		state_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_STATE_ID, 1, ebufp );
		source = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp ); 
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		device_pdp = PIN_POID_CREATE(db, "/device/vc", -1, ebufp);
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, device_pdp, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STATE_ID, state_id, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_SOURCE, source, ebufp);
	}
	else if (flag == SRCH_STB_BY_STATE_LCO)
	{
		sprintf(template,"%s","select x from /device where F1.type = '/device/stb' and F2 = V2 and F3 = V3 " );
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
		device_id = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SERIAL_NO, 1, ebufp );
		state_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_STATE_ID, 1, ebufp );
		source = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp ); 
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		device_pdp = PIN_POID_CREATE(db, "/device/stb", -1, ebufp);
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, device_pdp, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STATE_ID, state_id, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_SOURCE, source, ebufp);
	}
	else if (flag == SRCH_DEVICE_BY_PO_NO)
	{
		den_sp = 2;
		sprintf(template,"%s", "select X from /device 1, /mso_order 2 where 2.F1 = V1 and 2.F2 = 1.F3 " );
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
		device_id = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_PO_NO, 0, ebufp );

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, MSO_FLD_PO_NO, device_id, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, MSO_FLD_ORDER_OBJ, NULL, ebufp);
        }
	else
	{
		//Set error for return
		out_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, out_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
		PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_ERROR_CODE, "51221", ebufp);
		PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_ERROR_DESCR, "wrong flag in calling mso_op_search", ebufp);
		*ret_flistp = out_flistp;
		goto CLEANUP;

	}
	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_DEVICE_ID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_SOURCE, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_MANUFACTURER, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_MODEL, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STATE_ID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_SERIAL_NO, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_WARRANTY_END, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_CATEGORY, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_DEVICE_TYPE, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_VENDOR_WARRANTY_END, NULL, ebufp);
	//Inventory changes//
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_CHANNEL_NAME, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_VALID_TO, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_DELIVERY_STATUS, NULL, ebufp);
         /*Inventory Changes Nim*/
        PIN_FLIST_FLD_SET(result_flist, MSO_FLD_CR_ADJ_DATE, NULL, ebufp);
        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_COUNTRY, NULL, ebufp);
        PIN_FLIST_FLD_SET(result_flist, MSO_FLD_LICENSE_NO, NULL, ebufp);
        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_START_T, NULL, ebufp);
        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_RECEIPT_NO, NULL, ebufp);
        PIN_FLIST_FLD_SET(result_flist, MSO_FLD_ORDER_TYPE, NULL, ebufp);
        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ORDER_ID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_COUNT, NULL, ebufp);
	device_paired_flistp = PIN_FLIST_ELEM_ADD (result_flist, PIN_FLD_GROUP_DETAILS, 0, ebufp);
	PIN_FLIST_FLD_SET(device_paired_flistp, PIN_FLD_NAME, NULL, ebufp);
	arg_flist = PIN_FLIST_ELEM_ADD (result_flist, PIN_FLD_SERVICES, PIN_ELEMID_ANY, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_prov_order search input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
 
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		//Set error for return
		out_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, out_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
		PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_ERROR_CODE, "51221", ebufp);
		PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_ERROR_DESCR, "error in search execution in mso_op_search", ebufp);
		*ret_flistp = out_flistp;
		goto CLEANUP;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_devices search output flist", srch_out_flistp);

	// multiple devices handling starts
        while((result_flist = PIN_FLIST_ELEM_GET_NEXT(srch_out_flistp, PIN_FLD_RESULTS,
                        &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
        {
	    device_id = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_DEVICE_ID, 0, ebufp);
	    source = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_SOURCE, 0, ebufp);
	    acct_flistp = NULL;
	    fm_mso_srch_device_acct(ctxp,device_id,&den_sp,&acct_flistp,ebufp); 
	    if(!PIN_ERRBUF_IS_ERR(ebufp))
	    {
		if(acct_flistp)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "acct_flistp::", acct_flistp);
			PIN_FLIST_FLD_COPY(acct_flistp, PIN_FLD_ACCOUNT_NO, result_flist, 
						PIN_FLD_ACCOUNT_NO, ebufp);
		
		    //PIN_FLIST_DESTROY_EX(&acct_flistp, NULL);
		} else
		{
		    PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_NO, "", ebufp);
		}
	    }
	    PIN_FLIST_DESTROY_EX(&acct_flistp, NULL);

	    fm_mso_srch_device_sp_name(ctxp,source,&acct_flistp, ebufp); 
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
				PIN_FLIST_FLD_PUT(result_flist, PIN_FLD_COMPANY, company, ebufp);
			}
			else
			{
				PIN_FLIST_FLD_SET(result_flist, PIN_FLD_COMPANY, "", ebufp);
			}
		
		    PIN_FLIST_DESTROY_EX(&nameimfo, NULL);
		} else
		{
		    PIN_FLIST_FLD_SET(result_flist, PIN_FLD_COMPANY, "", ebufp);
		}
	    }
	    PIN_FLIST_DESTROY_EX(&acct_flistp, NULL);
        }


	*ret_flistp = srch_out_flistp;
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search output flist::", *ret_flistp);


CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	
}


/*Search for Device lifecyle*/
void
fm_mso_search_device_lifecycle(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp)
{
     pin_flist_t             *out_flistp = NULL;
     pin_flist_t             *srch_flistp = NULL;
     pin_flist_t             *srch_out_flistp = NULL;
	 pin_flist_t 			 *arg_flist=NULL;
	 pin_flist_t			 *child_acnt_info=NULL;
	 pin_flist_t			 *result_flist=NULL;
	 pin_flist_t			 *devices_flist=NULL;
	 pin_flist_t 			 *srch_acct_flistp=NULL;
	 pin_flist_t 			 *arg_acc_flist=NULL;
	 pin_flist_t			 *srch_acc_out_flistp=NULL;
	 pin_flist_t 			 *arg_created_flist=NULL;
	 pin_flist_t			 *arg_end_flist=NULL;
	 pin_flist_t			 *dev_flistp=NULL;
	 pin_flist_t			 *arg_acct_flistp=NULL;
     int32                   search_fail = 1;
	 int32					 *src_type=NULL;
     
	 
	int32					*flag_ptr = NULL;
	int32					flag = -1;
	 
	poid_t 					*pdp=NULL;
	poid_t					*account_pdp=NULL;
	poid_t					*srch_acct_pdp=NULL;
	poid_t 					*srch_pdp=NULL;
	
	
	int64					db = -1;
	int32 					srch_flag=256;
	char 					*account_no=NULL;
	char					template[256];
	char 					*vc_id=NULL;
	char					*stb_id=NULL; 
	char					*device_id=NULL; 
	char					*template_act=" select x from /account where F1 = V1 ";
	char					*action=NULL;
	time_t					*created_t=NULL;
	time_t					*end_t=NULL;
	 
     if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
				
    /**********************************************/
    /*  0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
     	0 PIN_FLD_FLAGS           INT [0] <new flag to be defined>
		0 PIN_FLD_SEARCH_TYPE     INT [0] <new macro to be difined>
     	0 PIN_FLD_ACCOUNT_NO      STR [0] "58006"
		0 MSO_FLD_VC_ID           STR [0] "000072316029"
		0 MSO_FLD_STB_ID          STR [0] "000072316029"
		0 PIN_FLD_ACTION          STR [0] "CREATED"
		0 PIN_FLD_START_T      TSTAMP [0] (9864332176)
		0 PIN_FLD_END_T        TSTAMP [0] (9864332176)
                0 PIN_FLD_DEVICE_ID           STR [0] "000072316029"

    */	
	/**********************************************/

     PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search input flist", i_flistp);
	 
	 
	src_type = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SEARCH_TYPE, 1, ebufp );
	flag_ptr = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp );
	//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_prov_order search input 0 flist", srch_flistp);
    account_no = (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp );
	//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_prov_order search input 0 flist", srch_flistp);	
	pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ); 
	db = PIN_POID_GET_DB(pdp);
	flag = *flag_ptr;
	/* Search flist preperation*/
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, srch_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	
		
	memset(template,256,'\0');
	/*****************************************/
	/* 	0 PIN_FLD_POID           POID [0] 0.0.0.1 /mso_lifecycle/device 1054633 0
		0 PIN_FLD_CREATED_T    TSTAMP [0] (1405416585) Tue Jul 15 14:59:45 2014
		0 PIN_FLD_MOD_T        TSTAMP [0] (1405416585) Tue Jul 15 14:59:45 2014
		0 PIN_FLD_READ_ACCESS     STR [0] "S"
		0 PIN_FLD_WRITE_ACCESS    STR [0] "S"
		0 PIN_FLD_ACCOUNT_OBJ    POID [0] 0.0.0.1 /account 293744 0
		0 PIN_FLD_ACTION          STR [0] ""
		0 PIN_FLD_DESCR           STR [0] ""
		0 PIN_FLD_PRE_OPCODE      INT [0] 2700
		0 PIN_FLD_PROGRAM_NAME    STR [0] "OAP"
		0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.0  0 0
		0 PIN_FLD_USERID         POID [0] 0.0.0.0  0 0
		0 PIN_FLD_DEVICES       ARRAY [0] allocated 20, used 9
		1     MSO_FLD_NEW_OWNER      POID [0] 0.0.0.0  0 0
		1     MSO_FLD_STB_ID          STR [0] "00:00:00:00:00:18"
		1     MSO_FLD_STB_OBJ        POID [0] 0.0.0.1 /device/stb -1 7
		1     MSO_FLD_VC_ID           STR [0] ""
		1     MSO_FLD_VC_OBJ         POID [0] 0.0.0.0  0 0
		1     PIN_FLD_NEW_BRAND      POID [0] 0.0.0.0  0 0
		1     PIN_FLD_NEW_STATE       INT [0] 1
		1     PIN_FLD_OLD_STATE       INT [0] 0
		1     PIN_FLD_OWNER          POID [0] 0.0.0.0  0 0
	*/
	/*****************************************/
	
    if (*src_type == SRCH_BY_DATE)
	{
		sprintf(template,"%s","select x from /mso_lifecycle/device where F1 >= V1 and F2 <= V2 and F3 = V3 order by created_t desc " );
		
		created_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_START_T, 1, ebufp );
		end_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 1, ebufp );
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
		arg_created_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		PIN_FLIST_FLD_SET(arg_created_flist, PIN_FLD_CREATED_T, created_t , ebufp);
	    arg_end_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_SET(arg_end_flist, PIN_FLD_CREATED_T,end_t, ebufp);
		
		//Search for account poid 
		
		account_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp );
		
		if(strcmp(account_no,"")==0 && strlen(account_no)==0)
		{
		       PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
					   //Set error for return
			   out_flistp = PIN_FLIST_CREATE(ebufp);
			   PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, out_flistp, PIN_FLD_POID, ebufp);
			   PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
			   PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_ERROR_CODE, "51221", ebufp);
			   PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_ERROR_DESCR, "account no must for date based mso_op_search", ebufp);
			   *ret_flistp = out_flistp;
			   goto CLEANUP;
		
		}
		
			
		srch_acct_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
		srch_acct_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(srch_acct_flistp, PIN_FLD_POID, srch_acct_pdp, ebufp);
		PIN_FLIST_FLD_SET(srch_acct_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
		PIN_FLIST_FLD_SET(srch_acct_flistp, PIN_FLD_TEMPLATE, template_act , ebufp);

		arg_acct_flistp = PIN_FLIST_ELEM_ADD(srch_acct_flistp, PIN_FLD_ARGS, 1, ebufp );
		PIN_FLIST_FLD_SET(arg_acct_flistp,PIN_FLD_ACCOUNT_NO,account_no,ebufp);
				
		result_flist = PIN_FLIST_ELEM_ADD(srch_acct_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp);
		
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_acct_flistp, &srch_acc_out_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&srch_acct_flistp, NULL);
		
		
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			 PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_search", ebufp);
			 goto CLEANUP;
		}
				 
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch search act_poid op flist", srch_acc_out_flistp);
		 
		result_flist = PIN_FLIST_ELEM_TAKE(srch_acc_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp ); 

		if(result_flist)	
		{
		    arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
			account_pdp = (poid_t *)PIN_FLIST_FLD_GET(result_flist, PIN_FLD_POID, 1, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, account_pdp, ebufp);
		}
		else
		{
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
					   //Set error for return
			   out_flistp = PIN_FLIST_CREATE(ebufp);
			   PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, out_flistp, PIN_FLD_POID, ebufp);
			   PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
			   PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_ERROR_CODE, "51221", ebufp);
			   PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_ERROR_DESCR, "Invalid account no in mso_op_search", ebufp);
			   *ret_flistp = out_flistp;
			   goto CLEANUP;
    	    		}
	    	}
		
		
		//Search for account poid ends
		
		
		
		
		//arg_account_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
		//PIN_FLIST_FLD_SET(arg_account_flistp, PIN_FLD_CREATED_T,end_t, ebufp);
		
		
	}
	else
	{
		//sprintf(template,"%s","select x from /mso_lifecycle where F1 = V1 " );
		vc_id = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_VC_ID, 1, ebufp );
		stb_id = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_STB_ID, 1, ebufp );
		device_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEVICE_ID, 1, ebufp );
		action = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTION,1 ,ebufp);
		//PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
		
		
		if( *src_type == SRCH_BY_ACTION)
		{
		    sprintf(template,"%s","select x from /mso_lifecycle/device where F1 = V1 order by created_t desc " );
			PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
		    arg_acc_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		    PIN_FLIST_FLD_SET(arg_acc_flist,PIN_FLD_ACTION,action,ebufp);
			
		}
		else if( *src_type == SRCH_BY_STB_ID)
		{
		
		    sprintf(template,"%s","select x from /mso_lifecycle/device where F1 = V1 order by created_t desc ");
			PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);			
			arg_acc_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
			dev_flistp= PIN_FLIST_ELEM_ADD(arg_acc_flist,PIN_FLD_DEVICES,0,ebufp);
		    PIN_FLIST_FLD_SET(dev_flistp, PIN_FLD_DEVICE_ID,stb_id,ebufp);
			
		}
		else if ( *src_type == SRCH_BY_VC_ID)
		{
		    sprintf(template,"%s","select x from /mso_lifecycle/device where F1 = V1 order by created_t desc ");
			PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);	
		    arg_acc_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
			dev_flistp= PIN_FLIST_ELEM_ADD(arg_acc_flist,PIN_FLD_DEVICES,0,ebufp);
		    PIN_FLIST_FLD_SET(dev_flistp,PIN_FLD_DEVICE_ID,vc_id, ebufp);
		}
                else if( *src_type == SRCH_BY_MAC_ID)
                {

                    sprintf(template,"%s","select x from /mso_lifecycle/device where F1 = V1 order by created_t desc ");
                        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
                        arg_acc_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
                        dev_flistp= PIN_FLIST_ELEM_ADD(arg_acc_flist,PIN_FLD_DEVICES,0,ebufp);
                    PIN_FLIST_FLD_SET(dev_flistp, PIN_FLD_DEVICE_ID,device_id,ebufp);

                }
                else if ( *src_type == SRCH_BY_IP)
                {
                    sprintf(template,"%s","select x from /mso_lifecycle/device where F1 = V1 order by created_t desc ");
                        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
                    arg_acc_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
                        dev_flistp= PIN_FLIST_ELEM_ADD(arg_acc_flist,PIN_FLD_DEVICES,0,ebufp);
                    PIN_FLIST_FLD_SET(dev_flistp,PIN_FLD_DEVICE_ID,device_id, ebufp);
                }

		else if ( *src_type == SRCH_BY_ACCOUNT_NO)
		{
		
		        sprintf(template,"%s","select x from /mso_lifecycle/device where F1 = V1 order by created_t desc " );
				PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
				account_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp );
			
				srch_acct_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
				srch_acct_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_PUT(srch_acct_flistp, PIN_FLD_POID, srch_acct_pdp, ebufp);
				PIN_FLIST_FLD_SET(srch_acct_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
				PIN_FLIST_FLD_SET(srch_acct_flistp, PIN_FLD_TEMPLATE, template_act , ebufp);

				arg_acct_flistp = PIN_FLIST_ELEM_ADD(srch_acct_flistp, PIN_FLD_ARGS, 1, ebufp );
				PIN_FLIST_FLD_SET(arg_acct_flistp,PIN_FLD_ACCOUNT_NO,account_no,ebufp);
				
				result_flist = PIN_FLIST_ELEM_ADD(srch_acct_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

				PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp);
		
				PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_acct_flistp, &srch_acc_out_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&srch_acct_flistp, NULL);
		
		
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					 PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_search", ebufp);
					 goto CLEANUP;
				}
				 
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch search act_poid op flist", srch_acc_out_flistp);
		 
				result_flist = PIN_FLIST_ELEM_TAKE(srch_acc_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp ); 

				if(result_flist)	
				{
				    arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
					account_pdp = (poid_t *)PIN_FLIST_FLD_GET(result_flist, PIN_FLD_POID, 1, ebufp );
					PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, account_pdp, ebufp);
				}
				else
				{
					if (PIN_ERRBUF_IS_ERR(ebufp))
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
					   //Set error for return
					   out_flistp = PIN_FLIST_CREATE(ebufp);
					   PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, out_flistp, PIN_FLD_POID, ebufp);
					   PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
					   PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_ERROR_CODE, "51221", ebufp);
					   PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_ERROR_DESCR, "Invalid account no in mso_op_search", ebufp);
					   *ret_flistp = out_flistp;
					   goto CLEANUP;
    		         }
	        	}
		
		    }
	}
	
	
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_prov_order search input tesmplate flist", srch_flistp);
		//vc_id = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_VC_ID, 1, ebufp );
		//stb_id = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_STB_ID, 1, ebufp ); 
	
		//arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		//PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_SOURCE, source, ebufp);
		
		
		
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch search input 3 flist", srch_flistp);
		
		result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_CREATED_T, NULL, ebufp);
		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACTION, NULL, ebufp);
		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_PROGRAM_NAME, NULL, ebufp);
		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp);
		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_DESCR, NULL, ebufp);
		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_USERID, NULL, ebufp);
		devices_flist = PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_DEVICES, 0, ebufp );
		PIN_FLIST_FLD_SET(devices_flist, PIN_FLD_NEW_STATE, NULL, ebufp);
		PIN_FLIST_FLD_SET(devices_flist, PIN_FLD_OLD_STATE, NULL, ebufp);
		PIN_FLIST_FLD_SET(devices_flist, MSO_FLD_VC_ID, NULL, ebufp);
		PIN_FLIST_FLD_SET(devices_flist, MSO_FLD_STB_ID, NULL, ebufp);
		PIN_FLIST_FLD_SET(devices_flist, PIN_FLD_ACCOUNT_NO, NULL, ebufp);
        	PIN_FLIST_FLD_SET(devices_flist, MSO_FLD_ACCOUNT_OWNER, NULL, ebufp);
		PIN_FLIST_FLD_SET(devices_flist, PIN_FLD_RECEIPT_NO, NULL, ebufp);
        	PIN_FLIST_FLD_SET(devices_flist, PIN_FLD_ORDER_ID, NULL, ebufp);
        	PIN_FLIST_FLD_SET(devices_flist, PIN_FLD_REFERENCE_ID, NULL, ebufp);
        	PIN_FLIST_FLD_SET(devices_flist, MSO_FLD_TRANSACTION_ID, NULL, ebufp);
        	PIN_FLIST_FLD_SET(devices_flist, PIN_FLD_STATE, NULL, ebufp);	
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch search input flist", srch_flistp);
		
	    PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch search output flist", srch_out_flistp);
	
	 

     if (PIN_ERRBUF_IS_ERR(ebufp))
     {
             PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_search", ebufp);
             goto CLEANUP;
     }
 
     if (PIN_ERRBUF_IS_ERR(ebufp))
     {
           PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
           //Set error for return
           out_flistp = PIN_FLIST_CREATE(ebufp);
           PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, out_flistp, PIN_FLD_POID, ebufp);
           PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
           PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_ERROR_CODE, "51221", ebufp);
           PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_ERROR_DESCR, "error in search execution in mso_op_search", ebufp);
           *ret_flistp = out_flistp;
           goto CLEANUP;
      }
      PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_prov_order search output flist", srch_out_flistp);
      *ret_flistp = srch_out_flistp;

CLEANUP:
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

}


/**************************************
 * fm_mso_search_schedule
 * Search the achedule actions
 ************************************/

void 
fm_mso_search_schedule(
	pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t             *out_flistp = NULL;
     	pin_flist_t             *srch_iflistp = NULL;
    	pin_flist_t             *srch_oflistp = NULL;
     	pin_flist_t             *args_flist=NULL;
     	pin_flist_t             *result_flist=NULL;
		pin_flist_t				*arg_acct_flistp=NULL;
		pin_flist_t				*srch_acc_out_flistp=NULL;
		pin_flist_t				*arg_flist=NULL;
		pin_flist_t				*srch_flistp=NULL;
		pin_flist_t				*srch_acct_flistp=NULL;
		pin_flist_t				*result_out_flist=NULL;
		pin_flist_t				*r_buf_to_flistp=NULL;
		pin_flist_t             *results_out_flistp=NULL;
		pin_flist_t             *r_flistp=NULL;
		pin_flist_t				*srch_acc_no_flistp=NULL;
		pin_flist_t				*arg_acc_no_flist=NULL;
		pin_flist_t				*result_acc_no_flist=NULL;
		pin_flist_t				*srch_acc_no_oflistp=NULL;
		pin_flist_t				*result_acct_flist=NULL;
		poid_t					*srch_acct_pdp=NULL;
		
	pin_buf_t	            *pin_bufp     = NULL;
	int32 		            elem_resluts_id=0;
	pin_cookie_t 		    cookie_results = NULL;
	//pin_cookie_t			cookie_acc_results = NULL;
    //int32			       elem_resluts_acc_id=0;
	poid_t					*account_pdp=NULL;
	poid_t				*user_pdp=NULL;
		
		
    char	             	template[256];
	char		*account_no=NULL;
	int32			srch_flag = 768;
	int32			srch_acct_flag = 256;
	int32			search_fail = 1;
	int32			*src_type=NULL;
	int32			*status=NULL;
	time_t			*start_t = NULL;
	time_t			*end_t	= NULL;
	int64			db = -1;
	poid_t			*pdp=NULL;
	poid_t			*srch_acc_no_pdp=NULL;
	poid_t			*dummy_pdp=NULL;
	char			*template_act=" select x from /account where F1 = V1 ";
	char			*template_act_search=" select x from /account where F1 = V1 OR F2 = V2 ";
	char			*acct_src_no=NULL;
	char			*action_name=NULL;
	
	 int32 		 tmp_business_type=-1;
	 int32 		 *business_type=NULL;
	 int32        account_type=0;

     	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_schedule input flist", i_flistp);

	pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ); 
	db = PIN_POID_GET_DB(pdp);
	src_type = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SEARCH_TYPE, 1, ebufp );
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_search_schedule", ebufp);
		goto CLEANUP;
	}
  
	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, srch_iflistp, PIN_FLD_POID, ebufp);
	
	if( *src_type == SRCH_BY_ACCOUNT_NO || *src_type == SRCH_BY_ACCT_NO_ACTION || *src_type == SRCH_BY_SCH_DATE)
    {
	     PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "@@fm_mso_search_schedule search if input flist", srch_iflistp);
	
	      			
			account_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp );
			
			srch_acct_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
			srch_acct_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(srch_acct_flistp, PIN_FLD_POID, srch_acct_pdp, ebufp);
			PIN_FLIST_FLD_SET(srch_acct_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
			PIN_FLIST_FLD_SET(srch_acct_flistp, PIN_FLD_TEMPLATE, template_act , ebufp);

			arg_acct_flistp = PIN_FLIST_ELEM_ADD(srch_acct_flistp, PIN_FLD_ARGS, 1, ebufp );
			PIN_FLIST_FLD_SET(arg_acct_flistp,PIN_FLD_ACCOUNT_NO,account_no,ebufp);
			if(*src_type == SRCH_BY_ACCOUNT_NO )	
			{
				 memset(template,256,'\0');
				sprintf(template,"%s","select x from /schedule where F1 = V1 order by when_t desc ");
				PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, template , ebufp);
			} 			
			if( *src_type == SRCH_BY_ACCT_NO_ACTION )
            {
				 memset(template,256,'\0');
				sprintf(template,"%s","select x from /schedule where F1 = V1 and F2 = V2 order by when_t desc ");
				PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, template , ebufp);
				action_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTION, 1, ebufp );
				
				args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 2, ebufp );
				PIN_FLIST_FLD_SET(args_flist , PIN_FLD_ACTION_NAME, action_name , ebufp);
            }
			if( *src_type == SRCH_BY_SCH_DATE )
			{
            	memset(template,256,'\0');
	
				sprintf(template,"%s","select x from /schedule where F1 > V1 and F2 < V2 and F3 = V3 order by when_t desc ");	
				PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, template , ebufp);
				
				
				start_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_START_T, 1, ebufp );
			end_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 1, ebufp );
			status = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_STATUS, 1, ebufp );
			action_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTION, 1, ebufp );
	
			args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp );
			PIN_FLIST_FLD_SET(args_flist , PIN_FLD_WHEN_T , start_t , ebufp);
			args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 2, ebufp );
			PIN_FLIST_FLD_SET(args_flist , PIN_FLD_WHEN_T , end_t , ebufp);
			
		    //args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 3, ebufp );
			//account_pdp = (poid_t *)PIN_FLIST_FLD_GET(result_out_flist, PIN_FLD_POID, 1, ebufp );
			//PIN_FLIST_FLD_SET(args_flist, PIN_FLD_ACCOUNT_OBJ, account_pdp, ebufp);
				
				
				
			}	
			result_flist = PIN_FLIST_ELEM_ADD(srch_acct_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

			PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_acct_flistp, &srch_acc_out_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&srch_acct_flistp, NULL);
			//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "@@fm_mso_search_schedule res if out flist", result_flist);
						
			result_out_flist = PIN_FLIST_ELEM_TAKE(srch_acc_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp ); 			
			
            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_schedule res if out flist", result_out_flist);			
   			
			if(result_out_flist)	
			{
			    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_schedule search if input flist", srch_iflistp);
				if( *src_type == SRCH_BY_SCH_DATE )
				{
					args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 3, ebufp );
					account_pdp = (poid_t *)PIN_FLIST_FLD_GET(result_out_flist, PIN_FLD_POID, 1, ebufp );
					PIN_FLIST_FLD_SET(args_flist, PIN_FLD_ACCOUNT_OBJ, account_pdp, ebufp);
				}
				else
				{
			    arg_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp );
				account_pdp = (poid_t *)PIN_FLIST_FLD_GET(result_out_flist, PIN_FLD_POID, 1, ebufp );
				PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, account_pdp, ebufp);
				}
			}
			else
			{
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
					   //Set error for return
					   out_flistp = PIN_FLIST_CREATE(ebufp);
					   PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, out_flistp, PIN_FLD_POID, ebufp);
					   PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
					   PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_ERROR_CODE, "51221", ebufp);
					   PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_ERROR_DESCR, "Invalid account no in mso_op_search", ebufp);
					   *ret_flistp = out_flistp;
					   goto CLEANUP;
    		    }
	        }
			
			
	}
	/*else if( *src_type == SRCH_BY_SCH_DATE)
	{
	   	    
		memset(template,256,'\0');
	
		sprintf(template,"%s","select x from /schedule where F1 >= V1 and F2 <= V2 and F3 = V3 ");
	
	    account_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp );
	
		srch_acct_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
		srch_acct_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(srch_acct_flistp, PIN_FLD_POID, srch_acct_pdp, ebufp);
		PIN_FLIST_FLD_SET(srch_acct_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
		PIN_FLIST_FLD_SET(srch_acct_flistp, PIN_FLD_TEMPLATE, template_act , ebufp);

		arg_acct_flistp = PIN_FLIST_ELEM_ADD(srch_acct_flistp, PIN_FLD_ARGS, 1, ebufp );
		PIN_FLIST_FLD_SET(arg_acct_flistp,PIN_FLD_ACCOUNT_NO,account_no,ebufp);
				
		result_flist = PIN_FLIST_ELEM_ADD(srch_acct_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "@@fm_mso_search_schedule res in out flist", srch_acct_flistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_acct_flistp, &srch_acc_out_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&srch_acct_flistp, NULL);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "@@fm_mso_search_schedule res if out flist", srch_acct_flistp);
						
		result_out_flist = PIN_FLIST_ELEM_TAKE(srch_acc_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp ); 			
			
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "@@fm_mso_search_schedule res if out flist", result_out_flist);			
   			
		if(result_out_flist)	
		{
		    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "@@fm_mso_search_schedule search if input flist", srch_iflistp);
			
			PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, template , ebufp);
	
			start_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_START_T, 1, ebufp );
			end_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 1, ebufp );
			status = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_STATUS, 1, ebufp );
			action_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTION, 1, ebufp );
	
			args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp );
			PIN_FLIST_FLD_SET(args_flist , PIN_FLD_WHEN_T , start_t , ebufp);
			args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 2, ebufp );
			PIN_FLIST_FLD_SET(args_flist , PIN_FLD_WHEN_T , end_t , ebufp);
			
		    args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 3, ebufp );
			account_pdp = (poid_t *)PIN_FLIST_FLD_GET(result_out_flist, PIN_FLD_POID, 1, ebufp );
			PIN_FLIST_FLD_SET(args_flist, PIN_FLD_ACCOUNT_OBJ, account_pdp, ebufp);
		}
	
	
	
	//args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 3, ebufp );
	//PIN_FLIST_FLD_SET(args_flist , PIN_FLD_STATUS, status , ebufp);
	//args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 4, ebufp );
	//PIN_FLIST_FLD_SET(args_flist , PIN_FLD_ACTION_NAME, action_name , ebufp);
	}*/
	else
	{
	
	    memset(template,256,'\0');
	
		sprintf(template,"%s","select x from /schedule where F1 >= V1 and F2 <= V2 and F3 = V3 and F4 = V4 order by when_t desc ");
		//sprintf(template,"%s","select x from /schedule where F1 >= V1 and F2 <= V2 and F3 = V3 ");
	    PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, template , ebufp);
	
		start_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_START_T, 1, ebufp );
		end_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 1, ebufp );
		status = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_STATUS, 1, ebufp );
		action_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTION, 1, ebufp );
	
		args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp );
		PIN_FLIST_FLD_SET(args_flist , PIN_FLD_WHEN_T , start_t , ebufp);
		args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_SET(args_flist , PIN_FLD_WHEN_T , end_t , ebufp);
			
		args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 4, ebufp );
		PIN_FLIST_FLD_SET(args_flist, PIN_FLD_STATUS, status, ebufp);
		
	    args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 3, ebufp );
		PIN_FLIST_FLD_SET(args_flist, PIN_FLD_ACTION_NAME, action_name, ebufp);
	
	}
	
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_schedule search results input flist", srch_iflistp);
	
	result_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS, 0, ebufp );
	
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist,PIN_FLD_ACCOUNT_OBJ , NULL , ebufp);
	PIN_FLIST_FLD_SET(result_flist,PIN_FLD_ACTION_NAME , NULL , ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STATUS , NULL , ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_WHEN_T , NULL , ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_CREATED_T , NULL , ebufp);
	PIN_FLIST_FLD_SET(result_flist,PIN_FLD_INPUT_FLIST ,NULL ,ebufp);
	
	// While data in results
	// convert the PIN_FLD_INPUT_FLIST into flist
	// read the user id
	// convert the poid to account no and return the flist.
	

	
	

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_schedule search else input flist", srch_iflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_schedule search else out flist", srch_oflistp);
	PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
	
	while ((results_out_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_oflistp,PIN_FLD_RESULTS,&elem_resluts_id, 1, &cookie_results, ebufp))!= NULL)
	{
						
		pin_buf_t	*pin_bufp	= NULL;
							
	 	pin_bufp = (pin_buf_t *) PIN_FLIST_FLD_GET(results_out_flistp, PIN_FLD_INPUT_FLIST, 0, ebufp );
		if (pin_bufp) 
		{ 
        //PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, pin_bufp->data);

        /* Convert the buffer to a flist */
        PIN_STR_TO_FLIST((char *)pin_bufp->data, 1, &r_buf_to_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " buf_to_r_flistp is " , r_buf_to_flistp);
		
		account_pdp=(poid_t *)PIN_FLIST_FLD_GET(results_out_flistp,PIN_FLD_ACCOUNT_OBJ, 1, ebufp );
		
		user_pdp=(poid_t *)PIN_FLIST_FLD_GET(r_buf_to_flistp, PIN_FLD_USERID, 1, ebufp );
		
		
		/*****************Account poid to account no search**************/
		
		srch_acc_no_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

        srch_acc_no_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_acc_no_flistp, PIN_FLD_POID, srch_acc_no_pdp, ebufp);
        PIN_FLIST_FLD_SET(srch_acc_no_flistp, PIN_FLD_FLAGS, &srch_acct_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_acc_no_flistp, PIN_FLD_TEMPLATE, template_act_search , ebufp);

        arg_acc_no_flist = PIN_FLIST_ELEM_ADD(srch_acc_no_flistp, PIN_FLD_ARGS, 1, ebufp );
		
		if(user_pdp)
		{ 		  
		     PIN_FLIST_FLD_SET(arg_acc_no_flist, PIN_FLD_POID, user_pdp, ebufp);
		}
		else
		{
			dummy_pdp = PIN_POID_CREATE(db, "/search",1, ebufp);
			PIN_FLIST_FLD_PUT(arg_acc_no_flist, PIN_FLD_POID, dummy_pdp, ebufp);
			//PIN_FLIST_FLD_SET(arg_acc_no_flist, PIN_FLD_POID, user_pdp, ebufp);
		}
		
		arg_acc_no_flist = PIN_FLIST_ELEM_ADD(srch_acc_no_flistp, PIN_FLD_ARGS, 2, ebufp );
        PIN_FLIST_FLD_SET(arg_acc_no_flist, PIN_FLD_POID, account_pdp, ebufp);
		
        result_acc_no_flist = PIN_FLIST_ELEM_ADD(srch_acc_no_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

        PIN_FLIST_FLD_SET(result_acc_no_flist, PIN_FLD_ACCOUNT_NO, NULL, ebufp);
		PIN_FLIST_FLD_SET(result_acc_no_flist, PIN_FLD_BUSINESS_TYPE, NULL, ebufp);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bulkacnt_from_acntno search input list", srch_acc_no_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_acc_no_flistp, &srch_acc_no_oflistp, ebufp);
        PIN_FLIST_DESTROY_EX(&srch_acc_no_flistp, NULL);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
		       PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ", ebufp);
               return;
        }
		
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_acnt_from_acntno search billinfo output list", srch_acc_no_oflistp);

		int32                   elem_resluts_acc_id=0;
		pin_cookie_t			cookie_acc_results = NULL;
		
		while ((result_acct_flist = PIN_FLIST_ELEM_GET_NEXT(srch_acc_no_oflistp,PIN_FLD_RESULTS,&elem_resluts_acc_id, 1, &cookie_acc_results, ebufp))!= NULL)
		{
		
		    			
			business_type =PIN_FLIST_FLD_GET(result_acct_flist, PIN_FLD_BUSINESS_TYPE, 1, ebufp);
		    
			tmp_business_type = *business_type;
			account_type = tmp_business_type/1000000;
			
			if ( account_type == ACCT_TYPE_SUBSCRIBER )
			{
			    acct_src_no=PIN_FLIST_FLD_GET(result_acct_flist, PIN_FLD_ACCOUNT_NO, 1, ebufp );
				PIN_FLIST_FLD_SET(results_out_flistp,PIN_FLD_ACCOUNT_NO ,acct_src_no,ebufp);
				
			}
			else if ( account_type == ACCT_TYPE_CSR )
			{
			   acct_src_no=PIN_FLIST_FLD_GET(result_acct_flist, PIN_FLD_ACCOUNT_NO, 1, ebufp );
			   PIN_FLIST_FLD_SET(results_out_flistp,PIN_FLD_USER_NAME ,acct_src_no,ebufp);
			   
			}
						
		}
		
				
		//PIN_FLIST_DESTROY_EX(&result_flist, NULL);
		/*****************Account poid to account no search**************/
		PIN_FLIST_FLD_DROP(results_out_flistp,PIN_FLD_INPUT_FLIST,ebufp);
		PIN_FLIST_FLD_DROP(results_out_flistp,PIN_FLD_ACCOUNT_OBJ,ebufp);
		
	    }
	}
	  
		
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		//Set error for return
		out_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, out_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
		PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_ERROR_CODE, "51221", ebufp);
		PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_ERROR_DESCR, "error in search execution in mso_op_search", ebufp);
		*ret_flistp = out_flistp;
		goto CLEANUP;
	}
	
	*ret_flistp = srch_oflistp;
	
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_acc_out_flistp,NULL);
	//PIN_FLIST_DESTROY_EX(&result_flist, NULL);
	//PIN_FLIST_DESTROY_EX(&srch_acc_out_flistp,NULL);
}


/*****************************************************************
* Function:     fm_mso_search_preactivated_service()
* Owner:        Sachidanand Joshi
* Decription:   Search preactivated VC/STB
* flag :
* 125 - SRCH_PREACTV_STB - search preactivated service using STB ID
* 126 - SRCH_PREACTV_VC  - search preactivated service using VC ID
*******************************************************************/
void
fm_mso_search_preactivated_service(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*out_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*tmp_flp = NULL;
	pin_flist_t		*robj_iflp = NULL;
	pin_flist_t		*robj_oflp = NULL;
	pin_flist_t		*device_flp = NULL;
	poid_t			*pdp = NULL;
	poid_t			*device_pdp = NULL;
	int32			srch_flag = 768;
	int32			args_cnt = 1;
	int32			*flag_ptr = NULL;
	int32			flag = -1;
	int32			*state_id = NULL;
	int32			search_fail = 1;
	int64			db = -1;
	char			*device_id = NULL;
	char			*source = NULL;
	char			template[] = " select X from /mso_catv_preactivated_svc 1, /device 2 where 1.F1 = 2.F2 and 2.F3 = V3 ";
	char            template1 [] = "select X from /plan 1 , /product 2, /deal 3, /mso_cfg_catv_pt 4 where 4.F1 = 2.F2 and 2.F3 = 3.F4 and 3.F5 = 1.F6 and 4.F7 = V7 ";
	char			func_name[] = "fm_mso_search_preactivated_service";
	char			msg[1024];
	char			*ca_idp = NULL;
	char			*nw_modep = NULL;
	char			*stb_id = NULL;
	char			*vc_id = NULL;
	char			*manufacturer = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		sprintf(msg,"%s: Error before processing");	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, msg, ebufp);
		return;
	}

	sprintf(msg,"%s: Input flist", func_name);	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg, i_flistp);
	
/*
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_search", ebufp);
		goto CLEANUP;
	}
*/

	flag_ptr = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp ); 
	pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ); 
	db = PIN_POID_GET_DB(pdp);
	flag = *flag_ptr;
	/* Search flist preperation*/
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, srch_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
	if(flag == SRCH_PREACTV_STB)
	{
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		//device_pdp = PIN_POID_CREATE(db, "/device/stb", -1, ebufp);
		PIN_FLIST_FLD_SET(arg_flist, MSO_FLD_STB_OBJ, device_pdp, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, device_pdp, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
		stb_id = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_STB_ID, 0, ebufp ); 
		PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_STB_ID, arg_flist, PIN_FLD_DEVICE_ID, ebufp);
	}
	else if (flag == SRCH_PREACTV_VC)
	{
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		//device_pdp = PIN_POID_CREATE(db, "/device/vc", -1, ebufp);
		PIN_FLIST_FLD_SET(arg_flist, MSO_FLD_VC_OBJ, device_pdp, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, device_pdp, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
		vc_id = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_VC_ID, 0, ebufp ); 
		PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_VC_ID, arg_flist, PIN_FLD_DEVICE_ID, ebufp);
	}
	else
	{
		//Set error for return
		out_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, out_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
		PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_ERROR_CODE, "51221", ebufp);
		PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_ERROR_DESCR, "wrong flag in calling mso_op_search", ebufp);
		*ret_flistp = out_flistp;
		goto CLEANUP;

	}
	PIN_FLIST_ELEM_SET(srch_flistp,NULL, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	
	sprintf(msg,"%s: PCM_OP_SEARCH input flist", func_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg, srch_flistp);
	/* Search input flist 
	 * 0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
	 * 0 PIN_FLD_FLAGS           INT [0] 256
	 * 0 PIN_FLD_TEMPLATE        STR [0] " select X from /mso_catv_preactivated_svc 1, /device 2 where 1.F1 = 2.F2 and 2.F3 = V3 "
	 * 0 PIN_FLD_ARGS          ARRAY [1] allocated 20, used 1
	 * 1      MSO_FLD_VC_OBJ    POID [0] 0.0.0.1 /device/vc -1
	 * 0 PIN_FLD_ARGS          ARRAY [2] allocated 20, used 1
	 * 1      PIN_FLD_POID      POID [0] 0.0.0.1 /device/vc -1 
	 * 0 PIN_FLD_ARGS          ARRAY [3] allocated 20, used 1
	 * 1      PIN_FLD_DEVICE_ID  STR [0] "000033167972"
	 * 0 PIN_FLD_RESULTS       ARRAY [*] allocated 20, used 1
	 * 1   MSO_FLD_CAS_SUBSCRIBER_ID    STR [0] NULL str ptr
	 * 1   MSO_FLD_NETWORK_NODE    STR [0] NULL str ptr
	 */

	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
 
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		//Set error for return
		out_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, out_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
		PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_ERROR_CODE, "51221", ebufp);
		PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_ERROR_DESCR, "error in search execution in mso_op_search", ebufp);
		*ret_flistp = out_flistp;
		goto CLEANUP;
	}
	sprintf(msg,"%s: PCM_OP_SEARCH return flist", func_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg, srch_out_flistp);
	if (PIN_FLIST_ELEM_COUNT(srch_out_flistp,PIN_FLD_RESULTS,ebufp)<1)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Invalid device id", ebufp);
		//Set error for return
		out_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, out_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
		PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_ERROR_CODE, "51221", ebufp);
		PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_ERROR_DESCR, "Invalid device id", ebufp);
		*ret_flistp = out_flistp;
		goto CLEANUP;
	}
	result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY,1, ebufp);
	ca_idp = PIN_FLIST_FLD_GET(result_flist, MSO_FLD_CA_ID, 0, ebufp);
	nw_modep = PIN_FLIST_FLD_GET(result_flist, MSO_FLD_NETWORK_NODE, 0, ebufp);
	manufacturer = PIN_FLIST_FLD_GET(result_flist, MSO_FLD_NETWORK_NODE, 0, ebufp);
	*ret_flistp = result_flist;

	/* Now read device object to get device id */
	robj_iflp = PIN_FLIST_CREATE(ebufp);
	if(vc_id)
		PIN_FLIST_FLD_COPY(result_flist, MSO_FLD_STB_OBJ, robj_iflp, PIN_FLD_POID, ebufp);
	if(stb_id && !strcmp(manufacturer,"NDS"))
		PIN_FLIST_FLD_COPY(result_flist, MSO_FLD_VC_OBJ, robj_iflp, PIN_FLD_POID, ebufp);
		
	if(!strcmp(manufacturer,"NDS")){
		PIN_FLIST_FLD_SET(robj_iflp, PIN_FLD_DEVICE_ID, NULL, ebufp);

        	sprintf(msg, "%s: PCM_OP_READ_FLDS input flist", func_name);
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, robj_iflp);
	
        	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, robj_iflp, &robj_oflp, ebufp);
        	if (PIN_ERRBUF_IS_ERR(ebufp))
        	{
                	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
                	//Set error for return
                	out_flistp = PIN_FLIST_CREATE(ebufp);
                	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, out_flistp, PIN_FLD_POID, ebufp);
                	PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
                	PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_ERROR_CODE, "51221", ebufp);
                	PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_ERROR_DESCR, "Device invalid", ebufp);
                	*ret_flistp = out_flistp;
                	goto CLEANUP;
        	}

        	sprintf(msg,"%s: PCM_OP_READ_FLDS return flist", func_name);
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg, robj_oflp);
	}
	if(stb_id){
		device_flp = PIN_FLIST_ELEM_ADD(*ret_flistp, PIN_FLD_DEVICES, 1, ebufp);	
		PIN_FLIST_FLD_COPY(result_flist, MSO_FLD_STB_OBJ, device_flp, MSO_FLD_STB_OBJ, ebufp);
		PIN_FLIST_FLD_SET(device_flp,PIN_FLD_DEVICE_ID, stb_id, ebufp);
		if(!strcmp(manufacturer,"NDS")){
		   device_flp = PIN_FLIST_ELEM_ADD(*ret_flistp, PIN_FLD_DEVICES, 2, ebufp);	
		   PIN_FLIST_FLD_COPY(robj_oflp, PIN_FLD_POID, device_flp, MSO_FLD_VC_OBJ, ebufp);
		   PIN_FLIST_FLD_COPY(robj_oflp, PIN_FLD_DEVICE_ID, device_flp, PIN_FLD_DEVICE_ID, ebufp);
		}
	}
	if(vc_id){
		device_flp = PIN_FLIST_ELEM_ADD(*ret_flistp, PIN_FLD_DEVICES, 1, ebufp);	
		PIN_FLIST_FLD_COPY(robj_oflp, PIN_FLD_POID, device_flp, MSO_FLD_STB_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(robj_oflp, PIN_FLD_DEVICE_ID, device_flp, PIN_FLD_DEVICE_ID, ebufp);
		device_flp = PIN_FLIST_ELEM_ADD(*ret_flistp, PIN_FLD_DEVICES, 2, ebufp);	
		PIN_FLIST_FLD_COPY(result_flist, MSO_FLD_VC_OBJ, device_flp, MSO_FLD_VC_OBJ, ebufp);
		PIN_FLIST_FLD_SET(device_flp,PIN_FLD_DEVICE_ID, vc_id, ebufp);
	}
		
	

	/* Now search plan name using ca_id 
	 * 0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
	 * 0 PIN_FLD_FLAGS           INT [0] 768
	 * 0 PIN_FLD_TEMPLATE        STR [0] "select X from /plan 1 , /product 2, /deal 3, /mso_cfg_catv_pt 4 where 4.F1 = 2.F2 and 2.F3 = 3.F4 and 3.F5 = 1.F6 and 4.F7 = V7 "
	 * 0 PIN_FLD_ARGS          ARRAY [1] allocated 20, used 1
	 * 1     PIN_FLD_PROVISIONING_TAG    STR [0] ""
	 * 0 PIN_FLD_ARGS          ARRAY [2] allocated 20, used 1
	 * 1     PIN_FLD_PROVISIONING_TAG    STR [0] ""
	 * 0 PIN_FLD_ARGS          ARRAY [3] allocated 20, used 1
	 * 1     PIN_FLD_POID           POID [0] NULL poid pointer
	 * 0 PIN_FLD_ARGS          ARRAY [4] allocated 20, used 1
	 * 1     PIN_FLD_PRODUCTS      ARRAY [*] allocated 20, used 1
	 * 2         PIN_FLD_PRODUCT_OBJ    POID [0] NULL poid pointer
	 * 0 PIN_FLD_ARGS          ARRAY [5] allocated 20, used 1
	 * 1     PIN_FLD_POID           POID [0] NULL poid pointer
	 * 0 PIN_FLD_ARGS          ARRAY [6] allocated 20, used 1
	 * 1     PIN_FLD_SERVICES      ARRAY [*] allocated 20, used 1
	 * 2         PIN_FLD_DEAL_OBJ       POID [0] NULL poid pointer
	 * 0 PIN_FLD_ARGS          ARRAY [7] allocated 20, used 1
	 * 1     MSO_FLD_CATV_PT_DATA  ARRAY [0] allocated 20, used 1
	 * 2         MSO_FLD_CA_ID           STR [0] "266"
	 * 0 PIN_FLD_RESULTS       ARRAY [*] allocated 20, used 1
	 * 1     PIN_FLD_NAME            STR [0] NULL str ptr
	*/
	sprintf(msg,"%s: Prepare search flist to search plan_name using CA_ID", func_name);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	PIN_FLIST_DESTROY_EX(&robj_iflp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, srch_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template1 , ebufp);

       // Add ARGS array
        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, args_cnt++, ebufp);
        PIN_FLIST_FLD_SET(arg_flist,PIN_FLD_PROVISIONING_TAG,"",ebufp);

        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, args_cnt++, ebufp);
        PIN_FLIST_FLD_SET(arg_flist,PIN_FLD_PROVISIONING_TAG,"",ebufp);

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

        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, args_cnt++, ebufp);
	if(!strcmp(nw_modep,"NDS"))
        	tmp_flp = PIN_FLIST_ELEM_ADD(arg_flist,MSO_FLD_CATV_PT_DATA,0,ebufp);
	else if (!strcmp(nw_modep,"CISCO"))
                tmp_flp = PIN_FLIST_ELEM_ADD(arg_flist,MSO_FLD_CATV_PT_DATA,1,ebufp);
	else if (!strcmp(nw_modep,"VERIM"))
        	tmp_flp = PIN_FLIST_ELEM_ADD(arg_flist,MSO_FLD_CATV_PT_DATA,2,ebufp);
        PIN_FLIST_FLD_SET(tmp_flp,MSO_FLD_CA_ID,ca_idp,ebufp);

        //Add result
        result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_NAME, NULL, ebufp);

        sprintf(msg, "%s: PCM_OP_SEARCH input flist", func_name);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, srch_flistp);

        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);

       if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
                //Set error for return
                out_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, out_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
                PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_ERROR_CODE, "51221", ebufp);
                PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_ERROR_DESCR, "error in search execution in mso_op_search", ebufp);
                *ret_flistp = out_flistp;
                goto CLEANUP;
        }
        sprintf(msg,"%s: PCM_OP_SEARCH return flist", func_name);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg, srch_out_flistp);
        if (PIN_FLIST_ELEM_COUNT(srch_out_flistp,PIN_FLD_RESULTS,ebufp)<1)
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Invalid device id, no plan associated with this device", ebufp);
                //Set error for return
                out_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, out_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
                PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_ERROR_CODE, "51221", ebufp);
                PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_ERROR_DESCR, "Invalid device id, no plan associated with this device", ebufp);
                *ret_flistp = out_flistp;
                goto CLEANUP;
        }
        result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY,1, ebufp);
	PIN_FLIST_FLD_DROP(result_flist, PIN_FLD_POID, ebufp);
        PIN_FLIST_CONCAT(*ret_flistp,result_flist,ebufp);


CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	
}



void
fm_mso_search_acct_bal(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp)
{

	char            *template = "select X from /mso_account_balance where F1 = V1 and F2 >= V2 and F3 < V3 order by rank asc ";
        poid_t          *search_pdp = NULL;
        poid_t          *pdp = NULL;
        int32           search_flags = SRCH_DISTINCT;
        int64           db_no = 0;
        pin_flist_t     *search_iflistp = NULL;
        pin_flist_t     *search_oflistp = NULL;
        pin_flist_t     *arg_flistp = NULL;
        pin_flist_t     *result_flistp = NULL;
	pin_flist_t	*flistp = NULL;
	pin_flist_t	*results_flistp = NULL;
	char		int_to_str[1024];
	int32		rec_id = 0;
	pin_cookie_t	cookie = NULL;
	poid_t		*class_pdp = NULL;
	poid_t		*bill_pdp = NULL;
	int64		class_pdp_id = 0;
	int64		bill_pdp_id = 0;
	pin_decimal_t	*amount = NULL;
	int32		status = 0;
	time_t		*end_t = NULL;
	int64		*int_end_t = NULL;
	char            poid_buf[PCM_MAX_POID_TYPE + 1];
	char            *p = NULL;
	int32           len = 0;
	char		*token = NULL;
	char		*event_type = NULL;
	char		*action_name = NULL;


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_acct_bal input flist: ",i_flistp);

	search_iflistp = PIN_FLIST_CREATE(ebufp);
        pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
        db_no = PIN_POID_GET_DB(pdp);
        search_pdp = (poid_t *)PIN_POID_CREATE(db_no, "/search", -1, ebufp);
        PIN_FLIST_FLD_PUT(search_iflistp,PIN_FLD_POID,(void *)search_pdp,ebufp);
        PIN_FLIST_FLD_SET(search_iflistp,PIN_FLD_FLAGS,&search_flags,ebufp);
        PIN_FLIST_FLD_SET(search_iflistp,PIN_FLD_TEMPLATE,template,ebufp);
        arg_flistp = PIN_FLIST_ELEM_ADD(search_iflistp,PIN_FLD_ARGS,1,ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, arg_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
        arg_flistp = PIN_FLIST_ELEM_ADD(search_iflistp,PIN_FLD_ARGS,2,ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_START_T, arg_flistp, PIN_FLD_POSTED_T, ebufp);
        arg_flistp = PIN_FLIST_ELEM_ADD(search_iflistp,PIN_FLD_ARGS,3,ebufp);
	/* Add 1 day to end_t */
	end_t = PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_END_T,0,ebufp);
	int_end_t = (int64 *)end_t;
	/*Add one second to end_t
	*int_end_t = *int_end_t + 86400;*/
	*int_end_t = *int_end_t + 1;
	end_t = (time_t *)int_end_t;
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POSTED_T, end_t,ebufp);
        result_flistp = PIN_FLIST_ELEM_ADD(search_iflistp,PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search input flist",search_iflistp);
        PCM_OP(ctxp,PCM_OP_SEARCH, 0, search_iflistp, &search_oflistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search output flist",search_oflistp);


	/* Set fields in return flist which are expected in output */
	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistp,PIN_FLD_POID,ebufp);
	
	if(!PIN_FLIST_ELEM_COUNT(search_oflistp, PIN_FLD_RESULTS, ebufp))
	{
		status = 1;
		PIN_FLIST_FLD_SET(*ret_flistp,PIN_FLD_STATUS,&status,ebufp);
		return;
	}
	PIN_FLIST_FLD_SET(*ret_flistp,PIN_FLD_STATUS,&status,ebufp);

	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(search_oflistp,
                PIN_FLD_RESULTS, &rec_id, 1, &cookie,
                ebufp)) != (pin_flist_t *)NULL)
	{
		results_flistp = PIN_FLIST_ELEM_ADD(*ret_flistp,PIN_FLD_RESULTS,rec_id,ebufp);
		
		/* Set Class poid_id0 as TRANS_ID */
		class_pdp = PIN_FLIST_FLD_GET(flistp,PIN_FLD_POID,0,ebufp);
//		class_pdp_id = (int64)PIN_POID_GET_ID(class_pdp);
//		memset(int_to_str, '\0', sizeof(int_to_str));
//		sprintf(int_to_str,"%d",class_pdp_id);

//		PIN_FLIST_FLD_SET(results_flistp,PIN_FLD_TRANS_ID,int_to_str,ebufp);
	
		len = sizeof( poid_buf );
                p = poid_buf;
                PIN_POID_TO_STR( class_pdp, &p, &len, ebufp);
	
		token = strtok(p, " ");
        	token = strtok(NULL, " ");
        	token = strtok(NULL, " ");

		PIN_FLIST_FLD_SET(results_flistp,PIN_FLD_TRANS_ID,token,ebufp);


		/* Set Item_obj_id0 as DOC_NO */
		bill_pdp = PIN_FLIST_FLD_GET(flistp,PIN_FLD_BILL_OBJ,0,ebufp);
//                bill_pdp_id = (int64)PIN_POID_GET_ID(bill_pdp);
//		memset(int_to_str, '\0', sizeof(int_to_str));
  //              sprintf(int_to_str,"%d",bill_pdp_id);

    //            PIN_FLIST_FLD_SET(results_flistp,PIN_FLD_DOC_NO,int_to_str,ebufp);	

		len = sizeof( poid_buf );
                p = poid_buf;
                PIN_POID_TO_STR( bill_pdp, &p, &len, ebufp);
	
		token = strtok(p, " ");
        	token = strtok(NULL, " ");
        	token = strtok(NULL, " ");

		PIN_FLIST_FLD_SET(results_flistp,PIN_FLD_DOC_NO,token,ebufp);

		/* Set Posted_t */
		PIN_FLIST_FLD_COPY(flistp,PIN_FLD_POSTED_T,results_flistp,PIN_FLD_POSTED_T,ebufp);

		/* Set PIN_FLD_EARNED_START_T */
		PIN_FLIST_FLD_COPY(flistp,PIN_FLD_EARNED_START_T,results_flistp,PIN_FLD_EARNED_START_T,ebufp);

		/* Set PIN_FLD_EARNED_END_T */
		PIN_FLIST_FLD_COPY(flistp,PIN_FLD_EARNED_END_T,results_flistp,PIN_FLD_EARNED_END_T,ebufp);

		/* Set PIN_FLD_DESCR */
		PIN_FLIST_FLD_COPY(flistp,PIN_FLD_DESCR,results_flistp,PIN_FLD_DESCR,ebufp);

		/* Set TOTAL_DEBITS & TOTAL_CREDITS */
		amount = PIN_FLIST_FLD_GET(flistp,PIN_FLD_AMOUNT,1,ebufp);
		if(!pbo_decimal_is_null(amount,ebufp))
		{
			if(pbo_decimal_sign(amount,ebufp) == -1)
			{
				PIN_FLIST_FLD_SET(results_flistp,PIN_FLD_TOTAL_CREDITS,amount,ebufp);
			}
			else
			{
				PIN_FLIST_FLD_SET(results_flistp,PIN_FLD_TOTAL_DEBITS,amount,ebufp);
			}
		}

		/* Set CURRENT_BAL */
		PIN_FLIST_FLD_COPY(flistp,PIN_FLD_CURRENT_BAL,results_flistp,PIN_FLD_CURRENT_BAL,ebufp);

		/* Set event_type as ACTION_NAME */
		//event_type = PIN_FLIST_FLD_GET(flistp, PIN_FLD_EVENT_TYPE, 1, ebufp);
//		if(event_type)
//		{
//			fm_mso_utils_get_event_action(event_type, &action_name, ebufp);
//			if(action_name)
//				PIN_FLIST_FLD_PUT(results_flistp, PIN_FLD_ACTION_NAME, action_name, ebufp);
//		}
		PIN_FLIST_FLD_COPY(flistp,PIN_FLD_ACTION_NAME,results_flistp,PIN_FLD_ACTION_NAME,ebufp);

		/* Set previous_total  as OPENING_BALANCE */
		PIN_FLIST_FLD_COPY(flistp,PIN_FLD_PREVIOUS_TOTAL,results_flistp,PIN_FLD_PREVIOUS_TOTAL,ebufp);

                /* Set DISCOUNT */
                PIN_FLIST_FLD_COPY(flistp,PIN_FLD_DISCOUNT,results_flistp,PIN_FLD_DISCOUNT,ebufp);


		/* Set PROGRAM_NAME */
		PIN_FLIST_FLD_COPY(flistp,PIN_FLD_PROGRAM_NAME,results_flistp,PIN_FLD_PROGRAM_NAME,ebufp);
		
	}

/* Cleanup memory */
	PIN_FLIST_DESTROY_EX(&search_iflistp, NULL);	
	PIN_FLIST_DESTROY_EX(&search_oflistp, NULL);	

	return;

}

void
fm_mso_search_contact_history(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	
	int32			srch_flag = 768;
 	int64			db = -1;

	poid_t 			*account_obj = NULL;	

	char			*template = "select X from /mso_contact_history where F1 = V1 " ;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_contact_history input flist", i_flistp);

	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp));
	account_obj = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	if ( !account_obj )
	{
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "account obj is mandatory for searching contact history", ebufp);
		*ret_flistp = r_flistp;
		return;
	}
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, srch_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_contact_history search input.", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_contact_history search output.", srch_out_flistp);

	CLEANUP:
	*ret_flistp = srch_out_flistp;
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
}

void
fm_mso_search_bb_usage_history(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*bal_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	
	int32			srch_flag = 768;
 	int64			db = -1;

	time_t			*start_t = NULL;
	time_t			*end_t = NULL;

	poid_t 			*service_obj = NULL;	

	char			*template = "select X from /event where F1 = V1 and F2 >= V2 and F3 <= V3 and poid_type = '/event/session/telco/broadband'" ;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_rmn input flist", i_flistp);

	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp));
	service_obj = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	start_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_START_T, 1, ebufp);
	end_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 1, ebufp);

	if ( !service_obj || !start_t || !end_t )
	{
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "service obj,start_t and end_t are mandatory for searching bb event history", ebufp);
		*ret_flistp = r_flistp;
		return;
	}
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, srch_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, arg_flist, PIN_FLD_SERVICE_OBJ, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_START_T, arg_flist, PIN_FLD_START_T, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_END_T, arg_flist, PIN_FLD_END_T, ebufp);


	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL , ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_OBJ, NULL , ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_SERVICE_OBJ, NULL , ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_START_T, NULL , ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_END_T, NULL , ebufp);
	bal_flistp = PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_BAL_IMPACTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_AMOUNT, NULL , ebufp);
	PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_AMOUNT_DEFERRED, NULL , ebufp);
	PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_QUANTITY, NULL , ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_bb_event_history input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_bb_event_history output flist", srch_out_flistp);

	
	CLEANUP:
	*ret_flistp = srch_out_flistp;

}


/**************************************************
* Function: 	fm_mso_srch_by_mobile()
* Decription:   Search the Mobile Number
*		
* 
* 
***************************************************/
void
fm_mso_srch_by_mobile(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{

	
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*nameinfo_billing = NULL;
	pin_flist_t		*nameinfo_flistp = NULL;
	pin_flist_t		*phones_flistp = NULL;	
	int32			srch_flag = 768;

	char			*template = "select X from /account where F1 = V1 " ;
 	int64			db = -1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_mobile input flist", in_flistp);

	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp));

	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, srch_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	/* *********************************************************************
	 * Mobile no. is stored in phones array where PIN_FLD_TYPE is 5 or 15 
	 * but here search is done on all phone numbers 
	 ***********************************************************************/
	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	nameinfo_flistp = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_NAMEINFO, PIN_ELEMID_ANY, ebufp );
	phones_flistp = PIN_FLIST_ELEM_ADD(nameinfo_flistp, PIN_FLD_PHONES, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PHONE, phones_flistp, PIN_FLD_PHONE, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_NO, NULL , ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_BUSINESS_TYPE, NULL , ebufp);

	nameinfo_billing = PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_NAMEINFO, 1, ebufp );
	PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_FIRST_NAME, NULL , ebufp);
	PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_MIDDLE_NAME, NULL , ebufp);
	PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_LAST_NAME, NULL , ebufp);
	PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_COMPANY, NULL , ebufp);
	PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_CITY, NULL , ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_mobile input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_mobile output flist", srch_out_flistp);

	
	CLEANUP:
	*ret_flistp = srch_out_flistp;
	return;
}

void
fm_mso_srch_by_email(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp)
{


        pin_flist_t             *srch_flistp = NULL;
        pin_flist_t             *arg_flist = NULL;
        pin_flist_t             *result_flist = NULL;
        pin_flist_t             *srch_out_flistp = NULL;
        pin_flist_t             *nameinfo_billing = NULL;
        pin_flist_t             *nameinfo_flistp = NULL;
        pin_flist_t             *phones_flistp = NULL;
        int32                   srch_flag = 768;

        char                    *template = "select X from /account where F1 = V1 " ;
        int64                   db = -1;
	
        int 			limit 	=20;
        int                     count = 0;
        int                     search_fail = 1;


	if (PIN_ERRBUF_IS_ERR(ebufp))
                return;

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_email input flist", in_flistp);

        db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp));

        srch_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, srch_flistp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

        /* *********************************************************************
         * Email add. is stored in phones array where PIN_FLD_TYPE is 16
         ***********************************************************************/
        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
//	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_EMAIL_ADDR, arg_flist, MSO_FLD_RMAIL, ebufp);
/*
        nameinfo_flistp = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_NAMEINFO, PIN_ELEMID_ANY, ebufp );
        phones_flistp = PIN_FLIST_ELEM_ADD(nameinfo_flistp, PIN_FLD_PHONES, PIN_ELEMID_ANY, ebufp );
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_EMAIL_ADDR, phones_flistp, PIN_FLD_PHONE, ebufp);

        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
*/
	nameinfo_flistp = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_NAMEINFO, 1, ebufp );
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_EMAIL_ADDR, nameinfo_flistp, PIN_FLD_EMAIL_ADDR, ebufp);

        result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, MSO_SRCH_LIMIT, ebufp );
        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_NO, NULL , ebufp);
        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_BUSINESS_TYPE, NULL , ebufp);
        PIN_FLIST_FLD_SET(result_flist, MSO_FLD_RMAIL, NULL , ebufp);

        nameinfo_billing = PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_NAMEINFO, 1, ebufp );
        PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_FIRST_NAME, NULL , ebufp);
        PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_MIDDLE_NAME, NULL , ebufp);
        PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_LAST_NAME, NULL , ebufp);
        PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_COMPANY, NULL , ebufp);
	PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_CITY, NULL , ebufp);
		
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_email input list", srch_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
                goto CLEANUP;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_email output flist", srch_out_flistp);

                /* Kaushik: 22-May-2018 : Added below block to return failure if email is not found */ 
                count = PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp);
                if(!count) {
                        PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
                        srch_out_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, srch_out_flistp, PIN_FLD_POID, ebufp);
                        PIN_FLIST_FLD_SET(srch_out_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
                        PIN_FLIST_FLD_SET(srch_out_flistp, PIN_FLD_ERROR_CODE, "51789", ebufp);
                        PIN_FLIST_FLD_SET(srch_out_flistp, PIN_FLD_ERROR_DESCR, "Email ID not found.", ebufp);
                }
                /* Added till here */



        CLEANUP:
        *ret_flistp = srch_out_flistp;
        return;
}

void 
fm_mso_search_bb_reason_code(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *srch_flistp = NULL;
        pin_flist_t             *arg_flist = NULL;
        pin_flist_t             *result_flist = NULL;
        pin_flist_t             *srch_out_flistp = NULL;
        int32                   srch_flag = 512;

        char                    *template = "select X from /strings where F1 = V1 " ;
        int64                   db = -1;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_reason_codes input flist", in_flistp);

        db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp));

        srch_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, srch_flistp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_DOMAIN, arg_flist, PIN_FLD_DOMAIN, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL , ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STRING, (char *)NULL , ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STRING_ID, (char *)NULL , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_reason_code input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_reason_code output flist", srch_out_flistp);

CLEANUP:
        *ret_flistp = srch_out_flistp;
        return;
}


/**************************************************
* Function:     fm_mso_srch_bb_devices()
* Owner:        Pawan
* Decription:   Search BB devices with different 
* 				input flag :
* 131 - search modem details by MAC ID/device id
* 132 - search router details by MAC ID/device id
* 132 - search cable router details by MAC ID/device id
* 137 - search NAT device details by MAC ID/device id
* 138 - search  device details by Serial No
***************************************************/
void
fm_mso_srch_bb_devices(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*out_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*acct_flistp = NULL;
	pin_flist_t		*nameimfo = NULL;	
	int32			srch_flag = 768;
	poid_t			*pdp = NULL;
	poid_t			*device_pdp = NULL;
	int32			*flag_ptr = NULL;
	int32			flag = -1;
	int32			*state_id = NULL;
	int32			search_fail = 1;
	int64			db = -1;
	char			*device_id = NULL;
	char			*source = NULL;
	char			*company = NULL;
	char			template[256];
        int32                   elem_id =0;
	int32                   den_sp = 2;
	pin_cookie_t			cookie = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_bb_devices input flist", i_flistp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_srch_bb_devices", ebufp);
		goto CLEANUP;
	}

	flag_ptr = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp ); 
	pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ); 
	db = PIN_POID_GET_DB(pdp);
	flag = *flag_ptr;
	/* Search flist preperation*/
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, srch_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	memset(template,256,'\0');
	if(flag == SRCH_MODEM_BY_MAC_ID)
	{
		
		sprintf(template,"%s","select x from /device where F1.type = V1 and F2 = V2 ");
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
		device_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEVICE_ID, 1, ebufp ); 
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		device_pdp = PIN_POID_CREATE(db, "/device/modem", -1, ebufp);
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, device_pdp, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_DEVICE_ID, device_id, ebufp);

	}
	else if (flag == SRCH_ROUTER_BY_MAC_ID)
        {		
		sprintf(template,"%s","select x from /device where F1.type = V1 and F2 = V2 " );
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
		device_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEVICE_ID, 1, ebufp ); 
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		device_pdp = PIN_POID_CREATE(db, "/device/router/wifi", -1, ebufp);
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, device_pdp, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_DEVICE_ID, device_id, ebufp);
	}
	else if (flag == SRCH_CABLE_ROUTER_BY_MAC_ID)
        {		
		sprintf(template,"%s","select x from /device where F1.type = V1 and F2 = V2 " );
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
		device_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEVICE_ID, 1, ebufp ); 
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		device_pdp = PIN_POID_CREATE(db, "/device/router/cable", -1, ebufp);
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, device_pdp, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_DEVICE_ID, device_id, ebufp);
	}
	else if (flag == SRCH_NAT_BY_DEVICE_ID)
        {		
		sprintf(template,"%s","select x from /device where F1.type = V1 and F2 = V2 " );
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
		device_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEVICE_ID, 1, ebufp ); 
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		device_pdp = PIN_POID_CREATE(db, "/device/nat", -1, ebufp);
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, device_pdp, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_DEVICE_ID, device_id, ebufp);
	}	
	else if (flag == SRCH_MODEM_BY_STATE_LCO)
	{
		sprintf(template,"%s","select x from /device where F1.type = V1 and F2 = V2 and F3 = V3 " );
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
		state_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_STATE_ID, 1, ebufp );
		source = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp ); 
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		device_pdp = PIN_POID_CREATE(db, "/device/modem", -1, ebufp);
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, device_pdp, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STATE_ID, state_id, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_SOURCE, source, ebufp);
	}
	else if (flag == SRCH_ROUTER_BY_STATE_LCO)
	{
		sprintf(template,"%s","select x from /device where F1.type = V1 and F2 = V2 and F3 = V3 " );
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
		state_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_STATE_ID, 1, ebufp );
		source = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp ); 
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		/*** 01072015 - Defect ID:1  - Pavan Bellala *******
		device_pdp = PIN_POID_CREATE(db, "/device/router", -1, ebufp);
		*** 01072015 - Defect ID:1  - Pavan Bellala *******/
		device_pdp = PIN_POID_CREATE(db, "/device/router/wifi", -1, ebufp);
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, device_pdp, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STATE_ID, state_id, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_SOURCE, source, ebufp);
	}
	else if (flag == SRCH_ROUTER_CAB_BY_STATE_LCO)
	{
		sprintf(template,"%s","select x from /device where F1.type = V1 and F2 = V2 and F3 = V3 " );
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
		state_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_STATE_ID, 1, ebufp );
		source = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp ); 
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		device_pdp = PIN_POID_CREATE(db, "/device/router/cable", -1, ebufp);
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, device_pdp, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STATE_ID, state_id, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_SOURCE, source, ebufp);
	}	
	else if (flag == SRCH_NAT_BY_STATE_LCO)
	{
		sprintf(template,"%s","select x from /device where F1.type = V1 and F2 = V2 and F3 = V3 " );
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
		state_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_STATE_ID, 1, ebufp );
		source = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp ); 
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		device_pdp = PIN_POID_CREATE(db, "/device/nat", -1, ebufp);
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, device_pdp, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STATE_ID, state_id, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_SOURCE, source, ebufp);
	}
	else if (flag == SRCH_DEVICE_BY_SL_NO)
	{
		sprintf(template,"%s","select x from /device where F1 = V1 " );
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
		device_id = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SERIAL_NO, 1, ebufp );
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, MSO_FLD_SERIAL_NO, device_id, ebufp);
	}
	else
	{
		//Set error for return
		out_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, out_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
		PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_ERROR_CODE, "51221", ebufp);
		PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_ERROR_DESCR, "wrong flag in calling mso_op_search", ebufp);
		*ret_flistp = out_flistp;
		goto CLEANUP;

	}
	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_DEVICE_ID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_SOURCE, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_MANUFACTURER, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_MODEL, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STATE_ID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_SERIAL_NO, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_WARRANTY_END, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_CATEGORY, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_DEVICE_TYPE, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_VENDOR_WARRANTY_END, NULL, ebufp);
        /*Inventory Changes Nim*/
        PIN_FLIST_FLD_SET(result_flist, MSO_FLD_CR_ADJ_DATE, NULL, ebufp);
        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_COUNTRY, NULL, ebufp);
        PIN_FLIST_FLD_SET(result_flist, MSO_FLD_LICENSE_NO, NULL, ebufp);
        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_START_T, NULL, ebufp);
        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_RECEIPT_NO, NULL, ebufp);
        PIN_FLIST_FLD_SET(result_flist, MSO_FLD_ORDER_TYPE, NULL, ebufp);
        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ORDER_ID, NULL, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_bb_devices search input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
 
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		//Set error for return
		out_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, out_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
		PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_ERROR_CODE, "51221", ebufp);
		PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_ERROR_DESCR, "error in search execution in mso_op_search", ebufp);
		*ret_flistp = out_flistp;
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_bb_devices search output flist", srch_out_flistp);

	// multiple devices handling starts
        while((result_flist = PIN_FLIST_ELEM_GET_NEXT(srch_out_flistp, PIN_FLD_RESULTS,
                        &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
        {
	    device_id = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_DEVICE_ID, 0, ebufp);
	    source = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_SOURCE, 0, ebufp);
	    acct_flistp = NULL;
	    fm_mso_srch_device_acct(ctxp,device_id,&den_sp,&acct_flistp, ebufp); 
	    if(!PIN_ERRBUF_IS_ERR(ebufp))
	    {
		if(acct_flistp)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "acct_flistp::", acct_flistp);
			PIN_FLIST_FLD_COPY(acct_flistp, PIN_FLD_ACCOUNT_NO, result_flist, 
						PIN_FLD_ACCOUNT_NO, ebufp);
		
		    //PIN_FLIST_DESTROY_EX(&acct_flistp, NULL);
		} else
		{
		    PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_NO, "", ebufp);
		}
	    }
	    PIN_FLIST_DESTROY_EX(&acct_flistp, NULL);

	    fm_mso_srch_device_sp_name(ctxp,source,&acct_flistp, ebufp); 
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
				PIN_FLIST_FLD_PUT(result_flist, PIN_FLD_COMPANY, company, ebufp);
			}
			else
			{
				PIN_FLIST_FLD_SET(result_flist, PIN_FLD_COMPANY, "", ebufp);
			}
		
		    PIN_FLIST_DESTROY_EX(&nameimfo, NULL);
		} else
		{
		    PIN_FLIST_FLD_SET(result_flist, PIN_FLD_COMPANY, "", ebufp);
		}
	    }
	    PIN_FLIST_DESTROY_EX(&acct_flistp, NULL);
        }


	*ret_flistp = srch_out_flistp;
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search output flist::", *ret_flistp);

CLEANUP:
	if(srch_flistp != NULL) {
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	}
	//PIN_FLIST_DESTROY_EX(&acct_flistp, NULL);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search output flist::", *ret_flistp);
	
}

/**************************************************
* Function:		fm_mso_search_ip_device()
* Owner:			Tim Zhao
* Decription:	Search IP device
*		
* 
* 0 PIN_FLD_POID               POID [0] 0.0.0.1 /search -1 0
* 0 PIN_FLD_FLAGS               INT [0] 200
* 0 MSO_FLD_CMTS_ID             STR [0] "1187"
* 0 PIN_FLD_PLAN_OBJ          POID [0] 0.0.0.1 /plan 12345 
* 0 PIN_FLD_TYPE                INT [0] 4
* 0 MSO_FLD_IP_START_RANGE      STR [0] "1.1.1.2"
* 
* 
***************************************************/
void
fm_mso_search_ip_device(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{

	
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*arg_flist1 = NULL;
	pin_flist_t		*arg_flist2 = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	
	char			*cmts_id  = NULL;
	int32			*type_ptr = NULL;
	int32			*flag_ptr = NULL;
	int32			flag = -1;
	int32			ip_step = 1;
	
	char		*ip_start_range_opted		= NULL;
	char		*ip_pool_name		= NULL;
	//char		*ip_start_range		= NULL;
	//char		*ip_end_range		= NULL;
	char		ip_start_range[32];
	char		ip_end_range[32];
	char		*ip_start_str		= NULL;
	char		*ip_end_str			= NULL;	
	unsigned int			ip_start_opted = 0;
	unsigned int			ip_start = 0;
	unsigned int			ip_end = 0;
	
	int32			srch_flag = 768;
	int32			search_fail = 1;

	char			template1[1024];
	char			template2[1024];
	//char			*template2=" select X from /device where poid_type like '/device/ip/%' and state_id=1 and REGEXP_LIKE(device_id,'\\d+\\.\\d+\\.\\d+\\.\\d+') and regexp_substr(device_id,'\\d+',1,1)*16777216+regexp_substr(device_id,'\\d+',1,2)*65536+regexp_substr(device_id,'\\d+',1,3)*256+regexp_substr(device_id,'\\d+',1,4) between V1 and V2 order by length(device_id),device_id ";
	//char			*template2=" select X from /device where poid_id0 in (select obj_id0 from mso_device_ip_data_t where ip_pool_name='%s') and state_id=1 and REGEXP_LIKE(device_id,'\\d+\\.\\d+\\.\\d+\\.\\d+') and regexp_substr(device_id,'\\d+',1,1)*16777216+regexp_substr(device_id,'\\d+',1,2)*65536+regexp_substr(device_id,'\\d+',1,3)*256+regexp_substr(device_id,'\\d+',1,4) >= V1 order by length(device_id),device_id ";
 	//int64			db = -1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_ip_device input flist", in_flistp);

	//db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp));
	
	flag_ptr = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_FLAGS, 1, ebufp ); 
	flag = *flag_ptr;
	
	cmts_id  = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_CMTS_ID, 1, ebufp);
	type_ptr = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_TYPE, 1, ebufp ); 
	ip_step = *type_ptr;
	
	ip_start_range_opted = PIN_FLIST_FLD_GET(in_flistp , MSO_FLD_IP_START_RANGE, 1 , ebufp);
	
	if (ip_start_range_opted && ip_start_range_opted != NULL) 
	{
		ip_start_opted += atoi(strtok(ip_start_range_opted, "."))*256*256*256;
		ip_start_opted += atoi(strtok(NULL, "."))*256*256;
		ip_start_opted += atoi(strtok(NULL, "."))*256;
		ip_start_opted += atoi(strtok(NULL, "."))*1;
	}
	
	sprintf(
	  template1,
	  " select X from /mso_ip_order where ip_pool_name = (select b.client_class_value from mso_cfg_cmts_cc_mapping_t a,mso_cc_mapping_t b where a.poid_id0=b.obj_id0 and a.cmts_id='%s' and b.client_class_id=100 and b.provisioning_tag = (select p.provisioning_tag from product_t p where p.poid_id0 in (select dp.product_obj_id0 from deal_products_t dp where dp.obj_id0 in (select ps.deal_obj_id0 from plan_services_t ps where ps.obj_id0=%lld)) and p.provisioning_tag is not NULL and rownum<=1)) ",
	  cmts_id,PIN_POID_GET_ID(PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp)) 
	);
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG ,template1);

	/*******************************************************************
	* Start
	*******************************************************************/
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, srch_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template1 , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STATE_ID, NULL , ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_IP_POOL_NAME, NULL , ebufp);
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_IP_START_RANGE, NULL , ebufp);
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_IP_END_RANGE, NULL , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_ip_device(/mso_ip_order) input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_ip_device(/mso_ip_order) output flist", srch_out_flistp);

	if(PIN_FLIST_ELEM_COUNT(srch_out_flistp , PIN_FLD_RESULTS , ebufp) > 0)
	{
		// IP Pool found
		pin_flist_t		*results_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		ip_pool_name = PIN_FLIST_FLD_GET(results_flistp , MSO_FLD_IP_POOL_NAME, 1 , ebufp);
		
/*
		if (ip_step > 1) 
		{
			if (ip_start % ip_step > 0)
			{
				ip_start = (ip_start / ip_step + 1) * ip_step;
			}
		}
*/		
		if ( (ip_start_opted>0) && (ip_start_opted % ip_step != 0) )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error start IP");
			
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error start IP", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			srch_out_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, srch_out_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(srch_out_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
			PIN_FLIST_FLD_SET(srch_out_flistp, PIN_FLD_ERROR_CODE, "51221", ebufp);
			PIN_FLIST_FLD_SET(srch_out_flistp, PIN_FLD_ERROR_DESCR, "Error start IP", ebufp);
			goto CLEANUP;
		}
		
		srch_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, srch_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
		arg_flist1 = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		arg_flist2 = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );

		if (ip_start_opted>0)
		{
			// search free IP devices by start IP opted
			sprintf(
			  template2,
			  " select X from /device where poid_id0 in (select obj_id0 from mso_device_ip_data_t where ip_pool_name='%s') and state_id=1 and REGEXP_LIKE(device_id,'\\d+\\.\\d+\\.\\d+\\.\\d+') and regexp_substr(device_id,'\\d+',1,1)*16777216+regexp_substr(device_id,'\\d+',1,2)*65536+regexp_substr(device_id,'\\d+',1,3)*256+regexp_substr(device_id,'\\d+',1,4) between V1 and V2 order by length(device_id),device_id ",
			  ip_pool_name
			);
			PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template2 , ebufp);

			ip_start = ip_start_opted;
			unsigned int ip_end_tmp = ip_start+ip_step-1;

			PIN_FLIST_FLD_SET(arg_flist1, PIN_FLD_STATE_ID, &ip_start , ebufp);
			PIN_FLIST_FLD_SET(arg_flist2, PIN_FLD_STATE_ID, &ip_end_tmp , ebufp);

			result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
			PIN_FLIST_FLD_SET(result_flist, PIN_FLD_DEVICE_ID, NULL , ebufp);
		
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_ip_device(/device) input list", srch_flistp);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
			
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
				goto CLEANUP;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_ip_device(/device) output flist", srch_out_flistp);
			
			if(PIN_FLIST_ELEM_COUNT(srch_out_flistp , PIN_FLD_RESULTS , ebufp) == ip_step) 
			{
				PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "IP Pool search completed");
				goto CLEANUP;
			}
		} 
		else if (ip_step>1)
		{
			//search any free IPs (IP number: 4,8,16,32,64)
			sprintf(
			  template2,
			  " select X from /device where poid_id0 in (select obj_id0 from mso_device_ip_data_t where ip_pool_name='%s') and state_id=1 and REGEXP_LIKE(device_id,'\\d+\\.\\d+\\.\\d+\\.\\d+') and regexp_substr(device_id,'\\d+',1,1)*16777216+regexp_substr(device_id,'\\d+',1,2)*65536+regexp_substr(device_id,'\\d+',1,3)*256+regexp_substr(device_id,'\\d+',1,4) >= V1 order by length(device_id),device_id ",
			  ip_pool_name
			);
			PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template2 , ebufp);

			result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, ip_step+1, ebufp );
			PIN_FLIST_FLD_SET(result_flist, PIN_FLD_DEVICE_ID, NULL , ebufp);

			ip_start = 0;
			for (;;) 
			{
				PIN_FLIST_FLD_SET(arg_flist1, PIN_FLD_STATE_ID, &ip_start , ebufp);
			//	PIN_FLIST_FLD_SET(arg_flist1, PIN_FLD_STATE_ID, &ip_step , ebufp);

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_ip_device(/device) input list", srch_flistp);
				PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
				
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
					goto CLEANUP;
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_ip_device(/device) output flist", srch_out_flistp);
				
				if(PIN_FLIST_ELEM_COUNT(srch_out_flistp , PIN_FLD_RESULTS , ebufp) >= ip_step) 
				{
					// check the first @ip_step results to see if it includes consecutive IPs.
					pin_flist_t		*results_flistp_first = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
					pin_flist_t		*results_flistp_last  = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, ip_step-1, 1, ebufp);

					ip_start_str = PIN_FLIST_FLD_GET(results_flistp_first , PIN_FLD_DEVICE_ID, 1 , ebufp);
					ip_end_str = PIN_FLIST_FLD_GET(results_flistp_last , PIN_FLD_DEVICE_ID, 1 , ebufp);
					strcpy(ip_start_range,ip_start_str);
					strcpy(ip_end_range,ip_end_str);

					ip_start += atoi(strtok(ip_start_range, "."))*256*256*256;
					ip_start += atoi(strtok(NULL, "."))*256*256;
					ip_start += atoi(strtok(NULL, "."))*256;
					ip_start += atoi(strtok(NULL, "."))*1;
					
					ip_end += atoi(strtok(ip_end_range, "."))*256*256*256;
					ip_end += atoi(strtok(NULL, "."))*256*256;
					ip_end += atoi(strtok(NULL, "."))*256;
					ip_end += atoi(strtok(NULL, "."))*1;
					
					if (ip_start+ip_step-1 == ip_end)
					{
						PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "IP Pool search completed");
						goto CLEANUP;
					}
					else
					{
						if (PIN_FLIST_ELEM_COUNT(srch_out_flistp , PIN_FLD_RESULTS , ebufp) == ip_step+1)
						{
							pin_flist_t		*results_flistp_next_start = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, ip_step, 1, ebufp);
							ip_start_str = PIN_FLIST_FLD_GET(results_flistp_next_start , PIN_FLD_DEVICE_ID, 1 , ebufp);
							strcpy(ip_start_range,ip_start_str);
							ip_start += atoi(strtok(ip_start_range, "."))*256*256*256;
							ip_start += atoi(strtok(NULL, "."))*256*256;
							ip_start += atoi(strtok(NULL, "."))*256;
							ip_start += atoi(strtok(NULL, "."))*1;

							if (ip_start % ip_step > 0)
							{
								ip_start = (ip_start / ip_step + 1) * ip_step;
							}
							
						}
					}

				}
				else{
					break;
				}
		
			}
		}
		else 
		{
			//search single free IP (IP number: 1)
			sprintf(
			  template2,
			  " select X from /device where poid_id0 in (select obj_id0 from mso_device_ip_data_t where ip_pool_name='%s') and state_id=1 and REGEXP_LIKE(device_id,'\\d+\\.\\d+\\.\\d+\\.\\d+') order by regexp_substr(device_id,'\\d+',1,1)*16777216+regexp_substr(device_id,'\\d+',1,2)*65536+regexp_substr(device_id,'\\d+',1,3)*256+regexp_substr(device_id,'\\d+',1,4) ",
			  ip_pool_name
			);
			PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template2 , ebufp);

			result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 100, ebufp );
			PIN_FLIST_FLD_SET(result_flist, PIN_FLD_DEVICE_ID, NULL , ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_ip_device(/device) input list", srch_flistp);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
			
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
				goto CLEANUP;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_ip_device(/device) output flist", srch_out_flistp);
			
			if(PIN_FLIST_ELEM_COUNT(srch_out_flistp , PIN_FLD_RESULTS , ebufp) > 0) 
			{
				PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "IP Pool search completed");
				goto CLEANUP;
			}
		}
				
		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No free IP Devices found");
		
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"No free IP Devices found", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		srch_out_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, srch_out_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(srch_out_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
		PIN_FLIST_FLD_SET(srch_out_flistp, PIN_FLD_ERROR_CODE, "51221", ebufp);
		PIN_FLIST_FLD_SET(srch_out_flistp, PIN_FLD_ERROR_DESCR, "No free IP Devices found", ebufp);
		goto CLEANUP;

	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "IP Pool not found");
		
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"IP Pool not found", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		srch_out_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, srch_out_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(srch_out_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
		PIN_FLIST_FLD_SET(srch_out_flistp, PIN_FLD_ERROR_CODE, "51221", ebufp);
		PIN_FLIST_FLD_SET(srch_out_flistp, PIN_FLD_ERROR_DESCR, "IP Pool not found", ebufp);
		goto CLEANUP;
		
	}
	
	CLEANUP:
	*ret_flistp = srch_out_flistp;
	return;
}

/***************************************************
* Function: fm_mso_get_prod_price_details
* Description: Compares the discount amount in 
*		input with the product price and returns
*		valid/invalid flag accordingly.
***************************************************/
void 
fm_mso_get_prod_price_details(
	pcm_context_t           *ctxp,
	pin_flist_t             *in_flistp,
	pin_flist_t             **ret_flistp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t             *read_in_flistp = NULL;
	pin_flist_t             *read_out_flistp = NULL;
	pin_flist_t             *srch_in_flistp = NULL;
	pin_flist_t             *srch_out_flistp = NULL;
	pin_flist_t             *arg_flistp = NULL;
	pin_flist_t             *tmp_flistp = NULL;
	pin_flist_t             *result_flist = NULL;
	pin_flist_t             *planlist_flistp = NULL;
	pin_flist_t             *plan_flistp = NULL;
	pin_flist_t             *prod_flistp = NULL;
	char                    *template = "select X from /mso_cfg_credit_limit 1, /plan 2  where  2.F1 = V1 and 2.F2 = 1.F3 and ( 1.F4 = V4 or 1.F5 = V5) and 1.F6 = V6 " ;
	char					*template_hw = "select X from /mso_cfg_credit_limit 1, /plan 2  where  2.F1 = V1 and 2.F2 = 1.F3 and ( 1.F4 = V4 or 1.F5 = V5) " ;
	char			*template_prod = "select X from /product 1, /deal 2, /plan 3 where 3.F1 = V1 and 2.F2 = 3.F3 and 2.F4 = 1.F5 ";
	pin_decimal_t			*disc_amt = NULL;
	pin_decimal_t			*prod_charge = NULL;
	poid_t					*srch_pdp = NULL;
	int64                   db = -1;
	int						srch_flag = 256;
	int						status_fail = 1;
	int						status_ok = 0;
	int						SUBS_FLAG = 0;
	int						valid = 0;
	int						invalid = 1;
	int						final_status = 0;
	int						zero = 0;
	int						*dis_type = NULL;
	//char					cycle_evt[] = "/event/billing/product/fee/cycle";
	char					dummy_name[2] ="";
	char					wc_star[2] ="*";
	//char					*evt_type = NULL;
	int32					elem_id =0;
	int32					p_elem_id =0;
	int32					pl_elem_id =0;
	pin_cookie_t			cookie = NULL;
	pin_cookie_t			p_cookie = NULL;
	pin_cookie_t			pl_cookie = NULL;	

	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_prod_price_details input flist", in_flistp);
	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp));

	tmp_flistp = PIN_FLIST_SUBSTR_TAKE(in_flistp, MSO_FLD_SUBSCRIPTION_PLANS, 1, ebufp);
	if(tmp_flistp) {
		PIN_FLIST_ELEM_PUT(in_flistp, tmp_flistp, PIN_FLD_PLAN_LISTS, 1, ebufp)
	}
	tmp_flistp = PIN_FLIST_SUBSTR_TAKE(in_flistp, MSO_FLD_HARDWARE_PLANS, 1, ebufp);
	if(tmp_flistp) {
		PIN_FLIST_ELEM_PUT(in_flistp, tmp_flistp, PIN_FLD_PLAN_LISTS, 2, ebufp)
	}	
	tmp_flistp = PIN_FLIST_SUBSTR_TAKE(in_flistp, MSO_FLD_INSTALLATION_PLANS, 1, ebufp);
	if(tmp_flistp) {
		PIN_FLIST_ELEM_PUT(in_flistp, tmp_flistp, PIN_FLD_PLAN_LISTS, 3, ebufp)
	}
	final_status = valid;
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Flist after adding plan list:", in_flistp);
	while((planlist_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_PLAN_LISTS,
	&pl_elem_id, 1, &pl_cookie, ebufp)) != (pin_flist_t *)NULL)
	{	
		p_cookie = NULL;
		while((plan_flistp = PIN_FLIST_ELEM_GET_NEXT(planlist_flistp, PIN_FLD_PLAN,
			&p_elem_id, 1, &p_cookie, ebufp)) != (pin_flist_t *)NULL)
		{	
			disc_amt = NULL;
			disc_amt = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_DISCOUNT, 1, ebufp);
			if (!disc_amt || pbo_decimal_is_null(disc_amt,ebufp))
			{
				continue;
			}
				
			/* Search /mso_cfg_credit_limit config for the plan in input. */
			srch_in_flistp = PIN_FLIST_CREATE(ebufp);
			srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
			PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
			PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
			
			arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
			PIN_FLIST_FLD_COPY(plan_flistp, PIN_FLD_PLAN_OBJ, arg_flistp, PIN_FLD_POID, ebufp);

			arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 2, ebufp );
			PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_NAME, dummy_name , ebufp);
			
			arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 3, ebufp );
			PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_PLAN_NAME, dummy_name , ebufp);

			arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 4, ebufp );
			tmp_flistp = PIN_FLIST_ELEM_ADD(arg_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp );
			PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_CITY, tmp_flistp, PIN_FLD_CITY, ebufp);			

			arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 5, ebufp );
			tmp_flistp = PIN_FLIST_ELEM_ADD(arg_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp );
			PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_CITY, wc_star, ebufp);
			
			/*  Bill when parameter in search is only required only for SUBS plan.
				If plan list pl_elem_id is 1(subs plan) then set BILL_WHEN passed in input
				otherwise bill when passed in input is ignored. (For Hardware plans) */
			if(pl_elem_id == 1) {
				arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 6, ebufp );
				tmp_flistp = PIN_FLIST_ELEM_ADD(arg_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp );
				PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_BILL_WHEN, tmp_flistp, PIN_FLD_BILL_WHEN, ebufp);
				PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);
			} else {
				PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template_hw , ebufp);
			}


			arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, 0, ebufp );
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_prod_price search input flist", srch_in_flistp);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
				*ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_fail, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51785", ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_prod_price: error in search.", ebufp);
				goto CLEANUP;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_prod_price search output flist.", srch_out_flistp);
			
			/*
			cookie = NULL;
			while((prod_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_out_flistp, PIN_FLD_RESULTS,
				&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
			{
	
				/* Get the Event type of product */	
				/*read_in_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_PRODUCT_OBJ, read_in_flistp, PIN_FLD_POID, ebufp);
				arg_flistp = PIN_FLIST_ELEM_ADD(read_in_flistp, PIN_FLD_USAGE_MAP, 0, ebufp );
				PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_EVENT_TYPE, (void *)NULL, ebufp);

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_prod_price_details read input", read_in_flistp);
				PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_in_flistp, &read_out_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&read_in_flistp, NULL);

				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling Read Flds", ebufp);
					*ret_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_fail, ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51782", ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_prod_price: error in Reading product.", ebufp);
					goto CLEANUP;
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_prod_price_details read output", read_out_flistp);
				
				arg_flistp = PIN_FLIST_ELEM_GET(prod_flistp, PIN_FLD_USAGE_MAP, 0, 1, ebufp );
				evt_type = PIN_FLIST_FLD_GET(arg_flistp, PIN_FLD_EVENT_TYPE, 1, ebufp);
				if (evt_type && strncmp(cycle_evt,evt_type,strlen(cycle_evt))==0)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Event type if Cycle forward.");
					SUBS_FLAG = 1;
				} */
				

				tmp_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp );
				arg_flistp = PIN_FLIST_ELEM_GET(tmp_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, 1, ebufp );
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_prod_price cities flist.", arg_flistp);
				if(arg_flistp)
				{
					/* Subscription plan and Hardware plan will have cycle fee products only so
						discount will be applied on cycle fee only. All other plans will have one
						time fee products so discount will be applied on purchase fee amount. 
						Subscription plan: pl_elem_id = 1; Hardware plan: pl_elem_id = 2;  */
					if(pl_elem_id == 1 || pl_elem_id == 2) {
						prod_charge = PIN_FLIST_FLD_GET(arg_flistp, PIN_FLD_CYCLE_FEE_AMT, 0, ebufp);
					} else {
						prod_charge = PIN_FLIST_FLD_GET(arg_flistp, PIN_FLD_PURCHASE_FEE_AMT, 0, ebufp);
					}
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,pbo_decimal_to_str(prod_charge, ebufp));
				}
				if(!arg_flistp || PIN_ERRBUF_IS_ERR(ebufp) || pbo_decimal_is_null(prod_charge, ebufp))
				{
					*ret_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_fail, ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51786", ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Plan price not configured correctly in /mso_cfg_credit_limit.", ebufp);
					goto CLEANUP;
				}
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,pbo_decimal_to_str(disc_amt, ebufp));
				/* If discount amount is more than charge amount then return error */
				if(pbo_decimal_compare(disc_amt, prod_charge, ebufp) > 0)
				{
					PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_STATUS, &invalid, ebufp);
					final_status = invalid;
					//PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_STATUS, &invalid, ebufp);
					
				}
				else
				{
					PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_STATUS, &valid, ebufp);
				}
				
		} // While plan
	}// While plan list
	tmp_flistp = PIN_FLIST_ELEM_TAKE(in_flistp, PIN_FLD_PLAN_LISTS, 1, 1, ebufp);
	if(tmp_flistp) {
		PIN_FLIST_SUBSTR_PUT(in_flistp, tmp_flistp, MSO_FLD_SUBSCRIPTION_PLANS, ebufp);
	}
	tmp_flistp = PIN_FLIST_ELEM_TAKE(in_flistp, PIN_FLD_PLAN_LISTS, 2, 1, ebufp);
	if(tmp_flistp) {
		PIN_FLIST_SUBSTR_PUT(in_flistp, tmp_flistp, MSO_FLD_HARDWARE_PLANS, ebufp);
	}
	tmp_flistp = PIN_FLIST_ELEM_TAKE(in_flistp, PIN_FLD_PLAN_LISTS, 3, 1, ebufp);
	if(tmp_flistp) {
		PIN_FLIST_SUBSTR_PUT(in_flistp, tmp_flistp, MSO_FLD_INSTALLATION_PLANS, ebufp);
	}	
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_STATUS, &final_status, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_prod_price_details Final flist.", in_flistp);
	*ret_flistp = PIN_FLIST_COPY(in_flistp, ebufp);
	CLEANUP:
		PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
        return;
}

void
fm_mso_get_payterm_for_plan(
	pcm_context_t           *ctxp,
	pin_flist_t             *in_flistp,
	pin_flist_t             **ret_flistp,
	pin_errbuf_t            *ebufp)
{

	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	char			*plan_name = NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_flist_t		*cities_flistp = NULL;
	pin_flist_t		*result_flist = NULL;
	char			*template = "select X from /mso_cfg_credit_limit where  F1 = V1 and F2 = V2 " ;
	int64			db = -1;
	int				srch_flag = 768;
	poid_t			*srch_pdp = NULL;
	int32			i = 0;
	int32			elem_id =0;
	int                     status_fail = 1;
        int                     status_ok = 0;
	pin_cookie_t	cookie = NULL;


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_payterm_for_plan input flist",in_flistp);
	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp));
	
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template, ebufp);
	
	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_PLAN_NAME, arg_flistp, MSO_FLD_PLAN_NAME, ebufp);
	
	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 2, ebufp );
	cities_flistp = PIN_FLIST_ELEM_ADD(arg_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_CITY, cities_flistp, PIN_FLD_CITY, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, 0, ebufp );
	cities_flistp = PIN_FLIST_ELEM_ADD(result_flist, MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(cities_flistp, PIN_FLD_BILL_WHEN, NULL, ebufp);


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_prod_price search input flist", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_payterm_for_plan: Error in calling SEARCH", ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_fail, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51785", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_get_payterm_for_plan: error in search.", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_payterm_for_plan search output flist.", srch_out_flistp);

	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_ok, ebufp);
	result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp );
	while((cities_flistp = PIN_FLIST_ELEM_GET_NEXT(result_flist, MSO_FLD_CITIES,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		arg_flistp = PIN_FLIST_ELEM_ADD(*ret_flistp, PIN_FLD_RESULTS, i, ebufp);
		PIN_FLIST_FLD_COPY(cities_flistp, PIN_FLD_BILL_WHEN, arg_flistp, PIN_FLD_BILL_WHEN, ebufp);
		i++;
	}
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_ok, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_payterm_for_plan output flist",*ret_flistp);
	CLEANUP:
		PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);

	return;
}

/**************************************************
* Function:     fm_mso_srch_device_acct()
* Owner:        Harish
* Decription:   Search the account associated to device
***************************************************/
static void
fm_mso_srch_device_acct(
	pcm_context_t           *ctxp,
	char                    *dev_id,
        int32                   *den_sp,
	pin_flist_t             **ret_flistp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t             *srch_flistp = NULL;
	pin_flist_t             *out_flistp = NULL;
	pin_flist_t             *srch_out_flistp = NULL;
	pin_flist_t             *arg_flist = NULL;
	pin_flist_t             *result_flist = NULL;
	pin_flist_t             *serv_flist = NULL;
	int32                   srch_flag = 256;
	poid_t                  *s_pdp = NULL;
	poid_t                  *new_d_pdp = NULL;
	int64                   db = 1;
	int32                   id = -1;
	char                    *s_template = "select x from /account 1, /device 2 "
						"where 1.F1 = 2.F2 and 2.F3 = V3 and 2.F4 = V4 ";
char                    *s_template_1 = "select x from /account 1, /device 2 "
						"where 1.F1 = 2.F2 and 2.F3 = V3 " ;
        char                    *den_nwp = "NDS1";
		char                    *dsn_nwp = "NDS2";
        char                    *hw_nwp = "NDS";

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_device_acct input device id");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,dev_id);

	srch_flistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
	
	/*
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS           INT [0] 256
	0 PIN_FLD_TEMPLATE        STR [0] "select x from /account 1, 
					/device 2 where 1.F1 = 2.F2 and 2.F3 = V3 "
	0 PIN_FLD_ARGS          ARRAY [1] allocated 20, used 1
	1     PIN_FLD_POID           POID [0] NULL poid pointer
	0 PIN_FLD_ARGS          ARRAY [2] allocated 20, used 1
	1     PIN_FLD_SERVICES    ARRAY [*]
	2       PIN_FLD_ACCOUNT_OBJ    POID [0] NULL poid pointer
	0 PIN_FLD_ARGS          ARRAY [3] allocated 20, used 1
	1     PIN_FLD_DEVICE_ID       STR [0] "10:59:34:5j:93:4a"
	0 PIN_FLD_RESULTS       ARRAY [0] allocated 20, used 1
	1     PIN_FLD_ACCOUNT_NO      STR [0] NULL str ptr

	nap(32695)> xop 7 0 1
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_RESULTS       ARRAY [0] allocated 20, used 2
	1     PIN_FLD_ACCOUNT_NO      STR [0] "1000015876"
	1     PIN_FLD_POID           POID [0] 0.0.0.1 /account 4870555 7
	*/

	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, s_pdp , ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag , ebufp);
	if (*den_sp == 2)
	{
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, s_template_1 , ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, s_template , ebufp);
	}
	

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
	serv_flist = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_SERVICES, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(serv_flist, PIN_FLD_ACCOUNT_OBJ, NULL , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_DEVICE_ID, dev_id , ebufp);

        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp );
                if (*den_sp == 1)
                {
                        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_MANUFACTURER, den_nwp, ebufp);
                }
				else if (*den_sp == 2)
                {
                        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_MANUFACTURER, dsn_nwp, ebufp);
                }
                else
                {
                        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_MANUFACTURER, hw_nwp, ebufp);
                }

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_NO, "" , ebufp);
	/*nameinfo_flist = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_NAMEINFO, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(nameinfo_flist, PIN_FLD_COMPANY, NULL , ebufp);*/
	

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_device_acct search input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_srch_device_acct - Error in calling SEARCH", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_device_acct search output flist", srch_out_flistp);
	if(PIN_FLIST_ELEM_COUNT(srch_out_flistp,PIN_FLD_RESULTS,ebufp) > 0)
	{
		result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
		*ret_flistp = PIN_FLIST_COPY(result_flist, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_device_acct return flist", *ret_flistp);
	}

	CLEANUP:
		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	return;
}



/**************************************************
* Function:     fm_mso_srch_device_sp_name()
* Owner:        Harish
* Decription:   Search the account associated to device
***************************************************/
static void
fm_mso_srch_device_sp_name(
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
* Function:             fm_mso_search_ip_pool()
* Owner:                Harish Kulkarni
* Decription:   Search IP Pool
*
*
* 0 PIN_FLD_POID               POID [0] 0.0.0.1 /search -1 0
* 0 PIN_FLD_USERID             POID [0] 0.0.0.1 /account 44029 10
* 0 PIN_FLD_DEVICE_TYPE     STR [0] "IP/STATIC"
* 0 PIN_FLD_FLAGS               INT [0] 201
*
*
***************************************************/
static void
fm_mso_search_ip_pool(
	pcm_context_t           *ctxp,
	pin_flist_t             *in_flistp,
	pin_flist_t             **ret_flistpp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t             *srch_iflistp = NULL;
	pin_flist_t             *srch_oflistp = NULL;
	pin_flist_t             *args_flist = NULL;
	int64                   db = -1;
	int32                   id = -1;
	int32                   srch_flag = 256;
	char                    *s_template1 = "select X from /mso_ip_order/ip/static where F1.type = V1 ";
	char                    *s_template2 = "select X from /mso_ip_order/ip/framed where F1.type = V1 ";
	char                    *dev_type = NULL;
	poid_t                  *pdp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_search_ip_pool", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_ip_pool input flist", in_flistp);
	dev_type = PIN_FLIST_FLD_GET(in_flistp,PIN_FLD_DEVICE_TYPE,0,ebufp);
	db = PIN_POID_GET_ID(PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp));
	if(!dev_type)
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_NONE, 0, 0, 0);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_search_ip_pool Device Type not found!!",
			ebufp);
		return;
	}

	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, srch_iflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &srch_flag, ebufp);


	if (dev_type && strcmp(dev_type,"IP/STATIC") == 0)
	{
		PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, s_template1 , ebufp);
		pdp = PIN_POID_CREATE(db, "/mso_ip_order/ip/static", id, ebufp);
	}

	if (dev_type && strcmp(dev_type,"IP/FRAMED") == 0)
	{
		PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, s_template2 , ebufp);
		pdp = PIN_POID_CREATE(db, "/mso_ip_order/ip/framed", id, ebufp);
	}

	args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_PUT(args_flist, PIN_FLD_POID, pdp , ebufp);

	args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_FLIST_FLD_SET(args_flist, MSO_FLD_IP_POOL_NAME, "" , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_ip_pool search input list", srch_iflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_search_ip_pool - Error in calling SEARCH", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_ip_pool search output flist", srch_oflistp);

	*ret_flistpp = PIN_FLIST_COPY(srch_oflistp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_ip_pool output flist", srch_oflistp);
	CLEANUP:
		PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	return;
}

/**************************************************
* Function:             fm_mso_search_failed_prov_resp()
* Owner:                Harish Kulkarni
* Decription:   For serching the failed provisioning responses
*
*
* 0 PIN_FLD_POID               POID [0] 0.0.0.1 /search -1 0
* 0 PIN_FLD_USERID             POID [0] 0.0.0.1 /account 44029 10
* 0 PIN_FLD_FLAGS               INT [0] 202
*
*
***************************************************/
static void
fm_mso_search_failed_prov_resp(
	pcm_context_t           *ctxp,
	pin_flist_t             *in_flistp,
	pin_flist_t             **ret_flistpp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t             *srch_iflistp = NULL;
	pin_flist_t             *srch_oflistp = NULL;
	pin_flist_t             *args_flist = NULL;
	int64                   db = -1;
	int32                   id = -1;
	int32                   srch_flag = 256;
	char                    *s_template = "select X from /mso_failed_prov_response where F1 <> V1 ";
	poid_t                  *pdp = NULL;
	int32                   failed_level = 0;
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_search_failed_prov_resp", ebufp);
			return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_failed_prov_resp input flist", in_flistp);

	db = PIN_POID_GET_ID(PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp));

	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, srch_iflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, s_template, ebufp);

	args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(args_flist, MSO_FLD_TASK_FAILED, &failed_level , ebufp);

	args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_failed_prov_resp search input list", srch_iflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_search_failed_prov_resp - Error in calling SEARCH", ebufp);
			goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_failed_prov_resp search output flist", srch_oflistp);

	*ret_flistpp = PIN_FLIST_COPY(srch_oflistp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_failed_prov_resp output flist", srch_oflistp);
	CLEANUP:
			PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
			PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	return;
}

/**************************************************
* Function:             fm_mso_search_prov_tags()
* Owner:                Harish Kulkarni
* Decription:   For serching the provisioning tags for given CMTS ID
*
*
* 0 PIN_FLD_POID               POID [0] 0.0.0.1 /search -1 0
* 0 PIN_FLD_USERID             POID [0] 0.0.0.1 /account 44029 10
* 0 PIN_FLD_FLAGS               INT [0] 203
* 0 MSO_FLD_CMTS_ID         STR [0] "KAMLA_MQ"
*
*
***************************************************/
static void
fm_mso_search_prov_tags(
	pcm_context_t           *ctxp,
	pin_flist_t             *in_flistp,
	pin_flist_t             **ret_flistpp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t             *srch_iflistp = NULL;
	pin_flist_t             *srch_oflistp = NULL;
	pin_flist_t             *args_flist = NULL;
	pin_flist_t             *cc_mapping_flistp = NULL;
	int64                   db = -1;
	int32                   id = -1;
	int32                   srch_flag = 768;
	char                    *s_template = "select X from /mso_config_bb_pt where F1 = V1 ";
	poid_t                  *pdp = NULL;
	int32                   dhcp_class = 300;
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_search_prov_tags", ebufp);
			return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_prov_tags input flist", in_flistp);

	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp));
	pdp = PIN_POID_CREATE(db, "/mso_config_bb_pt", -1, ebufp);

	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, srch_iflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, s_template, ebufp);

	args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(args_flist, PIN_FLD_POID, pdp, ebufp);

	args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(args_flist, PIN_FLD_PROVISIONING_TAG, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_prov_tags search input list", srch_iflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
    PIN_POID_DESTROY(pdp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_search_prov_tags - Error in calling SEARCH", ebufp);
			goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_prov_tags search output flist", srch_oflistp);

	*ret_flistpp = PIN_FLIST_COPY(srch_oflistp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_prov_tags output flist", srch_oflistp);
	CLEANUP:
			PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
			PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	return;
}

/**************************************************
* Function:             fm_mso_search_credit_limit_for_plan()
* Owner:                Nagaraja V
* Decription:   For serching the mso_cfg_credit_limit for given plan name
*
*
* 0 PIN_FLD_POID               POID [0] 0.0.0.1 /search -1 0
* 0 PIN_FLD_USERID             POID [0] 0.0.0.1 /account 44029 10
* 0 PIN_FLD_FLAGS               INT [0] 205
* 0 MSO_FLD_PLAN_NAME         STR [0] "PLAN_NAME"
*
***************************************************/
static void
fm_mso_search_credit_limit_for_plan(
	pcm_context_t           *ctxp,
	pin_flist_t             *in_flistp,
	pin_flist_t             **ret_flistpp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t             *srch_iflistp = NULL;
	pin_flist_t             *srch_oflistp = NULL;
	pin_flist_t             *args_flist = NULL;
	pin_flist_t             *cc_flist = NULL;
	int64                   db = -1;
	int32                   id = -1;
	int32                   srch_flag = 768;
	char                    *s_template = "select X from /mso_cfg_credit_limit where F1 = V1 ";
	poid_t                  *pdp = NULL;
	int32                   dhcp_class = 300;
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_search_cmts_cc_mapping", ebufp);
			return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_credit_limit_for_plan input flist", in_flistp);

	db = PIN_POID_GET_ID(PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp));
	//pdp = PIN_POID_CREATE(db, "/mso_cfg_credit_limit", -1, ebufp);

	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, srch_iflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, s_template, ebufp);

	args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_PLAN_NAME, args_flist, MSO_FLD_PLAN_NAME, ebufp);

	args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS, 0, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_credit_limit_for_plan search input list", srch_iflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_search_credit_limit_for_plan - Error in calling SEARCH", ebufp);
			goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_credit_limit_for_plan search output flist", srch_oflistp);

	*ret_flistpp = PIN_FLIST_COPY(srch_oflistp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_credit_limit_for_plan output flist", srch_oflistp);
	CLEANUP:
			PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
			PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	return;
}
/**************************************************
* Function:             fm_mso_search_cmts_cc_mapping()
* Owner:                Harish Kulkarni
* Decription:   For serching the mso_cfg_cmts_cc_mapping for given CMTS ID and prov_tag
*
*
* 0 PIN_FLD_POID               POID [0] 0.0.0.1 /search -1 0
* 0 PIN_FLD_USERID             POID [0] 0.0.0.1 /account 44029 10
* 0 PIN_FLD_FLAGS               INT [0] 204
* 0 MSO_FLD_CMTS_ID         STR [0] "KAMLA_MQ"
* 0 PIN_FLD_PROVISIONING_TAG    STR [0] "L_500KB_100KB_1024MB"
*
***************************************************/
static void
fm_mso_search_cmts_cc_mapping(
	pcm_context_t           *ctxp,
	pin_flist_t             *in_flistp,
	pin_flist_t             **ret_flistpp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t             *srch_iflistp = NULL;
	pin_flist_t             *srch_oflistp = NULL;
	pin_flist_t             *args_flist = NULL;
	pin_flist_t             *cc_flist = NULL;
	int64                   db = -1;
	int32                   id = -1;
	int32                   srch_flag = 768;
	char                    *s_template = "select X from /mso_cfg_cmts_cc_mapping where F1 = V1 and F2 = V2 ";
	poid_t                  *pdp = NULL;
	int32                   dhcp_class = 300;
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_search_cmts_cc_mapping", ebufp);
			return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_cmts_cc_mapping input flist", in_flistp);

	db = PIN_POID_GET_ID(PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp));
	//pdp = PIN_POID_CREATE(db, "/mso_config_bb_pt", -1, ebufp);

	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, srch_iflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, s_template, ebufp);

	args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_CMTS_ID, args_flist, MSO_FLD_CMTS_ID, ebufp);

	args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 2, ebufp );
	cc_flist = PIN_FLIST_ELEM_ADD(args_flist, MSO_FLD_CC_MAPPING, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROVISIONING_TAG, cc_flist, PIN_FLD_PROVISIONING_TAG, ebufp);

	args_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS, 0, ebufp );
	cc_flist = PIN_FLIST_ELEM_ADD(args_flist, MSO_FLD_CC_MAPPING, PIN_ELEMID_ANY, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_cmts_cc_mapping search input list", srch_iflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_search_cmts_cc_mapping - Error in calling SEARCH", ebufp);
			goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_cmts_cc_mapping search output flist", srch_oflistp);

	*ret_flistpp = PIN_FLIST_COPY(srch_oflistp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_cmts_cc_mapping output flist", srch_oflistp);
	CLEANUP:
			PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
			PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	return;
}

/**************************************************
* Function: 	fm_mso_srch_by_legacy_acc_no()
* Decription:   Search the Legacy Account no.
*		
* 
* 
***************************************************/
void
fm_mso_srch_by_legacy_acc_no(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *srch_flistp = NULL;
        pin_flist_t             *arg_flist = NULL;
        pin_flist_t             *result_flist = NULL;
        pin_flist_t             *srch_out_flistp = NULL;
        pin_flist_t             *nameinfo_billing = NULL;
        int32                   srch_flag = 768;

        char                    *template = "select X from /account where F1 = V1 " ;
        char                    *acnt_no = NULL;
	char  			leg_acc [20] = ""; 
        char                    *program_namep = NULL;
        int64                   db = -1;
        int32                   *bus_type_ptr = NULL;
        int32                   business_type = -1;
	int			count = 0;
	int			search_fail = 1;
	poid_t                  *srch_pdp = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_legacy_acc_no input flist", in_flistp);
        db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp));
        acnt_no =  PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_LEGACY_ACCOUNT_NO, 1, ebufp );
        program_namep =  PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	/*RAVI adding prefix "B-" to the account number */
		if ((acnt_no == NULL)||(strcmp(acnt_no,"")==0))
        	{
                	pin_set_err(ebufp, PIN_ERRLOC_FM,
                        	PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        	LEGACY_ACCOUNT_NUMBER_MISSING, 0, 0, 0);
                	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "legacy account number is missing",
                        	ebufp);
                        return;
       		}
	//MSO_FLD_LEGACY_ACCOUNT_NO_MISSING	
	
	/*memset(leg_acc,0,sizeof(leg_acc));
        	
	sprintf(leg_acc, "B-%s",acnt_no);*/

        srch_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, srch_flistp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	    PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
	    arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
        if (program_namep && strstr(program_namep, "DVB"))
        {
            memset(leg_acc, 0, sizeof(leg_acc));
            sprintf(leg_acc, "J-%s", acnt_no);
            PIN_FLIST_FLD_SET(arg_flist, MSO_FLD_LEGACY_ACCOUNT_NO, leg_acc, ebufp);
        }
        else
        {
            PIN_FLIST_FLD_SET(arg_flist, MSO_FLD_LEGACY_ACCOUNT_NO, acnt_no, ebufp);
        }

        result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
        PIN_FLIST_FLD_SET(result_flist, MSO_FLD_RMN, NULL , ebufp);
        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_NO, NULL , ebufp);
        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_BUSINESS_TYPE, NULL , ebufp);

        nameinfo_billing = PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_NAMEINFO, 1, ebufp );
        PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_FIRST_NAME, NULL , ebufp);
        PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_MIDDLE_NAME, NULL , ebufp);
        PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_LAST_NAME, NULL , ebufp);
        PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_COMPANY, NULL , ebufp);
        PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_CITY, NULL , ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_legacy_acc_no input list", srch_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
                goto CLEANUP;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_legacy_acc_no output flist", srch_out_flistp);
		count = PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp);
		if(!count) {
			PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
			srch_out_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, srch_out_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(srch_out_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
			PIN_FLIST_FLD_SET(srch_out_flistp, PIN_FLD_ERROR_CODE, "51791", ebufp);
			PIN_FLIST_FLD_SET(srch_out_flistp, PIN_FLD_ERROR_DESCR, "Legacy Account number not found.", ebufp);
		}
	
	CLEANUP:
	*ret_flistp = srch_out_flistp;
	return;
}

void
fm_mso_srch_by_salesperson_firstname(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{

	
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*nameinfo_billing = NULL;
	
	int32			srch_flag = 768;

	char			*template = "select X from /account where business_type = 21000000 and upper(F1) like V1 " ;
 	int64			db = -1;
	void			*vp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_salesperson_firstname input flist", in_flistp);

	vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_FIRST_NAME, 0, ebufp);
	if ( vp == NULL )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"Error in SEARCH for Sales Person- First Name field missing in input", ebufp);
		return;
	}
	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp));

	/*******************************************************************
	* Root Login Validation - Start
	*******************************************************************/
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, srch_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	nameinfo_billing = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_NAMEINFO, 1, ebufp );
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_FIRST_NAME, nameinfo_billing, PIN_FLD_FIRST_NAME, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_NO, NULL , ebufp);
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_RMN, NULL , ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_BUSINESS_TYPE, NULL , ebufp);

	nameinfo_billing = PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_NAMEINFO, 1, ebufp );
	PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_FIRST_NAME, NULL , ebufp);
	PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_MIDDLE_NAME, NULL , ebufp);
	PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_LAST_NAME, NULL , ebufp);
	PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_COMPANY, NULL , ebufp);
	PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_CITY, NULL , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_salesperson_firstname input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling Sales person firstname SEARCH", ebufp);
		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_salesperson_firstname output flist", srch_out_flistp);

	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	
	*ret_flistp = srch_out_flistp;

	return;
}



void
fm_mso_srch_by_agreement_no(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *srch_flistp = NULL;
        pin_flist_t             *arg_flist = NULL;
        pin_flist_t             *result_flist = NULL;
        pin_flist_t             *srch_out_flistp = NULL;
        pin_flist_t             *nameinfo_billing = NULL;
	pin_flist_t		*info_flistp = NULL;
        int32                   srch_flag = 768;

        char                    *template = "select X from /account 1, /service/telco/broadband 2 where 2.F1 = V1 and 1.F2 = 2.F3 " ;
        char                    *agg_nop = NULL;
        char                    aggp [80] = "";
        int64                   db = -1;
        int32                   *bus_type_ptr = NULL;
        int32                   business_type = -1;
        int                     count = 0;
        int                     search_fail = 1;
        poid_t                  *srch_pdp = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_legacy_acc_no input flist", in_flistp);
        db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp));
        agg_nop =  PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_AGREEMENT_NO, 1, ebufp );
        if ((agg_nop == NULL)||(strcmp(agg_nop,"")==0))
		{
                        pin_set_err(ebufp, PIN_ERRLOC_FM,
                                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                                PIN_ERR_BAD_ARG, MSO_FLD_AGREEMENT_NO, 0, 0);
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "agreement number is missing", ebufp);
                        return;
                }
        srch_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, srch_flistp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	info_flistp = PIN_FLIST_SUBSTR_ADD(arg_flist,MSO_FLD_BB_INFO,ebufp);
	if(strstr(agg_nop,"BB-"))
	{
		PIN_FLIST_FLD_SET(info_flistp, MSO_FLD_AGREEMENT_NO, agg_nop, ebufp);
	} else {
		memset(aggp,0,sizeof(aggp));
		sprintf(aggp, "BB-%s",agg_nop);
		PIN_FLIST_FLD_SET(info_flistp, MSO_FLD_AGREEMENT_NO, aggp, ebufp);
	}
        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL , ebufp);
        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, NULL , ebufp);

        result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
        PIN_FLIST_FLD_SET(result_flist, MSO_FLD_RMN, NULL , ebufp);
        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_NO, NULL , ebufp);
        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_BUSINESS_TYPE, NULL , ebufp);

        nameinfo_billing = PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_NAMEINFO, 1, ebufp );
        PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_FIRST_NAME, NULL , ebufp);
        PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_MIDDLE_NAME, NULL , ebufp);
        PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_LAST_NAME, NULL , ebufp);
        PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_COMPANY, NULL , ebufp);
        PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_CITY, NULL , ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_agreement_no input list", srch_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
                goto CLEANUP;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_agreement_no output flist", srch_out_flistp);
                count = PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp);
                if(!count) {
                        PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
                        srch_out_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, srch_out_flistp, PIN_FLD_POID, ebufp);
                        PIN_FLIST_FLD_SET(srch_out_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
                        PIN_FLIST_FLD_SET(srch_out_flistp, PIN_FLD_ERROR_CODE, "51792", ebufp);
                        PIN_FLIST_FLD_SET(srch_out_flistp, PIN_FLD_ERROR_DESCR, "Agreement number not found.", ebufp);
                }

CLEANUP:
        *ret_flistp = srch_out_flistp;
        return;
}

void fm_mso_srch_channel_master_data(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp)






{

        pin_flist_t     *s_flistp = NULL;
        pin_flist_t     *args_flistp = NULL;
        pin_flist_t     *res_flistp = NULL;
        pin_flist_t     *r_flistp = NULL;
        pin_flist_t     *data_flistp = NULL;
        pin_flist_t     *tmp_flistp = NULL;
        pin_flist_t     *sort_fld_flistp = NULL;
        pin_flist_t     *l_flistp = NULL;
        pin_flist_t     *data1_flistp = NULL;
        pin_flist_t     *data2_flistp = NULL;
        pin_flist_t     *data3_flistp = NULL;

        poid_t          *s_pdp =  NULL;
        poid_t          *tmp_pdp =  NULL;
        int64           db =  1;
        int32           elem_id =  0;
        pin_cookie_t    cookie =  NULL;
        int32           sflags =  256;
        char            template[512] =  "select X from /mso_cfg_catv_channel_master where F1.type = V1 ";
        char            *lang = NULL;
        char            *prev_vp = NULL;
        char            tmp_str[256];
        int32           counter = 0;
        int32           args_count = 2;
        char            *genre = NULL;
        char            *broadcaster = NULL;
        char            *quality = NULL;
        char            *popularity = NULL;
        int32           *type_p = NULL;
        int                     count = 0;
        int                     search_fail = 1;

        /* Clear the error buffer */
        PIN_ERR_CLEAR_ERR(ebufp);

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_channel_master_data enters");

        s_flistp = PIN_FLIST_CREATE(ebufp);
        s_pdp = PIN_POID_FROM_STR("0.0.0.1 /search -1 0", NULL, ebufp);
        PIN_FLIST_FLD_PUT(s_flistp,PIN_FLD_POID, s_pdp, ebufp);

        PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_FLAGS, (void*)&sflags, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
        tmp_pdp = PIN_POID_FROM_STR("0.0.0.1 /mso_cfg_catv_channel_master -1 0", NULL, ebufp);
        PIN_FLIST_FLD_PUT(args_flistp,PIN_FLD_POID, tmp_pdp, ebufp);

        if(i_flistp != NULL) {

                //Search by language
                lang = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_LANGUAGE, 1, ebufp);
                //Search by genre
                genre = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_CHANNEL_GENRE, 1, ebufp);
                //Search by broadcaster
                broadcaster = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_BROADCASTER, 1, ebufp);
                //Search by quality
                quality = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_QUALITY, 1, ebufp);
                //Search by popularity
                popularity = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_POPULARITY, 1, ebufp);
                //Search by type (pay/FTA)
                type_p = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_TYPE, 1, ebufp);

                if(lang != NULL) {
                        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, args_count, ebufp);
                        PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_LANGUAGE, lang, ebufp);
                        sprintf(tmp_str, " and F%d = V%d ", args_count, args_count);
                        strcat(template, tmp_str);
                        args_count++;
                }
                if(genre != NULL) {
                        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, args_count, ebufp);
                        PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_CHANNEL_GENRE, genre, ebufp);
                        sprintf(tmp_str, " and F%d = V%d ", args_count, args_count);
                        strcat(template, tmp_str);
                        args_count++;
                }
                if(broadcaster != NULL) {
                        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, args_count, ebufp);
                        PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_BROADCASTER, broadcaster, ebufp);
                        sprintf(tmp_str, " and F%d = V%d ", args_count, args_count);
                        strcat(template, tmp_str);
                        args_count++;
                }
                if(quality != NULL) {
                        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, args_count, ebufp);
                        PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_QUALITY, quality, ebufp);
                        sprintf(tmp_str, " and F%d = V%d ", args_count, args_count);
                        strcat(template, tmp_str);
                        args_count++;
                }
                if(popularity != NULL) {
                        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, args_count, ebufp);
                        PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_POPULARITY, popularity, ebufp);
                        sprintf(tmp_str, " and F%d = V%d ", args_count, args_count);
                        strcat(template, tmp_str);
                        args_count++;
                }
                if(type_p != NULL) {
                        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, args_count, ebufp);
                        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_TYPE, type_p, ebufp);
                        sprintf(tmp_str, " and F%d = V%d ", args_count, args_count);
                        strcat(template, tmp_str);
                }
        }
        PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_TEMPLATE, template, ebufp);
        res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_search search flist", s_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_flistp, ebufp);

        if (i_flistp == NULL) {
                *r_flistpp = PIN_FLIST_CREATE(ebufp);
                tmp_pdp = PIN_POID_FROM_STR("0.0.0.1 /account -1 0", NULL, ebufp);
                PIN_FLIST_FLD_PUT(*r_flistpp, PIN_FLD_POID, tmp_pdp, ebufp);

                fm_mso_sort_and_filter(ctxp, r_flistp, MSO_FLD_LANGUAGE, &data1_flistp, ebufp);
                fm_mso_sort_and_filter(ctxp, r_flistp, MSO_FLD_CHANNEL_GENRE, &data2_flistp, ebufp);
                fm_mso_sort_and_filter(ctxp, r_flistp, MSO_FLD_BROADCASTER, &data3_flistp, ebufp);

                PIN_FLIST_ELEM_PUT(*r_flistpp, data1_flistp, PIN_FLD_DATA_ARRAY, 0, ebufp);
                PIN_FLIST_ELEM_PUT(*r_flistpp, data2_flistp, PIN_FLD_DATA_ARRAY, 1, ebufp);
                PIN_FLIST_ELEM_PUT(*r_flistpp, data3_flistp, PIN_FLD_DATA_ARRAY, 2, ebufp);
        }
        else {
                count = PIN_FLIST_ELEM_COUNT(r_flistp, PIN_FLD_RESULTS, ebufp);
                if(!count) {
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51792", ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "No results for the selected options", ebufp);
                }
                *r_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);
        }
        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_channel_master_data exists");
        return ;
}

void fm_mso_sort_and_filter(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_fld_num_t     field,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp) {


        pin_flist_t     *sort_fld_flistp = NULL;
        pin_flist_t     *l_flistp = NULL;
        pin_flist_t     *data_flistp = NULL;
        pin_flist_t     *tmp_flistp = NULL;
        pin_flist_t     *res_flistp = NULL;

        int32           counter = 0;
        int32           elem_id = 0;
        pin_cookie_t    cookie = NULL;
        void            *vp = NULL;
        void            *prev_vp = NULL;

        /* Clear the error buffer */
        PIN_ERR_CLEAR_ERR(ebufp);

        sort_fld_flistp = PIN_FLIST_CREATE(ebufp);
        l_flistp = PIN_FLIST_ELEM_ADD(sort_fld_flistp,PIN_FLD_RESULTS, 0, ebufp );
        PIN_FLIST_FLD_SET(l_flistp, field, NULL, ebufp);
        PIN_FLIST_SORT(i_flistp,sort_fld_flistp,1,ebufp);
        PIN_FLIST_DESTROY_EX(&sort_fld_flistp, NULL);

        //PIN_FLIST_PRINT(r_flistp, NULL, ebufp);
        while((res_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_RESULTS,
                &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) {

                vp = PIN_FLIST_FLD_GET(res_flistp, field, 0, ebufp);

                if(vp == NULL || strlen(vp)==0) {
                        continue;
                }

                if (counter == 0) {
                        data_flistp = PIN_FLIST_CREATE(ebufp);
                        tmp_flistp = PIN_FLIST_ELEM_ADD(data_flistp, PIN_FLD_RESULTS, 0, ebufp);
                        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_NAME, vp, ebufp);
                        prev_vp = vp;
                        counter++;
                }
                else if (vp != NULL) {
                        if ( strcmp(prev_vp, vp) != 0) {
                                prev_vp = vp;
                                tmp_flistp = PIN_FLIST_ELEM_ADD(data_flistp, PIN_FLD_RESULTS, ++counter, ebufp);
                                PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_NAME, vp, ebufp);
                        }
                }
        }
        *r_flistpp = data_flistp;

}


/*****************************************************************************
* Function: 	fm_mso_search_ott_swap()
* Decription: Returns the swapped device details in a particular date range 
******************************************************************************/
void 
fm_mso_search_ott_swap(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_flist_t		*arg_flistpn = NULL;
	pin_flist_t		*result_flist = NULL;

	poid_t			*srch_pdp = NULL;
	int32			srch_flag = 256;
	int64			db = 1;
	int 			ott =OTT_MODEM_VAL;

	char			*template = "select X from /event/activity/mso_lifecycle/swap_device where  F1 = V1 and F2 between V2 and V3 " ;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_search_ott_swap", ebufp);
		return;
	}
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	arg_flistpn = PIN_FLIST_ELEM_ADD(arg_flistp, MSO_FLD_DEVICE_SWAP, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(arg_flistpn, MSO_FLD_OFFER_VALUE, &ott, ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 2, ebufp );
	arg_flistpn = PIN_FLIST_ELEM_ADD(arg_flistp, MSO_FLD_DEVICE_SWAP, PIN_ELEMID_ANY, ebufp );
       PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_START_T, arg_flistpn, PIN_FLD_VALID_TO, ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 3, ebufp );
	arg_flistpn = PIN_FLIST_ELEM_ADD(arg_flistp, MSO_FLD_DEVICE_SWAP, PIN_ELEMID_ANY, ebufp );
       PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_END_T, arg_flistpn, PIN_FLD_VALID_TO, ebufp);
	result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	arg_flistpn = PIN_FLIST_ELEM_ADD(result_flist, MSO_FLD_DEVICE_SWAP, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(arg_flistpn, PIN_FLD_OLD_VALUE, NULL, ebufp);
	PIN_FLIST_FLD_SET(arg_flistpn, PIN_FLD_NEW_VALUE, NULL, ebufp);
	PIN_FLIST_FLD_SET(arg_flistpn, PIN_FLD_ACCOUNT_NO, NULL, ebufp);
	PIN_FLIST_FLD_SET(arg_flistpn, PIN_FLD_VALID_TO, NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_ott_swap input flist", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);

	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_ott_swap output flist", srch_out_flistp);	

	*ret_flistp = PIN_FLIST_COPY(srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);

	return;
}

/**************************************************
* Function: 	fm_mso_srch_by_rmn_new()
* Owner:        Gautam Adak
* Decription:   Search the login availability
*		
* 
* 
***************************************************/
void
fm_mso_srch_by_rmn_new(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{

	
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*nameinfo_billing = NULL;
	poid_t			*srch_pdp = NULL;

	
	int32			srch_flag = 768;
	int32			count = 0;

	char			*template = "select X from /account 1, /service 2 where 1.F1 = V1 and  service_t.account_obj_id0=account_t.poid_id0 and service_t.poid_type='/service/telco/broadband' " ;
 	int64			db = -1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	srch_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_rmn input flist", in_flistp);

	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp));

	/*******************************************************************
	* Root Login Validation - Start
	*******************************************************************/
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, srch_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_RMN, arg_flist, MSO_FLD_RMN, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_NO, NULL , ebufp);


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_rmn input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_by_rmn output flist", srch_out_flistp);

		
	CLEANUP:
	count = PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp);
	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, srch_pdp , ebufp);	
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_COUNT, &count, ebufp);	
	return;
}

