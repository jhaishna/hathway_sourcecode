/* continuus file information --- %full_filespec: fm_cust_pol_valid_acctinfo.c~2:csrc:2 % */
/*******************************************************************
 *
 *      Copyright (c) 2003-2008 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char Sccs_Id[] = "@(#)%Portal Version: fm_cust_pol_valid_acctinfo.c:BillingVelocityInt:4:2006-Sep-05 04:28:17 %";
#endif

#include <stdio.h>
#include <stdlib.h> 
 
#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pin_bill.h"
#include "pinlog.h"
#include "pin_currency.h"
#include "psiu_currency.h"


#define DEFAULT_ELEM_ID		0

/*******************************************************************
 * Routines contained herein.
 *******************************************************************/
EXPORT_OP void
op_cust_pol_valid_acctinfo(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_cust_pol_valid_acctinfo(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * Routines needed from elsewhere.
 *******************************************************************/

extern pin_flist_t *
fm_cust_pol_valid_add_fail(
	pin_flist_t		*r_flistp,
	int32			field,
	int32			elemid,
	int32			result,
	char			*descr,
	void			*val,
	pin_errbuf_t		*ebufp);

extern void
fm_cust_pol_validate_fld_value(
        pcm_context_t   	*ctxp,
        pin_flist_t     	*in_flistp,
        pin_flist_t     	*i_flistp,
        pin_flist_t     	*r_flistp,
        pin_fld_num_t   	pin_fld_field_num,
        int32           	pin_fld_element_id,
        char            	*cfg_name,
        int32           	type,
        pin_errbuf_t    	*ebufp);

extern pin_flist_t *
fm_cust_pol_util_get_config_busness_type_from_cache(
	pin_errbuf_t            *ebufp);

extern pin_flist_t *
fm_cust_pol_util_get_config_customer_segment_from_cache(
	pin_errbuf_t            *ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_VALID_ACCTINFO  command
 *******************************************************************/
void
op_cust_pol_valid_acctinfo(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*r_flistp = NULL;
	
        if (PIN_ERR_IS_ERR(ebufp))
                return;

	/***********************************************************
	 * Null out results until we have some.
	 ***********************************************************/
	*ret_flistpp = NULL;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_CUST_POL_VALID_ACCTINFO) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_cust_pol_valid_acctinfo", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What did we get?
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_cust_pol_valid_acctinfo input flist", in_flistp);

	/***********************************************************
	 * We will not open any transactions with Policy FM
	 * since policies should NEVER modify the database.
	 ***********************************************************/

	/***********************************************************
	 * Call main function to do it
	 ***********************************************************/
	fm_cust_pol_valid_acctinfo(ctxp, in_flistp, &r_flistp, ebufp);

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		*ret_flistpp = (pin_flist_t *)NULL;
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_cust_pol_valid_acctinfo error", ebufp);
	} else {
		*ret_flistpp = r_flistp;
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_valid_acctinfo return flist", r_flistp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_valid_acctinfo()
 *
 *	Validate the given acctinfo.
 *
 *******************************************************************/
static void
fm_cust_pol_valid_acctinfo(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_cookie_t		cookie = NULL;
	pin_flist_t		*flistp = NULL;

	poid_t			*a_pdp = NULL;
	int32			result = 0;
	int32			eid = 0;
	int32			*partial = NULL;
	int32			dummy=0;
	int32			*uip = NULL;
	int32			*uisec_currp = NULL;
	int32			*uiparent_currp = NULL;
	char			*strp = NULL;
	char			*full_strp = NULL;
	pin_flist_t		*c_flistp = NULL;
	void			*vp = NULL;
        int32 			*resultp = NULL;


	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Create outgoing flist
	 ***********************************************************/
	*o_flistpp = PIN_FLIST_CREATE(ebufp);

	/***********************************************************
	 * Get (and add) the account poid.
	 ***********************************************************/
	a_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_POID, (void *)a_pdp, ebufp);

		
	/***********************************************************
	 * For now, assume pass.
	 ***********************************************************/
	result = PIN_CUST_VERIFY_PASSED;
	PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_RESULT, (void *)&result, ebufp);


	/***********************************************************
	 * Get the acctinfo array.
	 ***********************************************************/
	flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_ACCTINFO,
						 &eid, 0, &cookie, ebufp);
	/* 
	 * Get the partial flag
	 */
	partial = PIN_FLIST_FLD_GET(flistp, PIN_FLD_FLAGS, 1, ebufp);
	if (partial == NULL) {
		partial = &dummy;
	}

        /*****************************************************************
	 * Make sure currency is supported
	 *****************************************************************/
	uip = (int32 *)PIN_FLIST_FLD_GET(flistp, PIN_FLD_CURRENCY, 1, ebufp);

	if (uip != (int32 *) NULL) {
		/*
                 * We will do a broad check here basically making sure 
		 * it is within the range Portal defines for currency
		 * balance elements. This can be customized to restrict 
		 * to the currencies defined in the pin.beid file.
                 */
		if (! (*uip >= PIN_CURRENCY_VAL_MIN && 
		       *uip <= PIN_CURRENCY_VAL_MAX)) {

			/* invalid/unsupported currency */
			fm_cust_pol_valid_add_fail(*o_flistpp,
				PIN_FLD_CURRENCY, 
				(int32)NULL,
				PIN_CUST_VAL_ERR_INVALID, 
				(char *)NULL,
				(void *)uip, 
				ebufp);
		} 

	}

	/*
	 * Make sure secondary currency is supported
	 */
	uisec_currp = (int32 *)PIN_FLIST_FLD_GET(flistp, 
			PIN_FLD_CURRENCY_SECONDARY, 1, ebufp);

	/* 
	 * Check to see if secondary curr. is passed, and
	 * check to see if the curr. is a valid sec. curr.
	 */ 
	if (uisec_currp != (int32 *) NULL && uip != (int32 *) NULL) { 
		if (!psiu_currency_is_supported_combination(*uip,
				*uisec_currp, ebufp)) { 
			/* invalid secondary currency */
			fm_cust_pol_valid_add_fail(*o_flistpp,
				PIN_FLD_CURRENCY_SECONDARY, 
				(int32)NULL,
				PIN_CUST_VAL_ERR_INVALID, 
				(char *)NULL,
				(void *)uisec_currp, 
				ebufp);
		}
	}
		
	/*****************************************************************
	 * check for Business Type
         *****************************************************************/
        uip = (int32 *)PIN_FLIST_FLD_GET(flistp, PIN_FLD_BUSINESS_TYPE, 1, 
						ebufp);

        if (uip != (int32 *) NULL) {
                /*
                 * We need to make sure the business type is in the range of 
                 * the config business type and if there is no config in cache
                 *  then make sure business type is 0
                 */
		c_flistp = 
                        fm_cust_pol_util_get_config_busness_type_from_cache( ebufp );

		if ( c_flistp ) {

			vp = PIN_FLIST_ELEM_GET( c_flistp, 
						PIN_FLD_BUSINESS_TYPE_ARRAY, 
						*uip, 1, ebufp );
			if( !vp )
                        	/* invalid/unsupported business type */
                        	fm_cust_pol_valid_add_fail(*o_flistpp,
                                	PIN_FLD_BUSINESS_TYPE,
                               		(int32)NULL,
                                	PIN_CUST_VAL_ERR_INVALID,
                                	(char *)NULL,
                                	(void *)uip,
                                	ebufp);
               	} else {
			if( *uip != 0 ) {
                        	/* invalid/unsupported business type */
                        	fm_cust_pol_valid_add_fail(*o_flistpp,
                                	PIN_FLD_BUSINESS_TYPE,
                               		(int32)NULL,
                                	PIN_CUST_VAL_ERR_INVALID,
                                	(char *)NULL,
                                	(void *)uip,
                                	ebufp);
			}
		}
        }

	/*****************************************************************
	 * check for Customer Segment List
         *****************************************************************/
        strp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_CUSTOMER_SEGMENT_LIST,
						1, ebufp);

        if ( strp ) {
                /**************************************************************
                 * We need to make sure all the customer segments in the list 
		 * of customer segments are in the /config/customer_segment 
		 * object if there is no config in cache then make sure customer
		 * segment is NULL or "0"
                 **************************************************************/
		c_flistp = 
                        fm_cust_pol_util_get_config_customer_segment_from_cache(
					 ebufp );

		if ( c_flistp ) {
			/*****************************************************
			 * I need a pointer to the begining of the string for 
			 * the error if there is one
			 *****************************************************/
			full_strp = strp;

			while ( *strp != '\0' ) {
				eid = atoi(strp);
				vp = PIN_FLIST_ELEM_GET( c_flistp, 
						PIN_FLD_CUSTOMER_SEGMENTS, 
						eid, 1, ebufp );
				if( !vp && eid != DEFAULT_ELEM_ID ) {
					/**************************************
                        		 * invalid/unsupported customer segment
					 **************************************/
                        		fm_cust_pol_valid_add_fail(*o_flistpp,
                                		PIN_FLD_CUSTOMER_SEGMENT_LIST,
                               			(int32)NULL,
                                		PIN_CUST_VAL_ERR_INVALID,
                                		(char *)NULL,
                                		full_strp,
                                		ebufp);
					break;
				}
				
				for(; *strp != '|' && *strp != 0; strp++ );
				if ( *strp == '|' ) {
					strp++;
				}
			}
			PIN_FLIST_DESTROY_EX(&c_flistp, NULL);
               	} else {
			if( strcmp( strp, "0" ) != 0 ) {
				/**************************************
                        	 * invalid/unsupported customer segment
				 **************************************/
                        	fm_cust_pol_valid_add_fail(*o_flistpp,
                                	PIN_FLD_CUSTOMER_SEGMENT_LIST,
                               		(int32)NULL,
                                	PIN_CUST_VAL_ERR_INVALID,
                                	(char *)NULL,
                                	(void *)strp,
                                	ebufp);
			}
		}
        }
        
        resultp = PIN_FLIST_FLD_GET(*o_flistpp, PIN_FLD_RESULT, 1, ebufp);
        if (!resultp) {
                PIN_FLIST_ELEM_DROP(*o_flistpp, PIN_FLD_FIELD, 1, ebufp);
                goto cleanup;
        }
        switch (*resultp) {
        case PIN_CUST_VERIFY_FAILED:
                PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_RESULT, resultp,
                                  ebufp);
                break;
        case PIN_CUST_VERIFY_PASSED:
                break;
        }

 cleanup:
        /*
	 * Error?
	 */
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_valid_acctinfo error", ebufp);
	}

	return;
}
