#ifndef _MSO_ERRS_H
#define _MSO_ERRS_H

#define MSO_ERR_COMMISSION_FAIL	"56010"
#define MSO_ERR_COMMISSION_FAIL_MSG	"Failure in applying commission for this event"
#define MSO_ERR_RATING_FAIL	"56011"
#define MSO_ERR_RATING_FAIL_MSG "Failure in charging the CDR."
#define MSO_ERR_SUSPENSE_FAIL "56012"
#define MSO_ERR_SUSPENSE_FAIL_MSG "Failed to process as a previous record is suspended for this session"
#define MSO_ERR_SERVICE_NOT_FOUND	"56013"
#define MSO_ERR_SERVICE_NOT_FOUND_MSG	"Service not found for this event"
#define	MSO_ERR_PRICE_CALC_ACC_NOT_FOUND "56014"
#define	MSO_ERR_PRICE_CALC_ACC_NOT_FOUND_MSG "Account/Service Object for Price calcuation not found"
#define MSO_ERR_DAILY_USAGE_DETAIL_INPUT_ERR "56015"
#define MSO_ERR_DAILY_USAGE_DETAIL_INPUT_ERR_MSG "Could not find daily usage details"
#define MSO_ERR_SESSION_DUPLICATE_ERR_MSG	"Stop record of this session is already processed. No need to process cdr again."
#define MSO_ERR_HIGHER_CDR_PROCESSED_ERR_MSG	"Higher value cdr's already processed. So rejecting the cdr."
#define MSO_ERR_USG_SESSION_CREATE_ERR_MSG	"Usage session object creation error"
#define MSO_ERR_TRANS_ERR_MSG                   "Transaction open error"

#define MSO_ERR_PLAN_NOT_FOUND	56000

#define MSO_ERR_DAILY_USAGE_DETAIL_NOT_FOUND 56001
#define MSO_ERR_ERROR_RESUBMIT_SUSP_EVENT    56002
#define	MSO_ERR_DEACTIVATE_BB_SERVICE_FAILED 56003
#define MSO_ERR_SESSION_DUPLICATE	     56016
#define	MSO_ERR_HIGHER_CDR_PROCESSED	     56017
#define MSO_ERR_USG_SESSION_CREATE	     56018
#define MSO_ERR_TRANS_ERROR                  56019


