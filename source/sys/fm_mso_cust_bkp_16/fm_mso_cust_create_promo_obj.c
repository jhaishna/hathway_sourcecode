/*******************************************************************
 * File:	fm_mso_cust_create_promo_obj.c
 * Opcode:	MSO_OP_CUST_CREATE_PROMO_OBJ 
 * Owner:	Biju.J
 * Created:	18-AUG-2014
 * Description: This opcode is for creating, Modifying and Updating Expiry of plan in promotion class

	r << xx 1
		0 PIN_FLD_POID           POID [0] 0.0.0.1 /higher_arpu_promotion_plan 13488707914 0
		0 MSO_FLD_DL_SPEED     DECIMAL [0] 9999
		0 MSO_FLD_EXPIRATION_DATE    STR [0] "15325632111"
		0 PIN_FLD_ACCOUNT_OBJ    POID [0] 0.0.0.1 /account 11189929287 0
		0 PIN_FLD_AMOUNT       DECIMAL [0] 9999
		0 PIN_FLD_CHECK_NO        STR [0] "R"
		0 PIN_FLD_CITY            STR [0] "BANGALORE"
		0 PIN_FLD_CLASSMARK       STR [0] "P30"
		0 PIN_FLD_CLASS_NAME      STR [0] "change_plan"
		0 MSO_FLD_CUSTOMER_BIT	  STR [0] "ALL"
		0 PIN_FLD_PLAN_OBJ       POID [0] 0.0.0.1 /plan 3943714871 0
		0 MSO_FLD_DISPLAY_FLAG    STR [0] "Addition"
	xx
	xop MSO_OP_CUST_CREATE_PROMO_OBJ 0 1


	# number of field entries allocated 20, used 3
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /higher_arpu_promotion_plan 12925875285 0
	0 PIN_FLD_PLAN_OBJ       POID [0] 0.0.0.1 /plan 12345678 0
	0 PIN_FLD_DESCR           STR [0] "higher_arpu_promotion_plan creation completed successfully"


#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_create_promo_obj.c:ServerIDCVelocityInt:4:2006-Sep-14 11:34:45 %";
#endif

/*******************************************************************
 * Contains the MSO_OP_CUST_CREATE_PROMO_OBJ operation.
 *******************************************************************/

#include <stdio.h>
#include <string.h>

#include <pcm.h>
#include <pinlog.h>

#define FILE_LOGNAME "fm_mso_cust_create_promo_obj.c(1)"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "pin_cust.h"
#include "ops/act.h"
#include "ops/cust.h"
#include "ops/bill.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "pcm_ops.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "mso_cust.h"
#include "mso_prov.h"
#include "mso_lifecycle_id.h"
#include "mso_device.h"
#include "mso_utils.h"
#include "mso_errs.h"


/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_mso_cust_create_promo(
    cm_nap_connection_t    *connp,
    int32            opcode,
    int32            flags,
    pin_flist_t        *i_flistp,
    pin_flist_t        **r_flistpp,
    pin_errbuf_t        *ebufp);

static void
fm_mso_cust_create_promo(
    pcm_context_t        *ctxp,
    int32            flags,
    pin_flist_t        *i_flistp,
    pin_flist_t        **r_flistpp,
    pin_errbuf_t        *ebufp);

PIN_EXPORT char*
fm_mso_get_plan_name(
        pcm_context_t           *ctxp,
        pin_flist_t             *plan_obj,
        pin_flist_t             *plan_name,
        pin_errbuf_t            *ebufp);

static void
fm_mso_check_duplicate_promo_obj(
        pcm_context_t           *ctxp,
        pin_flist_t             *plan_obj,
        pin_flist_t        **r_flistpp,
        pin_errbuf_t            *ebufp);
		
void
fm_mso_set_promo_on_req(
	pcm_context_t           *ctxp,
	poid_t                  *account_obj,
	char                  *promo_obj,
	pin_errbuf_t            *ebufp);

