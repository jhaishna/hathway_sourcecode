DB_CONNECTION_STRING_INF="pin103/pin103@$ORACLE_SID"
echo "Starting the script: "
echo `date`
echo $1

if [ $1 -eq "0" ]; then
echo "No Date Passed"
echo "mso_process_dsa_commission -verbose -end_date 0"
	mso_process_dsa_commission -verbose -end_date 0 
else
	dt=`echo $1 | awk -F'/' '{ print $1 }'`
	mon=`echo $1 | awk -F'/' '{ print $2 }'`
	year=`echo $1 | awk -F'/' '{ print $3 }'`

	case $mon
	  in
		"01") moth="Jan";;
		"02") moth="Feb";;
		"03") moth="Mar";;
		"04") moth="Apr";;
		"05") moth="May";;
		"06") moth="Jun";;
		"07") moth="Jul";;
		"08") moth="Aug";;
		"09") moth="Sep";;
		"10") moth="Oct";;
		"11") moth="Nov";;
		"12") moth="Dec";;
		*)  echo "Not a Valid Month";
       	 exit;;
	esac

	sql_query="select ora2inf('$dt-$moth-$year') from dual;"
echo "sql_query = $sql_query"
	EXT_DATE=`sqlplus -s ${DB_CONNECTION_STRING_INF} << EOT
        set pagesize 0 head off echo off feedback off linesize 10000
        $sql_query
        exit;
	EOT`
echo $?
	EXT_DATE=`echo $EXT_DATE | sed -e 's/[ \t]*//g'`
echo "Unix Date is $EXT_DATE"
echo "mso_process_dsa_commission -verbose -end_date $EXT_DATE"
	mso_process_dsa_commission -verbose -end_date $EXT_DATE
fi

echo "Script finished:"
echo `date`
