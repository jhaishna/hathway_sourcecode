#!/bin/sh
#source .bash_profile
location="$1"
#rm $location/execution_logs
echo "collection report start:"
echo `date`
DB_STRING=`cat /home/pin/.pass/pin.db`
sqlplus -S ${DB_STRING} <<EOF
set termout off
set feedback off
set echo off
set linesize 50
set pagesize 30000
SET MARKUP HTML ON SPOOL ON PREFORMAT OFF ENTMAP ON
spool $location/MSO_FIN_COLLECTIONS_REG.xls
WHENEVER SQLERROR EXIT SQL.SQLCODE
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
execute MSO_FIN_COLL_REGISTER_REPORT; 
exit;
spool off;
EOF

sql_return_code=$?

if [ "$sql_return_code" -eq "0" ]; then
        echo "`date`:MSO_FIN_COLLECTION_REG Report success" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
else
        echo "`date`:MSO_FIN_COLLECTION_REG Report failed" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
fi

echo "collection_report end:"
echo `date`
echo "--------------------------------"
