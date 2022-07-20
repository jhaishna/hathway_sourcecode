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
static  char    Sccs_id[] = "@(#)%Portal Version: fm_cust_pol_valid_all.c:BillingVelocityInt:4:2006-Sep-05 04:32:22 %";
#endif


/*******************************************************************
 * Utilities used by the PCM_OP_CUST_POL_VALID_* policy operations.
 *******************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "pcm.h"
#include "ops/cust.h"
#include "hsregex.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pinlog.h"


/************************************************************************
 * Definitions.  NOTE:  May want to put some in a header file.  
 ************************************************************************/
#define EXPAND_FROMFILE        	0
#define VALIDATE_INFILE        	1
#define VALIDATE_NOTINFILE    	2
#define MAXLINE         	80

/*******************************************************************
 * Routines contained herein.
 *******************************************************************/
pin_flist_t *
fm_cust_pol_valid_add_fail(
	pin_flist_t	*r_flistp,
	u_int		field,
	u_int		elemid,
	u_int		result,
	char		*descr,
	void		*val,
	pin_errbuf_t	*ebufp);

void
fm_validate_existence(
	pin_flist_t	*flistp,
	int		partial,
	pin_flist_t	*r_flistp,	
	u_int		pin_fld_field_num,	
	u_int		pin_fld_element_id,
	u_int		*empty_field,		
	pin_errbuf_t	*ebufp);

void
fm_validate_format(
	pin_flist_t	*flistp,
	pin_flist_t	*r_flistp,
	u_int		pin_fld_field_num,	
	u_int		pin_fld_element_id,
	char		*format,		
	char		*errmsg,		
	pin_errbuf_t	*ebufp);

void 
fm_validate_value(
	pin_flist_t	*flistp,		
	pin_flist_t	*r_flistp,		
	u_int		pin_fld_field_num,	
	u_int		pin_fld_element_id,	
	char		*config_file,		
	int		type,			
	pin_errbuf_t	*ebufp);

void
fm_validate_existence_and_get(
	pin_flist_t	*flistp,		
	int		partial,
	u_int		pin_fld_field_num,	
	void		**valpp,  		
	pin_errbuf_t	*ebufp);

void
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

/************************************************************************
 * fm_cust_pol_valid_add_fail():
 *
 *   	Add a field validation failure element to the given flist
 *	using the info passed in. We assume the flist has N field
 *	failures on it already with element ids of 0 to (N-1) so
 *	we add a new element with an element id of N.
 *
 *	No error checking is done on the values passed in, but we
 *	do allow for some values to be optional according to the
 *	PCM_OP_CUST_POL_VALID_* return flist specs.
 *
 *	We return a pointer to the element we add to facilitate
 *	layering (by the caller) of the validation failures within
 *	substruct and array fields.
 *
 ************************************************************************/
pin_flist_t *
fm_cust_pol_valid_add_fail(
	pin_flist_t	*r_flistp,
	u_int		field,
	u_int		elemid,
	u_int		result,
	char		*descr,
	void		*val,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*flistp = NULL;
	int32		my_id = 0;
	u_int		type = 0;
	u_int		fail = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return(flistp);
	PIN_ERR_CLEAR_ERR(ebufp);
	
	/****************************************************************
	 * Create the field failure element.
	 ****************************************************************/
	my_id = PIN_FLIST_ELEM_COUNT(r_flistp, PIN_FLD_FIELD, ebufp); 
	flistp = PIN_FLIST_ELEM_ADD(r_flistp, PIN_FLD_FIELD, my_id, ebufp);

	/****************************************************************
	 * If this is the first time, mark the main result as _FAILED.
	 ****************************************************************/
	if (my_id == 0) {
		fail = PIN_CUST_VERIFY_FAILED;
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_RESULT,
			(void *)&fail, ebufp);
	}

	/****************************************************************
	 * The required fields.
	 ****************************************************************/
	/*
	** PIN_FLD_FIELD_NUM.
	*/
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_FIELD_NUM, (void *)&field, ebufp);

	/*
	** PIN_FLD_RESULT.
	*/
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_RESULT, (void *)&result, ebufp);

	/****************************************************************
	 * The optional fields.
	 ****************************************************************/
	/*
	** PIN_FLD_ELEMENT_ID
	*/
	if (elemid != (u_int)NULL) {
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_ELEMENT_ID,
			(void *)&elemid, ebufp);
	}

	/*
	** PIN_FLD_DESCR.
	*/
	if (descr != (char *)NULL) {
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_DESCR, (void *)descr, ebufp);
	}

	/*
	** The actual field value.
	*/
	type = PIN_FIELD_GET_TYPE(field);
	switch (type) {
	case PIN_FLDT_ARRAY:
	case PIN_FLDT_SUBSTRUCT:
		break;
	default:
		/* Val reqd on list - even if NULL */
		PIN_FLIST_FLD_SET(flistp, field, (void *)val, ebufp);
	}

	/* Error? */
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"error", ebufp);
	}

	return(flistp);
}


