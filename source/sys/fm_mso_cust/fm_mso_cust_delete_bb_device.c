/*******************************************************************
 * File:	fm_mso_cust_delete_bb_device.c
 * Opcode:	MSO_OP_CUST_DELETE_BB_DEVICE 
 * Owner:	Someshwar
 * Created:	8-DEC-2014
 * Description: Deletes a CPE
 *
 * Modification History:
 * Modified By: 
 * Date:
 * Modification details:  
 *
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_delete_bb_device.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "ops/act.h"
#include "ops/cust.h"
#include "ops/bill.h"
#include "ops/device.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "pin_cust.h"
#include "pin_pymt.h"
#include "pin_inv.h"
#include "mso_ops_flds.h"
#include "mso_ntf.h"
#include "mso_cust.h"
#include "mso_errs.h"

/*******************************************************************
 * Functions Defined outside this source file
 *******************************************************************/
extern int32
fm_mso_trans_open(
	pcm_context_t		*ctxp,
	poid_t			*pdp,
	int32			flag,
	pin_errbuf_t		*ebufp);

extern void 
fm_mso_trans_commit(
	pcm_context_t		*ctxp,
	poid_t			*pdp,
	pin_errbuf_t		*ebufp);

extern void 
fm_mso_trans_abort(
	pcm_context_t		*ctxp,
	poid_t			*pdp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * Fuctions defined in this code 
 *******************************************************************/

EXPORT_OP void
op_mso_cust_delete_bb_device(
	cm_nap_connection_t		*connp,
	int32				opcode,
	int32				flags,
	pin_flist_t			*in_flistp,
	pin_flist_t			**ret_flistpp,
	pin_errbuf_t			*ebufp);

static void
fm_mso_cust_delete_bb_device(
	pcm_context_t			*ctxp,
	pin_flist_t			*in_flistp,
	int32				*error_code,
	pin_flist_t			**r_flistp,
	pin_errbuf_t			*ebufp);

static void
set_error_descr(
	pin_flist_t			**ret_flistpp,
	int				error_code,
	pin_errbuf_t			*ebufp);

static void
get_device_poid(
	pcm_context_t			*ctxp,
	pin_flist_t			*i_flistpp,
	poid_t				**dev_pdp,
	int32				*error_code,
	pin_errbuf_t			*ebufp);


/*******************************************************************
 * Main routine for the MSO_OP_CUST_DELETE_BB_DEVICE command
 *******************************************************************/
void
op_mso_cust_delete_bb_device(
	cm_nap_connection_t		*connp,
	int32				opcode,
	int32				flags,
	pin_flist_t			*in_flistp,
	pin_flist_t			**ret_flistpp,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*r_flistp = NULL;
	pcm_context_t			*ctxp = connp->dm_ctx;
	int32				oap_success = 0;
	int32				oap_failure = 1;
	int32				error_code = 0;
	poid_t				*pdp = NULL;
	int32				t_status;
	char				code[10];


    /***********************************************************
     * Null out results until we have some.
     ***********************************************************/
    *ret_flistpp = NULL;
    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Insanity check.
     ***********************************************************/
    if (opcode != MSO_OP_CUST_DELETE_BB_DEVICE) {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_OPCODE, 0, 0, opcode);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "bad opcode in op_mso_cust_delete_bb_device", ebufp);
        return;
    }

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "op_mso_cust_delete_bb_device input flist", in_flistp);

    pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
    t_status = fm_mso_trans_open(ctxp, pdp, READWRITE, ebufp);

    /***********************************************************
     * Call main function to do it
     ***********************************************************/
    fm_mso_cust_delete_bb_device(ctxp, in_flistp, &error_code, &r_flistp, ebufp);

    /***********************************************************
     * Results.
     ***********************************************************/
    if (PIN_ERRBUF_IS_ERR(ebufp)) 
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_mso_cust_delete_bb_device error", ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,
		"op_mso_cust_delete_bb_device: Failed Input Flist", in_flistp);
	PIN_ERRBUF_RESET(ebufp);
        if (t_status == 0)
        {
            fm_mso_trans_abort(ctxp, pdp, ebufp);
        }

	*ret_flistpp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
	if (error_code)
	{
	    set_error_descr(ret_flistpp, error_code, ebufp);
	}
	else
	{
		*ret_flistpp = (pin_flist_t *)NULL;
	}

	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
    } 
    else 
    {
        if (t_status == 0)
        {
            fm_mso_trans_commit(ctxp, pdp, ebufp);
        }

        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &oap_success, ebufp);
        *ret_flistpp = r_flistp;
        PIN_ERRBUF_RESET(ebufp);
    }

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
         "op_mso_cust_delete_bb_device return flist", *ret_flistpp);

    return;
}


