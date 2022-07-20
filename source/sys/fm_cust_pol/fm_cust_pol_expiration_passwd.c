/* continuus file information --- %full_filespec: fm_cust_pol_expiration_passwd.c~12:csrc:1 % */
/*******************************************************************
 *
 *  @(#) %full_filespec: fm_cust_pol_expiration_passwd.c~12:csrc:1 %
 *
 *      Copyright (c) 2004-2006 Oracle. All rights reserved.
 * 
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/ 

#ifndef lint
static  char Sccs_Id[] = "@(#)%Portal Version: fm_cust_pol_expiration_passwd.c:BillingVelocityInt:1:2006-Sep-05 04:28:10 %";
#endif

#include <stdio.h>
#include <stdlib.h> 
#include <sys/types.h>
#include <unistd.h>

#ifdef MSDOS
#include <sys/socket.h>
#include <sys/timeb.h>
#endif
 
#ifdef __SVR4
#include <sys/systeminfo.h>	/* no gethostid() in Solaris */
#endif /*__SVR4*/

#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pinlog.h"

#define FILE_SOURCE_ID  "fm_cust_pol_expiration_passwd.c(1.16)"
#define NUMBER_OF_SECONDS_IN_DAY (24*60*60)

PIN_IMPORT int32   *fm_cust_pol_passwd_exp_daysp;

/*******************************************************************
 * Routines contained herein.
 *******************************************************************/
EXPORT_OP void
op_cust_pol_expiration_passwd(
        cm_nap_connection_t	*connp,
	int32			opcode,
        int32			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_EXPIRATION_PASSWD  command
 *******************************************************************/
void
op_cust_pol_expiration_passwd(
        cm_nap_connection_t	*connp,
	int32			opcode,
        int32			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*r_flistp = NULL;
	poid_t			*o_pdp;	
	/* Set default password age to 90 days */
	int32			passwd_exp_days = 90;
	int32			err;
	time_t			now = 0;
	time_t			exp_t = 0;

	/***********************************************************
	 * Null out results until we have some.
	 ***********************************************************/
	*ret_flistpp = NULL;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_CUST_POL_EXPIRATION_PASSWD) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_cust_pol_expiration_passwd", ebufp);
		return;
	}

	/***********************************************************
	 * We will not open any transactions with Policy FM
	 * since policies should NEVER modify the database.
	 ***********************************************************/

	now = pin_virtual_time((time_t *)NULL);

	if (fm_cust_pol_passwd_exp_daysp != (int32*) NULL) {
		passwd_exp_days = *fm_cust_pol_passwd_exp_daysp;
	}

	exp_t = now + NUMBER_OF_SECONDS_IN_DAY*passwd_exp_days;

	/***********************************************************
	 * Call main function to do it
	 ***********************************************************/
	r_flistp = PIN_FLIST_CREATE(ebufp);
	/***********************************************************
         * Get (and add) the object poid.
         ***********************************************************/
        o_pdp = (poid_t *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, (void *)o_pdp, ebufp);
        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_PASSWD_EXPIRATION_T, &exp_t, ebufp);


	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		*ret_flistpp = (pin_flist_t *)NULL;
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_cust_pol_expiration_passwd error", ebufp);
	} else {
		*ret_flistpp = r_flistp;
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_expration_passwd return flist", r_flistp);
	}

	return;
}