/************************************************************************
 * Functions used by our subs.
 ************************************************************************/
/************************************************************************
 * fm_validate_existence ()
 *
 *   	Function to validate the existence of a field.  If a null
 *	pointer was found, an error is returned.  The *empty_field
 *	flag will be set to PIN_BOOLEAN_TRUE if there's a 0 length
 *	string found.  This routine does NOT initialize this flag.
 *
 ************************************************************************/
void
fm_validate_existence(
pin_flist_t	*flistp,		/* Pointer to input flist	*/
int		partial,		/* if set, no mandatory fields  */
pin_flist_t	*r_flistp,		/* Pointer to return flist	*/
u_int		pin_fld_field_num,	/* Field currently checked	*/
u_int		pin_fld_element_id,	/* Element id of current array 	*/
u_int		*empty_field,		/* Set if empty field found	*/
pin_errbuf_t	*ebufp)			/* Error buffer pointer		*/

{
	void	*vp = NULL;		/* Void pointer			*/

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/****************************************************************
	 * Only fill the failed field descriptor if the field exists but
	 * there is nothing there (i.e. *vp == '\0').
	 ****************************************************************/
	if (partial) {
		vp = PIN_FLIST_FLD_GET(flistp, pin_fld_field_num, 1, ebufp);
	} else {
		vp = PIN_FLIST_FLD_GET(flistp, pin_fld_field_num, 0, ebufp);
	}

	if (vp != (char *)NULL && (!partial))
	{
		if (strlen(vp) == 0)
		{
			(void)fm_cust_pol_valid_add_fail(r_flistp,
				pin_fld_field_num, pin_fld_element_id,
				PIN_CUST_VAL_ERR_MISSING, 
				"Missing field.", vp, ebufp);
			*empty_field = PIN_BOOLEAN_TRUE;
		}
	} else if (!partial) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_MISSING_ARG,
			pin_fld_field_num, pin_fld_element_id, 0);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_validate_existence(): NULL ptr.", ebufp);
	}
	return;
}


/***********************************************************************
 * fm_validate_format ()
 *
 *   	Function to validate the format of a field.
 *
 ************************************************************************/
