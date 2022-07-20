/*******************************************************************
 * File:	fm_mso_cust_bb_hw_amc.c
 * Opcode:	MSO_OP_CUST_BB_HW_AMC 
 * Created:	26-AUG-2014
 * Description: This opcode is invoked for applying HW AMC charges 
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_bb_hw_amc.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "pin_cust.h"
#include "ops/act.h"
#include "ops/cust.h"
#include "ops/bal.h"
#include "ops/bill.h"
#include "pin_bill.h"
#include "fm_bal.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "mso_cust.h"
#include "mso_prov.h"
#include "mso_device.h"


/*******************************************************************
 * MSO_OP_CUST_BB_HW_AMC 
 *
 * This policy provides logic to apply AMC charges
 * If any new rental priorities comes add in this function fm_mso_cust_bb_hw_amc_get_mso_details(ctxp, 0,i_flistp, &m_rflistp, ebufp);
 * There is a technology check for the AMC where you need to add an entry 
 *******************************************************************/

/* main functions */
EXPORT_OP void
op_mso_cust_bb_hw_amc(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_bb_hw_amc(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

/* END main functions */

/* action functions */

static void
fm_mso_cust_bb_hw_amc(
        pcm_context_t           *ctxp,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_mso_cust_bb_hw_amc_search_device(
	pcm_context_t	*ctxp,
	pin_flist_t	 *i_flistp,
	 pin_flist_t    **r_flistpp,
	pin_errbuf_t	*ebufp);

static void
fm_mso_cust_bb_hw_amc_get_mso_details(
        pcm_context_t   *ctxp,
	int32		flag,
        pin_flist_t      *i_flistp,
         pin_flist_t    **r_flistpp,
        pin_errbuf_t    *ebufp);

static void
fm_mso_cust_bb_hw_amc_process(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_mso_cust_bb_hw_amc_search_product(
        pcm_context_t   *ctxp,
        pin_flist_t      *i_flistp,
        pin_flist_t    **r_flistpp,
        pin_errbuf_t    *ebufp);

extern int32
get_diff_time(
        pcm_context_t   *ctxp,
        time_t          timeStamp,
        time_t          timeStamp2,
        pin_errbuf_t    *ebufp);

static void
fm_mso_cust_bb_hw_amc_process_actions(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp);

static void
fm_mso_cust_bb_hw_amc_plan_deal_product(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp);

static void
fm_mso_cust_bb_hw_amc_cancel_deal(
        pcm_context_t   *ctxp,
        pin_flist_t      *i_flistp,
        pin_flist_t    **r_flistpp,
        pin_errbuf_t    *ebufp);

static void
fm_mso_cust_bb_hw_amc_purchase_deal(
        pcm_context_t   *ctxp,
        pin_flist_t      *i_flistp,
        pin_flist_t    **r_flistpp,
        pin_errbuf_t    *ebufp);

static void
fm_mso_cust_bb_hw_amc_get_sub_plan_details(
        pcm_context_t   *ctxp,
        pin_flist_t      *i_flistp,
         pin_flist_t    **r_flistpp,
        pin_errbuf_t    *ebufp);

static void
fm_mso_cust_bb_hw_amc_search_service(
        pcm_context_t   *ctxp,
        pin_flist_t      *i_flistp,
        pin_flist_t    **r_flistpp,
        pin_errbuf_t    *ebufp);

static int
fm_mso_cust_bb_hw_amc_search_exclusion(
        pcm_context_t   *ctxp,
        pin_flist_t      *i_flistp,
        pin_errbuf_t    *ebufp);

static void
fm_mso_cust_bb_hw_amc_parse_input(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp);

extern void
fm_mso_cust_bb_hw_amc_get_cycle_details(
        pcm_context_t   *ctxp,
        pin_flist_t      *i_flistp,
        pin_errbuf_t    *ebufp);

static int32 
get_bill_when_from_service(pcm_context_t          *ctxp,
                                 poid_t                 *svc_pdp,
                                 pin_errbuf_t            *ebufp);
static void 
set_bill_when_for_service(pcm_context_t          *ctxp,
                                 poid_t            *svc_pdp,
				 int32		  bill_when,
                                 pin_errbuf_t      *ebufp);

static  int32
fm_mso_cust_bb_hw_amc_get_old_bill_when(
        pcm_context_t   *ctxp,
        poid_t      *a_pdp,
	poid_t      *p_pdp,
        pin_errbuf_t    *ebufp);

static int 
fm_mso_cust_bb_hw_create_deferred_action(
        pcm_context_t   *ctxp,
	poid_t		*a_pdp,
	poid_t		*s_pdp,
	poid_t		*u_pdp,
	int64		diff,
        pin_errbuf_t    *ebufp);

extern void
fm_mso_cust_bb_hw_delete_deferred_actions(
        pcm_context_t   *ctxp,
        poid_t			*a_pdp,
        pin_errbuf_t    *ebufp);

extern void 
	fm_mso_search_plan_details(
        pcm_context_t                   *ctxp,
        pin_flist_t                     *i_flistp,
        pin_flist_t                     **o_flistpp,
        pin_errbuf_t                    *ebufp);

static int32 
fm_mso_get_active_amc_prod(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
	pin_flist_t		**ret_flist,
        pin_errbuf_t            *ebufp);

extern void
fm_mso_generate_credit_charge(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        poid_t                  *cancel_offer_obj,
        poid_t                  *cancel_prod_obj,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);


/* END action functions */

/*******************************************************************
 * MSO_OP_CUST_BB_HW_AMC  
 *******************************************************************/
void 
op_mso_cust_bb_hw_amc(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	int			*status = NULL;
	int32			*renew_flag = NULL;
	char            	*prog_name = NULL;
	void			*vp = NULL;
	pin_flist_t		*o_flistp = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_BB_HW_AMC) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_bb_hw_amc error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_bb_hw_amc input flist", i_flistp);
	
	fm_mso_cust_bb_hw_amc(ctxp, flags, i_flistp, &o_flistp, ebufp);

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " AMC FUNCTION completed ");

 	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"op_mso_cust_bb_hw_amc Failed input flist", i_flistp);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_bb_hw_amc error", ebufp);
        PIN_ERRBUF_RESET(ebufp);
		*r_flistpp = i_flistp;
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_mso_cust_bb_hw_amc result flist", o_flistp);
		*r_flistpp = o_flistp;
	}
	return;
}

/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void
fm_mso_cust_bb_hw_amc(
        pcm_context_t           *ctxp,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)

