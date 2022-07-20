#! /bin/bash

run_mso_notify_expiry_plans()
{
         echo "runnng" > mso_notify_expiry_plans.txt
	mso_notify_expiry_plans -number_of_days 7
	rm mso_notify_expiry_plans.txt;
}

cd $PIN_HOME/mso/apps/mso_bulk_notification
echo "current working directory is `pwd`"

if [ -e mso_notify_expiry_plans.txt ]
then
               echo " process is already running "

else
       echo "Executing the process mso_notify_expiry_plans"
       run_mso_notify_expiry_plans
fi
