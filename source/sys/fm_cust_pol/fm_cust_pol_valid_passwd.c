/*******************************************************************
 *
* Copyright (c) 1999, 2019, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char Sccs_Id[] = "@(#)$Id: fm_cust_pol_valid_passwd.c /cgbubrm_mainbrm.portalbase/1 2019/02/25 22:33:19 agangrad Exp $";
#endif

#include <stdio.h>
#include <stdlib.h> 
#include <ctype.h>
 
#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pinlog.h"

#define FM_PWD_MAX_PASSWD	255
#define FM_PWD_INVALID_PASSWD	"&aes|"

PIN_IMPORT int32    bparam_enable_password_restriction;
/*******************************************************************
 * Routines contained herein.
 *******************************************************************/
EXPORT_OP void
op_cust_pol_valid_passwd(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp);

static void
fm_cust_pol_valid_passwd(
	pcm_context_t		*ctxp,
	u_int			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistpp,
        pin_errbuf_t		*ebufp);

static void
fm_cust_pol_valid_pwd_srvc(
	pcm_context_t		*ctxp,
	u_int			flags,
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
	u_int           pin_fld_field_num,
	u_int           pin_fld_element_id,
	char            *cfg_name,
	int             type,
	pin_errbuf_t    *ebufp);
 
/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_VALID_PASSWD  command
 *******************************************************************/
