#!/bin/bash

conn_str=pin/pin123@$ORACLE_SID

OUT_FILE=tmp.out
sqlplus -s /nolog << eof >$OUT_FILE
connect ${conn_str}
@truncate_data.sql
eof

sqlldr control=ctl_file.ctl data=../bb_plan_report.csv USERID=pin/pin123@$ORACLE_SID

conn_str=pin/pin123@$ORACLE_SID

sqlplus -s /nolog << eof >>$OUT_FILE
connect ${conn_str}
@create_report.sql
eof

