/*******************************************************************
 * File:	fm_mso_cust_bb_get_info.c
 * Opcode:	MSO_OP_CUST_BB_GET_INFO 
 * Owner:	Leela Pratyush Vasireddy
 * Created:	25-JUL-2013
 * Description: This opcode is used for allocation of Device to Plan
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_cust_bb_get_info.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "ops/act.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "mso_cust.h"

#define MSO_FLAG_GET_CMTS_LIST 1
#define MSO_FLAG_GET_NODE_LIST 2
#define MSO_FLAG_GET_AMPLIFIER_LIST 3
#define DOCSIS_2_0 1
#define DOCSIS_3_0 2
#define ETHERNET 3
#define FIBER 4
#define WIRELESS 5

extern int32
fm_mso_trans_open(
        pcm_context_t   *ctxp,
        poid_t          *pdp,
        int32           flag,
        pin_errbuf_t    *ebufp);

extern void
fm_mso_trans_commit(
        pcm_context_t   *ctxp,
        poid_t          *pdp,
        pin_errbuf_t    *ebufp);

extern void
fm_mso_trans_abort(
        pcm_context_t   *ctxp,
        poid_t          *pdp,
        pin_errbuf_t    *ebufp);

static void
fm_mso_cust_bb_get_info(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_get_cmts_info(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

static void
fm_mso_get_node_info(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

static void
fm_mso_get_amplifier_info(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

/*******************************************************************
 * MSO_OP_CUST_BB_GET_INFO  
 *******************************************************************/

void 
op_mso_cust_bb_get_info(
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
	char            *prog_name = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Cnfg Master");	
	if (opcode != MSO_OP_CUST_BB_GET_INFO) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_bb_get_info error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_bb_get_info input flist", i_flistp);
	
	inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

	fm_mso_cust_bb_get_info(ctxp, flags, i_flistp, r_flistpp, ebufp);

 	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp) ) 
	{		
		PIN_ERRBUF_RESET(ebufp);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_bb_get_info error", ebufp);
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_bb_get_info output flist", *r_flistpp);
	}
	return;
}

/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_bb_get_info(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*out_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;

	poid_t			*routing_poid = NULL;
	
	int32			*search_flags = NULL ;
	int32                   cms_search = MSO_FLAG_GET_CMTS_LIST;
	int32                   node_search = MSO_FLAG_GET_NODE_LIST;
	int32                   amplifier_search = MSO_FLAG_GET_AMPLIFIER_LIST;	
	int32			bb_search_failure = 1; 

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_bb_get_info input flist", i_flistp);	
	
	routing_poid = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
	if ( !routing_poid )
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &bb_search_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51400", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Mandatory POID value not passed in input flist", ebufp);
		*r_flistpp = ret_flistp;
		return;		
	}
	search_flags = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp);
	if ( !search_flags )
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &bb_search_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51613", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Search type flag should be sent in input", ebufp);
		*r_flistpp = ret_flistp;
		return;		
	}
        if ( *(int32 *)search_flags == MSO_FLAG_GET_CMTS_LIST )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling search for getting CMTS info");
		fm_mso_get_cmts_info(ctxp, i_flistp, &out_flistp, ebufp);
	}
	else if ( *(int32 *)search_flags == MSO_FLAG_GET_NODE_LIST )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling search for getting Node info");
		fm_mso_get_node_info(ctxp, i_flistp, &out_flistp, ebufp);
	}
	else if ( *(int32 *)search_flags == MSO_FLAG_GET_AMPLIFIER_LIST )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling search for getting Amplifier info");
		fm_mso_get_amplifier_info(ctxp, i_flistp, &out_flistp, ebufp);
	}
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
/*		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &bb_search_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51614", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Config search error to retrieve bb info", ebufp); */
		*r_flistpp = out_flistp;
		return;		
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_bb_get_info output flist", out_flistp);
	*r_flistpp = PIN_FLIST_COPY(out_flistp,ebufp);
       	PIN_FLIST_DESTROY_EX(&out_flistp, NULL);
	return;
}

