/*******************************************************************
 *
* Copyright (c) 1996, 2015, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char Sccs_Id[] = "@(#)$Id: fm_cust_pol_post_commit.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 04:29:30 nishahan Exp $";
#endif

#include <stdio.h>
#include "pin_os_dir.h"

#include <strings.h>
#include <stdlib.h>
#include <ctype.h>

#ifndef MSDOS
#include <sysexits.h>
#else
#define EX_OK 0
#endif

#include "pcm.h"
#include "ops/delivery.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_mail.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pinlog.h"
#include "pin_os_string.h"

/*******************************************************************
 * Routines contained herein.
 *******************************************************************/
EXPORT_OP void
op_cust_pol_post_commit(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp);

static void
fm_cust_pol_post_commit(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistpp,
        pin_errbuf_t		*ebufp);

static void 
fm_cust_pol_post_commit_email(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_errbuf_t	*ebufp);


static void 
fm_cust_pol_post_commit_email_config(
	pin_flist_t		*i_flistp,
	pin_flist_t		*e_flistp,
	pin_errbuf_t		*ebufp);

static void 
fm_cust_pol_post_commit_email_parse(
        pcm_context_t           *ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*e_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_cust_pol_post_commit_email_macro(
        pcm_context_t           *ctxp,
	FILE		*fp,
	pin_flist_t	*in_flistp,
	pin_cust_buf_t	*bufp,
	pin_errbuf_t	*ebufp);

static void
fm_cust_pol_post_commit_macro_srvc_fld(
	pin_flist_t		*in_flistp,
	pin_cust_buf_t		*bufp,
	u_int			fldno,
	char			*type,
	pin_errbuf_t		*ebufp);

static void
fm_cust_pol_post_commit_email_send(
	pcm_context_t	*ctxp,
	pin_flist_t	*e_flistp,
	pin_errbuf_t	*ebufp);



/*******************************************************************
 * Routines needed from elsewhere.
 *******************************************************************/
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


/*******************************************************************
 * Defines specific to this module.
 *******************************************************************/
#define FM_CUST_POL_DEFAULT_MAILFILE	"default.welcome"

/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_POST_COMMIT  command
 *******************************************************************/
void
op_cust_pol_post_commit(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*i_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;

	/***********************************************************
	 * Null out results until we have some.
	 ***********************************************************/
	*ret_flistpp = NULL;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_CUST_POL_POST_COMMIT) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_cust_pol_post_commit", ebufp);
		return;
	}

	/***********************************************************
         * Debug what we got.
         ***********************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "op_cust_pol_post_commit input flist", in_flistp);

	/***********************************************************
	 * We will not open any transactions with Policy FM
	 * since policies should NEVER modify the database.
	 ***********************************************************/


	/***********************************************************
	 * Call main function to do it
	 ***********************************************************/
	i_flistp = PIN_FLIST_COPY(in_flistp, ebufp);
	fm_cust_pol_post_commit(ctxp, i_flistp, &r_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&i_flistp, NULL);

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		*ret_flistpp = (pin_flist_t *)NULL;
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_cust_pol_post_commit error", ebufp);
	} else {
		*ret_flistpp = r_flistp;
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_post_commit return flist", r_flistp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_post_commit()
 *
 *	Pre-creation legacy system hook.
 *
 *	Currently limited to sending welcome email.
 *
 *******************************************************************/
static void
fm_cust_pol_post_commit(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistpp,
        pin_errbuf_t		*ebufp)
{
	void			*vp = NULL;
	int			*msg_flgp = NULL;
	int			msg_flag = 0;
	
	

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Create outgoing flist
	 ***********************************************************/
	*out_flistpp = PIN_FLIST_CREATE(ebufp);

	vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_POID, vp, ebufp);

	/*
	 * Check of the new_account_welcome_msg flag is set 
	 */
        pin_conf("fm_cust_pol", "new_account_welcome_msg", 
		PIN_FLDT_INT, (caddr_t *)&msg_flgp, &(ebufp->pin_err)); 

	if (msg_flgp) {
		msg_flag = *msg_flgp;
		free(msg_flgp);
	}

        switch (ebufp->pin_err) {
        case PIN_ERR_NONE:
		if (msg_flag != 0) {
			break;
		}
		/*
		 * fall through.
		 */
	case PIN_ERR_NOT_FOUND:
		/*
		 * Default is not to send welcome message 
		 * upon new account creation
		 */
		PIN_ERR_CLEAR_ERR(ebufp);
		return;
        default:
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_INVALID_CONF, 0, 0, ebufp->pin_err);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"new_account_welcome_msg configuration error", ebufp);
		return;
        }
		
	/***********************************************************
	 * Send welcome email.
	 ***********************************************************/
	fm_cust_pol_post_commit_email(ctxp, in_flistp, ebufp);

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_post_commit error", ebufp);
	}

	return;
}

