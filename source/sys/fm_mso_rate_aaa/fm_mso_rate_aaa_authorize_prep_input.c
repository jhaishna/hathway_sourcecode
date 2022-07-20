/*******************************************************************
 * File:	fm_mso_rate_aaa_authorize_prep_input.c
 * Opcode:	MSO_OP_RATE_AAA_AUTHORIZE_PREP_INPUT
 * Owner:	Kaliprasad Mahunta
 * Created:	04-08-2015
 * Description:  Opcode to modify Input flist for start accounting
 *
********************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_rate_aaa_authorize_prep_input.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "ops/act.h"
#include "ops/tcf.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "mso_errs.h"
#include "mso_rate.h"

/*******************************************************************
 * MSO_OP_AAA_AUTHORIZE_PREP_INPUT
 *
 * 
 *******************************************************************/

EXPORT_OP void
op_mso_rate_aaa_authorize_prep_input(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_rate_aaa_authorize_prep_input(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);


/*******************************************************************
 * MSO_OP_RATE_AAA_AUTHORIZE_PREP_INPUT  
 *******************************************************************/
void 
op_mso_rate_aaa_authorize_prep_input(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	 pin_flist_t             *r_flistp = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_RATE_AAA_AUTHORIZE_PREP_INPUT) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_rate_aaa_authorize_prep_input error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_rate_aaa_authorize_prep_input input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	fm_mso_rate_aaa_authorize_prep_input(ctxp, flags, i_flistp, &r_flistp, ebufp);
 	/***********************************************************
	 * Results.
	 ***********************************************************/

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                *r_flistpp = (pin_flist_t *)NULL;
                PIN_FLIST_DESTROY(r_flistp, NULL);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_mso_rate_aaa_authorize_prep_input error", ebufp);
        } else {
                *r_flistpp = r_flistp;
                PIN_ERRBUF_CLEAR(ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "op_mso_rate_aaa_authorize_prep_input return flist", *r_flistpp);
		//PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
        }
	return;
}

/*******************************************************************
 * This is the default implementation for this opcode
 *******************************************************************/
