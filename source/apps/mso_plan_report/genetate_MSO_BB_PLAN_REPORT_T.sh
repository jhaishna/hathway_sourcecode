#!/bin/bash

rm mso_plan_report
make clean all > /dev/null
if [ -f mso_plan_report ]; then
	mso_plan_report -service_type bb >mso_plan_report.log
else
	echo "executable mso_plan_report does not exist"
	exit
fi


if [ ! -f bb_plan_report.csv ] ;then
	echo "File bb_plan_report.csv does not exist"
	exit
else
	cd load_data
	sqlldr_cmd.sh >/dev/null	
fi
