/*******************************************************************
 *
 * Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char Sccs_Id[] = "@(#)%Portal Version: fm_cust_pol_valid_nameinfo.c:BillingVelocityInt:3:2006-Sep-27 18:01:13 %";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pcm.h"
#include "ops/rate.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pin_rate.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "fm_bill_utils.h"

/***********************************************************************
 *Global Variable for business param
 ***********************************************************************/
PIN_IMPORT  int32	bparam_location_mode_for_taxation;
PIN_IMPORT  int64 provider_tax_count ;
/************************************************************************
 * Forward declarations.
 ************************************************************************/
EXPORT_OP void
op_cust_pol_valid_nameinfo(
        cm_nap_connection_t	*connp,
	int32			opcode,
        int32			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp);


static void 
fm_cust_pol_valid_nameinfo(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistpp,
        pin_errbuf_t		*ebufp);

static void
do_tax_validation(
	pcm_context_t   *ctxp, 
	pin_flist_t	*in_flistp,
	pin_flist_t	**s_flistpp,
	pin_errbuf_t    *ebufp);

/***********************************************************************
 * Routines needed from elsewhere
 ***********************************************************************/
extern int32
fm_bill_init_is_generic_tax_enabled () ;

extern void
fm_validate_existence(
	pin_flist_t	*flistp,
	int32		partialp,
	pin_flist_t	*r_flistp,	
	int32		pin_fld_field_num,	
	int32		pin_fld_element_id,
	int32		*empty_field,		
	pin_errbuf_t	*ebufp);


extern void
fm_validate_format(
	pin_flist_t	*flistp,
	pin_flist_t	*r_flistp,
	int32		pin_fld_field_num,	
	int32		pin_fld_element_id,
	char		*format,		
	char		*errmsg,		
	pin_errbuf_t	*ebufp);

extern void 
fm_validate_value(
	pin_flist_t	*flistp,		
	pin_flist_t	*r_flistp,		
	int32		pin_fld_field_num,	
	int32		pin_fld_element_id,	
	char		*config_file,		
	int		type,			
	pin_errbuf_t	*ebufp);

extern void
fm_cust_pol_validate_fld_value (
	pcm_context_t   *ctxp,
	pin_flist_t     *in_flistp,
	pin_flist_t     *i_flistp,
	pin_flist_t     *r_flistp,
	pin_fld_num_t   pin_fld_field_num,
	int32           pin_fld_element_id,
	char            *cfg_name,
	int             type,
	pin_errbuf_t    *ebufp);
 
extern pin_flist_t *
fm_cust_pol_valid_add_fail(
        pin_flist_t     *r_flistp,
        int32           field,
        int32           elemid,
        int32           result,
        char            *descr,
        void            *val,
        pin_errbuf_t    *ebufp);

extern void
fm_cust_pol_map_country(
        pin_flist_t     *flistp,
        pin_flist_t     *r_flistp,
        char            *country,
        pin_errbuf_t    *ebufp);

extern void
fm_bill_utils_tax_loc(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	*o_flistp,
	pin_errbuf_t	*ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_VALID_NAMEINFO  command
 *******************************************************************/
void
op_cust_pol_valid_nameinfo(
        cm_nap_connection_t	*connp,
	int32			opcode,
        int32			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*r_flistp = NULL;


	/***********************************************************
	 * Null out results until we have some.
	 ***********************************************************/
	*ret_flistpp = NULL;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_CUST_POL_VALID_NAMEINFO) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_cust_pol_valid_nameinfo", ebufp);
		return;
	}

	/***********************************************************
	 * Debug - What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_cust_pol_valid_nameinfo input flist", in_flistp);

	/***********************************************************
	 * We will not open any transactions with Policy FM.  We 
	 * don't need to since policies should NEVER modify the
	 * database
	 ***********************************************************/

	/***********************************************************
	 * Call main function to do it
	 ***********************************************************/
	fm_cust_pol_valid_nameinfo(ctxp, in_flistp, &r_flistp, ebufp);

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		*ret_flistpp = (pin_flist_t *)NULL;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_valid_nameinfo error", ebufp);
	} else {
		*ret_flistpp = r_flistp;
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_valid_nameinfo return flist", r_flistp);
	}

	return;
}


