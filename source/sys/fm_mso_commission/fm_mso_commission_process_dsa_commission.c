
/*******************************************************************

 * File:        fm_mso_commission_process_dsa_commission.c
 * Opcode:      MSO_OP_COMMISSION_PROCESS_DSA_COMMISSION
 * Description: 
 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_commission_process_dsa_commission.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "pcm.h"
#include "pin_cust.h"
#include "ops/act.h"
#include "ops/cust.h"
#include "ops/bill.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_rate.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "mso_cust.h"
#include "mso_commission.h"

#define MSO_NO_TECHNOLOGY 0
int when_annum=12;
int when_month=1;

EXPORT_OP void
op_mso_commission_process_dsa_commission (
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);


static void
fm_mso_commission_process_dsa_commission (
        pcm_context_t            *ctxp,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);


//static int 
//fm_mso_commission_get_dsa_comm_rule (
//	pcm_context_t		*ctxp, 
//	pin_flist_t		*i_flistp, 
//	pin_flist_t		**out_flp, 
//	pin_errbuf_t		*ebufp);

static int 
fm_mso_apply_commission (
	pcm_context_t		*ctxp, 
	pin_flist_t		*i_flistp, 
	pin_flist_t		**out_flp, 
	pin_errbuf_t		*ebufp);

//static void
//fm_mso_commission_dsa_comm_info (
//        pcm_context_t           *ctxp,
//        pin_flist_t             *i_flistp,
//        pin_flist_t             **r_flistpp,
//        pin_errbuf_t            *ebufp);

static void
fm_mso_commission_calculate_commission (
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             *rule_flistp,
        pin_flist_t             *result_flp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_mso_calculate_percentage_commission (
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             *rule_flistp,
        pin_flist_t             *result_flp,
	char			*rule_type,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_mso_calculate_commission_order_count(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             *rule_flistp,
        pin_flist_t             *result_flp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_mso_commission_create_dsa_comm_report (
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

char *itoa(int i);

static void 
fm_mso_commission_get_dsa_rule(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t             **r_flistpp,
	pin_errbuf_t            *ebufp);

static void 
fm_mso_commission_get_dsa_criteria(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	int32                   *criteria_id,
	pin_flist_t             **r_flistpp,
	pin_errbuf_t            *ebufp);

static void 
fm_mso_commission_get_end_customers(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t             **r_flistpp,
	pin_errbuf_t            *ebufp);

void
op_mso_commission_process_dsa_commission (
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp) {

        pcm_context_t           *ctxp = connp->dm_ctx;
        *r_flistpp              = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

        /*******************************************************************
         * Insanity Check
         *******************************************************************/
        if (opcode != MSO_OP_COMMISSION_PROCESS_DSA_COMMISSION) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_mso_commission_process_dsa_commission error",ebufp);
                return;
        }

        /******************************************************************
         * Debug: Input flist
         *******************************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_mso_commission_process_dsa_commission input flist", i_flistp);

        fm_mso_commission_process_dsa_commission(ctxp, flags, i_flistp, r_flistpp, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_mso_commission_process_dsa_commission output flist", *r_flistpp);

        return;
}

static void
fm_mso_commission_process_dsa_commission(
        pcm_context_t           *ctxp,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{

	//pin_flist_t		*dup_i_flistp = NULL;
	pin_flist_t		*comm_flistp = NULL;
	//pin_flist_t		*r_comm_flistp = NULL;
	//pin_flist_t		*cust_info_flistp = NULL;
	//pin_flist_t		*comm_obj_flistp = NULL;
	//pin_flist_t		*out_flp = NULL;
	pin_flist_t		*result_flp = NULL;
	pin_flist_t		*comm_out_flistp = NULL;
	pin_flist_t		*rule_flistp = NULL;
	pin_flist_t		*rule_flp = NULL;
	pin_flist_t		*criteria_flistp = NULL;
	poid_t			*dsa_acct_objp = NULL;
	time_t			*start_t = NULL;
	time_t			*end_t = NULL;
	//int64			n_accts = 0;
	//pin_decimal_t		*comm_amt = NULL;
	int32			elem_id = 0;
	int32			status_succ = 0;
	int32			status_fail = 1;
	int32			*criteria = NULL;
	pin_cookie_t            cookie = NULL;
	//int 			count = 0;
	time_t          *billed_upto = NULL;
	//time_t          *end_t = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_process_dsa_commission input flist ", i_flistp);

	//Get the start and end date
	dsa_acct_objp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	billed_upto = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BILL_CYCLE_T, 0, ebufp);
	end_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 0, ebufp);
	start_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_START_T, 0, ebufp);

	//Get the number of active accounts 
	//fm_mso_commission_get_dsa_comm_rule(ctxp, i_flistp, &out_flp, ebufp);
	//count = PIN_FLIST_ELEM_COUNT(out_flp, PIN_FLD_RESULTS,ebufp);
	//if (count == 0) {
	//	*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
	//	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_DESCR, "Rule not associated to the account", ebufp);
	//	return;
	//}

	//Prepare the dsa commission information flistp
	//dup_i_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
	//PIN_FLIST_FLD_SET(dup_i_flistp, PIN_FLD_COUNT, &n_accts, ebufp);
	//fm_mso_commission_dsa_comm_info(ctxp, out_flp, &comm_flistp, ebufp);
	fm_mso_commission_get_dsa_rule(ctxp, i_flistp, &rule_flistp, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
		return;

	if(!rule_flistp)
		return;

	rule_flp = PIN_FLIST_ELEM_GET(rule_flistp, PIN_FLD_RESULTS,PIN_ELEMID_ANY,0, ebufp);
	criteria = PIN_FLIST_FLD_GET(rule_flp, MSO_FLD_COMM_CRITERIA_ID, 0, ebufp);

	fm_mso_commission_get_dsa_criteria(ctxp, i_flistp, criteria, &criteria_flistp, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	if(!criteria_flistp)
		return;
	//Apply commission
	//fm_mso_commission_dsa_apply_commission(ctxp, comm_flistp, &r_comm_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_commission_process_dsa_commission: criteria_flistp:",criteria_flistp);
	while ((result_flp = PIN_FLIST_ELEM_GET_NEXT(criteria_flistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
	{
		fm_mso_commission_calculate_commission(ctxp, i_flistp, rule_flp, result_flp, &comm_out_flistp, ebufp);	
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			*r_flistpp = i_flistp;
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_DESCR, "Error in applying commission", ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_fail, ebufp);
			PIN_FLIST_DESTROY_EX(&comm_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&comm_out_flistp, NULL);
               		return;
		}
		/*
		if (strcmp(PIN_FLIST_FLD_GET(comm_out_flistp, PIN_FLD_DESCR, 0, ebufp),"Usage Success") == 0)
		{
			break;
		}
		*/
	}
/*	
	else
	{
		*r_flistpp = i_flistp;
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_DESCR, "No Rules to apply commission", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_fail, ebufp);
                return;
	}
*/
	*r_flistpp = comm_out_flistp;
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_succ, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Final process DSA commission output flist is ",*r_flistpp);

	PIN_FLIST_DESTROY_EX(&comm_flistp, NULL);
}

