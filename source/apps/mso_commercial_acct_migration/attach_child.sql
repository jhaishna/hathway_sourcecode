set heading off
set feedback off
set echo off
set NEWPAGE NONE
set trimspool ON
set linesize 500
set pagesize 0

select
'r << XX 1'||CHR(10)||
'0 PIN_FLD_POID  POID [0] 0.0.0.1 /group/billing '|| group_poid_id0 || ' 0' ||CHR(10)||
'0 PIN_FLD_MEMBERS      ARRAY [0]' ||CHR(10)||
'1 PIN_FLD_OBJECT   POID [0] 0.0.0.1 /account '|| child_account_obj_id0 || ' 0' ||CHR(10)||
'XX'|| CHR(10)||
'xop PCM_OP_BILL_GROUP_ADD_MEMBER 0 1' || CHR(10)
from rohan.ro;
