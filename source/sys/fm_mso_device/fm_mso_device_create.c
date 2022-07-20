/*
 * (#)$Id: fm_mso_device_create.c  $
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
* 0.1    |03/03/2015 | Harish Kulkarni       | Device Create custom opcode
* 0.2    |28/04/2015 | Sisir Samanta         | Added mac address validation
* 0.3    |12/12/2019 | Jyothirmayi Kachiraju | Added code to skip MAC validation for ONU (GPON) modem for JIO 
											   and Hathway Networks
*--------------------------------------------------------------------------------------------------------------*
****************************************************************************************************************/

#ifndef lint
static const char Sccs_id[] = "(#)$Id: fm_mso_device_create.c st_cgbubrm_gsivakum_bug-8453030/1 2009/05/05 23:37:48 gsivakum Exp $";
#endif

#include <stdio.h>
#include <string.h>
#include <pcm.h>
#include <pinlog.h>
#include <regex.h>
#include "ops/device.h"
#include "pin_device.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "fm_utils.h"
#include "pin_cust.h"
#include "mso_device.h"
#include "mso_prov.h"
#include "mso_ops_flds.h"

#define FILE_LOGNAME "fm_mso_device_create.c"
#define  ERROR_RECORD     2
#define  SUCCESS_RECORD     0

// Functions contained within this source file

EXPORT_OP void
op_mso_device_create(
	cm_nap_connection_t    *connp,
	int32            opcode,
	int32            flags,
	pin_flist_t        *i_flistp,
	pin_flist_t        **o_flistpp,
	pin_errbuf_t        *ebufp);

static void fm_mso_device_create(
	pcm_context_t   *ctxp,
	pin_flist_t     *task_flistp,
	pin_flist_t     **ret_flistp,
	pin_errbuf_t    *ebufp);

int fm_mso_device_match_patterns(
	char* pattern_string,
	 char* input_string
	 );


/*******************************************************************************
 * Main function which does the device Set State:
 ******************************************************************************/

void
op_mso_device_create(
	cm_nap_connection_t    *connp,
	int32            opcode,
	int32            flags,
	pin_flist_t        *i_flistp,
	pin_flist_t        **o_flistpp,
	pin_errbuf_t        *ebufp)
{
	pcm_context_t        *ctxp = connp->dm_ctx;

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*
	 * Insanity check.
	 */
	if (opcode != MSO_OP_DEVICE_CREATE)
	{

		pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_BAD_OPCODE, 0, 0, opcode);

		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"bad opcode in op_mso_device_create", ebufp);

		*o_flistpp = NULL;
		return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_mso_device_create input flist", i_flistp);

	// Call the function to do the Set State:
	fm_mso_device_create(ctxp, i_flistp, o_flistpp, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"op_mso_device_create error", ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,
		"op_mso_device_create error - input flist", i_flistp);

		*o_flistpp = NULL;
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"op_mso_device_create return flist",
				*o_flistpp);
	}

	return;

}