//static int
//fm_mso_commission_get_dsa_comm_rule (
//        pcm_context_t           *ctxp,
//        pin_flist_t             *i_flistp,
//        pin_flist_t             **out_flp,
//        pin_errbuf_t            *ebufp) {
//
//        poid_t                  *dsa_acct_objp = NULL;
//        poid_t                  *s_pdp = NULL;
//        //time_t                  *start_t = NULL;
//        //time_t                  *end_t = NULL;
//        //int32                   n_accts = 0;
//
//	pin_flist_t		*s_flistp = NULL;
//	//pin_flist_t		*r_s_flistp = NULL;
//	pin_flist_t		*args_flistp = NULL;
//	pin_flist_t		*r_flistp = NULL;
//
//	char                    *template=" select X from /mso_dsa_rule_associate where F1 = V1 ";
//        int64                   db = 1;
//        int32                   s_flags = 256; //distinct
//        //int32                   pp_status = 2; //Active
//        //int32                   serv_status = 10100; //Active
//        //pin_cookie_t            cookie = NULL; //Active
//	int 			count = 0;
//
//        if (PIN_ERRBUF_IS_ERR(ebufp))
//                return;
//        PIN_ERRBUF_CLEAR(ebufp);
//
//        //Get the start and end date
//        dsa_acct_objp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
//        //start_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_START_T, 0, ebufp);
//        //end_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 0, ebufp);
//
//        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_get_dsa_comm_rule input flist", i_flistp);
//
//	s_flistp = PIN_FLIST_CREATE(ebufp);
//	s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
//	PIN_FLIST_FLD_PUT(s_flistp,PIN_FLD_POID, s_pdp, ebufp);
//	PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_FLAGS, (void*)&s_flags, ebufp);
//	PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_TEMPLATE, template, ebufp);
//	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
//	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, args_flistp, MSO_FLD_SALESMAN_OBJ, ebufp);
//	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
//	PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_RULE_ID, 0, ebufp);	
//	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_START_T, NULL, ebufp);	
//	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_END_T, NULL, ebufp);	
//	PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_SALESMAN_OBJ, NULL, ebufp);	
//	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BILL_CYCLE_T, NULL, ebufp);	
//
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_get_dsa_comm_rule search input flist", s_flistp);
//	PCM_OP (ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_flistp, ebufp);
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_get_dsa_comm_rule search output flist", r_flistp);
//	if (PIN_ERRBUF_IS_ERR(ebufp))
//        {
//                PIN_ERR_LOG_MSG(3, "Testing error");
//                return;
//        }
//	count = PIN_FLIST_ELEM_COUNT(r_flistp, PIN_FLD_RESULTS,ebufp);
//        if (count != 0) {
//                *out_flp = r_flistp;
//        } else {
//                *out_flp = i_flistp;
//                PIN_FLIST_FLD_SET(*out_flp, PIN_FLD_DESCR, "No Rules associated for the account", ebufp);
//        }
//	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
//}

//static void 
//fm_mso_commission_dsa_comm_info (
//        pcm_context_t           *ctxp,
//        pin_flist_t             *i_flistp,
//        pin_flist_t             **r_flistpp,
//        pin_errbuf_t            *ebufp) 
//{
//
//	pin_flist_t		*res_flistp = NULL;
//	pin_flist_t		*rule_flp = NULL;
//	//pin_flist_t		*r_serv_flistp = NULL;
//	pin_flist_t		*r_s_flistp1 = NULL;
//	pin_flist_t		*tmp_res_flp1 = NULL;
//	pin_flist_t		*tmp_res_flp = NULL;
//	//pin_flist_t		*res_flp = NULL;
//	//time_t			*cur_time = NULL;
//	char                    *template="select X from /mso_dsa_comm_rule where F1 = V1 ";
//	char                    *template1="select X from /mso_dsa_comm_criteria where F1 = V1 ";
//	//char                    msg[256];
//	pin_flist_t             *s_flistp = NULL;
//	pin_flist_t             *r_s_flistp = NULL;
//	pin_flist_t             *args_flistp = NULL;
//	//pin_flist_t             *r_flistp = NULL;
//	pin_flist_t             *final_flistp = NULL;
//	//pin_flist_t             *tmp1_flp = NULL;
//	poid_t                  *s_pdp = NULL;
//	pin_cookie_t            cookie=NULL;	
//
//	int64                   db = 1;
//	int32                   s_flags = 256; //distinct
//	int32                   elem_id = 0;
//	int32                   count = 0;
//
//	if (PIN_ERRBUF_IS_ERR(ebufp))
//			return;
//	PIN_ERRBUF_CLEAR(ebufp);
//
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_dsa_comm_info input flist", i_flistp);
//
//	final_flistp = PIN_FLIST_CREATE(ebufp);
//	s_flistp = PIN_FLIST_CREATE(ebufp);
//	s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
//	PIN_FLIST_FLD_PUT(s_flistp,PIN_FLD_POID, s_pdp, ebufp);
//	PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_FLAGS, (void*)&s_flags, ebufp);
//	PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_TEMPLATE, template, ebufp);
//	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
//	//res_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
//	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_RULE_ID, args_flistp, MSO_FLD_RULE_ID, ebufp);
//	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
//	//PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_COMM_CRITERIA_ID, 0, ebufp);
//
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_dsa_comm_info search input flist", s_flistp);
//	PCM_OP (ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_s_flistp, ebufp);
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_dsa_comm_info search output flist", r_s_flistp);
//
//	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
//	if (!(PIN_FLIST_ELEM_GET(r_s_flistp, PIN_FLD_RESULTS, 0, 1, ebufp)))
//	{
//		pin_set_err(ebufp, PIN_ERRLOC_FM,
//				PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_BAD_ARG, 0, 0, 0);
//		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
//				"fm_mso_commission_dsa_comm_info rule not found error",ebufp);
//		PIN_FLIST_DESTROY_EX(&r_s_flistp, NULL);
//		return;	
//	}
//
//	count = PIN_FLIST_ELEM_COUNT(r_s_flistp, PIN_FLD_RESULTS,ebufp);
//	if(count > 0)
//	{
//		args_flistp = PIN_FLIST_ELEM_GET(r_s_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
//		PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_START_T, args_flistp, PIN_FLD_START_T, ebufp);
//		PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_END_T, args_flistp, PIN_FLD_END_T, ebufp);
//		PIN_FLIST_FLD_COPY(res_flistp, MSO_FLD_SALESMAN_OBJ, args_flistp, MSO_FLD_SALESMAN_OBJ, ebufp);
//	//	while ((rule_flp = PIN_FLIST_ELEM_GET_NEXT(r_s_flistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL) 
//	//	{
//		PIN_FLIST_ELEM_COPY(r_s_flistp, PIN_FLD_RESULTS, elem_id, final_flistp, PIN_FLD_RESULTS, elem_id, ebufp);
//		PIN_FLIST_FLD_COPY(res_flistp, MSO_FLD_SALESMAN_OBJ, rule_flp, MSO_FLD_SALESMAN_OBJ, ebufp);
//
//		s_flistp = PIN_FLIST_CREATE(ebufp);
//       		s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
//       		PIN_FLIST_FLD_PUT(s_flistp,PIN_FLD_POID, s_pdp, ebufp);
//       		PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_FLAGS, (void*)&s_flags, ebufp);
//       		PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_TEMPLATE, template1, ebufp);
//       		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
//       		PIN_FLIST_FLD_COPY(rule_flp, MSO_FLD_COMM_CRITERIA_ID, args_flistp,MSO_FLD_COMM_CRITERIA_ID , ebufp);
//       		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);		
//		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_RULE_TYPE, NULL, ebufp);
//		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_THRESHOLD_UPPER, 0, ebufp);
//		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_THRESHOLD_LOWER, 0, ebufp);
//		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_dsa_comm_info DSA criteria search input flist", s_flistp);
//		PCM_OP (ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_s_flistp1, ebufp);
//		PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
//		if(PIN_ERRBUF_IS_ERR(ebufp))
//		{
//			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
//				"fm_mso_commission_dsa_comm_info error searching criteria",ebufp);
//			PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
//			PIN_FLIST_DESTROY_EX(&r_s_flistp1, NULL);
//			return;	
//		}
//		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_dsa_comm_info DSA criteria search output flist", r_s_flistp1);
//		count = PIN_FLIST_ELEM_COUNT(r_s_flistp1, PIN_FLD_RESULTS,ebufp);
//		if (count == 0)
//                {
//                        pin_set_err(ebufp, PIN_ERRLOC_FM,
//			PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_BAD_ARG, 0, 0, 0);
//			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
//				"fm_mso_commission_dsa_comm_info critaria for rule not found error",ebufp);
//			return;
//		} 
//		else
//		{
//			while ((rule_flp = PIN_FLIST_ELEM_GET_NEXT(r_s_flistp1, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL) 
//			{
//				tmp_res_flp = PIN_FLIST_ELEM_GET(r_s_flistp1, PIN_FLD_RESULTS, 0, 1, ebufp);	
//				tmp_res_flp1 = PIN_FLIST_ELEM_GET(final_flistp, PIN_FLD_RESULTS, elem_id, 1, ebufp);
//				PIN_FLIST_FLD_COPY(tmp_res_flp, PIN_FLD_RULE_TYPE, tmp_res_flp1, PIN_FLD_RULE_TYPE, ebufp);
//				PIN_FLIST_FLD_COPY(tmp_res_flp, PIN_FLD_THRESHOLD_UPPER, tmp_res_flp1, PIN_FLD_THRESHOLD_UPPER, ebufp);
//				PIN_FLIST_FLD_COPY(tmp_res_flp, PIN_FLD_THRESHOLD_LOWER, tmp_res_flp1, PIN_FLD_THRESHOLD_LOWER, ebufp);
//				PIN_FLIST_FLD_COPY(res_flistp, MSO_FLD_SALESMAN_OBJ, tmp_res_flp1, MSO_FLD_SALESMAN_OBJ, ebufp);
//				PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_START_T, tmp_res_flp1, PIN_FLD_START_T, ebufp);
//				PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_END_T, tmp_res_flp1, PIN_FLD_END_T, ebufp);
//				PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_BILL_CYCLE_T, tmp_res_flp1, PIN_FLD_BILL_CYCLE_T, ebufp);
//			}
//		}
//		PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
//		//}
//	}
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_dsa_comm_info DSA final input flist", final_flistp);
//	*r_flistpp = final_flistp;
//        //PIN_FLIST_DESTROY_EX(&r_s_flistp, NULL);
//}

