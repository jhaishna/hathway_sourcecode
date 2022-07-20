/*******************************************************************
 * File:	fm_mso_utils_add_hint_billing.c
 * Opcode:	MSO_OP_UTILS_ADD_HINT_BILLING 
 * Owner:	Gautam Adak
 * Created:	17-SEPT-2013
 * Description: 
 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_utils_add_hint_billing.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "ops/act.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"



/**************************************
* External Functions start
***************************************/
/**************************************
* External Functions end
***************************************/

/*******************************************************************
 * MSO_OP_UTILS_ADD_HINT_BILLING 
 *
 * 
 *******************************************************************/

EXPORT_OP void
op_mso_utils_add_hint_billing(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_utils_add_hint_billing(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);


#define PIN_CYCLE_MODE_DEFER_CANCEL             3
#define PIN_CYCLE_MODE_REG_CYCLE                4

/*******************************************************************
 * MSO_OP_UTILS_ADD_HINT_BILLING  
 *******************************************************************/
void 
op_mso_utils_add_hint_billing(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	*r_flistpp		= NULL;
	poid_t			*inp_pdp = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_UTILS_ADD_HINT_BILLING) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_utils_add_hint_billing error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_utils_add_hint_billing input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	fm_mso_utils_add_hint_billing(ctxp, flags, i_flistp, r_flistpp, ebufp);
 	/***********************************************************
	 * Results.
	 ***********************************************************/

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_utils_add_hint_billing error", ebufp);
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_utils_add_hint_billing output flist", *r_flistpp);
	}
	return;
}



