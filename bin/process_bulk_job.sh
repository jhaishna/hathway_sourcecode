#! /bin/bash

grv_uploader()
{
         mso_mta_job_processor -object_type /mso_mta_job/grv_uploader & > /dev/null
         echo $! > mta_job_grv_uploader.txt
         pid=$(cat mta_job_grv_uploader.txt)
         wait $pid
         rm mta_job_grv_uploader.txt;
}

act_adj()
{
         mso_mta_job_processor -object_type /mso_mta_job/bulk_act_adj & > /dev/null
         echo $! > mta_job_act_adj.txt
         pid=$(cat mta_job_act_adj.txt)
         wait $pid
         rm mta_job_act_adj.txt;
}

bulk_add_mb_gb()
{
         mso_mta_job_processor -object_type /mso_mta_job/bulk_add_mb_gb & > /dev/null
         echo $! > bulk_add_mb_gb.txt
         pid=$(cat bulk_add_mb_gb.txt)
         wait $pid
         rm bulk_add_mb_gb.txt;
}

bulk_add_plan()
{
         mso_mta_job_processor -object_type /mso_mta_job/bulk_add_plan & > /dev/null
         echo $! > bulk_add_plan.txt
         pid=$(cat bulk_add_plan.txt)
         wait $pid
         rm bulk_add_plan.txt;
}


bulk_bill_hold()
{
         mso_mta_job_processor -object_type /mso_mta_job/bulk_bill_hold & > /dev/null
         echo $! > bulk_bill_hold.txt
         pid=$(cat bulk_bill_hold.txt)
         wait $pid
         rm bulk_bill_hold.txt;
}

########################################
bulk_change_plan()
{
         mso_mta_job_processor -object_type /mso_mta_job/bulk_change_plan & > /dev/null
         echo $! > bulk_change_plan.txt
         pid=$(cat bulk_change_plan.txt)
         wait $pid
         rm bulk_change_plan.txt;
}

bulk_check_bounce()
{
         mso_mta_job_processor -object_type /mso_mta_job/bulk_check_bounce & > /dev/null
         echo $! > bulk_check_bounce.txt
         pid=$(cat bulk_check_bounce.txt)
         wait $pid
         rm bulk_check_bounce.txt;
}

bulk_extend_validity()
{
         mso_mta_job_processor -object_type /mso_mta_job/bulk_extend_validity & > /dev/null
         echo $! > bulk_extend_validity.txt
         pid=$(cat bulk_extend_validity.txt)
         wait $pid
         rm bulk_extend_validity.txt;
}

bulk_reconnection()
{
         mso_mta_job_processor -object_type /mso_mta_job/bulk_reconnection & > /dev/null
         echo $! > bulk_reconnection.txt
         pid=$(cat bulk_reconnection.txt)
         wait $pid
         rm bulk_reconnection.txt;
}

bulk_refund_load()
{
         mso_mta_job_processor -object_type /mso_mta_job/bulk_refund_load & > /dev/null
         echo $! > bulk_refund_load.txt
         pid=$(cat bulk_refund_load.txt)
         wait $pid
         rm bulk_refund_load.txt;
}

bulk_refund_rev_load()
{
         mso_mta_job_processor -object_type /mso_mta_job/bulk_refund_rev_load & > /dev/null
         echo $! > bulk_refund_rev_load.txt
         pid=$(cat bulk_refund_rev_load.txt)
         wait $pid
         rm bulk_refund_rev_load.txt;
}
bulk_remove_plan()
{
         mso_mta_job_processor -object_type /mso_mta_job/bulk_remove_plan & > /dev/null
         echo $! > bulk_remove_plan.txt
         pid=$(cat bulk_remove_plan.txt)
         wait $pid
         rm bulk_remove_plan.txt;
}
bulk_renew()
{
         mso_mta_job_processor -object_type /mso_mta_job/bulk_renew & > /dev/null
         echo $! > bulk_renew.txt
         pid=$(cat bulk_renew.txt)
         wait $pid
         rm bulk_renew.txt;
}

bulk_swap_cpe()
{
         mso_mta_job_processor -object_type /mso_mta_job/bulk_swap_cpe & > /dev/null
         echo $! > bulk_swap_cpe.txt
         pid=$(cat bulk_swap_cpe.txt)
         wait $pid
         rm bulk_swap_cpe.txt;
}
bulk_termination()
{
         mso_mta_job_processor -object_type /mso_mta_job/bulk_termination & > /dev/null
         echo $! > bulk_termination.txt
         pid=$(cat bulk_termination.txt)
         wait $pid
         rm bulk_termination.txt;
}
bulk_topup()
{
         mso_mta_job_processor -object_type /mso_mta_job/bulk_topup & > /dev/null
         echo $! > bulk_topup.txt
         pid=$(cat bulk_topup.txt)
         wait $pid
         rm bulk_topup.txt;
}

