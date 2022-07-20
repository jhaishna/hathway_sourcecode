/*
 * @(#)%Portal Version: fm_device_pol_create.c:ServerIDCVelocityInt:4:2006-Sep-14 11:34:45 %
 *
 * Copyright (c) 2001 - 2006 Oracle. All rights reserved.
 *
 * This material is the confidential property of Oracle Corporation
 * or its licensors and may be used, reproduced, stored or transmitted
 * only in accordance with a valid Oracle license or sublicense agreement.
 *
 */

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_device_pol_create.c:ServerIDCVelocityInt:4:2006-Sep-14 11:34:45 %";
#endif

/*******************************************************************
 * Contains the PCM_OP_DEVICE_POL_CREATE operation.
 *******************************************************************/

#include <stdio.h>
#include <string.h>

#include <pcm.h>
#include <pinlog.h>

#define FILE_LOGNAME "fm_device_pol_create.c(1)"

#include "ops/device.h"
#include "ops/sim.h"
#include "ops/num.h"
#include "ops/bel.h"
#include "ops/voucher.h"
#include "ops/apn.h"
#include "ops/ip.h"
#include "pin_device.h"
#include "pin_sim.h"
#include "pin_num.h"
#include "pin_voucher.h"
#include "pin_apn.h"
#include "pin_ip.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_channel.h"
#include "fm_utils.h"
#include "mso_device.h"            //Custom Header file
#include "mso_ops_flds.h"

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_device_pol_create(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_device_pol_create(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_device_pol_lifecycle_create(
	pcm_context_t        *ctxp,
	pin_flist_t        *i_flistp,
	pin_errbuf_t        *ebufp);

void
mso_device_pol_device_id_exists(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

static int 
isValidMacAddress(
        const char      *stb_id) ;

/*******************************************************************
 * Routines needed from elsewhere.
 *******************************************************************/

/*******************************************************************
 * Main routine for the PCM_OP_DEVICE_POL_CREATE operation.
 *******************************************************************/
void
op_device_pol_create(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t	*r_flistp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Null out results
	 ***********************************************************/
	*r_flistpp = NULL;

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_DEVICE_POL_CREATE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_device_pol_create opcode error", ebufp);

		return;
	}
	
	/***********************************************************
	 * Debut what we got.
         ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_device_pol_create input flist", i_flistp);

	/***********************************************************
	 * Main rountine for this opcode
	 ***********************************************************/
	fm_device_pol_create(ctxp, flags, i_flistp, r_flistpp, ebufp);
	
	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_FLIST_DESTROY_EX(r_flistpp, ebufp);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_device_pol_create error", ebufp);
	} else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_device_pol_create output flist", *r_flistpp);
	}

	return;
}

/*******************************************************************
 * fm_device_pol_create:
 *
 *******************************************************************/