/*******************************************************************
 * fm_cust_pol_valid_nameinfo()
 *
 *     Function to validate an flist that is ready for on-line
 *     registration.
 *
 *******************************************************************/
static void 
fm_cust_pol_valid_nameinfo(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistpp,
        pin_errbuf_t		*ebufp)
{
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	pin_flist_t		*plistp = NULL;
	pin_errbuf_t		errbuf;
	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie2 = NULL;
	pin_cookie_t		cookie3	= NULL;
	poid_t			*poidp = NULL;
	int32			result = 0;
	int32			elemid, elemid2;
	int32			field = 0;
	int32			empty_field = 0;
	int32			count = 0;
	void                    *vp = NULL;
	void            	*valp = NULL;
	/*char			buffer[64];*/
	int32			err = 0;

	char			*country;
	char			*canon_country;
	char			*cfg_obj_name;
 	void            	*geo_vp = NULL;

	int			tax_valid_flag = 0;
	int32			*partialp;
	int32			dummy = 0;
	int			valid_country = 0;
	int32		        addr_used_for_taxation = PIN_BOOLEAN_FALSE;

	if(ebufp == NULL) ebufp = &errbuf;

	/***********************************************************
	 * Null out results until we have some.
	 ***********************************************************/
	*out_flistpp = NULL;

	PIN_ERR_CLEAR_ERR(ebufp);
	empty_field = PIN_BOOLEAN_FALSE;

	/***********************************************************
	 * Create outgoing flist
	 ***********************************************************/
	r_flistp = PIN_FLIST_CREATE(ebufp);

	/***********************************************************
	 * Copy the incoming PIN_FLD_POID to the outgoing list - 
	 * 	mandatory
	 ***********************************************************/
	poidp = (poid_t *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, (void *)poidp, ebufp);

	/***********************************************************
	 * While deleting the nameinfo array, make sure there is 
	 * atleast one nameinfo array left for the account.
	 ***********************************************************/
	flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_NAMEINFO, 
		&elemid, 0, &cookie, ebufp);
	if ((PIN_ERR_IS_ERR(ebufp) == 0) && (flistp == (pin_flist_t *)NULL)) {

		/***************************************************
		 * Looks like we are deleting this nameinfo array.
		 ****************************************************/
		s_flistp = PIN_FLIST_COPY(r_flistp, ebufp);
		PIN_FLIST_ELEM_SET(s_flistp, (pin_flist_t *)NULL, 
			PIN_FLD_NAMEINFO, PIN_ELEMID_ANY, ebufp);

		/****************************************************
	 	 * Read the info from the account.
	 	 ****************************************************/
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, s_flistp, 
			&flistp, ebufp);

		/****************************************************
	 	 * Any contact info left other than this one?
	 	 ****************************************************/
		if (PIN_FLIST_ELEM_COUNT(flistp, PIN_FLD_NAMEINFO, 
			ebufp) < 2) {

			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_BAD_ARG, PIN_FLD_NAMEINFO, 0, 0);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"Can't delete this contact info.", ebufp);

			result = PIN_CUST_VERIFY_FAILED;
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_RESULT, 
				(void *)&result, ebufp);
			*out_flistpp = r_flistp;
			return;
		}

		PIN_FLIST_DESTROY_EX(&flistp, NULL);
		PIN_FLIST_DESTROY_EX(&s_flistp, NULL);

	} else {
		PIN_ERR_CLEAR_ERR(ebufp);
	}

	/***********************************************************
	 * Validate tax jurisdiction if switch in pin.conf is ON
	 **********************************************************/
	pin_conf(FM_CUST_POL, "tax_valid", PIN_FLDT_INT,
			(caddr_t *)&vp, &err);

	switch (err) {
	case PIN_ERR_NONE:
		tax_valid_flag = *((int *)vp);
		free(vp);
		break;

	case PIN_ERR_NOT_FOUND:
		break;

	default:
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE, err, 0, 0, 0);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"Unable to read tax_valid from pin.conf",ebufp);
		break;
	}

	cookie = (pin_cookie_t)NULL;
	/***********************************************************
	 * Walk through the NAMEINFO array and start verifying the 
	 * fields one by one in each NAMEINFO instance.
	 ***********************************************************/
	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_NAMEINFO,
                &elemid, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) {

		addr_used_for_taxation = PIN_BOOLEAN_FALSE;

		/***************************************************
		 *  Extract the partial flag from the input flist
		 ****************************************************/
                partialp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_FLAGS, 1, ebufp);
                if (partialp == NULL) {
                        partialp = &dummy;
                }

		/***************************************************
		 * Create a error sub flist for this Nameinfo instance.
		 * (Assume we'll fail)
		 ****************************************************/
		s_flistp = PIN_FLIST_CREATE(ebufp);
		field = PIN_FLD_NAMEINFO;
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FIELD_NUM,
			(void *)&field, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_ELEMENT_ID,
			(void *)&elemid, ebufp);
		result = PIN_CUST_VERIFY_FAILED;
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_RESULT,
			(void *)&result, ebufp);

		/****************************************************
		 * Mandatory fields - 	PIN_FLD_LAST_NAME.
		 *			PIN_FLD_ADDRESS.
		 *			PIN_FLD_CITY.
		 *			PIN_FLD_COUNTRY.	
		 *
		 * We bail if any call to fm_validate_existence()
		 * fails because that means we got a NULL pointer.
		 * If empty_field is set, one was found.
		 ****************************************************/
		fm_validate_existence(flistp, *partialp, s_flistp, 
			PIN_FLD_LAST_NAME, 
			elemid, &empty_field, ebufp);
		fm_validate_existence(flistp, *partialp, s_flistp, 
			PIN_FLD_ADDRESS, 
			elemid, &empty_field, ebufp);
		fm_validate_existence(flistp, *partialp, s_flistp, 
			PIN_FLD_CITY, 
			elemid, &empty_field, ebufp);
		fm_validate_existence(flistp, *partialp, s_flistp, 
			PIN_FLD_COUNTRY, 
			elemid, &empty_field, ebufp);

		/****************************************************
		 * Get CANON_COUNTRY
		 ****************************************************/
		country = (char *)PIN_FLIST_FLD_GET(flistp,
                                        PIN_FLD_COUNTRY, 1, ebufp);
                if (country != NULL) {
                        fm_cust_pol_map_country(flistp, flistp, country, ebufp);
                        canon_country = (char *) PIN_FLIST_FLD_GET(flistp,
                                        PIN_FLD_CANON_COUNTRY, 1, ebufp);
                        if ( ( canon_country == (char *)NULL) ){
				/***************************************
				 * A country was supplied but did not
				 * map to a canon country so failed
				 * validation.
				 ***************************************/
                                (void)fm_cust_pol_valid_add_fail(s_flistp,
                                        PIN_FLD_COUNTRY, elemid,
                                        PIN_CUST_VAL_ERR_INVALID,
                                        "Country validation failed.",
                                        country, ebufp);
                        }
                        else {
				/***************************************
				 * A country has been provided and been
				 * validated
				 ***************************************/
                                valid_country = 1;
                        }
                }
                else {
			if (!*partialp) {
				/***************************************
				 * ERROR: Country is not provided and 
				 * partial flag is not set
				 ***************************************/
                        	fm_cust_pol_valid_add_fail(s_flistp,
                               		PIN_FLD_COUNTRY, elemid,
                                	PIN_ERR_MISSING_ARG,
                                	"Missing valid country",
                                	country,
                                	ebufp);
			}
			else if (PIN_FLIST_FLD_GET(flistp, PIN_FLD_STATE, 
								1, ebufp) ||
				 PIN_FLIST_FLD_GET(flistp, PIN_FLD_ZIP, 
								1, ebufp) ||
		    		 PIN_FLIST_FLD_GET(flistp, PIN_FLD_SALUTATION, 
								1, ebufp) ||
		    		 PIN_FLIST_ELEM_GET_NEXT(flistp, PIN_FLD_PHONES,					 &elemid2, 1, &cookie3, ebufp))   {
					 fm_cust_pol_valid_add_fail(s_flistp,
                               		PIN_FLD_COUNTRY, elemid,
                                	PIN_ERR_MISSING_ARG,
                                	"Missing valid country",
                                	country,
                                	ebufp);
				}
                }
		if (!valid_country) {
			goto continue_loop;
			/******************/
		}

		/************************************************************
		 * Check the state and zipcode
		 * Get config object name for PIN_FLD_STATE from
		 * /config/locales_validate.
		 ************************************************************/
		if(PIN_FLIST_FLD_GET(flistp, PIN_FLD_STATE, 1, ebufp)){
			cfg_obj_name = fm_utils_get_fld_validate_name( 
					ctxp, canon_country, "PIN_FLD_STATE", 
					ebufp);
			if( cfg_obj_name != (char *)NULL) {
				/*********************************************
				* Try to validate based on /config/fld_validate 
				* (XXX_STATE) obj.
				**********************************************/
				fm_cust_pol_validate_fld_value( ctxp, 
					in_flistp, flistp, s_flistp,
					PIN_FLD_STATE,
					elemid,
					cfg_obj_name,
					0,
					ebufp);
			}
		}


		/************************************************************
		 * Get config object name for PIN_FLD_ZIP from
		 * /config/locales_validate.
		 ************************************************************/
		if ( PIN_FLIST_FLD_GET(flistp, PIN_FLD_ZIP, 1, ebufp)){
			cfg_obj_name = fm_utils_get_fld_validate_name(
					ctxp, canon_country, "PIN_FLD_ZIP", 
					ebufp); 
			if( cfg_obj_name != (char *)NULL) { 
				/*********************************************
			     	* Try to validate based on /config/fld_validate 
			     	* (XXX_ZIP) obj.
			     	***********************************************/
				fm_cust_pol_validate_fld_value( ctxp, in_flistp,
					flistp, s_flistp,
					PIN_FLD_ZIP,
					elemid,
					cfg_obj_name,
					0,
					ebufp);
		    	}
		}

		/************************************************************
		 * Get config object name for PIN_FLD_SALUTATION from
		 * /config/locales_validate.
		 ************************************************************/
		if ( PIN_FLIST_FLD_GET(flistp, PIN_FLD_SALUTATION, 1, ebufp)){
			cfg_obj_name = fm_utils_get_fld_validate_name( 
					ctxp, canon_country, 
					"PIN_FLD_SALUTATION", ebufp);

			if( cfg_obj_name != (char *)NULL) {
			    	/*********************************************
			     	* Try to validate based on /config/fld_validate 
			     	* (XXX_SALUTATION) obj.
			     	*********************************************/
				fm_cust_pol_validate_fld_value( ctxp, 
					in_flistp, flistp, s_flistp,
					PIN_FLD_SALUTATION,
					elemid, cfg_obj_name,
					0, ebufp);
			}
		}

		/************************************************************
		 * Get config object name for PIN_FLD_PHONE from
		 * /config/locales_validate.
		 ************************************************************/
		if ( PIN_FLIST_ELEM_GET(flistp, PIN_FLD_PHONES, PIN_ELEMID_ANY, 
				1, ebufp)){
			cfg_obj_name = fm_utils_get_fld_validate_name( ctxp,
						canon_country, "PIN_FLD_PHONE",
						ebufp);
			if( cfg_obj_name != (char *)NULL) {
			    	/*********************************************
			     	* Check the formats of the phone numbers.
			     	*********************************************/
				cookie2 = (pin_cookie_t)NULL;
				count = PIN_FLIST_ELEM_COUNT(s_flistp, 
							PIN_FLD_FIELD, ebufp);
				while((plistp = 
					(pin_flist_t *)PIN_FLIST_ELEM_GET_NEXT(
						flistp, PIN_FLD_PHONES, 
						&elemid2, 1, &cookie2,
						ebufp))!= (pin_flist_t *)NULL) {
						pin_flist_t	*elistp;

						elistp= PIN_FLIST_CREATE(ebufp);
				/***********************************************
				 * Try to validate based on /config/fld_validate 
				 *********************************************/
					fm_cust_pol_validate_fld_value( ctxp, 
					in_flistp, plistp, elistp, 
					PIN_FLD_PHONE, 0, cfg_obj_name, 0,
					ebufp);
			
					if (PIN_FLIST_ELEM_COUNT(elistp, 
						PIN_FLD_FIELD, ebufp) > 0) {

					/* Field information re: failure */
						field = PIN_FLD_PHONES;
						PIN_FLIST_FLD_SET(elistp, 
							PIN_FLD_FIELD_NUM,
							(void *)&field, ebufp);
						PIN_FLIST_FLD_SET(elistp, 
						PIN_FLD_ELEMENT_ID,
						(void *)&elemid2, ebufp);
						result = PIN_CUST_VERIFY_FAILED;
						PIN_FLIST_FLD_SET(elistp, 
							PIN_FLD_RESULT,
							(void *)&result, ebufp);

						/* Attach the actual flist... */
						PIN_FLIST_ELEM_PUT(s_flistp, 
							elistp, PIN_FLD_FIELD, 
							count++, ebufp);
					} else {
						PIN_FLIST_DESTROY_EX(&elistp, 										NULL);
					}
				}
			}
		}

		/**********************************************************************
		 * Tax validation is to be performed if it is account billing address
		 * or service/product level address used for taxation. The latter is
		 * detected by the presence of /nameinfo as PIN_FLD_POID in NAMEINFO.
		 *********************************************************************/
		poidp = (poid_t *)PIN_FLIST_FLD_GET(flistp, PIN_FLD_POID, 1, ebufp);
		if (fm_utils_type_equal(poidp, PIN_OBJ_TYPE_NAMEINFO) ||
				(elemid == PIN_NAMEINFO_BILLING)) {
			addr_used_for_taxation = PIN_BOOLEAN_TRUE;
		}

		/****************************************************
		 * Get the GEOCODE (if passed in input)
		 ****************************************************/
		geo_vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_GEOCODE, 1, ebufp);

		/************************************************************
		 * Populate the GEOCODE in the return flist so that it 
		 * would be updated in the account.
		 ************************************************************/
		if (geo_vp && strcmp(geo_vp, "") &&
			addr_used_for_taxation &&
			(pin_conf_taxation_switch != PIN_RATE_TAX_SWITCH_NONE)) {

			PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_TAXPKG_TYPE,
				(void*)&tax_valid_flag, ebufp);
			PIN_FLIST_FLD_SET(r_flistp,PIN_FLD_GEOCODE, geo_vp, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_TAXPKG_TYPE,
				(void*)&tax_valid_flag, ebufp);
		}
		/*******************************************************
		 * do tax validation if applicable
		 *******************************************************/
		if (!(tax_valid_flag == PIN_RATE_TAXPKG_GENERIC && 
			!fm_bill_init_is_generic_tax_enabled())) {
			if( tax_valid_flag && addr_used_for_taxation &&
			    (pin_conf_taxation_switch != PIN_RATE_TAX_SWITCH_NONE) ) {

				/* Pass taxPkg to use in the validation */
				PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_TAXPKG_TYPE,
					(void*)&tax_valid_flag, ebufp);
				do_tax_validation(ctxp, in_flistp, &s_flistp, ebufp);
			}
		}

