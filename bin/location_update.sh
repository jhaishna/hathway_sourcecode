#! /bin/bash
## Modfified on 15-MAY-2015 to remive PID check, now lock file mechanism is used
modem_location_upd()
{
         echo "runnng" > mta_job_device_modem2.txt
	 mso_mta_job_processor -object_type /mso_mta_job/location_update/modem 

#         pid=$(cat mta_job_device_modem2.txt)
#         wait $pid
         rm mta_job_device_modem2.txt;
}
wifi_router_location_upd()
{
          echo "runnng" > mta_job_device_wifi_router2.txt
	 mso_mta_job_processor -object_type /mso_mta_job/location_update/router/wifi

#         pid=$(cat mta_job_device_wifi_router2.txt)
#         wait $pid
         rm mta_job_device_wifi_router2.txt;
}

cable_router_location_upd()
{
          echo "runnng" > mta_job_device_cable_router2.txt
	 mso_mta_job_processor -object_type /mso_mta_job/location_update/router/cable

#         pid=$(cat mta_job_device_cable_router2.txt)
#         wait $pid
         rm mta_job_device_cable_router2.txt;
}

nat_location_upd()
{
           echo "runnng" > mta_job_device_nat2.txt
	 mso_mta_job_processor -object_type /mso_mta_job/location_update/nat

#         pid=$(cat mta_job_device_nat2.txt)
#         wait $pid
         rm mta_job_device_nat2.txt;
}

static_ip_location_upd()
{
            echo "runnng" > mta_job_device_static_ip2.txt
	 mso_mta_job_processor -object_type /mso_mta_job/location_update/ip/static 

#         pid=$(cat mta_job_device_static_ip2.txt)
#         wait $pid
         rm mta_job_device_static_ip2.txt;
}

framed_ip_location_upd()
{
         echo "runnng" > mta_job_device_static_framed2.txt
	 mso_mta_job_processor -object_type /mso_mta_job/location_update/ip/framed

#         pid=$(cat mta_job_device_framed_ip2.txt)
#         wait $pid
         rm mta_job_device_static_framed2.txt;
}


stb_location_upd()
{
         echo "runnng" > mta_job_device_stb2.txt
	 mso_mta_job_processor -object_type /mso_mta_job/location_update/stb

#         pid=$(cat mta_job_device_stb2.txt)
#         wait $pid
         rm mta_job_device_stb2.txt;
}

vc_location_upd()
{
         echo "runnng" > mta_job_device_vc2.txt
	 mso_mta_job_processor -object_type /mso_mta_job/location_update/vc

#         pid=$(cat mta_job_device_vc2.txt)
#         wait $pid
         rm mta_job_device_vc2.txt;
}

cd $PIN_HOME/mso/apps/mso_bulk_inventory
echo "current working directory is `pwd`"

if [ -e mta_job_device_modem2.txt ]
then
       #echo "file is available";
#       pid1=$(cat mta_job_device_modem2.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#                modem_location_upd
#       fi
              echo " process is already running "
else
       echo "Executing modem_location_upd"
       modem_location_upd
fi

if [ -e mta_job_device_wifi_router2.txt ]
then
       #echo "file is available";
#       pid1=$(cat mta_job_wifi_router2.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#                wifi_router_location_upd
#       fi
              echo " process is already running "
else
       echo "Executing wifi_router_location_upd"
       wifi_router_location_upd
fi

if [ -e mta_job_device_cable_router2.txt ]
then
       #echo "file is available";
#       pid1=$(cat mta_job_device_cable_router2.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#                cable_router_location_upd
#       fi
              echo " process is already running "
else
       echo "Executing cable_router_location_upd"
       cable_router_location_upd
fi

if [ -e mta_job_device_nat2.txt ]
then
       #echo "file is available";
#       pid1=$(cat mta_job_device_nat2.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#                nat_location_upd
#       fi
              echo " process is already running "
else
       echo "Executing nat_location_upd"
       nat_location_upd
fi

if [ -e mta_job_device_static_ip2.txt ]
then
       #echo "file is available";
#       pid1=$(cat mta_job_device_static_ip2.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#                static_ip_location_upd
#       fi
              echo " process is already running "
else
       echo "Executing static_ip_location_upd"
       static_ip_location_upd
fi

if [ -e mta_job_device_framed_ip2.txt ]
then
       #echo "file is available";
#       pid1=$(cat mta_job_device_framed_ip2.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#                framed_ip_location_upd
#       fi
              echo " process is already running "
else
       echo "Executing framed_ip_location_upd"
       framed_ip_location_upd
fi

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

if [ -e mta_job_device_vc2.txt ]
then
       #echo "file is available";
#       pid1=$(cat mta_job_device_vc2.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#                vc_location_upd
#       fi
              echo " process is already running "
else
       echo "Executing vc_location_upd"
       vc_location_upd
fi

