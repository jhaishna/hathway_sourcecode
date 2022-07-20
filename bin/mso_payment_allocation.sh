#! /bin/bash
cd /home/pin/opt/portal/7.5/mso/bin/
echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for pymt_allocation in /home/pin/opt/portal/7.5/mso/bin/"
else
   echo "`date`:Testnap Connection Failed so job skipped for pymt_allocation in /home/pin/opt/portal/7.5/mso/bin/" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
   exit;
fi

payment_allocation()
{
         mso_pymt_allocation_with_segment -object_type /payment & > /dev/null
         echo $! > payment_allocation.txt
         pid=$(cat payment_allocation.txt)
         wait $pid
         rm payment_allocation.txt;
}

cd $PIN_HOME/mso/apps/mso_pymt_alloctaion_scehduled

echo "current working directory is `pwd`"

echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for pymt_allocation in PIN_HOME/mso/apps/mso_pymt_alloctaion_scehduled"
else
   echo "`date`:Testnap Connection Failed so job skipped for pymt_allocation in PIN_HOME/mso/apps/mso_pymt_alloctaion_scehduled" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
   exit;
fi

if [ -e payment_allocation.txt ]
then
       echo "file is available";
       pid1=$(cat payment_allocation.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
                payment_allocation
       fi
else
       echo "file is not available ";
        payment_allocation
fi

#checking the log to get the status of the report
fname="mso_payment_allocation.log"
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
                        echo "`date`:bulk_pymt_allocation Job Failed" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                else
                        echo "`date`:bulk_pymt_allocation Job Success" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                fi
        fi


done

