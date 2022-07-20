/*******************************************************************
 *
 *	Copyright (c) 2001-2007 Oracle. All rights reserved.
 *
 *	This material is the confidential property of Oracle Corporation
 *	or its licensors and may be used, reproduced, stored or transmitted
 *	only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/
#ifndef lint
static  char    Sccs_id[] = "@(#)%Portal Version: fm_inv_pol_common.c:RWSmod7.3.1Int:2:2007-Oct-05 06:02:15 %";
#endif

#include <stdio.h>
#include <string.h>
#include "pcm.h"
#include "ops/inv.h"

#include "pin_inv.h"
#include "cm_cache.h"
#include "fm_utils.h"
#include "fm_bill_utils.h"
#include "pinlog.h"
#include "fm_inv_pol_common.h"

extern cm_cache_t *fm_inv_pol_config_cache_ptr;


/*******************************************************************
 * Functions defined below
 *******************************************************************/
void
fm_inv_pol_add_str_to_buf(
        fm_inv_pol_str_buf_t   *bufp,
        char                   *string,
        pin_errbuf_t           *ebufp );

void
fm_inv_pol_format_invoice_finalize(
	fm_inv_pol_str_buf_t   *dbufp,
	char                   *type,
	pin_flist_t            **ret_flistpp,
	pin_errbuf_t           *ebufp);

void
fm_inv_pol_lookup_config_cache(
	pcm_context_t		*ctxp,
	poid_t			*a_pdp,
	pin_flist_t		**c_flistp,
	pin_errbuf_t		*ebufp);

void
fm_inv_pol_format_invoice_get_template(
	pcm_context_t           *ctxp,
	poid_t			*a_pdp,
	char			*locale,
	char			*type_str,
	pin_flist_t		**template_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * fm_inv_pol_add_str_to_buf():
 *
 * Function to keep adding to buffer with given string as input.
 * The length of the buffer is tracked with the lenp variable.
 * strlen is used to find out how much is currenlty used. pin_realloc
 * is used when needed to lengthen the buf, and the lenp will be
 * updated with the new length.
 *
 *******************************************************************/
void
fm_inv_pol_add_str_to_buf(
        fm_inv_pol_str_buf_t   *bufp,
        char                   *string,
        pin_errbuf_t           *ebufp )
{
        char                    *cp = NULL;
        char                    *op = NULL;

        if( PIN_ERR_IS_ERR( ebufp )) {
                return;
        }

        if( bufp == NULL || string == NULL ) {
                return;
        }

        /* in case they forget to set the chunk size */

        if( bufp->chunksize <= 0 ) {
                /* safeguard */
                bufp->chunksize = BUFSIZ;
        }

        /*********************************************************
         * If strp is still null, need to allocate first block
         *********************************************************/

        if( bufp->strp == NULL ) {
                bufp->strp = (char *)pin_malloc( bufp->chunksize );
                if (bufp->strp == NULL) {
                        pin_set_err(ebufp, PIN_ERRLOC_PCM,
                                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                                PIN_ERR_NO_MEM, 0, 0, 0);
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "Memory for buffer couldn't be allocated",
                                ebufp);
                        goto ErrOut;
                }

                memset( bufp->strp, 0, bufp->chunksize);
                bufp->size = bufp->chunksize;
                bufp->strsize = 0;
        }

        /***********************************************************
         * Now start copying the characters one at a time from the
         * input string to the bufp->strp buffer.  If we run out
         * of room in the bufp->strp, then realloc some more mem
         * and continue.  Keep track of size and strsize as we go.
         ***********************************************************/

        cp = string;

        /* calc starting point of output buffer */
        op = (char *)(bufp->strp + bufp->strsize );

        /* copy everything over */
        while( *cp != '\0' ) {

                /* if the string size has exceeded limit
                 * (use fudge factor of 50)
                 * Then realloc more memory
                 */
                if( bufp->strsize > bufp->size - 50 ) {

                        bufp->size = bufp->size + bufp->chunksize;
                        bufp->strp = (char *)pin_realloc( bufp->strp,
                                                          bufp->size );
			op = (char *)(bufp->strp + bufp->strsize );
                        if (bufp->strp == NULL) {
                                pin_set_err(ebufp, PIN_ERRLOC_PCM,
                                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                                        PIN_ERR_NO_MEM, 0, 0, 0);
                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "Memory for buffer couldn't be allocated",
                                                 ebufp);
                                goto ErrOut;
                        }
                }

                /* now copy the character  and also null terminator
                 * increment the local char pointers and also the
                 * strsize
                 */

                *op = *cp;
                cp++;
                op++;
                *op = '\0';
                bufp->strsize++;
        }

        /* string should be copie now */
        return;
ErrOut:
        return;
}

/***************************************************************************
 * fm_inv_pol_format_invoice_finalize():
 *
 * This function construct the return flist and put the buffer on it
 ***************************************************************************/
