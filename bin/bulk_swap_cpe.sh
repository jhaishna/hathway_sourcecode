#! /bin/bash
## Modfified on 18-MAY-2015 to remove PID check, now lock file mechanism is used
cd $PIN_HOME/mso/bin/
echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for bulk_writeoff_cpe in PIN_HOME/mso/bin/"
else
   echo "`date`:Testnap Connection Failed so job skipped for bulk_writeoff_cpe in PIN_HOME/mso/bin/" >> /home/pin/Thirdparty/BRM12/BRM//var/cron_global/global_cron_tab.log
   exit;
fi

prog_name='SWAP'
logfile='/home/pin/Thirdparty/BRM12/BRM/mso/apps/mso_bulk_audit/swap.log'

sh /home/pin/Thirdparty/BRM12/BRM/mso/apps/mso_bulk_audit/prerun.sh $prog_name > $logfile


bulk_swap_cpe()
{
         echo "runnng" > mta_job_bulk_swap_cpe.txt
	 mso_mta_job_processor -verbose -object_type /mso_mta_job/bulk_swap_cpe

#         pid=$(cat mta_job_bulk_swap_cpe.txt)
#         wait $pid
         rm mta_job_bulk_swap_cpe.txt;
}

cd $PIN_HOME/mso/apps/mso_bulk_inventory
echo "current working directory is `pwd`"

echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for bulk_writeoff_cpe in PIN_HOME/mso/apps/mso_bulk_inventory"
else
   echo "`date`:Testnap Connection Failed so job skipped for bulk_writeoff_cpe in PIN_HOME/mso/apps/mso_bulk_inventory" >> /home/pin/Thirdparty/BRM12/BRM/var/cron_global/global_cron_tab.log
   exit;
fi

if [ -e mta_job_bulk_swap_cpe.txt ]
then
#       echo "file is available";
#       pid1=$(cat mta_job_bulk_swap_cpe.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#		bulk_swap_cpe
#       fi
       echo "process is already running "
else
       echo "Executing the process bulk_swap_cpe"
	bulk_swap_cpe
fi

#checking the log to get the status of the report
fname="bulk_swap_cpe.log"
fail="1"

exec < /home/pin/Thirdparty/BRM12/BRM/mso/apps/mso_bulk_inventory/$fname


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
                        echo "`date`:bulk_swap_cpe Job Failed" >> /home/pin/Thirdparty/BRM12/BRM/var/cron_global/global_cron_tab.log
                else
                        echo "`date`:bulk_swap_cpe Job Success" >> /home/pin/Thirdparty/BRM12/BRM/var/cron_global/global_cron_tab.log
                fi
        fi


done

DB_STRING=`cat /home/pin/.pass/pin.db`
sqlplus -s ${DB_STRING} << EOF >> $logfile
@/home/pin/Thirdparty/BRM12/BRM/mso/apps/mso_bulk_audit/postrun.sql '$prog_name'
exit;
EOF

