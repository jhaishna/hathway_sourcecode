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
$error   = "attach_child.error";
$sucsess = "attach_child.success";

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
	@account_poids = split(',',$row);
	my $group_obj = @account_poids[0];
	print "group_obj:- $group_obj\n";

	splice @account_poids, 0, 1;
	print "account_poids:- @account_poids\n";
	
	my $i = 0;
	my $dynamic_flist = "";
	my $main_flist = "";

	my $dynamic_flist = << "END"
0 PIN_FLD_POID              POID [0]  0.0.0.1 /group/billing $group_obj 0
END
;	
	$main_flist .= $dynamic_flist;

	foreach $poid (@account_poids) {
	$dynamic_flist = << "XXX"
0 PIN_FLD_MEMBERS      ARRAY [$i]  allocated 20, used 1
1	PIN_FLD_OBJECT	POID [0] 0.0.0.1 /account $poid 0
XXX
;
	$main_flist .= $dynamic_flist;
	$i++;
	}
	print "main_flist for $group_obj:-\n$main_flist\n";
	
	#------------------------------------------------------------------------------------------------
	#Converting flist from String to flist format(pin_perl_str_to_flist)
	#------------------------------------------------------------------------------------------------
	$in_flistp = pcmif::pin_perl_str_to_flist($main_flist, $db_no, $ebufp);
	  
	#------------------------------------------------------------------------------------------------
	#calling the opcode PCM_OP_SEARCH using pcmif::pcm_perl_op function
	#------------------------------------------------------------------------------------------------
	$out_flistp = pcmif::pcm_perl_op($pcm_ctxp, "PCM_OP_BILL_GROUP_ADD_MEMBER", 0, $in_flistp, $ebufp);

	#------------------------------------------------------------------------------------------------
	# Checking for Error 
	# if any error print error in error log 
	#------------------------------------------------------------------------------------------------
	if (pcmif::pcm_perl_is_err($ebufp)) {
		print $sf "\nError for group_obj $group_obj";
		print $ef "\nError in the Opcode PCM_OP_BILL_GROUP_ADD_MEMBER for group_obj $group_obj \n$ebufp\n";
		#exit(1);
	}
	else {
		print $sf "\nSuccess for group_obj $group_obj";
	}
}
