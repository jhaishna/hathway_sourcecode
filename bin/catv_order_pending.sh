DB_STRING=`cat /home/pin/.pass/pin.db`

dt=`date "+%Y%b%d"`
echo $dt

/home/oracle/11g/database/client/11.2.0.4/bin/sqlplus -s ${DB_STRING} << EOF > /home/pin/catv_order_pending_$dt.log

@/home/pin/opt/portal/7.5/mso/bin/catv_order_pending.sql;

exit;
EOF
exit;