static void 
fm_mso_rate_aaa_authorize_prep_input(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*telco_flistp = NULL;
	pin_flist_t		*telco_info_r = NULL;
	pin_flist_t		*extended_flistp = NULL;
	pin_flist_t		*session_flistp = NULL;
	int32			operation_type=0;	
	int32			elem_count=0;	
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_authorize_prep_input input Flist", i_flistp);
	/*
	 * Algorithm for the opcode
	 * 1. Check for Mandatory parameters
	 * 2. based on acct status type, For START and UPDATE do the operation
	 * 		1 -> AUTHORIZE, 3-> REAUTHORIZE, 2-> STOP_ACCOUNTING
	 * 3. Search for MSO_FLD_BB_INFO.
	 * 6. If found copy the fields present to PIN_FLD_TELCO_INFO 
	 * 7.If not, Call the helper opcode.
	 */
	 
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_mso_rate_aaa_authorize_prep_input error",ebufp);
		return;
	}
	
	operation_type = *(int *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_REQ_MODE,0, ebufp);
	
	if(operation_type == 4){

		//get PIN_FLD_TELCO_INFO pointer and populate data on the flist
		telco_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_TELCO_INFO, PIN_ELEMID_ANY, 1, ebufp);


                //check for PIN_FLD_EXTENDED_INFO, if found copy and remove
                extended_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_EXTENDED_INFO, 0, ebufp);

                if(extended_flistp!= (pin_flist_t *)NULL){
                        elem_count = PIN_FLIST_ELEM_COUNT(extended_flistp, PIN_FLD_SESSION_INFO, ebufp);
                        if(elem_count > 0 ) {

                                session_flistp = PIN_FLIST_ELEM_GET(extended_flistp, PIN_FLD_SESSION_INFO, PIN_ELEMID_ANY, 1, ebufp);
                                PIN_FLIST_FLD_COPY(session_flistp,MSO_FLD_FRAMED_IP_ADDRESS,telco_flistp, MSO_FLD_FRAMED_IP_ADDRESS, ebufp);
                                PIN_FLIST_FLD_COPY(session_flistp,MSO_FLD_NE_IP_ADDRESS,telco_flistp, MSO_FLD_NE_IP_ADDRESS, ebufp);
                                PIN_FLIST_FLD_COPY(session_flistp,MSO_FLD_NE_ID,telco_flistp, MSO_FLD_NE_ID, ebufp);
                                PIN_FLIST_FLD_COPY(session_flistp,PIN_FLD_USER_NAME,telco_flistp, PIN_FLD_USER_NAME, ebufp);
                                PIN_FLIST_FLD_COPY(session_flistp,MSO_FLD_REMOTE_ID,telco_flistp, MSO_FLD_REMOTE_ID, ebufp);
                                PIN_FLIST_FLD_COPY(session_flistp,PIN_FLD_SERVICE_CODE,telco_flistp, PIN_FLD_SERVICE_CODE, ebufp);
                                PIN_FLIST_FLD_COPY(session_flistp,MSO_FLD_BYTES_UPLINK_BEF_FUP,telco_flistp, MSO_FLD_BYTES_UPLINK_BEF_FUP, ebufp);
                                PIN_FLIST_FLD_COPY(session_flistp,MSO_FLD_BYTES_DOWNLINK_BEF_FUP,telco_flistp, MSO_FLD_BYTES_DOWNLINK_BEF_FUP, ebufp);
                                PIN_FLIST_FLD_COPY(session_flistp,MSO_FLD_OLD_NETWORKID,telco_flistp, MSO_FLD_OLD_NETWORKID, ebufp);
                        }

                        PIN_FLIST_SUBSTR_DROP(i_flistp,PIN_FLD_EXTENDED_INFO,ebufp);
                }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_authorize_prep_input after drop session info input Flist", i_flistp);


	}

	//call PCM_OP_TCF_AAA_AUTHORIZE_PREP_INPUT

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_authorize_prep_input before helper opcode call input Flist", i_flistp);
	PCM_OP(ctxp, PCM_OP_TCF_AAA_AUTHORIZE_PREP_INPUT, 0, i_flistp, r_flistpp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_authorize_prep_input before helper opcode call return Flist", i_flistp);

        //check for PIN_FLD_EXTENDED_INFO, if found copy and remove
        extended_flistp = PIN_FLIST_SUBSTR_GET(*r_flistpp, PIN_FLD_EXTENDED_INFO, 0, ebufp);
	elem_count = 0;

        if(extended_flistp!= (pin_flist_t *)NULL){
                elem_count = PIN_FLIST_ELEM_COUNT(extended_flistp, PIN_FLD_TELCO_INFO, ebufp);
                if(elem_count > 0 ) {

                        telco_info_r = PIN_FLIST_ELEM_GET(extended_flistp, PIN_FLD_TELCO_INFO, PIN_ELEMID_ANY, 1, ebufp);
                        PIN_FLIST_FLD_COPY(telco_flistp,MSO_FLD_FRAMED_IP_ADDRESS,telco_info_r, MSO_FLD_FRAMED_IP_ADDRESS, ebufp);
                        PIN_FLIST_FLD_COPY(telco_flistp,MSO_FLD_NE_IP_ADDRESS,telco_info_r, MSO_FLD_NE_IP_ADDRESS, ebufp);
                        PIN_FLIST_FLD_COPY(telco_flistp,MSO_FLD_NE_ID,telco_info_r, MSO_FLD_NE_ID, ebufp);
                        PIN_FLIST_FLD_COPY(telco_flistp,PIN_FLD_USER_NAME,telco_info_r, PIN_FLD_USER_NAME, ebufp);
                        PIN_FLIST_FLD_COPY(telco_flistp,MSO_FLD_REMOTE_ID,telco_info_r, MSO_FLD_REMOTE_ID, ebufp);
                        PIN_FLIST_FLD_COPY(telco_flistp,PIN_FLD_SERVICE_CODE,telco_info_r, PIN_FLD_SERVICE_CODE, ebufp);
                        PIN_FLIST_FLD_COPY(telco_flistp,PIN_FLD_BYTES_IN,telco_info_r, PIN_FLD_BYTES_IN, ebufp);
                        PIN_FLIST_FLD_COPY(telco_flistp,PIN_FLD_BYTES_OUT,telco_info_r, PIN_FLD_BYTES_OUT, ebufp);
                        PIN_FLIST_FLD_COPY(telco_flistp,MSO_FLD_OLD_NETWORKID,telco_info_r, MSO_FLD_OLD_NETWORKID, ebufp);
                }

        }

	
	return;
}
