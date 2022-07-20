#!/bin/sh
#source .bash_profile
user="$1"
pass="$2"
SID="$3"
#rm $location/execution_logs
sqlplus -S $user/$pass@$SID <<EOF
create table temp_dup_charges_01AUG_ORIG as (
SELECT account_number account_no,
  service_obj_id0 service_obj_id0,
  product_obj_id0 product_obj_id0,
  bill_no bill_no,
  MIN(earned_start_t) earned_start_t,
  MAX(earned_end_T) earned_end_t,
  COUNT(product_obj_id0) CNT,
  tbl2.product_name product_name,
  SUM(amount) amount,
  tbl2.scaled_amount Scaled_amnt,
  floor((tbl2.scaled_amount * (months_between(MAX(earned_end_T),MIN(earned_start_t))))-SUM(amount)) ADJ_amnt  ,
  max(tbl1.mod_t) mod_t
FROM
  (SELECT
    /*+ parallel (E,8) parallel(EB,8) parallel(I,8) parallel(A1,8) */
    A1.account_no Account_number,
    e.poid_id0,
    e.service_obj_id0,
    eb.amount,
    eb.offering_obj_id0,
    eb.product_obj_id0,
    pp.plan_obj_id0,
    pp.status,
    inf2ora(e.earned_start_t)earned_start_t ,
    inf2ora(e.earned_end_t)earned_end_t,
    inf2ora(pp.mod_t)mod_t,
    bl.bill_no,
    ceil(inf2ora(pp.mod_t)- inf2ora(e.earned_end_t)) DIFF,
    ROUND((eb.amount      /(ceil(inf2ora(e.earned_end_t)-inf2ora(e.earned_start_t))))*ceil(inf2ora(pp.mod_t)- inf2ora(e.earned_end_t)),2) ADJ,
    ROUND((eb.amount      /(ceil(inf2ora(e.earned_end_t)-inf2ora(e.earned_start_t))))*ceil(inf2ora(e.earned_start_t)-inf2ora(pp.mod_t)),2) DUP,
    ROUND(eb.amount       * (ceil(inf2ora(e.earned_end_t)-inf2ora(pp.mod_t)))/(ceil(inf2ora(e.earned_end_t)-inf2ora(e.earned_start_t))),2) CHARGE
    --SUM(eb.AMOUNT) Amount
  FROM EVENT_T E,
    EVENT_BAL_IMPACTS_T EB,
    ITEM_T I,
    ACCOUNT_T A1,
    purchased_product_t pp,
    bill_t bl,
    purchased_product_cycle_fees_t ppc,
    (SELECT A.account_no,
      I.bill_obj_id0,
      I.service_obj_id0,
      COUNT(1)
    FROM item_t I,
      account_t A
    WHERE I.POID_TYPE    IN ( '/item/cycle_forward/mso_sb_pkg_bst','/item/cycle_forward/mso_sb_pkg_dstp', '/item/cycle_forward/mso_sb_pkg_dpop','/item/cycle_forward/mso_sb_pkg_dprp' , '/item/cycle_forward/mso_sb_alc_fta' ,'/item/cycle_forward/mso_sb_alc_paid', '/item/cycle_forward/mso_sb_adn_normal')
    AND I.status         IN (2,4)
    AND I.account_obj_id0 = A.poid_id0
    GROUP BY A.account_no,
      I.bill_obj_id0,
      I.service_obj_id0
    HAVING COUNT(1) >= 1
    )dup
  WHERE I.bill_obj_id0    = dup.bill_obj_id0
  AND I.poid_type        IN ('/item/cycle_forward/mso_sb_pkg_bst','/item/cycle_forward/mso_sb_pkg_dstp', '/item/cycle_forward/mso_sb_pkg_dpop','/item/cycle_forward/mso_sb_pkg_dprp', '/item/cycle_forward/mso_sb_alc_paid','/item/cycle_forward/mso_sb_alc_fta','/item/cycle_forward/mso_sb_adn_normal')
  AND E.item_obj_id0      = I.poid_id0
  AND E.POID_ID0          = EB.OBJ_ID0
  AND EB.impact_type      = 1
  AND A1.poid_id0         = E.account_obj_id0
  AND eb.offering_obj_id0 = pp.poid_id0
  AND bl.poid_id0         = i.bill_obj_id0
  AND ppc.obj_id0         =pp.poid_id0
    --and pp.status=3
  AND TO_CHAR(inf2ora(bl.end_t), 'yyyy-mm') = '2014-07'
  --AND a1.account_no                         = '1000452236'
  )tbl1,
  (SELECT upper(T7.NAME) PLAN_NAME,
    T7.descr PLAN_DESCRIPTION,
    T7.POID_ID0 PLAN_POID,
    T6.SERVICE_OBJ_TYPE SERVICE_TYPE,
    T5.OBJ_ID0 DEAL_POID,
    T4.POID_ID0 PRODUCT_POID,
    T4.NAME PRODUCT_NAME,
    t4.descr PRODUCT_DESCRIPTION,
    T4.provisioning_tag PROVISIONING_TAG,
    T4.priority PRIORITY,
    T8.event_type EVENT_TYPE,
    T1.FIX_AMOUNT FIX_AMOUNT,
    T1.SCALED_AMOUNT SCALED_AMOUNT,
    T3.CURRENCY,
    T3.TAX_CODE,
    t9.event_count Cnt,
    DECODE(T3.TAX_WHEN,1,'EVENT_TIME',T3.TAX_WHEN,T3.TAX_WHEN) TAX_WHEN,
    (SELECT ca_id
    FROM mso_cfg_catv_pt_t cp,
      CATV_NE_INFO_T c
    WHERE cp.provisioning_tag=t4.provisioning_tag
    AND c.obj_id0            =cp.poid_id0
    AND c.network_node       = 'NDS'
    ) NDS_CA_ID,
    (SELECT ca_id
    FROM mso_cfg_catv_pt_t cp,
      CATV_NE_INFO_T c
    WHERE cp.provisioning_tag=t4.provisioning_tag
    AND c.obj_id0            =cp.poid_id0
    AND c.network_node       = 'CISCO'
    ) CISCO_CA_ID,
    (SELECT ca_id
    FROM mso_cfg_catv_pt_t cp,
      CATV_NE_INFO_T c
    WHERE cp.provisioning_tag=t4.provisioning_tag
    AND c.obj_id0            =cp.poid_id0
    AND c.network_node LIKE 'CISCO1'
    ) CISCO1_CA_ID
  FROM RATE_BAL_IMPACTS_T T1,
    RATE_T T2,
    RATE_PLAN_T T3,
    PRODUCT_T T4,
    DEAL_PRODUCTS_T T5,
    PLAN_SERVICES_T T6,
    PLAN_T T7,
    product_usage_map_t T8,
    config_permitted_events_t T9
  WHERE T1.OBJ_ID0                 =T2.POID_ID0
  AND T2.RATE_PLAN_OBJ_ID0         =T3.POID_ID0
  AND T3.PRODUCT_OBJ_ID0           =T4.POID_ID0
  AND T4.POID_ID0                  =T5.PRODUCT_OBJ_ID0
  AND T4.POID_ID0                  =T8.obj_id0
  AND T5.OBJ_ID0                   =T6.DEAL_OBJ_ID0
  AND T6.OBJ_ID0                   =T7.POID_ID0
  AND T9.event_type                = T8.event_type
  )tbl2 WHERE tbl1.plan_obj_id0=tbl2.plan_poid 
  and tbl2.priority!=170
 GROUP BY account_number, service_obj_id0, product_obj_id0, bill_no, tbl2.product_name, tbl2.scaled_amount);
spool off;
exit;
EOF
