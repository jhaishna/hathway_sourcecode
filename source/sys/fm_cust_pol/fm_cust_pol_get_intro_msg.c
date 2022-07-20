/*******************************************************************
 *
* Copyright (c) 1999, 2015, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char Sccs_Id[] = "@(#)$Id: fm_cust_pol_get_intro_msg.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 04:29:17 nishahan Exp $";
#endif

#include <stdio.h>
#include "pin_os_dir.h"
#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pinlog.h"
#include <strings.h>
#include <stdlib.h>

/*******************************************************************
 * Contains the PCM_OP_CUST_POL_GET_INTRO_MSG operation.
 *
 * This source contains code to handle the get intro opcode.  It
 * will return an introductory message, normally associated with
 * on-line registration.  
 *******************************************************************/

EXPORT_OP void
op_cust_pol_get_intro_msg(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp);

static void
fm_cust_pol_get_intro_msg(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistpp,
        pin_errbuf_t		*ebufp);


static void
fm_cust_pol_get_intro(
	pin_flist_t	*out_flistp,
	pin_flist_t	*in_flistp,
	pin_errbuf_t	*ebufp);

static void
fm_cust_pol_get_intro_macro(
	FILE		*fp,
	pin_flist_t	*in_flistp,
	pin_cust_buf_t	*bufp,
	pin_errbuf_t	*ebufp);


/***********************************************************************
 * The following routines are defined elsewhere
 ***********************************************************************/
extern void
fm_cust_pol_buf_create(
	pin_cust_buf_t	**bufferpp,
	pin_errbuf_t	*ebufp);


extern void
fm_cust_pol_buf_destroy(
	pin_cust_buf_t		*bufferp,
	pin_errbuf_t		*ebufp);

extern void 
fm_cust_pol_buf_put(
	pin_cust_buf_t		*bufferp,
	char			c,
	pin_errbuf_t		*ebufp);

extern void 
fm_cust_pol_buf_cat(
	pin_cust_buf_t		*bufferp,
	char			*string,
	pin_errbuf_t		*ebufp);

extern void
fm_cust_pol_set_buf_fld(
	pin_flist_t		*flistp,
	u_int			fldno,
	char			*valp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_GET_INTRO_MSG operation.
 *******************************************************************/
void
op_cust_pol_get_intro_msg(
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
	if (opcode != PCM_OP_CUST_POL_GET_INTRO_MSG) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_cust_pol_get_intro_msg", ebufp);
		return;
	}

	/***********************************************************
	 * Call main function to do it
	 ***********************************************************/
	fm_cust_pol_get_intro_msg(ctxp, in_flistp, &r_flistp, ebufp);

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		*ret_flistpp = (pin_flist_t *)NULL;
		PIN_FLIST_DESTROY(r_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_cust_pol_get_intro_msg error", ebufp);
	} else {
		*ret_flistpp = r_flistp;
		PIN_ERR_CLEAR_ERR(ebufp);
	}

	return;
}


/*******************************************************************
 * fm_cust_pol_get_intro_msg()
 *
 *	Return the intro message according to the aacinfo supplied.
 *
 *******************************************************************/
static void
fm_cust_pol_get_intro_msg(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistpp,
        pin_errbuf_t		*ebufp)
{
	void			*vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Create outgoing flist
	 ***********************************************************/
	*out_flistpp = PIN_FLIST_CREATE(ebufp);

	vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_POID, vp, ebufp);

	/***********************************************************
	 * Get the intro message and put it on the outgoing flist
	 ***********************************************************/
	fm_cust_pol_get_intro(*out_flistpp, in_flistp, ebufp);

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if(PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_get_intro_msg error", ebufp);
	}

	return;
}

/********************************************************************
 * fm_cust_pol_get_intro():
 *
 *	Obtains intro message, currently read from a file.
 *
 ********************************************************************/
