#! /bin/bash
## Created on 19-MAY-2015
`:>/home/pin/opt/portal/7.5/mso/apps/mso_collections_notifications/mso_collections_notifications.log`
cd /home/pin/opt/portal/7.5/mso/bin/
echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for collection_notification in /home/pin/opt/portal/7.5/mso/bin/"
else
   echo "`date`:Testnap Connection Failed so job skipped for collection_notification in /home/pin/opt/portal/7.5/mso/bin/" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
   exit;
fi

collection_notification()
{
         echo "runnng" > collection_notification.txt
	 mso_collections_notification -verbose -object_type /mso_mta_job/collections/reminder_1
	 mso_collections_notification -verbose -object_type /mso_mta_job/collections/reminder_2
         rm collection_notification.txt;
}

cd $PIN_HOME/mso/apps/mso_bulk_notification
echo "current working directory is `pwd`"

echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for collection_notification in PIN_HOME/mso/apps/mso_bulk_notification"
else
   echo "`date`:Testnap Connection Failed so job skipped for collection_notification in PIN_HOME/mso/apps/mso_bulk_notification" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
   exit;
fi


if [ -e collection_notification.txt ]
then
       echo "process is already running "
else
       echo "Executing the process collection_notification"
	collection_notification
fi

#checking the log to get the status of the report
fname="mso_collections_notifications.log"
fail="1"

exec < /home/pin/opt/portal/7.5/mso/apps/mso_collections_notifications/$fname


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
                        echo "`date`:collection_notification Job Failed" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                else
                        echo "`date`:collection_notification Job Success" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                fi
        fi


done