static void fm_mso_device_create(
	pcm_context_t   *ctxp,
	pin_flist_t     *task_flistp,
	pin_flist_t     **ret_flistp,
	pin_errbuf_t    *ebufp)
{
	pin_flist_t     *r_flistp = NULL;
	pin_flist_t     *err_flistp = NULL;
	pin_flist_t     *acnt_info = NULL;
	int32           status_success = 0;
	int32           status_fail = 1;
	char                    *error_descr = NULL;
	char                    *error_code = NULL;
	char                    err_code[20];
	pin_err_location_t      err_location;
	pin_err_class_t         err_class;
	pin_err_t               pin_err;
	pin_fld_num_t           err_field;
	pin_rec_id_t            err_rec_id;
	pin_err_reserved_t      err_reserved;
	poid_t			*device_poid	= NULL;
	poid_t			*acc_pdp = NULL;
	char			*device_id = NULL;
	char			*device_poid_type = NULL;
	char			*device_type = NULL;
	char			*sourcep = NULL;
	int32                   new_status = SUCCESS_RECORD;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , "fm_mso_device_create input flist is " , task_flistp);
	device_poid = (poid_t *)PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_POID,0,ebufp);
	device_poid_type =(char *) PIN_POID_GET_TYPE(device_poid);
	device_id = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_ID,0,ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,device_poid_type);
	device_type = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_TYPE,0,ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,device_type);

	acc_pdp = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	if (acc_pdp == NULL)
	{
		sourcep = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_SOURCE, 0, ebufp);
		fm_mso_get_acnt_from_acntno(ctxp, sourcep, &acnt_info, ebufp);
		if (acnt_info)
		{
			acc_pdp = PIN_FLIST_FLD_TAKE(acnt_info, PIN_FLD_POID, 0, ebufp);
			PIN_FLIST_FLD_PUT(task_flistp, PIN_FLD_ACCOUNT_OBJ, acc_pdp, ebufp);
			PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
		}
	}


	// Changes for JIO-CPS Integration-ISP
	if ((device_poid_type)&& ((strcmp(device_poid_type,"/device/modem") == 0) || (strcmp(device_poid_type,"/device/nat") == 0)
		|| (strcmp(device_poid_type,"/device/router/cable") == 0)|| (strcmp(device_poid_type,"/device/router/wifi") == 0)))
	{
		//Skip MAC validation for ONU (GPON) modem for JIO and Hathway Networks and validate the device_id pattern for other device types.		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Skipping Device ID Validation for GPON devices ****** >>");
		
		if( ( device_type && 
			  !( strcmp(device_type, HW_GPON_MODEM) == 0 || 
				 strcmp(device_type, HW_ONU_GPON_MODEM) == 0 || 
				 strcmp(device_type, JIO_GPON_MODEM) == 0 ) ) && 
		    fm_mso_device_match_patterns("^([0-9a-fA-F]{2})(([/\\s:-][0-9a-fA-F]{2}){5})$", device_id) <=0 )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error in MAC address validation"); 
			err_flistp     = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_POID, err_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(err_flistp,PIN_FLD_STATUS, &status_fail,ebufp);
			PIN_FLIST_FLD_SET(err_flistp,PIN_FLD_ERROR_DESCR,"MAC address validation failed",ebufp);
			PIN_FLIST_FLD_SET(err_flistp,PIN_FLD_ERROR_CODE,"51039", ebufp);
			*ret_flistp = err_flistp;
			goto CLEANUP;
		}		
	}

	PCM_OP(ctxp,PCM_OP_DEVICE_CREATE, 0, task_flistp, &r_flistp , ebufp);
	
	/*******************************************
	*     Update the status of the object
	*******************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		err_location = ebufp->location;
		err_class = ebufp->pin_errclass;
		pin_err = ebufp->pin_err;
		err_field = ebufp->field;
		err_rec_id = ebufp->rec_id;

		sprintf(err_code,"%d",pin_err);
		PIN_ERRBUF_CLEAR (ebufp);
		new_status = ERROR_RECORD;
	}
	if(new_status == ERROR_RECORD)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error Occured in Device Creation"); 
		err_flistp     = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_POID, err_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(err_flistp,PIN_FLD_STATUS, &status_fail,ebufp);

		if(strcmp(err_code,"10") == 0)
		{
			PIN_FLIST_FLD_SET(err_flistp,PIN_FLD_ERROR_DESCR,"Duplicate Device ID",ebufp);
			PIN_FLIST_FLD_SET(err_flistp,PIN_FLD_ERROR_CODE,"51040", ebufp);
		}
		else if (strcmp(err_code,"46") == 0)
		{
			PIN_FLIST_FLD_SET(err_flistp,PIN_FLD_ERROR_DESCR," Bad Device id value " , ebufp);
			PIN_FLIST_FLD_SET(err_flistp,PIN_FLD_ERROR_CODE,err_code, ebufp);
		}
		else if (strcmp(err_code,"4") == 0)
		{
			PIN_FLIST_FLD_SET(err_flistp,PIN_FLD_ERROR_DESCR," Bad value passed " , ebufp);
			PIN_FLIST_FLD_SET(err_flistp,PIN_FLD_ERROR_CODE,err_code, ebufp);
		}
		else if(strcmp(err_code , "50") == 0)
		{
			PIN_FLIST_FLD_SET(err_flistp,PIN_FLD_ERROR_DESCR," Connection Error " , ebufp);
			PIN_FLIST_FLD_SET(err_flistp,PIN_FLD_ERROR_CODE,err_code, ebufp);
		}
		else if(error_descr || err_code)
		{
			PIN_FLIST_FLD_SET(err_flistp,PIN_FLD_ERROR_DESCR,"Failed in Execution", ebufp);
			PIN_FLIST_FLD_SET(err_flistp,PIN_FLD_ERROR_CODE, error_code, ebufp);
		}

		*ret_flistp = err_flistp;
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , "fm_mso_device_create output flist is " , *ret_flistp);
		goto CLEANUP;
	}

	PIN_FLIST_FLD_SET(r_flistp,PIN_FLD_STATUS, &status_success,ebufp);
	*ret_flistp = r_flistp;
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , "fm_mso_device_create output flist is " , *ret_flistp);

	CLEANUP:

	return;

}


//============================================================================
//
// FUNTION NAME     : fm_mso_integ_match_patterns
//
// DESCRIPTION      : Perform a regular expression match on a string against
//                    a regular expression
//
// INPUT PARAMETERS :
//      pattern_string              : string containing the regular expression
//      input_string                : the string possibly containing the pattern
//
// RETURN VALUE     :
//      0 - no match
//      1 - match
//     -1 - error
//
//
//============================================================================
int
fm_mso_device_match_patterns(char* pattern_string, char* input_string)
{
    regex_t *regex;
    regmatch_t *result;
    int return_val = 0;

    /* Make space for the regular expression */

    if ((regex = (regex_t *) malloc(sizeof(regex_t))) == NULL)
    {
        fprintf(stdout, "No more memory - aaaagh! (Die kicking and screaming.)\n");
        return -1;
    }
    else
    {
        memset(regex, 0, sizeof(regex_t));
    }

    if ((regex != NULL) && (regcomp(regex, pattern_string, REG_EXTENDED) == 0))
    {
        if((result = (regmatch_t *) malloc(sizeof(regmatch_t)))==0)
        {
            fprintf(stdout, "No more memory - aaaagh! (Die kicking and screaming.)\n");
            return_val = -1;
        }

        if (regexec(regex, input_string, 1, result, 0)==0) // found a match
        {
            if (strlen(input_string) == result->rm_eo) return_val = 1;
            else 0;
        }
        else
        {
            return_val = 0;
        }
    }

    if (regex != NULL)
    {
        regfree(regex); /* Free the regular expression data structure */
        free(regex);
    }
    if (result != NULL)
    {
        free(result);
    }

    return return_val;
} // fm_mso_op_match_patterns

