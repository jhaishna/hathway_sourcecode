#!/bin/ksh

`:>/home/pin/opt/portal/7.5/sys/test/bulk_lco_pymt.log`

cd /home/pin/opt/portal/7.5/mso/bin/
echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for bulk_payment in /home/pin/opt/portal/7.5/mso/bin/"
else
   echo "`date`:Testnap Connection Failed so job skipped for bulk_payment in /home/pin/opt/portal/7.5/mso/bin/" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
   exit;
fi

if [ "$1" == "-payments" -a $# -eq 1 ]; then
	echo `date`
	echo "Executing: mso_bulk_pymt_posting -object_type /mso_mta_job/payment "
	cd /home/pin/opt/portal/7.5/mso/apps/mso_bulk_payment_posting
	echo q | testnap;
	Rstatus=$?
	if [ $Rstatus -eq 0 ] ; then
   		echo "`date`:Testnap Connection Success for termination in /home/pin/opt/portal/7.5/mso/apps/mso_bulk_payment_posting"
	else
   		echo "`date`:Testnap Connection Failed so job skipped for termination in /home/pin/opt/portal/7.5/mso/apps/mso_bulk_payment_posting" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
   	exit;
	fi
 
	echo "current working directory is `pwd`"
        mso_bulk_pymt_posting -verbose -object_type /mso_mta_job/payment -verbose
	echo `date`
else
    #echo "Invalid input parameter"
    echo
    echo "Usage : $0 -payments"
    echo
    exit 1
fi

#checking the log to get the status of the report
fname="bulk_lco_pymt.log"
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
                        echo "`date`:bulk_payment_job rpt Failed" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                else
                        echo "`date`:bulk_payment_job rpt Success" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                fi
        fi


done
