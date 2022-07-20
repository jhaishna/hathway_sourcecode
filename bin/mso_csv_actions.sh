#!/bin/ksh


mta_input_file_path="$MSO_HOME/apps/mso_bulk_utilities/data/csv_creation/done"
mta_failed_file_path="$MSO_HOME/apps/mso_bulk_utilities/log/csv_creation/"


if [ "$1" == "-input" -a $# -eq 2 ]; then
    echo
    echo "Executing : MSO_CSV_CREATOR.pl -input $2 "
    MSO_CSV_CREATOR.pl $1 $2
   # return_status=$?
   # echo; echo
   # if [ $return_status -eq 0 ];then
      echo "Execution completed. "
    #fi
else
    #echo "Invalid input parameter"
    echo
    echo "Usage : $0 -input <input_file_name> "
    echo
    exit 1
fi


#######################################
#    FOR MTA PROCESS		      #	
#######################################

:>nohup.out #clearing nohup of current directory

file_name=$2

#pin_apply_bulk_adjustment -v -f $mta_input_file_path/$file_name.csv

#cp pin_mta_failed.csv $mta_failed_file_path/pin_bulk_adjust_mta_failed.csv
#echo "nohup pin_apply_bulk_adjustment -v -f $mta_input_file_path/$file_name.csv &"
