#! /bin/bash
fname="bmail.log"
fail="1"

cd /home/pin/Thirdparty/BRM12/BRM/mso/bin/
echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for bmail in /home/pin/Thirdparty/BRM12/BRM/mso/bin/"
else
   echo "`date`:Testnap Connection Failed so job skipped for bmail in /home/pin/Thirdparty/BRM12/BRM/mso/bin/" >> /home/pin/Thirdparty/BRM12/BRM/var/cron_global/global_cron_tab.log
   exit;
fi

email_pk()
{
	 mso_mta_job_processor -verbose -object_type /mso_mta_job/bulk_email_pk & > /dev/null
         echo $! > mta_job_email_pk.txt
         pid=$(cat mta_job_email_pk.txt)
         wait $pid
         rm mta_job_email_pk.txt;
}

cisco_bmail()
{
         mso_mta_job_processor -verbose -object_type /mso_mta_job/bulk_cisco_bmail & > /dev/null
         echo $! > mta_job_cisco_bmail.txt
         pid=$(cat mta_job_cisco_bmail.txt)
         wait $pid
         rm mta_job_cisco_bmail.txt;
}

cisco1_bmail()
{
         mso_mta_job_processor -verbose -object_type /mso_mta_job/bulk_cisco1_bmail & > /dev/null
         echo $! > mta_job_cisco1_bmail.txt
         pid=$(cat mta_job_cisco1_bmail.txt)
         wait $pid
         rm mta_job_cisco1_bmail.txt;
}

# VERIMATRIX changes for BMAIL - BEGIN
vm_bmail()
{
         mso_mta_job_processor -verbose -object_type /mso_mta_job/bulk_vm_bmail & > /dev/null
         echo $! > mta_job_vm_bmail.txt
         pid=$(cat mta_job_vm_bmail.txt)
         wait $pid
         rm mta_job_vm_bmail.txt;
}
# VERIMATRIX changes for BMAIL - END

log_check_function()
{
#checking the log to get the status of the report

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
                        echo "`date`:bulk_bmail Job for PK/CISCO/CISCO1 Failed" >> /home/pin/Thirdparty/BRM12/BRM/var/cron_global/global_cron_tab.log
                else
                        echo "`date`:bulk_bmail Job for PK/CISCO/CISCO1 Success" >> /home/pin/Thirdparty/BRM12/BRM/cron_global/global_cron_tab.log
                fi
        fi

done
}

bmail_log_clean_bup()
{

        `cat /home/pin/opt/portal/7.5/sys/test/$fname >> bmail_bkp_curr.log`
        `:>/home/pin/opt/portal/7.5/sys/test/$fname`
}


cd $PIN_HOME/mso/apps/mso_bulk_utilities
echo "current working directory is `pwd`"

echo q | testnap;
Rstatus=$?
if [ $Rstatus -eq 0 ] ; then
   echo "`date`:Testnap Connection Success for bmail in PIN_HOME/mso/apps/mso_bulk_utilities"
else
   echo "`date`:Testnap Connection Failed so job skipped for bmail in PIN_HOME/mso/apps/mso_bulk_utilities" >> /home/pin/Thirdparty/BRM12/BRM/var/cron_global/global_cron_tab.log
   exit;
fi


if [ -e mta_job_email_pk.txt ]
then
       echo "file is available";
       pid1=$(cat mta_job_email_pk.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
		email_pk
       fi

else
       echo "file is not available ";
	email_pk
fi

log_check_function
bmail_log_clean_bup

if [ -e mta_job_cisco1_bmail.txt ]
then
       echo "file is available";
       pid1=$(cat mta_job_cisco1_bmail.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
                cisco1_bmail
       fi

else
       echo "file is not available ";
        cisco1_bmail
fi

log_check_function
bmail_log_clean_bup

if [ -e mta_job_cisco_bmail.txt ]
then
       echo "file is available";
       pid1=$(cat mta_job_cisco_bmail.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
                cisco_bmail
       fi

else
       echo "file is not available ";
        cisco_bmail
fi

log_check_function
bmail_log_clean_bup

# VERIMATRIX changes for BMAIL - BEGIN
if [ -e mta_job_vm_bmail.txt ]
then
       echo "file is available";
       pid1=$(cat mta_job_vm_bmail.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
                vm_bmail
       fi

else
       echo "file is not available ";
        vm_bmail
fi
# VERIMATRIX changes for BMAIL - END

log_check_function
bmail_log_clean_bup

`mv /home/pin/Thirdparty/BRM12/BRM/sys/test/bmail_bkp_curr.log $fname`
