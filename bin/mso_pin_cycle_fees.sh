#! /bin/bash
## Modfified on 18-MAY-2015 to remove PID check, now lock file mechanism is used
run_pin_cycle_fees()
{
         echo "runnng" > mso_run_pin_cycle_fees.txt
	#Run pin_cycle_fees for cancel
        run_cycle_fees_cancel

#	pid=$(cat mso_run_pin_cycle_fees.txt)
#	wait $pid

	#Run pin_cycle_fees for renewal
        run_cycle_fees_regular
#        run_subscription_normal_bill $month$day

#	pid=$(cat mso_run_pin_cycle_fees.txt)
#	wait $pid
	rm mso_run_pin_cycle_fees.txt;
}



cd $PIN_HOME/apps/pin_billd
echo "current working directory is `pwd`"

if [ -e mso_run_pin_cycle_fees.txt ]
then
       #echo "file is available";
#       pid1=$(cat mso_run_pin_cycle_fees.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#                run_pin_cycle_fees
#       fi
       echo "process is already running "
else
       echo "Executing the process run_pin_cycle_fees"
       run_pin_cycle_fees
fi
