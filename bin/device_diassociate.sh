#/usr/bin/sh
#!/bin/sh
DB_STRING=`cat /home/pin/.pass/pin.db`
cd /home/pin/opt/portal/7.5/mso/apps/mso_operations/input_disassociation

echo 'File to be read is '$1
total_accounts=`cat $1 | wc -l`
echo 'Total number of accounts for status change is :' $total_accounts
a=1

now=$(date +"%m-%d-%Y "-" %T")
`:>/home/pin/opt/portal/7.5/mso/apps/mso_operations/input_disassociation/output/disassociation_failed.txt`
`:>/home/pin/opt/portal/7.5/mso/apps/mso_operations/input_disassociation/output/disassociation_Success.txt`

while [ $a -le $total_accounts ]
do
        cd /home/pin/opt/portal/7.5/mso/apps/mso_operations/input_disassociation

        b=`cat $1|head -$a|tail -1|awk -F"," '{print $1}'`
        c=`cat $1|head -$a|tail -1|awk -F"," '{print $2}'`
        d=`cat $1|head -$a|tail -1|awk -F"," '{print $3}'`
        e=`cat $1|head -$a|tail -1|awk -F"," '{print $4}'`

        #############################################################################################
        #       Checking for servicee status                                                        #
        #############################################################################################
        service_status=`sqlplus -silent ${DB_STRING} <<EOF
        SET PAGESIZE 0 FEEDBACK OFF VERIFY OFF HEADING OFF ECHO OFF
                select status from service_t where poid_id0 = $b;
        EXIT;
EOF`

        echo $service_status
        echo $b

        if [[ -z "$service_status" ]];then
                echo "Invalid service please check the service $b">>/home/pin/opt/portal/7.5/mso/apps/mso_operations/input_disassociation/output/disassociation_failed.txt
        elif [ $service_status == 10100 ] ||  [ $service_status == 10102 ];then
                echo "Servicee $b is not terminated so device failed to release for the account $d - $now">>/home/pin/opt/portal/7.5/mso/apps/mso_operations/input_disassociation/output/disassociation_failed.txt
        else
		echo "r << XXX 1" >> /home/pin/opt/portal/7.5/mso/apps/mso_operations/input_disassociationinput_flist_$a
                echo "0 PIN_FLD_SERVICES      ARRAY [0] allocated 20, used 1">>/home/pin/opt/portal/7.5/mso/apps/mso_operations/input_disassociationinput_flist_$a
                        echo "1     PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/catv $b 0">>/home/pin/opt/portal/7.5/mso/apps/mso_operations/input_disassociationinput_flist_$a
                        echo "1     PIN_FLD_ACCOUNT_OBJ    POID [0] 0.0.0.1 /account $d 0">>/home/pin/opt/portal/7.5/mso/apps/mso_operations/input_disassociationinput_flist_$a
                        echo "0 PIN_FLD_PROGRAM_NAME    STR [0] \"Automatic Service release\"">>/home/pin/opt/portal/7.5/mso/apps/mso_operations/input_disassociationinput_flist_$a
                        echo "0 PIN_FLD_FLAGS           INT [0] 0">>/home/pin/opt/portal/7.5/mso/apps/mso_operations/input_disassociationinput_flist_$a
                        echo "0 PIN_FLD_POID           POID [0] 0.0.0.1 /device/$e $c 10">>/home/pin/opt/portal/7.5/mso/apps/mso_operations/input_disassociationinput_flist_$a
                echo "XXX" >> /home/pin/opt/portal/7.5/mso/apps/mso_operations/input_disassociationinput_flist_$a
                echo "xop 2703 0 1" >>/home/pin/opt/portal/7.5/mso/apps/mso_operations/input_disassociationinput_flist_$a
                cd /home/pin/opt/portal/7.5/sys/test/
                testnap /home/pin/opt/portal/7.5/mso/apps/mso_operations/input_disassociationinput_flist_$a
                echo "Device_id $e - $c released for the account $d successfully">>/home/pin/opt/portal/7.5/mso/apps/mso_operations/input_disassociation/output/disassociation_Success.txt
        fi
#rm /home/pin/opt/portal/7.5/mso/apps/mso_operations/input_disassociationinput_flist_$a

a=`expr $a + 1`

done
