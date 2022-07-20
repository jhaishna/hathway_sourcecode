#! /bin/bash

cd /home/pin/opt/portal/7.5/mso/bin/
echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for bulk entity change in /home/pin/opt/portal/7.5/mso/bin"
else
   echo "`date`:Testnap Connection Failed so job skipped for bulk entity change in /home/pin/opt/portal/7.5/mso/bin" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
   exit;
fi

bulk_entity_change()
{
         mso_mta_job_processor -verbose -object_type /mso_mta_job/bulk_hierarchy & > /dev/null
         echo $! > mta_job_bulk_hierarchy.txt
         pid=$(cat mta_job_bulk_hierarchy.txt)
         wait $pid
         rm mta_job_bulk_hierarchy.txt;
}

cd $PIN_HOME/mso/apps/mso_bulk_utilities

echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for bulk entity change in PIN_HOME/mso/apps/mso_bulk_utilities"
else
   echo "`date`:Testnap Connection Failed so job skipped for bulk entity change in PIN_HOME/mso/apps/mso_bulk_utilities" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
   exit;
fi

echo "current working directory is `pwd`"

if [ -e mta_job_bulk_hierarchy.txt ]
then
       echo "file is available";
       pid1=$(cat mta_job_bulk_hierarchy.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
                bulk_entity_change
       fi
else
       echo "file is not available ";
        bulk_entity_change
fi

#checking the log to get the status of the report
fname="bulk_entity_change.log"
fail="1"

exec < /home/pin/opt/portal/7.5/sys/test/$fname


while read line
do

        if [[ $line == *"data errors"* ]]
        then
                orig_value=`echo $line | cut -d '=' -f2`
                orig_value=${orig_value// }
                #echo "*******$orig_value******"
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
                        echo "`date`:bulk_entity_change Job Failed" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                else
                        echo "`date`:bulk_entity_change Job Success" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                fi
        fi


done
