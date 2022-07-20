#!/bin/bash

`cp /data/opt/portal/7.5/var/pin_billd/mso_fdp_set_graceperiod.log /data/opt/portal/7.5/var/pin_billd/mso_fdp_set_graceperiod.log_bkp`
`:>/data/opt/portal/7.5/var/pin_billd/mso_fdp_set_graceperiod.log`

grace_result=0
add_on_grace_result=0;


gracelog_check()
{

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
                if [ "$num_value" -ge 20 ]
                then
                        grace_result=1
                fi
        fi
	done	
}

add_on_gracelog_check()
{

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
                if [ "$num_value" -ge 20 ]
                then
                        add_on_grace_result=1
                fi
        fi
	done
}

backup()
{

	 cd /data/opt/portal/7.5/var/pin_billd/
	`cat mso_fdp_set_graceperiod.log >> mso_fdp_set_graceperiod.log_bkp`
        `:>/data/opt/portal/7.5/var/pin_billd/mso_fdp_set_graceperiod.log`
	cd $PIN_HOME/apps/pin_billd

}

default_check() {
	
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
                	if [ "$orig_value" != "0." ]
                	then
                        	result=1
                	fi

        	fi

        	if [[ $line == *"number of errors"* ]]
        	then
                	if [[ $result -eq $fail ]];
                	then
                        	echo "`date`:mso_fdp_set_graceperiod for $1 Failed" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                	else
                       	 	echo "`date`:mso_fdp_set_graceperiod for $1 Success" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                	fi
        	fi

done

}

actions()
{
	 echo "starting normal grace @" `date`
         mso_fdp_set_graceperiod -grace_type default -verbose & >/dev/null 
	 echo $! > mso_fdp_set_graceperiod.txt
         pid=$(cat mso_fdp_set_graceperiod.txt)
         wait $pid
         rm mso_fdp_set_graceperiod.txt;
	 gracelog_check 
	 default_check "normal grace"        
	 backup
	 
	echo "`date`:add_on_grace period started"
	 mso_fdp_set_graceperiod -grace_type add_on_grace -verbose & >/dev/null
	 echo $! > mso_fdp_set_graceperiod.txt
         pid=$(cat mso_fdp_set_graceperiod.txt)
         wait $pid
         rm mso_fdp_set_graceperiod.txt;
	 add_on_gracelog_check
	 default_check "addon grace"
	 backup
}

actions_cycle_fees()
{
        echo "starting  pin_cycle_fees for defer_cancel@" `date`
        pin_cycle_fees -verbose -defer_cancel
         echo $! > pin_cycle_fees_catv.txt
        pid=$(cat pin_cycle_fees_catv.txt)
        wait $pid
        rm pin_cycle_fees_catv.txt ;
        echo "finished  pin_cycle_fees for defer_cancel@" `date`

        echo "starting  pin_cycle_fees for cancel@" `date`
        pin_cycle_fees -verbose -cancel
        echo $! > pin_cycle_fees_catv.txt
         pid=$(cat pin_cycle_fees_catv.txt)
        wait $pid
        rm pin_cycle_fees_catv.txt ;
        echo "finished  pin_cycle_fees for cancel@" `date`
	backup
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

if [ "$grace_result" == 1 ] || [ "$add_on_grace_result" == 1 ];
then

        echo "Grace or addon grace maximum failed cases reached so cancel_fees will not run"
else
	
	if [ -e pin_cycle_fees_catv.txt ]
       	then
       	   echo "file is available";
       	   pid1=$(cat pin_cycle_fees_catv.txt)
	   if ps -p $pid1 > /dev/null
	   then
      	        echo " process is already running ";
           else
               echo " process is not running ";
               actions_cycle_fees
       	   fi
       else
        	actions_cycle_fees
       fi	
fi

cd /data/opt/portal/7.5/var/pin_billd/
`mv mso_fdp_set_graceperiod.log_bkp mso_fdp_set_graceperiod.log`
