#! /bin/bash
## Modfified on 18-MAY-2015 to remove PID check, now lock file mechanism is used

bulk_renew()
{
         echo "runnng" > mta_job_bulk_renew.txt
	 mso_mta_job_processor -object_type /mso_mta_job/bulk_renew

#         pid=$(cat mta_job_bulk_renew.txt)
#         wait $pid
         rm mta_job_bulk_renew.txt;
}

cd $PIN_HOME/mso/apps/mso_bulk_plan
echo "current working directory is `pwd`"

if [ -e mta_job_bulk_renew.txt ]
then
#       echo "file is available";
#       pid1=$(cat mta_job_bulk_renew.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#		bulk_renew
#       fi
       echo "process is already running "
else
       echo "Executing the process bulk_renew"
	bulk_renew
fi

