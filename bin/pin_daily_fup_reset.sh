#!/bin/bash

VERSION=$PIN_VERSION

PINDIR=/home/pin/Thirdparty/BRM12/BRM/
LOGDIR=/home/pin/Thirdparty/BRM12/BRM/var/pin_billd

CNFDIR=${PINDIR}/apps/pin_billd

PATH=/usr/bin:/bin:${PINDIR}/bin

OUT_FILE=${CNFDIR}/fup_reset.dat
OUT_FILEC=${CNFDIR}/fup_reset_correct.log
OUT_FILENAP=${CNFDIR}/fup_resetcor.nap
START_DATE=`date +%d' '%b' '%y' '%H:%M:%S`
UNIX_START_DATE=`date +%s -d "$START_DATE"`
:>/home/pin/Thirdparty/BRM12/BRM/apps/pin_billd/fup_resetcor.log
:>/home/pin/Thirdparty/BRM12/BRM/apps/pin_billd/fup_resetcor.nap
#Adding Two Mail id in the list at the last on 31-Jan-2018 - Vineet

MAIL_BODY_FILE=${PINDIR}/apps/pin_billd/mailBodyFup.txt
#TOM='newton.cardoza@hathway.net','rohan.naik@hathway.net','vishal.vairal@hathway.net','sandeepvarma@hcspl.net','vineet.tripathi@hathway.net','zsstopshathway@zenithss.co.in','tcs-obrm-lead@hathway.net','shailesh.bhoyar@tcs.com','shreyansh.tiwari2@tcs.com','sushmitha.pk@tcs.com','vishal.dhakad1@tcs.com','tcs-pm@hathway.net','prafulla@hathway.net','tcs-ops-head@hathway.net'

DB_STRING=`cat /home/pin/.pass/pin.db`
sqlplus -S ${DB_STRING} <<EOF
EOF
CURRENT_DATE=`date +%d' '%b' '%y`;
UNIXTIME=`date +%s -d "$CURRENT_DATE"`;

cd ${CNFDIR}

#----------------------------------------------------------------#
#---------pin_cycle_fees -regular_cycle_fees---------------------#
# pin_cycle_fees -verbose -regular_cycle_fees -cust_type both

#pin_cycle_fees -verbose -regular_cycle_fees -cust_type prepay

#pin_cycle_fees -verbose -regular_cycle_fees -cust_type postpay

# postpay - service_bb_info_t.indicator 1
# prepay - service_bb_info_t.indicator 2

#--
echo "starting  pin_cycle_fees fup reset@" `date`
pin_cycle_fees -verbose -regular_cycle_fees -cust_type prepay
#wait
echo "finished  pin_cycle_fees for fup reset@" `date`

#----------------------------------------------------------------#
# Correction Automation FUP (Suresh Nadar)
#----------------------------------------------------------------#

#/home/oracle/11g/database/client/11.2.0.4/bin/sqlplus -s ${DB_STRING} << EOF > $OUT_FILEC
/home/oracle/app/oracle/product/12.2.0/client_1/bin/sqlplus -s ${DB_STRING} << EOF > $OUT_FILEC

UPDATE mso_cfg_credit_limit_t c SET PLAN_OBJ_ID0=(SELECT POID_ID0 FROM PLAN_T P WHERE upper(c.plan_name)=upper(P.name))
WHERE PLAN_OBJ_ID0 =0;

DROP TABLE AD_DATA_FUP_FAILED PURGE;

CREATE TABLE AD_DATA_FUP_FAILED
PARALLEL 8 NOLOGGING 
AS
select /*+  full(a) full(b) full(s) full(s1)*/
distinct(b.account_obj_id0) from 
purchased_product_cycle_fees_t a, 
purchased_product_t b, 
service_t s, 
service_bb_info_t s1
where
a.obj_id0 = b.poid_id0
and a.count=1 and a.unit=0
and b.account_obj_id0 = s.account_obj_id0 
and s.poid_id0 = s1.obj_id0
and b.status=1
and s1.indicator = 2 
and s1.fup_status <>0
and a.charged_to_t < $UNIXTIME 
and b.purchase_end_t > $UNIXTIME;

DROP TABLE AD_FUP1 PURGE;

CREATE TABLE AD_FUP1 AS
SELECT POID_ID0,ACCOUNT_NO FROM ACCOUNT_T WHERE POID_ID0 IN(SELECT ACCOUNT_OBJ_ID0 FROM AD_DATA_FUP_FAILED);

DROP TABLE AD_FUP2 PURGE;

CREATE TABLE AD_FUP2 AS
SELECT DISTINCT SERVICE_OBJ_ID0 ,ACCOUNT_OBJ_ID0,PT.POID_ID0 PURC_POID FROM PURCHASED_PRODUCT_T PT, AD_FUP1 X
WHERE PT.ACCOUNT_OBJ_ID0=X.POID_ID0
AND PT.SERVICE_OBJ_TYPE <> '/service/netflix'; 

DROP TABLE AD_FUP3 PURGE;

CREATE TABLE AD_FUP3 AS
SELECT DISTINCT PT.POID_ID0 SERV_POID,PT.ACCOUNT_OBJ_ID0,STATUS FROM SERVICE_T PT, AD_FUP1 X
WHERE PT.ACCOUNT_OBJ_ID0=X.POID_ID0
AND PT.POID_TYPE <> '/service/netflix'; 

DELETE FROM AD_FUP2 WHERE SERVICE_OBJ_ID0 IN(SELECT SERV_POID FROM AD_FUP3);

DELETE FROM AD_FUP3 WHERE STATUS=10103; 

COMMIT;

