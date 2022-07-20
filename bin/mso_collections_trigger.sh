#!/bin/ksh


if [ "$1" == "-collections" -a $# -eq 2 ]; then
	echo
	echo "Executing: mso_collections_notification -object_type /mso_mta_job/collections/$2 "
        mso_collections_notification -object_type /mso_mta_job/collections/$2 -verbose

else
    #echo "Invalid input parameter"
    echo
    echo "Usage : $0 -collections <reminder_X>"
    echo
    exit 1
fi
