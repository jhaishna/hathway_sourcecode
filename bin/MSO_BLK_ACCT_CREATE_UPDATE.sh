#!/bin/sh
#source .bash_profile
#location="$1"
#rm $location/execution_logs
echo "Bulk Account Creation start:"
echo `date`
DB_STRING=`cat /home/pin/.pass/pin.db`
sqlplus -S ${DB_STRING} <<EOF
set termout off
set feedback off
set echo off
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
execute mso_blk_acct_create_update;
exit;
spool off;
EOF

sql_return_code=$?

if [ "$sql_return_code" -eq "0" ]; then
        echo "`date`:MSO_BLK_ACCT_CREATE_UPDATE Procedure success" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
else
        echo "`date`:MSO_BLK_ACCT_CREATE_UPDATE Procedure failed" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
fi

echo "Bulk Account Creation end:"
echo `date`
echo "--------------------------------"


