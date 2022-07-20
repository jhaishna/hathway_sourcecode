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

#======================================================
# Input parameter validation
#======================================================
if($#ARGV != 1)
{
        print "\nInvalid input parameter";
        print "\nUsage: mso_catv_preactivation_LCO_updater.pl -input <input file name> ";
        print "\n       Ex: mso_catv_preactivation_LCO_updater.pl -input stb_input_file \n";
        exit 1;
}

#if ( $ARGV[0] ne '-device_type' or $ARGV[2] ne '-input' )
#{
#        print "Input parameter validation error\n";
#        print "\nUsage: mso_catv_preactivation_LCO_updater.pl -device_type <device_type> -input <input file name> ";
#        print "\n       Ex: mso_catv_preactivation_LCO_updater.pl -device_type /device/stb -input stb_input_file \n";
#        exit 1;
#}

#@object_values = split(/\//,$ARGV[1]);
print " input $ARGV[0]";

if  ( $ARGV[0] ne '-input' )
{
        print "Input parameter value validation error\n";
        print "\nUsage: mso_catv_preactivation_LCO_updater.pl -input <input file name> ";
        print "\n       Ex: mso_catv_preactivation_LCO_updater.pl -input stb_input_file \n";
        exit 1;
}
#else
#{
#     $app_type  = @object_values[2];
#}
$app_type = 'catv_preactivation_LCO_change';
#======================================================
# Read the configuration file and 
# get the loglevel and  opcode number 
#======================================================
$conf_file = "./pin.conf";
open (CONF, $conf_file) || ( print "\nSEVERE: Configuration file does not exist.. Exiting\n" and  exit 1 );
while (<CONF>) {
    $conf = $_ ;
    if ($conf  =~ /mso_catv_preactivation_LCO_updater/ ) {
        if ($conf  =~ /loglevel/ ) {
            @conf_values = split(/ /,$conf);
            $LOGLEVEL =  @conf_values[3];
        }
    }
}

if (length $LOGLEVEL <= 0) {
    print "\nSEVERE: loglevel not configured for mso_catv_preactivation_LCO_updater in pin.conf file.. Exiting\n";
    exit 1;
}


#======================================================
# Set path for template file, flist file, input file
# log file, reject file and error file 
#======================================================
#$input_file_name = $ARGV[3];
#$template_file = "./" . $app_type . "/" . $app_type . ".TMPL";
#$input_file    = "./" . $app_type . "/input/" . $input_file_name;
#$log_file      = "./" . $app_type . "/log/" . $input_file_name . ".log";
#$reject_file   = "./" . $app_type . "/reject/" . $input_file_name . ".reject"; #complete batch rejection
#$error_file    = "./" . $app_type . "/error/" . $input_file_name . ".error";   # specific record which failed during processing

$input_file_name = $ARGV[1];
#$template_file = "./template/" .catv_preactivation_LCO_change. "/" .catv_preactivation_LCO_change. ".TMPL";
$template_file = "./template/" . $app_type. "/" . $app_type . ".TMPL";
#$input_file    = "./data/" . catv_preactivation_LCO_change .  "/input/" . $input_file_name;
$input_file    = "./data/" . $app_type .  "/input/" . $input_file_name;
$log_file      = "./log/" . $app_type . "/" . $input_file_name . ".log";
$reject_file   = "./data/" . $app_type . "/reject/" . $input_file_name . ".reject"; #complete batch rejection
$error_file    = "./data/" . $app_type . "/error/" . $input_file_name . ".error";   # specific record which failed during processing

print "path $template_file";
open( INP, $input_file)     || ( print "\nSEVERE: Input file does not exist.. Exiting\n" and  exit 1 );
open (TMPL, $template_file) || ( print "\nSEVERE: Template file does not exist.. Exiting\n" and  exit 1) ;


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
print LOG "\nOpcode = $opcode";
print LOG "\nloglevel = $LOGLEVEL";

print "\nProcessing Input file = $input_file";
print "\nTemplate file         = $template_file";


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



#======================================================
# Read the input file template and store it in variable
#======================================================
while (<TMPL>) {
    if ($_  !~ m/^[#,\n]/ ) {
        $tmpl .= $_ ;
    } 
}

@detail_values_tmpl = split(/;/,$tmpl);
close(TMPL);

#======================================================
# Display for Debug purpose
#======================================================
&log ($DEBUG, "\nInput file template is:" . "\n$tmpl" );

#=====================================================
# set to 1 if error happens in detail record
#=====================================================
$return_status = 0;

#=====================================================
# Variables used to report processing summary
#=====================================================
$total_no_of_records = 0;       # total number of detail record
$parse_error  = 0;              # detail records failed in parsing, record level validation
$field_validation_error = 0;    # detail records failed in fields level validation
$opcode_error = 0;              # error in create obj execution
$success_count = 0;              # Successfully processed record


#======================================================
# Read the input file line by line
#======================================================
$line = <INP>;
while ($line) 
{
    if ( $line =~ m/^Header/ )
    {
        #======================================================
        # Ignore the header
        #         DO NOTHING      
        #======================================================
        
    }
    elsif (  $line =~ m/^[#,\n]/ )
    {
        #======================================================
        # Ignore Empty line and lines starting with hash (#)
        #         DO NOTHING
        #======================================================
    }
    else 
    {
        #======================================================
        # Process Detail record
        #======================================================
        &log ($DEBUG, "Processing record :" . "\n$line" );

        $total_no_of_records = $total_no_of_records + 1;

        my @values = split(/,/, $line);

        #======================================================
        # Record level validation
        #======================================================
        if ( @values != ( @detail_values_tmpl ))
        {
            $parse_error = $parse_error + 1;
            &log ($ERROR, "Incorrect number of field for the record: " . "\n$line" );

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
        foreach (@detail_values_tmpl)
        {
            @detail_tmpl_values = split(/:/,@detail_values_tmpl[$i]);
            if (@values[$i] !~ @detail_tmpl_values[1] ) 
            {
                $field_validation_error = $field_validation_error + 1;
                &log ($ERROR, "Field validation error for record: " . "\n$line" );
                
                $field_position = ++$i;
                $line =~ s/\n//;
                $line = $line . ", Field " . "$field_position " . @detail_tmpl_values[0] .  "  does not meet field validation requirement\n";
                print ERROR "$line";
                &log ($ERROR, "Please correct and process the error file: " . "$error_file" );
                goto NEXT_RECORD;
            }
        }


        #======================================================
        # LCO Search starts here
        #======================================================
        $account_no  = @values[0];
		$new_account_no = @values[3];
		$new_account_no =~ s/\n//;

        my $search_flist = << "END"
0 PIN_FLD_POID              POID [0]  0.0.0.1 /search -1  
0 PIN_FLD_FLAGS     INT [0] 512 
0 PIN_FLD_TEMPLATE        STR [0] "select X from /account where F1 = V1 " 
0 PIN_FLD_ARGS        ARRAY [1] 
1   PIN_FLD_ACCOUNT_NO  STR [0] "$account_no"
0 PIN_FLD_RESULTS       ARRAY [0]
1  PIN_FLD_POID        POID [0] NULL
END
;
        my $new_search_flist = << "XXX"
0 PIN_FLD_POID              POID [0]  0.0.0.1 /search -1  
0 PIN_FLD_FLAGS     INT [0] 512 
0 PIN_FLD_TEMPLATE        STR [0] "select X from /account where F1 = V1 " 
0 PIN_FLD_ARGS        ARRAY [1] 
1   PIN_FLD_ACCOUNT_NO  STR [0] "$new_account_no"
0 PIN_FLD_RESULTS       ARRAY [0]
1  PIN_FLD_POID        POID [0] NULL
XXX
;

        &log ($DEBUG, "flist for new LCO search :" . "\n$new_search_flist" );

        # Convert the flist string into an actual FList
        $new_search_flistp = pcmif::pin_perl_str_to_flist($new_search_flist, $db_no, $ebufp);
		
		 if (pcmif::pcm_perl_is_err($ebufp))
        {
                &log ($ERROR, "Search Flist conversion failed for Detail record: " . "\n$line" );
                #pcmif::pcm_perl_print_ebuf($ebufp);

                $parse_error = $parse_error + 1;

                $line =~ s/\n//;
                $line = $line . ",Search LCO failed for Detail record\n";
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
		
		$new_search_out_flistp = pcmif::pcm_perl_op($pcm_ctxp, 7,0, $new_search_flistp, $ebufp);
		
		 if (pcmif::pcm_perl_is_err($ebufp))
        {
            &log ($ERROR, "LCO search failed for Detail record: " . "\n$line" );
            #pcmif::pcm_perl_print_ebuf($ebufp);

            $opcode_error = $opcode_error + 1;

            $line =~ s/\n//;
            $line = $line . ",opcode execution failed, Please check cm.pinlog for detail\n";
            print ERROR "$line";
            &log ($ERROR, "Please correct and process the error file: " . "$error_file" );
            #==============================================
            # Destroy and create a new error buffer
            # Break and process next line
            #==============================================
            pcmif::pin_flist_destroy($new_search_flistp);
            pcmif::pcm_perl_destroy_ebuf($ebufp);
            $ebufp = pcmif::pcm_perl_new_ebuf();
            goto NEXT_RECORD;
        }

         $new_out_flist = pcmif::pin_perl_flist_to_str($new_search_out_flistp, $ebufp);

        &log ($DEBUG, "LCO search output" . "\n$new_out_flist" );
		
		
		 #======================================================
        # Get the LCO account poid 
        #======================================================
        @new_LCO_out = split(/\[0\]/, $new_out_flist);

        #======================================================
        # Validate LCO account poid
        #======================================================
        if (length @new_LCO_out[3] <= 0 ) 
        {
                $field_validation_error = $field_validation_error + 1;
                &log ($ERROR, "Invaid LCO account no for record: " . "\n$line" );

                $field_position = ++$i;
                $line =~ s/\n//;
                $line = $line . ", Invaid LCO account no\n";
                print ERROR "$line";
                &log ($ERROR, "Please correct and process the error file: " . "$error_file" );
                goto NEXT_RECORD;
        }
		
		@new_LCO_out[3] =~ s/\n//;
        &log ($DEBUG, "LCO obj is :" . "\n@LCO_out[3]" );
        pcmif::pin_flist_destroy($new_search_flistp);
        pcmif::pin_flist_destroy($new_search_out_flistp);
		
		
        &log ($DEBUG, "flist for LCO search :" . "\n$search_flist" );

        # Convert the flist string into an actual FList
        $search_flistp = pcmif::pin_perl_str_to_flist($search_flist, $db_no, $ebufp);
        if (pcmif::pcm_perl_is_err($ebufp))
        {
                &log ($ERROR, "Search Flist conversion failed for Detail record: " . "\n$line" );
                #pcmif::pcm_perl_print_ebuf($ebufp);

                $parse_error = $parse_error + 1;

                $line =~ s/\n//;
                $line = $line . ",Search LCO failed for Detail record\n";
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

        $search_out_flistp = pcmif::pcm_perl_op($pcm_ctxp, 7,
                                                     0, $search_flistp, $ebufp);

        if (pcmif::pcm_perl_is_err($ebufp))
        {
            &log ($ERROR, "LCO search failed for Detail record: " . "\n$line" );
            #pcmif::pcm_perl_print_ebuf($ebufp);

            $opcode_error = $opcode_error + 1;

            $line =~ s/\n//;
            $line = $line . ",opcode execution failed, Please check cm.pinlog for detail\n";
            print ERROR "$line";
            &log ($ERROR, "Please correct and process the error file: " . "$error_file" );
            #==============================================
            # Destroy and create a new error buffer
            # Break and process next line
            #==============================================
            pcmif::pin_flist_destroy($search_flistp);
            pcmif::pcm_perl_destroy_ebuf($ebufp);
            $ebufp = pcmif::pcm_perl_new_ebuf();
            goto NEXT_RECORD;
        }

        $out_flist = pcmif::pin_perl_flist_to_str($search_out_flistp, $ebufp);

        &log ($DEBUG, "LCO search output" . "\n$out_flist" );

        #======================================================
        # Get the LCO account poid 
        #======================================================
        @LCO_out = split(/\[0\]/, $out_flist);

        #======================================================
        # Validate LCO account poid
        #======================================================
        if (length @LCO_out[3] <= 0 ) 
        {
                $field_validation_error = $field_validation_error + 1;
                &log ($ERROR, "Invaid LCO account no for record: " . "\n$line" );

                $field_position = ++$i;
                $line =~ s/\n//;
                $line = $line . ", Invaid LCO account no\n";
                print ERROR "$line";
                &log ($ERROR, "Please correct and process the error file: " . "$error_file" );
                goto NEXT_RECORD;
        }

        @LCO_out[3] =~ s/\n//;
        &log ($DEBUG, "LCO obj is :" . "\n@LCO_out[3]" );
        pcmif::pin_flist_destroy($search_flistp);
        pcmif::pin_flist_destroy($search_out_flistp);
        # LCO search complete


        #======================================================
        # Device Search starts here
        #======================================================
        @values[2] =~ s/\n$//;
		@values[1] =~ s/\n$//;
        $device_id  = @values[2];
        $device_vc  = @values[1];
		
 		  my $device_stb_search_flist = << "YYY"
0 PIN_FLD_POID              POID [0]  0.0.0.1 /search -1  
0 PIN_FLD_FLAGS     INT [0] 512 
0 PIN_FLD_TEMPLATE        STR [0] "select X from /device where F1 = V1 and F2 = V2 " 
0 PIN_FLD_ARGS        ARRAY [1] 
1   PIN_FLD_DEVICE_ID  STR [0] "$device_id"
0 PIN_FLD_ARGS        ARRAY [2] 
1   PIN_FLD_POID  POID [0] 0.0.0.1 /device/stb -1
0 PIN_FLD_RESULTS       ARRAY [0]
1  PIN_FLD_POID        POID [0] NULL
YYY
;
		&log ($DEBUG, "flist for Device search :" . "\n$device_stb_search_flist" );

			# Convert the flist string into an actual FList
			$device_stb_search_flistp = pcmif::pin_perl_str_to_flist($device_stb_search_flist, $db_no, $ebufp);
			if (pcmif::pcm_perl_is_err($ebufp))
			{
					&log ($ERROR, "Search Flist conversion failed for Detail record: " . "\n$line" );
					#pcmif::pcm_perl_print_ebuf($ebufp);

					$parse_error = $parse_error + 1;

					$line =~ s/\n//;
					$line = $line . ",Search Device id failed for Detail record\n";
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
			
			$device_stb_search_out_flistp = pcmif::pcm_perl_op($pcm_ctxp, 7,
														 0, $device_stb_search_flistp, $ebufp);

			if (pcmif::pcm_perl_is_err($ebufp))
			{
				&log ($ERROR, "Device search failed for Detail record: " . "\n$line" );
				#pcmif::pcm_perl_print_ebuf($ebufp);

				$opcode_error = $opcode_error + 1;

				$line =~ s/\n//;
				$line = $line . ",opcode execution failed, Please check cm.pinlog for detail\n";
				print ERROR "$line";
				&log ($ERROR, "Please correct and process the error file: " . "$error_file" );
				#==============================================
				# Destroy and create a new error buffer
				# Break and process next line
				#==============================================
				pcmif::pin_flist_destroy($device_stb_search_flistp);
				pcmif::pcm_perl_destroy_ebuf($ebufp);
				$ebufp = pcmif::pcm_perl_new_ebuf();
				goto NEXT_RECORD;
			}

			
			$stb_out_flist = pcmif::pin_perl_flist_to_str($device_stb_search_out_flistp, $ebufp);

			&log ($DEBUG, "Device search output" . "\n$stb_out_flist" );

			#======================================================
			# Get the Device poid
			#======================================================
			@stb_Device_out = split(/\[0\]/, $stb_out_flist);

			#======================================================
			# Validate DEVICE poid
			#======================================================
			if (length @stb_Device_out[3] <= 0 ) 
			{
					$field_validation_error = $field_validation_error + 1;
					&log ($ERROR, "Invaid Device id for record: " . "\n$line" );

					$field_position = ++$i;
					$line =~ s/\n//;
					$line = $line . ", Invaid Device id \n";
					print ERROR "$line";
					&log ($ERROR, "Please correct and process the error file: " . "$error_file" );
					goto NEXT_RECORD;
			}

			&log ($DEBUG, "Device poid is:" . "\n@stb_Device_out[3]" );
			pcmif::pin_flist_destroy($device_stb_search_flistp);
			pcmif::pin_flist_destroy($device_stb_search_out_flistp);
			# Device search complete
			
		
		
		
		if (($device_vc eq "" ))
		{
		   print "No vc search requied";
		}
        else
		{
        my $device_search_flist = << "END"
0 PIN_FLD_POID              POID [0]  0.0.0.1 /search -1  
0 PIN_FLD_FLAGS     INT [0] 512 
0 PIN_FLD_TEMPLATE        STR [0] "select X from /device where F1 = V1 and F2 = V2 " 
0 PIN_FLD_ARGS        ARRAY [1] 
1   PIN_FLD_DEVICE_ID  STR [0] "$device_vc"
0 PIN_FLD_ARGS        ARRAY [2] 
1   PIN_FLD_POID  POID [0] 0.0.0.1 /device/vc -1
0 PIN_FLD_RESULTS       ARRAY [0]
1  PIN_FLD_POID        POID [0] NULL
END
;

			&log ($DEBUG, "flist for Device search :" . "\n$device_search_flist" );

			# Convert the flist string into an actual FList
			$device_search_flistp = pcmif::pin_perl_str_to_flist($device_search_flist, $db_no, $ebufp);
			if (pcmif::pcm_perl_is_err($ebufp))
			{
					&log ($ERROR, "Search Flist conversion failed for Detail record: " . "\n$line" );
					#pcmif::pcm_perl_print_ebuf($ebufp);

					$parse_error = $parse_error + 1;

					$line =~ s/\n//;
					$line = $line . ",Search Device id failed for Detail record\n";
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

			$device_search_out_flistp = pcmif::pcm_perl_op($pcm_ctxp, 7,
														 0, $device_search_flistp, $ebufp);

			if (pcmif::pcm_perl_is_err($ebufp))
			{
				&log ($ERROR, "Device search failed for Detail record: " . "\n$line" );
				#pcmif::pcm_perl_print_ebuf($ebufp);

				$opcode_error = $opcode_error + 1;

				$line =~ s/\n//;
				$line = $line . ",opcode execution failed, Please check cm.pinlog for detail\n";
				print ERROR "$line";
				&log ($ERROR, "Please correct and process the error file: " . "$error_file" );
				#==============================================
				# Destroy and create a new error buffer
				# Break and process next line
				#==============================================
				pcmif::pin_flist_destroy($device_search_flistp);
				pcmif::pcm_perl_destroy_ebuf($ebufp);
				$ebufp = pcmif::pcm_perl_new_ebuf();
				goto NEXT_RECORD;
			}

			$out_flist = pcmif::pin_perl_flist_to_str($device_search_out_flistp, $ebufp);

			&log ($DEBUG, "Device search output" . "\n$out_flist" );

			#======================================================
			# Get the Device poid
			#======================================================
			@Device_out = split(/\[0\]/, $out_flist);

			#======================================================
			# Validate DEVICE poid
			#======================================================
			if (length @Device_out[3] <= 0 ) 
			{
					$field_validation_error = $field_validation_error + 1;
					&log ($ERROR, "Invaid Device id for record: " . "\n$line" );

					$field_position = ++$i;
					$line =~ s/\n//;
					$line = $line . ", Invaid Device id \n";
					print ERROR "$line";
					&log ($ERROR, "Please correct and process the error file: " . "$error_file" );
					goto NEXT_RECORD;
			}

			&log ($DEBUG, "Device poid is:" . "\n@Device_out[3]" );
			pcmif::pin_flist_destroy($device_search_flistp);
			pcmif::pin_flist_destroy($device_search_out_flistp);
			# Device search complete
			
        }
		
		
		#===================================================
		# Service Search input flist
		#===================================================
	        @stb_Device_out[3] =~ s/\n$//; 	
		my $preac_ser_search_flist = << "XXX"
0 PIN_FLD_POID              POID [0]  0.0.0.1 /search -1  
0 PIN_FLD_FLAGS     INT [0] 256 
0 PIN_FLD_TEMPLATE        STR [0] "select X from /mso_catv_preactivated_svc where F1 = V1 and F2 = V2 " 
0 PIN_FLD_ARGS        ARRAY [1] 
1 MSO_FLD_STB_OBJ     POID [0] @stb_Device_out[3]
0 PIN_FLD_ARGS        ARRAY [2] 
1 PIN_FLD_ACCOUNT_OBJ    POID [0] @LCO_out[3]
0 PIN_FLD_RESULTS       ARRAY [0]
1 PIN_FLD_POID        POID [0] NULL
XXX
;
		&log ($DEBUG, "flist for Device search :" . "\n$preac_ser_search_flist" );

			# Convert the flist string into an actual FList
			$preac_ser_search_flistp = pcmif::pin_perl_str_to_flist($preac_ser_search_flist, $db_no, $ebufp);
			if (pcmif::pcm_perl_is_err($ebufp))
			{
					&log ($ERROR, "Search Flist conversion failed for Detail record: " . "\n$line" );
					#pcmif::pcm_perl_print_ebuf($ebufp);

					$parse_error = $parse_error + 1;

					$line =~ s/\n//;
					$line = $line . ",Search for pre_ac_service failed for Detail record\n";
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
		
		    $preac_ser_search_out_flistp = pcmif::pcm_perl_op($pcm_ctxp, 7,
														 0, $preac_ser_search_flistp, $ebufp);

			if (pcmif::pcm_perl_is_err($ebufp))
			{
				&log ($ERROR, "Preactivated service object search failed for Detail record: " . "\n$line" );
				#pcmif::pcm_perl_print_ebuf($ebufp);

				$opcode_error = $opcode_error + 1;

				$line =~ s/\n//;
				$line = $line . ",opcode execution failed, Please check cm.pinlog for detail\n";
				print ERROR "$line";
				&log ($ERROR, "Please correct and process the error file: " . "$error_file" );
				#==============================================
				# Destroy and create a new error buffer
				# Break and process next line
				#==============================================
				pcmif::pin_flist_destroy($preac_ser_search_flistp);
				pcmif::pcm_perl_destroy_ebuf($ebufp);
				$ebufp = pcmif::pcm_perl_new_ebuf();
				goto NEXT_RECORD;
			}
		
		    $preact_out_flist = pcmif::pin_perl_flist_to_str($preac_ser_search_out_flistp, $ebufp);

			&log ($DEBUG, "Preact service search output" . "\n$preact_out_flist" );

			#======================================================
			# Get the Device poid
			#======================================================
			@preac_Device_out = split(/\[0\]/, $preact_out_flist);

			#======================================================
			# Validate DEVICE poid
			#======================================================
			if (length @preac_Device_out[3] <= 0 ) 
			{
					$field_validation_error = $field_validation_error + 1;
					&log ($ERROR, "Invaid Preactivated service for record: " . "\n$line" );

					$field_position = ++$i;
					$line =~ s/\n//;
					$line = $line . ", Invaid Preactivated service id \n";
					print ERROR "$line";
					&log ($ERROR, "Please correct and process the error file: " . "$error_file" );
					goto NEXT_RECORD;
			}

		&log ($DEBUG, "Device poid is:" . "\n@preac_Device_out[3]" );
		pcmif::pin_flist_destroy($preac_ser_search_flistp);
		pcmif::pin_flist_destroy($preac_ser_search_out_flistp);
		# Preactivated Service search complete
		
	    #===================================================
        # update device location 
        #===================================================
        @preac_Device_out[3] =~ s/\n$//;
        @new_LCO_out[3] =~ s/\n$//;
		

        my $lco_update_flist = << "END"
0 PIN_FLD_POID            POID [0] @preac_Device_out[3]
0 PIN_FLD_ACCOUNT_OBJ     POID [0] @new_LCO_out[3] 
END
;
        # Convert the flist string into an actual FList
        $lco_update_flistp = pcmif::pin_perl_str_to_flist($lco_update_flist, $db_no, $ebufp);
        if (pcmif::pcm_perl_is_err($ebufp)) 
        {
            &log ($ERROR, "Flist conversion failed for Detail record: " . "\n$line" );
            #pcmif::pcm_perl_print_ebuf($ebufp);

            $parse_error = $parse_error + 1;

            $line =~ s/\n//;
            $line = $line . ",Flist conversion failed for Detail record\n";
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

        $lco_update_out_flistp = pcmif::pcm_perl_op($pcm_ctxp, 5,
                                                 0, $lco_update_flistp, $ebufp);

        if (pcmif::pcm_perl_is_err($ebufp)) 
        {
            &log ($ERROR, "Location update failed for Detail record: " . "\n$line" );
            #pcmif::pcm_perl_print_ebuf($ebufp);

            $opcode_error = $opcode_error + 1;

            $line =~ s/\n//;
            $line = $line . ",opcode execution failed, Please check cm.pinlog for detail\n";
            print ERROR "$line";
            &log ($ERROR, "Please correct and process the error file: " . "$error_file" );
            #==============================================
            # Destroy and create a new error buffer
            # Break and process next line
            #==============================================
            pcmif::pin_flist_destroy($lco_update_flistp);
            pcmif::pcm_perl_destroy_ebuf($ebufp);
            $ebufp = pcmif::pcm_perl_new_ebuf();
            goto NEXT_RECORD;
        }

        $lco_update_out_flist = pcmif::pin_perl_flist_to_str($lco_update_out_flistp, $ebufp);

        &log ($DEBUG, "Object updated successfully :" . "\n$location_update_out_flist" );
        $success_count = $success_count + 1;
        pcmif::pin_flist_destroy($lco_update_flistp);
        pcmif::pin_flist_destroy($lco_update_out_flistp);
		
	 		 
		

    }

    NEXT_RECORD:
        $flist = $orig_flist;
        $line = <INP>;
}


CLEANUP:
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
        print LOG "\n\t Records failed in opcode execution = $opcode_error \n";

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

