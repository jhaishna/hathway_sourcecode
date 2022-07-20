/****************************************************************
 * customer suppression reasons in case of bill suppression.
 ***************************************************************/
typedef enum mso_bill_suppression_reasons {
        MSO_CREDIT_DEBIT_NOTE_PENDING =  10,
        MSO_WRONG_PLAN_ACTIVATION =      11,
        MSO_WRONG_TOP_UP =               12,
        MSO_WRONG_PLAN_CHANGE =          13,
        MSO_CUSTOMER_REQUEST =           14,
        MSO_VIP_CUSTOMER =               15,
        MSO_ADDRESS_CHANGE =             16
} mso_bill_suppression_reasons_t;