/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_utils_add_hint_billing(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{

	pin_flist_t     *app_flistp = NULL;
	pin_flist_t     *srch_flistp = NULL;
	pin_flist_t	*arg_flistp  = NULL;
	pin_flist_t	*mso_service_flistp = NULL;
	pin_flist_t	*usage_map_flistp = NULL;


	char		*prog_name = NULL;
	char		*template = NULL;
	char		*bill_acts_srch_str = NULL;
	char		*mod_str = NULL;
	char		*srch_str = NULL;
	char		*account_no = NULL;
	char		*final_str = NULL;
	char		*str_1 = NULL;
	char		*str_2 = NULL;
	
	int32		*acnt_poid_id_ptr = NULL;
	int32		*bill_segment_ptr = NULL;
	int32		acnt_poid_id = -1;
	int32		bill_segment = -1;
	int32		flg_oob = 1;
	int32		mode = 0;
	int64		database = 1;
	int32		*srvc_indicator_ptr = NULL;
	int32		srvc_indicator = -1;




	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_add_hint_billing input flist ", in_flistp);	

	app_flistp = PIN_FLIST_SUBSTR_GET(in_flistp, PIN_FLD_APPLICATION_INFO, 1, ebufp);
	if (app_flistp && app_flistp != NULL)
	{
		prog_name = (char *)PIN_FLIST_FLD_GET(app_flistp, PIN_FLD_NAME, 0, ebufp);
	}

	srch_flistp = PIN_FLIST_SUBSTR_GET(in_flistp, PIN_FLD_SEARCH_FLIST, 0, ebufp);
	template = PIN_FLIST_FLD_GET(srch_flistp, PIN_FLD_TEMPLATE, 1, ebufp);
	if (app_flistp && app_flistp != NULL)
	{
		if (template && template != NULL)
		{
			srch_str = strstr(template, "from");
			if(prog_name && strcmp(prog_name, "pin_bill_accts") == 0)
			{
				mod_str = pin_malloc(512);
				memset(mod_str,'\0',sizeof(mod_str));
				strcpy(mod_str, "select /*+ parallel(billinfo_t, 32) parallel(account_t, 32) */ X ");
				strcat(mod_str, srch_str);

				PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, mod_str, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch_flistp pin_bill_accts", srch_flistp);		

				if(mod_str)
				{
					pin_free(mod_str);
				}
			}
			if(prog_name && strstr(prog_name, "pin_deferred_act"))
			{
				mod_str = pin_malloc(512);
				memset(mod_str,'\0',sizeof(mod_str));
				strcpy(mod_str, "select /*+ parallel(schdule_t, 32) */ X ");
				strcat(mod_str, srch_str);

				PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, mod_str, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch_flistp pin_deferred_act", srch_flistp);	
				if(mod_str)
				{
					pin_free(mod_str);
				}
			}
			if(prog_name && strcmp(prog_name, "pin_cycle_fees") == 0)
			{	
				mod_str	= pin_malloc(512);
				memset(mod_str,'\0',sizeof(mod_str));

				/*============================================
				Account poid id passed
				=============================================*/
				acnt_poid_id_ptr = (int32*)PIN_FLIST_FLD_GET(app_flistp, PIN_FLD_ACCOUNT_NUM, 1, ebufp); 
				if (acnt_poid_id_ptr )
				{
					acnt_poid_id = *acnt_poid_id_ptr; 
				}
				if (acnt_poid_id != -1)
				{
					flg_oob = 0;
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "account poid");
//					strcpy(mod_str, "select /*+ full(account_t) parallel(account_t, 32) parallel(purchased_product_t, 32) */ X ");
//					strcat(mod_str, srch_str);
					
//					final_str = pin_malloc(512);
//					memset(final_str,'\0',sizeof(final_str));
					
//					str_1 = strtok(mod_str, "order");
//					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str_1 );
//					strcat(final_str, str_1);

//					strcat(final_str, " and 2.F9 = V9 order ");

//					str_2 = strtok(NULL, "order");
//					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str_2 );

//					strcat(final_str, str_2);

//					PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, final_str, ebufp);
					mode = *(int32 *)PIN_FLIST_FLD_GET(app_flistp, PIN_FLD_MODE, 0, ebufp);
					if(mode == PIN_CYCLE_MODE_REG_CYCLE) { // Called by -regular_cycle_fees

					arg_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 13, ebufp);
					PIN_FLIST_FLD_PUT(arg_flistp, PIN_FLD_ACCOUNT_OBJ, 
					         (PIN_POID_CREATE(database, "/account", (int64)acnt_poid_id, ebufp))
					, ebufp);

					strcpy(mod_str, "select X from /account 1, /purchased_product 2 where account_t.poid_id0 > 1  and 2.F1 = V1 and ( 2.F2 <= V2 or ( 2.F3 > 2.F4 and 2.F5 <= V5 )) and ( 2.F6 > V6 or 2.F7 < 2.F8 or 2.F9 = V9 ) and ( 1.F10 = 2.F11 ) and ( 2.F12 = V12 ) and 2.F13 = V13 order by account_t.poid_id0 asc ");
					PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, mod_str, ebufp);

					} 
					else  if(mode == PIN_CYCLE_MODE_DEFER_CANCEL){ // Called by defer_cancel option

					arg_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 9, ebufp);
					PIN_FLIST_FLD_PUT(arg_flistp, PIN_FLD_ACCOUNT_OBJ, 
					         (PIN_POID_CREATE(database, "/account", (int64)acnt_poid_id, ebufp))
					, ebufp);

					strcpy(mod_str, "select X from /account 1, /purchased_product 2 where account_t.poid_id0 > 1 and 1.F1 = V1 and  1.F2 = 2.F3  and 2.F4 = V4 and 2.F5 = V5 and   2.F6 <= V6 and 2.F7 > V7 and 2.F8 >= V8 and 2.F9 = V9 order by account_t.poid_id0 asc ");
					PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, mod_str, ebufp);
					}
				
				}

				/*============================================
				Account no passed
				=============================================*/
				account_no = (char*)PIN_FLIST_FLD_GET(app_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp);
				if (account_no )
				{
					flg_oob = 0;
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "account no");	

					arg_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 9, ebufp);
					PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACCOUNT_NO,(char*)account_no , ebufp);

					strcpy(mod_str, "select X from /account 1, /purchased_product 2 where account_t.poid_id0 > 1 and 1.F1 = V1 and  1.F2 = 2.F3  and 2.F4 = V4 and 2.F5 = V5 and   2.F6 <= V6 and 2.F7 > V7 and 2.F8 >= V8 and 1.F9 = V9 order by account_t.poid_id0 asc ");
					PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, mod_str, ebufp);
				}
				/*============================================
				Billing Segment passed
				=============================================*/
				bill_segment_ptr = (int32*)PIN_FLIST_FLD_GET(app_flistp, PIN_FLD_BILLING_SEGMENT, 1, ebufp); 
				if (bill_segment_ptr )
				{
					bill_segment = *bill_segment_ptr; 
				}
				if (bill_segment != -1)
				{
					//Added on 19-MAY-17 for regular cycle fees option 
					mode = *(int32 *)PIN_FLIST_FLD_GET(app_flistp, PIN_FLD_MODE, 0, ebufp);
                                        if(mode == PIN_CYCLE_MODE_REG_CYCLE) {	
						flg_oob = 0;
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Bill segment with regular fees option");
						arg_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 13, ebufp);
						PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
						
						arg_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 14, ebufp);
						PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
					
						arg_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 15, ebufp);
						PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_BILLING_SEGMENT, &bill_segment , ebufp);
						
						strcpy(mod_str, "select /*+ parallel(8) full(account_t) full(purchased_product_t) full(purchased_product_cycle_fees_t) full(billinfo_t) */ X from /account 1, /purchased_product 2, /billinfo 3  where account_t.poid_id0 > 1  and 2.F1 = V1 and ( 2.F2 <= V2 or ( 2.F3 > 2.F4 and 2.F5 <= V5 )) and ( 2.F6 > V6 or 2.F7 < 2.F8 or 2.F9 = V9 ) and ( 1.F10 = 2.F11 ) and ( 2.F12 = V12 ) and 2.F13 = 3.F14 and 3.F15 = V15 order by account_t.poid_id0 asc ");
						
						PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, mod_str, ebufp);
					}
					else
					{
						flg_oob = 0;
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " bill_segment");
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "account no");	

						arg_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 9, ebufp);
						PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, NULL , ebufp);

						arg_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 10, ebufp);
						PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACCOUNT_OBJ, NULL , ebufp);

						arg_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 11, ebufp);
						PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_BILLING_SEGMENT, &bill_segment , ebufp);

						strcpy(mod_str, "select /*+ full(account_t) parallel(account_t, 32) parallel(purchased_product_t, 32) parallel(billinfo_t, 32) */ X from /account 1, /purchased_product 2, /billinfo 3 where account_t.poid_id0 > 1 and 1.F1 = V1 and  1.F2 = 2.F3  and 2.F4 = V4 and 2.F5 = V5 and  2.F6 <= V6 and 2.F7 > V7 and 2.F8 >= V8 and 1.F9 = 3.F10 and 3.F11 = V11 order by account_t.poid_id0 asc ");
						PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, mod_str, ebufp);
					}
				}
				
				srvc_indicator_ptr = (int32*)PIN_FLIST_FLD_GET(app_flistp, PIN_FLD_INDICATOR, 1, ebufp); 
				if (srvc_indicator_ptr )
				{
					srvc_indicator = *srvc_indicator_ptr; 
				}
				if (srvc_indicator != -1)
				{
					flg_oob = 0;
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Quota reset option ");

					if (srvc_indicator ==3)
					{
						PIN_ERR_LOG_MSG(3, "Regular cycle_fees for both accounts ");

						arg_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 13, ebufp);
						PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_PRODUCT_OBJ, NULL , ebufp);

						arg_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 14, ebufp);
						PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, NULL , ebufp);
						
						
						arg_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 15, ebufp);
						usage_map_flistp = PIN_FLIST_ELEM_ADD(arg_flistp, PIN_FLD_USAGE_MAP, 0, ebufp);

						PIN_FLIST_FLD_SET(usage_map_flistp, PIN_FLD_EVENT_TYPE, "%grant%", ebufp);

						strcpy(mod_str, "select /*+full(account_t) parallel(account_t,16) parallel(purchased_product_t,16) parallel(product_t,16)*/ X from /account 1, /purchased_product 2, /product 3  where account_t.poid_id0 > 1  and 2.F1 = V1 and ( 2.F2 <= V2 or ( 2.F3 > 2.F4 and 2.F5 <= V5 )) and ( 2.F6 > V6 or 2.F7 < 2.F8 or 2.F9 = V9 ) and ( 1.F10 = 2.F11 ) and ( 2.F12 = V12 ) and 2.F13 = 3.F14 and 3.F15 like V15 order by account_t.poid_id0 asc ");
						
					}
					else 
					{
						arg_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 13, ebufp);
						PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_SERVICE_OBJ, NULL , ebufp);

						arg_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 14, ebufp);
						PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, NULL , ebufp);

						arg_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 15, ebufp);
						mso_service_flistp = PIN_FLIST_SUBSTR_ADD(arg_flistp, MSO_FLD_BB_INFO, ebufp);
						PIN_FLIST_FLD_SET(mso_service_flistp, PIN_FLD_INDICATOR, &srvc_indicator, ebufp);
						
						arg_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 16, ebufp);
						PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_PRODUCT_OBJ, NULL , ebufp);

						arg_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 17, ebufp);
						PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, NULL , ebufp);
						
						
						arg_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 18, ebufp);
						usage_map_flistp = PIN_FLIST_ELEM_ADD(arg_flistp, PIN_FLD_USAGE_MAP, 0, ebufp);
						PIN_FLIST_FLD_SET(usage_map_flistp, PIN_FLD_EVENT_TYPE, "%grant%", ebufp);
						
						strcpy(mod_str, "select /*+ full(purchased_product_t) parallel(account_t,16) parallel(purchased_product_t,16) parallel(service_t,16) parallel(product_t,16)*/ X from /account 1, /purchased_product 2, /service/telco/broadband 3, /product 4  where account_t.poid_id0 > 1  and 2.F1 = V1 and ( 2.F2 <= V2 or ( 2.F3 > 2.F4 and 2.F5 <= V5 )) and ( 2.F6 > V6 or 2.F7 < 2.F8 or 2.F9 = V9 ) and ( 1.F10 = 2.F11 ) and ( 2.F12 = V12 ) and 2.F13 = 3.F14 and 3.F15 = V15 and 2.F16 = 4.F17 and 4.F18 like V18 order by account_t.poid_id0 asc ");
					}
						PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, mod_str, ebufp);
					

				}

				/*============================================
				OOB call
				=============================================*/
				if( flg_oob == 1)
				{
					strcpy(mod_str, "select /*+ full(account_t) parallel(account_t, 32) parallel(purchased_product_t, 32) */ X ");
					strcat(mod_str, srch_str);

					PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, mod_str, ebufp);
				}

				if(mod_str)
				{
					pin_free(mod_str);
				}

				if(final_str)
				{
					pin_free(final_str);
				}
			}


			if(prog_name && strcmp(prog_name, "pin_inv_accts") == 0)
			{	 
				mod_str	= pin_malloc(512);
				memset(mod_str,'\0',sizeof(mod_str));
				

				/*============================================
				Billing Segment passed
				=============================================*/
				bill_segment_ptr = (int32*)PIN_FLIST_FLD_GET(app_flistp, PIN_FLD_BILLING_SEGMENT, 1, ebufp); 
				if (bill_segment_ptr )
				{
					bill_segment = *bill_segment_ptr; 
				}
				if (bill_segment != -1)
				{
					flg_oob = 0;
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " bill_segment");
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "account no");	

					arg_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp);
					PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_BILLINFO_OBJ, NULL , ebufp);

					arg_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp);
					PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, NULL , ebufp);

					arg_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp);
					PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_BILLING_SEGMENT, &bill_segment , ebufp);

					strcpy(mod_str, "select /*+ full(bill_t) parallel(bill_t, 32) parallel(billinfo_t, 32) */ X from /bill 1, /billinfo 2 where 1.F1 > V1 and 1.F2= 2.F3 and 2.F4 = V4 and bill_t.poid_id0 > 1 and (bill_t.invoice_obj_id0 = 0 or bill_t.invoice_obj_id0 is NULL) order by bill_t.poid_id0 asc ");
					PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, mod_str, ebufp);
				}

				if( flg_oob == 1)
				{
					strcpy(mod_str, "select /*+ full(bill_t) parallel(bill_t, 32) */ X ");
					strcat(mod_str, srch_str);

					PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, mod_str, ebufp);
				}


				if(mod_str)
				{
					pin_free(mod_str);
				}
			}
 		}
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "in_flistp", in_flistp);
	*r_flistpp = PIN_FLIST_COPY(in_flistp, ebufp);
}

