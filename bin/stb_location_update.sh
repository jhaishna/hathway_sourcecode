#! /bin/bash
## Modfified on 15-MAY-2015 to remive PID check, now lock file mechanism is used

stb_location_upd()
{
         echo "runnng" > mta_job_device_stb2.txt
	 mso_mta_job_processor -object_type /mso_mta_job/location_update/stb

#         pid=$(cat mta_job_device_stb2.txt)
#         wait $pid
         rm mta_job_device_stb2.txt;
}

cd $PIN_HOME/mso/apps/mso_bulk_plan
echo "current working directory is `pwd`"

if [ -e mta_job_device_stb2.txt ]
then
       #echo "file is available";
#       pid1=$(cat mta_job_device_stb2.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#                stb_location_upd
#       fi
              echo " process is already running "
else
       echo "Executing stb_location_upd"
       stb_location_upd
fi