void
op_cust_pol_valid_passwd(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
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
	if (opcode != PCM_OP_CUST_POL_VALID_PASSWD) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_cust_pol_valid_passwd", ebufp);
		return;
	}

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_cust_pol_valid_passwd input flist", 
        in_flistp);

	/***********************************************************
	 * Call main function to do it
	 ***********************************************************/
	fm_cust_pol_valid_passwd(ctxp, flags, in_flistp, &r_flistp, ebufp);

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		*ret_flistpp = (pin_flist_t *)NULL;
		PIN_FLIST_DESTROY(r_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_cust_pol_valid_passwd error", ebufp);
	} else {
		*ret_flistpp = r_flistp;
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_valid_passwd return flist", r_flistp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_valid_passwd()
 *
 *	Validate the given passwd according to the given poid type.
 *
 *******************************************************************/
static void
fm_cust_pol_valid_passwd(
	pcm_context_t		*ctxp,
        u_int			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistpp,
        pin_errbuf_t		*ebufp)
{
	pin_flist_t		*r_flistp = NULL;

	poid_t			*o_pdp;
	const char		*o_type = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Create outgoing flist
	 ***********************************************************/
	r_flistp = PIN_FLIST_CREATE(ebufp);
	*out_flistpp = r_flistp;

	/***********************************************************
	 * Get (and add) the object poid.
	 ***********************************************************/
	o_pdp = (poid_t *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, (void *)o_pdp, ebufp);

	/***********************************************************
	 * We might have different rules for different services.
	 ***********************************************************/
	o_type = PIN_POID_GET_TYPE(o_pdp);

	if (!strncmp(o_type, "/service", 8)) {

		/* Any service password */
		fm_cust_pol_valid_pwd_srvc(ctxp, flags, in_flistp, r_flistp, ebufp);

	} else {

		/* Error - unknown/usupported type */
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_POID_TYPE, PIN_FLD_POID, 0, 0);

	}

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_valid_passwd error", ebufp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_valid_pwd_srvc():
 *
 *	Validate the service level password.
 *
 *******************************************************************/
static void
fm_cust_pol_valid_pwd_srvc(
	pcm_context_t		*ctxp,
        u_int			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		*r_flistp,
        pin_errbuf_t		*ebufp)
{
  u_int result;
  void *vp = NULL;
  const char *pwd = NULL;
  char *cpasswd = NULL;
  char *loginid = NULL;
  char *oldpwd = NULL;
  
  char tmploginid[255] = "";
  char tmppwd[255] = "";
  char tmpstr[255] = "";
  
  int32 i = 0;
  int32 hasUpper = 0, hasLower = 0, hasDigit = 0, hasSpecial = 0;
  const int32 PASSWD_PART = 3;
  
  pin_flist_t* search_flistp = NULL;
  pin_flist_t* args_flistp = NULL;
  pin_flist_t* result_flistp = NULL;
  pin_flist_t* search_result_flistp = NULL;

  pin_flist_t* cmp_flistp = NULL;
  pin_flist_t* cmp_result_flistp = NULL;
  u_int* password_matchp = NULL;
  u_int  password_match = 0;
  
  int32 search_flags = 0;
  int64 db_no = 0;
  
  poid_t* search_poidp = NULL;
  poid_t* event_poidp = NULL;
  poid_t* service_poidp = NULL;
  char* service_typep = NULL;
  pin_cookie_t cookie = NULL;
  
	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Pass by default.
	 ***********************************************************/
	result = PIN_CUST_VERIFY_PASSED;
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_RESULT, (void *)&result, ebufp);

	pwd = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PASSWD_CLEAR, 0, ebufp); 
	if (pwd) {

    if (!strncmp(pwd, FM_PWD_INVALID_PASSWD, strlen(FM_PWD_INVALID_PASSWD))) {
      PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, " Invalid Password."); 
      pin_errbuf_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE,
                                     PIN_ERR_VALIDATION_FAILED, 0, 0, 0, PIN_DOMAIN_ERRORS,
                                     PIN_ERR_VALIDATE_PASSWORD, 1, 0, NULL);
      goto cleanup;
      /***********/
    }

    if (bparam_enable_password_restriction) {
    service_poidp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
    service_typep = (char *)PIN_POID_GET_TYPE(service_poidp);
    if (! (!strncmp(service_typep, "/service/telco", 14) || !strncmp(service_typep, "/service/gsm", 12))) {
    
    vp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PASSWORDS, PIN_ELEMID_OLD, 0, ebufp);
    loginid = PIN_FLIST_FLD_GET(vp, PIN_FLD_LOGIN, 0, ebufp); 
    cpasswd = PIN_FLIST_FLD_GET(vp, PIN_FLD_PASSWD, 0, ebufp);

    if (!strcmp(loginid, pwd)) {
      PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, " Invalid Password: Password should not be the same as login-id"); 
      pin_errbuf_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE,
                                     PIN_ERR_VALIDATION_FAILED, 0, 0, 0, PIN_DOMAIN_ERRORS,
                                     PIN_ERR_VALIDATE_PASSWORD, 1, 0, NULL);
      goto cleanup;
      /***********/
    }
    
    if (strlen(pwd) < 8) {
      PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, " Invalid Password: Password must be a minimum of 8 characters or more"); 
      pin_errbuf_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE,
                                     PIN_ERR_VALIDATION_FAILED, 0, 0, 0, PIN_DOMAIN_ERRORS,
                                     PIN_ERR_VALIDATE_PASSWORD, 1, 0, NULL);
      goto cleanup;
      /***********/
    }
    
    for (i = 0; i < strlen(pwd); i++) {
      if (isalpha(pwd[i])) {
        if (isupper(pwd[i])) {
          hasUpper = 1;
        }
        else {
          hasLower = 1;
        }
      }
      else if (isdigit(pwd[i])) {
        hasDigit = 1;
      }
      else if (isprint(pwd[i])) {
        hasSpecial = 1;
      }
    }
    
    if (! hasUpper || ! hasLower || ! hasDigit || ! hasSpecial) {
      PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, " Invalid Password: Password must contain a combination of uppercase, lowercase, digit and special characters"); 
      pin_errbuf_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE,
                                     PIN_ERR_VALIDATION_FAILED, 0, 0, 0, PIN_DOMAIN_ERRORS,
                                     PIN_ERR_VALIDATE_PASSWORD, 1, 0, NULL);
      goto cleanup;
      /***********/
    }
   
    if(loginid){ 
    	for (i = 0; i < strlen(loginid); i++) {
      		tmploginid[i] = tolower(loginid[i]);
    	}
    }
    tmploginid[i] = '\0';
    
    for (i = 0; i < strlen(pwd); i++) {
      tmppwd[i] = tolower(pwd[i]);
    }
    tmppwd[i] = '\0';
    
    for (i = 0; i <= strlen(tmppwd) - PASSWD_PART; i++) {
      strncpy(tmpstr, &tmppwd[i], PASSWD_PART); tmpstr[PASSWD_PART] = '\0';
      if (strstr(tmploginid, tmpstr) != NULL) {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, " Invalid Password: Password is very similar to login-id"); 
        pin_errbuf_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE,
                                       PIN_ERR_VALIDATION_FAILED, 0, 0, 0, PIN_DOMAIN_ERRORS,
                                       PIN_ERR_VALIDATE_PASSWORD, 1, 0, NULL);
        goto cleanup;
        /***********/
      }
    }
   
    if (flags & PCM_OPFLG_CUST_REGISTRATION) {
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Customer Registration Flow, Skipping Password History Checks...\n");
    }
	else if (flags & PCM_OPFLG_CUST_CREATE_SERVICE) {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Service Creation Flow, Skipping Password History Checks...\n");
    }
    else {

    cmp_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_SET(cmp_flistp, PIN_FLD_POID, (void *)service_poidp, ebufp);
    PIN_FLIST_FLD_SET(cmp_flistp, PIN_FLD_PASSWD_CLEAR, (void *)pwd, ebufp);
    PIN_FLIST_FLD_SET(cmp_flistp, PIN_FLD_PASSWD, (void *)cpasswd, ebufp);

    PCM_OP(ctxp, PCM_OP_CUST_POL_COMPARE_PASSWD, 0, cmp_flistp, &cmp_result_flistp, ebufp);

    password_matchp = PIN_FLIST_FLD_GET(cmp_result_flistp, PIN_FLD_BOOLEAN, 0, ebufp);
    if (password_matchp) {
      password_match = *password_matchp;
    }

    if (cmp_result_flistp) {
      PIN_FLIST_DESTROY_EX(&cmp_result_flistp, NULL);
    }

    if (PIN_ERR_IS_ERR(ebufp)) {
      goto cleanup;
    }

    if (password_match == PIN_BOOLEAN_TRUE) {
      PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, " Invalid Password: Password is "
                                           "same as the current password"); 
      pin_errbuf_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE,
                                     PIN_ERR_VALIDATION_FAILED, 0, 0, 0, PIN_DOMAIN_ERRORS,
                                     PIN_ERR_VALIDATE_PASSWORD, 1, 0, NULL);
      goto cleanup;
      /***********/
    }

    search_flistp = PIN_FLIST_CREATE(ebufp);
    search_flags = 0;
    
    if (service_poidp) {
      db_no = PIN_POID_GET_DB(service_poidp);
    }
    
    search_poidp = PIN_POID_CREATE(db_no, "/search", -1, ebufp);
    event_poidp = PIN_POID_CREATE(db_no, "/event/customer/password", -1, ebufp);
    
    /* basic search */
    PIN_FLIST_FLD_PUT(search_flistp, PIN_FLD_POID, search_poidp, ebufp);
    PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_TEMPLATE, 
        " select X from /event where F1 = V1 and F2 = V2 order by F3 desc ", ebufp);
    PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_FLAGS, &search_flags, ebufp);
    
    /* retrieve a max of last 4 password events */
    result_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_RESULTS, 4, ebufp);
      result_flistp = PIN_FLIST_ELEM_ADD(result_flistp, PIN_FLD_PASSWORDS, 1, ebufp);
        PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PASSWD, NULL, ebufp);
        
    /* argument list */
    args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 1, ebufp);
      PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, service_poidp, ebufp);
    args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 2, ebufp);
      PIN_FLIST_FLD_PUT(args_flistp, PIN_FLD_POID, event_poidp, ebufp);
    args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 3, ebufp);
      PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_MOD_T, NULL, ebufp);
      
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search flist for old password changes", 
        search_flistp);
    
    PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_flistp, &search_result_flistp, ebufp);
    
    if (PIN_ERR_IS_ERR(ebufp)) {
      goto cleanup;
    }
    
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search result flist for old password changes", 
    search_result_flistp);

    while ((result_flistp = PIN_FLIST_ELEM_GET_NEXT(search_result_flistp, PIN_FLD_RESULTS, 
      &i, 1, &cookie, ebufp)) != NULL) {
      result_flistp = PIN_FLIST_ELEM_GET(result_flistp, PIN_FLD_PASSWORDS, 1, 0, ebufp);
      oldpwd = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_PASSWD, 0, ebufp);

      password_matchp = NULL;
      cmp_result_flistp = NULL;
      PIN_FLIST_FLD_SET(cmp_flistp, PIN_FLD_PASSWD, oldpwd, ebufp);

      PCM_OP(ctxp, PCM_OP_CUST_POL_COMPARE_PASSWD, 0, cmp_flistp, &cmp_result_flistp, ebufp);

      password_matchp = PIN_FLIST_FLD_GET(cmp_result_flistp, PIN_FLD_BOOLEAN, 0, ebufp);
      if (password_matchp) {
        password_match = *password_matchp;
      }

      if (cmp_result_flistp) {
        PIN_FLIST_DESTROY_EX(&cmp_result_flistp, NULL);
      }

      if (PIN_ERR_IS_ERR(ebufp)) {
        goto cleanup;
      }

      if (password_match == PIN_BOOLEAN_TRUE) {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, " Invalid Password: Password should not be "
                                             "the same as any of the last four passwords"); 
        pin_errbuf_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE,
                                       PIN_ERR_VALIDATION_FAILED, 0, 0, 0, PIN_DOMAIN_ERRORS,
                                       PIN_ERR_VALIDATE_PASSWORD, 1, 0, NULL);
        goto cleanup;
        /***********/
      }
    } /* end while loop */
    } /* end password history checks */
    
	} /* end not service-telco or service-gsm */
	} /* end of password restriction */
	} /* end if(pwd) block */



	/***********************************************************
	 * If it's there, is it too long?
         * Try to validate based on /config/fld_validate (Pswd) obj.
         ***********************************************************/
	fm_cust_pol_validate_fld_value( ctxp, i_flistp, i_flistp, r_flistp,
		PIN_FLD_PASSWD_CLEAR,
		0,              /* element_id   */
		"Pswd",
		1,
		ebufp);

cleanup:
  if (search_flistp) PIN_FLIST_DESTROY_EX(&search_flistp, NULL);
  if (search_result_flistp) PIN_FLIST_DESTROY_EX(&search_result_flistp, NULL);
  if (cmp_flistp) PIN_FLIST_DESTROY_EX(&cmp_flistp, NULL);

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_valid_pwd_srvc error", ebufp);
	}

	return;
}

