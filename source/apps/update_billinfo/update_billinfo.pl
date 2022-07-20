#!/home/pin/opt/portal/7.5/ThirdParty/perl/5.8.0/bin/perl
use pcmif;

my $src=$ARGV[0];

if ($src eq "") {
	print "Input file not specified\n";
	exit 200;
}

#--------------------------------------------------------------------------
# Assigning varibles for error and success files
#--------------------------------------------------------------------------
my $db_no = 1;
$error   = "billinfo_update.error";
$sucsess = "billinfo_update.success";

#--------------------------------------------------------------------------
#Opening file
#--------------------------------------------------------------------------
open(src, "<$src")	or die "Error could not open input file: $src!\n" ;
open my $ef, ">>", "$error" or die "Can't open the file: $!";
open my $sf, ">>", "$sucsess" or die "Can't open the file: $!";

#--------------------------------------------------------------------------
# pcm_perl_new_ebuf is a pcmif function which creates new 
# empty buffer and return a ebuf to handle errors
#--------------------------------------------------------------------------
$ebufp = pcmif::pcm_perl_new_ebuf();
	
#------------------------------------------------------------------------------------------
# pcm_perl_connect functions connects to BRM using PCM_CONNECT
#------------------------------------------------------------------------------------------
$pcm_ctxp = pcmif::pcm_perl_connect($db_no, $ebufp);

#------------------------------------------------------------------------------------------
# error checking if any error caused while pcm_connect
#------------------------------------------------------------------------------------------
if (pcmif::pcm_perl_is_err($ebufp)) 
{
    print $ef  "Error while connecting to cm\n$ebufp\n";	 
    exit(1);
}

while(my $row = <src>){

	chomp $row;
	my ($child_acct, $child_billinfo) = split(',',$row);
	print "Account_POID:- $child_acct\n";	
	
	my $main_flist = "";
	
	#--------------------------------------------
	#select gm.object_id0 child_acct ,
	#(select poid_id0 from billinfo_t where account_obj_id0 = gm.object_id0) child_billinfo,
	#(select actg_cycle_dom from billinfo_t where account_obj_id0 = gm.object_id0) child_dom,
	#(select bill_when from billinfo_t where account_obj_id0 = gm.object_id0) child_billwhen,
	#g.parent_id0 parent_acct,  b.poid_id0 parent_billinfo, b.actg_cycle_dom parent_dom, b.bill_when parent_bill_when
	#from group_t g, group_billing_members_t gm, billinfo_t b
	#where g.poid_id0 = gm.obj_id0
	#and b.account_obj_id0 = g.parent_id0
	#and gm.object_id0 = 4935497 ;*/	
	#--------------------------------------------

	my $main_flist = << "END"
0 PIN_FLD_POID           POID [0] 0.0.0.1 /account $child_acct 0
0 PIN_FLD_PROGRAM_NAME    STR [0] "Moving Corporate Child as Subordinates"
0 PIN_FLD_BILLINFO      ARRAY [0] allocated 20, used 2
1 PIN_FLD_POID           POID [0] 0.0.0.1 /billinfo $child_billinfo 0
1 PIN_FLD_BILL_WHEN   INT [0] 12
END
;	
	print "main_flist for $child_acct:-\n$main_flist\n";
	
	#------------------------------------------------------------------------------------------------
	#Converting flist from String to flist format(pin_perl_str_to_flist)
	#------------------------------------------------------------------------------------------------
	$in_flistp = pcmif::pin_perl_str_to_flist($main_flist, $db_no, $ebufp);
	  
	#------------------------------------------------------------------------------------------------
	#calling the opcode PCM_OP_SEARCH using pcmif::pcm_perl_op function
	#------------------------------------------------------------------------------------------------
	$out_flistp = pcmif::pcm_perl_op($pcm_ctxp, "PCM_OP_CUST_SET_BILLINFO", 0, $in_flistp, $ebufp);

	#------------------------------------------------------------------------------------------------
	# Checking for Error 
	# if any error print error in error log 
	#------------------------------------------------------------------------------------------------
	if (pcmif::pcm_perl_is_err($ebufp)) {
		print $sf "\nError for acct $child_acct";
		print $ef "\nError in the Opcode PCM_OP_CUST_SET_BILLINFO for _acct $child_acct \n$ebufp\n";
		#exit(1);
	}
	else {
		print $sf "\nSuccess for acct $child_acct";
	}
}

