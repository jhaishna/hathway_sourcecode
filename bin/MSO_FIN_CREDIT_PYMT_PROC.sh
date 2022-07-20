#!/bin/sh
#source .bash_profile
user="pin"
pass="Hw9Ob3Mr"
SID="PRODBRM"
sqlplus -S $user/$pass@$SID <<EOF
set termout off
set feedback off
set echo off
set linesize 500
set pagesize 0
set newpage 0
set timing on
WHENEVER SQLERROR EXIT SQL.SQLCODE
SET SERVEROUTPUT ON SIZE UNLIMITED
alter session set nls_date_format = 'DD-MON-YYYY';
EXECUTE MSO_FIN_CREDIT_PYMT_PROC;
exit;
EOF

sql_return_code=$?

if [ "$sql_return_code" -eq "0" ]; then
        echo "`date`:Credit payment proc success" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
else
        echo "`date`:credit payment proc success" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
fi

