#! /bin/bash
fname="adjustment.log"
fail="1"

cd $PIN_HOME/mso/bin/
echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for bulk_adjustment in $PIN_HOME/mso/bin/"
else
   echo "`date`:Testnap Connection Failed so job skipped for bulk_adjustment in $PIN_HOME/mso/bin/" >> /home/pin/Thirdparty/BRM12/BRM/var/cron_global/global_cron_tab.log
   exit;
fi

account_adj()
{
         mso_mta_job_processor -verbose -object_type /mso_mta_job/bulk_act_adj & > /dev/null
         echo $! > mta_job_act_adj.txt
         pid=$(cat mta_job_act_adj.txt)
         wait $pid
         rm mta_job_act_adj.txt;
}

bill_adj()
{
         mso_mta_job_processor -verbose -object_type /mso_mta_job/bulk_bill_adj & > /dev/null
         echo $! > mta_job_bill_adj.txt
         pid=$(cat mta_job_bill_adj.txt)
         wait $pid
         rm mta_job_bill_adj.txt;
}
adj_log_clean_bup()
{

        `cat /home/pin/Thirdparty/BRM12/BRM/sys/test/$fname >> adj_bkp_curr.log`
        `:>/home/pin/Thirdparty/BRM12/BRM/sys/test/$fname`

}

cd $PIN_HOME/mso/apps/mso_bulk_utilities
echo "current working directory is `pwd`"

echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for bulk_adjustment in PIN_HOME/mso/apps/mso_bulk_utilities"
else
   echo "`date`:Testnap Connection Failed so job skipped for bulk_adjustment in PIN_HOME/mso/apps/mso_bulk_utilities" >> /home/pin/Thirdparty/BRM12/BRM/var/cron_global/global_cron_tab.log
   exit;
fi

if [ -e mta_job_act_adj.txt ]
then
       #echo "file is available";
       pid1=$(cat mta_job_act_adj.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
                account_adj
       fi

else
       echo "file is not available "
       account_adj
fi

#checking the log to get the status of the report

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
                        echo "`date`:bulk_account_adj Job Failed" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                else
                        echo "`date`:bulk_account_adj Job Success" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                fi
        fi


done

adj_log_clean_bup

if [ -e mta_job_bill_adj.txt ]
then
       #echo "file is available";
       pid1=$(cat mta_job_bill_adj.txt)

       if ps -p $pid1 > /dev/null
       then
		 echo " process is already running "
       else
               echo " process is not running "
                bill_adj
       fi

else
       echo "file is not available ";
        bill_adj
fi

#checking the log to get the status of the report

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
                        echo "`date`:bulk_bill_adj Job Failed" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                else
                        echo "`date`:bulk_bill_adj Job Success" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                fi
        fi


done

adj_log_clean_bup
                     
`mv /home/pin/opt/portal/7.5/sys/test/adj_bkp_curr.log $fname`