static void 
fm_mso_commission_calculate_commission (
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             *rule_flistp,
        pin_flist_t             *result_flp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp) {

	//pin_flist_t		*r_flistp = NULL;
	//pin_flist_t		*res_flp = NULL;
	pin_flist_t		*out_flp = NULL;
	//int32			*rating_status = NULL;
	//int32			elem_id = 0;
	//pin_cookie_t            cookie=NULL;
	char			*rule_type = NULL;
	//int 			rule_id = 0;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_calculate_commission input flist", i_flistp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_calculate_commission rule_flistp flist", rule_flistp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_calculate_commission result_flp flist", result_flp);
//	while ((res_flp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
 //       {
		rule_type = PIN_FLIST_FLD_GET(result_flp, PIN_FLD_RULE_TYPE, 0, ebufp);
		//rule_id = *(int *)PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_RULE_ID, 0, ebufp);
		if (strcmp(rule_type,"PLAN_VALUE") == 0 || strcmp(rule_type,"PLAN_VALUE_ANNUM") == 0)
		{
			fm_mso_calculate_percentage_commission(ctxp, i_flistp, rule_flistp, result_flp,
								rule_type, &out_flp, ebufp);	
		}
		else if (strcmp(rule_type,"ORDER_COUNT") == 0)
		{
			fm_mso_calculate_commission_order_count(ctxp, i_flistp, rule_flistp, result_flp, &out_flp, ebufp);
		}
//	}	
	*r_flistpp = out_flp;
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_calculate_commission output flist", *r_flistpp);
}


static void
fm_mso_commission_create_dsa_comm_report (
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp) {

	pin_flist_t		*flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	//pin_flist_t		*tmp_flistp1 = NULL;
	pin_flist_t		*r_flistp = NULL;
	poid_t			*comm_objp = NULL;
	//int32			*comm_rule = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_create_dsa_comm_report input flist", i_flistp);

        /*Collect the required data from the input flist end*/
        comm_objp = PIN_POID_FROM_STR("0.0.0.1 /mso_dsa_comm_rpt/count_based -1", NULL, ebufp);

	flistp = PIN_FLIST_CREATE(ebufp);

        PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, comm_objp, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, flistp, MSO_FLD_SALESMAN_OBJ, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_END_T, flistp, PIN_FLD_EFFECTIVE_T, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_EVENT_OBJ, flistp, PIN_FLD_EVENT_OBJ, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_AMOUNT, flistp, MSO_FLD_COMMISSION_CHARGE, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_STATUS, flistp, PIN_FLD_STATUS, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_STATUS_MSG, flistp, PIN_FLD_STATUS_MSG, ebufp);

        tmp_flistp = PIN_FLIST_SUBSTR_ADD(flistp, PIN_FLD_CUSTOM_INFO, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNT, tmp_flistp, PIN_FLD_COUNT, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_create_report create report input flist", flistp);
        PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, flistp, &r_flistp, ebufp);
        if ( PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_commission_create_dsa_comm_report error in creating object", ebufp);
                goto cleanup;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_create_dsa_comm_report output flist", r_flistp);

        *r_flistpp = r_flistp;
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "r_flistpp is:", *r_flistpp);
cleanup:
        PIN_FLIST_DESTROY_EX(&flistp, NULL);

}

