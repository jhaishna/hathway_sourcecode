#!/bin/sh
#source .bash_profile
user="$1"
pass="$2"
SID="$3"
location="$4"
#rm $location/execution_logs
sqlplus -S $user/$pass@$SID <<EOF
set termout off
set feedback off
set echo off
set linesize 300
set pagesize 0
set newpage 0
spool $location/MSO_DEL_ADJ.csv
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
alter session force parallel dml parallel 8;
SELECT /*+ parallel(evt,8) parallel(pc,8) parallel(eb,8) parallel(bi,8) */ a.account_no ||','||round(-5*pc.scale,2) 
from event_t evt,event_product_fee_cycle_t pc,EVENT_BAL_IMPACTS_T eb,billinfo_t bi,event_billing_product_t ebp,account_t a where
evt.poid_type in
('/event/billing/product/fee/cycle/cycle_arrear_monthly/mso_sb_pkg_dpop',
'/event/billing/product/fee/cycle/cycle_arrear_monthly/mso_sb_pkg_dprp')
and evt.account_obj_id0 = bi.account_obj_id0
and bi.billing_segment = 1500
and evt.poid_id0 = pc.obj_id0
and evt.poid_id0 = eb.obj_id0
and eb.impact_type = 1
and eb.tax_code = 'MSO_Service_Tax'
and amount !=0
and a.poid_id0=evt.account_obj_id0
and ebp.obj_id0 = evt.poid_id0
and ebp.product_obj_id0 in (1306077,1304541,1306269,1303197);
spool off;
EOF
