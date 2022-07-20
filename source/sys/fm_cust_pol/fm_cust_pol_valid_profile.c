
/*****************************************************************************
 *
 *   fm_cust_pol_valid_profile.c
 *
* Copyright (c) 1996, 2012, Oracle and/or its affiliates. All rights reserved. 
 *
 *   This material is the confidential property of Oracle Corporation
 *   or its licensors and may be used, reproduced, stored or transmitted
 *   only in accordance with a valid Oracle license or sublicense  agreement.
 *
 *****************************************************************************/


#ifndef lint
static char Sccs_id[] = "@(#)$Id: fm_cust_pol_valid_profile.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 04:30:45 nishahan Exp $";
#endif

#include <stdio.h> 
#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pinlog.h"
#define SERVICE_PROFILE	"/profile/serv_extrating"

typedef struct {
  time_t from;
  time_t to;
}range_t;

EXPORT_OP void
op_cust_pol_valid_profile(
        cm_nap_connection_t     *connp,
        u_int                   opcode,
        u_int                   flags,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

static u_int
fm_cust_pol_check_for_overlap(
	range_t* 		time_range,
	int32 			count);
 
static u_int
fm_cust_pol_check_range(
	pin_flist_t 		*r_flistp,
	pin_errbuf_t 		*ebufp);

static u_int
check_name_and_value(
	cm_nap_connection_t     *connp,
        pin_flist_t             *in_flistp,
        pin_errbuf_t            *ebufp);

static u_int
check_bal_grp(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             *h_flistp,
        pin_errbuf_t            *ebufp);

static void
fm_cust_pol_valid_profile(
        cm_nap_connection_t	*connp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **out_flistpp,
        pin_errbuf_t            *ebufp);

PIN_IMPORT int32 rollover_transfer_enabled;

/*******************************************************************
* fm_cust_pol_check_for_overlap()
* This checks id range values in the time_range array overlaps or
* not.It returns IN_CUST_VERIFY_FAILED if it overlapped otherwise
* IN_CUST_VERIFY_PASSED.
*******************************************************************/
static u_int
fm_cust_pol_check_for_overlap(
	range_t* time_range,
	int32 count) 
{
	int32	i, j;

	for (i = 0; i < count; i++) {
		for (j = 0; j < count; j++) {
			if ((i != j) && 
			    (time_range[j].to == 0 || 
			     time_range[j].to >= time_range[i].from) &&
			     time_range[j].from <= time_range[i].to) {
				/* range overlapped */
				return PIN_CUST_VERIFY_FAILED;
			}
		}
	}	
	return PIN_CUST_VERIFY_PASSED;
}

/*******************************************************************
* fm_cust_pol_check_range()
* This gathers the ranges (valid_from and valid_to) from all
* recipients and checks if the ranges don't intersect.
*******************************************************************/
static u_int
fm_cust_pol_check_range(
	pin_flist_t *r_flistp,
	pin_errbuf_t *ebufp)
{
	pin_flist_t     *recip_flistp = NULL;
	pin_cookie_t	cookie = NULL;
	u_int           result = PIN_CUST_VERIFY_FAILED;
	int32		elem_id = 0;
	int32           count = 0;
	int32           index = 0;
	range_t         *time_range = NULL;
	time_t          *time = NULL;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return result;
		/************/
	}
        PIN_ERR_CLEAR_ERR(ebufp);

	count = PIN_FLIST_ELEM_COUNT( r_flistp, PIN_FLD_RECIPIENTS, ebufp);

	if (!count) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
			"fm_cust_pol_check_range  count is zero");
		goto cleanup;
		/***********/
	}

	time_range = (range_t *) malloc(count * sizeof(range_t));
	if (!time_range) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
		"fm_cust_pol_check_range: malloc failed");
		goto cleanup;
		/***********/
	}

	while (recip_flistp = PIN_FLIST_ELEM_GET_NEXT(r_flistp,
		PIN_FLD_RECIPIENTS, &elem_id, 1, &cookie, ebufp)) {

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_cust_pol_check_range  recip_flistp", 
			recip_flistp);

		time  = (time_t *)PIN_FLIST_FLD_GET( recip_flistp,
			PIN_FLD_VALID_FROM, 1, ebufp );
		if (!time) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
				"fm_cust_pol_check_range: could not get "
				"VALID_FROM time");
			goto cleanup;
			/***********/
		}
		time_range[index].from = *time;	  

		time = (time_t *) PIN_FLIST_FLD_GET( recip_flistp,
			PIN_FLD_VALID_TO, 1, ebufp );
		if (!time) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
				"fm_cust_pol_check_range: could not get "
				"VALID_TO time");
			goto cleanup;
			/***********/
		}
		time_range[index].to = *time;	  

		/* 
		 * Check the validity of from and to values.
		 * The values of from and to can be 0  
		 * that indicates the infinite boundary values.
		 */
		if (!(time_range[index].from == 0 || 
		      time_range[index].to   == 0) &&
		     (time_range[index].from >= time_range[index].to)) {

			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
				"fm_cust_pol_check_range: invalid valid_to "
				"and valid_from combination encountered: "
				"valid_from is greater than or the same as "
				"valid_to.");
			goto cleanup;	
			/************/
		}

		index++;
	}
	result = fm_cust_pol_check_for_overlap(time_range, index);

 cleanup:
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_check_range error", ebufp);
		result = PIN_CUST_VERIFY_FAILED;
	}
	if (time_range) {
		free(time_range);
	}
	return result;
}

