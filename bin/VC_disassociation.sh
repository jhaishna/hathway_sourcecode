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
set linesize 50
set pagesize 30000
SET MARKUP HTML ON SPOOL ON PREFORMAT OFF ENTMAP ON
spool $location/MSO_VC_DISACCOCIATION.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select ser.poid_id0,dt.poid_id0 device_poid,act.poid_id0,decode(salt.rec_id,0,'stb',1,'vc')
from pin.service_alias_list_t salt,pin.service_t ser,pin.device_t dt,pin.account_t act where
dt.device_id=salt.name
and act.poid_id0=ser.account_obj_id0
and ser.poid_id0=salt.obj_id0
and salt.rec_id in (0,1)
--and ser.account_obj_id0=1036204
--and act.status=10103
--and ser.status in (10100,10102)
and ser.status in (10103)
and dt.state_id=2
and dt.device_id in (select vc_id from MIG_WITH_STATC);
spool off;
exit;
EOF
