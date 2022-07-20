#!/bin/sh
TEMPLATE=./template_channel_master
IP_FILE=./ip_channel_master
TESTNAP_DIR="$PIN_HOME/sys/test"
CURRENT_DIR=`echo $PWD`
IPL_DIR=$CURRENT_DIR"/input/"
PROCESSED_DIR=$CURRENT_DIR"/processed/"
i=0
#for line in `cat $IP_FILE`
while read line
	do
		i=`expr $i + 1`
		TEMPLATE_NEW="./input/"$TEMPLATE"_"$i".ipl"
		`cp $TEMPLATE $TEMPLATE_NEW`
		F1=`echo $line | cut -f1 -d','`
		F2=`echo $line | cut -f2 -d','`
		F3=`echo $line | cut -f3 -d','`
		F4=`echo $line | cut -f4 -d','`
		F5=`echo $line | cut -f5 -d','`
		F6=`echo $line | cut -f6 -d','`
		sed "s/~GENRE~/$F4/" $TEMPLATE_NEW > tmp_template
		`mv tmp_template $TEMPLATE_NEW`
		sed "s/~ID~/$F2/" $TEMPLATE_NEW > tmp_template
		`mv tmp_template $TEMPLATE_NEW`
		sed "s/~NAME~/$F3/" $TEMPLATE_NEW > tmp_template
		`mv tmp_template $TEMPLATE_NEW`
		sed "s/~TYPE~/$F1/" $TEMPLATE_NEW > tmp_template
		`mv tmp_template $TEMPLATE_NEW`
		sed "s/~RATE~/$F5/" $TEMPLATE_NEW > tmp_template
		`mv tmp_template $TEMPLATE_NEW`
		sed "s/~PAID~/$F6/" $TEMPLATE_NEW > tmp_template
		`mv tmp_template $TEMPLATE_NEW`
	done < $IP_FILE
echo "Total records processed from $IP_FILE : $i"
cd $IPL_DIR
`ls -l *.ipl | grep ^- | awk '{print $9}' > IPL_LST`
for line in `cat $IPL_DIR"/IPL_LST"`
	do
		cd $TESTNAP_DIR
		testnap $IPL_DIR"/"$line >> log.txt
		`mv $IPL_DIR"/"$line $PROCESSED_DIR"/"$line`
	done	
exit