//For PROVISIONING BB
#define MSO_BB_MANDATORY_FIELD_CODE	52101
#define MSO_BB_MANDATORY_FIELD_DESCR	"Provisioning fail, mandatory field missing"
#define MSO_BB_INVALID_ACTION_CODE	52110
#define MSO_BB_INVALID_ACTION_DESCR	"Provisioning fail, Invalid provisioning action."
#define MSO_BB_PLAN_OBJ_MISSING		52210
#define MSO_BB_PLAN_OBJ_MISSING_ERROR	"Provisioning fail, plan poid missing"
#define MSO_BB_SEQNO_FAIL_CODE		52211
#define MSO_BB_SEQNO_FAIL_DESCR		"Provisioning fail, due to invalid sequence number"
#define MSO_BB_INTERNAL_ERROR		52213
#define MSO_BB_INTERNAL_ERROR_DESCR	"Provisioning fail, due to internal error, pleace check the logs"
#define MSO_BB_LOGIN_MISSING		52214
#define MSO_BB_LOGIN_MISSING_DESCR	"Provisioning fail, service login is missing"
#define MSO_BB_TECH_MISSING		52215
#define MSO_BB_TECH_MISSING_DESCR	"Provisioning fail, technology is missing in service"
#define MSO_BB_NE_ELEMENT_MISSING	52216
#define MSO_BB_NE_ELEMENT_MISSING_DESCR		"Provisioning fail, network element mssing in service"
#define MSO_NO_PROV_ACTION		52217
#define MSO_NO_PROV_ACTION_DESCR	"No provisioning action defined for this action flag and technology"
#define MSO_BB_TAL_FLAG_MISSING		52218
#define MSO_BB_TAL_FLAG_MISSING_DESCR		"Provisioning fail, isTAL flag not defined in service"
#define MSO_BB_1CLICK_FLAG_MISSING		52219
#define MSO_BB_1CLICK_FLAG_MISSING_DESCR		"Provisioning fail, is1Click flag not defined in service"
#define MSO_BB_INDICATOR_MISSING		52220
#define MSO_BB_INDICATOR_MISSING_DESCR		"Provisioning fail, Indicator field is missing in service"
#define MSO_BB_NAME_MISSING			52221
#define MSO_BB_NAME_MISSING_DESCR		"Provisioning fail, Name is missing for the account"
#define MSO_BB_PHONE_MISSING			52222
#define MSO_BB_PHONE_MISSING_DESCR		"Provisioning fail, Phone is missing for the account"
#define MSO_BB_EMAIL_MISSING			52223
#define MSO_BB_EMAIL_MISSING_DESCR		"Provisioning fail, Email is missing for the account"
#define MSO_BB_NW_ID_MISSING			52224
#define MSO_BB_NW_ID_MISSING_DESCR		"Provisioning fail, network id missing in service"
#define MSO_BB_QUOTA_ERROR			52225
#define MSO_BB_QUOTA_ERROR_DESCR		"Provisioning fail, quota balance not found in service"
#define MSO_BB_OLD_PLAN_ERROR			52226
#define MSO_BB_OLD_PLAN_ERROR_DESCR		"Provisioning fail, old plan not found for the account"
#define MSO_BB_CONFIG_PT_ERROR			52227
#define MSO_BB_CONFIG_PT_ERROR_DESCR		"Provisioning fail, mso_config_bb_pt cache not found."
#define MSO_BB_ACCOUNTNO_ERROR			52228
#define MSO_BB_ACCOUNTNO_ERROR_DESCR		"Provisioning fail, account_no not found"
#define MSO_BB_FIRSTNAME_ERROR			52229
#define MSO_BB_FIRSTNAME_ERROR_DESCR		"Provisioning fail, last name not found in account"
#define MSO_BB_LASTNAME_ERROR			52229
#define MSO_BB_LASTNAME_ERROR_DESCR		"Provisioning fail, last name not found in account"
#define MSO_BB_CLIENT_CLASS_ERROR		52230
#define MSO_BB_CLIENT_CLASS_ERROR_DESCR		"Provisioning fail, Client class not found"
#define MSO_BB_PROV_TAG_ERROR			52231
#define MSO_BB_PROV_TAG_ERROR_DESCR		"Provisioning fail, provisioning tag not found"
#define MSO_BB_OLD_LOGIN_MISSING		52232
#define MSO_BB_OLD_LOGIN_MISSING_DESCR		"Provisioning fail, Old Login missing in input"
#define MSO_BB_SAME_LOGIN_ERROR			52233
#define MSO_BB_SAME_LOGIN_ERROR_DESCR		"Provisioning fail, new login is same as old login"
#define MSO_BB_SAME_IP_ERROR			52234
#define MSO_BB_SAME_IP_ERROR_DESCR		"Provisioning fail, new IP is same as old IP"
#define MSO_BB_OLDPASS_ERROR			52235
#define MSO_BB_OLDPASS_ERROR_DESCR		"Provisioning fail, old password missing in input"
#define MSO_BB_SAME_PASS_ERROR			52236
#define MSO_BB_SAME_PASS_ERROR_DESCR		"Provisioning fail, new password is same as old password"
#define MSO_BB_CMTS_MASTER_ERROR		52237
#define MSO_BB_CMTS_MASTER_ERROR_DESCR		"Provisioning fail CMTS Master cache not found"
#define MSO_BB_CMTS_DEVICE_CLASS_ERROR		52238
#define MSO_BB_CMTS_DEVICE_CLASS_ERROR_DESCR	"Provisioning fail, CMTS Client Class not found"
#define MSO_BB_SPEED_ERROR			52239
#define MSO_BB_SPEED_ERROR_DESCR		"Provisioning fail, Speed Client Class not found"
#define MSO_BB_ULSPEED_ERROR			52239
#define MSO_BB_ULSPEED_ERROR_DESCR		"Provisioning fail, Upload Speed not found"
#define MSO_BB_DLSPEED_ERROR			52240
#define MSO_BB_DLSPEED_ERROR_DESCR		"Provisioning fail, Download Speed not found"
#define MSO_BB_TAL_ERROR			52241
#define MSO_BB_TAL_ERROR_DESCR			"Provisioning fail, Tal/DHCP class not found"
#define MSO_BB_COMPANY_MISSING			52242
#define MSO_BB_COMPANY_MISSING_DESCR		"Provisioning fail, Company missing for account"
#define MSO_BB_ADDRESS_MISSING			52243
#define MSO_BB_ADDRESS_MISSING_DESCR		"Provisioning fail, Address missing for account"
#define MSO_BB_IP_ADD_ERROR			52244
#define MSO_BB_IP_ADD_ERROR_DESCR		"Provisioning fail, No IP Address found for account"
#define MSO_BB_MAC_ADD_ERROR			52245
#define MSO_BB_MAC_ADD_ERROR_DESCR		"Provisioning fail, No MAC Address found for account"
#define MSO_BB_ID_ERROR				52246
#define MSO_BB_ID_ERROR_DESCR			"Provisioning fail, No ASAP ID found for the service"
#define MSO_BB_CITY_MISSING			52247
#define MSO_BB_CITY_MISSING_DESCR		"Provisioning fail, City is missing in the service"
#define MSO_BB_AGG_MISSING			52248
#define MSO_BB_AGG_MISSING_DESCR		"Provisioning fail, Aggreement No is missing in the service"
#define MSO_BB_CC_ID_ERROR			52249
#define MSO_BB_CC_ID_ERROR_DESCR		"Provisioning fail, Client Class ID missing for CMTS"
#define MSO_BB_CC_CONFIG_ERROR			52250
#define MSO_BB_CC_CONFIG_ERROR_DESCR		"Provisioning fail, CMTS Mapping config is invalid for the CMTS ID"
#define MSO_BB_TAL_AC_ERROR			52251
#define MSO_BB_TAL_AC_ERROR_DESCR			"Provisioning fail, this action is not valid for NON-TAL customer"
#define MSO_BB_NTAL_ERROR			52252
#define MSO_BB_NTAL_ERROR_DESCR			"Provisioning fail, this action is not valid for TAL customer"
#define MSO_BB_INV_TECH				52253
#define MSO_BB_INV_TECH_DESCR			"Provisioning fail, this technology is not provisionable"
#define MSO_BB_IP_DEVICE_ERROR			52254
#define MSO_BB_IP_DEVICE_ERROR_DESCR		"Provisioning fail, no static IP device found for TAL customer"
/**** Pavan Bellala 29-07-2015 ***********************************
 Defect 1136 : Validate device string pattern before provisioning
*****************************************************************/
#define MSO_BB_INVALID_MAC			52255
#define MSO_BB_INVALID_MAC_DESCR		"Provisioning fail,Invalid MAC ID"
#define MSO_BB_RESET_DEVICE_ERROR		52256
#define MSO_BB_RESET_DEVICE_ERROR_DESCR		"Reset device error"
#define MSO_BB_CONFIG_PT_TECH_ERROR             52257
#define MSO_BB_CONFIG_PT_TECH_ERROR_DESCR       "Provisioning fail, Mismatch tech in config"
// This is for MODEM validation in provisioning - No activation should be allowed
#define MSO_BB_NO_MODEM_FOUND			52258
#define MSO_BB_NO_MODEM_FOUND_ERROR_DESCR       "No modem device found for this account. Please allocate hardware"


