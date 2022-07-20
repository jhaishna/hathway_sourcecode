#!/bin/sh
#source .bash_profile
echo "Start of Report:`date`"
#location="$4"
#rm $location/execution_logs
DB_STRING=`cat /home/pin/.pass/pin.db`
sqlplus -S ${DB_STRING} <<EOF
set termout off
set feedback off
set echo off
set linesize 500
set newpage 0
set pagesize 30000
WHENEVER SQLERROR EXIT SQL.SQLCODE
SET SERVEROUTPUT ON SIZE UNLIMITED
alter session set nls_date_format = 'DD-MON-YYYY';
EXECUTE	CREATE_PURCHASE_PRD_DUMP;
exit;
EOF

sql_return_code=$?

if [ "$sql_return_code" -eq "0" ]; then
        echo "`date`:Purchased product dump success" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
else
        echo "`date`:Purchased product dump failed" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
fi

echo "End of Report:`date`"
