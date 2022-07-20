/* continuus file information --- %full_filespec: fm_cust_pol_util.c~16:csrc:1 % */
/*******************************************************************
 *
 *	 @(#) %full_filespec: fm_cust_pol_util.c~16:csrc:1 %
 *
* Copyright (c) 1999, 2013, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char Sccs_Id[] = "@(#)$Id: fm_cust_pol_util.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 04:30:19 nishahan Exp $";
#endif

#include <stdio.h>
#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "pin_cust.h"
#include "cm_cache.h"
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>

#define FILE_SOURCE_ID	"fm_cust_pol_util.c (1.31)"

/*******************************************************************
 * Local for holding cache ptr for business type
 *******************************************************************/
static pin_flist_t *fm_cust_pol_config_business_type_cache_flistp = NULL;

/*******************************************************************
 * external symbol for customer segment cm_cache pointer
 *******************************************************************/
extern cm_cache_t *fm_cust_pol_customer_segment_ptr;
        
/*******************************************************************
 * external symbol for country_currency_map cm_cache pointer
 *******************************************************************/
extern cm_cache_t *fm_cust_pol_ctrcur_map_ptr;
        
/***********************************************************************
 * The following routines are defined in this file and used elsewhere.
 ***********************************************************************/
extern void
fm_cust_pol_set_buf_fld(
	pin_flist_t		*flistp,
	u_int			fldno,
	char			*valp,
	pin_errbuf_t		*ebufp);

extern void
fm_cust_pol_copy_array(
	pin_flist_t		*dest_flistp,
	pin_flist_t		*src_flistp,
	u_int			arrayid,
	pin_errbuf_t		*ebufp);

extern void
fm_cust_pol_take_array(
	pin_flist_t		*dest_flistp,
	pin_flist_t		*src_flistp,
	u_int			arrayid,
	pin_errbuf_t		*ebufp);

extern void
fm_cust_pol_buf_create(
	pin_cust_buf_t	**bufferpp,
	pin_errbuf_t	*ebufp);


extern void
fm_cust_pol_buf_destroy(
	pin_cust_buf_t		*bufferp,
	pin_errbuf_t		*ebufp);

extern void
fm_cust_pol_buf_clear(
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
fm_cust_pol_buf_rtrim(
	pin_cust_buf_t		*bufferp);

extern int
fm_cust_pol_buf_length(
	pin_cust_buf_t		*bufferp);

/****************************************************************
 * Set a specified BUF type field onto an flist.  This is different
 * then the cust_pol_buf structure manipulated below.
 ****************************************************************/
void
fm_cust_pol_set_buf_fld(
	pin_flist_t		*flistp,
	u_int			fldno,
	char			*valp,
	pin_errbuf_t		*ebufp)
{
	pin_errbuf_t		errbuf;
	pin_buf_t		buf;

	if(ebufp == NULL) ebufp = &errbuf;
	else if (PIN_ERR_IS_ERR(ebufp)) return;

	PIN_ERR_CLEAR_ERR(ebufp);

	buf.flag = 0;
	buf.offset = 0;
	buf.xbuf_file = 0;
	buf.size = valp ? strlen(valp) : 0;
	buf.data = valp;
	PIN_FLIST_FLD_SET(flistp, fldno, (void *)&buf, ebufp);

	if(PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"flist error",ebufp);
		goto ErrOut;
	}

	return;
ErrOut:
	return;
}


/****************************************************************
 * copy array elements from the src flistp to the dest_flistp
 ****************************************************************/
