/*******************************************************************
 *
* Copyright (c) 2001, 2014, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char Sccs_Id[] = "@(#)$Id: fm_cust_pol_valid_login.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 04:30:36 nishahan Exp $";

#endif

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
 
#include "pcm.h"
#include "ops/cust.h"
#include "ops/subscription.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "fm_utils.h"
#include "pin_cust.h"
#include "pinlog.h"
#include "pin_msexchange.h"

#define FM_CUST_POL_MAX_LOGIN	255

/*******************************************************************
 * These should go in a header file.
 *******************************************************************/
#define FM_CUST_POL             "fm_cust_pol"
#define FM_NAUGHTYFILE_TOKEN     "naughty_file"
#define FM_DEFAULT_NAUGHTYFILE  "./naughty_file"
#define EXPAND_FROMFILE         0
#define VALIDATE_INFILE         1
#define VALIDATE_NOTINFILE      2
#define MAXLINE                 80

/*******************************************************************
 * Routines contained herein.
 *******************************************************************/
EXPORT_OP void
op_cust_pol_valid_login(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_cust_pol_valid_login(
	cm_nap_connection_t	*connp,
	pin_flist_t		*i_flistp,
	int32			flags,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

void
fm_cust_pol_valid_login_check_uniquiness_table(
	pcm_context_t		*ctxp,
	int64			db_no,
	int			msexchange_service_type,
	pin_flist_t		*i_flistp,
	pin_flist_t		*r_flistp,
	pin_errbuf_t		*ebufp);

void
fm_cust_pol_valid_login_check_uniquiness_service(
	pcm_context_t		*ctxp,
	int64			db_no,
	int			msexchange_service_type,
	pin_flist_t		*i_flistp,
	pin_flist_t		*r_flistp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * Routines needed from elsewhere.
 *******************************************************************/
extern void
fm_cust_pol_validate_fld_value (
	pcm_context_t   *ctxp,
	pin_flist_t     *in_flistp,
	pin_flist_t     *i_flistp,
	pin_flist_t     *r_flistp,
	pin_fld_num_t   pin_fld_field_num,
	u_int           pin_fld_element_id,
	char            *cfg_name,
	int             type,
	pin_errbuf_t    *ebufp);


extern pin_flist_t *
fm_cust_pol_valid_add_fail(
	pin_flist_t	*r_flistp,
	u_int		field,
	u_int		elemid,
	u_int		result,
	char		*descr,
	void		*val,
	pin_errbuf_t	*ebufp);

extern void
fm_cust_pol_gen_gsm_login(char   *generated_login,
			int32  size);
extern void 
pcm_set_given_poid_db_number(pcm_context_t *ctxp);

extern int32 
pcm_is_given_poid_db_number_set(pcm_context_t *ctxp);
/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_VALID_LOGIN  command
 *******************************************************************/
void
op_cust_pol_valid_login(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*r_flistp = NULL;

	/***********************************************************
	 * Null out results until we have some.
	 ***********************************************************/
	*ret_flistpp = NULL;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_CUST_POL_VALID_LOGIN) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_cust_pol_valid_login", ebufp);
		return;
	}

	/***********************************************************
	 * We will not open any transactions with Policy FM
	 * since policies should NEVER modify the database.
	 ***********************************************************/

	/***********************************************************
	 * Log input flist
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			  "op_cust_pol_valid_login input flist", in_flistp);

	/***********************************************************
	 * Call main function to do it 
	 ***********************************************************/
	fm_cust_pol_valid_login(connp, in_flistp, flags, &r_flistp, ebufp);

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		*ret_flistpp = (pin_flist_t *)NULL;
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_cust_pol_valid_login error", ebufp);
	} else {
		*ret_flistpp = r_flistp;
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_valid_login return flist", r_flistp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_valid_login()
 *
 *	Validate the given login according to the given poid type.
 *
 *	For now, logins are required (ie they can't be NULL) and
 *	must be less than the maximum length (currently 255).
 *
 *******************************************************************/
static void
fm_cust_pol_valid_login(cm_nap_connection_t	*connp,
			pin_flist_t		*i_flistp,
			int32			flags,
			pin_flist_t		**o_flistpp,
			pin_errbuf_t		*ebufp)
{
	pin_cookie_t		cookie = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*a_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	poid_t			*o_pdp = NULL;
	char			*login = NULL;
	char			*namep = NULL;
	void                    *vp = NULL;
	pcm_context_t		*ctxp = connp->dm_ctx;
	
	const char		*subtype = NULL;
	int32			result = PIN_CUST_VERIFY_PASSED;
	int32			elemid;
	int32			not_vc_flag = 1;
	int64			db_no;
	int			count = 0;
	int 			msexchange_service_type = 
	                                  PIN_MSEXCHANGE_SERVICE_NONE;



	/***********************************************************
         * Return immediately if there's an error in ebufp.
         ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Create outgoing flist
	 ***********************************************************/
	r_flistp = PIN_FLIST_CREATE(ebufp);
	*o_flistpp = r_flistp;

	/***********************************************************
	 * Get (and add) the object poid.
	 ***********************************************************/
	o_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, (void *)o_pdp, ebufp);


	/***********************************************************
	 * Get the input login.
	 ***********************************************************/
	login = (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_LOGIN, 1, ebufp);
	
	/* 
	 *  LOGIN could be NULL if SET_LOGIN is called to set an alias entry
	 *  during SERVICE creation
	 */
	if (login != NULL) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Validating Login");
		count =  PIN_FLIST_ELEM_COUNT(i_flistp, PIN_FLD_ALIAS_LIST, 
					      ebufp);
		/***************************************************
		 * Try to validate based on /config/fld_validate 
		 * (Login) obj, if it is a new service with alias
		 * list or modifying a login with no alias list
		 **************************************************/
		//if ((count < 1) || (fm_utils_op_is_ancestor(
		if ((fm_utils_op_is_ancestor(
			connp->coip, PCM_OP_CUST_CREATE_SERVICE))) {
			fm_cust_pol_validate_fld_value( ctxp, i_flistp, 
							i_flistp, r_flistp, 
							PIN_FLD_LOGIN, 
							0, "Login", 1, ebufp);
		}
	}

	/***********************************************************
         * Try to validate all of the alias list
         ***********************************************************/
	a_flistp = NULL;
	for ( 
		flistp = PIN_FLIST_ELEM_GET_NEXT( i_flistp, PIN_FLD_ALIAS_LIST,
						&elemid, 0, &cookie, ebufp );
		ebufp->pin_err == PIN_ERR_NONE;
		flistp = PIN_FLIST_ELEM_GET_NEXT( i_flistp, PIN_FLD_ALIAS_LIST,
						&elemid, 0, &cookie, ebufp )
	) {
		/***********************************************************
      	 	 * Prep the return flist
       		 ***********************************************************/
		if ( !a_flistp ) {
			a_flistp = PIN_FLIST_CREATE(ebufp);
		}

		if ( !flistp ) continue;

		/***********************************************************
      	 	 * Try to validate based on /config/fld_validate 
		 * (name) obj.  but since we cannot validate a value in
		 * PIN_FLD_NAME due to the facte we have to pass PIN_FLD_NAME
		 * as the name of the validation rule.  I will cady the 
		 * the PIN_FLD_NAME in a PIN_FLD_LOGIN and then manipulate 
		 * the error output if there is one.
       		 ***********************************************************/
		namep = PIN_FLIST_FLD_GET( flistp, PIN_FLD_NAME, 0, ebufp );
		if (strncmp(namep, "00", 2) == 0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "VC not required duplicate validation");
			not_vc_flag = 0;
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Validating service alia NAME");

			PIN_FLIST_FLD_SET( flistp, PIN_FLD_LOGIN, namep, ebufp );
		fm_cust_pol_validate_fld_value( ctxp, i_flistp, flistp, 
			a_flistp, PIN_FLD_LOGIN, 0, "Login", 1, ebufp);
		PIN_FLIST_FLD_DROP( flistp, PIN_FLD_LOGIN, ebufp );

		if (PIN_FLIST_ELEM_COUNT(a_flistp, PIN_FLD_FIELD, ebufp) != 0) {
			/***************************************************
			 * There is a validation error so normalize it to
			 * the input flist
			 ***************************************************/
			pin_fld_num_t field = PIN_FLD_ALIAS_LIST;
			PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_FIELD_NUM,
					(void *)&field, ebufp);
			PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_ELEMENT_ID,
					(void *)&elemid, ebufp);
			result = PIN_CUST_VERIFY_FAILED;
			PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_RESULT,
					(void *)&result, ebufp);
			count = PIN_FLIST_ELEM_COUNT(r_flistp, PIN_FLD_FIELD, 
					ebufp);
			PIN_FLIST_ELEM_PUT(r_flistp, a_flistp, PIN_FLD_FIELD,
					count, ebufp);
			a_flistp = PIN_FLIST_ELEM_GET( a_flistp, PIN_FLD_FIELD, 
					PIN_ELEMID_ANY, 0, ebufp );
			field = PIN_FLD_NAME;
			PIN_FLIST_FLD_SET( a_flistp, PIN_FLD_FIELD_NUM, 
					(void *)&field, ebufp );
			a_flistp = NULL;
		}
		}

	}
	if ( ebufp->pin_err == PIN_ERR_NOT_FOUND ) {
		PIN_ERR_CLEAR_ERR( ebufp );
	}

	/***************************************************************
	 * clean up any last unused a_flistp
	 ***************************************************************/
	PIN_FLIST_DESTROY_EX( &a_flistp, NULL );

	if (PIN_FLIST_ELEM_COUNT(r_flistp, PIN_FLD_FIELD, ebufp) != 0) {
               	result = PIN_CUST_VERIFY_FAILED;
		goto Done;
	}
	
	/***************************************************************
	 * Get the poid type to check service type. Used for E2K service
	 ***************************************************************/
	db_no = PIN_POID_GET_DB(o_pdp);
	subtype = PIN_POID_GET_TYPE(o_pdp);
	if ( subtype && 
		(strcmp(subtype, PIN_MSEXCHANGE_SERVICE_TYPE_USER) == 0)){
		msexchange_service_type = PIN_MSEXCHANGE_SERVICE_USER;
	}
	if ( subtype && 
		(strcmp(subtype, PIN_MSEXCHANGE_SERVICE_TYPE_FIRSTADMIN) == 0)){
		msexchange_service_type = PIN_MSEXCHANGE_SERVICE_FIRSTADMIN;
	}

	// In case the service is getting transfer then by pass the checks 
	// for /service/gsm and /service/telco and subtypes
	if(!fm_utils_op_is_ancestor(
                        connp->coip, PCM_OP_SUBSCRIPTION_TRANSFER_SUBSCRIPTION)){
		/***********************************************************
		 * Reject login change for /service/gsm type and subtype
		 ***********************************************************/
		if (subtype &&
			(!(strncmp(subtype, "/service/gsm", 12)))) {
			/***************************************************
			 * if user try to change the login and it's not allowed
			 ***************************************************/
			char * login = NULL;
			login = (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_LOGIN,
				1, ebufp);
			if (login == NULL) {
				goto Done;
			} else {
				if (!(fm_utils_op_is_ancestor(connp->coip,
					PCM_OP_CUST_COMMIT_CUSTOMER))&&
				!(fm_utils_op_is_ancestor(connp->coip,
					PCM_OP_CUST_MODIFY_CUSTOMER))&&
				!(flags & PCM_OPFLG_CUST_REGISTRATION)) {
					(void)fm_cust_pol_valid_add_fail(r_flistp,
					PIN_FLD_LOGIN, (u_int)NULL,
					PIN_CUST_VAL_ERR_REJECT,
					PIN_CUST_REJECT_VALUE_ERR_MSG,
					(void *)login, ebufp);
					result = PIN_CUST_VERIFY_FAILED;
					goto Done;
				} 
			}
		}

		/***********************************************************
		* Reject login change for /service/telco type and subtype
		***********************************************************/
	        if (subtype &&
        	    ((!(strncmp(subtype, "/service/telco/", 15))) ||
                	(!(strcmp(subtype, "/service/telco"))))) {
	                /***************************************************
        	         * if user try to change the login and it's not allowed
                	 ***************************************************/
        /*	        char * login = NULL;
	                login = (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_LOGIN,
                	                                  1, ebufp);
			if (login == NULL) {
				goto Done;
			} else {
				if (!(fm_utils_op_is_ancestor(connp->coip,
						     PCM_OP_CUST_COMMIT_CUSTOMER))&&
				!(fm_utils_op_is_ancestor(connp->coip,
						     PCM_OP_CUST_MODIFY_CUSTOMER))&&
				!(flags & PCM_OPFLG_CUST_REGISTRATION)) {
					(void)fm_cust_pol_valid_add_fail(r_flistp,
					PIN_FLD_LOGIN, (u_int)NULL,
					PIN_CUST_VAL_ERR_REJECT,
					PIN_CUST_REJECT_VALUE_ERR_MSG,
					(void *)login, ebufp);
					result = PIN_CUST_VERIFY_FAILED;
					goto Done;
				}
        	        } */
	        }  
	}
	/***********************************************************
	 * If we're still ok, check uniqueness, in case of multi_db 
	 * and single-schema TT env.
	 * In case of multi-db approach and  single-schema TT env, we 
	 * need to check in the uniqueness table, otherwise proceed 
	 * as before.
	 * We need to use both the service type and the login.
         ***********************************************************/
	if (cm_fm_is_multi_db() || cm_fm_is_timesten()) {

		int64		primary_db_no = 0;
		int64		trans_db_no = 0;
		pcm_context_t	*vctxp = NULL;
		poid_t		*trans_poidp = NULL;
		pin_flist_t	*trans_flistp = NULL;

		/***************************************************
 	 	 * Put in all the fields we need to insert the
 	 	 *  entry into the uniqueness table
 	 	 ***************************************************/
		cm_fm_get_primary_db_no(&primary_db_no, ebufp);

                trans_flistp = pcm_get_trans_flist(ctxp);
                if ( trans_flistp ) {
			trans_poidp = PIN_FLIST_FLD_GET(trans_flistp, 
							PIN_FLD_POID, 0, ebufp);
			trans_db_no = PIN_POID_GET_DB( trans_poidp );
                } else {
			trans_db_no = primary_db_no;
		}
		PIN_FLIST_DESTROY_EX( &trans_flistp, NULL );

		/* 
		 *if we are looking at a different db then we currently have a 
		 * transaction in then we need a new context 
		 */

		// in case of globaltransaction don't open new context
		//if(!pcm_is_given_poid_db_number_set(ctxp)){
		if(!(connp->coip->opflags & PCM_TRANS_OPEN_GLOBALTRANSACTION)){
			if ( primary_db_no != trans_db_no ) {
				PCM_CONTEXT_OPEN(&vctxp, (pin_flist_t *)0, ebufp);
			} else {
				vctxp = ctxp;
			}

			fm_cust_pol_valid_login_check_uniquiness_table( vctxp,
        				primary_db_no, 
					msexchange_service_type, 
					i_flistp, r_flistp, ebufp );

			if ( primary_db_no != trans_db_no ) {
				PCM_CONTEXT_CLOSE(vctxp, 0, NULL);
			}
		}else{
                        fm_cust_pol_valid_login_check_uniquiness_table( ctxp,
                                        primary_db_no,
                                        msexchange_service_type,
                                        i_flistp, r_flistp, ebufp );
		}
		
	} else {

		/*********************************************************
		 * This happens only for a single db installation
		 * where we don't have to search in the uniqueness table.
		 ********************************************************/
		if (not_vc_flag)
		{
			fm_cust_pol_valid_login_check_uniquiness_service( ctxp,
					db_no, msexchange_service_type, 
					i_flistp, r_flistp, ebufp );
		}
	}

	if (PIN_FLIST_ELEM_COUNT(r_flistp, PIN_FLD_FIELD, ebufp) != 0) {
               	result = PIN_CUST_VERIFY_FAILED;
		goto Done;
	}

