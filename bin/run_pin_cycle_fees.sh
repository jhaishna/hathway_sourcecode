#! /bin/bash

OPTION=$1

if [ $OPTION == "help" ]
then
	echo "Usage:"
	echo "./run_pin_cycle_fees <action>"
	echo "action: cancel | regular"
	exit 1
fi

cd $PIN_HOME/mso/scripts/pin_cycle_fees
echo "current working directory is `pwd`"

if [ $OPTION == "cancel" ]
then
    echo "Calling run_cycle_fees_cancel"
#    ./run_cycle_fees_cancel
elif [ $OPTION == "regular" ]
then
    echo "Calling run_cycle_fees_regular"
#    ./run_cycle_fees_regular
fi
