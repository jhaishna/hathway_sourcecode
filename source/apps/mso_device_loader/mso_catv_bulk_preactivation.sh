#!/bin/ksh


if [ "$1" == "-input" -a $# -eq 2 ]; then
    echo
    echo "Executing : mso_parse_create.pl -object_type /mso_mta_job/catv_preactivation -input $2 "
    mso_parse_create.pl -object_type /mso_mta_job/catv_preactivation -input $2
    return_status=$?
    echo; echo
    if [ $return_status -eq 0 ];then
        echo "Executing: mso_mta_job_processor -object_type /mso_mta_job/catv_preactivation "
        mso_mta_job_processor -object_type /mso_mta_job/catv_preactivation -verbose
    fi
else
    #echo "Invalid input parameter"
    echo
    echo "Usage : $0 -input <input_file_name>"
    echo
    exit 1
fi
