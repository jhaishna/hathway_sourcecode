#! /bin/bash
cd $PIN_HOME/mso/bin/
echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for bulk static ip waive off PIN_HOME/mso/bin"
else
   echo "`date`:Testnap Connection Failed so job skipped for bulk static ip waive off PIN_HOME/mso/bin" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
   exit;
fi

prog_name='SUSPEND'
logfile='/home/pin/opt/portal/7.5/mso/apps/mso_bulk_audit/bulk_waiveoff.log'

sh /home/pin/opt/portal/7.5/mso/apps/mso_bulk_audit/prerun.sh $prog_name > $logfile


waive_off()
{
         mso_mta_job_processor -verbose -object_type /mso_mta_job/bulk_static_ip_waive_off & > /dev/null
         echo $! > mta_job_bulk_waiveoff.txt
         pid=$(cat mta_job_bulk_waiveoff.txt)
         wait $pid
         rm mta_job_bulk_waiveoff.txt;
}

cd $PIN_HOME/mso/apps/mso_bulk_utilities_SI
echo "current working directory is `pwd`"
echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for bulk static ip waive off PIN_HOME/mso/apps/mso_bulk_utilities_SI"
else
   echo "`date`:Testnap Connection Failed so job skipped for bulk static ip waive off PIN_HOME/mso/apps/mso_bulk_utilities_SI" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
   exit;
fi

if [ -e mta_job_bulk_waiveoff.txt ]
then
       echo "file is available";
       pid1=$(cat mta_job_bulk_waiveoff.txt.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
		waive_off
       fi

else
       echo "file is not available ";
	waive_off
fi

DB_STRING=`cat /home/pin/.pass/pin.db`
sqlplus -s ${DB_STRING} << EOF >> $logfile
@/home/pin/opt/portal/7.5/mso/apps/mso_bulk_audit/postrun.sql '$prog_name'
exit;
EOF

