/*******************************************************************
 * File:	fm_mso_cust_get_pkg_channels.c
 * Opcode:	MSO_OP_CUST_GET_PKG_CHANNELS 
 * Created:	18-DEC-2018	
 * Description: 
 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 * This opcode is to retrieve channels mapped for a given plan object passed.
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_get_pkg_channels.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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
#include "cm_cache.h"

/*******************************************************************
 * MSO_OP_CUST_GET_PKG_CHANNELS 
 *******************************************************************/
extern cm_cache_t *mso_cfg_catv_channel_master_config_ptr;
extern int32 refresh_pt_interval;

PIN_IMPORT void 
fm_mso_catv_channel_master_config_cache(
        pcm_context_t		*ctxp, 
        pin_errbuf_t		*ebufp);

PIN_IMPORT void
fm_mso_search_plan_detail( 
        pcm_context_t	*ctxp,
        pin_flist_t	*i_flistp,
        pin_flist_t	**r_flistpp,
        pin_errbuf_t	*ebufp);

EXPORT_OP void
op_mso_cust_get_pkg_channels(
        cm_nap_connection_t	*connp,
        int32			opcode,
        int32			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**r_flistpp,
        pin_errbuf_t		*ebufp);

static void
fm_mso_cust_get_pkg_channels(
        pcm_context_t		*ctxp,
        int32			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**r_flistpp,
        pin_errbuf_t		*ebufp);