static void
fm_mso_calculate_percentage_commission(
	pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             *rule_flistp,
        pin_flist_t             *result_flp,
	char			*rule_type,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp) 
{
	int32                   s_flags = 768;
	int32                   elem_id = 0;
	int32                   elem_id1 = 0;
	int32                   elem_id2 = 0;
	int32                   *technology = NULL;
	char			*exemptp = NULL;
	//char			*taxcodep = NULL;
	char			*disconnec_flgp = NULL;
	char			*rate_type = NULL;
	char			*template = NULL;
	pin_flist_t             *s_flistp = NULL;
        pin_flist_t             *r_s_flistp = NULL;
        pin_flist_t             *args_flistp = NULL;
	pin_flist_t             *tmp_flp = NULL;
	pin_flist_t             *tmp1_flp = NULL;
	pin_flist_t             *city_flp = NULL;
        //pin_flist_t             *r_flistp = NULL;
        //pin_flist_t             *final_flistp = NULL;
        pin_flist_t             *chargetmp_flp = NULL;
        //pin_flist_t             *chargetmp_flp1 = NULL;
        pin_flist_t             *comm_outflp = NULL;
        pin_flist_t             *rpt_inflp = NULL;
        pin_flist_t             *rpt_outflp = NULL;
        pin_flist_t             *serv_flitp = NULL;
        poid_t                  *s_pdp = NULL;
        int64                   db = 1;
	int 			count = 0;
	pin_decimal_t           *charged_amt = (pin_decimal_t *)NULL;
	pin_decimal_t           *thr_from = (pin_decimal_t *)NULL;
	pin_decimal_t           *thr_to = (pin_decimal_t *)NULL;
	pin_decimal_t           *comm_amount = (pin_decimal_t *)NULL;
	pin_decimal_t           *comm_app_amtp = (pin_decimal_t *)NULL;
	pin_decimal_t           *amt =(pin_decimal_t *) NULL;
	pin_decimal_t           *percent_amt =(pin_decimal_t *) NULL;
        pin_cookie_t            cookie=NULL;	
        pin_cookie_t            cookie1=NULL;	
        pin_cookie_t            cookie2=NULL;	
	poid_t			*serv_pdp = NULL;
	pin_flist_t		*bb_info_flistp = NULL;
	pin_flist_t		*res_flp = NULL;
	pin_flist_t		*cc_flistp = NULL;
	pin_flist_t		*pln_flistp = NULL;
	pin_flist_t		*bb_flistp = NULL;
	pin_flist_t		*ct_flistp = NULL;
	char			*city = NULL;
	char			*city1 = NULL;
	int32			*bill_when = NULL;
	int32			*b_w = NULL;
	int32			prov_status = 1;
	int32			plan_type_subsc = 1;
	double			b_w_d = 0;	

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_calculate_percentage_commission input flist", i_flistp);

	percent_amt = pbo_decimal_from_str("100.0", ebufp);
	charged_amt = pbo_decimal_from_str("0.0", ebufp);
	comm_app_amtp = pbo_decimal_from_str("0.0", ebufp);
	comm_amount = pbo_decimal_from_str("0.0", ebufp);
	thr_from = pbo_decimal_from_str("0.0", ebufp);
	thr_to = pbo_decimal_from_str("0.0", ebufp);
		

	fm_mso_commission_get_end_customers(ctxp, i_flistp, &serv_flitp, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in fm_mso_commission_get_end_customers:", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &prov_status, ebufp); 
		PIN_FLIST_DESTROY_EX(&serv_flitp, NULL);
		return;
	}
	while((res_flp = PIN_FLIST_ELEM_GET_NEXT(serv_flitp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
	{
		serv_pdp = PIN_FLIST_FLD_GET(res_flp, PIN_FLD_POID, 0, ebufp);
		bb_info_flistp = PIN_FLIST_SUBSTR_GET(res_flp, MSO_FLD_BB_INFO, 0, ebufp);
		city = PIN_FLIST_FLD_GET(bb_info_flistp, PIN_FLD_CITY, 0, ebufp);
		bill_when = PIN_FLIST_FLD_GET(bb_info_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);
		technology = PIN_FLIST_FLD_GET(rule_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp);

	
		s_flistp = PIN_FLIST_CREATE(ebufp);
		s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
		PIN_FLIST_FLD_PUT(s_flistp,PIN_FLD_POID, s_pdp, ebufp);
		PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_FLAGS, (void*)&s_flags, ebufp);
		if (*technology != MSO_NO_TECHNOLOGY && strcmp(rule_type, "PLAN_VALUE") != 0)
       		{
			template = "select X from /mso_cfg_credit_limit 1, /profile/customer_care 2, /mso_plans_activation 3, /plan 4, "
				"/service/telco/broadband 5 where 2.F1 = V1 and 2.F2 = 3.F3  and 3.F4 = 4.F5 and 4.F6 = 1.F7 and 5.F8 = 1.F9 "
				"and 3.F10 = V10 and 3.F11 > V11 and 3.F12 <= V12 and 3.F13 = V13 and 5.F14 = 2.F15"
				" and 5.F16 = 1.F17 and 5.F18 = V18 and 5.F19 = V19 and profile_t.poid_type='/profile/customer_care' ";
       		}
       		if (*technology == MSO_NO_TECHNOLOGY && (strcmp(rule_type, "PLAN_VALUE") == 0))
       		{
			template = "select X from /mso_cfg_credit_limit 1, /profile/customer_care 2, /mso_plans_activation 3, /plan 4, "
				"/service/telco/broadband 5 where 2.F1 = V1 and 2.F2 = 3.F3  and 3.F4 = 4.F5 and 4.F6 = 1.F7 and 5.F8 = 1.F9 "
				"and 3.F10 = V10 and 3.F11 > V11 and 3.F12 <= V12 and 3.F13 = V13 and 5.F14 = 2.F15"
				" and 5.F16 = 1.F17 and 5.F18 = V18 and profile_t.poid_type='/profile/customer_care' ";
       		}
		PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_TEMPLATE, template, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
		cc_flistp = PIN_FLIST_SUBSTR_ADD(args_flistp, PIN_FLD_CUSTOMER_CARE_INFO, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, cc_flistp, MSO_FLD_SALESMAN_OBJ, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
		pln_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_PLAN, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(pln_flistp,PIN_FLD_PLAN_OBJ, NULL, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
		PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_POID, NULL, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 6, ebufp);
		PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_NAME, "", ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 7, ebufp);
		PIN_FLIST_FLD_SET(args_flistp,MSO_FLD_PLAN_NAME, "", ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 8, ebufp);
		bb_flistp = PIN_FLIST_SUBSTR_ADD(args_flistp, MSO_FLD_BB_INFO, ebufp);
		PIN_FLIST_FLD_SET(bb_flistp,PIN_FLD_CITY, "", ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 9, ebufp);
		ct_flistp = PIN_FLIST_ELEM_ADD(args_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(ct_flistp,PIN_FLD_CITY, "", ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 10, ebufp);
		PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_STATUS, &prov_status, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 11, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_BILL_CYCLE_T, args_flistp, PIN_FLD_MOD_T, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 12, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_REQ_END_T, args_flistp, PIN_FLD_MOD_T, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 13, ebufp);
		pln_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_PLAN, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(pln_flistp,PIN_FLD_TYPE, &plan_type_subsc, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 14, ebufp);
		PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 15, ebufp);
		PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 16, ebufp);
		bb_flistp = PIN_FLIST_SUBSTR_ADD(args_flistp, MSO_FLD_BB_INFO, ebufp);
		PIN_FLIST_FLD_SET(bb_flistp,PIN_FLD_BILL_WHEN, &prov_status, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 17, ebufp);
		ct_flistp = PIN_FLIST_ELEM_ADD(args_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(ct_flistp,PIN_FLD_BILL_WHEN, &prov_status, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 18, ebufp);
		PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_POID, serv_pdp, ebufp);

		if (technology && *technology != MSO_NO_TECHNOLOGY && strcmp(rule_type, "PLAN_VALUE") != 0)
		{
			args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 19, ebufp);
			bb_flistp = PIN_FLIST_SUBSTR_ADD(args_flistp, MSO_FLD_BB_INFO, ebufp);
			PIN_FLIST_FLD_SET(bb_flistp,MSO_FLD_TECHNOLOGY,technology, ebufp);
		}

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
		tmp_flp = PIN_FLIST_ELEM_ADD(args_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(tmp_flp, PIN_FLD_CHARGE_AMT, NULL, ebufp);
		PIN_FLIST_FLD_SET(tmp_flp, PIN_FLD_CITY, "", ebufp);
		PIN_FLIST_FLD_SET(tmp_flp, PIN_FLD_BILL_WHEN, &prov_status, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_calculate_percentage_commission search plan value input flist",s_flistp);
		PCM_OP (ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_s_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_calculate_percentage_commission search plan value output flist",r_s_flistp);
		count = PIN_FLIST_ELEM_COUNT(r_s_flistp, PIN_FLD_RESULTS,ebufp);
       		if (count == 0)
       		{
       		        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
       		            "fm_mso_commission_dsa_comm_info plan value not found for this service");
			PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&r_s_flistp, NULL);
       		        continue;
       		}
			
       		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_calculate_percentage_commission eligible for commission", r_s_flistp);
		elem_id1 = 0;
		cookie1 = NULL;
		charged_amt = pbo_decimal_from_str("0.0", ebufp);
		comm_app_amtp = pbo_decimal_from_str("0.0", ebufp);
		while ((tmp_flp = PIN_FLIST_ELEM_GET_NEXT(r_s_flistp, PIN_FLD_RESULTS, &elem_id1, 1, &cookie1, ebufp)) != NULL)
		{
       			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_calculate_percentage_commission tmp_flp:", tmp_flp);
			elem_id2 = 0;
			cookie2 = NULL;
			while ((city_flp = PIN_FLIST_ELEM_GET_NEXT(tmp_flp, MSO_FLD_CITIES, &elem_id2, 1, &cookie2, ebufp)) != NULL)
			{
       				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_calculate_percentage_commission city_flp:", city_flp);
				city1 = PIN_FLIST_FLD_GET(city_flp, PIN_FLD_CITY, 0, ebufp);
				b_w = PIN_FLIST_FLD_GET(city_flp, PIN_FLD_BILL_WHEN, 0, ebufp);
				b_w_d = (double) *b_w;
				if(city1 && b_w && strcmp(city1,city) == 0 && *b_w == *bill_when) 
				{
					amt = (pin_decimal_t *)PIN_FLIST_FLD_GET(city_flp, PIN_FLD_CHARGE_AMT, 0, ebufp);
					pbo_decimal_add_assign(charged_amt,amt, ebufp);	
					pbo_decimal_divide_assign(charged_amt, pbo_decimal_from_double(b_w_d, ebufp), ebufp);
				}
			}
		}

		chargetmp_flp = PIN_FLIST_CREATE(ebufp);
		s_pdp = PIN_POID_CREATE(db, "/account", -1, ebufp);
		PIN_FLIST_FLD_PUT(chargetmp_flp,PIN_FLD_POID, s_pdp, ebufp);		
		PIN_FLIST_FLD_SET(chargetmp_flp, PIN_FLD_CHARGE_AMT, charged_amt, ebufp);
		thr_from = (pin_decimal_t *)PIN_FLIST_FLD_GET(result_flp, PIN_FLD_THRESHOLD_LOWER, 0,  ebufp);
		PIN_FLIST_FLD_SET(chargetmp_flp, PIN_FLD_THRESHOLD_LOWER, thr_from, ebufp);
		thr_to = PIN_FLIST_FLD_GET(result_flp,PIN_FLD_THRESHOLD_UPPER, 0,  ebufp);
		PIN_FLIST_FLD_SET(chargetmp_flp, PIN_FLD_THRESHOLD_UPPER, thr_to, ebufp);
		rate_type = PIN_FLIST_FLD_GET(rule_flistp, PIN_FLD_RATE_TYPE, 0, ebufp);
		comm_amount = (pin_decimal_t *)PIN_FLIST_FLD_GET(rule_flistp,PIN_FLD_AMOUNT,  0,  ebufp);
		PIN_FLIST_FLD_SET(chargetmp_flp, PIN_FLD_AMOUNT, comm_amount, ebufp);
		exemptp = PIN_FLIST_FLD_GET(rule_flistp, MSO_FLD_TAX_EXEMPT, 1, ebufp);
		disconnec_flgp = PIN_FLIST_FLD_GET(rule_flistp, MSO_FLD_DISCONNECT_FLAG, 0, ebufp);
		PIN_FLIST_FLD_SET(chargetmp_flp, PIN_FLD_AMOUNT, thr_from, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_START_T, chargetmp_flp, PIN_FLD_START_T, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_END_T, chargetmp_flp, PIN_FLD_END_T, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Initial Charge FListp is", chargetmp_flp);
		if (exemptp && strcmp(exemptp, "Y") != 0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Tax Exempt is not set");
			PIN_FLIST_FLD_COPY(rule_flistp, PIN_FLD_TAX_CODE, chargetmp_flp, PIN_FLD_TAX_CODE, ebufp);
		}
		
		if ((pbo_decimal_compare((pin_decimal_t *)charged_amt, (pin_decimal_t *)thr_from, ebufp) >= 0) && 		
			(pbo_decimal_compare((pin_decimal_t *)charged_amt, (pin_decimal_t *)thr_to, ebufp) < 0))
		{
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, chargetmp_flp, MSO_FLD_SALESMAN_OBJ, ebufp);		
			if (strcmp(rate_type, "P") == 0)
			{
				pbo_decimal_multiply_assign(charged_amt, comm_amount, ebufp);		
				comm_app_amtp = pbo_decimal_divide(charged_amt,percent_amt, ebufp);
				PIN_FLIST_FLD_SET(chargetmp_flp, PIN_FLD_CHARGE_AMT, comm_app_amtp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Charge FListp is", chargetmp_flp);
			}
			else if ((strcmp(rate_type, "F") == 0))
			{
				PIN_FLIST_FLD_SET(chargetmp_flp, PIN_FLD_CHARGE_AMT, comm_amount, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Charge FListp is", chargetmp_flp);
			}
			fm_mso_apply_commission(ctxp, chargetmp_flp, &comm_outflp, ebufp);	
			*r_flistpp = PIN_FLIST_COPY(comm_outflp, ebufp);
			PIN_FLIST_DESTROY_EX(&chargetmp_flp, NULL);
			chargetmp_flp = NULL;
			if (comm_outflp)
       	         	{
	       	                rpt_inflp = PIN_FLIST_CREATE(ebufp);
       		                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Prepare Report input flist");
       	        	        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, rpt_inflp, MSO_FLD_SALESMAN_OBJ, ebufp);
	       	                args_flistp = PIN_FLIST_ELEM_GET(comm_outflp, PIN_FLD_RESULTS, 0, 0, ebufp);
       		                tmp1_flp = PIN_FLIST_ELEM_GET( args_flistp, PIN_FLD_SUB_BAL_IMPACTS, 0, 0, ebufp);
       	      	           	tmp_flp = PIN_FLIST_ELEM_GET(tmp1_flp, PIN_FLD_SUB_BALANCES, 0, 0, ebufp);
       	       	          	PIN_FLIST_FLD_COPY(tmp_flp, PIN_FLD_AMOUNT, rpt_inflp, PIN_FLD_AMOUNT, ebufp);
       	                 	PIN_FLIST_FLD_COPY(args_flistp, PIN_FLD_POID, rpt_inflp, PIN_FLD_POID, ebufp);
       	                 	PIN_FLIST_FLD_COPY(args_flistp, PIN_FLD_POID, rpt_inflp, PIN_FLD_EVENT_OBJ, ebufp);
       	                 	PIN_FLIST_FLD_COPY(comm_outflp, PIN_FLD_STATUS, rpt_inflp, PIN_FLD_STATUS, ebufp);
       	                 	PIN_FLIST_FLD_COPY(args_flistp, PIN_FLD_ACCOUNT_OBJ, rpt_inflp, PIN_FLD_ACCOUNT_OBJ, ebufp);
       	                 	PIN_FLIST_FLD_COPY(tmp_flp, PIN_FLD_VALID_FROM, rpt_inflp, PIN_FLD_END_T, ebufp);
       	                 	PIN_FLIST_FLD_SET( rpt_inflp ,PIN_FLD_STATUS_MSG,"Completed",  ebufp);
       	                 	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Input flist for creating report",rpt_inflp);
       	                 	fm_mso_commission_create_dsa_comm_report(ctxp, rpt_inflp, &rpt_outflp, ebufp);
       	                 	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Output flist after creating report",rpt_inflp);
       	         	}
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Not eligible for commission");
		}
	}
	if(*r_flistpp == NULL)
	{
		prov_status = 0;
		*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &prov_status, ebufp); 
	}

	CLEANUP:
        PIN_FLIST_DESTROY_EX(&serv_flitp, NULL);
        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&r_s_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&chargetmp_flp, NULL);
        PIN_FLIST_DESTROY_EX(&comm_outflp, NULL);
        PIN_FLIST_DESTROY_EX(&rpt_inflp, NULL);
        PIN_FLIST_DESTROY_EX(&rpt_outflp, NULL);
	if(!pbo_decimal_is_null(percent_amt, ebufp))
		pbo_decimal_destroy(&percent_amt); 

	if(!pbo_decimal_is_null(charged_amt, ebufp))
		pbo_decimal_destroy(&charged_amt); 

	if(!pbo_decimal_is_null(comm_app_amtp, ebufp))
		pbo_decimal_destroy(&comm_app_amtp); 

	return;
}

