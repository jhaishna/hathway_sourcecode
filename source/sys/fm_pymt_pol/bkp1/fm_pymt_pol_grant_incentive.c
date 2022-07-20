/*
 *
 * Copyright (c) 1998, 2009, Oracle and/or its affiliates.All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 *
 */

#ifndef lint
static char Sccs_id[] = "@(#)$Id: fm_pymt_pol_grant_incentive.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 05:45:34 nishahan Exp $";
#endif

#include <stdio.h>
#include <string.h>
#include "pcm.h"
#include "ops/pymt.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_pymt.h"
#include "pin_bill.h"
#include "pin_cust.h"
#include "pinlog.h"
#include "fm_utils.h"

EXPORT_OP void
op_pymt_pol_grant_incentive(
	cm_nap_connection_t	*connp,
	int			opcode,
	int			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

void
fm_pymt_pol_grant_incentive(
	pcm_context_t     	*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t            *ebufp);

void
fm_pymt_pol_grant_incentive_parse_customer_segment(
	char			*cust_segment_listp,
	int			*cust_segment_idp,
	pin_errbuf_t            *ebufp);

/*******************************************************************
 * Main routine for PCM_OP_PYMT_POL_GRANT_INCENTIVE  
 *******************************************************************/
void
op_pymt_pol_grant_incentive(
	cm_nap_connection_t	*connp,
	int			opcode,
	int			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Null out results until we have some.
	 ***********************************************************/
	*ret_flistpp = (pin_flist_t *)NULL;

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_PYMT_POL_GRANT_INCENTIVE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"bad opcode in op_pymt_pol_grant_incentive", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What did we get?
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_pymt_pol_grant_incentive input flist", in_flistp);
	
	fm_pymt_pol_grant_incentive(ctxp, in_flistp, ret_flistpp, ebufp);	

	/***********************************************************
	* Results.
	***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_pymt_pol_grant_incentive error", ebufp);
	} else {
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_pymt_pol_grant_incentive return flist", 
			*ret_flistpp);
	}

	return;
}

/************************************************************************
* fm_pymt_pol_grant_incentive()
* Call fm_pymt_pol_grant_incentive_parse_customer_segment to parse 
* customer segment string to return the appropriate customer segment id
************************************************************************/
void
fm_pymt_pol_grant_incentive(
	pcm_context_t      	*ctxp,
        pin_flist_t             *in_flistp,
	pin_flist_t             **ret_flistpp,
	pin_errbuf_t       	*ebufp)	
{
	char                    *cust_segment_listp = NULL;
	int                     cust_segment_idp = 0 ;
	pin_flist_t             *e_flistp = NULL;
	pin_flist_t             *inc_flistp = NULL;
	poid_t                  *a_pdp = NULL;  

	if (PIN_ERR_IS_ERR(ebufp))
		return;
 	PIN_ERR_CLEAR_ERR(ebufp);

	*ret_flistpp	=  PIN_FLIST_CREATE(ebufp);

	/***********************************************************
	* Get and set PIN_FLD_POID in the output flist
	***********************************************************/
	a_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_POID, (void *)a_pdp, ebufp);
	
	/***********************************************************
	* Get EXTENDED_INFO from the input flist and
	* Set error if not present
	***********************************************************/
	e_flistp = PIN_FLIST_SUBSTR_GET(in_flistp,
				 PIN_FLD_EXTENDED_INFO, 0, ebufp);
	if (e_flistp) {
		/*********************************************
		* Get PIN_FLD_INCENTIVE from EXTENDED_INFO and
		* Set error if not present
		*********************************************/
		inc_flistp = PIN_FLIST_SUBSTR_GET(e_flistp, 
					PIN_FLD_INCENTIVE, 0, ebufp);
		if (inc_flistp) {
		/*********************************************
                * Retrieve the first Customer segment id from
                * PIN_FLD_CUSTOMER_SEGMENT_LIST 
		* and populate the output flist
                *********************************************/
		cust_segment_listp = (char *)PIN_FLIST_FLD_GET(e_flistp, 
				PIN_FLD_CUSTOMER_SEGMENT_LIST, 1, ebufp);

		  /*************************************************
		  * Get CUSTOMER_SEGMENT_LIST from EXTENDED_INFO and
		  * Skip parsing if not present
		  **************************************************/
  		   if(cust_segment_listp && strlen(cust_segment_listp)){
		   fm_pymt_pol_grant_incentive_parse_customer_segment( cust_segment_listp,
							 &cust_segment_idp, ebufp );

 		   PIN_FLIST_FLD_SET(inc_flistp, PIN_FLD_CUSTOMER_SEGMENT, 
    				   (void *)&cust_segment_idp, ebufp);
		   } else {
 		   PIN_FLIST_FLD_SET(inc_flistp, PIN_FLD_CUSTOMER_SEGMENT, 
    				   		NULL, ebufp);
		   }	
		   PIN_FLIST_SUBSTR_SET(*ret_flistpp, e_flistp, 
					PIN_FLD_EXTENDED_INFO, ebufp);
		} else {
		pin_set_err(ebufp,PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE,
                             PIN_ERR_MISSING_ARG, PIN_FLD_INCENTIVE, 0, 0);       
        	}
	} else {
	pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_MISSING_ARG, PIN_FLD_EXTENDED_INFO, 0, 0);        
	}
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"fm_pymt_pol_grant_incentive error", ebufp);
	} else {
		PIN_ERR_CLEAR_ERR(ebufp);
	}
	return;
}

/************************************************************************
 * fm_pymt_pol_grant_incentive_parse_customer_segment()
 * Parse the customer segment string and get the first customer segment ID
 * The values of customer segment ids are delimited by pipe 
 ************************************************************************/
 void fm_pymt_pol_grant_incentive_parse_customer_segment(
        char               *cust_segment_listp,
        int                *cust_segment_idp,
	pin_errbuf_t	   *ebufp)
{

	int			len = 0;
	char			*p = NULL;
	char			*temp = NULL;


	len = strlen(cust_segment_listp);
	p = cust_segment_listp;
	/******************************************************************
	 * Remove the leading blanks
	 ******************************************************************/
	 if (p && len > 0) {
		 while (p[0] == ' ') {
		 p = p + sizeof(char);
		 }
		 temp = p;
	 }
	/**********************************************************
	* Get the first customer segment ID from the
	* customer segment string
	**********************************************************/
	len = 0;
	while (temp && (temp[0] != '|')){
		len++;
		temp = temp + sizeof(char);
	}
	temp = NULL;
	temp = (char *) malloc(sizeof(char) * (len+1));

	if (!temp) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
		PIN_ERRCLASS_SYSTEM_DETERMINATE,
		PIN_ERR_NO_MEM , 0, 0, 0);

		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		    "fm_pymt_pol_grant_incentive_parse_customer_segment opcode error",
									ebufp);
		return;
	}	

	strncpy(temp, p, len);
	/**********************************************
	* Null terminate the string
	**********************************************/
	strcat(temp,"\n");
	*cust_segment_idp = atoi(temp);
	
	free(temp);

	return;
}
