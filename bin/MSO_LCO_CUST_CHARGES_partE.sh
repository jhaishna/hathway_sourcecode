#!/bin/sh
#source .bash_profile
user="$1"
pass="$2"
SID="$3"
#location="$4"
#rm $location/execution_logs
sqlplus -S $user/$pass@$SID <<EOF
set termout off
set feedback off
set echo off
set linesize 500
set newpage 0
set pagesize 30000
SET SERVEROUTPUT ON SIZE UNLIMITED
EXECUTE	MSO_LCO_CUST_CHARGES_PARTE;
exit;
EOF
