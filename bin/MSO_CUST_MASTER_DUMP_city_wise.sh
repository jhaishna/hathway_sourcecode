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
spool $location/MSO_CUST_MASTER_AGRA.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='AGRA';
spool off;

spool $location/MSO_CUST_MASTER_AHMEDABAD.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='AHMEDABAD';
spool off;

spool $location/MSO_CUST_MASTER_ALLAHABAD.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='ALLAHABAD';
spool off;

spool $location/MSO_CUST_MASTER_AURANGABAD.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='AURANGABAD';
spool off;

spool $location/MSO_CUST_MASTER_BAGALKOT.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='BAGALKOT';
spool off;

spool $location/MSO_CUST_MASTER_BANGALORE.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='BANGALORE';
spool off;

spool $location/MSO_CUST_MASTER_BARODA.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='BARODA';
spool off;

spool $location/MSO_CUST_MASTER_BELGAUM.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='BELGAUM';
spool off;

spool $location/MSO_CUST_MASTER_BHOPAL_CITY.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='BHOPAL CITY' or city='BHOPAL';
spool off;

spool $location/MSO_CUST_MASTER_BIJNORE.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='BIJNORE';
spool off;

spool $location/MSO_CUST_MASTER_CHANDIGARH.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='CHANDIGARH';
spool off;

spool $location/MSO_CUST_MASTER_CHENNAI.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='CHENNAI';
spool off;

spool $location/MSO_CUST_MASTER_DELHI.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='DELHI';
spool off;

spool $location/MSO_CUST_MASTER_DHULE.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='DHULE';
spool off;

spool $location/MSO_CUST_MASTER_FARIDABAD.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='FARIDABAD';
spool off;

spool $location/MSO_CUST_MASTER_GAUTAM_BUDH_NAGAR.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='GAUTAM_BUDH_NAGAR';
spool off;

spool $location/MSO_CUST_MASTER_GHAZIABAD.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='GHAZIABAD';
spool off;

spool $location/MSO_CUST_MASTER_GOA.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='GOA';
spool off;

spool $location/MSO_CUST_MASTER_GURGAON.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='GURGAON';
spool off;

spool $location/MSO_CUST_MASTER_HOSHANGABAD_CITY.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='HOSHANGABAD CITY';
spool off;

spool $location/MSO_CUST_MASTER_HYDERABAD.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='HYDERABAD';
spool off;

spool $location/MSO_CUST_MASTER_INDORE_CITY.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='INDORE CITY' or city='INDORE';
spool off;

spool $location/MSO_CUST_MASTER_JABALPUR.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='JABALPUR';
spool off;

spool $location/MSO_CUST_MASTER_JAIPUR_CITY.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='JAIPUR CITY' or city='JAIPUR';
spool off;

spool $location/MSO_CUST_MASTER_JALANDHAR.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='JALANDHAR';
spool off;

spool $location/MSO_CUST_MASTER_JALNA.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='JALNA';
spool off;

spool $location/MSO_CUST_MASTER_KHURJA.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='KHURJA';
spool off;

spool $location/MSO_CUST_MASTER_LATUR.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='LATUR';
spool off;

spool $location/MSO_CUST_MASTER_LUCKNOW.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='LUCKNOW';
spool off;

spool $location/MSO_CUST_MASTER_LUDHIANA.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='LUDHIANA' or city='LUDHIYANA';
spool off;

spool $location/MSO_CUST_MASTER_MADURAI.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='MADURAI';
spool off;

spool $location/MSO_CUST_MASTER_MAINPURI.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='MAINPURI';
spool off;

spool $location/MSO_CUST_MASTER_MUMBAI.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='MUMBAI';
spool off;

spool $location/MSO_CUST_MASTER_MYSORE.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='MYSORE';
spool off;

spool $location/MSO_CUST_MASTER_NANDED.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='NANDED';
spool off;

spool $location/MSO_CUST_MASTER_NASIK.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='NASIK';
spool off;

spool $location/MSO_CUST_MASTER_PUNE.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='PUNE';
spool off;

spool $location/MSO_CUST_MASTER_RAIGAD.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='RAIGAD';
spool off;

spool $location/MSO_CUST_MASTER_RAIPUR_CITY.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='RAIPUR CITY';
spool off;

spool $location/MSO_CUST_MASTER_RAMPUR.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='RAMPUR';
spool off;

spool $location/MSO_CUST_MASTER_SHAJAPUR.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='SHAJAPUR';
spool off;

spool $location/MSO_CUST_MASTER_SURAT.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='SURAT';
spool off;

spool $location/MSO_CUST_MASTER_THANE.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='THANE';
spool off;

spool $location/MSO_CUST_MASTER_TUNDLA.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='TUNDLA';
spool off;

spool $location/MSO_CUST_MASTER_UJJAIN.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='UJJAIN';
spool off;

spool $location/MSO_CUST_MASTER_VARANASI.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='VARANASI';
spool off;

spool $location/MSO_CUST_MASTER_VISAKHAPATNAM.xls
alter session set nls_date_format = 'dd-MON-YY hh24:mi:ss';
select * from MSO_CUST_MASTER_RPT where city='VISAKHAPATNAM';
spool off;
exit;
EOF
