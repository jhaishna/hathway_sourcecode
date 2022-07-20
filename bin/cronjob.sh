#! /bin/bash

stb_device_load()
{
	 mso_mta_job_processor -object_type /mso_mta_job/device/stb & > /dev/null
         echo $! > mta_job_device_stb.txt
         pid=$(cat mta_job_device_stb.txt)
         wait $pid
         rm mta_job_device_stb.txt;
}

vc_device_load()
{
         mso_mta_job_processor -object_type /mso_mta_job/device/vc & > /dev/null
         echo $! > mta_job_device_vc.txt
         pid=$(cat mta_job_device_vc.txt)
         wait $pid
         rm mta_job_device_vc.txt;
}

cd $PIN_HOME/mso/apps/mso_device_loader
echo "current working directory is `pwd`"
if [ -e mta_job_device_stb.txt ]
then
       echo "file is available";
       pid1=$(cat mta_job_device_stb.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
		stb_device_load
       fi

else
       echo "file is not available ";
	stb_device_load
fi

if [ -e mta_job_device_vc.txt ]
then
       echo "file is available";
       pid1=$(cat mta_job_device_vc.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
                stb_device_load
       fi

else
       echo "file is not available ";
        stb_device_load
fi

