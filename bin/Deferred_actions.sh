#! /bin/bash
cd /home/pin/opt/portal/7.5/mso/bin/
echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for pin_deferred_act in /home/pin/opt/portal/7.5/mso/bin/"
else
   echo "`date`:Testnap Connection Failed so job skipped for pin_deferred_act in /home/pin/opt/portal/7.5/mso/bin/" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
   exit;
fi

actions()
{
	 pin_deferred_act -status pending -verbose & >/dev/null
         echo $! > deferred_action.txt
         pid=$(cat deferred_action.txt)
         wait $pid
         rm deferred_action.txt;
}

cd $PIN_HOME/apps/pin_billd
echo "current working directory is `pwd`"

echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for pin_deferred_act in PIN_HOME/apps/pin_billd"
else
   echo "`date`:Testnap Connection Failed so job skipped for pin_deferred_act in PIN_HOME/apps/pin_billd" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
   exit;
fi

if [ -e deferred_action.txt ]
then
       echo "file is available";
       pid1=$(cat deferred_action.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
		actions
       fi

else
       echo "file is not available ";
	actions
fi

#checking the log to get the status of the report
fname="deferred_actions.log"
fail="1"

exec < /home/pin/opt/portal/7.5/sys/test/$fname


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
                        echo "`date`:deferred_actions Job Failed" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                else
                        echo "`date`:deferred_actions Job Success" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                fi
        fi


done
