#! /bin/bash
## Modfified on 18-MAY-2015 to remove PID check, now lock file mechanism is used
cd /home/pin/opt/portal/7.5/mso/bin/
echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for device_pair in /home/pin/opt/portal/7.5/mso/bin/"
else
   echo "`date`:Testnap Connection Failed so job skipped for device_pair in /home/pin/opt/portal/7.5/mso/bin/" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
   exit;
fi


device_pair()
{
         echo "runnng" > mta_job_device_pair.txt
	 mso_mta_job_processor -verbose -object_type /mso_mta_job/device_pair

#         pid=$(cat mta_job_device_pair.txt)
#         wait $pid
         rm mta_job_device_pair.txt;
}

cd $PIN_HOME/mso/apps/mso_bulk_utilities

echo "current working directory is `pwd`"

echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for device_pair in PIN_HOME/mso/apps/mso_bulk_utilities"
else
   echo "`date`:Testnap Connection Failed so job skipped for device_pair in PIN_HOME/mso/apps/mso_bulk_utilities" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
   exit;
fi


prog_name='REF_LOAD'
logfile='/home/pin/opt/portal/7.5/mso/apps/mso_bulk_audit/refund_load.log'

sh /home/pin/opt/portal/7.5/mso/apps/mso_bulk_audit/prerun.sh $prog_name > $logfile


if [ -e mta_job_device_pair.txt ]
then
#       echo "file is available";
#       pid1=$(cat mta_job_device_pair.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#		device_pair
#       fi
       echo "process is already running "
else
       echo "Executing the process device_pair"
	device_pair
fi

#checking the log to get the status of the report
fname="device_pair.log"
fail="1"

exec < /home/pin/opt/portal/7.5/mso/apps/mso_bulk_utilities/$fname


while read line
do

        if [[ $line == *"data errors"* ]]
        then
                orig_value=`echo $line | cut -d '=' -f2`
                orig_value=${orig_value// }
                if [ "$orig_value" == "0." ]
                then
                        result=0
                fi

        fi
        if [[ $line == *"number of errors"* ]]
        then
                orig_value=`echo $line | cut -d '=' -f2`
                orig_value=${orig_value// }
                if [ "$orig_value" == "1." ]
                then
                        result=1
                fi

        fi

        if [[ $line == *"number of errors"* ]]
        then
                if [[ $result -eq $fail ]];
                then
                        echo "`date`:device_pair Job Failed" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                else
                        echo "`date`:device_pair Job Success" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                fi
        fi


done

#sh /home/pin/opt/portal/7.5/mso/apps/mso_bulk_audit/postrun.sh $prog_name > $logfile
