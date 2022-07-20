/*******************************************************************
 * File:	fm_mso_utils_mta_config_billing.c
 * Opcode:	MSO_OP_UTILS_MTA_CONFIG_BILLING 
 * Owner:	Gautam Adak
 * Created:	17-SEPT-2013
 * Description: 
 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_utils_mta_config_billing.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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



/**************************************
* External Functions start
***************************************/
/**************************************
* External Functions end
***************************************/

/*******************************************************************
 * MSO_OP_UTILS_MTA_CONFIG_BILLING 
 *
 * 
 *******************************************************************/

EXPORT_OP void
op_mso_utils_mta_config_billing(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_utils_mta_config_billing(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * MSO_OP_UTILS_MTA_CONFIG_BILLING  
 *******************************************************************/
void 
op_mso_utils_mta_config_billing(
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
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_UTILS_MTA_CONFIG_BILLING) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_utils_mta_config_billing error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_utils_mta_config_billing input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	fm_mso_utils_mta_config_billing(ctxp, i_flistp, r_flistpp, ebufp);
 	/***********************************************************
	 * Results.
	 ***********************************************************/

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_utils_mta_config_billing error", ebufp);
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_utils_mta_config_billing output flist", *r_flistpp);
	}
	return;
}



/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_utils_mta_config_billing(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{

	pin_flist_t     *app_flistp = NULL;
	pin_flist_t     *application_info = NULL;
	pin_flist_t     *srch_flistp = NULL;
	pin_flist_t     *param_flistp = NULL;

	char		*prog_name = NULL;
	char		*param_name = NULL;
	char		*param_value = NULL;
	char		*account_no = NULL;

	int32		elemid = 0;
	int32		acnt_pdp = -1;
	int32		bill_segment = -1;
	int32		postpay_indicator = 1;
	int32		prepay_indicator = 2;
	int32		indicator = 3;

	pin_cookie_t	cookie = NULL;
	

    	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_mta_config_billing input flist", in_flistp);
	app_flistp = PIN_FLIST_SUBSTR_GET(in_flistp, PIN_FLD_APPLICATION_INFO, 1, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_mta_config_billing app_flistp flist", in_flistp);

	if (app_flistp && app_flistp != NULL)
	{
		prog_name = (char *)PIN_FLIST_FLD_GET(app_flistp, PIN_FLD_NAME, 0, ebufp);
	}

	if (prog_name && strcmp(prog_name, "pin_cycle_fees") == 0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "pin_cycle_fees");
		while((param_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_PARAMS,&elemid, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			param_name = (char *)PIN_FLIST_FLD_GET(param_flistp, PIN_FLD_PARAM_NAME, 1, ebufp);
			if(param_name && strcmp(param_name,"-account") == 0)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "-account");
				param_value = (char *) PIN_FLIST_FLD_GET(param_flistp, PIN_FLD_PARAM_VALUE, 1, ebufp);
				acnt_pdp = atoi(param_value);
				application_info = PIN_FLIST_SUBSTR_GET(in_flistp, PIN_FLD_APPLICATION_INFO, 1, ebufp);
				PIN_FLIST_FLD_SET(application_info, PIN_FLD_ACCOUNT_NUM, &acnt_pdp, ebufp);
			}
	
			else if(param_name && strcmp(param_name,"-account_no") == 0)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "-account_no");
				account_no = (char *) PIN_FLIST_FLD_GET(param_flistp, PIN_FLD_PARAM_VALUE, 1, ebufp);
				application_info = PIN_FLIST_SUBSTR_GET(in_flistp, PIN_FLD_APPLICATION_INFO, 1, ebufp);
				PIN_FLIST_FLD_SET(application_info, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
			}
	
			if(param_name && strcmp(param_name, "-segment") == 0)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "-segment");
				param_value = (char *) PIN_FLIST_FLD_GET(param_flistp, PIN_FLD_PARAM_VALUE, 1, ebufp);
				bill_segment = atoi(param_value);
				application_info = PIN_FLIST_SUBSTR_GET(in_flistp, PIN_FLD_APPLICATION_INFO, 1, ebufp);
				PIN_FLIST_FLD_SET(application_info, PIN_FLD_BILLING_SEGMENT, &bill_segment, ebufp);
			}
			if(param_name && strcmp(param_name, "-cust_type") == 0)
             		{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Grant option for resetting quota for BB accounts");
				param_value = (char *) PIN_FLIST_FLD_GET(param_flistp, PIN_FLD_PARAM_VALUE, 1, ebufp);
				application_info = PIN_FLIST_SUBSTR_GET(in_flistp, PIN_FLD_APPLICATION_INFO, 1, ebufp);
				if(param_value!= NULL && strcmp(param_value, "prepay")==0)
				{
					PIN_FLIST_FLD_SET(application_info, PIN_FLD_INDICATOR, &prepay_indicator, ebufp);	
				}
				else if (param_value!= NULL && strcmp(param_value, "postpay")==0)
				{
					PIN_FLIST_FLD_SET(application_info, PIN_FLD_INDICATOR, &postpay_indicator, ebufp);
				}
				else if (param_value!= NULL && strcmp(param_value, "both")==0)
				{
					PIN_FLIST_FLD_SET(application_info, PIN_FLD_INDICATOR, &indicator, ebufp);
				}
				else
				{
					PIN_ERR_LOG_ERR(PIN_ERR_LEVEL_ERROR, "Invalid parameters for cust_type option");
					pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_MISSING_ARG, 0, 0, 0);
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "op_mso_utils_mta_config_billing error",	ebufp);
					return;
				}
		  	}
		}
	}
 	else if (prog_name && strcmp(prog_name, "pin_inv_accts") == 0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "pin_inv_accts");
		while((param_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_PARAMS,&elemid, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			param_name = (char *)PIN_FLIST_FLD_GET(param_flistp, PIN_FLD_PARAM_NAME, 1, ebufp);
			if(param_name && strcmp(param_name,"-segment") == 0)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "-segment");
				param_value = (char *) PIN_FLIST_FLD_GET(param_flistp, PIN_FLD_PARAM_VALUE, 1, ebufp);
				bill_segment = atoi(param_value);
				application_info = PIN_FLIST_SUBSTR_GET(in_flistp, PIN_FLD_APPLICATION_INFO, 1, ebufp);
				PIN_FLIST_FLD_SET(application_info, PIN_FLD_BILLING_SEGMENT, &bill_segment, ebufp);
			}
		}

	}


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_mta_config_billing app_flistp modified flist", in_flistp);

	*r_flistpp = PIN_FLIST_COPY(in_flistp, ebufp);
	return;



}