//For Device Lifecycle Mgmt
#define MSO_DEV_FLAG_ERR			52500
#define MSO_DEV_FLAG_ERR_DESCR			"Flags missing in input"
#define MSO_DEV_INTERNAL_ERR			52501
#define MSO_DEV_INTERNAL_ERR_DESCR		"Internal error, please check logs"
#define MSO_DEV_MANDATORY_ERR			52502
#define MSO_DEV_MANDATORY_ERR_DESCR		"Mandatory field missing"

//For Acc Mgmt
#define CUST_INTERNAL_ERROR                     51201
#define CUST_INTERNAL_ERROR_DESCR               "Operation fialed, due to internal error, check logs"
#define CUST_PROGRAM_NAME                       51202
#define CUST_PROGRAM_NAME_DESCR                 "Operation failed, as the program name is not passed"
#define CUST_DEVICE_ID                          51203
#define CUST_DEVICE_ID_DESCR                    "Operation failed, as device id is missing"
#define CUST_POID_ID                            51204
#define CUST_POID_ID_DESCR                      "Operation failed, as POID is missing"
#define CUST_NO_BB_DEVICE                       51205
#define CUST_NO_BB_DEVICE_DESCR                 "Operation failed, no device object found for the device id"
#define CUST_BB_DEVICE_STATE                    51206
#define CUST_BB_DEVICE_STATE_DESCR              "Operation failed, as the device is still in allocated state"



