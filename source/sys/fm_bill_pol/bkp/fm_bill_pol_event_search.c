/*
 *
* Copyright (c) 2004, 2015, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 *
 */

#ifndef lint
static  char    Sccs_id[] = "@(#)% %";
#endif

/*******************************************************************
 * This file contains the PCM_OP_BILL_POL_EVENT_SEARCH search operation.
 * Also included are subroutines specific to the operation. 
 *******************************************************************/

#include <stdio.h>

#include "pcm.h"
#include "ops/bill.h"
#include "cm_fm.h"
#include "pin_bill.h"
#include "pin_cust.h"
#include "pin_errs.h"
#include "pinlog.h"

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void op_bill_pol_event_search();
     void fm_bill_pol_event_search();
     void fm_bill_pol_get_item();
     void fm_bill_pol_get_event();
          
/*******************************************************************
 * Main routine for the PCM_OP_BILL_POL_EVENT_SEARCH operation.
 *******************************************************************/
void
op_bill_pol_event_search(connp, opcode, flags, i_flistp, r_flistpp, ebufp)
        cm_nap_connection_t	*connp;
	int32			opcode;
        int32			flags;
        pin_flist_t		*i_flistp;
        pin_flist_t		**r_flistpp;
        pin_errbuf_t		*ebufp;
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	 if (opcode != PCM_OP_BILL_POL_EVENT_SEARCH) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_bill_pol_event_search opcode error", ebufp);
		return;
	} 

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_bill_pol_event_search input flist", i_flistp);

	/***********************************************************
	 * Main Sub-routine to search events
	 ***********************************************************/
	fm_bill_pol_event_search(ctxp, flags, i_flistp, r_flistpp, ebufp); 

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_bill_pol_event_search error", ebufp);
	} else {
		/***************************************************
                 * Debug: What we're sending back.
                 ***************************************************/
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "op_bill_pol_event_search return flist", *r_flistpp);
        }

        return;
}

/*******************************************************************
 * fm_bill_pol_event_search()
 * This routine does setup work for reading fields from Input Flist
 * Accountpoid, BillinfoPoid and Billinfo and ItemPoids to Pass to
 * Search Template 
 ********************************************************************/