/*******************************************************************
 * Main routine for the MSO_OP_CUST_CREATE_PROMO_OBJ operation.
 *******************************************************************/
void
op_mso_cust_create_promo(
    cm_nap_connection_t    *connp,
    int32            opcode,
    int32            flags,
    pin_flist_t        *i_flistp,
    pin_flist_t        **r_flistpp,
    pin_errbuf_t        *ebufp)
{
    pcm_context_t        *ctxp = connp->dm_ctx;
    pin_flist_t    *r_flistp = NULL;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;
    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Null out results
     ***********************************************************/
    *r_flistpp = NULL;

    /***********************************************************
     * Insanity check.
     ***********************************************************/
    if (opcode != MSO_OP_CUST_CREATE_PROMO_OBJ) {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
            PIN_ERR_BAD_OPCODE, 0, 0, opcode);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_mso_cust_create_promo opcode error", ebufp);

        return;
    }
    
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "op_mso_cust_create_promo input flist", i_flistp);

    /***********************************************************
     * Main routine for this opcode
     ***********************************************************/
    fm_mso_cust_create_promo(ctxp, flags, i_flistp, r_flistpp, ebufp);
    
    /***********************************************************
     * Error?
     ***********************************************************/
    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        PIN_FLIST_DESTROY_EX(r_flistpp, ebufp);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_mso_cust_create_promo error", ebufp);
    } else {
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "op_mso_cust_create_promo output flist", *r_flistpp);
    }

    return;
}

/*******************************************************************
 * op_mso_cust_create_promo:
 *
 *******************************************************************/