void
fm_validate_format(
pin_flist_t	*flistp,		/* Pointer to input flist	*/
pin_flist_t	*r_flistp,		/* Pointer to return flist	*/
u_int		pin_fld_field_num,	/* Field currently checked	*/
u_int		pin_fld_element_id,	/* Element id of current array 	*/
char		*format,		/* Regular expression format	*/
char		*errmsg,		/* Error message in case error	*/
pin_errbuf_t	*ebufp)			/* Error buffer pointer		*/
{
	void	*vp = NULL;	    /* Void pointer			*/
	regex_t	expbuf;
	int	err;
        char    temp[256] = "";     /* Local buffer for format      */
        char    *tok = "";          /* Parsed token from format     */
        short   match = 0;          /* Match flag                   */


	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	if ( format == NULL )
		return;

	/****************************************************************
	 * Get the field.  Just return if it couldn't find the field.
	 ****************************************************************/
	vp = PIN_FLIST_FLD_GET(flistp, pin_fld_field_num, 1, ebufp);
	if ((vp == NULL) || (strlen(vp) == 0))
	{
		return;
	}

	/****************************************************************
         * Copy format to a local temp buffer because strtok will hack
         * it up.
         ****************************************************************/
	if ( strlen(format) >= sizeof(temp) )
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE, 0, 0, 0, 0);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_validate_format(): temp buffer overflows.", ebufp);
                return;
	}
        pin_strlcpy(temp, format,sizeof(temp));

	/****************************************************************
         * Start looking for a match.  There can be more than one
	 * regular expression in format.  Each regex will be delimited
	 * by a '|'.  
	 * FIX THIS:  We will need to handle cases where '|'
	 * 		is an actual character in the pattern.
         ****************************************************************/
        tok = (char *)strtok(temp, "|");
        while (tok && !match)
        {
                if (err = regcomp(&expbuf, tok, REG_EXTENDED | REG_NOSUB) != 0)
                {
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE, 0, 0, 0, err);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_validate_format(): regcomp failed. The error is in "
				"the reserved field. See hsregex.h for the meaning.", ebufp);
			regfree(&expbuf);
                        return;
                }
                /*****************************************************
                 * regexec() returns a zero value if there is a match.
                 *****************************************************/
                if (regexec(&expbuf, vp, (size_t)0, NULL, 0))
                {
                        tok = (char *)strtok(NULL, "|");
                } else {
                        match = 1;
                }
		/* Free the memory allocated by regcomp */
		regfree(&expbuf);
	}

	/****************************************************************
	 * If a match was not found, in other words, the format was wrong,
	 * then create the appropriate flist to express the inconformance.
	 ****************************************************************/ 
	if (!match)
        {
		(void)fm_cust_pol_valid_add_fail(r_flistp, pin_fld_field_num,
			pin_fld_element_id, PIN_CUST_VAL_ERR_INVALID, 
			errmsg, vp, ebufp);
        }
	return;
}


/**************************************************************************
 * fm_validate_value()   
 *
 *	Validates abbreviated string and sets the field in the flist to
 *	an expanded string if the caller wants it and one is available.  
 *
 *************************************************************************/