static void
fm_mso_cust_get_channel_master_or_map(
        pcm_context_t           *ctxp,
        int32                   flag,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_mso_cust_get_product_channel_map(
        pcm_context_t           *ctxp,
        poid_t                  *plan_pdp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_mso_catv_channel_master_config_cache_update(
        pcm_context_t *ctxp,
        pin_errbuf_t  *ebufp);

void fm_mso_filter_plan_product_channel(
        pcm_context_t   *ctxp,
        pin_flist_t     *pc_flistp,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp);

void
fm_mso_get_channel_map_and_master(
        pcm_context_t           *ctxp,
        pin_flist_t             *filtered_flistp,
        pin_errbuf_t            *ebufp);


/*******************************************************************
 * MSO_OP_CUST_GET_PKG_CHANNELS  
 *******************************************************************/
void 
op_mso_cust_get_pkg_channels(
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
    if (opcode != MSO_OP_CUST_GET_PKG_CHANNELS) {
        pin_set_err(ebufp, PIN_ERRLOC_FM, 
                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_OPCODE, 0, 0, opcode);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                "op_mso_cust_get_pkg_channels error",
                ebufp);
        return;
    }

    /*******************************************************************
     * Debug: Input flist 
     *******************************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
            "op_mso_cust_get_pkg_channels input flist", i_flistp);

    /*******************************************************************
     * Call the default implementation 
     *******************************************************************/
    fm_mso_cust_get_pkg_channels(ctxp, flags, i_flistp, r_flistpp, ebufp);
    /***********************************************************
     * Results.
     ***********************************************************/

    if (PIN_ERRBUF_IS_ERR(ebufp)) 
    {	
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_get_pkg_channels error", ebufp);
    }
    else
    {
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_get_pkg_channels output flist", *r_flistpp);
    }
    return;
}

/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_get_pkg_channels(
        pcm_context_t		*ctxp,
        int32			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**r_flistpp,
        pin_errbuf_t		*ebufp)
{

    pin_flist_t		*s_flistp = NULL;
    pin_flist_t		*args_flistp = NULL;
    pin_flist_t		*res_flistp = NULL;
    pin_flist_t		*ret_flistp = NULL;
    pin_flist_t 	*temp_flistp = NULL;
    poid_t			*plan_product_pdp = NULL;
    poid_t			*s_pdp = NULL;

    int64			db = 1;
    int32			sflag = 256;
    int32			count = 0;
    int32			set_get_pkg_channel_map_failure = 1;
    char			*template = "select X from /mso_cfg_catv_pt_channel_map where F1 = V1  ";
    char			*template_1 = "select X from /mso_cfg_plan_products_map where F1 = V1 and F2 = V2 ";
    char			*template_2 = "select X from /mso_cfg_product_channel_map where F1 = V1 and F2 = V2 ";
    char			*plan_namep = NULL;
    char			*pt_namep = NULL;
    int32			elem_id = 0;
    pin_cookie_t	cookie = NULL;

    pin_flist_t	*channel_master_flistp = NULL;
    pin_flist_t	*new_channel_master_flistp = NULL;
    pin_flist_t	*data_array_flistp = NULL;
    pin_flist_t	*channel_flistp = NULL;
    pin_flist_t	*c_flistp = NULL;
    char		*channel_idp = NULL;
    int32		channel_id = 0;
    int32		*action_flagp = NULL;
    poid_t 		*product_pdp = NULL;
    poid_t          *product_pdp1 = NULL;
    poid_t          *prev_product_pdp = NULL;
    int32           counter = 0;
    pin_flist_t     *filtered_flistp = NULL;
    pin_flist_t     *out_flistp = NULL;
    time_t      end_date = 0;
    pin_decimal_t   *ncf_amtp = NULL;
    pin_decimal_t   *tmp_amtp = NULL;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;
    PIN_ERRBUF_CLEAR(ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_pkg_channels input flist", i_flistp);

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
                "Error in calling fm_mso_cust_get_pkg_channels", ebufp);
    }

    plan_product_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
    plan_namep = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_PLAN_NAME, 1, ebufp);
    if (!plan_namep)
    {
        plan_namep = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PRODUCT_NAME, 1, ebufp);
    }
    action_flagp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp);

    if (plan_namep != NULL)
    {
        s_flistp = PIN_FLIST_CREATE(ebufp);
        ret_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, s_flistp, PIN_FLD_POID, ebufp);	
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_NAME, plan_namep, ebufp);	

        fm_mso_search_plan_detail(ctxp, s_flistp, &ret_flistp, ebufp);

        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
        if (!ret_flistp)
        {
            ret_flistp = PIN_FLIST_CREATE(ebufp);
            PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID,ebufp);
            PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &set_get_pkg_channel_map_failure, ebufp);
            PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "84000", ebufp);
            PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Get PKG Channal Map - Plan not found", ebufp);
            *r_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
            goto CLEANUP;
        }
        else
        {
            PIN_FLIST_FLD_COPY(ret_flistp, PIN_FLD_POID, i_flistp, PIN_FLD_POID, ebufp);
            plan_product_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
            PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
        }
    }


    if (plan_product_pdp != NULL && !PIN_POID_IS_NULL(plan_product_pdp)) {

        s_flistp = PIN_FLIST_CREATE(ebufp);
        s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
        PIN_FLIST_FLD_PUT(s_flistp,PIN_FLD_POID, s_pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_FLAGS, (void*)&sflag, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
        if (action_flagp && *action_flagp == 11)
        {
            PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_TEMPLATE, template, ebufp);
            PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_PLAN_OBJ, plan_product_pdp, ebufp);
        }
        else if (action_flagp && *action_flagp == 2 || *action_flagp == 1 || *action_flagp == 3)
        {
            PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_TEMPLATE, template_1, ebufp);
            PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_PLAN_OBJ, plan_product_pdp, ebufp);
        
            args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
            PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_END_T, &end_date, ebufp);
        }
        else if (action_flagp && *action_flagp == 4)
        {
            PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_TEMPLATE, template_2, ebufp);
            PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_PRODUCT_OBJ, plan_product_pdp, ebufp);

            args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
            PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_END_T, &end_date, ebufp);
        }

        res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp );

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_cust_get_pkg_channels search i_flistp", s_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &ret_flistp,ebufp); 
        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_cust_get_pkg_channels search o_flistp", ret_flistp);

        count = PIN_FLIST_ELEM_COUNT(ret_flistp, PIN_FLD_RESULTS, ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_cust_get_pkg_channels error",ebufp);
            PIN_ERRBUF_RESET(ebufp);
            PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
            ret_flistp = PIN_FLIST_CREATE(ebufp);
            PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID,ebufp);
            PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &set_get_pkg_channel_map_failure, ebufp);
            PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "84001", ebufp);
            PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Get PKG Channal Map Search Error", ebufp);

            *r_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
            goto CLEANUP;
        }

        if (count == 0) {
            PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
            ret_flistp = PIN_FLIST_CREATE(ebufp);
            PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID,ebufp);
            PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &set_get_pkg_channel_map_failure, ebufp);
            PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "84000", ebufp);
            PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "No PKG Channel Mapping found", ebufp);

            *r_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
            goto CLEANUP;
        }
        else{
            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_pkg_channels return flist", ret_flistp);
        }

    }
    else{
        ret_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID,ebufp);
        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &set_get_pkg_channel_map_failure, ebufp);
        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "84002", ebufp);
        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "PLAN OBJ is null", ebufp);

        *r_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
        goto CLEANUP;
    }


    /*Adding the unassigned channels
    if (ret_flistp != NULL && action_flagp != NULL && *action_flagp == 2) {
        fm_mso_cust_get_channel_master_or_map(ctxp, MSO_OBJECT_CHANNEL_MASTER, &channel_master_flistp, ebufp);
        if(channel_master_flistp != NULL) {
            new_channel_master_flistp = PIN_FLIST_CREATE(ebufp);
            data_array_flistp = PIN_FLIST_ELEM_ADD(new_channel_master_flistp, PIN_FLD_DATA_ARRAY, 0, ebufp);
        }
        while((channel_flistp =
                    PIN_FLIST_ELEM_GET_NEXT(channel_master_flistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp )) != NULL) {
            channel_idp = PIN_FLIST_FLD_GET(channel_flistp, MSO_FLD_CHANNEL_ID, 1, ebufp);
            if(channel_idp != NULL && strlen(channel_idp) > 0) {
                channel_id = atoi(channel_idp);
            }
            c_flistp = PIN_FLIST_ELEM_ADD(data_array_flistp, PIN_FLD_RESULTS, channel_id, ebufp);
            PIN_FLIST_FLD_COPY(channel_flistp, MSO_FLD_CHANNEL_ID, c_flistp, MSO_FLD_CHANNEL_ID, ebufp);
            PIN_FLIST_FLD_COPY(channel_flistp, MSO_FLD_CHANNEL_NAME, c_flistp, MSO_FLD_CHANNEL_NAME, ebufp);
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"SureshHere", data_array_flistp);
        elem_id = 0;
        cookie = NULL;
        res_flistp = PIN_FLIST_ELEM_GET(ret_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
        if (res_flistp != NULL) {
            while((channel_flistp =	PIN_FLIST_ELEM_GET_NEXT(res_flistp, MSO_FLD_CATV_CHANNELS, 
                            &elem_id, 1, &cookie, ebufp )) != NULL) 
            {
                channel_idp = PIN_FLIST_FLD_GET(channel_flistp, MSO_FLD_CHANNEL_ID, 1, ebufp);
                if(channel_idp != NULL && strlen(channel_idp) > 0) {
                    channel_id = atoi(channel_idp);
                    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Dropping");
                    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, channel_idp);
                    PIN_FLIST_ELEM_DROP(data_array_flistp, PIN_FLD_RESULTS, channel_id, ebufp);
                }
            }
            if (channel_master_flistp != NULL) {
                fm_mso_filter_plan_product_channel(ctxp, res_flistp, &filtered_flistp, ebufp);
                PIN_FLIST_CONCAT(filtered_flistp, new_channel_master_flistp, ebufp);
            }
        }
        else{
            PIN_FLIST_FLD_DROP(ret_flistp, PIN_FLD_STATUS, ebufp);
            PIN_FLIST_FLD_DROP(ret_flistp, PIN_FLD_ERROR_CODE, ebufp);
            PIN_FLIST_FLD_DROP(ret_flistp, PIN_FLD_ERROR_DESCR, ebufp);
            PIN_FLIST_ELEM_ADD(ret_flistp, PIN_FLD_RESULTS, 0, ebufp);
            if (channel_master_flistp != NULL) {
                PIN_FLIST_CONCAT(ret_flistp, new_channel_master_flistp, ebufp);
            }
        }
    }*/
    if (ret_flistp != NULL && action_flagp != NULL && (*action_flagp == 1 || *action_flagp == 3 || *action_flagp == 11)) {

        out_flistp = PIN_FLIST_ELEM_TAKE(ret_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);		
        ncf_amtp = pbo_decimal_from_str("0.0", ebufp);

        while (out_flistp && (channel_flistp = PIN_FLIST_ELEM_TAKE_NEXT(out_flistp, PIN_FLD_PRODUCTS, 
            &elem_id, 1, &cookie, ebufp )) != NULL) 
        {
            plan_product_pdp = PIN_FLIST_FLD_GET(channel_flistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
            fm_mso_cust_get_product_channel_map(ctxp, plan_product_pdp, &channel_master_flistp, ebufp);

            if (filtered_flistp == NULL)
            {
                filtered_flistp = PIN_FLIST_CREATE(ebufp);
            }
            if (channel_master_flistp)
            {
                pt_namep = PIN_FLIST_FLD_GET(channel_master_flistp, PIN_FLD_PROVISIONING_TAG, 1, ebufp);
                if (strstr(pt_namep, "NCF"))
                {
                    tmp_amtp = PIN_FLIST_FLD_GET(channel_master_flistp, MSO_FLD_PRICE, 1, ebufp);
                    if (tmp_amtp) pbo_decimal_add_assign(ncf_amtp, tmp_amtp, ebufp);
                }
                PIN_FLIST_ELEM_PUT(filtered_flistp, channel_master_flistp, PIN_FLD_PRODUCTS, elem_id, ebufp);
            }
        }
        
        if (*action_flagp == 11 )
        {
            fm_mso_filter_plan_product_channel(ctxp, out_flistp, &filtered_flistp, ebufp);
        }
        else
        {
            PIN_FLIST_CONCAT(filtered_flistp, out_flistp, ebufp);
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"filtered_flistp", filtered_flistp);
        
        fm_mso_get_channel_map_and_master(ctxp, filtered_flistp, ebufp);

        if (*action_flagp == 1)
        {
            tmp_amtp = PIN_FLIST_FLD_GET(filtered_flistp, MSO_FLD_RATE, 0, ebufp);
            PIN_FLIST_FLD_PUT(filtered_flistp, MSO_FLD_SUBSCRIPTION_CHARGE, pbo_decimal_subtract(tmp_amtp, ncf_amtp,  ebufp), ebufp);
            PIN_FLIST_FLD_PUT(filtered_flistp, MSO_FLD_NCF_CHARGE, ncf_amtp, ebufp);
        }
        if (*action_flagp == 3)
        {
            *r_flistpp = PIN_FLIST_CREATE(ebufp);
            PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);	
            res_flistp = PIN_FLIST_ELEM_ADD(*r_flistpp, PIN_FLD_RESULTS, 0, ebufp);
            PIN_FLIST_CONCAT(res_flistp, filtered_flistp, ebufp);
        }
        else
        {
            *r_flistpp = PIN_FLIST_COPY(filtered_flistp, ebufp);
        }
    } 
    else
    {
        *r_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
    }