#define CUST_BB_DEACTIVATE_INV_STATUS		"51600"
#define CUST_BB_DEACTIVATE_ALREADY_IN_PROGRESS	"51601"
#define CUST_BB_DEACTIVATE_HAVE_SUBSCRIPTION    "51602"


#define LEGACY_ACCOUNT_NUMBER_MISSING			51701
#define LEGACY_ACCOUNT_NUMBER_MISSING_DESCR		"Legacy number is missing or passed as null"	

//For Commission
#define PIN_ERRLOC_FM_COMM MSO_OP_COMMISSION_PROCESS_COMMISSION

//Rating Error Codes
#define RATING_ERR_BEF_RAT     			"50001"
#define RATING_ERR_BEF_RAT_DESC    		"Error before Rating"

#define RATING_ERR_DUP     			"50002"
#define RATING_ERR_DUP_DESC    			"Duplicate Record"

#define RATING_ERR_HIGH_QTY     		"50003"
#define RATING_ERR_HIGH_QTY_DESC    		"Higher Qty Already Rated"

#define RATING_ERR_VALIDATE     		"50004"
#define RATING_ERR_VALIDATE_DESC    		"Error before Rating"

#define RATING_ERR_NO_SERV    			"50005"
#define RATING_ERR_NO_SERV_DESC    		"Service Not Found"

#define RATING_ERR_TRANS    			"50006"
#define RATING_ERR_TRANS_DESC    		"Error in Opening Transactions"

#define RATING_ERR_GET_DATE    			"50007"
#define RATING_ERR_GET_DATE_DESC    		"Error in getting charge dates"

#define RATING_ERR_SPLIT_RAT    		"50008"
#define RATING_ERR_SPLIT_RAT_DESC    		"Split Rating Failed"

#define RATING_ERR_FAIL_RATING    		"50009"
#define RATING_ERR_FAIL_RATING_DESC    		"Rating Failed"

#define RATING_ERR_USG_SESS    			"50010"
#define RATING_ERR_USG_SESS_DESC    		"Error Updating Usage Session"

#define RATING_ERR_DAILY_USG    		"50011"
#define RATING_ERR_DAILY_USG_DESC    		"Error in Updating Daily Usage"

#define RATING_ERR_THRESHOLD    		"50012"
#define RATING_ERR_THRESHOLD_DESC    		"Error in Threshold Check"

#define RATING_SUCCESS    			"00"
#define RATING_SUCCESS_DESC    			"Success"

//Hybrid Accounts Tagging Errors
#define HYBRID_ACCTS_MANDFLDS_MISS   		"92001"
#define HYBRID_ACCTS_MANDFLDS_MISS_DESC    	"Mandatory Inputs are not passed in Input Flist !"

#define HYBRID_TAG_ERR		   		"92002"
#define HYBRID_TAG_ERR_DESC    			"Error in Hybrid Account Tagging !"