/*******************************************************************
 * Routines related to the sending of welcome email to new users.
 *******************************************************************/
/*******************************************************************
 * fm_cust_pol_post_commit_email():
 *
 *	Send a 'welcome' email to the newly created account.
 *
 *******************************************************************/
static void 
fm_cust_pol_post_commit_email(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*e_flistp = NULL;
	void		*vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Allocate the flist for the email (parameters).  Set poid.
	 ***********************************************************/
	e_flistp = PIN_FLIST_CREATE(ebufp);

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(e_flistp, PIN_FLD_POID, vp, ebufp);

	/***********************************************************
	 * Get the email parameters.
	 ***********************************************************/
	fm_cust_pol_post_commit_email_config(i_flistp, e_flistp, ebufp);

	/***********************************************************
	 * Parse the desired email file.
	 ***********************************************************/
	fm_cust_pol_post_commit_email_parse(ctxp, i_flistp, e_flistp, ebufp);

	/***********************************************************
	 * Send the email.
	 ***********************************************************/
	fm_cust_pol_post_commit_email_send(ctxp, e_flistp, ebufp);

	/***********************************************************
	 * Clean up.
	 ***********************************************************/
	/* Free local memory */
	PIN_FLIST_DESTROY_EX(&e_flistp, NULL);

	/* Error? */
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_post_commit_email error", ebufp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_post_commit_email_config():
 *
 *	Collect the basic configuration information for sending
 *	the welcome email. This includes putting together the
 *	sender and recipient addresses, setting the email subject
 *	and deriving the filename of the email template to send.
 *
 *******************************************************************/
static void 
fm_cust_pol_post_commit_email_config(
	pin_flist_t		*i_flistp,
	pin_flist_t		*e_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_cookie_t		cookie = NULL;
	pin_flist_t		*flistp = NULL;
	int			elemid = 0;

	char			*path = NULL;
	char			*fname = NULL;
	char			*subject = NULL;
	char			*domain = NULL;
	char			*sender = NULL;
	char			*suffix = NULL;

	char			from[1024];
	char			logout[1024];
	char			sname[1024];

	poid_t			*s_pdp = NULL;
	const char		*type = NULL;
	char			*login = NULL;
	char			*ptr = NULL;
	char			*aac_access = NULL;
	u_int			i = 0;

	from[0] = logout[0] = sname[0] = '\0';

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Obtain welcome email file directory (PIN_FLD_DIRECTORY)
	 ***********************************************************/
        pin_conf("fm_cust_pol", "welcome_dir", PIN_FLDT_STR,
		(caddr_t *)&path, &(ebufp->pin_err));

        switch (ebufp->pin_err) {
        case PIN_ERR_NONE:
                break;
	case PIN_ERR_NOT_FOUND:
		/* not configured */
		PIN_ERR_CLEAR_ERR(ebufp);
		return;
        default:
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_INVALID_CONF, 0, 0, ebufp->pin_err);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"welcome_dir configuration error", ebufp);
		return;
        }

	PIN_FLIST_FLD_PUT(e_flistp, PIN_FLD_DIRECTORY, (void *)path, ebufp);

	/***********************************************************
	 * Sender (Who is our postmaster?)
	 ***********************************************************/
	/***********************************************************
	 * We will have an entry for the postmaster in pin.conf file
	 ***********************************************************/
	pin_conf("fm_cust_pol", "sender", PIN_FLDT_STR,
		(caddr_t *)&sender, &(ebufp->pin_err));

	switch (ebufp->pin_err) {
	case PIN_ERR_NONE:
		/* copy this */
		if (sender != (char *)NULL) {
			if (sizeof(sname) < strlen(sender) + 1) { 
				/* a partial strcpy is better than no copy at all */
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_WARNING, "sender name truncated");
			}
			pin_strlcpy(sname, sender, sizeof(sname));
			free(sender);
		}
		break;
	case PIN_ERR_NOT_FOUND:
		/* Not Configured */
		/* Since this is a new field that we are looking
		 * for in the CM pin.conf file we will initialize
		 * this with a default string "postmaster" as was
		 * done previously
		 */
		strcpy(sname,"postmaster");
		PIN_ERR_CLEAR_ERR(ebufp);
		break;
	default:
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_INVALID_CONF, 0, 0, ebufp->pin_err);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"sender (domain) configuration error", ebufp);
		return;
	}
	
	pin_conf("fm_cust_pol", "domain", PIN_FLDT_STR,
		(caddr_t *)&domain, &(ebufp->pin_err));

	switch (ebufp->pin_err) {
	case PIN_ERR_NONE: {
		int len = strlen(sname);
		int size = sizeof(from) - 1;
		char *from_p = from;

		if (size < len) {
			len = size;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_WARNING, "sender name truncated");
		}
		if (len > 0) {
			strncpy(from_p, sname, len);
			from_p += len;
			size -= len;
		}
		if (size > 0) {
			from_p[0] = '@';
			from_p++;
			size --;
		}
		len = strlen(domain);
		if (size < len) {
			len = size;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_WARNING, "domain name truncated");
		}
		if (len > 0) {
			strncpy(from_p, domain, len);
			from_p += len;
			size -= len;
		}
		from_p[0] = '\0';

		PIN_FLIST_FLD_SET(e_flistp, PIN_FLD_SENDER,
			(void *)from, ebufp);
		} break;
	case PIN_ERR_NOT_FOUND:
		/* not configured */
		PIN_ERR_CLEAR_ERR(ebufp);
		break;
	default:
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_INVALID_CONF, 0, 0, ebufp->pin_err);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"sender (domain) configuration error", ebufp);
		return;
	}

	/***********************************************************
	 * Mailfile (PIN_FLD_FILENAME)
	 ***********************************************************/
	aac_access = (char *)PIN_FLIST_FLD_GET(i_flistp,
		PIN_FLD_AAC_ACCESS, 1, ebufp);

	if ((aac_access != (char *)NULL) &&
		(!strcmp(aac_access, PIN_CUST_AAC_ACCESS_MECCAD))) {

		fname = (char *)PIN_FLIST_FLD_GET(i_flistp,
			PIN_FLD_AAC_SOURCE, 1, ebufp);

	}
	else {
		fname = "default";
	}

	if (fname != (char *)NULL) {

		suffix=".welcome";
		path = (char *)malloc(strlen(fname) + strlen(suffix) + 5);
		if ( !path ) {
                	pin_set_err(ebufp, PIN_ERRLOC_FM,
                        	PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_NO_MEM, 0, 0, ebufp->pin_err);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"malloc failed configuration error", ebufp);
			return;
		}
		memset(path, 0, strlen(fname) + strlen(suffix) + 5);
		strcpy( path, fname);
		strcat(path, suffix);

		PIN_FLIST_FLD_PUT(e_flistp, PIN_FLD_FILENAME,
			(void *)path, ebufp);
	}

	/***********************************************************
	 * Recipients
	 ***********************************************************/
	flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_SERVICES,
		&elemid, 1, &cookie, ebufp);
	while(flistp != (pin_flist_t *)NULL) {

		/***************************************************
		 * What service is this srvc for?
		 ***************************************************/
		s_pdp = (poid_t *)PIN_FLIST_FLD_GET(flistp,
			PIN_FLD_SERVICE_OBJ, 0, ebufp);
		if(s_pdp) type = PIN_POID_GET_TYPE(s_pdp);

		/***************************************************
		 * Save this recipient.
		 * (For now, cheat and dupe srvc array w/ login)
		 ***************************************************/
		if((type != (char *)NULL) && !strcmp(type, "/service/email")) {

			login = (char *)PIN_FLIST_FLD_GET(flistp,
				PIN_FLD_LOGIN, 0, ebufp);

			if (domain != (char *)NULL && login != (char *)NULL) {

				/* attach correct domain */
				strcpy(logout, login);

				/* Force login to lower case */
				i=0;
				while(logout[i] != '\0') {
					logout[i] = tolower(logout[i]);
					i++;
				}

				ptr = (char *)strchr(logout, '@');
				if (ptr != (char *)NULL) {
					*ptr = '\0';
				}

				strcat(logout, "@");
				strcat(logout, domain);
				PIN_FLIST_FLD_SET(flistp, PIN_FLD_LOGIN, 
					(void *)logout, ebufp);

			} 

			PIN_FLIST_ELEM_SET(e_flistp, flistp,
				PIN_FLD_RECIPIENTS, elemid, ebufp);
		}

		/***************************************************
		 * Get the next recipient.
		 ***************************************************/
		flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_SERVICES,
			&elemid, 1, &cookie, ebufp);
	}


	/***********************************************************
	 * Subject
	 ***********************************************************/
	subject = (char *)"Welcome to the Internet";
	PIN_FLIST_FLD_SET(e_flistp, PIN_FLD_SUBJECT, (void *)subject, ebufp);

	/***********************************************************
	 * Clean up.
	 ***********************************************************/
	/* Free local memory */
	if (domain != (char *)NULL) {
		free(domain);
	}

	/* Error? */
	if(PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_post_commit_email_config error", ebufp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_post_commit_email_parse():
 *******************************************************************/
static void 
fm_cust_pol_post_commit_email_parse(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*e_flistp,
	pin_errbuf_t		*ebufp)
{
	char			*dir = NULL;
	char			*filename = NULL;
        char                    mailfile[PIN_MAXPATHLEN];
	pin_buf_t		*pin_bufp = NULL;

	FILE			*fp = NULL;
	pin_cust_buf_t		*bufferp = NULL;
	int			c = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * What template file do we use?
	 ***********************************************************/
	dir = (char *)PIN_FLIST_FLD_GET(e_flistp, PIN_FLD_DIRECTORY, 1, ebufp);

	if (dir == (char *)NULL) {
		PIN_ERR_CLEAR_ERR(ebufp);
		return;
	}

	filename = (char *)PIN_FLIST_FLD_GET(e_flistp,
		PIN_FLD_FILENAME, 1, ebufp);
	if (filename == (char *)NULL) {
		filename = FM_CUST_POL_DEFAULT_MAILFILE;
	}

	/***********************************************************
	 * Open the welcome template for reading.
	 ***********************************************************/
	pin_snprintf(mailfile, sizeof(mailfile), "%s/%s", dir, filename);

	fp = fopen(mailfile, "r");
	if (fp == NULL) {

		pin_snprintf(mailfile, sizeof(mailfile), "%s/%s", dir, FM_CUST_POL_DEFAULT_MAILFILE);
		fp = fopen(mailfile, "r");

		if (fp == NULL) {
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_FILE_IO, 0, 0, 0);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_WARNING,
				"error opening welcome mailfile", ebufp);
			goto errout;
		}

	}

	/***********************************************************
	 * Read until EOF
	 ***********************************************************/
	fm_cust_pol_buf_create(&bufferp, ebufp);
	if ( PIN_ERR_IS_ERR(ebufp) ) {
		goto errout;
	}

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
				fm_cust_pol_post_commit_email_macro( ctxp,
					fp, i_flistp, bufferp, ebufp);
			}
			else {
				/********************************************
				 * Turned out to not be a macro, so use
				 * both characters as is.
				 ********************************************/
				fm_cust_pol_buf_put(bufferp, '$', ebufp);
				fm_cust_pol_buf_put(bufferp, (char)c, ebufp);
			}
			break;

		case EOF:
			break;

		default:
			/********************************************
			 * Use this character
			 ********************************************/
			fm_cust_pol_buf_put(bufferp, (char)c, ebufp);
		}

		c = fgetc(fp);

	} /* End while */

	/***********************************************************
	 * Save the formatted letter.
	 ***********************************************************/
	pin_bufp = (pin_buf_t *)malloc(sizeof(pin_buf_t));
        if (pin_bufp == NULL) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_NO_MEM, 0, 0,
                        PCM_OP_CUST_POL_POST_COMMIT);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "Memory for pin_bufp couldn't be allocated", ebufp);
                goto errout;
        }

	pin_bufp->flag = 0;
	pin_bufp->offset = 0;
	pin_bufp->size = strlen(bufferp->bufp) + 1;
	pin_bufp->data = bufferp->bufp;
	pin_bufp->xbuf_file = (char *)0;

	PIN_FLIST_FLD_PUT(e_flistp, PIN_FLD_LETTER, (void *)pin_bufp, ebufp);

