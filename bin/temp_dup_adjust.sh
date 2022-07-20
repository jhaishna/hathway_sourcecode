#!/bin/sh
#source .bash_profile
user="$1"
pass="$2"
SID="$3"
#rm $location/execution_logs
sqlplus -S $user/$pass@$SID <<EOF
set termout off
set feedback off
set echo off
set linesize 500
set pagesize 0
set newpage 0
SET SERVEROUTPUT ON SIZE UNLIMITED
create table TEMP_DUP_CHARGES_30JUL as (
select /*+ parallel (E,8) parallel(EB,8) parallel(I,8) parallel(A1,8) */ A1.account_no Account_number, sum(eb.AMOUNT) Amount
FROM EVENT_T E, EVENT_BAL_IMPACTS_T EB,ITEM_T I,ACCOUNT_T A1,purchased_product_t pp , billinfo_t bi,
(select A.account_no,I.bill_obj_id0,I.service_obj_id0,count(1) from item_t I, account_t A
where I.POID_TYPE in (
'/item/cycle_forward/mso_sb_pkg_bst','/item/cycle_forward/mso_sb_pkg_dstp',
'/item/cycle_forward/mso_sb_pkg_dpop','/item/cycle_forward/mso_sb_pkg_dprp' , '/item/cycle_forward/mso_sb_alc_fta' ,'/item/cycle_forward/mso_sb_alc_paid',
'/item/cycle_forward/mso_sb_adn_normal')
and I.status in (2,4)
and  I.account_obj_id0 = A.poid_id0 
and A.et_zone like 'MH%' group by A.account_no,I.bill_obj_id0,I.service_obj_id0 having count(1) >=1)dup
where I.bill_obj_id0 = dup.bill_obj_id0
and I.poid_type in ('/item/cycle_forward/mso_sb_pkg_bst','/item/cycle_forward/mso_sb_pkg_dstp',
'/item/cycle_forward/mso_sb_pkg_dpop','/item/cycle_forward/mso_sb_pkg_dprp',
'/item/cycle_forward/mso_sb_alc_paid','/item/cycle_forward/mso_sb_alc_fta','/item/cycle_forward/mso_sb_adn_normal')
and E.item_obj_id0 = I.poid_id0
AND E.POID_ID0 = EB.OBJ_ID0
AND EB.impact_type = 1
AND eb.offering_obj_id0 = pp.poid_id0
and eb.product_obj_id0 = pp.product_obj_id0
and pp.status = 3
and bi.account_obj_id0 = e.account_obj_id0
and bi.account_obj_id0 = pp.account_obj_id0
and pp.purchase_end_t < bi.actg_last_t
AND A1.poid_id0 = E.account_obj_id0 group by A1.account_no);
spool off;
exit;
EOF
