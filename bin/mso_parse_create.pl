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
        $app_type  = @object_values[2];
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
    if ($conf  =~ /mso_parse_create/ ) {
        
        if ($conf  =~ /$ARGV[1] opcode/) {
            @conf_values = split(/ /,$conf);
            $opcode =  @conf_values[4];
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
    if  ($line =~ m/^HEADER/ and ( $app_type eq "stb" or 
                    $app_type eq "vc" or 
                    $app_type eq "modem" or 
                    $app_type eq "router" or 
                    $app_type eq "mc"  )) 
    {
        #======================================================
        # Header Found: Create an order object
        # Order object is to be created for only devices
        #======================================================
        &log ($DEBUG, "\nProcessing Header:" . "\n$line" );

        $header_record = $line;
        $order_type = "0.0.0.1 /mso_order/" . $app_type . " -1";
        ($header_sym,$sp_code,$supp_code,$ref_inv_no,$po_no,$ref_inv_date) = split(/,/, $line);

        $sp_code = trim($sp_code);
        $supp_code = trim($supp_code);
        $ref_inv_no = trim($ref_inv_no);
        $ref_inv_date = trim($ref_inv_date);
        $po_no = trim($po_no);

        if (  !(length $sp_code > 0  and 
                length $supp_code > 0 and 
                length $ref_inv_no > 0 and 
                length $ref_inv_date == 8 and 
                length $po_no > 0 ) ) 
        {
            move ($input_file, $reject_file) or die "Copy failed: $!";
            &log ($SEVERE, "Header validation failed. Please correct and reprocess the reject file:" . "$reject_file" );
            print "Severe: Header validation failed.. Exiting";
            # exit if error in header record
            $return_status = 1;
            goto CLEANUP;
        }

        my $order_flist = << "END"
0 PIN_FLD_POID              POID [0]  $order_type 
0 MSO_FLD_STOCK_POINT_CODE     STR [0] "$sp_code" 
0 MSO_FLD_SUPPLIER_CODE        STR [0] "$supp_code" 
0 MSO_FLD_INVOICE_NO        STR [0] "$ref_inv_no" 
0 MSO_FLD_INVOICE_DATE        STR [0] "$ref_inv_date" 
0 MSO_FLD_PO_NO            STR [0] "$po_no"
END
;

        &log ($DEBUG, "flist for Device Order creation opcode call:" . "\n$order_flist" );
        # Convert the flist string into an actual FList
        $order_flistp = pcmif::pin_perl_str_to_flist($order_flist, $db_no, $ebufp);
        if (pcmif::pcm_perl_is_err($ebufp))
        {
                #pcmif::pcm_perl_print_ebuf($ebufp);
                move ($input_file, $reject_file) or die "Copy failed: $!";
                &log ($SEVERE, "Flist conversion failed for Header record. Please correct and reprocess the reject file:" . "$reject_file" );
                print "\nSevere: Flist conversion failed for Header record.. Exiting";
                # exit if error in header record
                $return_status = 1;
                goto CLEANUP;
        }
        $out_flistp = pcmif::pcm_perl_op($pcm_ctxp, 1,
                                             0, $order_flistp, $ebufp);

        if (pcmif::pcm_perl_is_err($ebufp))
        {
                #pcmif::pcm_perl_print_ebuf($ebufp);
                move ($input_file, $reject_file) or die "Copy failed: $!";
                &log ($SEVERE, "Order Object creation failed for Header record. Please correct and reprocess the reject file:" . "$reject_file" );
                print "Severe: Order Object creation failed for Header record.. Exiting \n";
                pcmif::pin_flist_destroy($order_flistp);
                # exit if error in header record
                $return_status = 1;
                goto CLEANUP;
        }

        $out = pcmif::pin_perl_flist_to_str($out_flistp, $ebufp);
        &log ($DEBUG, "Order Object Poid is:" . "\n$out" );

        #======================================================
        # Get the Order poid and enrich the device flist 
        # with order poid 
        #======================================================
        @order_poid = split(/\[0\]/, $out);
        #print LOG "order poid @order_poid[1]\n";
        @order_poid[1] =~ s/\n//;
        $flist =~ s/order_obj/@order_poid[1]/;
        $orig_flist = $flist ;
        &log ($DEBUG, "Device flist after enriching Order_obj:" . "\n$flist" );
        pcmif::pin_flist_destroy($order_flistp);
        pcmif::pin_flist_destroy($out_flistp);

    }
    elsif ( $line =~ m/^HEADER/ )
    {
        #======================================================
        # Ignore the header except of devices            
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

        #======================================================
        # Header validation for device loading
        #======================================================
        if ( ! @order_poid and ( $app_type eq "stb" or
                                        $app_type eq "vc" or
                                        $app_type eq "modem" or
                                        $app_type eq "router" or
                                        $app_type eq "mc"  ))
        {
            print "\nHeader is required for devices. Exiting..";
            $parse_error = $parse_error + 1;
            move ($input_file, $reject_file) or die "Copy failed: $!";
            &log ($SEVERE, "Header is required for devices. Please correct and reprocess the reject file: " . "$reject_file" );
            $return_status = 1;
            goto CLEANUP;
        }

        my @values = split(/,/, $line);

        #======================================================
        # Record level validation
        #======================================================
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
            if (@values[$i] !~ @detail_tmpl_values[1] or @values[$i] =~ @detail_tmpl_values[2] ) 
            {
                $field_validation_error = $field_validation_error + 1;
                &log ($ERROR, "Field validation error for record: " . "\n$line" );

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

         if ( $app_type eq "catv_preactivation" )
        {
            #======================================================
            # LCO Search starts here
            #======================================================
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

            &log ($DEBUG, "flist for LCO search :" . "\n$search_flist" );

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

            &log ($DEBUG, "LCO search output" . "\n$out_flist" );

            #======================================================
            # Get the LCO account poid and enrich the device flist 
            # with account_obj
            #======================================================
            @LCO_out = split(/\[0\]/, $out_flist);

            #======================================================
            # Validate LCO account poid
            #======================================================
            if (length @LCO_out[3] <= 0 ) 
            {
                    $field_validation_error = $field_validation_error + 1;
                    &log ($ERROR, "Invaid LCO account no for record: " . "\n$line" );

                    if ( $error_in_record == 0 )
                    {
                        print ERROR "$header_record";
                        $error_in_record = 1;
                    }
                    
                    $field_position = ++$i;
                    $line =~ s/\n//;
                    $line = $line . ", Invaid LCO account no\n";
                    print ERROR "$line";
                    &log ($ERROR, "Please correct and process the error file: " . "$error_file" );
                    goto NEXT_RECORD;
            }

            @LCO_out[3] =~ s/\n//;
            $flist =~ s/LCO_obj/@LCO_out[3]/;
            &log ($DEBUG, "Device flist after enriching LCO_obj:" . "\n$flist" );
            pcmif::pin_flist_destroy($search_flistp);
            pcmif::pin_flist_destroy($search_out_flistp);
            # LCO search complete
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
0 PIN_FLD_POID              POID [0]  0.0.0.1 $ARGV[1] -1  
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
            &log ($ERROR, "Flist conversion failed for Detail record: " . "\n$line" );
            #pcmif::pcm_perl_print_ebuf($ebufp);

            $parse_error = $parse_error + 1;

            if ( $error_in_record == 0 )
            {
                print ERROR "$header_record";
                $error_in_record = 1;
            }

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

    }

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
