##!/bin/bash
DB_STRING=`cat /home/pin/.pass/pin.db`
sqlplus -S ${DB_STRING} <<EOF
EOF

dt=`date "+%Y%b%d"`
echo $dt

#/home/oracle/11g/database/client/11.2.0.4/bin/sqlplus -s ${DB_STRING} << EOF > /home/pin/opt/portal/7.5/mso/bin/dly_cancel_$dt.log

sqlplus -s /nolog << eof >>$OUT_FILE
connect ${DB_STRING}
@/home/pin/Thirdparty/BRM12/BRM/mso/bin/daily_pack_cancel.sql; 

exit;
EOF

CURRENT_DATE=`date`

#mail -s "Insertion for cancellation MTA is complete at $CURRENT_DATE " 'tcs-obrm-lead@hathway.net' -- -r "Cancel-Plan-MTA@hathway.net"  < /home/pin/opt/portal/7.5/mso/bin/dly_cancel_$dt.log

#mail -s "Insertion for cancellation MTA is complete at $CURRENT_DATE " 'tcs-obrm-lead@hathway.net,vineet.tripathi@hathway.net,shailesh.bhoyar@tcs.com,brmappsupport@hathway.net,shreyansh.tiwari2@tcs.com,brm-production-support@hathway.net,premkrishna.sherpa@blackpond.co.in,krishnagopal.kagupati@jhaishna.com' -- -r "Cancel-Plan-MTA@hathway.net"  < /home/pin/Thirdparty/BRM12/BRM/mso/bin/dly_cancel_$dt.log
