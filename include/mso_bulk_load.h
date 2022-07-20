#define BULK_DEVICE_LOAD	1
#define BULK_DEVICE_LOCATION_UPDATER	2
#define BULK_DEVICE_STATE_UPDATE	3
#define BULK_DEVICE_CATV_PREACTIVATION	4
#define BULK_ADD_PLAN	5
#define BULK_CHANGE_PLAN	6
#define BULK_CANCEL_PLAN	7
#define BULK_SERVICE_SUSPENSION	8
#define BULK_SERVICE_REACTIVATION	9
#define BULK_SERVICE_TERMINATION	10
#define BULK_ADJUSTMENT		11
#define BULK_RETRACK		12
#define BULK_OSD		13
#define BULK_BMAIL		14
#define BULK_CHANGE_BOUQUET_ID		15
#define BULK_SET_PERSONAL_BIT		16
#define BULK_CRF		17
#define BULK_HIERARCHY		18
#define BULK_IPPOOL_UPDATER		19
#define BULK_TOPUP 		20
#define BULK_RENEW		21
#define BULK_EXTEND_VALIDITY	22
#define BULK_ADDITION_MB_GB	23
#define BULK_WRITEOFF_CPE	24
#define BULK_SWAPPING_CPE	25
#define BULK_CHECK_BOUNCE	26
#define BULK_BILL_HOLD		27
#define BULK_REFUND_LOAD	28
#define BULK_REFUND_REV_LOAD	29
#define BULK_CR_DR_NOTE	31
#define BULK_UPLOAD_GRV	32
#define BULK_DEVICE_ATTR_UPDATER	33
#define BULK_CMTS_CHANGE	34
#define BULK_CREATE_ACCOUNT 35
#define BULK_UPDATE_GST_INFO 37
#define BULK_MODIFY_GST_INFO 38
#define BULK_STATIC_IP_WAIVE_OFF 39
#define BULK_DEVICE_PAIR 40



#define OPCODE_BULK_TOPUP              MSO_OP_CUST_TOP_UP_BB_PLAN
#define OPCODE_BULK_RENEW              MSO_OP_CUST_RENEW_BB_PLAN
#define OPCODE_BULK_EXTEND_VALIDITY    MSO_OP_CUST_EXTEND_VALIDITY
#define OPCODE_BULK_ADDITION_MB_GB     MSO_OP_CUST_ADD_PLAN
#define OPCODE_BULK_WRITEOFF_CPE       MSO_OP_CUST_DELETE_BB_DEVICE
#define OPCODE_BULK_SWAPPING_CPE       MSO_OP_CUST_SWAP_BB_DEVICE
#define OPCODE_BULK_CHECK_BOUNCE       MSO_OP_PYMT_REVERSE_PAYMENT
#define OPCODE_BULK_BILL_HOLD          MSO_OP_CUST_MODIFY_CUSTOMER
#define OPCODE_BULK_REFUND_LOAD        MSO_OP_PYMT_PROCESS_REFUND
#define OPCODE_BULK_REFUND_REV_LOAD    MSO_OP_PYMT_REVERSE_REFUND
#define OPCODE_BULK_CMTS_CHANGE        MSO_OP_CUST_MODIFY_CUSTOMER

#define TOPUP              "topup"
#define RENEW              "renew"
#define EXTEND_VALIDITY    "extend_validitiy"
#define ADDITION_MB_GB     "add_mb_gb"
#define WRITEOFF_CPE       "writeoff_cpe"
#define SWAPPING_CPE       "swapping_cpe"
#define CHECK_BOUNCE       "check_bounce"
#define BILL_HOLD          "bill_hold"
#define REFUND_LOAD        "refund_load"
#define REFUND_REV_LOAD    "refund_rev_load"
#define CMTS_CHANGE	   "cmts_change"

#define BULK_TOPUP_DESCR             "Bulk TOPUP"
#define BULK_RENEW_DESCR             "Bulk RENEW"
#define BULK_EXTEND_VALIDITY_DESCR   "Bulk EXTEND VALIDITY"
#define BULK_ADDITION_MB_GB_DESCR    "Bulk ADD MB_GB"
#define BULK_WRITEOFF_CPE_DESCR      "Bulk WRITEOFF CPE"
#define BULK_SWAPPING_CPE_DESCR      "Bulk SWAPPING CPE"
#define BULK_CHECK_BOUNCE_DESCR      "Bulk CHECK BOUNCE"
#define BULK_BILL_HOLD_DESCR         "Bulk BILL HOLD"
#define BULK_REFUND_LOAD_DESCR       "Bulk REFUND LOAD"
#define BULK_REFUND_REV_LOAD_DESCR   "Bulk REFUND REV LOAD"
#define BULK_CMTS_CHANGE_DESCR       "Bulk CMTS CHANGE"

