#!/bin/sh
#source .bash_profile
#user="$1"
#pass="$2"
#SID="$3"
location="/home/pin/opt/portal/7.5/mso/reports/output"
#rm $location/execution_logs
#sqlplus -S $user/$pass@$SID <<EOF
DB_STRING=`cat /home/pin/.pass/pin.db`
sqlplus -S ${DB_STRING} <<EOF
set termout off
set feedback off
set echo off
set linesize 500
set pagesize 0
set newpage 0
SET SERVEROUTPUT ON SIZE UNLIMITED
spool $location/POSTB_PARTB_WB_FEB_MAR19.csv_orig
select customer_id||','||agreement_no||','||bill_no||',"'||replace(name,',',' ')||'",'||entity_code||',"'||replace(entity_name,',',' ')||'",'||decoder||','||smartcard||',"'||replace(address,chr(10),'')||'",'||item_type||','||charges||','||igst||','||cgst||','||sgst||','||total||','||city||','||charge_start_date||','||charge_end_date||','||billing_due_date||','||bill_creation_date||','||payterm 
from POSTB_PARTB_WB_FEB_MAR19; 
spool off;
exit;
EOF