static void
fm_device_pol_create(
	pcm_context_t           *ctxp,
	int32                   flags,
	pin_flist_t             *i_flistp,
	pin_flist_t             **r_flistpp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t	*acnt_info = NULL;
    poid_t          *dev_poidp = NULL;
	poid_t		*acc_pdp = NULL;
    char            *poid_type = NULL;
    char            *stb_id = NULL;
    char            *vc_id = NULL;
    char            *network_node = NULL;
    char		*sourcep = NULL;
    int32           len;
    int32           i;
    time_t           offset_month=0;
    time_t          waranty_end_date=0;
    time_t           ship_date=0;
    time_t           *ship=NULL;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERR_CLEAR_ERR(ebufp);
	
	/*
	 * Get the device poid
	 */
	dev_poidp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
		acc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	if (acc_pdp == NULL)
	{
		sourcep = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SOURCE, 0, ebufp);
		fm_mso_get_acnt_from_acntno(ctxp, sourcep, &acnt_info, ebufp);
		if (acnt_info)
		{
			acc_pdp = PIN_FLIST_FLD_TAKE(acnt_info, PIN_FLD_POID, 0, ebufp);
			PIN_FLIST_FLD_PUT(i_flistp, PIN_FLD_ACCOUNT_OBJ, acc_pdp, ebufp);
			PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
		}
	}
 
      /*cahnges for Warranty-Start Date */
      PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Input Flist Coming", i_flistp);

      ship = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_CR_ADJ_DATE, 0, ebufp);

         if(ship == (time_t *)NULL)
               {
                       PIN_ERRBUF_CLEAR(ebufp);
               }
       else
       {
           /*PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST NIM - 1");*/
           PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE,i_flistp,PIN_FLD_START_T, ebufp);
           offset_month = *(time_t *)PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_WARRANTY_END_OFFSET, 0, ebufp);
           PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST - 1");
           ship_date = *(time_t *)PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_CR_ADJ_DATE, 0, ebufp);


      /* PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST - 2");*/
       waranty_end_date=offset_month/12*365*24*60*60+ship_date;
       PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST - 3");
       PIN_FLIST_FLD_SET(i_flistp, MSO_FLD_WARRANTY_END, &waranty_end_date, ebufp);
        }
     /*cahnges for Warranty-Start Date End here*/
	
	/*
	 * Get the device poid type
	 */
	poid_type = (char *) PIN_POID_GET_TYPE(dev_poidp);
	
	    if ( strcmp(poid_type, MSO_OBJ_TYPE_DEVICE_STB)    == 0 ||
         strcmp(poid_type, MSO_OBJ_TYPE_DEVICE_VC )    == 0 )
    {
        /**************************************************
        * Field level validation can be impelmented here
        ***************************************************/
        if (strcmp(poid_type, MSO_OBJ_TYPE_DEVICE_STB)    == 0)
        {
            stb_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEVICE_ID, 0, ebufp);
            network_node = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_MANUFACTURER, 0, ebufp);
            len = strlen(stb_id);
            if ( strstr(network_node, "NDS") )
            {
                if (len > 16) 
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"length greater than 16 digit stb id");

                    pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_VALUE, PIN_FLD_DEVICE_ID, 0, 0);
                    return;
                }

                /*while (*stb_id) 
                {
                    if (*stb_id > 47 && *stb_id < 58) 
                    {
                        i++;
                    }
                    else
                    {
                         PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"non digit stb id");
                        pin_set_err(ebufp, PIN_ERRLOC_FM,
                            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                            PIN_ERR_BAD_VALUE, PIN_FLD_DEVICE_ID, 0, 0);
                        return;
                    }
                    ++stb_id;
                }*/
           }
	/*	
            else if (strcmp(network_node,"CISCO") == 0 )
            {
                if (!isValidMacAddress(stb_id))
                {
                         PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"invalid mac address stb id");
                    pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_VALUE, PIN_FLD_DEVICE_ID, 0, 0);
                    return;
                }

            }
          */ 
        }

        if (strcmp(poid_type, MSO_OBJ_TYPE_DEVICE_VC)    == 0)
        {
            vc_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEVICE_ID, 0, ebufp);
            network_node = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_MANUFACTURER, 0, ebufp);
            len = strlen(vc_id);
            if ( strstr(network_node, "NDS") )
            {
                if (len > 12 )
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"length greater than 12 digit VC id");
                    pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_VALUE, PIN_FLD_DEVICE_ID, 0, 0);
                    return;
                }

                /*while (*vc_id) 
                {
                    if (*vc_id > 47 && *vc_id < 58) 
                    {
                        i++;
                    }
                    else
                    {
                         PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"non digit vc id");
                        pin_set_err(ebufp, PIN_ERRLOC_FM,
                            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                            PIN_ERR_BAD_VALUE, PIN_FLD_DEVICE_ID, 0, 0);
                        return;
                    }
                      ++vc_id;
                }*/
            }
		else if ( strstr(network_node, "GOSPELL") )
		{
                	if (len != 16 )
                	{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"length of VC id != 16");
				pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_BAD_VALUE, PIN_FLD_DEVICE_ID, 0, 0);
				return;
                	}
		}

            else 
            {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                    PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    PIN_ERR_BAD_VALUE, PIN_FLD_DEVICE_ID, 0, 0);
                return;
            }
        }

            /*check Uniquenesss */
	/*** commented because this function is avoiding DM error ***/
         mso_device_pol_device_id_exists (ctxp, i_flistp, r_flistpp, ebufp);
    }
	else
	{
		if (strcmp(poid_type, PIN_OBJ_TYPE_DEVICE_SIM) == 0){
			/*
			 * The device type is SIM
			 * Call the SIM device create policy code
			 */
			 PCM_OP(ctxp, PCM_OP_SIM_POL_DEVICE_CREATE, flags, i_flistp, 
				r_flistpp, ebufp);
		} 	
		else if (strcmp(poid_type, PIN_OBJ_TYPE_DEVICE_NUM) == 0){
			/*
			 * The device type is NUM
			 * Call the NUM device create policy code
			 */
			 PCM_OP(ctxp, PCM_OP_NUM_POL_DEVICE_CREATE, flags, i_flistp, 
				r_flistpp, ebufp);
		}
		else if (strcmp(poid_type, PIN_OBJ_TYPE_DEVICE_VOUCHER) == 0){
			/*
			 * The device type is VOUCHER
			 * Call the VOUCHER device create policy code
			 */
			 PCM_OP(ctxp, PCM_OP_VOUCHER_POL_DEVICE_CREATE, flags, i_flistp, 
				r_flistpp, ebufp);
		}
		else if (strcmp(poid_type, PIN_OBJ_TYPE_DEVICE_APN) == 0){
			/*
			 * The device type is APN
			 * Call the APN device create policy code
			 */
			 PCM_OP(ctxp, PCM_OP_APN_POL_DEVICE_CREATE, flags, i_flistp, 
				r_flistpp, ebufp);
		}
		else if (strcmp(poid_type, PIN_OBJ_TYPE_DEVICE_IP) == 0){
			/*
			 * The device type is IP
			 * Call the IP device create policy code
			 */
			 PCM_OP(ctxp, PCM_OP_IP_POL_DEVICE_CREATE, flags, i_flistp, 
				r_flistpp, ebufp);
		}

		else{
			*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
		}
	}

	fm_device_pol_lifecycle_create(ctxp, i_flistp, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_device_pol_create error", ebufp);
	}
	return;
}