#define BULK_TOPUP_ORDER_TYPE             "/mso_order/bulk_topup"
#define BULK_RENEW_ORDER_TYPE             "/mso_order/bulk_renew"
#define BULK_EXTEND_VALIDITY_ORDER_TYPE   "/mso_order/bulk_extend_validity"
#define BULK_ADDITION_MB_GB_ORDER_TYPE    "/mso_order/bulk_add_mb_gb"
#define BULK_WRITEOFF_CPE_ORDER_TYPE      "/mso_order/bulk_writeoff_cpe"
#define BULK_SWAPPING_CPE_ORDER_TYPE      "/mso_order/bulk_swap_cpe"
#define BULK_CHECK_BOUNCE_ORDER_TYPE      "/mso_order/bulk_check_bounce"
#define BULK_BILL_HOLD_ORDER_TYPE         "/mso_order/bulk_bill_hold"
#define BULK_REFUND_LOAD_ORDER_TYPE       "/mso_order/bulk_refund_load"
#define BULK_REFUND_REV_LOAD_ORDER_TYPE   "/mso_order/bulk_refund_rev_load"
#define BULK_CMTS_CHANGE_ORDER_TYPE       "/mso_order/bulk_cmts_change"
#define BULK_CREATE_ACCT_ORDER_TYPE       "/mso_order/bulk_create_account"

#define BULK_TOPUP_TASK_TYPE             "/mso_task/bulk_topup"
#define BULK_RENEW_TASK_TYPE             "/mso_task/bulk_renew"
#define BULK_EXTEND_VALIDITY_TASK_TYPE   "/mso_task/bulk_extend_validity"
#define BULK_ADDITION_MB_GB_TASK_TYPE    "/mso_task/bulk_add_mb_gb"
#define BULK_WRITEOFF_CPE_TASK_TYPE      "/mso_task/bulk_writeoff_cpe"
#define BULK_SWAPPING_CPE_TASK_TYPE      "/mso_task/bulk_swap_cpe"
#define BULK_CHECK_BOUNCE_TASK_TYPE      "/mso_task/bulk_check_bounce"
#define BULK_BILL_HOLD_TASK_TYPE         "/mso_task/bulk_bill_hold"
#define BULK_REFUND_LOAD_TASK_TYPE       "/mso_task/bulk_refund_load"
#define BULK_REFUND_REV_LOAD_TASK_TYPE   "/mso_task/bulk_refund_rev_load"
#define BULK_CMTS_CHANGE_TASK_TYPE       "/mso_task/bulk_cmts_change"
#define BULK_CREATE_ACCT_TASK_TYPE       "/mso_task/bulk_create_account"

#define BULK_TOPUP_JOB_TYPE             "/mso_mta_job/bulk_topup"
#define BULK_RENEW_JOB_TYPE             "/mso_mta_job/bulk_renew"
#define BULK_EXTEND_VALIDITY_JOB_TYPE   "/mso_mta_job/bulk_extend_validity"
#define BULK_ADDITION_MB_GB_JOB_TYPE    "/mso_mta_job/bulk_add_mb_gb"
#define BULK_WRITEOFF_CPE_JOB_TYPE      "/mso_mta_job/bulk_writeoff_cpe"
#define BULK_SWAPPING_CPE_JOB_TYPE      "/mso_mta_job/bulk_swap_cpe"
#define BULK_CHECK_BOUNCE_JOB_TYPE      "/mso_mta_job/bulk_check_bounce"
#define BULK_BILL_HOLD_JOB_TYPE         "/mso_mta_job/bulk_bill_hold"
#define BULK_REFUND_LOAD_JOB_TYPE       "/mso_mta_job/bulk_refund_load"
#define BULK_REFUND_REV_LOAD_JOB_TYPE   "/mso_mta_job/bulk_refund_rev_load"
#define BULK_CMTS_CHANGE_JOB_TYPE       "/mso_mta_job/bulk_cmts_change"
#define BULK_CREATE_ACCT_JOB_TYPE       "/mso_mta_job/bulk_create_account"

