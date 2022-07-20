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
static  char Sccs_Id[] = "@(#)%Portal Version: fm_cust_pol_get_config.c:BillingVelocityInt:3:2006-Sep-05 04:30:13 %";
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
#include <ctype.h>

/*******************************************************************
 * Contains the PCM_OP_CUST_POL_GET_CONFIG operation.
 *
 * Returns configuration information to be sent out to clients
 * during on-line registration at the end of the session so that
 * they can configure themselves. The results of this op become
 * the output of the CUST_CREATE_CUSTOMER op when all goes well.
 *******************************************************************/

/*******************************************************************
 * Routines contained herein.
 *******************************************************************/
EXPORT_OP void
op_cust_pol_get_config(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp);

static void
fm_cust_pol_get_config(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistpp,
        pin_errbuf_t		*ebufp);

static void
fm_cust_pol_read_config(
	pin_flist_t		*r_flistp,
	pin_flist_t		*in_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_cust_pol_get_config_stuff(
	pin_flist_t		*r_flistp,
	char			*key,
	char			*val,
	pin_errbuf_t		*ebufp);

static void
fm_cust_pol_get_config_parse_line(
	FILE			*fp,
	char			**keypp,
	char			**valuepp,
	u_int			*donep,
	pin_errbuf_t		*ebufp);

static void
fm_cust_pol_get_config_skip_blanks(
 	FILE			*fp);


/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_GET_CONFIG operation.
 *******************************************************************/
void
op_cust_pol_get_config(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*r_flistp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_CUST_POL_GET_CONFIG) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_cust_pol_get_config",  ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What did we get?
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_cust_pol_get_config input flist", in_flistp);

	/***********************************************************
	 * Call main function to do it
	 ***********************************************************/
	fm_cust_pol_get_config(ctxp, in_flistp, &r_flistp, ebufp);

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		*ret_flistpp = (pin_flist_t *)NULL;
		PIN_FLIST_DESTROY(r_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_cust_pol_get_config error", ebufp);
	} else {
		*ret_flistpp = r_flistp;
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_get_config return flist", r_flistp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_get_config()
 *
 *	Policy that takes the incoming flist of info contained
 *	on an srvc array and other, and adds more information and
 *	sends out the config information in big flist.
 *
 *******************************************************************/
static void
fm_cust_pol_get_config(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistpp,
        pin_errbuf_t		*ebufp)
{
	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Copy the incoming flist.
	 ***********************************************************/
	*out_flistpp = PIN_FLIST_COPY(in_flistp, ebufp);

	/***********************************************************
	 * Add the file configed parameters.
	 ***********************************************************/
	fm_cust_pol_read_config(*out_flistpp, in_flistp, ebufp);

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if(PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_get_config error", ebufp);
	}

	return;
}


/*******************************************************************
 * Routines for parsing the config parameters file.
 *******************************************************************/
/*******************************************************************
 * fm_cust_pol_read_config():
 *
 *	Read the config file to obtain general purpose config params.
 *
 *******************************************************************/
static void
fm_cust_pol_read_config(
	pin_flist_t		*r_flistp,
	pin_flist_t		*in_flistp,
	pin_errbuf_t		*ebufp)
{
	FILE			*fp = NULL;

	char			*fname = NULL;
	char			*path = NULL;
	char			*access = NULL;
        char                    conffile[PIN_MAXPATHLEN];

	u_int			done = 0;
	char			*key = NULL;
	char			*val = NULL;

	int32			perr;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/*************************************************************
	 * Obtain config_dir directory location
	 *************************************************************/
        pin_conf("fm_cust_pol",  "config_dir", PIN_FLDT_STR,
                        (caddr_t *)&path, &perr);
        switch (perr) {
        case PIN_ERR_NONE:
                break;
        default:
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_INVALID_CONF, 0, 0, perr);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"config_dir configuration error",ebufp);
		return;
        }

	/*************************************************************
	 * Obtain the filename (mecca = aac_source, default otherwise)
	 *************************************************************/
	access = (char *)PIN_FLIST_FLD_GET(in_flistp,
		PIN_FLD_AAC_ACCESS, 1, ebufp);

	if ((access != (char *)NULL) &&
		(!strcmp(access, PIN_CUST_AAC_ACCESS_MECCAD))) {

		fname = (char *)PIN_FLIST_FLD_GET(in_flistp,
			PIN_FLD_AAC_SOURCE, 0, ebufp);

	} else {

		fname = "default";
	}

	/*************************************************************
	 * path would never be NULL because we checked the pin_conf error
	 * we check it any way, but no else for pin_set_err
	 *************************************************************/

	if (path) {
		pin_snprintf(conffile,sizeof(conffile), "%s/%s.config", path, fname);
	
		/*************************************************************
		 * Open the config_file file for reading
		 *************************************************************/
		fp = fopen(conffile, "r");
		if(fp == NULL) {
	
			pin_snprintf(conffile,sizeof(conffile), "%s/default.config", path);
			fp = fopen(conffile, "r");
	
			if(fp == NULL) {
	                	pin_set_err(ebufp, PIN_ERRLOC_FM,
	                        	PIN_ERRCLASS_SYSTEM_DETERMINATE,
	                        	PIN_ERR_FILE_IO, 0, 0, 0);
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"error opening config file", ebufp);
				free(path);
				return;
			}
		}
		free(path);
	}


	/*************************************************************
	 * Read until EOF
	 *************************************************************/
	done = 0;
	while( ! feof(fp) && done != 1) {

		/**********************************************
		 * read next data line from file, returned
		 * malloc'd strings with key and value
		 **********************************************/
		fm_cust_pol_get_config_parse_line(fp, &key, &val, &done, ebufp);
		if(PIN_ERR_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"Err reading config file", ebufp);
			goto ErrOut;
		}

		if( done != 1) {

			/***************************************
			 * Stuff the configuration parameter
			 * into outgoing flist.
			 ***************************************/

			fm_cust_pol_get_config_stuff(r_flistp, key, val, ebufp);
			if(PIN_ERR_IS_ERR(ebufp)) {
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"Err reading config file", ebufp);
				goto ErrOut;
			}
		}

		/**********************************************
		 * Free memory just malloc'd while parsing line
		 **********************************************/

		if(key) free(key);
		key = NULL;
		if(val) free(val);
		val = NULL;

	} /* End while */

