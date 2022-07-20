#define MSO_FLAG_SRCH_BY_ID 1
#define MSO_FLAG_SRCH_BY_PIN 2
#define MSO_FLAG_SRCH_BY_ACCOUNT 3
#define MSO_FLAG_SRCH_BY_JV 4
#define MSO_FLAG_SRCH_BY_DT 5
#define MSO_FLAG_SRCH_BY_SDT 6
#define MSO_FLAG_SRCH_BY_LCO 7
#define MSO_FLAG_SRCH_LCO_OF_SUBSCRIBER 8
#define MSO_FLAG_SRCH_BY_SELF_ACCOUNT 9
#define MSO_FLAG_SRCH_BY_SERVICE 10

#define WHOLESALE "/profile/wholesale"
#define CUSTOMER_CARE "/profile/customer_care"

#define HDW_PROD_PRI_START  0
#define HDW_PROD_PRI_END    60
#define SUBS_PROD_PRI_START 90   //90 is for price zero plan, actual subscription plan priority to be considered from 100
#define SUBS_PROD_PRI_END   170
#define OTC_PROD_PRI_START  200
#define OTC_PROD_PRI_END    210

#define DEPOSIT_PRODUCT 20

#define PLAN_TYPE_ALL  0
#define PLAN_TYPE_HDW  1
#define PLAN_TYPE_SUBS 2
#define PLAN_TYPE_OTC  3
#define PLAN_TYPE_INST 4
#define PLAN_TYPE_MISC 5
#define PLAN_TYPE_AMC 6
#define PLAN_TYPE_TAL 7

#define ALL         0
#define CATV        1
#define BB          2

#define BILLINFOID_CATV "CATV"
#define BILLINFOID_BB "BB"
#define BILLINFOID_WS "WS_SETTLEMENT"



#define CORPORATE_PARENT 1
#define CORPORATE_CHILD 2

#define MAX_PP_COUNT 3000 //Maximum possible purchased_product

#define LOCAL_TRANS_OPEN_SUCCESS 0
#define LOCAL_TRANS_OPEN_FAIL 1

#define REGISTER_CUSTOMER "Customer Registration"
#define MODIFY_CUSTOMER "Modification of Customer information"
#define SUSPEND_SERVICE "Service Suspension due to non payment"
#define SERVICE_RECONNECT "Service Reconnect as customer  clears due amount"
#define HOLD_SERVICE "Customer requested Suspension"
#define UNHOLD_SERVICE "Customer requested Reactivation"
#define TERMINATE_SERVICE "Terminate Service Request"

#define DATA_CONSUMED_MB      1000103
#define FUP_DATA_CONSUMED_MB  1000104
#define FREE_MB               1100010
#define CURRENCY              356

#define EVENT_MSO_REFUND_CREDIT "/event/billing/mso_refund_credit"
#define EVENT_MSO_REFUND_DEBIT "/event/billing/mso_refund_debit"
#define CHK_BOUNCE_EVENT "/event/billing/mso_penalty"

#define MSO_ACTIVE_STATUS    10100
#define MSO_PREPAID           2
#define MSO_POSTPAID          1
#define MSO_ACCT_ONLY        (-1)


/******* Pavan Bellala 25-Nov-2015 ***************
Search flags for querying details from OAP
Macros moved from .c to header file
*************************************************/
#define SRCH_LOGIN                      0
#define SRCH_BY_ACNT_NO                 1
#define SRCH_BY_RMN                     2
#define SRCH_BY_NAME                    3
#define SRCH_TRN_HIST                   4
#define SRCH_BY_COMPANY                 5
#define SRCH_BY_CARD_NO                 6
#define SRCH_BY_MOBILE_NO               11
#define SRCH_BY_EMAIL                   12
#define SRCH_BY_LEGACY_ACC_NO           13
#define SRCH_BY_AGREEMENT_NO	     14
#define SRCH_OTT_SWAP_DETAILS	     15
#define SRCH_BY_RMN_NEW               16

#define SRCH_FOR_OPT_START              50
#define SRCH_FOR_OPT_END                99
#define MAX_VAL                         210

