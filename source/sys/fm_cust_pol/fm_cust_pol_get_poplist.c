/*******************************************************************
 *
 *      Copyright (c) 1999-2006 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char Sccs_Id[] = "@(#)%Portal Version: fm_cust_pol_get_poplist.c:BillingVelocityInt:2:2006-Sep-05 04:30:27 %";
#endif

#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>

#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pin_pop.h"
#include "pinlog.h"

/*******************************************************************
 * Contains the PCM_OP_CUST_POL_GET_POPLIST policy operation.
 *
 * Retrieves a list of local POPs based on the given phone number.
 * If no phone number is specified, a list of all POPs is returned.
 *
 *******************************************************************/

EXPORT_OP void
op_cust_pol_get_poplist(
        cm_nap_connection_t	*connp,	
	u_int			opcode,	
        u_int			flags,	
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp);

static void
fm_cust_pol_get_poplist(
	pcm_context_t	*ctxp,		
	pin_flist_t	*in_flistp,	
	pin_flist_t	*out_flistp,
	pin_errbuf_t	*ebufp);

 
/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_GET_POPLIST operation.
 *******************************************************************/
void
op_cust_pol_get_poplist(
        cm_nap_connection_t	*connp,		/* Connection pointer	*/
	u_int			opcode,		/* Opcode in question	*/
        u_int			flags,		/* Standard flags if any*/
        pin_flist_t		*in_flistp,	/* Input flist		*/
        pin_flist_t		**ret_flistpp,	/* Results flist	*/
        pin_errbuf_t		*ebufp)		/* Error buffer		*/
{
	pcm_context_t		*ctxp = connp->dm_ctx;	/* Connection	*/
	pin_flist_t		*r_flistp = NULL;	/* Local flist	*/
	void			*vp = NULL;

	/***********************************************************
	 * Null out results until we have some.
	 ***********************************************************/
	*ret_flistpp = NULL;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_CUST_POL_GET_POPLIST) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_cust_pol_get_poplist", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: what did we get?
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_cust_pol_get_poplist input flist", in_flistp);

	/***********************************************************
	 * Create the pop flist & copy the input poid to it.
	 ***********************************************************/
	r_flistp = PIN_FLIST_CREATE(ebufp);

	vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, vp, ebufp);

	/***********************************************************
	 * Call a sub-function to actually find the pops.
	 ***********************************************************/
	fm_cust_pol_get_poplist(ctxp, in_flistp, r_flistp, ebufp);

	/***********************************************************
	 * Result.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		*ret_flistpp = (pin_flist_t *)NULL;
		PIN_FLIST_DESTROY(r_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_cust_pol_get_poplist error", ebufp);
	} else {
		*ret_flistpp = r_flistp;
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_get_poplist return flist", r_flistp);
	}

	return;
}	/* op_cust_pol_get_poplist */

/********************************************************************
 * fm_cust_pol_get_poplist()
 *
 *	Obtains a list of pops and adds to the given outflistp.
 *	If no PIN_FLD_ANI is found on the input flist, then the
 *	entire pop list is returned.  Otherwise, only the POP
 *	matching the ANI is returned. 
 *
 ********************************************************************/