static void
fm_mso_cust_create_promo(
    pcm_context_t           *ctxp,
    int32                   flags,
    pin_flist_t             *i_flistp,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp)
{
	pin_flist_t 	*in_flistp = NULL;
	pin_flist_t 	*rr_flistpp = NULL;
	pin_flist_t 	*in_exp_flistp = NULL;
    poid_t          *plan_obj = NULL;
	//char            *promo_obj = NULL;
	//poid_t          *promo_poid = NULL;
	poid_t			*plan_name = NULL;
	poid_t			*account_obj = NULL;
	poid_t			*pro_poid = NULL;
    char            *ret_plan_name = NULL;
    pin_decimal_t 	*amount = NULL;
	pin_decimal_t 	*dl_speed = NULL;
	char 			*adhoc_type = NULL;
	char 			*city =NULL;
	char 			*allow_offer =NULL;
	char 			*activity = NULL;
	char 			*exp_month = NULL;
	char 			*on_request = NULL;
	char 			*action_flag = NULL;
	//char 			*is_existing = NULL;
	int64           db = 0;
	
	
	poid_t		    *pdp = NULL;
	
	
    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR(ebufp);
	
	//*rr_flistpp = PIN_FLIST_CREATE(ebufp);
	//rs_flistp = PIN_FLIST_CREATE(ebufp);
	
	plan_obj = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
	
	
	db = PIN_POID_GET_DB(plan_obj);
	
	//srch_flistp = PIN_FLIST_CREATE(ebufp);
    
	
	plan_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
	
	// Avoid duplicate entry in promotion configuration table
	//fm_mso_check_duplicate_promo_obj(ctxp,plan_obj,r_flistpp,ebufp);
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "is_existing is ");
	//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, is_existing);
	
	//if (is_existing && is_existing != NULL){
		//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			//"/higher_arpu_promotion_plan obj update output flist", *r_flistpp);
		//PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_PLAN_OBJ, plan_obj, ebufp);
		//PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_DESCR, "higher_arpu_promotion_plan creation completed successfully", ebufp);

		//*r_flistpp = PIN_FLIST_COPY(*r_flistpp, ebufp);
		//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			//"/higher_arpu_promotion_plan obj update output flist", *r_flistpp);	
		
		
	//} else {
		
	//if (*r_flistpp == (pin_flist_t	*)NULL){
		
		amount = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 0, ebufp);
		
		dl_speed = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_DL_SPEED, 0, ebufp);
		
		adhoc_type = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CHECK_NO, 0, ebufp);
		
		city = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CITY, 0, ebufp);
		
		allow_offer = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CLASSMARK, 0, ebufp);
		
		activity = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CLASS_NAME, 0, ebufp);
		
		exp_month = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_EXPIRATION_DATE, 0, ebufp);
		
		on_request = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_CUSTOMER_BIT, 0, ebufp);
		
		account_obj = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
		
		action_flag = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_DISPLAY_FLAG, 0, ebufp);
		/* Get Plan Name using Plan Poid */
		
		ret_plan_name = fm_mso_get_plan_name(ctxp,plan_obj,plan_name,ebufp);
			
			//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ret_plan_name is ");
			//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, ret_plan_name);			
		if ( ret_plan_name != NULL) {
						
				in_flistp = PIN_FLIST_CREATE(ebufp);
			
				
				
				PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_PLAN_OBJ, plan_obj, ebufp);
				PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_AMOUNT, amount, ebufp);
				PIN_FLIST_FLD_SET(in_flistp, MSO_FLD_DL_SPEED, dl_speed, ebufp);
				PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_CHECK_NO, adhoc_type, ebufp);
				PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_CITY, city, ebufp);
				PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_CLASSMARK, allow_offer, ebufp);
				PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_CLASS_NAME, activity, ebufp);
				PIN_FLIST_FLD_SET(in_flistp, MSO_FLD_EXPIRATION_DATE, exp_month, ebufp);
				PIN_FLIST_FLD_SET(in_flistp, MSO_FLD_CUSTOMER_BIT, on_request, ebufp);
				PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);

				PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_NAME, ret_plan_name, ebufp);
				
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"/higher_arpu_promotion_plan create input flist", in_flistp);
				
				if (action_flag && strcmp(action_flag, "Addition") == 0)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Promo obj Addition");
					pdp = PIN_POID_CREATE(db, "/higher_arpu_promotion_plan", -1, ebufp);
					PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, pdp, ebufp);
					
					PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, in_flistp, r_flistpp, ebufp);
					
					if (PIN_ERRBUF_IS_ERR(ebufp))
                	{
							PIN_ERRBUF_CLEAR(ebufp);
							*r_flistpp = PIN_FLIST_CREATE(ebufp);
							PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
							PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_PLAN_OBJ, plan_obj, ebufp);
							PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_DESCR, "DUPLICATE - This account/City has the same promotion plan configured already", ebufp);
							goto CLEANUP;
                	}
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Promo obj Addition -1");
					PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_DESCR, "higher_arpu_promotion_plan creation completed successfully", ebufp);
				}
				else if (action_flag && strcmp(action_flag, "Modify") == 0)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Promo obj Modify");
					pro_poid = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
					PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, pro_poid, ebufp);
					PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, in_flistp, r_flistpp, ebufp);
					
					PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_DESCR, "higher_arpu_promotion_plan updation completed successfully", ebufp);
					PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PLAN_OBJ, *r_flistpp, PIN_FLD_PLAN_OBJ, ebufp);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "/higher_arpu_promotion_plan obj create output flist", *r_flistpp);
					goto CLEANUP;
					
				}
				else if (action_flag && strcmp(action_flag, "Expiry") == 0)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Promo obj Expiry");
					in_exp_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(in_exp_flistp, PIN_FLD_CLASSMARK, allow_offer, ebufp); //Update Expiration date
					
					pro_poid = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
					PIN_FLIST_FLD_SET(in_exp_flistp, PIN_FLD_POID, pro_poid, ebufp);
					
					PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, in_exp_flistp, r_flistpp, ebufp);
					
					PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_DESCR, "higher_arpu_promotion_plan expiration updation completed successfully", ebufp);
					PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PLAN_OBJ, *r_flistpp, PIN_FLD_PLAN_OBJ, ebufp);
					
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "/higher_arpu_promotion_plan obj create output flist", *r_flistpp);
					
					goto CLEANUP;
				}
					
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"/higher_arpu_promotion_plan obj create output flist", *r_flistpp);
			
			PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PLAN_OBJ, *r_flistpp, PIN_FLD_PLAN_OBJ, ebufp);
			//PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_DESCR, "higher_arpu_promotion_plan creation completed successfully", ebufp);
			
			//Getting congig promo obj in order to update it in account table
			/*PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"/higher_arpu_promotion_plan obj create output flist 2", *r_flistpp);
			promo_int = PIN_POID_GET_ID(PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_POID, 0, ebufp));
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Promo obj is defined");
			sprintf(promo_obj,"%lld", promo_int);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,promo_obj);*/
			
		/*} else {
			PIN_ERRBUF_CLEAR(ebufp);
			rr_flistpp = PIN_FLIST_CREATE(ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST NO 2");
			PIN_FLIST_FLD_COPY(*r_flistpp, PIN_FLD_POID, rr_flistpp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(rr_flistpp, PIN_FLD_PLAN_OBJ, plan_obj, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST NO 3");
			PIN_FLIST_FLD_SET(rr_flistpp, PIN_FLD_DESCR, "higher_arpu_promotion_plan updation completed successfully", ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST NO 4");
			
			*r_flistpp = PIN_FLIST_COPY(rr_flistpp, ebufp);
			
		}*/
			//Getting congig promo obj in order to update it in account table
			/*PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"/higher_arpu_promotion_plan obj create output flist 2", *r_flistpp);
			promo_int = PIN_POID_GET_ID(PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_POID, 0, ebufp));
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Promo obj is defined");
			sprintf(promo_obj,"%lld", promo_int);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,promo_obj);*/
	
    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "/higher_arpu_promotion_plan create error", ebufp);
    }
	
	//Getting congig promo obj in order to update it in account table
	pin_flist_t 	*search_flistp = NULL;
	pin_flist_t 	*search_res_flistp = NULL;
	pin_flist_t 	*args_flistp = NULL;
	pin_flist_t 	*res_flistp = NULL;
	//poid_t			*promo_poid = NULL;
	poid_t			*search_pdp = NULL;
	int32           db1 = -1;
	int             search_flags = 512;
	int64           promo_int = 0;
	char 			promo_obj[250];
	char            search_template[100] = " select X from /higher_arpu_promotion_plan where F1 = V1";
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

        db1 = PIN_POID_GET_DB(plan_obj);

        search_pdp = PIN_POID_CREATE(db1, "/search", -1, ebufp);
        search_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(search_flistp, PIN_FLD_POID, search_pdp, ebufp);
        PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_FLAGS, &search_flags, ebufp);
        PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_TEMPLATE, search_template, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, plan_obj, ebufp);
        res_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_RESULTS, 0, ebufp);
        PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "get higher_arpu_promotion_plan poid search input list", search_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_flistp, &search_res_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&search_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in searching higher_arpu_promotion_plan poid", ebufp);
                PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "higher_arpu_promotion_plan poid search output list", search_res_flistp);

        res_flistp = PIN_FLIST_ELEM_GET(search_res_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
        
        //promo_poid = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_POID, 1, ebufp);
		
		promo_int = PIN_POID_GET_ID(PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_POID, 0, ebufp));
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Promo obj is defined");
		sprintf(promo_obj,"%lld", promo_int);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,promo_obj);

