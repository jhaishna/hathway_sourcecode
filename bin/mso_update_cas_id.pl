#!/home/pin/opt/portal/7.5/ThirdParty/perl/5.8.0/bin/perl

use pcmif;
use Sys::Hostname;
use File::Copy;
use File::Basename;
use File::Path;

$SEVERE = 0;
$ERROR = 1;
$INFO = 2;
$DEBUG = 3;

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
        print "\nUsage1: mso_update_cas_id.pl -input <input file name> ";
        print "\n       Ex: mso_update_cas_id.pl -input input_file \n";
        exit 1;
}

if ( $ARGV[0] ne '-input' )
{
#        print "Input parameter validation error\n";
        print "\nUsage2: mso_update_cas_id.pl -input <input file name> ";
        print "\n       Ex: mso_update_cas_id.pl -input input_file \n";
	exit 1;
}


@object_values = split(/\//,$ARGV[1]);


#======================================================
# Read the configuration file and 
# get the loglevel and  opcode number 
#======================================================
$conf_file = "./pin.conf";
open (CONF, $conf_file) || ( print "\nSEVERE: Configuration file does not exist.. Exiting\n" and  exit 1 );
while (<CONF>) {
    $conf = $_ ;
        if ($conf  =~ /loglevel/ ) {
            @conf_values = split(/ /,$conf);
            $LOGLEVEL =  @conf_values[3];
        }
}
#
#if (length $opcode <= 0) {
#    print "\nSEVERE: Opcode number not configured for $ARGV[1] in pin.conf file.. Exiting\n";
#    exit 1;
#}
#
if (length $LOGLEVEL <= 0) {
    print "\nSEVERE: loglevel not configured for mso_parse_create in pin.conf file.. Exiting\n";
    exit 1;
}


#======================================================
# Set path for template file, flist file, input file
# log file, reject file and error file 
#======================================================
$input_file_name = $ARGV[1];
#$template_file = "./template/" . $app_type . "/" . $app_type . ".TMPL";
#$flist_file    = "./template/" . $action .  ".IFL";
$input_file    = "./data/" . $app_type . "input/" . $input_file_name;
$log_file      = "./log/" . $app_type . $input_file_name . ".log";
$reject_file   = "./data/" . $app_type . "reject/" . $input_file_name . ".reject"; #complete batch rejection
$error_file    = "./data/" . $app_type . "error/" . $input_file_name . ".error";   # specific record which failed during processing

open( INP, $input_file)     || ( print "\nSEVERE: Input file does not exist.. Exiting\n" and  exit 1 );
#open (TMPL, $template_file) || ( print "\nSEVERE: Template file does not exist.. Exiting\n" and  exit 1) ;
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
open ( REJECT , ">$error_file")|| ( print "\nSEVERE: Can't open reject file for writting.. Exiting\n" and  exit 1);

#======================================================
# For information purpose
#======================================================
#print LOG "\nProcessing Input file = $input_file";
#print LOG "\nTemplate file         = $template_file";
#print LOG "\nFlist file i          = $flist_file";
#print LOG "\nOpcode = $opcode";
#print LOG "\nloglevel = $LOGLEVEL";

&log($DEBUG, "Processing Input file = $input_file");
&log($DEBUG, "loglevel              = $LOGLEVEL");

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
    #&log($DEBUG,"DEFAULT db:  $db_no ");
}




#=====================================================
# Read the flist file and store in the variable
#=====================================================
while (<FLIST>) {
        if ($_  =~ m/^[0-9]/ ) {
        $flist .= $_ ;
    } 
}

$orig_flist = $flist ;
close(FLIST);



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
$success_count = 0;             # Successfully processed record




