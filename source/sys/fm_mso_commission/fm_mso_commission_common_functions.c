/*******************************************************************
 * File:        fm_mso_commission_common_functions.c
 * Opcode:      None
 * Owner:       
 * Created:     17-OCT-2014
 * Description:  Common functions for "fm_mso_commission" module. 
 *               The functions should be moved to global common
 *               place if other modules need to use the same function
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_commission_common_functions.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "ops/act.h"
#include "ops/bill.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "mso_cust.h"
#include "pin_currency.h"
#include "ops/bal.h"
#include "mso_commission.h"

/***************************************
External subroutines -Start
****************************************/
extern void
fm_mso_util_get_customer_lco_info(
        pcm_context_t           *ctxp,
        poid_t                  *acnt_pdp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

extern void
fm_mso_utils_prepare_error_flist(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        char                    *error_code,
        char                    *error_descr,
        pin_errbuf_t            *ebufp);

extern void
fm_mso_utils_prepare_status_flist(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        int                     flag,
        char                    *status_descr,
        pin_errbuf_t            *ebufp);

extern int32 
fm_mso_trans_open(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	int32		flag,
	pin_errbuf_t	*ebufp);

extern void fm_mso_trans_commit(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	pin_errbuf_t	*ebufp);
/***************************************
External subroutines -End
****************************************/
int
fm_mso_commission_get_vald_lco_acct_comm_mdl (
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

int
fm_mso_commission_acct_has_agreement(
        pcm_context_t           *ctxp,
        poid_t                  *lco_acct_objp,
        pin_errbuf_t            *ebufp) ;


void
fm_mso_commission_record_failure (
        pcm_context_t           *ctxp,
        char                    *event_type,
        pin_flist_t             *i_flistp,
        pin_flist_t             *err_flistp,
        pin_errbuf_t            *ebufp);

void
fm_mso_commission_error_processing (
        pcm_context_t           *ctxp,
        int             	error_no,
        char             	*error_in,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

/***************************************************
Return values:
0: Success
1: Error before calling
2: commission_model is NULL
3: commission_model =0 (No commission)
4: Invalid commission service
***************************************************/
int 
fm_mso_commission_get_vald_lco_acct_comm_mdl (
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lco_flistp = NULL;
	pin_flist_t		*err_flistp = NULL;
	//pin_flist_t		*lco_acct_obj_flistp = NULL;
	int32			*commission_model = NULL;
	int32			*commission_service = NULL;
	poid_t			*acct_objp = NULL;
	poid_t			*lco_acct_objp = NULL;
	//char			*lco_account_no = NULL;
	//char			*lco_legacy_account_no = NULL;

	int32			ret_val=1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_commission_get_vald_lco_acct_comm_mdl", ebufp);
		return ret_val;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_get_vald_lco_acct_comm_mdl input flist", i_flistp);

	/*Retrieve LCO Account and its commission model-START*/
	acct_objp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);

	if (acct_objp != NULL ) 
	{
		fm_mso_util_get_customer_lco_info(ctxp,acct_objp,&lco_flistp,ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_get_vald_lco_acct_comm_mdl fm_mso_util_get_customer_lco_info return flist", lco_flistp);
		if (lco_flistp != NULL) 
		{
			commission_model   = (int32*)PIN_FLIST_FLD_GET(lco_flistp, MSO_FLD_COMMISION_MODEL, 0, ebufp);
			commission_service = (int32*)PIN_FLIST_FLD_GET(lco_flistp, MSO_FLD_COMMISION_SERVICE, 0, ebufp);
			lco_acct_objp = PIN_FLIST_FLD_GET(lco_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);

			/*Validating the LCO Commission Model-START*/
			if (commission_model == NULL) {
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "LCO COMMISSION - Commission Model is not defined");
				//fm_mso_utils_prepare_error_flist(ctxp, lco_flistp, &err_flistp,"61012", "LCO COMMISSION - Commission Model is not defined",ebufp);
				ret_val = 2;
				goto cleanup;
			}
			else if (*commission_model == 0) {
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"No commission_model");
				//fm_mso_utils_prepare_status_flist(ctxp, lco_flistp, &err_flistp, PIN_FLD_ACCOUNT_OBJ, "LCO COMMISSION - No Agreement for this LCO Account, hence skipping the commission", ebufp);
				ret_val = 3;
				goto cleanup;
			}
			/*Validating the LCO Commission Model-END*/

			/*Validating the LCO Commission Service-START*/
			if (commission_service != NULL && (*commission_service < 0 || *commission_service > 2)) {
				//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "LCO COMMISSION - Commission Service is invalid");
				//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Error2");
				//fm_mso_utils_prepare_error_flist(ctxp, lco_flistp, &err_flistp, "61013", "LCO COMMISSION - Commission Service is invalid",ebufp);
				ret_val =4;
				goto cleanup;
			}
			/*Validating the LCO Commission Service-END*/

			*r_flistpp = lco_flistp;
			ret_val =0; //Success
			return ret_val;
		}
		else if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERRBUF_CLEAR(ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "LCO COMMISSION - LCO Account does not exists");
			fm_mso_utils_prepare_error_flist(ctxp, lco_flistp, &err_flistp, "61014", "LCO COMMISSION - LCO Account does not exists",ebufp);
		}
	}

cleanup:
	//PIN_FLIST_DESTROY_EX(&lco_flistp, NULL);
	*r_flistpp = lco_flistp;
	/*Retrieve LCO Account and its commission model-END*/
	return ret_val;
}