{

	pin_flist_t     *d_rflistp = NULL;
	pin_flist_t     *m_rflistp = NULL;
        pin_flist_t     *p_rflistp = NULL;
	pin_flist_t	*result_plan_flistp = NULL;
	int         	delem_id = 0;
	pin_cookie_t    dcookie = NULL;
        int             melem_id = 0;
        pin_cookie_t    mcookie = NULL;
	pin_flist_t	*dresults_flistp = NULL;
	pin_flist_t     *mresults_flistp = NULL;
	poid_t		*hardware_mso_planp = NULL;
	pin_decimal_t   *priority = NULL;
	double           prod_priority_double = 0;
	int32            *action_flags = NULL;
	poid_t		*user_pdp = NULL;
	int32		*p_status = NULL;
	poid_t          *device_obj = NULL;
	poid_t		*plan_pdp = NULL;
	time_t		*subs_endt = NULL;
	char		*eve_type = NULL;
	pin_flist_t     *s_rflistp = NULL;
	int		pre = 2;
	int		post =1;
	int		*indp = NULL;
	int		*mode = NULL;
	pin_flist_t	*copy_iflistp = NULL;
	int             felem_id = 0;
	pin_cookie_t    fcookie = NULL;
	pin_flist_t	*final_rflistp = NULL;
	int32		count = 0;
	char		*amc_descr = "AMC Completed";
	pin_flist_t     *tmp_flistp = NULL;
	pin_flist_t	*read_inp_flistp = NULL;
	pin_flist_t	*read_ret_flisp = NULL;
	pin_flist_t	*credit_flistp = NULL;
	pin_flist_t	*arg_flistp = NULL;
	char		*plan_name = NULL;
	int		bill_when=0;
	pin_flist_t	*read_acc_flistp = NULL;
	pin_flist_t	*read_ret_flistp = NULL;
	char		*acc_city = NULL;
	pin_flist_t	*ret_flistpp = NULL;
	pin_decimal_t	*cyc_fee = NULL;	
	int32		found_device = 0;
	int32		plan_found = 0;
	int32		customer_device = 0;	
	int32		*tech = NULL;
	poid_t		*serv_pdp = NULL;
	int		*plan_type = NULL;
	pin_flist_t	*cancel_iflistp = NULL;
	pin_flist_t	*cancel_rflistp = NULL;
	poid_t		*acc_pdp = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
			"fm_mso_cust_bb_hw_amc input flist", i_flistp);
	copy_iflistp = PIN_FLIST_COPY(i_flistp, ebufp);
	action_flags = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 0, ebufp);
	mode = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_MODE, 0, ebufp);
	acc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);	

	fm_mso_cust_bb_hw_amc_parse_input(ctxp, i_flistp, &s_rflistp, ebufp);
	subs_endt = PIN_FLIST_FLD_GET(s_rflistp, PIN_FLD_CYCLE_END_T, 0, ebufp);
	plan_pdp = PIN_FLIST_FLD_GET(s_rflistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
	eve_type = PIN_FLIST_FLD_GET(s_rflistp, PIN_FLD_EVENT_TYPE, 0, ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,"amc_parse_input ERROR ", i_flistp);
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Mandatory fm_mso_cust_bb_hw_amc_parse_input input Missing", ebufp);
            PIN_ERRBUF_RESET(ebufp);
            *r_flistpp = copy_iflistp;
            return;
        }


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_mso_cust_bb_hw_amc PROCESS INPUT RESULT", s_rflistp);
	/**condition not to charge the amc for modem swap return and allow Rental to Outright to remove the current AMC deal if any avaiable***/
	if(action_flags && (*action_flags == AMC_ON_PREPAID_SWAP_O_R ||  *action_flags == AMC_ON_PREPAID_SWAP_R_R || *action_flags == AMC_ON_POSTPAID_SWAP_O_R
		|| *action_flags == AMC_ON_POSTPAID_SWAP_R_R))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Swap modem flow other than rental to outright", ebufp);
		*r_flistpp = copy_iflistp;
		return;
	}

	if(action_flags)
	{
		switch(*action_flags) 
		{
			case AMC_ON_PREPAID_TOPUP : 
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "AMC_ON_PREPAID_TOPUP");
				PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_INDICATOR, &pre, ebufp);
				break;
			case AMC_ON_PREPAID_RENEWAL :
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "AMC_ON_PREPAID_RENEWAL");
				PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_INDICATOR, &pre, ebufp);
				break;	
			case AMC_ON_PREPAID_SWAP_R_O :
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "AMC_ON_PREPAID_SWAP_R_O");
				PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_INDICATOR, &pre, ebufp);
				break;
			case AMC_ON_PREPAID_SWAP_O_R :
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "AMC_ON_PREPAID_SWAP_O_R");
				PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_INDICATOR, &pre, ebufp);
				break;
			case AMC_ON_PREPAID_SWAP_R_R :
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "AMC_ON_PREPAID_SWAP_R_R");
				PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_INDICATOR, &pre, ebufp);
				break;
			case AMC_ON_PREPAID_TERMINATION :
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "AMC_ON_PREPAID_TERMINATION");
				PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_INDICATOR, &pre, ebufp);
				break;
			case AMC_ON_POSTPAID_HW_PURCHASE :
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "AMC_ON_POSTPAID_HW_PURCHASE");
			 	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_INDICATOR, &post, ebufp);		
				break;
			case AMC_ON_POSTPAID_SWAP_R_O :
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "AMC_ON_POSTPAID_SWAP_R_O");
				PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_INDICATOR, &post, ebufp);
				break;
			case AMC_ON_POSTPAID_SWAP_O_R :
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "AMC_ON_POSTPAID_SWAP_O_R");
				PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_INDICATOR, &post, ebufp);
				break;
			case AMC_ON_POSTPAID_SWAP_R_R :
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "AMC_ON_POSTPAID_SWAP_R_R");
				PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_INDICATOR, &post, ebufp);
				break;
			case AMC_ON_POSTPAID_TERMINATION :
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "AMC_ON_POSTPAID_TERMINATION");
				PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_INDICATOR, &post, ebufp);
				break;
                        case AMC_ON_CHANGE_PRE_PRE :
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "AMC_ON_CHANGE_PRE_PRE");
				PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_INDICATOR, &pre, ebufp);
                                break;
                        case AMC_ON_CHANGE_PRE_POST :
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "AMC_ON_CHANGE_PRE_POST");
				PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_INDICATOR, &post, ebufp);
                                break;
                        case AMC_ON_CHANGE_POST_PRE :
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "AMC_ON_CHANGE_POST_PRE");
				PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_INDICATOR, &pre, ebufp);
                                break;
                        case AMC_ON_CHANGE_POST_POST :
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "AMC_ON_CHANGE_POST_POST");
				PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_INDICATOR, &post, ebufp);
                                break;


			default :
					        
                			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                       					 "fm_mso_cust_bb_hw_amc_process error flist", i_flistp);
                			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
							"UNSUPPORTED FLAG PASSED", ebufp);
                			*r_flistpp = copy_iflistp;
        	}
	}

				
					
	 
	user_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 0, ebufp);
	indp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_INDICATOR, 0, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_mso_cust_bb_hw_amc flist", i_flistp);

                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Mandatory input parameters Missing", ebufp);
                *r_flistpp = copy_iflistp;
		return;
        }

	/***Technology check***/
	serv_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	if(serv_pdp != NULL)
	{
		fm_mso_get_poid_details(ctxp, serv_pdp, &read_ret_flisp, ebufp);	
		if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error after calling fm_mso_get_poid_details", ebufp);
                        return;
                }
		else
		{
			mresults_flistp = PIN_FLIST_SUBSTR_GET(read_ret_flisp, MSO_FLD_BB_INFO, 1, ebufp);	
			if(mresults_flistp)
			{
				tech = (int32 *)PIN_FLIST_FLD_GET(mresults_flistp, MSO_FLD_TECHNOLOGY, 1, ebufp);
				if(tech && *tech == FIBER_TECH || *tech == MSO_ETHERNET)
				{
			        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_WARNING,"No need to go for amc for this technologies");
                    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "DEBUG......");
					*r_flistpp = copy_iflistp;
				    PIN_FLIST_DESTROY_EX(&read_ret_flisp, NULL);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        			"No need to go for amc for this technologies", i_flistp);	
                	return;
				}
                else
                {
				    PIN_FLIST_DESTROY_EX(&read_ret_flisp, NULL);
                }
			}
		}
	}
	/*Search Devices for the servie and read device plan */
	/**add changes for to check only modem device***/
	fm_mso_cust_bb_hw_amc_search_device(ctxp, i_flistp, &d_rflistp, ebufp);
	/* If no devices found just return */
	if(!PIN_FLIST_ELEM_COUNT(d_rflistp, PIN_FLD_RESULTS, ebufp))	
	{

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"No Modem Devices Found with Account/Service" , i_flistp);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"No Modem Devices Found with Account/Service", ebufp);

	}
	else
	{
		found_device = 1;
		dresults_flistp = PIN_FLIST_ELEM_GET(d_rflistp, PIN_FLD_RESULTS, 0, 1, ebufp);
		device_obj = PIN_FLIST_FLD_GET(dresults_flistp, PIN_FLD_POID, 0, ebufp);
		result_plan_flistp = PIN_FLIST_ELEM_GET(dresults_flistp, PIN_FLD_PLAN, 0, 1, ebufp);
		if(result_plan_flistp)
		{
			plan_type = PIN_FLIST_FLD_GET(result_plan_flistp, PIN_FLD_TYPE, 1, ebufp);
			if(plan_type && *plan_type == 3)
			{
				
				customer_device = 1;
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                	"This is a customer device No need to go for AMC" , i_flistp);
		                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_WARNING,"This is a customer device No need to go for AMC", ebufp);
				PIN_FLIST_DESTROY_EX(&d_rflistp, NULL);	
			}
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Contians Modem device");	
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"Output From Device Search", d_rflistp);

	/********************************************
	 * Go through Every Device and apply AMC Charges 
	 * Read /mso_purchased_plan from plan Array 
	 * get MSO_FLD_PURCHASED_PLAN_OBJ 
	 * *******************************************/

	final_rflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(final_rflistp, PIN_FLD_USERID, user_pdp, ebufp);
	PIN_FLIST_FLD_SET(final_rflistp, PIN_FLD_DESCR, amc_descr, ebufp);
	fm_mso_cust_bb_hw_amc_get_mso_details(ctxp, 0,i_flistp, &m_rflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"Output From MSO Search", m_rflistp);
	if(!PIN_FLIST_ELEM_COUNT(m_rflistp, PIN_FLD_RESULTS, ebufp))
	{

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"No Rental plan Found with Account/Service" , i_flistp);
	}
	else
	{
		plan_found = 1;
	}

	/**error handling**/
	if(found_device == 1 && plan_found == 1 && customer_device == 1)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Hardware plan attached to the Customer device");
		pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        	PIN_ERR_BAD_VALUE, PIN_FLD_PLAN, 0, 0);
		return;
	}

	else if(found_device == 0 && plan_found == 1)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Modem is missing for the Rental customer");
		pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                                PIN_ERR_BAD_VALUE, PIN_FLD_PLAN, 0, 0);
		return;
	}
	else if(found_device == 1 && plan_found == 0 && customer_device ==0)
	{
		fm_mso_cust_bb_hw_delete_deferred_actions(ctxp, acc_pdp, ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error after returning from function fm_mso_cust_bb_hw_delete_deferred_actions", ebufp);
                        return;
                }
		/**checking for any purchased_product active and cancel it before returning**/
                if(fm_mso_get_active_amc_prod(ctxp, i_flistp, &cancel_iflistp,ebufp))
                {
                        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, cancel_iflistp, PIN_FLD_USERID, ebufp );
                        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_FLAGS, cancel_iflistp, PIN_FLD_FLAGS, ebufp );
                        /**cancel the active and inactive purchased_products**/
                        fm_mso_cust_bb_hw_amc_cancel_deal(ctxp, cancel_iflistp, &cancel_rflistp, ebufp);
                        if (PIN_ERRBUF_IS_ERR(ebufp))
                        {
                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error after returning from function fm_mso_cust_bb_hw_amc_cancel_deal", ebufp);
                                PIN_FLIST_DESTROY_EX(&cancel_iflistp, NULL);
                                return;
                        }
                        PIN_FLIST_DESTROY_EX(&cancel_iflistp, NULL);
                        PIN_FLIST_DESTROY_EX(&cancel_rflistp, NULL);
                }
		if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error after returning from function fm_mso_get_active_amc_prod", ebufp);
                        return;
                }
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Rental plan doesn't exist for this customer");
                *r_flistpp = copy_iflistp;
                return;
	}
	if(found_device == 1 && plan_found == 0 && customer_device == 1)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"This is customer device no need to go from AMC" , i_flistp);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_WARNING, "This is customer device no need to go from AMC", ebufp);
		*r_flistpp = copy_iflistp;
		return;
	
	}

	/*If Hardware found */
	if(m_rflistp)
	{
		felem_id = 0;
		fcookie = NULL;
		p_rflistp = NULL;
		tmp_flistp = NULL;
		mresults_flistp = NULL;
		mresults_flistp = PIN_FLIST_ELEM_GET(m_rflistp, PIN_FLD_RESULTS,0,1,ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"Search HW with RENTAL" , mresults_flistp);

		/*getting plan name from readflds*/
		PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, mresults_flistp, PIN_FLD_POID, ebufp);
		read_inp_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(mresults_flistp, PIN_FLD_PLAN_OBJ, read_inp_flistp, PIN_FLD_POID, ebufp);
		
		PIN_FLIST_FLD_SET(read_inp_flistp,
					PIN_FLD_NAME, NULL, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read plan input", read_inp_flistp);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_inp_flistp, &read_ret_flisp, ebufp);
		
		plan_name = PIN_FLIST_FLD_GET(read_ret_flisp, PIN_FLD_NAME, 0, ebufp);
		PIN_FLIST_DESTROY_EX(&read_inp_flistp, NULL);

		 if (PIN_ERRBUF_IS_ERR(ebufp))
		 {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS of plan", ebufp);
			return;
		 }
		/*getting city name from acc_pdp*/
		read_acc_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(mresults_flistp, PIN_FLD_ACCOUNT_OBJ, read_acc_flistp, PIN_FLD_POID, ebufp);
		arg_flistp = PIN_FLIST_ELEM_ADD(read_acc_flistp, PIN_FLD_NAMEINFO, 1, ebufp);
		PIN_FLIST_FLD_SET(arg_flistp,
					PIN_FLD_CITY, NULL, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read acc input", read_acc_flistp);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_acc_flistp, &read_ret_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read acc return", read_ret_flistp);
		arg_flistp = PIN_FLIST_ELEM_GET(read_ret_flistp, PIN_FLD_NAMEINFO,PIN_ELEMID_ANY,0, ebufp);
		acc_city = PIN_FLIST_FLD_GET(arg_flistp, PIN_FLD_CITY, 0, ebufp);
		PIN_FLIST_DESTROY_EX(&read_inp_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&read_acc_flistp, NULL);

		 if (PIN_ERRBUF_IS_ERR(ebufp))
		 {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS of plan", ebufp);
			return;
		 }

		/*Preparing flist for passing to  price_calc function*/
		credit_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(mresults_flistp, PIN_FLD_ACCOUNT_OBJ, credit_flistp, PIN_FLD_POID, ebufp);
		arg_flistp = PIN_FLIST_ELEM_ADD(credit_flistp, PIN_FLD_PLAN, 0, ebufp);
		PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_PLAN_NAME, plan_name, ebufp);
		PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
		PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_BILL_WHEN, &bill_when, ebufp);
		PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_CITY, acc_city, ebufp);
		PIN_FLIST_FLD_SET(credit_flistp, PIN_FLD_ACTION, "AMC CALL", ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "credit_flistp", credit_flistp);
		/*calling function in price_calc*/	
		fm_mso_search_plan_details(ctxp, credit_flistp, &ret_flistpp, ebufp);

		PIN_FLIST_DESTROY_EX(&read_ret_flisp, NULL);
		PIN_FLIST_DESTROY_EX(&read_ret_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&credit_flistp, NULL);
		
		cyc_fee = PIN_FLIST_FLD_GET(ret_flistpp, PIN_FLD_CYCLE_FEE_AMT, 1, ebufp);
		if (cyc_fee && !pbo_decimal_is_zero(cyc_fee,ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
			    "AMC product is having cycle_fee so there is no cpe should charge");
			*r_flistpp = copy_iflistp;	
			return;    
		}
		PIN_FLIST_DESTROY_EX(&ret_flistpp, NULL);
		
		PIN_FLIST_FLD_SET(mresults_flistp, 
				PIN_FLD_FLAGS, action_flags, ebufp);
		PIN_FLIST_FLD_SET(mresults_flistp, PIN_FLD_USERID, user_pdp, ebufp);
		PIN_FLIST_FLD_SET(mresults_flistp,
				PIN_FLD_DEVICE_OBJ, device_obj, ebufp);
		PIN_FLIST_FLD_SET(mresults_flistp, 
				PIN_FLD_CYCLE_END_T, subs_endt, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USAGE_START_T, mresults_flistp, PIN_FLD_USAGE_START_T, ebufp);
		PIN_FLIST_FLD_SET(mresults_flistp, 
				PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
		PIN_FLIST_FLD_SET(mresults_flistp, 
				PIN_FLD_EVENT_TYPE, eve_type, ebufp);
		PIN_FLIST_FLD_SET(mresults_flistp, PIN_FLD_INDICATOR, indp ,ebufp);
		PIN_FLIST_FLD_SET(mresults_flistp, PIN_FLD_MODE, mode, ebufp);

		fm_mso_cust_bb_hw_amc_process
			(ctxp, mresults_flistp, &p_rflistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Got Result", p_rflistp);
		while ((tmp_flistp = PIN_FLIST_ELEM_GET_NEXT(p_rflistp, PIN_FLD_RESULTS,
						&felem_id, 1, &fcookie, ebufp)) != NULL)
		{
			if(tmp_flistp)
			{
				PIN_FLIST_ELEM_SET(final_rflistp, tmp_flistp, PIN_FLD_RESULTS, count, ebufp);
				count++;
			}
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Final Result", final_rflistp);
				
	}//if(m_rflistp)
	*r_flistpp = final_rflistp;
	PIN_FLIST_DESTROY_EX(&s_rflistp, NULL);	
	PIN_FLIST_DESTROY_EX(&copy_iflistp, NULL);

	
}

/**********************************************
 * Search Devices for the services 
 *********************************************/
static void
fm_mso_cust_bb_hw_amc_search_device(
        pcm_context_t   *ctxp,
        pin_flist_t      *i_flistp,
        pin_flist_t    **r_flistpp,
        pin_errbuf_t    *ebufp)
{
		
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*search_inflistp = NULL;
	pin_flist_t	*search_outflistp = NULL;
	pin_flist_t	*results_flistp = NULL;
	pin_flist_t	*services_flistp = NULL;
	pin_flist_t     *plan_flistp = NULL;
	
	char		*device_id = NULL;		
	char		search_template[100] = " select X from /device where F1 = V1 and F2 = V2 and F3.type = V3 and F4 != V4 ";
	int	    	search_flags = 768;
	int64		db = 1;
	int         	elem_id = 0;
	pin_cookie_t    cookie = NULL;
	int 		state_id = MSO_STB_STATE_ALLOCATED;
	int32		device_type = RENTAL_DEPOSIT_PLAN ;

	poid_t		*service_pdp = NULL;
	pin_flist_t     *s_flistp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
			"fm_mso_cust_bb_hw_amc_search_device input flist", i_flistp);

	service_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);

	/*************
	 * search flist to search device details based on service poid
	 ************/
	if(!service_pdp)
	{
		PIN_ERRBUF_CLEAR(ebufp);
		return ;
	}

	db = PIN_POID_GET_DB(service_pdp);
		
	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
	s_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_SERVICE_OBJ, service_pdp, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATE_ID, &state_id, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_PUT(args_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/device/modem", -1, ebufp), ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 4, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DEVICE_TYPE, OTT_MODEM, ebufp);

	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
			"search device input list", search_inflistp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                //On Error Return Cleanly
                PIN_ERRBUF_CLEAR(ebufp);

        }

	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		//On Error Return Cleanly
		PIN_ERRBUF_CLEAR(ebufp);
			
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
				"search device output flist", search_outflistp);
		*r_flistpp = search_outflistp;

	}
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	return;
	
}

/*************************************
 *Get mso_purchased_plan details 
*************************************/

static void
fm_mso_cust_bb_hw_amc_get_mso_details(
        pcm_context_t   *ctxp,
	int32		flag,
        pin_flist_t      *i_flistp,
         pin_flist_t    **r_flistpp,
        pin_errbuf_t    *ebufp)

{

        pin_flist_t     *args_flistp = NULL;
        pin_flist_t     *search_inflistp = NULL;
        pin_flist_t     *search_outflistp = NULL;
        pin_flist_t     *results_flistp = NULL;
        pin_flist_t     *services_flistp = NULL;

        char            *device_id = NULL;
        char            search_template[150] = " select X from /mso_purchased_plan 1,/purchased_product 2 where 1.F1 = V1 and (2.F2 = V2 or 2.F7 = V7) and 1.F3 = 2.F4 and (1.F5 = V5 or 1.F6 = V6) ";
        int             search_flags = 768;
        int64           db = 1;
      

        poid_t          *service_pdp = NULL;
	
        pin_flist_t     *s_flistp = NULL;
	pin_flist_t	*temp_flistp = NULL;
	pin_decimal_t	*pri_cm = pbo_decimal_from_str("700.0",ebufp);
	pin_decimal_t   *pri_dcm = pbo_decimal_from_str("770.0",ebufp);
	pin_decimal_t   *pri_dc2 = pbo_decimal_from_str("500.0",ebufp);
	pin_decimal_t   *pri_dc3 = pbo_decimal_from_str("570.0",ebufp);
	int32		act_status = 1;
	int32           inact_status = 2;
	poid_t		*acc_pdp = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
                
	PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_mso_cust_bb_hw_amc_get_mso_details input flist", i_flistp);

	acc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);	

        /*************
         * search flist to search device details based on service poid
         ************/
	db = PIN_POID_GET_DB(acc_pdp);

        search_inflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
        PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acc_pdp, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &act_status, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 3, ebufp);
	temp_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(temp_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, NULL, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);
	if(flag == 0)
	{
		args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 5, ebufp);
        	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRIORITY, pri_dc2, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 6, ebufp);
        	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRIORITY, pri_dc3, ebufp);
	}
	if(flag == 1)
        {
                args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 5, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRIORITY, pri_cm, ebufp);
                args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 6, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRIORITY, pri_dcm, ebufp);
        }
	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 7, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &inact_status, ebufp);
        results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "search mso_purchased_plan input list", search_inflistp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                //On Error Return Cleanly
                PIN_ERRBUF_CLEAR(ebufp);

        }

        PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                //On Error Return Cleanly
                PIN_ERRBUF_CLEAR(ebufp);

        }
        else
        {
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                "mso search output flist", search_outflistp);
                *r_flistpp = search_outflistp;

        }
        PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	pbo_decimal_destroy(&pri_dc2);
	pbo_decimal_destroy(&pri_dc3);
	pbo_decimal_destroy(&pri_cm);
	pbo_decimal_destroy(&pri_dcm);
        return;


}

/*************************************
 *Process each Hardware plan on device
*************************************/