static void
fm_mso_get_cmts_info(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t		*search_in_flistp = NULL;
	pin_flist_t		*search_out_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;

	int32                   srch_flag = 256;
	int32                   get_bb_info_failure = 1;
	int32                   db = -1;
	int32			*technology = NULL;	

	poid_t			*srch_pdp = NULL;
	poid_t			*routing_obj = NULL;
	
	char			*city = NULL;
        char                    *template = "select X from /mso_cfg_cmts_master where F1 = V1 ";

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_bb_get_info input flist", in_flistp);	

	routing_obj = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1, ebufp);
	city = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_CITY, 1, ebufp);
	technology = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_TECHNOLOGY, 1, ebufp);
	
	if ( !technology || !city || city == (char *)NULL )
	{
                res_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_STATUS, &get_bb_info_failure, ebufp);
		PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_ERROR_CODE, "51400", ebufp);
                PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_ERROR_DESCR, "Mandatory fields not passed in input flist: city (or) technology", ebufp);
		return;		
	}
	if ( !(*(int32 *)technology == DOCSIS_2_0) && !(*(int32 *)technology == DOCSIS_3_0 ))
	{
                res_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_STATUS, &get_bb_info_failure, ebufp);
		PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_ERROR_CODE, "51400", ebufp);
                PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_ERROR_DESCR, "CMTS Search can be performed only for DOCSIS technology", ebufp);
	} 
	db = PIN_POID_GET_DB(routing_obj);
	search_in_flistp = PIN_FLIST_CREATE(ebufp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(search_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(search_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(search_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(search_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CITY, city, ebufp);
	result_flistp = PIN_FLIST_ELEM_ADD(search_in_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, MSO_FLD_CMTS_ID, (char *)NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, MSO_FLD_NE_ID, (char *)NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, MSO_FLD_NE_IP_ADDRESS, (char *)NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_cmts_info search flist", search_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_in_flistp, &search_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_in_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
                res_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_STATUS, &get_bb_info_failure, ebufp);
		PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_ERROR_CODE, "51615", ebufp);
		PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_get_cmts_info: CMTS ID does not exist", ebufp);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, " fm_mso_get_cmts_info: Error in searching CMTS info", ebufp);
		PIN_FLIST_DESTROY_EX(&search_out_flistp, NULL);
		*ret_flistp = res_flistp;
		return;
	}
	result_flistp = PIN_FLIST_ELEM_GET(search_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if ( !result_flistp )
	{
                res_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_STATUS, &get_bb_info_failure, ebufp);
		PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_ERROR_CODE, "51615", ebufp);
		PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_get_cmts_info: CMTS ID does not exist", ebufp);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, " fm_mso_get_cmts_info: No CMTS info exists for this city", ebufp);
		PIN_FLIST_DESTROY_EX(&search_out_flistp, NULL);
		*ret_flistp = res_flistp;
		return;
	}
	else
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_cmts_info:search result flist", search_out_flistp);
	*ret_flistp = PIN_FLIST_COPY(search_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_out_flistp, NULL);
	return;
}

