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
EXECUTE MSO_LCO_CUST_CHARGES_partB_M1(to_date('20-AUG-2014','DD-MON-YYYY HH24:MI:SS'),to_date('29-AUG-2014','DD-MON-YYYY HH24:MI:SS'));
spool off;
exit;
EOF
