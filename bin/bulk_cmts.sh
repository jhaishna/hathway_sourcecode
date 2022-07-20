#! /bin/bash

bulk_cmts()
{
         echo "runnng" > mta_job_bulk_cmts.txt
	 mso_mta_job_processor -object_type /mso_mta_job/bulk_cmts_change

         rm mta_job_bulk_cmts.txt;
}

cd $PIN_HOME/mso/apps/mso_bulk_plan
echo "current working directory is `pwd`"

if [ -e mta_job_bulk_cmts.txt ]
then
       
	echo "process is already running "
else
       echo "Executing the process bulk_cmts_change"
	bulk_cmts
fi

