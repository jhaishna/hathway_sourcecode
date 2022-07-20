DB_STRING="rohan/rohan1980@prodbrm"
dt=`date "+%Y%b%d"`
echo $dt

/home/oracle/11g/database/client/11.2.0.4/bin/sqlplus -s ${DB_STRING} << EOF > /home/pin/catv_order_pending_$dt.log

@/home/pin/opt/portal/7.5/mso/bin/pdf_part2.sql;

exit;
EOF
dt=`date "+%Y%b%d"`
echo $dt

exit;

