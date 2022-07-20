#!/bin/sh
#source .bash_profile
echo "Start of report:`date`"
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
set newpage 0
set pagesize 30000
WHENEVER SQLERROR EXIT SQL.SQLCODE
SET SERVEROUTPUT ON SIZE UNLIMITED
EXECUTE	MSO_FIN_CRF_RPT_PROC;
exit;
EOF

sql_return_code=$?

if [ "$sql_return_code" -eq "0" ]; then
        echo "`date`:CRF Report PROC success" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
else
        echo "`date`:CRF Report PROC failed" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
fi

echo "End of report:`date`"
