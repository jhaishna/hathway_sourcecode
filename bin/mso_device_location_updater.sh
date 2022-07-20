#!/bin/ksh


if [ "$1" == "-device_type" -a "$3" == "-input" -a $# -eq 4 ]; then
    echo
    echo "Executing : mso_parse_update_location.pl -device_type $2 -input $4 "
    mso_parse_update_location.pl -device_type $2 -input $4
else
    #echo "Invalid input parameter"
    echo
    echo "Usage : $0 -device_type </device/xxx> -input <input_file_name>"
    echo
    exit 1
fi