static void
fm_mso_cust_bb_hw_amc_process(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
	char             msg[512] ;
	pin_decimal_t   *famountp = (pin_decimal_t *)NULL;
	pin_flist_t	*s_rflistp = NULL;
	pin_flist_t     *s_iflistp = NULL;
	pin_flist_t	*process_rflistp = NULL;
	poid_t		*a_pdp = NULL;
	pin_flist_t	*return_flist = NULL;
	pin_flist_t	*cancel_iflistp = NULL;
	pin_flist_t	*cancel_rflistp = NULL;	
	poid_t		*acc_pdp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_mso_cust_bb_hw_amc_process input flist", i_flistp);

	famountp = pbo_decimal_from_str("0", ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_mso_cust_bb_hw_amc_process flist", i_flistp);

                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Mandatory input Missing", ebufp);
                *r_flistpp = i_flistp;
		return;
        }

	
	/*get Subscription plan details */
	acc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	a_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
	s_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(s_iflistp, PIN_FLD_POID, a_pdp, ebufp);


        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Error Reading Subscription Details", i_flistp);

                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Inputs May be missing,Error Reading Subscription Details ", ebufp);
		PIN_FLIST_DESTROY_EX(&s_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&s_rflistp, NULL);
                *r_flistpp = i_flistp;
                return;
        }


	/*validate if this plan is in  exclusion list */
        if(fm_mso_cust_bb_hw_amc_search_exclusion(ctxp, s_iflistp, ebufp))
        {
                /*Plan is to be excluded 0 charges for AMC */
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "PLAN IS IN EXCLUSION LIST");
		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_AMOUNT, famountp, ebufp);
		/***added function to cancel schedule action during the amc exclusion check***/
		fm_mso_cust_bb_hw_delete_deferred_actions(ctxp, acc_pdp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error after returning from function fm_mso_cust_bb_hw_delete_deferred_actions", ebufp);
			return;
                }
		/**checking for any purchased_product active and cancel it before returning**/
		if(fm_mso_get_active_amc_prod(ctxp, i_flistp, &cancel_iflistp,ebufp))
		{
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, cancel_iflistp, PIN_FLD_USERID, ebufp );	
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_FLAGS, cancel_iflistp, PIN_FLD_FLAGS, ebufp );
			/**cancel the active and inactive purchased_products**/
			fm_mso_cust_bb_hw_amc_cancel_deal(ctxp, cancel_iflistp, &cancel_rflistp, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error after returning from function fm_mso_cust_bb_hw_amc_cancel_deal", ebufp);
				PIN_FLIST_DESTROY_EX(&cancel_iflistp, NULL);
				return;
			}	
			PIN_FLIST_DESTROY_EX(&cancel_iflistp, NULL);
			PIN_FLIST_DESTROY_EX(&cancel_rflistp, NULL);
		}
				
		if (PIN_ERRBUF_IS_ERR(ebufp))
        	{
                	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error after returning from function fm_mso_get_active_amc_prod", ebufp);
                	return;
        	}

		*r_flistpp = i_flistp;
		return;
	}

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Error Reading Subscription Details", i_flistp);

                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Inputs May be missing, Error in fm_mso_cust_bb_hw_amc_search_exclusion ", ebufp);
                PIN_FLIST_DESTROY_EX(&s_iflistp, NULL);
                PIN_FLIST_DESTROY_EX(&s_rflistp, NULL);
                *r_flistpp = i_flistp;
                return;
        }


	/*************************************************
	 *Call Main Function to Apply AMC Charges
	 ************************************************/
	fm_mso_cust_bb_hw_amc_process_actions(ctxp, i_flistp, &process_rflistp, ebufp);
			
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
               	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                    		"fm_mso_cust_bb_hw_amc_process flist", i_flistp);
               	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Failed in AMC MAIN PROCESS,Error in fm_mso_cust_bb_hw_amc_process_actions ", ebufp);
		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_AMOUNT, famountp, ebufp);
		*r_flistpp = i_flistp;
		return;
        }
	*r_flistpp = process_rflistp;

        PIN_FLIST_DESTROY_EX(&s_iflistp, NULL);
        PIN_FLIST_DESTROY_EX(&s_rflistp, NULL);
		pbo_decimal_destroy(&famountp);
	

}


/*************************************
 *search rental product details 
*************************************/
static void
fm_mso_cust_bb_hw_amc_search_product(
        pcm_context_t   *ctxp,
        pin_flist_t      *i_flistp,
        pin_flist_t    **r_flistpp,
        pin_errbuf_t    *ebufp)
{

        pin_flist_t     *args_flistp = NULL;
        pin_flist_t     *search_inflistp = NULL;
	pin_flist_t     *results_flistp = NULL;
        pin_flist_t     *search_outflistp = NULL;
        pin_flist_t     *services_flistp = NULL;

        char            search_template[200] = " select X from /purchased_product 1 , /product 2  where 1.F1 = 2.F2 and 1.F3 = V3 and 2.F4 = V4 ";
        int             search_flags = 768;
        int64           db = 1;
        int             elem_id = 0;
        pin_cookie_t    cookie = NULL;
	pin_decimal_t   *priority = NULL;
        

        poid_t          *a_pdp = NULL;
	poid_t          *p_pdp = NULL;
        pin_flist_t     *s_flistp = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
		PIN_ERRBUF_CLEAR(ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_mso_cust_bb_hw_amc_search_product input flist", i_flistp);

        a_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	priority = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PRIORITY, 0, ebufp);
		
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
               PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Mandatory Fields Missing", ebufp);
		return;
	}

        /*************
         * search flist to search device details based on service poid
         ************/
        db = PIN_POID_GET_DB(a_pdp);

        search_inflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
        PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);
		
        args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
		
        args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);
		
	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, a_pdp, ebufp);
		
	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 4, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRIORITY, priority, ebufp);
		
        results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "search product input list", search_inflistp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                //On Error Return Cleanly
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "search rental product input list", i_flistp);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error Preparing rental product Input for Search", ebufp);
		PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
		return;
				
	}

        PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
		
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                //On Error 
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "search input list", search_inflistp);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error calling Search in rental product", ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
				
				
	}
        else
        {
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                "search rental product output flist", search_outflistp);
		*r_flistpp = PIN_FLIST_COPY(search_outflistp, ebufp);	

        }
        PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
        return;
}

/***********************************************
 * MAIN Function for ALL AMC ACTIONS 
 **********************************************/
static void
fm_mso_cust_bb_hw_amc_process_actions(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp)
{
	
	int32		bill_when = 0;
	pin_flist_t     *results_flistp = NULL;
	pin_flist_t     *ppd_rflistp = NULL;
	pin_flist_t     *p_rflistp = NULL;
	pin_flist_t	*preturn_flistp = NULL;
	pin_flist_t     *pur_iflistp = NULL;
	int             elem_id = 0;
	pin_cookie_t    cookie = NULL;
	poid_t		*product_pdp = NULL;
	poid_t          *plan_pdp = NULL;
	poid_t          *deal_pdp = NULL;
	
	char 		*eve_type = NULL;
	char		a_eve_type[128] = "" ;
	pin_flist_t     *p_iflistp = NULL;
	pin_decimal_t   *amc_priority = (pin_decimal_t *)NULL;
	pin_decimal_t   *priority = NULL;
	pin_flist_t     *cancel_iflistp = NULL;
	pin_flist_t     *cancel_rflistp = NULL;
	pin_flist_t     *purchase_iflistp = NULL;
	pin_flist_t     *purchase_rflistp = NULL;
	poid_t          *user_pdp = NULL;
	time_t		*amc_charged_to_tp = NULL;
	time_t		amc_max_charged_to = 0 ;
	int             amc_elem_id = 0;
	pin_cookie_t    amc_cookie = NULL;
	pin_flist_t     *cycle_flistp = NULL;
	pin_flist_t     *c_flistp = NULL;
        int             celem_id = 0;
        pin_cookie_t    ccookie = NULL;
	int32		plan_flag = 0;
	poid_t          *service_pdp = NULL;
	poid_t          *a_pdp = NULL;
	char            *plan_code = NULL;
	char            *plan_name = NULL;
	poid_t          *device_objp = NULL;
	int32		*action_flags = NULL;
	char		*in_eve_type = NULL;
	time_t		*cycle_end_t = NULL;
	time_t		*cycle_start_t = NULL;
	int		*indp = NULL;
	char		*ind_str = NULL;
	int		*mode = NULL;
	char		*device_type = NULL;
	char		*device_search = NULL;
	time_t           current_t = 0;
        time_t          *eff_t = NULL;
        int64            diff = 0;
	char		msg[500];
	


        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_mso_cust_bb_hw_amc_process_actions input ", i_flistp);
	user_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 0, ebufp);
	service_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
	a_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	action_flags = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 0, ebufp);
	device_objp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEVICE_OBJ, 0, ebufp);
	priority = (pin_decimal_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PRIORITY, 0, ebufp);
	in_eve_type = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_EVENT_TYPE, 1, ebufp);
	cycle_end_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CYCLE_END_T, 0, ebufp);
	cycle_start_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USAGE_START_T, 1, ebufp);
	indp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_INDICATOR, 0, ebufp);
	mode = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_MODE, 0, ebufp);
	eff_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_EFFECTIVE_T, 0, ebufp);

	/* Calculate HW Activation Date for Specific cases ************ 
	 * Need to verify whether HW activation Date is passed 1 year */

	current_t = pin_virtual_time(NULL);
        current_t = fm_utils_time_round_to_midnight(current_t);
        diff = get_diff_time( ctxp, *eff_t, current_t, ebufp);
        sprintf(msg,"Current time is %d, HW Activation is on %d and diff is %d",current_t,*eff_t,diff);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);	

	

	/* based on the call to AMC decide PRE or POSTPAID */
	if(indp && *indp == 1)
		ind_str = "post";
	else
		ind_str = "pre";

	/*****************************************
	 * Calcualte AMC Priority 		 *
	 ****************************************/

        amc_priority  = pbo_decimal_from_str(AMC_PRIORITY_DIFF, ebufp);
        pbo_decimal_add_assign(amc_priority, priority ,ebufp);

        if  (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                "Error in fm_mso_cust_bb_hw_amc_process_actions", i_flistp);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "ERROR getting Initial Data, Error in fm_mso_cust_bb_hw_amc_process_actions",  ebufp);
		*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);	
                goto CLEANUP;
        }

	
	/*******************************************
	 *based on even set the event type and bill_when 
	 ******************************************/

	if(in_eve_type && strstr(in_eve_type, "monthly"))
	{
		strcpy(a_eve_type, AMC_CYCLE_MONTHLY);
		bill_when = 1;
	}
	if(in_eve_type && strstr(in_eve_type, "bimonthly"))
	{
		strcpy(a_eve_type, AMC_CYCLE_BMONTHLY);
		bill_when = 2;
	}
	if(in_eve_type && strstr(in_eve_type, "quarterly"))
	{
		strcpy(a_eve_type, AMC_CYCLE_QUARTERLY);
		bill_when = 3;
	}
	if(in_eve_type && strstr(in_eve_type, "quadmonthly"))
	{
		strcpy(a_eve_type, AMC_CYCLE_QMONTHLY);
		bill_when = 4;
	}
	if(in_eve_type && strstr(in_eve_type, "semiannual"))
	{
		strcpy(a_eve_type, AMC_CYCLE_SEMIANNUAL);
		bill_when = 6;
	}
	if(in_eve_type && strstr(in_eve_type, "annual") && !strstr(in_eve_type, "semiannual"))
	{
		strcpy(a_eve_type, AMC_CYCLE_ANNUAL);
		bill_when = 12;
	}
	/**adding new event 9 months plan***/
	if(in_eve_type && strstr(in_eve_type, "nine_months"))
        {
                strcpy(a_eve_type, AMC_CYCLE_NINE_MONTHS);
                bill_when = 9;
        }
	if(in_eve_type && strstr(in_eve_type, "thirty_days"))
	{
		strcpy(a_eve_type, "cycle_forward_thirty_days");
		bill_when = 1;
	}

	/*************************************************
	 * Find RENTAL TYPE HARDWARE PRODUCT 		 *
	 * FROM /purchased_product                       *
	 * **********************************************/

	fm_mso_cust_bb_hw_amc_plan_deal_product
		(ctxp, i_flistp, &ppd_rflistp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "AMC PLAN DEAL PRODUCT EVENT ", ppd_rflistp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                "Error in fetching AMC Plans ", i_flistp);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Looks like No AMC Plans Exist in DB",  ebufp);
		*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
                goto CLEANUP;
        }



	/***************************************************************
	 *Get the product Details that matches the PAY TERM of Customer*
	 *At any point of time only one plan for pay term	       *
	 **************************************************************/ 
	
	while ((results_flistp =   // Loop through each AMC product   
                 PIN_FLIST_ELEM_GET_NEXT(ppd_rflistp, PIN_FLD_RESULTS,
                                        &elem_id, 1, &cookie, ebufp)) != NULL)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"Process AMC PLAN Results " , results_flistp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, a_eve_type);
		product_pdp =  PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_POID, 0, ebufp);
		plan_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
		deal_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_DEAL_OBJ, 0, ebufp);
		eve_type = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_EVENT_TYPE, 1, ebufp);
		plan_code = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_NAME, 0, ebufp);
		plan_name = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_CODE, 0, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, eve_type);
		if(eve_type && a_eve_type && strstr(eve_type, a_eve_type) 
					&& strstr(eve_type, ind_str) )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "AMC PRODUCT EVENT MAPPING MATCHED");
			plan_flag = 1;
			break ;
		}
	}

        if (PIN_ERRBUF_IS_ERR(ebufp) || plan_flag == 0)
        {
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                "No AMC Plan exists for this Customer Setup", i_flistp);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "No Matching AMC Plans FOUND",  ebufp);
		*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
		goto CLEANUP;
        }

         	
        /****************************************************
	 * See IF AMC Product Exists on Customer Already    * 
	 ***************************************************/

	p_iflistp = PIN_FLIST_COPY(i_flistp, ebufp);
	
	
	PIN_FLIST_FLD_SET(p_iflistp, PIN_FLD_PRIORITY, amc_priority, ebufp);

        /**********************************************
         * CANCEL Existing AMC Plans           
	 * AMC_ON_PREPAID_TOPUP or similar actions
         *********************************************/

  	/*delete deferred action if any */
        if(action_flags && *action_flags == AMC_ON_POSTPAID_SWAP_R_O ||
                       *action_flags == AMC_ON_POSTPAID_SWAP_R_R ||
                       *action_flags == AMC_ON_POSTPAID_TERMINATION ||
                       *action_flags == AMC_ON_CHANGE_POST_PRE ||
                       *action_flags == AMC_ON_CHANGE_POST_POST )
        {
                fm_mso_cust_bb_hw_delete_deferred_actions(ctxp, a_pdp, ebufp);
        }

	if(action_flags && *action_flags == AMC_ON_PREPAID_TOPUP ||
			   *action_flags == AMC_ON_PREPAID_TERMINATION ||
			   *action_flags == AMC_ON_PREPAID_RENEWAL ||
			   *action_flags == AMC_ON_PREPAID_SWAP_R_O ||
		           *action_flags == AMC_ON_PREPAID_SWAP_R_R ||
			   *action_flags == AMC_ON_POSTPAID_SWAP_R_O ||
			   *action_flags == AMC_ON_POSTPAID_SWAP_R_R || 
			   *action_flags == AMC_ON_POSTPAID_TERMINATION ||
			   *action_flags == AMC_ON_CHANGE_PRE_PRE ||
			   *action_flags == AMC_ON_CHANGE_PRE_POST ||
			   *action_flags == AMC_ON_CHANGE_POST_PRE ||
			   *action_flags == AMC_ON_CHANGE_POST_POST )
	{
		//fm_mso_cust_bb_hw_amc_search_product(ctxp, p_iflistp, &p_rflistp, ebufp);
		fm_mso_get_active_amc_prod(ctxp, p_iflistp, &p_rflistp,ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                "RESULT OF AMC PRODUCTS", p_rflistp);

		if(PIN_FLIST_ELEM_COUNT(p_rflistp, PIN_FLD_RESULTS, ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Found Existing AMC PLAN");
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"Existing AMC Plans are ", p_rflistp);
			cancel_iflistp = PIN_FLIST_COPY(p_rflistp, ebufp);

			/*NOW CANCEL  THE EXISTING AMC PLANS */
			PIN_FLIST_FLD_SET(cancel_iflistp, PIN_FLD_USERID, user_pdp, ebufp);
			PIN_FLIST_FLD_SET(cancel_iflistp, PIN_FLD_FLAGS, action_flags, ebufp); 

			fm_mso_cust_bb_hw_amc_cancel_deal(ctxp, cancel_iflistp, &cancel_rflistp, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"Output Flist from Cancel AMC Plans" , cancel_rflistp);
			if(cancel_rflistp)
				*r_flistpp = PIN_FLIST_COPY(cancel_rflistp, ebufp);		
			else
				*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);

			if (PIN_ERRBUF_IS_ERR(ebufp))
        		{
                		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                	"Error in Cancelling Existing AMC Plans", cancel_iflistp);
                		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Cancelling Existing AMC Plans",  ebufp);
				*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
                		goto CLEANUP;
        		}

		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "NO ACTIVE AMC PLAN ON CUSTOMER");
			*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
		}
	}


	/* Now call Again to get returnthe latest AMC product CHARGED END DATE */
	PIN_FLIST_DESTROY_EX(&p_rflistp, NULL);

	fm_mso_cust_bb_hw_amc_search_product(ctxp, p_iflistp, &p_rflistp, ebufp);


        if(PIN_FLIST_ELEM_COUNT(p_rflistp, PIN_FLD_RESULTS, ebufp))
        {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "SET LAST AMC PRODUCT CHARGED DATE");
	
		while ((cycle_flistp =   // Loop through each AMC product and capture maximum charged to of AMC product
                        PIN_FLIST_ELEM_GET_NEXT(p_rflistp, PIN_FLD_RESULTS,
                                        &celem_id, 1, &ccookie, ebufp)) != NULL)
                {
                        c_flistp = PIN_FLIST_ELEM_GET(cycle_flistp, PIN_FLD_CYCLE_FEES, PIN_ELEMID_ANY, 1, ebufp);
			if(c_flistp)
                        	amc_charged_to_tp = PIN_FLIST_FLD_GET(c_flistp, PIN_FLD_CHARGED_TO_T, 1 , ebufp);
                        if(amc_charged_to_tp)
                        {
                                if(*amc_charged_to_tp > amc_max_charged_to)
                                {

                                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                                "Capture the MAX AMC charged time", cycle_flistp);
                                        amc_max_charged_to = *amc_charged_to_tp;
                                }

                        }
                }
	}



	/*************************************
         * HW RENTAL FOUND
         * EXISTING AMC DEALS CANCELLED
         * NOW GO and Purchase AMC Deal
         ***********************************/

	if(action_flags && ( *action_flags == AMC_ON_PREPAID_TOPUP && (diff > AMC_HW_EFFECTIVE_DATE) )   ||
                           ( *action_flags == AMC_ON_PREPAID_RENEWAL && (diff > AMC_HW_EFFECTIVE_DATE) ) ||
			   ( *action_flags == AMC_ON_CHANGE_PRE_PRE  && (diff > AMC_HW_EFFECTIVE_DATE) ) ||   
			   ( *action_flags == AMC_ON_CHANGE_PRE_POST )||
			   ( *action_flags == AMC_ON_CHANGE_POST_PRE && (diff > AMC_HW_EFFECTIVE_DATE) ) ||
			   ( *action_flags == AMC_ON_POSTPAID_SWAP_O_R )||
			   ( *action_flags == AMC_ON_POSTPAID_SWAP_R_R )||
			   ( *action_flags == AMC_ON_PREPAID_SWAP_O_R && (diff > AMC_HW_EFFECTIVE_DATE) ) || 
		           ( *action_flags == AMC_ON_PREPAID_SWAP_R_R && (diff > AMC_HW_EFFECTIVE_DATE) ) || 		
			   ( *action_flags == AMC_ON_POSTPAID_HW_PURCHASE) ||
			   ( *action_flags == AMC_ON_CHANGE_POST_POST) )
			     
        {
	
		purchase_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(purchase_iflistp, PIN_FLD_POID, a_pdp, ebufp);
		PIN_FLIST_FLD_SET(purchase_iflistp, PIN_FLD_SERVICE_OBJ, service_pdp, ebufp);
		PIN_FLIST_FLD_SET(purchase_iflistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
		PIN_FLIST_FLD_SET(purchase_iflistp, PIN_FLD_DEAL_OBJ, deal_pdp, ebufp);
		PIN_FLIST_FLD_SET(purchase_iflistp, PIN_FLD_PRODUCT_OBJ, product_pdp, ebufp);
		PIN_FLIST_FLD_SET(purchase_iflistp, PIN_FLD_USERID, user_pdp, ebufp);
		PIN_FLIST_FLD_SET(purchase_iflistp, PIN_FLD_FLAGS, action_flags, ebufp);
		PIN_FLIST_FLD_SET(purchase_iflistp, PIN_FLD_CHARGED_TO_T, &amc_max_charged_to , ebufp);
		PIN_FLIST_FLD_SET(purchase_iflistp, PIN_FLD_NAME, plan_name, ebufp);
		PIN_FLIST_FLD_SET(purchase_iflistp, PIN_FLD_CODE, plan_code, ebufp);
		PIN_FLIST_FLD_SET(purchase_iflistp, PIN_FLD_BILL_WHEN, &bill_when, ebufp);
		PIN_FLIST_FLD_SET(purchase_iflistp, PIN_FLD_DEVICE_OBJ, device_objp, ebufp); 
		PIN_FLIST_FLD_SET(purchase_iflistp, PIN_FLD_CYCLE_END_T, cycle_end_t, ebufp);
        if (cycle_start_t)
        {
		    PIN_FLIST_FLD_SET(purchase_iflistp, PIN_FLD_USAGE_START_T, cycle_start_t, ebufp);
        }
		PIN_FLIST_FLD_SET(purchase_iflistp, PIN_FLD_MODE, mode, ebufp);
		PIN_FLIST_FLD_SET(purchase_iflistp, PIN_FLD_EFFECTIVE_T, eff_t, ebufp);

		fm_mso_cust_bb_hw_amc_purchase_deal(ctxp, purchase_iflistp, &purchase_rflistp, ebufp);

		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"Error prepare flist for purchase deal", purchase_iflistp);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "ERROR Calling amc_purchase_deal",ebufp);
			*r_flistpp = PIN_FLIST_COPY(purchase_iflistp, ebufp);
			goto CLEANUP;
		}
		*r_flistpp = PIN_FLIST_COPY(purchase_rflistp, ebufp);	
	}
	else
	{
		*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
	}

