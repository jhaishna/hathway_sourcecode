#! /bin/bash

cd $PIN_HOME/mso/apps/mso_bulk_inventory
echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for grv_uploader in PIN_HOME/mso/bin/"
else
   echo "`date`:Testnap Connection Failed so job skipped for grv_uploader in PIN_HOME/mso/bin/" >> /home/pin/Thirdparty/BRM12/BRM/var/cron_global/global_cron_tab.log
   exit;
fi

prog_name='GRV_UPLOAD'
logfile='/home/pin/Thirdparty/BRM12/BRM/mso/apps/mso_bulk_audit/grv_upload.log'

sh /home/pin/Thirdparty/BRM12/BRM/mso/apps/mso_bulk_audit/prerun.sh $prog_name > $logfile



grv_uploader()
{
         echo "runnng" > mta_job_grv_uploader.txt
	 mso_mta_job_processor -verbose -object_type /mso_mta_job/grv_uploader

#         pid=$(cat mta_job_grv_uploader.txt)
#         wait $pid
         rm mta_job_grv_uploader.txt;
}

cd $PIN_HOME/mso/apps/mso_bulk_inventory
echo "current working directory is `pwd`"

echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for grv_uploader in PIN_HOME/mso/apps/mso_bulk_inventory"
else
   echo "`date`:Testnap Connection Failed so job skipped for grv_uploader in PIN_HOME/mso/apps/mso_bulk_inventory" >> /home/pin/Thirdparty/BRM12/BRM/var/cron_global/global_cron_tab.log
   exit;
fi

if [ -e mta_job_grv_uploader.txt ]
then
#       echo "file is available";
#       pid1=$(cat mta_job_grv_uploader.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#		grv_uploader
#       fi
       echo "process is already running "
else
       echo "Executing the process grv_uploader"
	grv_uploader
fi


#checking the log to get the status of the report
fname="bulk_grv_uploader.log"
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
                        echo "`date`:grv_uploader Job Failed" >> /home/pin/Thirdparty/BRM12/BRM/var/cron_global/global_cron_tab.log
                else
                        echo "`date`:grv_uploader Job Success" >> /home/pin/Thirdparty/BRM12/BRM/var/cron_global/global_cron_tab.log
                fi
        fi


done

DB_STRING=`cat /home/pin/.pass/pin.db`
sqlplus -s ${DB_STRING} << EOF >> $logfile
@/home/pin/Thirdparty/BRM12/BRM/mso/apps/mso_bulk_audit/postrun.sql '$prog_name'
exit;
EOF

