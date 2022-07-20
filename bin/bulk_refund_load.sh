#! /bin/bash
## Modfified on 18-MAY-2015 to remove PID check, now lock file mechanism is used
pid=`echo $$`
out1=`ps -ef|grep bulk_refund_load.sh|grep -v -e grep -e vim -e $pid`
out=`ps -ef|grep bulk_refund_load.sh|grep -v -e grep -e vim -e $pid|wc -l`

if [ $out -eq 0 ]
then
        echo "starting process"
else
        echo "exiting...as process is already running : $out1"
        exit
fi



cd $PIN_HOME/mso/bin/
echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for bulk_refund_loAD IN PIN_HOME/mso/bin/"
else
   echo "`date`:Testnap Connection Failed so job skipped for bulk_refund_load in PIN_HOME/mso/bin/" >> /home/pin/Thirdparty/BRM12/BRM/var/cron_global/global_cron_tab.log
   exit;
fi


bulk_refund_load()
{
         echo "runnng" > mta_job_bulk_refund_load.txt
	 mso_mta_job_processor -verbose -object_type /mso_mta_job/bulk_refund_load

#         pid=$(cat mta_job_bulk_refund_load.txt)
#         wait $pid
         rm mta_job_bulk_refund_load.txt;
}

cd $PIN_HOME/mso/apps/mso_bulk_AR
echo "current working directory is `pwd`"

echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for bulk_refund_load in PIN_HOME/mso/apps/mso_bulk_AR"
else
   echo "`date`:Testnap Connection Failed so job skipped for bulk_refund_load in PIN_HOME/mso/apps/mso_bulk_AR" >> /home/pin/Thirdparty/BRM12/BRM/var/cron_global/global_cron_tab.log
   exit;
fi


prog_name='REF_LOAD'
logfile='/home/pin/Thirdparty/BRM12/BRM/mso/apps/mso_bulk_audit/refund_load.log'

sh /home/pin/Thirdparty/BRM12/BRM/mso/apps/mso_bulk_audit/prerun.sh $prog_name > $logfile


if [ -e mta_job_bulk_refund_load.txt ]
then
#       echo "file is available";
#       pid1=$(cat mta_job_bulk_refund_load.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#		bulk_refund_load
#       fi
       echo "process is already running "
else
       echo "Executing the process bulk_refund_load"
	bulk_refund_load
fi

#checking the log to get the status of the report
fname="bulk_refund_load.log"
fail="1"

exec < /home/pin/Thirdparty/BRM12/BRM/mso/apps/mso_bulk_AR/$fname


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
                        echo "`date`:bulk_refund_load Job Failed" >> /home/pin/Thirdparty/BRM12/BRM/var/cron_global/global_cron_tab.log
                else
                        echo "`date`:bulk_refund_load Job Success" >> /home/pin/Thirdparty/BRM12/BRM/var/cron_global/global_cron_tab.log
                fi
        fi


done

sh /home/pin/Thirdparty/BRM12/BRM/mso/apps/mso_bulk_audit/postrun.sh $prog_name > $logfile