/*****************************************************************************
* check_name_and_value(...)
* This function validates /profile/serv_extrating and /profile/acct_extrating
* objects. It checks PIN_FLD_NAME and PIN_FLD_VALUE of PIN_FLD_DATA_ARRAY
* for NULL values. Sets the validation result as failed if null values found.
* For a /profile/serv_extrating, it checks whether a valid SERVICE_OBJ is
* passed during CREATE_PROFILE. Sets the validation result as failed if the 
* field is either not passed or is a NULL POID.
*****************************************************************************/
static u_int
check_name_and_value(
	cm_nap_connection_t     *connp,
        pin_flist_t             *in_flistp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t     	*inh_flistp = NULL;
	pin_flist_t     	*tmp_flistp = NULL;
	pin_cookie_t		cookie = NULL;
	int32			elemid = 0;
	int32			data_count = 0;
	int32			i = 0;
	char*			namep = NULL;
	char*			valuep = NULL;
	poid_t			*s_pdp = NULL;
        void                    *vp = NULL;
        const char              *poid_type = NULL;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"check_name_and_value in_flistp", in_flistp);

	vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
        if (vp) {
                poid_type = PIN_POID_GET_TYPE((poid_t *)vp);
        }
	if ( fm_utils_op_is_ancestor(connp->coip, PCM_OP_CUST_CREATE_PROFILE)
		&& poid_type && !strncmp(poid_type, SERVICE_PROFILE, strlen(SERVICE_PROFILE)) ) {
		s_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		if (PIN_POID_IS_NULL(s_pdp)) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, 
				"A valid PIN_FLD_SERVICE_OBJ is mandatory for creation of a service level profile");
			return PIN_CUST_VERIFY_FAILED;	
			/****************************/
		}
	}


	inh_flistp = PIN_FLIST_SUBSTR_GET(in_flistp, PIN_FLD_INHERITED_INFO,
			1, ebufp);

	if (inh_flistp) {
		data_count = PIN_FLIST_ELEM_COUNT(inh_flistp, 
				PIN_FLD_DATA_ARRAY, ebufp);
		for (i = 0; i < data_count; i++) {
			tmp_flistp = PIN_FLIST_ELEM_GET_NEXT(inh_flistp,
				PIN_FLD_DATA_ARRAY, &elemid, 1, &cookie, ebufp);

			if (!tmp_flistp) {
				continue;
				/*******/
			}
			namep = (char *) PIN_FLIST_FLD_GET(tmp_flistp, 
					PIN_FLD_NAME, 1, ebufp);
			valuep = (char *) PIN_FLIST_FLD_GET(tmp_flistp, 
					PIN_FLD_VALUE, 1, ebufp);
			if (!namep || !valuep || !strcmp(namep, "") || 
						!strcmp(valuep, "")) {
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, 
					"PIN_FLD_NAME or PIN_FLD_VALUE "
					"can not be NULL.");
				return PIN_CUST_VERIFY_FAILED;
				/****************************/
			}
		}
	}
	return PIN_CUST_VERIFY_PASSED;
}