void
fm_bill_pol_event_search(ctxp, flags, i_flistp, r_flistpp, ebufp)
	pcm_context_t		*ctxp;
	u_int			flags;
	pin_flist_t		*i_flistp;
	pin_flist_t		**r_flistpp;
	pin_errbuf_t		*ebufp;
{
	int64			database = 0;
	int32                   error_result = PIN_RESULT_PASS;
	
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*item_check_flistp = NULL;
	pin_flist_t             *ret_error_flistp = NULL;
	pin_flist_t		*ret_error_message_flistp = NULL;
	pin_flist_t		*read_flistp = NULL;
	
	poid_t			*a_pdp = NULL;
	poid_t			*billinfo_pdp = NULL;
	poid_t			*bill_pdp = NULL;
	
	u_int			args = 1;
	int 		        items_supplied = PIN_BOOLEAN_FALSE;
	
	char                    *error_descr = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);
	
	
	/* Check the Account Poid is there */
	a_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)){
		/***************************************************
		 * This is an error condition
		 ************************************/
		pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_MISSING_ARG, 0, 0, 0);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "fm_bill_pol_event_search:" 
		                         "account poid not given",i_flistp);
		return;
	}
	
	/**************************************************************
	 * Check Account Object poid valid or not
	 **************************************************************/
	read_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_POID,(void *)a_pdp, ebufp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_flistp, &r_flistp,ebufp);
	PIN_FLIST_DESTROY_EX(&read_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	
	/* If Account Poid Not Exists Then Return with Appropriate Message */ 	
	if(PIN_ERR_IS_ERR(ebufp))
	{
	    PIN_ERR_CLEAR_ERR(ebufp);
	    ret_error_message_flistp = PIN_FLIST_CREATE(ebufp); 
	    PIN_FLIST_FLD_SET(ret_error_message_flistp, PIN_FLD_POID,(void *)a_pdp,
	                                                               ebufp);
	    error_descr = "Invalid Account Poid in Flist";
	    error_result = PIN_RESULT_FAIL;
	    ret_error_flistp = PIN_FLIST_ELEM_ADD (ret_error_message_flistp,
	                                       PIN_FLD_RESULTS, args, ebufp);
	    PIN_FLIST_FLD_SET (ret_error_flistp, PIN_FLD_RESULT, 
	                                        (void *)&error_result, ebufp);
	    PIN_FLIST_FLD_SET (ret_error_flistp, PIN_FLD_DESCR, 
	                                       (void *)error_descr, ebufp);
	    *r_flistpp = ret_error_message_flistp;
  	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Output Flist ",*r_flistpp);
	    return;
	}
	
	/**************************************************************
	 * Get Billinfo Object poid and check it is valid or not
	 **************************************************************/
	billinfo_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp,
						PIN_FLD_BILLINFO_OBJ, 1, ebufp);

	if(billinfo_pdp)
	{
	    read_flistp = PIN_FLIST_CREATE(ebufp);
	    PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_POID,(void *)billinfo_pdp, 
		                				ebufp);
	    PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_flistp, &r_flistp,ebufp);
	    PIN_FLIST_DESTROY_EX(&read_flistp, NULL);
            PIN_FLIST_DESTROY_EX (&r_flistp, NULL );
	    
	    /* If Billinfo Poid Not Exists Then Return with Appropriate Message */
	    if(PIN_ERR_IS_ERR(ebufp))
		{
		   PIN_ERR_CLEAR_ERR(ebufp);
		   ret_error_message_flistp = PIN_FLIST_CREATE(ebufp);
		   PIN_FLIST_FLD_SET(ret_error_message_flistp, PIN_FLD_POID,
							(void *)a_pdp,ebufp);
		   error_descr = "Invalid Billinfo Poid in Flist";
		   error_result = PIN_RESULT_FAIL;
		   ret_error_flistp = PIN_FLIST_ELEM_ADD (ret_error_message_flistp,
	                           	            PIN_FLD_RESULTS, args, ebufp);
		   PIN_FLIST_FLD_SET (ret_error_flistp, PIN_FLD_RESULT, 
	                                        (void *)&error_result, ebufp);
	           PIN_FLIST_FLD_SET (ret_error_flistp, PIN_FLD_DESCR, 
	                                       (void *)error_descr, ebufp);
	           *r_flistpp = ret_error_message_flistp;
		   return;
		}
      }
      
	/**************************************************************
	 * Get Bill Object poid and check it is valid or not
	 **************************************************************/
	bill_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp,
						PIN_FLD_BILL_OBJ, 1, ebufp);
	if(bill_pdp)
	{
	    read_flistp = PIN_FLIST_CREATE(ebufp);
	    PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_POID,(void *)bill_pdp, 
									ebufp);
	    PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_flistp, &r_flistp,ebufp);
	    PIN_FLIST_DESTROY_EX(&read_flistp, NULL);
	    PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	    				   
	    /* If Bill Poid Not Exists Then Return with Appropriate Message */
	    if(PIN_ERR_IS_ERR(ebufp))
		{
			PIN_ERR_CLEAR_ERR(ebufp);
			ret_error_message_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_error_message_flistp, PIN_FLD_POID,
							(void *)a_pdp,ebufp);
			error_descr = "Invalid Bill Poid in Flist";
			error_result = PIN_RESULT_FAIL;
			ret_error_flistp = PIN_FLIST_ELEM_ADD (ret_error_message_flistp,
	                           	            PIN_FLD_RESULTS, args, ebufp);
		        PIN_FLIST_FLD_SET (ret_error_flistp, PIN_FLD_RESULT, 
	                                        (void *)&error_result, ebufp);
	                PIN_FLIST_FLD_SET (ret_error_flistp, PIN_FLD_DESCR, 
	                                       (void *)error_descr, ebufp);
	                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ret_error_message_flistp",
	                                               ret_error_message_flistp);	
	                *r_flistpp = ret_error_message_flistp;

	                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Return Flist r_flistpp",
	                                               *r_flistpp);
		         return;
		}
      }
	

	 /******************************************************************
	  * Check Whether Items given in input Flist. If so, Call Event Search
	  * method.  Otherwise  get Items from bill or billinfo, if they are 
	  * given in input flist. If only account is given,  call Event Search 
	  * Function. 
          *****************************************************************/
        if ( PIN_FLIST_ELEM_COUNT(i_flistp, PIN_FLD_ITEMS, ebufp) > 0 ){
		fm_bill_pol_get_event(ctxp,flags,a_pdp,i_flistp,r_flistpp,ebufp);
	}
	else 	
	{
	    /* If BillInfo or Bill Object is Present Then Get the Item Poids */
		  if ((billinfo_pdp != NULL) || (bill_pdp != NULL))
		  {
		      fm_bill_pol_get_item(ctxp, flags,i_flistp,a_pdp,
		             billinfo_pdp,bill_pdp,r_flistpp,ebufp);
		  }
		 else
		 {
	     	    /* Call Event Search for the given account */
		    fm_bill_pol_get_event(ctxp,flags,a_pdp,i_flistp,r_flistpp,ebufp);
		 }
	}

	if ( PIN_ERR_IS_ERR(ebufp) ) {
                PIN_ERR_LOG_EBUF (PIN_ERR_LEVEL_ERROR,
                       "fm_bill_pol_search opcode error", ebufp) ;
                PIN_FLIST_DESTROY_EX (&r_flistp, NULL ) ; 
                *r_flistpp = NULL ;
		return ;
        }   
}

/*******************************************************************
 * fm_bill_pol_get_item ():
 * This routine does setup work for doing the searchs, namely
 * assembling the search template and the argument array on the
 * input flist to PCM_OP_SEARCH  for Valid Item Poid
 * from given Account Poid, Billinfo and Bill Poid 
 *******************************************************************/

void fm_bill_pol_get_item(ctxp, flags,i_flistp,a_pdp,billinfo_pdp,
				bill_pdp,r_flistpp,ebufp)
     	pcm_context_t	*ctxp;
        int32		flags;
	pin_flist_t	*i_flistp;
        poid_t		*a_pdp;
	poid_t		*billinfo_pdp;
	poid_t		*bill_pdp;
	pin_flist_t     **r_flistpp;
	pin_errbuf_t	*ebufp;
	