CLEANUP:
    PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&out_flistp, NULL);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_pkg_channels return flist", *r_flistpp);
    return;
}

static void
fm_mso_cust_get_channel_master_or_map(
        pcm_context_t           *ctxp,
        int32                   flag,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{

    pin_flist_t             *s_flistp = NULL;
    pin_flist_t             *args_flistp = NULL;
    pin_flist_t             *res_flistp = NULL;
    pin_flist_t             *ret_flistp = NULL;

    poid_t                  *dummy_pdp = NULL;
    poid_t                  *s_pdp = NULL;

    int64                   db = 1;
    int32                   sflag = 256;
    int32                   count = 0;
    char                    *tmp_template = "select X from %s where F1.type = V1 ";
    char                    template[256];
    char                    object_type[80] = "/mso_cfg_catv_channel_master";

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;
    PIN_ERRBUF_CLEAR(ebufp);

    if(flag != MSO_OBJECT_CHANNEL_MASTER) {
        strcpy(object_type, "/mso_cfg_catv_pt_channel_map");
    }

    sprintf(template,tmp_template,object_type);

    s_flistp = PIN_FLIST_CREATE(ebufp);
    s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(s_flistp,PIN_FLD_POID, s_pdp, ebufp);
    PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_FLAGS, (void*)&sflag, ebufp);
    PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_TEMPLATE, template, ebufp);
    args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
    dummy_pdp = PIN_POID_CREATE(db, object_type, -1, ebufp);
    PIN_FLIST_FLD_PUT(args_flistp,PIN_FLD_POID, dummy_pdp, ebufp);
    res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_cust_get_pkg_channels search i_flistp", s_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &ret_flistp,ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_cust_get_pkg_channels search o_flistp", ret_flistp);

    count = PIN_FLIST_ELEM_COUNT(ret_flistp, PIN_FLD_RESULTS, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_cust_get_pkg_channels error",ebufp);
    }

    *r_flistpp = ret_flistp;

    PIN_FLIST_DESTROY_EX(&s_flistp, NULL);

    return;
}