/*****************************************************************************
* check_bal_grp(...)
* This checks the balgrp of the service against the balgrp of
* its subscription.
*
* Check if the subscription object has been populated in the service object
*   NO: the validation passes
*   YES: compare balgrp of the service with the balgrp of subscription
*      SAME: validation fails  
*      DIFFERENT: validation passes      
*****************************************************************************/
static u_int
check_bal_grp(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             *h_flistp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t     	*t_flistp = NULL;
	pin_flist_t     	*r_flistp = NULL;
	poid_t         		*bp = NULL;
	poid_t          	*sp = NULL;
	poid_t          	*sbp = NULL;
	poid_t          	*svc_pdp = NULL;
	pin_cookie_t    	cookie = NULL;
	int32           	recid = 0;
	

	if (PIN_ERR_IS_ERR(ebufp)) {
		return PIN_CUST_VERIFY_FAILED;
		/****************************/
	}
	PIN_ERR_CLEAR_ERR(ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"check_bal_grp input flist ", in_flistp);

	svc_pdp  = (poid_t *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ,  
				1, ebufp);
	if (!svc_pdp) {
		/* This is not a service profile. Validation passes */
  		return PIN_CUST_VERIFY_PASSED;
		/****************************/
	}

	/* Read the service object to get subscription object */
	t_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_POID, (void *)svc_pdp, ebufp);
	PIN_FLIST_FLD_SET(t_flistp,  PIN_FLD_SUBSCRIPTION_OBJ, NULL, ebufp);

	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, t_flistp, &r_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&t_flistp, NULL);
	
	sp = (poid_t *)PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_SUBSCRIPTION_OBJ,
				0, ebufp);
	if (PIN_POID_IS_NULL(sp)) {
		/*
		 * if there is no subscription then there is
		 * no member service
		 */
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		return PIN_CUST_VERIFY_PASSED;
		/****************************/
	}

	t_flistp =  PIN_FLIST_ELEM_GET_NEXT(h_flistp, PIN_FLD_RESOURCE_INFO, 
				&recid, 0, &cookie, ebufp);
	if(!t_flistp) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
			"validation: check_bal_grp :could not get "
			"RESOURCE_INFO");
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		return PIN_CUST_VERIFY_FAILED;
		/****************************/
	}
	bp = (poid_t *)PIN_FLIST_FLD_GET(t_flistp, PIN_FLD_BAL_GRP_OBJ,
		 		0, ebufp);
	if (PIN_POID_IS_NULL(bp)) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
			"validation: check_bal_grp :could not get bal_grp obj");
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		return PIN_CUST_VERIFY_FAILED;
		/****************************/
	}
	t_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_POID, sp, ebufp);
	PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_BAL_GRP_OBJ, NULL, ebufp);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, t_flistp, &r_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&t_flistp, NULL);
       
	sbp = (poid_t *)PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_BAL_GRP_OBJ,
					 0, ebufp);
	if (!PIN_POID_COMPARE(sbp, bp, 0, ebufp)) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
			"validation: check_bal_grp :PIN_CUST_VERIFY_FAILED ");
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		return PIN_CUST_VERIFY_FAILED;
		/****************************/
	}
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
		"validation: check_bal_grp :PIN_CUST_VERIFY_PASSED ");

	return PIN_CUST_VERIFY_PASSED;
}

