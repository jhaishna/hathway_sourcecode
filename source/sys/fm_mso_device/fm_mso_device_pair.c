/*
 * (#)$Id: fm_mso_device_pair.c st_cgbubrm_gsivakum_bug-8453030/1 2009/05/05 23:37:48 gsivakum Exp $
 *
 * Copyright (c) 2001, 2009, Oracle and/or its affiliates. All rights reserved.
 *
 * This material is the confidential property of Oracle Corporation or its
 * licensors and may be used, reproduced, stored or transmitted only in
 * accordance with a valid Oracle license or sublicense agreement.
 *
 */
/***************************************************************************************************************
*VERSION |   DATE    |    Author        |               DESCRIPTION                                            *
*--------------------------------------------------------------------------------------------------------------*
* 0.1    |12/12/2013 |     | CATV Device Pair Implementation
*
*--------------------------------------------------------------------------------------------------------------*
****************************************************************************************************************/


#ifndef lint
static const char Sccs_id[] = "(#)$Id: fm_mso_device_pair.c st_cgbubrm_gsivakum_bug-8453030/1 2009/05/05 23:37:48 gsivakum Exp $";
#endif

#include <stdio.h>
#include <string.h>
#include <pcm.h>
#include <pinlog.h>
#include "ops/device.h"
#include "pin_device.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "fm_utils.h"
#include "pin_cust.h"
#include "mso_device.h"
#include "mso_prov.h"
#include "mso_ops_flds.h"


#define FILE_LOGNAME "fm_mso_device_pair.c"


/*******************************************************************************
 * Functions Defined outside this source file
 ******************************************************************************/
extern int32
fm_mso_trans_open(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	int32		flag,
	pin_errbuf_t	*ebufp);

extern void 
fm_mso_trans_commit(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	pin_errbuf_t	*ebufp);

extern void 
fm_mso_trans_abort(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	pin_errbuf_t	*ebufp);


/*******************************************************************************
 * Functions contained within this source file
 ******************************************************************************/
EXPORT_OP void
op_mso_device_pair(
    cm_nap_connection_t    *connp,
    int32            opcode,
    int32            flags,
    pin_flist_t        *i_flistp,
    pin_flist_t        **o_flistpp,
    pin_errbuf_t        *ebufp);

static void
fm_mso_device_pair(
    pcm_context_t        *ctxp,
    pin_flist_t        *i_flistp,
    int32              *error_flag,
    pin_flist_t        **ret_flistpp,
    pin_errbuf_t       *ebufp);

void fm_mso_pair_device(
	pcm_context_t	*ctxp, 
	poid_t		*device_poid,
	char		*pairing_device_id, 
	poid_t		*stb_poid,
	char		*pairing_device_id_vc, 
	pin_errbuf_t	*ebufp);

int32 fm_mso_depair_device(
	pcm_context_t	*ctxp, 
	poid_t		*device_poid,
	char		*stb_id, 	
	poid_t		*device_poid_stb,
	char		*vc_id, 	
	pin_errbuf_t	*ebufp);

static void
fm_mso_get_device_info(
		pcm_context_t           *ctxp,
		char             *device_vc,
		char             *device_stb,
		int32				*flag,
		pin_flist_t             *i_flistp,
		pin_flist_t             **ret_flistp,
		pin_errbuf_t            *ebufp);



/*******************************************************************************

 ******************************************************************************/
void
op_mso_device_pair(
    cm_nap_connection_t    *connp,
    int32            opcode,
    int32            flags,
    pin_flist_t        *i_flistp,
    pin_flist_t        **o_flistpp,
    pin_errbuf_t        *ebufp)
{
    pcm_context_t        *ctxp = connp->dm_ctx;
    poid_t               *pdp = NULL;
    int32                t_status;
    int32                error_set = 0;
	int32				 flag=0;

    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR(ebufp);

    /*
     * Insanity check.
     */
    if (opcode != MSO_OP_DEVICE_PAIR) 
    {

        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
            PIN_ERR_BAD_OPCODE, 0, 0, opcode);

        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "bad opcode in op_mso_device_pair", ebufp);

        *o_flistpp = NULL;
        return;
    }


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "op_mso_device_pair input flist", i_flistp);

