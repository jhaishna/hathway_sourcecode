#!/bin/bash

USERNAME="$1"
PASSWD="$2"
IP_ADDRESS="$3"
PORT="$4"
ORACLE_SID="$5"
DB_STRING=`cat /home/pin/.pass/pin.db`

#conn_str=$userid/$passwd@$SID
#conn_str=$USERNAME/$PASSWD@$IP_ADDRESS:$PORT/$ORACLE_SID

#OUT_FILE=/home/pin/opt/portal/7.5/mso/apps/mso_bulk_utilities/bulk_acct_creation_proc.log
date_time=`date "+%d_%m_%Y_%H_%M_%S"`
LOG_FILE=/home/pin/opt/portal/7.5/mso/apps/mso_bulk_utilities/log/bulk_account_creation/bulk_acct_creation_proc'_'$date_time.log

echo "Script Execution Started"
sqlplus -S ${DB_STRING} <<EOF >$LOG_FILE

EXECUTE MSO_ACCT_ADDR_ID_UPDATE;

exit;
EOF

echo "Script Execution Completed !"