void
fm_cust_pol_copy_array(
	pin_flist_t		*dest_flistp,
	pin_flist_t		*src_flistp,
	u_int			arrayid,
	pin_errbuf_t		*ebufp)
{
	pin_errbuf_t		errbuf;
	pin_cookie_t		cookie;
	pin_flist_t		*srvc_flistp = NULL;
	int32			elemid = 0;

	if(ebufp == NULL) ebufp = &errbuf;
	else if (PIN_ERR_IS_ERR(ebufp)) return;

	PIN_ERR_CLEAR_ERR(ebufp);

	cookie = NULL;
	srvc_flistp = PIN_FLIST_ELEM_GET_NEXT(src_flistp, 
			arrayid, &elemid, 0, &cookie, ebufp);
	
	while( ! PIN_ERR_IS_ERR(ebufp)) {

		PIN_FLIST_ELEM_DROP( dest_flistp, arrayid, elemid, ebufp);
		switch(ebufp->pin_err) {
		case PIN_ERR_NOT_FOUND:
			PIN_ERR_CLEAR_ERR(ebufp);
			break;
		}

		PIN_FLIST_ELEM_SET(dest_flistp, srvc_flistp, arrayid, 
			elemid, ebufp);

		srvc_flistp = PIN_FLIST_ELEM_GET_NEXT(src_flistp, 
			arrayid, &elemid, 0, &cookie, ebufp);
	}
	switch(ebufp->pin_err) {
	case PIN_ERR_NONE:
		break;
	case PIN_ERR_NOT_FOUND:
		PIN_ERR_CLEAR_ERR(ebufp);
		break;
	default:
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"flist error", ebufp);
		goto ErrOut;
	}

	return;
ErrOut:
	return;
}
/****************************************************************
 * take/put array elements from the src flistp to the dest_flistp
 ****************************************************************/
void
fm_cust_pol_take_array(
	pin_flist_t		*dest_flistp,
	pin_flist_t		*src_flistp,
	u_int			arrayid,
	pin_errbuf_t		*ebufp)
{
	pin_errbuf_t		errbuf;
	pin_cookie_t		cookie;
	pin_flist_t		*srvc_flistp = NULL;
	int32			elemid = 0;

	if(ebufp == NULL) ebufp = &errbuf;
	else if (PIN_ERR_IS_ERR(ebufp)) return;

	PIN_ERR_CLEAR_ERR(ebufp);

	cookie = NULL;
	srvc_flistp = PIN_FLIST_ELEM_TAKE_NEXT(src_flistp, 
			arrayid, &elemid, 0, &cookie, ebufp);
	
	while( ! PIN_ERR_IS_ERR(ebufp)) {

		PIN_FLIST_ELEM_DROP( dest_flistp, arrayid, elemid, ebufp);
		switch(ebufp->pin_err) {
		case PIN_ERR_NOT_FOUND:
			PIN_ERR_CLEAR_ERR(ebufp);
			break;
		}

		PIN_FLIST_ELEM_PUT(dest_flistp, srvc_flistp, arrayid, 
			elemid, ebufp);

		srvc_flistp = PIN_FLIST_ELEM_TAKE_NEXT(src_flistp, 
			arrayid, &elemid, 0, &cookie, ebufp);
	}
	switch(ebufp->pin_err) {
	case PIN_ERR_NONE:
		break;
	case PIN_ERR_NOT_FOUND:
		PIN_ERR_CLEAR_ERR(ebufp);
		break;
	default:
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"flist error", ebufp);
		goto ErrOut;
	}

	return;
ErrOut:
	return;
}


/*****************************************************************
 * This funtion will initialize a cust_pol_buf structure, the cust_pol_buf
 * structure will be used during parsing to read one character at
 * a time from the socket and stuff them into a char array of
 * arbitrary size.
 *****************************************************************/
void
fm_cust_pol_buf_create(
	pin_cust_buf_t	**bufferpp,
	pin_errbuf_t	*ebufp)
{
	pin_errbuf_t	errbuf;

	if(ebufp == NULL) ebufp = &errbuf;
	else if (PIN_ERR_IS_ERR(ebufp)) return;

	PIN_ERR_CLEAR_ERR(ebufp);

	if(bufferpp == NULL) {
                pin_set_err(ebufp, PIN_ERRLOC_FM, 
                        PIN_ERRCLASS_SYSTEM_DETERMINATE, 
                        PIN_ERR_NULL_PTR, 0, 0, 0); 
		goto ErrOut;
	}

	*bufferpp = (pin_cust_buf_t *)malloc(sizeof(pin_cust_buf_t));
	if(*bufferpp == NULL) {
                pin_set_err(ebufp, PIN_ERRLOC_FM, 
                        PIN_ERRCLASS_SYSTEM_DETERMINATE, 
                        PIN_ERR_NO_MEM, 0, 0, 0); 
		goto ErrOut;
	}

	(*bufferpp)->bufp = (char *)NULL;
	(*bufferpp)->end = 0;
	(*bufferpp)->space = 0;
	(*bufferpp)->chunk = 256;

	return;
ErrOut:
	return;
}