static void
fm_mso_cust_delete_bb_device(
	pcm_context_t			*ctxp,
	pin_flist_t			*in_flistp,
	int32				*error_code,
	pin_flist_t			**out_flistp,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*dev_del_iflistp = NULL;
	pin_flist_t			*dev_del_oflistp = NULL;
	poid_t				*dev_pdp = NULL;
	char				*p_name = NULL;


	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_code = CUST_INTERNAL_ERROR;
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_cust_delete_bb_device: Input flist", in_flistp);

	p_name = (char *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PROGRAM_NAME, 0, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_code = CUST_PROGRAM_NAME;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_cust_delete_bb_device: Error", ebufp);
		return;
	}
	//Call the function to get the device poid
	get_device_poid(ctxp, in_flistp, &dev_pdp, error_code, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_cust_delete_bb_device: Error", ebufp);
		goto cleanup;
	}
	//Call the delete device opcode
	dev_del_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(dev_del_iflistp, PIN_FLD_POID,
		(void *)dev_pdp, ebufp);
	PIN_FLIST_FLD_SET(dev_del_iflistp, PIN_FLD_PROGRAM_NAME,
		(void *)p_name, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_cust_delete_bb_device: Delete device Input flist", dev_del_iflistp);
	PCM_OP(ctxp, PCM_OP_DEVICE_DELETE, 0, dev_del_iflistp, &dev_del_oflistp, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_code = CUST_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_cust_delete_bb_device: Error", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_cust_delete_bb_device: Delete device Output flist", dev_del_oflistp);
	*out_flistp = PIN_FLIST_COPY(dev_del_oflistp, ebufp);
cleanup:
	PIN_FLIST_DESTROY_EX(&dev_del_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&dev_del_oflistp, NULL);
	return;
}

static void
set_error_descr(
	pin_flist_t			**ret_flistpp,
	int				error_code,
	pin_errbuf_t			*ebufp)
{
	char				code[100] = {'\0'};

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"set_error_descr input flist", *ret_flistpp);
	sprintf(code,"%d",error_code);
	PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, code, ebufp);

	if (error_code == CUST_INTERNAL_ERROR)
	{
		strcpy(code,CUST_INTERNAL_ERROR_DESCR);
	}
	else if (error_code == CUST_PROGRAM_NAME)
	{
		strcpy(code,CUST_PROGRAM_NAME_DESCR);
	}
	else if (error_code == CUST_DEVICE_ID)
	{
		strcpy(code,CUST_DEVICE_ID_DESCR);
	}
	else if (error_code == CUST_POID_ID)
	{
		strcpy(code,CUST_POID_ID_DESCR);
	}
	else if (error_code == CUST_NO_BB_DEVICE)
	{
		strcpy(code,CUST_NO_BB_DEVICE_DESCR);
	}
	else if (error_code == CUST_BB_DEVICE_STATE)
	{
		strcpy(code,CUST_BB_DEVICE_STATE_DESCR);
	}
	PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, (void *)code, ebufp);
	return;
}


static void
get_device_poid(
	pcm_context_t			*ctxp,
	pin_flist_t			*i_flistpp,
	poid_t				**dev_pdp,
	int32				*error_code,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*s_iflistp = NULL;
	pin_flist_t			*s_oflistp = NULL;
	pin_flist_t			*args_flistp = NULL;
	poid_t				*s_pdp = NULL;
	poid_t				*pdp = NULL;
	char				*dev_id = NULL;
	char				*templ = "select X from /device where F1 = V1 ";
	int64				db = 0;
	int32				flag = 256;
	int				*state_id = 0;
	int32				count = 0;

	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_code = CUST_INTERNAL_ERROR;
		return;
	}
	dev_id = (char *)PIN_FLIST_FLD_GET(i_flistpp, PIN_FLD_DEVICE_ID, 0, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_code = CUST_DEVICE_ID;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"get_device_poid: Error", ebufp);
		return;
	}
	s_iflistp = PIN_FLIST_CREATE(ebufp);
	pdp = PCM_GET_USERID(ctxp);
	db = PIN_POID_GET_DB(pdp);
	s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_code = CUST_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"get_device_poid: Error", ebufp);
		return;
	}
	PIN_FLIST_FLD_PUT(s_iflistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_iflistp, PIN_FLD_FLAGS, (void *)&flag, ebufp);
	PIN_FLIST_FLD_SET(s_iflistp, PIN_FLD_TEMPLATE, (void *)templ, ebufp);
	//Add argument
	args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DEVICE_ID, (void *)dev_id, ebufp);
	//Add Results
	args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_RESULTS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATE_ID, (void *)NULL, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_code = CUST_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "get_device_poid: Error", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "get_device_poid: Search Input flist", s_iflistp);
	//Call the PCM_OP_SEARCH
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_iflistp, &s_oflistp, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_code = CUST_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "get_device_poid: Error", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "get_device_poid: Search Output flist", s_oflistp);
	count = PIN_FLIST_ELEM_COUNT(s_oflistp, PIN_FLD_RESULTS, ebufp);
	if(count == 0){
		*error_code = CUST_NO_BB_DEVICE;
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_DEVICE_ID, 0, 0);
		goto cleanup;
	}
	args_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_RESULTS, 0, 0, ebufp);
	if(args_flistp)
	{
		state_id = (int *)PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_STATE_ID, 0, ebufp);
		if(*state_id != MSO_BB_DEVICE_ALLOCATED)
		{
			*dev_pdp = (poid_t *)PIN_FLIST_FLD_TAKE(args_flistp, PIN_FLD_POID, 0, ebufp);
		}
		else
		{
			*error_code = CUST_BB_DEVICE_STATE;
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_BAD_VALUE, PIN_FLD_STATE_ID, 0, 0);
			goto cleanup;
		}
	}
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		*error_code = CUST_INTERNAL_ERROR;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "get_device_poid: Error", ebufp);
		goto cleanup;
	}
cleanup:
	PIN_FLIST_DESTROY_EX(&s_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&s_oflistp, NULL);
	return;
}