CLEANUP:
	PIN_FLIST_DESTROY_EX(&p_iflistp , NULL);
	PIN_FLIST_DESTROY_EX(&p_rflistp , NULL);
	PIN_FLIST_DESTROY_EX(&cancel_iflistp , NULL);
	PIN_FLIST_DESTROY_EX(&cancel_rflistp , NULL);
	PIN_FLIST_DESTROY_EX(&ppd_rflistp , NULL);
	PIN_FLIST_DESTROY_EX(&purchase_iflistp , NULL);
	PIN_FLIST_DESTROY_EX(&purchase_rflistp , NULL);
	return;
	pbo_decimal_destroy(&amc_priority);
}

/***********************************************
 * GET DETAILS OF AMC PRODUCT  DEAL PLAN       *
 **********************************************/

static void
fm_mso_cust_bb_hw_amc_plan_deal_product(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp)

{

        pin_flist_t     *args_flistp = NULL;
        pin_flist_t     *search_inflistp = NULL;
        pin_flist_t     *results_flistp = NULL;
	pin_flist_t     *presults_flistp = NULL;
        pin_flist_t     *search_outflistp = NULL;
	pin_flist_t     *psearch_outflistp = NULL;
        pin_flist_t     *services_flistp = NULL;
	pin_flist_t     *deal_flistp = NULL;
	pin_flist_t	*final_rflistp = NULL;
	pin_flist_t     *final_tflistp = NULL;
	pin_flist_t     *u_flistp = NULL;
	char		*eve_type = NULL;

        char            search_template[200] = " select X from /product where F1 = V1 ";
	char psearch_template[200] = " select X from /plan 1 , /deal 2 , /product 3  where 1.F1 = 2.F2 and 2.F3 = 3.F4 and 3.F4 = V4 ";

		
        int             search_flags = 768;
        int64           db = 1;
        int             elem_id = 0;
        pin_cookie_t    cookie = NULL;
	int             pelem_id = 0;
	pin_cookie_t    pcookie = NULL;
        pin_decimal_t   *priority = NULL;
	pin_decimal_t   *amc_priority = (pin_decimal_t *)NULL;
	pin_decimal_t   *p_diff = (pin_decimal_t *)NULL;
	poid_t          *service_pdp = NULL;
	poid_t          *product_pdp = NULL;
	poid_t          *plan_pdp = NULL;
	poid_t          *deal_pdp = NULL;
	char		*p_name = NULL;
	char		*p_code = NULL;
		

		
        poid_t          *a_pdp = NULL;
        poid_t          *p_pdp = NULL;
        pin_flist_t     *s_flistp = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_mso_cust_bb_hw_amc_plan_deal_product input flist", i_flistp);
						
	service_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
	db = PIN_POID_GET_DB(service_pdp);

        priority = (pin_decimal_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PRIORITY, 0, ebufp);
	amc_priority  = pbo_decimal_from_str(AMC_PRIORITY_DIFF, ebufp);
	pbo_decimal_add_assign(amc_priority, priority ,ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
               PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Mandatory Fields Missing", ebufp);
		*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);	
                return;
        }

        /*************
        * search flist to search device details based on service poid
        ************/
        db = PIN_POID_GET_DB(service_pdp);

        search_inflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
        PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRIORITY, amc_priority, ebufp);

        results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "search product input list", search_inflistp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                //On Error Return Cleanly
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "search product input list", i_flistp);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error Preparing Input for Search device details", ebufp);
                PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
		*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
                return;

        }

        PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                //On Error
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "search input list", search_inflistp);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error calling Search for device details", ebufp);
                PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
		*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
		return;
				
	}
		
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "search AMC product output flist", search_outflistp);

	/* Capture results in this flist */
	final_rflistp = PIN_FLIST_CREATE(ebufp);

        while ((results_flistp =   // Loop through each AMC product and capture results 
                 PIN_FLIST_ELEM_GET_NEXT(search_outflistp, PIN_FLD_RESULTS,
                                        &elem_id, 1, &cookie, ebufp)) != NULL)
        {
                product_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_POID, 0, ebufp);
		u_flistp = PIN_FLIST_ELEM_GET(results_flistp, PIN_FLD_USAGE_MAP, PIN_ELEMID_ANY, 0, ebufp);
		eve_type = PIN_FLIST_FLD_GET(u_flistp, PIN_FLD_EVENT_TYPE, 1 , ebufp);
		/* set results 
		final_tflistp = PIN_FLIST_ELEM_ADD(final_rflistp, PIN_FLD_RESULTS, elem_id, ebufp);
		PIN_FLIST_FLD_SET(final_tflistp, PIN_FLD_POID, product_pdp, ebufp); 
		if(eve_type)
			PIN_FLIST_FLD_SET(final_tflistp, PIN_FLD_EVENT_TYPE, eve_type, ebufp); */
		
		if(!eve_type || !product_pdp || PIN_ERRBUF_IS_ERR(ebufp))
		{
         		//On Error
          		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                      		"search input list", search_inflistp);
           		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error calling AMC product Search", ebufp);
                	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
			PIN_FLIST_DESTROY_EX(&final_rflistp, NULL);
			*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
			return;
		}
		
		/* search for Plan and Deal */
		PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
		PIN_FLIST_DESTROY_EX(&psearch_outflistp, NULL);
		
        	pelem_id = 0;
        	pcookie = NULL;
		
		
		search_inflistp = PIN_FLIST_CREATE(ebufp);
        	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
        	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, psearch_template, ebufp);
        	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
		s_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_SERVICES, 1, ebufp);
        	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_DEAL_OBJ, NULL, ebufp);
		
		args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);
		
		args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 3, ebufp);
		s_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_PRODUCTS, 1, ebufp);
        	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 4, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, product_pdp, ebufp);
        	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                       "search plan input list", search_inflistp);

        	if (PIN_ERRBUF_IS_ERR(ebufp))
        	{
                	//On Error Return Cleanly
                	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        	"search AMC plan input list", search_inflistp);
                	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error Preparing Input for AMC plan Search", ebufp);
                	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
			PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
			PIN_FLIST_DESTROY_EX(&final_rflistp, NULL);
			*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
                	return;

        	}

        	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &psearch_outflistp, ebufp);

        	if (PIN_ERRBUF_IS_ERR(ebufp))
        	{
                	//On Error
                	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        	"search plan input list", search_inflistp);
                	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error calling Search", ebufp);
                	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
			PIN_FLIST_DESTROY_EX(&psearch_outflistp, NULL);
			PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
			PIN_FLIST_DESTROY_EX(&final_rflistp, NULL);	
			*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
			return;
				
		}
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                	"search AMC Plan output flist", psearch_outflistp);
	
        	while ((presults_flistp =  //Get plan and deal for the plan got above
                 	PIN_FLIST_ELEM_GET_NEXT(psearch_outflistp, PIN_FLD_RESULTS,
                                        &pelem_id, 1, &pcookie, ebufp)) != NULL)
        	{
                	plan_pdp = PIN_FLIST_FLD_GET(presults_flistp, PIN_FLD_POID, 0, ebufp);
			deal_flistp = PIN_FLIST_ELEM_GET(presults_flistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY, 1, ebufp);
			deal_pdp = PIN_FLIST_FLD_GET(deal_flistp, PIN_FLD_DEAL_OBJ, 0, ebufp);
			p_name = PIN_FLIST_FLD_GET(presults_flistp, PIN_FLD_NAME, 0, ebufp);
			p_code = PIN_FLIST_FLD_GET(presults_flistp, PIN_FLD_CODE, 0, ebufp);
                
		
			if (PIN_ERRBUF_IS_ERR(ebufp))
        		{
                		//On Error
                		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        		"search plan input list", search_inflistp);
                		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error calling Search", ebufp);
                		PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
				PIN_FLIST_DESTROY_EX(&psearch_outflistp, NULL);
				PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);	
				PIN_FLIST_DESTROY_EX(&final_rflistp, NULL);
				*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
				return;
			}

			/* set results */
                	final_tflistp = PIN_FLIST_ELEM_ADD(final_rflistp, PIN_FLD_RESULTS, elem_id, ebufp);
                	PIN_FLIST_FLD_SET(final_tflistp, PIN_FLD_POID, product_pdp, ebufp);

                	if(eve_type)
                        	PIN_FLIST_FLD_SET(final_tflistp, PIN_FLD_EVENT_TYPE, eve_type, ebufp);
			PIN_FLIST_FLD_SET(final_tflistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
			PIN_FLIST_FLD_SET(final_tflistp, PIN_FLD_DEAL_OBJ, deal_pdp, ebufp);
			PIN_FLIST_FLD_SET(final_tflistp, PIN_FLD_NAME, p_name, ebufp);
			PIN_FLIST_FLD_SET(final_tflistp, PIN_FLD_CODE, p_code, ebufp);
		}

	}
	*r_flistpp = PIN_FLIST_COPY(final_rflistp, ebufp);	

        PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
        return;
	pbo_decimal_destroy(&amc_priority);
}

/* Main Function that Cancel AMC deal */