void 
fm_validate_value(
pin_flist_t	*flistp,		/* Pointer to input flist	*/
pin_flist_t	*r_flistp,		/* Pointer to return flist	*/
u_int		pin_fld_field_num,	/* Field currently checked	*/
u_int		pin_fld_element_id,	/* Element id of current array 	*/
char		*config_file,		/* File used for validation	*/
int		type,			/* VALIDATE|EXPAND		*/
pin_errbuf_t	*ebufp)			/* Error buffer pointer		*/
{

	FILE	*fp = NULL;		/* Pointer to the config file	*/
	char	buf[MAXLINE];		/* Buffer to read line into	*/
	char	errmsg[MAXLINE];	/* Error message buffer		*/
	short	found = 0;		/* Set if string found		*/ 
	char	*tok = NULL;		/* Token parsed			*/
	void	*vp = NULL;		/* Void pointer			*/

	buf[0] = errmsg[0] = '\0';

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/**************************************************************** 
	 * Initialize.  If the field can't be found, then return FAILED.
	 ****************************************************************/
	found = 0;
	vp = PIN_FLIST_FLD_GET(flistp, pin_fld_field_num, 0, ebufp);
	if (vp == NULL)
	{
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_MISSING_ARG, pin_fld_field_num, 0, 0);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_validate_value(): missing field.", ebufp);
		return;
	}
	if (!(fp = fopen(config_file, "r")))
	{
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_FILE_IO, 0, 0, 0);
		pin_snprintf(errmsg, sizeof(errmsg), "fm_validate_value(): fopen(%s \"r\") failed", 
			config_file);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, errmsg, ebufp);
		return;
	}

	/****************************************************************
	 * Go through file a line at a time and check for a match.
	 ****************************************************************/
	while (!found && fgets(buf, MAXLINE, fp) != NULL) 
	{
		/********************************************************
		 * Skip comments (sort of).
		 ********************************************************/
		if (buf[0] != '#')
		{
			tok = (char *)strtok(buf, "|\n");
			if (tok == (char *)NULL) {
				break;
			}
			if (strcmp(tok, (char *)vp) == 0)
			{
				found = 1;
			}
		}
	}

	/****************************************************************
	 * Create return flist if needed.			
	 ****************************************************************/
	switch(type)
	{
	case EXPAND_FROMFILE:
		if (found)
		{
			tok = (char *)strtok(NULL, "\n");
		} else {
			(void)fm_cust_pol_valid_add_fail(r_flistp,
				pin_fld_field_num, pin_fld_element_id,
				PIN_CUST_VAL_ERR_INVALID, 
				"Not a valid value.", vp, ebufp);
		}
		if (tok)
		{
			PIN_FLIST_FLD_SET(r_flistp, pin_fld_field_num, 
				(void *)tok, ebufp);
		} else {
			(void)fm_cust_pol_valid_add_fail(r_flistp,
				pin_fld_field_num, pin_fld_element_id,
				PIN_CUST_VAL_ERR_INVALID, 
				"No expanded equivalent.", vp, ebufp);
		}
		break;

	case VALIDATE_INFILE:
		if (!found)
		{
			(void)fm_cust_pol_valid_add_fail(r_flistp,
				pin_fld_field_num, pin_fld_element_id,
				PIN_CUST_VAL_ERR_INVALID, 
				"Not a valid value.", vp, ebufp);
		}
		break;

	case VALIDATE_NOTINFILE:
		if (found)
		{
			(void)fm_cust_pol_valid_add_fail(r_flistp,
				pin_fld_field_num, pin_fld_element_id,
				PIN_CUST_VAL_ERR_INVALID, 
				"Not a valid value.", vp, ebufp);
		}
		break;

	default:
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, 0, 0, type);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_validate_value(): unexpected type", ebufp);
	}

	/****************************************************************
	 * Close the file.
	 ****************************************************************/
	if (fclose(fp) == EOF)
	{
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_FILE_IO, 0, 0, 0);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_validate_value(): fclose() failed", ebufp);
	}

	return;
}

/************************************************************************
 * fm_validate_existence_and_get ()
 *
 *   	Function to validate the existence of a field.  If a null
 *	pointer was found, an error is returned.  If not, then the
 *      (pointer to the) value is returned.
 *
 ************************************************************************/
void
fm_validate_existence_and_get(
pin_flist_t	*flistp,		/* Pointer to input flist	*/
int		partial,		/* If set, no mandatory fields  */
u_int		pin_fld_field_num,	/* Field currently checked	*/
void		**valpp,  		/* Set if empty field found	*/
pin_errbuf_t	*ebufp)			/* Error buffer pointer		*/

{
	void	*vp = NULL;			/* Void pointer			*/

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	if (partial) {
		vp = PIN_FLIST_FLD_GET(flistp, pin_fld_field_num, 1, ebufp);
	} else {
		vp = PIN_FLIST_FLD_GET(flistp, pin_fld_field_num, 0, ebufp);
	}

	if ( (vp == (void *)NULL)  && !partial) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			    PIN_ERRCLASS_SYSTEM_DETERMINATE, 
			    PIN_ERR_MISSING_ARG,
			    pin_fld_field_num, 0, 0);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				 "fm_validate_existence(): NULL ptr.", ebufp);
	}
	*valpp = vp;
	return;
}

/*******************************************************************
 * fm_cust_pol_validate_fld_value():
 *
 *	Wrapper for PCM_OP_CUST_VALID_FLD
 *	Fills in r_flistp only on errors/no_pass.
 *
 *******************************************************************/
