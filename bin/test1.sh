DB_STRING=`cat ~/.pass/pin.db`
dt=`date "+%Y%b%d_%H%M%S"`
echo $dt

/home/oracle/11g/database/client/11.2.0.4/bin/sqlplus -s ${DB_STRING} << EOF > /home/pin/opt/portal/7.5/mso/bin/insert_$dt.log

@/home/pin/opt/portal/7.5/mso/bin/test_p.sql;

exit;
EOF
dt=`date "+%Y%b%d"`
echo $dt

exit;

