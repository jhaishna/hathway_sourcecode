#! /bin/bash
## Modfified on 18-MAY-2015 to remove PID check, now lock file mechanism is used
no_of_months=$1

run_mso_clean_bb_objects()
{
        echo "runnng" > mso_clean_bb_objects.txt
	#Run the cleanup for Daily Usage Objects
        run_mso_clean_bb_objects -m $no_of_months -t 1

#	pid=$(cat mso_clean_bb_objects.txt)
#	wait $pid

	#Run the cleanup for Trial Bill Report Objects
        run_mso_clean_bb_objects -m $no_of_months -t 2

#	pid=$(cat mso_clean_bb_objects.txt)
#	wait $pid

	#Run the cleanup for Device Lifecycle Objects
        run_mso_clean_bb_objects -m $no_of_months -t 3

#	pid=$(cat mso_clean_bb_objects.txt)
#	wait $pid

	#Run the cleanup for Suspended CDR Objects
        run_mso_clean_bb_objects -m $no_of_months -t 4

#	pid=$(cat mso_clean_bb_objects.txt)
#	wait $pid
	rm mso_clean_bb_objects.txt;
}

cd $PIN_HOME/mso/apps/mso_bulk_housekeeping
echo "current working directory is `pwd`"

if [ -e mso_clean_bb_objects.txt ]
then
       #echo "file is available";
#       pid1=$(cat mso_clean_bb_objects.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#                run_mso_clean_bb_objects
#       fi
       echo "process is already running "
else
       echo "Executing the process run_mso_clean_bb_objects"
       run_mso_clean_bb_objects
fi