static void
fm_mso_cust_get_product_channel_map(
        pcm_context_t           *ctxp,
        poid_t                  *product_pdp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{

    pin_flist_t             *s_flistp = NULL;
    pin_flist_t             *args_flistp = NULL;
    pin_flist_t             *res_flistp = NULL;
    pin_flist_t             *ret_flistp = NULL;

    poid_t                  *s_pdp = NULL;

    int64                   db = 1;
    int32                   sflag = 256;
    int32                   count = 0;
    char			        *template = "select X from /mso_cfg_product_channel_map where F1 = V1 and F2 = V2 ";
    time_t                  end_date = 0;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;
    PIN_ERRBUF_CLEAR(ebufp);

    s_flistp = PIN_FLIST_CREATE(ebufp);
    s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(s_flistp,PIN_FLD_POID, s_pdp, ebufp);
    PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_FLAGS, (void*)&sflag, ebufp);
    PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_TEMPLATE, template, ebufp);

    args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRODUCT_OBJ, product_pdp, ebufp);

    args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
    PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_END_T, &end_date, ebufp);

    res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_cust_get_pkg_channels search i_flistp", s_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &ret_flistp,ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_cust_get_pkg_channels search o_flistp", ret_flistp);

    count = PIN_FLIST_ELEM_COUNT(ret_flistp, PIN_FLD_RESULTS, ebufp);

    if (count > 0)
    {
        *r_flistpp = PIN_FLIST_ELEM_TAKE(ret_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
    }

    PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
    return;
}

