/*******************************************************************
 * File:	fm_mso_cust_get_device_details.c
 * Opcode:	MSO_OP_CUST_GET_DEVICE_DETAILS 
 * Owner:	Gautam Adak
 * Created:	17-SEPT-2013
 * Description: 
 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 * Input Flist
 *
	0 PIN_FLD_USERID         POID [0] 0.0.0.1 /account 88888
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 88888
	0 PIN_FLD_ALIAS_LIST    ARRAY [0] allocated 20, used 1
	1     PIN_FLD_NAME        STR [0] "280920130027"
	0 PIN_FLD_ALIAS_LIST    ARRAY [1] allocated 20, used 1
	1     PIN_FLD_NAME        STR [0] "3810201300028127"
	0 PIN_FLD_ALIAS_LIST    ARRAY [2] allocated 20, used 1
	1     PIN_FLD_NAME        STR [0] "3810208971300028127"
********************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_get_device_details.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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
 * MSO_OP_CUST_GET_DEVICE_DETAILS 
 *
 * 
 *******************************************************************/

EXPORT_OP void
op_mso_cust_get_device_details(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_get_device_details(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

PIN_IMPORT int32
fm_mso_is_den_sp(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_errbuf_t            *ebufp);


/*******************************************************************
 * MSO_OP_CUST_GET_DEVICE_DETAILS  
 *******************************************************************/
void 
op_mso_cust_get_device_details(
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
	if (opcode != MSO_OP_CUST_GET_DEVICE_DETAILS) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_get_device_details error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_get_device_details input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	fm_mso_cust_get_device_details(ctxp, flags, i_flistp, r_flistpp, ebufp);
 	/***********************************************************
	 * Results.
	 ***********************************************************/

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_get_device_details error", ebufp);
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_get_device_details output flist", *r_flistpp);
	}
	return;
}



/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_get_device_details(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*alias_list = NULL;
	pin_flist_t		*alias_list_1 = NULL;
	pin_flist_t		*alias_list_2 = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*result_flist = NULL;

	poid_t			*user_id = NULL;
	poid_t			*acnt_pdp = NULL;
	poid_t			*srch_pdp = NULL;

	char			template_1[256] = "select X from /device where  F1 = V1 " ;
	char			template_2[256] = "select X from /device where  F1 in ( V1, V2) " ;
	char			template_3[256] = "select X from /device where  F1 in ( V1, V2, V3 ) " ;
	char			*stb_id = NULL;
	char			*vc_id = NULL;
	char			*mac_id = NULL;

	int64			db = -1;
	int32			device_count = 0;
	int32			srch_flag = 512;

	pin_cookie_t		cookie = NULL;
	int			elem_id = 0;
	int			arg_elem_id = 1;
	int32			den_sp = 0;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_device_details input flist", i_flistp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_cust_get_device_details", ebufp);
		goto CLEANUP;
	}

	user_id =  PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID , 1, ebufp );
	acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp ); 

	db = PIN_POID_GET_DB(user_id);

	device_count = PIN_FLIST_ELEM_COUNT(i_flistp, PIN_FLD_ALIAS_LIST, ebufp);

	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);

