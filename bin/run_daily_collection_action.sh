#! /bin/bash
## Created on 19-MAY-2015
`:>/home/pin/opt/portal/7.5/apps/pin_collections/run_daily_collection_action.log`
cd /home/pin/opt/portal/7.5/mso/bin/
echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for collection_action in /home/pin/opt/portal/7.5/mso/bin/"
else
   echo "`date`:Testnap Connection Failed so job skipped for collection_action in /home/pin/opt/portal/7.5/mso/bin/" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
   exit;
fi

collection_action()
{
         echo "runnng" > collection_action.txt
	 pin_collections_process -verbose
         rm collection_action.txt;
}

cd $PIN_HOME/apps/pin_collections
echo "current working directory is `pwd`"

echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for collection_action in PIN_HOME/apps/pin_collections"
else
   echo "`date`:Testnap Connection Failed so job skipped for collection_action in PIN_HOME/apps/pin_collections" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
   exit;
fi


if [ -e collection_action.txt ]
then
       echo "process is already running "
else
       echo "Executing the process deferred_action"
	collection_action
fi

#checking the log to get the status of the report
fname="run_daily_collection_action.log"
fail="1"

exec < /home/pin/opt/portal/7.5/apps/pin_collections/$fname


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
                        echo "`date`:daily_collection_action Job Failed" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                else
                        echo "`date`:daily_collection_action Job Success" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                fi
        fi


done
