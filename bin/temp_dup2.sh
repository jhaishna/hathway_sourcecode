#!/bin/sh
#source .bash_profile
user="$1"
pass="$2"
SID="$3"
#rm $location/execution_logs
sqlplus -S $user/$pass@$SID <<EOF
CREATE TABLE temp_dup_basic_NON_MH_31_JUL nologging AS
  (SELECT /*+parallel(a1,16) parallel(e,16) parallel(eb,16) full (a1) full(eb) full(e)*/
  A1.account_no account_no ,
      E.poid_id0 event_poid ,
      dup.bill_obj_id0 bill_poid ,
      E.ITEM_OBJ_ID0 item_poid ,
      E.ITEM_OBJ_TYPE item_type ,
      inf2ora(e.earned_start_t) earned_start_t ,
      inf2ora(e.earned_end_T) earned_end_t ,
      eb.AMOUNT
    FROM EVENT_T E,
      EVENT_BAL_IMPACTS_T EB,
      ITEM_T I,
      ACCOUNT_T A1,
      (SELECT /*+parallel (a,16) parallel(i,16)*/ 
      A.account_no,
        I.bill_obj_id0,
        I.service_obj_id0,
        COUNT(1)
      FROM item_t I,
        account_t A
      WHERE I.POID_TYPE    IN ('/item/cycle_arrear/mso_sb_pkg_bst','/item/cycle_arrear/mso_sb_pkg_dstp', '/item/cycle_arrear/mso_sb_pkg_dpop','/item/cycle_arrear/mso_sb_pkg_dprp', '/item/cycle_forward/mso_sb_pkg_bst','/item/cycle_forward/mso_sb_pkg_dstp', '/item/cycle_forward/mso_sb_pkg_dpop','/item/cycle_forward/mso_sb_pkg_dprp')
      AND I.status         IN (2,4)
      AND I.account_obj_id0 = A.poid_id0
      AND A.et_zone NOT LIKE 'MH%'
      GROUP BY A.account_no,
        I.bill_obj_id0,
        I.service_obj_id0
      HAVING COUNT(1) > 1
      )dup
    WHERE I.bill_obj_id0 = dup.bill_obj_id0
    AND I.poid_type     IN ('/item/cycle_arrear/mso_sb_pkg_bst', '/item/cycle_arrear/mso_sb_pkg_dstp', '/item/cycle_arrear/mso_sb_pkg_dpop','/item/cycle_arrear/mso_sb_pkg_dprp', '/item/cycle_forward/mso_sb_pkg_bst','/item/cycle_forward/mso_sb_pkg_dstp', '/item/cycle_forward/mso_sb_pkg_dpop','/item/cycle_forward/mso_sb_pkg_dprp')
    AND E.item_obj_id0   = I.poid_id0
    AND E.POID_ID0       = EB.OBJ_ID0
    AND EB.impact_type   = 1
    AND A1.poid_id0      = E.account_obj_id0
  );
spool off;
exit;
EOF