/*
	if (device_count == 1 )
	{
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template_1 , ebufp);
		
		// changes done to retrive the any element id that came in the input instead of retriving the 0th id //	
		//alias_list = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp) ;
		alias_list = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_ALIAS_LIST,PIN_ELEMID_ANY ,0, ebufp) ;
		stb_id = PIN_FLIST_FLD_GET(alias_list, PIN_FLD_NAME, 0, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS,1 , ebufp);
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_DEVICE_ID, stb_id, ebufp );
	}
	else if (device_count == 2)
	{
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template_2 , ebufp);

		alias_list = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
		stb_id = PIN_FLIST_FLD_GET(alias_list, PIN_FLD_NAME, 0, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_DEVICE_ID, stb_id, ebufp );

		alias_list_1 = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_ALIAS_LIST, 1, 0, ebufp) ;
		vc_id = PIN_FLIST_FLD_GET(alias_list_1, PIN_FLD_NAME, 0, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_DEVICE_ID, vc_id, ebufp );
	}
	else if (device_count == 3)
	{
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template_3 , ebufp);

		alias_list = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_ALIAS_LIST, 0, 0, ebufp);
		stb_id = PIN_FLIST_FLD_GET(alias_list, PIN_FLD_NAME, 0, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_DEVICE_ID, stb_id, ebufp );

		alias_list_1 = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_ALIAS_LIST, 1, 0, ebufp);
		vc_id = PIN_FLIST_FLD_GET(alias_list_1, PIN_FLD_NAME, 0, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_DEVICE_ID, vc_id, ebufp );

		alias_list_2 = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_ALIAS_LIST, 2, 0, ebufp);
		mac_id = PIN_FLIST_FLD_GET(alias_list_2, PIN_FLD_NAME, 0, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_DEVICE_ID, mac_id, ebufp );
	}
*/

	while((alias_list = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_ALIAS_LIST,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, arg_elem_id, ebufp);
		PIN_FLIST_FLD_COPY(alias_list, PIN_FLD_NAME, arg_flist, PIN_FLD_DEVICE_ID, ebufp );
		arg_elem_id++;
	}

	den_sp = fm_mso_is_den_sp(ctxp, i_flistp, ebufp);

	if (den_sp == 1)
	{
			strcat(template_1, " and nvl(device_t.MANUFACTURER,'DUMMY') != 'NDS' and nvl(device_t.MANUFACTURER,'DUMMY') != 'NDS2' ");
			strcat(template_2, " and nvl(device_t.MANUFACTURER,'DUMMY') != 'NDS' and nvl(device_t.MANUFACTURER,'DUMMY') != 'NDS2' ");
			strcat(template_3, " and nvl(device_t.MANUFACTURER,'DUMMY') != 'NDS' and nvl(device_t.MANUFACTURER,'DUMMY') != 'NDS2' ");
	}
    else if (den_sp == 2)
	{
			strcat(template_1, " and nvl(device_t.MANUFACTURER,'DUMMY') != 'NDS' and nvl(device_t.MANUFACTURER,'DUMMY') != 'NDS1' ");
			strcat(template_2, " and nvl(device_t.MANUFACTURER,'DUMMY') != 'NDS' and nvl(device_t.MANUFACTURER,'DUMMY') != 'NDS1' ");
			strcat(template_3, " and nvl(device_t.MANUFACTURER,'DUMMY') != 'NDS' and nvl(device_t.MANUFACTURER,'DUMMY') != 'NDS1' ");
	}
	else
	{
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "1 exists");
			strcat(template_1, " and nvl(device_t.MANUFACTURER,'DUMMY') != 'NDS1' and nvl(device_t.MANUFACTURER,'DUMMY') != 'NDS2' ");
			strcat(template_2, " and nvl(device_t.MANUFACTURER,'DUMMY') != 'NDS1' and nvl(device_t.MANUFACTURER,'DUMMY') != 'NDS2' ");
			strcat(template_3, " and nvl(device_t.MANUFACTURER,'DUMMY') != 'NDS1' and nvl(device_t.MANUFACTURER,'DUMMY') != 'NDS2' ");
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "2 exists");
	}

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "3 exists");
	if (device_count == 1 )
	{
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template_1 , ebufp);
	}
	else if (device_count == 2)
	{
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template_2 , ebufp);
	}
	else if (device_count == 3)
	{
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template_3 , ebufp);
	}

	result_flist = 	PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_SERIAL_NO, NULL, ebufp );
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_WARRANTY_END, NULL, ebufp );
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_WARRANTY_END_OFFSET, NULL, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_DESCR, NULL, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_DEVICE_TYPE, NULL, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_MANUFACTURER, NULL, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_MODEL, NULL, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_SOURCE, NULL, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STATE_ID, NULL, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_DEVICE_ID, NULL, ebufp );
        
            /*Inventory Changes Nim*/
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_CR_ADJ_DATE, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_COUNTRY, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_LICENSE_NO, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_START_T, NULL, ebufp);
        PIN_FLIST_FLD_SET(result_flist, MSO_FLD_CHANNEL_NAME, NULL, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_device_details SEARCH input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_device_details SESRCH output flist", srch_out_flistp);


	CLEANUP:
	*r_flistpp = srch_out_flistp;
	return;
}

