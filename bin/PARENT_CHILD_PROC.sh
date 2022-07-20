#!/bin/sh
#source .bash_profile
DB_STRING=`cat /home/pin/.pass/pin.db`
sqlplus -S ${DB_STRING} <<EOF
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
EXECUTE PARENT_CHILD_PROC;
exit;
EOF

sql_return_code=$?

echo "End of network_mac replace Report time:"`date`

if [ "$sql_return_code" -eq "0" ]; then
        echo "`date`:PARENT_CHILD_PROC success" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
else
        echo "`date`:PARENT_CHILD_PROC failed" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
fi
