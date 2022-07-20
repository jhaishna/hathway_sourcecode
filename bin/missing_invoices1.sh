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
set pagesize 0
set newpage 0
set trimspool off
SET SERVEROUTPUT ON SIZE UNLIMITED
--spool $location/total_accounts.txt
select /*+ parallel(i,16) parallel(a,16) */
a.account_no from invoice_t i , account_t a where (i.status = 0 or i.status= 260320151) and i.account_obj_id0 = a.poid_id0 ;
spool off;
exit;
EOF