void fm_mso_filter_plan_product_channel(
        pcm_context_t   *ctxp,
        pin_flist_t     *pc_flistp,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp)
{
    pin_flist_t     *res_flistp = NULL;
    pin_flist_t     *data_flistp = NULL;
    pin_flist_t     *tmp_flistp = NULL;
    pin_flist_t     *s_plan_flistp = NULL;
    pin_flist_t     *sort_template_flistp = NULL;
    poid_t          *product_pdp = NULL;
    poid_t          *prev_product_pdp = NULL;
    pin_decimal_t	*ncf_amtp = NULL;
    pin_decimal_t	*tmp_amtp = NULL;
    char		*channel_idp = NULL;
    int32           counter = 0;
    int32           elem_id = 0;
    int32		channel_count = 0;
    int32		elem_counter = 0;
    pin_cookie_t    cookie = NULL;

    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                "Error in calling fm_mso_cust_get_pkg_channels", ebufp);
        return;
    }

    sort_template_flistp = PIN_FLIST_CREATE(ebufp);
    s_plan_flistp = PIN_FLIST_ELEM_ADD(sort_template_flistp, MSO_FLD_CATV_CHANNELS, PIN_ELEMID_ANY, ebufp);
    PIN_FLIST_FLD_SET(s_plan_flistp, PIN_FLD_PRODUCT_OBJ, 0,ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Sorting template flist", sort_template_flistp);
    PIN_FLIST_SORT(pc_flistp, sort_template_flistp, 0, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Sorted plan_channel_list", pc_flistp);

    //channel_count = PIN_FLIST_ELEM_COUNT(pc_flistp, MSO_FLD_CATV_CHANNELS, ebufp);
    ncf_amtp = pbo_decimal_from_str("0.0", ebufp);

    while((res_flistp = PIN_FLIST_ELEM_TAKE_NEXT(pc_flistp, MSO_FLD_CATV_CHANNELS,
                    &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) 
    {
        channel_idp = PIN_FLIST_FLD_GET(res_flistp, MSO_FLD_CHANNEL_ID, 0, ebufp);
        if (!strstr(channel_idp, "N/A"))
        {
            channel_count = channel_count + 1;
        }
        else
        {
            tmp_amtp = PIN_FLIST_FLD_GET(res_flistp, MSO_FLD_PRICE, 0, ebufp);
            pbo_decimal_add_assign(ncf_amtp, tmp_amtp, ebufp);
        }

        product_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);

        if(!product_pdp) {
            continue;
        }
        if (counter == 0) {
            data_flistp = PIN_FLIST_CREATE(ebufp);
            tmp_flistp = PIN_FLIST_ELEM_ADD(data_flistp, PIN_FLD_PRODUCTS, 0, ebufp);
            PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_PRODUCT_OBJ, tmp_flistp, PIN_FLD_PRODUCT_OBJ, ebufp);
            PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_PRODUCT_NAME, tmp_flistp, PIN_FLD_PRODUCT_NAME, ebufp);
            PIN_FLIST_FLD_COPY(res_flistp,PIN_FLD_PROVISIONING_TAG,
                    tmp_flistp,PIN_FLD_PROVISIONING_TAG,ebufp);
            PIN_FLIST_FLD_COPY(res_flistp, MSO_FLD_PRICE, tmp_flistp, MSO_FLD_PRICE, ebufp);

            PIN_FLIST_FLD_DROP(res_flistp, PIN_FLD_PRODUCT_OBJ, ebufp);
            PIN_FLIST_FLD_DROP(res_flistp, PIN_FLD_PRODUCT_NAME, ebufp);
            PIN_FLIST_FLD_DROP(res_flistp, PIN_FLD_PROVISIONING_TAG, ebufp);
            PIN_FLIST_FLD_DROP(res_flistp, MSO_FLD_PRICE, ebufp);
            PIN_FLIST_FLD_DROP(res_flistp, MSO_FLD_PKG_TYPE, ebufp);
            PIN_FLIST_FLD_DROP(res_flistp, PIN_FLD_DESCR, ebufp);

            PIN_FLIST_ELEM_PUT(tmp_flistp, res_flistp, MSO_FLD_CATV_CHANNELS, elem_id, ebufp);
            prev_product_pdp = product_pdp;
            counter++;
        }
        else if (product_pdp) {
            if ( PIN_POID_COMPARE(prev_product_pdp, product_pdp, 0, ebufp) != 0) {
                prev_product_pdp = product_pdp;
                tmp_flistp = PIN_FLIST_ELEM_ADD(data_flistp, PIN_FLD_PRODUCTS, ++counter, ebufp);
                PIN_FLIST_FLD_COPY(res_flistp,PIN_FLD_PRODUCT_OBJ,tmp_flistp,PIN_FLD_PRODUCT_OBJ,ebufp);
                PIN_FLIST_FLD_COPY(res_flistp,PIN_FLD_PRODUCT_NAME,
                        tmp_flistp,PIN_FLD_PRODUCT_NAME,ebufp);
                PIN_FLIST_FLD_COPY(res_flistp,PIN_FLD_PROVISIONING_TAG,
                        tmp_flistp,PIN_FLD_PROVISIONING_TAG,ebufp);
                PIN_FLIST_FLD_COPY(res_flistp, MSO_FLD_PRICE, tmp_flistp, MSO_FLD_PRICE, ebufp);
                PIN_FLIST_FLD_COPY(res_flistp, MSO_FLD_PKG_TYPE, tmp_flistp, MSO_FLD_PKG_TYPE, ebufp);
                PIN_FLIST_FLD_DROP(res_flistp, PIN_FLD_PRODUCT_OBJ, ebufp);
                PIN_FLIST_FLD_DROP(res_flistp, PIN_FLD_PRODUCT_NAME, ebufp);
                PIN_FLIST_FLD_DROP(res_flistp, PIN_FLD_PROVISIONING_TAG, ebufp);
                PIN_FLIST_FLD_DROP(res_flistp, MSO_FLD_PRICE, ebufp);
                PIN_FLIST_FLD_DROP(res_flistp, MSO_FLD_PKG_TYPE, ebufp);
                PIN_FLIST_FLD_DROP(res_flistp, PIN_FLD_DESCR, ebufp);

                PIN_FLIST_ELEM_PUT(tmp_flistp, res_flistp, MSO_FLD_CATV_CHANNELS, elem_id, ebufp);
            }
            else {
                PIN_FLIST_FLD_COPY(res_flistp,PIN_FLD_PRODUCT_OBJ,tmp_flistp,PIN_FLD_PRODUCT_OBJ,ebufp);
                PIN_FLIST_FLD_COPY(res_flistp,PIN_FLD_PRODUCT_NAME,
                        tmp_flistp,PIN_FLD_PRODUCT_NAME,ebufp);
                PIN_FLIST_FLD_COPY(res_flistp,PIN_FLD_PROVISIONING_TAG,
                        tmp_flistp,PIN_FLD_PROVISIONING_TAG,ebufp);
                PIN_FLIST_FLD_COPY(res_flistp, MSO_FLD_PRICE, tmp_flistp, MSO_FLD_PRICE, ebufp);
                PIN_FLIST_FLD_COPY(res_flistp, MSO_FLD_PKG_TYPE, tmp_flistp, MSO_FLD_PKG_TYPE, ebufp);

                PIN_FLIST_FLD_DROP(res_flistp, PIN_FLD_PRODUCT_OBJ, ebufp);
                PIN_FLIST_FLD_DROP(res_flistp, PIN_FLD_PRODUCT_NAME, ebufp);
                PIN_FLIST_FLD_DROP(res_flistp, PIN_FLD_PROVISIONING_TAG, ebufp);
                PIN_FLIST_FLD_DROP(res_flistp, MSO_FLD_PRICE, ebufp);
                PIN_FLIST_FLD_DROP(res_flistp, MSO_FLD_PKG_TYPE, ebufp);
                PIN_FLIST_FLD_DROP(res_flistp, PIN_FLD_DESCR, ebufp);

                PIN_FLIST_ELEM_PUT(tmp_flistp, res_flistp, MSO_FLD_CATV_CHANNELS, elem_id, ebufp);
            }
        }

        if (elem_id > elem_counter)
        {
            elem_counter = elem_id;
        }
    }
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"data_flistp", data_flistp);
    PIN_FLIST_CONCAT(pc_flistp, data_flistp, ebufp);
    PIN_FLIST_FLD_SET(pc_flistp, PIN_FLD_REFERENCE_COUNT, &channel_count, ebufp);
    tmp_amtp = PIN_FLIST_FLD_GET(pc_flistp, MSO_FLD_RATE, 0, ebufp);
    PIN_FLIST_FLD_PUT(pc_flistp, MSO_FLD_SUBSCRIPTION_CHARGE, pbo_decimal_subtract(tmp_amtp, ncf_amtp,  ebufp), ebufp);
    PIN_FLIST_FLD_PUT(pc_flistp, MSO_FLD_NCF_CHARGE, ncf_amtp, ebufp);
    PIN_FLIST_FLD_SET(pc_flistp, PIN_FLD_ROLLOVER_COUNT, &elem_counter, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"pc_flistp", pc_flistp);

    *r_flistpp = pc_flistp;

}