//    pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
//   t_status = fm_mso_trans_open(ctxp, pdp, 1, ebufp);

    fm_mso_device_pair(ctxp, i_flistp, &error_set, o_flistpp, ebufp);

/*    if ((PIN_ERRBUF_IS_ERR(ebufp) ||  error_set == 1 ) && t_status == 0 ) 
    {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"abort transaction");
        fm_mso_trans_abort(ctxp, pdp, ebufp);
    }
    else if ( ! PIN_ERRBUF_IS_ERR(ebufp) &&   error_set == 0 && t_status == 0)
    {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"commit transaction");
        fm_mso_trans_commit(ctxp, pdp, ebufp);
    }*/

    if (PIN_ERRBUF_IS_ERR(ebufp) ) 
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_mso_device_pair error", ebufp);
        //unhandled error -- Ideally should not 
        error_set = 1;
        PIN_ERRBUF_CLEAR(ebufp);
        *o_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *o_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_STATUS, &error_set, ebufp);
    }
    else if ( ! PIN_ERRBUF_IS_ERR(ebufp) )
    {
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *o_flistpp, PIN_FLD_POID, ebufp);
        //PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_STATUS, 0, ebufp);
    }

     PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "op_mso_device_pair return flist",
            *o_flistpp);

    return;
}


/*******************************************************************************
 * fm_mso_device_pair()
 *
 * Description:
 *
 ******************************************************************************/