static void
fm_mso_cust_bb_hw_amc_cancel_deal(
        pcm_context_t   *ctxp,
        pin_flist_t      *i_flistp,
        pin_flist_t    **r_flistpp,
        pin_errbuf_t    *ebufp)
{
	pin_flist_t    *results_flistp = NULL;
	int		elem_id = 0;
	pin_cookie_t    cookie = NULL;
	poid_t		*deal_pdp = NULL;
	poid_t          *plan_pdp = NULL;
	poid_t          *s_pdp = NULL;
	poid_t          *a_pdp = NULL;
	poid_t          *prod_pdp = NULL;
	pin_flist_t    *dinfo_iflistp = NULL;
	pin_flist_t    *dinfo_rflistp = NULL;
	pin_flist_t    *deposit_rflistp = NULL;
	pin_flist_t    *rental_rflistp = NULL;
	pin_flist_t    *temproduct_flistp = NULL;
	pin_flist_t     *ri_flistp = NULL;
	pin_flist_t     *charge_oflistp = NULL;
	int32		*p_statusp = NULL;
	pin_flist_t	*cancel_iflistp = NULL;
	pin_flist_t     *cancel_rflistp = NULL;
	pin_flist_t     *list_flistp = NULL;
	poid_t          *user_pdp = NULL;
	int		*pak_id = NULL;
	int             opcode = MSO_OP_CUST_BB_HW_AMC;
	int             belem_id = 0;
	pin_cookie_t    bcookie = NULL;
	pin_flist_t     *bal_flistp = NULL;
        int             *resouce_id = NULL;
        pin_decimal_t   *bal_impactp = NULL;
	pin_flist_t     *sub_balflistp = NULL;
	int32           *action_flags = NULL;
	int32           old_bill_when = 0;
	int32           new_bill_when = 0;
	int32		bill_when_changed = 0;
	int32		ref_flags = 1;
	poid_t          *pur_pdp = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_mso_cust_bb_hw_amc_cancel_deal input ", i_flistp);
	user_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 0, ebufp);
	action_flags = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 0, ebufp);
	

	while ((results_flistp =   // Loop through each AMC deal on customer and cancel 
                 PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_RESULTS,
                                        &elem_id, 1, &cookie, ebufp)) != NULL)
        {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"Results" , results_flistp);
		bill_when_changed = 0;
		deal_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_DEAL_OBJ, 0, ebufp);
		plan_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
		p_statusp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_STATUS, 0, ebufp);
		pak_id = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PACKAGE_ID, 0, ebufp);
		pur_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_POID, 0, ebufp);
		prod_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
		a_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
		s_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);

		/* DOn't Touch Already Cancelled products */
		if(*p_statusp && (*p_statusp == PIN_PRODUCT_STATUS_CANCELLED))
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"AMC DEAL ALREADY CANCELLED", results_flistp);
			continue;
		}
		
		
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                        "AMC CANCEL DEAL ERROR input flist", i_flistp);
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "TO cancel AMC, No Plan or Deal ", ebufp);
			return;
		}
		//Check if AMC charges to be refunded - AMC refundto be provided duringg only CHANGE PLAN for prepaid
		if(action_flags && *action_flags == AMC_ON_CHANGE_PRE_PRE ||
			   *action_flags == AMC_ON_CHANGE_PRE_POST )
		{
			ri_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ri_flistp, PIN_FLD_POID, a_pdp, ebufp);
			PIN_FLIST_FLD_SET(ri_flistp, PIN_FLD_SERVICE_OBJ, s_pdp, ebufp);
			PIN_FLIST_FLD_SET(ri_flistp, PIN_FLD_FLAGS, &ref_flags, ebufp);
			PIN_FLIST_FLD_SET(ri_flistp, PIN_FLD_OPCODE, &opcode, ebufp);
			fm_mso_generate_credit_charge(ctxp, ri_flistp, pur_pdp, prod_pdp, &charge_oflistp, ebufp);
			PIN_FLIST_DESTROY_EX(&ri_flistp, NULL);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_amc_change_plan prepaid refund flist", charge_oflistp);
			if  (PIN_ERRBUF_IS_ERR(ebufp)){
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in while proividing for disconnection credit", ebufp);
				PIN_FLIST_DESTROY_EX(&charge_oflistp, NULL);
				PIN_ERRBUF_CLEAR(ebufp);
				*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
				return;
			}
			PIN_FLIST_DESTROY_EX(&charge_oflistp, NULL);
		}

		dinfo_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(dinfo_iflistp, PIN_FLD_POID, deal_pdp, ebufp);
		temproduct_flistp = PIN_FLIST_ELEM_ADD(dinfo_iflistp, PIN_FLD_DEALS, 0, ebufp);
		PIN_FLIST_FLD_SET(temproduct_flistp, PIN_FLD_DEAL_OBJ, deal_pdp, ebufp);
		PIN_FLIST_FLD_SET(temproduct_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"Deal Info Input Flist" , dinfo_iflistp);
		/*call dealinfo function */
		fm_mso_get_deal_info(ctxp, dinfo_iflistp, &deposit_rflistp, &rental_rflistp, ebufp);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"Deal Details to Cancel", dinfo_iflistp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Deal Details to Cancel deposit", deposit_rflistp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Deal Details to Cancel rental", rental_rflistp);

		/*prepare input for CANCEL PLAN */
		cancel_iflistp = PIN_FLIST_CREATE(ebufp);
		a_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
		s_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
		PIN_FLIST_FLD_SET(cancel_iflistp, PIN_FLD_POID, a_pdp, ebufp);
		PIN_FLIST_FLD_SET(cancel_iflistp, PIN_FLD_SERVICE_OBJ, s_pdp, ebufp);
		list_flistp = PIN_FLIST_ELEM_ADD(cancel_iflistp, PIN_FLD_PLAN_LISTS, 0, ebufp);
		PIN_FLIST_FLD_SET(list_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
		PIN_FLIST_FLD_SET(list_flistp, PIN_FLD_PACKAGE_ID, pak_id, ebufp);
		PIN_FLIST_FLD_SET(cancel_iflistp, PIN_FLD_ACTION_NAME, "top_up_plan", ebufp);
		PIN_FLIST_FLD_SET(cancel_iflistp, PIN_FLD_PROGRAM_NAME, "top_up_plan", ebufp);
		PIN_FLIST_FLD_SET(cancel_iflistp, PIN_FLD_USERID, user_pdp, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                "Cancel Plan Input Flist " , cancel_iflistp);
			PIN_FLIST_DESTROY_EX(&dinfo_iflistp, NULL);					

		/* In case of change plan to post paid set corresponding bill_when */
		if(action_flags && *action_flags == AMC_ON_CHANGE_POST_PRE ||
                           *action_flags == AMC_ON_CHANGE_POST_POST )
		{

			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "BILL_WHEN HANDLING STARTED");
			old_bill_when = fm_mso_cust_bb_hw_amc_get_old_bill_when(ctxp, a_pdp, pur_pdp, ebufp);
			if(old_bill_when != 0)
			{
				new_bill_when = get_bill_when_from_service(ctxp, s_pdp, ebufp);
				if(new_bill_when != old_bill_when)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "GOT DIFFERENT BILL_WHEN");
					set_bill_when_for_service(ctxp, s_pdp, old_bill_when, ebufp);
					bill_when_changed = 1;
				}
			}
		}

		PCM_OP(ctxp, MSO_OP_CUST_CANCEL_PLAN, 0, cancel_iflistp, &cancel_rflistp, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"Cancel Plan Result " , cancel_rflistp);
                if(PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                        "AMC CANCEL DEAL ERROR input flist", i_flistp);
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "TO cancel AMC, No Plan or Deal ", ebufp);
			PIN_FLIST_DESTROY_EX(&cancel_rflistp, ebufp);
			PIN_FLIST_DESTROY_EX(&deposit_rflistp, ebufp);
			PIN_FLIST_DESTROY_EX(&rental_rflistp, ebufp);
			*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
			return;
                }
	
		/* set back the original bill_when */
		if(bill_when_changed == 1)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "BILL_WHEN RESET TO NEW");
			set_bill_when_for_service(ctxp, s_pdp, new_bill_when, ebufp);
		}

		PIN_FLIST_DESTROY_EX(&cancel_iflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&deposit_rflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&rental_rflistp, ebufp);


	}
	PIN_FLIST_DESTROY_EX(&deposit_rflistp, NULL);
	PIN_FLIST_DESTROY_EX(&rental_rflistp, NULL);
	PIN_FLIST_DESTROY_EX(&cancel_iflistp, NULL);
	if(cancel_rflistp && cancel_rflistp != NULL) 
		*r_flistpp = PIN_FLIST_COPY(cancel_rflistp, ebufp);
	else
		*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
}


/* Main Function that purchase AMC deal */

static void
fm_mso_cust_bb_hw_amc_purchase_deal(
        pcm_context_t   *ctxp,
        pin_flist_t      *i_flistp,
        pin_flist_t    **r_flistpp,
        pin_errbuf_t    *ebufp)
{

	pin_decimal_t   *quantityp = (pin_decimal_t *)NULL;
	//pin_decimal_t   *discountp = (pin_decimal_t *)NULL;
	int32		Status = 1;
	int32   	*action_flags = NULL;
	time_t  	pvt = 0;
	time_t	 	*p_roundp = NULL;
	time_t		*amc_end_t = NULL;
	time_t		*amc_start_t = NULL;
	time_t          current_t = (time_t)0;
	time_t		end_zero = 0;
	time_t		*amc_charged_top = NULL;
	poid_t		*account_pdp = NULL;
	poid_t          *service_pdp = NULL;
	poid_t          *deal_pdp = NULL;
	poid_t          *plan_pdp = NULL;
	poid_t          *user_pdp = NULL;
	poid_t          *product_pdp = NULL;
	pin_flist_t     *product_flistp = NULL;
	pin_flist_t     *deal_flistp = NULL;
	pin_flist_t     *purchase_flistp = NULL;
	pin_flist_t	*purchase_rflistp = NULL;
	pin_flist_t	*s_rflistp = NULL;
	int             elem_id = 0;
	pin_cookie_t    cookie = NULL;
	int             belem_id = 0;
	pin_cookie_t    bcookie = NULL;
	pin_flist_t     *results_flistp = NULL;
	pin_flist_t     *pkgid_flistp = NULL;
	pin_flist_t	*pkgid_rflistp = NULL;
	poid_t		*offering_objp = NULL;
	int32		*pkg_id = NULL;
	char		*plan_name = NULL;
	char            *plan_code = NULL;
	pin_flist_t     *mso_flistp = NULL;
	pin_flist_t     *mso_rflistp = NULL;
	int              zero = 0;
	pin_flist_t     *dn_iflistp = NULL;
	pin_flist_t	*dn_rflistp = NULL;
	char		*deal_name = NULL;
	int64            diff = 0;
	int64		 diff_r = 0;
	int64		 start_diff = 0;
	time_t		 amc_time_t = (time_t)0;
	char		 msg[200];
	int32           *bill_when = NULL;
	poid_t          *device_objp = NULL;
	int32           device_type = RENTAL_DEPOSIT_PLAN ;
	pin_flist_t	*bal_flistp = NULL;
	int		*resouce_id = NULL;
	pin_decimal_t   *bal_impactp = NULL;
	time_t          post_to_t = 0;
	struct tm       *timeeff = NULL;
	struct tm       *twlmonths = NULL;
	int		*mode = NULL;
	pin_flist_t     *last_iflistp = NULL;
	pin_flist_t     *last_rflistp = NULL;
	pin_flist_t     *last_wflistp = NULL;
	time_t		*last_status_t = NULL;
	time_t          *eff_t = NULL;
	int32		actual_bill_when = 0;
	int32		back_date = 0;
	time_t          renew_from_t = 0;
	time_t          renew_current_t = 0;
	struct tm       *renew_timeeff = NULL;
	int64           renew_diff = 0;
	time_t  	new_eff_t = 0;
	
	
	
        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);
	

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_mso_cust_bb_hw_amc_purchase_deal input ", i_flistp);
	
	action_flags = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 0, ebufp);
	current_t = pin_virtual_time((time_t *)NULL);
	current_t = fm_utils_time_round_to_midnight(current_t);
	quantityp  = pbo_decimal_from_str("1", ebufp);
	//discountp = pbo_decimal_from_str("0", ebufp);
	plan_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_NAME, 0, ebufp);
	plan_code = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CODE, 0, ebufp);
	bill_when = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);
	device_objp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEVICE_OBJ, 0, ebufp);
	mode = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_MODE, 1, ebufp);
	eff_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_EFFECTIVE_T, 0, ebufp);
	user_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 0, ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
		//On Error
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"Unable to process input details", i_flistp);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Cannot read Subs Prod Details", ebufp);
		*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);	
		return;
		

        }

	deal_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEAL_OBJ, 0, ebufp);
	/*Read Deal Name */
	dn_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(dn_iflistp, PIN_FLD_POID, deal_pdp, ebufp);
	PIN_FLIST_FLD_SET(dn_iflistp, PIN_FLD_NAME, NULL, ebufp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, dn_iflistp, &dn_rflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "REAL DEAL NAME output ", dn_rflistp);
	deal_name = PIN_FLIST_FLD_GET(dn_rflistp, PIN_FLD_NAME, 0, ebufp);

        /* Prepare Main FList for PURCHASE DEAL */
        purchase_flistp = PIN_FLIST_CREATE(ebufp);
        account_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
        service_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 0 , ebufp);
        PIN_FLIST_FLD_SET(purchase_flistp, PIN_FLD_POID, account_pdp, ebufp);
        PIN_FLIST_FLD_SET(purchase_flistp, PIN_FLD_SERVICE_OBJ, service_pdp, ebufp);
        PIN_FLIST_FLD_SET(purchase_flistp, PIN_FLD_PROGRAM_NAME, "MSO_OP_CUST_BB_HW_AMC", ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_mso_cust_bb_hw_amc_purchase_deal input ", i_flistp);
	deal_flistp = PIN_FLIST_SUBSTR_ADD(purchase_flistp, PIN_FLD_DEAL_INFO,  ebufp);
        plan_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
	PIN_FLIST_FLD_SET(deal_flistp, PIN_FLD_POID, deal_pdp, ebufp);
	PIN_FLIST_FLD_SET(deal_flistp, PIN_FLD_NAME, deal_name, ebufp);
        PIN_FLIST_FLD_SET(deal_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
	PIN_FLIST_FLD_SET(deal_flistp, PIN_FLD_DESCR, "CPE CHARGES", ebufp);

	


	product_flistp = PIN_FLIST_ELEM_ADD(deal_flistp,  PIN_FLD_PRODUCTS, 0, ebufp);
	product_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
	PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_PRODUCT_OBJ, product_pdp, ebufp);
	PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_STATUS , &Status, ebufp);
	PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_QUANTITY, quantityp,  ebufp); 
	PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_DESCR, "CPE CHARGES", ebufp);
	if (!pbo_decimal_is_null(quantityp, ebufp)) {
		pbo_decimal_destroy(&quantityp);
	}	