#define SRCH_ORDER_BY_ORDER_ID          101
#define SRCH_ORDER_BY_ERROR_CODE        102
#define SRCH_ORDER_BY_ERROR_DESCR       103
#define SRCH_ORDER_BY_PROV_ACTION       104
#define SRCH_ORDER_BY_CREATION_DATE     105
#define SRCH_ORDER_BY_STATUS            106
#define SRCH_ORDER_BY_ACCNT_ORDER_ID    107
#define SRCH_ORDER_BY_ACCNT_ERROR_CODE  108
#define SRCH_ORDER_BY_ACCNT_ERROR_DESCR 109
#define SRCH_ORDER_BY_ACCNT_PROV_ACTION 110
#define SRCH_ORDER_BY_ACCNT_CREATION_DATE       111
#define SRCH_ORDER_BY_ACCNT_STATUS              112
#define SRCH_VC_BY_CARD_NO              120
#define SRCH_STB_BY_NDS_NO              121
#define SRCH_STB_BY_SERIAL_NO           122
#define SRCH_VC_BY_STATE_LCO            123
#define SRCH_STB_BY_STATE_LCO           124
#define SRCH_PREACTV_STB                        125
#define SRCH_PREACTV_VC                 126
#define SRCH_ACCT_BAL                   127
#define SRCH_CONTACT_HISTORY            128
#define SRCH_BB_USAGE_HISTORY           129
#define SRCH_BB_REASON_CODE             130
#define SRCH_MODEM_BY_MAC_ID    131
#define SRCH_ROUTER_BY_MAC_ID   132
#define SRCH_CABLE_ROUTER_BY_MAC_ID     133
#define SRCH_MODEM_BY_STATE_LCO         134
#define SRCH_ROUTER_BY_STATE_LCO        135
#define SRCH_ROUTER_CAB_BY_STATE_LCO    136
#define SRCH_NAT_BY_DEVICE_ID   137
#define SRCH_NAT_BY_STATE_LCO   138
#define SRCH_DEVICE_BY_SL_NO    139
#define SRCH_DEVICE_BY_PO_NO    140
#define SRCH_PROD_PRICE_BY_POID         141
#define SRCH_PAYTERM_FOR_PLAN           142

#define ACCESS_LEVEL_STATE              1
#define ACCESS_LEVEL_CITY               2
#define ACCESS_LEVEL_ZIP                3

#define TRANS_SRCH_ALL                  0
#define TRANS_SRCH_ONETIME              1
#define TRANS_SRCH_MONTHLY              2
#define TRANS_SRCH_PAYMENT              3
#define TRANS_SRCH_ADJUSTMENT           4
#define TRANS_SRCH_LIFECYCLE            5
#define TRANS_SRCH_WALLET               6

#define SRCH_BY_NAME_LIMIT              PIN_ELEMID_ANY

#define SRCH_BY_LIFECYCLE               150
#define SRCH_BY_VC_ID                   2
#define SRCH_BY_STB_ID                  1
#define SRCH_BY_ACCOUNT_NO              0
#define SRCH_BY_DATE                    4
#define SRCH_BY_ACTION                  3
#define SRCH_BY_MAC_ID                  5
#define SRCH_BY_IP                      6

#define SRCH_SCHEDULE_BY_DATES          160
#define SRCH_BY_ACCT_NO_ACTION  3
#define SRCH_BY_SCH_DATE                1

// added by Tim
#define SRCH_DEVICE_IP          200

#define SRCH_IP_POOL            201
#define SRCH_FAILED_PROV_RESP           202
#define SRCH_CMTS_PROV_TAGS             203
#define SRCH_CMTS_CC_MAPPING            204
#define SRCH_CREDIT_LIMIT_FOR_PLAN    205

// Added by Muthu on 14th Oct 2015 - Search by Sales Person Firstname

#define SRCH_BY_ORGLIST    209
#define SRCH_SALES_PERSON_FIRST_NAME    210
#define SRCH_PROMO_OFFER  15
//New Tariff
#define SRCH_DISTINCT_CHANNEL_ATTR    171
#define SRCH_CHANNEL_MASTER    172