errout:
	/***********************************************************
	 * Clean up
	 ***********************************************************/
	/* Close the file template */
	if ( fp ) {
		fclose(fp);
	}
	if ( bufferp ) {
		pin_free(bufferp);
	}

	/* Error? */
	if(PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_post_commit_email_parse error", ebufp);
	}
	return;
}

/*************************************************************
 * This function handles macro substitution.  It assumes
 * that before calling this function the calling function
 * had already detected the characters '$' and '{' which
 * indicate a macro.  Then it reads until it sees a '}',
 * and tries the match the macro name and lookup the value
 * from the supplied in_flistp.
 *************************************************************/
static void
fm_cust_pol_post_commit_email_macro(
	pcm_context_t	*ctxp,
	FILE		*fp,
	pin_flist_t	*in_flistp,
	pin_cust_buf_t	*bufp,
	pin_errbuf_t	*ebufp)
{
	pin_errbuf_t	errbuf;
	char		macro[CM_FM_MAX_CONFIG_FILE_LINE_LEN+1];
	char		poid_buf[PCM_MAX_POID_TYPE + 1];
	int32		len = 0;
	char		*acct_num = NULL;
	char		*plan = NULL;
	poid_t		*poidp = NULL;
	int		i = 0;
	int		c = 0;
	char		*p = NULL;
	pin_flist_t	*a_flistp = NULL;
	pin_flist_t	*r_flistp = NULL;

	if(ebufp == NULL) ebufp = &errbuf;
	else if (PIN_ERR_IS_ERR( ebufp )) return;

	PIN_ERR_CLEAR_ERR(ebufp);

	memset(macro, 0, CM_FM_MAX_CONFIG_FILE_LINE_LEN+1);
	memset( poid_buf, 0, sizeof( poid_buf ) );

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
	 * If Account_name
	 *************************************************************/

	if     (strcasecmp( macro, "account_name")==0) {
		fm_cust_pol_post_commit_macro_srvc_fld( in_flistp, bufp, 
			PIN_FLD_LOGIN, "/service/ip", ebufp);
	}

	/*************************************************************
	 * Account Password
	 *************************************************************/

	else if (strcasecmp( macro, "account_password")==0) {
		fm_cust_pol_post_commit_macro_srvc_fld( in_flistp, bufp, 
			PIN_FLD_PASSWD_CLEAR, "/service/ip", ebufp);
	}

	/*************************************************************
	 * Email_name
	 *************************************************************/

	else if (strcasecmp( macro, "email_name")==0) {
		fm_cust_pol_post_commit_macro_srvc_fld( in_flistp, bufp, 
			PIN_FLD_LOGIN, "/service/email", ebufp);
	}

	/*************************************************************
	 * Email_password
	 *************************************************************/

	else if (strcasecmp( macro, "email_password")==0) {
		fm_cust_pol_post_commit_macro_srvc_fld( in_flistp, bufp, 
			PIN_FLD_PASSWD_CLEAR, "/service/email", ebufp);
	}

	/*************************************************************
	 * Acccount Poid
	 *************************************************************/

	else if (strcasecmp( macro, "acct_poid") ==0) {

		poidp = (poid_t *)PIN_FLIST_FLD_GET( in_flistp, 
			PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
		if(poidp) {
			len = sizeof( poid_buf );
			p = poid_buf;
			PIN_POID_TO_STR( poidp, &p, &len, ebufp);
			fm_cust_pol_buf_cat( bufp, poid_buf, ebufp);
		}
	}

	/*************************************************************
	 * Account num when it is available
	 *************************************************************/

	else if (strcasecmp( macro, "account_no")==0) {
		acct_num = NULL;
		a_flistp = PIN_FLIST_CREATE( ebufp );

		/* Read account obj from db */
		poidp = (poid_t *)PIN_FLIST_FLD_GET( in_flistp, 
			PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
		PIN_FLIST_FLD_SET( a_flistp, PIN_FLD_POID, (void*)poidp, ebufp);
		PIN_FLIST_FLD_SET( a_flistp, PIN_FLD_ACCOUNT_NO, NULL, ebufp );
		PCM_OP( ctxp, PCM_OP_READ_FLDS, 0, a_flistp, &r_flistp, ebufp );

		/* get account number from account obj */
		acct_num = (char *)PIN_FLIST_FLD_GET( r_flistp, 
			PIN_FLD_ACCOUNT_NO, 0, ebufp);
		if(acct_num) fm_cust_pol_buf_cat( bufp, acct_num, ebufp);

		/* clean up */
		PIN_FLIST_DESTROY_EX( &a_flistp, ebufp );
		PIN_FLIST_DESTROY_EX( &r_flistp, ebufp );
	}

	/*************************************************************
	 * Price Plan Name
	 *************************************************************/

	else if ( strcasecmp( macro, "price_plan" ) == 0 ) {
		plan = (char *)PIN_FLIST_FLD_GET( in_flistp, 
				PIN_FLD_NAME, 1, ebufp);
		r_flistp = NULL;
		if ( !plan ) {
			a_flistp = PIN_FLIST_CREATE( ebufp );

			/* Read plan obj name from db */
			poidp = (poid_t *)PIN_FLIST_FLD_GET( in_flistp, 
						PIN_FLD_POID, 0, ebufp);
			PIN_FLIST_FLD_SET( a_flistp, PIN_FLD_POID, 
						(void*)poidp, ebufp);
			PIN_FLIST_FLD_SET( a_flistp, PIN_FLD_NAME, NULL, 
						ebufp );
			PCM_OP( ctxp, PCM_OP_READ_FLDS, 0, a_flistp, &r_flistp, 
				ebufp );

			PIN_FLIST_DESTROY_EX( &a_flistp, ebufp );
		
			/* get plan name from read */
			plan = (char *)PIN_FLIST_FLD_GET( r_flistp, 
							PIN_FLD_NAME, 0, ebufp);
	
			/* clean up */
		}
		if ( plan ) fm_cust_pol_buf_cat( bufp, plan, ebufp );
		PIN_FLIST_DESTROY_EX( &r_flistp, ebufp );
	}
	/*************************************************************
	 * Unrecognized macro
	 *************************************************************/
	else {
		PIN_ERR_LOG_MSG( PIN_ERR_LEVEL_DEBUG,
			"Unrecognized macro");
	}

	/*************************************************************
	 * Error?
	 *************************************************************/
	if(PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR,
			"err substituting welcome file macro", ebufp);
	}
	return;
}


/*************************************************************
 * This function handles a macro field that requires looking
 * it up out of the srvc array.
 *************************************************************/
static void
fm_cust_pol_post_commit_macro_srvc_fld(
	pin_flist_t		*in_flistp,
	pin_cust_buf_t		*bufp,
	u_int			fldno,
	char			*type,
	pin_errbuf_t		*ebufp)
{
	pin_errbuf_t		errbuf;
	pin_cookie_t		cookie = NULL;
	void			*vp = NULL;
	pin_flist_t		*srvc_flistp = NULL;
	int			elemid = 0;
	const char		*stype = NULL;

	if(ebufp == NULL) ebufp = &errbuf;
	else if (PIN_ERR_IS_ERR(ebufp)) return;

	PIN_ERR_CLEAR_ERR(ebufp);

	/******************************************************
	 * Loop through all the srvc arrays.  Look for the
	 * the one that has the service type matching the
	 * supplied arg and when found get the fld specified
	 * as an arg and add it to the buf we are accruing.
	 ******************************************************/

	cookie = NULL;
	srvc_flistp = PIN_FLIST_ELEM_GET_NEXT( in_flistp, 
			PIN_FLD_SERVICES, &elemid, 0, &cookie, ebufp);
	while ( ! PIN_ERR_IS_ERR( ebufp)) {

		/***********************************************
		 * Get the poid from the srvc array element
		 ***********************************************/

		vp = PIN_FLIST_FLD_GET( srvc_flistp, PIN_FLD_SERVICE_OBJ,
				0, ebufp);
		stype = PIN_POID_GET_TYPE( (poid_t *)vp);
		/***********************************************
		 * If the type matches, then get the fld and 
		 * concat it to the buffer
		 ***********************************************/

		if( stype && strcmp( stype, type ) == 0 ) {

			vp = NULL;
			vp = PIN_FLIST_FLD_GET( srvc_flistp, 
				fldno, 0, ebufp);

			if(vp) fm_cust_pol_buf_cat( bufp, (char *)vp, ebufp);
			goto DoneOut;
		}

		/***********************************************
		 * Get next srvc array element
		 ***********************************************/

		srvc_flistp = PIN_FLIST_ELEM_GET_NEXT( in_flistp, 
			PIN_FLD_SERVICES, &elemid, 0, &cookie, ebufp);
	}
	switch(ebufp->pin_err) {
	case PIN_ERR_NONE:
		break;

	case PIN_ERR_NOT_FOUND:
		PIN_ERR_CLEAR_ERR(ebufp);
		break;
	default:
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR,
			"flist error", ebufp);
		goto ErrOut;
	}
	
DoneOut:
	return;
ErrOut:	
	return;
}


