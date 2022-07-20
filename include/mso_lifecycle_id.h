/*
* Account Types
*/

/*String ID definition*/
#define		ID_REGISTER_BUSINESS_UNIT			1
#define		ID_REGISTER_BUSINESS_USER			2
#define		ID_REGISTER_CUSTOMER				3
#define		ID_CREATE_BB_SERVICE				4
#define		ID_ALLOCATE_HARDWARE				5
#define		ID_MODIFY_ACTIVATION_PLAN_LIST			6
#define		ID_ACTIVATE_BB_SERVICE				7
#define		ID_ACTIVATE_CATV_SERVICE			8
#define		ID_SUSPEND_SERVICE				9
#define		ID_REACTIVATE_SERVICE				10
#define		ID_TERMINATE_SERVICE				11
#define		ID_HOLD_SERVICE					12
#define		ID_UNHOLD_SERVICE				13
#define		ID_ADD_PLAN					14
#define		ID_CHANGE_PLAN					15
#define		ID_REMOVE_PLAN					16
#define		ID_TOPUP_PREPAID				17
#define		ID_RENEW_PREPAID				18
#define		ID_RENEW_POSTPAID_AUTOMATIC			19
#define		ID_FUP_TOPUP					20
#define		ID_FDP_RENEW					21
#define		ID_MODIFY_CUSTOMER_ADDRESS			22
#define		ID_MODIFY_CUSTOMER_CONTACT			23
#define		ID_MODIFY_CUSTOMER_TAX_EXEMPTION		24
#define		ID_MODIFY_CUSTOMER_BILL_SUPPRESSION		25
#define		ID_VALIDITY_EXTENTION_FOR_A_SERVICE_PLAN	26
#define		ID_MODIFY_CREDIT_LIMIT				27
#define		ID_MODIFY_BB_SEVICE_CMTS			28
#define		ID_MODIFY_BB_SERVICE_AUTHENTICATION		29
#define		ID_MODIFY_CATV_SERVICE_REGION_KEY		30
#define		ID_MODIFY_CATV_SERVICE_BOUQUET_ID		31
#define		ID_MODIFY_CATV_SERVICE_CAF_RECEIVED_STATUS	32
#define		ID_MODIFY_CATV_SERVICE_CAS_SUBSCRIBER_ID	33
#define		ID_MODIFY_CATV_SERVICE_PERSONAL_BIT		34
#define		ID_HIERARCHY_MODIFICATION			35
#define		ID_BUSINESS_TYPE_MODIFICATION			36
#define		ID_BILL_INFORMATION_MODIFICATION		37
#define		ID_PAYMENT_INFORMATION_MODIFICATION		38
#define		ID_DEACTIVATE_VALIDITY_EXPIRY			39
#define		ID_DEACTIVATE_QUOTA_EXPIRY			40
#define		ID_SWAP_DEVICE					41
#define		ID_SWAP_IP					42
#define		ID_MODIFY_CUSTOMER_CHANGE_USERNAME_PASSWORD	43
#define		ID_MODIFY_CUSTOMER_AR_LIMIT_BUS_USER		44
#define		ID_MODIFY_ROLES_OF_CSR				45
#define		ID_MODIFY_ACCESSCONTROL_FOR_CSR			46
#define		ID_MODIFY_PROFILE_INFO_FOR_BUSINESS_UNIT	47
#define		ID_MODIFY_BB_SERVICE_LOGIN			48
#define		ID_RESET_BB_SERVICE_LOGIN_PASSWORD		49
#define		ID_MODIFY_CUSTOMER_DEACTIVATE_ACTIVATE_BUS_USER	50
#define		ID_DEVICE_DISASSOCIATE				51
#define		ID_ADDITONAL_MB					52
#define		ID_PAYMENT					53
#define		ID_PAYMENT_REVERSAL				54
#define		ID_GRV_UPLOAD					55
#define		ID_REFUND					56
#define		ID_REFUND_REVERSAL				57
#define		ID_DEPOSIT					58
#define		ID_CREDIT_DEBIT_NOTE				59
#define		ID_ADJUSTMENT					60
#define		ID_DISPUTE					61
#define		ID_SETTLEMENT_OF_DISPUTE			62
#define		ID_BILLING					63
#define		ID_PROV_ORDER_RESUBMIT				64
#define		ID_CREATE_SCHEDULE_ACTION			65
#define		ID_MODIFY_SCHEDULE_ACTION			66
#define		ID_RESUBMIT_SUSPENDED_EVENT			67
#define		ID_OPERATION_ADD_ACTION				68
#define		ID_OPERATION_MODIFY_ACTION			69
#define		ID_OPERATION_DELETE_ACTION			70
#define		ID_ADD_IP					71
#define		ID_REMOVE_IP					72
#define         ID_FUP_DOWNGRADE                                73
#define         ID_LATE_FEE                                     74
#define		ID_TECHNOLOGY_CHANGE				75
#define         ID_SWAP_TAGGING                                 76
#define         ID_SUBSCRIPTION_TRANSFER                        77
#define         ID_ADD_OFFERS                                   81
#define		ID_CREATE_GST_INFO				78
#define         ID_UPDATE_GST_INFO                              79
#define		ID_UPDATE_POI_POA				80
#define 	ID_PP_TYPE_CHANGE				82
#define         ID_ADD_OFFERS                                   81
#define         ID_ADD_NETFLIX                                  83
#define         ID_NETFLIX_PAYMENT                              84
#define         ID_CHANGE_PLAN_BB                             	86
#define         ID_NETFLIX_REVERSAL                             87
#define         ID_SWAP_OTT	                                88
/*IDs for Hybrid Account Tagging, De-Tagging and Re-Tagging*/
#define     ID_TAG_HYBRID_ACCT	                        	89
#define     ID_DETAG_HYBRID_ACCT	                    	90

/*String VERSION  definition*/
#define		VER_PAYMENT_COLLECT				1
#define		VER_PAYMENT_LCO_SOURCE				2
#define		VER_PAYMENT_LCO_DEST				3

#define		VER_PYMT_REV					1
#define		VER_PYMT_REV_LCO_SOURCE				2
#define		VER_PYMT_REV_LCO_DEST				3
#define		VER_PYMT_REV_CHQ_BOUNCE				4

#define		VER_CR_NOTE					1
#define		VER_DR_NOTE					2

#define 	TAG_HYBRID_ACCTS_EVENT				"/event/activity/mso_lifecycle/tag_hybrid_acct"
#define 	DETAG_HYBRID_ACCTS_EVENT			"/event/activity/mso_lifecycle/detag_hybrid_acct"



