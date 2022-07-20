#!/bin/ksh


if [ "$1" == "-input" -o "$1" == "-output" ]; then
   	echo "Executing : mso_bulk_entity_change.pl $1 $2 "
    	mso_bulk_entity_change.pl $1 $2
    	return_status=$?
    	if [ $return_status -eq 0 ]; then
		
		testnap hotfile 
		echo "Creating Jobs Wait..."
		mso_mta_job_processor -object_type /mso_mta_job/bulk_hierarchy  -verbose

        	#echo -e "Executing: mso_mta_bulk_entity_change -from_file hotfile\n"
		#mso_mta_bulk_entity_change -from_file hotfile -verbose
		mv hotfile hotfile_`date +%Y%m%d_%H%M%S`
		echo "Entity Change Process executed successfully"
    	fi
else
	echo "Usage:" ;
	echo "        FOR CREATING_ORDER:" ;
	echo "        -----------------" ;
	echo "        mso_bulk_entity_change.sh -input <input file name>" ;
	echo "        Ex: mso_bulk_entity_change.pl -input stb_input_file" ;
	echo "" ;
	echo "" ;
	echo "        TO GET_FAILED_ORDER_DETAILS:" ;
	echo "        --------------------------" ;
	echo "        mso_bulk_entity_change.sh -output <order_id>" ;
	echo "        Ex: mso_bulk_entity_change.pl -output BEC-20141229012319";
	echo ""

    	exit 1;
fi

