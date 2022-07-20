#!/bin/bash

conn_str=pin/GeDFfvkj@PRODBRM

OUT_FILE=sql_output.nap
sqlplus -s /nolog << eof >$OUT_FILE
connect ${conn_str}
@remove_member.sql
eof

