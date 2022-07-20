/*******************************************************************
 * File:        fm_mso_cust_netflix_register.c
 * Opcode:      MSO_OP_CUST_NETFLIX_REGISTER
 * Owner:       ShabeerHussain
 * Created:     06-SEP-2018
 * Description: This opcode will be used to update the /profile_offer_customer object
 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *

********************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_netflix_register.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "pcm_ops.h"
#include "pin_flds.h"
#include "pin_cust.h"
#include "ops/act.h"
#include "ops/cust.h"
#include "ops/bill.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "mso_lifecycle_id.h"

#define READONLY 0
#define READWRITE 1
#define LOCK_OBJ 2

#define LOCAL_TRANS_OPEN_SUCCESS 0
#define LOCAL_TRANS_OPEN_FAIL 1



/**************************************
* External Functions start
***************************************/
extern int32
fm_mso_trans_open(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	int32		flag,
	pin_errbuf_t	*ebufp);

extern void 
fm_mso_trans_commit(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	pin_errbuf_t	*ebufp);

extern void  
fm_mso_trans_abort(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	pin_errbuf_t	*ebufp);


/**************************************
* External Functions end
***************************************/


/*******************************************************************
 * MSO_OP_CUST_NETFLIX_REGISTER 
 *
 *******************************************************************/

EXPORT_OP void
op_mso_cust_netflix_register(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_netflix_register(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * MSO_OP_CUST_NETFLIX_REGISTER  
 *******************************************************************/
void 
op_mso_cust_netflix_register(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	int			local = LOCAL_TRANS_OPEN_SUCCESS;
	int			*status = NULL;
	poid_t			*inp_pdp = NULL;
	char                    *prog_name = NULL;
	int32                   status_uncommitted = 2;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_NETFLIX_REGISTER) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_netflix_register error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_netflix_register input flist", i_flistp);
	

	  /*******************************************************************
         * Call the default implementation
         *******************************************************************/
        prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
        if (!(prog_name && (strstr(prog_name,"pin_deferred_act") || strstr(prog_name,"BULK"))))
        {
                inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
                local = fm_mso_trans_open(ctxp, inp_pdp, READWRITE,ebufp);
                if(PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Unable to open Local Transaction: Error", ebufp);
                        PIN_ERRBUF_RESET(ebufp);
                        *r_flistpp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, inp_pdp, ebufp );
                        PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &status_uncommitted, ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "51010", ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, "Unable to open Local Transaction: Error", ebufp);
                        return;
                }
                if ( local == LOCAL_TRANS_OPEN_FAIL )
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Unable to open Local Transaction: Already Open", ebufp);
                        PIN_ERRBUF_RESET(ebufp);
                        *r_flistpp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, inp_pdp, ebufp );
                        PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &status_uncommitted, ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "51011", ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, "Unable to open Local Transaction: Already Open", ebufp);
                        return;
                }
        }
        else
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Called by 'pin_deferred_act' so transaction will not be handled at API level");
        }


	fm_mso_cust_netflix_register(ctxp, flags, i_flistp, r_flistpp, ebufp);
	status = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_STATUS, 1, ebufp);

 	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp) || (*(int*)status == 1)) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_netflix_register error", ebufp);
		if(local == LOCAL_TRANS_OPEN_SUCCESS )
		{
			fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
		}
	}
	else
	{
		if(local == LOCAL_TRANS_OPEN_SUCCESS )
		{
			fm_mso_trans_commit(ctxp, inp_pdp, ebufp);
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_netflix_register output flist", *r_flistpp);
	}
	return;
}



/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_netflix_register(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*o_flistp = NULL;
	pin_flist_t		*profile_offer_flistp = NULL;
	pin_flist_t             *profile_offer_oflistp = NULL;
	pin_flist_t     *args_flistp = NULL;
        pin_flist_t     *srch_flistp = NULL;
        pin_flist_t     *srch_out_flistp = NULL;
	pin_flist_t     *results_flistp = NULL;
	poid_t			*account_obj = NULL;
	poid_t			*prf_pdp = NULL;
	poid_t			*act_poid =  NULL;
	int32			 caller = MSO_OP_CUST_NETFLIX_REGISTER;
	int64			 db = -1;
	int32			offer_type = 1;
	int32			status=1;
	int32                   prf_update_succ=0;
	int32                   prf_update_failed=1;
	int32           srch_flag = 256;
	char			 mesg[200];
	char            *search_template = "select X from /profile_cust_offer where F1 = V1 and profile_cust_offer_t.offer_descr = 'Netflix'";
	char            *offer_descr = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_netflix_register input flist", i_flistp);	
	act_poid = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
        db = PIN_POID_GET_DB(act_poid);
	//search for /profile_cust_offer poid using account object
	srch_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, search_template, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, act_poid, ebufp);
        results_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
        PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID, NULL, ebufp);
        PIN_FLIST_FLD_SET(results_flistp, MSO_FLD_OFFER_DESCR,NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "profile cust offer search input list", srch_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp) || !srch_out_flistp)
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in profile cust offer search ", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, act_poid, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &prf_update_failed, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "71001", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in profile cust offer search", ebufp);
                goto CLEANUP;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "customer profile search output flist", srch_out_flistp);
	results_flistp= PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "customer profile search result flist", results_flistp);
        if (results_flistp) {
        	prf_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_POID, 1, ebufp);
		offer_descr = PIN_FLIST_FLD_GET(results_flistp, MSO_FLD_OFFER_DESCR, 1, ebufp);
            	if (prf_pdp && (offer_descr && strcmp(offer_descr,"Netflix") ==0 ))  {
			profile_offer_flistp = PIN_FLIST_CREATE(ebufp);
		        PIN_FLIST_FLD_SET(profile_offer_flistp, PIN_FLD_POID,prf_pdp, ebufp );
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_RMN, profile_offer_flistp, MSO_FLD_RMN,ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_ISREGISTERED, profile_offer_flistp, MSO_FLD_ISREGISTERED,ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_TOKENID, profile_offer_flistp, MSO_FLD_TOKENID,ebufp);
		        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_WRITE_FLDS input flist for profile_cust_offer UPDATE", profile_offer_flistp);
		        PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, profile_offer_flistp, &profile_offer_oflistp, ebufp);
		        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_WRITE_FLDS output flist for profile_cust_offer UPDATE", profile_offer_oflistp);
			if (PIN_ERRBUF_IS_ERR(ebufp)  || !srch_out_flistp)
		        {
                		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in updating Profile_Cust_offer Obj", ebufp);
		                PIN_ERRBUF_RESET(ebufp);
                		ret_flistp = PIN_FLIST_CREATE(ebufp);
		                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, act_poid, ebufp);
		                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &prf_update_failed, ebufp);
                		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "71002", ebufp);
		                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in updating Profile_Cust_offer Obj", ebufp);
                		goto CLEANUP;
		        }
	   		else {
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				ret_flistp = PIN_FLIST_COPY(profile_offer_oflistp,ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ACCOUNT_OBJ, act_poid, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &prf_update_succ, ebufp);
		                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_DESCR, "Success : Netflix profile registered succesfully", ebufp);
	       			// PIN_FLIST_DESTROY_EX(&profile_offer_flistp, NULL);
			}	
		}
  
	}
	CLEANUP:	
	*r_flistpp = PIN_FLIST_COPY(ret_flistp,ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&profile_offer_flistp, NULL);
	return;
}

