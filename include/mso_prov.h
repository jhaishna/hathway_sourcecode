/*
* CATV provisioning action flags
*/
#define CATV_ACTIVATION                   1
#define CATV_ADD_PACKAGE                    2
#define CATV_PLAN_CHANGE                    3
#define CATV_CANCEL_PLAN                    4
#define CATV_SUSPEND                        5
#define CATV_REACTIVATE                     6
#define CATV_TERMINATE                      7
#define CATV_CHANGE_VC                      8
#define CATV_CHANGE_STB                     9
#define CATV_LOCATION_CHANGE                10
#define CATV_EMAIL                          11
#define CATV_RETRACK                        12
#define CATV_SET_USER_PARAMETERS            13
#define CATV_FP_NDS                         14
#define CATV_FP_SINGLE_CHANNEL_PK           15
#define CATV_FP_ALL_CHANNELS_SINGLE_STB_PK  16
#define CATV_FP_ALL_STBS_PK                 17
#define CATV_RESEND_ZIPCODE                 18
#define CATV_CHANGE_BOUQUET_ID              19
#define CATV_SET_BIT                        20
#define CATV_PREACTIVATION                  21
#define CATV_CHANGE_PREACTIVATED_PLAN       22
#define CATV_SEND_OSD                       23
#define CATV_RESET_PIN_PK                   24
#define CATV_UNSET_BIT                      25
#define CATV_FP_GOSPEL                      26
#define      BB_ACTIVATION               100

/*
 * Action Mode for preactivated service modifications
 */
#define PREACTV_CHANGE_PLAN					101
#define PREACTV_SUSPEND_SERVICE				102
#define PREACTV_REACTIVATE_SUBSCRIBER		103
#define PREACTV_SWAP_STB					104
#define PREACTV_SWAP_VC						105
#define PREACTV_LOCATION_CHANGE				106
#define PREACTV_CHANGE_BOUQUET_ID			107
#define PREACTV_RESEND_ZIP_CODE				108
#define PREACTV_RESET_PIN					109
#define PREACTV_BMAIL						110
#define PREACTV_OSD							111
#define PREACTV_RETRACK						112

#define CATV_PREACTV_MOD_SVC				113


/*
* Search flags for failed order
*/
#define SEARCH_BY_ORDER_ID                  1
#define SEARCH_BY_ERROR_CODE                2
#define SEARCH_BY_ERROR_DESCR               3
#define SEARCH_BY_PROVISIONING_ACTION       4
#define SEARCH_BY_ORDER_CREATION_DATE       5
#define SEARCH_BY_STATUS       		    6
#define SEARCH_BY_ACCNT_ORDER_ID                  7
#define SEARCH_BY_ACCNT_ERROR_CODE                8
#define SEARCH_BY_ACCNT_ERROR_DESCR               9
#define SEARCH_BY_ACCNT_PROVISIONING_ACTION       10
#define SEARCH_BY_ACCNT_ORDER_CREATION_DATE       11
#define SEARCH_BY_ACCNT_STATUS                    12

/*
* Provisioning status
*/
#define MSO_PROV_STATE_NEW                  1
#define MSO_PROV_STATE_IN_PROGRESS          1
#define MSO_PROV_STATE_FAILED_PROVISIONING  3
#define MSO_PROV_STATE_CORRECTED            4
#define MSO_PROV_STATE_SUCCESS              0
#define MSO_PROV_STATE_SUSPENDED            5
#define MSO_PROV_STATE_HOLD                 6
#define MSO_PROV_STATE_TERMINATED           7
#define MSO_PROV_STATE_ACTIVE               2
#define MSO_PROV_STATE_DEACTIVE             4


/*
 * BB provisioning action flags
 */