#define HYBRID_DTAG_ERR		   		"92003"
#define HYBRID_DTAG_ERR_DESC    		"Error in Hybrid Account De-Tagging !"

#define HYBRID_RETAG_ERR		   	"92004"
#define HYBRID_RETAG_ERR_DESC    		"Error in Hybrid Account Re-Tagging !"

#define HYBRID_ACCTS_TAG_SUCCESS_DESC 		"Tagging of Hybrid accounts is Successful !"

#define HYBRID_TAG_MANDFLDS_INVAL		"92005"
#define HYBRID_TAG_MANDFLDS_INVAL_DESC 		"Mandatory inputs Main Account No / Tagging Account No missing or Invalid !"

#define HYBRID_TAG_ERR_1			"92006"
#define HYBRID_TAG_ERR_1_DESC 			"Main Account and Tagging Account Numbers cannot be the same !"

#define HYBRID_TAG_ERR_2			"92007"
#define HYBRID_TAG_ERR_2_DESC 			"Main Account Number not found !"

#define HYBRID_TAG_ERR_3			"92008"
#define HYBRID_TAG_ERR_3_DESC			"Business Type not found for the Main account !"

#define HYBRID_TAG_ERR_4			"92009"
#define HYBRID_TAG_ERR_4_DESC			"Bill Info ID not found for the Main account !"

#define HYBRID_TAG_ERR_5			"92010"
#define HYBRID_TAG_ERR_5_DESC			"Tagging account number not found !"

#define HYBRID_TAG_ERR_6			"92011"
#define HYBRID_TAG_ERR_6_DESC			"Business Type not found for the Tagging Account !"

#define HYBRID_TAG_ERR_7			"92012"
#define HYBRID_TAG_ERR_7_DESC			"Bill Info ID not found for the Tagging account !"

#define HYBRID_TAG_ERR_8			"92013"
#define HYBRID_TAG_ERR_8_DESC			"Main Account is already tagged to another account !"

#define HYBRID_TAG_ERR_9			"92014"
#define HYBRID_TAG_ERR_9_DESC			"Tagging Account is already tagged to another account !"

#define HYBRID_TAG_ERR_10			"92015"
#define HYBRID_TAG_ERR_10_DESC			"Main and Tagging accounts cannot have the same Bill Info ID !"

#define HYBRID_TAG_ERR_11			"92016"
#define HYBRID_TAG_ERR_11_DESC			"Main and Tagging accounts cannot have the same Business Type !"

#define HYBRID_TAG_ERR_12			"92017"
#define HYBRID_TAG_ERR_12_DESC			"Invalid Account Type for Main Account !"

#define HYBRID_TAG_ERR_13			"92018"
#define HYBRID_TAG_ERR_13_DESC			"Invalid Account Type for Tagging Account !"

#define HYBRID_TAG_ERR_14			"92019"
#define HYBRID_TAG_ERR_14_DESC			"Main and Tagging Accounts have the same Account Type !"

#define HYBRID_TAG_ERR_15			"92020"
#define HYBRID_TAG_ERR_15_DESC			"Error in updating Main Account !"

#define HYBRID_TAG_ERR_16			"92021"
#define HYBRID_TAG_ERR_16_DESC			"Error in updating Tagging Account !"

#define HYBRID_TAG_ERR_17			"92022"
#define HYBRID_TAG_ERR_17_DESC			"Error in updating Scheme Name in CATV Service Object !"

#define HYBRID_TAG_ERR_18			"92023"
#define HYBRID_TAG_ERR_18_DESC			"Device/STB not found !"

#define HYBRID_TAG_ERR_19			"92024"
#define HYBRID_TAG_ERR_19_DESC			"Life Cycle event creation for Tagging Hybrid Accounts failed !"

#define HYBRID_TAG_ERR_20			"92025"
#define HYBRID_TAG_ERR_20_DESC			"Tagging Account is already tagged to the Main Account !"

#define HYBRID_TAG_ERR_21			"92026"
#define HYBRID_TAG_ERR_21_DESC			"Main Account is already tagged to the same Tagging Account !"

