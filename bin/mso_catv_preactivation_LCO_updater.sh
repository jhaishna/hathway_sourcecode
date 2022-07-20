#!/bin/ksh
if [ "$1" == "-input" -a $# -eq 2 ]; then
    echo
#if [ "$1" == "-device_type" -a "$3" == "-input" -a $# -eq 4 ]; then
    echo "Executing : mso_catv_preactivation_LCO_updater.pl -input $2 "
    mso_catv_preactivation_LCO_updater.pl -input $2
else
    #echo "Invalid input parameter"
    echo
    echo "Usage : $0 -input <input_file_name>"
    echo
    exit 1
fi