/*****************************************************************
 * this function will destory a cust_pol_buf objects
 *****************************************************************/
void
fm_cust_pol_buf_destroy(
	pin_cust_buf_t		*bufferp,
	pin_errbuf_t		*ebufp)
{
	pin_errbuf_t		errbuf;

	if(ebufp == NULL) ebufp = &errbuf;

	PIN_ERR_CLEAR_ERR(ebufp);

	if(bufferp == NULL) {
		return;
	}

	if(bufferp->bufp) free(bufferp->bufp);
	if(bufferp) free(bufferp);

	return;
/*ErrOut:
	return;*/
}

/*****************************************************************
 * this function will clear a cust_pol_buf objects
 *****************************************************************/
void
fm_cust_pol_buf_clear(
	pin_cust_buf_t		*bufferp,
	pin_errbuf_t		*ebufp)
{
	pin_errbuf_t		errbuf;

	if(ebufp == NULL) ebufp = &errbuf;
	else if (PIN_ERR_IS_ERR(ebufp)) return;

	PIN_ERR_CLEAR_ERR(ebufp);

	if(bufferp == NULL) {
		return;
	}

	if(bufferp->bufp) free(bufferp->bufp);
	bufferp->bufp = NULL;

	return;
/*ErrOut:
	return;*/
}


/*****************************************************************
 * This funtion will put a single character into a plans parse
 * buffer.  space will be allocated as needed in chunk sizes
 *****************************************************************/
void 
fm_cust_pol_buf_put(
	pin_cust_buf_t		*bufferp,
	char			c,
	pin_errbuf_t		*ebufp)
{
	char			*p = NULL;
	pin_errbuf_t		errbuf;

	if(ebufp == NULL) ebufp = &errbuf;
	else if (PIN_ERR_IS_ERR(ebufp)) return;

	PIN_ERR_CLEAR_ERR(ebufp);
	

	if(bufferp == NULL) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, 
			"Error - null buf");
		return;
	}

	/**********************************************************
	 * If nothing is there yet, then alloc some memory and
	 * set it all up.
	 **********************************************************/

	if(bufferp->bufp == NULL ) {
		bufferp->bufp = (char *)malloc(bufferp->chunk);
		memset( bufferp->bufp, 0, bufferp->chunk);
		bufferp->space = bufferp->chunk;
		bufferp->end = 0;
		bufferp->bufp[bufferp->end] = '\0';
	}

	/**********************************************************
	 * If The space left is not big enough, then allocate
	 * another chunk
	 **********************************************************/

	if(bufferp->end >= bufferp->space - 1 ) {
		bufferp->space = bufferp->space + bufferp->chunk;
		p = (char *)realloc(bufferp->bufp, bufferp->space);

		/***************************************************
		 * Check for error - but if it occurs, don't reassign,
		 * because realloc doesn't clean up on failure
		 ***************************************************/
		if (p == NULL) {
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				    PIN_ERRCLASS_SYSTEM_DETERMINATE,
				    PIN_ERR_NO_MEM, 0, 0, 0);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					 "fm_cust_pol_buf_put: out of memory",
					 ebufp);
			goto ErrOut;
		}
		
		bufferp->bufp = p;
	}

	/**********************************************************
	 * Assign the new character to end of buffer
	 **********************************************************/

	bufferp->bufp[bufferp->end] = c;
	bufferp->end++;
	bufferp->bufp[bufferp->end] = '\0';

 ErrOut:
	return;
}

/*****************************************************************
 * This function will concatenate a string on to the end of
 * an existing string
 *****************************************************************/
