#! /bin/bash
## Modfified on 18-MAY-2015 to remove PID check, now lock file mechanism is used
bulk_topup()
{
         echo "runnng" > mta_job_bulk_topup.txt
	 mso_mta_job_processor -object_type /mso_mta_job/bulk_topup

#         pid=$(cat mta_job_bulk_topup.txt)
#         wait $pid
         rm mta_job_bulk_topup.txt;
}

cd $PIN_HOME/mso/apps/mso_bulk_plan
echo "current working directory is `pwd`"

if [ -e mta_job_bulk_topup.txt ]
then
        echo " process is already running "
#       pid1=$(cat mta_job_bulk_topup.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#		bulk_topup
#       fi

else
       echo "Executing bulk_topup"
	bulk_topup
fi