PIN_FLIST_DESTROY_EX(&dn_iflistp, NULL);
 	
	/* decide what is the start date ??? 			 *
	 * If the case is renewal then use the last charged date *
	 * If the case is topup then use pvt 			 */	

	if(action_flags && *action_flags == AMC_ON_PREPAID_TOPUP ||
			   *action_flags == AMC_ON_CHANGE_PRE_PRE ||
			   *action_flags == AMC_ON_CHANGE_POST_PRE ||
			   *action_flags == AMC_ON_PREPAID_SWAP_O_R ||
			   *action_flags == AMC_ON_PREPAID_SWAP_R_R )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"AMC ON TOPUP START DATE IS PVT");
		PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_PURCHASE_START_T, &end_zero, ebufp);
		PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_CYCLE_START_T, &end_zero, ebufp);
		PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_USAGE_START_T, &end_zero, ebufp);
		PIN_FLIST_FLD_SET(deal_flistp, PIN_FLD_START_T, &end_zero, ebufp);
		PIN_FLIST_FLD_SET(deal_flistp, PIN_FLD_END_T, &end_zero, ebufp);			

	}

	if(action_flags && *action_flags == AMC_ON_PREPAID_RENEWAL)
        {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
				"AMC ON RENEWAL START DATE IS HW PRODUCT LAST CHARGED TIME");
		amc_charged_top = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CHARGED_TO_T, 0, ebufp);
		current_t = pin_virtual_time((time_t *)NULL);

		if(*amc_charged_top > 0 && AMC_INACTIVE_DAYS > 0)
		{

			/*Read Service Last Status */
                	last_iflistp = PIN_FLIST_CREATE(ebufp);
                	PIN_FLIST_FLD_SET(last_iflistp, PIN_FLD_POID, service_pdp, ebufp);
                	PIN_FLIST_FLD_SET(last_iflistp, PIN_FLD_LAST_STATUS_T, NULL, ebufp);
                	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, last_iflistp, &last_rflistp , ebufp);
                	last_status_t = PIN_FLIST_FLD_GET(last_rflistp, PIN_FLD_LAST_STATUS_T, 0, ebufp);


			/*update Service Last Status */
                	PIN_FLIST_FLD_SET(last_iflistp, PIN_FLD_LAST_STATUS_T, eff_t, ebufp);
                	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, last_iflistp, &last_wflistp, ebufp);
			diff_r = *amc_charged_top - current_t;
			back_date = 1;

			/* AMC INACTIVE SHOULD OVERRIDE THIS */
			renew_current_t = pin_virtual_time((time_t *)NULL);
			renew_current_t = renew_current_t - AMC_INACTIVE_DAYS*86400;
			renew_timeeff = localtime(&renew_current_t);
			renew_from_t = mktime(renew_timeeff);
			renew_diff = renew_from_t - current_t;
			if(renew_diff > diff_r)
				diff_r = renew_diff;
		}
		else
		{
			diff_r = 0;
		}

		/*Set Renew Dates*/
                PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_PURCHASE_START_T, &diff_r, ebufp);
                PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_CYCLE_START_T, &diff_r, ebufp);
                PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_USAGE_START_T, &diff_r, ebufp);
		PIN_FLIST_FLD_SET(deal_flistp, PIN_FLD_START_T, &end_zero, ebufp);
		PIN_FLIST_FLD_SET(deal_flistp, PIN_FLD_END_T, &end_zero, ebufp);

        }


        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                //On Error
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Serious issue setting END Dates for AMC DEAL", purchase_flistp);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Cannot Proceed with AMC PURCHASE", ebufp);
		PIN_FLIST_DESTROY_EX(&purchase_flistp, ebufp);
		*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
                return;


        }



	if(action_flags && (*action_flags == AMC_ON_PREPAID_TOPUP ||
			    *action_flags == AMC_ON_PREPAID_RENEWAL ||
			    *action_flags == AMC_ON_CHANGE_PRE_PRE ||
			    *action_flags == AMC_ON_CHANGE_POST_PRE ||
			    *action_flags == AMC_ON_PREPAID_SWAP_O_R ||
			    *action_flags == AMC_ON_PREPAID_SWAP_R_R ))
	{ 
		amc_end_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CYCLE_END_T, 0, ebufp);
		amc_start_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USAGE_START_T, 1, ebufp);
		current_t = pin_virtual_time((time_t *)NULL);
		diff = *amc_end_t - current_t;  
        if (amc_start_t)
        {
		    start_diff = *amc_start_t - current_t;  
		    PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_USAGE_START_T, &start_diff, ebufp); 
        }

		/* Setting AMC End Dates to subscription end date */
		PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_PURCHASE_END_T, &diff, ebufp);
		PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_CYCLE_END_T, &diff, ebufp);
		PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_USAGE_END_T, &diff, ebufp); 
	}

	/* For postpaid purchase 12 months delayed */
	if(action_flags && (*action_flags == AMC_ON_POSTPAID_HW_PURCHASE ||
			    *action_flags == AMC_ON_CHANGE_PRE_POST ||
			    *action_flags == AMC_ON_POSTPAID_SWAP_O_R ||
			    *action_flags == AMC_ON_POSTPAID_SWAP_R_R ||
		            *action_flags == AMC_ON_CHANGE_POST_POST))
	{ 


		/*Normally POSTPAID AMC STARTS AFTER 12 MONTHS */
		current_t = pin_virtual_time((time_t *)NULL);
		new_eff_t = fm_utils_time_round_to_midnight(*eff_t);
		timeeff = localtime(&new_eff_t);
		timeeff->tm_mon = timeeff->tm_mon + 12;
		post_to_t = mktime (timeeff);
		diff = post_to_t - current_t; 

		if(mode && *mode == 0 && diff > 0)
                {
                        if(!fm_mso_cust_bb_hw_create_deferred_action(ctxp, account_pdp, service_pdp, user_pdp, diff, ebufp))
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        		"Unable to create deferred action ", i_flistp);
                		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Cannot Proceed with AMC PURCHASE", ebufp);
				*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
				return;
			 }
			 else
			 {
				//*r_flistpp = i_flistp;
				*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "POSTPAID PURCHASE LATER");
				return;
			 }
		}
		else
		{
			diff = 0;
			if(post_to_t > current_t)
				diff = post_to_t - current_t;;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "POSTPAID PURCHASE NOW ");
		}

		
		PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_PURCHASE_START_T, &diff, ebufp);
                PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_CYCLE_START_T, &diff, ebufp);
                PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_USAGE_START_T, &diff, ebufp);
                PIN_FLIST_FLD_SET(deal_flistp, PIN_FLD_START_T, &end_zero, ebufp);
                PIN_FLIST_FLD_SET(deal_flistp, PIN_FLD_END_T, &end_zero, ebufp);

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "AMC PURCHASE DEAL POSTPAID ");
	}

		
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "AMC PURCHASE DEAL INPUT", purchase_flistp);

	/* IF price calculation only then return with calc only mode */
	if(mode && *mode == 1)
	{
		PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_PURCHASE_DEAL, PCM_OPFLG_CALC_ONLY, 
					purchase_flistp, &purchase_rflistp, ebufp);

	}
	else
	{
		PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_PURCHASE_DEAL, 0,
                                        purchase_flistp, &purchase_rflistp, ebufp);
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "AMC PURCHASE DEAL OUTPUT", purchase_rflistp);

	if(action_flags && *action_flags == AMC_ON_PREPAID_RENEWAL && back_date == 1)
	{
		/*Reset Service Last Status */
		PIN_FLIST_FLD_SET(last_iflistp, PIN_FLD_LAST_STATUS_T, last_status_t, ebufp);
		PIN_FLIST_DESTROY_EX(&last_wflistp, NULL);
                PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, last_iflistp, &last_wflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&last_wflistp, NULL);
		PIN_FLIST_DESTROY_EX(&last_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&last_rflistp, NULL);
	}

	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                //On Error
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Error Purchasing AMC DEAL", purchase_flistp);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Cannot Proceed with AMC PURCHASE", ebufp);
                PIN_FLIST_DESTROY_EX(&purchase_flistp, NULL);
		*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
                return;

        }

	if(mode && *mode == 1)
	{
		/*return from here */
		PIN_FLIST_DESTROY_EX(&purchase_flistp, ebufp);
		*r_flistpp = PIN_FLIST_COPY(purchase_rflistp, ebufp);	
		return;
	}
		

	/*Now make entires to /mso_purchased_plan object */

	while ((results_flistp =   
		PIN_FLIST_ELEM_GET_NEXT(purchase_rflistp, PIN_FLD_PRODUCTS,
                   &elem_id, 1, &cookie, ebufp)) != NULL)
        {
		
		offering_objp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_OFFERING_OBJ, 0, ebufp);
		pkg_id = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PACKAGE_ID, 0, ebufp);
			
	}

	/*get service bill_when */
	actual_bill_when = get_bill_when_from_service(ctxp, service_pdp, ebufp);

	/*prepare INPUT FLIST FOR creating /mso_purchased_plan object */
	mso_flistp  = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(mso_flistp, PIN_FLD_POID, account_pdp,ebufp);
	PIN_FLIST_FLD_SET(mso_flistp, PIN_FLD_ACCOUNT_OBJ, account_pdp,ebufp);
	PIN_FLIST_FLD_SET(mso_flistp, PIN_FLD_SERVICE_OBJ, service_pdp, ebufp);
	PIN_FLIST_FLD_SET(mso_flistp, PIN_FLD_BILL_WHEN, &actual_bill_when, ebufp);
	deal_flistp = PIN_FLIST_ELEM_ADD(mso_flistp,  PIN_FLD_DEALS, 0, ebufp);
	PIN_FLIST_FLD_SET(deal_flistp, PIN_FLD_DEAL_OBJ, deal_pdp, ebufp);
        PIN_FLIST_FLD_SET(deal_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
	PIN_FLIST_FLD_SET(deal_flistp, PIN_FLD_CODE, plan_code, ebufp);
	PIN_FLIST_FLD_SET(deal_flistp, PIN_FLD_NAME, plan_name, ebufp);
	PIN_FLIST_FLD_SET(deal_flistp, PIN_FLD_BILL_WHEN, &actual_bill_when, ebufp);
	product_flistp = PIN_FLIST_ELEM_ADD(deal_flistp,  PIN_FLD_PRODUCTS, 0, ebufp);
	PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_PRODUCT_OBJ, product_pdp, ebufp);
	PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_OFFERING_OBJ, offering_objp, ebufp);
	PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_PACKAGE_ID, pkg_id, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                "MSO PURCHASED PLAN INPUT", mso_flistp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                //On Error
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Cannot Create Entry in /mso_purchased_plan ", mso_flistp);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Cannot Create /mso_purchased_plan", ebufp);
                PIN_FLIST_DESTROY_EX(&purchase_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&mso_flistp, NULL);
		*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);	
                return;


        }


	fm_mso_create_purchased_plan_object(ctxp, mso_flistp, NULL, NULL, &mso_rflistp, ebufp);


        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                //On Error
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Cannot Create Entry in /mso_purchased_plan ", mso_flistp);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Cannot Create /mso_purchased_plan", ebufp);
                PIN_FLIST_DESTROY_EX(&purchase_flistp, NULL);
                PIN_FLIST_DESTROY_EX(&mso_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&mso_rflistp, NULL);
		*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
                return;


        }


        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                "MSO PURCHASED OUTPUT", mso_rflistp);

        /*Now make entires to device */

	elem_id = 0;
	cookie = NULL;
	results_flistp = NULL;
        while ((results_flistp =
                PIN_FLIST_ELEM_GET_NEXT(mso_rflistp, PIN_FLD_PRODUCTS,
                   &elem_id, 1, &cookie, ebufp)) != NULL)
        {


                offering_objp = PIN_FLIST_FLD_GET(results_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, 0, ebufp);

        }

	PIN_FLIST_DESTROY_EX(&mso_flistp, NULL);


	mso_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(mso_flistp, PIN_FLD_POID, device_objp, ebufp);
	
	
	deal_flistp = PIN_FLIST_ELEM_ADD(mso_flistp, PIN_FLD_PLAN, 1, ebufp);
	PIN_FLIST_FLD_SET(deal_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, offering_objp, ebufp);
	PIN_FLIST_FLD_SET(deal_flistp, PIN_FLD_TYPE, &device_type, ebufp); 

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                "MSO ADD AMC TO DEVICE", mso_flistp);
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 32, mso_flistp, &mso_rflistp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                "MSO ADD AMC TO DEVICE output flsit", mso_rflistp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                //On Error
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Cannot Create Entry in /device ", mso_flistp);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Cannot update /device", ebufp);
                PIN_FLIST_DESTROY_EX(&purchase_flistp, NULL);
                PIN_FLIST_DESTROY_EX(&mso_flistp, NULL);
                PIN_FLIST_DESTROY_EX(&mso_rflistp, NULL);
		*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
                return;


        }

	
	/*set the bal impact to output or set 0*/

	PIN_FLIST_DESTROY_EX(&pkgid_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&mso_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&mso_rflistp, NULL);
	*r_flistpp = PIN_FLIST_COPY(purchase_rflistp, ebufp);	
	pbo_decimal_destroy(&quantityp);
}


/*************************************
 *Get Subscription plan details  
*************************************/

static void
fm_mso_cust_bb_hw_amc_get_sub_plan_details(
        pcm_context_t   *ctxp,
        pin_flist_t      *i_flistp,
        pin_flist_t    **r_flistpp,
        pin_errbuf_t    *ebufp)

{

        pin_flist_t     *args_flistp = NULL;
        pin_flist_t     *search_inflistp = NULL;
        pin_flist_t     *results_flistp = NULL;
	pin_flist_t     *presults_flistp = NULL;
        pin_flist_t     *search_outflistp = NULL;
	pin_flist_t     *psearch_outflistp = NULL;
        pin_flist_t     *services_flistp = NULL;
	pin_flist_t     *deal_flistp = NULL;
	pin_flist_t	*final_rflistp = NULL;
	pin_flist_t     *final_tflistp = NULL;
	pin_flist_t     *u_flistp = NULL;
	char		*eve_type = NULL;

        char            search_template[200] = " select X from /product where F1 = V1 ";
	char psearch_template[200] = " select X from /plan 1 , /deal 2 , /product 3  where 1.F1 = 2.F2 and 2.F3 = 3.F4 and 3.F4 = V4 ";

		
        int             search_flags = 768;
        int64           db = 1;
        int             elem_id = 0;
        pin_cookie_t    cookie = NULL;
	int             pelem_id = 0;
	pin_cookie_t    pcookie = NULL;
        pin_decimal_t   *priority = NULL;
	poid_t          *service_pdp = NULL;
	poid_t          *product_pdp = NULL;
	poid_t          *plan_pdp = NULL;
	poid_t          *deal_pdp = NULL;
	char		*p_name = NULL;
	char		*p_code = NULL;
		

		
        poid_t          *a_pdp = NULL;
        poid_t          *p_pdp = NULL;
        pin_flist_t     *s_flistp = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_mso_cust_bb_hw_amc_get_sub_plan_details input flist", i_flistp);
						
	p_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	db = PIN_POID_GET_DB(p_pdp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
               PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Mandatory Fields Missing", ebufp);
                return;
		*r_flistpp = i_flistp;
        }

        /*************
        * search flist to search device details based on service poid
        ************/

        search_inflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
        PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, p_pdp, ebufp);

        results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "search product input list", search_inflistp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                //On Error Return Cleanly
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "search product input list", i_flistp);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error Preparing Input for Search for sub plan details", ebufp);
                PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
		*r_flistpp = i_flistp;
                return;

        }

        PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                //On Error
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "search input list", search_inflistp);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error calling Search in sub plan details", ebufp);
                PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
		*r_flistpp = i_flistp;
		return;
				
	}
		
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "search SUBS product output flist", search_outflistp);

	/* Capture results in this flist */
	final_rflistp = PIN_FLIST_CREATE(ebufp);

        while ((results_flistp =   // Loop through each  product and capture results 
                 PIN_FLIST_ELEM_GET_NEXT(search_outflistp, PIN_FLD_RESULTS,
                                        &elem_id, 1, &cookie, ebufp)) != NULL)
        {
                product_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_POID, 0, ebufp);
		priority = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PRIORITY, 0, ebufp); 
		u_flistp = PIN_FLIST_ELEM_GET(results_flistp, PIN_FLD_USAGE_MAP, PIN_ELEMID_ANY, 0, ebufp);
		eve_type = PIN_FLIST_FLD_GET(u_flistp, PIN_FLD_EVENT_TYPE, 1 , ebufp);
		/* set results */
		PIN_FLIST_FLD_SET(final_rflistp, PIN_FLD_POID, product_pdp, ebufp);
		PIN_FLIST_FLD_SET(final_rflistp, PIN_FLD_PRIORITY, priority, ebufp);
			
		if(eve_type)
			PIN_FLIST_FLD_SET(final_rflistp, PIN_FLD_EVENT_TYPE, eve_type, ebufp);
		
		if(!eve_type || !product_pdp || PIN_ERRBUF_IS_ERR(ebufp))
		{
         		//On Error
          		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                      		"search input list", search_inflistp);
           		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error calling AMC product Search", ebufp);
                	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
			PIN_FLIST_DESTROY_EX(&final_rflistp, NULL);
			*r_flistpp = i_flistp;
			return;
		}
		
		/* search for Plan and Deal */
		PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
		PIN_FLIST_DESTROY_EX(&psearch_outflistp, NULL);
		
        	pelem_id = 0;
        	pcookie = NULL;
		
		
		search_inflistp = PIN_FLIST_CREATE(ebufp);
        	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
        	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, psearch_template, ebufp);
        	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
		s_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_SERVICES, 1, ebufp);
        	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_DEAL_OBJ, NULL, ebufp);
		
		args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);
		
		args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 3, ebufp);
		s_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_PRODUCTS, 1, ebufp);
        	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 4, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, product_pdp, ebufp);
        	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                       "search plan input list", search_inflistp);

        	if (PIN_ERRBUF_IS_ERR(ebufp))
        	{
                	//On Error Return Cleanly
                	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        	"search AMC plan input list", search_inflistp);
                	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error Preparing Input for AMC plan Search", ebufp);
                	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
			PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
			IN_FLIST_DESTROY_EX(&final_rflistp, NULL);
	        	*r_flistpp = i_flistp;	
                	return;

        	}

        	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &psearch_outflistp, ebufp);

        	if (PIN_ERRBUF_IS_ERR(ebufp))
        	{
                	//On Error
                	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        	"search plan input list", search_inflistp);
                	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error calling Search of products", ebufp);
                	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
			PIN_FLIST_DESTROY_EX(&psearch_outflistp, NULL);
			PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
			PIN_FLIST_DESTROY_EX(&final_rflistp, NULL);	
			*r_flistpp = i_flistp;
			return;
				
		}
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                	"search SUBS Plan output flist", psearch_outflistp);
	
        	while ((presults_flistp =  //Get plan and deal for the plan got above
                 	PIN_FLIST_ELEM_GET_NEXT(psearch_outflistp, PIN_FLD_RESULTS,
                                        &pelem_id, 1, &pcookie, ebufp)) != NULL)
        	{
                	plan_pdp = PIN_FLIST_FLD_GET(presults_flistp, PIN_FLD_POID, 0, ebufp);
			deal_flistp = PIN_FLIST_ELEM_GET(presults_flistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY, 1, ebufp);
			deal_pdp = PIN_FLIST_FLD_GET(deal_flistp, PIN_FLD_DEAL_OBJ, 0, ebufp);
			p_name = PIN_FLIST_FLD_GET(presults_flistp, PIN_FLD_NAME, 0, ebufp);
			p_code = PIN_FLIST_FLD_GET(presults_flistp, PIN_FLD_CODE, 0, ebufp);
                
		
			if (PIN_ERRBUF_IS_ERR(ebufp))
        		{
                		//On Error
                		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        		"search plan input list", search_inflistp);
                		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error calling Search,fetching plan service details", ebufp);
                		PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
				PIN_FLIST_DESTROY_EX(&psearch_outflistp, NULL);
				PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);	
				PIN_FLIST_DESTROY_EX(&final_rflistp, NULL);
				*r_flistpp = i_flistp;
				return;
			}

			PIN_FLIST_FLD_SET(final_rflistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
			PIN_FLIST_FLD_SET(final_rflistp, PIN_FLD_DEAL_OBJ, deal_pdp, ebufp);
			PIN_FLIST_FLD_SET(final_rflistp, PIN_FLD_NAME, p_name, ebufp);
			PIN_FLIST_FLD_SET(final_rflistp, PIN_FLD_CODE, p_code, ebufp);
		}

	}
	*r_flistpp = final_rflistp;

       	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
       	return;

}


