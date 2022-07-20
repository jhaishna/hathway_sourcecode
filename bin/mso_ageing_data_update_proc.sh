#!/bin/sh
#source .bash_profile
user="$1"
pass="$2"
SID="$3"
#rm $location/execution_logs
echo "Procedure Execution Started !"
sqlplus -S $user/$pass@$SID <<EOF
set termout off
set feedback off
set echo off
set linesize 50
set pagesize 30000
EXECUTE MSO_UPDATE_AGEING_DATA;
 exit;
EOF
echo "Procedure Execution Completed !"
