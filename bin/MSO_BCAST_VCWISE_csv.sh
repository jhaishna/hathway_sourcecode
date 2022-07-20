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
spool $location/MSO_BCAST_VCWISE_APR_JUL_14.csv
SELECT broadcaster
  ||','
  ||customer_id
  ||','
  ||agreement_no
  ||','
  ||smartcard
  ||','
  ||entity_code
  ||','
  ||city
  ||','
  ||product_name
  ||','
  ||phase
  ||','
  ||customer_type
  ||','
  ||from_date
  ||','
  ||to_date
FROM MSO_BCAST_VCwise_RPT
ORDER BY from_date,
  broadcaster;
spool off;
exit;
EOF
