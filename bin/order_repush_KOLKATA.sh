#!/bin/sh
#source .bash_profile
user="$1"
pass="$2"
SID="$3"
#rm $location/execution_logs
sqlplus -S $user/$pass@$SID <<EOF
UPDATE mso_prov_order_t SET status  = 1 WHERE status = 10 AND poid_id0 IN (select tbl.poid_id0 from (SELECT p.poid_id0 FROM mso_prov_order_t p , account_nameinfo_t a WHERE p.status  = 10 and p.account_obj_id0 = a.obj_id0 and a.city in ('KOLKATA') and a.rec_id = 1 and p.action like '%NDS' order by p.order_id asc )tbl where rownum <=50000 ) ;
commit;
spool off;
exit;
EOF
