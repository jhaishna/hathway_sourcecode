#!/bin/bash
DB_STRING=`cat /home/pin/.pass/pin.db`
OUT_FILE=sql_output.log
sqlplus -s /nolog << eof >$OUT_FILE
connect ${DB_STRING}
@create_input.sql

eof

