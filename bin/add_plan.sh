#! /bin/bash

cd /home/pin/Thirdparty/BRM12/BRM/mso/bin/

echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for add_plan in /home/pin/Thirdparty/BRM12/BRM/mso/bin/"
else
   echo "`date`:Testnap Connection Failed so job skipped for add_plan in /home/pin/Thirdparty/BRM12/BRM/mso/bin/" >> /home/pin/Thirdparty/BRM12/BRM/var/cron_global/global_cron_tab.log
   exit;
fi

add_plan()
{
	 mso_mta_job_processor -verbose -object_type /mso_mta_job/bulk_add_plan & > /dev/null
         echo $! > mta_job_add_plan.txt
         pid=$(cat mta_job_add_plan.txt)
         wait $pid
         rm mta_job_add_plan.txt;
}

cd $PIN_HOME/mso/apps/mso_bulk_utilities
echo "current working directory is `pwd`"

echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for add_plan in /home/pin/Thirdparty/BRM12/BRM/mso/apps/mso_bulk_utilities"
else
   echo "`date`:Testnap Connection Failed so job skipped for add_plan in /home/pin/Thirdparty/BRM12/BRM/mso/apps/mso_bulk_utilities" >> /home/pin/Thirdparty/BRM12/BRM/var/cron_global/global_cron_tab.log
   exit;
fi

if [ -e mta_job_add_plan.txt ]
then
       echo "file is available";
       pid1=$(cat mta_job_add_plan.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
		add_plan
       fi

else
       echo "file is not available ";
	add_plan
fi

#checking the log to get the status of the report
fname="add_plan.log"
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
                        echo "`date`:add_plan job Failed" >> /home/pin/Thirdparty/BRM12/BRM/var/cron_global/global_cron_tab.log
                else
                        echo "`date`:add_plan job Success" >> /home/pin/Thirdparty/BRM12/BRM/var/cron_global/global_cron_tab.log
                fi
        fi


done