static void
fm_mso_device_pair(
    pcm_context_t        *ctxp,
    pin_flist_t        *i_flistp,
    int32               *error_flag,
    pin_flist_t        **ret_flistpp,
    pin_errbuf_t        *ebufp)
{
    char                    *vc_id = NULL;
    char                    *get_vc = NULL;
	char                    *stb_id = NULL;
	char                    *paired_stb = NULL;
    pin_flist_t             *vc_id_r_flistp = NULL;
    pin_flist_t             *set_state_flistp = NULL;
    pin_flist_t             *set_state_o_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    pin_flist_t             *sub_flistp = NULL;
    pin_flist_t             *r_flist = NULL;
	pin_flist_t				*device_paired_flistp = NULL;
	
    poid_t                  *vc_poidp = NULL;
    poid_t                  *stb_poidp = NULL;
    char                    *program_name = "Pair/Un-pair VC/STB";
	int32					*flag = 0;
	int32					ret_flag = 0;
	int		elem_id = 0;

	pin_cookie_t	cookie = NULL;

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        return;
    }
    PIN_ERRBUF_CLEAR(ebufp);

    /*
     * Validate the input flist if required fields are present
     */
    vc_id = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_VC_ID, 1, ebufp);
	stb_id = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_STB_ID, 1, ebufp);
	flag =  PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp);
	
	if(vc_id == NULL ||  stb_id == NULL || PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
	    *error_flag = 1;
        *ret_flistpp = PIN_FLIST_CREATE(ebufp);
        //PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, error_flag, ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "52001", ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "VC and STB id are reqired in input", ebufp);
        goto CLEANUP;
	  
	} 

	if (*flag!=0 && *flag!=1 && *flag!=2  && *flag!=3)
	{
		PIN_ERRBUF_CLEAR(ebufp);
	    *error_flag = 1;
        *ret_flistpp = PIN_FLIST_CREATE(ebufp);
        //PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, error_flag, ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "52001", ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "invalid flag values passed  in input", ebufp);
        goto CLEANUP;
	  
	} 




    /*
     * Get VC id poid
     */
    fm_mso_get_device_info(ctxp, vc_id, stb_id, flag, i_flistp, &vc_id_r_flistp, ebufp);


	if (PIN_FLIST_ELEM_COUNT(vc_id_r_flistp, PIN_FLD_RESULTS, ebufp) == 2)
	{
		while((r_flist = PIN_FLIST_ELEM_GET_NEXT(vc_id_r_flistp, PIN_FLD_RESULTS,
						&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "r_flist ", r_flist);
			get_vc = PIN_FLIST_FLD_GET(r_flist, PIN_FLD_DEVICE_ID, 0, ebufp);
			if(strcmp(vc_id, get_vc) == 0 )
			{
				device_paired_flistp =  PIN_FLIST_ELEM_GET(r_flist, PIN_FLD_GROUP_DETAILS, PIN_ELEMID_ANY, 1, ebufp);
				vc_poidp = PIN_FLIST_FLD_GET(r_flist, PIN_FLD_POID, 0, ebufp);
				if (device_paired_flistp)
				{
					paired_stb = PIN_FLIST_FLD_GET(device_paired_flistp, PIN_FLD_NAME, 1, ebufp);
				}
			}
			if(strcmp(stb_id, get_vc) == 0 )
			{
				stb_poidp = PIN_FLIST_FLD_GET(r_flist, PIN_FLD_POID, 0, ebufp);
			}
		}			

		if(*flag==0 || *flag==2 )
		{	
			if(device_paired_flistp)
			{
				*error_flag = 1;
				*ret_flistpp = PIN_FLIST_CREATE(ebufp);
				//PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, error_flag, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "52002", ebufp);
				PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "VC is already paired", ebufp);
				goto CLEANUP;
			}
			else
			{
				fm_mso_pair_device(ctxp, vc_poidp, stb_id, stb_poidp, vc_id, ebufp);
				if(PIN_ERRBUF_IS_ERR(ebufp))
				{
					*error_flag = 1;
					*ret_flistpp = PIN_FLIST_CREATE(ebufp);
					//PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
					PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, error_flag, ebufp);
					PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "52003", ebufp);
					PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "DB Error in Pairing", ebufp);
					goto CLEANUP;
				}
				*ret_flistpp = PIN_FLIST_COPY(i_flistp,ebufp);
			}
		}
		else if(*flag==1 || *flag==3  )
		{
			if(device_paired_flistp)
			{				
				if(paired_stb && strcmp(paired_stb,stb_id)==0)
				{
					ret_flag = fm_mso_depair_device(ctxp, vc_poidp, stb_id, stb_poidp, vc_id, ebufp);
					if(PIN_ERRBUF_IS_ERR(ebufp) || ret_flag==0)
					{
						*error_flag = 1;
						*ret_flistpp = PIN_FLIST_CREATE(ebufp);
						//PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
						PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, error_flag, ebufp);
						PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "52004", ebufp);
						PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "Error in DePairing", ebufp);
						goto CLEANUP;
					}
					*ret_flistpp = PIN_FLIST_COPY(i_flistp,ebufp);
				}
				else
				{
					*error_flag = 1;
					*ret_flistpp = PIN_FLIST_CREATE(ebufp);
					//PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
					PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, error_flag, ebufp);
					PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "52005", ebufp);
					PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "Paired STB and provided STB do not match", ebufp);
					goto CLEANUP;

				}
			}
			else
			{												
					*error_flag = 1;
					*ret_flistpp = PIN_FLIST_CREATE(ebufp);
					//PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
					PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, error_flag, ebufp);
					PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "52006", ebufp);
					PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "VC is not paired with any device", ebufp);
					goto CLEANUP;						
			}

		}	

	}
	else
	{
		*error_flag = 1;
		*ret_flistpp = PIN_FLIST_CREATE(ebufp);
		//PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, error_flag, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "52007", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "VC/STB does not exists/not in ready to use state", ebufp);
		goto CLEANUP;
	}

    
      PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                    "fm_mso_device_pair set state output flist", *ret_flistpp);

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        PIN_ERRBUF_CLEAR(ebufp);
        *error_flag = 1;
        *ret_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, error_flag, ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "52009", ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "Paiing Failed for VC", ebufp);
        goto CLEANUP;
    }


CLEANUP:
    PIN_FLIST_DESTROY_EX(&set_state_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&vc_id_r_flistp, NULL);

    return;
}


