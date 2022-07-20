/*******************************************************************
 * File:	fm_mso_cust_catv_swap_srvc_tagging.c
 * Opcode:	MSO_OP_CUST_CATV_SWAP_SRVC_TAGGING 
 * Owner:	SREEKANTH Y	
 * Created:	19-OCT-2016	
 * Description: 
 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 * This opcode is for implementing the swap functionality for catv for parent and child  
 * tagging with in an account. 
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_catv_swap_srvc_tagging.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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
#include "mso_cust.h"
#include "mso_prov.h"
#include "mso_lifecycle_id.h"
#include "mso_device.h"
#include "mso_utils.h"
#include "mso_errs.h"

/*******************************************************************
 * MSO_OP_CUST_CATV_SWAP_SRVC_TAGGING 
 *******************************************************************/

/**************************************
* External Functions start
***************************************/
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

extern
void fm_mso_create_lifecycle_evt(
	pcm_context_t           *ctxp,
	pin_flist_t             *in_flistp,
	pin_flist_t             *out_flistp,
	int32                   action,
	pin_errbuf_t            *ebufp);


EXPORT_OP void
op_mso_cust_catv_swap_srvc_tagging(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_catv_swap_srvc_tagging(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_update_srvc_conn_type(
	pcm_context_t		*ctxp,
	poid_t			*act_pdp,
	poid_t			*srvc_pdp,
	int32			conn_type,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static int32 
fm_mso_check_base_pack_prov_tag(
	pcm_context_t		*ctxp,
	poid_t			*m_deal_pdp,
	poid_t			*c_deal_pdp,
	pin_errbuf_t		*ebufp);


/*******************************************************************
 * MSO_OP_CUST_CATV_SWAP_SRVC_TAGGING  
 *******************************************************************/
void 
op_mso_cust_catv_swap_srvc_tagging(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	pin_flist_t		*services_flistp = NULL;
	poid_t			*inp_pdp = NULL;
	int32 			status_uncommitted = 1;
	int32			*status = NULL;
    int32           subscriber_type = 48;
    char            *stb_idp = NULL;
	int                     local = LOCAL_TRANS_OPEN_SUCCESS;
	

	*r_flistpp		= NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_CATV_SWAP_SRVC_TAGGING ) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_catv_swap_srvc_tagging error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_catv_swap_srvc_tagging input flist", i_flistp);

    stb_idp = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_STB_ID, 1, ebufp);
    if (stb_idp)
    {
        fm_mso_cust_get_acc_from_sno(ctxp, i_flistp, &services_flistp, ebufp);
        if (services_flistp)
        {
            PIN_FLIST_FLD_COPY(services_flistp, PIN_FLD_ACCOUNT_OBJ, i_flistp, PIN_FLD_POID, ebufp);
            PIN_FLIST_FLD_COPY(services_flistp, PIN_FLD_SERVICE_OBJ, i_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
            PIN_FLIST_FLD_COPY(services_flistp, PIN_FLD_ACCOUNT_NO, i_flistp, PIN_FLD_ACCOUNT_NO, ebufp);
            PIN_FLIST_FLD_COPY(services_flistp, PIN_FLD_DEVICE_ID, i_flistp, PIN_FLD_NEW_VALUE, ebufp);
            PIN_FLIST_FLD_SET(i_flistp, MSO_FLD_SUBSCRIBER_TYPE, &subscriber_type, ebufp );
            PIN_FLIST_DESTROY_EX(&services_flistp, NULL);
        }
        else
        {
            PIN_FLIST_DESTROY_EX(&services_flistp, NULL);
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Main Device serial number not found");
            *r_flistpp = PIN_FLIST_CREATE(ebufp);
            PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, inp_pdp, ebufp );
            PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_uncommitted, ebufp);
            PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51430", ebufp);
            PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Device serial number not found", ebufp);
            return;
        }
    }

	inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	local = fm_mso_trans_open(ctxp, inp_pdp, 3,ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Unable to open Local Transaction: Error", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, inp_pdp, ebufp );
		PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &status_uncommitted, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "51413", ebufp);
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
		PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "51414", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, "Unable to open Local Transaction: Already Open", ebufp);
		return;
	}
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	fm_mso_cust_catv_swap_srvc_tagging(ctxp, flags, i_flistp, r_flistpp, ebufp);
 	/***********************************************************
	 * Results.
	 ***********************************************************/

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_catv_swap_srvc_tagging error", ebufp);
		if( local == LOCAL_TRANS_OPEN_SUCCESS)
		{
			fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
		}
		PIN_ERRBUF_RESET(ebufp);
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, inp_pdp, ebufp );
		PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &status_uncommitted, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "50095", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, "SWAP STB Failed due to BRM error", ebufp);
		return;
	}
	else
	{
		if ( r_flistpp != NULL){
			status = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_STATUS, 0, ebufp);
			if( local == LOCAL_TRANS_OPEN_SUCCESS  && (status != NULL && *status == 0))
			{
				fm_mso_trans_commit(ctxp, inp_pdp, ebufp);
			}
			else{
				fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
			} 
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_catv_swap_srvc_tagging output flist", *r_flistpp);
	}
	return;
}