/*******************************************************************
 * fm_cust_pol_post_commit_email_send():
 *
 *	Our sendmail interface routine. Sends out the pre-formated
 *	email letter to the specified list of recipients. Will also
 *	set the sender address if specified.
 *
 *******************************************************************/
static void
fm_cust_pol_post_commit_email_send(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_cookie_t	cookie = NULL;
	pin_flist_t	*flistp = NULL;
	pin_flist_t	*e_flistp = NULL;
	pin_flist_t	*m_flistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	int		elemid;

	char		*sub = NULL;
	char		*from = NULL;
	char		*to = NULL;
	char		*login = NULL;
	pin_buf_t	*body = NULL;
	void		*vp = NULL;
	u_int		dummy = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * No recipients means nothing to do.
	 ***********************************************************/
	if (PIN_FLIST_ELEM_COUNT(i_flistp, PIN_FLD_RECIPIENTS, ebufp) == 0) {
		PIN_ERR_CLEAR_ERR(ebufp);
		return;
	}

	/***********************************************************
	 * Get the mail parameters.
	 ***********************************************************/
	sub = (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SUBJECT, 1, ebufp);
	from = (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SENDER, 1, ebufp);
	body = (pin_buf_t *)PIN_FLIST_FLD_GET(i_flistp,
		PIN_FLD_LETTER, 1, ebufp);
		
	/***************************************************
	 * Walk through the recipients.
	 ***************************************************/
	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_RECIPIENTS,
				&elemid, 1, &cookie, ebufp)) != NULL) {

		/***************************************************
		 * Add this recipient to the list.
		 ***************************************************/
		login = (char *)PIN_FLIST_FLD_GET(flistp,
			PIN_FLD_LOGIN, 1, ebufp);

		if (login == NULL) {
			continue;
		}

		if (to == (char *)NULL) {
			to = strdup(login);
		} else {
			char	*temp;

			/***************************************************
			 * Need to have more memory to hold more logins
			 ***************************************************/
			temp = realloc(to, strlen(to) + strlen(login) + 2);

			if (temp == NULL) {
				pin_set_err(ebufp, PIN_ERRLOC_FM,
					    PIN_ERRCLASS_SYSTEM_DETERMINATE,
					    PIN_ERR_NO_MEM, PIN_FLD_LOGIN,
					    0, 0);
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"fm_cust_pol_post_commit_email_send:"
					"can't realloc \"to\"", ebufp);
				free(to);
				return;
			}

			to = temp;
			
			strcat(to, " ");
			strcat(to, login);
		}
	}

	/***********************************************************
	 * Prepare mail flist and send the message.
	***********************************************************/
	m_flistp = PIN_FLIST_CREATE(ebufp);

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(m_flistp, PIN_FLD_POID, vp, ebufp);

	e_flistp = PIN_FLIST_ELEM_ADD(m_flistp, PIN_FLD_MESSAGES, 0, ebufp);
	PIN_FLIST_FLD_SET(e_flistp, PIN_FLD_SENDER, (void *)from, ebufp);
	PIN_FLIST_FLD_SET(e_flistp, PIN_FLD_SUBJECT, (void *)sub, ebufp);

	PIN_FLIST_FLD_SET(e_flistp, PIN_FLD_LETTER, (void *)body, ebufp);

	e_flistp = PIN_FLIST_ELEM_ADD(e_flistp, PIN_FLD_RECIPIENTS, 0, ebufp);
	dummy = PIN_MAIL_TO;
	PIN_FLIST_FLD_SET(e_flistp, PIN_FLD_TYPE, (void *)&dummy, ebufp);
	PIN_FLIST_FLD_SET(e_flistp, PIN_FLD_EMAIL_ADDR,
		(void *)to, ebufp);
	free(to);

	PCM_OP(ctxp, PCM_OP_DELIVERY_MAIL_SENDMSGS, 0, m_flistp, &r_flistp,
		ebufp);

	/***********************************************************
	 * Cleanup.
	 ***********************************************************/
	PIN_FLIST_DESTROY_EX(&m_flistp, NULL);
	if (r_flistp) {
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
        }

	/***********************************************************
	 * No errors.
	 ***********************************************************/
	PIN_ERR_CLEAR_ERR(ebufp);

	return;
}