static void
fm_mso_get_device_info(
		pcm_context_t           *ctxp,
		char             *device_vc,
		char             *device_stb,
		int32				*flag,
		pin_flist_t             *i_flistp,
		pin_flist_t             **ret_flistp,
		pin_errbuf_t            *ebufp)
{
	pin_flist_t             *srch_flistp = NULL;
	pin_flist_t             *arg_flist = NULL;
	pin_flist_t             *result_flist = NULL;
	pin_flist_t             *srch_out_flistp = NULL;
	pin_flist_t             *srvc_flistp = NULL;
	pin_flist_t             *devices_array_stb = NULL;
	pin_flist_t             *devices_array_vc = NULL;
	char                    *stb_device_id = NULL;
	char                    *res_device_id = NULL;
	char                    *inp_device_id = NULL;
	char    template[500] ="";
	char    mf[500] ="NDS1";
	int                   device_flag = 0;
	int32                                   den_sp = 0;
	poid_t                  *srch_pdp = NULL;
	poid_t          *dev_srch_pdp = NULL;
	pin_flist_t     *dev_srch_flistp = NULL;
	pin_flist_t     *dev_srch_out_flistp = NULL;
	pin_flist_t     *device_paired_flistp = NULL;
	pin_flist_t     *result_array = NULL;
	pin_flist_t     *group_detail_array = NULL;
	pin_flist_t     *out_flistp = NULL;
	pin_flist_t     *ro_flistp = NULL;
	pin_flist_t     *ri_flistp = PIN_FLIST_CREATE(ebufp);
	pin_flist_t     *vc_device_id_out_fp = NULL;
	int32                   search_fail = 1;
	char            *cas_type = NULL;
	int32                   srch_flag = 512;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_device_info input flist", i_flistp);

	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_ORDER_OBJ, ri_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(ri_flistp, PIN_FLD_USERID, NULL, ebufp);

	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, ri_flistp, &ro_flistp, ebufp);


	den_sp = fm_mso_is_den_sp(ctxp, ro_flistp, ebufp);


	if (den_sp == 2)
	{
		sprintf(mf,"NDS2");
	}
	else
	{
		sprintf(mf,"NDS1");
	}



	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_device_info", ebufp);
		return;
	}


	srch_pdp = PIN_POID_CREATE(1, "/search", -1, ebufp);
	dev_srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(dev_srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(dev_srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);

	if (*flag==3 || *flag==2 )
	{
		sprintf(template,"select X from /device where ( (F1 = V1 and device_t.poid_type ='/device/vc')  or (F2 = V2 and device_t.poid_type ='/device/stb' )) and device_t.state_id in (1,2,3)  and device_t.MANUFACTURER = '%s' ",mf);
	}
	else
	{
		sprintf(template,"select X from /device where ( (F1 = V1 and device_t.poid_type ='/device/vc')  or (F2 = V2 and device_t.poid_type ='/device/stb' )) and device_t.state_id = 1  and device_t.MANUFACTURER = '%s'  ", mf);
	}

	PIN_FLIST_FLD_SET(dev_srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
	arg_flist = PIN_FLIST_ELEM_ADD(dev_srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_DEVICE_ID, device_vc, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(dev_srch_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_DEVICE_ID, device_stb, ebufp);


	result_flist = PIN_FLIST_ELEM_ADD(dev_srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_DEVICE_ID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_MANUFACTURER, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STATE_ID, NULL, ebufp);
	device_paired_flistp = PIN_FLIST_ELEM_ADD (result_flist, PIN_FLD_GROUP_DETAILS, 0, ebufp);
	PIN_FLIST_FLD_SET(device_paired_flistp, PIN_FLD_NAME, NULL, ebufp);


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_device_info search device using STB and paired VC input flist", dev_srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, dev_srch_flistp, &dev_srch_out_flistp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_device_info search device using STB and paired VC output flist", dev_srch_out_flistp);





	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Returning dev_srch_out_flistp stb");


	*ret_flistp = PIN_FLIST_COPY(dev_srch_out_flistp,ebufp);
	goto CLEANUP;


CLEANUP:
	PIN_FLIST_DESTROY_EX(&dev_srch_out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&ro_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&ri_flistp, NULL);


	return;
}



void fm_mso_pair_device(
	pcm_context_t	*ctxp, 
	poid_t		*device_poid,
	char		*pairing_device_id, 
	poid_t		*stb_poid,
	char		*pairing_device_id_vc, 
	pin_errbuf_t	*ebufp)
{

	pin_flist_t		*w_in_flistp = NULL;
	pin_flist_t		*w_o_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;


	w_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(w_in_flistp, PIN_FLD_POID, device_poid, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(w_in_flistp, PIN_FLD_GROUP_DETAILS, 0, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_NAME, pairing_device_id, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_pair_device write_flds input flist", w_in_flistp);
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, PCM_OPFLG_ADD_ENTRY, w_in_flistp,&w_o_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_pair_device write_flds output flist", w_o_flistp);

	PIN_FLIST_FLD_SET(w_in_flistp, PIN_FLD_POID, stb_poid, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(w_in_flistp, PIN_FLD_GROUP_DETAILS, 0, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_NAME, pairing_device_id_vc, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_pair_device write_flds input flist", w_in_flistp);
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, PCM_OPFLG_ADD_ENTRY, w_in_flistp,&w_o_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_pair_device write_flds output flist", w_o_flistp);
		

	PIN_FLIST_DESTROY_EX(&w_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&w_o_flistp, NULL);


}


int32 fm_mso_depair_device(
	pcm_context_t	*ctxp, 
	poid_t		*device_poid,
	char		*stb_id, 	
	poid_t		*device_poid_stb,
	char		*vc_id, 	
	pin_errbuf_t	*ebufp)
{

	char			*paired_device = NULL;
	char	 		msg[512];	
	int32    		rec_id = 0;
	poid_t                  *srch_pdp = NULL;
	pin_flist_t             *dev_srch_flistp = NULL;
    	pin_flist_t            	*dev_srch_out_flistp = NULL;
	pin_flist_t             *arg_flist = NULL;
	pin_flist_t             *result_flist = NULL;
	pin_flist_t             *result_array = NULL;
	pin_flist_t             *write_iflisp = NULL;
	pin_flist_t             *write_oflisp = NULL;
	pin_flist_t             *dev_ret_flistp = NULL;
	int32                   srch_flag = SRCH_DISTINCT;
	poid_t			*device_pdp = NULL;
	int32			elem_id = 0;
	pin_cookie_t		cookie = NULL;
	pin_flist_t		*group_array_flistp = NULL;
	pin_flist_t		*srch_flistp = NULL;	
	
	
    	if (PIN_ERRBUF_IS_ERR(ebufp))
    	{
        	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_depair_device", ebufp);
		return 0;
    	}
						
        	write_iflisp = PIN_FLIST_CREATE(ebufp);
        	PIN_FLIST_FLD_SET(write_iflisp , PIN_FLD_POID , device_poid , ebufp);
        	group_array_flistp = PIN_FLIST_ELEM_ADD(write_iflisp,PIN_FLD_GROUP_DETAILS,0,ebufp);	
		PIN_FLIST_FLD_SET(group_array_flistp, PIN_FLD_NAME, paired_device, ebufp);
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG ,"fm_mso_depair_device input is " , write_iflisp);
        	PCM_OP(ctxp, PCM_OP_DELETE_FLDS, 0, write_iflisp , &write_oflisp, ebufp);
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG ,"fm_mso_depair_device output is ", write_oflisp);

        	PIN_FLIST_FLD_SET(write_iflisp , PIN_FLD_POID , device_poid_stb , ebufp);
        	group_array_flistp = PIN_FLIST_ELEM_ADD(write_iflisp,PIN_FLD_GROUP_DETAILS,0,ebufp);	
		PIN_FLIST_FLD_SET(group_array_flistp, PIN_FLD_NAME, vc_id, ebufp);
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG ,"fm_mso_depair_device input is " , write_iflisp);
        	PCM_OP(ctxp, PCM_OP_DELETE_FLDS, 0, write_iflisp , &write_oflisp, ebufp);
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG ,"fm_mso_depair_device output is ", write_oflisp);


		return 1;



}