{
	pin_cookie_t	cookie = NULL;
	pin_flist_t	*s_flistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	pin_flist_t	*tflistp = NULL;
	pin_flist_t	*item_flist = NULL;
	pin_flist_t	*bflistp = NULL;
	pin_flist_t     *ret_error_item_flistp = NULL;
	pin_flist_t	*ret_error_flistp = NULL;
	pin_flist_t	*hflistp = NULL;
	
	poid_t		*s_pdp = NULL;
	poid_t		*item_objp = NULL;
	
	int32		rec_id = 0;
	int32		s_flags = SRCH_DISTINCT;
	int32		args = 1;
	int32           error_result = PIN_RESULT_PASS;
	int64		database = 0;
	
	char            *error_descr = NULL;
	char		buf[2*BUFSIZ];
	char		template[2*BUFSIZ];
	
	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);


	/**************************************************************
	 * Initialize the search template and keep adding arguments for
         * getting the list of bills. Get all the bills
	 **************************************************************/
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside fm_ar_get_item ");
	s_flistp = PIN_FLIST_CREATE(ebufp);
	database = (int64) PIN_POID_GET_DB(a_pdp);

	s_pdp = (poid_t *)PIN_POID_CREATE(database, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

        s_flags = SRCH_DISTINCT ;
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
	
	strcpy (template, "select X  from /item where  F1 = V1 ");
	args = 1 ;
	tflistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, args, ebufp);
	PIN_FLIST_FLD_SET (tflistp, PIN_FLD_ACCOUNT_OBJ,(void *) a_pdp, ebufp);
	args++;
	
	/* If Billinfo Poid Exists then add to Search Template */
	if(billinfo_pdp)
	{       
	   tflistp = PIN_FLIST_ELEM_ADD(s_flistp,PIN_FLD_ARGS, args, ebufp);
     	   PIN_FLIST_FLD_SET(tflistp, PIN_FLD_BILLINFO_OBJ,
					   (void *)billinfo_pdp, ebufp);
	   sprintf(buf, "and F%d = V%d ", args, args);
	   strcat(template, buf); 
	   args++;
	}
	
	/* If Bill Poid Exists then add to Search Template */		
	if(bill_pdp)
	{
	   tflistp = PIN_FLIST_ELEM_ADD(s_flistp,PIN_FLD_ARGS, args, ebufp);
	   PIN_FLIST_FLD_SET(tflistp, PIN_FLD_BILL_OBJ,(void *)bill_pdp, ebufp);		
	   sprintf(buf, "and F%d = V%d ", args, args);
	   strcat(template, buf); 
	   args++;
	}
	
	/**************************************************************
	 * Add the search template to the search flist
	 **************************************************************/
	PIN_FLIST_FLD_SET (s_flistp, PIN_FLD_TEMPLATE, (void *)template, ebufp);
	 
	/*********************************************************
	 * Add the results array for the search.
	 *********************************************************/
	tflistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET (tflistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET (tflistp, PIN_FLD_BILLINFO_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET (tflistp, PIN_FLD_BILL_OBJ, NULL, ebufp);
	
	/**********************************************************
	 * GET the Item  objects
 	 **********************************************************/
	PCM_OP (ctxp, PCM_OP_SEARCH, flags, s_flistp, &r_flistp, ebufp);
	
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);

        if (PIN_ERR_IS_ERR(ebufp) ) {
           PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                            "error in fm_bill_pol_get_item in search on Geeting itempoid ",
			    ebufp ) ;        
           PIN_FLIST_DESTROY_EX (&r_flistp, NULL) ;
           return ;
        }

	/* If Item Poid Not Exists Return With Apppropriate Message */
	hflistp = PIN_FLIST_ELEM_GET (r_flistp,PIN_FLD_RESULTS, 0, 1, ebufp);
  
  	if (hflistp == NULL) 
	{
	   error_descr = "Item Poid not Exists for Given Input Flist";
	   error_result = PIN_RESULT_PASS;
	   PIN_FLIST_FLD_SET (ret_error_item_flistp, PIN_FLD_POID, 
	   						(void *)a_pdp, ebufp);
	   ret_error_flistp = PIN_FLIST_ELEM_ADD (ret_error_item_flistp,
	   					PIN_FLD_RESULTS, args, ebufp);
	   PIN_FLIST_FLD_SET (ret_error_flistp, PIN_FLD_RESULT, 
	   					(void *)&error_result, ebufp);
	   PIN_FLIST_FLD_SET (ret_error_flistp, PIN_FLD_DESCR,
	   					(void *)error_descr, ebufp);
	   *r_flistpp = ret_error_item_flistp;
	   PIN_FLIST_DESTROY_EX (&r_flistp, NULL );
	   return;
	}
	
	/* If Item Poid Exists Then Add the Item Poids to Input Flist */
	
        while ( (bflistp = PIN_FLIST_ELEM_GET_NEXT (r_flistp,
		PIN_FLD_RESULTS, &rec_id, 1,&cookie, ebufp))
		                          != (pin_flist_t *)NULL ) {
				
	    item_flist = PIN_FLIST_ELEM_ADD(i_flistp,PIN_FLD_ITEMS,
	    					   args, ebufp);
	    item_objp = (poid_t *)PIN_FLIST_FLD_GET(bflistp, PIN_FLD_POID,
	    							1, ebufp);
	    PIN_FLIST_FLD_SET(item_flist,PIN_FLD_ITEM_OBJ,item_objp,ebufp);
	    args++;
	}
		
	PIN_FLIST_DESTROY_EX (&r_flistp, NULL );
	
	/* Fetch The Events */
	fm_bill_pol_get_event(ctxp,flags,a_pdp,i_flistp,r_flistpp,ebufp);
			
        if (PIN_ERR_IS_ERR(ebufp) ) {
               PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
               "fm_bill_pol_get_event - error in Fetching Billpoids",ebufp ) ;        
           	PIN_FLIST_DESTROY_EX (&r_flistp, NULL) ;
		return ;
	  }
	return ;	
                 
}    /* End  For Function to get Item Poid */
		

/*******************************************************************
 * fm_bill_pol_get_event ():
 * This routine does setup work for doing the searchs, namely
 * assembling the search template and the argument array on the
 * input flist to PCM_OP_SEARCH to search events 
 * The Function will filter Intermediate Event Types  i.e  dispute,
 * adjustment, settlement events
 *******************************************************************/
