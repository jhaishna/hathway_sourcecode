#!/bin/sh
#source .bash_profile
echo "Start of CUSTOMER_MASTER Report time:"`date`
#location="$4"
#rm $location/execution_logs
DB_STRING=`cat /home/pin/.pass/pin.db`
sqlplus -S ${DB_STRING} <<EOF
set termout off
set feedback off
set echo off
set linesize 50
set pagesize 30000
WHENEVER SQLERROR EXIT SQL.SQLCODE
SET MARKUP HTML ON SPOOL ON PREFORMAT OFF ENTMAP ON
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
execute MSO_CUST_MASTER;
exit;
EOF

sql_return_code=$?

if [ "$sql_return_code" -eq "0" ]; then
        echo "`date`:Cust master dump success" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
else
        echo "`date`:Cust master dump  failed" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
fi

echo "End of CUSTOMER_MASTER Report:"`date`
