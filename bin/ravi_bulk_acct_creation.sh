#! /bin/bash

bulk_acct_create()
{
         echo "running" > mta_job_bulk_acct_creation.txt
         mso_mta_job_processor -object_type /mso_mta_job/bulk_create_account

#         pid=$(cat mta_job_bulk_acct_creation.txt)
#         wait $pid
         rm mta_job_bulk_acct_creation.txt;
}

cd $PIN_HOME/mso/apps/mso_bulk_plan
echo "current working directory is `pwd`"

if [ -e mta_job_bulk_acct_creation.txt ]
then
#       echo "file is available";
#       pid1=$(cat mta_job_bulk_acct_creation.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#               bulk_acct_create
#       fi
       echo "process is already running "
else
       echo "Executing the process bulk account creation"
        bulk_acct_create
fi

sh /home/pin/opt/portal/7.5/mso/bin/bulk_acct_creation_proc.sh PIN GeDFfvKj 172.20.20.23 1723 PINDB

echo "Script Execution Completed"


