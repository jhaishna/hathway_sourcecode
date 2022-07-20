#! /bin/bash
fname="osd.log"
fail="1"

cd /home/pin/opt/portal/7.5/mso/bin/
echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for osd in /home/pin/opt/portal/7.5/mso/bin/"
else
   echo "`date`:Testnap Connection Failed so job skipped for osd in /home/pin/opt/portal/7.5/mso/bin/" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
   exit;
fi

prog_name='OSD'
logfile='/home/pin/opt/portal/7.5/mso/apps/mso_bulk_audit/osd.log'

sh /home/pin/opt/portal/7.5/mso/apps/mso_bulk_audit/prerun.sh $prog_name > $logfile



osd_pk()
{
	 mso_mta_job_processor -verbose -object_type /mso_mta_job/bulk_osd_pk & > /dev/null
         echo $! > mta_job_osd_pk.txt
         pid=$(cat mta_job_osd_pk.txt)
         wait $pid
         rm mta_job_osd_pk.txt;
}

cisco_osd()
{
         mso_mta_job_processor -verbose -object_type /mso_mta_job/bulk_cisco_osd & > /dev/null
         echo $! > mta_job_cisco_osd.txt
         pid=$(cat mta_job_cisco_osd.txt)
         wait $pid
         rm mta_job_cisco_osd.txt;
}

cisco1_osd()
{
         mso_mta_job_processor -verbose -object_type /mso_mta_job/bulk_cisco1_osd & > /dev/null
         echo $! > mta_job_cisco1_osd.txt
         pid=$(cat mta_job_cisco1_osd.txt)
         wait $pid
         rm mta_job_cisco1_osd.txt;
}

# VERIMATRIX changes for OSD - BEGIN
vm_osd()
{
         mso_mta_job_processor -verbose -object_type /mso_mta_job/bulk_vm_osd & > /dev/null
         echo $! > mta_job_vm_osd.txt
         pid=$(cat mta_job_vm_osd.txt)
         wait $pid
         rm mta_job_vm_osd.txt;
}
# VERIMATRIX changes for OSD - END

log_check_function()
{
#checking the log to get the status of the report

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
                        echo "`date`:bulk_osd Job for PK/CISCO/CISCO1/VM Failed" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                else
                        echo "`date`:bulk_osd Job for PK/CISCO/CISCO1/VM Success" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
                fi
        fi

done
}

osd_log_clean_bup()
{

        `cat /home/pin/opt/portal/7.5/sys/test/$fname >> osd_bkp_curr.log`
        `:>/home/pin/opt/portal/7.5/sys/test/$fname`

}


cd $PIN_HOME/mso/apps/mso_bulk_utilities
echo "current working directory is `pwd`"

echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for osd in PIN_HOME/mso/apps/mso_bulk_utilities"
else
   echo "`date`:Testnap Connection Failed so job skipped for osd in PIN_HOME/mso/apps/mso_bulk_utilities" >> /data/opt/portal/7.5/var/cron_global/global_cron_tab.log
   exit;
fi

if [ -e mta_job_osd_pk.txt ]
then
       echo "file is available";
       pid1=$(cat mta_job_osd_pk.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
		osd_pk
       fi

else
       echo "file is not available ";
	osd_pk
fi

log_check_function
osd_log_clean_bup

if [ -e mta_job_cisco1_osd.txt ]
then
       echo "file is available";
       pid1=$(cat mta_job_cisco1_osd.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
                cisco1_osd
       fi

else
       echo "file is not available ";
        cisco1_osd
fi

log_check_function
osd_log_clean_bup

if [ -e mta_job_cisco_osd.txt ]
then
       echo "file is available";
       pid1=$(cat mta_job_cisco_osd.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
                cisco_osd
       fi

else
       echo "file is not available ";
        cisco_osd
fi

log_check_function
osd_log_clean_bup

# VERIMATRIX changes for OSD - BEGIN
if [ -e mta_job_vm_osd.txt ]
then
       echo "file is available";
       pid1=$(cat mta_job_vm_osd.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
                vm_osd
       fi

else
       echo "file is not available ";
        vm_osd
fi
# VERIMATRIX changes for OSD - END

log_check_function
osd_log_clean_bup

`mv /home/pin/opt/portal/7.5/sys/test/osd_bkp_curr.log $fname`

DB_STRING=`cat /home/pin/.pass/pin.db`
sqlplus -s ${DB_STRING} << EOF >> $logfile
@/home/pin/opt/portal/7.5/mso/apps/mso_bulk_audit/postrun.sql '$prog_name'
exit;
EOF

