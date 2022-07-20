#!/bin/bash

DB_USER=pindev5
DB_PSSWD=pindev5
DB_NAME=DEVBRM

###################################################################
# Function will be called once usage error happened for the script
###################################################################
UsageError()
{
echo "Usage: mso_pymt_autopay.sh -f <> -v" "\n"
echo "	-f Mandatory option. Lco file name needs to be provided"; 
echo "	-v Optional option. MTA will run on verbose mode";
exit 1; 
}

#######################################################
# main function
#######################################################
if [ $# -lt 2 ]
then
	UsageError;
fi

I=0;
while [ $# -ne 0 ]
do
# assign 1st arg to variable ARG1 at
# first iteration. To ARG2 in second
# iteration, etc
 eval ARG${I}=$1
 I=`expr $I + 1`
 shift
done

if [ x$ARG0 = x'-f' ]; then
	fileName=$ARG1;
else
	if [ x$ARG1 = x'-f' ]; then
		fileName=$ARG2;
	else
		UsageError;
	fi
fi

ref_id=`date +%s`

#Empty existing list if any
:>lco_member_account_list.dat

#Read each line, and execute select statement,
# to get the LCO member accounts.
exec<$fileName
while read line
do
$ORACLE_HOME/bin/sqlplus -s $DB_USER/$DB_PSSWD@$DB_NAME >> lco_member_account_list.dat <<!
SET TRIMSPOOL ON
SET LIN 200
SET FEEDBACK OFF
set heading off
set verify off
set PAGESIZE 0
set TERMOUT off
select 
	profile_t.ACCOUNT_OBJ_ID0||'|'||
	(select ACCOUNT_NO from account_t where poid_id0 = profile_t.ACCOUNT_OBJ_ID0)||'|'||
	profile_customer_care_t.LCO_OBJ_ID0||'|'||
	(select ACCOUNT_NO from account_t where poid_id0 = profile_customer_care_t.LCO_OBJ_ID0)||'|'||
	'$ref_id'||'|'||
	get_current_cr_balance(profile_t.ACCOUNT_OBJ_ID0,356)
from profile_t join profile_customer_care_t on profile_t.POID_ID0 = profile_customer_care_t.OBJ_ID0
where profile_customer_care_t.LCO_OBJ_ID0 = '$line'
	and get_current_cr_balance(profile_t.ACCOUNT_OBJ_ID0,356) > 0 order by profile_customer_care_t.LCO_OBJ_ID0;
/
!
done

mso_pymt_autopay -f lco_member_account_list.dat -v
