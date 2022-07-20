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
alter session set nls_date_format = 'DD-MON-YYYY';
EXECUTE MSO_LCO_CUST_CHARGES_PARTB_M1(1409509800,1412015400);
spool off;
exit;
EOF