static int 
fm_mso_apply_commission (
	pcm_context_t		*ctxp, 
	pin_flist_t		*i_flistp, 
	pin_flist_t		**out_flp, 
	pin_errbuf_t		*ebufp)
{
	pin_flist_t             *actusg_iflistp = NULL;
	pin_flist_t             *actusg_oflistp = NULL;
	pin_flist_t             *r_flistp = NULL;
	pin_flist_t             *event_flp = NULL;
	pin_flist_t             *bal_flp = NULL;
        pin_flist_t             *r_s_flistp = NULL;
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	poid_t			*e_pdp = NULL;
        poid_t                  *s_pdp = NULL;
	int64			db = 1;
	int32                   resource_id=356;
	int32                   glid=1;
	int32                   usage_success = 0;
        int32                   usage_failure = 0;
	int32                   s_flags = 256;
	int 			count = 0;
	u_int                   impact_type=2;
	pin_decimal_t           *one_decp = NULL;
	char                    *tax_code = NULL;
	char			*template = "select X from /service/settlement/ws/dsa where F1=V1 and poid_type = '/service/settlement/ws/dsa' ";

	one_decp = pbo_decimal_from_str("1",ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_apply_commission input flist", i_flistp);
	actusg_iflistp = PIN_FLIST_CREATE(ebufp);
	s_flistp = PIN_FLIST_CREATE(ebufp);
	//e_pdp = PIN_POID_CREATE(db, "/event/billing/settlement/ws/dsa_commission", -1, ebufp);	
	s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
        PIN_FLIST_FLD_PUT(s_flistp,PIN_FLD_POID, s_pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_FLAGS, (void*)&s_flags, ebufp);
        PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_TEMPLATE, template, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_SALESMAN_OBJ, args_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_apply_commission search service input flist",s_flistp);
        PCM_OP (ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_s_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_apply_commission search service output flist",r_s_flistp);
	count = PIN_FLIST_ELEM_COUNT(r_s_flistp, PIN_FLD_RESULTS,ebufp);
        if (count == 0)
        {
               PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
                              "fm_mso_apply_commission plan value not found");
               *out_flp = i_flistp;
               PIN_FLIST_FLD_SET(*out_flp, PIN_FLD_DESCR, "wholesale service not found", ebufp);
               return;
        }
	tax_code = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_TAX_CODE, 1, ebufp);
	args_flistp = PIN_FLIST_ELEM_GET(r_s_flistp, PIN_FLD_RESULTS, 0, 0,ebufp);	
	e_pdp = PIN_POID_CREATE(db, "/event/billing/settlement/ws/dsa_commission", -1, ebufp);	
	PIN_FLIST_FLD_SET(actusg_iflistp,PIN_FLD_POID, e_pdp, ebufp);
	event_flp = PIN_FLIST_SUBSTR_ADD(actusg_iflistp, PIN_FLD_EVENT, ebufp);
	PIN_FLIST_FLD_SET(event_flp, PIN_FLD_POID, e_pdp, ebufp);	
	PIN_FLIST_FLD_COPY(args_flistp, PIN_FLD_POID, event_flp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_SALESMAN_OBJ, event_flp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	//PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_START_T, event_flp, PIN_FLD_START_T, ebufp);
	//PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_END_T, event_flp, PIN_FLD_END_T, ebufp);
	PIN_FLIST_FLD_SET(event_flp, PIN_FLD_START_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(event_flp, PIN_FLD_END_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(event_flp, PIN_FLD_NAME, "DSA Balance Impact", ebufp);	
	PIN_FLIST_FLD_SET(event_flp, PIN_FLD_PROGRAM_NAME, "DSA Commission", ebufp);	
	bal_flp = PIN_FLIST_ELEM_ADD(event_flp, PIN_FLD_BAL_IMPACTS, 0, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_SALESMAN_OBJ, bal_flp, PIN_FLD_ACCOUNT_OBJ, ebufp);	
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_CHARGE_AMT, bal_flp, PIN_FLD_AMOUNT, ebufp);	
	PIN_FLIST_FLD_SET(bal_flp, PIN_FLD_IMPACT_TYPE, (void *)&impact_type, ebufp);	
	PIN_FLIST_FLD_SET(bal_flp, PIN_FLD_PERCENT, (void *)one_decp, ebufp);	
	PIN_FLIST_FLD_SET(bal_flp, PIN_FLD_QUANTITY, (void *)one_decp, ebufp);	
	PIN_FLIST_FLD_SET(bal_flp, PIN_FLD_RESOURCE_ID,(void *)&resource_id, ebufp);	
	PIN_FLIST_FLD_SET(bal_flp, PIN_FLD_GL_ID,( void *)&glid, ebufp);
	if(tax_code)
	{
		PIN_FLIST_FLD_SET(bal_flp, PIN_FLD_TAX_CODE, &tax_code, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(bal_flp, PIN_FLD_TAX_CODE, "Dummy", ebufp);	
	}
	PIN_FLIST_FLD_SET(bal_flp, PIN_FLD_RATE_TAG, "Dummy", ebufp);	

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_apply_commission act_usage input flist",actusg_iflistp);
	PCM_OP(ctxp, PCM_OP_ACT_USAGE, 0,actusg_iflistp, &actusg_oflistp, ebufp);
	r_flistp = PIN_FLIST_CREATE(ebufp);
        if ( PIN_ERRBUF_IS_ERR(ebufp)) {
             PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
             PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &usage_failure, ebufp);
             PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR, "Error in usage", ebufp);
        }
        else {
             PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &usage_success, ebufp);
             PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
	     bal_flp = PIN_FLIST_ELEM_GET(actusg_oflistp, PIN_FLD_RESULTS, 0, 0, ebufp);
	     PIN_FLIST_ELEM_COPY(actusg_oflistp, PIN_FLD_RESULTS, 0, r_flistp,PIN_FLD_RESULTS, 0, ebufp);
             PIN_FLIST_FLD_COPY(bal_flp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
             PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR, "Usage Success", ebufp);
        }

        *out_flp = r_flistp;
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_apply_commission output flist", *out_flp);
        //PIN_FLIST_DESTROY_EX(&flistp, NULL);
        //PIN_FLIST_DESTROY_EX(&actusg_iflistp, NULL);
        //PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
        return;
}

