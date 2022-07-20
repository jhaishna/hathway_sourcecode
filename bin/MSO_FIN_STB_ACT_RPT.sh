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
set linesize 50
set pagesize 30000
SET MARKUP HTML ON SPOOL ON PREFORMAT OFF ENTMAP ON
spool $location/MSO_FIN_STB_ACT_RPT.xls
EXECUTE MSO_FIN_STB_ACT;
 exit;
spool off;
EOF