ErrOut:
	if(key) free(key);
	if(val) free(val);
	if(fp) fclose(fp);
	return;
}


/******************************************************
 * Stuff a config parameter into outgoing flist, 
 * depending on string value
 ******************************************************/
static void
fm_cust_pol_get_config_stuff(
	pin_flist_t		*r_flistp,
	char			*key,
	char			*val,
	pin_errbuf_t		*ebufp)
{
	pin_errbuf_t		errbuf;
	u_int			type;
	pin_flist_t		*flistp = NULL;
	u_int			port = 0;

	if(ebufp == NULL) ebufp = &errbuf;

	PIN_ERR_CLEAR_ERR(ebufp);

	if(key == NULL) {
		return;
	}

	/*
	 * If DNS Address param, then create host array entry
	 */
	else if( strcmp( key, "dns_address_1")==0) {
		flistp = PIN_FLIST_ELEM_ADD(r_flistp, PIN_FLD_HOST, 1, ebufp);

		PIN_FLIST_FLD_SET(flistp, PIN_FLD_HOSTNAME, (void *)val, ebufp);

		type = PIN_HOST_DNS;
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_TYPE, (void *)&type, ebufp);
	}

	else if (strcmp(key, "dns_address_2") == 0) {
		flistp = PIN_FLIST_ELEM_ADD(r_flistp, PIN_FLD_HOST, 2, ebufp);

		PIN_FLIST_FLD_SET(flistp, PIN_FLD_HOSTNAME, (void *)val, ebufp);

		type = PIN_HOST_DNS;
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_TYPE, (void *)&type, ebufp);
	}

	/*
	 * If SMTP Hostname, then look to see if already a
	 * host array elem yet or not.  We will count on it
	 * being in elemid 3, although this is not a hard
	 * and fast rule except for only within this policy
	 * call.  It may have already been setup if the smtp_port
	 * parameter was previously found.
	 */

	else if (strcmp(key, "smtp_hostname") == 0) {
		flistp = PIN_FLIST_ELEM_GET(r_flistp,
			PIN_FLD_HOST, 3, 1, ebufp);

		if (flistp == (pin_flist_t *)NULL) {
			flistp = PIN_FLIST_ELEM_ADD( r_flistp, 
				PIN_FLD_HOST, 3, ebufp);

			type = PIN_HOST_SMTP;
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_TYPE,
				(void *)&type, ebufp);
		}

		PIN_FLIST_FLD_SET(flistp, PIN_FLD_HOSTNAME, (void *)val, ebufp);
	}

	/*
	 * If SMTP port, then look to see if already a
	 * host array elem yet or not.  We will count on it
	 * being in elemid 3, although this is not a hard
	 * and fast rule except for only within this policy
	 * call.  It may have already been setup if the smtp_host
	 * parameter was previously found.
	 */
	else if (strcmp(key, "smtp_port") == 0) {
		flistp = PIN_FLIST_ELEM_GET(r_flistp,
			PIN_FLD_HOST, 3, 1, ebufp);

		if (flistp == (pin_flist_t *)NULL) {
			flistp = PIN_FLIST_ELEM_ADD(r_flistp, 
				PIN_FLD_HOST, 3, ebufp);

			type = PIN_HOST_SMTP;
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_TYPE,
				(void *)&type, ebufp);
		}

		port = atoi(val);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_PORT, (void *)&port, ebufp);

	}

	/*
	 * NNTP Hostname
	 */
	else if (strcmp(key, "nntp_hostname") == 0) {
		flistp = PIN_FLIST_ELEM_ADD(r_flistp, PIN_FLD_HOST, 4, ebufp);

		PIN_FLIST_FLD_SET(flistp, PIN_FLD_HOSTNAME, (void *)val, ebufp);

		type = PIN_HOST_NNTP;
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_TYPE, (void *)&type, ebufp);
	}

	/*
	 * Pop Host
	 */
	else if (strcmp(key, "pop_hostname") == 0) {
		flistp = PIN_FLIST_ELEM_ADD(r_flistp, PIN_FLD_HOST, 5, ebufp);

		PIN_FLIST_FLD_SET(flistp, PIN_FLD_HOSTNAME, (void *)val, ebufp);

		type = PIN_HOST_POP;
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_TYPE, (void *)&type, ebufp);	
	}

	/*
	 * Http URL
	 */
	else if (strcmp(key, "http_url") == 0) {
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_HTTP_URL,
			(void *)val, ebufp);
	}

	/*
	 * Support number
	 */
	else if (strcmp(key, "support_number") == 0) {
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_SUPPORT_PHONE,
			(void *)val, ebufp);
	}


	return;
