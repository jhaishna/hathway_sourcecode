#! /bin/bash

fname="retrack.log"
fail="1"

cd /home/pin/opt/portal/7.5/mso/bin/
echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for retrack in /home/pin/opt/portal/7.5/mso/bin/"
else
   echo "`date`:Testnap Connection Failed so job skipped for retrack in /home/pin/opt/portal/7.5/mso/bin/" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
   exit;
fi

nds_retrack()
{
	 mso_mta_job_processor -verbose -object_type /mso_mta_job/bulk_nds_retrack & > /dev/null
         echo $! > mta_job_nds_retrack.txt
         pid=$(cat mta_job_nds_retrack.txt)
         wait $pid
         rm mta_job_nds_retrack.txt;
}

cisco_retrack()
{
         mso_mta_job_processor -verbose -object_type /mso_mta_job/bulk_cisco_retrack & > /dev/null
         echo $! > mta_job_cisco_retrack.txt
         pid=$(cat mta_job_cisco_retrack.txt)
         wait $pid
         rm mta_job_cisco_retrack.txt;
}

cisco1_retrack()
{
         mso_mta_job_processor -verbose -object_type /mso_mta_job/bulk_cisco1_retrack & > /dev/null
         echo $! > mta_job_cisco1_retrack.txt
         pid=$(cat mta_job_cisco1_retrack.txt)
         wait $pid
         rm mta_job_cisco1_retrack.txt;
}

# VERIMATRIX changes for RETRACK
vm_retrack()
{
	 mso_mta_job_processor -verbose -object_type /mso_mta_job/bulk_vm_retrack & > /dev/null
         echo $! > mta_job_vm_retrack.txt
         pid=$(cat mta_job_vm_retrack.txt)
         wait $pid
         rm mta_job_vm_retrack.txt;
}

retrack_log_clean_bup()
{
	
	`cat /home/pin/opt/portal/7.5/sys/test/$fname >> retrack_bkp_curr.log`
	`:>/home/pin/opt/portal/7.5/sys/test/$fname`
	
}

cd $PIN_HOME/mso/apps/mso_bulk_utilities
echo "current working directory is `pwd`"

echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for retrack in PIN_HOME/mso/apps/mso_bulk_utilities"
else
   echo "`date`:Testnap Connection Failed so job skipped for retrack in PIN_HOME/mso/apps/mso_bulk_utilities" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
   exit;
fi

if [ -e mta_job_nds_retrack.txt ]
then
       echo "file is available";
       pid1=$(cat mta_job_nds_retrack.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
		nds_retrack
       fi

else
       echo "file is not available ";
	nds_retrack
fi

#checking the log to get the status of the report
#fname="retrack.log"
#fail="1"

exec < /home/pin/opt/portal/7.5/sys/test/$fname


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
                        echo "`date`:retrack for nds Job Failed" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                else
                        echo "`date`:retrack for nds Job Success" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                fi
        fi
	

done

retrack_log_clean_bup

if [ -e mta_job_cisco_retrack.txt ]
then
       echo "file is available";
       pid1=$(cat mta_job_cisco_retrack.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
                cisco_retrack
       fi

else
       echo "file is not available ";
        cisco_retrack
fi

#checking the log to get the status of the report
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
                        echo "`date`:retrack for cisco Job Failed" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                else
                        echo "`date`:retrack for cisco Job Success" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                fi
        fi


done

retrack_log_clean_bup

if [ -e mta_job_cisco1_retrack.txt ]
then
       echo "file is available";
       pid1=$(cat mta_job_cisco1_retrack.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
                cisco1_retrack
       fi

else
       echo "file is not available ";
        cisco1_retrack
fi

#checking the log to get the status of the report
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
                        echo "`date`:retrack for cisco1 Job Failed" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                else
                        echo "`date`:retrack for cisco1 Job Success" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                fi
        fi


done

retrack_log_clean_bup

# VERIMATRIX changes for RETRACK - BEGIN
if [ -e mta_job_vm_retrack.txt ]
then
       echo "file is available";
       pid1=$(cat mta_job_vm_retrack.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
		vm_retrack
       fi

else
       echo "file is not available ";
	vm_retrack
fi

#checking the log to get the status of the report
#fname="retrack.log"
#fail="1"

exec < /home/pin/opt/portal/7.5/sys/test/$fname


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
                        echo "`date`:retrack for vm Job Failed" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                else
                        echo "`date`:retrack for vm Job Success" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                fi
        fi
	

done

# VERIMATRIX changes for RETRACK - END

retrack_log_clean_bup

`mv /home/pin/opt/portal/7.5/sys/test/retrack_bkp_curr.log $fname`