static void
fm_mso_calculate_commission_order_count(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             *rule_flistp,
        pin_flist_t             *result_flp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
	int32                   s_flags = 256;
	//int32                   elem_id = 0;
	//int32                   elem_id1 = 0;
	char                    *exemptp = NULL;
	//char                    *taxcodep = NULL;
	char                    *disconnec_flgp = NULL;
	char                    *rate_type = NULL;
	//char                    count_str[50];
	//char                    tmp_str[50];
	char                    *template_str = NULL;
	int32                   serv_status = 10103;
	pin_flist_t             *s_flistp = NULL;
	pin_flist_t             *r_s_flistp = NULL;
	pin_flist_t             *args_flistp = NULL;
	pin_flist_t             *tmp_flp = NULL;
	//pin_flist_t             *city_flp = NULL;
	//pin_flist_t             *r_flistp = NULL;
	pin_flist_t             *tmp1_flp = NULL;
	pin_flist_t             *chargetmp_flp = NULL;
	//pin_flist_t             *chargetmp_flp1 = NULL;
	pin_flist_t             *comm_outflp = NULL;
	pin_flist_t             *rpt_inflp = NULL;
	pin_flist_t             *rpt_outflp = NULL;
	pin_flist_t             *substr_flistp = NULL;
	poid_t                  *s_pdp = NULL;
	int64                   db = 1;
	int32 		        count = 0;
	//int32 		        *ptr_count = 0;
	pin_decimal_t           *count_dec = (pin_decimal_t *)NULL;
	pin_decimal_t           *thr_from = (pin_decimal_t *)NULL;
	pin_decimal_t           *thr_to = (pin_decimal_t *)NULL;
	pin_decimal_t           *comm_amount = (pin_decimal_t *)NULL;
//	pin_decimal_t           *amt =(pin_decimal_t *) NULL;
	//pin_cookie_t            cookie=NULL;
	//pin_cookie_t            cookie1=NULL;
	int32			status_fail = 1;
	int32			status_pass = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	comm_amount = pbo_decimal_from_str("0.0", ebufp);
	thr_from = pbo_decimal_from_str("0.0", ebufp);
	thr_to = pbo_decimal_from_str("0.0", ebufp);
	count_dec = pbo_decimal_from_str("0.0", ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_calculate_commission_order_count input flist", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_calculate_commission_order_count rule flist", rule_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_calculate_commission_order_count criteria flist", result_flp);
	disconnec_flgp = PIN_FLIST_FLD_GET(rule_flistp, MSO_FLD_DISCONNECT_FLAG, 0, ebufp);
	if (strcmp(disconnec_flgp, "Y") == 0)
	{
		template_str = "select X from /profile/customer_care 1, /service/telco/broadband 2 where "
		"1.F1 = V1 and 2.F2 = 1.F3 and 2.F4 > V4 and 2.F5 <= V5 and profile_t.poid_type='/profile/customer_care' ";
	}
	else if(strcmp(disconnec_flgp, "N") == 0)
	{
		template_str = "select X from /profile/customer_care 1, /service/telco/broadband 2 where "
		"1.F1 = V1 and 2.F2 = 1.F3 and 2.F4 > V4 and 2.F5 <= V5 and 2.F6 <> V6 and profile_t.poid_type='/profile/customer_care' ";
	}
	s_flistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp,PIN_FLD_POID, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_FLAGS, (void*)&s_flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_TEMPLATE, template_str, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	tmp_flp = PIN_FLIST_SUBSTR_ADD(args_flistp, PIN_FLD_CUSTOMER_CARE_INFO, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, tmp_flp, MSO_FLD_SALESMAN_OBJ, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_PUT(args_flistp,PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_PUT(args_flistp,PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
	substr_flistp = PIN_FLIST_SUBSTR_ADD(args_flistp, MSO_FLD_BB_INFO, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_BILL_CYCLE_T, substr_flistp, PIN_FLD_START_T, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
	substr_flistp = PIN_FLIST_SUBSTR_ADD(args_flistp, MSO_FLD_BB_INFO, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_REQ_END_T, substr_flistp, PIN_FLD_START_T, ebufp);

	if(strcmp(disconnec_flgp, "N") == 0)
	{
		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 6, ebufp);
		PIN_FLIST_FLD_PUT(args_flistp,PIN_FLD_STATUS, &serv_status, ebufp);
	}

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_calculate_commission_order_count search plan value input flist",s_flistp);
	PCM_OP (ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_s_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
		"fm_mso_calculate_commission_order_count error");
		PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&r_s_flistp, NULL);
		*r_flistpp = PIN_FLIST_COPY(i_flistp,ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_DESCR, "Error in getting order count", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_fail, ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_calculate_commission_order_count search plan value output flist", r_s_flistp);

	//count = PIN_FLIST_ELEM_COUNT(r_s_flistp, PIN_FLD_RESULTS,ebufp);
	args_flistp = PIN_FLIST_ELEM_GET(r_s_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if (!args_flistp)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
				"fm_mso_calculate_commission_order_count No activated services for the period");
		PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&r_s_flistp, NULL);
		*r_flistpp = PIN_FLIST_COPY(i_flistp,ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_DESCR, "No activated services for the period", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_pass, ebufp);
		return;
	}


	chargetmp_flp = PIN_FLIST_CREATE(ebufp);
	s_pdp = PIN_POID_CREATE(db, "/account", -1, ebufp);
	PIN_FLIST_FLD_PUT(chargetmp_flp,PIN_FLD_POID, s_pdp, ebufp);
	thr_from = (pin_decimal_t *)PIN_FLIST_FLD_GET(result_flp, PIN_FLD_THRESHOLD_LOWER, 0,  ebufp);
	PIN_FLIST_FLD_SET(chargetmp_flp, PIN_FLD_THRESHOLD_LOWER, thr_from, ebufp);
	thr_to = PIN_FLIST_FLD_GET(result_flp,PIN_FLD_THRESHOLD_UPPER, 0,  ebufp);
	PIN_FLIST_FLD_SET(chargetmp_flp, PIN_FLD_THRESHOLD_UPPER, thr_to, ebufp);
	rate_type = PIN_FLIST_FLD_GET(rule_flistp, PIN_FLD_RATE_TYPE, 0, ebufp);
	comm_amount = (pin_decimal_t *)PIN_FLIST_FLD_GET(rule_flistp,PIN_FLD_AMOUNT,  0,  ebufp);
	exemptp = PIN_FLIST_FLD_GET(rule_flistp, MSO_FLD_TAX_EXEMPT, 0, ebufp);
	PIN_FLIST_FLD_SET(chargetmp_flp, PIN_FLD_AMOUNT, thr_from, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_START_T, chargetmp_flp, PIN_FLD_START_T, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_END_T, chargetmp_flp, PIN_FLD_END_T, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Initial Charge FListp is", chargetmp_flp);
	if (exemptp && strcmp(exemptp, "Y") != 0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Tax Exempt is not set");
		PIN_FLIST_FLD_COPY(rule_flistp, PIN_FLD_TAX_CODE, chargetmp_flp, PIN_FLD_TAX_CODE, ebufp);
	}
	count = PIN_FLIST_ELEM_COUNT(r_s_flistp, PIN_FLD_RESULTS,ebufp);
	count_dec = pbo_decimal_from_str((char *)itoa(count), ebufp);

	if ((pbo_decimal_compare((pin_decimal_t *)count_dec, (pin_decimal_t *)thr_from, ebufp) >= 0) &&
			(pbo_decimal_compare((pin_decimal_t *)count_dec, (pin_decimal_t *)thr_to, ebufp) < 0))
	{
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, chargetmp_flp, MSO_FLD_SALESMAN_OBJ, ebufp);
		PIN_FLIST_FLD_SET(chargetmp_flp, PIN_FLD_CHARGE_AMT, comm_amount, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Charge FListp is", chargetmp_flp);
		fm_mso_apply_commission(ctxp, chargetmp_flp, &comm_outflp, ebufp);
		*r_flistpp = PIN_FLIST_COPY(comm_outflp, ebufp);
		if (comm_outflp)
		{
			rpt_inflp = PIN_FLIST_CREATE(ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Prepare Report input flist");
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_SALESMAN_OBJ, rpt_inflp, MSO_FLD_SALESMAN_OBJ, ebufp);			
			args_flistp = PIN_FLIST_ELEM_GET(comm_outflp, PIN_FLD_RESULTS, 0, 0, ebufp);	
			tmp1_flp = PIN_FLIST_ELEM_GET( args_flistp, PIN_FLD_SUB_BAL_IMPACTS, 0, 0, ebufp);
			tmp_flp = PIN_FLIST_ELEM_GET(tmp1_flp, PIN_FLD_SUB_BALANCES, 0, 0, ebufp);					
			PIN_FLIST_FLD_COPY(tmp_flp, PIN_FLD_AMOUNT, rpt_inflp, PIN_FLD_AMOUNT, ebufp);
			PIN_FLIST_FLD_COPY(args_flistp, PIN_FLD_POID, rpt_inflp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(args_flistp, PIN_FLD_POID, rpt_inflp, PIN_FLD_EVENT_OBJ, ebufp);
			PIN_FLIST_FLD_SET( rpt_inflp ,PIN_FLD_COUNT, &count,  ebufp);
			PIN_FLIST_FLD_COPY(comm_outflp, PIN_FLD_STATUS, rpt_inflp, PIN_FLD_STATUS, ebufp);
			PIN_FLIST_FLD_COPY(args_flistp, PIN_FLD_ACCOUNT_OBJ, rpt_inflp, PIN_FLD_ACCOUNT_OBJ, ebufp);
			PIN_FLIST_FLD_COPY(tmp_flp, PIN_FLD_VALID_FROM, rpt_inflp, PIN_FLD_END_T, ebufp);
			PIN_FLIST_FLD_SET( rpt_inflp ,PIN_FLD_STATUS_MSG,"Completed",  ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Input flist for creating report",rpt_inflp);
			fm_mso_commission_create_dsa_comm_report(ctxp, rpt_inflp, &rpt_outflp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Output flist after creating report",rpt_inflp);
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Not eligible for commission");
	}

	if(*r_flistpp == NULL)
	{
		*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
	}

	CLEANUP:
	PIN_ERR_LOG_MSG(3,"DEBUG0");
        PIN_FLIST_DESTROY_EX(&chargetmp_flp, NULL);
        PIN_FLIST_DESTROY_EX(&comm_outflp, NULL);
        PIN_FLIST_DESTROY_EX(&rpt_inflp, NULL);
        PIN_FLIST_DESTROY_EX(&rpt_outflp, NULL);
        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&r_s_flistp, NULL);

	PIN_ERR_LOG_MSG(3,"DEBUG1");
	if(!pbo_decimal_is_null(count_dec, ebufp))
		pbo_decimal_destroy(&count_dec); 

	PIN_ERR_LOG_MSG(3,"DEBUG2");
	return;
}

char *itoa(int i) {
  char * res = malloc(8*sizeof(int));
  sprintf(res, "%d", i);
  PIN_ERR_LOG_MSG(3, "itoa variable");
  PIN_ERR_LOG_MSG(3, res);
  return res;
}

static void 
fm_mso_commission_get_dsa_rule(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t             **r_flistpp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t		*srch_iflistp = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	poid_t			*s_pdp = NULL;
	int32			s_flags = 256;
	int64			db = 1;
	char			*s_template = "select X from /mso_dsa_comm_rule where F1 = V1 ";
	//int				count = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_get_dsa_rule input flist", i_flistp);

	s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_iflistp = PIN_FLIST_CREATE(ebufp);

	PIN_FLIST_FLD_PUT(srch_iflistp,PIN_FLD_POID, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp,PIN_FLD_FLAGS, (void*)&s_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp,PIN_FLD_TEMPLATE, s_template, ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_RULE_ID, arg_flistp, MSO_FLD_RULE_ID, ebufp);

	res_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_get_dsa_rule search input flist", srch_iflistp);
	PCM_OP (ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_get_dsa_rule search output flist", srch_oflistp);

	PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"fm_mso_commission_get_dsa_rule error in searching dsa rule",ebufp);

		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
		return;
	
	}
	if (!(PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, 0, 1, ebufp)))
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_BAD_ARG, 0, 0, 0);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_commission_get_dsa_rule rule not found error",ebufp);
		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
		return;
	}
	*r_flistpp = PIN_FLIST_COPY(srch_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_get_dsa_rule output flist", *r_flistpp);
	PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);

	return;
}

static void 
fm_mso_commission_get_dsa_criteria(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	int32                   *criteria_id,
	pin_flist_t             **r_flistpp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t		*srch_iflistp = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	poid_t			*s_pdp = NULL;
	int32			s_flags = 256;
	int64			db = 1;
	char			*s_template = "select X from /mso_dsa_comm_criteria where F1 = V1 ";
	//int				count = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_get_dsa_criteria input flist", i_flistp);

	s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_iflistp = PIN_FLIST_CREATE(ebufp);

	PIN_FLIST_FLD_PUT(srch_iflistp,PIN_FLD_POID, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp,PIN_FLD_FLAGS, (void*)&s_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp,PIN_FLD_TEMPLATE, s_template, ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp,MSO_FLD_COMM_CRITERIA_ID, criteria_id, ebufp);

	res_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_RULE_TYPE, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_THRESHOLD_UPPER, 0, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_THRESHOLD_LOWER, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_get_dsa_criteria search input flist", srch_iflistp);
	PCM_OP (ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_get_dsa_criteria search output flist", srch_oflistp);

	PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"fm_mso_commission_get_dsa_criteria error in searching dsa criteria",ebufp);

		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
		return;
	
	}
	if (!(PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, 0, 1, ebufp)))
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_BAD_ARG, 0, 0, 0);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_commission_get_dsa_criteria rule not found error",ebufp);
		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
		return;
	}
	*r_flistpp = PIN_FLIST_COPY(srch_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_get_dsa_criteria output flist", *r_flistpp);
	PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);

	return;
}

