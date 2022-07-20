#! /bin/bash
## Modfified on 18-MAY-2015 to remove PID check, now lock file mechanism is used
modem_device_upd()
{
         echo "runnng" > mta_job_device_modem1.txt
	 mso_mta_job_processor -object_type /mso_mta_job/attribute_update/modem 
#         pid=$(cat mta_job_device_modem1.txt)
#         wait $pid
         rm mta_job_device_modem1.txt;
}
wifi_router_device_upd()
{
         mso_mta_job_processor -object_type /mso_mta_job/attribute_update/router/wifi 
         echo $! > mta_job_device_wifi_router1.txt
         pid=$(cat mta_job_device_wifi_router1.txt)
         wait $pid
         rm mta_job_device_wifi_router1.txt;
}

cable_router_device_upd()
{
         echo "runnng" > mta_job_device_cable_router1.txt
	 mso_mta_job_processor -object_type /mso_mta_job/attribute_update/router/cable 

#         pid=$(cat mta_job_device_cable_router1.txt)
#         wait $pid
         rm mta_job_device_cable_router1.txt;
}

nat_device_upd()
{
         echo "runnng" > mta_job_device_nat1.txt
	 mso_mta_job_processor -object_type /mso_mta_job/attribute_update/nat 

#         pid=$(cat mta_job_device_nat1.txt)
#         wait $pid
         rm mta_job_device_nat1.txt;
}

static_ip_device_upd()
{
         echo "runnng" > mta_job_device_static_ip1.txt
	 mso_mta_job_processor -object_type /mso_mta_job/attribute_update/ip/static

#         pid=$(cat mta_job_device_static_ip1.txt)
#         wait $pid
         rm mta_job_device_static_ip1.txt;
}

framed_ip_device_upd()
{
         echo "runnng" > mta_job_device_framed_ip1.txt
	 mso_mta_job_processor -object_type /mso_mta_job/attribute_update/ip/framed 

#         pid=$(cat mta_job_device_framed_ip1.txt)
#         wait $pid
         rm mta_job_device_framed_ip1.txt;
}

stb_device_upd()
{
         echo "runnng" > mta_job_device_stb1.txt
	 mso_mta_job_processor -object_type /mso_mta_job/attribute_update/stb

#         pid=$(cat mta_job_device_stb1.txt)
#         wait $pid
         rm mta_job_device_stb1.txt;
}

vc_device_upd()
{
         echo "runnng" > mta_job_device_vc1.txt
	 mso_mta_job_processor -object_type /mso_mta_job/attribute_update/vc

#         pid=$(cat mta_job_device_vc1.txt)
#         wait $pid
         rm mta_job_device_vc1.txt;
}

vc_state_upd()
{
         echo "runnng" > mta_job_state_vc1.txt
         mso_mta_job_processor -object_type /mso_mta_job/state_change/vc
         rm mta_job_state_vc1.txt;
}

stb_state_upd()
{
         echo "runnng" > mta_job_state_stb1.txt
         mso_mta_job_processor -object_type /mso_mta_job/state_change/stb
         rm mta_job_state_stb1.txt;
}

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

vc_location_upd()
{
         echo "runnng" > mta_job_location_vc1.txt
         mso_mta_job_processor -object_type /mso_mta_job/location_update/vc
         rm mta_job_location_vc1.txt;
}

stb_location_upd()
{
         echo "runnng" > mta_job_location_stb1.txt
         mso_mta_job_processor -object_type /mso_mta_job/location_update/stb
         rm mta_job_location_stb1.txt;
}

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

bulk_hierarchy_upd()
{
         echo "runnng" > mta_job_bulk_hierarchy1.txt
         mso_mta_job_processor -object_type /mso_mta_job/bulk_hierarchy
         rm mta_job_bulk_hierarchy1.txt;
}
date;
cd $PIN_HOME/mso/apps/mso_bulk_utilities
echo "current working directory is `pwd`"

if [ -e mta_job_device_modem1.txt ]
then
       echo "process is already running "
       #echo "file is available";
       #pid1=$(cat mta_job_device_modem1.txt)

       #if ps -p $pid1 > /dev/null
       #then
       #echo " process is already running "
       #else
       #        echo " process is not running "
       #         modem_device_upd
       #fi

else
       #echo "file is not available "
       echo "Executing the process modem_device_upd"
       modem_device_upd
fi

if [ -e mta_job_device_wifi_router1.txt ]
then
       #echo "file is available";
#       pid1=$(cat mta_job_wifi_router1.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#                wifi_router_device_upd
#       fi
      echo " process is already running "
else
       echo "Executing the process wifi_router_device_upd"
       wifi_router_device_upd
fi

if [ -e mta_job_device_cable_router1.txt ]
then
       #echo "file is available";
#       pid1=$(cat mta_job_device_cable_router1.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#                cable_router_device_upd
#       fi
      echo " process is already running "
else
       echo "Executing the process cable_router_device_upd"
       cable_router_device_upd
fi

if [ -e mta_job_device_nat1.txt ]
then
       #echo "file is available";
#       pid1=$(cat mta_job_device_nat1.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#                nat_device_upd
#       fi
      echo " process is already running "
else
       echo "Executing the process nat_device_upd"
       nat_device_upd
fi

if [ -e mta_job_device_static_ip1.txt ]
then
       #echo "file is available";
#       pid1=$(cat mta_job_device_static_ip1.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#                static_ip_device_upd
#       fi
echo " process is already running "
else
       echo "Executing the process static_ip_device_upd"
       static_ip_device_upd
fi

if [ -e mta_job_device_framed_ip1.txt ]
then
       #echo "file is available";
#       pid1=$(cat mta_job_device_framed_ip1.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#                framed_ip_device_upd
#       fi
      echo " process is already running "
else
       echo "Executing the process framed_ip_device_upd"
       framed_ip_device_upd
fi

if [ -e mta_job_device_stb1.txt ]
then
       #echo "file is available";
#       pid1=$(cat mta_job_device_stb1.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#                stb_device_upd
#       fi
      echo " process is already running "
else
       echo "Executing the process stb_device_upd"
       stb_device_upd
fi

if [ -e mta_job_device_vc1.txt ]
then
       #echo "file is available";
#       pid1=$(cat mta_job_device_vc1.txt)
#
#       if ps -p $pid1 > /dev/null
#       then
#               echo " process is already running "
#       else
#               echo " process is not running "
#                vc_device_upd
#       fi
      echo " process is already running "
else
       echo "Executing the process vc_device_upd"
       vc_device_upd
fi

if [ -e mta_job_state_vc1.txt ]
then
      echo " process is already running "
else
       echo "Executing the process vc_state_upd"
       vc_state_upd
fi

if [ -e mta_job_state_stb1.txt ]
then
      echo " process is already running "
else
       echo "Executing the process stb_state_upd"
       stb_state_upd
fi

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

if [ -e mta_job_location_vc1.txt ]
then
      echo " process is already running "
else
       echo "Executing the process vc_location_upd"
       vc_location_upd
fi

if [ -e mta_job_location_stb1.txt ]
then
      echo " process is already running "
else
       echo "Executing the process stb_location_upd"
       stb_location_upd
fi

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

if [ -e mta_job_bulk_hierarchy1.txt ]
then
      echo " process is already running "
else
       echo "Executing the process bulk_hierarchy_upd"
       bulk_hierarchy_upd
fi