void fm_bill_pol_get_event(ctxp,flags,a_pdp,i_flistp,r_flistpp,ebufp)
	pcm_context_t		*ctxp;
	u_int			flags;
	poid_t			*a_pdp;
	pin_flist_t		*i_flistp;
	pin_flist_t             **r_flistpp;
	pin_errbuf_t		*ebufp;
{
	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie1 = NULL;
	pin_cookie_t	        cookie_threshold = NULL;	
	pin_cookie_t		cookie_called_to = NULL;
	
	int64			database = 0;
	int64			id=0;
	int32			rec_id = 0;
	int32			rec_id1 = 0;
	int32			rec_id_called_to = 0;
		
	int32			inp_cnt = 0;
	int32			threshold_result_count=0;
	int32                   non_currency_flag=0;
	int32			item_count=0;
	int32                   error_result = PIN_RESULT_PASS;
	int32			currency_flag_check = 0;
	int32			i=1;
	int32                   *resource_idp = NULL;
	int32			event_count = 0;
	
	int		        proceed_further=PIN_BOOLEAN_TRUE;
	int 			check_threshold = PIN_BOOLEAN_FALSE;
	int                     items_supplied=PIN_BOOLEAN_FALSE;
	
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	pin_flist_t             *flistp_temp = NULL;
	pin_flist_t		*flistp_temp_t = NULL;
	pin_flist_t		*balimpactsflist = NULL;
	pin_flist_t		*flistp_t = NULL;
	pin_flist_t		*flistp_tt = NULL;
	pin_flist_t		*item_flistp = NULL;
	pin_flist_t		*bal_imp_flist = NULL;
	pin_flist_t	        *result_event_flistp = NULL;
	pin_flist_t		*ret_error_flistp = NULL;
	pin_flist_t		*r_flistp_t = NULL;
	pin_flist_t		*read_flist = NULL;
	
	poid_t			*s_pdp = NULL;
	poid_t			*item_objp = NULL;
	poid_t			*service_poid = NULL;
	poid_t			*tmp_poid = NULL;
	void			*vp = NULL;
	
	u_int			s_flags = 0;
	u_int			args = 1;
	u_int			args_t = 1;
	u_int			args_tt = 1;
	
	char                    *error_descr = NULL;
	char			base_template[4*BUFSIZ];	
	char			add_template[4*BUFSIZ];
	char			template[4*BUFSIZ];
	char			buf[4*BUFSIZ];
	char                    buf1[BUFSIZ];
	const char 		*poid_type = NULL;
	char                    *event_poid_type = "event_t.poid_type";
	char                    *event_type = NULL;
	char 			*intermediate_events[] = {
					PIN_OBJ_TYPE_EVENT_ITEM_DISPUTE,
					PIN_OBJ_TYPE_EVENT_ITEM_SETTLEMENT,
					PIN_OBJ_TYPE_EVENT_ITEM_ADJUSTMENT,
					PIN_OBJ_TYPE_EVENT_SETTLEMENT,
					PIN_OBJ_TYPE_EVENT_DISPUTE,
					PIN_OBJ_TYPE_EVENT_ADJUSTMENT,
					PIN_OBJ_TYPE_TAX_EVENT_DISPUTE,
					PIN_OBJ_TYPE_TAX_EVENT_SETTLEMENT,
					PIN_OBJ_TYPE_TAX_EVENT_ADJUSTMENT,
					""};
		
	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/**************************************************************
	 * Initialize return flist.
	 **************************************************************/
	*r_flistpp = NULL;
	
	/* Check for Threshold Value in Input FList */
	vp = PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_THRESHOLD,1,ebufp);
	if (vp) 
	{
		inp_cnt = *((int32 *)vp);
	    /* Check value of inp_cnt i.e If Threshold Value is 0 or -1 
	          Then Threshold Value should not be considered */
		if(inp_cnt > 0)
		{
			check_threshold = PIN_BOOLEAN_TRUE;
		}
		 
	} 

	/* Create the default search template */
	s_flistp = PIN_FLIST_CREATE(ebufp);
	database = (int64) PIN_POID_GET_DB(a_pdp);
	s_pdp = (poid_t *)PIN_POID_CREATE(database,"/search",-1,ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	
	strcpy(base_template, " select X from /event where F1 = V1 ");
	/*****************************************************
              * Exclude Intermediate  event types
        *****************************************************/
        sprintf(buf,"and ( %s not in ( '%s'", event_poid_type,
                           intermediate_events[0]);
        for (i=0; *(event_type = intermediate_events[i]) != '\0';i++) {
		pin_snprintf(buf1, sizeof(buf1), ", '%s'", event_type);
        	strcat(buf, buf1);
		
              }
	      strcat(buf, " ) )");
              strcat(base_template, buf);	
		      	
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS,args, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACCOUNT_OBJ,(void*)a_pdp, ebufp);
	args++;
	
	item_count = 1;
	add_template[0] = '\0';
	
	/* Check For Item Poids are there in PIN_FLD_ITEMS Array */
	if ( PIN_FLIST_ELEM_COUNT(i_flistp, PIN_FLD_ITEMS, ebufp) > 0 ){
        
	/* If Item Poid Exists Then Form Dynamic Template For Item Poids */
	
		cookie=NULL;
		rec_id=0;
		strcat(add_template, " and (");
		while ((item_flistp = PIN_FLIST_ELEM_GET_NEXT (i_flistp, 
			PIN_FLD_ITEMS, &rec_id, 1, 
			&cookie, ebufp)) != (pin_flist_t *)NULL) {
			item_objp = (poid_t *)PIN_FLIST_FLD_GET(item_flistp,
						PIN_FLD_ITEM_OBJ, 0, ebufp);

			/* Count_item is Checked to Form the Dynamic Template, 
			   When Count_item value is One, Nothing is added in 
			   Template Formation. For value more than one "or" is 
			   added to Form the Query */
			id = PIN_POID_GET_ID(item_objp);
			/* If Poid is having value -1 then Item Type 
			    will be considered */
			if (id == -1)
			{
			      /* Check For Item Type */
				 poid_type = PIN_POID_GET_TYPE(item_objp);
				 flistp = PIN_FLIST_ELEM_ADD(s_flistp,
					    PIN_FLD_ARGS, args, ebufp);
				 tmp_poid = PIN_POID_CREATE(database,
						    poid_type,-1,ebufp); 
				 PIN_FLIST_FLD_SET(flistp, PIN_FLD_ITEM_OBJ,
				            (void *)tmp_poid, ebufp);
				  if(item_count == 1){ 	 
				    sprintf(buf, " F%d like V%d ", args, args);
				  }
				  else {		 
				   sprintf(buf, "or F%d like V%d ", args, args);
				  }
				  strcat(add_template, buf);
				  args++;
			}
			else {
			       	flistp = PIN_FLIST_ELEM_ADD(s_flistp,
				PIN_FLD_ARGS, args, ebufp);
				 PIN_FLIST_FLD_SET(flistp, PIN_FLD_ITEM_OBJ,
				     	         (void*)item_objp, ebufp);
				  if(item_count ==1){		      
					sprintf(buf, " F%d = V%d ", args, args);
				  }
				  else{
				     sprintf(buf, "or F%d = V%d ", args, args);
				  }
				  strcat(add_template, buf); 
				  args++;
			}
		    item_count++;
	   	}
	   	strcat(add_template, ") " );
	}

	 /* Check For Other Feilds in Input Flist */

	/**************************************************************
	 * Do we have the time boundaries?
	 **************************************************************/
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_START_T, 1, ebufp);
	if (vp) {
		flistp = PIN_FLIST_ELEM_ADD(s_flistp,PIN_FLD_ARGS, args, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_START_T,(void *)vp, ebufp);
		sprintf(buf, "and F%d >= V%d ", args, args);
		strcat(add_template, buf);
		args++;
	}

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 1, ebufp);
	if (vp) {
		flistp = PIN_FLIST_ELEM_ADD(s_flistp,PIN_FLD_ARGS, args, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_END_T,(void *)vp, ebufp);
		sprintf(buf, "and F%d <= V%d ", args, args);
		strcat(add_template, buf);
		args++;
	}
       
	/**************************************************************
         * Do we have Minimum and Maximum Amount ? 
	 **************************************************************/
        vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_MINIMUM, 1, ebufp);
	if (vp) {
		flistp = PIN_FLIST_ELEM_ADD(s_flistp,PIN_FLD_ARGS, args, ebufp);
		bal_imp_flist = PIN_FLIST_ELEM_ADD(flistp,PIN_FLD_BAL_IMPACTS,
							PIN_ELEMID_ANY,ebufp);
		PIN_FLIST_FLD_SET(bal_imp_flist,PIN_FLD_AMOUNT,vp, ebufp);
		sprintf(buf, "and F%d >= V%d ", args, args);
		strcat(add_template, buf);
		args++;
	}

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_MAXIMUM, 1, ebufp);
	if (vp) {
		flistp = PIN_FLIST_ELEM_ADD(s_flistp,PIN_FLD_ARGS, args, ebufp);
		bal_imp_flist = PIN_FLIST_ELEM_ADD(flistp,PIN_FLD_BAL_IMPACTS,
							PIN_ELEMID_ANY,ebufp);
		PIN_FLIST_FLD_SET(bal_imp_flist,PIN_FLD_AMOUNT,vp, ebufp);
		sprintf(buf, "and F%d <= V%d ", args, args);
		strcat(add_template, buf);
		args++;
	} 
	
	/**************************************************************
         * Do we have Resource Id ?
	 **************************************************************/
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_RESOURCE_ID, 1, ebufp);
	if (vp) {
		flistp = PIN_FLIST_ELEM_ADD(s_flistp,PIN_FLD_ARGS, args, ebufp);
		bal_imp_flist = PIN_FLIST_ELEM_ADD(flistp,PIN_FLD_BAL_IMPACTS,
							PIN_ELEMID_ANY,ebufp);
		PIN_FLIST_FLD_SET(bal_imp_flist,PIN_FLD_RESOURCE_ID,vp, ebufp);
		sprintf(buf, "and F%d = V%d ", args, args);
		strcat(add_template, buf);
		args++;
	}
	
	/* Check For Event Type */
	poid_type  = (char *) PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_EVENT_TYPE, 1, 
									ebufp);
	if (poid_type != NULL) {
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, args, ebufp);
		tmp_poid = PIN_POID_CREATE(database,poid_type,-1,ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID,(void *)tmp_poid, ebufp);
		sprintf(buf, "and F%d like V%d ", args, args);
		strcat(add_template, buf);
		args++;
	
	}
	
	/* Check For Service  Poid  */
	service_poid = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 
								    1, ebufp);
	if(service_poid)
	{
		id = PIN_POID_GET_ID(service_poid);
		/* If Service Poid is -1 then Check For Service Type */
		if (id == -1)
		{
			poid_type = PIN_POID_GET_TYPE(service_poid);
			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, args,
			                                               ebufp);
			tmp_poid = PIN_POID_CREATE(database,poid_type,-1,ebufp); 
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_SERVICE_OBJ,
						(void *)tmp_poid, ebufp);
			sprintf(buf, "and F%d like V%d ", args, args);
			strcat(add_template, buf);
			args++;
		}
		else
		{
	  	flistp = PIN_FLIST_ELEM_ADD(s_flistp,PIN_FLD_ARGS, args, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_SERVICE_OBJ,
						  (void *)service_poid, ebufp);
		sprintf(buf, "and F%d = V%d ", args, args);
		strcat(add_template, buf); 
		args++;
		} 
			
	}
	
	 /**************************************************************
	 * Create the default search template
	 **************************************************************/
	sprintf(template, "%s %s", base_template, add_template);
	/* Add the search template to the search flist */
	PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_TEMPLATE,(void*)template,ebufp);
	
	/**************************************************************** 
	*  If Threshold Value is Present in Input Flist Then Check for 
	*   Total Number of Events 
	*******************************************************************/
	
	if (check_threshold == PIN_BOOLEAN_TRUE){
		/* Set  the Search Flag to Zero */
		s_flags=0;
	        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp,PIN_FLD_RESULTS,0,ebufp);
		/* Set the result Array to Null */
		PIN_FLIST_ELEM_SET (s_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp );
		PCM_OP(ctxp,PCM_OP_SEARCH,PCM_OPFLG_COUNT_ONLY,s_flistp,
							    &r_flistp,ebufp);
		PIN_FLIST_ELEM_GET_NEXT(r_flistp, PIN_FLD_RESULTS,
                          &threshold_result_count, 1, &cookie_threshold, ebufp);
		
		if(PIN_ERR_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_bill_pol_event_search error in Event Count ",ebufp);
			*r_flistpp = (pin_flist_t *)NULL;
			PIN_FLIST_DESTROY_EX(&r_flistp,NULL);
			PIN_FLIST_DESTROY_EX(&s_flistp,NULL);
			return;
		}
		
	/* If Events Are More then the Threshold Value Then Return with 
							Appropriate Message */
	
		if (threshold_result_count > inp_cnt) {
			result_event_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET (result_event_flistp, PIN_FLD_POID,
		 					(void *)a_pdp, ebufp);
			flistp_temp = PIN_FLIST_ELEM_ADD (result_event_flistp, 
						PIN_FLD_RESULTS, args_t, ebufp);	
			PIN_FLIST_FLD_SET (flistp_temp, PIN_FLD_THRESHOLD, 
						&threshold_result_count, ebufp);				
			error_descr = "Events Are More then the Threshold Value";
			error_result = PIN_RESULT_FAIL;
			PIN_FLIST_FLD_SET (flistp_temp, PIN_FLD_RESULT, 
						(void *)&error_result, ebufp);
			PIN_FLIST_FLD_SET (flistp_temp, PIN_FLD_DESCR, 
						(void *)error_descr, ebufp);
			proceed_further = PIN_BOOLEAN_FALSE;
                        PIN_FLIST_DESTROY_EX(&r_flistp,NULL);
			PIN_FLIST_DESTROY_EX(&s_flistp,NULL);
			*r_flistpp = result_event_flistp;
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "OutPut Flist",
								  *r_flistpp);
			return ;
		}

		/* Events are Less then the Threshold Value so Processed Further */
		else{
			proceed_further = PIN_BOOLEAN_TRUE;
			*r_flistpp = NULL;
			PIN_FLIST_DESTROY_EX(&r_flistp,NULL);
			item_flistp = NULL;
			flistp = NULL;
			args = 1;
			item_count = 1;
			/* Drop The Flag status to Null for Search FList
			   Drop The PIN_FLD_RESULTS  from Search FList 
			   so that same search flist will be used for 
			   searcching events */
			PIN_FLIST_FLD_DROP(s_flistp,PIN_FLD_FLAGS,ebufp);
	   	        PIN_FLIST_FLD_DROP(s_flistp,PIN_FLD_RESULTS,ebufp);
	       
		}
	}
     
    /* Fetch All The Events and put in the Output Flist */
		
       if(proceed_further == PIN_BOOLEAN_TRUE)
	{
		args++;
		cookie = NULL ;
		item_count = 1;
		/* Set the Search Flag */
		s_flags=SRCH_DISTINCT;
	       
	        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, 
									ebufp);
	        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "s_flistp ",s_flistp);
		
		flistp=NULL;
		/*   Add the PIN_FLD_RESULTS  */
		flistp = PIN_FLIST_ELEM_ADD(s_flistp,PIN_FLD_RESULTS,0,ebufp);
		/* perform the actual search *
		 * Define the fields we want to get from the search */
		/* Adding PIN_FLD_EVENTS ARRAY */
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, (void *) NULL,ebufp); 
		PIN_FLIST_FLD_SET(flistp,PIN_FLD_SERVICE_OBJ,(void *)NULL,ebufp);
		PIN_FLIST_FLD_SET(flistp,PIN_FLD_CREATED_T,(void *)NULL,ebufp);
		PIN_FLIST_FLD_SET(flistp,PIN_FLD_START_T,(void *)NULL,ebufp);
		PIN_FLIST_FLD_SET(flistp,PIN_FLD_END_T,(void *)NULL,ebufp);
		PIN_FLIST_FLD_SET(flistp,PIN_FLD_DESCR,(void *)NULL,ebufp);   
		PIN_FLIST_FLD_SET(flistp,PIN_FLD_CALLED_TO,(void *)NULL,ebufp);  
		PIN_FLIST_FLD_SET(flistp,PIN_FLD_UNIT,(void *)NULL,ebufp);
		PIN_FLIST_FLD_SET(flistp,PIN_FLD_NET_QUANTITY,(void *)NULL,ebufp);  
		/* Adding PIN_FLD_BAL_IMPACTS ARRAY */
		balimpactsflist = PIN_FLIST_ELEM_ADD(flistp,PIN_FLD_BAL_IMPACTS,
							PIN_ELEMID_ANY,ebufp);
		PIN_FLIST_FLD_SET(balimpactsflist,PIN_FLD_RESOURCE_ID ,
							(void *)NULL,ebufp); 
		PIN_FLIST_FLD_SET(balimpactsflist,PIN_FLD_AMOUNT,(void *)NULL,
									ebufp);
		PIN_FLIST_FLD_SET(balimpactsflist,PIN_FLD_DISCOUNT,(void *)NULL,
									ebufp);
		
		/* Perform the search */
		PCM_OP(ctxp,PCM_OP_SEARCH,flags,s_flistp,&r_flistp,ebufp);
				
		PIN_FLIST_DESTROY_EX(&s_flistp,NULL);
		
		if(PIN_ERR_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				           "fm_bill_event_search error",ebufp);
			*r_flistpp = (pin_flist_t *)NULL;
			PIN_FLIST_DESTROY_EX(&r_flistp,NULL);
			return;
		}
		
		event_count = PIN_FLIST_ELEM_COUNT(r_flistp,PIN_FLD_RESULTS,
						  ebufp);
		result_event_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET (result_event_flistp, PIN_FLD_POID,
		 					(void *)a_pdp, ebufp);
		args_t=1;
				
		/* If Events are not Present Then Return with appropriate Message */
		if(event_count == 0)
		{
			error_descr = "No Events are present for the GivenInput Flist";
			error_result = PIN_RESULT_PASS;
			PIN_FLIST_FLD_SET (result_event_flistp, PIN_FLD_POID,
							(void *)a_pdp, ebufp);
			ret_error_flistp = PIN_FLIST_ELEM_ADD (result_event_flistp,
						PIN_FLD_RESULTS, args, ebufp);
			PIN_FLIST_FLD_SET (ret_error_flistp, PIN_FLD_RESULT,
						(void *)&error_result, ebufp);
			PIN_FLIST_FLD_SET (ret_error_flistp, PIN_FLD_DESCR,
						(void *)error_descr, ebufp);
			*r_flistpp = result_event_flistp;
			PIN_FLIST_DESTROY_EX (&r_flistp, NULL );
			return;
		}
		
		rec_id=0;
		cookie=NULL;
		
		while ((flistp = PIN_FLIST_ELEM_GET_NEXT(r_flistp,
				PIN_FLD_RESULTS,&rec_id,1,
				&cookie,ebufp)) != (pin_flist_t *)NULL) {
		
			vp = PIN_FLIST_FLD_GET (flistp, PIN_FLD_POID, 0, ebufp);
		
			/* Filter the Intermediate  Event Types  i.e  dispute,
					adjustment, settlement events*/
			poid_type = PIN_POID_GET_TYPE(vp);
			
			/* Form the Output Flist As per Requirement */
			
			flistp_temp = PIN_FLIST_ELEM_ADD (result_event_flistp,
						PIN_FLD_RESULTS, args_t, ebufp);
			PIN_FLIST_FLD_SET (flistp_temp, PIN_FLD_THRESHOLD,
							(void *)NULL, ebufp);
			error_descr = "Success";
			error_result = PIN_RESULT_PASS;
			PIN_FLIST_FLD_SET (flistp_temp, PIN_FLD_RESULT,
							(void *)&error_result,ebufp);
			PIN_FLIST_FLD_SET (flistp_temp, PIN_FLD_DESCR,
							(void *)error_descr,ebufp);
			/* Add the PIN_FLD_EVENTS Array */				
			flistp_temp_t = PIN_FLIST_ELEM_ADD (flistp_temp, 
						PIN_FLD_EVENTS, args_t, ebufp);
			
			PIN_FLIST_FLD_SET (flistp_temp_t, PIN_FLD_EVENT_OBJ, vp, 
									ebufp);
			vp = PIN_FLIST_FLD_GET (flistp, PIN_FLD_SERVICE_OBJ, 0,
									ebufp);
			PIN_FLIST_FLD_SET (flistp_temp_t, PIN_FLD_SERVICE_OBJ, vp,
									ebufp);
			vp = PIN_FLIST_FLD_GET (flistp, PIN_FLD_CREATED_T, 0, ebufp);
			PIN_FLIST_FLD_SET (flistp_temp_t, PIN_FLD_CREATED_T,
							   (void *)vp, ebufp);
			vp = PIN_FLIST_FLD_GET (flistp, PIN_FLD_START_T, 0, ebufp);
			PIN_FLIST_FLD_SET (flistp_temp_t, PIN_FLD_START_T, 
							  (void *)vp, ebufp);
			vp = PIN_FLIST_FLD_GET (flistp, PIN_FLD_END_T, 0, ebufp);
			PIN_FLIST_FLD_SET (flistp_temp_t, PIN_FLD_END_T,
			                                    (void *)vp, ebufp);
			vp = PIN_FLIST_FLD_GET (flistp, PIN_FLD_DESCR, 0, ebufp);
			PIN_FLIST_FLD_SET (flistp_temp_t, PIN_FLD_DESCR, vp, ebufp);
			/* Check Whether The Event Type is /event/activity/telco */
			if (strcmp(poid_type, "/event/activity/telco") == 0)
				
			{
				flistp_t = PIN_FLIST_CREATE (ebufp);
				vp = PIN_FLIST_FLD_GET (flistp, PIN_FLD_POID, 0, ebufp);
				PIN_FLIST_FLD_SET (flistp_t, PIN_FLD_POID, vp, ebufp);
				read_flist = PIN_FLIST_ELEM_ADD (flistp_t, 
						PIN_FLD_TELCO_INFO, PIN_ELEMID_ANY, ebufp);
				PIN_FLIST_FLD_SET (read_flist, PIN_FLD_CALLED_TO, 
						(void *)NULL, ebufp); 
				PCM_OP (ctxp, PCM_OP_READ_FLDS, 0, flistp_t, 
							&r_flistp_t, ebufp);
				vp = PIN_FLIST_FLD_GET (r_flistp_t, PIN_FLD_CALLED_TO, 
							0, ebufp);
				PIN_FLIST_FLD_SET (flistp_temp_t, PIN_FLD_CALLED_TO, 
							vp, ebufp);
				PIN_FLIST_DESTROY_EX (&r_flistp_t, NULL); 
				PIN_FLIST_DESTROY_EX (&flistp_t, NULL); 
			}
			else
			/* If PIN_FLD_CALLED_TO is Not Present 
			Set PIN_FLD_CALLED_TO to NULL Value */	
			{
				vp = "";
				PIN_FLIST_FLD_SET (flistp_temp_t, 
						PIN_FLD_CALLED_TO, vp, ebufp);

				/* Check Whether Event Type is /event/delayed/session/gprs
				Getting the BYTES_IN (Bytes downloaded) and 
				BYTES_OUT (Bytes uploaded ) values and setting in Output list */	

				if (strcmp(poid_type, "/event/delayed/session/gprs") == 0)
				{
					read_flist = PIN_FLIST_CREATE (ebufp);
					vp = PIN_FLIST_FLD_GET (flistp, 
								PIN_FLD_POID, 0, ebufp);
					PIN_FLIST_FLD_SET (read_flist, 
							PIN_FLD_POID, vp, ebufp);	
					flistp_t = PIN_FLIST_SUBSTR_ADD (read_flist, 
								PIN_FLD_GPRS_INFO, ebufp);
					PIN_FLIST_FLD_SET (flistp_t, 
							PIN_FLD_BYTES_IN, NULL, ebufp);
					PIN_FLIST_FLD_SET (flistp_t, 
							PIN_FLD_BYTES_OUT, NULL, ebufp);
					PCM_OP (ctxp, PCM_OP_READ_FLDS, 0, read_flist, 
							&r_flistp_t, ebufp);

					r_flistp_t = PIN_FLIST_SUBSTR_GET (r_flistp_t, 
							PIN_FLD_GPRS_INFO, 1, ebufp);
					vp = (poid_t *)PIN_FLIST_FLD_GET (r_flistp_t, 
							PIN_FLD_BYTES_IN, 0, ebufp);
					PIN_FLIST_FLD_SET (flistp_temp_t, 
							PIN_FLD_BYTES_IN, vp, ebufp);
					vp = (poid_t *)PIN_FLIST_FLD_GET (r_flistp_t, 
							PIN_FLD_BYTES_OUT, 0, ebufp);
					PIN_FLIST_FLD_SET (flistp_temp_t,
							PIN_FLD_BYTES_OUT, vp, ebufp);

					PIN_FLIST_DESTROY_EX (&r_flistp_t, NULL); 
					PIN_FLIST_DESTROY_EX (&read_flist, NULL); 
				}
			}
			vp = PIN_FLIST_FLD_GET (flistp, PIN_FLD_UNIT, 0, ebufp);
			PIN_FLIST_FLD_SET (flistp_temp_t, PIN_FLD_UNIT, vp, ebufp);
			
			vp = PIN_FLIST_FLD_GET (flistp, PIN_FLD_NET_QUANTITY, 0, 
									ebufp);
			PIN_FLIST_FLD_SET (flistp_temp_t, PIN_FLD_NET_QUANTITY,
								    vp, ebufp);
			PIN_FLIST_FLD_SET(flistp_temp_t,PIN_FLD_FLAGS,
					(void *) &currency_flag_check,ebufp);
			cookie1=NULL;
			args_tt =1;
			non_currency_flag=0;
			while ((flistp_t = PIN_FLIST_ELEM_GET_NEXT 
						(flistp, PIN_FLD_BAL_IMPACTS,
						&rec_id1, 1, &cookie1, ebufp))
						!= (pin_flist_t *)NULL) {
				flistp_tt  = PIN_FLIST_ELEM_ADD (flistp_temp_t, 
						PIN_FLD_BAL_IMPACTS, args_tt, ebufp);
				vp = PIN_FLIST_FLD_GET (flistp_t, 
						PIN_FLD_RESOURCE_ID, 0, ebufp);
				PIN_FLIST_FLD_SET (flistp_tt, PIN_FLD_RESOURCE_ID,
								  vp, ebufp);
				/* Checking for Currency Resource balance */
				if(!non_currency_flag)
				   {
					   vp = (void *) PIN_FLIST_FLD_GET
					   	(flistp_t,PIN_FLD_RESOURCE_ID,0,
									ebufp);
					   resource_idp = (int32 *)vp;
					   if (resource_idp &&
					  !PIN_BEID_IS_CURRENCY(*resource_idp)){
						non_currency_flag = 1;
						PIN_FLIST_FLD_SET(flistp_temp_t,
						PIN_FLD_FLAGS,
						(void *)&non_currency_flag,ebufp);
					    }
				   }
				vp = PIN_FLIST_FLD_GET (flistp_t, PIN_FLD_AMOUNT,
								    0, ebufp);
				PIN_FLIST_FLD_SET (flistp_tt, PIN_FLD_AMOUNT,
								   vp, ebufp);
				vp = PIN_FLIST_FLD_GET (flistp_t, PIN_FLD_AMOUNT,
								   0, ebufp);
				PIN_FLIST_FLD_SET (flistp_tt, PIN_FLD_AMOUNT, vp,
									ebufp);
				vp = PIN_FLIST_FLD_GET (flistp_t, PIN_FLD_DISCOUNT,
								     0, ebufp);
				PIN_FLIST_FLD_SET (flistp_tt, PIN_FLD_DISCOUNT,
								   vp, ebufp);
				args_tt++;
			}
		         args_t++;
		 }
		*r_flistpp = result_event_flistp;
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Output Flist",*r_flistpp);

		PIN_FLIST_DESTROY_EX(&r_flistp,NULL); 
		
		if (PIN_ERR_IS_ERR(ebufp))
		{
		  PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Event Search Error",
		  							ebufp);
		}
			
	} 
	return;
} /* End of fm_bill_pol_get_event Function */
	
