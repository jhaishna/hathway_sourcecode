#rm $location/execution_logs
echo "CFO  report start:"
echo `date`
DB_STRING=`cat /home/pin/.pass/pin.db`
sqlplus -S ${DB_STRING} <<EOF
set termout off
set feedback off
set echo off
set linesize 500
set pagesize 0
set newpage 0
WHENEVER SQLERROR EXIT SQL.SQLCODE
SET SERVEROUTPUT ON SIZE UNLIMITED
alter session set nls_date_format = 'DD-MON-YYYY';
EXECUTE MSO_CFO_DASHBOARD_PROC;
exit;
EOF

sql_return_code=$?

if [ "$sql_return_code" -eq "0" ]; then
        echo "`date`:CFO DASH_BOARD RPT success" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
else
        echo "`date`:CFO DASH_BOARD RPT  failed" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
fi

echo "CFO Report end:"
echo `date`
echo "--------------------------------"