/*ErrOut:
	return;*/
}


/*****************************************************************
 * Parse a data line from config_file. returned malloc'd key
 * and val strings
 *****************************************************************/
static void
fm_cust_pol_get_config_parse_line(
	FILE			*fp,
	char			**keypp,
	char			**valuepp,
	u_int			*donep,
	pin_errbuf_t		*ebufp)
{
	int			i = 0;
	int			c = 0;
	int			done = 0;
	pin_errbuf_t		errbuf;
	char			kbuf[CM_FM_MAX_CONFIG_FILE_LINE_LEN];
	char			vbuf[CM_FM_MAX_CONFIG_FILE_LINE_LEN];

	if(ebufp == NULL) ebufp = &errbuf;

	PIN_ERR_CLEAR_ERR(ebufp);

	/************************************************************
	 * Free and clear the input pointer pointers for key and value
	 *************************************************************/

	if(*keypp) free(*keypp);
	if(*valuepp) free(*valuepp);
	*keypp = NULL;
	*valuepp = NULL;
	memset( vbuf, 0, CM_FM_MAX_CONFIG_FILE_LINE_LEN );
	memset( kbuf, 0, CM_FM_MAX_CONFIG_FILE_LINE_LEN );

	/************************************************************
	 * First get the identifier before the equal sign 
	 *************************************************************/

	/***************************************************************
         * skip leading spaces, blank lines and comments
	 ***************************************************************/

	fm_cust_pol_get_config_skip_blanks(fp);
	c = fgetc(fp);

	/**********************************************
	 * If the current char is an EOF, then that means
	 * we are simply at the end of the file, no 
	 * error.
	 **********************************************/

	if(c == EOF) {
		*donep = 1;
		goto DoneOut;
	}

	/**********************************************
	 * Read the key in until '=' sign 
	 **********************************************/

	done=0;
	i=0;
	while( 0 == done && i < CM_FM_MAX_CONFIG_FILE_LINE_LEN ) {
		switch(c) {
		case '=':
			done=1;
			break;

		case EOF:
		case '\n':
			/**************************************
			 * Since we are halfway through a key
			 * this is error
			 **************************************/

			pin_set_err(ebufp, PIN_ERRLOC_FM, 
				PIN_ERRCLASS_SYSTEM_DETERMINATE, 
				PIN_ERR_FILE_IO, 0, 0, 0); 
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"error parsing config_file", ebufp);
			goto ErrOut;

		default:
			kbuf[i++] = c;
			break;
		}

		c = fgetc(fp);
	}

	if ( i >= CM_FM_MAX_CONFIG_FILE_LINE_LEN ) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE, 
			PIN_ERR_FILE_IO, 0, 0, 0); 
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"error parsing config_file buffer too long", ebufp);
		goto ErrOut;
	}

	/*****************************************************
	 * Now strip trailing spaces from the key 
	 *****************************************************/

	i--;
	while(isspace(kbuf[i])) {
		kbuf[i--] = '\0';
	}

	/********************************************************
	 * Ok, now everything after the '=' sign is the value 
	 ********************************************************/

	/***************************************************
	 * First check for a '\n' or EOF that would signify
	 * null arg
	 ***************************************************/

	if( c == '\n' || c == EOF ) {
		goto NullOut;
	}

	/****************************
	 * Skip leading space 
	 ****************************/

	while(isspace(c) && c != '\n' ) {
		c = fgetc(fp);
	}

	/************************************************************
	 * Read in the string up until EOF or LINE END 
	 *************************************************************/

	done = 0;
	i = 0;
	while( 0 == done && i < CM_FM_MAX_CONFIG_FILE_LINE_LEN ) {
		switch(c) {
		case EOF:
			done = 1;
			break;
		case '\n':
			done = 1;
			break;

		default:
			vbuf[i++] = c;
			break;
		}

		if( ! done) {
			c = fgetc(fp);
		}
	}

	if ( i >= CM_FM_MAX_CONFIG_FILE_LINE_LEN ) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE, 
			PIN_ERR_FILE_IO, 0, 0, 0); 
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"error parsing config_file line too long", ebufp);
		goto ErrOut;
	}

	/************************************************************
	 * Strip trailing spaces
	 *************************************************************/

	i--;
	while(isspace(vbuf[i])) {
		vbuf[i--] = '\0';
	}

NullOut:
	/************************************************************
	 * Copy the bufs to the output char buffers
	 *************************************************************/

	if(kbuf) {
		*keypp = (char *)strdup( kbuf );

	}

	if(vbuf) {
		*valuepp = (char *)strdup( vbuf );
	}

	return;
DoneOut:
	*keypp = NULL;
	*valuepp = NULL;
	return;

ErrOut:
	*keypp = NULL;
	*valuepp = NULL;
	return;
}


/******************************************************
 * skip leading spaces, blank lines and comments
 ******************************************************/
static void
fm_cust_pol_get_config_skip_blanks(
 	FILE		*fp)
{
	int		c;

	c = fgetc(fp);
	if(c == '#') {
		c = fgetc(fp);
		while( c != EOF && c != '\n') {
			c = fgetc(fp);
		}
	}

	while(isspace(c) ) {
		c = fgetc(fp);
		if(c == '#') {
			c = fgetc(fp);
			while( c != EOF && c != '\n') {
				c = fgetc(fp);
			}
		}
	}

	/***********************************
	 * Put last character back on stream
	 ***********************************/

	if( c != EOF) ungetc(c, fp);

	return;
}