void
fm_inv_pol_format_invoice_finalize(
	fm_inv_pol_str_buf_t   *dbufp,
	char                   *type,
	pin_flist_t            **ret_flistpp,
	pin_errbuf_t           *ebufp)
{
	pin_buf_t		*pin_bufp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	
	/*****************************************************************
	 * Set up pin_bufp structure
	 *****************************************************************/
	pin_bufp = (pin_buf_t *) pin_malloc(sizeof(pin_buf_t));
	if (pin_bufp == NULL) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_NO_MEM, 0, 0, 0);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"Memory for pin_bufp couldn't be allocated", ebufp);
			goto ErrOut;
	}

	pin_bufp->flag = 0x0;
	pin_bufp->offset = 0;
	pin_bufp->xbuf_file = NULL;

	pin_bufp->size = dbufp->strsize + 1;  

	pin_bufp->data = (void *)dbufp->strp;
	dbufp->strp = NULL;            /* very important to set to null
					 * so that parent function does not
					 * free it
					 */

	/*****************************************************************
	 * Prepare the flist to put the format on
	 *****************************************************************/
	r_flistp = PIN_FLIST_CREATE(ebufp);

	flistp = PIN_FLIST_ELEM_ADD(r_flistp, PIN_FLD_FORMATS, 0, 
		ebufp);

	PIN_FLIST_FLD_SET(flistp, PIN_FLD_TYPE_STR, (void *)type, 
		ebufp);

	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_BUFFER, (void *)pin_bufp, ebufp);

	if( ! PIN_ERR_IS_ERR( ebufp )) {
		pin_bufp = NULL;
	}

	if( PIN_ERR_IS_ERR( ebufp )) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"Error putting buffer onto flist", ebufp);
		goto ErrOut;
	}


	*ret_flistpp = r_flistp;
	return;
ErrOut:
	if( r_flistp ) PIN_FLIST_DESTROY( r_flistp, NULL );
	if( pin_bufp ) {
		if( pin_bufp->data ) 
			pin_free( pin_bufp->data );
		pin_free( pin_bufp );
	}
	*ret_flistpp = NULL;
}

/***************************************************************************
 * fm_inv_pol_format_invoice_get_template():
 *
 * This function read the invoice template from the database
 * Used by PCM_OP_INV_POL_FORMAT_INVOICE_HTML and 
 *	 PCM_OP_INV_POL_FORMAT_INVOICE_XSLT
 ***************************************************************************/
void
fm_inv_pol_format_invoice_get_template(
	pcm_context_t           *ctxp,
	poid_t			*a_pdp,
	char			*locale,
	char			*type_str,
	pin_flist_t		**template_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*t_flistp = NULL;
	pin_flist_t		*c_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	void			*vp = NULL;

	char			*cur_locale = NULL;
	char			*name = NULL;
	int32			found = 0;
	int32			elemid = 0;
	int32			def_elemid = -1;
	pin_cookie_t		cookie = NULL;
	char 			tmp_str[1024] = {'\0'};
	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);


	/*********************************************************
	 * Then lookup cache and get the config object poid
	 *********************************************************/
	fm_inv_pol_lookup_config_cache(ctxp, a_pdp, &c_flistp, ebufp);

	if (c_flistp == NULL) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"Config cache lookup failed ",
			ebufp);
		goto ErrOut;
	}
	
	s_flistp = PIN_FLIST_CREATE(ebufp);

	vp = PIN_FLIST_FLD_GET(c_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_POID, vp, ebufp);

	/***********************************************************
	 * Read object from /config/invoice_templates.
	 ***********************************************************/
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, s_flistp, &t_flistp, ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			 "fm_inv_pol_format_invoice_get_template: "
			 "READ_OBJ error",
			 ebufp);
		goto ErrOut;
	}
	
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "Got the template flist.  Return flist :", t_flistp);

	/* Now search the config object for the type & locale */
	/* First, go through the PIN_FLD_INV_INFO array */
	found = 0;
	def_elemid = -1;
	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(t_flistp, 
			PIN_FLD_INV_INFO,  &elemid, 1, &cookie, ebufp)) 
			!= NULL) {

		name = (char *)PIN_FLIST_FLD_GET(flistp, PIN_FLD_NAME, 
			0, ebufp);
		if ( PIN_ERR_IS_ERR( ebufp ) || name == NULL) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"PIN_FLD_NAME field not found in array.", 
				ebufp);
			goto ErrOut;
		}
		cur_locale = (char *)PIN_FLIST_FLD_GET(flistp, 
			PIN_FLD_LOCALE, 0, ebufp);
		if ( PIN_ERR_IS_ERR( ebufp ) || cur_locale == NULL) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"PIN_FLD_LOCALE field not found in array.", 
				ebufp);
			goto ErrOut;
		}

		/* Keep track of the template with default locale */
		if (!strcasecmp(cur_locale, "en_US") || *cur_locale=='\0') {
			if ((*name == '\0' && !strcasecmp(type_str, "HTML"))
				|| !strcasecmp(type_str, name)) {
					def_elemid = elemid;
					/* Continue to look for a match */
			}
		}
		/* Find the element that matches the locale and type */
		if (!strcasecmp(locale, cur_locale) || 
				(!strcasecmp(locale, "en_US") && 
				*cur_locale == '\0')) {
			if ((*name == '\0' && !strcasecmp(type_str, "HTML")) 
				|| !strcasecmp(type_str, name)) {
					found = 1;
					break;
			}
		} 
	}
	
	if (!found) {
		if (def_elemid == -1) {
			if ( (strlen(type_str) + strlen(locale)) < 511 ) {
				sprintf(tmp_str, "Cannot find template for type:%s and "
					"locale:%s or default locale:en_US.", type_str,
					locale);
			}
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, tmp_str);
			goto ErrOut;
		} else {
			elemid = def_elemid;
		}
	}

	/* Now the template flist contains the buffer and the type */
	*template_flistpp = PIN_FLIST_ELEM_TAKE(t_flistp, PIN_FLD_INV_INFO, 
		elemid, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"The template buffer flist:", *template_flistpp);
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&c_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&t_flistp, NULL);
	return;