//end		
	
	account_obj = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "PIN_FLD_ACCOUNT_OBJ ");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, account_obj);
	//promo_obj = (char*)PIN_POID_GET_TYPE(PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_POID, 0, ebufp));
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Update account flag ");
	//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, promo_obj);
	
	fm_mso_set_promo_on_req(ctxp, account_obj, promo_obj, ebufp); //Update the promotion obj in account table regarding onrequest change
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Update account flag done");
	}
	else 
	{
		//PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PLAN_OBJ, *r_flistpp, PIN_FLD_PLAN_OBJ, ebufp);
		//PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_DESCR, "Plan provided does not exist", ebufp);
		poid_t *dumm_poid = NULL;
		PIN_ERRBUF_CLEAR(ebufp);
		rr_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST NO 2");
		PIN_FLIST_FLD_SET(rr_flistpp, PIN_FLD_POID, plan_obj, ebufp);
		//PIN_FLIST_FLD_SET(rr_flistpp, PIN_FLD_PLAN_OBJ, plan_obj, ebufp);
		PIN_FLIST_FLD_SET(rr_flistpp, PIN_FLD_DESCR, "Plan provided does not exist", ebufp);
		
		*r_flistpp = PIN_FLIST_COPY(rr_flistpp, ebufp);
			
	}
	CLEANUP:
		if(in_exp_flistp)
		PIN_FLIST_DESTROY_EX(&in_exp_flistp, NULL);
		if(in_flistp)
        PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
			
    return;
}

