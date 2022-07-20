#!/home/pin/opt/portal/7.5/ThirdParty/perl/5.8.0/bin/perl

use pcmif;
use Sys::Hostname;
use File::Copy;
use File::Basename;
use File::Path;

$args = $#ARGV + 1;
#foreach (@ARGV) {
#        print "$_\n";
#}
$date = `date +%Y%m%d`;
chomp($date);
$log_file = "mso_dsa_create_critaria$date.log";


#======================================================
# Input parameter validation
#======================================================
if($#ARGV != 1)
{
#       print "\nInvalid input parameter";
        print "\nUsage: $0 -input <input file name> ";
        print "\n       Ex: $0 -input create_rule_associate.csv\n";
	print "Sample input CSV file format will be like below\n";
	print "ACCOUNT_NO,ACCOUNT_POID,RULE_ID,BILL_WHEN,START_T,END_T,ACTIVE_FLAG\n";
        exit 1;
}

if ( $ARGV[0] ne '-input' )
{
#       print "Input parameter validation error\n";
	print "\nUsage: $0 -input <input file name> ";
        print "\n       Ex: $0 -input create_rule_associate.csv\n";
	print "Sample input CSV file format will be like below\n";
	print "ACCOUNT_NO,ACCOUNT_POID,RULE_ID,BILL_WHEN,START_T,END_T,ACTIVE_FLAG\n";
        exit 1;
}

if (!-e $ARGV[1])
{
	print "Error : Input file $ARGV[1] not present\n";
	print "\nUsage: $0 -input <input file name> ";
        print "\n     Ex: $0 -input create_rule_associate.csv\n";
	print "Sample input CSV file format will be like below\n";
	print "ACCOUNT_NO,ACCOUNT_POID,RULE_ID,BILL_WHEN,START_T,END_T,ACTIVE_FLAG\n";
        exit 1;
}

#===========================================================
# Insanity check if the input file is already processed.
# Check log file, reject file or error file already exist
#==========================================================
if (-e $log_file ) {
    print "\nSEVERE: Log file $log_file already exist. Please change the input Input file name or backup the log file.. Exiting\n" ;  
    exit 1 
}

open ( LOG, ">$log_file")   || ( print "\nSEVERE: Can't open log file for writting.. Exiting\n" and  exit 1);

#======================================================
# For information purpose
#======================================================
print LOG "\nProcessing Input file";

open (FH,"< $ARGV[1]") or die "Unable to open input file for reading";
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


#======================================================
# Read the input file line by line
#======================================================
while ($line = <FH>) 
{
	print "Inside the wile\n";
	my ($act_no,$act_poid,$rule_id,$bill_when,$start_t, $end_t, $flag) = split(/,/,$line);
        my $order_flist = << "END"
0 PIN_FLD_POID           POID [0] 0.0.0.1 /dsa_rule_associate -1 0
0 PIN_FLD_ACCOUNT_NO      STR [0] "$act_no"
0 MSO_FLD_SALESMAN_OBJ   POID [0] 0.0.0.1 /account $act_poid 0
0 MSO_FLD_RULE_ID         INT [0] $rule_id
0 PIN_FLD_BILL_CYCLE_T TSTAMP [0] ($bill_when) <null>
0 PIN_FLD_START_T      TSTAMP [0] ($start_t) <null>
0 PIN_FLD_END_T        TSTAMP [0] ($end_t) <null>
0 PIN_FLD_FLAGS           INT [0] $flag
END
;

        # Convert the flist string into an actual FList
	print LOG "Input flist is :\n $order_flist\n";
        $order_flistp = pcmif::pin_perl_str_to_flist($order_flist, $db_no, $ebufp);
        if (pcmif::pcm_perl_is_err($ebufp))
        {
                #pcmif::pcm_perl_print_ebuf($ebufp);
                print "\nSevere: Flist conversion failed.. Exiting";
                print LOG "\nSevere: Flist conversion failed.. Exiting";
                # exit if error in header record
		exit (1);
        }
        $out_flistp = pcmif::pcm_perl_op($pcm_ctxp, 1,
                                             0, $order_flistp, $ebufp);

        if (pcmif::pcm_perl_is_err($ebufp))
        {
                #pcmif::pcm_perl_print_ebuf($ebufp);
                print "Severe: Opcode Execution failed.. Exiting \n";
                print LOG "Severe: Opcode Execution failed.. Exiting \n";
                pcmif::pin_flist_destroy($order_flistp);
                # exit if error in header record
		exit (1);
        }

        $out = pcmif::pin_perl_flist_to_str($out_flistp, $ebufp);
	print "Output flist final is $out\n";
	print LOG "Output flist final is\n $out\n";
        pcmif::pin_flist_destroy($order_flistp);
        pcmif::pin_flist_destroy($out_flistp);

    }
