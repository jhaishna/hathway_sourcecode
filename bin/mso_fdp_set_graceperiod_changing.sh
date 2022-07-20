#!/bin/bash

`cp /data/opt/portal/7.5/var/pin_billd/mso_fdp_set_graceperiod.log /data/opt/portal/7.5/var/pin_billd/mso_fdp_set_graceperiod.log_bkp`
`:>/data/opt/portal/7.5/var/pin_billd/mso_fdp_set_graceperiod.log`
cd /home/pin/opt/portal/7.5/mso/bin/
echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for grace in /home/pin/opt/portal/7.5/mso/bin/"
else
   echo "`date`:Testnap Connection Failed so job skipped for grace in /home/pin/opt/portal/7.5/mso/bin/" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
   exit;
fi

actions()
{
	 mso_fdp_set_graceperiod -grace_type default -verbose & >/dev/null
         echo $! > mso_fdp_set_graceperiod.txt
         pid=$(cat mso_fdp_set_graceperiod.txt)
         wait $pid
         rm mso_fdp_set_graceperiod.txt;
    	 	
	 fname="mso_fdp_set_graceperiod.log"
	 fail="1"

	exec < /data/opt/portal/7.5/var/pin_billd/$fname


	while read line
	do

        if [[ $line == *"number of errors"* ]]
        then
                orig_value=`echo $line | cut -d '=' -f2`
                orig_value=${orig_value// }
		num_value=`echo $orig_value | cut -d '.' -f1`
                #echo "*******$orig_value******"
                if [ "$num_value" > 20 ]
                then
                        grace_result=1
                fi
		if [ "$orig_value" == "0." ]
                then
                        result=0
                fi	

        fi

	if [[ $result -eq $fail ]];
        then
        		echo "`date`:mso_fdp_set_graceperiod rpt Failed" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                else
                        echo "`date`:mso_fdp_set_graceperiod rpt Success" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
        	fi
	 
		 	
	 `cat mso_fdp_set_graceperiod.log >> mso_fdp_set_graceperiod.log_bkp`
	`:>/data/opt/portal/7.5/var/pin_billd/mso_fdp_set_graceperiod.log`

	 

	 echo "`date`:add_on_grace period started"
	 mso_fdp_set_graceperiod -grace_type add_on_grace -verbose & >/dev/null 
         echo $! > mso_fdp_set_graceperiod.txt
         pid=$(cat mso_fdp_set_graceperiod.txt)
         wait $pid
         rm mso_fdp_set_graceperiod.txt;

	 fname="mso_fdp_set_graceperiod.log"
         fail="1"

       	 exec < /data/opt/portal/7.5/var/pin_billd/$fname


         while read line
         do

         if [[ $line == *"number of errors"* ]]
         then
                orig_value=`echo $line | cut -d '=' -f2`
                orig_value=${orig_value// }
                num_value=`echo $orig_value | cut -d '.' -f1`
                #echo "*******$orig_value******"
                if [ "$num_value" > 20 ]
                then
                        add_on_grace_result=1
                fi

         fi


}


cd $PIN_HOME/apps/pin_billd
echo "current working directory is `pwd`"
echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for grace in PIN_HOME/apps/pin_billd"
else
   echo "`date`:Testnap Connection Failed so job skipped for grace in PIN_HOME/apps/pin_billd" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
   exit;
fi

if [ -e mso_fdp_set_graceperiod.txt ]
then
       echo "file is available";
       pid1=$(cat mso_fdp_set_graceperiod.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
		actions
       fi

else
	echo "Starting to run mso_fdp_set_graceperiod job ";
	echo `date`
	actions
	echo "End of mso_fdp_set_graceperiod job ";
	echo `date`
fi

#checking the log to get the status of the report
fname="mso_fdp_set_graceperiod.log"
fail="1"

exec < /data/opt/portal/7.5/var/pin_billd/$fname


while read line
do

        if [[ $line == *"data errors"* ]]
        then
                orig_value=`echo $line | cut -d '=' -f2`
                orig_value=${orig_value// }
                #echo "*******$orig_value******"
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
                        echo "`date`:mso_fdp_set_graceperiod rpt Failed" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                else
                        echo "`date`:mso_fdp_set_graceperiod rpt Success" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                fi
        fi


done

cd /data/opt/portal/7.5/var/pin_billd/ 
`cat mso_fdp_set_graceperiod.log >> mso_fdp_set_graceperiod.log_bkp`
`mv mso_fdp_set_graceperiod.log_bkp mso_fdp_set_graceperiod.log`

