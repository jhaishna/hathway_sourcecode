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
EXECUTE	MSO_LCO_CUST_CHARGES_PartB_M;
exit;
EOF