Done:
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&a_flistp, NULL);
	/***********************************************************
         * Set the overall result and return.
         ***********************************************************/
        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_RESULT, (void *)&result, ebufp);

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_valid_login error", ebufp);
	}
	return;
}

void
fm_cust_pol_valid_login_check_uniquiness_table(
	pcm_context_t		*ctxp,
	int64			db_no,
	int			msexchange_service_type,
	pin_flist_t		*i_flistp,
	pin_flist_t		*r_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*a_flistp = NULL;
	pin_flist_t		*l_flistp = NULL;
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	poid_t			*objp = NULL;
	poid_t			*s_pdp = NULL;
	void			*vp = NULL;
	const char		*subtype = NULL;
	char			*login = NULL;
	int32			result = PIN_CUST_VERIFY_PASSED;
	int32			elemid = 0;
	int32			elemid2 = 0;
	int32			search_flag = SRCH_DISTINCT;
	int64			curr_db = 0;
	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie2 = NULL;
	int32			cred = 0;
	int			count = 0;

        /***********************************************************
         * Return immediately if there's an error in ebufp.
         ***********************************************************/
        if (PIN_ERR_IS_ERR(ebufp))
                return;
        PIN_ERR_CLEAR_ERR(ebufp);


	s_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	/*********************************************************
	 * Create the search flist
	 ********************************************************/
	s_flistp = PIN_FLIST_CREATE(ebufp);
	curr_db = pcm_get_current_db_no(ctxp);
	objp = PIN_POID_CREATE(curr_db, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)objp, ebufp);

	/******************************************************************
	 * Set the search template.
	 ******************************************************************/

	/**********************************************************************
	 * If Service type is /service/msexchange/user or 
	 * /service/msexchange/user/firstadmin
	 * search login name in both /service/msexchange/user and 
	 * /service/msexchange/user/firstadmin objects
	 **********************************************************************/
	if( msexchange_service_type != PIN_MSEXCHANGE_SERVICE_NONE ){
		vp = (void *)"select X from /uniqueness where F1 = V1"
				"  AND( F2 = V2 OR F3 = V3 ) ";
	}
	else {
		vp = (void *)"select X from /uniqueness where F1 = V1 and "
				"F2 = V2 ";
	}

	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, vp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &search_flag, ebufp);

	/* Add the search criteria. */
	login = PIN_FLIST_FLD_GET( i_flistp, PIN_FLD_LOGIN, 1, ebufp );
	
	/* Look for PIN_FLD_LOG = login */
	l_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(l_flistp, PIN_FLD_LOGIN, (void *)login, ebufp);

	/**********************************************************************
	 * If service is /service/msexchange/user type, search login name also
	 * in /service/msexchange/user/firstadmin for unique login name and 
	 * visa versa.
	 * Add F3 and V3 Parameters in search template with service type
	 **********************************************************************/
	if( msexchange_service_type == PIN_MSEXCHANGE_SERVICE_USER ||
		msexchange_service_type == PIN_MSEXCHANGE_SERVICE_FIRSTADMIN ){
		a_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
		objp = PIN_POID_CREATE(db_no, 
					PIN_MSEXCHANGE_SERVICE_TYPE_FIRSTADMIN,
	       				(int64)-1, ebufp);
		PIN_FLIST_FLD_PUT(a_flistp, PIN_FLD_POID, (void *)objp, ebufp);

		a_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
		objp = PIN_POID_CREATE(db_no, PIN_MSEXCHANGE_SERVICE_TYPE_USER,
	       		(int64)-1, ebufp);
		PIN_FLIST_FLD_PUT(a_flistp, PIN_FLD_POID, (void *)objp, ebufp);
	} else {
		subtype = PIN_POID_GET_TYPE( s_pdp );
		a_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
		objp = PIN_POID_CREATE(db_no, subtype, (int64)-1, ebufp);
		PIN_FLIST_FLD_PUT(a_flistp, PIN_FLD_SERVICE_OBJ, (void *)objp, 
				ebufp);
	}
	a_flistp = NULL;

	PIN_FLIST_ELEM_PUT(s_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp);

	/************************************************************
	 * we only search if there was a login
	 * otherwise this was all setup for the alias list searches.
	 ************************************************************/
	if ( login ) {
		cred = CM_FM_BEGIN_OVERRIDE_SCOPE(CM_CRED_SCOPE_OVERRIDE_ROOT);
		PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_COUNT_ONLY,
			s_flistp, &a_flistp, ebufp);
		CM_FM_END_OVERRIDE_SCOPE(cred);

		(void)PIN_FLIST_ELEM_GET_NEXT(a_flistp, PIN_FLD_RESULTS, 
			&elemid, 0, &cookie, ebufp);	

		if (elemid > (int32)0) {
			(void)fm_cust_pol_valid_add_fail(r_flistp, 
				PIN_FLD_LOGIN, (u_int)NULL,
				PIN_CUST_VAL_ERR_DUPE,
				PIN_CUST_DUPE_VALUE_ERR_MSG, 
				(void *)login, ebufp);
               		result = PIN_CUST_VERIFY_FAILED;
		}
	}

	/***********************************************************
         * Try to validate all of the alias list
         ***********************************************************/
	PIN_FLIST_DESTROY_EX( &a_flistp, NULL );
	cookie = NULL;
	for ( 
		flistp = PIN_FLIST_ELEM_GET_NEXT( i_flistp, PIN_FLD_ALIAS_LIST,
						&elemid, 0, &cookie, ebufp );
		ebufp->pin_err == PIN_ERR_NONE;
		flistp = PIN_FLIST_ELEM_GET_NEXT( i_flistp, PIN_FLD_ALIAS_LIST,
                                                &elemid, 0, &cookie, ebufp )
	) {

		if ( !flistp ) continue;

		login = PIN_FLIST_FLD_GET( flistp, PIN_FLD_NAME, 0, ebufp );
	
		/*************************************************************
		 * the search flist is still set up we just need to replace
		 * the login value with an alias value.
		 *************************************************************/
		PIN_FLIST_FLD_SET(l_flistp, PIN_FLD_LOGIN, (void *)login, ebufp);

		cred = CM_FM_BEGIN_OVERRIDE_SCOPE(CM_CRED_SCOPE_OVERRIDE_ROOT);
		PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_COUNT_ONLY,
			s_flistp, &a_flistp, ebufp);
		CM_FM_END_OVERRIDE_SCOPE(cred);

		cookie2 = NULL;
		vp = PIN_FLIST_ELEM_GET_NEXT(a_flistp, PIN_FLD_RESULTS, 
			&elemid2, 0, &cookie2, ebufp);	

		PIN_FLIST_DESTROY_EX( &a_flistp, NULL );

		if (elemid2 > (int32)0) {
			pin_fld_num_t field = PIN_FLD_ALIAS_LIST;
			/***************************************************
			 * There is a validation error so normalize it to
			 * the input flist
			 ***************************************************/
			result = PIN_CUST_VERIFY_FAILED;
			count = PIN_FLIST_ELEM_COUNT(r_flistp, PIN_FLD_FIELD, 
					ebufp);
			a_flistp = PIN_FLIST_ELEM_ADD(r_flistp, PIN_FLD_FIELD,
					count, ebufp);
			PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_FIELD_NUM,
					(void *)&field, ebufp);
			PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_ELEMENT_ID,
					(void *)&elemid2, ebufp);
			PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_RESULT,
					(void *)&result, ebufp);
			(void)fm_cust_pol_valid_add_fail(a_flistp, 
				PIN_FLD_NAME, (u_int)NULL,
				PIN_CUST_VAL_ERR_DUPE,
				PIN_CUST_DUPE_VALUE_ERR_MSG, 
				(void *)login, ebufp);
			a_flistp = NULL;
		}
	}
	if ( ebufp->pin_err == PIN_ERR_NOT_FOUND ) {
		PIN_ERR_CLEAR_ERR( ebufp );
	}


	if ( result == PIN_CUST_VERIFY_FAILED ) {
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_RESULT,
				(void *)&result, ebufp);
	}
	PIN_FLIST_DESTROY_EX( &s_flistp, NULL );
}