ErrOut:
	*template_flistpp = NULL;
	PIN_FLIST_DESTROY_EX(&c_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&t_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&flistp, NULL);
}

/***************************************************************************
 * fm_inv_pol_lookup_config_cache():
 *
 * This function is also used in PCM_OP_INV_POL_FORMAT_VIEW_INVOICE
 ***************************************************************************/
void
fm_inv_pol_lookup_config_cache(
	pcm_context_t		*ctxp,
	poid_t			*a_pdp,
	pin_flist_t		**c_flistp,
	pin_errbuf_t		*ebufp)
{
        cm_cache_key_poid_t     cache_key;
        int32                   err = 0;
        pin_flist_t             *r_flistp = NULL;
        pin_flist_t             *b_flistp = NULL;
        pin_flist_t             *t_flistp = NULL;
        poid_t                  *b_pdp = NULL;
        poid_t                  *c_pdp = NULL;
        poid_t                  *new_c_pdp = NULL;
        int32                   elemid = 0;
        pin_cookie_t            cookie = NULL;
 
        /***********************************************************
         * If config cache itself is empty, just return.
         * Please note that this is an exception condition.
         * At the minimum, an Portal instance should have an 
         * invoice html template belonging to the root. (0.0.0.1 /account 1) 
         ***********************************************************/
        if (fm_inv_pol_config_cache_ptr == NULL) {
          return;
        }
 
        /***********************************************************
         * Get this account's brand hierarchy and lookup for a
         * cached event string, belonging to the most immediate
         * ancestor starting from self.
         ***********************************************************/
        fm_utils_lineage_get_brand_hierarchy(ctxp, a_pdp, &b_flistp, ebufp);
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_inv_pol_lookup_config_cache() : "
                        "Call to get brand hierarchy failed ",
                        ebufp);
                goto cleanup;
                /***********/
        }
 
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "Got brand hierarchy.  Return flist :", b_flistp);
 
        while ((t_flistp = PIN_FLIST_ELEM_GET_NEXT(b_flistp,
                PIN_FLD_RESULTS, &elemid, 1, &cookie, ebufp)) !=
                (pin_flist_t *)NULL) {
 
                /***********************************************************
                 * Get the brand object (nearest to farthest ancestor)
                 ***********************************************************/
                b_pdp = PIN_FLIST_FLD_GET(t_flistp, PIN_FLD_BRAND_OBJ,
                                0, ebufp);
 
                /***********************************************************
                 * Construct the cache key and lookup
                 ***********************************************************/
                cache_key.id = PIN_POID_GET_ID ( b_pdp );
		/* Use db = 0 to find the data, */
		/* to avoid db no confusion for multi-db environment. */
                cache_key.db = 0;
                r_flistp = cm_cache_find_entry (fm_inv_pol_config_cache_ptr,
                                (void *)&cache_key, &err);
 
                if (r_flistp != (pin_flist_t *)NULL) {
			/* Found the flist in cache, but the DB No of poid  */
			/* needs update */
			c_pdp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_POID, 
				0, ebufp);
			if (PIN_POID_GET_DB(c_pdp) != PIN_POID_GET_DB(b_pdp)) {
				new_c_pdp = PIN_POID_CREATE(
					PIN_POID_GET_DB(b_pdp), 
					PIN_POID_GET_TYPE(c_pdp), 
					PIN_POID_GET_ID(c_pdp),
					ebufp);
				PIN_FLIST_FLD_PUT(r_flistp, PIN_FLD_POID, 
					(void *)new_c_pdp, ebufp);
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"Found flist in cache:", r_flistp);
			break;
                }
 
        } /* of while */
 
	*c_flistp = r_flistp;
        PIN_FLIST_DESTROY_EX (&b_flistp, NULL);
        return;
cleanup:
        PIN_FLIST_DESTROY_EX (&b_flistp, NULL);
        PIN_FLIST_DESTROY_EX (&r_flistp, NULL);
        return;
}
