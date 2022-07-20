#! /bin/bash
cd $PIN_HOME/mso/bin/
echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for bulk_crf_change in PIN_HOME/mso/bin/"
else
   echo "`date`:Testnap Connection Failed so job skipped for bulk_crf_change in PIN_HOME/mso/bin/" >> /home/pin/Thirdparty/BRM12/BRM/var/cron_global/global_cron_tab.log
   exit;
fi


prog_name='CRF'
logfile='/home/pin/Thirdparty/BRM12/BRM/mso/apps/mso_bulk_audit/crf.log'

sh /home/pin/Thirdparty/BRM12/BRM/mso/apps/mso_bulk_audit/prerun.sh $prog_name > $logfile

 
bulk_crf_change()
{
         mso_mta_job_processor -verbose  -object_type  /mso_mta_job/bulk_crf & > /dev/null
         echo $! > mta_job_bulk_crf.txt
         pid=$(cat mta_job_bulk_crf.txt)
         wait $pid
         rm mta_job_bulk_crf.txt;
}

cd $PIN_HOME/mso/apps/mso_bulk_utilities

echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for bulk_crf_change in PIN_HOME/mso/apps/mso_bulk_utilities"
else
   echo "`date`:Testnap Connection Failed so job skipped for bulk_crf_change in PIN_HOME/mso/apps/mso_bulk_utilities" >> /home/pin/Thirdparty/BRM12/BRM/var/cron_global/global_cron_tab.log
   exit;
fi

echo "current working directory is `pwd`"

if [ -e mta_job_bulk_crf.txt ]
then
       echo "file is available";
       pid1=$(mta_job_bulk_crf.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
                bulk_crf_change
       fi
else
       echo "file is not available ";
        bulk_crf_change
fi

#checking the log to get the status of the report
fname="bulk_crf_change.log"
fail="1"

exec < /home/pin/Thirdparty/BRM12/BRM/sys/test/$fname


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
                        echo "`date`:bulk_crf_change job Failed" >> /home/pin/Thirdparty/BRM12/BRM/var/cron_global/global_cron_tab.log
                else
                        echo "`date`:bulk_crf_change job Success" >> /home/pin/Thirdparty/BRM12/BRM/var/cron_global/global_cron_tab.log
                fi
        fi


done

DB_STRING=`cat /home/pin/.pass/pin.db`
sqlplus -s ${DB_STRING} << EOF >> $logfile
@/home/pin/Thirdparty/BRM12/BRM/mso/apps/mso_bulk_audit/postrun.sql '$prog_name'
exit;
EOF

