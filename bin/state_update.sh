#! /bin/bash
## Modfified on 18-MAY-2015 to remove PID check, now lock file mechanism is used
modem_state_upd()
{
         echo "runnng" > mta_job_device_modem3.txt
	 mso_mta_job_processor -object_type /mso_mta_job/state_change/modem

#         pid=$(cat mta_job_device_modem3.txt)
#         wait $pid
         rm mta_job_device_modem3.txt;
}
wifi_router_state_upd()
{
         echo "runnng" > mta_job_device_wifi_router3.txt
	 mso_mta_job_processor -object_type /mso_mta_job/state_change/router/wifi

#	 pid=$(cat mta_job_device_wifi_router3.txt)
#         wait $pid
         rm mta_job_device_wifi_router3.txt;
}

cable_router_state_upd()
{
          echo "runnng" > mta_job_device_cable_router3.txt
	 mso_mta_job_processor -object_type /mso_mta_job/state_change/router/cable

#         pid=$(cat mta_job_device_cable_router3.txt)
#         wait $pid
         rm mta_job_device_cable_router3.txt;
}

nat_state_upd()
{
           echo "runnng" > mta_job_device_nat3.txt
	 mso_mta_job_processor -object_type /mso_mta_job/state_change/nat

#         pid=$(cat mta_job_device_nat3.txt)
#         wait $pid
         rm mta_job_device_nat3.txt;
}

static_ip_state_upd()
{
         echo "runnng" > mta_job_device_static_ip3.txt
	 mso_mta_job_processor -object_type /mso_mta_job/state_change/ip/static

#         pid=$(cat mta_job_device_static_ip3.txt)
#         wait $pid
         rm mta_job_device_static_ip3.txt;
}

framed_ip_state_upd()
{
         echo "runnng" > mta_job_device_framed_ip3.txt
	 mso_mta_job_processor -object_type /mso_mta_job/state_change/ip/framed

#         pid=$(cat mta_job_device_framed_ip3.txt)
#         wait $pid
         rm mta_job_device_framed_ip3.txt;
}


stb_state_upd()
{
         echo "runnng" > mta_job_device_stb3.txt
	 mso_mta_job_processor -object_type /mso_mta_job/state_change/stb

#         pid=$(cat mta_job_device_stb3.txt)
#         wait $pid
         rm mta_job_device_stb3.txt;
}

vc_state_upd()
{
         echo "runnng" > mta_job_device_vc3.txt
	 mso_mta_job_processor -object_type /mso_mta_job/state_change/vc

#         pid=$(cat mta_job_device_vc3.txt)
#         wait $pid
         rm mta_job_device_vc3.txt;
}

cd $PIN_HOME/mso/apps/mso_bulk_inventory
echo "current working directory is `pwd`"

if [ -e mta_job_device_modem3.txt ]
then
       #echo "file is available";
#       pid1=$(cat mta_job_device_modem3.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#                modem_state_upd
#       fi
       echo " process is already running "
else
       echo "Executing the process modem_state_upd"
       modem_state_upd
fi

if [ -e mta_job_device_wifi_router3.txt ]
then
       #echo "file is available";
#       pid1=$(cat mta_job_wifi_router3.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#                wifi_router_state_upd
#       fi
       echo " process is already running "
else
       echo "Executing the process wifi_router_state_upd"
       wifi_router_state_upd
fi

if [ -e mta_job_device_cable_router3.txt ]
then
       #echo "file is available";
#       pid1=$(cat mta_job_device_cable_router3.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#                cable_router_state_upd
#       fi
       echo " process is already running "
else
       echo "Executing the process cable_router_state_upd"
       cable_router_state_upd
fi

if [ -e mta_job_device_nat3.txt ]
then
       #echo "file is available";
#       pid1=$(cat mta_job_device_nat3.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#                nat_state_upd
#       fi
       echo " process is already running "
else
       echo "Executing the process nat_state_upd"
       nat_state_upd
fi

if [ -e mta_job_device_static_ip3.txt ]
then
       #echo "file is available";
#       pid1=$(cat mta_job_device_static_ip3.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#                static_ip_state_upd
#       fi
       echo " process is already running "
else
       echo "Executing the process static_ip_state_upd"
       static_ip_state_upd
fi

if [ -e mta_job_device_framed_ip3.txt ]
then
       #echo "file is available";
#       pid1=$(cat mta_job_device_framed_ip3.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#                framed_ip_state_upd
#       fi
       echo " process is already running "
else
       echo "Executing the process framed_ip_state_upd"
       framed_ip_state_upd
fi

if [ -e mta_job_device_stb3.txt ]
then
       #echo "file is available";
#       pid1=$(cat mta_job_device_stb3.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#                stb_state_upd
#       fi
       echo " process is already running "
else
       echo "Executing the process stb_state_upd"
       stb_state_upd
fi

if [ -e mta_job_device_vc3.txt ]
then
       #echo "file is available";
#       pid1=$(cat mta_job_device_vc3.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#                vc_state_upd
#       fi
       echo " process is already running "
else
       echo "Executing the process vc_state_upd"
       vc_state_upd
fi