#define DOC_BB_ACTIVATION               101
#define DOC_BB_UPDATE_CAP               102
#define DOC_BB_FUP_REVERSAL             103
#define DOC_BB_DEACTIVATE_PKG_EXP       104
#define DOC_BB_DEACTIVATE_PQUOTA_EXP    105
#define DOC_BB_SUSPEND                  106
#define DOC_BB_HOLD_SERV                107
#define DOC_BB_TOPUP_BEXPIRY_NPC        108
#define DOC_BB_TOPUP_BEXPIRY_PC         109
#define DOC_BB_RENEW_AEXPIRY_PRE        110
#define DOC_BB_AUTO_RENEW_POST          111
#define DOC_BB_REACTIVATE_SERV          112
#define DOC_BB_UNHOLD_SERV              113
#define DOC_BB_PC_QUOTA_NQUOTA          114
#define DOC_BB_PC_NQUOTA_QUOTA          115
#define DOC_BB_PC_QUOTA_QUOTA           116
#define DOC_BB_PC_NQUOTA_NQUOTA         117
#define DOC_BB_PC_PRE_POST              118
#define DOC_BB_PC_POST_PRE              119
#define DOC_BB_CAM_TI                   120
#define DOC_BB_CAM_TP                   121
#define DOC_BB_CAM_IT                   122
#define DOC_BB_CAM_PT                   123
#define DOC_BB_CAM_IP                   124
#define DOC_BB_CAM_PI                   125
#define DOC_BB_MODEM_CHANGE             126
#define DOC_BB_IP_CHANGE                127
#define DOC_BB_CMTS_CHANGE_T            128
#define DOC_BB_CMTS_CHANGE_NT           129
#define DOC_BB_CHANGE_NAME              130
#define DOC_BB_CHANGE_PASS              131
#define DOC_BB_END_SUBS                 132
#define DOC_BB_RESET_DEVICE             133
#define DOC_BB_TERMINATE_SERV		134
#define DOC_BB_CAM_TN                   135
#define DOC_BB_CAM_NT                   136
#define DOC_BB_CAM_IN                   137
#define DOC_BB_CAM_PN                   138
#define DOC_BB_CAM_NI                   139
#define DOC_BB_CAM_NP                   140
#define ETHERNET_BB_DEACTIVATE_PKG_EXP  204
#define ETH_BB_MAC_CHANGE		205
#define ETH_BB_ADD_SUBNET_MASK		206
#define ETH_BB_GET_USER_DETAIL		207
#define ETH_BB_GET_LAST_SESSION_DETAIL	208
#define ETH_BB_UPDATE_SUBS_POOL		209
#define ETH_BB_RELEASE_IP_ADDRESS	210
#define ETH_BB_TOPUP_BEXPIRY_LIMITED	211
#define DOC_BB_ADD_MB			212
#define DOC_BB_FUP_TOPUP                213
#define DOC_BB_PLAN_VALIDITY_EXTENSION  214
#define FIBER_BB_ADD_IP                 215
#define GPON_MAC_CHANGE                 216
//------For Subnet Mask
#define MSO_SUBNET_32			"255.255.255.224"
#define MSO_SUBNET_4			"255.255.255.252"
#define MSO_SUBNET_8			"255.255.255.248"
#define MSO_SUBNET_16			"255.255.255.240"
#define MSO_SUBNET_64			"255.255.255.192"

//------Quota Balance for Unlimited Plan
#define UNLIMITED_QUOTA		"1073740750258176"

//-----Client Class ID
#define TAL_CLASS		200
#define IP_POOL			100
#define DHCP_CLASS		300

//------Technology
#define DOCSIS2			1
#define DOCSIS3			2
#define ETHERNET		3
#define FIBER			4
#define GPON			5
//#define WIFI			5

/**** Values for Failed response process ****/
#define MSO_PROV_RESPONSE_NEW 0
#define MSO_PROV_RESPONSE_RESUBMIT 1

// Carrier_ID
#define HW_NETWORK_PROVIDER 0
#define JIO_NETWORK_PROVIDER 1
#define JIO_NETWORK_ELEMENT_ID "JCPS01"
#define HW_NETWORK_ELEMENT_ID "CPS01"