continue_loop:
		/***************************************************
                 * If failures, keep them, otherwise cleanup.
                 ***************************************************/
		if (PIN_FLIST_ELEM_COUNT(s_flistp, PIN_FLD_FIELD, ebufp) > 0)
		{
			PIN_FLIST_ELEM_PUT(r_flistp, s_flistp, PIN_FLD_FIELD, 
				elemid, ebufp);
		} else {
			if (addr_used_for_taxation){
				geo_vp = (char *)PIN_FLIST_FLD_GET(s_flistp,PIN_FLD_GEOCODE,1, ebufp);
				if(geo_vp){
					PIN_FLIST_FLD_SET(r_flistp,PIN_FLD_GEOCODE, (char*)geo_vp, ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_TAXPKG_TYPE,
                                        	(void*)&tax_valid_flag, ebufp);
				}
			}
			PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
		}
		valid_country = 0;
	}

	/***********************************************************
	 * Set the overall result and return.
	 ***********************************************************/
	if (PIN_FLIST_ELEM_COUNT(r_flistp, PIN_FLD_FIELD, ebufp) == 0)
	{
		result = PIN_CUST_VERIFY_PASSED;
	} else {
		result = PIN_CUST_VERIFY_FAILED;
	}
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_RESULT, (void *)&result, ebufp);

	*out_flistpp = r_flistp;

	return;
}