void 
fm_cust_pol_buf_cat(
	pin_cust_buf_t		*bufferp,
	char			*string,
	pin_errbuf_t		*ebufp)
{
	pin_errbuf_t		errbuf;

	if(ebufp == NULL) ebufp = &errbuf;
	else if (PIN_ERR_IS_ERR(ebufp)) return;

	PIN_ERR_CLEAR_ERR(ebufp);
	
	if ( !string ) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, 
			"Error - null string");
		return;
	}

	if(bufferp == NULL) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, 
			"Error - null buf");
		return;
	}

	/**********************************************************
	 * If nothing is there yet, then alloc some memory and
	 * set it all up.
	 **********************************************************/

	if(bufferp->bufp == NULL ) {
		bufferp->bufp = (char *)malloc(bufferp->chunk);
		if ( !bufferp->bufp ) {
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_NO_MEM, 0, 0, 0);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_cust_pol_buf_cat error: out of memory",
				ebufp);
			goto ErrOut;
		}
		memset( bufferp->bufp, 0, bufferp->chunk);
		bufferp->space = bufferp->chunk;
		bufferp->end = 0;
		bufferp->bufp[bufferp->end] = '\0';
	}

	/**********************************************************
	 * If The space left is not big enough, then allocate
	 * another chunk
	 **********************************************************/

	if(bufferp->end + strlen(string) >= (unsigned int)bufferp->space - 1 ) {
		bufferp->space = bufferp->space + 
				strlen(string) + bufferp->chunk;
		bufferp->bufp = (char *)realloc(bufferp->bufp, bufferp->space);
		if ( !bufferp->bufp ) {
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_NO_MEM, 0, 0, 0);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_cust_pol_buf_cat error: out of memory",
				ebufp);
			goto ErrOut;
		}
	}

	/**********************************************************
	 * Cat the string
	 **********************************************************/

	pin_strlcat(bufferp->bufp,string,bufferp->space);
	bufferp->end = strlen(bufferp->bufp);

ErrOut:
	return;
}


/*****************************************************************
 * This function will trim trailing spaces from the buf fields of 
 * the cust_pol_buf structure.
 *****************************************************************/
void
fm_cust_pol_buf_rtrim(
	pin_cust_buf_t		*bufferp)
{
	if(bufferp == NULL || bufferp->bufp == NULL) {
		return;
	}

	while(isspace((bufferp->bufp)[bufferp->end - 1])) {
		(bufferp->end)--;
		(bufferp->bufp)[bufferp->end] = '\0';
	}

	return;
}

/*****************************************************************
 * this returns the length of the given buf
 *****************************************************************/
int
fm_cust_pol_buf_length(
	pin_cust_buf_t		*bufferp)
{
	if(bufferp == NULL ) {
		return(0);
	}
	else {
		return((int)(bufferp->end));
	}
}

/*****************************************************************
 * return pointer to the config business type cache
 *****************************************************************/
pin_flist_t *
fm_cust_pol_util_get_config_busness_type_from_cache(
        pin_errbuf_t            *ebufp)
{
        return fm_cust_pol_config_business_type_cache_flistp;
}

/*****************************************************************
 * get the /config/business_type from DB
 *****************************************************************/
pin_flist_t *
fm_cust_pol_util_get_config_busness_type_from_db(
        pcm_context_t           *ctxp,
        int64                   db,
        pin_errbuf_t            *ebufp)
{
	poid_t                  *s_pdp = NULL;
	poid_t                  *a_pdp = NULL;
	pin_flist_t             *s_flistp = NULL;
	pin_flist_t             *r_flistp = NULL;
	pin_flist_t             *res_flistp = NULL;
	pin_flist_t             *arg_flistp = NULL;

	pin_cookie_t            cookie = NULL;
        int32                   elemid = 0;
	int32                   s_flags = 256;

        s_flistp = PIN_FLIST_CREATE(ebufp);

        s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, 
			"select X from /config where F1 = V1 ",
                       	ebufp);


        /* setup arguments */
        arg_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);

        a_pdp = PIN_POID_CREATE(db, PIN_OBJ_TYPE_CONFIG_BUSINESS_TYPE, -1, ebufp);
        PIN_FLIST_FLD_PUT(arg_flistp, PIN_FLD_POID, (void *)a_pdp,
                          ebufp);

        PIN_FLIST_ELEM_SET( s_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp);

	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_flistp, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_cust_pol_util_get_config_busness_type_from_db: "
			"error loading /config/business_type object",
                        ebufp);

                goto cleanup;
                /***********/
        }

	res_flistp = PIN_FLIST_ELEM_TAKE(r_flistp, PIN_FLD_RESULTS, 
					PIN_ELEMID_ANY, 1, ebufp);


