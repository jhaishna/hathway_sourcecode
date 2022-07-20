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
        print "\nUsage: mso_catv_cfg_pt_loader.pl -input <input file name> ";
        exit 1;
}

if ( $ARGV[0] ne '-input' )
{
#        print "Input parameter validation error\n";
        print "\nUsage: mso_catv_cfg_pt_loader.pl -input <input file name> ";
        exit 1;
}

#======================================================
# Set path for template file, flist file, input file
# log file, reject file and error file 
#======================================================
$input_file = $ARGV[1];
$log_file      = $app_type . "/log/" . $input_file_name . ".log";
$error_file    = $input_file . ".error";   # specific record which failed during processing

open( INP, $input_file)     || ( print "\nSEVERE: Input file does not exist.. Exiting\n" and  exit 1 );

#===========================================================
# Insanity check if the input file is already processed.
# Check log file, reject file or error file already exist
#==========================================================
if (-e $error_file ) {
    print "\nSEVERE: Error file $error_file already exist. Please change the input Input file name or backup the error file.. Exiting\n" ;  
    exit 1 
}

open ( ERROR , ">$error_file")|| ( print "\nSEVERE: Can't open error file for writting.. Exiting\n" and  exit 1);

#======================================================
# For information purpose
#======================================================
print "\nProcessing Input file = $input_file";

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
}


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
        $total_no_of_records = $total_no_of_records + 1;

        my @values = split(/,/, $line);

        #======================================================
        # Record level validation
        #======================================================
        if ( @values != 4)
        {
            $parse_error = $parse_error + 1;
            $line =~ s/\n//;
            $line = $line . ",Incorrect number of field for the record\n";
            print ERROR "$line";
            goto NEXT_RECORD;
        }


        $order_type = "0.0.0.1 /mso_cfg_catv_pt" . " -1";
        ($PROV_TAG,$PKG_TYPE,$NDS_CA_ID,$PK_CA_ID) = split(/,/, $line);

        $PROV_TAG = trim($PROV_TAG);
        $PKG_TYPE = trim($PKG_TYPE);
        $NDS_CA_ID = trim($NDS_CA_ID);
        $PK_CA_ID = trim($PK_CA_ID);

        if (  !(length $PROV_TAG > 0  and 
                length $PKG_TYPE > 0 and 
                length $NDS_CA_ID > 0 and 
                length $PK_CA_ID > 0 ) ) 
        {
            $field_validation_error = $field_validation_error + 1;
            $line =~ s/\n//;
            $line = $line . ",One or more fields are null in the record\n";
            print ERROR "$line";
            goto NEXT_RECORD;
        }


        my $search_flist = << "END"
