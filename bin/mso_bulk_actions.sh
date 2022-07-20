#!/bin/ksh


if [ "$1" == "-bulk_action" -a "$3" == "-input" -a $# -eq 4 ]; then
    echo
    echo "Executing : mso_bulk_parse_create.pl -object_type /mso_mta_job$2 -input $4 "
    mso_bulk_parse_create.pl -object_type /mso_mta_job/$2 -input $4
    return_status=$?
    echo; echo
    if [ $return_status -eq 0 ];then
        echo "Executing: mso_mta_job_processor -object_type /mso_mta_job$2 "
        mso_mta_job_processor -object_type /mso_mta_job$2 -verbose
    fi
else
    #echo "Invalid input parameter"
    echo
    echo "Usage : $0 -bulk_action </bulk_XXXXX> -input <input_file_name>"
    echo
    exit 1
fi
