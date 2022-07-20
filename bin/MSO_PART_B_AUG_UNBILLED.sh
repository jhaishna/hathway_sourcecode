#!/bin/sh
#source .bash_profile
user="$1"
pass="$2"
SID="$3"
#location="$4"
#rm $location/execution_logs
sqlplus -S $user/$pass@$SID <<EOF
alter session force parallel dml parallel 16;
INSERT /*APPEND parallel(tt,16)*/
INTO TMP_PARTB_JUL_AUG21_1 tt
SELECT CUSTOMER_ID,
  AGREEMENT_NO,
  BILL_NO,
  NAME,
  ENTITY_CODE,
  ENTITY_NAME,
  DECODER,
  SMARTCARD,
  ADDRESS,
  PRD_NAME,
  NVL(CHARGES,0),
  NVL(ST,0),
  CHARGES + ST,
  CITY,
  CHARGE_START_DATE,
  CHARGE_END_DATE,
  --BILLING_DUE_DATE,
  --BILL_CREATION_DATE,
  PAYTERM,
  EVENT_PDP,
  SESSION_OBJ_ID0,
  BILLINFO_PDP,
  ITEM_PDP,
  --LAST_BILL_T
  'UNBILLED' TYPE
FROM
  (SELECT
    /*+ parallel (a,16) full(a) parallel (an,16) full(an) parallel (pro,16) full(pro) parallel (pcc,16) full(pcc)
    parallel (a1,16) full(a1) parallel (an1,16) full(an1) (b,16) full(b)
    parallel (i,16) full(i) parallel (ebi,16) full(ebi) parallel (e,16) full(e) parallel (p,16) full(p) */
    a.ACCOUNT_NO CUSTOMER_ID,
    (SELECT agreement_no
    FROM SERVICE_CATV_INFO_T
    WHERE obj_id0 = i.SERVICE_OBJ_ID0
    ) AGREEMENT_NO,
    b.BILL_NO,
    an.first_name
    ||' '
    ||an.MIDDLE_NAME
    ||' '
    ||an.last_name NAME,
    a1.account_no ENTITY_CODE,
    an1.company ENTITY_NAME,
    (SELECT name
    FROM SERVICE_ALIAS_LIST_t
    WHERE obj_id0=i.SERVICE_OBJ_ID0
    AND rec_id  IN (0,2)
    ) DECODER,
    (SELECT name
    FROM SERVICE_ALIAS_LIST_t
    WHERE obj_id0=i.SERVICE_OBJ_ID0
    AND rec_id   = 1
    ) SMARTCARD,
    an.address ADDRESS,
    p.name PRD_NAME,
    NVL(amount,0) CHARGES,
    (SELECT SUM(NVL(amount,0))
    FROM EVENT_BAL_IMPACTS_T
    WHERE obj_id0  = ebi.obj_id0
    AND impact_type=4
    ) ST,
    an.city CITY,
    inf2ora(e.EARNED_START_T) CHARGE_START_DATE,
    inf2ora(e.EARNED_END_T) CHARGE_END_DATE,
    --INF2ORA(B.DUE_T) BILLING_DUE_DATE,
    --inf2ora(iv.created_t) BILL_CREATION_DATE,
    bi.bill_when
    ||' Month(s)' PAYTERM,
    e.poid_id0 EVENT_PDP,
    e.session_obj_id0 SESSION_OBJ_ID0,
    bi.poid_id0 BILLINFO_PDP,
    I.POID_ID0 ITEM_PDP,
    --inf2ora(bi.LAST_BILL_T) LAST_BILL_T
    'BILLED' TYPE
  FROM account_t a,
    account_nameinfo_t an,
    profile_t pro,
    profile_customer_care_t pcc,
    account_t a1,
    ACCOUNT_NAMEINFO_T AN1,
    --invoice_t iv,
    item_t i,
    EVENT_BAL_IMPACTS_T ebi,
    event_t e,
    product_t p,
    bill_t b,
    BILLINFO_T BI
  WHERE
    /*i.bill_obj_id0 = iv.bill_obj_id0
    AND b.poid_id0 = iv.bill_obj_id0
    AND*/
    bi.bill_obj_id0      =b.poid_id0
  AND i.bill_obj_id0     =b.poid_id0
  AND i.poid_type NOT   IN ('/item/cycle_arrear/mso_et','/item/cycle_forward/mso_et')
  AND e.ITEM_OBJ_ID0     = i.poid_id0
  AND ebi.OBJ_ID0        = e.poid_id0
  AND ebi.IMPACT_TYPE    =1
  AND e.service_obj_id0  =i.service_obj_id0
  AND p.poid_id0         =ebi.product_obj_id0
  and a.poid_id0=i.ACCOUNT_OBJ_ID0
  AND an.obj_id0         =a.poid_id0
  AND pro.account_obj_id0=a.poid_id0
  AND pcc.obj_id0        =pro.poid_id0
  AND an.rec_id          =1
  AND an1.rec_id         =1
  AND a1.poid_id0        =pcc.lco_obj_id0
  AND AN1.OBJ_ID0        =A1.POID_ID0
    --AND IV.ACCOUNT_OBJ_ID0 = A.POID_ID0
    --AND inf2ora(iv.created_t) BETWEEN start_dt AND end_dt
    -- AND B.DUE_T >=ORA2INF('16-JUL-2014')
    --AND B.DUE_T <=ORA2INF('31-JUL-2014')
    --AND iv.BILL_NO IS NOT NULL
    --AND iv.POID_TYPE='/invoice'
    --AND bi.poid_id0 =iv.billinfo_obj_id0
  AND BI.NEXT_BILL_T >= ORA2INF('01-AUG-2014')
  AND BI.NEXT_BILL_T  < ORA2INF('16-AUG-2014')
  );
COMMIT;
exit;
EOF
