#!/bin/bash

USERNAME="$1"
PASSWD="$2"
IP_ADDRESS="$3"
PORT="$4"
ORACLE_SID="$5"

#conn_str=$userid/$passwd@$SID
conn_str=$USERNAME/$PASSWD@$IP_ADDRESS:$PORT/$ORACLE_SID

#OUT_FILE=/home/pin/opt/portal/7.5/mso/apps/mso_bulk_utilities/bulk_acct_creation_proc.log
date_time=`date "+%d_%m_%Y_%H_%M_%S"`
LOG_FILE=/home/pin/opt/portal/7.5/mso/apps/mso_bulk_utilities/log/bulk_account_creation/bulk_acct_creation_proc'_'$date_time.log

sqlplus -s /nolog <<EOF >$LOG_FILE
connect ${conn_str}

TRUNCATE TABLE MSO_BLK_ACCT_DEVICE_TMP_T;
commit;
  
EXECUTE MSO_BLK_ACCT_CREATE_UPDATE;

exit;
EOF

OUT_FILE=/home/pin/opt/portal/7.5/mso/apps/mso_bulk_utilities/output/bulk_account_creation/acct_list'_'$date_time.csv

sqlplus -s /nolog <<EOF >$OUT_FILE
connect ${conn_str}

set serveroutput on;
set termout off
set heading off
set feedback off
set echo off
set NEWPAGE NONE
set trimspool ON
set linesize 500
set pagesize 0

SELECT 'STB_NUMBER,ACCOUNT_NUMBER' FROM DUAL;
SELECT SUBSTR(STB_NO, 1,25) STB_NO,',', SUBSTR(ACCOUNT_NO,1, 10) ACCOUNT_NO FROM MSO_BLK_ACCT_DEVICE_TMP_T;

exit;
EOF