bulk_writeoff_cpe()
{
         mso_mta_job_processor -object_type /mso_mta_job/bulk_writeoff_cpe & > /dev/null
         echo $! > bulk_writeoff_cpe.txt
         pid=$(cat bulk_writeoff_cpe.txt)
         wait $pid
         rm bulk_writeoff_cpe.txt;
}
bulk_suspension()
{
         mso_mta_job_processor -object_type /mso_mta_job/bulk_suspension & > /dev/null
         echo $! > bulk_suspension.txt
         pid=$(cat bulk_suspension.txt)
         wait $pid
         rm bulk_suspension.txt;
}
bulk_crdr_note()
{
         mso_mta_job_processor -object_type /mso_mta_job/bulk_crdr_note & > /dev/null
         echo $! > bulk_crdr_note.txt
         pid=$(cat bulk_crdr_note.txt)
         wait $pid
         rm bulk_crdr_note.txt;
}


cd $PIN_HOME/mso/apps/mso_bulk_utilities
echo "current working directory is `pwd`"

if [ -e mta_job_grv_uploader.txt ]
then
       #echo "file is available";
       pid1=$(cat mta_job_grv_uploader.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
                grv_uploader
       fi

else
       echo "file is not available "
       grv_uploader
fi


if [ -e mta_job_act_adj.txt ]
then
       #echo "file is available";
       pid1=$(cat mta_job_act_adj.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
                act_adj
       fi

else
       echo "file is not available "
       act_adj
fi

if [ -e bulk_add_mb_gb.txt ]
then
       #echo "file is available";
       pid1=$(cat bulk_add_mb_gb.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
                bulk_add_mb_gb
       fi

else
       echo "file is not available "
       bulk_add_mb_gb
fi

if [ -e bulk_add_plan.txt ]
then
       #echo "file is available";
       pid1=$(cat bulk_add_plan.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
                bulk_add_plan
       fi

else
       echo "file is not available "
       bulk_add_plan
fi

if [ -e bulk_bill_hold.txt ]
then
       #echo "file is available";
       pid1=$(cat bulk_bill_hold.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
                bulk_bill_hold
       fi

else
       echo "file is not available "
       bulk_bill_hold
fi

if [ -e bulk_change_plan.txt ]
then
       #echo "file is available";
       pid1=$(cat bulk_change_plan.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
                bulk_change_plan
       fi

else
       echo "file is not available "
       bulk_change_plan
fi

if [ -e bulk_check_bounce.txt ]
then
       #echo "file is available";
       pid1=$(cat bulk_check_bounce.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
                bulk_check_bounce
       fi

else
       echo "file is not available "
       bulk_check_bounce
fi

if [ -e bulk_extend_validity.txt ]
then
       #echo "file is available";
       pid1=$(cat bulk_extend_validity.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
                bulk_extend_validity
       fi

else
       echo "file is not available "
       bulk_extend_validity
fi

if [ -e bulk_reconnection.txt ]
then
       #echo "file is available";
       pid1=$(cat bulk_reconnection.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
                bulk_reconnection
       fi

else
       echo "file is not available "
       bulk_reconnection
fi

if [ -e bulk_refund_load.txt ]
then
       #echo "file is available";
       pid1=$(cat bulk_refund_load.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
                bulk_refund_load
       fi

else
       echo "file is not available "
       bulk_refund_load
fi

if [ -e bulk_refund_rev_load.txt ]
then
       #echo "file is available";
       pid1=$(cat bulk_refund_rev_load.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
                bulk_refund_rev_load
       fi

else
       echo "file is not available "
       bulk_refund_rev_load
fi

if [ -e bulk_remove_plan.txt ]
then
       #echo "file is available";
       pid1=$(cat bulk_remove_plan.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
                bulk_remove_plan
       fi

else
       echo "file is not available "
       bulk_remove_plan
fi

if [ -e bulk_renew.txt ]
then
       #echo "file is available";
       pid1=$(cat bulk_renew.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
                bulk_renew
       fi

else
       echo "file is not available "
       bulk_renew
fi

if [ -e bulk_swap_cpe.txt ]
then
       #echo "file is available";
       pid1=$(cat bulk_swap_cpe.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
                bulk_swap_cpe
       fi

else
       echo "file is not available "
       bulk_swap_cpe
fi

if [ -e bulk_termination.txt ]
then
       #echo "file is available";
       pid1=$(cat bulk_termination.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
                bulk_termination
       fi

else
       echo "file is not available "
       bulk_termination
fi

if [ -e bulk_topup.txt ]
then
       #echo "file is available";
       pid1=$(cat bulk_topup.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
                bulk_topup
       fi

else
       echo "file is not available "
       bulk_topup
fi

if [ -e bulk_writeoff_cpe.txt ]
then
       #echo "file is available";
       pid1=$(cat bulk_writeoff_cpe.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
                bulk_writeoff_cpe
       fi

else
       echo "file is not available "
       bulk_writeoff_cpe
fi

if [ -e bulk_suspension.txt ]
then
       #echo "file is available";
       pid1=$(cat bulk_suspension.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
                bulk_suspension
       fi

else
       echo "file is not available "
       bulk_suspension
fi

if [ -e bulk_crdr_note.txt ]
then
       #echo "file is available";
       pid1=$(cat bulk_crdr_note.txt)

       if ps -p $pid1 > /dev/null
       then
               echo " process is already running "
       else
               echo " process is not running "
                bulk_crdr_note
       fi

else
       echo "file is not available "
       bulk_crdr_note
fi