/*******************************************************************************
 * mso_device_pol_device_id_exists()
 *
 * Inputs: flist with device_id and poid of a /device
 * Outputs: void; ebuf set if device_id already exists
 *
 * Description:
 * This function searches the database for an existing object with the same
 * device_id as the input argument.  If one or more records are found, the
 * ebuf is set indicating the device_id already exists.
 ******************************************************************************/
void
mso_device_pol_device_id_exists(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
    pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
        int32                   cred = 0;               /* used for scopeing */
        int32                   s_flags = 0;            /* for searching */
        int64                   database = 0;           /* route the search */
        pin_flist_t             *search_flistp = NULL;  /* search input */
        pin_flist_t             *args_flistp = NULL;    /* search args */
        pin_flist_t             *r_flistp = NULL;       /* search output */
        poid_t                  *srchpp = NULL;         /* for routing */
        poid_t                  *dn_poidp = NULL;       /* for searching */
        char                    *template = NULL;       /* search template */
        char                    *poid_type = NULL;      /* for searches */
        void                    *vp = NULL;             /* for flist gets */
        int32            elemid = 0;        /* for results check */
         pin_cookie_t            cookie = NULL;        /* for results check */
        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                return;
        }
        PIN_ERRBUF_CLEAR(ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "mso_device_pol_device_id_exists input flist", i_flistp);
        /*
         * Setup search flist for uniqueness check
         */
        search_flistp = PIN_FLIST_CREATE(ebufp);
        vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
        database = PIN_POID_GET_DB((poid_t *)vp);
        poid_type = (char *)PIN_POID_GET_TYPE((poid_t *)vp);
        srchpp = PIN_POID_CREATE(database, "/search", -1, ebufp);
        PIN_FLIST_FLD_PUT(search_flistp, PIN_FLD_POID, 
        (void *)srchpp, ebufp);
        PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
        template = "select X from /device where F1 = V1 and F2 = V2 and F3 = V3 ";
        PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_TEMPLATE, 
        (void *)template, ebufp);
        /*
         * Add the first argument to the flist - the device_id
         */
        args_flistp = PIN_FLIST_ELEM_ADD(search_flistp,
                PIN_FLD_ARGS, 1, ebufp);
        /* Make sure the number is canonicalized before adding it */

        vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEVICE_ID, 0, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DEVICE_ID, vp, ebufp);
        /*
         * Add the second argument to the flist - the poid type
         */
        args_flistp = PIN_FLIST_ELEM_ADD(search_flistp,
                PIN_FLD_ARGS, 2, ebufp);
        dn_poidp = PIN_POID_CREATE(database, poid_type, -1, ebufp);
        PIN_FLIST_FLD_PUT(args_flistp, PIN_FLD_POID, 
        (void *)dn_poidp, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(search_flistp,
                PIN_FLD_ARGS, 3, ebufp);
        /* Make sure the number is canonicalized before adding it */

        vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_MANUFACTURER, 0, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_MANUFACTURER, vp, ebufp);
        /*
         * Add the null results array
         */
        PIN_FLIST_ELEM_SET(search_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp);
        /*
         * Perform the search (system-wide)
         */
        cred = CM_FM_BEGIN_OVERRIDE_SCOPE(CM_CRED_SCOPE_OVERRIDE_ROOT);
        PCM_OP(ctxp, PCM_OP_GLOBAL_SEARCH, PCM_OPFLG_COUNT_ONLY, search_flistp,
                &r_flistp, ebufp);
        CM_FM_END_OVERRIDE_SCOPE(cred);
        /*
         * If /device with given device_id already exists, set ebuf
         */
        PIN_FLIST_ELEM_GET_NEXT(r_flistp, PIN_FLD_RESULTS, &elemid, 0, 
            &cookie, ebufp);
        if (elemid > 0) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_DUPLICATE, PIN_FLD_DEVICE_ID, 0, 0);
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
                        "mso_device_pol_device_id_exists new device_id "
                        "already exists");
        }
        else {
            *r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
        }
        PIN_FLIST_DESTROY_EX(&search_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
        return;
}