static void
fm_cust_pol_get_poplist(
	pcm_context_t	*ctxp,			/* Connection context	*/
	pin_flist_t	*in_flistp,		/* Client's input flist */
	pin_flist_t	*out_flistp,		/* Search results flist */
	pin_errbuf_t	*ebufp)			/* Error buff		*/
{
	u_int64 	database;		/* Requested database 	*/
	poid_t		*poidp = NULL;		/* Input poid		*/
	poid_t		*objp = NULL;		/* New search poid	*/
	poid_t		*type_poidp;		/* Used for full poplist*/
	pin_cookie_t    cookie = NULL;  	/* For getting results	*/
	pin_cookie_t    cookie2 = NULL;  	/* For getting anis	*/
	pin_flist_t	*flistp = NULL;		/* Search flist created	*/
        pin_flist_t	*ani_flistp = NULL;	/* ANI array flist	*/
	pin_flist_t	*a_flistp = NULL;	/* New element pointer	*/
	pin_flist_t	*e_flistp = NULL;	/* Element ptr 4 pop arr*/
	pin_flist_t	*r_flistp = NULL;	/* Search return flist	*/
	pin_flist_t	*sort_flistp = NULL;	/* Sort flist		*/
	u_int64         id;			/* Search id 		*/
	u_int		type = POP_PRIMARY;	/* Only return prime pop*/ 
	int32           rec_id;         	/* PIN_FLD_RESULTS elem	*/
	int32           rec_id2;         	/* ANI results elem	*/
	void		*vp;			/* PIN_FLD_ANI value	*/
	void		*vp2;			/* Returned fields ptr	*/

	/***********************************************************
         * Check the error buffer.
         ***********************************************************/
        if (PIN_ERR_IS_ERR(ebufp))
                return;
        PIN_ERR_CLEAR_ERR(ebufp);

        /***********************************************************
         * Allocate the flist for searching.
         ***********************************************************/
        flistp = PIN_FLIST_CREATE(ebufp);

        /***********************************************************
         * Get the database number.
         ***********************************************************/
	poidp = (poid_t *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	database = PIN_POID_GET_DB(poidp);

        /***********************************************************
         * Allocate the search poid and give it to the flist.  Use
	 * the 2 arg search id if args are found, otherwise use
	 * the 1 arg search which returns all the pops ordered.
         ***********************************************************/
       	vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ANI, 1, ebufp);
	if (vp == NULL) {
		id = (u_int64)300;
	} else {
		id = (u_int64)301;
	}
        objp = PIN_POID_CREATE(database, "/search/pin", id, ebufp);
        PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)objp, ebufp);

        /***********************************************************
         * Add a search arguments array.
         ***********************************************************/
        a_flistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_ARGS, 1, ebufp);

        /***********************************************************
         * Determine whether the entire poplist should be returned
	 * or an ani to pop match should be done.  The rule is if
	 * there's a PIN_FLD_ANI field, return the entire list,
 	 * otherwise, do a ani to pop match but only return the
	 * primary one.
         ***********************************************************/
        type_poidp = PIN_POID_CREATE(database, "/pop", (int64)-1, ebufp);
	if (vp == NULL) {
		PIN_FLIST_FLD_PUT(a_flistp, PIN_FLD_POID,
			(void *)type_poidp, ebufp);
	} else {
        	/*
         	** 2 arg search, return pop that matches ani and is a
		** primary pop.
         	*/
        	ani_flistp = PIN_FLIST_CREATE(ebufp);
		/*
		** PIN_FLD_ANI for our first arg.
		*/
		PIN_FLIST_FLD_SET(ani_flistp, PIN_FLD_ANI, vp, ebufp );
		PIN_FLIST_ELEM_SET(a_flistp, ani_flistp, PIN_FLD_ANIS,
                	0, ebufp );
		/*
		** PIN_FLD_TYPE for our second arg.
		*/
		a_flistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_ARGS, 2, ebufp);
        	ani_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ani_flistp, PIN_FLD_TYPE, 
			(void *)&type, ebufp);
		PIN_FLIST_ELEM_SET(a_flistp, ani_flistp, PIN_FLD_ANIS, 1,
			ebufp);
	}

        /***********************************************************
         * Put on the PIN_FLD_RESULTS array for our results.
         ***********************************************************/
        a_flistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_RESULTS, 0, ebufp);
	/*
	** Return the pop, city, state and zip fields if the entire poplist 
	** was requested, otherwise return the same fields including
	** the flags if pop matching was requested.
	*/
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_PHONE, (void *)NULL, ebufp); 
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_CITY, (void *)NULL, ebufp); 
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_STATE, (void *)NULL, ebufp); 
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_ZIP, (void *)NULL, ebufp); 
	if (vp != NULL) {
		/*
		** Add the ANIS array if PIN_FLD_ANI arg was passed in.
		*/
        	ani_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ani_flistp, PIN_FLD_ANI, 
			(void *)NULL, ebufp);
		PIN_FLIST_FLD_SET(ani_flistp, PIN_FLD_TYPE, 
			(void *)NULL, ebufp);
		PIN_FLIST_FLD_SET(ani_flistp, PIN_FLD_FLAGS, 
			(void *)NULL, ebufp);
		PIN_FLIST_ELEM_SET(a_flistp, ani_flistp, PIN_FLD_ANIS, 0,
			ebufp);
	}
	
        /***********************************************************
         * Call the DM to do the search.
         ***********************************************************/
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, flistp, &r_flistp, ebufp);

	/***********************************************************
         * Walk the search results array adding each to the
	 * PIN_FLD_POP's array flist to pass back to the caller. 
         ***********************************************************/
        a_flistp = PIN_FLIST_ELEM_GET_NEXT(r_flistp, PIN_FLD_RESULTS,
                &rec_id, 1, &cookie, ebufp);
	while (a_flistp != (pin_flist_t *)NULL) {

        	e_flistp = PIN_FLIST_ELEM_ADD(out_flistp, PIN_FLD_POP, 
			rec_id, ebufp);

		/*
		** Take the PIN_FLD_PHONE and put it on the PIN_FLD_POP array.
		*/
		vp2 = PIN_FLIST_FLD_TAKE(a_flistp, PIN_FLD_PHONE, 0, ebufp);
		PIN_FLIST_FLD_PUT(e_flistp, PIN_FLD_PHONE, vp2, ebufp);
		/*
		** Take the PIN_FLD_CITY and put it on the PIN_FLD_POP array.
		*/
		vp2 = PIN_FLIST_FLD_TAKE(a_flistp, PIN_FLD_CITY, 0, ebufp);
		PIN_FLIST_FLD_PUT(e_flistp, PIN_FLD_CITY, vp2, ebufp);
		/*
		** Take the PIN_FLD_STATE and put it on the PIN_FLD_POP array.
		*/
		vp2 = PIN_FLIST_FLD_TAKE(a_flistp, PIN_FLD_STATE, 0, ebufp);
		PIN_FLIST_FLD_PUT(e_flistp, PIN_FLD_STATE, vp2, ebufp);
		/*
		** Take the PIN_FLD_ZIP and put it on the PIN_FLD_POP array.
		*/
		vp2 = PIN_FLIST_FLD_TAKE(a_flistp, PIN_FLD_ZIP, 0, ebufp);
		PIN_FLIST_FLD_PUT(e_flistp, PIN_FLD_ZIP, vp2, ebufp);
		/*
		** Walk the ani array (if we're pop matching) to get the 
		** PIN_FLD_FLAGS field, should only be one.
		*/
		if (vp != NULL) {
			ani_flistp = (pin_flist_t *)NULL;
			ani_flistp = PIN_FLIST_ELEM_GET_NEXT(a_flistp, 
				PIN_FLD_ANIS, &rec_id2, 0, &cookie2, ebufp);
			while (ani_flistp != (pin_flist_t *)NULL) {
				vp2 = PIN_FLIST_FLD_TAKE(ani_flistp, 
					PIN_FLD_FLAGS, 0, ebufp);
				PIN_FLIST_FLD_PUT(e_flistp, PIN_FLD_FLAGS, 
					vp2, ebufp);
				ani_flistp = PIN_FLIST_ELEM_GET_NEXT(a_flistp,
					PIN_FLD_ANIS, &rec_id2, 1, &cookie2, 
					ebufp);
			}
		}

		/***************************************************
                 * Get the next result.
                 ***************************************************/
                a_flistp = PIN_FLIST_ELEM_GET_NEXT(r_flistp,
                        PIN_FLD_RESULTS, &rec_id, 1, &cookie, ebufp);
	}

        /***********************************************************
         * Clean up.
         ***********************************************************/
	PIN_FLIST_DESTROY(flistp, NULL);
	PIN_FLIST_DESTROY(r_flistp, NULL);

        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_read_poplist error", ebufp);
        }

        return;
}	/* fm_cust_pol_read_poplist */

