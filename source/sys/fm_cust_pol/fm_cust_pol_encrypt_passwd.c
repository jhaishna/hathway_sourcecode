/*******************************************************************
 *
* Copyright (c) 1999, 2017, Oracle and/or its affiliates. All rights reserved.
 * 
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char Sccs_Id[] = "@(#)$Id: fm_cust_pol_encrypt_passwd.c /cgbubrm_mainbrm.portalbase/1 2017/07/16 21:06:18 crunkana Exp $";
#endif

/*******************************************************************
 * This file contains the PCM_OP_CUST_POL_ENCRYPT_PASSWD operation. 
 *
 * This op simply encrypts a clear text passwd based on the type
 * of the poid given and/or the requested encryption algorithm.
 *
 *******************************************************************/

#include <stdio.h> 
#include <stdlib.h>
#include <string.h> 
 
#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_cust.h"
#include "pin_type.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "pin_msexchange.h"

PIN_IMPORT char * sha1_encode(const char* clear_text, const char* inp_salt);
PIN_IMPORT char * pbkdf2_encode(const char* clear_text, const char* inp_salt);
/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_cust_pol_encrypt_passwd(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**r_flistpp,
        pin_errbuf_t		*ebufp);

static char *
fm_cust_pol_encrypt_clear(
	char	*pwd_clear,
	u_int	*errp);

static char *
fm_cust_pol_encrypt_md5(
	char	*pwd_clear,
	u_int	*errp);


/*******************************************************************
 * Routines needed from elsewhere.
 *******************************************************************/
extern	void
make_digest(
	char	*string,
	char	*digest);

/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_ENCRYPT_PASSWD operation.
 *******************************************************************/
void
op_cust_pol_encrypt_passwd(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**r_flistpp,
        pin_errbuf_t		*ebufp)
{
	poid_t			*o_pdp;
	char			*pwd_clear = NULL;
	char			*pwd_crypt = NULL;
	u_int			err = PIN_ERR_NONE;
	char                    *pwd_in_db = NULL;
	char                    *salt = NULL;

	/***********************************************************
	 * Null out results until we have some.
	 ***********************************************************/
	*r_flistpp = NULL;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_CUST_POL_ENCRYPT_PASSWD) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_pwd_encrypt: bad opcode", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What did we get?
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_cust_pol_encrypt_passwd input flist", i_flistp);

	/***********************************************************
	 * Get the object poid (type drives encrypt algorithm).
	 ***********************************************************/
	o_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

	/***********************************************************
	 * Get the clear text string.
	 ***********************************************************/
	pwd_clear = (char *)PIN_FLIST_FLD_GET(i_flistp,
		PIN_FLD_PASSWD_CLEAR, 0, ebufp);

	/***********************************************************
	 * Encrypt the passwd according to object type.
	 *
	 * For IP accounts, we use clear text 'encryption' to
	 * support CHAP. All other passwords use our md5 algo.
	 ***********************************************************/
	if (pwd_clear) {
		if (fm_utils_is_subtype(o_pdp, "/service/ip")||
		    fm_utils_is_subtype(o_pdp, "/service/telephony")||
		    fm_utils_is_subtype(o_pdp, "/service/ssg")||
		    fm_utils_is_subtype(o_pdp, "/service/ldap") ||
		    fm_utils_is_subtype(o_pdp, "/service/ip/gprs") ||
		    fm_utils_is_subtype(o_pdp, "/service/content") ||
		    fm_utils_is_subtype(o_pdp, "/service/contentprovider") ||
		    fm_utils_is_subtype(o_pdp, PIN_MSEXCHANGE_SERVICE_TYPE_USER) ||
		    fm_utils_is_subtype(o_pdp,
									PIN_MSEXCHANGE_SERVICE_TYPE_FIRSTADMIN) ||
		    fm_utils_is_subtype(o_pdp, "/service/wap/interactive")) {

			pwd_crypt = fm_cust_pol_encrypt_clear(pwd_clear, &(err));

		} else {

			if( flags == 1 ) {
				/***********************************************************
				 * Get the encrypted string from the input.
				 * When called from op_cust_pol_compare_passwd the
				 * i_flistp contains the earlier encoded password
				 ***********************************************************/
				pwd_in_db = (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PASSWD, 0, ebufp);

				if (PIN_ERR_IS_ERR(ebufp)) {
	 				goto cleanup;
	 			}

				if (pwd_in_db != NULL) {
					if (strncmp(pwd_in_db, "md5|", 4) == 0) {
						pwd_crypt = fm_cust_pol_encrypt_md5(pwd_clear, &(err));
					}
					else if(strncmp(pwd_in_db, "sha1|", 5) == 0){ // sha1 encoded format
						salt = strrchr(pwd_in_db, '|');
						salt++;
						pwd_crypt = (char *)sha1_encode(pwd_clear,salt);
					} else { // pbkdf2 encoded format
						salt = strrchr(pwd_in_db, '|');
						salt++;
						pwd_crypt = (char *)pbkdf2_encode(pwd_clear,salt);
					}
				} else {
	 				err = PIN_ERR_NULL_PTR;
	 			}
			}
			else { // For cust_create and cust_update use sha1
				pwd_crypt = (char *)pbkdf2_encode(pwd_clear,NULL);
			}
		}

		if (err != PIN_ERR_NONE) {
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				    PIN_ERRCLASS_SYSTEM_DETERMINATE,
				    err, 0, 0, 0);
		}
	} else {
		pwd_crypt = pwd_clear;
	}