void
fm_cust_pol_validate_fld_value( ctxp, in_flistp, i_flistp, r_flistp,
	 pin_fld_field_num, pin_fld_element_id, cfg_name, type, ebufp)
	pcm_context_t	*ctxp;
	pin_flist_t	*in_flistp;	/* flist with PIN_FLD_POID	*/
	pin_flist_t	*i_flistp;
	pin_flist_t	*r_flistp;
	pin_fld_num_t   pin_fld_field_num;
	u_int           pin_fld_element_id; 
	char		*cfg_name;
	int		type;		/* detail/not detail output	*/
	pin_errbuf_t	*ebufp;
{
	void		*fld_val = NULL;
	void		*vp = NULL;
        u_int		result = 0;
        u_int		detail = 0;
        pin_flist_t	*ci_flistp = NULL;
        pin_flist_t	*cr_flistp = NULL;
	/*pin_fld_num_t	fld_t;*/
	char		errmsg[MAXLINE];        /* Error message buffer	*/

	errmsg[0] = '\0';

	/***********************************************************
	 * Return immediately if there's an error in ebufp.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	if( cfg_name == (char *)NULL) {
		 pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_MISSING_ARG, 0, 0, 0);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_utils_validate_fld_value(): missing cfg_name field.",
			ebufp);
                return;
		/*****/
	}

	/*********************************************************
	 * get fld value
	 ********************************************************/
	fld_val = PIN_FLIST_FLD_GET(i_flistp, pin_fld_field_num, 1, ebufp);

	/*********************************************************
	 * check for poid
	 ********************************************************/
	vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	if( PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,
		"fm_utils_validate_fld_value(): PIN_FLD_POID is missing.",
		in_flistp);
		return;
		/*****/
	}

	/***********************************************************
         * Try to validate based on /config/fld_validate obj.
         ***********************************************************/
	ci_flistp = PIN_FLIST_CREATE(ebufp);

	PIN_FLIST_FLD_SET(ci_flistp, PIN_FLD_POID, vp, ebufp);
	PIN_FLIST_FLD_SET(ci_flistp, PIN_FLD_FIELD_NUM, (void *)&pin_fld_field_num,
		ebufp);
	PIN_FLIST_FLD_SET(ci_flistp, pin_fld_field_num, fld_val, ebufp);
	PIN_FLIST_FLD_SET(ci_flistp, PIN_FLD_NAME, cfg_name, ebufp);

	PCM_OP(ctxp, PCM_OP_CUST_VALID_FLD, 1, ci_flistp, &cr_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&ci_flistp, NULL);

	if( cr_flistp == (pin_flist_t *)NULL) {
		pin_snprintf(errmsg, sizeof(errmsg), 
			"fm_utils_validate_fld_value(): /config/fld_validate obj \"%s\" failed",
			cfg_name);

		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
               		0, pin_fld_field_num, 0, 0);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			errmsg, ebufp);
		return;
		/*****/
	}

	vp = PIN_FLIST_FLD_GET(cr_flistp,
			PIN_FLD_RESULT,0,ebufp);
	if (vp) {
		result = *(u_int *)vp;
	}
	if( result == PIN_CUST_VERIFY_FAILED ) {
		if( type) {
			vp = PIN_FLIST_FLD_GET(cr_flistp,
				PIN_FLD_DETAIL_TYPE,0,ebufp);
			if (vp) {
				detail = *(u_int *)vp;
			}
			(void)fm_cust_pol_valid_add_fail(r_flistp,
			 	pin_fld_field_num, pin_fld_element_id,
			 	detail, "Not a valid value.", fld_val,
			 	ebufp);
		} else {
			(void)fm_cust_pol_valid_add_fail(r_flistp,
			 	pin_fld_field_num, pin_fld_element_id,
			 	PIN_CUST_VAL_ERR_INVALID,
				"Not a valid value.", fld_val,
			 	ebufp);
		}
	}
	PIN_FLIST_DESTROY_EX(&cr_flistp, NULL);
	return;
}