/**********************************************
 * Search service for the BBINFO
 *********************************************/
static void
fm_mso_cust_bb_hw_amc_search_service(
        pcm_context_t   *ctxp,
        pin_flist_t      *i_flistp,
        pin_flist_t    **r_flistpp,
        pin_errbuf_t    *ebufp)
{
		
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*search_inflistp = NULL;
	pin_flist_t	*search_outflistp = NULL;
	pin_flist_t	*results_flistp = NULL;
	pin_flist_t	*services_flistp = NULL;
	pin_flist_t     *plan_flistp = NULL;
	
	
	char		search_template[100] = " select X from /service where F1 = V1 ";
	int	    	search_flags = 768;
	int64		db = 1;
	int         	elem_id = 0;
	pin_cookie_t    cookie = NULL;
	

	poid_t		*service_pdp = NULL;
	pin_flist_t     *s_flistp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
			"fm_mso_cust_bb_hw_amc_search_service input flist", i_flistp);

	service_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);

	/*************
	 * search flist to search service details based on service poid
	 ************/
	if(!service_pdp)
	{
		PIN_ERRBUF_CLEAR(ebufp);
		return ;
	}

	db = PIN_POID_GET_DB(service_pdp);
		
	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, service_pdp, ebufp);
	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
			"search service input list", search_inflistp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                //On Error Return Cleanly
                PIN_ERRBUF_CLEAR(ebufp);

        }

	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		//On Error Return Cleanly
		PIN_ERRBUF_CLEAR(ebufp);
			
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
				"search service output flist", search_outflistp);
				
		results_flistp = PIN_FLIST_ELEM_GET(search_outflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		*r_flistpp = PIN_FLIST_SUBSTR_GET(results_flistp, MSO_FLD_BB_INFO, 1, ebufp);
         

	}
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	return;
	
}


/**********************************************
* Search FOR THE AMC PLANS THAT ARE TO BE EXCLUDED 
*********************************************/
static int
fm_mso_cust_bb_hw_amc_search_exclusion(
        pcm_context_t   *ctxp,
        pin_flist_t      *i_flistp,
        pin_errbuf_t    *ebufp)
{
		
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*search_inflistp = NULL;
	pin_flist_t	*search_outflistp = NULL;
	pin_flist_t	*results_flistp = NULL;
	pin_flist_t     *plan_flistp = NULL;
	
	
	char		search_template[100] = " select X from /mso_amc_plan_exclusion where F1 = V1 ";
	int	    	search_flags = 768;
	int64		db = 1;
	int         	elem_id = 0;
	pin_cookie_t    cookie = NULL;
	
	poid_t		*plan_pdp = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
			"fm_mso_cust_bb_hw_amc_search_exclusion input flist", i_flistp);

	plan_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);

	/*************
	 * search flist to search plan details based on plan poid
	 ************/
	if(!plan_pdp)
	{
		PIN_ERRBUF_CLEAR(ebufp);
		return ;
	}

	db = PIN_POID_GET_DB(plan_pdp);
		
	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
			"search plan input list", search_inflistp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                //On Error Return Cleanly
                PIN_ERRBUF_CLEAR(ebufp);

        }

	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		//On Error Return Cleanly
		PIN_ERRBUF_CLEAR(ebufp);
		return 0;
			
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
				"amc exclusion plan", search_outflistp);
		if(PIN_FLIST_ELEM_COUNT(search_outflistp, PIN_FLD_RESULTS, ebufp))
		{
				return 1 ;
		}
		else
		{
				return 0;
		}
		
	}
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
	return;
	
}

/*parse Input */
static void
fm_mso_cust_bb_hw_amc_parse_input(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp)

{


	pin_flist_t     *results_flistp = NULL;
	pin_flist_t     *product_flistp = NULL;
	pin_flist_t     *cycle_flistp = NULL;
	pin_flist_t     *s_iflistp = NULL;
	pin_flist_t     *s_rflistp = NULL;
    int             elem_id = 0;
    pin_cookie_t    cookie = NULL;
	poid_t		*p_pdp = NULL;
	int              subsc_type = 0;
	pin_decimal_t   *priority = NULL;
    double           prty = 0;
	time_t		*cycle_end_t = NULL;
	pin_flist_t	*bal_flistp = NULL;
	int		*res_id = NULL;
	int32		*modep = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_mso_cust_bb_hw_amc_parse_input input ", i_flistp);
	modep = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_MODE, 0, ebufp);

	/* In Real Mode input contain only service poid */
	if(modep && *modep == 0)
	{
		fm_mso_cust_bb_hw_amc_get_cycle_details(ctxp, i_flistp, ebufp);
	}

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
             //On Error
            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Issue Parsing Input in real Mode", i_flistp);
		    PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error parsing passed input", ebufp);
            return;
        }
 
	
     while((results_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_RESULTS,&elem_id, 1, &cookie, ebufp)) != NULL)
     {
		
		product_flistp = PIN_FLIST_SUBSTR_GET(results_flistp, PIN_FLD_PRODUCT, 0, ebufp);
		cycle_flistp = PIN_FLIST_SUBSTR_GET(results_flistp, PIN_FLD_CYCLE_INFO, 0, ebufp);
		bal_flistp = PIN_FLIST_ELEM_GET(results_flistp, PIN_FLD_BAL_IMPACTS, PIN_ELEMID_ANY, 1, ebufp);
		if(bal_flistp)
			res_id = PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_RESOURCE_ID, 1, ebufp);
		if(res_id && *res_id != 356)
			continue;

		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "CANNOT PARSE INPUT" , i_flistp);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error parsing passed input", ebufp);
            PIN_ERRBUF_RESET(ebufp);
		}

		if(cycle_flistp)
		{
			cycle_end_t = PIN_FLIST_FLD_GET(cycle_flistp, PIN_FLD_CYCLE_END_T, 0, ebufp);
		}
		if(product_flistp)
		{
			p_pdp = PIN_FLIST_FLD_GET(product_flistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
		}

		if (PIN_ERRBUF_IS_ERR(ebufp))
        {
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error mandatory field not passed input", ebufp);
            PIN_ERRBUF_RESET(ebufp);
		    return;
        }

		s_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(s_iflistp, PIN_FLD_POID, p_pdp, ebufp);

		fm_mso_cust_bb_hw_amc_get_sub_plan_details(ctxp, s_iflistp, &s_rflistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"SEARCH SUBSCRIPTION PLAN OUTPUT", s_rflistp);

		priority = PIN_FLIST_FLD_GET(s_rflistp, PIN_FLD_PRIORITY, 1, ebufp);
		
		if(priority)
			prty = pbo_decimal_to_double(priority, ebufp);	
		if ( ( prty >= BB_COR_PREPAID_START && prty <= BB_COR_PREPAID_END ) ||
                        ( prty >= BB_COR_POSTPAID_START && prty <= BB_COR_POSTPAID_END ) )
		{
                        subsc_type = 1;
		}
		if ( ( prty >= BB_CYB_PREPAID_START && prty <= BB_CYB_PREPAID_END ) ||
                        ( prty >= BB_CYB_POSTPAID_START && prty <= BB_CYB_POSTPAID_END ) )
		{
                        subsc_type = 1;
		}
		if ( ( prty >= BB_SME_PREPAID_START && prty <= BB_SME_PREPAID_END ) ||
                        ( prty >= BB_SME_POSTPAID_START && prty <= BB_SME_POSTPAID_END ) )
		{
                        subsc_type = 1;
		}
		if ( ( prty >= BB_RET_PREPAID_START && prty <= BB_RET_PREPAID_END ) ||
                        ( prty >= BB_RET_POSTPAID_START && prty <= BB_RET_POSTPAID_END ) )
		{
	                subsc_type = 1;
		}		
		if(subsc_type)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "FOUND SUBSCRIPTION TYPE");
			PIN_FLIST_FLD_SET(s_rflistp, PIN_FLD_CYCLE_END_T, cycle_end_t, ebufp);
			*r_flistpp = s_rflistp;
			return;
		}
        if(!subsc_type)
        {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "NO SUBSCRIPTION TYPE IN INPUT");
            //PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"INPUT PASSED IS INCORRECT", s_rflistp);
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error parsing passed input", ebufp);
            PIN_ERRBUF_RESET(ebufp);
            *r_flistpp = i_flistp;
            return;
        }
      }
		
		PIN_FLIST_DESTROY_EX(&s_rflistp, NULL);
		PIN_FLIST_DESTROY_EX(&s_iflistp, NULL);
}

/**********************************************
 * Search service for the mso_purchased_plan 
 * Entries and get subscription Details  
 *********************************************/

extern  void
fm_mso_cust_bb_hw_amc_get_cycle_details(
        pcm_context_t   *ctxp,
        pin_flist_t      *i_flistp,
        pin_errbuf_t    *ebufp)
{


        pin_flist_t     *args_flistp = NULL;
        pin_flist_t     *search_inflistp = NULL;
        pin_flist_t     *search_outflistp = NULL;
        pin_flist_t     *results_flistp = NULL;
        pin_flist_t     *services_flistp = NULL;
	pin_flist_t     *subs_outflistp = NULL;

        char            *device_id = NULL;
        char            search_template[150] = " select X from /mso_purchased_plan 1,/purchased_product 2 where 1.F1 = V1 and 1.F2 = V2 and (2.F3 = V3 or 2.F6 = V6) and 1.F4 = 2.F5 ";
        int             search_flags = 768;
        int64           db = 1;
        int             elem_id = 0;
        pin_cookie_t    pcookie = NULL;
	int             pelem_id = 0;
        pin_cookie_t    cookie = NULL;
        poid_t          *service_pdp = NULL;
	poid_t          *a_pdp = NULL;
        pin_flist_t     *s_flistp = NULL;
	int32		subs_found = 0;
	poid_t		*subs_pdp = NULL;
	char		*prov_tag = NULL;
	pin_flist_t	*products_flistp = NULL;
	int32 		*p_status = NULL;
	time_t		*cycle_end_t = NULL;
	pin_flist_t     *res_flistp = NULL;
	pin_flist_t 	*pr_flistp = NULL;
	pin_flist_t	*cycle_flistp = NULL;
	poid_t		*p_pdp = NULL;
	int		inr = 356;
	char		*plan_type = "base plan";
	int32		act_status = 1;	
	int32		inact_status = 2;	

        if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
                
	PIN_ERRBUF_CLEAR(ebufp);

    	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_mso_cust_bb_hw_amc_get_cycle_details input flist", i_flistp);

	a_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	

        /*************
         * search flist to search device details based on service poid
         ************/
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                //On Error
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                "Subscription product Error preparation", search_inflistp);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error calling Subscription search", ebufp);
                return;

        }


	db = PIN_POID_GET_DB(a_pdp);

        search_inflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
        PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, a_pdp, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DESCR, plan_type, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &act_status, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 4, ebufp);
        products_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(products_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, NULL, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 5, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 6, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &inact_status, ebufp);
        results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "search mso_purchased_plan input list", search_inflistp);

        if (PIN_ERRBUF_IS_ERR(ebufp)) 
        {
                //On Error
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                "Subscription product Error preparation", search_inflistp);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error calling Subscription search", ebufp);
                PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
                PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
                return;

        }


        PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp) || search_outflistp == NULL)
        {
            //On Error
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"search input list Subscription search", search_inflistp);
        	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error calling Subscription Search", ebufp);
        	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
		return;
				
	}	

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                "Subscription Search", search_outflistp);
         /*search Through the results and get the product which has Prov Tag */
	while ((results_flistp =   
				PIN_FLIST_ELEM_GET_NEXT(search_outflistp, PIN_FLD_RESULTS,
                               &elem_id, 1, &cookie, ebufp)) != NULL)
        {
		p_status = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_STATUS , 1, ebufp);
		products_flistp = NULL;
		pelem_id = 0;
		pcookie = NULL;
		if(subs_found)
			break;
		if(p_status && (*p_status == MSO_PROV_ACTIVE || *p_status == MSO_PROV_IN_PROGRESS))
		{		
			while ((products_flistp =   
				PIN_FLIST_ELEM_GET_NEXT(results_flistp, PIN_FLD_PRODUCTS,
                              		&pelem_id, 1, &pcookie, ebufp)) != NULL)
			{
				prov_tag = (char *)PIN_FLIST_FLD_GET(products_flistp, PIN_FLD_PROVISIONING_TAG, 1, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Inside PROV TAG SEARCH", products_flistp);
				if(prov_tag && strlen(prov_tag)>0 )
		
				{
					subs_found = 1;
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "FOUND SUBS PRODUCT");
					subs_pdp = PIN_FLIST_FLD_GET(products_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, 0, ebufp);
					break;
				}
			}
              

        	}
	}
		
		
	if (PIN_ERRBUF_IS_ERR(ebufp) || subs_found == 0 ) 
        {
            	//On Error
            	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"Subscription product search error", search_inflistp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
            		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error calling Subscription search", ebufp);
		}
            	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
		return;
				
	}	
		
	/*get cycle details of product */
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	search_inflistp = PIN_FLIST_CREATE(ebufp);
		
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_POID, subs_pdp,ebufp);
		
	PCM_OP(ctxp, PCM_OP_READ_OBJ , 0, search_inflistp, &subs_outflistp, ebufp);
		
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                "Read Subs Product Output", subs_outflistp);
								
	if (PIN_ERRBUF_IS_ERR(ebufp) || subs_outflistp == NULL)
        {
            //On Error
            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"Error in reading subs product", search_inflistp);
        	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error read obj of subscription", ebufp);
        	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		PIN_FLIST_DESTROY_EX(&subs_outflistp, NULL);
		PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
		return;
				
	}

	p_pdp = PIN_FLIST_FLD_GET(subs_outflistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp); 
	//IF CYCLE Array Present get charged_to_t 
	if(PIN_FLIST_ELEM_COUNT(subs_outflistp, PIN_FLD_CYCLE_FEES, ebufp))
	{
		cycle_flistp = PIN_FLIST_ELEM_GET(subs_outflistp, PIN_FLD_CYCLE_FEES, PIN_ELEMID_ANY, 1, ebufp);
		cycle_end_t = PIN_FLIST_FLD_GET(cycle_flistp, PIN_FLD_CHARGED_TO_T, 0, ebufp);
	}
	else // set cycle_end_t as charged_to_t of subscription
	{
		cycle_end_t = PIN_FLIST_FLD_GET(subs_outflistp, PIN_FLD_CYCLE_END_T, 0, ebufp);
	}

        if (PIN_ERRBUF_IS_ERR(ebufp)) 
        {
            //On Error
            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                "cannot Read SUBS PRODUCT CYCLE DATES ", search_inflistp);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error read obj of subscription", ebufp);
                PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
                PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
		PIN_FLIST_DESTROY_EX(&subs_outflistp, NULL);
                return;

        }

	res_flistp = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_RESULTS, 0, ebufp);
	pr_flistp = PIN_FLIST_SUBSTR_ADD(res_flistp, PIN_FLD_PRODUCT, ebufp);
	PIN_FLIST_FLD_SET(pr_flistp, PIN_FLD_PRODUCT_OBJ, p_pdp, ebufp);
	pr_flistp = PIN_FLIST_SUBSTR_ADD(res_flistp, PIN_FLD_CYCLE_INFO, ebufp);
	PIN_FLIST_FLD_SET(pr_flistp, PIN_FLD_CYCLE_END_T, cycle_end_t, ebufp);
	pr_flistp = PIN_FLIST_ELEM_ADD(res_flistp, PIN_FLD_BAL_IMPACTS, 0, ebufp);
	PIN_FLIST_FLD_SET(pr_flistp, PIN_FLD_RESOURCE_ID, &inr, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"Modified Input Flist is ", i_flistp);

		
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	PIN_FLIST_DESTROY_EX(&subs_outflistp, ebufp);
	//PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
        return;
}


