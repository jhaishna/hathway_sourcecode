#!/bin/ksh


if [ "$1" == "-payment_allocation" -a $# -eq 1 ]; then
	echo
	echo "Executing: mso_bulk_pymt_posting -object_type /mso_mta_job/payment_allocation "
        mso_pymt_allocation -object_type /mso_mta_job/payment_allocation -verbose

else
    #echo "Invalid input parameter"
    echo
    echo "Usage : $0 -payment_allocation"
    echo
    exit 1
fi
