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
SET SERVEROUTPUT ON SIZE UNLIMITED
WHENEVER SQLERROR EXIT SQL.SQLCODE
EXECUTE MSO_LCO_CRF;
exit;
EOF

sql_return_code=$?

if [ "$sql_return_code" -eq "0" ]; then
        echo "`date`:LCO CRF Report success" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
else
        echo "`date`:LCO CRF Report failed" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
fi

echo "END of report:`date`"
