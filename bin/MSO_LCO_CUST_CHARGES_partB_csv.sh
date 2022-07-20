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
spool $location/MSO_LCO_CUST_CHARGES_partB.csv
select customer_id||','||agreement_no||','||bill_no||',"'||replace(name,',',' ')||'",'||entity_code||',"'||replace(entity_name,',',' ')||'",'||decoder||','||smartcard||',"'||replace(address,chr(10),'')||'",'||item_type||','||charges||','||st||','||total||','||city||','||charge_start_date||','||charge_end_date||','||billing_due_date||','||bill_creation_date||','||payterm from MSO_LCO_CUST_CHARGES_partB_rpt;
spool off;
exit;
EOF
