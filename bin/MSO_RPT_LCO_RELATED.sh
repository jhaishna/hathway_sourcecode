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
set linesize 50
set pagesize 30000
SET MARKUP HTML ON SPOOL ON PREFORMAT OFF ENTMAP ON
spool $location/MSO_LCO_RELATED.xls
EXECUTE MSO_RPT_LCO_RELATED;
select LCO_CODE,LCO_NAME,LOCATION,TOTAL_STBS_ISSUED,ACTIVE_STBS,DEACTIVE_STBS,ACTIVE_CRFS_RECEIVED,MTD_BILLING,MTD_COLLECTION,YTD_BILLING,YTD_COLLECTIONS,OS,NO_OF_MONTHS_OS from MSO_RPT_BRS_LCO_T;
 exit;
spool off;
EOF