UPDATE PURCHASED_PRODUCT_T X SET SERVICE_OBJ_ID0 =(SELECT SERV_POID FROM AD_FUP3 A WHERE X.ACCOUNT_OBJ_ID0=A.ACCOUNT_OBJ_ID0)
WHERE X.POID_ID0 IN(SELECT PURC_POID FROM AD_FUP2);

COMMIT;

DROP TABLE AD_CORRECT_TECH PURGE;

CREATE TABLE AD_CORRECT_TECH AS
SELECT PT.ACCOUNT_OBJ_ID0,PT.SERVICE_OBJ_ID0,PT.PRODUCT_OBJ_ID0,P.NAME,PROVISIONING_TAG FROM 
PURCHASED_PRODUCT_T PT,PRODUCT_T P WHERE PT.ACCOUNT_OBJ_ID0 IN(
SELECT POID_ID0 FROM AD_FUP1 WHERE POID_ID0 IN(
SELECT POID_ID0 FROM AD_FUP1
MINUS
SELECT ACCOUNT_OBJ_ID0 FROM AD_FUP2))
AND PT.STATUS <>3
AND PT.PRODUCT_OBJ_ID0=P.POID_ID0
AND P.PROVISIONING_TAG IS NOT NULL;

UPDATE SERVICE_BB_INFO_T SET TECHNOLOGY=2 WHERE OBJ_ID0 IN(
SELECT SERVICE_OBJ_ID0 FROM AD_CORRECT_TECH WHERE PROVISIONING_TAG LIKE 'D3%');

UPDATE SERVICE_BB_INFO_T SET TECHNOLOGY=1 WHERE OBJ_ID0 IN(
SELECT SERVICE_OBJ_ID0 FROM AD_CORRECT_TECH WHERE PROVISIONING_TAG LIKE 'D2%');

UPDATE SERVICE_BB_INFO_T SET TECHNOLOGY=5 WHERE OBJ_ID0 IN(
SELECT SERVICE_OBJ_ID0 FROM AD_CORRECT_TECH WHERE PROVISIONING_TAG LIKE 'GPON%');

COMMIT;

DROP TABLE AD_CORRECT_TECH PURGE;
DROP TABLE AD_FUP3 PURGE;
DROP TABLE AD_FUP2 PURGE;
DROP TABLE AD_DATA_FUP_FAILED PURGE;

exit;
EOF

/home/oracle/11g/database/client/11.2.0.4/bin/sqlplus -s ${DB_STRING} << EOF > $OUT_FILENAP
set echo off head off feedback off pagesize 0 trimspool on linesize 5000;

SELECT 'r << e 1'||CHR(10)||
'0 PIN_FLD_POID POID [0] 0.0.0.1 /account '||X.POID_ID0||' 0'||CHR(10)||
'0 PIN_FLD_PROGRAM_NAME STR [0] "pin_cycle_fees" '||CHR(10)||
'0 PIN_FLD_FLAGS INT [0] 256 '||CHR(10)||
'0 PIN_FLD_END_T TSTAMP [0] ('||$UNIXTIME||')'||CHR(10)||
'e'||CHR(10)||
'xop PCM_OP_SUBSCRIPTION_CYCLE_FORWARD 0 1'
FROM AD_FUP1 X;

exit;
EOF

if [ -e fup_resetcor.nap ]
then
	testnap fup_resetcor.nap >> fup_resetcor.log
fi

#----------------------------------------------------------------#
# Mail body
#----------------------------------------------------------------#

/home/oracle/11g/database/client/11.2.0.4/bin/sqlplus -s ${DB_STRING} << EOF >> $OUT_FILEC

DROP TABLE AD_DATA_FUP_FAILED PURGE;

CREATE TABLE AD_DATA_FUP_FAILED
PARALLEL 8 NOLOGGING
AS
select /*+  full(a) full(b) full(s) full(s1)*/
distinct(b.account_obj_id0) from
purchased_product_cycle_fees_t a,
purchased_product_t b,
service_t s,
service_bb_info_t s1
where
a.obj_id0 = b.poid_id0
and a.count=1 and a.unit=0
and b.account_obj_id0 = s.account_obj_id0
and s.poid_id0 = s1.obj_id0
and b.status=1
and s1.indicator = 2
and s1.fup_status <>0
and a.charged_to_t < $UNIXTIME
and b.purchase_end_t > $UNIXTIME;

exit;
EOF

/home/oracle/11g/database/client/11.2.0.4/bin/sqlplus -s ${DB_STRING} << EOF > $OUT_FILE
set echo off head off pagesize 0 trimspool on linesize 5000;
select * from AD_DATA_FUP_FAILED;
exit;
EOF

END_DATE=`date +%d' '%b' '%y' '%H:%M:%S`
UNIX_END_DATE=`date +%s -d "$END_DATE"`
DURATION=`expr $UNIX_END_DATE - $UNIX_START_DATE`
HOURS=`expr $DURATION / 60 / 60`
MINS=`expr $DURATION / 60`
SECS=`expr $DURATION % 60`
		
		
:>$MAIL_BODY_FILE
echo -e 'Dear All,\n\nPFB FUP Reset results' >> $MAIL_BODY_FILE;
echo -e '\nStart time = ' $START_DATE >> $MAIL_BODY_FILE;
echo -e '\nEnd time = ' $END_DATE >> $MAIL_BODY_FILE;
echo -e "\nDuration = $HOURS hours, $MINS mins and $SECS secs\n" >> $MAIL_BODY_FILE;
echo -e '\nTotal no of records pending for reset:\n' >> $MAIL_BODY_FILE;
cat $OUT_FILE >> $MAIL_BODY_FILE
echo -e '\nThanks and Regards,\nAMS Team' >> $MAIL_BODY_FILE;

mail -s "FUP Reset Status $CURRENT_DATE " $TOM -- -r "ams@hathway.net" < $MAIL_BODY_FILE


exit 0;

