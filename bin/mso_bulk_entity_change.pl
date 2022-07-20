#!/home/pin/opt/portal/7.5/ThirdParty/perl/5.8.0/bin/perl

use pcmif;
use Sys::Hostname;
use File::Copy;
use File::Basename;
use File::Path;
use POSIX qw(strftime);

$SEVERE = 0;
$ERROR = 1;
$INFO = 2;
$DEBUG = 3;
$MSO_OP_INTEG_CREATE_JOB = 11056;
$MSO_OP_INTEG_GET_FAILED_ORDER_DETAILS = 11062;

$args = $#ARGV + 1;
#foreach (@ARGV) {
#        print "$_\n";
#}


#======================================================
# Input parameter validation
#======================================================
if($#ARGV != 1)
{
#        print "\nInvalid input parameter";
        print "\nUsage: \n\tFOR CREATING_ORDER:\n\t-----------------\n\tmso_bulk_entity_change.pl -input <input file name> ";
        print "\n\tEx: mso_bulk_entity_change.pl -input stb_input_file \n";

        print "\n\n\tTO GET_FAILED_ORDER_DETAILS:\n\t--------------------------\n\tmso_bulk_entity_change.pl -output <order_id> ";
        print "\n\tEx: mso_bulk_entity_change.pl -output BEC-20141229012319 \n";
        exit 1;
}

if ( $ARGV[0] eq '-input' )
{

	#======================================================
	# Read the configuration file and 
	# get the loglevel and  opcode number 
	#======================================================
	$conf_file = "./pin.conf";
	open (CONF, $conf_file) || ( print "\nSEVERE: Configuration file does not exist.. Exiting\n" and  exit 1 );
	while (<CONF>) {
		$conf = $_ ;
		if ($conf  =~ /mso_bulk_entity_change/ ) {
			
			#if ($conf  =~ /$ARGV[1] opcode/) {
			if ($conf  =~ /@object_values[3] opcode/) {
				@conf_values = split(/ /,$conf);
				$opcode =  @conf_values[4];
				#printf( " conf values \n  $opcode");
			}
			elsif ($conf  =~ /loglevel/ ) {
				@conf_values = split(/ /,$conf);
				$LOGLEVEL =  @conf_values[3];
			}
		}
	}

	#if (length $opcode <= 0) {
	#    print "\nSEVERE: Opcode number not configured for $ARGV[1] in pin.conf file.. Exiting\n";
	#    exit 1;
	#}

	#if (length $LOGLEVEL <= 0) {
	#    print "\nSEVERE: loglevel not configured for mso_bulk_entity_change in pin.conf file.. Exiting\n";
	#    exit 1;
	#}

	#======================================================
	# Set path for template file, flist file, input file
	# log file, reject file and error file 
	#======================================================

	$input_file_name = $ARGV[1];
	$app_type = "bulk_entity_change";

	$template_file = "./template/" . $app_type . "/" . $app_type . ".TMPL";
	$reason_code_file =  "./template/" . $app_type . "/" . "reasonscode.list";
	#$flist_file    = "./template/" . $app_type . "/" . $app_type . ".IFL";
	$input_file    = "./data/" . $app_type . "/input/" . $input_file_name;
	$log_file      = "./log/" . $app_type . "/" . $input_file_name . ".log";
	$reject_file   = "./data/" . $app_type . "/reject/" . $input_file_name . ".reject"; #complete batch rejection
	$error_file    = "./data/" . $app_type . "/error/" . $input_file_name . ".error";   # specific record which failed during processing
	printf("$input_file");

	open ( INP, $input_file)     || ( print "\nSEVERE: Input file does not exist.. Exiting\n" and  exit 1 );
	open ( TMPL, $template_file) || ( print "\nSEVERE: Template file does not exist.. Exiting\n" and  exit 1) ;
	open ( RES_CODE, $reason_code_file) || ( print "\nSEVERE: reason code file does not exist.. Exiting\n" and  exit 1) ;
	#open (FLIST, $flist_file)   || ( print "\nSEVERE: Flist file does not exist.. Exiting\n" and  exit 1);

	#===========================================================
	# Insanity check if the input file is already processed.
	# Check log file, reject file or error file already exist
	#==========================================================
	if (-e $log_file ) {
		print "\nSEVERE: Log file $log_file already exist. Please change the input Input file name or backup the log file.. Exiting\n" ;  
		exit 1 
	}

	if (-e $error_file ) {
		print "\nSEVERE: Error file $error_file already exist. Please change the input Input file name or backup the error file.. Exiting\n" ;  
		exit 1 
	}

	if (-e $reject_file ) {
		print "\nSEVERE: Reject file $reject_file already exist. Please change the input Input file name or backup the reject file.. Exiting\n" ;  
		exit 1 
	}

	open ( LOG, ">$log_file")   || ( print "\nSEVERE: Can't open log file for writting.. Exiting\n" and  exit 1);
	open ( ERROR , ">$error_file")|| ( print "\nSEVERE: Can't open error file for writting.. Exiting\n" and  exit 1);

	#======================================================
	# For information purpose
	#======================================================
	print LOG "\nProcessing Input file = $input_file";
	print LOG "\nTemplate file         = $template_file";
	#print LOG "\nFlist file i          = $flist_file";
	print LOG "\nOpcode = $opcode";
	print LOG "\nloglevel = $LOGLEVEL";

	print "\nProcessing Input file = $input_file";
	print "\nTemplate file         = $template_file";
	#print "\nFlist file            = $flist_file";
	print "\nLog file              = $log_file";


	#
	# Get an ebuf for error reporting.
	#
	$ebufp = pcmif::pcm_perl_new_ebuf();
	my $db_no = 1;

	# Do a pcm_connect(), $db_no is a return.
	  
	$pcm_ctxp = pcmif::pcm_perl_connect($db_no, $ebufp);
	  
	  
	# Convert an ebuf to a print LOGable string.
	$ebp1 = pcmif::pcm_perl_ebuf_to_str($ebufp);
	  
	# Check for errors. Always do this.
	if (pcmif::pcm_perl_is_err($ebufp)) 
	{
		pcmif::pcm_perl_print_ebuf($ebufp);
		exit(1);
	} 
	else 
	{
		#print LOG "back from pcm_connect()\n";
		&log($DEBUG,"DEFAULT db:  $db_no ");
	}



	#=====================================================
	# Read the flist file and store in the variable
	#=====================================================
	#while (<FLIST>) {
	#        if ($_  =~ m/^[0-9]/ ) {
	#        $flist .= $_ ;
	#    } 
	#}
	#
	#$orig_flist = $flist ;
	#close(FLIST);

	#======================================================
	# Read the input file template and store it in variable
	#======================================================
	while (<TMPL>) {
		if ($_  !~ m/^[#,\n]/ ) 
		{
			$tmpl .= $_ ;
		} 
	}
	@detail_values_tmpl = split(/\;/,$tmpl);
	close(TMPL);

	#======================================================
	# Read the reason_code and store it in variable
	#======================================================
	my %reason_id_for;
	while (<RES_CODE>) {
			$_  =~ s/\s+/ /g;
			if ($_  =~ m/^-/ )
			{
					my @values = split(/ /,$_);
					$reason_id_for{"$values[1]"} = $values[2];
			}
	}
	close(RES_CODE);


	#======================================================
	# Display for Debug purpose
	#======================================================
	&log ($DEBUG, "\nInput file template is:" . "\n$tmpl" );
	&log ($DEBUG, "\nFlist template is:" . "\n$flist" );


	#=====================================================
	# set to 1 if error happens in detail record
	#=====================================================
	$error_in_record = 0;
	$return_status = 0;

	#=====================================================
	# Variables used to report processing summary
	#=====================================================
	$total_no_of_records = 0;       # total number of detail record
	$parse_error  = 0;              # detail records failed in parsing, record level validation
	$field_validation_error = 0;    # detail records failed in fields level validation
	$opcode_error = 0;              # error in create obj execution
	$success_count = 0;              # Successfully processed record


	#=================================================
	# Order Creation Start
	#=================================================

		my $order_id = "BEC-" . strftime("%Y%m%d%H%M%S",localtime);
		my $os_user = getpwuid( $> );

	my $create_order = << "END"
0 PIN_FLD_POID           POID [0] 0.0.0.1 /account -1 0
0 PIN_FLD_USERID         POID [0] 0.0.0.1 /account -1 0
0 PIN_FLD_PROGRAM_NAME    STR [0] "BULK|$os_user"
0 PIN_FLD_FLAGS           INT [0] 18
0 PIN_FLD_ORDER_OBJ      POID [0] 0.0.0.0 / 0 0
0 PIN_FLD_ORDER_ID        STR [0] "$order_id"
END
;

	#print "\nORDER_INPUT\n$create_order";

		$create_order_flistp = pcmif::pin_perl_str_to_flist($create_order, $db_no, $ebufp);
			
		if (pcmif::pcm_perl_is_err($ebufp))
		{
			&log ($ERROR, "Order Creation Flist conversion failed for input file" );
		}

		$create_order_out_flistp = pcmif::pcm_perl_op($pcm_ctxp, $MSO_OP_INTEG_CREATE_JOB, 0, $create_order_flistp, $ebufp);

		if (pcmif::pcm_perl_is_err($ebufp))
		{
			&log ($ERROR, "Order creation opcode call error" );
			#pcmif::pcm_perl_print_ebuf($ebufp);
		}

		$create_order_out_str = pcmif::pin_perl_flist_to_str($create_order_out_flistp, $ebufp);

		pcmif::pin_flist_destroy($create_order_flistp);
		pcmif::pin_flist_destroy($create_order_out_flistp);

		@create_order_out_arr = split("\n" , $create_order_out_str); 
		foreach my $create_order_out_line( @create_order_out_arr ) 
		{
			if ($create_order_out_line  =~ /PIN_FLD_POID/ )
			{
				$create_order_out_line =~ s/\s+/ /g;
				@create_order_out_field = split(' ' , $create_order_out_line);
				$order_poid = @create_order_out_field[4] . " " .@create_order_out_field[5] . " " .@create_order_out_field[6] . " " . @create_order_out_field[7];
			}
		}
		print ("\n=============================================================");
		print ("\nNote the order id '$order_id' for file '$input_file_name'");
		print ("\n=============================================================");
		
		$orderlist = "\n". "$order_id |". "\t" . "$input_file_name |" ."\t". strftime("%a %b %d %H:%M:%S %Y",localtime);
	
		open ( ORDERLIST, ">>.Order_List")   || ( print "\nSEVERE: Can't open ORDERLIST file for writting.. Exiting\n" and  exit 1);
		print ORDERLIST $orderlist ;
		close (ORDERLIST);
		
		
	#=================================================
	# Order Creation End
	#=================================================



	#======================================================
	# Read the input file line by line
	#======================================================
	$task_name = $order_id . "_task_0";

	my $hotfile_order_details =  << "END"
1     PIN_FLD_POID                   POID [0] 0.0.0.1 /account -1 0
1     PIN_FLD_USERID                 POID [0] 0.0.0.1 /account -1 0
1     MSO_FLD_TASK_NAME               STR [0] "$task_name"
1     PIN_FLD_FLAGS                   INT [0] 18
1     PIN_FLD_ORDER_OBJ              POID [0] $order_poid
1     PIN_FLD_ORDER_ID                STR [0] "$order_id"
1     PIN_FLD_PROGRAM_NAME            STR [0] "BULK|$os_user"
END
;

		$hotfile_order_details =~ s/\n$//;



	my $elem_id = 0;
	$line = <INP>;
	while ($line) 
	{
			#======================================================
			# Process Detail record
			#======================================================
			&log ($DEBUG, "Processing record :" . "\n$line" );


			my @values = split(/,/, $line);

			#======================================================
			# Record level validation
			#======================================================

			$parent_account_no = @values[0];
			$child_account_no  = @values[1];
			$reason_code       = @values[2];

			chomp ($reason_code);

			if ( $parent_account_no eq "New Parrent" &&
			     $child_account_no eq "Child"
    			   )
			{
				goto NEXT_RECORD;
			}

			$total_no_of_records = $total_no_of_records + 1;
			chomp($child_account_no);

			if ( @values != ( @detail_values_tmpl ))
			{
				$parse_error = $parse_error + 1;
				&log ($ERROR, "Incorrect number of field for the record: " . "\n$line" );
				if ( $error_in_record == 0 )
				{
					print ERROR "$header_record";
					$error_in_record = 1;
				}

				$line =~ s/\n//;
				$line = $line . ",Incorrect number of field for the record\n";
				print ERROR "$line";
				&log ($ERROR, "Please correct and process the error file: " . "$error_file" );
				goto NEXT_RECORD;
			}
			my $i ;
			$i = 0;

			#======================================================
			# Field level validation
			#======================================================
			foreach	(@detail_values_tmpl)
			{
				@detail_tmpl_values	= split(/:/,@detail_values_tmpl[$i]);
				if (@values[$i] !~ @detail_tmpl_values[1] )	
				{
					$field_validation_error	= $field_validation_error + 1;
					&log ($ERROR, "Field validation	error for record: " . "\n$line"	);

					if ( $error_in_record == 0 )
					{
						print ERROR	"$header_record";
						$error_in_record = 1;
					}
					
					$field_position	= ++$i;
					$line =~ s/\n//;
					$line =	$line .	", Field " . "$field_position "	. @detail_tmpl_values[0] .  " does not meet field validation requirement\n";
					print ERROR "$line";
					&log ($ERROR, "Please correct and process the error file: " . "$error_file" );
					goto NEXT_RECORD;
				}
			}
			#===================================================
			# Reason Code Start
			#===================================================
			my $reason_id = $reason_id_for{"$reason_code"};
			chomp ($reason_id); 
			#print ("\nreason_code--" . $reason_code . "--\n");
			#print ("\nreason_id--". $reason_id . "--\n");
			if ( $reason_code eq "" ||
				 $reason_id eq "" ) 
			{
					$field_validation_error = $field_validation_error + 1;
					&log ($ERROR, "Invalid reason code:" . "\n$line");

					if ( $error_in_record == 0 )
					{
						print ERROR	"$header_record";
						$error_in_record = 1;
					}
				
					@reason_id_list_arr = keys %reason_id_for;
					$reason_id_list =  join(', ', @reason_id_list_arr);
#					foreach (@reason_id_list_arr) {
#						$reason_id_list =  $reason_id_list . "," . $_ ;
#					}
					$line =~ s/\n//;

					$line = "For record -->" . $line . "<-- Field value '" . $reason_code . "' is invalid.\n   Valid reason IDs are: " . $reason_id_list;
					print ERROR "$line";
					&log ($ERROR, "Please correct and process the error file: " . "$error_file" );
					goto NEXT_RECORD;
			}

	#===================================================
	# Task Creation Start
	#===================================================
	if ($order_poid != "") {

	}

#	my $tmp_hotfile =  << "END"
#0 PIN_FLD_RESULTS                   ARRAY [$elem_id] allocated 20, used 2
#$hotfile_order_details
#1     PIN_FLD_PARENT_FLAGS            INT [0] 1
#1     MSO_FLD_TASK                  ARRAY [0] allocated 20, used 2
#2         PIN_FLD_STATUS             ENUM [0] 0
#2         PIN_FLD_PARENTS           ARRAY [0] allocated 20, used 2
#3             PIN_FLD_ACCOUNT_NO      STR [0] "$parent_account_no"
#3             PIN_FLD_DESCR           STR [0] "$child_account_no"
#END
#;

#=================================================================
       my $hotfile_order_details =  << "END"
0     PIN_FLD_POID                   POID [0] 0.0.0.1 /account -1 0
0     PIN_FLD_USERID                 POID [0] 0.0.0.1 /account -1 0
0     MSO_FLD_TASK_NAME               STR [0] "$task_name"
0     PIN_FLD_FLAGS                   INT [0] 18
0     PIN_FLD_ORDER_OBJ              POID [0] $order_poid
0     PIN_FLD_ORDER_ID                STR [0] "$order_id"
0     PIN_FLD_PROGRAM_NAME            STR [0] "BULK|$os_user"
END
;

        $hotfile_order_details =~ s/\n$//;

	my $tmp_hotfile =  << "END"
r << XX 1
$hotfile_order_details
0     PIN_FLD_PARENT_FLAGS            INT [0] 1
0     MSO_FLD_TASK                  ARRAY [0] allocated 20, used 2
1         PIN_FLD_STATUS             ENUM [0] 0
1         PIN_FLD_PARENTS           ARRAY [0] allocated 20, used 2
2             PIN_FLD_ACCOUNT_NO      STR [0] "$parent_account_no"
2             PIN_FLD_DESCR           STR [0] "$child_account_no"
2             PIN_FLD_REASON_ID       INT [0] $reason_id
XX
xop MSO_OP_INTEG_CREATE_JOB 0 1 
END
;
#=================================================================

		$elem_id = $elem_id +1;

		$hotfile = $hotfile . $tmp_hotfile;


		NEXT_RECORD:
			$flist = $orig_flist;
			$line = <INP>;
	}

	#	$hotfile = $hotfile_order_details  . $ hotfile;

	open ( HOTFILE, ">hotfile")   || ( print "\nSEVERE: Can't open hotfile file for writting.. Exiting\n" and  exit 1);
	print HOTFILE $hotfile ;
	close (HOTFILE);

	#===================================================
	# Task Creation End
	#===================================================

CLEANUP:
#        $success_count = $total_no_of_records - $parse_error - $field_validation_error - $opcode_error;
    if (pcmif::pcm_perl_is_err($ebufp)) 
    {
        print LOG "\n----------------------------------------------------";
        print LOG "\nError: Procees completed with ERROR\n";
        print "\n\t\t\t------------------------------------";
        print "\n\t\t\tError: Procees completed with ERROR";
        print "\n\t\t\tPlease check log file for detail ERROR";
        print "\n\t\t\t------------------------------------";
    }
    else
    {
        print LOG "\n\nProcessing Summary";
        print LOG "\n\t Total number of records = $total_no_of_records";
        print LOG "\n\t Records failed in parsing = $parse_error";
        print LOG "\n\t Records failed in field validation = $field_validation_error";
        print LOG "\n\t Records failed in opcode execution = $opcode_error";
        print LOG "\n\t Successfully processed record count = $success_count \n";

        print "\n\nProcessing Summary";
        print "\n\t Total number of records = $total_no_of_records";
        print "\n\t Records failed in parsing = $parse_error";
        print "\n\t Records failed in field validation = $field_validation_error";
        print "\n\t Records failed in opcode execution = $opcode_error";
        print "\n\t Successfully processed record count = $success_count \n";

    }

    &log ($DEBUG, "Doing cleanup " );
    close(INP) if (<FILE_IS_OPEN>);
    close(LOG) if (<FILE_IS_OPEN>);
    close(ERROR) if (<FILE_IS_OPEN>);
    exit $return_status;
}
elsif ($ARGV[0] eq '-output') 
{

	$app_type = "bulk_entity_change";
	#
	# Get an ebuf for error reporting.
	#
	$ebufp = pcmif::pcm_perl_new_ebuf();
	my $db_no = 1;

	# Do a pcm_connect(), $db_no is a return.
	  
	$pcm_ctxp = pcmif::pcm_perl_connect($db_no, $ebufp);
	  
	  
	# Convert an ebuf to a print LOGable string.
	$ebp1 = pcmif::pcm_perl_ebuf_to_str($ebufp);
	  
	# Check for errors. Always do this.
	if (pcmif::pcm_perl_is_err($ebufp)) 
	{
		pcmif::pcm_perl_print_ebuf($ebufp);
		exit(1);
	} 
	else 
	{
		#print LOG "back from pcm_connect()\n";
		&log($DEBUG,"DEFAULT db:  $db_no ");
	}

	#print "\n -output";
	#=================================================
	# Get Failed Orders Start
	#=================================================

		my $task_order_id = $ARGV[1] . "_task_0";


	my $search_failed_job = << "END"
0 PIN_FLD_POID           POID [0] 0.0.0.1 /account -1 0
0 PIN_FLD_ORDER_ID        STR [0] "$task_order_id"
END
;

	#print "\nORDER_INPUT\n$create_order";


		$search_failed_job_flist = pcmif::pin_perl_str_to_flist($search_failed_job, $db_no, $ebufp);

		if (pcmif::pcm_perl_is_err($ebufp))
		{
			&log ($ERROR, "Search failed jobs Flist conversion failed for input file" );
		}

		$search_failed_job_out_flist = pcmif::pcm_perl_op($pcm_ctxp, $MSO_OP_INTEG_GET_FAILED_ORDER_DETAILS, 0, $search_failed_job_flist, $ebufp);

		if (pcmif::pcm_perl_is_err($ebufp))
		{
			&log ($ERROR, "Search failed jobs opcode call error" );
			#pcmif::pcm_perl_print_ebuf($ebufp);
		}

		$search_failed_job_out_str = pcmif::pin_perl_flist_to_str($search_failed_job_out_flist, $ebufp);

		pcmif::pin_flist_destroy($search_failed_job_flist);
		pcmif::pin_flist_destroy($search_failed_job_out_flist);

		my $header = "PARENT,CHILD,ERROR DESCREPTION\n";
		@search_failed_job_out_arr = split("\n" , $search_failed_job_out_str); 
		foreach my $search_failed_job_out_line( @search_failed_job_out_arr ) 
		{
			if ($search_failed_job_out_line  =~ m/^3.*PIN_FLD_ACCOUNT_NO/ )
			{
				#print "-->$search_failed_job_out_line";
				$search_failed_job_out_line =~ s/\s+/ /g;
				@par_account_no_arr = split('"' , $search_failed_job_out_line );
				@tmp_par_account_no = split('"', @par_account_no_arr[1]);
				$par_account_no =  @tmp_par_account_no[0];
			}
			if ($search_failed_job_out_line  =~ m/^3.*PIN_FLD_DESCR/ )
			{
				$search_failed_job_out_line =~ s/\s+/ /g;
				@child_account_no_arr = split('"' , $search_failed_job_out_line );
				@tmp_child_account_no = split('"', @child_account_no_arr[1]);
				$child_account_no = @tmp_child_account_no[0];
			}
			if ($search_failed_job_out_line  =~ m/^1.*PIN_FLD_ERROR_DESCR/ )
			{
				$search_failed_job_out_line =~ s/\s+/ /g;
				@error_descr_arr = split('"' , $search_failed_job_out_line );
				@tmp_error_descr_arr = split('"', @error_descr_arr[1]);
				$err_descr = @tmp_error_descr_arr[0];
				
				if ($failed_job eq "" ) 
				{
					$failed_job = $par_account_no . "," . $child_account_no ."," . $err_descr ;
				}
				else
				{
					$failed_job = $failed_job ."\n" . $par_account_no . "," . $child_account_no ."," . $err_descr ;
				}
			}
		}
		$failed_job = $header . $failed_job;
		#print "failed_job = \n$failed_job";
		my $failed_job_file_name = "./log/" . $app_type ."/" . $task_order_id . ".csv";
		print "\nfailed_job_file_name=$failed_job_file_name";
		open  (CSVFILE, ">$failed_job_file_name")   || ( print "\nSEVERE: Can't open CSVFILE file for writting.. Exiting\n" and  exit 1);
		print (CSVFILE $failed_job) ;
		close (CSVFILE);

	#=================================================
	# Get Failed Orders End
	#=================================================

}
else
{
        print "\nUsage: mso_bulk_entity_change.pl -object_type <object_name> -input <input file name> ";
        print "\n       Ex: mso_bulk_entity_change.pl -input stb_input_file \n";
        exit 1;
}


sub trim($)
{
   my $string = shift;
   $string =~ s/^\s+//;
   $string =~ s/\s+$//;
   return $string;
}

sub ltrim($)
{
    my $string = shift;
    $string =~ s/^\s+//;
    return $string;
}

sub rtrim($)
{
    my $string = shift;
    $string =~ s/\s+$//;
    return $string;
}

sub log 
{
    if ($LOGLEVEL >= $_[0] ) 
    {
        print LOG "\n$_[1]";
    }
}