/********************************************
Return Values:

0: Success
1: Error before calling
2: commission_model is NULL
3: commission_model =0 (No commission)
4: Invalid commission service(valid: BB & CATV)

********************************************/
int
fm_mso_get_bu_commission_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		**out_flist,
	int32			flags,
	pin_errbuf_t		*ebufp) 
{
	pin_flist_t		*srch_i_flistp = NULL;
	pin_flist_t		*srch_r_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*wholesale_info = NULL;
//	pin_flist_t		*err_flistp = NULL;
	pin_flist_t		*result = NULL;

	poid_t			*srch_pdp = NULL;
	poid_t			*bu_acnt_pdp = NULL;

	int			srch_flags = 256;
	//int			count = 0;
	int			ret_val = 1;
	int32			*commission_model = NULL;
	int32			*commission_service = NULL;
	char			buff[128]="";

	//int64			db =1;
	char			*template = "select X from /profile/wholesale where F1.id = V1 and profile_t.poid_type = '/profile/wholesale' ";

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 1;
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bu_commission_info input flist", in_flist);

	if (flags == ACCT_TYPE_LCO)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "MSO_FLD_LCO_OBJ");
		bu_acnt_pdp = PIN_FLIST_FLD_GET(in_flist, MSO_FLD_LCO_OBJ, 1, ebufp );
	}
	else if (flags == ACCT_TYPE_DTR)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "MSO_FLD_DT_OBJ");
		bu_acnt_pdp = PIN_FLIST_FLD_GET(in_flist, MSO_FLD_DT_OBJ, 1, ebufp );
	}
	else if (flags == ACCT_TYPE_SUB_DTR)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "MSO_FLD_SDT_OBJ");
		bu_acnt_pdp = PIN_FLIST_FLD_GET(in_flist, MSO_FLD_SDT_OBJ, 1, ebufp );
	}

	sprintf(buff, "flags=%d", &flags) ;
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, buff);

	srch_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(PCM_GET_USERID(ctxp)),"/search",-1,ebufp);
	srch_i_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_i_flistp,PIN_FLD_POID,(void *)srch_pdp,ebufp);
	PIN_FLIST_FLD_SET(srch_i_flistp,PIN_FLD_FLAGS,(void *)&srch_flags,ebufp);
	PIN_FLIST_FLD_SET(srch_i_flistp,PIN_FLD_TEMPLATE,(void *)template,ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp,PIN_FLD_ARGS,1,ebufp);
	PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_ACCOUNT_OBJ,(void *)bu_acnt_pdp,ebufp);

	PIN_FLIST_ELEM_ADD(srch_i_flistp,PIN_FLD_RESULTS,0,ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bu_commission_info input flist", srch_i_flistp);
	PCM_OP(ctxp,PCM_OP_SEARCH,0,srch_i_flistp,&srch_r_flistp,ebufp);
	PIN_FLIST_DESTROY_EX(&srch_i_flistp, NULL);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bu_commission_info output flist", srch_r_flistp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error during search",ebufp);
		goto cleanup;
	}
	
	if (srch_r_flistp)
	{
		result         = PIN_FLIST_ELEM_GET(srch_r_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		if (result)
		{
			wholesale_info = PIN_FLIST_SUBSTR_GET(result, MSO_FLD_WHOLESALE_INFO, 1, ebufp);
			if (wholesale_info)
			{
				commission_model   = PIN_FLIST_FLD_GET(wholesale_info, MSO_FLD_COMMISION_MODEL, 1, ebufp );
				commission_service = PIN_FLIST_FLD_GET(wholesale_info, MSO_FLD_COMMISION_SERVICE, 1, ebufp );

				/*Validating the DT/SDT Commission Model-START*/
				if (commission_model == NULL) {
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "DT/SDT COMMISSION - Commission Model is not defined");
					//fm_mso_utils_prepare_error_flist(ctxp, in_flist, &err_flistp,"61015", "SDT/DT COMMISSION - Commission Model is not defined",ebufp);
					ret_val = 2;
					goto cleanup;
				}
				else if (*commission_model == 0) {
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"No commission_model");
					//fm_mso_utils_prepare_status_flist(ctxp, in_flist, &err_flistp, PIN_FLD_ACCOUNT_OBJ, "SDT/DT COMMISSION - No Agreement for this SDT/DT Account, hence skipping the commission", ebufp);
					ret_val = 3;
					goto cleanup;
				}
				/*Validating the LCO Commission Model-END*/

				/*Validating the LCO Commission Service-START*/
				if (commission_service != NULL && (*commission_service < 0 || *commission_service > 2)) {
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "SDT/DT COMMISSION - Commission Service is invalid");
					//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Error2");
					//fm_mso_utils_prepare_error_flist(ctxp, in_flist, &err_flistp, "61016", "SDT/DT COMMISSION - Commission Service is invalid",ebufp);
					ret_val =4;
					goto cleanup;
				}

				*out_flist = PIN_FLIST_COPY(wholesale_info, ebufp);
				ret_val =0; //Success
				return ret_val;
			}
		}
	}
