#!/home/pin/opt/portal/7.5/ThirdParty/perl/5.8.0/bin/perl

use Sys::Hostname;
use File::Copy;
use File::Basename;
use File::Path;

#======================================================
# Input parameter validation
#======================================================
#print $#ARGV;
if($#ARGV != 1)
{
        print "\nUsage: mso_parse_create.pl -input <input file name> ";
        print "\nNot enough arguments supplied\n";
	exit 1;
}

if ( $ARGV[0] ne '-input' )
{
        print "\nUsage: mso_parse_create.pl -input <input file name> ";
        print "\n Input file not supplied \n";
	exit 1;
}

#======================================================
# Set path for input file
# log file, reject file and error file 
#======================================================
$input_file_name = $ARGV[1];
$input_file    = "./data/csv_creation/input/" . $input_file_name;
$log_file      = "./log/csv_creation/" . $input_file_name . ".log";
$reject_file   = "./data/csv_creation/reject/" . $input_file_name . ".reject"; #rejected records
$error_file   = "./data/csv_creation/error/" . $input_file_name . ".error";
$done_file   = "./data/csv_creation/done/" . $input_file_name . ".csv";

`:>$done_file`;
`:>$reject_file`;
`:>$log_file`;

#printf("$input_file");
#======================================================
# For information purpose
#======================================================
print "\nProcessing Input file = $input_file";
print "\nError file         = $error_file";
print "\nLog file              = $log_file";
print "\nreject file              = $reject_file";

#=====================================================
# set to 1 if error happens in detail record
#=====================================================
$error_record = 0;
$return_status = 0;

#=====================================================
# Variables used to report processing summary
#=====================================================
$total_no_of_records = 0;       # total number of detail record
$success_count = 0;              # Successfully processed record

#======================================================
# Read the input file line by line
#======================================================
open(my $logh, '>>', $log_file) or die "Could not open file '$log_file' $!";
open(my $inp, '<', $input_file) or die "Could not open file '$input_file' $!";
print $logh "INFO - Creation of CSV continues\n";
while (my $line = <$inp>) 
{

        $total_no_of_records = $total_no_of_records + 1;

        my @values = split(/,/, $line);
		
	#======================================================
        # Record level validation
        #======================================================
		
		   	$account_no=@values[0];
		    	$adjust_amount=@values[1];
			$trans_type=@values[2];
			$tax_type=@values[3];
			$date=@values[4];
			$descr=@values[5];

				#print "\n$account_no,$adj_amount,$tax_type,$date,$descr";
	
				$acct_poid = `sqlplus -s pin/pin123\@prodbrm << EOF
				set feedback off head off pages 0 linesize 500
				select poid_id0 from account_t where account_no='$account_no';
EOF`;
		
			#======================================================
        		# Account number validation
        		#======================================================
	
			$sqlerrorverify = join(' ', $acct_poid);
			if ($sqlerrorverify =~ /ORA-01017/) {
			       	print $logh "\n###########################################################################################################\n";	
				print $logh "ERROR - Unable to login to SQLPLUS. Please configure DB properties in mainscript.pl\n";
				print "ERROR - Unable to login to SQLPLUS. Please configure DB properties\n";
				$descr =~ s/^\s+|\s+$//g;
				print $logh "Error record - $account_no,$adjust_amount,$trans_type,$tax_type,$date,$descr,'Invalid account'\n";
				print "ERROR - $sqlerrorverify\n";
				$error_record= $error_record + 1;
				exit;
			}	
			elsif ($sqlerrorverify =~ /ORA-/ || $sqlerrorverify =~ /SP2/) {
				print $logh "\n###########################################################################################################\n";
				print $logh "ERROR - Error while connecting to SQLPLUS\n";
				print "ERROR - Error while connecting to SQLPLUS\n";
				$descr =~ s/^\s+|\s+$//g;
				print $logh "ERROR - $account_no,$adjust_amount,$trans_type,$tax_type,$date,$descr,'Invalid account'\n";
                                print "ERROR - $sqlerrorverify\n";
				$error_record= $error_record + 1;
				exit;
			}
			elsif ($sqlerrorverify =~ /ORA-00904/) {
			       	print $logh "\n###########################################################################################################\n";	
				print $logh "ERROR - The account number you passed is not valid\n";
				print "\nERROR - Account number is not valid\n";
				open(my $sfh, '>>', $reject_file) or die "Could not open file '$reject_file' $!";
                                $descr =~ s/^\s+|\s+$//g;
				print $sfh "$account_no,$adjust_amount,$trans_type,$tax_type,$date,$descr,'Invalid account'\n";
                                print $logh "ERROR - $sqlerrorverify\n";
				close $sfh;
				$error_record= $error_record + 1;
				next;
			}
			elsif ($sqlerrorverify == ' ') {
                                print $logh "\n###########################################################################################################\n"; 
				print $logh "ERROR - The account number you passed is not valid\n";
                                print "ERROR - Account number is not valid\n";
                                $descr =~ s/^\s+|\s+$//g;
				print $logh "ERROR - $account_no,$adjust_amount,$trans_type,$tax_type,$date,$descr,'Invalid account'\n";
				open(my $sfh, '>>', $reject_file) or die "Could not open file '$reject_file' $!";
                                print $sfh "$account_no,$adjust_amount,$trans_type,$tax_type,$date,$descr,'Invalid account'\n";
                                close $sfh;
				$error_record= $error_record + 1;
                                next;
                        }

			#======================================================
                        # Transaction type validation
                        #======================================================
			
			if (uc $trans_type eq "D")
			{
				
				$adj_amount=$adjust_amount - ($adjust_amount * 2);
			
			}
			elsif (uc $trans_type eq "C")
			{
				
				$adj_amount=$adjust_amount;
			
			}
			else {
			
				print $logh "\n###########################################################################################################\n";
				print $logh "Transaction Type is invalid for the current record\n";
				print "Error-Transaction Type is invalid for the current record\n";	
				open(my $cfh, '>>', $reject_file) or die "Could not open file '$reject_file' $!";
				$descr =~ s/^\s+|\s+$//g;
				print $cfh "$account_no,$adjust_amount,$trans_type,$tax_type,$date,$descr\n";
				print $logh "ERROR - $account_no,$adjust_amount,$trans_type,$tax_type,$date,$descr,'Invalid transaction Type(Actual type is C or D)'\n";
				close $cfh;
				$error_record= $error_record + 1;	
				next;			
			}
	
			#======================================================
                        # Tax type validation
                        #======================================================	
			if (uc $tax_type ne "TAX" and uc $tax_type ne "NOTAX")
			{
				print $logh "\n###########################################################################################################\n";
				print $logh "Error - tax Type is invalid for the current record\n";
				print "Error-tax Type is invalid for the current record\n";	
				open(my $tfh, '>>', $reject_file) or die "Could not open file '$reject_file' $!";
				$descr =~ s/^\s+|\s+$//g;
				print $logh "ERROR - $account_no,$adjust_amount,$trans_type,$tax_type,$date,$descr,'Tax Type error(TAX or NOTAX)'\n";
				print $tfh "$account_no,$adjust_amount,$trans_type,$tax_type,$date,$descr\n";
				close $tfh;	
				$error_record= $error_record + 1;
				next;
							
			}
			#======================================================
                        # Date Format validation
                        #======================================================
			if ($date !~ m/^(1[0-2]|0[1-9])\/(3[01]|[12][0-9]|0[1-9])\/[0-9]{4}$/)
			{
			
				print $logh "\n###########################################################################################################\n";
				print $logh "Error - Date format is invalid for the current record\n";
                                print "Error - Date format is no valid for the current record\n";		
				open(my $rfh, '>>', $reject_file) or die "Could not open file '$reject_file' $!";
				$descr =~ s/^\s+|\s+$//g;
				print $logh "ERROR - $account_no,$adjust_amount,$trans_type,$tax_type,$date,$descr,'Date Format is wrong(Actual Format-MM/DD/YYYY)'\n";
				print $rfh "$account_no,$adjust_amount,$trans_type,$tax_type,$date,$descr\n";		
      				close $rfh;
				$error_record= $error_record + 1;
				next;	
			}

			
			#======================================================
                        # Description validation
                        #======================================================
                        #if ($descr eq ' ' or $descr eq '')
			chomp($descr);
                        if (!($descr))
                        {

                                print $logh "\n###########################################################################################################\n";
                                print $logh "Error - Description is missing for the current record\n";
                                print "Error - Description is missing for the current record\n";
                                open(my $rfh, '>>', $reject_file) or die "Could not open file '$reject_file' $!";
                                $descr =~ s/^\s+|\s+$//g;
                                print $logh "ERROR - $account_no,$adjust_amount,$trans_type,$tax_type,$date,$descr,'Description missingdescription Mandatory)'\n";
                                print $rfh "$account_no,$adjust_amount,$trans_type,$tax_type,$date,$descr\n";
                                close $rfh;
                                $error_record= $error_record + 1;
                                next;
                        }
			
			#======================================================
                        # Setting tax non tax values
                        #======================================================	
			if (uc $tax_type ne "TAX"){
                                $tax_type=1;

                        }
                        else{
                                $tax_type=2;
                        }


			$success_count = $success_count + 1;
			open(my $fh, '>>',$done_file) or die "Could not open file '$done_file' $!";
			$acct_poid =~ s/^\s+|\s+$//g;
			#print "0.0.0.1 /account $acct_poid,$adj_amount,,$tax_type,,,356,$date,1,1,$descr\n";
			print $fh "0.0.0.1 /account $acct_poid,$adj_amount,,$tax_type,,,356,$date,1,1,$descr\n";
			close $fh;		
			
}	
close $inp;	

print $logh "\n###########################################################################################################\n";
print $logh "Total number of record:$total_no_of_records\n";
print $logh "Number of Rejsected records:$error_record\n";
print $logh "Number of successfull records:$success_count\n";
print $logh "\n###########################################################################################################\n";

close $logh;	