/*get BILL_WHEN FROM Service */

static int32
get_bill_when_from_service(pcm_context_t          *ctxp,
                                 poid_t                 *svc_pdp,
                                 pin_errbuf_t            *ebufp)
{

	int32   bill_when = 0;
        pin_flist_t     *read_in_flistp = PIN_FLIST_CREATE(ebufp);
        pin_flist_t     *read_out_flistp = NULL;
        pin_flist_t     *bb_info_flistp = NULL;

        PIN_FLIST_FLD_SET(read_in_flistp, PIN_FLD_POID, svc_pdp, ebufp);
        bb_info_flistp = PIN_FLIST_ELEM_ADD(read_in_flistp, MSO_FLD_BB_INFO, 0, ebufp);
        PIN_FLIST_FLD_SET(bb_info_flistp, PIN_FLD_BILL_WHEN, NULL, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input flist to read bill_when from service", read_in_flistp);
        PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_in_flistp, &read_out_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Output flist to read bill_when from service", read_out_flistp);
        bb_info_flistp = PIN_FLIST_ELEM_GET(read_out_flistp, MSO_FLD_BB_INFO, 0 ,0, ebufp);
        bill_when = *(int32 *) PIN_FLIST_FLD_GET(bb_info_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);
        return bill_when;
}


/**********************************************
 * get old bill when before cancel 
 * this is useful in change plan case
 *********************************************/

static  int32
fm_mso_cust_bb_hw_amc_get_old_bill_when(
        pcm_context_t   *ctxp,
        poid_t      *a_pdp,
	poid_t      *p_pdp,
        pin_errbuf_t    *ebufp)
{


        pin_flist_t     *args_flistp = NULL;
        pin_flist_t     *search_inflistp = NULL;
        pin_flist_t     *search_outflistp = NULL;
        pin_flist_t     *results_flistp = NULL;
        pin_flist_t     *services_flistp = NULL;
	pin_flist_t     *subs_outflistp = NULL;
        
        char            search_template[100] = " select X from /mso_purchased_plan where F1 = V1 and F2 = V2 ";
        int             search_flags = 768;
        int64           db = 1;
        int             elem_id = 0;
        pin_cookie_t    cookie = NULL;
        poid_t          *service_pdp = NULL;
        pin_flist_t     *s_flistp = NULL;
	int32		*bill_whenp = NULL;
	pin_flist_t	*prod_flist = NULL;
	
		

        if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;
                
	PIN_ERRBUF_CLEAR(ebufp);

    	    

	db = PIN_POID_GET_DB(a_pdp);

        search_inflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
        PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, a_pdp, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp);
	prod_flist = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_PRODUCTS, 0, ebufp);
	PIN_FLIST_FLD_SET(prod_flist, MSO_FLD_PURCHASED_PRODUCT_OBJ, p_pdp, ebufp);
        results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "search fm_mso_cust_bb_hw_amc_get_old_bill_when", search_inflistp);

        if (PIN_ERRBUF_IS_ERR(ebufp)) 
        {
                //On Error
                PIN_ERRBUF_CLEAR(ebufp);
                PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
                PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
                return 0;

        }


        PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp) || search_outflistp == NULL)
        {
            //On Error
        	PIN_ERRBUF_CLEAR(ebufp);
        	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
		return 0;
	}
				
		

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                "fm_mso_cust_bb_hw_amc_get_old_bill_when output", search_outflistp);
         
	while ((results_flistp =   
				PIN_FLIST_ELEM_GET_NEXT(search_outflistp, PIN_FLD_RESULTS,
                               &elem_id, 1, &cookie, ebufp)) != NULL)
        {
		bill_whenp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_BILL_WHEN , 1, ebufp);
		
	}

	if (PIN_ERRBUF_IS_ERR(ebufp) || search_outflistp == NULL)
        {
            //On Error
                PIN_ERRBUF_CLEAR(ebufp);
                PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
                PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
                return 0;
        }

              
	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	if(bill_whenp)
		return *bill_whenp;
		
}


/*set BILL_WHEN FOR Service */

static void
set_bill_when_for_service(pcm_context_t          *ctxp,
                                 poid_t          *svc_pdp,
				 int32		  bill_when,
                                 pin_errbuf_t    *ebufp)
{

        pin_flist_t     *write_in_flistp = NULL;
        pin_flist_t     *write_out_flistp = NULL;
        pin_flist_t     *bb_info_flistp = NULL;

        write_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(write_in_flistp, PIN_FLD_POID, svc_pdp, ebufp);
        bb_info_flistp = PIN_FLIST_ELEM_ADD(write_in_flistp, MSO_FLD_BB_INFO, 0, ebufp);
        PIN_FLIST_FLD_SET(bb_info_flistp, PIN_FLD_BILL_WHEN, &bill_when, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input flist to write bill_when for service", write_in_flistp);
        PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_in_flistp, &write_out_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Output flist to write bill_when for service", write_out_flistp);
	PIN_FLIST_DESTROY_EX(&write_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&write_out_flistp, NULL);
		
        
}


/*create deferred action */
static int 
fm_mso_cust_bb_hw_create_deferred_action(
        pcm_context_t   *ctxp,
        poid_t          *a_pdp,
        poid_t          *s_pdp,
        poid_t          *u_pdp,
        int64           diff,
        pin_errbuf_t    *ebufp)
{

	pin_flist_t     *data_flistp = NULL;
	pin_flist_t     *s_flistp = NULL;
	int32		mode = 0;
	char            *char_buf = NULL;
	pin_buf_t       *pin_bufp = NULL;
	int32		amc_opcode = MSO_OP_CUST_BB_HW_AMC;
	pin_flist_t	*r_flistp = NULL;
	int32		len = 0;
	time_t          current_t = pin_virtual_time((time_t *)NULL);
	int32		action_flag = AMC_ON_POSTPAID_HW_PURCHASE;
	

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return 0;

        PIN_ERRBUF_CLEAR(ebufp);

	current_t = current_t + diff;

	/* create input flist for deferred action */
	data_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(data_flistp, PIN_FLD_POID, a_pdp, ebufp);
	PIN_FLIST_FLD_SET(data_flistp, PIN_FLD_SERVICE_OBJ, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(data_flistp, PIN_FLD_USERID, u_pdp, ebufp);
	PIN_FLIST_FLD_SET(data_flistp, PIN_FLD_FLAGS, &action_flag, ebufp);
	PIN_FLIST_FLD_SET(data_flistp, PIN_FLD_MODE, &mode , ebufp);
	
	/*create actual input */
	s_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_POID, a_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_WHEN_T, &current_t, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &mode , ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_PROGRAM_NAME, "MSO_OP_CUST_BB_HW_AMC", ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_ACTION_NAME, "MSO_OP_CUST_BB_HW_AMC", ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_SYS_DESCR, "MSO_OP_CUST_BB_HW_AMC", ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_OPCODE, &amc_opcode, ebufp);
	/* Put flist built for MSO_OP_CUST_ADD_PLAN
        * into buffer of PIN_FLD_INPUT_FLIST
        */
        PIN_FLIST_TO_STR(data_flistp, &char_buf, &len, ebufp);
        pin_bufp = (pin_buf_t *)calloc(1, sizeof(pin_buf_t));
        pin_bufp->data = (caddr_t)char_buf;
        pin_bufp->size = len + 1;
        pin_bufp->flag = 0;
        pin_bufp->offset = 0;
        pin_bufp->xbuf_file = NULL;
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_INPUT_FLIST, (void *)pin_bufp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Schedule create input flist", s_flistp);
        PCM_OP(ctxp, PCM_OP_ACT_SCHEDULE_CREATE, 0, s_flistp, &r_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Schedule create return flist", r_flistp);

	if(pin_bufp)
                 pin_free(pin_bufp);
        if (char_buf)
                pin_free(char_buf);
        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&data_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Create scheduler error.", ebufp);
                PIN_ERRBUF_CLEAR(ebufp);
                return 0;
        }
        return 1;
}


/*************************************
 *Delete all deferred actions on customer  
*************************************/

extern void
fm_mso_cust_bb_hw_delete_deferred_actions(
        pcm_context_t   *ctxp,
        poid_t		*a_pdp,
        pin_errbuf_t    *ebufp)

{

        pin_flist_t     *args_flistp = NULL;
        pin_flist_t     *search_inflistp = NULL;
        pin_flist_t     *results_flistp = NULL;
	pin_flist_t     *search_outflistp = NULL;
	pin_flist_t	*del_iflistp = NULL;
	pin_flist_t	*del_rflistp = NULL;
	poid_t		*def_pdp = NULL;
	int            	search_flags = 768;
        int64           db = 1;
        int             elem_id = 0;
        pin_cookie_t    cookie = NULL;
	int32		amc_opcode = MSO_OP_CUST_BB_HW_AMC;
	int32		amc_status = 0 ;
        char            search_template[200] = " select X from /schedule where F1 = V1 and F2 = V2 and F3 <> V3 ";
	int32		status = 1;	

		 
	
       

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);
        
						
	
	db = PIN_POID_GET_DB(a_pdp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
           PIN_ERRBUF_CLEAR(ebufp);
	   return;
        }

        /*************
        * search flist to search device details based on service poid
        ************/

        search_inflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
        PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, a_pdp, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_OPCODE, &amc_opcode, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &status, ebufp);	

        results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "search schedule objects input list", search_inflistp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERRBUF_CLEAR(ebufp);
		PIN_FLIST_DESTROY_EX(&search_inflistp, ebufp);
                return;

        }

        PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERRBUF_CLEAR(ebufp);
		PIN_FLIST_DESTROY_EX(&search_inflistp, ebufp);
		return;
				
	}
		
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "search schedule output flist", search_outflistp);

	 while ((results_flistp =   // Loop through each  product and capture results 
                 PIN_FLIST_ELEM_GET_NEXT(search_outflistp, PIN_FLD_RESULTS,
                                        &elem_id, 1, &cookie, ebufp)) != NULL)
        {
              	def_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_POID, 0, ebufp);
		del_iflistp = PIN_FLIST_CREATE(ebufp);
	 	PIN_FLIST_FLD_SET(del_iflistp, PIN_FLD_POID, def_pdp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                	"Delete schedule input flist", del_iflistp);

		PCM_OP(ctxp, PCM_OP_DELETE_OBJ, 0, del_iflistp, &del_rflistp, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "Delete schedule output flist", del_rflistp);

		PIN_FLIST_DESTROY_EX(&del_iflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&del_rflistp, ebufp);
			  
	}
				
				
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERRBUF_CLEAR(ebufp);
		PIN_FLIST_DESTROY_EX(&search_inflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&del_iflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&del_rflistp, ebufp);
		return;
			
	}

}


static int32 
fm_mso_get_active_amc_prod(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
	pin_flist_t		**out_flistpp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *srch_flistp = NULL;
        pin_flist_t             *arg_flist = NULL;
        pin_flist_t             *result_flist = NULL;
	poid_t			*srch_pdp = NULL;
        int64                   db = -1;
        int32                   srch_flag = 256;
        int                     status_active = 1;
        int                     status_inactive = 2;
	pin_flist_t		*srch_out_flistp = NULL;
        poid_t                  *acnt_pdp = NULL;
        poid_t                  *ser_pdp = NULL;
        char                    template[200] = "select X from /purchased_product 1,/product where 1.F1 = 2.F2 and 1.F3 = V3 and (1.F4 = V4 or 1.F7 = V7) and (2.F5 = V5 or 2.F6 = V6) ";
	pin_decimal_t   	*prio_700 = pbo_decimal_from_str("700.0",ebufp);		
	pin_decimal_t           *prio_770 = pbo_decimal_from_str("770.0",ebufp);
	int32			ret_val = 0;

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_active_amc_prod", ebufp);
        }

        acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	if(!acnt_pdp)
	{
		acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
	}
        PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG,"acnt_pdp", acnt_pdp);
        db = PIN_POID_GET_DB(acnt_pdp);
        srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
        srch_flistp = PIN_FLIST_CREATE(ebufp);

        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
	
	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);	

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL, ebufp);	

        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);
	
	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp );
        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STATUS, &status_active, ebufp);

        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 5, ebufp );
        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_PRIORITY, prio_700, ebufp);
	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 6, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_PRIORITY, prio_770, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 7, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STATUS, &status_inactive, ebufp);
        result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "BB Amc prodduct Search input flist", srch_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
                return -1;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "BB Amc product Search output flist", srch_out_flistp);
	if(PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp))
	{
		ret_val = 1;
	}
	*out_flistpp = PIN_FLIST_COPY(srch_out_flistp, ebufp);
	pbo_decimal_destroy(&prio_700);
	pbo_decimal_destroy(&prio_770);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	return ret_val;
}