cleanup:
	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		*r_flistpp = (pin_flist_t *)NULL;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_cust_pol_encrypt_passwd error", ebufp);

                if(pwd_crypt){
                    pin_free(pwd_crypt);
                }
	} else {
		*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
		PIN_FLIST_FLD_PUT(*r_flistpp, PIN_FLD_PASSWD,
				  (void *)pwd_crypt, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_encrypt_passwd return flist", *r_flistpp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_encrypt_clear():
 *
 *	'Encypts' a password into our clear text storage format.
 *
 *******************************************************************/
static char *
fm_cust_pol_encrypt_clear(
	char	*pwd_clear,
	u_int	*errp)
{
	char	*crypt;
	int	i;

	/***********************************************************
	 * Check for NULL passwd.
	 ***********************************************************/
	if (pwd_clear == (char *)NULL) {
		*errp = PIN_ERR_NONE;
		return(NULL);
	}

	/***********************************************************
	 * Put the string together.
	 ***********************************************************/
	i = strlen(PIN_PWD_ENCRYPT_TYPE_CLEAR) + 1 + strlen(pwd_clear) + 1;
	crypt = (char *)pin_malloc(i);
	if (!crypt) {
		*errp = PIN_ERR_NO_MEM;
		return NULL;
	}

	pin_snprintf(crypt, i, "%s|%s", PIN_PWD_ENCRYPT_TYPE_CLEAR, pwd_clear);

	/***********************************************************
	 * No errors.
	 ***********************************************************/
	*errp = PIN_ERR_NONE;

	return(crypt);
}

/*******************************************************************
 * fm_cust_pol_encrypt_md5():
 *
 *	Digests the given string w/MD5 and then converts the
 *	binary result into an ascii like string to facilitate
 *	storage. The ascii like string is a per byte hex 
 *	representation of the MD5's binary string.
 *
 * XXX: We should define the MD5 digest length (16) as a constant
 *	somewhere. The ascii like string is always twice as long.
 *
 *******************************************************************/
static char *
fm_cust_pol_encrypt_md5(
	char	*pwd_clear,
	u_int	*errp)
{
	char	*crypt;
	char	digest[16];
	char	*pseudo;
	int	i;

	/***********************************************************
	 * Check for NULL passwd.
	 ***********************************************************/
	if (pwd_clear == (char *)NULL) {
		*errp = PIN_ERR_NONE;
		return(NULL);
	}

	/***********************************************************
	 * Just call the MD5 routine.
	 ***********************************************************/
	make_digest(pwd_clear, digest);

	/***********************************************************
	 * Put the string together.
	 ***********************************************************/
	i = strlen(PIN_PWD_ENCRYPT_TYPE_MD5) + 1 + (2 * 16 + 1);
	crypt = (char *)pin_malloc(i);
	if (!crypt) {
		*errp = PIN_ERR_NO_MEM;
		return NULL;
	}
	sprintf(crypt, "%s|", PIN_PWD_ENCRYPT_TYPE_MD5);

	/***********************************************************
	 * 'pseudo-ascii' the results.
	 ***********************************************************/
	pseudo = (char *)&crypt[strlen(PIN_PWD_ENCRYPT_TYPE_MD5) + 1];

	for (i = 0; i < 16; i++)
		sprintf(pseudo + (2 * i), "%02x", digest[i] & 0xFF);
 
	/***********************************************************
	 * No errors.
	 ***********************************************************/
	*errp = PIN_ERR_NONE;

	return(crypt);
}

