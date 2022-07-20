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
$error   = "group_create.error";
$sucsess = "group_create.success";

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
	my $account_poid = $row;
	print "account_poid:- $account_poid\n";
	
	my $main_flist = "";
	
	#select a.poid_id0 from account_t a where a.account_no in (select distinct parent_account from rohan.group_creation_new);
	#select parent_account,count(child_account) from rohan.group_creation group by parent_account;
	#select child_account, count(child_account) from rohan.group_creation group by child_account having count(child_account) > 1;
	#select distinct a.account_no, a.group_obj_id0 from account_t a , rohan.group_creation g where 
	#		a.account_no = g.parent_account and group_obj_id0 > 0;

	#select object_id0 from group_billing_members_t gb where object_id0 in
	#(select distinct a.poid_id0 from rohan.group_creation r, account_t a 
	#		where r.child_account = a.account_no);

	my $main_flist = << "END"
0 PIN_FLD_POID		POID [0] 0.0.0.1 /account $account_poid 0
0 PIN_FLD_PARENT    POID [0] 0.0.0.1 /account $account_poid 0
0 PIN_FLD_NAME      STR [0] "0.0.0.1 /account $account_poid 0"
END
;	

	print "main_flist for $account_poid:-\n$main_flist\n";
	
	#------------------------------------------------------------------------------------------------
	#Converting flist from String to flist format(pin_perl_str_to_flist)
	#------------------------------------------------------------------------------------------------
	print "input_flist is: \n $main_flist";
	$in_flistp = pcmif::pin_perl_str_to_flist($main_flist, $db_no, $ebufp);
	
	print "input_flist is: \n $main_flist";
	
	#------------------------------------------------------------------------------------------------
	#calling the opcode PCM_OP_SEARCH using pcmif::pcm_perl_op function
	#------------------------------------------------------------------------------------------------
	$out_flistp = pcmif::pcm_perl_op($pcm_ctxp, "PCM_OP_BILL_GROUP_CREATE", 0, $in_flistp, $ebufp);

	#------------------------------------------------------------------------------------------------
	# Checking for Error 
	# if any error print error in error log 
	#------------------------------------------------------------------------------------------------
	if (pcmif::pcm_perl_is_err($ebufp)) {
		print $sf "\nError for account_poid $account_poid";
		print $ef "\nError in the Opcode PCM_OP_BILL_GROUP_CREATE for account_poid $account_poid \n$ebufp\n";
		#exit(1);
	}
	else {
		print $sf "\nSuccess for account_poid $account_poid";
	}
}
