#! /bin/bash
`:>/home/pin/opt/portal/7.5/mso/apps/pin_clean_rsvns/pin_clean_rsvns.log`
cd /home/pin/opt/portal/7.5/mso/bin/
echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for pin_clean_rsvns in /home/pin/opt/portal/7.5/mso/bin/"
else
   echo "`date`:Testnap Connection Failed so job skipped for pin_clean_rsvns in /home/pin/opt/portal/7.5/mso/bin/" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
   exit;
fi


run_pin_clean_rsvns()
{
	echo "runnng" > mso_run_pin_clean_rsvns.txt
	#Run  pin_clean_rsvns
	pin_clean_rsvns -verbose -object 1 -expiration_time 48
	rm mso_run_pin_clean_rsvns.txt
}

cd $PIN_HOME/apps/pin_clean_rsvns
echo "current working directory is `pwd`"

echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for pin_clean_rsvns in PIN_HOME/apps/pin_clean_rsvns"
else
   echo "`date`:Testnap Connection Failed so job skipped for pin_clean_rsvns in PIN_HOME/apps/pin_clean_rsvns" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
   exit;
fi


if [ -e mso_run_pin_clean_rsvns.txt ]
then
	echo `date`
	echo "process is already running "
else
	echo `date`
	echo "Executing the process pin_clean_rsvsn"
	run_pin_clean_rsvns
	echo `date`
fi

#checking the log to get the status of the report
fname="pin_clean_rsvns.log"
fail="1"

exec < /home/pin/opt/portal/7.5/mso/apps/pin_clean_rsvns/$fname


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
                        echo "`date`:pin_clean_rsvns Job Failed" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                else
                        echo "`date`:pin_clean_rsvns Job Success" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                fi
        fi


done

