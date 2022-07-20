#!/bin/sh
#source .bash_profile
user="$1"
pass="$2"
SID="$3"
location="$4"
#rm $location/execution_logs
sqlplus -S $user/$pass@$SID <<EOF
set termout off
set feedback off
set echo off
set linesize 500
set pagesize 0
set newpage 0
spool $location/MSO_LCO_CRF.csv
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
alter session force parallel dml parallel 8;
SELECT
act.account_no||','||
replace(replace(replace(ant.company,' - ','-'),' ','-' ),'/','') ||','||
replace(replace(ant.address,chr(10),' '),',',' ')||','||
ant.city||','||
(SELECT COUNT(DISTINCT ser.poid_id0)
FROM service_t ser,
profile_t p,
PROFILE_CUSTOMER_CARE_T pcc
WHERE ser.account_obj_id0=p.account_obj_id0
AND p.poid_id0 =pcc.OBJ_ID0
AND pcc.LCO_OBJ_ID0 =act.POID_ID0
AND ser.status =10100 )||','||
(SELECT COUNT(DISTINCT ser.poid_id0)
FROM service_t ser,
profile_t p,
PROFILE_CUSTOMER_CARE_T pcc
WHERE ser.account_obj_id0=p.account_obj_id0
AND p.poid_id0 =pcc.OBJ_ID0
AND pcc.LCO_OBJ_ID0 =act.POID_ID0
AND ser.status =10102 ) ||','||
(SELECT COUNT(DISTINCT ser.poid_id0)
FROM service_t ser,
profile_t p,
PROFILE_CUSTOMER_CARE_T pcc
WHERE ser.account_obj_id0=p.account_obj_id0
AND p.poid_id0 =pcc.OBJ_ID0
AND pcc.LCO_OBJ_ID0 =act.POID_ID0
AND ser.status =10103 ) ||','||
(SELECT COUNT(sci.CRF_FLAG)
FROM service_t ser,
profile_t p,
PROFILE_CUSTOMER_CARE_T pcc,
SERVICE_CATV_INFO_T sci
WHERE ser.account_obj_id0=p.account_obj_id0
AND p.poid_id0 =pcc.OBJ_ID0
AND pcc.LCO_OBJ_ID0 =act.POID_ID0
AND sci.obj_id0 =ser.poid_id0
AND sci.crf_flag =1 ) 
FROM account_t act,
account_nameinfo_t ant
WHERE act.BUSINESS_TYPE like '13%'
AND ant.OBJ_ID0 =act.POID_ID0
AND ant.REC_ID =1;
spool off;
EOF