void
fm_mso_get_channel_map_and_master(
        pcm_context_t           *ctxp,
        pin_flist_t		*filtered_flistp,
        pin_errbuf_t            *ebufp)
{
    pin_flist_t     *cache_flistp = NULL;
    pin_flist_t     *tmp_flistp = NULL;
    time_t          last_mod_t = 0;
    time_t          now_t = 0;
    cm_cache_key_iddbstr_t    kids;
    int32            err = PIN_ERR_NONE;
    pin_cookie_t    cookie = NULL;
    int32           rec_id = 0;
    pin_cookie_t    cookie_p = NULL;
    int32           elem_id_p = 0;
    pin_cookie_t    cookie_c = NULL;
    int32           elem_id_c = 0;
    int32		channel_count = 0;
    int32		tmp_chcount = 0;
    int32		plan_ncf_count = 0;
    pin_flist_t	*channel_flistp = NULL;
    char            *channel_name = NULL;
    pin_flist_t     *res_flistp = NULL;

    if ( PIN_ERRBUF_IS_ERR(ebufp) )
        return;
    PIN_ERRBUF_CLEAR(ebufp);

    /******************************************************
     * If the cache is not enabled, short circuit right away
     ******************************************************/
    if ( mso_cfg_catv_channel_master_config_ptr == (cm_cache_t *)NULL ) {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
                "no config flist for /mso_cfg_catv_channel_master cached ");
        pin_set_err(ebufp, PIN_ERRLOC_CM,
                PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_FLD_CACHE_NAME, 0, 0);

    }
    /**************************************************
     * See if the entry is in our data dictionary cache
     * Prepare the cm_cache_key_iddbstr_t structure to search
     **************************************************/
    kids.id = 0;
    kids.db = 0;
    kids.str = "/mso_cfg_catv_channel_master";
    cache_flistp = cm_cache_find_entry(mso_cfg_catv_channel_master_config_ptr,
            &kids, &err);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "cm_cache_find_entry found flist", cache_flistp);
    switch(err) {
        case PIN_ERR_NONE:
            break;
        default:
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
                    "mso_prov_catv_config_object from_cache: error "
                    "accessing data dictionary cache.");
            pin_set_err(ebufp, PIN_ERRLOC_CM,
                    PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_FLD_CACHE_NAME, 0, 0);
            goto cleanup;
    }

    // Addding check to remove core dump
    if((time_t*)PIN_FLIST_FLD_GET(cache_flistp, PIN_FLD_MOD_T,0,ebufp) != NULL )
        last_mod_t = *(time_t*)PIN_FLIST_FLD_GET(cache_flistp, PIN_FLD_MOD_T,0,ebufp);

    now_t = pin_virtual_time((time_t *)NULL);
    if(now_t >= last_mod_t + refresh_pt_interval )
    {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"calling mso_prov_catv_config_object_update to refresh config");
        fm_mso_catv_channel_master_config_cache_update(ctxp, ebufp);
        kids.id = 0;
        kids.db = 0;
        kids.str = "/mso_cfg_catv_channel_master";
        cache_flistp = cm_cache_find_entry(mso_cfg_catv_channel_master_config_ptr, &kids, &err);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "cm_cache_find_entry found flist", cache_flistp);
        switch(err)
        {
            case PIN_ERR_NONE:
                break;
            default:
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
                        "mso_prov_catv_config_object from_cache: error acessing data dictionary cache.");
                pin_set_err(ebufp, PIN_ERRLOC_CM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_FLD_CACHE_NAME, 0, 0);
                goto cleanup;
        }
    }

    elem_id_p = 0;
    cookie_p = NULL;

    while((res_flistp = PIN_FLIST_ELEM_GET_NEXT(filtered_flistp, PIN_FLD_PRODUCTS,
                    &elem_id_p, 1, &cookie_p, ebufp)) != (pin_flist_t *)NULL)
    {
        elem_id_c = 0;
        cookie_c = NULL;
        channel_count = 0;
        while((channel_flistp = PIN_FLIST_ELEM_GET_NEXT(res_flistp, MSO_FLD_CATV_CHANNELS,
                        &elem_id_c, 1, &cookie_c, ebufp )) != NULL)
        {
            channel_name = (char *)PIN_FLIST_FLD_GET(channel_flistp, MSO_FLD_CHANNEL_NAME, 1, ebufp);
            rec_id = 0;
            cookie = NULL;
            while ( (tmp_flistp = PIN_FLIST_ELEM_GET_NEXT (cache_flistp,
                            PIN_FLD_RESULTS, &rec_id, 1,&cookie, ebufp))!= (pin_flist_t *)NULL )
            {
                if (strcmp((char *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_CHANNEL_NAME, 0, ebufp ),
                            channel_name) == 0 )
                {
                    tmp_chcount = *(int32 *)PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_CHANNEL_TYPE, 0, ebufp);
                    channel_count = channel_count + tmp_chcount;
                    PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_CHANNEL_TYPE, 
                            channel_flistp, MSO_FLD_CHANNEL_TYPE, ebufp);
                    PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_CHANNEL_GENRE, 
                            channel_flistp, MSO_FLD_CHANNEL_GENRE, ebufp);
                    PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_QUALITY, channel_flistp, MSO_FLD_QUALITY, ebufp);
                    PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_PRICE, channel_flistp, MSO_FLD_PRICE, ebufp);
                    PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_LANGUAGE, channel_flistp, MSO_FLD_LANGUAGE, ebufp);
                    PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_BROADCASTER, channel_flistp, MSO_FLD_BROADCASTER, ebufp);
                }
            }
        }
        PIN_FLIST_FLD_SET(res_flistp, MSO_FLD_NCF_COUNT, &channel_count, ebufp);
        plan_ncf_count = plan_ncf_count + channel_count;
    }
    PIN_FLIST_FLD_SET(filtered_flistp, MSO_FLD_NCF_COUNT, &plan_ncf_count, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"filtered_flistp after adding channel details", filtered_flistp);	
