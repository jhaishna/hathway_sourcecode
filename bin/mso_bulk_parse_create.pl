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
if($#ARGV != 3)
{
#        print "\nInvalid input parameter";
        print "\nUsage: mso_parse_create.pl -object_type <object_name> -input <input file name> ";
        print "\n       Ex: mso_parse_create.pl -object_type /mso_mta_job/device/stb -input stb_input_file \n";
        exit 1;
}

if ( $ARGV[0] ne '-object_type' or $ARGV[2] ne '-input' )
{
#        print "Input parameter validation error\n";
        print "\nUsage: mso_parse_create.pl -object_type <object_name> -input <input file name> ";
        print "\n       Ex: mso_parse_create.pl -object_type /mso_mta_job/device/stb -input stb_input_file \n";
        exit 1;
}

@object_values = split(/\//,$ARGV[1]);

if  ( @object_values[1] ne 'mso_mta_job' )
{
#        print "Input parameter value validation error\n";
        print "\nUsage: mso_parse_create.pl object_type <object_name> -input <input file name> ";
        print "\n       Ex: mso_parse_create.pl object_type /mso_mta_job/device/stb -input stb_input_file \n";
        exit 1;
}
else
{
    if (@object_values[2] eq 'device' )
    {
        $app_type  = @object_values[3];
    }
    else
    {
        #$app_type  = @object_values[2];
        $app_type  = @object_values[3];
    }
}

#======================================================
# Read the configuration file and 
# get the loglevel and  opcode number 
#======================================================
$conf_file = "./pin.conf";
open (CONF, $conf_file) || ( print "\nSEVERE: Configuration file does not exist.. Exiting\n" and  exit 1 );
while (<CONF>) {
    $conf = $_ ;
    if ($conf  =~ /mso_bulk_parse_create/ ) {
        
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

if (length $opcode <= 0) {
    print "\nSEVERE: Opcode number not configured for $ARGV[1] in pin.conf file.. Exiting\n";
    exit 1;
}

if (length $LOGLEVEL <= 0) {
    print "\nSEVERE: loglevel not configured for mso_parse_create in pin.conf file.. Exiting\n";
    exit 1;
}

#======================================================
# Set path for template file, flist file, input file
# log file, reject file and error file 
#======================================================
$input_file_name = $ARGV[3];
$template_file = "./template/" . $app_type . "/" . $app_type . ".TMPL";
$flist_file    = "./template/" . $app_type . "/" . $app_type . ".IFL";
$input_file    = "./data/" . $app_type . "/input/" . $input_file_name;
$log_file      = "./log/" . $app_type . "/" . $input_file_name . ".log";
$reject_file   = "./data/" . $app_type . "/reject/" . $input_file_name . ".reject"; #complete batch rejection
$error_file    = "./data/" . $app_type . "/error/" . $input_file_name . ".error";   # specific record which failed during processing
printf("$input_file");

open( INP, $input_file)     || ( print "\nSEVERE: Input file does not exist.. Exiting\n" and  exit 1 );
open (TMPL, $template_file) || ( print "\nSEVERE: Template file does not exist.. Exiting\n" and  exit 1) ;
open (FLIST, $flist_file)   || ( print "\nSEVERE: Flist file does not exist.. Exiting\n" and  exit 1);

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
print LOG "\nFlist file i          = $flist_file";
print LOG "\nOpcode = $opcode";
print LOG "\nloglevel = $LOGLEVEL";

print "\nProcessing Input file = $input_file";
print "\nTemplate file         = $template_file";
print "\nFlist file            = $flist_file";
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
while (<FLIST>) {
        if ($_  =~ m/^[0-9]/ ) {
        $flist .= $_ ;
    } 
}

$orig_flist = $flist ;
close(FLIST);

#======================================================
# Read the input file template and store it in variable
#======================================================
while (<TMPL>) {
    if ($_  !~ m/^[#,\n]/ ) {
        $tmpl .= $_ ;
    } 
}

@detail_values_tmpl = split(/\;/,$tmpl);
close(TMPL);

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


#======================================================
# Read the input file line by line
#======================================================
$line = <INP>;
while ($line) 
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

		if ( $app_type eq "bulk_suspension" or $app_type eq "bulk_reconnection" or $app_type eq "bulk_termination" ) 
		{
		
		    $account_no=@values[0];
		    $agreement_no=@values[1];
			$service_type=@values[2];
			

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

            &log ($DEBUG, "flist for account search :" . "\n$search_flist" );

            # Convert the flist string into an actual FList
            $search_flistp = pcmif::pin_perl_str_to_flist($search_flist, $db_no, $ebufp);
            if (pcmif::pcm_perl_is_err($ebufp))
            {
                    &log ($ERROR, "Search Flist conversion failed for Detail record: " . "\n$line" );
                    #pcmif::pcm_perl_print_ebuf($ebufp);

                    $parse_error = $parse_error + 1;

                    if ( $error_in_record == 0 )
                    {
                        print ERROR "$header_record";
                        $error_in_record = 1;
                    }

                    $line =~ s/\n//;
                    $line = $line . ",Search account failed for Detail record\n";
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
                &log ($ERROR, "account search failed for Detail record: " . "\n$line" );
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
                pcmif::pin_flist_destroy($search_flistp);
                pcmif::pcm_perl_destroy_ebuf($ebufp);
                $ebufp = pcmif::pcm_perl_new_ebuf();
                goto NEXT_RECORD;
            }

            $out_flist = pcmif::pin_perl_flist_to_str($search_out_flistp, $ebufp);

            &log ($DEBUG, "service account search output" . "\n$out_flist" );

            #======================================================
            # Get the LCO account poid and enrich the device flist 
            # with account_obj
            #======================================================
            @account_out = split(/\[0\]/, $out_flist);

            #======================================================
            # Validate LCO account poid
            #======================================================
            if (length @account_out[3] <= 0 ) 
            {
                    $field_validation_error = $field_validation_error + 1;
                    &log ($ERROR, "Invalid account no for record: " . "\n$line" );

                    if ( $error_in_record == 0 )
                    {
                        print ERROR "$header_record";
                        $error_in_record = 1;
                    }
                    
                    $field_position = ++$i;
                    $line =~ s/\n//;
                    $line = $line . ", Invalid account no\n";
                    print ERROR "$line";
                    &log ($ERROR, "Please correct and process the error file: " . "$error_file" );
                    goto NEXT_RECORD;
            }

            pcmif::pin_flist_destroy($search_flistp);
            pcmif::pin_flist_destroy($search_out_flistp);
            # account search complete
        
		    @account_out[3] =~ s/\n//;
		
		#===================================================
		# Service validations against the account
		#===================================================
		
		   
		my $ser_search_flist = << "END"
0 PIN_FLD_POID              POID [0]  0.0.0.1 /search -1
0 PIN_FLD_FLAGS     INT [0]  256
0 PIN_FLD_TEMPLATE        STR [0] "select X from /service/catv where F1 = V1 and F2 = V2 "
0 PIN_FLD_ARGS        ARRAY [1]
1   PIN_FLD_ACCOUNT_OBJ  POID [0] @account_out[3]
0 PIN_FLD_ARGS        ARRAY [2]
1 MSO_FLD_CATV_INFO    SUBSTRUCT [0] allocated 20, used 9
2     MSO_FLD_AGREEMENT_NO    STR [0] "$agreement_no"
0 PIN_FLD_RESULTS       ARRAY [0]
1  PIN_FLD_POID        POID [0] NULL
END
;

	    &log ($DEBUG, "flist for service search :" . "\n$ser_search_flist" );

            # Convert the flist string into an actual FList
            $ser_search_flistp = pcmif::pin_perl_str_to_flist($ser_search_flist, $db_no, $ebufp);
			    
            if (pcmif::pcm_perl_is_err($ebufp))
            {
                    &log ($ERROR, "Search Flist conversion failed for Detail record: " . "\n$line" );
                    #pcmif::pcm_perl_print_ebuf($ebufp);

                    $parse_error = $parse_error + 1;

                    if ( $error_in_record == 0 )
                    {
                        print ERROR "$header_record";
                        $error_in_record = 1;
                    }

                    $line =~ s/\n//;
                    $line = $line . ",Search service failed for Detail record\n";
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

		    $ser_search_out_flistp = pcmif::pcm_perl_op($pcm_ctxp, 7,
                                                         0, $ser_search_flistp, $ebufp);
           
            if (pcmif::pcm_perl_is_err($ebufp))
            {
                &log ($ERROR, "service search failed for Detail record: " . "\n$line" );
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
                pcmif::pin_flist_destroy($ser_search_flistp);
                pcmif::pcm_perl_destroy_ebuf($ebufp);
                $ebufp = pcmif::pcm_perl_new_ebuf();
                goto NEXT_RECORD;
            }
			
            $ser_out_flist = pcmif::pin_perl_flist_to_str($ser_search_out_flistp, $ebufp);
            &log ($DEBUG, "service account search output" . "\n$ser_out_flist" );

            #======================================================
            # Get the service poid and enrich the device flist 
            # with account_obj
            #======================================================
            @ser_poid_out = split(/\[0\]/, $ser_out_flist);

            #======================================================
            # Validate service poid
            #======================================================
            if (length @ser_poid_out[3] <= 0 ) 
            {
                    $field_validation_error = $field_validation_error + 1;
                    &log ($ERROR, "Invalid service for record: " . "\n$line" );

                    if ( $error_in_record == 0 )
                    {
                        print ERROR "$header_record";
                        $error_in_record = 1;
                    }
                    
                    $field_position = ++$i;
                    $line =~ s/\n//;
                    $line = $line . ", Invalid service for record\n";
                    print ERROR "$line";
                    &log ($ERROR, "Please correct and process the error file: " . "$error_file" );
                    goto NEXT_RECORD;
            }

            pcmif::pin_flist_destroy($ser_search_flistp);
            pcmif::pin_flist_destroy($ser_search_out_flistp);
            # Service search complete
        	
		 @account_out[3] =~ s/\n//;
		 
		 if ($app_type eq "bulk_change_bouquet_id")
		 {
		 $flist =~ s/account_obj/@account_out[3]/g;
         	 $flist =~ s/service_obj/@ser_poid_out[3]/g;
		 }
		 else
		 {
		  $flist =~ s/account_no/@account_out[3]/g;
		 }
		#===================================================
		# Service validations ends
		#===================================================
		
         $flist =~ s/service_obj/@ser_poid_out[3]/g;
		}
		
		if ( $app_type eq "bulk_repush" )
                {
                    $order_id=@values[0];
                    $order_id=~s/\n//;
                    
            my $search_flist = << "END"
0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
0 PIN_FLD_FLAGS           INT [0] 256
0 PIN_FLD_TEMPLATE        STR [0] "select X from /mso_prov_order where F1 = V1 "
0 PIN_FLD_ARGS          ARRAY [1] allocated 20, used 1
1     PIN_FLD_ORDER_ID      STR [0] "$order_id"
0 PIN_FLD_RESULTS       ARRAY [0] allocated 20, used 4
1 PIN_FLD_POID           POID [0] NULL 
1 MSO_FLD_TASK_FAILED   INT [0] 0 
1 PIN_FLD_INPUT_FLIST     BUF [0]
END
;

  &log ($DEBUG, "flist for provisioning search :" . "\n$search_flist" );

            # Convert the flist string into an actual FList
            $search_flistp = pcmif::pin_perl_str_to_flist($search_flist, $db_no, $ebufp);
            #$search_flistp_failed_task = pcmif::pin_perl_str_to_flist($search_flist_failed_task, $db_no, $ebufp);
            if (pcmif::pcm_perl_is_err($ebufp))
            {
                    &log ($ERROR, "Search Flist conversion failed for Detail record: " . "\n$line" );
                    #pcmif::pcm_perl_print_ebuf($ebufp);

                    $parse_error = $parse_error + 1;

                    if ( $error_in_record == 0 )
                    {
                        print ERROR "$header_record";
                        $error_in_record = 1;
                    }

                    $line =~ s/\n//;
                    $line = $line . ",Search account failed for Detail record\n";
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
                &log ($ERROR, "order search failed for Detail record: " . "\n$line" );
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
                pcmif::pin_flist_destroy($search_flistp);
                pcmif::pcm_perl_destroy_ebuf($ebufp);
                $ebufp = pcmif::pcm_perl_new_ebuf();
                goto NEXT_RECORD;
            }

			
        	$out_flist = pcmif::pin_perl_flist_to_str($search_out_flistp, $ebufp);

            &log ($DEBUG, "provisioning order search output" . "\n$out_flist" );

            #======================================================
            # Get the LCO account poid and enrich the device flist
            # with account_obj
            #======================================================
            @order_out = split(/\[0\]/, $out_flist);

            #======================================================
            # Validate order poid
            #======================================================
            if (length @order_out[3] <= 0 )
            {
                    $field_validation_error = $field_validation_error + 1;
                    &log ($ERROR, "Invalid provisioning order for record: " . "\n$line" );

                    if ( $error_in_record == 0 )
                    {
                        print ERROR "$header_record";
                        $error_in_record = 1;
                    }

                    $field_position = ++$i;
                    $line =~ s/\n//;
                    $line = $line . ", Invalid provisoining order search\n";
                    print ERROR "$line";
                    &log ($ERROR, "Please correct and process the error file: " . "$error_file" );
                    goto NEXT_RECORD;
            }

                    
		 $flist_output_flistp_buf = pcmif::pcm_perl_op($pcm_ctxp, 43006,
                                                         0, $search_out_flistp, $ebufp);
                 

                 if (pcmif::pcm_perl_is_err($ebufp))
            {
                &log ($ERROR, "flist conversion failed for Detail record: " . "\n$line" );
                #pcmif::pcm_perl_print_ebuf($ebufp);

                $opcode_error = $opcode_error + 1;

                if ( $error_in_record == 0 )
                {
                    print ERROR "$header_record";
                    $error_in_record = 1;
                }

                $line =~ s/\n//;
                $line = $line . "43006,opcode execution failed, Please check cm.pinlog for detail\n";
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

                       #printf(" Flist to xml conversion output $flist");

                       $flist=pcmif::pin_perl_flist_to_str($flist_output_flistp_buf, $ebufp);

			pcmif::pin_flist_destroy($search_flistp);
                        pcmif::pin_flist_destroy($search_out_flistp);
                        pcmif::pin_flist_destroy($flist_output_flistp_buf);

                    #@order_out[3] =~ s/\n//; 
                    #@order_out_buf[3] =~ s/\n//; 
                    
                       #printf(" Flist to xml conversion output $search_output_flistp_buf");

                }                 

		if ( $app_type eq "bulk_set_personnel_bit" )
        {
		
		  $agreement_no=@values[0];
		
		#===================================================
		# Service validations against the account
		#===================================================
		
		   
		my $ser_search_flist = << "END"
0 PIN_FLD_POID              POID [0]  0.0.0.1 /search -1
0 PIN_FLD_FLAGS     INT [0]  256
0 PIN_FLD_TEMPLATE        STR [0] "select X from /service/catv where F1 = V1 "
0 PIN_FLD_ARGS        ARRAY [1]
1 MSO_FLD_CATV_INFO    SUBSTRUCT [0] allocated 20, used 9
2     MSO_FLD_AGREEMENT_NO    STR [0] "$agreement_no"
0 PIN_FLD_RESULTS       ARRAY [0]
1 PIN_FLD_POID    POID [0]
1     PIN_FLD_LOGIN           STR [0] ""
1     MSO_FLD_CATV_INFO    SUBSTRUCT [0] allocated 20, used 9
2         MSO_FLD_AGREEMENT_NO    STR [0] ""
END
;
		 &log ($DEBUG, "flist for service search :" . "\n$ser_search_flist" );

            # Convert the flist string into an actual FList
            $ser_search_flistp = pcmif::pin_perl_str_to_flist($ser_search_flist, $db_no, $ebufp);
			    
            if (pcmif::pcm_perl_is_err($ebufp))
            {
                    &log ($ERROR, "Search Flist conversion failed for Detail record: " . "\n$line" );
                    #pcmif::pcm_perl_print_ebuf($ebufp);

                    $parse_error = $parse_error + 1;

                    if ( $error_in_record == 0 )
                    {
                        print ERROR "$header_record";
                        $error_in_record = 1;
                    }

                    $line =~ s/\n//;
                    $line = $line . ",Search service failed for Detail record\n";
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

		    $ser_search_out_flistp = pcmif::pcm_perl_op($pcm_ctxp, 7,
                                                         0, $ser_search_flistp, $ebufp);
           
            if (pcmif::pcm_perl_is_err($ebufp))
            {
                &log ($ERROR, "service search failed for Detail record: " . "\n$line" );
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
                pcmif::pin_flist_destroy($ser_search_flistp);
                pcmif::pcm_perl_destroy_ebuf($ebufp);
                $ebufp = pcmif::pcm_perl_new_ebuf();
                goto NEXT_RECORD;
            }
			
			$ser_out_flist = pcmif::pin_perl_flist_to_str($ser_search_out_flistp, $ebufp);
            &log ($DEBUG, "service account search output" . "\n$ser_out_flist" );

            #======================================================
            # Get the service poid and enrich the device flist 
            # with account_obj
            #======================================================
            @ser_poid_out = split(/\[0\]/, $ser_out_flist);

            #======================================================
            # Validate service poid
            #======================================================
            if (length @ser_poid_out[3] <= 0 ) 
            {
                    $field_validation_error = $field_validation_error + 1;
                    &log ($ERROR, "Invalid service for record: " . "\n$line" );

                    if ( $error_in_record == 0 )
                    {
                        print ERROR "$header_record";
                        $error_in_record = 1;
                    }
                    
                    $field_position = ++$i;
                    $line =~ s/\n//;
                    $line = $line . ", Invalid service for record\n";
                    print ERROR "$line";
                    &log ($ERROR, "Please correct and process the error file: " . "$error_file" );
                    goto NEXT_RECORD;
            }

             @ser_poid_out[3]=~ s/\n//;
             @ser_poid_out[4]=~ s/\n//;
             $flist =~ s/service_obj/@ser_poid_out[3]/g;
             $flist =~ s/login/@ser_poid_out[4]/g;
            pcmif::pin_flist_destroy($ser_search_flistp);
            pcmif::pin_flist_destroy($ser_search_out_flistp);
            # Service search complete
        	
		
		#===================================================
		# Service validations ends
		#===================================================
		
		
		}
		
		if( $app_type eq  "bulk_osd_pk" or $app_type eq  "bulk_email_pk" or $app_type eq  "bulk_retrack" )
		{
		    $account_no  = @values[0];
		
			my $search_flist = << "END"
0 PIN_FLD_POID              POID [0]  0.0.0.1 /search -1  
0 PIN_FLD_FLAGS     INT [0] 256 
0 PIN_FLD_TEMPLATE        STR [0] "select X from /account where F1 = V1 " 
0 PIN_FLD_ARGS        ARRAY [1] 
1   PIN_FLD_ACCOUNT_NO  STR [0] "$account_no"
0 PIN_FLD_RESULTS       ARRAY [0]
1  PIN_FLD_POID        POID [0] NULL
END
;

            &log ($DEBUG, "flist for account search :" . "\n$search_flist" );

            # Convert the flist string into an actual FList
            $search_flistp = pcmif::pin_perl_str_to_flist($search_flist, $db_no, $ebufp);
            if (pcmif::pcm_perl_is_err($ebufp))
            {
                    &log ($ERROR, "Search Flist conversion failed for Detail record: " . "\n$line" );
                    #pcmif::pcm_perl_print_ebuf($ebufp);

                    $parse_error = $parse_error + 1;

                    if ( $error_in_record == 0 )
                    {
                        print ERROR "$header_record";
                        $error_in_record = 1;
                    }

                    $line =~ s/\n//;
                    $line = $line . ",Search account failed for Detail record\n";
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
                &log ($ERROR, "account search failed for Detail record: " . "\n$line" );
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
                pcmif::pin_flist_destroy($search_flistp);
                pcmif::pcm_perl_destroy_ebuf($ebufp);
                $ebufp = pcmif::pcm_perl_new_ebuf();
                goto NEXT_RECORD;
            }

            $out_flist = pcmif::pin_perl_flist_to_str($search_out_flistp, $ebufp);

            &log ($DEBUG, "service account search output" . "\n$out_flist" );

            #======================================================
            # Get the account poid and enrich the  flist 
            # with account_obj
            #======================================================
            @account_out = split(/\[0\]/, $out_flist);

            #======================================================
            # Validate account poid
            #======================================================
            if (length @account_out[3] <= 0 ) 
            {
                    $field_validation_error = $field_validation_error + 1;
                    &log ($ERROR, "Invalid account no for record: " . "\n$line" );

                    if ( $error_in_record == 0 )
                    {
                        print ERROR "$header_record";
                        $error_in_record = 1;
                    }
                    
                    $field_position = ++$i;
                    $line =~ s/\n//;
                    $line = $line . ", Invalid account no\n";
                    print ERROR "$line";
                    &log ($ERROR, "Please correct and process the error file: " . "$error_file" );
                    goto NEXT_RECORD;
            }

            pcmif::pin_flist_destroy($search_flistp);
            pcmif::pin_flist_destroy($search_out_flistp);
            # account search complete
		
		    #########################################
			# Device validations begins
			######################################### 
		
		      $mac_address=@values[3];
			  $mac_address =~ s/\n//;
			  
			  if ( $app_type eq  "bulk_retrack" )
			  {
			      $mac_address=@values[1];
				  $mac_address =~ s/\n//;
              }			  
			 		   
		my $dev_search_flist = << "END"
0 PIN_FLD_POID              POID [0]  0.0.0.1 /search -1
0 PIN_FLD_FLAGS     INT [0]  256
0 PIN_FLD_TEMPLATE        STR [0] "select X from /device where F1 = V1 "
0 PIN_FLD_ARGS        ARRAY [1]
1   PIN_FLD_DEVICE_ID STR [0] "$mac_address"
0 PIN_FLD_RESULTS       ARRAY [0]
1  PIN_FLD_POID        POID [0] NULL
END
;
		    &log ($DEBUG, "flist for Device search :" . "\n$dev_search_flist" );

            # Convert the flist string into an actual FList
            $dev_search_flistp = pcmif::pin_perl_str_to_flist($dev_search_flist, $db_no, $ebufp);
			    
            if (pcmif::pcm_perl_is_err($ebufp))
            {
                    &log ($ERROR, "Search Flist conversion failed for Detail record: " . "\n$line" );
                    #pcmif::pcm_perl_print_ebuf($ebufp);

                    $parse_error = $parse_error + 1;

                    if ( $error_in_record == 0 )
                    {
                        print ERROR "$header_record";
                        $error_in_record = 1;
                    }

                    $line =~ s/\n//;
                    $line = $line . ",Search device failed for Detail record\n";
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

		    $dev_search_out_flistp = pcmif::pcm_perl_op($pcm_ctxp, 7,
                                                         0, $dev_search_flistp, $ebufp);
           
            if (pcmif::pcm_perl_is_err($ebufp))
            {
                &log ($ERROR, "device search failed for Detail record: " . "\n$line" );
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
                pcmif::pin_flist_destroy($dev_search_flistp);
                pcmif::pcm_perl_destroy_ebuf($ebufp);
                $ebufp = pcmif::pcm_perl_new_ebuf();
                goto NEXT_RECORD;
            }
			
			$dev_search_out_flist = pcmif::pin_perl_flist_to_str($dev_search_out_flistp, $ebufp);
            &log ($DEBUG, "Device account search output" . "\n$dev_search_out_flistp" );

            #======================================================
            # Get the Device poid and enrich the device flist 
            # with account_obj
            #======================================================
            @dev_poid_out = split(/\[0\]/, $dev_search_out_flist);

            #======================================================
            # Validate Device poid
            #======================================================
            if (length @dev_poid_out[3] <= 0 ) 
            {
                    $field_validation_error = $field_validation_error + 1;
                    &log ($ERROR, "Invalid Device for record: " . "\n$line" );

                    if ( $error_in_record == 0 )
                    {
                        print ERROR "$header_record";
                        $error_in_record = 1;
                    }
                    
                    $field_position = ++$i;
                    $line =~ s/\n//;
                    $line = $line . ", Invalid Device for record\n";
                    print ERROR "$line";
                    &log ($ERROR, "Please correct and process the error file: " . "$error_file" );
                    goto NEXT_RECORD;
            }

            pcmif::pin_flist_destroy($dev_search_flistp);
            pcmif::pin_flist_destroy($dev_search_out_flistp);
            # Device search complete
        	
			#if( $app_type eq "bulk_email_pk" )
		   #{
		        @account_out[3]=~ s/\n//;
		        $flist =~ s/account_obj/@account_out[3]/g; 
		   #}
			
			
		    #########################################
			# Device validations ends
			#########################################
		
		
		}
		
					
    	  if( $app_type eq "bulk_act_adj" or 
	      $app_type eq "bulk_add_plan" or 
	      $app_type eq "bulk_remove_plan" or 
	      $app_type eq "bulk_change_plan")
	  {
			   
	  	$account_no  = @values[0];
	
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

		&log ($DEBUG, "flist for account search :" . "\n$search_flist" );

		# Convert the flist string into an actual FList
		$search_flistp = pcmif::pin_perl_str_to_flist($search_flist, $db_no, $ebufp);
		if (pcmif::pcm_perl_is_err($ebufp))
		{
			&log ($ERROR, "Search Flist conversion failed for Detail record: " . "\n$line" );
			#pcmif::pcm_perl_print_ebuf($ebufp);

			$parse_error = $parse_error + 1;

			if ( $error_in_record == 0 )
			{
			    print ERROR "$header_record";
			    $error_in_record = 1;
			}

			$line =~ s/\n//;
			$line = $line . ",Search account failed for Detail record\n";
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

		$search_out_flistp = pcmif::pcm_perl_op($pcm_ctxp, 7, 0, $search_flistp, $ebufp);

		if (pcmif::pcm_perl_is_err($ebufp))
		{
			&log ($ERROR, "account search failed for Detail record: " . "\n$line" );
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
			pcmif::pin_flist_destroy($search_flistp);
			pcmif::pcm_perl_destroy_ebuf($ebufp);
			$ebufp = pcmif::pcm_perl_new_ebuf();
			goto NEXT_RECORD;
		}

		$out_flist = pcmif::pin_perl_flist_to_str($search_out_flistp, $ebufp);

		&log ($DEBUG, "service account search output" . "\n$out_flist" );

		#======================================================
		# Get the LCO account poid and enrich the device flist 
		# with account_obj
		#======================================================
		@account_out = split(/\[0\]/, $out_flist);

		#======================================================
		# Validate account poid
		#======================================================
		if (length @account_out[3] <= 0 ) 
		{
		    $field_validation_error = $field_validation_error + 1;
		    &log ($ERROR, "Invalid account no for record: " . "\n$line" );

		    if ( $error_in_record == 0 )
		    {
	                print ERROR "$header_record";
	                $error_in_record = 1;
		    }
			
		    $field_position = ++$i;
		    $line =~ s/\n//;
		    $line = $line . ", Invalid account no\n";
		    print ERROR "$line";
		    &log ($ERROR, "Please correct and process the error file: " . "$error_file" );
		    goto NEXT_RECORD;
		}

		pcmif::pin_flist_destroy($search_flistp);
		pcmif::pin_flist_destroy($search_out_flistp);
		# account search complete
        
		   			   
	        @account_out[3]=~ s/\n//;
	        $flist =~ s/account_obj/@account_out[3]/g; 
		
	  }
		   if ( $app_type eq "bulk_act_adj" )
		   {
		   
		     $bill_no  = @values[2];
			 @account_out[3]=~ s/\n//;
		   
		    my $act_adj_src_flist = << "END"
0 PIN_FLD_DESCR           STR [0] "MSO_OP_AR_GET_DETAILS Search"
0 PIN_FLD_FLAGS           INT [0] 3
0 MSO_FLD_SERVICE_TYPE   ENUM [0] 1
0 PIN_FLD_PROGRAM_NAME    STR [0] "MSO_OP_AR_GET_DETAILS"
0 PIN_FLD_BILL_NO         STR [0] "1"
0 PIN_FLD_POID     POID [0] @account_out[3]
END
;

				&log ($DEBUG, "flist for account search :" . "\n$adj_src_flist" );

				# Convert the flist string into an actual FList
				$act_adj_src_flistp = pcmif::pin_perl_str_to_flist($act_adj_src_flist, $db_no, $ebufp);
				if (pcmif::pcm_perl_is_err($ebufp))
				{
					&log ($ERROR, "Search Flist conversion failed for Detail record: " . "\n$line" );
					#pcmif::pcm_perl_print_ebuf($ebufp);

					$parse_error = $parse_error + 1;

					if ( $error_in_record == 0 )
					{
						print ERROR "$header_record";
						$error_in_record = 1;
					}

					$line =~ s/\n//;
					$line = $line . ",Search account failed for Detail record\n";
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

				$act_adj_src_out_flistp = pcmif::pcm_perl_op($pcm_ctxp,13150,
														 0, $act_adj_src_flistp, $ebufp);
														 
		        #printf(" Account adj output flist $act_adj_src_out_flistp\n");												 

				if (pcmif::pcm_perl_is_err($ebufp))
				{
				&log ($ERROR, "account search failed for Detail record: " . "\n$line" );
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
					pcmif::pin_flist_destroy($act_adj_src_out);
					pcmif::pcm_perl_destroy_ebuf($ebufp);
					$ebufp = pcmif::pcm_perl_new_ebuf();
					goto NEXT_RECORD;
				}

				$act_adj_src_out_flist = pcmif::pin_perl_flist_to_str($act_adj_src_out_flistp, $ebufp);

				&log ($DEBUG, "adj account balance grp search output" . "\n$act_adj_src_out_flist" );

				#======================================================
				# Get the account poid and enrich the device flist 
				# with account_obj
				#======================================================
				#@bal_grp_out = split(/\[0\]/, $act_adj_src_out_flist);
				
				@bal_grp_res_out = split(/PIN_FLD_NAME/,$act_adj_src_out_flist);
				
				
                @ser_level_bal_grp_out='';			
			
				for (my $i = 0; $i < @bal_grp_res_out; $i++)
				{
					 
					#pick service group 
					if(@bal_grp_res_out[$i] =~ m/"Service Balance Group"/) 
					{ @ser_bal_grp_out = split(/\[0\]/,@bal_grp_res_out[$i]);
                      @ser_level_bal_grp_out=split(/0 PIN_FLD_RESULTS/,@ser_bal_grp_out[2]);
		      @ser_level_bal_grp_out_sp=split(/0 PIN_FLD_STATUS/,@ser_level_bal_grp_out[0]);
					  
					}
					#pick default balance group 
					if(@bal_grp_res_out[$i] =~ m/"Default Balance Group"/) 
					{ 
					  @def_bal_grp_out = split(/\[0\]/,@bal_grp_res_out[$i]);
                      @def_level_bal_grp_out=split(/0 PIN_FLD_RESULTS/,@def_bal_grp_out[2]);
					  @def_level_bal_grp_out_sp=split(/0 PIN_FLD_STATUS/,@def_level_bal_grp_out[0]);
					}
										
				}
			
			    @ser_level_bal_grp_out_sp[0]=~ s/\n//;
				@def_level_bal_grp_out_sp[0]=~ s/\n//;
				
                # Set service level balance group if service level balance group exists				
                if (length @ser_level_bal_grp_out_sp[0] > 2)
			    {
				    @ser_level_bal_grp_out_sp[0]=~ s/\n//;
				    @balance_grp=@ser_level_bal_grp_out_sp[0];
                    #printf(" balance grp set to service balance grp");							
				}
				else
				{	@def_level_bal_grp_out[0]=~ s/\n//;
				   	@balance_grp=@def_level_bal_grp_out[0];	
                    #printf(" balance grp set to default balance grp");					
				}
				
				$balance_grp=~ s/\n//;
			
				#======================================================
				# Validate account poid
				#======================================================
				if (length @balance_grp <= 0 ) 
				{
						$field_validation_error = $field_validation_error + 1;
						&log ($ERROR, "Invalid account no for record: " . "\n$line" );

						if ( $error_in_record == 0 )
						{
							print ERROR "$header_record";
							$error_in_record = 1;
						}
						
						$field_position = ++$i;
						$line =~ s/\n//;
						$line = $line . ", Invalid balance grp for no\n";
						print ERROR "$line";
						&log ($ERROR, "Please correct and process the error file:balance_group " . "$error_file" );
						goto NEXT_RECORD;
				}

				pcmif::pin_flist_destroy($act_adj_src_flistp);
				pcmif::pin_flist_destroy($act_adj_src_out_flistp);
				# account search complete
		        
				$flist =~ s/account_obj/@account_out[3]/g;
				
				$flist =~ s/balance_group/@balance_grp/g;

			
		   }
		   
		   if ( $app_type eq "bulk_bill_adj" )
		   {
		     $bill_no=@values[0];
		   
		   my $bill_adj_src_flist = << "END"
0 PIN_FLD_POID              POID [0]  0.0.0.1 /search -1  
0 PIN_FLD_FLAGS     INT [0] 512 
0 PIN_FLD_TEMPLATE        STR [0] "select X from /bill where F1 = V1 " 
0 PIN_FLD_ARGS        ARRAY [1] 
1   PIN_FLD_BILL_NO  STR [0] "$bill_no"
0 PIN_FLD_RESULTS       ARRAY [0]
1  PIN_FLD_POID        POID [0] NULL
END
;

				&log ($DEBUG, "flist for bill search :" . "\n$bill_adj_src_flist" );

				# Convert the flist string into an actual FList
				$bill_adj_src_flistp = pcmif::pin_perl_str_to_flist($bill_adj_src_flist, $db_no, $ebufp);
				if (pcmif::pcm_perl_is_err($ebufp))
				{
					&log ($ERROR, "Search Flist conversion failed for Detail record: " . "\n$line" );
					#pcmif::pcm_perl_print_ebuf($ebufp);

					$parse_error = $parse_error + 1;

					if ( $error_in_record == 0 )
					{
						print ERROR "$header_record";
						$error_in_record = 1;
					}

					$line =~ s/\n//;
					$line = $line . ",Search bill failed for Detail record\n";
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

				$bill_adj_src_out_flistp = pcmif::pcm_perl_op($pcm_ctxp, 7,
														 0, $bill_adj_src_flistp, $ebufp);

				if (pcmif::pcm_perl_is_err($ebufp))
				{
				&log ($ERROR, "account search failed for Detail record: " . "\n$line" );
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
					pcmif::pin_flist_destroy($bill_adj_src_flistp);
					pcmif::pcm_perl_destroy_ebuf($ebufp);
					$ebufp = pcmif::pcm_perl_new_ebuf();
					goto NEXT_RECORD;
				}

				$bill_adj_src_out_flist = pcmif::pin_perl_flist_to_str($bill_adj_src_out_flistp, $ebufp);

				&log ($DEBUG, "service account search output" . "\n$bill_adj_src_out_flist" );

				#======================================================
				# Get the bill and with account_obj
				#======================================================
				@bill_out = split(/\[0\]/, $bill_adj_src_out_flist);

				#======================================================
				# Validate bill poid
				#======================================================
				if (length @bill_out[3] <= 0 ) 
				{
						$field_validation_error = $field_validation_error + 1;
						&log ($ERROR, "Invalid account no for record: " . "\n$line" );

						if ( $error_in_record == 0 )
						{
							print ERROR "$header_record";
							$error_in_record = 1;
						}
						
						$field_position = ++$i;
						$line =~ s/\n//;
						$line = $line . ", bill no\n";
						print ERROR "$line";
						&log ($ERROR, "Please correct and process the error file: " . "$error_file" );
						goto NEXT_RECORD;
				}

				pcmif::pin_flist_destroy($bill_adj_src_flistp);
				pcmif::pin_flist_destroy($bill_adj_src_out_flistp);
				# account search complete
        
		   			   
		        @bill_out[3]=~ s/\n//;
		        $flist =~ s/bill_obj/@bill_out[3]/g;
		     
		   }
		   
		   
#		   if ( $app_type eq "bulk_change_bouquet_id" or $app_type eq "bulk_change_plan")
		   if ( $app_type eq "bulk_change_plan")
		   {
		        @account_out[3]=~ s/\n//;
		        $flist =~ s/account_obj/@account_out[3]/g;
						   
		   }
		   
		if ( $app_type eq "bulk_change_bouquet_id" )
        {
		
		    $vc_id       =@values[0];
		    $bouquet_id  =@values[1];
			

            my $search_flist = << "END"
0 PIN_FLD_POID               POID [0]  0.0.0.1 /search -1
0 PIN_FLD_FLAGS               INT [0] 512
0 PIN_FLD_TEMPLATE            STR [0] "select X from /service where F1 = lpad(V1, 12, '0') "
0 PIN_FLD_ARGS              ARRAY [1]
1     PIN_FLD_ALIAS_LIST    ARRAY [0] allocated 20, used 1
2         PIN_FLD_NAME        STR [0] "$vc_id"
0 PIN_FLD_RESULTS           ARRAY [0] allocated 20, used 1
1   PIN_FLD_ACCOUNT_OBJ      POID [0] NULL
END
;

            &log ($DEBUG, "flist for account search :" . "\n$search_flist" );

            # Convert the flist string into an actual FList
            $search_flistp = pcmif::pin_perl_str_to_flist($search_flist, $db_no, $ebufp);
            if (pcmif::pcm_perl_is_err($ebufp))
            {
                    &log ($ERROR, "Search Flist conversion failed for Detail record: " . "\n$line" );
                    #pcmif::pcm_perl_print_ebuf($ebufp);

                    $parse_error = $parse_error + 1;

                    if ( $error_in_record == 0 )
                    {
                        print ERROR "$header_record";
                        $error_in_record = 1;
                    }

                    $line =~ s/\n//;
                    $line = $line . ",Search account failed for Detail record\n";
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
                &log ($ERROR, "account search failed for Detail record: " . "\n$line" );
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
                pcmif::pin_flist_destroy($search_flistp);
                pcmif::pcm_perl_destroy_ebuf($ebufp);
                $ebufp = pcmif::pcm_perl_new_ebuf();
                goto NEXT_RECORD;
            }

            $out_flist = pcmif::pin_perl_flist_to_str($search_out_flistp, $ebufp);

            &log ($DEBUG, "service account search output" . "\n$out_flist" );

            #======================================================
            # Get the LCO account poid and enrich the device flist 
            # with account_obj
            #======================================================
            @acnt_srvc_out = split(/\[0\]/, $out_flist);

            #======================================================
            # Validate account and service 
            #======================================================

            if (length @acnt_srvc_out[3] <= 0 ) 
            {
                    $field_validation_error = $field_validation_error + 1;
                    &log ($ERROR, "Invalid account no for record: " . "\n$line" );

                    if ( $error_in_record == 0 )
                    {
                        print ERROR "$header_record";
                        $error_in_record = 1;
                    }
                    
                    $field_position = ++$i;
                    $line =~ s/\n//;
                    $line = $line . ", Invalid account no\n";
                    print ERROR "$line";
                    &log ($ERROR, "Please correct and process the error file: " . "$error_file" );
                    goto NEXT_RECORD;
            }

            #pcmif::pin_flist_destroy($search_flistp);
            #pcmif::pin_flist_destroy($search_out_flistp);
            # account search complete
        

			@tmp_arr=split("\n", @acnt_srvc_out[3]);
			@acnt_srvc_out[3]=@tmp_arr[0];
		    @acnt_srvc_out[3] =~ s/\n//;
			
			if (length @acnt_srvc_out[4] <= 0 ) 
            {
                    $field_validation_error = $field_validation_error + 1;
                    &log ($ERROR, "Invalid account no for record: " . "\n$line" );

                    if ( $error_in_record == 0 )
                    {
                        print ERROR "$header_record";
                        $error_in_record = 1;
                    }
                    
                    $field_position = ++$i;
                    $line =~ s/\n//;
                    $line = $line . ", Invalid account no\n";
                    print ERROR "$line";
                    &log ($ERROR, "Please correct and process the error file: " . "$error_file" );
                    goto NEXT_RECORD;
            }

            #pcmif::pin_flist_destroy($search_flistp);
            #pcmif::pin_flist_destroy($search_out_flistp);
            # account search complete
        
		    @acnt_srvc_out[4] =~ s/\n//;
		#===================================================
		# Service validations against the account
		#===================================================
            #pcmif::pin_flist_destroy($ser_search_flistp);
            #pcmif::pin_flist_destroy($ser_search_out_flistp);
            # Service search complete
        	
		 @acnt_srvc_out[3] =~ s/\n//;
		 $bouquet_id =~ s/\n//;

		$flist =~ s/account_obj/@acnt_srvc_out[3]/g;
		$flist =~ s/service_obj/@acnt_srvc_out[4]/g;
		$flist =~ s/bouquet_id/$bouquet_id/g;


		}		
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
        foreach (@detail_values_tmpl)
        {
            @detail_tmpl_values = split(/:/,@detail_values_tmpl[$i]);
            if (@values[$i] !~ @detail_tmpl_values[1] ) 
            {
                $field_validation_error = $field_validation_error + 1;
                &log ($ERROR, "Field validation error for record: " . "\n$line" );
                &log ($ERROR, "values[$i] = @values[$i]::::: detail_tmpl_values[1]=@detail_tmpl_values[1] \n" );

                if ( $error_in_record == 0 )
                {
                    print ERROR "$header_record";
                    $error_in_record = 1;
                }
                
                $field_position = ++$i;
                $line =~ s/\n//;
                $line = $line . ", Field " . "$field_position " . @detail_tmpl_values[0] .  " does not meet field validation requirement\n";
                print ERROR "$line";
                &log ($ERROR, "Please correct and process the error file: " . "$error_file" );
                goto NEXT_RECORD;
            }

            #======================================================
            # Substitute the flist template with actual values
            #======================================================
            @values[$i] =~ s/\n//;
            $flist =~ s/@detail_tmpl_values[0]/@values[$i]/g;
            $i++;
        }
		
     
        #=====================================================
        # Create buffer to store as BLOB
        #======================================================
        $flistp = pcmif::pin_perl_str_to_flist($flist, $db_no, $ebufp);
		
        if (pcmif::pcm_perl_is_err($ebufp)) 
        {
            &log ($ERROR, "Flist conversion failed for blob creation: " . "\n$flistp" );
            #pcmif::pcm_perl_print_ebuf($ebufp);

            $parse_error = $parse_error + 1;

            if ( $error_in_record == 0 )
            {
                print ERROR "$header_record";
                $error_in_record = 1;
            }

            $line =~ s/\n//;
            $line = $line . ",Flist conversion failed for blob creation\n";
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

        $out_flistp = pcmif::pcm_perl_op($pcm_ctxp, 43004,
                                                 0, $flistp, $ebufp);
        	
        if (pcmif::pcm_perl_is_err($ebufp)) 
        {
            &log ($ERROR, "BLOB creation failed for Detail record: " . "\n$line" );
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
            pcmif::pin_flist_destroy($flistp);
            pcmif::pcm_perl_destroy_ebuf($ebufp);
            $ebufp = pcmif::pcm_perl_new_ebuf();
            goto NEXT_RECORD;
        }

        $out = pcmif::pin_perl_flist_to_str($out_flistp, $ebufp);
		
		&log ($DEBUG, "Buffer created :" . "\n$out" );
        #======================================================
        # Get the buffer data and enrich the device flist 
        # with account_obj
        #======================================================
        @field = split(/\n/, $out);
        @buffer = split(/data:/, $out);

        pcmif::pin_flist_destroy($flistp);
        pcmif::pin_flist_destroy($out_flistp);
        # Buffer creation completed


        #===================================================
        # mso_mta_job object creation starts here 
        #===================================================
            my $obj_create_flist = << "END"
0 PIN_FLD_POID              POID [0]  0.0.0.1 /mso_mta_job/@object_values[3] -1  
0 PIN_FLD_OPCODE     INT [0] $opcode
0 PIN_FLD_STATUS        ENUM [0] 0
END
;

        #======================================================
        # Add buffer data
        #======================================================
        $obj_create_flist = $obj_create_flist . @field[2] . @buffer[1] ;

        # Convert the flist string into an actual FList
        $obj_create_flistp = pcmif::pin_perl_str_to_flist($obj_create_flist, $db_no, $ebufp);
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

        $obj_create_out_flistp = pcmif::pcm_perl_op($pcm_ctxp, 1,
                                                 0, $obj_create_flistp, $ebufp);

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
            pcmif::pin_flist_destroy($obj_create_flistp);
            pcmif::pcm_perl_destroy_ebuf($ebufp);
            $ebufp = pcmif::pcm_perl_new_ebuf();
            goto NEXT_RECORD;
        }

        $obj_create_out_flist = pcmif::pin_perl_flist_to_str($obj_create_out_flistp, $ebufp);

        &log ($DEBUG, "Object created successfully :" . "\n$obj_create_out_flist" );
        $success_count = $success_count + 1;
        pcmif::pin_flist_destroy($obj_create_flistp);
        pcmif::pin_flist_destroy($obj_create_out_flistp);

    

    NEXT_RECORD:
        $flist = $orig_flist;
        $line = <INP>;
}


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