#======================================================
# Read the input file line by line
#======================================================
	$line = <INP>;
	while ($line) 
	{
	#    if  ($line =~ m/^HEADER/) 
	#    {
			$total_no_of_records = $total_no_of_records + 1;
			#======================================================
			# Header Found: Create an order object
			# Order object is to be created for only devices
			#======================================================
			#&log ($DEBUG, "\nProcessing Header:" . "\n$line" );

			#$header_record = $line;
			$poid_type = "0.0.0.1 /mso_prov_order" . " -1";
			#($header_sym,$sp_code,$supp_code,$ref_inv_no,$po_no,$ref_inv_date) = split(/,/, $line);
			my @values = split(/,/, $line);
			$vc_id = @values[0];
			$vc_id = trim($vc_id);
			if (  !(length $vc_id > 0 ) ) 
			{
				$field_validation_error = $field_validation_error + 1;
	                    	if ( $error_in_record == 0 )
	                    	{
                        		print ERROR "$header_record";
                        		$error_in_record = 1;
                    		}
				$line =~ s/\n//;
				#print (REJECT "$line");
				$line = $line . "| Invalid agreement_no.\n";
				print (ERROR "$line");
				&log ($SEVERE, $line . "agreement_no invalid. Please correct and reprocess the error file:" . "$error_file" );
				#print "Severe: Header validation failed. agreement_no invalid. Exiting";

				goto NEXT_RECORD;
			}

			$inp_cas_subscriber_id = @values[1];
			$inp_cas_subscriber_id = trim($inp_cas_subscriber_id);
	                
			if (  length $inp_cas_subscriber_id != 8  )
                        {
				$field_validation_error = $field_validation_error + 1;
	                    	if ( $error_in_record == 0 )
	                    	{
                        		print ERROR "$header_record";
                        		$error_in_record = 1;
                    		}
				$line =~ s/\n//;
				#print (REJECT "$line");
				$line = $line . "| Invalid CAS_SUBSCRIBER_ID.\n";
				print (ERROR "$line");

                                &log ($SEVERE, "CAS_SUBSCRIBER_ID invalid. Please correct and reprocess the error file:" . "$error_file" );
                                #print "Severe: Header validation failed. CAS_SUBSCRIBER_ID invalid. Exiting";
                                # exit if error in header record
                                goto NEXT_RECORD;
                        }
	

	        my $srch_flist = << "END"
0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
0 PIN_FLD_FLAGS           INT [0] 768
0 PIN_FLD_TEMPLATE        STR [0] "select X from /service  where F1 = V1 "
0 PIN_FLD_ARGS          ARRAY [1] allocated 20, used 1
1   PIN_FLD_ALIAS_LIST    ARRAY [*] allocated 20, used 1
2        PIN_FLD_NAME            STR [0] "$vc_id"
0 PIN_FLD_RESULTS       ARRAY [*] allocated 20, used 1
1     MSO_FLD_CATV_INFO   SUBSTRUCT [0] NULL
END
;

			#&log ($DEBUG, "flist for servc search:" . "\n$srch_flist" );
			# Convert the flist string into an actual FList
			$srch_flistp = pcmif::pin_perl_str_to_flist($srch_flist, $db_no, $ebufp);
			if (pcmif::pcm_perl_is_err($ebufp))
			{
					$opcode_error = $opcode_error + 1;
					#pcmif::pcm_perl_print_ebuf($ebufp);
					move ($input_file, $reject_file) or die "Copy failed: $!";
					&log ($SEVERE, "Flist conversion failed for Header record. Please correct and reprocess the reject file:" . "$reject_file" );
					print "\nSevere: Flist conversion failed for Header record.. Exiting";
					# exit if error in header record
					$return_status = 1;
                			pcmif::pcm_perl_destroy_ebuf($ebufp);
                			$ebufp = pcmif::pcm_perl_new_ebuf();
					goto NEXT_RECORD;
			}
			$srch_out_flistp = pcmif::pcm_perl_op($pcm_ctxp, 7, 0, $srch_flistp, $ebufp);
			pcmif::pin_flist_destroy($srch_flistp);

			if (pcmif::pcm_perl_is_err($ebufp))
			{
					$opcode_error = $opcode_error + 1;
					#pcmif::pcm_perl_print_ebuf($ebufp);
					move ($input_file, $reject_file) or die "Copy failed: $!";
					&log ($SEVERE, "Search Failed:" . "$reject_file" );
					print "Severe: OSearch Failed.. Exiting \n";
					pcmif::pin_flist_destroy($order_flistp);
					# exit if error in header record
					$return_status = 1;
                			pcmif::pcm_perl_destroy_ebuf($ebufp);
                			$ebufp = pcmif::pcm_perl_new_ebuf();
					goto CLEANUP;
			}
			$srch_out_flist = pcmif::pin_perl_flist_to_str($srch_out_flistp, $ebufp);
			#&log ($DEBUG, "Preactivated Srvc Search Output is:" . "\n$srch_out_flist" );
			#======================================================
			#return if no record found
			#======================================================
			@flist_lines = split('\n', "$srch_out_flist");
			if ($#flist_lines < 3) {
				#&log ($DEBUG, "Search Output is:" . "\n$srch_out_flist" );
				goto NEXT_RECORD;
			}
			$srch_out_flist = pcmif::pin_perl_flist_to_str($srch_out_flistp, $ebufp);
			#&log ($DEBUG, "Search Output is:" . "\n$srch_out_flist" );

		   	@flist_lines={};
		   	@flist_lines = split('\n', "$srch_out_flist");
			foreach my $flist_line (@flist_lines) 
			{
				if ($flist_line =~ /^1\s+PIN_FLD_POID/)
				{
					 $line_poid = $flist_line;
					 $line_poid =~ s/^1/0/g;
				}
				if ($flist_line =~ /MSO_FLD_CAS_SUBSCRIBER_ID/)
				{
					 $cas_subscriber_id = $flist_line;
					 $cas_subscriber_id =~ s/^\s+/ /g;
					 @tmp_arr = split(' ', "$cas_subscriber_id");
					 $cas_subscriber_id =@tmp_arr[4];
				}
			}
			pcmif::pin_flist_destroy($srch_out_flistp);

			#======================================================
			# WRITE fields
			#======================================================
			&log ($DEBUG, "Updating MSO_FLD_CAS_SUBSCRIBER_ID from $cas_subscriber_id to $inp_cas_subscriber_id" );
              my $write_flds_iflist = << "END"
0 MSO_FLD_CATV_INFO          SUBSTRUCT [0] allocated 20, used 9
1     MSO_FLD_CAS_SUBSCRIBER_ID    STR [0] "$inp_cas_subscriber_id"
END
;

            $write_flds_iflist = $write_flds_iflist . $line_poid;
			&log ($ERROR, "Flist write_flds: " . "\n$write_flds_iflist" );
			# Convert the flist string into an actual FList
			$write_flds_iflistp = pcmif::pin_perl_str_to_flist($write_flds_iflist, $db_no, $ebufp);
			if (pcmif::pcm_perl_is_err($ebufp)) 
			{
				&log ($ERROR, "Flist 1 conversion failed for Detail record: " . "\n$line" );
				#pcmif::pcm_perl_print_ebuf($ebufp);

				$parse_error = $parse_error + 1;

				if ( $error_in_record == 0 )
				{
					print ERROR "$header_record";
					$error_in_record = 1;
				}

				$line =~ s/\n//;
				$line = $line . ",Flist 1 conversion failed for Detail record\n";
				print ERROR "$line";
				&log ($ERROR, "Please correct and process the error file: " . "$error_file" );
				#==============================================
				# Destroy and create a new error buffer
				# Break and process next line
				#==============================================
				pcmif::pcm_perl_destroy_ebuf($ebufp);
				$ebufp = pcmif::pcm_perl_new_ebuf();
				goto NEXT_RECORD;
			}

			$write_flds_oflistp = pcmif::pcm_perl_op($pcm_ctxp, 5, 0, $write_flds_iflistp, $ebufp);

			if (pcmif::pcm_perl_is_err($ebufp)) 
			{
				&log ($ERROR, "Object creation failed for Detail record: " . "\n$line" );
				#pcmif::pcm_perl_print_ebuf($ebufp);

				$opcode_error = $opcode_error + 1;

				if ( $error_in_record == 0 )
				{
					print ERROR "$header_record";
					$error_in_record = 1;
				}

				$line =~ s/\n//;
				$line = $line . ",opcode execution failed, Please check cm.pinlog for detail\n";
				print ERROR "$line";
				&log ($ERROR, "Please correct and process the error file: " . "$error_file" );
				#==============================================
				# Destroy and create a new error buffer
				# Break and process next line
				#==============================================
				pcmif::pin_flist_destroy($write_flds_iflistp);
				pcmif::pin_flist_destroy($write_flds_oflistp);
				pcmif::pcm_perl_destroy_ebuf($ebufp);
				$ebufp = pcmif::pcm_perl_new_ebuf();
				goto NEXT_RECORD;
			}
			pcmif::pin_flist_destroy($write_flds_iflistp);
			pcmif::pin_flist_destroy($write_flds_oflistp);


		NEXT_RECORD:
			$flist = $orig_flist;
			$line = <INP>;
	}


CLEANUP:
        $success_count = $total_no_of_records - $parse_error - $field_validation_error - $opcode_error;
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