/*******************************************************************
* fm_cust_pol_valid_profile()
*
*      Validate the profile info.
*
*******************************************************************/
static void
fm_cust_pol_valid_profile(
        cm_nap_connection_t	*connp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **out_flistpp,
        pin_errbuf_t            *ebufp)
{
	void            	*vp = NULL;
	u_int           	result = PIN_CUST_VERIFY_PASSED;
	pin_cookie_t		cookie = NULL;
	int32			elem_id = 0;
	pin_flist_t     	*r_flistp = NULL;
	pin_flist_t     	*h_flistp = NULL;
	const char		*poid_type = NULL;
	pcm_context_t		*ctxp = connp->dm_ctx;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERR_CLEAR_ERR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		    "fm_cust_pol_valid_profile in_flist", in_flistp);

	/***********************************************************
	* Create outgoing flist
	***********************************************************/

	*out_flistpp = PIN_FLIST_CREATE(ebufp);
 
	vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_POID, vp, ebufp);

	if (vp) {
		poid_type = PIN_POID_GET_TYPE((poid_t *)vp);
	}
	/************************************************************
	 * We only do profile validation if the profile is of type
         * rollover_transfer or serv_extrating or acct_extrating.
	 ************************************************************/
	if (poid_type && 
		(!strncmp(poid_type, "/profile/serv_extrating",
                                strlen("/profile/serv_extrating")) ||
                !strncmp(poid_type, "/profile/acct_extrating",
                                strlen("/profile/acct_extrating"))) ) {

		result = check_name_and_value(connp, in_flistp, ebufp);
		PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_RESULT, &result, ebufp);

		if (result == PIN_CUST_VERIFY_FAILED) {

			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_cust_pol_valid_profile validation failed", ebufp);
		}
		return;
	}

        if (poid_type && strncmp(poid_type, "/profile/rollover_transfer", 
				strlen("/profile/rollover_transfer")) ) {
	  PIN_ERR_CLEAR_ERR(ebufp);
	  PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_RESULT, &result, ebufp);
	  return;
	  /*****/
	}

	/************************************************************
	 * Check if this feature is enabled in the business param.
	 ************************************************************/
	if (!rollover_transfer_enabled) {
	  result = PIN_CUST_VERIFY_FAILED;
	  PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_RESULT, &result, ebufp);
	  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_WARNING,
		"Rollover Transfer is not enabled.");
	  return;
	  /*****/
	}

	h_flistp = PIN_FLIST_SUBSTR_GET(in_flistp,
				PIN_FLD_INHERITED_INFO, 0, ebufp);
	if (h_flistp) {
	  result = check_bal_grp(ctxp,in_flistp, h_flistp, ebufp);
	  if (result ==  PIN_CUST_VERIFY_FAILED) {
	    PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_RESULT, &result, ebufp);
	    return;
	    /*****/
	  }
	  while (r_flistp = PIN_FLIST_ELEM_GET_NEXT( h_flistp, 
		 PIN_FLD_RESOURCE_INFO, &elem_id, 1, &cookie, ebufp)) {

	      result = fm_cust_pol_check_range(r_flistp, ebufp); 
	      if (result == PIN_CUST_VERIFY_FAILED) {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_cust_pol_valid_profile validation failed r_flistp =",
		  r_flistp);
		break;
		/****/
	      }
	    }
	  PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_RESULT, &result, ebufp);
	  /***********************************************************
	   * Error?
	   ***********************************************************/
	  if (PIN_ERR_IS_ERR(ebufp)) {
	    PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		  "fm_cust_pol_valid_profile error", ebufp);
	  }
	}
	else {
	  PIN_ERR_CLEAR_ERR(ebufp);
	  PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_RESULT, &result, ebufp);
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
		"fm_cust_pol_valid_profile returning");
	return;
}

/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_VALID_PROFILE operation.
 *******************************************************************/
void
op_cust_pol_valid_profile(
        cm_nap_connection_t     *connp,
        u_int                   opcode,
        u_int                   flags,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *r_flistp = NULL;
 
        /***********************************************************
         * Null out results until we have some.
         ***********************************************************/
        *ret_flistpp = NULL;
        PIN_ERR_CLEAR_ERR(ebufp);
 
        /***********************************************************
         * Insanity check.
         ***********************************************************/
        if (opcode != PCM_OP_CUST_POL_VALID_PROFILE) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "bad opcode in op_cust_pol_valid_profile", ebufp);
                return;
		/*****/
        }
 
        /***********************************************************
         * Debug: what did we get?
         ***********************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "op_cust_pol_valid_profile input flist", in_flistp);
 
        /***********************************************************
         * Call main function to do it
         ***********************************************************/
        fm_cust_pol_valid_profile(connp, in_flistp, &r_flistp, ebufp);
 
        /***********************************************************
         * Results.
         ***********************************************************/
        if (PIN_ERR_IS_ERR(ebufp)) {
		*ret_flistpp = (pin_flist_t *)NULL;
                PIN_FLIST_DESTROY(r_flistp, NULL);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_cust_pol_valid_profile error", ebufp);
        } else {
                *ret_flistpp = r_flistp;
                PIN_ERR_CLEAR_ERR(ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "op_cust_pol_valid_profile return flist", r_flistp);
        }
 
        return;
}