/*************************************************************************
 * do_tax_validation()
 *
 *************************************************************************/
static void
do_tax_validation(
	pcm_context_t	*ctxp,
	pin_flist_t	*in_flistp,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp)
{
	/*pin_flist_t	*flistp;*/
	pin_flist_t	*rr_flistp = NULL;
	pin_flist_t	*t_flistp = NULL;
	pin_flist_t	*rt_flistp = NULL;
	pin_flist_t     *n_flistp = NULL;
	pin_flist_t     *name_flistp = NULL;
	poid_t		*a_pdp = NULL;
	void		*vp = NULL;
	void		*geo_vp = NULL;

	char		city[256];
	char		state[256];
	char		zip[256];
	int		r=0;
	int		n=0;
	char		*p=NULL;
	char		*q=NULL;

	int		*resultp;
	int		elemid = 0;
	int		elemid1 = 0;
	int32		command;
	int		match_found = 0;

	pin_cookie_t            cookie = NULL;


	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/* If validation fails, use element ID of NAMEINFO from original input */
	name_flistp = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_NAMEINFO,
				PIN_ELEMID_ANY, 0, ebufp);
	vp = PIN_FLIST_FLD_GET(name_flistp, PIN_FLD_ELEMENT_ID, 0, ebufp);
	if (vp) {
		elemid = *(int32 *)vp;
	}

	rr_flistp = (pin_flist_t *)PIN_FLIST_CREATE(ebufp);

	/*
	 * Take a copy of the flist and remove ACCOUNT_OBJ if neccessary.
	 * The tax validation code doesn't like receiving a type only
	 * poid because it tries to read stuff from the db based on it. So
	 * if its type only, we simply removed it 
	 */
	rt_flistp = PIN_FLIST_COPY(in_flistp, ebufp);
	a_pdp = PIN_FLIST_FLD_GET(rt_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	if (a_pdp && PIN_POID_IS_TYPE_ONLY(a_pdp)) {
		PIN_FLIST_FLD_DROP(rt_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	}
	
	fm_bill_utils_tax_loc(ctxp, rt_flistp, rr_flistp, ebufp);

	PIN_FLIST_DESTROY_EX(&rt_flistp, NULL);
	
	if (!PIN_FLIST_FLD_GET(rr_flistp, PIN_FLD_ORDER_ACCEPT, 1, ebufp) ||
            !PIN_FLIST_FLD_GET(rr_flistp, PIN_FLD_ORDER_ORIGIN, 1, ebufp) ||
            !PIN_FLIST_FLD_GET(rr_flistp, PIN_FLD_SHIP_FROM, 1, ebufp) ||
            !PIN_FLIST_FLD_GET(rr_flistp, PIN_FLD_SHIP_TO, 1, ebufp)) {

		goto tax_valid_fail;
		/******************/
	}

	/***********************************************************
	 * check it here to avoid future errors ...
	 ***********************************************************/
	vp = PIN_FLIST_FLD_GET(rr_flistp, PIN_FLD_ORDER_ORIGIN, 0, ebufp);
	if (vp != (void *)NULL) {
		if( (p = strchr((char *)vp, ';')) ) {
			for( r=2, p++; r; r--) {
				if( !(q = strchr(p, ';'))) break;
				p = q+1;
			}
                	if (r) {
				goto tax_valid_fail;
				/******************/
			}
		} else {
			r = sscanf((char *)vp, "%[^,], %s %s%n",
					 city, state, zip, &n);

			if (r < 3) {
				goto tax_valid_fail;
				/******************/
			}
		}
	}

	vp = PIN_FLIST_FLD_GET(rr_flistp, PIN_FLD_ORDER_ACCEPT, 0, ebufp);
	if (vp != (void *)NULL) {
		if( (p = strchr((char *)vp, ';')) ) {
			for( r=2, p++; r; r--) {
				if( !(q = strchr(p, ';'))) break;
				p = q+1;
			}
                	if (r) {
				goto tax_valid_fail;
				/******************/
			}
		} else {
			r = sscanf((char *)vp, "%[^,], %s %s%n",
					 city, state, zip, &n);
			if (r < 3) {
				goto tax_valid_fail;
				/******************/
			}
		}
	}
	vp = PIN_FLIST_FLD_GET(rr_flistp, PIN_FLD_SHIP_TO, 0, ebufp);
	if (vp != (void *)NULL) {
		if( (p=strchr((char *)vp, ';')) ) {
			for( r=2, p++; r; r--) {
				if( !(q = strchr(p, ';'))) break;
				p = q+1;
			}
                	if (r) {
				goto tax_valid_fail;
				/******************/
			}
		} else {
			r = sscanf((char *)vp, "%[^,], %s %s%n",
						 city, state, zip, &n);
			if (r < 3) {
				goto tax_valid_fail;
				/*******************/
			}
		}
	}
	vp = PIN_FLIST_FLD_GET(rr_flistp, PIN_FLD_SHIP_FROM, 0, ebufp);
	if (vp != (void *)NULL) {
		if( (p=strchr((char *)vp, ';')) ) {
			for( r=2, p++; r; r--) {
				if( !(q = strchr(p, ';'))) break;
				p = q+1;
			}
                	if (r) {
				goto tax_valid_fail;
				/******************/
			}
		} else {
			r = sscanf((char *)vp, "%[^,], %s %s%n",
						 city, state, zip, &n);
			if (r < 3) {
				goto tax_valid_fail;
				/*******************/
			}
		}
	}

	vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_TAXPKG_TYPE, 1, ebufp);
	PIN_FLIST_FLD_SET(rr_flistp, PIN_FLD_TAXPKG_TYPE, vp, ebufp);

	command = 1;
	PIN_FLIST_FLD_SET(rr_flistp, PIN_FLD_COMMAND, (void *)&command, ebufp);

	t_flistp = (pin_flist_t *)NULL;

	PCM_OP(ctxp, PCM_OP_RATE_TAX_CALC, 0, rr_flistp, &t_flistp, ebufp);

	if( t_flistp == (pin_flist_t *)NULL) {
		goto tax_valid_fail;
		/*******************/
	}

	resultp = (int *)PIN_FLIST_FLD_GET( t_flistp, PIN_FLD_RESULT, 1, ebufp);

	if( resultp == (int *)NULL) {
		PIN_FLIST_DESTROY_EX(&t_flistp, NULL);
		goto tax_valid_fail;
		/*******************/
	}

	r = *resultp;

	if( !r ) {
                goto tax_valid_fail;
                /*******************/
        }

	/* In case of GeoCode based Taxation geocode returned by third party
	 * software needs to be captured here.In case subscriber has given any
	 * geocode, it will be compared with the geocodes returned
	 */
	if (bparam_location_mode_for_taxation == PIN_RATE_LOC_GEOCODE){

		if(name_flistp != NULL){
			geo_vp = PIN_FLIST_FLD_GET(name_flistp, PIN_FLD_GEOCODE, 1, ebufp);
		}

		cookie = NULL;
		elemid1 = 0;
		while ((n_flistp = PIN_FLIST_ELEM_GET_NEXT(t_flistp, PIN_FLD_NAMEINFO,
                	&elemid1, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) {
			vp = (char *)PIN_FLIST_FLD_GET(n_flistp, PIN_FLD_GEOCODE, 1, ebufp);
			if(vp != NULL  && geo_vp == NULL){
				PIN_FLIST_FLD_SET(*r_flistpp,PIN_FLD_GEOCODE,(char*) vp,ebufp);
				match_found = 1;
				break;	
			}else if(vp != NULL && geo_vp != NULL){
				if(strcmp((char*)vp,(char *)geo_vp) == 0){	
					PIN_FLIST_FLD_SET(*r_flistpp,PIN_FLD_GEOCODE,(char*) vp,ebufp);
					match_found = 1;
					break;
				}
			}
		}

		if(!match_found){
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
                                 "No Matching Geocode is found");
			goto tax_valid_fail;
                	/*******************/
		}        	
	}

	PIN_FLIST_DESTROY_EX(&t_flistp, NULL);


	PIN_FLIST_DESTROY_EX(&rr_flistp, NULL);
	return;
	/*****/
	
tax_valid_fail:

	if( t_flistp != (pin_flist_t *)NULL) {
                PIN_FLIST_DESTROY_EX(&t_flistp, NULL);
        }


	if( rr_flistp != (pin_flist_t *)NULL) {
		PIN_FLIST_DESTROY_EX(&rr_flistp, NULL);
	}
	(void)fm_cust_pol_valid_add_fail(*r_flistpp,
		PIN_FLD_NAMEINFO, elemid,
		PIN_CUST_VAL_ERR_INVALID,
		"tax (jurisdiction) validation failed.", (void *)NULL,
		ebufp);
	return;
}