static void 
fm_mso_commission_get_end_customers(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t             **r_flistpp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t		*srch_iflistp = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*cust_care_flistp = NULL;
	pin_flist_t		*bb_info_flistp = NULL;
	poid_t			*s_pdp = NULL;
	int32			s_flags = 256;
	int64			db = 1;
	char			*s_template = "select X from /service/telco/broadband 1, /profile/customer_care 2 where "
						"1.F1 = 2.F2 and 2.F3 = V3 and profile_t.poid_type='/profile/customer_care' ";
	//int				count = 0;
	int32			bill_when = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_get_end_customers input flist", i_flistp);

	s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_iflistp = PIN_FLIST_CREATE(ebufp);

	PIN_FLIST_FLD_PUT(srch_iflistp,PIN_FLD_POID, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp,PIN_FLD_FLAGS, (void*)&s_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp,PIN_FLD_TEMPLATE, s_template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 3, ebufp);
	cust_care_flistp = PIN_FLIST_SUBSTR_ADD(args_flistp, PIN_FLD_CUSTOMER_CARE_INFO, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, cust_care_flistp, MSO_FLD_SALESMAN_OBJ, ebufp);

	res_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(res_flistp,PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
	bb_info_flistp = PIN_FLIST_SUBSTR_ADD(res_flistp, MSO_FLD_BB_INFO, ebufp);
	PIN_FLIST_FLD_SET(bb_info_flistp,PIN_FLD_CITY, "", ebufp);
	PIN_FLIST_FLD_SET(bb_info_flistp,PIN_FLD_BILL_WHEN, &bill_when, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_get_end_customers search input flist", srch_iflistp);
	PCM_OP (ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_get_end_customers search output flist", srch_oflistp);

	PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"fm_mso_commission_get_end_customers error in searching customer profile",ebufp);

		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
		return;
	
	}
	if (!(PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, 0, 1, ebufp)))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
			"fm_mso_commission_get_end_customers: not found !!");
		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
		return;
	}
	*r_flistpp = PIN_FLIST_COPY(srch_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);

	return;
}

