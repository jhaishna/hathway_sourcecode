#!/bin/bash

ARGS_COUNT=$#
#if [ ( $# -ne 4 ) -o ( "$1" != "-input" ) -o ( "$3" != "-output" ) ]
if [ $# -ne 4 -o "$1" != "-input" -o "$3" != "-output" ]
then
	echo -e "\nUSAGE: mso_catv_cfg_pt_updater.sh -input <input_file> -output <output_file>\n"
	exit;
fi

INPUT_FILE=$2

OUTPUT_FILE=$4
:>$OUTPUT_FILE

echo "Updaing file:'$INPUT_FILE' containing $RECORDS records"

#for record in "`cat $INPUT_FILE`"
records_count=`wc -l $INPUT_FILE|awk '{print $1}'`

for (( count=1; count<=$records_count; count++ ))
do
	record=`cat $INPUT_FILE|sed -n ${count}p`
	PROV_TAG=`echo $record |awk -F"," '{print $1}'`
	CA_ID=`echo $record |awk -F"," '{print $2}'`
	NODE=`echo $record |awk -F"," '{print $3}'`

	#if [ "$CA_ID" != "NA" -a "$CA_ID" != "" ]
	if [ "$CA_ID" != "" ]
	then
	
		echo "UPDATE catv_ne_info_t SET network_node = '$NODE' WHERE OBJ_ID0 = ( SELECT OBJ_ID0 FROM CATV_NE_INFO_T M2 , MSO_CFG_CATV_PT_T M1 WHERE M1.POID_ID0 = M2.OBJ_ID0 AND M1.PROVISIONING_TAG = '$PROV_TAG' AND M2.CA_ID = '$CA_ID' ) and CA_ID = '$CA_ID';" >>$OUTPUT_FILE
	
		echo "UPDATE catv_ne_info_t SET network_node = '$NODE' WHERE OBJ_ID0 = ( SELECT OBJ_ID0 FROM CATV_NE_INFO_T M2 , MSO_CFG_CATV_PT_T M1 WHERE M1.POID_ID0 = M2.OBJ_ID0 AND M1.PROVISIONING_TAG = '$PROV_TAG' AND M2.CA_ID = '$CA_ID' ) and CA_ID = '$CA_ID';"
	fi
done
