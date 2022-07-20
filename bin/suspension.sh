#! /bin/bash
cd /home/pin/Thirdparty/BRM12/BRM/mso/bin/
echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for suspension /home/pin/Thirdparty/BRM12/BRM/mso/bin"
else
   echo "`date`:Testnap Connection Failed so job skipped for suspension /home/pin/Thirdparty/BRM12/BRM/mso/bin" >> /home/pin/Thirdparty/BRM12/BRM/var/cron_global/global_cron_tab.log
   exit;
fi

prog_name='SUSPEND'
logfile='/home/pin/Thirdparty/BRM12/BRM/mso/apps/mso_bulk_audit/suspend.log'

sh /home/pin/Thirdparty/BRM12/BRM/mso/apps/mso_bulk_audit/prerun.sh $prog_name > $logfile


suspension()
{
         mso_mta_job_processor -verbose -object_type /mso_mta_job/bulk_suspension & > /dev/null
         echo $! > mta_job_service_suspension.txt
         pid=$(cat mta_job_service_suspension.txt)
         wait $pid
         rm mta_job_service_suspension.txt;
}

cd $PIN_HOME/mso/apps/mso_bulk_utilities
echo "current working directory is `pwd`"
echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for suspension PIN_HOME/mso/apps/mso_bulk_utilities"
else
   echo "`date`:Testnap Connection Failed so job skipped for suspension PIN_HOME/mso/apps/mso_bulk_utilities" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
   exit;
fi

if [ -e mta_job_service_suspension.txt ]
then
       echo "file is available";
       pid1=$(cat mta_job_service_suspension.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
		suspension
       fi

else
       echo "file is not available ";
	suspension
fi

DB_STRING=`cat /home/pin/.pass/pin.db`
sqlplus -s ${DB_STRING} << EOF >> $logfile
@/home/pin/Thirdparty/BRM12/BRM/mso/apps/mso_bulk_audit/postrun.sql '$prog_name'
exit;
EOF

