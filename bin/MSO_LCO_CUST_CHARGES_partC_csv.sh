#!/bin/sh
#source .bash_profile
#user="$1"
#pass="$2"
#SID="$3"
location="/home/pin/opt/portal/7.5/mso/reports/output"
#location="$4"
#rm $location/execution_logs
DB_STRING=`cat /home/pin/.pass/pin.db`
sqlplus -S ${DB_STRING} <<EOF
#sqlplus -S $user/$pass@$SID <<EOF
set termout off
set feedback off
set echo off
set linesize 500
set pagesize 0
set newpage 0
SET SERVEROUTPUT ON SIZE UNLIMITED
spool $location/POSTB_PARTC_SP_MH_DEC_JAN18.csv_orig
select customer_id||','||agreement_no||','||bill_no||',"'||replace(name,',',' ')||'",'||entity_code||',"'||replace(entity_name,',',' ')||'",'||decoder||','||smartcard||',"'||replace(address,chr(10),'')||'",'||item_type||','||total||','||lco_margin||','||brr_charges||','||gst_on_brr||','||payable_to_hathway||','||city||','||bill_creation_date from POSTB_PARTC_SP_MH_DEC_JAN18 where item_type <> 'Entertainment Tax' order by entity_code,customer_id,agreement_no,item_type asc;
spool off;
exit;
EOF