void
fm_mso_set_promo_on_req(
	pcm_context_t           *ctxp,
	poid_t                  *account_obj,
	char                  *promo_obj,
	pin_errbuf_t            *ebufp)
{

	pin_flist_t *search_flistp = NULL;
	pin_flist_t *search_res_flistp = NULL;
	pin_flist_t *args_flistp = NULL;
	pin_flist_t *result_flistp = NULL;
	pin_flist_t *updacc_inflistp = NULL;
	pin_flist_t *updacc_outflistp = NULL;

	poid_t		*search_pdp = NULL;
	poid_t		*account_poid = NULL;

	int32           db = -1;
	int             search_flags = 512;
	char            search_template[100] = " select X from /account where F1 = V1";
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

        db = PIN_POID_GET_DB(account_obj);

        search_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
        search_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(search_flistp, PIN_FLD_POID, search_pdp, ebufp);
        PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_FLAGS, &search_flags, ebufp);
        PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_TEMPLATE, search_template, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, account_obj, ebufp);
        result_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_RESULTS, 0, ebufp);
        PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
        PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, (char *)NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_set_promo_obj search account_poid input list", search_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_flistp, &search_res_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&search_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in searching account_poid", ebufp);
                PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_set_promo_obj search account_poid output list", search_res_flistp);

        result_flistp = PIN_FLIST_ELEM_GET(search_res_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
        if ( !result_flistp )
        {
                PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
                return;
        }
        account_poid = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_POID, 1, ebufp);
        updacc_inflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(updacc_inflistp, PIN_FLD_POID, account_poid, ebufp);
        PIN_FLIST_FLD_SET(updacc_inflistp, PIN_FLD_ACCOUNT_TAG, "YES", ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_set_promo_obj update input list", updacc_inflistp);
        PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, updacc_inflistp, &updacc_outflistp, ebufp);
        PIN_FLIST_DESTROY_EX(&updacc_inflistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_WRITE_FLDS", ebufp);
                PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
                PIN_FLIST_DESTROY_EX(&updacc_outflistp, NULL);
                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_set_promo_obj update output list", updacc_outflistp);
        PIN_FLIST_DESTROY_EX(&updacc_outflistp, NULL);
        PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
	return;
}	


