/*******************************************************************
 * File:	fm_mso_rate_aaa_create_susp_event.c
 * Opcode:	MSO_OP_RATE_AAA_CREATE_SUSP_EVENT 
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
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_rate_aaa_create_susp_event.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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


EXPORT_OP void
op_mso_rate_aaa_create_susp_event(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_rate_aaa_create_susp_event(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * MSO_OP_RATE_AAA_CREATE_SUSP_EVENT  
 *******************************************************************/
 void
op_mso_rate_aaa_create_susp_event(
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
	if (opcode != MSO_OP_RATE_AAA_CREATE_SUSP_EVENT) {
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
		"op_mso_rate_aaa_create_susp_event input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	fm_mso_rate_aaa_create_susp_event(ctxp, flags, i_flistp, r_flistpp, ebufp);
 	/***********************************************************
	 * Results.
	 ***********************************************************/

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_rate_aaa_create_susp_event error", ebufp);
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_rate_aaa_create_susp_event output flist", *r_flistpp);
	}
	return;
		
}


void
fm_mso_rate_aaa_create_susp_event(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp) {
		
	PIN_ERRBUF_CLEAR(ebufp);
	PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, i_flistp, r_flistpp, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
	    PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in creating the suspense event object", ebufp);
	    return;
	}		
	return;
}

 
 
