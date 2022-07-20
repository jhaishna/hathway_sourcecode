/*******************************************************************
 * File:	fm_mso_rate_aaa_resubmit_suspense_event.c
 * Opcode:	MSO_OP_RATE_AAA_RESUBMIT_SUSPENSE_EVENT 
 * Owner:	
 * Created:	
 * Description: 
 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 * Input Flist
 * 
 * PIN_FLD_POID	POID 		[0] 0.0.0.1 /mso_aaa_suspended_bb_event -1 0
 * PIN_FLD_ACCOUNT_OBJ		[0]	0.0.0.1 /account 1234 0
 * PIN_FLD_AUTHORIZATION_ID [0]	"SESSION-ID"
 * PIN_FLD_REQ_MODE			[0]	1/2/3
 * PIN_FLD_USER_NAME		[0]	"BB_test_user_001"
 * PIN_FLD_ERROR_CODE		[0]	10010
 * PIN_FLD_INPUT_FLIST		[0]	Input Flist to the AAA operation			
 * 
 * 
 */
 
 #ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_rate_aaa_resubmit_suspense_event.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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
#include "mso_lifecycle_id.h"



EXPORT_OP void
op_mso_rate_aaa_resubmit_suspense_event(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_rate_aaa_resubmit_suspense_event(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

extern
void fm_mso_create_lifecycle_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	int32			action,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * MSO_OP_RATE_AAA_RESUBMIT_SUSPENSE_EVENT  
 *******************************************************************/
void
op_mso_rate_aaa_resubmit_suspense_event(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp) {
		
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
	if (opcode != MSO_OP_RATE_AAA_RESUBMIT_SUSPENSE_EVENT) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_rate_aaa_search_susp_event error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_rate_aaa_resubmit_suspense_event input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	fm_mso_rate_aaa_resubmit_suspense_event(ctxp, flags, i_flistp, r_flistpp, ebufp);
 	/***********************************************************
	 * Results.
	 ***********************************************************/

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_rate_aaa_resubmit_suspense_event error", ebufp);
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_rate_aaa_resubmit_suspense_event output flist", *r_flistpp);
	}
	return;
		
}

void
fm_mso_rate_aaa_resubmit_suspense_event(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp) 
{
		
	pin_buf_t		*flist_buf = NULL;
	char			*flist_str;
	pin_flist_t		*aaa_input_flistp    = NULL;
	pin_flist_t		*aaa_output_flistp   = NULL;
	pin_flist_t		*read_obj_in_flistp  = NULL;
	pin_flist_t		*read_obj_out_flistp = NULL;
	pin_flist_t		*lc_inflistp = NULL;
	poid_t			*evt_pdp = NULL;
	int32			status = 0;
	int32			resubmit_flag = 1;
	

	evt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	
	lc_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, lc_inflistp, PIN_FLD_POID, ebufp );
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, lc_inflistp, PIN_FLD_PROGRAM_NAME, ebufp );

	read_obj_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_obj_in_flistp, PIN_FLD_POID, evt_pdp, ebufp);

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Reading suspense event.. ");
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_obj_in_flistp, &read_obj_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_mso_rate_aaa_resubmit_suspense_event read obj output flist: ", read_obj_out_flistp);
	PIN_FLIST_DESTROY_EX(&read_obj_in_flistp, NULL);

	flist_buf = PIN_FLIST_FLD_GET(read_obj_out_flistp, PIN_FLD_INPUT_FLIST, 0, ebufp);
	flist_str = flist_buf->data;
	*r_flistpp = PIN_FLIST_CREATE(ebufp);
        //PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, pin_bufp->data);

        /* Convert the buffer to a flist */
	PIN_STR_TO_FLIST(flist_str, 1, &aaa_input_flistp, ebufp);
	PIN_FLIST_FLD_COPY(read_obj_out_flistp, PIN_FLD_AUTHORIZATION_ID, lc_inflistp, PIN_FLD_AUTHORIZATION_ID, ebufp );
 	PIN_FLIST_FLD_COPY(read_obj_out_flistp, PIN_FLD_USER_NAME, lc_inflistp, PIN_FLD_USER_NAME, ebufp );


	/* Add the flag to indicate it is a resubmitted one. */
	PIN_FLIST_FLD_SET(aaa_input_flistp, PIN_FLD_FLAGS, &resubmit_flag, ebufp);

	/* Call BB AAA wrapper opcode with this input flist.*/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_rate_aaa_resubmit_suspense_event calling MSO_OP_RATE_AAA_RATE_BB_EVENT with input flist: ", aaa_input_flistp);
	PCM_OP(ctxp,MSO_OP_RATE_AAA_RATE_BB_EVENT,0, aaa_input_flistp, &aaa_output_flistp, ebufp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_mso_rate_aaa_resubmit_suspense_event err calling MSO_OP_RATE_AAA_RATE_BB_EVENT",ebufp);
		PIN_FLIST_DESTROY_EX(&aaa_input_flistp, NULL);
		status = 1;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_rate_aaa_resubmit_suspense_event MSO_OP_RATE_AAA_RATE_BB_EVENT output flist: ", aaa_output_flistp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status, ebufp);

	if (status == 0)
	{
		fm_mso_create_lifecycle_evt(ctxp, lc_inflistp, aaa_input_flistp, ID_RESUBMIT_SUSPENDED_EVENT, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
			PIN_ERRBUF_RESET(ebufp);
		}
	}

	PIN_FLIST_DESTROY_EX(&lc_inflistp, NULL);
	PIN_FLIST_DESTROY_EX(&aaa_input_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&aaa_output_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&read_obj_out_flistp, NULL);
	return;
}

 
 