cleanup:
	*out_flist = PIN_FLIST_COPY(wholesale_info, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_r_flistp, NULL);
	//*out_flist = err_flistp;

	return ret_val;
}

int
fm_mso_commission_acct_has_agreement(
        pcm_context_t           *ctxp,
        poid_t                  *lco_acct_objp,
        pin_errbuf_t            *ebufp) 
{
        pin_flist_t             *srch_i_flistp = NULL;
        pin_flist_t             *srch_r_flistp = NULL;
        pin_flist_t             *args_flistp = NULL;
        poid_t                  *srch_pdp = NULL;
        int                     srch_flags = 256;
        int                     count = 0;
        char                    *template = "select X from /purchased_product where F1 = V1 and status = 1 ";

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);
        srch_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(PCM_GET_USERID(ctxp)),"/search",-1,ebufp);
        srch_i_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_i_flistp,PIN_FLD_POID,(void *)srch_pdp,ebufp);
        PIN_FLIST_FLD_SET(srch_i_flistp,PIN_FLD_FLAGS,(void *)&srch_flags,ebufp);
        PIN_FLIST_FLD_SET(srch_i_flistp,PIN_FLD_TEMPLATE,(void *)template,ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp,PIN_FLD_ARGS,1,ebufp);
        PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_ACCOUNT_OBJ,(void *)lco_acct_objp,ebufp);

        PIN_FLIST_ELEM_ADD(srch_i_flistp,PIN_FLD_RESULTS,0,ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_acct_has_agreement input flist", srch_i_flistp);
        PCM_OP(ctxp,PCM_OP_SEARCH,0,srch_i_flistp,&srch_r_flistp,ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_acct_has_agreement output flist", srch_r_flistp);
        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error during search",ebufp);
                goto cleanup;
        }
        count = PIN_FLIST_ELEM_COUNT(srch_r_flistp, PIN_FLD_RESULTS, ebufp);

cleanup:
        PIN_FLIST_DESTROY_EX(&srch_i_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&srch_r_flistp, NULL);
        return count;
}