static void
fm_mso_get_node_info(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t		*search_in_flistp = NULL;
	pin_flist_t		*search_out_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;

	int32                   srch_flag = 256;
	int32                   get_bb_info_failure = 1;
	int32                   db = -1;

	poid_t			*srch_pdp = NULL;
	poid_t			*routing_obj = NULL;
	
	char			*cmts_id = NULL;
        char                    *template = "select X from /mso_cfg_node_master where F1 = V1 ";

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_bb_get_info input flist", in_flistp);	

	routing_obj = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1, ebufp);
	cmts_id = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_CMTS_ID, 1, ebufp);
	
	if ( !cmts_id || cmts_id == (char *)NULL )
	{
                res_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_STATUS, &get_bb_info_failure, ebufp);
		PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_ERROR_CODE, "51400", ebufp);
                PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_ERROR_DESCR, "Mandatory fields not passed in input flist: city (or) technology", ebufp);
		return;		
	}
	db = PIN_POID_GET_DB(routing_obj);
	search_in_flistp = PIN_FLIST_CREATE(ebufp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(search_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(search_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(search_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(search_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_CMTS_ID, cmts_id, ebufp);
	result_flistp = PIN_FLIST_ELEM_ADD(search_in_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, MSO_FLD_NODE_ID, (char *)NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, MSO_FLD_NODE_IP_ADDRESS, (char *)NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_cmts_info search flist", search_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_in_flistp, &search_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_in_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
                res_flistp = PIN_FLIST_CREATE(ebufp);
//		PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_STATUS, &get_bb_info_failure, ebufp);
		PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_ERROR_CODE, "51616", ebufp);
		PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_get_cmts_info: CMTS ID does not exist", ebufp);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, " fm_mso_get_cmts_info: Error in searching CMTS info", ebufp);
		PIN_FLIST_DESTROY_EX(&search_out_flistp, NULL);
		*ret_flistp = res_flistp;
		return;
	}
	result_flistp = PIN_FLIST_ELEM_GET(search_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if ( !result_flistp )
	{
                res_flistp = PIN_FLIST_CREATE(ebufp);
//		PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_STATUS, &get_bb_info_failure, ebufp);
		PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_ERROR_CODE, "51616", ebufp);
		PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_get_cmts_info: CMTS ID does not exist", ebufp);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, " fm_mso_get_cmts_info: CMTS ID does not exist ", ebufp);
		PIN_FLIST_DESTROY_EX(&search_out_flistp, NULL);
		*ret_flistp = res_flistp;
		return;
	}
	else
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_cmts_info:search result flist", search_out_flistp);
	*ret_flistp = PIN_FLIST_COPY(search_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_out_flistp, NULL);
}

static void
fm_mso_get_amplifier_info(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t		*search_in_flistp = NULL;
	pin_flist_t		*search_out_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;

	int32                   srch_flag = 256;
	int32                   get_bb_info_failure = 1;
	int32                   db = -1;

	poid_t			*srch_pdp = NULL;
	poid_t			*routing_obj = NULL;
	
	char			*node_id = NULL;
        char                    *template = "select X from /mso_cfg_amplifier_master where F1 = V1 ";

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_bb_get_info input flist", in_flistp);	

	routing_obj = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1, ebufp);
	node_id = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_NODE_ID, 1, ebufp);
	
	if ( !node_id || node_id == (char *)NULL )
	{
                res_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_STATUS, &get_bb_info_failure, ebufp);
		PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_ERROR_CODE, "51400", ebufp);
                PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_ERROR_DESCR, "Mandatory fields not passed in input flist: Node_id", ebufp);
		return;		
	}
	db = PIN_POID_GET_DB(routing_obj);
	search_in_flistp = PIN_FLIST_CREATE(ebufp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(search_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(search_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(search_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(search_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_NODE_ID, node_id, ebufp);
	result_flistp = PIN_FLIST_ELEM_ADD(search_in_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, MSO_FLD_AMPLIFIER_ID, (char *)NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, MSO_FLD_AMPLIFIER_IP_ADDRESS, (char *)NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_cmts_info search flist", search_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_in_flistp, &search_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_in_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
                res_flistp = PIN_FLIST_CREATE(ebufp);
//		PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_STATUS, &get_bb_info_failure, ebufp);
		PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_ERROR_CODE, "51617", ebufp);
		PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_get_cmts_info: CMTS ID does not exist", ebufp);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, " fm_mso_get_cmts_info: Error in searching Amplifier info", ebufp);
		PIN_FLIST_DESTROY_EX(&search_out_flistp, NULL);
		*ret_flistp = res_flistp;
		return;
	}
	result_flistp = PIN_FLIST_ELEM_GET(search_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if ( !result_flistp )
	{
                res_flistp = PIN_FLIST_CREATE(ebufp);
//		PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_STATUS, &get_bb_info_failure, ebufp);
		PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_ERROR_CODE, "51617", ebufp);
		PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_get_cmts_info: CMTS ID does not exist", ebufp);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, " fm_mso_get_cmts_info: Node id does not exist", ebufp);
		PIN_FLIST_DESTROY_EX(&search_out_flistp, NULL);
		*ret_flistp = res_flistp;
		return;
	}
	else
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_cmts_info:search result flist", search_out_flistp);
	*ret_flistp = PIN_FLIST_COPY(search_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_out_flistp, NULL);
}