cleanup:
        PIN_FLIST_DESTROY_EX (&s_flistp, NULL);
        PIN_FLIST_DESTROY_EX (&r_flistp, NULL);
        return res_flistp;

}


/*******************************************************************
 * fm_cust_pol_init_config_business_type_cache
 *      This routine creates a cache.
 *******************************************************************/
void
fm_cust_pol_init_config_business_type_cache(
        pcm_context_t   *ctxp,
        int64           database,
	pin_errbuf_t    *ebufp)
{
	pin_flist_t		*flistp = NULL;	
	int32			err = PIN_ERR_NONE;

        if(PIN_ERR_IS_ERR(ebufp)) {
                return;
        }

	flistp = fm_cust_pol_util_get_config_busness_type_from_db(ctxp,
                                                                  database, 
                                                                  ebufp);

        fm_cust_pol_config_business_type_cache_flistp = flistp;
}

/*****************************************************************
 * get the /config/country_currency_map from cache
 *****************************************************************/
pin_flist_t *
fm_cust_pol_util_get_config_country_currency_map_from_cache(
        char *country,
        pin_errbuf_t            *ebufp)
{
        cm_cache_key_iddbstr_t  kids;
        int32                   err = PIN_ERR_NONE;
        pin_flist_t             *flistp = (pin_flist_t *)NULL;

        /*
         * If the cache is not enabled, short circuit right away
         */
        if (fm_cust_pol_ctrcur_map_ptr == (cm_cache_t *)NULL) {
                return ((pin_flist_t *) NULL);
        }

        /*
         * See if the entry is in our data dictionary cache
         */
        kids.id = 0;    /* Not relevant for us */
        kids.db = 0;    /* Not relevant for us */
        kids.str = country;

        flistp = cm_cache_find_entry(fm_cust_pol_ctrcur_map_ptr, &kids, &err);
        switch(err) {
        case PIN_ERR_NONE:
                break;
        case PIN_ERR_NOT_FOUND:
                PIN_ERR_CLEAR_ERR(ebufp);       /* No real error... */
                break;
        default:
                pinlog(FILE_SOURCE_ID, __LINE__, LOG_FLAG_ERROR,
                        "fm_cust_pol_util_get_config_country_currency_map_from_cache: error "
			"accessing data dictionary cache.");
                pin_set_err(ebufp, PIN_ERRLOC_CM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        err, 0, 0, 0);
                return ((pin_flist_t *) NULL);
                /*****/
        }

        /* Return whatever we found in the cache */

        return flistp;
}


/*****************************************************************
 * get the /config/customer_segment from cache
 *****************************************************************/
pin_flist_t *
fm_cust_pol_util_get_config_customer_segment_from_cache(
        pin_errbuf_t            *ebufp)
{
	char			*key = PIN_OBJ_TYPE_CONFIG_CUSTOMER_SEGMENT;
	int32			err = PIN_ERR_NONE;
	pin_flist_t		*c_flistp = NULL;

        /********************************************************
         * If the cache is not enabled, short circuit right away
         ********************************************************/
	if (fm_cust_pol_customer_segment_ptr == NULL ) {
 		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Global pointer "
				"fm_cust_pol_customer_segment_ptr is NULL");

		return NULL;
	}

	/********************************************************
	 * Go through the cache to finf the customer segment obj
	 ********************************************************/
      
	c_flistp = cm_cache_find_entry(fm_cust_pol_customer_segment_ptr, 
						key, &err);
	switch(err) {
	    case PIN_ERR_NONE:
		break;
	    case PIN_ERR_NOT_FOUND:
		return NULL;
	    default:
                pinlog(FILE_SOURCE_ID, __LINE__, LOG_FLAG_ERROR,
                        "fm_cust_pol_validate_paymentterm(): error "
			"accessing data dictionary cache.");
                pin_set_err(ebufp, PIN_ERRLOC_CM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        err, 0, 0, 0);
                return NULL;
                /*****/
        }

        return c_flistp;
}