void
fm_mso_commission_record_failure (
        pcm_context_t           *ctxp,
        char                    *event_type,
        pin_flist_t             *i_flistp,
        pin_flist_t             *err_flistp,
        pin_errbuf_t            *ebufp) {



        pin_flist_t		*info_flistp = NULL;
        pin_flist_t		*r_info_flistp = NULL;
        pin_flist_t		*action_info_flistp = NULL;
        pin_flist_t		*prod_flistp = NULL;

        poid_t			*lco_comm_info_poidp = NULL;
        poid_t			*event_poidp = NULL;
        poid_t			*offering_poidp = NULL;
        poid_t			*item_pdp = NULL;
        poid_t			*dtr_acct_objp = NULL;
	poid_t			*d_pdp = NULL;

        int64			id = 1;
        int64			db = -1;
        //int32			processed_flag = 0; /* 0 - Not processed, 1 - Processed */
        int32			status = 0; /* 0 - Pending, 1 - Closed */
	int			local = 1;
	int			flistlen=0;


	pcm_context_t		*new_ctxp = NULL;

	pin_buf_t		*inp_flist_buf     = NULL;

	char			*fliststr = NULL;
	void			*vp = NULL;


        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_record_failure input flist1", i_flistp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_record_failure input flist2", err_flistp);

        event_poidp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

        /*Record information - LCO Account Details - Start*/
	db = PIN_POID_GET_DB(PCM_GET_USERID(ctxp));
        lco_comm_info_poidp = PIN_POID_CREATE(db, "/mso_comm_process_failure", -1, ebufp);
        info_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT (info_flistp, PIN_FLD_POID, lco_comm_info_poidp, ebufp);
		
		vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
		if (vp)
		{
			PIN_FLIST_FLD_SET(info_flistp, PIN_FLD_ACCOUNT_OBJ, vp, ebufp); 
		}

		vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SESSION_OBJ, 1, ebufp);
		if (vp)
		{
			PIN_FLIST_FLD_SET(info_flistp, PIN_FLD_SESSION_OBJ, vp, ebufp); 
		}

		vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_START_T, 1, ebufp);
		if (vp)
		{
			PIN_FLIST_FLD_SET(info_flistp, PIN_FLD_EFFECTIVE_T, vp, ebufp); 
		}
        PIN_FLIST_FLD_COPY (i_flistp, MSO_FLD_LCO_OBJ, info_flistp, MSO_FLD_LCO_OBJ, ebufp);
        PIN_FLIST_FLD_COPY (i_flistp, MSO_FLD_DT_OBJ, info_flistp, MSO_FLD_DT_OBJ, ebufp);
        PIN_FLIST_FLD_COPY (i_flistp, MSO_FLD_SDT_OBJ, info_flistp, MSO_FLD_SDT_OBJ, ebufp);
        PIN_FLIST_FLD_SET (info_flistp, PIN_FLD_EVENT_TYPE, (void*)event_type, ebufp);
        PIN_FLIST_FLD_SET (info_flistp, PIN_FLD_NUM_RETRIES, &status, ebufp);

	/***********************************
	Enrich with input flist buffer
	***********************************/
	if (i_flistp)
	{
		PIN_FLIST_TO_STR(i_flistp, &fliststr, &flistlen, ebufp );
		
		inp_flist_buf = ( pin_buf_t *) pin_malloc( strlen( fliststr ) + 1);
		if ( inp_flist_buf == NULL ) 
		{
			pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
			PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate nemory for inp_flist_buf", ebufp ); 
			goto CLEANUP;
		}

		if (fliststr != NULL )
		{
			inp_flist_buf->data   = ( caddr_t)fliststr;
			inp_flist_buf->size   = flistlen ;
		}
		else
		{
			inp_flist_buf->data   = (char *)NULL;
			inp_flist_buf->size   = 0;
		}

		inp_flist_buf->flag   = 0;
		inp_flist_buf->offset = 0;
		inp_flist_buf->xbuf_file = ( char *) NULL;

		PIN_FLIST_FLD_PUT( info_flistp, PIN_FLD_INPUT_FLIST, ( void *) inp_flist_buf, ebufp );


	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		goto CLEANUP;
	}



        dtr_acct_objp = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_DT_OBJ, 1, ebufp);
        if (PIN_POID_IS_NULL(dtr_acct_objp) == 0) {
            PIN_FLIST_FLD_COPY (err_flistp, MSO_FLD_DT_OBJ, info_flistp, MSO_FLD_DT_OBJ, ebufp);
        }
	else {
		vp = PIN_FLIST_FLD_GET(err_flistp, MSO_FLD_LCO_OBJ, 1, ebufp);
		if (vp)
		{
			PIN_FLIST_FLD_SET(info_flistp, MSO_FLD_LCO_OBJ, vp, ebufp); 
		}
	}

        //PIN_FLIST_FLD_SET (info_flistp, PIN_FLD_PROCESSED_FLAG, &processed_flag, ebufp);
        PIN_FLIST_FLD_SET (info_flistp, PIN_FLD_STATUS, &status, ebufp);

        item_pdp = PIN_FLIST_FLD_GET(err_flistp, PIN_FLD_ITEM_OBJ, 1, ebufp);
        if (item_pdp != NULL) {
                PIN_FLIST_FLD_COPY (err_flistp, PIN_FLD_ITEM_OBJ, info_flistp, PIN_FLD_ITEM_OBJ, ebufp);
				vp = PIN_FLIST_FLD_GET(err_flistp, MSO_FLD_COMMISSION_CHARGE, 1, ebufp);
				if (vp)
				{
					PIN_FLIST_FLD_SET(info_flistp, MSO_FLD_COMMISSION_CHARGE, vp, ebufp); 
				}
        }
        else {
                PIN_FLIST_FLD_SET (info_flistp, PIN_FLD_ITEM_OBJ, NULL, ebufp);
        }

        if ( PIN_FLIST_FLD_GET(err_flistp, PIN_FLD_ERROR_DESCR, 0, ebufp) != NULL ) {
                vp = PIN_FLIST_FLD_GET(err_flistp, PIN_FLD_ERROR_DESCR, 1, ebufp);
        }
        else {
				vp = PIN_FLIST_FLD_GET(err_flistp, PIN_FLD_DESCR, 1, ebufp);
        }
		if (vp)
		{
			PIN_FLIST_FLD_SET(info_flistp, PIN_FLD_STATUS_MSG, vp, ebufp); 
		}
        /*Record information - LCO Account Details - End*/
        if (item_pdp != NULL && PIN_POID_IS_NULL(item_pdp) != 0 ) {
            if ( strstr(event_type,"/event/billing/item/transfer")) {
                action_info_flistp = PIN_FLIST_ELEM_GET (i_flistp, PIN_FLD_ACTION_INFO, 0, 0, ebufp);
		   		vp = PIN_FLIST_FLD_GET(action_info_flistp, PIN_FLD_BUFFER, 1, ebufp);
				if (vp)
				{
					PIN_FLIST_FLD_SET(info_flistp, PIN_FLD_BUFFER, vp, ebufp); 
				}
            }
            if ( strstr(event_type,"/event/billing/product/fee/cycle")) {
                prod_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_PRODUCT, 1, ebufp);
                if (prod_flistp != NULL) {
                        offering_poidp = PIN_FLIST_FLD_GET(prod_flistp, PIN_FLD_OFFERING_OBJ, 1, ebufp);
                        PIN_FLIST_FLD_SET (info_flistp, PIN_FLD_OFFERING_OBJ, offering_poidp, ebufp);
                }

            }
        }
        PIN_ERRBUF_CLEAR(ebufp);
	PCM_CONTEXT_OPEN(&new_ctxp, (pin_flist_t *)0, ebufp);
	d_pdp = PIN_POID_CREATE(db, "/account", id, ebufp);
	local = fm_mso_trans_open(new_ctxp, d_pdp, 1, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_record_failure create record input flist", info_flistp);
        PCM_OP(new_ctxp, PCM_OP_CREATE_OBJ, 0, info_flistp, &r_info_flistp, ebufp);
        if ( PIN_ERRBUF_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in fm_mso_commission_record_failure create record", ebufp);
        }

	fm_mso_trans_commit(new_ctxp, d_pdp, ebufp);
	PCM_CONTEXT_CLOSE(new_ctxp, 0, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_record_failure create record output flist", r_info_flistp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_record_failure output flist", i_flistp);

CLEANUP:
        PIN_FLIST_DESTROY_EX(&info_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&r_info_flistp, NULL);
        PIN_POID_DESTROY(d_pdp, ebufp);

//	if (fliststr)
//	{
//		free (fliststr);
//	}
}

