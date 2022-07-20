/*******************************************************************
 *
 *      Copyright (c) 1999-2008 Oracle. All rights reserved.
 * 
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char Sccs_Id[] = "@(#)%Portal Version: fm_cust_pol_decrypt_passwd.c:CUPmod7.3PatchInt:1:2007-Jan-07 21:31:14 %";
#endif

/*******************************************************************
 * This file contains the PCM_OP_CUST_POL_DECRYPT_PASSWD operation. 
 *
 * This op simply decrypts a clear text passwd based on the type
 * of the poid given and/or the requested decryption algorithm.
 *
 *******************************************************************/

#include <stdio.h> 
#include <string.h> 
 
#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_cust.h"
#include "pin_errs.h"
#include "pinlog.h"

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_cust_pol_decrypt_passwd(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp);



/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_DECRYPT_PASSWD operation.
 *******************************************************************/
void
op_cust_pol_decrypt_passwd(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp)
{
	pin_flist_t		*r_flistp = NULL;
	char			*pwd_clear = NULL;
	char			*pwd_crypt = NULL;
	char			*how = NULL;
	char			*dumb = NULL;
	poid_t                  *svc_poid = NULL ;
        const char   		*svc_type = NULL;
	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_CUST_POL_DECRYPT_PASSWD) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_pwd_decrypt: bad opcode", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What did we get?
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_cust_pol_decrypt_passwd input flist", i_flistp);

	/***********************************************************
	 * Prep the return flist.
	 ***********************************************************/
	r_flistp = PIN_FLIST_COPY(i_flistp, ebufp);

	/***********************************************************
	 * Get the encrypted password string.
	 ***********************************************************/
	pwd_crypt = (char *)PIN_FLIST_FLD_GET(i_flistp,
		PIN_FLD_PASSWD, 0, ebufp);

	/***********************************************************
	 * How was the password encrypted?
	 ***********************************************************/
	if ( pwd_crypt )
		dumb = strdup(pwd_crypt);
	if ( dumb )
		how = (char *)strtok(dumb, "|");

	/***********************************************************
	 * Read the servie poid and check if it is /service/telco ?
	 * If it is reserve the condition based on what password is 
	 * set to be cleared
	 ***********************************************************/
	svc_poid = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
        svc_type = PIN_POID_GET_TYPE(svc_poid);
	if(svc_type && strstr(svc_type,"/service/telco") != 0) {
	if ((how == (char *)NULL) || !strcmp(how, PIN_PWD_ENCRYPT_TYPE_CLEAR)) {
		/* don't know how to do anything but CLEAR */
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_ARG, PIN_FLD_PASSWD, 0, 0);
	} else {

		pwd_clear = strtok(NULL, "|");
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_PASSWD_CLEAR,
			(void *)pwd_clear, ebufp);

	}
	} else {
	if ((how == (char *)NULL) || strcmp(how, PIN_PWD_ENCRYPT_TYPE_CLEAR)) {
		/* don't know how to do anything but CLEAR */
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_ARG, PIN_FLD_PASSWD, 0, 0);
	} else {

		pwd_clear = strtok(NULL, "|");
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_PASSWD_CLEAR,
			(void *)pwd_clear, ebufp);

	}
	}

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		*o_flistpp = (pin_flist_t *)NULL;
		PIN_FLIST_DESTROY(r_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_cust_pol_decrypt_passwd error", ebufp);
	} else {
		*o_flistpp = r_flistp;
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_decrypt_passwd return flist", r_flistp);
	}

	free( dumb );

	return;
}

