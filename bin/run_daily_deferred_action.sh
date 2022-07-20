#! /bin/bash
## Created on 19-MAY-2015

deferred_action()
{
         echo "runnng" > deferred_action.txt
	 pin_deferred_act -verbose
         rm deferred_action.txt;
}

cd $PIN_HOME/apps/pin_billd
echo "current working directory is `pwd`"

if [ -e deferred_action.txt ]
then
       echo "process is already running "
else
       echo "Executing the process deferred_action"
	deferred_action
fi