int 
isValidMacAddress(const char* stb_id) 
{
    int i = 0;
    int s = 0;

    while (*stb_id) 
    {
       if (*stb_id > 47 && *stb_id < 58 ||   /* 0-9 */
           *stb_id > 96 && *stb_id < 103 ||  /* a-f */
           *stb_id > 64 && *stb_id < 69 )    /* A-F */
       {
          i++;
       }
       else if (*stb_id == ':' ) 
       {
          if (i == 0 || i / 2 - 1 != s)
            break;
          ++s;
       }
       else 
       {
           s = -1;
       }
       ++stb_id;
    }

    return (i == 12 && (s == 5 || s == 0));
}

static void
fm_device_pol_lifecycle_create(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_errbuf_t            *ebufp)
{
	poid_t		*pdp=NULL;
	poid_t		*poid_pdp=NULL;
	poid_t		*lifecycle_pdp=NULL;
	poid_t		*account_pdp=NULL;
	int64		database=1;
	int64		calling_opcode=2700;
	int32		action_mode=1;
	char		*action = "CREATED";
	pin_flist_t	*set_lifecycle_flistp=NULL;
	pin_flist_t	*r_flistp = NULL;
	int32		flag = CREATE;
        time_t           *shipment=NULL;



	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_device_pol_create lifecycle error at begining", ebufp);
		return;
	}
	
	PIN_ERRBUF_CLEAR(ebufp);


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"MSO_OP_LIFECYCLE_DEVICE input flist", i_flistp);

	pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	database = PIN_POID_GET_DB(pdp);  

	/*****************************************************************************
		0 PIN_FLD_POID                POID [0] 0.0.0.1 /mso_lifecycle/device -1 0
		0 PIN_FLD_ACCOUNT_OBJ         POID [0] 0.0.0.1 /account 1 0
		0 PIN_FLD_ACTION               STR [0] "Device loading"
		0 PIN_FLD_DESCR                STR [0] "Device Create: 0.0.0.1 /device/vc 995189 0"
		0 PIN_FLD_PROGRAM_NAME         STR [0] "VC Device loading"
		0 PIN_FLD_SERVICE_OBJ         POID [0] 0.0.0.1 /service/catv 218076
		0 PIN_FLD_DEVICE_ID		STR [0] “1123425565”
		0 PIN_FLD_DEVICE_OBJ		POID [0] 0.0.0.0 /device/modem 11 1
		0 PIN_FLD_STATE_ID		INT [0] 1
	********************************************************************************/
	
	set_lifecycle_flistp = PIN_FLIST_CREATE(ebufp);
	lifecycle_pdp = (poid_t *)PIN_POID_CREATE(database, "/mso_lifecycle/device", -1, ebufp);
	PIN_FLIST_FLD_PUT(set_lifecycle_flistp, PIN_FLD_POID, (void *)lifecycle_pdp, ebufp);
	PIN_FLIST_FLD_SET(set_lifecycle_flistp, PIN_FLD_ACTION,action, ebufp);
	PIN_FLIST_FLD_SET(set_lifecycle_flistp, PIN_FLD_FLAGS, (void *)&flag, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME,set_lifecycle_flistp,PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ,set_lifecycle_flistp,PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ,set_lifecycle_flistp,PIN_FLD_ACCOUNT_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DEVICE_ID,set_lifecycle_flistp,PIN_FLD_DEVICE_ID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID,set_lifecycle_flistp,PIN_FLD_DEVICE_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_STATE_ID,set_lifecycle_flistp,PIN_FLD_STATE_ID, ebufp);
       /*Inventory Changes*/
        shipment = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_CR_ADJ_DATE, 0, ebufp);
         if(shipment == (time_t *)NULL)
       {
         PIN_ERRBUF_CLEAR(ebufp);
       }
       else
       {
       PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CR_ADJ_DATE,set_lifecycle_flistp,PIN_FLD_START_T, ebufp);
         }
    /*Inventory Changes End*/

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"MSO_OP_LIFECYCLE_DEVICE input flist",set_lifecycle_flistp);
	
	PCM_OP(ctxp, MSO_OP_LIFECYCLE_DEVICE, flag, set_lifecycle_flistp, &r_flistp, ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Back in DEVICE POL");
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"MSO_OP_LIFECYCLE_DEVICE output flist", r_flistp);	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		//PIN_FLIST_DESTROY_EX(&r_flistp, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"End of Inside error buffer");
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"op_device_pol_create lifecycle error", ebufp);
		goto cleanup;
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"op_device_pol_create lifecycle output flist", r_flistp);
	}
cleanup:
	PIN_FLIST_DESTROY_EX(&set_lifecycle_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
    return;
}