/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_catv_swap_srvc_tagging(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{

	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*device_iflistp = NULL;
	pin_flist_t		*device_oflistp = NULL;
	pin_flist_t		*dvc_srvc_flistp = NULL;
	pin_flist_t		*child_r_flistp = NULL;
	pin_flist_t		*main_r_flistp = NULL;
	pin_flist_t		*purch_prod_flistp = NULL;
	pin_flist_t		*prod_upd_flistp = NULL;
	pin_flist_t		*prod_upd_oflistp = NULL;
	pin_flist_t		*prod_iflistp = NULL;
	pin_flist_t		*prod_oflistp = NULL;
	pin_flist_t		*products_flistp = NULL;
	pin_flist_t		*tmp_flistp= NULL;
	pin_flist_t		*result_flistp= NULL;
	pin_flist_t		*read_iflistp= NULL;
	pin_flist_t		*read_oflistp= NULL;
	pin_flist_t		*out_flistp= NULL;
	pin_flist_t		*mout_flistp= NULL;
    pin_flist_t     *disc_flistp = NULL;
    pin_flist_t     *disc_oflistp = NULL;

	poid_t			*act_pdp = NULL;
	poid_t			*srvc_pdp = NULL;
	poid_t			*d_act_pdp = NULL;
	poid_t			*d_srvc_pdp = NULL;
	poid_t			*prod_pdp = NULL;
	poid_t			*prd_pdp = NULL;
	poid_t			*m_deal_pdp = NULL;
	poid_t			*c_deal_pdp = NULL;
    pin_decimal_t       *per_discp = NULL;
    time_t          disc_end_t = 1893436200;
	int32                   device_search_type = MSO_FLAG_SRCH_BY_ID;
	int32                   swap_failure = 1;
	int32                   conn_type = -1;
	int32                   new_conn_type = -1;
	int32			update_pkg_flag = 0;
	int32			srvc_update_flag = 0;
	int32			is_parent_act_plans = 0;//Parent Active Plans true
	int32			is_child_act_plans = 0;// Child Active Plans true
	int32			status = 1; //Error
	int32			elem_id = 0;
	int32			pelem_id = 0;
	int32			relem_id = 0;
	int32			*srvc_status = NULL;
    int32           *sub_typep = NULL;
    int32           pkg_type = 3;
    int32           disc_type = 0;
    int32           serv_type = 0;
    int32           *disc_status = NULL;	
	int			index = 0;

	char			*device_id = NULL;
	char			*c_prov_tag = NULL;
	char			*m_prov_tag = NULL;
	char			error_code[10];
	char			error_msg[250];

	pin_cookie_t		cookie = NULL;
	pin_cookie_t		pcookie = NULL;
	pin_cookie_t		rcookie = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_catv_swap_srvc_tagging input flist", i_flistp);
	

	act_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	srvc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
	device_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEVICE_ID, 0, ebufp);
    sub_typep = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SUBSCRIBER_TYPE, 1, ebufp);

	if (!act_pdp || !srvc_pdp || !device_id){
		strcpy(error_code, "50000");
		if (!act_pdp)
			strcpy(error_msg, "Accoutn OBJ is mising in input flist");
		else if (!srvc_pdp)
			strcpy(error_msg, "Parent SERVICE is missing in input flist");
		else if (!device_id)
			strcpy(error_msg, "Child STB is missing in input flist");

		PIN_ERRBUF_RESET(ebufp);
		fm_mso_utils_prepare_error_flist(ctxp,  i_flistp,&ret_flistp, error_code, error_msg, ebufp);
		*r_flistpp = ret_flistp;
		return;
	}

	// Check if PARENT SERVICE belongs to same account 
	read_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_iflistp, PIN_FLD_POID, srvc_pdp, ebufp );
	PIN_FLIST_FLD_SET(read_iflistp, PIN_FLD_ACCOUNT_OBJ, act_pdp, ebufp );
	PIN_FLIST_FLD_SET(read_iflistp, PIN_FLD_STATUS, NULL, ebufp );
	tmp_flistp = PIN_FLIST_SUBSTR_ADD(read_iflistp, MSO_FLD_CATV_INFO, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_CONN_TYPE, &conn_type, ebufp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_iflistp, &read_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&read_iflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"Error in getting the main service details", ebufp);
		return;
	}
	if (read_oflistp != NULL){
		srvc_status = PIN_FLIST_FLD_GET(read_oflistp, PIN_FLD_STATUS, 0, ebufp);
		if(act_pdp && PIN_POID_COMPARE(act_pdp, PIN_FLIST_FLD_GET(read_oflistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp), 0, ebufp) != 0)
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Main connection Subscription doesn't belong to the account passed", ebufp);
			strcpy(error_code, "51088");
			strcpy(error_msg, "Parent STB subscription doesn't belong to the same account");
			PIN_ERRBUF_RESET(ebufp);
			fm_mso_utils_prepare_error_flist(ctxp,  i_flistp,&ret_flistp, error_code, error_msg, ebufp);
			*r_flistpp = ret_flistp;
			PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
			return;
		}
		else if(srvc_status && *srvc_status != 10100){
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"parent subscription is not active", ebufp);
			strcpy(error_code, "51088");
			strcpy(error_msg, "Parent STB subscription is not active");
			PIN_ERRBUF_RESET(ebufp);
			fm_mso_utils_prepare_error_flist(ctxp,  i_flistp,&ret_flistp, error_code, error_msg, ebufp);
			*r_flistpp = ret_flistp;
			PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
			return;
		}
		tmp_flistp = PIN_FLIST_SUBSTR_GET(read_oflistp, MSO_FLD_CATV_INFO, 1, ebufp);
		if (tmp_flistp)
		{
			conn_type = *(int32 *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CONN_TYPE, 0, ebufp);
		}
	}
	PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
	

	// Check if given SERVICE is a main connection or not
	if (conn_type == 0 ){
		// If given srvc is main connection, then proceed to swap with child check
		device_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(device_iflistp, PIN_FLD_POID, act_pdp, ebufp );
		PIN_FLIST_FLD_SET(device_iflistp, PIN_FLD_DEVICE_ID, device_id, ebufp );
		PIN_FLIST_FLD_SET(device_iflistp, PIN_FLD_SEARCH_TYPE, &device_search_type, ebufp );

		fm_mso_get_device_info(ctxp, device_iflistp, &device_oflistp, ebufp );
		PIN_FLIST_DESTROY_EX(&device_iflistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
				"Error in getting the child STB details", ebufp);
			PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
			return;
		}
		if (!device_oflistp)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Child STB details not present in BRM");
			strcpy(error_code, "51090");
			strcpy(error_msg, "Child STB details not present in BRM");
			PIN_ERRBUF_RESET(ebufp);
			fm_mso_utils_prepare_error_flist(ctxp,  i_flistp,&ret_flistp, error_code, error_msg, ebufp);
			*r_flistpp = ret_flistp;
			PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
			return;
		}
		else{
			dvc_srvc_flistp = PIN_FLIST_ELEM_GET(device_oflistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY, 1, ebufp);
			if(!dvc_srvc_flistp){
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Child STB is not associated with any account");
				strcpy(error_code, "51088");
				strcpy(error_msg, "Child STB is not associated with any account");
				PIN_ERRBUF_RESET(ebufp);
				fm_mso_utils_prepare_error_flist(ctxp,  i_flistp,&ret_flistp, error_code, error_msg, ebufp);
				*r_flistpp = ret_flistp;
				PIN_FLIST_DESTROY_EX(&device_oflistp, NULL);
				PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
				return;
			}
			else{ 
				d_act_pdp = PIN_FLIST_FLD_GET(dvc_srvc_flistp,  PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
				d_srvc_pdp = PIN_FLIST_FLD_GET(dvc_srvc_flistp,  PIN_FLD_SERVICE_OBJ, 1, ebufp);
				// Check if child STB doesn't belong to same account of parent connection
				if ((act_pdp && d_act_pdp ) && PIN_POID_COMPARE(act_pdp, d_act_pdp, 0, ebufp) != 0 ){
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Child STB doesn't belong to same account, no swap allowed");
					strcpy(error_code, "51088");
					strcpy(error_msg, "Child STB doesn't belong to same account, no swap allowed");
					PIN_ERRBUF_RESET(ebufp);
					fm_mso_utils_prepare_error_flist(ctxp,  i_flistp,&ret_flistp, error_code, error_msg, ebufp);
					*r_flistpp = ret_flistp;
					PIN_FLIST_DESTROY_EX(&device_oflistp, NULL);
					PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
					return;
				}
				if(d_srvc_pdp && fm_mso_catv_conn_type(ctxp, d_srvc_pdp, ebufp) == 1)
				{
					//Proceed to update child STB connection_type to parent (Main connection)
					new_conn_type = 0;
					fm_mso_update_srvc_conn_type(ctxp, d_act_pdp, d_srvc_pdp, new_conn_type, &child_r_flistp, ebufp);
					if (PIN_ERRBUF_IS_ERR(ebufp))
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, 
							"Error in updating the child STB service");
						strcpy(error_code, "51093");
						strcpy(error_msg, "STB SWAP tagging is failed");
						PIN_ERRBUF_RESET(ebufp);
						fm_mso_utils_prepare_error_flist(ctxp,  i_flistp,&ret_flistp, error_code, error_msg, ebufp);
						*r_flistpp = ret_flistp;
						PIN_FLIST_DESTROY_EX(&device_oflistp, NULL);
						PIN_FLIST_DESTROY_EX(&child_r_flistp, NULL);
						PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
						return;
					}
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_catv_swap_srvc_tagging child update service ret flist", child_r_flistp);
                    if (sub_typep && *sub_typep == 48)
                    {
                        disc_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_PUT(disc_flistp, PIN_FLD_POID,
                            PIN_POID_CREATE(PIN_POID_GET_DB(d_act_pdp), "/service", (int64)-1, ebufp), ebufp);
                        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_NEW_VALUE, disc_flistp, PIN_FLD_DEVICE_ID, ebufp);
                        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_NO, disc_flistp, PIN_FLD_ACCOUNT_NO, ebufp);
                        PIN_FLIST_FLD_SET(disc_flistp, MSO_FLD_PKG_TYPE, &pkg_type, ebufp);
                        PIN_FLIST_FLD_SET(disc_flistp, PIN_FLD_DISCOUNT_FLAGS, &disc_type, ebufp);
                        PIN_FLIST_FLD_SET(disc_flistp, MSO_FLD_SERVICE_TYPE, &serv_type, ebufp);
                        PIN_FLIST_FLD_SET(disc_flistp, PIN_FLD_END_T, &disc_end_t, ebufp);
                        if (per_discp)
                        {
                            PIN_FLIST_FLD_SET(disc_flistp, PIN_FLD_DISCOUNT, per_discp, ebufp);
                        }
                        else
                        {
                            PIN_FLIST_FLD_PUT(disc_flistp, PIN_FLD_DISCOUNT,
                                     pbo_decimal_from_str("0.6", ebufp), ebufp);
                        }
                        PIN_FLIST_FLD_SET(disc_flistp, PIN_FLD_SERVICE_OBJ, d_srvc_pdp, ebufp);
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Discount config create input", disc_flistp);

                        PCM_OP(ctxp, MSO_OP_CUST_CATV_DISC_CONFIG, flags, disc_flistp, &disc_oflistp, ebufp);

                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                "Discount config create output", disc_oflistp);
                        disc_status = (int32 *)PIN_FLIST_FLD_GET(disc_oflistp, PIN_FLD_STATUS, 1, ebufp);
                        if ((disc_status && *disc_status == swap_failure) || PIN_ERRBUF_IS_ERR(ebufp))
                        {
                            PIN_FLIST_DESTROY_EX(&disc_oflistp, ebufp);
                            status = 3;
pin_set_err(ebufp, PIN_ERRLOC_CM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_DUPLICATE,
                                    PIN_FLD_DISCOUNT, 0, 0);
                            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Discount already configured", ebufp);
                            return;
                        }
                        PIN_FLIST_DESTROY_EX(&disc_oflistp, ebufp);
                    }
					if (child_r_flistp !=  NULL) //Proceed to update origin parent STB to Child STB conneciton
					{
						// Before updating parent connection, get the purchase product details of CHILD STB
						prod_iflistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_SET(prod_iflistp, PIN_FLD_POID, act_pdp, ebufp );
						PIN_FLIST_FLD_SET(prod_iflistp, PIN_FLD_SERVICE_OBJ, d_srvc_pdp, ebufp );
						fm_mso_get_purchased_prod_active(ctxp, prod_iflistp, &prod_oflistp, ebufp);
						PIN_FLIST_DESTROY_EX(&prod_iflistp, NULL);
						if (PIN_ERRBUF_IS_ERR(ebufp))
						{
							PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
								"Error in getting the product details", ebufp);
							PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
							return;
						}
						// Add product details to PRODUCTS array and this will enable us to update products
						purch_prod_flistp = PIN_FLIST_CREATE(ebufp);
						index = 0;
						if (prod_oflistp != NULL && (PIN_FLIST_ELEM_COUNT(prod_oflistp, PIN_FLD_RESULTS, ebufp) > 0 )){
							while ((result_flistp = PIN_FLIST_ELEM_GET_NEXT(prod_oflistp, PIN_FLD_RESULTS, &pelem_id, 1,                                                                                                                      &pcookie, ebufp)) != NULL ){
								prod_pdp = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
								if (fm_mso_catv_pt_pkg_type(ctxp, prod_pdp, ebufp) == 0){
									c_deal_pdp = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_DEAL_OBJ, 1, ebufp);
								}
								tmp_flistp = PIN_FLIST_ELEM_ADD(purch_prod_flistp, PIN_FLD_PRODUCTS, index, ebufp);
								PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
								PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_POID, tmp_flistp, PIN_FLD_POID, ebufp);
								index = index+1;
							}
						}
						else{
							is_child_act_plans = 1; // No active plans 
						}
						//Get the parent STB purchased product details 
						prod_iflistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_SET(prod_iflistp, PIN_FLD_POID, act_pdp, ebufp );
						PIN_FLIST_FLD_SET(prod_iflistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp );
						fm_mso_get_purchased_prod_active(ctxp, prod_iflistp, &out_flistp, ebufp);
						PIN_FLIST_DESTROY_EX(&prod_iflistp, NULL);
						if (PIN_ERRBUF_IS_ERR(ebufp))
						{
							PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
								"Error in getting the product details", ebufp);
							PIN_FLIST_DESTROY_EX(&device_oflistp, NULL);
							PIN_FLIST_DESTROY_EX(&child_r_flistp, NULL);
							PIN_FLIST_DESTROY_EX(&main_r_flistp, NULL);
							PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
							PIN_FLIST_DESTROY_EX(&prod_oflistp, NULL);
							PIN_FLIST_DESTROY_EX(&purch_prod_flistp, NULL);
							return;
						}
						if ( !out_flistp || (out_flistp != NULL && (PIN_FLIST_ELEM_COUNT(out_flistp, PIN_FLD_RESULTS, ebufp) == 0 ))){
							is_parent_act_plans = 1;//No active plans for Parent STB
						}
						if(is_parent_act_plans != is_child_act_plans ){
							strcpy(error_code, "51093");
							if(is_parent_act_plans)
								strcpy(error_msg, "There are no active Plans for Parent STB");
							else if(is_child_act_plans)
								strcpy(error_msg, "There are no active Plans for child STB");
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, error_msg);
							PIN_ERRBUF_RESET(ebufp);
							fm_mso_utils_prepare_error_flist(ctxp,  i_flistp,&ret_flistp, error_code, error_msg, ebufp);
							*r_flistpp = ret_flistp;
							PIN_FLIST_DESTROY_EX(&device_oflistp, NULL);
							PIN_FLIST_DESTROY_EX(&child_r_flistp, NULL);
							PIN_FLIST_DESTROY_EX(&purch_prod_flistp, NULL);
							PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
							PIN_FLIST_DESTROY_EX(&prod_oflistp, NULL);
							PIN_FLIST_DESTROY_EX(&out_flistp, NULL);
							return;
							
						}

						// Update parent STB connection type
						new_conn_type = 1;
						fm_mso_update_srvc_conn_type(ctxp, d_act_pdp, srvc_pdp, new_conn_type, &main_r_flistp, ebufp);
						if (PIN_ERRBUF_IS_ERR(ebufp))
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, 
								"Error in updating the parent STB service");
							strcpy(error_code, "51093");
							strcpy(error_msg, "STB SWAP tagging is failed");
							PIN_ERRBUF_RESET(ebufp);
							fm_mso_utils_prepare_error_flist(ctxp,  i_flistp,&ret_flistp, error_code, error_msg, ebufp);
							*r_flistpp = ret_flistp;
							PIN_FLIST_DESTROY_EX(&device_oflistp, NULL);
							PIN_FLIST_DESTROY_EX(&child_r_flistp, NULL);
							PIN_FLIST_DESTROY_EX(&main_r_flistp, NULL);
							PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
							PIN_FLIST_DESTROY_EX(&prod_oflistp, NULL);
							PIN_FLIST_DESTROY_EX(&out_flistp, NULL);
							PIN_FLIST_DESTROY_EX(&purch_prod_flistp, NULL);
							return;
						}
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_catv_swap_srvc_tagging child update service ret flist", 
										main_r_flistp);
                    if (sub_typep && *sub_typep == 48)
                    {
                        disc_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_PUT(disc_flistp, PIN_FLD_POID,
                            PIN_POID_CREATE(PIN_POID_GET_DB(d_act_pdp), "/service", (int64)-1, ebufp), ebufp);
                        PIN_FLIST_FLD_COPY(device_oflistp, PIN_FLD_DEVICE_ID, disc_flistp, PIN_FLD_DEVICE_ID, ebufp);
                        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_NO, disc_flistp, PIN_FLD_ACCOUNT_NO, ebufp);
                        PIN_FLIST_FLD_SET(disc_flistp, MSO_FLD_PKG_TYPE, &pkg_type, ebufp);
                        PIN_FLIST_FLD_SET(disc_flistp, PIN_FLD_DISCOUNT_FLAGS, &disc_type, ebufp);
                        PIN_FLIST_FLD_SET(disc_flistp, MSO_FLD_SERVICE_TYPE, &serv_type, ebufp);
                        disc_end_t = pin_virtual_time(NULL);
                        PIN_FLIST_FLD_SET(disc_flistp, PIN_FLD_END_T, &disc_end_t, ebufp);
                        if (per_discp)
                        {
                            PIN_FLIST_FLD_SET(disc_flistp, PIN_FLD_DISCOUNT, per_discp, ebufp);
                        }
                        else
                        {
                            PIN_FLIST_FLD_PUT(disc_flistp, PIN_FLD_DISCOUNT,
                                     pbo_decimal_from_str("0.6", ebufp), ebufp);
                        }
                        PIN_FLIST_FLD_SET(disc_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Discount config create input", disc_flistp);

                        PCM_OP(ctxp, MSO_OP_CUST_CATV_DISC_CONFIG, flags, disc_flistp, &disc_oflistp, ebufp);

                        PIN_FLIST_DESTROY_EX(&disc_flistp, ebufp);
                        PIN_FLIST_DESTROY_EX(&read_oflistp, ebufp);
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                "Discount config create output", disc_oflistp);
                        disc_status = (int32 *)PIN_FLIST_FLD_GET(disc_oflistp, PIN_FLD_STATUS, 1, ebufp);
                        if ((disc_status && *disc_status == swap_failure) || PIN_ERRBUF_IS_ERR(ebufp))
                        {
                            PIN_FLIST_DESTROY_EX(&disc_oflistp, ebufp);
                            status = 3;
                            pin_set_err(ebufp, PIN_ERRLOC_CM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_DUPLICATE,
                                    PIN_FLD_DISCOUNT, 0, 0);
                            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Discount already configured", ebufp);
                            return;
                        }
                        PIN_FLIST_DESTROY_EX(&disc_oflistp, ebufp);
                     }
						if (main_r_flistp != NULL)
						{	
							// If both STB not having active plans, then don't allow package update but swap is allowed
							if(is_parent_act_plans && is_child_act_plans ){
								update_pkg_flag = 0;
								srvc_update_flag = 1; // Only service update
							}
							// Add product details to PRODUCTS array
							if (!is_parent_act_plans && !srvc_update_flag){
								while ((result_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp, PIN_FLD_RESULTS, &relem_id, 1,                                                                                                                      &rcookie, ebufp)) != NULL ){
									prd_pdp = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
									if (fm_mso_catv_pt_pkg_type(ctxp, prd_pdp, ebufp) == 0){
										m_deal_pdp = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_DEAL_OBJ, 1, ebufp);
									}
									tmp_flistp = PIN_FLIST_ELEM_ADD(purch_prod_flistp, PIN_FLD_PRODUCTS, index, ebufp);
									PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_SERVICE_OBJ, d_srvc_pdp, ebufp);
									PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_POID, tmp_flistp, PIN_FLD_POID, ebufp);
									index = index+1;
								}
							}
							/*NTO : changes done to remove parent child depency */
							/*if (!srvc_update_flag){//This is to check if only SERVICE UPDATE for swapping
								if ( m_deal_pdp && c_deal_pdp && !fm_mso_check_base_pack_prov_tag(ctxp, m_deal_pdp, c_deal_pdp, ebufp)){
									update_pkg_flag = 1;
								}
								else{
									PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, 
										"PROV TAG mismatch error");
									strcpy(error_code, "51099");
									strcpy(error_msg, "Base pack for Main and Child connection id not in synchronized." 
											   "SWAP not allowed");
									PIN_ERRBUF_RESET(ebufp);
									fm_mso_utils_prepare_error_flist(ctxp,  i_flistp,&ret_flistp, error_code, error_msg, ebufp);
									*r_flistpp = ret_flistp;
									PIN_FLIST_DESTROY_EX(&device_oflistp, NULL);
									PIN_FLIST_DESTROY_EX(&child_r_flistp, NULL);
									PIN_FLIST_DESTROY_EX(&main_r_flistp, NULL);
									PIN_FLIST_DESTROY_EX(&purch_prod_flistp, NULL);
									PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
									PIN_FLIST_DESTROY_EX(&prod_oflistp, NULL);
									PIN_FLIST_DESTROY_EX(&out_flistp, NULL);
									return;
								}
							}*/
							PIN_FLIST_DESTROY_EX(&prod_oflistp, NULL);
							PIN_FLIST_DESTROY_EX(&out_flistp, NULL);
						}
						
					}
                                                        /*NTO : changes done to remove parent child depency */
					//if (!srvc_update_flag && update_pkg_flag){ //Procced to update purchased_product to replace child and parent SERVICE_OBJ 
					if (!srvc_update_flag ){
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_catv_swap_srvc_tagging products flist", 															purch_prod_flistp);
						while((products_flistp = PIN_FLIST_ELEM_GET_NEXT(purch_prod_flistp, PIN_FLD_PRODUCTS, &elem_id, 1, 															&cookie, ebufp)) != NULL )
						{	
							prod_upd_flistp  = PIN_FLIST_CREATE(ebufp);
							PIN_FLIST_FLD_COPY(products_flistp, PIN_FLD_POID, prod_upd_flistp, PIN_FLD_POID, ebufp);			
							PIN_FLIST_FLD_COPY(products_flistp, PIN_FLD_SERVICE_OBJ, prod_upd_flistp, PIN_FLD_SERVICE_OBJ, ebufp);									
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_catv_swap_srvc_tagging update prod flist", 															prod_upd_flistp);
							PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, prod_upd_flistp, &prod_upd_oflistp, ebufp);
							PIN_FLIST_DESTROY_EX(&prod_upd_flistp, NULL);
							if (PIN_ERRBUF_IS_ERR(ebufp))
							{
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, 
									"Error in updating products");
								strcpy(error_code, "51093");
								strcpy(error_msg, "STB SWAP tagging is failed");
								PIN_ERRBUF_RESET(ebufp);
								fm_mso_utils_prepare_error_flist(ctxp,  i_flistp,&ret_flistp, error_code, error_msg, ebufp);
								*r_flistpp = ret_flistp;
								PIN_FLIST_DESTROY_EX(&device_oflistp, NULL);
								PIN_FLIST_DESTROY_EX(&child_r_flistp, NULL);
								PIN_FLIST_DESTROY_EX(&main_r_flistp, NULL);
								PIN_FLIST_DESTROY_EX(&purch_prod_flistp, NULL);
								PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
								return;
							}
							if (prod_upd_oflistp != NULL){
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Product update is successful");	
								status = 0;//Success
							}
						}
					}
					else if(srvc_update_flag){
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "SERVICE update only done. No package update");
						status = 0;
					}
					else{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, 
							"SWAP is failed");
						strcpy(error_code, "51093");
						strcpy(error_msg, "STB SWAP tagging is failed");
						PIN_ERRBUF_RESET(ebufp);
						fm_mso_utils_prepare_error_flist(ctxp,  i_flistp,&ret_flistp, error_code, error_msg, ebufp);
						*r_flistpp = ret_flistp;
						PIN_FLIST_DESTROY_EX(&device_oflistp, NULL);
						PIN_FLIST_DESTROY_EX(&child_r_flistp, NULL);
						PIN_FLIST_DESTROY_EX(&main_r_flistp, NULL);
						PIN_FLIST_DESTROY_EX(&purch_prod_flistp, NULL);
						PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
						return;
					}
					PIN_FLIST_DESTROY_EX(&purch_prod_flistp, NULL);
					ret_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_OBJECT, d_srvc_pdp, ebufp );
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status, ebufp );
					//Generate life cycle event for swap tagging
					if (status == 0){ 
						fm_mso_create_lifecycle_evt(ctxp, i_flistp, ret_flistp, ID_SWAP_TAGGING, ebufp);
					}
				}
				else{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Device ID provided is not a child STB");
					strcpy(error_code, "51092");
					strcpy(error_msg, "Device ID provided is not a child STB");
					PIN_ERRBUF_RESET(ebufp);
					fm_mso_utils_prepare_error_flist(ctxp,  i_flistp,&ret_flistp, error_code, error_msg, ebufp);
					*r_flistpp = ret_flistp;
					PIN_FLIST_DESTROY_EX(&device_oflistp, NULL);
					PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
					return;
				}
			}
		}
	}
	else{
		strcpy(error_code, "51091");
		strcpy(error_msg, "Subscription trying to swap is not a main connection");
		PIN_ERRBUF_RESET(ebufp);
		fm_mso_utils_prepare_error_flist(ctxp,  i_flistp,&ret_flistp, error_code, error_msg, ebufp);
		*r_flistpp = ret_flistp;
		PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
		return;
	}

	*r_flistpp = ret_flistp;
	PIN_FLIST_DESTROY_EX(&device_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&child_r_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&main_r_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&purch_prod_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&prod_upd_oflistp, NULL);
	return;
}

