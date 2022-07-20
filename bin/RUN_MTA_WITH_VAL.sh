#!/bin/sh

while true
do

cd /home/pin/Thirdparty/BRM12/BRM/mso/bin

x=`free -g|grep 'Mem'|tr -s " "|cut -d ' ' -f4`
PID=`ps -ef|grep mso_cancel_plan|egrep -v grep|wc -l`
if [ $x -lt 10 ]
then
 echo $x "GB is < 5 hence stopping" > /home/pin/Thirdparty/BRM12/BRM/mso/bin/RUM_MTA.log

fi
#if [ $PID -gt 0 ]
#then
#ps -ef|grep mso_cancel_plan|egrep -v grep|tr -s " " |cut -d ' ' -f2|xargs kill -9
#fi

if [ $PID -eq 0 ]
then
echo $PID "MTA Not Running Hence Checking Pending" > /home/pin/Thirdparty/BRM12/BRM/mso/bin/RUM_MTA.log

DB_STRING=`cat /home/pin/.pass/pin.db`
OUT_FILE=/home/pin/Thirdparty/BRM12/BRM/mso/bin/COUNT_MTA.log

#/home/oracle/11g/database/client/11.2.0.4/bin/sqlplus -s ${DB_STRING} << EOF > $OUT_FILE

sqlplus -s /nolog << EOF  > $OUT_FILE
connect ${DB_STRING}

set echo off head off pagesize 0 trimspool on linesize 5000;
select COUNT(1) FROM cancel_plan_mta_t WHERE STATUS=0;
exit;
EOF

y=`cat $OUT_FILE`

if [ $y -gt 0 ]
then
 echo $y "Records Pending Hence Starting" > /home/pin/Thirdparty/BRM12/BRM/mso/bin/RUM_MTA.log

nohup mso_cancel_plan_with_val -verbose -object_type /cancel_plan &

wait

#/home/oracle/11g/database/client/11.2.0.4/bin/sqlplus -s ${DB_STRING} << EOF > $OUT_FILE

sqlplus -s /nolog << EOF > $OUT_FILE
connect ${DB_STRING}

set echo off head off pagesize 0 trimspool on linesize 5000;
select COUNT(1) FROM cancel_plan_mta_t WHERE STATUS=0;
exit;
EOF

y=`cat $OUT_FILE`

fi

if [ $y -eq 0 ]
then
#/home/oracle/11g/database/client/11.2.0.4/bin/sqlplus -s ${DB_STRING} << EOF > $OUT_FILE

sqlplus -s /nolog << EOF > $OUT_FILE
connect ${DB_STRING}

UPDATE cancel_plan_mta_t SET STATUS=0 WHERE STATUS=99;
COMMIT;

UPDATE   purchased_product_t
   SET   status = 3
 WHERE   (package_id, service_obj_id0, plan_obj_id0, deal_obj_id0) IN
               (SELECT   DISTINCT package_id,
                                  service_obj_id0,
                                  plan_obj_id0,
                                  deal_obj_id0
                  FROM   cancel_plan_mta_t
                 WHERE   ticket_no LIKE
                            TO_CHAR (SYSDATE, 'yyyymmdd') || '%00%'
                         AND status IN (1, 4))
         AND status = 1
         AND utc (purchase_end_t) <= TRUNC (SYSDATE);

COMMIT;

exit;
EOF
fi

fi
sleep 60

done

exit 0;