void
fm_cust_pol_valid_login_check_uniquiness_service(
	pcm_context_t		*ctxp,
	int64			db_no,
	int			msexchange_service_type,
	pin_flist_t		*i_flistp,
	pin_flist_t		*r_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*a_flistp = NULL;
	pin_flist_t		*l_flistp = NULL;
	pin_flist_t		*n_flistp = NULL;
	pin_flist_t		*s1_flistp = NULL;
	pin_flist_t		*s2_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	poid_t			*objp = NULL;
	poid_t			*s_pdp = NULL;
	void			*vp = NULL;
	const char		*subtype = NULL;
	char			*login = NULL;
	int32			result = PIN_CUST_VERIFY_PASSED;
	int32			elemid = 0;
	int32			count1 = 0;
	int32			count2 = 0;
	int32			search_flag = SRCH_DISTINCT;
	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie2 = NULL;
	int			count = 0;
	int32			cred = 0;

        /***********************************************************
         * Return immediately if there's an error in ebufp.
         ***********************************************************/
        if (PIN_ERR_IS_ERR(ebufp))
                return;
        PIN_ERR_CLEAR_ERR(ebufp);

	s_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	/*********************************************************
	 * Create the search flist
	 ********************************************************/
	s1_flistp = PIN_FLIST_CREATE(ebufp);
	s2_flistp = PIN_FLIST_CREATE(ebufp);
	objp = PIN_POID_CREATE(db_no, "/search", -1, ebufp);
	PIN_FLIST_FLD_SET(s1_flistp, PIN_FLD_POID, (void *)objp, ebufp);
	PIN_FLIST_FLD_PUT(s2_flistp, PIN_FLD_POID, (void *)objp, ebufp);

	/******************************************************************
	 * Set the search template.
	 ******************************************************************/

	/**********************************************************************
	 * If Service type is /service/msexchange/user or 
	 * /service/msexchange/user/firstadmin
	 * search login name in both /service/msexchange/user and 
	 * /service/msexchange/user/firstadmin objects
	 **********************************************************************/
	if( msexchange_service_type != PIN_MSEXCHANGE_SERVICE_NONE ){
		vp = (void*)"select X from /service where "
				"F1 = V1 AND ( F2 = V2 OR  F3 = V3 ) ";
	}
	else {
		vp = (void*)"select X from /service where "
				" F1 = V1 AND F2 = V2 ";
	}

	PIN_FLIST_FLD_SET(s1_flistp, PIN_FLD_TEMPLATE, vp, ebufp);
	PIN_FLIST_FLD_SET(s2_flistp, PIN_FLD_TEMPLATE, vp, ebufp);
	PIN_FLIST_FLD_SET(s1_flistp, PIN_FLD_FLAGS, &search_flag, ebufp);
	PIN_FLIST_FLD_SET(s2_flistp, PIN_FLD_FLAGS, &search_flag, ebufp);

	/* Add the search criteria. */
	login = PIN_FLIST_FLD_GET( i_flistp, PIN_FLD_LOGIN, 1, ebufp );
	
	/* Look for PIN_FLD_LOG = login */
	l_flistp = PIN_FLIST_ELEM_ADD(s1_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(l_flistp, PIN_FLD_LOGIN, (void *)login, ebufp);

	/* Look for PIN_FLD_ALIAS_LIST - PIN_FLD_NAME = login */
	n_flistp = PIN_FLIST_ELEM_ADD(s2_flistp, PIN_FLD_ARGS, 1, ebufp);
	n_flistp = PIN_FLIST_ELEM_ADD(n_flistp, PIN_FLD_ALIAS_LIST, 
					PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(n_flistp, PIN_FLD_NAME, (void *)login, ebufp);

	/**********************************************************************
	 * If service is /service/msexchange/user type, search login name also
	 * in /service/msexchange/user/firstadmin for unique login name and 
	 * vice versa.
	 * Add F3 and V3 Parameters in search template with service type
	 **********************************************************************/
	if( msexchange_service_type == PIN_MSEXCHANGE_SERVICE_USER ||
		msexchange_service_type == PIN_MSEXCHANGE_SERVICE_FIRSTADMIN ){
		a_flistp = PIN_FLIST_ELEM_ADD(s1_flistp, PIN_FLD_ARGS,2, ebufp);
		objp = PIN_POID_CREATE(db_no, 
			PIN_MSEXCHANGE_SERVICE_TYPE_FIRSTADMIN,
	       		(int64)-1, ebufp);
		PIN_FLIST_FLD_PUT(a_flistp, PIN_FLD_POID, (void *)objp, ebufp);
		PIN_FLIST_ELEM_SET(s2_flistp, a_flistp, PIN_FLD_ARGS, 2, ebufp);

		a_flistp = PIN_FLIST_ELEM_ADD(s1_flistp, PIN_FLD_ARGS,3, ebufp);
		objp = PIN_POID_CREATE(db_no, PIN_MSEXCHANGE_SERVICE_TYPE_USER,
	       		(int64)-1, ebufp);
		PIN_FLIST_FLD_PUT(a_flistp, PIN_FLD_POID, (void *)objp, ebufp);
		PIN_FLIST_ELEM_SET(s2_flistp, a_flistp, PIN_FLD_ARGS, 3, ebufp);
	} else {
		subtype = PIN_POID_GET_TYPE( s_pdp );
		a_flistp = PIN_FLIST_ELEM_ADD(s1_flistp, PIN_FLD_ARGS,2, ebufp);
		objp = PIN_POID_CREATE(db_no, subtype, (int64)-1, ebufp);
		PIN_FLIST_FLD_PUT(a_flistp, PIN_FLD_POID, (void *)objp, ebufp);
		PIN_FLIST_ELEM_SET(s2_flistp, a_flistp, PIN_FLD_ARGS, 2, ebufp);
	}

	PIN_FLIST_ELEM_PUT(s1_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_ELEM_PUT(s2_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp);

	/************************************************************
	 * we only search if there was a login
	 * otherwise this was all setup for the alias list searches.
	 ************************************************************/
	if( login ) {
		cred = CM_FM_BEGIN_OVERRIDE_SCOPE(CM_CRED_SCOPE_OVERRIDE_ROOT);
		PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_COUNT_ONLY,
			s1_flistp, &a_flistp, ebufp);

		cookie = NULL;
		(void) PIN_FLIST_ELEM_GET_NEXT(a_flistp, PIN_FLD_RESULTS, 
			&count1, 0, &cookie, ebufp);	
		PIN_FLIST_DESTROY_EX( &a_flistp, NULL );

		PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_COUNT_ONLY,
			s2_flistp, &a_flistp, ebufp);
		CM_FM_END_OVERRIDE_SCOPE(cred);

		cookie = NULL;
		(void) PIN_FLIST_ELEM_GET_NEXT(a_flistp, PIN_FLD_RESULTS, 
			&count2, 0, &cookie, ebufp);	
		PIN_FLIST_DESTROY_EX( &a_flistp, NULL );

		if (count1 > 0 || count2 > 0 ) {
			(void)fm_cust_pol_valid_add_fail(r_flistp, 
				PIN_FLD_LOGIN, (u_int)NULL,
				PIN_CUST_VAL_ERR_DUPE,
				PIN_CUST_DUPE_VALUE_ERR_MSG, 
				(void *)login, ebufp);
               		result = PIN_CUST_VERIFY_FAILED;
		}
	}

	/***********************************************************
         * Try to validate all of the alias list
         ***********************************************************/
	cookie = NULL;
	for ( 
		flistp = PIN_FLIST_ELEM_GET_NEXT( i_flistp, PIN_FLD_ALIAS_LIST,
						&elemid, 0, &cookie, ebufp );
		ebufp->pin_err == PIN_ERR_NONE;
		flistp = PIN_FLIST_ELEM_GET_NEXT( i_flistp, PIN_FLD_ALIAS_LIST,
						&elemid, 0, &cookie, ebufp )
	) {

		if ( !flistp ) continue;

		login = PIN_FLIST_FLD_GET( flistp, PIN_FLD_NAME, 0, ebufp );
	
		/**************************************************************
		 * the search flist is still set up we just need to replace
		 * the login value with an alias value.
		 **************************************************************/
		PIN_FLIST_FLD_SET(l_flistp, PIN_FLD_LOGIN, (void *)login, ebufp);
		PIN_FLIST_FLD_SET(n_flistp, PIN_FLD_NAME, (void *)login, ebufp);

		cred = CM_FM_BEGIN_OVERRIDE_SCOPE(CM_CRED_SCOPE_OVERRIDE_ROOT);
		PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_COUNT_ONLY,
			s1_flistp, &a_flistp, ebufp);

		cookie2 = NULL;
		(void) PIN_FLIST_ELEM_GET_NEXT(a_flistp, PIN_FLD_RESULTS, 
			&count1, 0, &cookie2, ebufp);	

		PIN_FLIST_DESTROY_EX( &a_flistp, NULL );

		PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_COUNT_ONLY,
			s2_flistp, &a_flistp, ebufp);
		CM_FM_END_OVERRIDE_SCOPE(cred);

		cookie2 = NULL;
		(void) PIN_FLIST_ELEM_GET_NEXT(a_flistp, PIN_FLD_RESULTS, 
			&count2, 0, &cookie2, ebufp);	

		PIN_FLIST_DESTROY_EX( &a_flistp, NULL );

		if (count1 > 0 || count2 > 0) {
			pin_fld_num_t field = PIN_FLD_ALIAS_LIST;
			/***************************************************
			 * There is a validation error so normalize it to
			 * the input flist
			 ***************************************************/
			result = PIN_CUST_VERIFY_FAILED;
			count = PIN_FLIST_ELEM_COUNT(r_flistp, PIN_FLD_FIELD, 
					ebufp);
			a_flistp = PIN_FLIST_ELEM_ADD(r_flistp, PIN_FLD_FIELD,
					count, ebufp);
			PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_FIELD_NUM,
					(void *)&field, ebufp);
			PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_ELEMENT_ID,
					(void *)&elemid, ebufp);
			PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_RESULT,
					(void *)&result, ebufp);
			(void)fm_cust_pol_valid_add_fail(a_flistp, 
				PIN_FLD_NAME, (u_int)NULL,
				PIN_CUST_VAL_ERR_DUPE,
				PIN_CUST_DUPE_VALUE_ERR_MSG, 
				(void *)login, ebufp);
			a_flistp = NULL;
		}
	}
	if ( ebufp->pin_err == PIN_ERR_NOT_FOUND ) {
		PIN_ERR_CLEAR_ERR( ebufp );
	}


	if ( result == PIN_CUST_VERIFY_FAILED ) {
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_RESULT,
				(void *)&result, ebufp);
	}
	PIN_FLIST_DESTROY_EX( &s1_flistp, NULL );
	PIN_FLIST_DESTROY_EX( &s2_flistp, NULL );
	
}