void
fm_mso_commission_error_processing (
        pcm_context_t           *ctxp,
	int             	err_no,
	char             	*err_in,
	pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{

	pin_flist_t             *ret_flistp;
	int32                   activate_service_failure = 1;
	char 			*err_code="51718";
	char			*tmp_err_desc=NULL;
	char			err_desc[1024];

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_error_processing input flist", i_flistp);

	switch (err_no) {
                case MSO_ERR_INVALID_TRANSFER_AMOUNT:
                        tmp_err_desc="Invalid Tranfer Amount.";
			break;
		case MSO_ERR_DTR_NO_ACTIVE_AGREEMENT:
                case MSO_ERR_LCO_NO_ACTIVE_AGREEMENT:
                        tmp_err_desc="No Active Agreement for LCO/DTR.";
			break;
                case MSO_ERR_LCO_COMMISSION_PROCESS_FAILED:
                        tmp_err_desc="Commission Process Failed";
			break;
                case MSO_ERR_LCO_INSUFFICIENT_BALANCE:
                        tmp_err_desc="Insufficient LCO Balance.";
			break;
                case MSO_ERR_LCO_CREDIT_LIMIT_EXCEEDED:
                        tmp_err_desc="LCO Credit Limit Exceeded";
			break;
                case MSO_ERR_INVALID_COMM_MDL_SERV:
                        tmp_err_desc="Invalid Commission Model";
			break;
                case MSO_ERR_LCO_BAL_BUCKET_NOT_FOUND:
                        tmp_err_desc="Balance Bucket Not Found for LCO/DTR.";
			break;
                case MSO_ERR_LCO_COMM_BUCKET_NOT_FOUND:
		case MSO_ERR_DTR_COMM_BUCKET_NOT_FOUND:
                        tmp_err_desc="Commission Bucket Not Found for LCO/DTR.";
			break;
                case MSO_ERR_DTR_COMMISSION_FAILURE:
                case MSO_ERR_LCO_COMMISSION_FAILURE:
                        tmp_err_desc="Commission failure for LCO/DTR.";
			break;
                case MSO_ERR_LCO_CREDIT_LIMIT_NOT_SET:
                        tmp_err_desc="LCO Credit Limit not set.";
			break;
		default:	
			tmp_err_desc="Error in processing";
	}
	sprintf(err_desc, "%s - %s", err_in, tmp_err_desc);
	ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, ret_flistp, PIN_FLD_POID, ebufp );
       	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &activate_service_failure, ebufp);
        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, err_code, ebufp);
        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, err_desc, ebufp);
	*r_flistpp = ret_flistp;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_error_processing output flist", *r_flistpp);
	
}
	