static void
fm_mso_check_duplicate_promo_obj(
        pcm_context_t           *ctxp,
        pin_flist_t             *plan_obj,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *srch_flistp = NULL;
        pin_flist_t             *rs_flistp = NULL;
		pin_flist_t             *result_flistp = NULL;
		pin_flist_t             *args_flistp = NULL;
		poid_t                  *pdp = NULL;
        u_int64                 db = 0;
        int32                   s_flags = 256;
		char 					*ret_value = NULL;
		char template[200] = "select X from /higher_arpu_promotion_plan where F1 = V1";
		
			if (plan_obj && plan_obj != NULL)
			{
			
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Plan obj is defined");
			PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Plan obj is : ", plan_obj);
			
			PIN_ERRBUF_CLEAR(ebufp);
			rs_flistp = PIN_FLIST_CREATE(ebufp);
			
			db = PIN_POID_GET_DB(plan_obj);
			pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
			srch_flistp = PIN_FLIST_CREATE(ebufp);
				
			PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, pdp, ebufp);
			PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
			PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template, ebufp);
			
			args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp);
			PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_PLAN_OBJ, plan_obj, ebufp);
			
			result_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
		
			PIN_FLIST_FLD_SET(result_flistp,PIN_FLD_NAME, NULL, ebufp);
		
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "duplicate check input flist", srch_flistp);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &rs_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Plan Name output flist", rs_flistp);
		
			result_flistp = PIN_FLIST_ELEM_GET(rs_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "duplicate check output flist", result_flistp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "plan is not existing");
				PIN_ERRBUF_CLEAR(ebufp);
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Plan is existing");
				//ret_value = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_NAME, 0, ebufp);
				*r_flistpp = PIN_FLIST_COPY(rs_flistp, ebufp);
			}
			
	}
	
	return;
}
/*******************************************************************************
 * fm_mso_get_plan_name()
 * Gets Plan Name using Plan Obj Id
 ******************************************************************************/
PIN_EXPORT char*
fm_mso_get_plan_name(
        pcm_context_t           *ctxp,
        pin_flist_t             *plan_obj,
        pin_flist_t             *plan_name,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *srch_flistp = NULL;
        pin_flist_t             *rs_flistp = NULL;
		pin_flist_t             *result_flistp = NULL;
		pin_flist_t             *args_flistp = NULL;
		poid_t                  *pdp = NULL;
        u_int64                 db = 0;
        int32                   s_flags = 256;
		char 					*ret_value = NULL;
		char template[200] = "select X from /plan where F1 = V1";
		
			if (plan_obj && plan_obj != NULL)
			{
			
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Plan obj is defined");
			PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Plan obj is : ", plan_obj);
			
			PIN_ERRBUF_CLEAR(ebufp);
			rs_flistp = PIN_FLIST_CREATE(ebufp);
			
			db = PIN_POID_GET_DB(plan_obj);
			pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
			srch_flistp = PIN_FLIST_CREATE(ebufp);
				
			PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, pdp, ebufp);
			PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
			PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template, ebufp);
			
			args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp);
			PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_POID, plan_obj, ebufp);
			
			result_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
		
			PIN_FLIST_FLD_SET(result_flistp,PIN_FLD_NAME, NULL, ebufp);
		
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Plan Name input flist", srch_flistp);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &rs_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Plan Name output flist", rs_flistp);
		
			result_flistp = PIN_FLIST_ELEM_GET(rs_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Plan Name search output flist", result_flistp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account is not active");
				PIN_ERRBUF_CLEAR(ebufp);
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Found Plan Details");
				ret_value = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_NAME, 0, ebufp);
			}
			
	}
/*
        PIN_ERRBUF_CLEAR(ebufp);
		rs_flistp = PIN_FLIST_CREATE(ebufp);
		
		srch_flistp = PIN_FLIST_CREATE(ebufp);

		
        if (PIN_ERR_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,"Plan name search error flist", plan_obj);
                return;
        }
        pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
		
        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
		
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, "select X from /plan where F1 = V1 ", ebufp);
		
        flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, plan_obj, ebufp);
		
        flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp);
        
		PIN_FLIST_FLD_SET(flistp,PIN_FLD_NAME, NULL, ebufp);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Plan Name Input Flist", srch_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &rs_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Plan name Output Flist", rs_flistp);

        r_flistp = PIN_FLIST_ELEM_GET(rs_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
        ret_value = PIN_FLIST_FLD_TAKE(r_flistp, PIN_FLD_NAME, 0, ebufp);

        if (PIN_ERR_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "Plan name error flist", rs_flistp);
                return;
        }
  */      
   //     PIN_ERR_CLEAR_ERR(ebufp);
        return ret_value;

}