#define HYBRID_ACCTS_DETAG_SUCCESS_DESC 	"De-Tagging of Hybrid accounts is Successful !"

#define HYBRID_DETAG_MANDFLDS_INVAL		"92027"
#define HYBRID_DETAG_MANDFLDS_INVAL_DESC 	"Mandatory inputs Main Account No / Tagging Account No missing or Invalid !"

#define HYBRID_DETAG_ERR_1			"92028"
#define HYBRID_DETAG_ERR_1_DESC 		"Main account number and De-Tagging Account Number cannot be the same !"

#define HYBRID_DETAG_ERR_2			"92029"
#define HYBRID_DETAG_ERR_2_DESC			"De-Tagging Account Number not found !"

#define HYBRID_DETAG_ERR_3			"92030"
#define HYBRID_DETAG_ERR_3_DESC			"Main Account Object Write Flds Failed !"

#define HYBRID_DETAG_ERR_4			"92031"
#define HYBRID_DETAG_ERR_4_DESC			"De-Tagging Account Object Write Flds Failed !"

#define HYBRID_DETAG_ERR_5			"92032"
#define HYBRID_DETAG_ERR_5_DESC			"Error in updating Scheme Name in the De-Tagging Account Service Object !"

#define HYBRID_DETAG_ERR_6			"92033"
#define HYBRID_DETAG_ERR_6_DESC			"Life Cycle event creation for De-Tagging Hybrid Accounts failed !"

#define HYBRID_DETAG_ERR_7			"92034"
#define HYBRID_DETAG_ERR_7_DESC			"Accounts are already De-tagged !"

#define HYBRID_DETAG_ERR_8			"92035"
#define HYBRID_DETAG_ERR_8_DESC			"Tagging Account is not tagged to Main Account !"

#define HYBRID_DETAG_ERR_9			"92036"
#define HYBRID_DETAG_ERR_9_DESC			"Main Account is not tagged to the given Tagging Account !"

#define HYBRID_ACCTS_RETAG_SUCCESS_DESC 	"Re-tagging Hybrid Accounts Successful !"

#define HYBRID_RETAG_MANDFLDS_INVAL		"92037"
#define HYBRID_RETAG_MANDFLDS_INVAL_DESC 	"Mandatory parameters Old Tagged account / New Account to be detagged not found missing or Invalid !"

#define HYBRID_RETAG_ERR_1			"92038"
#define HYBRID_RETAG_ERR_1_DESC 		"Old Account to be Detagged No and New Account to be tagged are same !"

#define HYBRID_RETAG_ERR_2			"92039"
#define HYBRID_RETAG_ERR_2_DESC 		"Detagging Account Number not found !"

#define HYBRID_RETAG_ERR_3			"92040"
#define HYBRID_RETAG_ERR_3_DESC			"Business Type not found for the Detagging account !"

#define HYBRID_RETAG_ERR_4			"92041"
#define HYBRID_RETAG_ERR_4_DESC			"Bill Info ID not found for the Main Account !"

#define HYBRID_RETAG_ERR_5			"92042"
#define HYBRID_RETAG_ERR_5_DESC			"Retagging account number not found !"

#define HYBRID_RETAG_ERR_6			"92043"
#define HYBRID_RETAG_ERR_6_DESC			"Business Type not found for the Retagging Account !"

#define HYBRID_RETAG_ERR_7			"92044"
#define HYBRID_RETAG_ERR_7_DESC			"Bill Info ID not found for the Retagging account !"

#define HYBRID_RETAG_ERR_8			"92045"
#define HYBRID_RETAG_ERR_8_DESC			"Retagging Account is already tagged to another Account !"

#define HYBRID_RETAG_ERR_9			"92046"
#define HYBRID_RETAG_ERR_9_DESC			"Retagging and Detagging Accounts do not have the same Bill Info ID !"

#define HYBRID_RETAG_ERR_10			"92047"
#define HYBRID_RETAG_ERR_10_DESC		"Retagging and Detagging Accounts do not have the same Business Type !"

