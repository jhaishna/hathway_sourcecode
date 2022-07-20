/*******************************************************************
 * File:        fm_mso_commission_lco_bal_transfer.c
 * Opcode:      MSO_OP_COMMISSION_LCO_BAL_TRANSFER
 * Owner:       
 * Created:     29-JUL-2014
 * Description: This opcode is for transferring the balance from LCO account
 *******************************************************************/
  
  
  
#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_commission_lco_bal_transfer.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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
op_mso_commission_lco_bal_transfer (
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_mso_commission_lco_bal_transfer (
        pcm_context_t     	 *ctxp,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);
  
  
void
op_mso_commission_lco_bal_transfer (
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
        if (opcode != MSO_OP_COMMISSION_LCO_BAL_TRANSFER) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_mso_commission_lco_bal_transfer error",ebufp);
                return;
        }
        /*******************************************************************
 
        /*******************************************************************
         * Debug: Input flist
         *******************************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "op_mso_commission_lco_bal_transfer input flist", i_flistp);
	fm_mso_commission_lco_bal_transfer(ctxp, flags, i_flistp, r_flistpp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,  "op_mso_commission_lco_bal_transfer output flist", *r_flistpp);
        return;
}
 
 
/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void
fm_mso_commission_lco_bal_transfer (
        pcm_context_t           *ctxp,
        int32             	flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp) {

	/* Variable Declaration Start */
        //pcm_context_t           *new_ctxp=NULL;
	//pin_flist_t             *flistp=NULL;
	pin_flist_t             *flistp0=NULL;
	//poid_t                  *tmp_pdp0=NULL;
	//poid_t                  *tmp_pdp1=NULL;
	pin_decimal_t           *tmp_deci0=NULL;
	pin_decimal_t           *tmp_deci1=NULL;
	//char                    *tmp_str0="LCO Commission";
	//int32                   tmp_enum1 = 356;
	//poid_t                  *tmp_pdp2=NULL;
	//int32                   tmp_enum2=1;
	//poid_t                  *tmp_pdp3=NULL;
	//int32                   tmp_enum3=1;
	//u_int                   tmp_enum4=0;
	pin_flist_t             *r_flistp=NULL;
	int32			*result=NULL;
	//void			*vp = NULL;
	pin_flist_t		*lco_bal_flistp = NULL;
	pin_flist_t		*r_lco_bal_flistp = NULL;
	pin_flist_t		*flistp1 = NULL;
	pin_flist_t		*r_bal_flistp = NULL;
	//poid_t			*bal_grp_poid = NULL;
	int32			failed = 1;
	//time_t                  *valid_from = NULL;
	//time_t                  new_valid_from = 0;
	//pin_flist_t		*bal_flistp = NULL;
	pin_flist_t		*rw_flistp = NULL;
	//int             err;
	int32             *vrfy_bal = NULL;
	
	/* Variable Declaration End */

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_lco_bal_transfer input flist ", i_flistp);

        vrfy_bal = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_VERIFY_BALANCE, 0, ebufp);

	/*Check whether the LCO Account has a balance or not-START*/
	if (*vrfy_bal !=0 && flags!=FROM_LCO_COMMISSION_BUCKET) {
	     lco_bal_flistp = PIN_FLIST_CREATE(ebufp);
	     flistp0 = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_BAL_INFO, 0, 0, ebufp);
	     PIN_FLIST_FLD_SET(lco_bal_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(flistp0, PIN_FLD_BAL_GRP_OBJ, 0, ebufp), ebufp);
	     flistp0 = PIN_FLIST_ELEM_ADD(lco_bal_flistp, PIN_FLD_BALANCES, 356, ebufp );
	     flistp1 = PIN_FLIST_ELEM_ADD(flistp0, PIN_FLD_SUB_BALANCES, 0, ebufp );
	     PIN_FLIST_FLD_SET(flistp1, PIN_FLD_CURRENT_BAL, 0, ebufp);
	     PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_commission_lco_bal_transfer get lco balance read_flds input flist", lco_bal_flistp);

	     PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, lco_bal_flistp, &r_lco_bal_flistp, ebufp);
	     if (PIN_ERRBUF_IS_ERR(ebufp)){
		     PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
		     goto cleanup;
     
	     }
	     PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_commission_lco_bal_transfer get lco balance read_flds output flist", r_lco_bal_flistp);
	     flistp0 = PIN_FLIST_ELEM_GET(r_lco_bal_flistp, PIN_FLD_BALANCES, 356, 0, ebufp);
	     flistp1 = PIN_FLIST_ELEM_GET(flistp0, PIN_FLD_SUB_BALANCES, 0, 0, ebufp);
	     tmp_deci0 = PIN_FLIST_FLD_GET(flistp1, PIN_FLD_CURRENT_BAL, 0, ebufp);
	     tmp_deci1 = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 0, ebufp);

     
	     if(pbo_decimal_compare(tmp_deci0,tmp_deci1,ebufp)==1){
	           PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_lco_bal_transfer - Insufficient funds in the source account", r_lco_bal_flistp);
	           PIN_FLIST_FLD_SET(r_lco_bal_flistp, PIN_FLD_RESULT, &failed, ebufp);     
	           PIN_FLIST_FLD_SET(r_lco_bal_flistp, PIN_FLD_DESCR, "Insufficient fund at the source account", ebufp);     
	           *r_flistpp = r_lco_bal_flistp; 
	           goto cleanup;
	     }
     	}
	/*Check whether the LCO Account has a balance or not-END*/

	PCM_OP(ctxp, PCM_OP_BILL_TRANSFER_BALANCE, 0, i_flistp, &r_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in test", ebufp);
		goto cleanup;
	}

	if(r_flistp!=NULL){
	    result = (int32*)PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_RESULT, 0, ebufp);
	    switch (*result) { 
		case 0: PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR, "Transfer successful!!!", ebufp); break;
		case 1: PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR, "Insufficient funds in the source account", ebufp); break;
		case 2: PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR, "Invalid transfer amount (-ve)", ebufp); break;
	    }
	    *r_flistpp = r_flistp;
	}

cleanup:
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_lco_bal_transfer output flist", *r_flistpp);
	PIN_FLIST_DESTROY_EX(&lco_bal_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&rw_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_bal_flistp, NULL);
        return;
}

