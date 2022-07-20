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
SELECT /*+ parallel(evt,8) parallel(pc,8) parallel(eb,8) parallel(bi,8) */ a.account_no ||','|| a.legacy_account_no ||','||inf2ora(evt.CREATED_T)||','||evt.NAME||','||evt.PROGRAM_NAME||','||evt.SYS_DESCR||','||round(eb.amount,2)||','||round(eb.percent,2)||','||round(pc.scale,2)||','||round(-5*pc.scale,2) FROM event_t evt,event_product_fee_cycle_t pc,EVENT_BAL_IMPACTS_T eb,billinfo_t bi,ACCOUNT_T a WHERE evt.poid_type IN('/event/billing/product/fee/cycle/cycle_arrear_monthly/mso_sb_pkg_dpop','/event/billing/product/fee/cycle/cycle_arrear_monthly/mso_sb_pkg_dstp','/event/billing/product/fee/cycle/cycle_arrear_monthly/mso_sb_pkg_dprp') and evt.account_obj_id0 = bi.account_obj_id0 and bi.billing_segment = 1500 and evt.poid_id0 = pc.obj_id0 and evt.poid_id0 = eb.obj_id0 and eb.impact_type = 1 and eb.tax_code='MSO_Service_Tax' and amount !=0 and a.poid_id0=evt.account_obj_id0;
spool off;
EOF