#define HYBRID_RETAG_ERR_11			"92048"
#define HYBRID_RETAG_ERR_11_DESC		"Invalid Account Type for Retagging Account !"

#define HYBRID_RETAG_ERR_12			"92049"
#define HYBRID_RETAG_ERR_12_DESC		"Invalid Account Type for Detagging Account !"

#define HYBRID_RETAG_ERR_13			"92050"
#define HYBRID_RETAG_ERR_13_DESC		"Retagging and Detagging Accounts do not have the same Account Type !"

#define HYBRID_RETAG_ERR_14			"92051"
#define HYBRID_RETAG_ERR_14_DESC		"De-Tagging Hybrid Accounts failed !"

#define HYBRID_RETAG_ERR_15			"92052"
#define HYBRID_RETAG_ERR_15_DESC		"Re-Tagging Hybrid Accounts failed !"

#define HYBRID_RETAG_ERR_16			"92053"
#define HYBRID_RETAG_ERR_16_DESC		"Account Poids for Old Tagged Account and New Tagging Account not found !"

#define HYBRID_RETAG_ERR_17			"92054"
#define HYBRID_RETAG_ERR_17_DESC		"De-Tagging Account is not tagged to any Main Account !"

//Termination Reasons Loading / Fetch Errors
#define TERM_REASONS_LOAD_SUCCESS_DESC 			"Termination Reason details loading is Successful !"

#define TERM_REASONS_LOAD_MANDFLDS_MISS   		"95001"
#define TERM_REASONS_LOAD_MANDFLDS_MISS_DESC    "Mandatory Inputs are not passed in Input Flist !"

#define TERM_REASONS_LOAD_ERR		   			"95002"
#define TERM_REASONS_LOAD_ERR_DESC    			"Error in loading termination reasons !"

#define TERM_REASONS_LOAD_ERR_1		   			"95003"
#define TERM_REASONS_LOAD_ERR_1_DESC    		"Type and SubType values are Invalid !"

#define TERM_REASONS_LOAD_ERR_2		   			"95004"
#define TERM_REASONS_LOAD_ERR_2_DESC    		"Reason Sub Type already exists for this Reason Type !"

#define TERM_REASONS_LOAD_ERR_3		   			"95005"
#define TERM_REASONS_LOAD_ERR_3_DESC    		"Error in Updating Reason Type Object !"

#define TERM_REASONS_LOAD_ERR_4		   			"95006"
#define TERM_REASONS_LOAD_ERR_4_DESC    		"Error in Creating Reason Type Object !"

#define TERM_REASONS_LOAD_ERR_5		   			"95007"
#define TERM_REASONS_LOAD_ERR_5_DESC    		"Error in fetching the Reason Type SubTypes details !"

#define TERM_REASONS_LOAD_SUCCESS_MSG		   	"Termination Reason Type and Subtype load Successful !"

#define TERM_REASONS_GET_ERR		   			"95008"
#define TERM_REASONS_GET_ERR_DESC    			"Error in fetching termination reasons !"

#define TERM_REASONS_DELETE_SUCCESS_MSG			"Termination Reason Type and Subtype delete Successful !"

#define TERM_REASONS_DELETE_ERR_1                                 "95009"
#define TERM_REASONS_DELETE_ERR_1_DESC                    "Reason type not exists !"

#define TERM_REASONS_DELETE_ERR_2                                 "95010"
#define TERM_REASONS_DELETE_ERR_2_DESC                    "Reason Sub Type not exists for this Reason Type !"

#define TERM_REASONS_DELETE_ERR_3                               "95011"
#define TERM_REASONS_DELETE_ERR_3_DESC                  "Error in deleting Reason Type Object !"

#define TERM_REASONS_EDIT_SUCCESS_MSG                 "Termination Reason Type and Subtype update Successful !"

#define TERM_REASONS_EDIT_ERR_1                               "95012"
#define TERM_REASONS_EDIT_ERR_1_DESC                  "Error in updating Reason Subtype !"
#endif


