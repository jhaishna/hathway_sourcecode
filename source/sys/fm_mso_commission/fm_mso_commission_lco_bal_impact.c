/*******************************************************************
 * File:        fm_mso_commission_lco_bal_impact.c
 * Opcode:      MSO_OP_COMMISSION_LCO_BAL_IMPACT
 * Owner:       
 * Created:     29-JUL-2014
 * Description: This opcode is for transferring the balance from LCO account
 *******************************************************************/
  
  
  
#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_commission_lco_bal_impact.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "pcm.h"
#include "ops/act.h"
#include "ops/cust.h"
#include "ops/bill.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "mso_cust.h"
#include "mso_commission.h"
  
EXPORT_OP void
op_mso_commission_lco_bal_impact (
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_mso_commission_lco_bal_impact (
        pcm_context_t     	 *ctxp,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);
  
  
//static void
//fm_mso_commission_prev_event (
//        pcm_context_t     	 *ctxp,
//        pin_flist_t             *i_flistp,
//        pin_flist_t             **r_flistpp,
//        pin_errbuf_t            *ebufp);
  
void
op_mso_commission_lco_bal_impact (
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp) { 
        pcm_context_t           *ctxp = connp->dm_ctx;
        *r_flistpp              = NULL;
 
        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                return;
        }
        PIN_ERRBUF_CLEAR(ebufp);
 
        /*******************************************************************
         * Insanity Check
         *******************************************************************/
        if (opcode != MSO_OP_COMMISSION_LCO_BAL_IMPACT) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_mso_commission_lco_bal_impact error",ebufp);
                return;
        }
        /*******************************************************************
 
        /*******************************************************************
         * Debug: Input flist
         *******************************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "op_mso_commission_lco_bal_impact input flist", i_flistp);
	fm_mso_commission_lco_bal_impact(ctxp, flags, i_flistp, r_flistpp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,  "op_mso_commission_lco_bal_impact output flist", *r_flistpp);
        return;
}
 
 
/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void
fm_mso_commission_lco_bal_impact (
        pcm_context_t           *ctxp,
        int32             	flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp) {

	/* Variable Declaration Start */
	pin_flist_t             *event_flistp=NULL;
	pin_flist_t             *sub_event_flistp=NULL;
	pin_flist_t             *event_bal_imp_flistp=NULL;
	pin_flist_t		*search_flistp = NULL;
        //pin_flist_t             *search_oflistp = NULL;
        poid_t                  *event_poidp=NULL;
	//poid_t                  *tmp_pdp1=NULL;
	poid_t                  *lco_acct_poidp=NULL;
	poid_t                  *lco_serv_poidp=NULL;
	//poid_t                  *tmp_pdp3=NULL;
	//char                    *tmp_str0="TEST";
	//char                    *tmp_str1="TEST";
	//poid_t                  *tmp_pdp4=NULL;
	//char                    *tmp_str2="TEST";
	time_t          	*start_t=NULL;
	time_t          	*end_t=NULL;
	//char                    *tmp_str3="/service/catv";
	//u_int                   tmp_enum2=3;
	//int32                   tmp_enum3=384850;
	//pin_decimal_t           *tmp_deci4=NULL;
	//poid_t                  *tmp_pdp5=NULL;
	//pin_decimal_t           *amount_decp=(pin_decimal_t *)NULL;
	u_int                   impact_type=2;
	//pin_decimal_t           *tmp_deci7=NULL;
	//pin_decimal_t           *tmp_deci8=NULL;
	int32                   resource_id=356;
	int32                   glid=1;
	//char                    *tmp_str4="0";
	//char                    *tmp_str5="Dummy";
        //int32			search_flag = 256;
        //poid_t		        *search_poid = NULL;
        //char			search_temp[300];
        //pin_flist_t             *arg_flistp=NULL;
        //pin_flist_t             *result_flistp=NULL;        
        //int32           	elemid = 0;
        //pin_cookie_t    	cookie = NULL;
        pin_flist_t            *charge_head_flistp=NULL;
        pin_flist_t             *return_usage_flistp=NULL;
        //char                    *msg = NULL;
        int32                   usage_success = 0;
        int32                   usage_failure = 0;
	int64			db = -1;
        pin_flist_t             *r_flistp=NULL;
	pin_decimal_t		*one_decp = NULL;
	/* Variable Declaration End */

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                return;
        }
        PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_lco_bal_impact input flistc", i_flistp);

	db = PIN_POID_GET_DB(PCM_GET_USERID(ctxp));
	event_flistp = PIN_FLIST_CREATE(ebufp);
	event_poidp = PIN_POID_CREATE(db, "/event/billing/settlement/ws/incollect/charge_head_based", -1, ebufp);
	PIN_FLIST_FLD_SET(event_flistp,PIN_FLD_POID, event_poidp, ebufp);
	sub_event_flistp = PIN_FLIST_SUBSTR_ADD(event_flistp, PIN_FLD_EVENT, ebufp);
	PIN_FLIST_FLD_PUT(sub_event_flistp, PIN_FLD_POID, event_poidp, ebufp);

        lco_acct_poidp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp );
        PIN_FLIST_FLD_SET(sub_event_flistp, PIN_FLD_ACCOUNT_OBJ, (void *)lco_acct_poidp, ebufp );
        lco_serv_poidp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp );
        PIN_FLIST_FLD_SET(sub_event_flistp, PIN_FLD_SERVICE_OBJ, (void *)lco_serv_poidp, ebufp );

	PIN_FLIST_FLD_SET(sub_event_flistp,PIN_FLD_NAME, "LCO Balance Impact", ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, sub_event_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SYS_DESCR, sub_event_flistp, PIN_FLD_SYS_DESCR, ebufp);
	
        start_t = (time_t*)PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_START_T,0,ebufp);
	PIN_FLIST_FLD_SET(sub_event_flistp,PIN_FLD_START_T, (void*)start_t, ebufp);
	end_t =  (time_t*)PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_END_T,0,ebufp);
	PIN_FLIST_FLD_SET(sub_event_flistp,PIN_FLD_END_T, (void*)end_t, ebufp);

	charge_head_flistp = PIN_FLIST_SUBSTR_GET(i_flistp,MSO_FLD_CHARGE_HEAD_INFO,0,ebufp);
        PIN_FLIST_SUBSTR_SET(sub_event_flistp, charge_head_flistp, MSO_FLD_CHARGE_HEAD_INFO, ebufp);

	event_bal_imp_flistp = PIN_FLIST_ELEM_ADD(sub_event_flistp, PIN_FLD_BAL_IMPACTS, 0, ebufp);
        PIN_FLIST_FLD_SET(event_bal_imp_flistp, PIN_FLD_ACCOUNT_OBJ, (void *)lco_acct_poidp, ebufp );
        
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_AMOUNT, event_bal_imp_flistp, PIN_FLD_AMOUNT, ebufp);

	PIN_FLIST_FLD_SET(event_bal_imp_flistp,PIN_FLD_IMPACT_TYPE, (void*)&impact_type, ebufp);
	one_decp = pbo_decimal_from_str("1",ebufp);
	PIN_FLIST_FLD_SET(event_bal_imp_flistp,PIN_FLD_PERCENT, (void*)one_decp, ebufp);
	PIN_FLIST_FLD_SET(event_bal_imp_flistp,PIN_FLD_QUANTITY, (void*)one_decp, ebufp);
	PIN_FLIST_FLD_SET(event_bal_imp_flistp,PIN_FLD_RESOURCE_ID, (void*)&resource_id, ebufp);
	PIN_FLIST_FLD_SET(event_bal_imp_flistp,PIN_FLD_GL_ID, (void*)&glid, ebufp);
	PIN_FLIST_FLD_SET(event_bal_imp_flistp,PIN_FLD_TAX_CODE, "Dummy", ebufp);
	PIN_FLIST_FLD_SET(event_bal_imp_flistp,PIN_FLD_RATE_TAG, "Dummy", ebufp);

        PCM_OP(ctxp, PCM_OP_ACT_USAGE, 0,event_flistp, &return_usage_flistp, ebufp);
        r_flistp = PIN_FLIST_CREATE(ebufp);
        if ( PIN_ERRBUF_IS_ERR(ebufp)) {
             PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
             PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &usage_failure, ebufp);
             PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR, "Error in usage", ebufp);
        }
        else {
             PIN_FLIST_FLD_COPY(return_usage_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
             PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &usage_success, ebufp);
             PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
             PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR, "Usage Success", ebufp);
        }

	if ( PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in test", ebufp);
		goto cleanup;
	}

cleanup:
	*r_flistpp = r_flistp;
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_lco_bal_impact output flist", *r_flistpp);
        PIN_FLIST_DESTROY_EX(&event_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&search_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&return_usage_flistp, NULL);
	return;
}