cleanup:
    if ( !cache_flistp )
    {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CATV prov cache Information not available in the cache.");
    }
    PIN_FLIST_DESTROY_EX(&cache_flistp, NULL);   

}

/*********************************************************************
 * fm_mso_catv_channel_master_config_cache_update()
 * This function will update cache
 *********************************************************************/
static void
fm_mso_catv_channel_master_config_cache_update(
        pcm_context_t	*ctxp,
        pin_errbuf_t	*ebufp)
{
    pin_flist_t             *s_flistp = NULL;
    pin_flist_t		*r_flistp = NULL;
    pin_flist_t		*res_flistp = NULL;
    pin_flist_t		*arg_flistp = NULL;
    poid_t			*s_pdp = NULL;
    poid_t			*pdp = NULL;
    int64			database;
    cm_cache_key_iddbstr_t  kids;
    time_t                  now = 0;
    int32                   s_flags = 256;
    int32			no_of_buckets = 1;
    int32			elem_count = 0;
    int32			err = PIN_ERR_NONE;
    int32			cnt;

    s_flistp = PIN_FLIST_CREATE(ebufp);

    /***********************************************************
     * assume db matches userid found in pin.conf
     ***********************************************************/
    pdp = PCM_GET_USERID(ctxp);
    database = PIN_POID_GET_DB(pdp);

    s_pdp = PIN_POID_CREATE(database, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
    PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
    PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE,
            "select X from /mso_cfg_catv_channel_master where F1.db = V1 ", ebufp);
    arg_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

    res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
    PIN_FLIST_FLD_SET(res_flistp, MSO_FLD_BROADCASTER, NULL, ebufp);
    PIN_FLIST_FLD_SET(res_flistp, MSO_FLD_CHANNEL_GENRE, NULL, ebufp);
    PIN_FLIST_FLD_SET(res_flistp, MSO_FLD_CHANNEL_ID, NULL, ebufp);
    PIN_FLIST_FLD_SET(res_flistp, MSO_FLD_CHANNEL_NAME, NULL, ebufp);
    PIN_FLIST_FLD_SET(res_flistp, MSO_FLD_CHANNEL_TYPE, NULL, ebufp);
    PIN_FLIST_FLD_SET(res_flistp, MSO_FLD_LANGUAGE, NULL, ebufp);
    PIN_FLIST_FLD_SET(res_flistp, MSO_FLD_POPULARITY, NULL, ebufp);
    PIN_FLIST_FLD_SET(res_flistp, MSO_FLD_PRICE, NULL, ebufp);
    PIN_FLIST_FLD_SET(res_flistp, MSO_FLD_QUALITY, NULL, ebufp);
    PIN_FLIST_FLD_SET(res_flistp, MSO_FLD_TRANSMISSION, NULL, ebufp);
    PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_DISTRIBUTOR, NULL, ebufp);
    PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_TYPE, NULL, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "cm_cache_find_entry input flist", s_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "cm_cache_find_entry output flist", r_flistp)

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                    "fm_mso_prov_catv_config_cache: error loading /mso_cfg_catv_channel_master object",
                    ebufp);
            goto cleanup;
        }

    cnt = PIN_FLIST_COUNT(r_flistp, ebufp);

    if (cnt <= 1) {
        goto cleanup;
    }

    PIN_FLIST_FLD_DROP(r_flistp, PIN_FLD_POID, ebufp);
    //Caching
    now = pin_virtual_time((time_t *)NULL);
    PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_MOD_T,&now,ebufp);
    //
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "cm_cache_find_entry res_flist", r_flistp)

        /*
         * Get the number of items in the flist so we can create an
         * appropriately sized cache
         */
        cnt = 0;
    if (r_flistp) {
        elem_count = PIN_FLIST_ELEM_COUNT(r_flistp, PIN_FLD_RESULTS, ebufp);
        res_flistp = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_RESULTS, 0, PIN_ELEMID_ANY, ebufp);
        cnt = PIN_FLIST_COUNT(res_flistp, ebufp);
        cnt = cnt * elem_count;
    }

    /*
     * If there's no data, there's no point initializing it, is there.
     * Especially since this is optional!
     */
    if (cnt < 1) {
        goto cleanup;
    }

    if (!mso_cfg_catv_channel_master_config_ptr)
    {
        goto cleanup;
    }

    kids.id  = 0;
    kids.db  = 0;
    kids.str = "/mso_cfg_catv_channel_master";
    cm_cache_update_entry(mso_cfg_catv_channel_master_config_ptr, &kids, r_flistp, &err);

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_cfg_channel_master_object cache error", ebufp);
    }
    else
    {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_cfg_channel_master_object cache loaded successfully");
    }

cleanup:
    PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
    PIN_POID_DESTROY(pdp, NULL);
    return;
}
