#!/bin/sh
#source .bash_profile
echo "Start of report:`date`"
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
EXECUTE	MSO_CUST_ALL_DATA_RPT_PROC;
exit;
EOF

sql_return_code=$?

if [ "$sql_return_code" -eq "0" ]; then
        echo "`date`:CUST_ALL_RPT success" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
else
        echo "`date`:CUST_ALL_RPT failed" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
fi

echo "END of report:`date`"