0 PIN_FLD_POID                 POID [0]  0.0.0.1 /search -1
0 PIN_FLD_FLAGS                 INT [0] 768
0 PIN_FLD_TEMPLATE              STR [0] "select X from /mso_cfg_catv_pt where F1 = V1 "
0 PIN_FLD_ARGS                ARRAY [1]
1   PIN_FLD_PROVISIONING_TAG    STR [0] "$PROV_TAG"
0 PIN_FLD_RESULTS             ARRAY [0]
1   MSO_FLD_CATV_PT_DATA      ARRAY [*] allocated 1, used 2
END
;
	print "\n\nsearch_flist=$search_flist";

        # Convert the flist string into an actual FList
        $search_flistp = pcmif::pin_perl_str_to_flist($search_flist, $db_no, $ebufp);
        if (pcmif::pcm_perl_is_err($ebufp))
        {
                $parse_error = $parse_error + 1;
                $line =~ s/\n//;
                $line = $line . ",Search Provisioning tag failed for Detail record\n";
                print ERROR "$line";
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
           #pcmif::pcm_perl_print_ebuf($ebufp);

           $opcode_error = $opcode_error + 1;
           $line =~ s/\n//;
           $line = $line . ",opcode execution failed in search, Please check cm.pinlog for detail\n";
           print ERROR "$line";
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
	
        print "\n\nout_flist=$out_flist";

	$nds_ca_id_exist=0;
	$pk_ca_id_exist=0;
	$elem_id=0;
	$pk_array="";
	$nds_array="";
	@prov_tag_out="";

	my @lines = split('\n', $out_flist);
	foreach my $line(@lines)
	{
	        if ( $line =~ m/^1.*PIN_FLD_POID/ )
	        {
	                @prov_tag_out = split(/\[0\]/, $line);
	                #print "\nline=$line\n";
	                #print "\nprov_tag_out[1]=@prov_tag_out[1]\n";
	        }
	
	        if ( $line =~ m/$NDS_CA_ID/ )
	        {
	                #print "\nline=$line\n";
	                $nds_ca_id_exist=1;
	        }
	
	        if ( $line =~ m/$PK_CA_ID/ )
	        {
	                $pk_ca_id_exist=1;
	                #print "\nline=$line\n";
	        }
	
	        if ( $line =~ m/^1.*MSO_FLD_CATV_PT_DATA/ )
	        {
	                my @arr = split(/\[/, $line);
	                my @arr1 = split(/\]/, @arr[1]);
        	        $current_elemid=@arr1[0];
                	print "\n\tcurrent_elemid=$current_elemid";
	                $elem_id = $elem_id +1;
	                #print "\nline=$line\n";
	        }
	}
        if ( $elem_id <= $current_elemid )
      	{
               	$elem_id = $current_elemid + 1 ;
      	}
	
	print "\nnds_ca_id_exist=$nds_ca_id_exist";
	print "\nnds_ca_id=$NDS_CA_ID\n";
	print "\npk_ca_id_exist=$pk_ca_id_exist";
	print "\npk_ca_id=$PK_CA_ID\n";
	
	if ($nds_ca_id_exist == 0)
	{
	        $elem_id_1=$elem_id + 1;
	        $nds_array=<< "END"
0 MSO_FLD_CATV_PT_DATA     ARRAY [$elem_id]
1    MSO_FLD_CA_ID           STR [0] "$NDS_CA_ID"
1    MSO_FLD_NETWORK_NODE    STR [0] ""
END
	}
	else
	{
		$elem_id_1=$elem_id;
	}
	
	if ($pk_ca_id_exist == 0)
	{
	        $pk_array=<< "END"
0 MSO_FLD_CATV_PT_DATA     ARRAY [$elem_id_1]
1    MSO_FLD_CA_ID           STR [0] "$PK_CA_ID"
1    MSO_FLD_NETWORK_NODE    STR [0] ""
END
	}
	
	$catv_pt_data_array=$nds_array . $pk_array;
	print "\ncatv_pt_data_array=$catv_pt_data_array";




       #======================================================
       # Get the LCO account poid and enrich the device flist 
       # with account_obj
       #======================================================

#       @prov_tag_out = split(/\[0\]/, $out_flist);
       pcmif::pin_flist_destroy($search_flistp);
       pcmif::pin_flist_destroy($search_out_flistp);
       # Provisioning tag search complete
     
       #======================================================
       # Modify if found else create a new one
       #======================================================
       print "\nprov_tag_out[1]=@prov_tag_out[1]";
       if (length @prov_tag_out[1] > 0 ) 
       {	
	    print ("\nAAA");
	    if (length $catv_pt_data_array <= 0 )
	    {
	    	print ("\nBBB");
		goto NEXT_RECORD;
	    }    
	    print ("\nCCC");
            #@LCO_out[3] =~ s/\n//;

            #===================================================
            # mso_mta_job object modify starts here 
            #===================================================
            my $obj_modify_flist = << "END"
0 PIN_FLD_POID              POID [0]  @prov_tag_out[1]
0 PIN_FLD_PROGRAM_NAME       STR [0] "pt_loader"
0 PIN_FLD_PROVISIONING_TAG   STR [0] "$PROV_TAG"
0 MSO_FLD_PKG_TYPE          ENUM [0] $PKG_TYPE
END
;
	$obj_modify_flist=$obj_modify_flist . $catv_pt_data_array;

	    print "\nobj_modify_flist=\n$obj_modify_flist\n";
            # Convert the flist string into an actual FList
            $obj_modify_flistp = pcmif::pin_perl_str_to_flist($obj_modify_flist, $db_no, $ebufp);
            if (pcmif::pcm_perl_is_err($ebufp)) 
            {
                $parse_error = $parse_error + 1;
                $line =~ s/\n//;
                $line = $line . ",Flist conversion failed for Detail record\n";
                print ERROR "$line";
                #==============================================
                # Destroy and create a new error buffer
                # Break and process next line
                #==============================================
                pcmif::pcm_perl_destroy_ebuf($ebufp);
                $ebufp = pcmif::pcm_perl_new_ebuf();
                goto NEXT_RECORD;
            }
	
            $obj_modify_out_flistp = pcmif::pcm_perl_op($pcm_ctxp, 5,
                                                     32, $obj_modify_flistp, $ebufp);

            if (pcmif::pcm_perl_is_err($ebufp)) 
            {
                $opcode_error = $opcode_error + 1;
                $line =~ s/\n//;
                $line = $line . ",opcode execution failed in update, Please check cm.pinlog for detail\n";
                print ERROR "$line";
                #==============================================
                # Destroy and create a new error buffer
                # Break and process next line
                #==============================================
                pcmif::pin_flist_destroy($obj_modify_flistp);
                pcmif::pcm_perl_destroy_ebuf($ebufp);
                $ebufp = pcmif::pcm_perl_new_ebuf();
                goto NEXT_RECORD;
            }

            $obj_modify_out_flist = pcmif::pin_perl_flist_to_str($obj_modify_out_flistp, $ebufp);

            pcmif::pin_flist_destroy($obj_modify_flistp);
            pcmif::pin_flist_destroy($obj_modify_out_flistp);

       }
       else
       {
            #===================================================
            # mso_mta_job object creation starts here 
            #===================================================
            my $obj_create_flist = << "END"
0 PIN_FLD_POID              POID [0]  0.0.0.1 /mso_cfg_catv_pt -1  
0 PIN_FLD_PROGRAM_NAME	     STR [0] "pt_loader"
0 PIN_FLD_PROVISIONING_TAG   STR [0] "$PROV_TAG"
0 MSO_FLD_PKG_TYPE          ENUM [0] $PKG_TYPE
0 MSO_FLD_CATV_PT_DATA     ARRAY [0]
1    MSO_FLD_CA_ID	     STR [0] "$NDS_CA_ID"
1    MSO_FLD_NETWORK_NODE    STR [0] ""
0 MSO_FLD_CATV_PT_DATA     ARRAY [1]
1    MSO_FLD_CA_ID	     STR [0] "$PK_CA_ID"
1    MSO_FLD_NETWORK_NODE    STR [0] ""
END
;
	    print "\nobj_create_flist=\n$obj_create_flist\n";

            # Convert the flist string into an actual FList
            $obj_create_flistp = pcmif::pin_perl_str_to_flist($obj_create_flist, $db_no, $ebufp);
            if (pcmif::pcm_perl_is_err($ebufp)) 
            {
                $parse_error = $parse_error + 1;
                $line =~ s/\n//;
                $line = $line . ",Flist conversion failed for Detail record\n";
                print ERROR "$line";
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
                $opcode_error = $opcode_error + 1;
                $line =~ s/\n//;
                $line = $line . ",opcode execution in crate, Please check cm.pinlog for detail\n";
                print ERROR "$line";
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

            pcmif::pin_flist_destroy($obj_create_flistp);
            pcmif::pin_flist_destroy($obj_create_out_flistp);

       }
     
       $success_count = $success_count + 1;
    }

    NEXT_RECORD:
        $line = <INP>;
}


CLEANUP:
#        $success_count = $total_no_of_records - $parse_error - $field_validation_error - $opcode_error;
    if (pcmif::pcm_perl_is_err($ebufp)) 
    {
        print "\n\t\t\t------------------------------------";
        print "\n\t\t\tError: Procees completed with ERROR";
        print "\n\t\t\tPlease check log file for detail ERROR";
        print "\n\t\t\t------------------------------------";
    }
    else
    {
        print "\n\nProcessing Summary";
        print "\n\t Total number of records = $total_no_of_records";
        print "\n\t Records failed in parsing = $parse_error";
        print "\n\t Records failed in field validation = $field_validation_error";
        print "\n\t Records failed in opcode execution = $opcode_error";
        print "\n\t Successfully processed record count = $success_count \n";

    }

    close(INP) if (<FILE_IS_OPEN>);
    close(ERROR) if (<FILE_IS_OPEN>);


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

