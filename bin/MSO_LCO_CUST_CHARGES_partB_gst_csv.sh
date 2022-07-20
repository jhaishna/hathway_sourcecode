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
set linesize 500
set pagesize 0
set newpage 0
SET SERVEROUTPUT ON SIZE UNLIMITED
spool $location/POSTB_PARTB_SP_DOM7_JUN_JULY17.csv_orig
select customer_id||','||agreement_no||','||bill_no||',"'||replace(name,',',' ')||'",'||entity_code||',"'||replace(entity_name,',',' ')||'",'||decoder||','||smartcard||',"'||replace(address,chr(10),'')||'",'||item_type||','||charges||','||st||','||total||','||city||','||charge_start_date||','||charge_end_date||','||billing_due_date||','||bill_creation_date||','||payterm 
from POSTB_PARTB_SP_DOM7_JUN_JULY17;
spool off;
exit;
EOF
