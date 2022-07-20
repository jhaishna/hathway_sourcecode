/*
* Notification action flags
*/
#define NTF_BILLING			1
#define NTF_DUE_REMINDER		2
#define NTF_PAYMENT			3
#define NTF_PAYMENT_REVERSAL		4
#define NTF_DEPOSIT_REFUND		5
#define NTF_SERVICE_ACTIVATION		6
#define NTF_SERVICE_SUSPENSION		7
#define NTF_SERVICE_REACTIVATION	8
#define NTF_SERVICE_TERMINATON		9
#define NTF_ADD_PLAN			10
#define NTF_CHANGE_PLAN			11
#define NTF_CANCEL_PLAN			12
#define NTF_REMINDER_POST_DUE_DATE		    13
#define NTF_WELCOME_NOTE        14
#define NTF_CHEQUE_REVERSAL           25
#define NTF_SERVICE_SWAP        32

//BB Notfications
#define NTF_BB_DUE_REMINDER 15
#define NTF_BB_AFTER_DUE 16
#define NTF_BEFORE_FUP 17
#define NTF_AFTER_FUP 18
#define NTF_BEFORE_EXPIRY 19
#define NTF_BB_ON_EXPIRY 20
#define NTF_BB_AFTER_EXPIRY 21
#define NTF_BB_CHANGE_PASSWORD 22
#define NTF_BB_DATA_LIMIT 23
#define NTF_BB_DATA_LIMIT_EXPIRED 24
#define NTF_CHEQUE_REVERSAL           25
#define NTF_BB_PAYMENT_CASH 26
#define NTF_BB_PAYMENT_CHEQUE 27
#define NTF_BB_PREPAID_LIM_USAGE 28
#define NTF_BB_UPGRADE_PLAN 		29
#define NTF_BB_PAYMENT_ONLINE	30
#define NTF_EXTRA_GB_TOPUP 31


#define NTF_STATE_NEW   1

/*
* Provisioning statuses
*/
#define NTF_STATE_NEW                  1
#define NTF_STATE_FAILED               2
#define NTF_STATE_SUCCESS              0


/*
* Bmail
*/
#define BMAIL_SUBJECT 			"Welcome to Hathway"