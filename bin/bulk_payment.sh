#! /bin/bash
## Created on 19-MAY-2015

bulk_payment()
{
         echo "runnng" > mta_job_bulk_payment.txt
	 #mso_bulk_pymt_posting -verbose
   	mso_bulk_pymt_posting -object_type /mso_mta_job/payment -verbose
         rm mta_job_bulk_payment.txt;
}

cd $PIN_HOME/mso/apps/mso_bulk_payment_posting
echo "current working directory is `pwd`"

if [ -e mta_job_bulk_payment.txt ]
then
       echo "process is already running "
else
       echo "Executing the process bulk_payment"
	bulk_payment
fi

