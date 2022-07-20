/*******************************************************************
 * File:	fm_mso_cust_get_pkg_channel_attr.c
 * Opcode:	MSO_OP_CUST_GET_PKG_CHANNELS 
 * Created:	18-DEC-2018	
 * Description: 
 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 * This opcode is to retrieve channel attributes
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_get_pkg_channel_attr.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "ops/act.h"
#include "ops/cust.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"

/*******************************************************************
 * MSO_OP_CUST_GET_PKG_CHANNELS 
 *******************************************************************/

EXPORT_OP void
op_mso_cust_get_pkg_channel_attr(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_get_pkg_channel_attr(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * MSO_OP_CUST_GET_PKG_CHANNELS  
 *******************************************************************/
void 
op_mso_cust_get_pkg_channel_attr(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	*r_flistpp		= NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_GET_PKG_CHANNEL_ATTR) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_get_pkg_channel_attr error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_get_pkg_channel_attr input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	fm_mso_cust_get_pkg_channel_attr(ctxp, flags, i_flistp, r_flistpp, ebufp);
 	/***********************************************************
	 * Results.
	 ***********************************************************/

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_get_pkg_channel_attr error", ebufp);
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_get_pkg_channel_attr output flist", *r_flistpp);
	}
	return;
}

/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_get_pkg_channel_attr(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{

	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*channel_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;

	poid_t			*plan_pdp = NULL;
	poid_t			*s_pdp = NULL;

	int64			db = 1;
	int32			sflag = 256;
	int32			count = 0;
	int32			set_get_pkg_channel_map_failure = 1;
	char			*template = "select X from /mso_cfg_catv_channel_master 1 , /mso_cfg_catv_pt_channel_map 2 where 2.F1 = V1 and 2.F2 = 1.F3 ";
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_pkg_channel_attr input flist", i_flistp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"Error in calling fm_mso_cust_get_pkg_channel_attr", ebufp);
	}

	plan_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

	if (plan_pdp != NULL && !PIN_POID_IS_NULL(plan_pdp)) {

		s_flistp = PIN_FLIST_CREATE(ebufp);
		s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
		PIN_FLIST_FLD_PUT(s_flistp,PIN_FLD_POID, s_pdp, ebufp);
		PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_FLAGS, (void*)&sflag, ebufp);
		PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_TEMPLATE, template, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
		channel_flistp = PIN_FLIST_ELEM_ADD(args_flistp, MSO_FLD_CATV_CHANNELS, 0, ebufp);
		PIN_FLIST_FLD_SET(channel_flistp, MSO_FLD_CHANNEL_NAME, "", ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_CHANNEL_NAME, "", ebufp);
		res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp );

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_cust_get_pkg_channel_attr search i_flistp", s_flistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &ret_flistp,ebufp); 
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_cust_get_pkg_channel_attr search o_flistp", ret_flistp);

		count = PIN_FLIST_ELEM_COUNT(ret_flistp, PIN_FLD_RESULTS, ebufp);

		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_cust_get_pkg_channel_attr error",ebufp);
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID,ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &set_get_pkg_channel_map_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "84001", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Get PKG Channal Attribute Search Error", ebufp);
		}

		if (count == 0) {
			PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
                	ret_flistp = PIN_FLIST_CREATE(ebufp);
                	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID,ebufp);
                	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &set_get_pkg_channel_map_failure, ebufp);
                	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "84000", ebufp);
                	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "No PKG Channel Attribute found", ebufp);
		}
		else{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_pkg_channel_attr return flist", ret_flistp);
		}

	}
	else{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID,ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &set_get_pkg_channel_map_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "84002", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "PLAN OBJ is null", ebufp);
		
	}
	*r_flistpp = ret_flistp;
	
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);

	return;
}
