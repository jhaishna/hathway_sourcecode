#! /bin/bash
`:>/home/pin/opt/portal/7.5/mso/apps/mso_bulk_notification/mso_prepaid_lim_usage_notif.log`
cd /home/pin/opt/portal/7.5/mso/bin/
echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for mso_prepaid_lim_usage_notif in /home/pin/opt/portal/7.5/mso/bin/"
else
   echo "`date`:Testnap Connection Failed so job skipped for mso_prepaid_lim_usage_notif in /home/pin/opt/portal/7.5/mso/bin/" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
   exit;
fi


run_mso_prepaid_lim_usage_notif()
{
        echo "runnng" > mso_prepaid_lim_usage_notif.txt
	mso_prepaid_lim_usage_notif -verbose
	rm mso_prepaid_lim_usage_notif.txt;
}

cd $PIN_HOME/mso/apps/mso_bulk_notification
echo "current working directory is `pwd`"

echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for mso_prepaid_lim_usage_notif in PIN_HOME/mso/apps/mso_bulk_notification"
else
   echo "`date`:Testnap Connection Failed so job skipped for mso_prepaid_lim_usage_notif in PIN_HOME/mso/apps/mso_bulk_notification" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
   exit;
fi


if [ -e mso_prepaid_lim_usage_notif.txt ]
then
               echo " process is already running "

else
       echo "Executing the process mso_prepaid_lim_usage_notif"
       run_mso_prepaid_lim_usage_notif
fi

#checking the log to get the status of the report
fname="mso_prepaid_lim_usage_notif.log"
fail="1"

exec < /home/pin/opt/portal/7.5/mso/apps/mso_bulk_notification/$fname


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
                        echo "`date`:mso_prepaid_lim_usage_notif Job Failed" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                else
                        echo "`date`:mso_prepaid_lim_usage_notif Job Success" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                fi
        fi


done