static void
fm_mso_update_srvc_conn_type(
        pcm_context_t           *ctxp,
        poid_t                  *act_pdp,
        poid_t                  *srvc_pdp,
        int32                   conn_type,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t		*srvc_iflistp = NULL;
	pin_flist_t		*srvc_oflistp = NULL;
	pin_flist_t		*srvc_arry_flistp = NULL;
	pin_flist_t		*inherit_flistp = NULL;
	pin_flist_t		*catv_flistp = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	*r_flistpp = PIN_FLIST_CREATE(ebufp);

	srvc_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(srvc_iflistp, PIN_FLD_POID, act_pdp, ebufp );
	PIN_FLIST_FLD_SET(srvc_iflistp, PIN_FLD_PROGRAM_NAME, "STB Swap tagging", ebufp);
	srvc_arry_flistp = PIN_FLIST_ELEM_ADD(srvc_iflistp, PIN_FLD_SERVICES, 0, ebufp);
	PIN_FLIST_FLD_SET(srvc_arry_flistp, PIN_FLD_POID, srvc_pdp, ebufp );
	inherit_flistp = PIN_FLIST_SUBSTR_ADD(srvc_arry_flistp, PIN_FLD_INHERITED_INFO,  ebufp);
	catv_flistp = PIN_FLIST_SUBSTR_ADD(inherit_flistp, MSO_FLD_CATV_INFO,  ebufp);
	PIN_FLIST_FLD_SET(catv_flistp, PIN_FLD_CONN_TYPE, &conn_type, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_catv_swap_srvc_tagging update service in flist", srvc_iflistp);
	PCM_OP(ctxp, PCM_OP_CUST_UPDATE_SERVICES, 0, srvc_iflistp, &srvc_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srvc_iflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"Error in updating the service", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_catv_swap_srvc_tagging update service out flist", srvc_oflistp);
	if (srvc_oflistp != NULL){
		srvc_arry_flistp = PIN_FLIST_ELEM_GET(srvc_oflistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY, 1, ebufp);
		PIN_FLIST_FLD_COPY(srvc_arry_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Service Connection type change is succesful");
	}
	PIN_FLIST_DESTROY_EX(&srvc_oflistp, NULL);
	return;

}

static int32 
fm_mso_check_base_pack_prov_tag(
        pcm_context_t           *ctxp,
        poid_t                  *m_deal_pdp,
        poid_t                  *c_deal_pdp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t             *prod_flistp = NULL;
	pin_flist_t             *m_prod_flistp = NULL;
	pin_flist_t             *c_deal_flistp = NULL;
	pin_flist_t             *m_deal_flistp = NULL;
	pin_flist_t             *out_flistp = NULL;
	pin_flist_t             *ret_flistp = NULL;

	poid_t			*mprod_pdp  = NULL;
	poid_t			*prod_pdp  = NULL;

	char			*m_prov_tag = NULL;
	char			*c_prov_tag = NULL;

	int32			is_prov_tag = 0;
	int32			main_cnt = -1;
	int32			child_cnt = -1;
	int			elem_id = 0;
	int			melem_id = 0;
	pin_cookie_t		cookie = NULL;
	pin_cookie_t		mcookie = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_RESET(ebufp);
	//Get the products from base pack deal for main and child connection
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "MAIN DEAL", m_deal_pdp);
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "CHILD DEAL", c_deal_pdp);

	fm_mso_utils_read_any_object(ctxp, c_deal_pdp, &c_deal_flistp, ebufp);
	fm_mso_utils_read_any_object(ctxp, m_deal_pdp, &m_deal_flistp, ebufp);
	
	child_cnt = PIN_FLIST_ELEM_COUNT(c_deal_flistp, PIN_FLD_PRODUCTS, ebufp);
	main_cnt = PIN_FLIST_ELEM_COUNT(m_deal_flistp, PIN_FLD_PRODUCTS, ebufp);
	
	if(main_cnt != child_cnt){
		is_prov_tag  = 1; // Set to FALSE
		PIN_FLIST_DESTROY_EX(&c_deal_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&m_deal_flistp, NULL);
		return is_prov_tag;
	}
	

	while((prod_flistp = PIN_FLIST_ELEM_GET_NEXT(c_deal_flistp, PIN_FLD_PRODUCTS, &elem_id, 1, &cookie, ebufp)) != NULL)
	{
		prod_pdp = PIN_FLIST_FLD_GET(prod_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
		fm_mso_utils_read_any_object(ctxp, prod_pdp, &out_flistp, ebufp);
		c_prov_tag = PIN_FLIST_FLD_GET(out_flistp, PIN_FLD_PROVISIONING_TAG, 1, ebufp);

		melem_id = 0;
		mcookie = NULL;		
		while((m_prod_flistp = PIN_FLIST_ELEM_GET_NEXT(m_deal_flistp, PIN_FLD_PRODUCTS, &melem_id, 1, &mcookie, ebufp)) != NULL){
			mprod_pdp = PIN_FLIST_FLD_GET(m_prod_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
			fm_mso_utils_read_any_object(ctxp, mprod_pdp, &ret_flistp, ebufp);
			m_prov_tag = PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_PROVISIONING_TAG, 1, ebufp);
			if (c_prov_tag && m_prov_tag && (strcmp(m_prov_tag, c_prov_tag) == 0))
			{
				is_prov_tag = 0;//Set to TRUE 
				break;
			}
			else{
				is_prov_tag = 1;//Set to FALSE 
			}
		}
	}
	PIN_FLIST_DESTROY_EX(&c_deal_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&m_deal_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
	return is_prov_tag;
}
