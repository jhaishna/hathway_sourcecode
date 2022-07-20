#! /bin/bash
`:>/home/pin/Thirdparty/BRM12/BRM/sys/test/device_load.log`
fname="device_load.log"
fail="1"

cd $PIN_HOME/mso/bin/ 
echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for device_load in $PIN_HOME/mso/bin/"
else
   echo "`date`:Testnap Connection Failed so job skipped for device_load in $PIN_HOME/mso/bin/" >> /home/pin/Thirdparty/BRM12/BRM/var/cron_global/global_cron_tab.log
   exit;
fi
# Modfified on 15-MAY-2015 to remove PID check, now lock file mechanism is used
modem_device_load()
{
         echo "runnng" > mta_job_device_modem.txt
	 mso_mta_job_processor -verbose -object_type /mso_mta_job/device/modem

#         pid=$(cat mta_job_device_modem.txt)
#         wait $pid
         rm mta_job_device_modem.txt;
}
wifi_router_device_load()
{
         mso_mta_job_processor -verbose -object_type /mso_mta_job/device/router/wifi 
         echo $! > mta_job_device_wifi_router.txt
#         pid=$(cat mta_job_device_wifi_router.txt)
#         wait $pid
         rm mta_job_device_wifi_router.txt;
}

cable_router_device_load()
{
         echo "runnng" > mta_job_device_cable_router.txt
	 mso_mta_job_processor -verbose -object_type /mso_mta_job/device/router/cable

#         pid=$(cat mta_job_device_cable_router.txt)
#         wait $pid
         rm mta_job_device_cable_router.txt;
}

nat_device_load()
{
          echo "runnng" > mta_job_device_nat.txt
	 mso_mta_job_processor -verbose -object_type /mso_mta_job/device/nat

#         pid=$(cat mta_job_device_nat.txt)
#         wait $pid
         rm mta_job_device_nat.txt;
}

static_ip_device_load()
{
          echo "runnng" > mta_job_device_static_ip.txt
	 mso_mta_job_processor -verbose -object_type /mso_mta_job/device/ip/static

#         pid=$(cat mta_job_device_static_ip.txt)
#         wait $pid
         rm mta_job_device_static_ip.txt;
}

framed_ip_device_load()
{
          echo "runnng" > mta_job_device_framed_ip.txt
	 mso_mta_job_processor -verbose -object_type /mso_mta_job/device/ip/framed

#         pid=$(cat mta_job_device_framed_ip.txt)
#         wait $pid
         rm mta_job_device_framed_ip.txt;
}


stb_device_load()
{
         mso_mta_job_processor -verbose -object_type /mso_mta_job/device/stb & > /dev/null
         echo $! > mta_job_device_stb.txt
         pid=$(cat mta_job_device_stb.txt)
         wait $pid
         rm mta_job_device_stb.txt;
}

vc_device_load()
{
         mso_mta_job_processor -verbose -object_type /mso_mta_job/device/vc & > /dev/null
         echo $! > mta_job_device_vc.txt
         pid=$(cat mta_job_device_vc.txt)
         wait $pid
         rm mta_job_device_vc.txt;
}
log_check_function()
{
#checking the log to get the status of the report
#exec < /home/pin/opt/portal/7.5/sys/test/$fname
exec < /home/pin/Thirdparty/BRM12/BRM/sys/test/$fname

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
                        echo "`date`:bulk_device_load Job for $1 Failed" >> /home/pin/Thirdparty/BRM12/BRM/var/cron_global/global_cron_tab.log
                else
                        echo "`date`:bulk_device_load Job for $1 Success" >> /home/pin/Thirdparty/BRM12/BRM/var/cron_global/global_cron_tab.log
                fi
        fi

done
}

device_load_log_clean_bup()
{

        `cat /home/pin/Thirdparty/BRM12/BRM/sys/test/$fname >> device_load_bkp_curr.log`
        `:>/home/pin/Thirdparty/BRM12/BRM/sys/test/$fname`
}

cd $PIN_HOME/mso/apps/mso_bulk_inventory
echo "current working directory is `pwd`"
echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for device_load in PIN_HOME/mso/apps/mso_bulk_inventory"
else
   echo "`date`:Testnap Connection Failed so job skipped for device_load in PIN_HOME/mso/apps/mso_bulk_inventory" >> /home/pin/Thirdparty/BRM12/BRM/var/cron_global/global_cron_tab.log
   exit;
fi

if [ -e mta_job_device_modem.txt ]
then
       #echo "file is available";
#       pid1=$(cat mta_job_device_modem.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#                modem_device_load
#       fi
              echo " process is already running "

else
       echo " Executing modem_device_load "
       modem_device_load
fi

log_check_function "modem"
device_load_log_clean_bup

if [ -e mta_job_device_wifi_router.txt ]
then
       #echo "file is available";
#       pid1=$(cat mta_job_wifi_router.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#                wifi_router_device_load
#       fi
              echo " process is already running "
else
       echo "Executing wifi_router_device_load "
       wifi_router_device_load
fi

log_check_function "wifi_router"
device_load_log_clean_bup

if [ -e mta_job_device_cable_router.txt ]
then
       #echo "file is available";
#       pid1=$(cat mta_job_device_cable_router.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#                cable_router_device_load
#       fi

          echo " process is already running "
else
       echo "Executing cable_router_device_load "
       cable_router_device_load
fi

log_check_function "cable_router"
device_load_log_clean_bup

if [ -e mta_job_device_nat.txt ]
then
       #echo "file is available";
#       pid1=$(cat mta_job_device_nat.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#                nat_device_load
#       fi
          echo " process is already running "
else
       echo "Executing nat_device_load "
       nat_device_load
fi

log_check_function "nat"
device_load_log_clean_bup

if [ -e mta_job_device_static_ip.txt ]
then
       #echo "file is available";
#       pid1=$(cat mta_job_device_static_ip.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#                static_ip_device_load
#       fi
          echo " process is already running "
else
       echo "Executing static_ip_device_load "
       static_ip_device_load
fi

log_check_function "statis_ip"
device_load_log_clean_bup

if [ -e mta_job_device_framed_ip.txt ]
then
       #echo "file is available";
#       pid1=$(cat mta_job_device_framed_ip.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#                framed_ip_device_load
#       fi
          echo " process is already running "
else
       echo "Executing framed_ip_device_load "
       framed_ip_device_load
fi

log_check_function "framed_ip"
device_load_log_clean_bup

if [ -e mta_job_device_stb.txt ]
then
       #echo "file is available";
       pid1=$(cat mta_job_device_stb.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
                stb_device_load
       fi

else
       echo "file is not available "
       stb_device_load
fi

log_check_function "stb"
device_load_log_clean_bup

if [ -e mta_job_device_vc.txt ]
then
       #echo "file is available";
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
        vc_device_load
fi

log_check_function "vc"
device_load_log_clean_bup

`mv /home/pin/Thirdparty/BRM12/BRM/sys/test/device_load_bkp_curr.log $fname`