static void
fm_cust_pol_get_intro(
	pin_flist_t	*out_flistp,
	pin_flist_t	*in_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_cust_buf_t	*bufp = NULL;
        char            introfile[PIN_MAXPATHLEN];
	char		*path = NULL;
	char		*fname = NULL;
	char		*access = NULL;

	int32		perr;

	FILE		*fp = NULL;

	int		c = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/*************************************************************
	 * Obtain directory where intro files exist 
	 *************************************************************/
        pin_conf("fm_cust_pol", "intro_dir", PIN_FLDT_STR, (caddr_t *)&path, &perr);
        switch (perr) {
        case PIN_ERR_NONE:
                break;
        default:
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_INVALID_CONF, 0, 0, perr);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"intro_dir configuration error", ebufp);
		goto ErrOut;
        }

	/*************************************************************
	 * Obtain the AAC_SOURCE field, which will be the filename
	 *************************************************************/
	access = (char *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_AAC_ACCESS, 1, ebufp);

	if ((access != (char *)NULL) &&
	    (!strcmp(access, PIN_CUST_AAC_ACCESS_MECCAD))) {

		fname = (char *)PIN_FLIST_FLD_GET(in_flistp,
			PIN_FLD_AAC_SOURCE, 1, ebufp);

	}

	if (fname == (char *)NULL) {
		fname = "default";
	}

	/*************************************************************
	 * Open the Intro file for reading
	 *************************************************************/
	pin_snprintf(introfile, sizeof(introfile), "%s/%s.intro", path, fname);

	fp = fopen(introfile, "r");
	if (fp == NULL) {

		pin_snprintf(introfile, sizeof(introfile), "%s/%s.intro", path, "default");
		fp = fopen(introfile, "r");

		if (fp == NULL) {
                	pin_set_err(ebufp, PIN_ERRLOC_FM,
                        	PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        	PIN_ERR_FILE_IO, 0, 0, 0);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"error opening intro file", ebufp);
			goto ErrOut;
		}

	}

	/*************************************************************
	 * Init some buffers
	 *************************************************************/
	fm_cust_pol_buf_create( &bufp, ebufp);

	/*************************************************************
	 * Read until EOF
	 *************************************************************/

	c = fgetc(fp);
	while( ! feof(fp) && ! PIN_ERR_IS_ERR(ebufp) ) {

		switch(c) {
		case '$':
			c = fgetc(fp);
			if(c == '{') {

				/********************************************
				 * This is a substitution macro, so go 
				 * parse it and get the value from flist
				 ********************************************/

				fm_cust_pol_get_intro_macro( fp, 
					in_flistp, bufp, ebufp);
			}
			else {
				/********************************************
				 * didn't turn out to be a macro, so put the
				 * dollar sign and current char into buf
				 ********************************************/

				fm_cust_pol_buf_put( bufp, '$', ebufp);
				fm_cust_pol_buf_put( bufp, (char)c, ebufp);
			}
			break;

		case EOF:
			break;

		default:
			/********************************************
			 * Put this char into buffer, not a macro
			 ********************************************/

			fm_cust_pol_buf_put( bufp, (char)c, ebufp);
		}

		/********************************************
		 * Get next char from file
		 ********************************************/

		c = fgetc(fp);

	} /* End while */

	/********************************************
	 * Ok, we have read the entire file and built
	 * up a buffer, so now put it on the outgoing
	 * flist in the specified flist field
	 ********************************************/

	fm_cust_pol_set_buf_fld(out_flistp, PIN_FLD_REG_INTRO_MSG, 
		bufp->bufp, ebufp);

	if(PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"flist error", ebufp);
		goto ErrOut;
	}
	
	/******************************************************
	 * Success!!!
	 ******************************************************/

	if(fp) fclose(fp);
	if(path) free(path);
	if(bufp) fm_cust_pol_buf_destroy(bufp, NULL);
	return;

ErrOut:
	if(fp) fclose(fp);
	if(path) free(path);
	if(bufp) fm_cust_pol_buf_destroy(bufp, NULL);
	return;
}



/**************************************************************
 * This function will do macro substitution.  It assumes that
 * a higher level function already read a '$' followed by '{'
 * and that the next sequence of characters read from the file
 * will represent a macro name....until a '}' char is found.
 * when the macro name has been read completely, try to match
 * against a known set of macro names and grab the data file 
 * from the in_flistp.
 **************************************************************/
static void
fm_cust_pol_get_intro_macro(
	FILE		*fp,
	pin_flist_t	*in_flistp,
	pin_cust_buf_t	*bufp,
	pin_errbuf_t	*ebufp)
{
	pin_errbuf_t	errbuf;
	char		macro[CM_FM_MAX_CONFIG_FILE_LINE_LEN+1];
	char		*valp = NULL;
	int		i = 0;
	int		c = 0;


	if(ebufp == NULL) ebufp = &errbuf;
	else if (PIN_ERR_IS_ERR( ebufp )) return;

	PIN_ERR_CLEAR_ERR(ebufp);

	memset(macro, 0, CM_FM_MAX_CONFIG_FILE_LINE_LEN+1);

	/*************************************************************
	 * Read from input file until we either hit the end of file
	 * or the closing curly brace for end of macro.
	 *************************************************************/

	c = fgetc(fp);
	i=0;
	while( !feof(fp) && c != '}' && i < CM_FM_MAX_CONFIG_FILE_LINE_LEN ) {
		macro[i++] = c;
		c = fgetc(fp);
	}

	macro[i] = '\0';

	/*************************************************************
	 * Price Plan Name
	 *************************************************************/

	if  (strcasecmp( macro, "price_plan")==0) {
		valp = NULL;
		valp = (char *)PIN_FLIST_FLD_GET( in_flistp, 
				PIN_FLD_AAC_PACKAGE, 0, ebufp);
		if(valp) fm_cust_pol_buf_cat( bufp, valp, ebufp);
	}

	/*************************************************************
	 * Promo code
	 *************************************************************/

	else if (strcasecmp( macro, "promo_code")==0) {
		valp = NULL;
		valp = (char *)PIN_FLIST_FLD_GET( in_flistp, 
				PIN_FLD_AAC_PROMO_CODE, 0, ebufp);
		if(valp) fm_cust_pol_buf_cat( bufp, valp, ebufp);
	}

	/*************************************************************
	 * Price plan description
	 *************************************************************/

	else if (strcasecmp( macro, "plan_description")==0) {
		valp = NULL;
		valp = (char *)PIN_FLIST_FLD_GET( in_flistp, 
				PIN_FLD_DESCR, 0, ebufp);
		if(valp) fm_cust_pol_buf_cat( bufp, valp, ebufp);
	}

	/*************************************************************
	 * Unrecognized macro
	 *************************************************************/
	else {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
			"unrecognized macro");
	}

	if(PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR,
			"err substituting intro file macro", ebufp);
		goto ErrOut;
	}

	return;
ErrOut:
	return;
}



